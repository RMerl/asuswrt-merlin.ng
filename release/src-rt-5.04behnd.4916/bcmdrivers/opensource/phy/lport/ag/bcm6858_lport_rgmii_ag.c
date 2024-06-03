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
#include "bcm6858_lport_rgmii_ag.h"
#define BLOCK_ADDR_COUNT_BITS 2
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

int ag_drv_lport_rgmii_cntrl_set(uint8_t rgmii_id, const lport_rgmii_cntrl *cntrl)
{
    uint32_t reg_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!cntrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((rgmii_id >= BLOCK_ADDR_COUNT) ||
       (cntrl->col_crs_mask >= _1BITS_MAX_VAL_) ||
       (cntrl->rx_err_mask >= _1BITS_MAX_VAL_) ||
       (cntrl->lpi_count >= _5BITS_MAX_VAL_) ||
       (cntrl->tx_clk_stop_en >= _1BITS_MAX_VAL_) ||
       (cntrl->tx_pause_en >= _1BITS_MAX_VAL_) ||
       (cntrl->rx_pause_en >= _1BITS_MAX_VAL_) ||
       (cntrl->rvmii_ref_sel >= _1BITS_MAX_VAL_) ||
       (cntrl->port_mode >= _3BITS_MAX_VAL_) ||
       (cntrl->id_mode_dis >= _1BITS_MAX_VAL_) ||
       (cntrl->rgmii_mode_en >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_cntrl = RU_FIELD_SET(rgmii_id, LPORT_RGMII, CNTRL, COL_CRS_MASK, reg_cntrl, cntrl->col_crs_mask);
    reg_cntrl = RU_FIELD_SET(rgmii_id, LPORT_RGMII, CNTRL, RX_ERR_MASK, reg_cntrl, cntrl->rx_err_mask);
    reg_cntrl = RU_FIELD_SET(rgmii_id, LPORT_RGMII, CNTRL, LPI_COUNT, reg_cntrl, cntrl->lpi_count);
    reg_cntrl = RU_FIELD_SET(rgmii_id, LPORT_RGMII, CNTRL, TX_CLK_STOP_EN, reg_cntrl, cntrl->tx_clk_stop_en);
    reg_cntrl = RU_FIELD_SET(rgmii_id, LPORT_RGMII, CNTRL, TX_PAUSE_EN, reg_cntrl, cntrl->tx_pause_en);
    reg_cntrl = RU_FIELD_SET(rgmii_id, LPORT_RGMII, CNTRL, RX_PAUSE_EN, reg_cntrl, cntrl->rx_pause_en);
    reg_cntrl = RU_FIELD_SET(rgmii_id, LPORT_RGMII, CNTRL, RVMII_REF_SEL, reg_cntrl, cntrl->rvmii_ref_sel);
    reg_cntrl = RU_FIELD_SET(rgmii_id, LPORT_RGMII, CNTRL, PORT_MODE, reg_cntrl, cntrl->port_mode);
    reg_cntrl = RU_FIELD_SET(rgmii_id, LPORT_RGMII, CNTRL, ID_MODE_DIS, reg_cntrl, cntrl->id_mode_dis);
    reg_cntrl = RU_FIELD_SET(rgmii_id, LPORT_RGMII, CNTRL, RGMII_MODE_EN, reg_cntrl, cntrl->rgmii_mode_en);

    RU_REG_WRITE(rgmii_id, LPORT_RGMII, CNTRL, reg_cntrl);

    return 0;
}

int ag_drv_lport_rgmii_cntrl_get(uint8_t rgmii_id, lport_rgmii_cntrl *cntrl)
{
    uint32_t reg_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!cntrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((rgmii_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(rgmii_id, LPORT_RGMII, CNTRL, reg_cntrl);

    cntrl->col_crs_mask = RU_FIELD_GET(rgmii_id, LPORT_RGMII, CNTRL, COL_CRS_MASK, reg_cntrl);
    cntrl->rx_err_mask = RU_FIELD_GET(rgmii_id, LPORT_RGMII, CNTRL, RX_ERR_MASK, reg_cntrl);
    cntrl->lpi_count = RU_FIELD_GET(rgmii_id, LPORT_RGMII, CNTRL, LPI_COUNT, reg_cntrl);
    cntrl->tx_clk_stop_en = RU_FIELD_GET(rgmii_id, LPORT_RGMII, CNTRL, TX_CLK_STOP_EN, reg_cntrl);
    cntrl->tx_pause_en = RU_FIELD_GET(rgmii_id, LPORT_RGMII, CNTRL, TX_PAUSE_EN, reg_cntrl);
    cntrl->rx_pause_en = RU_FIELD_GET(rgmii_id, LPORT_RGMII, CNTRL, RX_PAUSE_EN, reg_cntrl);
    cntrl->rvmii_ref_sel = RU_FIELD_GET(rgmii_id, LPORT_RGMII, CNTRL, RVMII_REF_SEL, reg_cntrl);
    cntrl->port_mode = RU_FIELD_GET(rgmii_id, LPORT_RGMII, CNTRL, PORT_MODE, reg_cntrl);
    cntrl->id_mode_dis = RU_FIELD_GET(rgmii_id, LPORT_RGMII, CNTRL, ID_MODE_DIS, reg_cntrl);
    cntrl->rgmii_mode_en = RU_FIELD_GET(rgmii_id, LPORT_RGMII, CNTRL, RGMII_MODE_EN, reg_cntrl);

    return 0;
}

int ag_drv_lport_rgmii_ib_status_set(uint8_t rgmii_id, uint8_t ib_status_ovrd, uint8_t link_decode, uint8_t duplex_decode, uint8_t speed_decode)
{
    uint32_t reg_ib_status=0;

#ifdef VALIDATE_PARMS
    if((rgmii_id >= BLOCK_ADDR_COUNT) ||
       (ib_status_ovrd >= _1BITS_MAX_VAL_) ||
       (link_decode >= _1BITS_MAX_VAL_) ||
       (duplex_decode >= _1BITS_MAX_VAL_) ||
       (speed_decode >= _2BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_ib_status = RU_FIELD_SET(rgmii_id, LPORT_RGMII, IB_STATUS, IB_STATUS_OVRD, reg_ib_status, ib_status_ovrd);
    reg_ib_status = RU_FIELD_SET(rgmii_id, LPORT_RGMII, IB_STATUS, LINK_DECODE, reg_ib_status, link_decode);
    reg_ib_status = RU_FIELD_SET(rgmii_id, LPORT_RGMII, IB_STATUS, DUPLEX_DECODE, reg_ib_status, duplex_decode);
    reg_ib_status = RU_FIELD_SET(rgmii_id, LPORT_RGMII, IB_STATUS, SPEED_DECODE, reg_ib_status, speed_decode);

    RU_REG_WRITE(rgmii_id, LPORT_RGMII, IB_STATUS, reg_ib_status);

    return 0;
}

int ag_drv_lport_rgmii_ib_status_get(uint8_t rgmii_id, uint8_t *ib_status_ovrd, uint8_t *link_decode, uint8_t *duplex_decode, uint8_t *speed_decode)
{
    uint32_t reg_ib_status=0;

#ifdef VALIDATE_PARMS
    if(!ib_status_ovrd || !link_decode || !duplex_decode || !speed_decode)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((rgmii_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(rgmii_id, LPORT_RGMII, IB_STATUS, reg_ib_status);

    *ib_status_ovrd = RU_FIELD_GET(rgmii_id, LPORT_RGMII, IB_STATUS, IB_STATUS_OVRD, reg_ib_status);
    *link_decode = RU_FIELD_GET(rgmii_id, LPORT_RGMII, IB_STATUS, LINK_DECODE, reg_ib_status);
    *duplex_decode = RU_FIELD_GET(rgmii_id, LPORT_RGMII, IB_STATUS, DUPLEX_DECODE, reg_ib_status);
    *speed_decode = RU_FIELD_GET(rgmii_id, LPORT_RGMII, IB_STATUS, SPEED_DECODE, reg_ib_status);

    return 0;
}

int ag_drv_lport_rgmii_rx_clock_delay_cntrl_set(uint8_t rgmii_id, const lport_rgmii_rx_clock_delay_cntrl *rx_clock_delay_cntrl)
{
    uint32_t reg_rx_clock_delay_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!rx_clock_delay_cntrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((rgmii_id >= BLOCK_ADDR_COUNT) ||
       (rx_clock_delay_cntrl->reset >= _1BITS_MAX_VAL_) ||
       (rx_clock_delay_cntrl->dly_override >= _1BITS_MAX_VAL_) ||
       (rx_clock_delay_cntrl->dly_sel >= _1BITS_MAX_VAL_) ||
       (rx_clock_delay_cntrl->bypass >= _1BITS_MAX_VAL_) ||
       (rx_clock_delay_cntrl->iddq >= _1BITS_MAX_VAL_) ||
       (rx_clock_delay_cntrl->drng >= _2BITS_MAX_VAL_) ||
       (rx_clock_delay_cntrl->ctri >= _2BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_rx_clock_delay_cntrl = RU_FIELD_SET(rgmii_id, LPORT_RGMII, RX_CLOCK_DELAY_CNTRL, RESET, reg_rx_clock_delay_cntrl, rx_clock_delay_cntrl->reset);
    reg_rx_clock_delay_cntrl = RU_FIELD_SET(rgmii_id, LPORT_RGMII, RX_CLOCK_DELAY_CNTRL, DLY_OVERRIDE, reg_rx_clock_delay_cntrl, rx_clock_delay_cntrl->dly_override);
    reg_rx_clock_delay_cntrl = RU_FIELD_SET(rgmii_id, LPORT_RGMII, RX_CLOCK_DELAY_CNTRL, DLY_SEL, reg_rx_clock_delay_cntrl, rx_clock_delay_cntrl->dly_sel);
    reg_rx_clock_delay_cntrl = RU_FIELD_SET(rgmii_id, LPORT_RGMII, RX_CLOCK_DELAY_CNTRL, BYPASS, reg_rx_clock_delay_cntrl, rx_clock_delay_cntrl->bypass);
    reg_rx_clock_delay_cntrl = RU_FIELD_SET(rgmii_id, LPORT_RGMII, RX_CLOCK_DELAY_CNTRL, IDDQ, reg_rx_clock_delay_cntrl, rx_clock_delay_cntrl->iddq);
    reg_rx_clock_delay_cntrl = RU_FIELD_SET(rgmii_id, LPORT_RGMII, RX_CLOCK_DELAY_CNTRL, DRNG, reg_rx_clock_delay_cntrl, rx_clock_delay_cntrl->drng);
    reg_rx_clock_delay_cntrl = RU_FIELD_SET(rgmii_id, LPORT_RGMII, RX_CLOCK_DELAY_CNTRL, CTRI, reg_rx_clock_delay_cntrl, rx_clock_delay_cntrl->ctri);

    RU_REG_WRITE(rgmii_id, LPORT_RGMII, RX_CLOCK_DELAY_CNTRL, reg_rx_clock_delay_cntrl);

    return 0;
}

int ag_drv_lport_rgmii_rx_clock_delay_cntrl_get(uint8_t rgmii_id, lport_rgmii_rx_clock_delay_cntrl *rx_clock_delay_cntrl)
{
    uint32_t reg_rx_clock_delay_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!rx_clock_delay_cntrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((rgmii_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(rgmii_id, LPORT_RGMII, RX_CLOCK_DELAY_CNTRL, reg_rx_clock_delay_cntrl);

    rx_clock_delay_cntrl->reset = RU_FIELD_GET(rgmii_id, LPORT_RGMII, RX_CLOCK_DELAY_CNTRL, RESET, reg_rx_clock_delay_cntrl);
    rx_clock_delay_cntrl->dly_override = RU_FIELD_GET(rgmii_id, LPORT_RGMII, RX_CLOCK_DELAY_CNTRL, DLY_OVERRIDE, reg_rx_clock_delay_cntrl);
    rx_clock_delay_cntrl->dly_sel = RU_FIELD_GET(rgmii_id, LPORT_RGMII, RX_CLOCK_DELAY_CNTRL, DLY_SEL, reg_rx_clock_delay_cntrl);
    rx_clock_delay_cntrl->bypass = RU_FIELD_GET(rgmii_id, LPORT_RGMII, RX_CLOCK_DELAY_CNTRL, BYPASS, reg_rx_clock_delay_cntrl);
    rx_clock_delay_cntrl->iddq = RU_FIELD_GET(rgmii_id, LPORT_RGMII, RX_CLOCK_DELAY_CNTRL, IDDQ, reg_rx_clock_delay_cntrl);
    rx_clock_delay_cntrl->drng = RU_FIELD_GET(rgmii_id, LPORT_RGMII, RX_CLOCK_DELAY_CNTRL, DRNG, reg_rx_clock_delay_cntrl);
    rx_clock_delay_cntrl->ctri = RU_FIELD_GET(rgmii_id, LPORT_RGMII, RX_CLOCK_DELAY_CNTRL, CTRI, reg_rx_clock_delay_cntrl);

    return 0;
}

int ag_drv_lport_rgmii_ate_rx_cntrl_exp_data_set(uint8_t rgmii_id, const lport_rgmii_ate_rx_cntrl_exp_data *ate_rx_cntrl_exp_data)
{
    uint32_t reg_ate_rx_cntrl_exp_data=0;

#ifdef VALIDATE_PARMS
    if(!ate_rx_cntrl_exp_data)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((rgmii_id >= BLOCK_ADDR_COUNT) ||
       (ate_rx_cntrl_exp_data->ate_en >= _1BITS_MAX_VAL_) ||
       (ate_rx_cntrl_exp_data->pkt_count_rst >= _1BITS_MAX_VAL_) ||
       (ate_rx_cntrl_exp_data->expected_data_1 >= _9BITS_MAX_VAL_) ||
       (ate_rx_cntrl_exp_data->expected_data_0 >= _9BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_ate_rx_cntrl_exp_data = RU_FIELD_SET(rgmii_id, LPORT_RGMII, ATE_RX_CNTRL_EXP_DATA, ATE_EN, reg_ate_rx_cntrl_exp_data, ate_rx_cntrl_exp_data->ate_en);
    reg_ate_rx_cntrl_exp_data = RU_FIELD_SET(rgmii_id, LPORT_RGMII, ATE_RX_CNTRL_EXP_DATA, PKT_COUNT_RST, reg_ate_rx_cntrl_exp_data, ate_rx_cntrl_exp_data->pkt_count_rst);
    reg_ate_rx_cntrl_exp_data = RU_FIELD_SET(rgmii_id, LPORT_RGMII, ATE_RX_CNTRL_EXP_DATA, GOOD_COUNT, reg_ate_rx_cntrl_exp_data, ate_rx_cntrl_exp_data->good_count);
    reg_ate_rx_cntrl_exp_data = RU_FIELD_SET(rgmii_id, LPORT_RGMII, ATE_RX_CNTRL_EXP_DATA, EXPECTED_DATA_1, reg_ate_rx_cntrl_exp_data, ate_rx_cntrl_exp_data->expected_data_1);
    reg_ate_rx_cntrl_exp_data = RU_FIELD_SET(rgmii_id, LPORT_RGMII, ATE_RX_CNTRL_EXP_DATA, EXPECTED_DATA_0, reg_ate_rx_cntrl_exp_data, ate_rx_cntrl_exp_data->expected_data_0);

    RU_REG_WRITE(rgmii_id, LPORT_RGMII, ATE_RX_CNTRL_EXP_DATA, reg_ate_rx_cntrl_exp_data);

    return 0;
}

int ag_drv_lport_rgmii_ate_rx_cntrl_exp_data_get(uint8_t rgmii_id, lport_rgmii_ate_rx_cntrl_exp_data *ate_rx_cntrl_exp_data)
{
    uint32_t reg_ate_rx_cntrl_exp_data=0;

#ifdef VALIDATE_PARMS
    if(!ate_rx_cntrl_exp_data)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((rgmii_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(rgmii_id, LPORT_RGMII, ATE_RX_CNTRL_EXP_DATA, reg_ate_rx_cntrl_exp_data);

    ate_rx_cntrl_exp_data->ate_en = RU_FIELD_GET(rgmii_id, LPORT_RGMII, ATE_RX_CNTRL_EXP_DATA, ATE_EN, reg_ate_rx_cntrl_exp_data);
    ate_rx_cntrl_exp_data->pkt_count_rst = RU_FIELD_GET(rgmii_id, LPORT_RGMII, ATE_RX_CNTRL_EXP_DATA, PKT_COUNT_RST, reg_ate_rx_cntrl_exp_data);
    ate_rx_cntrl_exp_data->good_count = RU_FIELD_GET(rgmii_id, LPORT_RGMII, ATE_RX_CNTRL_EXP_DATA, GOOD_COUNT, reg_ate_rx_cntrl_exp_data);
    ate_rx_cntrl_exp_data->expected_data_1 = RU_FIELD_GET(rgmii_id, LPORT_RGMII, ATE_RX_CNTRL_EXP_DATA, EXPECTED_DATA_1, reg_ate_rx_cntrl_exp_data);
    ate_rx_cntrl_exp_data->expected_data_0 = RU_FIELD_GET(rgmii_id, LPORT_RGMII, ATE_RX_CNTRL_EXP_DATA, EXPECTED_DATA_0, reg_ate_rx_cntrl_exp_data);

    return 0;
}

int ag_drv_lport_rgmii_ate_rx_exp_data_1_set(uint8_t rgmii_id, uint16_t expected_data_3, uint16_t expected_data_2)
{
    uint32_t reg_ate_rx_exp_data_1=0;

#ifdef VALIDATE_PARMS
    if((rgmii_id >= BLOCK_ADDR_COUNT) ||
       (expected_data_3 >= _9BITS_MAX_VAL_) ||
       (expected_data_2 >= _9BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_ate_rx_exp_data_1 = RU_FIELD_SET(rgmii_id, LPORT_RGMII, ATE_RX_EXP_DATA_1, EXPECTED_DATA_3, reg_ate_rx_exp_data_1, expected_data_3);
    reg_ate_rx_exp_data_1 = RU_FIELD_SET(rgmii_id, LPORT_RGMII, ATE_RX_EXP_DATA_1, EXPECTED_DATA_2, reg_ate_rx_exp_data_1, expected_data_2);

    RU_REG_WRITE(rgmii_id, LPORT_RGMII, ATE_RX_EXP_DATA_1, reg_ate_rx_exp_data_1);

    return 0;
}

int ag_drv_lport_rgmii_ate_rx_exp_data_1_get(uint8_t rgmii_id, uint16_t *expected_data_3, uint16_t *expected_data_2)
{
    uint32_t reg_ate_rx_exp_data_1=0;

#ifdef VALIDATE_PARMS
    if(!expected_data_3 || !expected_data_2)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((rgmii_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(rgmii_id, LPORT_RGMII, ATE_RX_EXP_DATA_1, reg_ate_rx_exp_data_1);

    *expected_data_3 = RU_FIELD_GET(rgmii_id, LPORT_RGMII, ATE_RX_EXP_DATA_1, EXPECTED_DATA_3, reg_ate_rx_exp_data_1);
    *expected_data_2 = RU_FIELD_GET(rgmii_id, LPORT_RGMII, ATE_RX_EXP_DATA_1, EXPECTED_DATA_2, reg_ate_rx_exp_data_1);

    return 0;
}

int ag_drv_lport_rgmii_ate_rx_status_0_get(uint8_t rgmii_id, uint8_t *rx_ok, uint16_t *received_data_1, uint16_t *received_data_0)
{
    uint32_t reg_ate_rx_status_0=0;

#ifdef VALIDATE_PARMS
    if(!rx_ok || !received_data_1 || !received_data_0)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((rgmii_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(rgmii_id, LPORT_RGMII, ATE_RX_STATUS_0, reg_ate_rx_status_0);

    *rx_ok = RU_FIELD_GET(rgmii_id, LPORT_RGMII, ATE_RX_STATUS_0, RX_OK, reg_ate_rx_status_0);
    *received_data_1 = RU_FIELD_GET(rgmii_id, LPORT_RGMII, ATE_RX_STATUS_0, RECEIVED_DATA_1, reg_ate_rx_status_0);
    *received_data_0 = RU_FIELD_GET(rgmii_id, LPORT_RGMII, ATE_RX_STATUS_0, RECEIVED_DATA_0, reg_ate_rx_status_0);

    return 0;
}

int ag_drv_lport_rgmii_ate_rx_status_1_get(uint8_t rgmii_id, uint16_t *received_data_3, uint16_t *received_data_2)
{
    uint32_t reg_ate_rx_status_1=0;

#ifdef VALIDATE_PARMS
    if(!received_data_3 || !received_data_2)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((rgmii_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(rgmii_id, LPORT_RGMII, ATE_RX_STATUS_1, reg_ate_rx_status_1);

    *received_data_3 = RU_FIELD_GET(rgmii_id, LPORT_RGMII, ATE_RX_STATUS_1, RECEIVED_DATA_3, reg_ate_rx_status_1);
    *received_data_2 = RU_FIELD_GET(rgmii_id, LPORT_RGMII, ATE_RX_STATUS_1, RECEIVED_DATA_2, reg_ate_rx_status_1);

    return 0;
}

int ag_drv_lport_rgmii_ate_tx_cntrl_set(uint8_t rgmii_id, const lport_rgmii_ate_tx_cntrl *ate_tx_cntrl)
{
    uint32_t reg_ate_tx_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!ate_tx_cntrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((rgmii_id >= BLOCK_ADDR_COUNT) ||
       (ate_tx_cntrl->pkt_ipg >= _6BITS_MAX_VAL_) ||
       (ate_tx_cntrl->payload_length >= _11BITS_MAX_VAL_) ||
       (ate_tx_cntrl->pkt_gen_en >= _1BITS_MAX_VAL_) ||
       (ate_tx_cntrl->start_stop >= _1BITS_MAX_VAL_) ||
       (ate_tx_cntrl->start_stop_ovrd >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_ate_tx_cntrl = RU_FIELD_SET(rgmii_id, LPORT_RGMII, ATE_TX_CNTRL, PKT_IPG, reg_ate_tx_cntrl, ate_tx_cntrl->pkt_ipg);
    reg_ate_tx_cntrl = RU_FIELD_SET(rgmii_id, LPORT_RGMII, ATE_TX_CNTRL, PAYLOAD_LENGTH, reg_ate_tx_cntrl, ate_tx_cntrl->payload_length);
    reg_ate_tx_cntrl = RU_FIELD_SET(rgmii_id, LPORT_RGMII, ATE_TX_CNTRL, PKT_CNT, reg_ate_tx_cntrl, ate_tx_cntrl->pkt_cnt);
    reg_ate_tx_cntrl = RU_FIELD_SET(rgmii_id, LPORT_RGMII, ATE_TX_CNTRL, PKT_GEN_EN, reg_ate_tx_cntrl, ate_tx_cntrl->pkt_gen_en);
    reg_ate_tx_cntrl = RU_FIELD_SET(rgmii_id, LPORT_RGMII, ATE_TX_CNTRL, START_STOP, reg_ate_tx_cntrl, ate_tx_cntrl->start_stop);
    reg_ate_tx_cntrl = RU_FIELD_SET(rgmii_id, LPORT_RGMII, ATE_TX_CNTRL, START_STOP_OVRD, reg_ate_tx_cntrl, ate_tx_cntrl->start_stop_ovrd);

    RU_REG_WRITE(rgmii_id, LPORT_RGMII, ATE_TX_CNTRL, reg_ate_tx_cntrl);

    return 0;
}

int ag_drv_lport_rgmii_ate_tx_cntrl_get(uint8_t rgmii_id, lport_rgmii_ate_tx_cntrl *ate_tx_cntrl)
{
    uint32_t reg_ate_tx_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!ate_tx_cntrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((rgmii_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(rgmii_id, LPORT_RGMII, ATE_TX_CNTRL, reg_ate_tx_cntrl);

    ate_tx_cntrl->pkt_ipg = RU_FIELD_GET(rgmii_id, LPORT_RGMII, ATE_TX_CNTRL, PKT_IPG, reg_ate_tx_cntrl);
    ate_tx_cntrl->payload_length = RU_FIELD_GET(rgmii_id, LPORT_RGMII, ATE_TX_CNTRL, PAYLOAD_LENGTH, reg_ate_tx_cntrl);
    ate_tx_cntrl->pkt_cnt = RU_FIELD_GET(rgmii_id, LPORT_RGMII, ATE_TX_CNTRL, PKT_CNT, reg_ate_tx_cntrl);
    ate_tx_cntrl->pkt_gen_en = RU_FIELD_GET(rgmii_id, LPORT_RGMII, ATE_TX_CNTRL, PKT_GEN_EN, reg_ate_tx_cntrl);
    ate_tx_cntrl->start_stop = RU_FIELD_GET(rgmii_id, LPORT_RGMII, ATE_TX_CNTRL, START_STOP, reg_ate_tx_cntrl);
    ate_tx_cntrl->start_stop_ovrd = RU_FIELD_GET(rgmii_id, LPORT_RGMII, ATE_TX_CNTRL, START_STOP_OVRD, reg_ate_tx_cntrl);

    return 0;
}

int ag_drv_lport_rgmii_ate_tx_data_0_set(uint8_t rgmii_id, uint16_t tx_data_1, uint16_t tx_data_0)
{
    uint32_t reg_ate_tx_data_0=0;

#ifdef VALIDATE_PARMS
    if((rgmii_id >= BLOCK_ADDR_COUNT) ||
       (tx_data_1 >= _9BITS_MAX_VAL_) ||
       (tx_data_0 >= _9BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_ate_tx_data_0 = RU_FIELD_SET(rgmii_id, LPORT_RGMII, ATE_TX_DATA_0, TX_DATA_1, reg_ate_tx_data_0, tx_data_1);
    reg_ate_tx_data_0 = RU_FIELD_SET(rgmii_id, LPORT_RGMII, ATE_TX_DATA_0, TX_DATA_0, reg_ate_tx_data_0, tx_data_0);

    RU_REG_WRITE(rgmii_id, LPORT_RGMII, ATE_TX_DATA_0, reg_ate_tx_data_0);

    return 0;
}

int ag_drv_lport_rgmii_ate_tx_data_0_get(uint8_t rgmii_id, uint16_t *tx_data_1, uint16_t *tx_data_0)
{
    uint32_t reg_ate_tx_data_0=0;

#ifdef VALIDATE_PARMS
    if(!tx_data_1 || !tx_data_0)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((rgmii_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(rgmii_id, LPORT_RGMII, ATE_TX_DATA_0, reg_ate_tx_data_0);

    *tx_data_1 = RU_FIELD_GET(rgmii_id, LPORT_RGMII, ATE_TX_DATA_0, TX_DATA_1, reg_ate_tx_data_0);
    *tx_data_0 = RU_FIELD_GET(rgmii_id, LPORT_RGMII, ATE_TX_DATA_0, TX_DATA_0, reg_ate_tx_data_0);

    return 0;
}

int ag_drv_lport_rgmii_ate_tx_data_1_set(uint8_t rgmii_id, uint16_t tx_data_3, uint16_t tx_data_2)
{
    uint32_t reg_ate_tx_data_1=0;

#ifdef VALIDATE_PARMS
    if((rgmii_id >= BLOCK_ADDR_COUNT) ||
       (tx_data_3 >= _9BITS_MAX_VAL_) ||
       (tx_data_2 >= _9BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_ate_tx_data_1 = RU_FIELD_SET(rgmii_id, LPORT_RGMII, ATE_TX_DATA_1, TX_DATA_3, reg_ate_tx_data_1, tx_data_3);
    reg_ate_tx_data_1 = RU_FIELD_SET(rgmii_id, LPORT_RGMII, ATE_TX_DATA_1, TX_DATA_2, reg_ate_tx_data_1, tx_data_2);

    RU_REG_WRITE(rgmii_id, LPORT_RGMII, ATE_TX_DATA_1, reg_ate_tx_data_1);

    return 0;
}

int ag_drv_lport_rgmii_ate_tx_data_1_get(uint8_t rgmii_id, uint16_t *tx_data_3, uint16_t *tx_data_2)
{
    uint32_t reg_ate_tx_data_1=0;

#ifdef VALIDATE_PARMS
    if(!tx_data_3 || !tx_data_2)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((rgmii_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(rgmii_id, LPORT_RGMII, ATE_TX_DATA_1, reg_ate_tx_data_1);

    *tx_data_3 = RU_FIELD_GET(rgmii_id, LPORT_RGMII, ATE_TX_DATA_1, TX_DATA_3, reg_ate_tx_data_1);
    *tx_data_2 = RU_FIELD_GET(rgmii_id, LPORT_RGMII, ATE_TX_DATA_1, TX_DATA_2, reg_ate_tx_data_1);

    return 0;
}

int ag_drv_lport_rgmii_ate_tx_data_2_set(uint8_t rgmii_id, uint16_t ether_type, uint8_t tx_data_5, uint8_t tx_data_4)
{
    uint32_t reg_ate_tx_data_2=0;

#ifdef VALIDATE_PARMS
    if((rgmii_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_ate_tx_data_2 = RU_FIELD_SET(rgmii_id, LPORT_RGMII, ATE_TX_DATA_2, ETHER_TYPE, reg_ate_tx_data_2, ether_type);
    reg_ate_tx_data_2 = RU_FIELD_SET(rgmii_id, LPORT_RGMII, ATE_TX_DATA_2, TX_DATA_5, reg_ate_tx_data_2, tx_data_5);
    reg_ate_tx_data_2 = RU_FIELD_SET(rgmii_id, LPORT_RGMII, ATE_TX_DATA_2, TX_DATA_4, reg_ate_tx_data_2, tx_data_4);

    RU_REG_WRITE(rgmii_id, LPORT_RGMII, ATE_TX_DATA_2, reg_ate_tx_data_2);

    return 0;
}

int ag_drv_lport_rgmii_ate_tx_data_2_get(uint8_t rgmii_id, uint16_t *ether_type, uint8_t *tx_data_5, uint8_t *tx_data_4)
{
    uint32_t reg_ate_tx_data_2=0;

#ifdef VALIDATE_PARMS
    if(!ether_type || !tx_data_5 || !tx_data_4)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((rgmii_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(rgmii_id, LPORT_RGMII, ATE_TX_DATA_2, reg_ate_tx_data_2);

    *ether_type = RU_FIELD_GET(rgmii_id, LPORT_RGMII, ATE_TX_DATA_2, ETHER_TYPE, reg_ate_tx_data_2);
    *tx_data_5 = RU_FIELD_GET(rgmii_id, LPORT_RGMII, ATE_TX_DATA_2, TX_DATA_5, reg_ate_tx_data_2);
    *tx_data_4 = RU_FIELD_GET(rgmii_id, LPORT_RGMII, ATE_TX_DATA_2, TX_DATA_4, reg_ate_tx_data_2);

    return 0;
}

int ag_drv_lport_rgmii_tx_delay_cntrl_0_set(uint8_t rgmii_id, const lport_rgmii_tx_delay_cntrl_0 *tx_delay_cntrl_0)
{
    uint32_t reg_tx_delay_cntrl_0=0;

#ifdef VALIDATE_PARMS
    if(!tx_delay_cntrl_0)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((rgmii_id >= BLOCK_ADDR_COUNT) ||
       (tx_delay_cntrl_0->txd3_del_ovrd_en >= _1BITS_MAX_VAL_) ||
       (tx_delay_cntrl_0->txd3_del_sel >= _6BITS_MAX_VAL_) ||
       (tx_delay_cntrl_0->txd2_del_ovrd_en >= _1BITS_MAX_VAL_) ||
       (tx_delay_cntrl_0->txd2_del_sel >= _6BITS_MAX_VAL_) ||
       (tx_delay_cntrl_0->txd1_del_ovrd_en >= _1BITS_MAX_VAL_) ||
       (tx_delay_cntrl_0->txd1_del_sel >= _6BITS_MAX_VAL_) ||
       (tx_delay_cntrl_0->txd0_del_ovrd_en >= _1BITS_MAX_VAL_) ||
       (tx_delay_cntrl_0->txd0_del_sel >= _6BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_tx_delay_cntrl_0 = RU_FIELD_SET(rgmii_id, LPORT_RGMII, TX_DELAY_CNTRL_0, TXD3_DEL_OVRD_EN, reg_tx_delay_cntrl_0, tx_delay_cntrl_0->txd3_del_ovrd_en);
    reg_tx_delay_cntrl_0 = RU_FIELD_SET(rgmii_id, LPORT_RGMII, TX_DELAY_CNTRL_0, TXD3_DEL_SEL, reg_tx_delay_cntrl_0, tx_delay_cntrl_0->txd3_del_sel);
    reg_tx_delay_cntrl_0 = RU_FIELD_SET(rgmii_id, LPORT_RGMII, TX_DELAY_CNTRL_0, TXD2_DEL_OVRD_EN, reg_tx_delay_cntrl_0, tx_delay_cntrl_0->txd2_del_ovrd_en);
    reg_tx_delay_cntrl_0 = RU_FIELD_SET(rgmii_id, LPORT_RGMII, TX_DELAY_CNTRL_0, TXD2_DEL_SEL, reg_tx_delay_cntrl_0, tx_delay_cntrl_0->txd2_del_sel);
    reg_tx_delay_cntrl_0 = RU_FIELD_SET(rgmii_id, LPORT_RGMII, TX_DELAY_CNTRL_0, TXD1_DEL_OVRD_EN, reg_tx_delay_cntrl_0, tx_delay_cntrl_0->txd1_del_ovrd_en);
    reg_tx_delay_cntrl_0 = RU_FIELD_SET(rgmii_id, LPORT_RGMII, TX_DELAY_CNTRL_0, TXD1_DEL_SEL, reg_tx_delay_cntrl_0, tx_delay_cntrl_0->txd1_del_sel);
    reg_tx_delay_cntrl_0 = RU_FIELD_SET(rgmii_id, LPORT_RGMII, TX_DELAY_CNTRL_0, TXD0_DEL_OVRD_EN, reg_tx_delay_cntrl_0, tx_delay_cntrl_0->txd0_del_ovrd_en);
    reg_tx_delay_cntrl_0 = RU_FIELD_SET(rgmii_id, LPORT_RGMII, TX_DELAY_CNTRL_0, TXD0_DEL_SEL, reg_tx_delay_cntrl_0, tx_delay_cntrl_0->txd0_del_sel);

    RU_REG_WRITE(rgmii_id, LPORT_RGMII, TX_DELAY_CNTRL_0, reg_tx_delay_cntrl_0);

    return 0;
}

int ag_drv_lport_rgmii_tx_delay_cntrl_0_get(uint8_t rgmii_id, lport_rgmii_tx_delay_cntrl_0 *tx_delay_cntrl_0)
{
    uint32_t reg_tx_delay_cntrl_0=0;

#ifdef VALIDATE_PARMS
    if(!tx_delay_cntrl_0)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((rgmii_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(rgmii_id, LPORT_RGMII, TX_DELAY_CNTRL_0, reg_tx_delay_cntrl_0);

    tx_delay_cntrl_0->txd3_del_ovrd_en = RU_FIELD_GET(rgmii_id, LPORT_RGMII, TX_DELAY_CNTRL_0, TXD3_DEL_OVRD_EN, reg_tx_delay_cntrl_0);
    tx_delay_cntrl_0->txd3_del_sel = RU_FIELD_GET(rgmii_id, LPORT_RGMII, TX_DELAY_CNTRL_0, TXD3_DEL_SEL, reg_tx_delay_cntrl_0);
    tx_delay_cntrl_0->txd2_del_ovrd_en = RU_FIELD_GET(rgmii_id, LPORT_RGMII, TX_DELAY_CNTRL_0, TXD2_DEL_OVRD_EN, reg_tx_delay_cntrl_0);
    tx_delay_cntrl_0->txd2_del_sel = RU_FIELD_GET(rgmii_id, LPORT_RGMII, TX_DELAY_CNTRL_0, TXD2_DEL_SEL, reg_tx_delay_cntrl_0);
    tx_delay_cntrl_0->txd1_del_ovrd_en = RU_FIELD_GET(rgmii_id, LPORT_RGMII, TX_DELAY_CNTRL_0, TXD1_DEL_OVRD_EN, reg_tx_delay_cntrl_0);
    tx_delay_cntrl_0->txd1_del_sel = RU_FIELD_GET(rgmii_id, LPORT_RGMII, TX_DELAY_CNTRL_0, TXD1_DEL_SEL, reg_tx_delay_cntrl_0);
    tx_delay_cntrl_0->txd0_del_ovrd_en = RU_FIELD_GET(rgmii_id, LPORT_RGMII, TX_DELAY_CNTRL_0, TXD0_DEL_OVRD_EN, reg_tx_delay_cntrl_0);
    tx_delay_cntrl_0->txd0_del_sel = RU_FIELD_GET(rgmii_id, LPORT_RGMII, TX_DELAY_CNTRL_0, TXD0_DEL_SEL, reg_tx_delay_cntrl_0);

    return 0;
}

int ag_drv_lport_rgmii_tx_delay_cntrl_1_set(uint8_t rgmii_id, const lport_rgmii_tx_delay_cntrl_1 *tx_delay_cntrl_1)
{
    uint32_t reg_tx_delay_cntrl_1=0;

#ifdef VALIDATE_PARMS
    if(!tx_delay_cntrl_1)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((rgmii_id >= BLOCK_ADDR_COUNT) ||
       (tx_delay_cntrl_1->txclk_id_del_ovrd_en >= _1BITS_MAX_VAL_) ||
       (tx_delay_cntrl_1->txclk_id_del_sel >= _4BITS_MAX_VAL_) ||
       (tx_delay_cntrl_1->txclk_del_ovrd_en >= _1BITS_MAX_VAL_) ||
       (tx_delay_cntrl_1->txclk_del_sel >= _4BITS_MAX_VAL_) ||
       (tx_delay_cntrl_1->txctl_del_ovrd_en >= _1BITS_MAX_VAL_) ||
       (tx_delay_cntrl_1->txctl_del_sel >= _6BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_tx_delay_cntrl_1 = RU_FIELD_SET(rgmii_id, LPORT_RGMII, TX_DELAY_CNTRL_1, TXCLK_ID_DEL_OVRD_EN, reg_tx_delay_cntrl_1, tx_delay_cntrl_1->txclk_id_del_ovrd_en);
    reg_tx_delay_cntrl_1 = RU_FIELD_SET(rgmii_id, LPORT_RGMII, TX_DELAY_CNTRL_1, TXCLK_ID_DEL_SEL, reg_tx_delay_cntrl_1, tx_delay_cntrl_1->txclk_id_del_sel);
    reg_tx_delay_cntrl_1 = RU_FIELD_SET(rgmii_id, LPORT_RGMII, TX_DELAY_CNTRL_1, TXCLK_DEL_OVRD_EN, reg_tx_delay_cntrl_1, tx_delay_cntrl_1->txclk_del_ovrd_en);
    reg_tx_delay_cntrl_1 = RU_FIELD_SET(rgmii_id, LPORT_RGMII, TX_DELAY_CNTRL_1, TXCLK_DEL_SEL, reg_tx_delay_cntrl_1, tx_delay_cntrl_1->txclk_del_sel);
    reg_tx_delay_cntrl_1 = RU_FIELD_SET(rgmii_id, LPORT_RGMII, TX_DELAY_CNTRL_1, TXCTL_DEL_OVRD_EN, reg_tx_delay_cntrl_1, tx_delay_cntrl_1->txctl_del_ovrd_en);
    reg_tx_delay_cntrl_1 = RU_FIELD_SET(rgmii_id, LPORT_RGMII, TX_DELAY_CNTRL_1, TXCTL_DEL_SEL, reg_tx_delay_cntrl_1, tx_delay_cntrl_1->txctl_del_sel);

    RU_REG_WRITE(rgmii_id, LPORT_RGMII, TX_DELAY_CNTRL_1, reg_tx_delay_cntrl_1);

    return 0;
}

int ag_drv_lport_rgmii_tx_delay_cntrl_1_get(uint8_t rgmii_id, lport_rgmii_tx_delay_cntrl_1 *tx_delay_cntrl_1)
{
    uint32_t reg_tx_delay_cntrl_1=0;

#ifdef VALIDATE_PARMS
    if(!tx_delay_cntrl_1)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((rgmii_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(rgmii_id, LPORT_RGMII, TX_DELAY_CNTRL_1, reg_tx_delay_cntrl_1);

    tx_delay_cntrl_1->txclk_id_del_ovrd_en = RU_FIELD_GET(rgmii_id, LPORT_RGMII, TX_DELAY_CNTRL_1, TXCLK_ID_DEL_OVRD_EN, reg_tx_delay_cntrl_1);
    tx_delay_cntrl_1->txclk_id_del_sel = RU_FIELD_GET(rgmii_id, LPORT_RGMII, TX_DELAY_CNTRL_1, TXCLK_ID_DEL_SEL, reg_tx_delay_cntrl_1);
    tx_delay_cntrl_1->txclk_del_ovrd_en = RU_FIELD_GET(rgmii_id, LPORT_RGMII, TX_DELAY_CNTRL_1, TXCLK_DEL_OVRD_EN, reg_tx_delay_cntrl_1);
    tx_delay_cntrl_1->txclk_del_sel = RU_FIELD_GET(rgmii_id, LPORT_RGMII, TX_DELAY_CNTRL_1, TXCLK_DEL_SEL, reg_tx_delay_cntrl_1);
    tx_delay_cntrl_1->txctl_del_ovrd_en = RU_FIELD_GET(rgmii_id, LPORT_RGMII, TX_DELAY_CNTRL_1, TXCTL_DEL_OVRD_EN, reg_tx_delay_cntrl_1);
    tx_delay_cntrl_1->txctl_del_sel = RU_FIELD_GET(rgmii_id, LPORT_RGMII, TX_DELAY_CNTRL_1, TXCTL_DEL_SEL, reg_tx_delay_cntrl_1);

    return 0;
}

int ag_drv_lport_rgmii_rx_delay_cntrl_0_set(uint8_t rgmii_id, const lport_rgmii_rx_delay_cntrl_0 *rx_delay_cntrl_0)
{
    uint32_t reg_rx_delay_cntrl_0=0;

#ifdef VALIDATE_PARMS
    if(!rx_delay_cntrl_0)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((rgmii_id >= BLOCK_ADDR_COUNT) ||
       (rx_delay_cntrl_0->rxd3_del_ovrd_en >= _1BITS_MAX_VAL_) ||
       (rx_delay_cntrl_0->rxd3_del_sel >= _6BITS_MAX_VAL_) ||
       (rx_delay_cntrl_0->rxd2_del_ovrd_en >= _1BITS_MAX_VAL_) ||
       (rx_delay_cntrl_0->rxd2_del_sel >= _6BITS_MAX_VAL_) ||
       (rx_delay_cntrl_0->rxd1_del_ovrd_en >= _1BITS_MAX_VAL_) ||
       (rx_delay_cntrl_0->rxd1_del_sel >= _6BITS_MAX_VAL_) ||
       (rx_delay_cntrl_0->rxd0_del_ovrd_en >= _1BITS_MAX_VAL_) ||
       (rx_delay_cntrl_0->rxd0_del_sel >= _6BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_rx_delay_cntrl_0 = RU_FIELD_SET(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_0, RXD3_DEL_OVRD_EN, reg_rx_delay_cntrl_0, rx_delay_cntrl_0->rxd3_del_ovrd_en);
    reg_rx_delay_cntrl_0 = RU_FIELD_SET(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_0, RXD3_DEL_SEL, reg_rx_delay_cntrl_0, rx_delay_cntrl_0->rxd3_del_sel);
    reg_rx_delay_cntrl_0 = RU_FIELD_SET(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_0, RXD2_DEL_OVRD_EN, reg_rx_delay_cntrl_0, rx_delay_cntrl_0->rxd2_del_ovrd_en);
    reg_rx_delay_cntrl_0 = RU_FIELD_SET(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_0, RXD2_DEL_SEL, reg_rx_delay_cntrl_0, rx_delay_cntrl_0->rxd2_del_sel);
    reg_rx_delay_cntrl_0 = RU_FIELD_SET(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_0, RXD1_DEL_OVRD_EN, reg_rx_delay_cntrl_0, rx_delay_cntrl_0->rxd1_del_ovrd_en);
    reg_rx_delay_cntrl_0 = RU_FIELD_SET(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_0, RXD1_DEL_SEL, reg_rx_delay_cntrl_0, rx_delay_cntrl_0->rxd1_del_sel);
    reg_rx_delay_cntrl_0 = RU_FIELD_SET(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_0, RXD0_DEL_OVRD_EN, reg_rx_delay_cntrl_0, rx_delay_cntrl_0->rxd0_del_ovrd_en);
    reg_rx_delay_cntrl_0 = RU_FIELD_SET(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_0, RXD0_DEL_SEL, reg_rx_delay_cntrl_0, rx_delay_cntrl_0->rxd0_del_sel);

    RU_REG_WRITE(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_0, reg_rx_delay_cntrl_0);

    return 0;
}

int ag_drv_lport_rgmii_rx_delay_cntrl_0_get(uint8_t rgmii_id, lport_rgmii_rx_delay_cntrl_0 *rx_delay_cntrl_0)
{
    uint32_t reg_rx_delay_cntrl_0=0;

#ifdef VALIDATE_PARMS
    if(!rx_delay_cntrl_0)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((rgmii_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_0, reg_rx_delay_cntrl_0);

    rx_delay_cntrl_0->rxd3_del_ovrd_en = RU_FIELD_GET(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_0, RXD3_DEL_OVRD_EN, reg_rx_delay_cntrl_0);
    rx_delay_cntrl_0->rxd3_del_sel = RU_FIELD_GET(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_0, RXD3_DEL_SEL, reg_rx_delay_cntrl_0);
    rx_delay_cntrl_0->rxd2_del_ovrd_en = RU_FIELD_GET(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_0, RXD2_DEL_OVRD_EN, reg_rx_delay_cntrl_0);
    rx_delay_cntrl_0->rxd2_del_sel = RU_FIELD_GET(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_0, RXD2_DEL_SEL, reg_rx_delay_cntrl_0);
    rx_delay_cntrl_0->rxd1_del_ovrd_en = RU_FIELD_GET(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_0, RXD1_DEL_OVRD_EN, reg_rx_delay_cntrl_0);
    rx_delay_cntrl_0->rxd1_del_sel = RU_FIELD_GET(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_0, RXD1_DEL_SEL, reg_rx_delay_cntrl_0);
    rx_delay_cntrl_0->rxd0_del_ovrd_en = RU_FIELD_GET(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_0, RXD0_DEL_OVRD_EN, reg_rx_delay_cntrl_0);
    rx_delay_cntrl_0->rxd0_del_sel = RU_FIELD_GET(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_0, RXD0_DEL_SEL, reg_rx_delay_cntrl_0);

    return 0;
}

int ag_drv_lport_rgmii_rx_delay_cntrl_1_set(uint8_t rgmii_id, const lport_rgmii_rx_delay_cntrl_1 *rx_delay_cntrl_1)
{
    uint32_t reg_rx_delay_cntrl_1=0;

#ifdef VALIDATE_PARMS
    if(!rx_delay_cntrl_1)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((rgmii_id >= BLOCK_ADDR_COUNT) ||
       (rx_delay_cntrl_1->rxd7_del_ovrd_en >= _1BITS_MAX_VAL_) ||
       (rx_delay_cntrl_1->rxd7_del_sel >= _6BITS_MAX_VAL_) ||
       (rx_delay_cntrl_1->rxd6_del_ovrd_en >= _1BITS_MAX_VAL_) ||
       (rx_delay_cntrl_1->rxd6_del_sel >= _6BITS_MAX_VAL_) ||
       (rx_delay_cntrl_1->rxd5_del_ovrd_en >= _1BITS_MAX_VAL_) ||
       (rx_delay_cntrl_1->rxd5_del_sel >= _6BITS_MAX_VAL_) ||
       (rx_delay_cntrl_1->rxd4_del_ovrd_en >= _1BITS_MAX_VAL_) ||
       (rx_delay_cntrl_1->rxd4_del_sel >= _6BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_rx_delay_cntrl_1 = RU_FIELD_SET(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_1, RXD7_DEL_OVRD_EN, reg_rx_delay_cntrl_1, rx_delay_cntrl_1->rxd7_del_ovrd_en);
    reg_rx_delay_cntrl_1 = RU_FIELD_SET(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_1, RXD7_DEL_SEL, reg_rx_delay_cntrl_1, rx_delay_cntrl_1->rxd7_del_sel);
    reg_rx_delay_cntrl_1 = RU_FIELD_SET(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_1, RXD6_DEL_OVRD_EN, reg_rx_delay_cntrl_1, rx_delay_cntrl_1->rxd6_del_ovrd_en);
    reg_rx_delay_cntrl_1 = RU_FIELD_SET(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_1, RXD6_DEL_SEL, reg_rx_delay_cntrl_1, rx_delay_cntrl_1->rxd6_del_sel);
    reg_rx_delay_cntrl_1 = RU_FIELD_SET(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_1, RXD5_DEL_OVRD_EN, reg_rx_delay_cntrl_1, rx_delay_cntrl_1->rxd5_del_ovrd_en);
    reg_rx_delay_cntrl_1 = RU_FIELD_SET(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_1, RXD5_DEL_SEL, reg_rx_delay_cntrl_1, rx_delay_cntrl_1->rxd5_del_sel);
    reg_rx_delay_cntrl_1 = RU_FIELD_SET(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_1, RXD4_DEL_OVRD_EN, reg_rx_delay_cntrl_1, rx_delay_cntrl_1->rxd4_del_ovrd_en);
    reg_rx_delay_cntrl_1 = RU_FIELD_SET(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_1, RXD4_DEL_SEL, reg_rx_delay_cntrl_1, rx_delay_cntrl_1->rxd4_del_sel);

    RU_REG_WRITE(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_1, reg_rx_delay_cntrl_1);

    return 0;
}

int ag_drv_lport_rgmii_rx_delay_cntrl_1_get(uint8_t rgmii_id, lport_rgmii_rx_delay_cntrl_1 *rx_delay_cntrl_1)
{
    uint32_t reg_rx_delay_cntrl_1=0;

#ifdef VALIDATE_PARMS
    if(!rx_delay_cntrl_1)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((rgmii_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_1, reg_rx_delay_cntrl_1);

    rx_delay_cntrl_1->rxd7_del_ovrd_en = RU_FIELD_GET(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_1, RXD7_DEL_OVRD_EN, reg_rx_delay_cntrl_1);
    rx_delay_cntrl_1->rxd7_del_sel = RU_FIELD_GET(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_1, RXD7_DEL_SEL, reg_rx_delay_cntrl_1);
    rx_delay_cntrl_1->rxd6_del_ovrd_en = RU_FIELD_GET(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_1, RXD6_DEL_OVRD_EN, reg_rx_delay_cntrl_1);
    rx_delay_cntrl_1->rxd6_del_sel = RU_FIELD_GET(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_1, RXD6_DEL_SEL, reg_rx_delay_cntrl_1);
    rx_delay_cntrl_1->rxd5_del_ovrd_en = RU_FIELD_GET(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_1, RXD5_DEL_OVRD_EN, reg_rx_delay_cntrl_1);
    rx_delay_cntrl_1->rxd5_del_sel = RU_FIELD_GET(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_1, RXD5_DEL_SEL, reg_rx_delay_cntrl_1);
    rx_delay_cntrl_1->rxd4_del_ovrd_en = RU_FIELD_GET(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_1, RXD4_DEL_OVRD_EN, reg_rx_delay_cntrl_1);
    rx_delay_cntrl_1->rxd4_del_sel = RU_FIELD_GET(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_1, RXD4_DEL_SEL, reg_rx_delay_cntrl_1);

    return 0;
}

int ag_drv_lport_rgmii_rx_delay_cntrl_2_set(uint8_t rgmii_id, const lport_rgmii_rx_delay_cntrl_2 *rx_delay_cntrl_2)
{
    uint32_t reg_rx_delay_cntrl_2=0;

#ifdef VALIDATE_PARMS
    if(!rx_delay_cntrl_2)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((rgmii_id >= BLOCK_ADDR_COUNT) ||
       (rx_delay_cntrl_2->rxclk_del_ovrd_en >= _1BITS_MAX_VAL_) ||
       (rx_delay_cntrl_2->rxclk_del_sel >= _4BITS_MAX_VAL_) ||
       (rx_delay_cntrl_2->rxctl_neg_del_ovrd_en >= _1BITS_MAX_VAL_) ||
       (rx_delay_cntrl_2->rxctl_neg_del_sel >= _6BITS_MAX_VAL_) ||
       (rx_delay_cntrl_2->rxctl_pos_del_ovrd_en >= _1BITS_MAX_VAL_) ||
       (rx_delay_cntrl_2->rxctl_pos_del_sel >= _6BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_rx_delay_cntrl_2 = RU_FIELD_SET(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_2, RXCLK_DEL_OVRD_EN, reg_rx_delay_cntrl_2, rx_delay_cntrl_2->rxclk_del_ovrd_en);
    reg_rx_delay_cntrl_2 = RU_FIELD_SET(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_2, RXCLK_DEL_SEL, reg_rx_delay_cntrl_2, rx_delay_cntrl_2->rxclk_del_sel);
    reg_rx_delay_cntrl_2 = RU_FIELD_SET(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_2, RXCTL_NEG_DEL_OVRD_EN, reg_rx_delay_cntrl_2, rx_delay_cntrl_2->rxctl_neg_del_ovrd_en);
    reg_rx_delay_cntrl_2 = RU_FIELD_SET(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_2, RXCTL_NEG_DEL_SEL, reg_rx_delay_cntrl_2, rx_delay_cntrl_2->rxctl_neg_del_sel);
    reg_rx_delay_cntrl_2 = RU_FIELD_SET(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_2, RXCTL_POS_DEL_OVRD_EN, reg_rx_delay_cntrl_2, rx_delay_cntrl_2->rxctl_pos_del_ovrd_en);
    reg_rx_delay_cntrl_2 = RU_FIELD_SET(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_2, RXCTL_POS_DEL_SEL, reg_rx_delay_cntrl_2, rx_delay_cntrl_2->rxctl_pos_del_sel);

    RU_REG_WRITE(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_2, reg_rx_delay_cntrl_2);

    return 0;
}

int ag_drv_lport_rgmii_rx_delay_cntrl_2_get(uint8_t rgmii_id, lport_rgmii_rx_delay_cntrl_2 *rx_delay_cntrl_2)
{
    uint32_t reg_rx_delay_cntrl_2=0;

#ifdef VALIDATE_PARMS
    if(!rx_delay_cntrl_2)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((rgmii_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_2, reg_rx_delay_cntrl_2);

    rx_delay_cntrl_2->rxclk_del_ovrd_en = RU_FIELD_GET(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_2, RXCLK_DEL_OVRD_EN, reg_rx_delay_cntrl_2);
    rx_delay_cntrl_2->rxclk_del_sel = RU_FIELD_GET(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_2, RXCLK_DEL_SEL, reg_rx_delay_cntrl_2);
    rx_delay_cntrl_2->rxctl_neg_del_ovrd_en = RU_FIELD_GET(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_2, RXCTL_NEG_DEL_OVRD_EN, reg_rx_delay_cntrl_2);
    rx_delay_cntrl_2->rxctl_neg_del_sel = RU_FIELD_GET(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_2, RXCTL_NEG_DEL_SEL, reg_rx_delay_cntrl_2);
    rx_delay_cntrl_2->rxctl_pos_del_ovrd_en = RU_FIELD_GET(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_2, RXCTL_POS_DEL_OVRD_EN, reg_rx_delay_cntrl_2);
    rx_delay_cntrl_2->rxctl_pos_del_sel = RU_FIELD_GET(rgmii_id, LPORT_RGMII, RX_DELAY_CNTRL_2, RXCTL_POS_DEL_SEL, reg_rx_delay_cntrl_2);

    return 0;
}

