/*
   Copyright (c) 2017 Broadcom Corporation
   All Rights Reserved

   <:label-BRCM:2017:DUAL/GPL:standard

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

/*
 *  Created on: Sep 2017
 *      Author: li.xu@broadcom.com
 */

/*
 * Phy drivers for 10G Active Ethernet Serdes
 */
#include "phy_drv_dsl_serdes.h"
#include "phy_drv_merlin16.h"
#include "bcm_chip_arch.h"

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

#define MAX_146CORES 3
#define MAX_146USXGMII_PORTS 4
static phy_serdes_t serdes_146class[MAX_146CORES][MAX_146USXGMII_PORTS] =
{
    {{
        .phy_type = PHY_TYPE_146CLASS_SERDES,
        .bp_intf_type = BP_INTF_TYPE_SGMII,
#if defined(CONFIG_BCM963146)
        .speed_caps = PHY_CAP_AUTONEG|PHY_CAP_2500|PHY_CAP_1000_FULL|PHY_CAP_100_FULL,
        .inter_phy_types = INTER_PHY_TYPES_AUS1KR2KXR_M,
#elif defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813)
        .speed_caps = PHY_CAP_AUTONEG|PHY_CAP_10000|PHY_CAP_5000|PHY_CAP_2500|PHY_CAP_1000_FULL|PHY_CAP_100_FULL,
        .inter_phy_types = INTER_PHY_TYPES_AUS1KR2KXR5KXR10R_M,
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
    }},
    {{
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
    }},
    {{
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
    }}
};

static int total_cores;
int phy_drv_serdes146_get_total_cores(void)
{
    return total_cores;
}

phy_dev_t *phy_drv_serdes146_get_phy_dev(int core_num, int port_num)
{
    phy_serdes_t *phy_serdes = &serdes_146class[core_num][port_num];
    phy_dev_t *phy_dev = phy_serdes->phy_dev;
    return phy_dev;
}

static int phy_drv_serdes_146class_init_lock(phy_dev_t *phy_dev)
{
    phy_serdes_t *phy_serdes;
    phy_serdes_t *phy_serdes_base;
    uint32_t v32;

    mutex_lock(&serdes_mutex);

    phy_serdes = &serdes_146class[phy_dev->core_index][phy_dev->usxgmii_m_index];
    phy_serdes_base = &serdes_146class[phy_dev->core_index][0];
    if (phy_serdes->inited)
    {
        printk("****** Duplicated Serdes at address %d defined in board parameter.\n", phy_dev->addr);
        BUG();
    }

    phy_serdes->print_log = 1;
    if (phy_dev_is_mphy(phy_dev))   /* Copy default driver from mphy_base */
    {
        memcpy(phy_serdes, phy_serdes_base, sizeof(*phy_serdes));
        phy_serdes->inited = 0;
    }

    phy_serdes->usxgmii_m_index = phy_dev->usxgmii_m_index;
    phy_serdes->core_num = phy_dev->core_index;
    phy_serdes->handle = phy_dev->priv;
    phy_dev->priv = phy_serdes;

    phy_serdes->phy_dev = phy_dev;
    printk("\n" NtcClr "=== Start of 10G Active Ethernet Initialization for core %d port %d ===" DflClr "\n",
        phy_serdes->core_num, phy_dev->usxgmii_m_index);

    if (total_cores <= phy_serdes->core_num)
        total_cores = phy_serdes->core_num + 1;

    /* Configure Serdes MDIO address */
    v32 = *ETH_SERDES_MISC_SERDES_CNTRL(phy_serdes->core_num);
    v32 &= ~ETH_REG_SERDES_PORTAD_M;
    v32 |= phy_dev->addr << ETH_REG_SERDES_PORTAD_S;
    *ETH_SERDES_MISC_SERDES_CNTRL(phy_serdes->core_num) = v32;

    phy_dsl_serdes_init(phy_dev);

    if (!phy_dev_is_mphy(phy_dev) || is_mphy_dev_base(phy_dev))
        merlin16_serdes_init(phy_dev);

    phy_dsl_serdes_post_init(phy_dev);

    printk(NtcClr "=== End of 10G Active Ethernet Initialization for core %d port %d ===" DflClr "\n",
        phy_serdes->core_num, phy_dev->usxgmii_m_index);

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

extern int ephy_leds_init(void *leds_info);

static int _phy_leds_init(phy_dev_t *phy_dev, void *leds_info)
{
    return ephy_leds_init(leds_info);
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
    .leds_init = _phy_leds_init,

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

static int _merlin_core_power_op(phy_dev_t *phy_dev, int power_level)
{
    /* 
        For USXGMII-M mode, we will let cascading PHY do the power down and keep
        Serdes on all the time, because Serdes is the aggregation of four ports of
        cascaded Copper PHY, while cascaded Copper PHY has separate four ports 
    */
    if (phy_dev_is_mphy(phy_dev))
    {
        if (power_level == 0)
            return 0;
        phy_dev = phy_dev->mphy_base;
    }

    return merlin_core_power_op(phy_dev, power_level);
}

static int dsl_merlin16_speed_set(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex)
{
    int rc;
    int i, usxgmii_m_ports;
    phy_dev_t *phy_dev_copper_base, *phy_dev_copper;
    phy_serdes_t *phy_serdes = phy_dev->priv;

    if (phy_dev_is_mphy(phy_dev))
    {
        if (!is_mphy_dev_base(phy_dev) || phy_serdes->inited >= 2)
            return 0;
    }

    rc = merlin_speed_set(phy_dev, speed, duplex);
    if (phy_dev_is_mphy(phy_dev) && phy_dev->cascade_next && phy_serdes->sfp_module_type == SFP_FIXED_PHY)
    {
        phy_dev_copper_base = phy_dev->cascade_next;
        usxgmii_m_ports = usxgmii_m_total_ports(phy_dev->usxgmii_m_type);

        for(i=0; i<usxgmii_m_ports; i++)
        {
            phy_dev_copper = mphy_dev_get(phy_dev_copper_base, i);
            rc += phy_dev_an_restart(phy_dev_copper);
        }
    }

    phy_serdes->inited = 2;
    phy_serdes->print_log = 0;
    return rc;
}

