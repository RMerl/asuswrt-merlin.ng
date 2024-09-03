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
    863,
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
