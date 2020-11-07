/*
   Copyright (c) 2015 Broadcom Corporation
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

#include "drivers_common_ag.h"
#include "wan_epon_ag.h"
bdmf_error_t ag_drv_wan_epon_wan_epon_10g_gearbox_set(const wan_epon_wan_epon_10g_gearbox *wan_epon_10g_gearbox)
{
    uint32_t reg_10g_gearbox=0;

#ifdef VALIDATE_PARMS
    if(!wan_epon_10g_gearbox)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((wan_epon_10g_gearbox->cfg_sgb_pon_10g_epon_rx_cgen_rstn >= _1BITS_MAX_VAL_) ||
       (wan_epon_10g_gearbox->cfg_sgb_pon_10g_epon_tx_cgen_rstn >= _1BITS_MAX_VAL_) ||
       (wan_epon_10g_gearbox->cfg_sgb_pon_10g_epon_rx_gbox_rstn >= _1BITS_MAX_VAL_) ||
       (wan_epon_10g_gearbox->cfg_sgb_pon_10g_epon_tx_gbox_rstn >= _1BITS_MAX_VAL_) ||
       (wan_epon_10g_gearbox->cfg_sgb_pon_10g_epon_clk_en >= _1BITS_MAX_VAL_) ||
       (wan_epon_10g_gearbox->cfg_sgb_pon_10g_epon_rx_data_end >= _1BITS_MAX_VAL_) ||
       (wan_epon_10g_gearbox->cfg_sgb_pon_10g_epon_tx2rx_loop_en >= _1BITS_MAX_VAL_) ||
       (wan_epon_10g_gearbox->cfg_sgb_pon_10g_epon_tx_fifo_off_ld >= _1BITS_MAX_VAL_) ||
       (wan_epon_10g_gearbox->cfg_sgb_pon_10g_epon_tx_fifo_off >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_10g_gearbox = RU_FIELD_SET(0, WAN_EPON, 10G_GEARBOX, CFG_SGB_PON_10G_EPON_RX_CGEN_RSTN, reg_10g_gearbox, wan_epon_10g_gearbox->cfg_sgb_pon_10g_epon_rx_cgen_rstn);
    reg_10g_gearbox = RU_FIELD_SET(0, WAN_EPON, 10G_GEARBOX, CFG_SGB_PON_10G_EPON_TX_CGEN_RSTN, reg_10g_gearbox, wan_epon_10g_gearbox->cfg_sgb_pon_10g_epon_tx_cgen_rstn);
    reg_10g_gearbox = RU_FIELD_SET(0, WAN_EPON, 10G_GEARBOX, CFG_SGB_PON_10G_EPON_RX_GBOX_RSTN, reg_10g_gearbox, wan_epon_10g_gearbox->cfg_sgb_pon_10g_epon_rx_gbox_rstn);
    reg_10g_gearbox = RU_FIELD_SET(0, WAN_EPON, 10G_GEARBOX, CFG_SGB_PON_10G_EPON_TX_GBOX_RSTN, reg_10g_gearbox, wan_epon_10g_gearbox->cfg_sgb_pon_10g_epon_tx_gbox_rstn);
    reg_10g_gearbox = RU_FIELD_SET(0, WAN_EPON, 10G_GEARBOX, CFG_SGB_PON_10G_EPON_CLK_EN, reg_10g_gearbox, wan_epon_10g_gearbox->cfg_sgb_pon_10g_epon_clk_en);
    reg_10g_gearbox = RU_FIELD_SET(0, WAN_EPON, 10G_GEARBOX, CFG_SGB_PON_10G_EPON_RX_DATA_END, reg_10g_gearbox, wan_epon_10g_gearbox->cfg_sgb_pon_10g_epon_rx_data_end);
    reg_10g_gearbox = RU_FIELD_SET(0, WAN_EPON, 10G_GEARBOX, CFG_SGB_PON_10G_EPON_TX2RX_LOOP_EN, reg_10g_gearbox, wan_epon_10g_gearbox->cfg_sgb_pon_10g_epon_tx2rx_loop_en);
    reg_10g_gearbox = RU_FIELD_SET(0, WAN_EPON, 10G_GEARBOX, CFG_SGB_PON_10G_EPON_TX_FIFO_OFF_LD, reg_10g_gearbox, wan_epon_10g_gearbox->cfg_sgb_pon_10g_epon_tx_fifo_off_ld);
    reg_10g_gearbox = RU_FIELD_SET(0, WAN_EPON, 10G_GEARBOX, CFG_SGB_PON_10G_EPON_TX_FIFO_OFF, reg_10g_gearbox, wan_epon_10g_gearbox->cfg_sgb_pon_10g_epon_tx_fifo_off);

    RU_REG_WRITE(0, WAN_EPON, 10G_GEARBOX, reg_10g_gearbox);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_wan_epon_wan_epon_10g_gearbox_get(wan_epon_wan_epon_10g_gearbox *wan_epon_10g_gearbox)
{
    uint32_t reg_10g_gearbox=0;

#ifdef VALIDATE_PARMS
    if(!wan_epon_10g_gearbox)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, WAN_EPON, 10G_GEARBOX, reg_10g_gearbox);

    wan_epon_10g_gearbox->cfg_sgb_pon_10g_epon_rx_cgen_rstn = RU_FIELD_GET(0, WAN_EPON, 10G_GEARBOX, CFG_SGB_PON_10G_EPON_RX_CGEN_RSTN, reg_10g_gearbox);
    wan_epon_10g_gearbox->cfg_sgb_pon_10g_epon_tx_cgen_rstn = RU_FIELD_GET(0, WAN_EPON, 10G_GEARBOX, CFG_SGB_PON_10G_EPON_TX_CGEN_RSTN, reg_10g_gearbox);
    wan_epon_10g_gearbox->cfg_sgb_pon_10g_epon_rx_gbox_rstn = RU_FIELD_GET(0, WAN_EPON, 10G_GEARBOX, CFG_SGB_PON_10G_EPON_RX_GBOX_RSTN, reg_10g_gearbox);
    wan_epon_10g_gearbox->cfg_sgb_pon_10g_epon_tx_gbox_rstn = RU_FIELD_GET(0, WAN_EPON, 10G_GEARBOX, CFG_SGB_PON_10G_EPON_TX_GBOX_RSTN, reg_10g_gearbox);
    wan_epon_10g_gearbox->cfg_sgb_pon_10g_epon_clk_en = RU_FIELD_GET(0, WAN_EPON, 10G_GEARBOX, CFG_SGB_PON_10G_EPON_CLK_EN, reg_10g_gearbox);
    wan_epon_10g_gearbox->cfg_sgb_pon_10g_epon_rx_data_end = RU_FIELD_GET(0, WAN_EPON, 10G_GEARBOX, CFG_SGB_PON_10G_EPON_RX_DATA_END, reg_10g_gearbox);
    wan_epon_10g_gearbox->cfg_sgb_pon_10g_epon_tx2rx_loop_en = RU_FIELD_GET(0, WAN_EPON, 10G_GEARBOX, CFG_SGB_PON_10G_EPON_TX2RX_LOOP_EN, reg_10g_gearbox);
    wan_epon_10g_gearbox->cfg_sgb_pon_10g_epon_tx_fifo_off_ld = RU_FIELD_GET(0, WAN_EPON, 10G_GEARBOX, CFG_SGB_PON_10G_EPON_TX_FIFO_OFF_LD, reg_10g_gearbox);
    wan_epon_10g_gearbox->cfg_sgb_pon_10g_epon_tx_fifo_off = RU_FIELD_GET(0, WAN_EPON, 10G_GEARBOX, CFG_SGB_PON_10G_EPON_TX_FIFO_OFF, reg_10g_gearbox);

    return BDMF_ERR_OK;
}

