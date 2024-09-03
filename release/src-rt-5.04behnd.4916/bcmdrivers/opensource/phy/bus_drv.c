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

#include "bus_drv.h"

extern bus_drv_t bus_drv_mdio_v1;
extern bus_drv_t bus_drv_i2c;
extern bus_drv_t bus_drv_sf2_ethsw;

bus_drv_t *bus_drv_get(bus_type_t bus_type)
{
    bus_drv_t *bus_drv = NULL;

    switch (bus_type)
    {
#ifdef BUS_MDIO_V1
    case BUS_TYPE_MDIO_V1:
        bus_drv = &bus_drv_mdio_v1;
        break;
#endif
       
#if defined(DSL_DEVICES)
#if defined(CONFIG_I2C) && !defined(CONFIG_I2C_GPIO)
    case BUS_TYPE_DSL_I2C:
        bus_drv = &bus_drv_i2c;
        break;
#endif
    case BUS_TYPE_DSL_ETHSW:
        bus_drv = &bus_drv_sf2_ethsw;
        break;
#endif
    default:
        break;
    }

    return bus_drv;
}
EXPORT_SYMBOL(bus_drv_get);

#define str(x) #x
#define match(x, _bus_type_str) do { \
    if (_bus_type_str && !strcmp(str(x) + strlen("BUS_TYPE_"), _bus_type_str)) \
        return bus_drv_get(x); \
    } while (0);

bus_drv_t *bus_drv_get_by_str(char *bus_type_str)
{
    match(BUS_TYPE_MDIO_V1, bus_type_str);
    match(BUS_TYPE_DSL_ETHSW, bus_type_str);
    match(BUS_TYPE_DSL_I2C, bus_type_str);

    return NULL;
}

