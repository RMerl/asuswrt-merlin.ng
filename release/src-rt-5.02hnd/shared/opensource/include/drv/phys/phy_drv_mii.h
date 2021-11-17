/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2, as published by
    the Free Software Foundation (the "GPL").
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    
    A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
    writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    
:>
*/

/*
 *  Created on: Dec 2015
 *      Author: yuval.raviv@broadcom.com
 */

#ifndef __PHY_DRV_MII_H__
#define __PHY_DRV_MII_H__

#include "phy_drv.h"

/* Generic MII registers shared by kernel network driver and CFE */
#define MII_BMCR                0x00    /* Basic mode control register          */
#define MII_BMSR                0x01    /* Basic mode status register           */
#define MII_PHYSID1             0x02    /* PHYS ID 1                            */
#define MII_PHYSID2             0x03    /* PHYS ID 2                            */
#define MII_ADVERTISE           0x04    /* Advertisement control reg            */
#define MII_CTRL1000            0x09    /* 1000BASE-T control                   */

/* Basic mode control register */
#define BMCR_SPEED1000          0x0040  /* MSB of Speed (1000)                  */
#define BMCR_FULLDPLX           0x0100  /* Full duplex                          */
#define BMCR_ANRESTART          0x0200  /* Auto negotiation restart             */
#define BMCR_POWERDOWN          0x0800  /* Enable low power state               */
#define BMCR_ANENABLE           0x1000  /* Enable auto negotiation              */
#define BMCR_SPEED100           0x2000  /* Select 100Mbps                       */
#define BMCR_RESET              0x8000  /* Reset to default state               */

/* Advertisement control register */
#define ADVERTISE_CSMA          0x0001  /* Only selector supported              */
#define ADVERTISE_10HALF        0x0020  /* Try for 10mbps half-duplex           */
#define ADVERTISE_10FULL        0x0040  /* Try for 10mbps full-duplex           */
#define ADVERTISE_100HALF       0x0080  /* Try for 100mbps half-duplex          */
#define ADVERTISE_100FULL       0x0100  /* Try for 100mbps full-duplex          */
#define ADVERTISE_PAUSE_CAP     0x0400  /* Try for pause                        */

/* 1000BASE-T Control register */
#define ADVERTISE_REPEATER      0x0400  /* Advertise Repeater mode              */
#define ADVERTISE_1000FULL      0x0200  /* Advertise 1000BASE-T full duplex     */
#define ADVERTISE_1000HALF      0x0100  /* Advertise 1000BASE-T half duplex     */

int mii_autoneg_restart(phy_dev_t *phy_dev);
int mii_power_set(phy_dev_t *phy_dev, int enable);
int mii_speed_set(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex);
int mii_caps_get(phy_dev_t *phy_dev, uint32_t *caps);
int mii_caps_set(phy_dev_t *phy_dev, uint32_t caps);
int mii_init(phy_dev_t *phy_dev);

#endif

