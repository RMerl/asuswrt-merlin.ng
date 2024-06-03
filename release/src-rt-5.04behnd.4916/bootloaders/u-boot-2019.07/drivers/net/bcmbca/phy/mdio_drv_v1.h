// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2016 Broadcom Corporation
   All Rights Reserved

    
*/

/*
 *  Created on: June 2017
 *      Author: dima.mamut@broadcom.com
 */

/*
 * MDIO driver for PON chips
 */

#ifndef __MDIO_DRV_V1_H__
#define __MDIO_DRV_V1_H__

#include "mdio_drv_common.h"

int32_t mdio_config_get(uint16_t *probe_mode, uint16_t *fast_mode);
int32_t mdio_config_set(uint16_t probe_mode, uint16_t fast_mode);
int32_t mdio_read_c22_register(uint32_t addr, uint32_t reg, uint16_t *val);
int32_t mdio_write_c22_register(uint32_t addr, uint32_t reg, uint16_t val);
int32_t mdio_read_c45_register(uint32_t addr, uint32_t dev, uint16_t reg, uint16_t *val);
int32_t mdio_write_c45_register(uint32_t addr, uint32_t dev, uint16_t reg, uint16_t val);

#endif	//__MDIO_DRV_V1_H__
