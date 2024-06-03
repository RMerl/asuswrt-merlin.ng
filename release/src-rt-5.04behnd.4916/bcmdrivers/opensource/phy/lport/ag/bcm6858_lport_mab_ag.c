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

#include "bcm6858_drivers_lport_ag.h"
#include "bcm6858_lport_mab_ag.h"
#define BLOCK_ADDR_COUNT_BITS 1
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

int ag_drv_lport_mab_cntrl_set(uint8_t xlmac_id, const lport_mab_cntrl *cntrl)
{
    uint32_t reg_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!cntrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT) ||
       (cntrl->link_down_rst_en >= _1BITS_MAX_VAL_) ||
       (cntrl->xgmii_tx_rst >= _1BITS_MAX_VAL_) ||
       (cntrl->gmii_tx_rst >= _4BITS_MAX_VAL_) ||
       (cntrl->xgmii_rx_rst >= _1BITS_MAX_VAL_) ||
       (cntrl->gmii_rx_rst >= _4BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_cntrl = RU_FIELD_SET(xlmac_id, LPORT_MAB, CNTRL, LINK_DOWN_RST_EN, reg_cntrl, cntrl->link_down_rst_en);
    reg_cntrl = RU_FIELD_SET(xlmac_id, LPORT_MAB, CNTRL, XGMII_TX_RST, reg_cntrl, cntrl->xgmii_tx_rst);
    reg_cntrl = RU_FIELD_SET(xlmac_id, LPORT_MAB, CNTRL, GMII_TX_RST, reg_cntrl, cntrl->gmii_tx_rst);
    reg_cntrl = RU_FIELD_SET(xlmac_id, LPORT_MAB, CNTRL, XGMII_RX_RST, reg_cntrl, cntrl->xgmii_rx_rst);
    reg_cntrl = RU_FIELD_SET(xlmac_id, LPORT_MAB, CNTRL, GMII_RX_RST, reg_cntrl, cntrl->gmii_rx_rst);

    RU_REG_WRITE(xlmac_id, LPORT_MAB, CNTRL, reg_cntrl);

    return 0;
}

int ag_drv_lport_mab_cntrl_get(uint8_t xlmac_id, lport_mab_cntrl *cntrl)
{
    uint32_t reg_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!cntrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, LPORT_MAB, CNTRL, reg_cntrl);

    cntrl->link_down_rst_en = RU_FIELD_GET(xlmac_id, LPORT_MAB, CNTRL, LINK_DOWN_RST_EN, reg_cntrl);
    cntrl->xgmii_tx_rst = RU_FIELD_GET(xlmac_id, LPORT_MAB, CNTRL, XGMII_TX_RST, reg_cntrl);
    cntrl->gmii_tx_rst = RU_FIELD_GET(xlmac_id, LPORT_MAB, CNTRL, GMII_TX_RST, reg_cntrl);
    cntrl->xgmii_rx_rst = RU_FIELD_GET(xlmac_id, LPORT_MAB, CNTRL, XGMII_RX_RST, reg_cntrl);
    cntrl->gmii_rx_rst = RU_FIELD_GET(xlmac_id, LPORT_MAB, CNTRL, GMII_RX_RST, reg_cntrl);

    return 0;
}

int ag_drv_lport_mab_tx_wrr_ctrl_set(uint8_t xlmac_id, const lport_mab_tx_wrr_ctrl *tx_wrr_ctrl)
{
    uint32_t reg_tx_wrr_ctrl=0;

#ifdef VALIDATE_PARMS
    if(!tx_wrr_ctrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT) ||
       (tx_wrr_ctrl->arb_mode >= _1BITS_MAX_VAL_) ||
       (tx_wrr_ctrl->p7_weight >= _4BITS_MAX_VAL_) ||
       (tx_wrr_ctrl->p6_weight >= _4BITS_MAX_VAL_) ||
       (tx_wrr_ctrl->p5_weight >= _4BITS_MAX_VAL_) ||
       (tx_wrr_ctrl->p4_weight >= _4BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_tx_wrr_ctrl = RU_FIELD_SET(xlmac_id, LPORT_MAB, TX_WRR_CTRL, ARB_MODE, reg_tx_wrr_ctrl, tx_wrr_ctrl->arb_mode);
    reg_tx_wrr_ctrl = RU_FIELD_SET(xlmac_id, LPORT_MAB, TX_WRR_CTRL, P7_WEIGHT, reg_tx_wrr_ctrl, tx_wrr_ctrl->p7_weight);
    reg_tx_wrr_ctrl = RU_FIELD_SET(xlmac_id, LPORT_MAB, TX_WRR_CTRL, P6_WEIGHT, reg_tx_wrr_ctrl, tx_wrr_ctrl->p6_weight);
    reg_tx_wrr_ctrl = RU_FIELD_SET(xlmac_id, LPORT_MAB, TX_WRR_CTRL, P5_WEIGHT, reg_tx_wrr_ctrl, tx_wrr_ctrl->p5_weight);
    reg_tx_wrr_ctrl = RU_FIELD_SET(xlmac_id, LPORT_MAB, TX_WRR_CTRL, P4_WEIGHT, reg_tx_wrr_ctrl, tx_wrr_ctrl->p4_weight);

    RU_REG_WRITE(xlmac_id, LPORT_MAB, TX_WRR_CTRL, reg_tx_wrr_ctrl);

    return 0;
}

int ag_drv_lport_mab_tx_wrr_ctrl_get(uint8_t xlmac_id, lport_mab_tx_wrr_ctrl *tx_wrr_ctrl)
{
    uint32_t reg_tx_wrr_ctrl=0;

#ifdef VALIDATE_PARMS
    if(!tx_wrr_ctrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, LPORT_MAB, TX_WRR_CTRL, reg_tx_wrr_ctrl);

    tx_wrr_ctrl->arb_mode = RU_FIELD_GET(xlmac_id, LPORT_MAB, TX_WRR_CTRL, ARB_MODE, reg_tx_wrr_ctrl);
    tx_wrr_ctrl->p7_weight = RU_FIELD_GET(xlmac_id, LPORT_MAB, TX_WRR_CTRL, P7_WEIGHT, reg_tx_wrr_ctrl);
    tx_wrr_ctrl->p6_weight = RU_FIELD_GET(xlmac_id, LPORT_MAB, TX_WRR_CTRL, P6_WEIGHT, reg_tx_wrr_ctrl);
    tx_wrr_ctrl->p5_weight = RU_FIELD_GET(xlmac_id, LPORT_MAB, TX_WRR_CTRL, P5_WEIGHT, reg_tx_wrr_ctrl);
    tx_wrr_ctrl->p4_weight = RU_FIELD_GET(xlmac_id, LPORT_MAB, TX_WRR_CTRL, P4_WEIGHT, reg_tx_wrr_ctrl);

    return 0;
}

int ag_drv_lport_mab_tx_threshold_set(uint8_t xlmac_id, const lport_mab_tx_threshold *tx_threshold)
{
    uint32_t reg_tx_threshold=0;

#ifdef VALIDATE_PARMS
    if(!tx_threshold)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT) ||
       (tx_threshold->xgmii1_tx_threshold >= _4BITS_MAX_VAL_) ||
       (tx_threshold->gmii7_tx_threshold >= _4BITS_MAX_VAL_) ||
       (tx_threshold->gmii6_tx_threshold >= _4BITS_MAX_VAL_) ||
       (tx_threshold->gmii5_tx_threshold >= _4BITS_MAX_VAL_) ||
       (tx_threshold->gmii4_tx_threshold >= _4BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_tx_threshold = RU_FIELD_SET(xlmac_id, LPORT_MAB, TX_THRESHOLD, XGMII1_TX_THRESHOLD, reg_tx_threshold, tx_threshold->xgmii1_tx_threshold);
    reg_tx_threshold = RU_FIELD_SET(xlmac_id, LPORT_MAB, TX_THRESHOLD, GMII7_TX_THRESHOLD, reg_tx_threshold, tx_threshold->gmii7_tx_threshold);
    reg_tx_threshold = RU_FIELD_SET(xlmac_id, LPORT_MAB, TX_THRESHOLD, GMII6_TX_THRESHOLD, reg_tx_threshold, tx_threshold->gmii6_tx_threshold);
    reg_tx_threshold = RU_FIELD_SET(xlmac_id, LPORT_MAB, TX_THRESHOLD, GMII5_TX_THRESHOLD, reg_tx_threshold, tx_threshold->gmii5_tx_threshold);
    reg_tx_threshold = RU_FIELD_SET(xlmac_id, LPORT_MAB, TX_THRESHOLD, GMII4_TX_THRESHOLD, reg_tx_threshold, tx_threshold->gmii4_tx_threshold);

    RU_REG_WRITE(xlmac_id, LPORT_MAB, TX_THRESHOLD, reg_tx_threshold);

    return 0;
}

int ag_drv_lport_mab_tx_threshold_get(uint8_t xlmac_id, lport_mab_tx_threshold *tx_threshold)
{
    uint32_t reg_tx_threshold=0;

#ifdef VALIDATE_PARMS
    if(!tx_threshold)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, LPORT_MAB, TX_THRESHOLD, reg_tx_threshold);

    tx_threshold->xgmii1_tx_threshold = RU_FIELD_GET(xlmac_id, LPORT_MAB, TX_THRESHOLD, XGMII1_TX_THRESHOLD, reg_tx_threshold);
    tx_threshold->gmii7_tx_threshold = RU_FIELD_GET(xlmac_id, LPORT_MAB, TX_THRESHOLD, GMII7_TX_THRESHOLD, reg_tx_threshold);
    tx_threshold->gmii6_tx_threshold = RU_FIELD_GET(xlmac_id, LPORT_MAB, TX_THRESHOLD, GMII6_TX_THRESHOLD, reg_tx_threshold);
    tx_threshold->gmii5_tx_threshold = RU_FIELD_GET(xlmac_id, LPORT_MAB, TX_THRESHOLD, GMII5_TX_THRESHOLD, reg_tx_threshold);
    tx_threshold->gmii4_tx_threshold = RU_FIELD_GET(xlmac_id, LPORT_MAB, TX_THRESHOLD, GMII4_TX_THRESHOLD, reg_tx_threshold);

    return 0;
}

int ag_drv_lport_mab_link_down_tx_data_set(uint8_t xlmac_id, uint8_t txctl, uint8_t txd)
{
    uint32_t reg_link_down_tx_data=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT) ||
       (txctl >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_link_down_tx_data = RU_FIELD_SET(xlmac_id, LPORT_MAB, LINK_DOWN_TX_DATA, TXCTL, reg_link_down_tx_data, txctl);
    reg_link_down_tx_data = RU_FIELD_SET(xlmac_id, LPORT_MAB, LINK_DOWN_TX_DATA, TXD, reg_link_down_tx_data, txd);

    RU_REG_WRITE(xlmac_id, LPORT_MAB, LINK_DOWN_TX_DATA, reg_link_down_tx_data);

    return 0;
}

int ag_drv_lport_mab_link_down_tx_data_get(uint8_t xlmac_id, uint8_t *txctl, uint8_t *txd)
{
    uint32_t reg_link_down_tx_data=0;

#ifdef VALIDATE_PARMS
    if(!txctl || !txd)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, LPORT_MAB, LINK_DOWN_TX_DATA, reg_link_down_tx_data);

    *txctl = RU_FIELD_GET(xlmac_id, LPORT_MAB, LINK_DOWN_TX_DATA, TXCTL, reg_link_down_tx_data);
    *txd = RU_FIELD_GET(xlmac_id, LPORT_MAB, LINK_DOWN_TX_DATA, TXD, reg_link_down_tx_data);

    return 0;
}

int ag_drv_lport_mab_status_get(uint8_t xlmac_id, lport_mab_status *status)
{
    uint32_t reg_status=0;

#ifdef VALIDATE_PARMS
    if(!status)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, LPORT_MAB, STATUS, reg_status);

    status->xgmii_rx_afifo_overrun = RU_FIELD_GET(xlmac_id, LPORT_MAB, STATUS, XGMII_RX_AFIFO_OVERRUN, reg_status);
    status->gmii_rx_afifo_overrun_vect = RU_FIELD_GET(xlmac_id, LPORT_MAB, STATUS, GMII_RX_AFIFO_OVERRUN_VECT, reg_status);
    status->xgmii_tx_frm_underrun = RU_FIELD_GET(xlmac_id, LPORT_MAB, STATUS, XGMII_TX_FRM_UNDERRUN, reg_status);
    status->xgmii_outstanding_credits_cnt_underrun = RU_FIELD_GET(xlmac_id, LPORT_MAB, STATUS, XGMII_OUTSTANDING_CREDITS_CNT_UNDERRUN, reg_status);
    status->gmii_outstanding_credits_cnt_underrun_vect = RU_FIELD_GET(xlmac_id, LPORT_MAB, STATUS, GMII_OUTSTANDING_CREDITS_CNT_UNDERRUN_VECT, reg_status);
    status->xgmii_tx_afifo_overrun = RU_FIELD_GET(xlmac_id, LPORT_MAB, STATUS, XGMII_TX_AFIFO_OVERRUN, reg_status);
    status->gmii_tx_afifo_overrun_vect = RU_FIELD_GET(xlmac_id, LPORT_MAB, STATUS, GMII_TX_AFIFO_OVERRUN_VECT, reg_status);

    return 0;
}

