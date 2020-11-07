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
#include "wan_serdes_ag.h"
bdmf_error_t ag_drv_wan_serdes_wan_serdes_pll_ctl_set(const wan_serdes_wan_serdes_pll_ctl *wan_serdes_pll_ctl)
{
    uint32_t reg_pll_ctl=0;

#ifdef VALIDATE_PARMS
    if(!wan_serdes_pll_ctl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((wan_serdes_pll_ctl->pll0_refin_en >= _1BITS_MAX_VAL_) ||
       (wan_serdes_pll_ctl->pll0_refout_en >= _1BITS_MAX_VAL_) ||
       (wan_serdes_pll_ctl->pll0_lcref_sel >= _1BITS_MAX_VAL_) ||
       (wan_serdes_pll_ctl->pll1_refin_en >= _1BITS_MAX_VAL_) ||
       (wan_serdes_pll_ctl->pll1_refout_en >= _1BITS_MAX_VAL_) ||
       (wan_serdes_pll_ctl->pll1_lcref_sel >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pll_ctl = RU_FIELD_SET(0, WAN_SERDES, PLL_CTL, PLL0_REFIN_EN, reg_pll_ctl, wan_serdes_pll_ctl->pll0_refin_en);
    reg_pll_ctl = RU_FIELD_SET(0, WAN_SERDES, PLL_CTL, PLL0_REFOUT_EN, reg_pll_ctl, wan_serdes_pll_ctl->pll0_refout_en);
    reg_pll_ctl = RU_FIELD_SET(0, WAN_SERDES, PLL_CTL, PLL0_LCREF_SEL, reg_pll_ctl, wan_serdes_pll_ctl->pll0_lcref_sel);
    reg_pll_ctl = RU_FIELD_SET(0, WAN_SERDES, PLL_CTL, PLL1_REFIN_EN, reg_pll_ctl, wan_serdes_pll_ctl->pll1_refin_en);
    reg_pll_ctl = RU_FIELD_SET(0, WAN_SERDES, PLL_CTL, PLL1_REFOUT_EN, reg_pll_ctl, wan_serdes_pll_ctl->pll1_refout_en);
    reg_pll_ctl = RU_FIELD_SET(0, WAN_SERDES, PLL_CTL, PLL1_LCREF_SEL, reg_pll_ctl, wan_serdes_pll_ctl->pll1_lcref_sel);

    RU_REG_WRITE(0, WAN_SERDES, PLL_CTL, reg_pll_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_wan_serdes_wan_serdes_pll_ctl_get(wan_serdes_wan_serdes_pll_ctl *wan_serdes_pll_ctl)
{
    uint32_t reg_pll_ctl=0;

#ifdef VALIDATE_PARMS
    if(!wan_serdes_pll_ctl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, WAN_SERDES, PLL_CTL, reg_pll_ctl);

    wan_serdes_pll_ctl->pll0_refin_en = RU_FIELD_GET(0, WAN_SERDES, PLL_CTL, PLL0_REFIN_EN, reg_pll_ctl);
    wan_serdes_pll_ctl->pll0_refout_en = RU_FIELD_GET(0, WAN_SERDES, PLL_CTL, PLL0_REFOUT_EN, reg_pll_ctl);
    wan_serdes_pll_ctl->pll0_lcref_sel = RU_FIELD_GET(0, WAN_SERDES, PLL_CTL, PLL0_LCREF_SEL, reg_pll_ctl);
    wan_serdes_pll_ctl->pll1_refin_en = RU_FIELD_GET(0, WAN_SERDES, PLL_CTL, PLL1_REFIN_EN, reg_pll_ctl);
    wan_serdes_pll_ctl->pll1_refout_en = RU_FIELD_GET(0, WAN_SERDES, PLL_CTL, PLL1_REFOUT_EN, reg_pll_ctl);
    wan_serdes_pll_ctl->pll1_lcref_sel = RU_FIELD_GET(0, WAN_SERDES, PLL_CTL, PLL1_LCREF_SEL, reg_pll_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_wan_serdes_wan_serdes_temp_ctl_get(uint16_t *wan_temperature_read)
{
    uint32_t reg_temp_ctl=0;

#ifdef VALIDATE_PARMS
    if(!wan_temperature_read)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, WAN_SERDES, TEMP_CTL, reg_temp_ctl);

    *wan_temperature_read = RU_FIELD_GET(0, WAN_SERDES, TEMP_CTL, WAN_TEMPERATURE_READ, reg_temp_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_wan_serdes_wan_serdes_pram_ctl_set(bdmf_boolean pram_go, bdmf_boolean pram_we, uint16_t pram_address)
{
    uint32_t reg_pram_ctl=0;

#ifdef VALIDATE_PARMS
    if((pram_we >= _1BITS_MAX_VAL_) ||
       (pram_go >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pram_ctl = RU_FIELD_SET(0, WAN_SERDES, PRAM_CTL, PRAM_ADDRESS, reg_pram_ctl, pram_address);
    reg_pram_ctl = RU_FIELD_SET(0, WAN_SERDES, PRAM_CTL, PRAM_WE, reg_pram_ctl, pram_we);
    reg_pram_ctl = RU_FIELD_SET(0, WAN_SERDES, PRAM_CTL, PRAM_GO, reg_pram_ctl, pram_go);

    RU_REG_WRITE(0, WAN_SERDES, PRAM_CTL, reg_pram_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_wan_serdes_wan_serdes_pram_ctl_get(bdmf_boolean *pram_go, bdmf_boolean *pram_we, uint16_t *pram_address)
{
    uint32_t reg_pram_ctl=0;

#ifdef VALIDATE_PARMS
    if(!pram_address || !pram_we || !pram_go)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, WAN_SERDES, PRAM_CTL, reg_pram_ctl);

    *pram_address = RU_FIELD_GET(0, WAN_SERDES, PRAM_CTL, PRAM_ADDRESS, reg_pram_ctl);
    *pram_we = RU_FIELD_GET(0, WAN_SERDES, PRAM_CTL, PRAM_WE, reg_pram_ctl);
    *pram_go = RU_FIELD_GET(0, WAN_SERDES, PRAM_CTL, PRAM_GO, reg_pram_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_wan_serdes_wan_serdes_pram_val_low_set(uint32_t val)
{
    uint32_t reg_pram_val_low=0;

#ifdef VALIDATE_PARMS
#endif

    reg_pram_val_low = RU_FIELD_SET(0, WAN_SERDES, PRAM_VAL_LOW, VAL, reg_pram_val_low, val);

    RU_REG_WRITE(0, WAN_SERDES, PRAM_VAL_LOW, reg_pram_val_low);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_wan_serdes_wan_serdes_pram_val_low_get(uint32_t *val)
{
    uint32_t reg_pram_val_low=0;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, WAN_SERDES, PRAM_VAL_LOW, reg_pram_val_low);

    *val = RU_FIELD_GET(0, WAN_SERDES, PRAM_VAL_LOW, VAL, reg_pram_val_low);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_wan_serdes_wan_serdes_pram_val_high_set(uint32_t val)
{
    uint32_t reg_pram_val_high=0;

#ifdef VALIDATE_PARMS
#endif

    reg_pram_val_high = RU_FIELD_SET(0, WAN_SERDES, PRAM_VAL_HIGH, VAL, reg_pram_val_high, val);

    RU_REG_WRITE(0, WAN_SERDES, PRAM_VAL_HIGH, reg_pram_val_high);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_wan_serdes_wan_serdes_pram_val_high_get(uint32_t *val)
{
    uint32_t reg_pram_val_high=0;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, WAN_SERDES, PRAM_VAL_HIGH, reg_pram_val_high);

    *val = RU_FIELD_GET(0, WAN_SERDES, PRAM_VAL_HIGH, VAL, reg_pram_val_high);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_wan_serdes_top_osr_ctrl_set(const wan_serdes_top_osr_ctrl *top_osr_ctrl)
{
    uint32_t reg_oversample_ctrl=0;

#ifdef VALIDATE_PARMS
    if(!top_osr_ctrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((top_osr_ctrl->top_osr_control_cfg_gpon_rx_clk >= _2BITS_MAX_VAL_) ||
       (top_osr_ctrl->top_osr_control_txfifo_rd_legacy_mode >= _1BITS_MAX_VAL_) ||
       (top_osr_ctrl->top_osr_control_txlbe_ser_en >= _1BITS_MAX_VAL_) ||
       (top_osr_ctrl->top_osr_control_txlbe_ser_init_val >= _3BITS_MAX_VAL_) ||
       (top_osr_ctrl->top_osr_control_txlbe_ser_order >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_oversample_ctrl = RU_FIELD_SET(0, WAN_SERDES, OVERSAMPLE_CTRL, CFG_GPON_RX_CLK, reg_oversample_ctrl, top_osr_ctrl->top_osr_control_cfg_gpon_rx_clk);
    reg_oversample_ctrl = RU_FIELD_SET(0, WAN_SERDES, OVERSAMPLE_CTRL, TXFIFO_RD_LEGACY_MODE, reg_oversample_ctrl, top_osr_ctrl->top_osr_control_txfifo_rd_legacy_mode);
    reg_oversample_ctrl = RU_FIELD_SET(0, WAN_SERDES, OVERSAMPLE_CTRL, TXLBE_SER_EN, reg_oversample_ctrl, top_osr_ctrl->top_osr_control_txlbe_ser_en);
    reg_oversample_ctrl = RU_FIELD_SET(0, WAN_SERDES, OVERSAMPLE_CTRL, TXLBE_SER_INIT_VAL, reg_oversample_ctrl, top_osr_ctrl->top_osr_control_txlbe_ser_init_val);
    reg_oversample_ctrl = RU_FIELD_SET(0, WAN_SERDES, OVERSAMPLE_CTRL, TXLBE_SER_ORDER, reg_oversample_ctrl, top_osr_ctrl->top_osr_control_txlbe_ser_order);

    RU_REG_WRITE(0, WAN_SERDES, OVERSAMPLE_CTRL, reg_oversample_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_wan_serdes_top_osr_ctrl_get(wan_serdes_top_osr_ctrl *top_osr_ctrl)
{
    uint32_t reg_oversample_ctrl=0;

#ifdef VALIDATE_PARMS
    if(!top_osr_ctrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, WAN_SERDES, OVERSAMPLE_CTRL, reg_oversample_ctrl);

    top_osr_ctrl->top_osr_control_cfg_gpon_rx_clk = RU_FIELD_GET(0, WAN_SERDES, OVERSAMPLE_CTRL, CFG_GPON_RX_CLK, reg_oversample_ctrl);
    top_osr_ctrl->top_osr_control_txfifo_rd_legacy_mode = RU_FIELD_GET(0, WAN_SERDES, OVERSAMPLE_CTRL, TXFIFO_RD_LEGACY_MODE, reg_oversample_ctrl);
    top_osr_ctrl->top_osr_control_txlbe_ser_en = RU_FIELD_GET(0, WAN_SERDES, OVERSAMPLE_CTRL, TXLBE_SER_EN, reg_oversample_ctrl);
    top_osr_ctrl->top_osr_control_txlbe_ser_init_val = RU_FIELD_GET(0, WAN_SERDES, OVERSAMPLE_CTRL, TXLBE_SER_INIT_VAL, reg_oversample_ctrl);
    top_osr_ctrl->top_osr_control_txlbe_ser_order = RU_FIELD_GET(0, WAN_SERDES, OVERSAMPLE_CTRL, TXLBE_SER_ORDER, reg_oversample_ctrl);

    return BDMF_ERR_OK;
}

