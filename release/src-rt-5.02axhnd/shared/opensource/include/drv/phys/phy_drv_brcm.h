/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2, as published by
    the Free Software Foundation (the "GPL").
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    
    A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
    writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    
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

