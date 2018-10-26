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

#include "bus_drv.h"
#include "mdio_drv_impl2.h"

/* Internal MDIO bus */
static int bus_6848_int_c22_read(uint32_t addr, uint16_t reg, uint16_t *val)
{
    return mdio_read_c22_register(MDIO_INT, addr, reg, val);
}

static int bus_6848_int_c22_write(uint32_t addr, uint16_t reg, uint16_t val)
{
    return mdio_write_c22_register(MDIO_INT, addr, reg, val);
}

static int bus_6848_int_c45_read(uint32_t addr, uint16_t dev, uint16_t reg, uint16_t *val)
{
    return mdio_read_c45_register(MDIO_INT, addr, dev, reg, val);
}

static int bus_6848_int_c45_write(uint32_t addr, uint16_t dev, uint16_t reg, uint16_t val)
{
    return mdio_write_c45_register(MDIO_INT, addr, dev, reg, val);
}

bus_drv_t bus_6848_int_drv =
{
    .c22_read = bus_6848_int_c22_read,
    .c22_write = bus_6848_int_c22_write,
    .c45_read = bus_6848_int_c45_read,
    .c45_write = bus_6848_int_c45_write,
};


/* External MDIO bus */
static int bus_6848_ext_c22_read(uint32_t addr, uint16_t reg, uint16_t *val)
{
    return mdio_read_c22_register(MDIO_EXT, addr, reg, val);
}

static int bus_6848_ext_c22_write(uint32_t addr, uint16_t reg, uint16_t val)
{
    return mdio_write_c22_register(MDIO_EXT, addr, reg, val);
}

static int bus_6848_ext_c45_read(uint32_t addr, uint16_t dev, uint16_t reg, uint16_t *val)
{
    return mdio_read_c45_register(MDIO_EXT, addr, dev, reg, val);
}

static int bus_6848_ext_c45_write(uint32_t addr, uint16_t dev, uint16_t reg, uint16_t val)
{
    return mdio_write_c45_register(MDIO_EXT, addr, dev, reg, val);
}

bus_drv_t bus_6848_ext_drv =
{
    .c22_read = bus_6848_ext_c22_read,
    .c22_write = bus_6848_ext_c22_write,
    .c45_read = bus_6848_ext_c45_read,
    .c45_write = bus_6848_ext_c45_write,
};
