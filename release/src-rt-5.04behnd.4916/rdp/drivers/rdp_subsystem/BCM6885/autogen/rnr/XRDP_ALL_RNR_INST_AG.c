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


#include "XRDP_ALL_RNR_INST_AG.h"

/******************************************************************************
 * Register: NAME: ALL_RNR_INST_MEM_ENTRY, TYPE: Type_RCQ_INST_INST_MEM_ENTRY
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: INSTRUCTION *****/
const ru_field_rec ALL_RNR_INST_MEM_ENTRY_INSTRUCTION_FIELD =
{
    "INSTRUCTION",
#if RU_INCLUDE_DESC
    "",
    "instructions memory\n",
#endif
    { ALL_RNR_INST_MEM_ENTRY_INSTRUCTION_FIELD_MASK },
    0,
    { ALL_RNR_INST_MEM_ENTRY_INSTRUCTION_FIELD_WIDTH },
    { ALL_RNR_INST_MEM_ENTRY_INSTRUCTION_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *ALL_RNR_INST_MEM_ENTRY_FIELDS[] =
{
    &ALL_RNR_INST_MEM_ENTRY_INSTRUCTION_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: ALL_RNR_INST_MEM_ENTRY *****/
const ru_reg_rec ALL_RNR_INST_MEM_ENTRY_REG =
{
    "MEM_ENTRY",
#if RU_INCLUDE_DESC
    "INSTRUCTION_MEMORY_ENTRY 0..8191 Register",
    "Instruction memory entry\n",
#endif
    { ALL_RNR_INST_MEM_ENTRY_REG_OFFSET },
    ALL_RNR_INST_MEM_ENTRY_REG_RAM_CNT,
    4,
    146,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    ALL_RNR_INST_MEM_ENTRY_FIELDS,
#endif
};

unsigned long ALL_RNR_INST_ADDRS[] =
{
    0xA1808000,
    0xA1828000,
    0xA1848000,
    0xA1868000,
    0xA1888000,
    0xA18A8000,
    0xA18C8000,
    0xA18E8000,
    0xA1A08000,
    0xA1A28000,
    0xA1A48000,
    0xA1A68000,
    0xA1A88000,
    0xA1AA8000,
    0xA1AC8000,
    0xA1AE8000,
    0xA1C08000,
    0xA1C28000,
    0xA1C48000,
    0xA1C68000,
    0xA1C88000,
    0xA1CA8000,
    0xA1CC8000,
    0xA1CE8000,
    0xB2008000,
    0xB2028000,
    0xB2048000,
    0xB2068000,
};

static const ru_reg_rec *ALL_RNR_INST_REGS[] =
{
    &ALL_RNR_INST_MEM_ENTRY_REG,
};

const ru_block_rec ALL_RNR_INST_BLOCK =
{
    "ALL_RNR_INST",
#if RU_INCLUDE_SIZE
    { 0x00008000 },
#endif
    ALL_RNR_INST_ADDRS,
    28,
    1,
    ALL_RNR_INST_REGS,
};
