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


#ifndef _XRDP_RNR_PRED_AG_H_
#define _XRDP_RNR_PRED_AG_H_

#include "ru_types.h"

#define RNR_PRED_MEM_ENTRY_PRED_MEM_FIELD_MASK 0x0000FFFF
#define RNR_PRED_MEM_ENTRY_PRED_MEM_FIELD_WIDTH 16
#define RNR_PRED_MEM_ENTRY_PRED_MEM_FIELD_SHIFT 0
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec RNR_PRED_MEM_ENTRY_PRED_MEM_FIELD;
#endif
extern const ru_reg_rec RNR_PRED_MEM_ENTRY_REG;
#define RNR_PRED_MEM_ENTRY_REG_OFFSET 0x00000000
#define RNR_PRED_MEM_ENTRY_REG_RAM_CNT 512

extern const ru_block_rec RNR_PRED_BLOCK;

#endif
