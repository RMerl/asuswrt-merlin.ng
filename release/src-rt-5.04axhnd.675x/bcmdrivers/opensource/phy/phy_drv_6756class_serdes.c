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
#include "phy_drv_merlin28.h"
#include "bcm_chip_arch.h"

static void dsl_merlin28_serdes_link_stats(phy_dev_t *phy_dev);
static int dsl_merlin28_speed_set(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex);
static int dsl_merlin28_light_detected(phy_dev_t *phy_dev)
{
#if 0
    phy_serdes_t *phy_serdes = phy_dev->priv;
    if (phy_serdes->signal_detect_gpio == -1 || phy_serdes->sfp_module_type == SFP_FIXED_PHY)
        return 1;
    return merlin28_ext_sigal_detected(phy_dev);
#endif

    return 1;
}

static phy_serdes_t serdes_6756class_extsw =
{
    .phy_type = PHY_TYPE_6756CLASS_SERDES,
    .bp_intf_type = BP_INTF_TYPE_SGMII,
    .speed_caps = PHY_CAP_2500,
    .inter_phy_types = INTER_PHY_TYPES_S1K2KR5R_M,
    .link_stats = dsl_merlin28_serdes_link_stats,
    .config_speed = PHY_SPEED_2500, /* If speed is not defined in DT, the speed here will be used */
    .power_mode = SERDES_NO_POWER_SAVING,
    .power_admin_on = 1,
    .speed_set = dsl_merlin28_speed_set,
    .power_set = merlin28_lane_power_op,
    .cur_power_level = -1,
};

static phy_serdes_t serdes_6756class_core[] =
{
    {
        .flag = CORE_STRUCTURE,
    }
};

static phy_serdes_t serdes_6756class[2][2] =
{
    {
        {
            .phy_type = PHY_TYPE_6756CLASS_SERDES,
            .bp_intf_type = BP_INTF_TYPE_SGMII,
            .speed_caps = PHY_CAP_AUTONEG|PHY_CAP_5000|PHY_CAP_2500|PHY_CAP_1000_FULL|PHY_CAP_100_FULL,
            .inter_phy_types = INTER_PHY_TYPES_S1K2KR5R_M,
            .link_stats = dsl_merlin28_serdes_link_stats,
            .config_speed = PHY_SPEED_AUTO,
            .power_mode = SERDES_BASIC_POWER_SAVING,
            .power_admin_on = 1,
            .speed_set = dsl_merlin28_speed_set,
            .power_set = merlin28_lane_power_op,
            .cur_power_level = -1,
            .light_detected = dsl_merlin28_light_detected,
            .flag = LANE_STRUCTURE,
            //.lbe_op = dsl_merlin28_lbe_op,
        },
        {
            .phy_type = PHY_TYPE_6756CLASS_SERDES,
            .bp_intf_type = BP_INTF_TYPE_SGMII,
            .speed_caps = PHY_CAP_AUTONEG|PHY_CAP_2500|PHY_CAP_1000_FULL|PHY_CAP_100_FULL,
            .inter_phy_types = INTER_PHY_TYPES_S1K2K_M,
            .link_stats = dsl_merlin28_serdes_link_stats,
            .config_speed = PHY_SPEED_AUTO,
            .power_mode = SERDES_BASIC_POWER_SAVING,
            .power_admin_on = 1,
            .speed_set = dsl_merlin28_speed_set,
            .power_set = merlin28_lane_power_op,
            .light_detected = dsl_merlin28_light_detected,
            .cur_power_level = -1,
            .flag = LANE_STRUCTURE,
            //.lbe_op = dsl_merlin28_lbe_op,
        },
    },
    #if 0
    {
        {
            .phy_type = PHY_TYPE_6756CLASS_SERDES,
            .bp_intf_type = BP_INTF_TYPE_SGMII,
            .speed_caps = PHY_CAP_AUTONEG|PHY_CAP_2500,
            .inter_phy_types = INTER_PHY_TYPES_U2K_M,
            .link_stats = dsl_merlin28_serdes_link_stats,
            .config_speed = PHY_SPEED_AUTO,
            .power_mode = SERDES_NO_POWER_SAVING,
            .power_admin_on = 1,
            .speed_set = dsl_merlin28_speed_set,
            .power_set = merlin28_lane_power_op,
            .cur_power_level = -1,
            .light_detected = dsl_merlin28_light_detected,
            //.lbe_op = dsl_merlin28_lbe_op,
        },
        {
            .phy_type = PHY_TYPE_6756CLASS_SERDES,
            .bp_intf_type = BP_INTF_TYPE_SGMII,
            .speed_caps = PHY_CAP_AUTONEG|PHY_CAP_2500,
            .inter_phy_types = INTER_PHY_TYPES_U2K_M,
            .link_stats = dsl_merlin28_serdes_link_stats,
            .config_speed = PHY_SPEED_AUTO,
            .power_mode = SERDES_NO_POWER_SAVING,
            .power_admin_on = 1,
            .speed_set = dsl_merlin28_speed_set,
            .power_set = merlin28_lane_power_op,
            .cur_power_level = -1,
            .light_detected = dsl_merlin28_light_detected,
            //.lbe_op = dsl_merlin28_lbe_op,
        },
    }
    #endif
};

static int total_lanes;
int phy_drv_serdes6756_get_total_lanes(void)
{
    return total_lanes;
}

phy_dev_t *phy_drv_serdes6756_get_phy_dev(int core_num, int lane_num)
{
    phy_serdes_t *phy_serdes = &serdes_6756class[core_num][lane_num];
    phy_dev_t *phy_dev = phy_serdes->phy_dev;
    return phy_dev;
}

static phy_drv_t phy_6756class_serdes_extsw;
static void get_dt_config_speed(phy_dev_t *phy_dev)
{
    uint32_t speed;
    phy_serdes_t *phy_serdes = phy_dev->priv;
    dt_handle_t handle = phy_serdes->handle;  // from dt_priv

    speed = dt_property_read_u32(handle, "config_speed");
    if (speed == -1)
        return;

    switch(speed)
    {
        case 2500:
            phy_serdes->config_speed = PHY_SPEED_2500;
            break;
        case 5000:
            phy_serdes->config_speed = PHY_SPEED_5000;
            break;
        case 10000:
            phy_serdes->config_speed = PHY_SPEED_5000;
            break;
    }
    return;
}

static int phy_drv_serdes_6756class_init_lock(phy_dev_t *phy_dev)
{
    phy_serdes_t *phy_serdes, *_phy_serdes;
    volatile uint32_t *xib_control = (void *) (XIB_CONTROL);
    static int xib_inited = 0;

    mutex_lock(&serdes_mutex);

    phy_serdes = _phy_serdes = phy_dev->priv;
    printknotice("=== Start of 5G Active Ethernet Initialization for core %d, lane %d ===", 
        phy_dev->core_index, phy_dev->lane_index);

    if (xib_inited == 0)
    {
        *xib_control |= XIB_CONTROL_RX_EN | XIB_CONTROL_TX_EN | XIB_CONTROL_LINK_DN_RST_EN;
        xib_inited = 1;
    }

    if (PhyIsPortConnectedToExternalSwitch(phy_dev))
    {
        phy_dev->phy_drv = &phy_6756class_serdes_extsw;
        phy_serdes = phy_dev->priv = &serdes_6756class_extsw;
        phy_serdes->priv = _phy_serdes->priv; /* copy core serdes structure pointer over */
        phy_serdes->handle = _phy_serdes->handle;
        /* Inter switch speed will use definition in the following priority
            1. DT, 2. Init value in serdes_6756class_extsw  3. highest speed */
        get_dt_config_speed(phy_dev);
        if (phy_serdes->config_speed == PHY_SPEED_UNKNOWN)   
            phy_serdes->config_speed = phy_serdes->highest_speed = 
                phy_caps_to_max_speed(phy_serdes->speed_caps & (~PHY_CAP_AUTONEG));
        phy_serdes->current_speed = phy_serdes->config_speed;
    }

    phy_dsl_serdes_init(phy_dev);
    merlin28_serdes_init(phy_dev);
    phy_serdes->inited = 2;
    phy_dsl_serdes_post_init(phy_dev);
    phy_serdes->inited = 3;

    printknotice("=== End of 5G Active Ethernet Initialization for core %d, lane %d ===", 
        phy_dev->core_index, phy_dev->lane_index);

    mutex_unlock(&serdes_mutex);

    return 0;
}

static int dsl_merlin28_priv_fun(phy_dev_t *phy_dev, int op_code, va_list ap)
{
    int reg;
    int val;
    uint32_t *valp;
    int ret = 0;

    switch(op_code)
    {
        case PHY_OP_RD_MDIO:
            reg = va_arg(ap, int);
            valp = va_arg(ap, uint32_t *);
            *valp = merlin28_pmi_read16(phy_dev->core_index, phy_dev->lane_index, reg >> 16, reg & 0xffff);
            break;
        case PHY_OP_WR_MDIO:
            reg = va_arg(ap, int);
            val = va_arg(ap, int);
            ret = merlin28_pmi_write16(phy_dev->core_index, phy_dev->lane_index, reg >> 16, reg & 0xffff, (uint16_t)val, 0);
            break;
    }
    return ret;
}

static phy_drv_t phy_6756class_serdes_extsw =
{
    .phy_type = PHY_TYPE_6756CLASS_SERDES,
    .read_status = phy_dsl_serdes_read_status_lock,
    .power_get = phy_dsl_serdes_power_get,
    .caps_get = dsl_serdes_caps_get,
    .config_speed_get = dsl_serdes_speed_get_lock,
    .priv_fun = dsl_merlin28_priv_fun,

    .name = "5GToEXTSW",
};

static int _phy_dsl_serdes_dev_add_lock(phy_dev_t *phy_dev)
{
    int rc;
    phy_serdes_t *phy_serdes;

    rc = phy_dsl_serdes_dev_add_lock(phy_dev);
    
    mutex_lock(&serdes_mutex);

    if (phy_dev->core_index > ARRAY_SIZE(serdes_6756class_core))
        BUG_CHECK("Core number exceeds prepared data structure.\n");

    phy_serdes = &serdes_6756class[phy_dev->core_index][phy_dev->lane_index];
    phy_serdes->priv = &serdes_6756class_core[phy_dev->core_index];
    phy_serdes->phy_dev = phy_dev;
    phy_serdes->handle = phy_dev->priv;
    phy_dev->priv = phy_serdes;
    phy_serdes->core_num = phy_dev->core_index;

    if (total_lanes <= phy_dev->lane_index)
        total_lanes = phy_dev->lane_index + 1;

    if (phy_serdes->inited)
        BUG_CHECK("****** Duplicated Serdes at address %d defined in board parameter.\n", phy_dev->addr);

    phy_serdes->inited = 1;

    mutex_unlock(&serdes_mutex);
    return 0;
}

phy_drv_t phy_drv_serdes_6756class =
{
    .init = phy_drv_serdes_6756class_init_lock,
    .phy_type = PHY_TYPE_6756CLASS_SERDES,
    .read_status = phy_dsl_serdes_read_status_lock,
    .apd_get = dsl_serdes_apd_get,
    .apd_set = dsl_serdes_apd_set_lock,
    .speed_set = dsl_serdes_cfg_speed_set_lock,
    .caps_get = dsl_serdes_caps_get,
    .config_speed_get = dsl_serdes_speed_get_lock,
    .priv_fun = dsl_merlin28_priv_fun,
    .power_set = phy_dsl_serdes_power_set_lock,
    .power_get = phy_dsl_serdes_power_get,
    .inter_phy_types_get = dsl_serdes_inter_phy_types_get,
    .get_phy_name = dsl_serdes_get_phy_name,
    .dev_add = _phy_dsl_serdes_dev_add_lock,
    .dt_priv = phy_dsl_serdes_dt_priv,
    .name = "5GAE",
};

static void dsl_merlin28_serdes_link_stats(phy_dev_t *phy_dev)
{
    merlin28_chk_lane_link_status(phy_dev);
}

static int dsl_merlin28_speed_set(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex)
{
    return merlin28_speed_set(phy_dev, speed, duplex);
}

