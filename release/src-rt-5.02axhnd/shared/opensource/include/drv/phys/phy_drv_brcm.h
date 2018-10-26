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
 *  Created on: Jan 2016
 *      Author: yuval.raviv@broadcom.com
 */

#ifndef __PHY_DRV_BRCM_H__
#define __PHY_DRV_BRCM_H__

#include "phy_drv.h"

#define RDB_ACCESS              0x8000

int brcm_read_status(phy_dev_t *phy_dev);

int brcm_shadow_read(phy_dev_t *phy_dev, int bank, uint16_t reg, uint16_t *val);
int brcm_shadow_write(phy_dev_t *phy_dev, int bank, uint16_t reg, uint16_t val);

int brcm_exp_read(phy_dev_t *phy_dev, uint16_t reg, uint16_t *val);
int brcm_exp_write(phy_dev_t *phy_dev, uint16_t reg, uint16_t val);

int brcm_rdb_set(phy_dev_t *phy_dev, int enable);
int brcm_rdb_read(phy_dev_t *phy_dev, uint16_t reg, uint16_t *val);
int brcm_rdb_write(phy_dev_t *phy_dev, uint16_t reg, uint16_t val);

int brcm_egphy_read(phy_dev_t *phy_dev, uint16_t reg, uint16_t *val);
int brcm_egphy_write(phy_dev_t *phy_dev, uint16_t reg, uint16_t val);

int brcm_egphy_force_auto_mdix_set(phy_dev_t *phy_dev, int enable);
int brcm_egphy_eth_wirespeed_set(phy_dev_t *phy_dev, int enable);

int brcm_egphy_apd_get(phy_dev_t *phy_dev, int *enable);
int brcm_egphy_apd_set(phy_dev_t *phy_dev, int enable);
int brcm_egphy_eee_get(phy_dev_t *phy_dev, int *enable);
int brcm_egphy_eee_set(phy_dev_t *phy_dev, int enable);
int brcm_egphy_eee_resolution_get(phy_dev_t *phy_dev, int *enable);

int brcm_ephy_apd_get(phy_dev_t *phy_dev, int *enable);
int brcm_ephy_apd_set(phy_dev_t *phy_dev, int enable);
int brcm_ephy_eee_get(phy_dev_t *phy_dev, int *enable);
int brcm_ephy_eee_set(phy_dev_t *phy_dev, int enable);
int brcm_ephy_eee_resolution_get(phy_dev_t *phy_dev, int *enable);

int brcm_shadow_18_read(phy_dev_t *phy_dev, uint16_t reg, uint16_t *val);
int brcm_shadow_18_write(phy_dev_t *phy_dev, uint16_t reg, uint16_t val);
int brcm_shadow_1c_read(phy_dev_t *phy_dev, uint16_t reg, uint16_t *val);
int brcm_shadow_1c_write(phy_dev_t *phy_dev, uint16_t reg, uint16_t val);

int brcm_shadow_18_force_auto_mdix_set(phy_dev_t *phy_dev, int enable);
int brcm_shadow_18_eth_wirespeed_set(phy_dev_t *phy_dev, int enable);
int brcm_shadow_1c_apd_get(phy_dev_t *phy_dev, int *enable);
int brcm_shadow_1c_apd_set(phy_dev_t *phy_dev, int enable);
int brcm_shadow_rgmii_init(phy_dev_t *phy_dev);

#endif

