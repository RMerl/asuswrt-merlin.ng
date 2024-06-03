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


#include "XRDP_RNR_INST_AG.h"

/******************************************************************************
 * Register: NAME: RNR_INST_MEM_ENTRY, TYPE: Type_RCQ_INST_RCQ_MEM_ENTRY
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: INSTRUCTION *****/
const ru_field_rec RNR_INST_MEM_ENTRY_INSTRUCTION_FIELD =
{
    "INSTRUCTION",
#if RU_INCLUDE_DESC
    "",
    "instructions memory\n",
#endif
    { RNR_INST_MEM_ENTRY_INSTRUCTION_FIELD_MASK },
    0,
    { RNR_INST_MEM_ENTRY_INSTRUCTION_FIELD_WIDTH },
    { RNR_INST_MEM_ENTRY_INSTRUCTION_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_INST_MEM_ENTRY_FIELDS[] =
{
    &RNR_INST_MEM_ENTRY_INSTRUCTION_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_INST_MEM_ENTRY *****/
const ru_reg_rec RNR_INST_MEM_ENTRY_REG =
{
    "MEM_ENTRY",
#if RU_INCLUDE_DESC
    "INSTRUCTION_MEMORY_ENTRY Register",
    "Instruction memory entry\n",
#endif
    { RNR_INST_MEM_ENTRY_REG_OFFSET },
    RNR_INST_MEM_ENTRY_REG_RAM_CNT,
    4,
    838,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_INST_MEM_ENTRY_FIELDS,
#endif
};

unsigned long RNR_INST_ADDRS[] =
{
    0x82610000,
    0x82630000,
    0x82650000,
    0x82670000,
    0x82690000,
    0x826B0000,
    0x826D0000,
    0x826F0000,
    0x82710000,
    0x82730000,
    0x82750000,
    0x82770000,
    0x82790000,
    0x827B0000,
};

static const ru_reg_rec *RNR_INST_REGS[] =
{
    &RNR_INST_MEM_ENTRY_REG,
};

const ru_block_rec RNR_INST_BLOCK =
{
    "RNR_INST",
    RNR_INST_ADDRS,
    14,
    1,
    RNR_INST_REGS,
};
