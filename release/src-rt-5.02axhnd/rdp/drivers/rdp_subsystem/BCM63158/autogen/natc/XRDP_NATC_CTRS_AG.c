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
 * Field: NATC_CTRS_TBL_CACHE_HIT_COUNT_CACHE_HIT_COUNT
 ******************************************************************************/
const ru_field_rec NATC_CTRS_TBL_CACHE_HIT_COUNT_CACHE_HIT_COUNT_FIELD =
{
    "CACHE_HIT_COUNT",
#if RU_INCLUDE_DESC
    "",
    "32-bit total cache hit count value for statistics collection",
#endif
    NATC_CTRS_TBL_CACHE_HIT_COUNT_CACHE_HIT_COUNT_FIELD_MASK,
    0,
    NATC_CTRS_TBL_CACHE_HIT_COUNT_CACHE_HIT_COUNT_FIELD_WIDTH,
    NATC_CTRS_TBL_CACHE_HIT_COUNT_CACHE_HIT_COUNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_CTRS_TBL_CACHE_MISS_COUNT_CACHE_MISS_COUNT
 ******************************************************************************/
const ru_field_rec NATC_CTRS_TBL_CACHE_MISS_COUNT_CACHE_MISS_COUNT_FIELD =
{
    "CACHE_MISS_COUNT",
#if RU_INCLUDE_DESC
    "",
    "32-bit total cache miss count value for statistics collection",
#endif
    NATC_CTRS_TBL_CACHE_MISS_COUNT_CACHE_MISS_COUNT_FIELD_MASK,
    0,
    NATC_CTRS_TBL_CACHE_MISS_COUNT_CACHE_MISS_COUNT_FIELD_WIDTH,
    NATC_CTRS_TBL_CACHE_MISS_COUNT_CACHE_MISS_COUNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_CTRS_TBL_DDR_REQUEST_COUNT_DDR_REQUEST_COUNT
 ******************************************************************************/
const ru_field_rec NATC_CTRS_TBL_DDR_REQUEST_COUNT_DDR_REQUEST_COUNT_FIELD =
{
    "DDR_REQUEST_COUNT",
#if RU_INCLUDE_DESC
    "",
    "32-bit total DDR request count value for statistics collection",
#endif
    NATC_CTRS_TBL_DDR_REQUEST_COUNT_DDR_REQUEST_COUNT_FIELD_MASK,
    0,
    NATC_CTRS_TBL_DDR_REQUEST_COUNT_DDR_REQUEST_COUNT_FIELD_WIDTH,
    NATC_CTRS_TBL_DDR_REQUEST_COUNT_DDR_REQUEST_COUNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_CTRS_TBL_DDR_EVICT_COUNT_DDR_EVICT_COUNT
 ******************************************************************************/
const ru_field_rec NATC_CTRS_TBL_DDR_EVICT_COUNT_DDR_EVICT_COUNT_FIELD =
{
    "DDR_EVICT_COUNT",
#if RU_INCLUDE_DESC
    "",
    "32-bit total DDR evict count value for statistics collection",
#endif
    NATC_CTRS_TBL_DDR_EVICT_COUNT_DDR_EVICT_COUNT_FIELD_MASK,
    0,
    NATC_CTRS_TBL_DDR_EVICT_COUNT_DDR_EVICT_COUNT_FIELD_WIDTH,
    NATC_CTRS_TBL_DDR_EVICT_COUNT_DDR_EVICT_COUNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NATC_CTRS_TBL_DDR_BLOCK_COUNT_DDR_BLOCK_COUNT
 ******************************************************************************/
const ru_field_rec NATC_CTRS_TBL_DDR_BLOCK_COUNT_DDR_BLOCK_COUNT_FIELD =
{
    "DDR_BLOCK_COUNT",
#if RU_INCLUDE_DESC
    "",
    "32-bit total DDR blocked access count value for statistics collection",
#endif
    NATC_CTRS_TBL_DDR_BLOCK_COUNT_DDR_BLOCK_COUNT_FIELD_MASK,
    0,
    NATC_CTRS_TBL_DDR_BLOCK_COUNT_DDR_BLOCK_COUNT_FIELD_WIDTH,
    NATC_CTRS_TBL_DDR_BLOCK_COUNT_DDR_BLOCK_COUNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: NATC_CTRS_TBL_CACHE_HIT_COUNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_CTRS_TBL_CACHE_HIT_COUNT_FIELDS[] =
{
    &NATC_CTRS_TBL_CACHE_HIT_COUNT_CACHE_HIT_COUNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec NATC_CTRS_TBL_CACHE_HIT_COUNT_REG = 
{
    "TBL_CACHE_HIT_COUNT",
#if RU_INCLUDE_DESC
    "NAT table NAT Cache Hit Count",
    "NATC CACHE HIT COUNT",
#endif
    NATC_CTRS_TBL_CACHE_HIT_COUNT_REG_OFFSET,
    0,
    0,
    953,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    NATC_CTRS_TBL_CACHE_HIT_COUNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: NATC_CTRS_TBL_CACHE_MISS_COUNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_CTRS_TBL_CACHE_MISS_COUNT_FIELDS[] =
{
    &NATC_CTRS_TBL_CACHE_MISS_COUNT_CACHE_MISS_COUNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec NATC_CTRS_TBL_CACHE_MISS_COUNT_REG = 
{
    "TBL_CACHE_MISS_COUNT",
#if RU_INCLUDE_DESC
    "NAT table NAT Cache Miss Count",
    "NATC CACHE MISS COUNT",
#endif
    NATC_CTRS_TBL_CACHE_MISS_COUNT_REG_OFFSET,
    0,
    0,
    954,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    NATC_CTRS_TBL_CACHE_MISS_COUNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: NATC_CTRS_TBL_DDR_REQUEST_COUNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_CTRS_TBL_DDR_REQUEST_COUNT_FIELDS[] =
{
    &NATC_CTRS_TBL_DDR_REQUEST_COUNT_DDR_REQUEST_COUNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec NATC_CTRS_TBL_DDR_REQUEST_COUNT_REG = 
{
    "TBL_DDR_REQUEST_COUNT",
#if RU_INCLUDE_DESC
    "NAT table NAT DDR Request Count",
    "NATC DDR REQUEST COUNT",
#endif
    NATC_CTRS_TBL_DDR_REQUEST_COUNT_REG_OFFSET,
    0,
    0,
    955,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    NATC_CTRS_TBL_DDR_REQUEST_COUNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: NATC_CTRS_TBL_DDR_EVICT_COUNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_CTRS_TBL_DDR_EVICT_COUNT_FIELDS[] =
{
    &NATC_CTRS_TBL_DDR_EVICT_COUNT_DDR_EVICT_COUNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec NATC_CTRS_TBL_DDR_EVICT_COUNT_REG = 
{
    "TBL_DDR_EVICT_COUNT",
#if RU_INCLUDE_DESC
    "NAT table NAT DDR Evict Count",
    "NATC DDR EVICT COUNT",
#endif
    NATC_CTRS_TBL_DDR_EVICT_COUNT_REG_OFFSET,
    0,
    0,
    956,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    NATC_CTRS_TBL_DDR_EVICT_COUNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: NATC_CTRS_TBL_DDR_BLOCK_COUNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_CTRS_TBL_DDR_BLOCK_COUNT_FIELDS[] =
{
    &NATC_CTRS_TBL_DDR_BLOCK_COUNT_DDR_BLOCK_COUNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec NATC_CTRS_TBL_DDR_BLOCK_COUNT_REG = 
{
    "TBL_DDR_BLOCK_COUNT",
#if RU_INCLUDE_DESC
    "NAT table NAT DDR Block Count",
    "DDR BLOCK COUNT",
#endif
    NATC_CTRS_TBL_DDR_BLOCK_COUNT_REG_OFFSET,
    0,
    0,
    957,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    NATC_CTRS_TBL_DDR_BLOCK_COUNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Block: NATC_CTRS
 ******************************************************************************/
static const ru_reg_rec *NATC_CTRS_REGS[] =
{
    &NATC_CTRS_TBL_CACHE_HIT_COUNT_REG,
    &NATC_CTRS_TBL_CACHE_MISS_COUNT_REG,
    &NATC_CTRS_TBL_DDR_REQUEST_COUNT_REG,
    &NATC_CTRS_TBL_DDR_EVICT_COUNT_REG,
    &NATC_CTRS_TBL_DDR_BLOCK_COUNT_REG,
};

unsigned long NATC_CTRS_ADDRS[] =
{
    0x82e50350,
    0x82e50364,
    0x82e50378,
    0x82e5038c,
    0x82e503a0,
    0x82e503b4,
    0x82e503c8,
    0x82e503dc,
};

const ru_block_rec NATC_CTRS_BLOCK = 
{
    "NATC_CTRS",
    NATC_CTRS_ADDRS,
    8,
    5,
    NATC_CTRS_REGS
};

/* End of file XRDP_NATC_CTRS.c */
