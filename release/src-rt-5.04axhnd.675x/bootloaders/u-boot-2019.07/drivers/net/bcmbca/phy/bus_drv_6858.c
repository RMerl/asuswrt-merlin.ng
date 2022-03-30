// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    
*/

/*
 *  Created on: Dec 2015
 *      Author: yuval.raviv@broadcom.com
 */

#include "bus_drv.h"
#include "lport_mdio.h"

/* LPORT MDIO bus */
static int bus_6858_lport_c22_read(uint32_t addr, uint16_t reg, uint16_t *val)
{
    return lport_mdio22_rd(addr, reg, val);
}

static int bus_6858_lport_c22_write(uint32_t addr, uint16_t reg, uint16_t val)
{
    return lport_mdio22_wr(addr, reg, val);
}

static int bus_6858_lport_c45_read(uint32_t addr, uint16_t dev, uint16_t reg, uint16_t *val)
{
    return lport_mdio45_rd(addr, dev, reg, val);
}

static int bus_6858_lport_c45_write(uint32_t addr, uint16_t dev, uint16_t reg, uint16_t val)
{
    return lport_mdio45_wr(addr, dev, reg, val);
}

bus_drv_t bus_6858_lport_drv =
{
    .c22_read = bus_6858_lport_c22_read,
    .c22_write = bus_6858_lport_c22_write,
    .c45_read = bus_6858_lport_c45_read,
    .c45_write = bus_6858_lport_c45_write,
};
