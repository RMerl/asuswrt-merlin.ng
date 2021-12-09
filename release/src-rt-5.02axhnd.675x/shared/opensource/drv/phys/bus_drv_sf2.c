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
 *  Created on: Nov 2016
 *      Author: steven.hsieh@broadcom.com
 */

#include "bus_drv.h"
#include "mdio_drv_sf2.h"
#ifndef _UBOOT_
#include "linux/kernel.h"
#include "linux/spinlock.h"
#endif
#include "bcmnet.h"
#ifndef _UBOOT_
#include "bcmsfp_i2c.h"
#endif

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
