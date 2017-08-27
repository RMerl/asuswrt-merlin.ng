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

#define REG_18                              0x18
#define REG_1C                              0x1c

#define REG_18_SHADOW_SHIFT                 12
#define REG_1C_SHADOW_SHIFT                 10

#define REG_18_SHADOW_MISC_CTRL             0x07

#define REG_1C_SHADOW_APD                   0x0a
#define REG_1C_SHADOW_SPARE_CTRL            0x05
#define REG_1C_SHADOW_CLK_ALIGN_CTRL        0x03

#define MISC_CTRL_RGMII_OOB_STATUS_DISABLE  (1 << 5)
#define MISC_CTRL_RGMII_ENABLE              (1 << 7)
#define MISC_CTRL_RGMII_RXC_DELAY           (1 << 8)

#define APD_ENABLE                          (1 << 5)
#define APD_COMPATIBILITY                   (1 << 8)
#define APD_WAKEUP_TIMER                    (1 << 0)

#define SPARE_CTRL_CLK125_OUTPUT            (1 << 0)
#define SPARE_CTRL_CLK125_APD_DISABLE       (1 << 1)

#define CLK_ALIGN_CLK_DELAY_ENABLE          (1 << 9)

#define WRITE_ENABLE                        (1 << 15)

static int _apd_set(phy_dev_t *phy_dev, int enable)
{
    uint16_t val;
    int ret;

    val = (REG_1C_SHADOW_APD << REG_1C_SHADOW_SHIFT) | APD_WAKEUP_TIMER;
    if (enable)
        val |= APD_ENABLE | APD_COMPATIBILITY;

    if ((ret = phy_bus_write(phy_dev, REG_1C, val | WRITE_ENABLE)))
        goto Exit;

    val = (REG_1C_SHADOW_SPARE_CTRL << REG_1C_SHADOW_SHIFT);

    if ((ret = phy_bus_write(phy_dev, REG_1C, val)))
        goto Exit;

    if ((ret = phy_bus_read(phy_dev, REG_1C, &val)))
        goto Exit;

    if (enable)
    {
        val &= ~SPARE_CTRL_CLK125_OUTPUT;
        val &= ~SPARE_CTRL_CLK125_APD_DISABLE;
    }
    else
    {
        val |= SPARE_CTRL_CLK125_OUTPUT;
        val |= SPARE_CTRL_CLK125_APD_DISABLE;
    }

    if ((ret = phy_bus_write(phy_dev, REG_1C, val | WRITE_ENABLE)))
        goto Exit;

Exit:
    return ret;
}

static int _phy_rgmii_init(phy_dev_t *phy_dev)
{
    uint16_t val;
    int ret;

    val = REG_18_SHADOW_MISC_CTRL | (REG_18_SHADOW_MISC_CTRL << REG_18_SHADOW_SHIFT);

    if ((ret = phy_bus_write(phy_dev, REG_18, val)))
        goto Exit;

    if ((ret = phy_bus_read(phy_dev, REG_18, &val)))
        goto Exit;

    val &= ~MISC_CTRL_RGMII_OOB_STATUS_DISABLE;
    val |= MISC_CTRL_RGMII_ENABLE | WRITE_ENABLE;
#if defined(CONFIG_BCM947189)
    val &= ~MISC_CTRL_RGMII_RXC_DELAY;
#else
    val |= MISC_CTRL_RGMII_RXC_DELAY;
#endif

    if ((ret = phy_bus_write(phy_dev, REG_18, val)))
        goto Exit;

    val = REG_1C_SHADOW_CLK_ALIGN_CTRL << REG_1C_SHADOW_SHIFT;

    if ((ret = phy_bus_write(phy_dev, REG_1C, val)))
        goto Exit;

    if ((ret = phy_bus_read(phy_dev, REG_1C, &val)))
        goto Exit;

#if defined(CONFIG_BCM947189)
    val &= ~CLK_ALIGN_CLK_DELAY_ENABLE;
#endif
    val |= WRITE_ENABLE;
    if ((ret = phy_bus_write(phy_dev, REG_1C, val)))
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

Exit:
    return ret;
}

phy_drv_t phy_drv_ext1 =
{
    .phy_type = PHY_TYPE_EXT1,
    .name = "EXT1",
    .power_set = mii_power_set,
    .apd_set = _apd_set,
    .read_status = brcm_read_status,
    .speed_set = mii_speed_set,
    .caps_get = mii_caps_get,
    .caps_set = mii_caps_set,
    .init = _phy_init,
};
