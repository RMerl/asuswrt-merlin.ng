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
static int _mdio_cfg_get(bus_cfg_t *bus_cfg)
{
    return mdio_config_get(&bus_cfg->probe_mode, &bus_cfg->fast_mode);
}

static int _mdio_cfg_set(bus_cfg_t *bus_cfg)
{
    return mdio_config_set(bus_cfg->probe_mode, bus_cfg->fast_mode);
}

static int _mdio_c22_read(uint32_t addr, uint16_t reg, uint16_t *val)
{
    return mdio_read_c22_register(addr, reg, val);
}

static int _mdio_c22_write(uint32_t addr, uint16_t reg, uint16_t val)
{
    return mdio_write_c22_register(addr, reg, val);
}

static int _mdio_c45_read(uint32_t addr, uint16_t dev, uint16_t reg, uint16_t *val)
{
    return mdio_read_c45_register(addr, dev, reg, val);
}

static int _mdio_c45_write(uint32_t addr, uint16_t dev, uint16_t reg, uint16_t val)
{
    return mdio_write_c45_register(addr, dev, reg, val);
}

static int _mdio_c45_comp_read(uint32_t addr, uint16_t dev, uint16_t reg, uint16_t *val, int regIn[], int regOut[])
{
    return mdio_comp_read_c45_register(addr, dev, reg, val, regIn, regOut);
}

static int _mdio_c45_comp_write(uint32_t addr, uint16_t dev, uint16_t reg, uint16_t val, int regIn[], int regOut[])
{
    return mdio_comp_write_c45_register(addr, dev, reg, val, regIn, regOut);
}

bus_drv_t bus_drv_sf2_ethsw =
{
    .cfg_get = _mdio_cfg_get,
    .cfg_set = _mdio_cfg_set,
    .c22_read  = _mdio_c22_read,
    .c22_write = _mdio_c22_write,
    .c45_read  = _mdio_c45_read,
    .c45_write = _mdio_c45_write,
    .c45_comp_read = _mdio_c45_comp_read,
    .c45_comp_write = _mdio_c45_comp_write,
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
   
bus_drv_t bus_drv_i2c =
{
    .c22_read = _sfp_i2c_phy_read,
    .c22_write = _sfp_i2c_phy_write,
    .bus_type = BUS_TYPE_DSL_I2C,
};
#endif /* CONFIG_I2C */
