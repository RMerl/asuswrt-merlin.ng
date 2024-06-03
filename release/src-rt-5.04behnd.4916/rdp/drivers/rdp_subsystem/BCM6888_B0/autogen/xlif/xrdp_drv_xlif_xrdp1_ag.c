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
#include "xrdp_drv_xlif_xrdp1_ag.h"

#define BLOCK_ADDR_COUNT_BITS 2
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_xlif_xrdp1_channel_xlif_rx_if_if_dis_set(uint8_t channel_id, bdmf_boolean disable)
{
    uint32_t reg_channel_xlif_rx_if_if_dis = 0;

#ifdef VALIDATE_PARMS
    if ((channel_id >= BLOCK_ADDR_COUNT) ||
       (disable >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_channel_xlif_rx_if_if_dis = RU_FIELD_SET(channel_id, XLIF_XRDP1, CHANNEL_XLIF_RX_IF_IF_DIS, DISABLE, reg_channel_xlif_rx_if_if_dis, disable);

    RU_REG_WRITE(channel_id, XLIF_XRDP1, CHANNEL_XLIF_RX_IF_IF_DIS, reg_channel_xlif_rx_if_if_dis);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xlif_xrdp1_channel_xlif_rx_if_if_dis_get(uint8_t channel_id, bdmf_boolean *disable)
{
    uint32_t reg_channel_xlif_rx_if_if_dis;

#ifdef VALIDATE_PARMS
    if (!disable)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((channel_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(channel_id, XLIF_XRDP1, CHANNEL_XLIF_RX_IF_IF_DIS, reg_channel_xlif_rx_if_if_dis);

    *disable = RU_FIELD_GET(channel_id, XLIF_XRDP1, CHANNEL_XLIF_RX_IF_IF_DIS, DISABLE, reg_channel_xlif_rx_if_if_dis);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xlif_xrdp1_channel_xlif_rx_if_oflw_flag_get(uint8_t channel_id, bdmf_boolean *oflw)
{
    uint32_t reg_channel_xlif_rx_if_oflw_flag;

#ifdef VALIDATE_PARMS
    if (!oflw)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((channel_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(channel_id, XLIF_XRDP1, CHANNEL_XLIF_RX_IF_OFLW_FLAG, reg_channel_xlif_rx_if_oflw_flag);

    *oflw = RU_FIELD_GET(channel_id, XLIF_XRDP1, CHANNEL_XLIF_RX_IF_OFLW_FLAG, OFLW, reg_channel_xlif_rx_if_oflw_flag);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xlif_xrdp1_channel_xlif_rx_if_err_flag_get(uint8_t channel_id, bdmf_boolean *err)
{
    uint32_t reg_channel_xlif_rx_if_err_flag;

#ifdef VALIDATE_PARMS
    if (!err)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((channel_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(channel_id, XLIF_XRDP1, CHANNEL_XLIF_RX_IF_ERR_FLAG, reg_channel_xlif_rx_if_err_flag);

    *err = RU_FIELD_GET(channel_id, XLIF_XRDP1, CHANNEL_XLIF_RX_IF_ERR_FLAG, ERR, reg_channel_xlif_rx_if_err_flag);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xlif_xrdp1_channel_xlif_rx_flow_control_cosmap_en_set(uint8_t channel_id, bdmf_boolean pfc_en, bdmf_boolean llfc_en)
{
    uint32_t reg_channel_xlif_rx_flow_control_cosmap_en = 0;

#ifdef VALIDATE_PARMS
    if ((channel_id >= BLOCK_ADDR_COUNT) ||
       (pfc_en >= _1BITS_MAX_VAL_) ||
       (llfc_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_channel_xlif_rx_flow_control_cosmap_en = RU_FIELD_SET(channel_id, XLIF_XRDP1, CHANNEL_XLIF_RX_FLOW_CONTROL_COSMAP_EN, PFC_EN, reg_channel_xlif_rx_flow_control_cosmap_en, pfc_en);
    reg_channel_xlif_rx_flow_control_cosmap_en = RU_FIELD_SET(channel_id, XLIF_XRDP1, CHANNEL_XLIF_RX_FLOW_CONTROL_COSMAP_EN, LLFC_EN, reg_channel_xlif_rx_flow_control_cosmap_en, llfc_en);

    RU_REG_WRITE(channel_id, XLIF_XRDP1, CHANNEL_XLIF_RX_FLOW_CONTROL_COSMAP_EN, reg_channel_xlif_rx_flow_control_cosmap_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xlif_xrdp1_channel_xlif_rx_flow_control_cosmap_en_get(uint8_t channel_id, bdmf_boolean *pfc_en, bdmf_boolean *llfc_en)
{
    uint32_t reg_channel_xlif_rx_flow_control_cosmap_en;

#ifdef VALIDATE_PARMS
    if (!pfc_en || !llfc_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((channel_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(channel_id, XLIF_XRDP1, CHANNEL_XLIF_RX_FLOW_CONTROL_COSMAP_EN, reg_channel_xlif_rx_flow_control_cosmap_en);

    *pfc_en = RU_FIELD_GET(channel_id, XLIF_XRDP1, CHANNEL_XLIF_RX_FLOW_CONTROL_COSMAP_EN, PFC_EN, reg_channel_xlif_rx_flow_control_cosmap_en);
    *llfc_en = RU_FIELD_GET(channel_id, XLIF_XRDP1, CHANNEL_XLIF_RX_FLOW_CONTROL_COSMAP_EN, LLFC_EN, reg_channel_xlif_rx_flow_control_cosmap_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xlif_xrdp1_channel_xlif_rx_flow_control_cosmap_set(uint8_t channel_id, uint16_t value)
{
    uint32_t reg_channel_xlif_rx_flow_control_cosmap = 0;

#ifdef VALIDATE_PARMS
    if ((channel_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_channel_xlif_rx_flow_control_cosmap = RU_FIELD_SET(channel_id, XLIF_XRDP1, CHANNEL_XLIF_RX_FLOW_CONTROL_COSMAP, VALUE, reg_channel_xlif_rx_flow_control_cosmap, value);

    RU_REG_WRITE(channel_id, XLIF_XRDP1, CHANNEL_XLIF_RX_FLOW_CONTROL_COSMAP, reg_channel_xlif_rx_flow_control_cosmap);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xlif_xrdp1_channel_xlif_rx_flow_control_cosmap_get(uint8_t channel_id, uint16_t *value)
{
    uint32_t reg_channel_xlif_rx_flow_control_cosmap;

#ifdef VALIDATE_PARMS
    if (!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((channel_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(channel_id, XLIF_XRDP1, CHANNEL_XLIF_RX_FLOW_CONTROL_COSMAP, reg_channel_xlif_rx_flow_control_cosmap);

    *value = RU_FIELD_GET(channel_id, XLIF_XRDP1, CHANNEL_XLIF_RX_FLOW_CONTROL_COSMAP, VALUE, reg_channel_xlif_rx_flow_control_cosmap);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xlif_xrdp1_channel_xlif_tx_if_if_enable_set(uint8_t channel_id, bdmf_boolean disable_with_credits, bdmf_boolean disable_wo_credits)
{
    uint32_t reg_channel_xlif_tx_if_if_enable = 0;

#ifdef VALIDATE_PARMS
    if ((channel_id >= BLOCK_ADDR_COUNT) ||
       (disable_with_credits >= _1BITS_MAX_VAL_) ||
       (disable_wo_credits >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_channel_xlif_tx_if_if_enable = RU_FIELD_SET(channel_id, XLIF_XRDP1, CHANNEL_XLIF_TX_IF_IF_ENABLE, DISABLE_WITH_CREDITS, reg_channel_xlif_tx_if_if_enable, disable_with_credits);
    reg_channel_xlif_tx_if_if_enable = RU_FIELD_SET(channel_id, XLIF_XRDP1, CHANNEL_XLIF_TX_IF_IF_ENABLE, DISABLE_WO_CREDITS, reg_channel_xlif_tx_if_if_enable, disable_wo_credits);

    RU_REG_WRITE(channel_id, XLIF_XRDP1, CHANNEL_XLIF_TX_IF_IF_ENABLE, reg_channel_xlif_tx_if_if_enable);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xlif_xrdp1_channel_xlif_tx_if_if_enable_get(uint8_t channel_id, bdmf_boolean *disable_with_credits, bdmf_boolean *disable_wo_credits)
{
    uint32_t reg_channel_xlif_tx_if_if_enable;

#ifdef VALIDATE_PARMS
    if (!disable_with_credits || !disable_wo_credits)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((channel_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(channel_id, XLIF_XRDP1, CHANNEL_XLIF_TX_IF_IF_ENABLE, reg_channel_xlif_tx_if_if_enable);

    *disable_with_credits = RU_FIELD_GET(channel_id, XLIF_XRDP1, CHANNEL_XLIF_TX_IF_IF_ENABLE, DISABLE_WITH_CREDITS, reg_channel_xlif_tx_if_if_enable);
    *disable_wo_credits = RU_FIELD_GET(channel_id, XLIF_XRDP1, CHANNEL_XLIF_TX_IF_IF_ENABLE, DISABLE_WO_CREDITS, reg_channel_xlif_tx_if_if_enable);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xlif_xrdp1_channel_xlif_tx_if_read_credits_get(uint8_t channel_id, uint16_t *value)
{
    uint32_t reg_channel_xlif_tx_if_read_credits;

#ifdef VALIDATE_PARMS
    if (!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((channel_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(channel_id, XLIF_XRDP1, CHANNEL_XLIF_TX_IF_READ_CREDITS, reg_channel_xlif_tx_if_read_credits);

    *value = RU_FIELD_GET(channel_id, XLIF_XRDP1, CHANNEL_XLIF_TX_IF_READ_CREDITS, VALUE, reg_channel_xlif_tx_if_read_credits);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xlif_xrdp1_channel_xlif_tx_if_set_credits_set(uint8_t channel_id, uint16_t value, bdmf_boolean en)
{
    uint32_t reg_channel_xlif_tx_if_set_credits = 0;

#ifdef VALIDATE_PARMS
    if ((channel_id >= BLOCK_ADDR_COUNT) ||
       (value >= _10BITS_MAX_VAL_) ||
       (en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_channel_xlif_tx_if_set_credits = RU_FIELD_SET(channel_id, XLIF_XRDP1, CHANNEL_XLIF_TX_IF_SET_CREDITS, VALUE, reg_channel_xlif_tx_if_set_credits, value);
    reg_channel_xlif_tx_if_set_credits = RU_FIELD_SET(channel_id, XLIF_XRDP1, CHANNEL_XLIF_TX_IF_SET_CREDITS, EN, reg_channel_xlif_tx_if_set_credits, en);

    RU_REG_WRITE(channel_id, XLIF_XRDP1, CHANNEL_XLIF_TX_IF_SET_CREDITS, reg_channel_xlif_tx_if_set_credits);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xlif_xrdp1_channel_xlif_tx_if_set_credits_get(uint8_t channel_id, uint16_t *value, bdmf_boolean *en)
{
    uint32_t reg_channel_xlif_tx_if_set_credits;

#ifdef VALIDATE_PARMS
    if (!value || !en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((channel_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(channel_id, XLIF_XRDP1, CHANNEL_XLIF_TX_IF_SET_CREDITS, reg_channel_xlif_tx_if_set_credits);

    *value = RU_FIELD_GET(channel_id, XLIF_XRDP1, CHANNEL_XLIF_TX_IF_SET_CREDITS, VALUE, reg_channel_xlif_tx_if_set_credits);
    *en = RU_FIELD_GET(channel_id, XLIF_XRDP1, CHANNEL_XLIF_TX_IF_SET_CREDITS, EN, reg_channel_xlif_tx_if_set_credits);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xlif_xrdp1_channel_xlif_tx_if_out_ctrl_set(uint8_t channel_id, bdmf_boolean mac_txerr, bdmf_boolean mac_txcrcerr, bdmf_boolean mac_txosts_sinext, uint8_t mac_txcrcmode)
{
    uint32_t reg_channel_xlif_tx_if_out_ctrl = 0;

#ifdef VALIDATE_PARMS
    if ((channel_id >= BLOCK_ADDR_COUNT) ||
       (mac_txerr >= _1BITS_MAX_VAL_) ||
       (mac_txcrcerr >= _1BITS_MAX_VAL_) ||
       (mac_txosts_sinext >= _1BITS_MAX_VAL_) ||
       (mac_txcrcmode >= _2BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_channel_xlif_tx_if_out_ctrl = RU_FIELD_SET(channel_id, XLIF_XRDP1, CHANNEL_XLIF_TX_IF_OUT_CTRL, MAC_TXERR, reg_channel_xlif_tx_if_out_ctrl, mac_txerr);
    reg_channel_xlif_tx_if_out_ctrl = RU_FIELD_SET(channel_id, XLIF_XRDP1, CHANNEL_XLIF_TX_IF_OUT_CTRL, MAC_TXCRCERR, reg_channel_xlif_tx_if_out_ctrl, mac_txcrcerr);
    reg_channel_xlif_tx_if_out_ctrl = RU_FIELD_SET(channel_id, XLIF_XRDP1, CHANNEL_XLIF_TX_IF_OUT_CTRL, MAC_TXOSTS_SINEXT, reg_channel_xlif_tx_if_out_ctrl, mac_txosts_sinext);
    reg_channel_xlif_tx_if_out_ctrl = RU_FIELD_SET(channel_id, XLIF_XRDP1, CHANNEL_XLIF_TX_IF_OUT_CTRL, MAC_TXCRCMODE, reg_channel_xlif_tx_if_out_ctrl, mac_txcrcmode);

    RU_REG_WRITE(channel_id, XLIF_XRDP1, CHANNEL_XLIF_TX_IF_OUT_CTRL, reg_channel_xlif_tx_if_out_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xlif_xrdp1_channel_xlif_tx_if_out_ctrl_get(uint8_t channel_id, bdmf_boolean *mac_txerr, bdmf_boolean *mac_txcrcerr, bdmf_boolean *mac_txosts_sinext, uint8_t *mac_txcrcmode)
{
    uint32_t reg_channel_xlif_tx_if_out_ctrl;

#ifdef VALIDATE_PARMS
    if (!mac_txerr || !mac_txcrcerr || !mac_txosts_sinext || !mac_txcrcmode)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((channel_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(channel_id, XLIF_XRDP1, CHANNEL_XLIF_TX_IF_OUT_CTRL, reg_channel_xlif_tx_if_out_ctrl);

    *mac_txerr = RU_FIELD_GET(channel_id, XLIF_XRDP1, CHANNEL_XLIF_TX_IF_OUT_CTRL, MAC_TXERR, reg_channel_xlif_tx_if_out_ctrl);
    *mac_txcrcerr = RU_FIELD_GET(channel_id, XLIF_XRDP1, CHANNEL_XLIF_TX_IF_OUT_CTRL, MAC_TXCRCERR, reg_channel_xlif_tx_if_out_ctrl);
    *mac_txosts_sinext = RU_FIELD_GET(channel_id, XLIF_XRDP1, CHANNEL_XLIF_TX_IF_OUT_CTRL, MAC_TXOSTS_SINEXT, reg_channel_xlif_tx_if_out_ctrl);
    *mac_txcrcmode = RU_FIELD_GET(channel_id, XLIF_XRDP1, CHANNEL_XLIF_TX_IF_OUT_CTRL, MAC_TXCRCMODE, reg_channel_xlif_tx_if_out_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xlif_xrdp1_channel_xlif_tx_if_urun_port_enable_set(uint8_t channel_id, bdmf_boolean enable)
{
    uint32_t reg_channel_xlif_tx_if_urun_port_enable = 0;

#ifdef VALIDATE_PARMS
    if ((channel_id >= BLOCK_ADDR_COUNT) ||
       (enable >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_channel_xlif_tx_if_urun_port_enable = RU_FIELD_SET(channel_id, XLIF_XRDP1, CHANNEL_XLIF_TX_IF_URUN_PORT_ENABLE, ENABLE, reg_channel_xlif_tx_if_urun_port_enable, enable);

    RU_REG_WRITE(channel_id, XLIF_XRDP1, CHANNEL_XLIF_TX_IF_URUN_PORT_ENABLE, reg_channel_xlif_tx_if_urun_port_enable);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xlif_xrdp1_channel_xlif_tx_if_urun_port_enable_get(uint8_t channel_id, bdmf_boolean *enable)
{
    uint32_t reg_channel_xlif_tx_if_urun_port_enable;

#ifdef VALIDATE_PARMS
    if (!enable)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((channel_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(channel_id, XLIF_XRDP1, CHANNEL_XLIF_TX_IF_URUN_PORT_ENABLE, reg_channel_xlif_tx_if_urun_port_enable);

    *enable = RU_FIELD_GET(channel_id, XLIF_XRDP1, CHANNEL_XLIF_TX_IF_URUN_PORT_ENABLE, ENABLE, reg_channel_xlif_tx_if_urun_port_enable);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xlif_xrdp1_channel_xlif_tx_if_tx_threshold_set(uint8_t channel_id, uint8_t value)
{
    uint32_t reg_channel_xlif_tx_if_tx_threshold = 0;

#ifdef VALIDATE_PARMS
    if ((channel_id >= BLOCK_ADDR_COUNT) ||
       (value >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_channel_xlif_tx_if_tx_threshold = RU_FIELD_SET(channel_id, XLIF_XRDP1, CHANNEL_XLIF_TX_IF_TX_THRESHOLD, VALUE, reg_channel_xlif_tx_if_tx_threshold, value);

    RU_REG_WRITE(channel_id, XLIF_XRDP1, CHANNEL_XLIF_TX_IF_TX_THRESHOLD, reg_channel_xlif_tx_if_tx_threshold);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xlif_xrdp1_channel_xlif_tx_if_tx_threshold_get(uint8_t channel_id, uint8_t *value)
{
    uint32_t reg_channel_xlif_tx_if_tx_threshold;

#ifdef VALIDATE_PARMS
    if (!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((channel_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(channel_id, XLIF_XRDP1, CHANNEL_XLIF_TX_IF_TX_THRESHOLD, reg_channel_xlif_tx_if_tx_threshold);

    *value = RU_FIELD_GET(channel_id, XLIF_XRDP1, CHANNEL_XLIF_TX_IF_TX_THRESHOLD, VALUE, reg_channel_xlif_tx_if_tx_threshold);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xlif_xrdp1_channel_xlif_tx_if_tdm_mode_set(uint8_t channel_id, bdmf_boolean value)
{
    uint32_t reg_channel_xlif_tx_if_tdm_mode = 0;

#ifdef VALIDATE_PARMS
    if ((channel_id >= BLOCK_ADDR_COUNT) ||
       (value >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_channel_xlif_tx_if_tdm_mode = RU_FIELD_SET(channel_id, XLIF_XRDP1, CHANNEL_XLIF_TX_IF_TDM_MODE, VALUE, reg_channel_xlif_tx_if_tdm_mode, value);

    RU_REG_WRITE(channel_id, XLIF_XRDP1, CHANNEL_XLIF_TX_IF_TDM_MODE, reg_channel_xlif_tx_if_tdm_mode);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xlif_xrdp1_channel_xlif_tx_if_tdm_mode_get(uint8_t channel_id, bdmf_boolean *value)
{
    uint32_t reg_channel_xlif_tx_if_tdm_mode;

#ifdef VALIDATE_PARMS
    if (!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((channel_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(channel_id, XLIF_XRDP1, CHANNEL_XLIF_TX_IF_TDM_MODE, reg_channel_xlif_tx_if_tdm_mode);

    *value = RU_FIELD_GET(channel_id, XLIF_XRDP1, CHANNEL_XLIF_TX_IF_TDM_MODE, VALUE, reg_channel_xlif_tx_if_tdm_mode);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xlif_xrdp1_channel_xlif_tx_flow_control_cosmap_en_stat_get(uint8_t channel_id, bdmf_boolean *pfc_en, bdmf_boolean *llfc_en)
{
    uint32_t reg_channel_xlif_tx_flow_control_cosmap_en_stat;

#ifdef VALIDATE_PARMS
    if (!pfc_en || !llfc_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((channel_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(channel_id, XLIF_XRDP1, CHANNEL_XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT, reg_channel_xlif_tx_flow_control_cosmap_en_stat);

    *pfc_en = RU_FIELD_GET(channel_id, XLIF_XRDP1, CHANNEL_XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT, PFC_EN, reg_channel_xlif_tx_flow_control_cosmap_en_stat);
    *llfc_en = RU_FIELD_GET(channel_id, XLIF_XRDP1, CHANNEL_XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT, LLFC_EN, reg_channel_xlif_tx_flow_control_cosmap_en_stat);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xlif_xrdp1_channel_xlif_tx_flow_control_cosmap_stat_get(uint8_t channel_id, uint16_t *value)
{
    uint32_t reg_channel_xlif_tx_flow_control_cosmap_stat;

#ifdef VALIDATE_PARMS
    if (!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((channel_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(channel_id, XLIF_XRDP1, CHANNEL_XLIF_TX_FLOW_CONTROL_COSMAP_STAT, reg_channel_xlif_tx_flow_control_cosmap_stat);

    *value = RU_FIELD_GET(channel_id, XLIF_XRDP1, CHANNEL_XLIF_TX_FLOW_CONTROL_COSMAP_STAT, VALUE, reg_channel_xlif_tx_flow_control_cosmap_stat);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xlif_xrdp1_channel_debug_bus_sel_set(uint8_t channel_id, uint8_t select_module, uint8_t select_lane)
{
    uint32_t reg_channel_debug_bus_sel = 0;

#ifdef VALIDATE_PARMS
    if ((channel_id >= BLOCK_ADDR_COUNT) ||
       (select_module >= _2BITS_MAX_VAL_) ||
       (select_lane >= _2BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_channel_debug_bus_sel = RU_FIELD_SET(channel_id, XLIF_XRDP1, CHANNEL_DEBUG_BUS_SEL, SELECT_MODULE, reg_channel_debug_bus_sel, select_module);
    reg_channel_debug_bus_sel = RU_FIELD_SET(channel_id, XLIF_XRDP1, CHANNEL_DEBUG_BUS_SEL, SELECT_LANE, reg_channel_debug_bus_sel, select_lane);

    RU_REG_WRITE(channel_id, XLIF_XRDP1, CHANNEL_DEBUG_BUS_SEL, reg_channel_debug_bus_sel);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xlif_xrdp1_channel_debug_bus_sel_get(uint8_t channel_id, uint8_t *select_module, uint8_t *select_lane)
{
    uint32_t reg_channel_debug_bus_sel;

#ifdef VALIDATE_PARMS
    if (!select_module || !select_lane)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((channel_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(channel_id, XLIF_XRDP1, CHANNEL_DEBUG_BUS_SEL, reg_channel_debug_bus_sel);

    *select_module = RU_FIELD_GET(channel_id, XLIF_XRDP1, CHANNEL_DEBUG_BUS_SEL, SELECT_MODULE, reg_channel_debug_bus_sel);
    *select_lane = RU_FIELD_GET(channel_id, XLIF_XRDP1, CHANNEL_DEBUG_BUS_SEL, SELECT_LANE, reg_channel_debug_bus_sel);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xlif_xrdp1_channel_xlif_eee_ind_get(uint8_t channel_id, bdmf_boolean *lpi_rx_detect, bdmf_boolean *lpi_tx_detect)
{
    uint32_t reg_channel_xlif_eee_ind;

#ifdef VALIDATE_PARMS
    if (!lpi_rx_detect || !lpi_tx_detect)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((channel_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(channel_id, XLIF_XRDP1, CHANNEL_XLIF_EEE_IND, reg_channel_xlif_eee_ind);

    *lpi_rx_detect = RU_FIELD_GET(channel_id, XLIF_XRDP1, CHANNEL_XLIF_EEE_IND, LPI_RX_DETECT, reg_channel_xlif_eee_ind);
    *lpi_tx_detect = RU_FIELD_GET(channel_id, XLIF_XRDP1, CHANNEL_XLIF_EEE_IND, LPI_TX_DETECT, reg_channel_xlif_eee_ind);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xlif_xrdp1_channel_q_off_ind_get(uint8_t channel_id, uint8_t *q_off, bdmf_boolean *failover_on)
{
    uint32_t reg_channel_q_off_ind;

#ifdef VALIDATE_PARMS
    if (!q_off || !failover_on)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((channel_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(channel_id, XLIF_XRDP1, CHANNEL_Q_OFF_IND, reg_channel_q_off_ind);

    *q_off = RU_FIELD_GET(channel_id, XLIF_XRDP1, CHANNEL_Q_OFF_IND, Q_OFF, reg_channel_q_off_ind);
    *failover_on = RU_FIELD_GET(channel_id, XLIF_XRDP1, CHANNEL_Q_OFF_IND, FAILOVER_ON, reg_channel_q_off_ind);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xlif_xrdp1_channel_lane_to_port_map_port_map_set(uint8_t channel_id, uint8_t lane0, uint8_t lane1, uint8_t lane2, uint8_t lane3)
{
    uint32_t reg_channel_lane_to_port_map_port_map = 0;

#ifdef VALIDATE_PARMS
    if ((channel_id >= BLOCK_ADDR_COUNT) ||
       (lane0 >= _2BITS_MAX_VAL_) ||
       (lane1 >= _2BITS_MAX_VAL_) ||
       (lane2 >= _2BITS_MAX_VAL_) ||
       (lane3 >= _2BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_channel_lane_to_port_map_port_map = RU_FIELD_SET(channel_id, XLIF_XRDP1, CHANNEL_LANE_TO_PORT_MAP_PORT_MAP, LANE0, reg_channel_lane_to_port_map_port_map, lane0);
    reg_channel_lane_to_port_map_port_map = RU_FIELD_SET(channel_id, XLIF_XRDP1, CHANNEL_LANE_TO_PORT_MAP_PORT_MAP, LANE1, reg_channel_lane_to_port_map_port_map, lane1);
    reg_channel_lane_to_port_map_port_map = RU_FIELD_SET(channel_id, XLIF_XRDP1, CHANNEL_LANE_TO_PORT_MAP_PORT_MAP, LANE2, reg_channel_lane_to_port_map_port_map, lane2);
    reg_channel_lane_to_port_map_port_map = RU_FIELD_SET(channel_id, XLIF_XRDP1, CHANNEL_LANE_TO_PORT_MAP_PORT_MAP, LANE3, reg_channel_lane_to_port_map_port_map, lane3);

    RU_REG_WRITE(channel_id, XLIF_XRDP1, CHANNEL_LANE_TO_PORT_MAP_PORT_MAP, reg_channel_lane_to_port_map_port_map);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xlif_xrdp1_channel_lane_to_port_map_port_map_get(uint8_t channel_id, uint8_t *lane0, uint8_t *lane1, uint8_t *lane2, uint8_t *lane3)
{
    uint32_t reg_channel_lane_to_port_map_port_map;

#ifdef VALIDATE_PARMS
    if (!lane0 || !lane1 || !lane2 || !lane3)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((channel_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(channel_id, XLIF_XRDP1, CHANNEL_LANE_TO_PORT_MAP_PORT_MAP, reg_channel_lane_to_port_map_port_map);

    *lane0 = RU_FIELD_GET(channel_id, XLIF_XRDP1, CHANNEL_LANE_TO_PORT_MAP_PORT_MAP, LANE0, reg_channel_lane_to_port_map_port_map);
    *lane1 = RU_FIELD_GET(channel_id, XLIF_XRDP1, CHANNEL_LANE_TO_PORT_MAP_PORT_MAP, LANE1, reg_channel_lane_to_port_map_port_map);
    *lane2 = RU_FIELD_GET(channel_id, XLIF_XRDP1, CHANNEL_LANE_TO_PORT_MAP_PORT_MAP, LANE2, reg_channel_lane_to_port_map_port_map);
    *lane3 = RU_FIELD_GET(channel_id, XLIF_XRDP1, CHANNEL_LANE_TO_PORT_MAP_PORT_MAP, LANE3, reg_channel_lane_to_port_map_port_map);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL

typedef enum
{
    bdmf_address_channel_xlif_rx_if_if_dis,
    bdmf_address_channel_xlif_rx_if_oflw_flag,
    bdmf_address_channel_xlif_rx_if_err_flag,
    bdmf_address_channel_xlif_rx_flow_control_cosmap_en,
    bdmf_address_channel_xlif_rx_flow_control_cosmap,
    bdmf_address_channel_xlif_tx_if_if_enable,
    bdmf_address_channel_xlif_tx_if_read_credits,
    bdmf_address_channel_xlif_tx_if_set_credits,
    bdmf_address_channel_xlif_tx_if_out_ctrl,
    bdmf_address_channel_xlif_tx_if_urun_port_enable,
    bdmf_address_channel_xlif_tx_if_tx_threshold,
    bdmf_address_channel_xlif_tx_if_tdm_mode,
    bdmf_address_channel_xlif_tx_flow_control_cosmap_en_stat,
    bdmf_address_channel_xlif_tx_flow_control_cosmap_stat,
    bdmf_address_channel_debug_bus_sel,
    bdmf_address_channel_xlif_eee_ind,
    bdmf_address_channel_q_off_ind,
    bdmf_address_channel_lane_to_port_map_port_map,
}
bdmf_address;

static int ag_drv_xlif_xrdp1_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_xlif_xrdp1_channel_xlif_rx_if_if_dis:
        ag_err = ag_drv_xlif_xrdp1_channel_xlif_rx_if_if_dis_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xlif_xrdp1_channel_xlif_rx_flow_control_cosmap_en:
        ag_err = ag_drv_xlif_xrdp1_channel_xlif_rx_flow_control_cosmap_en_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_xlif_xrdp1_channel_xlif_rx_flow_control_cosmap:
        ag_err = ag_drv_xlif_xrdp1_channel_xlif_rx_flow_control_cosmap_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xlif_xrdp1_channel_xlif_tx_if_if_enable:
        ag_err = ag_drv_xlif_xrdp1_channel_xlif_tx_if_if_enable_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_xlif_xrdp1_channel_xlif_tx_if_set_credits:
        ag_err = ag_drv_xlif_xrdp1_channel_xlif_tx_if_set_credits_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_xlif_xrdp1_channel_xlif_tx_if_out_ctrl:
        ag_err = ag_drv_xlif_xrdp1_channel_xlif_tx_if_out_ctrl_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber);
        break;
    case cli_xlif_xrdp1_channel_xlif_tx_if_urun_port_enable:
        ag_err = ag_drv_xlif_xrdp1_channel_xlif_tx_if_urun_port_enable_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xlif_xrdp1_channel_xlif_tx_if_tx_threshold:
        ag_err = ag_drv_xlif_xrdp1_channel_xlif_tx_if_tx_threshold_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xlif_xrdp1_channel_xlif_tx_if_tdm_mode:
        ag_err = ag_drv_xlif_xrdp1_channel_xlif_tx_if_tdm_mode_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xlif_xrdp1_channel_debug_bus_sel:
        ag_err = ag_drv_xlif_xrdp1_channel_debug_bus_sel_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_xlif_xrdp1_channel_lane_to_port_map_port_map:
        ag_err = ag_drv_xlif_xrdp1_channel_lane_to_port_map_port_map_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber);
        break;
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

int bcm_xlif_xrdp1_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_xlif_xrdp1_channel_xlif_rx_if_if_dis:
    {
        bdmf_boolean disable;
        ag_err = ag_drv_xlif_xrdp1_channel_xlif_rx_if_if_dis_get(parm[1].value.unumber, &disable);
        bdmf_session_print(session, "disable = %u = 0x%x\n", disable, disable);
        break;
    }
    case cli_xlif_xrdp1_channel_xlif_rx_if_oflw_flag:
    {
        bdmf_boolean oflw;
        ag_err = ag_drv_xlif_xrdp1_channel_xlif_rx_if_oflw_flag_get(parm[1].value.unumber, &oflw);
        bdmf_session_print(session, "oflw = %u = 0x%x\n", oflw, oflw);
        break;
    }
    case cli_xlif_xrdp1_channel_xlif_rx_if_err_flag:
    {
        bdmf_boolean err;
        ag_err = ag_drv_xlif_xrdp1_channel_xlif_rx_if_err_flag_get(parm[1].value.unumber, &err);
        bdmf_session_print(session, "err = %u = 0x%x\n", err, err);
        break;
    }
    case cli_xlif_xrdp1_channel_xlif_rx_flow_control_cosmap_en:
    {
        bdmf_boolean pfc_en;
        bdmf_boolean llfc_en;
        ag_err = ag_drv_xlif_xrdp1_channel_xlif_rx_flow_control_cosmap_en_get(parm[1].value.unumber, &pfc_en, &llfc_en);
        bdmf_session_print(session, "pfc_en = %u = 0x%x\n", pfc_en, pfc_en);
        bdmf_session_print(session, "llfc_en = %u = 0x%x\n", llfc_en, llfc_en);
        break;
    }
    case cli_xlif_xrdp1_channel_xlif_rx_flow_control_cosmap:
    {
        uint16_t value;
        ag_err = ag_drv_xlif_xrdp1_channel_xlif_rx_flow_control_cosmap_get(parm[1].value.unumber, &value);
        bdmf_session_print(session, "value = %u = 0x%x\n", value, value);
        break;
    }
    case cli_xlif_xrdp1_channel_xlif_tx_if_if_enable:
    {
        bdmf_boolean disable_with_credits;
        bdmf_boolean disable_wo_credits;
        ag_err = ag_drv_xlif_xrdp1_channel_xlif_tx_if_if_enable_get(parm[1].value.unumber, &disable_with_credits, &disable_wo_credits);
        bdmf_session_print(session, "disable_with_credits = %u = 0x%x\n", disable_with_credits, disable_with_credits);
        bdmf_session_print(session, "disable_wo_credits = %u = 0x%x\n", disable_wo_credits, disable_wo_credits);
        break;
    }
    case cli_xlif_xrdp1_channel_xlif_tx_if_read_credits:
    {
        uint16_t value;
        ag_err = ag_drv_xlif_xrdp1_channel_xlif_tx_if_read_credits_get(parm[1].value.unumber, &value);
        bdmf_session_print(session, "value = %u = 0x%x\n", value, value);
        break;
    }
    case cli_xlif_xrdp1_channel_xlif_tx_if_set_credits:
    {
        uint16_t value;
        bdmf_boolean en;
        ag_err = ag_drv_xlif_xrdp1_channel_xlif_tx_if_set_credits_get(parm[1].value.unumber, &value, &en);
        bdmf_session_print(session, "value = %u = 0x%x\n", value, value);
        bdmf_session_print(session, "en = %u = 0x%x\n", en, en);
        break;
    }
    case cli_xlif_xrdp1_channel_xlif_tx_if_out_ctrl:
    {
        bdmf_boolean mac_txerr;
        bdmf_boolean mac_txcrcerr;
        bdmf_boolean mac_txosts_sinext;
        uint8_t mac_txcrcmode;
        ag_err = ag_drv_xlif_xrdp1_channel_xlif_tx_if_out_ctrl_get(parm[1].value.unumber, &mac_txerr, &mac_txcrcerr, &mac_txosts_sinext, &mac_txcrcmode);
        bdmf_session_print(session, "mac_txerr = %u = 0x%x\n", mac_txerr, mac_txerr);
        bdmf_session_print(session, "mac_txcrcerr = %u = 0x%x\n", mac_txcrcerr, mac_txcrcerr);
        bdmf_session_print(session, "mac_txosts_sinext = %u = 0x%x\n", mac_txosts_sinext, mac_txosts_sinext);
        bdmf_session_print(session, "mac_txcrcmode = %u = 0x%x\n", mac_txcrcmode, mac_txcrcmode);
        break;
    }
    case cli_xlif_xrdp1_channel_xlif_tx_if_urun_port_enable:
    {
        bdmf_boolean enable;
        ag_err = ag_drv_xlif_xrdp1_channel_xlif_tx_if_urun_port_enable_get(parm[1].value.unumber, &enable);
        bdmf_session_print(session, "enable = %u = 0x%x\n", enable, enable);
        break;
    }
    case cli_xlif_xrdp1_channel_xlif_tx_if_tx_threshold:
    {
        uint8_t value;
        ag_err = ag_drv_xlif_xrdp1_channel_xlif_tx_if_tx_threshold_get(parm[1].value.unumber, &value);
        bdmf_session_print(session, "value = %u = 0x%x\n", value, value);
        break;
    }
    case cli_xlif_xrdp1_channel_xlif_tx_if_tdm_mode:
    {
        bdmf_boolean value;
        ag_err = ag_drv_xlif_xrdp1_channel_xlif_tx_if_tdm_mode_get(parm[1].value.unumber, &value);
        bdmf_session_print(session, "value = %u = 0x%x\n", value, value);
        break;
    }
    case cli_xlif_xrdp1_channel_xlif_tx_flow_control_cosmap_en_stat:
    {
        bdmf_boolean pfc_en;
        bdmf_boolean llfc_en;
        ag_err = ag_drv_xlif_xrdp1_channel_xlif_tx_flow_control_cosmap_en_stat_get(parm[1].value.unumber, &pfc_en, &llfc_en);
        bdmf_session_print(session, "pfc_en = %u = 0x%x\n", pfc_en, pfc_en);
        bdmf_session_print(session, "llfc_en = %u = 0x%x\n", llfc_en, llfc_en);
        break;
    }
    case cli_xlif_xrdp1_channel_xlif_tx_flow_control_cosmap_stat:
    {
        uint16_t value;
        ag_err = ag_drv_xlif_xrdp1_channel_xlif_tx_flow_control_cosmap_stat_get(parm[1].value.unumber, &value);
        bdmf_session_print(session, "value = %u = 0x%x\n", value, value);
        break;
    }
    case cli_xlif_xrdp1_channel_debug_bus_sel:
    {
        uint8_t select_module;
        uint8_t select_lane;
        ag_err = ag_drv_xlif_xrdp1_channel_debug_bus_sel_get(parm[1].value.unumber, &select_module, &select_lane);
        bdmf_session_print(session, "select_module = %u = 0x%x\n", select_module, select_module);
        bdmf_session_print(session, "select_lane = %u = 0x%x\n", select_lane, select_lane);
        break;
    }
    case cli_xlif_xrdp1_channel_xlif_eee_ind:
    {
        bdmf_boolean lpi_rx_detect;
        bdmf_boolean lpi_tx_detect;
        ag_err = ag_drv_xlif_xrdp1_channel_xlif_eee_ind_get(parm[1].value.unumber, &lpi_rx_detect, &lpi_tx_detect);
        bdmf_session_print(session, "lpi_rx_detect = %u = 0x%x\n", lpi_rx_detect, lpi_rx_detect);
        bdmf_session_print(session, "lpi_tx_detect = %u = 0x%x\n", lpi_tx_detect, lpi_tx_detect);
        break;
    }
    case cli_xlif_xrdp1_channel_q_off_ind:
    {
        uint8_t q_off;
        bdmf_boolean failover_on;
        ag_err = ag_drv_xlif_xrdp1_channel_q_off_ind_get(parm[1].value.unumber, &q_off, &failover_on);
        bdmf_session_print(session, "q_off = %u = 0x%x\n", q_off, q_off);
        bdmf_session_print(session, "failover_on = %u = 0x%x\n", failover_on, failover_on);
        break;
    }
    case cli_xlif_xrdp1_channel_lane_to_port_map_port_map:
    {
        uint8_t lane0;
        uint8_t lane1;
        uint8_t lane2;
        uint8_t lane3;
        ag_err = ag_drv_xlif_xrdp1_channel_lane_to_port_map_port_map_get(parm[1].value.unumber, &lane0, &lane1, &lane2, &lane3);
        bdmf_session_print(session, "lane0 = %u = 0x%x\n", lane0, lane0);
        bdmf_session_print(session, "lane1 = %u = 0x%x\n", lane1, lane1);
        bdmf_session_print(session, "lane2 = %u = 0x%x\n", lane2, lane2);
        bdmf_session_print(session, "lane3 = %u = 0x%x\n", lane3, lane3);
        break;
    }
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

#ifdef HAL_DRV_TEST_ENABLE
static int ag_drv_xlif_xrdp1_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    uint8_t channel_id = parm[1].value.unumber;
    bdmf_error_t ag_err = BDMF_ERR_OK;
    uint32_t test_success_cnt = 0;
    uint32_t test_failure_cnt = 0;

    {
        bdmf_boolean disable = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_xlif_xrdp1_channel_xlif_rx_if_if_dis_set(%u %u)\n", channel_id,
            disable);
        ag_err = ag_drv_xlif_xrdp1_channel_xlif_rx_if_if_dis_set(channel_id, disable);
        if (!ag_err)
            ag_err = ag_drv_xlif_xrdp1_channel_xlif_rx_if_if_dis_get(channel_id, &disable);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xlif_xrdp1_channel_xlif_rx_if_if_dis_get(%u %u)\n", channel_id,
                disable);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (disable != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean oflw = gtmv(m, 1);
        if (!ag_err)
            ag_err = ag_drv_xlif_xrdp1_channel_xlif_rx_if_oflw_flag_get(channel_id, &oflw);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xlif_xrdp1_channel_xlif_rx_if_oflw_flag_get(%u %u)\n", channel_id,
                oflw);
        }
    }

    {
        bdmf_boolean err = gtmv(m, 1);
        if (!ag_err)
            ag_err = ag_drv_xlif_xrdp1_channel_xlif_rx_if_err_flag_get(channel_id, &err);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xlif_xrdp1_channel_xlif_rx_if_err_flag_get(%u %u)\n", channel_id,
                err);
        }
    }

    {
        bdmf_boolean pfc_en = gtmv(m, 1);
        bdmf_boolean llfc_en = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_xlif_xrdp1_channel_xlif_rx_flow_control_cosmap_en_set(%u %u %u)\n", channel_id,
            pfc_en, llfc_en);
        ag_err = ag_drv_xlif_xrdp1_channel_xlif_rx_flow_control_cosmap_en_set(channel_id, pfc_en, llfc_en);
        if (!ag_err)
            ag_err = ag_drv_xlif_xrdp1_channel_xlif_rx_flow_control_cosmap_en_get(channel_id, &pfc_en, &llfc_en);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xlif_xrdp1_channel_xlif_rx_flow_control_cosmap_en_get(%u %u %u)\n", channel_id,
                pfc_en, llfc_en);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (pfc_en != gtmv(m, 1) || llfc_en != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t value = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_xlif_xrdp1_channel_xlif_rx_flow_control_cosmap_set(%u %u)\n", channel_id,
            value);
        ag_err = ag_drv_xlif_xrdp1_channel_xlif_rx_flow_control_cosmap_set(channel_id, value);
        if (!ag_err)
            ag_err = ag_drv_xlif_xrdp1_channel_xlif_rx_flow_control_cosmap_get(channel_id, &value);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xlif_xrdp1_channel_xlif_rx_flow_control_cosmap_get(%u %u)\n", channel_id,
                value);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (value != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean disable_with_credits = gtmv(m, 1);
        bdmf_boolean disable_wo_credits = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_xlif_xrdp1_channel_xlif_tx_if_if_enable_set(%u %u %u)\n", channel_id,
            disable_with_credits, disable_wo_credits);
        ag_err = ag_drv_xlif_xrdp1_channel_xlif_tx_if_if_enable_set(channel_id, disable_with_credits, disable_wo_credits);
        if (!ag_err)
            ag_err = ag_drv_xlif_xrdp1_channel_xlif_tx_if_if_enable_get(channel_id, &disable_with_credits, &disable_wo_credits);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xlif_xrdp1_channel_xlif_tx_if_if_enable_get(%u %u %u)\n", channel_id,
                disable_with_credits, disable_wo_credits);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (disable_with_credits != gtmv(m, 1) || disable_wo_credits != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t value = gtmv(m, 10);
        if (!ag_err)
            ag_err = ag_drv_xlif_xrdp1_channel_xlif_tx_if_read_credits_get(channel_id, &value);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xlif_xrdp1_channel_xlif_tx_if_read_credits_get(%u %u)\n", channel_id,
                value);
        }
    }

    {
        uint16_t value = gtmv(m, 10);
        bdmf_boolean en = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_xlif_xrdp1_channel_xlif_tx_if_set_credits_set(%u %u %u)\n", channel_id,
            value, en);
        ag_err = ag_drv_xlif_xrdp1_channel_xlif_tx_if_set_credits_set(channel_id, value, en);
        if (!ag_err)
            ag_err = ag_drv_xlif_xrdp1_channel_xlif_tx_if_set_credits_get(channel_id, &value, &en);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xlif_xrdp1_channel_xlif_tx_if_set_credits_get(%u %u %u)\n", channel_id,
                value, en);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (value != gtmv(m, 10) || en != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean mac_txerr = gtmv(m, 1);
        bdmf_boolean mac_txcrcerr = gtmv(m, 1);
        bdmf_boolean mac_txosts_sinext = gtmv(m, 1);
        uint8_t mac_txcrcmode = gtmv(m, 2);
        bdmf_session_print(session, "ag_drv_xlif_xrdp1_channel_xlif_tx_if_out_ctrl_set(%u %u %u %u %u)\n", channel_id,
            mac_txerr, mac_txcrcerr, mac_txosts_sinext, mac_txcrcmode);
        ag_err = ag_drv_xlif_xrdp1_channel_xlif_tx_if_out_ctrl_set(channel_id, mac_txerr, mac_txcrcerr, mac_txosts_sinext, mac_txcrcmode);
        if (!ag_err)
            ag_err = ag_drv_xlif_xrdp1_channel_xlif_tx_if_out_ctrl_get(channel_id, &mac_txerr, &mac_txcrcerr, &mac_txosts_sinext, &mac_txcrcmode);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xlif_xrdp1_channel_xlif_tx_if_out_ctrl_get(%u %u %u %u %u)\n", channel_id,
                mac_txerr, mac_txcrcerr, mac_txosts_sinext, mac_txcrcmode);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (mac_txerr != gtmv(m, 1) || mac_txcrcerr != gtmv(m, 1) || mac_txosts_sinext != gtmv(m, 1) || mac_txcrcmode != gtmv(m, 2))
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
        bdmf_session_print(session, "ag_drv_xlif_xrdp1_channel_xlif_tx_if_urun_port_enable_set(%u %u)\n", channel_id,
            enable);
        ag_err = ag_drv_xlif_xrdp1_channel_xlif_tx_if_urun_port_enable_set(channel_id, enable);
        if (!ag_err)
            ag_err = ag_drv_xlif_xrdp1_channel_xlif_tx_if_urun_port_enable_get(channel_id, &enable);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xlif_xrdp1_channel_xlif_tx_if_urun_port_enable_get(%u %u)\n", channel_id,
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
        uint8_t value = gtmv(m, 4);
        bdmf_session_print(session, "ag_drv_xlif_xrdp1_channel_xlif_tx_if_tx_threshold_set(%u %u)\n", channel_id,
            value);
        ag_err = ag_drv_xlif_xrdp1_channel_xlif_tx_if_tx_threshold_set(channel_id, value);
        if (!ag_err)
            ag_err = ag_drv_xlif_xrdp1_channel_xlif_tx_if_tx_threshold_get(channel_id, &value);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xlif_xrdp1_channel_xlif_tx_if_tx_threshold_get(%u %u)\n", channel_id,
                value);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (value != gtmv(m, 4))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean value = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_xlif_xrdp1_channel_xlif_tx_if_tdm_mode_set(%u %u)\n", channel_id,
            value);
        ag_err = ag_drv_xlif_xrdp1_channel_xlif_tx_if_tdm_mode_set(channel_id, value);
        if (!ag_err)
            ag_err = ag_drv_xlif_xrdp1_channel_xlif_tx_if_tdm_mode_get(channel_id, &value);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xlif_xrdp1_channel_xlif_tx_if_tdm_mode_get(%u %u)\n", channel_id,
                value);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (value != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean pfc_en = gtmv(m, 1);
        bdmf_boolean llfc_en = gtmv(m, 1);
        if (!ag_err)
            ag_err = ag_drv_xlif_xrdp1_channel_xlif_tx_flow_control_cosmap_en_stat_get(channel_id, &pfc_en, &llfc_en);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xlif_xrdp1_channel_xlif_tx_flow_control_cosmap_en_stat_get(%u %u %u)\n", channel_id,
                pfc_en, llfc_en);
        }
    }

    {
        uint16_t value = gtmv(m, 16);
        if (!ag_err)
            ag_err = ag_drv_xlif_xrdp1_channel_xlif_tx_flow_control_cosmap_stat_get(channel_id, &value);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xlif_xrdp1_channel_xlif_tx_flow_control_cosmap_stat_get(%u %u)\n", channel_id,
                value);
        }
    }

    {
        uint8_t select_module = gtmv(m, 2);
        uint8_t select_lane = gtmv(m, 2);
        bdmf_session_print(session, "ag_drv_xlif_xrdp1_channel_debug_bus_sel_set(%u %u %u)\n", channel_id,
            select_module, select_lane);
        ag_err = ag_drv_xlif_xrdp1_channel_debug_bus_sel_set(channel_id, select_module, select_lane);
        if (!ag_err)
            ag_err = ag_drv_xlif_xrdp1_channel_debug_bus_sel_get(channel_id, &select_module, &select_lane);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xlif_xrdp1_channel_debug_bus_sel_get(%u %u %u)\n", channel_id,
                select_module, select_lane);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (select_module != gtmv(m, 2) || select_lane != gtmv(m, 2))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean lpi_rx_detect = gtmv(m, 1);
        bdmf_boolean lpi_tx_detect = gtmv(m, 1);
        if (!ag_err)
            ag_err = ag_drv_xlif_xrdp1_channel_xlif_eee_ind_get(channel_id, &lpi_rx_detect, &lpi_tx_detect);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xlif_xrdp1_channel_xlif_eee_ind_get(%u %u %u)\n", channel_id,
                lpi_rx_detect, lpi_tx_detect);
        }
    }

    {
        uint8_t q_off = gtmv(m, 8);
        bdmf_boolean failover_on = gtmv(m, 1);
        if (!ag_err)
            ag_err = ag_drv_xlif_xrdp1_channel_q_off_ind_get(channel_id, &q_off, &failover_on);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xlif_xrdp1_channel_q_off_ind_get(%u %u %u)\n", channel_id,
                q_off, failover_on);
        }
    }

    {
        uint8_t lane0 = gtmv(m, 2);
        uint8_t lane1 = gtmv(m, 2);
        uint8_t lane2 = gtmv(m, 2);
        uint8_t lane3 = gtmv(m, 2);
        bdmf_session_print(session, "ag_drv_xlif_xrdp1_channel_lane_to_port_map_port_map_set(%u %u %u %u %u)\n", channel_id,
            lane0, lane1, lane2, lane3);
        ag_err = ag_drv_xlif_xrdp1_channel_lane_to_port_map_port_map_set(channel_id, lane0, lane1, lane2, lane3);
        if (!ag_err)
            ag_err = ag_drv_xlif_xrdp1_channel_lane_to_port_map_port_map_get(channel_id, &lane0, &lane1, &lane2, &lane3);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xlif_xrdp1_channel_lane_to_port_map_port_map_get(%u %u %u %u %u)\n", channel_id,
                lane0, lane1, lane2, lane3);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (lane0 != gtmv(m, 2) || lane1 != gtmv(m, 2) || lane2 != gtmv(m, 2) || lane3 != gtmv(m, 2))
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
static int ag_drv_xlif_xrdp1_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_channel_xlif_rx_if_if_dis: reg = &RU_REG(XLIF_XRDP1, CHANNEL_XLIF_RX_IF_IF_DIS); blk = &RU_BLK(XLIF_XRDP1); break;
    case bdmf_address_channel_xlif_rx_if_oflw_flag: reg = &RU_REG(XLIF_XRDP1, CHANNEL_XLIF_RX_IF_OFLW_FLAG); blk = &RU_BLK(XLIF_XRDP1); break;
    case bdmf_address_channel_xlif_rx_if_err_flag: reg = &RU_REG(XLIF_XRDP1, CHANNEL_XLIF_RX_IF_ERR_FLAG); blk = &RU_BLK(XLIF_XRDP1); break;
    case bdmf_address_channel_xlif_rx_flow_control_cosmap_en: reg = &RU_REG(XLIF_XRDP1, CHANNEL_XLIF_RX_FLOW_CONTROL_COSMAP_EN); blk = &RU_BLK(XLIF_XRDP1); break;
    case bdmf_address_channel_xlif_rx_flow_control_cosmap: reg = &RU_REG(XLIF_XRDP1, CHANNEL_XLIF_RX_FLOW_CONTROL_COSMAP); blk = &RU_BLK(XLIF_XRDP1); break;
    case bdmf_address_channel_xlif_tx_if_if_enable: reg = &RU_REG(XLIF_XRDP1, CHANNEL_XLIF_TX_IF_IF_ENABLE); blk = &RU_BLK(XLIF_XRDP1); break;
    case bdmf_address_channel_xlif_tx_if_read_credits: reg = &RU_REG(XLIF_XRDP1, CHANNEL_XLIF_TX_IF_READ_CREDITS); blk = &RU_BLK(XLIF_XRDP1); break;
    case bdmf_address_channel_xlif_tx_if_set_credits: reg = &RU_REG(XLIF_XRDP1, CHANNEL_XLIF_TX_IF_SET_CREDITS); blk = &RU_BLK(XLIF_XRDP1); break;
    case bdmf_address_channel_xlif_tx_if_out_ctrl: reg = &RU_REG(XLIF_XRDP1, CHANNEL_XLIF_TX_IF_OUT_CTRL); blk = &RU_BLK(XLIF_XRDP1); break;
    case bdmf_address_channel_xlif_tx_if_urun_port_enable: reg = &RU_REG(XLIF_XRDP1, CHANNEL_XLIF_TX_IF_URUN_PORT_ENABLE); blk = &RU_BLK(XLIF_XRDP1); break;
    case bdmf_address_channel_xlif_tx_if_tx_threshold: reg = &RU_REG(XLIF_XRDP1, CHANNEL_XLIF_TX_IF_TX_THRESHOLD); blk = &RU_BLK(XLIF_XRDP1); break;
    case bdmf_address_channel_xlif_tx_if_tdm_mode: reg = &RU_REG(XLIF_XRDP1, CHANNEL_XLIF_TX_IF_TDM_MODE); blk = &RU_BLK(XLIF_XRDP1); break;
    case bdmf_address_channel_xlif_tx_flow_control_cosmap_en_stat: reg = &RU_REG(XLIF_XRDP1, CHANNEL_XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT); blk = &RU_BLK(XLIF_XRDP1); break;
    case bdmf_address_channel_xlif_tx_flow_control_cosmap_stat: reg = &RU_REG(XLIF_XRDP1, CHANNEL_XLIF_TX_FLOW_CONTROL_COSMAP_STAT); blk = &RU_BLK(XLIF_XRDP1); break;
    case bdmf_address_channel_debug_bus_sel: reg = &RU_REG(XLIF_XRDP1, CHANNEL_DEBUG_BUS_SEL); blk = &RU_BLK(XLIF_XRDP1); break;
    case bdmf_address_channel_xlif_eee_ind: reg = &RU_REG(XLIF_XRDP1, CHANNEL_XLIF_EEE_IND); blk = &RU_BLK(XLIF_XRDP1); break;
    case bdmf_address_channel_q_off_ind: reg = &RU_REG(XLIF_XRDP1, CHANNEL_Q_OFF_IND); blk = &RU_BLK(XLIF_XRDP1); break;
    case bdmf_address_channel_lane_to_port_map_port_map: reg = &RU_REG(XLIF_XRDP1, CHANNEL_LANE_TO_PORT_MAP_PORT_MAP); blk = &RU_BLK(XLIF_XRDP1); break;
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

bdmfmon_handle_t ag_drv_xlif_xrdp1_cli_init(bdmfmon_handle_t root_dir)
{
    bdmfmon_handle_t dir;

    dir = bdmfmon_dir_add(root_dir, "xlif_xrdp1", "xlif_xrdp1", BDMF_ACCESS_ADMIN, NULL);
    BUG_ON(dir == NULL);

    {
        static bdmfmon_cmd_parm_t set_channel_xlif_rx_if_if_dis[] = {
            BDMFMON_MAKE_PARM("channel_id", "channel_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("disable", "disable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_channel_xlif_rx_flow_control_cosmap_en[] = {
            BDMFMON_MAKE_PARM("channel_id", "channel_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pfc_en", "pfc_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("llfc_en", "llfc_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_channel_xlif_rx_flow_control_cosmap[] = {
            BDMFMON_MAKE_PARM("channel_id", "channel_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("value", "value", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_channel_xlif_tx_if_if_enable[] = {
            BDMFMON_MAKE_PARM("channel_id", "channel_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("disable_with_credits", "disable_with_credits", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("disable_wo_credits", "disable_wo_credits", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_channel_xlif_tx_if_set_credits[] = {
            BDMFMON_MAKE_PARM("channel_id", "channel_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("value", "value", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("en", "en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_channel_xlif_tx_if_out_ctrl[] = {
            BDMFMON_MAKE_PARM("channel_id", "channel_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mac_txerr", "mac_txerr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mac_txcrcerr", "mac_txcrcerr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mac_txosts_sinext", "mac_txosts_sinext", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mac_txcrcmode", "mac_txcrcmode", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_channel_xlif_tx_if_urun_port_enable[] = {
            BDMFMON_MAKE_PARM("channel_id", "channel_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enable", "enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_channel_xlif_tx_if_tx_threshold[] = {
            BDMFMON_MAKE_PARM("channel_id", "channel_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("value", "value", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_channel_xlif_tx_if_tdm_mode[] = {
            BDMFMON_MAKE_PARM("channel_id", "channel_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("value", "value", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_channel_debug_bus_sel[] = {
            BDMFMON_MAKE_PARM("channel_id", "channel_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("select_module", "select_module", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("select_lane", "select_lane", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_channel_lane_to_port_map_port_map[] = {
            BDMFMON_MAKE_PARM("channel_id", "channel_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("lane0", "lane0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("lane1", "lane1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("lane2", "lane2", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("lane3", "lane3", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "channel_xlif_rx_if_if_dis", .val = cli_xlif_xrdp1_channel_xlif_rx_if_if_dis, .parms = set_channel_xlif_rx_if_if_dis },
            { .name = "channel_xlif_rx_flow_control_cosmap_en", .val = cli_xlif_xrdp1_channel_xlif_rx_flow_control_cosmap_en, .parms = set_channel_xlif_rx_flow_control_cosmap_en },
            { .name = "channel_xlif_rx_flow_control_cosmap", .val = cli_xlif_xrdp1_channel_xlif_rx_flow_control_cosmap, .parms = set_channel_xlif_rx_flow_control_cosmap },
            { .name = "channel_xlif_tx_if_if_enable", .val = cli_xlif_xrdp1_channel_xlif_tx_if_if_enable, .parms = set_channel_xlif_tx_if_if_enable },
            { .name = "channel_xlif_tx_if_set_credits", .val = cli_xlif_xrdp1_channel_xlif_tx_if_set_credits, .parms = set_channel_xlif_tx_if_set_credits },
            { .name = "channel_xlif_tx_if_out_ctrl", .val = cli_xlif_xrdp1_channel_xlif_tx_if_out_ctrl, .parms = set_channel_xlif_tx_if_out_ctrl },
            { .name = "channel_xlif_tx_if_urun_port_enable", .val = cli_xlif_xrdp1_channel_xlif_tx_if_urun_port_enable, .parms = set_channel_xlif_tx_if_urun_port_enable },
            { .name = "channel_xlif_tx_if_tx_threshold", .val = cli_xlif_xrdp1_channel_xlif_tx_if_tx_threshold, .parms = set_channel_xlif_tx_if_tx_threshold },
            { .name = "channel_xlif_tx_if_tdm_mode", .val = cli_xlif_xrdp1_channel_xlif_tx_if_tdm_mode, .parms = set_channel_xlif_tx_if_tdm_mode },
            { .name = "channel_debug_bus_sel", .val = cli_xlif_xrdp1_channel_debug_bus_sel, .parms = set_channel_debug_bus_sel },
            { .name = "channel_lane_to_port_map_port_map", .val = cli_xlif_xrdp1_channel_lane_to_port_map_port_map, .parms = set_channel_lane_to_port_map_port_map },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", ag_drv_xlif_xrdp1_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t get_default[] = {
            BDMFMON_MAKE_PARM("channel_id", "channel_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "channel_xlif_rx_if_if_dis", .val = cli_xlif_xrdp1_channel_xlif_rx_if_if_dis, .parms = get_default },
            { .name = "channel_xlif_rx_if_oflw_flag", .val = cli_xlif_xrdp1_channel_xlif_rx_if_oflw_flag, .parms = get_default },
            { .name = "channel_xlif_rx_if_err_flag", .val = cli_xlif_xrdp1_channel_xlif_rx_if_err_flag, .parms = get_default },
            { .name = "channel_xlif_rx_flow_control_cosmap_en", .val = cli_xlif_xrdp1_channel_xlif_rx_flow_control_cosmap_en, .parms = get_default },
            { .name = "channel_xlif_rx_flow_control_cosmap", .val = cli_xlif_xrdp1_channel_xlif_rx_flow_control_cosmap, .parms = get_default },
            { .name = "channel_xlif_tx_if_if_enable", .val = cli_xlif_xrdp1_channel_xlif_tx_if_if_enable, .parms = get_default },
            { .name = "channel_xlif_tx_if_read_credits", .val = cli_xlif_xrdp1_channel_xlif_tx_if_read_credits, .parms = get_default },
            { .name = "channel_xlif_tx_if_set_credits", .val = cli_xlif_xrdp1_channel_xlif_tx_if_set_credits, .parms = get_default },
            { .name = "channel_xlif_tx_if_out_ctrl", .val = cli_xlif_xrdp1_channel_xlif_tx_if_out_ctrl, .parms = get_default },
            { .name = "channel_xlif_tx_if_urun_port_enable", .val = cli_xlif_xrdp1_channel_xlif_tx_if_urun_port_enable, .parms = get_default },
            { .name = "channel_xlif_tx_if_tx_threshold", .val = cli_xlif_xrdp1_channel_xlif_tx_if_tx_threshold, .parms = get_default },
            { .name = "channel_xlif_tx_if_tdm_mode", .val = cli_xlif_xrdp1_channel_xlif_tx_if_tdm_mode, .parms = get_default },
            { .name = "channel_xlif_tx_flow_control_cosmap_en_stat", .val = cli_xlif_xrdp1_channel_xlif_tx_flow_control_cosmap_en_stat, .parms = get_default },
            { .name = "channel_xlif_tx_flow_control_cosmap_stat", .val = cli_xlif_xrdp1_channel_xlif_tx_flow_control_cosmap_stat, .parms = get_default },
            { .name = "channel_debug_bus_sel", .val = cli_xlif_xrdp1_channel_debug_bus_sel, .parms = get_default },
            { .name = "channel_xlif_eee_ind", .val = cli_xlif_xrdp1_channel_xlif_eee_ind, .parms = get_default },
            { .name = "channel_q_off_ind", .val = cli_xlif_xrdp1_channel_q_off_ind, .parms = get_default },
            { .name = "channel_lane_to_port_map_port_map", .val = cli_xlif_xrdp1_channel_lane_to_port_map_port_map, .parms = get_default },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_xlif_xrdp1_cli_get,
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
        BDMFMON_MAKE_PARM(dir, "test", "test", ag_drv_xlif_xrdp1_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0)            BDMFMON_MAKE_PARM("channel_id", "channel_id", BDMFMON_PARM_UNUMBER, 0));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name = "CHANNEL_XLIF_RX_IF_IF_DIS", .val = bdmf_address_channel_xlif_rx_if_if_dis },
            { .name = "CHANNEL_XLIF_RX_IF_OFLW_FLAG", .val = bdmf_address_channel_xlif_rx_if_oflw_flag },
            { .name = "CHANNEL_XLIF_RX_IF_ERR_FLAG", .val = bdmf_address_channel_xlif_rx_if_err_flag },
            { .name = "CHANNEL_XLIF_RX_FLOW_CONTROL_COSMAP_EN", .val = bdmf_address_channel_xlif_rx_flow_control_cosmap_en },
            { .name = "CHANNEL_XLIF_RX_FLOW_CONTROL_COSMAP", .val = bdmf_address_channel_xlif_rx_flow_control_cosmap },
            { .name = "CHANNEL_XLIF_TX_IF_IF_ENABLE", .val = bdmf_address_channel_xlif_tx_if_if_enable },
            { .name = "CHANNEL_XLIF_TX_IF_READ_CREDITS", .val = bdmf_address_channel_xlif_tx_if_read_credits },
            { .name = "CHANNEL_XLIF_TX_IF_SET_CREDITS", .val = bdmf_address_channel_xlif_tx_if_set_credits },
            { .name = "CHANNEL_XLIF_TX_IF_OUT_CTRL", .val = bdmf_address_channel_xlif_tx_if_out_ctrl },
            { .name = "CHANNEL_XLIF_TX_IF_URUN_PORT_ENABLE", .val = bdmf_address_channel_xlif_tx_if_urun_port_enable },
            { .name = "CHANNEL_XLIF_TX_IF_TX_THRESHOLD", .val = bdmf_address_channel_xlif_tx_if_tx_threshold },
            { .name = "CHANNEL_XLIF_TX_IF_TDM_MODE", .val = bdmf_address_channel_xlif_tx_if_tdm_mode },
            { .name = "CHANNEL_XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT", .val = bdmf_address_channel_xlif_tx_flow_control_cosmap_en_stat },
            { .name = "CHANNEL_XLIF_TX_FLOW_CONTROL_COSMAP_STAT", .val = bdmf_address_channel_xlif_tx_flow_control_cosmap_stat },
            { .name = "CHANNEL_DEBUG_BUS_SEL", .val = bdmf_address_channel_debug_bus_sel },
            { .name = "CHANNEL_XLIF_EEE_IND", .val = bdmf_address_channel_xlif_eee_ind },
            { .name = "CHANNEL_Q_OFF_IND", .val = bdmf_address_channel_q_off_ind },
            { .name = "CHANNEL_LANE_TO_PORT_MAP_PORT_MAP", .val = bdmf_address_channel_lane_to_port_map_port_map },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "address", "address", bcm_xlif_xrdp1_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index1", "channel_id", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

    return dir;
}
#endif /* #ifdef USE_BDMF_SHELL */
