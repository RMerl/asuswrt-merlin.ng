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

/*
 *  Created on: Dec 2016
 *      Author: steven.hsieh@broadcom.com
 */

/*
 * Phy drivers for 63138, 63148, 4908
 */
#include "phy_drv_dsl_serdes.h"
#define PHY_SPEED_HIGHEST_SUPPORT PHY_SPEED_10000

DEFINE_MUTEX(serdes_mutex);

#define PHY_CAP_AUTO_SPEED_MASK ((PHY_CAP_AUTONEG<<1)-1)

enum {SFP_MODULE_OUT, SFP_MODULE_IN, SFP_LINK_UP};

int phy_dsl_serdes_init(phy_dev_t *phy_dev);
static phy_serdes_t *serdes[MAX_SERDES_NUMBER];
static int serdes_index;

static void set_common_speed_range(phy_dev_t *phy_dev)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;

    if (phy_serdes->sfp_module_type == SFP_FIXED_PHY ||
        !(phy_dev->cascade_next->flag & PHY_FLAG_NOT_PRESENTED))
        cascade_phy_dev_caps_get(phy_dev, CAPS_TYPE_SUPPORTED, &phy_serdes->common_speed_caps);
    else
        phy_serdes->common_speed_caps = phy_serdes->speed_caps;

    phy_serdes->common_highest_speed = phy_caps_to_max_speed(phy_serdes->common_speed_caps);
    phy_serdes->common_highest_speed_cap = phy_speed_to_cap(phy_serdes->common_highest_speed, PHY_DUPLEX_FULL);
    phy_serdes->common_lowest_speed = phy_caps_to_min_speed(phy_serdes->common_speed_caps);
    phy_serdes->common_lowest_speed_cap = phy_speed_to_cap(phy_serdes->common_lowest_speed, PHY_DUPLEX_FULL);
}

static int get_dt_config_xfi(phy_dev_t *phy_dev)
{
    const char *_config_xfi;
    int an_enabled = 0;
    char config_xfi[256];

    phy_serdes_t *phy_serdes = phy_dev->priv;
    dt_handle_t handle = phy_serdes->handle;  // from dt_priv

    _config_xfi = dt_property_read_string(handle, "config-xfi");
    if (!_config_xfi)
        return -1;

    strncpy(config_xfi, _config_xfi, sizeof(config_xfi)-1);
    config_xfi[sizeof(config_xfi)-1]=0;

    if ((an_enabled = strcasecmp(config_xfi+strlen(config_xfi)-strlen("_A"), "_A") == 0))
        config_xfi[strlen(config_xfi)-strlen("_A")] = 0;

    if (strcasecmp(config_xfi, "1000Base-X") == 0)
        phy_dev->current_inter_phy_type = INTER_PHY_TYPE_1000BASE_X;
    if (strcasecmp(config_xfi, "2500Base-X") == 0)
        phy_dev->current_inter_phy_type = INTER_PHY_TYPE_2500BASE_X;
    if (strcasecmp(config_xfi, "2.5GBase-X") == 0)
        phy_dev->current_inter_phy_type = INTER_PHY_TYPE_2P5GBASE_X;
    if (strcasecmp(config_xfi, "5GBase-R") == 0)
        phy_dev->current_inter_phy_type = INTER_PHY_TYPE_5GBASE_R;
    if (strcasecmp(config_xfi, "10GBase-R") == 0)
        phy_dev->current_inter_phy_type = INTER_PHY_TYPE_10GBASE_R;
    if (strcasecmp(config_xfi, "USXGMII") == 0)
        phy_dev->current_inter_phy_type = INTER_PHY_TYPE_USXGMII;

    if (an_enabled && !INTER_PHY_TYPE_AN_SUPPORT(phy_dev->current_inter_phy_type))
    {
        printkwarn("Serdes at address %d with Mode %s does not support AN enabled, ignore it.\n", 
            phy_dev->addr, config_xfi);
        an_enabled = 0;
    }
    if (INTER_PHY_TYPE_AN_ONLY(phy_dev->current_inter_phy_type))
        an_enabled = 1;

    phy_dev->an_enabled = an_enabled;
    phy_dev->common_inter_phy_types = phy_dev->configured_inter_phy_types = (1<<phy_dev->current_inter_phy_type);
    if ((phy_dev->inter_phy_types & phy_dev->configured_inter_phy_types) == 0)
        BUG_CHECK("Configured XFI Mode in DT File Is Not Supported by the Serdes at address %d\n", phy_dev->addr);

    return 0;
}

static int get_dt_config_speed(phy_dev_t *phy_dev)
{
    uint32_t speed;
    phy_serdes_t *phy_serdes = phy_dev->priv;
    dt_handle_t handle = phy_serdes->handle;  // from dt_priv

    if ((speed = dt_property_read_u32(handle, "config-speed")) == -1)
    {
    	speed = dt_property_read_u32(handle, "config_speed");
    	if (speed == -1)
            return -1;
        else
            printkwarn("\"config_speed\" in DT file is deprecated, please use \"config-speed\"");
    }

    switch(speed)
    {
        case 1000:
            phy_serdes->config_speed = PHY_SPEED_1000;
            break;
        case 2500:
            phy_serdes->config_speed = PHY_SPEED_2500;
            break;
        case 5000:
            phy_serdes->config_speed = PHY_SPEED_5000;
            break;
        case 10000:
            phy_serdes->config_speed = PHY_SPEED_10000;
            break;
    }
    return 0;
}

int phy_dsl_serdes_init(phy_dev_t *phy_dev)
{
    phy_serdes_t *phy_serdes = (phy_serdes_t *)phy_dev->priv;
    int ret = 0;
    int bug = 0;

    if ((serdes_index+1) > ARRAY_SIZE(serdes))
        BUG_CHECK("******** ERROR: Too many Serdeses number to support.\n");

    serdes[serdes_index++] = phy_serdes;
    phy_dev->inter_phy_types = phy_serdes->inter_phy_types;

    phy_serdes->highest_speed = phy_caps_to_max_speed(phy_serdes->speed_caps & (~PHY_CAP_AUTONEG));
    phy_serdes->lowest_speed = phy_caps_to_min_speed(phy_serdes->speed_caps & (~PHY_CAP_AUTONEG));
    phy_serdes->highest_speed_cap = phy_speed_to_cap(phy_serdes->highest_speed, PHY_DUPLEX_FULL);
    phy_serdes->lowest_speed_cap = phy_speed_to_cap(phy_serdes->lowest_speed, PHY_DUPLEX_FULL);
    phy_serdes->usxgmii_m_index = phy_dev->usxgmii_m_index;
    set_common_speed_range(phy_dev);

    if (PhyIsPortConnectedToExternalSwitch(phy_dev) || PhyIsFixedConnection(phy_dev))
    {
            /* Inter switch speed will use definition in the following priority
            1. DT, 2. Init config_speed value in phy_serdes_t 3. highest speed */
        if (get_dt_config_speed(phy_dev) == 0)
            phy_serdes->current_speed = phy_serdes->config_speed;
            
        if (get_dt_config_xfi(phy_dev) == 0)
        {
            uint32_t supported_speed_caps;
            get_inter_phy_supported_speed_caps(phy_dev->configured_inter_phy_types, &supported_speed_caps);
            phy_serdes->current_speed = phy_serdes->config_speed = phy_caps_to_max_speed(supported_speed_caps);
        }
            
        if (phy_serdes->config_speed == PHY_SPEED_UNKNOWN)
            phy_serdes->config_speed = phy_serdes->highest_speed;
        phy_serdes->power_mode = SERDES_NO_POWER_SAVING;
        dsl_serdes_power_set(phy_dev, 1);
        phy_serdes->signal_detect_gpio = -1;
        goto end;
    }

    phy_serdes->laser_status = -1;
    dsl_serdes_sfp_lbe_op(phy_dev, LASER_OFF); /* Notify no SFP to turn off laser in the beginning, just in case hardware set on */
    phy_serdes->sfp_module_type = SFP_FIXED_PHY;

#if defined(CONFIG_I2C) && defined(CONFIG_BCM_OPTICALDET)
    if ((phy_dev->cascade_next == NULL))
    {
        phy_serdes->sfp_module_type = SFP_NO_MODULE;
        phy_drv_dsl_i2c_create_lock(phy_dev);
    }
#endif

    /* Get SFP MOD_ABS GPIO definition */
    phy_serdes->signal_detect_gpio = -1;
    phy_serdes->sfp_module_detect_gpio = -1;

    if (phy_serdes->sfp_module_type == SFP_NO_MODULE) {   /* Copper PHY should not check LOS */
        if (BpGetSfpDetectGpio(&phy_serdes->sfp_module_detect_gpio) == BP_SUCCESS
#if defined(CONFIG_BP_PHYS_INTF)
                || BpGetSfpModDetectGpio(phy_serdes->bp_intf_type,
                    phy_dev->core_index + phy_dev->lane_index, &phy_serdes->sfp_module_detect_gpio) == BP_SUCCESS
                    /* If we have multi core with multi lane chip in the future, interface index here need to change */
#endif
           ) {
            phy_serdes->sfp_module_detect_gpio &= BP_GPIO_NUM_MASK;
        }

        /* Get LOS GPIO definition */
        if (BpGetSgmiiGpios(&phy_serdes->signal_detect_gpio) == BP_SUCCESS
#if defined(CONFIG_BP_PHYS_INTF)
                || BpGetSfpSigDetect(phy_serdes->bp_intf_type,
                    phy_serdes->core_num, &phy_serdes->signal_detect_gpio) == BP_SUCCESS
#endif
           ) {
            phy_serdes->signal_detect_invert = (phy_serdes->signal_detect_gpio & BP_ACTIVE_LOW) > 0;
            phy_serdes->signal_detect_gpio &= BP_GPIO_NUM_MASK;
        }
    }

    /* Validate MOD_ABS GPIO definition */
    if (phy_serdes->sfp_module_detect_gpio != -1) {
        printk("GPIO %d set as SFP MOD_ABS for %s Serdes addr %d module insertion detection\n",
                phy_serdes->sfp_module_detect_gpio, phy_dev_speed_to_str(phy_serdes->highest_speed),
                phy_dev->addr);
#ifndef CONFIG_BRCM_QEMU
        kerSysSetGpioDirInput(phy_serdes->sfp_module_detect_gpio);
#endif
    }
    else {
        if (phy_serdes->sfp_module_type == SFP_NO_MODULE) { /* SFP(No Copper PHY) case */
                printkwarn("Warning: ***** No GPIO Pin defined for %s Serdes addr %d SFP module insertion.",
                        phy_dev_speed_to_str(phy_serdes->highest_speed), phy_dev->addr);
        }
    }

    /* Validate LOS GPIO Definition */
    if (phy_serdes->signal_detect_gpio != -1) {
        printk("GPIO %d set as %s Serdes addr %d Loss Of Signal Detection\n",
                phy_serdes->signal_detect_gpio, phy_dev_speed_to_str(phy_serdes->highest_speed), phy_dev->addr);
    }
    else {
        if (phy_serdes->sfp_module_type == SFP_NO_MODULE) { /* SFP(No Copper PHY) case */
            printk("Error: ****** No GPIO for Loss Of Signal Detection defined for %s Serdes addr %d.\n",
                    phy_dev_speed_to_str(phy_serdes->highest_speed), phy_dev->addr);
            printk("              Wrong board parameters or wrong board design.\n");
            bug = 1;
        }
    }

    if (bug)
        BUG_CHECK("Serdes Initialization failed\n");
end:
    return ret;
}

char *dsl_serdes_get_phy_name(phy_dev_t *phy_dev)
{
    static char buf[128];
    phy_serdes_t *phy_serdes = phy_dev->priv;
    int sz = sizeof(buf), n = 0;

    if(phy_dev_is_mphy(phy_dev))
        n += snprintf(buf+n, sz-n, "%s#M%d.%d", phy_dev->phy_drv->name,
            phy_dev->core_index, phy_serdes->usxgmii_m_index);
    else
        n += snprintf(buf+n, sz-n, "%s#L%d.%d", phy_dev->phy_drv->name,
            phy_dev->core_index, phy_dev->lane_index);
    return buf;
}

static int phy_dsl_set_configured_types(phy_dev_t *phy_dev)
{
    phy_dev_t *next_phy;

    cascade_phy_set_common_inter_types(phy_dev);
    phy_dev->configured_inter_phy_types = phy_dev->common_inter_phy_types;
    next_phy = cascade_phy_get_next(phy_dev);
    if (next_phy)
    {
        phy_dev_configured_inter_phy_types_set(next_phy, INTER_PHY_TYPE_UP, phy_dev->common_inter_phy_types);
    }
    return 0;
}

/* best type for SFP module */
static inter_phy_type_t sfp_phy_get_best_inter_phy_configure_type(phy_dev_t *phy_dev,
    inter_phy_type_t inter_phy_types, phy_speed_t speed)
{
    phy_dev_t *phy_i2c = phy_dev->cascade_next;
    inter_phy_type_t best_type;

    /* Check if non multi speed mode is available since most SFP module only support non multi speed mode */
    if (phy_i2c->flag & PHY_FLAG_DYNAMIC && !(phy_i2c->flag & PHY_FLAG_NOT_PRESENTED) &&
        (phy_i2c->flag & PHY_FLAG_COPPER_CONFIGURABLE_SFP_TYPE))
        best_type = phy_get_best_inter_phy_configure_type(phy_dev,
                inter_phy_types & ((~INTER_PHY_TYPE_MULTI_SPEED_AN_MASK_M)|INTER_PHY_TYPE_SGMII_M), speed);
    else
        best_type = phy_get_best_inter_phy_configure_type(phy_dev,
                inter_phy_types & (~INTER_PHY_TYPE_MULTI_SPEED_AN_MASK_M), speed);


    /* If not available for this speed, then check multi speed mode availability */
    if (best_type == INTER_PHY_TYPE_UNKNOWN)
        best_type = phy_get_best_inter_phy_configure_type(phy_dev, inter_phy_types, speed);

    return best_type;
}

int phy_dsl_serdes_post_init(phy_dev_t *phy_dev)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;

    phy_dsl_set_configured_types(phy_dev);
    if (phy_serdes->sfp_module_type == SFP_FIXED_PHY)
    {

        if (phy_dev->common_inter_phy_types == 0)
            /* Just unbelive this can happen on board design, should be software bug */
#ifdef TUFAX3000_V2
            printk("No common INTER PHY Capablities found between Serdes and PHY! Wrong board design.\n");
#else
            BUG_CHECK("No common INTER PHY Capablities found between Serdes and PHY! Wrong board design.\n");
#endif

        phy_dev->current_inter_phy_type = phy_get_best_inter_phy_configure_type(phy_dev,
                phy_dev->configured_inter_phy_types, phy_serdes->config_speed);
        if (!PhyIsPortConnectedToExternalSwitch(phy_dev) && !PhyIsFixedConnection(phy_dev) &&
            !IS_USXGMII_MULTI_PORTS(phy_dev) && !phy_dev_is_broadcom_phy(phy_dev->cascade_next))
        /* Work around for some non Broadcom PHYs not sync link status between Copper side and Serdes side */
        /* Exclude external switch connection from power down operation */
        {
            dsl_serdes_power_set(phy_dev, 0);
            return 0;
        }
    }
    else
        phy_dev->current_inter_phy_type = sfp_phy_get_best_inter_phy_configure_type(phy_dev,
                phy_dev->configured_inter_phy_types, phy_serdes->config_speed);

    return dsl_serdes_cfg_speed_set(phy_dev, phy_serdes->config_speed, PHY_DUPLEX_FULL);
}

static int phy_dsl_serdes_power_set(phy_dev_t *phy_dev, int enable)
{
    int rc = 0;
    phy_serdes_t *phy_serdes = (phy_serdes_t *)phy_dev->priv;

    phy_serdes->power_admin_on = enable > 0;

    /* Bypass power on when external PHY link is down; - For non BRCM PHY and power saving */
    /* Exclude external switch connection from power down operation */
    if (!PhyIsPortConnectedToExternalSwitch(phy_dev) && !PhyIsFixedConnection(phy_dev) &&
        phy_serdes->sfp_module_type == SFP_FIXED_PHY && phy_dev->usxgmii_m_type == USXGMII_M_NONE &&
        phy_dev->cascade_next->link == 0 && enable && phy_serdes->power_mode >= SERDES_BASIC_POWER_SAVING &&
        !phy_dev_is_broadcom_phy(phy_dev->cascade_next))
        return 0;

    dsl_serdes_power_set(phy_dev, enable);

    if (enable)
        dsl_serdes_cfg_speed_set(phy_dev, phy_serdes->config_speed, PHY_DUPLEX_FULL);
    return rc;
}

int phy_dsl_serdes_power_set_lock(phy_dev_t *phy_dev, int enable)
{
    int rc;
    mutex_lock(&serdes_mutex);
    rc = phy_dsl_serdes_power_set(phy_dev, enable);
    mutex_unlock(&serdes_mutex);
    return rc;
}

int phy_dsl_serdes_power_get(phy_dev_t *phy_dev, int *enable)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;

    *enable = phy_serdes->power_admin_on > 0;
    return 0;
}

int dsl_phy_enable_an(phy_dev_t *phy_dev)
{
    u16 val16 = 0;

    phy_bus_read(phy_dev, MII_CONTROL, &val16);

    if (val16 & MII_CONTROL_AN_ENABLE)
        return 0;

    val16 |= MII_CONTROL_AN_ENABLE|MII_CONTROL_RESTART_AUTONEG;
    phy_bus_write(phy_dev, MII_CONTROL, val16);
    msleep(50);
    return 1;
}

#include "crossbar_dev.h"
phy_dev_t *crossbar_get_phy_by_type(int phy_type);

#define SERDES_AUTO_DETECT_INT_MS 300
#define SERDES_FIBER_SETTLE_DELAY_MS 200

int dsl_serdes_check_power(phy_dev_t *phy_dev)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;

    if (phy_serdes->cur_power_level == SERDES_POWER_DOWN)
    {
        if (!phy_serdes->power_admin_on)
            return ETHCTL_ERROR_POWER_ADMIN_DOWN;
        else
            return ETHCTL_ERROR_POWER_SAVING_DOWN;
    }
    return 0;
}

int dsl_serdes_single_speed_set(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;
    int rc = 0;
    int cfg_speed_cap;
    int org_power_level = phy_serdes->cur_power_level;

    if (!phy_serdes || !phy_serdes->inited)
        return 0;

    if (speed == PHY_SPEED_AUTO && !((1<<phy_dev->current_inter_phy_type) & INTER_PHY_TYPE_MULTI_SPEED_AN_MASK_M))
        speed = phy_serdes->highest_speed;

    cfg_speed_cap = phy_speed_to_cap(speed, PHY_DUPLEX_FULL);
    if (!(cfg_speed_cap & phy_serdes->common_speed_caps)) {
        printk("Not supported speed: 0x%x\n", speed);
        rc = -1;
        goto end;
    }

   if (phy_dev->cascade_next && (phy_dev->cascade_next->flag & PHY_FLAG_DYNAMIC) && (phy_serdes->sfp_module_type == SFP_NO_MODULE))
        goto end;

    if (phy_serdes->cur_power_level == SERDES_POWER_DOWN)
        dsl_powerup_serdes(phy_dev);

    phy_serdes->current_speed = speed;
    rc += phy_serdes->speed_set(phy_dev, speed, duplex);

end:
    if (org_power_level != phy_serdes->cur_power_level)
        dsl_powerdown_serdes(phy_dev);

    return rc;
}

static void link_status_check(phy_dev_t *phy_dev)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;
    phy_dev_t *phy_next = phy_dev->cascade_next;

    if (phy_serdes->sfp_module_type == SFP_FIXED_PHY || (phy_next->flag & PHY_FLAG_COPPER_SFP_TYPE) ||
            !dsl_serdes_silent_start_supported(phy_dev) || phy_dev->link)
    {
        phy_serdes->link_stats(phy_dev);
        return;
    }

    if (!dsl_serdes_silent_start_light_detected(phy_dev))
    {
        dsl_serdes_sfp_lbe_op(phy_dev, LASER_OFF);
        return;
    }

    dsl_serdes_sfp_lbe_op(phy_dev, LASER_ON);
    msleep(SERDES_AUTO_DETECT_INT_MS);

    phy_serdes->link_stats(phy_dev);
}

/*
   The testing result shows lower speed will be easier to link up
   during the fibre insertion, thus we are doing retry of the highest
   speed when linked in non highest speed.
 */
#define PHY_CAP_SPEED_MASK ((PHY_CAP_10000 << 1) - 1)
#define IS_PHY_HIGHEST_SPEED_CAP(speed_caps, curSpeed) (!((speed_caps & (PHY_CAP_SPEED_MASK)) & (~((curSpeed<<1)-1))))

static phy_speed_t get_next_lower_speed(phy_speed_t speed)
{
    int i;
    static phy_speed_t phy_speed_array[] = {
        PHY_SPEED_10, PHY_SPEED_100, PHY_SPEED_1000, PHY_SPEED_2500,
        PHY_SPEED_5000, PHY_SPEED_10000};

    for(i = 0; i < ARRAY_SIZE(phy_speed_array); i++)
        if (speed == phy_speed_array[i])
            break;

    if (i==0 || i == ARRAY_SIZE(phy_speed_array))
        return PHY_SPEED_UNKNOWN;

    return phy_speed_array[i-1];
}

static void dsl_serdes_speed_detect(phy_dev_t *phy_dev)
{
    static int retry = 0;
    phy_serdes_t *phy_serdes = phy_dev->priv;
    u16 rnd;
    phy_dev_t *phy_i2c = phy_dev->cascade_next;
    inter_phy_type_t inter_type;
    inter_phy_type_t highest_speed_inter_phy_type = INTER_PHY_TYPE_UNKNOWN;
    uint32_t inter_phy_types;
    phy_speed_t speed;

    if (PhyIsPortConnectedToExternalSwitch(phy_dev) || PhyIsFixedConnection(phy_dev))
        return;
    if (phy_serdes->config_speed != PHY_SPEED_AUTO)
        return;

    /* Introduce random phase shift to get bigger chanse to link up on back to back connection */
    if (retry == 0)
    {
        get_random_bytes(&rnd, sizeof(rnd));
        msleep(1000 * (rnd%100) / 100);
    }

    if (retry == 0 &&  phy_serdes->sfp_status >= SFP_MODULE_IN)
    {
        link_status_check(phy_dev);
        if (phy_dev->link) goto end;
    }

    /* Scan different multi speed AN Serdes type */
    inter_phy_types = (phy_dev->configured_inter_phy_types & INTER_PHY_TYPE_MULTI_SPEED_AN_MASK_M) &
        (~INTER_PHY_TYPE_SGMII_M); // Let's do SGMII detection in the end to avoid mis link up with 1000Base-X

    for (inter_type = INTER_PHY_TYPE_UNKNOWN - 1; inter_phy_types; inter_type--)
    {
        if ((inter_phy_types & (1<<inter_type)) == 0)
            continue;

        phy_dev->current_inter_phy_type = inter_type;
        phy_i2c->current_inter_phy_type = inter_type;
        phy_dev_speed_set(phy_i2c, PHY_SPEED_AUTO, PHY_DUPLEX_FULL);
        dsl_serdes_single_speed_set(phy_dev, PHY_SPEED_AUTO, PHY_DUPLEX_FULL);

        msleep(SERDES_AUTO_DETECT_INT_MS);
        link_status_check(phy_dev);
        inter_phy_types &= ~(1<<inter_type);
        if (phy_dev->link)
            goto LinkUp;
    }

    for (speed = phy_serdes->common_highest_speed; speed >= phy_serdes->common_lowest_speed;
            speed = get_next_lower_speed(speed))
    {
        if(!(phy_speed_to_cap(speed, PHY_DUPLEX_FULL) & phy_serdes->common_speed_caps))
            continue;

        /* Scan different Serdes type for a certain speed */
        inter_phy_types = phy_dev->configured_inter_phy_types & phy_speed_to_inter_phy_speed_mask(speed);
        for (inter_type = INTER_PHY_TYPE_MIN; inter_phy_types; inter_type++)
        {
            if ((inter_phy_types & (1<<inter_type)) == 0)
                continue;

            if (highest_speed_inter_phy_type == INTER_PHY_TYPE_UNKNOWN)
                highest_speed_inter_phy_type = inter_type;

            phy_dev->current_inter_phy_type = inter_type;
            phy_i2c->current_inter_phy_type = inter_type;
            phy_dev_speed_set(phy_i2c, speed, PHY_DUPLEX_FULL);
            if(dsl_serdes_single_speed_set(phy_dev, speed, PHY_DUPLEX_FULL))
            {
                inter_phy_types &= ~(1<<inter_type);
                continue;
            }
            inter_phy_types &= ~(1<<inter_type);

            msleep(SERDES_AUTO_DETECT_INT_MS);
            link_status_check(phy_dev);
            if (phy_dev->link)
                goto LinkUp;
        }
    }

    /* Test SGMII AN in the last */
    if ((inter_phy_types = phy_dev->configured_inter_phy_types & INTER_PHY_TYPE_SGMII_M))
    {
        inter_type = INTER_PHY_TYPE_SGMII;
        phy_dev->current_inter_phy_type = inter_type;
        phy_i2c->current_inter_phy_type = inter_type;
        phy_dev_speed_set(phy_i2c, PHY_SPEED_AUTO, PHY_DUPLEX_FULL);
        dsl_serdes_single_speed_set(phy_dev, PHY_SPEED_AUTO, PHY_DUPLEX_FULL);

        msleep(SERDES_AUTO_DETECT_INT_MS);
        link_status_check(phy_dev);
        if (phy_dev->link)
            goto LinkUp;
    }
    goto NoLinkUp;

LinkUp:
    if (phy_dev->speed == phy_serdes->common_highest_speed)
        goto end;

    if (retry)
    {
        phy_dev_status_reverse_propagate(phy_dev);
        goto end; /* If we retried already, return; */
    }

    /* Otherwise, take a sleep to let fibre settle down, then retry higher speed */
    retry++;
    phy_dev->link = 0;  /* Set link down to insure lower level driver link up process correct during retry */
    msleep(SERDES_FIBER_SETTLE_DELAY_MS);
    dsl_serdes_speed_detect(phy_dev);
    goto end;

NoLinkUp:
    /*
        No link up here.
        Set speed to highest when in NO_POWER_SAVING_MODE until next detection
    */
    if( phy_serdes->power_mode == SERDES_NO_POWER_SAVING)
    {
        phy_dev->current_inter_phy_type = sfp_phy_get_best_inter_phy_configure_type(phy_dev,
            phy_dev->configured_inter_phy_types, phy_serdes->common_highest_speed);
        if (INTER_PHY_TYPE_IS_MULTI_SPEED_AN(phy_dev->current_inter_phy_type))
            dsl_serdes_single_speed_set(phy_dev, PHY_SPEED_AUTO, PHY_DUPLEX_FULL);
        else
        {
            phy_dev->current_inter_phy_type = highest_speed_inter_phy_type;
            dsl_serdes_single_speed_set(phy_dev, phy_serdes->common_highest_speed, PHY_DUPLEX_FULL);
        }
    }
end:
    retry = 0;
}

int dsl_serdes_caps_get(phy_dev_t *phy_dev, int caps_type, uint32_t *caps)
{
    phy_dsl_priv_t *phy_dsl_priv = phy_dev->priv;

    if (caps_type == CAPS_TYPE_SUPPORTED)
        *caps = phy_dsl_priv->speed_caps;
    else if (caps_type == CAPS_TYPE_ADVERTISE)
    {
        if (phy_dsl_priv->config_speed == PHY_SPEED_AUTO)
            *caps = phy_dsl_priv->speed_caps|PHY_CAP_AUTONEG;
        else
            *caps = phy_speed_to_cap(phy_dsl_priv->config_speed, PHY_DUPLEX_FULL);
    }
    else
        *caps = 0;

    return 0;
}

int dsl_serdes_cfg_speed_set_lock(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex)
{
    int rc = 0;
    phy_serdes_t *phy_serdes = (phy_serdes_t *)phy_dev->priv;

    mutex_lock(&serdes_mutex);

    if (!PhyIsPortConnectedToExternalSwitch(phy_dev) && !PhyIsFixedConnection(phy_dev) &&
        !phy_dev_is_broadcom_phy(phy_dev->cascade_next) &&
        phy_serdes->sfp_module_type == SFP_FIXED_PHY && phy_dev->usxgmii_m_type == USXGMII_M_NONE)
    {   /* Work around for some non Broadcom PHYs not sync link status between Copper side and Serdes side */
        /* Exclude external switch connection from power down operation */
        if (phy_dev->cascade_next->link == 0)
        {
            dsl_serdes_power_set(phy_dev, 0);
            phy_dev->link = 0;
            goto ret;
        }
        else
            dsl_serdes_power_set(phy_dev, 1);
    }

    rc = dsl_serdes_cfg_speed_set(phy_dev, speed, duplex);
ret:
    mutex_unlock(&serdes_mutex);
    return rc;
}

int dsl_serdes_cfg_speed_set(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;
    phy_dev_t *copper_phy = phy_dev->cascade_next;
    int rc = 0;
    int i;

    phy_serdes->config_speed = speed;

    if (!phy_serdes->power_admin_on)
        return 0;

    if (phy_serdes->sfp_module_type == SFP_FIXED_PHY)
    {
        phy_dev->current_inter_phy_type = phy_get_best_inter_phy_configure_type(phy_dev,
                phy_dev->configured_inter_phy_types, phy_serdes->config_speed);

        dsl_powerup_serdes(phy_dev);

        rc = dsl_serdes_single_speed_set(phy_dev, speed, duplex);
        if (IS_USER_CONFIG())
            phy_serdes->link_changed = 1;
        else {
            /* If it is copper PHY and its link is up, wait until Serdes link up
               for MAC configuration timing */
            if(copper_phy->link)
            {
                for (i=0; i<20; i++)
                {
                    phy_serdes->link_stats(phy_dev);
                    if (phy_dev->link)
                        break;
                    msleep(10);
                }

                if (!phy_dev->link)
                    printkwarn("Warning: Serdes at %d link does not go up following external copper PHY at %d.",
                        phy_dev->addr, copper_phy->addr);
        	}

        	if (!PhyIsPortConnectedToExternalSwitch(phy_dev) && !PhyIsFixedConnection(phy_dev))
        	{
                phy_dev->link = 0;

                if( phy_serdes->power_mode == SERDES_ADVANCED_POWER_SAVING)
                    dsl_powerdown_serdes(phy_dev);
            }
        }

    }
    else    /* SFP Module */
    {
        phy_dev->current_inter_phy_type = sfp_phy_get_best_inter_phy_configure_type(phy_dev,
                phy_dev->configured_inter_phy_types, phy_serdes->config_speed);

        if (IS_USER_CONFIG())
        /* Force it down for hardware not link down long enough for software to detect */
            phy_serdes->link_changed = 1;

        /* When calling from user configuration. I2C PHY does not know it needs to set current_inter_phy_type
            because it can't distiguish speed scanning call from user configuration call. Only Serdes is aware
            this difference, so Serdes needs to set I2C PHY's current_inter_phy_type here in user configuration case */
        if (phy_dev->current_inter_phy_type != INTER_PHY_TYPE_UNKNOWN)  /* Otherwise need speed scanning */
        {
            phy_dev_current_inter_phy_types_set(copper_phy, INTER_PHY_TYPE_DOWN, phy_dev->current_inter_phy_type);
            rc = dsl_serdes_single_speed_set(phy_dev, speed, duplex);
            rc += phy_dev_speed_set(copper_phy, speed, duplex);
        }
    }

    return rc;
}
EXPORT_SYMBOL(dsl_serdes_cfg_speed_set);

void phy_drv_sfp_group_list(void)
{
    phy_serdes_t *phy_serdes;
    char *serdes_type;
    char *serdes_status;
    int i, sfp_serdeses = 0;
    phy_dev_t *phy_dev;

    for (i = 0; i < MAX_SERDES_NUMBER; i++) {
        if (!serdes[i]) break;;

        phy_serdes = serdes[i];
        phy_dev = phy_serdes->phy_dev;
        if (!(phy_dev->cascade_next->flag & PHY_FLAG_DYNAMIC)) continue;

        sfp_serdeses++;
        switch(phy_serdes->highest_speed_cap) {
            case PHY_CAP_10000:
                serdes_type = "10G AE Serdes";
                break;
            case PHY_CAP_2500:
                serdes_type = "2.5G Serdes";
                break;
            case PHY_CAP_1000_FULL:
                serdes_type = "1G Serdes";
                break;
            default:
                serdes_type = "unknown";
        }

        switch(phy_serdes->sfp_status) {
            case SFP_MODULE_OUT:
                serdes_status = "unplug";
                break;
            case SFP_MODULE_IN:
                serdes_status = "plug in";
                break;
            case SFP_LINK_UP:
                serdes_status = "link up";
                break;
            default:
                serdes_status = "unkown";
                break;
        }

        if (sfp_serdeses == 1)
            printk("%-16s %-16s %-16s\n", "Serdes ID", "Serdes Type", "SFP Status");
        if (phy_serdes->phy_type == PHY_TYPE_158CLASS_SERDES)
            printk("%-16s %-16s %-16s\n", "N/A", serdes_type, serdes_status);
        else
            printk("%-16d %-16s %-16s\n", phy_serdes->phy_dev->addr, serdes_type, serdes_status);
    }

    if (sfp_serdeses == 0)
        printk(" No SFP design in this board\n");
}

static int dsl_sfp_module_detected(phy_dev_t *phy_dev)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;
    int rc = 0;
    int rc_i2c = 0;

    if (phy_serdes->sfp_module_type == SFP_FIXED_PHY)
        return 1;

    if (phy_serdes->sfp_module_detect_gpio != -1)
    {
#ifndef CONFIG_BRCM_QEMU
        rc = kerSysGetGpioValue(phy_serdes->sfp_module_detect_gpio) == 0;
#else
        return 1;
#endif
    }
    else
        rc = phy_serdes_sfp_module_detected(phy_dev);

    /* Skip I2C detection if we are in SFP_MODULE_IN status */
    if (phy_serdes->sfp_status >= SFP_MODULE_IN)
        return rc;

#if defined(CONFIG_I2C)
    rc_i2c = phy_i2c_module_detect(phy_dev);

    if (rc & !rc_i2c)   /* If GPIO detected but I2C does not, wait for I2C driver */
        rc = 0;
#endif
    return rc + rc_i2c;
}

/*
   Module detection is not going through SGMII,
   so it can be done even under SGMII power down.
 */
static int dsl_sfp_module_detect(phy_dev_t *phy_dev)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;
    TRX_TYPE trx_type = TRX_TYPE_ETHERNET;
    phy_speed_t max_spd;
    phy_dev_t *phy_i2c = phy_dev->cascade_next;

    /* Don't do module detection for fixed connection desgin */
    if (phy_serdes->sfp_module_type == SFP_FIXED_PHY)
        return 1;

    if (dsl_sfp_module_detected(phy_dev) == 0)
    {
        if (phy_i2c->flag & PHY_FLAG_NOT_PRESENTED)
            return 0;

        phy_serdes->sfp_module_type = SFP_NO_MODULE;
        phy_i2c->flag |= PHY_FLAG_NOT_PRESENTED;
        printk("SFP Module at Address %d Core %d is Unplugged\n",
                phy_dev->addr, phy_serdes->core_num);
        dsl_serdes_sfp_lbe_op(phy_dev, LASER_OFF);
        phy_dev->link = 0;
        phy_i2c->name[0] = 0;

        return 0;
    }

    if (phy_serdes->sfp_module_type != SFP_NO_MODULE)   /* Module detection done and no unplug */
        return 1;

#if defined(CONFIG_I2C) && defined(CONFIG_BCM_OPTICALDET)
    msleep(300);    /* Let I2C driver prepare data */

    if (trx_get_type(phy_i2c->addr, &trx_type) == OPTICALDET_NOSFP)  /* I2C driver not sync with us yet */
        return 0;

    if (trx_type != TRX_TYPE_XPON)
        phy_i2c_module_type_detect(phy_dev);
#endif

    if (trx_type == TRX_TYPE_XPON)
        phy_serdes->sfp_module_type = SFP_GPON_MODULE;
    else
        phy_serdes->sfp_module_type = SFP_AE_MODULE;

    phy_i2c->flag &= ~PHY_FLAG_NOT_PRESENTED;
    max_spd = phy_max_speed_get(phy_i2c);

    if (phy_serdes->sfp_module_type == SFP_GPON_MODULE)
        printk("GPON Module ");
    else if (phy_i2c->flag & PHY_FLAG_COPPER_CONFIGURABLE_SFP_TYPE) // SGMII SFP_COPPER
        printk("%s SGMII Copper SFP Module ", phy_dev_speed_to_str(max_spd));
    else if (phy_i2c->flag & PHY_FLAG_COPPER_SFP_TYPE) //SFP_COPPER
        printk("%s Copper SFP Module ", phy_dev_speed_to_str(max_spd));
    else
        printk("SFP Module ");
    printk(KERN_CONT "is Plugged in at Serdes address %d core %d lane %d\n", phy_dev->addr,
        phy_dev->core_index, phy_dev->lane_index);

    return 1;
}

int phy_dsl_serdes_read_status(phy_dev_t *phy_dev)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;
    phy_dev_t *phy_next = phy_dev->cascade_next;
    phy_speed_t org_speed = phy_dev->speed;
    int org_link = phy_dev->link;


    if (!phy_serdes || !phy_serdes->inited)
        goto read_end;

    if (PhyIsPortConnectedToExternalSwitch(phy_dev) || PhyIsFixedConnection(phy_dev))
    {
        dsl_serdes_speed_detect(phy_dev);
        phy_serdes->link_stats(phy_dev);
        goto read_end;
    }

    if (phy_serdes->link_changed)   /* Force link temporary down to notify upper layer of event */
    {
        phy_dev->link = 0;
        phy_serdes->link_changed = 0;
        if (phy_serdes->sfp_status == SFP_LINK_UP)
        {
            phy_serdes->sfp_status = SFP_MODULE_IN;
            if (phy_serdes->config_speed == PHY_SPEED_AUTO)
                dsl_serdes_single_speed_set(phy_dev, phy_serdes->common_highest_speed, PHY_DUPLEX_FULL);
            else
                dsl_serdes_single_speed_set(phy_dev, phy_serdes->config_speed, PHY_DUPLEX_FULL);
        }
        if (phy_serdes->sfp_module_type == SFP_FIXED_PHY)
            goto read_end;
        else
            goto sfp_end;
    }

    if (phy_serdes->sfp_module_type == SFP_FIXED_PHY)
        goto read_end;

    if (!phy_serdes->power_admin_on)
    {
        phy_dev->link = 0;
        goto read_end;
    }

    switch (phy_serdes->sfp_status)
    {
        case SFP_MODULE_OUT:
sfp_module_out:
            if(phy_serdes->sfp_status == SFP_MODULE_OUT && dsl_sfp_module_detect(phy_dev))
                goto sfp_module_in;

            if ( phy_serdes->sfp_status == SFP_MODULE_IN)
            {
                phy_dsl_set_configured_types(phy_dev);
                set_common_speed_range(phy_dev);
            }

            phy_serdes->sfp_status = SFP_MODULE_OUT;
            goto sfp_end;

        case SFP_MODULE_IN:
sfp_module_in:
            if(phy_serdes->sfp_status >= SFP_MODULE_IN && !dsl_sfp_module_detect(phy_dev))
            {
                phy_serdes->sfp_status = SFP_MODULE_IN;
                goto sfp_module_out;
            }

            if(phy_serdes->sfp_module_type != SFP_AE_MODULE)    /* PON module */
            {
                phy_serdes->sfp_status = SFP_MODULE_IN;
                goto sfp_end;
            }

            dsl_powerup_serdes(phy_dev);

            if (dsl_serdes_silent_start_light_detected(phy_dev))
                dsl_serdes_sfp_lbe_op(phy_dev, LASER_ON);

            if(phy_serdes->sfp_status < SFP_MODULE_IN)
            {
                cascade_phy_set_common_inter_types(phy_dev);
                phy_dsl_set_configured_types(phy_dev);
                set_common_speed_range(phy_dev);
                phy_dev->current_inter_phy_type = phy_next->current_inter_phy_type =
                    sfp_phy_get_best_inter_phy_configure_type(phy_dev,
                        phy_dev->configured_inter_phy_types, phy_serdes->config_speed);
                phy_dev_speed_set(phy_next, phy_serdes->config_speed, PHY_DUPLEX_FULL);
                dsl_serdes_single_speed_set(phy_dev, phy_serdes->config_speed, PHY_DUPLEX_FULL);
            }

            if(phy_serdes->sfp_status <= SFP_MODULE_IN)
            {
                phy_serdes->sfp_status = SFP_MODULE_IN;

                if (dsl_serdes_silent_start_light_detected(phy_dev))
                    dsl_serdes_sfp_lbe_op(phy_dev, LASER_ON);

                /*
                    Even silent start does not detect valid signal ans laser might be off,
                    we still need to do speed detection because SS is speed dependant and
                    work in single direction.
                */
                if (phy_serdes->config_speed == PHY_SPEED_AUTO)
                    dsl_serdes_speed_detect(phy_dev);

                link_status_check(phy_dev);
                if (phy_dev->link)
                    goto sfp_link_up;
            }
            phy_serdes->sfp_status = SFP_MODULE_IN;
            goto sfp_end;

        case SFP_LINK_UP:
sfp_link_up:
            if(phy_serdes->sfp_status == SFP_LINK_UP)
            {
                if(!dsl_sfp_module_detect(phy_dev))
                {
                    phy_dev->link = 0;
                    goto sfp_module_out;
                }
                link_status_check(phy_dev);
                if (org_link && phy_dev->link && org_speed != phy_dev->speed)
                    phy_dev->link = 0;

                if(!phy_dev->link)
                {
                    /* If link goes down, we must put the link to the highest.
                        because link detection assume the phy is in highest speed when
                        link is down and will check link before doing different speed config */
                    if(phy_serdes->config_speed == PHY_SPEED_AUTO)
                    {
                        phy_dev->current_inter_phy_type = sfp_phy_get_best_inter_phy_configure_type(phy_dev,
                                phy_dev->configured_inter_phy_types, phy_serdes->common_highest_speed);
                        if (phy_dev->current_inter_phy_type != INTER_PHY_TYPE_UNKNOWN)
                            dsl_serdes_single_speed_set(phy_dev, phy_serdes->common_highest_speed, PHY_DUPLEX_FULL);
                    }
                    goto sfp_module_in;
                }
            }
            else
                phy_dev_status_reverse_propagate(phy_dev);

            phy_serdes->sfp_status = SFP_LINK_UP;
            goto sfp_end;
    }

sfp_end:
    if ((phy_serdes->power_mode == SERDES_BASIC_POWER_SAVING && phy_serdes->sfp_status == SFP_MODULE_OUT) ||
        (phy_serdes->power_mode == SERDES_ADVANCED_POWER_SAVING && phy_serdes->sfp_status != SFP_LINK_UP))
        dsl_powerdown_serdes(phy_dev);

read_end:
    return 0;
}

int phy_dsl_serdes_dt_priv(dt_handle_t handle, uint32_t addr, uint32_t phy_mode, void **_priv)
{
    *_priv = (void *)handle;

    return 0;
}

int phy_dsl_serdes_dev_add_lock(phy_dev_t *phy_dev)
{
    mutex_lock(&serdes_mutex);
    mutex_unlock(&serdes_mutex);
    return 0;
}

int phy_dsl_serdes_read_status_lock(phy_dev_t *phy_dev)
{
    int rc;

    mutex_lock(&serdes_mutex);

    rc = phy_dsl_serdes_read_status(phy_dev);

    mutex_unlock(&serdes_mutex);
    return rc;
}

int phy_dev_sltstt_get(phy_dev_t *phy_dev, int *mode)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;
    if (!dsl_serdes_silent_start_supported(phy_dev))
        return -1;
    *mode = phy_serdes->silent_start_enabled;
    return 0;
}
EXPORT_SYMBOL(phy_dev_sltstt_get);

int phy_dev_sltstt_set(phy_dev_t *phy_dev, int mode)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;
    if (!dsl_serdes_silent_start_supported(phy_dev))
        return -1;
    phy_serdes->silent_start_enabled = !!mode;

    if (phy_serdes->silent_start_enabled)
        dsl_serdes_sfp_lbe_op(phy_dev, LASER_OFF);
    else
        dsl_serdes_sfp_lbe_op(phy_dev, LASER_ON);
    return 0;
}
EXPORT_SYMBOL(phy_dev_sltstt_set);

int dsl_serdes_power_mode_set(phy_dev_t *phy_dev, int mode)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;

    if (phy_serdes->power_mode == mode)
        return 0;

    phy_serdes->power_mode = mode;

    switch (mode)
    {
        case SERDES_NO_POWER_SAVING:
            phy_dsl_serdes_power_set(phy_dev, 1);
            break;
        case SERDES_BASIC_POWER_SAVING:
        case SERDES_ADVANCED_POWER_SAVING:
            phy_dsl_serdes_power_set(phy_dev, 1);
            dsl_powerdown_serdes(phy_dev);
            break;
        case SERDES_FORCE_OFF:
            phy_dsl_serdes_power_set(phy_dev, 0);
            break;
    }

    return 0;
}

int dsl_serdes_apd_get(phy_dev_t *phy_dev, int *enable)
{
    dsl_serdes_power_mode_get(phy_dev, enable);
    return 0;
}

int dsl_serdes_apd_set_lock(phy_dev_t *phy_dev, int enable)
{
    mutex_lock(&serdes_mutex);

    dsl_serdes_power_mode_set(phy_dev, enable);

    mutex_unlock(&serdes_mutex);
    return 0;
}

int dsl_serdes_power_mode_get(phy_dev_t *phy_dev, int *mode)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;

    if (!phy_serdes || !phy_serdes->inited)
        return -1;

   *mode = phy_serdes->power_mode;
    return 0;
}

int dsl_serdes_speed_get_lock(phy_dev_t *phy_dev, phy_speed_t *speed, phy_duplex_t *duplex)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;

    if (!phy_serdes || !phy_serdes->inited)
        return -1;

    mutex_lock(&serdes_mutex);
    *speed = phy_serdes->config_speed;
    *duplex = PHY_DUPLEX_FULL;
    mutex_unlock(&serdes_mutex);
    return 0;
}

int dsl_serdes_inter_phy_types_get(phy_dev_t *phy_dev, inter_phy_types_dir_t if_dir, uint32_t *types)
{
    phy_serdes_t *serdes = phy_dev->priv;

    *types = serdes->inter_phy_types;
    return 0;
}

