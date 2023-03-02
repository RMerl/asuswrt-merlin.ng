// SPDX-License-Identifier: GPL-2.0
/*
   Copyright (c) 2017 Broadcom Corporation
   All Rights Reserved

 */

/*
 *  Created on: Sep 2017
 *      Author: li.xu@broadcom.com
 */

/*
 * Phy drivers for 10G Active Ethernet Serdes
 */
#include "phy_drv_dsl_serdes.h"
#include "phy_drv_merlin16.h"
//#include "bcm_chip_arch.h"
#define BP_INTF_TYPE_SGMII          5       

#define ETH_SERDES_0_MISC_BASE          0x837ff500
#define ETH_SERDES_MISC_CORE_OFFSET          0x100
#define ETH_SERDES_MISC_SERDES_CNTRL(core)  ((volatile uint32_t *)(ETH_SERDES_0_MISC_BASE + (core)*ETH_SERDES_MISC_CORE_OFFSET))
    #define ETH_REG_SERDES_IDDQ         (1<<0)
    #define ETH_REG_SERDES_REFCLK_RESET (1<<1)
    #define ETH_REG_SERDES_RESET        (1<<2)
    #define ETH_REG_SERDES_REFSEL(x)    ((x)<<3)
    #define ETH_REG_SERDES_PORTAD_S     (6)
    #define ETH_REG_SERDES_PORTAD_M     (0x1f<<6)


static void dsl_merlin16_serdes_link_stats(phy_dev_t *phy_dev);
static int dsl_merlin16_speed_set(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex);
static int _merlin_core_power_op(phy_dev_t *phy_dev, int power_level);
static int dsl_merlin16_light_detected(phy_dev_t *phy_dev)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;
    if (phy_serdes->signal_detect_gpio == -1 || phy_serdes->sfp_module_type == SFP_FIXED_PHY)
        return 1;

    return merlin_ext_sigal_detected(phy_dev);
}

static phy_serdes_t serdes_146class[3] =
{
    {
        .phy_type = PHY_TYPE_146CLASS_SERDES,
        .bp_intf_type = BP_INTF_TYPE_SGMII,
#if defined(CONFIG_BCM963146)
        .speed_caps = PHY_CAP_AUTONEG|PHY_CAP_2500|PHY_CAP_1000_FULL|PHY_CAP_100_FULL,
        .inter_phy_types = INTER_PHY_TYPES_AUS1KR2KXR_M,
#elif defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813)
        .speed_caps = PHY_CAP_AUTONEG|PHY_CAP_5000|PHY_CAP_2500|PHY_CAP_1000_FULL|PHY_CAP_100_FULL,
        .inter_phy_types = INTER_PHY_TYPES_AUS1KR2KXR5KXR_M,
#endif
        .link_stats = dsl_merlin16_serdes_link_stats,
        .config_speed = PHY_SPEED_AUTO,
        .power_mode = SERDES_BASIC_POWER_SAVING,
        .power_admin_on = 1,
        .speed_set = dsl_merlin16_speed_set,
        .power_set = _merlin_core_power_op,
        .cur_power_level = -1,
        .light_detected = dsl_merlin16_light_detected,
        //.lbe_op = dsl_merlin16_lbe_op,
    },
    {
        .phy_type = PHY_TYPE_146CLASS_SERDES,
        .bp_intf_type = BP_INTF_TYPE_SGMII,

#if defined(CONFIG_BCM963146)
#define CONFIG_BCM963146_TESTING
#if defined(CONFIG_BCM963146_TESTING)
        .speed_caps = PHY_CAP_AUTONEG|PHY_CAP_5000|PHY_CAP_2500|PHY_CAP_1000_FULL|PHY_CAP_100_FULL,
        .inter_phy_types = INTER_PHY_TYPES_AMUS1KR2KXR5KXR_M,
#else
        .speed_caps = PHY_CAP_AUTONEG|PHY_CAP_2500|PHY_CAP_1000_FULL|PHY_CAP_100_FULL,
        .inter_phy_types = INTER_PHY_TYPES_AUS1KR2KXR_M,
#endif
#elif defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813)
        .speed_caps = PHY_CAP_AUTONEG|PHY_CAP_10000|PHY_CAP_5000|PHY_CAP_2500|PHY_CAP_1000_FULL|PHY_CAP_100_FULL,
        .inter_phy_types = INTER_PHY_TYPES_AUS1KR2KXR5KXR10R_M,
#endif
        .link_stats = dsl_merlin16_serdes_link_stats,
        .config_speed = PHY_SPEED_AUTO,
        .power_mode = SERDES_BASIC_POWER_SAVING,
        .power_admin_on = 1,
        .speed_set = dsl_merlin16_speed_set,
        .light_detected = dsl_merlin16_light_detected,
        .cur_power_level = -1,
        .power_set = _merlin_core_power_op,
        //.lbe_op = dsl_merlin16_lbe_op,
    },
    {
        .phy_type = PHY_TYPE_146CLASS_SERDES,
        .bp_intf_type = BP_INTF_TYPE_SGMII,
        .speed_caps = PHY_CAP_AUTONEG|PHY_CAP_10000|PHY_CAP_5000|PHY_CAP_2500|PHY_CAP_1000_FULL|PHY_CAP_100_FULL,
        .inter_phy_types = INTER_PHY_TYPES_AMUS1KR2KXR5KXR10R_M,
        .link_stats = dsl_merlin16_serdes_link_stats,
        .config_speed = PHY_SPEED_AUTO,
        .power_mode = SERDES_BASIC_POWER_SAVING,
        .power_admin_on = 1,
        .speed_set = dsl_merlin16_speed_set,
        .light_detected = dsl_merlin16_light_detected,
        .cur_power_level = -1,
        .power_set = _merlin_core_power_op,
        //.lbe_op = dsl_merlin16_lbe_op,
    }
};

static int total_cores;
int phy_drv_serdes146_get_total_cores(void)
{
    return total_cores;
}

phy_dev_t *phy_drv_serdes146_get_phy_dev(int core_num)
{
    phy_serdes_t *phy_serdes = &serdes_146class[core_num];
    phy_dev_t *phy_dev = phy_serdes->phy_dev;
    return phy_dev;
}

static int phy_drv_serdes_146class_init_lock(phy_dev_t *phy_dev)
{
    phy_serdes_t *phy_serdes;
    uint32_t v32;

    mutex_lock(&serdes_mutex);

    phy_serdes = phy_dev->priv = &serdes_146class[phy_dev->core_index];
    phy_serdes->core_num = phy_dev->core_index;

    if (phy_serdes->inited)
    {
        printk("****** Duplicated Serdes at address %d defined in board parameter.\n", phy_dev->addr);
        BUG();
    }
    phy_serdes->phy_dev = phy_dev;
    printk("\n" NtcClr "=== Start of 10G Active Ethernet Initialization for core %d ===" DflClr "\n", phy_serdes->core_num);

    if (total_cores <= phy_serdes->core_num)
        total_cores = phy_serdes->core_num + 1;

    /* Configure Serdes MDIO address */
    v32 = *ETH_SERDES_MISC_SERDES_CNTRL(phy_serdes->core_num);
    v32 &= ~ETH_REG_SERDES_PORTAD_M;
    v32 |= phy_dev->addr << ETH_REG_SERDES_PORTAD_S;
    *ETH_SERDES_MISC_SERDES_CNTRL(phy_serdes->core_num) = v32;

    phy_dsl_serdes_init(phy_dev);
    merlin16_serdes_init(phy_dev);
    phy_serdes->inited = 1;
    phy_dsl_serdes_post_init(phy_dev);
    phy_serdes->inited = 2;

    printk(NtcClr "=== End of 10G Active Ethernet Initialization for core %d ===" DflClr "\n", phy_serdes->core_num);

    mutex_unlock(&serdes_mutex);

    return 0;
}

static int dsl_merlin16_priv_fun_lock(phy_dev_t *phy_dev, int op_code, va_list ap)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;
    int reg;
    int val;
    uint32_t *valp;
    int ret = 0;

    mutex_lock(&serdes_mutex);

    if ((ret = dsl_serdes_check_power(phy_dev)))
        goto end;

    switch(op_code)
    {
        case PHY_OP_RD_MDIO:
            reg = va_arg(ap, int);
            valp = va_arg(ap, uint32_t *);
            *valp = merlin_pmi_read16(phy_serdes->core_num, 0, reg >> 16, reg & 0xffff);
            break;
        case PHY_OP_WR_MDIO:
            reg = va_arg(ap, int);
            val = va_arg(ap, int);
            ret = merlin_pmi_write16(phy_serdes->core_num, 0, reg >> 16, reg & 0xffff, (uint16_t)val, 0);
            break;
    }

end:
    mutex_unlock(&serdes_mutex);
    return ret;
}

static int phy_dsl_serdes_dev_add_lock(phy_dev_t *phy_dev)
{
    mutex_lock(&serdes_mutex);
	phy_dev->priv = NULL;
    mutex_unlock(&serdes_mutex);
    return 0;
}

static int phy_dsl_serdes_dt_priv(dt_handle_t handle, uint32_t addr, uint32_t phy_mode, void **_priv)
{
    *_priv = (void *)handle.np;

    return 0;
}

phy_drv_t phy_drv_serdes_146class =
{
    .init = phy_drv_serdes_146class_init_lock,
    .phy_type = PHY_TYPE_146CLASS_SERDES,
    .read_status = phy_dsl_serdes_read_status_lock,
    .apd_get = dsl_serdes_apd_get,
    .apd_set = dsl_serdes_apd_set_lock,
    .speed_set = dsl_serdes_cfg_speed_set_lock,
    .caps_get = dsl_serdes_caps_get,
    .config_speed_get = dsl_serdes_speed_get_lock,
    .priv_fun = dsl_merlin16_priv_fun_lock,
    .power_set = phy_dsl_serdes_power_set_lock,
    .power_get = phy_dsl_serdes_power_get,
    .inter_phy_types_get = dsl_serdes_inter_phy_types_get,
    .get_phy_name = dsl_serdes_get_phy_name,
    .dev_add = phy_dsl_serdes_dev_add_lock,
    .dt_priv = phy_dsl_serdes_dt_priv,

#if defined(CONFIG_BCM963146)
    .name = "2.5GAE",
#else
    .name = "10GAE",
#endif
};

static void dsl_merlin16_serdes_link_stats(phy_dev_t *phy_dev)
{
    merlin_chk_lane_link_status(phy_dev);
}

static void merlin_set_msbus_clk_source(phy_dev_t *phy_dev, int power_level)
{
#if defined(ETH_PHY_TOP_XPORT0_CLK_CNTRL)
    uint32_t v32;
    phy_serdes_t *phy_serdes = phy_dev->priv;
    int power_level0, power_level1;
    int cur_source;
    phy_serdes_t *phy_serdes0, *phy_serdes1;

    if(phy_dev->core_index > 1)
        return;

    if (phy_serdes->cur_power_level == power_level)
        return;

    phy_serdes0 = &serdes_146class[0][0];
    phy_serdes1 = &serdes_146class[1][0];

    if (phy_dev->core_index)
    {
        power_level1 = power_level;
        if (phy_serdes0->inited)
            power_level0 = serdes_146class[0][0].cur_power_level;
        else
            power_level0 = 0;
    }
    else
    {
        power_level0 = power_level;
        if (phy_serdes1->inited)
            power_level1 = serdes_146class[1][0].cur_power_level;
        else
            power_level1 = 0;
    }

    v32 = *ETH_PHY_TOP_XPORT0_CLK_CNTRL; 
    cur_source = v32 & ETHSW_XPORT0_CLK_CNTRL_MSBUS_CLK_SEL;

    /* We want to set MSBUS clock source to external XGPHY if both are available because 
       SFP module needs to scan different speedand will constantly affect peer Serdes */
    if (power_level0 && power_level1 && 
        ((phy_serdes0->sfp_module_type == SFP_FIXED_PHY && phy_serdes1->sfp_module_type != SFP_FIXED_PHY) ||
        (phy_serdes1->sfp_module_type == SFP_FIXED_PHY && phy_serdes0->sfp_module_type != SFP_FIXED_PHY)) &&
        ((cur_source == 0 && phy_serdes0->sfp_module_type != SFP_FIXED_PHY) ||
        (cur_source == 1 && phy_serdes1->sfp_module_type != SFP_FIXED_PHY)))
        goto flip;
  
    if ((cur_source && power_level1) || (!cur_source && power_level0) || (!power_level0 && !power_level1))
        return;

    /* We need to flip source */
flip:
    if (cur_source)
        v32 &= ~ETHSW_XPORT0_CLK_CNTRL_MSBUS_CLK_SEL;
    else
        v32 |= ETHSW_XPORT0_CLK_CNTRL_MSBUS_CLK_SEL;

    *ETH_PHY_TOP_XPORT0_CLK_CNTRL = v32; 
#endif
}

static int _merlin_core_power_op(phy_dev_t *phy_dev, int power_level)
{
    int ret;
    /* 
        For USXGMII-M mode, we will let cascading PHY do the power down and keep
        Serdes on all the time, because Serdes is the aggregation of four ports of
        cascaded Copper PHY, while cascaded Copper PHY has separate four ports 
    */

    merlin_set_msbus_clk_source(phy_dev, power_level);
    ret = merlin_core_power_op(phy_dev, power_level);

    return ret;
}

static int dsl_merlin16_speed_set(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex)
{
    return merlin_speed_set(phy_dev, speed, duplex);
}

