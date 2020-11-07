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
#include "serdes_status_ag.h"
int ag_drv_serdes_status_pll_ctl_set(const serdes_status_pll_ctl *pll_ctl)
{
    uint32_t reg_pll_ctl=0;

#ifdef VALIDATE_PARMS
    if(!pll_ctl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((pll_ctl->pll0_refin_en >= _1BITS_MAX_VAL_) ||
       (pll_ctl->pll0_refout_en >= _1BITS_MAX_VAL_) ||
       (pll_ctl->pll0_lcref_sel >= _1BITS_MAX_VAL_) ||
       (pll_ctl->pll1_refin_en >= _1BITS_MAX_VAL_) ||
       (pll_ctl->pll1_refout_en >= _1BITS_MAX_VAL_) ||
       (pll_ctl->pll1_lcref_sel >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_pll_ctl = RU_FIELD_SET(0, SERDES_STATUS, PLL_CTL, PLL0_REFIN_EN, reg_pll_ctl, pll_ctl->pll0_refin_en);
    reg_pll_ctl = RU_FIELD_SET(0, SERDES_STATUS, PLL_CTL, PLL0_REFOUT_EN, reg_pll_ctl, pll_ctl->pll0_refout_en);
    reg_pll_ctl = RU_FIELD_SET(0, SERDES_STATUS, PLL_CTL, PLL0_LCREF_SEL, reg_pll_ctl, pll_ctl->pll0_lcref_sel);
    reg_pll_ctl = RU_FIELD_SET(0, SERDES_STATUS, PLL_CTL, PLL1_REFIN_EN, reg_pll_ctl, pll_ctl->pll1_refin_en);
    reg_pll_ctl = RU_FIELD_SET(0, SERDES_STATUS, PLL_CTL, PLL1_REFOUT_EN, reg_pll_ctl, pll_ctl->pll1_refout_en);
    reg_pll_ctl = RU_FIELD_SET(0, SERDES_STATUS, PLL_CTL, PLL1_LCREF_SEL, reg_pll_ctl, pll_ctl->pll1_lcref_sel);

    RU_REG_WRITE(0, SERDES_STATUS, PLL_CTL, reg_pll_ctl);

    return 0;
}

int ag_drv_serdes_status_pll_ctl_get(serdes_status_pll_ctl *pll_ctl)
{
    uint32_t reg_pll_ctl=0;

#ifdef VALIDATE_PARMS
    if(!pll_ctl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, SERDES_STATUS, PLL_CTL, reg_pll_ctl);

    pll_ctl->pll0_refin_en = RU_FIELD_GET(0, SERDES_STATUS, PLL_CTL, PLL0_REFIN_EN, reg_pll_ctl);
    pll_ctl->pll0_refout_en = RU_FIELD_GET(0, SERDES_STATUS, PLL_CTL, PLL0_REFOUT_EN, reg_pll_ctl);
    pll_ctl->pll0_lcref_sel = RU_FIELD_GET(0, SERDES_STATUS, PLL_CTL, PLL0_LCREF_SEL, reg_pll_ctl);
    pll_ctl->pll1_refin_en = RU_FIELD_GET(0, SERDES_STATUS, PLL_CTL, PLL1_REFIN_EN, reg_pll_ctl);
    pll_ctl->pll1_refout_en = RU_FIELD_GET(0, SERDES_STATUS, PLL_CTL, PLL1_REFOUT_EN, reg_pll_ctl);
    pll_ctl->pll1_lcref_sel = RU_FIELD_GET(0, SERDES_STATUS, PLL_CTL, PLL1_LCREF_SEL, reg_pll_ctl);

    return 0;
}

int ag_drv_serdes_status_temp_ctl_get(uint16_t *wan_temperature_read)
{
    uint32_t reg_temp_ctl=0;

#ifdef VALIDATE_PARMS
    if(!wan_temperature_read)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, SERDES_STATUS, TEMP_CTL, reg_temp_ctl);

    *wan_temperature_read = RU_FIELD_GET(0, SERDES_STATUS, TEMP_CTL, WAN_TEMPERATURE_READ, reg_temp_ctl);

    return 0;
}

int ag_drv_serdes_status_pram_ctl_set(uint16_t pram_address, uint8_t pram_we, uint8_t pram_go)
{
    uint32_t reg_pram_ctl=0;

#ifdef VALIDATE_PARMS
    if((pram_we >= _1BITS_MAX_VAL_) ||
       (pram_go >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_pram_ctl = RU_FIELD_SET(0, SERDES_STATUS, PRAM_CTL, PRAM_ADDRESS, reg_pram_ctl, pram_address);
    reg_pram_ctl = RU_FIELD_SET(0, SERDES_STATUS, PRAM_CTL, PRAM_WE, reg_pram_ctl, pram_we);
    reg_pram_ctl = RU_FIELD_SET(0, SERDES_STATUS, PRAM_CTL, PRAM_GO, reg_pram_ctl, pram_go);

    RU_REG_WRITE(0, SERDES_STATUS, PRAM_CTL, reg_pram_ctl);

    return 0;
}

int ag_drv_serdes_status_pram_ctl_get(uint16_t *pram_address, uint8_t *pram_we, uint8_t *pram_go)
{
    uint32_t reg_pram_ctl=0;

#ifdef VALIDATE_PARMS
    if(!pram_address || !pram_we || !pram_go)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, SERDES_STATUS, PRAM_CTL, reg_pram_ctl);

    *pram_address = RU_FIELD_GET(0, SERDES_STATUS, PRAM_CTL, PRAM_ADDRESS, reg_pram_ctl);
    *pram_we = RU_FIELD_GET(0, SERDES_STATUS, PRAM_CTL, PRAM_WE, reg_pram_ctl);
    *pram_go = RU_FIELD_GET(0, SERDES_STATUS, PRAM_CTL, PRAM_GO, reg_pram_ctl);

    return 0;
}

int ag_drv_serdes_status_pram_val_low_set(uint32_t val)
{
    uint32_t reg_pram_val_low=0;

#ifdef VALIDATE_PARMS
#endif

    reg_pram_val_low = RU_FIELD_SET(0, SERDES_STATUS, PRAM_VAL_LOW, VAL, reg_pram_val_low, val);

    RU_REG_WRITE(0, SERDES_STATUS, PRAM_VAL_LOW, reg_pram_val_low);

    return 0;
}

int ag_drv_serdes_status_pram_val_low_get(uint32_t *val)
{
    uint32_t reg_pram_val_low=0;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, SERDES_STATUS, PRAM_VAL_LOW, reg_pram_val_low);

    *val = RU_FIELD_GET(0, SERDES_STATUS, PRAM_VAL_LOW, VAL, reg_pram_val_low);

    return 0;
}

int ag_drv_serdes_status_pram_val_high_set(uint32_t val)
{
    uint32_t reg_pram_val_high=0;

#ifdef VALIDATE_PARMS
#endif

    reg_pram_val_high = RU_FIELD_SET(0, SERDES_STATUS, PRAM_VAL_HIGH, VAL, reg_pram_val_high, val);

    RU_REG_WRITE(0, SERDES_STATUS, PRAM_VAL_HIGH, reg_pram_val_high);

    return 0;
}

int ag_drv_serdes_status_pram_val_high_get(uint32_t *val)
{
    uint32_t reg_pram_val_high=0;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, SERDES_STATUS, PRAM_VAL_HIGH, reg_pram_val_high);

    *val = RU_FIELD_GET(0, SERDES_STATUS, PRAM_VAL_HIGH, VAL, reg_pram_val_high);

    return 0;
}

int ag_drv_serdes_status_oversample_ctrl_set(const serdes_status_oversample_ctrl *oversample_ctrl)
{
    uint32_t reg_oversample_ctrl=0;

#ifdef VALIDATE_PARMS
    if(!oversample_ctrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((oversample_ctrl->cfg_gpon_rx_clk >= _2BITS_MAX_VAL_) ||
       (oversample_ctrl->txfifo_rd_legacy_mode >= _1BITS_MAX_VAL_) ||
       (oversample_ctrl->txlbe_ser_en >= _1BITS_MAX_VAL_) ||
       (oversample_ctrl->txlbe_ser_init_val >= _3BITS_MAX_VAL_) ||
       (oversample_ctrl->txlbe_ser_order >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_oversample_ctrl = RU_FIELD_SET(0, SERDES_STATUS, OVERSAMPLE_CTRL, CFG_GPON_RX_CLK, reg_oversample_ctrl, oversample_ctrl->cfg_gpon_rx_clk);
    reg_oversample_ctrl = RU_FIELD_SET(0, SERDES_STATUS, OVERSAMPLE_CTRL, TXFIFO_RD_LEGACY_MODE, reg_oversample_ctrl, oversample_ctrl->txfifo_rd_legacy_mode);
    reg_oversample_ctrl = RU_FIELD_SET(0, SERDES_STATUS, OVERSAMPLE_CTRL, TXLBE_SER_EN, reg_oversample_ctrl, oversample_ctrl->txlbe_ser_en);
    reg_oversample_ctrl = RU_FIELD_SET(0, SERDES_STATUS, OVERSAMPLE_CTRL, TXLBE_SER_INIT_VAL, reg_oversample_ctrl, oversample_ctrl->txlbe_ser_init_val);
    reg_oversample_ctrl = RU_FIELD_SET(0, SERDES_STATUS, OVERSAMPLE_CTRL, TXLBE_SER_ORDER, reg_oversample_ctrl, oversample_ctrl->txlbe_ser_order);

    RU_REG_WRITE(0, SERDES_STATUS, OVERSAMPLE_CTRL, reg_oversample_ctrl);

    return 0;
}

int ag_drv_serdes_status_oversample_ctrl_get(serdes_status_oversample_ctrl *oversample_ctrl)
{
    uint32_t reg_oversample_ctrl=0;

#ifdef VALIDATE_PARMS
    if(!oversample_ctrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, SERDES_STATUS, OVERSAMPLE_CTRL, reg_oversample_ctrl);

    oversample_ctrl->cfg_gpon_rx_clk = RU_FIELD_GET(0, SERDES_STATUS, OVERSAMPLE_CTRL, CFG_GPON_RX_CLK, reg_oversample_ctrl);
    oversample_ctrl->txfifo_rd_legacy_mode = RU_FIELD_GET(0, SERDES_STATUS, OVERSAMPLE_CTRL, TXFIFO_RD_LEGACY_MODE, reg_oversample_ctrl);
    oversample_ctrl->txlbe_ser_en = RU_FIELD_GET(0, SERDES_STATUS, OVERSAMPLE_CTRL, TXLBE_SER_EN, reg_oversample_ctrl);
    oversample_ctrl->txlbe_ser_init_val = RU_FIELD_GET(0, SERDES_STATUS, OVERSAMPLE_CTRL, TXLBE_SER_INIT_VAL, reg_oversample_ctrl);
    oversample_ctrl->txlbe_ser_order = RU_FIELD_GET(0, SERDES_STATUS, OVERSAMPLE_CTRL, TXLBE_SER_ORDER, reg_oversample_ctrl);

    return 0;
}

