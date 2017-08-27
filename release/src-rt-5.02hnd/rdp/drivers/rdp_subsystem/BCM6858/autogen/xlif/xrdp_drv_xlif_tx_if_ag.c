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
#include "xrdp_drv_xlif_tx_if_ag.h"

#define BLOCK_ADDR_COUNT_BITS 3
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_xlif_tx_if_if_enable_set(uint8_t channel_id, bdmf_boolean disable_with_credits, bdmf_boolean disable_wo_credits)
{
    uint32_t reg_if_enable=0;

#ifdef VALIDATE_PARMS
    if((channel_id >= BLOCK_ADDR_COUNT) ||
       (disable_with_credits >= _1BITS_MAX_VAL_) ||
       (disable_wo_credits >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_if_enable = RU_FIELD_SET(channel_id, XLIF_TX_IF, IF_ENABLE, DISABLE_WITH_CREDITS, reg_if_enable, disable_with_credits);
    reg_if_enable = RU_FIELD_SET(channel_id, XLIF_TX_IF, IF_ENABLE, DISABLE_WO_CREDITS, reg_if_enable, disable_wo_credits);

    RU_REG_WRITE(channel_id, XLIF_TX_IF, IF_ENABLE, reg_if_enable);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xlif_tx_if_if_enable_get(uint8_t channel_id, bdmf_boolean *disable_with_credits, bdmf_boolean *disable_wo_credits)
{
    uint32_t reg_if_enable;

#ifdef VALIDATE_PARMS
    if(!disable_with_credits || !disable_wo_credits)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((channel_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(channel_id, XLIF_TX_IF, IF_ENABLE, reg_if_enable);

    *disable_with_credits = RU_FIELD_GET(channel_id, XLIF_TX_IF, IF_ENABLE, DISABLE_WITH_CREDITS, reg_if_enable);
    *disable_wo_credits = RU_FIELD_GET(channel_id, XLIF_TX_IF, IF_ENABLE, DISABLE_WO_CREDITS, reg_if_enable);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xlif_tx_if_read_credits_get(uint8_t channel_id, uint16_t *value)
{
    uint32_t reg_read_credits;

#ifdef VALIDATE_PARMS
    if(!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((channel_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(channel_id, XLIF_TX_IF, READ_CREDITS, reg_read_credits);

    *value = RU_FIELD_GET(channel_id, XLIF_TX_IF, READ_CREDITS, VALUE, reg_read_credits);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xlif_tx_if_set_credits_set(uint8_t channel_id, uint16_t value, bdmf_boolean en)
{
    uint32_t reg_set_credits=0;

#ifdef VALIDATE_PARMS
    if((channel_id >= BLOCK_ADDR_COUNT) ||
       (value >= _10BITS_MAX_VAL_) ||
       (en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_set_credits = RU_FIELD_SET(channel_id, XLIF_TX_IF, SET_CREDITS, VALUE, reg_set_credits, value);
    reg_set_credits = RU_FIELD_SET(channel_id, XLIF_TX_IF, SET_CREDITS, EN, reg_set_credits, en);

    RU_REG_WRITE(channel_id, XLIF_TX_IF, SET_CREDITS, reg_set_credits);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xlif_tx_if_set_credits_get(uint8_t channel_id, uint16_t *value, bdmf_boolean *en)
{
    uint32_t reg_set_credits;

#ifdef VALIDATE_PARMS
    if(!value || !en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((channel_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(channel_id, XLIF_TX_IF, SET_CREDITS, reg_set_credits);

    *value = RU_FIELD_GET(channel_id, XLIF_TX_IF, SET_CREDITS, VALUE, reg_set_credits);
    *en = RU_FIELD_GET(channel_id, XLIF_TX_IF, SET_CREDITS, EN, reg_set_credits);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xlif_tx_if_out_ctrl_set(uint8_t channel_id, bdmf_boolean mac_txerr, bdmf_boolean mac_txcrcerr, bdmf_boolean mac_txosts_sinext, uint8_t mac_txcrcmode)
{
    uint32_t reg_out_ctrl=0;

#ifdef VALIDATE_PARMS
    if((channel_id >= BLOCK_ADDR_COUNT) ||
       (mac_txerr >= _1BITS_MAX_VAL_) ||
       (mac_txcrcerr >= _1BITS_MAX_VAL_) ||
       (mac_txosts_sinext >= _1BITS_MAX_VAL_) ||
       (mac_txcrcmode >= _2BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_out_ctrl = RU_FIELD_SET(channel_id, XLIF_TX_IF, OUT_CTRL, MAC_TXERR, reg_out_ctrl, mac_txerr);
    reg_out_ctrl = RU_FIELD_SET(channel_id, XLIF_TX_IF, OUT_CTRL, MAC_TXCRCERR, reg_out_ctrl, mac_txcrcerr);
    reg_out_ctrl = RU_FIELD_SET(channel_id, XLIF_TX_IF, OUT_CTRL, MAC_TXOSTS_SINEXT, reg_out_ctrl, mac_txosts_sinext);
    reg_out_ctrl = RU_FIELD_SET(channel_id, XLIF_TX_IF, OUT_CTRL, MAC_TXCRCMODE, reg_out_ctrl, mac_txcrcmode);

    RU_REG_WRITE(channel_id, XLIF_TX_IF, OUT_CTRL, reg_out_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xlif_tx_if_out_ctrl_get(uint8_t channel_id, bdmf_boolean *mac_txerr, bdmf_boolean *mac_txcrcerr, bdmf_boolean *mac_txosts_sinext, uint8_t *mac_txcrcmode)
{
    uint32_t reg_out_ctrl;

#ifdef VALIDATE_PARMS
    if(!mac_txerr || !mac_txcrcerr || !mac_txosts_sinext || !mac_txcrcmode)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((channel_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(channel_id, XLIF_TX_IF, OUT_CTRL, reg_out_ctrl);

    *mac_txerr = RU_FIELD_GET(channel_id, XLIF_TX_IF, OUT_CTRL, MAC_TXERR, reg_out_ctrl);
    *mac_txcrcerr = RU_FIELD_GET(channel_id, XLIF_TX_IF, OUT_CTRL, MAC_TXCRCERR, reg_out_ctrl);
    *mac_txosts_sinext = RU_FIELD_GET(channel_id, XLIF_TX_IF, OUT_CTRL, MAC_TXOSTS_SINEXT, reg_out_ctrl);
    *mac_txcrcmode = RU_FIELD_GET(channel_id, XLIF_TX_IF, OUT_CTRL, MAC_TXCRCMODE, reg_out_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xlif_tx_if_urun_port_enable_set(uint8_t channel_id, bdmf_boolean enable)
{
    uint32_t reg_urun_port_enable=0;

#ifdef VALIDATE_PARMS
    if((channel_id >= BLOCK_ADDR_COUNT) ||
       (enable >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_urun_port_enable = RU_FIELD_SET(channel_id, XLIF_TX_IF, URUN_PORT_ENABLE, ENABLE, reg_urun_port_enable, enable);

    RU_REG_WRITE(channel_id, XLIF_TX_IF, URUN_PORT_ENABLE, reg_urun_port_enable);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xlif_tx_if_urun_port_enable_get(uint8_t channel_id, bdmf_boolean *enable)
{
    uint32_t reg_urun_port_enable;

#ifdef VALIDATE_PARMS
    if(!enable)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((channel_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(channel_id, XLIF_TX_IF, URUN_PORT_ENABLE, reg_urun_port_enable);

    *enable = RU_FIELD_GET(channel_id, XLIF_TX_IF, URUN_PORT_ENABLE, ENABLE, reg_urun_port_enable);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xlif_tx_if_tx_threshold_set(uint8_t channel_id, uint8_t value)
{
    uint32_t reg_tx_threshold=0;

#ifdef VALIDATE_PARMS
    if((channel_id >= BLOCK_ADDR_COUNT) ||
       (value >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tx_threshold = RU_FIELD_SET(channel_id, XLIF_TX_IF, TX_THRESHOLD, VALUE, reg_tx_threshold, value);

    RU_REG_WRITE(channel_id, XLIF_TX_IF, TX_THRESHOLD, reg_tx_threshold);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xlif_tx_if_tx_threshold_get(uint8_t channel_id, uint8_t *value)
{
    uint32_t reg_tx_threshold;

#ifdef VALIDATE_PARMS
    if(!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((channel_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(channel_id, XLIF_TX_IF, TX_THRESHOLD, reg_tx_threshold);

    *value = RU_FIELD_GET(channel_id, XLIF_TX_IF, TX_THRESHOLD, VALUE, reg_tx_threshold);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL
typedef enum
{
    bdmf_address_if_enable,
    bdmf_address_read_credits,
    bdmf_address_set_credits,
    bdmf_address_out_ctrl,
    bdmf_address_urun_port_enable,
    bdmf_address_tx_threshold,
}
bdmf_address;

static int bcm_xlif_tx_if_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_xlif_tx_if_if_enable:
        err = ag_drv_xlif_tx_if_if_enable_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_xlif_tx_if_set_credits:
        err = ag_drv_xlif_tx_if_set_credits_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_xlif_tx_if_out_ctrl:
        err = ag_drv_xlif_tx_if_out_ctrl_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber);
        break;
    case cli_xlif_tx_if_urun_port_enable:
        err = ag_drv_xlif_tx_if_urun_port_enable_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xlif_tx_if_tx_threshold:
        err = ag_drv_xlif_tx_if_tx_threshold_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

int bcm_xlif_tx_if_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_xlif_tx_if_if_enable:
    {
        bdmf_boolean disable_with_credits;
        bdmf_boolean disable_wo_credits;
        err = ag_drv_xlif_tx_if_if_enable_get(parm[1].value.unumber, &disable_with_credits, &disable_wo_credits);
        bdmf_session_print(session, "disable_with_credits = %u (0x%x)\n", disable_with_credits, disable_with_credits);
        bdmf_session_print(session, "disable_wo_credits = %u (0x%x)\n", disable_wo_credits, disable_wo_credits);
        break;
    }
    case cli_xlif_tx_if_read_credits:
    {
        uint16_t value;
        err = ag_drv_xlif_tx_if_read_credits_get(parm[1].value.unumber, &value);
        bdmf_session_print(session, "value = %u (0x%x)\n", value, value);
        break;
    }
    case cli_xlif_tx_if_set_credits:
    {
        uint16_t value;
        bdmf_boolean en;
        err = ag_drv_xlif_tx_if_set_credits_get(parm[1].value.unumber, &value, &en);
        bdmf_session_print(session, "value = %u (0x%x)\n", value, value);
        bdmf_session_print(session, "en = %u (0x%x)\n", en, en);
        break;
    }
    case cli_xlif_tx_if_out_ctrl:
    {
        bdmf_boolean mac_txerr;
        bdmf_boolean mac_txcrcerr;
        bdmf_boolean mac_txosts_sinext;
        uint8_t mac_txcrcmode;
        err = ag_drv_xlif_tx_if_out_ctrl_get(parm[1].value.unumber, &mac_txerr, &mac_txcrcerr, &mac_txosts_sinext, &mac_txcrcmode);
        bdmf_session_print(session, "mac_txerr = %u (0x%x)\n", mac_txerr, mac_txerr);
        bdmf_session_print(session, "mac_txcrcerr = %u (0x%x)\n", mac_txcrcerr, mac_txcrcerr);
        bdmf_session_print(session, "mac_txosts_sinext = %u (0x%x)\n", mac_txosts_sinext, mac_txosts_sinext);
        bdmf_session_print(session, "mac_txcrcmode = %u (0x%x)\n", mac_txcrcmode, mac_txcrcmode);
        break;
    }
    case cli_xlif_tx_if_urun_port_enable:
    {
        bdmf_boolean enable;
        err = ag_drv_xlif_tx_if_urun_port_enable_get(parm[1].value.unumber, &enable);
        bdmf_session_print(session, "enable = %u (0x%x)\n", enable, enable);
        break;
    }
    case cli_xlif_tx_if_tx_threshold:
    {
        uint8_t value;
        err = ag_drv_xlif_tx_if_tx_threshold_get(parm[1].value.unumber, &value);
        bdmf_session_print(session, "value = %u (0x%x)\n", value, value);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_xlif_tx_if_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    uint8_t channel_id = parm[1].value.unumber;
    bdmf_error_t err = BDMF_ERR_OK;

    {
        bdmf_boolean disable_with_credits=gtmv(m, 1);
        bdmf_boolean disable_wo_credits=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_xlif_tx_if_if_enable_set(%u %u %u)\n", channel_id, disable_with_credits, disable_wo_credits);
        if(!err) ag_drv_xlif_tx_if_if_enable_set(channel_id, disable_with_credits, disable_wo_credits);
        if(!err) ag_drv_xlif_tx_if_if_enable_get( channel_id, &disable_with_credits, &disable_wo_credits);
        if(!err) bdmf_session_print(session, "ag_drv_xlif_tx_if_if_enable_get(%u %u %u)\n", channel_id, disable_with_credits, disable_wo_credits);
        if(err || disable_with_credits!=gtmv(m, 1) || disable_wo_credits!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t value=gtmv(m, 10);
        if(!err) ag_drv_xlif_tx_if_read_credits_get( channel_id, &value);
        if(!err) bdmf_session_print(session, "ag_drv_xlif_tx_if_read_credits_get(%u %u)\n", channel_id, value);
    }
    {
        uint16_t value=gtmv(m, 10);
        bdmf_boolean en=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_xlif_tx_if_set_credits_set(%u %u %u)\n", channel_id, value, en);
        if(!err) ag_drv_xlif_tx_if_set_credits_set(channel_id, value, en);
        if(!err) ag_drv_xlif_tx_if_set_credits_get( channel_id, &value, &en);
        if(!err) bdmf_session_print(session, "ag_drv_xlif_tx_if_set_credits_get(%u %u %u)\n", channel_id, value, en);
        if(err || value!=gtmv(m, 10) || en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean mac_txerr=gtmv(m, 1);
        bdmf_boolean mac_txcrcerr=gtmv(m, 1);
        bdmf_boolean mac_txosts_sinext=gtmv(m, 1);
        uint8_t mac_txcrcmode=gtmv(m, 2);
        if(!err) bdmf_session_print(session, "ag_drv_xlif_tx_if_out_ctrl_set(%u %u %u %u %u)\n", channel_id, mac_txerr, mac_txcrcerr, mac_txosts_sinext, mac_txcrcmode);
        if(!err) ag_drv_xlif_tx_if_out_ctrl_set(channel_id, mac_txerr, mac_txcrcerr, mac_txosts_sinext, mac_txcrcmode);
        if(!err) ag_drv_xlif_tx_if_out_ctrl_get( channel_id, &mac_txerr, &mac_txcrcerr, &mac_txosts_sinext, &mac_txcrcmode);
        if(!err) bdmf_session_print(session, "ag_drv_xlif_tx_if_out_ctrl_get(%u %u %u %u %u)\n", channel_id, mac_txerr, mac_txcrcerr, mac_txosts_sinext, mac_txcrcmode);
        if(err || mac_txerr!=gtmv(m, 1) || mac_txcrcerr!=gtmv(m, 1) || mac_txosts_sinext!=gtmv(m, 1) || mac_txcrcmode!=gtmv(m, 2))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean enable=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_xlif_tx_if_urun_port_enable_set(%u %u)\n", channel_id, enable);
        if(!err) ag_drv_xlif_tx_if_urun_port_enable_set(channel_id, enable);
        if(!err) ag_drv_xlif_tx_if_urun_port_enable_get( channel_id, &enable);
        if(!err) bdmf_session_print(session, "ag_drv_xlif_tx_if_urun_port_enable_get(%u %u)\n", channel_id, enable);
        if(err || enable!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t value=gtmv(m, 4);
        if(!err) bdmf_session_print(session, "ag_drv_xlif_tx_if_tx_threshold_set(%u %u)\n", channel_id, value);
        if(!err) ag_drv_xlif_tx_if_tx_threshold_set(channel_id, value);
        if(!err) ag_drv_xlif_tx_if_tx_threshold_get( channel_id, &value);
        if(!err) bdmf_session_print(session, "ag_drv_xlif_tx_if_tx_threshold_get(%u %u)\n", channel_id, value);
        if(err || value!=gtmv(m, 4))
            return err ? err : BDMF_ERR_IO;
    }
    return err;
}

static int bcm_xlif_tx_if_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_if_enable : reg = &RU_REG(XLIF_TX_IF, IF_ENABLE); blk = &RU_BLK(XLIF_TX_IF); break;
    case bdmf_address_read_credits : reg = &RU_REG(XLIF_TX_IF, READ_CREDITS); blk = &RU_BLK(XLIF_TX_IF); break;
    case bdmf_address_set_credits : reg = &RU_REG(XLIF_TX_IF, SET_CREDITS); blk = &RU_BLK(XLIF_TX_IF); break;
    case bdmf_address_out_ctrl : reg = &RU_REG(XLIF_TX_IF, OUT_CTRL); blk = &RU_BLK(XLIF_TX_IF); break;
    case bdmf_address_urun_port_enable : reg = &RU_REG(XLIF_TX_IF, URUN_PORT_ENABLE); blk = &RU_BLK(XLIF_TX_IF); break;
    case bdmf_address_tx_threshold : reg = &RU_REG(XLIF_TX_IF, TX_THRESHOLD); blk = &RU_BLK(XLIF_TX_IF); break;
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

bdmfmon_handle_t ag_drv_xlif_tx_if_cli_init(bdmfmon_handle_t driver_dir)
{
    bdmfmon_handle_t dir;

    if ((dir = bdmfmon_dir_find(driver_dir, "xlif_tx_if"))!=NULL)
        return dir;
    dir = bdmfmon_dir_add(driver_dir, "xlif_tx_if", "xlif_tx_if", BDMF_ACCESS_ADMIN, NULL);

    {
        static bdmfmon_cmd_parm_t set_if_enable[]={
            BDMFMON_MAKE_PARM_ENUM("channel_id", "channel_id", channel_id_enum_table, 0),
            BDMFMON_MAKE_PARM("disable_with_credits", "disable_with_credits", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("disable_wo_credits", "disable_wo_credits", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_set_credits[]={
            BDMFMON_MAKE_PARM_ENUM("channel_id", "channel_id", channel_id_enum_table, 0),
            BDMFMON_MAKE_PARM("value", "value", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("en", "en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_out_ctrl[]={
            BDMFMON_MAKE_PARM_ENUM("channel_id", "channel_id", channel_id_enum_table, 0),
            BDMFMON_MAKE_PARM("mac_txerr", "mac_txerr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mac_txcrcerr", "mac_txcrcerr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mac_txosts_sinext", "mac_txosts_sinext", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mac_txcrcmode", "mac_txcrcmode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_urun_port_enable[]={
            BDMFMON_MAKE_PARM_ENUM("channel_id", "channel_id", channel_id_enum_table, 0),
            BDMFMON_MAKE_PARM("enable", "enable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tx_threshold[]={
            BDMFMON_MAKE_PARM_ENUM("channel_id", "channel_id", channel_id_enum_table, 0),
            BDMFMON_MAKE_PARM("value", "value", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="if_enable", .val=cli_xlif_tx_if_if_enable, .parms=set_if_enable },
            { .name="set_credits", .val=cli_xlif_tx_if_set_credits, .parms=set_set_credits },
            { .name="out_ctrl", .val=cli_xlif_tx_if_out_ctrl, .parms=set_out_ctrl },
            { .name="urun_port_enable", .val=cli_xlif_tx_if_urun_port_enable, .parms=set_urun_port_enable },
            { .name="tx_threshold", .val=cli_xlif_tx_if_tx_threshold, .parms=set_tx_threshold },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", bcm_xlif_tx_if_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t set_default[]={
            BDMFMON_MAKE_PARM_ENUM("channel_id", "channel_id", channel_id_enum_table, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="if_enable", .val=cli_xlif_tx_if_if_enable, .parms=set_default },
            { .name="read_credits", .val=cli_xlif_tx_if_read_credits, .parms=set_default },
            { .name="set_credits", .val=cli_xlif_tx_if_set_credits, .parms=set_default },
            { .name="out_ctrl", .val=cli_xlif_tx_if_out_ctrl, .parms=set_default },
            { .name="urun_port_enable", .val=cli_xlif_tx_if_urun_port_enable, .parms=set_default },
            { .name="tx_threshold", .val=cli_xlif_tx_if_tx_threshold, .parms=set_default },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_xlif_tx_if_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name="low" , .val=bdmf_test_method_low },
            { .name="mid" , .val=bdmf_test_method_mid },
            { .name="high" , .val=bdmf_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "test", "test", bcm_xlif_tx_if_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0),
            BDMFMON_MAKE_PARM_ENUM("channel_id", "channel_id", channel_id_enum_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name="IF_ENABLE" , .val=bdmf_address_if_enable },
            { .name="READ_CREDITS" , .val=bdmf_address_read_credits },
            { .name="SET_CREDITS" , .val=bdmf_address_set_credits },
            { .name="OUT_CTRL" , .val=bdmf_address_out_ctrl },
            { .name="URUN_PORT_ENABLE" , .val=bdmf_address_urun_port_enable },
            { .name="TX_THRESHOLD" , .val=bdmf_address_tx_threshold },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "address", "address", bcm_xlif_tx_if_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM_ENUM("index1", "channel_id", channel_id_enum_table, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
    return dir;
}
#endif /* USE_BDMF_SHELL */

