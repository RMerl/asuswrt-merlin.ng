/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard
    
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


#include "XRDP_PSRAM_MEM_AG.h"

/******************************************************************************
 * Register: NAME: PSRAM_MEM_MEMORY_DATA, TYPE: Type_PSRAM_MEMORY_PSRAM_MEMORY_DATA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec PSRAM_MEM_MEMORY_DATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "data\n",
#endif
    { PSRAM_MEM_MEMORY_DATA_DATA_FIELD_MASK },
    0,
    { PSRAM_MEM_MEMORY_DATA_DATA_FIELD_WIDTH },
    { PSRAM_MEM_MEMORY_DATA_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PSRAM_MEM_MEMORY_DATA_FIELDS[] =
{
    &PSRAM_MEM_MEMORY_DATA_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: PSRAM_MEM_MEMORY_DATA *****/
const ru_reg_rec PSRAM_MEM_MEMORY_DATA_REG =
{
    "MEMORY_DATA",
#if RU_INCLUDE_DESC
    "PSRAM_MEM_ENTRY 0..131071 Register",
    "psram_mem_entry\n",
#endif
    { PSRAM_MEM_MEMORY_DATA_REG_OFFSET },
    PSRAM_MEM_MEMORY_DATA_REG_RAM_CNT,
    4,
    666,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    PSRAM_MEM_MEMORY_DATA_FIELDS,
#endif
};

unsigned long PSRAM_MEM_ADDRS[] =
{
    0x82000000,
    0x82200000,
};

static const ru_reg_rec *PSRAM_MEM_REGS[] =
{
    &PSRAM_MEM_MEMORY_DATA_REG,
};

const ru_block_rec PSRAM_MEM_BLOCK =
{
    "PSRAM_MEM",
    PSRAM_MEM_ADDRS,
    2,
    1,
    PSRAM_MEM_REGS,
};
