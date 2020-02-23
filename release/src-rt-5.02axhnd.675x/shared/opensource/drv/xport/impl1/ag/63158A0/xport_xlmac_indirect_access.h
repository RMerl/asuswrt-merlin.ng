/*
   Copyright (c) 2017 Broadcom Corporation
   All Rights Reserved

   <:label-BRCM:2017:DUAL/GPL:standard

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
