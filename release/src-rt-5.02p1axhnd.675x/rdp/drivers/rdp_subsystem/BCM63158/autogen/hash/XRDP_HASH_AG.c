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
 * Field: HASH_GENERAL_CONFIGURATION_PWR_SAV_EN_VALUE
 ******************************************************************************/
const ru_field_rec HASH_GENERAL_CONFIGURATION_PWR_SAV_EN_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "Value",
    ".",
#endif
    HASH_GENERAL_CONFIGURATION_PWR_SAV_EN_VALUE_FIELD_MASK,
    0,
    HASH_GENERAL_CONFIGURATION_PWR_SAV_EN_VALUE_FIELD_WIDTH,
    HASH_GENERAL_CONFIGURATION_PWR_SAV_EN_VALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_GENERAL_CONFIGURATION_PWR_SAV_EN_RESERVED0
 ******************************************************************************/
const ru_field_rec HASH_GENERAL_CONFIGURATION_PWR_SAV_EN_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    HASH_GENERAL_CONFIGURATION_PWR_SAV_EN_RESERVED0_FIELD_MASK,
    0,
    HASH_GENERAL_CONFIGURATION_PWR_SAV_EN_RESERVED0_FIELD_WIDTH,
    HASH_GENERAL_CONFIGURATION_PWR_SAV_EN_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_GENERAL_CONFIGURATION_PAD_HIGH_VAL
 ******************************************************************************/
const ru_field_rec HASH_GENERAL_CONFIGURATION_PAD_HIGH_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "Value",
    "Determines the padding value added to keys according to the selected MASK",
#endif
    HASH_GENERAL_CONFIGURATION_PAD_HIGH_VAL_FIELD_MASK,
    0,
    HASH_GENERAL_CONFIGURATION_PAD_HIGH_VAL_FIELD_WIDTH,
    HASH_GENERAL_CONFIGURATION_PAD_HIGH_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_GENERAL_CONFIGURATION_PAD_HIGH_RESERVED0
 ******************************************************************************/
const ru_field_rec HASH_GENERAL_CONFIGURATION_PAD_HIGH_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    HASH_GENERAL_CONFIGURATION_PAD_HIGH_RESERVED0_FIELD_MASK,
    0,
    HASH_GENERAL_CONFIGURATION_PAD_HIGH_RESERVED0_FIELD_WIDTH,
    HASH_GENERAL_CONFIGURATION_PAD_HIGH_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_GENERAL_CONFIGURATION_PAD_LOW_VAL
 ******************************************************************************/
const ru_field_rec HASH_GENERAL_CONFIGURATION_PAD_LOW_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "Value",
    "Determines the padding value added to keys according to the selected MASK",
#endif
    HASH_GENERAL_CONFIGURATION_PAD_LOW_VAL_FIELD_MASK,
    0,
    HASH_GENERAL_CONFIGURATION_PAD_LOW_VAL_FIELD_WIDTH,
    HASH_GENERAL_CONFIGURATION_PAD_LOW_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_GENERAL_CONFIGURATION_MULT_HIT_ERR_VAL
 ******************************************************************************/
const ru_field_rec HASH_GENERAL_CONFIGURATION_MULT_HIT_ERR_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "Value",
    "In case of multiple match this reg captures the hit indication per engine. This is a read clear reg.",
#endif
    HASH_GENERAL_CONFIGURATION_MULT_HIT_ERR_VAL_FIELD_MASK,
    0,
    HASH_GENERAL_CONFIGURATION_MULT_HIT_ERR_VAL_FIELD_WIDTH,
    HASH_GENERAL_CONFIGURATION_MULT_HIT_ERR_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: HASH_GENERAL_CONFIGURATION_MULT_HIT_ERR_RESERVED0
 ******************************************************************************/
const ru_field_rec HASH_GENERAL_CONFIGURATION_MULT_HIT_ERR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    HASH_GENERAL_CONFIGURATION_MULT_HIT_ERR_RESERVED0_FIELD_MASK,
    0,
    HASH_GENERAL_CONFIGURATION_MULT_HIT_ERR_RESERVED0_FIELD_WIDTH,
    HASH_GENERAL_CONFIGURATION_MULT_HIT_ERR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: HASH_GENERAL_CONFIGURATION_UNDO_FIX_FRST_MUL_HIT
 ******************************************************************************/
const ru_field_rec HASH_GENERAL_CONFIGURATION_UNDO_FIX_FRST_MUL_HIT_FIELD =
{
    "FRST_MUL_HIT",
#if RU_INCLUDE_DESC
    "first_mult_hit",
    "The bug fixed lacking in identification and reporting when a multiple hit occurs in the first search."
    "",
#endif
    HASH_GENERAL_CONFIGURATION_UNDO_FIX_FRST_MUL_HIT_FIELD_MASK,
    0,
    HASH_GENERAL_CONFIGURATION_UNDO_FIX_FRST_MUL_HIT_FIELD_WIDTH,
    HASH_GENERAL_CONFIGURATION_UNDO_FIX_FRST_MUL_HIT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_GENERAL_CONFIGURATION_UNDO_FIX_RESERVED0
 ******************************************************************************/
const ru_field_rec HASH_GENERAL_CONFIGURATION_UNDO_FIX_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    HASH_GENERAL_CONFIGURATION_UNDO_FIX_RESERVED0_FIELD_MASK,
    0,
    HASH_GENERAL_CONFIGURATION_UNDO_FIX_RESERVED0_FIELD_WIDTH,
    HASH_GENERAL_CONFIGURATION_UNDO_FIX_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_PM_COUNTERS_HITS_CNT
 ******************************************************************************/
const ru_field_rec HASH_PM_COUNTERS_HITS_CNT_FIELD =
{
    "CNT",
#if RU_INCLUDE_DESC
    "CNT",
    ".",
#endif
    HASH_PM_COUNTERS_HITS_CNT_FIELD_MASK,
    0,
    HASH_PM_COUNTERS_HITS_CNT_FIELD_WIDTH,
    HASH_PM_COUNTERS_HITS_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: HASH_PM_COUNTERS_SRCHS_CNT
 ******************************************************************************/
const ru_field_rec HASH_PM_COUNTERS_SRCHS_CNT_FIELD =
{
    "CNT",
#if RU_INCLUDE_DESC
    "CNT",
    ".",
#endif
    HASH_PM_COUNTERS_SRCHS_CNT_FIELD_MASK,
    0,
    HASH_PM_COUNTERS_SRCHS_CNT_FIELD_WIDTH,
    HASH_PM_COUNTERS_SRCHS_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: HASH_PM_COUNTERS_MISS_CNT
 ******************************************************************************/
const ru_field_rec HASH_PM_COUNTERS_MISS_CNT_FIELD =
{
    "CNT",
#if RU_INCLUDE_DESC
    "CNT",
    ".",
#endif
    HASH_PM_COUNTERS_MISS_CNT_FIELD_MASK,
    0,
    HASH_PM_COUNTERS_MISS_CNT_FIELD_WIDTH,
    HASH_PM_COUNTERS_MISS_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: HASH_PM_COUNTERS_HIT_1ST_ACS_CNT
 ******************************************************************************/
const ru_field_rec HASH_PM_COUNTERS_HIT_1ST_ACS_CNT_FIELD =
{
    "CNT",
#if RU_INCLUDE_DESC
    "CNT",
    ".",
#endif
    HASH_PM_COUNTERS_HIT_1ST_ACS_CNT_FIELD_MASK,
    0,
    HASH_PM_COUNTERS_HIT_1ST_ACS_CNT_FIELD_WIDTH,
    HASH_PM_COUNTERS_HIT_1ST_ACS_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: HASH_PM_COUNTERS_HIT_2ND_ACS_CNT
 ******************************************************************************/
const ru_field_rec HASH_PM_COUNTERS_HIT_2ND_ACS_CNT_FIELD =
{
    "CNT",
#if RU_INCLUDE_DESC
    "CNT",
    ".",
#endif
    HASH_PM_COUNTERS_HIT_2ND_ACS_CNT_FIELD_MASK,
    0,
    HASH_PM_COUNTERS_HIT_2ND_ACS_CNT_FIELD_WIDTH,
    HASH_PM_COUNTERS_HIT_2ND_ACS_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: HASH_PM_COUNTERS_HIT_3RD_ACS_CNT
 ******************************************************************************/
const ru_field_rec HASH_PM_COUNTERS_HIT_3RD_ACS_CNT_FIELD =
{
    "CNT",
#if RU_INCLUDE_DESC
    "CNT",
    ".",
#endif
    HASH_PM_COUNTERS_HIT_3RD_ACS_CNT_FIELD_MASK,
    0,
    HASH_PM_COUNTERS_HIT_3RD_ACS_CNT_FIELD_WIDTH,
    HASH_PM_COUNTERS_HIT_3RD_ACS_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: HASH_PM_COUNTERS_HIT_4TH_ACS_CNT
 ******************************************************************************/
const ru_field_rec HASH_PM_COUNTERS_HIT_4TH_ACS_CNT_FIELD =
{
    "CNT",
#if RU_INCLUDE_DESC
    "CNT",
    ".",
#endif
    HASH_PM_COUNTERS_HIT_4TH_ACS_CNT_FIELD_MASK,
    0,
    HASH_PM_COUNTERS_HIT_4TH_ACS_CNT_FIELD_WIDTH,
    HASH_PM_COUNTERS_HIT_4TH_ACS_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: HASH_PM_COUNTERS_FRZ_CNT_VAL
 ******************************************************************************/
const ru_field_rec HASH_PM_COUNTERS_FRZ_CNT_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "freeze_cnt",
    "Freezes counters update",
#endif
    HASH_PM_COUNTERS_FRZ_CNT_VAL_FIELD_MASK,
    0,
    HASH_PM_COUNTERS_FRZ_CNT_VAL_FIELD_WIDTH,
    HASH_PM_COUNTERS_FRZ_CNT_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_PM_COUNTERS_FRZ_CNT_RESERVED0
 ******************************************************************************/
const ru_field_rec HASH_PM_COUNTERS_FRZ_CNT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    HASH_PM_COUNTERS_FRZ_CNT_RESERVED0_FIELD_MASK,
    0,
    HASH_PM_COUNTERS_FRZ_CNT_RESERVED0_FIELD_WIDTH,
    HASH_PM_COUNTERS_FRZ_CNT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_LKUP_TBL_CFG_TBL_CFG_HASH_BASE_ADDR
 ******************************************************************************/
const ru_field_rec HASH_LKUP_TBL_CFG_TBL_CFG_HASH_BASE_ADDR_FIELD =
{
    "HASH_BASE_ADDR",
#if RU_INCLUDE_DESC
    "hash_base_addr",
    "Base address of the hash ram per engine."
    "Varies between 0 to 1535"
    "Indicates from which address start looking the key."
    "Note, base address must be aligned to table size - table size of 128 cant get base 64",
#endif
    HASH_LKUP_TBL_CFG_TBL_CFG_HASH_BASE_ADDR_FIELD_MASK,
    0,
    HASH_LKUP_TBL_CFG_TBL_CFG_HASH_BASE_ADDR_FIELD_WIDTH,
    HASH_LKUP_TBL_CFG_TBL_CFG_HASH_BASE_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_LKUP_TBL_CFG_TBL_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec HASH_LKUP_TBL_CFG_TBL_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    HASH_LKUP_TBL_CFG_TBL_CFG_RESERVED0_FIELD_MASK,
    0,
    HASH_LKUP_TBL_CFG_TBL_CFG_RESERVED0_FIELD_WIDTH,
    HASH_LKUP_TBL_CFG_TBL_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_LKUP_TBL_CFG_TBL_CFG_TBL_SIZE
 ******************************************************************************/
const ru_field_rec HASH_LKUP_TBL_CFG_TBL_CFG_TBL_SIZE_FIELD =
{
    "TBL_SIZE",
#if RU_INCLUDE_DESC
    "Table_Size",
    "Number of entries in the  table per engine."
    "Total  entries should be multiplied with the number of engines - by 4.",
#endif
    HASH_LKUP_TBL_CFG_TBL_CFG_TBL_SIZE_FIELD_MASK,
    0,
    HASH_LKUP_TBL_CFG_TBL_CFG_TBL_SIZE_FIELD_WIDTH,
    HASH_LKUP_TBL_CFG_TBL_CFG_TBL_SIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_LKUP_TBL_CFG_TBL_CFG_RESERVED1
 ******************************************************************************/
const ru_field_rec HASH_LKUP_TBL_CFG_TBL_CFG_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    HASH_LKUP_TBL_CFG_TBL_CFG_RESERVED1_FIELD_MASK,
    0,
    HASH_LKUP_TBL_CFG_TBL_CFG_RESERVED1_FIELD_WIDTH,
    HASH_LKUP_TBL_CFG_TBL_CFG_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_LKUP_TBL_CFG_TBL_CFG_MAX_HOP
 ******************************************************************************/
const ru_field_rec HASH_LKUP_TBL_CFG_TBL_CFG_MAX_HOP_FIELD =
{
    "MAX_HOP",
#if RU_INCLUDE_DESC
    "Max_Hop",
    "Max Search depth per engine."
    "Supports up to 16 and cannot exceed table size."
    "For performance requirement it should be limited to 4",
#endif
    HASH_LKUP_TBL_CFG_TBL_CFG_MAX_HOP_FIELD_MASK,
    0,
    HASH_LKUP_TBL_CFG_TBL_CFG_MAX_HOP_FIELD_WIDTH,
    HASH_LKUP_TBL_CFG_TBL_CFG_MAX_HOP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_LKUP_TBL_CFG_TBL_CFG_CAM_EN
 ******************************************************************************/
const ru_field_rec HASH_LKUP_TBL_CFG_TBL_CFG_CAM_EN_FIELD =
{
    "CAM_EN",
#if RU_INCLUDE_DESC
    "cam_en",
    "CAM Search is enabled."
    "If the key not found in the hash table and this flag enabled the key will be searched in the CAm.",
#endif
    HASH_LKUP_TBL_CFG_TBL_CFG_CAM_EN_FIELD_MASK,
    0,
    HASH_LKUP_TBL_CFG_TBL_CFG_CAM_EN_FIELD_WIDTH,
    HASH_LKUP_TBL_CFG_TBL_CFG_CAM_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_LKUP_TBL_CFG_TBL_CFG_DIRECT_LKUP_EN
 ******************************************************************************/
const ru_field_rec HASH_LKUP_TBL_CFG_TBL_CFG_DIRECT_LKUP_EN_FIELD =
{
    "DIRECT_LKUP_EN",
#if RU_INCLUDE_DESC
    "direct_lkup_en",
    "Direct lookup enable."
    "Allows accessing the table without hash calculation- direct access.",
#endif
    HASH_LKUP_TBL_CFG_TBL_CFG_DIRECT_LKUP_EN_FIELD_MASK,
    0,
    HASH_LKUP_TBL_CFG_TBL_CFG_DIRECT_LKUP_EN_FIELD_WIDTH,
    HASH_LKUP_TBL_CFG_TBL_CFG_DIRECT_LKUP_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_LKUP_TBL_CFG_TBL_CFG_HASH_TYPE
 ******************************************************************************/
const ru_field_rec HASH_LKUP_TBL_CFG_TBL_CFG_HASH_TYPE_FIELD =
{
    "HASH_TYPE",
#if RU_INCLUDE_DESC
    "Hash_Type",
    "Hash function type",
#endif
    HASH_LKUP_TBL_CFG_TBL_CFG_HASH_TYPE_FIELD_MASK,
    0,
    HASH_LKUP_TBL_CFG_TBL_CFG_HASH_TYPE_FIELD_WIDTH,
    HASH_LKUP_TBL_CFG_TBL_CFG_HASH_TYPE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_LKUP_TBL_CFG_TBL_CFG_RESERVED2
 ******************************************************************************/
const ru_field_rec HASH_LKUP_TBL_CFG_TBL_CFG_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    HASH_LKUP_TBL_CFG_TBL_CFG_RESERVED2_FIELD_MASK,
    0,
    HASH_LKUP_TBL_CFG_TBL_CFG_RESERVED2_FIELD_WIDTH,
    HASH_LKUP_TBL_CFG_TBL_CFG_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_LKUP_TBL_CFG_TBL_CFG_INT_CNTX_SIZE
 ******************************************************************************/
const ru_field_rec HASH_LKUP_TBL_CFG_TBL_CFG_INT_CNTX_SIZE_FIELD =
{
    "INT_CNTX_SIZE",
#if RU_INCLUDE_DESC
    "int_cntx_size",
    "If the key smaller than 60 bit, then it supported to store in the remaining bits an internal context data 3B or 6B.",
#endif
    HASH_LKUP_TBL_CFG_TBL_CFG_INT_CNTX_SIZE_FIELD_MASK,
    0,
    HASH_LKUP_TBL_CFG_TBL_CFG_INT_CNTX_SIZE_FIELD_WIDTH,
    HASH_LKUP_TBL_CFG_TBL_CFG_INT_CNTX_SIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_LKUP_TBL_CFG_TBL_CFG_RESERVED3
 ******************************************************************************/
const ru_field_rec HASH_LKUP_TBL_CFG_TBL_CFG_RESERVED3_FIELD =
{
    "RESERVED3",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    HASH_LKUP_TBL_CFG_TBL_CFG_RESERVED3_FIELD_MASK,
    0,
    HASH_LKUP_TBL_CFG_TBL_CFG_RESERVED3_FIELD_WIDTH,
    HASH_LKUP_TBL_CFG_TBL_CFG_RESERVED3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_LKUP_TBL_CFG_KEY_MASK_HIGH_MASKH
 ******************************************************************************/
const ru_field_rec HASH_LKUP_TBL_CFG_KEY_MASK_HIGH_MASKH_FIELD =
{
    "MASKH",
#if RU_INCLUDE_DESC
    "MASKH",
    "MASK HIGH applied on the 28 msb of the current part of key for the current search table."
    "The value used for padding purpose and comparison to the hash content."
    "",
#endif
    HASH_LKUP_TBL_CFG_KEY_MASK_HIGH_MASKH_FIELD_MASK,
    0,
    HASH_LKUP_TBL_CFG_KEY_MASK_HIGH_MASKH_FIELD_WIDTH,
    HASH_LKUP_TBL_CFG_KEY_MASK_HIGH_MASKH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_LKUP_TBL_CFG_KEY_MASK_HIGH_RESERVED0
 ******************************************************************************/
const ru_field_rec HASH_LKUP_TBL_CFG_KEY_MASK_HIGH_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    HASH_LKUP_TBL_CFG_KEY_MASK_HIGH_RESERVED0_FIELD_MASK,
    0,
    HASH_LKUP_TBL_CFG_KEY_MASK_HIGH_RESERVED0_FIELD_WIDTH,
    HASH_LKUP_TBL_CFG_KEY_MASK_HIGH_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_LKUP_TBL_CFG_KEY_MASK_LOW_MASKL
 ******************************************************************************/
const ru_field_rec HASH_LKUP_TBL_CFG_KEY_MASK_LOW_MASKL_FIELD =
{
    "MASKL",
#if RU_INCLUDE_DESC
    "MASKL",
    "MASK LOW applied on the 32 lsb of the current part of key for the current search table."
    ""
    "The value used for padding purpose and comparison to the hash content.",
#endif
    HASH_LKUP_TBL_CFG_KEY_MASK_LOW_MASKL_FIELD_MASK,
    0,
    HASH_LKUP_TBL_CFG_KEY_MASK_LOW_MASKL_FIELD_WIDTH,
    HASH_LKUP_TBL_CFG_KEY_MASK_LOW_MASKL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_LKUP_TBL_CFG_CNTXT_CFG_BASE_ADDRESS
 ******************************************************************************/
const ru_field_rec HASH_LKUP_TBL_CFG_CNTXT_CFG_BASE_ADDRESS_FIELD =
{
    "BASE_ADDRESS",
#if RU_INCLUDE_DESC
    "Base_Address",
    "Context table base address in the RAM (6Bytes X 3264entries) ."
    "Indicates from which address start looking at the context."
    "The address varies between 0 to 3264 (including 196 CAM entries)"
    ""
    "It should be calculated according below formula:"
    "Context_base_addr[12:0] = sum of (table_size_per_engine*num_of_eng*context_size)/6 for all preceding tables"
    "",
#endif
    HASH_LKUP_TBL_CFG_CNTXT_CFG_BASE_ADDRESS_FIELD_MASK,
    0,
    HASH_LKUP_TBL_CFG_CNTXT_CFG_BASE_ADDRESS_FIELD_WIDTH,
    HASH_LKUP_TBL_CFG_CNTXT_CFG_BASE_ADDRESS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_LKUP_TBL_CFG_CNTXT_CFG_FIRST_HASH_IDX
 ******************************************************************************/
const ru_field_rec HASH_LKUP_TBL_CFG_CNTXT_CFG_FIRST_HASH_IDX_FIELD =
{
    "FIRST_HASH_IDX",
#if RU_INCLUDE_DESC
    "First_hash_index",
    "Indicates the first entry of the particular table in the context table."
    ""
    "It should be calculated according to below formula:"
    "First_hash_index = sum of (table_size_per_engine*num_of_eng) for all preceding tables",
#endif
    HASH_LKUP_TBL_CFG_CNTXT_CFG_FIRST_HASH_IDX_FIELD_MASK,
    0,
    HASH_LKUP_TBL_CFG_CNTXT_CFG_FIRST_HASH_IDX_FIELD_WIDTH,
    HASH_LKUP_TBL_CFG_CNTXT_CFG_FIRST_HASH_IDX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_LKUP_TBL_CFG_CNTXT_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec HASH_LKUP_TBL_CFG_CNTXT_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    HASH_LKUP_TBL_CFG_CNTXT_CFG_RESERVED0_FIELD_MASK,
    0,
    HASH_LKUP_TBL_CFG_CNTXT_CFG_RESERVED0_FIELD_WIDTH,
    HASH_LKUP_TBL_CFG_CNTXT_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_LKUP_TBL_CFG_CNTXT_CFG_CNXT_SIZE
 ******************************************************************************/
const ru_field_rec HASH_LKUP_TBL_CFG_CNTXT_CFG_CNXT_SIZE_FIELD =
{
    "CNXT_SIZE",
#if RU_INCLUDE_DESC
    "Context_size",
    "Context entry size (in the context RAM)."
    "Varies between 0B to 12B in steps of 3B"
    ""
    "Context may also be extracted directly from Look-up Table (up to 6B).",
#endif
    HASH_LKUP_TBL_CFG_CNTXT_CFG_CNXT_SIZE_FIELD_MASK,
    0,
    HASH_LKUP_TBL_CFG_CNTXT_CFG_CNXT_SIZE_FIELD_WIDTH,
    HASH_LKUP_TBL_CFG_CNTXT_CFG_CNXT_SIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_LKUP_TBL_CFG_CNTXT_CFG_RESERVED1
 ******************************************************************************/
const ru_field_rec HASH_LKUP_TBL_CFG_CNTXT_CFG_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    HASH_LKUP_TBL_CFG_CNTXT_CFG_RESERVED1_FIELD_MASK,
    0,
    HASH_LKUP_TBL_CFG_CNTXT_CFG_RESERVED1_FIELD_WIDTH,
    HASH_LKUP_TBL_CFG_CNTXT_CFG_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_CAM_CONFIGURATION_CNTXT_CFG_BASE_ADDRESS
 ******************************************************************************/
const ru_field_rec HASH_CAM_CONFIGURATION_CNTXT_CFG_BASE_ADDRESS_FIELD =
{
    "BASE_ADDRESS",
#if RU_INCLUDE_DESC
    "Base_Address",
    "Context table base address in the RAM (6Bytes X 3264entries) ."
    "Indicates from which address start looking at the context."
    "The address varies between 0 to 3264 (including 196 CAM entries)"
    ""
    "It should be calculated according below formula:"
    "Context_base_addr[12:0] = sum of (table_size*context_size)/6 for all preceding tables"
    "",
#endif
    HASH_CAM_CONFIGURATION_CNTXT_CFG_BASE_ADDRESS_FIELD_MASK,
    0,
    HASH_CAM_CONFIGURATION_CNTXT_CFG_BASE_ADDRESS_FIELD_WIDTH,
    HASH_CAM_CONFIGURATION_CNTXT_CFG_BASE_ADDRESS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_CAM_CONFIGURATION_CNTXT_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec HASH_CAM_CONFIGURATION_CNTXT_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    HASH_CAM_CONFIGURATION_CNTXT_CFG_RESERVED0_FIELD_MASK,
    0,
    HASH_CAM_CONFIGURATION_CNTXT_CFG_RESERVED0_FIELD_WIDTH,
    HASH_CAM_CONFIGURATION_CNTXT_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_CAM_INDIRECT_OP_CMD
 ******************************************************************************/
const ru_field_rec HASH_CAM_INDIRECT_OP_CMD_FIELD =
{
    "CMD",
#if RU_INCLUDE_DESC
    "Command",
    ".",
#endif
    HASH_CAM_INDIRECT_OP_CMD_FIELD_MASK,
    0,
    HASH_CAM_INDIRECT_OP_CMD_FIELD_WIDTH,
    HASH_CAM_INDIRECT_OP_CMD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_CAM_INDIRECT_OP_RESERVED0
 ******************************************************************************/
const ru_field_rec HASH_CAM_INDIRECT_OP_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    HASH_CAM_INDIRECT_OP_RESERVED0_FIELD_MASK,
    0,
    HASH_CAM_INDIRECT_OP_RESERVED0_FIELD_WIDTH,
    HASH_CAM_INDIRECT_OP_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_CAM_INDIRECT_OP_DONE_VAL
 ******************************************************************************/
const ru_field_rec HASH_CAM_INDIRECT_OP_DONE_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "val",
    ".",
#endif
    HASH_CAM_INDIRECT_OP_DONE_VAL_FIELD_MASK,
    0,
    HASH_CAM_INDIRECT_OP_DONE_VAL_FIELD_WIDTH,
    HASH_CAM_INDIRECT_OP_DONE_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: HASH_CAM_INDIRECT_OP_DONE_RESERVED0
 ******************************************************************************/
const ru_field_rec HASH_CAM_INDIRECT_OP_DONE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    HASH_CAM_INDIRECT_OP_DONE_RESERVED0_FIELD_MASK,
    0,
    HASH_CAM_INDIRECT_OP_DONE_RESERVED0_FIELD_WIDTH,
    HASH_CAM_INDIRECT_OP_DONE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: HASH_CAM_INDIRECT_ADDR_KEY1_IND
 ******************************************************************************/
const ru_field_rec HASH_CAM_INDIRECT_ADDR_KEY1_IND_FIELD =
{
    "KEY1_IND",
#if RU_INCLUDE_DESC
    "Key1_indication",
    "This bit indicate if the operation (RD/WR) is performed on the key0 or key1 part of the entry",
#endif
    HASH_CAM_INDIRECT_ADDR_KEY1_IND_FIELD_MASK,
    0,
    HASH_CAM_INDIRECT_ADDR_KEY1_IND_FIELD_WIDTH,
    HASH_CAM_INDIRECT_ADDR_KEY1_IND_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_CAM_INDIRECT_ADDR_ENTRY_ADDR
 ******************************************************************************/
const ru_field_rec HASH_CAM_INDIRECT_ADDR_ENTRY_ADDR_FIELD =
{
    "ENTRY_ADDR",
#if RU_INCLUDE_DESC
    "Entry_Address",
    "Address of the entry",
#endif
    HASH_CAM_INDIRECT_ADDR_ENTRY_ADDR_FIELD_MASK,
    0,
    HASH_CAM_INDIRECT_ADDR_ENTRY_ADDR_FIELD_WIDTH,
    HASH_CAM_INDIRECT_ADDR_ENTRY_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_CAM_INDIRECT_ADDR_RESERVED0
 ******************************************************************************/
const ru_field_rec HASH_CAM_INDIRECT_ADDR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    HASH_CAM_INDIRECT_ADDR_RESERVED0_FIELD_MASK,
    0,
    HASH_CAM_INDIRECT_ADDR_RESERVED0_FIELD_WIDTH,
    HASH_CAM_INDIRECT_ADDR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_CAM_INDIRECT_VLID_IN_VALID
 ******************************************************************************/
const ru_field_rec HASH_CAM_INDIRECT_VLID_IN_VALID_FIELD =
{
    "VALID",
#if RU_INCLUDE_DESC
    "Valid",
    ".",
#endif
    HASH_CAM_INDIRECT_VLID_IN_VALID_FIELD_MASK,
    0,
    HASH_CAM_INDIRECT_VLID_IN_VALID_FIELD_WIDTH,
    HASH_CAM_INDIRECT_VLID_IN_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_CAM_INDIRECT_VLID_IN_RESERVED0
 ******************************************************************************/
const ru_field_rec HASH_CAM_INDIRECT_VLID_IN_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    HASH_CAM_INDIRECT_VLID_IN_RESERVED0_FIELD_MASK,
    0,
    HASH_CAM_INDIRECT_VLID_IN_RESERVED0_FIELD_WIDTH,
    HASH_CAM_INDIRECT_VLID_IN_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_CAM_INDIRECT_VLID_OUT_VALID
 ******************************************************************************/
const ru_field_rec HASH_CAM_INDIRECT_VLID_OUT_VALID_FIELD =
{
    "VALID",
#if RU_INCLUDE_DESC
    "Valid",
    ".",
#endif
    HASH_CAM_INDIRECT_VLID_OUT_VALID_FIELD_MASK,
    0,
    HASH_CAM_INDIRECT_VLID_OUT_VALID_FIELD_WIDTH,
    HASH_CAM_INDIRECT_VLID_OUT_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_CAM_INDIRECT_VLID_OUT_RESERVED0
 ******************************************************************************/
const ru_field_rec HASH_CAM_INDIRECT_VLID_OUT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    HASH_CAM_INDIRECT_VLID_OUT_RESERVED0_FIELD_MASK,
    0,
    HASH_CAM_INDIRECT_VLID_OUT_RESERVED0_FIELD_WIDTH,
    HASH_CAM_INDIRECT_VLID_OUT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_CAM_INDIRECT_RSLT_MATCH
 ******************************************************************************/
const ru_field_rec HASH_CAM_INDIRECT_RSLT_MATCH_FIELD =
{
    "MATCH",
#if RU_INCLUDE_DESC
    "match",
    "indicate if a match was found",
#endif
    HASH_CAM_INDIRECT_RSLT_MATCH_FIELD_MASK,
    0,
    HASH_CAM_INDIRECT_RSLT_MATCH_FIELD_WIDTH,
    HASH_CAM_INDIRECT_RSLT_MATCH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: HASH_CAM_INDIRECT_RSLT_RESERVED0
 ******************************************************************************/
const ru_field_rec HASH_CAM_INDIRECT_RSLT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    HASH_CAM_INDIRECT_RSLT_RESERVED0_FIELD_MASK,
    0,
    HASH_CAM_INDIRECT_RSLT_RESERVED0_FIELD_WIDTH,
    HASH_CAM_INDIRECT_RSLT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: HASH_CAM_INDIRECT_RSLT_INDEX
 ******************************************************************************/
const ru_field_rec HASH_CAM_INDIRECT_RSLT_INDEX_FIELD =
{
    "INDEX",
#if RU_INCLUDE_DESC
    "index",
    "index related to a match result",
#endif
    HASH_CAM_INDIRECT_RSLT_INDEX_FIELD_MASK,
    0,
    HASH_CAM_INDIRECT_RSLT_INDEX_FIELD_WIDTH,
    HASH_CAM_INDIRECT_RSLT_INDEX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: HASH_CAM_INDIRECT_RSLT_RESERVED1
 ******************************************************************************/
const ru_field_rec HASH_CAM_INDIRECT_RSLT_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    HASH_CAM_INDIRECT_RSLT_RESERVED1_FIELD_MASK,
    0,
    HASH_CAM_INDIRECT_RSLT_RESERVED1_FIELD_WIDTH,
    HASH_CAM_INDIRECT_RSLT_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: HASH_CAM_INDIRECT_KEY_IN_VALUE
 ******************************************************************************/
const ru_field_rec HASH_CAM_INDIRECT_KEY_IN_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "Value",
    ".",
#endif
    HASH_CAM_INDIRECT_KEY_IN_VALUE_FIELD_MASK,
    0,
    HASH_CAM_INDIRECT_KEY_IN_VALUE_FIELD_WIDTH,
    HASH_CAM_INDIRECT_KEY_IN_VALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_CAM_INDIRECT_KEY_OUT_VALUE
 ******************************************************************************/
const ru_field_rec HASH_CAM_INDIRECT_KEY_OUT_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "Value",
    ".",
#endif
    HASH_CAM_INDIRECT_KEY_OUT_VALUE_FIELD_MASK,
    0,
    HASH_CAM_INDIRECT_KEY_OUT_VALUE_FIELD_WIDTH,
    HASH_CAM_INDIRECT_KEY_OUT_VALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: HASH_CAM_BIST_BIST_STATUS_VALUE
 ******************************************************************************/
const ru_field_rec HASH_CAM_BIST_BIST_STATUS_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "Value",
    ".",
#endif
    HASH_CAM_BIST_BIST_STATUS_VALUE_FIELD_MASK,
    0,
    HASH_CAM_BIST_BIST_STATUS_VALUE_FIELD_WIDTH,
    HASH_CAM_BIST_BIST_STATUS_VALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: HASH_CAM_BIST_BIST_DBG_COMPARE_EN_VALUE
 ******************************************************************************/
const ru_field_rec HASH_CAM_BIST_BIST_DBG_COMPARE_EN_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "Value",
    ".",
#endif
    HASH_CAM_BIST_BIST_DBG_COMPARE_EN_VALUE_FIELD_MASK,
    0,
    HASH_CAM_BIST_BIST_DBG_COMPARE_EN_VALUE_FIELD_WIDTH,
    HASH_CAM_BIST_BIST_DBG_COMPARE_EN_VALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_CAM_BIST_BIST_DBG_COMPARE_EN_RESERVED0
 ******************************************************************************/
const ru_field_rec HASH_CAM_BIST_BIST_DBG_COMPARE_EN_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    HASH_CAM_BIST_BIST_DBG_COMPARE_EN_RESERVED0_FIELD_MASK,
    0,
    HASH_CAM_BIST_BIST_DBG_COMPARE_EN_RESERVED0_FIELD_WIDTH,
    HASH_CAM_BIST_BIST_DBG_COMPARE_EN_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_CAM_BIST_BIST_DBG_DATA_VALUE
 ******************************************************************************/
const ru_field_rec HASH_CAM_BIST_BIST_DBG_DATA_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "Value",
    ".",
#endif
    HASH_CAM_BIST_BIST_DBG_DATA_VALUE_FIELD_MASK,
    0,
    HASH_CAM_BIST_BIST_DBG_DATA_VALUE_FIELD_WIDTH,
    HASH_CAM_BIST_BIST_DBG_DATA_VALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_CAM_BIST_BIST_DBG_DATA_SLICE_OR_STATUS_SEL_VALUE
 ******************************************************************************/
const ru_field_rec HASH_CAM_BIST_BIST_DBG_DATA_SLICE_OR_STATUS_SEL_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "Value",
    ".",
#endif
    HASH_CAM_BIST_BIST_DBG_DATA_SLICE_OR_STATUS_SEL_VALUE_FIELD_MASK,
    0,
    HASH_CAM_BIST_BIST_DBG_DATA_SLICE_OR_STATUS_SEL_VALUE_FIELD_WIDTH,
    HASH_CAM_BIST_BIST_DBG_DATA_SLICE_OR_STATUS_SEL_VALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_CAM_BIST_BIST_DBG_DATA_SLICE_OR_STATUS_SEL_RESERVED0
 ******************************************************************************/
const ru_field_rec HASH_CAM_BIST_BIST_DBG_DATA_SLICE_OR_STATUS_SEL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    HASH_CAM_BIST_BIST_DBG_DATA_SLICE_OR_STATUS_SEL_RESERVED0_FIELD_MASK,
    0,
    HASH_CAM_BIST_BIST_DBG_DATA_SLICE_OR_STATUS_SEL_RESERVED0_FIELD_WIDTH,
    HASH_CAM_BIST_BIST_DBG_DATA_SLICE_OR_STATUS_SEL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_CAM_BIST_BIST_DBG_DATA_VALID_VALUE
 ******************************************************************************/
const ru_field_rec HASH_CAM_BIST_BIST_DBG_DATA_VALID_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "Value",
    ".",
#endif
    HASH_CAM_BIST_BIST_DBG_DATA_VALID_VALUE_FIELD_MASK,
    0,
    HASH_CAM_BIST_BIST_DBG_DATA_VALID_VALUE_FIELD_WIDTH,
    HASH_CAM_BIST_BIST_DBG_DATA_VALID_VALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_CAM_BIST_BIST_DBG_DATA_VALID_RESERVED0
 ******************************************************************************/
const ru_field_rec HASH_CAM_BIST_BIST_DBG_DATA_VALID_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    HASH_CAM_BIST_BIST_DBG_DATA_VALID_RESERVED0_FIELD_MASK,
    0,
    HASH_CAM_BIST_BIST_DBG_DATA_VALID_RESERVED0_FIELD_WIDTH,
    HASH_CAM_BIST_BIST_DBG_DATA_VALID_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_CAM_BIST_BIST_EN_VALUE
 ******************************************************************************/
const ru_field_rec HASH_CAM_BIST_BIST_EN_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "Value",
    ".",
#endif
    HASH_CAM_BIST_BIST_EN_VALUE_FIELD_MASK,
    0,
    HASH_CAM_BIST_BIST_EN_VALUE_FIELD_WIDTH,
    HASH_CAM_BIST_BIST_EN_VALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_CAM_BIST_BIST_EN_RESERVED0
 ******************************************************************************/
const ru_field_rec HASH_CAM_BIST_BIST_EN_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    HASH_CAM_BIST_BIST_EN_RESERVED0_FIELD_MASK,
    0,
    HASH_CAM_BIST_BIST_EN_RESERVED0_FIELD_WIDTH,
    HASH_CAM_BIST_BIST_EN_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_CAM_BIST_BIST_MODE_VALUE
 ******************************************************************************/
const ru_field_rec HASH_CAM_BIST_BIST_MODE_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "Value",
    ".",
#endif
    HASH_CAM_BIST_BIST_MODE_VALUE_FIELD_MASK,
    0,
    HASH_CAM_BIST_BIST_MODE_VALUE_FIELD_WIDTH,
    HASH_CAM_BIST_BIST_MODE_VALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_CAM_BIST_BIST_MODE_RESERVED0
 ******************************************************************************/
const ru_field_rec HASH_CAM_BIST_BIST_MODE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    HASH_CAM_BIST_BIST_MODE_RESERVED0_FIELD_MASK,
    0,
    HASH_CAM_BIST_BIST_MODE_RESERVED0_FIELD_WIDTH,
    HASH_CAM_BIST_BIST_MODE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_CAM_BIST_BIST_RST_L_VALUE
 ******************************************************************************/
const ru_field_rec HASH_CAM_BIST_BIST_RST_L_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "Value",
    ".",
#endif
    HASH_CAM_BIST_BIST_RST_L_VALUE_FIELD_MASK,
    0,
    HASH_CAM_BIST_BIST_RST_L_VALUE_FIELD_WIDTH,
    HASH_CAM_BIST_BIST_RST_L_VALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_CAM_BIST_BIST_RST_L_RESERVED0
 ******************************************************************************/
const ru_field_rec HASH_CAM_BIST_BIST_RST_L_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    HASH_CAM_BIST_BIST_RST_L_RESERVED0_FIELD_MASK,
    0,
    HASH_CAM_BIST_BIST_RST_L_RESERVED0_FIELD_WIDTH,
    HASH_CAM_BIST_BIST_RST_L_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_CAM_BIST_BIST_SKIP_ERROR_CNT_VALUE
 ******************************************************************************/
const ru_field_rec HASH_CAM_BIST_BIST_SKIP_ERROR_CNT_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "Value",
    ".",
#endif
    HASH_CAM_BIST_BIST_SKIP_ERROR_CNT_VALUE_FIELD_MASK,
    0,
    HASH_CAM_BIST_BIST_SKIP_ERROR_CNT_VALUE_FIELD_WIDTH,
    HASH_CAM_BIST_BIST_SKIP_ERROR_CNT_VALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_CAM_BIST_BIST_SKIP_ERROR_CNT_RESERVED0
 ******************************************************************************/
const ru_field_rec HASH_CAM_BIST_BIST_SKIP_ERROR_CNT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    HASH_CAM_BIST_BIST_SKIP_ERROR_CNT_RESERVED0_FIELD_MASK,
    0,
    HASH_CAM_BIST_BIST_SKIP_ERROR_CNT_RESERVED0_FIELD_WIDTH,
    HASH_CAM_BIST_BIST_SKIP_ERROR_CNT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_CAM_BIST_DBG_EN_VALUE
 ******************************************************************************/
const ru_field_rec HASH_CAM_BIST_DBG_EN_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "Value",
    ".",
#endif
    HASH_CAM_BIST_DBG_EN_VALUE_FIELD_MASK,
    0,
    HASH_CAM_BIST_DBG_EN_VALUE_FIELD_WIDTH,
    HASH_CAM_BIST_DBG_EN_VALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_CAM_BIST_DBG_EN_RESERVED0
 ******************************************************************************/
const ru_field_rec HASH_CAM_BIST_DBG_EN_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    HASH_CAM_BIST_DBG_EN_RESERVED0_FIELD_MASK,
    0,
    HASH_CAM_BIST_DBG_EN_RESERVED0_FIELD_WIDTH,
    HASH_CAM_BIST_DBG_EN_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_CAM_BIST_BIST_CASCADE_SELECT_VALUE
 ******************************************************************************/
const ru_field_rec HASH_CAM_BIST_BIST_CASCADE_SELECT_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "Value",
    ".",
#endif
    HASH_CAM_BIST_BIST_CASCADE_SELECT_VALUE_FIELD_MASK,
    0,
    HASH_CAM_BIST_BIST_CASCADE_SELECT_VALUE_FIELD_WIDTH,
    HASH_CAM_BIST_BIST_CASCADE_SELECT_VALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_CAM_BIST_BIST_CASCADE_SELECT_RESERVED0
 ******************************************************************************/
const ru_field_rec HASH_CAM_BIST_BIST_CASCADE_SELECT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    HASH_CAM_BIST_BIST_CASCADE_SELECT_RESERVED0_FIELD_MASK,
    0,
    HASH_CAM_BIST_BIST_CASCADE_SELECT_RESERVED0_FIELD_WIDTH,
    HASH_CAM_BIST_BIST_CASCADE_SELECT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_CAM_BIST_BIST_BLOCK_SELECT_VALUE
 ******************************************************************************/
const ru_field_rec HASH_CAM_BIST_BIST_BLOCK_SELECT_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "Value",
    ".",
#endif
    HASH_CAM_BIST_BIST_BLOCK_SELECT_VALUE_FIELD_MASK,
    0,
    HASH_CAM_BIST_BIST_BLOCK_SELECT_VALUE_FIELD_WIDTH,
    HASH_CAM_BIST_BIST_BLOCK_SELECT_VALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_CAM_BIST_BIST_BLOCK_SELECT_RESERVED0
 ******************************************************************************/
const ru_field_rec HASH_CAM_BIST_BIST_BLOCK_SELECT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    HASH_CAM_BIST_BIST_BLOCK_SELECT_RESERVED0_FIELD_MASK,
    0,
    HASH_CAM_BIST_BIST_BLOCK_SELECT_RESERVED0_FIELD_WIDTH,
    HASH_CAM_BIST_BIST_BLOCK_SELECT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_CAM_BIST_BIST_REPAIR_ENABLE_VALUE
 ******************************************************************************/
const ru_field_rec HASH_CAM_BIST_BIST_REPAIR_ENABLE_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "Value",
    ".",
#endif
    HASH_CAM_BIST_BIST_REPAIR_ENABLE_VALUE_FIELD_MASK,
    0,
    HASH_CAM_BIST_BIST_REPAIR_ENABLE_VALUE_FIELD_WIDTH,
    HASH_CAM_BIST_BIST_REPAIR_ENABLE_VALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_CAM_BIST_BIST_REPAIR_ENABLE_RESERVED0
 ******************************************************************************/
const ru_field_rec HASH_CAM_BIST_BIST_REPAIR_ENABLE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    HASH_CAM_BIST_BIST_REPAIR_ENABLE_RESERVED0_FIELD_MASK,
    0,
    HASH_CAM_BIST_BIST_REPAIR_ENABLE_RESERVED0_FIELD_WIDTH,
    HASH_CAM_BIST_BIST_REPAIR_ENABLE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_INTR_CTRL_ISR_INVLD_CMD
 ******************************************************************************/
const ru_field_rec HASH_INTR_CTRL_ISR_INVLD_CMD_FIELD =
{
    "INVLD_CMD",
#if RU_INCLUDE_DESC
    "invalid_cmd",
    "Command cfg field is invalid (equals to 0)",
#endif
    HASH_INTR_CTRL_ISR_INVLD_CMD_FIELD_MASK,
    0,
    HASH_INTR_CTRL_ISR_INVLD_CMD_FIELD_WIDTH,
    HASH_INTR_CTRL_ISR_INVLD_CMD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_INTR_CTRL_ISR_MULT_MATCH
 ******************************************************************************/
const ru_field_rec HASH_INTR_CTRL_ISR_MULT_MATCH_FIELD =
{
    "MULT_MATCH",
#if RU_INCLUDE_DESC
    "multiple_match",
    "During the search process same key was found a valid in multiple engines.",
#endif
    HASH_INTR_CTRL_ISR_MULT_MATCH_FIELD_MASK,
    0,
    HASH_INTR_CTRL_ISR_MULT_MATCH_FIELD_WIDTH,
    HASH_INTR_CTRL_ISR_MULT_MATCH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_INTR_CTRL_ISR_HASH_0_IDX_OVFLV
 ******************************************************************************/
const ru_field_rec HASH_INTR_CTRL_ISR_HASH_0_IDX_OVFLV_FIELD =
{
    "HASH_0_IDX_OVFLV",
#if RU_INCLUDE_DESC
    "hash_0_idx_overflow",
    "hash table index over flow at hash engine",
#endif
    HASH_INTR_CTRL_ISR_HASH_0_IDX_OVFLV_FIELD_MASK,
    0,
    HASH_INTR_CTRL_ISR_HASH_0_IDX_OVFLV_FIELD_WIDTH,
    HASH_INTR_CTRL_ISR_HASH_0_IDX_OVFLV_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_INTR_CTRL_ISR_HASH_1_IDX_OVFLV
 ******************************************************************************/
const ru_field_rec HASH_INTR_CTRL_ISR_HASH_1_IDX_OVFLV_FIELD =
{
    "HASH_1_IDX_OVFLV",
#if RU_INCLUDE_DESC
    "hash_1_idx_overflow",
    "hash table over flow at hash engine",
#endif
    HASH_INTR_CTRL_ISR_HASH_1_IDX_OVFLV_FIELD_MASK,
    0,
    HASH_INTR_CTRL_ISR_HASH_1_IDX_OVFLV_FIELD_WIDTH,
    HASH_INTR_CTRL_ISR_HASH_1_IDX_OVFLV_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_INTR_CTRL_ISR_HASH_2_IDX_OVFLV
 ******************************************************************************/
const ru_field_rec HASH_INTR_CTRL_ISR_HASH_2_IDX_OVFLV_FIELD =
{
    "HASH_2_IDX_OVFLV",
#if RU_INCLUDE_DESC
    "hash_2_idx_overflow",
    "hash table index over flow at hash engine",
#endif
    HASH_INTR_CTRL_ISR_HASH_2_IDX_OVFLV_FIELD_MASK,
    0,
    HASH_INTR_CTRL_ISR_HASH_2_IDX_OVFLV_FIELD_WIDTH,
    HASH_INTR_CTRL_ISR_HASH_2_IDX_OVFLV_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_INTR_CTRL_ISR_HASH_3_IDX_OVFLV
 ******************************************************************************/
const ru_field_rec HASH_INTR_CTRL_ISR_HASH_3_IDX_OVFLV_FIELD =
{
    "HASH_3_IDX_OVFLV",
#if RU_INCLUDE_DESC
    "hash_3_idx_overflow",
    "hash table index over flow at hash engine",
#endif
    HASH_INTR_CTRL_ISR_HASH_3_IDX_OVFLV_FIELD_MASK,
    0,
    HASH_INTR_CTRL_ISR_HASH_3_IDX_OVFLV_FIELD_WIDTH,
    HASH_INTR_CTRL_ISR_HASH_3_IDX_OVFLV_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_INTR_CTRL_ISR_CNTXT_IDX_OVFLV
 ******************************************************************************/
const ru_field_rec HASH_INTR_CTRL_ISR_CNTXT_IDX_OVFLV_FIELD =
{
    "CNTXT_IDX_OVFLV",
#if RU_INCLUDE_DESC
    "cntxt_idx_overflow",
    "Context table index over flow",
#endif
    HASH_INTR_CTRL_ISR_CNTXT_IDX_OVFLV_FIELD_MASK,
    0,
    HASH_INTR_CTRL_ISR_CNTXT_IDX_OVFLV_FIELD_WIDTH,
    HASH_INTR_CTRL_ISR_CNTXT_IDX_OVFLV_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_INTR_CTRL_ISR_RESERVED0
 ******************************************************************************/
const ru_field_rec HASH_INTR_CTRL_ISR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    HASH_INTR_CTRL_ISR_RESERVED0_FIELD_MASK,
    0,
    HASH_INTR_CTRL_ISR_RESERVED0_FIELD_WIDTH,
    HASH_INTR_CTRL_ISR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_INTR_CTRL_ISM_ISM
 ******************************************************************************/
const ru_field_rec HASH_INTR_CTRL_ISM_ISM_FIELD =
{
    "ISM",
#if RU_INCLUDE_DESC
    "Interrupt_status_masked",
    "Status Masked of corresponding interrupt source in the ISR",
#endif
    HASH_INTR_CTRL_ISM_ISM_FIELD_MASK,
    0,
    HASH_INTR_CTRL_ISM_ISM_FIELD_WIDTH,
    HASH_INTR_CTRL_ISM_ISM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: HASH_INTR_CTRL_IER_IEM
 ******************************************************************************/
const ru_field_rec HASH_INTR_CTRL_IER_IEM_FIELD =
{
    "IEM",
#if RU_INCLUDE_DESC
    "Interrupt_enable_mask",
    "Each bit in the mask controls the corresponding interrupt source in the IER",
#endif
    HASH_INTR_CTRL_IER_IEM_FIELD_MASK,
    0,
    HASH_INTR_CTRL_IER_IEM_FIELD_WIDTH,
    HASH_INTR_CTRL_IER_IEM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_INTR_CTRL_ITR_IST
 ******************************************************************************/
const ru_field_rec HASH_INTR_CTRL_ITR_IST_FIELD =
{
    "IST",
#if RU_INCLUDE_DESC
    "Interrupt_simulation_test",
    "Each bit in the mask tests the corresponding interrupt source in the ISR",
#endif
    HASH_INTR_CTRL_ITR_IST_FIELD_MASK,
    0,
    HASH_INTR_CTRL_ITR_IST_FIELD_WIDTH,
    HASH_INTR_CTRL_ITR_IST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_DEBUG_DBG0_VAL
 ******************************************************************************/
const ru_field_rec HASH_DEBUG_DBG0_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "dbg",
    "read debug register",
#endif
    HASH_DEBUG_DBG0_VAL_FIELD_MASK,
    0,
    HASH_DEBUG_DBG0_VAL_FIELD_WIDTH,
    HASH_DEBUG_DBG0_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: HASH_DEBUG_DBG1_VAL
 ******************************************************************************/
const ru_field_rec HASH_DEBUG_DBG1_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "dbg",
    "read debug register",
#endif
    HASH_DEBUG_DBG1_VAL_FIELD_MASK,
    0,
    HASH_DEBUG_DBG1_VAL_FIELD_WIDTH,
    HASH_DEBUG_DBG1_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: HASH_DEBUG_DBG2_VAL
 ******************************************************************************/
const ru_field_rec HASH_DEBUG_DBG2_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "dbg",
    "read debug register",
#endif
    HASH_DEBUG_DBG2_VAL_FIELD_MASK,
    0,
    HASH_DEBUG_DBG2_VAL_FIELD_WIDTH,
    HASH_DEBUG_DBG2_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: HASH_DEBUG_DBG3_VAL
 ******************************************************************************/
const ru_field_rec HASH_DEBUG_DBG3_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "dbg",
    "read debug register",
#endif
    HASH_DEBUG_DBG3_VAL_FIELD_MASK,
    0,
    HASH_DEBUG_DBG3_VAL_FIELD_WIDTH,
    HASH_DEBUG_DBG3_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: HASH_DEBUG_DBG4_VAL
 ******************************************************************************/
const ru_field_rec HASH_DEBUG_DBG4_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "dbg",
    "read debug register",
#endif
    HASH_DEBUG_DBG4_VAL_FIELD_MASK,
    0,
    HASH_DEBUG_DBG4_VAL_FIELD_WIDTH,
    HASH_DEBUG_DBG4_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: HASH_DEBUG_DBG5_VAL
 ******************************************************************************/
const ru_field_rec HASH_DEBUG_DBG5_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "dbg",
    "read debug register",
#endif
    HASH_DEBUG_DBG5_VAL_FIELD_MASK,
    0,
    HASH_DEBUG_DBG5_VAL_FIELD_WIDTH,
    HASH_DEBUG_DBG5_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: HASH_DEBUG_DBG6_VAL
 ******************************************************************************/
const ru_field_rec HASH_DEBUG_DBG6_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "dbg",
    "read debug register",
#endif
    HASH_DEBUG_DBG6_VAL_FIELD_MASK,
    0,
    HASH_DEBUG_DBG6_VAL_FIELD_WIDTH,
    HASH_DEBUG_DBG6_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: HASH_DEBUG_DBG7_VAL
 ******************************************************************************/
const ru_field_rec HASH_DEBUG_DBG7_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "dbg",
    "read debug register",
#endif
    HASH_DEBUG_DBG7_VAL_FIELD_MASK,
    0,
    HASH_DEBUG_DBG7_VAL_FIELD_WIDTH,
    HASH_DEBUG_DBG7_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: HASH_DEBUG_DBG8_VAL
 ******************************************************************************/
const ru_field_rec HASH_DEBUG_DBG8_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "dbg",
    "read debug register",
#endif
    HASH_DEBUG_DBG8_VAL_FIELD_MASK,
    0,
    HASH_DEBUG_DBG8_VAL_FIELD_WIDTH,
    HASH_DEBUG_DBG8_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: HASH_DEBUG_DBG9_VAL
 ******************************************************************************/
const ru_field_rec HASH_DEBUG_DBG9_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "dbg",
    "read debug register",
#endif
    HASH_DEBUG_DBG9_VAL_FIELD_MASK,
    0,
    HASH_DEBUG_DBG9_VAL_FIELD_WIDTH,
    HASH_DEBUG_DBG9_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: HASH_DEBUG_DBG10_VAL
 ******************************************************************************/
const ru_field_rec HASH_DEBUG_DBG10_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "dbg",
    "read debug register",
#endif
    HASH_DEBUG_DBG10_VAL_FIELD_MASK,
    0,
    HASH_DEBUG_DBG10_VAL_FIELD_WIDTH,
    HASH_DEBUG_DBG10_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: HASH_DEBUG_DBG11_VAL
 ******************************************************************************/
const ru_field_rec HASH_DEBUG_DBG11_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "dbg",
    "read debug register",
#endif
    HASH_DEBUG_DBG11_VAL_FIELD_MASK,
    0,
    HASH_DEBUG_DBG11_VAL_FIELD_WIDTH,
    HASH_DEBUG_DBG11_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: HASH_DEBUG_DBG12_VAL
 ******************************************************************************/
const ru_field_rec HASH_DEBUG_DBG12_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "dbg",
    "read debug register",
#endif
    HASH_DEBUG_DBG12_VAL_FIELD_MASK,
    0,
    HASH_DEBUG_DBG12_VAL_FIELD_WIDTH,
    HASH_DEBUG_DBG12_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: HASH_DEBUG_DBG13_VAL
 ******************************************************************************/
const ru_field_rec HASH_DEBUG_DBG13_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "dbg",
    "read debug register",
#endif
    HASH_DEBUG_DBG13_VAL_FIELD_MASK,
    0,
    HASH_DEBUG_DBG13_VAL_FIELD_WIDTH,
    HASH_DEBUG_DBG13_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: HASH_DEBUG_DBG14_VAL
 ******************************************************************************/
const ru_field_rec HASH_DEBUG_DBG14_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "dbg",
    "read debug register",
#endif
    HASH_DEBUG_DBG14_VAL_FIELD_MASK,
    0,
    HASH_DEBUG_DBG14_VAL_FIELD_WIDTH,
    HASH_DEBUG_DBG14_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: HASH_DEBUG_DBG15_VAL
 ******************************************************************************/
const ru_field_rec HASH_DEBUG_DBG15_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "dbg",
    "read debug register",
#endif
    HASH_DEBUG_DBG15_VAL_FIELD_MASK,
    0,
    HASH_DEBUG_DBG15_VAL_FIELD_WIDTH,
    HASH_DEBUG_DBG15_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: HASH_DEBUG_DBG16_VAL
 ******************************************************************************/
const ru_field_rec HASH_DEBUG_DBG16_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "dbg",
    "read debug register",
#endif
    HASH_DEBUG_DBG16_VAL_FIELD_MASK,
    0,
    HASH_DEBUG_DBG16_VAL_FIELD_WIDTH,
    HASH_DEBUG_DBG16_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: HASH_DEBUG_DBG17_VAL
 ******************************************************************************/
const ru_field_rec HASH_DEBUG_DBG17_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "dbg",
    "read debug register",
#endif
    HASH_DEBUG_DBG17_VAL_FIELD_MASK,
    0,
    HASH_DEBUG_DBG17_VAL_FIELD_WIDTH,
    HASH_DEBUG_DBG17_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: HASH_DEBUG_DBG18_VAL
 ******************************************************************************/
const ru_field_rec HASH_DEBUG_DBG18_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "dbg",
    "read debug register",
#endif
    HASH_DEBUG_DBG18_VAL_FIELD_MASK,
    0,
    HASH_DEBUG_DBG18_VAL_FIELD_WIDTH,
    HASH_DEBUG_DBG18_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: HASH_DEBUG_DBG19_VAL
 ******************************************************************************/
const ru_field_rec HASH_DEBUG_DBG19_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "dbg",
    "read debug register",
#endif
    HASH_DEBUG_DBG19_VAL_FIELD_MASK,
    0,
    HASH_DEBUG_DBG19_VAL_FIELD_WIDTH,
    HASH_DEBUG_DBG19_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: HASH_DEBUG_DBG20_VAL
 ******************************************************************************/
const ru_field_rec HASH_DEBUG_DBG20_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "dbg",
    "read debug register",
#endif
    HASH_DEBUG_DBG20_VAL_FIELD_MASK,
    0,
    HASH_DEBUG_DBG20_VAL_FIELD_WIDTH,
    HASH_DEBUG_DBG20_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: HASH_DEBUG_DBG_SEL_VAL
 ******************************************************************************/
const ru_field_rec HASH_DEBUG_DBG_SEL_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "dbg_select",
    "debug sel",
#endif
    HASH_DEBUG_DBG_SEL_VAL_FIELD_MASK,
    0,
    HASH_DEBUG_DBG_SEL_VAL_FIELD_WIDTH,
    HASH_DEBUG_DBG_SEL_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_DEBUG_DBG_SEL_RESERVED0
 ******************************************************************************/
const ru_field_rec HASH_DEBUG_DBG_SEL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    HASH_DEBUG_DBG_SEL_RESERVED0_FIELD_MASK,
    0,
    HASH_DEBUG_DBG_SEL_RESERVED0_FIELD_WIDTH,
    HASH_DEBUG_DBG_SEL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_AGING_RAM_AGING_DATA
 ******************************************************************************/
const ru_field_rec HASH_AGING_RAM_AGING_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA",
    ".",
#endif
    HASH_AGING_RAM_AGING_DATA_FIELD_MASK,
    0,
    HASH_AGING_RAM_AGING_DATA_FIELD_WIDTH,
    HASH_AGING_RAM_AGING_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_CONTEXT_RAM_CONTEXT_47_24_DATA
 ******************************************************************************/
const ru_field_rec HASH_CONTEXT_RAM_CONTEXT_47_24_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA",
    ".",
#endif
    HASH_CONTEXT_RAM_CONTEXT_47_24_DATA_FIELD_MASK,
    0,
    HASH_CONTEXT_RAM_CONTEXT_47_24_DATA_FIELD_WIDTH,
    HASH_CONTEXT_RAM_CONTEXT_47_24_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_CONTEXT_RAM_CONTEXT_47_24_RESERVED0
 ******************************************************************************/
const ru_field_rec HASH_CONTEXT_RAM_CONTEXT_47_24_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    HASH_CONTEXT_RAM_CONTEXT_47_24_RESERVED0_FIELD_MASK,
    0,
    HASH_CONTEXT_RAM_CONTEXT_47_24_RESERVED0_FIELD_WIDTH,
    HASH_CONTEXT_RAM_CONTEXT_47_24_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_CONTEXT_RAM_CONTEXT_23_0_DATA
 ******************************************************************************/
const ru_field_rec HASH_CONTEXT_RAM_CONTEXT_23_0_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA",
    ".",
#endif
    HASH_CONTEXT_RAM_CONTEXT_23_0_DATA_FIELD_MASK,
    0,
    HASH_CONTEXT_RAM_CONTEXT_23_0_DATA_FIELD_WIDTH,
    HASH_CONTEXT_RAM_CONTEXT_23_0_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_CONTEXT_RAM_CONTEXT_23_0_RESERVED0
 ******************************************************************************/
const ru_field_rec HASH_CONTEXT_RAM_CONTEXT_23_0_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    HASH_CONTEXT_RAM_CONTEXT_23_0_RESERVED0_FIELD_MASK,
    0,
    HASH_CONTEXT_RAM_CONTEXT_23_0_RESERVED0_FIELD_WIDTH,
    HASH_CONTEXT_RAM_CONTEXT_23_0_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_RAM_ENG_HIGH_KEY_59_28_OR_DAT
 ******************************************************************************/
const ru_field_rec HASH_RAM_ENG_HIGH_KEY_59_28_OR_DAT_FIELD =
{
    "KEY_59_28_OR_DAT",
#if RU_INCLUDE_DESC
    "key_or_data",
    "This field contains one of the two: key extension or internal context data."
    "It defined by table configuration.",
#endif
    HASH_RAM_ENG_HIGH_KEY_59_28_OR_DAT_FIELD_MASK,
    0,
    HASH_RAM_ENG_HIGH_KEY_59_28_OR_DAT_FIELD_WIDTH,
    HASH_RAM_ENG_HIGH_KEY_59_28_OR_DAT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_RAM_ENG_LOW_SKP
 ******************************************************************************/
const ru_field_rec HASH_RAM_ENG_LOW_SKP_FIELD =
{
    "SKP",
#if RU_INCLUDE_DESC
    "skip",
    "Indicates not to search at this entry due to the ongoing update of the entry.",
#endif
    HASH_RAM_ENG_LOW_SKP_FIELD_MASK,
    0,
    HASH_RAM_ENG_LOW_SKP_FIELD_WIDTH,
    HASH_RAM_ENG_LOW_SKP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_RAM_ENG_LOW_CFG
 ******************************************************************************/
const ru_field_rec HASH_RAM_ENG_LOW_CFG_FIELD =
{
    "CFG",
#if RU_INCLUDE_DESC
    "Config",
    "Determines the table config number, between 1-7."
    ""
    "Config 0 is used to indicate invalid entry",
#endif
    HASH_RAM_ENG_LOW_CFG_FIELD_MASK,
    0,
    HASH_RAM_ENG_LOW_CFG_FIELD_WIDTH,
    HASH_RAM_ENG_LOW_CFG_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_RAM_ENG_LOW_KEY_11_0
 ******************************************************************************/
const ru_field_rec HASH_RAM_ENG_LOW_KEY_11_0_FIELD =
{
    "KEY_11_0",
#if RU_INCLUDE_DESC
    "key_11_0",
    "Includes the first part of the key."
    "This field is preserved for key use only.",
#endif
    HASH_RAM_ENG_LOW_KEY_11_0_FIELD_MASK,
    0,
    HASH_RAM_ENG_LOW_KEY_11_0_FIELD_WIDTH,
    HASH_RAM_ENG_LOW_KEY_11_0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: HASH_RAM_ENG_LOW_KEY_27_12_OR_DAT
 ******************************************************************************/
const ru_field_rec HASH_RAM_ENG_LOW_KEY_27_12_OR_DAT_FIELD =
{
    "KEY_27_12_OR_DAT",
#if RU_INCLUDE_DESC
    "key_or_data",
    "This field contains one of the two: key extension or internal context data."
    "It defined by table configuration.",
#endif
    HASH_RAM_ENG_LOW_KEY_27_12_OR_DAT_FIELD_MASK,
    0,
    HASH_RAM_ENG_LOW_KEY_27_12_OR_DAT_FIELD_WIDTH,
    HASH_RAM_ENG_LOW_KEY_27_12_OR_DAT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: HASH_GENERAL_CONFIGURATION_PWR_SAV_EN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_GENERAL_CONFIGURATION_PWR_SAV_EN_FIELDS[] =
{
    &HASH_GENERAL_CONFIGURATION_PWR_SAV_EN_VALUE_FIELD,
    &HASH_GENERAL_CONFIGURATION_PWR_SAV_EN_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    822,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    HASH_GENERAL_CONFIGURATION_PWR_SAV_EN_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_GENERAL_CONFIGURATION_PAD_HIGH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_GENERAL_CONFIGURATION_PAD_HIGH_FIELDS[] =
{
    &HASH_GENERAL_CONFIGURATION_PAD_HIGH_VAL_FIELD,
    &HASH_GENERAL_CONFIGURATION_PAD_HIGH_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    823,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    HASH_GENERAL_CONFIGURATION_PAD_HIGH_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_GENERAL_CONFIGURATION_PAD_LOW
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_GENERAL_CONFIGURATION_PAD_LOW_FIELDS[] =
{
    &HASH_GENERAL_CONFIGURATION_PAD_LOW_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    824,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_GENERAL_CONFIGURATION_PAD_LOW_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_GENERAL_CONFIGURATION_MULT_HIT_ERR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_GENERAL_CONFIGURATION_MULT_HIT_ERR_FIELDS[] =
{
    &HASH_GENERAL_CONFIGURATION_MULT_HIT_ERR_VAL_FIELD,
    &HASH_GENERAL_CONFIGURATION_MULT_HIT_ERR_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    825,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    HASH_GENERAL_CONFIGURATION_MULT_HIT_ERR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_GENERAL_CONFIGURATION_UNDO_FIX
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_GENERAL_CONFIGURATION_UNDO_FIX_FIELDS[] =
{
    &HASH_GENERAL_CONFIGURATION_UNDO_FIX_FRST_MUL_HIT_FIELD,
    &HASH_GENERAL_CONFIGURATION_UNDO_FIX_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    826,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    HASH_GENERAL_CONFIGURATION_UNDO_FIX_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_PM_COUNTERS_HITS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_PM_COUNTERS_HITS_FIELDS[] =
{
    &HASH_PM_COUNTERS_HITS_CNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    827,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_PM_COUNTERS_HITS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_PM_COUNTERS_SRCHS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_PM_COUNTERS_SRCHS_FIELDS[] =
{
    &HASH_PM_COUNTERS_SRCHS_CNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    828,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_PM_COUNTERS_SRCHS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_PM_COUNTERS_MISS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_PM_COUNTERS_MISS_FIELDS[] =
{
    &HASH_PM_COUNTERS_MISS_CNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    829,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_PM_COUNTERS_MISS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_PM_COUNTERS_HIT_1ST_ACS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_PM_COUNTERS_HIT_1ST_ACS_FIELDS[] =
{
    &HASH_PM_COUNTERS_HIT_1ST_ACS_CNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    830,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_PM_COUNTERS_HIT_1ST_ACS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_PM_COUNTERS_HIT_2ND_ACS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_PM_COUNTERS_HIT_2ND_ACS_FIELDS[] =
{
    &HASH_PM_COUNTERS_HIT_2ND_ACS_CNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    831,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_PM_COUNTERS_HIT_2ND_ACS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_PM_COUNTERS_HIT_3RD_ACS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_PM_COUNTERS_HIT_3RD_ACS_FIELDS[] =
{
    &HASH_PM_COUNTERS_HIT_3RD_ACS_CNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    832,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_PM_COUNTERS_HIT_3RD_ACS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_PM_COUNTERS_HIT_4TH_ACS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_PM_COUNTERS_HIT_4TH_ACS_FIELDS[] =
{
    &HASH_PM_COUNTERS_HIT_4TH_ACS_CNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    833,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_PM_COUNTERS_HIT_4TH_ACS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_PM_COUNTERS_FRZ_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_PM_COUNTERS_FRZ_CNT_FIELDS[] =
{
    &HASH_PM_COUNTERS_FRZ_CNT_VAL_FIELD,
    &HASH_PM_COUNTERS_FRZ_CNT_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    834,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    HASH_PM_COUNTERS_FRZ_CNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_LKUP_TBL_CFG_TBL_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_LKUP_TBL_CFG_TBL_CFG_FIELDS[] =
{
    &HASH_LKUP_TBL_CFG_TBL_CFG_HASH_BASE_ADDR_FIELD,
    &HASH_LKUP_TBL_CFG_TBL_CFG_RESERVED0_FIELD,
    &HASH_LKUP_TBL_CFG_TBL_CFG_TBL_SIZE_FIELD,
    &HASH_LKUP_TBL_CFG_TBL_CFG_RESERVED1_FIELD,
    &HASH_LKUP_TBL_CFG_TBL_CFG_MAX_HOP_FIELD,
    &HASH_LKUP_TBL_CFG_TBL_CFG_CAM_EN_FIELD,
    &HASH_LKUP_TBL_CFG_TBL_CFG_DIRECT_LKUP_EN_FIELD,
    &HASH_LKUP_TBL_CFG_TBL_CFG_HASH_TYPE_FIELD,
    &HASH_LKUP_TBL_CFG_TBL_CFG_RESERVED2_FIELD,
    &HASH_LKUP_TBL_CFG_TBL_CFG_INT_CNTX_SIZE_FIELD,
    &HASH_LKUP_TBL_CFG_TBL_CFG_RESERVED3_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    835,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    11,
    HASH_LKUP_TBL_CFG_TBL_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_LKUP_TBL_CFG_KEY_MASK_HIGH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_LKUP_TBL_CFG_KEY_MASK_HIGH_FIELDS[] =
{
    &HASH_LKUP_TBL_CFG_KEY_MASK_HIGH_MASKH_FIELD,
    &HASH_LKUP_TBL_CFG_KEY_MASK_HIGH_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    836,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    HASH_LKUP_TBL_CFG_KEY_MASK_HIGH_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_LKUP_TBL_CFG_KEY_MASK_LOW
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_LKUP_TBL_CFG_KEY_MASK_LOW_FIELDS[] =
{
    &HASH_LKUP_TBL_CFG_KEY_MASK_LOW_MASKL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    837,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_LKUP_TBL_CFG_KEY_MASK_LOW_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_LKUP_TBL_CFG_CNTXT_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_LKUP_TBL_CFG_CNTXT_CFG_FIELDS[] =
{
    &HASH_LKUP_TBL_CFG_CNTXT_CFG_BASE_ADDRESS_FIELD,
    &HASH_LKUP_TBL_CFG_CNTXT_CFG_FIRST_HASH_IDX_FIELD,
    &HASH_LKUP_TBL_CFG_CNTXT_CFG_RESERVED0_FIELD,
    &HASH_LKUP_TBL_CFG_CNTXT_CFG_CNXT_SIZE_FIELD,
    &HASH_LKUP_TBL_CFG_CNTXT_CFG_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    838,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    HASH_LKUP_TBL_CFG_CNTXT_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_CAM_CONFIGURATION_CNTXT_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_CONFIGURATION_CNTXT_CFG_FIELDS[] =
{
    &HASH_CAM_CONFIGURATION_CNTXT_CFG_BASE_ADDRESS_FIELD,
    &HASH_CAM_CONFIGURATION_CNTXT_CFG_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    839,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    HASH_CAM_CONFIGURATION_CNTXT_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_CAM_INDIRECT_OP
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_INDIRECT_OP_FIELDS[] =
{
    &HASH_CAM_INDIRECT_OP_CMD_FIELD,
    &HASH_CAM_INDIRECT_OP_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    840,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    HASH_CAM_INDIRECT_OP_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_CAM_INDIRECT_OP_DONE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_INDIRECT_OP_DONE_FIELDS[] =
{
    &HASH_CAM_INDIRECT_OP_DONE_VAL_FIELD,
    &HASH_CAM_INDIRECT_OP_DONE_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    841,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    HASH_CAM_INDIRECT_OP_DONE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_CAM_INDIRECT_ADDR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_INDIRECT_ADDR_FIELDS[] =
{
    &HASH_CAM_INDIRECT_ADDR_KEY1_IND_FIELD,
    &HASH_CAM_INDIRECT_ADDR_ENTRY_ADDR_FIELD,
    &HASH_CAM_INDIRECT_ADDR_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    842,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    HASH_CAM_INDIRECT_ADDR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_CAM_INDIRECT_VLID_IN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_INDIRECT_VLID_IN_FIELDS[] =
{
    &HASH_CAM_INDIRECT_VLID_IN_VALID_FIELD,
    &HASH_CAM_INDIRECT_VLID_IN_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    843,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    HASH_CAM_INDIRECT_VLID_IN_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_CAM_INDIRECT_VLID_OUT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_INDIRECT_VLID_OUT_FIELDS[] =
{
    &HASH_CAM_INDIRECT_VLID_OUT_VALID_FIELD,
    &HASH_CAM_INDIRECT_VLID_OUT_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    844,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    HASH_CAM_INDIRECT_VLID_OUT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_CAM_INDIRECT_RSLT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_INDIRECT_RSLT_FIELDS[] =
{
    &HASH_CAM_INDIRECT_RSLT_MATCH_FIELD,
    &HASH_CAM_INDIRECT_RSLT_RESERVED0_FIELD,
    &HASH_CAM_INDIRECT_RSLT_INDEX_FIELD,
    &HASH_CAM_INDIRECT_RSLT_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    845,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    HASH_CAM_INDIRECT_RSLT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_CAM_INDIRECT_KEY_IN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_INDIRECT_KEY_IN_FIELDS[] =
{
    &HASH_CAM_INDIRECT_KEY_IN_VALUE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    846,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_CAM_INDIRECT_KEY_IN_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_CAM_INDIRECT_KEY_OUT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_INDIRECT_KEY_OUT_FIELDS[] =
{
    &HASH_CAM_INDIRECT_KEY_OUT_VALUE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    847,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_CAM_INDIRECT_KEY_OUT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_CAM_BIST_BIST_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_BIST_BIST_STATUS_FIELDS[] =
{
    &HASH_CAM_BIST_BIST_STATUS_VALUE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    848,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_CAM_BIST_BIST_STATUS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_CAM_BIST_BIST_DBG_COMPARE_EN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_BIST_BIST_DBG_COMPARE_EN_FIELDS[] =
{
    &HASH_CAM_BIST_BIST_DBG_COMPARE_EN_VALUE_FIELD,
    &HASH_CAM_BIST_BIST_DBG_COMPARE_EN_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    849,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    HASH_CAM_BIST_BIST_DBG_COMPARE_EN_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_CAM_BIST_BIST_DBG_DATA
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_BIST_BIST_DBG_DATA_FIELDS[] =
{
    &HASH_CAM_BIST_BIST_DBG_DATA_VALUE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    850,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_CAM_BIST_BIST_DBG_DATA_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_CAM_BIST_BIST_DBG_DATA_SLICE_OR_STATUS_SEL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_BIST_BIST_DBG_DATA_SLICE_OR_STATUS_SEL_FIELDS[] =
{
    &HASH_CAM_BIST_BIST_DBG_DATA_SLICE_OR_STATUS_SEL_VALUE_FIELD,
    &HASH_CAM_BIST_BIST_DBG_DATA_SLICE_OR_STATUS_SEL_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    851,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    HASH_CAM_BIST_BIST_DBG_DATA_SLICE_OR_STATUS_SEL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_CAM_BIST_BIST_DBG_DATA_VALID
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_BIST_BIST_DBG_DATA_VALID_FIELDS[] =
{
    &HASH_CAM_BIST_BIST_DBG_DATA_VALID_VALUE_FIELD,
    &HASH_CAM_BIST_BIST_DBG_DATA_VALID_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    852,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    HASH_CAM_BIST_BIST_DBG_DATA_VALID_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_CAM_BIST_BIST_EN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_BIST_BIST_EN_FIELDS[] =
{
    &HASH_CAM_BIST_BIST_EN_VALUE_FIELD,
    &HASH_CAM_BIST_BIST_EN_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    853,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    HASH_CAM_BIST_BIST_EN_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_CAM_BIST_BIST_MODE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_BIST_BIST_MODE_FIELDS[] =
{
    &HASH_CAM_BIST_BIST_MODE_VALUE_FIELD,
    &HASH_CAM_BIST_BIST_MODE_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    854,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    HASH_CAM_BIST_BIST_MODE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_CAM_BIST_BIST_RST_L
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_BIST_BIST_RST_L_FIELDS[] =
{
    &HASH_CAM_BIST_BIST_RST_L_VALUE_FIELD,
    &HASH_CAM_BIST_BIST_RST_L_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    855,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    HASH_CAM_BIST_BIST_RST_L_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_CAM_BIST_BIST_SKIP_ERROR_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_BIST_BIST_SKIP_ERROR_CNT_FIELDS[] =
{
    &HASH_CAM_BIST_BIST_SKIP_ERROR_CNT_VALUE_FIELD,
    &HASH_CAM_BIST_BIST_SKIP_ERROR_CNT_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    856,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    HASH_CAM_BIST_BIST_SKIP_ERROR_CNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_CAM_BIST_DBG_EN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_BIST_DBG_EN_FIELDS[] =
{
    &HASH_CAM_BIST_DBG_EN_VALUE_FIELD,
    &HASH_CAM_BIST_DBG_EN_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    857,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    HASH_CAM_BIST_DBG_EN_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_CAM_BIST_BIST_CASCADE_SELECT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_BIST_BIST_CASCADE_SELECT_FIELDS[] =
{
    &HASH_CAM_BIST_BIST_CASCADE_SELECT_VALUE_FIELD,
    &HASH_CAM_BIST_BIST_CASCADE_SELECT_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    858,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    HASH_CAM_BIST_BIST_CASCADE_SELECT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_CAM_BIST_BIST_BLOCK_SELECT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_BIST_BIST_BLOCK_SELECT_FIELDS[] =
{
    &HASH_CAM_BIST_BIST_BLOCK_SELECT_VALUE_FIELD,
    &HASH_CAM_BIST_BIST_BLOCK_SELECT_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    859,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    HASH_CAM_BIST_BIST_BLOCK_SELECT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_CAM_BIST_BIST_REPAIR_ENABLE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_BIST_BIST_REPAIR_ENABLE_FIELDS[] =
{
    &HASH_CAM_BIST_BIST_REPAIR_ENABLE_VALUE_FIELD,
    &HASH_CAM_BIST_BIST_REPAIR_ENABLE_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    860,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    HASH_CAM_BIST_BIST_REPAIR_ENABLE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_INTR_CTRL_ISR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_INTR_CTRL_ISR_FIELDS[] =
{
    &HASH_INTR_CTRL_ISR_INVLD_CMD_FIELD,
    &HASH_INTR_CTRL_ISR_MULT_MATCH_FIELD,
    &HASH_INTR_CTRL_ISR_HASH_0_IDX_OVFLV_FIELD,
    &HASH_INTR_CTRL_ISR_HASH_1_IDX_OVFLV_FIELD,
    &HASH_INTR_CTRL_ISR_HASH_2_IDX_OVFLV_FIELD,
    &HASH_INTR_CTRL_ISR_HASH_3_IDX_OVFLV_FIELD,
    &HASH_INTR_CTRL_ISR_CNTXT_IDX_OVFLV_FIELD,
    &HASH_INTR_CTRL_ISR_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    861,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    HASH_INTR_CTRL_ISR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_INTR_CTRL_ISM
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_INTR_CTRL_ISM_FIELDS[] =
{
    &HASH_INTR_CTRL_ISM_ISM_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    862,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_INTR_CTRL_ISM_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_INTR_CTRL_IER
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_INTR_CTRL_IER_FIELDS[] =
{
    &HASH_INTR_CTRL_IER_IEM_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    863,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_INTR_CTRL_IER_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_INTR_CTRL_ITR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_INTR_CTRL_ITR_FIELDS[] =
{
    &HASH_INTR_CTRL_ITR_IST_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    864,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_INTR_CTRL_ITR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_DEBUG_DBG0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_DEBUG_DBG0_FIELDS[] =
{
    &HASH_DEBUG_DBG0_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    865,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_DEBUG_DBG0_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_DEBUG_DBG1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_DEBUG_DBG1_FIELDS[] =
{
    &HASH_DEBUG_DBG1_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    866,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_DEBUG_DBG1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_DEBUG_DBG2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_DEBUG_DBG2_FIELDS[] =
{
    &HASH_DEBUG_DBG2_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    867,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_DEBUG_DBG2_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_DEBUG_DBG3
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_DEBUG_DBG3_FIELDS[] =
{
    &HASH_DEBUG_DBG3_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    868,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_DEBUG_DBG3_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_DEBUG_DBG4
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_DEBUG_DBG4_FIELDS[] =
{
    &HASH_DEBUG_DBG4_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    869,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_DEBUG_DBG4_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_DEBUG_DBG5
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_DEBUG_DBG5_FIELDS[] =
{
    &HASH_DEBUG_DBG5_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    870,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_DEBUG_DBG5_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_DEBUG_DBG6
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_DEBUG_DBG6_FIELDS[] =
{
    &HASH_DEBUG_DBG6_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    871,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_DEBUG_DBG6_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_DEBUG_DBG7
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_DEBUG_DBG7_FIELDS[] =
{
    &HASH_DEBUG_DBG7_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    872,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_DEBUG_DBG7_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_DEBUG_DBG8
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_DEBUG_DBG8_FIELDS[] =
{
    &HASH_DEBUG_DBG8_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    873,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_DEBUG_DBG8_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_DEBUG_DBG9
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_DEBUG_DBG9_FIELDS[] =
{
    &HASH_DEBUG_DBG9_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    874,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_DEBUG_DBG9_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_DEBUG_DBG10
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_DEBUG_DBG10_FIELDS[] =
{
    &HASH_DEBUG_DBG10_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    875,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_DEBUG_DBG10_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_DEBUG_DBG11
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_DEBUG_DBG11_FIELDS[] =
{
    &HASH_DEBUG_DBG11_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    876,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_DEBUG_DBG11_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_DEBUG_DBG12
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_DEBUG_DBG12_FIELDS[] =
{
    &HASH_DEBUG_DBG12_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    877,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_DEBUG_DBG12_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_DEBUG_DBG13
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_DEBUG_DBG13_FIELDS[] =
{
    &HASH_DEBUG_DBG13_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    878,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_DEBUG_DBG13_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_DEBUG_DBG14
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_DEBUG_DBG14_FIELDS[] =
{
    &HASH_DEBUG_DBG14_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    879,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_DEBUG_DBG14_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_DEBUG_DBG15
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_DEBUG_DBG15_FIELDS[] =
{
    &HASH_DEBUG_DBG15_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    880,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_DEBUG_DBG15_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_DEBUG_DBG16
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_DEBUG_DBG16_FIELDS[] =
{
    &HASH_DEBUG_DBG16_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    881,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_DEBUG_DBG16_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_DEBUG_DBG17
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_DEBUG_DBG17_FIELDS[] =
{
    &HASH_DEBUG_DBG17_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    882,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_DEBUG_DBG17_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_DEBUG_DBG18
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_DEBUG_DBG18_FIELDS[] =
{
    &HASH_DEBUG_DBG18_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    883,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_DEBUG_DBG18_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_DEBUG_DBG19
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_DEBUG_DBG19_FIELDS[] =
{
    &HASH_DEBUG_DBG19_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    884,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_DEBUG_DBG19_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_DEBUG_DBG20
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_DEBUG_DBG20_FIELDS[] =
{
    &HASH_DEBUG_DBG20_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    885,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_DEBUG_DBG20_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_DEBUG_DBG_SEL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_DEBUG_DBG_SEL_FIELDS[] =
{
    &HASH_DEBUG_DBG_SEL_VAL_FIELD,
    &HASH_DEBUG_DBG_SEL_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    886,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    HASH_DEBUG_DBG_SEL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_AGING_RAM_AGING
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_AGING_RAM_AGING_FIELDS[] =
{
    &HASH_AGING_RAM_AGING_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    887,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_AGING_RAM_AGING_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_CONTEXT_RAM_CONTEXT_47_24
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CONTEXT_RAM_CONTEXT_47_24_FIELDS[] =
{
    &HASH_CONTEXT_RAM_CONTEXT_47_24_DATA_FIELD,
    &HASH_CONTEXT_RAM_CONTEXT_47_24_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    888,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    HASH_CONTEXT_RAM_CONTEXT_47_24_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_CONTEXT_RAM_CONTEXT_23_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CONTEXT_RAM_CONTEXT_23_0_FIELDS[] =
{
    &HASH_CONTEXT_RAM_CONTEXT_23_0_DATA_FIELD,
    &HASH_CONTEXT_RAM_CONTEXT_23_0_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    889,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    HASH_CONTEXT_RAM_CONTEXT_23_0_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_RAM_ENG_HIGH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_RAM_ENG_HIGH_FIELDS[] =
{
    &HASH_RAM_ENG_HIGH_KEY_59_28_OR_DAT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    890,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_RAM_ENG_HIGH_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: HASH_RAM_ENG_LOW
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_RAM_ENG_LOW_FIELDS[] =
{
    &HASH_RAM_ENG_LOW_SKP_FIELD,
    &HASH_RAM_ENG_LOW_CFG_FIELD,
    &HASH_RAM_ENG_LOW_KEY_11_0_FIELD,
    &HASH_RAM_ENG_LOW_KEY_27_12_OR_DAT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec HASH_RAM_ENG_LOW_REG = 
{
    "RAM_ENG_LOW",
#if RU_INCLUDE_DESC
    "ENG_BITS_31_0 Register",
    "Includes the MSB field of the hash entry",
#endif
    HASH_RAM_ENG_LOW_REG_OFFSET,
    HASH_RAM_ENG_LOW_REG_RAM_CNT,
    8,
    891,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    HASH_RAM_ENG_LOW_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
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
