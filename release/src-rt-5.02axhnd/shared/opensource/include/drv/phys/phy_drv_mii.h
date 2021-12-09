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
#include "linux/mii.h"

/* Basic mode control register */
#define BMCR_POWERDOWN          BMCR_PDOWN

/* 1000BASE-T Control register */
#define ADVERTISE_REPEATER      0x0400  /* Advertise Repeater mode              */


int mii_phyid_get(phy_dev_t *phy_dev, uint32_t *phyid);
int mii_autoneg_restart(phy_dev_t *phy_dev);
int mii_power_get(phy_dev_t *phy_dev, int *enable);
int mii_power_set(phy_dev_t *phy_dev, int enable);
int mii_speed_set(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex);
int mii_caps_get(phy_dev_t *phy_dev, int caps_type, uint32_t *caps);
int mii_caps_set(phy_dev_t *phy_dev, uint32_t caps);
int mii_init(phy_dev_t *phy_dev);
int mii_stat_get(phy_dev_t *phy_dev, uint32_t *stat);

#endif

