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
 * Field: FPM_FPM_CTL_TP_MUX_CNTRL
 ******************************************************************************/
const ru_field_rec FPM_FPM_CTL_TP_MUX_CNTRL_FIELD =
{
    "TP_MUX_CNTRL",
#if RU_INCLUDE_DESC
    "",
    "Test port mux control bits used to drive test signals from "
    "different submodules.",
#endif
    FPM_FPM_CTL_TP_MUX_CNTRL_FIELD_MASK,
    0,
    FPM_FPM_CTL_TP_MUX_CNTRL_FIELD_WIDTH,
    FPM_FPM_CTL_TP_MUX_CNTRL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_FPM_CTL_ENABLE_HIGH_TOK_ALWAYS
 ******************************************************************************/
const ru_field_rec FPM_FPM_CTL_ENABLE_HIGH_TOK_ALWAYS_FIELD =
{
    "ENABLE_HIGH_TOK_ALWAYS",
#if RU_INCLUDE_DESC
    "",
    "Enable the method2 rule of always allocating higher token "
    "even though there is a lower value token available\n "
    "0 = Disable method2 rule (normal operation - like in A0 chip version)\n "
    "1 = Enable method2 rule \n ",
#endif
    FPM_FPM_CTL_ENABLE_HIGH_TOK_ALWAYS_FIELD_MASK,
    0,
    FPM_FPM_CTL_ENABLE_HIGH_TOK_ALWAYS_FIELD_WIDTH,
    FPM_FPM_CTL_ENABLE_HIGH_TOK_ALWAYS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_FPM_CTL_MEM_CORRUPT_CHECK_DISABLE
 ******************************************************************************/
const ru_field_rec FPM_FPM_CTL_MEM_CORRUPT_CHECK_DISABLE_FIELD =
{
    "MEM_CORRUPT_CHECK_DISABLE",
#if RU_INCLUDE_DESC
    "",
    "Disabling check for index memory corrupt during alloc/free/mcast "
    "updates. This should be used for debug purposes only \n"
    "0 = Enable memory corruption check (normal operation)\n "
    "1 = Disable memory corruption check \n ",
#endif
    FPM_FPM_CTL_MEM_CORRUPT_CHECK_DISABLE_FIELD_MASK,
    0,
    FPM_FPM_CTL_MEM_CORRUPT_CHECK_DISABLE_FIELD_WIDTH,
    FPM_FPM_CTL_MEM_CORRUPT_CHECK_DISABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_FPM_CTL_STOP_ALLOC_CACHE_LOAD
 ******************************************************************************/
const ru_field_rec FPM_FPM_CTL_STOP_ALLOC_CACHE_LOAD_FIELD =
{
    "STOP_ALLOC_CACHE_LOAD",
#if RU_INCLUDE_DESC
    "",
    "Stop loading allocation fifo/cache with new tokens. This is "
    "should be used for debug purposes only\n"
    "0 = Enable loading new tokens (normal operation)\n "
    "1 = Disable loading new tokens\n ",
#endif
    FPM_FPM_CTL_STOP_ALLOC_CACHE_LOAD_FIELD_MASK,
    0,
    FPM_FPM_CTL_STOP_ALLOC_CACHE_LOAD_FIELD_WIDTH,
    FPM_FPM_CTL_STOP_ALLOC_CACHE_LOAD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_FPM_CTL_RESERVED0
 ******************************************************************************/
const ru_field_rec FPM_FPM_CTL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_FPM_CTL_RESERVED0_FIELD_MASK,
    0,
    FPM_FPM_CTL_RESERVED0_FIELD_WIDTH,
    FPM_FPM_CTL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_FPM_CTL_POOL2_ENABLE
 ******************************************************************************/
const ru_field_rec FPM_FPM_CTL_POOL2_ENABLE_FIELD =
{
    "POOL2_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "Enable POOL2 token allocation / deallocation\n"
    "0 = Disabled\n "
    "1 = Enabled\n ",
#endif
    FPM_FPM_CTL_POOL2_ENABLE_FIELD_MASK,
    0,
    FPM_FPM_CTL_POOL2_ENABLE_FIELD_WIDTH,
    FPM_FPM_CTL_POOL2_ENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_FPM_CTL_POOL1_ENABLE
 ******************************************************************************/
const ru_field_rec FPM_FPM_CTL_POOL1_ENABLE_FIELD =
{
    "POOL1_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "Enable POOL1 token allocation / deallocation\n"
    "0 = Disabled\n "
    "1 = Enabled\n ",
#endif
    FPM_FPM_CTL_POOL1_ENABLE_FIELD_MASK,
    0,
    FPM_FPM_CTL_POOL1_ENABLE_FIELD_WIDTH,
    FPM_FPM_CTL_POOL1_ENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_FPM_CTL_STRICT_PRIORITY_REQUEST_TYPE
 ******************************************************************************/
const ru_field_rec FPM_FPM_CTL_STRICT_PRIORITY_REQUEST_TYPE_FIELD =
{
    "STRICT_PRIORITY_REQUEST_TYPE",
#if RU_INCLUDE_DESC
    "",
    "Request that has highest priority. This bit is relevant "
    "when bits[7:5]=3'b000\n"
    "0 = Free/Mcast request\n "
    "1 = Alloc request\n ",
#endif
    FPM_FPM_CTL_STRICT_PRIORITY_REQUEST_TYPE_FIELD_MASK,
    0,
    FPM_FPM_CTL_STRICT_PRIORITY_REQUEST_TYPE_FIELD_WIDTH,
    FPM_FPM_CTL_STRICT_PRIORITY_REQUEST_TYPE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_FPM_CTL_FPM_BB_SOFT_RESET
 ******************************************************************************/
const ru_field_rec FPM_FPM_CTL_FPM_BB_SOFT_RESET_FIELD =
{
    "FPM_BB_SOFT_RESET",
#if RU_INCLUDE_DESC
    "",
    "Set to 1 to hold the FPM Broadbus interface in reset. "
    "This is useful for maintaining a known state on that "
    "interface when Runner is powered down.",
#endif
    FPM_FPM_CTL_FPM_BB_SOFT_RESET_FIELD_MASK,
    0,
    FPM_FPM_CTL_FPM_BB_SOFT_RESET_FIELD_WIDTH,
    FPM_FPM_CTL_FPM_BB_SOFT_RESET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_FPM_CTL_WEIGHT_FOR_ROUND_ROBIN_POLICY
 ******************************************************************************/
const ru_field_rec FPM_FPM_CTL_WEIGHT_FOR_ROUND_ROBIN_POLICY_FIELD =
{
    "WEIGHT_FOR_ROUND_ROBIN_POLICY",
#if RU_INCLUDE_DESC
    "",
    "Weight value for weighted-round-robin arbitration policy, "
    "ie., when bits[7:5]=3'b010. When this value is zero, it is "
    "interpreted as 64 (decimal)",
#endif
    FPM_FPM_CTL_WEIGHT_FOR_ROUND_ROBIN_POLICY_FIELD_MASK,
    0,
    FPM_FPM_CTL_WEIGHT_FOR_ROUND_ROBIN_POLICY_FIELD_WIDTH,
    FPM_FPM_CTL_WEIGHT_FOR_ROUND_ROBIN_POLICY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_FPM_CTL_ARBITRATION_POLICY
 ******************************************************************************/
const ru_field_rec FPM_FPM_CTL_ARBITRATION_POLICY_FIELD =
{
    "ARBITRATION_POLICY",
#if RU_INCLUDE_DESC
    "",
    "Desired Arbitration Policy\n"
    "3'b000 = Strict Priority\n"
    "3'b001 = Normal Round Robin\n"
    "3'b010 = Weighted Round Robin\n"
    "3'b011 to 3'b111 = Reserved\n",
#endif
    FPM_FPM_CTL_ARBITRATION_POLICY_FIELD_MASK,
    0,
    FPM_FPM_CTL_ARBITRATION_POLICY_FIELD_WIDTH,
    FPM_FPM_CTL_ARBITRATION_POLICY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_FPM_CTL_INIT_MEM
 ******************************************************************************/
const ru_field_rec FPM_FPM_CTL_INIT_MEM_FIELD =
{
    "INIT_MEM",
#if RU_INCLUDE_DESC
    "",
    "Clear memory - Initialize all bits of the usage index array "
    "memory to zero's\n"
    "This is a self clearing bit. Once software writes a 1'b1 to enable, "
    "hardware initializes the memory and resets this bit back to 1'b0 at "
    "completion of initialization. Software can poll this bit and check "
    "for a value a zero that indicates initialization completion status ",
#endif
    FPM_FPM_CTL_INIT_MEM_FIELD_MASK,
    0,
    FPM_FPM_CTL_INIT_MEM_FIELD_WIDTH,
    FPM_FPM_CTL_INIT_MEM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_FPM_CTL_INIT_MEM_POOL2
 ******************************************************************************/
const ru_field_rec FPM_FPM_CTL_INIT_MEM_POOL2_FIELD =
{
    "INIT_MEM_POOL2",
#if RU_INCLUDE_DESC
    "",
    "Clear memory - Initialize all bits of the usage index array "
    "memory to zero's\n"
    "This is a self clearing bit. Once software writes a 1'b1 to enable, "
    "hardware initializes the memory and resets this bit back to 1'b0 at "
    "completion of initialization. Software can poll this bit and check "
    "for a value a zero that indicates initialization completion status ",
#endif
    FPM_FPM_CTL_INIT_MEM_POOL2_FIELD_MASK,
    0,
    FPM_FPM_CTL_INIT_MEM_POOL2_FIELD_WIDTH,
    FPM_FPM_CTL_INIT_MEM_POOL2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_FPM_CTL_RESERVED1
 ******************************************************************************/
const ru_field_rec FPM_FPM_CTL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_FPM_CTL_RESERVED1_FIELD_MASK,
    0,
    FPM_FPM_CTL_RESERVED1_FIELD_WIDTH,
    FPM_FPM_CTL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_FPM_CFG1_RESERVED0
 ******************************************************************************/
const ru_field_rec FPM_FPM_CFG1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_FPM_CFG1_RESERVED0_FIELD_MASK,
    0,
    FPM_FPM_CFG1_RESERVED0_FIELD_WIDTH,
    FPM_FPM_CFG1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_FPM_CFG1_POOL4_CACHE_BYPASS_EN
 ******************************************************************************/
const ru_field_rec FPM_FPM_CFG1_POOL4_CACHE_BYPASS_EN_FIELD =
{
    "POOL4_CACHE_BYPASS_EN",
#if RU_INCLUDE_DESC
    "",
    "Enable Cache bypass mode\n"
    "0 = Disable\n "
    "1 = Enable\n ",
#endif
    FPM_FPM_CFG1_POOL4_CACHE_BYPASS_EN_FIELD_MASK,
    0,
    FPM_FPM_CFG1_POOL4_CACHE_BYPASS_EN_FIELD_WIDTH,
    FPM_FPM_CFG1_POOL4_CACHE_BYPASS_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_FPM_CFG1_POOL3_CACHE_BYPASS_EN
 ******************************************************************************/
const ru_field_rec FPM_FPM_CFG1_POOL3_CACHE_BYPASS_EN_FIELD =
{
    "POOL3_CACHE_BYPASS_EN",
#if RU_INCLUDE_DESC
    "",
    "Enable Cache bypass mode\n"
    "0 = Disable\n "
    "1 = Enable\n ",
#endif
    FPM_FPM_CFG1_POOL3_CACHE_BYPASS_EN_FIELD_MASK,
    0,
    FPM_FPM_CFG1_POOL3_CACHE_BYPASS_EN_FIELD_WIDTH,
    FPM_FPM_CFG1_POOL3_CACHE_BYPASS_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_FPM_CFG1_POOL2_CACHE_BYPASS_EN
 ******************************************************************************/
const ru_field_rec FPM_FPM_CFG1_POOL2_CACHE_BYPASS_EN_FIELD =
{
    "POOL2_CACHE_BYPASS_EN",
#if RU_INCLUDE_DESC
    "",
    "Enable Cache bypass mode\n"
    "0 = Disable\n "
    "1 = Enable\n ",
#endif
    FPM_FPM_CFG1_POOL2_CACHE_BYPASS_EN_FIELD_MASK,
    0,
    FPM_FPM_CFG1_POOL2_CACHE_BYPASS_EN_FIELD_WIDTH,
    FPM_FPM_CFG1_POOL2_CACHE_BYPASS_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_FPM_CFG1_POOL1_CACHE_BYPASS_EN
 ******************************************************************************/
const ru_field_rec FPM_FPM_CFG1_POOL1_CACHE_BYPASS_EN_FIELD =
{
    "POOL1_CACHE_BYPASS_EN",
#if RU_INCLUDE_DESC
    "",
    "Enable Cache bypass mode\n"
    "0 = Disable\n "
    "1 = Enable\n ",
#endif
    FPM_FPM_CFG1_POOL1_CACHE_BYPASS_EN_FIELD_MASK,
    0,
    FPM_FPM_CFG1_POOL1_CACHE_BYPASS_EN_FIELD_WIDTH,
    FPM_FPM_CFG1_POOL1_CACHE_BYPASS_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_FPM_CFG1_POOL4_SEARCH_MODE
 ******************************************************************************/
const ru_field_rec FPM_FPM_CFG1_POOL4_SEARCH_MODE_FIELD =
{
    "POOL4_SEARCH_MODE",
#if RU_INCLUDE_DESC
    "",
    "Index memory search method\n"
    "(For more info refer to FPM architecture wiki page)\n"
    "0 = Method 1\n "
    "1 = Method 2\n ",
#endif
    FPM_FPM_CFG1_POOL4_SEARCH_MODE_FIELD_MASK,
    0,
    FPM_FPM_CFG1_POOL4_SEARCH_MODE_FIELD_WIDTH,
    FPM_FPM_CFG1_POOL4_SEARCH_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_FPM_CFG1_POOL3_SEARCH_MODE
 ******************************************************************************/
const ru_field_rec FPM_FPM_CFG1_POOL3_SEARCH_MODE_FIELD =
{
    "POOL3_SEARCH_MODE",
#if RU_INCLUDE_DESC
    "",
    "Index memory search method\n"
    "(For more info refer to FPM architecture wiki page)\n"
    "0 = Method 1\n "
    "1 = Method 2\n ",
#endif
    FPM_FPM_CFG1_POOL3_SEARCH_MODE_FIELD_MASK,
    0,
    FPM_FPM_CFG1_POOL3_SEARCH_MODE_FIELD_WIDTH,
    FPM_FPM_CFG1_POOL3_SEARCH_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_FPM_CFG1_POOL2_SEARCH_MODE
 ******************************************************************************/
const ru_field_rec FPM_FPM_CFG1_POOL2_SEARCH_MODE_FIELD =
{
    "POOL2_SEARCH_MODE",
#if RU_INCLUDE_DESC
    "",
    "Index memory search method\n"
    "(For more info refer to FPM architecture wiki page)\n"
    "0 = Method 1\n "
    "1 = Method 2\n ",
#endif
    FPM_FPM_CFG1_POOL2_SEARCH_MODE_FIELD_MASK,
    0,
    FPM_FPM_CFG1_POOL2_SEARCH_MODE_FIELD_WIDTH,
    FPM_FPM_CFG1_POOL2_SEARCH_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_FPM_CFG1_POOL1_SEARCH_MODE
 ******************************************************************************/
const ru_field_rec FPM_FPM_CFG1_POOL1_SEARCH_MODE_FIELD =
{
    "POOL1_SEARCH_MODE",
#if RU_INCLUDE_DESC
    "",
    "Index memory search method\n"
    "(For more info refer to FPM architecture wiki page)\n"
    "0 = Method 1\n "
    "1 = Method 2\n ",
#endif
    FPM_FPM_CFG1_POOL1_SEARCH_MODE_FIELD_MASK,
    0,
    FPM_FPM_CFG1_POOL1_SEARCH_MODE_FIELD_WIDTH,
    FPM_FPM_CFG1_POOL1_SEARCH_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_FPM_WEIGHT_DDR1_FREE_WEIGHT
 ******************************************************************************/
const ru_field_rec FPM_FPM_WEIGHT_DDR1_FREE_WEIGHT_FIELD =
{
    "DDR1_FREE_WEIGHT",
#if RU_INCLUDE_DESC
    "",
    "Weight assigned to each free to pool for DDR1\n",
#endif
    FPM_FPM_WEIGHT_DDR1_FREE_WEIGHT_FIELD_MASK,
    0,
    FPM_FPM_WEIGHT_DDR1_FREE_WEIGHT_FIELD_WIDTH,
    FPM_FPM_WEIGHT_DDR1_FREE_WEIGHT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_FPM_WEIGHT_DDR1_ALLOC_WEIGHT
 ******************************************************************************/
const ru_field_rec FPM_FPM_WEIGHT_DDR1_ALLOC_WEIGHT_FIELD =
{
    "DDR1_ALLOC_WEIGHT",
#if RU_INCLUDE_DESC
    "",
    "Weight assigned to each alloc from pool for DDR1\n",
#endif
    FPM_FPM_WEIGHT_DDR1_ALLOC_WEIGHT_FIELD_MASK,
    0,
    FPM_FPM_WEIGHT_DDR1_ALLOC_WEIGHT_FIELD_WIDTH,
    FPM_FPM_WEIGHT_DDR1_ALLOC_WEIGHT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_FPM_WEIGHT_DDR0_FREE_WEIGHT
 ******************************************************************************/
const ru_field_rec FPM_FPM_WEIGHT_DDR0_FREE_WEIGHT_FIELD =
{
    "DDR0_FREE_WEIGHT",
#if RU_INCLUDE_DESC
    "",
    "Weight assigned to each free to pool for DDR0\n",
#endif
    FPM_FPM_WEIGHT_DDR0_FREE_WEIGHT_FIELD_MASK,
    0,
    FPM_FPM_WEIGHT_DDR0_FREE_WEIGHT_FIELD_WIDTH,
    FPM_FPM_WEIGHT_DDR0_FREE_WEIGHT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_FPM_WEIGHT_DDR0_ALLOC_WEIGHT
 ******************************************************************************/
const ru_field_rec FPM_FPM_WEIGHT_DDR0_ALLOC_WEIGHT_FIELD =
{
    "DDR0_ALLOC_WEIGHT",
#if RU_INCLUDE_DESC
    "",
    "Weight assigned to each alloc from pool for DDR0\n",
#endif
    FPM_FPM_WEIGHT_DDR0_ALLOC_WEIGHT_FIELD_MASK,
    0,
    FPM_FPM_WEIGHT_DDR0_ALLOC_WEIGHT_FIELD_WIDTH,
    FPM_FPM_WEIGHT_DDR0_ALLOC_WEIGHT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_FPM_BB_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec FPM_FPM_BB_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_FPM_BB_CFG_RESERVED0_FIELD_MASK,
    0,
    FPM_FPM_BB_CFG_RESERVED0_FIELD_WIDTH,
    FPM_FPM_BB_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_FPM_BB_CFG_BB_DDR_SEL
 ******************************************************************************/
const ru_field_rec FPM_FPM_BB_CFG_BB_DDR_SEL_FIELD =
{
    "BB_DDR_SEL",
#if RU_INCLUDE_DESC
    "",
    "Select pool/DDR to be used when FPM_BB allocates tokens\n"
    "11 = reserved\n "
    "10 = allocate from both pools\n "
    "01 = pool1/DDR1\n "
    "00 = pool0/DDR0\n ",
#endif
    FPM_FPM_BB_CFG_BB_DDR_SEL_FIELD_MASK,
    0,
    FPM_FPM_BB_CFG_BB_DDR_SEL_FIELD_WIDTH,
    FPM_FPM_BB_CFG_BB_DDR_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_INTR_MSK_RESERVED0
 ******************************************************************************/
const ru_field_rec FPM_POOL1_INTR_MSK_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_POOL1_INTR_MSK_RESERVED0_FIELD_MASK,
    0,
    FPM_POOL1_INTR_MSK_RESERVED0_FIELD_WIDTH,
    FPM_POOL1_INTR_MSK_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_INTR_MSK_EXPIRED_TOKEN_RECOV_MSK
 ******************************************************************************/
const ru_field_rec FPM_POOL1_INTR_MSK_EXPIRED_TOKEN_RECOV_MSK_FIELD =
{
    "EXPIRED_TOKEN_RECOV_MSK",
#if RU_INCLUDE_DESC
    "",
    "Expired token recovered interrupt mask. \n",
#endif
    FPM_POOL1_INTR_MSK_EXPIRED_TOKEN_RECOV_MSK_FIELD_MASK,
    0,
    FPM_POOL1_INTR_MSK_EXPIRED_TOKEN_RECOV_MSK_FIELD_WIDTH,
    FPM_POOL1_INTR_MSK_EXPIRED_TOKEN_RECOV_MSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_INTR_MSK_EXPIRED_TOKEN_DET_MSK
 ******************************************************************************/
const ru_field_rec FPM_POOL1_INTR_MSK_EXPIRED_TOKEN_DET_MSK_FIELD =
{
    "EXPIRED_TOKEN_DET_MSK",
#if RU_INCLUDE_DESC
    "",
    "Expired token detect interrupt mask. \n",
#endif
    FPM_POOL1_INTR_MSK_EXPIRED_TOKEN_DET_MSK_FIELD_MASK,
    0,
    FPM_POOL1_INTR_MSK_EXPIRED_TOKEN_DET_MSK_FIELD_WIDTH,
    FPM_POOL1_INTR_MSK_EXPIRED_TOKEN_DET_MSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_INTR_MSK_ILLEGAL_ALLOC_REQUEST_MSK
 ******************************************************************************/
const ru_field_rec FPM_POOL1_INTR_MSK_ILLEGAL_ALLOC_REQUEST_MSK_FIELD =
{
    "ILLEGAL_ALLOC_REQUEST_MSK",
#if RU_INCLUDE_DESC
    "",
    "Illegal token request interrupt mask. \n",
#endif
    FPM_POOL1_INTR_MSK_ILLEGAL_ALLOC_REQUEST_MSK_FIELD_MASK,
    0,
    FPM_POOL1_INTR_MSK_ILLEGAL_ALLOC_REQUEST_MSK_FIELD_WIDTH,
    FPM_POOL1_INTR_MSK_ILLEGAL_ALLOC_REQUEST_MSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_INTR_MSK_ILLEGAL_ADDRESS_ACCESS_MSK
 ******************************************************************************/
const ru_field_rec FPM_POOL1_INTR_MSK_ILLEGAL_ADDRESS_ACCESS_MSK_FIELD =
{
    "ILLEGAL_ADDRESS_ACCESS_MSK",
#if RU_INCLUDE_DESC
    "",
    "Illegal/un-implemented register/memory space access  interrupt mask. \n",
#endif
    FPM_POOL1_INTR_MSK_ILLEGAL_ADDRESS_ACCESS_MSK_FIELD_MASK,
    0,
    FPM_POOL1_INTR_MSK_ILLEGAL_ADDRESS_ACCESS_MSK_FIELD_WIDTH,
    FPM_POOL1_INTR_MSK_ILLEGAL_ADDRESS_ACCESS_MSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_INTR_MSK_XON_MSK
 ******************************************************************************/
const ru_field_rec FPM_POOL1_INTR_MSK_XON_MSK_FIELD =
{
    "XON_MSK",
#if RU_INCLUDE_DESC
    "",
    "XON_STATE interrupt mask. \n",
#endif
    FPM_POOL1_INTR_MSK_XON_MSK_FIELD_MASK,
    0,
    FPM_POOL1_INTR_MSK_XON_MSK_FIELD_WIDTH,
    FPM_POOL1_INTR_MSK_XON_MSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_INTR_MSK_XOFF_MSK
 ******************************************************************************/
const ru_field_rec FPM_POOL1_INTR_MSK_XOFF_MSK_FIELD =
{
    "XOFF_MSK",
#if RU_INCLUDE_DESC
    "",
    "XOFF_STATE interrupt mask. \n",
#endif
    FPM_POOL1_INTR_MSK_XOFF_MSK_FIELD_MASK,
    0,
    FPM_POOL1_INTR_MSK_XOFF_MSK_FIELD_WIDTH,
    FPM_POOL1_INTR_MSK_XOFF_MSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_INTR_MSK_MEMORY_CORRUPT_MSK
 ******************************************************************************/
const ru_field_rec FPM_POOL1_INTR_MSK_MEMORY_CORRUPT_MSK_FIELD =
{
    "MEMORY_CORRUPT_MSK",
#if RU_INCLUDE_DESC
    "",
    "Index Memory corrupt interrupt mask. \n",
#endif
    FPM_POOL1_INTR_MSK_MEMORY_CORRUPT_MSK_FIELD_MASK,
    0,
    FPM_POOL1_INTR_MSK_MEMORY_CORRUPT_MSK_FIELD_WIDTH,
    FPM_POOL1_INTR_MSK_MEMORY_CORRUPT_MSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_INTR_MSK_POOL_DIS_FREE_MULTI_MSK
 ******************************************************************************/
const ru_field_rec FPM_POOL1_INTR_MSK_POOL_DIS_FREE_MULTI_MSK_FIELD =
{
    "POOL_DIS_FREE_MULTI_MSK",
#if RU_INCLUDE_DESC
    "",
    "Free or Mcast update on disabled pool interrupt mask . \n",
#endif
    FPM_POOL1_INTR_MSK_POOL_DIS_FREE_MULTI_MSK_FIELD_MASK,
    0,
    FPM_POOL1_INTR_MSK_POOL_DIS_FREE_MULTI_MSK_FIELD_WIDTH,
    FPM_POOL1_INTR_MSK_POOL_DIS_FREE_MULTI_MSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_INTR_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK
 ******************************************************************************/
const ru_field_rec FPM_POOL1_INTR_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD =
{
    "MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK",
#if RU_INCLUDE_DESC
    "",
    "Token multi-cast value update request with index out-of-range. \n",
#endif
    FPM_POOL1_INTR_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD_MASK,
    0,
    FPM_POOL1_INTR_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD_WIDTH,
    FPM_POOL1_INTR_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_INTR_MSK_MULTI_TOKEN_NO_VALID_MSK
 ******************************************************************************/
const ru_field_rec FPM_POOL1_INTR_MSK_MULTI_TOKEN_NO_VALID_MSK_FIELD =
{
    "MULTI_TOKEN_NO_VALID_MSK",
#if RU_INCLUDE_DESC
    "",
    "Token multi-cast value update request with invalid token. \n",
#endif
    FPM_POOL1_INTR_MSK_MULTI_TOKEN_NO_VALID_MSK_FIELD_MASK,
    0,
    FPM_POOL1_INTR_MSK_MULTI_TOKEN_NO_VALID_MSK_FIELD_WIDTH,
    FPM_POOL1_INTR_MSK_MULTI_TOKEN_NO_VALID_MSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_INTR_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK
 ******************************************************************************/
const ru_field_rec FPM_POOL1_INTR_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD =
{
    "FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK",
#if RU_INCLUDE_DESC
    "",
    "De-allocation token request with index out-of-range. \n",
#endif
    FPM_POOL1_INTR_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD_MASK,
    0,
    FPM_POOL1_INTR_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD_WIDTH,
    FPM_POOL1_INTR_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_INTR_MSK_FREE_TOKEN_NO_VALID_MSK
 ******************************************************************************/
const ru_field_rec FPM_POOL1_INTR_MSK_FREE_TOKEN_NO_VALID_MSK_FIELD =
{
    "FREE_TOKEN_NO_VALID_MSK",
#if RU_INCLUDE_DESC
    "",
    "De-allocation token request with invalid token. \n",
#endif
    FPM_POOL1_INTR_MSK_FREE_TOKEN_NO_VALID_MSK_FIELD_MASK,
    0,
    FPM_POOL1_INTR_MSK_FREE_TOKEN_NO_VALID_MSK_FIELD_WIDTH,
    FPM_POOL1_INTR_MSK_FREE_TOKEN_NO_VALID_MSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_INTR_MSK_POOL_FULL_MSK
 ******************************************************************************/
const ru_field_rec FPM_POOL1_INTR_MSK_POOL_FULL_MSK_FIELD =
{
    "POOL_FULL_MSK",
#if RU_INCLUDE_DESC
    "",
    "Usage Index Pool is fully allocated interrupt mask. \n",
#endif
    FPM_POOL1_INTR_MSK_POOL_FULL_MSK_FIELD_MASK,
    0,
    FPM_POOL1_INTR_MSK_POOL_FULL_MSK_FIELD_WIDTH,
    FPM_POOL1_INTR_MSK_POOL_FULL_MSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_INTR_MSK_FREE_FIFO_FULL_MSK
 ******************************************************************************/
const ru_field_rec FPM_POOL1_INTR_MSK_FREE_FIFO_FULL_MSK_FIELD =
{
    "FREE_FIFO_FULL_MSK",
#if RU_INCLUDE_DESC
    "",
    "De-Allocation FIFO Full Interrupt mask. \n",
#endif
    FPM_POOL1_INTR_MSK_FREE_FIFO_FULL_MSK_FIELD_MASK,
    0,
    FPM_POOL1_INTR_MSK_FREE_FIFO_FULL_MSK_FIELD_WIDTH,
    FPM_POOL1_INTR_MSK_FREE_FIFO_FULL_MSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_INTR_MSK_ALLOC_FIFO_FULL_MSK
 ******************************************************************************/
const ru_field_rec FPM_POOL1_INTR_MSK_ALLOC_FIFO_FULL_MSK_FIELD =
{
    "ALLOC_FIFO_FULL_MSK",
#if RU_INCLUDE_DESC
    "",
    "Allocation FIFO Full Interrupt mask. \n",
#endif
    FPM_POOL1_INTR_MSK_ALLOC_FIFO_FULL_MSK_FIELD_MASK,
    0,
    FPM_POOL1_INTR_MSK_ALLOC_FIFO_FULL_MSK_FIELD_WIDTH,
    FPM_POOL1_INTR_MSK_ALLOC_FIFO_FULL_MSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_INTR_STS_RESERVED0
 ******************************************************************************/
const ru_field_rec FPM_POOL1_INTR_STS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_POOL1_INTR_STS_RESERVED0_FIELD_MASK,
    0,
    FPM_POOL1_INTR_STS_RESERVED0_FIELD_WIDTH,
    FPM_POOL1_INTR_STS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_INTR_STS_EXPIRED_TOKEN_RECOV_STS
 ******************************************************************************/
const ru_field_rec FPM_POOL1_INTR_STS_EXPIRED_TOKEN_RECOV_STS_FIELD =
{
    "EXPIRED_TOKEN_RECOV_STS",
#if RU_INCLUDE_DESC
    "",
    "Expired token recovered interrupt. This is set when an expired token has been recovered"
    "and returned to pool as an available token. \n",
#endif
    FPM_POOL1_INTR_STS_EXPIRED_TOKEN_RECOV_STS_FIELD_MASK,
    0,
    FPM_POOL1_INTR_STS_EXPIRED_TOKEN_RECOV_STS_FIELD_WIDTH,
    FPM_POOL1_INTR_STS_EXPIRED_TOKEN_RECOV_STS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_INTR_STS_EXPIRED_TOKEN_DET_STS
 ******************************************************************************/
const ru_field_rec FPM_POOL1_INTR_STS_EXPIRED_TOKEN_DET_STS_FIELD =
{
    "EXPIRED_TOKEN_DET_STS",
#if RU_INCLUDE_DESC
    "",
    "Expired token detect interrupt. This is set when the token recovery logic detects "
    "a token that has been held for the entire duration of the aging timer. \n",
#endif
    FPM_POOL1_INTR_STS_EXPIRED_TOKEN_DET_STS_FIELD_MASK,
    0,
    FPM_POOL1_INTR_STS_EXPIRED_TOKEN_DET_STS_FIELD_WIDTH,
    FPM_POOL1_INTR_STS_EXPIRED_TOKEN_DET_STS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_INTR_STS_ILLEGAL_ALLOC_REQUEST_STS
 ******************************************************************************/
const ru_field_rec FPM_POOL1_INTR_STS_ILLEGAL_ALLOC_REQUEST_STS_FIELD =
{
    "ILLEGAL_ALLOC_REQUEST_STS",
#if RU_INCLUDE_DESC
    "",
    "Illegal token request interrupt. This will be active when the pool is disabled, there is "
    "a request for a new token and the alloc fifo for the selected token size is empty. "
    "Along with interrupt being sent "
    "an error reply packet will be sent out with o_ubus_error_out asserted. \n",
#endif
    FPM_POOL1_INTR_STS_ILLEGAL_ALLOC_REQUEST_STS_FIELD_MASK,
    0,
    FPM_POOL1_INTR_STS_ILLEGAL_ALLOC_REQUEST_STS_FIELD_WIDTH,
    FPM_POOL1_INTR_STS_ILLEGAL_ALLOC_REQUEST_STS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_INTR_STS_ILLEGAL_ADDRESS_ACCESS_STS
 ******************************************************************************/
const ru_field_rec FPM_POOL1_INTR_STS_ILLEGAL_ADDRESS_ACCESS_STS_FIELD =
{
    "ILLEGAL_ADDRESS_ACCESS_STS",
#if RU_INCLUDE_DESC
    "",
    "Illegal/un-implemented register/memory space access interrupt. This will be active "
    "when there is an attempt to read from an unimplemented register or memory space. Along "
    "with interrupt being sent an error reply packet will be sent out with "
    "o_ubus_error_out asserted. \n",
#endif
    FPM_POOL1_INTR_STS_ILLEGAL_ADDRESS_ACCESS_STS_FIELD_MASK,
    0,
    FPM_POOL1_INTR_STS_ILLEGAL_ADDRESS_ACCESS_STS_FIELD_WIDTH,
    FPM_POOL1_INTR_STS_ILLEGAL_ADDRESS_ACCESS_STS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_INTR_STS_XON_STATE_STS
 ******************************************************************************/
const ru_field_rec FPM_POOL1_INTR_STS_XON_STATE_STS_FIELD =
{
    "XON_STATE_STS",
#if RU_INCLUDE_DESC
    "",
    "Number of available tokens is greater than or equal to XON_THRESHOLD value in XON/XOFF "
    "Threshold configuration register. This is a functional status bit, not an error status bit. "
    "Using this information FPM generates \"backpressure\" output signal that is used by other "
    "UBUS client logics to throttle its operation. For example, UNIMAC logic can use \"backpressure\" "
    "signal to transfer \"PAUSE\" Ethernet flow control packets to throttle incoming frames on "
    "Ethernet interface. \n",
#endif
    FPM_POOL1_INTR_STS_XON_STATE_STS_FIELD_MASK,
    0,
    FPM_POOL1_INTR_STS_XON_STATE_STS_FIELD_WIDTH,
    FPM_POOL1_INTR_STS_XON_STATE_STS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_INTR_STS_XOFF_STATE_STS
 ******************************************************************************/
const ru_field_rec FPM_POOL1_INTR_STS_XOFF_STATE_STS_FIELD =
{
    "XOFF_STATE_STS",
#if RU_INCLUDE_DESC
    "",
    "Number of available tokens is less than or equal to XOFF_THRESHOLD value in XON/XOFF "
    "Threshold configuration register. This is a functional status bit, not an error status bit. "
    "Using this information FPM generates \"backpressure\" output signal that is used by other "
    "UBUS client logics to throttle its operation. For example, UNIMAC logic can use \"backpressure\" "
    "signal to transfer \"PAUSE\" Ethernet flow control packets to throttle incoming frames on "
    "Ethernet interface. \n",
#endif
    FPM_POOL1_INTR_STS_XOFF_STATE_STS_FIELD_MASK,
    0,
    FPM_POOL1_INTR_STS_XOFF_STATE_STS_FIELD_WIDTH,
    FPM_POOL1_INTR_STS_XOFF_STATE_STS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_INTR_STS_MEMORY_CORRUPT_STS
 ******************************************************************************/
const ru_field_rec FPM_POOL1_INTR_STS_MEMORY_CORRUPT_STS_FIELD =
{
    "MEMORY_CORRUPT_STS",
#if RU_INCLUDE_DESC
    "",
    "Index Memory is corrupted. \n"
    "During updates of the usage array, token manager checks if the use count and search tree value "
    "in the array has a legal value. If the use count or search tree value is not correct before "
    "updating, logic generates an error and interrupt. As long as the interrupt is active no more "
    "valid tokens will be allocated because this is a catastrophic error. Following are the two "
    "error conditions that are checked - \n"
    "1. During search for a free token, a particular token use count value indicates it is "
    "allocated (use count is greater than 0), but corresponding upper level search tree value "
    "indicates the token is still available (with bit value of 1'b0, instead of 1'b1). This is "
    "an error. \n"
    "2. During search for a free token, a particular token use count value indicates that it is "
    "free (use count is 0), but corresponding upper level search tree value indicates the token "
    "is not available (with bit value of 1'b1, instead of 1'b0). This is an error. \n",
#endif
    FPM_POOL1_INTR_STS_MEMORY_CORRUPT_STS_FIELD_MASK,
    0,
    FPM_POOL1_INTR_STS_MEMORY_CORRUPT_STS_FIELD_WIDTH,
    FPM_POOL1_INTR_STS_MEMORY_CORRUPT_STS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_INTR_STS_POOL_DIS_FREE_MULTI_STS
 ******************************************************************************/
const ru_field_rec FPM_POOL1_INTR_STS_POOL_DIS_FREE_MULTI_STS_FIELD =
{
    "POOL_DIS_FREE_MULTI_STS",
#if RU_INCLUDE_DESC
    "",
    "Free or Mcast update on disabled pool interrupt. \n"
    "This bit goes active when a free or multi-cast request is received and FPM is not enabled, "
    "i.e., pool enable bit in FPM control register is not set to 1'b1. \n",
#endif
    FPM_POOL1_INTR_STS_POOL_DIS_FREE_MULTI_STS_FIELD_MASK,
    0,
    FPM_POOL1_INTR_STS_POOL_DIS_FREE_MULTI_STS_FIELD_WIDTH,
    FPM_POOL1_INTR_STS_POOL_DIS_FREE_MULTI_STS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_INTR_STS_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS
 ******************************************************************************/
const ru_field_rec FPM_POOL1_INTR_STS_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD =
{
    "MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS",
#if RU_INCLUDE_DESC
    "",
    "Token multi-cast value update request with index out-of-range Interrupt. \n"
    "This is set when the token index is not aligned to the pool size. This is determined by "
    "examining the pool select field (bits[29:28]) and the 3 lsbs of the token index (bits[14:12]). "
    "There is no associated count for this error. Note: this error is not checked if auto_pool_en is "
    "set. The auto_pool_en bit is always set when using the new token format without a pool select field. \n",
#endif
    FPM_POOL1_INTR_STS_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD_MASK,
    0,
    FPM_POOL1_INTR_STS_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD_WIDTH,
    FPM_POOL1_INTR_STS_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_INTR_STS_MULTI_TOKEN_NO_VALID_STS
 ******************************************************************************/
const ru_field_rec FPM_POOL1_INTR_STS_MULTI_TOKEN_NO_VALID_STS_FIELD =
{
    "MULTI_TOKEN_NO_VALID_STS",
#if RU_INCLUDE_DESC
    "",
    "Token multi-cast value update request with invalid token Interrupt. \n"
    "Invalid multi-cast token is determined when one or more the following conditions are met - \n"
    "1. Incoming multi-cast request token has valid bit (bit[31]) set to 1'b0 \n"
    "2. Incoming multi-cast request token index is not aligned to the pool size indicated "
    "by the pool select field (bits[29:28]) \n"
    "3. Incoming multi-cast request token has use count field (bit[6:0]) set to zero \n"
    "4. Incoming multi-cast request token entry in the usage array indicates it is not an allocated "
    "token, i.e., associated use count value for this count in the usage array is zero \n"
    "5. After updating the use count value, the new use count value exceeds 0x7E \n"
    "Note: item 2 is not checked if auto_pool_en is set. The auto_pool_en bit is always "
    "set when using the new token format without a pool select field. \n",
#endif
    FPM_POOL1_INTR_STS_MULTI_TOKEN_NO_VALID_STS_FIELD_MASK,
    0,
    FPM_POOL1_INTR_STS_MULTI_TOKEN_NO_VALID_STS_FIELD_WIDTH,
    FPM_POOL1_INTR_STS_MULTI_TOKEN_NO_VALID_STS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_INTR_STS_FREE_TOKEN_INDEX_OUT_OF_RANGE_STS
 ******************************************************************************/
const ru_field_rec FPM_POOL1_INTR_STS_FREE_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD =
{
    "FREE_TOKEN_INDEX_OUT_OF_RANGE_STS",
#if RU_INCLUDE_DESC
    "",
    "De-allocation token request with index out-of-range Interrupt. \n"
    "Free token index out of range is determined when one or more of the following conditions are met - \n"
    "1. Incoming free request token index is not aligned to the pool size indicated "
    "by the pool select field (bits[29:28]) \n"
    "2. The buffer size indicated by the size field (bits[11:0]) is greater than the size of the allocated "
    "token. \n"
    "There is no associated count for this error. "
    "Note: item 1 is not checked if auto_pool_en is set. The auto_pool_en bit is always "
    "set when using the new token format without a pool select field. \n",
#endif
    FPM_POOL1_INTR_STS_FREE_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD_MASK,
    0,
    FPM_POOL1_INTR_STS_FREE_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD_WIDTH,
    FPM_POOL1_INTR_STS_FREE_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_INTR_STS_FREE_TOKEN_NO_VALID_STS
 ******************************************************************************/
const ru_field_rec FPM_POOL1_INTR_STS_FREE_TOKEN_NO_VALID_STS_FIELD =
{
    "FREE_TOKEN_NO_VALID_STS",
#if RU_INCLUDE_DESC
    "",
    "De-allocation token request with invalid token Interrupt. \n"
    "Invalid free token is determined when one or more the following conditions are met - \n"
    "1. Incoming free request token has valid bit (bit[31]) set to 1'b0 \n"
    "2. Incoming free request token index is not aligned to the pool size indicated "
    "by the pool select field (bits[29:28]) \n"
    "3. Incoming free request token entry in the usage array indicates it is not an allocated "
    "token, i.e., associated use count value for this count in the usage array is zero \n"
    "Note: item 2 is not checked if auto_pool_en is set. The auto_pool_en bit is always "
    "set when using the new token format without a pool select field. \n",
#endif
    FPM_POOL1_INTR_STS_FREE_TOKEN_NO_VALID_STS_FIELD_MASK,
    0,
    FPM_POOL1_INTR_STS_FREE_TOKEN_NO_VALID_STS_FIELD_WIDTH,
    FPM_POOL1_INTR_STS_FREE_TOKEN_NO_VALID_STS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_INTR_STS_POOL_FULL_STS
 ******************************************************************************/
const ru_field_rec FPM_POOL1_INTR_STS_POOL_FULL_STS_FIELD =
{
    "POOL_FULL_STS",
#if RU_INCLUDE_DESC
    "",
    "Usage Index Pool is fully allocated interrupt. This is a functional status "
    "bit, not an error status bit. This indicates that token pool is fully allocated "
    "and there are no free tokens available. This bit will be active (high) as long as there "
    "no free tokens available to allocate. This bit is intended to be used for debug purpose only. \n",
#endif
    FPM_POOL1_INTR_STS_POOL_FULL_STS_FIELD_MASK,
    0,
    FPM_POOL1_INTR_STS_POOL_FULL_STS_FIELD_WIDTH,
    FPM_POOL1_INTR_STS_POOL_FULL_STS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_INTR_STS_FREE_FIFO_FULL_STS
 ******************************************************************************/
const ru_field_rec FPM_POOL1_INTR_STS_FREE_FIFO_FULL_STS_FIELD =
{
    "FREE_FIFO_FULL_STS",
#if RU_INCLUDE_DESC
    "",
    "De-Allocation FIFO Full Interrupt. This is a functional status bit, not an error status bit. "
    "This indicates that de-allocation FIFO is full with tokens needs to be freed and will be "
    "active (high) as long as FIFO is full. This status is intended to be used for "
    "debug purpose only. \n",
#endif
    FPM_POOL1_INTR_STS_FREE_FIFO_FULL_STS_FIELD_MASK,
    0,
    FPM_POOL1_INTR_STS_FREE_FIFO_FULL_STS_FIELD_WIDTH,
    FPM_POOL1_INTR_STS_FREE_FIFO_FULL_STS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_INTR_STS_ALLOC_FIFO_FULL_STS
 ******************************************************************************/
const ru_field_rec FPM_POOL1_INTR_STS_ALLOC_FIFO_FULL_STS_FIELD =
{
    "ALLOC_FIFO_FULL_STS",
#if RU_INCLUDE_DESC
    "",
    "Allocation FIFO Full Interrupt. This is a functional status bit, not an error status bit. "
    "This indicates that allocation FIFO is full with new tokens to be allocated and will be "
    "active (high) as long as FIFO is full. This status is intended to be used for "
    "debug purpose only. \n",
#endif
    FPM_POOL1_INTR_STS_ALLOC_FIFO_FULL_STS_FIELD_MASK,
    0,
    FPM_POOL1_INTR_STS_ALLOC_FIFO_FULL_STS_FIELD_WIDTH,
    FPM_POOL1_INTR_STS_ALLOC_FIFO_FULL_STS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_STALL_MSK_RESERVED0
 ******************************************************************************/
const ru_field_rec FPM_POOL1_STALL_MSK_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_POOL1_STALL_MSK_RESERVED0_FIELD_MASK,
    0,
    FPM_POOL1_STALL_MSK_RESERVED0_FIELD_WIDTH,
    FPM_POOL1_STALL_MSK_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_STALL_MSK_MEMORY_CORRUPT_STALL_MSK
 ******************************************************************************/
const ru_field_rec FPM_POOL1_STALL_MSK_MEMORY_CORRUPT_STALL_MSK_FIELD =
{
    "MEMORY_CORRUPT_STALL_MSK",
#if RU_INCLUDE_DESC
    "",
    "Stall FPM on Index Memory corrupt interrupt status. \n",
#endif
    FPM_POOL1_STALL_MSK_MEMORY_CORRUPT_STALL_MSK_FIELD_MASK,
    0,
    FPM_POOL1_STALL_MSK_MEMORY_CORRUPT_STALL_MSK_FIELD_WIDTH,
    FPM_POOL1_STALL_MSK_MEMORY_CORRUPT_STALL_MSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_STALL_MSK_RESERVED1
 ******************************************************************************/
const ru_field_rec FPM_POOL1_STALL_MSK_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_POOL1_STALL_MSK_RESERVED1_FIELD_MASK,
    0,
    FPM_POOL1_STALL_MSK_RESERVED1_FIELD_WIDTH,
    FPM_POOL1_STALL_MSK_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_STALL_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK
 ******************************************************************************/
const ru_field_rec FPM_POOL1_STALL_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD =
{
    "MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK",
#if RU_INCLUDE_DESC
    "",
    "Stall FPM on Token multi-cast value update request with index out-of-range interrupt status. \n",
#endif
    FPM_POOL1_STALL_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD_MASK,
    0,
    FPM_POOL1_STALL_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD_WIDTH,
    FPM_POOL1_STALL_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_STALL_MSK_MULTI_TOKEN_NO_VALID_STALL_MSK
 ******************************************************************************/
const ru_field_rec FPM_POOL1_STALL_MSK_MULTI_TOKEN_NO_VALID_STALL_MSK_FIELD =
{
    "MULTI_TOKEN_NO_VALID_STALL_MSK",
#if RU_INCLUDE_DESC
    "",
    "Stall FPM on Token multi-cast value update request with invalid token interrupt status. \n",
#endif
    FPM_POOL1_STALL_MSK_MULTI_TOKEN_NO_VALID_STALL_MSK_FIELD_MASK,
    0,
    FPM_POOL1_STALL_MSK_MULTI_TOKEN_NO_VALID_STALL_MSK_FIELD_WIDTH,
    FPM_POOL1_STALL_MSK_MULTI_TOKEN_NO_VALID_STALL_MSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_STALL_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK
 ******************************************************************************/
const ru_field_rec FPM_POOL1_STALL_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD =
{
    "FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK",
#if RU_INCLUDE_DESC
    "",
    "Stall FPM on De-allocation token request with index out-of-range interrupt status. \n",
#endif
    FPM_POOL1_STALL_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD_MASK,
    0,
    FPM_POOL1_STALL_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD_WIDTH,
    FPM_POOL1_STALL_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_STALL_MSK_FREE_TOKEN_NO_VALID_STALL_MSK
 ******************************************************************************/
const ru_field_rec FPM_POOL1_STALL_MSK_FREE_TOKEN_NO_VALID_STALL_MSK_FIELD =
{
    "FREE_TOKEN_NO_VALID_STALL_MSK",
#if RU_INCLUDE_DESC
    "",
    "Stall FPM on De-allocation token request with invalid token interrupt status. \n",
#endif
    FPM_POOL1_STALL_MSK_FREE_TOKEN_NO_VALID_STALL_MSK_FIELD_MASK,
    0,
    FPM_POOL1_STALL_MSK_FREE_TOKEN_NO_VALID_STALL_MSK_FIELD_WIDTH,
    FPM_POOL1_STALL_MSK_FREE_TOKEN_NO_VALID_STALL_MSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_STALL_MSK_RESERVED2
 ******************************************************************************/
const ru_field_rec FPM_POOL1_STALL_MSK_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_POOL1_STALL_MSK_RESERVED2_FIELD_MASK,
    0,
    FPM_POOL1_STALL_MSK_RESERVED2_FIELD_WIDTH,
    FPM_POOL1_STALL_MSK_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_INTR_MSK_RESERVED0
 ******************************************************************************/
const ru_field_rec FPM_POOL2_INTR_MSK_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_POOL2_INTR_MSK_RESERVED0_FIELD_MASK,
    0,
    FPM_POOL2_INTR_MSK_RESERVED0_FIELD_WIDTH,
    FPM_POOL2_INTR_MSK_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_INTR_MSK_EXPIRED_TOKEN_RECOV_MSK
 ******************************************************************************/
const ru_field_rec FPM_POOL2_INTR_MSK_EXPIRED_TOKEN_RECOV_MSK_FIELD =
{
    "EXPIRED_TOKEN_RECOV_MSK",
#if RU_INCLUDE_DESC
    "",
    "Expired token recovered interrupt mask. \n",
#endif
    FPM_POOL2_INTR_MSK_EXPIRED_TOKEN_RECOV_MSK_FIELD_MASK,
    0,
    FPM_POOL2_INTR_MSK_EXPIRED_TOKEN_RECOV_MSK_FIELD_WIDTH,
    FPM_POOL2_INTR_MSK_EXPIRED_TOKEN_RECOV_MSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_INTR_MSK_EXPIRED_TOKEN_DET_MSK
 ******************************************************************************/
const ru_field_rec FPM_POOL2_INTR_MSK_EXPIRED_TOKEN_DET_MSK_FIELD =
{
    "EXPIRED_TOKEN_DET_MSK",
#if RU_INCLUDE_DESC
    "",
    "Expired token detect interrupt mask. \n",
#endif
    FPM_POOL2_INTR_MSK_EXPIRED_TOKEN_DET_MSK_FIELD_MASK,
    0,
    FPM_POOL2_INTR_MSK_EXPIRED_TOKEN_DET_MSK_FIELD_WIDTH,
    FPM_POOL2_INTR_MSK_EXPIRED_TOKEN_DET_MSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_INTR_MSK_ILLEGAL_ALLOC_REQUEST_MSK
 ******************************************************************************/
const ru_field_rec FPM_POOL2_INTR_MSK_ILLEGAL_ALLOC_REQUEST_MSK_FIELD =
{
    "ILLEGAL_ALLOC_REQUEST_MSK",
#if RU_INCLUDE_DESC
    "",
    "Illegal token request interrupt mask. \n",
#endif
    FPM_POOL2_INTR_MSK_ILLEGAL_ALLOC_REQUEST_MSK_FIELD_MASK,
    0,
    FPM_POOL2_INTR_MSK_ILLEGAL_ALLOC_REQUEST_MSK_FIELD_WIDTH,
    FPM_POOL2_INTR_MSK_ILLEGAL_ALLOC_REQUEST_MSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_INTR_MSK_ILLEGAL_ADDRESS_ACCESS_MSK
 ******************************************************************************/
const ru_field_rec FPM_POOL2_INTR_MSK_ILLEGAL_ADDRESS_ACCESS_MSK_FIELD =
{
    "ILLEGAL_ADDRESS_ACCESS_MSK",
#if RU_INCLUDE_DESC
    "",
    "Illegal/un-implemented register/memory space access  interrupt mask. \n",
#endif
    FPM_POOL2_INTR_MSK_ILLEGAL_ADDRESS_ACCESS_MSK_FIELD_MASK,
    0,
    FPM_POOL2_INTR_MSK_ILLEGAL_ADDRESS_ACCESS_MSK_FIELD_WIDTH,
    FPM_POOL2_INTR_MSK_ILLEGAL_ADDRESS_ACCESS_MSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_INTR_MSK_XON_MSK
 ******************************************************************************/
const ru_field_rec FPM_POOL2_INTR_MSK_XON_MSK_FIELD =
{
    "XON_MSK",
#if RU_INCLUDE_DESC
    "",
    "XON_STATE interrupt mask. \n",
#endif
    FPM_POOL2_INTR_MSK_XON_MSK_FIELD_MASK,
    0,
    FPM_POOL2_INTR_MSK_XON_MSK_FIELD_WIDTH,
    FPM_POOL2_INTR_MSK_XON_MSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_INTR_MSK_XOFF_MSK
 ******************************************************************************/
const ru_field_rec FPM_POOL2_INTR_MSK_XOFF_MSK_FIELD =
{
    "XOFF_MSK",
#if RU_INCLUDE_DESC
    "",
    "XOFF_STATE interrupt mask. \n",
#endif
    FPM_POOL2_INTR_MSK_XOFF_MSK_FIELD_MASK,
    0,
    FPM_POOL2_INTR_MSK_XOFF_MSK_FIELD_WIDTH,
    FPM_POOL2_INTR_MSK_XOFF_MSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_INTR_MSK_MEMORY_CORRUPT_MSK
 ******************************************************************************/
const ru_field_rec FPM_POOL2_INTR_MSK_MEMORY_CORRUPT_MSK_FIELD =
{
    "MEMORY_CORRUPT_MSK",
#if RU_INCLUDE_DESC
    "",
    "Index Memory corrupt interrupt mask. \n",
#endif
    FPM_POOL2_INTR_MSK_MEMORY_CORRUPT_MSK_FIELD_MASK,
    0,
    FPM_POOL2_INTR_MSK_MEMORY_CORRUPT_MSK_FIELD_WIDTH,
    FPM_POOL2_INTR_MSK_MEMORY_CORRUPT_MSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_INTR_MSK_POOL_DIS_FREE_MULTI_MSK
 ******************************************************************************/
const ru_field_rec FPM_POOL2_INTR_MSK_POOL_DIS_FREE_MULTI_MSK_FIELD =
{
    "POOL_DIS_FREE_MULTI_MSK",
#if RU_INCLUDE_DESC
    "",
    "Free or Mcast update on disabled pool interrupt mask . \n",
#endif
    FPM_POOL2_INTR_MSK_POOL_DIS_FREE_MULTI_MSK_FIELD_MASK,
    0,
    FPM_POOL2_INTR_MSK_POOL_DIS_FREE_MULTI_MSK_FIELD_WIDTH,
    FPM_POOL2_INTR_MSK_POOL_DIS_FREE_MULTI_MSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_INTR_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK
 ******************************************************************************/
const ru_field_rec FPM_POOL2_INTR_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD =
{
    "MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK",
#if RU_INCLUDE_DESC
    "",
    "Token multi-cast value update request with index out-of-range. \n",
#endif
    FPM_POOL2_INTR_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD_MASK,
    0,
    FPM_POOL2_INTR_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD_WIDTH,
    FPM_POOL2_INTR_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_INTR_MSK_MULTI_TOKEN_NO_VALID_MSK
 ******************************************************************************/
const ru_field_rec FPM_POOL2_INTR_MSK_MULTI_TOKEN_NO_VALID_MSK_FIELD =
{
    "MULTI_TOKEN_NO_VALID_MSK",
#if RU_INCLUDE_DESC
    "",
    "Token multi-cast value update request with invalid token. \n",
#endif
    FPM_POOL2_INTR_MSK_MULTI_TOKEN_NO_VALID_MSK_FIELD_MASK,
    0,
    FPM_POOL2_INTR_MSK_MULTI_TOKEN_NO_VALID_MSK_FIELD_WIDTH,
    FPM_POOL2_INTR_MSK_MULTI_TOKEN_NO_VALID_MSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_INTR_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK
 ******************************************************************************/
const ru_field_rec FPM_POOL2_INTR_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD =
{
    "FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK",
#if RU_INCLUDE_DESC
    "",
    "De-allocation token request with index out-of-range. \n",
#endif
    FPM_POOL2_INTR_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD_MASK,
    0,
    FPM_POOL2_INTR_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD_WIDTH,
    FPM_POOL2_INTR_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_INTR_MSK_FREE_TOKEN_NO_VALID_MSK
 ******************************************************************************/
const ru_field_rec FPM_POOL2_INTR_MSK_FREE_TOKEN_NO_VALID_MSK_FIELD =
{
    "FREE_TOKEN_NO_VALID_MSK",
#if RU_INCLUDE_DESC
    "",
    "De-allocation token request with invalid token. \n",
#endif
    FPM_POOL2_INTR_MSK_FREE_TOKEN_NO_VALID_MSK_FIELD_MASK,
    0,
    FPM_POOL2_INTR_MSK_FREE_TOKEN_NO_VALID_MSK_FIELD_WIDTH,
    FPM_POOL2_INTR_MSK_FREE_TOKEN_NO_VALID_MSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_INTR_MSK_POOL_FULL_MSK
 ******************************************************************************/
const ru_field_rec FPM_POOL2_INTR_MSK_POOL_FULL_MSK_FIELD =
{
    "POOL_FULL_MSK",
#if RU_INCLUDE_DESC
    "",
    "Usage Index Pool is fully allocated interrupt mask. \n",
#endif
    FPM_POOL2_INTR_MSK_POOL_FULL_MSK_FIELD_MASK,
    0,
    FPM_POOL2_INTR_MSK_POOL_FULL_MSK_FIELD_WIDTH,
    FPM_POOL2_INTR_MSK_POOL_FULL_MSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_INTR_MSK_FREE_FIFO_FULL_MSK
 ******************************************************************************/
const ru_field_rec FPM_POOL2_INTR_MSK_FREE_FIFO_FULL_MSK_FIELD =
{
    "FREE_FIFO_FULL_MSK",
#if RU_INCLUDE_DESC
    "",
    "De-Allocation FIFO Full Interrupt mask. \n",
#endif
    FPM_POOL2_INTR_MSK_FREE_FIFO_FULL_MSK_FIELD_MASK,
    0,
    FPM_POOL2_INTR_MSK_FREE_FIFO_FULL_MSK_FIELD_WIDTH,
    FPM_POOL2_INTR_MSK_FREE_FIFO_FULL_MSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_INTR_MSK_ALLOC_FIFO_FULL_MSK
 ******************************************************************************/
const ru_field_rec FPM_POOL2_INTR_MSK_ALLOC_FIFO_FULL_MSK_FIELD =
{
    "ALLOC_FIFO_FULL_MSK",
#if RU_INCLUDE_DESC
    "",
    "Allocation FIFO Full Interrupt mask. \n",
#endif
    FPM_POOL2_INTR_MSK_ALLOC_FIFO_FULL_MSK_FIELD_MASK,
    0,
    FPM_POOL2_INTR_MSK_ALLOC_FIFO_FULL_MSK_FIELD_WIDTH,
    FPM_POOL2_INTR_MSK_ALLOC_FIFO_FULL_MSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_INTR_STS_RESERVED0
 ******************************************************************************/
const ru_field_rec FPM_POOL2_INTR_STS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_POOL2_INTR_STS_RESERVED0_FIELD_MASK,
    0,
    FPM_POOL2_INTR_STS_RESERVED0_FIELD_WIDTH,
    FPM_POOL2_INTR_STS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_INTR_STS_EXPIRED_TOKEN_RECOV_STS
 ******************************************************************************/
const ru_field_rec FPM_POOL2_INTR_STS_EXPIRED_TOKEN_RECOV_STS_FIELD =
{
    "EXPIRED_TOKEN_RECOV_STS",
#if RU_INCLUDE_DESC
    "",
    "Expired token recovered interrupt. This is set when an expired token has been recovered"
    "and returned to pool as an available token. \n",
#endif
    FPM_POOL2_INTR_STS_EXPIRED_TOKEN_RECOV_STS_FIELD_MASK,
    0,
    FPM_POOL2_INTR_STS_EXPIRED_TOKEN_RECOV_STS_FIELD_WIDTH,
    FPM_POOL2_INTR_STS_EXPIRED_TOKEN_RECOV_STS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_INTR_STS_EXPIRED_TOKEN_DET_STS
 ******************************************************************************/
const ru_field_rec FPM_POOL2_INTR_STS_EXPIRED_TOKEN_DET_STS_FIELD =
{
    "EXPIRED_TOKEN_DET_STS",
#if RU_INCLUDE_DESC
    "",
    "Expired token detect interrupt. This is set when the token recovery logic detects "
    "a token that has been held for the entire duration of the aging timer. \n",
#endif
    FPM_POOL2_INTR_STS_EXPIRED_TOKEN_DET_STS_FIELD_MASK,
    0,
    FPM_POOL2_INTR_STS_EXPIRED_TOKEN_DET_STS_FIELD_WIDTH,
    FPM_POOL2_INTR_STS_EXPIRED_TOKEN_DET_STS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_INTR_STS_ILLEGAL_ALLOC_REQUEST_STS
 ******************************************************************************/
const ru_field_rec FPM_POOL2_INTR_STS_ILLEGAL_ALLOC_REQUEST_STS_FIELD =
{
    "ILLEGAL_ALLOC_REQUEST_STS",
#if RU_INCLUDE_DESC
    "",
    "Illegal token request interrupt. This will be active when the pool is disabled, there is "
    "a request for a new token and the alloc fifo for the selected token size is empty. "
    "Along with interrupt being sent "
    "an error reply packet will be sent out with o_ubus_error_out asserted. \n",
#endif
    FPM_POOL2_INTR_STS_ILLEGAL_ALLOC_REQUEST_STS_FIELD_MASK,
    0,
    FPM_POOL2_INTR_STS_ILLEGAL_ALLOC_REQUEST_STS_FIELD_WIDTH,
    FPM_POOL2_INTR_STS_ILLEGAL_ALLOC_REQUEST_STS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_INTR_STS_ILLEGAL_ADDRESS_ACCESS_STS
 ******************************************************************************/
const ru_field_rec FPM_POOL2_INTR_STS_ILLEGAL_ADDRESS_ACCESS_STS_FIELD =
{
    "ILLEGAL_ADDRESS_ACCESS_STS",
#if RU_INCLUDE_DESC
    "",
    "Illegal/un-implemented register/memory space access interrupt. This will be active "
    "when there is an attempt to read from an unimplemented register or memory space. Along "
    "with interrupt being sent an error reply packet will be sent out with "
    "o_ubus_error_out asserted. \n",
#endif
    FPM_POOL2_INTR_STS_ILLEGAL_ADDRESS_ACCESS_STS_FIELD_MASK,
    0,
    FPM_POOL2_INTR_STS_ILLEGAL_ADDRESS_ACCESS_STS_FIELD_WIDTH,
    FPM_POOL2_INTR_STS_ILLEGAL_ADDRESS_ACCESS_STS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_INTR_STS_XON_STATE_STS
 ******************************************************************************/
const ru_field_rec FPM_POOL2_INTR_STS_XON_STATE_STS_FIELD =
{
    "XON_STATE_STS",
#if RU_INCLUDE_DESC
    "",
    "Number of available tokens is greater than or equal to XON_THRESHOLD value in XON/XOFF "
    "Threshold configuration register. This is a functional status bit, not an error status bit. "
    "Using this information FPM generates \"backpressure\" output signal that is used by other "
    "UBUS client logics to throttle its operation. For example, UNIMAC logic can use \"backpressure\" "
    "signal to transfer \"PAUSE\" Ethernet flow control packets to throttle incoming frames on "
    "Ethernet interface. \n",
#endif
    FPM_POOL2_INTR_STS_XON_STATE_STS_FIELD_MASK,
    0,
    FPM_POOL2_INTR_STS_XON_STATE_STS_FIELD_WIDTH,
    FPM_POOL2_INTR_STS_XON_STATE_STS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_INTR_STS_XOFF_STATE_STS
 ******************************************************************************/
const ru_field_rec FPM_POOL2_INTR_STS_XOFF_STATE_STS_FIELD =
{
    "XOFF_STATE_STS",
#if RU_INCLUDE_DESC
    "",
    "Number of available tokens is less than or equal to XOFF_THRESHOLD value in XON/XOFF "
    "Threshold configuration register. This is a functional status bit, not an error status bit. "
    "Using this information FPM generates \"backpressure\" output signal that is used by other "
    "UBUS client logics to throttle its operation. For example, UNIMAC logic can use \"backpressure\" "
    "signal to transfer \"PAUSE\" Ethernet flow control packets to throttle incoming frames on "
    "Ethernet interface. \n",
#endif
    FPM_POOL2_INTR_STS_XOFF_STATE_STS_FIELD_MASK,
    0,
    FPM_POOL2_INTR_STS_XOFF_STATE_STS_FIELD_WIDTH,
    FPM_POOL2_INTR_STS_XOFF_STATE_STS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_INTR_STS_MEMORY_CORRUPT_STS
 ******************************************************************************/
const ru_field_rec FPM_POOL2_INTR_STS_MEMORY_CORRUPT_STS_FIELD =
{
    "MEMORY_CORRUPT_STS",
#if RU_INCLUDE_DESC
    "",
    "Index Memory is corrupted. \n"
    "During updates of the usage array, token manager checks if the use count and search tree value "
    "in the array has a legal value. If the use count or search tree value is not correct before "
    "updating, logic generates an error and interrupt. As long as the interrupt is active no more "
    "valid tokens will be allocated because this is a catastrophic error. Following are the two "
    "error conditions that are checked - \n"
    "1. During search for a free token, a particular token use count value indicates it is "
    "allocated (use count is greater than 0), but corresponding upper level search tree value "
    "indicates the token is still available (with bit value of 1'b0, instead of 1'b1). This is "
    "an error. \n"
    "2. During search for a free token, a particular token use count value indicates that it is "
    "free (use count is 0), but corresponding upper level search tree value indicates the token "
    "is not available (with bit value of 1'b1, instead of 1'b0). This is an error. \n",
#endif
    FPM_POOL2_INTR_STS_MEMORY_CORRUPT_STS_FIELD_MASK,
    0,
    FPM_POOL2_INTR_STS_MEMORY_CORRUPT_STS_FIELD_WIDTH,
    FPM_POOL2_INTR_STS_MEMORY_CORRUPT_STS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_INTR_STS_POOL_DIS_FREE_MULTI_STS
 ******************************************************************************/
const ru_field_rec FPM_POOL2_INTR_STS_POOL_DIS_FREE_MULTI_STS_FIELD =
{
    "POOL_DIS_FREE_MULTI_STS",
#if RU_INCLUDE_DESC
    "",
    "Free or Mcast update on disabled pool interrupt. \n"
    "This bit goes active when a free or multi-cast request is received and FPM is not enabled, "
    "i.e., pool enable bit in FPM control register is not set to 1'b1. \n",
#endif
    FPM_POOL2_INTR_STS_POOL_DIS_FREE_MULTI_STS_FIELD_MASK,
    0,
    FPM_POOL2_INTR_STS_POOL_DIS_FREE_MULTI_STS_FIELD_WIDTH,
    FPM_POOL2_INTR_STS_POOL_DIS_FREE_MULTI_STS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_INTR_STS_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS
 ******************************************************************************/
const ru_field_rec FPM_POOL2_INTR_STS_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD =
{
    "MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS",
#if RU_INCLUDE_DESC
    "",
    "Token multi-cast value update request with index out-of-range Interrupt. \n"
    "This is set when the token index is not aligned to the pool size. This is determined by "
    "examining the pool select field (bits[29:28]) and the 3 lsbs of the token index (bits[14:12]). "
    "There is no associated count for this error. Note: this error is not checked if auto_pool_en is "
    "set. The auto_pool_en bit is always set when using the new token format without a pool select field. \n",
#endif
    FPM_POOL2_INTR_STS_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD_MASK,
    0,
    FPM_POOL2_INTR_STS_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD_WIDTH,
    FPM_POOL2_INTR_STS_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_INTR_STS_MULTI_TOKEN_NO_VALID_STS
 ******************************************************************************/
const ru_field_rec FPM_POOL2_INTR_STS_MULTI_TOKEN_NO_VALID_STS_FIELD =
{
    "MULTI_TOKEN_NO_VALID_STS",
#if RU_INCLUDE_DESC
    "",
    "Token multi-cast value update request with invalid token Interrupt. \n"
    "Invalid multi-cast token is determined when one or more the following conditions are met - \n"
    "1. Incoming multi-cast request token has valid bit (bit[31]) set to 1'b0 \n"
    "2. Incoming multi-cast request token index is not aligned to the pool size indicated "
    "by the pool select field (bits[29:28]) \n"
    "3. Incoming multi-cast request token has use count field (bit[6:0]) set to zero \n"
    "4. Incoming multi-cast request token entry in the usage array indicates it is not an allocated "
    "token, i.e., associated use count value for this count in the usage array is zero \n"
    "5. After updating the use count value, the new use count value exceeds 0x7E \n"
    "Note: item 2 is not checked if auto_pool_en is set. The auto_pool_en bit is always "
    "set when using the new token format without a pool select field. \n",
#endif
    FPM_POOL2_INTR_STS_MULTI_TOKEN_NO_VALID_STS_FIELD_MASK,
    0,
    FPM_POOL2_INTR_STS_MULTI_TOKEN_NO_VALID_STS_FIELD_WIDTH,
    FPM_POOL2_INTR_STS_MULTI_TOKEN_NO_VALID_STS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_INTR_STS_FREE_TOKEN_INDEX_OUT_OF_RANGE_STS
 ******************************************************************************/
const ru_field_rec FPM_POOL2_INTR_STS_FREE_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD =
{
    "FREE_TOKEN_INDEX_OUT_OF_RANGE_STS",
#if RU_INCLUDE_DESC
    "",
    "De-allocation token request with index out-of-range Interrupt. \n"
    "Free token index out of range is determined when one or more of the following conditions are met - \n"
    "1. Incoming free request token index is not aligned to the pool size indicated "
    "by the pool select field (bits[29:28]) \n"
    "2. The buffer size indicated by the size field (bits[11:0]) is greater than the size of the allocated "
    "token. \n"
    "There is no associated count for this error. "
    "Note: item 1 is not checked if auto_pool_en is set. The auto_pool_en bit is always "
    "set when using the new token format without a pool select field. \n",
#endif
    FPM_POOL2_INTR_STS_FREE_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD_MASK,
    0,
    FPM_POOL2_INTR_STS_FREE_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD_WIDTH,
    FPM_POOL2_INTR_STS_FREE_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_INTR_STS_FREE_TOKEN_NO_VALID_STS
 ******************************************************************************/
const ru_field_rec FPM_POOL2_INTR_STS_FREE_TOKEN_NO_VALID_STS_FIELD =
{
    "FREE_TOKEN_NO_VALID_STS",
#if RU_INCLUDE_DESC
    "",
    "De-allocation token request with invalid token Interrupt. \n"
    "Invalid free token is determined when one or more the following conditions are met - \n"
    "1. Incoming free request token has valid bit (bit[31]) set to 1'b0 \n"
    "2. Incoming free request token index is not aligned to the pool size indicated "
    "by the pool select field (bits[29:28]) \n"
    "3. Incoming free request token entry in the usage array indicates it is not an allocated "
    "token, i.e., associated use count value for this count in the usage array is zero \n"
    "Note: item 2 is not checked if auto_pool_en is set. The auto_pool_en bit is always "
    "set when using the new token format without a pool select field. \n",
#endif
    FPM_POOL2_INTR_STS_FREE_TOKEN_NO_VALID_STS_FIELD_MASK,
    0,
    FPM_POOL2_INTR_STS_FREE_TOKEN_NO_VALID_STS_FIELD_WIDTH,
    FPM_POOL2_INTR_STS_FREE_TOKEN_NO_VALID_STS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_INTR_STS_POOL_FULL_STS
 ******************************************************************************/
const ru_field_rec FPM_POOL2_INTR_STS_POOL_FULL_STS_FIELD =
{
    "POOL_FULL_STS",
#if RU_INCLUDE_DESC
    "",
    "Usage Index Pool is fully allocated interrupt. This is a functional status "
    "bit, not an error status bit. This indicates that token pool is fully allocated "
    "and there are no free tokens available. This bit will be active (high) as long as there "
    "no free tokens available to allocate. This bit is intended to be used for debug purpose only. \n",
#endif
    FPM_POOL2_INTR_STS_POOL_FULL_STS_FIELD_MASK,
    0,
    FPM_POOL2_INTR_STS_POOL_FULL_STS_FIELD_WIDTH,
    FPM_POOL2_INTR_STS_POOL_FULL_STS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_INTR_STS_FREE_FIFO_FULL_STS
 ******************************************************************************/
const ru_field_rec FPM_POOL2_INTR_STS_FREE_FIFO_FULL_STS_FIELD =
{
    "FREE_FIFO_FULL_STS",
#if RU_INCLUDE_DESC
    "",
    "De-Allocation FIFO Full Interrupt. This is a functional status bit, not an error status bit. "
    "This indicates that de-allocation FIFO is full with tokens needs to be freed and will be "
    "active (high) as long as FIFO is full. This status is intended to be used for "
    "debug purpose only. \n",
#endif
    FPM_POOL2_INTR_STS_FREE_FIFO_FULL_STS_FIELD_MASK,
    0,
    FPM_POOL2_INTR_STS_FREE_FIFO_FULL_STS_FIELD_WIDTH,
    FPM_POOL2_INTR_STS_FREE_FIFO_FULL_STS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_INTR_STS_ALLOC_FIFO_FULL_STS
 ******************************************************************************/
const ru_field_rec FPM_POOL2_INTR_STS_ALLOC_FIFO_FULL_STS_FIELD =
{
    "ALLOC_FIFO_FULL_STS",
#if RU_INCLUDE_DESC
    "",
    "Allocation FIFO Full Interrupt. This is a functional status bit, not an error status bit. "
    "This indicates that allocation FIFO is full with new tokens to be allocated and will be "
    "active (high) as long as FIFO is full. This status is intended to be used for "
    "debug purpose only. \n",
#endif
    FPM_POOL2_INTR_STS_ALLOC_FIFO_FULL_STS_FIELD_MASK,
    0,
    FPM_POOL2_INTR_STS_ALLOC_FIFO_FULL_STS_FIELD_WIDTH,
    FPM_POOL2_INTR_STS_ALLOC_FIFO_FULL_STS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_STALL_MSK_RESERVED0
 ******************************************************************************/
const ru_field_rec FPM_POOL2_STALL_MSK_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_POOL2_STALL_MSK_RESERVED0_FIELD_MASK,
    0,
    FPM_POOL2_STALL_MSK_RESERVED0_FIELD_WIDTH,
    FPM_POOL2_STALL_MSK_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_STALL_MSK_MEMORY_CORRUPT_STALL_MSK
 ******************************************************************************/
const ru_field_rec FPM_POOL2_STALL_MSK_MEMORY_CORRUPT_STALL_MSK_FIELD =
{
    "MEMORY_CORRUPT_STALL_MSK",
#if RU_INCLUDE_DESC
    "",
    "Stall FPM on Index Memory corrupt interrupt status. \n",
#endif
    FPM_POOL2_STALL_MSK_MEMORY_CORRUPT_STALL_MSK_FIELD_MASK,
    0,
    FPM_POOL2_STALL_MSK_MEMORY_CORRUPT_STALL_MSK_FIELD_WIDTH,
    FPM_POOL2_STALL_MSK_MEMORY_CORRUPT_STALL_MSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_STALL_MSK_RESERVED1
 ******************************************************************************/
const ru_field_rec FPM_POOL2_STALL_MSK_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_POOL2_STALL_MSK_RESERVED1_FIELD_MASK,
    0,
    FPM_POOL2_STALL_MSK_RESERVED1_FIELD_WIDTH,
    FPM_POOL2_STALL_MSK_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_STALL_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK
 ******************************************************************************/
const ru_field_rec FPM_POOL2_STALL_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD =
{
    "MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK",
#if RU_INCLUDE_DESC
    "",
    "Stall FPM on Token multi-cast value update request with index out-of-range interrupt status. \n",
#endif
    FPM_POOL2_STALL_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD_MASK,
    0,
    FPM_POOL2_STALL_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD_WIDTH,
    FPM_POOL2_STALL_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_STALL_MSK_MULTI_TOKEN_NO_VALID_STALL_MSK
 ******************************************************************************/
const ru_field_rec FPM_POOL2_STALL_MSK_MULTI_TOKEN_NO_VALID_STALL_MSK_FIELD =
{
    "MULTI_TOKEN_NO_VALID_STALL_MSK",
#if RU_INCLUDE_DESC
    "",
    "Stall FPM on Token multi-cast value update request with invalid token interrupt status. \n",
#endif
    FPM_POOL2_STALL_MSK_MULTI_TOKEN_NO_VALID_STALL_MSK_FIELD_MASK,
    0,
    FPM_POOL2_STALL_MSK_MULTI_TOKEN_NO_VALID_STALL_MSK_FIELD_WIDTH,
    FPM_POOL2_STALL_MSK_MULTI_TOKEN_NO_VALID_STALL_MSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_STALL_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK
 ******************************************************************************/
const ru_field_rec FPM_POOL2_STALL_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD =
{
    "FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK",
#if RU_INCLUDE_DESC
    "",
    "Stall FPM on De-allocation token request with index out-of-range interrupt status. \n",
#endif
    FPM_POOL2_STALL_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD_MASK,
    0,
    FPM_POOL2_STALL_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD_WIDTH,
    FPM_POOL2_STALL_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_STALL_MSK_FREE_TOKEN_NO_VALID_STALL_MSK
 ******************************************************************************/
const ru_field_rec FPM_POOL2_STALL_MSK_FREE_TOKEN_NO_VALID_STALL_MSK_FIELD =
{
    "FREE_TOKEN_NO_VALID_STALL_MSK",
#if RU_INCLUDE_DESC
    "",
    "Stall FPM on De-allocation token request with invalid token interrupt status. \n",
#endif
    FPM_POOL2_STALL_MSK_FREE_TOKEN_NO_VALID_STALL_MSK_FIELD_MASK,
    0,
    FPM_POOL2_STALL_MSK_FREE_TOKEN_NO_VALID_STALL_MSK_FIELD_WIDTH,
    FPM_POOL2_STALL_MSK_FREE_TOKEN_NO_VALID_STALL_MSK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_STALL_MSK_RESERVED2
 ******************************************************************************/
const ru_field_rec FPM_POOL2_STALL_MSK_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_POOL2_STALL_MSK_RESERVED2_FIELD_MASK,
    0,
    FPM_POOL2_STALL_MSK_RESERVED2_FIELD_WIDTH,
    FPM_POOL2_STALL_MSK_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_CFG1_RESERVED0
 ******************************************************************************/
const ru_field_rec FPM_POOL1_CFG1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_POOL1_CFG1_RESERVED0_FIELD_MASK,
    0,
    FPM_POOL1_CFG1_RESERVED0_FIELD_WIDTH,
    FPM_POOL1_CFG1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_CFG1_FP_BUF_SIZE
 ******************************************************************************/
const ru_field_rec FPM_POOL1_CFG1_FP_BUF_SIZE_FIELD =
{
    "FP_BUF_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Selects the size of the buffer to be used in the pool. "
    "All buffers must be the same size. \n"
    " 0 - 512 byte buffers \n"
    " 1 - 256 byte buffers \n"
    " all other values - reserved \n",
#endif
    FPM_POOL1_CFG1_FP_BUF_SIZE_FIELD_MASK,
    0,
    FPM_POOL1_CFG1_FP_BUF_SIZE_FIELD_WIDTH,
    FPM_POOL1_CFG1_FP_BUF_SIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_CFG1_RESERVED1
 ******************************************************************************/
const ru_field_rec FPM_POOL1_CFG1_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_POOL1_CFG1_RESERVED1_FIELD_MASK,
    0,
    FPM_POOL1_CFG1_RESERVED1_FIELD_WIDTH,
    FPM_POOL1_CFG1_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_CFG2_POOL_BASE_ADDRESS
 ******************************************************************************/
const ru_field_rec FPM_POOL1_CFG2_POOL_BASE_ADDRESS_FIELD =
{
    "POOL_BASE_ADDRESS",
#if RU_INCLUDE_DESC
    "",
    "Buffer base address. 7:2 must be 0x00.\n",
#endif
    FPM_POOL1_CFG2_POOL_BASE_ADDRESS_FIELD_MASK,
    0,
    FPM_POOL1_CFG2_POOL_BASE_ADDRESS_FIELD_WIDTH,
    FPM_POOL1_CFG2_POOL_BASE_ADDRESS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_CFG2_RESERVED0
 ******************************************************************************/
const ru_field_rec FPM_POOL1_CFG2_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_POOL1_CFG2_RESERVED0_FIELD_MASK,
    0,
    FPM_POOL1_CFG2_RESERVED0_FIELD_WIDTH,
    FPM_POOL1_CFG2_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_CFG3_POOL_BASE_ADDRESS_POOL2
 ******************************************************************************/
const ru_field_rec FPM_POOL1_CFG3_POOL_BASE_ADDRESS_POOL2_FIELD =
{
    "POOL_BASE_ADDRESS_POOL2",
#if RU_INCLUDE_DESC
    "",
    "Buffer base address. 7:2 must be 0x00.\n",
#endif
    FPM_POOL1_CFG3_POOL_BASE_ADDRESS_POOL2_FIELD_MASK,
    0,
    FPM_POOL1_CFG3_POOL_BASE_ADDRESS_POOL2_FIELD_WIDTH,
    FPM_POOL1_CFG3_POOL_BASE_ADDRESS_POOL2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_CFG3_RESERVED0
 ******************************************************************************/
const ru_field_rec FPM_POOL1_CFG3_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_POOL1_CFG3_RESERVED0_FIELD_MASK,
    0,
    FPM_POOL1_CFG3_RESERVED0_FIELD_WIDTH,
    FPM_POOL1_CFG3_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_STAT1_OVRFL
 ******************************************************************************/
const ru_field_rec FPM_POOL1_STAT1_OVRFL_FIELD =
{
    "OVRFL",
#if RU_INCLUDE_DESC
    "",
    "Free Pool overflow count",
#endif
    FPM_POOL1_STAT1_OVRFL_FIELD_MASK,
    0,
    FPM_POOL1_STAT1_OVRFL_FIELD_WIDTH,
    FPM_POOL1_STAT1_OVRFL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_STAT1_UNDRFL
 ******************************************************************************/
const ru_field_rec FPM_POOL1_STAT1_UNDRFL_FIELD =
{
    "UNDRFL",
#if RU_INCLUDE_DESC
    "",
    "Free Pool underflow count",
#endif
    FPM_POOL1_STAT1_UNDRFL_FIELD_MASK,
    0,
    FPM_POOL1_STAT1_UNDRFL_FIELD_WIDTH,
    FPM_POOL1_STAT1_UNDRFL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_STAT2_POOL_FULL
 ******************************************************************************/
const ru_field_rec FPM_POOL1_STAT2_POOL_FULL_FIELD =
{
    "POOL_FULL",
#if RU_INCLUDE_DESC
    "",
    "POOL is full\n"
    "This indicates that all tokens have been allocated and there no free tokens available. "
    "This bit will be active as long as all usage array is fully allocated. \n",
#endif
    FPM_POOL1_STAT2_POOL_FULL_FIELD_MASK,
    0,
    FPM_POOL1_STAT2_POOL_FULL_FIELD_WIDTH,
    FPM_POOL1_STAT2_POOL_FULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_STAT2_RESERVED0
 ******************************************************************************/
const ru_field_rec FPM_POOL1_STAT2_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_POOL1_STAT2_RESERVED0_FIELD_MASK,
    0,
    FPM_POOL1_STAT2_RESERVED0_FIELD_WIDTH,
    FPM_POOL1_STAT2_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_STAT2_FREE_FIFO_FULL
 ******************************************************************************/
const ru_field_rec FPM_POOL1_STAT2_FREE_FIFO_FULL_FIELD =
{
    "FREE_FIFO_FULL",
#if RU_INCLUDE_DESC
    "",
    "FREE_FIFO is full. \n",
#endif
    FPM_POOL1_STAT2_FREE_FIFO_FULL_FIELD_MASK,
    0,
    FPM_POOL1_STAT2_FREE_FIFO_FULL_FIELD_WIDTH,
    FPM_POOL1_STAT2_FREE_FIFO_FULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_STAT2_FREE_FIFO_EMPTY
 ******************************************************************************/
const ru_field_rec FPM_POOL1_STAT2_FREE_FIFO_EMPTY_FIELD =
{
    "FREE_FIFO_EMPTY",
#if RU_INCLUDE_DESC
    "",
    "FREE_FIFO is empty \n",
#endif
    FPM_POOL1_STAT2_FREE_FIFO_EMPTY_FIELD_MASK,
    0,
    FPM_POOL1_STAT2_FREE_FIFO_EMPTY_FIELD_WIDTH,
    FPM_POOL1_STAT2_FREE_FIFO_EMPTY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_STAT2_ALLOC_FIFO_FULL
 ******************************************************************************/
const ru_field_rec FPM_POOL1_STAT2_ALLOC_FIFO_FULL_FIELD =
{
    "ALLOC_FIFO_FULL",
#if RU_INCLUDE_DESC
    "",
    "ALLOC_FIFO is full \n",
#endif
    FPM_POOL1_STAT2_ALLOC_FIFO_FULL_FIELD_MASK,
    0,
    FPM_POOL1_STAT2_ALLOC_FIFO_FULL_FIELD_WIDTH,
    FPM_POOL1_STAT2_ALLOC_FIFO_FULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_STAT2_ALLOC_FIFO_EMPTY
 ******************************************************************************/
const ru_field_rec FPM_POOL1_STAT2_ALLOC_FIFO_EMPTY_FIELD =
{
    "ALLOC_FIFO_EMPTY",
#if RU_INCLUDE_DESC
    "",
    "ALLOC_FIFO is empty. \n",
#endif
    FPM_POOL1_STAT2_ALLOC_FIFO_EMPTY_FIELD_MASK,
    0,
    FPM_POOL1_STAT2_ALLOC_FIFO_EMPTY_FIELD_WIDTH,
    FPM_POOL1_STAT2_ALLOC_FIFO_EMPTY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_STAT2_RESERVED1
 ******************************************************************************/
const ru_field_rec FPM_POOL1_STAT2_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_POOL1_STAT2_RESERVED1_FIELD_MASK,
    0,
    FPM_POOL1_STAT2_RESERVED1_FIELD_WIDTH,
    FPM_POOL1_STAT2_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_STAT2_NUM_OF_TOKENS_AVAILABLE
 ******************************************************************************/
const ru_field_rec FPM_POOL1_STAT2_NUM_OF_TOKENS_AVAILABLE_FIELD =
{
    "NUM_OF_TOKENS_AVAILABLE",
#if RU_INCLUDE_DESC
    "",
    "Count of tokens available for allocation. \n"
    "This provides a count of number of free tokens that available for allocation in "
    "the usage array. This value is updated instantaneously as tokens are allocated or "
    "freed from the array. \n",
#endif
    FPM_POOL1_STAT2_NUM_OF_TOKENS_AVAILABLE_FIELD_MASK,
    0,
    FPM_POOL1_STAT2_NUM_OF_TOKENS_AVAILABLE_FIELD_WIDTH,
    FPM_POOL1_STAT2_NUM_OF_TOKENS_AVAILABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_STAT3_RESERVED0
 ******************************************************************************/
const ru_field_rec FPM_POOL1_STAT3_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_POOL1_STAT3_RESERVED0_FIELD_MASK,
    0,
    FPM_POOL1_STAT3_RESERVED0_FIELD_WIDTH,
    FPM_POOL1_STAT3_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_STAT3_NUM_OF_NOT_VALID_TOKEN_FREES
 ******************************************************************************/
const ru_field_rec FPM_POOL1_STAT3_NUM_OF_NOT_VALID_TOKEN_FREES_FIELD =
{
    "NUM_OF_NOT_VALID_TOKEN_FREES",
#if RU_INCLUDE_DESC
    "",
    "Count of de-allocate token requests with invalid tokens. "
    "For more information on conditions under which this counter is incremented, refer to "
    "POOL1_INTR_STS register (offset 0x14) bit[3] explanation in this document. \n",
#endif
    FPM_POOL1_STAT3_NUM_OF_NOT_VALID_TOKEN_FREES_FIELD_MASK,
    0,
    FPM_POOL1_STAT3_NUM_OF_NOT_VALID_TOKEN_FREES_FIELD_WIDTH,
    FPM_POOL1_STAT3_NUM_OF_NOT_VALID_TOKEN_FREES_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_STAT4_RESERVED0
 ******************************************************************************/
const ru_field_rec FPM_POOL1_STAT4_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_POOL1_STAT4_RESERVED0_FIELD_MASK,
    0,
    FPM_POOL1_STAT4_RESERVED0_FIELD_WIDTH,
    FPM_POOL1_STAT4_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_STAT4_NUM_OF_NOT_VALID_TOKEN_MULTI
 ******************************************************************************/
const ru_field_rec FPM_POOL1_STAT4_NUM_OF_NOT_VALID_TOKEN_MULTI_FIELD =
{
    "NUM_OF_NOT_VALID_TOKEN_MULTI",
#if RU_INCLUDE_DESC
    "",
    "Count of multi-cast token update requests with either valid bit not set, "
    "For more information on conditions under which this counter is incremented, refer to "
    "POOL1_INTR_STS register (offset 0x14) bit[5] explanation in this document. \n",
#endif
    FPM_POOL1_STAT4_NUM_OF_NOT_VALID_TOKEN_MULTI_FIELD_MASK,
    0,
    FPM_POOL1_STAT4_NUM_OF_NOT_VALID_TOKEN_MULTI_FIELD_WIDTH,
    FPM_POOL1_STAT4_NUM_OF_NOT_VALID_TOKEN_MULTI_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_VALID
 ******************************************************************************/
const ru_field_rec FPM_POOL1_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_VALID_FIELD =
{
    "MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_VALID",
#if RU_INCLUDE_DESC
    "",
    "This bit provides status of the token in bits[30:0] of this register \n"
    "0 = New token is not captured \n"
    "1 = New token is captured \n",
#endif
    FPM_POOL1_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_VALID_FIELD_MASK,
    0,
    FPM_POOL1_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_VALID_FIELD_WIDTH,
    FPM_POOL1_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN
 ******************************************************************************/
const ru_field_rec FPM_POOL1_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_FIELD =
{
    "MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN",
#if RU_INCLUDE_DESC
    "",
    "Token that causes memory corrupt interrupt active. If there are multiple tokens "
    "that causes this error, only the first one is captured. To capture successive tokens that "
    "causes the error this register should be cleared by writing any random value, in addition, "
    "memory corrupt status bit (bit[8]) in interrupt status register 0x14 should be cleared. "
    "Bitmap for these bits is shown below "
    "(reserved bits are zeros) \n"
    "Bit[30]    - Reserved \n"
    "Bit[29:12] - Token \n"
    "Bit[11:0]  - Buffer size in bytes \n",
#endif
    FPM_POOL1_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_FIELD_MASK,
    0,
    FPM_POOL1_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_FIELD_WIDTH,
    FPM_POOL1_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_STAT6_INVALID_FREE_TOKEN_VALID
 ******************************************************************************/
const ru_field_rec FPM_POOL1_STAT6_INVALID_FREE_TOKEN_VALID_FIELD =
{
    "INVALID_FREE_TOKEN_VALID",
#if RU_INCLUDE_DESC
    "",
    "This bit provides status of the token in bits[30:0] of this register \n"
    "0 = New token is not captured \n"
    "1 = New token is captured \n",
#endif
    FPM_POOL1_STAT6_INVALID_FREE_TOKEN_VALID_FIELD_MASK,
    0,
    FPM_POOL1_STAT6_INVALID_FREE_TOKEN_VALID_FIELD_WIDTH,
    FPM_POOL1_STAT6_INVALID_FREE_TOKEN_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_STAT6_INVALID_FREE_TOKEN
 ******************************************************************************/
const ru_field_rec FPM_POOL1_STAT6_INVALID_FREE_TOKEN_FIELD =
{
    "INVALID_FREE_TOKEN",
#if RU_INCLUDE_DESC
    "",
    "Token that causes intr[3] or intr[4] active. If there are multiple tokens "
    "that causes this error, only the first one is captured. To capture successive tokens that "
    "causes the error this register should be cleared by writing any random value. "
    "Bitmap for these bits is shown below "
    "(reserved bits are either zeros or can reflect the length of the packet associated with the freed token) \n"
    "Bit[30]    - Reserved \n"
    "Bit[29:12] - Token \n"
    "Bit[11:0]  - Reserved \n",
#endif
    FPM_POOL1_STAT6_INVALID_FREE_TOKEN_FIELD_MASK,
    0,
    FPM_POOL1_STAT6_INVALID_FREE_TOKEN_FIELD_WIDTH,
    FPM_POOL1_STAT6_INVALID_FREE_TOKEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_STAT7_INVALID_MCAST_TOKEN_VALID
 ******************************************************************************/
const ru_field_rec FPM_POOL1_STAT7_INVALID_MCAST_TOKEN_VALID_FIELD =
{
    "INVALID_MCAST_TOKEN_VALID",
#if RU_INCLUDE_DESC
    "",
    "This bit provides status of the token in bits[30:0] of this register \n"
    "0 = New token is not captured \n"
    "1 = New token is captured \n",
#endif
    FPM_POOL1_STAT7_INVALID_MCAST_TOKEN_VALID_FIELD_MASK,
    0,
    FPM_POOL1_STAT7_INVALID_MCAST_TOKEN_VALID_FIELD_WIDTH,
    FPM_POOL1_STAT7_INVALID_MCAST_TOKEN_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_STAT7_INVALID_MCAST_TOKEN
 ******************************************************************************/
const ru_field_rec FPM_POOL1_STAT7_INVALID_MCAST_TOKEN_FIELD =
{
    "INVALID_MCAST_TOKEN",
#if RU_INCLUDE_DESC
    "",
    "Token that causes intr[5] or intr[6] active. If there are multiple tokens "
    "that causes this error, only the first one is captured. To capture successive tokens that "
    "causes the error this register should be cleared by writing any random value. "
    "Bitmap for these bits is shown below "
    "(reserved bits are zeros) \n"
    "Bit[30]    - Reserved \n"
    "Bit[29:12] - Token \n"
    "Bit[11]    - Mcast update type (refer to register 0x224[11]) \n"
    "Bit[10:7]  - Reserved \n"
    "Bit[6:0]   - Mcast value \n",
#endif
    FPM_POOL1_STAT7_INVALID_MCAST_TOKEN_FIELD_MASK,
    0,
    FPM_POOL1_STAT7_INVALID_MCAST_TOKEN_FIELD_WIDTH,
    FPM_POOL1_STAT7_INVALID_MCAST_TOKEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_STAT8_RESERVED0
 ******************************************************************************/
const ru_field_rec FPM_POOL1_STAT8_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_POOL1_STAT8_RESERVED0_FIELD_MASK,
    0,
    FPM_POOL1_STAT8_RESERVED0_FIELD_WIDTH,
    FPM_POOL1_STAT8_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_STAT8_TOKENS_AVAILABLE_LOW_WTMK
 ******************************************************************************/
const ru_field_rec FPM_POOL1_STAT8_TOKENS_AVAILABLE_LOW_WTMK_FIELD =
{
    "TOKENS_AVAILABLE_LOW_WTMK",
#if RU_INCLUDE_DESC
    "",
    "Lowest value the NUM_OF_TOKENS_AVAIL count has reached. ",
#endif
    FPM_POOL1_STAT8_TOKENS_AVAILABLE_LOW_WTMK_FIELD_MASK,
    0,
    FPM_POOL1_STAT8_TOKENS_AVAILABLE_LOW_WTMK_FIELD_WIDTH,
    FPM_POOL1_STAT8_TOKENS_AVAILABLE_LOW_WTMK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_STAT1_OVRFL
 ******************************************************************************/
const ru_field_rec FPM_POOL2_STAT1_OVRFL_FIELD =
{
    "OVRFL",
#if RU_INCLUDE_DESC
    "",
    "Free Pool overflow count",
#endif
    FPM_POOL2_STAT1_OVRFL_FIELD_MASK,
    0,
    FPM_POOL2_STAT1_OVRFL_FIELD_WIDTH,
    FPM_POOL2_STAT1_OVRFL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_STAT1_UNDRFL
 ******************************************************************************/
const ru_field_rec FPM_POOL2_STAT1_UNDRFL_FIELD =
{
    "UNDRFL",
#if RU_INCLUDE_DESC
    "",
    "Free Pool underflow count",
#endif
    FPM_POOL2_STAT1_UNDRFL_FIELD_MASK,
    0,
    FPM_POOL2_STAT1_UNDRFL_FIELD_WIDTH,
    FPM_POOL2_STAT1_UNDRFL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_STAT2_POOL_FULL
 ******************************************************************************/
const ru_field_rec FPM_POOL2_STAT2_POOL_FULL_FIELD =
{
    "POOL_FULL",
#if RU_INCLUDE_DESC
    "",
    "POOL is full\n"
    "This indicates that all tokens have been allocated and there no free tokens available. "
    "This bit will be active as long as all usage array is fully allocated. \n",
#endif
    FPM_POOL2_STAT2_POOL_FULL_FIELD_MASK,
    0,
    FPM_POOL2_STAT2_POOL_FULL_FIELD_WIDTH,
    FPM_POOL2_STAT2_POOL_FULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_STAT2_RESERVED0
 ******************************************************************************/
const ru_field_rec FPM_POOL2_STAT2_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_POOL2_STAT2_RESERVED0_FIELD_MASK,
    0,
    FPM_POOL2_STAT2_RESERVED0_FIELD_WIDTH,
    FPM_POOL2_STAT2_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_STAT2_FREE_FIFO_FULL
 ******************************************************************************/
const ru_field_rec FPM_POOL2_STAT2_FREE_FIFO_FULL_FIELD =
{
    "FREE_FIFO_FULL",
#if RU_INCLUDE_DESC
    "",
    "FREE_FIFO is full. \n",
#endif
    FPM_POOL2_STAT2_FREE_FIFO_FULL_FIELD_MASK,
    0,
    FPM_POOL2_STAT2_FREE_FIFO_FULL_FIELD_WIDTH,
    FPM_POOL2_STAT2_FREE_FIFO_FULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_STAT2_FREE_FIFO_EMPTY
 ******************************************************************************/
const ru_field_rec FPM_POOL2_STAT2_FREE_FIFO_EMPTY_FIELD =
{
    "FREE_FIFO_EMPTY",
#if RU_INCLUDE_DESC
    "",
    "FREE_FIFO is empty \n",
#endif
    FPM_POOL2_STAT2_FREE_FIFO_EMPTY_FIELD_MASK,
    0,
    FPM_POOL2_STAT2_FREE_FIFO_EMPTY_FIELD_WIDTH,
    FPM_POOL2_STAT2_FREE_FIFO_EMPTY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_STAT2_ALLOC_FIFO_FULL
 ******************************************************************************/
const ru_field_rec FPM_POOL2_STAT2_ALLOC_FIFO_FULL_FIELD =
{
    "ALLOC_FIFO_FULL",
#if RU_INCLUDE_DESC
    "",
    "ALLOC_FIFO is full \n",
#endif
    FPM_POOL2_STAT2_ALLOC_FIFO_FULL_FIELD_MASK,
    0,
    FPM_POOL2_STAT2_ALLOC_FIFO_FULL_FIELD_WIDTH,
    FPM_POOL2_STAT2_ALLOC_FIFO_FULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_STAT2_ALLOC_FIFO_EMPTY
 ******************************************************************************/
const ru_field_rec FPM_POOL2_STAT2_ALLOC_FIFO_EMPTY_FIELD =
{
    "ALLOC_FIFO_EMPTY",
#if RU_INCLUDE_DESC
    "",
    "ALLOC_FIFO is empty. \n",
#endif
    FPM_POOL2_STAT2_ALLOC_FIFO_EMPTY_FIELD_MASK,
    0,
    FPM_POOL2_STAT2_ALLOC_FIFO_EMPTY_FIELD_WIDTH,
    FPM_POOL2_STAT2_ALLOC_FIFO_EMPTY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_STAT2_RESERVED1
 ******************************************************************************/
const ru_field_rec FPM_POOL2_STAT2_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_POOL2_STAT2_RESERVED1_FIELD_MASK,
    0,
    FPM_POOL2_STAT2_RESERVED1_FIELD_WIDTH,
    FPM_POOL2_STAT2_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_STAT2_NUM_OF_TOKENS_AVAILABLE
 ******************************************************************************/
const ru_field_rec FPM_POOL2_STAT2_NUM_OF_TOKENS_AVAILABLE_FIELD =
{
    "NUM_OF_TOKENS_AVAILABLE",
#if RU_INCLUDE_DESC
    "",
    "Count of tokens available for allocation. \n"
    "This provides a count of number of free tokens that available for allocation in "
    "the usage array. This value is updated instantaneously as tokens are allocated or "
    "freed from the array. \n",
#endif
    FPM_POOL2_STAT2_NUM_OF_TOKENS_AVAILABLE_FIELD_MASK,
    0,
    FPM_POOL2_STAT2_NUM_OF_TOKENS_AVAILABLE_FIELD_WIDTH,
    FPM_POOL2_STAT2_NUM_OF_TOKENS_AVAILABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_STAT3_RESERVED0
 ******************************************************************************/
const ru_field_rec FPM_POOL2_STAT3_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_POOL2_STAT3_RESERVED0_FIELD_MASK,
    0,
    FPM_POOL2_STAT3_RESERVED0_FIELD_WIDTH,
    FPM_POOL2_STAT3_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_STAT3_NUM_OF_NOT_VALID_TOKEN_FREES
 ******************************************************************************/
const ru_field_rec FPM_POOL2_STAT3_NUM_OF_NOT_VALID_TOKEN_FREES_FIELD =
{
    "NUM_OF_NOT_VALID_TOKEN_FREES",
#if RU_INCLUDE_DESC
    "",
    "Count of de-allocate token requests with invalid tokens. "
    "For more information on conditions under which this counter is incremented, refer to "
    "POOL1_INTR_STS register (offset 0x14) bit[3] explanation in this document. \n",
#endif
    FPM_POOL2_STAT3_NUM_OF_NOT_VALID_TOKEN_FREES_FIELD_MASK,
    0,
    FPM_POOL2_STAT3_NUM_OF_NOT_VALID_TOKEN_FREES_FIELD_WIDTH,
    FPM_POOL2_STAT3_NUM_OF_NOT_VALID_TOKEN_FREES_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_STAT4_RESERVED0
 ******************************************************************************/
const ru_field_rec FPM_POOL2_STAT4_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_POOL2_STAT4_RESERVED0_FIELD_MASK,
    0,
    FPM_POOL2_STAT4_RESERVED0_FIELD_WIDTH,
    FPM_POOL2_STAT4_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_STAT4_NUM_OF_NOT_VALID_TOKEN_MULTI
 ******************************************************************************/
const ru_field_rec FPM_POOL2_STAT4_NUM_OF_NOT_VALID_TOKEN_MULTI_FIELD =
{
    "NUM_OF_NOT_VALID_TOKEN_MULTI",
#if RU_INCLUDE_DESC
    "",
    "Count of multi-cast token update requests with either valid bit not set, "
    "For more information on conditions under which this counter is incremented, refer to "
    "POOL1_INTR_STS register (offset 0x14) bit[5] explanation in this document. \n",
#endif
    FPM_POOL2_STAT4_NUM_OF_NOT_VALID_TOKEN_MULTI_FIELD_MASK,
    0,
    FPM_POOL2_STAT4_NUM_OF_NOT_VALID_TOKEN_MULTI_FIELD_WIDTH,
    FPM_POOL2_STAT4_NUM_OF_NOT_VALID_TOKEN_MULTI_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_VALID
 ******************************************************************************/
const ru_field_rec FPM_POOL2_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_VALID_FIELD =
{
    "MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_VALID",
#if RU_INCLUDE_DESC
    "",
    "This bit provides status of the token in bits[30:0] of this register \n"
    "0 = New token is not captured \n"
    "1 = New token is captured \n",
#endif
    FPM_POOL2_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_VALID_FIELD_MASK,
    0,
    FPM_POOL2_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_VALID_FIELD_WIDTH,
    FPM_POOL2_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN
 ******************************************************************************/
const ru_field_rec FPM_POOL2_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_FIELD =
{
    "MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN",
#if RU_INCLUDE_DESC
    "",
    "Token that causes memory corrupt interrupt active. If there are multiple tokens "
    "that causes this error, only the first one is captured. To capture successive tokens that "
    "causes the error this register should be cleared by writing any random value, in addition, "
    "memory corrupt status bit (bit[8]) in interrupt status register 0x14 should be cleared. "
    "Bitmap for these bits is shown below "
    "(reserved bits are zeros) \n"
    "Bit[30]    - Reserved \n"
    "Bit[29:12] - Token \n"
    "Bit[11:0]  - Buffer size in bytes \n",
#endif
    FPM_POOL2_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_FIELD_MASK,
    0,
    FPM_POOL2_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_FIELD_WIDTH,
    FPM_POOL2_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_STAT6_INVALID_FREE_TOKEN_VALID
 ******************************************************************************/
const ru_field_rec FPM_POOL2_STAT6_INVALID_FREE_TOKEN_VALID_FIELD =
{
    "INVALID_FREE_TOKEN_VALID",
#if RU_INCLUDE_DESC
    "",
    "This bit provides status of the token in bits[30:0] of this register \n"
    "0 = New token is not captured \n"
    "1 = New token is captured \n",
#endif
    FPM_POOL2_STAT6_INVALID_FREE_TOKEN_VALID_FIELD_MASK,
    0,
    FPM_POOL2_STAT6_INVALID_FREE_TOKEN_VALID_FIELD_WIDTH,
    FPM_POOL2_STAT6_INVALID_FREE_TOKEN_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_STAT6_INVALID_FREE_TOKEN
 ******************************************************************************/
const ru_field_rec FPM_POOL2_STAT6_INVALID_FREE_TOKEN_FIELD =
{
    "INVALID_FREE_TOKEN",
#if RU_INCLUDE_DESC
    "",
    "Token that causes intr[3] or intr[4] active. If there are multiple tokens "
    "that causes this error, only the first one is captured. To capture successive tokens that "
    "causes the error this register should be cleared by writing any random value. "
    "Bitmap for these bits is shown below "
    "(reserved bits are either zeros or can reflect the length of the packet associated with the freed token) \n"
    "Bit[30]    - Reserved \n"
    "Bit[29:12] - Token \n"
    "Bit[11:0]  - Reserved \n",
#endif
    FPM_POOL2_STAT6_INVALID_FREE_TOKEN_FIELD_MASK,
    0,
    FPM_POOL2_STAT6_INVALID_FREE_TOKEN_FIELD_WIDTH,
    FPM_POOL2_STAT6_INVALID_FREE_TOKEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_STAT7_INVALID_MCAST_TOKEN_VALID
 ******************************************************************************/
const ru_field_rec FPM_POOL2_STAT7_INVALID_MCAST_TOKEN_VALID_FIELD =
{
    "INVALID_MCAST_TOKEN_VALID",
#if RU_INCLUDE_DESC
    "",
    "This bit provides status of the token in bits[30:0] of this register \n"
    "0 = New token is not captured \n"
    "1 = New token is captured \n",
#endif
    FPM_POOL2_STAT7_INVALID_MCAST_TOKEN_VALID_FIELD_MASK,
    0,
    FPM_POOL2_STAT7_INVALID_MCAST_TOKEN_VALID_FIELD_WIDTH,
    FPM_POOL2_STAT7_INVALID_MCAST_TOKEN_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_STAT7_INVALID_MCAST_TOKEN
 ******************************************************************************/
const ru_field_rec FPM_POOL2_STAT7_INVALID_MCAST_TOKEN_FIELD =
{
    "INVALID_MCAST_TOKEN",
#if RU_INCLUDE_DESC
    "",
    "Token that causes intr[5] or intr[6] active. If there are multiple tokens "
    "that causes this error, only the first one is captured. To capture successive tokens that "
    "causes the error this register should be cleared by writing any random value. "
    "Bitmap for these bits is shown below "
    "(reserved bits are zeros) \n"
    "Bit[30]    - Reserved \n"
    "Bit[29:12] - Token \n"
    "Bit[11]    - Mcast update type (refer to register 0x224[11]) \n"
    "Bit[10:7]  - Reserved \n"
    "Bit[6:0]   - Mcast value \n",
#endif
    FPM_POOL2_STAT7_INVALID_MCAST_TOKEN_FIELD_MASK,
    0,
    FPM_POOL2_STAT7_INVALID_MCAST_TOKEN_FIELD_WIDTH,
    FPM_POOL2_STAT7_INVALID_MCAST_TOKEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_STAT8_RESERVED0
 ******************************************************************************/
const ru_field_rec FPM_POOL2_STAT8_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_POOL2_STAT8_RESERVED0_FIELD_MASK,
    0,
    FPM_POOL2_STAT8_RESERVED0_FIELD_WIDTH,
    FPM_POOL2_STAT8_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_STAT8_TOKENS_AVAILABLE_LOW_WTMK
 ******************************************************************************/
const ru_field_rec FPM_POOL2_STAT8_TOKENS_AVAILABLE_LOW_WTMK_FIELD =
{
    "TOKENS_AVAILABLE_LOW_WTMK",
#if RU_INCLUDE_DESC
    "",
    "Lowest value the NUM_OF_TOKENS_AVAIL count has reached. ",
#endif
    FPM_POOL2_STAT8_TOKENS_AVAILABLE_LOW_WTMK_FIELD_MASK,
    0,
    FPM_POOL2_STAT8_TOKENS_AVAILABLE_LOW_WTMK_FIELD_WIDTH,
    FPM_POOL2_STAT8_TOKENS_AVAILABLE_LOW_WTMK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_XON_XOFF_CFG_XON_THRESHOLD
 ******************************************************************************/
const ru_field_rec FPM_POOL1_XON_XOFF_CFG_XON_THRESHOLD_FIELD =
{
    "XON_THRESHOLD",
#if RU_INCLUDE_DESC
    "",
    "XON Threshold value \n",
#endif
    FPM_POOL1_XON_XOFF_CFG_XON_THRESHOLD_FIELD_MASK,
    0,
    FPM_POOL1_XON_XOFF_CFG_XON_THRESHOLD_FIELD_WIDTH,
    FPM_POOL1_XON_XOFF_CFG_XON_THRESHOLD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_XON_XOFF_CFG_XOFF_THRESHOLD
 ******************************************************************************/
const ru_field_rec FPM_POOL1_XON_XOFF_CFG_XOFF_THRESHOLD_FIELD =
{
    "XOFF_THRESHOLD",
#if RU_INCLUDE_DESC
    "",
    "XOFF Threshold value \n",
#endif
    FPM_POOL1_XON_XOFF_CFG_XOFF_THRESHOLD_FIELD_MASK,
    0,
    FPM_POOL1_XON_XOFF_CFG_XOFF_THRESHOLD_FIELD_WIDTH,
    FPM_POOL1_XON_XOFF_CFG_XOFF_THRESHOLD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_FPM_NOT_EMPTY_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec FPM_FPM_NOT_EMPTY_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_FPM_NOT_EMPTY_CFG_RESERVED0_FIELD_MASK,
    0,
    FPM_FPM_NOT_EMPTY_CFG_RESERVED0_FIELD_WIDTH,
    FPM_FPM_NOT_EMPTY_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_FPM_NOT_EMPTY_CFG_NOT_EMPTY_THRESHOLD
 ******************************************************************************/
const ru_field_rec FPM_FPM_NOT_EMPTY_CFG_NOT_EMPTY_THRESHOLD_FIELD =
{
    "NOT_EMPTY_THRESHOLD",
#if RU_INCLUDE_DESC
    "",
    "Threshold value for reasserting pool_not_empty to FPM_BB\n",
#endif
    FPM_FPM_NOT_EMPTY_CFG_NOT_EMPTY_THRESHOLD_FIELD_MASK,
    0,
    FPM_FPM_NOT_EMPTY_CFG_NOT_EMPTY_THRESHOLD_FIELD_WIDTH,
    FPM_FPM_NOT_EMPTY_CFG_NOT_EMPTY_THRESHOLD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_MEM_CTL_MEM_WR
 ******************************************************************************/
const ru_field_rec FPM_MEM_CTL_MEM_WR_FIELD =
{
    "MEM_WR",
#if RU_INCLUDE_DESC
    "",
    "Write control bit for Usage index array memory. This is a self "
    "clearing bit, cleared by hardware to zero once memory write is  "
    "complete. Software can write more locations if the bit value is zero",
#endif
    FPM_MEM_CTL_MEM_WR_FIELD_MASK,
    0,
    FPM_MEM_CTL_MEM_WR_FIELD_WIDTH,
    FPM_MEM_CTL_MEM_WR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_MEM_CTL_MEM_RD
 ******************************************************************************/
const ru_field_rec FPM_MEM_CTL_MEM_RD_FIELD =
{
    "MEM_RD",
#if RU_INCLUDE_DESC
    "",
    "Read control bit for Usage index array memory. This is a self "
    "clearing bit, cleared by hardware to zero once memory read is  "
    "complete. Software can read more locations if the bit value is zero",
#endif
    FPM_MEM_CTL_MEM_RD_FIELD_MASK,
    0,
    FPM_MEM_CTL_MEM_RD_FIELD_WIDTH,
    FPM_MEM_CTL_MEM_RD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_MEM_CTL_MEM_SEL
 ******************************************************************************/
const ru_field_rec FPM_MEM_CTL_MEM_SEL_FIELD =
{
    "MEM_SEL",
#if RU_INCLUDE_DESC
    "",
    "2'b00 = Reserved\n"
    "2'b01 = FPM Memory\n"
    "2'b10 = Reserved\n"
    "2'b11 = When memory is enabled, bit[31]=1, this value will allow a write "
    "to NUM_OF_TOKENS_AVAILABLE field [17:0] in POOL1_STAT2 register (offset 0x54). "
    "This should be used for debug purposes only",
#endif
    FPM_MEM_CTL_MEM_SEL_FIELD_MASK,
    0,
    FPM_MEM_CTL_MEM_SEL_FIELD_WIDTH,
    FPM_MEM_CTL_MEM_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_MEM_CTL_RESERVED0
 ******************************************************************************/
const ru_field_rec FPM_MEM_CTL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_MEM_CTL_RESERVED0_FIELD_MASK,
    0,
    FPM_MEM_CTL_RESERVED0_FIELD_WIDTH,
    FPM_MEM_CTL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_MEM_CTL_MEM_ADDR
 ******************************************************************************/
const ru_field_rec FPM_MEM_CTL_MEM_ADDR_FIELD =
{
    "MEM_ADDR",
#if RU_INCLUDE_DESC
    "",
    "Memory address for write/read location\n"
    "This is DWord aligned address",
#endif
    FPM_MEM_CTL_MEM_ADDR_FIELD_MASK,
    0,
    FPM_MEM_CTL_MEM_ADDR_FIELD_WIDTH,
    FPM_MEM_CTL_MEM_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_MEM_CTL_RESERVED1
 ******************************************************************************/
const ru_field_rec FPM_MEM_CTL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_MEM_CTL_RESERVED1_FIELD_MASK,
    0,
    FPM_MEM_CTL_RESERVED1_FIELD_WIDTH,
    FPM_MEM_CTL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_MEM_DATA1_MEM_DATA1
 ******************************************************************************/
const ru_field_rec FPM_MEM_DATA1_MEM_DATA1_FIELD =
{
    "MEM_DATA1",
#if RU_INCLUDE_DESC
    "",
    "Memory Data 1\n"
    "This contains the lower 32 bits (bits[31:0]) of 32/64 bit data ",
#endif
    FPM_MEM_DATA1_MEM_DATA1_FIELD_MASK,
    0,
    FPM_MEM_DATA1_MEM_DATA1_FIELD_WIDTH,
    FPM_MEM_DATA1_MEM_DATA1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_MEM_DATA2_MEM_DATA2
 ******************************************************************************/
const ru_field_rec FPM_MEM_DATA2_MEM_DATA2_FIELD =
{
    "MEM_DATA2",
#if RU_INCLUDE_DESC
    "",
    "Memory Data 2\n"
    "This contains the upper 32 bits (bits[63:32]) of 64 bit data. "
    "The value in this register should be ignored during 32 bit access",
#endif
    FPM_MEM_DATA2_MEM_DATA2_FIELD_MASK,
    0,
    FPM_MEM_DATA2_MEM_DATA2_FIELD_WIDTH,
    FPM_MEM_DATA2_MEM_DATA2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_TOKEN_RECOVER_CTL_RESERVED0
 ******************************************************************************/
const ru_field_rec FPM_TOKEN_RECOVER_CTL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_TOKEN_RECOVER_CTL_RESERVED0_FIELD_MASK,
    0,
    FPM_TOKEN_RECOVER_CTL_RESERVED0_FIELD_WIDTH,
    FPM_TOKEN_RECOVER_CTL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_TOKEN_RECOVER_CTL_CLR_RECOVERED_TOKEN_COUNT
 ******************************************************************************/
const ru_field_rec FPM_TOKEN_RECOVER_CTL_CLR_RECOVERED_TOKEN_COUNT_FIELD =
{
    "CLR_RECOVERED_TOKEN_COUNT",
#if RU_INCLUDE_DESC
    "",
    "This is a self-clearing bit. Write a 1 to the bit to reset "
    " the RECOVERED_TOKEN_COUNT to 0.\n",
#endif
    FPM_TOKEN_RECOVER_CTL_CLR_RECOVERED_TOKEN_COUNT_FIELD_MASK,
    0,
    FPM_TOKEN_RECOVER_CTL_CLR_RECOVERED_TOKEN_COUNT_FIELD_WIDTH,
    FPM_TOKEN_RECOVER_CTL_CLR_RECOVERED_TOKEN_COUNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_TOKEN_RECOVER_CTL_CLR_EXPIRED_TOKEN_COUNT
 ******************************************************************************/
const ru_field_rec FPM_TOKEN_RECOVER_CTL_CLR_EXPIRED_TOKEN_COUNT_FIELD =
{
    "CLR_EXPIRED_TOKEN_COUNT",
#if RU_INCLUDE_DESC
    "",
    "This is a self-clearing bit. Write a 1 to the bit to reset "
    " the EXPIRED_TOKEN_COUNT to 0.\n",
#endif
    FPM_TOKEN_RECOVER_CTL_CLR_EXPIRED_TOKEN_COUNT_FIELD_MASK,
    0,
    FPM_TOKEN_RECOVER_CTL_CLR_EXPIRED_TOKEN_COUNT_FIELD_WIDTH,
    FPM_TOKEN_RECOVER_CTL_CLR_EXPIRED_TOKEN_COUNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_TOKEN_RECOVER_CTL_FORCE_TOKEN_RECLAIM
 ******************************************************************************/
const ru_field_rec FPM_TOKEN_RECOVER_CTL_FORCE_TOKEN_RECLAIM_FIELD =
{
    "FORCE_TOKEN_RECLAIM",
#if RU_INCLUDE_DESC
    "",
    "Non-automated token recovery.\n"
    "This bit can be used when automatic token return is not enabled. "
    "When software gets an interrupt indicating that the token recovery "
    "process has detected expired tokens, it can set this bit to force "
    "the expired tokens to be reclaimed.\n"
    "1 = Enabled\n "
    "0 = Disabled\n ",
#endif
    FPM_TOKEN_RECOVER_CTL_FORCE_TOKEN_RECLAIM_FIELD_MASK,
    0,
    FPM_TOKEN_RECOVER_CTL_FORCE_TOKEN_RECLAIM_FIELD_WIDTH,
    FPM_TOKEN_RECOVER_CTL_FORCE_TOKEN_RECLAIM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_TOKEN_RECOVER_CTL_TOKEN_RECLAIM_ENA
 ******************************************************************************/
const ru_field_rec FPM_TOKEN_RECOVER_CTL_TOKEN_RECLAIM_ENA_FIELD =
{
    "TOKEN_RECLAIM_ENA",
#if RU_INCLUDE_DESC
    "",
    "Enable automatic return of marked tokens to the freepool\n"
    "1 = Enabled\n "
    "0 = Disabled\n ",
#endif
    FPM_TOKEN_RECOVER_CTL_TOKEN_RECLAIM_ENA_FIELD_MASK,
    0,
    FPM_TOKEN_RECOVER_CTL_TOKEN_RECLAIM_ENA_FIELD_WIDTH,
    FPM_TOKEN_RECOVER_CTL_TOKEN_RECLAIM_ENA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_TOKEN_RECOVER_CTL_TOKEN_REMARK_ENA
 ******************************************************************************/
const ru_field_rec FPM_TOKEN_RECOVER_CTL_TOKEN_REMARK_ENA_FIELD =
{
    "TOKEN_REMARK_ENA",
#if RU_INCLUDE_DESC
    "",
    "Enable remarking of tokens for multiple passes through "
    "the token recovery process. The mark bit is set on all "
    "tokens on the first pass through the loop. When this "
    "bit is set, the mark bits will be set again on all subsequent "
    "passes through the loop. It is anticipated that this bit will "
    "always be set when token recovery is enabled. It is provided "
    "as a potential debug tool.\n"
    "1 = Enabled\n "
    "0 = Disabled\n ",
#endif
    FPM_TOKEN_RECOVER_CTL_TOKEN_REMARK_ENA_FIELD_MASK,
    0,
    FPM_TOKEN_RECOVER_CTL_TOKEN_REMARK_ENA_FIELD_WIDTH,
    FPM_TOKEN_RECOVER_CTL_TOKEN_REMARK_ENA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_TOKEN_RECOVER_CTL_SINGLE_PASS_ENA
 ******************************************************************************/
const ru_field_rec FPM_TOKEN_RECOVER_CTL_SINGLE_PASS_ENA_FIELD =
{
    "SINGLE_PASS_ENA",
#if RU_INCLUDE_DESC
    "",
    "If token recovery is enabled, the single-pass control will "
    "indicate whether the hardware should perform just one iteration "
    "of the token recovery process or will continuously loop "
    "through the token recovery process.\n"
    "1 = Single pass\n "
    "0 = Auto repeat\n ",
#endif
    FPM_TOKEN_RECOVER_CTL_SINGLE_PASS_ENA_FIELD_MASK,
    0,
    FPM_TOKEN_RECOVER_CTL_SINGLE_PASS_ENA_FIELD_WIDTH,
    FPM_TOKEN_RECOVER_CTL_SINGLE_PASS_ENA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_TOKEN_RECOVER_CTL_TOKEN_RECOVER_ENA
 ******************************************************************************/
const ru_field_rec FPM_TOKEN_RECOVER_CTL_TOKEN_RECOVER_ENA_FIELD =
{
    "TOKEN_RECOVER_ENA",
#if RU_INCLUDE_DESC
    "",
    "Token recovery enable\n"
    "1 = Enabled\n "
    "0 = Disabled\n ",
#endif
    FPM_TOKEN_RECOVER_CTL_TOKEN_RECOVER_ENA_FIELD_MASK,
    0,
    FPM_TOKEN_RECOVER_CTL_TOKEN_RECOVER_ENA_FIELD_WIDTH,
    FPM_TOKEN_RECOVER_CTL_TOKEN_RECOVER_ENA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_SHORT_AGING_TIMER_TIMER
 ******************************************************************************/
const ru_field_rec FPM_SHORT_AGING_TIMER_TIMER_FIELD =
{
    "TIMER",
#if RU_INCLUDE_DESC
    "",
    "Aging timer used in token recovery\n",
#endif
    FPM_SHORT_AGING_TIMER_TIMER_FIELD_MASK,
    0,
    FPM_SHORT_AGING_TIMER_TIMER_FIELD_WIDTH,
    FPM_SHORT_AGING_TIMER_TIMER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_LONG_AGING_TIMER_TIMER
 ******************************************************************************/
const ru_field_rec FPM_LONG_AGING_TIMER_TIMER_FIELD =
{
    "TIMER",
#if RU_INCLUDE_DESC
    "",
    "Aging timer used in token recovery\n",
#endif
    FPM_LONG_AGING_TIMER_TIMER_FIELD_MASK,
    0,
    FPM_LONG_AGING_TIMER_TIMER_FIELD_WIDTH,
    FPM_LONG_AGING_TIMER_TIMER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_CACHE_RECYCLE_TIMER_RESERVED0
 ******************************************************************************/
const ru_field_rec FPM_CACHE_RECYCLE_TIMER_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_CACHE_RECYCLE_TIMER_RESERVED0_FIELD_MASK,
    0,
    FPM_CACHE_RECYCLE_TIMER_RESERVED0_FIELD_WIDTH,
    FPM_CACHE_RECYCLE_TIMER_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_CACHE_RECYCLE_TIMER_RECYCLE_TIMER
 ******************************************************************************/
const ru_field_rec FPM_CACHE_RECYCLE_TIMER_RECYCLE_TIMER_FIELD =
{
    "RECYCLE_TIMER",
#if RU_INCLUDE_DESC
    "",
    "Timer used in token recovery logic. Upon expiration of timer, one token from the allocate "
    "cache will be freed. Over time, all cached tokens will be recycled back to the freepool. "
    "This will prevent the cached tokens frm being aged out by the token recovery logic. This "
    "timer should be set to a value so that all tokens can be recycled before the aging timer "
    "expires.\n",
#endif
    FPM_CACHE_RECYCLE_TIMER_RECYCLE_TIMER_FIELD_MASK,
    0,
    FPM_CACHE_RECYCLE_TIMER_RECYCLE_TIMER_FIELD_WIDTH,
    FPM_CACHE_RECYCLE_TIMER_RECYCLE_TIMER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_EXPIRED_TOKEN_COUNT_POOL1_COUNT
 ******************************************************************************/
const ru_field_rec FPM_EXPIRED_TOKEN_COUNT_POOL1_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Cumulative count of the number of expired tokens detected in the "
    "token recovery process. The count can be cleared by setting the "
    "CLR_EXPIRED_TOKEN_COUNT in the TOKEN_RECOVER_CTL register\n",
#endif
    FPM_EXPIRED_TOKEN_COUNT_POOL1_COUNT_FIELD_MASK,
    0,
    FPM_EXPIRED_TOKEN_COUNT_POOL1_COUNT_FIELD_WIDTH,
    FPM_EXPIRED_TOKEN_COUNT_POOL1_COUNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_RECOVERED_TOKEN_COUNT_POOL1_COUNT
 ******************************************************************************/
const ru_field_rec FPM_RECOVERED_TOKEN_COUNT_POOL1_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Cumulative count of the number of expired tokens that were freed in the "
    "token recovery process. The count can be cleared by setting the "
    "CLR_RECOVERED_TOKEN_COUNT in the TOKEN_RECOVER_CTL register\n",
#endif
    FPM_RECOVERED_TOKEN_COUNT_POOL1_COUNT_FIELD_MASK,
    0,
    FPM_RECOVERED_TOKEN_COUNT_POOL1_COUNT_FIELD_WIDTH,
    FPM_RECOVERED_TOKEN_COUNT_POOL1_COUNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_EXPIRED_TOKEN_COUNT_POOL2_COUNT
 ******************************************************************************/
const ru_field_rec FPM_EXPIRED_TOKEN_COUNT_POOL2_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Cumulative count of the number of expired tokens detected in the "
    "token recovery process. The count can be cleared by setting the "
    "CLR_EXPIRED_TOKEN_COUNT in the TOKEN_RECOVER_CTL register\n",
#endif
    FPM_EXPIRED_TOKEN_COUNT_POOL2_COUNT_FIELD_MASK,
    0,
    FPM_EXPIRED_TOKEN_COUNT_POOL2_COUNT_FIELD_WIDTH,
    FPM_EXPIRED_TOKEN_COUNT_POOL2_COUNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_RECOVERED_TOKEN_COUNT_POOL2_COUNT
 ******************************************************************************/
const ru_field_rec FPM_RECOVERED_TOKEN_COUNT_POOL2_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Cumulative count of the number of expired tokens that were freed in the "
    "token recovery process. The count can be cleared by setting the "
    "CLR_RECOVERED_TOKEN_COUNT in the TOKEN_RECOVER_CTL register\n",
#endif
    FPM_RECOVERED_TOKEN_COUNT_POOL2_COUNT_FIELD_MASK,
    0,
    FPM_RECOVERED_TOKEN_COUNT_POOL2_COUNT_FIELD_WIDTH,
    FPM_RECOVERED_TOKEN_COUNT_POOL2_COUNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_TOKEN_RECOVER_START_END_POOL1_RESERVED0
 ******************************************************************************/
const ru_field_rec FPM_TOKEN_RECOVER_START_END_POOL1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_TOKEN_RECOVER_START_END_POOL1_RESERVED0_FIELD_MASK,
    0,
    FPM_TOKEN_RECOVER_START_END_POOL1_RESERVED0_FIELD_WIDTH,
    FPM_TOKEN_RECOVER_START_END_POOL1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_TOKEN_RECOVER_START_END_POOL1_START_INDEX
 ******************************************************************************/
const ru_field_rec FPM_TOKEN_RECOVER_START_END_POOL1_START_INDEX_FIELD =
{
    "START_INDEX",
#if RU_INCLUDE_DESC
    "",
    "Start of token index range to be used when performing token recovery.\n",
#endif
    FPM_TOKEN_RECOVER_START_END_POOL1_START_INDEX_FIELD_MASK,
    0,
    FPM_TOKEN_RECOVER_START_END_POOL1_START_INDEX_FIELD_WIDTH,
    FPM_TOKEN_RECOVER_START_END_POOL1_START_INDEX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_TOKEN_RECOVER_START_END_POOL1_RESERVED1
 ******************************************************************************/
const ru_field_rec FPM_TOKEN_RECOVER_START_END_POOL1_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_TOKEN_RECOVER_START_END_POOL1_RESERVED1_FIELD_MASK,
    0,
    FPM_TOKEN_RECOVER_START_END_POOL1_RESERVED1_FIELD_WIDTH,
    FPM_TOKEN_RECOVER_START_END_POOL1_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_TOKEN_RECOVER_START_END_POOL1_END_INDEX
 ******************************************************************************/
const ru_field_rec FPM_TOKEN_RECOVER_START_END_POOL1_END_INDEX_FIELD =
{
    "END_INDEX",
#if RU_INCLUDE_DESC
    "",
    "End of token index range to be used when performing token recovery.\n",
#endif
    FPM_TOKEN_RECOVER_START_END_POOL1_END_INDEX_FIELD_MASK,
    0,
    FPM_TOKEN_RECOVER_START_END_POOL1_END_INDEX_FIELD_WIDTH,
    FPM_TOKEN_RECOVER_START_END_POOL1_END_INDEX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_TOKEN_RECOVER_START_END_POOL2_RESERVED0
 ******************************************************************************/
const ru_field_rec FPM_TOKEN_RECOVER_START_END_POOL2_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_TOKEN_RECOVER_START_END_POOL2_RESERVED0_FIELD_MASK,
    0,
    FPM_TOKEN_RECOVER_START_END_POOL2_RESERVED0_FIELD_WIDTH,
    FPM_TOKEN_RECOVER_START_END_POOL2_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_TOKEN_RECOVER_START_END_POOL2_START_INDEX
 ******************************************************************************/
const ru_field_rec FPM_TOKEN_RECOVER_START_END_POOL2_START_INDEX_FIELD =
{
    "START_INDEX",
#if RU_INCLUDE_DESC
    "",
    "Start of token index range to be used when performing token recovery.\n",
#endif
    FPM_TOKEN_RECOVER_START_END_POOL2_START_INDEX_FIELD_MASK,
    0,
    FPM_TOKEN_RECOVER_START_END_POOL2_START_INDEX_FIELD_WIDTH,
    FPM_TOKEN_RECOVER_START_END_POOL2_START_INDEX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_TOKEN_RECOVER_START_END_POOL2_RESERVED1
 ******************************************************************************/
const ru_field_rec FPM_TOKEN_RECOVER_START_END_POOL2_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_TOKEN_RECOVER_START_END_POOL2_RESERVED1_FIELD_MASK,
    0,
    FPM_TOKEN_RECOVER_START_END_POOL2_RESERVED1_FIELD_WIDTH,
    FPM_TOKEN_RECOVER_START_END_POOL2_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_TOKEN_RECOVER_START_END_POOL2_END_INDEX
 ******************************************************************************/
const ru_field_rec FPM_TOKEN_RECOVER_START_END_POOL2_END_INDEX_FIELD =
{
    "END_INDEX",
#if RU_INCLUDE_DESC
    "",
    "End of token index range to be used when performing token recovery.\n",
#endif
    FPM_TOKEN_RECOVER_START_END_POOL2_END_INDEX_FIELD_MASK,
    0,
    FPM_TOKEN_RECOVER_START_END_POOL2_END_INDEX_FIELD_WIDTH,
    FPM_TOKEN_RECOVER_START_END_POOL2_END_INDEX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_ALLOC_DEALLOC_TOKEN_VALID
 ******************************************************************************/
const ru_field_rec FPM_POOL1_ALLOC_DEALLOC_TOKEN_VALID_FIELD =
{
    "TOKEN_VALID",
#if RU_INCLUDE_DESC
    "",
    "Valid Token Indicator\n"
    "0: No buffers available\n"
    "1: A valid token index is provided. If a token is de-allocated/freed without this bit set "
    "that causes an error and the token will be ignored, error counter in register offset 0xB8 will be "
    "incremented. ",
#endif
    FPM_POOL1_ALLOC_DEALLOC_TOKEN_VALID_FIELD_MASK,
    0,
    FPM_POOL1_ALLOC_DEALLOC_TOKEN_VALID_FIELD_WIDTH,
    FPM_POOL1_ALLOC_DEALLOC_TOKEN_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_ALLOC_DEALLOC_RESERVED0
 ******************************************************************************/
const ru_field_rec FPM_POOL1_ALLOC_DEALLOC_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_POOL1_ALLOC_DEALLOC_RESERVED0_FIELD_MASK,
    0,
    FPM_POOL1_ALLOC_DEALLOC_RESERVED0_FIELD_WIDTH,
    FPM_POOL1_ALLOC_DEALLOC_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_ALLOC_DEALLOC_DDR
 ******************************************************************************/
const ru_field_rec FPM_POOL1_ALLOC_DEALLOC_DDR_FIELD =
{
    "DDR",
#if RU_INCLUDE_DESC
    "",
    "DDR Identifier\n"
    "0: DDR0\n"
    "1: DDR1\n",
#endif
    FPM_POOL1_ALLOC_DEALLOC_DDR_FIELD_MASK,
    0,
    FPM_POOL1_ALLOC_DEALLOC_DDR_FIELD_WIDTH,
    FPM_POOL1_ALLOC_DEALLOC_DDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_ALLOC_DEALLOC_TOKEN_INDEX
 ******************************************************************************/
const ru_field_rec FPM_POOL1_ALLOC_DEALLOC_TOKEN_INDEX_FIELD =
{
    "TOKEN_INDEX",
#if RU_INCLUDE_DESC
    "",
    "Buffer Index Pointer\n",
#endif
    FPM_POOL1_ALLOC_DEALLOC_TOKEN_INDEX_FIELD_MASK,
    0,
    FPM_POOL1_ALLOC_DEALLOC_TOKEN_INDEX_FIELD_WIDTH,
    FPM_POOL1_ALLOC_DEALLOC_TOKEN_INDEX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL1_ALLOC_DEALLOC_TOKEN_SIZE
 ******************************************************************************/
const ru_field_rec FPM_POOL1_ALLOC_DEALLOC_TOKEN_SIZE_FIELD =
{
    "TOKEN_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Buffer length or packet size in bytes",
#endif
    FPM_POOL1_ALLOC_DEALLOC_TOKEN_SIZE_FIELD_MASK,
    0,
    FPM_POOL1_ALLOC_DEALLOC_TOKEN_SIZE_FIELD_WIDTH,
    FPM_POOL1_ALLOC_DEALLOC_TOKEN_SIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_ALLOC_DEALLOC_TOKEN_VALID
 ******************************************************************************/
const ru_field_rec FPM_POOL2_ALLOC_DEALLOC_TOKEN_VALID_FIELD =
{
    "TOKEN_VALID",
#if RU_INCLUDE_DESC
    "",
    "Valid Token Indicator\n"
    "0: No buffers available\n"
    "1: A valid token index is provided. If a token is de-allocated/freed without this bit set "
    "that causes an error and the token will be ignored, error counter in register offset 0xB8 will be "
    "incremented. ",
#endif
    FPM_POOL2_ALLOC_DEALLOC_TOKEN_VALID_FIELD_MASK,
    0,
    FPM_POOL2_ALLOC_DEALLOC_TOKEN_VALID_FIELD_WIDTH,
    FPM_POOL2_ALLOC_DEALLOC_TOKEN_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_ALLOC_DEALLOC_RESERVED0
 ******************************************************************************/
const ru_field_rec FPM_POOL2_ALLOC_DEALLOC_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_POOL2_ALLOC_DEALLOC_RESERVED0_FIELD_MASK,
    0,
    FPM_POOL2_ALLOC_DEALLOC_RESERVED0_FIELD_WIDTH,
    FPM_POOL2_ALLOC_DEALLOC_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_ALLOC_DEALLOC_DDR
 ******************************************************************************/
const ru_field_rec FPM_POOL2_ALLOC_DEALLOC_DDR_FIELD =
{
    "DDR",
#if RU_INCLUDE_DESC
    "",
    "DDR Identifier\n"
    "0: DDR0\n"
    "1: DDR1\n",
#endif
    FPM_POOL2_ALLOC_DEALLOC_DDR_FIELD_MASK,
    0,
    FPM_POOL2_ALLOC_DEALLOC_DDR_FIELD_WIDTH,
    FPM_POOL2_ALLOC_DEALLOC_DDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_ALLOC_DEALLOC_TOKEN_INDEX
 ******************************************************************************/
const ru_field_rec FPM_POOL2_ALLOC_DEALLOC_TOKEN_INDEX_FIELD =
{
    "TOKEN_INDEX",
#if RU_INCLUDE_DESC
    "",
    "Buffer Index Pointer\n",
#endif
    FPM_POOL2_ALLOC_DEALLOC_TOKEN_INDEX_FIELD_MASK,
    0,
    FPM_POOL2_ALLOC_DEALLOC_TOKEN_INDEX_FIELD_WIDTH,
    FPM_POOL2_ALLOC_DEALLOC_TOKEN_INDEX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL2_ALLOC_DEALLOC_TOKEN_SIZE
 ******************************************************************************/
const ru_field_rec FPM_POOL2_ALLOC_DEALLOC_TOKEN_SIZE_FIELD =
{
    "TOKEN_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Buffer length or packet size in bytes",
#endif
    FPM_POOL2_ALLOC_DEALLOC_TOKEN_SIZE_FIELD_MASK,
    0,
    FPM_POOL2_ALLOC_DEALLOC_TOKEN_SIZE_FIELD_WIDTH,
    FPM_POOL2_ALLOC_DEALLOC_TOKEN_SIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL3_ALLOC_DEALLOC_TOKEN_VALID
 ******************************************************************************/
const ru_field_rec FPM_POOL3_ALLOC_DEALLOC_TOKEN_VALID_FIELD =
{
    "TOKEN_VALID",
#if RU_INCLUDE_DESC
    "",
    "Valid Token Indicator\n"
    "0: No buffers available\n"
    "1: A valid token index is provided. If a token is de-allocated/freed without this bit set "
    "that causes an error and the token will be ignored, error counter in register offset 0xB8 will be "
    "incremented. ",
#endif
    FPM_POOL3_ALLOC_DEALLOC_TOKEN_VALID_FIELD_MASK,
    0,
    FPM_POOL3_ALLOC_DEALLOC_TOKEN_VALID_FIELD_WIDTH,
    FPM_POOL3_ALLOC_DEALLOC_TOKEN_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL3_ALLOC_DEALLOC_RESERVED0
 ******************************************************************************/
const ru_field_rec FPM_POOL3_ALLOC_DEALLOC_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_POOL3_ALLOC_DEALLOC_RESERVED0_FIELD_MASK,
    0,
    FPM_POOL3_ALLOC_DEALLOC_RESERVED0_FIELD_WIDTH,
    FPM_POOL3_ALLOC_DEALLOC_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL3_ALLOC_DEALLOC_DDR
 ******************************************************************************/
const ru_field_rec FPM_POOL3_ALLOC_DEALLOC_DDR_FIELD =
{
    "DDR",
#if RU_INCLUDE_DESC
    "",
    "DDR Identifier\n"
    "0: DDR0\n"
    "1: DDR1\n",
#endif
    FPM_POOL3_ALLOC_DEALLOC_DDR_FIELD_MASK,
    0,
    FPM_POOL3_ALLOC_DEALLOC_DDR_FIELD_WIDTH,
    FPM_POOL3_ALLOC_DEALLOC_DDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL3_ALLOC_DEALLOC_TOKEN_INDEX
 ******************************************************************************/
const ru_field_rec FPM_POOL3_ALLOC_DEALLOC_TOKEN_INDEX_FIELD =
{
    "TOKEN_INDEX",
#if RU_INCLUDE_DESC
    "",
    "Buffer Index Pointer\n",
#endif
    FPM_POOL3_ALLOC_DEALLOC_TOKEN_INDEX_FIELD_MASK,
    0,
    FPM_POOL3_ALLOC_DEALLOC_TOKEN_INDEX_FIELD_WIDTH,
    FPM_POOL3_ALLOC_DEALLOC_TOKEN_INDEX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL3_ALLOC_DEALLOC_TOKEN_SIZE
 ******************************************************************************/
const ru_field_rec FPM_POOL3_ALLOC_DEALLOC_TOKEN_SIZE_FIELD =
{
    "TOKEN_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Buffer length or packet size in bytes",
#endif
    FPM_POOL3_ALLOC_DEALLOC_TOKEN_SIZE_FIELD_MASK,
    0,
    FPM_POOL3_ALLOC_DEALLOC_TOKEN_SIZE_FIELD_WIDTH,
    FPM_POOL3_ALLOC_DEALLOC_TOKEN_SIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL4_ALLOC_DEALLOC_TOKEN_VALID
 ******************************************************************************/
const ru_field_rec FPM_POOL4_ALLOC_DEALLOC_TOKEN_VALID_FIELD =
{
    "TOKEN_VALID",
#if RU_INCLUDE_DESC
    "",
    "Valid Token Indicator\n"
    "0: No buffers available\n"
    "1: A valid token index is provided. If a token is de-allocated/freed without this bit set "
    "that causes an error and the token will be ignored, error counter in register offset 0xB8 will be "
    "incremented. ",
#endif
    FPM_POOL4_ALLOC_DEALLOC_TOKEN_VALID_FIELD_MASK,
    0,
    FPM_POOL4_ALLOC_DEALLOC_TOKEN_VALID_FIELD_WIDTH,
    FPM_POOL4_ALLOC_DEALLOC_TOKEN_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL4_ALLOC_DEALLOC_RESERVED0
 ******************************************************************************/
const ru_field_rec FPM_POOL4_ALLOC_DEALLOC_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_POOL4_ALLOC_DEALLOC_RESERVED0_FIELD_MASK,
    0,
    FPM_POOL4_ALLOC_DEALLOC_RESERVED0_FIELD_WIDTH,
    FPM_POOL4_ALLOC_DEALLOC_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL4_ALLOC_DEALLOC_DDR
 ******************************************************************************/
const ru_field_rec FPM_POOL4_ALLOC_DEALLOC_DDR_FIELD =
{
    "DDR",
#if RU_INCLUDE_DESC
    "",
    "DDR Identifier\n"
    "0: DDR0\n"
    "1: DDR1\n",
#endif
    FPM_POOL4_ALLOC_DEALLOC_DDR_FIELD_MASK,
    0,
    FPM_POOL4_ALLOC_DEALLOC_DDR_FIELD_WIDTH,
    FPM_POOL4_ALLOC_DEALLOC_DDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL4_ALLOC_DEALLOC_TOKEN_INDEX
 ******************************************************************************/
const ru_field_rec FPM_POOL4_ALLOC_DEALLOC_TOKEN_INDEX_FIELD =
{
    "TOKEN_INDEX",
#if RU_INCLUDE_DESC
    "",
    "Buffer Index Pointer\n",
#endif
    FPM_POOL4_ALLOC_DEALLOC_TOKEN_INDEX_FIELD_MASK,
    0,
    FPM_POOL4_ALLOC_DEALLOC_TOKEN_INDEX_FIELD_WIDTH,
    FPM_POOL4_ALLOC_DEALLOC_TOKEN_INDEX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL4_ALLOC_DEALLOC_TOKEN_SIZE
 ******************************************************************************/
const ru_field_rec FPM_POOL4_ALLOC_DEALLOC_TOKEN_SIZE_FIELD =
{
    "TOKEN_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Buffer length or packet size in bytes",
#endif
    FPM_POOL4_ALLOC_DEALLOC_TOKEN_SIZE_FIELD_MASK,
    0,
    FPM_POOL4_ALLOC_DEALLOC_TOKEN_SIZE_FIELD_WIDTH,
    FPM_POOL4_ALLOC_DEALLOC_TOKEN_SIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_SPARE_SPARE_BITS
 ******************************************************************************/
const ru_field_rec FPM_SPARE_SPARE_BITS_FIELD =
{
    "SPARE_BITS",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_SPARE_SPARE_BITS_FIELD_MASK,
    0,
    FPM_SPARE_SPARE_BITS_FIELD_WIDTH,
    FPM_SPARE_SPARE_BITS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL_MULTI_TOKEN_VALID
 ******************************************************************************/
const ru_field_rec FPM_POOL_MULTI_TOKEN_VALID_FIELD =
{
    "TOKEN_VALID",
#if RU_INCLUDE_DESC
    "",
    "Valid Token Indicator\n"
    "0: No buffers available\n"
    "1: A valid token index is provided. If a token multi-cast value is updated without this bit set, "
    "that causes an error and the token will be ignored, error counter in register offset 0xBC "
    "will be incremented. ",
#endif
    FPM_POOL_MULTI_TOKEN_VALID_FIELD_MASK,
    0,
    FPM_POOL_MULTI_TOKEN_VALID_FIELD_WIDTH,
    FPM_POOL_MULTI_TOKEN_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL_MULTI_RESERVED0
 ******************************************************************************/
const ru_field_rec FPM_POOL_MULTI_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_POOL_MULTI_RESERVED0_FIELD_MASK,
    0,
    FPM_POOL_MULTI_RESERVED0_FIELD_WIDTH,
    FPM_POOL_MULTI_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL_MULTI_DDR
 ******************************************************************************/
const ru_field_rec FPM_POOL_MULTI_DDR_FIELD =
{
    "DDR",
#if RU_INCLUDE_DESC
    "",
    "DDR Identifier\n"
    "0: DDR0\n"
    "1: DDR1\n",
#endif
    FPM_POOL_MULTI_DDR_FIELD_MASK,
    0,
    FPM_POOL_MULTI_DDR_FIELD_WIDTH,
    FPM_POOL_MULTI_DDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL_MULTI_TOKEN_INDEX
 ******************************************************************************/
const ru_field_rec FPM_POOL_MULTI_TOKEN_INDEX_FIELD =
{
    "TOKEN_INDEX",
#if RU_INCLUDE_DESC
    "",
    "Buffer Index Pointer\n",
#endif
    FPM_POOL_MULTI_TOKEN_INDEX_FIELD_MASK,
    0,
    FPM_POOL_MULTI_TOKEN_INDEX_FIELD_WIDTH,
    FPM_POOL_MULTI_TOKEN_INDEX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL_MULTI_UPDATE_TYPE
 ******************************************************************************/
const ru_field_rec FPM_POOL_MULTI_UPDATE_TYPE_FIELD =
{
    "UPDATE_TYPE",
#if RU_INCLUDE_DESC
    "",
    "1'b0 - Count value is replaced with new value in bits[6:0]\n"
    "1'b1 - Count value is incremented by value in bits[6:0]\n",
#endif
    FPM_POOL_MULTI_UPDATE_TYPE_FIELD_MASK,
    0,
    FPM_POOL_MULTI_UPDATE_TYPE_FIELD_WIDTH,
    FPM_POOL_MULTI_UPDATE_TYPE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL_MULTI_RESERVED1
 ******************************************************************************/
const ru_field_rec FPM_POOL_MULTI_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_POOL_MULTI_RESERVED1_FIELD_MASK,
    0,
    FPM_POOL_MULTI_RESERVED1_FIELD_WIDTH,
    FPM_POOL_MULTI_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_POOL_MULTI_TOKEN_MULTI
 ******************************************************************************/
const ru_field_rec FPM_POOL_MULTI_TOKEN_MULTI_FIELD =
{
    "TOKEN_MULTI",
#if RU_INCLUDE_DESC
    "",
    "New Multi-cast Value",
#endif
    FPM_POOL_MULTI_TOKEN_MULTI_FIELD_MASK,
    0,
    FPM_POOL_MULTI_TOKEN_MULTI_FIELD_WIDTH,
    FPM_POOL_MULTI_TOKEN_MULTI_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_FPM_BB_FORCE_FORCE
 ******************************************************************************/
const ru_field_rec FPM_FPM_BB_FORCE_FORCE_FIELD =
{
    "FORCE",
#if RU_INCLUDE_DESC
    "FORCE",
    "Write 1 to force BB transaction",
#endif
    FPM_FPM_BB_FORCE_FORCE_FIELD_MASK,
    0,
    FPM_FPM_BB_FORCE_FORCE_FIELD_WIDTH,
    FPM_FPM_BB_FORCE_FORCE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_FPM_BB_FORCE_RESERVED0
 ******************************************************************************/
const ru_field_rec FPM_FPM_BB_FORCE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_FPM_BB_FORCE_RESERVED0_FIELD_MASK,
    0,
    FPM_FPM_BB_FORCE_RESERVED0_FIELD_WIDTH,
    FPM_FPM_BB_FORCE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_FPM_BB_FORCED_CTRL_CTRL
 ******************************************************************************/
const ru_field_rec FPM_FPM_BB_FORCED_CTRL_CTRL_FIELD =
{
    "CTRL",
#if RU_INCLUDE_DESC
    "CTRL",
    "Forced control",
#endif
    FPM_FPM_BB_FORCED_CTRL_CTRL_FIELD_MASK,
    0,
    FPM_FPM_BB_FORCED_CTRL_CTRL_FIELD_WIDTH,
    FPM_FPM_BB_FORCED_CTRL_CTRL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_FPM_BB_FORCED_CTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec FPM_FPM_BB_FORCED_CTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_FPM_BB_FORCED_CTRL_RESERVED0_FIELD_MASK,
    0,
    FPM_FPM_BB_FORCED_CTRL_RESERVED0_FIELD_WIDTH,
    FPM_FPM_BB_FORCED_CTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_FPM_BB_FORCED_ADDR_TA_ADDR
 ******************************************************************************/
const ru_field_rec FPM_FPM_BB_FORCED_ADDR_TA_ADDR_FIELD =
{
    "TA_ADDR",
#if RU_INCLUDE_DESC
    "TA_ADDR",
    "Forced TA address",
#endif
    FPM_FPM_BB_FORCED_ADDR_TA_ADDR_FIELD_MASK,
    0,
    FPM_FPM_BB_FORCED_ADDR_TA_ADDR_FIELD_WIDTH,
    FPM_FPM_BB_FORCED_ADDR_TA_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_FPM_BB_FORCED_ADDR_DEST_ADDR
 ******************************************************************************/
const ru_field_rec FPM_FPM_BB_FORCED_ADDR_DEST_ADDR_FIELD =
{
    "DEST_ADDR",
#if RU_INCLUDE_DESC
    "DEST_ADD",
    "Forced destination address",
#endif
    FPM_FPM_BB_FORCED_ADDR_DEST_ADDR_FIELD_MASK,
    0,
    FPM_FPM_BB_FORCED_ADDR_DEST_ADDR_FIELD_WIDTH,
    FPM_FPM_BB_FORCED_ADDR_DEST_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_FPM_BB_FORCED_ADDR_RESERVED0
 ******************************************************************************/
const ru_field_rec FPM_FPM_BB_FORCED_ADDR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_FPM_BB_FORCED_ADDR_RESERVED0_FIELD_MASK,
    0,
    FPM_FPM_BB_FORCED_ADDR_RESERVED0_FIELD_WIDTH,
    FPM_FPM_BB_FORCED_ADDR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_FPM_BB_FORCED_DATA_DATA
 ******************************************************************************/
const ru_field_rec FPM_FPM_BB_FORCED_DATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA",
    "Forced data",
#endif
    FPM_FPM_BB_FORCED_DATA_DATA_FIELD_MASK,
    0,
    FPM_FPM_BB_FORCED_DATA_DATA_FIELD_WIDTH,
    FPM_FPM_BB_FORCED_DATA_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_FPM_BB_DECODE_CFG_DEST_ID
 ******************************************************************************/
const ru_field_rec FPM_FPM_BB_DECODE_CFG_DEST_ID_FIELD =
{
    "DEST_ID",
#if RU_INCLUDE_DESC
    "DEST_ID",
    "destination id",
#endif
    FPM_FPM_BB_DECODE_CFG_DEST_ID_FIELD_MASK,
    0,
    FPM_FPM_BB_DECODE_CFG_DEST_ID_FIELD_WIDTH,
    FPM_FPM_BB_DECODE_CFG_DEST_ID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_FPM_BB_DECODE_CFG_OVERRIDE_EN
 ******************************************************************************/
const ru_field_rec FPM_FPM_BB_DECODE_CFG_OVERRIDE_EN_FIELD =
{
    "OVERRIDE_EN",
#if RU_INCLUDE_DESC
    "OVERRIDE_EN",
    "Enable override",
#endif
    FPM_FPM_BB_DECODE_CFG_OVERRIDE_EN_FIELD_MASK,
    0,
    FPM_FPM_BB_DECODE_CFG_OVERRIDE_EN_FIELD_WIDTH,
    FPM_FPM_BB_DECODE_CFG_OVERRIDE_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_FPM_BB_DECODE_CFG_ROUTE_ADDR
 ******************************************************************************/
const ru_field_rec FPM_FPM_BB_DECODE_CFG_ROUTE_ADDR_FIELD =
{
    "ROUTE_ADDR",
#if RU_INCLUDE_DESC
    "ROUTE_ADDR",
    "route address",
#endif
    FPM_FPM_BB_DECODE_CFG_ROUTE_ADDR_FIELD_MASK,
    0,
    FPM_FPM_BB_DECODE_CFG_ROUTE_ADDR_FIELD_WIDTH,
    FPM_FPM_BB_DECODE_CFG_ROUTE_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_FPM_BB_DECODE_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec FPM_FPM_BB_DECODE_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_FPM_BB_DECODE_CFG_RESERVED0_FIELD_MASK,
    0,
    FPM_FPM_BB_DECODE_CFG_RESERVED0_FIELD_WIDTH,
    FPM_FPM_BB_DECODE_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_FPM_BB_DBG_CFG_RXFIFO_SW_ADDR
 ******************************************************************************/
const ru_field_rec FPM_FPM_BB_DBG_CFG_RXFIFO_SW_ADDR_FIELD =
{
    "RXFIFO_SW_ADDR",
#if RU_INCLUDE_DESC
    "RXFIFO_SW_ADDR",
    "SW address for reading FPM BB RXFIFO",
#endif
    FPM_FPM_BB_DBG_CFG_RXFIFO_SW_ADDR_FIELD_MASK,
    0,
    FPM_FPM_BB_DBG_CFG_RXFIFO_SW_ADDR_FIELD_WIDTH,
    FPM_FPM_BB_DBG_CFG_RXFIFO_SW_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_FPM_BB_DBG_CFG_TXFIFO_SW_ADDR
 ******************************************************************************/
const ru_field_rec FPM_FPM_BB_DBG_CFG_TXFIFO_SW_ADDR_FIELD =
{
    "TXFIFO_SW_ADDR",
#if RU_INCLUDE_DESC
    "TXFIFO_SW_ADDR",
    "SW address for reading FPM BB TXFIFO",
#endif
    FPM_FPM_BB_DBG_CFG_TXFIFO_SW_ADDR_FIELD_MASK,
    0,
    FPM_FPM_BB_DBG_CFG_TXFIFO_SW_ADDR_FIELD_WIDTH,
    FPM_FPM_BB_DBG_CFG_TXFIFO_SW_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_FPM_BB_DBG_CFG_RXFIFO_SW_RST
 ******************************************************************************/
const ru_field_rec FPM_FPM_BB_DBG_CFG_RXFIFO_SW_RST_FIELD =
{
    "RXFIFO_SW_RST",
#if RU_INCLUDE_DESC
    "RXFIFO_SW_RST",
    "SW reset for FPM BB RXFIFO",
#endif
    FPM_FPM_BB_DBG_CFG_RXFIFO_SW_RST_FIELD_MASK,
    0,
    FPM_FPM_BB_DBG_CFG_RXFIFO_SW_RST_FIELD_WIDTH,
    FPM_FPM_BB_DBG_CFG_RXFIFO_SW_RST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_FPM_BB_DBG_CFG_TXFIFO_SW_RST
 ******************************************************************************/
const ru_field_rec FPM_FPM_BB_DBG_CFG_TXFIFO_SW_RST_FIELD =
{
    "TXFIFO_SW_RST",
#if RU_INCLUDE_DESC
    "TXFIFO_SW_RST",
    "SW reset for FPM BB TXFIFO",
#endif
    FPM_FPM_BB_DBG_CFG_TXFIFO_SW_RST_FIELD_MASK,
    0,
    FPM_FPM_BB_DBG_CFG_TXFIFO_SW_RST_FIELD_WIDTH,
    FPM_FPM_BB_DBG_CFG_TXFIFO_SW_RST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_FPM_BB_DBG_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec FPM_FPM_BB_DBG_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_FPM_BB_DBG_CFG_RESERVED0_FIELD_MASK,
    0,
    FPM_FPM_BB_DBG_CFG_RESERVED0_FIELD_WIDTH,
    FPM_FPM_BB_DBG_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_EMPTY
 ******************************************************************************/
const ru_field_rec FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_EMPTY_FIELD =
{
    "FIFO_EMPTY",
#if RU_INCLUDE_DESC
    "FIFO_EMPTY",
    "FIFO is empty",
#endif
    FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_EMPTY_FIELD_MASK,
    0,
    FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_EMPTY_FIELD_WIDTH,
    FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_EMPTY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_FULL
 ******************************************************************************/
const ru_field_rec FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_FULL_FIELD =
{
    "FIFO_FULL",
#if RU_INCLUDE_DESC
    "FIFO_FULL",
    "FIFO is full",
#endif
    FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_FULL_FIELD_MASK,
    0,
    FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_FULL_FIELD_WIDTH,
    FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_FULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_FPM_BB_DBG_RXFIFO_STS_RESERVED0
 ******************************************************************************/
const ru_field_rec FPM_FPM_BB_DBG_RXFIFO_STS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_FPM_BB_DBG_RXFIFO_STS_RESERVED0_FIELD_MASK,
    0,
    FPM_FPM_BB_DBG_RXFIFO_STS_RESERVED0_FIELD_WIDTH,
    FPM_FPM_BB_DBG_RXFIFO_STS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_USED_WORDS
 ******************************************************************************/
const ru_field_rec FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_USED_WORDS_FIELD =
{
    "FIFO_USED_WORDS",
#if RU_INCLUDE_DESC
    "FIFO_USED_WORDS",
    "Used words",
#endif
    FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_USED_WORDS_FIELD_MASK,
    0,
    FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_USED_WORDS_FIELD_WIDTH,
    FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_USED_WORDS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_FPM_BB_DBG_RXFIFO_STS_RESERVED1
 ******************************************************************************/
const ru_field_rec FPM_FPM_BB_DBG_RXFIFO_STS_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_FPM_BB_DBG_RXFIFO_STS_RESERVED1_FIELD_MASK,
    0,
    FPM_FPM_BB_DBG_RXFIFO_STS_RESERVED1_FIELD_WIDTH,
    FPM_FPM_BB_DBG_RXFIFO_STS_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_RD_CNTR
 ******************************************************************************/
const ru_field_rec FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_RD_CNTR_FIELD =
{
    "FIFO_RD_CNTR",
#if RU_INCLUDE_DESC
    "FIFO_RD_CNTR",
    "Write counter",
#endif
    FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_RD_CNTR_FIELD_MASK,
    0,
    FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_RD_CNTR_FIELD_WIDTH,
    FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_RD_CNTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_FPM_BB_DBG_RXFIFO_STS_RESERVED2
 ******************************************************************************/
const ru_field_rec FPM_FPM_BB_DBG_RXFIFO_STS_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_FPM_BB_DBG_RXFIFO_STS_RESERVED2_FIELD_MASK,
    0,
    FPM_FPM_BB_DBG_RXFIFO_STS_RESERVED2_FIELD_WIDTH,
    FPM_FPM_BB_DBG_RXFIFO_STS_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_WR_CNTR
 ******************************************************************************/
const ru_field_rec FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_WR_CNTR_FIELD =
{
    "FIFO_WR_CNTR",
#if RU_INCLUDE_DESC
    "FIFO_WR_CNTR",
    "Write counter",
#endif
    FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_WR_CNTR_FIELD_MASK,
    0,
    FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_WR_CNTR_FIELD_WIDTH,
    FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_WR_CNTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_FPM_BB_DBG_RXFIFO_STS_RESERVED3
 ******************************************************************************/
const ru_field_rec FPM_FPM_BB_DBG_RXFIFO_STS_RESERVED3_FIELD =
{
    "RESERVED3",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_FPM_BB_DBG_RXFIFO_STS_RESERVED3_FIELD_MASK,
    0,
    FPM_FPM_BB_DBG_RXFIFO_STS_RESERVED3_FIELD_WIDTH,
    FPM_FPM_BB_DBG_RXFIFO_STS_RESERVED3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_EMPTY
 ******************************************************************************/
const ru_field_rec FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_EMPTY_FIELD =
{
    "FIFO_EMPTY",
#if RU_INCLUDE_DESC
    "FIFO_EMPTY",
    "FIFO is empty",
#endif
    FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_EMPTY_FIELD_MASK,
    0,
    FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_EMPTY_FIELD_WIDTH,
    FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_EMPTY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_FULL
 ******************************************************************************/
const ru_field_rec FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_FULL_FIELD =
{
    "FIFO_FULL",
#if RU_INCLUDE_DESC
    "FIFO_FULL",
    "FIFO is full",
#endif
    FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_FULL_FIELD_MASK,
    0,
    FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_FULL_FIELD_WIDTH,
    FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_FULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_FPM_BB_DBG_TXFIFO_STS_RESERVED0
 ******************************************************************************/
const ru_field_rec FPM_FPM_BB_DBG_TXFIFO_STS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_FPM_BB_DBG_TXFIFO_STS_RESERVED0_FIELD_MASK,
    0,
    FPM_FPM_BB_DBG_TXFIFO_STS_RESERVED0_FIELD_WIDTH,
    FPM_FPM_BB_DBG_TXFIFO_STS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_USED_WORDS
 ******************************************************************************/
const ru_field_rec FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_USED_WORDS_FIELD =
{
    "FIFO_USED_WORDS",
#if RU_INCLUDE_DESC
    "FIFO_USED_WORDS",
    "Used words",
#endif
    FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_USED_WORDS_FIELD_MASK,
    0,
    FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_USED_WORDS_FIELD_WIDTH,
    FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_USED_WORDS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_FPM_BB_DBG_TXFIFO_STS_RESERVED1
 ******************************************************************************/
const ru_field_rec FPM_FPM_BB_DBG_TXFIFO_STS_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_FPM_BB_DBG_TXFIFO_STS_RESERVED1_FIELD_MASK,
    0,
    FPM_FPM_BB_DBG_TXFIFO_STS_RESERVED1_FIELD_WIDTH,
    FPM_FPM_BB_DBG_TXFIFO_STS_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_RD_CNTR
 ******************************************************************************/
const ru_field_rec FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_RD_CNTR_FIELD =
{
    "FIFO_RD_CNTR",
#if RU_INCLUDE_DESC
    "FIFO_RD_CNTR",
    "Write counter",
#endif
    FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_RD_CNTR_FIELD_MASK,
    0,
    FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_RD_CNTR_FIELD_WIDTH,
    FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_RD_CNTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_FPM_BB_DBG_TXFIFO_STS_RESERVED2
 ******************************************************************************/
const ru_field_rec FPM_FPM_BB_DBG_TXFIFO_STS_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_FPM_BB_DBG_TXFIFO_STS_RESERVED2_FIELD_MASK,
    0,
    FPM_FPM_BB_DBG_TXFIFO_STS_RESERVED2_FIELD_WIDTH,
    FPM_FPM_BB_DBG_TXFIFO_STS_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_WR_CNTR
 ******************************************************************************/
const ru_field_rec FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_WR_CNTR_FIELD =
{
    "FIFO_WR_CNTR",
#if RU_INCLUDE_DESC
    "FIFO_WR_CNTR",
    "Write counter",
#endif
    FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_WR_CNTR_FIELD_MASK,
    0,
    FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_WR_CNTR_FIELD_WIDTH,
    FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_WR_CNTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_FPM_BB_DBG_TXFIFO_STS_RESERVED3
 ******************************************************************************/
const ru_field_rec FPM_FPM_BB_DBG_TXFIFO_STS_RESERVED3_FIELD =
{
    "RESERVED3",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FPM_FPM_BB_DBG_TXFIFO_STS_RESERVED3_FIELD_MASK,
    0,
    FPM_FPM_BB_DBG_TXFIFO_STS_RESERVED3_FIELD_WIDTH,
    FPM_FPM_BB_DBG_TXFIFO_STS_RESERVED3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_FPM_BB_DBG_RXFIFO_DATA1_DATA
 ******************************************************************************/
const ru_field_rec FPM_FPM_BB_DBG_RXFIFO_DATA1_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA",
    "data",
#endif
    FPM_FPM_BB_DBG_RXFIFO_DATA1_DATA_FIELD_MASK,
    0,
    FPM_FPM_BB_DBG_RXFIFO_DATA1_DATA_FIELD_WIDTH,
    FPM_FPM_BB_DBG_RXFIFO_DATA1_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_FPM_BB_DBG_RXFIFO_DATA2_DATA
 ******************************************************************************/
const ru_field_rec FPM_FPM_BB_DBG_RXFIFO_DATA2_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA",
    "data",
#endif
    FPM_FPM_BB_DBG_RXFIFO_DATA2_DATA_FIELD_MASK,
    0,
    FPM_FPM_BB_DBG_RXFIFO_DATA2_DATA_FIELD_WIDTH,
    FPM_FPM_BB_DBG_RXFIFO_DATA2_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_FPM_BB_DBG_TXFIFO_DATA1_DATA
 ******************************************************************************/
const ru_field_rec FPM_FPM_BB_DBG_TXFIFO_DATA1_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA",
    "data",
#endif
    FPM_FPM_BB_DBG_TXFIFO_DATA1_DATA_FIELD_MASK,
    0,
    FPM_FPM_BB_DBG_TXFIFO_DATA1_DATA_FIELD_WIDTH,
    FPM_FPM_BB_DBG_TXFIFO_DATA1_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_FPM_BB_DBG_TXFIFO_DATA2_DATA
 ******************************************************************************/
const ru_field_rec FPM_FPM_BB_DBG_TXFIFO_DATA2_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA",
    "data",
#endif
    FPM_FPM_BB_DBG_TXFIFO_DATA2_DATA_FIELD_MASK,
    0,
    FPM_FPM_BB_DBG_TXFIFO_DATA2_DATA_FIELD_WIDTH,
    FPM_FPM_BB_DBG_TXFIFO_DATA2_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: FPM_FPM_BB_DBG_TXFIFO_DATA3_DATA
 ******************************************************************************/
const ru_field_rec FPM_FPM_BB_DBG_TXFIFO_DATA3_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA",
    "data",
#endif
    FPM_FPM_BB_DBG_TXFIFO_DATA3_DATA_FIELD_MASK,
    0,
    FPM_FPM_BB_DBG_TXFIFO_DATA3_DATA_FIELD_WIDTH,
    FPM_FPM_BB_DBG_TXFIFO_DATA3_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: FPM_FPM_CTL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_FPM_CTL_FIELDS[] =
{
    &FPM_FPM_CTL_TP_MUX_CNTRL_FIELD,
    &FPM_FPM_CTL_ENABLE_HIGH_TOK_ALWAYS_FIELD,
    &FPM_FPM_CTL_MEM_CORRUPT_CHECK_DISABLE_FIELD,
    &FPM_FPM_CTL_STOP_ALLOC_CACHE_LOAD_FIELD,
    &FPM_FPM_CTL_RESERVED0_FIELD,
    &FPM_FPM_CTL_POOL2_ENABLE_FIELD,
    &FPM_FPM_CTL_POOL1_ENABLE_FIELD,
    &FPM_FPM_CTL_STRICT_PRIORITY_REQUEST_TYPE_FIELD,
    &FPM_FPM_CTL_FPM_BB_SOFT_RESET_FIELD,
    &FPM_FPM_CTL_WEIGHT_FOR_ROUND_ROBIN_POLICY_FIELD,
    &FPM_FPM_CTL_ARBITRATION_POLICY_FIELD,
    &FPM_FPM_CTL_INIT_MEM_FIELD,
    &FPM_FPM_CTL_INIT_MEM_POOL2_FIELD,
    &FPM_FPM_CTL_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_FPM_CTL_REG = 
{
    "FPM_CTL",
#if RU_INCLUDE_DESC
    "FPM Control Register",
    "",
#endif
    FPM_FPM_CTL_REG_OFFSET,
    0,
    0,
    169,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    14,
    FPM_FPM_CTL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_FPM_CFG1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_FPM_CFG1_FIELDS[] =
{
    &FPM_FPM_CFG1_RESERVED0_FIELD,
    &FPM_FPM_CFG1_POOL4_CACHE_BYPASS_EN_FIELD,
    &FPM_FPM_CFG1_POOL3_CACHE_BYPASS_EN_FIELD,
    &FPM_FPM_CFG1_POOL2_CACHE_BYPASS_EN_FIELD,
    &FPM_FPM_CFG1_POOL1_CACHE_BYPASS_EN_FIELD,
    &FPM_FPM_CFG1_POOL4_SEARCH_MODE_FIELD,
    &FPM_FPM_CFG1_POOL3_SEARCH_MODE_FIELD,
    &FPM_FPM_CFG1_POOL2_SEARCH_MODE_FIELD,
    &FPM_FPM_CFG1_POOL1_SEARCH_MODE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_FPM_CFG1_REG = 
{
    "FPM_CFG1",
#if RU_INCLUDE_DESC
    "FPM Configuration Register",
    "",
#endif
    FPM_FPM_CFG1_REG_OFFSET,
    0,
    0,
    170,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    FPM_FPM_CFG1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_FPM_WEIGHT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_FPM_WEIGHT_FIELDS[] =
{
    &FPM_FPM_WEIGHT_DDR1_FREE_WEIGHT_FIELD,
    &FPM_FPM_WEIGHT_DDR1_ALLOC_WEIGHT_FIELD,
    &FPM_FPM_WEIGHT_DDR0_FREE_WEIGHT_FIELD,
    &FPM_FPM_WEIGHT_DDR0_ALLOC_WEIGHT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_FPM_WEIGHT_REG = 
{
    "FPM_WEIGHT",
#if RU_INCLUDE_DESC
    "FPM Configuration Register",
    "",
#endif
    FPM_FPM_WEIGHT_REG_OFFSET,
    0,
    0,
    171,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    FPM_FPM_WEIGHT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_FPM_BB_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_FPM_BB_CFG_FIELDS[] =
{
    &FPM_FPM_BB_CFG_RESERVED0_FIELD,
    &FPM_FPM_BB_CFG_BB_DDR_SEL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_FPM_BB_CFG_REG = 
{
    "FPM_BB_CFG",
#if RU_INCLUDE_DESC
    "FPM_BB Configuration Register",
    "",
#endif
    FPM_FPM_BB_CFG_REG_OFFSET,
    0,
    0,
    172,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_FPM_BB_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_POOL1_INTR_MSK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL1_INTR_MSK_FIELDS[] =
{
    &FPM_POOL1_INTR_MSK_RESERVED0_FIELD,
    &FPM_POOL1_INTR_MSK_EXPIRED_TOKEN_RECOV_MSK_FIELD,
    &FPM_POOL1_INTR_MSK_EXPIRED_TOKEN_DET_MSK_FIELD,
    &FPM_POOL1_INTR_MSK_ILLEGAL_ALLOC_REQUEST_MSK_FIELD,
    &FPM_POOL1_INTR_MSK_ILLEGAL_ADDRESS_ACCESS_MSK_FIELD,
    &FPM_POOL1_INTR_MSK_XON_MSK_FIELD,
    &FPM_POOL1_INTR_MSK_XOFF_MSK_FIELD,
    &FPM_POOL1_INTR_MSK_MEMORY_CORRUPT_MSK_FIELD,
    &FPM_POOL1_INTR_MSK_POOL_DIS_FREE_MULTI_MSK_FIELD,
    &FPM_POOL1_INTR_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD,
    &FPM_POOL1_INTR_MSK_MULTI_TOKEN_NO_VALID_MSK_FIELD,
    &FPM_POOL1_INTR_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD,
    &FPM_POOL1_INTR_MSK_FREE_TOKEN_NO_VALID_MSK_FIELD,
    &FPM_POOL1_INTR_MSK_POOL_FULL_MSK_FIELD,
    &FPM_POOL1_INTR_MSK_FREE_FIFO_FULL_MSK_FIELD,
    &FPM_POOL1_INTR_MSK_ALLOC_FIFO_FULL_MSK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_POOL1_INTR_MSK_REG = 
{
    "POOL1_INTR_MSK",
#if RU_INCLUDE_DESC
    "POOL2 Interrupt Mask Register",
    "Mask bits are active high and are disabled by default. Software enables "
    "desired bits as necessary",
#endif
    FPM_POOL1_INTR_MSK_REG_OFFSET,
    0,
    0,
    173,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    16,
    FPM_POOL1_INTR_MSK_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_POOL1_INTR_STS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL1_INTR_STS_FIELDS[] =
{
    &FPM_POOL1_INTR_STS_RESERVED0_FIELD,
    &FPM_POOL1_INTR_STS_EXPIRED_TOKEN_RECOV_STS_FIELD,
    &FPM_POOL1_INTR_STS_EXPIRED_TOKEN_DET_STS_FIELD,
    &FPM_POOL1_INTR_STS_ILLEGAL_ALLOC_REQUEST_STS_FIELD,
    &FPM_POOL1_INTR_STS_ILLEGAL_ADDRESS_ACCESS_STS_FIELD,
    &FPM_POOL1_INTR_STS_XON_STATE_STS_FIELD,
    &FPM_POOL1_INTR_STS_XOFF_STATE_STS_FIELD,
    &FPM_POOL1_INTR_STS_MEMORY_CORRUPT_STS_FIELD,
    &FPM_POOL1_INTR_STS_POOL_DIS_FREE_MULTI_STS_FIELD,
    &FPM_POOL1_INTR_STS_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD,
    &FPM_POOL1_INTR_STS_MULTI_TOKEN_NO_VALID_STS_FIELD,
    &FPM_POOL1_INTR_STS_FREE_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD,
    &FPM_POOL1_INTR_STS_FREE_TOKEN_NO_VALID_STS_FIELD,
    &FPM_POOL1_INTR_STS_POOL_FULL_STS_FIELD,
    &FPM_POOL1_INTR_STS_FREE_FIFO_FULL_STS_FIELD,
    &FPM_POOL1_INTR_STS_ALLOC_FIFO_FULL_STS_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_POOL1_INTR_STS_REG = 
{
    "POOL1_INTR_STS",
#if RU_INCLUDE_DESC
    "POOL2 Interrupt Status Register",
    "Interrupt bits are active high. When a bit in this register is set to 1 and the "
    "corresponding bit in interrupt mask register is set to 1, interrupt to CPU will "
    "occur. When set (1), interrupts bits can be cleared (0) by writing a 1 to the "
    "desired bit. ",
#endif
    FPM_POOL1_INTR_STS_REG_OFFSET,
    0,
    0,
    174,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    16,
    FPM_POOL1_INTR_STS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_POOL1_STALL_MSK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL1_STALL_MSK_FIELDS[] =
{
    &FPM_POOL1_STALL_MSK_RESERVED0_FIELD,
    &FPM_POOL1_STALL_MSK_MEMORY_CORRUPT_STALL_MSK_FIELD,
    &FPM_POOL1_STALL_MSK_RESERVED1_FIELD,
    &FPM_POOL1_STALL_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD,
    &FPM_POOL1_STALL_MSK_MULTI_TOKEN_NO_VALID_STALL_MSK_FIELD,
    &FPM_POOL1_STALL_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD,
    &FPM_POOL1_STALL_MSK_FREE_TOKEN_NO_VALID_STALL_MSK_FIELD,
    &FPM_POOL1_STALL_MSK_RESERVED2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_POOL1_STALL_MSK_REG = 
{
    "POOL1_STALL_MSK",
#if RU_INCLUDE_DESC
    "POOL2 Stall FPM mask",
    "Software sets desired stall bits that upon corresponding active interrupt status "
    "will stall FPM from new allocation, de-allocation, and mcast update process. Listed "
    "below are the supported interrupt statuses \n"
    "1. Invalid free token (bit[3] of interrupt status register 0x14)\n"
    "2. Invalid free token with index out-of-range (bit[4] of interrupt status register 0x14)\n"
    "3. Invalid mcast token (bit[5] of interrupt status register 0x14)\n"
    "4. Invalid mcast token with index out-of-range (bit[6] of interrupt status register 0x14)\n"
    "5. Memory corrupt status (bit[8] of interrupt status register 0x14)\n"
    "When state machine is stalled, registers and memory can still be accessed. Any new token "
    "allocation request will be serviced with valid tokens (if available in alloc cache) and invalid "
    "tokens (if alloc cache is empty). Any new de-allocation/mcast update requests will be either stored "
    "in de-allocation fifo (if there is space in free fifo) or dropped (if free fifo is full). "
    "Bit locations in this register matches the location of corrseponding interrupt status bits in "
    "register 0x14. To un-stall (enable) state machine interrupt status bits (in register 0x14) "
    "corresponding to these mask bits should be cleared. Stall mask bits are active high and "
    "are disabled by default. This is for debug purposes only. ",
#endif
    FPM_POOL1_STALL_MSK_REG_OFFSET,
    0,
    0,
    175,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    FPM_POOL1_STALL_MSK_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_POOL2_INTR_MSK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL2_INTR_MSK_FIELDS[] =
{
    &FPM_POOL2_INTR_MSK_RESERVED0_FIELD,
    &FPM_POOL2_INTR_MSK_EXPIRED_TOKEN_RECOV_MSK_FIELD,
    &FPM_POOL2_INTR_MSK_EXPIRED_TOKEN_DET_MSK_FIELD,
    &FPM_POOL2_INTR_MSK_ILLEGAL_ALLOC_REQUEST_MSK_FIELD,
    &FPM_POOL2_INTR_MSK_ILLEGAL_ADDRESS_ACCESS_MSK_FIELD,
    &FPM_POOL2_INTR_MSK_XON_MSK_FIELD,
    &FPM_POOL2_INTR_MSK_XOFF_MSK_FIELD,
    &FPM_POOL2_INTR_MSK_MEMORY_CORRUPT_MSK_FIELD,
    &FPM_POOL2_INTR_MSK_POOL_DIS_FREE_MULTI_MSK_FIELD,
    &FPM_POOL2_INTR_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD,
    &FPM_POOL2_INTR_MSK_MULTI_TOKEN_NO_VALID_MSK_FIELD,
    &FPM_POOL2_INTR_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_MSK_FIELD,
    &FPM_POOL2_INTR_MSK_FREE_TOKEN_NO_VALID_MSK_FIELD,
    &FPM_POOL2_INTR_MSK_POOL_FULL_MSK_FIELD,
    &FPM_POOL2_INTR_MSK_FREE_FIFO_FULL_MSK_FIELD,
    &FPM_POOL2_INTR_MSK_ALLOC_FIFO_FULL_MSK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_POOL2_INTR_MSK_REG = 
{
    "POOL2_INTR_MSK",
#if RU_INCLUDE_DESC
    "POOL2 Interrupt Mask Register",
    "Mask bits are active high and are disabled by default. Software enables "
    "desired bits as necessary",
#endif
    FPM_POOL2_INTR_MSK_REG_OFFSET,
    0,
    0,
    176,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    16,
    FPM_POOL2_INTR_MSK_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_POOL2_INTR_STS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL2_INTR_STS_FIELDS[] =
{
    &FPM_POOL2_INTR_STS_RESERVED0_FIELD,
    &FPM_POOL2_INTR_STS_EXPIRED_TOKEN_RECOV_STS_FIELD,
    &FPM_POOL2_INTR_STS_EXPIRED_TOKEN_DET_STS_FIELD,
    &FPM_POOL2_INTR_STS_ILLEGAL_ALLOC_REQUEST_STS_FIELD,
    &FPM_POOL2_INTR_STS_ILLEGAL_ADDRESS_ACCESS_STS_FIELD,
    &FPM_POOL2_INTR_STS_XON_STATE_STS_FIELD,
    &FPM_POOL2_INTR_STS_XOFF_STATE_STS_FIELD,
    &FPM_POOL2_INTR_STS_MEMORY_CORRUPT_STS_FIELD,
    &FPM_POOL2_INTR_STS_POOL_DIS_FREE_MULTI_STS_FIELD,
    &FPM_POOL2_INTR_STS_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD,
    &FPM_POOL2_INTR_STS_MULTI_TOKEN_NO_VALID_STS_FIELD,
    &FPM_POOL2_INTR_STS_FREE_TOKEN_INDEX_OUT_OF_RANGE_STS_FIELD,
    &FPM_POOL2_INTR_STS_FREE_TOKEN_NO_VALID_STS_FIELD,
    &FPM_POOL2_INTR_STS_POOL_FULL_STS_FIELD,
    &FPM_POOL2_INTR_STS_FREE_FIFO_FULL_STS_FIELD,
    &FPM_POOL2_INTR_STS_ALLOC_FIFO_FULL_STS_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_POOL2_INTR_STS_REG = 
{
    "POOL2_INTR_STS",
#if RU_INCLUDE_DESC
    "POOL2 Interrupt Status Register",
    "Interrupt bits are active high. When a bit in this register is set to 1 and the "
    "corresponding bit in interrupt mask register is set to 1, interrupt to CPU will "
    "occur. When set (1), interrupts bits can be cleared (0) by writing a 1 to the "
    "desired bit. ",
#endif
    FPM_POOL2_INTR_STS_REG_OFFSET,
    0,
    0,
    177,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    16,
    FPM_POOL2_INTR_STS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_POOL2_STALL_MSK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL2_STALL_MSK_FIELDS[] =
{
    &FPM_POOL2_STALL_MSK_RESERVED0_FIELD,
    &FPM_POOL2_STALL_MSK_MEMORY_CORRUPT_STALL_MSK_FIELD,
    &FPM_POOL2_STALL_MSK_RESERVED1_FIELD,
    &FPM_POOL2_STALL_MSK_MULTI_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD,
    &FPM_POOL2_STALL_MSK_MULTI_TOKEN_NO_VALID_STALL_MSK_FIELD,
    &FPM_POOL2_STALL_MSK_FREE_TOKEN_INDEX_OUT_OF_RANGE_STALL_MSK_FIELD,
    &FPM_POOL2_STALL_MSK_FREE_TOKEN_NO_VALID_STALL_MSK_FIELD,
    &FPM_POOL2_STALL_MSK_RESERVED2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_POOL2_STALL_MSK_REG = 
{
    "POOL2_STALL_MSK",
#if RU_INCLUDE_DESC
    "POOL2 Stall FPM mask",
    "Software sets desired stall bits that upon corresponding active interrupt status "
    "will stall FPM from new allocation, de-allocation, and mcast update process. Listed "
    "below are the supported interrupt statuses \n"
    "1. Invalid free token (bit[3] of interrupt status register 0x14)\n"
    "2. Invalid free token with index out-of-range (bit[4] of interrupt status register 0x14)\n"
    "3. Invalid mcast token (bit[5] of interrupt status register 0x14)\n"
    "4. Invalid mcast token with index out-of-range (bit[6] of interrupt status register 0x14)\n"
    "5. Memory corrupt status (bit[8] of interrupt status register 0x14)\n"
    "When state machine is stalled, registers and memory can still be accessed. Any new token "
    "allocation request will be serviced with valid tokens (if available in alloc cache) and invalid "
    "tokens (if alloc cache is empty). Any new de-allocation/mcast update requests will be either stored "
    "in de-allocation fifo (if there is space in free fifo) or dropped (if free fifo is full). "
    "Bit locations in this register matches the location of corrseponding interrupt status bits in "
    "register 0x14. To un-stall (enable) state machine interrupt status bits (in register 0x14) "
    "corresponding to these mask bits should be cleared. Stall mask bits are active high and "
    "are disabled by default. This is for debug purposes only. ",
#endif
    FPM_POOL2_STALL_MSK_REG_OFFSET,
    0,
    0,
    178,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    FPM_POOL2_STALL_MSK_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_POOL1_CFG1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL1_CFG1_FIELDS[] =
{
    &FPM_POOL1_CFG1_RESERVED0_FIELD,
    &FPM_POOL1_CFG1_FP_BUF_SIZE_FIELD,
    &FPM_POOL1_CFG1_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_POOL1_CFG1_REG = 
{
    "POOL1_CFG1",
#if RU_INCLUDE_DESC
    "POOL1 Configuration Register 1",
    "",
#endif
    FPM_POOL1_CFG1_REG_OFFSET,
    0,
    0,
    179,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    FPM_POOL1_CFG1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_POOL1_CFG2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL1_CFG2_FIELDS[] =
{
    &FPM_POOL1_CFG2_POOL_BASE_ADDRESS_FIELD,
    &FPM_POOL1_CFG2_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_POOL1_CFG2_REG = 
{
    "POOL1_CFG2",
#if RU_INCLUDE_DESC
    "POOL1 Configuration Register 2",
    "This register sets the physical base address of this memory. "
    "The memory block should be the number of buffers times "
    "the buffer size. This is mainly used for multi-pool memory "
    "configuration. NOTE: POOL_BASE_ADDRESS[7:2] and reserved[1:0] "
    "field must be written with 0x00 in the BCM3382 because its"
    "token-to-address converter assumes the buffers start on "
    "a 2kB boundary.",
#endif
    FPM_POOL1_CFG2_REG_OFFSET,
    0,
    0,
    180,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_POOL1_CFG2_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_POOL1_CFG3
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL1_CFG3_FIELDS[] =
{
    &FPM_POOL1_CFG3_POOL_BASE_ADDRESS_POOL2_FIELD,
    &FPM_POOL1_CFG3_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_POOL1_CFG3_REG = 
{
    "POOL1_CFG3",
#if RU_INCLUDE_DESC
    "POOL1 Configuration Register 3",
    "This register sets the physical base address of this memory. "
    "The memory block should be the number of buffers times "
    "the buffer size. This is mainly used for multi-pool memory "
    "configuration. ",
#endif
    FPM_POOL1_CFG3_REG_OFFSET,
    0,
    0,
    181,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_POOL1_CFG3_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_POOL1_STAT1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL1_STAT1_FIELDS[] =
{
    &FPM_POOL1_STAT1_OVRFL_FIELD,
    &FPM_POOL1_STAT1_UNDRFL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_POOL1_STAT1_REG = 
{
    "POOL1_STAT1",
#if RU_INCLUDE_DESC
    "POOL2 Status Register 1",
    "This read only register allows software to read the count of "
    "free pool overflows and underflows. A overflow condition occurs when pool is empty, "
    "ie., no tokens are allocated and free/mcast request is encountered. A underflow "
    "condition occurs when pool is full, ie., there are no free tokens and a "
    "allocation request is encountered. When the counter values reaches maximum "
    "count, it will hold the max value and not increment the count "
    "value unless it is cleared. Any write to this register will clear both "
    "both counters. ",
#endif
    FPM_POOL1_STAT1_REG_OFFSET,
    0,
    0,
    182,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_POOL1_STAT1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_POOL1_STAT2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL1_STAT2_FIELDS[] =
{
    &FPM_POOL1_STAT2_POOL_FULL_FIELD,
    &FPM_POOL1_STAT2_RESERVED0_FIELD,
    &FPM_POOL1_STAT2_FREE_FIFO_FULL_FIELD,
    &FPM_POOL1_STAT2_FREE_FIFO_EMPTY_FIELD,
    &FPM_POOL1_STAT2_ALLOC_FIFO_FULL_FIELD,
    &FPM_POOL1_STAT2_ALLOC_FIFO_EMPTY_FIELD,
    &FPM_POOL1_STAT2_RESERVED1_FIELD,
    &FPM_POOL1_STAT2_NUM_OF_TOKENS_AVAILABLE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_POOL1_STAT2_REG = 
{
    "POOL1_STAT2",
#if RU_INCLUDE_DESC
    "POOL2 Status Register 2",
    "This read only register provide status of index memory, "
    "alloc & free cache/fifos. These are real time statuses "
    "and bits are not sticky. Write to any bits will have no effect. ",
#endif
    FPM_POOL1_STAT2_REG_OFFSET,
    0,
    0,
    183,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    FPM_POOL1_STAT2_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_POOL1_STAT3
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL1_STAT3_FIELDS[] =
{
    &FPM_POOL1_STAT3_RESERVED0_FIELD,
    &FPM_POOL1_STAT3_NUM_OF_NOT_VALID_TOKEN_FREES_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_POOL1_STAT3_REG = 
{
    "POOL1_STAT3",
#if RU_INCLUDE_DESC
    "POOL2 Status Register 3",
    "This read only register allows software to read the count of "
    "free token requests with in-valid tokens "
    "When the counter values reaches maximum "
    "count, it will hold the max value and not increment the count "
    "value unless it is cleared. Any write to this register will clear "
    "count value. ",
#endif
    FPM_POOL1_STAT3_REG_OFFSET,
    0,
    0,
    184,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_POOL1_STAT3_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_POOL1_STAT4
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL1_STAT4_FIELDS[] =
{
    &FPM_POOL1_STAT4_RESERVED0_FIELD,
    &FPM_POOL1_STAT4_NUM_OF_NOT_VALID_TOKEN_MULTI_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_POOL1_STAT4_REG = 
{
    "POOL1_STAT4",
#if RU_INCLUDE_DESC
    "POOL2 Status Register 4",
    "This read only register allows software to read the count of "
    "multi-cast token update requests with in-valid tokens. "
    "When the counter values reaches maximum "
    "count, it will hold the max value and not increment the count "
    "value unless it is cleared. Any write to this register will clear "
    "count value. ",
#endif
    FPM_POOL1_STAT4_REG_OFFSET,
    0,
    0,
    185,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_POOL1_STAT4_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_POOL1_STAT5
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL1_STAT5_FIELDS[] =
{
    &FPM_POOL1_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_VALID_FIELD,
    &FPM_POOL1_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_POOL1_STAT5_REG = 
{
    "POOL1_STAT5",
#if RU_INCLUDE_DESC
    "POOL2 Status Register 5",
    "This read only register allows software to read the alloc token that "
    "causes memory corrupt interrupt (intr[8]) to go active. This is for debug "
    "purposes only. Any write to this register will clear "
    "token value (makes all bits zero). ",
#endif
    FPM_POOL1_STAT5_REG_OFFSET,
    0,
    0,
    186,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_POOL1_STAT5_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_POOL1_STAT6
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL1_STAT6_FIELDS[] =
{
    &FPM_POOL1_STAT6_INVALID_FREE_TOKEN_VALID_FIELD,
    &FPM_POOL1_STAT6_INVALID_FREE_TOKEN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_POOL1_STAT6_REG = 
{
    "POOL1_STAT6",
#if RU_INCLUDE_DESC
    "POOL2 Status Register 6",
    "This read only register allows software to read the free token that "
    "causes invalid free request or free token with index out-of-range interrupts "
    "(intr[3] or intr[4]) to go active. This is for debug "
    "purposes only. Any write to this register will clear "
    "token value (makes all bits zero). ",
#endif
    FPM_POOL1_STAT6_REG_OFFSET,
    0,
    0,
    187,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_POOL1_STAT6_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_POOL1_STAT7
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL1_STAT7_FIELDS[] =
{
    &FPM_POOL1_STAT7_INVALID_MCAST_TOKEN_VALID_FIELD,
    &FPM_POOL1_STAT7_INVALID_MCAST_TOKEN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_POOL1_STAT7_REG = 
{
    "POOL1_STAT7",
#if RU_INCLUDE_DESC
    "POOL2 Status Register 7",
    "This read only register allows software to read the multi-cast token that "
    "causes invalid mcast request or mcast token with index out-of-range interrupts "
    "(intr[5] or intr[6]) to go active. This is for debug "
    "purposes only. Any write to this register will clear "
    "token value (makes all bits zero). ",
#endif
    FPM_POOL1_STAT7_REG_OFFSET,
    0,
    0,
    188,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_POOL1_STAT7_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_POOL1_STAT8
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL1_STAT8_FIELDS[] =
{
    &FPM_POOL1_STAT8_RESERVED0_FIELD,
    &FPM_POOL1_STAT8_TOKENS_AVAILABLE_LOW_WTMK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_POOL1_STAT8_REG = 
{
    "POOL1_STAT8",
#if RU_INCLUDE_DESC
    "POOL2 Status Register 8",
    "This register allows software to read the lowest value "
    "the NUM_OF_TOKENS_AVAILABLE count reached since the last "
    "time it was cleared. Any write to this register will "
    "reset the value back to the maximum number of tokens (0x10000) ",
#endif
    FPM_POOL1_STAT8_REG_OFFSET,
    0,
    0,
    189,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_POOL1_STAT8_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_POOL2_STAT1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL2_STAT1_FIELDS[] =
{
    &FPM_POOL2_STAT1_OVRFL_FIELD,
    &FPM_POOL2_STAT1_UNDRFL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_POOL2_STAT1_REG = 
{
    "POOL2_STAT1",
#if RU_INCLUDE_DESC
    "POOL2 Status Register 1",
    "This read only register allows software to read the count of "
    "free pool overflows and underflows. A overflow condition occurs when pool is empty, "
    "ie., no tokens are allocated and free/mcast request is encountered. A underflow "
    "condition occurs when pool is full, ie., there are no free tokens and a "
    "allocation request is encountered. When the counter values reaches maximum "
    "count, it will hold the max value and not increment the count "
    "value unless it is cleared. Any write to this register will clear both "
    "both counters. ",
#endif
    FPM_POOL2_STAT1_REG_OFFSET,
    0,
    0,
    190,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_POOL2_STAT1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_POOL2_STAT2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL2_STAT2_FIELDS[] =
{
    &FPM_POOL2_STAT2_POOL_FULL_FIELD,
    &FPM_POOL2_STAT2_RESERVED0_FIELD,
    &FPM_POOL2_STAT2_FREE_FIFO_FULL_FIELD,
    &FPM_POOL2_STAT2_FREE_FIFO_EMPTY_FIELD,
    &FPM_POOL2_STAT2_ALLOC_FIFO_FULL_FIELD,
    &FPM_POOL2_STAT2_ALLOC_FIFO_EMPTY_FIELD,
    &FPM_POOL2_STAT2_RESERVED1_FIELD,
    &FPM_POOL2_STAT2_NUM_OF_TOKENS_AVAILABLE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_POOL2_STAT2_REG = 
{
    "POOL2_STAT2",
#if RU_INCLUDE_DESC
    "POOL2 Status Register 2",
    "This read only register provide status of index memory, "
    "alloc & free cache/fifos. These are real time statuses "
    "and bits are not sticky. Write to any bits will have no effect. ",
#endif
    FPM_POOL2_STAT2_REG_OFFSET,
    0,
    0,
    191,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    FPM_POOL2_STAT2_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_POOL2_STAT3
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL2_STAT3_FIELDS[] =
{
    &FPM_POOL2_STAT3_RESERVED0_FIELD,
    &FPM_POOL2_STAT3_NUM_OF_NOT_VALID_TOKEN_FREES_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_POOL2_STAT3_REG = 
{
    "POOL2_STAT3",
#if RU_INCLUDE_DESC
    "POOL2 Status Register 3",
    "This read only register allows software to read the count of "
    "free token requests with in-valid tokens "
    "When the counter values reaches maximum "
    "count, it will hold the max value and not increment the count "
    "value unless it is cleared. Any write to this register will clear "
    "count value. ",
#endif
    FPM_POOL2_STAT3_REG_OFFSET,
    0,
    0,
    192,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_POOL2_STAT3_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_POOL2_STAT4
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL2_STAT4_FIELDS[] =
{
    &FPM_POOL2_STAT4_RESERVED0_FIELD,
    &FPM_POOL2_STAT4_NUM_OF_NOT_VALID_TOKEN_MULTI_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_POOL2_STAT4_REG = 
{
    "POOL2_STAT4",
#if RU_INCLUDE_DESC
    "POOL2 Status Register 4",
    "This read only register allows software to read the count of "
    "multi-cast token update requests with in-valid tokens. "
    "When the counter values reaches maximum "
    "count, it will hold the max value and not increment the count "
    "value unless it is cleared. Any write to this register will clear "
    "count value. ",
#endif
    FPM_POOL2_STAT4_REG_OFFSET,
    0,
    0,
    193,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_POOL2_STAT4_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_POOL2_STAT5
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL2_STAT5_FIELDS[] =
{
    &FPM_POOL2_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_VALID_FIELD,
    &FPM_POOL2_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_POOL2_STAT5_REG = 
{
    "POOL2_STAT5",
#if RU_INCLUDE_DESC
    "POOL2 Status Register 5",
    "This read only register allows software to read the alloc token that "
    "causes memory corrupt interrupt (intr[8]) to go active. This is for debug "
    "purposes only. Any write to this register will clear "
    "token value (makes all bits zero). ",
#endif
    FPM_POOL2_STAT5_REG_OFFSET,
    0,
    0,
    194,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_POOL2_STAT5_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_POOL2_STAT6
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL2_STAT6_FIELDS[] =
{
    &FPM_POOL2_STAT6_INVALID_FREE_TOKEN_VALID_FIELD,
    &FPM_POOL2_STAT6_INVALID_FREE_TOKEN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_POOL2_STAT6_REG = 
{
    "POOL2_STAT6",
#if RU_INCLUDE_DESC
    "POOL2 Status Register 6",
    "This read only register allows software to read the free token that "
    "causes invalid free request or free token with index out-of-range interrupts "
    "(intr[3] or intr[4]) to go active. This is for debug "
    "purposes only. Any write to this register will clear "
    "token value (makes all bits zero). ",
#endif
    FPM_POOL2_STAT6_REG_OFFSET,
    0,
    0,
    195,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_POOL2_STAT6_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_POOL2_STAT7
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL2_STAT7_FIELDS[] =
{
    &FPM_POOL2_STAT7_INVALID_MCAST_TOKEN_VALID_FIELD,
    &FPM_POOL2_STAT7_INVALID_MCAST_TOKEN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_POOL2_STAT7_REG = 
{
    "POOL2_STAT7",
#if RU_INCLUDE_DESC
    "POOL2 Status Register 7",
    "This read only register allows software to read the multi-cast token that "
    "causes invalid mcast request or mcast token with index out-of-range interrupts "
    "(intr[5] or intr[6]) to go active. This is for debug "
    "purposes only. Any write to this register will clear "
    "token value (makes all bits zero). ",
#endif
    FPM_POOL2_STAT7_REG_OFFSET,
    0,
    0,
    196,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_POOL2_STAT7_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_POOL2_STAT8
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL2_STAT8_FIELDS[] =
{
    &FPM_POOL2_STAT8_RESERVED0_FIELD,
    &FPM_POOL2_STAT8_TOKENS_AVAILABLE_LOW_WTMK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_POOL2_STAT8_REG = 
{
    "POOL2_STAT8",
#if RU_INCLUDE_DESC
    "POOL2 Status Register 8",
    "This register allows software to read the lowest value "
    "the NUM_OF_TOKENS_AVAILABLE count reached since the last "
    "time it was cleared. Any write to this register will "
    "reset the value back to the maximum number of tokens (0x10000) ",
#endif
    FPM_POOL2_STAT8_REG_OFFSET,
    0,
    0,
    197,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_POOL2_STAT8_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_POOL1_XON_XOFF_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL1_XON_XOFF_CFG_FIELDS[] =
{
    &FPM_POOL1_XON_XOFF_CFG_XON_THRESHOLD_FIELD,
    &FPM_POOL1_XON_XOFF_CFG_XOFF_THRESHOLD_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_POOL1_XON_XOFF_CFG_REG = 
{
    "POOL1_XON_XOFF_CFG",
#if RU_INCLUDE_DESC
    "POOL1 XON/XOFF Threshold Configuration Register",
    "",
#endif
    FPM_POOL1_XON_XOFF_CFG_REG_OFFSET,
    0,
    0,
    198,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_POOL1_XON_XOFF_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_FPM_NOT_EMPTY_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_FPM_NOT_EMPTY_CFG_FIELDS[] =
{
    &FPM_FPM_NOT_EMPTY_CFG_RESERVED0_FIELD,
    &FPM_FPM_NOT_EMPTY_CFG_NOT_EMPTY_THRESHOLD_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_FPM_NOT_EMPTY_CFG_REG = 
{
    "FPM_NOT_EMPTY_CFG",
#if RU_INCLUDE_DESC
    "FPM_NOT_EMPTY Threshold Configuration Register",
    "",
#endif
    FPM_FPM_NOT_EMPTY_CFG_REG_OFFSET,
    0,
    0,
    199,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_FPM_NOT_EMPTY_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_MEM_CTL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_MEM_CTL_FIELDS[] =
{
    &FPM_MEM_CTL_MEM_WR_FIELD,
    &FPM_MEM_CTL_MEM_RD_FIELD,
    &FPM_MEM_CTL_MEM_SEL_FIELD,
    &FPM_MEM_CTL_RESERVED0_FIELD,
    &FPM_MEM_CTL_MEM_ADDR_FIELD,
    &FPM_MEM_CTL_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_MEM_CTL_REG = 
{
    "MEM_CTL",
#if RU_INCLUDE_DESC
    "Back door Memory Access Control Register",
    "",
#endif
    FPM_MEM_CTL_REG_OFFSET,
    0,
    0,
    200,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    FPM_MEM_CTL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_MEM_DATA1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_MEM_DATA1_FIELDS[] =
{
    &FPM_MEM_DATA1_MEM_DATA1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_MEM_DATA1_REG = 
{
    "MEM_DATA1",
#if RU_INCLUDE_DESC
    "Back door Memory Data1 Register",
    "",
#endif
    FPM_MEM_DATA1_REG_OFFSET,
    0,
    0,
    201,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_MEM_DATA1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_MEM_DATA2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_MEM_DATA2_FIELDS[] =
{
    &FPM_MEM_DATA2_MEM_DATA2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_MEM_DATA2_REG = 
{
    "MEM_DATA2",
#if RU_INCLUDE_DESC
    "Back door Memory Data2 Register",
    "",
#endif
    FPM_MEM_DATA2_REG_OFFSET,
    0,
    0,
    202,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_MEM_DATA2_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_TOKEN_RECOVER_CTL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_TOKEN_RECOVER_CTL_FIELDS[] =
{
    &FPM_TOKEN_RECOVER_CTL_RESERVED0_FIELD,
    &FPM_TOKEN_RECOVER_CTL_CLR_RECOVERED_TOKEN_COUNT_FIELD,
    &FPM_TOKEN_RECOVER_CTL_CLR_EXPIRED_TOKEN_COUNT_FIELD,
    &FPM_TOKEN_RECOVER_CTL_FORCE_TOKEN_RECLAIM_FIELD,
    &FPM_TOKEN_RECOVER_CTL_TOKEN_RECLAIM_ENA_FIELD,
    &FPM_TOKEN_RECOVER_CTL_TOKEN_REMARK_ENA_FIELD,
    &FPM_TOKEN_RECOVER_CTL_SINGLE_PASS_ENA_FIELD,
    &FPM_TOKEN_RECOVER_CTL_TOKEN_RECOVER_ENA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_TOKEN_RECOVER_CTL_REG = 
{
    "TOKEN_RECOVER_CTL",
#if RU_INCLUDE_DESC
    "Token Recovery Control Register",
    "",
#endif
    FPM_TOKEN_RECOVER_CTL_REG_OFFSET,
    0,
    0,
    203,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    FPM_TOKEN_RECOVER_CTL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_SHORT_AGING_TIMER
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_SHORT_AGING_TIMER_FIELDS[] =
{
    &FPM_SHORT_AGING_TIMER_TIMER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_SHORT_AGING_TIMER_REG = 
{
    "SHORT_AGING_TIMER",
#if RU_INCLUDE_DESC
    "Long Aging Timer",
    "",
#endif
    FPM_SHORT_AGING_TIMER_REG_OFFSET,
    0,
    0,
    204,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_SHORT_AGING_TIMER_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_LONG_AGING_TIMER
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_LONG_AGING_TIMER_FIELDS[] =
{
    &FPM_LONG_AGING_TIMER_TIMER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_LONG_AGING_TIMER_REG = 
{
    "LONG_AGING_TIMER",
#if RU_INCLUDE_DESC
    "Long Aging Timer",
    "",
#endif
    FPM_LONG_AGING_TIMER_REG_OFFSET,
    0,
    0,
    205,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_LONG_AGING_TIMER_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_CACHE_RECYCLE_TIMER
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_CACHE_RECYCLE_TIMER_FIELDS[] =
{
    &FPM_CACHE_RECYCLE_TIMER_RESERVED0_FIELD,
    &FPM_CACHE_RECYCLE_TIMER_RECYCLE_TIMER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_CACHE_RECYCLE_TIMER_REG = 
{
    "CACHE_RECYCLE_TIMER",
#if RU_INCLUDE_DESC
    "Token Cache Recycle Timer",
    "",
#endif
    FPM_CACHE_RECYCLE_TIMER_REG_OFFSET,
    0,
    0,
    206,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_CACHE_RECYCLE_TIMER_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_EXPIRED_TOKEN_COUNT_POOL1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_EXPIRED_TOKEN_COUNT_POOL1_FIELDS[] =
{
    &FPM_EXPIRED_TOKEN_COUNT_POOL1_COUNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_EXPIRED_TOKEN_COUNT_POOL1_REG = 
{
    "EXPIRED_TOKEN_COUNT_POOL1",
#if RU_INCLUDE_DESC
    "Expired Token Count",
    "",
#endif
    FPM_EXPIRED_TOKEN_COUNT_POOL1_REG_OFFSET,
    0,
    0,
    207,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_EXPIRED_TOKEN_COUNT_POOL1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_RECOVERED_TOKEN_COUNT_POOL1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_RECOVERED_TOKEN_COUNT_POOL1_FIELDS[] =
{
    &FPM_RECOVERED_TOKEN_COUNT_POOL1_COUNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_RECOVERED_TOKEN_COUNT_POOL1_REG = 
{
    "RECOVERED_TOKEN_COUNT_POOL1",
#if RU_INCLUDE_DESC
    "Recovered Token Count",
    "",
#endif
    FPM_RECOVERED_TOKEN_COUNT_POOL1_REG_OFFSET,
    0,
    0,
    208,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_RECOVERED_TOKEN_COUNT_POOL1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_EXPIRED_TOKEN_COUNT_POOL2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_EXPIRED_TOKEN_COUNT_POOL2_FIELDS[] =
{
    &FPM_EXPIRED_TOKEN_COUNT_POOL2_COUNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_EXPIRED_TOKEN_COUNT_POOL2_REG = 
{
    "EXPIRED_TOKEN_COUNT_POOL2",
#if RU_INCLUDE_DESC
    "Expired Token Count",
    "",
#endif
    FPM_EXPIRED_TOKEN_COUNT_POOL2_REG_OFFSET,
    0,
    0,
    209,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_EXPIRED_TOKEN_COUNT_POOL2_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_RECOVERED_TOKEN_COUNT_POOL2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_RECOVERED_TOKEN_COUNT_POOL2_FIELDS[] =
{
    &FPM_RECOVERED_TOKEN_COUNT_POOL2_COUNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_RECOVERED_TOKEN_COUNT_POOL2_REG = 
{
    "RECOVERED_TOKEN_COUNT_POOL2",
#if RU_INCLUDE_DESC
    "Recovered Token Count",
    "",
#endif
    FPM_RECOVERED_TOKEN_COUNT_POOL2_REG_OFFSET,
    0,
    0,
    210,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_RECOVERED_TOKEN_COUNT_POOL2_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_TOKEN_RECOVER_START_END_POOL1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_TOKEN_RECOVER_START_END_POOL1_FIELDS[] =
{
    &FPM_TOKEN_RECOVER_START_END_POOL1_RESERVED0_FIELD,
    &FPM_TOKEN_RECOVER_START_END_POOL1_START_INDEX_FIELD,
    &FPM_TOKEN_RECOVER_START_END_POOL1_RESERVED1_FIELD,
    &FPM_TOKEN_RECOVER_START_END_POOL1_END_INDEX_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_TOKEN_RECOVER_START_END_POOL1_REG = 
{
    "TOKEN_RECOVER_START_END_POOL1",
#if RU_INCLUDE_DESC
    "Token Recovery Start/End Range",
    "",
#endif
    FPM_TOKEN_RECOVER_START_END_POOL1_REG_OFFSET,
    0,
    0,
    211,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    FPM_TOKEN_RECOVER_START_END_POOL1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_TOKEN_RECOVER_START_END_POOL2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_TOKEN_RECOVER_START_END_POOL2_FIELDS[] =
{
    &FPM_TOKEN_RECOVER_START_END_POOL2_RESERVED0_FIELD,
    &FPM_TOKEN_RECOVER_START_END_POOL2_START_INDEX_FIELD,
    &FPM_TOKEN_RECOVER_START_END_POOL2_RESERVED1_FIELD,
    &FPM_TOKEN_RECOVER_START_END_POOL2_END_INDEX_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_TOKEN_RECOVER_START_END_POOL2_REG = 
{
    "TOKEN_RECOVER_START_END_POOL2",
#if RU_INCLUDE_DESC
    "Token Recovery Start/End Range",
    "",
#endif
    FPM_TOKEN_RECOVER_START_END_POOL2_REG_OFFSET,
    0,
    0,
    212,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    FPM_TOKEN_RECOVER_START_END_POOL2_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_POOL1_ALLOC_DEALLOC
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL1_ALLOC_DEALLOC_FIELDS[] =
{
    &FPM_POOL1_ALLOC_DEALLOC_TOKEN_VALID_FIELD,
    &FPM_POOL1_ALLOC_DEALLOC_RESERVED0_FIELD,
    &FPM_POOL1_ALLOC_DEALLOC_DDR_FIELD,
    &FPM_POOL1_ALLOC_DEALLOC_TOKEN_INDEX_FIELD,
    &FPM_POOL1_ALLOC_DEALLOC_TOKEN_SIZE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_POOL1_ALLOC_DEALLOC_REG = 
{
    "POOL1_ALLOC_DEALLOC",
#if RU_INCLUDE_DESC
    "POOL4 Allocation & De-allocation/Free Management Register",
    "The free pool FIFO contains pointers to the buffers in the pool. "
    "To allocate a buffer from the pool, read "
    "token from this port. To de-allocate/free a buffer to the pool , write the token "
    "of the buffer to this port. After reset, software must initialize the "
    "FIFO. The buffer size is given in the control register above. All "
    "buffers must be of the same size and contiguous. ",
#endif
    FPM_POOL1_ALLOC_DEALLOC_REG_OFFSET,
    0,
    0,
    213,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    FPM_POOL1_ALLOC_DEALLOC_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_POOL2_ALLOC_DEALLOC
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL2_ALLOC_DEALLOC_FIELDS[] =
{
    &FPM_POOL2_ALLOC_DEALLOC_TOKEN_VALID_FIELD,
    &FPM_POOL2_ALLOC_DEALLOC_RESERVED0_FIELD,
    &FPM_POOL2_ALLOC_DEALLOC_DDR_FIELD,
    &FPM_POOL2_ALLOC_DEALLOC_TOKEN_INDEX_FIELD,
    &FPM_POOL2_ALLOC_DEALLOC_TOKEN_SIZE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_POOL2_ALLOC_DEALLOC_REG = 
{
    "POOL2_ALLOC_DEALLOC",
#if RU_INCLUDE_DESC
    "POOL4 Allocation & De-allocation/Free Management Register",
    "The free pool FIFO contains pointers to the buffers in the pool. "
    "To allocate a buffer from the pool, read "
    "token from this port. To de-allocate/free a buffer to the pool , write the token "
    "of the buffer to this port. After reset, software must initialize the "
    "FIFO. The buffer size is given in the control register above. All "
    "buffers must be of the same size and contiguous. ",
#endif
    FPM_POOL2_ALLOC_DEALLOC_REG_OFFSET,
    0,
    0,
    214,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    FPM_POOL2_ALLOC_DEALLOC_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_POOL3_ALLOC_DEALLOC
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL3_ALLOC_DEALLOC_FIELDS[] =
{
    &FPM_POOL3_ALLOC_DEALLOC_TOKEN_VALID_FIELD,
    &FPM_POOL3_ALLOC_DEALLOC_RESERVED0_FIELD,
    &FPM_POOL3_ALLOC_DEALLOC_DDR_FIELD,
    &FPM_POOL3_ALLOC_DEALLOC_TOKEN_INDEX_FIELD,
    &FPM_POOL3_ALLOC_DEALLOC_TOKEN_SIZE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_POOL3_ALLOC_DEALLOC_REG = 
{
    "POOL3_ALLOC_DEALLOC",
#if RU_INCLUDE_DESC
    "POOL4 Allocation & De-allocation/Free Management Register",
    "The free pool FIFO contains pointers to the buffers in the pool. "
    "To allocate a buffer from the pool, read "
    "token from this port. To de-allocate/free a buffer to the pool , write the token "
    "of the buffer to this port. After reset, software must initialize the "
    "FIFO. The buffer size is given in the control register above. All "
    "buffers must be of the same size and contiguous. ",
#endif
    FPM_POOL3_ALLOC_DEALLOC_REG_OFFSET,
    0,
    0,
    215,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    FPM_POOL3_ALLOC_DEALLOC_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_POOL4_ALLOC_DEALLOC
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL4_ALLOC_DEALLOC_FIELDS[] =
{
    &FPM_POOL4_ALLOC_DEALLOC_TOKEN_VALID_FIELD,
    &FPM_POOL4_ALLOC_DEALLOC_RESERVED0_FIELD,
    &FPM_POOL4_ALLOC_DEALLOC_DDR_FIELD,
    &FPM_POOL4_ALLOC_DEALLOC_TOKEN_INDEX_FIELD,
    &FPM_POOL4_ALLOC_DEALLOC_TOKEN_SIZE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_POOL4_ALLOC_DEALLOC_REG = 
{
    "POOL4_ALLOC_DEALLOC",
#if RU_INCLUDE_DESC
    "POOL4 Allocation & De-allocation/Free Management Register",
    "The free pool FIFO contains pointers to the buffers in the pool. "
    "To allocate a buffer from the pool, read "
    "token from this port. To de-allocate/free a buffer to the pool , write the token "
    "of the buffer to this port. After reset, software must initialize the "
    "FIFO. The buffer size is given in the control register above. All "
    "buffers must be of the same size and contiguous. ",
#endif
    FPM_POOL4_ALLOC_DEALLOC_REG_OFFSET,
    0,
    0,
    216,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    FPM_POOL4_ALLOC_DEALLOC_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_SPARE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_SPARE_FIELDS[] =
{
    &FPM_SPARE_SPARE_BITS_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_SPARE_REG = 
{
    "SPARE",
#if RU_INCLUDE_DESC
    "Spare Register for future use",
    "",
#endif
    FPM_SPARE_REG_OFFSET,
    0,
    0,
    217,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_SPARE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_POOL_MULTI
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_POOL_MULTI_FIELDS[] =
{
    &FPM_POOL_MULTI_TOKEN_VALID_FIELD,
    &FPM_POOL_MULTI_RESERVED0_FIELD,
    &FPM_POOL_MULTI_DDR_FIELD,
    &FPM_POOL_MULTI_TOKEN_INDEX_FIELD,
    &FPM_POOL_MULTI_UPDATE_TYPE_FIELD,
    &FPM_POOL_MULTI_RESERVED1_FIELD,
    &FPM_POOL_MULTI_TOKEN_MULTI_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_POOL_MULTI_REG = 
{
    "POOL_MULTI",
#if RU_INCLUDE_DESC
    "Multi-cast Token Update Control Register",
    "Update/Modify the multi-cast value of the token",
#endif
    FPM_POOL_MULTI_REG_OFFSET,
    0,
    0,
    218,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    FPM_POOL_MULTI_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_FPM_BB_FORCE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_FPM_BB_FORCE_FIELDS[] =
{
    &FPM_FPM_BB_FORCE_FORCE_FIELD,
    &FPM_FPM_BB_FORCE_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_FPM_BB_FORCE_REG = 
{
    "FPM_BB_FORCE",
#if RU_INCLUDE_DESC
    "FPM_BB_FORCE Register",
    "Write this register to force FPM_BB transaction",
#endif
    FPM_FPM_BB_FORCE_REG_OFFSET,
    0,
    0,
    219,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_FPM_BB_FORCE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_FPM_BB_FORCED_CTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_FPM_BB_FORCED_CTRL_FIELDS[] =
{
    &FPM_FPM_BB_FORCED_CTRL_CTRL_FIELD,
    &FPM_FPM_BB_FORCED_CTRL_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_FPM_BB_FORCED_CTRL_REG = 
{
    "FPM_BB_FORCED_CTRL",
#if RU_INCLUDE_DESC
    "FPM_BB_FORCED_CTRL Register",
    "Control to be sent on forced transaction",
#endif
    FPM_FPM_BB_FORCED_CTRL_REG_OFFSET,
    0,
    0,
    220,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    FPM_FPM_BB_FORCED_CTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_FPM_BB_FORCED_ADDR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_FPM_BB_FORCED_ADDR_FIELDS[] =
{
    &FPM_FPM_BB_FORCED_ADDR_TA_ADDR_FIELD,
    &FPM_FPM_BB_FORCED_ADDR_DEST_ADDR_FIELD,
    &FPM_FPM_BB_FORCED_ADDR_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_FPM_BB_FORCED_ADDR_REG = 
{
    "FPM_BB_FORCED_ADDR",
#if RU_INCLUDE_DESC
    "FPM_BB_FORCED_ADDR Register",
    "Address to be sent on forced transaction",
#endif
    FPM_FPM_BB_FORCED_ADDR_REG_OFFSET,
    0,
    0,
    221,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    FPM_FPM_BB_FORCED_ADDR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_FPM_BB_FORCED_DATA
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_FPM_BB_FORCED_DATA_FIELDS[] =
{
    &FPM_FPM_BB_FORCED_DATA_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_FPM_BB_FORCED_DATA_REG = 
{
    "FPM_BB_FORCED_DATA",
#if RU_INCLUDE_DESC
    "FPM_BB_FORCED_DATA Register",
    "Data to be sent on forced transaction",
#endif
    FPM_FPM_BB_FORCED_DATA_REG_OFFSET,
    0,
    0,
    222,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_FPM_BB_FORCED_DATA_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_FPM_BB_DECODE_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_FPM_BB_DECODE_CFG_FIELDS[] =
{
    &FPM_FPM_BB_DECODE_CFG_DEST_ID_FIELD,
    &FPM_FPM_BB_DECODE_CFG_OVERRIDE_EN_FIELD,
    &FPM_FPM_BB_DECODE_CFG_ROUTE_ADDR_FIELD,
    &FPM_FPM_BB_DECODE_CFG_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_FPM_BB_DECODE_CFG_REG = 
{
    "FPM_BB_DECODE_CFG",
#if RU_INCLUDE_DESC
    "FPM_BB_DECODE_CFG Register",
    "set configuration for BB decoder",
#endif
    FPM_FPM_BB_DECODE_CFG_REG_OFFSET,
    0,
    0,
    223,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    FPM_FPM_BB_DECODE_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_FPM_BB_DBG_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_FPM_BB_DBG_CFG_FIELDS[] =
{
    &FPM_FPM_BB_DBG_CFG_RXFIFO_SW_ADDR_FIELD,
    &FPM_FPM_BB_DBG_CFG_TXFIFO_SW_ADDR_FIELD,
    &FPM_FPM_BB_DBG_CFG_RXFIFO_SW_RST_FIELD,
    &FPM_FPM_BB_DBG_CFG_TXFIFO_SW_RST_FIELD,
    &FPM_FPM_BB_DBG_CFG_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_FPM_BB_DBG_CFG_REG = 
{
    "FPM_BB_DBG_CFG",
#if RU_INCLUDE_DESC
    "FPM_BB_DBG_CFG Register",
    "Set SW addr to read FPM_BB FIFOs",
#endif
    FPM_FPM_BB_DBG_CFG_REG_OFFSET,
    0,
    0,
    224,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    FPM_FPM_BB_DBG_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_FPM_BB_DBG_RXFIFO_STS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_FPM_BB_DBG_RXFIFO_STS_FIELDS[] =
{
    &FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_EMPTY_FIELD,
    &FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_FULL_FIELD,
    &FPM_FPM_BB_DBG_RXFIFO_STS_RESERVED0_FIELD,
    &FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_USED_WORDS_FIELD,
    &FPM_FPM_BB_DBG_RXFIFO_STS_RESERVED1_FIELD,
    &FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_RD_CNTR_FIELD,
    &FPM_FPM_BB_DBG_RXFIFO_STS_RESERVED2_FIELD,
    &FPM_FPM_BB_DBG_RXFIFO_STS_FIFO_WR_CNTR_FIELD,
    &FPM_FPM_BB_DBG_RXFIFO_STS_RESERVED3_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_FPM_BB_DBG_RXFIFO_STS_REG = 
{
    "FPM_BB_DBG_RXFIFO_STS",
#if RU_INCLUDE_DESC
    "FPM_BB_DBG_RXFIFO_STS Register",
    "Status of FPM BB RXFIFO",
#endif
    FPM_FPM_BB_DBG_RXFIFO_STS_REG_OFFSET,
    0,
    0,
    225,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    FPM_FPM_BB_DBG_RXFIFO_STS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_FPM_BB_DBG_TXFIFO_STS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_FPM_BB_DBG_TXFIFO_STS_FIELDS[] =
{
    &FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_EMPTY_FIELD,
    &FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_FULL_FIELD,
    &FPM_FPM_BB_DBG_TXFIFO_STS_RESERVED0_FIELD,
    &FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_USED_WORDS_FIELD,
    &FPM_FPM_BB_DBG_TXFIFO_STS_RESERVED1_FIELD,
    &FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_RD_CNTR_FIELD,
    &FPM_FPM_BB_DBG_TXFIFO_STS_RESERVED2_FIELD,
    &FPM_FPM_BB_DBG_TXFIFO_STS_FIFO_WR_CNTR_FIELD,
    &FPM_FPM_BB_DBG_TXFIFO_STS_RESERVED3_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_FPM_BB_DBG_TXFIFO_STS_REG = 
{
    "FPM_BB_DBG_TXFIFO_STS",
#if RU_INCLUDE_DESC
    "FPM_BB_DBG_TXFIFO_STS Register",
    "Status of FPM BB TXFIFO",
#endif
    FPM_FPM_BB_DBG_TXFIFO_STS_REG_OFFSET,
    0,
    0,
    226,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    FPM_FPM_BB_DBG_TXFIFO_STS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_FPM_BB_DBG_RXFIFO_DATA1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_FPM_BB_DBG_RXFIFO_DATA1_FIELDS[] =
{
    &FPM_FPM_BB_DBG_RXFIFO_DATA1_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_FPM_BB_DBG_RXFIFO_DATA1_REG = 
{
    "FPM_BB_DBG_RXFIFO_DATA1",
#if RU_INCLUDE_DESC
    "FPM_BB_DBG_RXFIFO_DATA1 Register",
    "Data from FPM BB RXFIFO bits [31:0]",
#endif
    FPM_FPM_BB_DBG_RXFIFO_DATA1_REG_OFFSET,
    0,
    0,
    227,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_FPM_BB_DBG_RXFIFO_DATA1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_FPM_BB_DBG_RXFIFO_DATA2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_FPM_BB_DBG_RXFIFO_DATA2_FIELDS[] =
{
    &FPM_FPM_BB_DBG_RXFIFO_DATA2_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_FPM_BB_DBG_RXFIFO_DATA2_REG = 
{
    "FPM_BB_DBG_RXFIFO_DATA2",
#if RU_INCLUDE_DESC
    "FPM_BB_DBG_RXFIFO_DATA2 Register",
    "Data from FPM BB RXFIFO bits [39:32]",
#endif
    FPM_FPM_BB_DBG_RXFIFO_DATA2_REG_OFFSET,
    0,
    0,
    228,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_FPM_BB_DBG_RXFIFO_DATA2_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_FPM_BB_DBG_TXFIFO_DATA1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_FPM_BB_DBG_TXFIFO_DATA1_FIELDS[] =
{
    &FPM_FPM_BB_DBG_TXFIFO_DATA1_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_FPM_BB_DBG_TXFIFO_DATA1_REG = 
{
    "FPM_BB_DBG_TXFIFO_DATA1",
#if RU_INCLUDE_DESC
    "FPM_BB_DBG_TXFIFO_DATA1 Register",
    "Data from FPM BB TXFIFO bits [31:0]",
#endif
    FPM_FPM_BB_DBG_TXFIFO_DATA1_REG_OFFSET,
    0,
    0,
    229,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_FPM_BB_DBG_TXFIFO_DATA1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_FPM_BB_DBG_TXFIFO_DATA2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_FPM_BB_DBG_TXFIFO_DATA2_FIELDS[] =
{
    &FPM_FPM_BB_DBG_TXFIFO_DATA2_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_FPM_BB_DBG_TXFIFO_DATA2_REG = 
{
    "FPM_BB_DBG_TXFIFO_DATA2",
#if RU_INCLUDE_DESC
    "FPM_BB_DBG_TXFIFO_DATA2 Register",
    "Data from FPM BB TXFIFO bits [63:32]",
#endif
    FPM_FPM_BB_DBG_TXFIFO_DATA2_REG_OFFSET,
    0,
    0,
    230,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_FPM_BB_DBG_TXFIFO_DATA2_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: FPM_FPM_BB_DBG_TXFIFO_DATA3
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FPM_FPM_BB_DBG_TXFIFO_DATA3_FIELDS[] =
{
    &FPM_FPM_BB_DBG_TXFIFO_DATA3_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FPM_FPM_BB_DBG_TXFIFO_DATA3_REG = 
{
    "FPM_BB_DBG_TXFIFO_DATA3",
#if RU_INCLUDE_DESC
    "FPM_BB_DBG_TXFIFO_DATA3 Register",
    "Data from FPM BB TXFIFO bits [79:64]",
#endif
    FPM_FPM_BB_DBG_TXFIFO_DATA3_REG_OFFSET,
    0,
    0,
    231,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    FPM_FPM_BB_DBG_TXFIFO_DATA3_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Block: FPM
 ******************************************************************************/
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
    &FPM_POOL1_CFG1_REG,
    &FPM_POOL1_CFG2_REG,
    &FPM_POOL1_CFG3_REG,
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
    &FPM_POOL1_XON_XOFF_CFG_REG,
    &FPM_FPM_NOT_EMPTY_CFG_REG,
    &FPM_MEM_CTL_REG,
    &FPM_MEM_DATA1_REG,
    &FPM_MEM_DATA2_REG,
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
    &FPM_POOL1_ALLOC_DEALLOC_REG,
    &FPM_POOL2_ALLOC_DEALLOC_REG,
    &FPM_POOL3_ALLOC_DEALLOC_REG,
    &FPM_POOL4_ALLOC_DEALLOC_REG,
    &FPM_SPARE_REG,
    &FPM_POOL_MULTI_REG,
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
};

unsigned long FPM_ADDRS[] =
{
    0x82a00000,
};

const ru_block_rec FPM_BLOCK = 
{
    "FPM",
    FPM_ADDRS,
    1,
    63,
    FPM_REGS
};

/* End of file XRDP_FPM.c */
