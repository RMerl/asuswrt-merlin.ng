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
 * mib_indirect_access.h
 *
 *  Created on: 6 בספט׳ 2015
 *      Author: yonatani
 */

#ifndef SHARED_OPENSOURCE_DRV_LPORT_MIB_INDIRECT_ACCESS_H_
#define SHARED_OPENSOURCE_DRV_LPORT_MIB_INDIRECT_ACCESS_H_
#include "bcm6858_mib_conf_ag.h"

#define MIB_INDIRECT_WRITE(i,b,r,v) mib_indirect_write(i, RU_BLK(b).addr[i]+RU_REG_OFFSET(b,r),v)
#define MIB_INDIRECT_READ(i,b,r,v) mib_indirect_read(i, RU_BLK(b).addr[i]+RU_REG_OFFSET(b,r),&v)

static void mib_indirect_write(uint32_t portid, uint64_t address, uint64_t val)
{
    //fill using holding register
    uint32_t xlmacid = (portid >> 2) & 0x1;
    uint32_t holding = (uint32_t)((val >> 32) & 0xFFFFFFFF);
    uint32_t regval = (uint32_t)(val & 0xFFFFFFFF);
    ag_drv_mib_conf_mib0_write_holding_set(xlmacid, holding);

    WRITE_32(address, regval);
}

static void mib_indirect_read(uint32_t portid, uint64_t address, uint64_t *val)
{
    //fill using holding register
    uint32_t xlmacid = (portid >> 2) & 0x1;
    uint32_t value32;
    uint32_t holding;

    READ_32(address, value32);

    ag_drv_mib_conf_mib0_read_holding_get(xlmacid, &holding);

    *val = (((uint64_t)holding << 32) | value32);
}

#endif //SHARED_OPENSOURCE_DRV_LPORT_MIB_INDIRECT_ACCESS_H_
