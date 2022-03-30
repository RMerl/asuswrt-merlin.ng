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
 *  Created on: Oct 2016
 *      Author: ido.brezel@broadcom.com
 */

/*
 * Generic phy status mechanism for pon phy driver
 */

#include "phy_drv.h"

static phy_dev_t *g_phy_dev = NULL;
static int g_link;
static phy_speed_t g_speed = PHY_SPEED_UNKNOWN;
static phy_duplex_t g_duplex = PHY_DUPLEX_UNKNOWN;

phy_dev_t * enet_pon_phy_dev_get(void)
{
    return g_phy_dev;
}
EXPORT_SYMBOL(enet_pon_phy_dev_get);

void enet_pon_drv_link_change(int link)
{
    if (g_link == link)
        return;

    g_link = link;

    if (g_phy_dev)
        phy_dev_link_change_notify(g_phy_dev);
}
EXPORT_SYMBOL(enet_pon_drv_link_change);

void enet_pon_drv_speed_change(phy_speed_t speed, phy_duplex_t duplex)
{
    g_speed = speed;
    g_duplex = duplex;
}
EXPORT_SYMBOL(enet_pon_drv_speed_change);

static int _drv_init(phy_drv_t *phy_drv)
{
    return g_phy_dev != NULL;
}

static int _phy_init(phy_dev_t *phy_dev)
{
    phy_dev->mii_type = PHY_MII_TYPE_SERDES;
    phy_dev_link_change_notify(phy_dev);

    return 0;
}

static int _phy_read_status(phy_dev_t *phy_dev)
{
    phy_dev->speed = PHY_SPEED_UNKNOWN;
    phy_dev->duplex = PHY_DUPLEX_UNKNOWN;

    phy_dev->link = g_link;
    if (!phy_dev->link)
        goto Exit;

    phy_dev->speed = g_speed;
    phy_dev->duplex = g_duplex;

Exit:
    return 0;
}

static int _link_change_register(phy_dev_t *phy_dev)
{
    g_phy_dev = phy_dev;

    return 0;
}

static int _link_change_unregister(phy_dev_t *phy_dev)
{
    g_phy_dev = NULL;

    return 0;
}

phy_drv_t phy_drv_pon =
{
    .phy_type = PHY_TYPE_PON,
    .name = "PON",
    .init = _phy_init,
    .read_status = _phy_read_status,
    .drv_init = _drv_init,
    .link_change_register = _link_change_register,
    .link_change_unregister = _link_change_unregister,
};
EXPORT_SYMBOL(phy_drv_pon);

