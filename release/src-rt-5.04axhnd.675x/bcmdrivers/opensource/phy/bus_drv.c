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

extern bus_drv_t bus_6858_lport_drv;
extern bus_drv_t bus_mdio_v1_int_drv;
extern bus_drv_t bus_mdio_v1_ext_drv;
extern bus_drv_t bus_i2c_drv;
extern bus_drv_t bus_sf2_ethsw_drv;
extern bus_drv_t bus_sf2_c45_drv;
int bus_probe_mode;

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
       
#if defined(DSL_DEVICES)
#if defined(CONFIG_I2C) && !defined(CONFIG_I2C_GPIO)
    case BUS_TYPE_DSL_I2C:
        bus_drv = &bus_i2c_drv;
        break;
#endif
    case BUS_TYPE_DSL_ETHSW:
        bus_drv = &bus_sf2_ethsw_drv;
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
    match(BUS_TYPE_DSL_ETHSW, bus_type_str);
    match(BUS_TYPE_DSL_I2C, bus_type_str);

    return NULL;
}

