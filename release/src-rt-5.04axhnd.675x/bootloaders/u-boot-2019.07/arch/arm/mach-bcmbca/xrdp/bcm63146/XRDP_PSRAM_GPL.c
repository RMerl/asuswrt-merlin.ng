// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    
*/

#include "ru.h"

/******************************************************************************
 * Register: PSRAM_MEMORY_DATA
 ******************************************************************************/
const ru_reg_rec PSRAM_MEMORY_DATA_REG = 
{
    "MEMORY_DATA",
#if RU_INCLUDE_DESC
    "PSRAM_MEM_ENTRY %i Register",
    "psram_mem_entry",
#endif
    PSRAM_MEMORY_DATA_REG_OFFSET,
    PSRAM_MEMORY_DATA_REG_RAM_CNT,
    4,
    765,
};

/******************************************************************************
 * Block: PSRAM
 ******************************************************************************/
static const ru_reg_rec *PSRAM_REGS[] =
{
    &PSRAM_MEMORY_DATA_REG,
};

unsigned long PSRAM_ADDRS[] =
{
    0x82000000,
};

const ru_block_rec PSRAM_BLOCK = 
{
    "PSRAM",
    PSRAM_ADDRS,
    1,
    41,
    PSRAM_REGS
};

/* End of file XRDP_PSRAM_GPL_AG_trim.c */
