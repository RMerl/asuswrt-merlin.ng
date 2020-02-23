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

#include "bcm63158_drivers_xport_ag.h"
#include "bcm63158_xport_xlmac_core_ag.h"
#include "xport_xlmac_indirect_access.h"
#define BLOCK_ADDR_COUNT_BITS 2
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

int ag_drv_xport_xlmac_core_ctrl_set(uint8_t port_id, const xport_xlmac_core_ctrl *ctrl)
{
    uint64_t reg_ctrl=0;

#ifdef VALIDATE_PARMS
    if(!ctrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (ctrl->extended_hig2_en >= _1BITS_MAX_VAL_) ||
       (ctrl->link_status_select >= _1BITS_MAX_VAL_) ||
       (ctrl->sw_link_status >= _1BITS_MAX_VAL_) ||
       (ctrl->xgmii_ipg_check_disable >= _1BITS_MAX_VAL_) ||
       (ctrl->rs_soft_reset >= _1BITS_MAX_VAL_) ||
       (ctrl->rsvd_5 >= _1BITS_MAX_VAL_) ||
       (ctrl->local_lpbk_leak_enb >= _1BITS_MAX_VAL_) ||
       (ctrl->rsvd_4 >= _1BITS_MAX_VAL_) ||
       (ctrl->soft_reset >= _1BITS_MAX_VAL_) ||
       (ctrl->lag_failover_en >= _1BITS_MAX_VAL_) ||
       (ctrl->remove_failover_lpbk >= _1BITS_MAX_VAL_) ||
       (ctrl->rsvd_1 >= _1BITS_MAX_VAL_) ||
       (ctrl->local_lpbk >= _1BITS_MAX_VAL_) ||
       (ctrl->rx_en >= _1BITS_MAX_VAL_) ||
       (ctrl->tx_en >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, CTRL, EXTENDED_HIG2_EN, reg_ctrl, (uint64_t)ctrl->extended_hig2_en);
    reg_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, CTRL, LINK_STATUS_SELECT, reg_ctrl, (uint64_t)ctrl->link_status_select);
    reg_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, CTRL, SW_LINK_STATUS, reg_ctrl, (uint64_t)ctrl->sw_link_status);
    reg_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, CTRL, XGMII_IPG_CHECK_DISABLE, reg_ctrl, (uint64_t)ctrl->xgmii_ipg_check_disable);
    reg_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, CTRL, RS_SOFT_RESET, reg_ctrl, (uint64_t)ctrl->rs_soft_reset);
    reg_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, CTRL, RSVD_5, reg_ctrl, (uint64_t)ctrl->rsvd_5);
    reg_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, CTRL, LOCAL_LPBK_LEAK_ENB, reg_ctrl, (uint64_t)ctrl->local_lpbk_leak_enb);
    reg_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, CTRL, RSVD_4, reg_ctrl, (uint64_t)ctrl->rsvd_4);
    reg_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, CTRL, SOFT_RESET, reg_ctrl, (uint64_t)ctrl->soft_reset);
    reg_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, CTRL, LAG_FAILOVER_EN, reg_ctrl, (uint64_t)ctrl->lag_failover_en);
    reg_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, CTRL, REMOVE_FAILOVER_LPBK, reg_ctrl, (uint64_t)ctrl->remove_failover_lpbk);
    reg_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, CTRL, RSVD_1, reg_ctrl, (uint64_t)ctrl->rsvd_1);
    reg_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, CTRL, LOCAL_LPBK, reg_ctrl, (uint64_t)ctrl->local_lpbk);
    reg_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, CTRL, RX_EN, reg_ctrl, (uint64_t)ctrl->rx_en);
    reg_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, CTRL, TX_EN, reg_ctrl, (uint64_t)ctrl->tx_en);

    XPORT_XLMAC_INDIRECT_WRITE(port_id, XPORT_XLMAC_CORE, CTRL, reg_ctrl);

    return 0;
}

int ag_drv_xport_xlmac_core_ctrl_get(uint8_t port_id, xport_xlmac_core_ctrl *ctrl)
{
    uint64_t reg_ctrl=0;

#ifdef VALIDATE_PARMS
    if(!ctrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, CTRL, reg_ctrl);

    ctrl->extended_hig2_en = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, CTRL, EXTENDED_HIG2_EN, reg_ctrl);
    ctrl->link_status_select = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, CTRL, LINK_STATUS_SELECT, reg_ctrl);
    ctrl->sw_link_status = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, CTRL, SW_LINK_STATUS, reg_ctrl);
    ctrl->xgmii_ipg_check_disable = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, CTRL, XGMII_IPG_CHECK_DISABLE, reg_ctrl);
    ctrl->rs_soft_reset = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, CTRL, RS_SOFT_RESET, reg_ctrl);
    ctrl->rsvd_5 = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, CTRL, RSVD_5, reg_ctrl);
    ctrl->local_lpbk_leak_enb = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, CTRL, LOCAL_LPBK_LEAK_ENB, reg_ctrl);
    ctrl->rsvd_4 = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, CTRL, RSVD_4, reg_ctrl);
    ctrl->soft_reset = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, CTRL, SOFT_RESET, reg_ctrl);
    ctrl->lag_failover_en = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, CTRL, LAG_FAILOVER_EN, reg_ctrl);
    ctrl->remove_failover_lpbk = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, CTRL, REMOVE_FAILOVER_LPBK, reg_ctrl);
    ctrl->rsvd_1 = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, CTRL, RSVD_1, reg_ctrl);
    ctrl->local_lpbk = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, CTRL, LOCAL_LPBK, reg_ctrl);
    ctrl->rx_en = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, CTRL, RX_EN, reg_ctrl);
    ctrl->tx_en = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, CTRL, TX_EN, reg_ctrl);

    return 0;
}

int ag_drv_xport_xlmac_core_mode_set(uint8_t port_id, uint8_t speed_mode, uint8_t no_sop_for_crc_hg, uint8_t hdr_mode)
{
    uint64_t reg_mode=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (speed_mode >= _3BITS_MAX_VAL_) ||
       (no_sop_for_crc_hg >= _1BITS_MAX_VAL_) ||
       (hdr_mode >= _3BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_mode = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, MODE, SPEED_MODE, reg_mode, (uint64_t)speed_mode);
    reg_mode = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, MODE, NO_SOP_FOR_CRC_HG, reg_mode, (uint64_t)no_sop_for_crc_hg);
    reg_mode = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, MODE, HDR_MODE, reg_mode, (uint64_t)hdr_mode);

    XPORT_XLMAC_INDIRECT_WRITE(port_id, XPORT_XLMAC_CORE, MODE, reg_mode);

    return 0;
}

int ag_drv_xport_xlmac_core_mode_get(uint8_t port_id, uint8_t *speed_mode, uint8_t *no_sop_for_crc_hg, uint8_t *hdr_mode)
{
    uint64_t reg_mode=0;

#ifdef VALIDATE_PARMS
    if(!speed_mode || !no_sop_for_crc_hg || !hdr_mode)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, MODE, reg_mode);

    *speed_mode = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, MODE, SPEED_MODE, reg_mode);
    *no_sop_for_crc_hg = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, MODE, NO_SOP_FOR_CRC_HG, reg_mode);
    *hdr_mode = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, MODE, HDR_MODE, reg_mode);

    return 0;
}

int ag_drv_xport_xlmac_core_spare0_set(uint8_t port_id, uint32_t rsvd)
{
    uint64_t reg_spare0=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_spare0 = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, SPARE0, RSVD, reg_spare0, (uint64_t)rsvd);

    XPORT_XLMAC_INDIRECT_WRITE(port_id, XPORT_XLMAC_CORE, SPARE0, reg_spare0);

    return 0;
}

int ag_drv_xport_xlmac_core_spare0_get(uint8_t port_id, uint32_t *rsvd)
{
    uint64_t reg_spare0=0;

#ifdef VALIDATE_PARMS
    if(!rsvd)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, SPARE0, reg_spare0);

    *rsvd = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, SPARE0, RSVD, reg_spare0);

    return 0;
}

int ag_drv_xport_xlmac_core_spare1_set(uint8_t port_id, uint8_t rsvd)
{
    uint64_t reg_spare1=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (rsvd >= _2BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_spare1 = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, SPARE1, RSVD, reg_spare1, (uint64_t)rsvd);

    XPORT_XLMAC_INDIRECT_WRITE(port_id, XPORT_XLMAC_CORE, SPARE1, reg_spare1);

    return 0;
}

int ag_drv_xport_xlmac_core_spare1_get(uint8_t port_id, uint8_t *rsvd)
{
    uint64_t reg_spare1=0;

#ifdef VALIDATE_PARMS
    if(!rsvd)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, SPARE1, reg_spare1);

    *rsvd = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, SPARE1, RSVD, reg_spare1);

    return 0;
}

int ag_drv_xport_xlmac_core_tx_ctrl_set(uint8_t port_id, const xport_xlmac_core_tx_ctrl *tx_ctrl)
{
    uint64_t reg_tx_ctrl=0;

#ifdef VALIDATE_PARMS
    if(!tx_ctrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (tx_ctrl->tx_threshold >= _4BITS_MAX_VAL_) ||
       (tx_ctrl->ep_discard >= _1BITS_MAX_VAL_) ||
       (tx_ctrl->tx_preamble_length >= _4BITS_MAX_VAL_) ||
       (tx_ctrl->throt_num >= _6BITS_MAX_VAL_) ||
       (tx_ctrl->average_ipg >= _7BITS_MAX_VAL_) ||
       (tx_ctrl->pad_threshold >= _7BITS_MAX_VAL_) ||
       (tx_ctrl->pad_en >= _1BITS_MAX_VAL_) ||
       (tx_ctrl->tx_any_start >= _1BITS_MAX_VAL_) ||
       (tx_ctrl->discard >= _1BITS_MAX_VAL_) ||
       (tx_ctrl->crc_mode >= _2BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_tx_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, TX_CTRL, TX_THRESHOLD, reg_tx_ctrl, (uint64_t)tx_ctrl->tx_threshold);
    reg_tx_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, TX_CTRL, EP_DISCARD, reg_tx_ctrl, (uint64_t)tx_ctrl->ep_discard);
    reg_tx_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, TX_CTRL, TX_PREAMBLE_LENGTH, reg_tx_ctrl, (uint64_t)tx_ctrl->tx_preamble_length);
    reg_tx_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, TX_CTRL, THROT_DENOM, reg_tx_ctrl, (uint64_t)tx_ctrl->throt_denom);
    reg_tx_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, TX_CTRL, THROT_NUM, reg_tx_ctrl, (uint64_t)tx_ctrl->throt_num);
    reg_tx_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, TX_CTRL, AVERAGE_IPG, reg_tx_ctrl, (uint64_t)tx_ctrl->average_ipg);
    reg_tx_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, TX_CTRL, PAD_THRESHOLD, reg_tx_ctrl, (uint64_t)tx_ctrl->pad_threshold);
    reg_tx_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, TX_CTRL, PAD_EN, reg_tx_ctrl, (uint64_t)tx_ctrl->pad_en);
    reg_tx_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, TX_CTRL, TX_ANY_START, reg_tx_ctrl, (uint64_t)tx_ctrl->tx_any_start);
    reg_tx_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, TX_CTRL, DISCARD, reg_tx_ctrl, (uint64_t)tx_ctrl->discard);
    reg_tx_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, TX_CTRL, CRC_MODE, reg_tx_ctrl, (uint64_t)tx_ctrl->crc_mode);

    XPORT_XLMAC_INDIRECT_WRITE(port_id, XPORT_XLMAC_CORE, TX_CTRL, reg_tx_ctrl);

    return 0;
}

int ag_drv_xport_xlmac_core_tx_ctrl_get(uint8_t port_id, xport_xlmac_core_tx_ctrl *tx_ctrl)
{
    uint64_t reg_tx_ctrl=0;

#ifdef VALIDATE_PARMS
    if(!tx_ctrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, TX_CTRL, reg_tx_ctrl);

    tx_ctrl->tx_threshold = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, TX_CTRL, TX_THRESHOLD, reg_tx_ctrl);
    tx_ctrl->ep_discard = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, TX_CTRL, EP_DISCARD, reg_tx_ctrl);
    tx_ctrl->tx_preamble_length = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, TX_CTRL, TX_PREAMBLE_LENGTH, reg_tx_ctrl);
    tx_ctrl->throt_denom = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, TX_CTRL, THROT_DENOM, reg_tx_ctrl);
    tx_ctrl->throt_num = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, TX_CTRL, THROT_NUM, reg_tx_ctrl);
    tx_ctrl->average_ipg = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, TX_CTRL, AVERAGE_IPG, reg_tx_ctrl);
    tx_ctrl->pad_threshold = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, TX_CTRL, PAD_THRESHOLD, reg_tx_ctrl);
    tx_ctrl->pad_en = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, TX_CTRL, PAD_EN, reg_tx_ctrl);
    tx_ctrl->tx_any_start = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, TX_CTRL, TX_ANY_START, reg_tx_ctrl);
    tx_ctrl->discard = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, TX_CTRL, DISCARD, reg_tx_ctrl);
    tx_ctrl->crc_mode = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, TX_CTRL, CRC_MODE, reg_tx_ctrl);

    return 0;
}

int ag_drv_xport_xlmac_core_tx_mac_sa_set(uint8_t port_id, uint64_t ctrl_sa)
{
    uint64_t reg_tx_mac_sa=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (ctrl_sa >= _48BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_tx_mac_sa = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, TX_MAC_SA, CTRL_SA, reg_tx_mac_sa, (uint64_t)ctrl_sa);

    XPORT_XLMAC_INDIRECT_WRITE(port_id, XPORT_XLMAC_CORE, TX_MAC_SA, reg_tx_mac_sa);

    return 0;
}

int ag_drv_xport_xlmac_core_tx_mac_sa_get(uint8_t port_id, uint64_t *ctrl_sa)
{
    uint64_t reg_tx_mac_sa=0;

#ifdef VALIDATE_PARMS
    if(!ctrl_sa)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, TX_MAC_SA, reg_tx_mac_sa);

    *ctrl_sa = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, TX_MAC_SA, CTRL_SA, reg_tx_mac_sa);

    return 0;
}

int ag_drv_xport_xlmac_core_rx_ctrl_set(uint8_t port_id, const xport_xlmac_core_rx_ctrl *rx_ctrl)
{
    uint64_t reg_rx_ctrl=0;

#ifdef VALIDATE_PARMS
    if(!rx_ctrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (rx_ctrl->rx_pass_pfc >= _1BITS_MAX_VAL_) ||
       (rx_ctrl->rx_pass_pause >= _1BITS_MAX_VAL_) ||
       (rx_ctrl->rx_pass_ctrl >= _1BITS_MAX_VAL_) ||
       (rx_ctrl->rsvd_3 >= _1BITS_MAX_VAL_) ||
       (rx_ctrl->rsvd_2 >= _1BITS_MAX_VAL_) ||
       (rx_ctrl->runt_threshold >= _7BITS_MAX_VAL_) ||
       (rx_ctrl->strict_preamble >= _1BITS_MAX_VAL_) ||
       (rx_ctrl->strip_crc >= _1BITS_MAX_VAL_) ||
       (rx_ctrl->rx_any_start >= _1BITS_MAX_VAL_) ||
       (rx_ctrl->rsvd_1 >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_rx_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, RX_CTRL, RX_PASS_PFC, reg_rx_ctrl, (uint64_t)rx_ctrl->rx_pass_pfc);
    reg_rx_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, RX_CTRL, RX_PASS_PAUSE, reg_rx_ctrl, (uint64_t)rx_ctrl->rx_pass_pause);
    reg_rx_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, RX_CTRL, RX_PASS_CTRL, reg_rx_ctrl, (uint64_t)rx_ctrl->rx_pass_ctrl);
    reg_rx_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, RX_CTRL, RSVD_3, reg_rx_ctrl, (uint64_t)rx_ctrl->rsvd_3);
    reg_rx_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, RX_CTRL, RSVD_2, reg_rx_ctrl, (uint64_t)rx_ctrl->rsvd_2);
    reg_rx_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, RX_CTRL, RUNT_THRESHOLD, reg_rx_ctrl, (uint64_t)rx_ctrl->runt_threshold);
    reg_rx_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, RX_CTRL, STRICT_PREAMBLE, reg_rx_ctrl, (uint64_t)rx_ctrl->strict_preamble);
    reg_rx_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, RX_CTRL, STRIP_CRC, reg_rx_ctrl, (uint64_t)rx_ctrl->strip_crc);
    reg_rx_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, RX_CTRL, RX_ANY_START, reg_rx_ctrl, (uint64_t)rx_ctrl->rx_any_start);
    reg_rx_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, RX_CTRL, RSVD_1, reg_rx_ctrl, (uint64_t)rx_ctrl->rsvd_1);

    XPORT_XLMAC_INDIRECT_WRITE(port_id, XPORT_XLMAC_CORE, RX_CTRL, reg_rx_ctrl);

    return 0;
}

int ag_drv_xport_xlmac_core_rx_ctrl_get(uint8_t port_id, xport_xlmac_core_rx_ctrl *rx_ctrl)
{
    uint64_t reg_rx_ctrl=0;

#ifdef VALIDATE_PARMS
    if(!rx_ctrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, RX_CTRL, reg_rx_ctrl);

    rx_ctrl->rx_pass_pfc = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, RX_CTRL, RX_PASS_PFC, reg_rx_ctrl);
    rx_ctrl->rx_pass_pause = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, RX_CTRL, RX_PASS_PAUSE, reg_rx_ctrl);
    rx_ctrl->rx_pass_ctrl = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, RX_CTRL, RX_PASS_CTRL, reg_rx_ctrl);
    rx_ctrl->rsvd_3 = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, RX_CTRL, RSVD_3, reg_rx_ctrl);
    rx_ctrl->rsvd_2 = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, RX_CTRL, RSVD_2, reg_rx_ctrl);
    rx_ctrl->runt_threshold = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, RX_CTRL, RUNT_THRESHOLD, reg_rx_ctrl);
    rx_ctrl->strict_preamble = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, RX_CTRL, STRICT_PREAMBLE, reg_rx_ctrl);
    rx_ctrl->strip_crc = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, RX_CTRL, STRIP_CRC, reg_rx_ctrl);
    rx_ctrl->rx_any_start = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, RX_CTRL, RX_ANY_START, reg_rx_ctrl);
    rx_ctrl->rsvd_1 = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, RX_CTRL, RSVD_1, reg_rx_ctrl);

    return 0;
}

int ag_drv_xport_xlmac_core_rx_mac_sa_set(uint8_t port_id, uint64_t rx_sa)
{
    uint64_t reg_rx_mac_sa=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (rx_sa >= _48BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_rx_mac_sa = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, RX_MAC_SA, RX_SA, reg_rx_mac_sa, (uint64_t)rx_sa);

    XPORT_XLMAC_INDIRECT_WRITE(port_id, XPORT_XLMAC_CORE, RX_MAC_SA, reg_rx_mac_sa);

    return 0;
}

int ag_drv_xport_xlmac_core_rx_mac_sa_get(uint8_t port_id, uint64_t *rx_sa)
{
    uint64_t reg_rx_mac_sa=0;

#ifdef VALIDATE_PARMS
    if(!rx_sa)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, RX_MAC_SA, reg_rx_mac_sa);

    *rx_sa = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, RX_MAC_SA, RX_SA, reg_rx_mac_sa);

    return 0;
}

int ag_drv_xport_xlmac_core_rx_max_size_set(uint8_t port_id, uint16_t rx_max_size)
{
    uint64_t reg_rx_max_size=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (rx_max_size >= _14BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_rx_max_size = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, RX_MAX_SIZE, RX_MAX_SIZE, reg_rx_max_size, (uint64_t)rx_max_size);

    XPORT_XLMAC_INDIRECT_WRITE(port_id, XPORT_XLMAC_CORE, RX_MAX_SIZE, reg_rx_max_size);

    return 0;
}

int ag_drv_xport_xlmac_core_rx_max_size_get(uint8_t port_id, uint16_t *rx_max_size)
{
    uint64_t reg_rx_max_size=0;

#ifdef VALIDATE_PARMS
    if(!rx_max_size)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, RX_MAX_SIZE, reg_rx_max_size);

    *rx_max_size = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, RX_MAX_SIZE, RX_MAX_SIZE, reg_rx_max_size);

    return 0;
}

int ag_drv_xport_xlmac_core_rx_vlan_tag_set(uint8_t port_id, uint8_t outer_vlan_tag_enable, uint8_t inner_vlan_tag_enable, uint16_t outer_vlan_tag, uint16_t inner_vlan_tag)
{
    uint64_t reg_rx_vlan_tag=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (outer_vlan_tag_enable >= _1BITS_MAX_VAL_) ||
       (inner_vlan_tag_enable >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_rx_vlan_tag = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, RX_VLAN_TAG, OUTER_VLAN_TAG_ENABLE, reg_rx_vlan_tag, (uint64_t)outer_vlan_tag_enable);
    reg_rx_vlan_tag = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, RX_VLAN_TAG, INNER_VLAN_TAG_ENABLE, reg_rx_vlan_tag, (uint64_t)inner_vlan_tag_enable);
    reg_rx_vlan_tag = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, RX_VLAN_TAG, OUTER_VLAN_TAG, reg_rx_vlan_tag, (uint64_t)outer_vlan_tag);
    reg_rx_vlan_tag = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, RX_VLAN_TAG, INNER_VLAN_TAG, reg_rx_vlan_tag, (uint64_t)inner_vlan_tag);

    XPORT_XLMAC_INDIRECT_WRITE(port_id, XPORT_XLMAC_CORE, RX_VLAN_TAG, reg_rx_vlan_tag);

    return 0;
}

int ag_drv_xport_xlmac_core_rx_vlan_tag_get(uint8_t port_id, uint8_t *outer_vlan_tag_enable, uint8_t *inner_vlan_tag_enable, uint16_t *outer_vlan_tag, uint16_t *inner_vlan_tag)
{
    uint64_t reg_rx_vlan_tag=0;

#ifdef VALIDATE_PARMS
    if(!outer_vlan_tag_enable || !inner_vlan_tag_enable || !outer_vlan_tag || !inner_vlan_tag)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, RX_VLAN_TAG, reg_rx_vlan_tag);

    *outer_vlan_tag_enable = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, RX_VLAN_TAG, OUTER_VLAN_TAG_ENABLE, reg_rx_vlan_tag);
    *inner_vlan_tag_enable = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, RX_VLAN_TAG, INNER_VLAN_TAG_ENABLE, reg_rx_vlan_tag);
    *outer_vlan_tag = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, RX_VLAN_TAG, OUTER_VLAN_TAG, reg_rx_vlan_tag);
    *inner_vlan_tag = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, RX_VLAN_TAG, INNER_VLAN_TAG, reg_rx_vlan_tag);

    return 0;
}

int ag_drv_xport_xlmac_core_rx_lss_ctrl_set(uint8_t port_id, const xport_xlmac_core_rx_lss_ctrl *rx_lss_ctrl)
{
    uint64_t reg_rx_lss_ctrl=0;

#ifdef VALIDATE_PARMS
    if(!rx_lss_ctrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (rx_lss_ctrl->reset_flow_control_timers_on_link_down >= _1BITS_MAX_VAL_) ||
       (rx_lss_ctrl->drop_tx_data_on_link_interrupt >= _1BITS_MAX_VAL_) ||
       (rx_lss_ctrl->drop_tx_data_on_remote_fault >= _1BITS_MAX_VAL_) ||
       (rx_lss_ctrl->drop_tx_data_on_local_fault >= _1BITS_MAX_VAL_) ||
       (rx_lss_ctrl->link_interruption_disable >= _1BITS_MAX_VAL_) ||
       (rx_lss_ctrl->use_external_faults_for_tx >= _1BITS_MAX_VAL_) ||
       (rx_lss_ctrl->remote_fault_disable >= _1BITS_MAX_VAL_) ||
       (rx_lss_ctrl->local_fault_disable >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_rx_lss_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, RX_LSS_CTRL, RESET_FLOW_CONTROL_TIMERS_ON_LINK_DOWN, reg_rx_lss_ctrl, (uint64_t)rx_lss_ctrl->reset_flow_control_timers_on_link_down);
    reg_rx_lss_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, RX_LSS_CTRL, DROP_TX_DATA_ON_LINK_INTERRUPT, reg_rx_lss_ctrl, (uint64_t)rx_lss_ctrl->drop_tx_data_on_link_interrupt);
    reg_rx_lss_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, RX_LSS_CTRL, DROP_TX_DATA_ON_REMOTE_FAULT, reg_rx_lss_ctrl, (uint64_t)rx_lss_ctrl->drop_tx_data_on_remote_fault);
    reg_rx_lss_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, RX_LSS_CTRL, DROP_TX_DATA_ON_LOCAL_FAULT, reg_rx_lss_ctrl, (uint64_t)rx_lss_ctrl->drop_tx_data_on_local_fault);
    reg_rx_lss_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, RX_LSS_CTRL, LINK_INTERRUPTION_DISABLE, reg_rx_lss_ctrl, (uint64_t)rx_lss_ctrl->link_interruption_disable);
    reg_rx_lss_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, RX_LSS_CTRL, USE_EXTERNAL_FAULTS_FOR_TX, reg_rx_lss_ctrl, (uint64_t)rx_lss_ctrl->use_external_faults_for_tx);
    reg_rx_lss_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, RX_LSS_CTRL, REMOTE_FAULT_DISABLE, reg_rx_lss_ctrl, (uint64_t)rx_lss_ctrl->remote_fault_disable);
    reg_rx_lss_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, RX_LSS_CTRL, LOCAL_FAULT_DISABLE, reg_rx_lss_ctrl, (uint64_t)rx_lss_ctrl->local_fault_disable);

    XPORT_XLMAC_INDIRECT_WRITE(port_id, XPORT_XLMAC_CORE, RX_LSS_CTRL, reg_rx_lss_ctrl);

    return 0;
}

int ag_drv_xport_xlmac_core_rx_lss_ctrl_get(uint8_t port_id, xport_xlmac_core_rx_lss_ctrl *rx_lss_ctrl)
{
    uint64_t reg_rx_lss_ctrl=0;

#ifdef VALIDATE_PARMS
    if(!rx_lss_ctrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, RX_LSS_CTRL, reg_rx_lss_ctrl);

    rx_lss_ctrl->reset_flow_control_timers_on_link_down = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, RX_LSS_CTRL, RESET_FLOW_CONTROL_TIMERS_ON_LINK_DOWN, reg_rx_lss_ctrl);
    rx_lss_ctrl->drop_tx_data_on_link_interrupt = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, RX_LSS_CTRL, DROP_TX_DATA_ON_LINK_INTERRUPT, reg_rx_lss_ctrl);
    rx_lss_ctrl->drop_tx_data_on_remote_fault = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, RX_LSS_CTRL, DROP_TX_DATA_ON_REMOTE_FAULT, reg_rx_lss_ctrl);
    rx_lss_ctrl->drop_tx_data_on_local_fault = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, RX_LSS_CTRL, DROP_TX_DATA_ON_LOCAL_FAULT, reg_rx_lss_ctrl);
    rx_lss_ctrl->link_interruption_disable = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, RX_LSS_CTRL, LINK_INTERRUPTION_DISABLE, reg_rx_lss_ctrl);
    rx_lss_ctrl->use_external_faults_for_tx = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, RX_LSS_CTRL, USE_EXTERNAL_FAULTS_FOR_TX, reg_rx_lss_ctrl);
    rx_lss_ctrl->remote_fault_disable = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, RX_LSS_CTRL, REMOTE_FAULT_DISABLE, reg_rx_lss_ctrl);
    rx_lss_ctrl->local_fault_disable = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, RX_LSS_CTRL, LOCAL_FAULT_DISABLE, reg_rx_lss_ctrl);

    return 0;
}

int ag_drv_xport_xlmac_core_rx_lss_status_get(uint8_t port_id, uint8_t *link_interruption_status, uint8_t *remote_fault_status, uint8_t *local_fault_status)
{
    uint64_t reg_rx_lss_status=0;

#ifdef VALIDATE_PARMS
    if(!link_interruption_status || !remote_fault_status || !local_fault_status)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, RX_LSS_STATUS, reg_rx_lss_status);

    *link_interruption_status = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, RX_LSS_STATUS, LINK_INTERRUPTION_STATUS, reg_rx_lss_status);
    *remote_fault_status = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, RX_LSS_STATUS, REMOTE_FAULT_STATUS, reg_rx_lss_status);
    *local_fault_status = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, RX_LSS_STATUS, LOCAL_FAULT_STATUS, reg_rx_lss_status);

    return 0;
}

int ag_drv_xport_xlmac_core_clear_rx_lss_status_set(uint8_t port_id, uint8_t clear_link_interruption_status, uint8_t clear_remote_fault_status, uint8_t clear_local_fault_status)
{
    uint64_t reg_clear_rx_lss_status=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (clear_link_interruption_status >= _1BITS_MAX_VAL_) ||
       (clear_remote_fault_status >= _1BITS_MAX_VAL_) ||
       (clear_local_fault_status >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_clear_rx_lss_status = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, CLEAR_RX_LSS_STATUS, CLEAR_LINK_INTERRUPTION_STATUS, reg_clear_rx_lss_status, (uint64_t)clear_link_interruption_status);
    reg_clear_rx_lss_status = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, CLEAR_RX_LSS_STATUS, CLEAR_REMOTE_FAULT_STATUS, reg_clear_rx_lss_status, (uint64_t)clear_remote_fault_status);
    reg_clear_rx_lss_status = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, CLEAR_RX_LSS_STATUS, CLEAR_LOCAL_FAULT_STATUS, reg_clear_rx_lss_status, (uint64_t)clear_local_fault_status);

    XPORT_XLMAC_INDIRECT_WRITE(port_id, XPORT_XLMAC_CORE, CLEAR_RX_LSS_STATUS, reg_clear_rx_lss_status);

    return 0;
}

int ag_drv_xport_xlmac_core_clear_rx_lss_status_get(uint8_t port_id, uint8_t *clear_link_interruption_status, uint8_t *clear_remote_fault_status, uint8_t *clear_local_fault_status)
{
    uint64_t reg_clear_rx_lss_status=0;

#ifdef VALIDATE_PARMS
    if(!clear_link_interruption_status || !clear_remote_fault_status || !clear_local_fault_status)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, CLEAR_RX_LSS_STATUS, reg_clear_rx_lss_status);

    *clear_link_interruption_status = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, CLEAR_RX_LSS_STATUS, CLEAR_LINK_INTERRUPTION_STATUS, reg_clear_rx_lss_status);
    *clear_remote_fault_status = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, CLEAR_RX_LSS_STATUS, CLEAR_REMOTE_FAULT_STATUS, reg_clear_rx_lss_status);
    *clear_local_fault_status = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, CLEAR_RX_LSS_STATUS, CLEAR_LOCAL_FAULT_STATUS, reg_clear_rx_lss_status);

    return 0;
}

int ag_drv_xport_xlmac_core_pause_ctrl_set(uint8_t port_id, const xport_xlmac_core_pause_ctrl *pause_ctrl)
{
    uint64_t reg_pause_ctrl=0;

#ifdef VALIDATE_PARMS
    if(!pause_ctrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (pause_ctrl->rsvd_2 >= _1BITS_MAX_VAL_) ||
       (pause_ctrl->rsvd_1 >= _1BITS_MAX_VAL_) ||
       (pause_ctrl->rx_pause_en >= _1BITS_MAX_VAL_) ||
       (pause_ctrl->tx_pause_en >= _1BITS_MAX_VAL_) ||
       (pause_ctrl->pause_refresh_en >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_pause_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, PAUSE_CTRL, PAUSE_XOFF_TIMER, reg_pause_ctrl, (uint64_t)pause_ctrl->pause_xoff_timer);
    reg_pause_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, PAUSE_CTRL, RSVD_2, reg_pause_ctrl, (uint64_t)pause_ctrl->rsvd_2);
    reg_pause_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, PAUSE_CTRL, RSVD_1, reg_pause_ctrl, (uint64_t)pause_ctrl->rsvd_1);
    reg_pause_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, PAUSE_CTRL, RX_PAUSE_EN, reg_pause_ctrl, (uint64_t)pause_ctrl->rx_pause_en);
    reg_pause_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, PAUSE_CTRL, TX_PAUSE_EN, reg_pause_ctrl, (uint64_t)pause_ctrl->tx_pause_en);
    reg_pause_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, PAUSE_CTRL, PAUSE_REFRESH_EN, reg_pause_ctrl, (uint64_t)pause_ctrl->pause_refresh_en);
    reg_pause_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, PAUSE_CTRL, PAUSE_REFRESH_TIMER, reg_pause_ctrl, (uint64_t)pause_ctrl->pause_refresh_timer);

    XPORT_XLMAC_INDIRECT_WRITE(port_id, XPORT_XLMAC_CORE, PAUSE_CTRL, reg_pause_ctrl);

    return 0;
}

int ag_drv_xport_xlmac_core_pause_ctrl_get(uint8_t port_id, xport_xlmac_core_pause_ctrl *pause_ctrl)
{
    uint64_t reg_pause_ctrl=0;

#ifdef VALIDATE_PARMS
    if(!pause_ctrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, PAUSE_CTRL, reg_pause_ctrl);

    pause_ctrl->pause_xoff_timer = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, PAUSE_CTRL, PAUSE_XOFF_TIMER, reg_pause_ctrl);
    pause_ctrl->rsvd_2 = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, PAUSE_CTRL, RSVD_2, reg_pause_ctrl);
    pause_ctrl->rsvd_1 = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, PAUSE_CTRL, RSVD_1, reg_pause_ctrl);
    pause_ctrl->rx_pause_en = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, PAUSE_CTRL, RX_PAUSE_EN, reg_pause_ctrl);
    pause_ctrl->tx_pause_en = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, PAUSE_CTRL, TX_PAUSE_EN, reg_pause_ctrl);
    pause_ctrl->pause_refresh_en = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, PAUSE_CTRL, PAUSE_REFRESH_EN, reg_pause_ctrl);
    pause_ctrl->pause_refresh_timer = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, PAUSE_CTRL, PAUSE_REFRESH_TIMER, reg_pause_ctrl);

    return 0;
}

int ag_drv_xport_xlmac_core_pfc_ctrl_set(uint8_t port_id, const xport_xlmac_core_pfc_ctrl *pfc_ctrl)
{
    uint64_t reg_pfc_ctrl=0;

#ifdef VALIDATE_PARMS
    if(!pfc_ctrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (pfc_ctrl->tx_pfc_en >= _1BITS_MAX_VAL_) ||
       (pfc_ctrl->rx_pfc_en >= _1BITS_MAX_VAL_) ||
       (pfc_ctrl->pfc_stats_en >= _1BITS_MAX_VAL_) ||
       (pfc_ctrl->rsvd >= _1BITS_MAX_VAL_) ||
       (pfc_ctrl->force_pfc_xon >= _1BITS_MAX_VAL_) ||
       (pfc_ctrl->pfc_refresh_en >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_pfc_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, PFC_CTRL, TX_PFC_EN, reg_pfc_ctrl, (uint64_t)pfc_ctrl->tx_pfc_en);
    reg_pfc_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, PFC_CTRL, RX_PFC_EN, reg_pfc_ctrl, (uint64_t)pfc_ctrl->rx_pfc_en);
    reg_pfc_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, PFC_CTRL, PFC_STATS_EN, reg_pfc_ctrl, (uint64_t)pfc_ctrl->pfc_stats_en);
    reg_pfc_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, PFC_CTRL, RSVD, reg_pfc_ctrl, (uint64_t)pfc_ctrl->rsvd);
    reg_pfc_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, PFC_CTRL, FORCE_PFC_XON, reg_pfc_ctrl, (uint64_t)pfc_ctrl->force_pfc_xon);
    reg_pfc_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, PFC_CTRL, PFC_REFRESH_EN, reg_pfc_ctrl, (uint64_t)pfc_ctrl->pfc_refresh_en);
    reg_pfc_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, PFC_CTRL, PFC_XOFF_TIMER, reg_pfc_ctrl, (uint64_t)pfc_ctrl->pfc_xoff_timer);
    reg_pfc_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, PFC_CTRL, PFC_REFRESH_TIMER, reg_pfc_ctrl, (uint64_t)pfc_ctrl->pfc_refresh_timer);

    XPORT_XLMAC_INDIRECT_WRITE(port_id, XPORT_XLMAC_CORE, PFC_CTRL, reg_pfc_ctrl);

    return 0;
}

int ag_drv_xport_xlmac_core_pfc_ctrl_get(uint8_t port_id, xport_xlmac_core_pfc_ctrl *pfc_ctrl)
{
    uint64_t reg_pfc_ctrl=0;

#ifdef VALIDATE_PARMS
    if(!pfc_ctrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, PFC_CTRL, reg_pfc_ctrl);

    pfc_ctrl->tx_pfc_en = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, PFC_CTRL, TX_PFC_EN, reg_pfc_ctrl);
    pfc_ctrl->rx_pfc_en = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, PFC_CTRL, RX_PFC_EN, reg_pfc_ctrl);
    pfc_ctrl->pfc_stats_en = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, PFC_CTRL, PFC_STATS_EN, reg_pfc_ctrl);
    pfc_ctrl->rsvd = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, PFC_CTRL, RSVD, reg_pfc_ctrl);
    pfc_ctrl->force_pfc_xon = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, PFC_CTRL, FORCE_PFC_XON, reg_pfc_ctrl);
    pfc_ctrl->pfc_refresh_en = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, PFC_CTRL, PFC_REFRESH_EN, reg_pfc_ctrl);
    pfc_ctrl->pfc_xoff_timer = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, PFC_CTRL, PFC_XOFF_TIMER, reg_pfc_ctrl);
    pfc_ctrl->pfc_refresh_timer = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, PFC_CTRL, PFC_REFRESH_TIMER, reg_pfc_ctrl);

    return 0;
}

int ag_drv_xport_xlmac_core_pfc_type_set(uint8_t port_id, uint16_t pfc_eth_type)
{
    uint64_t reg_pfc_type=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_pfc_type = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, PFC_TYPE, PFC_ETH_TYPE, reg_pfc_type, (uint64_t)pfc_eth_type);

    XPORT_XLMAC_INDIRECT_WRITE(port_id, XPORT_XLMAC_CORE, PFC_TYPE, reg_pfc_type);

    return 0;
}

int ag_drv_xport_xlmac_core_pfc_type_get(uint8_t port_id, uint16_t *pfc_eth_type)
{
    uint64_t reg_pfc_type=0;

#ifdef VALIDATE_PARMS
    if(!pfc_eth_type)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, PFC_TYPE, reg_pfc_type);

    *pfc_eth_type = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, PFC_TYPE, PFC_ETH_TYPE, reg_pfc_type);

    return 0;
}

int ag_drv_xport_xlmac_core_pfc_opcode_set(uint8_t port_id, uint16_t pfc_opcode)
{
    uint64_t reg_pfc_opcode=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_pfc_opcode = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, PFC_OPCODE, PFC_OPCODE, reg_pfc_opcode, (uint64_t)pfc_opcode);

    XPORT_XLMAC_INDIRECT_WRITE(port_id, XPORT_XLMAC_CORE, PFC_OPCODE, reg_pfc_opcode);

    return 0;
}

int ag_drv_xport_xlmac_core_pfc_opcode_get(uint8_t port_id, uint16_t *pfc_opcode)
{
    uint64_t reg_pfc_opcode=0;

#ifdef VALIDATE_PARMS
    if(!pfc_opcode)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, PFC_OPCODE, reg_pfc_opcode);

    *pfc_opcode = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, PFC_OPCODE, PFC_OPCODE, reg_pfc_opcode);

    return 0;
}

int ag_drv_xport_xlmac_core_pfc_da_set(uint8_t port_id, uint64_t pfc_macda)
{
    uint64_t reg_pfc_da=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (pfc_macda >= _48BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_pfc_da = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, PFC_DA, PFC_MACDA, reg_pfc_da, (uint64_t)pfc_macda);

    XPORT_XLMAC_INDIRECT_WRITE(port_id, XPORT_XLMAC_CORE, PFC_DA, reg_pfc_da);

    return 0;
}

int ag_drv_xport_xlmac_core_pfc_da_get(uint8_t port_id, uint64_t *pfc_macda)
{
    uint64_t reg_pfc_da=0;

#ifdef VALIDATE_PARMS
    if(!pfc_macda)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, PFC_DA, reg_pfc_da);

    *pfc_macda = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, PFC_DA, PFC_MACDA, reg_pfc_da);

    return 0;
}

int ag_drv_xport_xlmac_core_llfc_ctrl_set(uint8_t port_id, const xport_xlmac_core_llfc_ctrl *llfc_ctrl)
{
    uint64_t reg_llfc_ctrl=0;

#ifdef VALIDATE_PARMS
    if(!llfc_ctrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (llfc_ctrl->no_som_for_crc_llfc >= _1BITS_MAX_VAL_) ||
       (llfc_ctrl->llfc_crc_ignore >= _1BITS_MAX_VAL_) ||
       (llfc_ctrl->llfc_cut_through_mode >= _1BITS_MAX_VAL_) ||
       (llfc_ctrl->llfc_in_ipg_only >= _1BITS_MAX_VAL_) ||
       (llfc_ctrl->rx_llfc_en >= _1BITS_MAX_VAL_) ||
       (llfc_ctrl->tx_llfc_en >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_llfc_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, LLFC_CTRL, LLFC_IMG, reg_llfc_ctrl, (uint64_t)llfc_ctrl->llfc_img);
    reg_llfc_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, LLFC_CTRL, NO_SOM_FOR_CRC_LLFC, reg_llfc_ctrl, (uint64_t)llfc_ctrl->no_som_for_crc_llfc);
    reg_llfc_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, LLFC_CTRL, LLFC_CRC_IGNORE, reg_llfc_ctrl, (uint64_t)llfc_ctrl->llfc_crc_ignore);
    reg_llfc_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, LLFC_CTRL, LLFC_CUT_THROUGH_MODE, reg_llfc_ctrl, (uint64_t)llfc_ctrl->llfc_cut_through_mode);
    reg_llfc_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, LLFC_CTRL, LLFC_IN_IPG_ONLY, reg_llfc_ctrl, (uint64_t)llfc_ctrl->llfc_in_ipg_only);
    reg_llfc_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, LLFC_CTRL, RX_LLFC_EN, reg_llfc_ctrl, (uint64_t)llfc_ctrl->rx_llfc_en);
    reg_llfc_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, LLFC_CTRL, TX_LLFC_EN, reg_llfc_ctrl, (uint64_t)llfc_ctrl->tx_llfc_en);

    XPORT_XLMAC_INDIRECT_WRITE(port_id, XPORT_XLMAC_CORE, LLFC_CTRL, reg_llfc_ctrl);

    return 0;
}

int ag_drv_xport_xlmac_core_llfc_ctrl_get(uint8_t port_id, xport_xlmac_core_llfc_ctrl *llfc_ctrl)
{
    uint64_t reg_llfc_ctrl=0;

#ifdef VALIDATE_PARMS
    if(!llfc_ctrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, LLFC_CTRL, reg_llfc_ctrl);

    llfc_ctrl->llfc_img = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, LLFC_CTRL, LLFC_IMG, reg_llfc_ctrl);
    llfc_ctrl->no_som_for_crc_llfc = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, LLFC_CTRL, NO_SOM_FOR_CRC_LLFC, reg_llfc_ctrl);
    llfc_ctrl->llfc_crc_ignore = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, LLFC_CTRL, LLFC_CRC_IGNORE, reg_llfc_ctrl);
    llfc_ctrl->llfc_cut_through_mode = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, LLFC_CTRL, LLFC_CUT_THROUGH_MODE, reg_llfc_ctrl);
    llfc_ctrl->llfc_in_ipg_only = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, LLFC_CTRL, LLFC_IN_IPG_ONLY, reg_llfc_ctrl);
    llfc_ctrl->rx_llfc_en = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, LLFC_CTRL, RX_LLFC_EN, reg_llfc_ctrl);
    llfc_ctrl->tx_llfc_en = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, LLFC_CTRL, TX_LLFC_EN, reg_llfc_ctrl);

    return 0;
}

int ag_drv_xport_xlmac_core_tx_llfc_msg_fields_set(uint8_t port_id, uint16_t llfc_xoff_time, uint8_t tx_llfc_fc_obj_logical, uint8_t tx_llfc_msg_type_logical)
{
    uint64_t reg_tx_llfc_msg_fields=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (tx_llfc_fc_obj_logical >= _4BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_tx_llfc_msg_fields = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, TX_LLFC_MSG_FIELDS, LLFC_XOFF_TIME, reg_tx_llfc_msg_fields, (uint64_t)llfc_xoff_time);
    reg_tx_llfc_msg_fields = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, TX_LLFC_MSG_FIELDS, TX_LLFC_FC_OBJ_LOGICAL, reg_tx_llfc_msg_fields, (uint64_t)tx_llfc_fc_obj_logical);
    reg_tx_llfc_msg_fields = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, TX_LLFC_MSG_FIELDS, TX_LLFC_MSG_TYPE_LOGICAL, reg_tx_llfc_msg_fields, (uint64_t)tx_llfc_msg_type_logical);

    XPORT_XLMAC_INDIRECT_WRITE(port_id, XPORT_XLMAC_CORE, TX_LLFC_MSG_FIELDS, reg_tx_llfc_msg_fields);

    return 0;
}

int ag_drv_xport_xlmac_core_tx_llfc_msg_fields_get(uint8_t port_id, uint16_t *llfc_xoff_time, uint8_t *tx_llfc_fc_obj_logical, uint8_t *tx_llfc_msg_type_logical)
{
    uint64_t reg_tx_llfc_msg_fields=0;

#ifdef VALIDATE_PARMS
    if(!llfc_xoff_time || !tx_llfc_fc_obj_logical || !tx_llfc_msg_type_logical)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, TX_LLFC_MSG_FIELDS, reg_tx_llfc_msg_fields);

    *llfc_xoff_time = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, TX_LLFC_MSG_FIELDS, LLFC_XOFF_TIME, reg_tx_llfc_msg_fields);
    *tx_llfc_fc_obj_logical = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, TX_LLFC_MSG_FIELDS, TX_LLFC_FC_OBJ_LOGICAL, reg_tx_llfc_msg_fields);
    *tx_llfc_msg_type_logical = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, TX_LLFC_MSG_FIELDS, TX_LLFC_MSG_TYPE_LOGICAL, reg_tx_llfc_msg_fields);

    return 0;
}

int ag_drv_xport_xlmac_core_rx_llfc_msg_fields_set(uint8_t port_id, uint8_t rx_llfc_fc_obj_physical, uint8_t rx_llfc_msg_type_physical, uint8_t rx_llfc_fc_obj_logical, uint8_t rx_llfc_msg_type_logical)
{
    uint64_t reg_rx_llfc_msg_fields=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (rx_llfc_fc_obj_physical >= _4BITS_MAX_VAL_) ||
       (rx_llfc_fc_obj_logical >= _4BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_rx_llfc_msg_fields = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, RX_LLFC_MSG_FIELDS, RX_LLFC_FC_OBJ_PHYSICAL, reg_rx_llfc_msg_fields, (uint64_t)rx_llfc_fc_obj_physical);
    reg_rx_llfc_msg_fields = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, RX_LLFC_MSG_FIELDS, RX_LLFC_MSG_TYPE_PHYSICAL, reg_rx_llfc_msg_fields, (uint64_t)rx_llfc_msg_type_physical);
    reg_rx_llfc_msg_fields = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, RX_LLFC_MSG_FIELDS, RX_LLFC_FC_OBJ_LOGICAL, reg_rx_llfc_msg_fields, (uint64_t)rx_llfc_fc_obj_logical);
    reg_rx_llfc_msg_fields = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, RX_LLFC_MSG_FIELDS, RX_LLFC_MSG_TYPE_LOGICAL, reg_rx_llfc_msg_fields, (uint64_t)rx_llfc_msg_type_logical);

    XPORT_XLMAC_INDIRECT_WRITE(port_id, XPORT_XLMAC_CORE, RX_LLFC_MSG_FIELDS, reg_rx_llfc_msg_fields);

    return 0;
}

int ag_drv_xport_xlmac_core_rx_llfc_msg_fields_get(uint8_t port_id, uint8_t *rx_llfc_fc_obj_physical, uint8_t *rx_llfc_msg_type_physical, uint8_t *rx_llfc_fc_obj_logical, uint8_t *rx_llfc_msg_type_logical)
{
    uint64_t reg_rx_llfc_msg_fields=0;

#ifdef VALIDATE_PARMS
    if(!rx_llfc_fc_obj_physical || !rx_llfc_msg_type_physical || !rx_llfc_fc_obj_logical || !rx_llfc_msg_type_logical)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, RX_LLFC_MSG_FIELDS, reg_rx_llfc_msg_fields);

    *rx_llfc_fc_obj_physical = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, RX_LLFC_MSG_FIELDS, RX_LLFC_FC_OBJ_PHYSICAL, reg_rx_llfc_msg_fields);
    *rx_llfc_msg_type_physical = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, RX_LLFC_MSG_FIELDS, RX_LLFC_MSG_TYPE_PHYSICAL, reg_rx_llfc_msg_fields);
    *rx_llfc_fc_obj_logical = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, RX_LLFC_MSG_FIELDS, RX_LLFC_FC_OBJ_LOGICAL, reg_rx_llfc_msg_fields);
    *rx_llfc_msg_type_logical = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, RX_LLFC_MSG_FIELDS, RX_LLFC_MSG_TYPE_LOGICAL, reg_rx_llfc_msg_fields);

    return 0;
}

int ag_drv_xport_xlmac_core_tx_timestamp_fifo_data_get(uint8_t port_id, uint8_t *ts_entry_valid, uint16_t *sequence_id, uint32_t *time_stamp)
{
    uint64_t reg_tx_timestamp_fifo_data=0;

#ifdef VALIDATE_PARMS
    if(!ts_entry_valid || !sequence_id || !time_stamp)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, TX_TIMESTAMP_FIFO_DATA, reg_tx_timestamp_fifo_data);

    *ts_entry_valid = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, TX_TIMESTAMP_FIFO_DATA, TS_ENTRY_VALID, reg_tx_timestamp_fifo_data);
    *sequence_id = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, TX_TIMESTAMP_FIFO_DATA, SEQUENCE_ID, reg_tx_timestamp_fifo_data);
    *time_stamp = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, TX_TIMESTAMP_FIFO_DATA, TIME_STAMP, reg_tx_timestamp_fifo_data);

    return 0;
}

int ag_drv_xport_xlmac_core_tx_timestamp_fifo_status_get(uint8_t port_id, uint8_t *entry_count)
{
    uint64_t reg_tx_timestamp_fifo_status=0;

#ifdef VALIDATE_PARMS
    if(!entry_count)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, TX_TIMESTAMP_FIFO_STATUS, reg_tx_timestamp_fifo_status);

    *entry_count = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, TX_TIMESTAMP_FIFO_STATUS, ENTRY_COUNT, reg_tx_timestamp_fifo_status);

    return 0;
}

int ag_drv_xport_xlmac_core_fifo_status_get(uint8_t port_id, xport_xlmac_core_fifo_status *fifo_status)
{
    uint64_t reg_fifo_status=0;

#ifdef VALIDATE_PARMS
    if(!fifo_status)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, FIFO_STATUS, reg_fifo_status);

    fifo_status->link_status = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, FIFO_STATUS, LINK_STATUS, reg_fifo_status);
    fifo_status->rx_pkt_overflow = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, FIFO_STATUS, RX_PKT_OVERFLOW, reg_fifo_status);
    fifo_status->tx_ts_fifo_overflow = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, FIFO_STATUS, TX_TS_FIFO_OVERFLOW, reg_fifo_status);
    fifo_status->tx_llfc_msg_overflow = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, FIFO_STATUS, TX_LLFC_MSG_OVERFLOW, reg_fifo_status);
    fifo_status->rsvd_2 = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, FIFO_STATUS, RSVD_2, reg_fifo_status);
    fifo_status->tx_pkt_overflow = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, FIFO_STATUS, TX_PKT_OVERFLOW, reg_fifo_status);
    fifo_status->tx_pkt_underflow = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, FIFO_STATUS, TX_PKT_UNDERFLOW, reg_fifo_status);
    fifo_status->rx_msg_overflow = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, FIFO_STATUS, RX_MSG_OVERFLOW, reg_fifo_status);
    fifo_status->rsvd_1 = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, FIFO_STATUS, RSVD_1, reg_fifo_status);

    return 0;
}

int ag_drv_xport_xlmac_core_clear_fifo_status_set(uint8_t port_id, const xport_xlmac_core_clear_fifo_status *clear_fifo_status)
{
    uint64_t reg_clear_fifo_status=0;

#ifdef VALIDATE_PARMS
    if(!clear_fifo_status)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (clear_fifo_status->clear_rx_pkt_overflow >= _1BITS_MAX_VAL_) ||
       (clear_fifo_status->clear_tx_ts_fifo_overflow >= _1BITS_MAX_VAL_) ||
       (clear_fifo_status->clear_tx_llfc_msg_overflow >= _1BITS_MAX_VAL_) ||
       (clear_fifo_status->rsvd_2 >= _1BITS_MAX_VAL_) ||
       (clear_fifo_status->clear_tx_pkt_overflow >= _1BITS_MAX_VAL_) ||
       (clear_fifo_status->clear_tx_pkt_underflow >= _1BITS_MAX_VAL_) ||
       (clear_fifo_status->clear_rx_msg_overflow >= _1BITS_MAX_VAL_) ||
       (clear_fifo_status->rsvd_1 >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_clear_fifo_status = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, CLEAR_FIFO_STATUS, CLEAR_RX_PKT_OVERFLOW, reg_clear_fifo_status, (uint64_t)clear_fifo_status->clear_rx_pkt_overflow);
    reg_clear_fifo_status = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, CLEAR_FIFO_STATUS, CLEAR_TX_TS_FIFO_OVERFLOW, reg_clear_fifo_status, (uint64_t)clear_fifo_status->clear_tx_ts_fifo_overflow);
    reg_clear_fifo_status = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, CLEAR_FIFO_STATUS, CLEAR_TX_LLFC_MSG_OVERFLOW, reg_clear_fifo_status, (uint64_t)clear_fifo_status->clear_tx_llfc_msg_overflow);
    reg_clear_fifo_status = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, CLEAR_FIFO_STATUS, RSVD_2, reg_clear_fifo_status, (uint64_t)clear_fifo_status->rsvd_2);
    reg_clear_fifo_status = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, CLEAR_FIFO_STATUS, CLEAR_TX_PKT_OVERFLOW, reg_clear_fifo_status, (uint64_t)clear_fifo_status->clear_tx_pkt_overflow);
    reg_clear_fifo_status = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, CLEAR_FIFO_STATUS, CLEAR_TX_PKT_UNDERFLOW, reg_clear_fifo_status, (uint64_t)clear_fifo_status->clear_tx_pkt_underflow);
    reg_clear_fifo_status = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, CLEAR_FIFO_STATUS, CLEAR_RX_MSG_OVERFLOW, reg_clear_fifo_status, (uint64_t)clear_fifo_status->clear_rx_msg_overflow);
    reg_clear_fifo_status = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, CLEAR_FIFO_STATUS, RSVD_1, reg_clear_fifo_status, (uint64_t)clear_fifo_status->rsvd_1);

    XPORT_XLMAC_INDIRECT_WRITE(port_id, XPORT_XLMAC_CORE, CLEAR_FIFO_STATUS, reg_clear_fifo_status);

    return 0;
}

int ag_drv_xport_xlmac_core_clear_fifo_status_get(uint8_t port_id, xport_xlmac_core_clear_fifo_status *clear_fifo_status)
{
    uint64_t reg_clear_fifo_status=0;

#ifdef VALIDATE_PARMS
    if(!clear_fifo_status)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, CLEAR_FIFO_STATUS, reg_clear_fifo_status);

    clear_fifo_status->clear_rx_pkt_overflow = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, CLEAR_FIFO_STATUS, CLEAR_RX_PKT_OVERFLOW, reg_clear_fifo_status);
    clear_fifo_status->clear_tx_ts_fifo_overflow = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, CLEAR_FIFO_STATUS, CLEAR_TX_TS_FIFO_OVERFLOW, reg_clear_fifo_status);
    clear_fifo_status->clear_tx_llfc_msg_overflow = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, CLEAR_FIFO_STATUS, CLEAR_TX_LLFC_MSG_OVERFLOW, reg_clear_fifo_status);
    clear_fifo_status->rsvd_2 = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, CLEAR_FIFO_STATUS, RSVD_2, reg_clear_fifo_status);
    clear_fifo_status->clear_tx_pkt_overflow = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, CLEAR_FIFO_STATUS, CLEAR_TX_PKT_OVERFLOW, reg_clear_fifo_status);
    clear_fifo_status->clear_tx_pkt_underflow = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, CLEAR_FIFO_STATUS, CLEAR_TX_PKT_UNDERFLOW, reg_clear_fifo_status);
    clear_fifo_status->clear_rx_msg_overflow = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, CLEAR_FIFO_STATUS, CLEAR_RX_MSG_OVERFLOW, reg_clear_fifo_status);
    clear_fifo_status->rsvd_1 = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, CLEAR_FIFO_STATUS, RSVD_1, reg_clear_fifo_status);

    return 0;
}

int ag_drv_xport_xlmac_core_lag_failover_status_get(uint8_t port_id, uint8_t *rsvd, uint8_t *lag_failover_loopback)
{
    uint64_t reg_lag_failover_status=0;

#ifdef VALIDATE_PARMS
    if(!rsvd || !lag_failover_loopback)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, LAG_FAILOVER_STATUS, reg_lag_failover_status);

    *rsvd = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, LAG_FAILOVER_STATUS, RSVD, reg_lag_failover_status);
    *lag_failover_loopback = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, LAG_FAILOVER_STATUS, LAG_FAILOVER_LOOPBACK, reg_lag_failover_status);

    return 0;
}

int ag_drv_xport_xlmac_core_eee_ctrl_set(uint8_t port_id, uint8_t rsvd, uint8_t eee_en)
{
    uint64_t reg_eee_ctrl=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (rsvd >= _1BITS_MAX_VAL_) ||
       (eee_en >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_eee_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, EEE_CTRL, RSVD, reg_eee_ctrl, (uint64_t)rsvd);
    reg_eee_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, EEE_CTRL, EEE_EN, reg_eee_ctrl, (uint64_t)eee_en);

    XPORT_XLMAC_INDIRECT_WRITE(port_id, XPORT_XLMAC_CORE, EEE_CTRL, reg_eee_ctrl);

    return 0;
}

int ag_drv_xport_xlmac_core_eee_ctrl_get(uint8_t port_id, uint8_t *rsvd, uint8_t *eee_en)
{
    uint64_t reg_eee_ctrl=0;

#ifdef VALIDATE_PARMS
    if(!rsvd || !eee_en)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, EEE_CTRL, reg_eee_ctrl);

    *rsvd = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, EEE_CTRL, RSVD, reg_eee_ctrl);
    *eee_en = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, EEE_CTRL, EEE_EN, reg_eee_ctrl);

    return 0;
}

int ag_drv_xport_xlmac_core_eee_timers_set(uint8_t port_id, uint16_t eee_ref_count, uint16_t eee_wake_timer, uint32_t eee_delay_entry_timer)
{
    uint64_t reg_eee_timers=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_eee_timers = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, EEE_TIMERS, EEE_REF_COUNT, reg_eee_timers, (uint64_t)eee_ref_count);
    reg_eee_timers = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, EEE_TIMERS, EEE_WAKE_TIMER, reg_eee_timers, (uint64_t)eee_wake_timer);
    reg_eee_timers = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, EEE_TIMERS, EEE_DELAY_ENTRY_TIMER, reg_eee_timers, (uint64_t)eee_delay_entry_timer);

    XPORT_XLMAC_INDIRECT_WRITE(port_id, XPORT_XLMAC_CORE, EEE_TIMERS, reg_eee_timers);

    return 0;
}

int ag_drv_xport_xlmac_core_eee_timers_get(uint8_t port_id, uint16_t *eee_ref_count, uint16_t *eee_wake_timer, uint32_t *eee_delay_entry_timer)
{
    uint64_t reg_eee_timers=0;

#ifdef VALIDATE_PARMS
    if(!eee_ref_count || !eee_wake_timer || !eee_delay_entry_timer)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, EEE_TIMERS, reg_eee_timers);

    *eee_ref_count = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, EEE_TIMERS, EEE_REF_COUNT, reg_eee_timers);
    *eee_wake_timer = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, EEE_TIMERS, EEE_WAKE_TIMER, reg_eee_timers);
    *eee_delay_entry_timer = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, EEE_TIMERS, EEE_DELAY_ENTRY_TIMER, reg_eee_timers);

    return 0;
}

int ag_drv_xport_xlmac_core_eee_1_sec_link_status_timer_set(uint8_t port_id, uint32_t one_second_timer)
{
    uint64_t reg_eee_1_sec_link_status_timer=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (one_second_timer >= _24BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_eee_1_sec_link_status_timer = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, EEE_1_SEC_LINK_STATUS_TIMER, ONE_SECOND_TIMER, reg_eee_1_sec_link_status_timer, (uint64_t)one_second_timer);

    XPORT_XLMAC_INDIRECT_WRITE(port_id, XPORT_XLMAC_CORE, EEE_1_SEC_LINK_STATUS_TIMER, reg_eee_1_sec_link_status_timer);

    return 0;
}

int ag_drv_xport_xlmac_core_eee_1_sec_link_status_timer_get(uint8_t port_id, uint32_t *one_second_timer)
{
    uint64_t reg_eee_1_sec_link_status_timer=0;

#ifdef VALIDATE_PARMS
    if(!one_second_timer)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, EEE_1_SEC_LINK_STATUS_TIMER, reg_eee_1_sec_link_status_timer);

    *one_second_timer = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, EEE_1_SEC_LINK_STATUS_TIMER, ONE_SECOND_TIMER, reg_eee_1_sec_link_status_timer);

    return 0;
}

int ag_drv_xport_xlmac_core_higig_hdr_0_set(uint8_t port_id, uint64_t higig_hdr_0)
{
    uint64_t reg_higig_hdr_0=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_higig_hdr_0 = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, HIGIG_HDR_0, HIGIG_HDR_0, reg_higig_hdr_0, (uint64_t)higig_hdr_0);

    XPORT_XLMAC_INDIRECT_WRITE(port_id, XPORT_XLMAC_CORE, HIGIG_HDR_0, reg_higig_hdr_0);

    return 0;
}

int ag_drv_xport_xlmac_core_higig_hdr_0_get(uint8_t port_id, uint64_t *higig_hdr_0)
{
    uint64_t reg_higig_hdr_0=0;

#ifdef VALIDATE_PARMS
    if(!higig_hdr_0)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, HIGIG_HDR_0, reg_higig_hdr_0);

    *higig_hdr_0 = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, HIGIG_HDR_0, HIGIG_HDR_0, reg_higig_hdr_0);

    return 0;
}

int ag_drv_xport_xlmac_core_higig_hdr_1_set(uint8_t port_id, uint64_t higig_hdr_1)
{
    uint64_t reg_higig_hdr_1=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_higig_hdr_1 = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, HIGIG_HDR_1, HIGIG_HDR_1, reg_higig_hdr_1, (uint64_t)higig_hdr_1);

    XPORT_XLMAC_INDIRECT_WRITE(port_id, XPORT_XLMAC_CORE, HIGIG_HDR_1, reg_higig_hdr_1);

    return 0;
}

int ag_drv_xport_xlmac_core_higig_hdr_1_get(uint8_t port_id, uint64_t *higig_hdr_1)
{
    uint64_t reg_higig_hdr_1=0;

#ifdef VALIDATE_PARMS
    if(!higig_hdr_1)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, HIGIG_HDR_1, reg_higig_hdr_1);

    *higig_hdr_1 = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, HIGIG_HDR_1, HIGIG_HDR_1, reg_higig_hdr_1);

    return 0;
}

int ag_drv_xport_xlmac_core_gmii_eee_ctrl_set(uint8_t port_id, uint8_t gmii_lpi_predict_mode_en, uint16_t gmii_lpi_predict_threshold)
{
    uint64_t reg_gmii_eee_ctrl=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (gmii_lpi_predict_mode_en >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gmii_eee_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, GMII_EEE_CTRL, GMII_LPI_PREDICT_MODE_EN, reg_gmii_eee_ctrl, (uint64_t)gmii_lpi_predict_mode_en);
    reg_gmii_eee_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, GMII_EEE_CTRL, GMII_LPI_PREDICT_THRESHOLD, reg_gmii_eee_ctrl, (uint64_t)gmii_lpi_predict_threshold);

    XPORT_XLMAC_INDIRECT_WRITE(port_id, XPORT_XLMAC_CORE, GMII_EEE_CTRL, reg_gmii_eee_ctrl);

    return 0;
}

int ag_drv_xport_xlmac_core_gmii_eee_ctrl_get(uint8_t port_id, uint8_t *gmii_lpi_predict_mode_en, uint16_t *gmii_lpi_predict_threshold)
{
    uint64_t reg_gmii_eee_ctrl=0;

#ifdef VALIDATE_PARMS
    if(!gmii_lpi_predict_mode_en || !gmii_lpi_predict_threshold)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, GMII_EEE_CTRL, reg_gmii_eee_ctrl);

    *gmii_lpi_predict_mode_en = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, GMII_EEE_CTRL, GMII_LPI_PREDICT_MODE_EN, reg_gmii_eee_ctrl);
    *gmii_lpi_predict_threshold = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, GMII_EEE_CTRL, GMII_LPI_PREDICT_THRESHOLD, reg_gmii_eee_ctrl);

    return 0;
}

int ag_drv_xport_xlmac_core_timestamp_adjust_set(uint8_t port_id, uint8_t ts_use_cs_offset, uint8_t ts_tsts_adjust, uint16_t ts_osts_adjust)
{
    uint64_t reg_timestamp_adjust=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (ts_use_cs_offset >= _1BITS_MAX_VAL_) ||
       (ts_tsts_adjust >= _6BITS_MAX_VAL_) ||
       (ts_osts_adjust >= _9BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_timestamp_adjust = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, TIMESTAMP_ADJUST, TS_USE_CS_OFFSET, reg_timestamp_adjust, (uint64_t)ts_use_cs_offset);
    reg_timestamp_adjust = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, TIMESTAMP_ADJUST, TS_TSTS_ADJUST, reg_timestamp_adjust, (uint64_t)ts_tsts_adjust);
    reg_timestamp_adjust = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, TIMESTAMP_ADJUST, TS_OSTS_ADJUST, reg_timestamp_adjust, (uint64_t)ts_osts_adjust);

    XPORT_XLMAC_INDIRECT_WRITE(port_id, XPORT_XLMAC_CORE, TIMESTAMP_ADJUST, reg_timestamp_adjust);

    return 0;
}

int ag_drv_xport_xlmac_core_timestamp_adjust_get(uint8_t port_id, uint8_t *ts_use_cs_offset, uint8_t *ts_tsts_adjust, uint16_t *ts_osts_adjust)
{
    uint64_t reg_timestamp_adjust=0;

#ifdef VALIDATE_PARMS
    if(!ts_use_cs_offset || !ts_tsts_adjust || !ts_osts_adjust)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, TIMESTAMP_ADJUST, reg_timestamp_adjust);

    *ts_use_cs_offset = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, TIMESTAMP_ADJUST, TS_USE_CS_OFFSET, reg_timestamp_adjust);
    *ts_tsts_adjust = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, TIMESTAMP_ADJUST, TS_TSTS_ADJUST, reg_timestamp_adjust);
    *ts_osts_adjust = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, TIMESTAMP_ADJUST, TS_OSTS_ADJUST, reg_timestamp_adjust);

    return 0;
}

int ag_drv_xport_xlmac_core_timestamp_byte_adjust_set(uint8_t port_id, uint8_t rx_timer_byte_adjust_en, uint16_t rx_timer_byte_adjust, uint8_t tx_timer_byte_adjust_en, uint16_t tx_timer_byte_adjust)
{
    uint64_t reg_timestamp_byte_adjust=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (rx_timer_byte_adjust_en >= _1BITS_MAX_VAL_) ||
       (rx_timer_byte_adjust >= _10BITS_MAX_VAL_) ||
       (tx_timer_byte_adjust_en >= _1BITS_MAX_VAL_) ||
       (tx_timer_byte_adjust >= _10BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_timestamp_byte_adjust = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, TIMESTAMP_BYTE_ADJUST, RX_TIMER_BYTE_ADJUST_EN, reg_timestamp_byte_adjust, (uint64_t)rx_timer_byte_adjust_en);
    reg_timestamp_byte_adjust = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, TIMESTAMP_BYTE_ADJUST, RX_TIMER_BYTE_ADJUST, reg_timestamp_byte_adjust, (uint64_t)rx_timer_byte_adjust);
    reg_timestamp_byte_adjust = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, TIMESTAMP_BYTE_ADJUST, TX_TIMER_BYTE_ADJUST_EN, reg_timestamp_byte_adjust, (uint64_t)tx_timer_byte_adjust_en);
    reg_timestamp_byte_adjust = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, TIMESTAMP_BYTE_ADJUST, TX_TIMER_BYTE_ADJUST, reg_timestamp_byte_adjust, (uint64_t)tx_timer_byte_adjust);

    XPORT_XLMAC_INDIRECT_WRITE(port_id, XPORT_XLMAC_CORE, TIMESTAMP_BYTE_ADJUST, reg_timestamp_byte_adjust);

    return 0;
}

int ag_drv_xport_xlmac_core_timestamp_byte_adjust_get(uint8_t port_id, uint8_t *rx_timer_byte_adjust_en, uint16_t *rx_timer_byte_adjust, uint8_t *tx_timer_byte_adjust_en, uint16_t *tx_timer_byte_adjust)
{
    uint64_t reg_timestamp_byte_adjust=0;

#ifdef VALIDATE_PARMS
    if(!rx_timer_byte_adjust_en || !rx_timer_byte_adjust || !tx_timer_byte_adjust_en || !tx_timer_byte_adjust)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, TIMESTAMP_BYTE_ADJUST, reg_timestamp_byte_adjust);

    *rx_timer_byte_adjust_en = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, TIMESTAMP_BYTE_ADJUST, RX_TIMER_BYTE_ADJUST_EN, reg_timestamp_byte_adjust);
    *rx_timer_byte_adjust = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, TIMESTAMP_BYTE_ADJUST, RX_TIMER_BYTE_ADJUST, reg_timestamp_byte_adjust);
    *tx_timer_byte_adjust_en = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, TIMESTAMP_BYTE_ADJUST, TX_TIMER_BYTE_ADJUST_EN, reg_timestamp_byte_adjust);
    *tx_timer_byte_adjust = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, TIMESTAMP_BYTE_ADJUST, TX_TIMER_BYTE_ADJUST, reg_timestamp_byte_adjust);

    return 0;
}

int ag_drv_xport_xlmac_core_tx_crc_corrupt_ctrl_set(uint8_t port_id, uint32_t prog_tx_crc, uint8_t tx_crc_corruption_mode, uint8_t tx_crc_corrupt_en, uint8_t tx_err_corrupts_crc)
{
    uint64_t reg_tx_crc_corrupt_ctrl=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (tx_crc_corruption_mode >= _1BITS_MAX_VAL_) ||
       (tx_crc_corrupt_en >= _1BITS_MAX_VAL_) ||
       (tx_err_corrupts_crc >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_tx_crc_corrupt_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, TX_CRC_CORRUPT_CTRL, PROG_TX_CRC, reg_tx_crc_corrupt_ctrl, (uint64_t)prog_tx_crc);
    reg_tx_crc_corrupt_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, TX_CRC_CORRUPT_CTRL, TX_CRC_CORRUPTION_MODE, reg_tx_crc_corrupt_ctrl, (uint64_t)tx_crc_corruption_mode);
    reg_tx_crc_corrupt_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, TX_CRC_CORRUPT_CTRL, TX_CRC_CORRUPT_EN, reg_tx_crc_corrupt_ctrl, (uint64_t)tx_crc_corrupt_en);
    reg_tx_crc_corrupt_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, TX_CRC_CORRUPT_CTRL, TX_ERR_CORRUPTS_CRC, reg_tx_crc_corrupt_ctrl, (uint64_t)tx_err_corrupts_crc);

    XPORT_XLMAC_INDIRECT_WRITE(port_id, XPORT_XLMAC_CORE, TX_CRC_CORRUPT_CTRL, reg_tx_crc_corrupt_ctrl);

    return 0;
}

int ag_drv_xport_xlmac_core_tx_crc_corrupt_ctrl_get(uint8_t port_id, uint32_t *prog_tx_crc, uint8_t *tx_crc_corruption_mode, uint8_t *tx_crc_corrupt_en, uint8_t *tx_err_corrupts_crc)
{
    uint64_t reg_tx_crc_corrupt_ctrl=0;

#ifdef VALIDATE_PARMS
    if(!prog_tx_crc || !tx_crc_corruption_mode || !tx_crc_corrupt_en || !tx_err_corrupts_crc)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, TX_CRC_CORRUPT_CTRL, reg_tx_crc_corrupt_ctrl);

    *prog_tx_crc = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, TX_CRC_CORRUPT_CTRL, PROG_TX_CRC, reg_tx_crc_corrupt_ctrl);
    *tx_crc_corruption_mode = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, TX_CRC_CORRUPT_CTRL, TX_CRC_CORRUPTION_MODE, reg_tx_crc_corrupt_ctrl);
    *tx_crc_corrupt_en = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, TX_CRC_CORRUPT_CTRL, TX_CRC_CORRUPT_EN, reg_tx_crc_corrupt_ctrl);
    *tx_err_corrupts_crc = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, TX_CRC_CORRUPT_CTRL, TX_ERR_CORRUPTS_CRC, reg_tx_crc_corrupt_ctrl);

    return 0;
}

int ag_drv_xport_xlmac_core_e2e_ctrl_set(uint8_t port_id, const xport_xlmac_core_e2e_ctrl *e2e_ctrl)
{
    uint64_t reg_e2e_ctrl=0;

#ifdef VALIDATE_PARMS
    if(!e2e_ctrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (e2e_ctrl->e2efc_dual_modid_en >= _1BITS_MAX_VAL_) ||
       (e2e_ctrl->e2ecc_legacy_imp_en >= _1BITS_MAX_VAL_) ||
       (e2e_ctrl->e2ecc_dual_modid_en >= _1BITS_MAX_VAL_) ||
       (e2e_ctrl->honor_pause_for_e2e >= _1BITS_MAX_VAL_) ||
       (e2e_ctrl->e2e_enable >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_e2e_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, E2E_CTRL, E2EFC_DUAL_MODID_EN, reg_e2e_ctrl, (uint64_t)e2e_ctrl->e2efc_dual_modid_en);
    reg_e2e_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, E2E_CTRL, E2ECC_LEGACY_IMP_EN, reg_e2e_ctrl, (uint64_t)e2e_ctrl->e2ecc_legacy_imp_en);
    reg_e2e_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, E2E_CTRL, E2ECC_DUAL_MODID_EN, reg_e2e_ctrl, (uint64_t)e2e_ctrl->e2ecc_dual_modid_en);
    reg_e2e_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, E2E_CTRL, HONOR_PAUSE_FOR_E2E, reg_e2e_ctrl, (uint64_t)e2e_ctrl->honor_pause_for_e2e);
    reg_e2e_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, E2E_CTRL, E2E_ENABLE, reg_e2e_ctrl, (uint64_t)e2e_ctrl->e2e_enable);

    XPORT_XLMAC_INDIRECT_WRITE(port_id, XPORT_XLMAC_CORE, E2E_CTRL, reg_e2e_ctrl);

    return 0;
}

int ag_drv_xport_xlmac_core_e2e_ctrl_get(uint8_t port_id, xport_xlmac_core_e2e_ctrl *e2e_ctrl)
{
    uint64_t reg_e2e_ctrl=0;

#ifdef VALIDATE_PARMS
    if(!e2e_ctrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, E2E_CTRL, reg_e2e_ctrl);

    e2e_ctrl->e2efc_dual_modid_en = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, E2E_CTRL, E2EFC_DUAL_MODID_EN, reg_e2e_ctrl);
    e2e_ctrl->e2ecc_legacy_imp_en = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, E2E_CTRL, E2ECC_LEGACY_IMP_EN, reg_e2e_ctrl);
    e2e_ctrl->e2ecc_dual_modid_en = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, E2E_CTRL, E2ECC_DUAL_MODID_EN, reg_e2e_ctrl);
    e2e_ctrl->honor_pause_for_e2e = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, E2E_CTRL, HONOR_PAUSE_FOR_E2E, reg_e2e_ctrl);
    e2e_ctrl->e2e_enable = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, E2E_CTRL, E2E_ENABLE, reg_e2e_ctrl);

    return 0;
}

int ag_drv_xport_xlmac_core_e2ecc_module_hdr_0_set(uint8_t port_id, uint64_t e2ecc_module_hdr_0)
{
    uint64_t reg_e2ecc_module_hdr_0=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_e2ecc_module_hdr_0 = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, E2ECC_MODULE_HDR_0, E2ECC_MODULE_HDR_0, reg_e2ecc_module_hdr_0, (uint64_t)e2ecc_module_hdr_0);

    XPORT_XLMAC_INDIRECT_WRITE(port_id, XPORT_XLMAC_CORE, E2ECC_MODULE_HDR_0, reg_e2ecc_module_hdr_0);

    return 0;
}

int ag_drv_xport_xlmac_core_e2ecc_module_hdr_0_get(uint8_t port_id, uint64_t *e2ecc_module_hdr_0)
{
    uint64_t reg_e2ecc_module_hdr_0=0;

#ifdef VALIDATE_PARMS
    if(!e2ecc_module_hdr_0)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, E2ECC_MODULE_HDR_0, reg_e2ecc_module_hdr_0);

    *e2ecc_module_hdr_0 = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, E2ECC_MODULE_HDR_0, E2ECC_MODULE_HDR_0, reg_e2ecc_module_hdr_0);

    return 0;
}

int ag_drv_xport_xlmac_core_e2ecc_module_hdr_1_set(uint8_t port_id, uint64_t e2ecc_module_hdr_1)
{
    uint64_t reg_e2ecc_module_hdr_1=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_e2ecc_module_hdr_1 = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, E2ECC_MODULE_HDR_1, E2ECC_MODULE_HDR_1, reg_e2ecc_module_hdr_1, (uint64_t)e2ecc_module_hdr_1);

    XPORT_XLMAC_INDIRECT_WRITE(port_id, XPORT_XLMAC_CORE, E2ECC_MODULE_HDR_1, reg_e2ecc_module_hdr_1);

    return 0;
}

int ag_drv_xport_xlmac_core_e2ecc_module_hdr_1_get(uint8_t port_id, uint64_t *e2ecc_module_hdr_1)
{
    uint64_t reg_e2ecc_module_hdr_1=0;

#ifdef VALIDATE_PARMS
    if(!e2ecc_module_hdr_1)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, E2ECC_MODULE_HDR_1, reg_e2ecc_module_hdr_1);

    *e2ecc_module_hdr_1 = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, E2ECC_MODULE_HDR_1, E2ECC_MODULE_HDR_1, reg_e2ecc_module_hdr_1);

    return 0;
}

int ag_drv_xport_xlmac_core_e2ecc_data_hdr_0_set(uint8_t port_id, uint64_t e2ecc_data_hdr_0)
{
    uint64_t reg_e2ecc_data_hdr_0=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_e2ecc_data_hdr_0 = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, E2ECC_DATA_HDR_0, E2ECC_DATA_HDR_0, reg_e2ecc_data_hdr_0, (uint64_t)e2ecc_data_hdr_0);

    XPORT_XLMAC_INDIRECT_WRITE(port_id, XPORT_XLMAC_CORE, E2ECC_DATA_HDR_0, reg_e2ecc_data_hdr_0);

    return 0;
}

int ag_drv_xport_xlmac_core_e2ecc_data_hdr_0_get(uint8_t port_id, uint64_t *e2ecc_data_hdr_0)
{
    uint64_t reg_e2ecc_data_hdr_0=0;

#ifdef VALIDATE_PARMS
    if(!e2ecc_data_hdr_0)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, E2ECC_DATA_HDR_0, reg_e2ecc_data_hdr_0);

    *e2ecc_data_hdr_0 = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, E2ECC_DATA_HDR_0, E2ECC_DATA_HDR_0, reg_e2ecc_data_hdr_0);

    return 0;
}

int ag_drv_xport_xlmac_core_e2ecc_data_hdr_1_set(uint8_t port_id, uint64_t e2ecc_data_hdr_1)
{
    uint64_t reg_e2ecc_data_hdr_1=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_e2ecc_data_hdr_1 = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, E2ECC_DATA_HDR_1, E2ECC_DATA_HDR_1, reg_e2ecc_data_hdr_1, (uint64_t)e2ecc_data_hdr_1);

    XPORT_XLMAC_INDIRECT_WRITE(port_id, XPORT_XLMAC_CORE, E2ECC_DATA_HDR_1, reg_e2ecc_data_hdr_1);

    return 0;
}

int ag_drv_xport_xlmac_core_e2ecc_data_hdr_1_get(uint8_t port_id, uint64_t *e2ecc_data_hdr_1)
{
    uint64_t reg_e2ecc_data_hdr_1=0;

#ifdef VALIDATE_PARMS
    if(!e2ecc_data_hdr_1)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, E2ECC_DATA_HDR_1, reg_e2ecc_data_hdr_1);

    *e2ecc_data_hdr_1 = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, E2ECC_DATA_HDR_1, E2ECC_DATA_HDR_1, reg_e2ecc_data_hdr_1);

    return 0;
}

int ag_drv_xport_xlmac_core_e2efc_module_hdr_0_set(uint8_t port_id, uint64_t e2efc_module_hdr_0)
{
    uint64_t reg_e2efc_module_hdr_0=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_e2efc_module_hdr_0 = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, E2EFC_MODULE_HDR_0, E2EFC_MODULE_HDR_0, reg_e2efc_module_hdr_0, (uint64_t)e2efc_module_hdr_0);

    XPORT_XLMAC_INDIRECT_WRITE(port_id, XPORT_XLMAC_CORE, E2EFC_MODULE_HDR_0, reg_e2efc_module_hdr_0);

    return 0;
}

int ag_drv_xport_xlmac_core_e2efc_module_hdr_0_get(uint8_t port_id, uint64_t *e2efc_module_hdr_0)
{
    uint64_t reg_e2efc_module_hdr_0=0;

#ifdef VALIDATE_PARMS
    if(!e2efc_module_hdr_0)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, E2EFC_MODULE_HDR_0, reg_e2efc_module_hdr_0);

    *e2efc_module_hdr_0 = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, E2EFC_MODULE_HDR_0, E2EFC_MODULE_HDR_0, reg_e2efc_module_hdr_0);

    return 0;
}

int ag_drv_xport_xlmac_core_e2efc_module_hdr_1_set(uint8_t port_id, uint64_t e2efc_module_hdr_1)
{
    uint64_t reg_e2efc_module_hdr_1=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_e2efc_module_hdr_1 = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, E2EFC_MODULE_HDR_1, E2EFC_MODULE_HDR_1, reg_e2efc_module_hdr_1, (uint64_t)e2efc_module_hdr_1);

    XPORT_XLMAC_INDIRECT_WRITE(port_id, XPORT_XLMAC_CORE, E2EFC_MODULE_HDR_1, reg_e2efc_module_hdr_1);

    return 0;
}

int ag_drv_xport_xlmac_core_e2efc_module_hdr_1_get(uint8_t port_id, uint64_t *e2efc_module_hdr_1)
{
    uint64_t reg_e2efc_module_hdr_1=0;

#ifdef VALIDATE_PARMS
    if(!e2efc_module_hdr_1)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, E2EFC_MODULE_HDR_1, reg_e2efc_module_hdr_1);

    *e2efc_module_hdr_1 = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, E2EFC_MODULE_HDR_1, E2EFC_MODULE_HDR_1, reg_e2efc_module_hdr_1);

    return 0;
}

int ag_drv_xport_xlmac_core_e2efc_data_hdr_0_set(uint8_t port_id, uint64_t e2efc_data_hdr_0)
{
    uint64_t reg_e2efc_data_hdr_0=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_e2efc_data_hdr_0 = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, E2EFC_DATA_HDR_0, E2EFC_DATA_HDR_0, reg_e2efc_data_hdr_0, (uint64_t)e2efc_data_hdr_0);

    XPORT_XLMAC_INDIRECT_WRITE(port_id, XPORT_XLMAC_CORE, E2EFC_DATA_HDR_0, reg_e2efc_data_hdr_0);

    return 0;
}

int ag_drv_xport_xlmac_core_e2efc_data_hdr_0_get(uint8_t port_id, uint64_t *e2efc_data_hdr_0)
{
    uint64_t reg_e2efc_data_hdr_0=0;

#ifdef VALIDATE_PARMS
    if(!e2efc_data_hdr_0)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, E2EFC_DATA_HDR_0, reg_e2efc_data_hdr_0);

    *e2efc_data_hdr_0 = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, E2EFC_DATA_HDR_0, E2EFC_DATA_HDR_0, reg_e2efc_data_hdr_0);

    return 0;
}

int ag_drv_xport_xlmac_core_e2efc_data_hdr_1_set(uint8_t port_id, uint64_t e2efc_data_hdr_1)
{
    uint64_t reg_e2efc_data_hdr_1=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_e2efc_data_hdr_1 = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, E2EFC_DATA_HDR_1, E2EFC_DATA_HDR_1, reg_e2efc_data_hdr_1, (uint64_t)e2efc_data_hdr_1);

    XPORT_XLMAC_INDIRECT_WRITE(port_id, XPORT_XLMAC_CORE, E2EFC_DATA_HDR_1, reg_e2efc_data_hdr_1);

    return 0;
}

int ag_drv_xport_xlmac_core_e2efc_data_hdr_1_get(uint8_t port_id, uint64_t *e2efc_data_hdr_1)
{
    uint64_t reg_e2efc_data_hdr_1=0;

#ifdef VALIDATE_PARMS
    if(!e2efc_data_hdr_1)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, E2EFC_DATA_HDR_1, reg_e2efc_data_hdr_1);

    *e2efc_data_hdr_1 = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, E2EFC_DATA_HDR_1, E2EFC_DATA_HDR_1, reg_e2efc_data_hdr_1);

    return 0;
}

int ag_drv_xport_xlmac_core_txfifo_cell_cnt_get(uint8_t port_id, uint8_t *cell_cnt)
{
    uint64_t reg_txfifo_cell_cnt=0;

#ifdef VALIDATE_PARMS
    if(!cell_cnt)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, TXFIFO_CELL_CNT, reg_txfifo_cell_cnt);

    *cell_cnt = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, TXFIFO_CELL_CNT, CELL_CNT, reg_txfifo_cell_cnt);

    return 0;
}

int ag_drv_xport_xlmac_core_txfifo_cell_req_cnt_get(uint8_t port_id, uint8_t *req_cnt)
{
    uint64_t reg_txfifo_cell_req_cnt=0;

#ifdef VALIDATE_PARMS
    if(!req_cnt)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, TXFIFO_CELL_REQ_CNT, reg_txfifo_cell_req_cnt);

    *req_cnt = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, TXFIFO_CELL_REQ_CNT, REQ_CNT, reg_txfifo_cell_req_cnt);

    return 0;
}

int ag_drv_xport_xlmac_core_mem_ctrl_set(uint8_t port_id, uint16_t tx_cdc_mem_ctrl_tm, uint16_t rx_cdc_mem_ctrl_tm)
{
    uint64_t reg_mem_ctrl=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (tx_cdc_mem_ctrl_tm >= _12BITS_MAX_VAL_) ||
       (rx_cdc_mem_ctrl_tm >= _12BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_mem_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, MEM_CTRL, TX_CDC_MEM_CTRL_TM, reg_mem_ctrl, (uint64_t)tx_cdc_mem_ctrl_tm);
    reg_mem_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, MEM_CTRL, RX_CDC_MEM_CTRL_TM, reg_mem_ctrl, (uint64_t)rx_cdc_mem_ctrl_tm);

    XPORT_XLMAC_INDIRECT_WRITE(port_id, XPORT_XLMAC_CORE, MEM_CTRL, reg_mem_ctrl);

    return 0;
}

int ag_drv_xport_xlmac_core_mem_ctrl_get(uint8_t port_id, uint16_t *tx_cdc_mem_ctrl_tm, uint16_t *rx_cdc_mem_ctrl_tm)
{
    uint64_t reg_mem_ctrl=0;

#ifdef VALIDATE_PARMS
    if(!tx_cdc_mem_ctrl_tm || !rx_cdc_mem_ctrl_tm)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, MEM_CTRL, reg_mem_ctrl);

    *tx_cdc_mem_ctrl_tm = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, MEM_CTRL, TX_CDC_MEM_CTRL_TM, reg_mem_ctrl);
    *rx_cdc_mem_ctrl_tm = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, MEM_CTRL, RX_CDC_MEM_CTRL_TM, reg_mem_ctrl);

    return 0;
}

int ag_drv_xport_xlmac_core_ecc_ctrl_set(uint8_t port_id, uint8_t tx_cdc_ecc_ctrl_en, uint8_t rx_cdc_ecc_ctrl_en)
{
    uint64_t reg_ecc_ctrl=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (tx_cdc_ecc_ctrl_en >= _1BITS_MAX_VAL_) ||
       (rx_cdc_ecc_ctrl_en >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_ecc_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, ECC_CTRL, TX_CDC_ECC_CTRL_EN, reg_ecc_ctrl, (uint64_t)tx_cdc_ecc_ctrl_en);
    reg_ecc_ctrl = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, ECC_CTRL, RX_CDC_ECC_CTRL_EN, reg_ecc_ctrl, (uint64_t)rx_cdc_ecc_ctrl_en);

    XPORT_XLMAC_INDIRECT_WRITE(port_id, XPORT_XLMAC_CORE, ECC_CTRL, reg_ecc_ctrl);

    return 0;
}

int ag_drv_xport_xlmac_core_ecc_ctrl_get(uint8_t port_id, uint8_t *tx_cdc_ecc_ctrl_en, uint8_t *rx_cdc_ecc_ctrl_en)
{
    uint64_t reg_ecc_ctrl=0;

#ifdef VALIDATE_PARMS
    if(!tx_cdc_ecc_ctrl_en || !rx_cdc_ecc_ctrl_en)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, ECC_CTRL, reg_ecc_ctrl);

    *tx_cdc_ecc_ctrl_en = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, ECC_CTRL, TX_CDC_ECC_CTRL_EN, reg_ecc_ctrl);
    *rx_cdc_ecc_ctrl_en = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, ECC_CTRL, RX_CDC_ECC_CTRL_EN, reg_ecc_ctrl);

    return 0;
}

int ag_drv_xport_xlmac_core_ecc_force_double_bit_err_set(uint8_t port_id, uint8_t tx_cdc_force_double_bit_err, uint8_t rx_cdc_force_double_bit_err)
{
    uint64_t reg_ecc_force_double_bit_err=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (tx_cdc_force_double_bit_err >= _1BITS_MAX_VAL_) ||
       (rx_cdc_force_double_bit_err >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_ecc_force_double_bit_err = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, ECC_FORCE_DOUBLE_BIT_ERR, TX_CDC_FORCE_DOUBLE_BIT_ERR, reg_ecc_force_double_bit_err, (uint64_t)tx_cdc_force_double_bit_err);
    reg_ecc_force_double_bit_err = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, ECC_FORCE_DOUBLE_BIT_ERR, RX_CDC_FORCE_DOUBLE_BIT_ERR, reg_ecc_force_double_bit_err, (uint64_t)rx_cdc_force_double_bit_err);

    XPORT_XLMAC_INDIRECT_WRITE(port_id, XPORT_XLMAC_CORE, ECC_FORCE_DOUBLE_BIT_ERR, reg_ecc_force_double_bit_err);

    return 0;
}

int ag_drv_xport_xlmac_core_ecc_force_double_bit_err_get(uint8_t port_id, uint8_t *tx_cdc_force_double_bit_err, uint8_t *rx_cdc_force_double_bit_err)
{
    uint64_t reg_ecc_force_double_bit_err=0;

#ifdef VALIDATE_PARMS
    if(!tx_cdc_force_double_bit_err || !rx_cdc_force_double_bit_err)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, ECC_FORCE_DOUBLE_BIT_ERR, reg_ecc_force_double_bit_err);

    *tx_cdc_force_double_bit_err = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, ECC_FORCE_DOUBLE_BIT_ERR, TX_CDC_FORCE_DOUBLE_BIT_ERR, reg_ecc_force_double_bit_err);
    *rx_cdc_force_double_bit_err = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, ECC_FORCE_DOUBLE_BIT_ERR, RX_CDC_FORCE_DOUBLE_BIT_ERR, reg_ecc_force_double_bit_err);

    return 0;
}

int ag_drv_xport_xlmac_core_ecc_force_single_bit_err_set(uint8_t port_id, uint8_t tx_cdc_force_single_bit_err, uint8_t rx_cdc_force_single_bit_err)
{
    uint64_t reg_ecc_force_single_bit_err=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (tx_cdc_force_single_bit_err >= _1BITS_MAX_VAL_) ||
       (rx_cdc_force_single_bit_err >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_ecc_force_single_bit_err = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, ECC_FORCE_SINGLE_BIT_ERR, TX_CDC_FORCE_SINGLE_BIT_ERR, reg_ecc_force_single_bit_err, (uint64_t)tx_cdc_force_single_bit_err);
    reg_ecc_force_single_bit_err = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, ECC_FORCE_SINGLE_BIT_ERR, RX_CDC_FORCE_SINGLE_BIT_ERR, reg_ecc_force_single_bit_err, (uint64_t)rx_cdc_force_single_bit_err);

    XPORT_XLMAC_INDIRECT_WRITE(port_id, XPORT_XLMAC_CORE, ECC_FORCE_SINGLE_BIT_ERR, reg_ecc_force_single_bit_err);

    return 0;
}

int ag_drv_xport_xlmac_core_ecc_force_single_bit_err_get(uint8_t port_id, uint8_t *tx_cdc_force_single_bit_err, uint8_t *rx_cdc_force_single_bit_err)
{
    uint64_t reg_ecc_force_single_bit_err=0;

#ifdef VALIDATE_PARMS
    if(!tx_cdc_force_single_bit_err || !rx_cdc_force_single_bit_err)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, ECC_FORCE_SINGLE_BIT_ERR, reg_ecc_force_single_bit_err);

    *tx_cdc_force_single_bit_err = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, ECC_FORCE_SINGLE_BIT_ERR, TX_CDC_FORCE_SINGLE_BIT_ERR, reg_ecc_force_single_bit_err);
    *rx_cdc_force_single_bit_err = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, ECC_FORCE_SINGLE_BIT_ERR, RX_CDC_FORCE_SINGLE_BIT_ERR, reg_ecc_force_single_bit_err);

    return 0;
}

int ag_drv_xport_xlmac_core_rx_cdc_ecc_status_get(uint8_t port_id, uint8_t *rx_cdc_double_bit_err, uint8_t *rx_cdc_single_bit_err)
{
    uint64_t reg_rx_cdc_ecc_status=0;

#ifdef VALIDATE_PARMS
    if(!rx_cdc_double_bit_err || !rx_cdc_single_bit_err)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, RX_CDC_ECC_STATUS, reg_rx_cdc_ecc_status);

    *rx_cdc_double_bit_err = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, RX_CDC_ECC_STATUS, RX_CDC_DOUBLE_BIT_ERR, reg_rx_cdc_ecc_status);
    *rx_cdc_single_bit_err = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, RX_CDC_ECC_STATUS, RX_CDC_SINGLE_BIT_ERR, reg_rx_cdc_ecc_status);

    return 0;
}

int ag_drv_xport_xlmac_core_tx_cdc_ecc_status_get(uint8_t port_id, uint8_t *tx_cdc_double_bit_err, uint8_t *tx_cdc_single_bit_err)
{
    uint64_t reg_tx_cdc_ecc_status=0;

#ifdef VALIDATE_PARMS
    if(!tx_cdc_double_bit_err || !tx_cdc_single_bit_err)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, TX_CDC_ECC_STATUS, reg_tx_cdc_ecc_status);

    *tx_cdc_double_bit_err = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, TX_CDC_ECC_STATUS, TX_CDC_DOUBLE_BIT_ERR, reg_tx_cdc_ecc_status);
    *tx_cdc_single_bit_err = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, TX_CDC_ECC_STATUS, TX_CDC_SINGLE_BIT_ERR, reg_tx_cdc_ecc_status);

    return 0;
}

int ag_drv_xport_xlmac_core_clear_ecc_status_set(uint8_t port_id, uint8_t clear_tx_cdc_double_bit_err, uint8_t clear_tx_cdc_single_bit_err, uint8_t clear_rx_cdc_double_bit_err, uint8_t clear_rx_cdc_single_bit_err)
{
    uint64_t reg_clear_ecc_status=0;

#ifdef VALIDATE_PARMS
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (clear_tx_cdc_double_bit_err >= _1BITS_MAX_VAL_) ||
       (clear_tx_cdc_single_bit_err >= _1BITS_MAX_VAL_) ||
       (clear_rx_cdc_double_bit_err >= _1BITS_MAX_VAL_) ||
       (clear_rx_cdc_single_bit_err >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_clear_ecc_status = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, CLEAR_ECC_STATUS, CLEAR_TX_CDC_DOUBLE_BIT_ERR, reg_clear_ecc_status, (uint64_t)clear_tx_cdc_double_bit_err);
    reg_clear_ecc_status = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, CLEAR_ECC_STATUS, CLEAR_TX_CDC_SINGLE_BIT_ERR, reg_clear_ecc_status, (uint64_t)clear_tx_cdc_single_bit_err);
    reg_clear_ecc_status = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, CLEAR_ECC_STATUS, CLEAR_RX_CDC_DOUBLE_BIT_ERR, reg_clear_ecc_status, (uint64_t)clear_rx_cdc_double_bit_err);
    reg_clear_ecc_status = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, CLEAR_ECC_STATUS, CLEAR_RX_CDC_SINGLE_BIT_ERR, reg_clear_ecc_status, (uint64_t)clear_rx_cdc_single_bit_err);

    XPORT_XLMAC_INDIRECT_WRITE(port_id, XPORT_XLMAC_CORE, CLEAR_ECC_STATUS, reg_clear_ecc_status);

    return 0;
}

int ag_drv_xport_xlmac_core_clear_ecc_status_get(uint8_t port_id, uint8_t *clear_tx_cdc_double_bit_err, uint8_t *clear_tx_cdc_single_bit_err, uint8_t *clear_rx_cdc_double_bit_err, uint8_t *clear_rx_cdc_single_bit_err)
{
    uint64_t reg_clear_ecc_status=0;

#ifdef VALIDATE_PARMS
    if(!clear_tx_cdc_double_bit_err || !clear_tx_cdc_single_bit_err || !clear_rx_cdc_double_bit_err || !clear_rx_cdc_single_bit_err)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, CLEAR_ECC_STATUS, reg_clear_ecc_status);

    *clear_tx_cdc_double_bit_err = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, CLEAR_ECC_STATUS, CLEAR_TX_CDC_DOUBLE_BIT_ERR, reg_clear_ecc_status);
    *clear_tx_cdc_single_bit_err = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, CLEAR_ECC_STATUS, CLEAR_TX_CDC_SINGLE_BIT_ERR, reg_clear_ecc_status);
    *clear_rx_cdc_double_bit_err = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, CLEAR_ECC_STATUS, CLEAR_RX_CDC_DOUBLE_BIT_ERR, reg_clear_ecc_status);
    *clear_rx_cdc_single_bit_err = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, CLEAR_ECC_STATUS, CLEAR_RX_CDC_SINGLE_BIT_ERR, reg_clear_ecc_status);

    return 0;
}

int ag_drv_xport_xlmac_core_intr_status_get(uint8_t port_id, xport_xlmac_core_intr_status *intr_status)
{
    uint64_t reg_intr_status=0;

#ifdef VALIDATE_PARMS
    if(!intr_status)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, INTR_STATUS, reg_intr_status);

    intr_status->sum_ts_entry_valid = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, INTR_STATUS, SUM_TS_ENTRY_VALID, reg_intr_status);
    intr_status->sum_link_interruption_status = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, INTR_STATUS, SUM_LINK_INTERRUPTION_STATUS, reg_intr_status);
    intr_status->sum_remote_fault_status = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, INTR_STATUS, SUM_REMOTE_FAULT_STATUS, reg_intr_status);
    intr_status->sum_local_fault_status = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, INTR_STATUS, SUM_LOCAL_FAULT_STATUS, reg_intr_status);
    intr_status->sum_rx_cdc_double_bit_err = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, INTR_STATUS, SUM_RX_CDC_DOUBLE_BIT_ERR, reg_intr_status);
    intr_status->sum_rx_cdc_single_bit_err = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, INTR_STATUS, SUM_RX_CDC_SINGLE_BIT_ERR, reg_intr_status);
    intr_status->sum_tx_cdc_double_bit_err = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, INTR_STATUS, SUM_TX_CDC_DOUBLE_BIT_ERR, reg_intr_status);
    intr_status->sum_tx_cdc_single_bit_err = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, INTR_STATUS, SUM_TX_CDC_SINGLE_BIT_ERR, reg_intr_status);
    intr_status->sum_rx_msg_overflow = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, INTR_STATUS, SUM_RX_MSG_OVERFLOW, reg_intr_status);
    intr_status->sum_rx_pkt_overflow = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, INTR_STATUS, SUM_RX_PKT_OVERFLOW, reg_intr_status);
    intr_status->sum_tx_ts_fifo_overflow = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, INTR_STATUS, SUM_TX_TS_FIFO_OVERFLOW, reg_intr_status);
    intr_status->sum_tx_llfc_msg_overflow = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, INTR_STATUS, SUM_TX_LLFC_MSG_OVERFLOW, reg_intr_status);
    intr_status->sum_tx_pkt_overflow = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, INTR_STATUS, SUM_TX_PKT_OVERFLOW, reg_intr_status);
    intr_status->sum_tx_pkt_underflow = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, INTR_STATUS, SUM_TX_PKT_UNDERFLOW, reg_intr_status);

    return 0;
}

int ag_drv_xport_xlmac_core_intr_enable_set(uint8_t port_id, const xport_xlmac_core_intr_enable *intr_enable)
{
    uint64_t reg_intr_enable=0;

#ifdef VALIDATE_PARMS
    if(!intr_enable)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((port_id >= BLOCK_ADDR_COUNT) ||
       (intr_enable->en_ts_entry_valid >= _1BITS_MAX_VAL_) ||
       (intr_enable->en_link_interruption_status >= _1BITS_MAX_VAL_) ||
       (intr_enable->en_remote_fault_status >= _1BITS_MAX_VAL_) ||
       (intr_enable->en_local_fault_status >= _1BITS_MAX_VAL_) ||
       (intr_enable->en_rx_cdc_double_bit_err >= _1BITS_MAX_VAL_) ||
       (intr_enable->en_rx_cdc_single_bit_err >= _1BITS_MAX_VAL_) ||
       (intr_enable->en_tx_cdc_double_bit_err >= _1BITS_MAX_VAL_) ||
       (intr_enable->en_tx_cdc_single_bit_err >= _1BITS_MAX_VAL_) ||
       (intr_enable->en_rx_msg_overflow >= _1BITS_MAX_VAL_) ||
       (intr_enable->en_rx_pkt_overflow >= _1BITS_MAX_VAL_) ||
       (intr_enable->en_tx_ts_fifo_overflow >= _1BITS_MAX_VAL_) ||
       (intr_enable->en_tx_llfc_msg_overflow >= _1BITS_MAX_VAL_) ||
       (intr_enable->en_tx_pkt_overflow >= _1BITS_MAX_VAL_) ||
       (intr_enable->en_tx_pkt_underflow >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_intr_enable = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, INTR_ENABLE, EN_TS_ENTRY_VALID, reg_intr_enable, (uint64_t)intr_enable->en_ts_entry_valid);
    reg_intr_enable = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, INTR_ENABLE, EN_LINK_INTERRUPTION_STATUS, reg_intr_enable, (uint64_t)intr_enable->en_link_interruption_status);
    reg_intr_enable = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, INTR_ENABLE, EN_REMOTE_FAULT_STATUS, reg_intr_enable, (uint64_t)intr_enable->en_remote_fault_status);
    reg_intr_enable = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, INTR_ENABLE, EN_LOCAL_FAULT_STATUS, reg_intr_enable, (uint64_t)intr_enable->en_local_fault_status);
    reg_intr_enable = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, INTR_ENABLE, EN_RX_CDC_DOUBLE_BIT_ERR, reg_intr_enable, (uint64_t)intr_enable->en_rx_cdc_double_bit_err);
    reg_intr_enable = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, INTR_ENABLE, EN_RX_CDC_SINGLE_BIT_ERR, reg_intr_enable, (uint64_t)intr_enable->en_rx_cdc_single_bit_err);
    reg_intr_enable = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, INTR_ENABLE, EN_TX_CDC_DOUBLE_BIT_ERR, reg_intr_enable, (uint64_t)intr_enable->en_tx_cdc_double_bit_err);
    reg_intr_enable = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, INTR_ENABLE, EN_TX_CDC_SINGLE_BIT_ERR, reg_intr_enable, (uint64_t)intr_enable->en_tx_cdc_single_bit_err);
    reg_intr_enable = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, INTR_ENABLE, EN_RX_MSG_OVERFLOW, reg_intr_enable, (uint64_t)intr_enable->en_rx_msg_overflow);
    reg_intr_enable = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, INTR_ENABLE, EN_RX_PKT_OVERFLOW, reg_intr_enable, (uint64_t)intr_enable->en_rx_pkt_overflow);
    reg_intr_enable = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, INTR_ENABLE, EN_TX_TS_FIFO_OVERFLOW, reg_intr_enable, (uint64_t)intr_enable->en_tx_ts_fifo_overflow);
    reg_intr_enable = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, INTR_ENABLE, EN_TX_LLFC_MSG_OVERFLOW, reg_intr_enable, (uint64_t)intr_enable->en_tx_llfc_msg_overflow);
    reg_intr_enable = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, INTR_ENABLE, EN_TX_PKT_OVERFLOW, reg_intr_enable, (uint64_t)intr_enable->en_tx_pkt_overflow);
    reg_intr_enable = RU_FIELD_SET(port_id, XPORT_XLMAC_CORE, INTR_ENABLE, EN_TX_PKT_UNDERFLOW, reg_intr_enable, (uint64_t)intr_enable->en_tx_pkt_underflow);

    XPORT_XLMAC_INDIRECT_WRITE(port_id, XPORT_XLMAC_CORE, INTR_ENABLE, reg_intr_enable);

    return 0;
}

int ag_drv_xport_xlmac_core_intr_enable_get(uint8_t port_id, xport_xlmac_core_intr_enable *intr_enable)
{
    uint64_t reg_intr_enable=0;

#ifdef VALIDATE_PARMS
    if(!intr_enable)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, INTR_ENABLE, reg_intr_enable);

    intr_enable->en_ts_entry_valid = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, INTR_ENABLE, EN_TS_ENTRY_VALID, reg_intr_enable);
    intr_enable->en_link_interruption_status = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, INTR_ENABLE, EN_LINK_INTERRUPTION_STATUS, reg_intr_enable);
    intr_enable->en_remote_fault_status = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, INTR_ENABLE, EN_REMOTE_FAULT_STATUS, reg_intr_enable);
    intr_enable->en_local_fault_status = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, INTR_ENABLE, EN_LOCAL_FAULT_STATUS, reg_intr_enable);
    intr_enable->en_rx_cdc_double_bit_err = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, INTR_ENABLE, EN_RX_CDC_DOUBLE_BIT_ERR, reg_intr_enable);
    intr_enable->en_rx_cdc_single_bit_err = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, INTR_ENABLE, EN_RX_CDC_SINGLE_BIT_ERR, reg_intr_enable);
    intr_enable->en_tx_cdc_double_bit_err = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, INTR_ENABLE, EN_TX_CDC_DOUBLE_BIT_ERR, reg_intr_enable);
    intr_enable->en_tx_cdc_single_bit_err = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, INTR_ENABLE, EN_TX_CDC_SINGLE_BIT_ERR, reg_intr_enable);
    intr_enable->en_rx_msg_overflow = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, INTR_ENABLE, EN_RX_MSG_OVERFLOW, reg_intr_enable);
    intr_enable->en_rx_pkt_overflow = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, INTR_ENABLE, EN_RX_PKT_OVERFLOW, reg_intr_enable);
    intr_enable->en_tx_ts_fifo_overflow = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, INTR_ENABLE, EN_TX_TS_FIFO_OVERFLOW, reg_intr_enable);
    intr_enable->en_tx_llfc_msg_overflow = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, INTR_ENABLE, EN_TX_LLFC_MSG_OVERFLOW, reg_intr_enable);
    intr_enable->en_tx_pkt_overflow = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, INTR_ENABLE, EN_TX_PKT_OVERFLOW, reg_intr_enable);
    intr_enable->en_tx_pkt_underflow = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, INTR_ENABLE, EN_TX_PKT_UNDERFLOW, reg_intr_enable);

    return 0;
}

int ag_drv_xport_xlmac_core_version_id_get(uint8_t port_id, uint16_t *xlmac_version)
{
    uint64_t reg_version_id=0;

#ifdef VALIDATE_PARMS
    if(!xlmac_version)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((port_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    XPORT_XLMAC_INDIRECT_READ(port_id, XPORT_XLMAC_CORE, VERSION_ID, reg_version_id);

    *xlmac_version = RU_FIELD_GET(port_id, XPORT_XLMAC_CORE, VERSION_ID, XLMAC_VERSION, reg_version_id);

    return 0;
}

