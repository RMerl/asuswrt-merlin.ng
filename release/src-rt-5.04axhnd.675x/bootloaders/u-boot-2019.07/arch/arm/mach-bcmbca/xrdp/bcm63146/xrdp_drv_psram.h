// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    
*/

#ifndef _XRDP_DRV_PSRAM_H_
#define _XRDP_DRV_PSRAM_H_

#include "access_macros.h"
#include "bdmf_interface.h"
#include "rdp_common.h"


/**************************************************************************************************/
/* data: data - data                                                                              */
/**************************************************************************************************/
typedef struct
{
    uint32_t memory_data[32];
} psram_memory_data;


bdmf_error_t ag_drv_psram_memory_data_set(uint32_t psram_enrty, const psram_memory_data *memory_data);
bdmf_error_t ag_drv_psram_memory_data_get(uint32_t psram_enrty, psram_memory_data *memory_data);
#endif

