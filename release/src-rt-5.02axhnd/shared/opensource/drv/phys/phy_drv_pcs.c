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

/*
 * Phy driver for 6838/6848 AE PCS
 */

#include "bus_drv.h"
#include "phy_drv.h"
#include "phy_drv_mii.h"

#define PCS_XCTL1           0x10
#define PCS_XCTL2           0x11
#define PCS_XCTL3           0x12
#define PCS_XCTL4           0x13
#define PCS_XSTAT1          0x14
#define PCS_XSTAT2          0x15
#define PCS_XSTAT3          0x16

static int pcs_read_status(phy_dev_t *phy_dev)
{
    uint16_t val;
    int ret, speed, duplex;

    phy_dev->link = 0;
    phy_dev->speed = PHY_SPEED_UNKNOWN;
    phy_dev->duplex = PHY_DUPLEX_UNKNOWN;

    if ((ret = phy_bus_read(phy_dev, PCS_XSTAT1, &val)))
        return ret;

    phy_dev->link = ((val >> 1) & 0x1);

    if (!phy_dev->link)
        return 0;

    speed = ((val >> 3) & 0x3);
    duplex = ((val >> 2) & 0x1);

    if (speed == 0)
        phy_dev->speed = PHY_SPEED_10;
    else if (speed == 1)
        phy_dev->speed = PHY_SPEED_100;
    else if (speed == 2)
        phy_dev->speed = PHY_SPEED_1000;

    phy_dev->duplex = duplex ? PHY_DUPLEX_FULL : PHY_DUPLEX_HALF;

    return 0;
}

static int pcs_speed_set(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex)
{
    udelay(10000);
    phy_dev_read_status(phy_dev);

    return 0;
}

static int pcs_init(phy_dev_t *phy_dev)
{
    uint16_t val;
    int ret;

    /* Reset the PCS */
    if ((ret = phy_bus_write(phy_dev, MII_BMCR, 0x8000)))
        goto Exit;

    /* Enable auto detection of Fiber/SGMII mode */
    if ((ret = phy_bus_write(phy_dev, PCS_XCTL1, 0x4110)))
        goto Exit;

    /* Enable parallel detection */
    if ((ret = phy_bus_write(phy_dev, PCS_XCTL2, 0x0007)))
        goto Exit;

    /* Turn on support of 100/10 */
    if ((ret = phy_bus_write(phy_dev, PCS_XCTL4, val | 0x0801)))
        goto Exit;

    /* Advertise full-duplex and pause capabilities */
    if ((ret = phy_bus_write(phy_dev, MII_ADVERTISE, 0x01a0)))
        goto Exit;

    /* Restart auto negotiation */
    if ((ret = phy_bus_write(phy_dev, MII_BMCR, 0x1340)))
        goto Exit;

Exit:
    return ret;
}

phy_drv_t phy_drv_pcs =
{
    .phy_type = PHY_TYPE_PCS,
    .name = "PCS",
    .power_get = mii_power_get,
    .power_set = mii_power_set,
    .read_status = pcs_read_status,
    .speed_set = pcs_speed_set,
    .init = pcs_init,
};
