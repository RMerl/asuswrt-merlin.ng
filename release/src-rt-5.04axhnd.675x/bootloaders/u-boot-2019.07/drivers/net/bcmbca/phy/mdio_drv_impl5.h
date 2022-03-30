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
 * MDIO driver for BCM96846 ,BCM96878, BCM96856 and BCM96855 
 */

#ifndef __MDIO_DRV_IMPL5_H__
#define __MDIO_DRV_IMPL5_H__

#include "mdio_drv_common.h"

typedef enum
{
    MDIO_EXT = 0,
    MDIO_INT = 1,
} mdio_type_t;

int32_t mdio_read_c22_register(mdio_type_t type, uint32_t addr, uint32_t reg, uint16_t *val);
int32_t mdio_write_c22_register(mdio_type_t type, uint32_t addr, uint32_t reg, uint16_t val);
int32_t mdio_read_c45_register(mdio_type_t type, uint32_t addr, uint32_t dev, uint16_t reg, uint16_t *val);
int32_t mdio_write_c45_register(mdio_type_t type, uint32_t addr, uint32_t dev, uint16_t reg, uint16_t val);

#endif	//__MDIO_DRV_IMPL5_H__
