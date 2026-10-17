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


#include "XRDP_ALL_RNR_MEM_AG.h"

/******************************************************************************
 * Register: NAME: ALL_RNR_MEM_MEM_ENTRY, TYPE: Type_RCQ_MEMORY_DATA_MEM_ENTRY
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA_MEM *****/
const ru_field_rec ALL_RNR_MEM_MEM_ENTRY_DATA_MEM_FIELD =
{
    "DATA_MEM",
#if RU_INCLUDE_DESC
    "",
    "data memory\nCHANGED_BY_HW\n\n",
#endif
    { ALL_RNR_MEM_MEM_ENTRY_DATA_MEM_FIELD_MASK },
    0,
    { ALL_RNR_MEM_MEM_ENTRY_DATA_MEM_FIELD_WIDTH },
    { ALL_RNR_MEM_MEM_ENTRY_DATA_MEM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *ALL_RNR_MEM_MEM_ENTRY_FIELDS[] =
{
    &ALL_RNR_MEM_MEM_ENTRY_DATA_MEM_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: ALL_RNR_MEM_MEM_ENTRY *****/
const ru_reg_rec ALL_RNR_MEM_MEM_ENTRY_REG =
{
    "MEM_ENTRY",
#if RU_INCLUDE_DESC
    "DATA_MEMORY_ENTRY 0..4095 Register",
    "Data memory entry\n",
#endif
    { ALL_RNR_MEM_MEM_ENTRY_REG_OFFSET },
    ALL_RNR_MEM_MEM_ENTRY_REG_RAM_CNT,
    4,
    147,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    ALL_RNR_MEM_MEM_ENTRY_FIELDS,
#endif
};

unsigned long ALL_RNR_MEM_ADDRS[] =
{
    0xA1800000,
    0xA1820000,
    0xA1840000,
    0xA1860000,
    0xA1880000,
    0xA18A0000,
    0xA18C0000,
    0xA18E0000,
    0xA1A00000,
    0xA1A20000,
    0xA1A40000,
    0xA1A60000,
    0xA1A80000,
    0xA1AA0000,
    0xA1AC0000,
    0xA1AE0000,
    0xA1C00000,
    0xA1C20000,
    0xA1C40000,
    0xA1C60000,
    0xA1C80000,
    0xA1CA0000,
    0xA1CC0000,
    0xA1CE0000,
    0xB2000000,
    0xB2020000,
    0xB2040000,
    0xB2060000,
};

static const ru_reg_rec *ALL_RNR_MEM_REGS[] =
{
    &ALL_RNR_MEM_MEM_ENTRY_REG,
};

const ru_block_rec ALL_RNR_MEM_BLOCK =
{
    "ALL_RNR_MEM",
#if RU_INCLUDE_SIZE
    { 0x00004000 },
#endif
    ALL_RNR_MEM_ADDRS,
    28,
    1,
    ALL_RNR_MEM_REGS,
};
