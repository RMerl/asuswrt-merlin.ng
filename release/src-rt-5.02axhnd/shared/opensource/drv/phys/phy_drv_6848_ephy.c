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
 * Phy driver for 6848 internal dual 100M phy
 */

#include "bus_drv.h"
#include "phy_drv.h"
#include "phy_drv_mii.h"
#include "phy_drv_brcm.h"
#include "rdp_map.h"
#include "access_macros.h"

static uint32_t enabled_ports;

static int _phy_afe(phy_dev_t *phy_dev)
{
    int ret;
    
    if ((ret = brcm_shadow_write(phy_dev, 2,  0x1a, 0x3be0))) /* Set internal and external current trim values */
        goto Exit;

    if ((ret = brcm_shadow_write(phy_dev, 3,  0x23, 0x0006))) /* Cal reset */
        goto Exit;

    if ((ret = brcm_shadow_write(phy_dev, 3,  0x23, 0x0000))) /* Cal reset disable */
        goto Exit;

    if ((ret = brcm_shadow_write(phy_dev, 3,  0x07, 0x0002))) /* PCS Test Reg */
        goto Exit;

Exit:
    return ret;
}

static int _phy_init(phy_dev_t *phy_dev)
{
    int ret;
    uint16_t val;

    if ((ret = mii_init(phy_dev)))
        goto Exit;

    /* LED Control: set LEDs to blinky link mode */
    val = 0;
    val |= (1 << 3); /* Set LED0 to mode blinky link mode */
    val |= (1 << 7); /* Set LED1 to mode blinky link mode */
    if ((ret = brcm_shadow_write(phy_dev, 2,  0x15, val)))
        goto Exit;

    /* Enable force auto MDIX */
    if ((ret = brcm_shadow_read(phy_dev, 1,  0x10, &val)))
        goto Exit;

    if ((ret = brcm_shadow_write(phy_dev, 1,  0x10, val | (1 << 14))))
        goto Exit;

    /* AFE workaround */
    if ((ret = _phy_afe(phy_dev)))
        goto Exit;

Exit:
    return ret;
}

static int _phy_cfg(uint32_t port_map)
{
    uint32_t data;

    /* RESET_N --> 1 */
    READ_32(EGPHY_EPHY_RESET_CNTRL, data);
    data |= 0x10;
    WRITE_32(EGPHY_EPHY_RESET_CNTRL, data);
    udelay(50);

    /* DLLBYP_PIN_F --> 1 */
    data = 0x02;
    WRITE_32(EGPHY_EPHY_TEST_CNTRL, data);
    udelay(50);

    /* RESET_N --> 0 */
    READ_32(EGPHY_EPHY_RESET_CNTRL, data);
    data &= ~0x10;
    WRITE_32(EGPHY_EPHY_RESET_CNTRL, data);
    udelay(50);

    /* DLLBYP_PIN_F --> 1 */
    data = 0x02;
    WRITE_32(EGPHY_EPHY_TEST_CNTRL, data);
    udelay(50);

    /* RESET_N_PHY_(0-3), RESET_N --> 1 */
    data |= 0x1f;
    WRITE_32(EGPHY_EPHY_RESET_CNTRL, data);
    udelay(50);

    /* RESET_N_PHY_(0-3), RESET_N --> 0 */
    data = 0x00;
    WRITE_32(EGPHY_EPHY_RESET_CNTRL, data);
    udelay(50);

    /* EGPHY_EPHY_PWR_MGNT */
    data = 0X21fffffd;
    WRITE_32(EGPHY_EPHY_PWR_MGNT, data);
    udelay(50);

    if (port_map & 0x01) /* port 0 */
        data &= ~(0x1f << 20); /* EXT_PWR_DOWN_PHY_0 */
    if (port_map & 0x02) /* port 1 */
        data &= ~(0x1f << 15); /* EXT_PWR_DOWN_PHY_1 */
    if (port_map & 0x04) /* port 2 */
        data &= ~(0x1f << 10); /* EXT_PWR_DOWN_PHY_2 */
    if (port_map & 0x08) /* port 3 */
        data &= ~(0x1f << 5); /* EXT_PWR_DOWN_PHY_3 */
    if (port_map)
        data &= ~0x2000001c; /* IDDQ_GLOBAL_PWR, EXT_PWR_DOWN_BIAS, EXT_PWR_DOWN_DLL, EXT_PWR_DOWN_PHY */

    WRITE_32(EGPHY_EPHY_PWR_MGNT, data);
    udelay(50);

    /* RESET_N --> 1 */
    READ_32(EGPHY_EPHY_RESET_CNTRL, data);
    data |= 0x10;
    WRITE_32(EGPHY_EPHY_RESET_CNTRL, data);
    udelay(50);

    /* RESET_N --> 0 */
    data = 0x00;
    WRITE_32(EGPHY_EPHY_RESET_CNTRL, data);
    udelay(50);

    return 0;
}

static int _phy_dev_add(phy_dev_t *phy_dev)
{
    uint32_t port = (uint32_t)phy_dev->priv;

    enabled_ports |= (1 << port);

    return 0;
}

static int _phy_dev_del(phy_dev_t *phy_dev)
{
    uint32_t port = (uint32_t)phy_dev->priv;

    enabled_ports &= ~(1 << port);

    return 0;
}

static int _phy_drv_init(phy_drv_t *phy_drv)
{
    if (_phy_cfg(enabled_ports & 0xf))
    {
        printk("Failed to initialize the ephy driver\n");
        return -1;
    }

    phy_drv->initialized = 1;

    return 0;
}

#ifndef _CFE_
#include <linux/delay.h>

#define SP100_CARRIER_ERROR_MASK ((1<<9)|(1<<8)|(1<<5))
static int _phy_workaround_check(phy_dev_t *phy_dev)
{
    uint16_t val;

    /* Read the auxiliary status summary register */
    if ((phy_dev_read(phy_dev, 0x19, &val)))
        goto Error;

    /* Ensure the link is still up and speed is still 100M */
    if (!((val >> 2) & 0x1) || !((val >> 3) & 0x1))
        return 0;

    /* Read the receive packet counter register */
    if (brcm_shadow_read(phy_dev, 3, 0x12, &val))
        goto Error;

    /* If packets were received, the phy is working normally */    
    if (val)
        return 0;

    /* Read the auxiliary status register */
    if (phy_dev_read(phy_dev, 0x11, &val))
        goto Error;

    /* If a false carrier sense was detected, reset the phy */
    if ((val & SP100_CARRIER_ERROR_MASK) == SP100_CARRIER_ERROR_MASK)
    {
        printk ("**** Detected EPHY unusual status, reset PHY to recover ****\n");
        phy_dev_init(phy_dev);
        return 0;
    }

    /* Packets were not received yet nor a false carrier sense was detected, keep polling */
    return 1;

Error:
    printk("_phy_workaround_check: Error accessing phy register\n");
    return 0;
}

static void _phy_work_handler(phy_dev_t *phy_dev)
{
    int ret;
    uint16_t val;

    /* Enable and reset RX packet counter */
    val = 0;
    val |= (1 << 0); /* Packet Check Enable */
    val |= (1 << 1); /* Receive Packet Counter Clear */
    if ((ret = brcm_shadow_write(phy_dev, 3,  0x11, val)))
        goto Error;

    /* Clear RX packet counter by reading it */
    if ((ret = brcm_shadow_read(phy_dev, 3,  0x12, &val)))
        goto Error;

    /* Clear error detected bits in the auxiliary status register by reading it */
    if ((ret = phy_dev_read(phy_dev, 0x11, &val)))
        goto Error;

    /* Poll the phy until packets received or a reset was done to fix a false carrier sense problem */
    while (_phy_workaround_check(phy_dev))
        msleep(1000);

    return;

Error:
    printk ("_phy_work_handler: Error accessing phy register\n");
    return;
}
#endif

static int _phy_read_status(phy_dev_t *phy_dev)
{
    int ret;
    uint16_t val;
#ifndef _CFE_
    phy_speed_t speed = phy_dev->speed;
#endif

    phy_dev->link = 0;
    phy_dev->speed = PHY_SPEED_UNKNOWN;
    phy_dev->duplex = PHY_DUPLEX_UNKNOWN;
    phy_dev->pause_rx = 0;
    phy_dev->pause_tx = 0;

    if ((ret = phy_dev_read(phy_dev, 0x19, &val)))
        goto Exit;

    phy_dev->link = ((val >> 2) & 0x1);

    if (!phy_dev->link)
        return 0;

    phy_dev->speed = ((val >> 3) & 0x1) ? PHY_SPEED_100 : PHY_SPEED_10; 
    phy_dev->duplex = ((val >> 0) & 0x1) ? PHY_DUPLEX_FULL : PHY_DUPLEX_HALF;
    phy_dev->pause_rx = phy_dev->pause_tx = ((val >> 11) & 0x1);

#ifndef _CFE_
    if (phy_dev->link && phy_dev->speed != speed && phy_dev->speed == PHY_SPEED_100)
        ret = phy_dev_queue_work(phy_dev, _phy_work_handler);
#endif

Exit:
    return ret;
}

static int _phy_power_get(phy_dev_t *phy_dev, int *enable)
{
    uint32_t data;
    uint32_t port = (uint32_t)phy_dev->priv;
    
    READ_32(EGPHY_EPHY_PWR_MGNT, data);
    *enable = (data & (0x1 << (0x1c - port))) ? 0 : 1;

    return 0;
}

static int _phy_power_set(phy_dev_t *phy_dev, int enable)
{
    uint32_t data;
    uint32_t port = (uint32_t)phy_dev->priv;
    
    READ_32(EGPHY_EPHY_PWR_MGNT, data);
    if (enable)
        data &= ~(0x1 << (0x1c - port));
    else
        data |= (0x1 << (0x1c - port));

    WRITE_32(EGPHY_EPHY_PWR_MGNT, data);
    udelay(50);

    return 0;
}

phy_drv_t phy_drv_6848_ephy =
{
    .phy_type = PHY_TYPE_6848_EPHY,
    .name = "EPHY",
    .power_get = _phy_power_get,
    .power_set = _phy_power_set,
    .apd_set = brcm_ephy_apd_set,
    .eee_get = brcm_ephy_eee_get,
    .eee_set = brcm_ephy_eee_set,
    .eee_resolution_get = brcm_ephy_eee_resolution_get,
    .read_status = _phy_read_status,
    .speed_set = mii_speed_set,
    .caps_get = mii_caps_get,
    .caps_set = mii_caps_set,
    .phyid_get = mii_phyid_get,
    .init = _phy_init,
    .dev_add = _phy_dev_add,
    .dev_del = _phy_dev_del,
    .drv_init = _phy_drv_init,
};
