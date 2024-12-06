/*
   Copyright (c) 2016 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2016:DUAL/GPL:standard

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
 *  Created on: Aug 2017
 *      Author: dima.mamut@broadcom.com
 */

#include "bus_drv.h"
#include "mdio_drv_v1.h"

static int _mdio_cfg_get(bus_cfg_t *bus_cfg)
{
    return mdio_config_get(&bus_cfg->probe_mode, &bus_cfg->fast_mode);
}

static int _mdio_cfg_set(bus_cfg_t *bus_cfg)
{
    return mdio_config_set(bus_cfg->probe_mode, bus_cfg->fast_mode);
}

static int _mdio_c22_read(uint32_t addr, uint16_t reg, uint16_t *val)
{
    return mdio_read_c22_register(addr, reg, val);
}

static int _mdio_c22_write(uint32_t addr, uint16_t reg, uint16_t val)
{
    return mdio_write_c22_register(addr, reg, val);
}

static int _mdio_c45_read(uint32_t addr, uint16_t dev, uint16_t reg, uint16_t *val)
{
    return mdio_read_c45_register(addr, dev, reg, val);
}

static int _mdio_c45_write(uint32_t addr, uint16_t dev, uint16_t reg, uint16_t val)
{
    return mdio_write_c45_register(addr, dev, reg, val);
}

static int _mdio_c45_comp_read(uint32_t addr, uint16_t dev, uint16_t reg, uint16_t *val, int regIn[], int regOut[])
{
    return mdio_comp_read_c45_register(addr, dev, reg, val, regIn, regOut);
}

static int _mdio_c45_comp_write(uint32_t addr, uint16_t dev, uint16_t reg, uint16_t val, int regIn[], int regOut[])
{
    return mdio_comp_write_c45_register(addr, dev, reg, val, regIn, regOut);
}

bus_drv_t bus_drv_mdio_v1 =
{
    .cfg_get = _mdio_cfg_get,
    .cfg_set = _mdio_cfg_set,
    .c22_read = _mdio_c22_read,
    .c22_write = _mdio_c22_write,
    .c45_read = _mdio_c45_read,
    .c45_write = _mdio_c45_write,
    .c45_comp_read = _mdio_c45_comp_read,
    .c45_comp_write = _mdio_c45_comp_write,
};

