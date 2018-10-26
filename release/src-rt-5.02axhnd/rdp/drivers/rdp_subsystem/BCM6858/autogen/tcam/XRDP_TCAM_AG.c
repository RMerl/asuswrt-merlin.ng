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
 * Field: TCAM_CONTEXT_RAM_TCAM_TCAM_CONTEXT_RAM_CONTEXT_DATA
 ******************************************************************************/
const ru_field_rec TCAM_CONTEXT_RAM_TCAM_TCAM_CONTEXT_RAM_CONTEXT_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA",
    ".",
#endif
    TCAM_CONTEXT_RAM_TCAM_TCAM_CONTEXT_RAM_CONTEXT_DATA_FIELD_MASK,
    0,
    TCAM_CONTEXT_RAM_TCAM_TCAM_CONTEXT_RAM_CONTEXT_DATA_FIELD_WIDTH,
    TCAM_CONTEXT_RAM_TCAM_TCAM_CONTEXT_RAM_CONTEXT_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TCAM_CFG_TCAM_TCAM_CFG_BANK_EN_VALUE
 ******************************************************************************/
const ru_field_rec TCAM_CFG_TCAM_TCAM_CFG_BANK_EN_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "Value",
    ".",
#endif
    TCAM_CFG_TCAM_TCAM_CFG_BANK_EN_VALUE_FIELD_MASK,
    0,
    TCAM_CFG_TCAM_TCAM_CFG_BANK_EN_VALUE_FIELD_WIDTH,
    TCAM_CFG_TCAM_TCAM_CFG_BANK_EN_VALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TCAM_CFG_TCAM_TCAM_CFG_BANK_EN_RESERVED0
 ******************************************************************************/
const ru_field_rec TCAM_CFG_TCAM_TCAM_CFG_BANK_EN_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    TCAM_CFG_TCAM_TCAM_CFG_BANK_EN_RESERVED0_FIELD_MASK,
    0,
    TCAM_CFG_TCAM_TCAM_CFG_BANK_EN_RESERVED0_FIELD_WIDTH,
    TCAM_CFG_TCAM_TCAM_CFG_BANK_EN_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TCAM_CFG_TCAM_TCAM_CFG_GLOBAL_MASK_VALUE
 ******************************************************************************/
const ru_field_rec TCAM_CFG_TCAM_TCAM_CFG_GLOBAL_MASK_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "Value",
    ".",
#endif
    TCAM_CFG_TCAM_TCAM_CFG_GLOBAL_MASK_VALUE_FIELD_MASK,
    0,
    TCAM_CFG_TCAM_TCAM_CFG_GLOBAL_MASK_VALUE_FIELD_WIDTH,
    TCAM_CFG_TCAM_TCAM_CFG_GLOBAL_MASK_VALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TCAM_COUNTERS_TCAM_TCAM_COUNTERS_SRCH_256_CNT
 ******************************************************************************/
const ru_field_rec TCAM_COUNTERS_TCAM_TCAM_COUNTERS_SRCH_256_CNT_FIELD =
{
    "CNT",
#if RU_INCLUDE_DESC
    "CNT",
    ".",
#endif
    TCAM_COUNTERS_TCAM_TCAM_COUNTERS_SRCH_256_CNT_FIELD_MASK,
    0,
    TCAM_COUNTERS_TCAM_TCAM_COUNTERS_SRCH_256_CNT_FIELD_WIDTH,
    TCAM_COUNTERS_TCAM_TCAM_COUNTERS_SRCH_256_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: TCAM_COUNTERS_TCAM_TCAM_COUNTERS_HIT_256_CNT
 ******************************************************************************/
const ru_field_rec TCAM_COUNTERS_TCAM_TCAM_COUNTERS_HIT_256_CNT_FIELD =
{
    "CNT",
#if RU_INCLUDE_DESC
    "CNT",
    ".",
#endif
    TCAM_COUNTERS_TCAM_TCAM_COUNTERS_HIT_256_CNT_FIELD_MASK,
    0,
    TCAM_COUNTERS_TCAM_TCAM_COUNTERS_HIT_256_CNT_FIELD_WIDTH,
    TCAM_COUNTERS_TCAM_TCAM_COUNTERS_HIT_256_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: TCAM_COUNTERS_TCAM_TCAM_COUNTERS_SRCH_512_CNT
 ******************************************************************************/
const ru_field_rec TCAM_COUNTERS_TCAM_TCAM_COUNTERS_SRCH_512_CNT_FIELD =
{
    "CNT",
#if RU_INCLUDE_DESC
    "CNT",
    ".",
#endif
    TCAM_COUNTERS_TCAM_TCAM_COUNTERS_SRCH_512_CNT_FIELD_MASK,
    0,
    TCAM_COUNTERS_TCAM_TCAM_COUNTERS_SRCH_512_CNT_FIELD_WIDTH,
    TCAM_COUNTERS_TCAM_TCAM_COUNTERS_SRCH_512_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: TCAM_COUNTERS_TCAM_TCAM_COUNTERS_HIT_512_CNT
 ******************************************************************************/
const ru_field_rec TCAM_COUNTERS_TCAM_TCAM_COUNTERS_HIT_512_CNT_FIELD =
{
    "CNT",
#if RU_INCLUDE_DESC
    "CNT",
    ".",
#endif
    TCAM_COUNTERS_TCAM_TCAM_COUNTERS_HIT_512_CNT_FIELD_MASK,
    0,
    TCAM_COUNTERS_TCAM_TCAM_COUNTERS_HIT_512_CNT_FIELD_WIDTH,
    TCAM_COUNTERS_TCAM_TCAM_COUNTERS_HIT_512_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: TCAM_INDIRECT_TCAM_TCAM_INDIRECT_OP_CMD
 ******************************************************************************/
const ru_field_rec TCAM_INDIRECT_TCAM_TCAM_INDIRECT_OP_CMD_FIELD =
{
    "CMD",
#if RU_INCLUDE_DESC
    "Command",
    ".",
#endif
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_OP_CMD_FIELD_MASK,
    0,
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_OP_CMD_FIELD_WIDTH,
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_OP_CMD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TCAM_INDIRECT_TCAM_TCAM_INDIRECT_OP_RESERVED0
 ******************************************************************************/
const ru_field_rec TCAM_INDIRECT_TCAM_TCAM_INDIRECT_OP_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_OP_RESERVED0_FIELD_MASK,
    0,
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_OP_RESERVED0_FIELD_WIDTH,
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_OP_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TCAM_INDIRECT_TCAM_TCAM_INDIRECT_OP_DONE_DONE
 ******************************************************************************/
const ru_field_rec TCAM_INDIRECT_TCAM_TCAM_INDIRECT_OP_DONE_DONE_FIELD =
{
    "DONE",
#if RU_INCLUDE_DESC
    "Done",
    ".",
#endif
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_OP_DONE_DONE_FIELD_MASK,
    0,
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_OP_DONE_DONE_FIELD_WIDTH,
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_OP_DONE_DONE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: TCAM_INDIRECT_TCAM_TCAM_INDIRECT_OP_DONE_RESERVED0
 ******************************************************************************/
const ru_field_rec TCAM_INDIRECT_TCAM_TCAM_INDIRECT_OP_DONE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_OP_DONE_RESERVED0_FIELD_MASK,
    0,
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_OP_DONE_RESERVED0_FIELD_WIDTH,
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_OP_DONE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: TCAM_INDIRECT_TCAM_TCAM_INDIRECT_ADDR_KEY1_IND
 ******************************************************************************/
const ru_field_rec TCAM_INDIRECT_TCAM_TCAM_INDIRECT_ADDR_KEY1_IND_FIELD =
{
    "KEY1_IND",
#if RU_INCLUDE_DESC
    "Key1_indication",
    "This bit indicate if the operation (RD/WR) is performed on the key0 or key1 part of the entry",
#endif
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_ADDR_KEY1_IND_FIELD_MASK,
    0,
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_ADDR_KEY1_IND_FIELD_WIDTH,
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_ADDR_KEY1_IND_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TCAM_INDIRECT_TCAM_TCAM_INDIRECT_ADDR_ENTRY_ADDR
 ******************************************************************************/
const ru_field_rec TCAM_INDIRECT_TCAM_TCAM_INDIRECT_ADDR_ENTRY_ADDR_FIELD =
{
    "ENTRY_ADDR",
#if RU_INCLUDE_DESC
    "Entry_Address",
    "Address of the entry",
#endif
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_ADDR_ENTRY_ADDR_FIELD_MASK,
    0,
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_ADDR_ENTRY_ADDR_FIELD_WIDTH,
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_ADDR_ENTRY_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TCAM_INDIRECT_TCAM_TCAM_INDIRECT_ADDR_RESERVED0
 ******************************************************************************/
const ru_field_rec TCAM_INDIRECT_TCAM_TCAM_INDIRECT_ADDR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_ADDR_RESERVED0_FIELD_MASK,
    0,
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_ADDR_RESERVED0_FIELD_WIDTH,
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_ADDR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TCAM_INDIRECT_TCAM_TCAM_INDIRECT_VLID_IN_VALID
 ******************************************************************************/
const ru_field_rec TCAM_INDIRECT_TCAM_TCAM_INDIRECT_VLID_IN_VALID_FIELD =
{
    "VALID",
#if RU_INCLUDE_DESC
    "Valid",
    ".",
#endif
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_VLID_IN_VALID_FIELD_MASK,
    0,
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_VLID_IN_VALID_FIELD_WIDTH,
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_VLID_IN_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TCAM_INDIRECT_TCAM_TCAM_INDIRECT_VLID_IN_RESERVED0
 ******************************************************************************/
const ru_field_rec TCAM_INDIRECT_TCAM_TCAM_INDIRECT_VLID_IN_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_VLID_IN_RESERVED0_FIELD_MASK,
    0,
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_VLID_IN_RESERVED0_FIELD_WIDTH,
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_VLID_IN_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TCAM_INDIRECT_TCAM_TCAM_INDIRECT_VLID_OUT_VALID
 ******************************************************************************/
const ru_field_rec TCAM_INDIRECT_TCAM_TCAM_INDIRECT_VLID_OUT_VALID_FIELD =
{
    "VALID",
#if RU_INCLUDE_DESC
    "Valid",
    ".",
#endif
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_VLID_OUT_VALID_FIELD_MASK,
    0,
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_VLID_OUT_VALID_FIELD_WIDTH,
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_VLID_OUT_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TCAM_INDIRECT_TCAM_TCAM_INDIRECT_VLID_OUT_RESERVED0
 ******************************************************************************/
const ru_field_rec TCAM_INDIRECT_TCAM_TCAM_INDIRECT_VLID_OUT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_VLID_OUT_RESERVED0_FIELD_MASK,
    0,
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_VLID_OUT_RESERVED0_FIELD_WIDTH,
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_VLID_OUT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TCAM_INDIRECT_TCAM_TCAM_INDIRECT_RSLT_MATCH
 ******************************************************************************/
const ru_field_rec TCAM_INDIRECT_TCAM_TCAM_INDIRECT_RSLT_MATCH_FIELD =
{
    "MATCH",
#if RU_INCLUDE_DESC
    "match",
    "indicate if a match was found",
#endif
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_RSLT_MATCH_FIELD_MASK,
    0,
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_RSLT_MATCH_FIELD_WIDTH,
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_RSLT_MATCH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: TCAM_INDIRECT_TCAM_TCAM_INDIRECT_RSLT_RESERVED0
 ******************************************************************************/
const ru_field_rec TCAM_INDIRECT_TCAM_TCAM_INDIRECT_RSLT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_RSLT_RESERVED0_FIELD_MASK,
    0,
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_RSLT_RESERVED0_FIELD_WIDTH,
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_RSLT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: TCAM_INDIRECT_TCAM_TCAM_INDIRECT_RSLT_INDEX
 ******************************************************************************/
const ru_field_rec TCAM_INDIRECT_TCAM_TCAM_INDIRECT_RSLT_INDEX_FIELD =
{
    "INDEX",
#if RU_INCLUDE_DESC
    "index",
    "index related to a match result",
#endif
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_RSLT_INDEX_FIELD_MASK,
    0,
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_RSLT_INDEX_FIELD_WIDTH,
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_RSLT_INDEX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: TCAM_INDIRECT_TCAM_TCAM_INDIRECT_RSLT_RESERVED1
 ******************************************************************************/
const ru_field_rec TCAM_INDIRECT_TCAM_TCAM_INDIRECT_RSLT_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_RSLT_RESERVED1_FIELD_MASK,
    0,
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_RSLT_RESERVED1_FIELD_WIDTH,
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_RSLT_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: TCAM_INDIRECT_TCAM_TCAM_INDIRECT_KEY_IN_VALUE
 ******************************************************************************/
const ru_field_rec TCAM_INDIRECT_TCAM_TCAM_INDIRECT_KEY_IN_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "Value",
    ".",
#endif
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_KEY_IN_VALUE_FIELD_MASK,
    0,
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_KEY_IN_VALUE_FIELD_WIDTH,
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_KEY_IN_VALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TCAM_INDIRECT_TCAM_TCAM_INDIRECT_KEY_OUT_VALUE
 ******************************************************************************/
const ru_field_rec TCAM_INDIRECT_TCAM_TCAM_INDIRECT_KEY_OUT_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "Value",
    ".",
#endif
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_KEY_OUT_VALUE_FIELD_MASK,
    0,
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_KEY_OUT_VALUE_FIELD_WIDTH,
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_KEY_OUT_VALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: TCAM_DEBUG_BUS_TCAM_DEBUG_BUS_SELECT_SELECT_MODULE
 ******************************************************************************/
const ru_field_rec TCAM_DEBUG_BUS_TCAM_DEBUG_BUS_SELECT_SELECT_MODULE_FIELD =
{
    "SELECT_MODULE",
#if RU_INCLUDE_DESC
    "Select_Module",
    "selection"
    "",
#endif
    TCAM_DEBUG_BUS_TCAM_DEBUG_BUS_SELECT_SELECT_MODULE_FIELD_MASK,
    0,
    TCAM_DEBUG_BUS_TCAM_DEBUG_BUS_SELECT_SELECT_MODULE_FIELD_WIDTH,
    TCAM_DEBUG_BUS_TCAM_DEBUG_BUS_SELECT_SELECT_MODULE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TCAM_DEBUG_BUS_TCAM_DEBUG_BUS_SELECT_RESERVED0
 ******************************************************************************/
const ru_field_rec TCAM_DEBUG_BUS_TCAM_DEBUG_BUS_SELECT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    TCAM_DEBUG_BUS_TCAM_DEBUG_BUS_SELECT_RESERVED0_FIELD_MASK,
    0,
    TCAM_DEBUG_BUS_TCAM_DEBUG_BUS_SELECT_RESERVED0_FIELD_WIDTH,
    TCAM_DEBUG_BUS_TCAM_DEBUG_BUS_SELECT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: TCAM_CONTEXT_RAM_TCAM_TCAM_CONTEXT_RAM_CONTEXT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_CONTEXT_RAM_TCAM_TCAM_CONTEXT_RAM_CONTEXT_FIELDS[] =
{
    &TCAM_CONTEXT_RAM_TCAM_TCAM_CONTEXT_RAM_CONTEXT_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec TCAM_CONTEXT_RAM_TCAM_TCAM_CONTEXT_RAM_CONTEXT_REG = 
{
    "CONTEXT_RAM_TCAM_TCAM_CONTEXT_RAM_CONTEXT",
#if RU_INCLUDE_DESC
    "CONTEXT %i Register",
    "Each 64 bit entry in the context ram occupies two addresses:"
    "For 64bit entry number i:"
    "the 32 least significant bits of the context are in address 2*i"
    "the 32 most significant bits of the context are in address 2*i +1",
#endif
    TCAM_CONTEXT_RAM_TCAM_TCAM_CONTEXT_RAM_CONTEXT_REG_OFFSET,
    TCAM_CONTEXT_RAM_TCAM_TCAM_CONTEXT_RAM_CONTEXT_REG_RAM_CNT,
    4,
    689,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_CONTEXT_RAM_TCAM_TCAM_CONTEXT_RAM_CONTEXT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: TCAM_CFG_TCAM_TCAM_CFG_BANK_EN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_CFG_TCAM_TCAM_CFG_BANK_EN_FIELDS[] =
{
    &TCAM_CFG_TCAM_TCAM_CFG_BANK_EN_VALUE_FIELD,
    &TCAM_CFG_TCAM_TCAM_CFG_BANK_EN_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec TCAM_CFG_TCAM_TCAM_CFG_BANK_EN_REG = 
{
    "CFG_TCAM_TCAM_CFG_BANK_EN",
#if RU_INCLUDE_DESC
    "BANK_ENABLE Register",
    "The TCAM is divided into 8 banks. banks can be disabled to save power. bit i correspond to addresses i*128:i*128+127",
#endif
    TCAM_CFG_TCAM_TCAM_CFG_BANK_EN_REG_OFFSET,
    0,
    0,
    690,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    TCAM_CFG_TCAM_TCAM_CFG_BANK_EN_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: TCAM_CFG_TCAM_TCAM_CFG_GLOBAL_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_CFG_TCAM_TCAM_CFG_GLOBAL_MASK_FIELDS[] =
{
    &TCAM_CFG_TCAM_TCAM_CFG_GLOBAL_MASK_VALUE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec TCAM_CFG_TCAM_TCAM_CFG_GLOBAL_MASK_REG = 
{
    "CFG_TCAM_TCAM_CFG_GLOBAL_MASK",
#if RU_INCLUDE_DESC
    "GLOBAL_MASK %i Register",
    "Global Mask - 256bit mask for all entries. Default value enable all bits."
    "",
#endif
    TCAM_CFG_TCAM_TCAM_CFG_GLOBAL_MASK_REG_OFFSET,
    TCAM_CFG_TCAM_TCAM_CFG_GLOBAL_MASK_REG_RAM_CNT,
    4,
    691,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_CFG_TCAM_TCAM_CFG_GLOBAL_MASK_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: TCAM_COUNTERS_TCAM_TCAM_COUNTERS_SRCH_256
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_COUNTERS_TCAM_TCAM_COUNTERS_SRCH_256_FIELDS[] =
{
    &TCAM_COUNTERS_TCAM_TCAM_COUNTERS_SRCH_256_CNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec TCAM_COUNTERS_TCAM_TCAM_COUNTERS_SRCH_256_REG = 
{
    "COUNTERS_TCAM_TCAM_COUNTERS_SRCH_256",
#if RU_INCLUDE_DESC
    "SEARCHES_256BIT Register",
    "Number of 256bit key searches",
#endif
    TCAM_COUNTERS_TCAM_TCAM_COUNTERS_SRCH_256_REG_OFFSET,
    0,
    0,
    692,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_COUNTERS_TCAM_TCAM_COUNTERS_SRCH_256_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: TCAM_COUNTERS_TCAM_TCAM_COUNTERS_HIT_256
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_COUNTERS_TCAM_TCAM_COUNTERS_HIT_256_FIELDS[] =
{
    &TCAM_COUNTERS_TCAM_TCAM_COUNTERS_HIT_256_CNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec TCAM_COUNTERS_TCAM_TCAM_COUNTERS_HIT_256_REG = 
{
    "COUNTERS_TCAM_TCAM_COUNTERS_HIT_256",
#if RU_INCLUDE_DESC
    "HITS_256BIT Register",
    "Number of 256bit key hits",
#endif
    TCAM_COUNTERS_TCAM_TCAM_COUNTERS_HIT_256_REG_OFFSET,
    0,
    0,
    693,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_COUNTERS_TCAM_TCAM_COUNTERS_HIT_256_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: TCAM_COUNTERS_TCAM_TCAM_COUNTERS_SRCH_512
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_COUNTERS_TCAM_TCAM_COUNTERS_SRCH_512_FIELDS[] =
{
    &TCAM_COUNTERS_TCAM_TCAM_COUNTERS_SRCH_512_CNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec TCAM_COUNTERS_TCAM_TCAM_COUNTERS_SRCH_512_REG = 
{
    "COUNTERS_TCAM_TCAM_COUNTERS_SRCH_512",
#if RU_INCLUDE_DESC
    "SEARCHES_512BIT Register",
    "Number of 512it key searches",
#endif
    TCAM_COUNTERS_TCAM_TCAM_COUNTERS_SRCH_512_REG_OFFSET,
    0,
    0,
    694,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_COUNTERS_TCAM_TCAM_COUNTERS_SRCH_512_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: TCAM_COUNTERS_TCAM_TCAM_COUNTERS_HIT_512
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_COUNTERS_TCAM_TCAM_COUNTERS_HIT_512_FIELDS[] =
{
    &TCAM_COUNTERS_TCAM_TCAM_COUNTERS_HIT_512_CNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec TCAM_COUNTERS_TCAM_TCAM_COUNTERS_HIT_512_REG = 
{
    "COUNTERS_TCAM_TCAM_COUNTERS_HIT_512",
#if RU_INCLUDE_DESC
    "HITS_512BIT Register",
    "Number of 512bit key hits",
#endif
    TCAM_COUNTERS_TCAM_TCAM_COUNTERS_HIT_512_REG_OFFSET,
    0,
    0,
    695,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_COUNTERS_TCAM_TCAM_COUNTERS_HIT_512_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: TCAM_INDIRECT_TCAM_TCAM_INDIRECT_OP
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_INDIRECT_TCAM_TCAM_INDIRECT_OP_FIELDS[] =
{
    &TCAM_INDIRECT_TCAM_TCAM_INDIRECT_OP_CMD_FIELD,
    &TCAM_INDIRECT_TCAM_TCAM_INDIRECT_OP_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec TCAM_INDIRECT_TCAM_TCAM_INDIRECT_OP_REG = 
{
    "INDIRECT_TCAM_TCAM_INDIRECT_OP",
#if RU_INCLUDE_DESC
    "OPERATION Register",
    "TCAM Operation:"
    "0 - TCAM READ"
    "1 - TCAM Write"
    "2 - TCAM Compare"
    "3 - TCAM valid bit reset"
    "Writing to this register triggers the operation. All other relevant register should be ready before SW writes to this register.",
#endif
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_OP_REG_OFFSET,
    0,
    0,
    696,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_OP_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: TCAM_INDIRECT_TCAM_TCAM_INDIRECT_OP_DONE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_INDIRECT_TCAM_TCAM_INDIRECT_OP_DONE_FIELDS[] =
{
    &TCAM_INDIRECT_TCAM_TCAM_INDIRECT_OP_DONE_DONE_FIELD,
    &TCAM_INDIRECT_TCAM_TCAM_INDIRECT_OP_DONE_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec TCAM_INDIRECT_TCAM_TCAM_INDIRECT_OP_DONE_REG = 
{
    "INDIRECT_TCAM_TCAM_INDIRECT_OP_DONE",
#if RU_INCLUDE_DESC
    "OPERATION_DONE Register",
    "Raised when the TCAM operation is completed (cleared by HW on write to the OPERATION regiser)",
#endif
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_OP_DONE_REG_OFFSET,
    0,
    0,
    697,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_OP_DONE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: TCAM_INDIRECT_TCAM_TCAM_INDIRECT_ADDR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_INDIRECT_TCAM_TCAM_INDIRECT_ADDR_FIELDS[] =
{
    &TCAM_INDIRECT_TCAM_TCAM_INDIRECT_ADDR_KEY1_IND_FIELD,
    &TCAM_INDIRECT_TCAM_TCAM_INDIRECT_ADDR_ENTRY_ADDR_FIELD,
    &TCAM_INDIRECT_TCAM_TCAM_INDIRECT_ADDR_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec TCAM_INDIRECT_TCAM_TCAM_INDIRECT_ADDR_REG = 
{
    "INDIRECT_TCAM_TCAM_INDIRECT_ADDR",
#if RU_INCLUDE_DESC
    "ADDRESS Register",
    "Key Address to be used in RD/WR opoerations.",
#endif
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_ADDR_REG_OFFSET,
    0,
    0,
    698,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_ADDR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: TCAM_INDIRECT_TCAM_TCAM_INDIRECT_VLID_IN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_INDIRECT_TCAM_TCAM_INDIRECT_VLID_IN_FIELDS[] =
{
    &TCAM_INDIRECT_TCAM_TCAM_INDIRECT_VLID_IN_VALID_FIELD,
    &TCAM_INDIRECT_TCAM_TCAM_INDIRECT_VLID_IN_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec TCAM_INDIRECT_TCAM_TCAM_INDIRECT_VLID_IN_REG = 
{
    "INDIRECT_TCAM_TCAM_INDIRECT_VLID_IN",
#if RU_INCLUDE_DESC
    "VALID_IN Register",
    "Valid value to be written - this value is relevant during write operation on key0.",
#endif
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_VLID_IN_REG_OFFSET,
    0,
    0,
    699,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_VLID_IN_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: TCAM_INDIRECT_TCAM_TCAM_INDIRECT_VLID_OUT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_INDIRECT_TCAM_TCAM_INDIRECT_VLID_OUT_FIELDS[] =
{
    &TCAM_INDIRECT_TCAM_TCAM_INDIRECT_VLID_OUT_VALID_FIELD,
    &TCAM_INDIRECT_TCAM_TCAM_INDIRECT_VLID_OUT_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec TCAM_INDIRECT_TCAM_TCAM_INDIRECT_VLID_OUT_REG = 
{
    "INDIRECT_TCAM_TCAM_INDIRECT_VLID_OUT",
#if RU_INCLUDE_DESC
    "VALID_OUT Register",
    "Valid value read from the TCAM - this value is relevant during read operation on key0.",
#endif
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_VLID_OUT_REG_OFFSET,
    0,
    0,
    700,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_VLID_OUT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: TCAM_INDIRECT_TCAM_TCAM_INDIRECT_RSLT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_INDIRECT_TCAM_TCAM_INDIRECT_RSLT_FIELDS[] =
{
    &TCAM_INDIRECT_TCAM_TCAM_INDIRECT_RSLT_MATCH_FIELD,
    &TCAM_INDIRECT_TCAM_TCAM_INDIRECT_RSLT_RESERVED0_FIELD,
    &TCAM_INDIRECT_TCAM_TCAM_INDIRECT_RSLT_INDEX_FIELD,
    &TCAM_INDIRECT_TCAM_TCAM_INDIRECT_RSLT_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec TCAM_INDIRECT_TCAM_TCAM_INDIRECT_RSLT_REG = 
{
    "INDIRECT_TCAM_TCAM_INDIRECT_RSLT",
#if RU_INCLUDE_DESC
    "SEARCH_RESULT Register",
    "The result of a search operation",
#endif
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_RSLT_REG_OFFSET,
    0,
    0,
    701,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_RSLT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: TCAM_INDIRECT_TCAM_TCAM_INDIRECT_KEY_IN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_INDIRECT_TCAM_TCAM_INDIRECT_KEY_IN_FIELDS[] =
{
    &TCAM_INDIRECT_TCAM_TCAM_INDIRECT_KEY_IN_VALUE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec TCAM_INDIRECT_TCAM_TCAM_INDIRECT_KEY_IN_REG = 
{
    "INDIRECT_TCAM_TCAM_INDIRECT_KEY_IN",
#if RU_INCLUDE_DESC
    "KEY_IN %i Register",
    "Key to be used in Write/Compare operations."
    "The Key is 256bit long and is represented by 8 registers. The lower address register correspond to the least significant bits of the key.",
#endif
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_KEY_IN_REG_OFFSET,
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_KEY_IN_REG_RAM_CNT,
    4,
    702,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_KEY_IN_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: TCAM_INDIRECT_TCAM_TCAM_INDIRECT_KEY_OUT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_INDIRECT_TCAM_TCAM_INDIRECT_KEY_OUT_FIELDS[] =
{
    &TCAM_INDIRECT_TCAM_TCAM_INDIRECT_KEY_OUT_VALUE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec TCAM_INDIRECT_TCAM_TCAM_INDIRECT_KEY_OUT_REG = 
{
    "INDIRECT_TCAM_TCAM_INDIRECT_KEY_OUT",
#if RU_INCLUDE_DESC
    "KEY_OUT %i Register",
    "Key returned from the CAM in a read operation. The Key is 256bit long and is represented by 8 registers. The lower address register correspond to the least significant bits of the key.",
#endif
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_KEY_OUT_REG_OFFSET,
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_KEY_OUT_REG_RAM_CNT,
    4,
    703,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_INDIRECT_TCAM_TCAM_INDIRECT_KEY_OUT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: TCAM_DEBUG_BUS_TCAM_DEBUG_BUS_SELECT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_DEBUG_BUS_TCAM_DEBUG_BUS_SELECT_FIELDS[] =
{
    &TCAM_DEBUG_BUS_TCAM_DEBUG_BUS_SELECT_SELECT_MODULE_FIELD,
    &TCAM_DEBUG_BUS_TCAM_DEBUG_BUS_SELECT_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec TCAM_DEBUG_BUS_TCAM_DEBUG_BUS_SELECT_REG = 
{
    "DEBUG_BUS_TCAM_DEBUG_BUS_SELECT",
#if RU_INCLUDE_DESC
    "SELECT Register",
    "Select",
#endif
    TCAM_DEBUG_BUS_TCAM_DEBUG_BUS_SELECT_REG_OFFSET,
    0,
    0,
    704,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    TCAM_DEBUG_BUS_TCAM_DEBUG_BUS_SELECT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Block: TCAM
 ******************************************************************************/
static const ru_reg_rec *TCAM_REGS[] =
{
    &TCAM_CONTEXT_RAM_TCAM_TCAM_CONTEXT_RAM_CONTEXT_REG,
    &TCAM_CFG_TCAM_TCAM_CFG_BANK_EN_REG,
    &TCAM_CFG_TCAM_TCAM_CFG_GLOBAL_MASK_REG,
    &TCAM_COUNTERS_TCAM_TCAM_COUNTERS_SRCH_256_REG,
    &TCAM_COUNTERS_TCAM_TCAM_COUNTERS_HIT_256_REG,
    &TCAM_COUNTERS_TCAM_TCAM_COUNTERS_SRCH_512_REG,
    &TCAM_COUNTERS_TCAM_TCAM_COUNTERS_HIT_512_REG,
    &TCAM_INDIRECT_TCAM_TCAM_INDIRECT_OP_REG,
    &TCAM_INDIRECT_TCAM_TCAM_INDIRECT_OP_DONE_REG,
    &TCAM_INDIRECT_TCAM_TCAM_INDIRECT_ADDR_REG,
    &TCAM_INDIRECT_TCAM_TCAM_INDIRECT_VLID_IN_REG,
    &TCAM_INDIRECT_TCAM_TCAM_INDIRECT_VLID_OUT_REG,
    &TCAM_INDIRECT_TCAM_TCAM_INDIRECT_RSLT_REG,
    &TCAM_INDIRECT_TCAM_TCAM_INDIRECT_KEY_IN_REG,
    &TCAM_INDIRECT_TCAM_TCAM_INDIRECT_KEY_OUT_REG,
    &TCAM_DEBUG_BUS_TCAM_DEBUG_BUS_SELECT_REG,
};

unsigned long TCAM_ADDRS[] =
{
    0x82e00000,
};

const ru_block_rec TCAM_BLOCK = 
{
    "TCAM",
    TCAM_ADDRS,
    1,
    16,
    TCAM_REGS
};

/* End of file XRDP_TCAM.c */
