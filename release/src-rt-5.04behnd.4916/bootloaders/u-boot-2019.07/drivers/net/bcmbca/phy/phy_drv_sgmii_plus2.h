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
 * Common functions for SGMIIPLUS2 core
 */

#ifndef __PHY_DRV_SGMII_PLUS2_H__
#define __PHY_DRV_SGMII_PLUS2_H__

#include "phy_drv.h"

typedef enum
{
    SGMII_REFCLK_UNKNOWN,
    SGMII_REFCLK_50,
    SGMII_REFCLK_80,
    SGMII_REFCLK_125,
} sgmii_refclk_t;

int sgmii_read(phy_dev_t *phy_dev, uint16_t reg, uint16_t *val);
int sgmii_write(phy_dev_t *phy_dev, uint16_t reg, uint16_t val);

int sgmii_speed_set(phy_dev_t *phy_dev, phy_speed_t speed);
int sgmii_refclk_init(phy_dev_t *phy_dev, sgmii_refclk_t refclk);
int sgmii_read_status(phy_dev_t *phy_dev);

#endif

