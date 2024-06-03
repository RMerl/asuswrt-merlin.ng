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


#include "XRDP_TCAM_AG.h"

/******************************************************************************
 * Register: NAME: TCAM_CONTEXT_RAM_CONTEXT, TYPE: Type_TCAM_TCAM_CONTEXT_RAM_CONTEXT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec TCAM_CONTEXT_RAM_CONTEXT_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { TCAM_CONTEXT_RAM_CONTEXT_DATA_FIELD_MASK },
    0,
    { TCAM_CONTEXT_RAM_CONTEXT_DATA_FIELD_WIDTH },
    { TCAM_CONTEXT_RAM_CONTEXT_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_CONTEXT_RAM_CONTEXT_FIELDS[] =
{
    &TCAM_CONTEXT_RAM_CONTEXT_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_CONTEXT_RAM_CONTEXT *****/
const ru_reg_rec TCAM_CONTEXT_RAM_CONTEXT_REG =
{
    "CONTEXT_RAM_CONTEXT",
#if RU_INCLUDE_DESC
    "CONTEXT 0..8191 Register",
    "Each 128 bit entry in the context ram occupies four addresses:\nFor 128bit entry number i:\nthe 32 least significant bits of the context are in address 4*i\nthe 32 most significant bits of the context are in address 4*i+3\n\n",
#endif
    { TCAM_CONTEXT_RAM_CONTEXT_REG_OFFSET },
    TCAM_CONTEXT_RAM_CONTEXT_REG_RAM_CNT,
    4,
    1052,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_CONTEXT_RAM_CONTEXT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_CFG_BANK_EN, TYPE: Type_TCAM_TCAM_CFG_BANK_EN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALUE *****/
const ru_field_rec TCAM_CFG_BANK_EN_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { TCAM_CFG_BANK_EN_VALUE_FIELD_MASK },
    0,
    { TCAM_CFG_BANK_EN_VALUE_FIELD_WIDTH },
    { TCAM_CFG_BANK_EN_VALUE_FIELD_SHIFT },
    65535,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_CFG_BANK_EN_FIELDS[] =
{
    &TCAM_CFG_BANK_EN_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_CFG_BANK_EN *****/
const ru_reg_rec TCAM_CFG_BANK_EN_REG =
{
    "CFG_BANK_EN",
#if RU_INCLUDE_DESC
    "BANK_ENABLE Register",
    "The TCAM is divided into 8 banks. banks can be disabled to save power. bit i correspond to addresses i*128:i*128+127\n",
#endif
    { TCAM_CFG_BANK_EN_REG_OFFSET },
    0,
    0,
    1053,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_CFG_BANK_EN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_CFG_TM_TCAM0, TYPE: Type_TCAM_TCAM_CFG_TM_TCAM0
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALUE *****/
const ru_field_rec TCAM_CFG_TM_TCAM0_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { TCAM_CFG_TM_TCAM0_VALUE_FIELD_MASK },
    0,
    { TCAM_CFG_TM_TCAM0_VALUE_FIELD_WIDTH },
    { TCAM_CFG_TM_TCAM0_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_CFG_TM_TCAM0_FIELDS[] =
{
    &TCAM_CFG_TM_TCAM0_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_CFG_TM_TCAM0 *****/
const ru_reg_rec TCAM_CFG_TM_TCAM0_REG =
{
    "CFG_TM_TCAM0",
#if RU_INCLUDE_DESC
    "TM_TCAM0 Register",
    "TM_Control\n",
#endif
    { TCAM_CFG_TM_TCAM0_REG_OFFSET },
    0,
    0,
    1054,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_CFG_TM_TCAM0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_CFG_TM_TCAM1, TYPE: Type_TCAM_TCAM_CFG_TM_TCAM1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALUE *****/
const ru_field_rec TCAM_CFG_TM_TCAM1_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { TCAM_CFG_TM_TCAM1_VALUE_FIELD_MASK },
    0,
    { TCAM_CFG_TM_TCAM1_VALUE_FIELD_WIDTH },
    { TCAM_CFG_TM_TCAM1_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_CFG_TM_TCAM1_FIELDS[] =
{
    &TCAM_CFG_TM_TCAM1_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_CFG_TM_TCAM1 *****/
const ru_reg_rec TCAM_CFG_TM_TCAM1_REG =
{
    "CFG_TM_TCAM1",
#if RU_INCLUDE_DESC
    "TM_TCAM1 Register",
    "TM_control\n",
#endif
    { TCAM_CFG_TM_TCAM1_REG_OFFSET },
    0,
    0,
    1055,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_CFG_TM_TCAM1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_CFG_GLOBAL_MASK, TYPE: Type_TCAM_TCAM_CFG_GLOBAL_MASK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALUE *****/
const ru_field_rec TCAM_CFG_GLOBAL_MASK_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { TCAM_CFG_GLOBAL_MASK_VALUE_FIELD_MASK },
    0,
    { TCAM_CFG_GLOBAL_MASK_VALUE_FIELD_WIDTH },
    { TCAM_CFG_GLOBAL_MASK_VALUE_FIELD_SHIFT },
    4294967295,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_CFG_GLOBAL_MASK_FIELDS[] =
{
    &TCAM_CFG_GLOBAL_MASK_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_CFG_GLOBAL_MASK *****/
const ru_reg_rec TCAM_CFG_GLOBAL_MASK_REG =
{
    "CFG_GLOBAL_MASK",
#if RU_INCLUDE_DESC
    "GLOBAL_MASK 0..7 Register",
    "Global Mask - 256bit mask for all entries. Default value enable all bits.\n\n",
#endif
    { TCAM_CFG_GLOBAL_MASK_REG_OFFSET },
    TCAM_CFG_GLOBAL_MASK_REG_RAM_CNT,
    4,
    1056,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_CFG_GLOBAL_MASK_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_COUNTERS_SRCH_SHORT_KEY, TYPE: Type_TCAM_TCAM_COUNTERS_SRCH_SHORT_KEY
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CNT *****/
const ru_field_rec TCAM_COUNTERS_SRCH_SHORT_KEY_CNT_FIELD =
{
    "CNT",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { TCAM_COUNTERS_SRCH_SHORT_KEY_CNT_FIELD_MASK },
    0,
    { TCAM_COUNTERS_SRCH_SHORT_KEY_CNT_FIELD_WIDTH },
    { TCAM_COUNTERS_SRCH_SHORT_KEY_CNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_COUNTERS_SRCH_SHORT_KEY_FIELDS[] =
{
    &TCAM_COUNTERS_SRCH_SHORT_KEY_CNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_COUNTERS_SRCH_SHORT_KEY *****/
const ru_reg_rec TCAM_COUNTERS_SRCH_SHORT_KEY_REG =
{
    "COUNTERS_SRCH_SHORT_KEY",
#if RU_INCLUDE_DESC
    "SEARCHES_SHORT_KEY Register",
    "Number of short key searches\n",
#endif
    { TCAM_COUNTERS_SRCH_SHORT_KEY_REG_OFFSET },
    0,
    0,
    1057,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_COUNTERS_SRCH_SHORT_KEY_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_COUNTERS_HIT_SHORT_KEY, TYPE: Type_TCAM_TCAM_COUNTERS_HIT_SHORT_KEY
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CNT *****/
const ru_field_rec TCAM_COUNTERS_HIT_SHORT_KEY_CNT_FIELD =
{
    "CNT",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { TCAM_COUNTERS_HIT_SHORT_KEY_CNT_FIELD_MASK },
    0,
    { TCAM_COUNTERS_HIT_SHORT_KEY_CNT_FIELD_WIDTH },
    { TCAM_COUNTERS_HIT_SHORT_KEY_CNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_COUNTERS_HIT_SHORT_KEY_FIELDS[] =
{
    &TCAM_COUNTERS_HIT_SHORT_KEY_CNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_COUNTERS_HIT_SHORT_KEY *****/
const ru_reg_rec TCAM_COUNTERS_HIT_SHORT_KEY_REG =
{
    "COUNTERS_HIT_SHORT_KEY",
#if RU_INCLUDE_DESC
    "HITS_SHORT_KEY Register",
    "Number of short key hits\n",
#endif
    { TCAM_COUNTERS_HIT_SHORT_KEY_REG_OFFSET },
    0,
    0,
    1058,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_COUNTERS_HIT_SHORT_KEY_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_COUNTERS_SRCH_LONG_KEY, TYPE: Type_TCAM_TCAM_COUNTERS_SRCH_LONG_KEY
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CNT *****/
const ru_field_rec TCAM_COUNTERS_SRCH_LONG_KEY_CNT_FIELD =
{
    "CNT",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { TCAM_COUNTERS_SRCH_LONG_KEY_CNT_FIELD_MASK },
    0,
    { TCAM_COUNTERS_SRCH_LONG_KEY_CNT_FIELD_WIDTH },
    { TCAM_COUNTERS_SRCH_LONG_KEY_CNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_COUNTERS_SRCH_LONG_KEY_FIELDS[] =
{
    &TCAM_COUNTERS_SRCH_LONG_KEY_CNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_COUNTERS_SRCH_LONG_KEY *****/
const ru_reg_rec TCAM_COUNTERS_SRCH_LONG_KEY_REG =
{
    "COUNTERS_SRCH_LONG_KEY",
#if RU_INCLUDE_DESC
    "SEARCHES_LONG_KEY Register",
    "Number of long key searches\n",
#endif
    { TCAM_COUNTERS_SRCH_LONG_KEY_REG_OFFSET },
    0,
    0,
    1059,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_COUNTERS_SRCH_LONG_KEY_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_COUNTERS_HIT_LONG_KEY, TYPE: Type_TCAM_TCAM_COUNTERS_HIT_LONG_KEY
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CNT *****/
const ru_field_rec TCAM_COUNTERS_HIT_LONG_KEY_CNT_FIELD =
{
    "CNT",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { TCAM_COUNTERS_HIT_LONG_KEY_CNT_FIELD_MASK },
    0,
    { TCAM_COUNTERS_HIT_LONG_KEY_CNT_FIELD_WIDTH },
    { TCAM_COUNTERS_HIT_LONG_KEY_CNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_COUNTERS_HIT_LONG_KEY_FIELDS[] =
{
    &TCAM_COUNTERS_HIT_LONG_KEY_CNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_COUNTERS_HIT_LONG_KEY *****/
const ru_reg_rec TCAM_COUNTERS_HIT_LONG_KEY_REG =
{
    "COUNTERS_HIT_LONG_KEY",
#if RU_INCLUDE_DESC
    "HITS_LONG_KEY Register",
    "Number of long key hits\n",
#endif
    { TCAM_COUNTERS_HIT_LONG_KEY_REG_OFFSET },
    0,
    0,
    1060,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_COUNTERS_HIT_LONG_KEY_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_INDIRECT_OP, TYPE: Type_TCAM_TCAM_INDIRECT_OP
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CMD *****/
const ru_field_rec TCAM_INDIRECT_OP_CMD_FIELD =
{
    "CMD",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { TCAM_INDIRECT_OP_CMD_FIELD_MASK },
    0,
    { TCAM_INDIRECT_OP_CMD_FIELD_WIDTH },
    { TCAM_INDIRECT_OP_CMD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_INDIRECT_OP_FIELDS[] =
{
    &TCAM_INDIRECT_OP_CMD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_INDIRECT_OP *****/
const ru_reg_rec TCAM_INDIRECT_OP_REG =
{
    "INDIRECT_OP",
#if RU_INCLUDE_DESC
    "OPERATION Register",
    "TCAM Operation:\n0 - TCAM READ\n1 - TCAM Write\n2 - TCAM Compare\n3 - TCAM valid bit reset\nWriting to this register triggers the operation. All other relevant register should be ready before SW writes to this register.\n",
#endif
    { TCAM_INDIRECT_OP_REG_OFFSET },
    0,
    0,
    1061,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_INDIRECT_OP_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_INDIRECT_OP_DONE, TYPE: Type_TCAM_TCAM_INDIRECT_OP_DONE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DONE *****/
const ru_field_rec TCAM_INDIRECT_OP_DONE_DONE_FIELD =
{
    "DONE",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { TCAM_INDIRECT_OP_DONE_DONE_FIELD_MASK },
    0,
    { TCAM_INDIRECT_OP_DONE_DONE_FIELD_WIDTH },
    { TCAM_INDIRECT_OP_DONE_DONE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_INDIRECT_OP_DONE_FIELDS[] =
{
    &TCAM_INDIRECT_OP_DONE_DONE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_INDIRECT_OP_DONE *****/
const ru_reg_rec TCAM_INDIRECT_OP_DONE_REG =
{
    "INDIRECT_OP_DONE",
#if RU_INCLUDE_DESC
    "OPERATION_DONE Register",
    "Raised when the TCAM operation is completed (cleared by HW on write to the OPERATION regiser)\n",
#endif
    { TCAM_INDIRECT_OP_DONE_REG_OFFSET },
    0,
    0,
    1062,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_INDIRECT_OP_DONE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_INDIRECT_ADDR, TYPE: Type_TCAM_TCAM_INDIRECT_ADDR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: KEY1_IND *****/
const ru_field_rec TCAM_INDIRECT_ADDR_KEY1_IND_FIELD =
{
    "KEY1_IND",
#if RU_INCLUDE_DESC
    "",
    "This bit indicate if the operation (RD/WR) is performed on the key0 or key1 part of the entry\n",
#endif
    { TCAM_INDIRECT_ADDR_KEY1_IND_FIELD_MASK },
    0,
    { TCAM_INDIRECT_ADDR_KEY1_IND_FIELD_WIDTH },
    { TCAM_INDIRECT_ADDR_KEY1_IND_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ENTRY_ADDR *****/
const ru_field_rec TCAM_INDIRECT_ADDR_ENTRY_ADDR_FIELD =
{
    "ENTRY_ADDR",
#if RU_INCLUDE_DESC
    "",
    "Address of the entry\n",
#endif
    { TCAM_INDIRECT_ADDR_ENTRY_ADDR_FIELD_MASK },
    0,
    { TCAM_INDIRECT_ADDR_ENTRY_ADDR_FIELD_WIDTH },
    { TCAM_INDIRECT_ADDR_ENTRY_ADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_INDIRECT_ADDR_FIELDS[] =
{
    &TCAM_INDIRECT_ADDR_KEY1_IND_FIELD,
    &TCAM_INDIRECT_ADDR_ENTRY_ADDR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_INDIRECT_ADDR *****/
const ru_reg_rec TCAM_INDIRECT_ADDR_REG =
{
    "INDIRECT_ADDR",
#if RU_INCLUDE_DESC
    "ADDRESS Register",
    "Key Address to be used in RD/WR opoerations.\n",
#endif
    { TCAM_INDIRECT_ADDR_REG_OFFSET },
    0,
    0,
    1063,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    TCAM_INDIRECT_ADDR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_INDIRECT_VLID_IN, TYPE: Type_TCAM_TCAM_INDIRECT_VLID_IN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALID *****/
const ru_field_rec TCAM_INDIRECT_VLID_IN_VALID_FIELD =
{
    "VALID",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { TCAM_INDIRECT_VLID_IN_VALID_FIELD_MASK },
    0,
    { TCAM_INDIRECT_VLID_IN_VALID_FIELD_WIDTH },
    { TCAM_INDIRECT_VLID_IN_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_INDIRECT_VLID_IN_FIELDS[] =
{
    &TCAM_INDIRECT_VLID_IN_VALID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_INDIRECT_VLID_IN *****/
const ru_reg_rec TCAM_INDIRECT_VLID_IN_REG =
{
    "INDIRECT_VLID_IN",
#if RU_INCLUDE_DESC
    "VALID_IN Register",
    "Valid value to be written - this value is relevant during write operation on key0.\n",
#endif
    { TCAM_INDIRECT_VLID_IN_REG_OFFSET },
    0,
    0,
    1064,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_INDIRECT_VLID_IN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_INDIRECT_VLID_OUT, TYPE: Type_TCAM_TCAM_INDIRECT_VLID_OUT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALID *****/
const ru_field_rec TCAM_INDIRECT_VLID_OUT_VALID_FIELD =
{
    "VALID",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { TCAM_INDIRECT_VLID_OUT_VALID_FIELD_MASK },
    0,
    { TCAM_INDIRECT_VLID_OUT_VALID_FIELD_WIDTH },
    { TCAM_INDIRECT_VLID_OUT_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_INDIRECT_VLID_OUT_FIELDS[] =
{
    &TCAM_INDIRECT_VLID_OUT_VALID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_INDIRECT_VLID_OUT *****/
const ru_reg_rec TCAM_INDIRECT_VLID_OUT_REG =
{
    "INDIRECT_VLID_OUT",
#if RU_INCLUDE_DESC
    "VALID_OUT Register",
    "Valid value read from the TCAM - this value is relevant during read operation on key0.\n",
#endif
    { TCAM_INDIRECT_VLID_OUT_REG_OFFSET },
    0,
    0,
    1065,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_INDIRECT_VLID_OUT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_INDIRECT_RSLT, TYPE: Type_TCAM_TCAM_INDIRECT_RSLT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MATCH *****/
const ru_field_rec TCAM_INDIRECT_RSLT_MATCH_FIELD =
{
    "MATCH",
#if RU_INCLUDE_DESC
    "",
    "indicate if a match was found\n",
#endif
    { TCAM_INDIRECT_RSLT_MATCH_FIELD_MASK },
    0,
    { TCAM_INDIRECT_RSLT_MATCH_FIELD_WIDTH },
    { TCAM_INDIRECT_RSLT_MATCH_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: INDEX *****/
const ru_field_rec TCAM_INDIRECT_RSLT_INDEX_FIELD =
{
    "INDEX",
#if RU_INCLUDE_DESC
    "",
    "index related to a match result\n",
#endif
    { TCAM_INDIRECT_RSLT_INDEX_FIELD_MASK },
    0,
    { TCAM_INDIRECT_RSLT_INDEX_FIELD_WIDTH },
    { TCAM_INDIRECT_RSLT_INDEX_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_INDIRECT_RSLT_FIELDS[] =
{
    &TCAM_INDIRECT_RSLT_MATCH_FIELD,
    &TCAM_INDIRECT_RSLT_INDEX_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_INDIRECT_RSLT *****/
const ru_reg_rec TCAM_INDIRECT_RSLT_REG =
{
    "INDIRECT_RSLT",
#if RU_INCLUDE_DESC
    "SEARCH_RESULT Register",
    "The result of a search operation\n",
#endif
    { TCAM_INDIRECT_RSLT_REG_OFFSET },
    0,
    0,
    1066,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    TCAM_INDIRECT_RSLT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_INDIRECT_KEY_IN, TYPE: Type_TCAM_TCAM_INDIRECT_KEY_IN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALUE *****/
const ru_field_rec TCAM_INDIRECT_KEY_IN_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { TCAM_INDIRECT_KEY_IN_VALUE_FIELD_MASK },
    0,
    { TCAM_INDIRECT_KEY_IN_VALUE_FIELD_WIDTH },
    { TCAM_INDIRECT_KEY_IN_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_INDIRECT_KEY_IN_FIELDS[] =
{
    &TCAM_INDIRECT_KEY_IN_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_INDIRECT_KEY_IN *****/
const ru_reg_rec TCAM_INDIRECT_KEY_IN_REG =
{
    "INDIRECT_KEY_IN",
#if RU_INCLUDE_DESC
    "KEY_IN 0..7 Register",
    "Key to be used in Write/Compare operations.\nThe Key is 256bit long and is represented by 8 registers. The lower address register correspond to the least significant bits of the key.\n",
#endif
    { TCAM_INDIRECT_KEY_IN_REG_OFFSET },
    TCAM_INDIRECT_KEY_IN_REG_RAM_CNT,
    4,
    1067,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_INDIRECT_KEY_IN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_INDIRECT_KEY_OUT, TYPE: Type_TCAM_TCAM_INDIRECT_KEY_OUT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALUE *****/
const ru_field_rec TCAM_INDIRECT_KEY_OUT_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { TCAM_INDIRECT_KEY_OUT_VALUE_FIELD_MASK },
    0,
    { TCAM_INDIRECT_KEY_OUT_VALUE_FIELD_WIDTH },
    { TCAM_INDIRECT_KEY_OUT_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_INDIRECT_KEY_OUT_FIELDS[] =
{
    &TCAM_INDIRECT_KEY_OUT_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_INDIRECT_KEY_OUT *****/
const ru_reg_rec TCAM_INDIRECT_KEY_OUT_REG =
{
    "INDIRECT_KEY_OUT",
#if RU_INCLUDE_DESC
    "KEY_OUT 0..7 Register",
    "Key returned from the CAM in a read operation. The Key is 256bit long and is represented by 8 registers. The lower address register correspond to the least significant bits of the key.\n",
#endif
    { TCAM_INDIRECT_KEY_OUT_REG_OFFSET },
    TCAM_INDIRECT_KEY_OUT_REG_RAM_CNT,
    4,
    1068,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_INDIRECT_KEY_OUT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_TCAM_DEBUG_BUS_SELECT, TYPE: Type_TCAM_DEBUG_BUS_SELECT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SELECT_MODULE *****/
const ru_field_rec TCAM_TCAM_DEBUG_BUS_SELECT_SELECT_MODULE_FIELD =
{
    "SELECT_MODULE",
#if RU_INCLUDE_DESC
    "",
    "selection\n\n",
#endif
    { TCAM_TCAM_DEBUG_BUS_SELECT_SELECT_MODULE_FIELD_MASK },
    0,
    { TCAM_TCAM_DEBUG_BUS_SELECT_SELECT_MODULE_FIELD_WIDTH },
    { TCAM_TCAM_DEBUG_BUS_SELECT_SELECT_MODULE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_TCAM_DEBUG_BUS_SELECT_FIELDS[] =
{
    &TCAM_TCAM_DEBUG_BUS_SELECT_SELECT_MODULE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_TCAM_DEBUG_BUS_SELECT *****/
const ru_reg_rec TCAM_TCAM_DEBUG_BUS_SELECT_REG =
{
    "TCAM_DEBUG_BUS_SELECT",
#if RU_INCLUDE_DESC
    "SELECT Register",
    "Select\n",
#endif
    { TCAM_TCAM_DEBUG_BUS_SELECT_REG_OFFSET },
    0,
    0,
    1069,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_TCAM_DEBUG_BUS_SELECT_FIELDS,
#endif
};

unsigned long TCAM_ADDRS[] =
{
    0x828C0000,
};

static const ru_reg_rec *TCAM_REGS[] =
{
    &TCAM_CONTEXT_RAM_CONTEXT_REG,
    &TCAM_CFG_BANK_EN_REG,
    &TCAM_CFG_TM_TCAM0_REG,
    &TCAM_CFG_TM_TCAM1_REG,
    &TCAM_CFG_GLOBAL_MASK_REG,
    &TCAM_COUNTERS_SRCH_SHORT_KEY_REG,
    &TCAM_COUNTERS_HIT_SHORT_KEY_REG,
    &TCAM_COUNTERS_SRCH_LONG_KEY_REG,
    &TCAM_COUNTERS_HIT_LONG_KEY_REG,
    &TCAM_INDIRECT_OP_REG,
    &TCAM_INDIRECT_OP_DONE_REG,
    &TCAM_INDIRECT_ADDR_REG,
    &TCAM_INDIRECT_VLID_IN_REG,
    &TCAM_INDIRECT_VLID_OUT_REG,
    &TCAM_INDIRECT_RSLT_REG,
    &TCAM_INDIRECT_KEY_IN_REG,
    &TCAM_INDIRECT_KEY_OUT_REG,
    &TCAM_TCAM_DEBUG_BUS_SELECT_REG,
};

const ru_block_rec TCAM_BLOCK =
{
    "TCAM",
    TCAM_ADDRS,
    1,
    18,
    TCAM_REGS,
};
