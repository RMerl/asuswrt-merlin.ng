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

extern bus_drv_t bus_6858_lport_drv;
extern bus_drv_t bus_mdio_v1_int_drv;
extern bus_drv_t bus_mdio_v1_ext_drv;
extern bus_drv_t bus_47189_gmac0_drv;
extern bus_drv_t bus_47189_gmac1_drv;
extern bus_drv_t bus_i2c_drv;
extern bus_drv_t bus_sf2_ethsw_drv;
extern bus_drv_t bus_sf2_c45_drv;

bus_drv_t *bus_drv_get(bus_type_t bus_type)
{
    bus_drv_t *bus_drv = NULL;

    switch (bus_type)
    {
#ifdef CONFIG_BCM96858
    case BUS_TYPE_6858_LPORT:
        bus_drv = &bus_6858_lport_drv;
        break;
#endif
#if defined(CONFIG_BCM96846) || defined(CONFIG_BCM96856) || defined(CONFIG_BCM96878) || defined(CONFIG_BCM96855)   
    case BUS_TYPE_MDIO_V1_INT:
        bus_drv = &bus_mdio_v1_int_drv;
        break;
    case BUS_TYPE_MDIO_V1_EXT:
        bus_drv = &bus_mdio_v1_ext_drv;
        break;
#endif
       
#ifdef CONFIG_BCM947189
    case BUS_TYPE_47189_GMAC0:
        bus_drv = &bus_47189_gmac0_drv;
        break;
    case BUS_TYPE_47189_GMAC1:
        bus_drv = &bus_47189_gmac1_drv;
        break;
#endif
#if defined(DSL_DEVICES)
#if defined(CONFIG_I2C) && !defined(CONFIG_I2C_GPIO)
    case BUS_TYPE_DSL_I2C:
        bus_drv = &bus_i2c_drv;
        break;
#endif
    case BUS_TYPE_DSL_ETHSW:
#if defined(CONFIG_BCM63146) || defined(CONFIG_BCM4912) || defined(CONFIG_BCM6813)
        bus_drv = &bus_mdio_v1_int_drv;
#else
        bus_drv = &bus_sf2_ethsw_drv;
#endif
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
    if (!strcmp(str(x) + strlen("BUS_TYPE_"), _bus_type_str)) \
        return bus_drv_get(x); \
    } while (0);

bus_drv_t *bus_drv_get_by_str(char *bus_type_str)
{
    match(BUS_TYPE_6858_LPORT, bus_type_str);
    match(BUS_TYPE_MDIO_V1_INT, bus_type_str);
    match(BUS_TYPE_MDIO_V1_EXT, bus_type_str);
    match(BUS_TYPE_47189_GMAC0, bus_type_str);
    match(BUS_TYPE_47189_GMAC1, bus_type_str);
    match(BUS_TYPE_DSL_ETHSW, bus_type_str);
    match(BUS_TYPE_DSL_I2C, bus_type_str);

    return NULL;
}

