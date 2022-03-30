/*
   Copyright (c) 2017 Broadcom Corporation
   All Rights Reserved

   <:label-BRCM:2017:DUAL/GPL:standard
   
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
 * xport_xlmac_indirect_access.h
 *
 */

#ifndef SHARED_OPENSOURCE_DRV_XPORT_XLMAC_INDIRECT_ACCESS_H_
#define SHARED_OPENSOURCE_DRV_XPORT_XLMAC_INDIRECT_ACCESS_H_

#include "ru_types.h"
#include "bcm63158_xport_xlmac_reg_ag.h"
#include "bcm63158_xport_mib_reg_ag.h"

#define XPORT_XLMAC_INDIRECT_WRITE(i,b,r,v) xport_xlmac_indirect_write(0, RU_BLK(b).addr[i] + RU_REG_OFFSET(b,r),v)
#define XPORT_XLMAC_INDIRECT_READ(i,b,r,v) xport_xlmac_indirect_read(0, RU_BLK(b).addr[i] + RU_REG_OFFSET(b,r),&v)
#define XPORT_MIB_INDIRECT_WRITE(i,b,r,v) xport_xlmac_indirect_write(1, RU_BLK(b).addr[i] + RU_REG_OFFSET(b,r),v)
#define XPORT_MIB_INDIRECT_READ(i,b,r,v) xport_xlmac_indirect_read(1, RU_BLK(b).addr[i] + RU_REG_OFFSET(b,r),&v)

static void xport_xlmac_indirect_write(int mib, uintptr_t address, uint64_t val)
{
    //fill using holding register
    uint32_t holding = (uint32_t)((val >> 32) & 0xFFFFFFFF);
    uint32_t regval = (uint32_t)(val & 0xFFFFFFFF);
    if (mib)
    {
        ag_drv_xport_mib_reg_dir_acc_data_write_set(holding);
    }
    else
    {
        ag_drv_xport_xlmac_reg_dir_acc_data_write_set(holding);
    }

    WRITE_32(address, regval);
}

static void xport_xlmac_indirect_read(int mib, uintptr_t address, uint64_t *val)
{
    //fill using holding register
    uint32_t value32;
    uint32_t holding;

    READ_32(address, value32);
    if (mib)
    {
        ag_drv_xport_mib_reg_dir_acc_data_read_get(&holding);
    }
    else
    {
        ag_drv_xport_xlmac_reg_dir_acc_data_read_get(&holding);
    }

    *val = (((uint64_t)holding << 32) | value32);
}

#endif /* SHARED_OPENSOURCE_DRV_XPORT_XLMAC_INDIRECT_ACCESS_H_ */
