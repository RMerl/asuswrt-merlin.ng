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
