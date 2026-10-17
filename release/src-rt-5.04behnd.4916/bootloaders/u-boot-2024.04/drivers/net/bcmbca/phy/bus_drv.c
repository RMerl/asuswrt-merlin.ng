// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2024 Broadcom Corporation
   All Rights Reserved
*/

/*
 *  Created on: Dec 2015
 *      Author: yuval.raviv@broadcom.com
 */

#include "bus_drv.h"

extern bus_drv_t bus_drv_sf2_ethsw;

extern bus_drv_t bus_drv_mdio_v1;
extern bus_drv_t *get_bus_drv_i2c(void);
int bus_probe_mode;

bus_drv_t *bus_drv_get(bus_type_t bus_type)
{
    bus_drv_t *bus_drv = NULL;

    switch (bus_type)
    {
#ifdef BUS_MDIO_V1
    case BUS_TYPE_MDIO_V1:
    case BUS_TYPE_DSL_ETHSW:
        bus_drv = &bus_drv_mdio_v1;
        break;
#endif
       
#if defined(DSL_DEVICES)
#if defined(CONFIG_I2C) && !defined(CONFIG_I2C_GPIO)
    case BUS_TYPE_DSL_I2C:
#if 1
	printf("revisit %s:%d\n", __FILE__,__LINE__);
#else
        bus_drv = get_bus_drv_i2c();
#endif
        break;
#endif
#ifndef BUS_MDIO_V1
    case BUS_TYPE_DSL_ETHSW:
        bus_drv = &bus_drv_sf2_ethsw;
        break;
#endif
#endif
    default:
        break;
    }

    return bus_drv;
}
EXPORT_SYMBOL(bus_drv_get);

#define str(x) #x
#define match(x, _bus_type_str) do { \
    if (!strcmp(str(x) + strlen("BUS_TYPE_"), _bus_type_str)) \
        return bus_drv_get(x); \
    } while (0);

bus_drv_t *bus_drv_get_by_str(char *bus_type_str)
{
    match(BUS_TYPE_MDIO_V1, bus_type_str);
    match(BUS_TYPE_DSL_ETHSW, bus_type_str);
    match(BUS_TYPE_DSL_I2C, bus_type_str);

    return NULL;
}

