// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    
*/

/*
 *  Created on: Jan 2016
 *      Author: yuval.raviv@broadcom.com
 */

#ifndef __PHY_DRV_BRCM_H__
#define __PHY_DRV_BRCM_H__

#include "phy_drv.h"

#define RDB_ACCESS              0x8000

#define MII_EXPAND_REG_REG      0x17
    #define MII_EXPAND_REG_MARK     (0xf<<8)
#define MII_EXPAND_REG_VAL      0x15

#define MII_ECD_CTRL_STATUS     0xc0
    #define ECD_RUN_IMMEDIATE   (1<<15)
    #define ECD_BREAK_LINK      (1<<12)
    #define ECD_DIAG_IN_PROG    (1<<11)

#define MII_ECD_CTRL_FAULT_TYPE 0xc1
    #define PA_CD_CODE_SHIFT            4
    #define PA_CD_CODE_MASK             0xf
    #define PA_CD_CODE_INVALID          0x0
    #define PA_CD_CODE_PAIR_OK          0x1
    #define PA_CD_CODE_PAIR_OPEN        0x2
    #define PA_CD_CODE_PAIR_INTRA_SHORT 0x3
    #define PA_CD_CODE_PAIR_INTER_SHORT 0x4
    #define PA_CD_CODE_PAIR_GET(v, p)          (((v)>>((p)*4))&0xf)
    #define PA_CD_CODE_PAIR_SET(v, p)          (((v)&0xf)<<((p)*4))
    #define PA_CD_CODE_PAIR_ALL_OK      0x1111
    #define PA_CD_CODE_PAIR_ALL_OPEN    0x2222

#define MII_ECD_CABLE_LEN 0xc2

int brcm_cable_diag_run(phy_dev_t *phy_dev, int *result, int *pair_len);
int brcm_read_status(phy_dev_t *phy_dev);
int brcm_read_status_rgmii_ib_override(phy_dev_t *phy_dev);

int brcm_shadow_read(phy_dev_t *phy_dev, int bank, uint16_t reg, uint16_t *val);
int brcm_shadow_write(phy_dev_t *phy_dev, int bank, uint16_t reg, uint16_t val);

int brcm_exp_read(phy_dev_t *phy_dev, uint16_t reg, uint16_t *val);
int brcm_exp_write(phy_dev_t *phy_dev, uint16_t reg, uint16_t val);

int brcm_misc_read(phy_dev_t *phy_dev, uint16_t reg, int chnl, uint16_t *val);
int brcm_misc_write(phy_dev_t *phy_dev, uint16_t reg, int chnl, uint16_t val);

int brcm_rdb_set(phy_dev_t *phy_dev, int enable);
int brcm_rdb_read(phy_dev_t *phy_dev, uint16_t reg, uint16_t *val);
int brcm_rdb_write(phy_dev_t *phy_dev, uint16_t reg, uint16_t val);

int brcm_egphy_read(phy_dev_t *phy_dev, uint16_t reg, uint16_t *val);
int brcm_egphy_write(phy_dev_t *phy_dev, uint16_t reg, uint16_t val);

int brcm_egphy_force_auto_mdix_set(phy_dev_t *phy_dev, int enable);
int brcm_egphy_force_auto_mdix_get(phy_dev_t *phy_dev, int *enable);

int brcm_egphy_eth_wirespeed_get(phy_dev_t *phy_dev, int *enable);
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
int brcm_shadow_18_force_auto_mdix_get(phy_dev_t *phy_dev, int *enable);

int brcm_shadow_18_eth_wirespeed_get(phy_dev_t *phy_dev, int *enable);
int brcm_shadow_18_eth_wirespeed_set(phy_dev_t *phy_dev, int enable);

int brcm_shadow_1c_apd_get(phy_dev_t *phy_dev, int *enable);
int brcm_shadow_1c_apd_set(phy_dev_t *phy_dev, int enable);
int brcm_shadow_rgmii_init(phy_dev_t *phy_dev);

int brcm_loopback_set(phy_dev_t *phy_dev, int enable, phy_speed_t speed);
int brcm_loopback_get(phy_dev_t *phy_dev, int *enable, phy_speed_t *speed);


#define IsC45Phy(phy) (phy->phy_drv->phy_type == PHY_TYPE_EXT3)

static inline int phy_bus_c45_read32(phy_dev_t *phy_dev, uint32_t reg32, uint16_t *val_p)
{
    return phy_bus_c45_read(phy_dev, ((reg32)>>16)&0xffff, reg32&0xffff, val_p) +
        phy_bus_c45_read(phy_dev, ((reg32)>>16)&0xffff, reg32&0xffff, val_p);
}
#define phy_bus_c45_write32(phy_dev, reg32, val) \
    phy_bus_c45_write(phy_dev, ((reg32)>>16)&0xffff, reg32&0xffff, val)

int ethsw_phy_exp_rw(phy_dev_t *phy_dev, uint32_t reg, uint16_t *v16_p, int rd);

static inline int ethsw_phy_exp_read(phy_dev_t *phy_dev, uint32_t reg, uint16_t *v16_p)
{
    return ethsw_phy_exp_rw(phy_dev, reg, v16_p, 1);
}

static inline int ethsw_phy_exp_write(phy_dev_t *phy_dev, uint32_t reg, uint16_t v16)
{
    return ethsw_phy_exp_rw(phy_dev, reg, &v16, 0); 
}

#endif

