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
 *  Created on: Dec 2015
 *      Author: yuval.raviv@broadcom.com
 */

#ifndef __BUS_DRV_H__
#define __BUS_DRV_H__

#include "os_dep.h"

typedef enum
{
    BUS_TYPE_UNKNOWN,
    BUS_TYPE_6838_EXT,
    BUS_TYPE_6838_EGPHY,
    BUS_TYPE_6838_SATA,
    BUS_TYPE_6838_AE,
    BUS_TYPE_6848_INT,
    BUS_TYPE_6848_EXT,
    BUS_TYPE_6858_LPORT,
    BUS_TYPE_6846_INT,
    BUS_TYPE_6846_EXT,
    BUS_TYPE_47189_GMAC0,
    BUS_TYPE_47189_GMAC1,
    BUS_TYPE_DSL_ETHSW,
    BUS_TYPE_DSL_I2C,
} bus_type_t;

typedef struct bus_drv_t
{
    int (*c22_read)(uint32_t addr, uint16_t reg, uint16_t *val);
    int (*c22_write)(uint32_t addr, uint16_t reg, uint16_t val);
    int (*c45_read)(uint32_t addr, uint16_t dev, uint16_t reg, uint16_t *val);
    int (*c45_write)(uint32_t addr, uint16_t dev, uint16_t reg, uint16_t val);
    bus_type_t bus_type;
} bus_drv_t;

bus_drv_t *bus_drv_get(bus_type_t bus_type);

static inline int bus_read(bus_drv_t *bus_drv, uint32_t addr, uint16_t reg, uint16_t *val)
{
    return bus_drv->c22_read(addr, reg, val);
}

static inline int bus_write(bus_drv_t *bus_drv, uint32_t addr, uint16_t reg, uint16_t val)
{
    return bus_drv->c22_write(addr, reg, val);
}

static inline int bus_c45_read(bus_drv_t *bus_drv, uint32_t addr, uint16_t dev, uint16_t reg, uint16_t *val)
{
    return bus_drv->c45_read(addr, dev, reg, val);
}

static inline int bus_c45_write(bus_drv_t *bus_drv, uint32_t addr, uint16_t dev, uint16_t reg, uint16_t val)
{
    return bus_drv->c45_write(addr, dev, reg, val);
}

#endif
