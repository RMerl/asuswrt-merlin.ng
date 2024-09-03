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


#ifndef _XRDP_NATC_CTRS_AG_H_
#define _XRDP_NATC_CTRS_AG_H_

#include "ru_types.h"

#define NATC_CTRS_CACHE_HIT_COUNT_CACHE_HIT_COUNT_FIELD_MASK 0xFFFFFFFF
#define NATC_CTRS_CACHE_HIT_COUNT_CACHE_HIT_COUNT_FIELD_WIDTH 32
#define NATC_CTRS_CACHE_HIT_COUNT_CACHE_HIT_COUNT_FIELD_SHIFT 0
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec NATC_CTRS_CACHE_HIT_COUNT_CACHE_HIT_COUNT_FIELD;
#endif
extern const ru_reg_rec NATC_CTRS_CACHE_HIT_COUNT_REG;
#define NATC_CTRS_CACHE_HIT_COUNT_REG_OFFSET 0x00000358

#define NATC_CTRS_CACHE_MISS_COUNT_CACHE_MISS_COUNT_FIELD_MASK 0xFFFFFFFF
#define NATC_CTRS_CACHE_MISS_COUNT_CACHE_MISS_COUNT_FIELD_WIDTH 32
#define NATC_CTRS_CACHE_MISS_COUNT_CACHE_MISS_COUNT_FIELD_SHIFT 0
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec NATC_CTRS_CACHE_MISS_COUNT_CACHE_MISS_COUNT_FIELD;
#endif
extern const ru_reg_rec NATC_CTRS_CACHE_MISS_COUNT_REG;
#define NATC_CTRS_CACHE_MISS_COUNT_REG_OFFSET 0x0000035C

#define NATC_CTRS_DDR_REQUEST_COUNT_DDR_REQUEST_COUNT_FIELD_MASK 0xFFFFFFFF
#define NATC_CTRS_DDR_REQUEST_COUNT_DDR_REQUEST_COUNT_FIELD_WIDTH 32
#define NATC_CTRS_DDR_REQUEST_COUNT_DDR_REQUEST_COUNT_FIELD_SHIFT 0
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec NATC_CTRS_DDR_REQUEST_COUNT_DDR_REQUEST_COUNT_FIELD;
#endif
extern const ru_reg_rec NATC_CTRS_DDR_REQUEST_COUNT_REG;
#define NATC_CTRS_DDR_REQUEST_COUNT_REG_OFFSET 0x00000360

#define NATC_CTRS_DDR_EVICT_COUNT_DDR_EVICT_COUNT_FIELD_MASK 0xFFFFFFFF
#define NATC_CTRS_DDR_EVICT_COUNT_DDR_EVICT_COUNT_FIELD_WIDTH 32
#define NATC_CTRS_DDR_EVICT_COUNT_DDR_EVICT_COUNT_FIELD_SHIFT 0
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec NATC_CTRS_DDR_EVICT_COUNT_DDR_EVICT_COUNT_FIELD;
#endif
extern const ru_reg_rec NATC_CTRS_DDR_EVICT_COUNT_REG;
#define NATC_CTRS_DDR_EVICT_COUNT_REG_OFFSET 0x00000364

#define NATC_CTRS_DDR_BLOCK_COUNT_DDR_BLOCK_COUNT_FIELD_MASK 0xFFFFFFFF
#define NATC_CTRS_DDR_BLOCK_COUNT_DDR_BLOCK_COUNT_FIELD_WIDTH 32
#define NATC_CTRS_DDR_BLOCK_COUNT_DDR_BLOCK_COUNT_FIELD_SHIFT 0
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec NATC_CTRS_DDR_BLOCK_COUNT_DDR_BLOCK_COUNT_FIELD;
#endif
extern const ru_reg_rec NATC_CTRS_DDR_BLOCK_COUNT_REG;
#define NATC_CTRS_DDR_BLOCK_COUNT_REG_OFFSET 0x00000368

extern const ru_block_rec NATC_CTRS_BLOCK;

#endif
