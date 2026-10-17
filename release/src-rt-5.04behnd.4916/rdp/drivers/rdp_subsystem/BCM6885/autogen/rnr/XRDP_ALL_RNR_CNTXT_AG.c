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


#include "XRDP_ALL_RNR_CNTXT_AG.h"

/******************************************************************************
 * Register: NAME: ALL_RNR_CNTXT_MEM_ENTRY, TYPE: Type_RCQ_CNTXT_CNTXT_MEM_ENTRY
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CONTEXT_ENTRY *****/
const ru_field_rec ALL_RNR_CNTXT_MEM_ENTRY_CONTEXT_ENTRY_FIELD =
{
    "CONTEXT_ENTRY",
#if RU_INCLUDE_DESC
    "",
    "Context mem entry\n",
#endif
    { ALL_RNR_CNTXT_MEM_ENTRY_CONTEXT_ENTRY_FIELD_MASK },
    0,
    { ALL_RNR_CNTXT_MEM_ENTRY_CONTEXT_ENTRY_FIELD_WIDTH },
    { ALL_RNR_CNTXT_MEM_ENTRY_CONTEXT_ENTRY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *ALL_RNR_CNTXT_MEM_ENTRY_FIELDS[] =
{
    &ALL_RNR_CNTXT_MEM_ENTRY_CONTEXT_ENTRY_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: ALL_RNR_CNTXT_MEM_ENTRY *****/
const ru_reg_rec ALL_RNR_CNTXT_MEM_ENTRY_REG =
{
    "MEM_ENTRY",
#if RU_INCLUDE_DESC
    "CONTEXT_MEM_ENTRY 0..511 Register",
    "Context mem entry\n",
#endif
    { ALL_RNR_CNTXT_MEM_ENTRY_REG_OFFSET },
    ALL_RNR_CNTXT_MEM_ENTRY_REG_RAM_CNT,
    4,
    145,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    ALL_RNR_CNTXT_MEM_ENTRY_FIELDS,
#endif
};

unsigned long ALL_RNR_CNTXT_ADDRS[] =
{
    0xA1810000,
    0xA1830000,
    0xA1850000,
    0xA1870000,
    0xA1890000,
    0xA18B0000,
    0xA18D0000,
    0xA18F0000,
    0xA1A10000,
    0xA1A30000,
    0xA1A50000,
    0xA1A70000,
    0xA1A90000,
    0xA1AB0000,
    0xA1AD0000,
    0xA1AF0000,
    0xA1C10000,
    0xA1C30000,
    0xA1C50000,
    0xA1C70000,
    0xA1C90000,
    0xA1CB0000,
    0xA1CD0000,
    0xA1CF0000,
    0xB2010000,
    0xB2030000,
    0xB2050000,
    0xB2070000,
};

static const ru_reg_rec *ALL_RNR_CNTXT_REGS[] =
{
    &ALL_RNR_CNTXT_MEM_ENTRY_REG,
};

const ru_block_rec ALL_RNR_CNTXT_BLOCK =
{
    "ALL_RNR_CNTXT",
#if RU_INCLUDE_SIZE
    { 0x00000800 },
#endif
    ALL_RNR_CNTXT_ADDRS,
    28,
    1,
    ALL_RNR_CNTXT_REGS,
};
