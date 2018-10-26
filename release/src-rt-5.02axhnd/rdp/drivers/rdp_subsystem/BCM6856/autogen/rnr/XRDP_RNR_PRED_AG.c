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
 * Field: RNR_PRED_MEM_ENTRY_PRED_MEM
 ******************************************************************************/
const ru_field_rec RNR_PRED_MEM_ENTRY_PRED_MEM_FIELD =
{
    "PRED_MEM",
#if RU_INCLUDE_DESC
    "MEM_PRED_MAIN",
    "MEM_PRED_MAIN",
#endif
    RNR_PRED_MEM_ENTRY_PRED_MEM_FIELD_MASK,
    0,
    RNR_PRED_MEM_ENTRY_PRED_MEM_FIELD_WIDTH,
    RNR_PRED_MEM_ENTRY_PRED_MEM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: RNR_PRED_MEM_ENTRY
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_PRED_MEM_ENTRY_FIELDS[] =
{
    &RNR_PRED_MEM_ENTRY_PRED_MEM_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RNR_PRED_MEM_ENTRY_REG = 
{
    "MEM_ENTRY",
#if RU_INCLUDE_DESC
    "PREDICTION_MEMORY_CORE %i Register",
    "Prediction memory MAIN core",
#endif
    RNR_PRED_MEM_ENTRY_REG_OFFSET,
    RNR_PRED_MEM_ENTRY_REG_RAM_CNT,
    2,
    313,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_PRED_MEM_ENTRY_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Block: RNR_PRED
 ******************************************************************************/
static const ru_reg_rec *RNR_PRED_REGS[] =
{
    &RNR_PRED_MEM_ENTRY_REG,
};

unsigned long RNR_PRED_ADDRS[] =
{
    0x82c1c000,
    0x82c3c000,
    0x82c5c000,
    0x82c7c000,
    0x82c9c000,
    0x82cbc000,
    0x82cdc000,
    0x82cfc000,
};

const ru_block_rec RNR_PRED_BLOCK = 
{
    "RNR_PRED",
    RNR_PRED_ADDRS,
    8,
    1,
    RNR_PRED_REGS
};

/* End of file XRDP_RNR_PRED.c */
