// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

   
   */
/*
 * xlmac_indirect_access.h
 *
 *  Created on: 6 בספט׳ 2015
 *      Author: yonatani
 */

#ifndef SHARED_OPENSOURCE_DRV_LPORT_XLMAC_INDIRECT_ACCESS_H_
#define SHARED_OPENSOURCE_DRV_LPORT_XLMAC_INDIRECT_ACCESS_H_

#include "ru_types.h"
#include "bcm6858_xlmac_conf_ag.h"

#define XLMAC_INDIRECT_WRITE(i,b,r,v) xlmac_indirect_write(i,RU_BLK(b).addr[i] + RU_REG_OFFSET(b,r),v)
#define XLMAC_INDIRECT_READ(i,b,r,v) xlmac_indirect_read(i,RU_BLK(b).addr[i] + RU_REG_OFFSET(b,r),&v)

static void xlmac_indirect_write(uint32_t portid, uint64_t address, uint64_t val)
{
    //fill using holding register
    uint32_t xlmacid = (portid >> 2) & 0x1;
    uint32_t holding = (uint32_t)((val >> 32) & 0xFFFFFFFF);
    uint32_t regval = (uint32_t)(val & 0xFFFFFFFF);
    ag_drv_xlmac_conf_dir_acc_data_write_set(xlmacid, holding);

    WRITE_32(address, regval);
}

static void xlmac_indirect_read(uint32_t portid, uint64_t address, uint64_t *val)
{
    //fill using holding register
    uint32_t xlmacid = (portid >> 2) & 0x1;
    uint32_t value32;
    uint32_t holding;

    READ_32(address, value32);

    ag_drv_xlmac_conf_dir_acc_data_read_get(xlmacid, &holding);

    *val = (((uint64_t)holding << 32) | value32);
}

#endif /* SHARED_OPENSOURCE_DRV_LPORT_XLMAC_INDIRECT_ACCESS_H_ */
