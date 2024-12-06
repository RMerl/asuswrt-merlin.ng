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


#include "XRDP_RNR_MEM_AG.h"

/******************************************************************************
 * Register: NAME: RNR_MEM_HIGH, TYPE: Type_RCQ_MEMORY_RCQ_MEM_HIGH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA_MEM *****/
const ru_field_rec RNR_MEM_HIGH_DATA_MEM_FIELD =
{
    "DATA_MEM",
#if RU_INCLUDE_DESC
    "",
    "data memory\n",
#endif
    { RNR_MEM_HIGH_DATA_MEM_FIELD_MASK },
    0,
    { RNR_MEM_HIGH_DATA_MEM_FIELD_WIDTH },
    { RNR_MEM_HIGH_DATA_MEM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_MEM_HIGH_FIELDS[] =
{
    &RNR_MEM_HIGH_DATA_MEM_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_MEM_HIGH *****/
const ru_reg_rec RNR_MEM_HIGH_REG =
{
    "HIGH",
#if RU_INCLUDE_DESC
    "DATA_MEMORY_ENTRY_HIGH Register",
    "Data memory entry\n",
#endif
    { RNR_MEM_HIGH_REG_OFFSET },
    RNR_MEM_HIGH_REG_RAM_CNT,
    8,
    864,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_MEM_HIGH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: RNR_MEM_LOW, TYPE: Type_RCQ_MEMORY_RCQ_MEM_LOW
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA_MEM *****/
const ru_field_rec RNR_MEM_LOW_DATA_MEM_FIELD =
{
    "DATA_MEM",
#if RU_INCLUDE_DESC
    "",
    "data memory\n",
#endif
    { RNR_MEM_LOW_DATA_MEM_FIELD_MASK },
    0,
    { RNR_MEM_LOW_DATA_MEM_FIELD_WIDTH },
    { RNR_MEM_LOW_DATA_MEM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_MEM_LOW_FIELDS[] =
{
    &RNR_MEM_LOW_DATA_MEM_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_MEM_LOW *****/
const ru_reg_rec RNR_MEM_LOW_REG =
{
    "LOW",
#if RU_INCLUDE_DESC
    "DATA_MEMORY_ENTRY_LOW Register",
    "Data memory entry\n",
#endif
    { RNR_MEM_LOW_REG_OFFSET },
    RNR_MEM_LOW_REG_RAM_CNT,
    8,
    865,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_MEM_LOW_FIELDS,
#endif
};

unsigned long RNR_MEM_ADDRS[] =
{
    0x82600000,
    0x82620000,
    0x82640000,
    0x82660000,
    0x82680000,
    0x826A0000,
    0x826C0000,
    0x826E0000,
    0x82700000,
    0x82720000,
    0x82740000,
    0x82760000,
    0x82780000,
    0x827A0000,
};

static const ru_reg_rec *RNR_MEM_REGS[] =
{
    &RNR_MEM_HIGH_REG,
    &RNR_MEM_LOW_REG,
};

const ru_block_rec RNR_MEM_BLOCK =
{
    "RNR_MEM",
    RNR_MEM_ADDRS,
    14,
    2,
    RNR_MEM_REGS,
};
