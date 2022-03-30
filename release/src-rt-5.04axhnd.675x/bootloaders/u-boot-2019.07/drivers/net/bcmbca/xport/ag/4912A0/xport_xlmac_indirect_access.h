// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2017 Broadcom Corporation
   All Rights Reserved

   */
/*
 * xport_xlmac_indirect_access.h
 *
 */

#ifndef SHARED_OPENSOURCE_DRV_XPORT_XLMAC_INDIRECT_ACCESS_H_
#define SHARED_OPENSOURCE_DRV_XPORT_XLMAC_INDIRECT_ACCESS_H_

#include "ru_types.h"
#include "bcm4912_xport_xlmac_reg_ag.h"
#include "bcm4912_xport_mib_reg_ag.h"

#define XLMAC_NUM_S 2
#define XPORT_XLMAC_INDIRECT_WRITE(i,b,r,v) xport_xlmac_indirect_write(0, i>>XLMAC_NUM_S, RU_BLK(b).addr[i] + RU_REG_OFFSET(b,r),v)
#define XPORT_XLMAC_INDIRECT_READ(i,b,r,v) xport_xlmac_indirect_read(0, i>>XLMAC_NUM_S, RU_BLK(b).addr[i] + RU_REG_OFFSET(b,r),&v)
#define XPORT_MIB_INDIRECT_WRITE(i,b,r,v) xport_xlmac_indirect_write(1, i>>XLMAC_NUM_S, RU_BLK(b).addr[i] + RU_REG_OFFSET(b,r),v)
#define XPORT_MIB_INDIRECT_READ(i,b,r,v) xport_xlmac_indirect_read(1, i>>XLMAC_NUM_S, RU_BLK(b).addr[i] + RU_REG_OFFSET(b,r),&v)

static void xport_xlmac_indirect_write(int mib, int xlmac_id, uintptr_t address, uint64_t val)
{
    //fill using holding register
    uint32_t holding = (uint32_t)((val >> 32) & 0xFFFFFFFF);
    uint32_t regval = (uint32_t)(val & 0xFFFFFFFF);
    if (mib)
    {
        ag_drv_xport_mib_reg_dir_acc_data_write_set(xlmac_id, holding);
    }
    else
    {
        ag_drv_xport_xlmac_reg_dir_acc_data_write_set(xlmac_id, holding);
    }

    WRITE_32(address, regval);
}

static void xport_xlmac_indirect_read(int mib, int xlmac_id, uintptr_t address, uint64_t *val)
{
    //fill using holding register
    uint32_t value32;
    uint32_t holding;

    READ_32(address, value32);
    if (mib)
    {
        ag_drv_xport_mib_reg_dir_acc_data_read_get(xlmac_id, &holding);
    }
    else
    {
        ag_drv_xport_xlmac_reg_dir_acc_data_read_get(xlmac_id, &holding);
    }

    *val = (((uint64_t)holding << 32) | value32);
}

#endif /* SHARED_OPENSOURCE_DRV_XPORT_XLMAC_INDIRECT_ACCESS_H_ */
