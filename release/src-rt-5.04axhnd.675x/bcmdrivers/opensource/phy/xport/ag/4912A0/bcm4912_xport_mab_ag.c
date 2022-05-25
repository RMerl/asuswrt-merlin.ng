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

#include "bcm4912_drivers_xport_ag.h"
#include "bcm4912_xport_mab_ag.h"
#define BLOCK_ADDR_COUNT_BITS 1
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

int ag_drv_xport_mab_ctrl_set(uint8_t xlmac_id, uint8_t tx_credit_disab, uint8_t tx_fifo_rst, uint8_t tx_port_rst, uint8_t rx_port_rst)
{
    uint32_t reg_control=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT) ||
       (tx_credit_disab >= _4BITS_MAX_VAL_) ||
       (tx_fifo_rst >= _4BITS_MAX_VAL_) ||
       (tx_port_rst >= _4BITS_MAX_VAL_) ||
       (rx_port_rst >= _4BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_control = RU_FIELD_SET(xlmac_id, XPORT_MAB, CONTROL, TX_CREDIT_DISAB, reg_control, tx_credit_disab);
    reg_control = RU_FIELD_SET(xlmac_id, XPORT_MAB, CONTROL, TX_FIFO_RST, reg_control, tx_fifo_rst);
    reg_control = RU_FIELD_SET(xlmac_id, XPORT_MAB, CONTROL, TX_PORT_RST, reg_control, tx_port_rst);
    reg_control = RU_FIELD_SET(xlmac_id, XPORT_MAB, CONTROL, RX_PORT_RST, reg_control, rx_port_rst);

    RU_REG_WRITE(xlmac_id, XPORT_MAB, CONTROL, reg_control);

    return 0;
}

int ag_drv_xport_mab_ctrl_get(uint8_t xlmac_id, uint8_t *tx_credit_disab, uint8_t *tx_fifo_rst, uint8_t *tx_port_rst, uint8_t *rx_port_rst)
{
    uint32_t reg_control=0;

#ifdef VALIDATE_PARMS
    if(!tx_credit_disab || !tx_fifo_rst || !tx_port_rst || !rx_port_rst)
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

    RU_REG_READ(xlmac_id, XPORT_MAB, CONTROL, reg_control);

    *tx_credit_disab = RU_FIELD_GET(xlmac_id, XPORT_MAB, CONTROL, TX_CREDIT_DISAB, reg_control);
    *tx_fifo_rst = RU_FIELD_GET(xlmac_id, XPORT_MAB, CONTROL, TX_FIFO_RST, reg_control);
    *tx_port_rst = RU_FIELD_GET(xlmac_id, XPORT_MAB, CONTROL, TX_PORT_RST, reg_control);
    *rx_port_rst = RU_FIELD_GET(xlmac_id, XPORT_MAB, CONTROL, RX_PORT_RST, reg_control);

    return 0;
}

int ag_drv_xport_mab_tx_wrr_ctrl_set(uint8_t xlmac_id, const xport_mab_tx_wrr_ctrl *tx_wrr_ctrl)
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
       (tx_wrr_ctrl->p3_weight >= _4BITS_MAX_VAL_) ||
       (tx_wrr_ctrl->p2_weight >= _4BITS_MAX_VAL_) ||
       (tx_wrr_ctrl->p1_weight >= _4BITS_MAX_VAL_) ||
       (tx_wrr_ctrl->p0_weight >= _4BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_tx_wrr_ctrl = RU_FIELD_SET(xlmac_id, XPORT_MAB, TX_WRR_CTRL, ARB_MODE, reg_tx_wrr_ctrl, tx_wrr_ctrl->arb_mode);
    reg_tx_wrr_ctrl = RU_FIELD_SET(xlmac_id, XPORT_MAB, TX_WRR_CTRL, P3_WEIGHT, reg_tx_wrr_ctrl, tx_wrr_ctrl->p3_weight);
    reg_tx_wrr_ctrl = RU_FIELD_SET(xlmac_id, XPORT_MAB, TX_WRR_CTRL, P2_WEIGHT, reg_tx_wrr_ctrl, tx_wrr_ctrl->p2_weight);
    reg_tx_wrr_ctrl = RU_FIELD_SET(xlmac_id, XPORT_MAB, TX_WRR_CTRL, P1_WEIGHT, reg_tx_wrr_ctrl, tx_wrr_ctrl->p1_weight);
    reg_tx_wrr_ctrl = RU_FIELD_SET(xlmac_id, XPORT_MAB, TX_WRR_CTRL, P0_WEIGHT, reg_tx_wrr_ctrl, tx_wrr_ctrl->p0_weight);

    RU_REG_WRITE(xlmac_id, XPORT_MAB, TX_WRR_CTRL, reg_tx_wrr_ctrl);

    return 0;
}

int ag_drv_xport_mab_tx_wrr_ctrl_get(uint8_t xlmac_id, xport_mab_tx_wrr_ctrl *tx_wrr_ctrl)
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

    RU_REG_READ(xlmac_id, XPORT_MAB, TX_WRR_CTRL, reg_tx_wrr_ctrl);

    tx_wrr_ctrl->arb_mode = RU_FIELD_GET(xlmac_id, XPORT_MAB, TX_WRR_CTRL, ARB_MODE, reg_tx_wrr_ctrl);
    tx_wrr_ctrl->p3_weight = RU_FIELD_GET(xlmac_id, XPORT_MAB, TX_WRR_CTRL, P3_WEIGHT, reg_tx_wrr_ctrl);
    tx_wrr_ctrl->p2_weight = RU_FIELD_GET(xlmac_id, XPORT_MAB, TX_WRR_CTRL, P2_WEIGHT, reg_tx_wrr_ctrl);
    tx_wrr_ctrl->p1_weight = RU_FIELD_GET(xlmac_id, XPORT_MAB, TX_WRR_CTRL, P1_WEIGHT, reg_tx_wrr_ctrl);
    tx_wrr_ctrl->p0_weight = RU_FIELD_GET(xlmac_id, XPORT_MAB, TX_WRR_CTRL, P0_WEIGHT, reg_tx_wrr_ctrl);

    return 0;
}

int ag_drv_xport_mab_tx_threshold_set(uint8_t xlmac_id, const xport_mab_tx_threshold *tx_threshold)
{
    uint32_t reg_tx_threshold=0;

#ifdef VALIDATE_PARMS
    if(!tx_threshold)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT) ||
       (tx_threshold->xgmii3_tx_threshold >= _4BITS_MAX_VAL_) ||
       (tx_threshold->xgmii2_tx_threshold >= _4BITS_MAX_VAL_) ||
       (tx_threshold->xgmii1_tx_threshold >= _4BITS_MAX_VAL_) ||
       (tx_threshold->xgmii0_tx_threshold >= _4BITS_MAX_VAL_) ||
       (tx_threshold->gmii3_tx_threshold >= _4BITS_MAX_VAL_) ||
       (tx_threshold->gmii2_tx_threshold >= _4BITS_MAX_VAL_) ||
       (tx_threshold->gmii1_tx_threshold >= _4BITS_MAX_VAL_) ||
       (tx_threshold->gmii0_tx_threshold >= _4BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_tx_threshold = RU_FIELD_SET(xlmac_id, XPORT_MAB, TX_THRESHOLD, XGMII3_TX_THRESHOLD, reg_tx_threshold, tx_threshold->xgmii3_tx_threshold);
    reg_tx_threshold = RU_FIELD_SET(xlmac_id, XPORT_MAB, TX_THRESHOLD, XGMII2_TX_THRESHOLD, reg_tx_threshold, tx_threshold->xgmii2_tx_threshold);
    reg_tx_threshold = RU_FIELD_SET(xlmac_id, XPORT_MAB, TX_THRESHOLD, XGMII1_TX_THRESHOLD, reg_tx_threshold, tx_threshold->xgmii1_tx_threshold);
    reg_tx_threshold = RU_FIELD_SET(xlmac_id, XPORT_MAB, TX_THRESHOLD, XGMII0_TX_THRESHOLD, reg_tx_threshold, tx_threshold->xgmii0_tx_threshold);
    reg_tx_threshold = RU_FIELD_SET(xlmac_id, XPORT_MAB, TX_THRESHOLD, GMII3_TX_THRESHOLD, reg_tx_threshold, tx_threshold->gmii3_tx_threshold);
    reg_tx_threshold = RU_FIELD_SET(xlmac_id, XPORT_MAB, TX_THRESHOLD, GMII2_TX_THRESHOLD, reg_tx_threshold, tx_threshold->gmii2_tx_threshold);
    reg_tx_threshold = RU_FIELD_SET(xlmac_id, XPORT_MAB, TX_THRESHOLD, GMII1_TX_THRESHOLD, reg_tx_threshold, tx_threshold->gmii1_tx_threshold);
    reg_tx_threshold = RU_FIELD_SET(xlmac_id, XPORT_MAB, TX_THRESHOLD, GMII0_TX_THRESHOLD, reg_tx_threshold, tx_threshold->gmii0_tx_threshold);

    RU_REG_WRITE(xlmac_id, XPORT_MAB, TX_THRESHOLD, reg_tx_threshold);

    return 0;
}

int ag_drv_xport_mab_tx_threshold_get(uint8_t xlmac_id, xport_mab_tx_threshold *tx_threshold)
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

    RU_REG_READ(xlmac_id, XPORT_MAB, TX_THRESHOLD, reg_tx_threshold);

    tx_threshold->xgmii3_tx_threshold = RU_FIELD_GET(xlmac_id, XPORT_MAB, TX_THRESHOLD, XGMII3_TX_THRESHOLD, reg_tx_threshold);
    tx_threshold->xgmii2_tx_threshold = RU_FIELD_GET(xlmac_id, XPORT_MAB, TX_THRESHOLD, XGMII2_TX_THRESHOLD, reg_tx_threshold);
    tx_threshold->xgmii1_tx_threshold = RU_FIELD_GET(xlmac_id, XPORT_MAB, TX_THRESHOLD, XGMII1_TX_THRESHOLD, reg_tx_threshold);
    tx_threshold->xgmii0_tx_threshold = RU_FIELD_GET(xlmac_id, XPORT_MAB, TX_THRESHOLD, XGMII0_TX_THRESHOLD, reg_tx_threshold);
    tx_threshold->gmii3_tx_threshold = RU_FIELD_GET(xlmac_id, XPORT_MAB, TX_THRESHOLD, GMII3_TX_THRESHOLD, reg_tx_threshold);
    tx_threshold->gmii2_tx_threshold = RU_FIELD_GET(xlmac_id, XPORT_MAB, TX_THRESHOLD, GMII2_TX_THRESHOLD, reg_tx_threshold);
    tx_threshold->gmii1_tx_threshold = RU_FIELD_GET(xlmac_id, XPORT_MAB, TX_THRESHOLD, GMII1_TX_THRESHOLD, reg_tx_threshold);
    tx_threshold->gmii0_tx_threshold = RU_FIELD_GET(xlmac_id, XPORT_MAB, TX_THRESHOLD, GMII0_TX_THRESHOLD, reg_tx_threshold);

    return 0;
}

int ag_drv_xport_mab_status_get(uint8_t xlmac_id, uint8_t *tx_frm_underrun_vect, uint8_t *tx_outstanding_credits_cnt_underrun_vect, uint8_t *tx_fifo_overrun_vect, uint8_t *rx_fifo_overrun_vect)
{
    uint32_t reg_status=0;

#ifdef VALIDATE_PARMS
    if(!tx_frm_underrun_vect || !tx_outstanding_credits_cnt_underrun_vect || !tx_fifo_overrun_vect || !rx_fifo_overrun_vect)
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

    RU_REG_READ(xlmac_id, XPORT_MAB, STATUS, reg_status);

    *tx_frm_underrun_vect = RU_FIELD_GET(xlmac_id, XPORT_MAB, STATUS, TX_FRM_UNDERRUN_VECT, reg_status);
    *tx_outstanding_credits_cnt_underrun_vect = RU_FIELD_GET(xlmac_id, XPORT_MAB, STATUS, TX_OUTSTANDING_CREDITS_CNT_UNDERRUN_VECT, reg_status);
    *tx_fifo_overrun_vect = RU_FIELD_GET(xlmac_id, XPORT_MAB, STATUS, TX_FIFO_OVERRUN_VECT, reg_status);
    *rx_fifo_overrun_vect = RU_FIELD_GET(xlmac_id, XPORT_MAB, STATUS, RX_FIFO_OVERRUN_VECT, reg_status);

    return 0;
}

