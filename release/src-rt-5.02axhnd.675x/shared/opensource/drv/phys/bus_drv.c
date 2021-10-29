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

extern bus_drv_t bus_6838_ext_drv;
extern bus_drv_t bus_6838_egphy_drv;
extern bus_drv_t bus_6838_sata_drv;
extern bus_drv_t bus_6838_ae_drv;
extern bus_drv_t bus_6848_int_drv;
extern bus_drv_t bus_6848_ext_drv;
extern bus_drv_t bus_6858_lport_drv;
extern bus_drv_t bus_6846_int_drv;
extern bus_drv_t bus_6846_ext_drv;
extern bus_drv_t bus_6878_int_drv;
extern bus_drv_t bus_6878_ext_drv;
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
#ifdef CONFIG_BCM96838
    case BUS_TYPE_6838_EXT:
        bus_drv = &bus_6838_ext_drv;
        break;
    case BUS_TYPE_6838_EGPHY:
        bus_drv = &bus_6838_egphy_drv;
        break;
    case BUS_TYPE_6838_SATA:
        bus_drv = &bus_6838_sata_drv;
        break;
    case BUS_TYPE_6838_AE:
        bus_drv = &bus_6838_ae_drv;
        break;
#endif
#ifdef CONFIG_BCM96848
    case BUS_TYPE_6848_INT:
        bus_drv = &bus_6848_int_drv;
        break;
    case BUS_TYPE_6848_EXT:
        bus_drv = &bus_6848_ext_drv;
        break;
#endif
#ifdef CONFIG_BCM96858
    case BUS_TYPE_6858_LPORT:
        bus_drv = &bus_6858_lport_drv;
        break;
#endif
#if defined(CONFIG_BCM96846) || defined(CONFIG_BCM96856) || defined(CONFIG_BCM96878)     
    case BUS_TYPE_6846_INT:
        bus_drv = &bus_6846_int_drv;
        break;
    case BUS_TYPE_6846_EXT:
        bus_drv = &bus_6846_ext_drv;
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
        bus_drv = &bus_sf2_ethsw_drv;
        break;
#endif
    default:
        break;
    }

    return bus_drv;
}
EXPORT_SYMBOL(bus_drv_get);
