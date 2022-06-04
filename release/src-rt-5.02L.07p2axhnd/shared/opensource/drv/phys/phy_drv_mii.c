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

#include "bus_drv.h"
#include "phy_drv.h"
#include "phy_drv_mii.h"

int mii_phyid_get(phy_dev_t *phy_dev, uint32_t *phyid)
{
    int ret;
    uint16_t phyid1, phyid2;

    if ((ret = phy_bus_read(phy_dev, MII_PHYSID1, &phyid1)))
        goto Exit;

    if ((ret = phy_bus_read(phy_dev, MII_PHYSID2, &phyid2)))
        goto Exit;

    *phyid = phyid1 << 16 | phyid2;

Exit:
    return ret;
}

int mii_autoneg_restart(phy_dev_t *phy_dev)
{
    uint16_t val;
    int ret;

    if ((ret = phy_bus_read(phy_dev, MII_BMCR, &val)))
        goto Exit;

    if ((ret = phy_bus_write(phy_dev, MII_BMCR, val | BMCR_ANRESTART)))
        goto Exit;

Exit:
    return ret;
}

int mii_power_get(phy_dev_t *phy_dev, int *enable)
{
    uint16_t val;
    int ret;

    if ((ret = phy_bus_read(phy_dev, MII_BMCR, &val)))
        goto Exit;

    *enable = (val & BMCR_POWERDOWN) ? 0 : 1;

Exit:
    return ret;
}

int mii_power_set(phy_dev_t *phy_dev, int enable)
{
    uint16_t val;
    int ret;

    if ((ret = phy_bus_read(phy_dev, MII_BMCR, &val)))
        goto Exit;

    if (!enable || (val & BMCR_POWERDOWN))
        val |= BMCR_ANRESTART;

    if (enable)
        val &= ~BMCR_POWERDOWN;
    else
        val |= BMCR_POWERDOWN;

    if ((ret = phy_bus_write(phy_dev, MII_BMCR, val))) 
        goto Exit;

Exit:
    return ret;
}

int mii_caps_get(phy_dev_t *phy_dev, int caps_type,  uint32_t *pcaps)
{
    int ret;
    uint32_t caps = 0;

    if (caps_type == CAPS_TYPE_ADVERTISE)
    {
        int speed = 0;
        uint16_t bmcr, ctrl1000, adv;

        if ((ret = phy_bus_read(phy_dev, MII_BMCR, &bmcr)))
            goto Exit;

        if ((ret = phy_bus_read(phy_dev, MII_CTRL1000, &ctrl1000)))
            goto Exit;

        if ((ret = phy_bus_read(phy_dev, MII_ADVERTISE, &adv)))
            goto Exit;

        if (bmcr & BMCR_ANENABLE)
        {
            caps |= PHY_CAP_AUTONEG;

            if (adv & ADVERTISE_PAUSE_CAP)      caps |= PHY_CAP_PAUSE;
            if (adv & ADVERTISE_PAUSE_ASYM)     caps |= PHY_CAP_PAUSE_ASYM;
            if (adv & ADVERTISE_10HALF)         caps |= PHY_CAP_10_HALF;
            if (adv & ADVERTISE_10FULL)         caps |= PHY_CAP_10_FULL;
            if (adv & ADVERTISE_100HALF)        caps |= PHY_CAP_100_HALF;
            if (adv & ADVERTISE_100FULL)        caps |= PHY_CAP_100_FULL;
            if (ctrl1000 & ADVERTISE_1000HALF)  caps |= PHY_CAP_1000_HALF;
            if (ctrl1000 & ADVERTISE_1000FULL)  caps |= PHY_CAP_1000_FULL;
            if (ctrl1000 & ADVERTISE_REPEATER)  caps |= PHY_CAP_REPEATER;
        }
        else
        {
            speed = (bmcr & BMCR_SPEED100) >> 13 | (bmcr & BMCR_SPEED1000) >> 5;

            if (speed == 0) caps |= bmcr & BMCR_FULLDPLX ? PHY_CAP_10_FULL : PHY_CAP_10_HALF;
            if (speed == 1) caps |= bmcr & BMCR_FULLDPLX ? PHY_CAP_100_FULL : PHY_CAP_100_HALF;
            if (speed == 2) caps |= bmcr & BMCR_FULLDPLX ? PHY_CAP_1000_FULL : PHY_CAP_1000_HALF;
        }
        
    }
    else if (caps_type == CAPS_TYPE_SUPPORTED)
    {
        uint16_t estatus = 0;
        uint16_t bmsr    = 0;

        if ((ret = phy_bus_read(phy_dev, MII_BMSR, &bmsr)))
            goto Exit;

        if ((phy_dev->mii_type != PHY_MII_TYPE_MII)
            && (phy_dev->mii_type != PHY_MII_TYPE_TMII))
        {
            if ((ret = phy_bus_read(phy_dev, MII_ESTATUS, &estatus)))
                goto Exit;
        }
        
        if (bmsr & BMSR_ANEGCAPABLE)        caps |= PHY_CAP_AUTONEG;
        if (bmsr & BMSR_10HALF)             caps |= PHY_CAP_10_HALF;
        if (bmsr & BMSR_10FULL)             caps |= PHY_CAP_10_FULL;
        if (bmsr & BMSR_100HALF)            caps |= PHY_CAP_100_HALF;
        if (bmsr & BMSR_100FULL)            caps |= PHY_CAP_100_FULL;
        if (estatus & ESTATUS_1000_THALF)   caps |=  PHY_CAP_1000_HALF;
        if (estatus & ESTATUS_1000_TFULL)   caps |=  PHY_CAP_1000_FULL;

        if (phy_dev->disable_hd)
            caps &= ~(PHY_CAP_10_HALF | PHY_CAP_100_HALF | PHY_CAP_1000_HALF);
    }
    else if (caps_type == CAPS_TYPE_LP_ADVERTISED)
    {
        uint16_t stat1000, lpa;

        if ((ret = phy_bus_read(phy_dev, MII_STAT1000, &stat1000)))
            goto Exit;

        if ((ret = phy_bus_read(phy_dev, MII_LPA, &lpa)))
            goto Exit;

        if (lpa & LPA_LPACK)
        {
            caps |= PHY_CAP_AUTONEG;
            if (lpa & LPA_PAUSE_CAP)        caps |= PHY_CAP_PAUSE;
            if (lpa & LPA_PAUSE_ASYM)       caps |= PHY_CAP_PAUSE_ASYM;
            if (lpa & LPA_10HALF)           caps |= PHY_CAP_10_HALF;
            if (lpa & LPA_10FULL)           caps |= PHY_CAP_10_FULL;
            if (lpa & LPA_100HALF)          caps |= PHY_CAP_100_HALF;
            if (lpa & LPA_100FULL)          caps |= PHY_CAP_100_FULL;
            if (stat1000 & LPA_1000HALF)    caps |= PHY_CAP_1000_HALF;
            if (stat1000 & LPA_1000FULL)    caps |= PHY_CAP_1000_FULL;
        }
    }

    *pcaps = caps;

Exit:
    return ret;
}

int mii_caps_set(phy_dev_t *phy_dev, uint32_t caps)
{
    int ret;
    uint16_t bmcr, ctrl1000, adv;

    if ((ret = phy_bus_read(phy_dev, MII_BMCR, &bmcr)))
        goto Exit;

    if ((ret = phy_bus_read(phy_dev, MII_CTRL1000, &ctrl1000)))
        goto Exit;

    if ((ret = phy_bus_read(phy_dev, MII_ADVERTISE, &adv)))
        goto Exit;

    bmcr &= ~(BMCR_ANENABLE | BMCR_FULLDPLX | BMCR_SPEED100 | BMCR_SPEED1000);
    ctrl1000 &= ~(ADVERTISE_1000HALF | ADVERTISE_1000FULL | ADVERTISE_REPEATER);
    adv &= ~(ADVERTISE_PAUSE_CAP | ADVERTISE_PAUSE_ASYM | ADVERTISE_10HALF | ADVERTISE_10FULL | ADVERTISE_100HALF | ADVERTISE_100FULL);

    if (phy_dev->disable_hd)
        caps &= ~(PHY_CAP_10_HALF | PHY_CAP_100_HALF | PHY_CAP_1000_HALF);

    if (caps & (PHY_CAP_1000_HALF | PHY_CAP_1000_FULL))
        bmcr |= BMCR_SPEED1000;
    else if (caps & (PHY_CAP_100_HALF | PHY_CAP_100_FULL))
        bmcr |= BMCR_SPEED100;

    if (caps & (PHY_CAP_10_FULL | PHY_CAP_100_FULL | PHY_CAP_1000_FULL))
        bmcr |= BMCR_FULLDPLX;

    if (caps & (PHY_CAP_1000_HALF | PHY_CAP_1000_FULL))
        caps |= PHY_CAP_AUTONEG;

    if (caps & PHY_CAP_AUTONEG)
    {
        bmcr |= BMCR_ANENABLE | BMCR_ANRESTART;

        if (caps & PHY_CAP_PAUSE)
            adv |= ADVERTISE_PAUSE_CAP;

        if (caps & PHY_CAP_PAUSE_ASYM)
            adv |= ADVERTISE_PAUSE_ASYM;

        if (caps & PHY_CAP_10_HALF)
            adv |= ADVERTISE_10HALF;

        if (caps & PHY_CAP_10_FULL)
            adv |= ADVERTISE_10FULL;

        if (caps & PHY_CAP_100_HALF)
            adv |= ADVERTISE_100HALF;

        if (caps & PHY_CAP_100_FULL)
            adv |= ADVERTISE_100FULL;

        if (caps & PHY_CAP_1000_HALF)
            ctrl1000 |= ADVERTISE_1000HALF;

        if (caps & PHY_CAP_1000_FULL)
            ctrl1000 |= ADVERTISE_1000FULL;

        if (caps & PHY_CAP_REPEATER)
            ctrl1000 |= ADVERTISE_REPEATER;
    }

    if ((ret = phy_bus_write(phy_dev, MII_ADVERTISE, adv)))
        goto Exit;

    if ((ret = phy_bus_write(phy_dev, MII_CTRL1000, ctrl1000)))
        goto Exit;

    if ((ret = phy_bus_write(phy_dev, MII_BMCR, bmcr)))
        goto Exit;

Exit:
    return ret;
}

int mii_speed_set(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex)
{
    int ret;
    uint32_t caps;

    if (speed == PHY_SPEED_UNKNOWN)
    {
        speed = PHY_SPEED_1000;
        duplex = PHY_DUPLEX_FULL;
    }

    if ((ret = mii_caps_get(phy_dev, CAPS_TYPE_ADVERTISE, &caps)))
        goto Exit;

    caps &= ~(PHY_CAP_10_HALF | PHY_CAP_10_FULL |
        PHY_CAP_100_HALF | PHY_CAP_100_FULL |
        PHY_CAP_1000_HALF | PHY_CAP_1000_FULL);
    caps |= PHY_CAP_AUTONEG;

    switch (speed)
    {
    case PHY_SPEED_1000:
        caps |= PHY_CAP_1000_HALF | ((duplex == PHY_DUPLEX_FULL) ? PHY_CAP_1000_FULL : 0);
    case PHY_SPEED_100:
        caps |= PHY_CAP_100_HALF | ((duplex == PHY_DUPLEX_FULL) ? PHY_CAP_100_FULL : 0);
    case PHY_SPEED_10:
        caps |= PHY_CAP_10_HALF | ((duplex == PHY_DUPLEX_FULL) ? PHY_CAP_10_FULL : 0);
        break;
    default:
        break;
    }

    if ((ret = mii_caps_set(phy_dev, caps)))
        goto Exit;

Exit:
    return ret;
}

int mii_init(phy_dev_t *phy_dev)
{
    int ret;
    phy_speed_t speed = PHY_SPEED_100;
    phy_duplex_t duplex = PHY_DUPLEX_FULL;

    /* Reset phy */
    if ((ret = phy_bus_write(phy_dev, MII_BMCR, BMCR_RESET)))
        goto Exit;

    if ((ret = mii_caps_set(phy_dev, PHY_CAP_AUTONEG | PHY_CAP_PAUSE | PHY_CAP_REPEATER)))
        goto Exit;

    /* Set supported speed */
    if ((phy_dev->mii_type != PHY_MII_TYPE_MII) &&  (phy_dev->mii_type != PHY_MII_TYPE_TMII))
        speed = PHY_SPEED_1000;

    ret = mii_speed_set(phy_dev, speed, duplex);

Exit:
    return ret;
}

int mii_isolate_phy(phy_dev_t *phy, int isolate)
{
    uint16_t val;
    int ret;
    int enabled;

    ret = phy_dev_power_get(phy, &enabled);
    if (!enabled)
        return -1;

    ret += phy_bus_read(phy, MII_BMCR, &val);
    if (isolate) {
        val |= BMCR_ISOLATE;
    } else {
        val &= ~BMCR_ISOLATE;
    }
    ret += phy_bus_write(phy, MII_BMCR, val);
    return ret;
}

