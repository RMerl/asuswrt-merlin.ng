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


#include "XRDP_RNR_PRED_AG.h"

/******************************************************************************
 * Register: NAME: RNR_PRED_MEM_ENTRY, TYPE: Type_RCQ_PRED_RCQ_MEM_ENTRY
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PRED_MEM *****/
const ru_field_rec RNR_PRED_MEM_ENTRY_PRED_MEM_FIELD =
{
    "PRED_MEM",
#if RU_INCLUDE_DESC
    "",
    "MEM_PRED_MAIN\n",
#endif
    { RNR_PRED_MEM_ENTRY_PRED_MEM_FIELD_MASK },
    0,
    { RNR_PRED_MEM_ENTRY_PRED_MEM_FIELD_WIDTH },
    { RNR_PRED_MEM_ENTRY_PRED_MEM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RNR_PRED_MEM_ENTRY_FIELDS[] =
{
    &RNR_PRED_MEM_ENTRY_PRED_MEM_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: RNR_PRED_MEM_ENTRY *****/
const ru_reg_rec RNR_PRED_MEM_ENTRY_REG =
{
    "MEM_ENTRY",
#if RU_INCLUDE_DESC
    "PREDICTION_MEMORY_CORE 0..511 Register",
    "Prediction memory MAIN core\n",
#endif
    { RNR_PRED_MEM_ENTRY_REG_OFFSET },
    RNR_PRED_MEM_ENTRY_REG_RAM_CNT,
    2,
    841,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    RNR_PRED_MEM_ENTRY_FIELDS,
#endif
};

unsigned long RNR_PRED_ADDRS[] =
{
    0x8261C000,
    0x8263C000,
    0x8265C000,
    0x8267C000,
    0x8269C000,
    0x826BC000,
    0x826DC000,
    0x826FC000,
    0x8271C000,
    0x8273C000,
    0x8275C000,
    0x8277C000,
    0x8279C000,
    0x827BC000,
};

static const ru_reg_rec *RNR_PRED_REGS[] =
{
    &RNR_PRED_MEM_ENTRY_REG,
};

const ru_block_rec RNR_PRED_BLOCK =
{
    "RNR_PRED",
    RNR_PRED_ADDRS,
    14,
    1,
    RNR_PRED_REGS,
};
