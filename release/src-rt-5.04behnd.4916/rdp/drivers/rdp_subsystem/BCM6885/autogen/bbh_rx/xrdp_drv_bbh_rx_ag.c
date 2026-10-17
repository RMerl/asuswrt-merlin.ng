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
#include "xrdp_drv_bbh_rx_ag.h"

#define BLOCK_ADDR_COUNT_BITS 2
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_bbh_rx_timer_set(uint8_t bbh_id, uint8_t index, uint16_t timer)
{
    uint32_t reg_general_configuration_flowctrl = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, reg_general_configuration_flowctrl);

    reg_general_configuration_flowctrl = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, TIMER, reg_general_configuration_flowctrl, timer);

    RU_REG_RAM_WRITE(bbh_id, index, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, reg_general_configuration_flowctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_timer_get(uint8_t bbh_id, uint8_t index, uint16_t *timer)
{
    uint32_t reg_general_configuration_flowctrl;

#ifdef VALIDATE_PARMS
    if (!timer)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, reg_general_configuration_flowctrl);

    *timer = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, TIMER, reg_general_configuration_flowctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_flow_control_force_set(uint8_t bbh_id, uint8_t index, bdmf_boolean fcforce)
{
    uint32_t reg_general_configuration_flowctrl = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8) ||
       (fcforce >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, reg_general_configuration_flowctrl);

    reg_general_configuration_flowctrl = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, FCFORCE, reg_general_configuration_flowctrl, fcforce);

    RU_REG_RAM_WRITE(bbh_id, index, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, reg_general_configuration_flowctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_flow_control_force_get(uint8_t bbh_id, uint8_t index, bdmf_boolean *fcforce)
{
    uint32_t reg_general_configuration_flowctrl;

#ifdef VALIDATE_PARMS
    if (!fcforce)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, reg_general_configuration_flowctrl);

    *fcforce = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, FCFORCE, reg_general_configuration_flowctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_flow_control_runner_enable_set(uint8_t bbh_id, uint8_t index, bdmf_boolean fcrnren)
{
    uint32_t reg_general_configuration_flowctrl = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8) ||
       (fcrnren >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, reg_general_configuration_flowctrl);

    reg_general_configuration_flowctrl = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, FCRNREN, reg_general_configuration_flowctrl, fcrnren);

    RU_REG_RAM_WRITE(bbh_id, index, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, reg_general_configuration_flowctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_flow_control_runner_enable_get(uint8_t bbh_id, uint8_t index, bdmf_boolean *fcrnren)
{
    uint32_t reg_general_configuration_flowctrl;

#ifdef VALIDATE_PARMS
    if (!fcrnren)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, reg_general_configuration_flowctrl);

    *fcrnren = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, FCRNREN, reg_general_configuration_flowctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_flow_control_qm_enable_set(uint8_t bbh_id, uint8_t index, bdmf_boolean fcqmen)
{
    uint32_t reg_general_configuration_flowctrl = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8) ||
       (fcqmen >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, reg_general_configuration_flowctrl);

    reg_general_configuration_flowctrl = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, FCQMEN, reg_general_configuration_flowctrl, fcqmen);

    RU_REG_RAM_WRITE(bbh_id, index, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, reg_general_configuration_flowctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_flow_control_qm_enable_get(uint8_t bbh_id, uint8_t index, bdmf_boolean *fcqmen)
{
    uint32_t reg_general_configuration_flowctrl;

#ifdef VALIDATE_PARMS
    if (!fcqmen)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, reg_general_configuration_flowctrl);

    *fcqmen = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, FCQMEN, reg_general_configuration_flowctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_flow_ctrl_timer_set(uint8_t bbh_id, uint8_t index, uint16_t timer)
{
    uint32_t reg_general_configuration_flowctrl = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, reg_general_configuration_flowctrl);

    reg_general_configuration_flowctrl = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, TIMER, reg_general_configuration_flowctrl, timer);

    RU_REG_RAM_WRITE(bbh_id, index, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, reg_general_configuration_flowctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_flow_ctrl_timer_get(uint8_t bbh_id, uint8_t index, uint16_t *timer)
{
    uint32_t reg_general_configuration_flowctrl;

#ifdef VALIDATE_PARMS
    if (!timer)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, reg_general_configuration_flowctrl);

    *timer = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, TIMER, reg_general_configuration_flowctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_flow_ctrl_force_set(uint8_t bbh_id, uint8_t index, bdmf_boolean fcforce)
{
    uint32_t reg_general_configuration_flowctrl = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8) ||
       (fcforce >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, reg_general_configuration_flowctrl);

    reg_general_configuration_flowctrl = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, FCFORCE, reg_general_configuration_flowctrl, fcforce);

    RU_REG_RAM_WRITE(bbh_id, index, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, reg_general_configuration_flowctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_flow_ctrl_force_get(uint8_t bbh_id, uint8_t index, bdmf_boolean *fcforce)
{
    uint32_t reg_general_configuration_flowctrl;

#ifdef VALIDATE_PARMS
    if (!fcforce)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, reg_general_configuration_flowctrl);

    *fcforce = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, FCFORCE, reg_general_configuration_flowctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_flow_ctrl_rnr_en_set(uint8_t bbh_id, uint8_t index, bdmf_boolean fcrnren)
{
    uint32_t reg_general_configuration_flowctrl = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8) ||
       (fcrnren >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, reg_general_configuration_flowctrl);

    reg_general_configuration_flowctrl = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, FCRNREN, reg_general_configuration_flowctrl, fcrnren);

    RU_REG_RAM_WRITE(bbh_id, index, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, reg_general_configuration_flowctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_flow_ctrl_rnr_en_get(uint8_t bbh_id, uint8_t index, bdmf_boolean *fcrnren)
{
    uint32_t reg_general_configuration_flowctrl;

#ifdef VALIDATE_PARMS
    if (!fcrnren)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, reg_general_configuration_flowctrl);

    *fcrnren = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, FCRNREN, reg_general_configuration_flowctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_bbcfg_set(uint8_t bbh_id, uint16_t dispbbid, uint16_t sbpmbbid)
{
    uint32_t reg_general_configuration_bbcfg = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (dispbbid >= _9BITS_MAX_VAL_) ||
       (sbpmbbid >= _9BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_configuration_bbcfg = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_BBCFG, DISPBBID, reg_general_configuration_bbcfg, dispbbid);
    reg_general_configuration_bbcfg = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_BBCFG, SBPMBBID, reg_general_configuration_bbcfg, sbpmbbid);

    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_BBCFG, reg_general_configuration_bbcfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_bbcfg_get(uint8_t bbh_id, uint16_t *dispbbid, uint16_t *sbpmbbid)
{
    uint32_t reg_general_configuration_bbcfg;

#ifdef VALIDATE_PARMS
    if (!dispbbid || !sbpmbbid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_BBCFG, reg_general_configuration_bbcfg);

    *dispbbid = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_BBCFG, DISPBBID, reg_general_configuration_bbcfg);
    *sbpmbbid = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_BBCFG, SBPMBBID, reg_general_configuration_bbcfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_enable_set(uint8_t bbh_id, uint8_t pkten, bdmf_boolean sbpmen, bdmf_boolean geminit, bdmf_boolean grace_dis)
{
    uint32_t reg_general_configuration_enable = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (sbpmen >= _1BITS_MAX_VAL_) ||
       (geminit >= _1BITS_MAX_VAL_) ||
       (grace_dis >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_configuration_enable = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_ENABLE, PKTEN, reg_general_configuration_enable, pkten);
    reg_general_configuration_enable = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_ENABLE, SBPMEN, reg_general_configuration_enable, sbpmen);
    reg_general_configuration_enable = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_ENABLE, GEMINIT, reg_general_configuration_enable, geminit);
    reg_general_configuration_enable = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_ENABLE, GRACE_DIS, reg_general_configuration_enable, grace_dis);

    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_ENABLE, reg_general_configuration_enable);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_enable_get(uint8_t bbh_id, uint8_t *pkten, bdmf_boolean *sbpmen, bdmf_boolean *geminit, bdmf_boolean *grace_dis)
{
    uint32_t reg_general_configuration_enable;

#ifdef VALIDATE_PARMS
    if (!pkten || !sbpmen || !geminit || !grace_dis)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_ENABLE, reg_general_configuration_enable);

    *pkten = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_ENABLE, PKTEN, reg_general_configuration_enable);
    *sbpmen = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_ENABLE, SBPMEN, reg_general_configuration_enable);
    *geminit = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_ENABLE, GEMINIT, reg_general_configuration_enable);
    *grace_dis = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_ENABLE, GRACE_DIS, reg_general_configuration_enable);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_minpkt_set(uint8_t bbh_id, uint8_t minpkt_normal, uint8_t minpkt_omci)
{
    uint32_t reg_general_configuration_minpkt = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_configuration_minpkt = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MINPKT, MINPKT_NORMAL, reg_general_configuration_minpkt, minpkt_normal);
    reg_general_configuration_minpkt = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MINPKT, MINPKT_OMCI, reg_general_configuration_minpkt, minpkt_omci);

    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MINPKT, reg_general_configuration_minpkt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_minpkt_get(uint8_t bbh_id, uint8_t *minpkt_normal, uint8_t *minpkt_omci)
{
    uint32_t reg_general_configuration_minpkt;

#ifdef VALIDATE_PARMS
    if (!minpkt_normal || !minpkt_omci)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MINPKT, reg_general_configuration_minpkt);

    *minpkt_normal = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MINPKT, MINPKT_NORMAL, reg_general_configuration_minpkt);
    *minpkt_omci = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MINPKT, MINPKT_OMCI, reg_general_configuration_minpkt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_maxpkt_set(uint8_t bbh_id, uint16_t maxpkt)
{
    uint32_t reg_general_configuration_maxpkt = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (maxpkt >= _14BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_configuration_maxpkt = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MAXPKT, MAXPKT, reg_general_configuration_maxpkt, maxpkt);

    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MAXPKT, reg_general_configuration_maxpkt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_maxpkt_get(uint8_t bbh_id, uint16_t *maxpkt)
{
    uint32_t reg_general_configuration_maxpkt;

#ifdef VALIDATE_PARMS
    if (!maxpkt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MAXPKT, reg_general_configuration_maxpkt);

    *maxpkt = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MAXPKT, MAXPKT, reg_general_configuration_maxpkt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_clk_gate_cntrl_set(uint8_t bbh_id, const bbh_rx_general_configuration_clk_gate_cntrl *general_configuration_clk_gate_cntrl)
{
    uint32_t reg_general_configuration_clk_gate_cntrl = 0;

#ifdef VALIDATE_PARMS
    if(!general_configuration_clk_gate_cntrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (general_configuration_clk_gate_cntrl->bypass_clk_gate >= _1BITS_MAX_VAL_) ||
       (general_configuration_clk_gate_cntrl->keep_alive_en >= _1BITS_MAX_VAL_) ||
       (general_configuration_clk_gate_cntrl->keep_alive_intrvl >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_configuration_clk_gate_cntrl = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_CLK_GATE_CNTRL, BYPASS_CLK_GATE, reg_general_configuration_clk_gate_cntrl, general_configuration_clk_gate_cntrl->bypass_clk_gate);
    reg_general_configuration_clk_gate_cntrl = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_CLK_GATE_CNTRL, TIMER_VAL, reg_general_configuration_clk_gate_cntrl, general_configuration_clk_gate_cntrl->timer_val);
    reg_general_configuration_clk_gate_cntrl = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_CLK_GATE_CNTRL, KEEP_ALIVE_EN, reg_general_configuration_clk_gate_cntrl, general_configuration_clk_gate_cntrl->keep_alive_en);
    reg_general_configuration_clk_gate_cntrl = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_CLK_GATE_CNTRL, KEEP_ALIVE_INTRVL, reg_general_configuration_clk_gate_cntrl, general_configuration_clk_gate_cntrl->keep_alive_intrvl);
    reg_general_configuration_clk_gate_cntrl = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_CLK_GATE_CNTRL, KEEP_ALIVE_CYC, reg_general_configuration_clk_gate_cntrl, general_configuration_clk_gate_cntrl->keep_alive_cyc);

    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_CLK_GATE_CNTRL, reg_general_configuration_clk_gate_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_clk_gate_cntrl_get(uint8_t bbh_id, bbh_rx_general_configuration_clk_gate_cntrl *general_configuration_clk_gate_cntrl)
{
    uint32_t reg_general_configuration_clk_gate_cntrl;

#ifdef VALIDATE_PARMS
    if (!general_configuration_clk_gate_cntrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_CLK_GATE_CNTRL, reg_general_configuration_clk_gate_cntrl);

    general_configuration_clk_gate_cntrl->bypass_clk_gate = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_CLK_GATE_CNTRL, BYPASS_CLK_GATE, reg_general_configuration_clk_gate_cntrl);
    general_configuration_clk_gate_cntrl->timer_val = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_CLK_GATE_CNTRL, TIMER_VAL, reg_general_configuration_clk_gate_cntrl);
    general_configuration_clk_gate_cntrl->keep_alive_en = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_CLK_GATE_CNTRL, KEEP_ALIVE_EN, reg_general_configuration_clk_gate_cntrl);
    general_configuration_clk_gate_cntrl->keep_alive_intrvl = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_CLK_GATE_CNTRL, KEEP_ALIVE_INTRVL, reg_general_configuration_clk_gate_cntrl);
    general_configuration_clk_gate_cntrl->keep_alive_cyc = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_CLK_GATE_CNTRL, KEEP_ALIVE_CYC, reg_general_configuration_clk_gate_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_rxdbgsel_set(uint8_t bbh_id, uint8_t rxdbgsel)
{
    uint32_t reg_general_configuration_rxdbgsel = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (rxdbgsel >= _5BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_configuration_rxdbgsel = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_RXDBGSEL, RXDBGSEL, reg_general_configuration_rxdbgsel, rxdbgsel);

    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_RXDBGSEL, reg_general_configuration_rxdbgsel);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_rxdbgsel_get(uint8_t bbh_id, uint8_t *rxdbgsel)
{
    uint32_t reg_general_configuration_rxdbgsel;

#ifdef VALIDATE_PARMS
    if (!rxdbgsel)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_RXDBGSEL, reg_general_configuration_rxdbgsel);

    *rxdbgsel = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_RXDBGSEL, RXDBGSEL, reg_general_configuration_rxdbgsel);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_rxrstrst_set(uint8_t bbh_id, const bbh_rx_general_configuration_rxrstrst *general_configuration_rxrstrst)
{
    uint32_t reg_general_configuration_rxrstrst = 0;

#ifdef VALIDATE_PARMS
    if(!general_configuration_rxrstrst)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (general_configuration_rxrstrst->portrst >= _1BITS_MAX_VAL_) ||
       (general_configuration_rxrstrst->psramrst >= _1BITS_MAX_VAL_) ||
       (general_configuration_rxrstrst->pdfiforst >= _1BITS_MAX_VAL_) ||
       (general_configuration_rxrstrst->sbpmfiforst >= _1BITS_MAX_VAL_) ||
       (general_configuration_rxrstrst->dispcreditrst >= _1BITS_MAX_VAL_) ||
       (general_configuration_rxrstrst->upldfiforst >= _1BITS_MAX_VAL_) ||
       (general_configuration_rxrstrst->dispcredit >= _7BITS_MAX_VAL_) ||
       (general_configuration_rxrstrst->dispviq >= _9BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_configuration_rxrstrst = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_RXRSTRST, PORTRST, reg_general_configuration_rxrstrst, general_configuration_rxrstrst->portrst);
    reg_general_configuration_rxrstrst = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_RXRSTRST, PSRAMRST, reg_general_configuration_rxrstrst, general_configuration_rxrstrst->psramrst);
    reg_general_configuration_rxrstrst = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_RXRSTRST, PDFIFORST, reg_general_configuration_rxrstrst, general_configuration_rxrstrst->pdfiforst);
    reg_general_configuration_rxrstrst = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_RXRSTRST, SBPMFIFORST, reg_general_configuration_rxrstrst, general_configuration_rxrstrst->sbpmfiforst);
    reg_general_configuration_rxrstrst = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_RXRSTRST, DISPCREDITRST, reg_general_configuration_rxrstrst, general_configuration_rxrstrst->dispcreditrst);
    reg_general_configuration_rxrstrst = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_RXRSTRST, UPLDFIFORST, reg_general_configuration_rxrstrst, general_configuration_rxrstrst->upldfiforst);
    reg_general_configuration_rxrstrst = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_RXRSTRST, PFBCH, reg_general_configuration_rxrstrst, general_configuration_rxrstrst->pfbch);
    reg_general_configuration_rxrstrst = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_RXRSTRST, DISPCREDIT, reg_general_configuration_rxrstrst, general_configuration_rxrstrst->dispcredit);
    reg_general_configuration_rxrstrst = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_RXRSTRST, DISPVIQ, reg_general_configuration_rxrstrst, general_configuration_rxrstrst->dispviq);

    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_RXRSTRST, reg_general_configuration_rxrstrst);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_rxrstrst_get(uint8_t bbh_id, bbh_rx_general_configuration_rxrstrst *general_configuration_rxrstrst)
{
    uint32_t reg_general_configuration_rxrstrst;

#ifdef VALIDATE_PARMS
    if (!general_configuration_rxrstrst)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_RXRSTRST, reg_general_configuration_rxrstrst);

    general_configuration_rxrstrst->portrst = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_RXRSTRST, PORTRST, reg_general_configuration_rxrstrst);
    general_configuration_rxrstrst->psramrst = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_RXRSTRST, PSRAMRST, reg_general_configuration_rxrstrst);
    general_configuration_rxrstrst->pdfiforst = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_RXRSTRST, PDFIFORST, reg_general_configuration_rxrstrst);
    general_configuration_rxrstrst->sbpmfiforst = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_RXRSTRST, SBPMFIFORST, reg_general_configuration_rxrstrst);
    general_configuration_rxrstrst->dispcreditrst = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_RXRSTRST, DISPCREDITRST, reg_general_configuration_rxrstrst);
    general_configuration_rxrstrst->upldfiforst = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_RXRSTRST, UPLDFIFORST, reg_general_configuration_rxrstrst);
    general_configuration_rxrstrst->pfbch = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_RXRSTRST, PFBCH, reg_general_configuration_rxrstrst);
    general_configuration_rxrstrst->dispcredit = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_RXRSTRST, DISPCREDIT, reg_general_configuration_rxrstrst);
    general_configuration_rxrstrst->dispviq = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_RXRSTRST, DISPVIQ, reg_general_configuration_rxrstrst);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_sbpmcfg_set(uint8_t bbh_id, uint8_t max_otf_sbpm_req)
{
    uint32_t reg_general_configuration_sbpmcfg = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (max_otf_sbpm_req >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_configuration_sbpmcfg = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_SBPMCFG, MAXREQ, reg_general_configuration_sbpmcfg, max_otf_sbpm_req);

    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_SBPMCFG, reg_general_configuration_sbpmcfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_sbpmcfg_get(uint8_t bbh_id, uint8_t *max_otf_sbpm_req)
{
    uint32_t reg_general_configuration_sbpmcfg;

#ifdef VALIDATE_PARMS
    if (!max_otf_sbpm_req)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_SBPMCFG, reg_general_configuration_sbpmcfg);

    *max_otf_sbpm_req = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_SBPMCFG, MAXREQ, reg_general_configuration_sbpmcfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_psrambase_set(uint8_t bbh_id, uint32_t psrambase)
{
    uint32_t reg_general_configuration_psrambase = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (psrambase >= _20BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_configuration_psrambase = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PSRAMBASE, PSRAMBASE, reg_general_configuration_psrambase, psrambase);

    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PSRAMBASE, reg_general_configuration_psrambase);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_psrambase_get(uint8_t bbh_id, uint32_t *psrambase)
{
    uint32_t reg_general_configuration_psrambase;

#ifdef VALIDATE_PARMS
    if (!psrambase)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PSRAMBASE, reg_general_configuration_psrambase);

    *psrambase = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PSRAMBASE, PSRAMBASE, reg_general_configuration_psrambase);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_ingresschannel_set(uint8_t bbh_id, uint8_t ingresschannel)
{
    uint32_t reg_general_configuration_ingresschannel = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (ingresschannel >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_configuration_ingresschannel = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_INGRESSCHANNEL, INGRESSCHANNEL, reg_general_configuration_ingresschannel, ingresschannel);

    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_INGRESSCHANNEL, reg_general_configuration_ingresschannel);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_ingresschannel_get(uint8_t bbh_id, uint8_t *ingresschannel)
{
    uint32_t reg_general_configuration_ingresschannel;

#ifdef VALIDATE_PARMS
    if (!ingresschannel)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_INGRESSCHANNEL, reg_general_configuration_ingresschannel);

    *ingresschannel = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_INGRESSCHANNEL, INGRESSCHANNEL, reg_general_configuration_ingresschannel);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_bbovrdroute_set(uint8_t bbh_id, uint32_t route)
{
    uint32_t reg_general_configuration_bbovrdroute = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (route >= _25BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_configuration_bbovrdroute = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_BBOVRDROUTE, ROUTE, reg_general_configuration_bbovrdroute, route);

    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_BBOVRDROUTE, reg_general_configuration_bbovrdroute);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_bbovrdroute_get(uint8_t bbh_id, uint32_t *route)
{
    uint32_t reg_general_configuration_bbovrdroute;

#ifdef VALIDATE_PARMS
    if (!route)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_BBOVRDROUTE, reg_general_configuration_bbovrdroute);

    *route = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_BBOVRDROUTE, ROUTE, reg_general_configuration_bbovrdroute);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_bbovrdctrl_set(uint8_t bbh_id, uint16_t dest, bdmf_boolean en)
{
    uint32_t reg_general_configuration_bbovrdctrl = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (dest >= _9BITS_MAX_VAL_) ||
       (en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_configuration_bbovrdctrl = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_BBOVRDCTRL, DEST, reg_general_configuration_bbovrdctrl, dest);
    reg_general_configuration_bbovrdctrl = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_BBOVRDCTRL, EN, reg_general_configuration_bbovrdctrl, en);

    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_BBOVRDCTRL, reg_general_configuration_bbovrdctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_bbovrdctrl_get(uint8_t bbh_id, uint16_t *dest, bdmf_boolean *en)
{
    uint32_t reg_general_configuration_bbovrdctrl;

#ifdef VALIDATE_PARMS
    if (!dest || !en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_BBOVRDCTRL, reg_general_configuration_bbovrdctrl);

    *dest = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_BBOVRDCTRL, DEST, reg_general_configuration_bbovrdctrl);
    *en = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_BBOVRDCTRL, EN, reg_general_configuration_bbovrdctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_gemdone_get(uint8_t bbh_id, bdmf_boolean *gemdone)
{
    uint32_t reg_general_configuration_gemdone;

#ifdef VALIDATE_PARMS
    if (!gemdone)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_GEMDONE, reg_general_configuration_gemdone);

    *gemdone = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_GEMDONE, GEMDONE, reg_general_configuration_gemdone);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_mactype_set(uint8_t bbh_id, uint8_t index, uint8_t mactype)
{
    uint32_t reg_general_configuration_mactype = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8) ||
       (mactype >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_configuration_mactype = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MACTYPE, MACTYPE, reg_general_configuration_mactype, mactype);

    RU_REG_RAM_WRITE(bbh_id, index, BBH_RX, GENERAL_CONFIGURATION_MACTYPE, reg_general_configuration_mactype);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_mactype_get(uint8_t bbh_id, uint8_t index, uint8_t *mactype)
{
    uint32_t reg_general_configuration_mactype;

#ifdef VALIDATE_PARMS
    if (!mactype)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, BBH_RX, GENERAL_CONFIGURATION_MACTYPE, reg_general_configuration_mactype);

    *mactype = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MACTYPE, MACTYPE, reg_general_configuration_mactype);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_portth_set(uint8_t bbh_id, uint8_t index, uint8_t portthr)
{
    uint32_t reg_general_configuration_portth = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8) ||
       (portthr >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_configuration_portth = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PORTTH, PORTTHR, reg_general_configuration_portth, portthr);

    RU_REG_RAM_WRITE(bbh_id, index, BBH_RX, GENERAL_CONFIGURATION_PORTTH, reg_general_configuration_portth);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_portth_get(uint8_t bbh_id, uint8_t index, uint8_t *portthr)
{
    uint32_t reg_general_configuration_portth;

#ifdef VALIDATE_PARMS
    if (!portthr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, BBH_RX, GENERAL_CONFIGURATION_PORTTH, reg_general_configuration_portth);

    *portthr = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PORTTH, PORTTHR, reg_general_configuration_portth);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_sopoffset_set(uint8_t bbh_id, uint8_t index, uint8_t sopoffset)
{
    uint32_t reg_general_configuration_sopoffset = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8) ||
       (sopoffset >= _7BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_configuration_sopoffset = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_SOPOFFSET, SOPOFFSET, reg_general_configuration_sopoffset, sopoffset);

    RU_REG_RAM_WRITE(bbh_id, index, BBH_RX, GENERAL_CONFIGURATION_SOPOFFSET, reg_general_configuration_sopoffset);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_sopoffset_get(uint8_t bbh_id, uint8_t index, uint8_t *sopoffset)
{
    uint32_t reg_general_configuration_sopoffset;

#ifdef VALIDATE_PARMS
    if (!sopoffset)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, BBH_RX, GENERAL_CONFIGURATION_SOPOFFSET, reg_general_configuration_sopoffset);

    *sopoffset = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_SOPOFFSET, SOPOFFSET, reg_general_configuration_sopoffset);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_ingresspid_set(uint8_t bbh_id, uint8_t channel_num, uint8_t ingresspid)
{
    uint32_t reg_general_configuration_ingresspid = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (channel_num >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_configuration_ingresspid = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_INGRESSPID, INGRESSPID, reg_general_configuration_ingresspid, ingresspid);

    RU_REG_RAM_WRITE(bbh_id, channel_num, BBH_RX, GENERAL_CONFIGURATION_INGRESSPID, reg_general_configuration_ingresspid);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_ingresspid_get(uint8_t bbh_id, uint8_t channel_num, uint8_t *ingresspid)
{
    uint32_t reg_general_configuration_ingresspid;

#ifdef VALIDATE_PARMS
    if (!ingresspid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (channel_num >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, channel_num, BBH_RX, GENERAL_CONFIGURATION_INGRESSPID, reg_general_configuration_ingresspid);

    *ingresspid = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_INGRESSPID, INGRESSPID, reg_general_configuration_ingresspid);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_portweight_set(uint8_t bbh_id, uint8_t index, uint8_t portweight)
{
    uint32_t reg_general_configuration_portweight = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8) ||
       (portweight >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_configuration_portweight = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PORTWEIGHT, PORTWEIGHT, reg_general_configuration_portweight, portweight);

    RU_REG_RAM_WRITE(bbh_id, index, BBH_RX, GENERAL_CONFIGURATION_PORTWEIGHT, reg_general_configuration_portweight);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_portweight_get(uint8_t bbh_id, uint8_t index, uint8_t *portweight)
{
    uint32_t reg_general_configuration_portweight;

#ifdef VALIDATE_PARMS
    if (!portweight)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, BBH_RX, GENERAL_CONFIGURATION_PORTWEIGHT, reg_general_configuration_portweight);

    *portweight = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PORTWEIGHT, PORTWEIGHT, reg_general_configuration_portweight);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_portviqmask_set(uint8_t bbh_id, uint8_t viq_index, uint16_t portviqmask)
{
    uint32_t reg_general_configuration_portviqmask = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (viq_index >= 8) ||
       (portviqmask >= _9BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_configuration_portviqmask = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PORTVIQMASK, PORTVIQMASK, reg_general_configuration_portviqmask, portviqmask);

    RU_REG_RAM_WRITE(bbh_id, viq_index, BBH_RX, GENERAL_CONFIGURATION_PORTVIQMASK, reg_general_configuration_portviqmask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_portviqmask_get(uint8_t bbh_id, uint8_t viq_index, uint16_t *portviqmask)
{
    uint32_t reg_general_configuration_portviqmask;

#ifdef VALIDATE_PARMS
    if (!portviqmask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (viq_index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, viq_index, BBH_RX, GENERAL_CONFIGURATION_PORTVIQMASK, reg_general_configuration_portviqmask);

    *portviqmask = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PORTVIQMASK, PORTVIQMASK, reg_general_configuration_portviqmask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_excviqmask_set(uint8_t bbh_id, uint8_t viq_index, uint16_t excviqmask, bdmf_boolean excviqen, bdmf_boolean excviqoamen)
{
    uint32_t reg_general_configuration_excviqmask = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (viq_index >= 8) ||
       (excviqmask >= _9BITS_MAX_VAL_) ||
       (excviqen >= _1BITS_MAX_VAL_) ||
       (excviqoamen >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_configuration_excviqmask = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_EXCVIQMASK, EXCVIQMASK, reg_general_configuration_excviqmask, excviqmask);
    reg_general_configuration_excviqmask = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_EXCVIQMASK, EXCVIQEN, reg_general_configuration_excviqmask, excviqen);
    reg_general_configuration_excviqmask = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_EXCVIQMASK, EXCVIQOAMEN, reg_general_configuration_excviqmask, excviqoamen);

    RU_REG_RAM_WRITE(bbh_id, viq_index, BBH_RX, GENERAL_CONFIGURATION_EXCVIQMASK, reg_general_configuration_excviqmask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_excviqmask_get(uint8_t bbh_id, uint8_t viq_index, uint16_t *excviqmask, bdmf_boolean *excviqen, bdmf_boolean *excviqoamen)
{
    uint32_t reg_general_configuration_excviqmask;

#ifdef VALIDATE_PARMS
    if (!excviqmask || !excviqen || !excviqoamen)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (viq_index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, viq_index, BBH_RX, GENERAL_CONFIGURATION_EXCVIQMASK, reg_general_configuration_excviqmask);

    *excviqmask = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_EXCVIQMASK, EXCVIQMASK, reg_general_configuration_excviqmask);
    *excviqen = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_EXCVIQMASK, EXCVIQEN, reg_general_configuration_excviqmask);
    *excviqoamen = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_EXCVIQMASK, EXCVIQOAMEN, reg_general_configuration_excviqmask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_dispviq_set(uint8_t bbh_id, uint8_t viq_index, uint8_t viq)
{
    uint32_t reg_general_configuration_dispviq = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (viq_index >= 9) ||
       (viq >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_configuration_dispviq = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_DISPVIQ, VIQ, reg_general_configuration_dispviq, viq);

    RU_REG_RAM_WRITE(bbh_id, viq_index, BBH_RX, GENERAL_CONFIGURATION_DISPVIQ, reg_general_configuration_dispviq);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_dispviq_get(uint8_t bbh_id, uint8_t viq_index, uint8_t *viq)
{
    uint32_t reg_general_configuration_dispviq;

#ifdef VALIDATE_PARMS
    if (!viq)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (viq_index >= 9))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, viq_index, BBH_RX, GENERAL_CONFIGURATION_DISPVIQ, reg_general_configuration_dispviq);

    *viq = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_DISPVIQ, VIQ, reg_general_configuration_dispviq);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_viqweight_set(uint8_t bbh_id, uint8_t index, uint8_t viqweight)
{
    uint32_t reg_general_configuration_viqweight = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 9) ||
       (viqweight >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_configuration_viqweight = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_VIQWEIGHT, VIQWEIGHT, reg_general_configuration_viqweight, viqweight);

    RU_REG_RAM_WRITE(bbh_id, index, BBH_RX, GENERAL_CONFIGURATION_VIQWEIGHT, reg_general_configuration_viqweight);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_viqweight_get(uint8_t bbh_id, uint8_t index, uint8_t *viqweight)
{
    uint32_t reg_general_configuration_viqweight;

#ifdef VALIDATE_PARMS
    if (!viqweight)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 9))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, BBH_RX, GENERAL_CONFIGURATION_VIQWEIGHT, reg_general_configuration_viqweight);

    *viqweight = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_VIQWEIGHT, VIQWEIGHT, reg_general_configuration_viqweight);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_patterncfg_set(uint8_t bbh_id, uint8_t viq_index, uint8_t location, bdmf_boolean no_drop_en, bdmf_boolean viqclassen, uint16_t viq)
{
    uint32_t reg_general_configuration_patterncfg = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (viq_index >= 8) ||
       (location >= _5BITS_MAX_VAL_) ||
       (no_drop_en >= _1BITS_MAX_VAL_) ||
       (viqclassen >= _1BITS_MAX_VAL_) ||
       (viq >= _9BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_configuration_patterncfg = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PATTERNCFG, LOCATION, reg_general_configuration_patterncfg, location);
    reg_general_configuration_patterncfg = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PATTERNCFG, NO_DROP_EN, reg_general_configuration_patterncfg, no_drop_en);
    reg_general_configuration_patterncfg = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PATTERNCFG, VIQCLASSEN, reg_general_configuration_patterncfg, viqclassen);
    reg_general_configuration_patterncfg = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PATTERNCFG, VIQ, reg_general_configuration_patterncfg, viq);

    RU_REG_RAM_WRITE(bbh_id, viq_index, BBH_RX, GENERAL_CONFIGURATION_PATTERNCFG, reg_general_configuration_patterncfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_patterncfg_get(uint8_t bbh_id, uint8_t viq_index, uint8_t *location, bdmf_boolean *no_drop_en, bdmf_boolean *viqclassen, uint16_t *viq)
{
    uint32_t reg_general_configuration_patterncfg;

#ifdef VALIDATE_PARMS
    if (!location || !no_drop_en || !viqclassen || !viq)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (viq_index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, viq_index, BBH_RX, GENERAL_CONFIGURATION_PATTERNCFG, reg_general_configuration_patterncfg);

    *location = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PATTERNCFG, LOCATION, reg_general_configuration_patterncfg);
    *no_drop_en = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PATTERNCFG, NO_DROP_EN, reg_general_configuration_patterncfg);
    *viqclassen = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PATTERNCFG, VIQCLASSEN, reg_general_configuration_patterncfg);
    *viq = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PATTERNCFG, VIQ, reg_general_configuration_patterncfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_patterndata_set(uint8_t bbh_id, uint8_t zero, const bbh_rx_general_configuration_patterndata *general_configuration_patterndata)
{
#ifdef VALIDATE_PARMS
    if(!general_configuration_patterndata)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (zero >= 3))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_WRITE(bbh_id, zero * 2 + 0, BBH_RX, GENERAL_CONFIGURATION_PATTERNDATA, general_configuration_patterndata->pattern_data_config[0]);
    RU_REG_RAM_WRITE(bbh_id, zero * 2 + 1, BBH_RX, GENERAL_CONFIGURATION_PATTERNDATA, general_configuration_patterndata->pattern_data_config[1]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_patterndata_get(uint8_t bbh_id, uint8_t zero, bbh_rx_general_configuration_patterndata *general_configuration_patterndata)
{
#ifdef VALIDATE_PARMS
    if (!general_configuration_patterndata)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (zero >= 3))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, zero * 2 + 0, BBH_RX, GENERAL_CONFIGURATION_PATTERNDATA, general_configuration_patterndata->pattern_data_config[0]);
    RU_REG_RAM_READ(bbh_id, zero * 2 + 1, BBH_RX, GENERAL_CONFIGURATION_PATTERNDATA, general_configuration_patterndata->pattern_data_config[1]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_patternmask_set(uint8_t bbh_id, uint8_t index, uint32_t patternmask)
{
    uint32_t reg_general_configuration_patternmask = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 6))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_configuration_patternmask = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PATTERNMASK, PATTERNMASK, reg_general_configuration_patternmask, patternmask);

    RU_REG_RAM_WRITE(bbh_id, index, BBH_RX, GENERAL_CONFIGURATION_PATTERNMASK, reg_general_configuration_patternmask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_patternmask_get(uint8_t bbh_id, uint8_t index, uint32_t *patternmask)
{
    uint32_t reg_general_configuration_patternmask;

#ifdef VALIDATE_PARMS
    if (!patternmask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 6))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, BBH_RX, GENERAL_CONFIGURATION_PATTERNMASK, reg_general_configuration_patternmask);

    *patternmask = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PATTERNMASK, PATTERNMASK, reg_general_configuration_patternmask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_patterndata8b_set(uint8_t bbh_id, uint8_t viq_index, uint32_t patterndata)
{
    uint32_t reg_general_configuration_patterndata8b = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (viq_index >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_configuration_patterndata8b = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PATTERNDATA8B, PATTERNDATA, reg_general_configuration_patterndata8b, patterndata);

    RU_REG_RAM_WRITE(bbh_id, viq_index, BBH_RX, GENERAL_CONFIGURATION_PATTERNDATA8B, reg_general_configuration_patterndata8b);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_patterndata8b_get(uint8_t bbh_id, uint8_t viq_index, uint32_t *patterndata)
{
    uint32_t reg_general_configuration_patterndata8b;

#ifdef VALIDATE_PARMS
    if (!patterndata)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (viq_index >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, viq_index, BBH_RX, GENERAL_CONFIGURATION_PATTERNDATA8B, reg_general_configuration_patterndata8b);

    *patterndata = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PATTERNDATA8B, PATTERNDATA, reg_general_configuration_patterndata8b);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_patternmask8b_set(uint8_t bbh_id, uint8_t viq_index, uint32_t patternmask)
{
    uint32_t reg_general_configuration_patternmask8b = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (viq_index >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_configuration_patternmask8b = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PATTERNMASK8B, PATTERNMASK, reg_general_configuration_patternmask8b, patternmask);

    RU_REG_RAM_WRITE(bbh_id, viq_index, BBH_RX, GENERAL_CONFIGURATION_PATTERNMASK8B, reg_general_configuration_patternmask8b);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_patternmask8b_get(uint8_t bbh_id, uint8_t viq_index, uint32_t *patternmask)
{
    uint32_t reg_general_configuration_patternmask8b;

#ifdef VALIDATE_PARMS
    if (!patternmask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (viq_index >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, viq_index, BBH_RX, GENERAL_CONFIGURATION_PATTERNMASK8B, reg_general_configuration_patternmask8b);

    *patternmask = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PATTERNMASK8B, PATTERNMASK, reg_general_configuration_patternmask8b);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_patternen_set(uint8_t bbh_id, uint8_t viq_index, uint8_t patternen)
{
    uint32_t reg_general_configuration_patternen = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (viq_index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_configuration_patternen = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PATTERNEN, PATTERNEN, reg_general_configuration_patternen, patternen);

    RU_REG_RAM_WRITE(bbh_id, viq_index, BBH_RX, GENERAL_CONFIGURATION_PATTERNEN, reg_general_configuration_patternen);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_patternen_get(uint8_t bbh_id, uint8_t viq_index, uint8_t *patternen)
{
    uint32_t reg_general_configuration_patternen;

#ifdef VALIDATE_PARMS
    if (!patternen)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (viq_index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, viq_index, BBH_RX, GENERAL_CONFIGURATION_PATTERNEN, reg_general_configuration_patternen);

    *patternen = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PATTERNEN, PATTERNEN, reg_general_configuration_patternen);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_viqfcen_set(uint8_t bbh_id, uint8_t viq_index, uint16_t viqfcen)
{
    uint32_t reg_general_configuration_viqfcen = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (viq_index >= 8) ||
       (viqfcen >= _9BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_configuration_viqfcen = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_VIQFCEN, VIQFCEN, reg_general_configuration_viqfcen, viqfcen);

    RU_REG_RAM_WRITE(bbh_id, viq_index, BBH_RX, GENERAL_CONFIGURATION_VIQFCEN, reg_general_configuration_viqfcen);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_viqfcen_get(uint8_t bbh_id, uint8_t viq_index, uint16_t *viqfcen)
{
    uint32_t reg_general_configuration_viqfcen;

#ifdef VALIDATE_PARMS
    if (!viqfcen)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (viq_index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, viq_index, BBH_RX, GENERAL_CONFIGURATION_VIQFCEN, reg_general_configuration_viqfcen);

    *viqfcen = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_VIQFCEN, VIQFCEN, reg_general_configuration_viqfcen);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_noneth_set(uint8_t bbh_id, uint8_t viq_index, uint16_t flowid, bdmf_boolean enable)
{
    uint32_t reg_general_configuration_noneth = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (viq_index >= 8) ||
       (flowid >= _13BITS_MAX_VAL_) ||
       (enable >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_configuration_noneth = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_NONETH, FLOWID, reg_general_configuration_noneth, flowid);
    reg_general_configuration_noneth = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_NONETH, ENABLE, reg_general_configuration_noneth, enable);

    RU_REG_RAM_WRITE(bbh_id, viq_index, BBH_RX, GENERAL_CONFIGURATION_NONETH, reg_general_configuration_noneth);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_noneth_get(uint8_t bbh_id, uint8_t viq_index, uint16_t *flowid, bdmf_boolean *enable)
{
    uint32_t reg_general_configuration_noneth;

#ifdef VALIDATE_PARMS
    if (!flowid || !enable)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (viq_index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, viq_index, BBH_RX, GENERAL_CONFIGURATION_NONETH, reg_general_configuration_noneth);

    *flowid = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_NONETH, FLOWID, reg_general_configuration_noneth);
    *enable = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_NONETH, ENABLE, reg_general_configuration_noneth);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_pm_counters_inpkt_get(uint8_t bbh_id, uint32_t zero, bbh_rx_pm_counters_inpkt *pm_counters_inpkt)
{
#ifdef VALIDATE_PARMS
    if (!pm_counters_inpkt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (zero >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, zero * 2 + 0, BBH_RX, PM_COUNTERS_INPKT, pm_counters_inpkt->pm_counters_inpkt[0]);
    RU_REG_RAM_READ(bbh_id, zero * 2 + 1, BBH_RX, PM_COUNTERS_INPKT, pm_counters_inpkt->pm_counters_inpkt[1]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_pm_counters_inbyte_get(uint8_t bbh_id, uint32_t zero, bbh_rx_pm_counters_inbyte *pm_counters_inbyte)
{
#ifdef VALIDATE_PARMS
    if (!pm_counters_inbyte)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (zero >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, zero * 2 + 0, BBH_RX, PM_COUNTERS_INBYTE, pm_counters_inbyte->pm_counters_inbyte[0]);
    RU_REG_RAM_READ(bbh_id, zero * 2 + 1, BBH_RX, PM_COUNTERS_INBYTE, pm_counters_inbyte->pm_counters_inbyte[1]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_pm_counters_tooshort_get(uint8_t bbh_id, uint32_t zero, bbh_rx_pm_counters_tooshort *pm_counters_tooshort)
{
#ifdef VALIDATE_PARMS
    if (!pm_counters_tooshort)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (zero >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, zero * 2 + 0, BBH_RX, PM_COUNTERS_TOOSHORT, pm_counters_tooshort->pm_counters_too_short[0]);
    RU_REG_RAM_READ(bbh_id, zero * 2 + 1, BBH_RX, PM_COUNTERS_TOOSHORT, pm_counters_tooshort->pm_counters_too_short[1]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_pm_counters_toolong_get(uint8_t bbh_id, uint32_t zero, bbh_rx_pm_counters_toolong *pm_counters_toolong)
{
#ifdef VALIDATE_PARMS
    if (!pm_counters_toolong)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (zero >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, zero * 2 + 0, BBH_RX, PM_COUNTERS_TOOLONG, pm_counters_toolong->pm_counters_too_long[0]);
    RU_REG_RAM_READ(bbh_id, zero * 2 + 1, BBH_RX, PM_COUNTERS_TOOLONG, pm_counters_toolong->pm_counters_too_long[1]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_pm_counters_crcerror_get(uint8_t bbh_id, uint32_t zero, bbh_rx_pm_counters_crcerror *pm_counters_crcerror)
{
#ifdef VALIDATE_PARMS
    if (!pm_counters_crcerror)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (zero >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, zero * 2 + 0, BBH_RX, PM_COUNTERS_CRCERROR, pm_counters_crcerror->pm_counters_crc_error[0]);
    RU_REG_RAM_READ(bbh_id, zero * 2 + 1, BBH_RX, PM_COUNTERS_CRCERROR, pm_counters_crcerror->pm_counters_crc_error[1]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_pm_counters_fcdrop_get(uint8_t bbh_id, uint32_t zero, bbh_rx_pm_counters_fcdrop *pm_counters_fcdrop)
{
#ifdef VALIDATE_PARMS
    if (!pm_counters_fcdrop)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (zero >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, zero * 2 + 0, BBH_RX, PM_COUNTERS_FCDROP, pm_counters_fcdrop->pm_counters_fc_drop[0]);
    RU_REG_RAM_READ(bbh_id, zero * 2 + 1, BBH_RX, PM_COUNTERS_FCDROP, pm_counters_fcdrop->pm_counters_fc_drop[1]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_pm_counters_truncate_get(uint8_t bbh_id, uint32_t zero, bbh_rx_pm_counters_truncate *pm_counters_truncate)
{
#ifdef VALIDATE_PARMS
    if (!pm_counters_truncate)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (zero >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, zero * 2 + 0, BBH_RX, PM_COUNTERS_TRUNCATE, pm_counters_truncate->pm_counters_truncate[0]);
    RU_REG_RAM_READ(bbh_id, zero * 2 + 1, BBH_RX, PM_COUNTERS_TRUNCATE, pm_counters_truncate->pm_counters_truncate[1]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_pm_counters_totaldrop_get(uint8_t bbh_id, uint32_t zero, bbh_rx_pm_counters_totaldrop *pm_counters_totaldrop)
{
#ifdef VALIDATE_PARMS
    if (!pm_counters_totaldrop)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (zero >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, zero * 2 + 0, BBH_RX, PM_COUNTERS_TOTALDROP, pm_counters_totaldrop->pm_counters_total_drop[0]);
    RU_REG_RAM_READ(bbh_id, zero * 2 + 1, BBH_RX, PM_COUNTERS_TOTALDROP, pm_counters_totaldrop->pm_counters_total_drop[1]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_pm_counters_dummypdcnt_get(uint8_t bbh_id, uint32_t *pmvalue)
{
    uint32_t reg_pm_counters_dummypdcnt;

#ifdef VALIDATE_PARMS
    if (!pmvalue)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, PM_COUNTERS_DUMMYPDCNT, reg_pm_counters_dummypdcnt);

    *pmvalue = RU_FIELD_GET(bbh_id, BBH_RX, PM_COUNTERS_DUMMYPDCNT, PMVALUE, reg_pm_counters_dummypdcnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_pon_rmon_counters_r64_get(uint8_t bbh_id, uint32_t *r64)
{
    uint32_t reg_pon_rmon_counters_r64;

#ifdef VALIDATE_PARMS
    if (!r64)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, PON_RMON_COUNTERS_R64, reg_pon_rmon_counters_r64);

    *r64 = RU_FIELD_GET(bbh_id, BBH_RX, PON_RMON_COUNTERS_R64, R64, reg_pon_rmon_counters_r64);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_pon_rmon_counters_r127_get(uint8_t bbh_id, uint32_t *r127)
{
    uint32_t reg_pon_rmon_counters_r127;

#ifdef VALIDATE_PARMS
    if (!r127)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, PON_RMON_COUNTERS_R127, reg_pon_rmon_counters_r127);

    *r127 = RU_FIELD_GET(bbh_id, BBH_RX, PON_RMON_COUNTERS_R127, R127, reg_pon_rmon_counters_r127);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_pon_rmon_counters_r255_get(uint8_t bbh_id, uint32_t *r255)
{
    uint32_t reg_pon_rmon_counters_r255;

#ifdef VALIDATE_PARMS
    if (!r255)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, PON_RMON_COUNTERS_R255, reg_pon_rmon_counters_r255);

    *r255 = RU_FIELD_GET(bbh_id, BBH_RX, PON_RMON_COUNTERS_R255, R255, reg_pon_rmon_counters_r255);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_pon_rmon_counters_r511_get(uint8_t bbh_id, uint32_t *r511)
{
    uint32_t reg_pon_rmon_counters_r511;

#ifdef VALIDATE_PARMS
    if (!r511)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, PON_RMON_COUNTERS_R511, reg_pon_rmon_counters_r511);

    *r511 = RU_FIELD_GET(bbh_id, BBH_RX, PON_RMON_COUNTERS_R511, R511, reg_pon_rmon_counters_r511);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_pon_rmon_counters_r1023_get(uint8_t bbh_id, uint32_t *r1023)
{
    uint32_t reg_pon_rmon_counters_r1023;

#ifdef VALIDATE_PARMS
    if (!r1023)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, PON_RMON_COUNTERS_R1023, reg_pon_rmon_counters_r1023);

    *r1023 = RU_FIELD_GET(bbh_id, BBH_RX, PON_RMON_COUNTERS_R1023, R1023, reg_pon_rmon_counters_r1023);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_pon_rmon_counters_r1518_get(uint8_t bbh_id, uint32_t *r1518)
{
    uint32_t reg_pon_rmon_counters_r1518;

#ifdef VALIDATE_PARMS
    if (!r1518)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, PON_RMON_COUNTERS_R1518, reg_pon_rmon_counters_r1518);

    *r1518 = RU_FIELD_GET(bbh_id, BBH_RX, PON_RMON_COUNTERS_R1518, R1518, reg_pon_rmon_counters_r1518);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_pon_rmon_counters_r2047_get(uint8_t bbh_id, uint32_t *r1518)
{
    uint32_t reg_pon_rmon_counters_r2047;

#ifdef VALIDATE_PARMS
    if (!r1518)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, PON_RMON_COUNTERS_R2047, reg_pon_rmon_counters_r2047);

    *r1518 = RU_FIELD_GET(bbh_id, BBH_RX, PON_RMON_COUNTERS_R2047, R1518, reg_pon_rmon_counters_r2047);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_pon_rmon_counters_r4095_get(uint8_t bbh_id, uint32_t *r4095)
{
    uint32_t reg_pon_rmon_counters_r4095;

#ifdef VALIDATE_PARMS
    if (!r4095)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, PON_RMON_COUNTERS_R4095, reg_pon_rmon_counters_r4095);

    *r4095 = RU_FIELD_GET(bbh_id, BBH_RX, PON_RMON_COUNTERS_R4095, R4095, reg_pon_rmon_counters_r4095);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_pon_rmon_counters_r9216_get(uint8_t bbh_id, uint32_t *r9216)
{
    uint32_t reg_pon_rmon_counters_r9216;

#ifdef VALIDATE_PARMS
    if (!r9216)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, PON_RMON_COUNTERS_R9216, reg_pon_rmon_counters_r9216);

    *r9216 = RU_FIELD_GET(bbh_id, BBH_RX, PON_RMON_COUNTERS_R9216, R9216, reg_pon_rmon_counters_r9216);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_pon_rmon_counters_r16383_get(uint8_t bbh_id, uint32_t *r16383)
{
    uint32_t reg_pon_rmon_counters_r16383;

#ifdef VALIDATE_PARMS
    if (!r16383)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, PON_RMON_COUNTERS_R16383, reg_pon_rmon_counters_r16383);

    *r16383 = RU_FIELD_GET(bbh_id, BBH_RX, PON_RMON_COUNTERS_R16383, R16383, reg_pon_rmon_counters_r16383);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_debug_pfbuw_get(uint8_t bbh_id, uint8_t zero, bbh_rx_debug_pfbuw *debug_pfbuw)
{
#ifdef VALIDATE_PARMS
    if (!debug_pfbuw)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (zero >= 1))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, zero * 8 + 0, BBH_RX, DEBUG_PFBUW, debug_pfbuw->debug_pfbuw[0]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 1, BBH_RX, DEBUG_PFBUW, debug_pfbuw->debug_pfbuw[1]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 2, BBH_RX, DEBUG_PFBUW, debug_pfbuw->debug_pfbuw[2]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 3, BBH_RX, DEBUG_PFBUW, debug_pfbuw->debug_pfbuw[3]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 4, BBH_RX, DEBUG_PFBUW, debug_pfbuw->debug_pfbuw[4]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 5, BBH_RX, DEBUG_PFBUW, debug_pfbuw->debug_pfbuw[5]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 6, BBH_RX, DEBUG_PFBUW, debug_pfbuw->debug_pfbuw[6]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 7, BBH_RX, DEBUG_PFBUW, debug_pfbuw->debug_pfbuw[7]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_debug_patternuw_get(uint8_t bbh_id, uint8_t zero, bbh_rx_debug_patternuw *debug_patternuw)
{
#ifdef VALIDATE_PARMS
    if (!debug_patternuw)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (zero >= 1))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, zero * 8 + 0, BBH_RX, DEBUG_PATTERNUW, debug_patternuw->debug_patternuw[0]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 1, BBH_RX, DEBUG_PATTERNUW, debug_patternuw->debug_patternuw[1]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 2, BBH_RX, DEBUG_PATTERNUW, debug_patternuw->debug_patternuw[2]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 3, BBH_RX, DEBUG_PATTERNUW, debug_patternuw->debug_patternuw[3]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 4, BBH_RX, DEBUG_PATTERNUW, debug_patternuw->debug_patternuw[4]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 5, BBH_RX, DEBUG_PATTERNUW, debug_patternuw->debug_patternuw[5]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 6, BBH_RX, DEBUG_PATTERNUW, debug_patternuw->debug_patternuw[6]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 7, BBH_RX, DEBUG_PATTERNUW, debug_patternuw->debug_patternuw[7]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_debug_excuw_get(uint8_t bbh_id, uint8_t zero, bbh_rx_debug_excuw *debug_excuw)
{
#ifdef VALIDATE_PARMS
    if (!debug_excuw)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (zero >= 1))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, zero * 8 + 0, BBH_RX, DEBUG_EXCUW, debug_excuw->debug_excuw[0]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 1, BBH_RX, DEBUG_EXCUW, debug_excuw->debug_excuw[1]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 2, BBH_RX, DEBUG_EXCUW, debug_excuw->debug_excuw[2]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 3, BBH_RX, DEBUG_EXCUW, debug_excuw->debug_excuw[3]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 4, BBH_RX, DEBUG_EXCUW, debug_excuw->debug_excuw[4]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 5, BBH_RX, DEBUG_EXCUW, debug_excuw->debug_excuw[5]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 6, BBH_RX, DEBUG_EXCUW, debug_excuw->debug_excuw[6]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 7, BBH_RX, DEBUG_EXCUW, debug_excuw->debug_excuw[7]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_debug_portuw_get(uint8_t bbh_id, uint8_t zero, bbh_rx_debug_portuw *debug_portuw)
{
#ifdef VALIDATE_PARMS
    if (!debug_portuw)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (zero >= 1))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, zero * 8 + 0, BBH_RX, DEBUG_PORTUW, debug_portuw->debug_portuw[0]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 1, BBH_RX, DEBUG_PORTUW, debug_portuw->debug_portuw[1]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 2, BBH_RX, DEBUG_PORTUW, debug_portuw->debug_portuw[2]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 3, BBH_RX, DEBUG_PORTUW, debug_portuw->debug_portuw[3]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 4, BBH_RX, DEBUG_PORTUW, debug_portuw->debug_portuw[4]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 5, BBH_RX, DEBUG_PORTUW, debug_portuw->debug_portuw[5]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 6, BBH_RX, DEBUG_PORTUW, debug_portuw->debug_portuw[6]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 7, BBH_RX, DEBUG_PORTUW, debug_portuw->debug_portuw[7]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_debug_firstsbn_get(uint8_t bbh_id, uint8_t zero, bbh_rx_debug_firstsbn *debug_firstsbn)
{
#ifdef VALIDATE_PARMS
    if (!debug_firstsbn)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (zero >= 1))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, zero * 8 + 0, BBH_RX, DEBUG_FIRSTSBN, debug_firstsbn->debug_first_sbn[0]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 1, BBH_RX, DEBUG_FIRSTSBN, debug_firstsbn->debug_first_sbn[1]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 2, BBH_RX, DEBUG_FIRSTSBN, debug_firstsbn->debug_first_sbn[2]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 3, BBH_RX, DEBUG_FIRSTSBN, debug_firstsbn->debug_first_sbn[3]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 4, BBH_RX, DEBUG_FIRSTSBN, debug_firstsbn->debug_first_sbn[4]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 5, BBH_RX, DEBUG_FIRSTSBN, debug_firstsbn->debug_first_sbn[5]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 6, BBH_RX, DEBUG_FIRSTSBN, debug_firstsbn->debug_first_sbn[6]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 7, BBH_RX, DEBUG_FIRSTSBN, debug_firstsbn->debug_first_sbn[7]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_debug_currentsbn_get(uint8_t bbh_id, uint8_t zero, bbh_rx_debug_currentsbn *debug_currentsbn)
{
#ifdef VALIDATE_PARMS
    if (!debug_currentsbn)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (zero >= 1))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, zero * 8 + 0, BBH_RX, DEBUG_CURRENTSBN, debug_currentsbn->debug_current_sbn[0]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 1, BBH_RX, DEBUG_CURRENTSBN, debug_currentsbn->debug_current_sbn[1]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 2, BBH_RX, DEBUG_CURRENTSBN, debug_currentsbn->debug_current_sbn[2]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 3, BBH_RX, DEBUG_CURRENTSBN, debug_currentsbn->debug_current_sbn[3]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 4, BBH_RX, DEBUG_CURRENTSBN, debug_currentsbn->debug_current_sbn[4]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 5, BBH_RX, DEBUG_CURRENTSBN, debug_currentsbn->debug_current_sbn[5]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 6, BBH_RX, DEBUG_CURRENTSBN, debug_currentsbn->debug_current_sbn[6]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 7, BBH_RX, DEBUG_CURRENTSBN, debug_currentsbn->debug_current_sbn[7]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_debug_sn_get(uint8_t bbh_id, uint8_t zero, bbh_rx_debug_sn *debug_sn)
{
#ifdef VALIDATE_PARMS
    if (!debug_sn)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (zero >= 1))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, zero * 8 + 0, BBH_RX, DEBUG_SN, debug_sn->debug_sn[0]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 1, BBH_RX, DEBUG_SN, debug_sn->debug_sn[1]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 2, BBH_RX, DEBUG_SN, debug_sn->debug_sn[2]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 3, BBH_RX, DEBUG_SN, debug_sn->debug_sn[3]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 4, BBH_RX, DEBUG_SN, debug_sn->debug_sn[4]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 5, BBH_RX, DEBUG_SN, debug_sn->debug_sn[5]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 6, BBH_RX, DEBUG_SN, debug_sn->debug_sn[6]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 7, BBH_RX, DEBUG_SN, debug_sn->debug_sn[7]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_debug_viqmask_get(uint8_t bbh_id, uint8_t zero, bbh_rx_debug_viqmask *debug_viqmask)
{
#ifdef VALIDATE_PARMS
    if (!debug_viqmask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (zero >= 1))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, zero * 8 + 0, BBH_RX, DEBUG_VIQMASK, debug_viqmask->debug_viq_mask[0]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 1, BBH_RX, DEBUG_VIQMASK, debug_viqmask->debug_viq_mask[1]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 2, BBH_RX, DEBUG_VIQMASK, debug_viqmask->debug_viq_mask[2]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 3, BBH_RX, DEBUG_VIQMASK, debug_viqmask->debug_viq_mask[3]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 4, BBH_RX, DEBUG_VIQMASK, debug_viqmask->debug_viq_mask[4]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 5, BBH_RX, DEBUG_VIQMASK, debug_viqmask->debug_viq_mask[5]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 6, BBH_RX, DEBUG_VIQMASK, debug_viqmask->debug_viq_mask[6]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 7, BBH_RX, DEBUG_VIQMASK, debug_viqmask->debug_viq_mask[7]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_debug_plen_get(uint8_t bbh_id, uint8_t zero, bbh_rx_debug_plen *debug_plen)
{
#ifdef VALIDATE_PARMS
    if (!debug_plen)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (zero >= 1))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, zero * 8 + 0, BBH_RX, DEBUG_PLEN, debug_plen->debug_plen[0]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 1, BBH_RX, DEBUG_PLEN, debug_plen->debug_plen[1]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 2, BBH_RX, DEBUG_PLEN, debug_plen->debug_plen[2]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 3, BBH_RX, DEBUG_PLEN, debug_plen->debug_plen[3]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 4, BBH_RX, DEBUG_PLEN, debug_plen->debug_plen[4]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 5, BBH_RX, DEBUG_PLEN, debug_plen->debug_plen[5]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 6, BBH_RX, DEBUG_PLEN, debug_plen->debug_plen[6]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 7, BBH_RX, DEBUG_PLEN, debug_plen->debug_plen[7]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_debug_eopok_get(uint8_t bbh_id, uint8_t *context)
{
    uint32_t reg_debug_eopok;

#ifdef VALIDATE_PARMS
    if (!context)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, DEBUG_EOPOK, reg_debug_eopok);

    *context = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_EOPOK, CONTEXT, reg_debug_eopok);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_debug_viqfull_get(uint8_t bbh_id, uint8_t *context)
{
    uint32_t reg_debug_viqfull;

#ifdef VALIDATE_PARMS
    if (!context)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, DEBUG_VIQFULL, reg_debug_viqfull);

    *context = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_VIQFULL, CONTEXT, reg_debug_viqfull);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_debug_portreq_get(uint8_t bbh_id, uint8_t *context)
{
    uint32_t reg_debug_portreq;

#ifdef VALIDATE_PARMS
    if (!context)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, DEBUG_PORTREQ, reg_debug_portreq);

    *context = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_PORTREQ, CONTEXT, reg_debug_portreq);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_debug_dummyreq_get(uint8_t bbh_id, uint8_t *context)
{
    uint32_t reg_debug_dummyreq;

#ifdef VALIDATE_PARMS
    if (!context)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, DEBUG_DUMMYREQ, reg_debug_dummyreq);

    *context = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_DUMMYREQ, CONTEXT, reg_debug_dummyreq);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_debug_inreassembly_get(uint8_t bbh_id, uint8_t *context)
{
    uint32_t reg_debug_inreassembly;

#ifdef VALIDATE_PARMS
    if (!context)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, DEBUG_INREASSEMBLY, reg_debug_inreassembly);

    *context = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_INREASSEMBLY, CONTEXT, reg_debug_inreassembly);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_debug_dummypd_get(uint8_t bbh_id, uint8_t *context)
{
    uint32_t reg_debug_dummypd;

#ifdef VALIDATE_PARMS
    if (!context)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, DEBUG_DUMMYPD, reg_debug_dummypd);

    *context = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_DUMMYPD, CONTEXT, reg_debug_dummypd);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_debug_ufuw_get(uint8_t bbh_id, uint8_t *uw)
{
    uint32_t reg_debug_ufuw;

#ifdef VALIDATE_PARMS
    if (!uw)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, DEBUG_UFUW, reg_debug_ufuw);

    *uw = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_UFUW, UW, reg_debug_ufuw);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_debug_pendconnectuw_get(uint8_t bbh_id, uint8_t *uw)
{
    uint32_t reg_debug_pendconnectuw;

#ifdef VALIDATE_PARMS
    if (!uw)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, DEBUG_PENDCONNECTUW, reg_debug_pendconnectuw);

    *uw = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_PENDCONNECTUW, UW, reg_debug_pendconnectuw);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_debug_dbgvec_get(uint8_t bbh_id, uint8_t zero, bbh_rx_debug_dbgvec *debug_dbgvec)
{
#ifdef VALIDATE_PARMS
    if (!debug_dbgvec)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (zero >= 3))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, zero * 8 + 0, BBH_RX, DEBUG_DBGVEC, debug_dbgvec->debug_dbg_veq[0]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 1, BBH_RX, DEBUG_DBGVEC, debug_dbgvec->debug_dbg_veq[1]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 2, BBH_RX, DEBUG_DBGVEC, debug_dbgvec->debug_dbg_veq[2]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 3, BBH_RX, DEBUG_DBGVEC, debug_dbgvec->debug_dbg_veq[3]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 4, BBH_RX, DEBUG_DBGVEC, debug_dbgvec->debug_dbg_veq[4]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 5, BBH_RX, DEBUG_DBGVEC, debug_dbgvec->debug_dbg_veq[5]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 6, BBH_RX, DEBUG_DBGVEC, debug_dbgvec->debug_dbg_veq[6]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 7, BBH_RX, DEBUG_DBGVEC, debug_dbgvec->debug_dbg_veq[7]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_debug_pduw_get(uint8_t bbh_id, uint8_t zero, bbh_rx_debug_pduw *debug_pduw)
{
#ifdef VALIDATE_PARMS
    if (!debug_pduw)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (zero >= 1))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, zero * 8 + 0, BBH_RX, DEBUG_PDUW, debug_pduw->debug_pduw[0]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 1, BBH_RX, DEBUG_PDUW, debug_pduw->debug_pduw[1]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 2, BBH_RX, DEBUG_PDUW, debug_pduw->debug_pduw[2]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 3, BBH_RX, DEBUG_PDUW, debug_pduw->debug_pduw[3]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 4, BBH_RX, DEBUG_PDUW, debug_pduw->debug_pduw[4]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 5, BBH_RX, DEBUG_PDUW, debug_pduw->debug_pduw[5]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 6, BBH_RX, DEBUG_PDUW, debug_pduw->debug_pduw[6]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 7, BBH_RX, DEBUG_PDUW, debug_pduw->debug_pduw[7]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_debug_sbpmack_get(uint8_t bbh_id, uint8_t zero, bbh_rx_debug_sbpmack *debug_sbpmack)
{
#ifdef VALIDATE_PARMS
    if (!debug_sbpmack)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (zero >= 1))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, zero * 8 + 0, BBH_RX, DEBUG_SBPMACK, debug_sbpmack->debug_sbpm_ack[0]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 1, BBH_RX, DEBUG_SBPMACK, debug_sbpmack->debug_sbpm_ack[1]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 2, BBH_RX, DEBUG_SBPMACK, debug_sbpmack->debug_sbpm_ack[2]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 3, BBH_RX, DEBUG_SBPMACK, debug_sbpmack->debug_sbpm_ack[3]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 4, BBH_RX, DEBUG_SBPMACK, debug_sbpmack->debug_sbpm_ack[4]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 5, BBH_RX, DEBUG_SBPMACK, debug_sbpmack->debug_sbpm_ack[5]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 6, BBH_RX, DEBUG_SBPMACK, debug_sbpmack->debug_sbpm_ack[6]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 7, BBH_RX, DEBUG_SBPMACK, debug_sbpmack->debug_sbpm_ack[7]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_debug_writereplyack_get(uint8_t bbh_id, uint8_t zero, bbh_rx_debug_writereplyack *debug_writereplyack)
{
#ifdef VALIDATE_PARMS
    if (!debug_writereplyack)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (zero >= 1))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, zero * 8 + 0, BBH_RX, DEBUG_WRITEREPLYACK, debug_writereplyack->debug_write_replay_ack[0]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 1, BBH_RX, DEBUG_WRITEREPLYACK, debug_writereplyack->debug_write_replay_ack[1]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 2, BBH_RX, DEBUG_WRITEREPLYACK, debug_writereplyack->debug_write_replay_ack[2]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 3, BBH_RX, DEBUG_WRITEREPLYACK, debug_writereplyack->debug_write_replay_ack[3]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 4, BBH_RX, DEBUG_WRITEREPLYACK, debug_writereplyack->debug_write_replay_ack[4]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 5, BBH_RX, DEBUG_WRITEREPLYACK, debug_writereplyack->debug_write_replay_ack[5]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 6, BBH_RX, DEBUG_WRITEREPLYACK, debug_writereplyack->debug_write_replay_ack[6]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 7, BBH_RX, DEBUG_WRITEREPLYACK, debug_writereplyack->debug_write_replay_ack[7]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_debug_dispcreditack_get(uint8_t bbh_id, uint8_t zero, bbh_rx_debug_dispcreditack *debug_dispcreditack)
{
#ifdef VALIDATE_PARMS
    if (!debug_dispcreditack)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (zero >= 1))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, zero * 8 + 0, BBH_RX, DEBUG_DISPCREDITACK, debug_dispcreditack->debug_disp_credit_ack[0]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 1, BBH_RX, DEBUG_DISPCREDITACK, debug_dispcreditack->debug_disp_credit_ack[1]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 2, BBH_RX, DEBUG_DISPCREDITACK, debug_dispcreditack->debug_disp_credit_ack[2]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 3, BBH_RX, DEBUG_DISPCREDITACK, debug_dispcreditack->debug_disp_credit_ack[3]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 4, BBH_RX, DEBUG_DISPCREDITACK, debug_dispcreditack->debug_disp_credit_ack[4]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 5, BBH_RX, DEBUG_DISPCREDITACK, debug_dispcreditack->debug_disp_credit_ack[5]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 6, BBH_RX, DEBUG_DISPCREDITACK, debug_dispcreditack->debug_disp_credit_ack[6]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 7, BBH_RX, DEBUG_DISPCREDITACK, debug_dispcreditack->debug_disp_credit_ack[7]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_debug_pdrddata_get(uint8_t bbh_id, uint8_t zero, bbh_rx_debug_pdrddata *debug_pdrddata)
{
#ifdef VALIDATE_PARMS
    if (!debug_pdrddata)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (zero >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, zero * 8 + 0, BBH_RX, DEBUG_PDRDDATA, debug_pdrddata->debug_pdrd_data[0]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 1, BBH_RX, DEBUG_PDRDDATA, debug_pdrddata->debug_pdrd_data[1]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 2, BBH_RX, DEBUG_PDRDDATA, debug_pdrddata->debug_pdrd_data[2]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 3, BBH_RX, DEBUG_PDRDDATA, debug_pdrddata->debug_pdrd_data[3]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 4, BBH_RX, DEBUG_PDRDDATA, debug_pdrddata->debug_pdrd_data[4]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 5, BBH_RX, DEBUG_PDRDDATA, debug_pdrddata->debug_pdrd_data[5]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 6, BBH_RX, DEBUG_PDRDDATA, debug_pdrddata->debug_pdrd_data[6]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 7, BBH_RX, DEBUG_PDRDDATA, debug_pdrddata->debug_pdrd_data[7]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_debug_sbnfifo_get(uint8_t bbh_id, uint8_t zero, bbh_rx_debug_sbnfifo *debug_sbnfifo)
{
#ifdef VALIDATE_PARMS
    if (!debug_sbnfifo)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (zero >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, zero * 8 + 0, BBH_RX, DEBUG_SBNFIFO, debug_sbnfifo->sbn_fifo[0]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 1, BBH_RX, DEBUG_SBNFIFO, debug_sbnfifo->sbn_fifo[1]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 2, BBH_RX, DEBUG_SBNFIFO, debug_sbnfifo->sbn_fifo[2]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 3, BBH_RX, DEBUG_SBNFIFO, debug_sbnfifo->sbn_fifo[3]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 4, BBH_RX, DEBUG_SBNFIFO, debug_sbnfifo->sbn_fifo[4]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 5, BBH_RX, DEBUG_SBNFIFO, debug_sbnfifo->sbn_fifo[5]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 6, BBH_RX, DEBUG_SBNFIFO, debug_sbnfifo->sbn_fifo[6]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 7, BBH_RX, DEBUG_SBNFIFO, debug_sbnfifo->sbn_fifo[7]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_debug_sbnprefetch_get(uint8_t bbh_id, uint16_t *oddbnentry, bdmf_boolean *oddvalid, uint16_t *evenbnentry, bdmf_boolean *evenvalid)
{
    uint32_t reg_debug_sbnprefetch;

#ifdef VALIDATE_PARMS
    if (!oddbnentry || !oddvalid || !evenbnentry || !evenvalid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, DEBUG_SBNPREFETCH, reg_debug_sbnprefetch);

    *oddbnentry = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_SBNPREFETCH, ODDBNENTRY, reg_debug_sbnprefetch);
    *oddvalid = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_SBNPREFETCH, ODDVALID, reg_debug_sbnprefetch);
    *evenbnentry = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_SBNPREFETCH, EVENBNENTRY, reg_debug_sbnprefetch);
    *evenvalid = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_SBNPREFETCH, EVENVALID, reg_debug_sbnprefetch);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_debug_excfifo_get(uint8_t bbh_id, uint8_t zero, bbh_rx_debug_excfifo *debug_excfifo)
{
#ifdef VALIDATE_PARMS
    if (!debug_excfifo)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (zero >= 16))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, zero * 8 + 0, BBH_RX, DEBUG_EXCFIFO, debug_excfifo->debug_exc_fifo[0]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 1, BBH_RX, DEBUG_EXCFIFO, debug_excfifo->debug_exc_fifo[1]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 2, BBH_RX, DEBUG_EXCFIFO, debug_excfifo->debug_exc_fifo[2]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 3, BBH_RX, DEBUG_EXCFIFO, debug_excfifo->debug_exc_fifo[3]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 4, BBH_RX, DEBUG_EXCFIFO, debug_excfifo->debug_exc_fifo[4]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 5, BBH_RX, DEBUG_EXCFIFO, debug_excfifo->debug_exc_fifo[5]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 6, BBH_RX, DEBUG_EXCFIFO, debug_excfifo->debug_exc_fifo[6]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 7, BBH_RX, DEBUG_EXCFIFO, debug_excfifo->debug_exc_fifo[7]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_bbh_rx_pon_cnpl_debug_gem_out_of_range_get(uint8_t bbh_id, uint32_t *pmvalue)
{
    uint32_t reg_bbh_rx_pon_cnpl_debug_gem_out_of_range;

#ifdef VALIDATE_PARMS
    if (!pmvalue)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, BBH_RX_PON_CNPL_DEBUG_GEM_OUT_OF_RANGE, reg_bbh_rx_pon_cnpl_debug_gem_out_of_range);

    *pmvalue = RU_FIELD_GET(bbh_id, BBH_RX, BBH_RX_PON_CNPL_DEBUG_GEM_OUT_OF_RANGE, PMVALUE, reg_bbh_rx_pon_cnpl_debug_gem_out_of_range);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_bbh_rx_pon_cnpl_debug_gem_out_of_range_flow_get(uint8_t bbh_id, uint16_t *flow)
{
    uint32_t reg_bbh_rx_pon_cnpl_debug_gem_out_of_range_flow;

#ifdef VALIDATE_PARMS
    if (!flow)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, BBH_RX_PON_CNPL_DEBUG_GEM_OUT_OF_RANGE_FLOW, reg_bbh_rx_pon_cnpl_debug_gem_out_of_range_flow);

    *flow = RU_FIELD_GET(bbh_id, BBH_RX, BBH_RX_PON_CNPL_DEBUG_GEM_OUT_OF_RANGE_FLOW, FLOW, reg_bbh_rx_pon_cnpl_debug_gem_out_of_range_flow);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_bbh_rx_pon_cnpl_debug_usedwords_get(uint8_t bbh_id, uint8_t *usedwords)
{
    uint32_t reg_bbh_rx_pon_cnpl_debug_usedwords;

#ifdef VALIDATE_PARMS
    if (!usedwords)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, BBH_RX_PON_CNPL_DEBUG_USEDWORDS, reg_bbh_rx_pon_cnpl_debug_usedwords);

    *usedwords = RU_FIELD_GET(bbh_id, BBH_RX, BBH_RX_PON_CNPL_DEBUG_USEDWORDS, USEDWORDS, reg_bbh_rx_pon_cnpl_debug_usedwords);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_gem_mode_gemmode_set(uint8_t bbh_id, uint8_t zero, const bbh_rx_gem_mode_gemmode *gem_mode_gemmode)
{
#ifdef VALIDATE_PARMS
    if(!gem_mode_gemmode)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (zero >= 256))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_WRITE(bbh_id, zero * 8 + 0, BBH_RX, GEM_MODE_GEMMODE, gem_mode_gemmode->debug_gem_mode[0]);
    RU_REG_RAM_WRITE(bbh_id, zero * 8 + 1, BBH_RX, GEM_MODE_GEMMODE, gem_mode_gemmode->debug_gem_mode[1]);
    RU_REG_RAM_WRITE(bbh_id, zero * 8 + 2, BBH_RX, GEM_MODE_GEMMODE, gem_mode_gemmode->debug_gem_mode[2]);
    RU_REG_RAM_WRITE(bbh_id, zero * 8 + 3, BBH_RX, GEM_MODE_GEMMODE, gem_mode_gemmode->debug_gem_mode[3]);
    RU_REG_RAM_WRITE(bbh_id, zero * 8 + 4, BBH_RX, GEM_MODE_GEMMODE, gem_mode_gemmode->debug_gem_mode[4]);
    RU_REG_RAM_WRITE(bbh_id, zero * 8 + 5, BBH_RX, GEM_MODE_GEMMODE, gem_mode_gemmode->debug_gem_mode[5]);
    RU_REG_RAM_WRITE(bbh_id, zero * 8 + 6, BBH_RX, GEM_MODE_GEMMODE, gem_mode_gemmode->debug_gem_mode[6]);
    RU_REG_RAM_WRITE(bbh_id, zero * 8 + 7, BBH_RX, GEM_MODE_GEMMODE, gem_mode_gemmode->debug_gem_mode[7]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_gem_mode_gemmode_get(uint8_t bbh_id, uint8_t zero, bbh_rx_gem_mode_gemmode *gem_mode_gemmode)
{
#ifdef VALIDATE_PARMS
    if (!gem_mode_gemmode)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (zero >= 256))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, zero * 8 + 0, BBH_RX, GEM_MODE_GEMMODE, gem_mode_gemmode->debug_gem_mode[0]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 1, BBH_RX, GEM_MODE_GEMMODE, gem_mode_gemmode->debug_gem_mode[1]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 2, BBH_RX, GEM_MODE_GEMMODE, gem_mode_gemmode->debug_gem_mode[2]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 3, BBH_RX, GEM_MODE_GEMMODE, gem_mode_gemmode->debug_gem_mode[3]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 4, BBH_RX, GEM_MODE_GEMMODE, gem_mode_gemmode->debug_gem_mode[4]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 5, BBH_RX, GEM_MODE_GEMMODE, gem_mode_gemmode->debug_gem_mode[5]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 6, BBH_RX, GEM_MODE_GEMMODE, gem_mode_gemmode->debug_gem_mode[6]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 7, BBH_RX, GEM_MODE_GEMMODE, gem_mode_gemmode->debug_gem_mode[7]);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL

typedef enum
{
    bdmf_address_general_configuration_bbcfg,
    bdmf_address_general_configuration_enable,
    bdmf_address_general_configuration_minpkt,
    bdmf_address_general_configuration_maxpkt,
    bdmf_address_general_configuration_clk_gate_cntrl,
    bdmf_address_general_configuration_rxdbgsel,
    bdmf_address_general_configuration_rxrstrst,
    bdmf_address_general_configuration_sbpmcfg,
    bdmf_address_general_configuration_psrambase,
    bdmf_address_general_configuration_ingresschannel,
    bdmf_address_general_configuration_bbovrdroute,
    bdmf_address_general_configuration_bbovrdctrl,
    bdmf_address_general_configuration_gemdone,
    bdmf_address_general_configuration_mactype,
    bdmf_address_general_configuration_portth,
    bdmf_address_general_configuration_flowctrl,
    bdmf_address_general_configuration_sopoffset,
    bdmf_address_general_configuration_ingresspid,
    bdmf_address_general_configuration_portweight,
    bdmf_address_general_configuration_portviqmask,
    bdmf_address_general_configuration_excviqmask,
    bdmf_address_general_configuration_dispviq,
    bdmf_address_general_configuration_viqweight,
    bdmf_address_general_configuration_patterncfg,
    bdmf_address_general_configuration_patterndata,
    bdmf_address_general_configuration_patternmask,
    bdmf_address_general_configuration_patterndata8b,
    bdmf_address_general_configuration_patternmask8b,
    bdmf_address_general_configuration_patternen,
    bdmf_address_general_configuration_viqfcen,
    bdmf_address_general_configuration_noneth,
    bdmf_address_pm_counters_inpkt,
    bdmf_address_pm_counters_inbyte,
    bdmf_address_pm_counters_tooshort,
    bdmf_address_pm_counters_toolong,
    bdmf_address_pm_counters_crcerror,
    bdmf_address_pm_counters_fcdrop,
    bdmf_address_pm_counters_truncate,
    bdmf_address_pm_counters_totaldrop,
    bdmf_address_pm_counters_dummypdcnt,
    bdmf_address_pon_rmon_counters_r64,
    bdmf_address_pon_rmon_counters_r127,
    bdmf_address_pon_rmon_counters_r255,
    bdmf_address_pon_rmon_counters_r511,
    bdmf_address_pon_rmon_counters_r1023,
    bdmf_address_pon_rmon_counters_r1518,
    bdmf_address_pon_rmon_counters_r2047,
    bdmf_address_pon_rmon_counters_r4095,
    bdmf_address_pon_rmon_counters_r9216,
    bdmf_address_pon_rmon_counters_r16383,
    bdmf_address_debug_pfbuw,
    bdmf_address_debug_patternuw,
    bdmf_address_debug_excuw,
    bdmf_address_debug_portuw,
    bdmf_address_debug_firstsbn,
    bdmf_address_debug_currentsbn,
    bdmf_address_debug_sn,
    bdmf_address_debug_viqmask,
    bdmf_address_debug_plen,
    bdmf_address_debug_eopok,
    bdmf_address_debug_viqfull,
    bdmf_address_debug_portreq,
    bdmf_address_debug_dummyreq,
    bdmf_address_debug_inreassembly,
    bdmf_address_debug_dummypd,
    bdmf_address_debug_ufuw,
    bdmf_address_debug_pendconnectuw,
    bdmf_address_debug_dbgvec,
    bdmf_address_debug_pduw,
    bdmf_address_debug_sbpmack,
    bdmf_address_debug_writereplyack,
    bdmf_address_debug_dispcreditack,
    bdmf_address_debug_pdrddata,
    bdmf_address_debug_sbnfifo,
    bdmf_address_debug_sbnprefetch,
    bdmf_address_debug_excfifo,
    bdmf_address_bbh_rx_pon_cnpl_debug_gem_out_of_range,
    bdmf_address_bbh_rx_pon_cnpl_debug_gem_out_of_range_flow,
    bdmf_address_bbh_rx_pon_cnpl_debug_usedwords,
    bdmf_address_gem_mode_gemmode,
}
bdmf_address;

static int ag_drv_bbh_rx_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_bbh_rx_timer:
        ag_err = ag_drv_bbh_rx_timer_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bbh_rx_flow_control_force:
        ag_err = ag_drv_bbh_rx_flow_control_force_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bbh_rx_flow_control_runner_enable:
        ag_err = ag_drv_bbh_rx_flow_control_runner_enable_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bbh_rx_flow_control_qm_enable:
        ag_err = ag_drv_bbh_rx_flow_control_qm_enable_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bbh_rx_flow_ctrl_timer:
        ag_err = ag_drv_bbh_rx_flow_ctrl_timer_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bbh_rx_flow_ctrl_force:
        ag_err = ag_drv_bbh_rx_flow_ctrl_force_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bbh_rx_flow_ctrl_rnr_en:
        ag_err = ag_drv_bbh_rx_flow_ctrl_rnr_en_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bbh_rx_general_configuration_bbcfg:
        ag_err = ag_drv_bbh_rx_general_configuration_bbcfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bbh_rx_general_configuration_enable:
        ag_err = ag_drv_bbh_rx_general_configuration_enable_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber);
        break;
    case cli_bbh_rx_general_configuration_minpkt:
        ag_err = ag_drv_bbh_rx_general_configuration_minpkt_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bbh_rx_general_configuration_maxpkt:
        ag_err = ag_drv_bbh_rx_general_configuration_maxpkt_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_bbh_rx_general_configuration_clk_gate_cntrl:
    {
        bbh_rx_general_configuration_clk_gate_cntrl general_configuration_clk_gate_cntrl = { .bypass_clk_gate = parm[2].value.unumber, .timer_val = parm[3].value.unumber, .keep_alive_en = parm[4].value.unumber, .keep_alive_intrvl = parm[5].value.unumber, .keep_alive_cyc = parm[6].value.unumber};
        ag_err = ag_drv_bbh_rx_general_configuration_clk_gate_cntrl_set(parm[1].value.unumber, &general_configuration_clk_gate_cntrl);
        break;
    }
    case cli_bbh_rx_general_configuration_rxdbgsel:
        ag_err = ag_drv_bbh_rx_general_configuration_rxdbgsel_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_bbh_rx_general_configuration_rxrstrst:
    {
        bbh_rx_general_configuration_rxrstrst general_configuration_rxrstrst = { .portrst = parm[2].value.unumber, .psramrst = parm[3].value.unumber, .pdfiforst = parm[4].value.unumber, .sbpmfiforst = parm[5].value.unumber, .dispcreditrst = parm[6].value.unumber, .upldfiforst = parm[7].value.unumber, .pfbch = parm[8].value.unumber, .dispcredit = parm[9].value.unumber, .dispviq = parm[10].value.unumber};
        ag_err = ag_drv_bbh_rx_general_configuration_rxrstrst_set(parm[1].value.unumber, &general_configuration_rxrstrst);
        break;
    }
    case cli_bbh_rx_general_configuration_sbpmcfg:
        ag_err = ag_drv_bbh_rx_general_configuration_sbpmcfg_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_bbh_rx_general_configuration_psrambase:
        ag_err = ag_drv_bbh_rx_general_configuration_psrambase_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_bbh_rx_general_configuration_ingresschannel:
        ag_err = ag_drv_bbh_rx_general_configuration_ingresschannel_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_bbh_rx_general_configuration_bbovrdroute:
        ag_err = ag_drv_bbh_rx_general_configuration_bbovrdroute_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_bbh_rx_general_configuration_bbovrdctrl:
        ag_err = ag_drv_bbh_rx_general_configuration_bbovrdctrl_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bbh_rx_general_configuration_mactype:
        ag_err = ag_drv_bbh_rx_general_configuration_mactype_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bbh_rx_general_configuration_portth:
        ag_err = ag_drv_bbh_rx_general_configuration_portth_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bbh_rx_general_configuration_sopoffset:
        ag_err = ag_drv_bbh_rx_general_configuration_sopoffset_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bbh_rx_general_configuration_ingresspid:
        ag_err = ag_drv_bbh_rx_general_configuration_ingresspid_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bbh_rx_general_configuration_portweight:
        ag_err = ag_drv_bbh_rx_general_configuration_portweight_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bbh_rx_general_configuration_portviqmask:
        ag_err = ag_drv_bbh_rx_general_configuration_portviqmask_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bbh_rx_general_configuration_excviqmask:
        ag_err = ag_drv_bbh_rx_general_configuration_excviqmask_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber);
        break;
    case cli_bbh_rx_general_configuration_dispviq:
        ag_err = ag_drv_bbh_rx_general_configuration_dispviq_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bbh_rx_general_configuration_viqweight:
        ag_err = ag_drv_bbh_rx_general_configuration_viqweight_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bbh_rx_general_configuration_patterncfg:
        ag_err = ag_drv_bbh_rx_general_configuration_patterncfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber, parm[6].value.unumber);
        break;
    case cli_bbh_rx_general_configuration_patterndata:
    {
        bbh_rx_general_configuration_patterndata general_configuration_patterndata = { .pattern_data_config = { parm[3].value.unumber, parm[4].value.unumber}};
        ag_err = ag_drv_bbh_rx_general_configuration_patterndata_set(parm[1].value.unumber, parm[2].value.unumber, &general_configuration_patterndata);
        break;
    }
    case cli_bbh_rx_general_configuration_patternmask:
        ag_err = ag_drv_bbh_rx_general_configuration_patternmask_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bbh_rx_general_configuration_patterndata8b:
        ag_err = ag_drv_bbh_rx_general_configuration_patterndata8b_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bbh_rx_general_configuration_patternmask8b:
        ag_err = ag_drv_bbh_rx_general_configuration_patternmask8b_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bbh_rx_general_configuration_patternen:
        ag_err = ag_drv_bbh_rx_general_configuration_patternen_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bbh_rx_general_configuration_viqfcen:
        ag_err = ag_drv_bbh_rx_general_configuration_viqfcen_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bbh_rx_general_configuration_noneth:
        ag_err = ag_drv_bbh_rx_general_configuration_noneth_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_bbh_rx_gem_mode_gemmode:
    {
        bbh_rx_gem_mode_gemmode gem_mode_gemmode = { .debug_gem_mode = { parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber, parm[6].value.unumber, parm[7].value.unumber, parm[8].value.unumber, parm[9].value.unumber, parm[10].value.unumber}};
        ag_err = ag_drv_bbh_rx_gem_mode_gemmode_set(parm[1].value.unumber, parm[2].value.unumber, &gem_mode_gemmode);
        break;
    }
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

int bcm_bbh_rx_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_bbh_rx_timer:
    {
        uint16_t timer;
        ag_err = ag_drv_bbh_rx_timer_get(parm[1].value.unumber, parm[2].value.unumber, &timer);
        bdmf_session_print(session, "timer = %u = 0x%x\n", timer, timer);
        break;
    }
    case cli_bbh_rx_flow_control_force:
    {
        bdmf_boolean fcforce;
        ag_err = ag_drv_bbh_rx_flow_control_force_get(parm[1].value.unumber, parm[2].value.unumber, &fcforce);
        bdmf_session_print(session, "fcforce = %u = 0x%x\n", fcforce, fcforce);
        break;
    }
    case cli_bbh_rx_flow_control_runner_enable:
    {
        bdmf_boolean fcrnren;
        ag_err = ag_drv_bbh_rx_flow_control_runner_enable_get(parm[1].value.unumber, parm[2].value.unumber, &fcrnren);
        bdmf_session_print(session, "fcrnren = %u = 0x%x\n", fcrnren, fcrnren);
        break;
    }
    case cli_bbh_rx_flow_control_qm_enable:
    {
        bdmf_boolean fcqmen;
        ag_err = ag_drv_bbh_rx_flow_control_qm_enable_get(parm[1].value.unumber, parm[2].value.unumber, &fcqmen);
        bdmf_session_print(session, "fcqmen = %u = 0x%x\n", fcqmen, fcqmen);
        break;
    }
    case cli_bbh_rx_flow_ctrl_timer:
    {
        uint16_t timer;
        ag_err = ag_drv_bbh_rx_flow_ctrl_timer_get(parm[1].value.unumber, parm[2].value.unumber, &timer);
        bdmf_session_print(session, "timer = %u = 0x%x\n", timer, timer);
        break;
    }
    case cli_bbh_rx_flow_ctrl_force:
    {
        bdmf_boolean fcforce;
        ag_err = ag_drv_bbh_rx_flow_ctrl_force_get(parm[1].value.unumber, parm[2].value.unumber, &fcforce);
        bdmf_session_print(session, "fcforce = %u = 0x%x\n", fcforce, fcforce);
        break;
    }
    case cli_bbh_rx_flow_ctrl_rnr_en:
    {
        bdmf_boolean fcrnren;
        ag_err = ag_drv_bbh_rx_flow_ctrl_rnr_en_get(parm[1].value.unumber, parm[2].value.unumber, &fcrnren);
        bdmf_session_print(session, "fcrnren = %u = 0x%x\n", fcrnren, fcrnren);
        break;
    }
    case cli_bbh_rx_general_configuration_bbcfg:
    {
        uint16_t dispbbid;
        uint16_t sbpmbbid;
        ag_err = ag_drv_bbh_rx_general_configuration_bbcfg_get(parm[1].value.unumber, &dispbbid, &sbpmbbid);
        bdmf_session_print(session, "dispbbid = %u = 0x%x\n", dispbbid, dispbbid);
        bdmf_session_print(session, "sbpmbbid = %u = 0x%x\n", sbpmbbid, sbpmbbid);
        break;
    }
    case cli_bbh_rx_general_configuration_enable:
    {
        uint8_t pkten;
        bdmf_boolean sbpmen;
        bdmf_boolean geminit;
        bdmf_boolean grace_dis;
        ag_err = ag_drv_bbh_rx_general_configuration_enable_get(parm[1].value.unumber, &pkten, &sbpmen, &geminit, &grace_dis);
        bdmf_session_print(session, "pkten = %u = 0x%x\n", pkten, pkten);
        bdmf_session_print(session, "sbpmen = %u = 0x%x\n", sbpmen, sbpmen);
        bdmf_session_print(session, "geminit = %u = 0x%x\n", geminit, geminit);
        bdmf_session_print(session, "grace_dis = %u = 0x%x\n", grace_dis, grace_dis);
        break;
    }
    case cli_bbh_rx_general_configuration_minpkt:
    {
        uint8_t minpkt_normal;
        uint8_t minpkt_omci;
        ag_err = ag_drv_bbh_rx_general_configuration_minpkt_get(parm[1].value.unumber, &minpkt_normal, &minpkt_omci);
        bdmf_session_print(session, "minpkt_normal = %u = 0x%x\n", minpkt_normal, minpkt_normal);
        bdmf_session_print(session, "minpkt_omci = %u = 0x%x\n", minpkt_omci, minpkt_omci);
        break;
    }
    case cli_bbh_rx_general_configuration_maxpkt:
    {
        uint16_t maxpkt;
        ag_err = ag_drv_bbh_rx_general_configuration_maxpkt_get(parm[1].value.unumber, &maxpkt);
        bdmf_session_print(session, "maxpkt = %u = 0x%x\n", maxpkt, maxpkt);
        break;
    }
    case cli_bbh_rx_general_configuration_clk_gate_cntrl:
    {
        bbh_rx_general_configuration_clk_gate_cntrl general_configuration_clk_gate_cntrl;
        ag_err = ag_drv_bbh_rx_general_configuration_clk_gate_cntrl_get(parm[1].value.unumber, &general_configuration_clk_gate_cntrl);
        bdmf_session_print(session, "bypass_clk_gate = %u = 0x%x\n", general_configuration_clk_gate_cntrl.bypass_clk_gate, general_configuration_clk_gate_cntrl.bypass_clk_gate);
        bdmf_session_print(session, "timer_val = %u = 0x%x\n", general_configuration_clk_gate_cntrl.timer_val, general_configuration_clk_gate_cntrl.timer_val);
        bdmf_session_print(session, "keep_alive_en = %u = 0x%x\n", general_configuration_clk_gate_cntrl.keep_alive_en, general_configuration_clk_gate_cntrl.keep_alive_en);
        bdmf_session_print(session, "keep_alive_intrvl = %u = 0x%x\n", general_configuration_clk_gate_cntrl.keep_alive_intrvl, general_configuration_clk_gate_cntrl.keep_alive_intrvl);
        bdmf_session_print(session, "keep_alive_cyc = %u = 0x%x\n", general_configuration_clk_gate_cntrl.keep_alive_cyc, general_configuration_clk_gate_cntrl.keep_alive_cyc);
        break;
    }
    case cli_bbh_rx_general_configuration_rxdbgsel:
    {
        uint8_t rxdbgsel;
        ag_err = ag_drv_bbh_rx_general_configuration_rxdbgsel_get(parm[1].value.unumber, &rxdbgsel);
        bdmf_session_print(session, "rxdbgsel = %u = 0x%x\n", rxdbgsel, rxdbgsel);
        break;
    }
    case cli_bbh_rx_general_configuration_rxrstrst:
    {
        bbh_rx_general_configuration_rxrstrst general_configuration_rxrstrst;
        ag_err = ag_drv_bbh_rx_general_configuration_rxrstrst_get(parm[1].value.unumber, &general_configuration_rxrstrst);
        bdmf_session_print(session, "portrst = %u = 0x%x\n", general_configuration_rxrstrst.portrst, general_configuration_rxrstrst.portrst);
        bdmf_session_print(session, "psramrst = %u = 0x%x\n", general_configuration_rxrstrst.psramrst, general_configuration_rxrstrst.psramrst);
        bdmf_session_print(session, "pdfiforst = %u = 0x%x\n", general_configuration_rxrstrst.pdfiforst, general_configuration_rxrstrst.pdfiforst);
        bdmf_session_print(session, "sbpmfiforst = %u = 0x%x\n", general_configuration_rxrstrst.sbpmfiforst, general_configuration_rxrstrst.sbpmfiforst);
        bdmf_session_print(session, "dispcreditrst = %u = 0x%x\n", general_configuration_rxrstrst.dispcreditrst, general_configuration_rxrstrst.dispcreditrst);
        bdmf_session_print(session, "upldfiforst = %u = 0x%x\n", general_configuration_rxrstrst.upldfiforst, general_configuration_rxrstrst.upldfiforst);
        bdmf_session_print(session, "pfbch = %u = 0x%x\n", general_configuration_rxrstrst.pfbch, general_configuration_rxrstrst.pfbch);
        bdmf_session_print(session, "dispcredit = %u = 0x%x\n", general_configuration_rxrstrst.dispcredit, general_configuration_rxrstrst.dispcredit);
        bdmf_session_print(session, "dispviq = %u = 0x%x\n", general_configuration_rxrstrst.dispviq, general_configuration_rxrstrst.dispviq);
        break;
    }
    case cli_bbh_rx_general_configuration_sbpmcfg:
    {
        uint8_t max_otf_sbpm_req;
        ag_err = ag_drv_bbh_rx_general_configuration_sbpmcfg_get(parm[1].value.unumber, &max_otf_sbpm_req);
        bdmf_session_print(session, "max_otf_sbpm_req = %u = 0x%x\n", max_otf_sbpm_req, max_otf_sbpm_req);
        break;
    }
    case cli_bbh_rx_general_configuration_psrambase:
    {
        uint32_t psrambase;
        ag_err = ag_drv_bbh_rx_general_configuration_psrambase_get(parm[1].value.unumber, &psrambase);
        bdmf_session_print(session, "psrambase = %u = 0x%x\n", psrambase, psrambase);
        break;
    }
    case cli_bbh_rx_general_configuration_ingresschannel:
    {
        uint8_t ingresschannel;
        ag_err = ag_drv_bbh_rx_general_configuration_ingresschannel_get(parm[1].value.unumber, &ingresschannel);
        bdmf_session_print(session, "ingresschannel = %u = 0x%x\n", ingresschannel, ingresschannel);
        break;
    }
    case cli_bbh_rx_general_configuration_bbovrdroute:
    {
        uint32_t route;
        ag_err = ag_drv_bbh_rx_general_configuration_bbovrdroute_get(parm[1].value.unumber, &route);
        bdmf_session_print(session, "route = %u = 0x%x\n", route, route);
        break;
    }
    case cli_bbh_rx_general_configuration_bbovrdctrl:
    {
        uint16_t dest;
        bdmf_boolean en;
        ag_err = ag_drv_bbh_rx_general_configuration_bbovrdctrl_get(parm[1].value.unumber, &dest, &en);
        bdmf_session_print(session, "dest = %u = 0x%x\n", dest, dest);
        bdmf_session_print(session, "en = %u = 0x%x\n", en, en);
        break;
    }
    case cli_bbh_rx_general_configuration_gemdone:
    {
        bdmf_boolean gemdone;
        ag_err = ag_drv_bbh_rx_general_configuration_gemdone_get(parm[1].value.unumber, &gemdone);
        bdmf_session_print(session, "gemdone = %u = 0x%x\n", gemdone, gemdone);
        break;
    }
    case cli_bbh_rx_general_configuration_mactype:
    {
        uint8_t mactype;
        ag_err = ag_drv_bbh_rx_general_configuration_mactype_get(parm[1].value.unumber, parm[2].value.unumber, &mactype);
        bdmf_session_print(session, "mactype = %u = 0x%x\n", mactype, mactype);
        break;
    }
    case cli_bbh_rx_general_configuration_portth:
    {
        uint8_t portthr;
        ag_err = ag_drv_bbh_rx_general_configuration_portth_get(parm[1].value.unumber, parm[2].value.unumber, &portthr);
        bdmf_session_print(session, "portthr = %u = 0x%x\n", portthr, portthr);
        break;
    }
    case cli_bbh_rx_general_configuration_sopoffset:
    {
        uint8_t sopoffset;
        ag_err = ag_drv_bbh_rx_general_configuration_sopoffset_get(parm[1].value.unumber, parm[2].value.unumber, &sopoffset);
        bdmf_session_print(session, "sopoffset = %u = 0x%x\n", sopoffset, sopoffset);
        break;
    }
    case cli_bbh_rx_general_configuration_ingresspid:
    {
        uint8_t ingresspid;
        ag_err = ag_drv_bbh_rx_general_configuration_ingresspid_get(parm[1].value.unumber, parm[2].value.unumber, &ingresspid);
        bdmf_session_print(session, "ingresspid = %u = 0x%x\n", ingresspid, ingresspid);
        break;
    }
    case cli_bbh_rx_general_configuration_portweight:
    {
        uint8_t portweight;
        ag_err = ag_drv_bbh_rx_general_configuration_portweight_get(parm[1].value.unumber, parm[2].value.unumber, &portweight);
        bdmf_session_print(session, "portweight = %u = 0x%x\n", portweight, portweight);
        break;
    }
    case cli_bbh_rx_general_configuration_portviqmask:
    {
        uint16_t portviqmask;
        ag_err = ag_drv_bbh_rx_general_configuration_portviqmask_get(parm[1].value.unumber, parm[2].value.unumber, &portviqmask);
        bdmf_session_print(session, "portviqmask = %u = 0x%x\n", portviqmask, portviqmask);
        break;
    }
    case cli_bbh_rx_general_configuration_excviqmask:
    {
        uint16_t excviqmask;
        bdmf_boolean excviqen;
        bdmf_boolean excviqoamen;
        ag_err = ag_drv_bbh_rx_general_configuration_excviqmask_get(parm[1].value.unumber, parm[2].value.unumber, &excviqmask, &excviqen, &excviqoamen);
        bdmf_session_print(session, "excviqmask = %u = 0x%x\n", excviqmask, excviqmask);
        bdmf_session_print(session, "excviqen = %u = 0x%x\n", excviqen, excviqen);
        bdmf_session_print(session, "excviqoamen = %u = 0x%x\n", excviqoamen, excviqoamen);
        break;
    }
    case cli_bbh_rx_general_configuration_dispviq:
    {
        uint8_t viq;
        ag_err = ag_drv_bbh_rx_general_configuration_dispviq_get(parm[1].value.unumber, parm[2].value.unumber, &viq);
        bdmf_session_print(session, "viq = %u = 0x%x\n", viq, viq);
        break;
    }
    case cli_bbh_rx_general_configuration_viqweight:
    {
        uint8_t viqweight;
        ag_err = ag_drv_bbh_rx_general_configuration_viqweight_get(parm[1].value.unumber, parm[2].value.unumber, &viqweight);
        bdmf_session_print(session, "viqweight = %u = 0x%x\n", viqweight, viqweight);
        break;
    }
    case cli_bbh_rx_general_configuration_patterncfg:
    {
        uint8_t location;
        bdmf_boolean no_drop_en;
        bdmf_boolean viqclassen;
        uint16_t viq;
        ag_err = ag_drv_bbh_rx_general_configuration_patterncfg_get(parm[1].value.unumber, parm[2].value.unumber, &location, &no_drop_en, &viqclassen, &viq);
        bdmf_session_print(session, "location = %u = 0x%x\n", location, location);
        bdmf_session_print(session, "no_drop_en = %u = 0x%x\n", no_drop_en, no_drop_en);
        bdmf_session_print(session, "viqclassen = %u = 0x%x\n", viqclassen, viqclassen);
        bdmf_session_print(session, "viq = %u = 0x%x\n", viq, viq);
        break;
    }
    case cli_bbh_rx_general_configuration_patterndata:
    {
        bbh_rx_general_configuration_patterndata general_configuration_patterndata;
        ag_err = ag_drv_bbh_rx_general_configuration_patterndata_get(parm[1].value.unumber, parm[2].value.unumber, &general_configuration_patterndata);
        bdmf_session_print(session, "pattern_data_config[0] = %u = 0x%x\n", general_configuration_patterndata.pattern_data_config[0], general_configuration_patterndata.pattern_data_config[0]);
        bdmf_session_print(session, "pattern_data_config[1] = %u = 0x%x\n", general_configuration_patterndata.pattern_data_config[1], general_configuration_patterndata.pattern_data_config[1]);
        break;
    }
    case cli_bbh_rx_general_configuration_patternmask:
    {
        uint32_t patternmask;
        ag_err = ag_drv_bbh_rx_general_configuration_patternmask_get(parm[1].value.unumber, parm[2].value.unumber, &patternmask);
        bdmf_session_print(session, "patternmask = %u = 0x%x\n", patternmask, patternmask);
        break;
    }
    case cli_bbh_rx_general_configuration_patterndata8b:
    {
        uint32_t patterndata;
        ag_err = ag_drv_bbh_rx_general_configuration_patterndata8b_get(parm[1].value.unumber, parm[2].value.unumber, &patterndata);
        bdmf_session_print(session, "patterndata = %u = 0x%x\n", patterndata, patterndata);
        break;
    }
    case cli_bbh_rx_general_configuration_patternmask8b:
    {
        uint32_t patternmask;
        ag_err = ag_drv_bbh_rx_general_configuration_patternmask8b_get(parm[1].value.unumber, parm[2].value.unumber, &patternmask);
        bdmf_session_print(session, "patternmask = %u = 0x%x\n", patternmask, patternmask);
        break;
    }
    case cli_bbh_rx_general_configuration_patternen:
    {
        uint8_t patternen;
        ag_err = ag_drv_bbh_rx_general_configuration_patternen_get(parm[1].value.unumber, parm[2].value.unumber, &patternen);
        bdmf_session_print(session, "patternen = %u = 0x%x\n", patternen, patternen);
        break;
    }
    case cli_bbh_rx_general_configuration_viqfcen:
    {
        uint16_t viqfcen;
        ag_err = ag_drv_bbh_rx_general_configuration_viqfcen_get(parm[1].value.unumber, parm[2].value.unumber, &viqfcen);
        bdmf_session_print(session, "viqfcen = %u = 0x%x\n", viqfcen, viqfcen);
        break;
    }
    case cli_bbh_rx_general_configuration_noneth:
    {
        uint16_t flowid;
        bdmf_boolean enable;
        ag_err = ag_drv_bbh_rx_general_configuration_noneth_get(parm[1].value.unumber, parm[2].value.unumber, &flowid, &enable);
        bdmf_session_print(session, "flowid = %u = 0x%x\n", flowid, flowid);
        bdmf_session_print(session, "enable = %u = 0x%x\n", enable, enable);
        break;
    }
    case cli_bbh_rx_pm_counters_inpkt:
    {
        bbh_rx_pm_counters_inpkt pm_counters_inpkt;
        ag_err = ag_drv_bbh_rx_pm_counters_inpkt_get(parm[1].value.unumber, parm[2].value.unumber, &pm_counters_inpkt);
        bdmf_session_print(session, "pm_counters_inpkt[0] = %u = 0x%x\n", pm_counters_inpkt.pm_counters_inpkt[0], pm_counters_inpkt.pm_counters_inpkt[0]);
        bdmf_session_print(session, "pm_counters_inpkt[1] = %u = 0x%x\n", pm_counters_inpkt.pm_counters_inpkt[1], pm_counters_inpkt.pm_counters_inpkt[1]);
        break;
    }
    case cli_bbh_rx_pm_counters_inbyte:
    {
        bbh_rx_pm_counters_inbyte pm_counters_inbyte;
        ag_err = ag_drv_bbh_rx_pm_counters_inbyte_get(parm[1].value.unumber, parm[2].value.unumber, &pm_counters_inbyte);
        bdmf_session_print(session, "pm_counters_inbyte[0] = %u = 0x%x\n", pm_counters_inbyte.pm_counters_inbyte[0], pm_counters_inbyte.pm_counters_inbyte[0]);
        bdmf_session_print(session, "pm_counters_inbyte[1] = %u = 0x%x\n", pm_counters_inbyte.pm_counters_inbyte[1], pm_counters_inbyte.pm_counters_inbyte[1]);
        break;
    }
    case cli_bbh_rx_pm_counters_tooshort:
    {
        bbh_rx_pm_counters_tooshort pm_counters_tooshort;
        ag_err = ag_drv_bbh_rx_pm_counters_tooshort_get(parm[1].value.unumber, parm[2].value.unumber, &pm_counters_tooshort);
        bdmf_session_print(session, "pm_counters_too_short[0] = %u = 0x%x\n", pm_counters_tooshort.pm_counters_too_short[0], pm_counters_tooshort.pm_counters_too_short[0]);
        bdmf_session_print(session, "pm_counters_too_short[1] = %u = 0x%x\n", pm_counters_tooshort.pm_counters_too_short[1], pm_counters_tooshort.pm_counters_too_short[1]);
        break;
    }
    case cli_bbh_rx_pm_counters_toolong:
    {
        bbh_rx_pm_counters_toolong pm_counters_toolong;
        ag_err = ag_drv_bbh_rx_pm_counters_toolong_get(parm[1].value.unumber, parm[2].value.unumber, &pm_counters_toolong);
        bdmf_session_print(session, "pm_counters_too_long[0] = %u = 0x%x\n", pm_counters_toolong.pm_counters_too_long[0], pm_counters_toolong.pm_counters_too_long[0]);
        bdmf_session_print(session, "pm_counters_too_long[1] = %u = 0x%x\n", pm_counters_toolong.pm_counters_too_long[1], pm_counters_toolong.pm_counters_too_long[1]);
        break;
    }
    case cli_bbh_rx_pm_counters_crcerror:
    {
        bbh_rx_pm_counters_crcerror pm_counters_crcerror;
        ag_err = ag_drv_bbh_rx_pm_counters_crcerror_get(parm[1].value.unumber, parm[2].value.unumber, &pm_counters_crcerror);
        bdmf_session_print(session, "pm_counters_crc_error[0] = %u = 0x%x\n", pm_counters_crcerror.pm_counters_crc_error[0], pm_counters_crcerror.pm_counters_crc_error[0]);
        bdmf_session_print(session, "pm_counters_crc_error[1] = %u = 0x%x\n", pm_counters_crcerror.pm_counters_crc_error[1], pm_counters_crcerror.pm_counters_crc_error[1]);
        break;
    }
    case cli_bbh_rx_pm_counters_fcdrop:
    {
        bbh_rx_pm_counters_fcdrop pm_counters_fcdrop;
        ag_err = ag_drv_bbh_rx_pm_counters_fcdrop_get(parm[1].value.unumber, parm[2].value.unumber, &pm_counters_fcdrop);
        bdmf_session_print(session, "pm_counters_fc_drop[0] = %u = 0x%x\n", pm_counters_fcdrop.pm_counters_fc_drop[0], pm_counters_fcdrop.pm_counters_fc_drop[0]);
        bdmf_session_print(session, "pm_counters_fc_drop[1] = %u = 0x%x\n", pm_counters_fcdrop.pm_counters_fc_drop[1], pm_counters_fcdrop.pm_counters_fc_drop[1]);
        break;
    }
    case cli_bbh_rx_pm_counters_truncate:
    {
        bbh_rx_pm_counters_truncate pm_counters_truncate;
        ag_err = ag_drv_bbh_rx_pm_counters_truncate_get(parm[1].value.unumber, parm[2].value.unumber, &pm_counters_truncate);
        bdmf_session_print(session, "pm_counters_truncate[0] = %u = 0x%x\n", pm_counters_truncate.pm_counters_truncate[0], pm_counters_truncate.pm_counters_truncate[0]);
        bdmf_session_print(session, "pm_counters_truncate[1] = %u = 0x%x\n", pm_counters_truncate.pm_counters_truncate[1], pm_counters_truncate.pm_counters_truncate[1]);
        break;
    }
    case cli_bbh_rx_pm_counters_totaldrop:
    {
        bbh_rx_pm_counters_totaldrop pm_counters_totaldrop;
        ag_err = ag_drv_bbh_rx_pm_counters_totaldrop_get(parm[1].value.unumber, parm[2].value.unumber, &pm_counters_totaldrop);
        bdmf_session_print(session, "pm_counters_total_drop[0] = %u = 0x%x\n", pm_counters_totaldrop.pm_counters_total_drop[0], pm_counters_totaldrop.pm_counters_total_drop[0]);
        bdmf_session_print(session, "pm_counters_total_drop[1] = %u = 0x%x\n", pm_counters_totaldrop.pm_counters_total_drop[1], pm_counters_totaldrop.pm_counters_total_drop[1]);
        break;
    }
    case cli_bbh_rx_pm_counters_dummypdcnt:
    {
        uint32_t pmvalue;
        ag_err = ag_drv_bbh_rx_pm_counters_dummypdcnt_get(parm[1].value.unumber, &pmvalue);
        bdmf_session_print(session, "pmvalue = %u = 0x%x\n", pmvalue, pmvalue);
        break;
    }
    case cli_bbh_rx_pon_rmon_counters_r64:
    {
        uint32_t r64;
        ag_err = ag_drv_bbh_rx_pon_rmon_counters_r64_get(parm[1].value.unumber, &r64);
        bdmf_session_print(session, "r64 = %u = 0x%x\n", r64, r64);
        break;
    }
    case cli_bbh_rx_pon_rmon_counters_r127:
    {
        uint32_t r127;
        ag_err = ag_drv_bbh_rx_pon_rmon_counters_r127_get(parm[1].value.unumber, &r127);
        bdmf_session_print(session, "r127 = %u = 0x%x\n", r127, r127);
        break;
    }
    case cli_bbh_rx_pon_rmon_counters_r255:
    {
        uint32_t r255;
        ag_err = ag_drv_bbh_rx_pon_rmon_counters_r255_get(parm[1].value.unumber, &r255);
        bdmf_session_print(session, "r255 = %u = 0x%x\n", r255, r255);
        break;
    }
    case cli_bbh_rx_pon_rmon_counters_r511:
    {
        uint32_t r511;
        ag_err = ag_drv_bbh_rx_pon_rmon_counters_r511_get(parm[1].value.unumber, &r511);
        bdmf_session_print(session, "r511 = %u = 0x%x\n", r511, r511);
        break;
    }
    case cli_bbh_rx_pon_rmon_counters_r1023:
    {
        uint32_t r1023;
        ag_err = ag_drv_bbh_rx_pon_rmon_counters_r1023_get(parm[1].value.unumber, &r1023);
        bdmf_session_print(session, "r1023 = %u = 0x%x\n", r1023, r1023);
        break;
    }
    case cli_bbh_rx_pon_rmon_counters_r1518:
    {
        uint32_t r1518;
        ag_err = ag_drv_bbh_rx_pon_rmon_counters_r1518_get(parm[1].value.unumber, &r1518);
        bdmf_session_print(session, "r1518 = %u = 0x%x\n", r1518, r1518);
        break;
    }
    case cli_bbh_rx_pon_rmon_counters_r2047:
    {
        uint32_t r1518;
        ag_err = ag_drv_bbh_rx_pon_rmon_counters_r2047_get(parm[1].value.unumber, &r1518);
        bdmf_session_print(session, "r1518 = %u = 0x%x\n", r1518, r1518);
        break;
    }
    case cli_bbh_rx_pon_rmon_counters_r4095:
    {
        uint32_t r4095;
        ag_err = ag_drv_bbh_rx_pon_rmon_counters_r4095_get(parm[1].value.unumber, &r4095);
        bdmf_session_print(session, "r4095 = %u = 0x%x\n", r4095, r4095);
        break;
    }
    case cli_bbh_rx_pon_rmon_counters_r9216:
    {
        uint32_t r9216;
        ag_err = ag_drv_bbh_rx_pon_rmon_counters_r9216_get(parm[1].value.unumber, &r9216);
        bdmf_session_print(session, "r9216 = %u = 0x%x\n", r9216, r9216);
        break;
    }
    case cli_bbh_rx_pon_rmon_counters_r16383:
    {
        uint32_t r16383;
        ag_err = ag_drv_bbh_rx_pon_rmon_counters_r16383_get(parm[1].value.unumber, &r16383);
        bdmf_session_print(session, "r16383 = %u = 0x%x\n", r16383, r16383);
        break;
    }
    case cli_bbh_rx_debug_pfbuw:
    {
        bbh_rx_debug_pfbuw debug_pfbuw;
        ag_err = ag_drv_bbh_rx_debug_pfbuw_get(parm[1].value.unumber, parm[2].value.unumber, &debug_pfbuw);
        bdmf_session_print(session, "debug_pfbuw[0] = %u = 0x%x\n", debug_pfbuw.debug_pfbuw[0], debug_pfbuw.debug_pfbuw[0]);
        bdmf_session_print(session, "debug_pfbuw[1] = %u = 0x%x\n", debug_pfbuw.debug_pfbuw[1], debug_pfbuw.debug_pfbuw[1]);
        bdmf_session_print(session, "debug_pfbuw[2] = %u = 0x%x\n", debug_pfbuw.debug_pfbuw[2], debug_pfbuw.debug_pfbuw[2]);
        bdmf_session_print(session, "debug_pfbuw[3] = %u = 0x%x\n", debug_pfbuw.debug_pfbuw[3], debug_pfbuw.debug_pfbuw[3]);
        bdmf_session_print(session, "debug_pfbuw[4] = %u = 0x%x\n", debug_pfbuw.debug_pfbuw[4], debug_pfbuw.debug_pfbuw[4]);
        bdmf_session_print(session, "debug_pfbuw[5] = %u = 0x%x\n", debug_pfbuw.debug_pfbuw[5], debug_pfbuw.debug_pfbuw[5]);
        bdmf_session_print(session, "debug_pfbuw[6] = %u = 0x%x\n", debug_pfbuw.debug_pfbuw[6], debug_pfbuw.debug_pfbuw[6]);
        bdmf_session_print(session, "debug_pfbuw[7] = %u = 0x%x\n", debug_pfbuw.debug_pfbuw[7], debug_pfbuw.debug_pfbuw[7]);
        break;
    }
    case cli_bbh_rx_debug_patternuw:
    {
        bbh_rx_debug_patternuw debug_patternuw;
        ag_err = ag_drv_bbh_rx_debug_patternuw_get(parm[1].value.unumber, parm[2].value.unumber, &debug_patternuw);
        bdmf_session_print(session, "debug_patternuw[0] = %u = 0x%x\n", debug_patternuw.debug_patternuw[0], debug_patternuw.debug_patternuw[0]);
        bdmf_session_print(session, "debug_patternuw[1] = %u = 0x%x\n", debug_patternuw.debug_patternuw[1], debug_patternuw.debug_patternuw[1]);
        bdmf_session_print(session, "debug_patternuw[2] = %u = 0x%x\n", debug_patternuw.debug_patternuw[2], debug_patternuw.debug_patternuw[2]);
        bdmf_session_print(session, "debug_patternuw[3] = %u = 0x%x\n", debug_patternuw.debug_patternuw[3], debug_patternuw.debug_patternuw[3]);
        bdmf_session_print(session, "debug_patternuw[4] = %u = 0x%x\n", debug_patternuw.debug_patternuw[4], debug_patternuw.debug_patternuw[4]);
        bdmf_session_print(session, "debug_patternuw[5] = %u = 0x%x\n", debug_patternuw.debug_patternuw[5], debug_patternuw.debug_patternuw[5]);
        bdmf_session_print(session, "debug_patternuw[6] = %u = 0x%x\n", debug_patternuw.debug_patternuw[6], debug_patternuw.debug_patternuw[6]);
        bdmf_session_print(session, "debug_patternuw[7] = %u = 0x%x\n", debug_patternuw.debug_patternuw[7], debug_patternuw.debug_patternuw[7]);
        break;
    }
    case cli_bbh_rx_debug_excuw:
    {
        bbh_rx_debug_excuw debug_excuw;
        ag_err = ag_drv_bbh_rx_debug_excuw_get(parm[1].value.unumber, parm[2].value.unumber, &debug_excuw);
        bdmf_session_print(session, "debug_excuw[0] = %u = 0x%x\n", debug_excuw.debug_excuw[0], debug_excuw.debug_excuw[0]);
        bdmf_session_print(session, "debug_excuw[1] = %u = 0x%x\n", debug_excuw.debug_excuw[1], debug_excuw.debug_excuw[1]);
        bdmf_session_print(session, "debug_excuw[2] = %u = 0x%x\n", debug_excuw.debug_excuw[2], debug_excuw.debug_excuw[2]);
        bdmf_session_print(session, "debug_excuw[3] = %u = 0x%x\n", debug_excuw.debug_excuw[3], debug_excuw.debug_excuw[3]);
        bdmf_session_print(session, "debug_excuw[4] = %u = 0x%x\n", debug_excuw.debug_excuw[4], debug_excuw.debug_excuw[4]);
        bdmf_session_print(session, "debug_excuw[5] = %u = 0x%x\n", debug_excuw.debug_excuw[5], debug_excuw.debug_excuw[5]);
        bdmf_session_print(session, "debug_excuw[6] = %u = 0x%x\n", debug_excuw.debug_excuw[6], debug_excuw.debug_excuw[6]);
        bdmf_session_print(session, "debug_excuw[7] = %u = 0x%x\n", debug_excuw.debug_excuw[7], debug_excuw.debug_excuw[7]);
        break;
    }
    case cli_bbh_rx_debug_portuw:
    {
        bbh_rx_debug_portuw debug_portuw;
        ag_err = ag_drv_bbh_rx_debug_portuw_get(parm[1].value.unumber, parm[2].value.unumber, &debug_portuw);
        bdmf_session_print(session, "debug_portuw[0] = %u = 0x%x\n", debug_portuw.debug_portuw[0], debug_portuw.debug_portuw[0]);
        bdmf_session_print(session, "debug_portuw[1] = %u = 0x%x\n", debug_portuw.debug_portuw[1], debug_portuw.debug_portuw[1]);
        bdmf_session_print(session, "debug_portuw[2] = %u = 0x%x\n", debug_portuw.debug_portuw[2], debug_portuw.debug_portuw[2]);
        bdmf_session_print(session, "debug_portuw[3] = %u = 0x%x\n", debug_portuw.debug_portuw[3], debug_portuw.debug_portuw[3]);
        bdmf_session_print(session, "debug_portuw[4] = %u = 0x%x\n", debug_portuw.debug_portuw[4], debug_portuw.debug_portuw[4]);
        bdmf_session_print(session, "debug_portuw[5] = %u = 0x%x\n", debug_portuw.debug_portuw[5], debug_portuw.debug_portuw[5]);
        bdmf_session_print(session, "debug_portuw[6] = %u = 0x%x\n", debug_portuw.debug_portuw[6], debug_portuw.debug_portuw[6]);
        bdmf_session_print(session, "debug_portuw[7] = %u = 0x%x\n", debug_portuw.debug_portuw[7], debug_portuw.debug_portuw[7]);
        break;
    }
    case cli_bbh_rx_debug_firstsbn:
    {
        bbh_rx_debug_firstsbn debug_firstsbn;
        ag_err = ag_drv_bbh_rx_debug_firstsbn_get(parm[1].value.unumber, parm[2].value.unumber, &debug_firstsbn);
        bdmf_session_print(session, "debug_first_sbn[0] = %u = 0x%x\n", debug_firstsbn.debug_first_sbn[0], debug_firstsbn.debug_first_sbn[0]);
        bdmf_session_print(session, "debug_first_sbn[1] = %u = 0x%x\n", debug_firstsbn.debug_first_sbn[1], debug_firstsbn.debug_first_sbn[1]);
        bdmf_session_print(session, "debug_first_sbn[2] = %u = 0x%x\n", debug_firstsbn.debug_first_sbn[2], debug_firstsbn.debug_first_sbn[2]);
        bdmf_session_print(session, "debug_first_sbn[3] = %u = 0x%x\n", debug_firstsbn.debug_first_sbn[3], debug_firstsbn.debug_first_sbn[3]);
        bdmf_session_print(session, "debug_first_sbn[4] = %u = 0x%x\n", debug_firstsbn.debug_first_sbn[4], debug_firstsbn.debug_first_sbn[4]);
        bdmf_session_print(session, "debug_first_sbn[5] = %u = 0x%x\n", debug_firstsbn.debug_first_sbn[5], debug_firstsbn.debug_first_sbn[5]);
        bdmf_session_print(session, "debug_first_sbn[6] = %u = 0x%x\n", debug_firstsbn.debug_first_sbn[6], debug_firstsbn.debug_first_sbn[6]);
        bdmf_session_print(session, "debug_first_sbn[7] = %u = 0x%x\n", debug_firstsbn.debug_first_sbn[7], debug_firstsbn.debug_first_sbn[7]);
        break;
    }
    case cli_bbh_rx_debug_currentsbn:
    {
        bbh_rx_debug_currentsbn debug_currentsbn;
        ag_err = ag_drv_bbh_rx_debug_currentsbn_get(parm[1].value.unumber, parm[2].value.unumber, &debug_currentsbn);
        bdmf_session_print(session, "debug_current_sbn[0] = %u = 0x%x\n", debug_currentsbn.debug_current_sbn[0], debug_currentsbn.debug_current_sbn[0]);
        bdmf_session_print(session, "debug_current_sbn[1] = %u = 0x%x\n", debug_currentsbn.debug_current_sbn[1], debug_currentsbn.debug_current_sbn[1]);
        bdmf_session_print(session, "debug_current_sbn[2] = %u = 0x%x\n", debug_currentsbn.debug_current_sbn[2], debug_currentsbn.debug_current_sbn[2]);
        bdmf_session_print(session, "debug_current_sbn[3] = %u = 0x%x\n", debug_currentsbn.debug_current_sbn[3], debug_currentsbn.debug_current_sbn[3]);
        bdmf_session_print(session, "debug_current_sbn[4] = %u = 0x%x\n", debug_currentsbn.debug_current_sbn[4], debug_currentsbn.debug_current_sbn[4]);
        bdmf_session_print(session, "debug_current_sbn[5] = %u = 0x%x\n", debug_currentsbn.debug_current_sbn[5], debug_currentsbn.debug_current_sbn[5]);
        bdmf_session_print(session, "debug_current_sbn[6] = %u = 0x%x\n", debug_currentsbn.debug_current_sbn[6], debug_currentsbn.debug_current_sbn[6]);
        bdmf_session_print(session, "debug_current_sbn[7] = %u = 0x%x\n", debug_currentsbn.debug_current_sbn[7], debug_currentsbn.debug_current_sbn[7]);
        break;
    }
    case cli_bbh_rx_debug_sn:
    {
        bbh_rx_debug_sn debug_sn;
        ag_err = ag_drv_bbh_rx_debug_sn_get(parm[1].value.unumber, parm[2].value.unumber, &debug_sn);
        bdmf_session_print(session, "debug_sn[0] = %u = 0x%x\n", debug_sn.debug_sn[0], debug_sn.debug_sn[0]);
        bdmf_session_print(session, "debug_sn[1] = %u = 0x%x\n", debug_sn.debug_sn[1], debug_sn.debug_sn[1]);
        bdmf_session_print(session, "debug_sn[2] = %u = 0x%x\n", debug_sn.debug_sn[2], debug_sn.debug_sn[2]);
        bdmf_session_print(session, "debug_sn[3] = %u = 0x%x\n", debug_sn.debug_sn[3], debug_sn.debug_sn[3]);
        bdmf_session_print(session, "debug_sn[4] = %u = 0x%x\n", debug_sn.debug_sn[4], debug_sn.debug_sn[4]);
        bdmf_session_print(session, "debug_sn[5] = %u = 0x%x\n", debug_sn.debug_sn[5], debug_sn.debug_sn[5]);
        bdmf_session_print(session, "debug_sn[6] = %u = 0x%x\n", debug_sn.debug_sn[6], debug_sn.debug_sn[6]);
        bdmf_session_print(session, "debug_sn[7] = %u = 0x%x\n", debug_sn.debug_sn[7], debug_sn.debug_sn[7]);
        break;
    }
    case cli_bbh_rx_debug_viqmask:
    {
        bbh_rx_debug_viqmask debug_viqmask;
        ag_err = ag_drv_bbh_rx_debug_viqmask_get(parm[1].value.unumber, parm[2].value.unumber, &debug_viqmask);
        bdmf_session_print(session, "debug_viq_mask[0] = %u = 0x%x\n", debug_viqmask.debug_viq_mask[0], debug_viqmask.debug_viq_mask[0]);
        bdmf_session_print(session, "debug_viq_mask[1] = %u = 0x%x\n", debug_viqmask.debug_viq_mask[1], debug_viqmask.debug_viq_mask[1]);
        bdmf_session_print(session, "debug_viq_mask[2] = %u = 0x%x\n", debug_viqmask.debug_viq_mask[2], debug_viqmask.debug_viq_mask[2]);
        bdmf_session_print(session, "debug_viq_mask[3] = %u = 0x%x\n", debug_viqmask.debug_viq_mask[3], debug_viqmask.debug_viq_mask[3]);
        bdmf_session_print(session, "debug_viq_mask[4] = %u = 0x%x\n", debug_viqmask.debug_viq_mask[4], debug_viqmask.debug_viq_mask[4]);
        bdmf_session_print(session, "debug_viq_mask[5] = %u = 0x%x\n", debug_viqmask.debug_viq_mask[5], debug_viqmask.debug_viq_mask[5]);
        bdmf_session_print(session, "debug_viq_mask[6] = %u = 0x%x\n", debug_viqmask.debug_viq_mask[6], debug_viqmask.debug_viq_mask[6]);
        bdmf_session_print(session, "debug_viq_mask[7] = %u = 0x%x\n", debug_viqmask.debug_viq_mask[7], debug_viqmask.debug_viq_mask[7]);
        break;
    }
    case cli_bbh_rx_debug_plen:
    {
        bbh_rx_debug_plen debug_plen;
        ag_err = ag_drv_bbh_rx_debug_plen_get(parm[1].value.unumber, parm[2].value.unumber, &debug_plen);
        bdmf_session_print(session, "debug_plen[0] = %u = 0x%x\n", debug_plen.debug_plen[0], debug_plen.debug_plen[0]);
        bdmf_session_print(session, "debug_plen[1] = %u = 0x%x\n", debug_plen.debug_plen[1], debug_plen.debug_plen[1]);
        bdmf_session_print(session, "debug_plen[2] = %u = 0x%x\n", debug_plen.debug_plen[2], debug_plen.debug_plen[2]);
        bdmf_session_print(session, "debug_plen[3] = %u = 0x%x\n", debug_plen.debug_plen[3], debug_plen.debug_plen[3]);
        bdmf_session_print(session, "debug_plen[4] = %u = 0x%x\n", debug_plen.debug_plen[4], debug_plen.debug_plen[4]);
        bdmf_session_print(session, "debug_plen[5] = %u = 0x%x\n", debug_plen.debug_plen[5], debug_plen.debug_plen[5]);
        bdmf_session_print(session, "debug_plen[6] = %u = 0x%x\n", debug_plen.debug_plen[6], debug_plen.debug_plen[6]);
        bdmf_session_print(session, "debug_plen[7] = %u = 0x%x\n", debug_plen.debug_plen[7], debug_plen.debug_plen[7]);
        break;
    }
    case cli_bbh_rx_debug_eopok:
    {
        uint8_t context;
        ag_err = ag_drv_bbh_rx_debug_eopok_get(parm[1].value.unumber, &context);
        bdmf_session_print(session, "context = %u = 0x%x\n", context, context);
        break;
    }
    case cli_bbh_rx_debug_viqfull:
    {
        uint8_t context;
        ag_err = ag_drv_bbh_rx_debug_viqfull_get(parm[1].value.unumber, &context);
        bdmf_session_print(session, "context = %u = 0x%x\n", context, context);
        break;
    }
    case cli_bbh_rx_debug_portreq:
    {
        uint8_t context;
        ag_err = ag_drv_bbh_rx_debug_portreq_get(parm[1].value.unumber, &context);
        bdmf_session_print(session, "context = %u = 0x%x\n", context, context);
        break;
    }
    case cli_bbh_rx_debug_dummyreq:
    {
        uint8_t context;
        ag_err = ag_drv_bbh_rx_debug_dummyreq_get(parm[1].value.unumber, &context);
        bdmf_session_print(session, "context = %u = 0x%x\n", context, context);
        break;
    }
    case cli_bbh_rx_debug_inreassembly:
    {
        uint8_t context;
        ag_err = ag_drv_bbh_rx_debug_inreassembly_get(parm[1].value.unumber, &context);
        bdmf_session_print(session, "context = %u = 0x%x\n", context, context);
        break;
    }
    case cli_bbh_rx_debug_dummypd:
    {
        uint8_t context;
        ag_err = ag_drv_bbh_rx_debug_dummypd_get(parm[1].value.unumber, &context);
        bdmf_session_print(session, "context = %u = 0x%x\n", context, context);
        break;
    }
    case cli_bbh_rx_debug_ufuw:
    {
        uint8_t uw;
        ag_err = ag_drv_bbh_rx_debug_ufuw_get(parm[1].value.unumber, &uw);
        bdmf_session_print(session, "uw = %u = 0x%x\n", uw, uw);
        break;
    }
    case cli_bbh_rx_debug_pendconnectuw:
    {
        uint8_t uw;
        ag_err = ag_drv_bbh_rx_debug_pendconnectuw_get(parm[1].value.unumber, &uw);
        bdmf_session_print(session, "uw = %u = 0x%x\n", uw, uw);
        break;
    }
    case cli_bbh_rx_debug_dbgvec:
    {
        bbh_rx_debug_dbgvec debug_dbgvec;
        ag_err = ag_drv_bbh_rx_debug_dbgvec_get(parm[1].value.unumber, parm[2].value.unumber, &debug_dbgvec);
        bdmf_session_print(session, "debug_dbg_veq[0] = %u = 0x%x\n", debug_dbgvec.debug_dbg_veq[0], debug_dbgvec.debug_dbg_veq[0]);
        bdmf_session_print(session, "debug_dbg_veq[1] = %u = 0x%x\n", debug_dbgvec.debug_dbg_veq[1], debug_dbgvec.debug_dbg_veq[1]);
        bdmf_session_print(session, "debug_dbg_veq[2] = %u = 0x%x\n", debug_dbgvec.debug_dbg_veq[2], debug_dbgvec.debug_dbg_veq[2]);
        bdmf_session_print(session, "debug_dbg_veq[3] = %u = 0x%x\n", debug_dbgvec.debug_dbg_veq[3], debug_dbgvec.debug_dbg_veq[3]);
        bdmf_session_print(session, "debug_dbg_veq[4] = %u = 0x%x\n", debug_dbgvec.debug_dbg_veq[4], debug_dbgvec.debug_dbg_veq[4]);
        bdmf_session_print(session, "debug_dbg_veq[5] = %u = 0x%x\n", debug_dbgvec.debug_dbg_veq[5], debug_dbgvec.debug_dbg_veq[5]);
        bdmf_session_print(session, "debug_dbg_veq[6] = %u = 0x%x\n", debug_dbgvec.debug_dbg_veq[6], debug_dbgvec.debug_dbg_veq[6]);
        bdmf_session_print(session, "debug_dbg_veq[7] = %u = 0x%x\n", debug_dbgvec.debug_dbg_veq[7], debug_dbgvec.debug_dbg_veq[7]);
        break;
    }
    case cli_bbh_rx_debug_pduw:
    {
        bbh_rx_debug_pduw debug_pduw;
        ag_err = ag_drv_bbh_rx_debug_pduw_get(parm[1].value.unumber, parm[2].value.unumber, &debug_pduw);
        bdmf_session_print(session, "debug_pduw[0] = %u = 0x%x\n", debug_pduw.debug_pduw[0], debug_pduw.debug_pduw[0]);
        bdmf_session_print(session, "debug_pduw[1] = %u = 0x%x\n", debug_pduw.debug_pduw[1], debug_pduw.debug_pduw[1]);
        bdmf_session_print(session, "debug_pduw[2] = %u = 0x%x\n", debug_pduw.debug_pduw[2], debug_pduw.debug_pduw[2]);
        bdmf_session_print(session, "debug_pduw[3] = %u = 0x%x\n", debug_pduw.debug_pduw[3], debug_pduw.debug_pduw[3]);
        bdmf_session_print(session, "debug_pduw[4] = %u = 0x%x\n", debug_pduw.debug_pduw[4], debug_pduw.debug_pduw[4]);
        bdmf_session_print(session, "debug_pduw[5] = %u = 0x%x\n", debug_pduw.debug_pduw[5], debug_pduw.debug_pduw[5]);
        bdmf_session_print(session, "debug_pduw[6] = %u = 0x%x\n", debug_pduw.debug_pduw[6], debug_pduw.debug_pduw[6]);
        bdmf_session_print(session, "debug_pduw[7] = %u = 0x%x\n", debug_pduw.debug_pduw[7], debug_pduw.debug_pduw[7]);
        break;
    }
    case cli_bbh_rx_debug_sbpmack:
    {
        bbh_rx_debug_sbpmack debug_sbpmack;
        ag_err = ag_drv_bbh_rx_debug_sbpmack_get(parm[1].value.unumber, parm[2].value.unumber, &debug_sbpmack);
        bdmf_session_print(session, "debug_sbpm_ack[0] = %u = 0x%x\n", debug_sbpmack.debug_sbpm_ack[0], debug_sbpmack.debug_sbpm_ack[0]);
        bdmf_session_print(session, "debug_sbpm_ack[1] = %u = 0x%x\n", debug_sbpmack.debug_sbpm_ack[1], debug_sbpmack.debug_sbpm_ack[1]);
        bdmf_session_print(session, "debug_sbpm_ack[2] = %u = 0x%x\n", debug_sbpmack.debug_sbpm_ack[2], debug_sbpmack.debug_sbpm_ack[2]);
        bdmf_session_print(session, "debug_sbpm_ack[3] = %u = 0x%x\n", debug_sbpmack.debug_sbpm_ack[3], debug_sbpmack.debug_sbpm_ack[3]);
        bdmf_session_print(session, "debug_sbpm_ack[4] = %u = 0x%x\n", debug_sbpmack.debug_sbpm_ack[4], debug_sbpmack.debug_sbpm_ack[4]);
        bdmf_session_print(session, "debug_sbpm_ack[5] = %u = 0x%x\n", debug_sbpmack.debug_sbpm_ack[5], debug_sbpmack.debug_sbpm_ack[5]);
        bdmf_session_print(session, "debug_sbpm_ack[6] = %u = 0x%x\n", debug_sbpmack.debug_sbpm_ack[6], debug_sbpmack.debug_sbpm_ack[6]);
        bdmf_session_print(session, "debug_sbpm_ack[7] = %u = 0x%x\n", debug_sbpmack.debug_sbpm_ack[7], debug_sbpmack.debug_sbpm_ack[7]);
        break;
    }
    case cli_bbh_rx_debug_writereplyack:
    {
        bbh_rx_debug_writereplyack debug_writereplyack;
        ag_err = ag_drv_bbh_rx_debug_writereplyack_get(parm[1].value.unumber, parm[2].value.unumber, &debug_writereplyack);
        bdmf_session_print(session, "debug_write_replay_ack[0] = %u = 0x%x\n", debug_writereplyack.debug_write_replay_ack[0], debug_writereplyack.debug_write_replay_ack[0]);
        bdmf_session_print(session, "debug_write_replay_ack[1] = %u = 0x%x\n", debug_writereplyack.debug_write_replay_ack[1], debug_writereplyack.debug_write_replay_ack[1]);
        bdmf_session_print(session, "debug_write_replay_ack[2] = %u = 0x%x\n", debug_writereplyack.debug_write_replay_ack[2], debug_writereplyack.debug_write_replay_ack[2]);
        bdmf_session_print(session, "debug_write_replay_ack[3] = %u = 0x%x\n", debug_writereplyack.debug_write_replay_ack[3], debug_writereplyack.debug_write_replay_ack[3]);
        bdmf_session_print(session, "debug_write_replay_ack[4] = %u = 0x%x\n", debug_writereplyack.debug_write_replay_ack[4], debug_writereplyack.debug_write_replay_ack[4]);
        bdmf_session_print(session, "debug_write_replay_ack[5] = %u = 0x%x\n", debug_writereplyack.debug_write_replay_ack[5], debug_writereplyack.debug_write_replay_ack[5]);
        bdmf_session_print(session, "debug_write_replay_ack[6] = %u = 0x%x\n", debug_writereplyack.debug_write_replay_ack[6], debug_writereplyack.debug_write_replay_ack[6]);
        bdmf_session_print(session, "debug_write_replay_ack[7] = %u = 0x%x\n", debug_writereplyack.debug_write_replay_ack[7], debug_writereplyack.debug_write_replay_ack[7]);
        break;
    }
    case cli_bbh_rx_debug_dispcreditack:
    {
        bbh_rx_debug_dispcreditack debug_dispcreditack;
        ag_err = ag_drv_bbh_rx_debug_dispcreditack_get(parm[1].value.unumber, parm[2].value.unumber, &debug_dispcreditack);
        bdmf_session_print(session, "debug_disp_credit_ack[0] = %u = 0x%x\n", debug_dispcreditack.debug_disp_credit_ack[0], debug_dispcreditack.debug_disp_credit_ack[0]);
        bdmf_session_print(session, "debug_disp_credit_ack[1] = %u = 0x%x\n", debug_dispcreditack.debug_disp_credit_ack[1], debug_dispcreditack.debug_disp_credit_ack[1]);
        bdmf_session_print(session, "debug_disp_credit_ack[2] = %u = 0x%x\n", debug_dispcreditack.debug_disp_credit_ack[2], debug_dispcreditack.debug_disp_credit_ack[2]);
        bdmf_session_print(session, "debug_disp_credit_ack[3] = %u = 0x%x\n", debug_dispcreditack.debug_disp_credit_ack[3], debug_dispcreditack.debug_disp_credit_ack[3]);
        bdmf_session_print(session, "debug_disp_credit_ack[4] = %u = 0x%x\n", debug_dispcreditack.debug_disp_credit_ack[4], debug_dispcreditack.debug_disp_credit_ack[4]);
        bdmf_session_print(session, "debug_disp_credit_ack[5] = %u = 0x%x\n", debug_dispcreditack.debug_disp_credit_ack[5], debug_dispcreditack.debug_disp_credit_ack[5]);
        bdmf_session_print(session, "debug_disp_credit_ack[6] = %u = 0x%x\n", debug_dispcreditack.debug_disp_credit_ack[6], debug_dispcreditack.debug_disp_credit_ack[6]);
        bdmf_session_print(session, "debug_disp_credit_ack[7] = %u = 0x%x\n", debug_dispcreditack.debug_disp_credit_ack[7], debug_dispcreditack.debug_disp_credit_ack[7]);
        break;
    }
    case cli_bbh_rx_debug_pdrddata:
    {
        bbh_rx_debug_pdrddata debug_pdrddata;
        ag_err = ag_drv_bbh_rx_debug_pdrddata_get(parm[1].value.unumber, parm[2].value.unumber, &debug_pdrddata);
        bdmf_session_print(session, "debug_pdrd_data[0] = %u = 0x%x\n", debug_pdrddata.debug_pdrd_data[0], debug_pdrddata.debug_pdrd_data[0]);
        bdmf_session_print(session, "debug_pdrd_data[1] = %u = 0x%x\n", debug_pdrddata.debug_pdrd_data[1], debug_pdrddata.debug_pdrd_data[1]);
        bdmf_session_print(session, "debug_pdrd_data[2] = %u = 0x%x\n", debug_pdrddata.debug_pdrd_data[2], debug_pdrddata.debug_pdrd_data[2]);
        bdmf_session_print(session, "debug_pdrd_data[3] = %u = 0x%x\n", debug_pdrddata.debug_pdrd_data[3], debug_pdrddata.debug_pdrd_data[3]);
        bdmf_session_print(session, "debug_pdrd_data[4] = %u = 0x%x\n", debug_pdrddata.debug_pdrd_data[4], debug_pdrddata.debug_pdrd_data[4]);
        bdmf_session_print(session, "debug_pdrd_data[5] = %u = 0x%x\n", debug_pdrddata.debug_pdrd_data[5], debug_pdrddata.debug_pdrd_data[5]);
        bdmf_session_print(session, "debug_pdrd_data[6] = %u = 0x%x\n", debug_pdrddata.debug_pdrd_data[6], debug_pdrddata.debug_pdrd_data[6]);
        bdmf_session_print(session, "debug_pdrd_data[7] = %u = 0x%x\n", debug_pdrddata.debug_pdrd_data[7], debug_pdrddata.debug_pdrd_data[7]);
        break;
    }
    case cli_bbh_rx_debug_sbnfifo:
    {
        bbh_rx_debug_sbnfifo debug_sbnfifo;
        ag_err = ag_drv_bbh_rx_debug_sbnfifo_get(parm[1].value.unumber, parm[2].value.unumber, &debug_sbnfifo);
        bdmf_session_print(session, "sbn_fifo[0] = %u = 0x%x\n", debug_sbnfifo.sbn_fifo[0], debug_sbnfifo.sbn_fifo[0]);
        bdmf_session_print(session, "sbn_fifo[1] = %u = 0x%x\n", debug_sbnfifo.sbn_fifo[1], debug_sbnfifo.sbn_fifo[1]);
        bdmf_session_print(session, "sbn_fifo[2] = %u = 0x%x\n", debug_sbnfifo.sbn_fifo[2], debug_sbnfifo.sbn_fifo[2]);
        bdmf_session_print(session, "sbn_fifo[3] = %u = 0x%x\n", debug_sbnfifo.sbn_fifo[3], debug_sbnfifo.sbn_fifo[3]);
        bdmf_session_print(session, "sbn_fifo[4] = %u = 0x%x\n", debug_sbnfifo.sbn_fifo[4], debug_sbnfifo.sbn_fifo[4]);
        bdmf_session_print(session, "sbn_fifo[5] = %u = 0x%x\n", debug_sbnfifo.sbn_fifo[5], debug_sbnfifo.sbn_fifo[5]);
        bdmf_session_print(session, "sbn_fifo[6] = %u = 0x%x\n", debug_sbnfifo.sbn_fifo[6], debug_sbnfifo.sbn_fifo[6]);
        bdmf_session_print(session, "sbn_fifo[7] = %u = 0x%x\n", debug_sbnfifo.sbn_fifo[7], debug_sbnfifo.sbn_fifo[7]);
        break;
    }
    case cli_bbh_rx_debug_sbnprefetch:
    {
        uint16_t oddbnentry;
        bdmf_boolean oddvalid;
        uint16_t evenbnentry;
        bdmf_boolean evenvalid;
        ag_err = ag_drv_bbh_rx_debug_sbnprefetch_get(parm[1].value.unumber, &oddbnentry, &oddvalid, &evenbnentry, &evenvalid);
        bdmf_session_print(session, "oddbnentry = %u = 0x%x\n", oddbnentry, oddbnentry);
        bdmf_session_print(session, "oddvalid = %u = 0x%x\n", oddvalid, oddvalid);
        bdmf_session_print(session, "evenbnentry = %u = 0x%x\n", evenbnentry, evenbnentry);
        bdmf_session_print(session, "evenvalid = %u = 0x%x\n", evenvalid, evenvalid);
        break;
    }
    case cli_bbh_rx_debug_excfifo:
    {
        bbh_rx_debug_excfifo debug_excfifo;
        ag_err = ag_drv_bbh_rx_debug_excfifo_get(parm[1].value.unumber, parm[2].value.unumber, &debug_excfifo);
        bdmf_session_print(session, "debug_exc_fifo[0] = %u = 0x%x\n", debug_excfifo.debug_exc_fifo[0], debug_excfifo.debug_exc_fifo[0]);
        bdmf_session_print(session, "debug_exc_fifo[1] = %u = 0x%x\n", debug_excfifo.debug_exc_fifo[1], debug_excfifo.debug_exc_fifo[1]);
        bdmf_session_print(session, "debug_exc_fifo[2] = %u = 0x%x\n", debug_excfifo.debug_exc_fifo[2], debug_excfifo.debug_exc_fifo[2]);
        bdmf_session_print(session, "debug_exc_fifo[3] = %u = 0x%x\n", debug_excfifo.debug_exc_fifo[3], debug_excfifo.debug_exc_fifo[3]);
        bdmf_session_print(session, "debug_exc_fifo[4] = %u = 0x%x\n", debug_excfifo.debug_exc_fifo[4], debug_excfifo.debug_exc_fifo[4]);
        bdmf_session_print(session, "debug_exc_fifo[5] = %u = 0x%x\n", debug_excfifo.debug_exc_fifo[5], debug_excfifo.debug_exc_fifo[5]);
        bdmf_session_print(session, "debug_exc_fifo[6] = %u = 0x%x\n", debug_excfifo.debug_exc_fifo[6], debug_excfifo.debug_exc_fifo[6]);
        bdmf_session_print(session, "debug_exc_fifo[7] = %u = 0x%x\n", debug_excfifo.debug_exc_fifo[7], debug_excfifo.debug_exc_fifo[7]);
        break;
    }
    case cli_bbh_rx_bbh_rx_pon_cnpl_debug_gem_out_of_range:
    {
        uint32_t pmvalue;
        ag_err = ag_drv_bbh_rx_bbh_rx_pon_cnpl_debug_gem_out_of_range_get(parm[1].value.unumber, &pmvalue);
        bdmf_session_print(session, "pmvalue = %u = 0x%x\n", pmvalue, pmvalue);
        break;
    }
    case cli_bbh_rx_bbh_rx_pon_cnpl_debug_gem_out_of_range_flow:
    {
        uint16_t flow;
        ag_err = ag_drv_bbh_rx_bbh_rx_pon_cnpl_debug_gem_out_of_range_flow_get(parm[1].value.unumber, &flow);
        bdmf_session_print(session, "flow = %u = 0x%x\n", flow, flow);
        break;
    }
    case cli_bbh_rx_bbh_rx_pon_cnpl_debug_usedwords:
    {
        uint8_t usedwords;
        ag_err = ag_drv_bbh_rx_bbh_rx_pon_cnpl_debug_usedwords_get(parm[1].value.unumber, &usedwords);
        bdmf_session_print(session, "usedwords = %u = 0x%x\n", usedwords, usedwords);
        break;
    }
    case cli_bbh_rx_gem_mode_gemmode:
    {
        bbh_rx_gem_mode_gemmode gem_mode_gemmode;
        ag_err = ag_drv_bbh_rx_gem_mode_gemmode_get(parm[1].value.unumber, parm[2].value.unumber, &gem_mode_gemmode);
        bdmf_session_print(session, "debug_gem_mode[0] = %u = 0x%x\n", gem_mode_gemmode.debug_gem_mode[0], gem_mode_gemmode.debug_gem_mode[0]);
        bdmf_session_print(session, "debug_gem_mode[1] = %u = 0x%x\n", gem_mode_gemmode.debug_gem_mode[1], gem_mode_gemmode.debug_gem_mode[1]);
        bdmf_session_print(session, "debug_gem_mode[2] = %u = 0x%x\n", gem_mode_gemmode.debug_gem_mode[2], gem_mode_gemmode.debug_gem_mode[2]);
        bdmf_session_print(session, "debug_gem_mode[3] = %u = 0x%x\n", gem_mode_gemmode.debug_gem_mode[3], gem_mode_gemmode.debug_gem_mode[3]);
        bdmf_session_print(session, "debug_gem_mode[4] = %u = 0x%x\n", gem_mode_gemmode.debug_gem_mode[4], gem_mode_gemmode.debug_gem_mode[4]);
        bdmf_session_print(session, "debug_gem_mode[5] = %u = 0x%x\n", gem_mode_gemmode.debug_gem_mode[5], gem_mode_gemmode.debug_gem_mode[5]);
        bdmf_session_print(session, "debug_gem_mode[6] = %u = 0x%x\n", gem_mode_gemmode.debug_gem_mode[6], gem_mode_gemmode.debug_gem_mode[6]);
        bdmf_session_print(session, "debug_gem_mode[7] = %u = 0x%x\n", gem_mode_gemmode.debug_gem_mode[7], gem_mode_gemmode.debug_gem_mode[7]);
        break;
    }
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

#ifdef HAL_DRV_TEST_ENABLE
static int ag_drv_bbh_rx_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    uint8_t bbh_id = parm[1].value.unumber;
    bdmf_error_t ag_err = BDMF_ERR_OK;
    uint32_t test_success_cnt = 0;
    uint32_t test_failure_cnt = 0;

    {
        uint8_t index = gtmv(m, 3);
        uint16_t timer = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_bbh_rx_timer_set(%u %u %u)\n", bbh_id, index,
            timer);
        ag_err = ag_drv_bbh_rx_timer_set(bbh_id, index, timer);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_timer_get(bbh_id, index, &timer);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_timer_get(%u %u %u)\n", bbh_id, index,
                timer);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (timer != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t index = gtmv(m, 3);
        bdmf_boolean fcforce = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_bbh_rx_flow_control_force_set(%u %u %u)\n", bbh_id, index,
            fcforce);
        ag_err = ag_drv_bbh_rx_flow_control_force_set(bbh_id, index, fcforce);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_flow_control_force_get(bbh_id, index, &fcforce);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_flow_control_force_get(%u %u %u)\n", bbh_id, index,
                fcforce);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (fcforce != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t index = gtmv(m, 3);
        bdmf_boolean fcrnren = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_bbh_rx_flow_control_runner_enable_set(%u %u %u)\n", bbh_id, index,
            fcrnren);
        ag_err = ag_drv_bbh_rx_flow_control_runner_enable_set(bbh_id, index, fcrnren);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_flow_control_runner_enable_get(bbh_id, index, &fcrnren);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_flow_control_runner_enable_get(%u %u %u)\n", bbh_id, index,
                fcrnren);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (fcrnren != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t index = gtmv(m, 3);
        bdmf_boolean fcqmen = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_bbh_rx_flow_control_qm_enable_set(%u %u %u)\n", bbh_id, index,
            fcqmen);
        ag_err = ag_drv_bbh_rx_flow_control_qm_enable_set(bbh_id, index, fcqmen);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_flow_control_qm_enable_get(bbh_id, index, &fcqmen);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_flow_control_qm_enable_get(%u %u %u)\n", bbh_id, index,
                fcqmen);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (fcqmen != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t index = gtmv(m, 3);
        uint16_t timer = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_bbh_rx_flow_ctrl_timer_set(%u %u %u)\n", bbh_id, index,
            timer);
        ag_err = ag_drv_bbh_rx_flow_ctrl_timer_set(bbh_id, index, timer);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_flow_ctrl_timer_get(bbh_id, index, &timer);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_flow_ctrl_timer_get(%u %u %u)\n", bbh_id, index,
                timer);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (timer != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t index = gtmv(m, 3);
        bdmf_boolean fcforce = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_bbh_rx_flow_ctrl_force_set(%u %u %u)\n", bbh_id, index,
            fcforce);
        ag_err = ag_drv_bbh_rx_flow_ctrl_force_set(bbh_id, index, fcforce);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_flow_ctrl_force_get(bbh_id, index, &fcforce);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_flow_ctrl_force_get(%u %u %u)\n", bbh_id, index,
                fcforce);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (fcforce != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t index = gtmv(m, 3);
        bdmf_boolean fcrnren = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_bbh_rx_flow_ctrl_rnr_en_set(%u %u %u)\n", bbh_id, index,
            fcrnren);
        ag_err = ag_drv_bbh_rx_flow_ctrl_rnr_en_set(bbh_id, index, fcrnren);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_flow_ctrl_rnr_en_get(bbh_id, index, &fcrnren);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_flow_ctrl_rnr_en_get(%u %u %u)\n", bbh_id, index,
                fcrnren);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (fcrnren != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t dispbbid = gtmv(m, 9);
        uint16_t sbpmbbid = gtmv(m, 9);
        bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_bbcfg_set(%u %u %u)\n", bbh_id,
            dispbbid, sbpmbbid);
        ag_err = ag_drv_bbh_rx_general_configuration_bbcfg_set(bbh_id, dispbbid, sbpmbbid);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_general_configuration_bbcfg_get(bbh_id, &dispbbid, &sbpmbbid);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_bbcfg_get(%u %u %u)\n", bbh_id,
                dispbbid, sbpmbbid);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (dispbbid != gtmv(m, 9) || sbpmbbid != gtmv(m, 9))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t pkten = gtmv(m, 8);
        bdmf_boolean sbpmen = gtmv(m, 1);
        bdmf_boolean geminit = gtmv(m, 1);
        bdmf_boolean grace_dis = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_enable_set(%u %u %u %u %u)\n", bbh_id,
            pkten, sbpmen, geminit, grace_dis);
        ag_err = ag_drv_bbh_rx_general_configuration_enable_set(bbh_id, pkten, sbpmen, geminit, grace_dis);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_general_configuration_enable_get(bbh_id, &pkten, &sbpmen, &geminit, &grace_dis);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_enable_get(%u %u %u %u %u)\n", bbh_id,
                pkten, sbpmen, geminit, grace_dis);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (pkten != gtmv(m, 8) || sbpmen != gtmv(m, 1) || geminit != gtmv(m, 1) || grace_dis != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t minpkt_normal = gtmv(m, 8);
        uint8_t minpkt_omci = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_minpkt_set(%u %u %u)\n", bbh_id,
            minpkt_normal, minpkt_omci);
        ag_err = ag_drv_bbh_rx_general_configuration_minpkt_set(bbh_id, minpkt_normal, minpkt_omci);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_general_configuration_minpkt_get(bbh_id, &minpkt_normal, &minpkt_omci);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_minpkt_get(%u %u %u)\n", bbh_id,
                minpkt_normal, minpkt_omci);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (minpkt_normal != gtmv(m, 8) || minpkt_omci != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t maxpkt = gtmv(m, 14);
        bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_maxpkt_set(%u %u)\n", bbh_id,
            maxpkt);
        ag_err = ag_drv_bbh_rx_general_configuration_maxpkt_set(bbh_id, maxpkt);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_general_configuration_maxpkt_get(bbh_id, &maxpkt);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_maxpkt_get(%u %u)\n", bbh_id,
                maxpkt);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (maxpkt != gtmv(m, 14))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bbh_rx_general_configuration_clk_gate_cntrl general_configuration_clk_gate_cntrl = {.bypass_clk_gate = gtmv(m, 1), .timer_val = gtmv(m, 8), .keep_alive_en = gtmv(m, 1), .keep_alive_intrvl = gtmv(m, 3), .keep_alive_cyc = gtmv(m, 8)};
        bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_clk_gate_cntrl_set(%u %u %u %u %u %u)\n", bbh_id,
            general_configuration_clk_gate_cntrl.bypass_clk_gate, general_configuration_clk_gate_cntrl.timer_val, general_configuration_clk_gate_cntrl.keep_alive_en, general_configuration_clk_gate_cntrl.keep_alive_intrvl, 
            general_configuration_clk_gate_cntrl.keep_alive_cyc);
        ag_err = ag_drv_bbh_rx_general_configuration_clk_gate_cntrl_set(bbh_id, &general_configuration_clk_gate_cntrl);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_general_configuration_clk_gate_cntrl_get(bbh_id, &general_configuration_clk_gate_cntrl);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_clk_gate_cntrl_get(%u %u %u %u %u %u)\n", bbh_id,
                general_configuration_clk_gate_cntrl.bypass_clk_gate, general_configuration_clk_gate_cntrl.timer_val, general_configuration_clk_gate_cntrl.keep_alive_en, general_configuration_clk_gate_cntrl.keep_alive_intrvl, 
                general_configuration_clk_gate_cntrl.keep_alive_cyc);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (general_configuration_clk_gate_cntrl.bypass_clk_gate != gtmv(m, 1) || general_configuration_clk_gate_cntrl.timer_val != gtmv(m, 8) || general_configuration_clk_gate_cntrl.keep_alive_en != gtmv(m, 1) || general_configuration_clk_gate_cntrl.keep_alive_intrvl != gtmv(m, 3) || general_configuration_clk_gate_cntrl.keep_alive_cyc != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t rxdbgsel = gtmv(m, 5);
        bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_rxdbgsel_set(%u %u)\n", bbh_id,
            rxdbgsel);
        ag_err = ag_drv_bbh_rx_general_configuration_rxdbgsel_set(bbh_id, rxdbgsel);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_general_configuration_rxdbgsel_get(bbh_id, &rxdbgsel);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_rxdbgsel_get(%u %u)\n", bbh_id,
                rxdbgsel);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (rxdbgsel != gtmv(m, 5))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bbh_rx_general_configuration_rxrstrst general_configuration_rxrstrst = {.portrst = gtmv(m, 1), .psramrst = gtmv(m, 1), .pdfiforst = gtmv(m, 1), .sbpmfiforst = gtmv(m, 1), .dispcreditrst = gtmv(m, 1), .upldfiforst = gtmv(m, 1), .pfbch = gtmv(m, 8), .dispcredit = gtmv(m, 7), .dispviq = gtmv(m, 9)};
        bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_rxrstrst_set(%u %u %u %u %u %u %u %u %u %u)\n", bbh_id,
            general_configuration_rxrstrst.portrst, general_configuration_rxrstrst.psramrst, general_configuration_rxrstrst.pdfiforst, general_configuration_rxrstrst.sbpmfiforst, 
            general_configuration_rxrstrst.dispcreditrst, general_configuration_rxrstrst.upldfiforst, general_configuration_rxrstrst.pfbch, general_configuration_rxrstrst.dispcredit, 
            general_configuration_rxrstrst.dispviq);
        ag_err = ag_drv_bbh_rx_general_configuration_rxrstrst_set(bbh_id, &general_configuration_rxrstrst);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_general_configuration_rxrstrst_get(bbh_id, &general_configuration_rxrstrst);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_rxrstrst_get(%u %u %u %u %u %u %u %u %u %u)\n", bbh_id,
                general_configuration_rxrstrst.portrst, general_configuration_rxrstrst.psramrst, general_configuration_rxrstrst.pdfiforst, general_configuration_rxrstrst.sbpmfiforst, 
                general_configuration_rxrstrst.dispcreditrst, general_configuration_rxrstrst.upldfiforst, general_configuration_rxrstrst.pfbch, general_configuration_rxrstrst.dispcredit, 
                general_configuration_rxrstrst.dispviq);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (general_configuration_rxrstrst.portrst != gtmv(m, 1) || general_configuration_rxrstrst.psramrst != gtmv(m, 1) || general_configuration_rxrstrst.pdfiforst != gtmv(m, 1) || general_configuration_rxrstrst.sbpmfiforst != gtmv(m, 1) || general_configuration_rxrstrst.dispcreditrst != gtmv(m, 1) || general_configuration_rxrstrst.upldfiforst != gtmv(m, 1) || general_configuration_rxrstrst.pfbch != gtmv(m, 8) || general_configuration_rxrstrst.dispcredit != gtmv(m, 7) || general_configuration_rxrstrst.dispviq != gtmv(m, 9))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t max_otf_sbpm_req = gtmv(m, 6);
        bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_sbpmcfg_set(%u %u)\n", bbh_id,
            max_otf_sbpm_req);
        ag_err = ag_drv_bbh_rx_general_configuration_sbpmcfg_set(bbh_id, max_otf_sbpm_req);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_general_configuration_sbpmcfg_get(bbh_id, &max_otf_sbpm_req);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_sbpmcfg_get(%u %u)\n", bbh_id,
                max_otf_sbpm_req);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (max_otf_sbpm_req != gtmv(m, 6))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t psrambase = gtmv(m, 20);
        bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_psrambase_set(%u %u)\n", bbh_id,
            psrambase);
        ag_err = ag_drv_bbh_rx_general_configuration_psrambase_set(bbh_id, psrambase);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_general_configuration_psrambase_get(bbh_id, &psrambase);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_psrambase_get(%u %u)\n", bbh_id,
                psrambase);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (psrambase != gtmv(m, 20))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t ingresschannel = gtmv(m, 4);
        bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_ingresschannel_set(%u %u)\n", bbh_id,
            ingresschannel);
        ag_err = ag_drv_bbh_rx_general_configuration_ingresschannel_set(bbh_id, ingresschannel);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_general_configuration_ingresschannel_get(bbh_id, &ingresschannel);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_ingresschannel_get(%u %u)\n", bbh_id,
                ingresschannel);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ingresschannel != gtmv(m, 4))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t route = gtmv(m, 25);
        bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_bbovrdroute_set(%u %u)\n", bbh_id,
            route);
        ag_err = ag_drv_bbh_rx_general_configuration_bbovrdroute_set(bbh_id, route);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_general_configuration_bbovrdroute_get(bbh_id, &route);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_bbovrdroute_get(%u %u)\n", bbh_id,
                route);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (route != gtmv(m, 25))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t dest = gtmv(m, 9);
        bdmf_boolean en = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_bbovrdctrl_set(%u %u %u)\n", bbh_id,
            dest, en);
        ag_err = ag_drv_bbh_rx_general_configuration_bbovrdctrl_set(bbh_id, dest, en);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_general_configuration_bbovrdctrl_get(bbh_id, &dest, &en);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_bbovrdctrl_get(%u %u %u)\n", bbh_id,
                dest, en);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (dest != gtmv(m, 9) || en != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean gemdone = gtmv(m, 1);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_general_configuration_gemdone_get(bbh_id, &gemdone);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_gemdone_get(%u %u)\n", bbh_id,
                gemdone);
        }
    }

    {
        uint8_t index = gtmv(m, 3);
        uint8_t mactype = gtmv(m, 4);
        bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_mactype_set(%u %u %u)\n", bbh_id, index,
            mactype);
        ag_err = ag_drv_bbh_rx_general_configuration_mactype_set(bbh_id, index, mactype);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_general_configuration_mactype_get(bbh_id, index, &mactype);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_mactype_get(%u %u %u)\n", bbh_id, index,
                mactype);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (mactype != gtmv(m, 4))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t index = gtmv(m, 3);
        uint8_t portthr = gtmv(m, 6);
        bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_portth_set(%u %u %u)\n", bbh_id, index,
            portthr);
        ag_err = ag_drv_bbh_rx_general_configuration_portth_set(bbh_id, index, portthr);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_general_configuration_portth_get(bbh_id, index, &portthr);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_portth_get(%u %u %u)\n", bbh_id, index,
                portthr);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (portthr != gtmv(m, 6))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t index = gtmv(m, 3);
        uint8_t sopoffset = gtmv(m, 7);
        bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_sopoffset_set(%u %u %u)\n", bbh_id, index,
            sopoffset);
        ag_err = ag_drv_bbh_rx_general_configuration_sopoffset_set(bbh_id, index, sopoffset);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_general_configuration_sopoffset_get(bbh_id, index, &sopoffset);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_sopoffset_get(%u %u %u)\n", bbh_id, index,
                sopoffset);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (sopoffset != gtmv(m, 7))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t channel_num = gtmv(m, 3);
        uint8_t ingresspid = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_ingresspid_set(%u %u %u)\n", bbh_id, channel_num,
            ingresspid);
        ag_err = ag_drv_bbh_rx_general_configuration_ingresspid_set(bbh_id, channel_num, ingresspid);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_general_configuration_ingresspid_get(bbh_id, channel_num, &ingresspid);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_ingresspid_get(%u %u %u)\n", bbh_id, channel_num,
                ingresspid);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ingresspid != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t index = gtmv(m, 3);
        uint8_t portweight = gtmv(m, 3);
        bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_portweight_set(%u %u %u)\n", bbh_id, index,
            portweight);
        ag_err = ag_drv_bbh_rx_general_configuration_portweight_set(bbh_id, index, portweight);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_general_configuration_portweight_get(bbh_id, index, &portweight);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_portweight_get(%u %u %u)\n", bbh_id, index,
                portweight);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (portweight != gtmv(m, 3))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t viq_index = gtmv(m, 3);
        uint16_t portviqmask = gtmv(m, 9);
        bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_portviqmask_set(%u %u %u)\n", bbh_id, viq_index,
            portviqmask);
        ag_err = ag_drv_bbh_rx_general_configuration_portviqmask_set(bbh_id, viq_index, portviqmask);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_general_configuration_portviqmask_get(bbh_id, viq_index, &portviqmask);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_portviqmask_get(%u %u %u)\n", bbh_id, viq_index,
                portviqmask);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (portviqmask != gtmv(m, 9))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t viq_index = gtmv(m, 3);
        uint16_t excviqmask = gtmv(m, 9);
        bdmf_boolean excviqen = gtmv(m, 1);
        bdmf_boolean excviqoamen = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_excviqmask_set(%u %u %u %u %u)\n", bbh_id, viq_index,
            excviqmask, excviqen, excviqoamen);
        ag_err = ag_drv_bbh_rx_general_configuration_excviqmask_set(bbh_id, viq_index, excviqmask, excviqen, excviqoamen);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_general_configuration_excviqmask_get(bbh_id, viq_index, &excviqmask, &excviqen, &excviqoamen);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_excviqmask_get(%u %u %u %u %u)\n", bbh_id, viq_index,
                excviqmask, excviqen, excviqoamen);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (excviqmask != gtmv(m, 9) || excviqen != gtmv(m, 1) || excviqoamen != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t viq_index = gtmv(m, 3);
        uint8_t viq = gtmv(m, 4);
        bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_dispviq_set(%u %u %u)\n", bbh_id, viq_index,
            viq);
        ag_err = ag_drv_bbh_rx_general_configuration_dispviq_set(bbh_id, viq_index, viq);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_general_configuration_dispviq_get(bbh_id, viq_index, &viq);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_dispviq_get(%u %u %u)\n", bbh_id, viq_index,
                viq);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (viq != gtmv(m, 4))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t index = gtmv(m, 3);
        uint8_t viqweight = gtmv(m, 3);
        bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_viqweight_set(%u %u %u)\n", bbh_id, index,
            viqweight);
        ag_err = ag_drv_bbh_rx_general_configuration_viqweight_set(bbh_id, index, viqweight);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_general_configuration_viqweight_get(bbh_id, index, &viqweight);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_viqweight_get(%u %u %u)\n", bbh_id, index,
                viqweight);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (viqweight != gtmv(m, 3))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t viq_index = gtmv(m, 3);
        uint8_t location = gtmv(m, 5);
        bdmf_boolean no_drop_en = gtmv(m, 1);
        bdmf_boolean viqclassen = gtmv(m, 1);
        uint16_t viq = gtmv(m, 9);
        bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_patterncfg_set(%u %u %u %u %u %u)\n", bbh_id, viq_index,
            location, no_drop_en, viqclassen, viq);
        ag_err = ag_drv_bbh_rx_general_configuration_patterncfg_set(bbh_id, viq_index, location, no_drop_en, viqclassen, viq);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_general_configuration_patterncfg_get(bbh_id, viq_index, &location, &no_drop_en, &viqclassen, &viq);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_patterncfg_get(%u %u %u %u %u %u)\n", bbh_id, viq_index,
                location, no_drop_en, viqclassen, viq);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (location != gtmv(m, 5) || no_drop_en != gtmv(m, 1) || viqclassen != gtmv(m, 1) || viq != gtmv(m, 9))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t zero = gtmv(m, 1);
        bbh_rx_general_configuration_patterndata general_configuration_patterndata = {.pattern_data_config = {gtmv(m, 32), gtmv(m, 32)}};
        bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_patterndata_set(%u %u %u %u)\n", bbh_id, zero,
            general_configuration_patterndata.pattern_data_config[0], general_configuration_patterndata.pattern_data_config[1]);
        ag_err = ag_drv_bbh_rx_general_configuration_patterndata_set(bbh_id, zero, &general_configuration_patterndata);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_general_configuration_patterndata_get(bbh_id, zero, &general_configuration_patterndata);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_patterndata_get(%u %u %u %u)\n", bbh_id, zero,
                general_configuration_patterndata.pattern_data_config[0], general_configuration_patterndata.pattern_data_config[1]);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (general_configuration_patterndata.pattern_data_config[0] != gtmv(m, 32) || general_configuration_patterndata.pattern_data_config[1] != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t index = gtmv(m, 2);
        uint32_t patternmask = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_patternmask_set(%u %u %u)\n", bbh_id, index,
            patternmask);
        ag_err = ag_drv_bbh_rx_general_configuration_patternmask_set(bbh_id, index, patternmask);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_general_configuration_patternmask_get(bbh_id, index, &patternmask);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_patternmask_get(%u %u %u)\n", bbh_id, index,
                patternmask);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (patternmask != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t viq_index = gtmv(m, 2);
        uint32_t patterndata = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_patterndata8b_set(%u %u %u)\n", bbh_id, viq_index,
            patterndata);
        ag_err = ag_drv_bbh_rx_general_configuration_patterndata8b_set(bbh_id, viq_index, patterndata);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_general_configuration_patterndata8b_get(bbh_id, viq_index, &patterndata);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_patterndata8b_get(%u %u %u)\n", bbh_id, viq_index,
                patterndata);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (patterndata != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t viq_index = gtmv(m, 2);
        uint32_t patternmask = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_patternmask8b_set(%u %u %u)\n", bbh_id, viq_index,
            patternmask);
        ag_err = ag_drv_bbh_rx_general_configuration_patternmask8b_set(bbh_id, viq_index, patternmask);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_general_configuration_patternmask8b_get(bbh_id, viq_index, &patternmask);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_patternmask8b_get(%u %u %u)\n", bbh_id, viq_index,
                patternmask);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (patternmask != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t viq_index = gtmv(m, 3);
        uint8_t patternen = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_patternen_set(%u %u %u)\n", bbh_id, viq_index,
            patternen);
        ag_err = ag_drv_bbh_rx_general_configuration_patternen_set(bbh_id, viq_index, patternen);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_general_configuration_patternen_get(bbh_id, viq_index, &patternen);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_patternen_get(%u %u %u)\n", bbh_id, viq_index,
                patternen);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (patternen != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t viq_index = gtmv(m, 3);
        uint16_t viqfcen = gtmv(m, 9);
        bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_viqfcen_set(%u %u %u)\n", bbh_id, viq_index,
            viqfcen);
        ag_err = ag_drv_bbh_rx_general_configuration_viqfcen_set(bbh_id, viq_index, viqfcen);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_general_configuration_viqfcen_get(bbh_id, viq_index, &viqfcen);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_viqfcen_get(%u %u %u)\n", bbh_id, viq_index,
                viqfcen);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (viqfcen != gtmv(m, 9))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t viq_index = gtmv(m, 3);
        uint16_t flowid = gtmv(m, 13);
        bdmf_boolean enable = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_noneth_set(%u %u %u %u)\n", bbh_id, viq_index,
            flowid, enable);
        ag_err = ag_drv_bbh_rx_general_configuration_noneth_set(bbh_id, viq_index, flowid, enable);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_general_configuration_noneth_get(bbh_id, viq_index, &flowid, &enable);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_noneth_get(%u %u %u %u)\n", bbh_id, viq_index,
                flowid, enable);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (flowid != gtmv(m, 13) || enable != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t zero = gtmv(m, 2);
        bbh_rx_pm_counters_inpkt pm_counters_inpkt = {.pm_counters_inpkt = {gtmv(m, 32), gtmv(m, 32)}};
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_pm_counters_inpkt_get(bbh_id, zero, &pm_counters_inpkt);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_pm_counters_inpkt_get(%u %u %u %u)\n", bbh_id, zero,
                pm_counters_inpkt.pm_counters_inpkt[0], pm_counters_inpkt.pm_counters_inpkt[1]);
        }
    }

    {
        uint32_t zero = gtmv(m, 2);
        bbh_rx_pm_counters_inbyte pm_counters_inbyte = {.pm_counters_inbyte = {gtmv(m, 32), gtmv(m, 32)}};
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_pm_counters_inbyte_get(bbh_id, zero, &pm_counters_inbyte);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_pm_counters_inbyte_get(%u %u %u %u)\n", bbh_id, zero,
                pm_counters_inbyte.pm_counters_inbyte[0], pm_counters_inbyte.pm_counters_inbyte[1]);
        }
    }

    {
        uint32_t zero = gtmv(m, 2);
        bbh_rx_pm_counters_tooshort pm_counters_tooshort = {.pm_counters_too_short = {gtmv(m, 32), gtmv(m, 32)}};
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_pm_counters_tooshort_get(bbh_id, zero, &pm_counters_tooshort);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_pm_counters_tooshort_get(%u %u %u %u)\n", bbh_id, zero,
                pm_counters_tooshort.pm_counters_too_short[0], pm_counters_tooshort.pm_counters_too_short[1]);
        }
    }

    {
        uint32_t zero = gtmv(m, 2);
        bbh_rx_pm_counters_toolong pm_counters_toolong = {.pm_counters_too_long = {gtmv(m, 32), gtmv(m, 32)}};
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_pm_counters_toolong_get(bbh_id, zero, &pm_counters_toolong);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_pm_counters_toolong_get(%u %u %u %u)\n", bbh_id, zero,
                pm_counters_toolong.pm_counters_too_long[0], pm_counters_toolong.pm_counters_too_long[1]);
        }
    }

    {
        uint32_t zero = gtmv(m, 2);
        bbh_rx_pm_counters_crcerror pm_counters_crcerror = {.pm_counters_crc_error = {gtmv(m, 32), gtmv(m, 32)}};
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_pm_counters_crcerror_get(bbh_id, zero, &pm_counters_crcerror);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_pm_counters_crcerror_get(%u %u %u %u)\n", bbh_id, zero,
                pm_counters_crcerror.pm_counters_crc_error[0], pm_counters_crcerror.pm_counters_crc_error[1]);
        }
    }

    {
        uint32_t zero = gtmv(m, 2);
        bbh_rx_pm_counters_fcdrop pm_counters_fcdrop = {.pm_counters_fc_drop = {gtmv(m, 32), gtmv(m, 32)}};
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_pm_counters_fcdrop_get(bbh_id, zero, &pm_counters_fcdrop);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_pm_counters_fcdrop_get(%u %u %u %u)\n", bbh_id, zero,
                pm_counters_fcdrop.pm_counters_fc_drop[0], pm_counters_fcdrop.pm_counters_fc_drop[1]);
        }
    }

    {
        uint32_t zero = gtmv(m, 2);
        bbh_rx_pm_counters_truncate pm_counters_truncate = {.pm_counters_truncate = {gtmv(m, 32), gtmv(m, 32)}};
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_pm_counters_truncate_get(bbh_id, zero, &pm_counters_truncate);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_pm_counters_truncate_get(%u %u %u %u)\n", bbh_id, zero,
                pm_counters_truncate.pm_counters_truncate[0], pm_counters_truncate.pm_counters_truncate[1]);
        }
    }

    {
        uint32_t zero = gtmv(m, 2);
        bbh_rx_pm_counters_totaldrop pm_counters_totaldrop = {.pm_counters_total_drop = {gtmv(m, 32), gtmv(m, 32)}};
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_pm_counters_totaldrop_get(bbh_id, zero, &pm_counters_totaldrop);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_pm_counters_totaldrop_get(%u %u %u %u)\n", bbh_id, zero,
                pm_counters_totaldrop.pm_counters_total_drop[0], pm_counters_totaldrop.pm_counters_total_drop[1]);
        }
    }

    {
        uint32_t pmvalue = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_pm_counters_dummypdcnt_get(bbh_id, &pmvalue);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_pm_counters_dummypdcnt_get(%u %u)\n", bbh_id,
                pmvalue);
        }
    }

    {
        uint32_t r64 = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_pon_rmon_counters_r64_get(bbh_id, &r64);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_pon_rmon_counters_r64_get(%u %u)\n", bbh_id,
                r64);
        }
    }

    {
        uint32_t r127 = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_pon_rmon_counters_r127_get(bbh_id, &r127);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_pon_rmon_counters_r127_get(%u %u)\n", bbh_id,
                r127);
        }
    }

    {
        uint32_t r255 = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_pon_rmon_counters_r255_get(bbh_id, &r255);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_pon_rmon_counters_r255_get(%u %u)\n", bbh_id,
                r255);
        }
    }

    {
        uint32_t r511 = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_pon_rmon_counters_r511_get(bbh_id, &r511);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_pon_rmon_counters_r511_get(%u %u)\n", bbh_id,
                r511);
        }
    }

    {
        uint32_t r1023 = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_pon_rmon_counters_r1023_get(bbh_id, &r1023);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_pon_rmon_counters_r1023_get(%u %u)\n", bbh_id,
                r1023);
        }
    }

    {
        uint32_t r1518 = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_pon_rmon_counters_r1518_get(bbh_id, &r1518);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_pon_rmon_counters_r1518_get(%u %u)\n", bbh_id,
                r1518);
        }
    }

    {
        uint32_t r1518 = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_pon_rmon_counters_r2047_get(bbh_id, &r1518);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_pon_rmon_counters_r2047_get(%u %u)\n", bbh_id,
                r1518);
        }
    }

    {
        uint32_t r4095 = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_pon_rmon_counters_r4095_get(bbh_id, &r4095);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_pon_rmon_counters_r4095_get(%u %u)\n", bbh_id,
                r4095);
        }
    }

    {
        uint32_t r9216 = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_pon_rmon_counters_r9216_get(bbh_id, &r9216);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_pon_rmon_counters_r9216_get(%u %u)\n", bbh_id,
                r9216);
        }
    }

    {
        uint32_t r16383 = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_pon_rmon_counters_r16383_get(bbh_id, &r16383);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_pon_rmon_counters_r16383_get(%u %u)\n", bbh_id,
                r16383);
        }
    }

    {
        uint8_t zero = gtmv(m, 0);
        bbh_rx_debug_pfbuw debug_pfbuw = {.debug_pfbuw = {gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_debug_pfbuw_get(bbh_id, zero, &debug_pfbuw);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_debug_pfbuw_get(%u %u %u %u %u %u %u %u %u %u)\n", bbh_id, zero,
                debug_pfbuw.debug_pfbuw[0], debug_pfbuw.debug_pfbuw[1], debug_pfbuw.debug_pfbuw[2], debug_pfbuw.debug_pfbuw[3], 
                debug_pfbuw.debug_pfbuw[4], debug_pfbuw.debug_pfbuw[5], debug_pfbuw.debug_pfbuw[6], debug_pfbuw.debug_pfbuw[7]);
        }
    }

    {
        uint8_t zero = gtmv(m, 0);
        bbh_rx_debug_patternuw debug_patternuw = {.debug_patternuw = {gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_debug_patternuw_get(bbh_id, zero, &debug_patternuw);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_debug_patternuw_get(%u %u %u %u %u %u %u %u %u %u)\n", bbh_id, zero,
                debug_patternuw.debug_patternuw[0], debug_patternuw.debug_patternuw[1], debug_patternuw.debug_patternuw[2], debug_patternuw.debug_patternuw[3], 
                debug_patternuw.debug_patternuw[4], debug_patternuw.debug_patternuw[5], debug_patternuw.debug_patternuw[6], debug_patternuw.debug_patternuw[7]);
        }
    }

    {
        uint8_t zero = gtmv(m, 0);
        bbh_rx_debug_excuw debug_excuw = {.debug_excuw = {gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_debug_excuw_get(bbh_id, zero, &debug_excuw);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_debug_excuw_get(%u %u %u %u %u %u %u %u %u %u)\n", bbh_id, zero,
                debug_excuw.debug_excuw[0], debug_excuw.debug_excuw[1], debug_excuw.debug_excuw[2], debug_excuw.debug_excuw[3], 
                debug_excuw.debug_excuw[4], debug_excuw.debug_excuw[5], debug_excuw.debug_excuw[6], debug_excuw.debug_excuw[7]);
        }
    }

    {
        uint8_t zero = gtmv(m, 0);
        bbh_rx_debug_portuw debug_portuw = {.debug_portuw = {gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_debug_portuw_get(bbh_id, zero, &debug_portuw);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_debug_portuw_get(%u %u %u %u %u %u %u %u %u %u)\n", bbh_id, zero,
                debug_portuw.debug_portuw[0], debug_portuw.debug_portuw[1], debug_portuw.debug_portuw[2], debug_portuw.debug_portuw[3], 
                debug_portuw.debug_portuw[4], debug_portuw.debug_portuw[5], debug_portuw.debug_portuw[6], debug_portuw.debug_portuw[7]);
        }
    }

    {
        uint8_t zero = gtmv(m, 0);
        bbh_rx_debug_firstsbn debug_firstsbn = {.debug_first_sbn = {gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_debug_firstsbn_get(bbh_id, zero, &debug_firstsbn);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_debug_firstsbn_get(%u %u %u %u %u %u %u %u %u %u)\n", bbh_id, zero,
                debug_firstsbn.debug_first_sbn[0], debug_firstsbn.debug_first_sbn[1], debug_firstsbn.debug_first_sbn[2], debug_firstsbn.debug_first_sbn[3], 
                debug_firstsbn.debug_first_sbn[4], debug_firstsbn.debug_first_sbn[5], debug_firstsbn.debug_first_sbn[6], debug_firstsbn.debug_first_sbn[7]);
        }
    }

    {
        uint8_t zero = gtmv(m, 0);
        bbh_rx_debug_currentsbn debug_currentsbn = {.debug_current_sbn = {gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_debug_currentsbn_get(bbh_id, zero, &debug_currentsbn);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_debug_currentsbn_get(%u %u %u %u %u %u %u %u %u %u)\n", bbh_id, zero,
                debug_currentsbn.debug_current_sbn[0], debug_currentsbn.debug_current_sbn[1], debug_currentsbn.debug_current_sbn[2], debug_currentsbn.debug_current_sbn[3], 
                debug_currentsbn.debug_current_sbn[4], debug_currentsbn.debug_current_sbn[5], debug_currentsbn.debug_current_sbn[6], debug_currentsbn.debug_current_sbn[7]);
        }
    }

    {
        uint8_t zero = gtmv(m, 0);
        bbh_rx_debug_sn debug_sn = {.debug_sn = {gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_debug_sn_get(bbh_id, zero, &debug_sn);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_debug_sn_get(%u %u %u %u %u %u %u %u %u %u)\n", bbh_id, zero,
                debug_sn.debug_sn[0], debug_sn.debug_sn[1], debug_sn.debug_sn[2], debug_sn.debug_sn[3], 
                debug_sn.debug_sn[4], debug_sn.debug_sn[5], debug_sn.debug_sn[6], debug_sn.debug_sn[7]);
        }
    }

    {
        uint8_t zero = gtmv(m, 0);
        bbh_rx_debug_viqmask debug_viqmask = {.debug_viq_mask = {gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_debug_viqmask_get(bbh_id, zero, &debug_viqmask);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_debug_viqmask_get(%u %u %u %u %u %u %u %u %u %u)\n", bbh_id, zero,
                debug_viqmask.debug_viq_mask[0], debug_viqmask.debug_viq_mask[1], debug_viqmask.debug_viq_mask[2], debug_viqmask.debug_viq_mask[3], 
                debug_viqmask.debug_viq_mask[4], debug_viqmask.debug_viq_mask[5], debug_viqmask.debug_viq_mask[6], debug_viqmask.debug_viq_mask[7]);
        }
    }

    {
        uint8_t zero = gtmv(m, 0);
        bbh_rx_debug_plen debug_plen = {.debug_plen = {gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_debug_plen_get(bbh_id, zero, &debug_plen);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_debug_plen_get(%u %u %u %u %u %u %u %u %u %u)\n", bbh_id, zero,
                debug_plen.debug_plen[0], debug_plen.debug_plen[1], debug_plen.debug_plen[2], debug_plen.debug_plen[3], 
                debug_plen.debug_plen[4], debug_plen.debug_plen[5], debug_plen.debug_plen[6], debug_plen.debug_plen[7]);
        }
    }

    {
        uint8_t context = gtmv(m, 8);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_debug_eopok_get(bbh_id, &context);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_debug_eopok_get(%u %u)\n", bbh_id,
                context);
        }
    }

    {
        uint8_t context = gtmv(m, 8);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_debug_viqfull_get(bbh_id, &context);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_debug_viqfull_get(%u %u)\n", bbh_id,
                context);
        }
    }

    {
        uint8_t context = gtmv(m, 8);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_debug_portreq_get(bbh_id, &context);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_debug_portreq_get(%u %u)\n", bbh_id,
                context);
        }
    }

    {
        uint8_t context = gtmv(m, 8);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_debug_dummyreq_get(bbh_id, &context);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_debug_dummyreq_get(%u %u)\n", bbh_id,
                context);
        }
    }

    {
        uint8_t context = gtmv(m, 8);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_debug_inreassembly_get(bbh_id, &context);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_debug_inreassembly_get(%u %u)\n", bbh_id,
                context);
        }
    }

    {
        uint8_t context = gtmv(m, 8);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_debug_dummypd_get(bbh_id, &context);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_debug_dummypd_get(%u %u)\n", bbh_id,
                context);
        }
    }

    {
        uint8_t uw = gtmv(m, 3);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_debug_ufuw_get(bbh_id, &uw);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_debug_ufuw_get(%u %u)\n", bbh_id,
                uw);
        }
    }

    {
        uint8_t uw = gtmv(m, 3);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_debug_pendconnectuw_get(bbh_id, &uw);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_debug_pendconnectuw_get(%u %u)\n", bbh_id,
                uw);
        }
    }

    {
        uint8_t zero = gtmv(m, 1);
        bbh_rx_debug_dbgvec debug_dbgvec = {.debug_dbg_veq = {gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_debug_dbgvec_get(bbh_id, zero, &debug_dbgvec);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_debug_dbgvec_get(%u %u %u %u %u %u %u %u %u %u)\n", bbh_id, zero,
                debug_dbgvec.debug_dbg_veq[0], debug_dbgvec.debug_dbg_veq[1], debug_dbgvec.debug_dbg_veq[2], debug_dbgvec.debug_dbg_veq[3], 
                debug_dbgvec.debug_dbg_veq[4], debug_dbgvec.debug_dbg_veq[5], debug_dbgvec.debug_dbg_veq[6], debug_dbgvec.debug_dbg_veq[7]);
        }
    }

    {
        uint8_t zero = gtmv(m, 0);
        bbh_rx_debug_pduw debug_pduw = {.debug_pduw = {gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_debug_pduw_get(bbh_id, zero, &debug_pduw);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_debug_pduw_get(%u %u %u %u %u %u %u %u %u %u)\n", bbh_id, zero,
                debug_pduw.debug_pduw[0], debug_pduw.debug_pduw[1], debug_pduw.debug_pduw[2], debug_pduw.debug_pduw[3], 
                debug_pduw.debug_pduw[4], debug_pduw.debug_pduw[5], debug_pduw.debug_pduw[6], debug_pduw.debug_pduw[7]);
        }
    }

    {
        uint8_t zero = gtmv(m, 0);
        bbh_rx_debug_sbpmack debug_sbpmack = {.debug_sbpm_ack = {gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_debug_sbpmack_get(bbh_id, zero, &debug_sbpmack);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_debug_sbpmack_get(%u %u %u %u %u %u %u %u %u %u)\n", bbh_id, zero,
                debug_sbpmack.debug_sbpm_ack[0], debug_sbpmack.debug_sbpm_ack[1], debug_sbpmack.debug_sbpm_ack[2], debug_sbpmack.debug_sbpm_ack[3], 
                debug_sbpmack.debug_sbpm_ack[4], debug_sbpmack.debug_sbpm_ack[5], debug_sbpmack.debug_sbpm_ack[6], debug_sbpmack.debug_sbpm_ack[7]);
        }
    }

    {
        uint8_t zero = gtmv(m, 0);
        bbh_rx_debug_writereplyack debug_writereplyack = {.debug_write_replay_ack = {gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_debug_writereplyack_get(bbh_id, zero, &debug_writereplyack);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_debug_writereplyack_get(%u %u %u %u %u %u %u %u %u %u)\n", bbh_id, zero,
                debug_writereplyack.debug_write_replay_ack[0], debug_writereplyack.debug_write_replay_ack[1], debug_writereplyack.debug_write_replay_ack[2], debug_writereplyack.debug_write_replay_ack[3], 
                debug_writereplyack.debug_write_replay_ack[4], debug_writereplyack.debug_write_replay_ack[5], debug_writereplyack.debug_write_replay_ack[6], debug_writereplyack.debug_write_replay_ack[7]);
        }
    }

    {
        uint8_t zero = gtmv(m, 0);
        bbh_rx_debug_dispcreditack debug_dispcreditack = {.debug_disp_credit_ack = {gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_debug_dispcreditack_get(bbh_id, zero, &debug_dispcreditack);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_debug_dispcreditack_get(%u %u %u %u %u %u %u %u %u %u)\n", bbh_id, zero,
                debug_dispcreditack.debug_disp_credit_ack[0], debug_dispcreditack.debug_disp_credit_ack[1], debug_dispcreditack.debug_disp_credit_ack[2], debug_dispcreditack.debug_disp_credit_ack[3], 
                debug_dispcreditack.debug_disp_credit_ack[4], debug_dispcreditack.debug_disp_credit_ack[5], debug_dispcreditack.debug_disp_credit_ack[6], debug_dispcreditack.debug_disp_credit_ack[7]);
        }
    }

    {
        uint8_t zero = gtmv(m, 2);
        bbh_rx_debug_pdrddata debug_pdrddata = {.debug_pdrd_data = {gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_debug_pdrddata_get(bbh_id, zero, &debug_pdrddata);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_debug_pdrddata_get(%u %u %u %u %u %u %u %u %u %u)\n", bbh_id, zero,
                debug_pdrddata.debug_pdrd_data[0], debug_pdrddata.debug_pdrd_data[1], debug_pdrddata.debug_pdrd_data[2], debug_pdrddata.debug_pdrd_data[3], 
                debug_pdrddata.debug_pdrd_data[4], debug_pdrddata.debug_pdrd_data[5], debug_pdrddata.debug_pdrd_data[6], debug_pdrddata.debug_pdrd_data[7]);
        }
    }

    {
        uint8_t zero = gtmv(m, 2);
        bbh_rx_debug_sbnfifo debug_sbnfifo = {.sbn_fifo = {gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_debug_sbnfifo_get(bbh_id, zero, &debug_sbnfifo);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_debug_sbnfifo_get(%u %u %u %u %u %u %u %u %u %u)\n", bbh_id, zero,
                debug_sbnfifo.sbn_fifo[0], debug_sbnfifo.sbn_fifo[1], debug_sbnfifo.sbn_fifo[2], debug_sbnfifo.sbn_fifo[3], 
                debug_sbnfifo.sbn_fifo[4], debug_sbnfifo.sbn_fifo[5], debug_sbnfifo.sbn_fifo[6], debug_sbnfifo.sbn_fifo[7]);
        }
    }

    {
        uint16_t oddbnentry = gtmv(m, 14);
        bdmf_boolean oddvalid = gtmv(m, 1);
        uint16_t evenbnentry = gtmv(m, 14);
        bdmf_boolean evenvalid = gtmv(m, 1);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_debug_sbnprefetch_get(bbh_id, &oddbnentry, &oddvalid, &evenbnentry, &evenvalid);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_debug_sbnprefetch_get(%u %u %u %u %u)\n", bbh_id,
                oddbnentry, oddvalid, evenbnentry, evenvalid);
        }
    }

    {
        uint8_t zero = gtmv(m, 4);
        bbh_rx_debug_excfifo debug_excfifo = {.debug_exc_fifo = {gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_debug_excfifo_get(bbh_id, zero, &debug_excfifo);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_debug_excfifo_get(%u %u %u %u %u %u %u %u %u %u)\n", bbh_id, zero,
                debug_excfifo.debug_exc_fifo[0], debug_excfifo.debug_exc_fifo[1], debug_excfifo.debug_exc_fifo[2], debug_excfifo.debug_exc_fifo[3], 
                debug_excfifo.debug_exc_fifo[4], debug_excfifo.debug_exc_fifo[5], debug_excfifo.debug_exc_fifo[6], debug_excfifo.debug_exc_fifo[7]);
        }
    }

    {
        uint32_t pmvalue = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_bbh_rx_pon_cnpl_debug_gem_out_of_range_get(bbh_id, &pmvalue);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_bbh_rx_pon_cnpl_debug_gem_out_of_range_get(%u %u)\n", bbh_id,
                pmvalue);
        }
    }

    {
        uint16_t flow = gtmv(m, 13);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_bbh_rx_pon_cnpl_debug_gem_out_of_range_flow_get(bbh_id, &flow);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_bbh_rx_pon_cnpl_debug_gem_out_of_range_flow_get(%u %u)\n", bbh_id,
                flow);
        }
    }

    {
        uint8_t usedwords = gtmv(m, 5);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_bbh_rx_pon_cnpl_debug_usedwords_get(bbh_id, &usedwords);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_bbh_rx_pon_cnpl_debug_usedwords_get(%u %u)\n", bbh_id,
                usedwords);
        }
    }

    {
        uint8_t zero = gtmv(m, 8);
        bbh_rx_gem_mode_gemmode gem_mode_gemmode = {.debug_gem_mode = {gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        bdmf_session_print(session, "ag_drv_bbh_rx_gem_mode_gemmode_set(%u %u %u %u %u %u %u %u %u %u)\n", bbh_id, zero,
            gem_mode_gemmode.debug_gem_mode[0], gem_mode_gemmode.debug_gem_mode[1], gem_mode_gemmode.debug_gem_mode[2], gem_mode_gemmode.debug_gem_mode[3], 
            gem_mode_gemmode.debug_gem_mode[4], gem_mode_gemmode.debug_gem_mode[5], gem_mode_gemmode.debug_gem_mode[6], gem_mode_gemmode.debug_gem_mode[7]);
        ag_err = ag_drv_bbh_rx_gem_mode_gemmode_set(bbh_id, zero, &gem_mode_gemmode);
        if (!ag_err)
            ag_err = ag_drv_bbh_rx_gem_mode_gemmode_get(bbh_id, zero, &gem_mode_gemmode);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_gem_mode_gemmode_get(%u %u %u %u %u %u %u %u %u %u)\n", bbh_id, zero,
                gem_mode_gemmode.debug_gem_mode[0], gem_mode_gemmode.debug_gem_mode[1], gem_mode_gemmode.debug_gem_mode[2], gem_mode_gemmode.debug_gem_mode[3], 
                gem_mode_gemmode.debug_gem_mode[4], gem_mode_gemmode.debug_gem_mode[5], gem_mode_gemmode.debug_gem_mode[6], gem_mode_gemmode.debug_gem_mode[7]);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (gem_mode_gemmode.debug_gem_mode[0] != gtmv(m, 32) || gem_mode_gemmode.debug_gem_mode[1] != gtmv(m, 32) || gem_mode_gemmode.debug_gem_mode[2] != gtmv(m, 32) || gem_mode_gemmode.debug_gem_mode[3] != gtmv(m, 32) || gem_mode_gemmode.debug_gem_mode[4] != gtmv(m, 32) || gem_mode_gemmode.debug_gem_mode[5] != gtmv(m, 32) || gem_mode_gemmode.debug_gem_mode[6] != gtmv(m, 32) || gem_mode_gemmode.debug_gem_mode[7] != gtmv(m, 32))
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
static int ag_drv_bbh_rx_cli_ext_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m, input_method = parm[0].value.unumber;
    uint8_t bbh_id = parm[2].value.unumber;
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
    case cli_bbh_rx_timer:
    {
        uint8_t max_index = 8;
        uint8_t index = gtmv(m, 3);
        uint16_t timer = gtmv(m, 16);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_timer_set(%u %u %u)\n", bbh_id, index,
                timer);
            ag_err = ag_drv_bbh_rx_timer_set(bbh_id, index, timer);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        timer = gtmv(m, 16);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_timer_set(%u %u %u)\n", bbh_id, index,
                timer);
            ag_err = ag_drv_bbh_rx_timer_set(bbh_id, index, timer);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_bbh_rx_timer_get(bbh_id, index, &timer);

            bdmf_session_print(session, "ag_drv_bbh_rx_timer_get(%u %u %u)\n", bbh_id, index,
                timer);

            if (timer != gtmv(m, 16) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of timer completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_bbh_rx_flow_control_force:
    {
        uint8_t max_index = 8;
        uint8_t index = gtmv(m, 3);
        bdmf_boolean fcforce = gtmv(m, 1);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_flow_control_force_set(%u %u %u)\n", bbh_id, index,
                fcforce);
            ag_err = ag_drv_bbh_rx_flow_control_force_set(bbh_id, index, fcforce);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        fcforce = gtmv(m, 1);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_flow_control_force_set(%u %u %u)\n", bbh_id, index,
                fcforce);
            ag_err = ag_drv_bbh_rx_flow_control_force_set(bbh_id, index, fcforce);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_bbh_rx_flow_control_force_get(bbh_id, index, &fcforce);

            bdmf_session_print(session, "ag_drv_bbh_rx_flow_control_force_get(%u %u %u)\n", bbh_id, index,
                fcforce);

            if (fcforce != gtmv(m, 1) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of flow_control_force completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_bbh_rx_flow_control_runner_enable:
    {
        uint8_t max_index = 8;
        uint8_t index = gtmv(m, 3);
        bdmf_boolean fcrnren = gtmv(m, 1);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_flow_control_runner_enable_set(%u %u %u)\n", bbh_id, index,
                fcrnren);
            ag_err = ag_drv_bbh_rx_flow_control_runner_enable_set(bbh_id, index, fcrnren);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        fcrnren = gtmv(m, 1);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_flow_control_runner_enable_set(%u %u %u)\n", bbh_id, index,
                fcrnren);
            ag_err = ag_drv_bbh_rx_flow_control_runner_enable_set(bbh_id, index, fcrnren);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_bbh_rx_flow_control_runner_enable_get(bbh_id, index, &fcrnren);

            bdmf_session_print(session, "ag_drv_bbh_rx_flow_control_runner_enable_get(%u %u %u)\n", bbh_id, index,
                fcrnren);

            if (fcrnren != gtmv(m, 1) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of flow_control_runner_enable completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_bbh_rx_flow_control_qm_enable:
    {
        uint8_t max_index = 8;
        uint8_t index = gtmv(m, 3);
        bdmf_boolean fcqmen = gtmv(m, 1);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_flow_control_qm_enable_set(%u %u %u)\n", bbh_id, index,
                fcqmen);
            ag_err = ag_drv_bbh_rx_flow_control_qm_enable_set(bbh_id, index, fcqmen);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        fcqmen = gtmv(m, 1);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_flow_control_qm_enable_set(%u %u %u)\n", bbh_id, index,
                fcqmen);
            ag_err = ag_drv_bbh_rx_flow_control_qm_enable_set(bbh_id, index, fcqmen);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_bbh_rx_flow_control_qm_enable_get(bbh_id, index, &fcqmen);

            bdmf_session_print(session, "ag_drv_bbh_rx_flow_control_qm_enable_get(%u %u %u)\n", bbh_id, index,
                fcqmen);

            if (fcqmen != gtmv(m, 1) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of flow_control_qm_enable completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_bbh_rx_flow_ctrl_timer:
    {
        uint8_t max_index = 8;
        uint8_t index = gtmv(m, 3);
        uint16_t timer = gtmv(m, 16);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_flow_ctrl_timer_set(%u %u %u)\n", bbh_id, index,
                timer);
            ag_err = ag_drv_bbh_rx_flow_ctrl_timer_set(bbh_id, index, timer);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        timer = gtmv(m, 16);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_flow_ctrl_timer_set(%u %u %u)\n", bbh_id, index,
                timer);
            ag_err = ag_drv_bbh_rx_flow_ctrl_timer_set(bbh_id, index, timer);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_bbh_rx_flow_ctrl_timer_get(bbh_id, index, &timer);

            bdmf_session_print(session, "ag_drv_bbh_rx_flow_ctrl_timer_get(%u %u %u)\n", bbh_id, index,
                timer);

            if (timer != gtmv(m, 16) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of flow_ctrl_timer completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_bbh_rx_flow_ctrl_force:
    {
        uint8_t max_index = 8;
        uint8_t index = gtmv(m, 3);
        bdmf_boolean fcforce = gtmv(m, 1);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_flow_ctrl_force_set(%u %u %u)\n", bbh_id, index,
                fcforce);
            ag_err = ag_drv_bbh_rx_flow_ctrl_force_set(bbh_id, index, fcforce);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        fcforce = gtmv(m, 1);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_flow_ctrl_force_set(%u %u %u)\n", bbh_id, index,
                fcforce);
            ag_err = ag_drv_bbh_rx_flow_ctrl_force_set(bbh_id, index, fcforce);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_bbh_rx_flow_ctrl_force_get(bbh_id, index, &fcforce);

            bdmf_session_print(session, "ag_drv_bbh_rx_flow_ctrl_force_get(%u %u %u)\n", bbh_id, index,
                fcforce);

            if (fcforce != gtmv(m, 1) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of flow_ctrl_force completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_bbh_rx_flow_ctrl_rnr_en:
    {
        uint8_t max_index = 8;
        uint8_t index = gtmv(m, 3);
        bdmf_boolean fcrnren = gtmv(m, 1);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_flow_ctrl_rnr_en_set(%u %u %u)\n", bbh_id, index,
                fcrnren);
            ag_err = ag_drv_bbh_rx_flow_ctrl_rnr_en_set(bbh_id, index, fcrnren);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        fcrnren = gtmv(m, 1);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_flow_ctrl_rnr_en_set(%u %u %u)\n", bbh_id, index,
                fcrnren);
            ag_err = ag_drv_bbh_rx_flow_ctrl_rnr_en_set(bbh_id, index, fcrnren);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_bbh_rx_flow_ctrl_rnr_en_get(bbh_id, index, &fcrnren);

            bdmf_session_print(session, "ag_drv_bbh_rx_flow_ctrl_rnr_en_get(%u %u %u)\n", bbh_id, index,
                fcrnren);

            if (fcrnren != gtmv(m, 1) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of flow_ctrl_rnr_en completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_bbh_rx_general_configuration_mactype:
    {
        uint8_t max_index = 8;
        uint8_t index = gtmv(m, 3);
        uint8_t mactype = gtmv(m, 4);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_mactype_set(%u %u %u)\n", bbh_id, index,
                mactype);
            ag_err = ag_drv_bbh_rx_general_configuration_mactype_set(bbh_id, index, mactype);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        mactype = gtmv(m, 4);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_mactype_set(%u %u %u)\n", bbh_id, index,
                mactype);
            ag_err = ag_drv_bbh_rx_general_configuration_mactype_set(bbh_id, index, mactype);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_bbh_rx_general_configuration_mactype_get(bbh_id, index, &mactype);

            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_mactype_get(%u %u %u)\n", bbh_id, index,
                mactype);

            if (mactype != gtmv(m, 4) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of general_configuration_mactype completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_bbh_rx_general_configuration_portth:
    {
        uint8_t max_index = 8;
        uint8_t index = gtmv(m, 3);
        uint8_t portthr = gtmv(m, 6);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_portth_set(%u %u %u)\n", bbh_id, index,
                portthr);
            ag_err = ag_drv_bbh_rx_general_configuration_portth_set(bbh_id, index, portthr);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        portthr = gtmv(m, 6);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_portth_set(%u %u %u)\n", bbh_id, index,
                portthr);
            ag_err = ag_drv_bbh_rx_general_configuration_portth_set(bbh_id, index, portthr);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_bbh_rx_general_configuration_portth_get(bbh_id, index, &portthr);

            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_portth_get(%u %u %u)\n", bbh_id, index,
                portthr);

            if (portthr != gtmv(m, 6) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of general_configuration_portth completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_bbh_rx_general_configuration_sopoffset:
    {
        uint8_t max_index = 8;
        uint8_t index = gtmv(m, 3);
        uint8_t sopoffset = gtmv(m, 7);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_sopoffset_set(%u %u %u)\n", bbh_id, index,
                sopoffset);
            ag_err = ag_drv_bbh_rx_general_configuration_sopoffset_set(bbh_id, index, sopoffset);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        sopoffset = gtmv(m, 7);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_sopoffset_set(%u %u %u)\n", bbh_id, index,
                sopoffset);
            ag_err = ag_drv_bbh_rx_general_configuration_sopoffset_set(bbh_id, index, sopoffset);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_bbh_rx_general_configuration_sopoffset_get(bbh_id, index, &sopoffset);

            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_sopoffset_get(%u %u %u)\n", bbh_id, index,
                sopoffset);

            if (sopoffset != gtmv(m, 7) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of general_configuration_sopoffset completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_bbh_rx_general_configuration_ingresspid:
    {
        uint8_t max_channel_num = 8;
        uint8_t channel_num = gtmv(m, 3);
        uint8_t ingresspid = gtmv(m, 8);

        if ((start_idx >= max_channel_num) || (stop_idx >= max_channel_num))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_channel_num);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (channel_num = 0; channel_num < max_channel_num; channel_num++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_ingresspid_set(%u %u %u)\n", bbh_id, channel_num,
                ingresspid);
            ag_err = ag_drv_bbh_rx_general_configuration_ingresspid_set(bbh_id, channel_num, ingresspid);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", channel_num);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ingresspid = gtmv(m, 8);

        for (channel_num = start_idx; channel_num <= stop_idx; channel_num++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_ingresspid_set(%u %u %u)\n", bbh_id, channel_num,
                ingresspid);
            ag_err = ag_drv_bbh_rx_general_configuration_ingresspid_set(bbh_id, channel_num, ingresspid);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", channel_num);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (channel_num = 0; channel_num < max_channel_num; channel_num++)
        {
            if (channel_num < start_idx || channel_num > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_bbh_rx_general_configuration_ingresspid_get(bbh_id, channel_num, &ingresspid);

            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_ingresspid_get(%u %u %u)\n", bbh_id, channel_num,
                ingresspid);

            if (ingresspid != gtmv(m, 8) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", channel_num);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of general_configuration_ingresspid completed. Number of tested entries %u.\n", max_channel_num);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_bbh_rx_general_configuration_portweight:
    {
        uint8_t max_index = 8;
        uint8_t index = gtmv(m, 3);
        uint8_t portweight = gtmv(m, 3);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_portweight_set(%u %u %u)\n", bbh_id, index,
                portweight);
            ag_err = ag_drv_bbh_rx_general_configuration_portweight_set(bbh_id, index, portweight);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        portweight = gtmv(m, 3);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_portweight_set(%u %u %u)\n", bbh_id, index,
                portweight);
            ag_err = ag_drv_bbh_rx_general_configuration_portweight_set(bbh_id, index, portweight);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_bbh_rx_general_configuration_portweight_get(bbh_id, index, &portweight);

            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_portweight_get(%u %u %u)\n", bbh_id, index,
                portweight);

            if (portweight != gtmv(m, 3) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of general_configuration_portweight completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_bbh_rx_general_configuration_portviqmask:
    {
        uint8_t max_viq_index = 8;
        uint8_t viq_index = gtmv(m, 3);
        uint16_t portviqmask = gtmv(m, 9);

        if ((start_idx >= max_viq_index) || (stop_idx >= max_viq_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_viq_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (viq_index = 0; viq_index < max_viq_index; viq_index++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_portviqmask_set(%u %u %u)\n", bbh_id, viq_index,
                portviqmask);
            ag_err = ag_drv_bbh_rx_general_configuration_portviqmask_set(bbh_id, viq_index, portviqmask);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", viq_index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        portviqmask = gtmv(m, 9);

        for (viq_index = start_idx; viq_index <= stop_idx; viq_index++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_portviqmask_set(%u %u %u)\n", bbh_id, viq_index,
                portviqmask);
            ag_err = ag_drv_bbh_rx_general_configuration_portviqmask_set(bbh_id, viq_index, portviqmask);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", viq_index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (viq_index = 0; viq_index < max_viq_index; viq_index++)
        {
            if (viq_index < start_idx || viq_index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_bbh_rx_general_configuration_portviqmask_get(bbh_id, viq_index, &portviqmask);

            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_portviqmask_get(%u %u %u)\n", bbh_id, viq_index,
                portviqmask);

            if (portviqmask != gtmv(m, 9) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", viq_index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of general_configuration_portviqmask completed. Number of tested entries %u.\n", max_viq_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_bbh_rx_general_configuration_excviqmask:
    {
        uint8_t max_viq_index = 8;
        uint8_t viq_index = gtmv(m, 3);
        uint16_t excviqmask = gtmv(m, 9);
        bdmf_boolean excviqen = gtmv(m, 1);
        bdmf_boolean excviqoamen = gtmv(m, 1);

        if ((start_idx >= max_viq_index) || (stop_idx >= max_viq_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_viq_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (viq_index = 0; viq_index < max_viq_index; viq_index++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_excviqmask_set(%u %u %u %u %u)\n", bbh_id, viq_index,
                excviqmask, excviqen, excviqoamen);
            ag_err = ag_drv_bbh_rx_general_configuration_excviqmask_set(bbh_id, viq_index, excviqmask, excviqen, excviqoamen);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", viq_index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        excviqmask = gtmv(m, 9);
        excviqen = gtmv(m, 1);
        excviqoamen = gtmv(m, 1);

        for (viq_index = start_idx; viq_index <= stop_idx; viq_index++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_excviqmask_set(%u %u %u %u %u)\n", bbh_id, viq_index,
                excviqmask, excviqen, excviqoamen);
            ag_err = ag_drv_bbh_rx_general_configuration_excviqmask_set(bbh_id, viq_index, excviqmask, excviqen, excviqoamen);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", viq_index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (viq_index = 0; viq_index < max_viq_index; viq_index++)
        {
            if (viq_index < start_idx || viq_index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_bbh_rx_general_configuration_excviqmask_get(bbh_id, viq_index, &excviqmask, &excviqen, &excviqoamen);

            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_excviqmask_get(%u %u %u %u %u)\n", bbh_id, viq_index,
                excviqmask, excviqen, excviqoamen);

            if (excviqmask != gtmv(m, 9) || excviqen != gtmv(m, 1) || excviqoamen != gtmv(m, 1) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", viq_index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of general_configuration_excviqmask completed. Number of tested entries %u.\n", max_viq_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_bbh_rx_general_configuration_dispviq:
    {
        uint8_t max_viq_index = 9;
        uint8_t viq_index = gtmv(m, 3);
        uint8_t viq = gtmv(m, 4);

        if ((start_idx >= max_viq_index) || (stop_idx >= max_viq_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_viq_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (viq_index = 0; viq_index < max_viq_index; viq_index++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_dispviq_set(%u %u %u)\n", bbh_id, viq_index,
                viq);
            ag_err = ag_drv_bbh_rx_general_configuration_dispviq_set(bbh_id, viq_index, viq);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", viq_index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        viq = gtmv(m, 4);

        for (viq_index = start_idx; viq_index <= stop_idx; viq_index++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_dispviq_set(%u %u %u)\n", bbh_id, viq_index,
                viq);
            ag_err = ag_drv_bbh_rx_general_configuration_dispviq_set(bbh_id, viq_index, viq);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", viq_index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (viq_index = 0; viq_index < max_viq_index; viq_index++)
        {
            if (viq_index < start_idx || viq_index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_bbh_rx_general_configuration_dispviq_get(bbh_id, viq_index, &viq);

            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_dispviq_get(%u %u %u)\n", bbh_id, viq_index,
                viq);

            if (viq != gtmv(m, 4) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", viq_index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of general_configuration_dispviq completed. Number of tested entries %u.\n", max_viq_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_bbh_rx_general_configuration_viqweight:
    {
        uint8_t max_index = 9;
        uint8_t index = gtmv(m, 3);
        uint8_t viqweight = gtmv(m, 3);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_viqweight_set(%u %u %u)\n", bbh_id, index,
                viqweight);
            ag_err = ag_drv_bbh_rx_general_configuration_viqweight_set(bbh_id, index, viqweight);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        viqweight = gtmv(m, 3);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_viqweight_set(%u %u %u)\n", bbh_id, index,
                viqweight);
            ag_err = ag_drv_bbh_rx_general_configuration_viqweight_set(bbh_id, index, viqweight);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_bbh_rx_general_configuration_viqweight_get(bbh_id, index, &viqweight);

            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_viqweight_get(%u %u %u)\n", bbh_id, index,
                viqweight);

            if (viqweight != gtmv(m, 3) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of general_configuration_viqweight completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_bbh_rx_general_configuration_patterncfg:
    {
        uint8_t max_viq_index = 8;
        uint8_t viq_index = gtmv(m, 3);
        uint8_t location = gtmv(m, 5);
        bdmf_boolean no_drop_en = gtmv(m, 1);
        bdmf_boolean viqclassen = gtmv(m, 1);
        uint16_t viq = gtmv(m, 9);

        if ((start_idx >= max_viq_index) || (stop_idx >= max_viq_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_viq_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (viq_index = 0; viq_index < max_viq_index; viq_index++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_patterncfg_set(%u %u %u %u %u %u)\n", bbh_id, viq_index,
                location, no_drop_en, viqclassen, viq);
            ag_err = ag_drv_bbh_rx_general_configuration_patterncfg_set(bbh_id, viq_index, location, no_drop_en, viqclassen, viq);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", viq_index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        location = gtmv(m, 5);
        no_drop_en = gtmv(m, 1);
        viqclassen = gtmv(m, 1);
        viq = gtmv(m, 9);

        for (viq_index = start_idx; viq_index <= stop_idx; viq_index++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_patterncfg_set(%u %u %u %u %u %u)\n", bbh_id, viq_index,
                location, no_drop_en, viqclassen, viq);
            ag_err = ag_drv_bbh_rx_general_configuration_patterncfg_set(bbh_id, viq_index, location, no_drop_en, viqclassen, viq);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", viq_index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (viq_index = 0; viq_index < max_viq_index; viq_index++)
        {
            if (viq_index < start_idx || viq_index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_bbh_rx_general_configuration_patterncfg_get(bbh_id, viq_index, &location, &no_drop_en, &viqclassen, &viq);

            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_patterncfg_get(%u %u %u %u %u %u)\n", bbh_id, viq_index,
                location, no_drop_en, viqclassen, viq);

            if (location != gtmv(m, 5) || no_drop_en != gtmv(m, 1) || viqclassen != gtmv(m, 1) || viq != gtmv(m, 9) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", viq_index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of general_configuration_patterncfg completed. Number of tested entries %u.\n", max_viq_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_bbh_rx_general_configuration_patterndata:
    {
        uint8_t max_zero = 3;
        uint8_t zero = gtmv(m, 1);
        bbh_rx_general_configuration_patterndata general_configuration_patterndata = {.pattern_data_config = {gtmv(m, 32), gtmv(m, 32) } };

        if ((start_idx >= max_zero) || (stop_idx >= max_zero))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_zero);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (zero = 0; zero < max_zero; zero++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_patterndata_set(%u %u %u %u)\n", bbh_id, zero,
                general_configuration_patterndata.pattern_data_config[0], general_configuration_patterndata.pattern_data_config[1]);
            ag_err = ag_drv_bbh_rx_general_configuration_patterndata_set(bbh_id, zero, &general_configuration_patterndata);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", zero);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        general_configuration_patterndata.pattern_data_config[0] = gtmv(m, 32);
        general_configuration_patterndata.pattern_data_config[1] = gtmv(m, 32);

        for (zero = start_idx; zero <= stop_idx; zero++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_patterndata_set(%u %u %u %u)\n", bbh_id, zero,
                general_configuration_patterndata.pattern_data_config[0], general_configuration_patterndata.pattern_data_config[1]);
            ag_err = ag_drv_bbh_rx_general_configuration_patterndata_set(bbh_id, zero, &general_configuration_patterndata);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", zero);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (zero = 0; zero < max_zero; zero++)
        {
            if (zero < start_idx || zero > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_bbh_rx_general_configuration_patterndata_get(bbh_id, zero, &general_configuration_patterndata);

            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_patterndata_get(%u %u %u %u)\n", bbh_id, zero,
                general_configuration_patterndata.pattern_data_config[0], general_configuration_patterndata.pattern_data_config[1]);

            if (general_configuration_patterndata.pattern_data_config[0] != gtmv(m, 32) || 
                general_configuration_patterndata.pattern_data_config[1] != gtmv(m, 32) || 
                0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", zero);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of general_configuration_patterndata completed. Number of tested entries %u.\n", max_zero);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_bbh_rx_general_configuration_patternmask:
    {
        uint8_t max_index = 6;
        uint8_t index = gtmv(m, 2);
        uint32_t patternmask = gtmv(m, 32);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_patternmask_set(%u %u %u)\n", bbh_id, index,
                patternmask);
            ag_err = ag_drv_bbh_rx_general_configuration_patternmask_set(bbh_id, index, patternmask);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        patternmask = gtmv(m, 32);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_patternmask_set(%u %u %u)\n", bbh_id, index,
                patternmask);
            ag_err = ag_drv_bbh_rx_general_configuration_patternmask_set(bbh_id, index, patternmask);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_bbh_rx_general_configuration_patternmask_get(bbh_id, index, &patternmask);

            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_patternmask_get(%u %u %u)\n", bbh_id, index,
                patternmask);

            if (patternmask != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of general_configuration_patternmask completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_bbh_rx_general_configuration_patterndata8b:
    {
        uint8_t max_viq_index = 4;
        uint8_t viq_index = gtmv(m, 2);
        uint32_t patterndata = gtmv(m, 32);

        if ((start_idx >= max_viq_index) || (stop_idx >= max_viq_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_viq_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (viq_index = 0; viq_index < max_viq_index; viq_index++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_patterndata8b_set(%u %u %u)\n", bbh_id, viq_index,
                patterndata);
            ag_err = ag_drv_bbh_rx_general_configuration_patterndata8b_set(bbh_id, viq_index, patterndata);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", viq_index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        patterndata = gtmv(m, 32);

        for (viq_index = start_idx; viq_index <= stop_idx; viq_index++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_patterndata8b_set(%u %u %u)\n", bbh_id, viq_index,
                patterndata);
            ag_err = ag_drv_bbh_rx_general_configuration_patterndata8b_set(bbh_id, viq_index, patterndata);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", viq_index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (viq_index = 0; viq_index < max_viq_index; viq_index++)
        {
            if (viq_index < start_idx || viq_index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_bbh_rx_general_configuration_patterndata8b_get(bbh_id, viq_index, &patterndata);

            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_patterndata8b_get(%u %u %u)\n", bbh_id, viq_index,
                patterndata);

            if (patterndata != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", viq_index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of general_configuration_patterndata8b completed. Number of tested entries %u.\n", max_viq_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_bbh_rx_general_configuration_patternmask8b:
    {
        uint8_t max_viq_index = 4;
        uint8_t viq_index = gtmv(m, 2);
        uint32_t patternmask = gtmv(m, 32);

        if ((start_idx >= max_viq_index) || (stop_idx >= max_viq_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_viq_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (viq_index = 0; viq_index < max_viq_index; viq_index++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_patternmask8b_set(%u %u %u)\n", bbh_id, viq_index,
                patternmask);
            ag_err = ag_drv_bbh_rx_general_configuration_patternmask8b_set(bbh_id, viq_index, patternmask);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", viq_index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        patternmask = gtmv(m, 32);

        for (viq_index = start_idx; viq_index <= stop_idx; viq_index++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_patternmask8b_set(%u %u %u)\n", bbh_id, viq_index,
                patternmask);
            ag_err = ag_drv_bbh_rx_general_configuration_patternmask8b_set(bbh_id, viq_index, patternmask);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", viq_index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (viq_index = 0; viq_index < max_viq_index; viq_index++)
        {
            if (viq_index < start_idx || viq_index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_bbh_rx_general_configuration_patternmask8b_get(bbh_id, viq_index, &patternmask);

            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_patternmask8b_get(%u %u %u)\n", bbh_id, viq_index,
                patternmask);

            if (patternmask != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", viq_index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of general_configuration_patternmask8b completed. Number of tested entries %u.\n", max_viq_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_bbh_rx_general_configuration_patternen:
    {
        uint8_t max_viq_index = 8;
        uint8_t viq_index = gtmv(m, 3);
        uint8_t patternen = gtmv(m, 8);

        if ((start_idx >= max_viq_index) || (stop_idx >= max_viq_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_viq_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (viq_index = 0; viq_index < max_viq_index; viq_index++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_patternen_set(%u %u %u)\n", bbh_id, viq_index,
                patternen);
            ag_err = ag_drv_bbh_rx_general_configuration_patternen_set(bbh_id, viq_index, patternen);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", viq_index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        patternen = gtmv(m, 8);

        for (viq_index = start_idx; viq_index <= stop_idx; viq_index++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_patternen_set(%u %u %u)\n", bbh_id, viq_index,
                patternen);
            ag_err = ag_drv_bbh_rx_general_configuration_patternen_set(bbh_id, viq_index, patternen);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", viq_index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (viq_index = 0; viq_index < max_viq_index; viq_index++)
        {
            if (viq_index < start_idx || viq_index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_bbh_rx_general_configuration_patternen_get(bbh_id, viq_index, &patternen);

            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_patternen_get(%u %u %u)\n", bbh_id, viq_index,
                patternen);

            if (patternen != gtmv(m, 8) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", viq_index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of general_configuration_patternen completed. Number of tested entries %u.\n", max_viq_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_bbh_rx_general_configuration_viqfcen:
    {
        uint8_t max_viq_index = 8;
        uint8_t viq_index = gtmv(m, 3);
        uint16_t viqfcen = gtmv(m, 9);

        if ((start_idx >= max_viq_index) || (stop_idx >= max_viq_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_viq_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (viq_index = 0; viq_index < max_viq_index; viq_index++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_viqfcen_set(%u %u %u)\n", bbh_id, viq_index,
                viqfcen);
            ag_err = ag_drv_bbh_rx_general_configuration_viqfcen_set(bbh_id, viq_index, viqfcen);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", viq_index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        viqfcen = gtmv(m, 9);

        for (viq_index = start_idx; viq_index <= stop_idx; viq_index++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_viqfcen_set(%u %u %u)\n", bbh_id, viq_index,
                viqfcen);
            ag_err = ag_drv_bbh_rx_general_configuration_viqfcen_set(bbh_id, viq_index, viqfcen);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", viq_index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (viq_index = 0; viq_index < max_viq_index; viq_index++)
        {
            if (viq_index < start_idx || viq_index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_bbh_rx_general_configuration_viqfcen_get(bbh_id, viq_index, &viqfcen);

            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_viqfcen_get(%u %u %u)\n", bbh_id, viq_index,
                viqfcen);

            if (viqfcen != gtmv(m, 9) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", viq_index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of general_configuration_viqfcen completed. Number of tested entries %u.\n", max_viq_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_bbh_rx_general_configuration_noneth:
    {
        uint8_t max_viq_index = 8;
        uint8_t viq_index = gtmv(m, 3);
        uint16_t flowid = gtmv(m, 13);
        bdmf_boolean enable = gtmv(m, 1);

        if ((start_idx >= max_viq_index) || (stop_idx >= max_viq_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_viq_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (viq_index = 0; viq_index < max_viq_index; viq_index++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_noneth_set(%u %u %u %u)\n", bbh_id, viq_index,
                flowid, enable);
            ag_err = ag_drv_bbh_rx_general_configuration_noneth_set(bbh_id, viq_index, flowid, enable);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", viq_index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        flowid = gtmv(m, 13);
        enable = gtmv(m, 1);

        for (viq_index = start_idx; viq_index <= stop_idx; viq_index++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_noneth_set(%u %u %u %u)\n", bbh_id, viq_index,
                flowid, enable);
            ag_err = ag_drv_bbh_rx_general_configuration_noneth_set(bbh_id, viq_index, flowid, enable);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", viq_index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (viq_index = 0; viq_index < max_viq_index; viq_index++)
        {
            if (viq_index < start_idx || viq_index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_bbh_rx_general_configuration_noneth_get(bbh_id, viq_index, &flowid, &enable);

            bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_noneth_get(%u %u %u %u)\n", bbh_id, viq_index,
                flowid, enable);

            if (flowid != gtmv(m, 13) || enable != gtmv(m, 1) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", viq_index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of general_configuration_noneth completed. Number of tested entries %u.\n", max_viq_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_bbh_rx_gem_mode_gemmode:
    {
        uint8_t max_zero = 256;
        uint8_t zero = gtmv(m, 8);
        bbh_rx_gem_mode_gemmode gem_mode_gemmode = {.debug_gem_mode = {gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32) } };

        if ((start_idx >= max_zero) || (stop_idx >= max_zero))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_zero);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (zero = 0; zero < max_zero; zero++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_gem_mode_gemmode_set(%u %u %u %u %u %u %u %u %u %u)\n", bbh_id, zero,
                gem_mode_gemmode.debug_gem_mode[0], gem_mode_gemmode.debug_gem_mode[1], gem_mode_gemmode.debug_gem_mode[2], gem_mode_gemmode.debug_gem_mode[3], 
                gem_mode_gemmode.debug_gem_mode[4], gem_mode_gemmode.debug_gem_mode[5], gem_mode_gemmode.debug_gem_mode[6], gem_mode_gemmode.debug_gem_mode[7]);
            ag_err = ag_drv_bbh_rx_gem_mode_gemmode_set(bbh_id, zero, &gem_mode_gemmode);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", zero);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        gem_mode_gemmode.debug_gem_mode[0] = gtmv(m, 32);
        gem_mode_gemmode.debug_gem_mode[1] = gtmv(m, 32);
        gem_mode_gemmode.debug_gem_mode[2] = gtmv(m, 32);
        gem_mode_gemmode.debug_gem_mode[3] = gtmv(m, 32);
        gem_mode_gemmode.debug_gem_mode[4] = gtmv(m, 32);
        gem_mode_gemmode.debug_gem_mode[5] = gtmv(m, 32);
        gem_mode_gemmode.debug_gem_mode[6] = gtmv(m, 32);
        gem_mode_gemmode.debug_gem_mode[7] = gtmv(m, 32);

        for (zero = start_idx; zero <= stop_idx; zero++)
        {
            bdmf_session_print(session, "ag_drv_bbh_rx_gem_mode_gemmode_set(%u %u %u %u %u %u %u %u %u %u)\n", bbh_id, zero,
                gem_mode_gemmode.debug_gem_mode[0], gem_mode_gemmode.debug_gem_mode[1], gem_mode_gemmode.debug_gem_mode[2], gem_mode_gemmode.debug_gem_mode[3], 
                gem_mode_gemmode.debug_gem_mode[4], gem_mode_gemmode.debug_gem_mode[5], gem_mode_gemmode.debug_gem_mode[6], gem_mode_gemmode.debug_gem_mode[7]);
            ag_err = ag_drv_bbh_rx_gem_mode_gemmode_set(bbh_id, zero, &gem_mode_gemmode);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", zero);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (zero = 0; zero < max_zero; zero++)
        {
            if (zero < start_idx || zero > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_bbh_rx_gem_mode_gemmode_get(bbh_id, zero, &gem_mode_gemmode);

            bdmf_session_print(session, "ag_drv_bbh_rx_gem_mode_gemmode_get(%u %u %u %u %u %u %u %u %u %u)\n", bbh_id, zero,
                gem_mode_gemmode.debug_gem_mode[0], gem_mode_gemmode.debug_gem_mode[1], gem_mode_gemmode.debug_gem_mode[2], gem_mode_gemmode.debug_gem_mode[3], 
                gem_mode_gemmode.debug_gem_mode[4], gem_mode_gemmode.debug_gem_mode[5], gem_mode_gemmode.debug_gem_mode[6], gem_mode_gemmode.debug_gem_mode[7]);

            if (gem_mode_gemmode.debug_gem_mode[0] != gtmv(m, 32) || 
                gem_mode_gemmode.debug_gem_mode[1] != gtmv(m, 32) || 
                gem_mode_gemmode.debug_gem_mode[2] != gtmv(m, 32) || 
                gem_mode_gemmode.debug_gem_mode[3] != gtmv(m, 32) || 
                gem_mode_gemmode.debug_gem_mode[4] != gtmv(m, 32) || 
                gem_mode_gemmode.debug_gem_mode[5] != gtmv(m, 32) || 
                gem_mode_gemmode.debug_gem_mode[6] != gtmv(m, 32) || 
                gem_mode_gemmode.debug_gem_mode[7] != gtmv(m, 32) || 
                0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", zero);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of gem_mode_gemmode completed. Number of tested entries %u.\n", max_zero);
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
static int ag_drv_bbh_rx_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_general_configuration_bbcfg: reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_BBCFG); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_enable: reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_ENABLE); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_minpkt: reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_MINPKT); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_maxpkt: reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_MAXPKT); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_clk_gate_cntrl: reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_CLK_GATE_CNTRL); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_rxdbgsel: reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_RXDBGSEL); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_rxrstrst: reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_RXRSTRST); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_sbpmcfg: reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_SBPMCFG); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_psrambase: reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_PSRAMBASE); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_ingresschannel: reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_INGRESSCHANNEL); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_bbovrdroute: reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_BBOVRDROUTE); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_bbovrdctrl: reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_BBOVRDCTRL); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_gemdone: reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_GEMDONE); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_mactype: reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_MACTYPE); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_portth: reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_PORTTH); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_flowctrl: reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_sopoffset: reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_SOPOFFSET); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_ingresspid: reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_INGRESSPID); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_portweight: reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_PORTWEIGHT); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_portviqmask: reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_PORTVIQMASK); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_excviqmask: reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_EXCVIQMASK); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_dispviq: reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_DISPVIQ); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_viqweight: reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_VIQWEIGHT); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_patterncfg: reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_PATTERNCFG); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_patterndata: reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_PATTERNDATA); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_patternmask: reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_PATTERNMASK); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_patterndata8b: reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_PATTERNDATA8B); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_patternmask8b: reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_PATTERNMASK8B); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_patternen: reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_PATTERNEN); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_viqfcen: reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_VIQFCEN); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_noneth: reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_NONETH); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_pm_counters_inpkt: reg = &RU_REG(BBH_RX, PM_COUNTERS_INPKT); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_pm_counters_inbyte: reg = &RU_REG(BBH_RX, PM_COUNTERS_INBYTE); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_pm_counters_tooshort: reg = &RU_REG(BBH_RX, PM_COUNTERS_TOOSHORT); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_pm_counters_toolong: reg = &RU_REG(BBH_RX, PM_COUNTERS_TOOLONG); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_pm_counters_crcerror: reg = &RU_REG(BBH_RX, PM_COUNTERS_CRCERROR); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_pm_counters_fcdrop: reg = &RU_REG(BBH_RX, PM_COUNTERS_FCDROP); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_pm_counters_truncate: reg = &RU_REG(BBH_RX, PM_COUNTERS_TRUNCATE); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_pm_counters_totaldrop: reg = &RU_REG(BBH_RX, PM_COUNTERS_TOTALDROP); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_pm_counters_dummypdcnt: reg = &RU_REG(BBH_RX, PM_COUNTERS_DUMMYPDCNT); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_pon_rmon_counters_r64: reg = &RU_REG(BBH_RX, PON_RMON_COUNTERS_R64); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_pon_rmon_counters_r127: reg = &RU_REG(BBH_RX, PON_RMON_COUNTERS_R127); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_pon_rmon_counters_r255: reg = &RU_REG(BBH_RX, PON_RMON_COUNTERS_R255); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_pon_rmon_counters_r511: reg = &RU_REG(BBH_RX, PON_RMON_COUNTERS_R511); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_pon_rmon_counters_r1023: reg = &RU_REG(BBH_RX, PON_RMON_COUNTERS_R1023); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_pon_rmon_counters_r1518: reg = &RU_REG(BBH_RX, PON_RMON_COUNTERS_R1518); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_pon_rmon_counters_r2047: reg = &RU_REG(BBH_RX, PON_RMON_COUNTERS_R2047); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_pon_rmon_counters_r4095: reg = &RU_REG(BBH_RX, PON_RMON_COUNTERS_R4095); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_pon_rmon_counters_r9216: reg = &RU_REG(BBH_RX, PON_RMON_COUNTERS_R9216); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_pon_rmon_counters_r16383: reg = &RU_REG(BBH_RX, PON_RMON_COUNTERS_R16383); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_debug_pfbuw: reg = &RU_REG(BBH_RX, DEBUG_PFBUW); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_debug_patternuw: reg = &RU_REG(BBH_RX, DEBUG_PATTERNUW); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_debug_excuw: reg = &RU_REG(BBH_RX, DEBUG_EXCUW); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_debug_portuw: reg = &RU_REG(BBH_RX, DEBUG_PORTUW); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_debug_firstsbn: reg = &RU_REG(BBH_RX, DEBUG_FIRSTSBN); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_debug_currentsbn: reg = &RU_REG(BBH_RX, DEBUG_CURRENTSBN); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_debug_sn: reg = &RU_REG(BBH_RX, DEBUG_SN); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_debug_viqmask: reg = &RU_REG(BBH_RX, DEBUG_VIQMASK); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_debug_plen: reg = &RU_REG(BBH_RX, DEBUG_PLEN); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_debug_eopok: reg = &RU_REG(BBH_RX, DEBUG_EOPOK); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_debug_viqfull: reg = &RU_REG(BBH_RX, DEBUG_VIQFULL); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_debug_portreq: reg = &RU_REG(BBH_RX, DEBUG_PORTREQ); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_debug_dummyreq: reg = &RU_REG(BBH_RX, DEBUG_DUMMYREQ); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_debug_inreassembly: reg = &RU_REG(BBH_RX, DEBUG_INREASSEMBLY); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_debug_dummypd: reg = &RU_REG(BBH_RX, DEBUG_DUMMYPD); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_debug_ufuw: reg = &RU_REG(BBH_RX, DEBUG_UFUW); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_debug_pendconnectuw: reg = &RU_REG(BBH_RX, DEBUG_PENDCONNECTUW); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_debug_dbgvec: reg = &RU_REG(BBH_RX, DEBUG_DBGVEC); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_debug_pduw: reg = &RU_REG(BBH_RX, DEBUG_PDUW); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_debug_sbpmack: reg = &RU_REG(BBH_RX, DEBUG_SBPMACK); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_debug_writereplyack: reg = &RU_REG(BBH_RX, DEBUG_WRITEREPLYACK); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_debug_dispcreditack: reg = &RU_REG(BBH_RX, DEBUG_DISPCREDITACK); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_debug_pdrddata: reg = &RU_REG(BBH_RX, DEBUG_PDRDDATA); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_debug_sbnfifo: reg = &RU_REG(BBH_RX, DEBUG_SBNFIFO); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_debug_sbnprefetch: reg = &RU_REG(BBH_RX, DEBUG_SBNPREFETCH); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_debug_excfifo: reg = &RU_REG(BBH_RX, DEBUG_EXCFIFO); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_bbh_rx_pon_cnpl_debug_gem_out_of_range: reg = &RU_REG(BBH_RX, BBH_RX_PON_CNPL_DEBUG_GEM_OUT_OF_RANGE); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_bbh_rx_pon_cnpl_debug_gem_out_of_range_flow: reg = &RU_REG(BBH_RX, BBH_RX_PON_CNPL_DEBUG_GEM_OUT_OF_RANGE_FLOW); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_bbh_rx_pon_cnpl_debug_usedwords: reg = &RU_REG(BBH_RX, BBH_RX_PON_CNPL_DEBUG_USEDWORDS); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_gem_mode_gemmode: reg = &RU_REG(BBH_RX, GEM_MODE_GEMMODE); blk = &RU_BLK(BBH_RX); break;
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

bdmfmon_handle_t ag_drv_bbh_rx_cli_init(bdmfmon_handle_t root_dir)
{
    bdmfmon_handle_t dir;

    dir = bdmfmon_dir_add(root_dir, "bbh_rx", "bbh_rx", BDMF_ACCESS_ADMIN, NULL);
    BUG_ON(dir == NULL);

    {
        static bdmfmon_cmd_parm_t set_timer[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("timer", "timer", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_flow_control_force[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fcforce", "fcforce", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_flow_control_runner_enable[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fcrnren", "fcrnren", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_flow_control_qm_enable[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fcqmen", "fcqmen", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_flow_ctrl_timer[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("timer", "timer", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_flow_ctrl_force[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fcforce", "fcforce", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_flow_ctrl_rnr_en[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fcrnren", "fcrnren", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_configuration_bbcfg[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("dispbbid", "dispbbid", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("sbpmbbid", "sbpmbbid", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_configuration_enable[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pkten", "pkten", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("sbpmen", "sbpmen", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("geminit", "geminit", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("grace_dis", "grace_dis", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_configuration_minpkt[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("minpkt_normal", "minpkt_normal", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("minpkt_omci", "minpkt_omci", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_configuration_maxpkt[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("maxpkt", "maxpkt", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_configuration_clk_gate_cntrl[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bypass_clk_gate", "bypass_clk_gate", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("timer_val", "timer_val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_en", "keep_alive_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_intrvl", "keep_alive_intrvl", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_cyc", "keep_alive_cyc", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_configuration_rxdbgsel[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("rxdbgsel", "rxdbgsel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_configuration_rxrstrst[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("portrst", "portrst", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("psramrst", "psramrst", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pdfiforst", "pdfiforst", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("sbpmfiforst", "sbpmfiforst", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("dispcreditrst", "dispcreditrst", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("upldfiforst", "upldfiforst", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pfbch", "pfbch", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("dispcredit", "dispcredit", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("dispviq", "dispviq", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_configuration_sbpmcfg[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("max_otf_sbpm_req", "max_otf_sbpm_req", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_configuration_psrambase[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("psrambase", "psrambase", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_configuration_ingresschannel[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ingresschannel", "ingresschannel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_configuration_bbovrdroute[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("route", "route", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_configuration_bbovrdctrl[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("dest", "dest", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("en", "en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_configuration_mactype[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mactype", "mactype", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_configuration_portth[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("portthr", "portthr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_configuration_sopoffset[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("sopoffset", "sopoffset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_configuration_ingresspid[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("channel_num", "channel_num", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ingresspid", "ingresspid", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_configuration_portweight[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("portweight", "portweight", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_configuration_portviqmask[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("viq_index", "viq_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("portviqmask", "portviqmask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_configuration_excviqmask[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("viq_index", "viq_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("excviqmask", "excviqmask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("excviqen", "excviqen", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("excviqoamen", "excviqoamen", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_configuration_dispviq[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("viq_index", "viq_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("viq", "viq", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_configuration_viqweight[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("viqweight", "viqweight", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_configuration_patterncfg[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("viq_index", "viq_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("location", "location", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("no_drop_en", "no_drop_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("viqclassen", "viqclassen", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("viq", "viq", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_configuration_patterndata[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pattern_data_config0", "pattern_data_config0", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("pattern_data_config1", "pattern_data_config1", BDMFMON_PARM_HEX, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_configuration_patternmask[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("patternmask", "patternmask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_configuration_patterndata8b[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("viq_index", "viq_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("patterndata", "patterndata", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_configuration_patternmask8b[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("viq_index", "viq_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("patternmask", "patternmask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_configuration_patternen[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("viq_index", "viq_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("patternen", "patternen", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_configuration_viqfcen[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("viq_index", "viq_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("viqfcen", "viqfcen", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_configuration_noneth[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("viq_index", "viq_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("flowid", "flowid", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enable", "enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gem_mode_gemmode[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("debug_gem_mode0", "debug_gem_mode0", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("debug_gem_mode1", "debug_gem_mode1", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("debug_gem_mode2", "debug_gem_mode2", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("debug_gem_mode3", "debug_gem_mode3", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("debug_gem_mode4", "debug_gem_mode4", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("debug_gem_mode5", "debug_gem_mode5", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("debug_gem_mode6", "debug_gem_mode6", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("debug_gem_mode7", "debug_gem_mode7", BDMFMON_PARM_HEX, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "timer", .val = cli_bbh_rx_timer, .parms = set_timer },
            { .name = "flow_control_force", .val = cli_bbh_rx_flow_control_force, .parms = set_flow_control_force },
            { .name = "flow_control_runner_enable", .val = cli_bbh_rx_flow_control_runner_enable, .parms = set_flow_control_runner_enable },
            { .name = "flow_control_qm_enable", .val = cli_bbh_rx_flow_control_qm_enable, .parms = set_flow_control_qm_enable },
            { .name = "flow_ctrl_timer", .val = cli_bbh_rx_flow_ctrl_timer, .parms = set_flow_ctrl_timer },
            { .name = "flow_ctrl_force", .val = cli_bbh_rx_flow_ctrl_force, .parms = set_flow_ctrl_force },
            { .name = "flow_ctrl_rnr_en", .val = cli_bbh_rx_flow_ctrl_rnr_en, .parms = set_flow_ctrl_rnr_en },
            { .name = "general_configuration_bbcfg", .val = cli_bbh_rx_general_configuration_bbcfg, .parms = set_general_configuration_bbcfg },
            { .name = "general_configuration_enable", .val = cli_bbh_rx_general_configuration_enable, .parms = set_general_configuration_enable },
            { .name = "general_configuration_minpkt", .val = cli_bbh_rx_general_configuration_minpkt, .parms = set_general_configuration_minpkt },
            { .name = "general_configuration_maxpkt", .val = cli_bbh_rx_general_configuration_maxpkt, .parms = set_general_configuration_maxpkt },
            { .name = "general_configuration_clk_gate_cntrl", .val = cli_bbh_rx_general_configuration_clk_gate_cntrl, .parms = set_general_configuration_clk_gate_cntrl },
            { .name = "general_configuration_rxdbgsel", .val = cli_bbh_rx_general_configuration_rxdbgsel, .parms = set_general_configuration_rxdbgsel },
            { .name = "general_configuration_rxrstrst", .val = cli_bbh_rx_general_configuration_rxrstrst, .parms = set_general_configuration_rxrstrst },
            { .name = "general_configuration_sbpmcfg", .val = cli_bbh_rx_general_configuration_sbpmcfg, .parms = set_general_configuration_sbpmcfg },
            { .name = "general_configuration_psrambase", .val = cli_bbh_rx_general_configuration_psrambase, .parms = set_general_configuration_psrambase },
            { .name = "general_configuration_ingresschannel", .val = cli_bbh_rx_general_configuration_ingresschannel, .parms = set_general_configuration_ingresschannel },
            { .name = "general_configuration_bbovrdroute", .val = cli_bbh_rx_general_configuration_bbovrdroute, .parms = set_general_configuration_bbovrdroute },
            { .name = "general_configuration_bbovrdctrl", .val = cli_bbh_rx_general_configuration_bbovrdctrl, .parms = set_general_configuration_bbovrdctrl },
            { .name = "general_configuration_mactype", .val = cli_bbh_rx_general_configuration_mactype, .parms = set_general_configuration_mactype },
            { .name = "general_configuration_portth", .val = cli_bbh_rx_general_configuration_portth, .parms = set_general_configuration_portth },
            { .name = "general_configuration_sopoffset", .val = cli_bbh_rx_general_configuration_sopoffset, .parms = set_general_configuration_sopoffset },
            { .name = "general_configuration_ingresspid", .val = cli_bbh_rx_general_configuration_ingresspid, .parms = set_general_configuration_ingresspid },
            { .name = "general_configuration_portweight", .val = cli_bbh_rx_general_configuration_portweight, .parms = set_general_configuration_portweight },
            { .name = "general_configuration_portviqmask", .val = cli_bbh_rx_general_configuration_portviqmask, .parms = set_general_configuration_portviqmask },
            { .name = "general_configuration_excviqmask", .val = cli_bbh_rx_general_configuration_excviqmask, .parms = set_general_configuration_excviqmask },
            { .name = "general_configuration_dispviq", .val = cli_bbh_rx_general_configuration_dispviq, .parms = set_general_configuration_dispviq },
            { .name = "general_configuration_viqweight", .val = cli_bbh_rx_general_configuration_viqweight, .parms = set_general_configuration_viqweight },
            { .name = "general_configuration_patterncfg", .val = cli_bbh_rx_general_configuration_patterncfg, .parms = set_general_configuration_patterncfg },
            { .name = "general_configuration_patterndata", .val = cli_bbh_rx_general_configuration_patterndata, .parms = set_general_configuration_patterndata },
            { .name = "general_configuration_patternmask", .val = cli_bbh_rx_general_configuration_patternmask, .parms = set_general_configuration_patternmask },
            { .name = "general_configuration_patterndata8b", .val = cli_bbh_rx_general_configuration_patterndata8b, .parms = set_general_configuration_patterndata8b },
            { .name = "general_configuration_patternmask8b", .val = cli_bbh_rx_general_configuration_patternmask8b, .parms = set_general_configuration_patternmask8b },
            { .name = "general_configuration_patternen", .val = cli_bbh_rx_general_configuration_patternen, .parms = set_general_configuration_patternen },
            { .name = "general_configuration_viqfcen", .val = cli_bbh_rx_general_configuration_viqfcen, .parms = set_general_configuration_viqfcen },
            { .name = "general_configuration_noneth", .val = cli_bbh_rx_general_configuration_noneth, .parms = set_general_configuration_noneth },
            { .name = "gem_mode_gemmode", .val = cli_bbh_rx_gem_mode_gemmode, .parms = set_gem_mode_gemmode },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", ag_drv_bbh_rx_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t get_default[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_timer[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_flow_control_force[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_flow_control_runner_enable[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_flow_control_qm_enable[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_flow_ctrl_timer[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_flow_ctrl_force[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_flow_ctrl_rnr_en[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_general_configuration_mactype[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_general_configuration_portth[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_general_configuration_sopoffset[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_general_configuration_ingresspid[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("channel_num", "channel_num", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_general_configuration_portweight[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_general_configuration_portviqmask[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("viq_index", "viq_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_general_configuration_excviqmask[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("viq_index", "viq_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_general_configuration_dispviq[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("viq_index", "viq_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_general_configuration_viqweight[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_general_configuration_patterncfg[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("viq_index", "viq_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_general_configuration_patterndata[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_general_configuration_patternmask[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_general_configuration_patterndata8b[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("viq_index", "viq_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_general_configuration_patternmask8b[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("viq_index", "viq_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_general_configuration_patternen[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("viq_index", "viq_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_general_configuration_viqfcen[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("viq_index", "viq_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_general_configuration_noneth[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("viq_index", "viq_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_pm_counters_inpkt[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_pm_counters_inbyte[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_pm_counters_tooshort[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_pm_counters_toolong[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_pm_counters_crcerror[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_pm_counters_fcdrop[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_pm_counters_truncate[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_pm_counters_totaldrop[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_debug_pfbuw[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_debug_patternuw[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_debug_excuw[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_debug_portuw[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_debug_firstsbn[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_debug_currentsbn[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_debug_sn[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_debug_viqmask[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_debug_plen[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_debug_dbgvec[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_debug_pduw[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_debug_sbpmack[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_debug_writereplyack[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_debug_dispcreditack[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_debug_pdrddata[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_debug_sbnfifo[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_debug_excfifo[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_gem_mode_gemmode[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "timer", .val = cli_bbh_rx_timer, .parms = get_timer },
            { .name = "flow_control_force", .val = cli_bbh_rx_flow_control_force, .parms = get_flow_control_force },
            { .name = "flow_control_runner_enable", .val = cli_bbh_rx_flow_control_runner_enable, .parms = get_flow_control_runner_enable },
            { .name = "flow_control_qm_enable", .val = cli_bbh_rx_flow_control_qm_enable, .parms = get_flow_control_qm_enable },
            { .name = "flow_ctrl_timer", .val = cli_bbh_rx_flow_ctrl_timer, .parms = get_flow_ctrl_timer },
            { .name = "flow_ctrl_force", .val = cli_bbh_rx_flow_ctrl_force, .parms = get_flow_ctrl_force },
            { .name = "flow_ctrl_rnr_en", .val = cli_bbh_rx_flow_ctrl_rnr_en, .parms = get_flow_ctrl_rnr_en },
            { .name = "general_configuration_bbcfg", .val = cli_bbh_rx_general_configuration_bbcfg, .parms = get_default },
            { .name = "general_configuration_enable", .val = cli_bbh_rx_general_configuration_enable, .parms = get_default },
            { .name = "general_configuration_minpkt", .val = cli_bbh_rx_general_configuration_minpkt, .parms = get_default },
            { .name = "general_configuration_maxpkt", .val = cli_bbh_rx_general_configuration_maxpkt, .parms = get_default },
            { .name = "general_configuration_clk_gate_cntrl", .val = cli_bbh_rx_general_configuration_clk_gate_cntrl, .parms = get_default },
            { .name = "general_configuration_rxdbgsel", .val = cli_bbh_rx_general_configuration_rxdbgsel, .parms = get_default },
            { .name = "general_configuration_rxrstrst", .val = cli_bbh_rx_general_configuration_rxrstrst, .parms = get_default },
            { .name = "general_configuration_sbpmcfg", .val = cli_bbh_rx_general_configuration_sbpmcfg, .parms = get_default },
            { .name = "general_configuration_psrambase", .val = cli_bbh_rx_general_configuration_psrambase, .parms = get_default },
            { .name = "general_configuration_ingresschannel", .val = cli_bbh_rx_general_configuration_ingresschannel, .parms = get_default },
            { .name = "general_configuration_bbovrdroute", .val = cli_bbh_rx_general_configuration_bbovrdroute, .parms = get_default },
            { .name = "general_configuration_bbovrdctrl", .val = cli_bbh_rx_general_configuration_bbovrdctrl, .parms = get_default },
            { .name = "general_configuration_gemdone", .val = cli_bbh_rx_general_configuration_gemdone, .parms = get_default },
            { .name = "general_configuration_mactype", .val = cli_bbh_rx_general_configuration_mactype, .parms = get_general_configuration_mactype },
            { .name = "general_configuration_portth", .val = cli_bbh_rx_general_configuration_portth, .parms = get_general_configuration_portth },
            { .name = "general_configuration_sopoffset", .val = cli_bbh_rx_general_configuration_sopoffset, .parms = get_general_configuration_sopoffset },
            { .name = "general_configuration_ingresspid", .val = cli_bbh_rx_general_configuration_ingresspid, .parms = get_general_configuration_ingresspid },
            { .name = "general_configuration_portweight", .val = cli_bbh_rx_general_configuration_portweight, .parms = get_general_configuration_portweight },
            { .name = "general_configuration_portviqmask", .val = cli_bbh_rx_general_configuration_portviqmask, .parms = get_general_configuration_portviqmask },
            { .name = "general_configuration_excviqmask", .val = cli_bbh_rx_general_configuration_excviqmask, .parms = get_general_configuration_excviqmask },
            { .name = "general_configuration_dispviq", .val = cli_bbh_rx_general_configuration_dispviq, .parms = get_general_configuration_dispviq },
            { .name = "general_configuration_viqweight", .val = cli_bbh_rx_general_configuration_viqweight, .parms = get_general_configuration_viqweight },
            { .name = "general_configuration_patterncfg", .val = cli_bbh_rx_general_configuration_patterncfg, .parms = get_general_configuration_patterncfg },
            { .name = "general_configuration_patterndata", .val = cli_bbh_rx_general_configuration_patterndata, .parms = get_general_configuration_patterndata },
            { .name = "general_configuration_patternmask", .val = cli_bbh_rx_general_configuration_patternmask, .parms = get_general_configuration_patternmask },
            { .name = "general_configuration_patterndata8b", .val = cli_bbh_rx_general_configuration_patterndata8b, .parms = get_general_configuration_patterndata8b },
            { .name = "general_configuration_patternmask8b", .val = cli_bbh_rx_general_configuration_patternmask8b, .parms = get_general_configuration_patternmask8b },
            { .name = "general_configuration_patternen", .val = cli_bbh_rx_general_configuration_patternen, .parms = get_general_configuration_patternen },
            { .name = "general_configuration_viqfcen", .val = cli_bbh_rx_general_configuration_viqfcen, .parms = get_general_configuration_viqfcen },
            { .name = "general_configuration_noneth", .val = cli_bbh_rx_general_configuration_noneth, .parms = get_general_configuration_noneth },
            { .name = "pm_counters_inpkt", .val = cli_bbh_rx_pm_counters_inpkt, .parms = get_pm_counters_inpkt },
            { .name = "pm_counters_inbyte", .val = cli_bbh_rx_pm_counters_inbyte, .parms = get_pm_counters_inbyte },
            { .name = "pm_counters_tooshort", .val = cli_bbh_rx_pm_counters_tooshort, .parms = get_pm_counters_tooshort },
            { .name = "pm_counters_toolong", .val = cli_bbh_rx_pm_counters_toolong, .parms = get_pm_counters_toolong },
            { .name = "pm_counters_crcerror", .val = cli_bbh_rx_pm_counters_crcerror, .parms = get_pm_counters_crcerror },
            { .name = "pm_counters_fcdrop", .val = cli_bbh_rx_pm_counters_fcdrop, .parms = get_pm_counters_fcdrop },
            { .name = "pm_counters_truncate", .val = cli_bbh_rx_pm_counters_truncate, .parms = get_pm_counters_truncate },
            { .name = "pm_counters_totaldrop", .val = cli_bbh_rx_pm_counters_totaldrop, .parms = get_pm_counters_totaldrop },
            { .name = "pm_counters_dummypdcnt", .val = cli_bbh_rx_pm_counters_dummypdcnt, .parms = get_default },
            { .name = "pon_rmon_counters_r64", .val = cli_bbh_rx_pon_rmon_counters_r64, .parms = get_default },
            { .name = "pon_rmon_counters_r127", .val = cli_bbh_rx_pon_rmon_counters_r127, .parms = get_default },
            { .name = "pon_rmon_counters_r255", .val = cli_bbh_rx_pon_rmon_counters_r255, .parms = get_default },
            { .name = "pon_rmon_counters_r511", .val = cli_bbh_rx_pon_rmon_counters_r511, .parms = get_default },
            { .name = "pon_rmon_counters_r1023", .val = cli_bbh_rx_pon_rmon_counters_r1023, .parms = get_default },
            { .name = "pon_rmon_counters_r1518", .val = cli_bbh_rx_pon_rmon_counters_r1518, .parms = get_default },
            { .name = "pon_rmon_counters_r2047", .val = cli_bbh_rx_pon_rmon_counters_r2047, .parms = get_default },
            { .name = "pon_rmon_counters_r4095", .val = cli_bbh_rx_pon_rmon_counters_r4095, .parms = get_default },
            { .name = "pon_rmon_counters_r9216", .val = cli_bbh_rx_pon_rmon_counters_r9216, .parms = get_default },
            { .name = "pon_rmon_counters_r16383", .val = cli_bbh_rx_pon_rmon_counters_r16383, .parms = get_default },
            { .name = "debug_pfbuw", .val = cli_bbh_rx_debug_pfbuw, .parms = get_debug_pfbuw },
            { .name = "debug_patternuw", .val = cli_bbh_rx_debug_patternuw, .parms = get_debug_patternuw },
            { .name = "debug_excuw", .val = cli_bbh_rx_debug_excuw, .parms = get_debug_excuw },
            { .name = "debug_portuw", .val = cli_bbh_rx_debug_portuw, .parms = get_debug_portuw },
            { .name = "debug_firstsbn", .val = cli_bbh_rx_debug_firstsbn, .parms = get_debug_firstsbn },
            { .name = "debug_currentsbn", .val = cli_bbh_rx_debug_currentsbn, .parms = get_debug_currentsbn },
            { .name = "debug_sn", .val = cli_bbh_rx_debug_sn, .parms = get_debug_sn },
            { .name = "debug_viqmask", .val = cli_bbh_rx_debug_viqmask, .parms = get_debug_viqmask },
            { .name = "debug_plen", .val = cli_bbh_rx_debug_plen, .parms = get_debug_plen },
            { .name = "debug_eopok", .val = cli_bbh_rx_debug_eopok, .parms = get_default },
            { .name = "debug_viqfull", .val = cli_bbh_rx_debug_viqfull, .parms = get_default },
            { .name = "debug_portreq", .val = cli_bbh_rx_debug_portreq, .parms = get_default },
            { .name = "debug_dummyreq", .val = cli_bbh_rx_debug_dummyreq, .parms = get_default },
            { .name = "debug_inreassembly", .val = cli_bbh_rx_debug_inreassembly, .parms = get_default },
            { .name = "debug_dummypd", .val = cli_bbh_rx_debug_dummypd, .parms = get_default },
            { .name = "debug_ufuw", .val = cli_bbh_rx_debug_ufuw, .parms = get_default },
            { .name = "debug_pendconnectuw", .val = cli_bbh_rx_debug_pendconnectuw, .parms = get_default },
            { .name = "debug_dbgvec", .val = cli_bbh_rx_debug_dbgvec, .parms = get_debug_dbgvec },
            { .name = "debug_pduw", .val = cli_bbh_rx_debug_pduw, .parms = get_debug_pduw },
            { .name = "debug_sbpmack", .val = cli_bbh_rx_debug_sbpmack, .parms = get_debug_sbpmack },
            { .name = "debug_writereplyack", .val = cli_bbh_rx_debug_writereplyack, .parms = get_debug_writereplyack },
            { .name = "debug_dispcreditack", .val = cli_bbh_rx_debug_dispcreditack, .parms = get_debug_dispcreditack },
            { .name = "debug_pdrddata", .val = cli_bbh_rx_debug_pdrddata, .parms = get_debug_pdrddata },
            { .name = "debug_sbnfifo", .val = cli_bbh_rx_debug_sbnfifo, .parms = get_debug_sbnfifo },
            { .name = "debug_sbnprefetch", .val = cli_bbh_rx_debug_sbnprefetch, .parms = get_default },
            { .name = "debug_excfifo", .val = cli_bbh_rx_debug_excfifo, .parms = get_debug_excfifo },
            { .name = "bbh_rx_pon_cnpl_debug_gem_out_of_range", .val = cli_bbh_rx_bbh_rx_pon_cnpl_debug_gem_out_of_range, .parms = get_default },
            { .name = "bbh_rx_pon_cnpl_debug_gem_out_of_range_flow", .val = cli_bbh_rx_bbh_rx_pon_cnpl_debug_gem_out_of_range_flow, .parms = get_default },
            { .name = "bbh_rx_pon_cnpl_debug_usedwords", .val = cli_bbh_rx_bbh_rx_pon_cnpl_debug_usedwords, .parms = get_default },
            { .name = "gem_mode_gemmode", .val = cli_bbh_rx_gem_mode_gemmode, .parms = get_gem_mode_gemmode },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_bbh_rx_cli_get,
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
        BDMFMON_MAKE_PARM(dir, "test", "test", ag_drv_bbh_rx_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0)            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_cmd_parm_t ext_test_default[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "timer", .val = cli_bbh_rx_timer, .parms = ext_test_default},
            { .name = "flow_control_force", .val = cli_bbh_rx_flow_control_force, .parms = ext_test_default},
            { .name = "flow_control_runner_enable", .val = cli_bbh_rx_flow_control_runner_enable, .parms = ext_test_default},
            { .name = "flow_control_qm_enable", .val = cli_bbh_rx_flow_control_qm_enable, .parms = ext_test_default},
            { .name = "flow_ctrl_timer", .val = cli_bbh_rx_flow_ctrl_timer, .parms = ext_test_default},
            { .name = "flow_ctrl_force", .val = cli_bbh_rx_flow_ctrl_force, .parms = ext_test_default},
            { .name = "flow_ctrl_rnr_en", .val = cli_bbh_rx_flow_ctrl_rnr_en, .parms = ext_test_default},
            { .name = "general_configuration_mactype", .val = cli_bbh_rx_general_configuration_mactype, .parms = ext_test_default},
            { .name = "general_configuration_portth", .val = cli_bbh_rx_general_configuration_portth, .parms = ext_test_default},
            { .name = "general_configuration_sopoffset", .val = cli_bbh_rx_general_configuration_sopoffset, .parms = ext_test_default},
            { .name = "general_configuration_ingresspid", .val = cli_bbh_rx_general_configuration_ingresspid, .parms = ext_test_default},
            { .name = "general_configuration_portweight", .val = cli_bbh_rx_general_configuration_portweight, .parms = ext_test_default},
            { .name = "general_configuration_portviqmask", .val = cli_bbh_rx_general_configuration_portviqmask, .parms = ext_test_default},
            { .name = "general_configuration_excviqmask", .val = cli_bbh_rx_general_configuration_excviqmask, .parms = ext_test_default},
            { .name = "general_configuration_dispviq", .val = cli_bbh_rx_general_configuration_dispviq, .parms = ext_test_default},
            { .name = "general_configuration_viqweight", .val = cli_bbh_rx_general_configuration_viqweight, .parms = ext_test_default},
            { .name = "general_configuration_patterncfg", .val = cli_bbh_rx_general_configuration_patterncfg, .parms = ext_test_default},
            { .name = "general_configuration_patterndata", .val = cli_bbh_rx_general_configuration_patterndata, .parms = ext_test_default},
            { .name = "general_configuration_patternmask", .val = cli_bbh_rx_general_configuration_patternmask, .parms = ext_test_default},
            { .name = "general_configuration_patterndata8b", .val = cli_bbh_rx_general_configuration_patterndata8b, .parms = ext_test_default},
            { .name = "general_configuration_patternmask8b", .val = cli_bbh_rx_general_configuration_patternmask8b, .parms = ext_test_default},
            { .name = "general_configuration_patternen", .val = cli_bbh_rx_general_configuration_patternen, .parms = ext_test_default},
            { .name = "general_configuration_viqfcen", .val = cli_bbh_rx_general_configuration_viqfcen, .parms = ext_test_default},
            { .name = "general_configuration_noneth", .val = cli_bbh_rx_general_configuration_noneth, .parms = ext_test_default},
            { .name = "gem_mode_gemmode", .val = cli_bbh_rx_gem_mode_gemmode, .parms = ext_test_default},
            BDMFMON_ENUM_LAST
        };
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name = "low", .val = ag_drv_cli_test_method_low },
            { .name = "mid", .val = ag_drv_cli_test_method_mid },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "ext_test", "ext_test", ag_drv_bbh_rx_cli_ext_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0),
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0),
            BDMFMON_MAKE_PARM("start_idx", "array index to start test", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL),
            BDMFMON_MAKE_PARM("stop_idx", "array index to stop test", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name = "GENERAL_CONFIGURATION_BBCFG", .val = bdmf_address_general_configuration_bbcfg },
            { .name = "GENERAL_CONFIGURATION_ENABLE", .val = bdmf_address_general_configuration_enable },
            { .name = "GENERAL_CONFIGURATION_MINPKT", .val = bdmf_address_general_configuration_minpkt },
            { .name = "GENERAL_CONFIGURATION_MAXPKT", .val = bdmf_address_general_configuration_maxpkt },
            { .name = "GENERAL_CONFIGURATION_CLK_GATE_CNTRL", .val = bdmf_address_general_configuration_clk_gate_cntrl },
            { .name = "GENERAL_CONFIGURATION_RXDBGSEL", .val = bdmf_address_general_configuration_rxdbgsel },
            { .name = "GENERAL_CONFIGURATION_RXRSTRST", .val = bdmf_address_general_configuration_rxrstrst },
            { .name = "GENERAL_CONFIGURATION_SBPMCFG", .val = bdmf_address_general_configuration_sbpmcfg },
            { .name = "GENERAL_CONFIGURATION_PSRAMBASE", .val = bdmf_address_general_configuration_psrambase },
            { .name = "GENERAL_CONFIGURATION_INGRESSCHANNEL", .val = bdmf_address_general_configuration_ingresschannel },
            { .name = "GENERAL_CONFIGURATION_BBOVRDROUTE", .val = bdmf_address_general_configuration_bbovrdroute },
            { .name = "GENERAL_CONFIGURATION_BBOVRDCTRL", .val = bdmf_address_general_configuration_bbovrdctrl },
            { .name = "GENERAL_CONFIGURATION_GEMDONE", .val = bdmf_address_general_configuration_gemdone },
            { .name = "GENERAL_CONFIGURATION_MACTYPE", .val = bdmf_address_general_configuration_mactype },
            { .name = "GENERAL_CONFIGURATION_PORTTH", .val = bdmf_address_general_configuration_portth },
            { .name = "GENERAL_CONFIGURATION_FLOWCTRL", .val = bdmf_address_general_configuration_flowctrl },
            { .name = "GENERAL_CONFIGURATION_SOPOFFSET", .val = bdmf_address_general_configuration_sopoffset },
            { .name = "GENERAL_CONFIGURATION_INGRESSPID", .val = bdmf_address_general_configuration_ingresspid },
            { .name = "GENERAL_CONFIGURATION_PORTWEIGHT", .val = bdmf_address_general_configuration_portweight },
            { .name = "GENERAL_CONFIGURATION_PORTVIQMASK", .val = bdmf_address_general_configuration_portviqmask },
            { .name = "GENERAL_CONFIGURATION_EXCVIQMASK", .val = bdmf_address_general_configuration_excviqmask },
            { .name = "GENERAL_CONFIGURATION_DISPVIQ", .val = bdmf_address_general_configuration_dispviq },
            { .name = "GENERAL_CONFIGURATION_VIQWEIGHT", .val = bdmf_address_general_configuration_viqweight },
            { .name = "GENERAL_CONFIGURATION_PATTERNCFG", .val = bdmf_address_general_configuration_patterncfg },
            { .name = "GENERAL_CONFIGURATION_PATTERNDATA", .val = bdmf_address_general_configuration_patterndata },
            { .name = "GENERAL_CONFIGURATION_PATTERNMASK", .val = bdmf_address_general_configuration_patternmask },
            { .name = "GENERAL_CONFIGURATION_PATTERNDATA8B", .val = bdmf_address_general_configuration_patterndata8b },
            { .name = "GENERAL_CONFIGURATION_PATTERNMASK8B", .val = bdmf_address_general_configuration_patternmask8b },
            { .name = "GENERAL_CONFIGURATION_PATTERNEN", .val = bdmf_address_general_configuration_patternen },
            { .name = "GENERAL_CONFIGURATION_VIQFCEN", .val = bdmf_address_general_configuration_viqfcen },
            { .name = "GENERAL_CONFIGURATION_NONETH", .val = bdmf_address_general_configuration_noneth },
            { .name = "PM_COUNTERS_INPKT", .val = bdmf_address_pm_counters_inpkt },
            { .name = "PM_COUNTERS_INBYTE", .val = bdmf_address_pm_counters_inbyte },
            { .name = "PM_COUNTERS_TOOSHORT", .val = bdmf_address_pm_counters_tooshort },
            { .name = "PM_COUNTERS_TOOLONG", .val = bdmf_address_pm_counters_toolong },
            { .name = "PM_COUNTERS_CRCERROR", .val = bdmf_address_pm_counters_crcerror },
            { .name = "PM_COUNTERS_FCDROP", .val = bdmf_address_pm_counters_fcdrop },
            { .name = "PM_COUNTERS_TRUNCATE", .val = bdmf_address_pm_counters_truncate },
            { .name = "PM_COUNTERS_TOTALDROP", .val = bdmf_address_pm_counters_totaldrop },
            { .name = "PM_COUNTERS_DUMMYPDCNT", .val = bdmf_address_pm_counters_dummypdcnt },
            { .name = "PON_RMON_COUNTERS_R64", .val = bdmf_address_pon_rmon_counters_r64 },
            { .name = "PON_RMON_COUNTERS_R127", .val = bdmf_address_pon_rmon_counters_r127 },
            { .name = "PON_RMON_COUNTERS_R255", .val = bdmf_address_pon_rmon_counters_r255 },
            { .name = "PON_RMON_COUNTERS_R511", .val = bdmf_address_pon_rmon_counters_r511 },
            { .name = "PON_RMON_COUNTERS_R1023", .val = bdmf_address_pon_rmon_counters_r1023 },
            { .name = "PON_RMON_COUNTERS_R1518", .val = bdmf_address_pon_rmon_counters_r1518 },
            { .name = "PON_RMON_COUNTERS_R2047", .val = bdmf_address_pon_rmon_counters_r2047 },
            { .name = "PON_RMON_COUNTERS_R4095", .val = bdmf_address_pon_rmon_counters_r4095 },
            { .name = "PON_RMON_COUNTERS_R9216", .val = bdmf_address_pon_rmon_counters_r9216 },
            { .name = "PON_RMON_COUNTERS_R16383", .val = bdmf_address_pon_rmon_counters_r16383 },
            { .name = "DEBUG_PFBUW", .val = bdmf_address_debug_pfbuw },
            { .name = "DEBUG_PATTERNUW", .val = bdmf_address_debug_patternuw },
            { .name = "DEBUG_EXCUW", .val = bdmf_address_debug_excuw },
            { .name = "DEBUG_PORTUW", .val = bdmf_address_debug_portuw },
            { .name = "DEBUG_FIRSTSBN", .val = bdmf_address_debug_firstsbn },
            { .name = "DEBUG_CURRENTSBN", .val = bdmf_address_debug_currentsbn },
            { .name = "DEBUG_SN", .val = bdmf_address_debug_sn },
            { .name = "DEBUG_VIQMASK", .val = bdmf_address_debug_viqmask },
            { .name = "DEBUG_PLEN", .val = bdmf_address_debug_plen },
            { .name = "DEBUG_EOPOK", .val = bdmf_address_debug_eopok },
            { .name = "DEBUG_VIQFULL", .val = bdmf_address_debug_viqfull },
            { .name = "DEBUG_PORTREQ", .val = bdmf_address_debug_portreq },
            { .name = "DEBUG_DUMMYREQ", .val = bdmf_address_debug_dummyreq },
            { .name = "DEBUG_INREASSEMBLY", .val = bdmf_address_debug_inreassembly },
            { .name = "DEBUG_DUMMYPD", .val = bdmf_address_debug_dummypd },
            { .name = "DEBUG_UFUW", .val = bdmf_address_debug_ufuw },
            { .name = "DEBUG_PENDCONNECTUW", .val = bdmf_address_debug_pendconnectuw },
            { .name = "DEBUG_DBGVEC", .val = bdmf_address_debug_dbgvec },
            { .name = "DEBUG_PDUW", .val = bdmf_address_debug_pduw },
            { .name = "DEBUG_SBPMACK", .val = bdmf_address_debug_sbpmack },
            { .name = "DEBUG_WRITEREPLYACK", .val = bdmf_address_debug_writereplyack },
            { .name = "DEBUG_DISPCREDITACK", .val = bdmf_address_debug_dispcreditack },
            { .name = "DEBUG_PDRDDATA", .val = bdmf_address_debug_pdrddata },
            { .name = "DEBUG_SBNFIFO", .val = bdmf_address_debug_sbnfifo },
            { .name = "DEBUG_SBNPREFETCH", .val = bdmf_address_debug_sbnprefetch },
            { .name = "DEBUG_EXCFIFO", .val = bdmf_address_debug_excfifo },
            { .name = "BBH_RX_PON_CNPL_DEBUG_GEM_OUT_OF_RANGE", .val = bdmf_address_bbh_rx_pon_cnpl_debug_gem_out_of_range },
            { .name = "BBH_RX_PON_CNPL_DEBUG_GEM_OUT_OF_RANGE_FLOW", .val = bdmf_address_bbh_rx_pon_cnpl_debug_gem_out_of_range_flow },
            { .name = "BBH_RX_PON_CNPL_DEBUG_USEDWORDS", .val = bdmf_address_bbh_rx_pon_cnpl_debug_usedwords },
            { .name = "GEM_MODE_GEMMODE", .val = bdmf_address_gem_mode_gemmode },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "address", "address", bcm_bbh_rx_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index1", "bbh_id", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

    return dir;
}
#endif /* #ifdef USE_BDMF_SHELL */
