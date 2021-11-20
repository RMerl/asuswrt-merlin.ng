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
 * Phy driver for external 1G phy: BCM50210, BCM54210
 */

#include "bus_drv.h"
#include "phy_drv.h"
#include "phy_drv_mii.h"
#include "phy_drv_brcm.h"

#define MODE_CTRL           0x0021 /* Mode Control */
#define COPPER_MISC_CTRL    0x002f /* Copper Miscellaneous Control */
#define CLOCK_ALIGN_CTRL    0x0013 /* Clock Alignment Control Regsiter */

static int _phy_access_sgmii_registers(phy_dev_t *phy_dev, int enable)
{
    uint16_t val;
    int ret;

    if ((ret = phy_dev_read(phy_dev, RDB_ACCESS | MODE_CTRL, &val)))
        goto Exit;

    if (enable)
        val |= (1 << 0); /* Select SGMII overlay registers */
    else
        val &= ~(1 << 0); /* Select copper overlay registers */

    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | MODE_CTRL, val)))
        goto Exit;

Exit:
    return ret;
}

static int _phy_sgmii_init(phy_dev_t *phy_dev)
{
    uint16_t val = 0x1140;
    int ret;

    /* Select SGMII overlay registers */
    if ((ret = _phy_access_sgmii_registers(phy_dev, 1)))
        goto Exit;

    /* Initialize the SGMII Control register */
    if ((ret = phy_dev_write(phy_dev, MII_BMCR, val)))
        goto Exit;

    /* Select copper overlay registers */
    if ((ret = _phy_access_sgmii_registers(phy_dev, 0)))
        goto Exit;

Exit:
    return ret;
}

static int _phy_rgmii_init(phy_dev_t *phy_dev)
{
    uint16_t val;
    int ret;

    /* Copper Miscellaneous Control */
    if ((ret = phy_dev_read(phy_dev, RDB_ACCESS | COPPER_MISC_CTRL, &val)))
        goto Exit;

    val |= (1 << 5); /* Disable RGMII Out of Band Status */;
    val |= (1 << 7); /* Enable RGMII interface */

    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | COPPER_MISC_CTRL, val)))
        goto Exit;

Exit:
    return ret;
}

static int _phy_set_delay(phy_dev_t *phy_dev)
{
    uint16_t val;
    int ret;

    /* Copper Miscellaneous Control */
    if ((ret = phy_dev_read(phy_dev, RDB_ACCESS | COPPER_MISC_CTRL, &val)))
        goto Exit;

    if (phy_dev->delay_rx)
        val |= (1 << 8); /* Enable RGMII RXC delay */
    else
        val &= ~(1 << 8); /* Disable RGMII RXC delay */

    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | COPPER_MISC_CTRL, val)))
        goto Exit;

    /* Clock Alignment Control Regsiter */
    if ((ret = phy_dev_read(phy_dev, RDB_ACCESS | CLOCK_ALIGN_CTRL, &val)))
        goto Exit;

    if (phy_dev->delay_tx)
        val |= (1 << 9); /* Enable GTXCLK delay */
    else
        val &= ~(1 << 9); /* Disable GTXCLK delay */

    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | CLOCK_ALIGN_CTRL, val)))
        goto Exit;

Exit:
    return ret;
}

static int _phy_mode_select(phy_dev_t *phy_dev)
{
    uint16_t val;
    int ret;

    /* Mode Control */
    if ((ret = phy_dev_read(phy_dev, RDB_ACCESS | MODE_CTRL, &val)))
        goto Exit;

    val &= ~(3 << 1); /* Clear Mode select bits */

    if (phy_dev->mii_type == PHY_MII_TYPE_RGMII)
        val |= (0 << 1); /* RGMII to Copper */
    if (phy_dev->mii_type == PHY_MII_TYPE_SGMII)
        val |= (2 << 1); /* SGMII to Copper */

    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | MODE_CTRL, val)))
        goto Exit;

    /* Enable SGMII auto-negotiation */
    if ((phy_dev->mii_type == PHY_MII_TYPE_SGMII) && (ret = _phy_sgmii_init(phy_dev)))
        goto Exit;

    /* Enable RGMII interface */
    if ((phy_dev->mii_type == PHY_MII_TYPE_RGMII) && (ret = _phy_rgmii_init(phy_dev)))
        goto Exit;

    /* Restart auto-negotiation */
    if ((ret = phy_dev_read(phy_dev, MII_BMCR, &val)))
        goto Exit;

    val |= (1 << 12); /* Enable auto negotiation */
    val |= (1 << 9); /* Restart auto negotiation */

    if ((ret = phy_dev_write(phy_dev, MII_BMCR, val)))
        goto Exit;

Exit:
    return ret;
}

static int _phy_init(phy_dev_t *phy_dev)
{
    int ret;

    if ((ret = mii_init(phy_dev)))
        goto Exit;

    if ((ret = _phy_set_delay(phy_dev)))
        goto Exit;

    if ((ret = _phy_mode_select(phy_dev)))
        goto Exit;

    if ((ret = brcm_egphy_force_auto_mdix_set(phy_dev, 1)))
        goto Exit;

    if ((ret = brcm_egphy_eth_wirespeed_set(phy_dev, 1)))
        goto Exit;

Exit:
    return ret;
}

phy_drv_t phy_drv_ext2 =
{
    .phy_type = PHY_TYPE_EXT2,
    .name = "EXT2",
    .read = brcm_egphy_read,
    .write = brcm_egphy_write,
    .power_get = mii_power_get,
    .power_set = mii_power_set,
    .apd_get = brcm_egphy_apd_get,
    .apd_set = brcm_egphy_apd_set,
    .eee_get = brcm_egphy_eee_get,
    .eee_set = brcm_egphy_eee_set,
    .eee_resolution_get = brcm_egphy_eee_resolution_get,
    .read_status = brcm_read_status,
    .speed_set = mii_speed_set,
    .caps_get = mii_caps_get,
    .caps_set = mii_caps_set,
    .auto_mdix_set = brcm_egphy_force_auto_mdix_set,
    .auto_mdix_get = brcm_egphy_force_auto_mdix_get,
    .phyid_get = mii_phyid_get,
    .init = _phy_init,
};
