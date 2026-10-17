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


#include "XRDP_FPM_AG.h"

/******************************************************************************
 * Register: NAME: FPM_CTL, TYPE: Type_FPM_BLOCK_FPM_TOP_CTL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TP_MUX_CNTRL *****/
const ru_field_rec FPM_CTL_TP_MUX_CNTRL_FIELD =
{
    "TP_MUX_CNTRL",
#if RU_INCLUDE_DESC
    "",
    "Test port mux control bits used to drive test signals from different submodules.\n",
#endif
    { FPM_CTL_TP_MUX_CNTRL_FIELD_MASK },
    0,
    { FPM_CTL_TP_MUX_CNTRL_FIELD_WIDTH },
    { FPM_CTL_TP_MUX_CNTRL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_CTL_FIELDS[] =
{
    &FPM_CTL_TP_MUX_CNTRL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_CTL *****/
const ru_reg_rec FPM_CTL_REG =
{
    "CTL",
#if RU_INCLUDE_DESC
    "CTL Register",
    "No Description\n",
#endif
    { FPM_CTL_REG_OFFSET },
    0,
    0,
    1447,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_CTL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_WEIGHT, TYPE: Type_FPM_BLOCK_FPM_TOP_WEIGHT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CORE_0_ALLOC_WEIGHT *****/
const ru_field_rec FPM_WEIGHT_CORE_0_ALLOC_WEIGHT_FIELD =
{
    "CORE_0_ALLOC_WEIGHT",
#if RU_INCLUDE_DESC
    "",
    "Weight assigned to each alloc from pool for CORE_0\n",
#endif
    { FPM_WEIGHT_CORE_0_ALLOC_WEIGHT_FIELD_MASK },
    0,
    { FPM_WEIGHT_CORE_0_ALLOC_WEIGHT_FIELD_WIDTH },
    { FPM_WEIGHT_CORE_0_ALLOC_WEIGHT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CORE_0_FREE_WEIGHT *****/
const ru_field_rec FPM_WEIGHT_CORE_0_FREE_WEIGHT_FIELD =
{
    "CORE_0_FREE_WEIGHT",
#if RU_INCLUDE_DESC
    "",
    "Weight assigned to each free to pool for CORE_0\n",
#endif
    { FPM_WEIGHT_CORE_0_FREE_WEIGHT_FIELD_MASK },
    0,
    { FPM_WEIGHT_CORE_0_FREE_WEIGHT_FIELD_WIDTH },
    { FPM_WEIGHT_CORE_0_FREE_WEIGHT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CORE_1_ALLOC_WEIGHT *****/
const ru_field_rec FPM_WEIGHT_CORE_1_ALLOC_WEIGHT_FIELD =
{
    "CORE_1_ALLOC_WEIGHT",
#if RU_INCLUDE_DESC
    "",
    "Weight assigned to each alloc from pool for CORE_1\n",
#endif
    { FPM_WEIGHT_CORE_1_ALLOC_WEIGHT_FIELD_MASK },
    0,
    { FPM_WEIGHT_CORE_1_ALLOC_WEIGHT_FIELD_WIDTH },
    { FPM_WEIGHT_CORE_1_ALLOC_WEIGHT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CORE_1_FREE_WEIGHT *****/
const ru_field_rec FPM_WEIGHT_CORE_1_FREE_WEIGHT_FIELD =
{
    "CORE_1_FREE_WEIGHT",
#if RU_INCLUDE_DESC
    "",
    "Weight assigned to each free to pool for CORE_1\n",
#endif
    { FPM_WEIGHT_CORE_1_FREE_WEIGHT_FIELD_MASK },
    0,
    { FPM_WEIGHT_CORE_1_FREE_WEIGHT_FIELD_WIDTH },
    { FPM_WEIGHT_CORE_1_FREE_WEIGHT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_WEIGHT_FIELDS[] =
{
    &FPM_WEIGHT_CORE_0_ALLOC_WEIGHT_FIELD,
    &FPM_WEIGHT_CORE_0_FREE_WEIGHT_FIELD,
    &FPM_WEIGHT_CORE_1_ALLOC_WEIGHT_FIELD,
    &FPM_WEIGHT_CORE_1_FREE_WEIGHT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_WEIGHT *****/
const ru_reg_rec FPM_WEIGHT_REG =
{
    "WEIGHT",
#if RU_INCLUDE_DESC
    "WEIGHT Register",
    "No Description\n",
#endif
    { FPM_WEIGHT_REG_OFFSET },
    0,
    0,
    1448,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    FPM_WEIGHT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_BB_CFG, TYPE: Type_FPM_BLOCK_FPM_TOP_BB_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BB_CORE_SEL *****/
const ru_field_rec FPM_BB_CFG_BB_CORE_SEL_FIELD =
{
    "BB_CORE_SEL",
#if RU_INCLUDE_DESC
    "",
    "Select CORE to be used when BB allocates tokens 11 = reserved 10 = allocate from both cores 01 = CORE_1 00 = CORE_0\n",
#endif
    { FPM_BB_CFG_BB_CORE_SEL_FIELD_MASK },
    0,
    { FPM_BB_CFG_BB_CORE_SEL_FIELD_WIDTH },
    { FPM_BB_CFG_BB_CORE_SEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R3 *****/
const ru_field_rec FPM_BB_CFG_R3_FIELD =
{
    "R3",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_BB_CFG_R3_FIELD_MASK },
    0,
    { FPM_BB_CFG_R3_FIELD_WIDTH },
    { FPM_BB_CFG_R3_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BB_FAST_ACK *****/
const ru_field_rec FPM_BB_CFG_BB_FAST_ACK_FIELD =
{
    "BB_FAST_ACK",
#if RU_INCLUDE_DESC
    "",
    "Enables fast-ack mode for Broadbus 0 = disabled - will wait to for token if pool is not empty 1 = enabled - will return invalid token if alloc_fifo is empty\n",
#endif
    { FPM_BB_CFG_BB_FAST_ACK_FIELD_MASK },
    0,
    { FPM_BB_CFG_BB_FAST_ACK_FIELD_WIDTH },
    { FPM_BB_CFG_BB_FAST_ACK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R2 *****/
const ru_field_rec FPM_BB_CFG_R2_FIELD =
{
    "R2",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_BB_CFG_R2_FIELD_MASK },
    0,
    { FPM_BB_CFG_R2_FIELD_WIDTH },
    { FPM_BB_CFG_R2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BB_SOFT_RESET *****/
const ru_field_rec FPM_BB_CFG_BB_SOFT_RESET_FIELD =
{
    "BB_SOFT_RESET",
#if RU_INCLUDE_DESC
    "",
    "Set to 1 to hold the FPM Broadbus interface in reset. This is useful for maintaining a known state on that interface when Runner is powered down.\n",
#endif
    { FPM_BB_CFG_BB_SOFT_RESET_FIELD_MASK },
    0,
    { FPM_BB_CFG_BB_SOFT_RESET_FIELD_WIDTH },
    { FPM_BB_CFG_BB_SOFT_RESET_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec FPM_BB_CFG_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_BB_CFG_R1_FIELD_MASK },
    0,
    { FPM_BB_CFG_R1_FIELD_WIDTH },
    { FPM_BB_CFG_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_BB_CFG_FIELDS[] =
{
    &FPM_BB_CFG_BB_CORE_SEL_FIELD,
    &FPM_BB_CFG_R3_FIELD,
    &FPM_BB_CFG_BB_FAST_ACK_FIELD,
    &FPM_BB_CFG_R2_FIELD,
    &FPM_BB_CFG_BB_SOFT_RESET_FIELD,
    &FPM_BB_CFG_R1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_BB_CFG *****/
const ru_reg_rec FPM_BB_CFG_REG =
{
    "BB_CFG",
#if RU_INCLUDE_DESC
    "BB_CFG Register",
    "No Description\n",
#endif
    { FPM_BB_CFG_REG_OFFSET },
    0,
    0,
    1449,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    FPM_BB_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_SPARE, TYPE: Type_FPM_BLOCK_FPM_TOP_SPARE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SPARE_BITS *****/
const ru_field_rec FPM_SPARE_SPARE_BITS_FIELD =
{
    "SPARE_BITS",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_SPARE_SPARE_BITS_FIELD_MASK },
    0,
    { FPM_SPARE_SPARE_BITS_FIELD_WIDTH },
    { FPM_SPARE_SPARE_BITS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_SPARE_FIELDS[] =
{
    &FPM_SPARE_SPARE_BITS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_SPARE *****/
const ru_reg_rec FPM_SPARE_REG =
{
    "SPARE",
#if RU_INCLUDE_DESC
    "SPARE Register",
    "No Description\n",
#endif
    { FPM_SPARE_REG_OFFSET },
    0,
    0,
    1450,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_SPARE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_PRBS_INVALID_GEN, TYPE: Type_FPM_BLOCK_FPM_TOP_PRBS_INVALID_GEN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MASK *****/
const ru_field_rec FPM_PRBS_INVALID_GEN_MASK_FIELD =
{
    "MASK",
#if RU_INCLUDE_DESC
    "",
    "ORing bit mask for output PRBS signal.\n",
#endif
    { FPM_PRBS_INVALID_GEN_MASK_FIELD_MASK },
    0,
    { FPM_PRBS_INVALID_GEN_MASK_FIELD_WIDTH },
    { FPM_PRBS_INVALID_GEN_MASK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ENABLE *****/
const ru_field_rec FPM_PRBS_INVALID_GEN_ENABLE_FIELD =
{
    "ENABLE",
#if RU_INCLUDE_DESC
    "",
    "Enable PRBS invalid token generation.\n",
#endif
    { FPM_PRBS_INVALID_GEN_ENABLE_FIELD_MASK },
    0,
    { FPM_PRBS_INVALID_GEN_ENABLE_FIELD_WIDTH },
    { FPM_PRBS_INVALID_GEN_ENABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_PRBS_INVALID_GEN_FIELDS[] =
{
    &FPM_PRBS_INVALID_GEN_MASK_FIELD,
    &FPM_PRBS_INVALID_GEN_ENABLE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_PRBS_INVALID_GEN *****/
const ru_reg_rec FPM_PRBS_INVALID_GEN_REG =
{
    "PRBS_INVALID_GEN",
#if RU_INCLUDE_DESC
    "PRBS_INVALID_GEN Register",
    "No Description\n",
#endif
    { FPM_PRBS_INVALID_GEN_REG_OFFSET },
    0,
    0,
    1451,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_PRBS_INVALID_GEN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_CTL_1, TYPE: Type_FPM_BLOCK_FPM_POOL_0_CTL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: R3 *****/
const ru_field_rec FPM_CTL_1_R3_FIELD =
{
    "R3",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_CTL_1_R3_FIELD_MASK },
    0,
    { FPM_CTL_1_R3_FIELD_WIDTH },
    { FPM_CTL_1_R3_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: INIT_MEM *****/
const ru_field_rec FPM_CTL_1_INIT_MEM_FIELD =
{
    "INIT_MEM",
#if RU_INCLUDE_DESC
    "",
    "Clear memory - Initialize all bits of the usage index array memory to zero's\nThis is a self clearing bit. Once software writes a 1'b1 to enable, hardware initializes the memory and resets this bit back to 1'b0 at completion of initialization. Software can poll this bit and check for a value a zero that indicates initialization completion status\n",
#endif
    { FPM_CTL_1_INIT_MEM_FIELD_MASK },
    0,
    { FPM_CTL_1_INIT_MEM_FIELD_WIDTH },
    { FPM_CTL_1_INIT_MEM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R2 *****/
const ru_field_rec FPM_CTL_1_R2_FIELD =
{
    "R2",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_CTL_1_R2_FIELD_MASK },
    0,
    { FPM_CTL_1_R2_FIELD_WIDTH },
    { FPM_CTL_1_R2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_ENABLE *****/
const ru_field_rec FPM_CTL_1_POOL_ENABLE_FIELD =
{
    "POOL_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "Enable POOL token allocation / deallocation\n0 = Disabled\n1 = Enabled\n\n",
#endif
    { FPM_CTL_1_POOL_ENABLE_FIELD_MASK },
    0,
    { FPM_CTL_1_POOL_ENABLE_FIELD_WIDTH },
    { FPM_CTL_1_POOL_ENABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec FPM_CTL_1_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_CTL_1_R1_FIELD_MASK },
    0,
    { FPM_CTL_1_R1_FIELD_WIDTH },
    { FPM_CTL_1_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: STOP_ALLOC_CACHE_LOAD *****/
const ru_field_rec FPM_CTL_1_STOP_ALLOC_CACHE_LOAD_FIELD =
{
    "STOP_ALLOC_CACHE_LOAD",
#if RU_INCLUDE_DESC
    "",
    "Stop loading allocation fifo/cache with new tokens. This is should be used for debug purposes only\n0 = Enable loading new tokens (normal operation)\n1 = Disable loading new tokens\n\n",
#endif
    { FPM_CTL_1_STOP_ALLOC_CACHE_LOAD_FIELD_MASK },
    0,
    { FPM_CTL_1_STOP_ALLOC_CACHE_LOAD_FIELD_WIDTH },
    { FPM_CTL_1_STOP_ALLOC_CACHE_LOAD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MEM_CORRUPT_CHECK_DISABLE *****/
const ru_field_rec FPM_CTL_1_MEM_CORRUPT_CHECK_DISABLE_FIELD =
{
    "MEM_CORRUPT_CHECK_DISABLE",
#if RU_INCLUDE_DESC
    "",
    "Disabling check for index memory corrupt during alloc/free/mcast updates. This should be used for debug purposes only\n0 = Enable memory corruption check (normal operation)\n1 = Disable memory corruption check\n\n",
#endif
    { FPM_CTL_1_MEM_CORRUPT_CHECK_DISABLE_FIELD_MASK },
    0,
    { FPM_CTL_1_MEM_CORRUPT_CHECK_DISABLE_FIELD_WIDTH },
    { FPM_CTL_1_MEM_CORRUPT_CHECK_DISABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TP_MUX_CNTRL *****/
const ru_field_rec FPM_CTL_1_TP_MUX_CNTRL_FIELD =
{
    "TP_MUX_CNTRL",
#if RU_INCLUDE_DESC
    "",
    "Test port mux control bits used to drive test signals from different submodules.\n",
#endif
    { FPM_CTL_1_TP_MUX_CNTRL_FIELD_MASK },
    0,
    { FPM_CTL_1_TP_MUX_CNTRL_FIELD_WIDTH },
    { FPM_CTL_1_TP_MUX_CNTRL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_CTL_1_FIELDS[] =
{
    &FPM_CTL_1_R3_FIELD,
    &FPM_CTL_1_INIT_MEM_FIELD,
    &FPM_CTL_1_R2_FIELD,
    &FPM_CTL_1_POOL_ENABLE_FIELD,
    &FPM_CTL_1_R1_FIELD,
    &FPM_CTL_1_STOP_ALLOC_CACHE_LOAD_FIELD,
    &FPM_CTL_1_MEM_CORRUPT_CHECK_DISABLE_FIELD,
    &FPM_CTL_1_TP_MUX_CNTRL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_CTL_1 *****/
const ru_reg_rec FPM_CTL_1_REG =
{
    "CTL_1",
#if RU_INCLUDE_DESC
    "CTL Register",
    "No Description\n",
#endif
    { FPM_CTL_1_REG_OFFSET },
    0,
    0,
    1452,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    FPM_CTL_1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_CFG1, TYPE: Type_FPM_BLOCK_FPM_POOL_0_CFG1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_SEARCH_MODE *****/
const ru_field_rec FPM_CFG1_POOL_SEARCH_MODE_FIELD =
{
    "POOL_SEARCH_MODE",
#if RU_INCLUDE_DESC
    "",
    "Index memory search method\n(For more info refer to FPM architecture wiki page)\n0 = Method 1\n1 = Method 2\n\n",
#endif
    { FPM_CFG1_POOL_SEARCH_MODE_FIELD_MASK },
    0,
    { FPM_CFG1_POOL_SEARCH_MODE_FIELD_WIDTH },
    { FPM_CFG1_POOL_SEARCH_MODE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ENABLE_JUMBO_SUPPORT *****/
const ru_field_rec FPM_CFG1_ENABLE_JUMBO_SUPPORT_FIELD =
{
    "ENABLE_JUMBO_SUPPORT",
#if RU_INCLUDE_DESC
    "",
    "Enable Jumbo Support0 = Disabled 1\n1 = Enabled 2\n\n",
#endif
    { FPM_CFG1_ENABLE_JUMBO_SUPPORT_FIELD_MASK },
    0,
    { FPM_CFG1_ENABLE_JUMBO_SUPPORT_FIELD_WIDTH },
    { FPM_CFG1_ENABLE_JUMBO_SUPPORT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R4 *****/
const ru_field_rec FPM_CFG1_R4_FIELD =
{
    "R4",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_CFG1_R4_FIELD_MASK },
    0,
    { FPM_CFG1_R4_FIELD_WIDTH },
    { FPM_CFG1_R4_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_0_DISABLE *****/
const ru_field_rec FPM_CFG1_POOL_0_DISABLE_FIELD =
{
    "POOL_0_DISABLE",
#if RU_INCLUDE_DESC
    "",
    "Pool search disables for invidual token sizes\n\n",
#endif
    { FPM_CFG1_POOL_0_DISABLE_FIELD_MASK },
    0,
    { FPM_CFG1_POOL_0_DISABLE_FIELD_WIDTH },
    { FPM_CFG1_POOL_0_DISABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_1_DISABLE *****/
const ru_field_rec FPM_CFG1_POOL_1_DISABLE_FIELD =
{
    "POOL_1_DISABLE",
#if RU_INCLUDE_DESC
    "",
    "Pool search disables for invidual token sizes\n\n",
#endif
    { FPM_CFG1_POOL_1_DISABLE_FIELD_MASK },
    0,
    { FPM_CFG1_POOL_1_DISABLE_FIELD_WIDTH },
    { FPM_CFG1_POOL_1_DISABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_2_DISABLE *****/
const ru_field_rec FPM_CFG1_POOL_2_DISABLE_FIELD =
{
    "POOL_2_DISABLE",
#if RU_INCLUDE_DESC
    "",
    "Pool search disables for invidual token sizes\n\n",
#endif
    { FPM_CFG1_POOL_2_DISABLE_FIELD_MASK },
    0,
    { FPM_CFG1_POOL_2_DISABLE_FIELD_WIDTH },
    { FPM_CFG1_POOL_2_DISABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_3_DISABLE *****/
const ru_field_rec FPM_CFG1_POOL_3_DISABLE_FIELD =
{
    "POOL_3_DISABLE",
#if RU_INCLUDE_DESC
    "",
    "Pool search disables for invidual token sizes\n\n",
#endif
    { FPM_CFG1_POOL_3_DISABLE_FIELD_MASK },
    0,
    { FPM_CFG1_POOL_3_DISABLE_FIELD_WIDTH },
    { FPM_CFG1_POOL_3_DISABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R3 *****/
const ru_field_rec FPM_CFG1_R3_FIELD =
{
    "R3",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_CFG1_R3_FIELD_MASK },
    0,
    { FPM_CFG1_R3_FIELD_WIDTH },
    { FPM_CFG1_R3_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MARK_EN *****/
const ru_field_rec FPM_CFG1_MARK_EN_FIELD =
{
    "MARK_EN",
#if RU_INCLUDE_DESC
    "",
    "Enables token marking mode\nThis bit enables token marking mode for token recovery at the expense of reducing the number of buffers in half.\n0 = Buffer marking disabled\n1 = Buffer marking enabled\n\n",
#endif
    { FPM_CFG1_MARK_EN_FIELD_MASK },
    0,
    { FPM_CFG1_MARK_EN_FIELD_WIDTH },
    { FPM_CFG1_MARK_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R2 *****/
const ru_field_rec FPM_CFG1_R2_FIELD =
{
    "R2",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_CFG1_R2_FIELD_MASK },
    0,
    { FPM_CFG1_R2_FIELD_WIDTH },
    { FPM_CFG1_R2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BUF_SIZE *****/
const ru_field_rec FPM_CFG1_BUF_SIZE_FIELD =
{
    "BUF_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Selects the size of the buffer to be used in the pool. All buffers must be the same size.\n0 - 512 byte buffers\n1 - 256 byte buffers\nall other values - reserved\n\n",
#endif
    { FPM_CFG1_BUF_SIZE_FIELD_MASK },
    0,
    { FPM_CFG1_BUF_SIZE_FIELD_WIDTH },
    { FPM_CFG1_BUF_SIZE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec FPM_CFG1_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_CFG1_R1_FIELD_MASK },
    0,
    { FPM_CFG1_R1_FIELD_WIDTH },
    { FPM_CFG1_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_CFG1_FIELDS[] =
{
    &FPM_CFG1_POOL_SEARCH_MODE_FIELD,
    &FPM_CFG1_ENABLE_JUMBO_SUPPORT_FIELD,
    &FPM_CFG1_R4_FIELD,
    &FPM_CFG1_POOL_0_DISABLE_FIELD,
    &FPM_CFG1_POOL_1_DISABLE_FIELD,
    &FPM_CFG1_POOL_2_DISABLE_FIELD,
    &FPM_CFG1_POOL_3_DISABLE_FIELD,
    &FPM_CFG1_R3_FIELD,
    &FPM_CFG1_MARK_EN_FIELD,
    &FPM_CFG1_R2_FIELD,
    &FPM_CFG1_BUF_SIZE_FIELD,
    &FPM_CFG1_R1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_CFG1 *****/
const ru_reg_rec FPM_CFG1_REG =
{
    "CFG1",
#if RU_INCLUDE_DESC
    "CFG1 Register",
    "No Description\n",
#endif
    { FPM_CFG1_REG_OFFSET },
    0,
    0,
    1453,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    12,
    FPM_CFG1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_CFG2, TYPE: Type_FPM_BLOCK_FPM_POOL_0_CFG2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec FPM_CFG2_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_CFG2_R1_FIELD_MASK },
    0,
    { FPM_CFG2_R1_FIELD_WIDTH },
    { FPM_CFG2_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_BASE_ADDRESS *****/
const ru_field_rec FPM_CFG2_POOL_BASE_ADDRESS_FIELD =
{
    "POOL_BASE_ADDRESS",
#if RU_INCLUDE_DESC
    "",
    "Buffer base address. 7:2 must be 0x00.\n\n",
#endif
    { FPM_CFG2_POOL_BASE_ADDRESS_FIELD_MASK },
    0,
    { FPM_CFG2_POOL_BASE_ADDRESS_FIELD_WIDTH },
    { FPM_CFG2_POOL_BASE_ADDRESS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_CFG2_FIELDS[] =
{
    &FPM_CFG2_R1_FIELD,
    &FPM_CFG2_POOL_BASE_ADDRESS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_CFG2 *****/
const ru_reg_rec FPM_CFG2_REG =
{
    "CFG2",
#if RU_INCLUDE_DESC
    "CFG2 Register",
    "This register sets the physical base address of this memory. The memory block should be the number of buffers times the buffer size. This is mainly used for multi-pool memory configuration. NOTE: POOL_BASE_ADDRESS[7:2] and reserved[1:0] field must be written with 0x00 in the BCM3382 because itstoken-to-address converter assumes the buffers start on a 2kB boundary.\n",
#endif
    { FPM_CFG2_REG_OFFSET },
    0,
    0,
    1454,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_CFG2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_INTR_MSK, TYPE: Type_FPM_BLOCK_FPM_POOL_0_INTR_MSK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ALLOC_FIFO_FULL_MSK *****/
const ru_field_rec FPM_INTR_MSK_ALLOC_FIFO_FULL_MSK_FIELD =
{
    "ALLOC_FIFO_FULL_MSK",
#if RU_INCLUDE_DESC
    "",
    "Allocation FIFO Full Interrupt mask.\n\n",
#endif
    { FPM_INTR_MSK_ALLOC_FIFO_FULL_MSK_FIELD_MASK },
    0,
    { FPM_INTR_MSK_ALLOC_FIFO_FULL_MSK_FIELD_WIDTH },
    { FPM_INTR_MSK_ALLOC_FIFO_FULL_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_FIFO_FULL_MSK *****/
const ru_field_rec FPM_INTR_MSK_FREE_FIFO_FULL_MSK_FIELD =
{
    "FREE_FIFO_FULL_MSK",
#if RU_INCLUDE_DESC
    "",
    "De-Allocation FIFO Full Interrupt mask.\n\n",
#endif
    { FPM_INTR_MSK_FREE_FIFO_FULL_MSK_FIELD_MASK },
    0,
    { FPM_INTR_MSK_FREE_FIFO_FULL_MSK_FIELD_WIDTH },
    { FPM_INTR_MSK_FREE_FIFO_FULL_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_FULL_MSK *****/
const ru_field_rec FPM_INTR_MSK_POOL_FULL_MSK_FIELD =
{
    "POOL_FULL_MSK",
#if RU_INCLUDE_DESC
    "",
    "Usage Index Pool is fully allocated interrupt mask.\n\n",
#endif
    { FPM_INTR_MSK_POOL_FULL_MSK_FIELD_MASK },
    0,
    { FPM_INTR_MSK_POOL_FULL_MSK_FIELD_WIDTH },
    { FPM_INTR_MSK_POOL_FULL_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_TOKEN_NO_VALID_MSK *****/
const ru_field_rec FPM_INTR_MSK_FREE_TOKEN_NO_VALID_MSK_FIELD =
{
    "FREE_TOKEN_NO_VALID_MSK",
#if RU_INCLUDE_DESC
    "",
    "De-allocation token request with invalid token.\n\n",
#endif
    { FPM_INTR_MSK_FREE_TOKEN_NO_VALID_MSK_FIELD_MASK },
    0,
    { FPM_INTR_MSK_FREE_TOKEN_NO_VALID_MSK_FIELD_WIDTH },
    { FPM_INTR_MSK_FREE_TOKEN_NO_VALID_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK *****/
const ru_field_rec FPM_INTR_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD =
{
    "FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK",
#if RU_INCLUDE_DESC
    "",
    "De-allocation token request with index out-of-range.\n\n",
#endif
    { FPM_INTR_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD_MASK },
    0,
    { FPM_INTR_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD_WIDTH },
    { FPM_INTR_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MULTI_TOKEN_NO_VALID_MSK *****/
const ru_field_rec FPM_INTR_MSK_MULTI_TOKEN_NO_VALID_MSK_FIELD =
{
    "MULTI_TOKEN_NO_VALID_MSK",
#if RU_INCLUDE_DESC
    "",
    "Token multi-cast value update request with invalid token.\n\n",
#endif
    { FPM_INTR_MSK_MULTI_TOKEN_NO_VALID_MSK_FIELD_MASK },
    0,
    { FPM_INTR_MSK_MULTI_TOKEN_NO_VALID_MSK_FIELD_WIDTH },
    { FPM_INTR_MSK_MULTI_TOKEN_NO_VALID_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK *****/
const ru_field_rec FPM_INTR_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD =
{
    "MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK",
#if RU_INCLUDE_DESC
    "",
    "Token multi-cast value update request with index out-of-range.\n\n",
#endif
    { FPM_INTR_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD_MASK },
    0,
    { FPM_INTR_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD_WIDTH },
    { FPM_INTR_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_DIS_FREE_MULTI_MSK *****/
const ru_field_rec FPM_INTR_MSK_POOL_DIS_FREE_MULTI_MSK_FIELD =
{
    "POOL_DIS_FREE_MULTI_MSK",
#if RU_INCLUDE_DESC
    "",
    "Free or Mcast update on disabled pool interrupt mask .\n\n",
#endif
    { FPM_INTR_MSK_POOL_DIS_FREE_MULTI_MSK_FIELD_MASK },
    0,
    { FPM_INTR_MSK_POOL_DIS_FREE_MULTI_MSK_FIELD_WIDTH },
    { FPM_INTR_MSK_POOL_DIS_FREE_MULTI_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MEMORY_CORRUPT_MSK *****/
const ru_field_rec FPM_INTR_MSK_MEMORY_CORRUPT_MSK_FIELD =
{
    "MEMORY_CORRUPT_MSK",
#if RU_INCLUDE_DESC
    "",
    "Index Memory corrupt interrupt mask.\n\n",
#endif
    { FPM_INTR_MSK_MEMORY_CORRUPT_MSK_FIELD_MASK },
    0,
    { FPM_INTR_MSK_MEMORY_CORRUPT_MSK_FIELD_WIDTH },
    { FPM_INTR_MSK_MEMORY_CORRUPT_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: XOFF_MSK *****/
const ru_field_rec FPM_INTR_MSK_XOFF_MSK_FIELD =
{
    "XOFF_MSK",
#if RU_INCLUDE_DESC
    "",
    "XOFF_STATE interrupt mask.\n\n",
#endif
    { FPM_INTR_MSK_XOFF_MSK_FIELD_MASK },
    0,
    { FPM_INTR_MSK_XOFF_MSK_FIELD_WIDTH },
    { FPM_INTR_MSK_XOFF_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: XON_MSK *****/
const ru_field_rec FPM_INTR_MSK_XON_MSK_FIELD =
{
    "XON_MSK",
#if RU_INCLUDE_DESC
    "",
    "XON_STATE interrupt mask.\n\n",
#endif
    { FPM_INTR_MSK_XON_MSK_FIELD_MASK },
    0,
    { FPM_INTR_MSK_XON_MSK_FIELD_WIDTH },
    { FPM_INTR_MSK_XON_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ILLEGAL_ADDRESS_ACCESS_MSK *****/
const ru_field_rec FPM_INTR_MSK_ILLEGAL_ADDRESS_ACCESS_MSK_FIELD =
{
    "ILLEGAL_ADDRESS_ACCESS_MSK",
#if RU_INCLUDE_DESC
    "",
    "Illegal/un-implemented register/memory space access  interrupt mask.\n\n",
#endif
    { FPM_INTR_MSK_ILLEGAL_ADDRESS_ACCESS_MSK_FIELD_MASK },
    0,
    { FPM_INTR_MSK_ILLEGAL_ADDRESS_ACCESS_MSK_FIELD_WIDTH },
    { FPM_INTR_MSK_ILLEGAL_ADDRESS_ACCESS_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ILLEGAL_ALLOC_REQUEST_MSK *****/
const ru_field_rec FPM_INTR_MSK_ILLEGAL_ALLOC_REQUEST_MSK_FIELD =
{
    "ILLEGAL_ALLOC_REQUEST_MSK",
#if RU_INCLUDE_DESC
    "",
    "Illegal token request interrupt mask.\n\n",
#endif
    { FPM_INTR_MSK_ILLEGAL_ALLOC_REQUEST_MSK_FIELD_MASK },
    0,
    { FPM_INTR_MSK_ILLEGAL_ALLOC_REQUEST_MSK_FIELD_WIDTH },
    { FPM_INTR_MSK_ILLEGAL_ALLOC_REQUEST_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EXPIRED_TOKEN_DET_MSK *****/
const ru_field_rec FPM_INTR_MSK_EXPIRED_TOKEN_DET_MSK_FIELD =
{
    "EXPIRED_TOKEN_DET_MSK",
#if RU_INCLUDE_DESC
    "",
    "Expired token detect interrupt mask.\n\n",
#endif
    { FPM_INTR_MSK_EXPIRED_TOKEN_DET_MSK_FIELD_MASK },
    0,
    { FPM_INTR_MSK_EXPIRED_TOKEN_DET_MSK_FIELD_WIDTH },
    { FPM_INTR_MSK_EXPIRED_TOKEN_DET_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EXPIRED_TOKEN_RECOV_MSK *****/
const ru_field_rec FPM_INTR_MSK_EXPIRED_TOKEN_RECOV_MSK_FIELD =
{
    "EXPIRED_TOKEN_RECOV_MSK",
#if RU_INCLUDE_DESC
    "",
    "Expired token recovered interrupt mask.\n\n",
#endif
    { FPM_INTR_MSK_EXPIRED_TOKEN_RECOV_MSK_FIELD_MASK },
    0,
    { FPM_INTR_MSK_EXPIRED_TOKEN_RECOV_MSK_FIELD_WIDTH },
    { FPM_INTR_MSK_EXPIRED_TOKEN_RECOV_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec FPM_INTR_MSK_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_INTR_MSK_R1_FIELD_MASK },
    0,
    { FPM_INTR_MSK_R1_FIELD_WIDTH },
    { FPM_INTR_MSK_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_INTR_MSK_FIELDS[] =
{
    &FPM_INTR_MSK_ALLOC_FIFO_FULL_MSK_FIELD,
    &FPM_INTR_MSK_FREE_FIFO_FULL_MSK_FIELD,
    &FPM_INTR_MSK_POOL_FULL_MSK_FIELD,
    &FPM_INTR_MSK_FREE_TOKEN_NO_VALID_MSK_FIELD,
    &FPM_INTR_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD,
    &FPM_INTR_MSK_MULTI_TOKEN_NO_VALID_MSK_FIELD,
    &FPM_INTR_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD,
    &FPM_INTR_MSK_POOL_DIS_FREE_MULTI_MSK_FIELD,
    &FPM_INTR_MSK_MEMORY_CORRUPT_MSK_FIELD,
    &FPM_INTR_MSK_XOFF_MSK_FIELD,
    &FPM_INTR_MSK_XON_MSK_FIELD,
    &FPM_INTR_MSK_ILLEGAL_ADDRESS_ACCESS_MSK_FIELD,
    &FPM_INTR_MSK_ILLEGAL_ALLOC_REQUEST_MSK_FIELD,
    &FPM_INTR_MSK_EXPIRED_TOKEN_DET_MSK_FIELD,
    &FPM_INTR_MSK_EXPIRED_TOKEN_RECOV_MSK_FIELD,
    &FPM_INTR_MSK_R1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_INTR_MSK *****/
const ru_reg_rec FPM_INTR_MSK_REG =
{
    "INTR_MSK",
#if RU_INCLUDE_DESC
    "INTR_MSK Register",
    "Mask bits are active high and are disabled by default. Software enables desired bits as necessary\n",
#endif
    { FPM_INTR_MSK_REG_OFFSET },
    0,
    0,
    1455,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    16,
    FPM_INTR_MSK_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_INTR_STS, TYPE: Type_FPM_BLOCK_FPM_POOL_0_INTR_STS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ALLOC_FIFO_FULL_STS *****/
const ru_field_rec FPM_INTR_STS_ALLOC_FIFO_FULL_STS_FIELD =
{
    "ALLOC_FIFO_FULL_STS",
#if RU_INCLUDE_DESC
    "",
    "Allocation FIFO Full Interrupt. This is a functional status bit, not an error status bit. This indicates that allocation FIFO is full with new tokens to be allocated and will be active (high) as long as FIFO is full. This status is intended to be used for debug purpose only.\n\n",
#endif
    { FPM_INTR_STS_ALLOC_FIFO_FULL_STS_FIELD_MASK },
    0,
    { FPM_INTR_STS_ALLOC_FIFO_FULL_STS_FIELD_WIDTH },
    { FPM_INTR_STS_ALLOC_FIFO_FULL_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_FIFO_FULL_STS *****/
const ru_field_rec FPM_INTR_STS_FREE_FIFO_FULL_STS_FIELD =
{
    "FREE_FIFO_FULL_STS",
#if RU_INCLUDE_DESC
    "",
    "De-Allocation FIFO Full Interrupt. This is a functional status bit, not an error status bit. This indicates that de-allocation FIFO is full with tokens needs to be freed and will be active (high) as long as FIFO is full. This status is intended to be used for debug purpose only.\n\n",
#endif
    { FPM_INTR_STS_FREE_FIFO_FULL_STS_FIELD_MASK },
    0,
    { FPM_INTR_STS_FREE_FIFO_FULL_STS_FIELD_WIDTH },
    { FPM_INTR_STS_FREE_FIFO_FULL_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_FULL_STS *****/
const ru_field_rec FPM_INTR_STS_POOL_FULL_STS_FIELD =
{
    "POOL_FULL_STS",
#if RU_INCLUDE_DESC
    "",
    "Usage Index Pool is fully allocated interrupt. This is a functional status bit, not an error status bit. This indicates that token pool is fully allocated and there are no free tokens available. This bit will be active (high) as long as there no free tokens available to allocate. This bit is intended to be used for debug purpose only.\n\n",
#endif
    { FPM_INTR_STS_POOL_FULL_STS_FIELD_MASK },
    0,
    { FPM_INTR_STS_POOL_FULL_STS_FIELD_WIDTH },
    { FPM_INTR_STS_POOL_FULL_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_TOKEN_NO_VALID_STS *****/
const ru_field_rec FPM_INTR_STS_FREE_TOKEN_NO_VALID_STS_FIELD =
{
    "FREE_TOKEN_NO_VALID_STS",
#if RU_INCLUDE_DESC
    "",
    "De-allocation token request with invalid token Interrupt.\nInvalid free token is determined when one or more the following conditions are met -\n1. Incoming multi-cast request token is not valid ( 0xffffffff )\n2. Incoming free request token entry in the usage array indicates it is not an allocated token, i.e., associated use count value for this count in the usage array is zero\nNote: item 2 is not checked if auto_pool_en is set. The auto_pool_en bit is always set when using the new token format without a pool select field.\n\n",
#endif
    { FPM_INTR_STS_FREE_TOKEN_NO_VALID_STS_FIELD_MASK },
    0,
    { FPM_INTR_STS_FREE_TOKEN_NO_VALID_STS_FIELD_WIDTH },
    { FPM_INTR_STS_FREE_TOKEN_NO_VALID_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_TOKEN_INDEX_OUT_OF_RANGE_STS *****/
const ru_field_rec FPM_INTR_STS_FREE_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD =
{
    "FREE_TOKEN_INDEX_OUT_OF_RANGE_STS",
#if RU_INCLUDE_DESC
    "",
    "Not applicable for BCM3391.  All token values are valid.\nDe-allocation token request with index out-of-range Interrupt.\nFree token index out of range is determined when one or more of the following conditions are met -\n1. Incoming free request token index is not aligned to the pool size indicated by the pool select field (bits[29:28])\n2. The buffer size indicated by the size field (bits[11:0]) is greater than the size of the allocated token.\nThere is no associated count for this error. Note: item 1 is not checked if auto_pool_en is set. The auto_pool_en bit is always set when using the new token format without a pool select field.\n\n",
#endif
    { FPM_INTR_STS_FREE_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD_MASK },
    0,
    { FPM_INTR_STS_FREE_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD_WIDTH },
    { FPM_INTR_STS_FREE_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MULTI_TOKEN_NO_VALID_STS *****/
const ru_field_rec FPM_INTR_STS_MULTI_TOKEN_NO_VALID_STS_FIELD =
{
    "MULTI_TOKEN_NO_VALID_STS",
#if RU_INCLUDE_DESC
    "",
    "Token multi-cast value update request with invalid token Interrupt.\nInvalid multi-cast token is determined when one or more the following conditions are met -\n1. Incoming multi-cast request token is not valid ( 0xffffffff )\n2. Incoming multi-cast request token has use count field set to zero\n3. Incoming multi-cast request token entry in the usage array indicates it is not an allocated token, i.e., associated use count value for this count in the usage array is zero\n4. After updating the use count value, the new use count value exceeds 0x7E\nNote: item 2 is not checked if auto_pool_en is set. The auto_pool_en bit is always set when using the new token format without a pool select field.\n\n",
#endif
    { FPM_INTR_STS_MULTI_TOKEN_NO_VALID_STS_FIELD_MASK },
    0,
    { FPM_INTR_STS_MULTI_TOKEN_NO_VALID_STS_FIELD_WIDTH },
    { FPM_INTR_STS_MULTI_TOKEN_NO_VALID_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS *****/
const ru_field_rec FPM_INTR_STS_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD =
{
    "MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS",
#if RU_INCLUDE_DESC
    "",
    "Token multi-cast value update request with index out-of-range Interrupt.\nThis is set when the token index is not aligned to the pool size. This is determined by examining the pool select field (bits[29:28]) and the 3 lsbs of the token index (bits[14:12]). There is no associated count for this error. Note: this error is not checked if auto_pool_en is set. The auto_pool_en bit is always set when using the new token format without a pool select field.\n\n",
#endif
    { FPM_INTR_STS_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD_MASK },
    0,
    { FPM_INTR_STS_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD_WIDTH },
    { FPM_INTR_STS_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_DIS_FREE_MULTI_STS *****/
const ru_field_rec FPM_INTR_STS_POOL_DIS_FREE_MULTI_STS_FIELD =
{
    "POOL_DIS_FREE_MULTI_STS",
#if RU_INCLUDE_DESC
    "",
    "Free or Mcast update on disabled pool interrupt.\nThis bit goes active when a free or multi-cast request is received and FPM is not enabled, i.e., pool enable bit in FPM control register is not set to 1'b1.\n\n",
#endif
    { FPM_INTR_STS_POOL_DIS_FREE_MULTI_STS_FIELD_MASK },
    0,
    { FPM_INTR_STS_POOL_DIS_FREE_MULTI_STS_FIELD_WIDTH },
    { FPM_INTR_STS_POOL_DIS_FREE_MULTI_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MEMORY_CORRUPT_STS *****/
const ru_field_rec FPM_INTR_STS_MEMORY_CORRUPT_STS_FIELD =
{
    "MEMORY_CORRUPT_STS",
#if RU_INCLUDE_DESC
    "",
    "Index Memory is corrupted.\nDuring updates of the usage array, token manager checks if the use count and search tree value in the array has a legal value. If the use count or search tree value is not correct before updating, logic generates an error and interrupt. As long as the interrupt is active no more valid tokens will be allocated because this is a catastrophic error. Following are the two error conditions that are checked -\n1.    During search for a free token, a particular token use count value indicates it is allocated (use count is greater than 0), but corresponding upper level search tree value indicates the token is still available (with bit value of 1'b0, instead of 1'b1). This is an error.\n2.    During search for a free token, a particular token use count value indicates that it is free (use count is 0), but corresponding upper level search tree value indicates the token is not available (with bit value of 1'b1, instead of 1'b0). This is an error.\n\n",
#endif
    { FPM_INTR_STS_MEMORY_CORRUPT_STS_FIELD_MASK },
    0,
    { FPM_INTR_STS_MEMORY_CORRUPT_STS_FIELD_WIDTH },
    { FPM_INTR_STS_MEMORY_CORRUPT_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: XOFF_STATE_STS *****/
const ru_field_rec FPM_INTR_STS_XOFF_STATE_STS_FIELD =
{
    "XOFF_STATE_STS",
#if RU_INCLUDE_DESC
    "",
    "Number of available tokens is less than or equal to XOFF_THRESHOLD value in XON/XOFF Threshold configuration register. This is a functional status bit, not an error status bit. Using this information FPM generates backpressure output signal that is used by other UBUS client logics to throttle its operation. For example, UNIMAC logic can use backpressure signal to transfer PAUSE Ethernet flow control packets to throttle incoming frames on Ethernet interface.\n\n",
#endif
    { FPM_INTR_STS_XOFF_STATE_STS_FIELD_MASK },
    0,
    { FPM_INTR_STS_XOFF_STATE_STS_FIELD_WIDTH },
    { FPM_INTR_STS_XOFF_STATE_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: XON_STATE_STS *****/
const ru_field_rec FPM_INTR_STS_XON_STATE_STS_FIELD =
{
    "XON_STATE_STS",
#if RU_INCLUDE_DESC
    "",
    "Number of available tokens is greater than or equal to XON_THRESHOLD value in XON/XOFF Threshold configuration register. This is a functional status bit, not an error status bit. Using this information FPM generates backpressure output signal that is used by other UBUS client logics to throttle its operation. For example, UNIMAC logic can use backpressure signal to transfer PAUSE Ethernet flow control packets to throttle incoming frames on Ethernet interface.\n\n",
#endif
    { FPM_INTR_STS_XON_STATE_STS_FIELD_MASK },
    0,
    { FPM_INTR_STS_XON_STATE_STS_FIELD_WIDTH },
    { FPM_INTR_STS_XON_STATE_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ILLEGAL_ADDRESS_ACCESS_STS *****/
const ru_field_rec FPM_INTR_STS_ILLEGAL_ADDRESS_ACCESS_STS_FIELD =
{
    "ILLEGAL_ADDRESS_ACCESS_STS",
#if RU_INCLUDE_DESC
    "",
    "Illegal/un-implemented register/memory space access interrupt. This will be active when there is an attempt to read from an unimplemented register or memory space. Along with interrupt being sent an error reply packet will be sent out with o_ubus_error_out asserted.\n\n",
#endif
    { FPM_INTR_STS_ILLEGAL_ADDRESS_ACCESS_STS_FIELD_MASK },
    0,
    { FPM_INTR_STS_ILLEGAL_ADDRESS_ACCESS_STS_FIELD_WIDTH },
    { FPM_INTR_STS_ILLEGAL_ADDRESS_ACCESS_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ILLEGAL_ALLOC_REQUEST_STS *****/
const ru_field_rec FPM_INTR_STS_ILLEGAL_ALLOC_REQUEST_STS_FIELD =
{
    "ILLEGAL_ALLOC_REQUEST_STS",
#if RU_INCLUDE_DESC
    "",
    "Illegal token request interrupt. This will be active when the pool is disabled, there is a request for a new token and the alloc fifo for the selected token size is empty. Along with interrupt being sent an error reply packet will be sent out with o_ubus_error_out asserted.\n\n",
#endif
    { FPM_INTR_STS_ILLEGAL_ALLOC_REQUEST_STS_FIELD_MASK },
    0,
    { FPM_INTR_STS_ILLEGAL_ALLOC_REQUEST_STS_FIELD_WIDTH },
    { FPM_INTR_STS_ILLEGAL_ALLOC_REQUEST_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EXPIRED_TOKEN_DET_STS *****/
const ru_field_rec FPM_INTR_STS_EXPIRED_TOKEN_DET_STS_FIELD =
{
    "EXPIRED_TOKEN_DET_STS",
#if RU_INCLUDE_DESC
    "",
    "Expired token detect interrupt. This is set when the token recovery logic detects a token that has been held for the entire duration of the aging timer.\n\n",
#endif
    { FPM_INTR_STS_EXPIRED_TOKEN_DET_STS_FIELD_MASK },
    0,
    { FPM_INTR_STS_EXPIRED_TOKEN_DET_STS_FIELD_WIDTH },
    { FPM_INTR_STS_EXPIRED_TOKEN_DET_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EXPIRED_TOKEN_RECOV_STS *****/
const ru_field_rec FPM_INTR_STS_EXPIRED_TOKEN_RECOV_STS_FIELD =
{
    "EXPIRED_TOKEN_RECOV_STS",
#if RU_INCLUDE_DESC
    "",
    "Expired token recovered interrupt. This is set when an expired token has been recoveredand returned to pool as an available token.\n\n",
#endif
    { FPM_INTR_STS_EXPIRED_TOKEN_RECOV_STS_FIELD_MASK },
    0,
    { FPM_INTR_STS_EXPIRED_TOKEN_RECOV_STS_FIELD_WIDTH },
    { FPM_INTR_STS_EXPIRED_TOKEN_RECOV_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec FPM_INTR_STS_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_INTR_STS_R1_FIELD_MASK },
    0,
    { FPM_INTR_STS_R1_FIELD_WIDTH },
    { FPM_INTR_STS_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_INTR_STS_FIELDS[] =
{
    &FPM_INTR_STS_ALLOC_FIFO_FULL_STS_FIELD,
    &FPM_INTR_STS_FREE_FIFO_FULL_STS_FIELD,
    &FPM_INTR_STS_POOL_FULL_STS_FIELD,
    &FPM_INTR_STS_FREE_TOKEN_NO_VALID_STS_FIELD,
    &FPM_INTR_STS_FREE_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD,
    &FPM_INTR_STS_MULTI_TOKEN_NO_VALID_STS_FIELD,
    &FPM_INTR_STS_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD,
    &FPM_INTR_STS_POOL_DIS_FREE_MULTI_STS_FIELD,
    &FPM_INTR_STS_MEMORY_CORRUPT_STS_FIELD,
    &FPM_INTR_STS_XOFF_STATE_STS_FIELD,
    &FPM_INTR_STS_XON_STATE_STS_FIELD,
    &FPM_INTR_STS_ILLEGAL_ADDRESS_ACCESS_STS_FIELD,
    &FPM_INTR_STS_ILLEGAL_ALLOC_REQUEST_STS_FIELD,
    &FPM_INTR_STS_EXPIRED_TOKEN_DET_STS_FIELD,
    &FPM_INTR_STS_EXPIRED_TOKEN_RECOV_STS_FIELD,
    &FPM_INTR_STS_R1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_INTR_STS *****/
const ru_reg_rec FPM_INTR_STS_REG =
{
    "INTR_STS",
#if RU_INCLUDE_DESC
    "INTR_STS Register",
    "Interrupt bits are active high. When a bit in this register is set to 1 and the corresponding bit in interrupt mask register is set to 1, interrupt to CPU will occur. When set (1), interrupts bits can be cleared (0) by writing a 1 to the desired bit.\n",
#endif
    { FPM_INTR_STS_REG_OFFSET },
    0,
    0,
    1456,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    16,
    FPM_INTR_STS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_STALL_MSK, TYPE: Type_FPM_BLOCK_FPM_POOL_0_STALL_MSK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: R3 *****/
const ru_field_rec FPM_STALL_MSK_R3_FIELD =
{
    "R3",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_STALL_MSK_R3_FIELD_MASK },
    0,
    { FPM_STALL_MSK_R3_FIELD_WIDTH },
    { FPM_STALL_MSK_R3_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_TOKEN_NO_VALID_STALL_MSK *****/
const ru_field_rec FPM_STALL_MSK_FREE_TOKEN_NO_VALID_STALL_MSK_FIELD =
{
    "FREE_TOKEN_NO_VALID_STALL_MSK",
#if RU_INCLUDE_DESC
    "",
    "Stall FPM on De-allocation token request with invalid token interrupt status.\n\n",
#endif
    { FPM_STALL_MSK_FREE_TOKEN_NO_VALID_STALL_MSK_FIELD_MASK },
    0,
    { FPM_STALL_MSK_FREE_TOKEN_NO_VALID_STALL_MSK_FIELD_WIDTH },
    { FPM_STALL_MSK_FREE_TOKEN_NO_VALID_STALL_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK *****/
const ru_field_rec FPM_STALL_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD =
{
    "FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK",
#if RU_INCLUDE_DESC
    "",
    "Stall FPM on De-allocation token request with index out-of-range interrupt status.\n\n",
#endif
    { FPM_STALL_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD_MASK },
    0,
    { FPM_STALL_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD_WIDTH },
    { FPM_STALL_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MULTI_TOKEN_NO_VALID_STALL_MSK *****/
const ru_field_rec FPM_STALL_MSK_MULTI_TOKEN_NO_VALID_STALL_MSK_FIELD =
{
    "MULTI_TOKEN_NO_VALID_STALL_MSK",
#if RU_INCLUDE_DESC
    "",
    "Stall FPM on Token multi-cast value update request with invalid token interrupt status.\n\n",
#endif
    { FPM_STALL_MSK_MULTI_TOKEN_NO_VALID_STALL_MSK_FIELD_MASK },
    0,
    { FPM_STALL_MSK_MULTI_TOKEN_NO_VALID_STALL_MSK_FIELD_WIDTH },
    { FPM_STALL_MSK_MULTI_TOKEN_NO_VALID_STALL_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK *****/
const ru_field_rec FPM_STALL_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD =
{
    "MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK",
#if RU_INCLUDE_DESC
    "",
    "Stall FPM on Token multi-cast value update request with index out-of-range interrupt status.\n\n",
#endif
    { FPM_STALL_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD_MASK },
    0,
    { FPM_STALL_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD_WIDTH },
    { FPM_STALL_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R2 *****/
const ru_field_rec FPM_STALL_MSK_R2_FIELD =
{
    "R2",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_STALL_MSK_R2_FIELD_MASK },
    0,
    { FPM_STALL_MSK_R2_FIELD_WIDTH },
    { FPM_STALL_MSK_R2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MEMORY_CORRUPT_STALL_MSK *****/
const ru_field_rec FPM_STALL_MSK_MEMORY_CORRUPT_STALL_MSK_FIELD =
{
    "MEMORY_CORRUPT_STALL_MSK",
#if RU_INCLUDE_DESC
    "",
    "Stall FPM on Index Memory corrupt interrupt status.\n\n",
#endif
    { FPM_STALL_MSK_MEMORY_CORRUPT_STALL_MSK_FIELD_MASK },
    0,
    { FPM_STALL_MSK_MEMORY_CORRUPT_STALL_MSK_FIELD_WIDTH },
    { FPM_STALL_MSK_MEMORY_CORRUPT_STALL_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec FPM_STALL_MSK_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_STALL_MSK_R1_FIELD_MASK },
    0,
    { FPM_STALL_MSK_R1_FIELD_WIDTH },
    { FPM_STALL_MSK_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_STALL_MSK_FIELDS[] =
{
    &FPM_STALL_MSK_R3_FIELD,
    &FPM_STALL_MSK_FREE_TOKEN_NO_VALID_STALL_MSK_FIELD,
    &FPM_STALL_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD,
    &FPM_STALL_MSK_MULTI_TOKEN_NO_VALID_STALL_MSK_FIELD,
    &FPM_STALL_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD,
    &FPM_STALL_MSK_R2_FIELD,
    &FPM_STALL_MSK_MEMORY_CORRUPT_STALL_MSK_FIELD,
    &FPM_STALL_MSK_R1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_STALL_MSK *****/
const ru_reg_rec FPM_STALL_MSK_REG =
{
    "STALL_MSK",
#if RU_INCLUDE_DESC
    "STALL_MSK Register",
    "Software sets desired stall bits that upon corresponding active interrupt status will stall FPM from new allocation, de-allocation, and mcast update process. Listed below are the supported interrupt statuses\n1. Invalid free token (bit[3] of interrupt status register 0x14)\n2. Invalid free token with index out-of-range (bit[4] of interrupt status register 0x14)\n3. Invalid mcast token (bit[5] of interrupt status register 0x14)\n4. Invalid mcast token with index out-of-range (bit[6] of interrupt status register 0x14)\n5. Memory corrupt status (bit[8] of interrupt status register 0x14)\nWhen state machine is stalled, registers and memory can still be accessed. Any new token allocation request will be serviced with valid tokens (if available in alloc cache) and invalid tokens (if alloc cache is empty). Any new de-allocation/mcast update requests will be either stored in de-allocation fifo (if there is space in free fifo) or dropped (if free fifo is full). Bit locations in this register matches the location of corrseponding interrupt status bits in register 0x14. To un-stall (enable) state machine interrupt status bits (in register 0x14) corresponding to these mask bits should be cleared. Stall mask bits are active high and are disabled by default. This is for debug purposes only.\n",
#endif
    { FPM_STALL_MSK_REG_OFFSET },
    0,
    0,
    1457,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    FPM_STALL_MSK_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_CFG3, TYPE: Type_FPM_BLOCK_FPM_POOL_0_CFG3
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: HALT_ALLOC_TIMER *****/
const ru_field_rec FPM_CFG3_HALT_ALLOC_TIMER_FIELD =
{
    "HALT_ALLOC_TIMER",
#if RU_INCLUDE_DESC
    "",
    "Timeout value for the UBUS interface\nReset value is 0x200.\n",
#endif
    { FPM_CFG3_HALT_ALLOC_TIMER_FIELD_MASK },
    0,
    { FPM_CFG3_HALT_ALLOC_TIMER_FIELD_WIDTH },
    { FPM_CFG3_HALT_ALLOC_TIMER_FIELD_SHIFT },
    512,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_CFG3_FIELDS[] =
{
    &FPM_CFG3_HALT_ALLOC_TIMER_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_CFG3 *****/
const ru_reg_rec FPM_CFG3_REG =
{
    "CFG3",
#if RU_INCLUDE_DESC
    "CFG3 Register",
    "Config ubus timer value.\n",
#endif
    { FPM_CFG3_REG_OFFSET },
    0,
    0,
    1458,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_CFG3_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_STAT1, TYPE: Type_FPM_BLOCK_FPM_POOL_0_STAT1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: UNDRFL *****/
const ru_field_rec FPM_STAT1_UNDRFL_FIELD =
{
    "UNDRFL",
#if RU_INCLUDE_DESC
    "",
    "Free Pool underflow count\n",
#endif
    { FPM_STAT1_UNDRFL_FIELD_MASK },
    0,
    { FPM_STAT1_UNDRFL_FIELD_WIDTH },
    { FPM_STAT1_UNDRFL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: OVRFL *****/
const ru_field_rec FPM_STAT1_OVRFL_FIELD =
{
    "OVRFL",
#if RU_INCLUDE_DESC
    "",
    "Free Pool overflow count\n",
#endif
    { FPM_STAT1_OVRFL_FIELD_MASK },
    0,
    { FPM_STAT1_OVRFL_FIELD_WIDTH },
    { FPM_STAT1_OVRFL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_STAT1_FIELDS[] =
{
    &FPM_STAT1_UNDRFL_FIELD,
    &FPM_STAT1_OVRFL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_STAT1 *****/
const ru_reg_rec FPM_STAT1_REG =
{
    "STAT1",
#if RU_INCLUDE_DESC
    "STAT1 Register",
    "This read only register allows software to read the count of free pool overflows and underflows. A overflow condition occurs when pool is empty, ie., no tokens are allocated and free/mcast request is encountered. A underflow condition occurs when pool is full, ie., there are no free tokens and a allocation request is encountered. When the counter values reaches maximum count, it will hold the max value and not increment the count value unless it is cleared. Any write to this register will clear both both counters.\n",
#endif
    { FPM_STAT1_REG_OFFSET },
    0,
    0,
    1459,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_STAT1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_STAT2, TYPE: Type_FPM_BLOCK_FPM_POOL_0_STAT2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: NUM_OF_TOKENS_AVAILABLE *****/
const ru_field_rec FPM_STAT2_NUM_OF_TOKENS_AVAILABLE_FIELD =
{
    "NUM_OF_TOKENS_AVAILABLE",
#if RU_INCLUDE_DESC
    "",
    "Count of tokens available for allocation.\nThis provides a count of number of free tokens that available for allocation in the usage array. This value is updated instantaneously as tokens are allocated or freed from the array.\n\n",
#endif
    { FPM_STAT2_NUM_OF_TOKENS_AVAILABLE_FIELD_MASK },
    0,
    { FPM_STAT2_NUM_OF_TOKENS_AVAILABLE_FIELD_WIDTH },
    { FPM_STAT2_NUM_OF_TOKENS_AVAILABLE_FIELD_SHIFT },
    524288,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R2 *****/
const ru_field_rec FPM_STAT2_R2_FIELD =
{
    "R2",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_STAT2_R2_FIELD_MASK },
    0,
    { FPM_STAT2_R2_FIELD_WIDTH },
    { FPM_STAT2_R2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ALLOC_FIFO_EMPTY *****/
const ru_field_rec FPM_STAT2_ALLOC_FIFO_EMPTY_FIELD =
{
    "ALLOC_FIFO_EMPTY",
#if RU_INCLUDE_DESC
    "",
    "ALLOC_FIFO is empty.\n\n",
#endif
    { FPM_STAT2_ALLOC_FIFO_EMPTY_FIELD_MASK },
    0,
    { FPM_STAT2_ALLOC_FIFO_EMPTY_FIELD_WIDTH },
    { FPM_STAT2_ALLOC_FIFO_EMPTY_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ALLOC_FIFO_FULL *****/
const ru_field_rec FPM_STAT2_ALLOC_FIFO_FULL_FIELD =
{
    "ALLOC_FIFO_FULL",
#if RU_INCLUDE_DESC
    "",
    "ALLOC_FIFO is full\n\n",
#endif
    { FPM_STAT2_ALLOC_FIFO_FULL_FIELD_MASK },
    0,
    { FPM_STAT2_ALLOC_FIFO_FULL_FIELD_WIDTH },
    { FPM_STAT2_ALLOC_FIFO_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_FIFO_EMPTY *****/
const ru_field_rec FPM_STAT2_FREE_FIFO_EMPTY_FIELD =
{
    "FREE_FIFO_EMPTY",
#if RU_INCLUDE_DESC
    "",
    "FREE_FIFO is empty\n\n",
#endif
    { FPM_STAT2_FREE_FIFO_EMPTY_FIELD_MASK },
    0,
    { FPM_STAT2_FREE_FIFO_EMPTY_FIELD_WIDTH },
    { FPM_STAT2_FREE_FIFO_EMPTY_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_FIFO_FULL *****/
const ru_field_rec FPM_STAT2_FREE_FIFO_FULL_FIELD =
{
    "FREE_FIFO_FULL",
#if RU_INCLUDE_DESC
    "",
    "FREE_FIFO is full.\n\n",
#endif
    { FPM_STAT2_FREE_FIFO_FULL_FIELD_MASK },
    0,
    { FPM_STAT2_FREE_FIFO_FULL_FIELD_WIDTH },
    { FPM_STAT2_FREE_FIFO_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec FPM_STAT2_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_STAT2_R1_FIELD_MASK },
    0,
    { FPM_STAT2_R1_FIELD_WIDTH },
    { FPM_STAT2_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_FULL *****/
const ru_field_rec FPM_STAT2_POOL_FULL_FIELD =
{
    "POOL_FULL",
#if RU_INCLUDE_DESC
    "",
    "POOL is full\nThis indicates that all tokens have been allocated and there no free tokens available. This bit will be active as long as all usage array is fully allocated.\n\n",
#endif
    { FPM_STAT2_POOL_FULL_FIELD_MASK },
    0,
    { FPM_STAT2_POOL_FULL_FIELD_WIDTH },
    { FPM_STAT2_POOL_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_STAT2_FIELDS[] =
{
    &FPM_STAT2_NUM_OF_TOKENS_AVAILABLE_FIELD,
    &FPM_STAT2_R2_FIELD,
    &FPM_STAT2_ALLOC_FIFO_EMPTY_FIELD,
    &FPM_STAT2_ALLOC_FIFO_FULL_FIELD,
    &FPM_STAT2_FREE_FIFO_EMPTY_FIELD,
    &FPM_STAT2_FREE_FIFO_FULL_FIELD,
    &FPM_STAT2_R1_FIELD,
    &FPM_STAT2_POOL_FULL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_STAT2 *****/
const ru_reg_rec FPM_STAT2_REG =
{
    "STAT2",
#if RU_INCLUDE_DESC
    "STAT2 Register",
    "This read only register provide status of index memory, alloc & free cache/fifos. These are real time statuses and bits are not sticky. Write to any bits will have no effect.\n",
#endif
    { FPM_STAT2_REG_OFFSET },
    0,
    0,
    1460,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    FPM_STAT2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_STAT3, TYPE: Type_FPM_BLOCK_FPM_POOL_0_STAT3
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: NUM_OF_NOT_VALID_TOKEN_FREES *****/
const ru_field_rec FPM_STAT3_NUM_OF_NOT_VALID_TOKEN_FREES_FIELD =
{
    "NUM_OF_NOT_VALID_TOKEN_FREES",
#if RU_INCLUDE_DESC
    "",
    "Count of de-allocate token requests with invalid tokens. For more information on conditions under which this counter is incremented, refer to POOL1_INTR_STS register (offset 0x14) bit[3] explanation in this document.\n\n",
#endif
    { FPM_STAT3_NUM_OF_NOT_VALID_TOKEN_FREES_FIELD_MASK },
    0,
    { FPM_STAT3_NUM_OF_NOT_VALID_TOKEN_FREES_FIELD_WIDTH },
    { FPM_STAT3_NUM_OF_NOT_VALID_TOKEN_FREES_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec FPM_STAT3_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_STAT3_R1_FIELD_MASK },
    0,
    { FPM_STAT3_R1_FIELD_WIDTH },
    { FPM_STAT3_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_STAT3_FIELDS[] =
{
    &FPM_STAT3_NUM_OF_NOT_VALID_TOKEN_FREES_FIELD,
    &FPM_STAT3_R1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_STAT3 *****/
const ru_reg_rec FPM_STAT3_REG =
{
    "STAT3",
#if RU_INCLUDE_DESC
    "STAT3 Register",
    "This read only register allows software to read the count of free token requests with in-valid tokens When the counter values reaches maximum count, it will hold the max value and not increment the count value unless it is cleared. Any write to this register will clear count value.\n",
#endif
    { FPM_STAT3_REG_OFFSET },
    0,
    0,
    1461,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_STAT3_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_STAT4, TYPE: Type_FPM_BLOCK_FPM_POOL_0_STAT4
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: NUM_OF_NOT_VALID_TOKEN_MULTI *****/
const ru_field_rec FPM_STAT4_NUM_OF_NOT_VALID_TOKEN_MULTI_FIELD =
{
    "NUM_OF_NOT_VALID_TOKEN_MULTI",
#if RU_INCLUDE_DESC
    "",
    "Count of multi-cast token update requests with either valid bit not set, For more information on conditions under which this counter is incremented, refer to POOL1_INTR_STS register (offset 0x14) bit[5] explanation in this document.\n\n",
#endif
    { FPM_STAT4_NUM_OF_NOT_VALID_TOKEN_MULTI_FIELD_MASK },
    0,
    { FPM_STAT4_NUM_OF_NOT_VALID_TOKEN_MULTI_FIELD_WIDTH },
    { FPM_STAT4_NUM_OF_NOT_VALID_TOKEN_MULTI_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec FPM_STAT4_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_STAT4_R1_FIELD_MASK },
    0,
    { FPM_STAT4_R1_FIELD_WIDTH },
    { FPM_STAT4_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_STAT4_FIELDS[] =
{
    &FPM_STAT4_NUM_OF_NOT_VALID_TOKEN_MULTI_FIELD,
    &FPM_STAT4_R1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_STAT4 *****/
const ru_reg_rec FPM_STAT4_REG =
{
    "STAT4",
#if RU_INCLUDE_DESC
    "STAT4 Register",
    "This read only register allows software to read the count of multi-cast token update requests with in-valid tokens. When the counter values reaches maximum count, it will hold the max value and not increment the count value unless it is cleared. Any write to this register will clear count value.\n",
#endif
    { FPM_STAT4_REG_OFFSET },
    0,
    0,
    1462,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_STAT4_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_STAT5, TYPE: Type_FPM_BLOCK_FPM_POOL_0_STAT5
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN *****/
const ru_field_rec FPM_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_FIELD =
{
    "MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN",
#if RU_INCLUDE_DESC
    "",
    "Token that causes memory corrupt interrupt active. If there are multiple tokens that causes this error, only the first one is captured. To capture successive tokens that causes the error this register should be cleared by writing any random value, in addition, memory corrupt status bit (bit[8]) in interrupt status register 0x14 should be cleared. Bitmap for these bits is shown below (reserved bits are zeros)\nBit[30]    - Reserved\nBit[29:12] - Token\nBit[11:0]  - Buffer size in bytes\n\n",
#endif
    { FPM_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_FIELD_MASK },
    0,
    { FPM_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_FIELD_WIDTH },
    { FPM_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_STAT5_FIELDS[] =
{
    &FPM_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_STAT5 *****/
const ru_reg_rec FPM_STAT5_REG =
{
    "STAT5",
#if RU_INCLUDE_DESC
    "STAT5 Register",
    "This read only register allows software to read the alloc token that causes memory corrupt interrupt (intr[8]) to go active. This is for debug purposes only. Any write to this register will clear token value (makes all bits one).\n",
#endif
    { FPM_STAT5_REG_OFFSET },
    0,
    0,
    1463,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_STAT5_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_STAT6, TYPE: Type_FPM_BLOCK_FPM_POOL_0_STAT6
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: INVALID_FREE_TOKEN *****/
const ru_field_rec FPM_STAT6_INVALID_FREE_TOKEN_FIELD =
{
    "INVALID_FREE_TOKEN",
#if RU_INCLUDE_DESC
    "",
    "Token that causes intr[3] or intr[4] active. If there are multiple tokens that causes this error, only the first one is captured. To capture successive tokens that causes the error this register should be cleared by writing any random value. Bitmap for these bits is shown below (reserved bits are either zeros or can reflect the length of the packet associated with the freed token)\nBit[30]    - Reserved\nBit[29:12] - Token\nBit[11:0]  - Reserved\n\n",
#endif
    { FPM_STAT6_INVALID_FREE_TOKEN_FIELD_MASK },
    0,
    { FPM_STAT6_INVALID_FREE_TOKEN_FIELD_WIDTH },
    { FPM_STAT6_INVALID_FREE_TOKEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_STAT6_FIELDS[] =
{
    &FPM_STAT6_INVALID_FREE_TOKEN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_STAT6 *****/
const ru_reg_rec FPM_STAT6_REG =
{
    "STAT6",
#if RU_INCLUDE_DESC
    "STAT6 Register",
    "This read only register allows software to read the free token that causes invalid free request or free token with index out-of-range interrupts (intr[3] or intr[4]) to go active. This is for debug purposes only. Any write to this register will clear token value (makes all bits zero).\n",
#endif
    { FPM_STAT6_REG_OFFSET },
    0,
    0,
    1464,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_STAT6_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_STAT7, TYPE: Type_FPM_BLOCK_FPM_POOL_0_STAT7
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: INVALID_MCAST_TOKEN *****/
const ru_field_rec FPM_STAT7_INVALID_MCAST_TOKEN_FIELD =
{
    "INVALID_MCAST_TOKEN",
#if RU_INCLUDE_DESC
    "",
    "Token that causes intr[5] or intr[6] active. If there are multiple tokens that causes this error, only the first one is captured. To capture successive tokens that causes the error this register should be cleared by writing any random value. Bitmap for these bits is shown below (reserved bits are zeros)\nBit[30]    - Reserved\nBit[29:12] - Token\nBit[11]    - Mcast update type (refer to register 0x224[11])\nBit[10:7]  - Reserved\nBit[6:0]   - Mcast value\n\n",
#endif
    { FPM_STAT7_INVALID_MCAST_TOKEN_FIELD_MASK },
    0,
    { FPM_STAT7_INVALID_MCAST_TOKEN_FIELD_WIDTH },
    { FPM_STAT7_INVALID_MCAST_TOKEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_STAT7_FIELDS[] =
{
    &FPM_STAT7_INVALID_MCAST_TOKEN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_STAT7 *****/
const ru_reg_rec FPM_STAT7_REG =
{
    "STAT7",
#if RU_INCLUDE_DESC
    "STAT7 Register",
    "This read only register allows software to read the multi-cast token that causes invalid mcast request or mcast token with index out-of-range interrupts (intr[5] or intr[6]) to go active. This is for debug purposes only. Any write to this register will clear token value (makes all bits one).\n",
#endif
    { FPM_STAT7_REG_OFFSET },
    0,
    0,
    1465,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_STAT7_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_STAT8, TYPE: Type_FPM_BLOCK_FPM_POOL_0_STAT8
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKENS_AVAILABLE_LOW_WTMK *****/
const ru_field_rec FPM_STAT8_TOKENS_AVAILABLE_LOW_WTMK_FIELD =
{
    "TOKENS_AVAILABLE_LOW_WTMK",
#if RU_INCLUDE_DESC
    "",
    "Lowest value the NUM_OF_TOKENS_AVAIL count has reached.\n",
#endif
    { FPM_STAT8_TOKENS_AVAILABLE_LOW_WTMK_FIELD_MASK },
    0,
    { FPM_STAT8_TOKENS_AVAILABLE_LOW_WTMK_FIELD_WIDTH },
    { FPM_STAT8_TOKENS_AVAILABLE_LOW_WTMK_FIELD_SHIFT },
    65536,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec FPM_STAT8_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_STAT8_R1_FIELD_MASK },
    0,
    { FPM_STAT8_R1_FIELD_WIDTH },
    { FPM_STAT8_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_STAT8_FIELDS[] =
{
    &FPM_STAT8_TOKENS_AVAILABLE_LOW_WTMK_FIELD,
    &FPM_STAT8_R1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_STAT8 *****/
const ru_reg_rec FPM_STAT8_REG =
{
    "STAT8",
#if RU_INCLUDE_DESC
    "STAT8 Register",
    "This register allows software to read the lowest value the NUM_OF_TOKENS_AVAILABLE count reached since the last time it was cleared. Any write to this register will reset the value back to the maximum number of tokens (0x100000)\n",
#endif
    { FPM_STAT8_REG_OFFSET },
    0,
    0,
    1466,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_STAT8_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_XON_XOFF_CFG, TYPE: Type_FPM_BLOCK_FPM_POOL_0_XON_XOFF_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: XOFF_THRESHOLD *****/
const ru_field_rec FPM_XON_XOFF_CFG_XOFF_THRESHOLD_FIELD =
{
    "XOFF_THRESHOLD",
#if RU_INCLUDE_DESC
    "",
    "XOFF Threshold value\nIndicates the lower threshold of available tokens at which XOFF condition is set.\nEach threshold value represents 256 buffers\n",
#endif
    { FPM_XON_XOFF_CFG_XOFF_THRESHOLD_FIELD_MASK },
    0,
    { FPM_XON_XOFF_CFG_XOFF_THRESHOLD_FIELD_WIDTH },
    { FPM_XON_XOFF_CFG_XOFF_THRESHOLD_FIELD_SHIFT },
    48,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: XON_THRESHOLD *****/
const ru_field_rec FPM_XON_XOFF_CFG_XON_THRESHOLD_FIELD =
{
    "XON_THRESHOLD",
#if RU_INCLUDE_DESC
    "",
    "XON Threshold value\nIndicates the upper threshold of available tokens at which XON condition is set.\nEach threshold value represents 256 buffers\n",
#endif
    { FPM_XON_XOFF_CFG_XON_THRESHOLD_FIELD_MASK },
    0,
    { FPM_XON_XOFF_CFG_XON_THRESHOLD_FIELD_WIDTH },
    { FPM_XON_XOFF_CFG_XON_THRESHOLD_FIELD_SHIFT },
    64,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_XON_XOFF_CFG_FIELDS[] =
{
    &FPM_XON_XOFF_CFG_XOFF_THRESHOLD_FIELD,
    &FPM_XON_XOFF_CFG_XON_THRESHOLD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_XON_XOFF_CFG *****/
const ru_reg_rec FPM_XON_XOFF_CFG_REG =
{
    "XON_XOFF_CFG",
#if RU_INCLUDE_DESC
    "XON_XOFF_CFG Register",
    "No Description\n",
#endif
    { FPM_XON_XOFF_CFG_REG_OFFSET },
    0,
    0,
    1467,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_XON_XOFF_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_NOT_EMPTY_CFG, TYPE: Type_FPM_BLOCK_FPM_POOL_0_NOT_EMPTY_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: NOT_EMPTY_THRESHOLD *****/
const ru_field_rec FPM_NOT_EMPTY_CFG_NOT_EMPTY_THRESHOLD_FIELD =
{
    "NOT_EMPTY_THRESHOLD",
#if RU_INCLUDE_DESC
    "",
    "Threshold value for reasserting pool_not_empty to FPM_BB\n\n",
#endif
    { FPM_NOT_EMPTY_CFG_NOT_EMPTY_THRESHOLD_FIELD_MASK },
    0,
    { FPM_NOT_EMPTY_CFG_NOT_EMPTY_THRESHOLD_FIELD_WIDTH },
    { FPM_NOT_EMPTY_CFG_NOT_EMPTY_THRESHOLD_FIELD_SHIFT },
    8,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec FPM_NOT_EMPTY_CFG_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_NOT_EMPTY_CFG_R1_FIELD_MASK },
    0,
    { FPM_NOT_EMPTY_CFG_R1_FIELD_WIDTH },
    { FPM_NOT_EMPTY_CFG_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_NOT_EMPTY_CFG_FIELDS[] =
{
    &FPM_NOT_EMPTY_CFG_NOT_EMPTY_THRESHOLD_FIELD,
    &FPM_NOT_EMPTY_CFG_R1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_NOT_EMPTY_CFG *****/
const ru_reg_rec FPM_NOT_EMPTY_CFG_REG =
{
    "NOT_EMPTY_CFG",
#if RU_INCLUDE_DESC
    "NOT_EMPTY_CFG Register",
    "No Description\n",
#endif
    { FPM_NOT_EMPTY_CFG_REG_OFFSET },
    0,
    0,
    1468,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_NOT_EMPTY_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_TOKEN_RECOVER_CTL, TYPE: Type_FPM_BLOCK_FPM_POOL_0_TOKEN_RECOVER_CTL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN_RECOVER_ENA *****/
const ru_field_rec FPM_TOKEN_RECOVER_CTL_TOKEN_RECOVER_ENA_FIELD =
{
    "TOKEN_RECOVER_ENA",
#if RU_INCLUDE_DESC
    "",
    "Token recovery enable\n1 = Enabled\n0 = Disabled\n\n",
#endif
    { FPM_TOKEN_RECOVER_CTL_TOKEN_RECOVER_ENA_FIELD_MASK },
    0,
    { FPM_TOKEN_RECOVER_CTL_TOKEN_RECOVER_ENA_FIELD_WIDTH },
    { FPM_TOKEN_RECOVER_CTL_TOKEN_RECOVER_ENA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SINGLE_PASS_ENA *****/
const ru_field_rec FPM_TOKEN_RECOVER_CTL_SINGLE_PASS_ENA_FIELD =
{
    "SINGLE_PASS_ENA",
#if RU_INCLUDE_DESC
    "",
    "If token recovery is enabled, the single-pass control will indicate whether the hardware should perform just one iteration of the token recovery process or will continuously loop through the token recovery process.\n1 = Single pass\n0 = Auto repeat\n\n",
#endif
    { FPM_TOKEN_RECOVER_CTL_SINGLE_PASS_ENA_FIELD_MASK },
    0,
    { FPM_TOKEN_RECOVER_CTL_SINGLE_PASS_ENA_FIELD_WIDTH },
    { FPM_TOKEN_RECOVER_CTL_SINGLE_PASS_ENA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN_REMARK_ENA *****/
const ru_field_rec FPM_TOKEN_RECOVER_CTL_TOKEN_REMARK_ENA_FIELD =
{
    "TOKEN_REMARK_ENA",
#if RU_INCLUDE_DESC
    "",
    "Enable remarking of tokens for multiple passes through the token recovery process. The mark bit is set on all tokens on the first pass through the loop. When this bit is set, the mark bits will be set again on all subsequent passes through the loop. It is anticipated that this bit will always be set when token recovery is enabled. It is provided as a potential debug tool.\n1 = Enabled\n0 = Disabled\n\n",
#endif
    { FPM_TOKEN_RECOVER_CTL_TOKEN_REMARK_ENA_FIELD_MASK },
    0,
    { FPM_TOKEN_RECOVER_CTL_TOKEN_REMARK_ENA_FIELD_WIDTH },
    { FPM_TOKEN_RECOVER_CTL_TOKEN_REMARK_ENA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN_RECLAIM_ENA *****/
const ru_field_rec FPM_TOKEN_RECOVER_CTL_TOKEN_RECLAIM_ENA_FIELD =
{
    "TOKEN_RECLAIM_ENA",
#if RU_INCLUDE_DESC
    "",
    "Enable automatic return of marked tokens to the freepool\n1 = Enabled\n0 = Disabled\n\n",
#endif
    { FPM_TOKEN_RECOVER_CTL_TOKEN_RECLAIM_ENA_FIELD_MASK },
    0,
    { FPM_TOKEN_RECOVER_CTL_TOKEN_RECLAIM_ENA_FIELD_WIDTH },
    { FPM_TOKEN_RECOVER_CTL_TOKEN_RECLAIM_ENA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FORCE_TOKEN_RECLAIM *****/
const ru_field_rec FPM_TOKEN_RECOVER_CTL_FORCE_TOKEN_RECLAIM_FIELD =
{
    "FORCE_TOKEN_RECLAIM",
#if RU_INCLUDE_DESC
    "",
    "Non-automated token recovery.\nThis bit can be used when automatic token return is not enabled. When software gets an interrupt indicating that the token recovery process has detected expired tokens, it can set this bit to force the expired tokens to be reclaimed.\n1 = Enabled\n0 = Disabled\n\n",
#endif
    { FPM_TOKEN_RECOVER_CTL_FORCE_TOKEN_RECLAIM_FIELD_MASK },
    0,
    { FPM_TOKEN_RECOVER_CTL_FORCE_TOKEN_RECLAIM_FIELD_WIDTH },
    { FPM_TOKEN_RECOVER_CTL_FORCE_TOKEN_RECLAIM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CLR_EXPIRED_TOKEN_COUNT *****/
const ru_field_rec FPM_TOKEN_RECOVER_CTL_CLR_EXPIRED_TOKEN_COUNT_FIELD =
{
    "CLR_EXPIRED_TOKEN_COUNT",
#if RU_INCLUDE_DESC
    "",
    "This is a self-clearing bit. Write a 1 to the bit to reset  the EXPIRED_TOKEN_COUNT to 0.\n\n",
#endif
    { FPM_TOKEN_RECOVER_CTL_CLR_EXPIRED_TOKEN_COUNT_FIELD_MASK },
    0,
    { FPM_TOKEN_RECOVER_CTL_CLR_EXPIRED_TOKEN_COUNT_FIELD_WIDTH },
    { FPM_TOKEN_RECOVER_CTL_CLR_EXPIRED_TOKEN_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CLR_RECOVERED_TOKEN_COUNT *****/
const ru_field_rec FPM_TOKEN_RECOVER_CTL_CLR_RECOVERED_TOKEN_COUNT_FIELD =
{
    "CLR_RECOVERED_TOKEN_COUNT",
#if RU_INCLUDE_DESC
    "",
    "This is a self-clearing bit. Write a 1 to the bit to reset  the RECOVERED_TOKEN_COUNT to 0.\n\n",
#endif
    { FPM_TOKEN_RECOVER_CTL_CLR_RECOVERED_TOKEN_COUNT_FIELD_MASK },
    0,
    { FPM_TOKEN_RECOVER_CTL_CLR_RECOVERED_TOKEN_COUNT_FIELD_WIDTH },
    { FPM_TOKEN_RECOVER_CTL_CLR_RECOVERED_TOKEN_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec FPM_TOKEN_RECOVER_CTL_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_TOKEN_RECOVER_CTL_R1_FIELD_MASK },
    0,
    { FPM_TOKEN_RECOVER_CTL_R1_FIELD_WIDTH },
    { FPM_TOKEN_RECOVER_CTL_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_TOKEN_RECOVER_CTL_FIELDS[] =
{
    &FPM_TOKEN_RECOVER_CTL_TOKEN_RECOVER_ENA_FIELD,
    &FPM_TOKEN_RECOVER_CTL_SINGLE_PASS_ENA_FIELD,
    &FPM_TOKEN_RECOVER_CTL_TOKEN_REMARK_ENA_FIELD,
    &FPM_TOKEN_RECOVER_CTL_TOKEN_RECLAIM_ENA_FIELD,
    &FPM_TOKEN_RECOVER_CTL_FORCE_TOKEN_RECLAIM_FIELD,
    &FPM_TOKEN_RECOVER_CTL_CLR_EXPIRED_TOKEN_COUNT_FIELD,
    &FPM_TOKEN_RECOVER_CTL_CLR_RECOVERED_TOKEN_COUNT_FIELD,
    &FPM_TOKEN_RECOVER_CTL_R1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_TOKEN_RECOVER_CTL *****/
const ru_reg_rec FPM_TOKEN_RECOVER_CTL_REG =
{
    "TOKEN_RECOVER_CTL",
#if RU_INCLUDE_DESC
    "TOKEN_RECOVER_CTL Register",
    "No Description\n",
#endif
    { FPM_TOKEN_RECOVER_CTL_REG_OFFSET },
    0,
    0,
    1469,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    FPM_TOKEN_RECOVER_CTL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_SHORT_AGING_TIMER, TYPE: Type_FPM_BLOCK_FPM_POOL_0_SHORT_AGING_TIMER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TIMER *****/
const ru_field_rec FPM_SHORT_AGING_TIMER_TIMER_FIELD =
{
    "TIMER",
#if RU_INCLUDE_DESC
    "",
    "Aging timer used in token recovery\n\n",
#endif
    { FPM_SHORT_AGING_TIMER_TIMER_FIELD_MASK },
    0,
    { FPM_SHORT_AGING_TIMER_TIMER_FIELD_WIDTH },
    { FPM_SHORT_AGING_TIMER_TIMER_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_SHORT_AGING_TIMER_FIELDS[] =
{
    &FPM_SHORT_AGING_TIMER_TIMER_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_SHORT_AGING_TIMER *****/
const ru_reg_rec FPM_SHORT_AGING_TIMER_REG =
{
    "SHORT_AGING_TIMER",
#if RU_INCLUDE_DESC
    "SHORT_AGING_TIMER Register",
    "No Description\n",
#endif
    { FPM_SHORT_AGING_TIMER_REG_OFFSET },
    0,
    0,
    1470,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_SHORT_AGING_TIMER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_LONG_AGING_TIMER, TYPE: Type_FPM_BLOCK_FPM_POOL_0_LONG_AGING_TIMER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TIMER *****/
const ru_field_rec FPM_LONG_AGING_TIMER_TIMER_FIELD =
{
    "TIMER",
#if RU_INCLUDE_DESC
    "",
    "Aging timer used in token recovery\n\n",
#endif
    { FPM_LONG_AGING_TIMER_TIMER_FIELD_MASK },
    0,
    { FPM_LONG_AGING_TIMER_TIMER_FIELD_WIDTH },
    { FPM_LONG_AGING_TIMER_TIMER_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_LONG_AGING_TIMER_FIELDS[] =
{
    &FPM_LONG_AGING_TIMER_TIMER_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_LONG_AGING_TIMER *****/
const ru_reg_rec FPM_LONG_AGING_TIMER_REG =
{
    "LONG_AGING_TIMER",
#if RU_INCLUDE_DESC
    "LONG_AGING_TIMER Register",
    "No Description\n",
#endif
    { FPM_LONG_AGING_TIMER_REG_OFFSET },
    0,
    0,
    1471,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_LONG_AGING_TIMER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_CACHE_RECYCLE_TIMER, TYPE: Type_FPM_BLOCK_FPM_POOL_0_CACHE_RECYCLE_TIMER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RECYCLE_TIMER *****/
const ru_field_rec FPM_CACHE_RECYCLE_TIMER_RECYCLE_TIMER_FIELD =
{
    "RECYCLE_TIMER",
#if RU_INCLUDE_DESC
    "",
    "Timer used in token recovery logic. Upon expiration of timer, one token from the allocate cache will be freed. Over time, all cached tokens will be recycled back to the freepool. This will prevent the cached tokens frm being aged out by the token recovery logic. This timer should be set to a value so that all tokens can be recycled before the aging timer expires.\n\n",
#endif
    { FPM_CACHE_RECYCLE_TIMER_RECYCLE_TIMER_FIELD_MASK },
    0,
    { FPM_CACHE_RECYCLE_TIMER_RECYCLE_TIMER_FIELD_WIDTH },
    { FPM_CACHE_RECYCLE_TIMER_RECYCLE_TIMER_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec FPM_CACHE_RECYCLE_TIMER_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_CACHE_RECYCLE_TIMER_R1_FIELD_MASK },
    0,
    { FPM_CACHE_RECYCLE_TIMER_R1_FIELD_WIDTH },
    { FPM_CACHE_RECYCLE_TIMER_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_CACHE_RECYCLE_TIMER_FIELDS[] =
{
    &FPM_CACHE_RECYCLE_TIMER_RECYCLE_TIMER_FIELD,
    &FPM_CACHE_RECYCLE_TIMER_R1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_CACHE_RECYCLE_TIMER *****/
const ru_reg_rec FPM_CACHE_RECYCLE_TIMER_REG =
{
    "CACHE_RECYCLE_TIMER",
#if RU_INCLUDE_DESC
    "CACHE_RECYCLE_TIMER Register",
    "No Description\n",
#endif
    { FPM_CACHE_RECYCLE_TIMER_REG_OFFSET },
    0,
    0,
    1472,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_CACHE_RECYCLE_TIMER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_EXPIRED_TOKEN_COUNT, TYPE: Type_FPM_BLOCK_FPM_POOL_0_EXPIRED_TOKEN_COUNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec FPM_EXPIRED_TOKEN_COUNT_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Cumulative count of the number of expired tokens detected in the token recovery process. The count can be cleared by setting the CLR_EXPIRED_TOKEN_COUNT in the TOKEN_RECOVER_CTL register\n\n",
#endif
    { FPM_EXPIRED_TOKEN_COUNT_COUNT_FIELD_MASK },
    0,
    { FPM_EXPIRED_TOKEN_COUNT_COUNT_FIELD_WIDTH },
    { FPM_EXPIRED_TOKEN_COUNT_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_EXPIRED_TOKEN_COUNT_FIELDS[] =
{
    &FPM_EXPIRED_TOKEN_COUNT_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_EXPIRED_TOKEN_COUNT *****/
const ru_reg_rec FPM_EXPIRED_TOKEN_COUNT_REG =
{
    "EXPIRED_TOKEN_COUNT",
#if RU_INCLUDE_DESC
    "EXPIRED_TOKEN_COUNT Register",
    "No Description\n",
#endif
    { FPM_EXPIRED_TOKEN_COUNT_REG_OFFSET },
    0,
    0,
    1473,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_EXPIRED_TOKEN_COUNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_RECOVERED_TOKEN_COUNT, TYPE: Type_FPM_BLOCK_FPM_POOL_0_RECOVERED_TOKEN_COUNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec FPM_RECOVERED_TOKEN_COUNT_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Cumulative count of the number of expired tokens that were freed in the token recovery process. The count can be cleared by setting the CLR_RECOVERED_TOKEN_COUNT in the TOKEN_RECOVER_CTL register\n\n",
#endif
    { FPM_RECOVERED_TOKEN_COUNT_COUNT_FIELD_MASK },
    0,
    { FPM_RECOVERED_TOKEN_COUNT_COUNT_FIELD_WIDTH },
    { FPM_RECOVERED_TOKEN_COUNT_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_RECOVERED_TOKEN_COUNT_FIELDS[] =
{
    &FPM_RECOVERED_TOKEN_COUNT_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_RECOVERED_TOKEN_COUNT *****/
const ru_reg_rec FPM_RECOVERED_TOKEN_COUNT_REG =
{
    "RECOVERED_TOKEN_COUNT",
#if RU_INCLUDE_DESC
    "RECOVERED_TOKEN_COUNT Register",
    "No Description\n",
#endif
    { FPM_RECOVERED_TOKEN_COUNT_REG_OFFSET },
    0,
    0,
    1474,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_RECOVERED_TOKEN_COUNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_TOKEN_RECOVER_START_END, TYPE: Type_FPM_BLOCK_FPM_POOL_0_TOKEN_RECOVER_START_END
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: END_INDEX *****/
const ru_field_rec FPM_TOKEN_RECOVER_START_END_END_INDEX_FIELD =
{
    "END_INDEX",
#if RU_INCLUDE_DESC
    "",
    "End of token index range to be used when performing token recovery.\n\n",
#endif
    { FPM_TOKEN_RECOVER_START_END_END_INDEX_FIELD_MASK },
    0,
    { FPM_TOKEN_RECOVER_START_END_END_INDEX_FIELD_WIDTH },
    { FPM_TOKEN_RECOVER_START_END_END_INDEX_FIELD_SHIFT },
    16383,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: START_INDEX *****/
const ru_field_rec FPM_TOKEN_RECOVER_START_END_START_INDEX_FIELD =
{
    "START_INDEX",
#if RU_INCLUDE_DESC
    "",
    "Start of token index range to be used when performing token recovery.\n\n",
#endif
    { FPM_TOKEN_RECOVER_START_END_START_INDEX_FIELD_MASK },
    0,
    { FPM_TOKEN_RECOVER_START_END_START_INDEX_FIELD_WIDTH },
    { FPM_TOKEN_RECOVER_START_END_START_INDEX_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_TOKEN_RECOVER_START_END_FIELDS[] =
{
    &FPM_TOKEN_RECOVER_START_END_END_INDEX_FIELD,
    &FPM_TOKEN_RECOVER_START_END_START_INDEX_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_TOKEN_RECOVER_START_END *****/
const ru_reg_rec FPM_TOKEN_RECOVER_START_END_REG =
{
    "TOKEN_RECOVER_START_END",
#if RU_INCLUDE_DESC
    "TOKEN_RECOVER_START_END Register",
    "No Description\n",
#endif
    { FPM_TOKEN_RECOVER_START_END_REG_OFFSET },
    0,
    0,
    1475,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_TOKEN_RECOVER_START_END_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_SIZE_0_ALLOC_DEALLOC, TYPE: Type_FPM_BLOCK_FPM_TOKEN_0_SIZE_0_ALLOC_DEALLOC
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN_SIZE *****/
const ru_field_rec FPM_SIZE_0_ALLOC_DEALLOC_TOKEN_SIZE_FIELD =
{
    "TOKEN_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Buffer length or packet size in bytes\n",
#endif
    { FPM_SIZE_0_ALLOC_DEALLOC_TOKEN_SIZE_FIELD_MASK },
    0,
    { FPM_SIZE_0_ALLOC_DEALLOC_TOKEN_SIZE_FIELD_WIDTH },
    { FPM_SIZE_0_ALLOC_DEALLOC_TOKEN_SIZE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN_INDEX *****/
const ru_field_rec FPM_SIZE_0_ALLOC_DEALLOC_TOKEN_INDEX_FIELD =
{
    "TOKEN_INDEX",
#if RU_INCLUDE_DESC
    "",
    "Buffer Index Pointer >> 3\n\n",
#endif
    { FPM_SIZE_0_ALLOC_DEALLOC_TOKEN_INDEX_FIELD_MASK },
    0,
    { FPM_SIZE_0_ALLOC_DEALLOC_TOKEN_INDEX_FIELD_WIDTH },
    { FPM_SIZE_0_ALLOC_DEALLOC_TOKEN_INDEX_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SIZE_ID *****/
const ru_field_rec FPM_SIZE_0_ALLOC_DEALLOC_SIZE_ID_FIELD =
{
    "SIZE_ID",
#if RU_INCLUDE_DESC
    "",
    "Token Size Indicator\n2'h0: 8x buffers ( Jumbo = 18x buffers )\n2'h1: 4x buffers\n2'h2: 2x buffers\n2'h3: 1x buffers\n\n",
#endif
    { FPM_SIZE_0_ALLOC_DEALLOC_SIZE_ID_FIELD_MASK },
    0,
    { FPM_SIZE_0_ALLOC_DEALLOC_SIZE_ID_FIELD_WIDTH },
    { FPM_SIZE_0_ALLOC_DEALLOC_SIZE_ID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_SIZE_0_ALLOC_DEALLOC_FIELDS[] =
{
    &FPM_SIZE_0_ALLOC_DEALLOC_TOKEN_SIZE_FIELD,
    &FPM_SIZE_0_ALLOC_DEALLOC_TOKEN_INDEX_FIELD,
    &FPM_SIZE_0_ALLOC_DEALLOC_SIZE_ID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_SIZE_0_ALLOC_DEALLOC *****/
const ru_reg_rec FPM_SIZE_0_ALLOC_DEALLOC_REG =
{
    "SIZE_0_ALLOC_DEALLOC",
#if RU_INCLUDE_DESC
    "SIZE_0_ALLOC_DEALLOC Register",
    "The free pool FIFO contains pointers to the buffers in the pool. To allocate a buffer from the pool, read token from this port. To de-allocate/free a buffer to the pool , write the token of the buffer to this port. After reset, software must initialize the FIFO. The buffer size is given in the control register above. All buffers must be of the same size and contiguous. InValid Token Indicator\n0: No buffers available\n1: A valid token index is provided. If a token is de-allocated/freed without this bit set that causes an error and the token will be ignored, error counter in register offset 0xB8 will be incremented.\n",
#endif
    { FPM_SIZE_0_ALLOC_DEALLOC_REG_OFFSET },
    0,
    0,
    1476,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    FPM_SIZE_0_ALLOC_DEALLOC_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_SIZE_1_ALLOC_DEALLOC, TYPE: Type_FPM_BLOCK_FPM_TOKEN_0_SIZE_1_ALLOC_DEALLOC
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN_SIZE *****/
const ru_field_rec FPM_SIZE_1_ALLOC_DEALLOC_TOKEN_SIZE_FIELD =
{
    "TOKEN_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Buffer length or packet size in bytes\n",
#endif
    { FPM_SIZE_1_ALLOC_DEALLOC_TOKEN_SIZE_FIELD_MASK },
    0,
    { FPM_SIZE_1_ALLOC_DEALLOC_TOKEN_SIZE_FIELD_WIDTH },
    { FPM_SIZE_1_ALLOC_DEALLOC_TOKEN_SIZE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN_INDEX *****/
const ru_field_rec FPM_SIZE_1_ALLOC_DEALLOC_TOKEN_INDEX_FIELD =
{
    "TOKEN_INDEX",
#if RU_INCLUDE_DESC
    "",
    "Buffer Index Pointer >> 2\n[1:0] are always 0 and omitted.\n\n",
#endif
    { FPM_SIZE_1_ALLOC_DEALLOC_TOKEN_INDEX_FIELD_MASK },
    0,
    { FPM_SIZE_1_ALLOC_DEALLOC_TOKEN_INDEX_FIELD_WIDTH },
    { FPM_SIZE_1_ALLOC_DEALLOC_TOKEN_INDEX_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SIZE_ID *****/
const ru_field_rec FPM_SIZE_1_ALLOC_DEALLOC_SIZE_ID_FIELD =
{
    "SIZE_ID",
#if RU_INCLUDE_DESC
    "",
    "Token Size Indicator\n2'h0: 8x buffers ( Jumbo = 18x buffers )\n2'h1: 4x buffers\n2'h2: 2x buffers\n2'h3: 1x buffers\n\n",
#endif
    { FPM_SIZE_1_ALLOC_DEALLOC_SIZE_ID_FIELD_MASK },
    0,
    { FPM_SIZE_1_ALLOC_DEALLOC_SIZE_ID_FIELD_WIDTH },
    { FPM_SIZE_1_ALLOC_DEALLOC_SIZE_ID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_SIZE_1_ALLOC_DEALLOC_FIELDS[] =
{
    &FPM_SIZE_1_ALLOC_DEALLOC_TOKEN_SIZE_FIELD,
    &FPM_SIZE_1_ALLOC_DEALLOC_TOKEN_INDEX_FIELD,
    &FPM_SIZE_1_ALLOC_DEALLOC_SIZE_ID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_SIZE_1_ALLOC_DEALLOC *****/
const ru_reg_rec FPM_SIZE_1_ALLOC_DEALLOC_REG =
{
    "SIZE_1_ALLOC_DEALLOC",
#if RU_INCLUDE_DESC
    "SIZE_1_ALLOC_DEALLOC Register",
    "The free pool FIFO contains pointers to the buffers in the pool. To allocate a buffer from the pool, read token from this port. To de-allocate/free a buffer to the pool , write the token of the buffer to this port. After reset, software must initialize the FIFO. The buffer size is given in the control register above. All buffers must be of the same size and contiguous. InValid Token Indicator\n0: No buffers available\n1: A valid token index is provided. If a token is de-allocated/freed without this bit set that causes an error and the token will be ignored, error counter in register offset 0xB8 will be incremented.\n",
#endif
    { FPM_SIZE_1_ALLOC_DEALLOC_REG_OFFSET },
    0,
    0,
    1477,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    FPM_SIZE_1_ALLOC_DEALLOC_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_SIZE_2_ALLOC_DEALLOC, TYPE: Type_FPM_BLOCK_FPM_TOKEN_0_SIZE_2_ALLOC_DEALLOC
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN_SIZE *****/
const ru_field_rec FPM_SIZE_2_ALLOC_DEALLOC_TOKEN_SIZE_FIELD =
{
    "TOKEN_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Buffer length or packet size in bytes\n",
#endif
    { FPM_SIZE_2_ALLOC_DEALLOC_TOKEN_SIZE_FIELD_MASK },
    0,
    { FPM_SIZE_2_ALLOC_DEALLOC_TOKEN_SIZE_FIELD_WIDTH },
    { FPM_SIZE_2_ALLOC_DEALLOC_TOKEN_SIZE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN_INDEX *****/
const ru_field_rec FPM_SIZE_2_ALLOC_DEALLOC_TOKEN_INDEX_FIELD =
{
    "TOKEN_INDEX",
#if RU_INCLUDE_DESC
    "",
    "Buffer Index Pointer >> 1\n\n",
#endif
    { FPM_SIZE_2_ALLOC_DEALLOC_TOKEN_INDEX_FIELD_MASK },
    0,
    { FPM_SIZE_2_ALLOC_DEALLOC_TOKEN_INDEX_FIELD_WIDTH },
    { FPM_SIZE_2_ALLOC_DEALLOC_TOKEN_INDEX_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SIZE_ID *****/
const ru_field_rec FPM_SIZE_2_ALLOC_DEALLOC_SIZE_ID_FIELD =
{
    "SIZE_ID",
#if RU_INCLUDE_DESC
    "",
    "Token Size Indicator\n2'h0: 8x buffers ( Jumbo = 18x buffers )\n2'h1: 4x buffers\n2'h2: 2x buffers\n2'h3: 1x buffers\n\n",
#endif
    { FPM_SIZE_2_ALLOC_DEALLOC_SIZE_ID_FIELD_MASK },
    0,
    { FPM_SIZE_2_ALLOC_DEALLOC_SIZE_ID_FIELD_WIDTH },
    { FPM_SIZE_2_ALLOC_DEALLOC_SIZE_ID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_SIZE_2_ALLOC_DEALLOC_FIELDS[] =
{
    &FPM_SIZE_2_ALLOC_DEALLOC_TOKEN_SIZE_FIELD,
    &FPM_SIZE_2_ALLOC_DEALLOC_TOKEN_INDEX_FIELD,
    &FPM_SIZE_2_ALLOC_DEALLOC_SIZE_ID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_SIZE_2_ALLOC_DEALLOC *****/
const ru_reg_rec FPM_SIZE_2_ALLOC_DEALLOC_REG =
{
    "SIZE_2_ALLOC_DEALLOC",
#if RU_INCLUDE_DESC
    "SIZE_2_ALLOC_DEALLOC Register",
    "The free pool FIFO contains pointers to the buffers in the pool. To allocate a buffer from the pool, read token from this port. To de-allocate/free a buffer to the pool , write the token of the buffer to this port. After reset, software must initialize the FIFO. The buffer size is given in the control register above. All buffers must be of the same size and contiguous. InValid Token Indicator\n0: No buffers available\n1: A valid token index is provided. If a token is de-allocated/freed without this bit set that causes an error and the token will be ignored, error counter in register offset 0xB8 will be incremented.\n",
#endif
    { FPM_SIZE_2_ALLOC_DEALLOC_REG_OFFSET },
    0,
    0,
    1478,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    FPM_SIZE_2_ALLOC_DEALLOC_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_SIZE_3_ALLOC_DEALLOC, TYPE: Type_FPM_BLOCK_FPM_TOKEN_0_SIZE_3_ALLOC_DEALLOC
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN_SIZE *****/
const ru_field_rec FPM_SIZE_3_ALLOC_DEALLOC_TOKEN_SIZE_FIELD =
{
    "TOKEN_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Buffer length or packet size in bytes\n",
#endif
    { FPM_SIZE_3_ALLOC_DEALLOC_TOKEN_SIZE_FIELD_MASK },
    0,
    { FPM_SIZE_3_ALLOC_DEALLOC_TOKEN_SIZE_FIELD_WIDTH },
    { FPM_SIZE_3_ALLOC_DEALLOC_TOKEN_SIZE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN_INDEX *****/
const ru_field_rec FPM_SIZE_3_ALLOC_DEALLOC_TOKEN_INDEX_FIELD =
{
    "TOKEN_INDEX",
#if RU_INCLUDE_DESC
    "",
    "Buffer Index Pointer\n\n",
#endif
    { FPM_SIZE_3_ALLOC_DEALLOC_TOKEN_INDEX_FIELD_MASK },
    0,
    { FPM_SIZE_3_ALLOC_DEALLOC_TOKEN_INDEX_FIELD_WIDTH },
    { FPM_SIZE_3_ALLOC_DEALLOC_TOKEN_INDEX_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SIZE_ID *****/
const ru_field_rec FPM_SIZE_3_ALLOC_DEALLOC_SIZE_ID_FIELD =
{
    "SIZE_ID",
#if RU_INCLUDE_DESC
    "",
    "Token Size Indicator\n2'h0: 8x buffers ( Jumbo = 18x buffers )\n2'h1: 4x buffers\n2'h2: 2x buffers\n2'h3: 1x buffers\n\n",
#endif
    { FPM_SIZE_3_ALLOC_DEALLOC_SIZE_ID_FIELD_MASK },
    0,
    { FPM_SIZE_3_ALLOC_DEALLOC_SIZE_ID_FIELD_WIDTH },
    { FPM_SIZE_3_ALLOC_DEALLOC_SIZE_ID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_SIZE_3_ALLOC_DEALLOC_FIELDS[] =
{
    &FPM_SIZE_3_ALLOC_DEALLOC_TOKEN_SIZE_FIELD,
    &FPM_SIZE_3_ALLOC_DEALLOC_TOKEN_INDEX_FIELD,
    &FPM_SIZE_3_ALLOC_DEALLOC_SIZE_ID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_SIZE_3_ALLOC_DEALLOC *****/
const ru_reg_rec FPM_SIZE_3_ALLOC_DEALLOC_REG =
{
    "SIZE_3_ALLOC_DEALLOC",
#if RU_INCLUDE_DESC
    "SIZE_3_ALLOC_DEALLOC Register",
    "The free pool FIFO contains pointers to the buffers in the pool. To allocate a buffer from the pool, read token from this port. To de-allocate/free a buffer to the pool , write the token of the buffer to this port. After reset, software must initialize the FIFO. The buffer size is given in the control register above. All buffers must be of the same size and contiguous. InValid Token Indicator\n0: No buffers available\n1: A valid token index is provided. If a token is de-allocated/freed without this bit set that causes an error and the token will be ignored, error counter in register offset 0xB8 will be incremented.\n",
#endif
    { FPM_SIZE_3_ALLOC_DEALLOC_REG_OFFSET },
    0,
    0,
    1479,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    FPM_SIZE_3_ALLOC_DEALLOC_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_MULTI, TYPE: Type_FPM_BLOCK_FPM_TOKEN_0_MULTI
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN_MULTI *****/
const ru_field_rec FPM_MULTI_TOKEN_MULTI_FIELD =
{
    "TOKEN_MULTI",
#if RU_INCLUDE_DESC
    "",
    "New Multi-cast Value\n",
#endif
    { FPM_MULTI_TOKEN_MULTI_FIELD_MASK },
    0,
    { FPM_MULTI_TOKEN_MULTI_FIELD_WIDTH },
    { FPM_MULTI_TOKEN_MULTI_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_write,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R2 *****/
const ru_field_rec FPM_MULTI_R2_FIELD =
{
    "R2",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_MULTI_R2_FIELD_MASK },
    0,
    { FPM_MULTI_R2_FIELD_WIDTH },
    { FPM_MULTI_R2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: UPDATE_TYPE *****/
const ru_field_rec FPM_MULTI_UPDATE_TYPE_FIELD =
{
    "UPDATE_TYPE",
#if RU_INCLUDE_DESC
    "",
    "1'b0 - Count value is replaced with new value in bits[6:0]\n1'b1 - Count value is incremented by value in bits[6:0]\n\n",
#endif
    { FPM_MULTI_UPDATE_TYPE_FIELD_MASK },
    0,
    { FPM_MULTI_UPDATE_TYPE_FIELD_WIDTH },
    { FPM_MULTI_UPDATE_TYPE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_write,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN_INDEX *****/
const ru_field_rec FPM_MULTI_TOKEN_INDEX_FIELD =
{
    "TOKEN_INDEX",
#if RU_INCLUDE_DESC
    "",
    "Buffer Index Pointer\n\n",
#endif
    { FPM_MULTI_TOKEN_INDEX_FIELD_MASK },
    0,
    { FPM_MULTI_TOKEN_INDEX_FIELD_WIDTH },
    { FPM_MULTI_TOKEN_INDEX_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_write,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR *****/
const ru_field_rec FPM_MULTI_DDR_FIELD =
{
    "DDR",
#if RU_INCLUDE_DESC
    "",
    "DDR Identifier\n0: DDR0\n1: DDR1\n\n",
#endif
    { FPM_MULTI_DDR_FIELD_MASK },
    0,
    { FPM_MULTI_DDR_FIELD_WIDTH },
    { FPM_MULTI_DDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_write,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec FPM_MULTI_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_MULTI_R1_FIELD_MASK },
    0,
    { FPM_MULTI_R1_FIELD_WIDTH },
    { FPM_MULTI_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN_VALID *****/
const ru_field_rec FPM_MULTI_TOKEN_VALID_FIELD =
{
    "TOKEN_VALID",
#if RU_INCLUDE_DESC
    "",
    "Valid Token Indicator\n0: No buffers available\n1: A valid token index is provided. If a token multi-cast value is updated without this bit set, that causes an error and the token will be ignored, error counter in register offset 0xBC will be incremented.\n",
#endif
    { FPM_MULTI_TOKEN_VALID_FIELD_MASK },
    0,
    { FPM_MULTI_TOKEN_VALID_FIELD_WIDTH },
    { FPM_MULTI_TOKEN_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_write,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_MULTI_FIELDS[] =
{
    &FPM_MULTI_TOKEN_MULTI_FIELD,
    &FPM_MULTI_R2_FIELD,
    &FPM_MULTI_UPDATE_TYPE_FIELD,
    &FPM_MULTI_TOKEN_INDEX_FIELD,
    &FPM_MULTI_DDR_FIELD,
    &FPM_MULTI_R1_FIELD,
    &FPM_MULTI_TOKEN_VALID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_MULTI *****/
const ru_reg_rec FPM_MULTI_REG =
{
    "MULTI",
#if RU_INCLUDE_DESC
    "MULTI Register",
    "Update/Modify the multi-cast value of the token\n",
#endif
    { FPM_MULTI_REG_OFFSET },
    0,
    0,
    1480,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    FPM_MULTI_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_SEARCH_DATA_0, TYPE: Type_FPM_BLOCK_FPM_SEARCH_0_SEARCH_DATA_0
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec FPM_SEARCH_DATA_0_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "mem_data[31:0]\n",
#endif
    { FPM_SEARCH_DATA_0_DATA_FIELD_MASK },
    0,
    { FPM_SEARCH_DATA_0_DATA_FIELD_WIDTH },
    { FPM_SEARCH_DATA_0_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_SEARCH_DATA_0_FIELDS[] =
{
    &FPM_SEARCH_DATA_0_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_SEARCH_DATA_0 *****/
const ru_reg_rec FPM_SEARCH_DATA_0_REG =
{
    "SEARCH_DATA_0",
#if RU_INCLUDE_DESC
    "SEARCH_DATA_0 Register",
    "search_mem_line[31:0]\n",
#endif
    { FPM_SEARCH_DATA_0_REG_OFFSET },
    FPM_SEARCH_DATA_0_REG_RAM_CNT,
    16,
    1481,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_SEARCH_DATA_0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_SEARCH_DATA_1, TYPE: Type_FPM_BLOCK_FPM_SEARCH_0_SEARCH_DATA_1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec FPM_SEARCH_DATA_1_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "data[47:32]\n",
#endif
    { FPM_SEARCH_DATA_1_DATA_FIELD_MASK },
    0,
    { FPM_SEARCH_DATA_1_DATA_FIELD_WIDTH },
    { FPM_SEARCH_DATA_1_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_SEARCH_DATA_1_FIELDS[] =
{
    &FPM_SEARCH_DATA_1_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_SEARCH_DATA_1 *****/
const ru_reg_rec FPM_SEARCH_DATA_1_REG =
{
    "SEARCH_DATA_1",
#if RU_INCLUDE_DESC
    "SEARCH_DATA_1 Register",
    "search_mem_line[47:32]\n",
#endif
    { FPM_SEARCH_DATA_1_REG_OFFSET },
    FPM_SEARCH_DATA_1_REG_RAM_CNT,
    16,
    1482,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_SEARCH_DATA_1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_SEARCH_DATA_2, TYPE: Type_FPM_BLOCK_FPM_SEARCH_0_SEARCH_DATA_2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec FPM_SEARCH_DATA_2_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "mem_line[79:48]\n",
#endif
    { FPM_SEARCH_DATA_2_DATA_FIELD_MASK },
    0,
    { FPM_SEARCH_DATA_2_DATA_FIELD_WIDTH },
    { FPM_SEARCH_DATA_2_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_SEARCH_DATA_2_FIELDS[] =
{
    &FPM_SEARCH_DATA_2_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_SEARCH_DATA_2 *****/
const ru_reg_rec FPM_SEARCH_DATA_2_REG =
{
    "SEARCH_DATA_2",
#if RU_INCLUDE_DESC
    "SEARCH_DATA_2 Register",
    "search_mem_line[79:48]\n",
#endif
    { FPM_SEARCH_DATA_2_REG_OFFSET },
    FPM_SEARCH_DATA_2_REG_RAM_CNT,
    16,
    1483,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_SEARCH_DATA_2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_SEARCH_DATA_3, TYPE: Type_FPM_BLOCK_FPM_SEARCH_0_SEARCH_DATA_3
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec FPM_SEARCH_DATA_3_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "mem_line[95:80]\n",
#endif
    { FPM_SEARCH_DATA_3_DATA_FIELD_MASK },
    0,
    { FPM_SEARCH_DATA_3_DATA_FIELD_WIDTH },
    { FPM_SEARCH_DATA_3_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_SEARCH_DATA_3_FIELDS[] =
{
    &FPM_SEARCH_DATA_3_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_SEARCH_DATA_3 *****/
const ru_reg_rec FPM_SEARCH_DATA_3_REG =
{
    "SEARCH_DATA_3",
#if RU_INCLUDE_DESC
    "SEARCH_DATA_3 Register",
    "search_mem_line[95:80]\n",
#endif
    { FPM_SEARCH_DATA_3_REG_OFFSET },
    FPM_SEARCH_DATA_3_REG_RAM_CNT,
    16,
    1484,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_SEARCH_DATA_3_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_MULTICAST_DATA, TYPE: Type_FPM_BLOCK_FPM_MULTI_0_MULTICAST_DATA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec FPM_MULTICAST_DATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "mc ram 32bits\n",
#endif
    { FPM_MULTICAST_DATA_DATA_FIELD_MASK },
    0,
    { FPM_MULTICAST_DATA_DATA_FIELD_WIDTH },
    { FPM_MULTICAST_DATA_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_MULTICAST_DATA_FIELDS[] =
{
    &FPM_MULTICAST_DATA_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_MULTICAST_DATA *****/
const ru_reg_rec FPM_MULTICAST_DATA_REG =
{
    "MULTICAST_DATA",
#if RU_INCLUDE_DESC
    "MULTICAST_DATA 0..16383 Register",
    "MC ram (1 bit per token data)\n",
#endif
    { FPM_MULTICAST_DATA_REG_OFFSET },
    FPM_MULTICAST_DATA_REG_RAM_CNT,
    4,
    1485,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_MULTICAST_DATA_FIELDS,
#endif
};

unsigned long FPM_ADDRS[] =
{
    0xB1E00000,
};

static const ru_reg_rec *FPM_REGS[] =
{
    &FPM_CTL_REG,
    &FPM_WEIGHT_REG,
    &FPM_BB_CFG_REG,
    &FPM_SPARE_REG,
    &FPM_PRBS_INVALID_GEN_REG,
    &FPM_CTL_1_REG,
    &FPM_CFG1_REG,
    &FPM_CFG2_REG,
    &FPM_INTR_MSK_REG,
    &FPM_INTR_STS_REG,
    &FPM_STALL_MSK_REG,
    &FPM_CFG3_REG,
    &FPM_STAT1_REG,
    &FPM_STAT2_REG,
    &FPM_STAT3_REG,
    &FPM_STAT4_REG,
    &FPM_STAT5_REG,
    &FPM_STAT6_REG,
    &FPM_STAT7_REG,
    &FPM_STAT8_REG,
    &FPM_XON_XOFF_CFG_REG,
    &FPM_NOT_EMPTY_CFG_REG,
    &FPM_TOKEN_RECOVER_CTL_REG,
    &FPM_SHORT_AGING_TIMER_REG,
    &FPM_LONG_AGING_TIMER_REG,
    &FPM_CACHE_RECYCLE_TIMER_REG,
    &FPM_EXPIRED_TOKEN_COUNT_REG,
    &FPM_RECOVERED_TOKEN_COUNT_REG,
    &FPM_TOKEN_RECOVER_START_END_REG,
    &FPM_SIZE_0_ALLOC_DEALLOC_REG,
    &FPM_SIZE_1_ALLOC_DEALLOC_REG,
    &FPM_SIZE_2_ALLOC_DEALLOC_REG,
    &FPM_SIZE_3_ALLOC_DEALLOC_REG,
    &FPM_MULTI_REG,
    &FPM_SEARCH_DATA_0_REG,
    &FPM_SEARCH_DATA_1_REG,
    &FPM_SEARCH_DATA_2_REG,
    &FPM_SEARCH_DATA_3_REG,
    &FPM_MULTICAST_DATA_REG,
};

const ru_block_rec FPM_BLOCK =
{
    "FPM",
#if RU_INCLUDE_SIZE
    { 0x00070000 },
#endif
    FPM_ADDRS,
    1,
    39,
    FPM_REGS,
};
