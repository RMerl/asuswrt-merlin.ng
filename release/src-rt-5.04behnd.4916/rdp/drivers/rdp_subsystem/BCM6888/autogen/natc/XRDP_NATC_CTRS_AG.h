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
