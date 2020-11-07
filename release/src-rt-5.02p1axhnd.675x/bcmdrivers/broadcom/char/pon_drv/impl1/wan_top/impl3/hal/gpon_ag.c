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
int ag_drv_gpon_gearbox_0_set(const gpon_gearbox_0 *gearbox_0)
{
    uint32_t reg_gearbox_0=0;

#ifdef VALIDATE_PARMS
    if(!gearbox_0)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_ptg_status2_sel >= _1BITS_MAX_VAL_) ||
       (gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_ptg_status1_sel >= _1BITS_MAX_VAL_) ||
       (gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_fifo_status_sel >= _1BITS_MAX_VAL_) ||
       (gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_txlbe_bit_order >= _1BITS_MAX_VAL_) ||
       (gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_rx_16bit_order >= _1BITS_MAX_VAL_) ||
       (gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_tx_8bit_order >= _1BITS_MAX_VAL_) ||
       (gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_tx_16bit_order >= _1BITS_MAX_VAL_) ||
       (gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_tx_pointer_distance_min >= _5BITS_MAX_VAL_) ||
       (gearbox_0->cr_wan_top_wan_misc_gpon_gearbox_0_fifo_cfg_0_asym_loopback >= _1BITS_MAX_VAL_) ||
       (gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_tx_pointer_distance_max >= _5BITS_MAX_VAL_) ||
       (gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_tx_bit_inv >= _1BITS_MAX_VAL_) ||
       (gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_tx_wr_ptr_dly >= _1BITS_MAX_VAL_) ||
       (gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_tx_wr_ptr_adv >= _1BITS_MAX_VAL_) ||
       (gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_clear_txfifo_collision >= _1BITS_MAX_VAL_) ||
       (gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_loopback_rx >= _1BITS_MAX_VAL_) ||
       (gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_clear_txfifo_drifted >= _1BITS_MAX_VAL_) ||
       (gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_sw_reset_txfifo_reset >= _1BITS_MAX_VAL_) ||
       (gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_sw_reset_txpg_reset >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gearbox_0 = RU_FIELD_SET(0, GPON, GEARBOX_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_PTG_STATUS2_SEL, reg_gearbox_0, gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_ptg_status2_sel);
    reg_gearbox_0 = RU_FIELD_SET(0, GPON, GEARBOX_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_PTG_STATUS1_SEL, reg_gearbox_0, gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_ptg_status1_sel);
    reg_gearbox_0 = RU_FIELD_SET(0, GPON, GEARBOX_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_STATUS_SEL, reg_gearbox_0, gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_fifo_status_sel);
    reg_gearbox_0 = RU_FIELD_SET(0, GPON, GEARBOX_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_CFG_0_TXLBE_BIT_ORDER, reg_gearbox_0, gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_txlbe_bit_order);
    reg_gearbox_0 = RU_FIELD_SET(0, GPON, GEARBOX_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_CFG_0_RX_16BIT_ORDER, reg_gearbox_0, gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_rx_16bit_order);
    reg_gearbox_0 = RU_FIELD_SET(0, GPON, GEARBOX_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_CFG_0_TX_8BIT_ORDER, reg_gearbox_0, gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_tx_8bit_order);
    reg_gearbox_0 = RU_FIELD_SET(0, GPON, GEARBOX_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_CFG_0_TX_16BIT_ORDER, reg_gearbox_0, gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_tx_16bit_order);
    reg_gearbox_0 = RU_FIELD_SET(0, GPON, GEARBOX_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_CFG_0_TX_POINTER_DISTANCE_MIN, reg_gearbox_0, gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_tx_pointer_distance_min);
    reg_gearbox_0 = RU_FIELD_SET(0, GPON, GEARBOX_0, CR_WAN_TOP_WAN_MISC_GPON_GEARBOX_0_FIFO_CFG_0_ASYM_LOOPBACK, reg_gearbox_0, gearbox_0->cr_wan_top_wan_misc_gpon_gearbox_0_fifo_cfg_0_asym_loopback);
    reg_gearbox_0 = RU_FIELD_SET(0, GPON, GEARBOX_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_CFG_0_TX_POINTER_DISTANCE_MAX, reg_gearbox_0, gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_tx_pointer_distance_max);
    reg_gearbox_0 = RU_FIELD_SET(0, GPON, GEARBOX_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_CFG_0_TX_BIT_INV, reg_gearbox_0, gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_tx_bit_inv);
    reg_gearbox_0 = RU_FIELD_SET(0, GPON, GEARBOX_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_CFG_0_TX_WR_PTR_DLY, reg_gearbox_0, gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_tx_wr_ptr_dly);
    reg_gearbox_0 = RU_FIELD_SET(0, GPON, GEARBOX_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_CFG_0_TX_WR_PTR_ADV, reg_gearbox_0, gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_tx_wr_ptr_adv);
    reg_gearbox_0 = RU_FIELD_SET(0, GPON, GEARBOX_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_CFG_0_CLEAR_TXFIFO_COLLISION, reg_gearbox_0, gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_clear_txfifo_collision);
    reg_gearbox_0 = RU_FIELD_SET(0, GPON, GEARBOX_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_CFG_0_LOOPBACK_RX, reg_gearbox_0, gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_loopback_rx);
    reg_gearbox_0 = RU_FIELD_SET(0, GPON, GEARBOX_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_CFG_0_CLEAR_TXFIFO_DRIFTED, reg_gearbox_0, gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_clear_txfifo_drifted);
    reg_gearbox_0 = RU_FIELD_SET(0, GPON, GEARBOX_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_SW_RESET_TXFIFO_RESET, reg_gearbox_0, gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_sw_reset_txfifo_reset);
    reg_gearbox_0 = RU_FIELD_SET(0, GPON, GEARBOX_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_SW_RESET_TXPG_RESET, reg_gearbox_0, gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_sw_reset_txpg_reset);

    RU_REG_WRITE(0, GPON, GEARBOX_0, reg_gearbox_0);

    return 0;
}

int ag_drv_gpon_gearbox_0_get(gpon_gearbox_0 *gearbox_0)
{
    uint32_t reg_gearbox_0=0;

#ifdef VALIDATE_PARMS
    if(!gearbox_0)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, GPON, GEARBOX_0, reg_gearbox_0);

    gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_ptg_status2_sel = RU_FIELD_GET(0, GPON, GEARBOX_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_PTG_STATUS2_SEL, reg_gearbox_0);
    gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_ptg_status1_sel = RU_FIELD_GET(0, GPON, GEARBOX_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_PTG_STATUS1_SEL, reg_gearbox_0);
    gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_fifo_status_sel = RU_FIELD_GET(0, GPON, GEARBOX_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_STATUS_SEL, reg_gearbox_0);
    gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_txlbe_bit_order = RU_FIELD_GET(0, GPON, GEARBOX_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_CFG_0_TXLBE_BIT_ORDER, reg_gearbox_0);
    gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_rx_16bit_order = RU_FIELD_GET(0, GPON, GEARBOX_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_CFG_0_RX_16BIT_ORDER, reg_gearbox_0);
    gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_tx_8bit_order = RU_FIELD_GET(0, GPON, GEARBOX_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_CFG_0_TX_8BIT_ORDER, reg_gearbox_0);
    gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_tx_16bit_order = RU_FIELD_GET(0, GPON, GEARBOX_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_CFG_0_TX_16BIT_ORDER, reg_gearbox_0);
    gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_tx_pointer_distance_min = RU_FIELD_GET(0, GPON, GEARBOX_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_CFG_0_TX_POINTER_DISTANCE_MIN, reg_gearbox_0);
    gearbox_0->cr_wan_top_wan_misc_gpon_gearbox_0_fifo_cfg_0_asym_loopback = RU_FIELD_GET(0, GPON, GEARBOX_0, CR_WAN_TOP_WAN_MISC_GPON_GEARBOX_0_FIFO_CFG_0_ASYM_LOOPBACK, reg_gearbox_0);
    gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_tx_pointer_distance_max = RU_FIELD_GET(0, GPON, GEARBOX_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_CFG_0_TX_POINTER_DISTANCE_MAX, reg_gearbox_0);
    gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_tx_bit_inv = RU_FIELD_GET(0, GPON, GEARBOX_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_CFG_0_TX_BIT_INV, reg_gearbox_0);
    gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_tx_wr_ptr_dly = RU_FIELD_GET(0, GPON, GEARBOX_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_CFG_0_TX_WR_PTR_DLY, reg_gearbox_0);
    gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_tx_wr_ptr_adv = RU_FIELD_GET(0, GPON, GEARBOX_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_CFG_0_TX_WR_PTR_ADV, reg_gearbox_0);
    gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_clear_txfifo_collision = RU_FIELD_GET(0, GPON, GEARBOX_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_CFG_0_CLEAR_TXFIFO_COLLISION, reg_gearbox_0);
    gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_loopback_rx = RU_FIELD_GET(0, GPON, GEARBOX_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_CFG_0_LOOPBACK_RX, reg_gearbox_0);
    gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_clear_txfifo_drifted = RU_FIELD_GET(0, GPON, GEARBOX_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_CFG_0_CLEAR_TXFIFO_DRIFTED, reg_gearbox_0);
    gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_sw_reset_txfifo_reset = RU_FIELD_GET(0, GPON, GEARBOX_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_SW_RESET_TXFIFO_RESET, reg_gearbox_0);
    gearbox_0->cr_xgwan_top_wan_misc_gpon_gearbox_sw_reset_txpg_reset = RU_FIELD_GET(0, GPON, GEARBOX_0, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_SW_RESET_TXPG_RESET, reg_gearbox_0);

    return 0;
}

int ag_drv_gpon_pattern_cfg1_set(const gpon_pattern_cfg1 *pattern_cfg1)
{
    uint32_t reg_pattern_cfg1=0;

#ifdef VALIDATE_PARMS
    if(!pattern_cfg1)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((pattern_cfg1->cr_xgwan_top_wan_misc_gpon_gearbox_pattern_cfg1_pg_mode >= _3BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_pattern_cfg1 = RU_FIELD_SET(0, GPON, PATTERN_CFG1, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_PATTERN_CFG1_FILLER, reg_pattern_cfg1, pattern_cfg1->cr_xgwan_top_wan_misc_gpon_gearbox_pattern_cfg1_filler);
    reg_pattern_cfg1 = RU_FIELD_SET(0, GPON, PATTERN_CFG1, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_PATTERN_CFG1_PAYLOAD, reg_pattern_cfg1, pattern_cfg1->cr_xgwan_top_wan_misc_gpon_gearbox_pattern_cfg1_payload);
    reg_pattern_cfg1 = RU_FIELD_SET(0, GPON, PATTERN_CFG1, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_PATTERN_CFG1_HEADER, reg_pattern_cfg1, pattern_cfg1->cr_xgwan_top_wan_misc_gpon_gearbox_pattern_cfg1_header);
    reg_pattern_cfg1 = RU_FIELD_SET(0, GPON, PATTERN_CFG1, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_PATTERN_CFG1_PG_MODE, reg_pattern_cfg1, pattern_cfg1->cr_xgwan_top_wan_misc_gpon_gearbox_pattern_cfg1_pg_mode);

    RU_REG_WRITE(0, GPON, PATTERN_CFG1, reg_pattern_cfg1);

    return 0;
}

int ag_drv_gpon_pattern_cfg1_get(gpon_pattern_cfg1 *pattern_cfg1)
{
    uint32_t reg_pattern_cfg1=0;

#ifdef VALIDATE_PARMS
    if(!pattern_cfg1)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, GPON, PATTERN_CFG1, reg_pattern_cfg1);

    pattern_cfg1->cr_xgwan_top_wan_misc_gpon_gearbox_pattern_cfg1_filler = RU_FIELD_GET(0, GPON, PATTERN_CFG1, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_PATTERN_CFG1_FILLER, reg_pattern_cfg1);
    pattern_cfg1->cr_xgwan_top_wan_misc_gpon_gearbox_pattern_cfg1_payload = RU_FIELD_GET(0, GPON, PATTERN_CFG1, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_PATTERN_CFG1_PAYLOAD, reg_pattern_cfg1);
    pattern_cfg1->cr_xgwan_top_wan_misc_gpon_gearbox_pattern_cfg1_header = RU_FIELD_GET(0, GPON, PATTERN_CFG1, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_PATTERN_CFG1_HEADER, reg_pattern_cfg1);
    pattern_cfg1->cr_xgwan_top_wan_misc_gpon_gearbox_pattern_cfg1_pg_mode = RU_FIELD_GET(0, GPON, PATTERN_CFG1, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_PATTERN_CFG1_PG_MODE, reg_pattern_cfg1);

    return 0;
}

int ag_drv_gpon_pattern_cfg2_set(uint8_t cr_xgwan_top_wan_misc_gpon_gearbox_pattern_cfg2_gap_size, uint8_t cr_xgwan_top_wan_misc_gpon_gearbox_pattern_cfg2_burst_size)
{
    uint32_t reg_pattern_cfg2=0;

#ifdef VALIDATE_PARMS
#endif

    reg_pattern_cfg2 = RU_FIELD_SET(0, GPON, PATTERN_CFG2, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_PATTERN_CFG2_GAP_SIZE, reg_pattern_cfg2, cr_xgwan_top_wan_misc_gpon_gearbox_pattern_cfg2_gap_size);
    reg_pattern_cfg2 = RU_FIELD_SET(0, GPON, PATTERN_CFG2, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_PATTERN_CFG2_BURST_SIZE, reg_pattern_cfg2, cr_xgwan_top_wan_misc_gpon_gearbox_pattern_cfg2_burst_size);

    RU_REG_WRITE(0, GPON, PATTERN_CFG2, reg_pattern_cfg2);

    return 0;
}

int ag_drv_gpon_pattern_cfg2_get(uint8_t *cr_xgwan_top_wan_misc_gpon_gearbox_pattern_cfg2_gap_size, uint8_t *cr_xgwan_top_wan_misc_gpon_gearbox_pattern_cfg2_burst_size)
{
    uint32_t reg_pattern_cfg2=0;

#ifdef VALIDATE_PARMS
    if(!cr_xgwan_top_wan_misc_gpon_gearbox_pattern_cfg2_gap_size || !cr_xgwan_top_wan_misc_gpon_gearbox_pattern_cfg2_burst_size)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, GPON, PATTERN_CFG2, reg_pattern_cfg2);

    *cr_xgwan_top_wan_misc_gpon_gearbox_pattern_cfg2_gap_size = RU_FIELD_GET(0, GPON, PATTERN_CFG2, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_PATTERN_CFG2_GAP_SIZE, reg_pattern_cfg2);
    *cr_xgwan_top_wan_misc_gpon_gearbox_pattern_cfg2_burst_size = RU_FIELD_GET(0, GPON, PATTERN_CFG2, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_PATTERN_CFG2_BURST_SIZE, reg_pattern_cfg2);

    return 0;
}

int ag_drv_gpon_gearbox_2_set(const gpon_gearbox_2 *gearbox_2)
{
    uint32_t reg_gearbox_2=0;

#ifdef VALIDATE_PARMS
    if(!gearbox_2)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((gearbox_2->cr_xgwan_top_wan_misc_gpon_gearbox_config_burst_delay_cyc >= _4BITS_MAX_VAL_) ||
       (gearbox_2->cr_xgwan_top_wan_misc_gpon_gearbox_tx_vld_delay_cyc >= _3BITS_MAX_VAL_) ||
       (gearbox_2->cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_1_tx_wr_pointer >= _5BITS_MAX_VAL_) ||
       (gearbox_2->cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_1_tx_rd_pointer >= _5BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gearbox_2 = RU_FIELD_SET(0, GPON, GEARBOX_2, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_CONFIG_BURST_DELAY_CYC, reg_gearbox_2, gearbox_2->cr_xgwan_top_wan_misc_gpon_gearbox_config_burst_delay_cyc);
    reg_gearbox_2 = RU_FIELD_SET(0, GPON, GEARBOX_2, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_TX_VLD_DELAY_CYC, reg_gearbox_2, gearbox_2->cr_xgwan_top_wan_misc_gpon_gearbox_tx_vld_delay_cyc);
    reg_gearbox_2 = RU_FIELD_SET(0, GPON, GEARBOX_2, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_CFG_1_TX_WR_POINTER, reg_gearbox_2, gearbox_2->cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_1_tx_wr_pointer);
    reg_gearbox_2 = RU_FIELD_SET(0, GPON, GEARBOX_2, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_CFG_1_TX_RD_POINTER, reg_gearbox_2, gearbox_2->cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_1_tx_rd_pointer);

    RU_REG_WRITE(0, GPON, GEARBOX_2, reg_gearbox_2);

    return 0;
}

int ag_drv_gpon_gearbox_2_get(gpon_gearbox_2 *gearbox_2)
{
    uint32_t reg_gearbox_2=0;

#ifdef VALIDATE_PARMS
    if(!gearbox_2)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, GPON, GEARBOX_2, reg_gearbox_2);

    gearbox_2->cr_xgwan_top_wan_misc_gpon_gearbox_config_burst_delay_cyc = RU_FIELD_GET(0, GPON, GEARBOX_2, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_CONFIG_BURST_DELAY_CYC, reg_gearbox_2);
    gearbox_2->cr_xgwan_top_wan_misc_gpon_gearbox_tx_vld_delay_cyc = RU_FIELD_GET(0, GPON, GEARBOX_2, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_TX_VLD_DELAY_CYC, reg_gearbox_2);
    gearbox_2->cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_1_tx_wr_pointer = RU_FIELD_GET(0, GPON, GEARBOX_2, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_CFG_1_TX_WR_POINTER, reg_gearbox_2);
    gearbox_2->cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_1_tx_rd_pointer = RU_FIELD_GET(0, GPON, GEARBOX_2, CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_FIFO_CFG_1_TX_RD_POINTER, reg_gearbox_2);

    return 0;
}

