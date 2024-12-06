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
 * Register: NAME: FPM_FPM_CTL, TYPE: Type_FPM_BLOCK_FPM_CTRL_FPM_CTL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: R4 *****/
const ru_field_rec FPM_FPM_CTL_R4_FIELD =
{
    "R4",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_FPM_CTL_R4_FIELD_MASK },
    0,
    { FPM_FPM_CTL_R4_FIELD_WIDTH },
    { FPM_FPM_CTL_R4_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: INIT_MEM_POOL3 *****/
const ru_field_rec FPM_FPM_CTL_INIT_MEM_POOL3_FIELD =
{
    "INIT_MEM_POOL3",
#if RU_INCLUDE_DESC
    "",
    "Clear memory - Initialize all bits of the usage index array memory to zero's\nThis is a self clearing bit. Once software writes a 1'b1 to enable, hardware initializes the memory and resets this bit back to 1'b0 at completion of initialization. Software can poll this bit and check for a value a zero that indicates initialization completion status\n\n",
#endif
    { FPM_FPM_CTL_INIT_MEM_POOL3_FIELD_MASK },
    0,
    { FPM_FPM_CTL_INIT_MEM_POOL3_FIELD_WIDTH },
    { FPM_FPM_CTL_INIT_MEM_POOL3_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: INIT_MEM_POOL2 *****/
const ru_field_rec FPM_FPM_CTL_INIT_MEM_POOL2_FIELD =
{
    "INIT_MEM_POOL2",
#if RU_INCLUDE_DESC
    "",
    "Clear memory - Initialize all bits of the usage index array memory to zero's\nThis is a self clearing bit. Once software writes a 1'b1 to enable, hardware initializes the memory and resets this bit back to 1'b0 at completion of initialization. Software can poll this bit and check for a value a zero that indicates initialization completion status\n\n",
#endif
    { FPM_FPM_CTL_INIT_MEM_POOL2_FIELD_MASK },
    0,
    { FPM_FPM_CTL_INIT_MEM_POOL2_FIELD_WIDTH },
    { FPM_FPM_CTL_INIT_MEM_POOL2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: INIT_MEM *****/
const ru_field_rec FPM_FPM_CTL_INIT_MEM_FIELD =
{
    "INIT_MEM",
#if RU_INCLUDE_DESC
    "",
    "Clear memory - Initialize all bits of the usage index array memory to zero's\nThis is a self clearing bit. Once software writes a 1'b1 to enable, hardware initializes the memory and resets this bit back to 1'b0 at completion of initialization. Software can poll this bit and check for a value a zero that indicates initialization completion status\n\n",
#endif
    { FPM_FPM_CTL_INIT_MEM_FIELD_MASK },
    0,
    { FPM_FPM_CTL_INIT_MEM_FIELD_WIDTH },
    { FPM_FPM_CTL_INIT_MEM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R3 *****/
const ru_field_rec FPM_FPM_CTL_R3_FIELD =
{
    "R3",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_FPM_CTL_R3_FIELD_MASK },
    0,
    { FPM_FPM_CTL_R3_FIELD_WIDTH },
    { FPM_FPM_CTL_R3_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FPM_BB_SOFT_RESET *****/
const ru_field_rec FPM_FPM_CTL_FPM_BB_SOFT_RESET_FIELD =
{
    "FPM_BB_SOFT_RESET",
#if RU_INCLUDE_DESC
    "",
    "Set to 1 to hold the FPM Broadbus interface in reset. This is useful for maintaining a known state on that interface when Runner is powered down.\n\n",
#endif
    { FPM_FPM_CTL_FPM_BB_SOFT_RESET_FIELD_MASK },
    0,
    { FPM_FPM_CTL_FPM_BB_SOFT_RESET_FIELD_WIDTH },
    { FPM_FPM_CTL_FPM_BB_SOFT_RESET_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R2 *****/
const ru_field_rec FPM_FPM_CTL_R2_FIELD =
{
    "R2",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_FPM_CTL_R2_FIELD_MASK },
    0,
    { FPM_FPM_CTL_R2_FIELD_WIDTH },
    { FPM_FPM_CTL_R2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL1_ENABLE *****/
const ru_field_rec FPM_FPM_CTL_POOL1_ENABLE_FIELD =
{
    "POOL1_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "Enable POOL1 token allocation / deallocation\n0 = Disabled\n1 = Enabled\n\n\n",
#endif
    { FPM_FPM_CTL_POOL1_ENABLE_FIELD_MASK },
    0,
    { FPM_FPM_CTL_POOL1_ENABLE_FIELD_WIDTH },
    { FPM_FPM_CTL_POOL1_ENABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL2_ENABLE *****/
const ru_field_rec FPM_FPM_CTL_POOL2_ENABLE_FIELD =
{
    "POOL2_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "Enable POOL2 token allocation / deallocation\n0 = Disabled\n1 = Enabled\n\n\n",
#endif
    { FPM_FPM_CTL_POOL2_ENABLE_FIELD_MASK },
    0,
    { FPM_FPM_CTL_POOL2_ENABLE_FIELD_WIDTH },
    { FPM_FPM_CTL_POOL2_ENABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL3_ENABLE *****/
const ru_field_rec FPM_FPM_CTL_POOL3_ENABLE_FIELD =
{
    "POOL3_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "Enable POOL2 token allocation / deallocation\n0 = Disabled\n1 = Enabled\n\n\n",
#endif
    { FPM_FPM_CTL_POOL3_ENABLE_FIELD_MASK },
    0,
    { FPM_FPM_CTL_POOL3_ENABLE_FIELD_WIDTH },
    { FPM_FPM_CTL_POOL3_ENABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec FPM_FPM_CTL_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_FPM_CTL_R1_FIELD_MASK },
    0,
    { FPM_FPM_CTL_R1_FIELD_WIDTH },
    { FPM_FPM_CTL_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: STOP_ALLOC_CACHE_LOAD *****/
const ru_field_rec FPM_FPM_CTL_STOP_ALLOC_CACHE_LOAD_FIELD =
{
    "STOP_ALLOC_CACHE_LOAD",
#if RU_INCLUDE_DESC
    "",
    "Stop loading allocation fifo/cache with new tokens. This is should be used for debug purposes only\n0 = Enable loading new tokens (normal operation)\n1 = Disable loading new tokens\n\n\n",
#endif
    { FPM_FPM_CTL_STOP_ALLOC_CACHE_LOAD_FIELD_MASK },
    0,
    { FPM_FPM_CTL_STOP_ALLOC_CACHE_LOAD_FIELD_WIDTH },
    { FPM_FPM_CTL_STOP_ALLOC_CACHE_LOAD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MEM_CORRUPT_CHECK_DISABLE *****/
const ru_field_rec FPM_FPM_CTL_MEM_CORRUPT_CHECK_DISABLE_FIELD =
{
    "MEM_CORRUPT_CHECK_DISABLE",
#if RU_INCLUDE_DESC
    "",
    "Disabling check for index memory corrupt during alloc/free/mcast updates. This should be used for debug purposes only\n0 = Enable memory corruption check (normal operation)\n1 = Disable memory corruption check\n\n\n",
#endif
    { FPM_FPM_CTL_MEM_CORRUPT_CHECK_DISABLE_FIELD_MASK },
    0,
    { FPM_FPM_CTL_MEM_CORRUPT_CHECK_DISABLE_FIELD_WIDTH },
    { FPM_FPM_CTL_MEM_CORRUPT_CHECK_DISABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TP_MUX_CNTRL *****/
const ru_field_rec FPM_FPM_CTL_TP_MUX_CNTRL_FIELD =
{
    "TP_MUX_CNTRL",
#if RU_INCLUDE_DESC
    "",
    "Test port mux control bits used to drive test signals from different submodules.\n\n",
#endif
    { FPM_FPM_CTL_TP_MUX_CNTRL_FIELD_MASK },
    0,
    { FPM_FPM_CTL_TP_MUX_CNTRL_FIELD_WIDTH },
    { FPM_FPM_CTL_TP_MUX_CNTRL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_FPM_CTL_FIELDS[] =
{
    &FPM_FPM_CTL_R4_FIELD,
    &FPM_FPM_CTL_INIT_MEM_POOL3_FIELD,
    &FPM_FPM_CTL_INIT_MEM_POOL2_FIELD,
    &FPM_FPM_CTL_INIT_MEM_FIELD,
    &FPM_FPM_CTL_R3_FIELD,
    &FPM_FPM_CTL_FPM_BB_SOFT_RESET_FIELD,
    &FPM_FPM_CTL_R2_FIELD,
    &FPM_FPM_CTL_POOL1_ENABLE_FIELD,
    &FPM_FPM_CTL_POOL2_ENABLE_FIELD,
    &FPM_FPM_CTL_POOL3_ENABLE_FIELD,
    &FPM_FPM_CTL_R1_FIELD,
    &FPM_FPM_CTL_STOP_ALLOC_CACHE_LOAD_FIELD,
    &FPM_FPM_CTL_MEM_CORRUPT_CHECK_DISABLE_FIELD,
    &FPM_FPM_CTL_TP_MUX_CNTRL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_FPM_CTL *****/
const ru_reg_rec FPM_FPM_CTL_REG =
{
    "FPM_CTL",
#if RU_INCLUDE_DESC
    "FPM_CTL Register",
    "No Description\n",
#endif
    { FPM_FPM_CTL_REG_OFFSET },
    0,
    0,
    442,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    14,
    FPM_FPM_CTL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_FPM_CFG1, TYPE: Type_FPM_BLOCK_FPM_CTRL_FPM_CFG1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL1_SEARCH_MODE *****/
const ru_field_rec FPM_FPM_CFG1_POOL1_SEARCH_MODE_FIELD =
{
    "POOL1_SEARCH_MODE",
#if RU_INCLUDE_DESC
    "",
    "Index memory search method\n(For more info refer to FPM architecture wiki page)\n0 = Method 1\n1 = Method 2\n\n\n",
#endif
    { FPM_FPM_CFG1_POOL1_SEARCH_MODE_FIELD_MASK },
    0,
    { FPM_FPM_CFG1_POOL1_SEARCH_MODE_FIELD_WIDTH },
    { FPM_FPM_CFG1_POOL1_SEARCH_MODE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ENABLE_JUMBO_SUPPORT *****/
const ru_field_rec FPM_FPM_CFG1_ENABLE_JUMBO_SUPPORT_FIELD =
{
    "ENABLE_JUMBO_SUPPORT",
#if RU_INCLUDE_DESC
    "",
    "Enable Jumbo Support0 = Disabled 1\n1 = Enabled 2\n\n\n",
#endif
    { FPM_FPM_CFG1_ENABLE_JUMBO_SUPPORT_FIELD_MASK },
    0,
    { FPM_FPM_CFG1_ENABLE_JUMBO_SUPPORT_FIELD_WIDTH },
    { FPM_FPM_CFG1_ENABLE_JUMBO_SUPPORT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec FPM_FPM_CFG1_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_FPM_CFG1_R1_FIELD_MASK },
    0,
    { FPM_FPM_CFG1_R1_FIELD_WIDTH },
    { FPM_FPM_CFG1_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_FPM_CFG1_FIELDS[] =
{
    &FPM_FPM_CFG1_POOL1_SEARCH_MODE_FIELD,
    &FPM_FPM_CFG1_ENABLE_JUMBO_SUPPORT_FIELD,
    &FPM_FPM_CFG1_R1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_FPM_CFG1 *****/
const ru_reg_rec FPM_FPM_CFG1_REG =
{
    "FPM_CFG1",
#if RU_INCLUDE_DESC
    "FPM_CFG1 Register",
    "No Description\n",
#endif
    { FPM_FPM_CFG1_REG_OFFSET },
    0,
    0,
    443,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    FPM_FPM_CFG1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_FPM_WEIGHT, TYPE: Type_FPM_BLOCK_FPM_CTRL_FPM_WEIGHT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR0_ALLOC_WEIGHT *****/
const ru_field_rec FPM_FPM_WEIGHT_DDR0_ALLOC_WEIGHT_FIELD =
{
    "DDR0_ALLOC_WEIGHT",
#if RU_INCLUDE_DESC
    "",
    "Weight assigned to each alloc from pool for DDR0\n\n",
#endif
    { FPM_FPM_WEIGHT_DDR0_ALLOC_WEIGHT_FIELD_MASK },
    0,
    { FPM_FPM_WEIGHT_DDR0_ALLOC_WEIGHT_FIELD_WIDTH },
    { FPM_FPM_WEIGHT_DDR0_ALLOC_WEIGHT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR0_FREE_WEIGHT *****/
const ru_field_rec FPM_FPM_WEIGHT_DDR0_FREE_WEIGHT_FIELD =
{
    "DDR0_FREE_WEIGHT",
#if RU_INCLUDE_DESC
    "",
    "Weight assigned to each free to pool for DDR0\n\n",
#endif
    { FPM_FPM_WEIGHT_DDR0_FREE_WEIGHT_FIELD_MASK },
    0,
    { FPM_FPM_WEIGHT_DDR0_FREE_WEIGHT_FIELD_WIDTH },
    { FPM_FPM_WEIGHT_DDR0_FREE_WEIGHT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR1_ALLOC_WEIGHT *****/
const ru_field_rec FPM_FPM_WEIGHT_DDR1_ALLOC_WEIGHT_FIELD =
{
    "DDR1_ALLOC_WEIGHT",
#if RU_INCLUDE_DESC
    "",
    "Weight assigned to each alloc from pool for DDR1\n\n",
#endif
    { FPM_FPM_WEIGHT_DDR1_ALLOC_WEIGHT_FIELD_MASK },
    0,
    { FPM_FPM_WEIGHT_DDR1_ALLOC_WEIGHT_FIELD_WIDTH },
    { FPM_FPM_WEIGHT_DDR1_ALLOC_WEIGHT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR1_FREE_WEIGHT *****/
const ru_field_rec FPM_FPM_WEIGHT_DDR1_FREE_WEIGHT_FIELD =
{
    "DDR1_FREE_WEIGHT",
#if RU_INCLUDE_DESC
    "",
    "Weight assigned to each free to pool for DDR1\n\n",
#endif
    { FPM_FPM_WEIGHT_DDR1_FREE_WEIGHT_FIELD_MASK },
    0,
    { FPM_FPM_WEIGHT_DDR1_FREE_WEIGHT_FIELD_WIDTH },
    { FPM_FPM_WEIGHT_DDR1_FREE_WEIGHT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_FPM_WEIGHT_FIELDS[] =
{
    &FPM_FPM_WEIGHT_DDR0_ALLOC_WEIGHT_FIELD,
    &FPM_FPM_WEIGHT_DDR0_FREE_WEIGHT_FIELD,
    &FPM_FPM_WEIGHT_DDR1_ALLOC_WEIGHT_FIELD,
    &FPM_FPM_WEIGHT_DDR1_FREE_WEIGHT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_FPM_WEIGHT *****/
const ru_reg_rec FPM_FPM_WEIGHT_REG =
{
    "FPM_WEIGHT",
#if RU_INCLUDE_DESC
    "FPM_WEIGHT Register",
    "No Description\n",
#endif
    { FPM_FPM_WEIGHT_REG_OFFSET },
    0,
    0,
    444,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    FPM_FPM_WEIGHT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_FPM_BB_CFG, TYPE: Type_FPM_BLOCK_FPM_CTRL_FPM_BB_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BB_DDR_SEL *****/
const ru_field_rec FPM_FPM_BB_CFG_BB_DDR_SEL_FIELD =
{
    "BB_DDR_SEL",
#if RU_INCLUDE_DESC
    "",
    "Select pool/DDR to be used when FPM_BB allocates tokens\n11 = reserved\n10 = allocate from both pools\n01 = pool1/DDR1\n00 = pool0/DDR0\n\n\n",
#endif
    { FPM_FPM_BB_CFG_BB_DDR_SEL_FIELD_MASK },
    0,
    { FPM_FPM_BB_CFG_BB_DDR_SEL_FIELD_WIDTH },
    { FPM_FPM_BB_CFG_BB_DDR_SEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec FPM_FPM_BB_CFG_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_FPM_BB_CFG_R1_FIELD_MASK },
    0,
    { FPM_FPM_BB_CFG_R1_FIELD_WIDTH },
    { FPM_FPM_BB_CFG_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_FPM_BB_CFG_FIELDS[] =
{
    &FPM_FPM_BB_CFG_BB_DDR_SEL_FIELD,
    &FPM_FPM_BB_CFG_R1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_FPM_BB_CFG *****/
const ru_reg_rec FPM_FPM_BB_CFG_REG =
{
    "FPM_BB_CFG",
#if RU_INCLUDE_DESC
    "FPM_BB_CFG Register",
    "No Description\n",
#endif
    { FPM_FPM_BB_CFG_REG_OFFSET },
    0,
    0,
    445,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_FPM_BB_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_POOL1_INTR_MSK, TYPE: Type_FPM_BLOCK_FPM_CTRL_POOL1_INTR_MSK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ALLOC_FIFO_FULL_MSK *****/
const ru_field_rec FPM_POOL1_INTR_MSK_ALLOC_FIFO_FULL_MSK_FIELD =
{
    "ALLOC_FIFO_FULL_MSK",
#if RU_INCLUDE_DESC
    "",
    "Allocation FIFO Full Interrupt mask.\n\n",
#endif
    { FPM_POOL1_INTR_MSK_ALLOC_FIFO_FULL_MSK_FIELD_MASK },
    0,
    { FPM_POOL1_INTR_MSK_ALLOC_FIFO_FULL_MSK_FIELD_WIDTH },
    { FPM_POOL1_INTR_MSK_ALLOC_FIFO_FULL_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_FIFO_FULL_MSK *****/
const ru_field_rec FPM_POOL1_INTR_MSK_FREE_FIFO_FULL_MSK_FIELD =
{
    "FREE_FIFO_FULL_MSK",
#if RU_INCLUDE_DESC
    "",
    "De-Allocation FIFO Full Interrupt mask.\n\n",
#endif
    { FPM_POOL1_INTR_MSK_FREE_FIFO_FULL_MSK_FIELD_MASK },
    0,
    { FPM_POOL1_INTR_MSK_FREE_FIFO_FULL_MSK_FIELD_WIDTH },
    { FPM_POOL1_INTR_MSK_FREE_FIFO_FULL_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_FULL_MSK *****/
const ru_field_rec FPM_POOL1_INTR_MSK_POOL_FULL_MSK_FIELD =
{
    "POOL_FULL_MSK",
#if RU_INCLUDE_DESC
    "",
    "Usage Index Pool is fully allocated interrupt mask.\n\n",
#endif
    { FPM_POOL1_INTR_MSK_POOL_FULL_MSK_FIELD_MASK },
    0,
    { FPM_POOL1_INTR_MSK_POOL_FULL_MSK_FIELD_WIDTH },
    { FPM_POOL1_INTR_MSK_POOL_FULL_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_TOKEN_NO_VALID_MSK *****/
const ru_field_rec FPM_POOL1_INTR_MSK_FREE_TOKEN_NO_VALID_MSK_FIELD =
{
    "FREE_TOKEN_NO_VALID_MSK",
#if RU_INCLUDE_DESC
    "",
    "De-allocation token request with invalid token.\n\n",
#endif
    { FPM_POOL1_INTR_MSK_FREE_TOKEN_NO_VALID_MSK_FIELD_MASK },
    0,
    { FPM_POOL1_INTR_MSK_FREE_TOKEN_NO_VALID_MSK_FIELD_WIDTH },
    { FPM_POOL1_INTR_MSK_FREE_TOKEN_NO_VALID_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK *****/
const ru_field_rec FPM_POOL1_INTR_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD =
{
    "FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK",
#if RU_INCLUDE_DESC
    "",
    "De-allocation token request with index out-of-range.\n\n",
#endif
    { FPM_POOL1_INTR_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD_MASK },
    0,
    { FPM_POOL1_INTR_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD_WIDTH },
    { FPM_POOL1_INTR_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MULTI_TOKEN_NO_VALID_MSK *****/
const ru_field_rec FPM_POOL1_INTR_MSK_MULTI_TOKEN_NO_VALID_MSK_FIELD =
{
    "MULTI_TOKEN_NO_VALID_MSK",
#if RU_INCLUDE_DESC
    "",
    "Token multi-cast value update request with invalid token.\n\n",
#endif
    { FPM_POOL1_INTR_MSK_MULTI_TOKEN_NO_VALID_MSK_FIELD_MASK },
    0,
    { FPM_POOL1_INTR_MSK_MULTI_TOKEN_NO_VALID_MSK_FIELD_WIDTH },
    { FPM_POOL1_INTR_MSK_MULTI_TOKEN_NO_VALID_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK *****/
const ru_field_rec FPM_POOL1_INTR_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD =
{
    "MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK",
#if RU_INCLUDE_DESC
    "",
    "Token multi-cast value update request with index out-of-range.\n\n",
#endif
    { FPM_POOL1_INTR_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD_MASK },
    0,
    { FPM_POOL1_INTR_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD_WIDTH },
    { FPM_POOL1_INTR_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_DIS_FREE_MULTI_MSK *****/
const ru_field_rec FPM_POOL1_INTR_MSK_POOL_DIS_FREE_MULTI_MSK_FIELD =
{
    "POOL_DIS_FREE_MULTI_MSK",
#if RU_INCLUDE_DESC
    "",
    "Free or Mcast update on disabled pool interrupt mask .\n\n",
#endif
    { FPM_POOL1_INTR_MSK_POOL_DIS_FREE_MULTI_MSK_FIELD_MASK },
    0,
    { FPM_POOL1_INTR_MSK_POOL_DIS_FREE_MULTI_MSK_FIELD_WIDTH },
    { FPM_POOL1_INTR_MSK_POOL_DIS_FREE_MULTI_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MEMORY_CORRUPT_MSK *****/
const ru_field_rec FPM_POOL1_INTR_MSK_MEMORY_CORRUPT_MSK_FIELD =
{
    "MEMORY_CORRUPT_MSK",
#if RU_INCLUDE_DESC
    "",
    "Index Memory corrupt interrupt mask.\n\n",
#endif
    { FPM_POOL1_INTR_MSK_MEMORY_CORRUPT_MSK_FIELD_MASK },
    0,
    { FPM_POOL1_INTR_MSK_MEMORY_CORRUPT_MSK_FIELD_WIDTH },
    { FPM_POOL1_INTR_MSK_MEMORY_CORRUPT_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: XOFF_MSK *****/
const ru_field_rec FPM_POOL1_INTR_MSK_XOFF_MSK_FIELD =
{
    "XOFF_MSK",
#if RU_INCLUDE_DESC
    "",
    "XOFF_STATE interrupt mask.\n\n",
#endif
    { FPM_POOL1_INTR_MSK_XOFF_MSK_FIELD_MASK },
    0,
    { FPM_POOL1_INTR_MSK_XOFF_MSK_FIELD_WIDTH },
    { FPM_POOL1_INTR_MSK_XOFF_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: XON_MSK *****/
const ru_field_rec FPM_POOL1_INTR_MSK_XON_MSK_FIELD =
{
    "XON_MSK",
#if RU_INCLUDE_DESC
    "",
    "XON_STATE interrupt mask.\n\n",
#endif
    { FPM_POOL1_INTR_MSK_XON_MSK_FIELD_MASK },
    0,
    { FPM_POOL1_INTR_MSK_XON_MSK_FIELD_WIDTH },
    { FPM_POOL1_INTR_MSK_XON_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ILLEGAL_ADDRESS_ACCESS_MSK *****/
const ru_field_rec FPM_POOL1_INTR_MSK_ILLEGAL_ADDRESS_ACCESS_MSK_FIELD =
{
    "ILLEGAL_ADDRESS_ACCESS_MSK",
#if RU_INCLUDE_DESC
    "",
    "Illegal/un-implemented register/memory space access  interrupt mask.\n\n",
#endif
    { FPM_POOL1_INTR_MSK_ILLEGAL_ADDRESS_ACCESS_MSK_FIELD_MASK },
    0,
    { FPM_POOL1_INTR_MSK_ILLEGAL_ADDRESS_ACCESS_MSK_FIELD_WIDTH },
    { FPM_POOL1_INTR_MSK_ILLEGAL_ADDRESS_ACCESS_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ILLEGAL_ALLOC_REQUEST_MSK *****/
const ru_field_rec FPM_POOL1_INTR_MSK_ILLEGAL_ALLOC_REQUEST_MSK_FIELD =
{
    "ILLEGAL_ALLOC_REQUEST_MSK",
#if RU_INCLUDE_DESC
    "",
    "Illegal token request interrupt mask.\n\n",
#endif
    { FPM_POOL1_INTR_MSK_ILLEGAL_ALLOC_REQUEST_MSK_FIELD_MASK },
    0,
    { FPM_POOL1_INTR_MSK_ILLEGAL_ALLOC_REQUEST_MSK_FIELD_WIDTH },
    { FPM_POOL1_INTR_MSK_ILLEGAL_ALLOC_REQUEST_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EXPIRED_TOKEN_DET_MSK *****/
const ru_field_rec FPM_POOL1_INTR_MSK_EXPIRED_TOKEN_DET_MSK_FIELD =
{
    "EXPIRED_TOKEN_DET_MSK",
#if RU_INCLUDE_DESC
    "",
    "Expired token detect interrupt mask.\n\n",
#endif
    { FPM_POOL1_INTR_MSK_EXPIRED_TOKEN_DET_MSK_FIELD_MASK },
    0,
    { FPM_POOL1_INTR_MSK_EXPIRED_TOKEN_DET_MSK_FIELD_WIDTH },
    { FPM_POOL1_INTR_MSK_EXPIRED_TOKEN_DET_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EXPIRED_TOKEN_RECOV_MSK *****/
const ru_field_rec FPM_POOL1_INTR_MSK_EXPIRED_TOKEN_RECOV_MSK_FIELD =
{
    "EXPIRED_TOKEN_RECOV_MSK",
#if RU_INCLUDE_DESC
    "",
    "Expired token recovered interrupt mask.\n\n",
#endif
    { FPM_POOL1_INTR_MSK_EXPIRED_TOKEN_RECOV_MSK_FIELD_MASK },
    0,
    { FPM_POOL1_INTR_MSK_EXPIRED_TOKEN_RECOV_MSK_FIELD_WIDTH },
    { FPM_POOL1_INTR_MSK_EXPIRED_TOKEN_RECOV_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec FPM_POOL1_INTR_MSK_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_POOL1_INTR_MSK_R1_FIELD_MASK },
    0,
    { FPM_POOL1_INTR_MSK_R1_FIELD_WIDTH },
    { FPM_POOL1_INTR_MSK_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL1_INTR_MSK_FIELDS[] =
{
    &FPM_POOL1_INTR_MSK_ALLOC_FIFO_FULL_MSK_FIELD,
    &FPM_POOL1_INTR_MSK_FREE_FIFO_FULL_MSK_FIELD,
    &FPM_POOL1_INTR_MSK_POOL_FULL_MSK_FIELD,
    &FPM_POOL1_INTR_MSK_FREE_TOKEN_NO_VALID_MSK_FIELD,
    &FPM_POOL1_INTR_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD,
    &FPM_POOL1_INTR_MSK_MULTI_TOKEN_NO_VALID_MSK_FIELD,
    &FPM_POOL1_INTR_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD,
    &FPM_POOL1_INTR_MSK_POOL_DIS_FREE_MULTI_MSK_FIELD,
    &FPM_POOL1_INTR_MSK_MEMORY_CORRUPT_MSK_FIELD,
    &FPM_POOL1_INTR_MSK_XOFF_MSK_FIELD,
    &FPM_POOL1_INTR_MSK_XON_MSK_FIELD,
    &FPM_POOL1_INTR_MSK_ILLEGAL_ADDRESS_ACCESS_MSK_FIELD,
    &FPM_POOL1_INTR_MSK_ILLEGAL_ALLOC_REQUEST_MSK_FIELD,
    &FPM_POOL1_INTR_MSK_EXPIRED_TOKEN_DET_MSK_FIELD,
    &FPM_POOL1_INTR_MSK_EXPIRED_TOKEN_RECOV_MSK_FIELD,
    &FPM_POOL1_INTR_MSK_R1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_POOL1_INTR_MSK *****/
const ru_reg_rec FPM_POOL1_INTR_MSK_REG =
{
    "POOL1_INTR_MSK",
#if RU_INCLUDE_DESC
    "POOL1_INTR_MSK Register",
    "Mask bits are active high and are disabled by default. Software enables desired bits as necessary\n\n",
#endif
    { FPM_POOL1_INTR_MSK_REG_OFFSET },
    0,
    0,
    446,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    16,
    FPM_POOL1_INTR_MSK_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_POOL1_INTR_STS, TYPE: Type_FPM_BLOCK_FPM_CTRL_POOL1_INTR_STS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ALLOC_FIFO_FULL_STS *****/
const ru_field_rec FPM_POOL1_INTR_STS_ALLOC_FIFO_FULL_STS_FIELD =
{
    "ALLOC_FIFO_FULL_STS",
#if RU_INCLUDE_DESC
    "",
    "Allocation FIFO Full Interrupt. This is a functional status bit, not an error status bit. This indicates that allocation FIFO is full with new tokens to be allocated and will be active (high) as long as FIFO is full. This status is intended to be used for debug purpose only.\n\n",
#endif
    { FPM_POOL1_INTR_STS_ALLOC_FIFO_FULL_STS_FIELD_MASK },
    0,
    { FPM_POOL1_INTR_STS_ALLOC_FIFO_FULL_STS_FIELD_WIDTH },
    { FPM_POOL1_INTR_STS_ALLOC_FIFO_FULL_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_FIFO_FULL_STS *****/
const ru_field_rec FPM_POOL1_INTR_STS_FREE_FIFO_FULL_STS_FIELD =
{
    "FREE_FIFO_FULL_STS",
#if RU_INCLUDE_DESC
    "",
    "De-Allocation FIFO Full Interrupt. This is a functional status bit, not an error status bit. This indicates that de-allocation FIFO is full with tokens needs to be freed and will be active (high) as long as FIFO is full. This status is intended to be used for debug purpose only.\n\n",
#endif
    { FPM_POOL1_INTR_STS_FREE_FIFO_FULL_STS_FIELD_MASK },
    0,
    { FPM_POOL1_INTR_STS_FREE_FIFO_FULL_STS_FIELD_WIDTH },
    { FPM_POOL1_INTR_STS_FREE_FIFO_FULL_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_FULL_STS *****/
const ru_field_rec FPM_POOL1_INTR_STS_POOL_FULL_STS_FIELD =
{
    "POOL_FULL_STS",
#if RU_INCLUDE_DESC
    "",
    "Usage Index Pool is fully allocated interrupt. This is a functional status bit, not an error status bit. This indicates that token pool is fully allocated and there are no free tokens available. This bit will be active (high) as long as there no free tokens available to allocate. This bit is intended to be used for debug purpose only.\n\n",
#endif
    { FPM_POOL1_INTR_STS_POOL_FULL_STS_FIELD_MASK },
    0,
    { FPM_POOL1_INTR_STS_POOL_FULL_STS_FIELD_WIDTH },
    { FPM_POOL1_INTR_STS_POOL_FULL_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_TOKEN_NO_VALID_STS *****/
const ru_field_rec FPM_POOL1_INTR_STS_FREE_TOKEN_NO_VALID_STS_FIELD =
{
    "FREE_TOKEN_NO_VALID_STS",
#if RU_INCLUDE_DESC
    "",
    "De-allocation token request with invalid token Interrupt.\nInvalid free token is determined when one or more the following conditions are met -\n1. Incoming multi-cast request token is not valid ( 0xffffffff )\n2. Incoming free request token entry in the usage array indicates it is not an allocated token, i.e., associated use count value for this count in the usage array is zero\nNote: item 2 is not checked if auto_pool_en is set. The auto_pool_en bit is always set when using the new token format without a pool select field.\n\n",
#endif
    { FPM_POOL1_INTR_STS_FREE_TOKEN_NO_VALID_STS_FIELD_MASK },
    0,
    { FPM_POOL1_INTR_STS_FREE_TOKEN_NO_VALID_STS_FIELD_WIDTH },
    { FPM_POOL1_INTR_STS_FREE_TOKEN_NO_VALID_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_TOKEN_INDEX_OUT_OF_RANGE_STS *****/
const ru_field_rec FPM_POOL1_INTR_STS_FREE_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD =
{
    "FREE_TOKEN_INDEX_OUT_OF_RANGE_STS",
#if RU_INCLUDE_DESC
    "",
    "Not applicable for BCM3391.  All token values are valid.\nDe-allocation token request with index out-of-range Interrupt.\nFree token index out of range is determined when one or more of the following conditions are met -\n1. Incoming free request token index is not aligned to the pool size indicated by the pool select field (bits[29:28])\n2. The buffer size indicated by the size field (bits[11:0]) is greater than the size of the allocated token.\nThere is no associated count for this error. Note: item 1 is not checked if auto_pool_en is set. The auto_pool_en bit is always set when using the new token format without a pool select field.\n\n",
#endif
    { FPM_POOL1_INTR_STS_FREE_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD_MASK },
    0,
    { FPM_POOL1_INTR_STS_FREE_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD_WIDTH },
    { FPM_POOL1_INTR_STS_FREE_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MULTI_TOKEN_NO_VALID_STS *****/
const ru_field_rec FPM_POOL1_INTR_STS_MULTI_TOKEN_NO_VALID_STS_FIELD =
{
    "MULTI_TOKEN_NO_VALID_STS",
#if RU_INCLUDE_DESC
    "",
    "Token multi-cast value update request with invalid token Interrupt.\nInvalid multi-cast token is determined when one or more the following conditions are met -\n1. Incoming multi-cast request token is not valid ( 0xffffffff )\n2. Incoming multi-cast request token has use count field set to zero\n3. Incoming multi-cast request token entry in the usage array indicates it is not an allocated token, i.e., associated use count value for this count in the usage array is zero\n4. After updating the use count value, the new use count value exceeds 0x7E\nNote: item 2 is not checked if auto_pool_en is set. The auto_pool_en bit is always set when using the new token format without a pool select field.\n\n",
#endif
    { FPM_POOL1_INTR_STS_MULTI_TOKEN_NO_VALID_STS_FIELD_MASK },
    0,
    { FPM_POOL1_INTR_STS_MULTI_TOKEN_NO_VALID_STS_FIELD_WIDTH },
    { FPM_POOL1_INTR_STS_MULTI_TOKEN_NO_VALID_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS *****/
const ru_field_rec FPM_POOL1_INTR_STS_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD =
{
    "MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS",
#if RU_INCLUDE_DESC
    "",
    "Token multi-cast value update request with index out-of-range Interrupt.\nThis is set when the token index is not aligned to the pool size. This is determined by examining the pool select field (bits[29:28]) and the 3 lsbs of the token index (bits[14:12]). There is no associated count for this error. Note: this error is not checked if auto_pool_en is set. The auto_pool_en bit is always set when using the new token format without a pool select field.\n\n",
#endif
    { FPM_POOL1_INTR_STS_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD_MASK },
    0,
    { FPM_POOL1_INTR_STS_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD_WIDTH },
    { FPM_POOL1_INTR_STS_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_DIS_FREE_MULTI_STS *****/
const ru_field_rec FPM_POOL1_INTR_STS_POOL_DIS_FREE_MULTI_STS_FIELD =
{
    "POOL_DIS_FREE_MULTI_STS",
#if RU_INCLUDE_DESC
    "",
    "Free or Mcast update on disabled pool interrupt.\nThis bit goes active when a free or multi-cast request is received and FPM is not enabled, i.e., pool enable bit in FPM control register is not set to 1'b1.\n\n",
#endif
    { FPM_POOL1_INTR_STS_POOL_DIS_FREE_MULTI_STS_FIELD_MASK },
    0,
    { FPM_POOL1_INTR_STS_POOL_DIS_FREE_MULTI_STS_FIELD_WIDTH },
    { FPM_POOL1_INTR_STS_POOL_DIS_FREE_MULTI_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MEMORY_CORRUPT_STS *****/
const ru_field_rec FPM_POOL1_INTR_STS_MEMORY_CORRUPT_STS_FIELD =
{
    "MEMORY_CORRUPT_STS",
#if RU_INCLUDE_DESC
    "",
    "Index Memory is corrupted.\nDuring updates of the usage array, token manager checks if the use count and search tree value in the array has a legal value. If the use count or search tree value is not correct before updating, logic generates an error and interrupt. As long as the interrupt is active no more valid tokens will be allocated because this is a catastrophic error. Following are the two error conditions that are checked -\n1.    During search for a free token, a particular token use count value indicates it is allocated (use count is greater than 0), but corresponding upper level search tree value indicates the token is still available (with bit value of 1'b0, instead of 1'b1). This is an error.\n2.    During search for a free token, a particular token use count value indicates that it is free (use count is 0), but corresponding upper level search tree value indicates the token is not available (with bit value of 1'b1, instead of 1'b0). This is an error.\n\n",
#endif
    { FPM_POOL1_INTR_STS_MEMORY_CORRUPT_STS_FIELD_MASK },
    0,
    { FPM_POOL1_INTR_STS_MEMORY_CORRUPT_STS_FIELD_WIDTH },
    { FPM_POOL1_INTR_STS_MEMORY_CORRUPT_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: XOFF_STATE_STS *****/
const ru_field_rec FPM_POOL1_INTR_STS_XOFF_STATE_STS_FIELD =
{
    "XOFF_STATE_STS",
#if RU_INCLUDE_DESC
    "",
    "Number of available tokens is less than or equal to XOFF_THRESHOLD value in XON/XOFF Threshold configuration register. This is a functional status bit, not an error status bit. Using this information FPM generates backpressure output signal that is used by other UBUS client logics to throttle its operation. For example, UNIMAC logic can use backpressure signal to transfer PAUSE Ethernet flow control packets to throttle incoming frames on Ethernet interface.\n\n",
#endif
    { FPM_POOL1_INTR_STS_XOFF_STATE_STS_FIELD_MASK },
    0,
    { FPM_POOL1_INTR_STS_XOFF_STATE_STS_FIELD_WIDTH },
    { FPM_POOL1_INTR_STS_XOFF_STATE_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: XON_STATE_STS *****/
const ru_field_rec FPM_POOL1_INTR_STS_XON_STATE_STS_FIELD =
{
    "XON_STATE_STS",
#if RU_INCLUDE_DESC
    "",
    "Number of available tokens is greater than or equal to XON_THRESHOLD value in XON/XOFF Threshold configuration register. This is a functional status bit, not an error status bit. Using this information FPM generates backpressure output signal that is used by other UBUS client logics to throttle its operation. For example, UNIMAC logic can use backpressure signal to transfer PAUSE Ethernet flow control packets to throttle incoming frames on Ethernet interface.\n\n",
#endif
    { FPM_POOL1_INTR_STS_XON_STATE_STS_FIELD_MASK },
    0,
    { FPM_POOL1_INTR_STS_XON_STATE_STS_FIELD_WIDTH },
    { FPM_POOL1_INTR_STS_XON_STATE_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ILLEGAL_ADDRESS_ACCESS_STS *****/
const ru_field_rec FPM_POOL1_INTR_STS_ILLEGAL_ADDRESS_ACCESS_STS_FIELD =
{
    "ILLEGAL_ADDRESS_ACCESS_STS",
#if RU_INCLUDE_DESC
    "",
    "Illegal/un-implemented register/memory space access interrupt. This will be active when there is an attempt to read from an unimplemented register or memory space. Along with interrupt being sent an error reply packet will be sent out with o_ubus_error_out asserted.\n\n",
#endif
    { FPM_POOL1_INTR_STS_ILLEGAL_ADDRESS_ACCESS_STS_FIELD_MASK },
    0,
    { FPM_POOL1_INTR_STS_ILLEGAL_ADDRESS_ACCESS_STS_FIELD_WIDTH },
    { FPM_POOL1_INTR_STS_ILLEGAL_ADDRESS_ACCESS_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ILLEGAL_ALLOC_REQUEST_STS *****/
const ru_field_rec FPM_POOL1_INTR_STS_ILLEGAL_ALLOC_REQUEST_STS_FIELD =
{
    "ILLEGAL_ALLOC_REQUEST_STS",
#if RU_INCLUDE_DESC
    "",
    "Illegal token request interrupt. This will be active when the pool is disabled, there is a request for a new token and the alloc fifo for the selected token size is empty. Along with interrupt being sent an error reply packet will be sent out with o_ubus_error_out asserted.\n\n",
#endif
    { FPM_POOL1_INTR_STS_ILLEGAL_ALLOC_REQUEST_STS_FIELD_MASK },
    0,
    { FPM_POOL1_INTR_STS_ILLEGAL_ALLOC_REQUEST_STS_FIELD_WIDTH },
    { FPM_POOL1_INTR_STS_ILLEGAL_ALLOC_REQUEST_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EXPIRED_TOKEN_DET_STS *****/
const ru_field_rec FPM_POOL1_INTR_STS_EXPIRED_TOKEN_DET_STS_FIELD =
{
    "EXPIRED_TOKEN_DET_STS",
#if RU_INCLUDE_DESC
    "",
    "Expired token detect interrupt. This is set when the token recovery logic detects a token that has been held for the entire duration of the aging timer.\n\n",
#endif
    { FPM_POOL1_INTR_STS_EXPIRED_TOKEN_DET_STS_FIELD_MASK },
    0,
    { FPM_POOL1_INTR_STS_EXPIRED_TOKEN_DET_STS_FIELD_WIDTH },
    { FPM_POOL1_INTR_STS_EXPIRED_TOKEN_DET_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EXPIRED_TOKEN_RECOV_STS *****/
const ru_field_rec FPM_POOL1_INTR_STS_EXPIRED_TOKEN_RECOV_STS_FIELD =
{
    "EXPIRED_TOKEN_RECOV_STS",
#if RU_INCLUDE_DESC
    "",
    "Expired token recovered interrupt. This is set when an expired token has been recoveredand returned to pool as an available token.\n\n",
#endif
    { FPM_POOL1_INTR_STS_EXPIRED_TOKEN_RECOV_STS_FIELD_MASK },
    0,
    { FPM_POOL1_INTR_STS_EXPIRED_TOKEN_RECOV_STS_FIELD_WIDTH },
    { FPM_POOL1_INTR_STS_EXPIRED_TOKEN_RECOV_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec FPM_POOL1_INTR_STS_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_POOL1_INTR_STS_R1_FIELD_MASK },
    0,
    { FPM_POOL1_INTR_STS_R1_FIELD_WIDTH },
    { FPM_POOL1_INTR_STS_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL1_INTR_STS_FIELDS[] =
{
    &FPM_POOL1_INTR_STS_ALLOC_FIFO_FULL_STS_FIELD,
    &FPM_POOL1_INTR_STS_FREE_FIFO_FULL_STS_FIELD,
    &FPM_POOL1_INTR_STS_POOL_FULL_STS_FIELD,
    &FPM_POOL1_INTR_STS_FREE_TOKEN_NO_VALID_STS_FIELD,
    &FPM_POOL1_INTR_STS_FREE_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD,
    &FPM_POOL1_INTR_STS_MULTI_TOKEN_NO_VALID_STS_FIELD,
    &FPM_POOL1_INTR_STS_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD,
    &FPM_POOL1_INTR_STS_POOL_DIS_FREE_MULTI_STS_FIELD,
    &FPM_POOL1_INTR_STS_MEMORY_CORRUPT_STS_FIELD,
    &FPM_POOL1_INTR_STS_XOFF_STATE_STS_FIELD,
    &FPM_POOL1_INTR_STS_XON_STATE_STS_FIELD,
    &FPM_POOL1_INTR_STS_ILLEGAL_ADDRESS_ACCESS_STS_FIELD,
    &FPM_POOL1_INTR_STS_ILLEGAL_ALLOC_REQUEST_STS_FIELD,
    &FPM_POOL1_INTR_STS_EXPIRED_TOKEN_DET_STS_FIELD,
    &FPM_POOL1_INTR_STS_EXPIRED_TOKEN_RECOV_STS_FIELD,
    &FPM_POOL1_INTR_STS_R1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_POOL1_INTR_STS *****/
const ru_reg_rec FPM_POOL1_INTR_STS_REG =
{
    "POOL1_INTR_STS",
#if RU_INCLUDE_DESC
    "POOL1_INTR_STS Register",
    "Interrupt bits are active high. When a bit in this register is set to 1 and the corresponding bit in interrupt mask register is set to 1, interrupt to CPU will occur. When set (1), interrupts bits can be cleared (0) by writing a 1 to the desired bit.\n\n",
#endif
    { FPM_POOL1_INTR_STS_REG_OFFSET },
    0,
    0,
    447,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    16,
    FPM_POOL1_INTR_STS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_POOL1_STALL_MSK, TYPE: Type_FPM_BLOCK_FPM_CTRL_POOL1_STALL_MSK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: R3 *****/
const ru_field_rec FPM_POOL1_STALL_MSK_R3_FIELD =
{
    "R3",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_POOL1_STALL_MSK_R3_FIELD_MASK },
    0,
    { FPM_POOL1_STALL_MSK_R3_FIELD_WIDTH },
    { FPM_POOL1_STALL_MSK_R3_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_TOKEN_NO_VALID_STALL_MSK *****/
const ru_field_rec FPM_POOL1_STALL_MSK_FREE_TOKEN_NO_VALID_STALL_MSK_FIELD =
{
    "FREE_TOKEN_NO_VALID_STALL_MSK",
#if RU_INCLUDE_DESC
    "",
    "Stall FPM on De-allocation token request with invalid token interrupt status.\n\n",
#endif
    { FPM_POOL1_STALL_MSK_FREE_TOKEN_NO_VALID_STALL_MSK_FIELD_MASK },
    0,
    { FPM_POOL1_STALL_MSK_FREE_TOKEN_NO_VALID_STALL_MSK_FIELD_WIDTH },
    { FPM_POOL1_STALL_MSK_FREE_TOKEN_NO_VALID_STALL_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK *****/
const ru_field_rec FPM_POOL1_STALL_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD =
{
    "FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK",
#if RU_INCLUDE_DESC
    "",
    "Stall FPM on De-allocation token request with index out-of-range interrupt status.\n\n",
#endif
    { FPM_POOL1_STALL_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD_MASK },
    0,
    { FPM_POOL1_STALL_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD_WIDTH },
    { FPM_POOL1_STALL_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MULTI_TOKEN_NO_VALID_STALL_MSK *****/
const ru_field_rec FPM_POOL1_STALL_MSK_MULTI_TOKEN_NO_VALID_STALL_MSK_FIELD =
{
    "MULTI_TOKEN_NO_VALID_STALL_MSK",
#if RU_INCLUDE_DESC
    "",
    "Stall FPM on Token multi-cast value update request with invalid token interrupt status.\n\n",
#endif
    { FPM_POOL1_STALL_MSK_MULTI_TOKEN_NO_VALID_STALL_MSK_FIELD_MASK },
    0,
    { FPM_POOL1_STALL_MSK_MULTI_TOKEN_NO_VALID_STALL_MSK_FIELD_WIDTH },
    { FPM_POOL1_STALL_MSK_MULTI_TOKEN_NO_VALID_STALL_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK *****/
const ru_field_rec FPM_POOL1_STALL_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD =
{
    "MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK",
#if RU_INCLUDE_DESC
    "",
    "Stall FPM on Token multi-cast value update request with index out-of-range interrupt status.\n\n",
#endif
    { FPM_POOL1_STALL_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD_MASK },
    0,
    { FPM_POOL1_STALL_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD_WIDTH },
    { FPM_POOL1_STALL_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R2 *****/
const ru_field_rec FPM_POOL1_STALL_MSK_R2_FIELD =
{
    "R2",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_POOL1_STALL_MSK_R2_FIELD_MASK },
    0,
    { FPM_POOL1_STALL_MSK_R2_FIELD_WIDTH },
    { FPM_POOL1_STALL_MSK_R2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MEMORY_CORRUPT_STALL_MSK *****/
const ru_field_rec FPM_POOL1_STALL_MSK_MEMORY_CORRUPT_STALL_MSK_FIELD =
{
    "MEMORY_CORRUPT_STALL_MSK",
#if RU_INCLUDE_DESC
    "",
    "Stall FPM on Index Memory corrupt interrupt status.\n\n",
#endif
    { FPM_POOL1_STALL_MSK_MEMORY_CORRUPT_STALL_MSK_FIELD_MASK },
    0,
    { FPM_POOL1_STALL_MSK_MEMORY_CORRUPT_STALL_MSK_FIELD_WIDTH },
    { FPM_POOL1_STALL_MSK_MEMORY_CORRUPT_STALL_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec FPM_POOL1_STALL_MSK_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_POOL1_STALL_MSK_R1_FIELD_MASK },
    0,
    { FPM_POOL1_STALL_MSK_R1_FIELD_WIDTH },
    { FPM_POOL1_STALL_MSK_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL1_STALL_MSK_FIELDS[] =
{
    &FPM_POOL1_STALL_MSK_R3_FIELD,
    &FPM_POOL1_STALL_MSK_FREE_TOKEN_NO_VALID_STALL_MSK_FIELD,
    &FPM_POOL1_STALL_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD,
    &FPM_POOL1_STALL_MSK_MULTI_TOKEN_NO_VALID_STALL_MSK_FIELD,
    &FPM_POOL1_STALL_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD,
    &FPM_POOL1_STALL_MSK_R2_FIELD,
    &FPM_POOL1_STALL_MSK_MEMORY_CORRUPT_STALL_MSK_FIELD,
    &FPM_POOL1_STALL_MSK_R1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_POOL1_STALL_MSK *****/
const ru_reg_rec FPM_POOL1_STALL_MSK_REG =
{
    "POOL1_STALL_MSK",
#if RU_INCLUDE_DESC
    "POOL1_STALL_MSK Register",
    "Software sets desired stall bits that upon corresponding active interrupt status will stall FPM from new allocation, de-allocation, and mcast update process. Listed below are the supported interrupt statuses\n1. Invalid free token (bit[3] of interrupt status register 0x14)\n2. Invalid free token with index out-of-range (bit[4] of interrupt status register 0x14)\n3. Invalid mcast token (bit[5] of interrupt status register 0x14)\n4. Invalid mcast token with index out-of-range (bit[6] of interrupt status register 0x14)\n5. Memory corrupt status (bit[8] of interrupt status register 0x14)\nWhen state machine is stalled, registers and memory can still be accessed. Any new token allocation request will be serviced with valid tokens (if available in alloc cache) and invalid tokens (if alloc cache is empty). Any new de-allocation/mcast update requests will be either stored in de-allocation fifo (if there is space in free fifo) or dropped (if free fifo is full). Bit locations in this register matches the location of corrseponding interrupt status bits in register 0x14. To un-stall (enable) state machine interrupt status bits (in register 0x14) corresponding to these mask bits should be cleared. Stall mask bits are active high and are disabled by default. This is for debug purposes only.\n\n",
#endif
    { FPM_POOL1_STALL_MSK_REG_OFFSET },
    0,
    0,
    448,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    FPM_POOL1_STALL_MSK_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_POOL2_INTR_MSK, TYPE: Type_FPM_BLOCK_FPM_CTRL_POOL2_INTR_MSK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ALLOC_FIFO_FULL_MSK *****/
const ru_field_rec FPM_POOL2_INTR_MSK_ALLOC_FIFO_FULL_MSK_FIELD =
{
    "ALLOC_FIFO_FULL_MSK",
#if RU_INCLUDE_DESC
    "",
    "Allocation FIFO Full Interrupt mask.\n\n",
#endif
    { FPM_POOL2_INTR_MSK_ALLOC_FIFO_FULL_MSK_FIELD_MASK },
    0,
    { FPM_POOL2_INTR_MSK_ALLOC_FIFO_FULL_MSK_FIELD_WIDTH },
    { FPM_POOL2_INTR_MSK_ALLOC_FIFO_FULL_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_FIFO_FULL_MSK *****/
const ru_field_rec FPM_POOL2_INTR_MSK_FREE_FIFO_FULL_MSK_FIELD =
{
    "FREE_FIFO_FULL_MSK",
#if RU_INCLUDE_DESC
    "",
    "De-Allocation FIFO Full Interrupt mask.\n\n",
#endif
    { FPM_POOL2_INTR_MSK_FREE_FIFO_FULL_MSK_FIELD_MASK },
    0,
    { FPM_POOL2_INTR_MSK_FREE_FIFO_FULL_MSK_FIELD_WIDTH },
    { FPM_POOL2_INTR_MSK_FREE_FIFO_FULL_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_FULL_MSK *****/
const ru_field_rec FPM_POOL2_INTR_MSK_POOL_FULL_MSK_FIELD =
{
    "POOL_FULL_MSK",
#if RU_INCLUDE_DESC
    "",
    "Usage Index Pool is fully allocated interrupt mask.\n\n",
#endif
    { FPM_POOL2_INTR_MSK_POOL_FULL_MSK_FIELD_MASK },
    0,
    { FPM_POOL2_INTR_MSK_POOL_FULL_MSK_FIELD_WIDTH },
    { FPM_POOL2_INTR_MSK_POOL_FULL_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_TOKEN_NO_VALID_MSK *****/
const ru_field_rec FPM_POOL2_INTR_MSK_FREE_TOKEN_NO_VALID_MSK_FIELD =
{
    "FREE_TOKEN_NO_VALID_MSK",
#if RU_INCLUDE_DESC
    "",
    "De-allocation token request with invalid token.\n\n",
#endif
    { FPM_POOL2_INTR_MSK_FREE_TOKEN_NO_VALID_MSK_FIELD_MASK },
    0,
    { FPM_POOL2_INTR_MSK_FREE_TOKEN_NO_VALID_MSK_FIELD_WIDTH },
    { FPM_POOL2_INTR_MSK_FREE_TOKEN_NO_VALID_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK *****/
const ru_field_rec FPM_POOL2_INTR_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD =
{
    "FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK",
#if RU_INCLUDE_DESC
    "",
    "De-allocation token request with index out-of-range.\n\n",
#endif
    { FPM_POOL2_INTR_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD_MASK },
    0,
    { FPM_POOL2_INTR_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD_WIDTH },
    { FPM_POOL2_INTR_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MULTI_TOKEN_NO_VALID_MSK *****/
const ru_field_rec FPM_POOL2_INTR_MSK_MULTI_TOKEN_NO_VALID_MSK_FIELD =
{
    "MULTI_TOKEN_NO_VALID_MSK",
#if RU_INCLUDE_DESC
    "",
    "Token multi-cast value update request with invalid token.\n\n",
#endif
    { FPM_POOL2_INTR_MSK_MULTI_TOKEN_NO_VALID_MSK_FIELD_MASK },
    0,
    { FPM_POOL2_INTR_MSK_MULTI_TOKEN_NO_VALID_MSK_FIELD_WIDTH },
    { FPM_POOL2_INTR_MSK_MULTI_TOKEN_NO_VALID_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK *****/
const ru_field_rec FPM_POOL2_INTR_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD =
{
    "MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK",
#if RU_INCLUDE_DESC
    "",
    "Token multi-cast value update request with index out-of-range.\n\n",
#endif
    { FPM_POOL2_INTR_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD_MASK },
    0,
    { FPM_POOL2_INTR_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD_WIDTH },
    { FPM_POOL2_INTR_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_DIS_FREE_MULTI_MSK *****/
const ru_field_rec FPM_POOL2_INTR_MSK_POOL_DIS_FREE_MULTI_MSK_FIELD =
{
    "POOL_DIS_FREE_MULTI_MSK",
#if RU_INCLUDE_DESC
    "",
    "Free or Mcast update on disabled pool interrupt mask .\n\n",
#endif
    { FPM_POOL2_INTR_MSK_POOL_DIS_FREE_MULTI_MSK_FIELD_MASK },
    0,
    { FPM_POOL2_INTR_MSK_POOL_DIS_FREE_MULTI_MSK_FIELD_WIDTH },
    { FPM_POOL2_INTR_MSK_POOL_DIS_FREE_MULTI_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MEMORY_CORRUPT_MSK *****/
const ru_field_rec FPM_POOL2_INTR_MSK_MEMORY_CORRUPT_MSK_FIELD =
{
    "MEMORY_CORRUPT_MSK",
#if RU_INCLUDE_DESC
    "",
    "Index Memory corrupt interrupt mask.\n\n",
#endif
    { FPM_POOL2_INTR_MSK_MEMORY_CORRUPT_MSK_FIELD_MASK },
    0,
    { FPM_POOL2_INTR_MSK_MEMORY_CORRUPT_MSK_FIELD_WIDTH },
    { FPM_POOL2_INTR_MSK_MEMORY_CORRUPT_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: XOFF_MSK *****/
const ru_field_rec FPM_POOL2_INTR_MSK_XOFF_MSK_FIELD =
{
    "XOFF_MSK",
#if RU_INCLUDE_DESC
    "",
    "XOFF_STATE interrupt mask.\n\n",
#endif
    { FPM_POOL2_INTR_MSK_XOFF_MSK_FIELD_MASK },
    0,
    { FPM_POOL2_INTR_MSK_XOFF_MSK_FIELD_WIDTH },
    { FPM_POOL2_INTR_MSK_XOFF_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: XON_MSK *****/
const ru_field_rec FPM_POOL2_INTR_MSK_XON_MSK_FIELD =
{
    "XON_MSK",
#if RU_INCLUDE_DESC
    "",
    "XON_STATE interrupt mask.\n\n",
#endif
    { FPM_POOL2_INTR_MSK_XON_MSK_FIELD_MASK },
    0,
    { FPM_POOL2_INTR_MSK_XON_MSK_FIELD_WIDTH },
    { FPM_POOL2_INTR_MSK_XON_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ILLEGAL_ADDRESS_ACCESS_MSK *****/
const ru_field_rec FPM_POOL2_INTR_MSK_ILLEGAL_ADDRESS_ACCESS_MSK_FIELD =
{
    "ILLEGAL_ADDRESS_ACCESS_MSK",
#if RU_INCLUDE_DESC
    "",
    "Illegal/un-implemented register/memory space access  interrupt mask.\n\n",
#endif
    { FPM_POOL2_INTR_MSK_ILLEGAL_ADDRESS_ACCESS_MSK_FIELD_MASK },
    0,
    { FPM_POOL2_INTR_MSK_ILLEGAL_ADDRESS_ACCESS_MSK_FIELD_WIDTH },
    { FPM_POOL2_INTR_MSK_ILLEGAL_ADDRESS_ACCESS_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ILLEGAL_ALLOC_REQUEST_MSK *****/
const ru_field_rec FPM_POOL2_INTR_MSK_ILLEGAL_ALLOC_REQUEST_MSK_FIELD =
{
    "ILLEGAL_ALLOC_REQUEST_MSK",
#if RU_INCLUDE_DESC
    "",
    "Illegal token request interrupt mask.\n\n",
#endif
    { FPM_POOL2_INTR_MSK_ILLEGAL_ALLOC_REQUEST_MSK_FIELD_MASK },
    0,
    { FPM_POOL2_INTR_MSK_ILLEGAL_ALLOC_REQUEST_MSK_FIELD_WIDTH },
    { FPM_POOL2_INTR_MSK_ILLEGAL_ALLOC_REQUEST_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EXPIRED_TOKEN_DET_MSK *****/
const ru_field_rec FPM_POOL2_INTR_MSK_EXPIRED_TOKEN_DET_MSK_FIELD =
{
    "EXPIRED_TOKEN_DET_MSK",
#if RU_INCLUDE_DESC
    "",
    "Expired token detect interrupt mask.\n\n",
#endif
    { FPM_POOL2_INTR_MSK_EXPIRED_TOKEN_DET_MSK_FIELD_MASK },
    0,
    { FPM_POOL2_INTR_MSK_EXPIRED_TOKEN_DET_MSK_FIELD_WIDTH },
    { FPM_POOL2_INTR_MSK_EXPIRED_TOKEN_DET_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EXPIRED_TOKEN_RECOV_MSK *****/
const ru_field_rec FPM_POOL2_INTR_MSK_EXPIRED_TOKEN_RECOV_MSK_FIELD =
{
    "EXPIRED_TOKEN_RECOV_MSK",
#if RU_INCLUDE_DESC
    "",
    "Expired token recovered interrupt mask.\n\n",
#endif
    { FPM_POOL2_INTR_MSK_EXPIRED_TOKEN_RECOV_MSK_FIELD_MASK },
    0,
    { FPM_POOL2_INTR_MSK_EXPIRED_TOKEN_RECOV_MSK_FIELD_WIDTH },
    { FPM_POOL2_INTR_MSK_EXPIRED_TOKEN_RECOV_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec FPM_POOL2_INTR_MSK_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_POOL2_INTR_MSK_R1_FIELD_MASK },
    0,
    { FPM_POOL2_INTR_MSK_R1_FIELD_WIDTH },
    { FPM_POOL2_INTR_MSK_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL2_INTR_MSK_FIELDS[] =
{
    &FPM_POOL2_INTR_MSK_ALLOC_FIFO_FULL_MSK_FIELD,
    &FPM_POOL2_INTR_MSK_FREE_FIFO_FULL_MSK_FIELD,
    &FPM_POOL2_INTR_MSK_POOL_FULL_MSK_FIELD,
    &FPM_POOL2_INTR_MSK_FREE_TOKEN_NO_VALID_MSK_FIELD,
    &FPM_POOL2_INTR_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD,
    &FPM_POOL2_INTR_MSK_MULTI_TOKEN_NO_VALID_MSK_FIELD,
    &FPM_POOL2_INTR_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD,
    &FPM_POOL2_INTR_MSK_POOL_DIS_FREE_MULTI_MSK_FIELD,
    &FPM_POOL2_INTR_MSK_MEMORY_CORRUPT_MSK_FIELD,
    &FPM_POOL2_INTR_MSK_XOFF_MSK_FIELD,
    &FPM_POOL2_INTR_MSK_XON_MSK_FIELD,
    &FPM_POOL2_INTR_MSK_ILLEGAL_ADDRESS_ACCESS_MSK_FIELD,
    &FPM_POOL2_INTR_MSK_ILLEGAL_ALLOC_REQUEST_MSK_FIELD,
    &FPM_POOL2_INTR_MSK_EXPIRED_TOKEN_DET_MSK_FIELD,
    &FPM_POOL2_INTR_MSK_EXPIRED_TOKEN_RECOV_MSK_FIELD,
    &FPM_POOL2_INTR_MSK_R1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_POOL2_INTR_MSK *****/
const ru_reg_rec FPM_POOL2_INTR_MSK_REG =
{
    "POOL2_INTR_MSK",
#if RU_INCLUDE_DESC
    "POOL2_INTR_MSK Register",
    "Mask bits are active high and are disabled by default. Software enables desired bits as necessary\n\n",
#endif
    { FPM_POOL2_INTR_MSK_REG_OFFSET },
    0,
    0,
    449,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    16,
    FPM_POOL2_INTR_MSK_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_POOL2_INTR_STS, TYPE: Type_FPM_BLOCK_FPM_CTRL_POOL2_INTR_STS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ALLOC_FIFO_FULL_STS *****/
const ru_field_rec FPM_POOL2_INTR_STS_ALLOC_FIFO_FULL_STS_FIELD =
{
    "ALLOC_FIFO_FULL_STS",
#if RU_INCLUDE_DESC
    "",
    "Allocation FIFO Full Interrupt. This is a functional status bit, not an error status bit. This indicates that allocation FIFO is full with new tokens to be allocated and will be active (high) as long as FIFO is full. This status is intended to be used for debug purpose only.\n\n",
#endif
    { FPM_POOL2_INTR_STS_ALLOC_FIFO_FULL_STS_FIELD_MASK },
    0,
    { FPM_POOL2_INTR_STS_ALLOC_FIFO_FULL_STS_FIELD_WIDTH },
    { FPM_POOL2_INTR_STS_ALLOC_FIFO_FULL_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_FIFO_FULL_STS *****/
const ru_field_rec FPM_POOL2_INTR_STS_FREE_FIFO_FULL_STS_FIELD =
{
    "FREE_FIFO_FULL_STS",
#if RU_INCLUDE_DESC
    "",
    "De-Allocation FIFO Full Interrupt. This is a functional status bit, not an error status bit. This indicates that de-allocation FIFO is full with tokens needs to be freed and will be active (high) as long as FIFO is full. This status is intended to be used for debug purpose only.\n\n",
#endif
    { FPM_POOL2_INTR_STS_FREE_FIFO_FULL_STS_FIELD_MASK },
    0,
    { FPM_POOL2_INTR_STS_FREE_FIFO_FULL_STS_FIELD_WIDTH },
    { FPM_POOL2_INTR_STS_FREE_FIFO_FULL_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_FULL_STS *****/
const ru_field_rec FPM_POOL2_INTR_STS_POOL_FULL_STS_FIELD =
{
    "POOL_FULL_STS",
#if RU_INCLUDE_DESC
    "",
    "Usage Index Pool is fully allocated interrupt. This is a functional status bit, not an error status bit. This indicates that token pool is fully allocated and there are no free tokens available. This bit will be active (high) as long as there no free tokens available to allocate. This bit is intended to be used for debug purpose only.\n\n",
#endif
    { FPM_POOL2_INTR_STS_POOL_FULL_STS_FIELD_MASK },
    0,
    { FPM_POOL2_INTR_STS_POOL_FULL_STS_FIELD_WIDTH },
    { FPM_POOL2_INTR_STS_POOL_FULL_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_TOKEN_NO_VALID_STS *****/
const ru_field_rec FPM_POOL2_INTR_STS_FREE_TOKEN_NO_VALID_STS_FIELD =
{
    "FREE_TOKEN_NO_VALID_STS",
#if RU_INCLUDE_DESC
    "",
    "De-allocation token request with invalid token Interrupt.\nInvalid free token is determined when one or more the following conditions are met -\n1. Incoming multi-cast request token is not valid ( 0xffffffff )\n2. Incoming free request token entry in the usage array indicates it is not an allocated token, i.e., associated use count value for this count in the usage array is zero\nNote: item 2 is not checked if auto_pool_en is set. The auto_pool_en bit is always set when using the new token format without a pool select field.\n\n",
#endif
    { FPM_POOL2_INTR_STS_FREE_TOKEN_NO_VALID_STS_FIELD_MASK },
    0,
    { FPM_POOL2_INTR_STS_FREE_TOKEN_NO_VALID_STS_FIELD_WIDTH },
    { FPM_POOL2_INTR_STS_FREE_TOKEN_NO_VALID_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_TOKEN_INDEX_OUT_OF_RANGE_STS *****/
const ru_field_rec FPM_POOL2_INTR_STS_FREE_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD =
{
    "FREE_TOKEN_INDEX_OUT_OF_RANGE_STS",
#if RU_INCLUDE_DESC
    "",
    "Not applicable for BCM3391.  All token values are valid.\nDe-allocation token request with index out-of-range Interrupt.\nFree token index out of range is determined when one or more of the following conditions are met -\n1. Incoming free request token index is not aligned to the pool size indicated by the pool select field (bits[29:28])\n2. The buffer size indicated by the size field (bits[11:0]) is greater than the size of the allocated token.\nThere is no associated count for this error. Note: item 1 is not checked if auto_pool_en is set. The auto_pool_en bit is always set when using the new token format without a pool select field.\n\n",
#endif
    { FPM_POOL2_INTR_STS_FREE_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD_MASK },
    0,
    { FPM_POOL2_INTR_STS_FREE_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD_WIDTH },
    { FPM_POOL2_INTR_STS_FREE_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MULTI_TOKEN_NO_VALID_STS *****/
const ru_field_rec FPM_POOL2_INTR_STS_MULTI_TOKEN_NO_VALID_STS_FIELD =
{
    "MULTI_TOKEN_NO_VALID_STS",
#if RU_INCLUDE_DESC
    "",
    "Token multi-cast value update request with invalid token Interrupt.\nInvalid multi-cast token is determined when one or more the following conditions are met -\n1. Incoming multi-cast request token is not valid ( 0xffffffff )\n2. Incoming multi-cast request token has use count field set to zero\n3. Incoming multi-cast request token entry in the usage array indicates it is not an allocated token, i.e., associated use count value for this count in the usage array is zero\n4. After updating the use count value, the new use count value exceeds 0x7E\nNote: item 2 is not checked if auto_pool_en is set. The auto_pool_en bit is always set when using the new token format without a pool select field.\n\n",
#endif
    { FPM_POOL2_INTR_STS_MULTI_TOKEN_NO_VALID_STS_FIELD_MASK },
    0,
    { FPM_POOL2_INTR_STS_MULTI_TOKEN_NO_VALID_STS_FIELD_WIDTH },
    { FPM_POOL2_INTR_STS_MULTI_TOKEN_NO_VALID_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS *****/
const ru_field_rec FPM_POOL2_INTR_STS_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD =
{
    "MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS",
#if RU_INCLUDE_DESC
    "",
    "Token multi-cast value update request with index out-of-range Interrupt.\nThis is set when the token index is not aligned to the pool size. This is determined by examining the pool select field (bits[29:28]) and the 3 lsbs of the token index (bits[14:12]). There is no associated count for this error. Note: this error is not checked if auto_pool_en is set. The auto_pool_en bit is always set when using the new token format without a pool select field.\n\n",
#endif
    { FPM_POOL2_INTR_STS_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD_MASK },
    0,
    { FPM_POOL2_INTR_STS_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD_WIDTH },
    { FPM_POOL2_INTR_STS_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_DIS_FREE_MULTI_STS *****/
const ru_field_rec FPM_POOL2_INTR_STS_POOL_DIS_FREE_MULTI_STS_FIELD =
{
    "POOL_DIS_FREE_MULTI_STS",
#if RU_INCLUDE_DESC
    "",
    "Free or Mcast update on disabled pool interrupt.\nThis bit goes active when a free or multi-cast request is received and FPM is not enabled, i.e., pool enable bit in FPM control register is not set to 1'b1.\n\n",
#endif
    { FPM_POOL2_INTR_STS_POOL_DIS_FREE_MULTI_STS_FIELD_MASK },
    0,
    { FPM_POOL2_INTR_STS_POOL_DIS_FREE_MULTI_STS_FIELD_WIDTH },
    { FPM_POOL2_INTR_STS_POOL_DIS_FREE_MULTI_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MEMORY_CORRUPT_STS *****/
const ru_field_rec FPM_POOL2_INTR_STS_MEMORY_CORRUPT_STS_FIELD =
{
    "MEMORY_CORRUPT_STS",
#if RU_INCLUDE_DESC
    "",
    "Index Memory is corrupted.\nDuring updates of the usage array, token manager checks if the use count and search tree value in the array has a legal value. If the use count or search tree value is not correct before updating, logic generates an error and interrupt. As long as the interrupt is active no more valid tokens will be allocated because this is a catastrophic error. Following are the two error conditions that are checked -\n1.    During search for a free token, a particular token use count value indicates it is allocated (use count is greater than 0), but corresponding upper level search tree value indicates the token is still available (with bit value of 1'b0, instead of 1'b1). This is an error.\n2.    During search for a free token, a particular token use count value indicates that it is free (use count is 0), but corresponding upper level search tree value indicates the token is not available (with bit value of 1'b1, instead of 1'b0). This is an error.\n\n",
#endif
    { FPM_POOL2_INTR_STS_MEMORY_CORRUPT_STS_FIELD_MASK },
    0,
    { FPM_POOL2_INTR_STS_MEMORY_CORRUPT_STS_FIELD_WIDTH },
    { FPM_POOL2_INTR_STS_MEMORY_CORRUPT_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: XOFF_STATE_STS *****/
const ru_field_rec FPM_POOL2_INTR_STS_XOFF_STATE_STS_FIELD =
{
    "XOFF_STATE_STS",
#if RU_INCLUDE_DESC
    "",
    "Number of available tokens is less than or equal to XOFF_THRESHOLD value in XON/XOFF Threshold configuration register. This is a functional status bit, not an error status bit. Using this information FPM generates backpressure output signal that is used by other UBUS client logics to throttle its operation. For example, UNIMAC logic can use backpressure signal to transfer PAUSE Ethernet flow control packets to throttle incoming frames on Ethernet interface.\n\n",
#endif
    { FPM_POOL2_INTR_STS_XOFF_STATE_STS_FIELD_MASK },
    0,
    { FPM_POOL2_INTR_STS_XOFF_STATE_STS_FIELD_WIDTH },
    { FPM_POOL2_INTR_STS_XOFF_STATE_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: XON_STATE_STS *****/
const ru_field_rec FPM_POOL2_INTR_STS_XON_STATE_STS_FIELD =
{
    "XON_STATE_STS",
#if RU_INCLUDE_DESC
    "",
    "Number of available tokens is greater than or equal to XON_THRESHOLD value in XON/XOFF Threshold configuration register. This is a functional status bit, not an error status bit. Using this information FPM generates backpressure output signal that is used by other UBUS client logics to throttle its operation. For example, UNIMAC logic can use backpressure signal to transfer PAUSE Ethernet flow control packets to throttle incoming frames on Ethernet interface.\n\n",
#endif
    { FPM_POOL2_INTR_STS_XON_STATE_STS_FIELD_MASK },
    0,
    { FPM_POOL2_INTR_STS_XON_STATE_STS_FIELD_WIDTH },
    { FPM_POOL2_INTR_STS_XON_STATE_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ILLEGAL_ADDRESS_ACCESS_STS *****/
const ru_field_rec FPM_POOL2_INTR_STS_ILLEGAL_ADDRESS_ACCESS_STS_FIELD =
{
    "ILLEGAL_ADDRESS_ACCESS_STS",
#if RU_INCLUDE_DESC
    "",
    "Illegal/un-implemented register/memory space access interrupt. This will be active when there is an attempt to read from an unimplemented register or memory space. Along with interrupt being sent an error reply packet will be sent out with o_ubus_error_out asserted.\n\n",
#endif
    { FPM_POOL2_INTR_STS_ILLEGAL_ADDRESS_ACCESS_STS_FIELD_MASK },
    0,
    { FPM_POOL2_INTR_STS_ILLEGAL_ADDRESS_ACCESS_STS_FIELD_WIDTH },
    { FPM_POOL2_INTR_STS_ILLEGAL_ADDRESS_ACCESS_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ILLEGAL_ALLOC_REQUEST_STS *****/
const ru_field_rec FPM_POOL2_INTR_STS_ILLEGAL_ALLOC_REQUEST_STS_FIELD =
{
    "ILLEGAL_ALLOC_REQUEST_STS",
#if RU_INCLUDE_DESC
    "",
    "Illegal token request interrupt. This will be active when the pool is disabled, there is a request for a new token and the alloc fifo for the selected token size is empty. Along with interrupt being sent an error reply packet will be sent out with o_ubus_error_out asserted.\n\n",
#endif
    { FPM_POOL2_INTR_STS_ILLEGAL_ALLOC_REQUEST_STS_FIELD_MASK },
    0,
    { FPM_POOL2_INTR_STS_ILLEGAL_ALLOC_REQUEST_STS_FIELD_WIDTH },
    { FPM_POOL2_INTR_STS_ILLEGAL_ALLOC_REQUEST_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EXPIRED_TOKEN_DET_STS *****/
const ru_field_rec FPM_POOL2_INTR_STS_EXPIRED_TOKEN_DET_STS_FIELD =
{
    "EXPIRED_TOKEN_DET_STS",
#if RU_INCLUDE_DESC
    "",
    "Expired token detect interrupt. This is set when the token recovery logic detects a token that has been held for the entire duration of the aging timer.\n\n",
#endif
    { FPM_POOL2_INTR_STS_EXPIRED_TOKEN_DET_STS_FIELD_MASK },
    0,
    { FPM_POOL2_INTR_STS_EXPIRED_TOKEN_DET_STS_FIELD_WIDTH },
    { FPM_POOL2_INTR_STS_EXPIRED_TOKEN_DET_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EXPIRED_TOKEN_RECOV_STS *****/
const ru_field_rec FPM_POOL2_INTR_STS_EXPIRED_TOKEN_RECOV_STS_FIELD =
{
    "EXPIRED_TOKEN_RECOV_STS",
#if RU_INCLUDE_DESC
    "",
    "Expired token recovered interrupt. This is set when an expired token has been recoveredand returned to pool as an available token.\n\n",
#endif
    { FPM_POOL2_INTR_STS_EXPIRED_TOKEN_RECOV_STS_FIELD_MASK },
    0,
    { FPM_POOL2_INTR_STS_EXPIRED_TOKEN_RECOV_STS_FIELD_WIDTH },
    { FPM_POOL2_INTR_STS_EXPIRED_TOKEN_RECOV_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec FPM_POOL2_INTR_STS_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_POOL2_INTR_STS_R1_FIELD_MASK },
    0,
    { FPM_POOL2_INTR_STS_R1_FIELD_WIDTH },
    { FPM_POOL2_INTR_STS_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL2_INTR_STS_FIELDS[] =
{
    &FPM_POOL2_INTR_STS_ALLOC_FIFO_FULL_STS_FIELD,
    &FPM_POOL2_INTR_STS_FREE_FIFO_FULL_STS_FIELD,
    &FPM_POOL2_INTR_STS_POOL_FULL_STS_FIELD,
    &FPM_POOL2_INTR_STS_FREE_TOKEN_NO_VALID_STS_FIELD,
    &FPM_POOL2_INTR_STS_FREE_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD,
    &FPM_POOL2_INTR_STS_MULTI_TOKEN_NO_VALID_STS_FIELD,
    &FPM_POOL2_INTR_STS_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD,
    &FPM_POOL2_INTR_STS_POOL_DIS_FREE_MULTI_STS_FIELD,
    &FPM_POOL2_INTR_STS_MEMORY_CORRUPT_STS_FIELD,
    &FPM_POOL2_INTR_STS_XOFF_STATE_STS_FIELD,
    &FPM_POOL2_INTR_STS_XON_STATE_STS_FIELD,
    &FPM_POOL2_INTR_STS_ILLEGAL_ADDRESS_ACCESS_STS_FIELD,
    &FPM_POOL2_INTR_STS_ILLEGAL_ALLOC_REQUEST_STS_FIELD,
    &FPM_POOL2_INTR_STS_EXPIRED_TOKEN_DET_STS_FIELD,
    &FPM_POOL2_INTR_STS_EXPIRED_TOKEN_RECOV_STS_FIELD,
    &FPM_POOL2_INTR_STS_R1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_POOL2_INTR_STS *****/
const ru_reg_rec FPM_POOL2_INTR_STS_REG =
{
    "POOL2_INTR_STS",
#if RU_INCLUDE_DESC
    "POOL2_INTR_STS Register",
    "Interrupt bits are active high. When a bit in this register is set to 1 and the corresponding bit in interrupt mask register is set to 1, interrupt to CPU will occur. When set (1), interrupts bits can be cleared (0) by writing a 1 to the desired bit.\n\n",
#endif
    { FPM_POOL2_INTR_STS_REG_OFFSET },
    0,
    0,
    450,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    16,
    FPM_POOL2_INTR_STS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_POOL2_STALL_MSK, TYPE: Type_FPM_BLOCK_FPM_CTRL_POOL2_STALL_MSK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: R3 *****/
const ru_field_rec FPM_POOL2_STALL_MSK_R3_FIELD =
{
    "R3",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_POOL2_STALL_MSK_R3_FIELD_MASK },
    0,
    { FPM_POOL2_STALL_MSK_R3_FIELD_WIDTH },
    { FPM_POOL2_STALL_MSK_R3_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_TOKEN_NO_VALID_STALL_MSK *****/
const ru_field_rec FPM_POOL2_STALL_MSK_FREE_TOKEN_NO_VALID_STALL_MSK_FIELD =
{
    "FREE_TOKEN_NO_VALID_STALL_MSK",
#if RU_INCLUDE_DESC
    "",
    "Stall FPM on De-allocation token request with invalid token interrupt status.\n\n",
#endif
    { FPM_POOL2_STALL_MSK_FREE_TOKEN_NO_VALID_STALL_MSK_FIELD_MASK },
    0,
    { FPM_POOL2_STALL_MSK_FREE_TOKEN_NO_VALID_STALL_MSK_FIELD_WIDTH },
    { FPM_POOL2_STALL_MSK_FREE_TOKEN_NO_VALID_STALL_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK *****/
const ru_field_rec FPM_POOL2_STALL_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD =
{
    "FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK",
#if RU_INCLUDE_DESC
    "",
    "Stall FPM on De-allocation token request with index out-of-range interrupt status.\n\n",
#endif
    { FPM_POOL2_STALL_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD_MASK },
    0,
    { FPM_POOL2_STALL_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD_WIDTH },
    { FPM_POOL2_STALL_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MULTI_TOKEN_NO_VALID_STALL_MSK *****/
const ru_field_rec FPM_POOL2_STALL_MSK_MULTI_TOKEN_NO_VALID_STALL_MSK_FIELD =
{
    "MULTI_TOKEN_NO_VALID_STALL_MSK",
#if RU_INCLUDE_DESC
    "",
    "Stall FPM on Token multi-cast value update request with invalid token interrupt status.\n\n",
#endif
    { FPM_POOL2_STALL_MSK_MULTI_TOKEN_NO_VALID_STALL_MSK_FIELD_MASK },
    0,
    { FPM_POOL2_STALL_MSK_MULTI_TOKEN_NO_VALID_STALL_MSK_FIELD_WIDTH },
    { FPM_POOL2_STALL_MSK_MULTI_TOKEN_NO_VALID_STALL_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK *****/
const ru_field_rec FPM_POOL2_STALL_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD =
{
    "MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK",
#if RU_INCLUDE_DESC
    "",
    "Stall FPM on Token multi-cast value update request with index out-of-range interrupt status.\n\n",
#endif
    { FPM_POOL2_STALL_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD_MASK },
    0,
    { FPM_POOL2_STALL_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD_WIDTH },
    { FPM_POOL2_STALL_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R2 *****/
const ru_field_rec FPM_POOL2_STALL_MSK_R2_FIELD =
{
    "R2",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_POOL2_STALL_MSK_R2_FIELD_MASK },
    0,
    { FPM_POOL2_STALL_MSK_R2_FIELD_WIDTH },
    { FPM_POOL2_STALL_MSK_R2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MEMORY_CORRUPT_STALL_MSK *****/
const ru_field_rec FPM_POOL2_STALL_MSK_MEMORY_CORRUPT_STALL_MSK_FIELD =
{
    "MEMORY_CORRUPT_STALL_MSK",
#if RU_INCLUDE_DESC
    "",
    "Stall FPM on Index Memory corrupt interrupt status.\n\n",
#endif
    { FPM_POOL2_STALL_MSK_MEMORY_CORRUPT_STALL_MSK_FIELD_MASK },
    0,
    { FPM_POOL2_STALL_MSK_MEMORY_CORRUPT_STALL_MSK_FIELD_WIDTH },
    { FPM_POOL2_STALL_MSK_MEMORY_CORRUPT_STALL_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec FPM_POOL2_STALL_MSK_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_POOL2_STALL_MSK_R1_FIELD_MASK },
    0,
    { FPM_POOL2_STALL_MSK_R1_FIELD_WIDTH },
    { FPM_POOL2_STALL_MSK_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL2_STALL_MSK_FIELDS[] =
{
    &FPM_POOL2_STALL_MSK_R3_FIELD,
    &FPM_POOL2_STALL_MSK_FREE_TOKEN_NO_VALID_STALL_MSK_FIELD,
    &FPM_POOL2_STALL_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD,
    &FPM_POOL2_STALL_MSK_MULTI_TOKEN_NO_VALID_STALL_MSK_FIELD,
    &FPM_POOL2_STALL_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD,
    &FPM_POOL2_STALL_MSK_R2_FIELD,
    &FPM_POOL2_STALL_MSK_MEMORY_CORRUPT_STALL_MSK_FIELD,
    &FPM_POOL2_STALL_MSK_R1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_POOL2_STALL_MSK *****/
const ru_reg_rec FPM_POOL2_STALL_MSK_REG =
{
    "POOL2_STALL_MSK",
#if RU_INCLUDE_DESC
    "POOL2_STALL_MSK Register",
    "Software sets desired stall bits that upon corresponding active interrupt status will stall FPM from new allocation, de-allocation, and mcast update process. Listed below are the supported interrupt statuses\n1. Invalid free token (bit[3] of interrupt status register 0x14)\n2. Invalid free token with index out-of-range (bit[4] of interrupt status register 0x14)\n3. Invalid mcast token (bit[5] of interrupt status register 0x14)\n4. Invalid mcast token with index out-of-range (bit[6] of interrupt status register 0x14)\n5. Memory corrupt status (bit[8] of interrupt status register 0x14)\nWhen state machine is stalled, registers and memory can still be accessed. Any new token allocation request will be serviced with valid tokens (if available in alloc cache) and invalid tokens (if alloc cache is empty). Any new de-allocation/mcast update requests will be either stored in de-allocation fifo (if there is space in free fifo) or dropped (if free fifo is full). Bit locations in this register matches the location of corrseponding interrupt status bits in register 0x14. To un-stall (enable) state machine interrupt status bits (in register 0x14) corresponding to these mask bits should be cleared. Stall mask bits are active high and are disabled by default. This is for debug purposes only.\n\n",
#endif
    { FPM_POOL2_STALL_MSK_REG_OFFSET },
    0,
    0,
    451,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    FPM_POOL2_STALL_MSK_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_POOL3_INTR_MSK, TYPE: Type_FPM_BLOCK_FPM_CTRL_POOL3_INTR_MSK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ALLOC_FIFO_FULL_MSK *****/
const ru_field_rec FPM_POOL3_INTR_MSK_ALLOC_FIFO_FULL_MSK_FIELD =
{
    "ALLOC_FIFO_FULL_MSK",
#if RU_INCLUDE_DESC
    "",
    "Allocation FIFO Full Interrupt mask.\n\n",
#endif
    { FPM_POOL3_INTR_MSK_ALLOC_FIFO_FULL_MSK_FIELD_MASK },
    0,
    { FPM_POOL3_INTR_MSK_ALLOC_FIFO_FULL_MSK_FIELD_WIDTH },
    { FPM_POOL3_INTR_MSK_ALLOC_FIFO_FULL_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_FIFO_FULL_MSK *****/
const ru_field_rec FPM_POOL3_INTR_MSK_FREE_FIFO_FULL_MSK_FIELD =
{
    "FREE_FIFO_FULL_MSK",
#if RU_INCLUDE_DESC
    "",
    "De-Allocation FIFO Full Interrupt mask.\n\n",
#endif
    { FPM_POOL3_INTR_MSK_FREE_FIFO_FULL_MSK_FIELD_MASK },
    0,
    { FPM_POOL3_INTR_MSK_FREE_FIFO_FULL_MSK_FIELD_WIDTH },
    { FPM_POOL3_INTR_MSK_FREE_FIFO_FULL_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_FULL_MSK *****/
const ru_field_rec FPM_POOL3_INTR_MSK_POOL_FULL_MSK_FIELD =
{
    "POOL_FULL_MSK",
#if RU_INCLUDE_DESC
    "",
    "Usage Index Pool is fully allocated interrupt mask.\n\n",
#endif
    { FPM_POOL3_INTR_MSK_POOL_FULL_MSK_FIELD_MASK },
    0,
    { FPM_POOL3_INTR_MSK_POOL_FULL_MSK_FIELD_WIDTH },
    { FPM_POOL3_INTR_MSK_POOL_FULL_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_TOKEN_NO_VALID_MSK *****/
const ru_field_rec FPM_POOL3_INTR_MSK_FREE_TOKEN_NO_VALID_MSK_FIELD =
{
    "FREE_TOKEN_NO_VALID_MSK",
#if RU_INCLUDE_DESC
    "",
    "De-allocation token request with invalid token.\n\n",
#endif
    { FPM_POOL3_INTR_MSK_FREE_TOKEN_NO_VALID_MSK_FIELD_MASK },
    0,
    { FPM_POOL3_INTR_MSK_FREE_TOKEN_NO_VALID_MSK_FIELD_WIDTH },
    { FPM_POOL3_INTR_MSK_FREE_TOKEN_NO_VALID_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK *****/
const ru_field_rec FPM_POOL3_INTR_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD =
{
    "FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK",
#if RU_INCLUDE_DESC
    "",
    "De-allocation token request with index out-of-range.\n\n",
#endif
    { FPM_POOL3_INTR_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD_MASK },
    0,
    { FPM_POOL3_INTR_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD_WIDTH },
    { FPM_POOL3_INTR_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MULTI_TOKEN_NO_VALID_MSK *****/
const ru_field_rec FPM_POOL3_INTR_MSK_MULTI_TOKEN_NO_VALID_MSK_FIELD =
{
    "MULTI_TOKEN_NO_VALID_MSK",
#if RU_INCLUDE_DESC
    "",
    "Token multi-cast value update request with invalid token.\n\n",
#endif
    { FPM_POOL3_INTR_MSK_MULTI_TOKEN_NO_VALID_MSK_FIELD_MASK },
    0,
    { FPM_POOL3_INTR_MSK_MULTI_TOKEN_NO_VALID_MSK_FIELD_WIDTH },
    { FPM_POOL3_INTR_MSK_MULTI_TOKEN_NO_VALID_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK *****/
const ru_field_rec FPM_POOL3_INTR_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD =
{
    "MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK",
#if RU_INCLUDE_DESC
    "",
    "Token multi-cast value update request with index out-of-range.\n\n",
#endif
    { FPM_POOL3_INTR_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD_MASK },
    0,
    { FPM_POOL3_INTR_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD_WIDTH },
    { FPM_POOL3_INTR_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_DIS_FREE_MULTI_MSK *****/
const ru_field_rec FPM_POOL3_INTR_MSK_POOL_DIS_FREE_MULTI_MSK_FIELD =
{
    "POOL_DIS_FREE_MULTI_MSK",
#if RU_INCLUDE_DESC
    "",
    "Free or Mcast update on disabled pool interrupt mask .\n\n",
#endif
    { FPM_POOL3_INTR_MSK_POOL_DIS_FREE_MULTI_MSK_FIELD_MASK },
    0,
    { FPM_POOL3_INTR_MSK_POOL_DIS_FREE_MULTI_MSK_FIELD_WIDTH },
    { FPM_POOL3_INTR_MSK_POOL_DIS_FREE_MULTI_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MEMORY_CORRUPT_MSK *****/
const ru_field_rec FPM_POOL3_INTR_MSK_MEMORY_CORRUPT_MSK_FIELD =
{
    "MEMORY_CORRUPT_MSK",
#if RU_INCLUDE_DESC
    "",
    "Index Memory corrupt interrupt mask.\n\n",
#endif
    { FPM_POOL3_INTR_MSK_MEMORY_CORRUPT_MSK_FIELD_MASK },
    0,
    { FPM_POOL3_INTR_MSK_MEMORY_CORRUPT_MSK_FIELD_WIDTH },
    { FPM_POOL3_INTR_MSK_MEMORY_CORRUPT_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: XOFF_MSK *****/
const ru_field_rec FPM_POOL3_INTR_MSK_XOFF_MSK_FIELD =
{
    "XOFF_MSK",
#if RU_INCLUDE_DESC
    "",
    "XOFF_STATE interrupt mask.\n\n",
#endif
    { FPM_POOL3_INTR_MSK_XOFF_MSK_FIELD_MASK },
    0,
    { FPM_POOL3_INTR_MSK_XOFF_MSK_FIELD_WIDTH },
    { FPM_POOL3_INTR_MSK_XOFF_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: XON_MSK *****/
const ru_field_rec FPM_POOL3_INTR_MSK_XON_MSK_FIELD =
{
    "XON_MSK",
#if RU_INCLUDE_DESC
    "",
    "XON_STATE interrupt mask.\n\n",
#endif
    { FPM_POOL3_INTR_MSK_XON_MSK_FIELD_MASK },
    0,
    { FPM_POOL3_INTR_MSK_XON_MSK_FIELD_WIDTH },
    { FPM_POOL3_INTR_MSK_XON_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ILLEGAL_ADDRESS_ACCESS_MSK *****/
const ru_field_rec FPM_POOL3_INTR_MSK_ILLEGAL_ADDRESS_ACCESS_MSK_FIELD =
{
    "ILLEGAL_ADDRESS_ACCESS_MSK",
#if RU_INCLUDE_DESC
    "",
    "Illegal/un-implemented register/memory space access  interrupt mask.\n\n",
#endif
    { FPM_POOL3_INTR_MSK_ILLEGAL_ADDRESS_ACCESS_MSK_FIELD_MASK },
    0,
    { FPM_POOL3_INTR_MSK_ILLEGAL_ADDRESS_ACCESS_MSK_FIELD_WIDTH },
    { FPM_POOL3_INTR_MSK_ILLEGAL_ADDRESS_ACCESS_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ILLEGAL_ALLOC_REQUEST_MSK *****/
const ru_field_rec FPM_POOL3_INTR_MSK_ILLEGAL_ALLOC_REQUEST_MSK_FIELD =
{
    "ILLEGAL_ALLOC_REQUEST_MSK",
#if RU_INCLUDE_DESC
    "",
    "Illegal token request interrupt mask.\n\n",
#endif
    { FPM_POOL3_INTR_MSK_ILLEGAL_ALLOC_REQUEST_MSK_FIELD_MASK },
    0,
    { FPM_POOL3_INTR_MSK_ILLEGAL_ALLOC_REQUEST_MSK_FIELD_WIDTH },
    { FPM_POOL3_INTR_MSK_ILLEGAL_ALLOC_REQUEST_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EXPIRED_TOKEN_DET_MSK *****/
const ru_field_rec FPM_POOL3_INTR_MSK_EXPIRED_TOKEN_DET_MSK_FIELD =
{
    "EXPIRED_TOKEN_DET_MSK",
#if RU_INCLUDE_DESC
    "",
    "Expired token detect interrupt mask.\n\n",
#endif
    { FPM_POOL3_INTR_MSK_EXPIRED_TOKEN_DET_MSK_FIELD_MASK },
    0,
    { FPM_POOL3_INTR_MSK_EXPIRED_TOKEN_DET_MSK_FIELD_WIDTH },
    { FPM_POOL3_INTR_MSK_EXPIRED_TOKEN_DET_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EXPIRED_TOKEN_RECOV_MSK *****/
const ru_field_rec FPM_POOL3_INTR_MSK_EXPIRED_TOKEN_RECOV_MSK_FIELD =
{
    "EXPIRED_TOKEN_RECOV_MSK",
#if RU_INCLUDE_DESC
    "",
    "Expired token recovered interrupt mask.\n\n",
#endif
    { FPM_POOL3_INTR_MSK_EXPIRED_TOKEN_RECOV_MSK_FIELD_MASK },
    0,
    { FPM_POOL3_INTR_MSK_EXPIRED_TOKEN_RECOV_MSK_FIELD_WIDTH },
    { FPM_POOL3_INTR_MSK_EXPIRED_TOKEN_RECOV_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec FPM_POOL3_INTR_MSK_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_POOL3_INTR_MSK_R1_FIELD_MASK },
    0,
    { FPM_POOL3_INTR_MSK_R1_FIELD_WIDTH },
    { FPM_POOL3_INTR_MSK_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL3_INTR_MSK_FIELDS[] =
{
    &FPM_POOL3_INTR_MSK_ALLOC_FIFO_FULL_MSK_FIELD,
    &FPM_POOL3_INTR_MSK_FREE_FIFO_FULL_MSK_FIELD,
    &FPM_POOL3_INTR_MSK_POOL_FULL_MSK_FIELD,
    &FPM_POOL3_INTR_MSK_FREE_TOKEN_NO_VALID_MSK_FIELD,
    &FPM_POOL3_INTR_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD,
    &FPM_POOL3_INTR_MSK_MULTI_TOKEN_NO_VALID_MSK_FIELD,
    &FPM_POOL3_INTR_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD,
    &FPM_POOL3_INTR_MSK_POOL_DIS_FREE_MULTI_MSK_FIELD,
    &FPM_POOL3_INTR_MSK_MEMORY_CORRUPT_MSK_FIELD,
    &FPM_POOL3_INTR_MSK_XOFF_MSK_FIELD,
    &FPM_POOL3_INTR_MSK_XON_MSK_FIELD,
    &FPM_POOL3_INTR_MSK_ILLEGAL_ADDRESS_ACCESS_MSK_FIELD,
    &FPM_POOL3_INTR_MSK_ILLEGAL_ALLOC_REQUEST_MSK_FIELD,
    &FPM_POOL3_INTR_MSK_EXPIRED_TOKEN_DET_MSK_FIELD,
    &FPM_POOL3_INTR_MSK_EXPIRED_TOKEN_RECOV_MSK_FIELD,
    &FPM_POOL3_INTR_MSK_R1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_POOL3_INTR_MSK *****/
const ru_reg_rec FPM_POOL3_INTR_MSK_REG =
{
    "POOL3_INTR_MSK",
#if RU_INCLUDE_DESC
    "POOL3_INTR_MSK Register",
    "Mask bits are active high and are disabled by default. Software enables desired bits as necessary\n\n",
#endif
    { FPM_POOL3_INTR_MSK_REG_OFFSET },
    0,
    0,
    452,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    16,
    FPM_POOL3_INTR_MSK_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_POOL3_INTR_STS, TYPE: Type_FPM_BLOCK_FPM_CTRL_POOL3_INTR_STS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ALLOC_FIFO_FULL_STS *****/
const ru_field_rec FPM_POOL3_INTR_STS_ALLOC_FIFO_FULL_STS_FIELD =
{
    "ALLOC_FIFO_FULL_STS",
#if RU_INCLUDE_DESC
    "",
    "Allocation FIFO Full Interrupt. This is a functional status bit, not an error status bit. This indicates that allocation FIFO is full with new tokens to be allocated and will be active (high) as long as FIFO is full. This status is intended to be used for debug purpose only.\n\n",
#endif
    { FPM_POOL3_INTR_STS_ALLOC_FIFO_FULL_STS_FIELD_MASK },
    0,
    { FPM_POOL3_INTR_STS_ALLOC_FIFO_FULL_STS_FIELD_WIDTH },
    { FPM_POOL3_INTR_STS_ALLOC_FIFO_FULL_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_FIFO_FULL_STS *****/
const ru_field_rec FPM_POOL3_INTR_STS_FREE_FIFO_FULL_STS_FIELD =
{
    "FREE_FIFO_FULL_STS",
#if RU_INCLUDE_DESC
    "",
    "De-Allocation FIFO Full Interrupt. This is a functional status bit, not an error status bit. This indicates that de-allocation FIFO is full with tokens needs to be freed and will be active (high) as long as FIFO is full. This status is intended to be used for debug purpose only.\n\n",
#endif
    { FPM_POOL3_INTR_STS_FREE_FIFO_FULL_STS_FIELD_MASK },
    0,
    { FPM_POOL3_INTR_STS_FREE_FIFO_FULL_STS_FIELD_WIDTH },
    { FPM_POOL3_INTR_STS_FREE_FIFO_FULL_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_FULL_STS *****/
const ru_field_rec FPM_POOL3_INTR_STS_POOL_FULL_STS_FIELD =
{
    "POOL_FULL_STS",
#if RU_INCLUDE_DESC
    "",
    "Usage Index Pool is fully allocated interrupt. This is a functional status bit, not an error status bit. This indicates that token pool is fully allocated and there are no free tokens available. This bit will be active (high) as long as there no free tokens available to allocate. This bit is intended to be used for debug purpose only.\n\n",
#endif
    { FPM_POOL3_INTR_STS_POOL_FULL_STS_FIELD_MASK },
    0,
    { FPM_POOL3_INTR_STS_POOL_FULL_STS_FIELD_WIDTH },
    { FPM_POOL3_INTR_STS_POOL_FULL_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_TOKEN_NO_VALID_STS *****/
const ru_field_rec FPM_POOL3_INTR_STS_FREE_TOKEN_NO_VALID_STS_FIELD =
{
    "FREE_TOKEN_NO_VALID_STS",
#if RU_INCLUDE_DESC
    "",
    "De-allocation token request with invalid token Interrupt.\nInvalid free token is determined when one or more the following conditions are met -\n1. Incoming multi-cast request token is not valid ( 0xffffffff )\n2. Incoming free request token entry in the usage array indicates it is not an allocated token, i.e., associated use count value for this count in the usage array is zero\nNote: item 2 is not checked if auto_pool_en is set. The auto_pool_en bit is always set when using the new token format without a pool select field.\n\n",
#endif
    { FPM_POOL3_INTR_STS_FREE_TOKEN_NO_VALID_STS_FIELD_MASK },
    0,
    { FPM_POOL3_INTR_STS_FREE_TOKEN_NO_VALID_STS_FIELD_WIDTH },
    { FPM_POOL3_INTR_STS_FREE_TOKEN_NO_VALID_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_TOKEN_INDEX_OUT_OF_RANGE_STS *****/
const ru_field_rec FPM_POOL3_INTR_STS_FREE_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD =
{
    "FREE_TOKEN_INDEX_OUT_OF_RANGE_STS",
#if RU_INCLUDE_DESC
    "",
    "Not applicable for BCM3391.  All token values are valid.\nDe-allocation token request with index out-of-range Interrupt.\nFree token index out of range is determined when one or more of the following conditions are met -\n1. Incoming free request token index is not aligned to the pool size indicated by the pool select field (bits[29:28])\n2. The buffer size indicated by the size field (bits[11:0]) is greater than the size of the allocated token.\nThere is no associated count for this error. Note: item 1 is not checked if auto_pool_en is set. The auto_pool_en bit is always set when using the new token format without a pool select field.\n\n",
#endif
    { FPM_POOL3_INTR_STS_FREE_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD_MASK },
    0,
    { FPM_POOL3_INTR_STS_FREE_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD_WIDTH },
    { FPM_POOL3_INTR_STS_FREE_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MULTI_TOKEN_NO_VALID_STS *****/
const ru_field_rec FPM_POOL3_INTR_STS_MULTI_TOKEN_NO_VALID_STS_FIELD =
{
    "MULTI_TOKEN_NO_VALID_STS",
#if RU_INCLUDE_DESC
    "",
    "Token multi-cast value update request with invalid token Interrupt.\nInvalid multi-cast token is determined when one or more the following conditions are met -\n1. Incoming multi-cast request token is not valid ( 0xffffffff )\n2. Incoming multi-cast request token has use count field set to zero\n3. Incoming multi-cast request token entry in the usage array indicates it is not an allocated token, i.e., associated use count value for this count in the usage array is zero\n4. After updating the use count value, the new use count value exceeds 0x7E\nNote: item 2 is not checked if auto_pool_en is set. The auto_pool_en bit is always set when using the new token format without a pool select field.\n\n",
#endif
    { FPM_POOL3_INTR_STS_MULTI_TOKEN_NO_VALID_STS_FIELD_MASK },
    0,
    { FPM_POOL3_INTR_STS_MULTI_TOKEN_NO_VALID_STS_FIELD_WIDTH },
    { FPM_POOL3_INTR_STS_MULTI_TOKEN_NO_VALID_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS *****/
const ru_field_rec FPM_POOL3_INTR_STS_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD =
{
    "MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS",
#if RU_INCLUDE_DESC
    "",
    "Token multi-cast value update request with index out-of-range Interrupt.\nThis is set when the token index is not aligned to the pool size. This is determined by examining the pool select field (bits[29:28]) and the 3 lsbs of the token index (bits[14:12]). There is no associated count for this error. Note: this error is not checked if auto_pool_en is set. The auto_pool_en bit is always set when using the new token format without a pool select field.\n\n",
#endif
    { FPM_POOL3_INTR_STS_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD_MASK },
    0,
    { FPM_POOL3_INTR_STS_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD_WIDTH },
    { FPM_POOL3_INTR_STS_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_DIS_FREE_MULTI_STS *****/
const ru_field_rec FPM_POOL3_INTR_STS_POOL_DIS_FREE_MULTI_STS_FIELD =
{
    "POOL_DIS_FREE_MULTI_STS",
#if RU_INCLUDE_DESC
    "",
    "Free or Mcast update on disabled pool interrupt.\nThis bit goes active when a free or multi-cast request is received and FPM is not enabled, i.e., pool enable bit in FPM control register is not set to 1'b1.\n\n",
#endif
    { FPM_POOL3_INTR_STS_POOL_DIS_FREE_MULTI_STS_FIELD_MASK },
    0,
    { FPM_POOL3_INTR_STS_POOL_DIS_FREE_MULTI_STS_FIELD_WIDTH },
    { FPM_POOL3_INTR_STS_POOL_DIS_FREE_MULTI_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MEMORY_CORRUPT_STS *****/
const ru_field_rec FPM_POOL3_INTR_STS_MEMORY_CORRUPT_STS_FIELD =
{
    "MEMORY_CORRUPT_STS",
#if RU_INCLUDE_DESC
    "",
    "Index Memory is corrupted.\nDuring updates of the usage array, token manager checks if the use count and search tree value in the array has a legal value. If the use count or search tree value is not correct before updating, logic generates an error and interrupt. As long as the interrupt is active no more valid tokens will be allocated because this is a catastrophic error. Following are the two error conditions that are checked -\n1.    During search for a free token, a particular token use count value indicates it is allocated (use count is greater than 0), but corresponding upper level search tree value indicates the token is still available (with bit value of 1'b0, instead of 1'b1). This is an error.\n2.    During search for a free token, a particular token use count value indicates that it is free (use count is 0), but corresponding upper level search tree value indicates the token is not available (with bit value of 1'b1, instead of 1'b0). This is an error.\n\n",
#endif
    { FPM_POOL3_INTR_STS_MEMORY_CORRUPT_STS_FIELD_MASK },
    0,
    { FPM_POOL3_INTR_STS_MEMORY_CORRUPT_STS_FIELD_WIDTH },
    { FPM_POOL3_INTR_STS_MEMORY_CORRUPT_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: XOFF_STATE_STS *****/
const ru_field_rec FPM_POOL3_INTR_STS_XOFF_STATE_STS_FIELD =
{
    "XOFF_STATE_STS",
#if RU_INCLUDE_DESC
    "",
    "Number of available tokens is less than or equal to XOFF_THRESHOLD value in XON/XOFF Threshold configuration register. This is a functional status bit, not an error status bit. Using this information FPM generates backpressure output signal that is used by other UBUS client logics to throttle its operation. For example, UNIMAC logic can use backpressure signal to transfer PAUSE Ethernet flow control packets to throttle incoming frames on Ethernet interface.\n\n",
#endif
    { FPM_POOL3_INTR_STS_XOFF_STATE_STS_FIELD_MASK },
    0,
    { FPM_POOL3_INTR_STS_XOFF_STATE_STS_FIELD_WIDTH },
    { FPM_POOL3_INTR_STS_XOFF_STATE_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: XON_STATE_STS *****/
const ru_field_rec FPM_POOL3_INTR_STS_XON_STATE_STS_FIELD =
{
    "XON_STATE_STS",
#if RU_INCLUDE_DESC
    "",
    "Number of available tokens is greater than or equal to XON_THRESHOLD value in XON/XOFF Threshold configuration register. This is a functional status bit, not an error status bit. Using this information FPM generates backpressure output signal that is used by other UBUS client logics to throttle its operation. For example, UNIMAC logic can use backpressure signal to transfer PAUSE Ethernet flow control packets to throttle incoming frames on Ethernet interface.\n\n",
#endif
    { FPM_POOL3_INTR_STS_XON_STATE_STS_FIELD_MASK },
    0,
    { FPM_POOL3_INTR_STS_XON_STATE_STS_FIELD_WIDTH },
    { FPM_POOL3_INTR_STS_XON_STATE_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ILLEGAL_ADDRESS_ACCESS_STS *****/
const ru_field_rec FPM_POOL3_INTR_STS_ILLEGAL_ADDRESS_ACCESS_STS_FIELD =
{
    "ILLEGAL_ADDRESS_ACCESS_STS",
#if RU_INCLUDE_DESC
    "",
    "Illegal/un-implemented register/memory space access interrupt. This will be active when there is an attempt to read from an unimplemented register or memory space. Along with interrupt being sent an error reply packet will be sent out with o_ubus_error_out asserted.\n\n",
#endif
    { FPM_POOL3_INTR_STS_ILLEGAL_ADDRESS_ACCESS_STS_FIELD_MASK },
    0,
    { FPM_POOL3_INTR_STS_ILLEGAL_ADDRESS_ACCESS_STS_FIELD_WIDTH },
    { FPM_POOL3_INTR_STS_ILLEGAL_ADDRESS_ACCESS_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ILLEGAL_ALLOC_REQUEST_STS *****/
const ru_field_rec FPM_POOL3_INTR_STS_ILLEGAL_ALLOC_REQUEST_STS_FIELD =
{
    "ILLEGAL_ALLOC_REQUEST_STS",
#if RU_INCLUDE_DESC
    "",
    "Illegal token request interrupt. This will be active when the pool is disabled, there is a request for a new token and the alloc fifo for the selected token size is empty. Along with interrupt being sent an error reply packet will be sent out with o_ubus_error_out asserted.\n\n",
#endif
    { FPM_POOL3_INTR_STS_ILLEGAL_ALLOC_REQUEST_STS_FIELD_MASK },
    0,
    { FPM_POOL3_INTR_STS_ILLEGAL_ALLOC_REQUEST_STS_FIELD_WIDTH },
    { FPM_POOL3_INTR_STS_ILLEGAL_ALLOC_REQUEST_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EXPIRED_TOKEN_DET_STS *****/
const ru_field_rec FPM_POOL3_INTR_STS_EXPIRED_TOKEN_DET_STS_FIELD =
{
    "EXPIRED_TOKEN_DET_STS",
#if RU_INCLUDE_DESC
    "",
    "Expired token detect interrupt. This is set when the token recovery logic detects a token that has been held for the entire duration of the aging timer.\n\n",
#endif
    { FPM_POOL3_INTR_STS_EXPIRED_TOKEN_DET_STS_FIELD_MASK },
    0,
    { FPM_POOL3_INTR_STS_EXPIRED_TOKEN_DET_STS_FIELD_WIDTH },
    { FPM_POOL3_INTR_STS_EXPIRED_TOKEN_DET_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EXPIRED_TOKEN_RECOV_STS *****/
const ru_field_rec FPM_POOL3_INTR_STS_EXPIRED_TOKEN_RECOV_STS_FIELD =
{
    "EXPIRED_TOKEN_RECOV_STS",
#if RU_INCLUDE_DESC
    "",
    "Expired token recovered interrupt. This is set when an expired token has been recoveredand returned to pool as an available token.\n\n",
#endif
    { FPM_POOL3_INTR_STS_EXPIRED_TOKEN_RECOV_STS_FIELD_MASK },
    0,
    { FPM_POOL3_INTR_STS_EXPIRED_TOKEN_RECOV_STS_FIELD_WIDTH },
    { FPM_POOL3_INTR_STS_EXPIRED_TOKEN_RECOV_STS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec FPM_POOL3_INTR_STS_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_POOL3_INTR_STS_R1_FIELD_MASK },
    0,
    { FPM_POOL3_INTR_STS_R1_FIELD_WIDTH },
    { FPM_POOL3_INTR_STS_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL3_INTR_STS_FIELDS[] =
{
    &FPM_POOL3_INTR_STS_ALLOC_FIFO_FULL_STS_FIELD,
    &FPM_POOL3_INTR_STS_FREE_FIFO_FULL_STS_FIELD,
    &FPM_POOL3_INTR_STS_POOL_FULL_STS_FIELD,
    &FPM_POOL3_INTR_STS_FREE_TOKEN_NO_VALID_STS_FIELD,
    &FPM_POOL3_INTR_STS_FREE_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD,
    &FPM_POOL3_INTR_STS_MULTI_TOKEN_NO_VALID_STS_FIELD,
    &FPM_POOL3_INTR_STS_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD,
    &FPM_POOL3_INTR_STS_POOL_DIS_FREE_MULTI_STS_FIELD,
    &FPM_POOL3_INTR_STS_MEMORY_CORRUPT_STS_FIELD,
    &FPM_POOL3_INTR_STS_XOFF_STATE_STS_FIELD,
    &FPM_POOL3_INTR_STS_XON_STATE_STS_FIELD,
    &FPM_POOL3_INTR_STS_ILLEGAL_ADDRESS_ACCESS_STS_FIELD,
    &FPM_POOL3_INTR_STS_ILLEGAL_ALLOC_REQUEST_STS_FIELD,
    &FPM_POOL3_INTR_STS_EXPIRED_TOKEN_DET_STS_FIELD,
    &FPM_POOL3_INTR_STS_EXPIRED_TOKEN_RECOV_STS_FIELD,
    &FPM_POOL3_INTR_STS_R1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_POOL3_INTR_STS *****/
const ru_reg_rec FPM_POOL3_INTR_STS_REG =
{
    "POOL3_INTR_STS",
#if RU_INCLUDE_DESC
    "POOL3_INTR_STS Register",
    "Interrupt bits are active high. When a bit in this register is set to 1 and the corresponding bit in interrupt mask register is set to 1, interrupt to CPU will occur. When set (1), interrupts bits can be cleared (0) by writing a 1 to the desired bit.\n\n",
#endif
    { FPM_POOL3_INTR_STS_REG_OFFSET },
    0,
    0,
    453,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    16,
    FPM_POOL3_INTR_STS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_POOL3_STALL_MSK, TYPE: Type_FPM_BLOCK_FPM_CTRL_POOL3_STALL_MSK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: R3 *****/
const ru_field_rec FPM_POOL3_STALL_MSK_R3_FIELD =
{
    "R3",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_POOL3_STALL_MSK_R3_FIELD_MASK },
    0,
    { FPM_POOL3_STALL_MSK_R3_FIELD_WIDTH },
    { FPM_POOL3_STALL_MSK_R3_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_TOKEN_NO_VALID_STALL_MSK *****/
const ru_field_rec FPM_POOL3_STALL_MSK_FREE_TOKEN_NO_VALID_STALL_MSK_FIELD =
{
    "FREE_TOKEN_NO_VALID_STALL_MSK",
#if RU_INCLUDE_DESC
    "",
    "Stall FPM on De-allocation token request with invalid token interrupt status.\n\n",
#endif
    { FPM_POOL3_STALL_MSK_FREE_TOKEN_NO_VALID_STALL_MSK_FIELD_MASK },
    0,
    { FPM_POOL3_STALL_MSK_FREE_TOKEN_NO_VALID_STALL_MSK_FIELD_WIDTH },
    { FPM_POOL3_STALL_MSK_FREE_TOKEN_NO_VALID_STALL_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK *****/
const ru_field_rec FPM_POOL3_STALL_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD =
{
    "FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK",
#if RU_INCLUDE_DESC
    "",
    "Stall FPM on De-allocation token request with index out-of-range interrupt status.\n\n",
#endif
    { FPM_POOL3_STALL_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD_MASK },
    0,
    { FPM_POOL3_STALL_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD_WIDTH },
    { FPM_POOL3_STALL_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MULTI_TOKEN_NO_VALID_STALL_MSK *****/
const ru_field_rec FPM_POOL3_STALL_MSK_MULTI_TOKEN_NO_VALID_STALL_MSK_FIELD =
{
    "MULTI_TOKEN_NO_VALID_STALL_MSK",
#if RU_INCLUDE_DESC
    "",
    "Stall FPM on Token multi-cast value update request with invalid token interrupt status.\n\n",
#endif
    { FPM_POOL3_STALL_MSK_MULTI_TOKEN_NO_VALID_STALL_MSK_FIELD_MASK },
    0,
    { FPM_POOL3_STALL_MSK_MULTI_TOKEN_NO_VALID_STALL_MSK_FIELD_WIDTH },
    { FPM_POOL3_STALL_MSK_MULTI_TOKEN_NO_VALID_STALL_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK *****/
const ru_field_rec FPM_POOL3_STALL_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD =
{
    "MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK",
#if RU_INCLUDE_DESC
    "",
    "Stall FPM on Token multi-cast value update request with index out-of-range interrupt status.\n\n",
#endif
    { FPM_POOL3_STALL_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD_MASK },
    0,
    { FPM_POOL3_STALL_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD_WIDTH },
    { FPM_POOL3_STALL_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R2 *****/
const ru_field_rec FPM_POOL3_STALL_MSK_R2_FIELD =
{
    "R2",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_POOL3_STALL_MSK_R2_FIELD_MASK },
    0,
    { FPM_POOL3_STALL_MSK_R2_FIELD_WIDTH },
    { FPM_POOL3_STALL_MSK_R2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MEMORY_CORRUPT_STALL_MSK *****/
const ru_field_rec FPM_POOL3_STALL_MSK_MEMORY_CORRUPT_STALL_MSK_FIELD =
{
    "MEMORY_CORRUPT_STALL_MSK",
#if RU_INCLUDE_DESC
    "",
    "Stall FPM on Index Memory corrupt interrupt status.\n\n",
#endif
    { FPM_POOL3_STALL_MSK_MEMORY_CORRUPT_STALL_MSK_FIELD_MASK },
    0,
    { FPM_POOL3_STALL_MSK_MEMORY_CORRUPT_STALL_MSK_FIELD_WIDTH },
    { FPM_POOL3_STALL_MSK_MEMORY_CORRUPT_STALL_MSK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec FPM_POOL3_STALL_MSK_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_POOL3_STALL_MSK_R1_FIELD_MASK },
    0,
    { FPM_POOL3_STALL_MSK_R1_FIELD_WIDTH },
    { FPM_POOL3_STALL_MSK_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL3_STALL_MSK_FIELDS[] =
{
    &FPM_POOL3_STALL_MSK_R3_FIELD,
    &FPM_POOL3_STALL_MSK_FREE_TOKEN_NO_VALID_STALL_MSK_FIELD,
    &FPM_POOL3_STALL_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD,
    &FPM_POOL3_STALL_MSK_MULTI_TOKEN_NO_VALID_STALL_MSK_FIELD,
    &FPM_POOL3_STALL_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD,
    &FPM_POOL3_STALL_MSK_R2_FIELD,
    &FPM_POOL3_STALL_MSK_MEMORY_CORRUPT_STALL_MSK_FIELD,
    &FPM_POOL3_STALL_MSK_R1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_POOL3_STALL_MSK *****/
const ru_reg_rec FPM_POOL3_STALL_MSK_REG =
{
    "POOL3_STALL_MSK",
#if RU_INCLUDE_DESC
    "POOL3_STALL_MSK Register",
    "Software sets desired stall bits that upon corresponding active interrupt status will stall FPM from new allocation, de-allocation, and mcast update process. Listed below are the supported interrupt statuses\n1. Invalid free token (bit[3] of interrupt status register 0x14)\n2. Invalid free token with index out-of-range (bit[4] of interrupt status register 0x14)\n3. Invalid mcast token (bit[5] of interrupt status register 0x14)\n4. Invalid mcast token with index out-of-range (bit[6] of interrupt status register 0x14)\n5. Memory corrupt status (bit[8] of interrupt status register 0x14)\nWhen state machine is stalled, registers and memory can still be accessed. Any new token allocation request will be serviced with valid tokens (if available in alloc cache) and invalid tokens (if alloc cache is empty). Any new de-allocation/mcast update requests will be either stored in de-allocation fifo (if there is space in free fifo) or dropped (if free fifo is full). Bit locations in this register matches the location of corrseponding interrupt status bits in register 0x14. To un-stall (enable) state machine interrupt status bits (in register 0x14) corresponding to these mask bits should be cleared. Stall mask bits are active high and are disabled by default. This is for debug purposes only.\n\n",
#endif
    { FPM_POOL3_STALL_MSK_REG_OFFSET },
    0,
    0,
    454,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    FPM_POOL3_STALL_MSK_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_POOL1_CFG1, TYPE: Type_FPM_BLOCK_FPM_CTRL_POOL1_CFG1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: R2 *****/
const ru_field_rec FPM_POOL1_CFG1_R2_FIELD =
{
    "R2",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_POOL1_CFG1_R2_FIELD_MASK },
    0,
    { FPM_POOL1_CFG1_R2_FIELD_WIDTH },
    { FPM_POOL1_CFG1_R2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FP_BUF_SIZE *****/
const ru_field_rec FPM_POOL1_CFG1_FP_BUF_SIZE_FIELD =
{
    "FP_BUF_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Selects the size of the buffer to be used in the pool. All buffers must be the same size.\n0 - 512 byte buffers\n1 - 256 byte buffers\nall other values - reserved\n\n",
#endif
    { FPM_POOL1_CFG1_FP_BUF_SIZE_FIELD_MASK },
    0,
    { FPM_POOL1_CFG1_FP_BUF_SIZE_FIELD_WIDTH },
    { FPM_POOL1_CFG1_FP_BUF_SIZE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec FPM_POOL1_CFG1_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_POOL1_CFG1_R1_FIELD_MASK },
    0,
    { FPM_POOL1_CFG1_R1_FIELD_WIDTH },
    { FPM_POOL1_CFG1_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL1_CFG1_FIELDS[] =
{
    &FPM_POOL1_CFG1_R2_FIELD,
    &FPM_POOL1_CFG1_FP_BUF_SIZE_FIELD,
    &FPM_POOL1_CFG1_R1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_POOL1_CFG1 *****/
const ru_reg_rec FPM_POOL1_CFG1_REG =
{
    "POOL1_CFG1",
#if RU_INCLUDE_DESC
    "POOL1_CFG1 Register",
    "No Description\n",
#endif
    { FPM_POOL1_CFG1_REG_OFFSET },
    0,
    0,
    455,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    FPM_POOL1_CFG1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_POOL1_CFG2, TYPE: Type_FPM_BLOCK_FPM_CTRL_POOL1_CFG2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec FPM_POOL1_CFG2_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_POOL1_CFG2_R1_FIELD_MASK },
    0,
    { FPM_POOL1_CFG2_R1_FIELD_WIDTH },
    { FPM_POOL1_CFG2_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_BASE_ADDRESS *****/
const ru_field_rec FPM_POOL1_CFG2_POOL_BASE_ADDRESS_FIELD =
{
    "POOL_BASE_ADDRESS",
#if RU_INCLUDE_DESC
    "",
    "Buffer base address. 7:2 must be 0x00.\n\n",
#endif
    { FPM_POOL1_CFG2_POOL_BASE_ADDRESS_FIELD_MASK },
    0,
    { FPM_POOL1_CFG2_POOL_BASE_ADDRESS_FIELD_WIDTH },
    { FPM_POOL1_CFG2_POOL_BASE_ADDRESS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL1_CFG2_FIELDS[] =
{
    &FPM_POOL1_CFG2_R1_FIELD,
    &FPM_POOL1_CFG2_POOL_BASE_ADDRESS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_POOL1_CFG2 *****/
const ru_reg_rec FPM_POOL1_CFG2_REG =
{
    "POOL1_CFG2",
#if RU_INCLUDE_DESC
    "POOL1_CFG2 Register",
    "This register sets the physical base address of this memory. The memory block should be the number of buffers times the buffer size. This is mainly used for multi-pool memory configuration. NOTE: POOL_BASE_ADDRESS[7:2] and reserved[1:0] field must be written with 0x00 in the BCM3382 because itstoken-to-address converter assumes the buffers start on a 2kB boundary.\n\n",
#endif
    { FPM_POOL1_CFG2_REG_OFFSET },
    0,
    0,
    456,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_POOL1_CFG2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_POOL1_CFG3, TYPE: Type_FPM_BLOCK_FPM_CTRL_POOL1_CFG3
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec FPM_POOL1_CFG3_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_POOL1_CFG3_R1_FIELD_MASK },
    0,
    { FPM_POOL1_CFG3_R1_FIELD_WIDTH },
    { FPM_POOL1_CFG3_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_BASE_ADDRESS_POOL2 *****/
const ru_field_rec FPM_POOL1_CFG3_POOL_BASE_ADDRESS_POOL2_FIELD =
{
    "POOL_BASE_ADDRESS_POOL2",
#if RU_INCLUDE_DESC
    "",
    "Buffer base address. 7:2 must be 0x00.\n\n",
#endif
    { FPM_POOL1_CFG3_POOL_BASE_ADDRESS_POOL2_FIELD_MASK },
    0,
    { FPM_POOL1_CFG3_POOL_BASE_ADDRESS_POOL2_FIELD_WIDTH },
    { FPM_POOL1_CFG3_POOL_BASE_ADDRESS_POOL2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL1_CFG3_FIELDS[] =
{
    &FPM_POOL1_CFG3_R1_FIELD,
    &FPM_POOL1_CFG3_POOL_BASE_ADDRESS_POOL2_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_POOL1_CFG3 *****/
const ru_reg_rec FPM_POOL1_CFG3_REG =
{
    "POOL1_CFG3",
#if RU_INCLUDE_DESC
    "POOL1_CFG3 Register",
    "This register sets the physical base address of this memory. The memory block should be the number of buffers times the buffer size. This is mainly used for multi-pool memory configuration.\n\n",
#endif
    { FPM_POOL1_CFG3_REG_OFFSET },
    0,
    0,
    457,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_POOL1_CFG3_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_POOL1_CFG4, TYPE: Type_FPM_BLOCK_FPM_CTRL_POOL1_CFG4
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec FPM_POOL1_CFG4_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_POOL1_CFG4_R1_FIELD_MASK },
    0,
    { FPM_POOL1_CFG4_R1_FIELD_WIDTH },
    { FPM_POOL1_CFG4_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_BASE_ADDRESS_POOL3 *****/
const ru_field_rec FPM_POOL1_CFG4_POOL_BASE_ADDRESS_POOL3_FIELD =
{
    "POOL_BASE_ADDRESS_POOL3",
#if RU_INCLUDE_DESC
    "",
    "Buffer base address. 7:2 must be 0x00.\n\n",
#endif
    { FPM_POOL1_CFG4_POOL_BASE_ADDRESS_POOL3_FIELD_MASK },
    0,
    { FPM_POOL1_CFG4_POOL_BASE_ADDRESS_POOL3_FIELD_WIDTH },
    { FPM_POOL1_CFG4_POOL_BASE_ADDRESS_POOL3_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL1_CFG4_FIELDS[] =
{
    &FPM_POOL1_CFG4_R1_FIELD,
    &FPM_POOL1_CFG4_POOL_BASE_ADDRESS_POOL3_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_POOL1_CFG4 *****/
const ru_reg_rec FPM_POOL1_CFG4_REG =
{
    "POOL1_CFG4",
#if RU_INCLUDE_DESC
    "POOL1_CFG4 Register",
    "This register sets the physical base address of this memory. The memory block should be the number of buffers times the buffer size. This is mainly used for multi-pool memory configuration.\n\n",
#endif
    { FPM_POOL1_CFG4_REG_OFFSET },
    0,
    0,
    458,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_POOL1_CFG4_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_POOL1_STAT1, TYPE: Type_FPM_BLOCK_FPM_CTRL_POOL1_STAT1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: UNDRFL *****/
const ru_field_rec FPM_POOL1_STAT1_UNDRFL_FIELD =
{
    "UNDRFL",
#if RU_INCLUDE_DESC
    "",
    "Free Pool underflow count\n\n",
#endif
    { FPM_POOL1_STAT1_UNDRFL_FIELD_MASK },
    0,
    { FPM_POOL1_STAT1_UNDRFL_FIELD_WIDTH },
    { FPM_POOL1_STAT1_UNDRFL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: OVRFL *****/
const ru_field_rec FPM_POOL1_STAT1_OVRFL_FIELD =
{
    "OVRFL",
#if RU_INCLUDE_DESC
    "",
    "Free Pool overflow count\n\n",
#endif
    { FPM_POOL1_STAT1_OVRFL_FIELD_MASK },
    0,
    { FPM_POOL1_STAT1_OVRFL_FIELD_WIDTH },
    { FPM_POOL1_STAT1_OVRFL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL1_STAT1_FIELDS[] =
{
    &FPM_POOL1_STAT1_UNDRFL_FIELD,
    &FPM_POOL1_STAT1_OVRFL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_POOL1_STAT1 *****/
const ru_reg_rec FPM_POOL1_STAT1_REG =
{
    "POOL1_STAT1",
#if RU_INCLUDE_DESC
    "POOL1_STAT1 Register",
    "This read only register allows software to read the count of free pool overflows and underflows. A overflow condition occurs when pool is empty, ie., no tokens are allocated and free/mcast request is encountered. A underflow condition occurs when pool is full, ie., there are no free tokens and a allocation request is encountered. When the counter values reaches maximum count, it will hold the max value and not increment the count value unless it is cleared. Any write to this register will clear both both counters.\n\n",
#endif
    { FPM_POOL1_STAT1_REG_OFFSET },
    0,
    0,
    459,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_POOL1_STAT1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_POOL1_STAT2, TYPE: Type_FPM_BLOCK_FPM_CTRL_POOL1_STAT2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: NUM_OF_TOKENS_AVAILABLE *****/
const ru_field_rec FPM_POOL1_STAT2_NUM_OF_TOKENS_AVAILABLE_FIELD =
{
    "NUM_OF_TOKENS_AVAILABLE",
#if RU_INCLUDE_DESC
    "",
    "Count of tokens available for allocation.\nThis provides a count of number of free tokens that available for allocation in the usage array. This value is updated instantaneously as tokens are allocated or freed from the array.\n\n",
#endif
    { FPM_POOL1_STAT2_NUM_OF_TOKENS_AVAILABLE_FIELD_MASK },
    0,
    { FPM_POOL1_STAT2_NUM_OF_TOKENS_AVAILABLE_FIELD_WIDTH },
    { FPM_POOL1_STAT2_NUM_OF_TOKENS_AVAILABLE_FIELD_SHIFT },
    262144,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R2 *****/
const ru_field_rec FPM_POOL1_STAT2_R2_FIELD =
{
    "R2",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_POOL1_STAT2_R2_FIELD_MASK },
    0,
    { FPM_POOL1_STAT2_R2_FIELD_WIDTH },
    { FPM_POOL1_STAT2_R2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ALLOC_FIFO_EMPTY *****/
const ru_field_rec FPM_POOL1_STAT2_ALLOC_FIFO_EMPTY_FIELD =
{
    "ALLOC_FIFO_EMPTY",
#if RU_INCLUDE_DESC
    "",
    "ALLOC_FIFO is empty.\n\n",
#endif
    { FPM_POOL1_STAT2_ALLOC_FIFO_EMPTY_FIELD_MASK },
    0,
    { FPM_POOL1_STAT2_ALLOC_FIFO_EMPTY_FIELD_WIDTH },
    { FPM_POOL1_STAT2_ALLOC_FIFO_EMPTY_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ALLOC_FIFO_FULL *****/
const ru_field_rec FPM_POOL1_STAT2_ALLOC_FIFO_FULL_FIELD =
{
    "ALLOC_FIFO_FULL",
#if RU_INCLUDE_DESC
    "",
    "ALLOC_FIFO is full\n\n",
#endif
    { FPM_POOL1_STAT2_ALLOC_FIFO_FULL_FIELD_MASK },
    0,
    { FPM_POOL1_STAT2_ALLOC_FIFO_FULL_FIELD_WIDTH },
    { FPM_POOL1_STAT2_ALLOC_FIFO_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_FIFO_EMPTY *****/
const ru_field_rec FPM_POOL1_STAT2_FREE_FIFO_EMPTY_FIELD =
{
    "FREE_FIFO_EMPTY",
#if RU_INCLUDE_DESC
    "",
    "FREE_FIFO is empty\n\n",
#endif
    { FPM_POOL1_STAT2_FREE_FIFO_EMPTY_FIELD_MASK },
    0,
    { FPM_POOL1_STAT2_FREE_FIFO_EMPTY_FIELD_WIDTH },
    { FPM_POOL1_STAT2_FREE_FIFO_EMPTY_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_FIFO_FULL *****/
const ru_field_rec FPM_POOL1_STAT2_FREE_FIFO_FULL_FIELD =
{
    "FREE_FIFO_FULL",
#if RU_INCLUDE_DESC
    "",
    "FREE_FIFO is full.\n\n",
#endif
    { FPM_POOL1_STAT2_FREE_FIFO_FULL_FIELD_MASK },
    0,
    { FPM_POOL1_STAT2_FREE_FIFO_FULL_FIELD_WIDTH },
    { FPM_POOL1_STAT2_FREE_FIFO_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec FPM_POOL1_STAT2_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_POOL1_STAT2_R1_FIELD_MASK },
    0,
    { FPM_POOL1_STAT2_R1_FIELD_WIDTH },
    { FPM_POOL1_STAT2_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_FULL *****/
const ru_field_rec FPM_POOL1_STAT2_POOL_FULL_FIELD =
{
    "POOL_FULL",
#if RU_INCLUDE_DESC
    "",
    "POOL is full\nThis indicates that all tokens have been allocated and there no free tokens available. This bit will be active as long as all usage array is fully allocated.\n\n",
#endif
    { FPM_POOL1_STAT2_POOL_FULL_FIELD_MASK },
    0,
    { FPM_POOL1_STAT2_POOL_FULL_FIELD_WIDTH },
    { FPM_POOL1_STAT2_POOL_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL1_STAT2_FIELDS[] =
{
    &FPM_POOL1_STAT2_NUM_OF_TOKENS_AVAILABLE_FIELD,
    &FPM_POOL1_STAT2_R2_FIELD,
    &FPM_POOL1_STAT2_ALLOC_FIFO_EMPTY_FIELD,
    &FPM_POOL1_STAT2_ALLOC_FIFO_FULL_FIELD,
    &FPM_POOL1_STAT2_FREE_FIFO_EMPTY_FIELD,
    &FPM_POOL1_STAT2_FREE_FIFO_FULL_FIELD,
    &FPM_POOL1_STAT2_R1_FIELD,
    &FPM_POOL1_STAT2_POOL_FULL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_POOL1_STAT2 *****/
const ru_reg_rec FPM_POOL1_STAT2_REG =
{
    "POOL1_STAT2",
#if RU_INCLUDE_DESC
    "POOL1_STAT2 Register",
    "This read only register provide status of index memory, alloc & free cache/fifos. These are real time statuses and bits are not sticky. Write to any bits will have no effect.\n\n",
#endif
    { FPM_POOL1_STAT2_REG_OFFSET },
    0,
    0,
    460,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    FPM_POOL1_STAT2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_POOL1_STAT3, TYPE: Type_FPM_BLOCK_FPM_CTRL_POOL1_STAT3
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: NUM_OF_NOT_VALID_TOKEN_FREES *****/
const ru_field_rec FPM_POOL1_STAT3_NUM_OF_NOT_VALID_TOKEN_FREES_FIELD =
{
    "NUM_OF_NOT_VALID_TOKEN_FREES",
#if RU_INCLUDE_DESC
    "",
    "Count of de-allocate token requests with invalid tokens. For more information on conditions under which this counter is incremented, refer to POOL1_INTR_STS register (offset 0x14) bit[3] explanation in this document.\n\n",
#endif
    { FPM_POOL1_STAT3_NUM_OF_NOT_VALID_TOKEN_FREES_FIELD_MASK },
    0,
    { FPM_POOL1_STAT3_NUM_OF_NOT_VALID_TOKEN_FREES_FIELD_WIDTH },
    { FPM_POOL1_STAT3_NUM_OF_NOT_VALID_TOKEN_FREES_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec FPM_POOL1_STAT3_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_POOL1_STAT3_R1_FIELD_MASK },
    0,
    { FPM_POOL1_STAT3_R1_FIELD_WIDTH },
    { FPM_POOL1_STAT3_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL1_STAT3_FIELDS[] =
{
    &FPM_POOL1_STAT3_NUM_OF_NOT_VALID_TOKEN_FREES_FIELD,
    &FPM_POOL1_STAT3_R1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_POOL1_STAT3 *****/
const ru_reg_rec FPM_POOL1_STAT3_REG =
{
    "POOL1_STAT3",
#if RU_INCLUDE_DESC
    "POOL1_STAT3 Register",
    "This read only register allows software to read the count of free token requests with in-valid tokens When the counter values reaches maximum count, it will hold the max value and not increment the count value unless it is cleared. Any write to this register will clear count value.\n\n",
#endif
    { FPM_POOL1_STAT3_REG_OFFSET },
    0,
    0,
    461,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_POOL1_STAT3_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_POOL1_STAT4, TYPE: Type_FPM_BLOCK_FPM_CTRL_POOL1_STAT4
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: NUM_OF_NOT_VALID_TOKEN_MULTI *****/
const ru_field_rec FPM_POOL1_STAT4_NUM_OF_NOT_VALID_TOKEN_MULTI_FIELD =
{
    "NUM_OF_NOT_VALID_TOKEN_MULTI",
#if RU_INCLUDE_DESC
    "",
    "Count of multi-cast token update requests with either valid bit not set, For more information on conditions under which this counter is incremented, refer to POOL1_INTR_STS register (offset 0x14) bit[5] explanation in this document.\n\n",
#endif
    { FPM_POOL1_STAT4_NUM_OF_NOT_VALID_TOKEN_MULTI_FIELD_MASK },
    0,
    { FPM_POOL1_STAT4_NUM_OF_NOT_VALID_TOKEN_MULTI_FIELD_WIDTH },
    { FPM_POOL1_STAT4_NUM_OF_NOT_VALID_TOKEN_MULTI_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec FPM_POOL1_STAT4_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_POOL1_STAT4_R1_FIELD_MASK },
    0,
    { FPM_POOL1_STAT4_R1_FIELD_WIDTH },
    { FPM_POOL1_STAT4_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL1_STAT4_FIELDS[] =
{
    &FPM_POOL1_STAT4_NUM_OF_NOT_VALID_TOKEN_MULTI_FIELD,
    &FPM_POOL1_STAT4_R1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_POOL1_STAT4 *****/
const ru_reg_rec FPM_POOL1_STAT4_REG =
{
    "POOL1_STAT4",
#if RU_INCLUDE_DESC
    "POOL1_STAT4 Register",
    "This read only register allows software to read the count of multi-cast token update requests with in-valid tokens. When the counter values reaches maximum count, it will hold the max value and not increment the count value unless it is cleared. Any write to this register will clear count value.\n\n",
#endif
    { FPM_POOL1_STAT4_REG_OFFSET },
    0,
    0,
    462,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_POOL1_STAT4_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_POOL1_STAT5, TYPE: Type_FPM_BLOCK_FPM_CTRL_POOL1_STAT5
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN *****/
const ru_field_rec FPM_POOL1_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_FIELD =
{
    "MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN",
#if RU_INCLUDE_DESC
    "",
    "Token that causes memory corrupt interrupt active. If there are multiple tokens that causes this error, only the first one is captured. To capture successive tokens that causes the error this register should be cleared by writing any random value, in addition, memory corrupt status bit (bit[8]) in interrupt status register 0x14 should be cleared. Bitmap for these bits is shown below (reserved bits are zeros)\nBit[30]    - Reserved\nBit[29:12] - Token\nBit[11:0]  - Buffer size in bytes\n\n",
#endif
    { FPM_POOL1_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_FIELD_MASK },
    0,
    { FPM_POOL1_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_FIELD_WIDTH },
    { FPM_POOL1_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL1_STAT5_FIELDS[] =
{
    &FPM_POOL1_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_POOL1_STAT5 *****/
const ru_reg_rec FPM_POOL1_STAT5_REG =
{
    "POOL1_STAT5",
#if RU_INCLUDE_DESC
    "POOL1_STAT5 Register",
    "This read only register allows software to read the alloc token that causes memory corrupt interrupt (intr[8]) to go active. This is for debug purposes only. Any write to this register will clear token value (makes all bits one).\n\n",
#endif
    { FPM_POOL1_STAT5_REG_OFFSET },
    0,
    0,
    463,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_POOL1_STAT5_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_POOL1_STAT6, TYPE: Type_FPM_BLOCK_FPM_CTRL_POOL1_STAT6
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: INVALID_FREE_TOKEN *****/
const ru_field_rec FPM_POOL1_STAT6_INVALID_FREE_TOKEN_FIELD =
{
    "INVALID_FREE_TOKEN",
#if RU_INCLUDE_DESC
    "",
    "Token that causes intr[3] or intr[4] active. If there are multiple tokens that causes this error, only the first one is captured. To capture successive tokens that causes the error this register should be cleared by writing any random value. Bitmap for these bits is shown below (reserved bits are either zeros or can reflect the length of the packet associated with the freed token)\nBit[30]    - Reserved\nBit[29:12] - Token\nBit[11:0]  - Reserved\n\n",
#endif
    { FPM_POOL1_STAT6_INVALID_FREE_TOKEN_FIELD_MASK },
    0,
    { FPM_POOL1_STAT6_INVALID_FREE_TOKEN_FIELD_WIDTH },
    { FPM_POOL1_STAT6_INVALID_FREE_TOKEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL1_STAT6_FIELDS[] =
{
    &FPM_POOL1_STAT6_INVALID_FREE_TOKEN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_POOL1_STAT6 *****/
const ru_reg_rec FPM_POOL1_STAT6_REG =
{
    "POOL1_STAT6",
#if RU_INCLUDE_DESC
    "POOL1_STAT6 Register",
    "This read only register allows software to read the free token that causes invalid free request or free token with index out-of-range interrupts (intr[3] or intr[4]) to go active. This is for debug purposes only. Any write to this register will clear token value (makes all bits zero).\n\n",
#endif
    { FPM_POOL1_STAT6_REG_OFFSET },
    0,
    0,
    464,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_POOL1_STAT6_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_POOL1_STAT7, TYPE: Type_FPM_BLOCK_FPM_CTRL_POOL1_STAT7
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: INVALID_MCAST_TOKEN *****/
const ru_field_rec FPM_POOL1_STAT7_INVALID_MCAST_TOKEN_FIELD =
{
    "INVALID_MCAST_TOKEN",
#if RU_INCLUDE_DESC
    "",
    "Token that causes intr[5] or intr[6] active. If there are multiple tokens that causes this error, only the first one is captured. To capture successive tokens that causes the error this register should be cleared by writing any random value. Bitmap for these bits is shown below (reserved bits are zeros)\nBit[30]    - Reserved\nBit[29:12] - Token\nBit[11]    - Mcast update type (refer to register 0x224[11])\nBit[10:7]  - Reserved\nBit[6:0]   - Mcast value\n\n",
#endif
    { FPM_POOL1_STAT7_INVALID_MCAST_TOKEN_FIELD_MASK },
    0,
    { FPM_POOL1_STAT7_INVALID_MCAST_TOKEN_FIELD_WIDTH },
    { FPM_POOL1_STAT7_INVALID_MCAST_TOKEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL1_STAT7_FIELDS[] =
{
    &FPM_POOL1_STAT7_INVALID_MCAST_TOKEN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_POOL1_STAT7 *****/
const ru_reg_rec FPM_POOL1_STAT7_REG =
{
    "POOL1_STAT7",
#if RU_INCLUDE_DESC
    "POOL1_STAT7 Register",
    "This read only register allows software to read the multi-cast token that causes invalid mcast request or mcast token with index out-of-range interrupts (intr[5] or intr[6]) to go active. This is for debug purposes only. Any write to this register will clear token value (makes all bits one).\n\n",
#endif
    { FPM_POOL1_STAT7_REG_OFFSET },
    0,
    0,
    465,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_POOL1_STAT7_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_POOL1_STAT8, TYPE: Type_FPM_BLOCK_FPM_CTRL_POOL1_STAT8
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKENS_AVAILABLE_LOW_WTMK *****/
const ru_field_rec FPM_POOL1_STAT8_TOKENS_AVAILABLE_LOW_WTMK_FIELD =
{
    "TOKENS_AVAILABLE_LOW_WTMK",
#if RU_INCLUDE_DESC
    "",
    "Lowest value the NUM_OF_TOKENS_AVAIL count has reached.\n\n",
#endif
    { FPM_POOL1_STAT8_TOKENS_AVAILABLE_LOW_WTMK_FIELD_MASK },
    0,
    { FPM_POOL1_STAT8_TOKENS_AVAILABLE_LOW_WTMK_FIELD_WIDTH },
    { FPM_POOL1_STAT8_TOKENS_AVAILABLE_LOW_WTMK_FIELD_SHIFT },
    65536,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec FPM_POOL1_STAT8_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_POOL1_STAT8_R1_FIELD_MASK },
    0,
    { FPM_POOL1_STAT8_R1_FIELD_WIDTH },
    { FPM_POOL1_STAT8_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL1_STAT8_FIELDS[] =
{
    &FPM_POOL1_STAT8_TOKENS_AVAILABLE_LOW_WTMK_FIELD,
    &FPM_POOL1_STAT8_R1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_POOL1_STAT8 *****/
const ru_reg_rec FPM_POOL1_STAT8_REG =
{
    "POOL1_STAT8",
#if RU_INCLUDE_DESC
    "POOL1_STAT8 Register",
    "This register allows software to read the lowest value the NUM_OF_TOKENS_AVAILABLE count reached since the last time it was cleared. Any write to this register will reset the value back to the maximum number of tokens (0x100000)\n\n",
#endif
    { FPM_POOL1_STAT8_REG_OFFSET },
    0,
    0,
    466,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_POOL1_STAT8_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_POOL2_STAT1, TYPE: Type_FPM_BLOCK_FPM_CTRL_POOL2_STAT1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: UNDRFL *****/
const ru_field_rec FPM_POOL2_STAT1_UNDRFL_FIELD =
{
    "UNDRFL",
#if RU_INCLUDE_DESC
    "",
    "Free Pool underflow count\n\n",
#endif
    { FPM_POOL2_STAT1_UNDRFL_FIELD_MASK },
    0,
    { FPM_POOL2_STAT1_UNDRFL_FIELD_WIDTH },
    { FPM_POOL2_STAT1_UNDRFL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: OVRFL *****/
const ru_field_rec FPM_POOL2_STAT1_OVRFL_FIELD =
{
    "OVRFL",
#if RU_INCLUDE_DESC
    "",
    "Free Pool overflow count\n\n",
#endif
    { FPM_POOL2_STAT1_OVRFL_FIELD_MASK },
    0,
    { FPM_POOL2_STAT1_OVRFL_FIELD_WIDTH },
    { FPM_POOL2_STAT1_OVRFL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL2_STAT1_FIELDS[] =
{
    &FPM_POOL2_STAT1_UNDRFL_FIELD,
    &FPM_POOL2_STAT1_OVRFL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_POOL2_STAT1 *****/
const ru_reg_rec FPM_POOL2_STAT1_REG =
{
    "POOL2_STAT1",
#if RU_INCLUDE_DESC
    "POOL2_STAT1 Register",
    "This read only register allows software to read the count of free pool overflows and underflows. A overflow condition occurs when pool is empty, ie., no tokens are allocated and free/mcast request is encountered. A underflow condition occurs when pool is full, ie., there are no free tokens and a allocation request is encountered. When the counter values reaches maximum count, it will hold the max value and not increment the count value unless it is cleared. Any write to this register will clear both both counters.\n\n",
#endif
    { FPM_POOL2_STAT1_REG_OFFSET },
    0,
    0,
    467,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_POOL2_STAT1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_POOL2_STAT2, TYPE: Type_FPM_BLOCK_FPM_CTRL_POOL2_STAT2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: NUM_OF_TOKENS_AVAILABLE *****/
const ru_field_rec FPM_POOL2_STAT2_NUM_OF_TOKENS_AVAILABLE_FIELD =
{
    "NUM_OF_TOKENS_AVAILABLE",
#if RU_INCLUDE_DESC
    "",
    "Count of tokens available for allocation.\nThis provides a count of number of free tokens that available for allocation in the usage array. This value is updated instantaneously as tokens are allocated or freed from the array.\n\n",
#endif
    { FPM_POOL2_STAT2_NUM_OF_TOKENS_AVAILABLE_FIELD_MASK },
    0,
    { FPM_POOL2_STAT2_NUM_OF_TOKENS_AVAILABLE_FIELD_WIDTH },
    { FPM_POOL2_STAT2_NUM_OF_TOKENS_AVAILABLE_FIELD_SHIFT },
    65536,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R2 *****/
const ru_field_rec FPM_POOL2_STAT2_R2_FIELD =
{
    "R2",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_POOL2_STAT2_R2_FIELD_MASK },
    0,
    { FPM_POOL2_STAT2_R2_FIELD_WIDTH },
    { FPM_POOL2_STAT2_R2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ALLOC_FIFO_EMPTY *****/
const ru_field_rec FPM_POOL2_STAT2_ALLOC_FIFO_EMPTY_FIELD =
{
    "ALLOC_FIFO_EMPTY",
#if RU_INCLUDE_DESC
    "",
    "ALLOC_FIFO is empty.\n\n",
#endif
    { FPM_POOL2_STAT2_ALLOC_FIFO_EMPTY_FIELD_MASK },
    0,
    { FPM_POOL2_STAT2_ALLOC_FIFO_EMPTY_FIELD_WIDTH },
    { FPM_POOL2_STAT2_ALLOC_FIFO_EMPTY_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ALLOC_FIFO_FULL *****/
const ru_field_rec FPM_POOL2_STAT2_ALLOC_FIFO_FULL_FIELD =
{
    "ALLOC_FIFO_FULL",
#if RU_INCLUDE_DESC
    "",
    "ALLOC_FIFO is full\n\n",
#endif
    { FPM_POOL2_STAT2_ALLOC_FIFO_FULL_FIELD_MASK },
    0,
    { FPM_POOL2_STAT2_ALLOC_FIFO_FULL_FIELD_WIDTH },
    { FPM_POOL2_STAT2_ALLOC_FIFO_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_FIFO_EMPTY *****/
const ru_field_rec FPM_POOL2_STAT2_FREE_FIFO_EMPTY_FIELD =
{
    "FREE_FIFO_EMPTY",
#if RU_INCLUDE_DESC
    "",
    "FREE_FIFO is empty\n\n",
#endif
    { FPM_POOL2_STAT2_FREE_FIFO_EMPTY_FIELD_MASK },
    0,
    { FPM_POOL2_STAT2_FREE_FIFO_EMPTY_FIELD_WIDTH },
    { FPM_POOL2_STAT2_FREE_FIFO_EMPTY_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_FIFO_FULL *****/
const ru_field_rec FPM_POOL2_STAT2_FREE_FIFO_FULL_FIELD =
{
    "FREE_FIFO_FULL",
#if RU_INCLUDE_DESC
    "",
    "FREE_FIFO is full.\n\n",
#endif
    { FPM_POOL2_STAT2_FREE_FIFO_FULL_FIELD_MASK },
    0,
    { FPM_POOL2_STAT2_FREE_FIFO_FULL_FIELD_WIDTH },
    { FPM_POOL2_STAT2_FREE_FIFO_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec FPM_POOL2_STAT2_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_POOL2_STAT2_R1_FIELD_MASK },
    0,
    { FPM_POOL2_STAT2_R1_FIELD_WIDTH },
    { FPM_POOL2_STAT2_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_FULL *****/
const ru_field_rec FPM_POOL2_STAT2_POOL_FULL_FIELD =
{
    "POOL_FULL",
#if RU_INCLUDE_DESC
    "",
    "POOL is full\nThis indicates that all tokens have been allocated and there no free tokens available. This bit will be active as long as all usage array is fully allocated.\n\n",
#endif
    { FPM_POOL2_STAT2_POOL_FULL_FIELD_MASK },
    0,
    { FPM_POOL2_STAT2_POOL_FULL_FIELD_WIDTH },
    { FPM_POOL2_STAT2_POOL_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL2_STAT2_FIELDS[] =
{
    &FPM_POOL2_STAT2_NUM_OF_TOKENS_AVAILABLE_FIELD,
    &FPM_POOL2_STAT2_R2_FIELD,
    &FPM_POOL2_STAT2_ALLOC_FIFO_EMPTY_FIELD,
    &FPM_POOL2_STAT2_ALLOC_FIFO_FULL_FIELD,
    &FPM_POOL2_STAT2_FREE_FIFO_EMPTY_FIELD,
    &FPM_POOL2_STAT2_FREE_FIFO_FULL_FIELD,
    &FPM_POOL2_STAT2_R1_FIELD,
    &FPM_POOL2_STAT2_POOL_FULL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_POOL2_STAT2 *****/
const ru_reg_rec FPM_POOL2_STAT2_REG =
{
    "POOL2_STAT2",
#if RU_INCLUDE_DESC
    "POOL2_STAT2 Register",
    "This read only register provide status of index memory, alloc & free cache/fifos. These are real time statuses and bits are not sticky. Write to any bits will have no effect.\n\n",
#endif
    { FPM_POOL2_STAT2_REG_OFFSET },
    0,
    0,
    468,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    FPM_POOL2_STAT2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_POOL2_STAT3, TYPE: Type_FPM_BLOCK_FPM_CTRL_POOL2_STAT3
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: NUM_OF_NOT_VALID_TOKEN_FREES *****/
const ru_field_rec FPM_POOL2_STAT3_NUM_OF_NOT_VALID_TOKEN_FREES_FIELD =
{
    "NUM_OF_NOT_VALID_TOKEN_FREES",
#if RU_INCLUDE_DESC
    "",
    "Count of de-allocate token requests with invalid tokens. For more information on conditions under which this counter is incremented, refer to POOL1_INTR_STS register (offset 0x14) bit[3] explanation in this document.\n\n",
#endif
    { FPM_POOL2_STAT3_NUM_OF_NOT_VALID_TOKEN_FREES_FIELD_MASK },
    0,
    { FPM_POOL2_STAT3_NUM_OF_NOT_VALID_TOKEN_FREES_FIELD_WIDTH },
    { FPM_POOL2_STAT3_NUM_OF_NOT_VALID_TOKEN_FREES_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec FPM_POOL2_STAT3_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_POOL2_STAT3_R1_FIELD_MASK },
    0,
    { FPM_POOL2_STAT3_R1_FIELD_WIDTH },
    { FPM_POOL2_STAT3_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL2_STAT3_FIELDS[] =
{
    &FPM_POOL2_STAT3_NUM_OF_NOT_VALID_TOKEN_FREES_FIELD,
    &FPM_POOL2_STAT3_R1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_POOL2_STAT3 *****/
const ru_reg_rec FPM_POOL2_STAT3_REG =
{
    "POOL2_STAT3",
#if RU_INCLUDE_DESC
    "POOL2_STAT3 Register",
    "This read only register allows software to read the count of free token requests with in-valid tokens When the counter values reaches maximum count, it will hold the max value and not increment the count value unless it is cleared. Any write to this register will clear count value.\n\n",
#endif
    { FPM_POOL2_STAT3_REG_OFFSET },
    0,
    0,
    469,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_POOL2_STAT3_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_POOL2_STAT4, TYPE: Type_FPM_BLOCK_FPM_CTRL_POOL2_STAT4
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: NUM_OF_NOT_VALID_TOKEN_MULTI *****/
const ru_field_rec FPM_POOL2_STAT4_NUM_OF_NOT_VALID_TOKEN_MULTI_FIELD =
{
    "NUM_OF_NOT_VALID_TOKEN_MULTI",
#if RU_INCLUDE_DESC
    "",
    "Count of multi-cast token update requests with either valid bit not set, For more information on conditions under which this counter is incremented, refer to POOL1_INTR_STS register (offset 0x14) bit[5] explanation in this document.\n\n",
#endif
    { FPM_POOL2_STAT4_NUM_OF_NOT_VALID_TOKEN_MULTI_FIELD_MASK },
    0,
    { FPM_POOL2_STAT4_NUM_OF_NOT_VALID_TOKEN_MULTI_FIELD_WIDTH },
    { FPM_POOL2_STAT4_NUM_OF_NOT_VALID_TOKEN_MULTI_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec FPM_POOL2_STAT4_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_POOL2_STAT4_R1_FIELD_MASK },
    0,
    { FPM_POOL2_STAT4_R1_FIELD_WIDTH },
    { FPM_POOL2_STAT4_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL2_STAT4_FIELDS[] =
{
    &FPM_POOL2_STAT4_NUM_OF_NOT_VALID_TOKEN_MULTI_FIELD,
    &FPM_POOL2_STAT4_R1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_POOL2_STAT4 *****/
const ru_reg_rec FPM_POOL2_STAT4_REG =
{
    "POOL2_STAT4",
#if RU_INCLUDE_DESC
    "POOL2_STAT4 Register",
    "This read only register allows software to read the count of multi-cast token update requests with in-valid tokens. When the counter values reaches maximum count, it will hold the max value and not increment the count value unless it is cleared. Any write to this register will clear count value.\n\n",
#endif
    { FPM_POOL2_STAT4_REG_OFFSET },
    0,
    0,
    470,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_POOL2_STAT4_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_POOL2_STAT5, TYPE: Type_FPM_BLOCK_FPM_CTRL_POOL2_STAT5
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN *****/
const ru_field_rec FPM_POOL2_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_FIELD =
{
    "MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN",
#if RU_INCLUDE_DESC
    "",
    "Token that causes memory corrupt interrupt active. If there are multiple tokens that causes this error, only the first one is captured. To capture successive tokens that causes the error this register should be cleared by writing any random value, in addition, memory corrupt status bit (bit[8]) in interrupt status register 0x14 should be cleared. Bitmap for these bits is shown below (reserved bits are zeros)\nBit[30]    - Reserved\nBit[29:12] - Token\nBit[11:0]  - Buffer size in bytes\n\n",
#endif
    { FPM_POOL2_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_FIELD_MASK },
    0,
    { FPM_POOL2_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_FIELD_WIDTH },
    { FPM_POOL2_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL2_STAT5_FIELDS[] =
{
    &FPM_POOL2_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_POOL2_STAT5 *****/
const ru_reg_rec FPM_POOL2_STAT5_REG =
{
    "POOL2_STAT5",
#if RU_INCLUDE_DESC
    "POOL2_STAT5 Register",
    "This read only register allows software to read the alloc token that causes memory corrupt interrupt (intr[8]) to go active. This is for debug purposes only. Any write to this register will clear token value (makes all bits one).\n\n",
#endif
    { FPM_POOL2_STAT5_REG_OFFSET },
    0,
    0,
    471,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_POOL2_STAT5_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_POOL2_STAT6, TYPE: Type_FPM_BLOCK_FPM_CTRL_POOL2_STAT6
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: INVALID_FREE_TOKEN *****/
const ru_field_rec FPM_POOL2_STAT6_INVALID_FREE_TOKEN_FIELD =
{
    "INVALID_FREE_TOKEN",
#if RU_INCLUDE_DESC
    "",
    "Token that causes intr[3] or intr[4] active. If there are multiple tokens that causes this error, only the first one is captured. To capture successive tokens that causes the error this register should be cleared by writing any random value. Bitmap for these bits is shown below (reserved bits are either zeros or can reflect the length of the packet associated with the freed token)\nBit[30]    - Reserved\nBit[29:12] - Token\nBit[11:0]  - Reserved\n\n",
#endif
    { FPM_POOL2_STAT6_INVALID_FREE_TOKEN_FIELD_MASK },
    0,
    { FPM_POOL2_STAT6_INVALID_FREE_TOKEN_FIELD_WIDTH },
    { FPM_POOL2_STAT6_INVALID_FREE_TOKEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL2_STAT6_FIELDS[] =
{
    &FPM_POOL2_STAT6_INVALID_FREE_TOKEN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_POOL2_STAT6 *****/
const ru_reg_rec FPM_POOL2_STAT6_REG =
{
    "POOL2_STAT6",
#if RU_INCLUDE_DESC
    "POOL2_STAT6 Register",
    "This read only register allows software to read the free token that causes invalid free request or free token with index out-of-range interrupts (intr[3] or intr[4]) to go active. This is for debug purposes only. Any write to this register will clear token value (makes all bits zero).\n\n",
#endif
    { FPM_POOL2_STAT6_REG_OFFSET },
    0,
    0,
    472,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_POOL2_STAT6_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_POOL2_STAT7, TYPE: Type_FPM_BLOCK_FPM_CTRL_POOL2_STAT7
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: INVALID_MCAST_TOKEN *****/
const ru_field_rec FPM_POOL2_STAT7_INVALID_MCAST_TOKEN_FIELD =
{
    "INVALID_MCAST_TOKEN",
#if RU_INCLUDE_DESC
    "",
    "Token that causes intr[5] or intr[6] active. If there are multiple tokens that causes this error, only the first one is captured. To capture successive tokens that causes the error this register should be cleared by writing any random value. Bitmap for these bits is shown below (reserved bits are zeros)\nBit[30]    - Reserved\nBit[29:12] - Token\nBit[11]    - Mcast update type (refer to register 0x224[11])\nBit[10:7]  - Reserved\nBit[6:0]   - Mcast value\n\n",
#endif
    { FPM_POOL2_STAT7_INVALID_MCAST_TOKEN_FIELD_MASK },
    0,
    { FPM_POOL2_STAT7_INVALID_MCAST_TOKEN_FIELD_WIDTH },
    { FPM_POOL2_STAT7_INVALID_MCAST_TOKEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL2_STAT7_FIELDS[] =
{
    &FPM_POOL2_STAT7_INVALID_MCAST_TOKEN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_POOL2_STAT7 *****/
const ru_reg_rec FPM_POOL2_STAT7_REG =
{
    "POOL2_STAT7",
#if RU_INCLUDE_DESC
    "POOL2_STAT7 Register",
    "This read only register allows software to read the multi-cast token that causes invalid mcast request or mcast token with index out-of-range interrupts (intr[5] or intr[6]) to go active. This is for debug purposes only. Any write to this register will clear token value (makes all bits one).\n\n",
#endif
    { FPM_POOL2_STAT7_REG_OFFSET },
    0,
    0,
    473,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_POOL2_STAT7_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_POOL2_STAT8, TYPE: Type_FPM_BLOCK_FPM_CTRL_POOL2_STAT8
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKENS_AVAILABLE_LOW_WTMK *****/
const ru_field_rec FPM_POOL2_STAT8_TOKENS_AVAILABLE_LOW_WTMK_FIELD =
{
    "TOKENS_AVAILABLE_LOW_WTMK",
#if RU_INCLUDE_DESC
    "",
    "Lowest value the NUM_OF_TOKENS_AVAIL count has reached.\n\n",
#endif
    { FPM_POOL2_STAT8_TOKENS_AVAILABLE_LOW_WTMK_FIELD_MASK },
    0,
    { FPM_POOL2_STAT8_TOKENS_AVAILABLE_LOW_WTMK_FIELD_WIDTH },
    { FPM_POOL2_STAT8_TOKENS_AVAILABLE_LOW_WTMK_FIELD_SHIFT },
    65536,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec FPM_POOL2_STAT8_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_POOL2_STAT8_R1_FIELD_MASK },
    0,
    { FPM_POOL2_STAT8_R1_FIELD_WIDTH },
    { FPM_POOL2_STAT8_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL2_STAT8_FIELDS[] =
{
    &FPM_POOL2_STAT8_TOKENS_AVAILABLE_LOW_WTMK_FIELD,
    &FPM_POOL2_STAT8_R1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_POOL2_STAT8 *****/
const ru_reg_rec FPM_POOL2_STAT8_REG =
{
    "POOL2_STAT8",
#if RU_INCLUDE_DESC
    "POOL2_STAT8 Register",
    "This register allows software to read the lowest value the NUM_OF_TOKENS_AVAILABLE count reached since the last time it was cleared. Any write to this register will reset the value back to the maximum number of tokens (0x100000)\n\n",
#endif
    { FPM_POOL2_STAT8_REG_OFFSET },
    0,
    0,
    474,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_POOL2_STAT8_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_POOL3_STAT1, TYPE: Type_FPM_BLOCK_FPM_CTRL_POOL3_STAT1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: UNDRFL *****/
const ru_field_rec FPM_POOL3_STAT1_UNDRFL_FIELD =
{
    "UNDRFL",
#if RU_INCLUDE_DESC
    "",
    "Free Pool underflow count\n\n",
#endif
    { FPM_POOL3_STAT1_UNDRFL_FIELD_MASK },
    0,
    { FPM_POOL3_STAT1_UNDRFL_FIELD_WIDTH },
    { FPM_POOL3_STAT1_UNDRFL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: OVRFL *****/
const ru_field_rec FPM_POOL3_STAT1_OVRFL_FIELD =
{
    "OVRFL",
#if RU_INCLUDE_DESC
    "",
    "Free Pool overflow count\n\n",
#endif
    { FPM_POOL3_STAT1_OVRFL_FIELD_MASK },
    0,
    { FPM_POOL3_STAT1_OVRFL_FIELD_WIDTH },
    { FPM_POOL3_STAT1_OVRFL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL3_STAT1_FIELDS[] =
{
    &FPM_POOL3_STAT1_UNDRFL_FIELD,
    &FPM_POOL3_STAT1_OVRFL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_POOL3_STAT1 *****/
const ru_reg_rec FPM_POOL3_STAT1_REG =
{
    "POOL3_STAT1",
#if RU_INCLUDE_DESC
    "POOL3_STAT1 Register",
    "This read only register allows software to read the count of free pool overflows and underflows. A overflow condition occurs when pool is empty, ie., no tokens are allocated and free/mcast request is encountered. A underflow condition occurs when pool is full, ie., there are no free tokens and a allocation request is encountered. When the counter values reaches maximum count, it will hold the max value and not increment the count value unless it is cleared. Any write to this register will clear both both counters.\n\n",
#endif
    { FPM_POOL3_STAT1_REG_OFFSET },
    0,
    0,
    475,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_POOL3_STAT1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_POOL3_STAT2, TYPE: Type_FPM_BLOCK_FPM_CTRL_POOL3_STAT2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: NUM_OF_TOKENS_AVAILABLE *****/
const ru_field_rec FPM_POOL3_STAT2_NUM_OF_TOKENS_AVAILABLE_FIELD =
{
    "NUM_OF_TOKENS_AVAILABLE",
#if RU_INCLUDE_DESC
    "",
    "Count of tokens available for allocation.\nThis provides a count of number of free tokens that available for allocation in the usage array. This value is updated instantaneously as tokens are allocated or freed from the array.\n\n",
#endif
    { FPM_POOL3_STAT2_NUM_OF_TOKENS_AVAILABLE_FIELD_MASK },
    0,
    { FPM_POOL3_STAT2_NUM_OF_TOKENS_AVAILABLE_FIELD_WIDTH },
    { FPM_POOL3_STAT2_NUM_OF_TOKENS_AVAILABLE_FIELD_SHIFT },
    65536,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R2 *****/
const ru_field_rec FPM_POOL3_STAT2_R2_FIELD =
{
    "R2",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_POOL3_STAT2_R2_FIELD_MASK },
    0,
    { FPM_POOL3_STAT2_R2_FIELD_WIDTH },
    { FPM_POOL3_STAT2_R2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ALLOC_FIFO_EMPTY *****/
const ru_field_rec FPM_POOL3_STAT2_ALLOC_FIFO_EMPTY_FIELD =
{
    "ALLOC_FIFO_EMPTY",
#if RU_INCLUDE_DESC
    "",
    "ALLOC_FIFO is empty.\n\n",
#endif
    { FPM_POOL3_STAT2_ALLOC_FIFO_EMPTY_FIELD_MASK },
    0,
    { FPM_POOL3_STAT2_ALLOC_FIFO_EMPTY_FIELD_WIDTH },
    { FPM_POOL3_STAT2_ALLOC_FIFO_EMPTY_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ALLOC_FIFO_FULL *****/
const ru_field_rec FPM_POOL3_STAT2_ALLOC_FIFO_FULL_FIELD =
{
    "ALLOC_FIFO_FULL",
#if RU_INCLUDE_DESC
    "",
    "ALLOC_FIFO is full\n\n",
#endif
    { FPM_POOL3_STAT2_ALLOC_FIFO_FULL_FIELD_MASK },
    0,
    { FPM_POOL3_STAT2_ALLOC_FIFO_FULL_FIELD_WIDTH },
    { FPM_POOL3_STAT2_ALLOC_FIFO_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_FIFO_EMPTY *****/
const ru_field_rec FPM_POOL3_STAT2_FREE_FIFO_EMPTY_FIELD =
{
    "FREE_FIFO_EMPTY",
#if RU_INCLUDE_DESC
    "",
    "FREE_FIFO is empty\n\n",
#endif
    { FPM_POOL3_STAT2_FREE_FIFO_EMPTY_FIELD_MASK },
    0,
    { FPM_POOL3_STAT2_FREE_FIFO_EMPTY_FIELD_WIDTH },
    { FPM_POOL3_STAT2_FREE_FIFO_EMPTY_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FREE_FIFO_FULL *****/
const ru_field_rec FPM_POOL3_STAT2_FREE_FIFO_FULL_FIELD =
{
    "FREE_FIFO_FULL",
#if RU_INCLUDE_DESC
    "",
    "FREE_FIFO is full.\n\n",
#endif
    { FPM_POOL3_STAT2_FREE_FIFO_FULL_FIELD_MASK },
    0,
    { FPM_POOL3_STAT2_FREE_FIFO_FULL_FIELD_WIDTH },
    { FPM_POOL3_STAT2_FREE_FIFO_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec FPM_POOL3_STAT2_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_POOL3_STAT2_R1_FIELD_MASK },
    0,
    { FPM_POOL3_STAT2_R1_FIELD_WIDTH },
    { FPM_POOL3_STAT2_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_FULL *****/
const ru_field_rec FPM_POOL3_STAT2_POOL_FULL_FIELD =
{
    "POOL_FULL",
#if RU_INCLUDE_DESC
    "",
    "POOL is full\nThis indicates that all tokens have been allocated and there no free tokens available. This bit will be active as long as all usage array is fully allocated.\n\n",
#endif
    { FPM_POOL3_STAT2_POOL_FULL_FIELD_MASK },
    0,
    { FPM_POOL3_STAT2_POOL_FULL_FIELD_WIDTH },
    { FPM_POOL3_STAT2_POOL_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL3_STAT2_FIELDS[] =
{
    &FPM_POOL3_STAT2_NUM_OF_TOKENS_AVAILABLE_FIELD,
    &FPM_POOL3_STAT2_R2_FIELD,
    &FPM_POOL3_STAT2_ALLOC_FIFO_EMPTY_FIELD,
    &FPM_POOL3_STAT2_ALLOC_FIFO_FULL_FIELD,
    &FPM_POOL3_STAT2_FREE_FIFO_EMPTY_FIELD,
    &FPM_POOL3_STAT2_FREE_FIFO_FULL_FIELD,
    &FPM_POOL3_STAT2_R1_FIELD,
    &FPM_POOL3_STAT2_POOL_FULL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_POOL3_STAT2 *****/
const ru_reg_rec FPM_POOL3_STAT2_REG =
{
    "POOL3_STAT2",
#if RU_INCLUDE_DESC
    "POOL3_STAT2 Register",
    "This read only register provide status of index memory, alloc & free cache/fifos. These are real time statuses and bits are not sticky. Write to any bits will have no effect.\n\n",
#endif
    { FPM_POOL3_STAT2_REG_OFFSET },
    0,
    0,
    476,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    FPM_POOL3_STAT2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_POOL3_STAT3, TYPE: Type_FPM_BLOCK_FPM_CTRL_POOL3_STAT3
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: NUM_OF_NOT_VALID_TOKEN_FREES *****/
const ru_field_rec FPM_POOL3_STAT3_NUM_OF_NOT_VALID_TOKEN_FREES_FIELD =
{
    "NUM_OF_NOT_VALID_TOKEN_FREES",
#if RU_INCLUDE_DESC
    "",
    "Count of de-allocate token requests with invalid tokens. For more information on conditions under which this counter is incremented, refer to POOL1_INTR_STS register (offset 0x14) bit[3] explanation in this document.\n\n",
#endif
    { FPM_POOL3_STAT3_NUM_OF_NOT_VALID_TOKEN_FREES_FIELD_MASK },
    0,
    { FPM_POOL3_STAT3_NUM_OF_NOT_VALID_TOKEN_FREES_FIELD_WIDTH },
    { FPM_POOL3_STAT3_NUM_OF_NOT_VALID_TOKEN_FREES_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec FPM_POOL3_STAT3_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_POOL3_STAT3_R1_FIELD_MASK },
    0,
    { FPM_POOL3_STAT3_R1_FIELD_WIDTH },
    { FPM_POOL3_STAT3_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL3_STAT3_FIELDS[] =
{
    &FPM_POOL3_STAT3_NUM_OF_NOT_VALID_TOKEN_FREES_FIELD,
    &FPM_POOL3_STAT3_R1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_POOL3_STAT3 *****/
const ru_reg_rec FPM_POOL3_STAT3_REG =
{
    "POOL3_STAT3",
#if RU_INCLUDE_DESC
    "POOL3_STAT3 Register",
    "This read only register allows software to read the count of free token requests with in-valid tokens When the counter values reaches maximum count, it will hold the max value and not increment the count value unless it is cleared. Any write to this register will clear count value.\n\n",
#endif
    { FPM_POOL3_STAT3_REG_OFFSET },
    0,
    0,
    477,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_POOL3_STAT3_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_POOL3_STAT5, TYPE: Type_FPM_BLOCK_FPM_CTRL_POOL3_STAT5
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN *****/
const ru_field_rec FPM_POOL3_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_FIELD =
{
    "MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN",
#if RU_INCLUDE_DESC
    "",
    "Token that causes memory corrupt interrupt active. If there are multiple tokens that causes this error, only the first one is captured. To capture successive tokens that causes the error this register should be cleared by writing any random value, in addition, memory corrupt status bit (bit[8]) in interrupt status register 0x14 should be cleared. Bitmap for these bits is shown below (reserved bits are zeros)\nBit[30]    - Reserved\nBit[29:12] - Token\nBit[11:0]  - Buffer size in bytes\n\n",
#endif
    { FPM_POOL3_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_FIELD_MASK },
    0,
    { FPM_POOL3_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_FIELD_WIDTH },
    { FPM_POOL3_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL3_STAT5_FIELDS[] =
{
    &FPM_POOL3_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_POOL3_STAT5 *****/
const ru_reg_rec FPM_POOL3_STAT5_REG =
{
    "POOL3_STAT5",
#if RU_INCLUDE_DESC
    "POOL3_STAT5 Register",
    "This read only register allows software to read the alloc token that causes memory corrupt interrupt (intr[8]) to go active. This is for debug purposes only. Any write to this register will clear token value (makes all bits one).\n\n",
#endif
    { FPM_POOL3_STAT5_REG_OFFSET },
    0,
    0,
    478,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_POOL3_STAT5_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_POOL3_STAT6, TYPE: Type_FPM_BLOCK_FPM_CTRL_POOL3_STAT6
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: INVALID_FREE_TOKEN *****/
const ru_field_rec FPM_POOL3_STAT6_INVALID_FREE_TOKEN_FIELD =
{
    "INVALID_FREE_TOKEN",
#if RU_INCLUDE_DESC
    "",
    "Token that causes intr[3] or intr[4] active. If there are multiple tokens that causes this error, only the first one is captured. To capture successive tokens that causes the error this register should be cleared by writing any random value. Bitmap for these bits is shown below (reserved bits are either zeros or can reflect the length of the packet associated with the freed token)\nBit[30]    - Reserved\nBit[29:12] - Token\nBit[11:0]  - Reserved\n\n",
#endif
    { FPM_POOL3_STAT6_INVALID_FREE_TOKEN_FIELD_MASK },
    0,
    { FPM_POOL3_STAT6_INVALID_FREE_TOKEN_FIELD_WIDTH },
    { FPM_POOL3_STAT6_INVALID_FREE_TOKEN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL3_STAT6_FIELDS[] =
{
    &FPM_POOL3_STAT6_INVALID_FREE_TOKEN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_POOL3_STAT6 *****/
const ru_reg_rec FPM_POOL3_STAT6_REG =
{
    "POOL3_STAT6",
#if RU_INCLUDE_DESC
    "POOL3_STAT6 Register",
    "This read only register allows software to read the free token that causes invalid free request or free token with index out-of-range interrupts (intr[3] or intr[4]) to go active. This is for debug purposes only. Any write to this register will clear token value (makes all bits zero).\n\n",
#endif
    { FPM_POOL3_STAT6_REG_OFFSET },
    0,
    0,
    479,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_POOL3_STAT6_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_POOL3_STAT8, TYPE: Type_FPM_BLOCK_FPM_CTRL_POOL3_STAT8
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKENS_AVAILABLE_LOW_WTMK *****/
const ru_field_rec FPM_POOL3_STAT8_TOKENS_AVAILABLE_LOW_WTMK_FIELD =
{
    "TOKENS_AVAILABLE_LOW_WTMK",
#if RU_INCLUDE_DESC
    "",
    "Lowest value the NUM_OF_TOKENS_AVAIL count has reached.\n\n",
#endif
    { FPM_POOL3_STAT8_TOKENS_AVAILABLE_LOW_WTMK_FIELD_MASK },
    0,
    { FPM_POOL3_STAT8_TOKENS_AVAILABLE_LOW_WTMK_FIELD_WIDTH },
    { FPM_POOL3_STAT8_TOKENS_AVAILABLE_LOW_WTMK_FIELD_SHIFT },
    65536,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec FPM_POOL3_STAT8_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_POOL3_STAT8_R1_FIELD_MASK },
    0,
    { FPM_POOL3_STAT8_R1_FIELD_WIDTH },
    { FPM_POOL3_STAT8_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL3_STAT8_FIELDS[] =
{
    &FPM_POOL3_STAT8_TOKENS_AVAILABLE_LOW_WTMK_FIELD,
    &FPM_POOL3_STAT8_R1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_POOL3_STAT8 *****/
const ru_reg_rec FPM_POOL3_STAT8_REG =
{
    "POOL3_STAT8",
#if RU_INCLUDE_DESC
    "POOL3_STAT8 Register",
    "This register allows software to read the lowest value the NUM_OF_TOKENS_AVAILABLE count reached since the last time it was cleared. Any write to this register will reset the value back to the maximum number of tokens (0x100000)\n\n",
#endif
    { FPM_POOL3_STAT8_REG_OFFSET },
    0,
    0,
    480,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_POOL3_STAT8_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_POOL1_XON_XOFF_CFG, TYPE: Type_FPM_BLOCK_FPM_CTRL_POOL1_XON_XOFF_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: XOFF_THRESHOLD *****/
const ru_field_rec FPM_POOL1_XON_XOFF_CFG_XOFF_THRESHOLD_FIELD =
{
    "XOFF_THRESHOLD",
#if RU_INCLUDE_DESC
    "",
    "XOFF Threshold value\nIndicates the lower threshold of available tokens at which XOFF condition is set.\nEach threshold value represents 256 buffers\n\n",
#endif
    { FPM_POOL1_XON_XOFF_CFG_XOFF_THRESHOLD_FIELD_MASK },
    0,
    { FPM_POOL1_XON_XOFF_CFG_XOFF_THRESHOLD_FIELD_WIDTH },
    { FPM_POOL1_XON_XOFF_CFG_XOFF_THRESHOLD_FIELD_SHIFT },
    48,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: XON_THRESHOLD *****/
const ru_field_rec FPM_POOL1_XON_XOFF_CFG_XON_THRESHOLD_FIELD =
{
    "XON_THRESHOLD",
#if RU_INCLUDE_DESC
    "",
    "XON Threshold value\nIndicates the upper threshold of available tokens at which XON condition is set.\nEach threshold value represents 256 buffers\n\n",
#endif
    { FPM_POOL1_XON_XOFF_CFG_XON_THRESHOLD_FIELD_MASK },
    0,
    { FPM_POOL1_XON_XOFF_CFG_XON_THRESHOLD_FIELD_WIDTH },
    { FPM_POOL1_XON_XOFF_CFG_XON_THRESHOLD_FIELD_SHIFT },
    64,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL1_XON_XOFF_CFG_FIELDS[] =
{
    &FPM_POOL1_XON_XOFF_CFG_XOFF_THRESHOLD_FIELD,
    &FPM_POOL1_XON_XOFF_CFG_XON_THRESHOLD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_POOL1_XON_XOFF_CFG *****/
const ru_reg_rec FPM_POOL1_XON_XOFF_CFG_REG =
{
    "POOL1_XON_XOFF_CFG",
#if RU_INCLUDE_DESC
    "POOL1_XON_XOFF_CFG Register",
    "Units(resolution) of 256 tokens\n",
#endif
    { FPM_POOL1_XON_XOFF_CFG_REG_OFFSET },
    0,
    0,
    481,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_POOL1_XON_XOFF_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_FPM_NOT_EMPTY_CFG, TYPE: Type_FPM_BLOCK_FPM_CTRL_FPM_NOT_EMPTY_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: NOT_EMPTY_THRESHOLD *****/
const ru_field_rec FPM_FPM_NOT_EMPTY_CFG_NOT_EMPTY_THRESHOLD_FIELD =
{
    "NOT_EMPTY_THRESHOLD",
#if RU_INCLUDE_DESC
    "",
    "Threshold value for reasserting pool_not_empty to FPM_BB\n\n",
#endif
    { FPM_FPM_NOT_EMPTY_CFG_NOT_EMPTY_THRESHOLD_FIELD_MASK },
    0,
    { FPM_FPM_NOT_EMPTY_CFG_NOT_EMPTY_THRESHOLD_FIELD_WIDTH },
    { FPM_FPM_NOT_EMPTY_CFG_NOT_EMPTY_THRESHOLD_FIELD_SHIFT },
    8,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec FPM_FPM_NOT_EMPTY_CFG_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_FPM_NOT_EMPTY_CFG_R1_FIELD_MASK },
    0,
    { FPM_FPM_NOT_EMPTY_CFG_R1_FIELD_WIDTH },
    { FPM_FPM_NOT_EMPTY_CFG_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_FPM_NOT_EMPTY_CFG_FIELDS[] =
{
    &FPM_FPM_NOT_EMPTY_CFG_NOT_EMPTY_THRESHOLD_FIELD,
    &FPM_FPM_NOT_EMPTY_CFG_R1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_FPM_NOT_EMPTY_CFG *****/
const ru_reg_rec FPM_FPM_NOT_EMPTY_CFG_REG =
{
    "FPM_NOT_EMPTY_CFG",
#if RU_INCLUDE_DESC
    "FPM_NOT_EMPTY_CFG Register",
    "No Description\n",
#endif
    { FPM_FPM_NOT_EMPTY_CFG_REG_OFFSET },
    0,
    0,
    482,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_FPM_NOT_EMPTY_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_MEM_CTL, TYPE: Type_FPM_BLOCK_FPM_CTRL_MEM_CTL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: R2 *****/
const ru_field_rec FPM_MEM_CTL_R2_FIELD =
{
    "R2",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_MEM_CTL_R2_FIELD_MASK },
    0,
    { FPM_MEM_CTL_R2_FIELD_WIDTH },
    { FPM_MEM_CTL_R2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MEM_ADDR *****/
const ru_field_rec FPM_MEM_CTL_MEM_ADDR_FIELD =
{
    "MEM_ADDR",
#if RU_INCLUDE_DESC
    "",
    "Memory address for write/read location\nThis is DWord aligned address\n\n",
#endif
    { FPM_MEM_CTL_MEM_ADDR_FIELD_MASK },
    0,
    { FPM_MEM_CTL_MEM_ADDR_FIELD_WIDTH },
    { FPM_MEM_CTL_MEM_ADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec FPM_MEM_CTL_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_MEM_CTL_R1_FIELD_MASK },
    0,
    { FPM_MEM_CTL_R1_FIELD_WIDTH },
    { FPM_MEM_CTL_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MEM_SEL *****/
const ru_field_rec FPM_MEM_CTL_MEM_SEL_FIELD =
{
    "MEM_SEL",
#if RU_INCLUDE_DESC
    "",
    "2'b00 = Reserved\n2'b01 = FPM Memory\n2'b10 = Reserved\n2'b11 = When memory is enabled, bit[31]=1, this value will allow a write to NUM_OF_TOKENS_AVAILABLE field [21:0] in POOL1_STAT2 register (offset 0x54). This should be used for debug purposes only\n\n",
#endif
    { FPM_MEM_CTL_MEM_SEL_FIELD_MASK },
    0,
    { FPM_MEM_CTL_MEM_SEL_FIELD_WIDTH },
    { FPM_MEM_CTL_MEM_SEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MEM_RD *****/
const ru_field_rec FPM_MEM_CTL_MEM_RD_FIELD =
{
    "MEM_RD",
#if RU_INCLUDE_DESC
    "",
    "Read control bit for Usage index array memory. This is a self clearing bit, cleared by hardware to zero once memory read is  complete. Software can read more locations if the bit value is zero\n\n",
#endif
    { FPM_MEM_CTL_MEM_RD_FIELD_MASK },
    0,
    { FPM_MEM_CTL_MEM_RD_FIELD_WIDTH },
    { FPM_MEM_CTL_MEM_RD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MEM_WR *****/
const ru_field_rec FPM_MEM_CTL_MEM_WR_FIELD =
{
    "MEM_WR",
#if RU_INCLUDE_DESC
    "",
    "Write control bit for Usage index array memory. This is a self clearing bit, cleared by hardware to zero once memory write is  complete. Software can write more locations if the bit value is zero\n\n",
#endif
    { FPM_MEM_CTL_MEM_WR_FIELD_MASK },
    0,
    { FPM_MEM_CTL_MEM_WR_FIELD_WIDTH },
    { FPM_MEM_CTL_MEM_WR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_MEM_CTL_FIELDS[] =
{
    &FPM_MEM_CTL_R2_FIELD,
    &FPM_MEM_CTL_MEM_ADDR_FIELD,
    &FPM_MEM_CTL_R1_FIELD,
    &FPM_MEM_CTL_MEM_SEL_FIELD,
    &FPM_MEM_CTL_MEM_RD_FIELD,
    &FPM_MEM_CTL_MEM_WR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_MEM_CTL *****/
const ru_reg_rec FPM_MEM_CTL_REG =
{
    "MEM_CTL",
#if RU_INCLUDE_DESC
    "MEM_CTL Register",
    "No Description\n",
#endif
    { FPM_MEM_CTL_REG_OFFSET },
    0,
    0,
    483,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    FPM_MEM_CTL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_MEM_DATA1, TYPE: Type_FPM_BLOCK_FPM_CTRL_MEM_DATA1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MEM_DATA1 *****/
const ru_field_rec FPM_MEM_DATA1_MEM_DATA1_FIELD =
{
    "MEM_DATA1",
#if RU_INCLUDE_DESC
    "",
    "Memory Data 1\nThis contains the lower 32 bits (bits[31:0]) of 32/64 bit data\n\n",
#endif
    { FPM_MEM_DATA1_MEM_DATA1_FIELD_MASK },
    0,
    { FPM_MEM_DATA1_MEM_DATA1_FIELD_WIDTH },
    { FPM_MEM_DATA1_MEM_DATA1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_MEM_DATA1_FIELDS[] =
{
    &FPM_MEM_DATA1_MEM_DATA1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_MEM_DATA1 *****/
const ru_reg_rec FPM_MEM_DATA1_REG =
{
    "MEM_DATA1",
#if RU_INCLUDE_DESC
    "MEM_DATA1 Register",
    "No Description\n",
#endif
    { FPM_MEM_DATA1_REG_OFFSET },
    0,
    0,
    484,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_MEM_DATA1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_MEM_DATA2, TYPE: Type_FPM_BLOCK_FPM_CTRL_MEM_DATA2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MEM_DATA2 *****/
const ru_field_rec FPM_MEM_DATA2_MEM_DATA2_FIELD =
{
    "MEM_DATA2",
#if RU_INCLUDE_DESC
    "",
    "Memory Data 2\nThis contains the upper 32 bits (bits[63:32]) of 64 bit data. The value in this register should be ignored during 32 bit access\n\n",
#endif
    { FPM_MEM_DATA2_MEM_DATA2_FIELD_MASK },
    0,
    { FPM_MEM_DATA2_MEM_DATA2_FIELD_WIDTH },
    { FPM_MEM_DATA2_MEM_DATA2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_MEM_DATA2_FIELDS[] =
{
    &FPM_MEM_DATA2_MEM_DATA2_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_MEM_DATA2 *****/
const ru_reg_rec FPM_MEM_DATA2_REG =
{
    "MEM_DATA2",
#if RU_INCLUDE_DESC
    "MEM_DATA2 Register",
    "No Description\n",
#endif
    { FPM_MEM_DATA2_REG_OFFSET },
    0,
    0,
    485,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_MEM_DATA2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_SPARE, TYPE: Type_FPM_BLOCK_FPM_CTRL_SPARE
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
    486,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_SPARE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_TOKEN_RECOVER_CTL, TYPE: Type_FPM_BLOCK_FPM_CTRL_TOKEN_RECOVER_CTL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN_RECOVER_ENA *****/
const ru_field_rec FPM_TOKEN_RECOVER_CTL_TOKEN_RECOVER_ENA_FIELD =
{
    "TOKEN_RECOVER_ENA",
#if RU_INCLUDE_DESC
    "",
    "Token recovery enable\n1 = Enabled\n0 = Disabled\n\n\n",
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
    "If token recovery is enabled, the single-pass control will indicate whether the hardware should perform just one iteration of the token recovery process or will continuously loop through the token recovery process.\n1 = Single pass\n0 = Auto repeat\n\n\n",
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
    "Enable remarking of tokens for multiple passes through the token recovery process. The mark bit is set on all tokens on the first pass through the loop. When this bit is set, the mark bits will be set again on all subsequent passes through the loop. It is anticipated that this bit will always be set when token recovery is enabled. It is provided as a potential debug tool.\n1 = Enabled\n0 = Disabled\n\n\n",
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
    "Enable automatic return of marked tokens to the freepool\n1 = Enabled\n0 = Disabled\n\n\n",
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
    "Non-automated token recovery.\nThis bit can be used when automatic token return is not enabled. When software gets an interrupt indicating that the token recovery process has detected expired tokens, it can set this bit to force the expired tokens to be reclaimed.\n1 = Enabled\n0 = Disabled\n\n\n",
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
    487,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    FPM_TOKEN_RECOVER_CTL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_SHORT_AGING_TIMER, TYPE: Type_FPM_BLOCK_FPM_CTRL_SHORT_AGING_TIMER
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
    488,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_SHORT_AGING_TIMER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_LONG_AGING_TIMER, TYPE: Type_FPM_BLOCK_FPM_CTRL_LONG_AGING_TIMER
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
    489,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_LONG_AGING_TIMER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_CACHE_RECYCLE_TIMER, TYPE: Type_FPM_BLOCK_FPM_CTRL_CACHE_RECYCLE_TIMER
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
    490,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_CACHE_RECYCLE_TIMER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_EXPIRED_TOKEN_COUNT_POOL1, TYPE: Type_FPM_BLOCK_FPM_CTRL_EXPIRED_TOKEN_COUNT_POOL1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec FPM_EXPIRED_TOKEN_COUNT_POOL1_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Cumulative count of the number of expired tokens detected in the token recovery process. The count can be cleared by setting the CLR_EXPIRED_TOKEN_COUNT in the TOKEN_RECOVER_CTL register\n\n",
#endif
    { FPM_EXPIRED_TOKEN_COUNT_POOL1_COUNT_FIELD_MASK },
    0,
    { FPM_EXPIRED_TOKEN_COUNT_POOL1_COUNT_FIELD_WIDTH },
    { FPM_EXPIRED_TOKEN_COUNT_POOL1_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_EXPIRED_TOKEN_COUNT_POOL1_FIELDS[] =
{
    &FPM_EXPIRED_TOKEN_COUNT_POOL1_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_EXPIRED_TOKEN_COUNT_POOL1 *****/
const ru_reg_rec FPM_EXPIRED_TOKEN_COUNT_POOL1_REG =
{
    "EXPIRED_TOKEN_COUNT_POOL1",
#if RU_INCLUDE_DESC
    "EXPIRED_TOKEN_COUNT_POOL1 Register",
    "No Description\n",
#endif
    { FPM_EXPIRED_TOKEN_COUNT_POOL1_REG_OFFSET },
    0,
    0,
    491,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_EXPIRED_TOKEN_COUNT_POOL1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_RECOVERED_TOKEN_COUNT_POOL1, TYPE: Type_FPM_BLOCK_FPM_CTRL_RECOVERED_TOKEN_COUNT_POOL1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec FPM_RECOVERED_TOKEN_COUNT_POOL1_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Cumulative count of the number of expired tokens that were freed in the token recovery process. The count can be cleared by setting the CLR_RECOVERED_TOKEN_COUNT in the TOKEN_RECOVER_CTL register\n\n",
#endif
    { FPM_RECOVERED_TOKEN_COUNT_POOL1_COUNT_FIELD_MASK },
    0,
    { FPM_RECOVERED_TOKEN_COUNT_POOL1_COUNT_FIELD_WIDTH },
    { FPM_RECOVERED_TOKEN_COUNT_POOL1_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_RECOVERED_TOKEN_COUNT_POOL1_FIELDS[] =
{
    &FPM_RECOVERED_TOKEN_COUNT_POOL1_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_RECOVERED_TOKEN_COUNT_POOL1 *****/
const ru_reg_rec FPM_RECOVERED_TOKEN_COUNT_POOL1_REG =
{
    "RECOVERED_TOKEN_COUNT_POOL1",
#if RU_INCLUDE_DESC
    "RECOVERED_TOKEN_COUNT_POOL1 Register",
    "No Description\n",
#endif
    { FPM_RECOVERED_TOKEN_COUNT_POOL1_REG_OFFSET },
    0,
    0,
    492,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_RECOVERED_TOKEN_COUNT_POOL1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_EXPIRED_TOKEN_COUNT_POOL2, TYPE: Type_FPM_BLOCK_FPM_CTRL_EXPIRED_TOKEN_COUNT_POOL2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec FPM_EXPIRED_TOKEN_COUNT_POOL2_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Cumulative count of the number of expired tokens detected in the token recovery process. The count can be cleared by setting the CLR_EXPIRED_TOKEN_COUNT in the TOKEN_RECOVER_CTL register\n\n",
#endif
    { FPM_EXPIRED_TOKEN_COUNT_POOL2_COUNT_FIELD_MASK },
    0,
    { FPM_EXPIRED_TOKEN_COUNT_POOL2_COUNT_FIELD_WIDTH },
    { FPM_EXPIRED_TOKEN_COUNT_POOL2_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_EXPIRED_TOKEN_COUNT_POOL2_FIELDS[] =
{
    &FPM_EXPIRED_TOKEN_COUNT_POOL2_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_EXPIRED_TOKEN_COUNT_POOL2 *****/
const ru_reg_rec FPM_EXPIRED_TOKEN_COUNT_POOL2_REG =
{
    "EXPIRED_TOKEN_COUNT_POOL2",
#if RU_INCLUDE_DESC
    "EXPIRED_TOKEN_COUNT_POOL2 Register",
    "No Description\n",
#endif
    { FPM_EXPIRED_TOKEN_COUNT_POOL2_REG_OFFSET },
    0,
    0,
    493,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_EXPIRED_TOKEN_COUNT_POOL2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_RECOVERED_TOKEN_COUNT_POOL2, TYPE: Type_FPM_BLOCK_FPM_CTRL_RECOVERED_TOKEN_COUNT_POOL2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec FPM_RECOVERED_TOKEN_COUNT_POOL2_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Cumulative count of the number of expired tokens that were freed in the token recovery process. The count can be cleared by setting the CLR_RECOVERED_TOKEN_COUNT in the TOKEN_RECOVER_CTL register\n\n",
#endif
    { FPM_RECOVERED_TOKEN_COUNT_POOL2_COUNT_FIELD_MASK },
    0,
    { FPM_RECOVERED_TOKEN_COUNT_POOL2_COUNT_FIELD_WIDTH },
    { FPM_RECOVERED_TOKEN_COUNT_POOL2_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_RECOVERED_TOKEN_COUNT_POOL2_FIELDS[] =
{
    &FPM_RECOVERED_TOKEN_COUNT_POOL2_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_RECOVERED_TOKEN_COUNT_POOL2 *****/
const ru_reg_rec FPM_RECOVERED_TOKEN_COUNT_POOL2_REG =
{
    "RECOVERED_TOKEN_COUNT_POOL2",
#if RU_INCLUDE_DESC
    "RECOVERED_TOKEN_COUNT_POOL2 Register",
    "No Description\n",
#endif
    { FPM_RECOVERED_TOKEN_COUNT_POOL2_REG_OFFSET },
    0,
    0,
    494,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_RECOVERED_TOKEN_COUNT_POOL2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_TOKEN_RECOVER_START_END_POOL1, TYPE: Type_FPM_BLOCK_FPM_CTRL_TOKEN_RECOVER_START_END_POOL1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: END_INDEX *****/
const ru_field_rec FPM_TOKEN_RECOVER_START_END_POOL1_END_INDEX_FIELD =
{
    "END_INDEX",
#if RU_INCLUDE_DESC
    "",
    "End of token index range to be used when performing token recovery.\n\n",
#endif
    { FPM_TOKEN_RECOVER_START_END_POOL1_END_INDEX_FIELD_MASK },
    0,
    { FPM_TOKEN_RECOVER_START_END_POOL1_END_INDEX_FIELD_WIDTH },
    { FPM_TOKEN_RECOVER_START_END_POOL1_END_INDEX_FIELD_SHIFT },
    16383,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: START_INDEX *****/
const ru_field_rec FPM_TOKEN_RECOVER_START_END_POOL1_START_INDEX_FIELD =
{
    "START_INDEX",
#if RU_INCLUDE_DESC
    "",
    "Start of token index range to be used when performing token recovery.\n\n",
#endif
    { FPM_TOKEN_RECOVER_START_END_POOL1_START_INDEX_FIELD_MASK },
    0,
    { FPM_TOKEN_RECOVER_START_END_POOL1_START_INDEX_FIELD_WIDTH },
    { FPM_TOKEN_RECOVER_START_END_POOL1_START_INDEX_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_TOKEN_RECOVER_START_END_POOL1_FIELDS[] =
{
    &FPM_TOKEN_RECOVER_START_END_POOL1_END_INDEX_FIELD,
    &FPM_TOKEN_RECOVER_START_END_POOL1_START_INDEX_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_TOKEN_RECOVER_START_END_POOL1 *****/
const ru_reg_rec FPM_TOKEN_RECOVER_START_END_POOL1_REG =
{
    "TOKEN_RECOVER_START_END_POOL1",
#if RU_INCLUDE_DESC
    "TOKEN_RECOVER_START_END_POOL1 Register",
    "No Description\n",
#endif
    { FPM_TOKEN_RECOVER_START_END_POOL1_REG_OFFSET },
    0,
    0,
    495,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_TOKEN_RECOVER_START_END_POOL1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_TOKEN_RECOVER_START_END_POOL2, TYPE: Type_FPM_BLOCK_FPM_CTRL_TOKEN_RECOVER_START_END_POOL2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: END_INDEX *****/
const ru_field_rec FPM_TOKEN_RECOVER_START_END_POOL2_END_INDEX_FIELD =
{
    "END_INDEX",
#if RU_INCLUDE_DESC
    "",
    "End of token index range to be used when performing token recovery.\n\n",
#endif
    { FPM_TOKEN_RECOVER_START_END_POOL2_END_INDEX_FIELD_MASK },
    0,
    { FPM_TOKEN_RECOVER_START_END_POOL2_END_INDEX_FIELD_WIDTH },
    { FPM_TOKEN_RECOVER_START_END_POOL2_END_INDEX_FIELD_SHIFT },
    16383,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: START_INDEX *****/
const ru_field_rec FPM_TOKEN_RECOVER_START_END_POOL2_START_INDEX_FIELD =
{
    "START_INDEX",
#if RU_INCLUDE_DESC
    "",
    "Start of token index range to be used when performing token recovery.\n\n",
#endif
    { FPM_TOKEN_RECOVER_START_END_POOL2_START_INDEX_FIELD_MASK },
    0,
    { FPM_TOKEN_RECOVER_START_END_POOL2_START_INDEX_FIELD_WIDTH },
    { FPM_TOKEN_RECOVER_START_END_POOL2_START_INDEX_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_TOKEN_RECOVER_START_END_POOL2_FIELDS[] =
{
    &FPM_TOKEN_RECOVER_START_END_POOL2_END_INDEX_FIELD,
    &FPM_TOKEN_RECOVER_START_END_POOL2_START_INDEX_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_TOKEN_RECOVER_START_END_POOL2 *****/
const ru_reg_rec FPM_TOKEN_RECOVER_START_END_POOL2_REG =
{
    "TOKEN_RECOVER_START_END_POOL2",
#if RU_INCLUDE_DESC
    "TOKEN_RECOVER_START_END_POOL2 Register",
    "No Description\n",
#endif
    { FPM_TOKEN_RECOVER_START_END_POOL2_REG_OFFSET },
    0,
    0,
    496,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_TOKEN_RECOVER_START_END_POOL2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_PRBS_INVALID_GEN, TYPE: Type_FPM_BLOCK_FPM_CTRL_PRBS_INVALID_GEN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MASK *****/
const ru_field_rec FPM_PRBS_INVALID_GEN_MASK_FIELD =
{
    "MASK",
#if RU_INCLUDE_DESC
    "",
    "ORing bit mask for output PRBS signal.\n\n",
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
    "Enable PRBS invalid token generation.\n\n",
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
    497,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_PRBS_INVALID_GEN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_POOL1_ALLOC_DEALLOC, TYPE: Type_FPM_BLOCK_FPM_POOL_POOL1_ALLOC_DEALLOC
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN_SIZE *****/
const ru_field_rec FPM_POOL1_ALLOC_DEALLOC_TOKEN_SIZE_FIELD =
{
    "TOKEN_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Buffer length or packet size in bytes\n\n",
#endif
    { FPM_POOL1_ALLOC_DEALLOC_TOKEN_SIZE_FIELD_MASK },
    0,
    { FPM_POOL1_ALLOC_DEALLOC_TOKEN_SIZE_FIELD_WIDTH },
    { FPM_POOL1_ALLOC_DEALLOC_TOKEN_SIZE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN_INDEX *****/
const ru_field_rec FPM_POOL1_ALLOC_DEALLOC_TOKEN_INDEX_FIELD =
{
    "TOKEN_INDEX",
#if RU_INCLUDE_DESC
    "",
    "Buffer Index Pointer\n\n",
#endif
    { FPM_POOL1_ALLOC_DEALLOC_TOKEN_INDEX_FIELD_MASK },
    0,
    { FPM_POOL1_ALLOC_DEALLOC_TOKEN_INDEX_FIELD_WIDTH },
    { FPM_POOL1_ALLOC_DEALLOC_TOKEN_INDEX_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR *****/
const ru_field_rec FPM_POOL1_ALLOC_DEALLOC_DDR_FIELD =
{
    "DDR",
#if RU_INCLUDE_DESC
    "",
    "DDR Identifier\n0: DDR0\n1: DDR1\n\n",
#endif
    { FPM_POOL1_ALLOC_DEALLOC_DDR_FIELD_MASK },
    0,
    { FPM_POOL1_ALLOC_DEALLOC_DDR_FIELD_WIDTH },
    { FPM_POOL1_ALLOC_DEALLOC_DDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN_VALID *****/
const ru_field_rec FPM_POOL1_ALLOC_DEALLOC_TOKEN_VALID_FIELD =
{
    "TOKEN_VALID",
#if RU_INCLUDE_DESC
    "",
    "Valid Token Indicator\n0: No buffers available\n1: A valid token index is provided. If a token is de-allocated/freed without this bit set that causes an error and the token will be ignored, error counter in register offset 0xB8 will be incremented.\n\n",
#endif
    { FPM_POOL1_ALLOC_DEALLOC_TOKEN_VALID_FIELD_MASK },
    0,
    { FPM_POOL1_ALLOC_DEALLOC_TOKEN_VALID_FIELD_WIDTH },
    { FPM_POOL1_ALLOC_DEALLOC_TOKEN_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL1_ALLOC_DEALLOC_FIELDS[] =
{
    &FPM_POOL1_ALLOC_DEALLOC_TOKEN_SIZE_FIELD,
    &FPM_POOL1_ALLOC_DEALLOC_TOKEN_INDEX_FIELD,
    &FPM_POOL1_ALLOC_DEALLOC_DDR_FIELD,
    &FPM_POOL1_ALLOC_DEALLOC_TOKEN_VALID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_POOL1_ALLOC_DEALLOC *****/
const ru_reg_rec FPM_POOL1_ALLOC_DEALLOC_REG =
{
    "POOL1_ALLOC_DEALLOC",
#if RU_INCLUDE_DESC
    "POOL1_ALLOC_DEALLOC Register",
    "The free pool FIFO contains pointers to the buffers in the pool. To allocate a buffer from the pool, read token from this port. To de-allocate/free a buffer to the pool , write the token of the buffer to this port. After reset, software must initialize the FIFO. The buffer size is given in the control register above. All buffers must be of the same size and contiguous.\n\n",
#endif
    { FPM_POOL1_ALLOC_DEALLOC_REG_OFFSET },
    0,
    0,
    498,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    FPM_POOL1_ALLOC_DEALLOC_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_POOL2_ALLOC_DEALLOC, TYPE: Type_FPM_BLOCK_FPM_POOL_POOL2_ALLOC_DEALLOC
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN_SIZE *****/
const ru_field_rec FPM_POOL2_ALLOC_DEALLOC_TOKEN_SIZE_FIELD =
{
    "TOKEN_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Buffer length or packet size in bytes\n\n",
#endif
    { FPM_POOL2_ALLOC_DEALLOC_TOKEN_SIZE_FIELD_MASK },
    0,
    { FPM_POOL2_ALLOC_DEALLOC_TOKEN_SIZE_FIELD_WIDTH },
    { FPM_POOL2_ALLOC_DEALLOC_TOKEN_SIZE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN_INDEX *****/
const ru_field_rec FPM_POOL2_ALLOC_DEALLOC_TOKEN_INDEX_FIELD =
{
    "TOKEN_INDEX",
#if RU_INCLUDE_DESC
    "",
    "Buffer Index Pointer\n\n",
#endif
    { FPM_POOL2_ALLOC_DEALLOC_TOKEN_INDEX_FIELD_MASK },
    0,
    { FPM_POOL2_ALLOC_DEALLOC_TOKEN_INDEX_FIELD_WIDTH },
    { FPM_POOL2_ALLOC_DEALLOC_TOKEN_INDEX_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR *****/
const ru_field_rec FPM_POOL2_ALLOC_DEALLOC_DDR_FIELD =
{
    "DDR",
#if RU_INCLUDE_DESC
    "",
    "DDR Identifier\n0: DDR0\n1: DDR1\n\n",
#endif
    { FPM_POOL2_ALLOC_DEALLOC_DDR_FIELD_MASK },
    0,
    { FPM_POOL2_ALLOC_DEALLOC_DDR_FIELD_WIDTH },
    { FPM_POOL2_ALLOC_DEALLOC_DDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN_VALID *****/
const ru_field_rec FPM_POOL2_ALLOC_DEALLOC_TOKEN_VALID_FIELD =
{
    "TOKEN_VALID",
#if RU_INCLUDE_DESC
    "",
    "Valid Token Indicator\n0: No buffers available\n1: A valid token index is provided. If a token is de-allocated/freed without this bit set that causes an error and the token will be ignored, error counter in register offset 0xB8 will be incremented.\n\n",
#endif
    { FPM_POOL2_ALLOC_DEALLOC_TOKEN_VALID_FIELD_MASK },
    0,
    { FPM_POOL2_ALLOC_DEALLOC_TOKEN_VALID_FIELD_WIDTH },
    { FPM_POOL2_ALLOC_DEALLOC_TOKEN_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL2_ALLOC_DEALLOC_FIELDS[] =
{
    &FPM_POOL2_ALLOC_DEALLOC_TOKEN_SIZE_FIELD,
    &FPM_POOL2_ALLOC_DEALLOC_TOKEN_INDEX_FIELD,
    &FPM_POOL2_ALLOC_DEALLOC_DDR_FIELD,
    &FPM_POOL2_ALLOC_DEALLOC_TOKEN_VALID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_POOL2_ALLOC_DEALLOC *****/
const ru_reg_rec FPM_POOL2_ALLOC_DEALLOC_REG =
{
    "POOL2_ALLOC_DEALLOC",
#if RU_INCLUDE_DESC
    "POOL2_ALLOC_DEALLOC Register",
    "The free pool FIFO contains pointers to the buffers in the pool. To allocate a buffer from the pool, read token from this port. To de-allocate/free a buffer to the pool , write the token of the buffer to this port. After reset, software must initialize the FIFO. The buffer size is given in the control register above. All buffers must be of the same size and contiguous.\n\n",
#endif
    { FPM_POOL2_ALLOC_DEALLOC_REG_OFFSET },
    0,
    0,
    499,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    FPM_POOL2_ALLOC_DEALLOC_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_POOL3_ALLOC_DEALLOC, TYPE: Type_FPM_BLOCK_FPM_POOL_POOL3_ALLOC_DEALLOC
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN_SIZE *****/
const ru_field_rec FPM_POOL3_ALLOC_DEALLOC_TOKEN_SIZE_FIELD =
{
    "TOKEN_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Buffer length or packet size in bytes\n\n",
#endif
    { FPM_POOL3_ALLOC_DEALLOC_TOKEN_SIZE_FIELD_MASK },
    0,
    { FPM_POOL3_ALLOC_DEALLOC_TOKEN_SIZE_FIELD_WIDTH },
    { FPM_POOL3_ALLOC_DEALLOC_TOKEN_SIZE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN_INDEX *****/
const ru_field_rec FPM_POOL3_ALLOC_DEALLOC_TOKEN_INDEX_FIELD =
{
    "TOKEN_INDEX",
#if RU_INCLUDE_DESC
    "",
    "Buffer Index Pointer\n\n",
#endif
    { FPM_POOL3_ALLOC_DEALLOC_TOKEN_INDEX_FIELD_MASK },
    0,
    { FPM_POOL3_ALLOC_DEALLOC_TOKEN_INDEX_FIELD_WIDTH },
    { FPM_POOL3_ALLOC_DEALLOC_TOKEN_INDEX_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR *****/
const ru_field_rec FPM_POOL3_ALLOC_DEALLOC_DDR_FIELD =
{
    "DDR",
#if RU_INCLUDE_DESC
    "",
    "DDR Identifier\n0: DDR0\n1: DDR1\n\n",
#endif
    { FPM_POOL3_ALLOC_DEALLOC_DDR_FIELD_MASK },
    0,
    { FPM_POOL3_ALLOC_DEALLOC_DDR_FIELD_WIDTH },
    { FPM_POOL3_ALLOC_DEALLOC_DDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN_VALID *****/
const ru_field_rec FPM_POOL3_ALLOC_DEALLOC_TOKEN_VALID_FIELD =
{
    "TOKEN_VALID",
#if RU_INCLUDE_DESC
    "",
    "Valid Token Indicator\n0: No buffers available\n1: A valid token index is provided. If a token is de-allocated/freed without this bit set that causes an error and the token will be ignored, error counter in register offset 0xB8 will be incremented.\n\n",
#endif
    { FPM_POOL3_ALLOC_DEALLOC_TOKEN_VALID_FIELD_MASK },
    0,
    { FPM_POOL3_ALLOC_DEALLOC_TOKEN_VALID_FIELD_WIDTH },
    { FPM_POOL3_ALLOC_DEALLOC_TOKEN_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL3_ALLOC_DEALLOC_FIELDS[] =
{
    &FPM_POOL3_ALLOC_DEALLOC_TOKEN_SIZE_FIELD,
    &FPM_POOL3_ALLOC_DEALLOC_TOKEN_INDEX_FIELD,
    &FPM_POOL3_ALLOC_DEALLOC_DDR_FIELD,
    &FPM_POOL3_ALLOC_DEALLOC_TOKEN_VALID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_POOL3_ALLOC_DEALLOC *****/
const ru_reg_rec FPM_POOL3_ALLOC_DEALLOC_REG =
{
    "POOL3_ALLOC_DEALLOC",
#if RU_INCLUDE_DESC
    "POOL3_ALLOC_DEALLOC Register",
    "The free pool FIFO contains pointers to the buffers in the pool. To allocate a buffer from the pool, read token from this port. To de-allocate/free a buffer to the pool , write the token of the buffer to this port. After reset, software must initialize the FIFO. The buffer size is given in the control register above. All buffers must be of the same size and contiguous.\n\n",
#endif
    { FPM_POOL3_ALLOC_DEALLOC_REG_OFFSET },
    0,
    0,
    500,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    FPM_POOL3_ALLOC_DEALLOC_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_POOL4_ALLOC_DEALLOC, TYPE: Type_FPM_BLOCK_FPM_POOL_POOL4_ALLOC_DEALLOC
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN_SIZE *****/
const ru_field_rec FPM_POOL4_ALLOC_DEALLOC_TOKEN_SIZE_FIELD =
{
    "TOKEN_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Buffer length or packet size in bytes\n\n",
#endif
    { FPM_POOL4_ALLOC_DEALLOC_TOKEN_SIZE_FIELD_MASK },
    0,
    { FPM_POOL4_ALLOC_DEALLOC_TOKEN_SIZE_FIELD_WIDTH },
    { FPM_POOL4_ALLOC_DEALLOC_TOKEN_SIZE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN_INDEX *****/
const ru_field_rec FPM_POOL4_ALLOC_DEALLOC_TOKEN_INDEX_FIELD =
{
    "TOKEN_INDEX",
#if RU_INCLUDE_DESC
    "",
    "Buffer Index Pointer\n\n",
#endif
    { FPM_POOL4_ALLOC_DEALLOC_TOKEN_INDEX_FIELD_MASK },
    0,
    { FPM_POOL4_ALLOC_DEALLOC_TOKEN_INDEX_FIELD_WIDTH },
    { FPM_POOL4_ALLOC_DEALLOC_TOKEN_INDEX_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR *****/
const ru_field_rec FPM_POOL4_ALLOC_DEALLOC_DDR_FIELD =
{
    "DDR",
#if RU_INCLUDE_DESC
    "",
    "DDR Identifier\n0: DDR0\n1: DDR1\n\n",
#endif
    { FPM_POOL4_ALLOC_DEALLOC_DDR_FIELD_MASK },
    0,
    { FPM_POOL4_ALLOC_DEALLOC_DDR_FIELD_WIDTH },
    { FPM_POOL4_ALLOC_DEALLOC_DDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN_VALID *****/
const ru_field_rec FPM_POOL4_ALLOC_DEALLOC_TOKEN_VALID_FIELD =
{
    "TOKEN_VALID",
#if RU_INCLUDE_DESC
    "",
    "Valid Token Indicator\n0: No buffers available\n1: A valid token index is provided. If a token is de-allocated/freed without this bit set that causes an error and the token will be ignored, error counter in register offset 0xB8 will be incremented.\n\n",
#endif
    { FPM_POOL4_ALLOC_DEALLOC_TOKEN_VALID_FIELD_MASK },
    0,
    { FPM_POOL4_ALLOC_DEALLOC_TOKEN_VALID_FIELD_WIDTH },
    { FPM_POOL4_ALLOC_DEALLOC_TOKEN_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL4_ALLOC_DEALLOC_FIELDS[] =
{
    &FPM_POOL4_ALLOC_DEALLOC_TOKEN_SIZE_FIELD,
    &FPM_POOL4_ALLOC_DEALLOC_TOKEN_INDEX_FIELD,
    &FPM_POOL4_ALLOC_DEALLOC_DDR_FIELD,
    &FPM_POOL4_ALLOC_DEALLOC_TOKEN_VALID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_POOL4_ALLOC_DEALLOC *****/
const ru_reg_rec FPM_POOL4_ALLOC_DEALLOC_REG =
{
    "POOL4_ALLOC_DEALLOC",
#if RU_INCLUDE_DESC
    "POOL4_ALLOC_DEALLOC Register",
    "The free pool FIFO contains pointers to the buffers in the pool. To allocate a buffer from the pool, read token from this port. To de-allocate/free a buffer to the pool , write the token of the buffer to this port. After reset, software must initialize the FIFO. The buffer size is given in the control register above. All buffers must be of the same size and contiguous.\n\n",
#endif
    { FPM_POOL4_ALLOC_DEALLOC_REG_OFFSET },
    0,
    0,
    501,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    FPM_POOL4_ALLOC_DEALLOC_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_POOL_MULTI, TYPE: Type_FPM_BLOCK_FPM_POOL_POOL_MULTI
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN_MULTI *****/
const ru_field_rec FPM_POOL_MULTI_TOKEN_MULTI_FIELD =
{
    "TOKEN_MULTI",
#if RU_INCLUDE_DESC
    "",
    "New Multi-cast Value\n\n",
#endif
    { FPM_POOL_MULTI_TOKEN_MULTI_FIELD_MASK },
    0,
    { FPM_POOL_MULTI_TOKEN_MULTI_FIELD_WIDTH },
    { FPM_POOL_MULTI_TOKEN_MULTI_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_write,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R2 *****/
const ru_field_rec FPM_POOL_MULTI_R2_FIELD =
{
    "R2",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_POOL_MULTI_R2_FIELD_MASK },
    0,
    { FPM_POOL_MULTI_R2_FIELD_WIDTH },
    { FPM_POOL_MULTI_R2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: UPDATE_TYPE *****/
const ru_field_rec FPM_POOL_MULTI_UPDATE_TYPE_FIELD =
{
    "UPDATE_TYPE",
#if RU_INCLUDE_DESC
    "",
    "1'b0 - Count value is replaced with new value in bits[6:0]\n1'b1 - Count value is incremented by value in bits[6:0]\n\n",
#endif
    { FPM_POOL_MULTI_UPDATE_TYPE_FIELD_MASK },
    0,
    { FPM_POOL_MULTI_UPDATE_TYPE_FIELD_WIDTH },
    { FPM_POOL_MULTI_UPDATE_TYPE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_write,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN_INDEX *****/
const ru_field_rec FPM_POOL_MULTI_TOKEN_INDEX_FIELD =
{
    "TOKEN_INDEX",
#if RU_INCLUDE_DESC
    "",
    "Buffer Index Pointer\n\n",
#endif
    { FPM_POOL_MULTI_TOKEN_INDEX_FIELD_MASK },
    0,
    { FPM_POOL_MULTI_TOKEN_INDEX_FIELD_WIDTH },
    { FPM_POOL_MULTI_TOKEN_INDEX_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_write,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR *****/
const ru_field_rec FPM_POOL_MULTI_DDR_FIELD =
{
    "DDR",
#if RU_INCLUDE_DESC
    "",
    "DDR Identifier\n0: DDR0\n1: DDR1\n\n",
#endif
    { FPM_POOL_MULTI_DDR_FIELD_MASK },
    0,
    { FPM_POOL_MULTI_DDR_FIELD_WIDTH },
    { FPM_POOL_MULTI_DDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_write,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN_VALID *****/
const ru_field_rec FPM_POOL_MULTI_TOKEN_VALID_FIELD =
{
    "TOKEN_VALID",
#if RU_INCLUDE_DESC
    "",
    "Valid Token Indicator\n0: No buffers available\n1: A valid token index is provided. If a token multi-cast value is updated without this bit set, that causes an error and the token will be ignored, error counter in register offset 0xBC will be incremented.\n\n",
#endif
    { FPM_POOL_MULTI_TOKEN_VALID_FIELD_MASK },
    0,
    { FPM_POOL_MULTI_TOKEN_VALID_FIELD_WIDTH },
    { FPM_POOL_MULTI_TOKEN_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_write,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL_MULTI_FIELDS[] =
{
    &FPM_POOL_MULTI_TOKEN_MULTI_FIELD,
    &FPM_POOL_MULTI_R2_FIELD,
    &FPM_POOL_MULTI_UPDATE_TYPE_FIELD,
    &FPM_POOL_MULTI_TOKEN_INDEX_FIELD,
    &FPM_POOL_MULTI_DDR_FIELD,
    &FPM_POOL_MULTI_TOKEN_VALID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_POOL_MULTI *****/
const ru_reg_rec FPM_POOL_MULTI_REG =
{
    "POOL_MULTI",
#if RU_INCLUDE_DESC
    "POOL_MULTI Register",
    "Update/Modify the multi-cast value of the token\n\n",
#endif
    { FPM_POOL_MULTI_REG_OFFSET },
    0,
    0,
    502,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    FPM_POOL_MULTI_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_POOL1_ALLOC_DEALLOC_1, TYPE: Type_FPM_BLOCK_FPM_POOL_0_POOL1_ALLOC_DEALLOC
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN_SIZE *****/
const ru_field_rec FPM_POOL1_ALLOC_DEALLOC_1_TOKEN_SIZE_FIELD =
{
    "TOKEN_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Buffer length or packet size in bytes\n\n",
#endif
    { FPM_POOL1_ALLOC_DEALLOC_1_TOKEN_SIZE_FIELD_MASK },
    0,
    { FPM_POOL1_ALLOC_DEALLOC_1_TOKEN_SIZE_FIELD_WIDTH },
    { FPM_POOL1_ALLOC_DEALLOC_1_TOKEN_SIZE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN_INDEX *****/
const ru_field_rec FPM_POOL1_ALLOC_DEALLOC_1_TOKEN_INDEX_FIELD =
{
    "TOKEN_INDEX",
#if RU_INCLUDE_DESC
    "",
    "Buffer Index Pointer\n\n",
#endif
    { FPM_POOL1_ALLOC_DEALLOC_1_TOKEN_INDEX_FIELD_MASK },
    0,
    { FPM_POOL1_ALLOC_DEALLOC_1_TOKEN_INDEX_FIELD_WIDTH },
    { FPM_POOL1_ALLOC_DEALLOC_1_TOKEN_INDEX_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR *****/
const ru_field_rec FPM_POOL1_ALLOC_DEALLOC_1_DDR_FIELD =
{
    "DDR",
#if RU_INCLUDE_DESC
    "",
    "DDR Identifier\n0: DDR0\n1: DDR1\n\n",
#endif
    { FPM_POOL1_ALLOC_DEALLOC_1_DDR_FIELD_MASK },
    0,
    { FPM_POOL1_ALLOC_DEALLOC_1_DDR_FIELD_WIDTH },
    { FPM_POOL1_ALLOC_DEALLOC_1_DDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN_VALID *****/
const ru_field_rec FPM_POOL1_ALLOC_DEALLOC_1_TOKEN_VALID_FIELD =
{
    "TOKEN_VALID",
#if RU_INCLUDE_DESC
    "",
    "Valid Token Indicator\n0: No buffers available\n1: A valid token index is provided. If a token is de-allocated/freed without this bit set that causes an error and the token will be ignored, error counter in register offset 0xB8 will be incremented.\n\n",
#endif
    { FPM_POOL1_ALLOC_DEALLOC_1_TOKEN_VALID_FIELD_MASK },
    0,
    { FPM_POOL1_ALLOC_DEALLOC_1_TOKEN_VALID_FIELD_WIDTH },
    { FPM_POOL1_ALLOC_DEALLOC_1_TOKEN_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL1_ALLOC_DEALLOC_1_FIELDS[] =
{
    &FPM_POOL1_ALLOC_DEALLOC_1_TOKEN_SIZE_FIELD,
    &FPM_POOL1_ALLOC_DEALLOC_1_TOKEN_INDEX_FIELD,
    &FPM_POOL1_ALLOC_DEALLOC_1_DDR_FIELD,
    &FPM_POOL1_ALLOC_DEALLOC_1_TOKEN_VALID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_POOL1_ALLOC_DEALLOC_1 *****/
const ru_reg_rec FPM_POOL1_ALLOC_DEALLOC_1_REG =
{
    "POOL1_ALLOC_DEALLOC_1",
#if RU_INCLUDE_DESC
    "POOL1_ALLOC_DEALLOC Register",
    "The free pool FIFO contains pointers to the buffers in the pool. To allocate a buffer from the pool, read token from this port. To de-allocate/free a buffer to the pool , write the token of the buffer to this port. After reset, software must initialize the FIFO. The buffer size is given in the control register above. All buffers must be of the same size and contiguous.\n\n",
#endif
    { FPM_POOL1_ALLOC_DEALLOC_1_REG_OFFSET },
    0,
    0,
    503,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    FPM_POOL1_ALLOC_DEALLOC_1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_POOL2_ALLOC_DEALLOC_1, TYPE: Type_FPM_BLOCK_FPM_POOL_0_POOL2_ALLOC_DEALLOC
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN_SIZE *****/
const ru_field_rec FPM_POOL2_ALLOC_DEALLOC_1_TOKEN_SIZE_FIELD =
{
    "TOKEN_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Buffer length or packet size in bytes\n\n",
#endif
    { FPM_POOL2_ALLOC_DEALLOC_1_TOKEN_SIZE_FIELD_MASK },
    0,
    { FPM_POOL2_ALLOC_DEALLOC_1_TOKEN_SIZE_FIELD_WIDTH },
    { FPM_POOL2_ALLOC_DEALLOC_1_TOKEN_SIZE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN_INDEX *****/
const ru_field_rec FPM_POOL2_ALLOC_DEALLOC_1_TOKEN_INDEX_FIELD =
{
    "TOKEN_INDEX",
#if RU_INCLUDE_DESC
    "",
    "Buffer Index Pointer\n\n",
#endif
    { FPM_POOL2_ALLOC_DEALLOC_1_TOKEN_INDEX_FIELD_MASK },
    0,
    { FPM_POOL2_ALLOC_DEALLOC_1_TOKEN_INDEX_FIELD_WIDTH },
    { FPM_POOL2_ALLOC_DEALLOC_1_TOKEN_INDEX_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR *****/
const ru_field_rec FPM_POOL2_ALLOC_DEALLOC_1_DDR_FIELD =
{
    "DDR",
#if RU_INCLUDE_DESC
    "",
    "DDR Identifier\n0: DDR0\n1: DDR1\n\n",
#endif
    { FPM_POOL2_ALLOC_DEALLOC_1_DDR_FIELD_MASK },
    0,
    { FPM_POOL2_ALLOC_DEALLOC_1_DDR_FIELD_WIDTH },
    { FPM_POOL2_ALLOC_DEALLOC_1_DDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN_VALID *****/
const ru_field_rec FPM_POOL2_ALLOC_DEALLOC_1_TOKEN_VALID_FIELD =
{
    "TOKEN_VALID",
#if RU_INCLUDE_DESC
    "",
    "Valid Token Indicator\n0: No buffers available\n1: A valid token index is provided. If a token is de-allocated/freed without this bit set that causes an error and the token will be ignored, error counter in register offset 0xB8 will be incremented.\n\n",
#endif
    { FPM_POOL2_ALLOC_DEALLOC_1_TOKEN_VALID_FIELD_MASK },
    0,
    { FPM_POOL2_ALLOC_DEALLOC_1_TOKEN_VALID_FIELD_WIDTH },
    { FPM_POOL2_ALLOC_DEALLOC_1_TOKEN_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL2_ALLOC_DEALLOC_1_FIELDS[] =
{
    &FPM_POOL2_ALLOC_DEALLOC_1_TOKEN_SIZE_FIELD,
    &FPM_POOL2_ALLOC_DEALLOC_1_TOKEN_INDEX_FIELD,
    &FPM_POOL2_ALLOC_DEALLOC_1_DDR_FIELD,
    &FPM_POOL2_ALLOC_DEALLOC_1_TOKEN_VALID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_POOL2_ALLOC_DEALLOC_1 *****/
const ru_reg_rec FPM_POOL2_ALLOC_DEALLOC_1_REG =
{
    "POOL2_ALLOC_DEALLOC_1",
#if RU_INCLUDE_DESC
    "POOL2_ALLOC_DEALLOC Register",
    "The free pool FIFO contains pointers to the buffers in the pool. To allocate a buffer from the pool, read token from this port. To de-allocate/free a buffer to the pool , write the token of the buffer to this port. After reset, software must initialize the FIFO. The buffer size is given in the control register above. All buffers must be of the same size and contiguous.\n\n",
#endif
    { FPM_POOL2_ALLOC_DEALLOC_1_REG_OFFSET },
    0,
    0,
    504,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    FPM_POOL2_ALLOC_DEALLOC_1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_POOL3_ALLOC_DEALLOC_1, TYPE: Type_FPM_BLOCK_FPM_POOL_0_POOL3_ALLOC_DEALLOC
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN_SIZE *****/
const ru_field_rec FPM_POOL3_ALLOC_DEALLOC_1_TOKEN_SIZE_FIELD =
{
    "TOKEN_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Buffer length or packet size in bytes\n\n",
#endif
    { FPM_POOL3_ALLOC_DEALLOC_1_TOKEN_SIZE_FIELD_MASK },
    0,
    { FPM_POOL3_ALLOC_DEALLOC_1_TOKEN_SIZE_FIELD_WIDTH },
    { FPM_POOL3_ALLOC_DEALLOC_1_TOKEN_SIZE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN_INDEX *****/
const ru_field_rec FPM_POOL3_ALLOC_DEALLOC_1_TOKEN_INDEX_FIELD =
{
    "TOKEN_INDEX",
#if RU_INCLUDE_DESC
    "",
    "Buffer Index Pointer\n\n",
#endif
    { FPM_POOL3_ALLOC_DEALLOC_1_TOKEN_INDEX_FIELD_MASK },
    0,
    { FPM_POOL3_ALLOC_DEALLOC_1_TOKEN_INDEX_FIELD_WIDTH },
    { FPM_POOL3_ALLOC_DEALLOC_1_TOKEN_INDEX_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR *****/
const ru_field_rec FPM_POOL3_ALLOC_DEALLOC_1_DDR_FIELD =
{
    "DDR",
#if RU_INCLUDE_DESC
    "",
    "DDR Identifier\n0: DDR0\n1: DDR1\n\n",
#endif
    { FPM_POOL3_ALLOC_DEALLOC_1_DDR_FIELD_MASK },
    0,
    { FPM_POOL3_ALLOC_DEALLOC_1_DDR_FIELD_WIDTH },
    { FPM_POOL3_ALLOC_DEALLOC_1_DDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN_VALID *****/
const ru_field_rec FPM_POOL3_ALLOC_DEALLOC_1_TOKEN_VALID_FIELD =
{
    "TOKEN_VALID",
#if RU_INCLUDE_DESC
    "",
    "Valid Token Indicator\n0: No buffers available\n1: A valid token index is provided. If a token is de-allocated/freed without this bit set that causes an error and the token will be ignored, error counter in register offset 0xB8 will be incremented.\n\n",
#endif
    { FPM_POOL3_ALLOC_DEALLOC_1_TOKEN_VALID_FIELD_MASK },
    0,
    { FPM_POOL3_ALLOC_DEALLOC_1_TOKEN_VALID_FIELD_WIDTH },
    { FPM_POOL3_ALLOC_DEALLOC_1_TOKEN_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL3_ALLOC_DEALLOC_1_FIELDS[] =
{
    &FPM_POOL3_ALLOC_DEALLOC_1_TOKEN_SIZE_FIELD,
    &FPM_POOL3_ALLOC_DEALLOC_1_TOKEN_INDEX_FIELD,
    &FPM_POOL3_ALLOC_DEALLOC_1_DDR_FIELD,
    &FPM_POOL3_ALLOC_DEALLOC_1_TOKEN_VALID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_POOL3_ALLOC_DEALLOC_1 *****/
const ru_reg_rec FPM_POOL3_ALLOC_DEALLOC_1_REG =
{
    "POOL3_ALLOC_DEALLOC_1",
#if RU_INCLUDE_DESC
    "POOL3_ALLOC_DEALLOC Register",
    "The free pool FIFO contains pointers to the buffers in the pool. To allocate a buffer from the pool, read token from this port. To de-allocate/free a buffer to the pool , write the token of the buffer to this port. After reset, software must initialize the FIFO. The buffer size is given in the control register above. All buffers must be of the same size and contiguous.\n\n",
#endif
    { FPM_POOL3_ALLOC_DEALLOC_1_REG_OFFSET },
    0,
    0,
    505,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    FPM_POOL3_ALLOC_DEALLOC_1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_POOL4_ALLOC_DEALLOC_1, TYPE: Type_FPM_BLOCK_FPM_POOL_0_POOL4_ALLOC_DEALLOC
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN_SIZE *****/
const ru_field_rec FPM_POOL4_ALLOC_DEALLOC_1_TOKEN_SIZE_FIELD =
{
    "TOKEN_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Buffer length or packet size in bytes\n\n",
#endif
    { FPM_POOL4_ALLOC_DEALLOC_1_TOKEN_SIZE_FIELD_MASK },
    0,
    { FPM_POOL4_ALLOC_DEALLOC_1_TOKEN_SIZE_FIELD_WIDTH },
    { FPM_POOL4_ALLOC_DEALLOC_1_TOKEN_SIZE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN_INDEX *****/
const ru_field_rec FPM_POOL4_ALLOC_DEALLOC_1_TOKEN_INDEX_FIELD =
{
    "TOKEN_INDEX",
#if RU_INCLUDE_DESC
    "",
    "Buffer Index Pointer\n\n",
#endif
    { FPM_POOL4_ALLOC_DEALLOC_1_TOKEN_INDEX_FIELD_MASK },
    0,
    { FPM_POOL4_ALLOC_DEALLOC_1_TOKEN_INDEX_FIELD_WIDTH },
    { FPM_POOL4_ALLOC_DEALLOC_1_TOKEN_INDEX_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR *****/
const ru_field_rec FPM_POOL4_ALLOC_DEALLOC_1_DDR_FIELD =
{
    "DDR",
#if RU_INCLUDE_DESC
    "",
    "DDR Identifier\n0: DDR0\n1: DDR1\n\n",
#endif
    { FPM_POOL4_ALLOC_DEALLOC_1_DDR_FIELD_MASK },
    0,
    { FPM_POOL4_ALLOC_DEALLOC_1_DDR_FIELD_WIDTH },
    { FPM_POOL4_ALLOC_DEALLOC_1_DDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN_VALID *****/
const ru_field_rec FPM_POOL4_ALLOC_DEALLOC_1_TOKEN_VALID_FIELD =
{
    "TOKEN_VALID",
#if RU_INCLUDE_DESC
    "",
    "Valid Token Indicator\n0: No buffers available\n1: A valid token index is provided. If a token is de-allocated/freed without this bit set that causes an error and the token will be ignored, error counter in register offset 0xB8 will be incremented.\n\n",
#endif
    { FPM_POOL4_ALLOC_DEALLOC_1_TOKEN_VALID_FIELD_MASK },
    0,
    { FPM_POOL4_ALLOC_DEALLOC_1_TOKEN_VALID_FIELD_WIDTH },
    { FPM_POOL4_ALLOC_DEALLOC_1_TOKEN_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL4_ALLOC_DEALLOC_1_FIELDS[] =
{
    &FPM_POOL4_ALLOC_DEALLOC_1_TOKEN_SIZE_FIELD,
    &FPM_POOL4_ALLOC_DEALLOC_1_TOKEN_INDEX_FIELD,
    &FPM_POOL4_ALLOC_DEALLOC_1_DDR_FIELD,
    &FPM_POOL4_ALLOC_DEALLOC_1_TOKEN_VALID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_POOL4_ALLOC_DEALLOC_1 *****/
const ru_reg_rec FPM_POOL4_ALLOC_DEALLOC_1_REG =
{
    "POOL4_ALLOC_DEALLOC_1",
#if RU_INCLUDE_DESC
    "POOL4_ALLOC_DEALLOC Register",
    "The free pool FIFO contains pointers to the buffers in the pool. To allocate a buffer from the pool, read token from this port. To de-allocate/free a buffer to the pool , write the token of the buffer to this port. After reset, software must initialize the FIFO. The buffer size is given in the control register above. All buffers must be of the same size and contiguous.\n\n",
#endif
    { FPM_POOL4_ALLOC_DEALLOC_1_REG_OFFSET },
    0,
    0,
    506,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    FPM_POOL4_ALLOC_DEALLOC_1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_POOL_MULTI_1, TYPE: Type_FPM_BLOCK_FPM_POOL_0_POOL_MULTI
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN_MULTI *****/
const ru_field_rec FPM_POOL_MULTI_1_TOKEN_MULTI_FIELD =
{
    "TOKEN_MULTI",
#if RU_INCLUDE_DESC
    "",
    "New Multi-cast Value\n\n",
#endif
    { FPM_POOL_MULTI_1_TOKEN_MULTI_FIELD_MASK },
    0,
    { FPM_POOL_MULTI_1_TOKEN_MULTI_FIELD_WIDTH },
    { FPM_POOL_MULTI_1_TOKEN_MULTI_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_write,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R2 *****/
const ru_field_rec FPM_POOL_MULTI_1_R2_FIELD =
{
    "R2",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_POOL_MULTI_1_R2_FIELD_MASK },
    0,
    { FPM_POOL_MULTI_1_R2_FIELD_WIDTH },
    { FPM_POOL_MULTI_1_R2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: UPDATE_TYPE *****/
const ru_field_rec FPM_POOL_MULTI_1_UPDATE_TYPE_FIELD =
{
    "UPDATE_TYPE",
#if RU_INCLUDE_DESC
    "",
    "1'b0 - Count value is replaced with new value in bits[6:0]\n1'b1 - Count value is incremented by value in bits[6:0]\n\n",
#endif
    { FPM_POOL_MULTI_1_UPDATE_TYPE_FIELD_MASK },
    0,
    { FPM_POOL_MULTI_1_UPDATE_TYPE_FIELD_WIDTH },
    { FPM_POOL_MULTI_1_UPDATE_TYPE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_write,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN_INDEX *****/
const ru_field_rec FPM_POOL_MULTI_1_TOKEN_INDEX_FIELD =
{
    "TOKEN_INDEX",
#if RU_INCLUDE_DESC
    "",
    "Buffer Index Pointer\n\n",
#endif
    { FPM_POOL_MULTI_1_TOKEN_INDEX_FIELD_MASK },
    0,
    { FPM_POOL_MULTI_1_TOKEN_INDEX_FIELD_WIDTH },
    { FPM_POOL_MULTI_1_TOKEN_INDEX_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_write,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DDR *****/
const ru_field_rec FPM_POOL_MULTI_1_DDR_FIELD =
{
    "DDR",
#if RU_INCLUDE_DESC
    "",
    "DDR Identifier\n0: DDR0\n1: DDR1\n\n",
#endif
    { FPM_POOL_MULTI_1_DDR_FIELD_MASK },
    0,
    { FPM_POOL_MULTI_1_DDR_FIELD_WIDTH },
    { FPM_POOL_MULTI_1_DDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_write,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TOKEN_VALID *****/
const ru_field_rec FPM_POOL_MULTI_1_TOKEN_VALID_FIELD =
{
    "TOKEN_VALID",
#if RU_INCLUDE_DESC
    "",
    "Valid Token Indicator\n0: No buffers available\n1: A valid token index is provided. If a token multi-cast value is updated without this bit set, that causes an error and the token will be ignored, error counter in register offset 0xBC will be incremented.\n\n",
#endif
    { FPM_POOL_MULTI_1_TOKEN_VALID_FIELD_MASK },
    0,
    { FPM_POOL_MULTI_1_TOKEN_VALID_FIELD_WIDTH },
    { FPM_POOL_MULTI_1_TOKEN_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_write,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL_MULTI_1_FIELDS[] =
{
    &FPM_POOL_MULTI_1_TOKEN_MULTI_FIELD,
    &FPM_POOL_MULTI_1_R2_FIELD,
    &FPM_POOL_MULTI_1_UPDATE_TYPE_FIELD,
    &FPM_POOL_MULTI_1_TOKEN_INDEX_FIELD,
    &FPM_POOL_MULTI_1_DDR_FIELD,
    &FPM_POOL_MULTI_1_TOKEN_VALID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_POOL_MULTI_1 *****/
const ru_reg_rec FPM_POOL_MULTI_1_REG =
{
    "POOL_MULTI_1",
#if RU_INCLUDE_DESC
    "POOL_MULTI Register",
    "Update/Modify the multi-cast value of the token\n\n",
#endif
    { FPM_POOL_MULTI_1_REG_OFFSET },
    0,
    0,
    507,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    FPM_POOL_MULTI_1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_SEARCH_DATA_0, TYPE: Type_FPM_BLOCK_FPM_SEARCH_0_SEARCH_DATA_0
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SEARCHDATA0 *****/
const ru_field_rec FPM_SEARCH_DATA_0_SEARCHDATA0_FIELD =
{
    "SEARCHDATA0",
#if RU_INCLUDE_DESC
    "",
    "Search Tree\n\n",
#endif
    { FPM_SEARCH_DATA_0_SEARCHDATA0_FIELD_MASK },
    0,
    { FPM_SEARCH_DATA_0_SEARCHDATA0_FIELD_WIDTH },
    { FPM_SEARCH_DATA_0_SEARCHDATA0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SEARCHDATA1 *****/
const ru_field_rec FPM_SEARCH_DATA_0_SEARCHDATA1_FIELD =
{
    "SEARCHDATA1",
#if RU_INCLUDE_DESC
    "",
    "Search Tree\n\n",
#endif
    { FPM_SEARCH_DATA_0_SEARCHDATA1_FIELD_MASK },
    0,
    { FPM_SEARCH_DATA_0_SEARCHDATA1_FIELD_WIDTH },
    { FPM_SEARCH_DATA_0_SEARCHDATA1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SEARCHDATA2 *****/
const ru_field_rec FPM_SEARCH_DATA_0_SEARCHDATA2_FIELD =
{
    "SEARCHDATA2",
#if RU_INCLUDE_DESC
    "",
    "Search Tree\n\n",
#endif
    { FPM_SEARCH_DATA_0_SEARCHDATA2_FIELD_MASK },
    0,
    { FPM_SEARCH_DATA_0_SEARCHDATA2_FIELD_WIDTH },
    { FPM_SEARCH_DATA_0_SEARCHDATA2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SEARCHDATA3 *****/
const ru_field_rec FPM_SEARCH_DATA_0_SEARCHDATA3_FIELD =
{
    "SEARCHDATA3",
#if RU_INCLUDE_DESC
    "",
    "Search Tree\n\n",
#endif
    { FPM_SEARCH_DATA_0_SEARCHDATA3_FIELD_MASK },
    0,
    { FPM_SEARCH_DATA_0_SEARCHDATA3_FIELD_WIDTH },
    { FPM_SEARCH_DATA_0_SEARCHDATA3_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SEARCHDATA4 *****/
const ru_field_rec FPM_SEARCH_DATA_0_SEARCHDATA4_FIELD =
{
    "SEARCHDATA4",
#if RU_INCLUDE_DESC
    "",
    "Search Tree\n\n",
#endif
    { FPM_SEARCH_DATA_0_SEARCHDATA4_FIELD_MASK },
    0,
    { FPM_SEARCH_DATA_0_SEARCHDATA4_FIELD_WIDTH },
    { FPM_SEARCH_DATA_0_SEARCHDATA4_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SEARCHDATA5 *****/
const ru_field_rec FPM_SEARCH_DATA_0_SEARCHDATA5_FIELD =
{
    "SEARCHDATA5",
#if RU_INCLUDE_DESC
    "",
    "Search Tree\n\n",
#endif
    { FPM_SEARCH_DATA_0_SEARCHDATA5_FIELD_MASK },
    0,
    { FPM_SEARCH_DATA_0_SEARCHDATA5_FIELD_WIDTH },
    { FPM_SEARCH_DATA_0_SEARCHDATA5_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SEARCHDATA6 *****/
const ru_field_rec FPM_SEARCH_DATA_0_SEARCHDATA6_FIELD =
{
    "SEARCHDATA6",
#if RU_INCLUDE_DESC
    "",
    "Search Tree\n\n",
#endif
    { FPM_SEARCH_DATA_0_SEARCHDATA6_FIELD_MASK },
    0,
    { FPM_SEARCH_DATA_0_SEARCHDATA6_FIELD_WIDTH },
    { FPM_SEARCH_DATA_0_SEARCHDATA6_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SEARCHDATA7 *****/
const ru_field_rec FPM_SEARCH_DATA_0_SEARCHDATA7_FIELD =
{
    "SEARCHDATA7",
#if RU_INCLUDE_DESC
    "",
    "Search Tree\n\n",
#endif
    { FPM_SEARCH_DATA_0_SEARCHDATA7_FIELD_MASK },
    0,
    { FPM_SEARCH_DATA_0_SEARCHDATA7_FIELD_WIDTH },
    { FPM_SEARCH_DATA_0_SEARCHDATA7_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SEARCHDATA8 *****/
const ru_field_rec FPM_SEARCH_DATA_0_SEARCHDATA8_FIELD =
{
    "SEARCHDATA8",
#if RU_INCLUDE_DESC
    "",
    "Search Tree\n\n",
#endif
    { FPM_SEARCH_DATA_0_SEARCHDATA8_FIELD_MASK },
    0,
    { FPM_SEARCH_DATA_0_SEARCHDATA8_FIELD_WIDTH },
    { FPM_SEARCH_DATA_0_SEARCHDATA8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SEARCHDATA9 *****/
const ru_field_rec FPM_SEARCH_DATA_0_SEARCHDATA9_FIELD =
{
    "SEARCHDATA9",
#if RU_INCLUDE_DESC
    "",
    "Search Tree\n\n",
#endif
    { FPM_SEARCH_DATA_0_SEARCHDATA9_FIELD_MASK },
    0,
    { FPM_SEARCH_DATA_0_SEARCHDATA9_FIELD_WIDTH },
    { FPM_SEARCH_DATA_0_SEARCHDATA9_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SEARCHDATA10_L *****/
const ru_field_rec FPM_SEARCH_DATA_0_SEARCHDATA10_L_FIELD =
{
    "SEARCHDATA10_L",
#if RU_INCLUDE_DESC
    "",
    "Search Tree.\n2 lsbs of data10\n\n",
#endif
    { FPM_SEARCH_DATA_0_SEARCHDATA10_L_FIELD_MASK },
    0,
    { FPM_SEARCH_DATA_0_SEARCHDATA10_L_FIELD_WIDTH },
    { FPM_SEARCH_DATA_0_SEARCHDATA10_L_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_SEARCH_DATA_0_FIELDS[] =
{
    &FPM_SEARCH_DATA_0_SEARCHDATA0_FIELD,
    &FPM_SEARCH_DATA_0_SEARCHDATA1_FIELD,
    &FPM_SEARCH_DATA_0_SEARCHDATA2_FIELD,
    &FPM_SEARCH_DATA_0_SEARCHDATA3_FIELD,
    &FPM_SEARCH_DATA_0_SEARCHDATA4_FIELD,
    &FPM_SEARCH_DATA_0_SEARCHDATA5_FIELD,
    &FPM_SEARCH_DATA_0_SEARCHDATA6_FIELD,
    &FPM_SEARCH_DATA_0_SEARCHDATA7_FIELD,
    &FPM_SEARCH_DATA_0_SEARCHDATA8_FIELD,
    &FPM_SEARCH_DATA_0_SEARCHDATA9_FIELD,
    &FPM_SEARCH_DATA_0_SEARCHDATA10_L_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_SEARCH_DATA_0 *****/
const ru_reg_rec FPM_SEARCH_DATA_0_REG =
{
    "SEARCH_DATA_0",
#if RU_INCLUDE_DESC
    "SEARCH_DATA_0 Register",
    "Search Tree Branch Status Values ( Index 15 )\n3'b111 = No tokens available\n3'b011 = 256byte tokens available\n3'b010 = 512byte tokens or smaller available\n3'b001 = 1Kbyte tokens or smaller available\n3'b000 = 2Kbyte tokens or smaller available\n\nsearch_data_0. lsbs[31:0] of 64b data bus. {16b0, 16tokens[15..0] x 3}\n\n",
#endif
    { FPM_SEARCH_DATA_0_REG_OFFSET },
    FPM_SEARCH_DATA_0_REG_RAM_CNT,
    16,
    508,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    11,
    FPM_SEARCH_DATA_0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_SEARCH_DATA_1, TYPE: Type_FPM_BLOCK_FPM_SEARCH_0_SEARCH_DATA_1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SEARCHDATA10_M *****/
const ru_field_rec FPM_SEARCH_DATA_1_SEARCHDATA10_M_FIELD =
{
    "SEARCHDATA10_M",
#if RU_INCLUDE_DESC
    "",
    "Search Tree.\nmsb of data10\n\n",
#endif
    { FPM_SEARCH_DATA_1_SEARCHDATA10_M_FIELD_MASK },
    0,
    { FPM_SEARCH_DATA_1_SEARCHDATA10_M_FIELD_WIDTH },
    { FPM_SEARCH_DATA_1_SEARCHDATA10_M_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SEARCHDATA11 *****/
const ru_field_rec FPM_SEARCH_DATA_1_SEARCHDATA11_FIELD =
{
    "SEARCHDATA11",
#if RU_INCLUDE_DESC
    "",
    "Search Tree\n\n",
#endif
    { FPM_SEARCH_DATA_1_SEARCHDATA11_FIELD_MASK },
    0,
    { FPM_SEARCH_DATA_1_SEARCHDATA11_FIELD_WIDTH },
    { FPM_SEARCH_DATA_1_SEARCHDATA11_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SEARCHDATA12 *****/
const ru_field_rec FPM_SEARCH_DATA_1_SEARCHDATA12_FIELD =
{
    "SEARCHDATA12",
#if RU_INCLUDE_DESC
    "",
    "Search Tree\n\n",
#endif
    { FPM_SEARCH_DATA_1_SEARCHDATA12_FIELD_MASK },
    0,
    { FPM_SEARCH_DATA_1_SEARCHDATA12_FIELD_WIDTH },
    { FPM_SEARCH_DATA_1_SEARCHDATA12_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SEARCHDATA13 *****/
const ru_field_rec FPM_SEARCH_DATA_1_SEARCHDATA13_FIELD =
{
    "SEARCHDATA13",
#if RU_INCLUDE_DESC
    "",
    "Search Tree\n\n",
#endif
    { FPM_SEARCH_DATA_1_SEARCHDATA13_FIELD_MASK },
    0,
    { FPM_SEARCH_DATA_1_SEARCHDATA13_FIELD_WIDTH },
    { FPM_SEARCH_DATA_1_SEARCHDATA13_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SEARCHDATA14 *****/
const ru_field_rec FPM_SEARCH_DATA_1_SEARCHDATA14_FIELD =
{
    "SEARCHDATA14",
#if RU_INCLUDE_DESC
    "",
    "Search Tree\n\n",
#endif
    { FPM_SEARCH_DATA_1_SEARCHDATA14_FIELD_MASK },
    0,
    { FPM_SEARCH_DATA_1_SEARCHDATA14_FIELD_WIDTH },
    { FPM_SEARCH_DATA_1_SEARCHDATA14_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SEARCHDATA15 *****/
const ru_field_rec FPM_SEARCH_DATA_1_SEARCHDATA15_FIELD =
{
    "SEARCHDATA15",
#if RU_INCLUDE_DESC
    "",
    "Search Tree\n\n",
#endif
    { FPM_SEARCH_DATA_1_SEARCHDATA15_FIELD_MASK },
    0,
    { FPM_SEARCH_DATA_1_SEARCHDATA15_FIELD_WIDTH },
    { FPM_SEARCH_DATA_1_SEARCHDATA15_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec FPM_SEARCH_DATA_1_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_SEARCH_DATA_1_R1_FIELD_MASK },
    0,
    { FPM_SEARCH_DATA_1_R1_FIELD_WIDTH },
    { FPM_SEARCH_DATA_1_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_SEARCH_DATA_1_FIELDS[] =
{
    &FPM_SEARCH_DATA_1_SEARCHDATA10_M_FIELD,
    &FPM_SEARCH_DATA_1_SEARCHDATA11_FIELD,
    &FPM_SEARCH_DATA_1_SEARCHDATA12_FIELD,
    &FPM_SEARCH_DATA_1_SEARCHDATA13_FIELD,
    &FPM_SEARCH_DATA_1_SEARCHDATA14_FIELD,
    &FPM_SEARCH_DATA_1_SEARCHDATA15_FIELD,
    &FPM_SEARCH_DATA_1_R1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_SEARCH_DATA_1 *****/
const ru_reg_rec FPM_SEARCH_DATA_1_REG =
{
    "SEARCH_DATA_1",
#if RU_INCLUDE_DESC
    "SEARCH_DATA_1 Register",
    "Search Tree Branch Status Values ( Index 15 )\n3'b111 = No tokens available\n3'b011 = 256byte tokens available\n3'b010 = 512byte tokens or smaller available\n3'b001 = 1Kbyte tokens or smaller available\n3'b000 = 2Kbyte tokens or smaller available\n\nsearch_data_1. msbs[63:32] of 64b data bus. {16b0, 16tokens[15..0] x 3}\n",
#endif
    { FPM_SEARCH_DATA_1_REG_OFFSET },
    FPM_SEARCH_DATA_1_REG_RAM_CNT,
    16,
    509,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    FPM_SEARCH_DATA_1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_SEARCH_DATA_2, TYPE: Type_FPM_BLOCK_FPM_SEARCH_0_SEARCH_DATA_2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SEARCHDATA0 *****/
const ru_field_rec FPM_SEARCH_DATA_2_SEARCHDATA0_FIELD =
{
    "SEARCHDATA0",
#if RU_INCLUDE_DESC
    "",
    "Search Tree\n\n",
#endif
    { FPM_SEARCH_DATA_2_SEARCHDATA0_FIELD_MASK },
    0,
    { FPM_SEARCH_DATA_2_SEARCHDATA0_FIELD_WIDTH },
    { FPM_SEARCH_DATA_2_SEARCHDATA0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SEARCHDATA1 *****/
const ru_field_rec FPM_SEARCH_DATA_2_SEARCHDATA1_FIELD =
{
    "SEARCHDATA1",
#if RU_INCLUDE_DESC
    "",
    "Search Tree\n\n",
#endif
    { FPM_SEARCH_DATA_2_SEARCHDATA1_FIELD_MASK },
    0,
    { FPM_SEARCH_DATA_2_SEARCHDATA1_FIELD_WIDTH },
    { FPM_SEARCH_DATA_2_SEARCHDATA1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SEARCHDATA2 *****/
const ru_field_rec FPM_SEARCH_DATA_2_SEARCHDATA2_FIELD =
{
    "SEARCHDATA2",
#if RU_INCLUDE_DESC
    "",
    "Search Tree\n\n",
#endif
    { FPM_SEARCH_DATA_2_SEARCHDATA2_FIELD_MASK },
    0,
    { FPM_SEARCH_DATA_2_SEARCHDATA2_FIELD_WIDTH },
    { FPM_SEARCH_DATA_2_SEARCHDATA2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SEARCHDATA3 *****/
const ru_field_rec FPM_SEARCH_DATA_2_SEARCHDATA3_FIELD =
{
    "SEARCHDATA3",
#if RU_INCLUDE_DESC
    "",
    "Search Tree\n\n",
#endif
    { FPM_SEARCH_DATA_2_SEARCHDATA3_FIELD_MASK },
    0,
    { FPM_SEARCH_DATA_2_SEARCHDATA3_FIELD_WIDTH },
    { FPM_SEARCH_DATA_2_SEARCHDATA3_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SEARCHDATA4 *****/
const ru_field_rec FPM_SEARCH_DATA_2_SEARCHDATA4_FIELD =
{
    "SEARCHDATA4",
#if RU_INCLUDE_DESC
    "",
    "Search Tree\n\n",
#endif
    { FPM_SEARCH_DATA_2_SEARCHDATA4_FIELD_MASK },
    0,
    { FPM_SEARCH_DATA_2_SEARCHDATA4_FIELD_WIDTH },
    { FPM_SEARCH_DATA_2_SEARCHDATA4_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SEARCHDATA5 *****/
const ru_field_rec FPM_SEARCH_DATA_2_SEARCHDATA5_FIELD =
{
    "SEARCHDATA5",
#if RU_INCLUDE_DESC
    "",
    "Search Tree\n\n",
#endif
    { FPM_SEARCH_DATA_2_SEARCHDATA5_FIELD_MASK },
    0,
    { FPM_SEARCH_DATA_2_SEARCHDATA5_FIELD_WIDTH },
    { FPM_SEARCH_DATA_2_SEARCHDATA5_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SEARCHDATA6 *****/
const ru_field_rec FPM_SEARCH_DATA_2_SEARCHDATA6_FIELD =
{
    "SEARCHDATA6",
#if RU_INCLUDE_DESC
    "",
    "Search Tree\n\n",
#endif
    { FPM_SEARCH_DATA_2_SEARCHDATA6_FIELD_MASK },
    0,
    { FPM_SEARCH_DATA_2_SEARCHDATA6_FIELD_WIDTH },
    { FPM_SEARCH_DATA_2_SEARCHDATA6_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SEARCHDATA7 *****/
const ru_field_rec FPM_SEARCH_DATA_2_SEARCHDATA7_FIELD =
{
    "SEARCHDATA7",
#if RU_INCLUDE_DESC
    "",
    "Search Tree\n\n",
#endif
    { FPM_SEARCH_DATA_2_SEARCHDATA7_FIELD_MASK },
    0,
    { FPM_SEARCH_DATA_2_SEARCHDATA7_FIELD_WIDTH },
    { FPM_SEARCH_DATA_2_SEARCHDATA7_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SEARCHDATA8 *****/
const ru_field_rec FPM_SEARCH_DATA_2_SEARCHDATA8_FIELD =
{
    "SEARCHDATA8",
#if RU_INCLUDE_DESC
    "",
    "Search Tree\n\n",
#endif
    { FPM_SEARCH_DATA_2_SEARCHDATA8_FIELD_MASK },
    0,
    { FPM_SEARCH_DATA_2_SEARCHDATA8_FIELD_WIDTH },
    { FPM_SEARCH_DATA_2_SEARCHDATA8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SEARCHDATA9 *****/
const ru_field_rec FPM_SEARCH_DATA_2_SEARCHDATA9_FIELD =
{
    "SEARCHDATA9",
#if RU_INCLUDE_DESC
    "",
    "Search Tree\n\n",
#endif
    { FPM_SEARCH_DATA_2_SEARCHDATA9_FIELD_MASK },
    0,
    { FPM_SEARCH_DATA_2_SEARCHDATA9_FIELD_WIDTH },
    { FPM_SEARCH_DATA_2_SEARCHDATA9_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SEARCHDATA10_L *****/
const ru_field_rec FPM_SEARCH_DATA_2_SEARCHDATA10_L_FIELD =
{
    "SEARCHDATA10_L",
#if RU_INCLUDE_DESC
    "",
    "Search Tree.\n2 lsbs of data10\n\n",
#endif
    { FPM_SEARCH_DATA_2_SEARCHDATA10_L_FIELD_MASK },
    0,
    { FPM_SEARCH_DATA_2_SEARCHDATA10_L_FIELD_WIDTH },
    { FPM_SEARCH_DATA_2_SEARCHDATA10_L_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_SEARCH_DATA_2_FIELDS[] =
{
    &FPM_SEARCH_DATA_2_SEARCHDATA0_FIELD,
    &FPM_SEARCH_DATA_2_SEARCHDATA1_FIELD,
    &FPM_SEARCH_DATA_2_SEARCHDATA2_FIELD,
    &FPM_SEARCH_DATA_2_SEARCHDATA3_FIELD,
    &FPM_SEARCH_DATA_2_SEARCHDATA4_FIELD,
    &FPM_SEARCH_DATA_2_SEARCHDATA5_FIELD,
    &FPM_SEARCH_DATA_2_SEARCHDATA6_FIELD,
    &FPM_SEARCH_DATA_2_SEARCHDATA7_FIELD,
    &FPM_SEARCH_DATA_2_SEARCHDATA8_FIELD,
    &FPM_SEARCH_DATA_2_SEARCHDATA9_FIELD,
    &FPM_SEARCH_DATA_2_SEARCHDATA10_L_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_SEARCH_DATA_2 *****/
const ru_reg_rec FPM_SEARCH_DATA_2_REG =
{
    "SEARCH_DATA_2",
#if RU_INCLUDE_DESC
    "SEARCH_DATA_2 Register",
    "Search Tree Branch Status Values ( Index 15 )\n3'b111 = No tokens available\n3'b011 = 256byte tokens available\n3'b010 = 512byte tokens or smaller available\n3'b001 = 1Kbyte tokens or smaller available\n3'b000 = 2Kbyte tokens or smaller available\n\nsearch_data_2. lsbs[31:0] of 64b data bus. {16b0, 16tokens[31..16] x 3}\n\n",
#endif
    { FPM_SEARCH_DATA_2_REG_OFFSET },
    FPM_SEARCH_DATA_2_REG_RAM_CNT,
    16,
    510,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    11,
    FPM_SEARCH_DATA_2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_SEARCH_DATA_3, TYPE: Type_FPM_BLOCK_FPM_SEARCH_0_SEARCH_DATA_3
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SEARCHDATA10_M *****/
const ru_field_rec FPM_SEARCH_DATA_3_SEARCHDATA10_M_FIELD =
{
    "SEARCHDATA10_M",
#if RU_INCLUDE_DESC
    "",
    "Search Tree.\nmsb of data10\n\n",
#endif
    { FPM_SEARCH_DATA_3_SEARCHDATA10_M_FIELD_MASK },
    0,
    { FPM_SEARCH_DATA_3_SEARCHDATA10_M_FIELD_WIDTH },
    { FPM_SEARCH_DATA_3_SEARCHDATA10_M_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SEARCHDATA11 *****/
const ru_field_rec FPM_SEARCH_DATA_3_SEARCHDATA11_FIELD =
{
    "SEARCHDATA11",
#if RU_INCLUDE_DESC
    "",
    "Search Tree\n\n",
#endif
    { FPM_SEARCH_DATA_3_SEARCHDATA11_FIELD_MASK },
    0,
    { FPM_SEARCH_DATA_3_SEARCHDATA11_FIELD_WIDTH },
    { FPM_SEARCH_DATA_3_SEARCHDATA11_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SEARCHDATA12 *****/
const ru_field_rec FPM_SEARCH_DATA_3_SEARCHDATA12_FIELD =
{
    "SEARCHDATA12",
#if RU_INCLUDE_DESC
    "",
    "Search Tree\n\n",
#endif
    { FPM_SEARCH_DATA_3_SEARCHDATA12_FIELD_MASK },
    0,
    { FPM_SEARCH_DATA_3_SEARCHDATA12_FIELD_WIDTH },
    { FPM_SEARCH_DATA_3_SEARCHDATA12_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SEARCHDATA13 *****/
const ru_field_rec FPM_SEARCH_DATA_3_SEARCHDATA13_FIELD =
{
    "SEARCHDATA13",
#if RU_INCLUDE_DESC
    "",
    "Search Tree\n\n",
#endif
    { FPM_SEARCH_DATA_3_SEARCHDATA13_FIELD_MASK },
    0,
    { FPM_SEARCH_DATA_3_SEARCHDATA13_FIELD_WIDTH },
    { FPM_SEARCH_DATA_3_SEARCHDATA13_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SEARCHDATA14 *****/
const ru_field_rec FPM_SEARCH_DATA_3_SEARCHDATA14_FIELD =
{
    "SEARCHDATA14",
#if RU_INCLUDE_DESC
    "",
    "Search Tree\n\n",
#endif
    { FPM_SEARCH_DATA_3_SEARCHDATA14_FIELD_MASK },
    0,
    { FPM_SEARCH_DATA_3_SEARCHDATA14_FIELD_WIDTH },
    { FPM_SEARCH_DATA_3_SEARCHDATA14_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SEARCHDATA15 *****/
const ru_field_rec FPM_SEARCH_DATA_3_SEARCHDATA15_FIELD =
{
    "SEARCHDATA15",
#if RU_INCLUDE_DESC
    "",
    "Search Tree\n\n",
#endif
    { FPM_SEARCH_DATA_3_SEARCHDATA15_FIELD_MASK },
    0,
    { FPM_SEARCH_DATA_3_SEARCHDATA15_FIELD_WIDTH },
    { FPM_SEARCH_DATA_3_SEARCHDATA15_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: R1 *****/
const ru_field_rec FPM_SEARCH_DATA_3_R1_FIELD =
{
    "R1",
#if RU_INCLUDE_DESC
    "",
    "No Descriotion\n",
#endif
    { FPM_SEARCH_DATA_3_R1_FIELD_MASK },
    0,
    { FPM_SEARCH_DATA_3_R1_FIELD_WIDTH },
    { FPM_SEARCH_DATA_3_R1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_SEARCH_DATA_3_FIELDS[] =
{
    &FPM_SEARCH_DATA_3_SEARCHDATA10_M_FIELD,
    &FPM_SEARCH_DATA_3_SEARCHDATA11_FIELD,
    &FPM_SEARCH_DATA_3_SEARCHDATA12_FIELD,
    &FPM_SEARCH_DATA_3_SEARCHDATA13_FIELD,
    &FPM_SEARCH_DATA_3_SEARCHDATA14_FIELD,
    &FPM_SEARCH_DATA_3_SEARCHDATA15_FIELD,
    &FPM_SEARCH_DATA_3_R1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_SEARCH_DATA_3 *****/
const ru_reg_rec FPM_SEARCH_DATA_3_REG =
{
    "SEARCH_DATA_3",
#if RU_INCLUDE_DESC
    "SEARCH_DATA_3 Register",
    "Search Tree Branch Status Values ( Index 15 )\n3'b111 = No tokens available\n3'b011 = 256byte tokens available\n3'b010 = 512byte tokens or smaller available\n3'b001 = 1Kbyte tokens or smaller available\n3'b000 = 2Kbyte tokens or smaller available\n\nsearch_data_3. msbs[63:32] of 64b data bus. {16b0, 16tokens[31..16] x 3}\nView\n",
#endif
    { FPM_SEARCH_DATA_3_REG_OFFSET },
    FPM_SEARCH_DATA_3_REG_RAM_CNT,
    16,
    511,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    FPM_SEARCH_DATA_3_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_MULTICAST_DATA, TYPE: Type_FPM_BLOCK_FPM_MULTI_0_MULTICAST_DATA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MULTICAST *****/
const ru_field_rec FPM_MULTICAST_DATA_MULTICAST_FIELD =
{
    "MULTICAST",
#if RU_INCLUDE_DESC
    "",
    "Multicast Value:\n32b for 32 tokens\n\n",
#endif
    { FPM_MULTICAST_DATA_MULTICAST_FIELD_MASK },
    0,
    { FPM_MULTICAST_DATA_MULTICAST_FIELD_WIDTH },
    { FPM_MULTICAST_DATA_MULTICAST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_MULTICAST_DATA_FIELDS[] =
{
    &FPM_MULTICAST_DATA_MULTICAST_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_MULTICAST_DATA *****/
const ru_reg_rec FPM_MULTICAST_DATA_REG =
{
    "MULTICAST_DATA",
#if RU_INCLUDE_DESC
    "MULTICAST_DATA 0..8191 Register",
    "\n\n",
#endif
    { FPM_MULTICAST_DATA_REG_OFFSET },
    FPM_MULTICAST_DATA_REG_RAM_CNT,
    4,
    512,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_MULTICAST_DATA_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_COMPUTE_POOL_DATA, TYPE: Type_FPM_BLOCK_FPM_COMPUTE_POOL_0_COMPUTE_POOL_DATA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: POOLID *****/
const ru_field_rec FPM_COMPUTE_POOL_DATA_POOLID_FIELD =
{
    "POOLID",
#if RU_INCLUDE_DESC
    "",
    "Computed pool\n0 - pool id 0\n1 - pool id 1\n2 - pool id 2\n3 - pool id 3\n0xff - token not allocated\n\n",
#endif
    { FPM_COMPUTE_POOL_DATA_POOLID_FIELD_MASK },
    0,
    { FPM_COMPUTE_POOL_DATA_POOLID_FIELD_WIDTH },
    { FPM_COMPUTE_POOL_DATA_POOLID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_COMPUTE_POOL_DATA_FIELDS[] =
{
    &FPM_COMPUTE_POOL_DATA_POOLID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_COMPUTE_POOL_DATA *****/
const ru_reg_rec FPM_COMPUTE_POOL_DATA_REG =
{
    "COMPUTE_POOL_DATA",
#if RU_INCLUDE_DESC
    "COMPUTE_POOL_DATA 0..16383 Register",
    "Returns the pool id for the token index associated with the address offset.\n\neach token number returns corresponding value in bits [7:0].\neach 4 addresses, in 1B resolution is 4 tokens, so for 64K tokens, we have 16K address space.\n\n",
#endif
    { FPM_COMPUTE_POOL_DATA_REG_OFFSET },
    FPM_COMPUTE_POOL_DATA_REG_RAM_CNT,
    4,
    513,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_COMPUTE_POOL_DATA_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_FPM_BB_FORCE, TYPE: Type_FPM_BLOCK_FPM_BB_FPM_BB_FORCE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FORCE *****/
const ru_field_rec FPM_FPM_BB_FORCE_FORCE_FIELD =
{
    "FORCE",
#if RU_INCLUDE_DESC
    "",
    "Write 1 to force BB transaction\n",
#endif
    { FPM_FPM_BB_FORCE_FORCE_FIELD_MASK },
    0,
    { FPM_FPM_BB_FORCE_FORCE_FIELD_WIDTH },
    { FPM_FPM_BB_FORCE_FORCE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_FPM_BB_FORCE_FIELDS[] =
{
    &FPM_FPM_BB_FORCE_FORCE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_FPM_BB_FORCE *****/
const ru_reg_rec FPM_FPM_BB_FORCE_REG =
{
    "FPM_BB_FORCE",
#if RU_INCLUDE_DESC
    "FPM_BB_FORCE Register",
    "Write this register to force FPM_BB transaction\n",
#endif
    { FPM_FPM_BB_FORCE_REG_OFFSET },
    0,
    0,
    514,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_FPM_BB_FORCE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_FPM_BB_FORCED_CTRL, TYPE: Type_FPM_BLOCK_FPM_BB_FPM_BB_FORCED_CTRL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CTRL *****/
const ru_field_rec FPM_FPM_BB_FORCED_CTRL_CTRL_FIELD =
{
    "CTRL",
#if RU_INCLUDE_DESC
    "",
    "Forced control\n",
#endif
    { FPM_FPM_BB_FORCED_CTRL_CTRL_FIELD_MASK },
    0,
    { FPM_FPM_BB_FORCED_CTRL_CTRL_FIELD_WIDTH },
    { FPM_FPM_BB_FORCED_CTRL_CTRL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_FPM_BB_FORCED_CTRL_FIELDS[] =
{
    &FPM_FPM_BB_FORCED_CTRL_CTRL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_FPM_BB_FORCED_CTRL *****/
const ru_reg_rec FPM_FPM_BB_FORCED_CTRL_REG =
{
    "FPM_BB_FORCED_CTRL",
#if RU_INCLUDE_DESC
    "FPM_BB_FORCED_CTRL Register",
    "Control to be sent on forced transaction\n",
#endif
    { FPM_FPM_BB_FORCED_CTRL_REG_OFFSET },
    0,
    0,
    515,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_FPM_BB_FORCED_CTRL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_FPM_BB_FORCED_ADDR, TYPE: Type_FPM_BLOCK_FPM_BB_FPM_BB_FORCED_ADDR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TA_ADDR *****/
const ru_field_rec FPM_FPM_BB_FORCED_ADDR_TA_ADDR_FIELD =
{
    "TA_ADDR",
#if RU_INCLUDE_DESC
    "",
    "Forced TA address\n",
#endif
    { FPM_FPM_BB_FORCED_ADDR_TA_ADDR_FIELD_MASK },
    0,
    { FPM_FPM_BB_FORCED_ADDR_TA_ADDR_FIELD_WIDTH },
    { FPM_FPM_BB_FORCED_ADDR_TA_ADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DEST_ADDR *****/
const ru_field_rec FPM_FPM_BB_FORCED_ADDR_DEST_ADDR_FIELD =
{
    "DEST_ADDR",
#if RU_INCLUDE_DESC
    "",
    "Forced destination address\n",
#endif
    { FPM_FPM_BB_FORCED_ADDR_DEST_ADDR_FIELD_MASK },
    0,
    { FPM_FPM_BB_FORCED_ADDR_DEST_ADDR_FIELD_WIDTH },
    { FPM_FPM_BB_FORCED_ADDR_DEST_ADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_FPM_BB_FORCED_ADDR_FIELDS[] =
{
    &FPM_FPM_BB_FORCED_ADDR_TA_ADDR_FIELD,
    &FPM_FPM_BB_FORCED_ADDR_DEST_ADDR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_FPM_BB_FORCED_ADDR *****/
const ru_reg_rec FPM_FPM_BB_FORCED_ADDR_REG =
{
    "FPM_BB_FORCED_ADDR",
#if RU_INCLUDE_DESC
    "FPM_BB_FORCED_ADDR Register",
    "Address to be sent on forced transaction\n",
#endif
    { FPM_FPM_BB_FORCED_ADDR_REG_OFFSET },
    0,
    0,
    516,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_FPM_BB_FORCED_ADDR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_FPM_BB_FORCED_DATA, TYPE: Type_FPM_BLOCK_FPM_BB_FPM_BB_FORCED_DATA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec FPM_FPM_BB_FORCED_DATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "Forced data\n",
#endif
    { FPM_FPM_BB_FORCED_DATA_DATA_FIELD_MASK },
    0,
    { FPM_FPM_BB_FORCED_DATA_DATA_FIELD_WIDTH },
    { FPM_FPM_BB_FORCED_DATA_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_FPM_BB_FORCED_DATA_FIELDS[] =
{
    &FPM_FPM_BB_FORCED_DATA_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_FPM_BB_FORCED_DATA *****/
const ru_reg_rec FPM_FPM_BB_FORCED_DATA_REG =
{
    "FPM_BB_FORCED_DATA",
#if RU_INCLUDE_DESC
    "FPM_BB_FORCED_DATA Register",
    "Data to be sent on forced transaction\n",
#endif
    { FPM_FPM_BB_FORCED_DATA_REG_OFFSET },
    0,
    0,
    517,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_FPM_BB_FORCED_DATA_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_FPM_BB_DECODE_CFG, TYPE: Type_FPM_BLOCK_FPM_BB_FPM_BB_DECODE_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DEST_ID *****/
const ru_field_rec FPM_FPM_BB_DECODE_CFG_DEST_ID_FIELD =
{
    "DEST_ID",
#if RU_INCLUDE_DESC
    "",
    "destination id\n",
#endif
    { FPM_FPM_BB_DECODE_CFG_DEST_ID_FIELD_MASK },
    0,
    { FPM_FPM_BB_DECODE_CFG_DEST_ID_FIELD_WIDTH },
    { FPM_FPM_BB_DECODE_CFG_DEST_ID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: OVERRIDE_EN *****/
const ru_field_rec FPM_FPM_BB_DECODE_CFG_OVERRIDE_EN_FIELD =
{
    "OVERRIDE_EN",
#if RU_INCLUDE_DESC
    "",
    "Enable override\n",
#endif
    { FPM_FPM_BB_DECODE_CFG_OVERRIDE_EN_FIELD_MASK },
    0,
    { FPM_FPM_BB_DECODE_CFG_OVERRIDE_EN_FIELD_WIDTH },
    { FPM_FPM_BB_DECODE_CFG_OVERRIDE_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ROUTE_ADDR *****/
const ru_field_rec FPM_FPM_BB_DECODE_CFG_ROUTE_ADDR_FIELD =
{
    "ROUTE_ADDR",
#if RU_INCLUDE_DESC
    "",
    "route address\n",
#endif
    { FPM_FPM_BB_DECODE_CFG_ROUTE_ADDR_FIELD_MASK },
    0,
    { FPM_FPM_BB_DECODE_CFG_ROUTE_ADDR_FIELD_WIDTH },
    { FPM_FPM_BB_DECODE_CFG_ROUTE_ADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_FPM_BB_DECODE_CFG_FIELDS[] =
{
    &FPM_FPM_BB_DECODE_CFG_DEST_ID_FIELD,
    &FPM_FPM_BB_DECODE_CFG_OVERRIDE_EN_FIELD,
    &FPM_FPM_BB_DECODE_CFG_ROUTE_ADDR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_FPM_BB_DECODE_CFG *****/
const ru_reg_rec FPM_FPM_BB_DECODE_CFG_REG =
{
    "FPM_BB_DECODE_CFG",
#if RU_INCLUDE_DESC
    "FPM_BB_DECODE_CFG Register",
    "set configuration for BB decoder\n",
#endif
    { FPM_FPM_BB_DECODE_CFG_REG_OFFSET },
    0,
    0,
    518,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    FPM_FPM_BB_DECODE_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_FPM_BB_DBG_CFG, TYPE: Type_FPM_BLOCK_FPM_BB_FPM_BB_DBG_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RXFIFO_SW_ADDR *****/
const ru_field_rec FPM_FPM_BB_DBG_CFG_RXFIFO_SW_ADDR_FIELD =
{
    "RXFIFO_SW_ADDR",
#if RU_INCLUDE_DESC
    "",
    "SW address for reading FPM BB RXFIFO\n",
#endif
    { FPM_FPM_BB_DBG_CFG_RXFIFO_SW_ADDR_FIELD_MASK },
    0,
    { FPM_FPM_BB_DBG_CFG_RXFIFO_SW_ADDR_FIELD_WIDTH },
    { FPM_FPM_BB_DBG_CFG_RXFIFO_SW_ADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TXFIFO_SW_ADDR *****/
const ru_field_rec FPM_FPM_BB_DBG_CFG_TXFIFO_SW_ADDR_FIELD =
{
    "TXFIFO_SW_ADDR",
#if RU_INCLUDE_DESC
    "",
    "SW address for reading FPM BB TXFIFO\n",
#endif
    { FPM_FPM_BB_DBG_CFG_TXFIFO_SW_ADDR_FIELD_MASK },
    0,
    { FPM_FPM_BB_DBG_CFG_TXFIFO_SW_ADDR_FIELD_WIDTH },
    { FPM_FPM_BB_DBG_CFG_TXFIFO_SW_ADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RXFIFO_SW_RST *****/
const ru_field_rec FPM_FPM_BB_DBG_CFG_RXFIFO_SW_RST_FIELD =
{
    "RXFIFO_SW_RST",
#if RU_INCLUDE_DESC
    "",
    "SW reset for FPM BB RXFIFO\n",
#endif
    { FPM_FPM_BB_DBG_CFG_RXFIFO_SW_RST_FIELD_MASK },
    0,
    { FPM_FPM_BB_DBG_CFG_RXFIFO_SW_RST_FIELD_WIDTH },
    { FPM_FPM_BB_DBG_CFG_RXFIFO_SW_RST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TXFIFO_SW_RST *****/
const ru_field_rec FPM_FPM_BB_DBG_CFG_TXFIFO_SW_RST_FIELD =
{
    "TXFIFO_SW_RST",
#if RU_INCLUDE_DESC
    "",
    "SW reset for FPM BB TXFIFO\n",
#endif
    { FPM_FPM_BB_DBG_CFG_TXFIFO_SW_RST_FIELD_MASK },
    0,
    { FPM_FPM_BB_DBG_CFG_TXFIFO_SW_RST_FIELD_WIDTH },
    { FPM_FPM_BB_DBG_CFG_TXFIFO_SW_RST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_FPM_BB_DBG_CFG_FIELDS[] =
{
    &FPM_FPM_BB_DBG_CFG_RXFIFO_SW_ADDR_FIELD,
    &FPM_FPM_BB_DBG_CFG_TXFIFO_SW_ADDR_FIELD,
    &FPM_FPM_BB_DBG_CFG_RXFIFO_SW_RST_FIELD,
    &FPM_FPM_BB_DBG_CFG_TXFIFO_SW_RST_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_FPM_BB_DBG_CFG *****/
const ru_reg_rec FPM_FPM_BB_DBG_CFG_REG =
{
    "FPM_BB_DBG_CFG",
#if RU_INCLUDE_DESC
    "FPM_BB_DBG_CFG Register",
    "Set SW addr to read FPM_BB FIFOs\n",
#endif
    { FPM_FPM_BB_DBG_CFG_REG_OFFSET },
    0,
    0,
    519,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    FPM_FPM_BB_DBG_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_FPM_BB_DBG_RXFIFO_STS, TYPE: Type_FPM_BLOCK_FPM_BB_FPM_BB_DBG_RXFIFO_STS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FIFO_EMPTY *****/
const ru_field_rec FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_EMPTY_FIELD =
{
    "FIFO_EMPTY",
#if RU_INCLUDE_DESC
    "",
    "FIFO is empty\n",
#endif
    { FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_EMPTY_FIELD_MASK },
    0,
    { FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_EMPTY_FIELD_WIDTH },
    { FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_EMPTY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FIFO_FULL *****/
const ru_field_rec FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_FULL_FIELD =
{
    "FIFO_FULL",
#if RU_INCLUDE_DESC
    "",
    "FIFO is full\n",
#endif
    { FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_FULL_FIELD_MASK },
    0,
    { FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_FULL_FIELD_WIDTH },
    { FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FIFO_USED_WORDS *****/
const ru_field_rec FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_USED_WORDS_FIELD =
{
    "FIFO_USED_WORDS",
#if RU_INCLUDE_DESC
    "",
    "Used words\n",
#endif
    { FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_USED_WORDS_FIELD_MASK },
    0,
    { FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_USED_WORDS_FIELD_WIDTH },
    { FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_USED_WORDS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FIFO_RD_CNTR *****/
const ru_field_rec FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_RD_CNTR_FIELD =
{
    "FIFO_RD_CNTR",
#if RU_INCLUDE_DESC
    "",
    "Write counter\n",
#endif
    { FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_RD_CNTR_FIELD_MASK },
    0,
    { FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_RD_CNTR_FIELD_WIDTH },
    { FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_RD_CNTR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FIFO_WR_CNTR *****/
const ru_field_rec FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_WR_CNTR_FIELD =
{
    "FIFO_WR_CNTR",
#if RU_INCLUDE_DESC
    "",
    "Write counter\n",
#endif
    { FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_WR_CNTR_FIELD_MASK },
    0,
    { FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_WR_CNTR_FIELD_WIDTH },
    { FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_WR_CNTR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_FPM_BB_DBG_RXFIFO_STS_FIELDS[] =
{
    &FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_EMPTY_FIELD,
    &FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_FULL_FIELD,
    &FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_USED_WORDS_FIELD,
    &FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_RD_CNTR_FIELD,
    &FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_WR_CNTR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_FPM_BB_DBG_RXFIFO_STS *****/
const ru_reg_rec FPM_FPM_BB_DBG_RXFIFO_STS_REG =
{
    "FPM_BB_DBG_RXFIFO_STS",
#if RU_INCLUDE_DESC
    "FPM_BB_DBG_RXFIFO_STS Register",
    "Status of FPM BB RXFIFO\n",
#endif
    { FPM_FPM_BB_DBG_RXFIFO_STS_REG_OFFSET },
    0,
    0,
    520,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    FPM_FPM_BB_DBG_RXFIFO_STS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_FPM_BB_DBG_TXFIFO_STS, TYPE: Type_FPM_BLOCK_FPM_BB_FPM_BB_DBG_TXFIFO_STS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FIFO_EMPTY *****/
const ru_field_rec FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_EMPTY_FIELD =
{
    "FIFO_EMPTY",
#if RU_INCLUDE_DESC
    "",
    "FIFO is empty\n",
#endif
    { FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_EMPTY_FIELD_MASK },
    0,
    { FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_EMPTY_FIELD_WIDTH },
    { FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_EMPTY_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FIFO_FULL *****/
const ru_field_rec FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_FULL_FIELD =
{
    "FIFO_FULL",
#if RU_INCLUDE_DESC
    "",
    "FIFO is full\n",
#endif
    { FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_FULL_FIELD_MASK },
    0,
    { FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_FULL_FIELD_WIDTH },
    { FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FIFO_USED_WORDS *****/
const ru_field_rec FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_USED_WORDS_FIELD =
{
    "FIFO_USED_WORDS",
#if RU_INCLUDE_DESC
    "",
    "Used words\n",
#endif
    { FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_USED_WORDS_FIELD_MASK },
    0,
    { FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_USED_WORDS_FIELD_WIDTH },
    { FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_USED_WORDS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FIFO_RD_CNTR *****/
const ru_field_rec FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_RD_CNTR_FIELD =
{
    "FIFO_RD_CNTR",
#if RU_INCLUDE_DESC
    "",
    "Write counter\n",
#endif
    { FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_RD_CNTR_FIELD_MASK },
    0,
    { FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_RD_CNTR_FIELD_WIDTH },
    { FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_RD_CNTR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FIFO_WR_CNTR *****/
const ru_field_rec FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_WR_CNTR_FIELD =
{
    "FIFO_WR_CNTR",
#if RU_INCLUDE_DESC
    "",
    "Write counter\n",
#endif
    { FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_WR_CNTR_FIELD_MASK },
    0,
    { FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_WR_CNTR_FIELD_WIDTH },
    { FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_WR_CNTR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_FPM_BB_DBG_TXFIFO_STS_FIELDS[] =
{
    &FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_EMPTY_FIELD,
    &FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_FULL_FIELD,
    &FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_USED_WORDS_FIELD,
    &FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_RD_CNTR_FIELD,
    &FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_WR_CNTR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_FPM_BB_DBG_TXFIFO_STS *****/
const ru_reg_rec FPM_FPM_BB_DBG_TXFIFO_STS_REG =
{
    "FPM_BB_DBG_TXFIFO_STS",
#if RU_INCLUDE_DESC
    "FPM_BB_DBG_TXFIFO_STS Register",
    "Status of FPM BB TXFIFO\n",
#endif
    { FPM_FPM_BB_DBG_TXFIFO_STS_REG_OFFSET },
    0,
    0,
    521,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    FPM_FPM_BB_DBG_TXFIFO_STS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_FPM_BB_DBG_RXFIFO_DATA1, TYPE: Type_FPM_BLOCK_FPM_BB_FPM_BB_DBG_RXFIFO_DATA1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec FPM_FPM_BB_DBG_RXFIFO_DATA1_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "data\n",
#endif
    { FPM_FPM_BB_DBG_RXFIFO_DATA1_DATA_FIELD_MASK },
    0,
    { FPM_FPM_BB_DBG_RXFIFO_DATA1_DATA_FIELD_WIDTH },
    { FPM_FPM_BB_DBG_RXFIFO_DATA1_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_FPM_BB_DBG_RXFIFO_DATA1_FIELDS[] =
{
    &FPM_FPM_BB_DBG_RXFIFO_DATA1_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_FPM_BB_DBG_RXFIFO_DATA1 *****/
const ru_reg_rec FPM_FPM_BB_DBG_RXFIFO_DATA1_REG =
{
    "FPM_BB_DBG_RXFIFO_DATA1",
#if RU_INCLUDE_DESC
    "FPM_BB_DBG_RXFIFO_DATA1 Register",
    "Data from FPM BB RXFIFO bits [31:0]\n",
#endif
    { FPM_FPM_BB_DBG_RXFIFO_DATA1_REG_OFFSET },
    0,
    0,
    522,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_FPM_BB_DBG_RXFIFO_DATA1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_FPM_BB_DBG_RXFIFO_DATA2, TYPE: Type_FPM_BLOCK_FPM_BB_FPM_BB_DBG_RXFIFO_DATA2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec FPM_FPM_BB_DBG_RXFIFO_DATA2_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "data\n",
#endif
    { FPM_FPM_BB_DBG_RXFIFO_DATA2_DATA_FIELD_MASK },
    0,
    { FPM_FPM_BB_DBG_RXFIFO_DATA2_DATA_FIELD_WIDTH },
    { FPM_FPM_BB_DBG_RXFIFO_DATA2_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_FPM_BB_DBG_RXFIFO_DATA2_FIELDS[] =
{
    &FPM_FPM_BB_DBG_RXFIFO_DATA2_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_FPM_BB_DBG_RXFIFO_DATA2 *****/
const ru_reg_rec FPM_FPM_BB_DBG_RXFIFO_DATA2_REG =
{
    "FPM_BB_DBG_RXFIFO_DATA2",
#if RU_INCLUDE_DESC
    "FPM_BB_DBG_RXFIFO_DATA2 Register",
    "Data from FPM BB RXFIFO bits [39:32]\n",
#endif
    { FPM_FPM_BB_DBG_RXFIFO_DATA2_REG_OFFSET },
    0,
    0,
    523,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_FPM_BB_DBG_RXFIFO_DATA2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_FPM_BB_DBG_TXFIFO_DATA1, TYPE: Type_FPM_BLOCK_FPM_BB_FPM_BB_DBG_TXFIFO_DATA1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec FPM_FPM_BB_DBG_TXFIFO_DATA1_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "data\n",
#endif
    { FPM_FPM_BB_DBG_TXFIFO_DATA1_DATA_FIELD_MASK },
    0,
    { FPM_FPM_BB_DBG_TXFIFO_DATA1_DATA_FIELD_WIDTH },
    { FPM_FPM_BB_DBG_TXFIFO_DATA1_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_FPM_BB_DBG_TXFIFO_DATA1_FIELDS[] =
{
    &FPM_FPM_BB_DBG_TXFIFO_DATA1_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_FPM_BB_DBG_TXFIFO_DATA1 *****/
const ru_reg_rec FPM_FPM_BB_DBG_TXFIFO_DATA1_REG =
{
    "FPM_BB_DBG_TXFIFO_DATA1",
#if RU_INCLUDE_DESC
    "FPM_BB_DBG_TXFIFO_DATA1 Register",
    "Data from FPM BB TXFIFO bits [31:0]\n",
#endif
    { FPM_FPM_BB_DBG_TXFIFO_DATA1_REG_OFFSET },
    0,
    0,
    524,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_FPM_BB_DBG_TXFIFO_DATA1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_FPM_BB_DBG_TXFIFO_DATA2, TYPE: Type_FPM_BLOCK_FPM_BB_FPM_BB_DBG_TXFIFO_DATA2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec FPM_FPM_BB_DBG_TXFIFO_DATA2_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "data\n",
#endif
    { FPM_FPM_BB_DBG_TXFIFO_DATA2_DATA_FIELD_MASK },
    0,
    { FPM_FPM_BB_DBG_TXFIFO_DATA2_DATA_FIELD_WIDTH },
    { FPM_FPM_BB_DBG_TXFIFO_DATA2_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_FPM_BB_DBG_TXFIFO_DATA2_FIELDS[] =
{
    &FPM_FPM_BB_DBG_TXFIFO_DATA2_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_FPM_BB_DBG_TXFIFO_DATA2 *****/
const ru_reg_rec FPM_FPM_BB_DBG_TXFIFO_DATA2_REG =
{
    "FPM_BB_DBG_TXFIFO_DATA2",
#if RU_INCLUDE_DESC
    "FPM_BB_DBG_TXFIFO_DATA2 Register",
    "Data from FPM BB TXFIFO bits [63:32]\n",
#endif
    { FPM_FPM_BB_DBG_TXFIFO_DATA2_REG_OFFSET },
    0,
    0,
    525,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_FPM_BB_DBG_TXFIFO_DATA2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_FPM_BB_DBG_TXFIFO_DATA3, TYPE: Type_FPM_BLOCK_FPM_BB_FPM_BB_DBG_TXFIFO_DATA3
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec FPM_FPM_BB_DBG_TXFIFO_DATA3_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "data\n",
#endif
    { FPM_FPM_BB_DBG_TXFIFO_DATA3_DATA_FIELD_MASK },
    0,
    { FPM_FPM_BB_DBG_TXFIFO_DATA3_DATA_FIELD_WIDTH },
    { FPM_FPM_BB_DBG_TXFIFO_DATA3_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_FPM_BB_DBG_TXFIFO_DATA3_FIELDS[] =
{
    &FPM_FPM_BB_DBG_TXFIFO_DATA3_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_FPM_BB_DBG_TXFIFO_DATA3 *****/
const ru_reg_rec FPM_FPM_BB_DBG_TXFIFO_DATA3_REG =
{
    "FPM_BB_DBG_TXFIFO_DATA3",
#if RU_INCLUDE_DESC
    "FPM_BB_DBG_TXFIFO_DATA3 Register",
    "Data from FPM BB TXFIFO bits [79:64]\n",
#endif
    { FPM_FPM_BB_DBG_TXFIFO_DATA3_REG_OFFSET },
    0,
    0,
    526,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_FPM_BB_DBG_TXFIFO_DATA3_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_FPM_BB_MISC, TYPE: Type_FPM_BLOCK_FPM_BB_FPM_BB_MISC
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: OLD_TASK_NUM *****/
const ru_field_rec FPM_FPM_BB_MISC_OLD_TASK_NUM_FIELD =
{
    "OLD_TASK_NUM",
#if RU_INCLUDE_DESC
    "",
    "old_task_num_format.\n1: default. task_num in legacy bits (according to bb_message excel)\n0: new. task_num in bits 59:56 of bb_fpm_data_in\n",
#endif
    { FPM_FPM_BB_MISC_OLD_TASK_NUM_FIELD_MASK },
    0,
    { FPM_FPM_BB_MISC_OLD_TASK_NUM_FIELD_WIDTH },
    { FPM_FPM_BB_MISC_OLD_TASK_NUM_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ALC_FRE_ARB_RR *****/
const ru_field_rec FPM_FPM_BB_MISC_ALC_FRE_ARB_RR_FIELD =
{
    "ALC_FRE_ARB_RR",
#if RU_INCLUDE_DESC
    "",
    "alloc_free_arb_rr\n0: default(legacy) - no rr, free has priority over alloc.\n1: new option. rr between free and alloc\n",
#endif
    { FPM_FPM_BB_MISC_ALC_FRE_ARB_RR_FIELD_MASK },
    0,
    { FPM_FPM_BB_MISC_ALC_FRE_ARB_RR_FIELD_WIDTH },
    { FPM_FPM_BB_MISC_ALC_FRE_ARB_RR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ALC_FST_ACK *****/
const ru_field_rec FPM_FPM_BB_MISC_ALC_FST_ACK_FIELD =
{
    "ALC_FST_ACK",
#if RU_INCLUDE_DESC
    "",
    "alloc_free_rr\n0: default(legacy) - no fast ack. wait until all free commands were popped, and array was searched.\n1: new option. ack for alloc is returned immediately according to emptiness of alloc prefetch fifo only.\n",
#endif
    { FPM_FPM_BB_MISC_ALC_FST_ACK_FIELD_MASK },
    0,
    { FPM_FPM_BB_MISC_ALC_FST_ACK_FIELD_WIDTH },
    { FPM_FPM_BB_MISC_ALC_FST_ACK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_0_SIZE *****/
const ru_field_rec FPM_FPM_BB_MISC_POOL_0_SIZE_FIELD =
{
    "POOL_0_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Select how many token are related to pool0:\n\n0: 6 tkns,\n1: 7 tkns,\n2: 8 tkns(default),\n3: 20 tkns,\n\n",
#endif
    { FPM_FPM_BB_MISC_POOL_0_SIZE_FIELD_MASK },
    0,
    { FPM_FPM_BB_MISC_POOL_0_SIZE_FIELD_WIDTH },
    { FPM_FPM_BB_MISC_POOL_0_SIZE_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POOL_1_SIZE *****/
const ru_field_rec FPM_FPM_BB_MISC_POOL_1_SIZE_FIELD =
{
    "POOL_1_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Select how many token are related to pool1:\n0: 4 tkns(default),\n1: 5 tkns,\n2: 6 tkns,\n3: 8 tkns\n\nNote: No corresponding configuration for pools_2/3_size, since they always get default: 2 tkns for pool2, 1 tkn for pool3\n\nNote: No corresponding configuration for pools_2/3_size, since they always get default: 2 tkns for pool2, 1 tkn for pool3\n\n",
#endif
    { FPM_FPM_BB_MISC_POOL_1_SIZE_FIELD_MASK },
    0,
    { FPM_FPM_BB_MISC_POOL_1_SIZE_FIELD_WIDTH },
    { FPM_FPM_BB_MISC_POOL_1_SIZE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POOLX_EN *****/
const ru_field_rec FPM_FPM_BB_MISC_POOLX_EN_FIELD =
{
    "POOLX_EN",
#if RU_INCLUDE_DESC
    "",
    "en bit per pool.\nIf the bit is 1, the corresponding pool is enabled, if the bit is not 1, the corresponding pool is not enabled.\n12: pool0 en,\n13: pool1 en,\n14: pool2 en,\n15: pool3 en,\n\n\n",
#endif
    { FPM_FPM_BB_MISC_POOLX_EN_FIELD_MASK },
    0,
    { FPM_FPM_BB_MISC_POOLX_EN_FIELD_WIDTH },
    { FPM_FPM_BB_MISC_POOLX_EN_FIELD_SHIFT },
    15,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_FPM_BB_MISC_FIELDS[] =
{
    &FPM_FPM_BB_MISC_OLD_TASK_NUM_FIELD,
    &FPM_FPM_BB_MISC_ALC_FRE_ARB_RR_FIELD,
    &FPM_FPM_BB_MISC_ALC_FST_ACK_FIELD,
    &FPM_FPM_BB_MISC_POOL_0_SIZE_FIELD,
    &FPM_FPM_BB_MISC_POOL_1_SIZE_FIELD,
    &FPM_FPM_BB_MISC_POOLX_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_FPM_BB_MISC *****/
const ru_reg_rec FPM_FPM_BB_MISC_REG =
{
    "FPM_BB_MISC",
#if RU_INCLUDE_DESC
    "FPM_BB_MISC Register",
    "misc register\n",
#endif
    { FPM_FPM_BB_MISC_REG_OFFSET },
    0,
    0,
    527,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    FPM_FPM_BB_MISC_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: FPM_CLK_GATE_CNTRL, TYPE: Type_FPM_BLOCK_FPM_BB_CLK_GATE_CNTRL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BYPASS_CLK_GATE *****/
const ru_field_rec FPM_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD =
{
    "BYPASS_CLK_GATE",
#if RU_INCLUDE_DESC
    "",
    "If set to 1b1 will disable the clock gate logic such to always enable the clock\n",
#endif
    { FPM_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_MASK },
    0,
    { FPM_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_WIDTH },
    { FPM_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TIMER_VAL *****/
const ru_field_rec FPM_CLK_GATE_CNTRL_TIMER_VAL_FIELD =
{
    "TIMER_VAL",
#if RU_INCLUDE_DESC
    "",
    "For how long should the clock stay active once all conditions for clock disable are met.\n\n\n",
#endif
    { FPM_CLK_GATE_CNTRL_TIMER_VAL_FIELD_MASK },
    0,
    { FPM_CLK_GATE_CNTRL_TIMER_VAL_FIELD_WIDTH },
    { FPM_CLK_GATE_CNTRL_TIMER_VAL_FIELD_SHIFT },
    15,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEEP_ALIVE_EN *****/
const ru_field_rec FPM_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD =
{
    "KEEP_ALIVE_EN",
#if RU_INCLUDE_DESC
    "",
    "Enables the keep alive logic which will periodically enable the clock to assure that no deadlock of clock being removed completely will occur\n",
#endif
    { FPM_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_MASK },
    0,
    { FPM_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_WIDTH },
    { FPM_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEEP_ALIVE_INTRVL *****/
const ru_field_rec FPM_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD =
{
    "KEEP_ALIVE_INTRVL",
#if RU_INCLUDE_DESC
    "",
    "If the KEEP alive option is enabled the field will determine for how many cycles should the clock be active\n",
#endif
    { FPM_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_MASK },
    0,
    { FPM_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_WIDTH },
    { FPM_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEEP_ALIVE_CYC *****/
const ru_field_rec FPM_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD =
{
    "KEEP_ALIVE_CYC",
#if RU_INCLUDE_DESC
    "",
    "If the KEEP alive option is enabled this field will determine for how many cycles should the clock be disabled (minus the KEEP_ALIVE_INTERVAL)\n\nSo KEEP_ALIVE_CYCLE must be larger than KEEP_ALIVE_INTERVAL.\n",
#endif
    { FPM_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_MASK },
    0,
    { FPM_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_WIDTH },
    { FPM_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_SHIFT },
    15,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_CLK_GATE_CNTRL_FIELDS[] =
{
    &FPM_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD,
    &FPM_CLK_GATE_CNTRL_TIMER_VAL_FIELD,
    &FPM_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD,
    &FPM_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD,
    &FPM_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: FPM_CLK_GATE_CNTRL *****/
const ru_reg_rec FPM_CLK_GATE_CNTRL_REG =
{
    "CLK_GATE_CNTRL",
#if RU_INCLUDE_DESC
    "CLOCK_GATE_CONTROL Register",
    "Clock Gate control register including timer config and bypass control\n",
#endif
    { FPM_CLK_GATE_CNTRL_REG_OFFSET },
    0,
    0,
    528,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    FPM_CLK_GATE_CNTRL_FIELDS,
#endif
};

unsigned long FPM_ADDRS[] =
{
    0x82A00000,
};

static const ru_reg_rec *FPM_REGS[] =
{
    &FPM_FPM_CTL_REG,
    &FPM_FPM_CFG1_REG,
    &FPM_FPM_WEIGHT_REG,
    &FPM_FPM_BB_CFG_REG,
    &FPM_POOL1_INTR_MSK_REG,
    &FPM_POOL1_INTR_STS_REG,
    &FPM_POOL1_STALL_MSK_REG,
    &FPM_POOL2_INTR_MSK_REG,
    &FPM_POOL2_INTR_STS_REG,
    &FPM_POOL2_STALL_MSK_REG,
    &FPM_POOL3_INTR_MSK_REG,
    &FPM_POOL3_INTR_STS_REG,
    &FPM_POOL3_STALL_MSK_REG,
    &FPM_POOL1_CFG1_REG,
    &FPM_POOL1_CFG2_REG,
    &FPM_POOL1_CFG3_REG,
    &FPM_POOL1_CFG4_REG,
    &FPM_POOL1_STAT1_REG,
    &FPM_POOL1_STAT2_REG,
    &FPM_POOL1_STAT3_REG,
    &FPM_POOL1_STAT4_REG,
    &FPM_POOL1_STAT5_REG,
    &FPM_POOL1_STAT6_REG,
    &FPM_POOL1_STAT7_REG,
    &FPM_POOL1_STAT8_REG,
    &FPM_POOL2_STAT1_REG,
    &FPM_POOL2_STAT2_REG,
    &FPM_POOL2_STAT3_REG,
    &FPM_POOL2_STAT4_REG,
    &FPM_POOL2_STAT5_REG,
    &FPM_POOL2_STAT6_REG,
    &FPM_POOL2_STAT7_REG,
    &FPM_POOL2_STAT8_REG,
    &FPM_POOL3_STAT1_REG,
    &FPM_POOL3_STAT2_REG,
    &FPM_POOL3_STAT3_REG,
    &FPM_POOL3_STAT5_REG,
    &FPM_POOL3_STAT6_REG,
    &FPM_POOL3_STAT8_REG,
    &FPM_POOL1_XON_XOFF_CFG_REG,
    &FPM_FPM_NOT_EMPTY_CFG_REG,
    &FPM_MEM_CTL_REG,
    &FPM_MEM_DATA1_REG,
    &FPM_MEM_DATA2_REG,
    &FPM_SPARE_REG,
    &FPM_TOKEN_RECOVER_CTL_REG,
    &FPM_SHORT_AGING_TIMER_REG,
    &FPM_LONG_AGING_TIMER_REG,
    &FPM_CACHE_RECYCLE_TIMER_REG,
    &FPM_EXPIRED_TOKEN_COUNT_POOL1_REG,
    &FPM_RECOVERED_TOKEN_COUNT_POOL1_REG,
    &FPM_EXPIRED_TOKEN_COUNT_POOL2_REG,
    &FPM_RECOVERED_TOKEN_COUNT_POOL2_REG,
    &FPM_TOKEN_RECOVER_START_END_POOL1_REG,
    &FPM_TOKEN_RECOVER_START_END_POOL2_REG,
    &FPM_PRBS_INVALID_GEN_REG,
    &FPM_POOL1_ALLOC_DEALLOC_REG,
    &FPM_POOL2_ALLOC_DEALLOC_REG,
    &FPM_POOL3_ALLOC_DEALLOC_REG,
    &FPM_POOL4_ALLOC_DEALLOC_REG,
    &FPM_POOL_MULTI_REG,
    &FPM_POOL1_ALLOC_DEALLOC_1_REG,
    &FPM_POOL2_ALLOC_DEALLOC_1_REG,
    &FPM_POOL3_ALLOC_DEALLOC_1_REG,
    &FPM_POOL4_ALLOC_DEALLOC_1_REG,
    &FPM_POOL_MULTI_1_REG,
    &FPM_SEARCH_DATA_0_REG,
    &FPM_SEARCH_DATA_1_REG,
    &FPM_SEARCH_DATA_2_REG,
    &FPM_SEARCH_DATA_3_REG,
    &FPM_MULTICAST_DATA_REG,
    &FPM_COMPUTE_POOL_DATA_REG,
    &FPM_FPM_BB_FORCE_REG,
    &FPM_FPM_BB_FORCED_CTRL_REG,
    &FPM_FPM_BB_FORCED_ADDR_REG,
    &FPM_FPM_BB_FORCED_DATA_REG,
    &FPM_FPM_BB_DECODE_CFG_REG,
    &FPM_FPM_BB_DBG_CFG_REG,
    &FPM_FPM_BB_DBG_RXFIFO_STS_REG,
    &FPM_FPM_BB_DBG_TXFIFO_STS_REG,
    &FPM_FPM_BB_DBG_RXFIFO_DATA1_REG,
    &FPM_FPM_BB_DBG_RXFIFO_DATA2_REG,
    &FPM_FPM_BB_DBG_TXFIFO_DATA1_REG,
    &FPM_FPM_BB_DBG_TXFIFO_DATA2_REG,
    &FPM_FPM_BB_DBG_TXFIFO_DATA3_REG,
    &FPM_FPM_BB_MISC_REG,
    &FPM_CLK_GATE_CNTRL_REG,
};

const ru_block_rec FPM_BLOCK =
{
    "FPM",
    FPM_ADDRS,
    1,
    87,
    FPM_REGS,
};
