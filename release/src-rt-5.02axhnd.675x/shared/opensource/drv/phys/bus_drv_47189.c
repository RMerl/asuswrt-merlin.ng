/*
   <:copyright-BRCM:2016:DUAL/GPL:standard
   
      Copyright (c) 2016 Broadcom 
      All Rights Reserved
   
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
 * Bus driver for BCM47189. Implements all c22 and c45 operations over MDIO
 * (GMAC).
 * This includes two classes:
 *   - bus_47189_gmac0_drv: for MDIO access through GMAC0
 *   - bus_47189_gmac0_drv: for MDIO access through GMAC1
 *
 * They are both identical except for the target GMAC.
 *
 * NOTE: Possible future improvement
 * An additional private data field in the bus_drv_t class could encode the GMAC
 * core number so that one single class definition can be used for both cores.
 */

#include "bus_drv.h"
#include "mdio_drv_impl3.h"

static int bus_47189_gmac0_c22_read(uint32_t addr, uint16_t reg, uint16_t *val)
{
    return mdio_read_c22_register(0, addr, reg, val);
}

static int bus_47189_gmac0_c22_write(uint32_t addr, uint16_t reg, uint16_t val)
{
    return mdio_write_c22_register(0, addr, reg, val);
}

static int bus_47189_gmac0_c45_read(uint32_t addr, uint16_t dev, uint16_t reg, uint16_t *val)
{
    return mdio_read_c45_register(0, addr, dev, reg, val);
}

static int bus_47189_gmac0_c45_write(uint32_t addr, uint16_t dev, uint16_t reg, uint16_t val)
{
    return mdio_write_c45_register(0, addr, dev, reg, val);
}

bus_drv_t bus_47189_gmac0_drv =
{
    .c22_read = bus_47189_gmac0_c22_read,
    .c22_write = bus_47189_gmac0_c22_write,
    .c45_read = bus_47189_gmac0_c45_read,
    .c45_write = bus_47189_gmac0_c45_write,
};


static int bus_47189_gmac1_c22_read(uint32_t addr, uint16_t reg, uint16_t *val)
{
    return mdio_read_c22_register(1, addr, reg, val);
}

static int bus_47189_gmac1_c22_write(uint32_t addr, uint16_t reg, uint16_t val)
{
    return mdio_write_c22_register(1, addr, reg, val);
}

static int bus_47189_gmac1_c45_read(uint32_t addr, uint16_t dev, uint16_t reg, uint16_t *val)
{
    return mdio_read_c45_register(1, addr, dev, reg, val);
}

static int bus_47189_gmac1_c45_write(uint32_t addr, uint16_t dev, uint16_t reg, uint16_t val)
{
    return mdio_write_c45_register(1, addr, dev, reg, val);
}

bus_drv_t bus_47189_gmac1_drv =
{
    .c22_read = bus_47189_gmac1_c22_read,
    .c22_write = bus_47189_gmac1_c22_write,
    .c45_read = bus_47189_gmac1_c45_read,
    .c45_write = bus_47189_gmac1_c45_write,
};
