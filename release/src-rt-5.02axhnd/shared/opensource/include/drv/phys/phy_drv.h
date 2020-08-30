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
 *  Created on: Dec 2015
 *      Author: yuval.raviv@broadcom.com
 */

#ifndef __PHY_DRV_H__
#define __PHY_DRV_H__

#include "bus_drv.h"

#define PHY_CAP_10_HALF         (1 << 0)
#define PHY_CAP_10_FULL         (1 << 1)
#define PHY_CAP_100_HALF        (1 << 2)
#define PHY_CAP_100_FULL        (1 << 3)
#define PHY_CAP_1000_HALF       (1 << 4)
#define PHY_CAP_1000_FULL       (1 << 5)
#define PHY_CAP_2500            (1 << 6)
#define PHY_CAP_5000            (1 << 7)
#define PHY_CAP_10000           (1 << 8)
#define PHY_CAP_AUTONEG         (1 << 9)
#define PHY_CAP_PAUSE           (1 << 10)
#define PHY_CAP_PAUSE_ASYM      (1 << 11)
#define PHY_CAP_REPEATER        (1 << 12)

typedef enum
{
    PHY_SPEED_UNKNOWN,
    PHY_SPEED_AUTO = PHY_SPEED_UNKNOWN,
    PHY_SPEED_10,
    PHY_SPEED_100,
    PHY_SPEED_1000,
    PHY_SPEED_2500,
    PHY_SPEED_5000,
    PHY_SPEED_10000,
} phy_speed_t;

typedef enum
{
    PHY_DUPLEX_UNKNOWN,
    PHY_DUPLEX_HALF,
    PHY_DUPLEX_FULL,
} phy_duplex_t;

typedef enum
{
    PHY_MII_TYPE_UNKNOWN,
    PHY_MII_TYPE_MII,
    PHY_MII_TYPE_TMII,
    PHY_MII_TYPE_GMII,
    PHY_MII_TYPE_RGMII,
    PHY_MII_TYPE_SGMII,
    PHY_MII_TYPE_HSGMII,
    PHY_MII_TYPE_XFI,
    PHY_MII_TYPE_SERDES,
} phy_mii_type_t;

typedef enum
{
    PHY_TYPE_UNKNOWN,
    PHY_TYPE_6848_EPHY,
    PHY_TYPE_6848_EGPHY,
    PHY_TYPE_6848_SGMII,
    PHY_TYPE_PCS,
    PHY_TYPE_6858_EGPHY,
    PHY_TYPE_6846_EGPHY,
    PHY_TYPE_6856_SGMII,
    PHY_TYPE_EXT1,
    PHY_TYPE_EXT2,
    PHY_TYPE_EXT3,
    PHY_TYPE_LPORT_SERDES,
    PHY_TYPE_53125,
    PHY_TYPE_PON,
    PHY_TYPE_SF2_GPHY,
    PHY_TYPE_SF2_CL45_PHY,
    PHY_TYPE_SF2_SERDES,
    PHY_TYPE_I2C_PHY,
    PHY_TYPE_XGAE,
    PHY_TYPE_CROSSBAR,
    PHY_TYPE_MAC2MAC,
    PHY_TYPE_MAX,
} phy_type_t;

typedef void (*link_change_cb_t)(void *ctx);

/* Phy device */
typedef struct phy_dev_s
{
    struct phy_drv_s *phy_drv;
    phy_mii_type_t mii_type;
    link_change_cb_t link_change_cb;
    void *link_change_ctx;
    uint32_t addr;          // contains phy address only
    uint32_t meta_id;       // contains extra phyId info from board param
    void *priv;
    int link;
    phy_speed_t speed;
    phy_duplex_t duplex;
    int pause_rx;
    int pause_tx;
    int delay_rx;
    int delay_tx;
    int disable_hd;
    int swap_pair;
    int flag;
#ifdef EXT_BCM53134
    uint16_t resetGpio;
#endif
#ifdef EXT_BCM84880
    int led_mode;
#endif
    
    /* For cascaded PHY */
    void *sw_port;
    struct phy_dev_s *cascade_next;
    struct phy_dev_s *cascade_prev;
} phy_dev_t;

#define PHY_FLAG_NOT_PRESENTED      (1<<0)      /* for SFP module indicating not inserted */
#define PHY_FLAG_POWER_SET_ENABLED  (1<<1)
#define PHY_FLAG_DYNAMIC            (1<<2)

#define CAPS_TYPE_ADVERTISE      0
#define CAPS_TYPE_SUPPORTED      1
#define CAPS_TYPE_LP_ADVERTISED  2

/* Phy driver */
typedef struct phy_drv_s
{
    phy_type_t phy_type;
    char *name;
    int initialized;
    bus_drv_t *bus_drv;
    int (*read)(phy_dev_t *phy_dev, uint16_t reg, uint16_t *val);
    int (*write)(phy_dev_t *phy_dev, uint16_t reg, uint16_t val);
    int (*power_get)(phy_dev_t *phy_dev, int *enable);
    int (*power_set)(phy_dev_t *phy_dev, int enable);
    int (*apd_get)(phy_dev_t *phy_dev, int *enable);
    int (*apd_set)(phy_dev_t *phy_dev, int enable);
    int (*eee_get)(phy_dev_t *phy_dev, int *enable);
    int (*eee_set)(phy_dev_t *phy_dev, int enable);
    int (*eee_resolution_get)(phy_dev_t *phy_dev, int *enable);
    int (*read_status)(phy_dev_t *phy_dev);
    int (*speed_set)(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex);
    int (*config_speed_get)(phy_dev_t *phy_dev, phy_speed_t *speed, phy_duplex_t *duplex);
    int (*caps_get)(phy_dev_t *phy_dev, int caps_type, uint32_t *caps);
    int (*caps_set)(phy_dev_t *phy_dev, uint32_t caps);
    int (*phyid_get)(phy_dev_t *phy_dev, uint32_t *phyid);
    int (*init)(phy_dev_t *phy_dev);
    int (*link_change_register)(phy_dev_t *phy_dev);
    int (*link_change_unregister)(phy_dev_t *phy_dev);
    int (*dev_add)(phy_dev_t *phy_dev);
    int (*dev_del)(phy_dev_t *phy_dev);
    int (*drv_init)(struct phy_drv_s *phy_drv);
    int (*isolate_phy)(phy_dev_t *phy_dev, int isolate);
    int (*pair_swap_set)(phy_dev_t *phy_dev, int enable);
} phy_drv_t;

typedef struct
{
    char *desc;
    uint16_t reg;
    uint16_t val;
} prog_entry_t;

phy_dev_t *phy_dev_get(phy_type_t phy_type, uint32_t addr);
phy_dev_t *phy_dev_add(phy_type_t phy_type, uint32_t addr, void *priv);
int phy_dev_del(phy_dev_t *phy_dev);
int phy_isolate_phy(phy_dev_t *phy, int isolate);
phy_dev_t *phy_dev_get_by_i2c(int bus_num);
int phy_dev_get_uid(phy_dev_t *phy);
phy_dev_t *phy_dev_get_by_uid(int uid);

int phy_drivers_set(void);
int phy_drivers_init(void);
int phy_driver_set(phy_drv_t *phy_drv);
int phy_driver_init(phy_type_t phy_type);

char *phy_dev_mii_type_to_str(phy_mii_type_t mii_type);
char *phy_dev_speed_to_str(phy_speed_t speed);
char *phy_dev_duplex_to_str(phy_duplex_t duplex);

void phy_dev_print_status(phy_dev_t *phy_dev);
void phy_dev_prog(phy_dev_t *phy_dev, prog_entry_t *prog_entry);

void phy_dev_link_change_register(phy_dev_t *phy_dev, link_change_cb_t cb, void *ctx);
void phy_dev_link_change_unregister(phy_dev_t *phy_dev);
void phy_dev_link_change_notify(phy_dev_t *phy_dev);

typedef void (*phy_dev_work_func_t)(phy_dev_t *phy_dev);
int phy_dev_queue_work(phy_dev_t *phy_dev, phy_dev_work_func_t func);

#define is_cascade_phy(phy) (phy->cascade_prev || phy->cascade_next)
#define cascade_phy_get_next(phy)   ((phy->cascade_next && !(phy->cascade_next->flag & PHY_FLAG_NOT_PRESENTED))? phy->cascade_next : NULL)

static inline phy_dev_t *cascade_phy_get_first(phy_dev_t *phy_dev)
{
    phy_dev_t *phy;

    for(phy=phy_dev; phy->cascade_prev; phy=phy->cascade_prev);
    return phy;
}

static inline phy_dev_t *cascade_phy_get_last(phy_dev_t *phy_dev)
{
    phy_dev_t *phy;

    if (!phy_dev) return NULL;
    for(phy=phy_dev;
        phy->cascade_next && !(phy->cascade_next->flag & PHY_FLAG_NOT_PRESENTED);
        phy=phy->cascade_next);
    return phy;
}

static inline int phy_bus_read(phy_dev_t *phy_dev, uint16_t reg, uint16_t *val)
{
    if (!phy_dev->phy_drv->bus_drv)
        return 0;

    return phy_dev->phy_drv->bus_drv->c22_read(phy_dev->addr, reg, val);
}

static inline int phy_bus_write(phy_dev_t *phy_dev, uint16_t reg, uint16_t val)
{
    if (!phy_dev->phy_drv->bus_drv)
        return 0;

    return phy_dev->phy_drv->bus_drv->c22_write(phy_dev->addr, reg, val);
}

static inline int phy_bus_c45_read(phy_dev_t *phy_dev, uint16_t dev, uint16_t reg, uint16_t *val)
{
    if (!phy_dev->phy_drv->bus_drv)
        return 0;

    return phy_dev->phy_drv->bus_drv->c45_read(phy_dev->addr, dev, reg, val);
}

static inline int phy_bus_c45_write(phy_dev_t *phy_dev, uint16_t dev, uint16_t reg, uint16_t val)
{
    if (!phy_dev->phy_drv->bus_drv)
        return 0;

    return phy_dev->phy_drv->bus_drv->c45_write(phy_dev->addr, dev, reg, val);
}

static inline int phy_dev_read(phy_dev_t *phy_dev, uint16_t reg, uint16_t *val)
{
    if (phy_dev->phy_drv->read)
        return phy_dev->phy_drv->read(phy_dev, reg, val);
    else
        return phy_bus_read(phy_dev, reg, val);
}

static inline int phy_dev_write(phy_dev_t *phy_dev, uint16_t reg, uint16_t val)
{
    if (phy_dev->phy_drv->write)
        return phy_dev->phy_drv->write(phy_dev, reg, val);
    else
        return phy_bus_write(phy_dev, reg, val);
}

static inline int phy_dev_isolate_phy(phy_dev_t *phy_dev, int isolate)
{
    if (!phy_dev->phy_drv->isolate_phy)
        return 0;

    return phy_dev->phy_drv->isolate_phy(phy_dev, isolate);
}

static inline int phy_dev_pair_swap_set(phy_dev_t *phy_dev, int enable)
{
    if (!phy_dev->phy_drv->pair_swap_set)
        return 0;

    return phy_dev->phy_drv->pair_swap_set(phy_dev, enable);
}

static inline int phy_dev_power_get(phy_dev_t *phy_dev, int *enable)
{
    *enable = 0;

    if (!phy_dev->phy_drv->power_get)
        return 0;

    return phy_dev->phy_drv->power_get(phy_dev, enable);
}

static inline int phy_dev_power_set(phy_dev_t *phy_dev, int enable)
{
    if (is_cascade_phy(phy_dev))
    {
        phy_dev_t *cascade;
        for (cascade = cascade_phy_get_first(phy_dev); cascade; cascade = cascade_phy_get_next(cascade))
        {
            //printk("phy %s:%d power_set=%d\n", (phy_dev->phy_drv) ? cascade->phy_drv->name : NULL, cascade->addr, enable);
            if (enable)
                cascade->flag |= PHY_FLAG_POWER_SET_ENABLED;
            else
                cascade->flag &= ~PHY_FLAG_POWER_SET_ENABLED;
                
            if (cascade->phy_drv->power_set)
                cascade->phy_drv->power_set(cascade, enable);
        }
        return 0;
    }
    //printk("phy %s:%d power_set=%d\n", (phy_dev->phy_drv) ? phy_dev->phy_drv->name : NULL, phy_dev->addr, enable);
    if (enable)
        phy_dev->flag |= PHY_FLAG_POWER_SET_ENABLED;
    else
        phy_dev->flag &= ~PHY_FLAG_POWER_SET_ENABLED;
        
    if (!phy_dev->phy_drv->power_set)
        return 0;

    return phy_dev->phy_drv->power_set(phy_dev, enable);
}

static inline int phy_dev_apd_get(phy_dev_t *phy_dev, int *enable)
{
    *enable = 0;

    if (!phy_dev->phy_drv->apd_get)
        return 0;

    return phy_dev->phy_drv->apd_get(phy_dev, enable);
}

static inline int phy_dev_apd_set(phy_dev_t *phy_dev, int enable)
{
    if (!phy_dev->phy_drv->apd_set)
        return 0;

    return phy_dev->phy_drv->apd_set(phy_dev, enable);
}

static inline int phy_dev_eee_get(phy_dev_t *phy_dev, int *enable)
{
    *enable = 0;

    if (!phy_dev->phy_drv->eee_get)
        return 0;

    return phy_dev->phy_drv->eee_get(phy_dev, enable);
}

static inline int phy_dev_eee_set(phy_dev_t *phy_dev, int enable)
{
    if (!phy_dev->phy_drv->eee_set)
        return 0;

    return phy_dev->phy_drv->eee_set(phy_dev, enable);
}

static inline int phy_dev_eee_resolution_get(phy_dev_t *phy_dev, int *enable)
{
    *enable = 0;

    if (!phy_dev->phy_drv->eee_resolution_get)
        return 0;

    return phy_dev->phy_drv->eee_resolution_get(phy_dev, enable);
}

static inline int phy_dev_read_status(phy_dev_t *phy_dev)
{
    int ret = 0;
#if !defined(DSL_RUNNER_DEVICE)
    phy_speed_t speed = phy_dev->speed;
#endif

    if (!phy_dev->phy_drv->read_status)
        goto Exit;

    if ((ret = phy_dev->phy_drv->read_status(phy_dev)))
        goto Exit;

#if !defined(DSL_RUNNER_DEVICE) /* DSL product has revert chain direction due to dyanmic module support */
    if (phy_dev->speed == speed)
        goto Exit;

    if (!phy_dev->cascade_next)
        goto Exit;

    if (!phy_dev->cascade_next->phy_drv->speed_set)
        goto Exit;

    if ((ret = phy_dev->cascade_next->phy_drv->speed_set(phy_dev->cascade_next, phy_dev->speed, phy_dev->duplex)))
        goto Exit;
#endif

#if defined(DSL_RUNNER_DEVICE)
    // administratively force link down if ethernet phy is not in power enable state
    if (phy_dev->phy_drv->phy_type != PHY_TYPE_PON &&
        !(phy_dev->flag & PHY_FLAG_POWER_SET_ENABLED)) {
        phy_dev->link = 0;
    }
#endif
Exit:
    return ret;
}

static inline int phy_dev_config_speed_get(phy_dev_t *phy_dev, phy_speed_t *speed, phy_duplex_t *duplex)
{
    if (!phy_dev->phy_drv->config_speed_get)
        return 0;

    return phy_dev->phy_drv->config_speed_get(phy_dev, speed, duplex);
}

static inline int phy_dev_speed_set(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex)
{
    if (!phy_dev->phy_drv->speed_set)
        return 0;

    return phy_dev->phy_drv->speed_set(phy_dev, speed, duplex);
}

static inline void phy_dev_status_propagate(phy_dev_t *end_phy)
{
    phy_dev_t *phy_dev;

    for (phy_dev = end_phy->cascade_prev; phy_dev; phy_dev = phy_dev->cascade_prev)
    {
        phy_dev_speed_set(phy_dev, end_phy->speed, end_phy->duplex);
        phy_dev->link = end_phy->link;
        phy_dev->speed = end_phy->speed;
        phy_dev->duplex = end_phy->duplex;
    }
}

static inline int phy_dev_caps_get(phy_dev_t *phy_dev, int caps_type, uint32_t *caps)
{
    *caps = 0;

    if (!phy_dev->phy_drv->caps_get)
        return 0;

    return phy_dev->phy_drv->caps_get(phy_dev, caps_type, caps);
}

static inline int phy_dev_caps_set(phy_dev_t *phy_dev, uint32_t caps)
{
    if (!phy_dev->phy_drv->caps_set)
        return 0;

    return phy_dev->phy_drv->caps_set(phy_dev, caps);
}

static inline phy_speed_t phy_caps_to_max_speed(uint32_t caps)
{
    int i;
    static int speed[] = {PHY_CAP_10000, PHY_SPEED_10000, PHY_CAP_5000, PHY_SPEED_5000, PHY_CAP_2500, PHY_SPEED_2500,
        PHY_CAP_1000_FULL, PHY_SPEED_1000, PHY_CAP_1000_HALF, PHY_SPEED_1000,
        PHY_CAP_100_FULL, PHY_SPEED_100, PHY_CAP_100_HALF, PHY_SPEED_100,
        PHY_CAP_10_FULL, PHY_SPEED_10, PHY_CAP_10_HALF, PHY_SPEED_10};

    for (i=0; i<sizeof(speed)/sizeof(speed[0]); i+=2) {
        if (caps & speed[i])
            return speed[i+1];
    }
    return PHY_SPEED_UNKNOWN;
}

static inline uint32_t phy_speed_to_caps(phy_speed_t speed, phy_duplex_t duplex)
{
    static uint32_t caps[] = {PHY_CAP_AUTONEG, PHY_CAP_10_FULL, PHY_CAP_100_FULL, 
        PHY_CAP_1000_FULL, PHY_CAP_2500, PHY_CAP_5000, PHY_CAP_10000};
    uint32_t cap;

    cap = caps[speed];
    if (speed < PHY_SPEED_1000 && duplex != PHY_DUPLEX_FULL)
        cap >>= 1;
    return cap;
}

static inline int phy_dev_phyid_get(phy_dev_t *phy_dev, uint32_t *phyid)
{
    *phyid = 0;

    if (!phy_dev->phy_drv->phyid_get)
        return 0;

    return phy_dev->phy_drv->phyid_get(phy_dev, phyid);
}

static inline int phy_dev_init(phy_dev_t *first_phy)
{
    int rc = 0;
    phy_dev_t *phy_dev;

    for (phy_dev = cascade_phy_get_first(first_phy); phy_dev; phy_dev = phy_dev->cascade_next)
    {
        phy_dev->link = 0;
        phy_dev->speed = PHY_SPEED_UNKNOWN;
        phy_dev->duplex = PHY_DUPLEX_UNKNOWN;

        if (phy_dev->phy_drv->init != NULL)
            rc |= phy_dev->phy_drv->init(phy_dev);

        if (phy_dev->phy_drv->phy_type != PHY_TYPE_CROSSBAR)
        {
            if (phy_dev->phy_drv->phy_type != PHY_TYPE_SF2_SERDES &&
                phy_dev->phy_drv->phy_type != PHY_TYPE_SF2_CL45_PHY)
                rc |= phy_dev_apd_set(phy_dev, 1);

            rc |= phy_dev_eee_set(phy_dev, 1);
        }
    }

    return rc;
}

static inline int cascade_phy_dev_isolate_phy(phy_dev_t *phy_dev, int isolate)
{
    if (is_cascade_phy(phy_dev))
    {
        int rc = 0;
        phy_dev_t *cascade;
        for (cascade = cascade_phy_get_first(phy_dev); cascade; cascade = cascade_phy_get_next(cascade))
            rc |= phy_dev_isolate_phy(cascade, isolate);
        return rc;
    }
    return phy_dev_isolate_phy(phy_dev, isolate);
}

static inline int cascade_phy_dev_apd_set(phy_dev_t *phy_dev, int enable)
{
    if (is_cascade_phy(phy_dev))
    {
        int rc = 0;
        phy_dev_t *cascade;
        for (cascade = cascade_phy_get_first(phy_dev); cascade; cascade = cascade_phy_get_next(cascade))
            rc |= phy_dev_apd_set(cascade, enable);
        return rc;
    }
    return phy_dev_apd_set(phy_dev, enable);
}

static inline int cascade_phy_dev_eee_set(phy_dev_t *phy_dev, int enable)
{
    if (is_cascade_phy(phy_dev))
    {
        int rc = 0;
        phy_dev_t *cascade;
        for (cascade = cascade_phy_get_first(phy_dev); cascade; cascade = cascade_phy_get_next(cascade))
            rc |= phy_dev_eee_set(cascade, enable);
        return rc;
    }
    return phy_dev_eee_set(phy_dev, enable);
}

static inline int cascade_phy_dev_speed_set(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex)
{
    if (is_cascade_phy(phy_dev))
    {
        int rc = 0;
        phy_dev_t *cascade;
        for (cascade = cascade_phy_get_first(phy_dev); cascade; cascade = cascade_phy_get_next(cascade))
        {
            rc |= phy_dev_speed_set(cascade, speed, duplex);
        }
        return rc;
    }
    return phy_dev_speed_set(phy_dev, speed, duplex);
}

// find minimum caps 
static inline int cascade_phy_dev_caps_get(phy_dev_t *phy_dev, int caps_type, uint32_t *caps)
{
    if (is_cascade_phy(phy_dev))
    {
        int rc = 0;
        phy_dev_t *cascade;
        for (cascade = cascade_phy_get_first(phy_dev), *caps = 0; cascade; cascade = cascade_phy_get_next(cascade))
        {
            uint32_t cascade_caps;
            if (cascade->phy_drv->caps_get)
            {
                rc |= cascade->phy_drv->caps_get(cascade, caps_type, &cascade_caps);
                if (cascade_caps) *caps = (*caps) ? *caps & cascade_caps : cascade_caps;
            }
        }
        return rc;
    }
    return phy_dev_caps_get(phy_dev, caps_type, caps);
}

static inline int cascade_phy_dev_caps_set(phy_dev_t *phy_dev, uint32_t caps)
{
    if (is_cascade_phy(phy_dev))
    {
        int rc = 0;
        phy_dev_t *cascade;
        for (cascade = cascade_phy_get_first(phy_dev); cascade; cascade = cascade_phy_get_next(cascade))
            rc |= phy_dev_caps_set(cascade, caps);
        return rc;
    }
    return phy_dev_caps_set(phy_dev, caps);
}

static inline int cascade_phy_dev_power_set(phy_dev_t *phy_dev, int enable)
{
    // current phy_dev_power_set() already handle cascade
    return phy_dev_power_set(phy_dev, enable);
}

/* Get last non dynamic PHY */
static inline phy_dev_t *cascade_phy_get_last_active(phy_dev_t *phy_dev)
{
    phy_dev_t *phy = cascade_phy_get_last(phy_dev);
    if (phy && (phy->flag & PHY_FLAG_DYNAMIC))
        phy = phy->cascade_prev;
    return phy;
}

static inline void _phy_register_polling_timer(phy_dev_t *phy, link_change_cb_t cb, int _register)
{
    phy_dev_t *end_phy = cascade_phy_get_last_active(phy);
    if (_register)
        phy_dev_link_change_register(end_phy, cb, end_phy);
    else
        phy_dev_link_change_unregister(end_phy);
}
#define phy_register_polling_timer(phy, cb) _phy_register_polling_timer(phy, cb, 1)
#define phy_unregister_polling_timer(phy) _phy_register_polling_timer(phy, 0, 0)

static inline int phy_drv_dev_add(phy_dev_t *phy_dev)
{
    if (phy_dev->phy_drv->initialized)
        return 0;

    if (!phy_dev->phy_drv->dev_add)
        return 0;

    return phy_dev->phy_drv->dev_add(phy_dev);
}

static inline int phy_drv_dev_del(phy_dev_t *phy_dev)
{
    if (phy_dev->phy_drv->initialized)
        return 0;

    if (!phy_dev->phy_drv->dev_del)
        return 0;

    return phy_dev->phy_drv->dev_del(phy_dev);
}

static inline int phy_drv_init(phy_drv_t *phy_drv)
{
    if (phy_drv->initialized)
        return 0;

    if (!phy_drv->drv_init)
        return 0;

    return phy_drv->drv_init(phy_drv);
}

#endif
