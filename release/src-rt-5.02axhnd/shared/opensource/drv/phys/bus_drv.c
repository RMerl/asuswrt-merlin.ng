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

extern bus_drv_t bus_6838_ext_drv;
extern bus_drv_t bus_6838_egphy_drv;
extern bus_drv_t bus_6838_sata_drv;
extern bus_drv_t bus_6838_ae_drv;
extern bus_drv_t bus_6848_int_drv;
extern bus_drv_t bus_6848_ext_drv;
extern bus_drv_t bus_6858_lport_drv;
extern bus_drv_t bus_6846_int_drv;
extern bus_drv_t bus_6846_ext_drv;
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
#if defined(CONFIG_BCM96846) || defined(CONFIG_BCM96856)        
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
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM963158) || defined(CONFIG_BCM94908) || (defined(_BCM94908_) && defined(CFG_2P5G_LAN))
#if defined(CONFIG_I2C)
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
