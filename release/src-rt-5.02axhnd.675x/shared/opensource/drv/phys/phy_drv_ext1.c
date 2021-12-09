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
    .auto_mdix_set = brcm_shadow_18_force_auto_mdix_set,
    .auto_mdix_get = brcm_shadow_18_force_auto_mdix_get,
    .init = _phy_init,
};
