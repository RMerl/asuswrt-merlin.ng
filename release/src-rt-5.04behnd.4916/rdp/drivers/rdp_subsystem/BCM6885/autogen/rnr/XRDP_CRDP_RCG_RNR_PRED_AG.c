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


#include "XRDP_CRDP_RCG_RNR_PRED_AG.h"

/******************************************************************************
 * Register: NAME: CRDP_RCG_RNR_PRED_MEM_ENTRY, TYPE: Type_RCQ_PRED_PRED_MEM_ENTRY
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PRED_MEM *****/
const ru_field_rec CRDP_RCG_RNR_PRED_MEM_ENTRY_PRED_MEM_FIELD =
{
    "PRED_MEM",
#if RU_INCLUDE_DESC
    "",
    "MEM_PRED_MAIN\n",
#endif
    { CRDP_RCG_RNR_PRED_MEM_ENTRY_PRED_MEM_FIELD_MASK },
    0,
    { CRDP_RCG_RNR_PRED_MEM_ENTRY_PRED_MEM_FIELD_WIDTH },
    { CRDP_RCG_RNR_PRED_MEM_ENTRY_PRED_MEM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CRDP_RCG_RNR_PRED_MEM_ENTRY_FIELDS[] =
{
    &CRDP_RCG_RNR_PRED_MEM_ENTRY_PRED_MEM_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CRDP_RCG_RNR_PRED_MEM_ENTRY *****/
const ru_reg_rec CRDP_RCG_RNR_PRED_MEM_ENTRY_REG =
{
    "MEM_ENTRY",
#if RU_INCLUDE_DESC
    "PREDICTION_MEMORY_CORE 0..511 Register",
    "Prediction memory MAIN core\n",
#endif
    { CRDP_RCG_RNR_PRED_MEM_ENTRY_REG_OFFSET },
    CRDP_RCG_RNR_PRED_MEM_ENTRY_REG_RAM_CNT,
    4,
    1225,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CRDP_RCG_RNR_PRED_MEM_ENTRY_FIELDS,
#endif
};

unsigned long CRDP_RCG_RNR_PRED_ADDRS[] =
{
    0xA1814000,
    0xA1834000,
    0xA1854000,
    0xA1874000,
    0xA1894000,
    0xA18B4000,
    0xA18D4000,
    0xA18F4000,
    0xA1A14000,
    0xA1A34000,
    0xA1A54000,
    0xA1A74000,
    0xA1A94000,
    0xA1AB4000,
    0xA1AD4000,
    0xA1AF4000,
    0xA1C14000,
    0xA1C34000,
    0xA1C54000,
    0xA1C74000,
    0xA1C94000,
    0xA1CB4000,
    0xA1CD4000,
    0xA1CF4000,
};

static const ru_reg_rec *CRDP_RCG_RNR_PRED_REGS[] =
{
    &CRDP_RCG_RNR_PRED_MEM_ENTRY_REG,
};

const ru_block_rec CRDP_RCG_RNR_PRED_BLOCK =
{
    "CRDP_RCG_RNR_PRED",
#if RU_INCLUDE_SIZE
    { 0x00000800 },
#endif
    CRDP_RCG_RNR_PRED_ADDRS,
    24,
    1,
    CRDP_RCG_RNR_PRED_REGS,
};
