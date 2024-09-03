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


#ifndef _XRDP_PSRAM_MEM_AG_H_
#define _XRDP_PSRAM_MEM_AG_H_

#include "ru_types.h"

#define PSRAM_MEM_MEMORY_DATA_DATA_FIELD_MASK 0xFFFFFFFF
#define PSRAM_MEM_MEMORY_DATA_DATA_FIELD_WIDTH 32
#define PSRAM_MEM_MEMORY_DATA_DATA_FIELD_SHIFT 0
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec PSRAM_MEM_MEMORY_DATA_DATA_FIELD;
#endif
extern const ru_reg_rec PSRAM_MEM_MEMORY_DATA_REG;
#define PSRAM_MEM_MEMORY_DATA_REG_OFFSET 0x00000000
#define PSRAM_MEM_MEMORY_DATA_REG_RAM_CNT 131072

extern const ru_block_rec PSRAM_MEM_BLOCK;

#endif
