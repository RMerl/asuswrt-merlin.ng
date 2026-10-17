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


#include "XRDP_CRDP_EGR_RNR_INST_AG.h"

/******************************************************************************
 * Register: NAME: CRDP_EGR_RNR_INST_MEM_ENTRY, TYPE: Type_RCQ_INST_INST_MEM_ENTRY
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: INSTRUCTION *****/
const ru_field_rec CRDP_EGR_RNR_INST_MEM_ENTRY_INSTRUCTION_FIELD =
{
    "INSTRUCTION",
#if RU_INCLUDE_DESC
    "",
    "instructions memory\n",
#endif
    { CRDP_EGR_RNR_INST_MEM_ENTRY_INSTRUCTION_FIELD_MASK },
    0,
    { CRDP_EGR_RNR_INST_MEM_ENTRY_INSTRUCTION_FIELD_WIDTH },
    { CRDP_EGR_RNR_INST_MEM_ENTRY_INSTRUCTION_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_EGR_RNR_INST_MEM_ENTRY_FIELDS[] =
{
    &CRDP_EGR_RNR_INST_MEM_ENTRY_INSTRUCTION_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_EGR_RNR_INST_MEM_ENTRY *****/
const ru_reg_rec CRDP_EGR_RNR_INST_MEM_ENTRY_REG =
{
    "MEM_ENTRY",
#if RU_INCLUDE_DESC
    "INSTRUCTION_MEMORY_ENTRY 0..8191 Register",
    "Instruction memory entry\n",
#endif
    { CRDP_EGR_RNR_INST_MEM_ENTRY_REG_OFFSET },
    CRDP_EGR_RNR_INST_MEM_ENTRY_REG_RAM_CNT,
    4,
    910,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_EGR_RNR_INST_MEM_ENTRY_FIELDS,
#endif
};

unsigned long CRDP_EGR_RNR_INST_ADDRS[] =
{
    0xB2008000,
    0xB2028000,
    0xB2048000,
    0xB2068000,
};

static const ru_reg_rec *CRDP_EGR_RNR_INST_REGS[] =
{
    &CRDP_EGR_RNR_INST_MEM_ENTRY_REG,
};

const ru_block_rec CRDP_EGR_RNR_INST_BLOCK =
{
    "CRDP_EGR_RNR_INST",
#if RU_INCLUDE_SIZE
    { 0x00008000 },
#endif
    CRDP_EGR_RNR_INST_ADDRS,
    4,
    1,
    CRDP_EGR_RNR_INST_REGS,
};
