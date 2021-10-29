/*
   Copyright (c) 2013 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2013:DUAL/GPL:standard
    
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
 *  Created on: Nov 2016
 *      Author: steven.hsieh@broadcom.com
 */

/*
 * MDIO driver for StarFighter2 - 4908/63138/63148/63158
 */

#include "mdio_drv_sf2.h"

#ifdef _CFE_
#define spin_lock_bh(x)
#define spin_unlock_bh(x)
#else
#include <linux/spinlock.h>
DEFINE_SPINLOCK(mdio_access);
#endif

#if defined(CONFIG_BCM963158)
    // 63158 only has one mdio master, so no need to switch

#define sf2_mdio_master_enable()
#define sf2_mdio_master_disable()

#else   // !BCM963158
#define SF2_MDIO_MASTER                     0x01

static void sf2_mdio_master_enable(void)
{
    volatile uint32_t *sw_ctrl_reg = (void *)(SWITCH_REG_BASE);
    uint32_t val32 = *sw_ctrl_reg;
    
    val32 |= SF2_MDIO_MASTER;
    *sw_ctrl_reg = val32;
}

static void sf2_mdio_master_disable(void)
{
    volatile uint32_t *sw_ctrl_reg = (void *)(SWITCH_REG_BASE);
    uint32_t val32 = *sw_ctrl_reg;
    val32 &= ~SF2_MDIO_MASTER;
    *sw_ctrl_reg = val32;
}
#endif // !BCM963158

int32_t mdio_read_c22_register(mdio_type_t type, uint32_t addr, uint32_t reg, uint16_t *val)
{
    int ret = MDIO_OK;

    spin_lock_bh(&mdio_access);
    sf2_mdio_master_disable();

    /* Read twice for DSL based chip requested by hardware */
    ret = mdio_cmd_read_22((void *)SWITCH_MDIO_BASE, addr, reg, val);
    ret += mdio_cmd_read_22((void *)SWITCH_MDIO_BASE, addr, reg, val);

    sf2_mdio_master_enable();
    spin_unlock_bh(&mdio_access);

    return ret;
}
EXPORT_SYMBOL(mdio_read_c22_register);

int32_t mdio_write_c22_register(mdio_type_t type, uint32_t addr, uint32_t reg, uint16_t val)
{
    int ret = MDIO_OK;

    spin_lock_bh(&mdio_access);
    sf2_mdio_master_disable();

    ret = mdio_cmd_write_22((void *)SWITCH_MDIO_BASE, addr, reg, val);

    sf2_mdio_master_enable();
    spin_unlock_bh(&mdio_access);

    return ret;
}
EXPORT_SYMBOL(mdio_write_c22_register);

int32_t mdio_read_c45_register(mdio_type_t type, uint32_t addr, uint32_t dev, uint16_t reg, uint16_t *val)
{
    int ret = MDIO_OK;

    spin_lock_bh(&mdio_access);
    sf2_mdio_master_disable();

    ret = mdio_cmd_read_45((void *)SWITCH_MDIO_BASE, addr, dev, reg, val);

    sf2_mdio_master_enable();
    spin_unlock_bh(&mdio_access);

    return ret;
}
EXPORT_SYMBOL(mdio_read_c45_register);

int32_t mdio_write_c45_register(mdio_type_t type, uint32_t addr, uint32_t dev, uint16_t reg, uint16_t val)
{
    int ret = MDIO_OK;

    spin_lock_bh(&mdio_access);
    sf2_mdio_master_disable();

    ret = mdio_cmd_write_45((void *)SWITCH_MDIO_BASE, addr, dev, reg, val);

    sf2_mdio_master_enable();
    spin_unlock_bh(&mdio_access);

    return ret;
}
EXPORT_SYMBOL(mdio_write_c45_register);
