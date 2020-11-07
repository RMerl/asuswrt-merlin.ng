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
#include "misc_ag.h"
int ag_drv_misc_misc_0_set(const misc_misc_0 *misc_0)
{
    uint32_t reg_0=0;

#ifdef VALIDATE_PARMS
    if(!misc_0)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((misc_0->onu2g_pmd_status_sel >= _1BITS_MAX_VAL_) ||
       (misc_0->refin_en >= _1BITS_MAX_VAL_) ||
       (misc_0->refout_en >= _1BITS_MAX_VAL_) ||
       (misc_0->mdio_mode >= _1BITS_MAX_VAL_) ||
       (misc_0->mdio_fast_mode >= _1BITS_MAX_VAL_) ||
       (misc_0->epon_tx_fifo_off_ld >= _1BITS_MAX_VAL_) ||
       (misc_0->onu2g_phya >= _5BITS_MAX_VAL_) ||
       (misc_0->epon_gbox_rx_width_mode >= _1BITS_MAX_VAL_) ||
       (misc_0->epon_ae_2p5_full_rate_mode >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_0 = RU_FIELD_SET(0, MISC, 0, ONU2G_PMD_STATUS_SEL, reg_0, misc_0->onu2g_pmd_status_sel);
    reg_0 = RU_FIELD_SET(0, MISC, 0, REFIN_EN, reg_0, misc_0->refin_en);
    reg_0 = RU_FIELD_SET(0, MISC, 0, REFOUT_EN, reg_0, misc_0->refout_en);
    reg_0 = RU_FIELD_SET(0, MISC, 0, MDIO_MODE, reg_0, misc_0->mdio_mode);
    reg_0 = RU_FIELD_SET(0, MISC, 0, MDIO_FAST_MODE, reg_0, misc_0->mdio_fast_mode);
    reg_0 = RU_FIELD_SET(0, MISC, 0, EPON_TX_FIFO_OFF_LD, reg_0, misc_0->epon_tx_fifo_off_ld);
    reg_0 = RU_FIELD_SET(0, MISC, 0, ONU2G_PHYA, reg_0, misc_0->onu2g_phya);
    reg_0 = RU_FIELD_SET(0, MISC, 0, EPON_GBOX_RX_WIDTH_MODE, reg_0, misc_0->epon_gbox_rx_width_mode);
    reg_0 = RU_FIELD_SET(0, MISC, 0, EPON_AE_2P5_FULL_RATE_MODE, reg_0, misc_0->epon_ae_2p5_full_rate_mode);
    reg_0 = RU_FIELD_SET(0, MISC, 0, PMD_LANE_MODE, reg_0, misc_0->pmd_lane_mode);

    RU_REG_WRITE(0, MISC, 0, reg_0);

    return 0;
}

int ag_drv_misc_misc_0_get(misc_misc_0 *misc_0)
{
    uint32_t reg_0=0;

#ifdef VALIDATE_PARMS
    if(!misc_0)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, MISC, 0, reg_0);

    misc_0->onu2g_pmd_status_sel = RU_FIELD_GET(0, MISC, 0, ONU2G_PMD_STATUS_SEL, reg_0);
    misc_0->refin_en = RU_FIELD_GET(0, MISC, 0, REFIN_EN, reg_0);
    misc_0->refout_en = RU_FIELD_GET(0, MISC, 0, REFOUT_EN, reg_0);
    misc_0->mdio_mode = RU_FIELD_GET(0, MISC, 0, MDIO_MODE, reg_0);
    misc_0->mdio_fast_mode = RU_FIELD_GET(0, MISC, 0, MDIO_FAST_MODE, reg_0);
    misc_0->epon_tx_fifo_off_ld = RU_FIELD_GET(0, MISC, 0, EPON_TX_FIFO_OFF_LD, reg_0);
    misc_0->onu2g_phya = RU_FIELD_GET(0, MISC, 0, ONU2G_PHYA, reg_0);
    misc_0->epon_gbox_rx_width_mode = RU_FIELD_GET(0, MISC, 0, EPON_GBOX_RX_WIDTH_MODE, reg_0);
    misc_0->epon_ae_2p5_full_rate_mode = RU_FIELD_GET(0, MISC, 0, EPON_AE_2P5_FULL_RATE_MODE, reg_0);
    misc_0->pmd_lane_mode = RU_FIELD_GET(0, MISC, 0, PMD_LANE_MODE, reg_0);

    return 0;
}

int ag_drv_misc_misc_1_set(uint16_t pmd_core_1_mode, uint16_t pmd_core_0_mode)
{
    uint32_t reg_1=0;

#ifdef VALIDATE_PARMS
#endif

    reg_1 = RU_FIELD_SET(0, MISC, 1, PMD_CORE_0_MODE, reg_1, pmd_core_0_mode);
    reg_1 = RU_FIELD_SET(0, MISC, 1, PMD_CORE_1_MODE, reg_1, pmd_core_1_mode);

    RU_REG_WRITE(0, MISC, 1, reg_1);

    return 0;
}

int ag_drv_misc_misc_1_get(uint16_t *pmd_core_1_mode, uint16_t *pmd_core_0_mode)
{
    uint32_t reg_1=0;

#ifdef VALIDATE_PARMS
    if(!pmd_core_0_mode || !pmd_core_1_mode)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, MISC, 1, reg_1);

    *pmd_core_0_mode = RU_FIELD_GET(0, MISC, 1, PMD_CORE_0_MODE, reg_1);
    *pmd_core_1_mode = RU_FIELD_GET(0, MISC, 1, PMD_CORE_1_MODE, reg_1);

    return 0;
}

int ag_drv_misc_misc_2_set(const misc_misc_2 *misc_2)
{
    uint32_t reg_2=0;

#ifdef VALIDATE_PARMS
    if(!misc_2)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((misc_2->pmd_rx_mode >= _1BITS_MAX_VAL_) ||
       (misc_2->pmd_ln_dp_h_rstb >= _1BITS_MAX_VAL_) ||
       (misc_2->pmd_ln_h_rstb >= _1BITS_MAX_VAL_) ||
       (misc_2->pmd_core_0_dp_h_rstb >= _1BITS_MAX_VAL_) ||
       (misc_2->pmd_core_1_dp_h_rstb >= _1BITS_MAX_VAL_) ||
       (misc_2->pmd_por_h_rstb >= _1BITS_MAX_VAL_) ||
       (misc_2->pmd_ext_los >= _1BITS_MAX_VAL_) ||
       (misc_2->pmd_ln_tx_h_pwrdn >= _1BITS_MAX_VAL_) ||
       (misc_2->pmd_ln_rx_h_pwrdn >= _1BITS_MAX_VAL_) ||
       (misc_2->pmd_tx_disable >= _1BITS_MAX_VAL_) ||
       (misc_2->pmd_tx_osr_mode >= _4BITS_MAX_VAL_) ||
       (misc_2->pmd_tx_mode >= _2BITS_MAX_VAL_) ||
       (misc_2->pmd_rx_osr_mode >= _4BITS_MAX_VAL_) ||
       (misc_2->cfgactiveethernet2p5 >= _1BITS_MAX_VAL_) ||
       (misc_2->cfgngpontxclk >= _2BITS_MAX_VAL_) ||
       (misc_2->cfgngponrxclk >= _2BITS_MAX_VAL_) ||
       (misc_2->cfg_apm_mux_sel_0 >= _1BITS_MAX_VAL_) ||
       (misc_2->cfg_apm_mux_sel_1 >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_2 = RU_FIELD_SET(0, MISC, 2, PMD_RX_MODE, reg_2, misc_2->pmd_rx_mode);
    reg_2 = RU_FIELD_SET(0, MISC, 2, PMD_LN_DP_H_RSTB, reg_2, misc_2->pmd_ln_dp_h_rstb);
    reg_2 = RU_FIELD_SET(0, MISC, 2, PMD_LN_H_RSTB, reg_2, misc_2->pmd_ln_h_rstb);
    reg_2 = RU_FIELD_SET(0, MISC, 2, PMD_CORE_0_DP_H_RSTB, reg_2, misc_2->pmd_core_0_dp_h_rstb);
    reg_2 = RU_FIELD_SET(0, MISC, 2, PMD_CORE_1_DP_H_RSTB, reg_2, misc_2->pmd_core_1_dp_h_rstb);
    reg_2 = RU_FIELD_SET(0, MISC, 2, PMD_POR_H_RSTB, reg_2, misc_2->pmd_por_h_rstb);
    reg_2 = RU_FIELD_SET(0, MISC, 2, PMD_EXT_LOS, reg_2, misc_2->pmd_ext_los);
    reg_2 = RU_FIELD_SET(0, MISC, 2, PMD_LN_TX_H_PWRDN, reg_2, misc_2->pmd_ln_tx_h_pwrdn);
    reg_2 = RU_FIELD_SET(0, MISC, 2, PMD_LN_RX_H_PWRDN, reg_2, misc_2->pmd_ln_rx_h_pwrdn);
    reg_2 = RU_FIELD_SET(0, MISC, 2, PMD_TX_DISABLE, reg_2, misc_2->pmd_tx_disable);
    reg_2 = RU_FIELD_SET(0, MISC, 2, PMD_TX_OSR_MODE, reg_2, misc_2->pmd_tx_osr_mode);
    reg_2 = RU_FIELD_SET(0, MISC, 2, PMD_TX_MODE, reg_2, misc_2->pmd_tx_mode);
    reg_2 = RU_FIELD_SET(0, MISC, 2, PMD_RX_OSR_MODE, reg_2, misc_2->pmd_rx_osr_mode);
    reg_2 = RU_FIELD_SET(0, MISC, 2, CFGACTIVEETHERNET2P5, reg_2, misc_2->cfgactiveethernet2p5);
    reg_2 = RU_FIELD_SET(0, MISC, 2, CFGNGPONTXCLK, reg_2, misc_2->cfgngpontxclk);
    reg_2 = RU_FIELD_SET(0, MISC, 2, CFGNGPONRXCLK, reg_2, misc_2->cfgngponrxclk);
    reg_2 = RU_FIELD_SET(0, MISC, 2, CFG_APM_MUX_SEL_0, reg_2, misc_2->cfg_apm_mux_sel_0);
    reg_2 = RU_FIELD_SET(0, MISC, 2, CFG_APM_MUX_SEL_1, reg_2, misc_2->cfg_apm_mux_sel_1);

    RU_REG_WRITE(0, MISC, 2, reg_2);

    return 0;
}

int ag_drv_misc_misc_2_get(misc_misc_2 *misc_2)
{
    uint32_t reg_2=0;

#ifdef VALIDATE_PARMS
    if(!misc_2)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, MISC, 2, reg_2);

    misc_2->pmd_rx_mode = RU_FIELD_GET(0, MISC, 2, PMD_RX_MODE, reg_2);
    misc_2->pmd_ln_dp_h_rstb = RU_FIELD_GET(0, MISC, 2, PMD_LN_DP_H_RSTB, reg_2);
    misc_2->pmd_ln_h_rstb = RU_FIELD_GET(0, MISC, 2, PMD_LN_H_RSTB, reg_2);
    misc_2->pmd_core_0_dp_h_rstb = RU_FIELD_GET(0, MISC, 2, PMD_CORE_0_DP_H_RSTB, reg_2);
    misc_2->pmd_core_1_dp_h_rstb = RU_FIELD_GET(0, MISC, 2, PMD_CORE_1_DP_H_RSTB, reg_2);
    misc_2->pmd_por_h_rstb = RU_FIELD_GET(0, MISC, 2, PMD_POR_H_RSTB, reg_2);
    misc_2->pmd_ext_los = RU_FIELD_GET(0, MISC, 2, PMD_EXT_LOS, reg_2);
    misc_2->pmd_ln_tx_h_pwrdn = RU_FIELD_GET(0, MISC, 2, PMD_LN_TX_H_PWRDN, reg_2);
    misc_2->pmd_ln_rx_h_pwrdn = RU_FIELD_GET(0, MISC, 2, PMD_LN_RX_H_PWRDN, reg_2);
    misc_2->pmd_tx_disable = RU_FIELD_GET(0, MISC, 2, PMD_TX_DISABLE, reg_2);
    misc_2->pmd_tx_osr_mode = RU_FIELD_GET(0, MISC, 2, PMD_TX_OSR_MODE, reg_2);
    misc_2->pmd_tx_mode = RU_FIELD_GET(0, MISC, 2, PMD_TX_MODE, reg_2);
    misc_2->pmd_rx_osr_mode = RU_FIELD_GET(0, MISC, 2, PMD_RX_OSR_MODE, reg_2);
    misc_2->cfgactiveethernet2p5 = RU_FIELD_GET(0, MISC, 2, CFGACTIVEETHERNET2P5, reg_2);
    misc_2->cfgngpontxclk = RU_FIELD_GET(0, MISC, 2, CFGNGPONTXCLK, reg_2);
    misc_2->cfgngponrxclk = RU_FIELD_GET(0, MISC, 2, CFGNGPONRXCLK, reg_2);
    misc_2->cfg_apm_mux_sel_0 = RU_FIELD_GET(0, MISC, 2, CFG_APM_MUX_SEL_0, reg_2);
    misc_2->cfg_apm_mux_sel_1 = RU_FIELD_GET(0, MISC, 2, CFG_APM_MUX_SEL_1, reg_2);

    return 0;
}

int ag_drv_misc_misc_3_set(const misc_misc_3 *misc_3)
{
    uint32_t reg_3=0;

#ifdef VALIDATE_PARMS
    if(!misc_3)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((misc_3->mem_reb >= _1BITS_MAX_VAL_) ||
       (misc_3->laser_invert >= _1BITS_MAX_VAL_) ||
       (misc_3->laser_mode >= _3BITS_MAX_VAL_) ||
       (misc_3->wan_interface_select >= _1BITS_MAX_VAL_) ||
       (misc_3->laser_oe >= _1BITS_MAX_VAL_) ||
       (misc_3->ntr_sync_period_sel >= _4BITS_MAX_VAL_) ||
       (misc_3->wan_debug_sel >= _4BITS_MAX_VAL_) ||
       (misc_3->epon_tx_fifo_off >= _6BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_3 = RU_FIELD_SET(0, MISC, 3, MEM_REB, reg_3, misc_3->mem_reb);
    reg_3 = RU_FIELD_SET(0, MISC, 3, LASER_INVERT, reg_3, misc_3->laser_invert);
    reg_3 = RU_FIELD_SET(0, MISC, 3, LASER_MODE, reg_3, misc_3->laser_mode);
    reg_3 = RU_FIELD_SET(0, MISC, 3, WAN_INTERFACE_SELECT, reg_3, misc_3->wan_interface_select);
    reg_3 = RU_FIELD_SET(0, MISC, 3, LASER_OE, reg_3, misc_3->laser_oe);
    reg_3 = RU_FIELD_SET(0, MISC, 3, NTR_SYNC_PERIOD_SEL, reg_3, misc_3->ntr_sync_period_sel);
    reg_3 = RU_FIELD_SET(0, MISC, 3, WAN_DEBUG_SEL, reg_3, misc_3->wan_debug_sel);
    reg_3 = RU_FIELD_SET(0, MISC, 3, EPON_DEBUG_SEL, reg_3, misc_3->epon_debug_sel);
    reg_3 = RU_FIELD_SET(0, MISC, 3, EPON_TX_FIFO_OFF, reg_3, misc_3->epon_tx_fifo_off);

    RU_REG_WRITE(0, MISC, 3, reg_3);

    return 0;
}

int ag_drv_misc_misc_3_get(misc_misc_3 *misc_3)
{
    uint32_t reg_3=0;

#ifdef VALIDATE_PARMS
    if(!misc_3)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, MISC, 3, reg_3);

    misc_3->mem_reb = RU_FIELD_GET(0, MISC, 3, MEM_REB, reg_3);
    misc_3->laser_invert = RU_FIELD_GET(0, MISC, 3, LASER_INVERT, reg_3);
    misc_3->laser_mode = RU_FIELD_GET(0, MISC, 3, LASER_MODE, reg_3);
    misc_3->wan_interface_select = RU_FIELD_GET(0, MISC, 3, WAN_INTERFACE_SELECT, reg_3);
    misc_3->laser_oe = RU_FIELD_GET(0, MISC, 3, LASER_OE, reg_3);
    misc_3->ntr_sync_period_sel = RU_FIELD_GET(0, MISC, 3, NTR_SYNC_PERIOD_SEL, reg_3);
    misc_3->wan_debug_sel = RU_FIELD_GET(0, MISC, 3, WAN_DEBUG_SEL, reg_3);
    misc_3->epon_debug_sel = RU_FIELD_GET(0, MISC, 3, EPON_DEBUG_SEL, reg_3);
    misc_3->epon_tx_fifo_off = RU_FIELD_GET(0, MISC, 3, EPON_TX_FIFO_OFF, reg_3);

    return 0;
}

