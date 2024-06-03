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
#include "bcm63158_xport_mab_ag.h"
int ag_drv_xport_mab_ctrl_set(const xport_mab_ctrl *ctrl)
{
    uint32_t reg_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!ctrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((ctrl->link_down_rst_en >= _1BITS_MAX_VAL_) ||
       (ctrl->xgmii_tx_rst >= _1BITS_MAX_VAL_) ||
       (ctrl->gmii_tx_rst >= _4BITS_MAX_VAL_) ||
       (ctrl->xgmii_rx_rst >= _1BITS_MAX_VAL_) ||
       (ctrl->gmii_rx_rst >= _4BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_cntrl = RU_FIELD_SET(0, XPORT_MAB, CNTRL, LINK_DOWN_RST_EN, reg_cntrl, ctrl->link_down_rst_en);
    reg_cntrl = RU_FIELD_SET(0, XPORT_MAB, CNTRL, XGMII_TX_RST, reg_cntrl, ctrl->xgmii_tx_rst);
    reg_cntrl = RU_FIELD_SET(0, XPORT_MAB, CNTRL, GMII_TX_RST, reg_cntrl, ctrl->gmii_tx_rst);
    reg_cntrl = RU_FIELD_SET(0, XPORT_MAB, CNTRL, XGMII_RX_RST, reg_cntrl, ctrl->xgmii_rx_rst);
    reg_cntrl = RU_FIELD_SET(0, XPORT_MAB, CNTRL, GMII_RX_RST, reg_cntrl, ctrl->gmii_rx_rst);

    RU_REG_WRITE(0, XPORT_MAB, CNTRL, reg_cntrl);

    return 0;
}

int ag_drv_xport_mab_ctrl_get(xport_mab_ctrl *ctrl)
{
    uint32_t reg_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!ctrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_MAB, CNTRL, reg_cntrl);

    ctrl->link_down_rst_en = RU_FIELD_GET(0, XPORT_MAB, CNTRL, LINK_DOWN_RST_EN, reg_cntrl);
    ctrl->xgmii_tx_rst = RU_FIELD_GET(0, XPORT_MAB, CNTRL, XGMII_TX_RST, reg_cntrl);
    ctrl->gmii_tx_rst = RU_FIELD_GET(0, XPORT_MAB, CNTRL, GMII_TX_RST, reg_cntrl);
    ctrl->xgmii_rx_rst = RU_FIELD_GET(0, XPORT_MAB, CNTRL, XGMII_RX_RST, reg_cntrl);
    ctrl->gmii_rx_rst = RU_FIELD_GET(0, XPORT_MAB, CNTRL, GMII_RX_RST, reg_cntrl);

    return 0;
}

int ag_drv_xport_mab_tx_wrr_ctrl_set(const xport_mab_tx_wrr_ctrl *tx_wrr_ctrl)
{
    uint32_t reg_tx_wrr_ctrl=0;

#ifdef VALIDATE_PARMS
    if(!tx_wrr_ctrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((tx_wrr_ctrl->arb_mode >= _1BITS_MAX_VAL_) ||
       (tx_wrr_ctrl->p3_weight >= _4BITS_MAX_VAL_) ||
       (tx_wrr_ctrl->p2_weight >= _4BITS_MAX_VAL_) ||
       (tx_wrr_ctrl->p1_weight >= _4BITS_MAX_VAL_) ||
       (tx_wrr_ctrl->p0_weight >= _4BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_tx_wrr_ctrl = RU_FIELD_SET(0, XPORT_MAB, TX_WRR_CTRL, ARB_MODE, reg_tx_wrr_ctrl, tx_wrr_ctrl->arb_mode);
    reg_tx_wrr_ctrl = RU_FIELD_SET(0, XPORT_MAB, TX_WRR_CTRL, P3_WEIGHT, reg_tx_wrr_ctrl, tx_wrr_ctrl->p3_weight);
    reg_tx_wrr_ctrl = RU_FIELD_SET(0, XPORT_MAB, TX_WRR_CTRL, P2_WEIGHT, reg_tx_wrr_ctrl, tx_wrr_ctrl->p2_weight);
    reg_tx_wrr_ctrl = RU_FIELD_SET(0, XPORT_MAB, TX_WRR_CTRL, P1_WEIGHT, reg_tx_wrr_ctrl, tx_wrr_ctrl->p1_weight);
    reg_tx_wrr_ctrl = RU_FIELD_SET(0, XPORT_MAB, TX_WRR_CTRL, P0_WEIGHT, reg_tx_wrr_ctrl, tx_wrr_ctrl->p0_weight);

    RU_REG_WRITE(0, XPORT_MAB, TX_WRR_CTRL, reg_tx_wrr_ctrl);

    return 0;
}

int ag_drv_xport_mab_tx_wrr_ctrl_get(xport_mab_tx_wrr_ctrl *tx_wrr_ctrl)
{
    uint32_t reg_tx_wrr_ctrl=0;

#ifdef VALIDATE_PARMS
    if(!tx_wrr_ctrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_MAB, TX_WRR_CTRL, reg_tx_wrr_ctrl);

    tx_wrr_ctrl->arb_mode = RU_FIELD_GET(0, XPORT_MAB, TX_WRR_CTRL, ARB_MODE, reg_tx_wrr_ctrl);
    tx_wrr_ctrl->p3_weight = RU_FIELD_GET(0, XPORT_MAB, TX_WRR_CTRL, P3_WEIGHT, reg_tx_wrr_ctrl);
    tx_wrr_ctrl->p2_weight = RU_FIELD_GET(0, XPORT_MAB, TX_WRR_CTRL, P2_WEIGHT, reg_tx_wrr_ctrl);
    tx_wrr_ctrl->p1_weight = RU_FIELD_GET(0, XPORT_MAB, TX_WRR_CTRL, P1_WEIGHT, reg_tx_wrr_ctrl);
    tx_wrr_ctrl->p0_weight = RU_FIELD_GET(0, XPORT_MAB, TX_WRR_CTRL, P0_WEIGHT, reg_tx_wrr_ctrl);

    return 0;
}

int ag_drv_xport_mab_tx_threshold_set(const xport_mab_tx_threshold *tx_threshold)
{
    uint32_t reg_tx_threshold=0;

#ifdef VALIDATE_PARMS
    if(!tx_threshold)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((tx_threshold->xgmii0_tx_threshold >= _4BITS_MAX_VAL_) ||
       (tx_threshold->gmii3_tx_threshold >= _4BITS_MAX_VAL_) ||
       (tx_threshold->gmii2_tx_threshold >= _4BITS_MAX_VAL_) ||
       (tx_threshold->gmii1_tx_threshold >= _4BITS_MAX_VAL_) ||
       (tx_threshold->gmii0_tx_threshold >= _4BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_tx_threshold = RU_FIELD_SET(0, XPORT_MAB, TX_THRESHOLD, XGMII0_TX_THRESHOLD, reg_tx_threshold, tx_threshold->xgmii0_tx_threshold);
    reg_tx_threshold = RU_FIELD_SET(0, XPORT_MAB, TX_THRESHOLD, GMII3_TX_THRESHOLD, reg_tx_threshold, tx_threshold->gmii3_tx_threshold);
    reg_tx_threshold = RU_FIELD_SET(0, XPORT_MAB, TX_THRESHOLD, GMII2_TX_THRESHOLD, reg_tx_threshold, tx_threshold->gmii2_tx_threshold);
    reg_tx_threshold = RU_FIELD_SET(0, XPORT_MAB, TX_THRESHOLD, GMII1_TX_THRESHOLD, reg_tx_threshold, tx_threshold->gmii1_tx_threshold);
    reg_tx_threshold = RU_FIELD_SET(0, XPORT_MAB, TX_THRESHOLD, GMII0_TX_THRESHOLD, reg_tx_threshold, tx_threshold->gmii0_tx_threshold);

    RU_REG_WRITE(0, XPORT_MAB, TX_THRESHOLD, reg_tx_threshold);

    return 0;
}

int ag_drv_xport_mab_tx_threshold_get(xport_mab_tx_threshold *tx_threshold)
{
    uint32_t reg_tx_threshold=0;

#ifdef VALIDATE_PARMS
    if(!tx_threshold)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_MAB, TX_THRESHOLD, reg_tx_threshold);

    tx_threshold->xgmii0_tx_threshold = RU_FIELD_GET(0, XPORT_MAB, TX_THRESHOLD, XGMII0_TX_THRESHOLD, reg_tx_threshold);
    tx_threshold->gmii3_tx_threshold = RU_FIELD_GET(0, XPORT_MAB, TX_THRESHOLD, GMII3_TX_THRESHOLD, reg_tx_threshold);
    tx_threshold->gmii2_tx_threshold = RU_FIELD_GET(0, XPORT_MAB, TX_THRESHOLD, GMII2_TX_THRESHOLD, reg_tx_threshold);
    tx_threshold->gmii1_tx_threshold = RU_FIELD_GET(0, XPORT_MAB, TX_THRESHOLD, GMII1_TX_THRESHOLD, reg_tx_threshold);
    tx_threshold->gmii0_tx_threshold = RU_FIELD_GET(0, XPORT_MAB, TX_THRESHOLD, GMII0_TX_THRESHOLD, reg_tx_threshold);

    return 0;
}

int ag_drv_xport_mab_link_down_tx_data_set(uint8_t txctl, uint8_t txd)
{
    uint32_t reg_link_down_tx_data=0;

#ifdef VALIDATE_PARMS
    if((txctl >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_link_down_tx_data = RU_FIELD_SET(0, XPORT_MAB, LINK_DOWN_TX_DATA, TXCTL, reg_link_down_tx_data, txctl);
    reg_link_down_tx_data = RU_FIELD_SET(0, XPORT_MAB, LINK_DOWN_TX_DATA, TXD, reg_link_down_tx_data, txd);

    RU_REG_WRITE(0, XPORT_MAB, LINK_DOWN_TX_DATA, reg_link_down_tx_data);

    return 0;
}

int ag_drv_xport_mab_link_down_tx_data_get(uint8_t *txctl, uint8_t *txd)
{
    uint32_t reg_link_down_tx_data=0;

#ifdef VALIDATE_PARMS
    if(!txctl || !txd)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_MAB, LINK_DOWN_TX_DATA, reg_link_down_tx_data);

    *txctl = RU_FIELD_GET(0, XPORT_MAB, LINK_DOWN_TX_DATA, TXCTL, reg_link_down_tx_data);
    *txd = RU_FIELD_GET(0, XPORT_MAB, LINK_DOWN_TX_DATA, TXD, reg_link_down_tx_data);

    return 0;
}

int ag_drv_xport_mab_status_get(xport_mab_status *status)
{
    uint32_t reg_status=0;

#ifdef VALIDATE_PARMS
    if(!status)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_MAB, STATUS, reg_status);

    status->xgmii_rx_afifo_overrun = RU_FIELD_GET(0, XPORT_MAB, STATUS, XGMII_RX_AFIFO_OVERRUN, reg_status);
    status->gmii_rx_afifo_overrun_vect = RU_FIELD_GET(0, XPORT_MAB, STATUS, GMII_RX_AFIFO_OVERRUN_VECT, reg_status);
    status->xgmii_tx_frm_underrun = RU_FIELD_GET(0, XPORT_MAB, STATUS, XGMII_TX_FRM_UNDERRUN, reg_status);
    status->xgmii_outstanding_credits_cnt_underrun = RU_FIELD_GET(0, XPORT_MAB, STATUS, XGMII_OUTSTANDING_CREDITS_CNT_UNDERRUN, reg_status);
    status->gmii_outstanding_credits_cnt_underrun_vect = RU_FIELD_GET(0, XPORT_MAB, STATUS, GMII_OUTSTANDING_CREDITS_CNT_UNDERRUN_VECT, reg_status);
    status->xgmii_tx_afifo_overrun = RU_FIELD_GET(0, XPORT_MAB, STATUS, XGMII_TX_AFIFO_OVERRUN, reg_status);
    status->gmii_tx_afifo_overrun_vect = RU_FIELD_GET(0, XPORT_MAB, STATUS, GMII_TX_AFIFO_OVERRUN_VECT, reg_status);

    return 0;
}

