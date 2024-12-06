/*
   Copyright (c) 2015 Broadcom
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
    673,
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
