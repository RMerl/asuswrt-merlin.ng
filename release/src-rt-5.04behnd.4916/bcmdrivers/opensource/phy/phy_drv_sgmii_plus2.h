/*
   Copyright (c) 2016 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2016:DUAL/GPL:standard
    
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
 *  Created on: Aug 2016
 *      Author: yuval.raviv@broadcom.com
 */

/*
 * Common functions for SGMIIPLUS2 core
 */

#ifndef __PHY_DRV_SGMII_PLUS2_H__
#define __PHY_DRV_SGMII_PLUS2_H__

#include "phy_drv.h"

typedef enum
{
    SGMII_REFCLK_UNKNOWN,
    SGMII_REFCLK_50,
    SGMII_REFCLK_80,
    SGMII_REFCLK_125,
} sgmii_refclk_t;

int sgmii_read(phy_dev_t *phy_dev, uint16_t reg, uint16_t *val);
int sgmii_write(phy_dev_t *phy_dev, uint16_t reg, uint16_t val);

int sgmii_speed_set(phy_dev_t *phy_dev, phy_speed_t speed);
int sgmii_refclk_init(phy_dev_t *phy_dev, sgmii_refclk_t refclk);
int sgmii_read_status(phy_dev_t *phy_dev);

#endif

