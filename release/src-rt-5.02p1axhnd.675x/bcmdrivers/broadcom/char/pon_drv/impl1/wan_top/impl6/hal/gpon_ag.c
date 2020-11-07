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
#include "gpon_ag.h"
bdmf_error_t ag_drv_gpon_gpon_gearbox_0_set(const gpon_gpon_gearbox_0 *gpon_gearbox_0)
{
    uint32_t reg_gearbox_0=0;

#ifdef VALIDATE_PARMS
    if(!gpon_gearbox_0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((gpon_gearbox_0->sw_reset_txpg_reset >= _1BITS_MAX_VAL_) ||
       (gpon_gearbox_0->sw_reset_txfifo_reset >= _1BITS_MAX_VAL_) ||
       (gpon_gearbox_0->fifo_cfg_0_clear_txfifo_drifted >= _1BITS_MAX_VAL_) ||
       (gpon_gearbox_0->fifo_cfg_0_loopback_rx >= _1BITS_MAX_VAL_) ||
       (gpon_gearbox_0->fifo_cfg_0_clear_txfifo_collision >= _1BITS_MAX_VAL_) ||
       (gpon_gearbox_0->fifo_cfg_0_tx_wr_ptr_adv >= _1BITS_MAX_VAL_) ||
       (gpon_gearbox_0->fifo_cfg_0_tx_wr_ptr_dly >= _1BITS_MAX_VAL_) ||
       (gpon_gearbox_0->fifo_cfg_0_tx_bit_inv >= _1BITS_MAX_VAL_) ||
       (gpon_gearbox_0->fifo_cfg_0_tx_ptr_dist_max >= _5BITS_MAX_VAL_) ||
       (gpon_gearbox_0->fifo_cfg_0_asym_loopback >= _1BITS_MAX_VAL_) ||
       (gpon_gearbox_0->fifo_cfg_0_tx_ptr_dist_min >= _5BITS_MAX_VAL_) ||
       (gpon_gearbox_0->fifo_cfg_0_tx_20bit_order >= _1BITS_MAX_VAL_) ||
       (gpon_gearbox_0->fifo_cfg_0_tx_8bit_order >= _1BITS_MAX_VAL_) ||
       (gpon_gearbox_0->fifo_cfg_0_rx_16bit_order >= _1BITS_MAX_VAL_) ||
       (gpon_gearbox_0->gpon_gearbox_0_gpon_gearbox_fifo_cfg_0_txlbe_bit_order >= _1BITS_MAX_VAL_) ||
       (gpon_gearbox_0->fifo_status_sel >= _1BITS_MAX_VAL_) ||
       (gpon_gearbox_0->ptg_status1_sel >= _1BITS_MAX_VAL_) ||
       (gpon_gearbox_0->ptg_status2_sel >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gearbox_0 = RU_FIELD_SET(0, GPON, GEARBOX_0, SW_RESET_TXPG_RESET, reg_gearbox_0, gpon_gearbox_0->sw_reset_txpg_reset);
    reg_gearbox_0 = RU_FIELD_SET(0, GPON, GEARBOX_0, SW_RESET_TXFIFO_RESET, reg_gearbox_0, gpon_gearbox_0->sw_reset_txfifo_reset);
    reg_gearbox_0 = RU_FIELD_SET(0, GPON, GEARBOX_0, FIFO_CFG_0_CLEAR_TXFIFO_DRIFTED, reg_gearbox_0, gpon_gearbox_0->fifo_cfg_0_clear_txfifo_drifted);
    reg_gearbox_0 = RU_FIELD_SET(0, GPON, GEARBOX_0, FIFO_CFG_0_LOOPBACK_RX, reg_gearbox_0, gpon_gearbox_0->fifo_cfg_0_loopback_rx);
    reg_gearbox_0 = RU_FIELD_SET(0, GPON, GEARBOX_0, FIFO_CFG_0_CLEAR_TXFIFO_COLLISION, reg_gearbox_0, gpon_gearbox_0->fifo_cfg_0_clear_txfifo_collision);
    reg_gearbox_0 = RU_FIELD_SET(0, GPON, GEARBOX_0, FIFO_CFG_0_TX_WR_PTR_ADV, reg_gearbox_0, gpon_gearbox_0->fifo_cfg_0_tx_wr_ptr_adv);
    reg_gearbox_0 = RU_FIELD_SET(0, GPON, GEARBOX_0, FIFO_CFG_0_TX_WR_PTR_DLY, reg_gearbox_0, gpon_gearbox_0->fifo_cfg_0_tx_wr_ptr_dly);
    reg_gearbox_0 = RU_FIELD_SET(0, GPON, GEARBOX_0, FIFO_CFG_0_TX_BIT_INV, reg_gearbox_0, gpon_gearbox_0->fifo_cfg_0_tx_bit_inv);
    reg_gearbox_0 = RU_FIELD_SET(0, GPON, GEARBOX_0, FIFO_CFG_0_TX_PTR_DIST_MAX, reg_gearbox_0, gpon_gearbox_0->fifo_cfg_0_tx_ptr_dist_max);
    reg_gearbox_0 = RU_FIELD_SET(0, GPON, GEARBOX_0, FIFO_CFG_0_ASYM_LOOPBACK, reg_gearbox_0, gpon_gearbox_0->fifo_cfg_0_asym_loopback);
    reg_gearbox_0 = RU_FIELD_SET(0, GPON, GEARBOX_0, FIFO_CFG_0_TX_PTR_DIST_MIN, reg_gearbox_0, gpon_gearbox_0->fifo_cfg_0_tx_ptr_dist_min);
    reg_gearbox_0 = RU_FIELD_SET(0, GPON, GEARBOX_0, FIFO_CFG_0_TX_20BIT_ORDER, reg_gearbox_0, gpon_gearbox_0->fifo_cfg_0_tx_20bit_order);
    reg_gearbox_0 = RU_FIELD_SET(0, GPON, GEARBOX_0, FIFO_CFG_0_TX_8BIT_ORDER, reg_gearbox_0, gpon_gearbox_0->fifo_cfg_0_tx_8bit_order);
    reg_gearbox_0 = RU_FIELD_SET(0, GPON, GEARBOX_0, FIFO_CFG_0_RX_16BIT_ORDER, reg_gearbox_0, gpon_gearbox_0->fifo_cfg_0_rx_16bit_order);
    reg_gearbox_0 = RU_FIELD_SET(0, GPON, GEARBOX_0, FIFO_CFG_0_TXLBE_BIT_ORDER, reg_gearbox_0, gpon_gearbox_0->gpon_gearbox_0_gpon_gearbox_fifo_cfg_0_txlbe_bit_order);
    reg_gearbox_0 = RU_FIELD_SET(0, GPON, GEARBOX_0, FIFO_STATUS_SEL, reg_gearbox_0, gpon_gearbox_0->fifo_status_sel);
    reg_gearbox_0 = RU_FIELD_SET(0, GPON, GEARBOX_0, PTG_STATUS1_SEL, reg_gearbox_0, gpon_gearbox_0->ptg_status1_sel);
    reg_gearbox_0 = RU_FIELD_SET(0, GPON, GEARBOX_0, PTG_STATUS2_SEL, reg_gearbox_0, gpon_gearbox_0->ptg_status2_sel);

    RU_REG_WRITE(0, GPON, GEARBOX_0, reg_gearbox_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gpon_gpon_gearbox_0_get(gpon_gpon_gearbox_0 *gpon_gearbox_0)
{
    uint32_t reg_gearbox_0=0;

#ifdef VALIDATE_PARMS
    if(!gpon_gearbox_0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, GPON, GEARBOX_0, reg_gearbox_0);

    gpon_gearbox_0->sw_reset_txpg_reset = RU_FIELD_GET(0, GPON, GEARBOX_0, SW_RESET_TXPG_RESET, reg_gearbox_0);
    gpon_gearbox_0->sw_reset_txfifo_reset = RU_FIELD_GET(0, GPON, GEARBOX_0, SW_RESET_TXFIFO_RESET, reg_gearbox_0);
    gpon_gearbox_0->fifo_cfg_0_clear_txfifo_drifted = RU_FIELD_GET(0, GPON, GEARBOX_0, FIFO_CFG_0_CLEAR_TXFIFO_DRIFTED, reg_gearbox_0);
    gpon_gearbox_0->fifo_cfg_0_loopback_rx = RU_FIELD_GET(0, GPON, GEARBOX_0, FIFO_CFG_0_LOOPBACK_RX, reg_gearbox_0);
    gpon_gearbox_0->fifo_cfg_0_clear_txfifo_collision = RU_FIELD_GET(0, GPON, GEARBOX_0, FIFO_CFG_0_CLEAR_TXFIFO_COLLISION, reg_gearbox_0);
    gpon_gearbox_0->fifo_cfg_0_tx_wr_ptr_adv = RU_FIELD_GET(0, GPON, GEARBOX_0, FIFO_CFG_0_TX_WR_PTR_ADV, reg_gearbox_0);
    gpon_gearbox_0->fifo_cfg_0_tx_wr_ptr_dly = RU_FIELD_GET(0, GPON, GEARBOX_0, FIFO_CFG_0_TX_WR_PTR_DLY, reg_gearbox_0);
    gpon_gearbox_0->fifo_cfg_0_tx_bit_inv = RU_FIELD_GET(0, GPON, GEARBOX_0, FIFO_CFG_0_TX_BIT_INV, reg_gearbox_0);
    gpon_gearbox_0->fifo_cfg_0_tx_ptr_dist_max = RU_FIELD_GET(0, GPON, GEARBOX_0, FIFO_CFG_0_TX_PTR_DIST_MAX, reg_gearbox_0);
    gpon_gearbox_0->fifo_cfg_0_asym_loopback = RU_FIELD_GET(0, GPON, GEARBOX_0, FIFO_CFG_0_ASYM_LOOPBACK, reg_gearbox_0);
    gpon_gearbox_0->fifo_cfg_0_tx_ptr_dist_min = RU_FIELD_GET(0, GPON, GEARBOX_0, FIFO_CFG_0_TX_PTR_DIST_MIN, reg_gearbox_0);
    gpon_gearbox_0->fifo_cfg_0_tx_20bit_order = RU_FIELD_GET(0, GPON, GEARBOX_0, FIFO_CFG_0_TX_20BIT_ORDER, reg_gearbox_0);
    gpon_gearbox_0->fifo_cfg_0_tx_8bit_order = RU_FIELD_GET(0, GPON, GEARBOX_0, FIFO_CFG_0_TX_8BIT_ORDER, reg_gearbox_0);
    gpon_gearbox_0->fifo_cfg_0_rx_16bit_order = RU_FIELD_GET(0, GPON, GEARBOX_0, FIFO_CFG_0_RX_16BIT_ORDER, reg_gearbox_0);
    gpon_gearbox_0->gpon_gearbox_0_gpon_gearbox_fifo_cfg_0_txlbe_bit_order = RU_FIELD_GET(0, GPON, GEARBOX_0, FIFO_CFG_0_TXLBE_BIT_ORDER, reg_gearbox_0);
    gpon_gearbox_0->fifo_status_sel = RU_FIELD_GET(0, GPON, GEARBOX_0, FIFO_STATUS_SEL, reg_gearbox_0);
    gpon_gearbox_0->ptg_status1_sel = RU_FIELD_GET(0, GPON, GEARBOX_0, PTG_STATUS1_SEL, reg_gearbox_0);
    gpon_gearbox_0->ptg_status2_sel = RU_FIELD_GET(0, GPON, GEARBOX_0, PTG_STATUS2_SEL, reg_gearbox_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gpon_gpon_gearbox_pattern_cfg1_set(const gpon_gpon_gearbox_pattern_cfg1 *gpon_gearbox_pattern_cfg1)
{
    uint32_t reg_gearbox_pattern_cfg1=0;

#ifdef VALIDATE_PARMS
    if(!gpon_gearbox_pattern_cfg1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((gpon_gearbox_pattern_cfg1->pg_mode >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gearbox_pattern_cfg1 = RU_FIELD_SET(0, GPON, GEARBOX_PATTERN_CFG1, PG_MODE, reg_gearbox_pattern_cfg1, gpon_gearbox_pattern_cfg1->pg_mode);
    reg_gearbox_pattern_cfg1 = RU_FIELD_SET(0, GPON, GEARBOX_PATTERN_CFG1, HEADER, reg_gearbox_pattern_cfg1, gpon_gearbox_pattern_cfg1->header);
    reg_gearbox_pattern_cfg1 = RU_FIELD_SET(0, GPON, GEARBOX_PATTERN_CFG1, PAYLOAD, reg_gearbox_pattern_cfg1, gpon_gearbox_pattern_cfg1->payload);
    reg_gearbox_pattern_cfg1 = RU_FIELD_SET(0, GPON, GEARBOX_PATTERN_CFG1, FILLER, reg_gearbox_pattern_cfg1, gpon_gearbox_pattern_cfg1->filler);

    RU_REG_WRITE(0, GPON, GEARBOX_PATTERN_CFG1, reg_gearbox_pattern_cfg1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gpon_gpon_gearbox_pattern_cfg1_get(gpon_gpon_gearbox_pattern_cfg1 *gpon_gearbox_pattern_cfg1)
{
    uint32_t reg_gearbox_pattern_cfg1=0;

#ifdef VALIDATE_PARMS
    if(!gpon_gearbox_pattern_cfg1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, GPON, GEARBOX_PATTERN_CFG1, reg_gearbox_pattern_cfg1);

    gpon_gearbox_pattern_cfg1->pg_mode = RU_FIELD_GET(0, GPON, GEARBOX_PATTERN_CFG1, PG_MODE, reg_gearbox_pattern_cfg1);
    gpon_gearbox_pattern_cfg1->header = RU_FIELD_GET(0, GPON, GEARBOX_PATTERN_CFG1, HEADER, reg_gearbox_pattern_cfg1);
    gpon_gearbox_pattern_cfg1->payload = RU_FIELD_GET(0, GPON, GEARBOX_PATTERN_CFG1, PAYLOAD, reg_gearbox_pattern_cfg1);
    gpon_gearbox_pattern_cfg1->filler = RU_FIELD_GET(0, GPON, GEARBOX_PATTERN_CFG1, FILLER, reg_gearbox_pattern_cfg1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gpon_gpon_gearbox_pattern_cfg2_set(uint8_t gap_size, uint8_t burst_size)
{
    uint32_t reg_gearbox_pattern_cfg2=0;

#ifdef VALIDATE_PARMS
#endif

    reg_gearbox_pattern_cfg2 = RU_FIELD_SET(0, GPON, GEARBOX_PATTERN_CFG2, BURST_SIZE, reg_gearbox_pattern_cfg2, burst_size);
    reg_gearbox_pattern_cfg2 = RU_FIELD_SET(0, GPON, GEARBOX_PATTERN_CFG2, GAP_SIZE, reg_gearbox_pattern_cfg2, gap_size);

    RU_REG_WRITE(0, GPON, GEARBOX_PATTERN_CFG2, reg_gearbox_pattern_cfg2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gpon_gpon_gearbox_pattern_cfg2_get(uint8_t *gap_size, uint8_t *burst_size)
{
    uint32_t reg_gearbox_pattern_cfg2=0;

#ifdef VALIDATE_PARMS
    if(!burst_size || !gap_size)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, GPON, GEARBOX_PATTERN_CFG2, reg_gearbox_pattern_cfg2);

    *burst_size = RU_FIELD_GET(0, GPON, GEARBOX_PATTERN_CFG2, BURST_SIZE, reg_gearbox_pattern_cfg2);
    *gap_size = RU_FIELD_GET(0, GPON, GEARBOX_PATTERN_CFG2, GAP_SIZE, reg_gearbox_pattern_cfg2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gpon_gpon_gearbox_2_set(const gpon_gpon_gearbox_2 *gpon_gearbox_2)
{
    uint32_t reg_gearbox_2=0;

#ifdef VALIDATE_PARMS
    if(!gpon_gearbox_2)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((gpon_gearbox_2->fifo_cfg_1_tx_rd_pointer >= _5BITS_MAX_VAL_) ||
       (gpon_gearbox_2->fifo_cfg_1_tx_wr_pointer >= _5BITS_MAX_VAL_) ||
       (gpon_gearbox_2->tx_vld_delay_cyc >= _3BITS_MAX_VAL_) ||
       (gpon_gearbox_2->config_burst_delay_cyc >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gearbox_2 = RU_FIELD_SET(0, GPON, GEARBOX_2, FIFO_CFG_1_TX_RD_POINTER, reg_gearbox_2, gpon_gearbox_2->fifo_cfg_1_tx_rd_pointer);
    reg_gearbox_2 = RU_FIELD_SET(0, GPON, GEARBOX_2, FIFO_CFG_1_TX_WR_POINTER, reg_gearbox_2, gpon_gearbox_2->fifo_cfg_1_tx_wr_pointer);
    reg_gearbox_2 = RU_FIELD_SET(0, GPON, GEARBOX_2, TX_VLD_DELAY_CYC, reg_gearbox_2, gpon_gearbox_2->tx_vld_delay_cyc);
    reg_gearbox_2 = RU_FIELD_SET(0, GPON, GEARBOX_2, CONFIG_BURST_DELAY_CYC, reg_gearbox_2, gpon_gearbox_2->config_burst_delay_cyc);

    RU_REG_WRITE(0, GPON, GEARBOX_2, reg_gearbox_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gpon_gpon_gearbox_2_get(gpon_gpon_gearbox_2 *gpon_gearbox_2)
{
    uint32_t reg_gearbox_2=0;

#ifdef VALIDATE_PARMS
    if(!gpon_gearbox_2)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, GPON, GEARBOX_2, reg_gearbox_2);

    gpon_gearbox_2->fifo_cfg_1_tx_rd_pointer = RU_FIELD_GET(0, GPON, GEARBOX_2, FIFO_CFG_1_TX_RD_POINTER, reg_gearbox_2);
    gpon_gearbox_2->fifo_cfg_1_tx_wr_pointer = RU_FIELD_GET(0, GPON, GEARBOX_2, FIFO_CFG_1_TX_WR_POINTER, reg_gearbox_2);
    gpon_gearbox_2->tx_vld_delay_cyc = RU_FIELD_GET(0, GPON, GEARBOX_2, TX_VLD_DELAY_CYC, reg_gearbox_2);
    gpon_gearbox_2->config_burst_delay_cyc = RU_FIELD_GET(0, GPON, GEARBOX_2, CONFIG_BURST_DELAY_CYC, reg_gearbox_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gpon_gpon_gearbox_status_get(uint32_t *cr_rd_data_clx)
{
    uint32_t reg_gearbox_status=0;

#ifdef VALIDATE_PARMS
    if(!cr_rd_data_clx)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, GPON, GEARBOX_STATUS, reg_gearbox_status);

    *cr_rd_data_clx = RU_FIELD_GET(0, GPON, GEARBOX_STATUS, CR_RD_DATA_CLX, reg_gearbox_status);

    return BDMF_ERR_OK;
}

