/*
   Copyright (c) 2016 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2016:DUAL/GPL:standard

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
 *  Created on: June 2017
 *      Author: dima.mamut@broadcom.com
 */

/*
 * MDIO driver for BCM96846 and 6856
 */

#include "bcm_map_part.h"
#include "mdio_drv_impl5.h"

#ifdef _CFE_
#define spin_lock_bh(x)
#define spin_unlock_bh(x)
#else
#include <linux/spinlock.h>
static DEFINE_SPINLOCK(mdio_access);
#endif

#define MDIO_CMD        (void *)MDIO_BASE

static void mdio_cfg_type(mdio_type_t type)
{
    if (type == MDIO_INT)
        TOP->MdioMasterSelect = 0;
    else
        TOP->MdioMasterSelect = 1;
}

int32_t mdio_read_c22_register(mdio_type_t type, uint32_t addr, uint32_t reg, uint16_t *val)
{
    int ret = MDIO_OK;

    spin_lock_bh(&mdio_access);
    mdio_cfg_type(type);
    ret =  mdio_cmd_read_22(MDIO_CMD, addr, reg, val);
    spin_unlock_bh(&mdio_access);

    return ret;
}
EXPORT_SYMBOL(mdio_read_c22_register);

int32_t mdio_write_c22_register(mdio_type_t type, uint32_t addr, uint32_t reg, uint16_t val)
{
    int ret = MDIO_OK;

    spin_lock_bh(&mdio_access);
    mdio_cfg_type(type);
    ret = mdio_cmd_write_22(MDIO_CMD, addr, reg, val);
    spin_unlock_bh(&mdio_access);

    return ret;
}
EXPORT_SYMBOL(mdio_write_c22_register);

int32_t mdio_read_c45_register(mdio_type_t type, uint32_t addr, uint32_t dev, uint16_t reg, uint16_t *val)
{
    int ret = MDIO_OK;

    spin_lock_bh(&mdio_access);
    mdio_cfg_type(type);
    ret = mdio_cmd_read_45(MDIO_CMD, addr, dev, reg, val);
    spin_unlock_bh(&mdio_access);

    return ret;
}
EXPORT_SYMBOL(mdio_read_c45_register);

int32_t mdio_write_c45_register(mdio_type_t type, uint32_t addr, uint32_t dev, uint16_t reg, uint16_t val)
{
    int ret = MDIO_OK;

    spin_lock_bh(&mdio_access);
    mdio_cfg_type(type);
    ret = mdio_cmd_write_45(MDIO_CMD, addr, dev, reg, val);
    spin_unlock_bh(&mdio_access);

    return ret;
}
EXPORT_SYMBOL(mdio_write_c45_register);


