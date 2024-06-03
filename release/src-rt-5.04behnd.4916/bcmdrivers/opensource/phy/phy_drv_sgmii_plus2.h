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

