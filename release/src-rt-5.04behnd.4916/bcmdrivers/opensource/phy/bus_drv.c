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

