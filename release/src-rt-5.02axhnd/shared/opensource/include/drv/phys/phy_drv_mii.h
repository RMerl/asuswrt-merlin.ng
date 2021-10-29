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

