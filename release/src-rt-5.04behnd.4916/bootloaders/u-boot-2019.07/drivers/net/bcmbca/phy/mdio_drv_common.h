// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2016 Broadcom Corporation
   All Rights Reserved


*/

/*
 *  Created on: Aug 2016
 *      Author: yuval.raviv@broadcom.com
 */

/*
 * MDIO common functions
 */

#ifndef __MDIO_DRV_COMMON_H__
#define __MDIO_DRV_COMMON_H__

#include "os_dep.h"

#define MDIO_ERROR  -1
#define MDIO_OK      0

int32_t mdio_cfg_get(uint32_t *p, uint16_t *probe_mode, uint16_t *mdio_clock_divider);
int32_t mdio_cfg_set(uint32_t *p, uint16_t probe_mode, uint16_t mdio_clock_divider);
int32_t mdio_cmd_read_22(uint32_t *p, uint32_t addr, uint32_t reg, uint16_t *val);
int32_t mdio_cmd_write_22(uint32_t *p, uint32_t addr, uint32_t reg, uint16_t val);
int32_t mdio_cmd_read_45(uint32_t *p, uint32_t addr, uint32_t dev, uint16_t reg, uint16_t *val);
int32_t mdio_cmd_write_45(uint32_t *p, uint32_t addr, uint32_t dev, uint16_t reg, uint16_t val);
int32_t mdio_comp_read_c45_register(uint32_t addr, uint32_t dev, uint16_t reg, uint16_t *val, int regIn[], int regOut[]);
int32_t mdio_comp_write_c45_register(uint32_t addr, uint32_t dev, uint16_t reg, uint16_t val, int regIn[], int RegOut[]);

#if defined(DSL_DEVICES)
#define FAST_CLOCK_DIVIDER 9
#else
#define FAST_CLOCK_DIVIDER 4
#endif

#endif
