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
#include "bcmsfp.h"
#include "trxbus.h"

DEFINE_MUTEX(i2c_mutex);

static int phy_i2c_speed_set_lock(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex);
static int phy_i2c_get_speed_caps(phy_dev_t *phy_dev, int caps_type, uint32_t *caps);
static int phy_i2c_power_get(phy_dev_t *phy_dev, int *enable)
{
    *enable = 1;
    return 0;
}

static char *i2c_get_phy_name(phy_dev_t *phy_dev)
{
    static char buf[64];
    phy_serdes_t *phy_serdes = phy_dev->priv;
    char *sfp_type;

    if (phy_serdes->sfp_module_type == SFP_GPON_MODULE)
        sfp_type = "GPON Module";
    else if (phy_dev->flag & PHY_FLAG_COPPER_CONFIGURABLE_SFP_TYPE)
        sfp_type = "SGMII SFP";
    else if (phy_dev->flag & (PHY_FLAG_COPPER_SFP_TYPE))
        sfp_type = "CopperSFP";
    else
        sfp_type = "SFP Module";
    sprintf(buf, "%s#%d", sfp_type, phy_dev->addr);
    return buf;
}

static int phy_i2c_config_speed_get(phy_dev_t *phy_dev, phy_speed_t *speed, phy_duplex_t *duplex);
phy_drv_t phy_drv_i2c_phy =
{
    .phy_type = PHY_TYPE_I2C_PHY,
    .speed_set = phy_i2c_speed_set_lock,
    .caps_get = phy_i2c_get_speed_caps,
    .config_speed_get = phy_i2c_config_speed_get,
    .power_get = phy_i2c_power_get,
    .get_phy_name = i2c_get_phy_name,
    .name = "I2C",
};

typedef struct phy_i2c_priv_s {
    phy_dsl_priv_m;
    int inited;
    int sgmii_mode;
    int an_restart_int;
    int sfp_mod_in;
} phy_i2c_priv_t;

static int phy_i2c_config_speed_get(phy_dev_t *phy_dev, phy_speed_t *speed, phy_duplex_t *duplex)
{
    phy_i2c_priv_t *phy_i2c_priv = phy_dev->priv;
    *speed = phy_i2c_priv->config_speed;
    *duplex = PHY_DUPLEX_FULL;
    return 0;
}

static int phy_i2c_module_detect_cb(int bus, void *opticaldet_desc)
{
    int rc = 0;
    phy_i2c_priv_t *phy_i2c_priv;
    phy_dev_t* phy_i2c = phy_dev_get_force(PHY_TYPE_I2C_PHY, bus);

    if (phy_i2c) {
        phy_i2c_priv = phy_i2c->priv;
        mutex_lock(&i2c_mutex);
        phy_i2c_priv->sfp_mod_in = 1;
        mutex_unlock(&i2c_mutex);
        rc = 1;
    }
	
    return rc;
}

static int phy_i2c_module_remove_cb(int bus)
{
    int rc = 0;
    phy_dev_t* phy_i2c = phy_dev_get_force(PHY_TYPE_I2C_PHY, bus);
    phy_i2c_priv_t *phy_i2c_priv;

    if (phy_i2c) {
        phy_i2c_priv = phy_i2c->priv;
        mutex_lock(&i2c_mutex);
        phy_i2c_priv->sfp_mod_in = 0;
        mutex_unlock(&i2c_mutex);
        rc = 1;
    }

    return rc;
}

static phy_i2c_priv_t phy_i2c_priv[MAX_SERDES_NUMBER];
int phy_drv_dsl_i2c_create_lock(phy_dev_t *phy_dev)
{
    int ret = 0;
    phy_dev_t *i2c_phy;
    int bus_num = -1;
    phy_drv_t *phy_drv;
    phy_serdes_t *phy_serdes;
    phy_i2c_priv_t *i2c_priv;
    static int phy_i2c_num = 0;

    mutex_lock(&i2c_mutex);

    ret += phy_driver_set(&phy_drv_i2c_phy);

    phy_drv = phy_dev->phy_drv; /* Parent PHY device */
    phy_serdes = phy_dev->priv;
#if defined(CONFIG_I2C) && defined(CONFIG_BCM_OPTICALDET)
    switch (phy_drv->phy_type)
    {
        case PHY_TYPE_138CLASS_SERDES:
        case PHY_TYPE_158CLASS_SERDES:
        case PHY_TYPE_146CLASS_SERDES:
        case PHY_TYPE_6756CLASS_SERDES:
            if ((bus_num = trxbus_dt_bus_get(phy_dev->dt_handle)) < 0) {
                printk("Error: ***** No I2C bus number defined for %s Serdes addr %d.\n",
                        phy_dev_speed_to_str(phy_serdes->highest_speed), phy_dev->addr);
                printk("              Wrong board dts definition or wrong board design.\n");
                BUG();
            }
            break;

        default:
            printk(" Unknown Serdes Type: %d\n", phy_drv->phy_type);
            BUG();
    }
#endif

    i2c_phy = phy_dev_add(PHY_TYPE_I2C_PHY, bus_num, 0);

    phy_dev->cascade_next = i2c_phy;
    i2c_phy->cascade_prev = phy_dev;
    i2c_phy->flag = PHY_FLAG_NOT_PRESENTED|PHY_FLAG_DYNAMIC;
    i2c_phy->bus_drv = bus_drv_get(BUS_TYPE_DSL_I2C);

    if (phy_i2c_num >= ARRAY_SIZE(phy_i2c_priv)) {
        printk(" ********* Error: Too many I2C PHYs defined\n");
        BUG();
    }
    i2c_priv = i2c_phy->priv = &phy_i2c_priv[phy_i2c_num++];
    i2c_priv->sgmii_mode = NONSGMII_MODE;

    mutex_unlock(&i2c_mutex);

    trxbus_register_mac(bus_num, 1, phy_i2c_module_detect_cb, phy_i2c_module_remove_cb);

    printk("Created I2C PHY driver at bus %d, for Serdes %s Core %d, address %d",
            bus_num, phy_drv->name, phy_serdes->core_num, phy_dev->addr);

    return ret;
}

#define AN_RESTART_SEC 5
static int phy_i2c_speed_set_lock(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex)
{
    phy_i2c_priv_t *phy_i2c_priv = phy_dev->priv;
    phy_serdes_t *phy_serdes = phy_dev->cascade_prev->priv;

    mutex_lock(&i2c_mutex);

    phy_i2c_priv->current_speed = speed;
    phy_i2c_priv->config_speed = phy_serdes->config_speed;

    if (!(phy_dev->flag & PHY_FLAG_COPPER_CONFIGURABLE_SFP_TYPE) ||
            (phy_dev->flag & PHY_FLAG_NOT_PRESENTED) ||
            speed > PHY_SPEED_1000)
        goto ret;

    /* Configure SFP PHY into SGMII mode */
    if (phy_i2c_priv->config_speed == PHY_SPEED_AUTO)
    {
        if (phy_i2c_priv->sgmii_mode == SGMII_AUTO)
        {
            if (phy_i2c_priv->an_restart_int++ == AN_RESTART_SEC)
            {
                phy_bus_write(phy_dev, MII_CONTROL, 0x1340); // Some module need to kick AN Restart
                phy_i2c_priv->an_restart_int = 0;
            }
            goto ret;   /* Let HW do job in auto mode */
        }

        if (phy_i2c_priv->sgmii_mode == NONSGMII_MODE)
        {
            phy_bus_write(phy_dev, 0x1b, 0x9084);    /* Enable SGMII mode */
            phy_bus_write(phy_dev, MII_CONTROL, 0x8000);
            phy_i2c_priv->sgmii_mode = SGMII_AUTO;
        }

        phy_bus_write(phy_dev, 0x9, 0x0f00);     /* Advertise 1kBase-T Full/Half-Duplex */
        phy_bus_write(phy_dev, 0x4, 0x0de1);     /* Adverstize 100/10Base-T Full/Half-Duplex */
        phy_i2c_priv->sgmii_mode = SGMII_AUTO;
        phy_dev->an_enabled = 1;
        phy_i2c_priv->inited = 1;
    }
    else
    {
        switch(speed)
        {
            case PHY_SPEED_1000:
                phy_bus_write(phy_dev, 0x9, 0x0f00);     /* Advertise 1kBase-T Full/Half-Duplex */
                phy_bus_write(phy_dev, 0x4, 0x0000);     /* Adverstize 100/10Base-T Full/Half-Duplex */
                break;
            case PHY_SPEED_100:
                phy_bus_write(phy_dev, 0x4, 0x0de1);     /* Adverstize 100/10Base-T Full/Half-Duplex */
                phy_bus_write(phy_dev, 0x9, 0x0000);
                break;
            default:
                break;
        }
        phy_i2c_priv->sgmii_mode = SGMII_FORCE;
        phy_dev->an_enabled = 1;
    }

    /* Do a final PHY reset to make configuration valid */
    phy_bus_write(phy_dev, MII_CONTROL, 0x1340);
    phy_bus_write(phy_dev, MII_CONTROL, 0x9140);

ret:
    mutex_unlock(&i2c_mutex);
    msleep(1);
    return 0;
}

static int phy_i2c_get_speed_caps(phy_dev_t *phy_dev, int caps_type, uint32_t *caps)
{
    uint32_t speed_caps = 0;
    phy_i2c_priv_t *phy_i2c_priv = phy_dev->priv;
    long val = 0;
    struct device *dev;

    if ((dev = trxbus_get_dev(phy_dev->addr)) == NULL)
        return -1;

    if (phy_i2c_priv->speed_caps == 0)
    {
        sfp_mon_read(dev, bcmsfp_mon_trx_compliance, 0, &val);
        if (val & BCM_SFF8472_CC_10GBASE_R)
            speed_caps |= PHY_CAP_10000|PHY_CAP_5000|PHY_CAP_2500|PHY_CAP_1000_FULL|PHY_CAP_100_FULL;

        if (val & BCM_SFF8472_CC_1GBASE_X)
            speed_caps |= PHY_CAP_5000|PHY_CAP_2500|PHY_CAP_1000_FULL|PHY_CAP_100_FULL;

        /* If no speed caps read out from SFP module,
           We set it to SFP module maximum ones */
        if (speed_caps == 0)
            speed_caps = PHY_CAP_10000|PHY_CAP_5000|PHY_CAP_2500|PHY_CAP_1000_FULL|PHY_CAP_100_FULL;

        phy_i2c_priv->speed_caps = speed_caps|PHY_CAP_AUTONEG;
    }

    *caps = phy_i2c_priv->speed_caps;
    return 0;
}

static int phy_i2c_writable(phy_dev_t *phy_i2c)
{
    uint16_t v16a, v16b, v16c;
    int rc = 0;

    phy_bus_read(phy_i2c, MII_CONTROL, &v16a);
    v16b = v16a ^ (1<<12);   /* flip bit 12 */
    phy_bus_write(phy_i2c, MII_CONTROL, v16b);
    phy_bus_read(phy_i2c, MII_CONTROL, &v16c);

    if (v16c == v16b)   /* Don't compare to v16a, since read-only one can read garbage sometimes */
        rc = 1;
    phy_bus_write(phy_i2c, MII_CONTROL, v16a);
    return rc;
}

#define I2C_LASER_ON_WAIT  250

int phy_i2c_module_detect(phy_dev_t *phy_dev)
{
    phy_dev_t *phy_i2c = phy_dev->cascade_next;
    phy_i2c_priv_t *phy_i2c_priv;
    int rc = 0;

    if (phy_i2c == NULL) return 0;

    phy_i2c_priv = phy_i2c->priv;
    mutex_lock(&i2c_mutex);
    rc = phy_i2c_priv->sfp_mod_in;
    mutex_unlock(&i2c_mutex);

    return rc;
}

void phy_i2c_module_type_detect(phy_dev_t *phy_dev)
{
    uint16_t v16;
    phy_dev_t *phy_i2c = phy_dev->cascade_next;
    phy_i2c_priv_t *phy_i2c_priv;

    if (phy_i2c == NULL) return;
    phy_i2c_priv = phy_i2c->priv;

    if (!(phy_i2c->flag & PHY_FLAG_NOT_PRESENTED)) /* I2C detected. Skip rescan if SFP module detected already */
        return;

    mutex_lock(&i2c_mutex);
    if (phy_i2c_priv->sfp_mod_in == 0) {
        mutex_unlock(&i2c_mutex);
        return;
    }

    /* 
        We have to turn on LBE here or SGMII SFP module using TX_DISABLE as
        PHY address access enabling signal will not respond to the 0xAC scanning 
    */
    dsl_serdes_sfp_lbe_op(phy_dev, LASER_ON); 
    msleep(I2C_LASER_ON_WAIT);
  
    /* If I2C read operation succeeds, I2C module is connected
       and which means it is a copper SFP module */
    phy_i2c->inter_phy_types = 0;
    phy_i2c_priv->speed_caps = 0;
    phy_i2c_priv->sgmii_mode = NONSGMII_MODE;
    phy_i2c_priv->inited = 0;
    phy_i2c_get_speed_caps(phy_i2c, CAPS_TYPE_SUPPORTED, &phy_i2c_priv->speed_caps);
    phy_i2c->flag &= ~(PHY_FLAG_COPPER_SFP_TYPE|PHY_FLAG_COPPER_CONFIGURABLE_SFP_TYPE);
    if (phy_bus_read(phy_i2c, MII_CONTROL, &v16) == 0 )
    {
        phy_i2c->flag |= PHY_FLAG_COPPER_SFP_TYPE;
        if (phy_i2c_writable(phy_i2c))
            phy_i2c->flag |= PHY_FLAG_COPPER_CONFIGURABLE_SFP_TYPE;
        phy_i2c->inter_phy_types = INTER_PHY_TYPE_SGMII_M;
    }

    /* For Writable SFP module, we only support SGMII */
    if (!(phy_i2c->flag & PHY_FLAG_COPPER_CONFIGURABLE_SFP_TYPE))
    {
        if (phy_i2c_priv->speed_caps & PHY_CAP_10000)
            phy_i2c->inter_phy_types |= INTER_PHY_TYPE_10GBASE_R_M;

        if (phy_i2c_priv->speed_caps & PHY_CAP_5000)
            phy_i2c->inter_phy_types |= INTER_PHY_TYPE_5GBASE_R_M|INTER_PHY_TYPE_5GBASE_X_M|INTER_PHY_TYPE_5000BASE_X_M;

        if (phy_i2c_priv->speed_caps & PHY_CAP_2500)
            phy_i2c->inter_phy_types |= INTER_PHY_TYPE_2P5GBASE_R_M|INTER_PHY_TYPE_2P5GBASE_X_M|INTER_PHY_TYPE_2500BASE_X_M;

        if (phy_i2c_priv->speed_caps & PHY_CAP_1000_FULL)
            phy_i2c->inter_phy_types |= INTER_PHY_TYPE_1000BASE_X_M;

        if (phy_i2c_priv->speed_caps & PHY_CAP_100_FULL)
            phy_i2c->inter_phy_types |= INTER_PHY_TYPE_100BASE_FX_M;

        if (phy_i2c->inter_phy_types == 0)
            phy_i2c->inter_phy_types = INTER_PHY_TYPE_FULLTYPES_M;

        phy_i2c->inter_phy_types |= INTER_PHY_TYPE_MULTI_SPEED_AN_MASK_M;
    }

    if (!(phy_i2c->flag & PHY_FLAG_COPPER_SFP_TYPE) && dsl_serdes_silent_start_light_detected(phy_dev))
        dsl_serdes_sfp_lbe_op(phy_dev, LASER_OFF);

    mutex_unlock(&i2c_mutex);
}

