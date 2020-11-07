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
bdmf_error_t ag_drv_misc_misc_0_set(const misc_misc_0 *misc_0)
{
    uint32_t reg_0=0;

#ifdef VALIDATE_PARMS
    if(!misc_0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((misc_0->cr_xgwan_top_wan_misc_epon_tx_fifo_off >= _3BITS_MAX_VAL_) ||
       (misc_0->cr_xgwan_top_wan_misc_onu2g_phya >= _5BITS_MAX_VAL_) ||
       (misc_0->cr_xgwan_top_wan_misc_epon_tx_fifo_off_ld >= _1BITS_MAX_VAL_) ||
       (misc_0->cr_xgwan_top_wan_misc_mdio_fast_mode >= _1BITS_MAX_VAL_) ||
       (misc_0->cr_xgwan_top_wan_misc_mdio_mode >= _1BITS_MAX_VAL_) ||
       (misc_0->cr_xgwan_top_wan_misc_refout_en >= _1BITS_MAX_VAL_) ||
       (misc_0->cr_xgwan_top_wan_misc_refin_en >= _1BITS_MAX_VAL_) ||
       (misc_0->cr_xgwan_top_wan_misc_onu2g_pmd_status_sel >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_0 = RU_FIELD_SET(0, MISC, 0, CR_XGWAN_TOP_WAN_MISC_PMD_LANE_MODE, reg_0, misc_0->cr_xgwan_top_wan_misc_pmd_lane_mode);
    reg_0 = RU_FIELD_SET(0, MISC, 0, CR_XGWAN_TOP_WAN_MISC_EPON_TX_FIFO_OFF, reg_0, misc_0->cr_xgwan_top_wan_misc_epon_tx_fifo_off);
    reg_0 = RU_FIELD_SET(0, MISC, 0, CR_XGWAN_TOP_WAN_MISC_ONU2G_PHYA, reg_0, misc_0->cr_xgwan_top_wan_misc_onu2g_phya);
    reg_0 = RU_FIELD_SET(0, MISC, 0, CR_XGWAN_TOP_WAN_MISC_EPON_TX_FIFO_OFF_LD, reg_0, misc_0->cr_xgwan_top_wan_misc_epon_tx_fifo_off_ld);
    reg_0 = RU_FIELD_SET(0, MISC, 0, CR_XGWAN_TOP_WAN_MISC_MDIO_FAST_MODE, reg_0, misc_0->cr_xgwan_top_wan_misc_mdio_fast_mode);
    reg_0 = RU_FIELD_SET(0, MISC, 0, CR_XGWAN_TOP_WAN_MISC_MDIO_MODE, reg_0, misc_0->cr_xgwan_top_wan_misc_mdio_mode);
    reg_0 = RU_FIELD_SET(0, MISC, 0, CR_XGWAN_TOP_WAN_MISC_REFOUT_EN, reg_0, misc_0->cr_xgwan_top_wan_misc_refout_en);
    reg_0 = RU_FIELD_SET(0, MISC, 0, CR_XGWAN_TOP_WAN_MISC_REFIN_EN, reg_0, misc_0->cr_xgwan_top_wan_misc_refin_en);
    reg_0 = RU_FIELD_SET(0, MISC, 0, CR_XGWAN_TOP_WAN_MISC_ONU2G_PMD_STATUS_SEL, reg_0, misc_0->cr_xgwan_top_wan_misc_onu2g_pmd_status_sel);

    RU_REG_WRITE(0, MISC, 0, reg_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_misc_misc_0_get(misc_misc_0 *misc_0)
{
    uint32_t reg_0=0;

#ifdef VALIDATE_PARMS
    if(!misc_0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, MISC, 0, reg_0);

    misc_0->cr_xgwan_top_wan_misc_pmd_lane_mode = RU_FIELD_GET(0, MISC, 0, CR_XGWAN_TOP_WAN_MISC_PMD_LANE_MODE, reg_0);
    misc_0->cr_xgwan_top_wan_misc_epon_tx_fifo_off = RU_FIELD_GET(0, MISC, 0, CR_XGWAN_TOP_WAN_MISC_EPON_TX_FIFO_OFF, reg_0);
    misc_0->cr_xgwan_top_wan_misc_onu2g_phya = RU_FIELD_GET(0, MISC, 0, CR_XGWAN_TOP_WAN_MISC_ONU2G_PHYA, reg_0);
    misc_0->cr_xgwan_top_wan_misc_epon_tx_fifo_off_ld = RU_FIELD_GET(0, MISC, 0, CR_XGWAN_TOP_WAN_MISC_EPON_TX_FIFO_OFF_LD, reg_0);
    misc_0->cr_xgwan_top_wan_misc_mdio_fast_mode = RU_FIELD_GET(0, MISC, 0, CR_XGWAN_TOP_WAN_MISC_MDIO_FAST_MODE, reg_0);
    misc_0->cr_xgwan_top_wan_misc_mdio_mode = RU_FIELD_GET(0, MISC, 0, CR_XGWAN_TOP_WAN_MISC_MDIO_MODE, reg_0);
    misc_0->cr_xgwan_top_wan_misc_refout_en = RU_FIELD_GET(0, MISC, 0, CR_XGWAN_TOP_WAN_MISC_REFOUT_EN, reg_0);
    misc_0->cr_xgwan_top_wan_misc_refin_en = RU_FIELD_GET(0, MISC, 0, CR_XGWAN_TOP_WAN_MISC_REFIN_EN, reg_0);
    misc_0->cr_xgwan_top_wan_misc_onu2g_pmd_status_sel = RU_FIELD_GET(0, MISC, 0, CR_XGWAN_TOP_WAN_MISC_ONU2G_PMD_STATUS_SEL, reg_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_misc_misc_1_set(uint16_t cr_xgwan_top_wan_misc_pmd_core_1_mode, uint16_t cr_xgwan_top_wan_misc_pmd_core_0_mode)
{
    uint32_t reg_1=0;

#ifdef VALIDATE_PARMS
#endif

    reg_1 = RU_FIELD_SET(0, MISC, 1, CR_XGWAN_TOP_WAN_MISC_PMD_CORE_1_MODE, reg_1, cr_xgwan_top_wan_misc_pmd_core_1_mode);
    reg_1 = RU_FIELD_SET(0, MISC, 1, CR_XGWAN_TOP_WAN_MISC_PMD_CORE_0_MODE, reg_1, cr_xgwan_top_wan_misc_pmd_core_0_mode);

    RU_REG_WRITE(0, MISC, 1, reg_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_misc_misc_1_get(uint16_t *cr_xgwan_top_wan_misc_pmd_core_1_mode, uint16_t *cr_xgwan_top_wan_misc_pmd_core_0_mode)
{
    uint32_t reg_1=0;

#ifdef VALIDATE_PARMS
    if(!cr_xgwan_top_wan_misc_pmd_core_1_mode || !cr_xgwan_top_wan_misc_pmd_core_0_mode)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, MISC, 1, reg_1);

    *cr_xgwan_top_wan_misc_pmd_core_1_mode = RU_FIELD_GET(0, MISC, 1, CR_XGWAN_TOP_WAN_MISC_PMD_CORE_1_MODE, reg_1);
    *cr_xgwan_top_wan_misc_pmd_core_0_mode = RU_FIELD_GET(0, MISC, 1, CR_XGWAN_TOP_WAN_MISC_PMD_CORE_0_MODE, reg_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_misc_misc_2_set(const misc_misc_2 *misc_2)
{
    uint32_t reg_2=0;

#ifdef VALIDATE_PARMS
    if(!misc_2)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((misc_2->cfg_apm_mux_sel_1 >= _1BITS_MAX_VAL_) ||
       (misc_2->cfg_apm_mux_sel_0 >= _1BITS_MAX_VAL_) ||
       (misc_2->cfgngponrxclk >= _2BITS_MAX_VAL_) ||
       (misc_2->cfgngpontxclk >= _2BITS_MAX_VAL_) ||
       (misc_2->cfgactiveethernet2p5 >= _1BITS_MAX_VAL_) ||
       (misc_2->cr_xgwan_top_wan_misc_pmd_rx_osr_mode >= _4BITS_MAX_VAL_) ||
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
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_2 = RU_FIELD_SET(0, MISC, 2, CFG_APM_MUX_SEL_1, reg_2, misc_2->cfg_apm_mux_sel_1);
    reg_2 = RU_FIELD_SET(0, MISC, 2, CFG_APM_MUX_SEL_0, reg_2, misc_2->cfg_apm_mux_sel_0);
    reg_2 = RU_FIELD_SET(0, MISC, 2, CFGNGPONRXCLK, reg_2, misc_2->cfgngponrxclk);
    reg_2 = RU_FIELD_SET(0, MISC, 2, CFGNGPONTXCLK, reg_2, misc_2->cfgngpontxclk);
    reg_2 = RU_FIELD_SET(0, MISC, 2, CFGACTIVEETHERNET2P5, reg_2, misc_2->cfgactiveethernet2p5);
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

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_misc_misc_2_get(misc_misc_2 *misc_2)
{
    uint32_t reg_2=0;

#ifdef VALIDATE_PARMS
    if(!misc_2)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, MISC, 2, reg_2);

    misc_2->cfg_apm_mux_sel_1 = RU_FIELD_GET(0, MISC, 2, CFG_APM_MUX_SEL_1, reg_2);
    misc_2->cfg_apm_mux_sel_0 = RU_FIELD_GET(0, MISC, 2, CFG_APM_MUX_SEL_0, reg_2);
    misc_2->cfgngponrxclk = RU_FIELD_GET(0, MISC, 2, CFGNGPONRXCLK, reg_2);
    misc_2->cfgngpontxclk = RU_FIELD_GET(0, MISC, 2, CFGNGPONTXCLK, reg_2);
    misc_2->cfgactiveethernet2p5 = RU_FIELD_GET(0, MISC, 2, CFGACTIVEETHERNET2P5, reg_2);
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

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_misc_misc_3_set(const misc_misc_3 *misc_3)
{
    uint32_t reg_3=0;

#ifdef VALIDATE_PARMS
    if(!misc_3)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((misc_3->cr_xgwan_top_wan_misc_wan_cfg_wan_debug_sel >= _4BITS_MAX_VAL_) ||
       (misc_3->cr_xgwan_top_wan_misc_wan_cfg_ntr_sync_period_sel >= _4BITS_MAX_VAL_) ||
       (misc_3->cr_xgwan_top_wan_misc_wan_cfg_laser_oe >= _1BITS_MAX_VAL_) ||
       (misc_3->cr_xgwan_top_wan_misc_wan_cfg_wan_interface_select >= _2BITS_MAX_VAL_) ||
       (misc_3->cr_xgwan_top_wan_misc_wan_cfg_laser_mode >= _3BITS_MAX_VAL_) ||
       (misc_3->cr_xgwan_top_wan_misc_wan_cfg_laser_invert >= _1BITS_MAX_VAL_) ||
       (misc_3->cr_xgwan_top_wan_misc_wan_cfg_mem_reb >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_3 = RU_FIELD_SET(0, MISC, 3, CR_XGWAN_TOP_WAN_MISC_WAN_CFG_EPON_DEBUG_SEL, reg_3, misc_3->cr_xgwan_top_wan_misc_wan_cfg_epon_debug_sel);
    reg_3 = RU_FIELD_SET(0, MISC, 3, CR_XGWAN_TOP_WAN_MISC_WAN_CFG_WAN_DEBUG_SEL, reg_3, misc_3->cr_xgwan_top_wan_misc_wan_cfg_wan_debug_sel);
    reg_3 = RU_FIELD_SET(0, MISC, 3, CR_XGWAN_TOP_WAN_MISC_WAN_CFG_NTR_SYNC_PERIOD_SEL, reg_3, misc_3->cr_xgwan_top_wan_misc_wan_cfg_ntr_sync_period_sel);
    reg_3 = RU_FIELD_SET(0, MISC, 3, CR_XGWAN_TOP_WAN_MISC_WAN_CFG_LASER_OE, reg_3, misc_3->cr_xgwan_top_wan_misc_wan_cfg_laser_oe);
    reg_3 = RU_FIELD_SET(0, MISC, 3, CR_XGWAN_TOP_WAN_MISC_WAN_CFG_WAN_INTERFACE_SELECT, reg_3, misc_3->cr_xgwan_top_wan_misc_wan_cfg_wan_interface_select);
    reg_3 = RU_FIELD_SET(0, MISC, 3, CR_XGWAN_TOP_WAN_MISC_WAN_CFG_LASER_MODE, reg_3, misc_3->cr_xgwan_top_wan_misc_wan_cfg_laser_mode);
    reg_3 = RU_FIELD_SET(0, MISC, 3, CR_XGWAN_TOP_WAN_MISC_WAN_CFG_LASER_INVERT, reg_3, misc_3->cr_xgwan_top_wan_misc_wan_cfg_laser_invert);
    reg_3 = RU_FIELD_SET(0, MISC, 3, CR_XGWAN_TOP_WAN_MISC_WAN_CFG_MEM_REB, reg_3, misc_3->cr_xgwan_top_wan_misc_wan_cfg_mem_reb);

    RU_REG_WRITE(0, MISC, 3, reg_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_misc_misc_3_get(misc_misc_3 *misc_3)
{
    uint32_t reg_3=0;

#ifdef VALIDATE_PARMS
    if(!misc_3)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, MISC, 3, reg_3);

    misc_3->cr_xgwan_top_wan_misc_wan_cfg_epon_debug_sel = RU_FIELD_GET(0, MISC, 3, CR_XGWAN_TOP_WAN_MISC_WAN_CFG_EPON_DEBUG_SEL, reg_3);
    misc_3->cr_xgwan_top_wan_misc_wan_cfg_wan_debug_sel = RU_FIELD_GET(0, MISC, 3, CR_XGWAN_TOP_WAN_MISC_WAN_CFG_WAN_DEBUG_SEL, reg_3);
    misc_3->cr_xgwan_top_wan_misc_wan_cfg_ntr_sync_period_sel = RU_FIELD_GET(0, MISC, 3, CR_XGWAN_TOP_WAN_MISC_WAN_CFG_NTR_SYNC_PERIOD_SEL, reg_3);
    misc_3->cr_xgwan_top_wan_misc_wan_cfg_laser_oe = RU_FIELD_GET(0, MISC, 3, CR_XGWAN_TOP_WAN_MISC_WAN_CFG_LASER_OE, reg_3);
    misc_3->cr_xgwan_top_wan_misc_wan_cfg_wan_interface_select = RU_FIELD_GET(0, MISC, 3, CR_XGWAN_TOP_WAN_MISC_WAN_CFG_WAN_INTERFACE_SELECT, reg_3);
    misc_3->cr_xgwan_top_wan_misc_wan_cfg_laser_mode = RU_FIELD_GET(0, MISC, 3, CR_XGWAN_TOP_WAN_MISC_WAN_CFG_LASER_MODE, reg_3);
    misc_3->cr_xgwan_top_wan_misc_wan_cfg_laser_invert = RU_FIELD_GET(0, MISC, 3, CR_XGWAN_TOP_WAN_MISC_WAN_CFG_LASER_INVERT, reg_3);
    misc_3->cr_xgwan_top_wan_misc_wan_cfg_mem_reb = RU_FIELD_GET(0, MISC, 3, CR_XGWAN_TOP_WAN_MISC_WAN_CFG_MEM_REB, reg_3);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL
enum
{
    BDMF_misc_0,
    BDMF_misc_1,
    BDMF_misc_2,
    BDMF_misc_3,
};

typedef enum
{
    bdmf_address_0,
    bdmf_address_1,
    bdmf_address_2,
    bdmf_address_3,
}
bdmf_address;

static int bcm_misc_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err;

    switch(parm[0].value.unumber)
    {
    case BDMF_misc_0:
    {
        misc_misc_0 misc_0 = { .cr_xgwan_top_wan_misc_pmd_lane_mode=parm[1].value.unumber, .cr_xgwan_top_wan_misc_epon_tx_fifo_off=parm[2].value.unumber, .cr_xgwan_top_wan_misc_onu2g_phya=parm[3].value.unumber, .cr_xgwan_top_wan_misc_epon_tx_fifo_off_ld=parm[4].value.unumber, .cr_xgwan_top_wan_misc_mdio_fast_mode=parm[5].value.unumber, .cr_xgwan_top_wan_misc_mdio_mode=parm[6].value.unumber, .cr_xgwan_top_wan_misc_refout_en=parm[7].value.unumber, .cr_xgwan_top_wan_misc_refin_en=parm[8].value.unumber, .cr_xgwan_top_wan_misc_onu2g_pmd_status_sel=parm[9].value.unumber};
        err = ag_drv_misc_misc_0_set(&misc_0);
        break;
    }
    case BDMF_misc_1:
        err = ag_drv_misc_misc_1_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case BDMF_misc_2:
    {
        misc_misc_2 misc_2 = { .cfg_apm_mux_sel_1=parm[1].value.unumber, .cfg_apm_mux_sel_0=parm[2].value.unumber, .cfgngponrxclk=parm[3].value.unumber, .cfgngpontxclk=parm[4].value.unumber, .cfgactiveethernet2p5=parm[5].value.unumber, .cr_xgwan_top_wan_misc_pmd_rx_osr_mode=parm[6].value.unumber, .cr_xgwan_top_wan_misc_pmd_tx_mode=parm[7].value.unumber, .cr_xgwan_top_wan_misc_pmd_tx_osr_mode=parm[8].value.unumber, .cr_xgwan_top_wan_misc_pmd_tx_disable=parm[9].value.unumber, .cr_xgwan_top_wan_misc_pmd_ln_rx_h_pwrdn=parm[10].value.unumber, .cr_xgwan_top_wan_misc_pmd_ln_tx_h_pwrdn=parm[11].value.unumber, .cr_xgwan_top_wan_misc_pmd_ext_los=parm[12].value.unumber, .cr_xgwan_top_wan_misc_pmd_por_h_rstb=parm[13].value.unumber, .cr_xgwan_top_wan_misc_pmd_core_1_dp_h_rstb=parm[14].value.unumber, .cr_xgwan_top_wan_misc_pmd_core_0_dp_h_rstb=parm[15].value.unumber, .cr_xgwan_top_wan_misc_pmd_ln_h_rstb=parm[16].value.unumber, .cr_xgwan_top_wan_misc_pmd_ln_dp_h_rstb=parm[17].value.unumber, .cr_xgwan_top_wan_misc_pmd_rx_mode=parm[18].value.unumber};
        err = ag_drv_misc_misc_2_set(&misc_2);
        break;
    }
    case BDMF_misc_3:
    {
        misc_misc_3 misc_3 = { .cr_xgwan_top_wan_misc_wan_cfg_epon_debug_sel=parm[1].value.unumber, .cr_xgwan_top_wan_misc_wan_cfg_wan_debug_sel=parm[2].value.unumber, .cr_xgwan_top_wan_misc_wan_cfg_ntr_sync_period_sel=parm[3].value.unumber, .cr_xgwan_top_wan_misc_wan_cfg_laser_oe=parm[4].value.unumber, .cr_xgwan_top_wan_misc_wan_cfg_wan_interface_select=parm[5].value.unumber, .cr_xgwan_top_wan_misc_wan_cfg_laser_mode=parm[6].value.unumber, .cr_xgwan_top_wan_misc_wan_cfg_laser_invert=parm[7].value.unumber, .cr_xgwan_top_wan_misc_wan_cfg_mem_reb=parm[8].value.unumber};
        err = ag_drv_misc_misc_3_set(&misc_3);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_misc_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err;

    switch(parm[0].value.unumber)
    {
    case BDMF_misc_0:
    {
        misc_misc_0 misc_0;
        err = ag_drv_misc_misc_0_get(&misc_0);
        bdmf_session_print(session, "cr_xgwan_top_wan_misc_pmd_lane_mode = %u = 0x%x\n", misc_0.cr_xgwan_top_wan_misc_pmd_lane_mode, misc_0.cr_xgwan_top_wan_misc_pmd_lane_mode);
        bdmf_session_print(session, "cr_xgwan_top_wan_misc_epon_tx_fifo_off = %u = 0x%x\n", misc_0.cr_xgwan_top_wan_misc_epon_tx_fifo_off, misc_0.cr_xgwan_top_wan_misc_epon_tx_fifo_off);
        bdmf_session_print(session, "cr_xgwan_top_wan_misc_onu2g_phya = %u = 0x%x\n", misc_0.cr_xgwan_top_wan_misc_onu2g_phya, misc_0.cr_xgwan_top_wan_misc_onu2g_phya);
        bdmf_session_print(session, "cr_xgwan_top_wan_misc_epon_tx_fifo_off_ld = %u = 0x%x\n", misc_0.cr_xgwan_top_wan_misc_epon_tx_fifo_off_ld, misc_0.cr_xgwan_top_wan_misc_epon_tx_fifo_off_ld);
        bdmf_session_print(session, "cr_xgwan_top_wan_misc_mdio_fast_mode = %u = 0x%x\n", misc_0.cr_xgwan_top_wan_misc_mdio_fast_mode, misc_0.cr_xgwan_top_wan_misc_mdio_fast_mode);
        bdmf_session_print(session, "cr_xgwan_top_wan_misc_mdio_mode = %u = 0x%x\n", misc_0.cr_xgwan_top_wan_misc_mdio_mode, misc_0.cr_xgwan_top_wan_misc_mdio_mode);
        bdmf_session_print(session, "cr_xgwan_top_wan_misc_refout_en = %u = 0x%x\n", misc_0.cr_xgwan_top_wan_misc_refout_en, misc_0.cr_xgwan_top_wan_misc_refout_en);
        bdmf_session_print(session, "cr_xgwan_top_wan_misc_refin_en = %u = 0x%x\n", misc_0.cr_xgwan_top_wan_misc_refin_en, misc_0.cr_xgwan_top_wan_misc_refin_en);
        bdmf_session_print(session, "cr_xgwan_top_wan_misc_onu2g_pmd_status_sel = %u = 0x%x\n", misc_0.cr_xgwan_top_wan_misc_onu2g_pmd_status_sel, misc_0.cr_xgwan_top_wan_misc_onu2g_pmd_status_sel);
        break;
    }
    case BDMF_misc_1:
    {
        uint16_t cr_xgwan_top_wan_misc_pmd_core_1_mode;
        uint16_t cr_xgwan_top_wan_misc_pmd_core_0_mode;
        err = ag_drv_misc_misc_1_get(&cr_xgwan_top_wan_misc_pmd_core_1_mode, &cr_xgwan_top_wan_misc_pmd_core_0_mode);
        bdmf_session_print(session, "cr_xgwan_top_wan_misc_pmd_core_1_mode = %u = 0x%x\n", cr_xgwan_top_wan_misc_pmd_core_1_mode, cr_xgwan_top_wan_misc_pmd_core_1_mode);
        bdmf_session_print(session, "cr_xgwan_top_wan_misc_pmd_core_0_mode = %u = 0x%x\n", cr_xgwan_top_wan_misc_pmd_core_0_mode, cr_xgwan_top_wan_misc_pmd_core_0_mode);
        break;
    }
    case BDMF_misc_2:
    {
        misc_misc_2 misc_2;
        err = ag_drv_misc_misc_2_get(&misc_2);
        bdmf_session_print(session, "cfg_apm_mux_sel_1 = %u = 0x%x\n", misc_2.cfg_apm_mux_sel_1, misc_2.cfg_apm_mux_sel_1);
        bdmf_session_print(session, "cfg_apm_mux_sel_0 = %u = 0x%x\n", misc_2.cfg_apm_mux_sel_0, misc_2.cfg_apm_mux_sel_0);
        bdmf_session_print(session, "cfgngponrxclk = %u = 0x%x\n", misc_2.cfgngponrxclk, misc_2.cfgngponrxclk);
        bdmf_session_print(session, "cfgngpontxclk = %u = 0x%x\n", misc_2.cfgngpontxclk, misc_2.cfgngpontxclk);
        bdmf_session_print(session, "cfgactiveethernet2p5 = %u = 0x%x\n", misc_2.cfgactiveethernet2p5, misc_2.cfgactiveethernet2p5);
        bdmf_session_print(session, "cr_xgwan_top_wan_misc_pmd_rx_osr_mode = %u = 0x%x\n", misc_2.cr_xgwan_top_wan_misc_pmd_rx_osr_mode, misc_2.cr_xgwan_top_wan_misc_pmd_rx_osr_mode);
        bdmf_session_print(session, "cr_xgwan_top_wan_misc_pmd_tx_mode = %u = 0x%x\n", misc_2.cr_xgwan_top_wan_misc_pmd_tx_mode, misc_2.cr_xgwan_top_wan_misc_pmd_tx_mode);
        bdmf_session_print(session, "cr_xgwan_top_wan_misc_pmd_tx_osr_mode = %u = 0x%x\n", misc_2.cr_xgwan_top_wan_misc_pmd_tx_osr_mode, misc_2.cr_xgwan_top_wan_misc_pmd_tx_osr_mode);
        bdmf_session_print(session, "cr_xgwan_top_wan_misc_pmd_tx_disable = %u = 0x%x\n", misc_2.cr_xgwan_top_wan_misc_pmd_tx_disable, misc_2.cr_xgwan_top_wan_misc_pmd_tx_disable);
        bdmf_session_print(session, "cr_xgwan_top_wan_misc_pmd_ln_rx_h_pwrdn = %u = 0x%x\n", misc_2.cr_xgwan_top_wan_misc_pmd_ln_rx_h_pwrdn, misc_2.cr_xgwan_top_wan_misc_pmd_ln_rx_h_pwrdn);
        bdmf_session_print(session, "cr_xgwan_top_wan_misc_pmd_ln_tx_h_pwrdn = %u = 0x%x\n", misc_2.cr_xgwan_top_wan_misc_pmd_ln_tx_h_pwrdn, misc_2.cr_xgwan_top_wan_misc_pmd_ln_tx_h_pwrdn);
        bdmf_session_print(session, "cr_xgwan_top_wan_misc_pmd_ext_los = %u = 0x%x\n", misc_2.cr_xgwan_top_wan_misc_pmd_ext_los, misc_2.cr_xgwan_top_wan_misc_pmd_ext_los);
        bdmf_session_print(session, "cr_xgwan_top_wan_misc_pmd_por_h_rstb = %u = 0x%x\n", misc_2.cr_xgwan_top_wan_misc_pmd_por_h_rstb, misc_2.cr_xgwan_top_wan_misc_pmd_por_h_rstb);
        bdmf_session_print(session, "cr_xgwan_top_wan_misc_pmd_core_1_dp_h_rstb = %u = 0x%x\n", misc_2.cr_xgwan_top_wan_misc_pmd_core_1_dp_h_rstb, misc_2.cr_xgwan_top_wan_misc_pmd_core_1_dp_h_rstb);
        bdmf_session_print(session, "cr_xgwan_top_wan_misc_pmd_core_0_dp_h_rstb = %u = 0x%x\n", misc_2.cr_xgwan_top_wan_misc_pmd_core_0_dp_h_rstb, misc_2.cr_xgwan_top_wan_misc_pmd_core_0_dp_h_rstb);
        bdmf_session_print(session, "cr_xgwan_top_wan_misc_pmd_ln_h_rstb = %u = 0x%x\n", misc_2.cr_xgwan_top_wan_misc_pmd_ln_h_rstb, misc_2.cr_xgwan_top_wan_misc_pmd_ln_h_rstb);
        bdmf_session_print(session, "cr_xgwan_top_wan_misc_pmd_ln_dp_h_rstb = %u = 0x%x\n", misc_2.cr_xgwan_top_wan_misc_pmd_ln_dp_h_rstb, misc_2.cr_xgwan_top_wan_misc_pmd_ln_dp_h_rstb);
        bdmf_session_print(session, "cr_xgwan_top_wan_misc_pmd_rx_mode = %u = 0x%x\n", misc_2.cr_xgwan_top_wan_misc_pmd_rx_mode, misc_2.cr_xgwan_top_wan_misc_pmd_rx_mode);
        break;
    }
    case BDMF_misc_3:
    {
        misc_misc_3 misc_3;
        err = ag_drv_misc_misc_3_get(&misc_3);
        bdmf_session_print(session, "cr_xgwan_top_wan_misc_wan_cfg_epon_debug_sel = %u = 0x%x\n", misc_3.cr_xgwan_top_wan_misc_wan_cfg_epon_debug_sel, misc_3.cr_xgwan_top_wan_misc_wan_cfg_epon_debug_sel);
        bdmf_session_print(session, "cr_xgwan_top_wan_misc_wan_cfg_wan_debug_sel = %u = 0x%x\n", misc_3.cr_xgwan_top_wan_misc_wan_cfg_wan_debug_sel, misc_3.cr_xgwan_top_wan_misc_wan_cfg_wan_debug_sel);
        bdmf_session_print(session, "cr_xgwan_top_wan_misc_wan_cfg_ntr_sync_period_sel = %u = 0x%x\n", misc_3.cr_xgwan_top_wan_misc_wan_cfg_ntr_sync_period_sel, misc_3.cr_xgwan_top_wan_misc_wan_cfg_ntr_sync_period_sel);
        bdmf_session_print(session, "cr_xgwan_top_wan_misc_wan_cfg_laser_oe = %u = 0x%x\n", misc_3.cr_xgwan_top_wan_misc_wan_cfg_laser_oe, misc_3.cr_xgwan_top_wan_misc_wan_cfg_laser_oe);
        bdmf_session_print(session, "cr_xgwan_top_wan_misc_wan_cfg_wan_interface_select = %u = 0x%x\n", misc_3.cr_xgwan_top_wan_misc_wan_cfg_wan_interface_select, misc_3.cr_xgwan_top_wan_misc_wan_cfg_wan_interface_select);
        bdmf_session_print(session, "cr_xgwan_top_wan_misc_wan_cfg_laser_mode = %u = 0x%x\n", misc_3.cr_xgwan_top_wan_misc_wan_cfg_laser_mode, misc_3.cr_xgwan_top_wan_misc_wan_cfg_laser_mode);
        bdmf_session_print(session, "cr_xgwan_top_wan_misc_wan_cfg_laser_invert = %u = 0x%x\n", misc_3.cr_xgwan_top_wan_misc_wan_cfg_laser_invert, misc_3.cr_xgwan_top_wan_misc_wan_cfg_laser_invert);
        bdmf_session_print(session, "cr_xgwan_top_wan_misc_wan_cfg_mem_reb = %u = 0x%x\n", misc_3.cr_xgwan_top_wan_misc_wan_cfg_mem_reb, misc_3.cr_xgwan_top_wan_misc_wan_cfg_mem_reb);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_misc_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    bdmf_error_t err = BDMF_ERR_OK;

    {
        misc_misc_0 misc_0 = {.cr_xgwan_top_wan_misc_pmd_lane_mode=gtmv(m, 16), .cr_xgwan_top_wan_misc_epon_tx_fifo_off=gtmv(m, 3), .cr_xgwan_top_wan_misc_onu2g_phya=gtmv(m, 5), .cr_xgwan_top_wan_misc_epon_tx_fifo_off_ld=gtmv(m, 1), .cr_xgwan_top_wan_misc_mdio_fast_mode=gtmv(m, 1), .cr_xgwan_top_wan_misc_mdio_mode=gtmv(m, 1), .cr_xgwan_top_wan_misc_refout_en=gtmv(m, 1), .cr_xgwan_top_wan_misc_refin_en=gtmv(m, 1), .cr_xgwan_top_wan_misc_onu2g_pmd_status_sel=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_misc_misc_0_set( %u %u %u %u %u %u %u %u %u)\n", misc_0.cr_xgwan_top_wan_misc_pmd_lane_mode, misc_0.cr_xgwan_top_wan_misc_epon_tx_fifo_off, misc_0.cr_xgwan_top_wan_misc_onu2g_phya, misc_0.cr_xgwan_top_wan_misc_epon_tx_fifo_off_ld, misc_0.cr_xgwan_top_wan_misc_mdio_fast_mode, misc_0.cr_xgwan_top_wan_misc_mdio_mode, misc_0.cr_xgwan_top_wan_misc_refout_en, misc_0.cr_xgwan_top_wan_misc_refin_en, misc_0.cr_xgwan_top_wan_misc_onu2g_pmd_status_sel);
        if(!err) ag_drv_misc_misc_0_set(&misc_0);
        if(!err) ag_drv_misc_misc_0_get( &misc_0);
        if(!err) bdmf_session_print(session, "ag_drv_misc_misc_0_get( %u %u %u %u %u %u %u %u %u)\n", misc_0.cr_xgwan_top_wan_misc_pmd_lane_mode, misc_0.cr_xgwan_top_wan_misc_epon_tx_fifo_off, misc_0.cr_xgwan_top_wan_misc_onu2g_phya, misc_0.cr_xgwan_top_wan_misc_epon_tx_fifo_off_ld, misc_0.cr_xgwan_top_wan_misc_mdio_fast_mode, misc_0.cr_xgwan_top_wan_misc_mdio_mode, misc_0.cr_xgwan_top_wan_misc_refout_en, misc_0.cr_xgwan_top_wan_misc_refin_en, misc_0.cr_xgwan_top_wan_misc_onu2g_pmd_status_sel);
        if(err || misc_0.cr_xgwan_top_wan_misc_pmd_lane_mode!=gtmv(m, 16) || misc_0.cr_xgwan_top_wan_misc_epon_tx_fifo_off!=gtmv(m, 3) || misc_0.cr_xgwan_top_wan_misc_onu2g_phya!=gtmv(m, 5) || misc_0.cr_xgwan_top_wan_misc_epon_tx_fifo_off_ld!=gtmv(m, 1) || misc_0.cr_xgwan_top_wan_misc_mdio_fast_mode!=gtmv(m, 1) || misc_0.cr_xgwan_top_wan_misc_mdio_mode!=gtmv(m, 1) || misc_0.cr_xgwan_top_wan_misc_refout_en!=gtmv(m, 1) || misc_0.cr_xgwan_top_wan_misc_refin_en!=gtmv(m, 1) || misc_0.cr_xgwan_top_wan_misc_onu2g_pmd_status_sel!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t cr_xgwan_top_wan_misc_pmd_core_1_mode=gtmv(m, 16);
        uint16_t cr_xgwan_top_wan_misc_pmd_core_0_mode=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_misc_misc_1_set( %u %u)\n", cr_xgwan_top_wan_misc_pmd_core_1_mode, cr_xgwan_top_wan_misc_pmd_core_0_mode);
        if(!err) ag_drv_misc_misc_1_set(cr_xgwan_top_wan_misc_pmd_core_1_mode, cr_xgwan_top_wan_misc_pmd_core_0_mode);
        if(!err) ag_drv_misc_misc_1_get( &cr_xgwan_top_wan_misc_pmd_core_1_mode, &cr_xgwan_top_wan_misc_pmd_core_0_mode);
        if(!err) bdmf_session_print(session, "ag_drv_misc_misc_1_get( %u %u)\n", cr_xgwan_top_wan_misc_pmd_core_1_mode, cr_xgwan_top_wan_misc_pmd_core_0_mode);
        if(err || cr_xgwan_top_wan_misc_pmd_core_1_mode!=gtmv(m, 16) || cr_xgwan_top_wan_misc_pmd_core_0_mode!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        misc_misc_2 misc_2 = {.cfg_apm_mux_sel_1=gtmv(m, 1), .cfg_apm_mux_sel_0=gtmv(m, 1), .cfgngponrxclk=gtmv(m, 2), .cfgngpontxclk=gtmv(m, 2), .cfgactiveethernet2p5=gtmv(m, 1), .cr_xgwan_top_wan_misc_pmd_rx_osr_mode=gtmv(m, 4), .cr_xgwan_top_wan_misc_pmd_tx_mode=gtmv(m, 2), .cr_xgwan_top_wan_misc_pmd_tx_osr_mode=gtmv(m, 4), .cr_xgwan_top_wan_misc_pmd_tx_disable=gtmv(m, 1), .cr_xgwan_top_wan_misc_pmd_ln_rx_h_pwrdn=gtmv(m, 1), .cr_xgwan_top_wan_misc_pmd_ln_tx_h_pwrdn=gtmv(m, 1), .cr_xgwan_top_wan_misc_pmd_ext_los=gtmv(m, 1), .cr_xgwan_top_wan_misc_pmd_por_h_rstb=gtmv(m, 1), .cr_xgwan_top_wan_misc_pmd_core_1_dp_h_rstb=gtmv(m, 1), .cr_xgwan_top_wan_misc_pmd_core_0_dp_h_rstb=gtmv(m, 1), .cr_xgwan_top_wan_misc_pmd_ln_h_rstb=gtmv(m, 1), .cr_xgwan_top_wan_misc_pmd_ln_dp_h_rstb=gtmv(m, 1), .cr_xgwan_top_wan_misc_pmd_rx_mode=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_misc_misc_2_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", misc_2.cfg_apm_mux_sel_1, misc_2.cfg_apm_mux_sel_0, misc_2.cfgngponrxclk, misc_2.cfgngpontxclk, misc_2.cfgactiveethernet2p5, misc_2.cr_xgwan_top_wan_misc_pmd_rx_osr_mode, misc_2.cr_xgwan_top_wan_misc_pmd_tx_mode, misc_2.cr_xgwan_top_wan_misc_pmd_tx_osr_mode, misc_2.cr_xgwan_top_wan_misc_pmd_tx_disable, misc_2.cr_xgwan_top_wan_misc_pmd_ln_rx_h_pwrdn, misc_2.cr_xgwan_top_wan_misc_pmd_ln_tx_h_pwrdn, misc_2.cr_xgwan_top_wan_misc_pmd_ext_los, misc_2.cr_xgwan_top_wan_misc_pmd_por_h_rstb, misc_2.cr_xgwan_top_wan_misc_pmd_core_1_dp_h_rstb, misc_2.cr_xgwan_top_wan_misc_pmd_core_0_dp_h_rstb, misc_2.cr_xgwan_top_wan_misc_pmd_ln_h_rstb, misc_2.cr_xgwan_top_wan_misc_pmd_ln_dp_h_rstb, misc_2.cr_xgwan_top_wan_misc_pmd_rx_mode);
        if(!err) ag_drv_misc_misc_2_set(&misc_2);
        if(!err) ag_drv_misc_misc_2_get( &misc_2);
        if(!err) bdmf_session_print(session, "ag_drv_misc_misc_2_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", misc_2.cfg_apm_mux_sel_1, misc_2.cfg_apm_mux_sel_0, misc_2.cfgngponrxclk, misc_2.cfgngpontxclk, misc_2.cfgactiveethernet2p5, misc_2.cr_xgwan_top_wan_misc_pmd_rx_osr_mode, misc_2.cr_xgwan_top_wan_misc_pmd_tx_mode, misc_2.cr_xgwan_top_wan_misc_pmd_tx_osr_mode, misc_2.cr_xgwan_top_wan_misc_pmd_tx_disable, misc_2.cr_xgwan_top_wan_misc_pmd_ln_rx_h_pwrdn, misc_2.cr_xgwan_top_wan_misc_pmd_ln_tx_h_pwrdn, misc_2.cr_xgwan_top_wan_misc_pmd_ext_los, misc_2.cr_xgwan_top_wan_misc_pmd_por_h_rstb, misc_2.cr_xgwan_top_wan_misc_pmd_core_1_dp_h_rstb, misc_2.cr_xgwan_top_wan_misc_pmd_core_0_dp_h_rstb, misc_2.cr_xgwan_top_wan_misc_pmd_ln_h_rstb, misc_2.cr_xgwan_top_wan_misc_pmd_ln_dp_h_rstb, misc_2.cr_xgwan_top_wan_misc_pmd_rx_mode);
        if(err || misc_2.cfg_apm_mux_sel_1!=gtmv(m, 1) || misc_2.cfg_apm_mux_sel_0!=gtmv(m, 1) || misc_2.cfgngponrxclk!=gtmv(m, 2) || misc_2.cfgngpontxclk!=gtmv(m, 2) || misc_2.cfgactiveethernet2p5!=gtmv(m, 1) || misc_2.cr_xgwan_top_wan_misc_pmd_rx_osr_mode!=gtmv(m, 4) || misc_2.cr_xgwan_top_wan_misc_pmd_tx_mode!=gtmv(m, 2) || misc_2.cr_xgwan_top_wan_misc_pmd_tx_osr_mode!=gtmv(m, 4) || misc_2.cr_xgwan_top_wan_misc_pmd_tx_disable!=gtmv(m, 1) || misc_2.cr_xgwan_top_wan_misc_pmd_ln_rx_h_pwrdn!=gtmv(m, 1) || misc_2.cr_xgwan_top_wan_misc_pmd_ln_tx_h_pwrdn!=gtmv(m, 1) || misc_2.cr_xgwan_top_wan_misc_pmd_ext_los!=gtmv(m, 1) || misc_2.cr_xgwan_top_wan_misc_pmd_por_h_rstb!=gtmv(m, 1) || misc_2.cr_xgwan_top_wan_misc_pmd_core_1_dp_h_rstb!=gtmv(m, 1) || misc_2.cr_xgwan_top_wan_misc_pmd_core_0_dp_h_rstb!=gtmv(m, 1) || misc_2.cr_xgwan_top_wan_misc_pmd_ln_h_rstb!=gtmv(m, 1) || misc_2.cr_xgwan_top_wan_misc_pmd_ln_dp_h_rstb!=gtmv(m, 1) || misc_2.cr_xgwan_top_wan_misc_pmd_rx_mode!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        misc_misc_3 misc_3 = {.cr_xgwan_top_wan_misc_wan_cfg_epon_debug_sel=gtmv(m, 8), .cr_xgwan_top_wan_misc_wan_cfg_wan_debug_sel=gtmv(m, 4), .cr_xgwan_top_wan_misc_wan_cfg_ntr_sync_period_sel=gtmv(m, 4), .cr_xgwan_top_wan_misc_wan_cfg_laser_oe=gtmv(m, 1), .cr_xgwan_top_wan_misc_wan_cfg_wan_interface_select=gtmv(m, 2), .cr_xgwan_top_wan_misc_wan_cfg_laser_mode=gtmv(m, 3), .cr_xgwan_top_wan_misc_wan_cfg_laser_invert=gtmv(m, 1), .cr_xgwan_top_wan_misc_wan_cfg_mem_reb=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_misc_misc_3_set( %u %u %u %u %u %u %u %u)\n", misc_3.cr_xgwan_top_wan_misc_wan_cfg_epon_debug_sel, misc_3.cr_xgwan_top_wan_misc_wan_cfg_wan_debug_sel, misc_3.cr_xgwan_top_wan_misc_wan_cfg_ntr_sync_period_sel, misc_3.cr_xgwan_top_wan_misc_wan_cfg_laser_oe, misc_3.cr_xgwan_top_wan_misc_wan_cfg_wan_interface_select, misc_3.cr_xgwan_top_wan_misc_wan_cfg_laser_mode, misc_3.cr_xgwan_top_wan_misc_wan_cfg_laser_invert, misc_3.cr_xgwan_top_wan_misc_wan_cfg_mem_reb);
        if(!err) ag_drv_misc_misc_3_set(&misc_3);
        if(!err) ag_drv_misc_misc_3_get( &misc_3);
        if(!err) bdmf_session_print(session, "ag_drv_misc_misc_3_get( %u %u %u %u %u %u %u %u)\n", misc_3.cr_xgwan_top_wan_misc_wan_cfg_epon_debug_sel, misc_3.cr_xgwan_top_wan_misc_wan_cfg_wan_debug_sel, misc_3.cr_xgwan_top_wan_misc_wan_cfg_ntr_sync_period_sel, misc_3.cr_xgwan_top_wan_misc_wan_cfg_laser_oe, misc_3.cr_xgwan_top_wan_misc_wan_cfg_wan_interface_select, misc_3.cr_xgwan_top_wan_misc_wan_cfg_laser_mode, misc_3.cr_xgwan_top_wan_misc_wan_cfg_laser_invert, misc_3.cr_xgwan_top_wan_misc_wan_cfg_mem_reb);
        if(err || misc_3.cr_xgwan_top_wan_misc_wan_cfg_epon_debug_sel!=gtmv(m, 8) || misc_3.cr_xgwan_top_wan_misc_wan_cfg_wan_debug_sel!=gtmv(m, 4) || misc_3.cr_xgwan_top_wan_misc_wan_cfg_ntr_sync_period_sel!=gtmv(m, 4) || misc_3.cr_xgwan_top_wan_misc_wan_cfg_laser_oe!=gtmv(m, 1) || misc_3.cr_xgwan_top_wan_misc_wan_cfg_wan_interface_select!=gtmv(m, 2) || misc_3.cr_xgwan_top_wan_misc_wan_cfg_laser_mode!=gtmv(m, 3) || misc_3.cr_xgwan_top_wan_misc_wan_cfg_laser_invert!=gtmv(m, 1) || misc_3.cr_xgwan_top_wan_misc_wan_cfg_mem_reb!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    return err;
}

static int bcm_misc_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint32_t i;
    uint32_t j;
    uint32_t index1_start=0;
    uint32_t index1_stop;
    uint32_t index2_start=0;
    uint32_t index2_stop;
    bdmfmon_cmd_parm_t * bdmf_parm;
    const ru_reg_rec * reg;
    const ru_block_rec * blk;
    const char * enum_string = bdmfmon_enum_parm_stringval(session, 0, parm[0].value.unumber);

    if(!enum_string)
        return BDMF_ERR_INTERNAL;

    switch (parm[0].value.unumber)
    {
    case bdmf_address_0 : reg = &RU_REG(MISC, 0); blk = &RU_BLK(MISC); break;
    case bdmf_address_1 : reg = &RU_REG(MISC, 1); blk = &RU_BLK(MISC); break;
    case bdmf_address_2 : reg = &RU_REG(MISC, 2); blk = &RU_BLK(MISC); break;
    case bdmf_address_3 : reg = &RU_REG(MISC, 3); blk = &RU_BLK(MISC); break;
    default :
        return BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    if((bdmf_parm = bdmfmon_find_named_parm(session,"index1")))
    {
        index1_start = bdmf_parm->value.unumber;
        index1_stop = index1_start + 1;
    }
    else
        index1_stop = blk->addr_count;
    if((bdmf_parm = bdmfmon_find_named_parm(session,"index2")))
    {
        index2_start = bdmf_parm->value.unumber;
        index2_stop = index2_start + 1;
    }
    else
        index2_stop = reg->ram_count + 1;
    if(index1_stop > blk->addr_count)
    {
        bdmf_session_print(session, "index1 (%u) is out of range (%u).\n", index1_stop, blk->addr_count);
        return BDMF_ERR_RANGE;
    }
    if(index2_stop > (reg->ram_count + 1))
    {
        bdmf_session_print(session, "index2 (%u) is out of range (%u).\n", index2_stop, reg->ram_count + 1);
        return BDMF_ERR_RANGE;
    }
    if(reg->ram_count)
        for (i = index1_start; i < index1_stop; i++)
        {
            bdmf_session_print(session, "index1 = %u\n", i);
            for (j = index2_start; j < index2_stop; j++)
                bdmf_session_print(session, 	 "(%5u) 0x%08X\n", j, (uint32_t)((uint32_t*)(blk->addr[i] + reg->addr) + j));
        }
    else
        for (i = index1_start; i < index1_stop; i++)
            bdmf_session_print(session, "(%3u) 0x%08X\n", i, blk->addr[i]+reg->addr);
    return 0;
}

bdmfmon_handle_t ag_drv_misc_cli_init(bdmfmon_handle_t driver_dir)
{
    bdmfmon_handle_t dir;

    if ((dir = bdmfmon_dir_find(driver_dir, "misc"))!=NULL)
        return dir;
    dir = bdmfmon_dir_add(driver_dir, "misc", "misc", BDMF_ACCESS_ADMIN, NULL);

    {
        static bdmfmon_cmd_parm_t set_misc_0[]={
            BDMFMON_MAKE_PARM("cr_xgwan_top_wan_misc_pmd_lane_mode", "cr_xgwan_top_wan_misc_pmd_lane_mode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cr_xgwan_top_wan_misc_epon_tx_fifo_off", "cr_xgwan_top_wan_misc_epon_tx_fifo_off", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cr_xgwan_top_wan_misc_onu2g_phya", "cr_xgwan_top_wan_misc_onu2g_phya", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cr_xgwan_top_wan_misc_epon_tx_fifo_off_ld", "cr_xgwan_top_wan_misc_epon_tx_fifo_off_ld", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cr_xgwan_top_wan_misc_mdio_fast_mode", "cr_xgwan_top_wan_misc_mdio_fast_mode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cr_xgwan_top_wan_misc_mdio_mode", "cr_xgwan_top_wan_misc_mdio_mode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cr_xgwan_top_wan_misc_refout_en", "cr_xgwan_top_wan_misc_refout_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cr_xgwan_top_wan_misc_refin_en", "cr_xgwan_top_wan_misc_refin_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cr_xgwan_top_wan_misc_onu2g_pmd_status_sel", "cr_xgwan_top_wan_misc_onu2g_pmd_status_sel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_misc_1[]={
            BDMFMON_MAKE_PARM("cr_xgwan_top_wan_misc_pmd_core_1_mode", "cr_xgwan_top_wan_misc_pmd_core_1_mode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cr_xgwan_top_wan_misc_pmd_core_0_mode", "cr_xgwan_top_wan_misc_pmd_core_0_mode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_misc_2[]={
            BDMFMON_MAKE_PARM("cfg_apm_mux_sel_1", "cfg_apm_mux_sel_1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfg_apm_mux_sel_0", "cfg_apm_mux_sel_0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgngponrxclk", "cfgngponrxclk", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgngpontxclk", "cfgngpontxclk", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgactiveethernet2p5", "cfgactiveethernet2p5", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cr_xgwan_top_wan_misc_pmd_rx_osr_mode", "cr_xgwan_top_wan_misc_pmd_rx_osr_mode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cr_xgwan_top_wan_misc_pmd_tx_mode", "cr_xgwan_top_wan_misc_pmd_tx_mode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cr_xgwan_top_wan_misc_pmd_tx_osr_mode", "cr_xgwan_top_wan_misc_pmd_tx_osr_mode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cr_xgwan_top_wan_misc_pmd_tx_disable", "cr_xgwan_top_wan_misc_pmd_tx_disable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cr_xgwan_top_wan_misc_pmd_ln_rx_h_pwrdn", "cr_xgwan_top_wan_misc_pmd_ln_rx_h_pwrdn", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cr_xgwan_top_wan_misc_pmd_ln_tx_h_pwrdn", "cr_xgwan_top_wan_misc_pmd_ln_tx_h_pwrdn", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cr_xgwan_top_wan_misc_pmd_ext_los", "cr_xgwan_top_wan_misc_pmd_ext_los", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cr_xgwan_top_wan_misc_pmd_por_h_rstb", "cr_xgwan_top_wan_misc_pmd_por_h_rstb", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cr_xgwan_top_wan_misc_pmd_core_1_dp_h_rstb", "cr_xgwan_top_wan_misc_pmd_core_1_dp_h_rstb", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cr_xgwan_top_wan_misc_pmd_core_0_dp_h_rstb", "cr_xgwan_top_wan_misc_pmd_core_0_dp_h_rstb", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cr_xgwan_top_wan_misc_pmd_ln_h_rstb", "cr_xgwan_top_wan_misc_pmd_ln_h_rstb", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cr_xgwan_top_wan_misc_pmd_ln_dp_h_rstb", "cr_xgwan_top_wan_misc_pmd_ln_dp_h_rstb", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cr_xgwan_top_wan_misc_pmd_rx_mode", "cr_xgwan_top_wan_misc_pmd_rx_mode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_misc_3[]={
            BDMFMON_MAKE_PARM("cr_xgwan_top_wan_misc_wan_cfg_epon_debug_sel", "cr_xgwan_top_wan_misc_wan_cfg_epon_debug_sel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cr_xgwan_top_wan_misc_wan_cfg_wan_debug_sel", "cr_xgwan_top_wan_misc_wan_cfg_wan_debug_sel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cr_xgwan_top_wan_misc_wan_cfg_ntr_sync_period_sel", "cr_xgwan_top_wan_misc_wan_cfg_ntr_sync_period_sel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cr_xgwan_top_wan_misc_wan_cfg_laser_oe", "cr_xgwan_top_wan_misc_wan_cfg_laser_oe", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cr_xgwan_top_wan_misc_wan_cfg_wan_interface_select", "cr_xgwan_top_wan_misc_wan_cfg_wan_interface_select", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cr_xgwan_top_wan_misc_wan_cfg_laser_mode", "cr_xgwan_top_wan_misc_wan_cfg_laser_mode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cr_xgwan_top_wan_misc_wan_cfg_laser_invert", "cr_xgwan_top_wan_misc_wan_cfg_laser_invert", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cr_xgwan_top_wan_misc_wan_cfg_mem_reb", "cr_xgwan_top_wan_misc_wan_cfg_mem_reb", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="misc_0", .val=BDMF_misc_0, .parms=set_misc_0 },
            { .name="misc_1", .val=BDMF_misc_1, .parms=set_misc_1 },
            { .name="misc_2", .val=BDMF_misc_2, .parms=set_misc_2 },
            { .name="misc_3", .val=BDMF_misc_3, .parms=set_misc_3 },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", bcm_misc_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t set_default[]={
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="misc_0", .val=BDMF_misc_0, .parms=set_default },
            { .name="misc_1", .val=BDMF_misc_1, .parms=set_default },
            { .name="misc_2", .val=BDMF_misc_2, .parms=set_default },
            { .name="misc_3", .val=BDMF_misc_3, .parms=set_default },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_misc_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name="low" , .val=bdmf_test_method_low },
            { .name="mid" , .val=bdmf_test_method_mid },
            { .name="high" , .val=bdmf_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "test", "test", bcm_misc_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name="0" , .val=bdmf_address_0 },
            { .name="1" , .val=bdmf_address_1 },
            { .name="2" , .val=bdmf_address_2 },
            { .name="3" , .val=bdmf_address_3 },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "address", "address", bcm_misc_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
    return dir;
}
#endif /* USE_BDMF_SHELL */

