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
 * Phy driver for external 1G phys: BCM5461S, BCM54810
 */

#include "bus_drv.h"
#include "phy_drv.h"
#include "phy_drv_mii.h"
#include "phy_drv_brcm.h"

static int _phy_init(phy_dev_t *phy_dev)
{
    int ret;

    /* Access IEEE register set instead of LRE */
    if ((ret = phy_bus_write(phy_dev, 0x0e, 0x06)))
        goto Exit;

    if ((ret = mii_init(phy_dev)))
        goto Exit;

    /* Disable BroadR-Reach function */
    if ((ret = brcm_exp_write(phy_dev, 0x90, 0x00)))
        goto Exit;

    if ((phy_dev->mii_type == PHY_MII_TYPE_RGMII) && (ret = brcm_shadow_rgmii_init(phy_dev)))
        goto Exit;

    if ((ret = brcm_shadow_18_force_auto_mdix_set(phy_dev, 1)))
        goto Exit;

    if ((ret = brcm_shadow_18_eth_wirespeed_set(phy_dev, 1)))
        goto Exit;

Exit:
    return ret;
}

phy_drv_t phy_drv_ext1 =
{
    .phy_type = PHY_TYPE_EXT1,
    .name = "EXT1",
    .power_get = mii_power_get,
    .power_set = mii_power_set,
    .apd_get = brcm_shadow_1c_apd_get,
    .apd_set = brcm_shadow_1c_apd_set,
    .read_status = brcm_read_status,
    .speed_set = mii_speed_set,
    .caps_get = mii_caps_get,
    .caps_set = mii_caps_set,
    .phyid_get = mii_phyid_get,
    .init = _phy_init,
};
