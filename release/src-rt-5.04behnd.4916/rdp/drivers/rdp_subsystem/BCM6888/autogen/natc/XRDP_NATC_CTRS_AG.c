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


#include "XRDP_NATC_CTRS_AG.h"

/******************************************************************************
 * Register: NAME: NATC_CTRS_CACHE_HIT_COUNT, TYPE: Type_NATC_CACHE_HIT_COUNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CACHE_HIT_COUNT *****/
const ru_field_rec NATC_CTRS_CACHE_HIT_COUNT_CACHE_HIT_COUNT_FIELD =
{
    "CACHE_HIT_COUNT",
#if RU_INCLUDE_DESC
    "",
    "32-bit total cache hit count value for statistics collection\n",
#endif
    { NATC_CTRS_CACHE_HIT_COUNT_CACHE_HIT_COUNT_FIELD_MASK },
    0,
    { NATC_CTRS_CACHE_HIT_COUNT_CACHE_HIT_COUNT_FIELD_WIDTH },
    { NATC_CTRS_CACHE_HIT_COUNT_CACHE_HIT_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_CTRS_CACHE_HIT_COUNT_FIELDS[] =
{
    &NATC_CTRS_CACHE_HIT_COUNT_CACHE_HIT_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NATC_CTRS_CACHE_HIT_COUNT *****/
const ru_reg_rec NATC_CTRS_CACHE_HIT_COUNT_REG =
{
    "CACHE_HIT_COUNT",
#if RU_INCLUDE_DESC
    "NAT Cache Hit Count",
    "NATC CACHE HIT COUNT\n",
#endif
    { NATC_CTRS_CACHE_HIT_COUNT_REG_OFFSET },
    0,
    0,
    631,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    NATC_CTRS_CACHE_HIT_COUNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NATC_CTRS_CACHE_MISS_COUNT, TYPE: Type_NATC_CACHE_MISS_COUNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CACHE_MISS_COUNT *****/
const ru_field_rec NATC_CTRS_CACHE_MISS_COUNT_CACHE_MISS_COUNT_FIELD =
{
    "CACHE_MISS_COUNT",
#if RU_INCLUDE_DESC
    "",
    "32-bit total cache miss count value for statistics collection\n",
#endif
    { NATC_CTRS_CACHE_MISS_COUNT_CACHE_MISS_COUNT_FIELD_MASK },
    0,
    { NATC_CTRS_CACHE_MISS_COUNT_CACHE_MISS_COUNT_FIELD_WIDTH },
    { NATC_CTRS_CACHE_MISS_COUNT_CACHE_MISS_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_CTRS_CACHE_MISS_COUNT_FIELDS[] =
{
    &NATC_CTRS_CACHE_MISS_COUNT_CACHE_MISS_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NATC_CTRS_CACHE_MISS_COUNT *****/
const ru_reg_rec NATC_CTRS_CACHE_MISS_COUNT_REG =
{
    "CACHE_MISS_COUNT",
#if RU_INCLUDE_DESC
    "NAT Cache Miss Count",
    "NATC CACHE MISS COUNT\n",
#endif
    { NATC_CTRS_CACHE_MISS_COUNT_REG_OFFSET },
    0,
    0,
    632,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    NATC_CTRS_CACHE_MISS_COUNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NATC_CTRS_DDR_REQUEST_COUNT, TYPE: Type_NATC_DDR_REQUEST_COUNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_REQUEST_COUNT *****/
const ru_field_rec NATC_CTRS_DDR_REQUEST_COUNT_DDR_REQUEST_COUNT_FIELD =
{
    "DDR_REQUEST_COUNT",
#if RU_INCLUDE_DESC
    "",
    "32-bit total DDR request count value for statistics collection\n",
#endif
    { NATC_CTRS_DDR_REQUEST_COUNT_DDR_REQUEST_COUNT_FIELD_MASK },
    0,
    { NATC_CTRS_DDR_REQUEST_COUNT_DDR_REQUEST_COUNT_FIELD_WIDTH },
    { NATC_CTRS_DDR_REQUEST_COUNT_DDR_REQUEST_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_CTRS_DDR_REQUEST_COUNT_FIELDS[] =
{
    &NATC_CTRS_DDR_REQUEST_COUNT_DDR_REQUEST_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NATC_CTRS_DDR_REQUEST_COUNT *****/
const ru_reg_rec NATC_CTRS_DDR_REQUEST_COUNT_REG =
{
    "DDR_REQUEST_COUNT",
#if RU_INCLUDE_DESC
    "DDR Request Count",
    "NATC DDR REQUEST COUNT\n",
#endif
    { NATC_CTRS_DDR_REQUEST_COUNT_REG_OFFSET },
    0,
    0,
    633,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    NATC_CTRS_DDR_REQUEST_COUNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NATC_CTRS_DDR_EVICT_COUNT, TYPE: Type_NATC_DDR_EVICT_COUNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_EVICT_COUNT *****/
const ru_field_rec NATC_CTRS_DDR_EVICT_COUNT_DDR_EVICT_COUNT_FIELD =
{
    "DDR_EVICT_COUNT",
#if RU_INCLUDE_DESC
    "",
    "32-bit total DDR evict count value for statistics collection.\nIt does not include the flush command evict count.\n",
#endif
    { NATC_CTRS_DDR_EVICT_COUNT_DDR_EVICT_COUNT_FIELD_MASK },
    0,
    { NATC_CTRS_DDR_EVICT_COUNT_DDR_EVICT_COUNT_FIELD_WIDTH },
    { NATC_CTRS_DDR_EVICT_COUNT_DDR_EVICT_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_CTRS_DDR_EVICT_COUNT_FIELDS[] =
{
    &NATC_CTRS_DDR_EVICT_COUNT_DDR_EVICT_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NATC_CTRS_DDR_EVICT_COUNT *****/
const ru_reg_rec NATC_CTRS_DDR_EVICT_COUNT_REG =
{
    "DDR_EVICT_COUNT",
#if RU_INCLUDE_DESC
    "NAT DDR Evict Count",
    "NATC DDR EVICT COUNT\n",
#endif
    { NATC_CTRS_DDR_EVICT_COUNT_REG_OFFSET },
    0,
    0,
    634,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    NATC_CTRS_DDR_EVICT_COUNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: NATC_CTRS_DDR_BLOCK_COUNT, TYPE: Type_NATC_DDR_BLOCK_COUNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR_BLOCK_COUNT *****/
const ru_field_rec NATC_CTRS_DDR_BLOCK_COUNT_DDR_BLOCK_COUNT_FIELD =
{
    "DDR_BLOCK_COUNT",
#if RU_INCLUDE_DESC
    "",
    "32-bit total DDR blocked access count value for statistics collection\n",
#endif
    { NATC_CTRS_DDR_BLOCK_COUNT_DDR_BLOCK_COUNT_FIELD_MASK },
    0,
    { NATC_CTRS_DDR_BLOCK_COUNT_DDR_BLOCK_COUNT_FIELD_WIDTH },
    { NATC_CTRS_DDR_BLOCK_COUNT_DDR_BLOCK_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_CTRS_DDR_BLOCK_COUNT_FIELDS[] =
{
    &NATC_CTRS_DDR_BLOCK_COUNT_DDR_BLOCK_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: NATC_CTRS_DDR_BLOCK_COUNT *****/
const ru_reg_rec NATC_CTRS_DDR_BLOCK_COUNT_REG =
{
    "DDR_BLOCK_COUNT",
#if RU_INCLUDE_DESC
    "NAT DDR Block Count",
    "DDR BLOCK COUNT\n",
#endif
    { NATC_CTRS_DDR_BLOCK_COUNT_REG_OFFSET },
    0,
    0,
    635,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    NATC_CTRS_DDR_BLOCK_COUNT_FIELDS,
#endif
};

unsigned long NATC_CTRS_ADDRS[] =
{
    0x82950000,
};

static const ru_reg_rec *NATC_CTRS_REGS[] =
{
    &NATC_CTRS_CACHE_HIT_COUNT_REG,
    &NATC_CTRS_CACHE_MISS_COUNT_REG,
    &NATC_CTRS_DDR_REQUEST_COUNT_REG,
    &NATC_CTRS_DDR_EVICT_COUNT_REG,
    &NATC_CTRS_DDR_BLOCK_COUNT_REG,
};

const ru_block_rec NATC_CTRS_BLOCK =
{
    "NATC_CTRS",
    NATC_CTRS_ADDRS,
    1,
    5,
    NATC_CTRS_REGS,
};
