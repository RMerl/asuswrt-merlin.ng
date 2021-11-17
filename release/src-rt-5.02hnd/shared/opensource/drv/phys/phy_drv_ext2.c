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
 * Phy driver for external 1G phy: BCM54210E
 */

#include "bus_drv.h"
#include "phy_drv.h"
#include "phy_drv_mii.h"
#include "phy_drv_brcm.h"

#define MISC_CTRL           0x002f /* Copper Miscellaneous Control */

static int _phy_rgmii_init(phy_dev_t *phy_dev)
{
    uint16_t val;
    int ret;

   /* Copper Miscellaneous Control */
    if ((ret = phy_dev_read(phy_dev, RDB_ACCESS | MISC_CTRL, &val)))
        goto Exit;

    val &= ~(1 << 5); /* Send out-of-band status info in RGMII mode */
    val |= (1 << 7); /* RGMII interface */
    val |= (1 << 8); /* Enables internal RXC delay */

    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | MISC_CTRL, val)))
        goto Exit;

Exit:
    return ret;
}

static int _phy_init(phy_dev_t *phy_dev)
{
    int ret;

    if ((ret = mii_init(phy_dev)))
        goto Exit;

    if (phy_dev->mii_type == PHY_MII_TYPE_RGMII)
        ret = _phy_rgmii_init(phy_dev);

    brcm_rdb_set(phy_dev, 0);

Exit:
    return ret;
}

phy_drv_t phy_drv_ext2 =
{
    .phy_type = PHY_TYPE_EXT2,
    .name = "EXT2",
    .read = brcm_egphy_read,
    .write = brcm_egphy_write,
    .power_set = mii_power_set,
    .apd_set = brcm_egphy_apd_set,
    .eee_get = brcm_egphy_eee_get,
    .eee_set = brcm_egphy_eee_set,
    .read_status = brcm_read_status,
    .speed_set = mii_speed_set,
    .caps_get = mii_caps_get,
    .caps_set = mii_caps_set,
    .init = _phy_init,
};
