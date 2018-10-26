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
 * Register: HASH_GENERAL_CONFIGURATION_PWR_SAV_EN
 ******************************************************************************/
const ru_reg_rec HASH_GENERAL_CONFIGURATION_PWR_SAV_EN_REG = 
{
    "GENERAL_CONFIGURATION_PWR_SAV_EN",
#if RU_INCLUDE_DESC
    "PWR_SAV_EN Register",
    "Power saving mode -"
    "detect that the accelerator has no activity and enter to power saving mode",
#endif
    HASH_GENERAL_CONFIGURATION_PWR_SAV_EN_REG_OFFSET,
    0,
    0,
    705,
};

/******************************************************************************
 * Register: HASH_GENERAL_CONFIGURATION_PAD_HIGH
 ******************************************************************************/
const ru_reg_rec HASH_GENERAL_CONFIGURATION_PAD_HIGH_REG = 
{
    "GENERAL_CONFIGURATION_PAD_HIGH",
#if RU_INCLUDE_DESC
    "PAD_VAL_HIGH Register",
    "Determines the padding value added to keys according to the selected MASK",
#endif
    HASH_GENERAL_CONFIGURATION_PAD_HIGH_REG_OFFSET,
    0,
    0,
    706,
};

/******************************************************************************
 * Register: HASH_GENERAL_CONFIGURATION_PAD_LOW
 ******************************************************************************/
const ru_reg_rec HASH_GENERAL_CONFIGURATION_PAD_LOW_REG = 
{
    "GENERAL_CONFIGURATION_PAD_LOW",
#if RU_INCLUDE_DESC
    "PAD_VAL_LOW Register",
    "Determines the padding value added to keys according to the selected MASK",
#endif
    HASH_GENERAL_CONFIGURATION_PAD_LOW_REG_OFFSET,
    0,
    0,
    707,
};

/******************************************************************************
 * Register: HASH_GENERAL_CONFIGURATION_MULT_HIT_ERR
 ******************************************************************************/
const ru_reg_rec HASH_GENERAL_CONFIGURATION_MULT_HIT_ERR_REG = 
{
    "GENERAL_CONFIGURATION_MULT_HIT_ERR",
#if RU_INCLUDE_DESC
    "MULT_HIT_ERR Register",
    "In case of multiple match this reg captures the hit indication per engine. This is a read clear reg.",
#endif
    HASH_GENERAL_CONFIGURATION_MULT_HIT_ERR_REG_OFFSET,
    0,
    0,
    708,
};

/******************************************************************************
 * Register: HASH_GENERAL_CONFIGURATION_UNDO_FIX
 ******************************************************************************/
const ru_reg_rec HASH_GENERAL_CONFIGURATION_UNDO_FIX_REG = 
{
    "GENERAL_CONFIGURATION_UNDO_FIX",
#if RU_INCLUDE_DESC
    "UNDO_FIX Register",
    "Consist of chicken bit per specific fix",
#endif
    HASH_GENERAL_CONFIGURATION_UNDO_FIX_REG_OFFSET,
    0,
    0,
    709,
};

/******************************************************************************
 * Register: HASH_PM_COUNTERS_HITS
 ******************************************************************************/
const ru_reg_rec HASH_PM_COUNTERS_HITS_REG = 
{
    "PM_COUNTERS_HITS",
#if RU_INCLUDE_DESC
    "HITS Register",
    "Number of key hits"
    ""
    "This reg is frozen when freeze bit asserted.",
#endif
    HASH_PM_COUNTERS_HITS_REG_OFFSET,
    0,
    0,
    710,
};

/******************************************************************************
 * Register: HASH_PM_COUNTERS_SRCHS
 ******************************************************************************/
const ru_reg_rec HASH_PM_COUNTERS_SRCHS_REG = 
{
    "PM_COUNTERS_SRCHS",
#if RU_INCLUDE_DESC
    "SEARCHES Register",
    "Number of key searches"
    ""
    "This register is updated only when freeze register is not set",
#endif
    HASH_PM_COUNTERS_SRCHS_REG_OFFSET,
    0,
    0,
    711,
};

/******************************************************************************
 * Register: HASH_PM_COUNTERS_MISS
 ******************************************************************************/
const ru_reg_rec HASH_PM_COUNTERS_MISS_REG = 
{
    "PM_COUNTERS_MISS",
#if RU_INCLUDE_DESC
    "MISSES Register",
    "Total NUM of misses"
    "read clear register"
    "updated only when freeze reg is 0",
#endif
    HASH_PM_COUNTERS_MISS_REG_OFFSET,
    0,
    0,
    712,
};

/******************************************************************************
 * Register: HASH_PM_COUNTERS_HIT_1ST_ACS
 ******************************************************************************/
const ru_reg_rec HASH_PM_COUNTERS_HIT_1ST_ACS_REG = 
{
    "PM_COUNTERS_HIT_1ST_ACS",
#if RU_INCLUDE_DESC
    "HIT_1ST_ACS Register",
    "Total NUM of misses"
    "read clear register"
    "updated only when freeze reg is 0",
#endif
    HASH_PM_COUNTERS_HIT_1ST_ACS_REG_OFFSET,
    0,
    0,
    713,
};

/******************************************************************************
 * Register: HASH_PM_COUNTERS_HIT_2ND_ACS
 ******************************************************************************/
const ru_reg_rec HASH_PM_COUNTERS_HIT_2ND_ACS_REG = 
{
    "PM_COUNTERS_HIT_2ND_ACS",
#if RU_INCLUDE_DESC
    "HIT_2ND_ACS Register",
    "Total NUM of misses"
    "read clear register"
    "updated only when freeze reg is 0",
#endif
    HASH_PM_COUNTERS_HIT_2ND_ACS_REG_OFFSET,
    0,
    0,
    714,
};

/******************************************************************************
 * Register: HASH_PM_COUNTERS_HIT_3RD_ACS
 ******************************************************************************/
const ru_reg_rec HASH_PM_COUNTERS_HIT_3RD_ACS_REG = 
{
    "PM_COUNTERS_HIT_3RD_ACS",
#if RU_INCLUDE_DESC
    "HIT_3RD_ACS Register",
    "Total NUM of misses"
    "read clear register"
    "updated only when freeze reg is 0",
#endif
    HASH_PM_COUNTERS_HIT_3RD_ACS_REG_OFFSET,
    0,
    0,
    715,
};

/******************************************************************************
 * Register: HASH_PM_COUNTERS_HIT_4TH_ACS
 ******************************************************************************/
const ru_reg_rec HASH_PM_COUNTERS_HIT_4TH_ACS_REG = 
{
    "PM_COUNTERS_HIT_4TH_ACS",
#if RU_INCLUDE_DESC
    "HIT_4TH_ACS Register",
    "Total NUM of misses"
    "read clear register"
    "updated only when freeze reg is 0",
#endif
    HASH_PM_COUNTERS_HIT_4TH_ACS_REG_OFFSET,
    0,
    0,
    716,
};

/******************************************************************************
 * Register: HASH_PM_COUNTERS_FRZ_CNT
 ******************************************************************************/
const ru_reg_rec HASH_PM_COUNTERS_FRZ_CNT_REG = 
{
    "PM_COUNTERS_FRZ_CNT",
#if RU_INCLUDE_DESC
    "FREEZE_CNT Register",
    "Freezes counters update - used to read all relevant counters at the same point."
    "",
#endif
    HASH_PM_COUNTERS_FRZ_CNT_REG_OFFSET,
    0,
    0,
    717,
};

/******************************************************************************
 * Register: HASH_LKUP_TBL_CFG_TBL_CFG
 ******************************************************************************/
const ru_reg_rec HASH_LKUP_TBL_CFG_TBL_CFG_REG = 
{
    "LKUP_TBL_CFG_TBL_CFG",
#if RU_INCLUDE_DESC
    "TBL_CFG Register",
    "Look-up table :  Configuration of LUT: table params + main flag",
#endif
    HASH_LKUP_TBL_CFG_TBL_CFG_REG_OFFSET,
    HASH_LKUP_TBL_CFG_TBL_CFG_REG_RAM_CNT,
    16,
    718,
};

/******************************************************************************
 * Register: HASH_LKUP_TBL_CFG_KEY_MASK_HIGH
 ******************************************************************************/
const ru_reg_rec HASH_LKUP_TBL_CFG_KEY_MASK_HIGH_REG = 
{
    "LKUP_TBL_CFG_KEY_MASK_HIGH",
#if RU_INCLUDE_DESC
    "KEY_MASK_HIGH Register",
    "Look-up table : key Mask on bits [59:32]"
    "Key consist of 60-bit."
    "by configuring mask the user can use different key lengths."
    "if the key is smaller than 60 bit it is padded with constant value according the the mask register.",
#endif
    HASH_LKUP_TBL_CFG_KEY_MASK_HIGH_REG_OFFSET,
    HASH_LKUP_TBL_CFG_KEY_MASK_HIGH_REG_RAM_CNT,
    16,
    719,
};

/******************************************************************************
 * Register: HASH_LKUP_TBL_CFG_KEY_MASK_LOW
 ******************************************************************************/
const ru_reg_rec HASH_LKUP_TBL_CFG_KEY_MASK_LOW_REG = 
{
    "LKUP_TBL_CFG_KEY_MASK_LOW",
#if RU_INCLUDE_DESC
    "KEY_MASK_LOW Register",
    "Look-up table key Mask on bits [31:0]"
    "Key consist of 60-bit."
    ""
    "By configuring mask the user can use different key lengths."
    ""
    "If the key is smaller than 60 bit it is padded with constant value according to the mask register.",
#endif
    HASH_LKUP_TBL_CFG_KEY_MASK_LOW_REG_OFFSET,
    HASH_LKUP_TBL_CFG_KEY_MASK_LOW_REG_RAM_CNT,
    16,
    720,
};

/******************************************************************************
 * Register: HASH_LKUP_TBL_CFG_CNTXT_CFG
 ******************************************************************************/
const ru_reg_rec HASH_LKUP_TBL_CFG_CNTXT_CFG_REG = 
{
    "LKUP_TBL_CFG_CNTXT_CFG",
#if RU_INCLUDE_DESC
    "CNTXT_CFG Register",
    "Look-up table: LUT Context Table configurations (base addr + entry context size)",
#endif
    HASH_LKUP_TBL_CFG_CNTXT_CFG_REG_OFFSET,
    HASH_LKUP_TBL_CFG_CNTXT_CFG_REG_RAM_CNT,
    16,
    721,
};

/******************************************************************************
 * Register: HASH_CAM_CONFIGURATION_CNTXT_CFG
 ******************************************************************************/
const ru_reg_rec HASH_CAM_CONFIGURATION_CNTXT_CFG_REG = 
{
    "CAM_CONFIGURATION_CNTXT_CFG",
#if RU_INCLUDE_DESC
    "CNTXT_CFG Register",
    "Look-up table: LUT Context Table configurations (base addr + entry context size)",
#endif
    HASH_CAM_CONFIGURATION_CNTXT_CFG_REG_OFFSET,
    0,
    0,
    722,
};

/******************************************************************************
 * Register: HASH_CAM_INDIRECT_OP
 ******************************************************************************/
const ru_reg_rec HASH_CAM_INDIRECT_OP_REG = 
{
    "CAM_INDIRECT_OP",
#if RU_INCLUDE_DESC
    "OPERATION Register",
    "TCAM Operation:"
    "0 - CAM READ"
    "1 - CAM Write"
    "2 - CAM Compare"
    "3 - CAM valid bit reset"
    "Writing to this register triggers the operation. All other relevant register should be ready before SW writes to this register.",
#endif
    HASH_CAM_INDIRECT_OP_REG_OFFSET,
    0,
    0,
    723,
};

/******************************************************************************
 * Register: HASH_CAM_INDIRECT_OP_DONE
 ******************************************************************************/
const ru_reg_rec HASH_CAM_INDIRECT_OP_DONE_REG = 
{
    "CAM_INDIRECT_OP_DONE",
#if RU_INCLUDE_DESC
    "OPERATION_DONE Register",
    "Raised when the CAM operation is completed (cleared by HW on writing to the OPERATION register)",
#endif
    HASH_CAM_INDIRECT_OP_DONE_REG_OFFSET,
    0,
    0,
    724,
};

/******************************************************************************
 * Register: HASH_CAM_INDIRECT_ADDR
 ******************************************************************************/
const ru_reg_rec HASH_CAM_INDIRECT_ADDR_REG = 
{
    "CAM_INDIRECT_ADDR",
#if RU_INCLUDE_DESC
    "ADDRESS Register",
    "Key Address to be used in RD/WR opoerations.",
#endif
    HASH_CAM_INDIRECT_ADDR_REG_OFFSET,
    0,
    0,
    725,
};

/******************************************************************************
 * Register: HASH_CAM_INDIRECT_VLID_IN
 ******************************************************************************/
const ru_reg_rec HASH_CAM_INDIRECT_VLID_IN_REG = 
{
    "CAM_INDIRECT_VLID_IN",
#if RU_INCLUDE_DESC
    "VALID_IN Register",
    "Valid value to be written - this value is relevant during write operation on key0.",
#endif
    HASH_CAM_INDIRECT_VLID_IN_REG_OFFSET,
    0,
    0,
    726,
};

/******************************************************************************
 * Register: HASH_CAM_INDIRECT_VLID_OUT
 ******************************************************************************/
const ru_reg_rec HASH_CAM_INDIRECT_VLID_OUT_REG = 
{
    "CAM_INDIRECT_VLID_OUT",
#if RU_INCLUDE_DESC
    "VALID_OUT Register",
    "Valid value read from the CAM - this value is relevant during read operation on key0.",
#endif
    HASH_CAM_INDIRECT_VLID_OUT_REG_OFFSET,
    0,
    0,
    727,
};

/******************************************************************************
 * Register: HASH_CAM_INDIRECT_RSLT
 ******************************************************************************/
const ru_reg_rec HASH_CAM_INDIRECT_RSLT_REG = 
{
    "CAM_INDIRECT_RSLT",
#if RU_INCLUDE_DESC
    "SEARCH_RESULT Register",
    "The result of a search operation",
#endif
    HASH_CAM_INDIRECT_RSLT_REG_OFFSET,
    0,
    0,
    728,
};

/******************************************************************************
 * Register: HASH_CAM_INDIRECT_KEY_IN
 ******************************************************************************/
const ru_reg_rec HASH_CAM_INDIRECT_KEY_IN_REG = 
{
    "CAM_INDIRECT_KEY_IN",
#if RU_INCLUDE_DESC
    "KEY_IN %i Register",
    "Key to be used in Write/Compare operations."
    "The Key is 64bit long and is represented by 2 registers. The lower address register corresponds to the most significant bits of the key.",
#endif
    HASH_CAM_INDIRECT_KEY_IN_REG_OFFSET,
    HASH_CAM_INDIRECT_KEY_IN_REG_RAM_CNT,
    4,
    729,
};

/******************************************************************************
 * Register: HASH_CAM_INDIRECT_KEY_OUT
 ******************************************************************************/
const ru_reg_rec HASH_CAM_INDIRECT_KEY_OUT_REG = 
{
    "CAM_INDIRECT_KEY_OUT",
#if RU_INCLUDE_DESC
    "KEY_OUT %i Register",
    "Key returned from the CAM in a read operation. The Key is 64bit long and is represented by 2 registers. The lower address register correspond to the most significant bits of the key.",
#endif
    HASH_CAM_INDIRECT_KEY_OUT_REG_OFFSET,
    HASH_CAM_INDIRECT_KEY_OUT_REG_RAM_CNT,
    4,
    730,
};

/******************************************************************************
 * Register: HASH_CAM_BIST_BIST_STATUS
 ******************************************************************************/
const ru_reg_rec HASH_CAM_BIST_BIST_STATUS_REG = 
{
    "CAM_BIST_BIST_STATUS",
#if RU_INCLUDE_DESC
    "BIST_STATUS Register",
    ".",
#endif
    HASH_CAM_BIST_BIST_STATUS_REG_OFFSET,
    0,
    0,
    731,
};

/******************************************************************************
 * Register: HASH_CAM_BIST_BIST_DBG_COMPARE_EN
 ******************************************************************************/
const ru_reg_rec HASH_CAM_BIST_BIST_DBG_COMPARE_EN_REG = 
{
    "CAM_BIST_BIST_DBG_COMPARE_EN",
#if RU_INCLUDE_DESC
    "BIST_DBG_COMPARE_EN Register",
    ".",
#endif
    HASH_CAM_BIST_BIST_DBG_COMPARE_EN_REG_OFFSET,
    0,
    0,
    732,
};

/******************************************************************************
 * Register: HASH_CAM_BIST_BIST_DBG_DATA
 ******************************************************************************/
const ru_reg_rec HASH_CAM_BIST_BIST_DBG_DATA_REG = 
{
    "CAM_BIST_BIST_DBG_DATA",
#if RU_INCLUDE_DESC
    "BIST_DBG_DATA Register",
    ".",
#endif
    HASH_CAM_BIST_BIST_DBG_DATA_REG_OFFSET,
    0,
    0,
    733,
};

/******************************************************************************
 * Register: HASH_CAM_BIST_BIST_DBG_DATA_SLICE_OR_STATUS_SEL
 ******************************************************************************/
const ru_reg_rec HASH_CAM_BIST_BIST_DBG_DATA_SLICE_OR_STATUS_SEL_REG = 
{
    "CAM_BIST_BIST_DBG_DATA_SLICE_OR_STATUS_SEL",
#if RU_INCLUDE_DESC
    "BIST_DBG_DATA_SLICE_OR_STATUS_SEL Register",
    ".",
#endif
    HASH_CAM_BIST_BIST_DBG_DATA_SLICE_OR_STATUS_SEL_REG_OFFSET,
    0,
    0,
    734,
};

/******************************************************************************
 * Register: HASH_CAM_BIST_BIST_DBG_DATA_VALID
 ******************************************************************************/
const ru_reg_rec HASH_CAM_BIST_BIST_DBG_DATA_VALID_REG = 
{
    "CAM_BIST_BIST_DBG_DATA_VALID",
#if RU_INCLUDE_DESC
    "BIST_DBG_DATA_VALID Register",
    ".",
#endif
    HASH_CAM_BIST_BIST_DBG_DATA_VALID_REG_OFFSET,
    0,
    0,
    735,
};

/******************************************************************************
 * Register: HASH_CAM_BIST_BIST_EN
 ******************************************************************************/
const ru_reg_rec HASH_CAM_BIST_BIST_EN_REG = 
{
    "CAM_BIST_BIST_EN",
#if RU_INCLUDE_DESC
    "BIST_EN Register",
    ".",
#endif
    HASH_CAM_BIST_BIST_EN_REG_OFFSET,
    0,
    0,
    736,
};

/******************************************************************************
 * Register: HASH_CAM_BIST_BIST_MODE
 ******************************************************************************/
const ru_reg_rec HASH_CAM_BIST_BIST_MODE_REG = 
{
    "CAM_BIST_BIST_MODE",
#if RU_INCLUDE_DESC
    "BIST_MODE Register",
    ".",
#endif
    HASH_CAM_BIST_BIST_MODE_REG_OFFSET,
    0,
    0,
    737,
};

/******************************************************************************
 * Register: HASH_CAM_BIST_BIST_RST_L
 ******************************************************************************/
const ru_reg_rec HASH_CAM_BIST_BIST_RST_L_REG = 
{
    "CAM_BIST_BIST_RST_L",
#if RU_INCLUDE_DESC
    "BIST_RST_L Register",
    ".",
#endif
    HASH_CAM_BIST_BIST_RST_L_REG_OFFSET,
    0,
    0,
    738,
};

/******************************************************************************
 * Register: HASH_CAM_BIST_BIST_SKIP_ERROR_CNT
 ******************************************************************************/
const ru_reg_rec HASH_CAM_BIST_BIST_SKIP_ERROR_CNT_REG = 
{
    "CAM_BIST_BIST_SKIP_ERROR_CNT",
#if RU_INCLUDE_DESC
    "BIST_SKIP_ERROR_CNT Register",
    ".",
#endif
    HASH_CAM_BIST_BIST_SKIP_ERROR_CNT_REG_OFFSET,
    0,
    0,
    739,
};

/******************************************************************************
 * Register: HASH_CAM_BIST_DBG_EN
 ******************************************************************************/
const ru_reg_rec HASH_CAM_BIST_DBG_EN_REG = 
{
    "CAM_BIST_DBG_EN",
#if RU_INCLUDE_DESC
    "DBG_EN Register",
    ".",
#endif
    HASH_CAM_BIST_DBG_EN_REG_OFFSET,
    0,
    0,
    740,
};

/******************************************************************************
 * Register: HASH_CAM_BIST_BIST_CASCADE_SELECT
 ******************************************************************************/
const ru_reg_rec HASH_CAM_BIST_BIST_CASCADE_SELECT_REG = 
{
    "CAM_BIST_BIST_CASCADE_SELECT",
#if RU_INCLUDE_DESC
    "BIST_CASCADE_SELECT Register",
    ".",
#endif
    HASH_CAM_BIST_BIST_CASCADE_SELECT_REG_OFFSET,
    0,
    0,
    741,
};

/******************************************************************************
 * Register: HASH_CAM_BIST_BIST_BLOCK_SELECT
 ******************************************************************************/
const ru_reg_rec HASH_CAM_BIST_BIST_BLOCK_SELECT_REG = 
{
    "CAM_BIST_BIST_BLOCK_SELECT",
#if RU_INCLUDE_DESC
    "BIST_BLOCK_SELECT Register",
    ".",
#endif
    HASH_CAM_BIST_BIST_BLOCK_SELECT_REG_OFFSET,
    0,
    0,
    742,
};

/******************************************************************************
 * Register: HASH_CAM_BIST_BIST_REPAIR_ENABLE
 ******************************************************************************/
const ru_reg_rec HASH_CAM_BIST_BIST_REPAIR_ENABLE_REG = 
{
    "CAM_BIST_BIST_REPAIR_ENABLE",
#if RU_INCLUDE_DESC
    "BIST_REPAIR_ENABLE Register",
    ".",
#endif
    HASH_CAM_BIST_BIST_REPAIR_ENABLE_REG_OFFSET,
    0,
    0,
    743,
};

/******************************************************************************
 * Register: HASH_INTR_CTRL_ISR
 ******************************************************************************/
const ru_reg_rec HASH_INTR_CTRL_ISR_REG = 
{
    "INTR_CTRL_ISR",
#if RU_INCLUDE_DESC
    "INTERRUPT_STATUS_REGISTER Register",
    "This register contains the current active hash interrupts. Each asserted bit represents an active interrupt source. The interrupt remains active until the software clears it by writing 1 to the corresponding bit.",
#endif
    HASH_INTR_CTRL_ISR_REG_OFFSET,
    0,
    0,
    744,
};

/******************************************************************************
 * Register: HASH_INTR_CTRL_ISM
 ******************************************************************************/
const ru_reg_rec HASH_INTR_CTRL_ISM_REG = 
{
    "INTR_CTRL_ISM",
#if RU_INCLUDE_DESC
    "INTERRUPT_STATUS_MASKED_REGISTER Register",
    "This register provides only the  enabled interrupts for each of the interrupt sources depicted in the ISR register.",
#endif
    HASH_INTR_CTRL_ISM_REG_OFFSET,
    0,
    0,
    745,
};

/******************************************************************************
 * Register: HASH_INTR_CTRL_IER
 ******************************************************************************/
const ru_reg_rec HASH_INTR_CTRL_IER_REG = 
{
    "INTR_CTRL_IER",
#if RU_INCLUDE_DESC
    "INTERRUPT_ENABLE_REGISTER Register",
    "This register provides an enable mask for each of the interrupt sources depicted in the ISR register.",
#endif
    HASH_INTR_CTRL_IER_REG_OFFSET,
    0,
    0,
    746,
};

/******************************************************************************
 * Register: HASH_INTR_CTRL_ITR
 ******************************************************************************/
const ru_reg_rec HASH_INTR_CTRL_ITR_REG = 
{
    "INTR_CTRL_ITR",
#if RU_INCLUDE_DESC
    "INTERRUPT_TEST_REGISTER Register",
    "This register enables testing by simulating interrupt sources. When the software sets a bit in the ITR, the corresponding bit in the ISR shows an active interrupt. The interrupt remains active until software clears the bit in the ITR",
#endif
    HASH_INTR_CTRL_ITR_REG_OFFSET,
    0,
    0,
    747,
};

/******************************************************************************
 * Register: HASH_DEBUG_DBG0
 ******************************************************************************/
const ru_reg_rec HASH_DEBUG_DBG0_REG = 
{
    "DEBUG_DBG0",
#if RU_INCLUDE_DESC
    "DBG0 Register",
    "debug reg",
#endif
    HASH_DEBUG_DBG0_REG_OFFSET,
    0,
    0,
    748,
};

/******************************************************************************
 * Register: HASH_DEBUG_DBG1
 ******************************************************************************/
const ru_reg_rec HASH_DEBUG_DBG1_REG = 
{
    "DEBUG_DBG1",
#if RU_INCLUDE_DESC
    "DBG1 Register",
    "debug reg",
#endif
    HASH_DEBUG_DBG1_REG_OFFSET,
    0,
    0,
    749,
};

/******************************************************************************
 * Register: HASH_DEBUG_DBG2
 ******************************************************************************/
const ru_reg_rec HASH_DEBUG_DBG2_REG = 
{
    "DEBUG_DBG2",
#if RU_INCLUDE_DESC
    "DBG2 Register",
    "debug reg",
#endif
    HASH_DEBUG_DBG2_REG_OFFSET,
    0,
    0,
    750,
};

/******************************************************************************
 * Register: HASH_DEBUG_DBG3
 ******************************************************************************/
const ru_reg_rec HASH_DEBUG_DBG3_REG = 
{
    "DEBUG_DBG3",
#if RU_INCLUDE_DESC
    "DBG3 Register",
    "debug reg",
#endif
    HASH_DEBUG_DBG3_REG_OFFSET,
    0,
    0,
    751,
};

/******************************************************************************
 * Register: HASH_DEBUG_DBG4
 ******************************************************************************/
const ru_reg_rec HASH_DEBUG_DBG4_REG = 
{
    "DEBUG_DBG4",
#if RU_INCLUDE_DESC
    "DBG4 Register",
    "debug reg",
#endif
    HASH_DEBUG_DBG4_REG_OFFSET,
    0,
    0,
    752,
};

/******************************************************************************
 * Register: HASH_DEBUG_DBG5
 ******************************************************************************/
const ru_reg_rec HASH_DEBUG_DBG5_REG = 
{
    "DEBUG_DBG5",
#if RU_INCLUDE_DESC
    "DBG5 Register",
    "debug reg",
#endif
    HASH_DEBUG_DBG5_REG_OFFSET,
    0,
    0,
    753,
};

/******************************************************************************
 * Register: HASH_DEBUG_DBG6
 ******************************************************************************/
const ru_reg_rec HASH_DEBUG_DBG6_REG = 
{
    "DEBUG_DBG6",
#if RU_INCLUDE_DESC
    "DBG6 Register",
    "debug reg",
#endif
    HASH_DEBUG_DBG6_REG_OFFSET,
    0,
    0,
    754,
};

/******************************************************************************
 * Register: HASH_DEBUG_DBG7
 ******************************************************************************/
const ru_reg_rec HASH_DEBUG_DBG7_REG = 
{
    "DEBUG_DBG7",
#if RU_INCLUDE_DESC
    "DBG7 Register",
    "debug reg",
#endif
    HASH_DEBUG_DBG7_REG_OFFSET,
    0,
    0,
    755,
};

/******************************************************************************
 * Register: HASH_DEBUG_DBG8
 ******************************************************************************/
const ru_reg_rec HASH_DEBUG_DBG8_REG = 
{
    "DEBUG_DBG8",
#if RU_INCLUDE_DESC
    "DBG8 Register",
    "debug reg",
#endif
    HASH_DEBUG_DBG8_REG_OFFSET,
    0,
    0,
    756,
};

/******************************************************************************
 * Register: HASH_DEBUG_DBG9
 ******************************************************************************/
const ru_reg_rec HASH_DEBUG_DBG9_REG = 
{
    "DEBUG_DBG9",
#if RU_INCLUDE_DESC
    "DBG9 Register",
    "debug reg",
#endif
    HASH_DEBUG_DBG9_REG_OFFSET,
    0,
    0,
    757,
};

/******************************************************************************
 * Register: HASH_DEBUG_DBG10
 ******************************************************************************/
const ru_reg_rec HASH_DEBUG_DBG10_REG = 
{
    "DEBUG_DBG10",
#if RU_INCLUDE_DESC
    "DBG10 Register",
    "debug reg",
#endif
    HASH_DEBUG_DBG10_REG_OFFSET,
    0,
    0,
    758,
};

/******************************************************************************
 * Register: HASH_DEBUG_DBG11
 ******************************************************************************/
const ru_reg_rec HASH_DEBUG_DBG11_REG = 
{
    "DEBUG_DBG11",
#if RU_INCLUDE_DESC
    "DBG11 Register",
    "debug reg",
#endif
    HASH_DEBUG_DBG11_REG_OFFSET,
    0,
    0,
    759,
};

/******************************************************************************
 * Register: HASH_DEBUG_DBG12
 ******************************************************************************/
const ru_reg_rec HASH_DEBUG_DBG12_REG = 
{
    "DEBUG_DBG12",
#if RU_INCLUDE_DESC
    "DBG12 Register",
    "debug reg",
#endif
    HASH_DEBUG_DBG12_REG_OFFSET,
    0,
    0,
    760,
};

/******************************************************************************
 * Register: HASH_DEBUG_DBG13
 ******************************************************************************/
const ru_reg_rec HASH_DEBUG_DBG13_REG = 
{
    "DEBUG_DBG13",
#if RU_INCLUDE_DESC
    "DBG13 Register",
    "debug reg",
#endif
    HASH_DEBUG_DBG13_REG_OFFSET,
    0,
    0,
    761,
};

/******************************************************************************
 * Register: HASH_DEBUG_DBG14
 ******************************************************************************/
const ru_reg_rec HASH_DEBUG_DBG14_REG = 
{
    "DEBUG_DBG14",
#if RU_INCLUDE_DESC
    "DBG14 Register",
    "debug reg",
#endif
    HASH_DEBUG_DBG14_REG_OFFSET,
    0,
    0,
    762,
};

/******************************************************************************
 * Register: HASH_DEBUG_DBG15
 ******************************************************************************/
const ru_reg_rec HASH_DEBUG_DBG15_REG = 
{
    "DEBUG_DBG15",
#if RU_INCLUDE_DESC
    "DBG15 Register",
    "debug reg",
#endif
    HASH_DEBUG_DBG15_REG_OFFSET,
    0,
    0,
    763,
};

/******************************************************************************
 * Register: HASH_DEBUG_DBG16
 ******************************************************************************/
const ru_reg_rec HASH_DEBUG_DBG16_REG = 
{
    "DEBUG_DBG16",
#if RU_INCLUDE_DESC
    "DBG16 Register",
    "debug reg",
#endif
    HASH_DEBUG_DBG16_REG_OFFSET,
    0,
    0,
    764,
};

/******************************************************************************
 * Register: HASH_DEBUG_DBG17
 ******************************************************************************/
const ru_reg_rec HASH_DEBUG_DBG17_REG = 
{
    "DEBUG_DBG17",
#if RU_INCLUDE_DESC
    "DBG17 Register",
    "debug reg",
#endif
    HASH_DEBUG_DBG17_REG_OFFSET,
    0,
    0,
    765,
};

/******************************************************************************
 * Register: HASH_DEBUG_DBG18
 ******************************************************************************/
const ru_reg_rec HASH_DEBUG_DBG18_REG = 
{
    "DEBUG_DBG18",
#if RU_INCLUDE_DESC
    "DBG18 Register",
    "debug reg",
#endif
    HASH_DEBUG_DBG18_REG_OFFSET,
    0,
    0,
    766,
};

/******************************************************************************
 * Register: HASH_DEBUG_DBG19
 ******************************************************************************/
const ru_reg_rec HASH_DEBUG_DBG19_REG = 
{
    "DEBUG_DBG19",
#if RU_INCLUDE_DESC
    "DBG19 Register",
    "debug reg",
#endif
    HASH_DEBUG_DBG19_REG_OFFSET,
    0,
    0,
    767,
};

/******************************************************************************
 * Register: HASH_DEBUG_DBG20
 ******************************************************************************/
const ru_reg_rec HASH_DEBUG_DBG20_REG = 
{
    "DEBUG_DBG20",
#if RU_INCLUDE_DESC
    "DBG20 Register",
    "debug reg",
#endif
    HASH_DEBUG_DBG20_REG_OFFSET,
    0,
    0,
    768,
};

/******************************************************************************
 * Register: HASH_DEBUG_DBG_SEL
 ******************************************************************************/
const ru_reg_rec HASH_DEBUG_DBG_SEL_REG = 
{
    "DEBUG_DBG_SEL",
#if RU_INCLUDE_DESC
    "DBG_SELECT Register",
    "debug select mux",
#endif
    HASH_DEBUG_DBG_SEL_REG_OFFSET,
    0,
    0,
    769,
};

/******************************************************************************
 * Register: HASH_AGING_RAM_AGING
 ******************************************************************************/
const ru_reg_rec HASH_AGING_RAM_AGING_REG = 
{
    "AGING_RAM_AGING",
#if RU_INCLUDE_DESC
    "AGING %i Register",
    "Each bit in the ram represents hash/CAM entry."
    "(6K hash entries + 64 CAM entries)/32= 194 rows"
    "Bit 0 at the ram corresponds to entry 0 (eng0), Bit 1 at the ram corresponds to entry 1 (eng1), and so on..",
#endif
    HASH_AGING_RAM_AGING_REG_OFFSET,
    HASH_AGING_RAM_AGING_REG_RAM_CNT,
    4,
    770,
};

/******************************************************************************
 * Register: HASH_CONTEXT_RAM_CONTEXT_47_24
 ******************************************************************************/
const ru_reg_rec HASH_CONTEXT_RAM_CONTEXT_47_24_REG = 
{
    "CONTEXT_RAM_CONTEXT_47_24",
#if RU_INCLUDE_DESC
    "CONTEXT_47_24 Register",
    "24 most significant bits of an entry (first 3B)",
#endif
    HASH_CONTEXT_RAM_CONTEXT_47_24_REG_OFFSET,
    HASH_CONTEXT_RAM_CONTEXT_47_24_REG_RAM_CNT,
    8,
    771,
};

/******************************************************************************
 * Register: HASH_CONTEXT_RAM_CONTEXT_23_0
 ******************************************************************************/
const ru_reg_rec HASH_CONTEXT_RAM_CONTEXT_23_0_REG = 
{
    "CONTEXT_RAM_CONTEXT_23_0",
#if RU_INCLUDE_DESC
    "CONTEXT_23_0 Register",
    "24 least significant bits of an entry (second 3B)",
#endif
    HASH_CONTEXT_RAM_CONTEXT_23_0_REG_OFFSET,
    HASH_CONTEXT_RAM_CONTEXT_23_0_REG_RAM_CNT,
    8,
    772,
};

/******************************************************************************
 * Register: HASH_RAM_ENG_HIGH
 ******************************************************************************/
const ru_reg_rec HASH_RAM_ENG_HIGH_REG = 
{
    "RAM_ENG_HIGH",
#if RU_INCLUDE_DESC
    "ENG_BITS_63_32 Register",
    "Includes the MSB field of the hash entry",
#endif
    HASH_RAM_ENG_HIGH_REG_OFFSET,
    HASH_RAM_ENG_HIGH_REG_RAM_CNT,
    8,
    773,
};

/******************************************************************************
 * Register: HASH_RAM_ENG_LOW
 ******************************************************************************/
const ru_reg_rec HASH_RAM_ENG_LOW_REG = 
{
    "RAM_ENG_LOW",
#if RU_INCLUDE_DESC
    "ENG_BITS_31_0 Register",
    "Includes the LSB field of the entry",
#endif
    HASH_RAM_ENG_LOW_REG_OFFSET,
    HASH_RAM_ENG_LOW_REG_RAM_CNT,
    8,
    774,
};

/******************************************************************************
 * Block: HASH
 ******************************************************************************/
static const ru_reg_rec *HASH_REGS[] =
{
    &HASH_GENERAL_CONFIGURATION_PWR_SAV_EN_REG,
    &HASH_GENERAL_CONFIGURATION_PAD_HIGH_REG,
    &HASH_GENERAL_CONFIGURATION_PAD_LOW_REG,
    &HASH_GENERAL_CONFIGURATION_MULT_HIT_ERR_REG,
    &HASH_GENERAL_CONFIGURATION_UNDO_FIX_REG,
    &HASH_PM_COUNTERS_HITS_REG,
    &HASH_PM_COUNTERS_SRCHS_REG,
    &HASH_PM_COUNTERS_MISS_REG,
    &HASH_PM_COUNTERS_HIT_1ST_ACS_REG,
    &HASH_PM_COUNTERS_HIT_2ND_ACS_REG,
    &HASH_PM_COUNTERS_HIT_3RD_ACS_REG,
    &HASH_PM_COUNTERS_HIT_4TH_ACS_REG,
    &HASH_PM_COUNTERS_FRZ_CNT_REG,
    &HASH_LKUP_TBL_CFG_TBL_CFG_REG,
    &HASH_LKUP_TBL_CFG_KEY_MASK_HIGH_REG,
    &HASH_LKUP_TBL_CFG_KEY_MASK_LOW_REG,
    &HASH_LKUP_TBL_CFG_CNTXT_CFG_REG,
    &HASH_CAM_CONFIGURATION_CNTXT_CFG_REG,
    &HASH_CAM_INDIRECT_OP_REG,
    &HASH_CAM_INDIRECT_OP_DONE_REG,
    &HASH_CAM_INDIRECT_ADDR_REG,
    &HASH_CAM_INDIRECT_VLID_IN_REG,
    &HASH_CAM_INDIRECT_VLID_OUT_REG,
    &HASH_CAM_INDIRECT_RSLT_REG,
    &HASH_CAM_INDIRECT_KEY_IN_REG,
    &HASH_CAM_INDIRECT_KEY_OUT_REG,
    &HASH_CAM_BIST_BIST_STATUS_REG,
    &HASH_CAM_BIST_BIST_DBG_COMPARE_EN_REG,
    &HASH_CAM_BIST_BIST_DBG_DATA_REG,
    &HASH_CAM_BIST_BIST_DBG_DATA_SLICE_OR_STATUS_SEL_REG,
    &HASH_CAM_BIST_BIST_DBG_DATA_VALID_REG,
    &HASH_CAM_BIST_BIST_EN_REG,
    &HASH_CAM_BIST_BIST_MODE_REG,
    &HASH_CAM_BIST_BIST_RST_L_REG,
    &HASH_CAM_BIST_BIST_SKIP_ERROR_CNT_REG,
    &HASH_CAM_BIST_DBG_EN_REG,
    &HASH_CAM_BIST_BIST_CASCADE_SELECT_REG,
    &HASH_CAM_BIST_BIST_BLOCK_SELECT_REG,
    &HASH_CAM_BIST_BIST_REPAIR_ENABLE_REG,
    &HASH_INTR_CTRL_ISR_REG,
    &HASH_INTR_CTRL_ISM_REG,
    &HASH_INTR_CTRL_IER_REG,
    &HASH_INTR_CTRL_ITR_REG,
    &HASH_DEBUG_DBG0_REG,
    &HASH_DEBUG_DBG1_REG,
    &HASH_DEBUG_DBG2_REG,
    &HASH_DEBUG_DBG3_REG,
    &HASH_DEBUG_DBG4_REG,
    &HASH_DEBUG_DBG5_REG,
    &HASH_DEBUG_DBG6_REG,
    &HASH_DEBUG_DBG7_REG,
    &HASH_DEBUG_DBG8_REG,
    &HASH_DEBUG_DBG9_REG,
    &HASH_DEBUG_DBG10_REG,
    &HASH_DEBUG_DBG11_REG,
    &HASH_DEBUG_DBG12_REG,
    &HASH_DEBUG_DBG13_REG,
    &HASH_DEBUG_DBG14_REG,
    &HASH_DEBUG_DBG15_REG,
    &HASH_DEBUG_DBG16_REG,
    &HASH_DEBUG_DBG17_REG,
    &HASH_DEBUG_DBG18_REG,
    &HASH_DEBUG_DBG19_REG,
    &HASH_DEBUG_DBG20_REG,
    &HASH_DEBUG_DBG_SEL_REG,
    &HASH_AGING_RAM_AGING_REG,
    &HASH_CONTEXT_RAM_CONTEXT_47_24_REG,
    &HASH_CONTEXT_RAM_CONTEXT_23_0_REG,
    &HASH_RAM_ENG_HIGH_REG,
    &HASH_RAM_ENG_LOW_REG,
};

unsigned long HASH_ADDRS[] =
{
    0x82e20000,
};

const ru_block_rec HASH_BLOCK = 
{
    "HASH",
    HASH_ADDRS,
    1,
    70,
    HASH_REGS
};

/* End of file XRDP_HASH.c */
