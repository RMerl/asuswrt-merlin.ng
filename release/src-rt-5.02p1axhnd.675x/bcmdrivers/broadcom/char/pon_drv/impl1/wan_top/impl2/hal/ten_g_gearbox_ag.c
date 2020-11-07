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
#include "ten_g_gearbox_ag.h"
int ag_drv_ten_g_gearbox_gearbox_set(const ten_g_gearbox_gearbox *gearbox)
{
    uint32_t reg_gearbox=0;

#ifdef VALIDATE_PARMS
    if(!gearbox)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((gearbox->cfg_sgb_pon_10g_epon_tx_fifo_off >= _3BITS_MAX_VAL_) ||
       (gearbox->cfg_sgb_pon_10g_epon_tx_fifo_off_ld >= _1BITS_MAX_VAL_) ||
       (gearbox->cfg_sgb_pon_10g_epon_tx2rx_loop_en >= _1BITS_MAX_VAL_) ||
       (gearbox->cfg_sgb_pon_10g_epon_rx_data_end >= _1BITS_MAX_VAL_) ||
       (gearbox->cfg_sgb_pon_10g_epon_clk_en >= _1BITS_MAX_VAL_) ||
       (gearbox->cfg_sgb_pon_10g_epon_tx_gbox_rstn >= _1BITS_MAX_VAL_) ||
       (gearbox->cfg_sgb_pon_10g_epon_rx_gbox_rstn >= _1BITS_MAX_VAL_) ||
       (gearbox->cfg_sgb_pon_10g_epon_tx_cgen_rstn >= _1BITS_MAX_VAL_) ||
       (gearbox->cfg_sgb_pon_10g_epon_rx_cgen_rstn >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gearbox = RU_FIELD_SET(0, TEN_G_GEARBOX, GEARBOX, CFG_SGB_PON_10G_EPON_TX_FIFO_OFF, reg_gearbox, gearbox->cfg_sgb_pon_10g_epon_tx_fifo_off);
    reg_gearbox = RU_FIELD_SET(0, TEN_G_GEARBOX, GEARBOX, CFG_SGB_PON_10G_EPON_TX_FIFO_OFF_LD, reg_gearbox, gearbox->cfg_sgb_pon_10g_epon_tx_fifo_off_ld);
    reg_gearbox = RU_FIELD_SET(0, TEN_G_GEARBOX, GEARBOX, CFG_SGB_PON_10G_EPON_TX2RX_LOOP_EN, reg_gearbox, gearbox->cfg_sgb_pon_10g_epon_tx2rx_loop_en);
    reg_gearbox = RU_FIELD_SET(0, TEN_G_GEARBOX, GEARBOX, CFG_SGB_PON_10G_EPON_RX_DATA_END, reg_gearbox, gearbox->cfg_sgb_pon_10g_epon_rx_data_end);
    reg_gearbox = RU_FIELD_SET(0, TEN_G_GEARBOX, GEARBOX, CFG_SGB_PON_10G_EPON_CLK_EN, reg_gearbox, gearbox->cfg_sgb_pon_10g_epon_clk_en);
    reg_gearbox = RU_FIELD_SET(0, TEN_G_GEARBOX, GEARBOX, CFG_SGB_PON_10G_EPON_TX_GBOX_RSTN, reg_gearbox, gearbox->cfg_sgb_pon_10g_epon_tx_gbox_rstn);
    reg_gearbox = RU_FIELD_SET(0, TEN_G_GEARBOX, GEARBOX, CFG_SGB_PON_10G_EPON_RX_GBOX_RSTN, reg_gearbox, gearbox->cfg_sgb_pon_10g_epon_rx_gbox_rstn);
    reg_gearbox = RU_FIELD_SET(0, TEN_G_GEARBOX, GEARBOX, CFG_SGB_PON_10G_EPON_TX_CGEN_RSTN, reg_gearbox, gearbox->cfg_sgb_pon_10g_epon_tx_cgen_rstn);
    reg_gearbox = RU_FIELD_SET(0, TEN_G_GEARBOX, GEARBOX, CFG_SGB_PON_10G_EPON_RX_CGEN_RSTN, reg_gearbox, gearbox->cfg_sgb_pon_10g_epon_rx_cgen_rstn);

    RU_REG_WRITE(0, TEN_G_GEARBOX, GEARBOX, reg_gearbox);

    return 0;
}

int ag_drv_ten_g_gearbox_gearbox_get(ten_g_gearbox_gearbox *gearbox)
{
    uint32_t reg_gearbox=0;

#ifdef VALIDATE_PARMS
    if(!gearbox)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, TEN_G_GEARBOX, GEARBOX, reg_gearbox);

    gearbox->cfg_sgb_pon_10g_epon_tx_fifo_off = RU_FIELD_GET(0, TEN_G_GEARBOX, GEARBOX, CFG_SGB_PON_10G_EPON_TX_FIFO_OFF, reg_gearbox);
    gearbox->cfg_sgb_pon_10g_epon_tx_fifo_off_ld = RU_FIELD_GET(0, TEN_G_GEARBOX, GEARBOX, CFG_SGB_PON_10G_EPON_TX_FIFO_OFF_LD, reg_gearbox);
    gearbox->cfg_sgb_pon_10g_epon_tx2rx_loop_en = RU_FIELD_GET(0, TEN_G_GEARBOX, GEARBOX, CFG_SGB_PON_10G_EPON_TX2RX_LOOP_EN, reg_gearbox);
    gearbox->cfg_sgb_pon_10g_epon_rx_data_end = RU_FIELD_GET(0, TEN_G_GEARBOX, GEARBOX, CFG_SGB_PON_10G_EPON_RX_DATA_END, reg_gearbox);
    gearbox->cfg_sgb_pon_10g_epon_clk_en = RU_FIELD_GET(0, TEN_G_GEARBOX, GEARBOX, CFG_SGB_PON_10G_EPON_CLK_EN, reg_gearbox);
    gearbox->cfg_sgb_pon_10g_epon_tx_gbox_rstn = RU_FIELD_GET(0, TEN_G_GEARBOX, GEARBOX, CFG_SGB_PON_10G_EPON_TX_GBOX_RSTN, reg_gearbox);
    gearbox->cfg_sgb_pon_10g_epon_rx_gbox_rstn = RU_FIELD_GET(0, TEN_G_GEARBOX, GEARBOX, CFG_SGB_PON_10G_EPON_RX_GBOX_RSTN, reg_gearbox);
    gearbox->cfg_sgb_pon_10g_epon_tx_cgen_rstn = RU_FIELD_GET(0, TEN_G_GEARBOX, GEARBOX, CFG_SGB_PON_10G_EPON_TX_CGEN_RSTN, reg_gearbox);
    gearbox->cfg_sgb_pon_10g_epon_rx_cgen_rstn = RU_FIELD_GET(0, TEN_G_GEARBOX, GEARBOX, CFG_SGB_PON_10G_EPON_RX_CGEN_RSTN, reg_gearbox);

    return 0;
}

