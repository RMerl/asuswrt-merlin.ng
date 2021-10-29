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
 *  Created on: Nov 2016
 *      Author: steven.hsieh@broadcom.com
 */

#include "bus_drv.h"
#include "mdio_drv_sf2.h"
#ifndef _CFE_
#ifndef _UBOOT_
#include "linux/kernel.h"
#include "linux/spinlock.h"
#endif
#include "bcmnet.h"
#ifndef _UBOOT_
#include "bcmsfp_i2c.h"
#endif
#endif

/* SF2 MDIO bus */
static int bus_sf2_ethsw_c22_read(uint32_t addr, uint16_t reg, uint16_t *val)
{
    return mdio_read_c22_register(MDIO_SF2, addr, reg, val);
}

static int bus_sf2_ethsw_c22_write(uint32_t addr, uint16_t reg, uint16_t val)
{
    return mdio_write_c22_register(MDIO_SF2, addr, reg, val);
}

static int bus_sf2_ethsw_c45_read(uint32_t addr, uint16_t dev, uint16_t reg, uint16_t *val)
{
    return mdio_read_c45_register(MDIO_SF2, addr, dev, reg, val);
}

static int bus_sf2_ethsw_c45_write(uint32_t addr, uint16_t dev, uint16_t reg, uint16_t val)
{
    return mdio_write_c45_register(MDIO_SF2, addr, dev, reg, val);
}

bus_drv_t bus_sf2_ethsw_drv =
{
    .c22_read  = bus_sf2_ethsw_c22_read,
    .c22_write = bus_sf2_ethsw_c22_write,
    .c45_read  = bus_sf2_ethsw_c45_read,
    .c45_write = bus_sf2_ethsw_c45_write,
};

#if defined(CONFIG_I2C)
static int _sfp_i2c_phy_read(uint32_t addr, uint16_t reg,  uint16_t *val)
{
    return bcmsfp_read_word(addr, SFP_CLIENT_PHY, reg, val);
}
    
static int _sfp_i2c_phy_write(uint32_t addr, uint16_t reg,  uint16_t val)
{
    return bcmsfp_write_word(addr, SFP_CLIENT_PHY, reg, val);
}
   
bus_drv_t bus_i2c_drv =
{
    .c22_read = _sfp_i2c_phy_read,
    .c22_write = _sfp_i2c_phy_write,
    .bus_type = BUS_TYPE_DSL_I2C,
};
#endif /* CONFIG_I2C */
