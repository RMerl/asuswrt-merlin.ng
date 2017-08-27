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

