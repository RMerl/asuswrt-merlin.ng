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
    BUS_TYPE_MDIO_V1,
    BUS_TYPE_DSL_ETHSW,
    BUS_TYPE_DSL_I2C,
} bus_type_t;

typedef struct bus_cfg_t
{
    uint16_t probe_mode;
    uint16_t fast_mode;
} bus_cfg_t;

typedef struct bus_drv_t
{
    int (*cfg_get)(bus_cfg_t *bus_cfg);
    int (*cfg_set)(bus_cfg_t *bus_cfg);
    int (*c22_read)(uint32_t addr, uint16_t reg, uint16_t *val);
    int (*c22_write)(uint32_t addr, uint16_t reg, uint16_t val);
    int (*c45_read)(uint32_t addr, uint16_t dev, uint16_t reg, uint16_t *val);
    int (*c45_write)(uint32_t addr, uint16_t dev, uint16_t reg, uint16_t val);
    int (*c45_comp_read)(uint32_t addr, uint16_t dev, uint16_t reg, uint16_t *val, int regIn[], int regOut[]);
    int (*c45_comp_write)(uint32_t addr, uint16_t dev, uint16_t reg, uint16_t val, int regIn[], int regOut[]);
    bus_type_t bus_type;
} bus_drv_t;

bus_drv_t *bus_drv_get(bus_type_t bus_type);

static inline int bus_cfg_get(bus_drv_t *bus_drv, bus_cfg_t *bus_cfg)
{
    if (!bus_drv->cfg_get)
        return 0;

    return bus_drv->cfg_get(bus_cfg);
}

static inline int bus_cfg_set(bus_drv_t *bus_drv, bus_cfg_t *bus_cfg)
{
    if (!bus_drv->cfg_set)
        return 0;

    return bus_drv->cfg_set(bus_cfg);
}

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
