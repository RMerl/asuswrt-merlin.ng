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

/******************************************************************************
 * Register: NATC_CTRS_TBL_CACHE_HIT_COUNT
 ******************************************************************************/
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
    829,
};

/******************************************************************************
 * Register: NATC_CTRS_TBL_CACHE_MISS_COUNT
 ******************************************************************************/
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
    830,
};

/******************************************************************************
 * Register: NATC_CTRS_TBL_DDR_REQUEST_COUNT
 ******************************************************************************/
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
    831,
};

/******************************************************************************
 * Register: NATC_CTRS_TBL_DDR_EVICT_COUNT
 ******************************************************************************/
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
    832,
};

/******************************************************************************
 * Register: NATC_CTRS_TBL_DDR_BLOCK_COUNT
 ******************************************************************************/
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
    833,
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
    0x82e50310,
    0x82e50324,
    0x82e50338,
    0x82e5034c,
    0x82e50360,
    0x82e50374,
    0x82e50388,
    0x82e5039c,
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
