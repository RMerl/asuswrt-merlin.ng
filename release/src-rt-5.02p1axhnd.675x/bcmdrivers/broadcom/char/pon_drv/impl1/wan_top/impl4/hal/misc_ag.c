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
    if((misc_0->cr_xgwan_top_wan_misc_onu2g_phya >= _5BITS_MAX_VAL_) ||
       (misc_0->cr_xgwan_top_wan_misc_mdio_fast_mode >= _1BITS_MAX_VAL_) ||
       (misc_0->cr_xgwan_top_wan_misc_mdio_mode >= _1BITS_MAX_VAL_) ||
       (misc_0->cr_xgwan_top_wan_misc_refout_en >= _1BITS_MAX_VAL_) ||
       (misc_0->cr_xgwan_top_wan_misc_refin_en >= _1BITS_MAX_VAL_) ||
       (misc_0->cr_xgwan_top_wan_misc_onu2g_pmd_status_sel >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_0 = RU_FIELD_SET(0, MISC, 0, CR_XGWAN_TOP_WAN_MISC_PMD_LANE_MODE, reg_0, misc_0->cr_xgwan_top_wan_misc_pmd_lane_mode);
    reg_0 = RU_FIELD_SET(0, MISC, 0, CR_XGWAN_TOP_WAN_MISC_ONU2G_PHYA, reg_0, misc_0->cr_xgwan_top_wan_misc_onu2g_phya);
    reg_0 = RU_FIELD_SET(0, MISC, 0, CR_XGWAN_TOP_WAN_MISC_MDIO_FAST_MODE, reg_0, misc_0->cr_xgwan_top_wan_misc_mdio_fast_mode);
    reg_0 = RU_FIELD_SET(0, MISC, 0, CR_XGWAN_TOP_WAN_MISC_MDIO_MODE, reg_0, misc_0->cr_xgwan_top_wan_misc_mdio_mode);
    reg_0 = RU_FIELD_SET(0, MISC, 0, CR_XGWAN_TOP_WAN_MISC_REFOUT_EN, reg_0, misc_0->cr_xgwan_top_wan_misc_refout_en);
    reg_0 = RU_FIELD_SET(0, MISC, 0, CR_XGWAN_TOP_WAN_MISC_REFIN_EN, reg_0, misc_0->cr_xgwan_top_wan_misc_refin_en);
    reg_0 = RU_FIELD_SET(0, MISC, 0, CR_XGWAN_TOP_WAN_MISC_ONU2G_PMD_STATUS_SEL, reg_0, misc_0->cr_xgwan_top_wan_misc_onu2g_pmd_status_sel);

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

    misc_0->cr_xgwan_top_wan_misc_pmd_lane_mode = RU_FIELD_GET(0, MISC, 0, CR_XGWAN_TOP_WAN_MISC_PMD_LANE_MODE, reg_0);
    misc_0->cr_xgwan_top_wan_misc_onu2g_phya = RU_FIELD_GET(0, MISC, 0, CR_XGWAN_TOP_WAN_MISC_ONU2G_PHYA, reg_0);
    misc_0->cr_xgwan_top_wan_misc_mdio_fast_mode = RU_FIELD_GET(0, MISC, 0, CR_XGWAN_TOP_WAN_MISC_MDIO_FAST_MODE, reg_0);
    misc_0->cr_xgwan_top_wan_misc_mdio_mode = RU_FIELD_GET(0, MISC, 0, CR_XGWAN_TOP_WAN_MISC_MDIO_MODE, reg_0);
    misc_0->cr_xgwan_top_wan_misc_refout_en = RU_FIELD_GET(0, MISC, 0, CR_XGWAN_TOP_WAN_MISC_REFOUT_EN, reg_0);
    misc_0->cr_xgwan_top_wan_misc_refin_en = RU_FIELD_GET(0, MISC, 0, CR_XGWAN_TOP_WAN_MISC_REFIN_EN, reg_0);
    misc_0->cr_xgwan_top_wan_misc_onu2g_pmd_status_sel = RU_FIELD_GET(0, MISC, 0, CR_XGWAN_TOP_WAN_MISC_ONU2G_PMD_STATUS_SEL, reg_0);

    return 0;
}

int ag_drv_misc_misc_1_set(uint16_t cr_xgwan_top_wan_misc_pmd_core_1_mode, uint16_t cr_xgwan_top_wan_misc_pmd_core_0_mode)
{
    uint32_t reg_1=0;

#ifdef VALIDATE_PARMS
#endif

    reg_1 = RU_FIELD_SET(0, MISC, 1, CR_XGWAN_TOP_WAN_MISC_PMD_CORE_1_MODE, reg_1, cr_xgwan_top_wan_misc_pmd_core_1_mode);
    reg_1 = RU_FIELD_SET(0, MISC, 1, CR_XGWAN_TOP_WAN_MISC_PMD_CORE_0_MODE, reg_1, cr_xgwan_top_wan_misc_pmd_core_0_mode);

    RU_REG_WRITE(0, MISC, 1, reg_1);

    return 0;
}

int ag_drv_misc_misc_1_get(uint16_t *cr_xgwan_top_wan_misc_pmd_core_1_mode, uint16_t *cr_xgwan_top_wan_misc_pmd_core_0_mode)
{
    uint32_t reg_1=0;

#ifdef VALIDATE_PARMS
    if(!cr_xgwan_top_wan_misc_pmd_core_1_mode || !cr_xgwan_top_wan_misc_pmd_core_0_mode)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, MISC, 1, reg_1);

    *cr_xgwan_top_wan_misc_pmd_core_1_mode = RU_FIELD_GET(0, MISC, 1, CR_XGWAN_TOP_WAN_MISC_PMD_CORE_1_MODE, reg_1);
    *cr_xgwan_top_wan_misc_pmd_core_0_mode = RU_FIELD_GET(0, MISC, 1, CR_XGWAN_TOP_WAN_MISC_PMD_CORE_0_MODE, reg_1);

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
    if((misc_2->cr_xgwan_top_wan_misc_pmd_rx_osr_mode >= _4BITS_MAX_VAL_) ||
       (misc_2->cr_xgwan_top_wan_misc_pmd_tx_mode >= _2BITS_MAX_VAL_) ||
       (misc_2->cr_xgwan_top_wan_misc_pmd_tx_osr_mode >= _4BITS_MAX_VAL_) ||
       (misc_2->cr_xgwan_top_wan_misc_pmd_tx_disable >= _1BITS_MAX_VAL_) ||
       (misc_2->cr_xgwan_top_wan_misc_pmd_ln_rx_h_pwrdn >= _1BITS_MAX_VAL_) ||
       (misc_2->cr_xgwan_top_wan_misc_pmd_ln_tx_h_pwrdn >= _1BITS_MAX_VAL_) ||
       (misc_2->cr_xgwan_top_wan_misc_pmd_ext_los >= _1BITS_MAX_VAL_) ||
       (misc_2->cr_xgwan_top_wan_misc_pmd_por_h_rstb >= _1BITS_MAX_VAL_) ||
       (misc_2->cr_xgwan_top_wan_misc_pmd_core_1_dp_h_rstb >= _1BITS_MAX_VAL_) ||
       (misc_2->cr_xgwan_top_wan_misc_pmd_core_0_dp_h_rstb >= _1BITS_MAX_VAL_) ||
       (misc_2->cr_xgwan_top_wan_misc_pmd_ln_h_rstb >= _1BITS_MAX_VAL_) ||
       (misc_2->cr_xgwan_top_wan_misc_pmd_ln_dp_h_rstb >= _1BITS_MAX_VAL_) ||
       (misc_2->cr_xgwan_top_wan_misc_pmd_rx_mode >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_2 = RU_FIELD_SET(0, MISC, 2, CR_XGWAN_TOP_WAN_MISC_PMD_RX_OSR_MODE, reg_2, misc_2->cr_xgwan_top_wan_misc_pmd_rx_osr_mode);
    reg_2 = RU_FIELD_SET(0, MISC, 2, CR_XGWAN_TOP_WAN_MISC_PMD_TX_MODE, reg_2, misc_2->cr_xgwan_top_wan_misc_pmd_tx_mode);
    reg_2 = RU_FIELD_SET(0, MISC, 2, CR_XGWAN_TOP_WAN_MISC_PMD_TX_OSR_MODE, reg_2, misc_2->cr_xgwan_top_wan_misc_pmd_tx_osr_mode);
    reg_2 = RU_FIELD_SET(0, MISC, 2, CR_XGWAN_TOP_WAN_MISC_PMD_TX_DISABLE, reg_2, misc_2->cr_xgwan_top_wan_misc_pmd_tx_disable);
    reg_2 = RU_FIELD_SET(0, MISC, 2, CR_XGWAN_TOP_WAN_MISC_PMD_LN_RX_H_PWRDN, reg_2, misc_2->cr_xgwan_top_wan_misc_pmd_ln_rx_h_pwrdn);
    reg_2 = RU_FIELD_SET(0, MISC, 2, CR_XGWAN_TOP_WAN_MISC_PMD_LN_TX_H_PWRDN, reg_2, misc_2->cr_xgwan_top_wan_misc_pmd_ln_tx_h_pwrdn);
    reg_2 = RU_FIELD_SET(0, MISC, 2, CR_XGWAN_TOP_WAN_MISC_PMD_EXT_LOS, reg_2, misc_2->cr_xgwan_top_wan_misc_pmd_ext_los);
    reg_2 = RU_FIELD_SET(0, MISC, 2, CR_XGWAN_TOP_WAN_MISC_PMD_POR_H_RSTB, reg_2, misc_2->cr_xgwan_top_wan_misc_pmd_por_h_rstb);
    reg_2 = RU_FIELD_SET(0, MISC, 2, CR_XGWAN_TOP_WAN_MISC_PMD_CORE_1_DP_H_RSTB, reg_2, misc_2->cr_xgwan_top_wan_misc_pmd_core_1_dp_h_rstb);
    reg_2 = RU_FIELD_SET(0, MISC, 2, CR_XGWAN_TOP_WAN_MISC_PMD_CORE_0_DP_H_RSTB, reg_2, misc_2->cr_xgwan_top_wan_misc_pmd_core_0_dp_h_rstb);
    reg_2 = RU_FIELD_SET(0, MISC, 2, CR_XGWAN_TOP_WAN_MISC_PMD_LN_H_RSTB, reg_2, misc_2->cr_xgwan_top_wan_misc_pmd_ln_h_rstb);
    reg_2 = RU_FIELD_SET(0, MISC, 2, CR_XGWAN_TOP_WAN_MISC_PMD_LN_DP_H_RSTB, reg_2, misc_2->cr_xgwan_top_wan_misc_pmd_ln_dp_h_rstb);
    reg_2 = RU_FIELD_SET(0, MISC, 2, CR_XGWAN_TOP_WAN_MISC_PMD_RX_MODE, reg_2, misc_2->cr_xgwan_top_wan_misc_pmd_rx_mode);

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

    misc_2->cr_xgwan_top_wan_misc_pmd_rx_osr_mode = RU_FIELD_GET(0, MISC, 2, CR_XGWAN_TOP_WAN_MISC_PMD_RX_OSR_MODE, reg_2);
    misc_2->cr_xgwan_top_wan_misc_pmd_tx_mode = RU_FIELD_GET(0, MISC, 2, CR_XGWAN_TOP_WAN_MISC_PMD_TX_MODE, reg_2);
    misc_2->cr_xgwan_top_wan_misc_pmd_tx_osr_mode = RU_FIELD_GET(0, MISC, 2, CR_XGWAN_TOP_WAN_MISC_PMD_TX_OSR_MODE, reg_2);
    misc_2->cr_xgwan_top_wan_misc_pmd_tx_disable = RU_FIELD_GET(0, MISC, 2, CR_XGWAN_TOP_WAN_MISC_PMD_TX_DISABLE, reg_2);
    misc_2->cr_xgwan_top_wan_misc_pmd_ln_rx_h_pwrdn = RU_FIELD_GET(0, MISC, 2, CR_XGWAN_TOP_WAN_MISC_PMD_LN_RX_H_PWRDN, reg_2);
    misc_2->cr_xgwan_top_wan_misc_pmd_ln_tx_h_pwrdn = RU_FIELD_GET(0, MISC, 2, CR_XGWAN_TOP_WAN_MISC_PMD_LN_TX_H_PWRDN, reg_2);
    misc_2->cr_xgwan_top_wan_misc_pmd_ext_los = RU_FIELD_GET(0, MISC, 2, CR_XGWAN_TOP_WAN_MISC_PMD_EXT_LOS, reg_2);
    misc_2->cr_xgwan_top_wan_misc_pmd_por_h_rstb = RU_FIELD_GET(0, MISC, 2, CR_XGWAN_TOP_WAN_MISC_PMD_POR_H_RSTB, reg_2);
    misc_2->cr_xgwan_top_wan_misc_pmd_core_1_dp_h_rstb = RU_FIELD_GET(0, MISC, 2, CR_XGWAN_TOP_WAN_MISC_PMD_CORE_1_DP_H_RSTB, reg_2);
    misc_2->cr_xgwan_top_wan_misc_pmd_core_0_dp_h_rstb = RU_FIELD_GET(0, MISC, 2, CR_XGWAN_TOP_WAN_MISC_PMD_CORE_0_DP_H_RSTB, reg_2);
    misc_2->cr_xgwan_top_wan_misc_pmd_ln_h_rstb = RU_FIELD_GET(0, MISC, 2, CR_XGWAN_TOP_WAN_MISC_PMD_LN_H_RSTB, reg_2);
    misc_2->cr_xgwan_top_wan_misc_pmd_ln_dp_h_rstb = RU_FIELD_GET(0, MISC, 2, CR_XGWAN_TOP_WAN_MISC_PMD_LN_DP_H_RSTB, reg_2);
    misc_2->cr_xgwan_top_wan_misc_pmd_rx_mode = RU_FIELD_GET(0, MISC, 2, CR_XGWAN_TOP_WAN_MISC_PMD_RX_MODE, reg_2);

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
    if((misc_3->cr_xgwan_top_wan_misc_wan_cfg_wan_debug_sel >= _4BITS_MAX_VAL_) ||
       (misc_3->cfg_ntr_periph_pulse_bypass >= _1BITS_MAX_VAL_) ||
       (misc_3->cfg_ntr_gpio_pulse_bypass >= _1BITS_MAX_VAL_) ||
       (misc_3->cfg_ntr_src >= _2BITS_MAX_VAL_) ||
       (misc_3->cr_xgwan_top_wan_misc_wan_cfg_laser_oe >= _1BITS_MAX_VAL_) ||
       (misc_3->cr_xgwan_top_wan_misc_wan_cfg_wan_interface_select >= _3BITS_MAX_VAL_) ||
       (misc_3->cr_xgwan_top_wan_misc_wan_cfg_laser_mode >= _2BITS_MAX_VAL_) ||
       (misc_3->cr_xgwan_top_wan_misc_wan_cfg_laser_invert >= _1BITS_MAX_VAL_) ||
       (misc_3->cr_xgwan_top_wan_misc_wan_cfg_mem_reb >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_3 = RU_FIELD_SET(0, MISC, 3, CR_XGWAN_TOP_WAN_MISC_WAN_CFG_WAN_DEBUG_SEL, reg_3, misc_3->cr_xgwan_top_wan_misc_wan_cfg_wan_debug_sel);
    reg_3 = RU_FIELD_SET(0, MISC, 3, CFG_NTR_PERIPH_PULSE_BYPASS, reg_3, misc_3->cfg_ntr_periph_pulse_bypass);
    reg_3 = RU_FIELD_SET(0, MISC, 3, CFG_NTR_GPIO_PULSE_BYPASS, reg_3, misc_3->cfg_ntr_gpio_pulse_bypass);
    reg_3 = RU_FIELD_SET(0, MISC, 3, CFG_NTR_SRC, reg_3, misc_3->cfg_ntr_src);
    reg_3 = RU_FIELD_SET(0, MISC, 3, CR_XGWAN_TOP_WAN_MISC_WAN_CFG_LASER_OE, reg_3, misc_3->cr_xgwan_top_wan_misc_wan_cfg_laser_oe);
    reg_3 = RU_FIELD_SET(0, MISC, 3, CR_XGWAN_TOP_WAN_MISC_WAN_CFG_WAN_INTERFACE_SELECT, reg_3, misc_3->cr_xgwan_top_wan_misc_wan_cfg_wan_interface_select);
    reg_3 = RU_FIELD_SET(0, MISC, 3, CR_XGWAN_TOP_WAN_MISC_WAN_CFG_LASER_MODE, reg_3, misc_3->cr_xgwan_top_wan_misc_wan_cfg_laser_mode);
    reg_3 = RU_FIELD_SET(0, MISC, 3, CR_XGWAN_TOP_WAN_MISC_WAN_CFG_LASER_INVERT, reg_3, misc_3->cr_xgwan_top_wan_misc_wan_cfg_laser_invert);
    reg_3 = RU_FIELD_SET(0, MISC, 3, CR_XGWAN_TOP_WAN_MISC_WAN_CFG_MEM_REB, reg_3, misc_3->cr_xgwan_top_wan_misc_wan_cfg_mem_reb);

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

    misc_3->cr_xgwan_top_wan_misc_wan_cfg_wan_debug_sel = RU_FIELD_GET(0, MISC, 3, CR_XGWAN_TOP_WAN_MISC_WAN_CFG_WAN_DEBUG_SEL, reg_3);
    misc_3->cfg_ntr_periph_pulse_bypass = RU_FIELD_GET(0, MISC, 3, CFG_NTR_PERIPH_PULSE_BYPASS, reg_3);
    misc_3->cfg_ntr_gpio_pulse_bypass = RU_FIELD_GET(0, MISC, 3, CFG_NTR_GPIO_PULSE_BYPASS, reg_3);
    misc_3->cfg_ntr_src = RU_FIELD_GET(0, MISC, 3, CFG_NTR_SRC, reg_3);
    misc_3->cr_xgwan_top_wan_misc_wan_cfg_laser_oe = RU_FIELD_GET(0, MISC, 3, CR_XGWAN_TOP_WAN_MISC_WAN_CFG_LASER_OE, reg_3);
    misc_3->cr_xgwan_top_wan_misc_wan_cfg_wan_interface_select = RU_FIELD_GET(0, MISC, 3, CR_XGWAN_TOP_WAN_MISC_WAN_CFG_WAN_INTERFACE_SELECT, reg_3);
    misc_3->cr_xgwan_top_wan_misc_wan_cfg_laser_mode = RU_FIELD_GET(0, MISC, 3, CR_XGWAN_TOP_WAN_MISC_WAN_CFG_LASER_MODE, reg_3);
    misc_3->cr_xgwan_top_wan_misc_wan_cfg_laser_invert = RU_FIELD_GET(0, MISC, 3, CR_XGWAN_TOP_WAN_MISC_WAN_CFG_LASER_INVERT, reg_3);
    misc_3->cr_xgwan_top_wan_misc_wan_cfg_mem_reb = RU_FIELD_GET(0, MISC, 3, CR_XGWAN_TOP_WAN_MISC_WAN_CFG_MEM_REB, reg_3);

    return 0;
}

int ag_drv_misc_misc_4_set(const misc_misc_4 *misc_4)
{
    uint32_t reg_4=0;

    reg_4 = RU_FIELD_SET(0, MISC, 4, CFG_NTR_PULSE_WIDTH, reg_4, misc_4->cfg_ntr_pulse_width);
    RU_REG_WRITE(0, MISC, 4, reg_4);

    return 0;
}

int ag_drv_misc_misc_4_get(misc_misc_4 *misc_4)
{
    uint32_t reg_4=0;

    RU_REG_READ(0, MISC, 4, reg_4);
    misc_4->cfg_ntr_pulse_width = RU_FIELD_GET(0, MISC, 4, CFG_NTR_PULSE_WIDTH, reg_4);

    return 0;
}
