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
#include "xrdp_drv_bbh_rx_ag.h"

#define BLOCK_ADDR_COUNT_BITS 3
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_bbh_rx_ploam_en_set(uint8_t bbh_id, bdmf_boolean ploamen)
{
    uint32_t reg_general_configuration_exclqcfg=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (ploamen >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_EXCLQCFG, reg_general_configuration_exclqcfg);

    reg_general_configuration_exclqcfg = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_EXCLQCFG, PLOAMEN, reg_general_configuration_exclqcfg, ploamen);

    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_EXCLQCFG, reg_general_configuration_exclqcfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_ploam_en_get(uint8_t bbh_id, bdmf_boolean *ploamen)
{
    uint32_t reg_general_configuration_exclqcfg;

#ifdef VALIDATE_PARMS
    if(!ploamen)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_EXCLQCFG, reg_general_configuration_exclqcfg);

    *ploamen = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_EXCLQCFG, PLOAMEN, reg_general_configuration_exclqcfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_user_priority3_en_set(uint8_t bbh_id, bdmf_boolean pri3en)
{
    uint32_t reg_general_configuration_exclqcfg=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (pri3en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_EXCLQCFG, reg_general_configuration_exclqcfg);

    reg_general_configuration_exclqcfg = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_EXCLQCFG, PRI3EN, reg_general_configuration_exclqcfg, pri3en);

    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_EXCLQCFG, reg_general_configuration_exclqcfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_user_priority3_en_get(uint8_t bbh_id, bdmf_boolean *pri3en)
{
    uint32_t reg_general_configuration_exclqcfg;

#ifdef VALIDATE_PARMS
    if(!pri3en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_EXCLQCFG, reg_general_configuration_exclqcfg);

    *pri3en = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_EXCLQCFG, PRI3EN, reg_general_configuration_exclqcfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_pause_en_set(uint8_t bbh_id, bdmf_boolean pauseen)
{
    uint32_t reg_general_configuration_exclqcfg=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (pauseen >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_EXCLQCFG, reg_general_configuration_exclqcfg);

    reg_general_configuration_exclqcfg = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_EXCLQCFG, PAUSEEN, reg_general_configuration_exclqcfg, pauseen);

    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_EXCLQCFG, reg_general_configuration_exclqcfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_pause_en_get(uint8_t bbh_id, bdmf_boolean *pauseen)
{
    uint32_t reg_general_configuration_exclqcfg;

#ifdef VALIDATE_PARMS
    if(!pauseen)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_EXCLQCFG, reg_general_configuration_exclqcfg);

    *pauseen = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_EXCLQCFG, PAUSEEN, reg_general_configuration_exclqcfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_pfc_en_set(uint8_t bbh_id, bdmf_boolean pfcen)
{
    uint32_t reg_general_configuration_exclqcfg=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (pfcen >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_EXCLQCFG, reg_general_configuration_exclqcfg);

    reg_general_configuration_exclqcfg = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_EXCLQCFG, PFCEN, reg_general_configuration_exclqcfg, pfcen);

    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_EXCLQCFG, reg_general_configuration_exclqcfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_pfc_en_get(uint8_t bbh_id, bdmf_boolean *pfcen)
{
    uint32_t reg_general_configuration_exclqcfg;

#ifdef VALIDATE_PARMS
    if(!pfcen)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_EXCLQCFG, reg_general_configuration_exclqcfg);

    *pfcen = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_EXCLQCFG, PFCEN, reg_general_configuration_exclqcfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_ctrl_en_set(uint8_t bbh_id, bdmf_boolean ctrlen)
{
    uint32_t reg_general_configuration_exclqcfg=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (ctrlen >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_EXCLQCFG, reg_general_configuration_exclqcfg);

    reg_general_configuration_exclqcfg = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_EXCLQCFG, CTRLEN, reg_general_configuration_exclqcfg, ctrlen);

    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_EXCLQCFG, reg_general_configuration_exclqcfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_ctrl_en_get(uint8_t bbh_id, bdmf_boolean *ctrlen)
{
    uint32_t reg_general_configuration_exclqcfg;

#ifdef VALIDATE_PARMS
    if(!ctrlen)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_EXCLQCFG, reg_general_configuration_exclqcfg);

    *ctrlen = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_EXCLQCFG, CTRLEN, reg_general_configuration_exclqcfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_pattern_en_set(uint8_t bbh_id, bdmf_boolean patternen)
{
    uint32_t reg_general_configuration_exclqcfg=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (patternen >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_EXCLQCFG, reg_general_configuration_exclqcfg);

    reg_general_configuration_exclqcfg = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_EXCLQCFG, PATTERNEN, reg_general_configuration_exclqcfg, patternen);

    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_EXCLQCFG, reg_general_configuration_exclqcfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_pattern_en_get(uint8_t bbh_id, bdmf_boolean *patternen)
{
    uint32_t reg_general_configuration_exclqcfg;

#ifdef VALIDATE_PARMS
    if(!patternen)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_EXCLQCFG, reg_general_configuration_exclqcfg);

    *patternen = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_EXCLQCFG, PATTERNEN, reg_general_configuration_exclqcfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_exc_en_set(uint8_t bbh_id, bdmf_boolean excen)
{
    uint32_t reg_general_configuration_exclqcfg=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (excen >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_EXCLQCFG, reg_general_configuration_exclqcfg);

    reg_general_configuration_exclqcfg = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_EXCLQCFG, EXCEN, reg_general_configuration_exclqcfg, excen);

    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_EXCLQCFG, reg_general_configuration_exclqcfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_exc_en_get(uint8_t bbh_id, bdmf_boolean *excen)
{
    uint32_t reg_general_configuration_exclqcfg;

#ifdef VALIDATE_PARMS
    if(!excen)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_EXCLQCFG, reg_general_configuration_exclqcfg);

    *excen = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_EXCLQCFG, EXCEN, reg_general_configuration_exclqcfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_pattern_recog_set(uint8_t bbh_id, const bbh_rx_pattern_recog *pattern_recog)
{
    uint32_t reg_general_configuration_patterndatalsb=0;
    uint32_t reg_general_configuration_patterndatamsb=0;
    uint32_t reg_general_configuration_patternmasklsb=0;
    uint32_t reg_general_configuration_patternmaskmsb=0;
    uint32_t reg_general_configuration_exclqcfg=0;

#ifdef VALIDATE_PARMS
    if(!pattern_recog)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (pattern_recog->pattenoffset >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_EXCLQCFG, reg_general_configuration_exclqcfg);

    reg_general_configuration_patterndatalsb = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PATTERNDATALSB, PATTERNDATALSB, reg_general_configuration_patterndatalsb, pattern_recog->patterndatalsb);
    reg_general_configuration_patterndatamsb = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PATTERNDATAMSB, PATTERNDATAMSB, reg_general_configuration_patterndatamsb, pattern_recog->patterndatamsb);
    reg_general_configuration_patternmasklsb = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PATTERNMASKLSB, PATTERNMASKLSB, reg_general_configuration_patternmasklsb, pattern_recog->patternmasklsb);
    reg_general_configuration_patternmaskmsb = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PATTERNMASKMSB, PATTERNMASKMSB, reg_general_configuration_patternmaskmsb, pattern_recog->patternmaskmsb);
    reg_general_configuration_exclqcfg = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_EXCLQCFG, PATTENOFFSET, reg_general_configuration_exclqcfg, pattern_recog->pattenoffset);

    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PATTERNDATALSB, reg_general_configuration_patterndatalsb);
    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PATTERNDATAMSB, reg_general_configuration_patterndatamsb);
    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PATTERNMASKLSB, reg_general_configuration_patternmasklsb);
    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PATTERNMASKMSB, reg_general_configuration_patternmaskmsb);
    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_EXCLQCFG, reg_general_configuration_exclqcfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_pattern_recog_get(uint8_t bbh_id, bbh_rx_pattern_recog *pattern_recog)
{
    uint32_t reg_general_configuration_patterndatalsb;
    uint32_t reg_general_configuration_patterndatamsb;
    uint32_t reg_general_configuration_patternmasklsb;
    uint32_t reg_general_configuration_patternmaskmsb;
    uint32_t reg_general_configuration_exclqcfg;

#ifdef VALIDATE_PARMS
    if(!pattern_recog)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PATTERNDATALSB, reg_general_configuration_patterndatalsb);
    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PATTERNDATAMSB, reg_general_configuration_patterndatamsb);
    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PATTERNMASKLSB, reg_general_configuration_patternmasklsb);
    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PATTERNMASKMSB, reg_general_configuration_patternmaskmsb);
    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_EXCLQCFG, reg_general_configuration_exclqcfg);

    pattern_recog->patterndatalsb = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PATTERNDATALSB, PATTERNDATALSB, reg_general_configuration_patterndatalsb);
    pattern_recog->patterndatamsb = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PATTERNDATAMSB, PATTERNDATAMSB, reg_general_configuration_patterndatamsb);
    pattern_recog->patternmasklsb = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PATTERNMASKLSB, PATTERNMASKLSB, reg_general_configuration_patternmasklsb);
    pattern_recog->patternmaskmsb = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PATTERNMASKMSB, PATTERNMASKMSB, reg_general_configuration_patternmaskmsb);
    pattern_recog->pattenoffset = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_EXCLQCFG, PATTENOFFSET, reg_general_configuration_exclqcfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_flow_ctrl_timer_set(uint8_t bbh_id, uint32_t timer)
{
    uint32_t reg_general_configuration_flowctrl=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (timer >= _24BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, reg_general_configuration_flowctrl);

    reg_general_configuration_flowctrl = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, TIMER, reg_general_configuration_flowctrl, timer);

    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, reg_general_configuration_flowctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_flow_ctrl_timer_get(uint8_t bbh_id, uint32_t *timer)
{
    uint32_t reg_general_configuration_flowctrl;

#ifdef VALIDATE_PARMS
    if(!timer)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, reg_general_configuration_flowctrl);

    *timer = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, TIMER, reg_general_configuration_flowctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_flow_ctrl_force_set(uint8_t bbh_id, bdmf_boolean fcforce)
{
    uint32_t reg_general_configuration_flowctrl=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (fcforce >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, reg_general_configuration_flowctrl);

    reg_general_configuration_flowctrl = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, FCFORCE, reg_general_configuration_flowctrl, fcforce);

    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, reg_general_configuration_flowctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_flow_ctrl_force_get(uint8_t bbh_id, bdmf_boolean *fcforce)
{
    uint32_t reg_general_configuration_flowctrl;

#ifdef VALIDATE_PARMS
    if(!fcforce)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, reg_general_configuration_flowctrl);

    *fcforce = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, FCFORCE, reg_general_configuration_flowctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_flow_ctrl_drops_config_set(uint8_t bbh_id, const bbh_rx_flow_ctrl_drops_config *flow_ctrl_drops_config)
{
    uint32_t reg_general_configuration_flowctrl=0;

#ifdef VALIDATE_PARMS
    if(!flow_ctrl_drops_config)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (flow_ctrl_drops_config->dispdropdis >= _1BITS_MAX_VAL_) ||
       (flow_ctrl_drops_config->sdmadropdis >= _1BITS_MAX_VAL_) ||
       (flow_ctrl_drops_config->sbpmdropdis >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, reg_general_configuration_flowctrl);

    reg_general_configuration_flowctrl = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, DISPDROPDIS, reg_general_configuration_flowctrl, flow_ctrl_drops_config->dispdropdis);
    reg_general_configuration_flowctrl = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, SDMADROPDIS, reg_general_configuration_flowctrl, flow_ctrl_drops_config->sdmadropdis);
    reg_general_configuration_flowctrl = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, SBPMDROPDIS, reg_general_configuration_flowctrl, flow_ctrl_drops_config->sbpmdropdis);

    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, reg_general_configuration_flowctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_flow_ctrl_drops_config_get(uint8_t bbh_id, bbh_rx_flow_ctrl_drops_config *flow_ctrl_drops_config)
{
    uint32_t reg_general_configuration_flowctrl;

#ifdef VALIDATE_PARMS
    if(!flow_ctrl_drops_config)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, reg_general_configuration_flowctrl);

    flow_ctrl_drops_config->dispdropdis = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, DISPDROPDIS, reg_general_configuration_flowctrl);
    flow_ctrl_drops_config->sdmadropdis = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, SDMADROPDIS, reg_general_configuration_flowctrl);
    flow_ctrl_drops_config->sbpmdropdis = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL, SBPMDROPDIS, reg_general_configuration_flowctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_sdma_bb_id_set(uint8_t bbh_id, uint8_t sdmabbid)
{
    uint32_t reg_general_configuration_bbcfg=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (sdmabbid >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_BBCFG, reg_general_configuration_bbcfg);

    reg_general_configuration_bbcfg = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_BBCFG, SDMABBID, reg_general_configuration_bbcfg, sdmabbid);

    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_BBCFG, reg_general_configuration_bbcfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_sdma_bb_id_get(uint8_t bbh_id, uint8_t *sdmabbid)
{
    uint32_t reg_general_configuration_bbcfg;

#ifdef VALIDATE_PARMS
    if(!sdmabbid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_BBCFG, reg_general_configuration_bbcfg);

    *sdmabbid = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_BBCFG, SDMABBID, reg_general_configuration_bbcfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_dispatcher_sbpm_bb_id_set(uint8_t bbh_id, uint8_t dispbbid, uint8_t sbpmbbid)
{
    uint32_t reg_general_configuration_bbcfg=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (dispbbid >= _6BITS_MAX_VAL_) ||
       (sbpmbbid >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_BBCFG, reg_general_configuration_bbcfg);

    reg_general_configuration_bbcfg = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_BBCFG, DISPBBID, reg_general_configuration_bbcfg, dispbbid);
    reg_general_configuration_bbcfg = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_BBCFG, SBPMBBID, reg_general_configuration_bbcfg, sbpmbbid);

    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_BBCFG, reg_general_configuration_bbcfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_dispatcher_sbpm_bb_id_get(uint8_t bbh_id, uint8_t *dispbbid, uint8_t *sbpmbbid)
{
    uint32_t reg_general_configuration_bbcfg;

#ifdef VALIDATE_PARMS
    if(!dispbbid || !sbpmbbid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
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

bdmf_error_t ag_drv_bbh_rx_dispatcher_virtual_queues_set(uint8_t bbh_id, uint8_t normalviq, uint8_t exclviq)
{
    uint32_t reg_general_configuration_dispviq=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (normalviq >= _5BITS_MAX_VAL_) ||
       (exclviq >= _5BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_configuration_dispviq = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_DISPVIQ, NORMALVIQ, reg_general_configuration_dispviq, normalviq);
    reg_general_configuration_dispviq = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_DISPVIQ, EXCLVIQ, reg_general_configuration_dispviq, exclviq);

    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_DISPVIQ, reg_general_configuration_dispviq);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_dispatcher_virtual_queues_get(uint8_t bbh_id, uint8_t *normalviq, uint8_t *exclviq)
{
    uint32_t reg_general_configuration_dispviq;

#ifdef VALIDATE_PARMS
    if(!normalviq || !exclviq)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_DISPVIQ, reg_general_configuration_dispviq);

    *normalviq = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_DISPVIQ, NORMALVIQ, reg_general_configuration_dispviq);
    *exclviq = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_DISPVIQ, EXCLVIQ, reg_general_configuration_dispviq);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_sdma_config_set(uint8_t bbh_id, const bbh_rx_sdma_config *sdma_config)
{
    uint32_t reg_general_configuration_sdmacfg=0;
    uint32_t reg_general_configuration_sdmaaddr=0;

#ifdef VALIDATE_PARMS
    if(!sdma_config)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (sdma_config->numofcd >= _7BITS_MAX_VAL_) ||
       (sdma_config->exclth >= _7BITS_MAX_VAL_) ||
       (sdma_config->database >= _6BITS_MAX_VAL_) ||
       (sdma_config->descbase >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_SDMACFG, reg_general_configuration_sdmacfg);

    reg_general_configuration_sdmacfg = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_SDMACFG, NUMOFCD, reg_general_configuration_sdmacfg, sdma_config->numofcd);
    reg_general_configuration_sdmacfg = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_SDMACFG, EXCLTH, reg_general_configuration_sdmacfg, sdma_config->exclth);
    reg_general_configuration_sdmaaddr = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_SDMAADDR, DATABASE, reg_general_configuration_sdmaaddr, sdma_config->database);
    reg_general_configuration_sdmaaddr = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_SDMAADDR, DESCBASE, reg_general_configuration_sdmaaddr, sdma_config->descbase);

    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_SDMACFG, reg_general_configuration_sdmacfg);
    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_SDMAADDR, reg_general_configuration_sdmaaddr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_sdma_config_get(uint8_t bbh_id, bbh_rx_sdma_config *sdma_config)
{
    uint32_t reg_general_configuration_sdmacfg;
    uint32_t reg_general_configuration_sdmaaddr;

#ifdef VALIDATE_PARMS
    if(!sdma_config)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_SDMACFG, reg_general_configuration_sdmacfg);
    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_SDMAADDR, reg_general_configuration_sdmaaddr);

    sdma_config->numofcd = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_SDMACFG, NUMOFCD, reg_general_configuration_sdmacfg);
    sdma_config->exclth = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_SDMACFG, EXCLTH, reg_general_configuration_sdmacfg);
    sdma_config->database = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_SDMAADDR, DATABASE, reg_general_configuration_sdmaaddr);
    sdma_config->descbase = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_SDMAADDR, DESCBASE, reg_general_configuration_sdmaaddr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_pkt_size0_set(uint8_t bbh_id, uint8_t minpkt0, uint16_t maxpkt0)
{
    uint32_t reg_general_configuration_minpkt0=0;
    uint32_t reg_general_configuration_maxpkt0=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (maxpkt0 >= _14BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MINPKT0, reg_general_configuration_minpkt0);
    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MAXPKT0, reg_general_configuration_maxpkt0);

    reg_general_configuration_minpkt0 = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MINPKT0, MINPKT0, reg_general_configuration_minpkt0, minpkt0);
    reg_general_configuration_maxpkt0 = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MAXPKT0, MAXPKT0, reg_general_configuration_maxpkt0, maxpkt0);

    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MINPKT0, reg_general_configuration_minpkt0);
    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MAXPKT0, reg_general_configuration_maxpkt0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_pkt_size0_get(uint8_t bbh_id, uint8_t *minpkt0, uint16_t *maxpkt0)
{
    uint32_t reg_general_configuration_minpkt0;
    uint32_t reg_general_configuration_maxpkt0;

#ifdef VALIDATE_PARMS
    if(!minpkt0 || !maxpkt0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MINPKT0, reg_general_configuration_minpkt0);
    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MAXPKT0, reg_general_configuration_maxpkt0);

    *minpkt0 = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MINPKT0, MINPKT0, reg_general_configuration_minpkt0);
    *maxpkt0 = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MAXPKT0, MAXPKT0, reg_general_configuration_maxpkt0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_pkt_size1_set(uint8_t bbh_id, uint8_t minpkt1, uint16_t maxpkt1)
{
    uint32_t reg_general_configuration_minpkt0=0;
    uint32_t reg_general_configuration_maxpkt0=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (maxpkt1 >= _14BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MINPKT0, reg_general_configuration_minpkt0);
    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MAXPKT0, reg_general_configuration_maxpkt0);

    reg_general_configuration_minpkt0 = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MINPKT0, MINPKT1, reg_general_configuration_minpkt0, minpkt1);
    reg_general_configuration_maxpkt0 = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MAXPKT0, MAXPKT1, reg_general_configuration_maxpkt0, maxpkt1);

    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MINPKT0, reg_general_configuration_minpkt0);
    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MAXPKT0, reg_general_configuration_maxpkt0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_pkt_size1_get(uint8_t bbh_id, uint8_t *minpkt1, uint16_t *maxpkt1)
{
    uint32_t reg_general_configuration_minpkt0;
    uint32_t reg_general_configuration_maxpkt0;

#ifdef VALIDATE_PARMS
    if(!minpkt1 || !maxpkt1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MINPKT0, reg_general_configuration_minpkt0);
    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MAXPKT0, reg_general_configuration_maxpkt0);

    *minpkt1 = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MINPKT0, MINPKT1, reg_general_configuration_minpkt0);
    *maxpkt1 = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MAXPKT0, MAXPKT1, reg_general_configuration_maxpkt0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_pkt_size2_set(uint8_t bbh_id, uint8_t minpkt2, uint16_t maxpkt2)
{
    uint32_t reg_general_configuration_minpkt0=0;
    uint32_t reg_general_configuration_maxpkt1=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (maxpkt2 >= _14BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MINPKT0, reg_general_configuration_minpkt0);
    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MAXPKT1, reg_general_configuration_maxpkt1);

    reg_general_configuration_minpkt0 = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MINPKT0, MINPKT2, reg_general_configuration_minpkt0, minpkt2);
    reg_general_configuration_maxpkt1 = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MAXPKT1, MAXPKT2, reg_general_configuration_maxpkt1, maxpkt2);

    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MINPKT0, reg_general_configuration_minpkt0);
    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MAXPKT1, reg_general_configuration_maxpkt1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_pkt_size2_get(uint8_t bbh_id, uint8_t *minpkt2, uint16_t *maxpkt2)
{
    uint32_t reg_general_configuration_minpkt0;
    uint32_t reg_general_configuration_maxpkt1;

#ifdef VALIDATE_PARMS
    if(!minpkt2 || !maxpkt2)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MINPKT0, reg_general_configuration_minpkt0);
    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MAXPKT1, reg_general_configuration_maxpkt1);

    *minpkt2 = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MINPKT0, MINPKT2, reg_general_configuration_minpkt0);
    *maxpkt2 = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MAXPKT1, MAXPKT2, reg_general_configuration_maxpkt1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_pkt_size3_set(uint8_t bbh_id, uint8_t minpkt3, uint16_t maxpkt3)
{
    uint32_t reg_general_configuration_minpkt0=0;
    uint32_t reg_general_configuration_maxpkt1=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (maxpkt3 >= _14BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MINPKT0, reg_general_configuration_minpkt0);
    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MAXPKT1, reg_general_configuration_maxpkt1);

    reg_general_configuration_minpkt0 = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MINPKT0, MINPKT3, reg_general_configuration_minpkt0, minpkt3);
    reg_general_configuration_maxpkt1 = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MAXPKT1, MAXPKT3, reg_general_configuration_maxpkt1, maxpkt3);

    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MINPKT0, reg_general_configuration_minpkt0);
    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MAXPKT1, reg_general_configuration_maxpkt1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_pkt_size3_get(uint8_t bbh_id, uint8_t *minpkt3, uint16_t *maxpkt3)
{
    uint32_t reg_general_configuration_minpkt0;
    uint32_t reg_general_configuration_maxpkt1;

#ifdef VALIDATE_PARMS
    if(!minpkt3 || !maxpkt3)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MINPKT0, reg_general_configuration_minpkt0);
    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MAXPKT1, reg_general_configuration_maxpkt1);

    *minpkt3 = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MINPKT0, MINPKT3, reg_general_configuration_minpkt0);
    *maxpkt3 = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MAXPKT1, MAXPKT3, reg_general_configuration_maxpkt1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_pkt_sel_group_0_set(uint8_t bbh_id, uint8_t minpktsel0, uint8_t maxpktsel0)
{
    uint32_t reg_general_configuration_perflowsets=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (minpktsel0 >= _2BITS_MAX_VAL_) ||
       (maxpktsel0 >= _2BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PERFLOWSETS, reg_general_configuration_perflowsets);

    reg_general_configuration_perflowsets = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PERFLOWSETS, MINPKTSEL0, reg_general_configuration_perflowsets, minpktsel0);
    reg_general_configuration_perflowsets = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PERFLOWSETS, MAXPKTSEL0, reg_general_configuration_perflowsets, maxpktsel0);

    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PERFLOWSETS, reg_general_configuration_perflowsets);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_pkt_sel_group_0_get(uint8_t bbh_id, uint8_t *minpktsel0, uint8_t *maxpktsel0)
{
    uint32_t reg_general_configuration_perflowsets;

#ifdef VALIDATE_PARMS
    if(!minpktsel0 || !maxpktsel0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PERFLOWSETS, reg_general_configuration_perflowsets);

    *minpktsel0 = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PERFLOWSETS, MINPKTSEL0, reg_general_configuration_perflowsets);
    *maxpktsel0 = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PERFLOWSETS, MAXPKTSEL0, reg_general_configuration_perflowsets);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_pkt_sel_group_1_set(uint8_t bbh_id, uint8_t minpktsel1, uint8_t maxpktsel1)
{
    uint32_t reg_general_configuration_perflowsets=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (minpktsel1 >= _2BITS_MAX_VAL_) ||
       (maxpktsel1 >= _2BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PERFLOWSETS, reg_general_configuration_perflowsets);

    reg_general_configuration_perflowsets = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PERFLOWSETS, MINPKTSEL1, reg_general_configuration_perflowsets, minpktsel1);
    reg_general_configuration_perflowsets = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PERFLOWSETS, MAXPKTSEL1, reg_general_configuration_perflowsets, maxpktsel1);

    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PERFLOWSETS, reg_general_configuration_perflowsets);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_pkt_sel_group_1_get(uint8_t bbh_id, uint8_t *minpktsel1, uint8_t *maxpktsel1)
{
    uint32_t reg_general_configuration_perflowsets;

#ifdef VALIDATE_PARMS
    if(!minpktsel1 || !maxpktsel1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PERFLOWSETS, reg_general_configuration_perflowsets);

    *minpktsel1 = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PERFLOWSETS, MINPKTSEL1, reg_general_configuration_perflowsets);
    *maxpktsel1 = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PERFLOWSETS, MAXPKTSEL1, reg_general_configuration_perflowsets);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_error_pm_counters_get(uint8_t bbh_id, bbh_rx_error_pm_counters *error_pm_counters)
{
    uint32_t reg_pm_counters_crcerrorploam;
    uint32_t reg_pm_counters_thirdflow;
    uint32_t reg_pm_counters_sopasop;
    uint32_t reg_pm_counters_nosbpmsbnploam;

#ifdef VALIDATE_PARMS
    if(!error_pm_counters)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, PM_COUNTERS_CRCERRORPLOAM, reg_pm_counters_crcerrorploam);
    RU_REG_READ(bbh_id, BBH_RX, PM_COUNTERS_THIRDFLOW, reg_pm_counters_thirdflow);
    RU_REG_READ(bbh_id, BBH_RX, PM_COUNTERS_SOPASOP, reg_pm_counters_sopasop);
    RU_REG_READ(bbh_id, BBH_RX, PM_COUNTERS_NOSBPMSBNPLOAM, reg_pm_counters_nosbpmsbnploam);

    error_pm_counters->crc_err_ploam = RU_FIELD_GET(bbh_id, BBH_RX, PM_COUNTERS_CRCERRORPLOAM, PMVALUE, reg_pm_counters_crcerrorploam);
    error_pm_counters->third_flow = RU_FIELD_GET(bbh_id, BBH_RX, PM_COUNTERS_THIRDFLOW, PMVALUE, reg_pm_counters_thirdflow);
    error_pm_counters->sop_after_sop = RU_FIELD_GET(bbh_id, BBH_RX, PM_COUNTERS_SOPASOP, PMVALUE, reg_pm_counters_sopasop);
    error_pm_counters->no_sbpm_bn_ploam = RU_FIELD_GET(bbh_id, BBH_RX, PM_COUNTERS_NOSBPMSBNPLOAM, PMVALUE, reg_pm_counters_nosbpmsbnploam);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_pm_counters_get(uint8_t bbh_id, bbh_rx_pm_counters *pm_counters)
{
    uint32_t reg_pm_counters_inpkt;
    uint32_t reg_pm_counters_crcerror;
    uint32_t reg_pm_counters_tooshort;
    uint32_t reg_pm_counters_toolong;
    uint32_t reg_pm_counters_nosbpmsbn;
    uint32_t reg_pm_counters_dispcong;
    uint32_t reg_pm_counters_nosdmacd;
    uint32_t reg_pm_counters_nosdmacdploam;
    uint32_t reg_pm_counters_dispcongploam;

#ifdef VALIDATE_PARMS
    if(!pm_counters)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, PM_COUNTERS_INPKT, reg_pm_counters_inpkt);
    RU_REG_READ(bbh_id, BBH_RX, PM_COUNTERS_CRCERROR, reg_pm_counters_crcerror);
    RU_REG_READ(bbh_id, BBH_RX, PM_COUNTERS_TOOSHORT, reg_pm_counters_tooshort);
    RU_REG_READ(bbh_id, BBH_RX, PM_COUNTERS_TOOLONG, reg_pm_counters_toolong);
    RU_REG_READ(bbh_id, BBH_RX, PM_COUNTERS_NOSBPMSBN, reg_pm_counters_nosbpmsbn);
    RU_REG_READ(bbh_id, BBH_RX, PM_COUNTERS_DISPCONG, reg_pm_counters_dispcong);
    RU_REG_READ(bbh_id, BBH_RX, PM_COUNTERS_NOSDMACD, reg_pm_counters_nosdmacd);
    RU_REG_READ(bbh_id, BBH_RX, PM_COUNTERS_NOSDMACDPLOAM, reg_pm_counters_nosdmacdploam);
    RU_REG_READ(bbh_id, BBH_RX, PM_COUNTERS_DISPCONGPLOAM, reg_pm_counters_dispcongploam);

    pm_counters->inpkt = RU_FIELD_GET(bbh_id, BBH_RX, PM_COUNTERS_INPKT, INPKT, reg_pm_counters_inpkt);
    pm_counters->crc_err = RU_FIELD_GET(bbh_id, BBH_RX, PM_COUNTERS_CRCERROR, PMVALUE, reg_pm_counters_crcerror);
    pm_counters->too_short = RU_FIELD_GET(bbh_id, BBH_RX, PM_COUNTERS_TOOSHORT, PMVALUE, reg_pm_counters_tooshort);
    pm_counters->too_long = RU_FIELD_GET(bbh_id, BBH_RX, PM_COUNTERS_TOOLONG, PMVALUE, reg_pm_counters_toolong);
    pm_counters->no_sbpm_sbn = RU_FIELD_GET(bbh_id, BBH_RX, PM_COUNTERS_NOSBPMSBN, PMVALUE, reg_pm_counters_nosbpmsbn);
    pm_counters->disp_cong = RU_FIELD_GET(bbh_id, BBH_RX, PM_COUNTERS_DISPCONG, PMVALUE, reg_pm_counters_dispcong);
    pm_counters->no_sdma_cd = RU_FIELD_GET(bbh_id, BBH_RX, PM_COUNTERS_NOSDMACD, PMVALUE, reg_pm_counters_nosdmacd);
    pm_counters->ploam_no_sdma_cd = RU_FIELD_GET(bbh_id, BBH_RX, PM_COUNTERS_NOSDMACDPLOAM, PMVALUE, reg_pm_counters_nosdmacdploam);
    pm_counters->ploam_disp_cong = RU_FIELD_GET(bbh_id, BBH_RX, PM_COUNTERS_DISPCONGPLOAM, PMVALUE, reg_pm_counters_dispcongploam);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_mac_mode_set(uint8_t bbh_id, bdmf_boolean macmode, bdmf_boolean gponmode, bdmf_boolean macvdsl)
{
    uint32_t reg_general_configuration_macmode=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (macmode >= _1BITS_MAX_VAL_) ||
       (gponmode >= _1BITS_MAX_VAL_) ||
       (macvdsl >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_configuration_macmode = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MACMODE, MACMODE, reg_general_configuration_macmode, macmode);
    reg_general_configuration_macmode = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MACMODE, GPONMODE, reg_general_configuration_macmode, gponmode);
    reg_general_configuration_macmode = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MACMODE, MACVDSL, reg_general_configuration_macmode, macvdsl);

    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MACMODE, reg_general_configuration_macmode);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_mac_mode_get(uint8_t bbh_id, bdmf_boolean *macmode, bdmf_boolean *gponmode, bdmf_boolean *macvdsl)
{
    uint32_t reg_general_configuration_macmode;

#ifdef VALIDATE_PARMS
    if(!macmode || !gponmode || !macvdsl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MACMODE, reg_general_configuration_macmode);

    *macmode = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MACMODE, MACMODE, reg_general_configuration_macmode);
    *gponmode = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MACMODE, GPONMODE, reg_general_configuration_macmode);
    *macvdsl = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MACMODE, MACVDSL, reg_general_configuration_macmode);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_sopoffset_set(uint8_t bbh_id, uint8_t sopoffset)
{
    uint32_t reg_general_configuration_sopoffset=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (sopoffset >= _7BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_configuration_sopoffset = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_SOPOFFSET, SOPOFFSET, reg_general_configuration_sopoffset, sopoffset);

    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_SOPOFFSET, reg_general_configuration_sopoffset);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_sopoffset_get(uint8_t bbh_id, uint8_t *sopoffset)
{
    uint32_t reg_general_configuration_sopoffset;

#ifdef VALIDATE_PARMS
    if(!sopoffset)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_SOPOFFSET, reg_general_configuration_sopoffset);

    *sopoffset = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_SOPOFFSET, SOPOFFSET, reg_general_configuration_sopoffset);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_crcomitdis_set(uint8_t bbh_id, bdmf_boolean crcomitdis)
{
    uint32_t reg_general_configuration_crcomitdis=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (crcomitdis >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_configuration_crcomitdis = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_CRCOMITDIS, CRCOMITDIS, reg_general_configuration_crcomitdis, crcomitdis);

    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_CRCOMITDIS, reg_general_configuration_crcomitdis);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_crcomitdis_get(uint8_t bbh_id, bdmf_boolean *crcomitdis)
{
    uint32_t reg_general_configuration_crcomitdis;

#ifdef VALIDATE_PARMS
    if(!crcomitdis)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_CRCOMITDIS, reg_general_configuration_crcomitdis);

    *crcomitdis = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_CRCOMITDIS, CRCOMITDIS, reg_general_configuration_crcomitdis);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_enable_set(uint8_t bbh_id, bdmf_boolean pkten, bdmf_boolean sbpmen)
{
    uint32_t reg_general_configuration_enable=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (pkten >= _1BITS_MAX_VAL_) ||
       (sbpmen >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_configuration_enable = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_ENABLE, PKTEN, reg_general_configuration_enable, pkten);
    reg_general_configuration_enable = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_ENABLE, SBPMEN, reg_general_configuration_enable, sbpmen);

    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_ENABLE, reg_general_configuration_enable);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_enable_get(uint8_t bbh_id, bdmf_boolean *pkten, bdmf_boolean *sbpmen)
{
    uint32_t reg_general_configuration_enable;

#ifdef VALIDATE_PARMS
    if(!pkten || !sbpmen)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_ENABLE, reg_general_configuration_enable);

    *pkten = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_ENABLE, PKTEN, reg_general_configuration_enable);
    *sbpmen = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_ENABLE, SBPMEN, reg_general_configuration_enable);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_g9991en_set(uint8_t bbh_id, bdmf_boolean enable, bdmf_boolean bytes4_7enable)
{
    uint32_t reg_general_configuration_g9991en=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (enable >= _1BITS_MAX_VAL_) ||
       (bytes4_7enable >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_configuration_g9991en = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_G9991EN, ENABLE, reg_general_configuration_g9991en, enable);
    reg_general_configuration_g9991en = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_G9991EN, BYTES4_7ENABLE, reg_general_configuration_g9991en, bytes4_7enable);

    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_G9991EN, reg_general_configuration_g9991en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_g9991en_get(uint8_t bbh_id, bdmf_boolean *enable, bdmf_boolean *bytes4_7enable)
{
    uint32_t reg_general_configuration_g9991en;

#ifdef VALIDATE_PARMS
    if(!enable || !bytes4_7enable)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_G9991EN, reg_general_configuration_g9991en);

    *enable = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_G9991EN, ENABLE, reg_general_configuration_g9991en);
    *bytes4_7enable = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_G9991EN, BYTES4_7ENABLE, reg_general_configuration_g9991en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_perflowth_set(uint8_t bbh_id, uint8_t flowth)
{
    uint32_t reg_general_configuration_perflowth=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_configuration_perflowth = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PERFLOWTH, FLOWTH, reg_general_configuration_perflowth, flowth);

    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PERFLOWTH, reg_general_configuration_perflowth);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_perflowth_get(uint8_t bbh_id, uint8_t *flowth)
{
    uint32_t reg_general_configuration_perflowth;

#ifdef VALIDATE_PARMS
    if(!flowth)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PERFLOWTH, reg_general_configuration_perflowth);

    *flowth = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_PERFLOWTH, FLOWTH, reg_general_configuration_perflowth);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_min_pkt_sel_flows_0_15_set(uint8_t bbh_id, uint8_t flow_id, uint8_t sel_id)
{
    uint32_t reg_general_configuration_minpktsel0=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MINPKTSEL0, reg_general_configuration_minpktsel0);

    FIELD_SET(reg_general_configuration_minpktsel0, (flow_id % 16) *2, 0x2, sel_id);

    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MINPKTSEL0, reg_general_configuration_minpktsel0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_min_pkt_sel_flows_0_15_get(uint8_t bbh_id, uint8_t flow_id, uint8_t *sel_id)
{
    uint32_t reg_general_configuration_minpktsel0;

#ifdef VALIDATE_PARMS
    if(!sel_id)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MINPKTSEL0, reg_general_configuration_minpktsel0);

    *sel_id = FIELD_GET(reg_general_configuration_minpktsel0, (flow_id % 16) *2, 0x2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_min_pkt_sel_flows_16_31_set(uint8_t bbh_id, uint8_t flow_id, uint8_t sel_id)
{
    uint32_t reg_general_configuration_minpktsel1=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MINPKTSEL1, reg_general_configuration_minpktsel1);

    FIELD_SET(reg_general_configuration_minpktsel1, (flow_id % 16) *2, 0x2, sel_id);

    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MINPKTSEL1, reg_general_configuration_minpktsel1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_min_pkt_sel_flows_16_31_get(uint8_t bbh_id, uint8_t flow_id, uint8_t *sel_id)
{
    uint32_t reg_general_configuration_minpktsel1;

#ifdef VALIDATE_PARMS
    if(!sel_id)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MINPKTSEL1, reg_general_configuration_minpktsel1);

    *sel_id = FIELD_GET(reg_general_configuration_minpktsel1, (flow_id % 16) *2, 0x2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_max_pkt_sel_flows_0_15_set(uint8_t bbh_id, uint8_t flow_id, uint8_t sel_id)
{
    /* Identical to min_pkt_sel_flows_0_15 */
    uint32_t reg_general_configuration_minpktsel0=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MAXPKTSEL0, reg_general_configuration_minpktsel0);

    FIELD_SET(reg_general_configuration_minpktsel0, (flow_id % 16) *2, 0x2, sel_id);

    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MAXPKTSEL0, reg_general_configuration_minpktsel0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_max_pkt_sel_flows_0_15_get(uint8_t bbh_id, uint8_t flow_id, uint8_t *sel_id)
{
    /* Identical to min_pkt_sel_flows_0_15 */
    uint32_t reg_general_configuration_minpktsel0;

#ifdef VALIDATE_PARMS
    if(!sel_id)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MAXPKTSEL0, reg_general_configuration_minpktsel0);

    *sel_id = FIELD_GET(reg_general_configuration_minpktsel0, (flow_id % 16) *2, 0x2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_max_pkt_sel_flows_16_31_set(uint8_t bbh_id, uint8_t flow_id, uint8_t sel_id)
{
    /* Identical to min_pkt_sel_flows_16_31 */
    uint32_t reg_general_configuration_minpktsel1=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MAXPKTSEL1, reg_general_configuration_minpktsel1);

    FIELD_SET(reg_general_configuration_minpktsel1, (flow_id % 16) *2, 0x2, sel_id);

    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MAXPKTSEL1, reg_general_configuration_minpktsel1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_max_pkt_sel_flows_16_31_get(uint8_t bbh_id, uint8_t flow_id, uint8_t *sel_id)
{
    /* Identical to min_pkt_sel_flows_16_31 */
    uint32_t reg_general_configuration_minpktsel1;

#ifdef VALIDATE_PARMS
    if(!sel_id)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_MAXPKTSEL1, reg_general_configuration_minpktsel1);

    *sel_id = FIELD_GET(reg_general_configuration_minpktsel1, (flow_id % 16) *2, 0x2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_sbpmcfg_set(uint8_t bbh_id, uint8_t max_otf_sbpm_req)
{
    uint32_t reg_general_configuration_sbpmcfg=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (max_otf_sbpm_req >= _4BITS_MAX_VAL_))
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
    if(!max_otf_sbpm_req)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_SBPMCFG, reg_general_configuration_sbpmcfg);

    *max_otf_sbpm_req = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_SBPMCFG, MAXREQ, reg_general_configuration_sbpmcfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_rxrstrst_set(uint8_t bbh_id, const bbh_rx_general_configuration_rxrstrst *general_configuration_rxrstrst)
{
    uint32_t reg_general_configuration_rxrstrst=0;

#ifdef VALIDATE_PARMS
    if(!general_configuration_rxrstrst)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (general_configuration_rxrstrst->inbufrst >= _1BITS_MAX_VAL_) ||
       (general_configuration_rxrstrst->burstbufrst >= _1BITS_MAX_VAL_) ||
       (general_configuration_rxrstrst->ingresscntxt >= _1BITS_MAX_VAL_) ||
       (general_configuration_rxrstrst->cmdfiforst >= _1BITS_MAX_VAL_) ||
       (general_configuration_rxrstrst->sbpmfiforst >= _1BITS_MAX_VAL_) ||
       (general_configuration_rxrstrst->coherencyfiforst >= _1BITS_MAX_VAL_) ||
       (general_configuration_rxrstrst->cntxtrst >= _1BITS_MAX_VAL_) ||
       (general_configuration_rxrstrst->sdmarst >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_configuration_rxrstrst = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_RXRSTRST, INBUFRST, reg_general_configuration_rxrstrst, general_configuration_rxrstrst->inbufrst);
    reg_general_configuration_rxrstrst = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_RXRSTRST, BURSTBUFRST, reg_general_configuration_rxrstrst, general_configuration_rxrstrst->burstbufrst);
    reg_general_configuration_rxrstrst = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_RXRSTRST, INGRESSCNTXT, reg_general_configuration_rxrstrst, general_configuration_rxrstrst->ingresscntxt);
    reg_general_configuration_rxrstrst = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_RXRSTRST, CMDFIFORST, reg_general_configuration_rxrstrst, general_configuration_rxrstrst->cmdfiforst);
    reg_general_configuration_rxrstrst = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_RXRSTRST, SBPMFIFORST, reg_general_configuration_rxrstrst, general_configuration_rxrstrst->sbpmfiforst);
    reg_general_configuration_rxrstrst = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_RXRSTRST, COHERENCYFIFORST, reg_general_configuration_rxrstrst, general_configuration_rxrstrst->coherencyfiforst);
    reg_general_configuration_rxrstrst = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_RXRSTRST, CNTXTRST, reg_general_configuration_rxrstrst, general_configuration_rxrstrst->cntxtrst);
    reg_general_configuration_rxrstrst = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_RXRSTRST, SDMARST, reg_general_configuration_rxrstrst, general_configuration_rxrstrst->sdmarst);

    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_RXRSTRST, reg_general_configuration_rxrstrst);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_rxrstrst_get(uint8_t bbh_id, bbh_rx_general_configuration_rxrstrst *general_configuration_rxrstrst)
{
    uint32_t reg_general_configuration_rxrstrst;

#ifdef VALIDATE_PARMS
    if(!general_configuration_rxrstrst)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_RXRSTRST, reg_general_configuration_rxrstrst);

    general_configuration_rxrstrst->inbufrst = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_RXRSTRST, INBUFRST, reg_general_configuration_rxrstrst);
    general_configuration_rxrstrst->burstbufrst = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_RXRSTRST, BURSTBUFRST, reg_general_configuration_rxrstrst);
    general_configuration_rxrstrst->ingresscntxt = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_RXRSTRST, INGRESSCNTXT, reg_general_configuration_rxrstrst);
    general_configuration_rxrstrst->cmdfiforst = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_RXRSTRST, CMDFIFORST, reg_general_configuration_rxrstrst);
    general_configuration_rxrstrst->sbpmfiforst = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_RXRSTRST, SBPMFIFORST, reg_general_configuration_rxrstrst);
    general_configuration_rxrstrst->coherencyfiforst = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_RXRSTRST, COHERENCYFIFORST, reg_general_configuration_rxrstrst);
    general_configuration_rxrstrst->cntxtrst = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_RXRSTRST, CNTXTRST, reg_general_configuration_rxrstrst);
    general_configuration_rxrstrst->sdmarst = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_RXRSTRST, SDMARST, reg_general_configuration_rxrstrst);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_rxdbgsel_set(uint8_t bbh_id, uint8_t rxdbgsel)
{
    uint32_t reg_general_configuration_rxdbgsel=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (rxdbgsel >= _4BITS_MAX_VAL_))
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
    if(!rxdbgsel)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_RXDBGSEL, reg_general_configuration_rxdbgsel);

    *rxdbgsel = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_RXDBGSEL, RXDBGSEL, reg_general_configuration_rxdbgsel);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_bbhrx_raddr_decoder_set(uint8_t bbh_id, uint8_t id_2overwr, uint16_t overwr_ra, bdmf_boolean overwr_en)
{
    uint32_t reg_general_configuration_bbhrx_raddr_decoder=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (id_2overwr >= _6BITS_MAX_VAL_) ||
       (overwr_ra >= _10BITS_MAX_VAL_) ||
       (overwr_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_configuration_bbhrx_raddr_decoder = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER, ID_2OVERWR, reg_general_configuration_bbhrx_raddr_decoder, id_2overwr);
    reg_general_configuration_bbhrx_raddr_decoder = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER, OVERWR_RA, reg_general_configuration_bbhrx_raddr_decoder, overwr_ra);
    reg_general_configuration_bbhrx_raddr_decoder = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER, OVERWR_EN, reg_general_configuration_bbhrx_raddr_decoder, overwr_en);

    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER, reg_general_configuration_bbhrx_raddr_decoder);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_bbhrx_raddr_decoder_get(uint8_t bbh_id, uint8_t *id_2overwr, uint16_t *overwr_ra, bdmf_boolean *overwr_en)
{
    uint32_t reg_general_configuration_bbhrx_raddr_decoder;

#ifdef VALIDATE_PARMS
    if(!id_2overwr || !overwr_ra || !overwr_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER, reg_general_configuration_bbhrx_raddr_decoder);

    *id_2overwr = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER, ID_2OVERWR, reg_general_configuration_bbhrx_raddr_decoder);
    *overwr_ra = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER, OVERWR_RA, reg_general_configuration_bbhrx_raddr_decoder);
    *overwr_en = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER, OVERWR_EN, reg_general_configuration_bbhrx_raddr_decoder);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_noneth_set(uint8_t bbh_id, uint8_t flowid, bdmf_boolean enable)
{
    uint32_t reg_general_configuration_noneth=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (enable >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_general_configuration_noneth = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_NONETH, FLOWID, reg_general_configuration_noneth, flowid);
    reg_general_configuration_noneth = RU_FIELD_SET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_NONETH, ENABLE, reg_general_configuration_noneth, enable);

    RU_REG_WRITE(bbh_id, BBH_RX, GENERAL_CONFIGURATION_NONETH, reg_general_configuration_noneth);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_noneth_get(uint8_t bbh_id, uint8_t *flowid, bdmf_boolean *enable)
{
    uint32_t reg_general_configuration_noneth;

#ifdef VALIDATE_PARMS
    if(!flowid || !enable)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, GENERAL_CONFIGURATION_NONETH, reg_general_configuration_noneth);

    *flowid = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_NONETH, FLOWID, reg_general_configuration_noneth);
    *enable = RU_FIELD_GET(bbh_id, BBH_RX, GENERAL_CONFIGURATION_NONETH, ENABLE, reg_general_configuration_noneth);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_general_configuration_clk_gate_cntrl_set(uint8_t bbh_id, const bbh_rx_general_configuration_clk_gate_cntrl *general_configuration_clk_gate_cntrl)
{
    uint32_t reg_general_configuration_clk_gate_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!general_configuration_clk_gate_cntrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
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
    if(!general_configuration_clk_gate_cntrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
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

bdmf_error_t ag_drv_bbh_rx_pm_counters_encrypterror_get(uint8_t bbh_id, uint32_t *encry_type_err)
{
    uint32_t reg_pm_counters_encrypterror;

#ifdef VALIDATE_PARMS
    if(!encry_type_err)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, PM_COUNTERS_ENCRYPTERROR, reg_pm_counters_encrypterror);

    *encry_type_err = RU_FIELD_GET(bbh_id, BBH_RX, PM_COUNTERS_ENCRYPTERROR, PMVALUE, reg_pm_counters_encrypterror);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_pm_counters_inploam_get(uint8_t bbh_id, uint32_t *inploam)
{
    uint32_t reg_pm_counters_inploam;

#ifdef VALIDATE_PARMS
    if(!inploam)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, PM_COUNTERS_INPLOAM, reg_pm_counters_inploam);

    *inploam = RU_FIELD_GET(bbh_id, BBH_RX, PM_COUNTERS_INPLOAM, INPLOAM, reg_pm_counters_inploam);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_pm_counters_epontyperror_get(uint8_t bbh_id, uint32_t *pmvalue)
{
    uint32_t reg_pm_counters_epontyperror;

#ifdef VALIDATE_PARMS
    if(!pmvalue)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, PM_COUNTERS_EPONTYPERROR, reg_pm_counters_epontyperror);

    *pmvalue = RU_FIELD_GET(bbh_id, BBH_RX, PM_COUNTERS_EPONTYPERROR, PMVALUE, reg_pm_counters_epontyperror);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_pm_counters_runterror_get(uint8_t bbh_id, uint16_t *pmvalue)
{
    uint32_t reg_pm_counters_runterror;

#ifdef VALIDATE_PARMS
    if(!pmvalue)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, PM_COUNTERS_RUNTERROR, reg_pm_counters_runterror);

    *pmvalue = RU_FIELD_GET(bbh_id, BBH_RX, PM_COUNTERS_RUNTERROR, PMVALUE, reg_pm_counters_runterror);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_debug_cntxtx0lsb_get(uint8_t bbh_id, bdmf_boolean *inreass, uint8_t *flowid, uint16_t *curoffset)
{
    uint32_t reg_debug_cntxtx0lsb;

#ifdef VALIDATE_PARMS
    if(!inreass || !flowid || !curoffset)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, DEBUG_CNTXTX0LSB, reg_debug_cntxtx0lsb);

    *inreass = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_CNTXTX0LSB, INREASS, reg_debug_cntxtx0lsb);
    *flowid = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_CNTXTX0LSB, FLOWID, reg_debug_cntxtx0lsb);
    *curoffset = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_CNTXTX0LSB, CUROFFSET, reg_debug_cntxtx0lsb);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_debug_cntxtx0msb_get(uint8_t bbh_id, uint16_t *curbn, uint16_t *firstbn)
{
    uint32_t reg_debug_cntxtx0msb;

#ifdef VALIDATE_PARMS
    if(!curbn || !firstbn)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, DEBUG_CNTXTX0MSB, reg_debug_cntxtx0msb);

    *curbn = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_CNTXTX0MSB, CURBN, reg_debug_cntxtx0msb);
    *firstbn = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_CNTXTX0MSB, FIRSTBN, reg_debug_cntxtx0msb);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_debug_cntxtx1lsb_get(uint8_t bbh_id, bdmf_boolean *inreass, uint8_t *flowid, uint16_t *curoffset)
{
    uint32_t reg_debug_cntxtx1lsb;

#ifdef VALIDATE_PARMS
    if(!inreass || !flowid || !curoffset)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, DEBUG_CNTXTX1LSB, reg_debug_cntxtx1lsb);

    *inreass = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_CNTXTX1LSB, INREASS, reg_debug_cntxtx1lsb);
    *flowid = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_CNTXTX1LSB, FLOWID, reg_debug_cntxtx1lsb);
    *curoffset = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_CNTXTX1LSB, CUROFFSET, reg_debug_cntxtx1lsb);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_debug_cntxtx1msb_get(uint8_t bbh_id, uint16_t *curbn, uint16_t *firstbn)
{
    uint32_t reg_debug_cntxtx1msb;

#ifdef VALIDATE_PARMS
    if(!curbn || !firstbn)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, DEBUG_CNTXTX1MSB, reg_debug_cntxtx1msb);

    *curbn = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_CNTXTX1MSB, CURBN, reg_debug_cntxtx1msb);
    *firstbn = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_CNTXTX1MSB, FIRSTBN, reg_debug_cntxtx1msb);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_debug_cntxtx0ingress_get(uint8_t bbh_id, bbh_rx_debug_cntxtx0ingress *debug_cntxtx0ingress)
{
    uint32_t reg_debug_cntxtx0ingress;

#ifdef VALIDATE_PARMS
    if(!debug_cntxtx0ingress)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, DEBUG_CNTXTX0INGRESS, reg_debug_cntxtx0ingress);

    debug_cntxtx0ingress->inreass = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_CNTXTX0INGRESS, INREASS, reg_debug_cntxtx0ingress);
    debug_cntxtx0ingress->sop = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_CNTXTX0INGRESS, SOP, reg_debug_cntxtx0ingress);
    debug_cntxtx0ingress->priority = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_CNTXTX0INGRESS, PRIORITY, reg_debug_cntxtx0ingress);
    debug_cntxtx0ingress->flowid = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_CNTXTX0INGRESS, FLOWID, reg_debug_cntxtx0ingress);
    debug_cntxtx0ingress->curoffset = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_CNTXTX0INGRESS, CUROFFSET, reg_debug_cntxtx0ingress);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_debug_cntxtx1ingress_get(uint8_t bbh_id, bbh_rx_debug_cntxtx1ingress *debug_cntxtx1ingress)
{
    uint32_t reg_debug_cntxtx1ingress;

#ifdef VALIDATE_PARMS
    if(!debug_cntxtx1ingress)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, DEBUG_CNTXTX1INGRESS, reg_debug_cntxtx1ingress);

    debug_cntxtx1ingress->inreass = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_CNTXTX1INGRESS, INREASS, reg_debug_cntxtx1ingress);
    debug_cntxtx1ingress->sop = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_CNTXTX1INGRESS, SOP, reg_debug_cntxtx1ingress);
    debug_cntxtx1ingress->priority = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_CNTXTX1INGRESS, PRIORITY, reg_debug_cntxtx1ingress);
    debug_cntxtx1ingress->flowid = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_CNTXTX1INGRESS, FLOWID, reg_debug_cntxtx1ingress);
    debug_cntxtx1ingress->curoffset = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_CNTXTX1INGRESS, CUROFFSET, reg_debug_cntxtx1ingress);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_debug_ibuw_get(uint8_t bbh_id, uint8_t *uw)
{
    uint32_t reg_debug_ibuw;

#ifdef VALIDATE_PARMS
    if(!uw)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, DEBUG_IBUW, reg_debug_ibuw);

    *uw = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_IBUW, UW, reg_debug_ibuw);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_debug_bbuw_get(uint8_t bbh_id, uint8_t *uw)
{
    uint32_t reg_debug_bbuw;

#ifdef VALIDATE_PARMS
    if(!uw)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, DEBUG_BBUW, reg_debug_bbuw);

    *uw = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_BBUW, UW, reg_debug_bbuw);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_debug_cfuw_get(uint8_t bbh_id, uint8_t *uw)
{
    uint32_t reg_debug_cfuw;

#ifdef VALIDATE_PARMS
    if(!uw)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, DEBUG_CFUW, reg_debug_cfuw);

    *uw = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_CFUW, UW, reg_debug_cfuw);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_debug_ackcnt_get(uint8_t bbh_id, uint8_t *sdma, uint8_t *connect)
{
    uint32_t reg_debug_ackcnt;

#ifdef VALIDATE_PARMS
    if(!sdma || !connect)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, DEBUG_ACKCNT, reg_debug_ackcnt);

    *sdma = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_ACKCNT, SDMA, reg_debug_ackcnt);
    *connect = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_ACKCNT, CONNECT, reg_debug_ackcnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_debug_coherencycnt_get(uint8_t bbh_id, uint8_t *normal, uint8_t *exclusive)
{
    uint32_t reg_debug_coherencycnt;

#ifdef VALIDATE_PARMS
    if(!normal || !exclusive)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, DEBUG_COHERENCYCNT, reg_debug_coherencycnt);

    *normal = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_COHERENCYCNT, NORMAL, reg_debug_coherencycnt);
    *exclusive = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_COHERENCYCNT, EXCLUSIVE, reg_debug_coherencycnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_debug_dbgvec_get(uint8_t bbh_id, uint32_t *dbgvec)
{
    uint32_t reg_debug_dbgvec;

#ifdef VALIDATE_PARMS
    if(!dbgvec)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, DEBUG_DBGVEC, reg_debug_dbgvec);

    *dbgvec = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_DBGVEC, DBGVEC, reg_debug_dbgvec);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_debug_ufuw_get(uint8_t bbh_id, uint8_t *uw)
{
    uint32_t reg_debug_ufuw;

#ifdef VALIDATE_PARMS
    if(!uw)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, DEBUG_UFUW, reg_debug_ufuw);

    *uw = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_UFUW, UW, reg_debug_ufuw);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_debug_creditcnt_get(uint8_t bbh_id, uint8_t *normal, uint8_t *exclusive)
{
    uint32_t reg_debug_creditcnt;

#ifdef VALIDATE_PARMS
    if(!normal || !exclusive)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, DEBUG_CREDITCNT, reg_debug_creditcnt);

    *normal = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_CREDITCNT, NORMAL, reg_debug_creditcnt);
    *exclusive = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_CREDITCNT, EXCLUSIVE, reg_debug_creditcnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_debug_sdmacnt_get(uint8_t bbh_id, uint8_t *ucd)
{
    uint32_t reg_debug_sdmacnt;

#ifdef VALIDATE_PARMS
    if(!ucd)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, DEBUG_SDMACNT, reg_debug_sdmacnt);

    *ucd = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_SDMACNT, UCD, reg_debug_sdmacnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_debug_cmfuw_get(uint8_t bbh_id, uint8_t *uw)
{
    uint32_t reg_debug_cmfuw;

#ifdef VALIDATE_PARMS
    if(!uw)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, DEBUG_CMFUW, reg_debug_cmfuw);

    *uw = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_CMFUW, UW, reg_debug_cmfuw);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_debug_sbnfifo_get(uint8_t bbh_id, uint8_t zero, bbh_rx_debug_sbnfifo *debug_sbnfifo)
{
#ifdef VALIDATE_PARMS
    if(!debug_sbnfifo)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (zero >= 2))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, zero *8 + 0, BBH_RX, DEBUG_SBNFIFO, debug_sbnfifo->sbn_fifo[0]);
    RU_REG_RAM_READ(bbh_id, zero *8 + 1, BBH_RX, DEBUG_SBNFIFO, debug_sbnfifo->sbn_fifo[1]);
    RU_REG_RAM_READ(bbh_id, zero *8 + 2, BBH_RX, DEBUG_SBNFIFO, debug_sbnfifo->sbn_fifo[2]);
    RU_REG_RAM_READ(bbh_id, zero *8 + 3, BBH_RX, DEBUG_SBNFIFO, debug_sbnfifo->sbn_fifo[3]);
    RU_REG_RAM_READ(bbh_id, zero *8 + 4, BBH_RX, DEBUG_SBNFIFO, debug_sbnfifo->sbn_fifo[4]);
    RU_REG_RAM_READ(bbh_id, zero *8 + 5, BBH_RX, DEBUG_SBNFIFO, debug_sbnfifo->sbn_fifo[5]);
    RU_REG_RAM_READ(bbh_id, zero *8 + 6, BBH_RX, DEBUG_SBNFIFO, debug_sbnfifo->sbn_fifo[6]);
    RU_REG_RAM_READ(bbh_id, zero *8 + 7, BBH_RX, DEBUG_SBNFIFO, debug_sbnfifo->sbn_fifo[7]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_debug_cmdfifo_get(uint8_t bbh_id, uint32_t zero, bbh_rx_debug_cmdfifo *debug_cmdfifo)
{
#ifdef VALIDATE_PARMS
    if(!debug_cmdfifo)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (zero >= 1))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, zero *4 + 0, BBH_RX, DEBUG_CMDFIFO, debug_cmdfifo->cmd_fifo[0]);
    RU_REG_RAM_READ(bbh_id, zero *4 + 1, BBH_RX, DEBUG_CMDFIFO, debug_cmdfifo->cmd_fifo[1]);
    RU_REG_RAM_READ(bbh_id, zero *4 + 2, BBH_RX, DEBUG_CMDFIFO, debug_cmdfifo->cmd_fifo[2]);
    RU_REG_RAM_READ(bbh_id, zero *4 + 3, BBH_RX, DEBUG_CMDFIFO, debug_cmdfifo->cmd_fifo[3]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_debug_sbnrecyclefifo_get(uint8_t bbh_id, uint8_t zero, bbh_rx_debug_sbnrecyclefifo *debug_sbnrecyclefifo)
{
#ifdef VALIDATE_PARMS
    if(!debug_sbnrecyclefifo)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (zero >= 1))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, zero *2 + 0, BBH_RX, DEBUG_SBNRECYCLEFIFO, debug_sbnrecyclefifo->sbn_recycle_fifo[0]);
    RU_REG_RAM_READ(bbh_id, zero *2 + 1, BBH_RX, DEBUG_SBNRECYCLEFIFO, debug_sbnrecyclefifo->sbn_recycle_fifo[1]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_debug_coherencycnt2_get(uint8_t bbh_id, uint8_t *cdsent, uint8_t *ackreceived)
{
    uint32_t reg_debug_coherencycnt2;

#ifdef VALIDATE_PARMS
    if(!cdsent || !ackreceived)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, DEBUG_COHERENCYCNT2, reg_debug_coherencycnt2);

    *cdsent = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_COHERENCYCNT2, CDSENT, reg_debug_coherencycnt2);
    *ackreceived = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_COHERENCYCNT2, ACKRECEIVED, reg_debug_coherencycnt2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_debug_dropstatus_set(uint8_t bbh_id, bdmf_boolean dispstatus, bdmf_boolean sdmastatus)
{
    uint32_t reg_debug_dropstatus=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (dispstatus >= _1BITS_MAX_VAL_) ||
       (sdmastatus >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_debug_dropstatus = RU_FIELD_SET(bbh_id, BBH_RX, DEBUG_DROPSTATUS, DISPSTATUS, reg_debug_dropstatus, dispstatus);
    reg_debug_dropstatus = RU_FIELD_SET(bbh_id, BBH_RX, DEBUG_DROPSTATUS, SDMASTATUS, reg_debug_dropstatus, sdmastatus);

    RU_REG_WRITE(bbh_id, BBH_RX, DEBUG_DROPSTATUS, reg_debug_dropstatus);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_rx_debug_dropstatus_get(uint8_t bbh_id, bdmf_boolean *dispstatus, bdmf_boolean *sdmastatus)
{
    uint32_t reg_debug_dropstatus;

#ifdef VALIDATE_PARMS
    if(!dispstatus || !sdmastatus)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_RX, DEBUG_DROPSTATUS, reg_debug_dropstatus);

    *dispstatus = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_DROPSTATUS, DISPSTATUS, reg_debug_dropstatus);
    *sdmastatus = RU_FIELD_GET(bbh_id, BBH_RX, DEBUG_DROPSTATUS, SDMASTATUS, reg_debug_dropstatus);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL
typedef enum
{
    bdmf_address_general_configuration_bbcfg,
    bdmf_address_general_configuration_dispviq,
    bdmf_address_general_configuration_patterndatalsb,
    bdmf_address_general_configuration_patterndatamsb,
    bdmf_address_general_configuration_patternmasklsb,
    bdmf_address_general_configuration_patternmaskmsb,
    bdmf_address_general_configuration_exclqcfg,
    bdmf_address_general_configuration_sdmaaddr,
    bdmf_address_general_configuration_sdmacfg,
    bdmf_address_general_configuration_minpkt0,
    bdmf_address_general_configuration_maxpkt0,
    bdmf_address_general_configuration_maxpkt1,
    bdmf_address_general_configuration_sopoffset,
    bdmf_address_general_configuration_flowctrl,
    bdmf_address_general_configuration_crcomitdis,
    bdmf_address_general_configuration_enable,
    bdmf_address_general_configuration_g9991en,
    bdmf_address_general_configuration_perflowth,
    bdmf_address_general_configuration_perflowsets,
    bdmf_address_general_configuration_minpktsel0,
    bdmf_address_general_configuration_minpktsel1,
    bdmf_address_general_configuration_maxpktsel0,
    bdmf_address_general_configuration_maxpktsel1,
    bdmf_address_general_configuration_macmode,
    bdmf_address_general_configuration_sbpmcfg,
    bdmf_address_general_configuration_rxrstrst,
    bdmf_address_general_configuration_rxdbgsel,
    bdmf_address_general_configuration_bbhrx_raddr_decoder,
    bdmf_address_general_configuration_noneth,
    bdmf_address_general_configuration_clk_gate_cntrl,
    bdmf_address_pm_counters_inpkt,
    bdmf_address_pm_counters_thirdflow,
    bdmf_address_pm_counters_sopasop,
    bdmf_address_pm_counters_tooshort,
    bdmf_address_pm_counters_toolong,
    bdmf_address_pm_counters_crcerror,
    bdmf_address_pm_counters_encrypterror,
    bdmf_address_pm_counters_dispcong,
    bdmf_address_pm_counters_nosbpmsbn,
    bdmf_address_pm_counters_nosdmacd,
    bdmf_address_pm_counters_inploam,
    bdmf_address_pm_counters_crcerrorploam,
    bdmf_address_pm_counters_dispcongploam,
    bdmf_address_pm_counters_nosbpmsbnploam,
    bdmf_address_pm_counters_nosdmacdploam,
    bdmf_address_pm_counters_epontyperror,
    bdmf_address_pm_counters_runterror,
    bdmf_address_debug_cntxtx0lsb,
    bdmf_address_debug_cntxtx0msb,
    bdmf_address_debug_cntxtx1lsb,
    bdmf_address_debug_cntxtx1msb,
    bdmf_address_debug_cntxtx0ingress,
    bdmf_address_debug_cntxtx1ingress,
    bdmf_address_debug_ibuw,
    bdmf_address_debug_bbuw,
    bdmf_address_debug_cfuw,
    bdmf_address_debug_ackcnt,
    bdmf_address_debug_coherencycnt,
    bdmf_address_debug_dbgvec,
    bdmf_address_debug_ufuw,
    bdmf_address_debug_creditcnt,
    bdmf_address_debug_sdmacnt,
    bdmf_address_debug_cmfuw,
    bdmf_address_debug_sbnfifo,
    bdmf_address_debug_cmdfifo,
    bdmf_address_debug_sbnrecyclefifo,
    bdmf_address_debug_coherencycnt2,
    bdmf_address_debug_dropstatus,
}
bdmf_address;

static int bcm_bbh_rx_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_bbh_rx_ploam_en:
        err = ag_drv_bbh_rx_ploam_en_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_bbh_rx_user_priority3_en:
        err = ag_drv_bbh_rx_user_priority3_en_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_bbh_rx_pause_en:
        err = ag_drv_bbh_rx_pause_en_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_bbh_rx_pfc_en:
        err = ag_drv_bbh_rx_pfc_en_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_bbh_rx_ctrl_en:
        err = ag_drv_bbh_rx_ctrl_en_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_bbh_rx_pattern_en:
        err = ag_drv_bbh_rx_pattern_en_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_bbh_rx_exc_en:
        err = ag_drv_bbh_rx_exc_en_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_bbh_rx_pattern_recog:
    {
        bbh_rx_pattern_recog pattern_recog = { .patterndatalsb=parm[2].value.unumber, .patterndatamsb=parm[3].value.unumber, .patternmasklsb=parm[4].value.unumber, .patternmaskmsb=parm[5].value.unumber, .pattenoffset=parm[6].value.unumber};
        err = ag_drv_bbh_rx_pattern_recog_set(parm[1].value.unumber, &pattern_recog);
        break;
    }
    case cli_bbh_rx_flow_ctrl_timer:
        err = ag_drv_bbh_rx_flow_ctrl_timer_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_bbh_rx_flow_ctrl_force:
        err = ag_drv_bbh_rx_flow_ctrl_force_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_bbh_rx_flow_ctrl_drops_config:
    {
        bbh_rx_flow_ctrl_drops_config flow_ctrl_drops_config = { .dispdropdis=parm[2].value.unumber, .sdmadropdis=parm[3].value.unumber, .sbpmdropdis=parm[4].value.unumber};
        err = ag_drv_bbh_rx_flow_ctrl_drops_config_set(parm[1].value.unumber, &flow_ctrl_drops_config);
        break;
    }
    case cli_bbh_rx_sdma_bb_id:
        err = ag_drv_bbh_rx_sdma_bb_id_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_bbh_rx_dispatcher_sbpm_bb_id:
        err = ag_drv_bbh_rx_dispatcher_sbpm_bb_id_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bbh_rx_dispatcher_virtual_queues:
        err = ag_drv_bbh_rx_dispatcher_virtual_queues_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bbh_rx_sdma_config:
    {
        bbh_rx_sdma_config sdma_config = { .numofcd=parm[2].value.unumber, .exclth=parm[3].value.unumber, .database=parm[4].value.unumber, .descbase=parm[5].value.unumber};
        err = ag_drv_bbh_rx_sdma_config_set(parm[1].value.unumber, &sdma_config);
        break;
    }
    case cli_bbh_rx_pkt_size0:
        err = ag_drv_bbh_rx_pkt_size0_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bbh_rx_pkt_size1:
        err = ag_drv_bbh_rx_pkt_size1_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bbh_rx_pkt_size2:
        err = ag_drv_bbh_rx_pkt_size2_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bbh_rx_pkt_size3:
        err = ag_drv_bbh_rx_pkt_size3_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bbh_rx_pkt_sel_group_0:
        err = ag_drv_bbh_rx_pkt_sel_group_0_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bbh_rx_pkt_sel_group_1:
        err = ag_drv_bbh_rx_pkt_sel_group_1_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bbh_rx_mac_mode:
        err = ag_drv_bbh_rx_mac_mode_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_bbh_rx_general_configuration_sopoffset:
        err = ag_drv_bbh_rx_general_configuration_sopoffset_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_bbh_rx_general_configuration_crcomitdis:
        err = ag_drv_bbh_rx_general_configuration_crcomitdis_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_bbh_rx_general_configuration_enable:
        err = ag_drv_bbh_rx_general_configuration_enable_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bbh_rx_general_configuration_g9991en:
        err = ag_drv_bbh_rx_general_configuration_g9991en_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bbh_rx_general_configuration_perflowth:
        err = ag_drv_bbh_rx_general_configuration_perflowth_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_bbh_rx_min_pkt_sel_flows_0_15:
        err = ag_drv_bbh_rx_min_pkt_sel_flows_0_15_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bbh_rx_min_pkt_sel_flows_16_31:
        err = ag_drv_bbh_rx_min_pkt_sel_flows_16_31_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bbh_rx_max_pkt_sel_flows_0_15:
        err = ag_drv_bbh_rx_max_pkt_sel_flows_0_15_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bbh_rx_max_pkt_sel_flows_16_31:
        err = ag_drv_bbh_rx_max_pkt_sel_flows_16_31_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bbh_rx_general_configuration_sbpmcfg:
        err = ag_drv_bbh_rx_general_configuration_sbpmcfg_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_bbh_rx_general_configuration_rxrstrst:
    {
        bbh_rx_general_configuration_rxrstrst general_configuration_rxrstrst = { .inbufrst=parm[2].value.unumber, .burstbufrst=parm[3].value.unumber, .ingresscntxt=parm[4].value.unumber, .cmdfiforst=parm[5].value.unumber, .sbpmfiforst=parm[6].value.unumber, .coherencyfiforst=parm[7].value.unumber, .cntxtrst=parm[8].value.unumber, .sdmarst=parm[9].value.unumber};
        err = ag_drv_bbh_rx_general_configuration_rxrstrst_set(parm[1].value.unumber, &general_configuration_rxrstrst);
        break;
    }
    case cli_bbh_rx_general_configuration_rxdbgsel:
        err = ag_drv_bbh_rx_general_configuration_rxdbgsel_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_bbh_rx_general_configuration_bbhrx_raddr_decoder:
        err = ag_drv_bbh_rx_general_configuration_bbhrx_raddr_decoder_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_bbh_rx_general_configuration_noneth:
        err = ag_drv_bbh_rx_general_configuration_noneth_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bbh_rx_general_configuration_clk_gate_cntrl:
    {
        bbh_rx_general_configuration_clk_gate_cntrl general_configuration_clk_gate_cntrl = { .bypass_clk_gate=parm[2].value.unumber, .timer_val=parm[3].value.unumber, .keep_alive_en=parm[4].value.unumber, .keep_alive_intrvl=parm[5].value.unumber, .keep_alive_cyc=parm[6].value.unumber};
        err = ag_drv_bbh_rx_general_configuration_clk_gate_cntrl_set(parm[1].value.unumber, &general_configuration_clk_gate_cntrl);
        break;
    }
    case cli_bbh_rx_debug_dropstatus:
        err = ag_drv_bbh_rx_debug_dropstatus_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

int bcm_bbh_rx_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_bbh_rx_ploam_en:
    {
        bdmf_boolean ploamen;
        err = ag_drv_bbh_rx_ploam_en_get(parm[1].value.unumber, &ploamen);
        bdmf_session_print(session, "ploamen = %u (0x%x)\n", ploamen, ploamen);
        break;
    }
    case cli_bbh_rx_user_priority3_en:
    {
        bdmf_boolean pri3en;
        err = ag_drv_bbh_rx_user_priority3_en_get(parm[1].value.unumber, &pri3en);
        bdmf_session_print(session, "pri3en = %u (0x%x)\n", pri3en, pri3en);
        break;
    }
    case cli_bbh_rx_pause_en:
    {
        bdmf_boolean pauseen;
        err = ag_drv_bbh_rx_pause_en_get(parm[1].value.unumber, &pauseen);
        bdmf_session_print(session, "pauseen = %u (0x%x)\n", pauseen, pauseen);
        break;
    }
    case cli_bbh_rx_pfc_en:
    {
        bdmf_boolean pfcen;
        err = ag_drv_bbh_rx_pfc_en_get(parm[1].value.unumber, &pfcen);
        bdmf_session_print(session, "pfcen = %u (0x%x)\n", pfcen, pfcen);
        break;
    }
    case cli_bbh_rx_ctrl_en:
    {
        bdmf_boolean ctrlen;
        err = ag_drv_bbh_rx_ctrl_en_get(parm[1].value.unumber, &ctrlen);
        bdmf_session_print(session, "ctrlen = %u (0x%x)\n", ctrlen, ctrlen);
        break;
    }
    case cli_bbh_rx_pattern_en:
    {
        bdmf_boolean patternen;
        err = ag_drv_bbh_rx_pattern_en_get(parm[1].value.unumber, &patternen);
        bdmf_session_print(session, "patternen = %u (0x%x)\n", patternen, patternen);
        break;
    }
    case cli_bbh_rx_exc_en:
    {
        bdmf_boolean excen;
        err = ag_drv_bbh_rx_exc_en_get(parm[1].value.unumber, &excen);
        bdmf_session_print(session, "excen = %u (0x%x)\n", excen, excen);
        break;
    }
    case cli_bbh_rx_pattern_recog:
    {
        bbh_rx_pattern_recog pattern_recog;
        err = ag_drv_bbh_rx_pattern_recog_get(parm[1].value.unumber, &pattern_recog);
        bdmf_session_print(session, "patterndatalsb = %u (0x%x)\n", pattern_recog.patterndatalsb, pattern_recog.patterndatalsb);
        bdmf_session_print(session, "patterndatamsb = %u (0x%x)\n", pattern_recog.patterndatamsb, pattern_recog.patterndatamsb);
        bdmf_session_print(session, "patternmasklsb = %u (0x%x)\n", pattern_recog.patternmasklsb, pattern_recog.patternmasklsb);
        bdmf_session_print(session, "patternmaskmsb = %u (0x%x)\n", pattern_recog.patternmaskmsb, pattern_recog.patternmaskmsb);
        bdmf_session_print(session, "pattenoffset = %u (0x%x)\n", pattern_recog.pattenoffset, pattern_recog.pattenoffset);
        break;
    }
    case cli_bbh_rx_flow_ctrl_timer:
    {
        uint32_t timer;
        err = ag_drv_bbh_rx_flow_ctrl_timer_get(parm[1].value.unumber, &timer);
        bdmf_session_print(session, "timer = %u (0x%x)\n", timer, timer);
        break;
    }
    case cli_bbh_rx_flow_ctrl_force:
    {
        bdmf_boolean fcforce;
        err = ag_drv_bbh_rx_flow_ctrl_force_get(parm[1].value.unumber, &fcforce);
        bdmf_session_print(session, "fcforce = %u (0x%x)\n", fcforce, fcforce);
        break;
    }
    case cli_bbh_rx_flow_ctrl_drops_config:
    {
        bbh_rx_flow_ctrl_drops_config flow_ctrl_drops_config;
        err = ag_drv_bbh_rx_flow_ctrl_drops_config_get(parm[1].value.unumber, &flow_ctrl_drops_config);
        bdmf_session_print(session, "dispdropdis = %u (0x%x)\n", flow_ctrl_drops_config.dispdropdis, flow_ctrl_drops_config.dispdropdis);
        bdmf_session_print(session, "sdmadropdis = %u (0x%x)\n", flow_ctrl_drops_config.sdmadropdis, flow_ctrl_drops_config.sdmadropdis);
        bdmf_session_print(session, "sbpmdropdis = %u (0x%x)\n", flow_ctrl_drops_config.sbpmdropdis, flow_ctrl_drops_config.sbpmdropdis);
        break;
    }
    case cli_bbh_rx_sdma_bb_id:
    {
        uint8_t sdmabbid;
        err = ag_drv_bbh_rx_sdma_bb_id_get(parm[1].value.unumber, &sdmabbid);
        bdmf_session_print(session, "sdmabbid = %u (0x%x)\n", sdmabbid, sdmabbid);
        break;
    }
    case cli_bbh_rx_dispatcher_sbpm_bb_id:
    {
        uint8_t dispbbid;
        uint8_t sbpmbbid;
        err = ag_drv_bbh_rx_dispatcher_sbpm_bb_id_get(parm[1].value.unumber, &dispbbid, &sbpmbbid);
        bdmf_session_print(session, "dispbbid = %u (0x%x)\n", dispbbid, dispbbid);
        bdmf_session_print(session, "sbpmbbid = %u (0x%x)\n", sbpmbbid, sbpmbbid);
        break;
    }
    case cli_bbh_rx_dispatcher_virtual_queues:
    {
        uint8_t normalviq;
        uint8_t exclviq;
        err = ag_drv_bbh_rx_dispatcher_virtual_queues_get(parm[1].value.unumber, &normalviq, &exclviq);
        bdmf_session_print(session, "normalviq = %u (0x%x)\n", normalviq, normalviq);
        bdmf_session_print(session, "exclviq = %u (0x%x)\n", exclviq, exclviq);
        break;
    }
    case cli_bbh_rx_sdma_config:
    {
        bbh_rx_sdma_config sdma_config;
        err = ag_drv_bbh_rx_sdma_config_get(parm[1].value.unumber, &sdma_config);
        bdmf_session_print(session, "numofcd = %u (0x%x)\n", sdma_config.numofcd, sdma_config.numofcd);
        bdmf_session_print(session, "exclth = %u (0x%x)\n", sdma_config.exclth, sdma_config.exclth);
        bdmf_session_print(session, "database = %u (0x%x)\n", sdma_config.database, sdma_config.database);
        bdmf_session_print(session, "descbase = %u (0x%x)\n", sdma_config.descbase, sdma_config.descbase);
        break;
    }
    case cli_bbh_rx_pkt_size0:
    {
        uint8_t minpkt0;
        uint16_t maxpkt0;
        err = ag_drv_bbh_rx_pkt_size0_get(parm[1].value.unumber, &minpkt0, &maxpkt0);
        bdmf_session_print(session, "minpkt0 = %u (0x%x)\n", minpkt0, minpkt0);
        bdmf_session_print(session, "maxpkt0 = %u (0x%x)\n", maxpkt0, maxpkt0);
        break;
    }
    case cli_bbh_rx_pkt_size1:
    {
        uint8_t minpkt1;
        uint16_t maxpkt1;
        err = ag_drv_bbh_rx_pkt_size1_get(parm[1].value.unumber, &minpkt1, &maxpkt1);
        bdmf_session_print(session, "minpkt1 = %u (0x%x)\n", minpkt1, minpkt1);
        bdmf_session_print(session, "maxpkt1 = %u (0x%x)\n", maxpkt1, maxpkt1);
        break;
    }
    case cli_bbh_rx_pkt_size2:
    {
        uint8_t minpkt2;
        uint16_t maxpkt2;
        err = ag_drv_bbh_rx_pkt_size2_get(parm[1].value.unumber, &minpkt2, &maxpkt2);
        bdmf_session_print(session, "minpkt2 = %u (0x%x)\n", minpkt2, minpkt2);
        bdmf_session_print(session, "maxpkt2 = %u (0x%x)\n", maxpkt2, maxpkt2);
        break;
    }
    case cli_bbh_rx_pkt_size3:
    {
        uint8_t minpkt3;
        uint16_t maxpkt3;
        err = ag_drv_bbh_rx_pkt_size3_get(parm[1].value.unumber, &minpkt3, &maxpkt3);
        bdmf_session_print(session, "minpkt3 = %u (0x%x)\n", minpkt3, minpkt3);
        bdmf_session_print(session, "maxpkt3 = %u (0x%x)\n", maxpkt3, maxpkt3);
        break;
    }
    case cli_bbh_rx_pkt_sel_group_0:
    {
        uint8_t minpktsel0;
        uint8_t maxpktsel0;
        err = ag_drv_bbh_rx_pkt_sel_group_0_get(parm[1].value.unumber, &minpktsel0, &maxpktsel0);
        bdmf_session_print(session, "minpktsel0 = %u (0x%x)\n", minpktsel0, minpktsel0);
        bdmf_session_print(session, "maxpktsel0 = %u (0x%x)\n", maxpktsel0, maxpktsel0);
        break;
    }
    case cli_bbh_rx_pkt_sel_group_1:
    {
        uint8_t minpktsel1;
        uint8_t maxpktsel1;
        err = ag_drv_bbh_rx_pkt_sel_group_1_get(parm[1].value.unumber, &minpktsel1, &maxpktsel1);
        bdmf_session_print(session, "minpktsel1 = %u (0x%x)\n", minpktsel1, minpktsel1);
        bdmf_session_print(session, "maxpktsel1 = %u (0x%x)\n", maxpktsel1, maxpktsel1);
        break;
    }
    case cli_bbh_rx_error_pm_counters:
    {
        bbh_rx_error_pm_counters error_pm_counters;
        err = ag_drv_bbh_rx_error_pm_counters_get(parm[1].value.unumber, &error_pm_counters);
        bdmf_session_print(session, "crc_err_ploam = %u (0x%x)\n", error_pm_counters.crc_err_ploam, error_pm_counters.crc_err_ploam);
        bdmf_session_print(session, "third_flow = %u (0x%x)\n", error_pm_counters.third_flow, error_pm_counters.third_flow);
        bdmf_session_print(session, "sop_after_sop = %u (0x%x)\n", error_pm_counters.sop_after_sop, error_pm_counters.sop_after_sop);
        bdmf_session_print(session, "no_sbpm_bn_ploam = %u (0x%x)\n", error_pm_counters.no_sbpm_bn_ploam, error_pm_counters.no_sbpm_bn_ploam);
        break;
    }
    case cli_bbh_rx_pm_counters:
    {
        bbh_rx_pm_counters pm_counters;
        err = ag_drv_bbh_rx_pm_counters_get(parm[1].value.unumber, &pm_counters);
        bdmf_session_print(session, "inpkt = %u (0x%x)\n", pm_counters.inpkt, pm_counters.inpkt);
        bdmf_session_print(session, "crc_err = %u (0x%x)\n", pm_counters.crc_err, pm_counters.crc_err);
        bdmf_session_print(session, "too_short = %u (0x%x)\n", pm_counters.too_short, pm_counters.too_short);
        bdmf_session_print(session, "too_long = %u (0x%x)\n", pm_counters.too_long, pm_counters.too_long);
        bdmf_session_print(session, "no_sbpm_sbn = %u (0x%x)\n", pm_counters.no_sbpm_sbn, pm_counters.no_sbpm_sbn);
        bdmf_session_print(session, "disp_cong = %u (0x%x)\n", pm_counters.disp_cong, pm_counters.disp_cong);
        bdmf_session_print(session, "no_sdma_cd = %u (0x%x)\n", pm_counters.no_sdma_cd, pm_counters.no_sdma_cd);
        bdmf_session_print(session, "ploam_no_sdma_cd = %u (0x%x)\n", pm_counters.ploam_no_sdma_cd, pm_counters.ploam_no_sdma_cd);
        bdmf_session_print(session, "ploam_disp_cong = %u (0x%x)\n", pm_counters.ploam_disp_cong, pm_counters.ploam_disp_cong);
        break;
    }
    case cli_bbh_rx_mac_mode:
    {
        bdmf_boolean macmode;
        bdmf_boolean gponmode;
        bdmf_boolean macvdsl;
        err = ag_drv_bbh_rx_mac_mode_get(parm[1].value.unumber, &macmode, &gponmode, &macvdsl);
        bdmf_session_print(session, "macmode = %u (0x%x)\n", macmode, macmode);
        bdmf_session_print(session, "gponmode = %u (0x%x)\n", gponmode, gponmode);
        bdmf_session_print(session, "macvdsl = %u (0x%x)\n", macvdsl, macvdsl);
        break;
    }
    case cli_bbh_rx_general_configuration_sopoffset:
    {
        uint8_t sopoffset;
        err = ag_drv_bbh_rx_general_configuration_sopoffset_get(parm[1].value.unumber, &sopoffset);
        bdmf_session_print(session, "sopoffset = %u (0x%x)\n", sopoffset, sopoffset);
        break;
    }
    case cli_bbh_rx_general_configuration_crcomitdis:
    {
        bdmf_boolean crcomitdis;
        err = ag_drv_bbh_rx_general_configuration_crcomitdis_get(parm[1].value.unumber, &crcomitdis);
        bdmf_session_print(session, "crcomitdis = %u (0x%x)\n", crcomitdis, crcomitdis);
        break;
    }
    case cli_bbh_rx_general_configuration_enable:
    {
        bdmf_boolean pkten;
        bdmf_boolean sbpmen;
        err = ag_drv_bbh_rx_general_configuration_enable_get(parm[1].value.unumber, &pkten, &sbpmen);
        bdmf_session_print(session, "pkten = %u (0x%x)\n", pkten, pkten);
        bdmf_session_print(session, "sbpmen = %u (0x%x)\n", sbpmen, sbpmen);
        break;
    }
    case cli_bbh_rx_general_configuration_g9991en:
    {
        bdmf_boolean enable;
        bdmf_boolean bytes4_7enable;
        err = ag_drv_bbh_rx_general_configuration_g9991en_get(parm[1].value.unumber, &enable, &bytes4_7enable);
        bdmf_session_print(session, "enable = %u (0x%x)\n", enable, enable);
        bdmf_session_print(session, "bytes4_7enable = %u (0x%x)\n", bytes4_7enable, bytes4_7enable);
        break;
    }
    case cli_bbh_rx_general_configuration_perflowth:
    {
        uint8_t flowth;
        err = ag_drv_bbh_rx_general_configuration_perflowth_get(parm[1].value.unumber, &flowth);
        bdmf_session_print(session, "flowth = %u (0x%x)\n", flowth, flowth);
        break;
    }
    case cli_bbh_rx_min_pkt_sel_flows_0_15:
    {
        uint8_t sel_id;
        err = ag_drv_bbh_rx_min_pkt_sel_flows_0_15_get(parm[1].value.unumber, parm[2].value.unumber, &sel_id);
        bdmf_session_print(session, "sel_id = %u (0x%x)\n", sel_id, sel_id);
        break;
    }
    case cli_bbh_rx_min_pkt_sel_flows_16_31:
    {
        uint8_t sel_id;
        err = ag_drv_bbh_rx_min_pkt_sel_flows_16_31_get(parm[1].value.unumber, parm[2].value.unumber, &sel_id);
        bdmf_session_print(session, "sel_id = %u (0x%x)\n", sel_id, sel_id);
        break;
    }
    case cli_bbh_rx_max_pkt_sel_flows_0_15:
    {
        uint8_t sel_id;
        err = ag_drv_bbh_rx_max_pkt_sel_flows_0_15_get(parm[1].value.unumber, parm[2].value.unumber, &sel_id);
        bdmf_session_print(session, "sel_id = %u (0x%x)\n", sel_id, sel_id);
        break;
    }
    case cli_bbh_rx_max_pkt_sel_flows_16_31:
    {
        uint8_t sel_id;
        err = ag_drv_bbh_rx_max_pkt_sel_flows_16_31_get(parm[1].value.unumber, parm[2].value.unumber, &sel_id);
        bdmf_session_print(session, "sel_id = %u (0x%x)\n", sel_id, sel_id);
        break;
    }
    case cli_bbh_rx_general_configuration_sbpmcfg:
    {
        uint8_t max_otf_sbpm_req;
        err = ag_drv_bbh_rx_general_configuration_sbpmcfg_get(parm[1].value.unumber, &max_otf_sbpm_req);
        bdmf_session_print(session, "max_otf_sbpm_req = %u (0x%x)\n", max_otf_sbpm_req, max_otf_sbpm_req);
        break;
    }
    case cli_bbh_rx_general_configuration_rxrstrst:
    {
        bbh_rx_general_configuration_rxrstrst general_configuration_rxrstrst;
        err = ag_drv_bbh_rx_general_configuration_rxrstrst_get(parm[1].value.unumber, &general_configuration_rxrstrst);
        bdmf_session_print(session, "inbufrst = %u (0x%x)\n", general_configuration_rxrstrst.inbufrst, general_configuration_rxrstrst.inbufrst);
        bdmf_session_print(session, "burstbufrst = %u (0x%x)\n", general_configuration_rxrstrst.burstbufrst, general_configuration_rxrstrst.burstbufrst);
        bdmf_session_print(session, "ingresscntxt = %u (0x%x)\n", general_configuration_rxrstrst.ingresscntxt, general_configuration_rxrstrst.ingresscntxt);
        bdmf_session_print(session, "cmdfiforst = %u (0x%x)\n", general_configuration_rxrstrst.cmdfiforst, general_configuration_rxrstrst.cmdfiforst);
        bdmf_session_print(session, "sbpmfiforst = %u (0x%x)\n", general_configuration_rxrstrst.sbpmfiforst, general_configuration_rxrstrst.sbpmfiforst);
        bdmf_session_print(session, "coherencyfiforst = %u (0x%x)\n", general_configuration_rxrstrst.coherencyfiforst, general_configuration_rxrstrst.coherencyfiforst);
        bdmf_session_print(session, "cntxtrst = %u (0x%x)\n", general_configuration_rxrstrst.cntxtrst, general_configuration_rxrstrst.cntxtrst);
        bdmf_session_print(session, "sdmarst = %u (0x%x)\n", general_configuration_rxrstrst.sdmarst, general_configuration_rxrstrst.sdmarst);
        break;
    }
    case cli_bbh_rx_general_configuration_rxdbgsel:
    {
        uint8_t rxdbgsel;
        err = ag_drv_bbh_rx_general_configuration_rxdbgsel_get(parm[1].value.unumber, &rxdbgsel);
        bdmf_session_print(session, "rxdbgsel = %u (0x%x)\n", rxdbgsel, rxdbgsel);
        break;
    }
    case cli_bbh_rx_general_configuration_bbhrx_raddr_decoder:
    {
        uint8_t id_2overwr;
        uint16_t overwr_ra;
        bdmf_boolean overwr_en;
        err = ag_drv_bbh_rx_general_configuration_bbhrx_raddr_decoder_get(parm[1].value.unumber, &id_2overwr, &overwr_ra, &overwr_en);
        bdmf_session_print(session, "id_2overwr = %u (0x%x)\n", id_2overwr, id_2overwr);
        bdmf_session_print(session, "overwr_ra = %u (0x%x)\n", overwr_ra, overwr_ra);
        bdmf_session_print(session, "overwr_en = %u (0x%x)\n", overwr_en, overwr_en);
        break;
    }
    case cli_bbh_rx_general_configuration_noneth:
    {
        uint8_t flowid;
        bdmf_boolean enable;
        err = ag_drv_bbh_rx_general_configuration_noneth_get(parm[1].value.unumber, &flowid, &enable);
        bdmf_session_print(session, "flowid = %u (0x%x)\n", flowid, flowid);
        bdmf_session_print(session, "enable = %u (0x%x)\n", enable, enable);
        break;
    }
    case cli_bbh_rx_general_configuration_clk_gate_cntrl:
    {
        bbh_rx_general_configuration_clk_gate_cntrl general_configuration_clk_gate_cntrl;
        err = ag_drv_bbh_rx_general_configuration_clk_gate_cntrl_get(parm[1].value.unumber, &general_configuration_clk_gate_cntrl);
        bdmf_session_print(session, "bypass_clk_gate = %u (0x%x)\n", general_configuration_clk_gate_cntrl.bypass_clk_gate, general_configuration_clk_gate_cntrl.bypass_clk_gate);
        bdmf_session_print(session, "timer_val = %u (0x%x)\n", general_configuration_clk_gate_cntrl.timer_val, general_configuration_clk_gate_cntrl.timer_val);
        bdmf_session_print(session, "keep_alive_en = %u (0x%x)\n", general_configuration_clk_gate_cntrl.keep_alive_en, general_configuration_clk_gate_cntrl.keep_alive_en);
        bdmf_session_print(session, "keep_alive_intrvl = %u (0x%x)\n", general_configuration_clk_gate_cntrl.keep_alive_intrvl, general_configuration_clk_gate_cntrl.keep_alive_intrvl);
        bdmf_session_print(session, "keep_alive_cyc = %u (0x%x)\n", general_configuration_clk_gate_cntrl.keep_alive_cyc, general_configuration_clk_gate_cntrl.keep_alive_cyc);
        break;
    }
    case cli_bbh_rx_pm_counters_encrypterror:
    {
        uint32_t encry_type_err;
        err = ag_drv_bbh_rx_pm_counters_encrypterror_get(parm[1].value.unumber, &encry_type_err);
        bdmf_session_print(session, "encry_type_err = %u (0x%x)\n", encry_type_err, encry_type_err);
        break;
    }
    case cli_bbh_rx_pm_counters_inploam:
    {
        uint32_t inploam;
        err = ag_drv_bbh_rx_pm_counters_inploam_get(parm[1].value.unumber, &inploam);
        bdmf_session_print(session, "inploam = %u (0x%x)\n", inploam, inploam);
        break;
    }
    case cli_bbh_rx_pm_counters_epontyperror:
    {
        uint32_t pmvalue;
        err = ag_drv_bbh_rx_pm_counters_epontyperror_get(parm[1].value.unumber, &pmvalue);
        bdmf_session_print(session, "pmvalue = %u (0x%x)\n", pmvalue, pmvalue);
        break;
    }
    case cli_bbh_rx_pm_counters_runterror:
    {
        uint16_t pmvalue;
        err = ag_drv_bbh_rx_pm_counters_runterror_get(parm[1].value.unumber, &pmvalue);
        bdmf_session_print(session, "pmvalue = %u (0x%x)\n", pmvalue, pmvalue);
        break;
    }
    case cli_bbh_rx_debug_cntxtx0lsb:
    {
        bdmf_boolean inreass;
        uint8_t flowid;
        uint16_t curoffset;
        err = ag_drv_bbh_rx_debug_cntxtx0lsb_get(parm[1].value.unumber, &inreass, &flowid, &curoffset);
        bdmf_session_print(session, "inreass = %u (0x%x)\n", inreass, inreass);
        bdmf_session_print(session, "flowid = %u (0x%x)\n", flowid, flowid);
        bdmf_session_print(session, "curoffset = %u (0x%x)\n", curoffset, curoffset);
        break;
    }
    case cli_bbh_rx_debug_cntxtx0msb:
    {
        uint16_t curbn;
        uint16_t firstbn;
        err = ag_drv_bbh_rx_debug_cntxtx0msb_get(parm[1].value.unumber, &curbn, &firstbn);
        bdmf_session_print(session, "curbn = %u (0x%x)\n", curbn, curbn);
        bdmf_session_print(session, "firstbn = %u (0x%x)\n", firstbn, firstbn);
        break;
    }
    case cli_bbh_rx_debug_cntxtx1lsb:
    {
        bdmf_boolean inreass;
        uint8_t flowid;
        uint16_t curoffset;
        err = ag_drv_bbh_rx_debug_cntxtx1lsb_get(parm[1].value.unumber, &inreass, &flowid, &curoffset);
        bdmf_session_print(session, "inreass = %u (0x%x)\n", inreass, inreass);
        bdmf_session_print(session, "flowid = %u (0x%x)\n", flowid, flowid);
        bdmf_session_print(session, "curoffset = %u (0x%x)\n", curoffset, curoffset);
        break;
    }
    case cli_bbh_rx_debug_cntxtx1msb:
    {
        uint16_t curbn;
        uint16_t firstbn;
        err = ag_drv_bbh_rx_debug_cntxtx1msb_get(parm[1].value.unumber, &curbn, &firstbn);
        bdmf_session_print(session, "curbn = %u (0x%x)\n", curbn, curbn);
        bdmf_session_print(session, "firstbn = %u (0x%x)\n", firstbn, firstbn);
        break;
    }
    case cli_bbh_rx_debug_cntxtx0ingress:
    {
        bbh_rx_debug_cntxtx0ingress debug_cntxtx0ingress;
        err = ag_drv_bbh_rx_debug_cntxtx0ingress_get(parm[1].value.unumber, &debug_cntxtx0ingress);
        bdmf_session_print(session, "inreass = %u (0x%x)\n", debug_cntxtx0ingress.inreass, debug_cntxtx0ingress.inreass);
        bdmf_session_print(session, "sop = %u (0x%x)\n", debug_cntxtx0ingress.sop, debug_cntxtx0ingress.sop);
        bdmf_session_print(session, "priority = %u (0x%x)\n", debug_cntxtx0ingress.priority, debug_cntxtx0ingress.priority);
        bdmf_session_print(session, "flowid = %u (0x%x)\n", debug_cntxtx0ingress.flowid, debug_cntxtx0ingress.flowid);
        bdmf_session_print(session, "curoffset = %u (0x%x)\n", debug_cntxtx0ingress.curoffset, debug_cntxtx0ingress.curoffset);
        break;
    }
    case cli_bbh_rx_debug_cntxtx1ingress:
    {
        bbh_rx_debug_cntxtx1ingress debug_cntxtx1ingress;
        err = ag_drv_bbh_rx_debug_cntxtx1ingress_get(parm[1].value.unumber, &debug_cntxtx1ingress);
        bdmf_session_print(session, "inreass = %u (0x%x)\n", debug_cntxtx1ingress.inreass, debug_cntxtx1ingress.inreass);
        bdmf_session_print(session, "sop = %u (0x%x)\n", debug_cntxtx1ingress.sop, debug_cntxtx1ingress.sop);
        bdmf_session_print(session, "priority = %u (0x%x)\n", debug_cntxtx1ingress.priority, debug_cntxtx1ingress.priority);
        bdmf_session_print(session, "flowid = %u (0x%x)\n", debug_cntxtx1ingress.flowid, debug_cntxtx1ingress.flowid);
        bdmf_session_print(session, "curoffset = %u (0x%x)\n", debug_cntxtx1ingress.curoffset, debug_cntxtx1ingress.curoffset);
        break;
    }
    case cli_bbh_rx_debug_ibuw:
    {
        uint8_t uw;
        err = ag_drv_bbh_rx_debug_ibuw_get(parm[1].value.unumber, &uw);
        bdmf_session_print(session, "uw = %u (0x%x)\n", uw, uw);
        break;
    }
    case cli_bbh_rx_debug_bbuw:
    {
        uint8_t uw;
        err = ag_drv_bbh_rx_debug_bbuw_get(parm[1].value.unumber, &uw);
        bdmf_session_print(session, "uw = %u (0x%x)\n", uw, uw);
        break;
    }
    case cli_bbh_rx_debug_cfuw:
    {
        uint8_t uw;
        err = ag_drv_bbh_rx_debug_cfuw_get(parm[1].value.unumber, &uw);
        bdmf_session_print(session, "uw = %u (0x%x)\n", uw, uw);
        break;
    }
    case cli_bbh_rx_debug_ackcnt:
    {
        uint8_t sdma;
        uint8_t connect;
        err = ag_drv_bbh_rx_debug_ackcnt_get(parm[1].value.unumber, &sdma, &connect);
        bdmf_session_print(session, "sdma = %u (0x%x)\n", sdma, sdma);
        bdmf_session_print(session, "connect = %u (0x%x)\n", connect, connect);
        break;
    }
    case cli_bbh_rx_debug_coherencycnt:
    {
        uint8_t normal;
        uint8_t exclusive;
        err = ag_drv_bbh_rx_debug_coherencycnt_get(parm[1].value.unumber, &normal, &exclusive);
        bdmf_session_print(session, "normal = %u (0x%x)\n", normal, normal);
        bdmf_session_print(session, "exclusive = %u (0x%x)\n", exclusive, exclusive);
        break;
    }
    case cli_bbh_rx_debug_dbgvec:
    {
        uint32_t dbgvec;
        err = ag_drv_bbh_rx_debug_dbgvec_get(parm[1].value.unumber, &dbgvec);
        bdmf_session_print(session, "dbgvec = %u (0x%x)\n", dbgvec, dbgvec);
        break;
    }
    case cli_bbh_rx_debug_ufuw:
    {
        uint8_t uw;
        err = ag_drv_bbh_rx_debug_ufuw_get(parm[1].value.unumber, &uw);
        bdmf_session_print(session, "uw = %u (0x%x)\n", uw, uw);
        break;
    }
    case cli_bbh_rx_debug_creditcnt:
    {
        uint8_t normal;
        uint8_t exclusive;
        err = ag_drv_bbh_rx_debug_creditcnt_get(parm[1].value.unumber, &normal, &exclusive);
        bdmf_session_print(session, "normal = %u (0x%x)\n", normal, normal);
        bdmf_session_print(session, "exclusive = %u (0x%x)\n", exclusive, exclusive);
        break;
    }
    case cli_bbh_rx_debug_sdmacnt:
    {
        uint8_t ucd;
        err = ag_drv_bbh_rx_debug_sdmacnt_get(parm[1].value.unumber, &ucd);
        bdmf_session_print(session, "ucd = %u (0x%x)\n", ucd, ucd);
        break;
    }
    case cli_bbh_rx_debug_cmfuw:
    {
        uint8_t uw;
        err = ag_drv_bbh_rx_debug_cmfuw_get(parm[1].value.unumber, &uw);
        bdmf_session_print(session, "uw = %u (0x%x)\n", uw, uw);
        break;
    }
    case cli_bbh_rx_debug_sbnfifo:
    {
        bbh_rx_debug_sbnfifo debug_sbnfifo;
        err = ag_drv_bbh_rx_debug_sbnfifo_get(parm[1].value.unumber, parm[2].value.unumber, &debug_sbnfifo);
        bdmf_session_print(session, "sbn_fifo[0] = %u (0x%x)\n", debug_sbnfifo.sbn_fifo[0], debug_sbnfifo.sbn_fifo[0]);
        bdmf_session_print(session, "sbn_fifo[1] = %u (0x%x)\n", debug_sbnfifo.sbn_fifo[1], debug_sbnfifo.sbn_fifo[1]);
        bdmf_session_print(session, "sbn_fifo[2] = %u (0x%x)\n", debug_sbnfifo.sbn_fifo[2], debug_sbnfifo.sbn_fifo[2]);
        bdmf_session_print(session, "sbn_fifo[3] = %u (0x%x)\n", debug_sbnfifo.sbn_fifo[3], debug_sbnfifo.sbn_fifo[3]);
        bdmf_session_print(session, "sbn_fifo[4] = %u (0x%x)\n", debug_sbnfifo.sbn_fifo[4], debug_sbnfifo.sbn_fifo[4]);
        bdmf_session_print(session, "sbn_fifo[5] = %u (0x%x)\n", debug_sbnfifo.sbn_fifo[5], debug_sbnfifo.sbn_fifo[5]);
        bdmf_session_print(session, "sbn_fifo[6] = %u (0x%x)\n", debug_sbnfifo.sbn_fifo[6], debug_sbnfifo.sbn_fifo[6]);
        bdmf_session_print(session, "sbn_fifo[7] = %u (0x%x)\n", debug_sbnfifo.sbn_fifo[7], debug_sbnfifo.sbn_fifo[7]);
        break;
    }
    case cli_bbh_rx_debug_cmdfifo:
    {
        bbh_rx_debug_cmdfifo debug_cmdfifo;
        err = ag_drv_bbh_rx_debug_cmdfifo_get(parm[1].value.unumber, parm[2].value.unumber, &debug_cmdfifo);
        bdmf_session_print(session, "cmd_fifo[0] = %u (0x%x)\n", debug_cmdfifo.cmd_fifo[0], debug_cmdfifo.cmd_fifo[0]);
        bdmf_session_print(session, "cmd_fifo[1] = %u (0x%x)\n", debug_cmdfifo.cmd_fifo[1], debug_cmdfifo.cmd_fifo[1]);
        bdmf_session_print(session, "cmd_fifo[2] = %u (0x%x)\n", debug_cmdfifo.cmd_fifo[2], debug_cmdfifo.cmd_fifo[2]);
        bdmf_session_print(session, "cmd_fifo[3] = %u (0x%x)\n", debug_cmdfifo.cmd_fifo[3], debug_cmdfifo.cmd_fifo[3]);
        break;
    }
    case cli_bbh_rx_debug_sbnrecyclefifo:
    {
        bbh_rx_debug_sbnrecyclefifo debug_sbnrecyclefifo;
        err = ag_drv_bbh_rx_debug_sbnrecyclefifo_get(parm[1].value.unumber, parm[2].value.unumber, &debug_sbnrecyclefifo);
        bdmf_session_print(session, "sbn_recycle_fifo[0] = %u (0x%x)\n", debug_sbnrecyclefifo.sbn_recycle_fifo[0], debug_sbnrecyclefifo.sbn_recycle_fifo[0]);
        bdmf_session_print(session, "sbn_recycle_fifo[1] = %u (0x%x)\n", debug_sbnrecyclefifo.sbn_recycle_fifo[1], debug_sbnrecyclefifo.sbn_recycle_fifo[1]);
        break;
    }
    case cli_bbh_rx_debug_coherencycnt2:
    {
        uint8_t cdsent;
        uint8_t ackreceived;
        err = ag_drv_bbh_rx_debug_coherencycnt2_get(parm[1].value.unumber, &cdsent, &ackreceived);
        bdmf_session_print(session, "cdsent = %u (0x%x)\n", cdsent, cdsent);
        bdmf_session_print(session, "ackreceived = %u (0x%x)\n", ackreceived, ackreceived);
        break;
    }
    case cli_bbh_rx_debug_dropstatus:
    {
        bdmf_boolean dispstatus;
        bdmf_boolean sdmastatus;
        err = ag_drv_bbh_rx_debug_dropstatus_get(parm[1].value.unumber, &dispstatus, &sdmastatus);
        bdmf_session_print(session, "dispstatus = %u (0x%x)\n", dispstatus, dispstatus);
        bdmf_session_print(session, "sdmastatus = %u (0x%x)\n", sdmastatus, sdmastatus);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_bbh_rx_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    uint8_t bbh_id = parm[1].value.unumber;
    bdmf_error_t err = BDMF_ERR_OK;

    {
        bdmf_boolean ploamen=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_ploam_en_set(%u %u)\n", bbh_id, ploamen);
        if(!err) ag_drv_bbh_rx_ploam_en_set(bbh_id, ploamen);
        if(!err) ag_drv_bbh_rx_ploam_en_get( bbh_id, &ploamen);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_ploam_en_get(%u %u)\n", bbh_id, ploamen);
        if(err || ploamen!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean pri3en=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_user_priority3_en_set(%u %u)\n", bbh_id, pri3en);
        if(!err) ag_drv_bbh_rx_user_priority3_en_set(bbh_id, pri3en);
        if(!err) ag_drv_bbh_rx_user_priority3_en_get( bbh_id, &pri3en);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_user_priority3_en_get(%u %u)\n", bbh_id, pri3en);
        if(err || pri3en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean pauseen=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_pause_en_set(%u %u)\n", bbh_id, pauseen);
        if(!err) ag_drv_bbh_rx_pause_en_set(bbh_id, pauseen);
        if(!err) ag_drv_bbh_rx_pause_en_get( bbh_id, &pauseen);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_pause_en_get(%u %u)\n", bbh_id, pauseen);
        if(err || pauseen!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean pfcen=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_pfc_en_set(%u %u)\n", bbh_id, pfcen);
        if(!err) ag_drv_bbh_rx_pfc_en_set(bbh_id, pfcen);
        if(!err) ag_drv_bbh_rx_pfc_en_get( bbh_id, &pfcen);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_pfc_en_get(%u %u)\n", bbh_id, pfcen);
        if(err || pfcen!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean ctrlen=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_ctrl_en_set(%u %u)\n", bbh_id, ctrlen);
        if(!err) ag_drv_bbh_rx_ctrl_en_set(bbh_id, ctrlen);
        if(!err) ag_drv_bbh_rx_ctrl_en_get( bbh_id, &ctrlen);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_ctrl_en_get(%u %u)\n", bbh_id, ctrlen);
        if(err || ctrlen!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean patternen=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_pattern_en_set(%u %u)\n", bbh_id, patternen);
        if(!err) ag_drv_bbh_rx_pattern_en_set(bbh_id, patternen);
        if(!err) ag_drv_bbh_rx_pattern_en_get( bbh_id, &patternen);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_pattern_en_get(%u %u)\n", bbh_id, patternen);
        if(err || patternen!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean excen=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_exc_en_set(%u %u)\n", bbh_id, excen);
        if(!err) ag_drv_bbh_rx_exc_en_set(bbh_id, excen);
        if(!err) ag_drv_bbh_rx_exc_en_get( bbh_id, &excen);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_exc_en_get(%u %u)\n", bbh_id, excen);
        if(err || excen!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bbh_rx_pattern_recog pattern_recog = {.patterndatalsb=gtmv(m, 32), .patterndatamsb=gtmv(m, 32), .patternmasklsb=gtmv(m, 32), .patternmaskmsb=gtmv(m, 32), .pattenoffset=gtmv(m, 4)};
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_pattern_recog_set(%u %u %u %u %u %u)\n", bbh_id, pattern_recog.patterndatalsb, pattern_recog.patterndatamsb, pattern_recog.patternmasklsb, pattern_recog.patternmaskmsb, pattern_recog.pattenoffset);
        if(!err) ag_drv_bbh_rx_pattern_recog_set(bbh_id, &pattern_recog);
        if(!err) ag_drv_bbh_rx_pattern_recog_get( bbh_id, &pattern_recog);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_pattern_recog_get(%u %u %u %u %u %u)\n", bbh_id, pattern_recog.patterndatalsb, pattern_recog.patterndatamsb, pattern_recog.patternmasklsb, pattern_recog.patternmaskmsb, pattern_recog.pattenoffset);
        if(err || pattern_recog.patterndatalsb!=gtmv(m, 32) || pattern_recog.patterndatamsb!=gtmv(m, 32) || pattern_recog.patternmasklsb!=gtmv(m, 32) || pattern_recog.patternmaskmsb!=gtmv(m, 32) || pattern_recog.pattenoffset!=gtmv(m, 4))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t timer=gtmv(m, 24);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_flow_ctrl_timer_set(%u %u)\n", bbh_id, timer);
        if(!err) ag_drv_bbh_rx_flow_ctrl_timer_set(bbh_id, timer);
        if(!err) ag_drv_bbh_rx_flow_ctrl_timer_get( bbh_id, &timer);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_flow_ctrl_timer_get(%u %u)\n", bbh_id, timer);
        if(err || timer!=gtmv(m, 24))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean fcforce=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_flow_ctrl_force_set(%u %u)\n", bbh_id, fcforce);
        if(!err) ag_drv_bbh_rx_flow_ctrl_force_set(bbh_id, fcforce);
        if(!err) ag_drv_bbh_rx_flow_ctrl_force_get( bbh_id, &fcforce);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_flow_ctrl_force_get(%u %u)\n", bbh_id, fcforce);
        if(err || fcforce!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bbh_rx_flow_ctrl_drops_config flow_ctrl_drops_config = {.dispdropdis=gtmv(m, 1), .sdmadropdis=gtmv(m, 1), .sbpmdropdis=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_flow_ctrl_drops_config_set(%u %u %u %u)\n", bbh_id, flow_ctrl_drops_config.dispdropdis, flow_ctrl_drops_config.sdmadropdis, flow_ctrl_drops_config.sbpmdropdis);
        if(!err) ag_drv_bbh_rx_flow_ctrl_drops_config_set(bbh_id, &flow_ctrl_drops_config);
        if(!err) ag_drv_bbh_rx_flow_ctrl_drops_config_get( bbh_id, &flow_ctrl_drops_config);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_flow_ctrl_drops_config_get(%u %u %u %u)\n", bbh_id, flow_ctrl_drops_config.dispdropdis, flow_ctrl_drops_config.sdmadropdis, flow_ctrl_drops_config.sbpmdropdis);
        if(err || flow_ctrl_drops_config.dispdropdis!=gtmv(m, 1) || flow_ctrl_drops_config.sdmadropdis!=gtmv(m, 1) || flow_ctrl_drops_config.sbpmdropdis!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t sdmabbid=gtmv(m, 6);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_sdma_bb_id_set(%u %u)\n", bbh_id, sdmabbid);
        if(!err) ag_drv_bbh_rx_sdma_bb_id_set(bbh_id, sdmabbid);
        if(!err) ag_drv_bbh_rx_sdma_bb_id_get( bbh_id, &sdmabbid);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_sdma_bb_id_get(%u %u)\n", bbh_id, sdmabbid);
        if(err || sdmabbid!=gtmv(m, 6))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t dispbbid=gtmv(m, 6);
        uint8_t sbpmbbid=gtmv(m, 6);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_dispatcher_sbpm_bb_id_set(%u %u %u)\n", bbh_id, dispbbid, sbpmbbid);
        if(!err) ag_drv_bbh_rx_dispatcher_sbpm_bb_id_set(bbh_id, dispbbid, sbpmbbid);
        if(!err) ag_drv_bbh_rx_dispatcher_sbpm_bb_id_get( bbh_id, &dispbbid, &sbpmbbid);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_dispatcher_sbpm_bb_id_get(%u %u %u)\n", bbh_id, dispbbid, sbpmbbid);
        if(err || dispbbid!=gtmv(m, 6) || sbpmbbid!=gtmv(m, 6))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t normalviq=gtmv(m, 5);
        uint8_t exclviq=gtmv(m, 5);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_dispatcher_virtual_queues_set(%u %u %u)\n", bbh_id, normalviq, exclviq);
        if(!err) ag_drv_bbh_rx_dispatcher_virtual_queues_set(bbh_id, normalviq, exclviq);
        if(!err) ag_drv_bbh_rx_dispatcher_virtual_queues_get( bbh_id, &normalviq, &exclviq);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_dispatcher_virtual_queues_get(%u %u %u)\n", bbh_id, normalviq, exclviq);
        if(err || normalviq!=gtmv(m, 5) || exclviq!=gtmv(m, 5))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bbh_rx_sdma_config sdma_config = {.numofcd=gtmv(m, 7), .exclth=gtmv(m, 7), .database=gtmv(m, 6), .descbase=gtmv(m, 6)};
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_sdma_config_set(%u %u %u %u %u)\n", bbh_id, sdma_config.numofcd, sdma_config.exclth, sdma_config.database, sdma_config.descbase);
        if(!err) ag_drv_bbh_rx_sdma_config_set(bbh_id, &sdma_config);
        if(!err) ag_drv_bbh_rx_sdma_config_get( bbh_id, &sdma_config);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_sdma_config_get(%u %u %u %u %u)\n", bbh_id, sdma_config.numofcd, sdma_config.exclth, sdma_config.database, sdma_config.descbase);
        if(err || sdma_config.numofcd!=gtmv(m, 7) || sdma_config.exclth!=gtmv(m, 7) || sdma_config.database!=gtmv(m, 6) || sdma_config.descbase!=gtmv(m, 6))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t minpkt0=gtmv(m, 8);
        uint16_t maxpkt0=gtmv(m, 14);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_pkt_size0_set(%u %u %u)\n", bbh_id, minpkt0, maxpkt0);
        if(!err) ag_drv_bbh_rx_pkt_size0_set(bbh_id, minpkt0, maxpkt0);
        if(!err) ag_drv_bbh_rx_pkt_size0_get( bbh_id, &minpkt0, &maxpkt0);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_pkt_size0_get(%u %u %u)\n", bbh_id, minpkt0, maxpkt0);
        if(err || minpkt0!=gtmv(m, 8) || maxpkt0!=gtmv(m, 14))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t minpkt1=gtmv(m, 8);
        uint16_t maxpkt1=gtmv(m, 14);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_pkt_size1_set(%u %u %u)\n", bbh_id, minpkt1, maxpkt1);
        if(!err) ag_drv_bbh_rx_pkt_size1_set(bbh_id, minpkt1, maxpkt1);
        if(!err) ag_drv_bbh_rx_pkt_size1_get( bbh_id, &minpkt1, &maxpkt1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_pkt_size1_get(%u %u %u)\n", bbh_id, minpkt1, maxpkt1);
        if(err || minpkt1!=gtmv(m, 8) || maxpkt1!=gtmv(m, 14))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t minpkt2=gtmv(m, 8);
        uint16_t maxpkt2=gtmv(m, 14);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_pkt_size2_set(%u %u %u)\n", bbh_id, minpkt2, maxpkt2);
        if(!err) ag_drv_bbh_rx_pkt_size2_set(bbh_id, minpkt2, maxpkt2);
        if(!err) ag_drv_bbh_rx_pkt_size2_get( bbh_id, &minpkt2, &maxpkt2);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_pkt_size2_get(%u %u %u)\n", bbh_id, minpkt2, maxpkt2);
        if(err || minpkt2!=gtmv(m, 8) || maxpkt2!=gtmv(m, 14))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t minpkt3=gtmv(m, 8);
        uint16_t maxpkt3=gtmv(m, 14);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_pkt_size3_set(%u %u %u)\n", bbh_id, minpkt3, maxpkt3);
        if(!err) ag_drv_bbh_rx_pkt_size3_set(bbh_id, minpkt3, maxpkt3);
        if(!err) ag_drv_bbh_rx_pkt_size3_get( bbh_id, &minpkt3, &maxpkt3);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_pkt_size3_get(%u %u %u)\n", bbh_id, minpkt3, maxpkt3);
        if(err || minpkt3!=gtmv(m, 8) || maxpkt3!=gtmv(m, 14))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t minpktsel0=gtmv(m, 2);
        uint8_t maxpktsel0=gtmv(m, 2);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_pkt_sel_group_0_set(%u %u %u)\n", bbh_id, minpktsel0, maxpktsel0);
        if(!err) ag_drv_bbh_rx_pkt_sel_group_0_set(bbh_id, minpktsel0, maxpktsel0);
        if(!err) ag_drv_bbh_rx_pkt_sel_group_0_get( bbh_id, &minpktsel0, &maxpktsel0);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_pkt_sel_group_0_get(%u %u %u)\n", bbh_id, minpktsel0, maxpktsel0);
        if(err || minpktsel0!=gtmv(m, 2) || maxpktsel0!=gtmv(m, 2))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t minpktsel1=gtmv(m, 2);
        uint8_t maxpktsel1=gtmv(m, 2);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_pkt_sel_group_1_set(%u %u %u)\n", bbh_id, minpktsel1, maxpktsel1);
        if(!err) ag_drv_bbh_rx_pkt_sel_group_1_set(bbh_id, minpktsel1, maxpktsel1);
        if(!err) ag_drv_bbh_rx_pkt_sel_group_1_get( bbh_id, &minpktsel1, &maxpktsel1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_pkt_sel_group_1_get(%u %u %u)\n", bbh_id, minpktsel1, maxpktsel1);
        if(err || minpktsel1!=gtmv(m, 2) || maxpktsel1!=gtmv(m, 2))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bbh_rx_error_pm_counters error_pm_counters = {.crc_err_ploam=gtmv(m, 32), .third_flow=gtmv(m, 32), .sop_after_sop=gtmv(m, 32), .no_sbpm_bn_ploam=gtmv(m, 32)};
        if(!err) ag_drv_bbh_rx_error_pm_counters_get( bbh_id, &error_pm_counters);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_error_pm_counters_get(%u %u %u %u %u)\n", bbh_id, error_pm_counters.crc_err_ploam, error_pm_counters.third_flow, error_pm_counters.sop_after_sop, error_pm_counters.no_sbpm_bn_ploam);
    }
    {
        bbh_rx_pm_counters pm_counters = {.inpkt=gtmv(m, 32), .crc_err=gtmv(m, 32), .too_short=gtmv(m, 32), .too_long=gtmv(m, 32), .no_sbpm_sbn=gtmv(m, 32), .disp_cong=gtmv(m, 32), .no_sdma_cd=gtmv(m, 32), .ploam_no_sdma_cd=gtmv(m, 32), .ploam_disp_cong=gtmv(m, 32)};
        if(!err) ag_drv_bbh_rx_pm_counters_get( bbh_id, &pm_counters);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_pm_counters_get(%u %u %u %u %u %u %u %u %u %u)\n", bbh_id, pm_counters.inpkt, pm_counters.crc_err, pm_counters.too_short, pm_counters.too_long, pm_counters.no_sbpm_sbn, pm_counters.disp_cong, pm_counters.no_sdma_cd, pm_counters.ploam_no_sdma_cd, pm_counters.ploam_disp_cong);
    }
    {
        bdmf_boolean macmode=gtmv(m, 1);
        bdmf_boolean gponmode=gtmv(m, 1);
        bdmf_boolean macvdsl=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_mac_mode_set(%u %u %u %u)\n", bbh_id, macmode, gponmode, macvdsl);
        if(!err) ag_drv_bbh_rx_mac_mode_set(bbh_id, macmode, gponmode, macvdsl);
        if(!err) ag_drv_bbh_rx_mac_mode_get( bbh_id, &macmode, &gponmode, &macvdsl);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_mac_mode_get(%u %u %u %u)\n", bbh_id, macmode, gponmode, macvdsl);
        if(err || macmode!=gtmv(m, 1) || gponmode!=gtmv(m, 1) || macvdsl!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t sopoffset=gtmv(m, 7);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_sopoffset_set(%u %u)\n", bbh_id, sopoffset);
        if(!err) ag_drv_bbh_rx_general_configuration_sopoffset_set(bbh_id, sopoffset);
        if(!err) ag_drv_bbh_rx_general_configuration_sopoffset_get( bbh_id, &sopoffset);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_sopoffset_get(%u %u)\n", bbh_id, sopoffset);
        if(err || sopoffset!=gtmv(m, 7))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean crcomitdis=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_crcomitdis_set(%u %u)\n", bbh_id, crcomitdis);
        if(!err) ag_drv_bbh_rx_general_configuration_crcomitdis_set(bbh_id, crcomitdis);
        if(!err) ag_drv_bbh_rx_general_configuration_crcomitdis_get( bbh_id, &crcomitdis);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_crcomitdis_get(%u %u)\n", bbh_id, crcomitdis);
        if(err || crcomitdis!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean pkten=gtmv(m, 1);
        bdmf_boolean sbpmen=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_enable_set(%u %u %u)\n", bbh_id, pkten, sbpmen);
        if(!err) ag_drv_bbh_rx_general_configuration_enable_set(bbh_id, pkten, sbpmen);
        if(!err) ag_drv_bbh_rx_general_configuration_enable_get( bbh_id, &pkten, &sbpmen);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_enable_get(%u %u %u)\n", bbh_id, pkten, sbpmen);
        if(err || pkten!=gtmv(m, 1) || sbpmen!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean enable=gtmv(m, 1);
        bdmf_boolean bytes4_7enable=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_g9991en_set(%u %u %u)\n", bbh_id, enable, bytes4_7enable);
        if(!err) ag_drv_bbh_rx_general_configuration_g9991en_set(bbh_id, enable, bytes4_7enable);
        if(!err) ag_drv_bbh_rx_general_configuration_g9991en_get( bbh_id, &enable, &bytes4_7enable);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_g9991en_get(%u %u %u)\n", bbh_id, enable, bytes4_7enable);
        if(err || enable!=gtmv(m, 1) || bytes4_7enable!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t flowth=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_perflowth_set(%u %u)\n", bbh_id, flowth);
        if(!err) ag_drv_bbh_rx_general_configuration_perflowth_set(bbh_id, flowth);
        if(!err) ag_drv_bbh_rx_general_configuration_perflowth_get( bbh_id, &flowth);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_perflowth_get(%u %u)\n", bbh_id, flowth);
        if(err || flowth!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t flow_id=gtmv(m, 4);
        uint8_t sel_id=gtmv(m, 2);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_min_pkt_sel_flows_0_15_set(%u %u %u)\n", bbh_id, flow_id, sel_id);
        if(!err) ag_drv_bbh_rx_min_pkt_sel_flows_0_15_set(bbh_id, flow_id, sel_id);
        if(!err) ag_drv_bbh_rx_min_pkt_sel_flows_0_15_get( bbh_id, flow_id, &sel_id);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_min_pkt_sel_flows_0_15_get(%u %u %u)\n", bbh_id, flow_id, sel_id);
        if(err || sel_id!=gtmv(m, 2))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t flow_id=gtmv(m, 4);
        uint8_t sel_id=gtmv(m, 2);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_min_pkt_sel_flows_16_31_set(%u %u %u)\n", bbh_id, flow_id, sel_id);
        if(!err) ag_drv_bbh_rx_min_pkt_sel_flows_16_31_set(bbh_id, flow_id, sel_id);
        if(!err) ag_drv_bbh_rx_min_pkt_sel_flows_16_31_get( bbh_id, flow_id, &sel_id);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_min_pkt_sel_flows_16_31_get(%u %u %u)\n", bbh_id, flow_id, sel_id);
        if(err || sel_id!=gtmv(m, 2))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t flow_id=gtmv(m, 4);
        uint8_t sel_id=gtmv(m, 2);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_max_pkt_sel_flows_0_15_set(%u %u %u)\n", bbh_id, flow_id, sel_id);
        if(!err) ag_drv_bbh_rx_max_pkt_sel_flows_0_15_set(bbh_id, flow_id, sel_id);
        if(!err) ag_drv_bbh_rx_max_pkt_sel_flows_0_15_get( bbh_id, flow_id, &sel_id);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_max_pkt_sel_flows_0_15_get(%u %u %u)\n", bbh_id, flow_id, sel_id);
        if(err || sel_id!=gtmv(m, 2))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t flow_id=gtmv(m, 4);
        uint8_t sel_id=gtmv(m, 2);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_max_pkt_sel_flows_16_31_set(%u %u %u)\n", bbh_id, flow_id, sel_id);
        if(!err) ag_drv_bbh_rx_max_pkt_sel_flows_16_31_set(bbh_id, flow_id, sel_id);
        if(!err) ag_drv_bbh_rx_max_pkt_sel_flows_16_31_get( bbh_id, flow_id, &sel_id);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_max_pkt_sel_flows_16_31_get(%u %u %u)\n", bbh_id, flow_id, sel_id);
        if(err || sel_id!=gtmv(m, 2))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t max_otf_sbpm_req=gtmv(m, 4);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_sbpmcfg_set(%u %u)\n", bbh_id, max_otf_sbpm_req);
        if(!err) ag_drv_bbh_rx_general_configuration_sbpmcfg_set(bbh_id, max_otf_sbpm_req);
        if(!err) ag_drv_bbh_rx_general_configuration_sbpmcfg_get( bbh_id, &max_otf_sbpm_req);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_sbpmcfg_get(%u %u)\n", bbh_id, max_otf_sbpm_req);
        if(err || max_otf_sbpm_req!=gtmv(m, 4))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bbh_rx_general_configuration_rxrstrst general_configuration_rxrstrst = {.inbufrst=gtmv(m, 1), .burstbufrst=gtmv(m, 1), .ingresscntxt=gtmv(m, 1), .cmdfiforst=gtmv(m, 1), .sbpmfiforst=gtmv(m, 1), .coherencyfiforst=gtmv(m, 1), .cntxtrst=gtmv(m, 1), .sdmarst=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_rxrstrst_set(%u %u %u %u %u %u %u %u %u)\n", bbh_id, general_configuration_rxrstrst.inbufrst, general_configuration_rxrstrst.burstbufrst, general_configuration_rxrstrst.ingresscntxt, general_configuration_rxrstrst.cmdfiforst, general_configuration_rxrstrst.sbpmfiforst, general_configuration_rxrstrst.coherencyfiforst, general_configuration_rxrstrst.cntxtrst, general_configuration_rxrstrst.sdmarst);
        if(!err) ag_drv_bbh_rx_general_configuration_rxrstrst_set(bbh_id, &general_configuration_rxrstrst);
        if(!err) ag_drv_bbh_rx_general_configuration_rxrstrst_get( bbh_id, &general_configuration_rxrstrst);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_rxrstrst_get(%u %u %u %u %u %u %u %u %u)\n", bbh_id, general_configuration_rxrstrst.inbufrst, general_configuration_rxrstrst.burstbufrst, general_configuration_rxrstrst.ingresscntxt, general_configuration_rxrstrst.cmdfiforst, general_configuration_rxrstrst.sbpmfiforst, general_configuration_rxrstrst.coherencyfiforst, general_configuration_rxrstrst.cntxtrst, general_configuration_rxrstrst.sdmarst);
        if(err || general_configuration_rxrstrst.inbufrst!=gtmv(m, 1) || general_configuration_rxrstrst.burstbufrst!=gtmv(m, 1) || general_configuration_rxrstrst.ingresscntxt!=gtmv(m, 1) || general_configuration_rxrstrst.cmdfiforst!=gtmv(m, 1) || general_configuration_rxrstrst.sbpmfiforst!=gtmv(m, 1) || general_configuration_rxrstrst.coherencyfiforst!=gtmv(m, 1) || general_configuration_rxrstrst.cntxtrst!=gtmv(m, 1) || general_configuration_rxrstrst.sdmarst!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t rxdbgsel=gtmv(m, 4);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_rxdbgsel_set(%u %u)\n", bbh_id, rxdbgsel);
        if(!err) ag_drv_bbh_rx_general_configuration_rxdbgsel_set(bbh_id, rxdbgsel);
        if(!err) ag_drv_bbh_rx_general_configuration_rxdbgsel_get( bbh_id, &rxdbgsel);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_rxdbgsel_get(%u %u)\n", bbh_id, rxdbgsel);
        if(err || rxdbgsel!=gtmv(m, 4))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t id_2overwr=gtmv(m, 6);
        uint16_t overwr_ra=gtmv(m, 10);
        bdmf_boolean overwr_en=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_bbhrx_raddr_decoder_set(%u %u %u %u)\n", bbh_id, id_2overwr, overwr_ra, overwr_en);
        if(!err) ag_drv_bbh_rx_general_configuration_bbhrx_raddr_decoder_set(bbh_id, id_2overwr, overwr_ra, overwr_en);
        if(!err) ag_drv_bbh_rx_general_configuration_bbhrx_raddr_decoder_get( bbh_id, &id_2overwr, &overwr_ra, &overwr_en);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_bbhrx_raddr_decoder_get(%u %u %u %u)\n", bbh_id, id_2overwr, overwr_ra, overwr_en);
        if(err || id_2overwr!=gtmv(m, 6) || overwr_ra!=gtmv(m, 10) || overwr_en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t flowid=gtmv(m, 8);
        bdmf_boolean enable=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_noneth_set(%u %u %u)\n", bbh_id, flowid, enable);
        if(!err) ag_drv_bbh_rx_general_configuration_noneth_set(bbh_id, flowid, enable);
        if(!err) ag_drv_bbh_rx_general_configuration_noneth_get( bbh_id, &flowid, &enable);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_noneth_get(%u %u %u)\n", bbh_id, flowid, enable);
        if(err || flowid!=gtmv(m, 8) || enable!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bbh_rx_general_configuration_clk_gate_cntrl general_configuration_clk_gate_cntrl = {.bypass_clk_gate=gtmv(m, 1), .timer_val=gtmv(m, 8), .keep_alive_en=gtmv(m, 1), .keep_alive_intrvl=gtmv(m, 3), .keep_alive_cyc=gtmv(m, 8)};
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_clk_gate_cntrl_set(%u %u %u %u %u %u)\n", bbh_id, general_configuration_clk_gate_cntrl.bypass_clk_gate, general_configuration_clk_gate_cntrl.timer_val, general_configuration_clk_gate_cntrl.keep_alive_en, general_configuration_clk_gate_cntrl.keep_alive_intrvl, general_configuration_clk_gate_cntrl.keep_alive_cyc);
        if(!err) ag_drv_bbh_rx_general_configuration_clk_gate_cntrl_set(bbh_id, &general_configuration_clk_gate_cntrl);
        if(!err) ag_drv_bbh_rx_general_configuration_clk_gate_cntrl_get( bbh_id, &general_configuration_clk_gate_cntrl);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_general_configuration_clk_gate_cntrl_get(%u %u %u %u %u %u)\n", bbh_id, general_configuration_clk_gate_cntrl.bypass_clk_gate, general_configuration_clk_gate_cntrl.timer_val, general_configuration_clk_gate_cntrl.keep_alive_en, general_configuration_clk_gate_cntrl.keep_alive_intrvl, general_configuration_clk_gate_cntrl.keep_alive_cyc);
        if(err || general_configuration_clk_gate_cntrl.bypass_clk_gate!=gtmv(m, 1) || general_configuration_clk_gate_cntrl.timer_val!=gtmv(m, 8) || general_configuration_clk_gate_cntrl.keep_alive_en!=gtmv(m, 1) || general_configuration_clk_gate_cntrl.keep_alive_intrvl!=gtmv(m, 3) || general_configuration_clk_gate_cntrl.keep_alive_cyc!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t encry_type_err=gtmv(m, 32);
        if(!err) ag_drv_bbh_rx_pm_counters_encrypterror_get( bbh_id, &encry_type_err);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_pm_counters_encrypterror_get(%u %u)\n", bbh_id, encry_type_err);
    }
    {
        uint32_t inploam=gtmv(m, 32);
        if(!err) ag_drv_bbh_rx_pm_counters_inploam_get( bbh_id, &inploam);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_pm_counters_inploam_get(%u %u)\n", bbh_id, inploam);
    }
    {
        uint32_t pmvalue=gtmv(m, 32);
        if(!err) ag_drv_bbh_rx_pm_counters_epontyperror_get( bbh_id, &pmvalue);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_pm_counters_epontyperror_get(%u %u)\n", bbh_id, pmvalue);
    }
    {
        uint16_t pmvalue=gtmv(m, 16);
        if(!err) ag_drv_bbh_rx_pm_counters_runterror_get( bbh_id, &pmvalue);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_pm_counters_runterror_get(%u %u)\n", bbh_id, pmvalue);
    }
    {
        bdmf_boolean inreass=gtmv(m, 1);
        uint8_t flowid=gtmv(m, 8);
        uint16_t curoffset=gtmv(m, 14);
        if(!err) ag_drv_bbh_rx_debug_cntxtx0lsb_get( bbh_id, &inreass, &flowid, &curoffset);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_debug_cntxtx0lsb_get(%u %u %u %u)\n", bbh_id, inreass, flowid, curoffset);
    }
    {
        uint16_t curbn=gtmv(m, 13);
        uint16_t firstbn=gtmv(m, 15);
        if(!err) ag_drv_bbh_rx_debug_cntxtx0msb_get( bbh_id, &curbn, &firstbn);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_debug_cntxtx0msb_get(%u %u %u)\n", bbh_id, curbn, firstbn);
    }
    {
        bdmf_boolean inreass=gtmv(m, 1);
        uint8_t flowid=gtmv(m, 8);
        uint16_t curoffset=gtmv(m, 14);
        if(!err) ag_drv_bbh_rx_debug_cntxtx1lsb_get( bbh_id, &inreass, &flowid, &curoffset);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_debug_cntxtx1lsb_get(%u %u %u %u)\n", bbh_id, inreass, flowid, curoffset);
    }
    {
        uint16_t curbn=gtmv(m, 13);
        uint16_t firstbn=gtmv(m, 15);
        if(!err) ag_drv_bbh_rx_debug_cntxtx1msb_get( bbh_id, &curbn, &firstbn);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_debug_cntxtx1msb_get(%u %u %u)\n", bbh_id, curbn, firstbn);
    }
    {
        bbh_rx_debug_cntxtx0ingress debug_cntxtx0ingress = {.inreass=gtmv(m, 1), .sop=gtmv(m, 1), .priority=gtmv(m, 2), .flowid=gtmv(m, 8), .curoffset=gtmv(m, 14)};
        if(!err) ag_drv_bbh_rx_debug_cntxtx0ingress_get( bbh_id, &debug_cntxtx0ingress);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_debug_cntxtx0ingress_get(%u %u %u %u %u %u)\n", bbh_id, debug_cntxtx0ingress.inreass, debug_cntxtx0ingress.sop, debug_cntxtx0ingress.priority, debug_cntxtx0ingress.flowid, debug_cntxtx0ingress.curoffset);
    }
    {
        bbh_rx_debug_cntxtx1ingress debug_cntxtx1ingress = {.inreass=gtmv(m, 1), .sop=gtmv(m, 1), .priority=gtmv(m, 2), .flowid=gtmv(m, 8), .curoffset=gtmv(m, 14)};
        if(!err) ag_drv_bbh_rx_debug_cntxtx1ingress_get( bbh_id, &debug_cntxtx1ingress);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_debug_cntxtx1ingress_get(%u %u %u %u %u %u)\n", bbh_id, debug_cntxtx1ingress.inreass, debug_cntxtx1ingress.sop, debug_cntxtx1ingress.priority, debug_cntxtx1ingress.flowid, debug_cntxtx1ingress.curoffset);
    }
    {
        uint8_t uw=gtmv(m, 3);
        if(!err) ag_drv_bbh_rx_debug_ibuw_get( bbh_id, &uw);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_debug_ibuw_get(%u %u)\n", bbh_id, uw);
    }
    {
        uint8_t uw=gtmv(m, 4);
        if(!err) ag_drv_bbh_rx_debug_bbuw_get( bbh_id, &uw);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_debug_bbuw_get(%u %u)\n", bbh_id, uw);
    }
    {
        uint8_t uw=gtmv(m, 6);
        if(!err) ag_drv_bbh_rx_debug_cfuw_get( bbh_id, &uw);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_debug_cfuw_get(%u %u)\n", bbh_id, uw);
    }
    {
        uint8_t sdma=gtmv(m, 5);
        uint8_t connect=gtmv(m, 5);
        if(!err) ag_drv_bbh_rx_debug_ackcnt_get( bbh_id, &sdma, &connect);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_debug_ackcnt_get(%u %u %u)\n", bbh_id, sdma, connect);
    }
    {
        uint8_t normal=gtmv(m, 5);
        uint8_t exclusive=gtmv(m, 5);
        if(!err) ag_drv_bbh_rx_debug_coherencycnt_get( bbh_id, &normal, &exclusive);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_debug_coherencycnt_get(%u %u %u)\n", bbh_id, normal, exclusive);
    }
    {
        uint32_t dbgvec=gtmv(m, 21);
        if(!err) ag_drv_bbh_rx_debug_dbgvec_get( bbh_id, &dbgvec);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_debug_dbgvec_get(%u %u)\n", bbh_id, dbgvec);
    }
    {
        uint8_t uw=gtmv(m, 3);
        if(!err) ag_drv_bbh_rx_debug_ufuw_get( bbh_id, &uw);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_debug_ufuw_get(%u %u)\n", bbh_id, uw);
    }
    {
        uint8_t normal=gtmv(m, 5);
        uint8_t exclusive=gtmv(m, 5);
        if(!err) ag_drv_bbh_rx_debug_creditcnt_get( bbh_id, &normal, &exclusive);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_debug_creditcnt_get(%u %u %u)\n", bbh_id, normal, exclusive);
    }
    {
        uint8_t ucd=gtmv(m, 7);
        if(!err) ag_drv_bbh_rx_debug_sdmacnt_get( bbh_id, &ucd);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_debug_sdmacnt_get(%u %u)\n", bbh_id, ucd);
    }
    {
        uint8_t uw=gtmv(m, 3);
        if(!err) ag_drv_bbh_rx_debug_cmfuw_get( bbh_id, &uw);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_debug_cmfuw_get(%u %u)\n", bbh_id, uw);
    }
    {
        uint8_t zero=gtmv(m, 1);
        bbh_rx_debug_sbnfifo debug_sbnfifo = {.sbn_fifo={gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if(!err) ag_drv_bbh_rx_debug_sbnfifo_get( bbh_id, zero, &debug_sbnfifo);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_debug_sbnfifo_get(%u %u %u %u %u %u %u %u %u %u)\n", bbh_id, zero, debug_sbnfifo.sbn_fifo[0], debug_sbnfifo.sbn_fifo[1], debug_sbnfifo.sbn_fifo[2], debug_sbnfifo.sbn_fifo[3], debug_sbnfifo.sbn_fifo[4], debug_sbnfifo.sbn_fifo[5], debug_sbnfifo.sbn_fifo[6], debug_sbnfifo.sbn_fifo[7]);
    }
    {
        uint32_t zero=gtmv(m, 0);
        bbh_rx_debug_cmdfifo debug_cmdfifo = {.cmd_fifo={gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if(!err) ag_drv_bbh_rx_debug_cmdfifo_get( bbh_id, zero, &debug_cmdfifo);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_debug_cmdfifo_get(%u %u %u %u %u %u)\n", bbh_id, zero, debug_cmdfifo.cmd_fifo[0], debug_cmdfifo.cmd_fifo[1], debug_cmdfifo.cmd_fifo[2], debug_cmdfifo.cmd_fifo[3]);
    }
    {
        uint8_t zero=gtmv(m, 0);
        bbh_rx_debug_sbnrecyclefifo debug_sbnrecyclefifo = {.sbn_recycle_fifo={gtmv(m, 32), gtmv(m, 32)}};
        if(!err) ag_drv_bbh_rx_debug_sbnrecyclefifo_get( bbh_id, zero, &debug_sbnrecyclefifo);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_debug_sbnrecyclefifo_get(%u %u %u %u)\n", bbh_id, zero, debug_sbnrecyclefifo.sbn_recycle_fifo[0], debug_sbnrecyclefifo.sbn_recycle_fifo[1]);
    }
    {
        uint8_t cdsent=gtmv(m, 7);
        uint8_t ackreceived=gtmv(m, 7);
        if(!err) ag_drv_bbh_rx_debug_coherencycnt2_get( bbh_id, &cdsent, &ackreceived);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_debug_coherencycnt2_get(%u %u %u)\n", bbh_id, cdsent, ackreceived);
    }
    {
        bdmf_boolean dispstatus=gtmv(m, 1);
        bdmf_boolean sdmastatus=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_debug_dropstatus_set(%u %u %u)\n", bbh_id, dispstatus, sdmastatus);
        if(!err) ag_drv_bbh_rx_debug_dropstatus_set(bbh_id, dispstatus, sdmastatus);
        if(!err) ag_drv_bbh_rx_debug_dropstatus_get( bbh_id, &dispstatus, &sdmastatus);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_rx_debug_dropstatus_get(%u %u %u)\n", bbh_id, dispstatus, sdmastatus);
        if(err || dispstatus!=gtmv(m, 1) || sdmastatus!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    return err;
}

static int bcm_bbh_rx_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_general_configuration_bbcfg : reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_BBCFG); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_dispviq : reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_DISPVIQ); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_patterndatalsb : reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_PATTERNDATALSB); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_patterndatamsb : reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_PATTERNDATAMSB); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_patternmasklsb : reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_PATTERNMASKLSB); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_patternmaskmsb : reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_PATTERNMASKMSB); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_exclqcfg : reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_EXCLQCFG); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_sdmaaddr : reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_SDMAADDR); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_sdmacfg : reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_SDMACFG); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_minpkt0 : reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_MINPKT0); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_maxpkt0 : reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_MAXPKT0); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_maxpkt1 : reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_MAXPKT1); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_sopoffset : reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_SOPOFFSET); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_flowctrl : reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_FLOWCTRL); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_crcomitdis : reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_CRCOMITDIS); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_enable : reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_ENABLE); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_g9991en : reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_G9991EN); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_perflowth : reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_PERFLOWTH); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_perflowsets : reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_PERFLOWSETS); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_minpktsel0 : reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_MINPKTSEL0); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_minpktsel1 : reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_MINPKTSEL1); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_maxpktsel0 : reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_MAXPKTSEL0); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_maxpktsel1 : reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_MAXPKTSEL1); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_macmode : reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_MACMODE); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_sbpmcfg : reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_SBPMCFG); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_rxrstrst : reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_RXRSTRST); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_rxdbgsel : reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_RXDBGSEL); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_bbhrx_raddr_decoder : reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_noneth : reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_NONETH); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_general_configuration_clk_gate_cntrl : reg = &RU_REG(BBH_RX, GENERAL_CONFIGURATION_CLK_GATE_CNTRL); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_pm_counters_inpkt : reg = &RU_REG(BBH_RX, PM_COUNTERS_INPKT); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_pm_counters_thirdflow : reg = &RU_REG(BBH_RX, PM_COUNTERS_THIRDFLOW); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_pm_counters_sopasop : reg = &RU_REG(BBH_RX, PM_COUNTERS_SOPASOP); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_pm_counters_tooshort : reg = &RU_REG(BBH_RX, PM_COUNTERS_TOOSHORT); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_pm_counters_toolong : reg = &RU_REG(BBH_RX, PM_COUNTERS_TOOLONG); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_pm_counters_crcerror : reg = &RU_REG(BBH_RX, PM_COUNTERS_CRCERROR); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_pm_counters_encrypterror : reg = &RU_REG(BBH_RX, PM_COUNTERS_ENCRYPTERROR); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_pm_counters_dispcong : reg = &RU_REG(BBH_RX, PM_COUNTERS_DISPCONG); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_pm_counters_nosbpmsbn : reg = &RU_REG(BBH_RX, PM_COUNTERS_NOSBPMSBN); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_pm_counters_nosdmacd : reg = &RU_REG(BBH_RX, PM_COUNTERS_NOSDMACD); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_pm_counters_inploam : reg = &RU_REG(BBH_RX, PM_COUNTERS_INPLOAM); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_pm_counters_crcerrorploam : reg = &RU_REG(BBH_RX, PM_COUNTERS_CRCERRORPLOAM); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_pm_counters_dispcongploam : reg = &RU_REG(BBH_RX, PM_COUNTERS_DISPCONGPLOAM); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_pm_counters_nosbpmsbnploam : reg = &RU_REG(BBH_RX, PM_COUNTERS_NOSBPMSBNPLOAM); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_pm_counters_nosdmacdploam : reg = &RU_REG(BBH_RX, PM_COUNTERS_NOSDMACDPLOAM); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_pm_counters_epontyperror : reg = &RU_REG(BBH_RX, PM_COUNTERS_EPONTYPERROR); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_pm_counters_runterror : reg = &RU_REG(BBH_RX, PM_COUNTERS_RUNTERROR); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_debug_cntxtx0lsb : reg = &RU_REG(BBH_RX, DEBUG_CNTXTX0LSB); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_debug_cntxtx0msb : reg = &RU_REG(BBH_RX, DEBUG_CNTXTX0MSB); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_debug_cntxtx1lsb : reg = &RU_REG(BBH_RX, DEBUG_CNTXTX1LSB); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_debug_cntxtx1msb : reg = &RU_REG(BBH_RX, DEBUG_CNTXTX1MSB); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_debug_cntxtx0ingress : reg = &RU_REG(BBH_RX, DEBUG_CNTXTX0INGRESS); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_debug_cntxtx1ingress : reg = &RU_REG(BBH_RX, DEBUG_CNTXTX1INGRESS); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_debug_ibuw : reg = &RU_REG(BBH_RX, DEBUG_IBUW); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_debug_bbuw : reg = &RU_REG(BBH_RX, DEBUG_BBUW); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_debug_cfuw : reg = &RU_REG(BBH_RX, DEBUG_CFUW); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_debug_ackcnt : reg = &RU_REG(BBH_RX, DEBUG_ACKCNT); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_debug_coherencycnt : reg = &RU_REG(BBH_RX, DEBUG_COHERENCYCNT); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_debug_dbgvec : reg = &RU_REG(BBH_RX, DEBUG_DBGVEC); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_debug_ufuw : reg = &RU_REG(BBH_RX, DEBUG_UFUW); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_debug_creditcnt : reg = &RU_REG(BBH_RX, DEBUG_CREDITCNT); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_debug_sdmacnt : reg = &RU_REG(BBH_RX, DEBUG_SDMACNT); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_debug_cmfuw : reg = &RU_REG(BBH_RX, DEBUG_CMFUW); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_debug_sbnfifo : reg = &RU_REG(BBH_RX, DEBUG_SBNFIFO); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_debug_cmdfifo : reg = &RU_REG(BBH_RX, DEBUG_CMDFIFO); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_debug_sbnrecyclefifo : reg = &RU_REG(BBH_RX, DEBUG_SBNRECYCLEFIFO); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_debug_coherencycnt2 : reg = &RU_REG(BBH_RX, DEBUG_COHERENCYCNT2); blk = &RU_BLK(BBH_RX); break;
    case bdmf_address_debug_dropstatus : reg = &RU_REG(BBH_RX, DEBUG_DROPSTATUS); blk = &RU_BLK(BBH_RX); break;
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

bdmfmon_handle_t ag_drv_bbh_rx_cli_init(bdmfmon_handle_t driver_dir)
{
    bdmfmon_handle_t dir;

    if ((dir = bdmfmon_dir_find(driver_dir, "bbh_rx"))!=NULL)
        return dir;
    dir = bdmfmon_dir_add(driver_dir, "bbh_rx", "bbh_rx", BDMF_ACCESS_ADMIN, NULL);

    {
        static bdmfmon_cmd_parm_t set_ploam_en[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("ploamen", "ploamen", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_user_priority3_en[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("pri3en", "pri3en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pause_en[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("pauseen", "pauseen", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pfc_en[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("pfcen", "pfcen", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_ctrl_en[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("ctrlen", "ctrlen", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pattern_en[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("patternen", "patternen", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_exc_en[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("excen", "excen", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pattern_recog[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("patterndatalsb", "patterndatalsb", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("patterndatamsb", "patterndatamsb", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("patternmasklsb", "patternmasklsb", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("patternmaskmsb", "patternmaskmsb", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pattenoffset", "pattenoffset", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_flow_ctrl_timer[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("timer", "timer", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_flow_ctrl_force[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("fcforce", "fcforce", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_flow_ctrl_drops_config[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("dispdropdis", "dispdropdis", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("sdmadropdis", "sdmadropdis", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("sbpmdropdis", "sbpmdropdis", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_sdma_bb_id[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("sdmabbid", "sdmabbid", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dispatcher_sbpm_bb_id[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("dispbbid", "dispbbid", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("sbpmbbid", "sbpmbbid", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dispatcher_virtual_queues[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("normalviq", "normalviq", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("exclviq", "exclviq", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_sdma_config[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("numofcd", "numofcd", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("exclth", "exclth", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("database", "database", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("descbase", "descbase", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pkt_size0[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("minpkt0", "minpkt0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("maxpkt0", "maxpkt0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pkt_size1[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("minpkt1", "minpkt1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("maxpkt1", "maxpkt1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pkt_size2[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("minpkt2", "minpkt2", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("maxpkt2", "maxpkt2", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pkt_size3[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("minpkt3", "minpkt3", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("maxpkt3", "maxpkt3", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pkt_sel_group_0[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("minpktsel0", "minpktsel0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("maxpktsel0", "maxpktsel0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pkt_sel_group_1[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("minpktsel1", "minpktsel1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("maxpktsel1", "maxpktsel1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_mac_mode[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("macmode", "macmode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("gponmode", "gponmode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("macvdsl", "macvdsl", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_configuration_sopoffset[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("sopoffset", "sopoffset", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_configuration_crcomitdis[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("crcomitdis", "crcomitdis", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_configuration_enable[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("pkten", "pkten", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("sbpmen", "sbpmen", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_configuration_g9991en[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("enable", "enable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("bytes4_7enable", "bytes4_7enable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_configuration_perflowth[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("flowth", "flowth", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_min_pkt_sel_flows_0_15[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("flow_id", "flow_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("sel_id", "sel_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_min_pkt_sel_flows_16_31[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("flow_id", "flow_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("sel_id", "sel_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_configuration_sbpmcfg[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("max_otf_sbpm_req", "max_otf_sbpm_req", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_configuration_rxrstrst[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("inbufrst", "inbufrst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("burstbufrst", "burstbufrst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ingresscntxt", "ingresscntxt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cmdfiforst", "cmdfiforst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("sbpmfiforst", "sbpmfiforst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("coherencyfiforst", "coherencyfiforst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cntxtrst", "cntxtrst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("sdmarst", "sdmarst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_configuration_rxdbgsel[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("rxdbgsel", "rxdbgsel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_configuration_bbhrx_raddr_decoder[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("id_2overwr", "id_2overwr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("overwr_ra", "overwr_ra", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("overwr_en", "overwr_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_configuration_noneth[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("flowid", "flowid", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("enable", "enable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_general_configuration_clk_gate_cntrl[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("bypass_clk_gate", "bypass_clk_gate", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("timer_val", "timer_val", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_en", "keep_alive_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_intrvl", "keep_alive_intrvl", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_cyc", "keep_alive_cyc", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_debug_dropstatus[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("dispstatus", "dispstatus", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("sdmastatus", "sdmastatus", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="ploam_en", .val=cli_bbh_rx_ploam_en, .parms=set_ploam_en },
            { .name="user_priority3_en", .val=cli_bbh_rx_user_priority3_en, .parms=set_user_priority3_en },
            { .name="pause_en", .val=cli_bbh_rx_pause_en, .parms=set_pause_en },
            { .name="pfc_en", .val=cli_bbh_rx_pfc_en, .parms=set_pfc_en },
            { .name="ctrl_en", .val=cli_bbh_rx_ctrl_en, .parms=set_ctrl_en },
            { .name="pattern_en", .val=cli_bbh_rx_pattern_en, .parms=set_pattern_en },
            { .name="exc_en", .val=cli_bbh_rx_exc_en, .parms=set_exc_en },
            { .name="pattern_recog", .val=cli_bbh_rx_pattern_recog, .parms=set_pattern_recog },
            { .name="flow_ctrl_timer", .val=cli_bbh_rx_flow_ctrl_timer, .parms=set_flow_ctrl_timer },
            { .name="flow_ctrl_force", .val=cli_bbh_rx_flow_ctrl_force, .parms=set_flow_ctrl_force },
            { .name="flow_ctrl_drops_config", .val=cli_bbh_rx_flow_ctrl_drops_config, .parms=set_flow_ctrl_drops_config },
            { .name="sdma_bb_id", .val=cli_bbh_rx_sdma_bb_id, .parms=set_sdma_bb_id },
            { .name="dispatcher_sbpm_bb_id", .val=cli_bbh_rx_dispatcher_sbpm_bb_id, .parms=set_dispatcher_sbpm_bb_id },
            { .name="dispatcher_virtual_queues", .val=cli_bbh_rx_dispatcher_virtual_queues, .parms=set_dispatcher_virtual_queues },
            { .name="sdma_config", .val=cli_bbh_rx_sdma_config, .parms=set_sdma_config },
            { .name="pkt_size0", .val=cli_bbh_rx_pkt_size0, .parms=set_pkt_size0 },
            { .name="pkt_size1", .val=cli_bbh_rx_pkt_size1, .parms=set_pkt_size1 },
            { .name="pkt_size2", .val=cli_bbh_rx_pkt_size2, .parms=set_pkt_size2 },
            { .name="pkt_size3", .val=cli_bbh_rx_pkt_size3, .parms=set_pkt_size3 },
            { .name="pkt_sel_group_0", .val=cli_bbh_rx_pkt_sel_group_0, .parms=set_pkt_sel_group_0 },
            { .name="pkt_sel_group_1", .val=cli_bbh_rx_pkt_sel_group_1, .parms=set_pkt_sel_group_1 },
            { .name="mac_mode", .val=cli_bbh_rx_mac_mode, .parms=set_mac_mode },
            { .name="general_configuration_sopoffset", .val=cli_bbh_rx_general_configuration_sopoffset, .parms=set_general_configuration_sopoffset },
            { .name="general_configuration_crcomitdis", .val=cli_bbh_rx_general_configuration_crcomitdis, .parms=set_general_configuration_crcomitdis },
            { .name="general_configuration_enable", .val=cli_bbh_rx_general_configuration_enable, .parms=set_general_configuration_enable },
            { .name="general_configuration_g9991en", .val=cli_bbh_rx_general_configuration_g9991en, .parms=set_general_configuration_g9991en },
            { .name="general_configuration_perflowth", .val=cli_bbh_rx_general_configuration_perflowth, .parms=set_general_configuration_perflowth },
            { .name="min_pkt_sel_flows_0_15", .val=cli_bbh_rx_min_pkt_sel_flows_0_15, .parms=set_min_pkt_sel_flows_0_15 },
            { .name="min_pkt_sel_flows_16_31", .val=cli_bbh_rx_min_pkt_sel_flows_16_31, .parms=set_min_pkt_sel_flows_16_31 },
            { .name="max_pkt_sel_flows_0_15", .val=cli_bbh_rx_max_pkt_sel_flows_0_15, .parms=set_min_pkt_sel_flows_0_15 },
            { .name="max_pkt_sel_flows_16_31", .val=cli_bbh_rx_max_pkt_sel_flows_16_31, .parms=set_min_pkt_sel_flows_16_31 },
            { .name="general_configuration_sbpmcfg", .val=cli_bbh_rx_general_configuration_sbpmcfg, .parms=set_general_configuration_sbpmcfg },
            { .name="general_configuration_rxrstrst", .val=cli_bbh_rx_general_configuration_rxrstrst, .parms=set_general_configuration_rxrstrst },
            { .name="general_configuration_rxdbgsel", .val=cli_bbh_rx_general_configuration_rxdbgsel, .parms=set_general_configuration_rxdbgsel },
            { .name="general_configuration_bbhrx_raddr_decoder", .val=cli_bbh_rx_general_configuration_bbhrx_raddr_decoder, .parms=set_general_configuration_bbhrx_raddr_decoder },
            { .name="general_configuration_noneth", .val=cli_bbh_rx_general_configuration_noneth, .parms=set_general_configuration_noneth },
            { .name="general_configuration_clk_gate_cntrl", .val=cli_bbh_rx_general_configuration_clk_gate_cntrl, .parms=set_general_configuration_clk_gate_cntrl },
            { .name="debug_dropstatus", .val=cli_bbh_rx_debug_dropstatus, .parms=set_debug_dropstatus },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", bcm_bbh_rx_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t set_default[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_debug_sbnfifo[]={
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_debug_cmdfifo[]={
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_debug_sbnrecyclefifo[]={
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="ploam_en", .val=cli_bbh_rx_ploam_en, .parms=set_default },
            { .name="user_priority3_en", .val=cli_bbh_rx_user_priority3_en, .parms=set_default },
            { .name="pause_en", .val=cli_bbh_rx_pause_en, .parms=set_default },
            { .name="pfc_en", .val=cli_bbh_rx_pfc_en, .parms=set_default },
            { .name="ctrl_en", .val=cli_bbh_rx_ctrl_en, .parms=set_default },
            { .name="pattern_en", .val=cli_bbh_rx_pattern_en, .parms=set_default },
            { .name="exc_en", .val=cli_bbh_rx_exc_en, .parms=set_default },
            { .name="pattern_recog", .val=cli_bbh_rx_pattern_recog, .parms=set_default },
            { .name="flow_ctrl_timer", .val=cli_bbh_rx_flow_ctrl_timer, .parms=set_default },
            { .name="flow_ctrl_force", .val=cli_bbh_rx_flow_ctrl_force, .parms=set_default },
            { .name="flow_ctrl_drops_config", .val=cli_bbh_rx_flow_ctrl_drops_config, .parms=set_default },
            { .name="sdma_bb_id", .val=cli_bbh_rx_sdma_bb_id, .parms=set_default },
            { .name="dispatcher_sbpm_bb_id", .val=cli_bbh_rx_dispatcher_sbpm_bb_id, .parms=set_default },
            { .name="dispatcher_virtual_queues", .val=cli_bbh_rx_dispatcher_virtual_queues, .parms=set_default },
            { .name="sdma_config", .val=cli_bbh_rx_sdma_config, .parms=set_default },
            { .name="pkt_size0", .val=cli_bbh_rx_pkt_size0, .parms=set_default },
            { .name="pkt_size1", .val=cli_bbh_rx_pkt_size1, .parms=set_default },
            { .name="pkt_size2", .val=cli_bbh_rx_pkt_size2, .parms=set_default },
            { .name="pkt_size3", .val=cli_bbh_rx_pkt_size3, .parms=set_default },
            { .name="pkt_sel_group_0", .val=cli_bbh_rx_pkt_sel_group_0, .parms=set_default },
            { .name="pkt_sel_group_1", .val=cli_bbh_rx_pkt_sel_group_1, .parms=set_default },
            { .name="error_pm_counters", .val=cli_bbh_rx_error_pm_counters, .parms=set_default },
            { .name="pm_counters", .val=cli_bbh_rx_pm_counters, .parms=set_default },
            { .name="mac_mode", .val=cli_bbh_rx_mac_mode, .parms=set_default },
            { .name="general_configuration_sopoffset", .val=cli_bbh_rx_general_configuration_sopoffset, .parms=set_default },
            { .name="general_configuration_crcomitdis", .val=cli_bbh_rx_general_configuration_crcomitdis, .parms=set_default },
            { .name="general_configuration_enable", .val=cli_bbh_rx_general_configuration_enable, .parms=set_default },
            { .name="general_configuration_g9991en", .val=cli_bbh_rx_general_configuration_g9991en, .parms=set_default },
            { .name="general_configuration_perflowth", .val=cli_bbh_rx_general_configuration_perflowth, .parms=set_default },
            { .name="min_pkt_sel_flows_0_15", .val=cli_bbh_rx_min_pkt_sel_flows_0_15, .parms=set_default },
            { .name="min_pkt_sel_flows_16_31", .val=cli_bbh_rx_min_pkt_sel_flows_16_31, .parms=set_default },
            { .name="max_pkt_sel_flows_0_15", .val=cli_bbh_rx_max_pkt_sel_flows_0_15, .parms=set_default },
            { .name="max_pkt_sel_flows_16_31", .val=cli_bbh_rx_max_pkt_sel_flows_16_31, .parms=set_default },
            { .name="general_configuration_sbpmcfg", .val=cli_bbh_rx_general_configuration_sbpmcfg, .parms=set_default },
            { .name="general_configuration_rxrstrst", .val=cli_bbh_rx_general_configuration_rxrstrst, .parms=set_default },
            { .name="general_configuration_rxdbgsel", .val=cli_bbh_rx_general_configuration_rxdbgsel, .parms=set_default },
            { .name="general_configuration_bbhrx_raddr_decoder", .val=cli_bbh_rx_general_configuration_bbhrx_raddr_decoder, .parms=set_default },
            { .name="general_configuration_noneth", .val=cli_bbh_rx_general_configuration_noneth, .parms=set_default },
            { .name="general_configuration_clk_gate_cntrl", .val=cli_bbh_rx_general_configuration_clk_gate_cntrl, .parms=set_default },
            { .name="pm_counters_encrypterror", .val=cli_bbh_rx_pm_counters_encrypterror, .parms=set_default },
            { .name="pm_counters_inploam", .val=cli_bbh_rx_pm_counters_inploam, .parms=set_default },
            { .name="pm_counters_epontyperror", .val=cli_bbh_rx_pm_counters_epontyperror, .parms=set_default },
            { .name="pm_counters_runterror", .val=cli_bbh_rx_pm_counters_runterror, .parms=set_default },
            { .name="debug_cntxtx0lsb", .val=cli_bbh_rx_debug_cntxtx0lsb, .parms=set_default },
            { .name="debug_cntxtx0msb", .val=cli_bbh_rx_debug_cntxtx0msb, .parms=set_default },
            { .name="debug_cntxtx1lsb", .val=cli_bbh_rx_debug_cntxtx1lsb, .parms=set_default },
            { .name="debug_cntxtx1msb", .val=cli_bbh_rx_debug_cntxtx1msb, .parms=set_default },
            { .name="debug_cntxtx0ingress", .val=cli_bbh_rx_debug_cntxtx0ingress, .parms=set_default },
            { .name="debug_cntxtx1ingress", .val=cli_bbh_rx_debug_cntxtx1ingress, .parms=set_default },
            { .name="debug_ibuw", .val=cli_bbh_rx_debug_ibuw, .parms=set_default },
            { .name="debug_bbuw", .val=cli_bbh_rx_debug_bbuw, .parms=set_default },
            { .name="debug_cfuw", .val=cli_bbh_rx_debug_cfuw, .parms=set_default },
            { .name="debug_ackcnt", .val=cli_bbh_rx_debug_ackcnt, .parms=set_default },
            { .name="debug_coherencycnt", .val=cli_bbh_rx_debug_coherencycnt, .parms=set_default },
            { .name="debug_dbgvec", .val=cli_bbh_rx_debug_dbgvec, .parms=set_default },
            { .name="debug_ufuw", .val=cli_bbh_rx_debug_ufuw, .parms=set_default },
            { .name="debug_creditcnt", .val=cli_bbh_rx_debug_creditcnt, .parms=set_default },
            { .name="debug_sdmacnt", .val=cli_bbh_rx_debug_sdmacnt, .parms=set_default },
            { .name="debug_cmfuw", .val=cli_bbh_rx_debug_cmfuw, .parms=set_default },
            { .name="debug_sbnfifo", .val=cli_bbh_rx_debug_sbnfifo, .parms=set_debug_sbnfifo },
            { .name="debug_cmdfifo", .val=cli_bbh_rx_debug_cmdfifo, .parms=set_debug_cmdfifo },
            { .name="debug_sbnrecyclefifo", .val=cli_bbh_rx_debug_sbnrecyclefifo, .parms=set_debug_sbnrecyclefifo },
            { .name="debug_coherencycnt2", .val=cli_bbh_rx_debug_coherencycnt2, .parms=set_default },
            { .name="debug_dropstatus", .val=cli_bbh_rx_debug_dropstatus, .parms=set_default },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_bbh_rx_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name="low" , .val=bdmf_test_method_low },
            { .name="mid" , .val=bdmf_test_method_mid },
            { .name="high" , .val=bdmf_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "test", "test", bcm_bbh_rx_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0),
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name="GENERAL_CONFIGURATION_BBCFG" , .val=bdmf_address_general_configuration_bbcfg },
            { .name="GENERAL_CONFIGURATION_DISPVIQ" , .val=bdmf_address_general_configuration_dispviq },
            { .name="GENERAL_CONFIGURATION_PATTERNDATALSB" , .val=bdmf_address_general_configuration_patterndatalsb },
            { .name="GENERAL_CONFIGURATION_PATTERNDATAMSB" , .val=bdmf_address_general_configuration_patterndatamsb },
            { .name="GENERAL_CONFIGURATION_PATTERNMASKLSB" , .val=bdmf_address_general_configuration_patternmasklsb },
            { .name="GENERAL_CONFIGURATION_PATTERNMASKMSB" , .val=bdmf_address_general_configuration_patternmaskmsb },
            { .name="GENERAL_CONFIGURATION_EXCLQCFG" , .val=bdmf_address_general_configuration_exclqcfg },
            { .name="GENERAL_CONFIGURATION_SDMAADDR" , .val=bdmf_address_general_configuration_sdmaaddr },
            { .name="GENERAL_CONFIGURATION_SDMACFG" , .val=bdmf_address_general_configuration_sdmacfg },
            { .name="GENERAL_CONFIGURATION_MINPKT0" , .val=bdmf_address_general_configuration_minpkt0 },
            { .name="GENERAL_CONFIGURATION_MAXPKT0" , .val=bdmf_address_general_configuration_maxpkt0 },
            { .name="GENERAL_CONFIGURATION_MAXPKT1" , .val=bdmf_address_general_configuration_maxpkt1 },
            { .name="GENERAL_CONFIGURATION_SOPOFFSET" , .val=bdmf_address_general_configuration_sopoffset },
            { .name="GENERAL_CONFIGURATION_FLOWCTRL" , .val=bdmf_address_general_configuration_flowctrl },
            { .name="GENERAL_CONFIGURATION_CRCOMITDIS" , .val=bdmf_address_general_configuration_crcomitdis },
            { .name="GENERAL_CONFIGURATION_ENABLE" , .val=bdmf_address_general_configuration_enable },
            { .name="GENERAL_CONFIGURATION_G9991EN" , .val=bdmf_address_general_configuration_g9991en },
            { .name="GENERAL_CONFIGURATION_PERFLOWTH" , .val=bdmf_address_general_configuration_perflowth },
            { .name="GENERAL_CONFIGURATION_PERFLOWSETS" , .val=bdmf_address_general_configuration_perflowsets },
            { .name="GENERAL_CONFIGURATION_MINPKTSEL0" , .val=bdmf_address_general_configuration_minpktsel0 },
            { .name="GENERAL_CONFIGURATION_MINPKTSEL1" , .val=bdmf_address_general_configuration_minpktsel1 },
            { .name="GENERAL_CONFIGURATION_MAXPKTSEL0" , .val=bdmf_address_general_configuration_maxpktsel0 },
            { .name="GENERAL_CONFIGURATION_MAXPKTSEL1" , .val=bdmf_address_general_configuration_maxpktsel1 },
            { .name="GENERAL_CONFIGURATION_MACMODE" , .val=bdmf_address_general_configuration_macmode },
            { .name="GENERAL_CONFIGURATION_SBPMCFG" , .val=bdmf_address_general_configuration_sbpmcfg },
            { .name="GENERAL_CONFIGURATION_RXRSTRST" , .val=bdmf_address_general_configuration_rxrstrst },
            { .name="GENERAL_CONFIGURATION_RXDBGSEL" , .val=bdmf_address_general_configuration_rxdbgsel },
            { .name="GENERAL_CONFIGURATION_BBHRX_RADDR_DECODER" , .val=bdmf_address_general_configuration_bbhrx_raddr_decoder },
            { .name="GENERAL_CONFIGURATION_NONETH" , .val=bdmf_address_general_configuration_noneth },
            { .name="GENERAL_CONFIGURATION_CLK_GATE_CNTRL" , .val=bdmf_address_general_configuration_clk_gate_cntrl },
            { .name="PM_COUNTERS_INPKT" , .val=bdmf_address_pm_counters_inpkt },
            { .name="PM_COUNTERS_THIRDFLOW" , .val=bdmf_address_pm_counters_thirdflow },
            { .name="PM_COUNTERS_SOPASOP" , .val=bdmf_address_pm_counters_sopasop },
            { .name="PM_COUNTERS_TOOSHORT" , .val=bdmf_address_pm_counters_tooshort },
            { .name="PM_COUNTERS_TOOLONG" , .val=bdmf_address_pm_counters_toolong },
            { .name="PM_COUNTERS_CRCERROR" , .val=bdmf_address_pm_counters_crcerror },
            { .name="PM_COUNTERS_ENCRYPTERROR" , .val=bdmf_address_pm_counters_encrypterror },
            { .name="PM_COUNTERS_DISPCONG" , .val=bdmf_address_pm_counters_dispcong },
            { .name="PM_COUNTERS_NOSBPMSBN" , .val=bdmf_address_pm_counters_nosbpmsbn },
            { .name="PM_COUNTERS_NOSDMACD" , .val=bdmf_address_pm_counters_nosdmacd },
            { .name="PM_COUNTERS_INPLOAM" , .val=bdmf_address_pm_counters_inploam },
            { .name="PM_COUNTERS_CRCERRORPLOAM" , .val=bdmf_address_pm_counters_crcerrorploam },
            { .name="PM_COUNTERS_DISPCONGPLOAM" , .val=bdmf_address_pm_counters_dispcongploam },
            { .name="PM_COUNTERS_NOSBPMSBNPLOAM" , .val=bdmf_address_pm_counters_nosbpmsbnploam },
            { .name="PM_COUNTERS_NOSDMACDPLOAM" , .val=bdmf_address_pm_counters_nosdmacdploam },
            { .name="PM_COUNTERS_EPONTYPERROR" , .val=bdmf_address_pm_counters_epontyperror },
            { .name="PM_COUNTERS_RUNTERROR" , .val=bdmf_address_pm_counters_runterror },
            { .name="DEBUG_CNTXTX0LSB" , .val=bdmf_address_debug_cntxtx0lsb },
            { .name="DEBUG_CNTXTX0MSB" , .val=bdmf_address_debug_cntxtx0msb },
            { .name="DEBUG_CNTXTX1LSB" , .val=bdmf_address_debug_cntxtx1lsb },
            { .name="DEBUG_CNTXTX1MSB" , .val=bdmf_address_debug_cntxtx1msb },
            { .name="DEBUG_CNTXTX0INGRESS" , .val=bdmf_address_debug_cntxtx0ingress },
            { .name="DEBUG_CNTXTX1INGRESS" , .val=bdmf_address_debug_cntxtx1ingress },
            { .name="DEBUG_IBUW" , .val=bdmf_address_debug_ibuw },
            { .name="DEBUG_BBUW" , .val=bdmf_address_debug_bbuw },
            { .name="DEBUG_CFUW" , .val=bdmf_address_debug_cfuw },
            { .name="DEBUG_ACKCNT" , .val=bdmf_address_debug_ackcnt },
            { .name="DEBUG_COHERENCYCNT" , .val=bdmf_address_debug_coherencycnt },
            { .name="DEBUG_DBGVEC" , .val=bdmf_address_debug_dbgvec },
            { .name="DEBUG_UFUW" , .val=bdmf_address_debug_ufuw },
            { .name="DEBUG_CREDITCNT" , .val=bdmf_address_debug_creditcnt },
            { .name="DEBUG_SDMACNT" , .val=bdmf_address_debug_sdmacnt },
            { .name="DEBUG_CMFUW" , .val=bdmf_address_debug_cmfuw },
            { .name="DEBUG_SBNFIFO" , .val=bdmf_address_debug_sbnfifo },
            { .name="DEBUG_CMDFIFO" , .val=bdmf_address_debug_cmdfifo },
            { .name="DEBUG_SBNRECYCLEFIFO" , .val=bdmf_address_debug_sbnrecyclefifo },
            { .name="DEBUG_COHERENCYCNT2" , .val=bdmf_address_debug_coherencycnt2 },
            { .name="DEBUG_DROPSTATUS" , .val=bdmf_address_debug_dropstatus },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "address", "address", bcm_bbh_rx_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM_ENUM("index1", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
    return dir;
}
#endif /* USE_BDMF_SHELL */

