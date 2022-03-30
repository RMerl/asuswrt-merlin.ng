// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    
*/

#include "rdp_common.h"
#include "xrdp_drv_drivers_common_ag.h"
#include "xrdp_drv_rnr_regs.h"

#define BLOCK_ADDR_COUNT_BITS 3
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_rnr_regs_cfg_cpu_wakeup_set(uint8_t rnr_id, uint8_t thread_num)
{
    uint32_t reg_cfg_cpu_wakeup=0;

#ifdef VALIDATE_PARMS
    if((rnr_id >= BLOCK_ADDR_COUNT) ||
       (thread_num >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cfg_cpu_wakeup = RU_FIELD_SET(rnr_id, RNR_REGS, CFG_CPU_WAKEUP, THREAD_NUM, reg_cfg_cpu_wakeup, thread_num);

    RU_REG_WRITE(rnr_id, RNR_REGS, CFG_CPU_WAKEUP, reg_cfg_cpu_wakeup);

    return BDMF_ERR_OK;
}

