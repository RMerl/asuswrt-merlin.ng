/*
   Copyright (c) 2016 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2016:DUAL/GPL:standard
    
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


