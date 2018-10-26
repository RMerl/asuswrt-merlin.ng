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

#include "ru.h"

#if RU_INCLUDE_FIELD_DB
/******************************************************************************
 * Field: RNR_INST_MEM_ENTRY_INSTRUCTION
 ******************************************************************************/
const ru_field_rec RNR_INST_MEM_ENTRY_INSTRUCTION_FIELD =
{
    "INSTRUCTION",
#if RU_INCLUDE_DESC
    "Instructions_memory",
    "instructions memory",
#endif
    RNR_INST_MEM_ENTRY_INSTRUCTION_FIELD_MASK,
    0,
    RNR_INST_MEM_ENTRY_INSTRUCTION_FIELD_WIDTH,
    RNR_INST_MEM_ENTRY_INSTRUCTION_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: RNR_INST_MEM_ENTRY
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_INST_MEM_ENTRY_FIELDS[] =
{
    &RNR_INST_MEM_ENTRY_INSTRUCTION_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_INST_MEM_ENTRY_REG = 
{
    "MEM_ENTRY",
#if RU_INCLUDE_DESC
    "INSTRUCTION_MEMORY_ENTRY Register",
    "Instruction memory entry",
#endif
    RNR_INST_MEM_ENTRY_REG_OFFSET,
    RNR_INST_MEM_ENTRY_REG_RAM_CNT,
    4,
    235,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_INST_MEM_ENTRY_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Block: RNR_INST
 ******************************************************************************/
static const ru_reg_rec *RNR_INST_REGS[] =
{
    &RNR_INST_MEM_ENTRY_REG,
};

unsigned long RNR_INST_ADDRS[] =
{
    0x82210000,
    0x82230000,
    0x82250000,
    0x82270000,
    0x82310000,
    0x82330000,
    0x82350000,
    0x82370000,
    0x82410000,
    0x82430000,
    0x82450000,
    0x82470000,
    0x82510000,
    0x82530000,
    0x82550000,
    0x82570000,
};

const ru_block_rec RNR_INST_BLOCK = 
{
    "RNR_INST",
    RNR_INST_ADDRS,
    16,
    1,
    RNR_INST_REGS
};

/* End of file XRDP_RNR_INST.c */
