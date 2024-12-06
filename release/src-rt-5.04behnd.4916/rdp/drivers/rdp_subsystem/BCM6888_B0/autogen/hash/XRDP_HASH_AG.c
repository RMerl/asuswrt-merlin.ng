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


#include "XRDP_HASH_AG.h"

/******************************************************************************
 * Register: NAME: HASH_GENERAL_CONFIGURATION_PWR_SAV_EN, TYPE: Type_HASH_BLK_HASH_GENERAL_CONFIGURATION_PWR_SAV_EN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALUE *****/
const ru_field_rec HASH_GENERAL_CONFIGURATION_PWR_SAV_EN_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { HASH_GENERAL_CONFIGURATION_PWR_SAV_EN_VALUE_FIELD_MASK },
    0,
    { HASH_GENERAL_CONFIGURATION_PWR_SAV_EN_VALUE_FIELD_WIDTH },
    { HASH_GENERAL_CONFIGURATION_PWR_SAV_EN_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_GENERAL_CONFIGURATION_PWR_SAV_EN_FIELDS[] =
{
    &HASH_GENERAL_CONFIGURATION_PWR_SAV_EN_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_GENERAL_CONFIGURATION_PWR_SAV_EN *****/
const ru_reg_rec HASH_GENERAL_CONFIGURATION_PWR_SAV_EN_REG =
{
    "GENERAL_CONFIGURATION_PWR_SAV_EN",
#if RU_INCLUDE_DESC
    "PWR_SAV_EN Register",
    "Power saving mode -\ndetect that the accelerator has no activity and enter to power saving mode\n",
#endif
    { HASH_GENERAL_CONFIGURATION_PWR_SAV_EN_REG_OFFSET },
    0,
    0,
    521,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_GENERAL_CONFIGURATION_PWR_SAV_EN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_GENERAL_CONFIGURATION_PAD_HIGH, TYPE: Type_HASH_BLK_HASH_GENERAL_CONFIGURATION_PAD_HIGH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec HASH_GENERAL_CONFIGURATION_PAD_HIGH_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "Determines the padding value added to keys according to the selected MASK\n",
#endif
    { HASH_GENERAL_CONFIGURATION_PAD_HIGH_VAL_FIELD_MASK },
    0,
    { HASH_GENERAL_CONFIGURATION_PAD_HIGH_VAL_FIELD_WIDTH },
    { HASH_GENERAL_CONFIGURATION_PAD_HIGH_VAL_FIELD_SHIFT },
    89478485,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_GENERAL_CONFIGURATION_PAD_HIGH_FIELDS[] =
{
    &HASH_GENERAL_CONFIGURATION_PAD_HIGH_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_GENERAL_CONFIGURATION_PAD_HIGH *****/
const ru_reg_rec HASH_GENERAL_CONFIGURATION_PAD_HIGH_REG =
{
    "GENERAL_CONFIGURATION_PAD_HIGH",
#if RU_INCLUDE_DESC
    "PAD_VAL_HIGH Register",
    "Determines the padding value added to keys according to the selected MASK\n",
#endif
    { HASH_GENERAL_CONFIGURATION_PAD_HIGH_REG_OFFSET },
    0,
    0,
    522,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_GENERAL_CONFIGURATION_PAD_HIGH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_GENERAL_CONFIGURATION_PAD_LOW, TYPE: Type_HASH_BLK_HASH_GENERAL_CONFIGURATION_PAD_LOW
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec HASH_GENERAL_CONFIGURATION_PAD_LOW_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "Determines the padding value added to keys according to the selected MASK\n",
#endif
    { HASH_GENERAL_CONFIGURATION_PAD_LOW_VAL_FIELD_MASK },
    0,
    { HASH_GENERAL_CONFIGURATION_PAD_LOW_VAL_FIELD_WIDTH },
    { HASH_GENERAL_CONFIGURATION_PAD_LOW_VAL_FIELD_SHIFT },
    1431655765,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_GENERAL_CONFIGURATION_PAD_LOW_FIELDS[] =
{
    &HASH_GENERAL_CONFIGURATION_PAD_LOW_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_GENERAL_CONFIGURATION_PAD_LOW *****/
const ru_reg_rec HASH_GENERAL_CONFIGURATION_PAD_LOW_REG =
{
    "GENERAL_CONFIGURATION_PAD_LOW",
#if RU_INCLUDE_DESC
    "PAD_VAL_LOW Register",
    "Determines the padding value added to keys according to the selected MASK\n",
#endif
    { HASH_GENERAL_CONFIGURATION_PAD_LOW_REG_OFFSET },
    0,
    0,
    523,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_GENERAL_CONFIGURATION_PAD_LOW_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_GENERAL_CONFIGURATION_MULT_HIT_ERR, TYPE: Type_HASH_BLK_HASH_GENERAL_CONFIGURATION_MULT_HIT_ERR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec HASH_GENERAL_CONFIGURATION_MULT_HIT_ERR_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "In case of multiple match this reg captures the hit indication per engine. This is a read clear reg.\n",
#endif
    { HASH_GENERAL_CONFIGURATION_MULT_HIT_ERR_VAL_FIELD_MASK },
    0,
    { HASH_GENERAL_CONFIGURATION_MULT_HIT_ERR_VAL_FIELD_WIDTH },
    { HASH_GENERAL_CONFIGURATION_MULT_HIT_ERR_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_GENERAL_CONFIGURATION_MULT_HIT_ERR_FIELDS[] =
{
    &HASH_GENERAL_CONFIGURATION_MULT_HIT_ERR_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_GENERAL_CONFIGURATION_MULT_HIT_ERR *****/
const ru_reg_rec HASH_GENERAL_CONFIGURATION_MULT_HIT_ERR_REG =
{
    "GENERAL_CONFIGURATION_MULT_HIT_ERR",
#if RU_INCLUDE_DESC
    "MULT_HIT_ERR Register",
    "In case of multiple match this reg captures the hit indication per engine. This is a read clear reg.\n",
#endif
    { HASH_GENERAL_CONFIGURATION_MULT_HIT_ERR_REG_OFFSET },
    0,
    0,
    524,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_GENERAL_CONFIGURATION_MULT_HIT_ERR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_GENERAL_CONFIGURATION_UNDO_FIX, TYPE: Type_HASH_BLK_HASH_GENERAL_CONFIGURATION_UNDO_FIX
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FRST_MUL_HIT *****/
const ru_field_rec HASH_GENERAL_CONFIGURATION_UNDO_FIX_FRST_MUL_HIT_FIELD =
{
    "FRST_MUL_HIT",
#if RU_INCLUDE_DESC
    "",
    "The bug fixed lacking in identification and reporting when a multiple hit occurs in the first search.\n\n",
#endif
    { HASH_GENERAL_CONFIGURATION_UNDO_FIX_FRST_MUL_HIT_FIELD_MASK },
    0,
    { HASH_GENERAL_CONFIGURATION_UNDO_FIX_FRST_MUL_HIT_FIELD_WIDTH },
    { HASH_GENERAL_CONFIGURATION_UNDO_FIX_FRST_MUL_HIT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_GENERAL_CONFIGURATION_UNDO_FIX_FIELDS[] =
{
    &HASH_GENERAL_CONFIGURATION_UNDO_FIX_FRST_MUL_HIT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_GENERAL_CONFIGURATION_UNDO_FIX *****/
const ru_reg_rec HASH_GENERAL_CONFIGURATION_UNDO_FIX_REG =
{
    "GENERAL_CONFIGURATION_UNDO_FIX",
#if RU_INCLUDE_DESC
    "UNDO_FIX Register",
    "Consist of chicken bit per specific fix\n",
#endif
    { HASH_GENERAL_CONFIGURATION_UNDO_FIX_REG_OFFSET },
    0,
    0,
    525,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_GENERAL_CONFIGURATION_UNDO_FIX_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_PM_COUNTERS_HITS, TYPE: Type_HASH_BLK_HASH_PM_COUNTERS_HITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CNT *****/
const ru_field_rec HASH_PM_COUNTERS_HITS_CNT_FIELD =
{
    "CNT",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { HASH_PM_COUNTERS_HITS_CNT_FIELD_MASK },
    0,
    { HASH_PM_COUNTERS_HITS_CNT_FIELD_WIDTH },
    { HASH_PM_COUNTERS_HITS_CNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_PM_COUNTERS_HITS_FIELDS[] =
{
    &HASH_PM_COUNTERS_HITS_CNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_PM_COUNTERS_HITS *****/
const ru_reg_rec HASH_PM_COUNTERS_HITS_REG =
{
    "PM_COUNTERS_HITS",
#if RU_INCLUDE_DESC
    "HITS Register",
    "Number of key hits\n\nThis reg is frozen when freeze bit asserted.\n",
#endif
    { HASH_PM_COUNTERS_HITS_REG_OFFSET },
    0,
    0,
    526,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_PM_COUNTERS_HITS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_PM_COUNTERS_SRCHS, TYPE: Type_HASH_BLK_HASH_PM_COUNTERS_SRCHS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CNT *****/
const ru_field_rec HASH_PM_COUNTERS_SRCHS_CNT_FIELD =
{
    "CNT",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { HASH_PM_COUNTERS_SRCHS_CNT_FIELD_MASK },
    0,
    { HASH_PM_COUNTERS_SRCHS_CNT_FIELD_WIDTH },
    { HASH_PM_COUNTERS_SRCHS_CNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_PM_COUNTERS_SRCHS_FIELDS[] =
{
    &HASH_PM_COUNTERS_SRCHS_CNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_PM_COUNTERS_SRCHS *****/
const ru_reg_rec HASH_PM_COUNTERS_SRCHS_REG =
{
    "PM_COUNTERS_SRCHS",
#if RU_INCLUDE_DESC
    "SEARCHES Register",
    "Number of key searches\n\nThis register is updated only when freeze register is not set\n",
#endif
    { HASH_PM_COUNTERS_SRCHS_REG_OFFSET },
    0,
    0,
    527,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_PM_COUNTERS_SRCHS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_PM_COUNTERS_MISS, TYPE: Type_HASH_BLK_HASH_PM_COUNTERS_MISS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CNT *****/
const ru_field_rec HASH_PM_COUNTERS_MISS_CNT_FIELD =
{
    "CNT",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { HASH_PM_COUNTERS_MISS_CNT_FIELD_MASK },
    0,
    { HASH_PM_COUNTERS_MISS_CNT_FIELD_WIDTH },
    { HASH_PM_COUNTERS_MISS_CNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_PM_COUNTERS_MISS_FIELDS[] =
{
    &HASH_PM_COUNTERS_MISS_CNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_PM_COUNTERS_MISS *****/
const ru_reg_rec HASH_PM_COUNTERS_MISS_REG =
{
    "PM_COUNTERS_MISS",
#if RU_INCLUDE_DESC
    "MISSES Register",
    "Total NUM of misses\nread clear register\nupdated only when freeze reg is 0\n",
#endif
    { HASH_PM_COUNTERS_MISS_REG_OFFSET },
    0,
    0,
    528,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_PM_COUNTERS_MISS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_PM_COUNTERS_HIT_1ST_ACS, TYPE: Type_HASH_BLK_HASH_PM_COUNTERS_HIT_1ST_ACS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CNT *****/
const ru_field_rec HASH_PM_COUNTERS_HIT_1ST_ACS_CNT_FIELD =
{
    "CNT",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { HASH_PM_COUNTERS_HIT_1ST_ACS_CNT_FIELD_MASK },
    0,
    { HASH_PM_COUNTERS_HIT_1ST_ACS_CNT_FIELD_WIDTH },
    { HASH_PM_COUNTERS_HIT_1ST_ACS_CNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_PM_COUNTERS_HIT_1ST_ACS_FIELDS[] =
{
    &HASH_PM_COUNTERS_HIT_1ST_ACS_CNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_PM_COUNTERS_HIT_1ST_ACS *****/
const ru_reg_rec HASH_PM_COUNTERS_HIT_1ST_ACS_REG =
{
    "PM_COUNTERS_HIT_1ST_ACS",
#if RU_INCLUDE_DESC
    "HIT_1ST_ACS Register",
    "Total NUM of misses\nread clear register\nupdated only when freeze reg is 0\n",
#endif
    { HASH_PM_COUNTERS_HIT_1ST_ACS_REG_OFFSET },
    0,
    0,
    529,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_PM_COUNTERS_HIT_1ST_ACS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_PM_COUNTERS_HIT_2ND_ACS, TYPE: Type_HASH_BLK_HASH_PM_COUNTERS_HIT_2ND_ACS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CNT *****/
const ru_field_rec HASH_PM_COUNTERS_HIT_2ND_ACS_CNT_FIELD =
{
    "CNT",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { HASH_PM_COUNTERS_HIT_2ND_ACS_CNT_FIELD_MASK },
    0,
    { HASH_PM_COUNTERS_HIT_2ND_ACS_CNT_FIELD_WIDTH },
    { HASH_PM_COUNTERS_HIT_2ND_ACS_CNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_PM_COUNTERS_HIT_2ND_ACS_FIELDS[] =
{
    &HASH_PM_COUNTERS_HIT_2ND_ACS_CNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_PM_COUNTERS_HIT_2ND_ACS *****/
const ru_reg_rec HASH_PM_COUNTERS_HIT_2ND_ACS_REG =
{
    "PM_COUNTERS_HIT_2ND_ACS",
#if RU_INCLUDE_DESC
    "HIT_2ND_ACS Register",
    "Total NUM of misses\nread clear register\nupdated only when freeze reg is 0\n",
#endif
    { HASH_PM_COUNTERS_HIT_2ND_ACS_REG_OFFSET },
    0,
    0,
    530,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_PM_COUNTERS_HIT_2ND_ACS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_PM_COUNTERS_HIT_3RD_ACS, TYPE: Type_HASH_BLK_HASH_PM_COUNTERS_HIT_3RD_ACS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CNT *****/
const ru_field_rec HASH_PM_COUNTERS_HIT_3RD_ACS_CNT_FIELD =
{
    "CNT",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { HASH_PM_COUNTERS_HIT_3RD_ACS_CNT_FIELD_MASK },
    0,
    { HASH_PM_COUNTERS_HIT_3RD_ACS_CNT_FIELD_WIDTH },
    { HASH_PM_COUNTERS_HIT_3RD_ACS_CNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_PM_COUNTERS_HIT_3RD_ACS_FIELDS[] =
{
    &HASH_PM_COUNTERS_HIT_3RD_ACS_CNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_PM_COUNTERS_HIT_3RD_ACS *****/
const ru_reg_rec HASH_PM_COUNTERS_HIT_3RD_ACS_REG =
{
    "PM_COUNTERS_HIT_3RD_ACS",
#if RU_INCLUDE_DESC
    "HIT_3RD_ACS Register",
    "Total NUM of misses\nread clear register\nupdated only when freeze reg is 0\n",
#endif
    { HASH_PM_COUNTERS_HIT_3RD_ACS_REG_OFFSET },
    0,
    0,
    531,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_PM_COUNTERS_HIT_3RD_ACS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_PM_COUNTERS_HIT_4TH_ACS, TYPE: Type_HASH_BLK_HASH_PM_COUNTERS_HIT_4TH_ACS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CNT *****/
const ru_field_rec HASH_PM_COUNTERS_HIT_4TH_ACS_CNT_FIELD =
{
    "CNT",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { HASH_PM_COUNTERS_HIT_4TH_ACS_CNT_FIELD_MASK },
    0,
    { HASH_PM_COUNTERS_HIT_4TH_ACS_CNT_FIELD_WIDTH },
    { HASH_PM_COUNTERS_HIT_4TH_ACS_CNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_PM_COUNTERS_HIT_4TH_ACS_FIELDS[] =
{
    &HASH_PM_COUNTERS_HIT_4TH_ACS_CNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_PM_COUNTERS_HIT_4TH_ACS *****/
const ru_reg_rec HASH_PM_COUNTERS_HIT_4TH_ACS_REG =
{
    "PM_COUNTERS_HIT_4TH_ACS",
#if RU_INCLUDE_DESC
    "HIT_4TH_ACS Register",
    "Total NUM of misses\nread clear register\nupdated only when freeze reg is 0\n",
#endif
    { HASH_PM_COUNTERS_HIT_4TH_ACS_REG_OFFSET },
    0,
    0,
    532,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_PM_COUNTERS_HIT_4TH_ACS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_PM_COUNTERS_FRZ_CNT, TYPE: Type_HASH_BLK_HASH_PM_COUNTERS_FRZ_CNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec HASH_PM_COUNTERS_FRZ_CNT_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "Freezes counters update\n",
#endif
    { HASH_PM_COUNTERS_FRZ_CNT_VAL_FIELD_MASK },
    0,
    { HASH_PM_COUNTERS_FRZ_CNT_VAL_FIELD_WIDTH },
    { HASH_PM_COUNTERS_FRZ_CNT_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_PM_COUNTERS_FRZ_CNT_FIELDS[] =
{
    &HASH_PM_COUNTERS_FRZ_CNT_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_PM_COUNTERS_FRZ_CNT *****/
const ru_reg_rec HASH_PM_COUNTERS_FRZ_CNT_REG =
{
    "PM_COUNTERS_FRZ_CNT",
#if RU_INCLUDE_DESC
    "FREEZE_CNT Register",
    "Freezes counters update - used to read all relevant counters at the same point.\n\n",
#endif
    { HASH_PM_COUNTERS_FRZ_CNT_REG_OFFSET },
    0,
    0,
    533,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_PM_COUNTERS_FRZ_CNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_LKUP_TBL_CFG_TBL_CFG, TYPE: Type_HASH_BLK_HASH_LKUP_TBL_CFG_TBL_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: HASH_BASE_ADDR *****/
const ru_field_rec HASH_LKUP_TBL_CFG_TBL_CFG_HASH_BASE_ADDR_FIELD =
{
    "HASH_BASE_ADDR",
#if RU_INCLUDE_DESC
    "",
    "Base address of the hash ram per engine.\nVaries between 0 to 1535\nIndicates from which address start looking the key.\nNote, base address must be aligned to table size - table size of 128 cant get base 64\n\nThis is true for all table sizes exclude 1.25K and 1.125K.\nFor those two table sizes, there is no limitation on base address and they can reside in any place in the hash RAM.\n",
#endif
    { HASH_LKUP_TBL_CFG_TBL_CFG_HASH_BASE_ADDR_FIELD_MASK },
    0,
    { HASH_LKUP_TBL_CFG_TBL_CFG_HASH_BASE_ADDR_FIELD_WIDTH },
    { HASH_LKUP_TBL_CFG_TBL_CFG_HASH_BASE_ADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TBL_SIZE *****/
const ru_field_rec HASH_LKUP_TBL_CFG_TBL_CFG_TBL_SIZE_FIELD =
{
    "TBL_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Number of entries in the  table per engine.\nTotal  entries should be multiplied with the number of engines - by 4.\n",
#endif
    { HASH_LKUP_TBL_CFG_TBL_CFG_TBL_SIZE_FIELD_MASK },
    0,
    { HASH_LKUP_TBL_CFG_TBL_CFG_TBL_SIZE_FIELD_WIDTH },
    { HASH_LKUP_TBL_CFG_TBL_CFG_TBL_SIZE_FIELD_SHIFT },
    4,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MAX_HOP *****/
const ru_field_rec HASH_LKUP_TBL_CFG_TBL_CFG_MAX_HOP_FIELD =
{
    "MAX_HOP",
#if RU_INCLUDE_DESC
    "",
    "Max Search depth per engine.\nSupports up to 16 and cannot exceed table size.\nFor performance requirement it should be limited to 4\n",
#endif
    { HASH_LKUP_TBL_CFG_TBL_CFG_MAX_HOP_FIELD_MASK },
    0,
    { HASH_LKUP_TBL_CFG_TBL_CFG_MAX_HOP_FIELD_WIDTH },
    { HASH_LKUP_TBL_CFG_TBL_CFG_MAX_HOP_FIELD_SHIFT },
    3,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CAM_EN *****/
const ru_field_rec HASH_LKUP_TBL_CFG_TBL_CFG_CAM_EN_FIELD =
{
    "CAM_EN",
#if RU_INCLUDE_DESC
    "",
    "CAM Search is enabled.\nIf the key not found in the hash table and this flag enabled the key will be searched in the CAm.\n",
#endif
    { HASH_LKUP_TBL_CFG_TBL_CFG_CAM_EN_FIELD_MASK },
    0,
    { HASH_LKUP_TBL_CFG_TBL_CFG_CAM_EN_FIELD_WIDTH },
    { HASH_LKUP_TBL_CFG_TBL_CFG_CAM_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DIRECT_LKUP_EN *****/
const ru_field_rec HASH_LKUP_TBL_CFG_TBL_CFG_DIRECT_LKUP_EN_FIELD =
{
    "DIRECT_LKUP_EN",
#if RU_INCLUDE_DESC
    "",
    "Direct lookup enable.\nAllows accessing the table without hash calculation- direct access.\nNot supported\n",
#endif
    { HASH_LKUP_TBL_CFG_TBL_CFG_DIRECT_LKUP_EN_FIELD_MASK },
    0,
    { HASH_LKUP_TBL_CFG_TBL_CFG_DIRECT_LKUP_EN_FIELD_WIDTH },
    { HASH_LKUP_TBL_CFG_TBL_CFG_DIRECT_LKUP_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: HASH_TYPE *****/
const ru_field_rec HASH_LKUP_TBL_CFG_TBL_CFG_HASH_TYPE_FIELD =
{
    "HASH_TYPE",
#if RU_INCLUDE_DESC
    "",
    "Hash function type\nNot supported\n",
#endif
    { HASH_LKUP_TBL_CFG_TBL_CFG_HASH_TYPE_FIELD_MASK },
    0,
    { HASH_LKUP_TBL_CFG_TBL_CFG_HASH_TYPE_FIELD_WIDTH },
    { HASH_LKUP_TBL_CFG_TBL_CFG_HASH_TYPE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: INT_CNTX_SIZE *****/
const ru_field_rec HASH_LKUP_TBL_CFG_TBL_CFG_INT_CNTX_SIZE_FIELD =
{
    "INT_CNTX_SIZE",
#if RU_INCLUDE_DESC
    "",
    "If the key smaller than 60 bit, then it supported to store in the remaining bits an internal context data 3B or 6B.\n",
#endif
    { HASH_LKUP_TBL_CFG_TBL_CFG_INT_CNTX_SIZE_FIELD_MASK },
    0,
    { HASH_LKUP_TBL_CFG_TBL_CFG_INT_CNTX_SIZE_FIELD_WIDTH },
    { HASH_LKUP_TBL_CFG_TBL_CFG_INT_CNTX_SIZE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_LKUP_TBL_CFG_TBL_CFG_FIELDS[] =
{
    &HASH_LKUP_TBL_CFG_TBL_CFG_HASH_BASE_ADDR_FIELD,
    &HASH_LKUP_TBL_CFG_TBL_CFG_TBL_SIZE_FIELD,
    &HASH_LKUP_TBL_CFG_TBL_CFG_MAX_HOP_FIELD,
    &HASH_LKUP_TBL_CFG_TBL_CFG_CAM_EN_FIELD,
    &HASH_LKUP_TBL_CFG_TBL_CFG_DIRECT_LKUP_EN_FIELD,
    &HASH_LKUP_TBL_CFG_TBL_CFG_HASH_TYPE_FIELD,
    &HASH_LKUP_TBL_CFG_TBL_CFG_INT_CNTX_SIZE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_LKUP_TBL_CFG_TBL_CFG *****/
const ru_reg_rec HASH_LKUP_TBL_CFG_TBL_CFG_REG =
{
    "LKUP_TBL_CFG_TBL_CFG",
#if RU_INCLUDE_DESC
    "TBL_CFG Register",
    "Look-up table :  Configuration of LUT: table params + main flag\n",
#endif
    { HASH_LKUP_TBL_CFG_TBL_CFG_REG_OFFSET },
    HASH_LKUP_TBL_CFG_TBL_CFG_REG_RAM_CNT,
    16,
    534,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    HASH_LKUP_TBL_CFG_TBL_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_LKUP_TBL_CFG_KEY_MASK_HIGH, TYPE: Type_HASH_BLK_HASH_LKUP_TBL_CFG_KEY_MASK_HIGH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MASKH *****/
const ru_field_rec HASH_LKUP_TBL_CFG_KEY_MASK_HIGH_MASKH_FIELD =
{
    "MASKH",
#if RU_INCLUDE_DESC
    "",
    "MASK HIGH applied on the 28 msb of the current part of key for the current search table.\nThe value used for padding purpose and comparison to the hash content.\n\n",
#endif
    { HASH_LKUP_TBL_CFG_KEY_MASK_HIGH_MASKH_FIELD_MASK },
    0,
    { HASH_LKUP_TBL_CFG_KEY_MASK_HIGH_MASKH_FIELD_WIDTH },
    { HASH_LKUP_TBL_CFG_KEY_MASK_HIGH_MASKH_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_LKUP_TBL_CFG_KEY_MASK_HIGH_FIELDS[] =
{
    &HASH_LKUP_TBL_CFG_KEY_MASK_HIGH_MASKH_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_LKUP_TBL_CFG_KEY_MASK_HIGH *****/
const ru_reg_rec HASH_LKUP_TBL_CFG_KEY_MASK_HIGH_REG =
{
    "LKUP_TBL_CFG_KEY_MASK_HIGH",
#if RU_INCLUDE_DESC
    "KEY_MASK_HIGH Register",
    "Look-up table : key Mask on bits [59:32]\nKey consist of 60-bit.\nby configuring mask the user can use different key lengths.\nif the key is smaller than 60 bit it is padded with constant value according the the mask register.\n",
#endif
    { HASH_LKUP_TBL_CFG_KEY_MASK_HIGH_REG_OFFSET },
    HASH_LKUP_TBL_CFG_KEY_MASK_HIGH_REG_RAM_CNT,
    16,
    535,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_LKUP_TBL_CFG_KEY_MASK_HIGH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_LKUP_TBL_CFG_KEY_MASK_LOW, TYPE: Type_HASH_BLK_HASH_LKUP_TBL_CFG_KEY_MASK_LOW
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MASKL *****/
const ru_field_rec HASH_LKUP_TBL_CFG_KEY_MASK_LOW_MASKL_FIELD =
{
    "MASKL",
#if RU_INCLUDE_DESC
    "",
    "MASK LOW applied on the 32 lsb of the current part of key for the current search table.\n\nThe value used for padding purpose and comparison to the hash content.\n",
#endif
    { HASH_LKUP_TBL_CFG_KEY_MASK_LOW_MASKL_FIELD_MASK },
    0,
    { HASH_LKUP_TBL_CFG_KEY_MASK_LOW_MASKL_FIELD_WIDTH },
    { HASH_LKUP_TBL_CFG_KEY_MASK_LOW_MASKL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_LKUP_TBL_CFG_KEY_MASK_LOW_FIELDS[] =
{
    &HASH_LKUP_TBL_CFG_KEY_MASK_LOW_MASKL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_LKUP_TBL_CFG_KEY_MASK_LOW *****/
const ru_reg_rec HASH_LKUP_TBL_CFG_KEY_MASK_LOW_REG =
{
    "LKUP_TBL_CFG_KEY_MASK_LOW",
#if RU_INCLUDE_DESC
    "KEY_MASK_LOW Register",
    "Look-up table key Mask on bits [31:0]\nKey consist of 60-bit.\n\nBy configuring mask the user can use different key lengths.\n\nIf the key is smaller than 60 bit it is padded with constant value according to the mask register.\n",
#endif
    { HASH_LKUP_TBL_CFG_KEY_MASK_LOW_REG_OFFSET },
    HASH_LKUP_TBL_CFG_KEY_MASK_LOW_REG_RAM_CNT,
    16,
    536,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_LKUP_TBL_CFG_KEY_MASK_LOW_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_LKUP_TBL_CFG_CNTXT_CFG, TYPE: Type_HASH_BLK_HASH_LKUP_TBL_CFG_CNTXT_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BASE_ADDRESS *****/
const ru_field_rec HASH_LKUP_TBL_CFG_CNTXT_CFG_BASE_ADDRESS_FIELD =
{
    "BASE_ADDRESS",
#if RU_INCLUDE_DESC
    "",
    "Context table base address in the RAM (6Bytes X 3264entries) .\nIndicates from which address start looking at the context.\nThe address varies between 0 to 3264 (including 196 CAM entries)\n\nIt should be calculated according below formula:\nContext_base_addr[12:0] = sum of (table_size_per_engine*num_of_eng*context_size)/6 for all preceding tables\n\n",
#endif
    { HASH_LKUP_TBL_CFG_CNTXT_CFG_BASE_ADDRESS_FIELD_MASK },
    0,
    { HASH_LKUP_TBL_CFG_CNTXT_CFG_BASE_ADDRESS_FIELD_WIDTH },
    { HASH_LKUP_TBL_CFG_CNTXT_CFG_BASE_ADDRESS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FIRST_HASH_IDX *****/
const ru_field_rec HASH_LKUP_TBL_CFG_CNTXT_CFG_FIRST_HASH_IDX_FIELD =
{
    "FIRST_HASH_IDX",
#if RU_INCLUDE_DESC
    "",
    "Indicates the first entry of the particular table in the context table.\n\nIt should be calculated according to below formula:\nFirst_hash_index = sum of (table_size_per_engine*num_of_eng) for all preceding tables\n",
#endif
    { HASH_LKUP_TBL_CFG_CNTXT_CFG_FIRST_HASH_IDX_FIELD_MASK },
    0,
    { HASH_LKUP_TBL_CFG_CNTXT_CFG_FIRST_HASH_IDX_FIELD_WIDTH },
    { HASH_LKUP_TBL_CFG_CNTXT_CFG_FIRST_HASH_IDX_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CNXT_SIZE *****/
const ru_field_rec HASH_LKUP_TBL_CFG_CNTXT_CFG_CNXT_SIZE_FIELD =
{
    "CNXT_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Context entry size (in the context RAM).\nVaries between 0B to 12B in steps of 3B\n\nContext may also be extracted directly from Look-up Table (up to 6B).\n",
#endif
    { HASH_LKUP_TBL_CFG_CNTXT_CFG_CNXT_SIZE_FIELD_MASK },
    0,
    { HASH_LKUP_TBL_CFG_CNTXT_CFG_CNXT_SIZE_FIELD_WIDTH },
    { HASH_LKUP_TBL_CFG_CNTXT_CFG_CNXT_SIZE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_LKUP_TBL_CFG_CNTXT_CFG_FIELDS[] =
{
    &HASH_LKUP_TBL_CFG_CNTXT_CFG_BASE_ADDRESS_FIELD,
    &HASH_LKUP_TBL_CFG_CNTXT_CFG_FIRST_HASH_IDX_FIELD,
    &HASH_LKUP_TBL_CFG_CNTXT_CFG_CNXT_SIZE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_LKUP_TBL_CFG_CNTXT_CFG *****/
const ru_reg_rec HASH_LKUP_TBL_CFG_CNTXT_CFG_REG =
{
    "LKUP_TBL_CFG_CNTXT_CFG",
#if RU_INCLUDE_DESC
    "CNTXT_CFG Register",
    "Look-up table: LUT Context Table configurations (base addr + entry context size)\n",
#endif
    { HASH_LKUP_TBL_CFG_CNTXT_CFG_REG_OFFSET },
    HASH_LKUP_TBL_CFG_CNTXT_CFG_REG_RAM_CNT,
    16,
    537,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    HASH_LKUP_TBL_CFG_CNTXT_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_CAM_CONFIGURATION_CNTXT_CFG, TYPE: Type_HASH_BLK_HASH_CAM_CONFIGURATION_CNTXT_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BASE_ADDRESS *****/
const ru_field_rec HASH_CAM_CONFIGURATION_CNTXT_CFG_BASE_ADDRESS_FIELD =
{
    "BASE_ADDRESS",
#if RU_INCLUDE_DESC
    "",
    "Context table base address in the RAM (6Bytes X 3264entries) .\nIndicates from which address start looking at the context.\nThe address varies between 0 to 3264 (including 196 CAM entries)\n\nIt should be calculated according below formula:\nContext_base_addr[12:0] = sum of (table_size*context_size)/6 for all preceding tables\n\n",
#endif
    { HASH_CAM_CONFIGURATION_CNTXT_CFG_BASE_ADDRESS_FIELD_MASK },
    0,
    { HASH_CAM_CONFIGURATION_CNTXT_CFG_BASE_ADDRESS_FIELD_WIDTH },
    { HASH_CAM_CONFIGURATION_CNTXT_CFG_BASE_ADDRESS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_CONFIGURATION_CNTXT_CFG_FIELDS[] =
{
    &HASH_CAM_CONFIGURATION_CNTXT_CFG_BASE_ADDRESS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_CAM_CONFIGURATION_CNTXT_CFG *****/
const ru_reg_rec HASH_CAM_CONFIGURATION_CNTXT_CFG_REG =
{
    "CAM_CONFIGURATION_CNTXT_CFG",
#if RU_INCLUDE_DESC
    "CNTXT_CFG Register",
    "Look-up table: LUT Context Table configurations (base addr + entry context size)\n",
#endif
    { HASH_CAM_CONFIGURATION_CNTXT_CFG_REG_OFFSET },
    0,
    0,
    538,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_CAM_CONFIGURATION_CNTXT_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_CAM_CONFIGURATION_TM_CFG, TYPE: Type_HASH_BLK_HASH_CAM_CONFIGURATION_TM_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec HASH_CAM_CONFIGURATION_TM_CFG_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "16bit for tm control\n",
#endif
    { HASH_CAM_CONFIGURATION_TM_CFG_VAL_FIELD_MASK },
    0,
    { HASH_CAM_CONFIGURATION_TM_CFG_VAL_FIELD_WIDTH },
    { HASH_CAM_CONFIGURATION_TM_CFG_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_CONFIGURATION_TM_CFG_FIELDS[] =
{
    &HASH_CAM_CONFIGURATION_TM_CFG_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_CAM_CONFIGURATION_TM_CFG *****/
const ru_reg_rec HASH_CAM_CONFIGURATION_TM_CFG_REG =
{
    "CAM_CONFIGURATION_TM_CFG",
#if RU_INCLUDE_DESC
    "TM_CTRL Register",
    "Functional control over CAMs TM bits\n",
#endif
    { HASH_CAM_CONFIGURATION_TM_CFG_REG_OFFSET },
    0,
    0,
    539,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_CAM_CONFIGURATION_TM_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_CAM_INDIRECT_OP, TYPE: Type_HASH_BLK_HASH_CAM_INDIRECT_OP
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CMD *****/
const ru_field_rec HASH_CAM_INDIRECT_OP_CMD_FIELD =
{
    "CMD",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { HASH_CAM_INDIRECT_OP_CMD_FIELD_MASK },
    0,
    { HASH_CAM_INDIRECT_OP_CMD_FIELD_WIDTH },
    { HASH_CAM_INDIRECT_OP_CMD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_INDIRECT_OP_FIELDS[] =
{
    &HASH_CAM_INDIRECT_OP_CMD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_CAM_INDIRECT_OP *****/
const ru_reg_rec HASH_CAM_INDIRECT_OP_REG =
{
    "CAM_INDIRECT_OP",
#if RU_INCLUDE_DESC
    "OPERATION Register",
    "TCAM Operation:\n0 - CAM READ\n1 - CAM Write\n2 - CAM Compare\n3 - CAM valid bit reset\nWriting to this register triggers the operation. All other relevant register should be ready before SW writes to this register.\n",
#endif
    { HASH_CAM_INDIRECT_OP_REG_OFFSET },
    0,
    0,
    540,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_CAM_INDIRECT_OP_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_CAM_INDIRECT_OP_DONE, TYPE: Type_HASH_BLK_HASH_CAM_INDIRECT_OP_DONE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec HASH_CAM_INDIRECT_OP_DONE_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { HASH_CAM_INDIRECT_OP_DONE_VAL_FIELD_MASK },
    0,
    { HASH_CAM_INDIRECT_OP_DONE_VAL_FIELD_WIDTH },
    { HASH_CAM_INDIRECT_OP_DONE_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_INDIRECT_OP_DONE_FIELDS[] =
{
    &HASH_CAM_INDIRECT_OP_DONE_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_CAM_INDIRECT_OP_DONE *****/
const ru_reg_rec HASH_CAM_INDIRECT_OP_DONE_REG =
{
    "CAM_INDIRECT_OP_DONE",
#if RU_INCLUDE_DESC
    "OPERATION_DONE Register",
    "Raised when the CAM operation is completed (cleared by HW on writing to the OPERATION register)\n",
#endif
    { HASH_CAM_INDIRECT_OP_DONE_REG_OFFSET },
    0,
    0,
    541,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_CAM_INDIRECT_OP_DONE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_CAM_INDIRECT_ADDR, TYPE: Type_HASH_BLK_HASH_CAM_INDIRECT_ADDR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: KEY1_IND *****/
const ru_field_rec HASH_CAM_INDIRECT_ADDR_KEY1_IND_FIELD =
{
    "KEY1_IND",
#if RU_INCLUDE_DESC
    "",
    "This bit indicate if the operation (RD/WR) is performed on the key0 or key1 part of the entry\n",
#endif
    { HASH_CAM_INDIRECT_ADDR_KEY1_IND_FIELD_MASK },
    0,
    { HASH_CAM_INDIRECT_ADDR_KEY1_IND_FIELD_WIDTH },
    { HASH_CAM_INDIRECT_ADDR_KEY1_IND_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ENTRY_ADDR *****/
const ru_field_rec HASH_CAM_INDIRECT_ADDR_ENTRY_ADDR_FIELD =
{
    "ENTRY_ADDR",
#if RU_INCLUDE_DESC
    "",
    "Address of the entry\n",
#endif
    { HASH_CAM_INDIRECT_ADDR_ENTRY_ADDR_FIELD_MASK },
    0,
    { HASH_CAM_INDIRECT_ADDR_ENTRY_ADDR_FIELD_WIDTH },
    { HASH_CAM_INDIRECT_ADDR_ENTRY_ADDR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_INDIRECT_ADDR_FIELDS[] =
{
    &HASH_CAM_INDIRECT_ADDR_KEY1_IND_FIELD,
    &HASH_CAM_INDIRECT_ADDR_ENTRY_ADDR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_CAM_INDIRECT_ADDR *****/
const ru_reg_rec HASH_CAM_INDIRECT_ADDR_REG =
{
    "CAM_INDIRECT_ADDR",
#if RU_INCLUDE_DESC
    "ADDRESS Register",
    "Key Address to be used in RD/WR opoerations.\n",
#endif
    { HASH_CAM_INDIRECT_ADDR_REG_OFFSET },
    0,
    0,
    542,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    HASH_CAM_INDIRECT_ADDR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_CAM_INDIRECT_VLID_IN, TYPE: Type_HASH_BLK_HASH_CAM_INDIRECT_VLID_IN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALID *****/
const ru_field_rec HASH_CAM_INDIRECT_VLID_IN_VALID_FIELD =
{
    "VALID",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { HASH_CAM_INDIRECT_VLID_IN_VALID_FIELD_MASK },
    0,
    { HASH_CAM_INDIRECT_VLID_IN_VALID_FIELD_WIDTH },
    { HASH_CAM_INDIRECT_VLID_IN_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_INDIRECT_VLID_IN_FIELDS[] =
{
    &HASH_CAM_INDIRECT_VLID_IN_VALID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_CAM_INDIRECT_VLID_IN *****/
const ru_reg_rec HASH_CAM_INDIRECT_VLID_IN_REG =
{
    "CAM_INDIRECT_VLID_IN",
#if RU_INCLUDE_DESC
    "VALID_IN Register",
    "Valid value to be written - this value is relevant during write operation on key0.\n",
#endif
    { HASH_CAM_INDIRECT_VLID_IN_REG_OFFSET },
    0,
    0,
    543,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_CAM_INDIRECT_VLID_IN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_CAM_INDIRECT_VLID_OUT, TYPE: Type_HASH_BLK_HASH_CAM_INDIRECT_VLID_OUT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALID *****/
const ru_field_rec HASH_CAM_INDIRECT_VLID_OUT_VALID_FIELD =
{
    "VALID",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { HASH_CAM_INDIRECT_VLID_OUT_VALID_FIELD_MASK },
    0,
    { HASH_CAM_INDIRECT_VLID_OUT_VALID_FIELD_WIDTH },
    { HASH_CAM_INDIRECT_VLID_OUT_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_INDIRECT_VLID_OUT_FIELDS[] =
{
    &HASH_CAM_INDIRECT_VLID_OUT_VALID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_CAM_INDIRECT_VLID_OUT *****/
const ru_reg_rec HASH_CAM_INDIRECT_VLID_OUT_REG =
{
    "CAM_INDIRECT_VLID_OUT",
#if RU_INCLUDE_DESC
    "VALID_OUT Register",
    "Valid value read from the CAM - this value is relevant during read operation on key0.\n",
#endif
    { HASH_CAM_INDIRECT_VLID_OUT_REG_OFFSET },
    0,
    0,
    544,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_CAM_INDIRECT_VLID_OUT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_CAM_INDIRECT_RSLT, TYPE: Type_HASH_BLK_HASH_CAM_INDIRECT_RSLT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MATCH *****/
const ru_field_rec HASH_CAM_INDIRECT_RSLT_MATCH_FIELD =
{
    "MATCH",
#if RU_INCLUDE_DESC
    "",
    "indicate if a match was found\n",
#endif
    { HASH_CAM_INDIRECT_RSLT_MATCH_FIELD_MASK },
    0,
    { HASH_CAM_INDIRECT_RSLT_MATCH_FIELD_WIDTH },
    { HASH_CAM_INDIRECT_RSLT_MATCH_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: INDEX *****/
const ru_field_rec HASH_CAM_INDIRECT_RSLT_INDEX_FIELD =
{
    "INDEX",
#if RU_INCLUDE_DESC
    "",
    "index related to a match result\n",
#endif
    { HASH_CAM_INDIRECT_RSLT_INDEX_FIELD_MASK },
    0,
    { HASH_CAM_INDIRECT_RSLT_INDEX_FIELD_WIDTH },
    { HASH_CAM_INDIRECT_RSLT_INDEX_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_INDIRECT_RSLT_FIELDS[] =
{
    &HASH_CAM_INDIRECT_RSLT_MATCH_FIELD,
    &HASH_CAM_INDIRECT_RSLT_INDEX_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_CAM_INDIRECT_RSLT *****/
const ru_reg_rec HASH_CAM_INDIRECT_RSLT_REG =
{
    "CAM_INDIRECT_RSLT",
#if RU_INCLUDE_DESC
    "SEARCH_RESULT Register",
    "The result of a search operation\n",
#endif
    { HASH_CAM_INDIRECT_RSLT_REG_OFFSET },
    0,
    0,
    545,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    HASH_CAM_INDIRECT_RSLT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_CAM_INDIRECT_KEY_IN, TYPE: Type_HASH_BLK_HASH_CAM_INDIRECT_KEY_IN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALUE *****/
const ru_field_rec HASH_CAM_INDIRECT_KEY_IN_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { HASH_CAM_INDIRECT_KEY_IN_VALUE_FIELD_MASK },
    0,
    { HASH_CAM_INDIRECT_KEY_IN_VALUE_FIELD_WIDTH },
    { HASH_CAM_INDIRECT_KEY_IN_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_INDIRECT_KEY_IN_FIELDS[] =
{
    &HASH_CAM_INDIRECT_KEY_IN_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_CAM_INDIRECT_KEY_IN *****/
const ru_reg_rec HASH_CAM_INDIRECT_KEY_IN_REG =
{
    "CAM_INDIRECT_KEY_IN",
#if RU_INCLUDE_DESC
    "KEY_IN 0..1 Register",
    "Key to be used in Write/Compare operations.\nThe Key is 64bit long and is represented by 2 registers. The lower address register corresponds to the most significant bits of the key.\n",
#endif
    { HASH_CAM_INDIRECT_KEY_IN_REG_OFFSET },
    HASH_CAM_INDIRECT_KEY_IN_REG_RAM_CNT,
    4,
    546,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_CAM_INDIRECT_KEY_IN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_CAM_INDIRECT_KEY_OUT, TYPE: Type_HASH_BLK_HASH_CAM_INDIRECT_KEY_OUT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALUE *****/
const ru_field_rec HASH_CAM_INDIRECT_KEY_OUT_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { HASH_CAM_INDIRECT_KEY_OUT_VALUE_FIELD_MASK },
    0,
    { HASH_CAM_INDIRECT_KEY_OUT_VALUE_FIELD_WIDTH },
    { HASH_CAM_INDIRECT_KEY_OUT_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_INDIRECT_KEY_OUT_FIELDS[] =
{
    &HASH_CAM_INDIRECT_KEY_OUT_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_CAM_INDIRECT_KEY_OUT *****/
const ru_reg_rec HASH_CAM_INDIRECT_KEY_OUT_REG =
{
    "CAM_INDIRECT_KEY_OUT",
#if RU_INCLUDE_DESC
    "KEY_OUT 0..1 Register",
    "Key returned from the CAM in a read operation. The Key is 64bit long and is represented by 2 registers. The lower address register correspond to the most significant bits of the key.\n",
#endif
    { HASH_CAM_INDIRECT_KEY_OUT_REG_OFFSET },
    HASH_CAM_INDIRECT_KEY_OUT_REG_RAM_CNT,
    4,
    547,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_CAM_INDIRECT_KEY_OUT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_CAM_BIST_BIST_STATUS, TYPE: Type_HASH_BLK_HASH_CAM_BIST_BIST_STATUS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALUE *****/
const ru_field_rec HASH_CAM_BIST_BIST_STATUS_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { HASH_CAM_BIST_BIST_STATUS_VALUE_FIELD_MASK },
    0,
    { HASH_CAM_BIST_BIST_STATUS_VALUE_FIELD_WIDTH },
    { HASH_CAM_BIST_BIST_STATUS_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_BIST_BIST_STATUS_FIELDS[] =
{
    &HASH_CAM_BIST_BIST_STATUS_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_CAM_BIST_BIST_STATUS *****/
const ru_reg_rec HASH_CAM_BIST_BIST_STATUS_REG =
{
    "CAM_BIST_BIST_STATUS",
#if RU_INCLUDE_DESC
    "BIST_STATUS Register",
    ".\n",
#endif
    { HASH_CAM_BIST_BIST_STATUS_REG_OFFSET },
    0,
    0,
    548,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_CAM_BIST_BIST_STATUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_CAM_BIST_BIST_DBG_COMPARE_EN, TYPE: Type_HASH_BLK_HASH_CAM_BIST_BIST_DBG_COMPARE_EN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALUE *****/
const ru_field_rec HASH_CAM_BIST_BIST_DBG_COMPARE_EN_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { HASH_CAM_BIST_BIST_DBG_COMPARE_EN_VALUE_FIELD_MASK },
    0,
    { HASH_CAM_BIST_BIST_DBG_COMPARE_EN_VALUE_FIELD_WIDTH },
    { HASH_CAM_BIST_BIST_DBG_COMPARE_EN_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_BIST_BIST_DBG_COMPARE_EN_FIELDS[] =
{
    &HASH_CAM_BIST_BIST_DBG_COMPARE_EN_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_CAM_BIST_BIST_DBG_COMPARE_EN *****/
const ru_reg_rec HASH_CAM_BIST_BIST_DBG_COMPARE_EN_REG =
{
    "CAM_BIST_BIST_DBG_COMPARE_EN",
#if RU_INCLUDE_DESC
    "BIST_DBG_COMPARE_EN Register",
    ".\n",
#endif
    { HASH_CAM_BIST_BIST_DBG_COMPARE_EN_REG_OFFSET },
    0,
    0,
    549,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_CAM_BIST_BIST_DBG_COMPARE_EN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_CAM_BIST_BIST_DBG_DATA, TYPE: Type_HASH_BLK_HASH_CAM_BIST_BIST_DBG_DATA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALUE *****/
const ru_field_rec HASH_CAM_BIST_BIST_DBG_DATA_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { HASH_CAM_BIST_BIST_DBG_DATA_VALUE_FIELD_MASK },
    0,
    { HASH_CAM_BIST_BIST_DBG_DATA_VALUE_FIELD_WIDTH },
    { HASH_CAM_BIST_BIST_DBG_DATA_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_BIST_BIST_DBG_DATA_FIELDS[] =
{
    &HASH_CAM_BIST_BIST_DBG_DATA_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_CAM_BIST_BIST_DBG_DATA *****/
const ru_reg_rec HASH_CAM_BIST_BIST_DBG_DATA_REG =
{
    "CAM_BIST_BIST_DBG_DATA",
#if RU_INCLUDE_DESC
    "BIST_DBG_DATA Register",
    ".\n",
#endif
    { HASH_CAM_BIST_BIST_DBG_DATA_REG_OFFSET },
    0,
    0,
    550,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_CAM_BIST_BIST_DBG_DATA_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_CAM_BIST_BIST_DBG_DATA_SLICE_OR_STATUS_SEL, TYPE: Type_HASH_BLK_HASH_CAM_BIST_BIST_DBG_DATA_SLICE_OR_STATUS_SEL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALUE *****/
const ru_field_rec HASH_CAM_BIST_BIST_DBG_DATA_SLICE_OR_STATUS_SEL_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { HASH_CAM_BIST_BIST_DBG_DATA_SLICE_OR_STATUS_SEL_VALUE_FIELD_MASK },
    0,
    { HASH_CAM_BIST_BIST_DBG_DATA_SLICE_OR_STATUS_SEL_VALUE_FIELD_WIDTH },
    { HASH_CAM_BIST_BIST_DBG_DATA_SLICE_OR_STATUS_SEL_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_BIST_BIST_DBG_DATA_SLICE_OR_STATUS_SEL_FIELDS[] =
{
    &HASH_CAM_BIST_BIST_DBG_DATA_SLICE_OR_STATUS_SEL_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_CAM_BIST_BIST_DBG_DATA_SLICE_OR_STATUS_SEL *****/
const ru_reg_rec HASH_CAM_BIST_BIST_DBG_DATA_SLICE_OR_STATUS_SEL_REG =
{
    "CAM_BIST_BIST_DBG_DATA_SLICE_OR_STATUS_SEL",
#if RU_INCLUDE_DESC
    "BIST_DBG_DATA_SLICE_OR_STATUS_SEL Register",
    ".\n",
#endif
    { HASH_CAM_BIST_BIST_DBG_DATA_SLICE_OR_STATUS_SEL_REG_OFFSET },
    0,
    0,
    551,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_CAM_BIST_BIST_DBG_DATA_SLICE_OR_STATUS_SEL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_CAM_BIST_BIST_DBG_DATA_VALID, TYPE: Type_HASH_BLK_HASH_CAM_BIST_BIST_DBG_DATA_VALID
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALUE *****/
const ru_field_rec HASH_CAM_BIST_BIST_DBG_DATA_VALID_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { HASH_CAM_BIST_BIST_DBG_DATA_VALID_VALUE_FIELD_MASK },
    0,
    { HASH_CAM_BIST_BIST_DBG_DATA_VALID_VALUE_FIELD_WIDTH },
    { HASH_CAM_BIST_BIST_DBG_DATA_VALID_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_BIST_BIST_DBG_DATA_VALID_FIELDS[] =
{
    &HASH_CAM_BIST_BIST_DBG_DATA_VALID_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_CAM_BIST_BIST_DBG_DATA_VALID *****/
const ru_reg_rec HASH_CAM_BIST_BIST_DBG_DATA_VALID_REG =
{
    "CAM_BIST_BIST_DBG_DATA_VALID",
#if RU_INCLUDE_DESC
    "BIST_DBG_DATA_VALID Register",
    ".\n",
#endif
    { HASH_CAM_BIST_BIST_DBG_DATA_VALID_REG_OFFSET },
    0,
    0,
    552,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_CAM_BIST_BIST_DBG_DATA_VALID_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_CAM_BIST_BIST_EN, TYPE: Type_HASH_BLK_HASH_CAM_BIST_BIST_EN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALUE *****/
const ru_field_rec HASH_CAM_BIST_BIST_EN_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { HASH_CAM_BIST_BIST_EN_VALUE_FIELD_MASK },
    0,
    { HASH_CAM_BIST_BIST_EN_VALUE_FIELD_WIDTH },
    { HASH_CAM_BIST_BIST_EN_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_BIST_BIST_EN_FIELDS[] =
{
    &HASH_CAM_BIST_BIST_EN_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_CAM_BIST_BIST_EN *****/
const ru_reg_rec HASH_CAM_BIST_BIST_EN_REG =
{
    "CAM_BIST_BIST_EN",
#if RU_INCLUDE_DESC
    "BIST_EN Register",
    ".\n",
#endif
    { HASH_CAM_BIST_BIST_EN_REG_OFFSET },
    0,
    0,
    553,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_CAM_BIST_BIST_EN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_CAM_BIST_BIST_MODE, TYPE: Type_HASH_BLK_HASH_CAM_BIST_BIST_MODE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALUE *****/
const ru_field_rec HASH_CAM_BIST_BIST_MODE_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { HASH_CAM_BIST_BIST_MODE_VALUE_FIELD_MASK },
    0,
    { HASH_CAM_BIST_BIST_MODE_VALUE_FIELD_WIDTH },
    { HASH_CAM_BIST_BIST_MODE_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_BIST_BIST_MODE_FIELDS[] =
{
    &HASH_CAM_BIST_BIST_MODE_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_CAM_BIST_BIST_MODE *****/
const ru_reg_rec HASH_CAM_BIST_BIST_MODE_REG =
{
    "CAM_BIST_BIST_MODE",
#if RU_INCLUDE_DESC
    "BIST_MODE Register",
    ".\n",
#endif
    { HASH_CAM_BIST_BIST_MODE_REG_OFFSET },
    0,
    0,
    554,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_CAM_BIST_BIST_MODE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_CAM_BIST_BIST_RST_L, TYPE: Type_HASH_BLK_HASH_CAM_BIST_BIST_RST_L
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALUE *****/
const ru_field_rec HASH_CAM_BIST_BIST_RST_L_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { HASH_CAM_BIST_BIST_RST_L_VALUE_FIELD_MASK },
    0,
    { HASH_CAM_BIST_BIST_RST_L_VALUE_FIELD_WIDTH },
    { HASH_CAM_BIST_BIST_RST_L_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_BIST_BIST_RST_L_FIELDS[] =
{
    &HASH_CAM_BIST_BIST_RST_L_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_CAM_BIST_BIST_RST_L *****/
const ru_reg_rec HASH_CAM_BIST_BIST_RST_L_REG =
{
    "CAM_BIST_BIST_RST_L",
#if RU_INCLUDE_DESC
    "BIST_RST_L Register",
    ".\n",
#endif
    { HASH_CAM_BIST_BIST_RST_L_REG_OFFSET },
    0,
    0,
    555,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_CAM_BIST_BIST_RST_L_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_CAM_BIST_BIST_SKIP_ERROR_CNT, TYPE: Type_HASH_BLK_HASH_CAM_BIST_BIST_SKIP_ERROR_CNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALUE *****/
const ru_field_rec HASH_CAM_BIST_BIST_SKIP_ERROR_CNT_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { HASH_CAM_BIST_BIST_SKIP_ERROR_CNT_VALUE_FIELD_MASK },
    0,
    { HASH_CAM_BIST_BIST_SKIP_ERROR_CNT_VALUE_FIELD_WIDTH },
    { HASH_CAM_BIST_BIST_SKIP_ERROR_CNT_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_BIST_BIST_SKIP_ERROR_CNT_FIELDS[] =
{
    &HASH_CAM_BIST_BIST_SKIP_ERROR_CNT_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_CAM_BIST_BIST_SKIP_ERROR_CNT *****/
const ru_reg_rec HASH_CAM_BIST_BIST_SKIP_ERROR_CNT_REG =
{
    "CAM_BIST_BIST_SKIP_ERROR_CNT",
#if RU_INCLUDE_DESC
    "BIST_SKIP_ERROR_CNT Register",
    ".\n",
#endif
    { HASH_CAM_BIST_BIST_SKIP_ERROR_CNT_REG_OFFSET },
    0,
    0,
    556,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_CAM_BIST_BIST_SKIP_ERROR_CNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_CAM_BIST_DBG_EN, TYPE: Type_HASH_BLK_HASH_CAM_BIST_DBG_EN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALUE *****/
const ru_field_rec HASH_CAM_BIST_DBG_EN_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { HASH_CAM_BIST_DBG_EN_VALUE_FIELD_MASK },
    0,
    { HASH_CAM_BIST_DBG_EN_VALUE_FIELD_WIDTH },
    { HASH_CAM_BIST_DBG_EN_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_BIST_DBG_EN_FIELDS[] =
{
    &HASH_CAM_BIST_DBG_EN_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_CAM_BIST_DBG_EN *****/
const ru_reg_rec HASH_CAM_BIST_DBG_EN_REG =
{
    "CAM_BIST_DBG_EN",
#if RU_INCLUDE_DESC
    "DBG_EN Register",
    ".\n",
#endif
    { HASH_CAM_BIST_DBG_EN_REG_OFFSET },
    0,
    0,
    557,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_CAM_BIST_DBG_EN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_CAM_BIST_BIST_CASCADE_SELECT, TYPE: Type_HASH_BLK_HASH_CAM_BIST_BIST_CASCADE_SELECT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALUE *****/
const ru_field_rec HASH_CAM_BIST_BIST_CASCADE_SELECT_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { HASH_CAM_BIST_BIST_CASCADE_SELECT_VALUE_FIELD_MASK },
    0,
    { HASH_CAM_BIST_BIST_CASCADE_SELECT_VALUE_FIELD_WIDTH },
    { HASH_CAM_BIST_BIST_CASCADE_SELECT_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_BIST_BIST_CASCADE_SELECT_FIELDS[] =
{
    &HASH_CAM_BIST_BIST_CASCADE_SELECT_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_CAM_BIST_BIST_CASCADE_SELECT *****/
const ru_reg_rec HASH_CAM_BIST_BIST_CASCADE_SELECT_REG =
{
    "CAM_BIST_BIST_CASCADE_SELECT",
#if RU_INCLUDE_DESC
    "BIST_CASCADE_SELECT Register",
    ".\n",
#endif
    { HASH_CAM_BIST_BIST_CASCADE_SELECT_REG_OFFSET },
    0,
    0,
    558,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_CAM_BIST_BIST_CASCADE_SELECT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_CAM_BIST_BIST_BLOCK_SELECT, TYPE: Type_HASH_BLK_HASH_CAM_BIST_BIST_BLOCK_SELECT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALUE *****/
const ru_field_rec HASH_CAM_BIST_BIST_BLOCK_SELECT_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { HASH_CAM_BIST_BIST_BLOCK_SELECT_VALUE_FIELD_MASK },
    0,
    { HASH_CAM_BIST_BIST_BLOCK_SELECT_VALUE_FIELD_WIDTH },
    { HASH_CAM_BIST_BIST_BLOCK_SELECT_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_BIST_BIST_BLOCK_SELECT_FIELDS[] =
{
    &HASH_CAM_BIST_BIST_BLOCK_SELECT_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_CAM_BIST_BIST_BLOCK_SELECT *****/
const ru_reg_rec HASH_CAM_BIST_BIST_BLOCK_SELECT_REG =
{
    "CAM_BIST_BIST_BLOCK_SELECT",
#if RU_INCLUDE_DESC
    "BIST_BLOCK_SELECT Register",
    ".\n",
#endif
    { HASH_CAM_BIST_BIST_BLOCK_SELECT_REG_OFFSET },
    0,
    0,
    559,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_CAM_BIST_BIST_BLOCK_SELECT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_CAM_BIST_BIST_REPAIR_ENABLE, TYPE: Type_HASH_BLK_HASH_CAM_BIST_BIST_REPAIR_ENABLE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALUE *****/
const ru_field_rec HASH_CAM_BIST_BIST_REPAIR_ENABLE_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { HASH_CAM_BIST_BIST_REPAIR_ENABLE_VALUE_FIELD_MASK },
    0,
    { HASH_CAM_BIST_BIST_REPAIR_ENABLE_VALUE_FIELD_WIDTH },
    { HASH_CAM_BIST_BIST_REPAIR_ENABLE_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_BIST_BIST_REPAIR_ENABLE_FIELDS[] =
{
    &HASH_CAM_BIST_BIST_REPAIR_ENABLE_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_CAM_BIST_BIST_REPAIR_ENABLE *****/
const ru_reg_rec HASH_CAM_BIST_BIST_REPAIR_ENABLE_REG =
{
    "CAM_BIST_BIST_REPAIR_ENABLE",
#if RU_INCLUDE_DESC
    "BIST_REPAIR_ENABLE Register",
    ".\n",
#endif
    { HASH_CAM_BIST_BIST_REPAIR_ENABLE_REG_OFFSET },
    0,
    0,
    560,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_CAM_BIST_BIST_REPAIR_ENABLE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_INTR_CTRL_ISR, TYPE: Type_HASH_BLK_HASH_INTR_CTRL_ISR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: INVLD_CMD *****/
const ru_field_rec HASH_INTR_CTRL_ISR_INVLD_CMD_FIELD =
{
    "INVLD_CMD",
#if RU_INCLUDE_DESC
    "",
    "Command cfg field is invalid (equals to 0)\n",
#endif
    { HASH_INTR_CTRL_ISR_INVLD_CMD_FIELD_MASK },
    0,
    { HASH_INTR_CTRL_ISR_INVLD_CMD_FIELD_WIDTH },
    { HASH_INTR_CTRL_ISR_INVLD_CMD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MULT_MATCH *****/
const ru_field_rec HASH_INTR_CTRL_ISR_MULT_MATCH_FIELD =
{
    "MULT_MATCH",
#if RU_INCLUDE_DESC
    "",
    "During the search process same key was found a valid in multiple engines.\n",
#endif
    { HASH_INTR_CTRL_ISR_MULT_MATCH_FIELD_MASK },
    0,
    { HASH_INTR_CTRL_ISR_MULT_MATCH_FIELD_WIDTH },
    { HASH_INTR_CTRL_ISR_MULT_MATCH_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: HASH_0_IDX_OVFLV *****/
const ru_field_rec HASH_INTR_CTRL_ISR_HASH_0_IDX_OVFLV_FIELD =
{
    "HASH_0_IDX_OVFLV",
#if RU_INCLUDE_DESC
    "",
    "hash table index over flow at hash engine\n",
#endif
    { HASH_INTR_CTRL_ISR_HASH_0_IDX_OVFLV_FIELD_MASK },
    0,
    { HASH_INTR_CTRL_ISR_HASH_0_IDX_OVFLV_FIELD_WIDTH },
    { HASH_INTR_CTRL_ISR_HASH_0_IDX_OVFLV_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: HASH_1_IDX_OVFLV *****/
const ru_field_rec HASH_INTR_CTRL_ISR_HASH_1_IDX_OVFLV_FIELD =
{
    "HASH_1_IDX_OVFLV",
#if RU_INCLUDE_DESC
    "",
    "hash table over flow at hash engine\n",
#endif
    { HASH_INTR_CTRL_ISR_HASH_1_IDX_OVFLV_FIELD_MASK },
    0,
    { HASH_INTR_CTRL_ISR_HASH_1_IDX_OVFLV_FIELD_WIDTH },
    { HASH_INTR_CTRL_ISR_HASH_1_IDX_OVFLV_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: HASH_2_IDX_OVFLV *****/
const ru_field_rec HASH_INTR_CTRL_ISR_HASH_2_IDX_OVFLV_FIELD =
{
    "HASH_2_IDX_OVFLV",
#if RU_INCLUDE_DESC
    "",
    "hash table index over flow at hash engine\n",
#endif
    { HASH_INTR_CTRL_ISR_HASH_2_IDX_OVFLV_FIELD_MASK },
    0,
    { HASH_INTR_CTRL_ISR_HASH_2_IDX_OVFLV_FIELD_WIDTH },
    { HASH_INTR_CTRL_ISR_HASH_2_IDX_OVFLV_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: HASH_3_IDX_OVFLV *****/
const ru_field_rec HASH_INTR_CTRL_ISR_HASH_3_IDX_OVFLV_FIELD =
{
    "HASH_3_IDX_OVFLV",
#if RU_INCLUDE_DESC
    "",
    "hash table index over flow at hash engine\n",
#endif
    { HASH_INTR_CTRL_ISR_HASH_3_IDX_OVFLV_FIELD_MASK },
    0,
    { HASH_INTR_CTRL_ISR_HASH_3_IDX_OVFLV_FIELD_WIDTH },
    { HASH_INTR_CTRL_ISR_HASH_3_IDX_OVFLV_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CNTXT_IDX_OVFLV *****/
const ru_field_rec HASH_INTR_CTRL_ISR_CNTXT_IDX_OVFLV_FIELD =
{
    "CNTXT_IDX_OVFLV",
#if RU_INCLUDE_DESC
    "",
    "Context table index over flow\n",
#endif
    { HASH_INTR_CTRL_ISR_CNTXT_IDX_OVFLV_FIELD_MASK },
    0,
    { HASH_INTR_CTRL_ISR_CNTXT_IDX_OVFLV_FIELD_WIDTH },
    { HASH_INTR_CTRL_ISR_CNTXT_IDX_OVFLV_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

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
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_INTR_CTRL_ISR *****/
const ru_reg_rec HASH_INTR_CTRL_ISR_REG =
{
    "INTR_CTRL_ISR",
#if RU_INCLUDE_DESC
    "INTERRUPT_STATUS_REGISTER Register",
    "This register contains the current active hash interrupts. Each asserted bit represents an active interrupt source. The interrupt remains active until the software clears it by writing 1 to the corresponding bit.\n",
#endif
    { HASH_INTR_CTRL_ISR_REG_OFFSET },
    0,
    0,
    561,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    HASH_INTR_CTRL_ISR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_INTR_CTRL_ISM, TYPE: Type_HASH_BLK_HASH_INTR_CTRL_ISM
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ISM *****/
const ru_field_rec HASH_INTR_CTRL_ISM_ISM_FIELD =
{
    "ISM",
#if RU_INCLUDE_DESC
    "",
    "Status Masked of corresponding interrupt source in the ISR\n",
#endif
    { HASH_INTR_CTRL_ISM_ISM_FIELD_MASK },
    0,
    { HASH_INTR_CTRL_ISM_ISM_FIELD_WIDTH },
    { HASH_INTR_CTRL_ISM_ISM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_INTR_CTRL_ISM_FIELDS[] =
{
    &HASH_INTR_CTRL_ISM_ISM_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_INTR_CTRL_ISM *****/
const ru_reg_rec HASH_INTR_CTRL_ISM_REG =
{
    "INTR_CTRL_ISM",
#if RU_INCLUDE_DESC
    "INTERRUPT_STATUS_MASKED_REGISTER Register",
    "This register provides only the  enabled interrupts for each of the interrupt sources depicted in the ISR register.\n",
#endif
    { HASH_INTR_CTRL_ISM_REG_OFFSET },
    0,
    0,
    562,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_INTR_CTRL_ISM_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_INTR_CTRL_IER, TYPE: Type_HASH_BLK_HASH_INTR_CTRL_IER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: IEM *****/
const ru_field_rec HASH_INTR_CTRL_IER_IEM_FIELD =
{
    "IEM",
#if RU_INCLUDE_DESC
    "",
    "Each bit in the mask controls the corresponding interrupt source in the IER\n",
#endif
    { HASH_INTR_CTRL_IER_IEM_FIELD_MASK },
    0,
    { HASH_INTR_CTRL_IER_IEM_FIELD_WIDTH },
    { HASH_INTR_CTRL_IER_IEM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_INTR_CTRL_IER_FIELDS[] =
{
    &HASH_INTR_CTRL_IER_IEM_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_INTR_CTRL_IER *****/
const ru_reg_rec HASH_INTR_CTRL_IER_REG =
{
    "INTR_CTRL_IER",
#if RU_INCLUDE_DESC
    "INTERRUPT_ENABLE_REGISTER Register",
    "This register provides an enable mask for each of the interrupt sources depicted in the ISR register.\n",
#endif
    { HASH_INTR_CTRL_IER_REG_OFFSET },
    0,
    0,
    563,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_INTR_CTRL_IER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_INTR_CTRL_ITR, TYPE: Type_HASH_BLK_HASH_INTR_CTRL_ITR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: IST *****/
const ru_field_rec HASH_INTR_CTRL_ITR_IST_FIELD =
{
    "IST",
#if RU_INCLUDE_DESC
    "",
    "Each bit in the mask tests the corresponding interrupt source in the ISR\n",
#endif
    { HASH_INTR_CTRL_ITR_IST_FIELD_MASK },
    0,
    { HASH_INTR_CTRL_ITR_IST_FIELD_WIDTH },
    { HASH_INTR_CTRL_ITR_IST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_INTR_CTRL_ITR_FIELDS[] =
{
    &HASH_INTR_CTRL_ITR_IST_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_INTR_CTRL_ITR *****/
const ru_reg_rec HASH_INTR_CTRL_ITR_REG =
{
    "INTR_CTRL_ITR",
#if RU_INCLUDE_DESC
    "INTERRUPT_TEST_REGISTER Register",
    "This register enables testing by simulating interrupt sources. When the software sets a bit in the ITR, the corresponding bit in the ISR shows an active interrupt. The interrupt remains active until software clears the bit in the ITR\n",
#endif
    { HASH_INTR_CTRL_ITR_REG_OFFSET },
    0,
    0,
    564,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_INTR_CTRL_ITR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_DEBUG_DBG0, TYPE: Type_HASH_BLK_HASH_DEBUG_DBG0
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec HASH_DEBUG_DBG0_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "read debug register\n",
#endif
    { HASH_DEBUG_DBG0_VAL_FIELD_MASK },
    0,
    { HASH_DEBUG_DBG0_VAL_FIELD_WIDTH },
    { HASH_DEBUG_DBG0_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_DEBUG_DBG0_FIELDS[] =
{
    &HASH_DEBUG_DBG0_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_DEBUG_DBG0 *****/
const ru_reg_rec HASH_DEBUG_DBG0_REG =
{
    "DEBUG_DBG0",
#if RU_INCLUDE_DESC
    "DBG0 Register",
    "debug reg\n",
#endif
    { HASH_DEBUG_DBG0_REG_OFFSET },
    0,
    0,
    565,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_DEBUG_DBG0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_DEBUG_DBG1, TYPE: Type_HASH_BLK_HASH_DEBUG_DBG1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec HASH_DEBUG_DBG1_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "read debug register\n",
#endif
    { HASH_DEBUG_DBG1_VAL_FIELD_MASK },
    0,
    { HASH_DEBUG_DBG1_VAL_FIELD_WIDTH },
    { HASH_DEBUG_DBG1_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_DEBUG_DBG1_FIELDS[] =
{
    &HASH_DEBUG_DBG1_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_DEBUG_DBG1 *****/
const ru_reg_rec HASH_DEBUG_DBG1_REG =
{
    "DEBUG_DBG1",
#if RU_INCLUDE_DESC
    "DBG1 Register",
    "debug reg\n",
#endif
    { HASH_DEBUG_DBG1_REG_OFFSET },
    0,
    0,
    566,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_DEBUG_DBG1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_DEBUG_DBG2, TYPE: Type_HASH_BLK_HASH_DEBUG_DBG2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec HASH_DEBUG_DBG2_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "read debug register\n",
#endif
    { HASH_DEBUG_DBG2_VAL_FIELD_MASK },
    0,
    { HASH_DEBUG_DBG2_VAL_FIELD_WIDTH },
    { HASH_DEBUG_DBG2_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_DEBUG_DBG2_FIELDS[] =
{
    &HASH_DEBUG_DBG2_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_DEBUG_DBG2 *****/
const ru_reg_rec HASH_DEBUG_DBG2_REG =
{
    "DEBUG_DBG2",
#if RU_INCLUDE_DESC
    "DBG2 Register",
    "debug reg\n",
#endif
    { HASH_DEBUG_DBG2_REG_OFFSET },
    0,
    0,
    567,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_DEBUG_DBG2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_DEBUG_DBG3, TYPE: Type_HASH_BLK_HASH_DEBUG_DBG3
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec HASH_DEBUG_DBG3_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "read debug register\n",
#endif
    { HASH_DEBUG_DBG3_VAL_FIELD_MASK },
    0,
    { HASH_DEBUG_DBG3_VAL_FIELD_WIDTH },
    { HASH_DEBUG_DBG3_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_DEBUG_DBG3_FIELDS[] =
{
    &HASH_DEBUG_DBG3_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_DEBUG_DBG3 *****/
const ru_reg_rec HASH_DEBUG_DBG3_REG =
{
    "DEBUG_DBG3",
#if RU_INCLUDE_DESC
    "DBG3 Register",
    "debug reg\n",
#endif
    { HASH_DEBUG_DBG3_REG_OFFSET },
    0,
    0,
    568,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_DEBUG_DBG3_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_DEBUG_DBG4, TYPE: Type_HASH_BLK_HASH_DEBUG_DBG4
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec HASH_DEBUG_DBG4_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "read debug register\n",
#endif
    { HASH_DEBUG_DBG4_VAL_FIELD_MASK },
    0,
    { HASH_DEBUG_DBG4_VAL_FIELD_WIDTH },
    { HASH_DEBUG_DBG4_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_DEBUG_DBG4_FIELDS[] =
{
    &HASH_DEBUG_DBG4_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_DEBUG_DBG4 *****/
const ru_reg_rec HASH_DEBUG_DBG4_REG =
{
    "DEBUG_DBG4",
#if RU_INCLUDE_DESC
    "DBG4 Register",
    "debug reg\n",
#endif
    { HASH_DEBUG_DBG4_REG_OFFSET },
    0,
    0,
    569,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_DEBUG_DBG4_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_DEBUG_DBG5, TYPE: Type_HASH_BLK_HASH_DEBUG_DBG5
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec HASH_DEBUG_DBG5_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "read debug register\n",
#endif
    { HASH_DEBUG_DBG5_VAL_FIELD_MASK },
    0,
    { HASH_DEBUG_DBG5_VAL_FIELD_WIDTH },
    { HASH_DEBUG_DBG5_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_DEBUG_DBG5_FIELDS[] =
{
    &HASH_DEBUG_DBG5_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_DEBUG_DBG5 *****/
const ru_reg_rec HASH_DEBUG_DBG5_REG =
{
    "DEBUG_DBG5",
#if RU_INCLUDE_DESC
    "DBG5 Register",
    "debug reg\n",
#endif
    { HASH_DEBUG_DBG5_REG_OFFSET },
    0,
    0,
    570,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_DEBUG_DBG5_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_DEBUG_DBG6, TYPE: Type_HASH_BLK_HASH_DEBUG_DBG6
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec HASH_DEBUG_DBG6_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "read debug register\n",
#endif
    { HASH_DEBUG_DBG6_VAL_FIELD_MASK },
    0,
    { HASH_DEBUG_DBG6_VAL_FIELD_WIDTH },
    { HASH_DEBUG_DBG6_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_DEBUG_DBG6_FIELDS[] =
{
    &HASH_DEBUG_DBG6_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_DEBUG_DBG6 *****/
const ru_reg_rec HASH_DEBUG_DBG6_REG =
{
    "DEBUG_DBG6",
#if RU_INCLUDE_DESC
    "DBG6 Register",
    "debug reg\n",
#endif
    { HASH_DEBUG_DBG6_REG_OFFSET },
    0,
    0,
    571,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_DEBUG_DBG6_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_DEBUG_DBG7, TYPE: Type_HASH_BLK_HASH_DEBUG_DBG7
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec HASH_DEBUG_DBG7_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "read debug register\n",
#endif
    { HASH_DEBUG_DBG7_VAL_FIELD_MASK },
    0,
    { HASH_DEBUG_DBG7_VAL_FIELD_WIDTH },
    { HASH_DEBUG_DBG7_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_DEBUG_DBG7_FIELDS[] =
{
    &HASH_DEBUG_DBG7_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_DEBUG_DBG7 *****/
const ru_reg_rec HASH_DEBUG_DBG7_REG =
{
    "DEBUG_DBG7",
#if RU_INCLUDE_DESC
    "DBG7 Register",
    "debug reg\n",
#endif
    { HASH_DEBUG_DBG7_REG_OFFSET },
    0,
    0,
    572,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_DEBUG_DBG7_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_DEBUG_DBG8, TYPE: Type_HASH_BLK_HASH_DEBUG_DBG8
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec HASH_DEBUG_DBG8_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "read debug register\n",
#endif
    { HASH_DEBUG_DBG8_VAL_FIELD_MASK },
    0,
    { HASH_DEBUG_DBG8_VAL_FIELD_WIDTH },
    { HASH_DEBUG_DBG8_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_DEBUG_DBG8_FIELDS[] =
{
    &HASH_DEBUG_DBG8_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_DEBUG_DBG8 *****/
const ru_reg_rec HASH_DEBUG_DBG8_REG =
{
    "DEBUG_DBG8",
#if RU_INCLUDE_DESC
    "DBG8 Register",
    "debug reg\n",
#endif
    { HASH_DEBUG_DBG8_REG_OFFSET },
    0,
    0,
    573,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_DEBUG_DBG8_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_DEBUG_DBG9, TYPE: Type_HASH_BLK_HASH_DEBUG_DBG9
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec HASH_DEBUG_DBG9_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "read debug register\n",
#endif
    { HASH_DEBUG_DBG9_VAL_FIELD_MASK },
    0,
    { HASH_DEBUG_DBG9_VAL_FIELD_WIDTH },
    { HASH_DEBUG_DBG9_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_DEBUG_DBG9_FIELDS[] =
{
    &HASH_DEBUG_DBG9_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_DEBUG_DBG9 *****/
const ru_reg_rec HASH_DEBUG_DBG9_REG =
{
    "DEBUG_DBG9",
#if RU_INCLUDE_DESC
    "DBG9 Register",
    "debug reg\n",
#endif
    { HASH_DEBUG_DBG9_REG_OFFSET },
    0,
    0,
    574,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_DEBUG_DBG9_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_DEBUG_DBG10, TYPE: Type_HASH_BLK_HASH_DEBUG_DBG10
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec HASH_DEBUG_DBG10_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "read debug register\n",
#endif
    { HASH_DEBUG_DBG10_VAL_FIELD_MASK },
    0,
    { HASH_DEBUG_DBG10_VAL_FIELD_WIDTH },
    { HASH_DEBUG_DBG10_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_DEBUG_DBG10_FIELDS[] =
{
    &HASH_DEBUG_DBG10_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_DEBUG_DBG10 *****/
const ru_reg_rec HASH_DEBUG_DBG10_REG =
{
    "DEBUG_DBG10",
#if RU_INCLUDE_DESC
    "DBG10 Register",
    "debug reg\n",
#endif
    { HASH_DEBUG_DBG10_REG_OFFSET },
    0,
    0,
    575,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_DEBUG_DBG10_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_DEBUG_DBG11, TYPE: Type_HASH_BLK_HASH_DEBUG_DBG11
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec HASH_DEBUG_DBG11_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "read debug register\n",
#endif
    { HASH_DEBUG_DBG11_VAL_FIELD_MASK },
    0,
    { HASH_DEBUG_DBG11_VAL_FIELD_WIDTH },
    { HASH_DEBUG_DBG11_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_DEBUG_DBG11_FIELDS[] =
{
    &HASH_DEBUG_DBG11_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_DEBUG_DBG11 *****/
const ru_reg_rec HASH_DEBUG_DBG11_REG =
{
    "DEBUG_DBG11",
#if RU_INCLUDE_DESC
    "DBG11 Register",
    "debug reg\n",
#endif
    { HASH_DEBUG_DBG11_REG_OFFSET },
    0,
    0,
    576,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_DEBUG_DBG11_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_DEBUG_DBG12, TYPE: Type_HASH_BLK_HASH_DEBUG_DBG12
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec HASH_DEBUG_DBG12_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "read debug register\n",
#endif
    { HASH_DEBUG_DBG12_VAL_FIELD_MASK },
    0,
    { HASH_DEBUG_DBG12_VAL_FIELD_WIDTH },
    { HASH_DEBUG_DBG12_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_DEBUG_DBG12_FIELDS[] =
{
    &HASH_DEBUG_DBG12_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_DEBUG_DBG12 *****/
const ru_reg_rec HASH_DEBUG_DBG12_REG =
{
    "DEBUG_DBG12",
#if RU_INCLUDE_DESC
    "DBG12 Register",
    "debug reg\n",
#endif
    { HASH_DEBUG_DBG12_REG_OFFSET },
    0,
    0,
    577,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_DEBUG_DBG12_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_DEBUG_DBG13, TYPE: Type_HASH_BLK_HASH_DEBUG_DBG13
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec HASH_DEBUG_DBG13_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "read debug register\n",
#endif
    { HASH_DEBUG_DBG13_VAL_FIELD_MASK },
    0,
    { HASH_DEBUG_DBG13_VAL_FIELD_WIDTH },
    { HASH_DEBUG_DBG13_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_DEBUG_DBG13_FIELDS[] =
{
    &HASH_DEBUG_DBG13_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_DEBUG_DBG13 *****/
const ru_reg_rec HASH_DEBUG_DBG13_REG =
{
    "DEBUG_DBG13",
#if RU_INCLUDE_DESC
    "DBG13 Register",
    "debug reg\n",
#endif
    { HASH_DEBUG_DBG13_REG_OFFSET },
    0,
    0,
    578,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_DEBUG_DBG13_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_DEBUG_DBG14, TYPE: Type_HASH_BLK_HASH_DEBUG_DBG14
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec HASH_DEBUG_DBG14_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "read debug register\n",
#endif
    { HASH_DEBUG_DBG14_VAL_FIELD_MASK },
    0,
    { HASH_DEBUG_DBG14_VAL_FIELD_WIDTH },
    { HASH_DEBUG_DBG14_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_DEBUG_DBG14_FIELDS[] =
{
    &HASH_DEBUG_DBG14_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_DEBUG_DBG14 *****/
const ru_reg_rec HASH_DEBUG_DBG14_REG =
{
    "DEBUG_DBG14",
#if RU_INCLUDE_DESC
    "DBG14 Register",
    "debug reg\n",
#endif
    { HASH_DEBUG_DBG14_REG_OFFSET },
    0,
    0,
    579,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_DEBUG_DBG14_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_DEBUG_DBG15, TYPE: Type_HASH_BLK_HASH_DEBUG_DBG15
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec HASH_DEBUG_DBG15_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "read debug register\n",
#endif
    { HASH_DEBUG_DBG15_VAL_FIELD_MASK },
    0,
    { HASH_DEBUG_DBG15_VAL_FIELD_WIDTH },
    { HASH_DEBUG_DBG15_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_DEBUG_DBG15_FIELDS[] =
{
    &HASH_DEBUG_DBG15_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_DEBUG_DBG15 *****/
const ru_reg_rec HASH_DEBUG_DBG15_REG =
{
    "DEBUG_DBG15",
#if RU_INCLUDE_DESC
    "DBG15 Register",
    "debug reg\n",
#endif
    { HASH_DEBUG_DBG15_REG_OFFSET },
    0,
    0,
    580,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_DEBUG_DBG15_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_DEBUG_DBG16, TYPE: Type_HASH_BLK_HASH_DEBUG_DBG16
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec HASH_DEBUG_DBG16_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "read debug register\n",
#endif
    { HASH_DEBUG_DBG16_VAL_FIELD_MASK },
    0,
    { HASH_DEBUG_DBG16_VAL_FIELD_WIDTH },
    { HASH_DEBUG_DBG16_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_DEBUG_DBG16_FIELDS[] =
{
    &HASH_DEBUG_DBG16_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_DEBUG_DBG16 *****/
const ru_reg_rec HASH_DEBUG_DBG16_REG =
{
    "DEBUG_DBG16",
#if RU_INCLUDE_DESC
    "DBG16 Register",
    "debug reg\n",
#endif
    { HASH_DEBUG_DBG16_REG_OFFSET },
    0,
    0,
    581,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_DEBUG_DBG16_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_DEBUG_DBG17, TYPE: Type_HASH_BLK_HASH_DEBUG_DBG17
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec HASH_DEBUG_DBG17_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "read debug register\n",
#endif
    { HASH_DEBUG_DBG17_VAL_FIELD_MASK },
    0,
    { HASH_DEBUG_DBG17_VAL_FIELD_WIDTH },
    { HASH_DEBUG_DBG17_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_DEBUG_DBG17_FIELDS[] =
{
    &HASH_DEBUG_DBG17_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_DEBUG_DBG17 *****/
const ru_reg_rec HASH_DEBUG_DBG17_REG =
{
    "DEBUG_DBG17",
#if RU_INCLUDE_DESC
    "DBG17 Register",
    "debug reg\n",
#endif
    { HASH_DEBUG_DBG17_REG_OFFSET },
    0,
    0,
    582,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_DEBUG_DBG17_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_DEBUG_DBG18, TYPE: Type_HASH_BLK_HASH_DEBUG_DBG18
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec HASH_DEBUG_DBG18_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "read debug register\n",
#endif
    { HASH_DEBUG_DBG18_VAL_FIELD_MASK },
    0,
    { HASH_DEBUG_DBG18_VAL_FIELD_WIDTH },
    { HASH_DEBUG_DBG18_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_DEBUG_DBG18_FIELDS[] =
{
    &HASH_DEBUG_DBG18_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_DEBUG_DBG18 *****/
const ru_reg_rec HASH_DEBUG_DBG18_REG =
{
    "DEBUG_DBG18",
#if RU_INCLUDE_DESC
    "DBG18 Register",
    "debug reg\n",
#endif
    { HASH_DEBUG_DBG18_REG_OFFSET },
    0,
    0,
    583,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_DEBUG_DBG18_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_DEBUG_DBG19, TYPE: Type_HASH_BLK_HASH_DEBUG_DBG19
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec HASH_DEBUG_DBG19_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "read debug register\n",
#endif
    { HASH_DEBUG_DBG19_VAL_FIELD_MASK },
    0,
    { HASH_DEBUG_DBG19_VAL_FIELD_WIDTH },
    { HASH_DEBUG_DBG19_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_DEBUG_DBG19_FIELDS[] =
{
    &HASH_DEBUG_DBG19_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_DEBUG_DBG19 *****/
const ru_reg_rec HASH_DEBUG_DBG19_REG =
{
    "DEBUG_DBG19",
#if RU_INCLUDE_DESC
    "DBG19 Register",
    "debug reg\n",
#endif
    { HASH_DEBUG_DBG19_REG_OFFSET },
    0,
    0,
    584,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_DEBUG_DBG19_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_DEBUG_DBG20, TYPE: Type_HASH_BLK_HASH_DEBUG_DBG20
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec HASH_DEBUG_DBG20_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "read debug register\n",
#endif
    { HASH_DEBUG_DBG20_VAL_FIELD_MASK },
    0,
    { HASH_DEBUG_DBG20_VAL_FIELD_WIDTH },
    { HASH_DEBUG_DBG20_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_DEBUG_DBG20_FIELDS[] =
{
    &HASH_DEBUG_DBG20_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_DEBUG_DBG20 *****/
const ru_reg_rec HASH_DEBUG_DBG20_REG =
{
    "DEBUG_DBG20",
#if RU_INCLUDE_DESC
    "DBG20 Register",
    "debug reg\n",
#endif
    { HASH_DEBUG_DBG20_REG_OFFSET },
    0,
    0,
    585,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_DEBUG_DBG20_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_DEBUG_DBG_SEL, TYPE: Type_HASH_BLK_HASH_DEBUG_DBG_SEL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec HASH_DEBUG_DBG_SEL_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "debug sel\n",
#endif
    { HASH_DEBUG_DBG_SEL_VAL_FIELD_MASK },
    0,
    { HASH_DEBUG_DBG_SEL_VAL_FIELD_WIDTH },
    { HASH_DEBUG_DBG_SEL_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_DEBUG_DBG_SEL_FIELDS[] =
{
    &HASH_DEBUG_DBG_SEL_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_DEBUG_DBG_SEL *****/
const ru_reg_rec HASH_DEBUG_DBG_SEL_REG =
{
    "DEBUG_DBG_SEL",
#if RU_INCLUDE_DESC
    "DBG_SELECT Register",
    "debug select mux\n",
#endif
    { HASH_DEBUG_DBG_SEL_REG_OFFSET },
    0,
    0,
    586,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_DEBUG_DBG_SEL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_AGING_RAM_AGING, TYPE: Type_HASH_BLK_HASH_AGING_RAM_AGING
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec HASH_AGING_RAM_AGING_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { HASH_AGING_RAM_AGING_DATA_FIELD_MASK },
    0,
    { HASH_AGING_RAM_AGING_DATA_FIELD_WIDTH },
    { HASH_AGING_RAM_AGING_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_AGING_RAM_AGING_FIELDS[] =
{
    &HASH_AGING_RAM_AGING_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_AGING_RAM_AGING *****/
const ru_reg_rec HASH_AGING_RAM_AGING_REG =
{
    "AGING_RAM_AGING",
#if RU_INCLUDE_DESC
    "AGING 0..129 Register",
    "Each bit in the ram represents hash/CAM entry.\n(6K hash entries + 64 CAM entries)/32= 194 rows\nBit 0 at the ram corresponds to entry 0 (eng0), Bit 1 at the ram corresponds to entry 1 (eng1), and so on..\n",
#endif
    { HASH_AGING_RAM_AGING_REG_OFFSET },
    HASH_AGING_RAM_AGING_REG_RAM_CNT,
    4,
    587,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_AGING_RAM_AGING_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_CONTEXT_RAM_CONTEXT_47_24, TYPE: Type_HASH_BLK_HASH_CONTEXT_RAM_CONTEXT_47_24
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec HASH_CONTEXT_RAM_CONTEXT_47_24_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { HASH_CONTEXT_RAM_CONTEXT_47_24_DATA_FIELD_MASK },
    0,
    { HASH_CONTEXT_RAM_CONTEXT_47_24_DATA_FIELD_WIDTH },
    { HASH_CONTEXT_RAM_CONTEXT_47_24_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CONTEXT_RAM_CONTEXT_47_24_FIELDS[] =
{
    &HASH_CONTEXT_RAM_CONTEXT_47_24_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_CONTEXT_RAM_CONTEXT_47_24 *****/
const ru_reg_rec HASH_CONTEXT_RAM_CONTEXT_47_24_REG =
{
    "CONTEXT_RAM_CONTEXT_47_24",
#if RU_INCLUDE_DESC
    "CONTEXT_47_24 Register",
    "24 most significant bits of an entry (first 3B)\n",
#endif
    { HASH_CONTEXT_RAM_CONTEXT_47_24_REG_OFFSET },
    HASH_CONTEXT_RAM_CONTEXT_47_24_REG_RAM_CNT,
    8,
    588,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_CONTEXT_RAM_CONTEXT_47_24_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_CONTEXT_RAM_CONTEXT_23_0, TYPE: Type_HASH_BLK_HASH_CONTEXT_RAM_CONTEXT_23_0
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec HASH_CONTEXT_RAM_CONTEXT_23_0_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { HASH_CONTEXT_RAM_CONTEXT_23_0_DATA_FIELD_MASK },
    0,
    { HASH_CONTEXT_RAM_CONTEXT_23_0_DATA_FIELD_WIDTH },
    { HASH_CONTEXT_RAM_CONTEXT_23_0_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CONTEXT_RAM_CONTEXT_23_0_FIELDS[] =
{
    &HASH_CONTEXT_RAM_CONTEXT_23_0_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_CONTEXT_RAM_CONTEXT_23_0 *****/
const ru_reg_rec HASH_CONTEXT_RAM_CONTEXT_23_0_REG =
{
    "CONTEXT_RAM_CONTEXT_23_0",
#if RU_INCLUDE_DESC
    "CONTEXT_23_0 Register",
    "24 least significant bits of an entry (second 3B)\n",
#endif
    { HASH_CONTEXT_RAM_CONTEXT_23_0_REG_OFFSET },
    HASH_CONTEXT_RAM_CONTEXT_23_0_REG_RAM_CNT,
    8,
    589,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_CONTEXT_RAM_CONTEXT_23_0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_RAM_ENG_HIGH, TYPE: Type_HASH_BLK_HASH_RAM_ENG_HIGH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: KEY_59_28_OR_DAT *****/
const ru_field_rec HASH_RAM_ENG_HIGH_KEY_59_28_OR_DAT_FIELD =
{
    "KEY_59_28_OR_DAT",
#if RU_INCLUDE_DESC
    "",
    "This field contains one of the two: key extension or internal context data.\nIt defined by table configuration.\n",
#endif
    { HASH_RAM_ENG_HIGH_KEY_59_28_OR_DAT_FIELD_MASK },
    0,
    { HASH_RAM_ENG_HIGH_KEY_59_28_OR_DAT_FIELD_WIDTH },
    { HASH_RAM_ENG_HIGH_KEY_59_28_OR_DAT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_RAM_ENG_HIGH_FIELDS[] =
{
    &HASH_RAM_ENG_HIGH_KEY_59_28_OR_DAT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_RAM_ENG_HIGH *****/
const ru_reg_rec HASH_RAM_ENG_HIGH_REG =
{
    "RAM_ENG_HIGH",
#if RU_INCLUDE_DESC
    "ENG_BITS_63_32 Register",
    "Includes the MSB field of the hash entry\n",
#endif
    { HASH_RAM_ENG_HIGH_REG_OFFSET },
    HASH_RAM_ENG_HIGH_REG_RAM_CNT,
    8,
    590,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_RAM_ENG_HIGH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_RAM_ENG_LOW, TYPE: Type_HASH_BLK_HASH_RAM_ENG_LOW
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SKP *****/
const ru_field_rec HASH_RAM_ENG_LOW_SKP_FIELD =
{
    "SKP",
#if RU_INCLUDE_DESC
    "",
    "Indicates not to search at this entry due to the ongoing update of the entry.\n",
#endif
    { HASH_RAM_ENG_LOW_SKP_FIELD_MASK },
    0,
    { HASH_RAM_ENG_LOW_SKP_FIELD_WIDTH },
    { HASH_RAM_ENG_LOW_SKP_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CFG *****/
const ru_field_rec HASH_RAM_ENG_LOW_CFG_FIELD =
{
    "CFG",
#if RU_INCLUDE_DESC
    "",
    "Determines the table config number, between 1-7.\n\nConfig 0 is used to indicate invalid entry\n",
#endif
    { HASH_RAM_ENG_LOW_CFG_FIELD_MASK },
    0,
    { HASH_RAM_ENG_LOW_CFG_FIELD_WIDTH },
    { HASH_RAM_ENG_LOW_CFG_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEY_11_0 *****/
const ru_field_rec HASH_RAM_ENG_LOW_KEY_11_0_FIELD =
{
    "KEY_11_0",
#if RU_INCLUDE_DESC
    "",
    "Includes the first part of the key.\nThis field is preserved for key use only.\n",
#endif
    { HASH_RAM_ENG_LOW_KEY_11_0_FIELD_MASK },
    0,
    { HASH_RAM_ENG_LOW_KEY_11_0_FIELD_WIDTH },
    { HASH_RAM_ENG_LOW_KEY_11_0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEY_27_12_OR_DAT *****/
const ru_field_rec HASH_RAM_ENG_LOW_KEY_27_12_OR_DAT_FIELD =
{
    "KEY_27_12_OR_DAT",
#if RU_INCLUDE_DESC
    "",
    "This field contains one of the two: key extension or internal context data.\nIt defined by table configuration.\n",
#endif
    { HASH_RAM_ENG_LOW_KEY_27_12_OR_DAT_FIELD_MASK },
    0,
    { HASH_RAM_ENG_LOW_KEY_27_12_OR_DAT_FIELD_WIDTH },
    { HASH_RAM_ENG_LOW_KEY_27_12_OR_DAT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_RAM_ENG_LOW_FIELDS[] =
{
    &HASH_RAM_ENG_LOW_SKP_FIELD,
    &HASH_RAM_ENG_LOW_CFG_FIELD,
    &HASH_RAM_ENG_LOW_KEY_11_0_FIELD,
    &HASH_RAM_ENG_LOW_KEY_27_12_OR_DAT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_RAM_ENG_LOW *****/
const ru_reg_rec HASH_RAM_ENG_LOW_REG =
{
    "RAM_ENG_LOW",
#if RU_INCLUDE_DESC
    "ENG_BITS_31_0 Register",
    "Includes the MSB field of the hash entry\n",
#endif
    { HASH_RAM_ENG_LOW_REG_OFFSET },
    HASH_RAM_ENG_LOW_REG_RAM_CNT,
    8,
    591,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    HASH_RAM_ENG_LOW_FIELDS,
#endif
};

unsigned long HASH_ADDRS[] =
{
    0x82920000,
};

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
    &HASH_CAM_CONFIGURATION_TM_CFG_REG,
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

const ru_block_rec HASH_BLOCK =
{
    "HASH",
    HASH_ADDRS,
    1,
    71,
    HASH_REGS,
};
