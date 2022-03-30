// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

*/

/*
 *  Created on: Nov 2016
 *      Author: steven.hsieh@broadcom.com
 */

#include "bus_drv.h"
#include "mdio_drv_sf2.h"
#include "linux/kernel.h"

/* SF2 MDIO bus */
int bus_sf2_ethsw_c22_read(uint32_t addr, uint16_t reg, uint16_t *val)
{
    return mdio_read_c22_register(MDIO_SF2, addr, reg, val);
}

int bus_sf2_ethsw_c22_write(uint32_t addr, uint16_t reg, uint16_t val)
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

#if defined(CONFIG_I2C) && !defined(CONFIG_I2C_GPIO)
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
