// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    
*/

#ifndef _XRDP_DRV_RNR_REGS_H_
#define _XRDP_DRV_RNR_REGS_H_

#include "access_macros.h"
#include "bdmf_interface.h"
#include "rdp_common.h"


bdmf_error_t ag_drv_rnr_regs_cfg_cpu_wakeup_set(uint8_t rnr_id, uint8_t thread_num);

#endif

