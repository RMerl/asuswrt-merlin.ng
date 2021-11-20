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
