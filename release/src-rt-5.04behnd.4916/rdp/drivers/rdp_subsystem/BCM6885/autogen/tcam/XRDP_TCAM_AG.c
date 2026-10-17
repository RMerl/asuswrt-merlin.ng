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
 * Register: NAME: TCAM_CFG_DINM, TYPE: Type_TCAM_TCAM_CFG_DINM
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_CFG_DINM_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { TCAM_CFG_DINM_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_CFG_DINM_RAM_LINE_FIELD_WIDTH },
    { TCAM_CFG_DINM_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_CFG_DINM_FIELDS[] =
{
    &TCAM_CFG_DINM_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_CFG_DINM *****/
const ru_reg_rec TCAM_CFG_DINM_REG =
{
    "CFG_DINM",
#if RU_INCLUDE_DESC
    "DINM 0..63 Register",
    ".\n",
#endif
    { TCAM_CFG_DINM_REG_OFFSET },
    TCAM_CFG_DINM_REG_RAM_CNT,
    4,
    1871,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_CFG_DINM_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_CFG_BLKSEL, TYPE: Type_TCAM_TCAM_CFG_BLKSEL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_CFG_BLKSEL_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { TCAM_CFG_BLKSEL_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_CFG_BLKSEL_RAM_LINE_FIELD_WIDTH },
    { TCAM_CFG_BLKSEL_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_CFG_BLKSEL_FIELDS[] =
{
    &TCAM_CFG_BLKSEL_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_CFG_BLKSEL *****/
const ru_reg_rec TCAM_CFG_BLKSEL_REG =
{
    "CFG_BLKSEL",
#if RU_INCLUDE_DESC
    "BLKSEL 0..1 Register",
    "Control the blksel inputs of tcam 0 (bank enable).\n",
#endif
    { TCAM_CFG_BLKSEL_REG_OFFSET },
    TCAM_CFG_BLKSEL_REG_RAM_CNT,
    4,
    1872,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_CFG_BLKSEL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_CFG_TM, TYPE: Type_TCAM_TCAM_CFG_TM
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_CFG_TM_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { TCAM_CFG_TM_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_CFG_TM_RAM_LINE_FIELD_WIDTH },
    { TCAM_CFG_TM_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_CFG_TM_FIELDS[] =
{
    &TCAM_CFG_TM_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_CFG_TM *****/
const ru_reg_rec TCAM_CFG_TM_REG =
{
    "CFG_TM",
#if RU_INCLUDE_DESC
    "TM 0..7 Register",
    "TM_Control\n",
#endif
    { TCAM_CFG_TM_REG_OFFSET },
    TCAM_CFG_TM_REG_RAM_CNT,
    4,
    1873,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_CFG_TM_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_CFG_VRESET, TYPE: Type_TCAM_TCAM_CFG_VRESET
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec TCAM_CFG_VRESET_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { TCAM_CFG_VRESET_VAL_FIELD_MASK },
    0,
    { TCAM_CFG_VRESET_VAL_FIELD_WIDTH },
    { TCAM_CFG_VRESET_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_CFG_VRESET_FIELDS[] =
{
    &TCAM_CFG_VRESET_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_CFG_VRESET *****/
const ru_reg_rec TCAM_CFG_VRESET_REG =
{
    "CFG_VRESET",
#if RU_INCLUDE_DESC
    "VRESET Register",
    "VRESET\n",
#endif
    { TCAM_CFG_VRESET_REG_OFFSET },
    0,
    0,
    1874,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_CFG_VRESET_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_COUNTERS_SEARCHES, TYPE: Type_TCAM_TCAM_COUNTERS_SEARCHES
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CNT *****/
const ru_field_rec TCAM_COUNTERS_SEARCHES_CNT_FIELD =
{
    "CNT",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { TCAM_COUNTERS_SEARCHES_CNT_FIELD_MASK },
    0,
    { TCAM_COUNTERS_SEARCHES_CNT_FIELD_WIDTH },
    { TCAM_COUNTERS_SEARCHES_CNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_COUNTERS_SEARCHES_FIELDS[] =
{
    &TCAM_COUNTERS_SEARCHES_CNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_COUNTERS_SEARCHES *****/
const ru_reg_rec TCAM_COUNTERS_SEARCHES_REG =
{
    "COUNTERS_SEARCHES",
#if RU_INCLUDE_DESC
    "SEARCHES Register",
    "Number of searches\n",
#endif
    { TCAM_COUNTERS_SEARCHES_REG_OFFSET },
    TCAM_COUNTERS_SEARCHES_REG_RAM_CNT,
    8,
    1875,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_COUNTERS_SEARCHES_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_COUNTERS_HITS, TYPE: Type_TCAM_TCAM_COUNTERS_HITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CNT *****/
const ru_field_rec TCAM_COUNTERS_HITS_CNT_FIELD =
{
    "CNT",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { TCAM_COUNTERS_HITS_CNT_FIELD_MASK },
    0,
    { TCAM_COUNTERS_HITS_CNT_FIELD_WIDTH },
    { TCAM_COUNTERS_HITS_CNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_COUNTERS_HITS_FIELDS[] =
{
    &TCAM_COUNTERS_HITS_CNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_COUNTERS_HITS *****/
const ru_reg_rec TCAM_COUNTERS_HITS_REG =
{
    "COUNTERS_HITS",
#if RU_INCLUDE_DESC
    "HITS Register",
    "Searches which resulted in at least one hit\n",
#endif
    { TCAM_COUNTERS_HITS_REG_OFFSET },
    TCAM_COUNTERS_HITS_REG_RAM_CNT,
    8,
    1876,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_COUNTERS_HITS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_TCAM_MEM_INDIRECT_OP, TYPE: Type_TCAM_MEM_INDIRECT_OP
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CMD *****/
const ru_field_rec TCAM_TCAM_MEM_INDIRECT_OP_CMD_FIELD =
{
    "CMD",
#if RU_INCLUDE_DESC
    "",
    ".\nTRIGGERS_HW\n\n",
#endif
    { TCAM_TCAM_MEM_INDIRECT_OP_CMD_FIELD_MASK },
    0,
    { TCAM_TCAM_MEM_INDIRECT_OP_CMD_FIELD_WIDTH },
    { TCAM_TCAM_MEM_INDIRECT_OP_CMD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_TCAM_MEM_INDIRECT_OP_FIELDS[] =
{
    &TCAM_TCAM_MEM_INDIRECT_OP_CMD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_TCAM_MEM_INDIRECT_OP *****/
const ru_reg_rec TCAM_TCAM_MEM_INDIRECT_OP_REG =
{
    "TCAM_MEM_INDIRECT_OP",
#if RU_INCLUDE_DESC
    "OPERATION Register",
    "0 - TCAM READ\n1 - TCAM Write\n2 - CNTX READ\n3 - CNTX Write\n4 - Search\n",
#endif
    { TCAM_TCAM_MEM_INDIRECT_OP_REG_OFFSET },
    0,
    0,
    1877,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_TCAM_MEM_INDIRECT_OP_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_TCAM_MEM_INDIRECT_OP_DONE, TYPE: Type_TCAM_MEM_INDIRECT_OP_DONE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DONE *****/
const ru_field_rec TCAM_TCAM_MEM_INDIRECT_OP_DONE_DONE_FIELD =
{
    "DONE",
#if RU_INCLUDE_DESC
    "",
    ".\nCHANGED_BY_HW\n\n",
#endif
    { TCAM_TCAM_MEM_INDIRECT_OP_DONE_DONE_FIELD_MASK },
    0,
    { TCAM_TCAM_MEM_INDIRECT_OP_DONE_DONE_FIELD_WIDTH },
    { TCAM_TCAM_MEM_INDIRECT_OP_DONE_DONE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_TCAM_MEM_INDIRECT_OP_DONE_FIELDS[] =
{
    &TCAM_TCAM_MEM_INDIRECT_OP_DONE_DONE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_TCAM_MEM_INDIRECT_OP_DONE *****/
const ru_reg_rec TCAM_TCAM_MEM_INDIRECT_OP_DONE_REG =
{
    "TCAM_MEM_INDIRECT_OP_DONE",
#if RU_INCLUDE_DESC
    "OPERATION_DONE Register",
    "Raised when the TCAM operation is completed (cleared by HW on write to the OPERATION regiser)\n",
#endif
    { TCAM_TCAM_MEM_INDIRECT_OP_DONE_REG_OFFSET },
    0,
    0,
    1878,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_TCAM_MEM_INDIRECT_OP_DONE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_TCAM_MEM_INDIRECT_ADDR, TYPE: Type_TCAM_MEM_INDIRECT_ADDR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ENTRY_ADDR *****/
const ru_field_rec TCAM_TCAM_MEM_INDIRECT_ADDR_ENTRY_ADDR_FIELD =
{
    "ENTRY_ADDR",
#if RU_INCLUDE_DESC
    "",
    "Address of the entry\nTRIGGERS_HW\n\n",
#endif
    { TCAM_TCAM_MEM_INDIRECT_ADDR_ENTRY_ADDR_FIELD_MASK },
    0,
    { TCAM_TCAM_MEM_INDIRECT_ADDR_ENTRY_ADDR_FIELD_WIDTH },
    { TCAM_TCAM_MEM_INDIRECT_ADDR_ENTRY_ADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MEM_ID *****/
const ru_field_rec TCAM_TCAM_MEM_INDIRECT_ADDR_MEM_ID_FIELD =
{
    "MEM_ID",
#if RU_INCLUDE_DESC
    "",
    "MEM_ID\n",
#endif
    { TCAM_TCAM_MEM_INDIRECT_ADDR_MEM_ID_FIELD_MASK },
    0,
    { TCAM_TCAM_MEM_INDIRECT_ADDR_MEM_ID_FIELD_WIDTH },
    { TCAM_TCAM_MEM_INDIRECT_ADDR_MEM_ID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_TCAM_MEM_INDIRECT_ADDR_FIELDS[] =
{
    &TCAM_TCAM_MEM_INDIRECT_ADDR_ENTRY_ADDR_FIELD,
    &TCAM_TCAM_MEM_INDIRECT_ADDR_MEM_ID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_TCAM_MEM_INDIRECT_ADDR *****/
const ru_reg_rec TCAM_TCAM_MEM_INDIRECT_ADDR_REG =
{
    "TCAM_MEM_INDIRECT_ADDR",
#if RU_INCLUDE_DESC
    "ADDRESS Register",
    "MEM ID and line address (TCAM or CNTX)\n",
#endif
    { TCAM_TCAM_MEM_INDIRECT_ADDR_REG_OFFSET },
    0,
    0,
    1879,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    TCAM_TCAM_MEM_INDIRECT_ADDR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_TCAM_MEM_INDIRECT_VALID_IN, TYPE: Type_TCAM_MEM_INDIRECT_VALID_IN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALID *****/
const ru_field_rec TCAM_TCAM_MEM_INDIRECT_VALID_IN_VALID_FIELD =
{
    "VALID",
#if RU_INCLUDE_DESC
    "",
    ".\n\n\n",
#endif
    { TCAM_TCAM_MEM_INDIRECT_VALID_IN_VALID_FIELD_MASK },
    0,
    { TCAM_TCAM_MEM_INDIRECT_VALID_IN_VALID_FIELD_WIDTH },
    { TCAM_TCAM_MEM_INDIRECT_VALID_IN_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_TCAM_MEM_INDIRECT_VALID_IN_FIELDS[] =
{
    &TCAM_TCAM_MEM_INDIRECT_VALID_IN_VALID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_TCAM_MEM_INDIRECT_VALID_IN *****/
const ru_reg_rec TCAM_TCAM_MEM_INDIRECT_VALID_IN_REG =
{
    "TCAM_MEM_INDIRECT_VALID_IN",
#if RU_INCLUDE_DESC
    "VALID_IN Register",
    "Valid value to be written - this value is relevant during write operation on key0.\n",
#endif
    { TCAM_TCAM_MEM_INDIRECT_VALID_IN_REG_OFFSET },
    0,
    0,
    1880,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_TCAM_MEM_INDIRECT_VALID_IN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_TCAM_MEM_INDIRECT_VALID_OUT, TYPE: Type_TCAM_MEM_INDIRECT_VALID_OUT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALID *****/
const ru_field_rec TCAM_TCAM_MEM_INDIRECT_VALID_OUT_VALID_FIELD =
{
    "VALID",
#if RU_INCLUDE_DESC
    "",
    ".\nTRIGGERS_HW\n\n",
#endif
    { TCAM_TCAM_MEM_INDIRECT_VALID_OUT_VALID_FIELD_MASK },
    0,
    { TCAM_TCAM_MEM_INDIRECT_VALID_OUT_VALID_FIELD_WIDTH },
    { TCAM_TCAM_MEM_INDIRECT_VALID_OUT_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_TCAM_MEM_INDIRECT_VALID_OUT_FIELDS[] =
{
    &TCAM_TCAM_MEM_INDIRECT_VALID_OUT_VALID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_TCAM_MEM_INDIRECT_VALID_OUT *****/
const ru_reg_rec TCAM_TCAM_MEM_INDIRECT_VALID_OUT_REG =
{
    "TCAM_MEM_INDIRECT_VALID_OUT",
#if RU_INCLUDE_DESC
    "VALID_OUT Register",
    "Valid value read from the TCAM - this value is relevant during read operation on key0.\n",
#endif
    { TCAM_TCAM_MEM_INDIRECT_VALID_OUT_REG_OFFSET },
    0,
    0,
    1881,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_TCAM_MEM_INDIRECT_VALID_OUT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_TCAM_MEM_INDIRECT_DATA_OUT, TYPE: Type_TCAM_MEM_INDIRECT_DATA_OUT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALUE *****/
const ru_field_rec TCAM_TCAM_MEM_INDIRECT_DATA_OUT_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "",
    ".\nTRIGGERS_HW\n\n",
#endif
    { TCAM_TCAM_MEM_INDIRECT_DATA_OUT_VALUE_FIELD_MASK },
    0,
    { TCAM_TCAM_MEM_INDIRECT_DATA_OUT_VALUE_FIELD_WIDTH },
    { TCAM_TCAM_MEM_INDIRECT_DATA_OUT_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_TCAM_MEM_INDIRECT_DATA_OUT_FIELDS[] =
{
    &TCAM_TCAM_MEM_INDIRECT_DATA_OUT_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_TCAM_MEM_INDIRECT_DATA_OUT *****/
const ru_reg_rec TCAM_TCAM_MEM_INDIRECT_DATA_OUT_REG =
{
    "TCAM_MEM_INDIRECT_DATA_OUT",
#if RU_INCLUDE_DESC
    "DATA_OUT 0..7 Register",
    "DATA_OUT\nTCAM Read- 128b\nCNTX Read- 64b\nSearch - 256b\nalways aligned to right\n",
#endif
    { TCAM_TCAM_MEM_INDIRECT_DATA_OUT_REG_OFFSET },
    TCAM_TCAM_MEM_INDIRECT_DATA_OUT_REG_RAM_CNT,
    4,
    1882,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_TCAM_MEM_INDIRECT_DATA_OUT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_TCAM_MEM_INDIRECT_DATA_IN, TYPE: Type_TCAM_MEM_INDIRECT_DATA_IN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALUE *****/
const ru_field_rec TCAM_TCAM_MEM_INDIRECT_DATA_IN_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "",
    ".\nCHANGED_BY_HW\n\n",
#endif
    { TCAM_TCAM_MEM_INDIRECT_DATA_IN_VALUE_FIELD_MASK },
    0,
    { TCAM_TCAM_MEM_INDIRECT_DATA_IN_VALUE_FIELD_WIDTH },
    { TCAM_TCAM_MEM_INDIRECT_DATA_IN_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_TCAM_MEM_INDIRECT_DATA_IN_FIELDS[] =
{
    &TCAM_TCAM_MEM_INDIRECT_DATA_IN_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_TCAM_MEM_INDIRECT_DATA_IN *****/
const ru_reg_rec TCAM_TCAM_MEM_INDIRECT_DATA_IN_REG =
{
    "TCAM_MEM_INDIRECT_DATA_IN",
#if RU_INCLUDE_DESC
    "DATA_IN 0..15 Register",
    "DATA_IN\nTCAM Write - 128bit\nCNTX Write - 64bit\nSearch (Key) - 512bit\nAligned to the left\n",
#endif
    { TCAM_TCAM_MEM_INDIRECT_DATA_IN_REG_OFFSET },
    TCAM_TCAM_MEM_INDIRECT_DATA_IN_REG_RAM_CNT,
    4,
    1883,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_TCAM_MEM_INDIRECT_DATA_IN_FIELDS,
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
    1884,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_TCAM_DEBUG_BUS_SELECT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_KEY_BUILDER_OFFSETS_0, TYPE: Type_TCAM_TCAM_PROFILES_KEY_BUILDER_OFFSETS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_0_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_0_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_0_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_0_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_KEY_BUILDER_OFFSETS_0_FIELDS[] =
{
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_0_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_KEY_BUILDER_OFFSETS_0 *****/
const ru_reg_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_0_REG =
{
    "PROFILES_KEY_BUILDER_OFFSETS_0",
#if RU_INCLUDE_DESC
    "KEY_BUILDER_OFFSETS 0..39 Register",
    "Maps between the 64B master key and the 16B tcam Keys.\nThe four low bytes of a tcam key can each be assigned with any aligned byte from the master key according to its configured index.\nThe 6 higher 16bit words of a tcam key can each be assigned with any aligned 16         bit words from the master key according to its configured index.\nEach configured index is 8 bits.\nMax value for byte offset is 63.\nMax value for 16bit word offset is 31.\nTotal bits in this RAM is:NUM_OF_TCAMS*(4+6)*8. LSB bits on lowest address corresponds to TCAM 0&LSB Byte of its key.\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_0_REG_OFFSET },
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_0_REG_RAM_CNT,
    580,
    1885,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_KEY_BUILDER_OFFSETS_1, TYPE: Type_TCAM_TCAM_PROFILES_KEY_BUILDER_OFFSETS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_1_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_1_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_1_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_1_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_KEY_BUILDER_OFFSETS_1_FIELDS[] =
{
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_1_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_KEY_BUILDER_OFFSETS_1 *****/
const ru_reg_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_1_REG =
{
    "PROFILES_KEY_BUILDER_OFFSETS_1",
#if RU_INCLUDE_DESC
    "KEY_BUILDER_OFFSETS 0..39 Register",
    "Maps between the 64B master key and the 16B tcam Keys.\nThe four low bytes of a tcam key can each be assigned with any aligned byte from the master key according to its configured index.\nThe 6 higher 16bit words of a tcam key can each be assigned with any aligned 16         bit words from the master key according to its configured index.\nEach configured index is 8 bits.\nMax value for byte offset is 63.\nMax value for 16bit word offset is 31.\nTotal bits in this RAM is:NUM_OF_TCAMS*(4+6)*8. LSB bits on lowest address corresponds to TCAM 0&LSB Byte of its key.\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_1_REG_OFFSET },
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_1_REG_RAM_CNT,
    580,
    1886,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_KEY_BUILDER_OFFSETS_2, TYPE: Type_TCAM_TCAM_PROFILES_KEY_BUILDER_OFFSETS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_2_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_2_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_2_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_2_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_KEY_BUILDER_OFFSETS_2_FIELDS[] =
{
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_2_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_KEY_BUILDER_OFFSETS_2 *****/
const ru_reg_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_2_REG =
{
    "PROFILES_KEY_BUILDER_OFFSETS_2",
#if RU_INCLUDE_DESC
    "KEY_BUILDER_OFFSETS 0..39 Register",
    "Maps between the 64B master key and the 16B tcam Keys.\nThe four low bytes of a tcam key can each be assigned with any aligned byte from the master key according to its configured index.\nThe 6 higher 16bit words of a tcam key can each be assigned with any aligned 16         bit words from the master key according to its configured index.\nEach configured index is 8 bits.\nMax value for byte offset is 63.\nMax value for 16bit word offset is 31.\nTotal bits in this RAM is:NUM_OF_TCAMS*(4+6)*8. LSB bits on lowest address corresponds to TCAM 0&LSB Byte of its key.\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_2_REG_OFFSET },
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_2_REG_RAM_CNT,
    580,
    1887,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_KEY_BUILDER_OFFSETS_3, TYPE: Type_TCAM_TCAM_PROFILES_KEY_BUILDER_OFFSETS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_3_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_3_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_3_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_3_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_KEY_BUILDER_OFFSETS_3_FIELDS[] =
{
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_3_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_KEY_BUILDER_OFFSETS_3 *****/
const ru_reg_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_3_REG =
{
    "PROFILES_KEY_BUILDER_OFFSETS_3",
#if RU_INCLUDE_DESC
    "KEY_BUILDER_OFFSETS 0..39 Register",
    "Maps between the 64B master key and the 16B tcam Keys.\nThe four low bytes of a tcam key can each be assigned with any aligned byte from the master key according to its configured index.\nThe 6 higher 16bit words of a tcam key can each be assigned with any aligned 16         bit words from the master key according to its configured index.\nEach configured index is 8 bits.\nMax value for byte offset is 63.\nMax value for 16bit word offset is 31.\nTotal bits in this RAM is:NUM_OF_TCAMS*(4+6)*8. LSB bits on lowest address corresponds to TCAM 0&LSB Byte of its key.\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_3_REG_OFFSET },
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_3_REG_RAM_CNT,
    580,
    1888,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_3_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_KEY_BUILDER_OFFSETS_4, TYPE: Type_TCAM_TCAM_PROFILES_KEY_BUILDER_OFFSETS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_4_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_4_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_4_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_4_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_KEY_BUILDER_OFFSETS_4_FIELDS[] =
{
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_4_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_KEY_BUILDER_OFFSETS_4 *****/
const ru_reg_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_4_REG =
{
    "PROFILES_KEY_BUILDER_OFFSETS_4",
#if RU_INCLUDE_DESC
    "KEY_BUILDER_OFFSETS 0..39 Register",
    "Maps between the 64B master key and the 16B tcam Keys.\nThe four low bytes of a tcam key can each be assigned with any aligned byte from the master key according to its configured index.\nThe 6 higher 16bit words of a tcam key can each be assigned with any aligned 16         bit words from the master key according to its configured index.\nEach configured index is 8 bits.\nMax value for byte offset is 63.\nMax value for 16bit word offset is 31.\nTotal bits in this RAM is:NUM_OF_TCAMS*(4+6)*8. LSB bits on lowest address corresponds to TCAM 0&LSB Byte of its key.\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_4_REG_OFFSET },
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_4_REG_RAM_CNT,
    580,
    1889,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_4_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_KEY_BUILDER_OFFSETS_5, TYPE: Type_TCAM_TCAM_PROFILES_KEY_BUILDER_OFFSETS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_5_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_5_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_5_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_5_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_KEY_BUILDER_OFFSETS_5_FIELDS[] =
{
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_5_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_KEY_BUILDER_OFFSETS_5 *****/
const ru_reg_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_5_REG =
{
    "PROFILES_KEY_BUILDER_OFFSETS_5",
#if RU_INCLUDE_DESC
    "KEY_BUILDER_OFFSETS 0..39 Register",
    "Maps between the 64B master key and the 16B tcam Keys.\nThe four low bytes of a tcam key can each be assigned with any aligned byte from the master key according to its configured index.\nThe 6 higher 16bit words of a tcam key can each be assigned with any aligned 16         bit words from the master key according to its configured index.\nEach configured index is 8 bits.\nMax value for byte offset is 63.\nMax value for 16bit word offset is 31.\nTotal bits in this RAM is:NUM_OF_TCAMS*(4+6)*8. LSB bits on lowest address corresponds to TCAM 0&LSB Byte of its key.\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_5_REG_OFFSET },
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_5_REG_RAM_CNT,
    580,
    1890,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_5_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_KEY_BUILDER_OFFSETS_6, TYPE: Type_TCAM_TCAM_PROFILES_KEY_BUILDER_OFFSETS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_6_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_6_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_6_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_6_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_KEY_BUILDER_OFFSETS_6_FIELDS[] =
{
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_6_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_KEY_BUILDER_OFFSETS_6 *****/
const ru_reg_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_6_REG =
{
    "PROFILES_KEY_BUILDER_OFFSETS_6",
#if RU_INCLUDE_DESC
    "KEY_BUILDER_OFFSETS 0..39 Register",
    "Maps between the 64B master key and the 16B tcam Keys.\nThe four low bytes of a tcam key can each be assigned with any aligned byte from the master key according to its configured index.\nThe 6 higher 16bit words of a tcam key can each be assigned with any aligned 16         bit words from the master key according to its configured index.\nEach configured index is 8 bits.\nMax value for byte offset is 63.\nMax value for 16bit word offset is 31.\nTotal bits in this RAM is:NUM_OF_TCAMS*(4+6)*8. LSB bits on lowest address corresponds to TCAM 0&LSB Byte of its key.\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_6_REG_OFFSET },
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_6_REG_RAM_CNT,
    580,
    1891,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_6_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_KEY_BUILDER_OFFSETS_7, TYPE: Type_TCAM_TCAM_PROFILES_KEY_BUILDER_OFFSETS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_7_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_7_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_7_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_7_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_KEY_BUILDER_OFFSETS_7_FIELDS[] =
{
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_7_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_KEY_BUILDER_OFFSETS_7 *****/
const ru_reg_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_7_REG =
{
    "PROFILES_KEY_BUILDER_OFFSETS_7",
#if RU_INCLUDE_DESC
    "KEY_BUILDER_OFFSETS 0..39 Register",
    "Maps between the 64B master key and the 16B tcam Keys.\nThe four low bytes of a tcam key can each be assigned with any aligned byte from the master key according to its configured index.\nThe 6 higher 16bit words of a tcam key can each be assigned with any aligned 16         bit words from the master key according to its configured index.\nEach configured index is 8 bits.\nMax value for byte offset is 63.\nMax value for 16bit word offset is 31.\nTotal bits in this RAM is:NUM_OF_TCAMS*(4+6)*8. LSB bits on lowest address corresponds to TCAM 0&LSB Byte of its key.\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_7_REG_OFFSET },
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_7_REG_RAM_CNT,
    580,
    1892,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_7_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_KEY_BUILDER_OFFSETS_8, TYPE: Type_TCAM_TCAM_PROFILES_KEY_BUILDER_OFFSETS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_8_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_8_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_8_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_8_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_KEY_BUILDER_OFFSETS_8_FIELDS[] =
{
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_8_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_KEY_BUILDER_OFFSETS_8 *****/
const ru_reg_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_8_REG =
{
    "PROFILES_KEY_BUILDER_OFFSETS_8",
#if RU_INCLUDE_DESC
    "KEY_BUILDER_OFFSETS 0..39 Register",
    "Maps between the 64B master key and the 16B tcam Keys.\nThe four low bytes of a tcam key can each be assigned with any aligned byte from the master key according to its configured index.\nThe 6 higher 16bit words of a tcam key can each be assigned with any aligned 16         bit words from the master key according to its configured index.\nEach configured index is 8 bits.\nMax value for byte offset is 63.\nMax value for 16bit word offset is 31.\nTotal bits in this RAM is:NUM_OF_TCAMS*(4+6)*8. LSB bits on lowest address corresponds to TCAM 0&LSB Byte of its key.\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_8_REG_OFFSET },
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_8_REG_RAM_CNT,
    580,
    1893,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_8_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_KEY_BUILDER_OFFSETS_9, TYPE: Type_TCAM_TCAM_PROFILES_KEY_BUILDER_OFFSETS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_9_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_9_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_9_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_9_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_KEY_BUILDER_OFFSETS_9_FIELDS[] =
{
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_9_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_KEY_BUILDER_OFFSETS_9 *****/
const ru_reg_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_9_REG =
{
    "PROFILES_KEY_BUILDER_OFFSETS_9",
#if RU_INCLUDE_DESC
    "KEY_BUILDER_OFFSETS 0..39 Register",
    "Maps between the 64B master key and the 16B tcam Keys.\nThe four low bytes of a tcam key can each be assigned with any aligned byte from the master key according to its configured index.\nThe 6 higher 16bit words of a tcam key can each be assigned with any aligned 16         bit words from the master key according to its configured index.\nEach configured index is 8 bits.\nMax value for byte offset is 63.\nMax value for 16bit word offset is 31.\nTotal bits in this RAM is:NUM_OF_TCAMS*(4+6)*8. LSB bits on lowest address corresponds to TCAM 0&LSB Byte of its key.\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_9_REG_OFFSET },
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_9_REG_RAM_CNT,
    580,
    1894,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_9_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_KEY_BUILDER_OFFSETS_10, TYPE: Type_TCAM_TCAM_PROFILES_KEY_BUILDER_OFFSETS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_10_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_10_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_10_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_10_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_KEY_BUILDER_OFFSETS_10_FIELDS[] =
{
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_10_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_KEY_BUILDER_OFFSETS_10 *****/
const ru_reg_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_10_REG =
{
    "PROFILES_KEY_BUILDER_OFFSETS_10",
#if RU_INCLUDE_DESC
    "KEY_BUILDER_OFFSETS 0..39 Register",
    "Maps between the 64B master key and the 16B tcam Keys.\nThe four low bytes of a tcam key can each be assigned with any aligned byte from the master key according to its configured index.\nThe 6 higher 16bit words of a tcam key can each be assigned with any aligned 16         bit words from the master key according to its configured index.\nEach configured index is 8 bits.\nMax value for byte offset is 63.\nMax value for 16bit word offset is 31.\nTotal bits in this RAM is:NUM_OF_TCAMS*(4+6)*8. LSB bits on lowest address corresponds to TCAM 0&LSB Byte of its key.\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_10_REG_OFFSET },
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_10_REG_RAM_CNT,
    580,
    1895,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_10_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_KEY_BUILDER_OFFSETS_11, TYPE: Type_TCAM_TCAM_PROFILES_KEY_BUILDER_OFFSETS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_11_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_11_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_11_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_11_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_KEY_BUILDER_OFFSETS_11_FIELDS[] =
{
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_11_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_KEY_BUILDER_OFFSETS_11 *****/
const ru_reg_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_11_REG =
{
    "PROFILES_KEY_BUILDER_OFFSETS_11",
#if RU_INCLUDE_DESC
    "KEY_BUILDER_OFFSETS 0..39 Register",
    "Maps between the 64B master key and the 16B tcam Keys.\nThe four low bytes of a tcam key can each be assigned with any aligned byte from the master key according to its configured index.\nThe 6 higher 16bit words of a tcam key can each be assigned with any aligned 16         bit words from the master key according to its configured index.\nEach configured index is 8 bits.\nMax value for byte offset is 63.\nMax value for 16bit word offset is 31.\nTotal bits in this RAM is:NUM_OF_TCAMS*(4+6)*8. LSB bits on lowest address corresponds to TCAM 0&LSB Byte of its key.\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_11_REG_OFFSET },
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_11_REG_RAM_CNT,
    580,
    1896,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_11_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_KEY_BUILDER_OFFSETS_12, TYPE: Type_TCAM_TCAM_PROFILES_KEY_BUILDER_OFFSETS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_12_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_12_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_12_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_12_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_KEY_BUILDER_OFFSETS_12_FIELDS[] =
{
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_12_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_KEY_BUILDER_OFFSETS_12 *****/
const ru_reg_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_12_REG =
{
    "PROFILES_KEY_BUILDER_OFFSETS_12",
#if RU_INCLUDE_DESC
    "KEY_BUILDER_OFFSETS 0..39 Register",
    "Maps between the 64B master key and the 16B tcam Keys.\nThe four low bytes of a tcam key can each be assigned with any aligned byte from the master key according to its configured index.\nThe 6 higher 16bit words of a tcam key can each be assigned with any aligned 16         bit words from the master key according to its configured index.\nEach configured index is 8 bits.\nMax value for byte offset is 63.\nMax value for 16bit word offset is 31.\nTotal bits in this RAM is:NUM_OF_TCAMS*(4+6)*8. LSB bits on lowest address corresponds to TCAM 0&LSB Byte of its key.\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_12_REG_OFFSET },
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_12_REG_RAM_CNT,
    580,
    1897,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_12_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_KEY_BUILDER_OFFSETS_13, TYPE: Type_TCAM_TCAM_PROFILES_KEY_BUILDER_OFFSETS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_13_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_13_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_13_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_13_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_KEY_BUILDER_OFFSETS_13_FIELDS[] =
{
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_13_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_KEY_BUILDER_OFFSETS_13 *****/
const ru_reg_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_13_REG =
{
    "PROFILES_KEY_BUILDER_OFFSETS_13",
#if RU_INCLUDE_DESC
    "KEY_BUILDER_OFFSETS 0..39 Register",
    "Maps between the 64B master key and the 16B tcam Keys.\nThe four low bytes of a tcam key can each be assigned with any aligned byte from the master key according to its configured index.\nThe 6 higher 16bit words of a tcam key can each be assigned with any aligned 16         bit words from the master key according to its configured index.\nEach configured index is 8 bits.\nMax value for byte offset is 63.\nMax value for 16bit word offset is 31.\nTotal bits in this RAM is:NUM_OF_TCAMS*(4+6)*8. LSB bits on lowest address corresponds to TCAM 0&LSB Byte of its key.\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_13_REG_OFFSET },
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_13_REG_RAM_CNT,
    580,
    1898,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_13_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_KEY_BUILDER_OFFSETS_14, TYPE: Type_TCAM_TCAM_PROFILES_KEY_BUILDER_OFFSETS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_14_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_14_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_14_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_14_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_KEY_BUILDER_OFFSETS_14_FIELDS[] =
{
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_14_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_KEY_BUILDER_OFFSETS_14 *****/
const ru_reg_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_14_REG =
{
    "PROFILES_KEY_BUILDER_OFFSETS_14",
#if RU_INCLUDE_DESC
    "KEY_BUILDER_OFFSETS 0..39 Register",
    "Maps between the 64B master key and the 16B tcam Keys.\nThe four low bytes of a tcam key can each be assigned with any aligned byte from the master key according to its configured index.\nThe 6 higher 16bit words of a tcam key can each be assigned with any aligned 16         bit words from the master key according to its configured index.\nEach configured index is 8 bits.\nMax value for byte offset is 63.\nMax value for 16bit word offset is 31.\nTotal bits in this RAM is:NUM_OF_TCAMS*(4+6)*8. LSB bits on lowest address corresponds to TCAM 0&LSB Byte of its key.\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_14_REG_OFFSET },
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_14_REG_RAM_CNT,
    580,
    1899,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_14_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_KEY_BUILDER_OFFSETS_15, TYPE: Type_TCAM_TCAM_PROFILES_KEY_BUILDER_OFFSETS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_15_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_15_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_15_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_15_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_KEY_BUILDER_OFFSETS_15_FIELDS[] =
{
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_15_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_KEY_BUILDER_OFFSETS_15 *****/
const ru_reg_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_15_REG =
{
    "PROFILES_KEY_BUILDER_OFFSETS_15",
#if RU_INCLUDE_DESC
    "KEY_BUILDER_OFFSETS 0..39 Register",
    "Maps between the 64B master key and the 16B tcam Keys.\nThe four low bytes of a tcam key can each be assigned with any aligned byte from the master key according to its configured index.\nThe 6 higher 16bit words of a tcam key can each be assigned with any aligned 16         bit words from the master key according to its configured index.\nEach configured index is 8 bits.\nMax value for byte offset is 63.\nMax value for 16bit word offset is 31.\nTotal bits in this RAM is:NUM_OF_TCAMS*(4+6)*8. LSB bits on lowest address corresponds to TCAM 0&LSB Byte of its key.\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_15_REG_OFFSET },
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_15_REG_RAM_CNT,
    580,
    1900,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_15_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_KEY_BUILDER_OFFSETS_16, TYPE: Type_TCAM_TCAM_PROFILES_KEY_BUILDER_OFFSETS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_16_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_16_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_16_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_16_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_KEY_BUILDER_OFFSETS_16_FIELDS[] =
{
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_16_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_KEY_BUILDER_OFFSETS_16 *****/
const ru_reg_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_16_REG =
{
    "PROFILES_KEY_BUILDER_OFFSETS_16",
#if RU_INCLUDE_DESC
    "KEY_BUILDER_OFFSETS 0..39 Register",
    "Maps between the 64B master key and the 16B tcam Keys.\nThe four low bytes of a tcam key can each be assigned with any aligned byte from the master key according to its configured index.\nThe 6 higher 16bit words of a tcam key can each be assigned with any aligned 16         bit words from the master key according to its configured index.\nEach configured index is 8 bits.\nMax value for byte offset is 63.\nMax value for 16bit word offset is 31.\nTotal bits in this RAM is:NUM_OF_TCAMS*(4+6)*8. LSB bits on lowest address corresponds to TCAM 0&LSB Byte of its key.\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_16_REG_OFFSET },
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_16_REG_RAM_CNT,
    580,
    1901,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_16_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_KEY_BUILDER_OFFSETS_17, TYPE: Type_TCAM_TCAM_PROFILES_KEY_BUILDER_OFFSETS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_17_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_17_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_17_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_17_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_KEY_BUILDER_OFFSETS_17_FIELDS[] =
{
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_17_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_KEY_BUILDER_OFFSETS_17 *****/
const ru_reg_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_17_REG =
{
    "PROFILES_KEY_BUILDER_OFFSETS_17",
#if RU_INCLUDE_DESC
    "KEY_BUILDER_OFFSETS 0..39 Register",
    "Maps between the 64B master key and the 16B tcam Keys.\nThe four low bytes of a tcam key can each be assigned with any aligned byte from the master key according to its configured index.\nThe 6 higher 16bit words of a tcam key can each be assigned with any aligned 16         bit words from the master key according to its configured index.\nEach configured index is 8 bits.\nMax value for byte offset is 63.\nMax value for 16bit word offset is 31.\nTotal bits in this RAM is:NUM_OF_TCAMS*(4+6)*8. LSB bits on lowest address corresponds to TCAM 0&LSB Byte of its key.\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_17_REG_OFFSET },
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_17_REG_RAM_CNT,
    580,
    1902,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_17_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_KEY_BUILDER_OFFSETS_18, TYPE: Type_TCAM_TCAM_PROFILES_KEY_BUILDER_OFFSETS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_18_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_18_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_18_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_18_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_KEY_BUILDER_OFFSETS_18_FIELDS[] =
{
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_18_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_KEY_BUILDER_OFFSETS_18 *****/
const ru_reg_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_18_REG =
{
    "PROFILES_KEY_BUILDER_OFFSETS_18",
#if RU_INCLUDE_DESC
    "KEY_BUILDER_OFFSETS 0..39 Register",
    "Maps between the 64B master key and the 16B tcam Keys.\nThe four low bytes of a tcam key can each be assigned with any aligned byte from the master key according to its configured index.\nThe 6 higher 16bit words of a tcam key can each be assigned with any aligned 16         bit words from the master key according to its configured index.\nEach configured index is 8 bits.\nMax value for byte offset is 63.\nMax value for 16bit word offset is 31.\nTotal bits in this RAM is:NUM_OF_TCAMS*(4+6)*8. LSB bits on lowest address corresponds to TCAM 0&LSB Byte of its key.\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_18_REG_OFFSET },
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_18_REG_RAM_CNT,
    580,
    1903,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_18_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_KEY_BUILDER_OFFSETS_19, TYPE: Type_TCAM_TCAM_PROFILES_KEY_BUILDER_OFFSETS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_19_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_19_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_19_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_19_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_KEY_BUILDER_OFFSETS_19_FIELDS[] =
{
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_19_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_KEY_BUILDER_OFFSETS_19 *****/
const ru_reg_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_19_REG =
{
    "PROFILES_KEY_BUILDER_OFFSETS_19",
#if RU_INCLUDE_DESC
    "KEY_BUILDER_OFFSETS 0..39 Register",
    "Maps between the 64B master key and the 16B tcam Keys.\nThe four low bytes of a tcam key can each be assigned with any aligned byte from the master key according to its configured index.\nThe 6 higher 16bit words of a tcam key can each be assigned with any aligned 16         bit words from the master key according to its configured index.\nEach configured index is 8 bits.\nMax value for byte offset is 63.\nMax value for 16bit word offset is 31.\nTotal bits in this RAM is:NUM_OF_TCAMS*(4+6)*8. LSB bits on lowest address corresponds to TCAM 0&LSB Byte of its key.\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_19_REG_OFFSET },
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_19_REG_RAM_CNT,
    580,
    1904,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_19_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_KEY_BUILDER_OFFSETS_20, TYPE: Type_TCAM_TCAM_PROFILES_KEY_BUILDER_OFFSETS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_20_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_20_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_20_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_20_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_KEY_BUILDER_OFFSETS_20_FIELDS[] =
{
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_20_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_KEY_BUILDER_OFFSETS_20 *****/
const ru_reg_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_20_REG =
{
    "PROFILES_KEY_BUILDER_OFFSETS_20",
#if RU_INCLUDE_DESC
    "KEY_BUILDER_OFFSETS 0..39 Register",
    "Maps between the 64B master key and the 16B tcam Keys.\nThe four low bytes of a tcam key can each be assigned with any aligned byte from the master key according to its configured index.\nThe 6 higher 16bit words of a tcam key can each be assigned with any aligned 16         bit words from the master key according to its configured index.\nEach configured index is 8 bits.\nMax value for byte offset is 63.\nMax value for 16bit word offset is 31.\nTotal bits in this RAM is:NUM_OF_TCAMS*(4+6)*8. LSB bits on lowest address corresponds to TCAM 0&LSB Byte of its key.\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_20_REG_OFFSET },
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_20_REG_RAM_CNT,
    580,
    1905,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_20_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_KEY_BUILDER_OFFSETS_21, TYPE: Type_TCAM_TCAM_PROFILES_KEY_BUILDER_OFFSETS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_21_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_21_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_21_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_21_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_KEY_BUILDER_OFFSETS_21_FIELDS[] =
{
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_21_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_KEY_BUILDER_OFFSETS_21 *****/
const ru_reg_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_21_REG =
{
    "PROFILES_KEY_BUILDER_OFFSETS_21",
#if RU_INCLUDE_DESC
    "KEY_BUILDER_OFFSETS 0..39 Register",
    "Maps between the 64B master key and the 16B tcam Keys.\nThe four low bytes of a tcam key can each be assigned with any aligned byte from the master key according to its configured index.\nThe 6 higher 16bit words of a tcam key can each be assigned with any aligned 16         bit words from the master key according to its configured index.\nEach configured index is 8 bits.\nMax value for byte offset is 63.\nMax value for 16bit word offset is 31.\nTotal bits in this RAM is:NUM_OF_TCAMS*(4+6)*8. LSB bits on lowest address corresponds to TCAM 0&LSB Byte of its key.\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_21_REG_OFFSET },
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_21_REG_RAM_CNT,
    580,
    1906,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_21_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_KEY_BUILDER_OFFSETS_22, TYPE: Type_TCAM_TCAM_PROFILES_KEY_BUILDER_OFFSETS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_22_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_22_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_22_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_22_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_KEY_BUILDER_OFFSETS_22_FIELDS[] =
{
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_22_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_KEY_BUILDER_OFFSETS_22 *****/
const ru_reg_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_22_REG =
{
    "PROFILES_KEY_BUILDER_OFFSETS_22",
#if RU_INCLUDE_DESC
    "KEY_BUILDER_OFFSETS 0..39 Register",
    "Maps between the 64B master key and the 16B tcam Keys.\nThe four low bytes of a tcam key can each be assigned with any aligned byte from the master key according to its configured index.\nThe 6 higher 16bit words of a tcam key can each be assigned with any aligned 16         bit words from the master key according to its configured index.\nEach configured index is 8 bits.\nMax value for byte offset is 63.\nMax value for 16bit word offset is 31.\nTotal bits in this RAM is:NUM_OF_TCAMS*(4+6)*8. LSB bits on lowest address corresponds to TCAM 0&LSB Byte of its key.\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_22_REG_OFFSET },
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_22_REG_RAM_CNT,
    580,
    1907,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_22_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_KEY_BUILDER_OFFSETS_23, TYPE: Type_TCAM_TCAM_PROFILES_KEY_BUILDER_OFFSETS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_23_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_23_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_23_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_23_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_KEY_BUILDER_OFFSETS_23_FIELDS[] =
{
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_23_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_KEY_BUILDER_OFFSETS_23 *****/
const ru_reg_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_23_REG =
{
    "PROFILES_KEY_BUILDER_OFFSETS_23",
#if RU_INCLUDE_DESC
    "KEY_BUILDER_OFFSETS 0..39 Register",
    "Maps between the 64B master key and the 16B tcam Keys.\nThe four low bytes of a tcam key can each be assigned with any aligned byte from the master key according to its configured index.\nThe 6 higher 16bit words of a tcam key can each be assigned with any aligned 16         bit words from the master key according to its configured index.\nEach configured index is 8 bits.\nMax value for byte offset is 63.\nMax value for 16bit word offset is 31.\nTotal bits in this RAM is:NUM_OF_TCAMS*(4+6)*8. LSB bits on lowest address corresponds to TCAM 0&LSB Byte of its key.\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_23_REG_OFFSET },
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_23_REG_RAM_CNT,
    580,
    1908,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_23_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_KEY_BUILDER_OFFSETS_24, TYPE: Type_TCAM_TCAM_PROFILES_KEY_BUILDER_OFFSETS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_24_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_24_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_24_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_24_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_KEY_BUILDER_OFFSETS_24_FIELDS[] =
{
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_24_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_KEY_BUILDER_OFFSETS_24 *****/
const ru_reg_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_24_REG =
{
    "PROFILES_KEY_BUILDER_OFFSETS_24",
#if RU_INCLUDE_DESC
    "KEY_BUILDER_OFFSETS 0..39 Register",
    "Maps between the 64B master key and the 16B tcam Keys.\nThe four low bytes of a tcam key can each be assigned with any aligned byte from the master key according to its configured index.\nThe 6 higher 16bit words of a tcam key can each be assigned with any aligned 16         bit words from the master key according to its configured index.\nEach configured index is 8 bits.\nMax value for byte offset is 63.\nMax value for 16bit word offset is 31.\nTotal bits in this RAM is:NUM_OF_TCAMS*(4+6)*8. LSB bits on lowest address corresponds to TCAM 0&LSB Byte of its key.\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_24_REG_OFFSET },
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_24_REG_RAM_CNT,
    580,
    1909,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_24_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_KEY_BUILDER_OFFSETS_25, TYPE: Type_TCAM_TCAM_PROFILES_KEY_BUILDER_OFFSETS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_25_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_25_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_25_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_25_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_KEY_BUILDER_OFFSETS_25_FIELDS[] =
{
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_25_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_KEY_BUILDER_OFFSETS_25 *****/
const ru_reg_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_25_REG =
{
    "PROFILES_KEY_BUILDER_OFFSETS_25",
#if RU_INCLUDE_DESC
    "KEY_BUILDER_OFFSETS 0..39 Register",
    "Maps between the 64B master key and the 16B tcam Keys.\nThe four low bytes of a tcam key can each be assigned with any aligned byte from the master key according to its configured index.\nThe 6 higher 16bit words of a tcam key can each be assigned with any aligned 16         bit words from the master key according to its configured index.\nEach configured index is 8 bits.\nMax value for byte offset is 63.\nMax value for 16bit word offset is 31.\nTotal bits in this RAM is:NUM_OF_TCAMS*(4+6)*8. LSB bits on lowest address corresponds to TCAM 0&LSB Byte of its key.\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_25_REG_OFFSET },
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_25_REG_RAM_CNT,
    580,
    1910,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_25_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_KEY_BUILDER_OFFSETS_26, TYPE: Type_TCAM_TCAM_PROFILES_KEY_BUILDER_OFFSETS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_26_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_26_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_26_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_26_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_KEY_BUILDER_OFFSETS_26_FIELDS[] =
{
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_26_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_KEY_BUILDER_OFFSETS_26 *****/
const ru_reg_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_26_REG =
{
    "PROFILES_KEY_BUILDER_OFFSETS_26",
#if RU_INCLUDE_DESC
    "KEY_BUILDER_OFFSETS 0..39 Register",
    "Maps between the 64B master key and the 16B tcam Keys.\nThe four low bytes of a tcam key can each be assigned with any aligned byte from the master key according to its configured index.\nThe 6 higher 16bit words of a tcam key can each be assigned with any aligned 16         bit words from the master key according to its configured index.\nEach configured index is 8 bits.\nMax value for byte offset is 63.\nMax value for 16bit word offset is 31.\nTotal bits in this RAM is:NUM_OF_TCAMS*(4+6)*8. LSB bits on lowest address corresponds to TCAM 0&LSB Byte of its key.\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_26_REG_OFFSET },
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_26_REG_RAM_CNT,
    580,
    1911,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_26_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_KEY_BUILDER_OFFSETS_27, TYPE: Type_TCAM_TCAM_PROFILES_KEY_BUILDER_OFFSETS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_27_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_27_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_27_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_27_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_KEY_BUILDER_OFFSETS_27_FIELDS[] =
{
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_27_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_KEY_BUILDER_OFFSETS_27 *****/
const ru_reg_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_27_REG =
{
    "PROFILES_KEY_BUILDER_OFFSETS_27",
#if RU_INCLUDE_DESC
    "KEY_BUILDER_OFFSETS 0..39 Register",
    "Maps between the 64B master key and the 16B tcam Keys.\nThe four low bytes of a tcam key can each be assigned with any aligned byte from the master key according to its configured index.\nThe 6 higher 16bit words of a tcam key can each be assigned with any aligned 16         bit words from the master key according to its configured index.\nEach configured index is 8 bits.\nMax value for byte offset is 63.\nMax value for 16bit word offset is 31.\nTotal bits in this RAM is:NUM_OF_TCAMS*(4+6)*8. LSB bits on lowest address corresponds to TCAM 0&LSB Byte of its key.\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_27_REG_OFFSET },
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_27_REG_RAM_CNT,
    580,
    1912,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_27_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_KEY_BUILDER_OFFSETS_28, TYPE: Type_TCAM_TCAM_PROFILES_KEY_BUILDER_OFFSETS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_28_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_28_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_28_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_28_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_KEY_BUILDER_OFFSETS_28_FIELDS[] =
{
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_28_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_KEY_BUILDER_OFFSETS_28 *****/
const ru_reg_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_28_REG =
{
    "PROFILES_KEY_BUILDER_OFFSETS_28",
#if RU_INCLUDE_DESC
    "KEY_BUILDER_OFFSETS 0..39 Register",
    "Maps between the 64B master key and the 16B tcam Keys.\nThe four low bytes of a tcam key can each be assigned with any aligned byte from the master key according to its configured index.\nThe 6 higher 16bit words of a tcam key can each be assigned with any aligned 16         bit words from the master key according to its configured index.\nEach configured index is 8 bits.\nMax value for byte offset is 63.\nMax value for 16bit word offset is 31.\nTotal bits in this RAM is:NUM_OF_TCAMS*(4+6)*8. LSB bits on lowest address corresponds to TCAM 0&LSB Byte of its key.\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_28_REG_OFFSET },
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_28_REG_RAM_CNT,
    580,
    1913,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_28_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_KEY_BUILDER_OFFSETS_29, TYPE: Type_TCAM_TCAM_PROFILES_KEY_BUILDER_OFFSETS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_29_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_29_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_29_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_29_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_KEY_BUILDER_OFFSETS_29_FIELDS[] =
{
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_29_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_KEY_BUILDER_OFFSETS_29 *****/
const ru_reg_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_29_REG =
{
    "PROFILES_KEY_BUILDER_OFFSETS_29",
#if RU_INCLUDE_DESC
    "KEY_BUILDER_OFFSETS 0..39 Register",
    "Maps between the 64B master key and the 16B tcam Keys.\nThe four low bytes of a tcam key can each be assigned with any aligned byte from the master key according to its configured index.\nThe 6 higher 16bit words of a tcam key can each be assigned with any aligned 16         bit words from the master key according to its configured index.\nEach configured index is 8 bits.\nMax value for byte offset is 63.\nMax value for 16bit word offset is 31.\nTotal bits in this RAM is:NUM_OF_TCAMS*(4+6)*8. LSB bits on lowest address corresponds to TCAM 0&LSB Byte of its key.\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_29_REG_OFFSET },
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_29_REG_RAM_CNT,
    580,
    1914,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_29_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_KEY_BUILDER_OFFSETS_30, TYPE: Type_TCAM_TCAM_PROFILES_KEY_BUILDER_OFFSETS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_30_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_30_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_30_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_30_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_KEY_BUILDER_OFFSETS_30_FIELDS[] =
{
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_30_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_KEY_BUILDER_OFFSETS_30 *****/
const ru_reg_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_30_REG =
{
    "PROFILES_KEY_BUILDER_OFFSETS_30",
#if RU_INCLUDE_DESC
    "KEY_BUILDER_OFFSETS 0..39 Register",
    "Maps between the 64B master key and the 16B tcam Keys.\nThe four low bytes of a tcam key can each be assigned with any aligned byte from the master key according to its configured index.\nThe 6 higher 16bit words of a tcam key can each be assigned with any aligned 16         bit words from the master key according to its configured index.\nEach configured index is 8 bits.\nMax value for byte offset is 63.\nMax value for 16bit word offset is 31.\nTotal bits in this RAM is:NUM_OF_TCAMS*(4+6)*8. LSB bits on lowest address corresponds to TCAM 0&LSB Byte of its key.\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_30_REG_OFFSET },
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_30_REG_RAM_CNT,
    580,
    1915,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_30_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_KEY_BUILDER_OFFSETS_31, TYPE: Type_TCAM_TCAM_PROFILES_KEY_BUILDER_OFFSETS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_31_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_31_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_31_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_31_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_KEY_BUILDER_OFFSETS_31_FIELDS[] =
{
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_31_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_KEY_BUILDER_OFFSETS_31 *****/
const ru_reg_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_31_REG =
{
    "PROFILES_KEY_BUILDER_OFFSETS_31",
#if RU_INCLUDE_DESC
    "KEY_BUILDER_OFFSETS 0..39 Register",
    "Maps between the 64B master key and the 16B tcam Keys.\nThe four low bytes of a tcam key can each be assigned with any aligned byte from the master key according to its configured index.\nThe 6 higher 16bit words of a tcam key can each be assigned with any aligned 16         bit words from the master key according to its configured index.\nEach configured index is 8 bits.\nMax value for byte offset is 63.\nMax value for 16bit word offset is 31.\nTotal bits in this RAM is:NUM_OF_TCAMS*(4+6)*8. LSB bits on lowest address corresponds to TCAM 0&LSB Byte of its key.\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_31_REG_OFFSET },
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_31_REG_RAM_CNT,
    580,
    1916,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_31_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_KEY_BUILDER_OFFSETS_32, TYPE: Type_TCAM_TCAM_PROFILES_KEY_BUILDER_OFFSETS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_32_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_32_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_32_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_32_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_KEY_BUILDER_OFFSETS_32_FIELDS[] =
{
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_32_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_KEY_BUILDER_OFFSETS_32 *****/
const ru_reg_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_32_REG =
{
    "PROFILES_KEY_BUILDER_OFFSETS_32",
#if RU_INCLUDE_DESC
    "KEY_BUILDER_OFFSETS 0..39 Register",
    "Maps between the 64B master key and the 16B tcam Keys.\nThe four low bytes of a tcam key can each be assigned with any aligned byte from the master key according to its configured index.\nThe 6 higher 16bit words of a tcam key can each be assigned with any aligned 16         bit words from the master key according to its configured index.\nEach configured index is 8 bits.\nMax value for byte offset is 63.\nMax value for 16bit word offset is 31.\nTotal bits in this RAM is:NUM_OF_TCAMS*(4+6)*8. LSB bits on lowest address corresponds to TCAM 0&LSB Byte of its key.\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_32_REG_OFFSET },
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_32_REG_RAM_CNT,
    580,
    1917,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_32_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_KEY_BUILDER_OFFSETS_33, TYPE: Type_TCAM_TCAM_PROFILES_KEY_BUILDER_OFFSETS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_33_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_33_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_33_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_33_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_KEY_BUILDER_OFFSETS_33_FIELDS[] =
{
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_33_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_KEY_BUILDER_OFFSETS_33 *****/
const ru_reg_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_33_REG =
{
    "PROFILES_KEY_BUILDER_OFFSETS_33",
#if RU_INCLUDE_DESC
    "KEY_BUILDER_OFFSETS 0..39 Register",
    "Maps between the 64B master key and the 16B tcam Keys.\nThe four low bytes of a tcam key can each be assigned with any aligned byte from the master key according to its configured index.\nThe 6 higher 16bit words of a tcam key can each be assigned with any aligned 16         bit words from the master key according to its configured index.\nEach configured index is 8 bits.\nMax value for byte offset is 63.\nMax value for 16bit word offset is 31.\nTotal bits in this RAM is:NUM_OF_TCAMS*(4+6)*8. LSB bits on lowest address corresponds to TCAM 0&LSB Byte of its key.\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_33_REG_OFFSET },
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_33_REG_RAM_CNT,
    580,
    1918,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_33_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_KEY_BUILDER_OFFSETS_34, TYPE: Type_TCAM_TCAM_PROFILES_KEY_BUILDER_OFFSETS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_34_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_34_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_34_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_34_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_KEY_BUILDER_OFFSETS_34_FIELDS[] =
{
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_34_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_KEY_BUILDER_OFFSETS_34 *****/
const ru_reg_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_34_REG =
{
    "PROFILES_KEY_BUILDER_OFFSETS_34",
#if RU_INCLUDE_DESC
    "KEY_BUILDER_OFFSETS 0..39 Register",
    "Maps between the 64B master key and the 16B tcam Keys.\nThe four low bytes of a tcam key can each be assigned with any aligned byte from the master key according to its configured index.\nThe 6 higher 16bit words of a tcam key can each be assigned with any aligned 16         bit words from the master key according to its configured index.\nEach configured index is 8 bits.\nMax value for byte offset is 63.\nMax value for 16bit word offset is 31.\nTotal bits in this RAM is:NUM_OF_TCAMS*(4+6)*8. LSB bits on lowest address corresponds to TCAM 0&LSB Byte of its key.\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_34_REG_OFFSET },
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_34_REG_RAM_CNT,
    580,
    1919,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_34_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_KEY_BUILDER_OFFSETS_35, TYPE: Type_TCAM_TCAM_PROFILES_KEY_BUILDER_OFFSETS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_35_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_35_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_35_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_35_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_KEY_BUILDER_OFFSETS_35_FIELDS[] =
{
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_35_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_KEY_BUILDER_OFFSETS_35 *****/
const ru_reg_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_35_REG =
{
    "PROFILES_KEY_BUILDER_OFFSETS_35",
#if RU_INCLUDE_DESC
    "KEY_BUILDER_OFFSETS 0..39 Register",
    "Maps between the 64B master key and the 16B tcam Keys.\nThe four low bytes of a tcam key can each be assigned with any aligned byte from the master key according to its configured index.\nThe 6 higher 16bit words of a tcam key can each be assigned with any aligned 16         bit words from the master key according to its configured index.\nEach configured index is 8 bits.\nMax value for byte offset is 63.\nMax value for 16bit word offset is 31.\nTotal bits in this RAM is:NUM_OF_TCAMS*(4+6)*8. LSB bits on lowest address corresponds to TCAM 0&LSB Byte of its key.\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_35_REG_OFFSET },
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_35_REG_RAM_CNT,
    580,
    1920,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_35_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_KEY_BUILDER_OFFSETS_36, TYPE: Type_TCAM_TCAM_PROFILES_KEY_BUILDER_OFFSETS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_36_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_36_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_36_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_36_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_KEY_BUILDER_OFFSETS_36_FIELDS[] =
{
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_36_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_KEY_BUILDER_OFFSETS_36 *****/
const ru_reg_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_36_REG =
{
    "PROFILES_KEY_BUILDER_OFFSETS_36",
#if RU_INCLUDE_DESC
    "KEY_BUILDER_OFFSETS 0..39 Register",
    "Maps between the 64B master key and the 16B tcam Keys.\nThe four low bytes of a tcam key can each be assigned with any aligned byte from the master key according to its configured index.\nThe 6 higher 16bit words of a tcam key can each be assigned with any aligned 16         bit words from the master key according to its configured index.\nEach configured index is 8 bits.\nMax value for byte offset is 63.\nMax value for 16bit word offset is 31.\nTotal bits in this RAM is:NUM_OF_TCAMS*(4+6)*8. LSB bits on lowest address corresponds to TCAM 0&LSB Byte of its key.\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_36_REG_OFFSET },
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_36_REG_RAM_CNT,
    580,
    1921,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_36_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_KEY_BUILDER_OFFSETS_37, TYPE: Type_TCAM_TCAM_PROFILES_KEY_BUILDER_OFFSETS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_37_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_37_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_37_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_37_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_KEY_BUILDER_OFFSETS_37_FIELDS[] =
{
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_37_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_KEY_BUILDER_OFFSETS_37 *****/
const ru_reg_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_37_REG =
{
    "PROFILES_KEY_BUILDER_OFFSETS_37",
#if RU_INCLUDE_DESC
    "KEY_BUILDER_OFFSETS 0..39 Register",
    "Maps between the 64B master key and the 16B tcam Keys.\nThe four low bytes of a tcam key can each be assigned with any aligned byte from the master key according to its configured index.\nThe 6 higher 16bit words of a tcam key can each be assigned with any aligned 16         bit words from the master key according to its configured index.\nEach configured index is 8 bits.\nMax value for byte offset is 63.\nMax value for 16bit word offset is 31.\nTotal bits in this RAM is:NUM_OF_TCAMS*(4+6)*8. LSB bits on lowest address corresponds to TCAM 0&LSB Byte of its key.\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_37_REG_OFFSET },
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_37_REG_RAM_CNT,
    580,
    1922,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_37_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_KEY_BUILDER_OFFSETS_38, TYPE: Type_TCAM_TCAM_PROFILES_KEY_BUILDER_OFFSETS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_38_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_38_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_38_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_38_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_KEY_BUILDER_OFFSETS_38_FIELDS[] =
{
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_38_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_KEY_BUILDER_OFFSETS_38 *****/
const ru_reg_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_38_REG =
{
    "PROFILES_KEY_BUILDER_OFFSETS_38",
#if RU_INCLUDE_DESC
    "KEY_BUILDER_OFFSETS 0..39 Register",
    "Maps between the 64B master key and the 16B tcam Keys.\nThe four low bytes of a tcam key can each be assigned with any aligned byte from the master key according to its configured index.\nThe 6 higher 16bit words of a tcam key can each be assigned with any aligned 16         bit words from the master key according to its configured index.\nEach configured index is 8 bits.\nMax value for byte offset is 63.\nMax value for 16bit word offset is 31.\nTotal bits in this RAM is:NUM_OF_TCAMS*(4+6)*8. LSB bits on lowest address corresponds to TCAM 0&LSB Byte of its key.\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_38_REG_OFFSET },
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_38_REG_RAM_CNT,
    580,
    1923,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_38_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_KEY_BUILDER_OFFSETS_39, TYPE: Type_TCAM_TCAM_PROFILES_KEY_BUILDER_OFFSETS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_39_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_39_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_39_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_39_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_KEY_BUILDER_OFFSETS_39_FIELDS[] =
{
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_39_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_KEY_BUILDER_OFFSETS_39 *****/
const ru_reg_rec TCAM_PROFILES_KEY_BUILDER_OFFSETS_39_REG =
{
    "PROFILES_KEY_BUILDER_OFFSETS_39",
#if RU_INCLUDE_DESC
    "KEY_BUILDER_OFFSETS 0..39 Register",
    "Maps between the 64B master key and the 16B tcam Keys.\nThe four low bytes of a tcam key can each be assigned with any aligned byte from the master key according to its configured index.\nThe 6 higher 16bit words of a tcam key can each be assigned with any aligned 16         bit words from the master key according to its configured index.\nEach configured index is 8 bits.\nMax value for byte offset is 63.\nMax value for 16bit word offset is 31.\nTotal bits in this RAM is:NUM_OF_TCAMS*(4+6)*8. LSB bits on lowest address corresponds to TCAM 0&LSB Byte of its key.\n",
#endif
    { TCAM_PROFILES_KEY_BUILDER_OFFSETS_39_REG_OFFSET },
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_39_REG_RAM_CNT,
    580,
    1924,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_KEY_BUILDER_OFFSETS_39_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_INNER_SHIFT_0, TYPE: Type_TCAM_TCAM_PROFILES_INNER_SHIFT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_INNER_SHIFT_0_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_0_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_INNER_SHIFT_0_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_INNER_SHIFT_0_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_INNER_SHIFT_0_FIELDS[] =
{
    &TCAM_PROFILES_INNER_SHIFT_0_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_INNER_SHIFT_0 *****/
const ru_reg_rec TCAM_PROFILES_INNER_SHIFT_0_REG =
{
    "PROFILES_INNER_SHIFT_0",
#if RU_INCLUDE_DESC
    "INNER_SHIFT 0..31 Register",
    "The context output can be shifted right based on the TCAM match vector and amount of nibbles to be shifted per match vector bit.\nEach configured shift is 4 bits.\nMax value for byte offset is 15.\nTotal bits in this RAM is:NUM_OF_CNTX*NUM_OF_TCAMS*4.\nLSB bits on lowest address corresponds to CNTX0,TCAM0.\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_0_REG_OFFSET },
    TCAM_PROFILES_INNER_SHIFT_0_REG_RAM_CNT,
    580,
    1925,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_INNER_SHIFT_0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_INNER_SHIFT_1, TYPE: Type_TCAM_TCAM_PROFILES_INNER_SHIFT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_INNER_SHIFT_1_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_1_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_INNER_SHIFT_1_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_INNER_SHIFT_1_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_INNER_SHIFT_1_FIELDS[] =
{
    &TCAM_PROFILES_INNER_SHIFT_1_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_INNER_SHIFT_1 *****/
const ru_reg_rec TCAM_PROFILES_INNER_SHIFT_1_REG =
{
    "PROFILES_INNER_SHIFT_1",
#if RU_INCLUDE_DESC
    "INNER_SHIFT 0..31 Register",
    "The context output can be shifted right based on the TCAM match vector and amount of nibbles to be shifted per match vector bit.\nEach configured shift is 4 bits.\nMax value for byte offset is 15.\nTotal bits in this RAM is:NUM_OF_CNTX*NUM_OF_TCAMS*4.\nLSB bits on lowest address corresponds to CNTX0,TCAM0.\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_1_REG_OFFSET },
    TCAM_PROFILES_INNER_SHIFT_1_REG_RAM_CNT,
    580,
    1926,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_INNER_SHIFT_1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_INNER_SHIFT_2, TYPE: Type_TCAM_TCAM_PROFILES_INNER_SHIFT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_INNER_SHIFT_2_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_2_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_INNER_SHIFT_2_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_INNER_SHIFT_2_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_INNER_SHIFT_2_FIELDS[] =
{
    &TCAM_PROFILES_INNER_SHIFT_2_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_INNER_SHIFT_2 *****/
const ru_reg_rec TCAM_PROFILES_INNER_SHIFT_2_REG =
{
    "PROFILES_INNER_SHIFT_2",
#if RU_INCLUDE_DESC
    "INNER_SHIFT 0..31 Register",
    "The context output can be shifted right based on the TCAM match vector and amount of nibbles to be shifted per match vector bit.\nEach configured shift is 4 bits.\nMax value for byte offset is 15.\nTotal bits in this RAM is:NUM_OF_CNTX*NUM_OF_TCAMS*4.\nLSB bits on lowest address corresponds to CNTX0,TCAM0.\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_2_REG_OFFSET },
    TCAM_PROFILES_INNER_SHIFT_2_REG_RAM_CNT,
    580,
    1927,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_INNER_SHIFT_2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_INNER_SHIFT_3, TYPE: Type_TCAM_TCAM_PROFILES_INNER_SHIFT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_INNER_SHIFT_3_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_3_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_INNER_SHIFT_3_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_INNER_SHIFT_3_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_INNER_SHIFT_3_FIELDS[] =
{
    &TCAM_PROFILES_INNER_SHIFT_3_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_INNER_SHIFT_3 *****/
const ru_reg_rec TCAM_PROFILES_INNER_SHIFT_3_REG =
{
    "PROFILES_INNER_SHIFT_3",
#if RU_INCLUDE_DESC
    "INNER_SHIFT 0..31 Register",
    "The context output can be shifted right based on the TCAM match vector and amount of nibbles to be shifted per match vector bit.\nEach configured shift is 4 bits.\nMax value for byte offset is 15.\nTotal bits in this RAM is:NUM_OF_CNTX*NUM_OF_TCAMS*4.\nLSB bits on lowest address corresponds to CNTX0,TCAM0.\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_3_REG_OFFSET },
    TCAM_PROFILES_INNER_SHIFT_3_REG_RAM_CNT,
    580,
    1928,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_INNER_SHIFT_3_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_INNER_SHIFT_4, TYPE: Type_TCAM_TCAM_PROFILES_INNER_SHIFT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_INNER_SHIFT_4_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_4_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_INNER_SHIFT_4_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_INNER_SHIFT_4_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_INNER_SHIFT_4_FIELDS[] =
{
    &TCAM_PROFILES_INNER_SHIFT_4_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_INNER_SHIFT_4 *****/
const ru_reg_rec TCAM_PROFILES_INNER_SHIFT_4_REG =
{
    "PROFILES_INNER_SHIFT_4",
#if RU_INCLUDE_DESC
    "INNER_SHIFT 0..31 Register",
    "The context output can be shifted right based on the TCAM match vector and amount of nibbles to be shifted per match vector bit.\nEach configured shift is 4 bits.\nMax value for byte offset is 15.\nTotal bits in this RAM is:NUM_OF_CNTX*NUM_OF_TCAMS*4.\nLSB bits on lowest address corresponds to CNTX0,TCAM0.\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_4_REG_OFFSET },
    TCAM_PROFILES_INNER_SHIFT_4_REG_RAM_CNT,
    580,
    1929,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_INNER_SHIFT_4_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_INNER_SHIFT_5, TYPE: Type_TCAM_TCAM_PROFILES_INNER_SHIFT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_INNER_SHIFT_5_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_5_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_INNER_SHIFT_5_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_INNER_SHIFT_5_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_INNER_SHIFT_5_FIELDS[] =
{
    &TCAM_PROFILES_INNER_SHIFT_5_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_INNER_SHIFT_5 *****/
const ru_reg_rec TCAM_PROFILES_INNER_SHIFT_5_REG =
{
    "PROFILES_INNER_SHIFT_5",
#if RU_INCLUDE_DESC
    "INNER_SHIFT 0..31 Register",
    "The context output can be shifted right based on the TCAM match vector and amount of nibbles to be shifted per match vector bit.\nEach configured shift is 4 bits.\nMax value for byte offset is 15.\nTotal bits in this RAM is:NUM_OF_CNTX*NUM_OF_TCAMS*4.\nLSB bits on lowest address corresponds to CNTX0,TCAM0.\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_5_REG_OFFSET },
    TCAM_PROFILES_INNER_SHIFT_5_REG_RAM_CNT,
    580,
    1930,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_INNER_SHIFT_5_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_INNER_SHIFT_6, TYPE: Type_TCAM_TCAM_PROFILES_INNER_SHIFT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_INNER_SHIFT_6_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_6_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_INNER_SHIFT_6_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_INNER_SHIFT_6_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_INNER_SHIFT_6_FIELDS[] =
{
    &TCAM_PROFILES_INNER_SHIFT_6_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_INNER_SHIFT_6 *****/
const ru_reg_rec TCAM_PROFILES_INNER_SHIFT_6_REG =
{
    "PROFILES_INNER_SHIFT_6",
#if RU_INCLUDE_DESC
    "INNER_SHIFT 0..31 Register",
    "The context output can be shifted right based on the TCAM match vector and amount of nibbles to be shifted per match vector bit.\nEach configured shift is 4 bits.\nMax value for byte offset is 15.\nTotal bits in this RAM is:NUM_OF_CNTX*NUM_OF_TCAMS*4.\nLSB bits on lowest address corresponds to CNTX0,TCAM0.\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_6_REG_OFFSET },
    TCAM_PROFILES_INNER_SHIFT_6_REG_RAM_CNT,
    580,
    1931,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_INNER_SHIFT_6_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_INNER_SHIFT_7, TYPE: Type_TCAM_TCAM_PROFILES_INNER_SHIFT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_INNER_SHIFT_7_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_7_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_INNER_SHIFT_7_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_INNER_SHIFT_7_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_INNER_SHIFT_7_FIELDS[] =
{
    &TCAM_PROFILES_INNER_SHIFT_7_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_INNER_SHIFT_7 *****/
const ru_reg_rec TCAM_PROFILES_INNER_SHIFT_7_REG =
{
    "PROFILES_INNER_SHIFT_7",
#if RU_INCLUDE_DESC
    "INNER_SHIFT 0..31 Register",
    "The context output can be shifted right based on the TCAM match vector and amount of nibbles to be shifted per match vector bit.\nEach configured shift is 4 bits.\nMax value for byte offset is 15.\nTotal bits in this RAM is:NUM_OF_CNTX*NUM_OF_TCAMS*4.\nLSB bits on lowest address corresponds to CNTX0,TCAM0.\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_7_REG_OFFSET },
    TCAM_PROFILES_INNER_SHIFT_7_REG_RAM_CNT,
    580,
    1932,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_INNER_SHIFT_7_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_INNER_SHIFT_8, TYPE: Type_TCAM_TCAM_PROFILES_INNER_SHIFT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_INNER_SHIFT_8_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_8_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_INNER_SHIFT_8_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_INNER_SHIFT_8_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_INNER_SHIFT_8_FIELDS[] =
{
    &TCAM_PROFILES_INNER_SHIFT_8_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_INNER_SHIFT_8 *****/
const ru_reg_rec TCAM_PROFILES_INNER_SHIFT_8_REG =
{
    "PROFILES_INNER_SHIFT_8",
#if RU_INCLUDE_DESC
    "INNER_SHIFT 0..31 Register",
    "The context output can be shifted right based on the TCAM match vector and amount of nibbles to be shifted per match vector bit.\nEach configured shift is 4 bits.\nMax value for byte offset is 15.\nTotal bits in this RAM is:NUM_OF_CNTX*NUM_OF_TCAMS*4.\nLSB bits on lowest address corresponds to CNTX0,TCAM0.\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_8_REG_OFFSET },
    TCAM_PROFILES_INNER_SHIFT_8_REG_RAM_CNT,
    580,
    1933,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_INNER_SHIFT_8_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_INNER_SHIFT_9, TYPE: Type_TCAM_TCAM_PROFILES_INNER_SHIFT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_INNER_SHIFT_9_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_9_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_INNER_SHIFT_9_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_INNER_SHIFT_9_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_INNER_SHIFT_9_FIELDS[] =
{
    &TCAM_PROFILES_INNER_SHIFT_9_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_INNER_SHIFT_9 *****/
const ru_reg_rec TCAM_PROFILES_INNER_SHIFT_9_REG =
{
    "PROFILES_INNER_SHIFT_9",
#if RU_INCLUDE_DESC
    "INNER_SHIFT 0..31 Register",
    "The context output can be shifted right based on the TCAM match vector and amount of nibbles to be shifted per match vector bit.\nEach configured shift is 4 bits.\nMax value for byte offset is 15.\nTotal bits in this RAM is:NUM_OF_CNTX*NUM_OF_TCAMS*4.\nLSB bits on lowest address corresponds to CNTX0,TCAM0.\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_9_REG_OFFSET },
    TCAM_PROFILES_INNER_SHIFT_9_REG_RAM_CNT,
    580,
    1934,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_INNER_SHIFT_9_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_INNER_SHIFT_10, TYPE: Type_TCAM_TCAM_PROFILES_INNER_SHIFT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_INNER_SHIFT_10_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_10_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_INNER_SHIFT_10_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_INNER_SHIFT_10_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_INNER_SHIFT_10_FIELDS[] =
{
    &TCAM_PROFILES_INNER_SHIFT_10_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_INNER_SHIFT_10 *****/
const ru_reg_rec TCAM_PROFILES_INNER_SHIFT_10_REG =
{
    "PROFILES_INNER_SHIFT_10",
#if RU_INCLUDE_DESC
    "INNER_SHIFT 0..31 Register",
    "The context output can be shifted right based on the TCAM match vector and amount of nibbles to be shifted per match vector bit.\nEach configured shift is 4 bits.\nMax value for byte offset is 15.\nTotal bits in this RAM is:NUM_OF_CNTX*NUM_OF_TCAMS*4.\nLSB bits on lowest address corresponds to CNTX0,TCAM0.\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_10_REG_OFFSET },
    TCAM_PROFILES_INNER_SHIFT_10_REG_RAM_CNT,
    580,
    1935,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_INNER_SHIFT_10_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_INNER_SHIFT_11, TYPE: Type_TCAM_TCAM_PROFILES_INNER_SHIFT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_INNER_SHIFT_11_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_11_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_INNER_SHIFT_11_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_INNER_SHIFT_11_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_INNER_SHIFT_11_FIELDS[] =
{
    &TCAM_PROFILES_INNER_SHIFT_11_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_INNER_SHIFT_11 *****/
const ru_reg_rec TCAM_PROFILES_INNER_SHIFT_11_REG =
{
    "PROFILES_INNER_SHIFT_11",
#if RU_INCLUDE_DESC
    "INNER_SHIFT 0..31 Register",
    "The context output can be shifted right based on the TCAM match vector and amount of nibbles to be shifted per match vector bit.\nEach configured shift is 4 bits.\nMax value for byte offset is 15.\nTotal bits in this RAM is:NUM_OF_CNTX*NUM_OF_TCAMS*4.\nLSB bits on lowest address corresponds to CNTX0,TCAM0.\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_11_REG_OFFSET },
    TCAM_PROFILES_INNER_SHIFT_11_REG_RAM_CNT,
    580,
    1936,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_INNER_SHIFT_11_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_INNER_SHIFT_12, TYPE: Type_TCAM_TCAM_PROFILES_INNER_SHIFT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_INNER_SHIFT_12_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_12_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_INNER_SHIFT_12_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_INNER_SHIFT_12_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_INNER_SHIFT_12_FIELDS[] =
{
    &TCAM_PROFILES_INNER_SHIFT_12_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_INNER_SHIFT_12 *****/
const ru_reg_rec TCAM_PROFILES_INNER_SHIFT_12_REG =
{
    "PROFILES_INNER_SHIFT_12",
#if RU_INCLUDE_DESC
    "INNER_SHIFT 0..31 Register",
    "The context output can be shifted right based on the TCAM match vector and amount of nibbles to be shifted per match vector bit.\nEach configured shift is 4 bits.\nMax value for byte offset is 15.\nTotal bits in this RAM is:NUM_OF_CNTX*NUM_OF_TCAMS*4.\nLSB bits on lowest address corresponds to CNTX0,TCAM0.\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_12_REG_OFFSET },
    TCAM_PROFILES_INNER_SHIFT_12_REG_RAM_CNT,
    580,
    1937,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_INNER_SHIFT_12_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_INNER_SHIFT_13, TYPE: Type_TCAM_TCAM_PROFILES_INNER_SHIFT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_INNER_SHIFT_13_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_13_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_INNER_SHIFT_13_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_INNER_SHIFT_13_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_INNER_SHIFT_13_FIELDS[] =
{
    &TCAM_PROFILES_INNER_SHIFT_13_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_INNER_SHIFT_13 *****/
const ru_reg_rec TCAM_PROFILES_INNER_SHIFT_13_REG =
{
    "PROFILES_INNER_SHIFT_13",
#if RU_INCLUDE_DESC
    "INNER_SHIFT 0..31 Register",
    "The context output can be shifted right based on the TCAM match vector and amount of nibbles to be shifted per match vector bit.\nEach configured shift is 4 bits.\nMax value for byte offset is 15.\nTotal bits in this RAM is:NUM_OF_CNTX*NUM_OF_TCAMS*4.\nLSB bits on lowest address corresponds to CNTX0,TCAM0.\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_13_REG_OFFSET },
    TCAM_PROFILES_INNER_SHIFT_13_REG_RAM_CNT,
    580,
    1938,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_INNER_SHIFT_13_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_INNER_SHIFT_14, TYPE: Type_TCAM_TCAM_PROFILES_INNER_SHIFT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_INNER_SHIFT_14_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_14_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_INNER_SHIFT_14_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_INNER_SHIFT_14_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_INNER_SHIFT_14_FIELDS[] =
{
    &TCAM_PROFILES_INNER_SHIFT_14_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_INNER_SHIFT_14 *****/
const ru_reg_rec TCAM_PROFILES_INNER_SHIFT_14_REG =
{
    "PROFILES_INNER_SHIFT_14",
#if RU_INCLUDE_DESC
    "INNER_SHIFT 0..31 Register",
    "The context output can be shifted right based on the TCAM match vector and amount of nibbles to be shifted per match vector bit.\nEach configured shift is 4 bits.\nMax value for byte offset is 15.\nTotal bits in this RAM is:NUM_OF_CNTX*NUM_OF_TCAMS*4.\nLSB bits on lowest address corresponds to CNTX0,TCAM0.\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_14_REG_OFFSET },
    TCAM_PROFILES_INNER_SHIFT_14_REG_RAM_CNT,
    580,
    1939,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_INNER_SHIFT_14_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_INNER_SHIFT_15, TYPE: Type_TCAM_TCAM_PROFILES_INNER_SHIFT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_INNER_SHIFT_15_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_15_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_INNER_SHIFT_15_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_INNER_SHIFT_15_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_INNER_SHIFT_15_FIELDS[] =
{
    &TCAM_PROFILES_INNER_SHIFT_15_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_INNER_SHIFT_15 *****/
const ru_reg_rec TCAM_PROFILES_INNER_SHIFT_15_REG =
{
    "PROFILES_INNER_SHIFT_15",
#if RU_INCLUDE_DESC
    "INNER_SHIFT 0..31 Register",
    "The context output can be shifted right based on the TCAM match vector and amount of nibbles to be shifted per match vector bit.\nEach configured shift is 4 bits.\nMax value for byte offset is 15.\nTotal bits in this RAM is:NUM_OF_CNTX*NUM_OF_TCAMS*4.\nLSB bits on lowest address corresponds to CNTX0,TCAM0.\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_15_REG_OFFSET },
    TCAM_PROFILES_INNER_SHIFT_15_REG_RAM_CNT,
    580,
    1940,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_INNER_SHIFT_15_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_INNER_SHIFT_16, TYPE: Type_TCAM_TCAM_PROFILES_INNER_SHIFT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_INNER_SHIFT_16_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_16_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_INNER_SHIFT_16_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_INNER_SHIFT_16_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_INNER_SHIFT_16_FIELDS[] =
{
    &TCAM_PROFILES_INNER_SHIFT_16_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_INNER_SHIFT_16 *****/
const ru_reg_rec TCAM_PROFILES_INNER_SHIFT_16_REG =
{
    "PROFILES_INNER_SHIFT_16",
#if RU_INCLUDE_DESC
    "INNER_SHIFT 0..31 Register",
    "The context output can be shifted right based on the TCAM match vector and amount of nibbles to be shifted per match vector bit.\nEach configured shift is 4 bits.\nMax value for byte offset is 15.\nTotal bits in this RAM is:NUM_OF_CNTX*NUM_OF_TCAMS*4.\nLSB bits on lowest address corresponds to CNTX0,TCAM0.\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_16_REG_OFFSET },
    TCAM_PROFILES_INNER_SHIFT_16_REG_RAM_CNT,
    580,
    1941,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_INNER_SHIFT_16_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_INNER_SHIFT_17, TYPE: Type_TCAM_TCAM_PROFILES_INNER_SHIFT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_INNER_SHIFT_17_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_17_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_INNER_SHIFT_17_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_INNER_SHIFT_17_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_INNER_SHIFT_17_FIELDS[] =
{
    &TCAM_PROFILES_INNER_SHIFT_17_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_INNER_SHIFT_17 *****/
const ru_reg_rec TCAM_PROFILES_INNER_SHIFT_17_REG =
{
    "PROFILES_INNER_SHIFT_17",
#if RU_INCLUDE_DESC
    "INNER_SHIFT 0..31 Register",
    "The context output can be shifted right based on the TCAM match vector and amount of nibbles to be shifted per match vector bit.\nEach configured shift is 4 bits.\nMax value for byte offset is 15.\nTotal bits in this RAM is:NUM_OF_CNTX*NUM_OF_TCAMS*4.\nLSB bits on lowest address corresponds to CNTX0,TCAM0.\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_17_REG_OFFSET },
    TCAM_PROFILES_INNER_SHIFT_17_REG_RAM_CNT,
    580,
    1942,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_INNER_SHIFT_17_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_INNER_SHIFT_18, TYPE: Type_TCAM_TCAM_PROFILES_INNER_SHIFT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_INNER_SHIFT_18_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_18_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_INNER_SHIFT_18_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_INNER_SHIFT_18_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_INNER_SHIFT_18_FIELDS[] =
{
    &TCAM_PROFILES_INNER_SHIFT_18_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_INNER_SHIFT_18 *****/
const ru_reg_rec TCAM_PROFILES_INNER_SHIFT_18_REG =
{
    "PROFILES_INNER_SHIFT_18",
#if RU_INCLUDE_DESC
    "INNER_SHIFT 0..31 Register",
    "The context output can be shifted right based on the TCAM match vector and amount of nibbles to be shifted per match vector bit.\nEach configured shift is 4 bits.\nMax value for byte offset is 15.\nTotal bits in this RAM is:NUM_OF_CNTX*NUM_OF_TCAMS*4.\nLSB bits on lowest address corresponds to CNTX0,TCAM0.\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_18_REG_OFFSET },
    TCAM_PROFILES_INNER_SHIFT_18_REG_RAM_CNT,
    580,
    1943,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_INNER_SHIFT_18_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_INNER_SHIFT_19, TYPE: Type_TCAM_TCAM_PROFILES_INNER_SHIFT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_INNER_SHIFT_19_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_19_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_INNER_SHIFT_19_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_INNER_SHIFT_19_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_INNER_SHIFT_19_FIELDS[] =
{
    &TCAM_PROFILES_INNER_SHIFT_19_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_INNER_SHIFT_19 *****/
const ru_reg_rec TCAM_PROFILES_INNER_SHIFT_19_REG =
{
    "PROFILES_INNER_SHIFT_19",
#if RU_INCLUDE_DESC
    "INNER_SHIFT 0..31 Register",
    "The context output can be shifted right based on the TCAM match vector and amount of nibbles to be shifted per match vector bit.\nEach configured shift is 4 bits.\nMax value for byte offset is 15.\nTotal bits in this RAM is:NUM_OF_CNTX*NUM_OF_TCAMS*4.\nLSB bits on lowest address corresponds to CNTX0,TCAM0.\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_19_REG_OFFSET },
    TCAM_PROFILES_INNER_SHIFT_19_REG_RAM_CNT,
    580,
    1944,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_INNER_SHIFT_19_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_INNER_SHIFT_20, TYPE: Type_TCAM_TCAM_PROFILES_INNER_SHIFT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_INNER_SHIFT_20_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_20_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_INNER_SHIFT_20_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_INNER_SHIFT_20_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_INNER_SHIFT_20_FIELDS[] =
{
    &TCAM_PROFILES_INNER_SHIFT_20_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_INNER_SHIFT_20 *****/
const ru_reg_rec TCAM_PROFILES_INNER_SHIFT_20_REG =
{
    "PROFILES_INNER_SHIFT_20",
#if RU_INCLUDE_DESC
    "INNER_SHIFT 0..31 Register",
    "The context output can be shifted right based on the TCAM match vector and amount of nibbles to be shifted per match vector bit.\nEach configured shift is 4 bits.\nMax value for byte offset is 15.\nTotal bits in this RAM is:NUM_OF_CNTX*NUM_OF_TCAMS*4.\nLSB bits on lowest address corresponds to CNTX0,TCAM0.\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_20_REG_OFFSET },
    TCAM_PROFILES_INNER_SHIFT_20_REG_RAM_CNT,
    580,
    1945,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_INNER_SHIFT_20_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_INNER_SHIFT_21, TYPE: Type_TCAM_TCAM_PROFILES_INNER_SHIFT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_INNER_SHIFT_21_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_21_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_INNER_SHIFT_21_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_INNER_SHIFT_21_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_INNER_SHIFT_21_FIELDS[] =
{
    &TCAM_PROFILES_INNER_SHIFT_21_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_INNER_SHIFT_21 *****/
const ru_reg_rec TCAM_PROFILES_INNER_SHIFT_21_REG =
{
    "PROFILES_INNER_SHIFT_21",
#if RU_INCLUDE_DESC
    "INNER_SHIFT 0..31 Register",
    "The context output can be shifted right based on the TCAM match vector and amount of nibbles to be shifted per match vector bit.\nEach configured shift is 4 bits.\nMax value for byte offset is 15.\nTotal bits in this RAM is:NUM_OF_CNTX*NUM_OF_TCAMS*4.\nLSB bits on lowest address corresponds to CNTX0,TCAM0.\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_21_REG_OFFSET },
    TCAM_PROFILES_INNER_SHIFT_21_REG_RAM_CNT,
    580,
    1946,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_INNER_SHIFT_21_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_INNER_SHIFT_22, TYPE: Type_TCAM_TCAM_PROFILES_INNER_SHIFT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_INNER_SHIFT_22_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_22_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_INNER_SHIFT_22_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_INNER_SHIFT_22_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_INNER_SHIFT_22_FIELDS[] =
{
    &TCAM_PROFILES_INNER_SHIFT_22_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_INNER_SHIFT_22 *****/
const ru_reg_rec TCAM_PROFILES_INNER_SHIFT_22_REG =
{
    "PROFILES_INNER_SHIFT_22",
#if RU_INCLUDE_DESC
    "INNER_SHIFT 0..31 Register",
    "The context output can be shifted right based on the TCAM match vector and amount of nibbles to be shifted per match vector bit.\nEach configured shift is 4 bits.\nMax value for byte offset is 15.\nTotal bits in this RAM is:NUM_OF_CNTX*NUM_OF_TCAMS*4.\nLSB bits on lowest address corresponds to CNTX0,TCAM0.\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_22_REG_OFFSET },
    TCAM_PROFILES_INNER_SHIFT_22_REG_RAM_CNT,
    580,
    1947,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_INNER_SHIFT_22_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_INNER_SHIFT_23, TYPE: Type_TCAM_TCAM_PROFILES_INNER_SHIFT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_INNER_SHIFT_23_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_23_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_INNER_SHIFT_23_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_INNER_SHIFT_23_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_INNER_SHIFT_23_FIELDS[] =
{
    &TCAM_PROFILES_INNER_SHIFT_23_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_INNER_SHIFT_23 *****/
const ru_reg_rec TCAM_PROFILES_INNER_SHIFT_23_REG =
{
    "PROFILES_INNER_SHIFT_23",
#if RU_INCLUDE_DESC
    "INNER_SHIFT 0..31 Register",
    "The context output can be shifted right based on the TCAM match vector and amount of nibbles to be shifted per match vector bit.\nEach configured shift is 4 bits.\nMax value for byte offset is 15.\nTotal bits in this RAM is:NUM_OF_CNTX*NUM_OF_TCAMS*4.\nLSB bits on lowest address corresponds to CNTX0,TCAM0.\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_23_REG_OFFSET },
    TCAM_PROFILES_INNER_SHIFT_23_REG_RAM_CNT,
    580,
    1948,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_INNER_SHIFT_23_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_INNER_SHIFT_24, TYPE: Type_TCAM_TCAM_PROFILES_INNER_SHIFT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_INNER_SHIFT_24_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_24_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_INNER_SHIFT_24_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_INNER_SHIFT_24_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_INNER_SHIFT_24_FIELDS[] =
{
    &TCAM_PROFILES_INNER_SHIFT_24_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_INNER_SHIFT_24 *****/
const ru_reg_rec TCAM_PROFILES_INNER_SHIFT_24_REG =
{
    "PROFILES_INNER_SHIFT_24",
#if RU_INCLUDE_DESC
    "INNER_SHIFT 0..31 Register",
    "The context output can be shifted right based on the TCAM match vector and amount of nibbles to be shifted per match vector bit.\nEach configured shift is 4 bits.\nMax value for byte offset is 15.\nTotal bits in this RAM is:NUM_OF_CNTX*NUM_OF_TCAMS*4.\nLSB bits on lowest address corresponds to CNTX0,TCAM0.\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_24_REG_OFFSET },
    TCAM_PROFILES_INNER_SHIFT_24_REG_RAM_CNT,
    580,
    1949,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_INNER_SHIFT_24_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_INNER_SHIFT_25, TYPE: Type_TCAM_TCAM_PROFILES_INNER_SHIFT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_INNER_SHIFT_25_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_25_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_INNER_SHIFT_25_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_INNER_SHIFT_25_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_INNER_SHIFT_25_FIELDS[] =
{
    &TCAM_PROFILES_INNER_SHIFT_25_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_INNER_SHIFT_25 *****/
const ru_reg_rec TCAM_PROFILES_INNER_SHIFT_25_REG =
{
    "PROFILES_INNER_SHIFT_25",
#if RU_INCLUDE_DESC
    "INNER_SHIFT 0..31 Register",
    "The context output can be shifted right based on the TCAM match vector and amount of nibbles to be shifted per match vector bit.\nEach configured shift is 4 bits.\nMax value for byte offset is 15.\nTotal bits in this RAM is:NUM_OF_CNTX*NUM_OF_TCAMS*4.\nLSB bits on lowest address corresponds to CNTX0,TCAM0.\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_25_REG_OFFSET },
    TCAM_PROFILES_INNER_SHIFT_25_REG_RAM_CNT,
    580,
    1950,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_INNER_SHIFT_25_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_INNER_SHIFT_26, TYPE: Type_TCAM_TCAM_PROFILES_INNER_SHIFT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_INNER_SHIFT_26_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_26_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_INNER_SHIFT_26_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_INNER_SHIFT_26_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_INNER_SHIFT_26_FIELDS[] =
{
    &TCAM_PROFILES_INNER_SHIFT_26_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_INNER_SHIFT_26 *****/
const ru_reg_rec TCAM_PROFILES_INNER_SHIFT_26_REG =
{
    "PROFILES_INNER_SHIFT_26",
#if RU_INCLUDE_DESC
    "INNER_SHIFT 0..31 Register",
    "The context output can be shifted right based on the TCAM match vector and amount of nibbles to be shifted per match vector bit.\nEach configured shift is 4 bits.\nMax value for byte offset is 15.\nTotal bits in this RAM is:NUM_OF_CNTX*NUM_OF_TCAMS*4.\nLSB bits on lowest address corresponds to CNTX0,TCAM0.\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_26_REG_OFFSET },
    TCAM_PROFILES_INNER_SHIFT_26_REG_RAM_CNT,
    580,
    1951,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_INNER_SHIFT_26_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_INNER_SHIFT_27, TYPE: Type_TCAM_TCAM_PROFILES_INNER_SHIFT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_INNER_SHIFT_27_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_27_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_INNER_SHIFT_27_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_INNER_SHIFT_27_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_INNER_SHIFT_27_FIELDS[] =
{
    &TCAM_PROFILES_INNER_SHIFT_27_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_INNER_SHIFT_27 *****/
const ru_reg_rec TCAM_PROFILES_INNER_SHIFT_27_REG =
{
    "PROFILES_INNER_SHIFT_27",
#if RU_INCLUDE_DESC
    "INNER_SHIFT 0..31 Register",
    "The context output can be shifted right based on the TCAM match vector and amount of nibbles to be shifted per match vector bit.\nEach configured shift is 4 bits.\nMax value for byte offset is 15.\nTotal bits in this RAM is:NUM_OF_CNTX*NUM_OF_TCAMS*4.\nLSB bits on lowest address corresponds to CNTX0,TCAM0.\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_27_REG_OFFSET },
    TCAM_PROFILES_INNER_SHIFT_27_REG_RAM_CNT,
    580,
    1952,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_INNER_SHIFT_27_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_INNER_SHIFT_28, TYPE: Type_TCAM_TCAM_PROFILES_INNER_SHIFT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_INNER_SHIFT_28_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_28_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_INNER_SHIFT_28_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_INNER_SHIFT_28_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_INNER_SHIFT_28_FIELDS[] =
{
    &TCAM_PROFILES_INNER_SHIFT_28_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_INNER_SHIFT_28 *****/
const ru_reg_rec TCAM_PROFILES_INNER_SHIFT_28_REG =
{
    "PROFILES_INNER_SHIFT_28",
#if RU_INCLUDE_DESC
    "INNER_SHIFT 0..31 Register",
    "The context output can be shifted right based on the TCAM match vector and amount of nibbles to be shifted per match vector bit.\nEach configured shift is 4 bits.\nMax value for byte offset is 15.\nTotal bits in this RAM is:NUM_OF_CNTX*NUM_OF_TCAMS*4.\nLSB bits on lowest address corresponds to CNTX0,TCAM0.\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_28_REG_OFFSET },
    TCAM_PROFILES_INNER_SHIFT_28_REG_RAM_CNT,
    580,
    1953,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_INNER_SHIFT_28_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_INNER_SHIFT_29, TYPE: Type_TCAM_TCAM_PROFILES_INNER_SHIFT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_INNER_SHIFT_29_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_29_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_INNER_SHIFT_29_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_INNER_SHIFT_29_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_INNER_SHIFT_29_FIELDS[] =
{
    &TCAM_PROFILES_INNER_SHIFT_29_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_INNER_SHIFT_29 *****/
const ru_reg_rec TCAM_PROFILES_INNER_SHIFT_29_REG =
{
    "PROFILES_INNER_SHIFT_29",
#if RU_INCLUDE_DESC
    "INNER_SHIFT 0..31 Register",
    "The context output can be shifted right based on the TCAM match vector and amount of nibbles to be shifted per match vector bit.\nEach configured shift is 4 bits.\nMax value for byte offset is 15.\nTotal bits in this RAM is:NUM_OF_CNTX*NUM_OF_TCAMS*4.\nLSB bits on lowest address corresponds to CNTX0,TCAM0.\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_29_REG_OFFSET },
    TCAM_PROFILES_INNER_SHIFT_29_REG_RAM_CNT,
    580,
    1954,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_INNER_SHIFT_29_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_INNER_SHIFT_30, TYPE: Type_TCAM_TCAM_PROFILES_INNER_SHIFT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_INNER_SHIFT_30_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_30_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_INNER_SHIFT_30_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_INNER_SHIFT_30_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_INNER_SHIFT_30_FIELDS[] =
{
    &TCAM_PROFILES_INNER_SHIFT_30_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_INNER_SHIFT_30 *****/
const ru_reg_rec TCAM_PROFILES_INNER_SHIFT_30_REG =
{
    "PROFILES_INNER_SHIFT_30",
#if RU_INCLUDE_DESC
    "INNER_SHIFT 0..31 Register",
    "The context output can be shifted right based on the TCAM match vector and amount of nibbles to be shifted per match vector bit.\nEach configured shift is 4 bits.\nMax value for byte offset is 15.\nTotal bits in this RAM is:NUM_OF_CNTX*NUM_OF_TCAMS*4.\nLSB bits on lowest address corresponds to CNTX0,TCAM0.\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_30_REG_OFFSET },
    TCAM_PROFILES_INNER_SHIFT_30_REG_RAM_CNT,
    580,
    1955,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_INNER_SHIFT_30_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_INNER_SHIFT_31, TYPE: Type_TCAM_TCAM_PROFILES_INNER_SHIFT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_INNER_SHIFT_31_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_31_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_INNER_SHIFT_31_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_INNER_SHIFT_31_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_INNER_SHIFT_31_FIELDS[] =
{
    &TCAM_PROFILES_INNER_SHIFT_31_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_INNER_SHIFT_31 *****/
const ru_reg_rec TCAM_PROFILES_INNER_SHIFT_31_REG =
{
    "PROFILES_INNER_SHIFT_31",
#if RU_INCLUDE_DESC
    "INNER_SHIFT 0..31 Register",
    "The context output can be shifted right based on the TCAM match vector and amount of nibbles to be shifted per match vector bit.\nEach configured shift is 4 bits.\nMax value for byte offset is 15.\nTotal bits in this RAM is:NUM_OF_CNTX*NUM_OF_TCAMS*4.\nLSB bits on lowest address corresponds to CNTX0,TCAM0.\n",
#endif
    { TCAM_PROFILES_INNER_SHIFT_31_REG_OFFSET },
    TCAM_PROFILES_INNER_SHIFT_31_REG_RAM_CNT,
    580,
    1956,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_INNER_SHIFT_31_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_CNTXIN_SP_ID_0, TYPE: Type_TCAM_TCAM_PROFILES_CNTXIN_SP_ID
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_CNTXIN_SP_ID_0_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_CNTXIN_SP_ID_0_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_CNTXIN_SP_ID_0_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_CNTXIN_SP_ID_0_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_CNTXIN_SP_ID_0_FIELDS[] =
{
    &TCAM_PROFILES_CNTXIN_SP_ID_0_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_CNTXIN_SP_ID_0 *****/
const ru_reg_rec TCAM_PROFILES_CNTXIN_SP_ID_0_REG =
{
    "PROFILES_CNTXIN_SP_ID_0",
#if RU_INCLUDE_DESC
    "CNTXIN_SP_ID 0..1 Register",
    "maps between SP and context RAMs.\n4 bits per Context RAM\n\nEach configured ID is 4 bits.\nMax value for ID is 7.\nTotal bits in this RAM is:NUM_OF_CNTX*4.\nLSB bits on lowest address corresponds to CNTX0\n",
#endif
    { TCAM_PROFILES_CNTXIN_SP_ID_0_REG_OFFSET },
    TCAM_PROFILES_CNTXIN_SP_ID_0_REG_RAM_CNT,
    580,
    1957,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_CNTXIN_SP_ID_0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_CNTXIN_SP_ID_1, TYPE: Type_TCAM_TCAM_PROFILES_CNTXIN_SP_ID
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_CNTXIN_SP_ID_1_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_CNTXIN_SP_ID_1_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_CNTXIN_SP_ID_1_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_CNTXIN_SP_ID_1_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_CNTXIN_SP_ID_1_FIELDS[] =
{
    &TCAM_PROFILES_CNTXIN_SP_ID_1_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_CNTXIN_SP_ID_1 *****/
const ru_reg_rec TCAM_PROFILES_CNTXIN_SP_ID_1_REG =
{
    "PROFILES_CNTXIN_SP_ID_1",
#if RU_INCLUDE_DESC
    "CNTXIN_SP_ID 0..1 Register",
    "maps between SP and context RAMs.\n4 bits per Context RAM\n\nEach configured ID is 4 bits.\nMax value for ID is 7.\nTotal bits in this RAM is:NUM_OF_CNTX*4.\nLSB bits on lowest address corresponds to CNTX0\n",
#endif
    { TCAM_PROFILES_CNTXIN_SP_ID_1_REG_OFFSET },
    TCAM_PROFILES_CNTXIN_SP_ID_1_REG_RAM_CNT,
    580,
    1958,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_CNTXIN_SP_ID_1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_CNTXIN_MASK_0, TYPE: Type_TCAM_TCAM_PROFILES_CNTXIN_MASK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_CNTXIN_MASK_0_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_CNTXIN_MASK_0_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_CNTXIN_MASK_0_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_CNTXIN_MASK_0_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_CNTXIN_MASK_0_FIELDS[] =
{
    &TCAM_PROFILES_CNTXIN_MASK_0_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_CNTXIN_MASK_0 *****/
const ru_reg_rec TCAM_PROFILES_CNTXIN_MASK_0_REG =
{
    "PROFILES_CNTXIN_MASK_0",
#if RU_INCLUDE_DESC
    "CNTXIN_MASK 0..7 Register",
    "Activating mask per Context\n16bits per context.\nTotal bits in this RAM is:NUM_OF_CNTX*NUM_OF_TCAM.\nLSB bits on lowest address corresponds to CNTX0.\n",
#endif
    { TCAM_PROFILES_CNTXIN_MASK_0_REG_OFFSET },
    TCAM_PROFILES_CNTXIN_MASK_0_REG_RAM_CNT,
    580,
    1959,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_CNTXIN_MASK_0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_CNTXIN_MASK_1, TYPE: Type_TCAM_TCAM_PROFILES_CNTXIN_MASK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_CNTXIN_MASK_1_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_CNTXIN_MASK_1_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_CNTXIN_MASK_1_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_CNTXIN_MASK_1_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_CNTXIN_MASK_1_FIELDS[] =
{
    &TCAM_PROFILES_CNTXIN_MASK_1_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_CNTXIN_MASK_1 *****/
const ru_reg_rec TCAM_PROFILES_CNTXIN_MASK_1_REG =
{
    "PROFILES_CNTXIN_MASK_1",
#if RU_INCLUDE_DESC
    "CNTXIN_MASK 0..7 Register",
    "Activating mask per Context\n16bits per context.\nTotal bits in this RAM is:NUM_OF_CNTX*NUM_OF_TCAM.\nLSB bits on lowest address corresponds to CNTX0.\n",
#endif
    { TCAM_PROFILES_CNTXIN_MASK_1_REG_OFFSET },
    TCAM_PROFILES_CNTXIN_MASK_1_REG_RAM_CNT,
    580,
    1960,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_CNTXIN_MASK_1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_CNTXIN_MASK_2, TYPE: Type_TCAM_TCAM_PROFILES_CNTXIN_MASK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_CNTXIN_MASK_2_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_CNTXIN_MASK_2_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_CNTXIN_MASK_2_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_CNTXIN_MASK_2_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_CNTXIN_MASK_2_FIELDS[] =
{
    &TCAM_PROFILES_CNTXIN_MASK_2_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_CNTXIN_MASK_2 *****/
const ru_reg_rec TCAM_PROFILES_CNTXIN_MASK_2_REG =
{
    "PROFILES_CNTXIN_MASK_2",
#if RU_INCLUDE_DESC
    "CNTXIN_MASK 0..7 Register",
    "Activating mask per Context\n16bits per context.\nTotal bits in this RAM is:NUM_OF_CNTX*NUM_OF_TCAM.\nLSB bits on lowest address corresponds to CNTX0.\n",
#endif
    { TCAM_PROFILES_CNTXIN_MASK_2_REG_OFFSET },
    TCAM_PROFILES_CNTXIN_MASK_2_REG_RAM_CNT,
    580,
    1961,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_CNTXIN_MASK_2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_CNTXIN_MASK_3, TYPE: Type_TCAM_TCAM_PROFILES_CNTXIN_MASK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_CNTXIN_MASK_3_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_CNTXIN_MASK_3_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_CNTXIN_MASK_3_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_CNTXIN_MASK_3_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_CNTXIN_MASK_3_FIELDS[] =
{
    &TCAM_PROFILES_CNTXIN_MASK_3_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_CNTXIN_MASK_3 *****/
const ru_reg_rec TCAM_PROFILES_CNTXIN_MASK_3_REG =
{
    "PROFILES_CNTXIN_MASK_3",
#if RU_INCLUDE_DESC
    "CNTXIN_MASK 0..7 Register",
    "Activating mask per Context\n16bits per context.\nTotal bits in this RAM is:NUM_OF_CNTX*NUM_OF_TCAM.\nLSB bits on lowest address corresponds to CNTX0.\n",
#endif
    { TCAM_PROFILES_CNTXIN_MASK_3_REG_OFFSET },
    TCAM_PROFILES_CNTXIN_MASK_3_REG_RAM_CNT,
    580,
    1962,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_CNTXIN_MASK_3_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_CNTXIN_MASK_4, TYPE: Type_TCAM_TCAM_PROFILES_CNTXIN_MASK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_CNTXIN_MASK_4_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_CNTXIN_MASK_4_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_CNTXIN_MASK_4_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_CNTXIN_MASK_4_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_CNTXIN_MASK_4_FIELDS[] =
{
    &TCAM_PROFILES_CNTXIN_MASK_4_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_CNTXIN_MASK_4 *****/
const ru_reg_rec TCAM_PROFILES_CNTXIN_MASK_4_REG =
{
    "PROFILES_CNTXIN_MASK_4",
#if RU_INCLUDE_DESC
    "CNTXIN_MASK 0..7 Register",
    "Activating mask per Context\n16bits per context.\nTotal bits in this RAM is:NUM_OF_CNTX*NUM_OF_TCAM.\nLSB bits on lowest address corresponds to CNTX0.\n",
#endif
    { TCAM_PROFILES_CNTXIN_MASK_4_REG_OFFSET },
    TCAM_PROFILES_CNTXIN_MASK_4_REG_RAM_CNT,
    580,
    1963,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_CNTXIN_MASK_4_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_CNTXIN_MASK_5, TYPE: Type_TCAM_TCAM_PROFILES_CNTXIN_MASK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_CNTXIN_MASK_5_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_CNTXIN_MASK_5_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_CNTXIN_MASK_5_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_CNTXIN_MASK_5_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_CNTXIN_MASK_5_FIELDS[] =
{
    &TCAM_PROFILES_CNTXIN_MASK_5_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_CNTXIN_MASK_5 *****/
const ru_reg_rec TCAM_PROFILES_CNTXIN_MASK_5_REG =
{
    "PROFILES_CNTXIN_MASK_5",
#if RU_INCLUDE_DESC
    "CNTXIN_MASK 0..7 Register",
    "Activating mask per Context\n16bits per context.\nTotal bits in this RAM is:NUM_OF_CNTX*NUM_OF_TCAM.\nLSB bits on lowest address corresponds to CNTX0.\n",
#endif
    { TCAM_PROFILES_CNTXIN_MASK_5_REG_OFFSET },
    TCAM_PROFILES_CNTXIN_MASK_5_REG_RAM_CNT,
    580,
    1964,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_CNTXIN_MASK_5_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_CNTXIN_MASK_6, TYPE: Type_TCAM_TCAM_PROFILES_CNTXIN_MASK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_CNTXIN_MASK_6_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_CNTXIN_MASK_6_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_CNTXIN_MASK_6_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_CNTXIN_MASK_6_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_CNTXIN_MASK_6_FIELDS[] =
{
    &TCAM_PROFILES_CNTXIN_MASK_6_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_CNTXIN_MASK_6 *****/
const ru_reg_rec TCAM_PROFILES_CNTXIN_MASK_6_REG =
{
    "PROFILES_CNTXIN_MASK_6",
#if RU_INCLUDE_DESC
    "CNTXIN_MASK 0..7 Register",
    "Activating mask per Context\n16bits per context.\nTotal bits in this RAM is:NUM_OF_CNTX*NUM_OF_TCAM.\nLSB bits on lowest address corresponds to CNTX0.\n",
#endif
    { TCAM_PROFILES_CNTXIN_MASK_6_REG_OFFSET },
    TCAM_PROFILES_CNTXIN_MASK_6_REG_RAM_CNT,
    580,
    1965,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_CNTXIN_MASK_6_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_CNTXIN_MASK_7, TYPE: Type_TCAM_TCAM_PROFILES_CNTXIN_MASK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_CNTXIN_MASK_7_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_CNTXIN_MASK_7_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_CNTXIN_MASK_7_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_CNTXIN_MASK_7_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_CNTXIN_MASK_7_FIELDS[] =
{
    &TCAM_PROFILES_CNTXIN_MASK_7_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_CNTXIN_MASK_7 *****/
const ru_reg_rec TCAM_PROFILES_CNTXIN_MASK_7_REG =
{
    "PROFILES_CNTXIN_MASK_7",
#if RU_INCLUDE_DESC
    "CNTXIN_MASK 0..7 Register",
    "Activating mask per Context\n16bits per context.\nTotal bits in this RAM is:NUM_OF_CNTX*NUM_OF_TCAM.\nLSB bits on lowest address corresponds to CNTX0.\n",
#endif
    { TCAM_PROFILES_CNTXIN_MASK_7_REG_OFFSET },
    TCAM_PROFILES_CNTXIN_MASK_7_REG_RAM_CNT,
    580,
    1966,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_CNTXIN_MASK_7_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_SP_MASK_0, TYPE: Type_TCAM_TCAM_PROFILES_SP_MASK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_SP_MASK_0_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_SP_MASK_0_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_SP_MASK_0_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_SP_MASK_0_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_SP_MASK_0_FIELDS[] =
{
    &TCAM_PROFILES_SP_MASK_0_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_SP_MASK_0 *****/
const ru_reg_rec TCAM_PROFILES_SP_MASK_0_REG =
{
    "PROFILES_SP_MASK_0",
#if RU_INCLUDE_DESC
    "SP_MASK 0..3 Register",
    "Mask per SP to config which TCAM results participate in the priority selection.\nTotal bits in this RAM is:NUM_OF_SEARCHES(8)*NUM_OF_TCAMS.\nLSB bits on lowest address corresponds to SP0.\n",
#endif
    { TCAM_PROFILES_SP_MASK_0_REG_OFFSET },
    TCAM_PROFILES_SP_MASK_0_REG_RAM_CNT,
    580,
    1967,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_SP_MASK_0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_SP_MASK_1, TYPE: Type_TCAM_TCAM_PROFILES_SP_MASK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_SP_MASK_1_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_SP_MASK_1_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_SP_MASK_1_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_SP_MASK_1_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_SP_MASK_1_FIELDS[] =
{
    &TCAM_PROFILES_SP_MASK_1_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_SP_MASK_1 *****/
const ru_reg_rec TCAM_PROFILES_SP_MASK_1_REG =
{
    "PROFILES_SP_MASK_1",
#if RU_INCLUDE_DESC
    "SP_MASK 0..3 Register",
    "Mask per SP to config which TCAM results participate in the priority selection.\nTotal bits in this RAM is:NUM_OF_SEARCHES(8)*NUM_OF_TCAMS.\nLSB bits on lowest address corresponds to SP0.\n",
#endif
    { TCAM_PROFILES_SP_MASK_1_REG_OFFSET },
    TCAM_PROFILES_SP_MASK_1_REG_RAM_CNT,
    580,
    1968,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_SP_MASK_1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_SP_MASK_2, TYPE: Type_TCAM_TCAM_PROFILES_SP_MASK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_SP_MASK_2_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_SP_MASK_2_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_SP_MASK_2_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_SP_MASK_2_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_SP_MASK_2_FIELDS[] =
{
    &TCAM_PROFILES_SP_MASK_2_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_SP_MASK_2 *****/
const ru_reg_rec TCAM_PROFILES_SP_MASK_2_REG =
{
    "PROFILES_SP_MASK_2",
#if RU_INCLUDE_DESC
    "SP_MASK 0..3 Register",
    "Mask per SP to config which TCAM results participate in the priority selection.\nTotal bits in this RAM is:NUM_OF_SEARCHES(8)*NUM_OF_TCAMS.\nLSB bits on lowest address corresponds to SP0.\n",
#endif
    { TCAM_PROFILES_SP_MASK_2_REG_OFFSET },
    TCAM_PROFILES_SP_MASK_2_REG_RAM_CNT,
    580,
    1969,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_SP_MASK_2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_SP_MASK_3, TYPE: Type_TCAM_TCAM_PROFILES_SP_MASK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_SP_MASK_3_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_SP_MASK_3_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_SP_MASK_3_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_SP_MASK_3_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_SP_MASK_3_FIELDS[] =
{
    &TCAM_PROFILES_SP_MASK_3_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_SP_MASK_3 *****/
const ru_reg_rec TCAM_PROFILES_SP_MASK_3_REG =
{
    "PROFILES_SP_MASK_3",
#if RU_INCLUDE_DESC
    "SP_MASK 0..3 Register",
    "Mask per SP to config which TCAM results participate in the priority selection.\nTotal bits in this RAM is:NUM_OF_SEARCHES(8)*NUM_OF_TCAMS.\nLSB bits on lowest address corresponds to SP0.\n",
#endif
    { TCAM_PROFILES_SP_MASK_3_REG_OFFSET },
    TCAM_PROFILES_SP_MASK_3_REG_RAM_CNT,
    580,
    1970,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_SP_MASK_3_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_RSLT_MUX_0, TYPE: Type_TCAM_TCAM_PROFILES_RSLT_MUX
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_RSLT_MUX_0_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_RSLT_MUX_0_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_RSLT_MUX_0_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_RSLT_MUX_0_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_RSLT_MUX_0_FIELDS[] =
{
    &TCAM_PROFILES_RSLT_MUX_0_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_RSLT_MUX_0 *****/
const ru_reg_rec TCAM_PROFILES_RSLT_MUX_0_REG =
{
    "PROFILES_RSLT_MUX_0",
#if RU_INCLUDE_DESC
    "RSLT_MUX 0..3 Register",
    "There are 8 result Muxes. Each configured with a mask vector specifying which context memories are valid for arbitration.\n16bits per result mux.\nTotal bits in this RAM is:NUM_OF_RSLT_MUX(8)*NUM_OF_CNTX.\nLSB bits on lowest address corresponds to NUM_OF_RSLT_MUX0\n",
#endif
    { TCAM_PROFILES_RSLT_MUX_0_REG_OFFSET },
    TCAM_PROFILES_RSLT_MUX_0_REG_RAM_CNT,
    580,
    1971,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_RSLT_MUX_0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_RSLT_MUX_1, TYPE: Type_TCAM_TCAM_PROFILES_RSLT_MUX
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_RSLT_MUX_1_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_RSLT_MUX_1_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_RSLT_MUX_1_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_RSLT_MUX_1_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_RSLT_MUX_1_FIELDS[] =
{
    &TCAM_PROFILES_RSLT_MUX_1_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_RSLT_MUX_1 *****/
const ru_reg_rec TCAM_PROFILES_RSLT_MUX_1_REG =
{
    "PROFILES_RSLT_MUX_1",
#if RU_INCLUDE_DESC
    "RSLT_MUX 0..3 Register",
    "There are 8 result Muxes. Each configured with a mask vector specifying which context memories are valid for arbitration.\n16bits per result mux.\nTotal bits in this RAM is:NUM_OF_RSLT_MUX(8)*NUM_OF_CNTX.\nLSB bits on lowest address corresponds to NUM_OF_RSLT_MUX0\n",
#endif
    { TCAM_PROFILES_RSLT_MUX_1_REG_OFFSET },
    TCAM_PROFILES_RSLT_MUX_1_REG_RAM_CNT,
    580,
    1972,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_RSLT_MUX_1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_RSLT_MUX_2, TYPE: Type_TCAM_TCAM_PROFILES_RSLT_MUX
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_RSLT_MUX_2_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_RSLT_MUX_2_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_RSLT_MUX_2_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_RSLT_MUX_2_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_RSLT_MUX_2_FIELDS[] =
{
    &TCAM_PROFILES_RSLT_MUX_2_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_RSLT_MUX_2 *****/
const ru_reg_rec TCAM_PROFILES_RSLT_MUX_2_REG =
{
    "PROFILES_RSLT_MUX_2",
#if RU_INCLUDE_DESC
    "RSLT_MUX 0..3 Register",
    "There are 8 result Muxes. Each configured with a mask vector specifying which context memories are valid for arbitration.\n16bits per result mux.\nTotal bits in this RAM is:NUM_OF_RSLT_MUX(8)*NUM_OF_CNTX.\nLSB bits on lowest address corresponds to NUM_OF_RSLT_MUX0\n",
#endif
    { TCAM_PROFILES_RSLT_MUX_2_REG_OFFSET },
    TCAM_PROFILES_RSLT_MUX_2_REG_RAM_CNT,
    580,
    1973,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_RSLT_MUX_2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_RSLT_MUX_3, TYPE: Type_TCAM_TCAM_PROFILES_RSLT_MUX
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_RSLT_MUX_3_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_RSLT_MUX_3_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_RSLT_MUX_3_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_RSLT_MUX_3_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_RSLT_MUX_3_FIELDS[] =
{
    &TCAM_PROFILES_RSLT_MUX_3_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_RSLT_MUX_3 *****/
const ru_reg_rec TCAM_PROFILES_RSLT_MUX_3_REG =
{
    "PROFILES_RSLT_MUX_3",
#if RU_INCLUDE_DESC
    "RSLT_MUX 0..3 Register",
    "There are 8 result Muxes. Each configured with a mask vector specifying which context memories are valid for arbitration.\n16bits per result mux.\nTotal bits in this RAM is:NUM_OF_RSLT_MUX(8)*NUM_OF_CNTX.\nLSB bits on lowest address corresponds to NUM_OF_RSLT_MUX0\n",
#endif
    { TCAM_PROFILES_RSLT_MUX_3_REG_OFFSET },
    TCAM_PROFILES_RSLT_MUX_3_REG_RAM_CNT,
    580,
    1974,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_RSLT_MUX_3_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_RSLT_BUILDER_0, TYPE: Type_TCAM_TCAM_PROFILES_RSLT_BUILDER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_RSLT_BUILDER_0_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_RSLT_BUILDER_0_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_RSLT_BUILDER_0_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_RSLT_BUILDER_0_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_RSLT_BUILDER_0_FIELDS[] =
{
    &TCAM_PROFILES_RSLT_BUILDER_0_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_RSLT_BUILDER_0 *****/
const ru_reg_rec TCAM_PROFILES_RSLT_BUILDER_0_REG =
{
    "PROFILES_RSLT_BUILDER_0",
#if RU_INCLUDE_DESC
    "RSLT_BUILDER 0..7 Register",
    "Mapping between the 63B RSLT_MUX outputs and the final 31B result (Byte 64 of the result carries the match vector).\nEach result Byte can be assigned with any Byte from the RSLT_MUX outputs according to its configured index.\nIndex field width is 8bit\nMax value for index is 63.\nTotal bits in this RAM is:RSLT_BYTES*8.\nLSB bits on lowest address corresponds to CNTX0.\n",
#endif
    { TCAM_PROFILES_RSLT_BUILDER_0_REG_OFFSET },
    TCAM_PROFILES_RSLT_BUILDER_0_REG_RAM_CNT,
    580,
    1975,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_RSLT_BUILDER_0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_RSLT_BUILDER_1, TYPE: Type_TCAM_TCAM_PROFILES_RSLT_BUILDER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_RSLT_BUILDER_1_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_RSLT_BUILDER_1_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_RSLT_BUILDER_1_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_RSLT_BUILDER_1_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_RSLT_BUILDER_1_FIELDS[] =
{
    &TCAM_PROFILES_RSLT_BUILDER_1_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_RSLT_BUILDER_1 *****/
const ru_reg_rec TCAM_PROFILES_RSLT_BUILDER_1_REG =
{
    "PROFILES_RSLT_BUILDER_1",
#if RU_INCLUDE_DESC
    "RSLT_BUILDER 0..7 Register",
    "Mapping between the 63B RSLT_MUX outputs and the final 31B result (Byte 64 of the result carries the match vector).\nEach result Byte can be assigned with any Byte from the RSLT_MUX outputs according to its configured index.\nIndex field width is 8bit\nMax value for index is 63.\nTotal bits in this RAM is:RSLT_BYTES*8.\nLSB bits on lowest address corresponds to CNTX0.\n",
#endif
    { TCAM_PROFILES_RSLT_BUILDER_1_REG_OFFSET },
    TCAM_PROFILES_RSLT_BUILDER_1_REG_RAM_CNT,
    580,
    1976,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_RSLT_BUILDER_1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_RSLT_BUILDER_2, TYPE: Type_TCAM_TCAM_PROFILES_RSLT_BUILDER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_RSLT_BUILDER_2_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_RSLT_BUILDER_2_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_RSLT_BUILDER_2_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_RSLT_BUILDER_2_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_RSLT_BUILDER_2_FIELDS[] =
{
    &TCAM_PROFILES_RSLT_BUILDER_2_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_RSLT_BUILDER_2 *****/
const ru_reg_rec TCAM_PROFILES_RSLT_BUILDER_2_REG =
{
    "PROFILES_RSLT_BUILDER_2",
#if RU_INCLUDE_DESC
    "RSLT_BUILDER 0..7 Register",
    "Mapping between the 63B RSLT_MUX outputs and the final 31B result (Byte 64 of the result carries the match vector).\nEach result Byte can be assigned with any Byte from the RSLT_MUX outputs according to its configured index.\nIndex field width is 8bit\nMax value for index is 63.\nTotal bits in this RAM is:RSLT_BYTES*8.\nLSB bits on lowest address corresponds to CNTX0.\n",
#endif
    { TCAM_PROFILES_RSLT_BUILDER_2_REG_OFFSET },
    TCAM_PROFILES_RSLT_BUILDER_2_REG_RAM_CNT,
    580,
    1977,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_RSLT_BUILDER_2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_RSLT_BUILDER_3, TYPE: Type_TCAM_TCAM_PROFILES_RSLT_BUILDER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_RSLT_BUILDER_3_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_RSLT_BUILDER_3_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_RSLT_BUILDER_3_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_RSLT_BUILDER_3_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_RSLT_BUILDER_3_FIELDS[] =
{
    &TCAM_PROFILES_RSLT_BUILDER_3_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_RSLT_BUILDER_3 *****/
const ru_reg_rec TCAM_PROFILES_RSLT_BUILDER_3_REG =
{
    "PROFILES_RSLT_BUILDER_3",
#if RU_INCLUDE_DESC
    "RSLT_BUILDER 0..7 Register",
    "Mapping between the 63B RSLT_MUX outputs and the final 31B result (Byte 64 of the result carries the match vector).\nEach result Byte can be assigned with any Byte from the RSLT_MUX outputs according to its configured index.\nIndex field width is 8bit\nMax value for index is 63.\nTotal bits in this RAM is:RSLT_BYTES*8.\nLSB bits on lowest address corresponds to CNTX0.\n",
#endif
    { TCAM_PROFILES_RSLT_BUILDER_3_REG_OFFSET },
    TCAM_PROFILES_RSLT_BUILDER_3_REG_RAM_CNT,
    580,
    1978,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_RSLT_BUILDER_3_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_RSLT_BUILDER_4, TYPE: Type_TCAM_TCAM_PROFILES_RSLT_BUILDER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_RSLT_BUILDER_4_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_RSLT_BUILDER_4_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_RSLT_BUILDER_4_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_RSLT_BUILDER_4_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_RSLT_BUILDER_4_FIELDS[] =
{
    &TCAM_PROFILES_RSLT_BUILDER_4_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_RSLT_BUILDER_4 *****/
const ru_reg_rec TCAM_PROFILES_RSLT_BUILDER_4_REG =
{
    "PROFILES_RSLT_BUILDER_4",
#if RU_INCLUDE_DESC
    "RSLT_BUILDER 0..7 Register",
    "Mapping between the 63B RSLT_MUX outputs and the final 31B result (Byte 64 of the result carries the match vector).\nEach result Byte can be assigned with any Byte from the RSLT_MUX outputs according to its configured index.\nIndex field width is 8bit\nMax value for index is 63.\nTotal bits in this RAM is:RSLT_BYTES*8.\nLSB bits on lowest address corresponds to CNTX0.\n",
#endif
    { TCAM_PROFILES_RSLT_BUILDER_4_REG_OFFSET },
    TCAM_PROFILES_RSLT_BUILDER_4_REG_RAM_CNT,
    580,
    1979,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_RSLT_BUILDER_4_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_RSLT_BUILDER_5, TYPE: Type_TCAM_TCAM_PROFILES_RSLT_BUILDER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_RSLT_BUILDER_5_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_RSLT_BUILDER_5_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_RSLT_BUILDER_5_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_RSLT_BUILDER_5_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_RSLT_BUILDER_5_FIELDS[] =
{
    &TCAM_PROFILES_RSLT_BUILDER_5_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_RSLT_BUILDER_5 *****/
const ru_reg_rec TCAM_PROFILES_RSLT_BUILDER_5_REG =
{
    "PROFILES_RSLT_BUILDER_5",
#if RU_INCLUDE_DESC
    "RSLT_BUILDER 0..7 Register",
    "Mapping between the 63B RSLT_MUX outputs and the final 31B result (Byte 64 of the result carries the match vector).\nEach result Byte can be assigned with any Byte from the RSLT_MUX outputs according to its configured index.\nIndex field width is 8bit\nMax value for index is 63.\nTotal bits in this RAM is:RSLT_BYTES*8.\nLSB bits on lowest address corresponds to CNTX0.\n",
#endif
    { TCAM_PROFILES_RSLT_BUILDER_5_REG_OFFSET },
    TCAM_PROFILES_RSLT_BUILDER_5_REG_RAM_CNT,
    580,
    1980,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_RSLT_BUILDER_5_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_RSLT_BUILDER_6, TYPE: Type_TCAM_TCAM_PROFILES_RSLT_BUILDER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_RSLT_BUILDER_6_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_RSLT_BUILDER_6_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_RSLT_BUILDER_6_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_RSLT_BUILDER_6_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_RSLT_BUILDER_6_FIELDS[] =
{
    &TCAM_PROFILES_RSLT_BUILDER_6_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_RSLT_BUILDER_6 *****/
const ru_reg_rec TCAM_PROFILES_RSLT_BUILDER_6_REG =
{
    "PROFILES_RSLT_BUILDER_6",
#if RU_INCLUDE_DESC
    "RSLT_BUILDER 0..7 Register",
    "Mapping between the 63B RSLT_MUX outputs and the final 31B result (Byte 64 of the result carries the match vector).\nEach result Byte can be assigned with any Byte from the RSLT_MUX outputs according to its configured index.\nIndex field width is 8bit\nMax value for index is 63.\nTotal bits in this RAM is:RSLT_BYTES*8.\nLSB bits on lowest address corresponds to CNTX0.\n",
#endif
    { TCAM_PROFILES_RSLT_BUILDER_6_REG_OFFSET },
    TCAM_PROFILES_RSLT_BUILDER_6_REG_RAM_CNT,
    580,
    1981,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_RSLT_BUILDER_6_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_RSLT_BUILDER_7, TYPE: Type_TCAM_TCAM_PROFILES_RSLT_BUILDER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RAM_LINE *****/
const ru_field_rec TCAM_PROFILES_RSLT_BUILDER_7_RAM_LINE_FIELD =
{
    "RAM_LINE",
#if RU_INCLUDE_DESC
    "",
    "RAM_LINE\n",
#endif
    { TCAM_PROFILES_RSLT_BUILDER_7_RAM_LINE_FIELD_MASK },
    0,
    { TCAM_PROFILES_RSLT_BUILDER_7_RAM_LINE_FIELD_WIDTH },
    { TCAM_PROFILES_RSLT_BUILDER_7_RAM_LINE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_RSLT_BUILDER_7_FIELDS[] =
{
    &TCAM_PROFILES_RSLT_BUILDER_7_RAM_LINE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_RSLT_BUILDER_7 *****/
const ru_reg_rec TCAM_PROFILES_RSLT_BUILDER_7_REG =
{
    "PROFILES_RSLT_BUILDER_7",
#if RU_INCLUDE_DESC
    "RSLT_BUILDER 0..7 Register",
    "Mapping between the 63B RSLT_MUX outputs and the final 31B result (Byte 64 of the result carries the match vector).\nEach result Byte can be assigned with any Byte from the RSLT_MUX outputs according to its configured index.\nIndex field width is 8bit\nMax value for index is 63.\nTotal bits in this RAM is:RSLT_BYTES*8.\nLSB bits on lowest address corresponds to CNTX0.\n",
#endif
    { TCAM_PROFILES_RSLT_BUILDER_7_REG_OFFSET },
    TCAM_PROFILES_RSLT_BUILDER_7_REG_RAM_CNT,
    580,
    1982,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_RSLT_BUILDER_7_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: TCAM_PROFILES_TCAM_CONCAT, TYPE: Type_TCAM_TCAM_PROFILES_TCAM_CONCAT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: EN *****/
const ru_field_rec TCAM_PROFILES_TCAM_CONCAT_EN_FIELD =
{
    "EN",
#if RU_INCLUDE_DESC
    "",
    "EN\n",
#endif
    { TCAM_PROFILES_TCAM_CONCAT_EN_FIELD_MASK },
    0,
    { TCAM_PROFILES_TCAM_CONCAT_EN_FIELD_WIDTH },
    { TCAM_PROFILES_TCAM_CONCAT_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TCAM_PROFILES_TCAM_CONCAT_FIELDS[] =
{
    &TCAM_PROFILES_TCAM_CONCAT_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: TCAM_PROFILES_TCAM_CONCAT *****/
const ru_reg_rec TCAM_PROFILES_TCAM_CONCAT_REG =
{
    "PROFILES_TCAM_CONCAT",
#if RU_INCLUDE_DESC
    "TCAM_CONCAT Register",
    "TCAM concatenation CFG\nEach bit indicate enables match-in from a consecutive (higher id) TCAM.\nBits 15,11,7,3 should be set low (forced by HW)\n",
#endif
    { TCAM_PROFILES_TCAM_CONCAT_REG_OFFSET },
    TCAM_PROFILES_TCAM_CONCAT_REG_RAM_CNT,
    580,
    1983,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TCAM_PROFILES_TCAM_CONCAT_FIELDS,
#endif
};

unsigned long TCAM_ADDRS[] =
{
    0xB4B40000,
};

static const ru_reg_rec *TCAM_REGS[] =
{
    &TCAM_CFG_DINM_REG,
    &TCAM_CFG_BLKSEL_REG,
    &TCAM_CFG_TM_REG,
    &TCAM_CFG_VRESET_REG,
    &TCAM_COUNTERS_SEARCHES_REG,
    &TCAM_COUNTERS_HITS_REG,
    &TCAM_TCAM_MEM_INDIRECT_OP_REG,
    &TCAM_TCAM_MEM_INDIRECT_OP_DONE_REG,
    &TCAM_TCAM_MEM_INDIRECT_ADDR_REG,
    &TCAM_TCAM_MEM_INDIRECT_VALID_IN_REG,
    &TCAM_TCAM_MEM_INDIRECT_VALID_OUT_REG,
    &TCAM_TCAM_MEM_INDIRECT_DATA_OUT_REG,
    &TCAM_TCAM_MEM_INDIRECT_DATA_IN_REG,
    &TCAM_TCAM_DEBUG_BUS_SELECT_REG,
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_0_REG,
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_1_REG,
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_2_REG,
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_3_REG,
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_4_REG,
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_5_REG,
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_6_REG,
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_7_REG,
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_8_REG,
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_9_REG,
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_10_REG,
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_11_REG,
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_12_REG,
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_13_REG,
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_14_REG,
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_15_REG,
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_16_REG,
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_17_REG,
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_18_REG,
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_19_REG,
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_20_REG,
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_21_REG,
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_22_REG,
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_23_REG,
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_24_REG,
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_25_REG,
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_26_REG,
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_27_REG,
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_28_REG,
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_29_REG,
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_30_REG,
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_31_REG,
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_32_REG,
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_33_REG,
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_34_REG,
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_35_REG,
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_36_REG,
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_37_REG,
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_38_REG,
    &TCAM_PROFILES_KEY_BUILDER_OFFSETS_39_REG,
    &TCAM_PROFILES_INNER_SHIFT_0_REG,
    &TCAM_PROFILES_INNER_SHIFT_1_REG,
    &TCAM_PROFILES_INNER_SHIFT_2_REG,
    &TCAM_PROFILES_INNER_SHIFT_3_REG,
    &TCAM_PROFILES_INNER_SHIFT_4_REG,
    &TCAM_PROFILES_INNER_SHIFT_5_REG,
    &TCAM_PROFILES_INNER_SHIFT_6_REG,
    &TCAM_PROFILES_INNER_SHIFT_7_REG,
    &TCAM_PROFILES_INNER_SHIFT_8_REG,
    &TCAM_PROFILES_INNER_SHIFT_9_REG,
    &TCAM_PROFILES_INNER_SHIFT_10_REG,
    &TCAM_PROFILES_INNER_SHIFT_11_REG,
    &TCAM_PROFILES_INNER_SHIFT_12_REG,
    &TCAM_PROFILES_INNER_SHIFT_13_REG,
    &TCAM_PROFILES_INNER_SHIFT_14_REG,
    &TCAM_PROFILES_INNER_SHIFT_15_REG,
    &TCAM_PROFILES_INNER_SHIFT_16_REG,
    &TCAM_PROFILES_INNER_SHIFT_17_REG,
    &TCAM_PROFILES_INNER_SHIFT_18_REG,
    &TCAM_PROFILES_INNER_SHIFT_19_REG,
    &TCAM_PROFILES_INNER_SHIFT_20_REG,
    &TCAM_PROFILES_INNER_SHIFT_21_REG,
    &TCAM_PROFILES_INNER_SHIFT_22_REG,
    &TCAM_PROFILES_INNER_SHIFT_23_REG,
    &TCAM_PROFILES_INNER_SHIFT_24_REG,
    &TCAM_PROFILES_INNER_SHIFT_25_REG,
    &TCAM_PROFILES_INNER_SHIFT_26_REG,
    &TCAM_PROFILES_INNER_SHIFT_27_REG,
    &TCAM_PROFILES_INNER_SHIFT_28_REG,
    &TCAM_PROFILES_INNER_SHIFT_29_REG,
    &TCAM_PROFILES_INNER_SHIFT_30_REG,
    &TCAM_PROFILES_INNER_SHIFT_31_REG,
    &TCAM_PROFILES_CNTXIN_SP_ID_0_REG,
    &TCAM_PROFILES_CNTXIN_SP_ID_1_REG,
    &TCAM_PROFILES_CNTXIN_MASK_0_REG,
    &TCAM_PROFILES_CNTXIN_MASK_1_REG,
    &TCAM_PROFILES_CNTXIN_MASK_2_REG,
    &TCAM_PROFILES_CNTXIN_MASK_3_REG,
    &TCAM_PROFILES_CNTXIN_MASK_4_REG,
    &TCAM_PROFILES_CNTXIN_MASK_5_REG,
    &TCAM_PROFILES_CNTXIN_MASK_6_REG,
    &TCAM_PROFILES_CNTXIN_MASK_7_REG,
    &TCAM_PROFILES_SP_MASK_0_REG,
    &TCAM_PROFILES_SP_MASK_1_REG,
    &TCAM_PROFILES_SP_MASK_2_REG,
    &TCAM_PROFILES_SP_MASK_3_REG,
    &TCAM_PROFILES_RSLT_MUX_0_REG,
    &TCAM_PROFILES_RSLT_MUX_1_REG,
    &TCAM_PROFILES_RSLT_MUX_2_REG,
    &TCAM_PROFILES_RSLT_MUX_3_REG,
    &TCAM_PROFILES_RSLT_BUILDER_0_REG,
    &TCAM_PROFILES_RSLT_BUILDER_1_REG,
    &TCAM_PROFILES_RSLT_BUILDER_2_REG,
    &TCAM_PROFILES_RSLT_BUILDER_3_REG,
    &TCAM_PROFILES_RSLT_BUILDER_4_REG,
    &TCAM_PROFILES_RSLT_BUILDER_5_REG,
    &TCAM_PROFILES_RSLT_BUILDER_6_REG,
    &TCAM_PROFILES_RSLT_BUILDER_7_REG,
    &TCAM_PROFILES_TCAM_CONCAT_REG,
};

const ru_block_rec TCAM_BLOCK =
{
    "TCAM",
#if RU_INCLUDE_SIZE
    { 0x00019B50 },
#endif
    TCAM_ADDRS,
    1,
    113,
    TCAM_REGS,
};
