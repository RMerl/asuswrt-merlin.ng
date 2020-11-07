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
int ag_drv_wan_serdes_pll_ctl_set(const wan_serdes_pll_ctl *pll_ctl)
{
    uint32_t reg_pll_ctl=0;

#ifdef VALIDATE_PARMS
    if(!pll_ctl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((pll_ctl->cfg_pll1_lcref_sel >= _1BITS_MAX_VAL_) ||
       (pll_ctl->cfg_pll1_refout_en >= _1BITS_MAX_VAL_) ||
       (pll_ctl->cfg_pll1_refin_en >= _1BITS_MAX_VAL_) ||
       (pll_ctl->cfg_pll0_lcref_sel >= _1BITS_MAX_VAL_) ||
       (pll_ctl->cfg_pll0_refout_en >= _1BITS_MAX_VAL_) ||
       (pll_ctl->cfg_pll0_refin_en >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_pll_ctl = RU_FIELD_SET(0, WAN_SERDES, PLL_CTL, CFG_PLL1_LCREF_SEL, reg_pll_ctl, pll_ctl->cfg_pll1_lcref_sel);
    reg_pll_ctl = RU_FIELD_SET(0, WAN_SERDES, PLL_CTL, CFG_PLL1_REFOUT_EN, reg_pll_ctl, pll_ctl->cfg_pll1_refout_en);
    reg_pll_ctl = RU_FIELD_SET(0, WAN_SERDES, PLL_CTL, CFG_PLL1_REFIN_EN, reg_pll_ctl, pll_ctl->cfg_pll1_refin_en);
    reg_pll_ctl = RU_FIELD_SET(0, WAN_SERDES, PLL_CTL, CFG_PLL0_LCREF_SEL, reg_pll_ctl, pll_ctl->cfg_pll0_lcref_sel);
    reg_pll_ctl = RU_FIELD_SET(0, WAN_SERDES, PLL_CTL, CFG_PLL0_REFOUT_EN, reg_pll_ctl, pll_ctl->cfg_pll0_refout_en);
    reg_pll_ctl = RU_FIELD_SET(0, WAN_SERDES, PLL_CTL, CFG_PLL0_REFIN_EN, reg_pll_ctl, pll_ctl->cfg_pll0_refin_en);

    RU_REG_WRITE(0, WAN_SERDES, PLL_CTL, reg_pll_ctl);

    return 0;
}

int ag_drv_wan_serdes_pll_ctl_get(wan_serdes_pll_ctl *pll_ctl)
{
    uint32_t reg_pll_ctl=0;

#ifdef VALIDATE_PARMS
    if(!pll_ctl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, WAN_SERDES, PLL_CTL, reg_pll_ctl);

    pll_ctl->cfg_pll1_lcref_sel = RU_FIELD_GET(0, WAN_SERDES, PLL_CTL, CFG_PLL1_LCREF_SEL, reg_pll_ctl);
    pll_ctl->cfg_pll1_refout_en = RU_FIELD_GET(0, WAN_SERDES, PLL_CTL, CFG_PLL1_REFOUT_EN, reg_pll_ctl);
    pll_ctl->cfg_pll1_refin_en = RU_FIELD_GET(0, WAN_SERDES, PLL_CTL, CFG_PLL1_REFIN_EN, reg_pll_ctl);
    pll_ctl->cfg_pll0_lcref_sel = RU_FIELD_GET(0, WAN_SERDES, PLL_CTL, CFG_PLL0_LCREF_SEL, reg_pll_ctl);
    pll_ctl->cfg_pll0_refout_en = RU_FIELD_GET(0, WAN_SERDES, PLL_CTL, CFG_PLL0_REFOUT_EN, reg_pll_ctl);
    pll_ctl->cfg_pll0_refin_en = RU_FIELD_GET(0, WAN_SERDES, PLL_CTL, CFG_PLL0_REFIN_EN, reg_pll_ctl);

    return 0;
}

int ag_drv_wan_serdes_temp_ctl_get(uint16_t *wan_temperature_data)
{
    uint32_t reg_temp_ctl=0;

#ifdef VALIDATE_PARMS
    if(!wan_temperature_data)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, WAN_SERDES, TEMP_CTL, reg_temp_ctl);

    *wan_temperature_data = RU_FIELD_GET(0, WAN_SERDES, TEMP_CTL, WAN_TEMPERATURE_DATA, reg_temp_ctl);

    return 0;
}

int ag_drv_wan_serdes_pram_ctl_set(const wan_serdes_pram_ctl *pram_ctl)
{
    uint32_t reg_pram_ctl=0;

#ifdef VALIDATE_PARMS
    if(!pram_ctl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((pram_ctl->cfg_pram_go >= _1BITS_MAX_VAL_) ||
       (pram_ctl->cfg_pram_we >= _1BITS_MAX_VAL_) ||
       (pram_ctl->cfg_pram_cs >= _1BITS_MAX_VAL_) ||
       (pram_ctl->cfg_pram_ability >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_pram_ctl = RU_FIELD_SET(0, WAN_SERDES, PRAM_CTL, CFG_PRAM_GO, reg_pram_ctl, pram_ctl->cfg_pram_go);
    reg_pram_ctl = RU_FIELD_SET(0, WAN_SERDES, PRAM_CTL, CFG_PRAM_WE, reg_pram_ctl, pram_ctl->cfg_pram_we);
    reg_pram_ctl = RU_FIELD_SET(0, WAN_SERDES, PRAM_CTL, CFG_PRAM_CS, reg_pram_ctl, pram_ctl->cfg_pram_cs);
    reg_pram_ctl = RU_FIELD_SET(0, WAN_SERDES, PRAM_CTL, CFG_PRAM_ABILITY, reg_pram_ctl, pram_ctl->cfg_pram_ability);
    reg_pram_ctl = RU_FIELD_SET(0, WAN_SERDES, PRAM_CTL, CFG_PRAM_DATAIN, reg_pram_ctl, pram_ctl->cfg_pram_datain);
    reg_pram_ctl = RU_FIELD_SET(0, WAN_SERDES, PRAM_CTL, CFG_PRAM_ADDR, reg_pram_ctl, pram_ctl->cfg_pram_addr);

    RU_REG_WRITE(0, WAN_SERDES, PRAM_CTL, reg_pram_ctl);

    return 0;
}

int ag_drv_wan_serdes_pram_ctl_get(wan_serdes_pram_ctl *pram_ctl)
{
    uint32_t reg_pram_ctl=0;

#ifdef VALIDATE_PARMS
    if(!pram_ctl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, WAN_SERDES, PRAM_CTL, reg_pram_ctl);

    pram_ctl->cfg_pram_go = RU_FIELD_GET(0, WAN_SERDES, PRAM_CTL, CFG_PRAM_GO, reg_pram_ctl);
    pram_ctl->cfg_pram_we = RU_FIELD_GET(0, WAN_SERDES, PRAM_CTL, CFG_PRAM_WE, reg_pram_ctl);
    pram_ctl->cfg_pram_cs = RU_FIELD_GET(0, WAN_SERDES, PRAM_CTL, CFG_PRAM_CS, reg_pram_ctl);
    pram_ctl->cfg_pram_ability = RU_FIELD_GET(0, WAN_SERDES, PRAM_CTL, CFG_PRAM_ABILITY, reg_pram_ctl);
    pram_ctl->cfg_pram_datain = RU_FIELD_GET(0, WAN_SERDES, PRAM_CTL, CFG_PRAM_DATAIN, reg_pram_ctl);
    pram_ctl->cfg_pram_addr = RU_FIELD_GET(0, WAN_SERDES, PRAM_CTL, CFG_PRAM_ADDR, reg_pram_ctl);

    return 0;
}

int ag_drv_wan_serdes_pram_ctl_2_set(uint32_t cfg_pram_datain_0)
{
    uint32_t reg_pram_ctl_2=0;

#ifdef VALIDATE_PARMS
#endif

    reg_pram_ctl_2 = RU_FIELD_SET(0, WAN_SERDES, PRAM_CTL_2, CFG_PRAM_DATAIN_0, reg_pram_ctl_2, cfg_pram_datain_0);

    RU_REG_WRITE(0, WAN_SERDES, PRAM_CTL_2, reg_pram_ctl_2);

    return 0;
}

int ag_drv_wan_serdes_pram_ctl_2_get(uint32_t *cfg_pram_datain_0)
{
    uint32_t reg_pram_ctl_2=0;

#ifdef VALIDATE_PARMS
    if(!cfg_pram_datain_0)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, WAN_SERDES, PRAM_CTL_2, reg_pram_ctl_2);

    *cfg_pram_datain_0 = RU_FIELD_GET(0, WAN_SERDES, PRAM_CTL_2, CFG_PRAM_DATAIN_0, reg_pram_ctl_2);

    return 0;
}

int ag_drv_wan_serdes_pram_ctl_3_set(uint32_t cfg_pram_datain_1)
{
    uint32_t reg_pram_ctl_3=0;

#ifdef VALIDATE_PARMS
#endif

    reg_pram_ctl_3 = RU_FIELD_SET(0, WAN_SERDES, PRAM_CTL_3, CFG_PRAM_DATAIN_1, reg_pram_ctl_3, cfg_pram_datain_1);

    RU_REG_WRITE(0, WAN_SERDES, PRAM_CTL_3, reg_pram_ctl_3);

    return 0;
}

int ag_drv_wan_serdes_pram_ctl_3_get(uint32_t *cfg_pram_datain_1)
{
    uint32_t reg_pram_ctl_3=0;

#ifdef VALIDATE_PARMS
    if(!cfg_pram_datain_1)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, WAN_SERDES, PRAM_CTL_3, reg_pram_ctl_3);

    *cfg_pram_datain_1 = RU_FIELD_GET(0, WAN_SERDES, PRAM_CTL_3, CFG_PRAM_DATAIN_1, reg_pram_ctl_3);

    return 0;
}

