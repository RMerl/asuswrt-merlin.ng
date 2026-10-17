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
 * Register: NAME: HASH_GENERAL_CONFIGURATION_GEN_CFG, TYPE: Type_HASH_BLK_HASH_GENERAL_CONFIGURATION_GEN_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BLOCK_ENABLE *****/
const ru_field_rec HASH_GENERAL_CONFIGURATION_GEN_CFG_BLOCK_ENABLE_FIELD =
{
    "BLOCK_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "block enable\n",
#endif
    { HASH_GENERAL_CONFIGURATION_GEN_CFG_BLOCK_ENABLE_FIELD_MASK },
    0,
    { HASH_GENERAL_CONFIGURATION_GEN_CFG_BLOCK_ENABLE_FIELD_WIDTH },
    { HASH_GENERAL_CONFIGURATION_GEN_CFG_BLOCK_ENABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: LKUP_ENG_SEARCH_DISABLE *****/
const ru_field_rec HASH_GENERAL_CONFIGURATION_GEN_CFG_LKUP_ENG_SEARCH_DISABLE_FIELD =
{
    "LKUP_ENG_SEARCH_DISABLE",
#if RU_INCLUDE_DESC
    "",
    "Disable lkup engine search - to save power\n",
#endif
    { HASH_GENERAL_CONFIGURATION_GEN_CFG_LKUP_ENG_SEARCH_DISABLE_FIELD_MASK },
    0,
    { HASH_GENERAL_CONFIGURATION_GEN_CFG_LKUP_ENG_SEARCH_DISABLE_FIELD_WIDTH },
    { HASH_GENERAL_CONFIGURATION_GEN_CFG_LKUP_ENG_SEARCH_DISABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: STORM_MOVE_DIS *****/
const ru_field_rec HASH_GENERAL_CONFIGURATION_GEN_CFG_STORM_MOVE_DIS_FIELD =
{
    "STORM_MOVE_DIS",
#if RU_INCLUDE_DESC
    "",
    "Disable move in Storm - move even if enabled will not effect storm logic\n",
#endif
    { HASH_GENERAL_CONFIGURATION_GEN_CFG_STORM_MOVE_DIS_FIELD_MASK },
    0,
    { HASH_GENERAL_CONFIGURATION_GEN_CFG_STORM_MOVE_DIS_FIELD_WIDTH },
    { HASH_GENERAL_CONFIGURATION_GEN_CFG_STORM_MOVE_DIS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_GENERAL_CONFIGURATION_GEN_CFG_FIELDS[] =
{
    &HASH_GENERAL_CONFIGURATION_GEN_CFG_BLOCK_ENABLE_FIELD,
    &HASH_GENERAL_CONFIGURATION_GEN_CFG_LKUP_ENG_SEARCH_DISABLE_FIELD,
    &HASH_GENERAL_CONFIGURATION_GEN_CFG_STORM_MOVE_DIS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_GENERAL_CONFIGURATION_GEN_CFG *****/
const ru_reg_rec HASH_GENERAL_CONFIGURATION_GEN_CFG_REG =
{
    "GENERAL_CONFIGURATION_GEN_CFG",
#if RU_INCLUDE_DESC
    "GENERAL_CFG Register",
    "General register with block enable\n",
#endif
    { HASH_GENERAL_CONFIGURATION_GEN_CFG_REG_OFFSET },
    0,
    0,
    1531,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    HASH_GENERAL_CONFIGURATION_GEN_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_GENERAL_CONFIGURATION_FIFO_THRESHOLD, TYPE: Type_HASH_BLK_HASH_GENERAL_CONFIGURATION_FIFO_THRESHOLD
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: HASH_TABLE_THRESHOLD *****/
const ru_field_rec HASH_GENERAL_CONFIGURATION_FIFO_THRESHOLD_HASH_TABLE_THRESHOLD_FIELD =
{
    "HASH_TABLE_THRESHOLD",
#if RU_INCLUDE_DESC
    "",
    "hash table FIFO thresh hold\n",
#endif
    { HASH_GENERAL_CONFIGURATION_FIFO_THRESHOLD_HASH_TABLE_THRESHOLD_FIELD_MASK },
    0,
    { HASH_GENERAL_CONFIGURATION_FIFO_THRESHOLD_HASH_TABLE_THRESHOLD_FIELD_WIDTH },
    { HASH_GENERAL_CONFIGURATION_FIFO_THRESHOLD_HASH_TABLE_THRESHOLD_FIELD_SHIFT },
    5,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CAM_THRESHOLD *****/
const ru_field_rec HASH_GENERAL_CONFIGURATION_FIFO_THRESHOLD_CAM_THRESHOLD_FIELD =
{
    "CAM_THRESHOLD",
#if RU_INCLUDE_DESC
    "",
    "CAM FIFO thresh hold\n",
#endif
    { HASH_GENERAL_CONFIGURATION_FIFO_THRESHOLD_CAM_THRESHOLD_FIELD_MASK },
    0,
    { HASH_GENERAL_CONFIGURATION_FIFO_THRESHOLD_CAM_THRESHOLD_FIELD_WIDTH },
    { HASH_GENERAL_CONFIGURATION_FIFO_THRESHOLD_CAM_THRESHOLD_FIELD_SHIFT },
    5,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CNTXT_IN_THRESHOLD *****/
const ru_field_rec HASH_GENERAL_CONFIGURATION_FIFO_THRESHOLD_CNTXT_IN_THRESHOLD_FIELD =
{
    "CNTXT_IN_THRESHOLD",
#if RU_INCLUDE_DESC
    "",
    "CNTXT IN FIFO thresh hold\n",
#endif
    { HASH_GENERAL_CONFIGURATION_FIFO_THRESHOLD_CNTXT_IN_THRESHOLD_FIELD_MASK },
    0,
    { HASH_GENERAL_CONFIGURATION_FIFO_THRESHOLD_CNTXT_IN_THRESHOLD_FIELD_WIDTH },
    { HASH_GENERAL_CONFIGURATION_FIFO_THRESHOLD_CNTXT_IN_THRESHOLD_FIELD_SHIFT },
    5,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: STORM_THRESHOLD *****/
const ru_field_rec HASH_GENERAL_CONFIGURATION_FIFO_THRESHOLD_STORM_THRESHOLD_FIELD =
{
    "STORM_THRESHOLD",
#if RU_INCLUDE_DESC
    "",
    "STORM FIFO thresh hold\n",
#endif
    { HASH_GENERAL_CONFIGURATION_FIFO_THRESHOLD_STORM_THRESHOLD_FIELD_MASK },
    0,
    { HASH_GENERAL_CONFIGURATION_FIFO_THRESHOLD_STORM_THRESHOLD_FIELD_WIDTH },
    { HASH_GENERAL_CONFIGURATION_FIFO_THRESHOLD_STORM_THRESHOLD_FIELD_SHIFT },
    27,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: STORM_THRESHOLD_ENABLE *****/
const ru_field_rec HASH_GENERAL_CONFIGURATION_FIFO_THRESHOLD_STORM_THRESHOLD_ENABLE_FIELD =
{
    "STORM_THRESHOLD_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "STORM FIFO thresh enable\n",
#endif
    { HASH_GENERAL_CONFIGURATION_FIFO_THRESHOLD_STORM_THRESHOLD_ENABLE_FIELD_MASK },
    0,
    { HASH_GENERAL_CONFIGURATION_FIFO_THRESHOLD_STORM_THRESHOLD_ENABLE_FIELD_WIDTH },
    { HASH_GENERAL_CONFIGURATION_FIFO_THRESHOLD_STORM_THRESHOLD_ENABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_GENERAL_CONFIGURATION_FIFO_THRESHOLD_FIELDS[] =
{
    &HASH_GENERAL_CONFIGURATION_FIFO_THRESHOLD_HASH_TABLE_THRESHOLD_FIELD,
    &HASH_GENERAL_CONFIGURATION_FIFO_THRESHOLD_CAM_THRESHOLD_FIELD,
    &HASH_GENERAL_CONFIGURATION_FIFO_THRESHOLD_CNTXT_IN_THRESHOLD_FIELD,
    &HASH_GENERAL_CONFIGURATION_FIFO_THRESHOLD_STORM_THRESHOLD_FIELD,
    &HASH_GENERAL_CONFIGURATION_FIFO_THRESHOLD_STORM_THRESHOLD_ENABLE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_GENERAL_CONFIGURATION_FIFO_THRESHOLD *****/
const ru_reg_rec HASH_GENERAL_CONFIGURATION_FIFO_THRESHOLD_REG =
{
    "GENERAL_CONFIGURATION_FIFO_THRESHOLD",
#if RU_INCLUDE_DESC
    "FIFO_THRESHOLD Register",
    ".\n",
#endif
    { HASH_GENERAL_CONFIGURATION_FIFO_THRESHOLD_REG_OFFSET },
    0,
    0,
    1532,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    HASH_GENERAL_CONFIGURATION_FIFO_THRESHOLD_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_GENERAL_CONFIGURATION_MULT_HIT_ERR, TYPE: Type_HASH_BLK_HASH_GENERAL_CONFIGURATION_MULT_HIT_ERR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ENGINE_VAL *****/
const ru_field_rec HASH_GENERAL_CONFIGURATION_MULT_HIT_ERR_ENGINE_VAL_FIELD =
{
    "ENGINE_VAL",
#if RU_INCLUDE_DESC
    "",
    "In case of multiple match this reg captures the hit indication per engine. This is a read clear reg.\n",
#endif
    { HASH_GENERAL_CONFIGURATION_MULT_HIT_ERR_ENGINE_VAL_FIELD_MASK },
    0,
    { HASH_GENERAL_CONFIGURATION_MULT_HIT_ERR_ENGINE_VAL_FIELD_WIDTH },
    { HASH_GENERAL_CONFIGURATION_MULT_HIT_ERR_ENGINE_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: KEY_VAL *****/
const ru_field_rec HASH_GENERAL_CONFIGURATION_MULT_HIT_ERR_KEY_VAL_FIELD =
{
    "KEY_VAL",
#if RU_INCLUDE_DESC
    "",
    "In case of multiple match this reg captures the 17 key lower bits, 16:0.\nThis is a read clear reg.\n",
#endif
    { HASH_GENERAL_CONFIGURATION_MULT_HIT_ERR_KEY_VAL_FIELD_MASK },
    0,
    { HASH_GENERAL_CONFIGURATION_MULT_HIT_ERR_KEY_VAL_FIELD_WIDTH },
    { HASH_GENERAL_CONFIGURATION_MULT_HIT_ERR_KEY_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_GENERAL_CONFIGURATION_MULT_HIT_ERR_FIELDS[] =
{
    &HASH_GENERAL_CONFIGURATION_MULT_HIT_ERR_ENGINE_VAL_FIELD,
    &HASH_GENERAL_CONFIGURATION_MULT_HIT_ERR_KEY_VAL_FIELD,
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
    1533,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    HASH_GENERAL_CONFIGURATION_MULT_HIT_ERR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_GENERAL_CONFIGURATION_AUTO_INIT_EN, TYPE: Type_HASH_BLK_HASH_GENERAL_CONFIGURATION_AUTO_INIT_EN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: AUTO_INIT_EN *****/
const ru_field_rec HASH_GENERAL_CONFIGURATION_AUTO_INIT_EN_AUTO_INIT_EN_FIELD =
{
    "AUTO_INIT_EN",
#if RU_INCLUDE_DESC
    "",
    "Enable auto init of memories:\nhash table, aging(MAC)\nOnce set it will init the BD Ram memories. It will clear when finished\n\n",
#endif
    { HASH_GENERAL_CONFIGURATION_AUTO_INIT_EN_AUTO_INIT_EN_FIELD_MASK },
    0,
    { HASH_GENERAL_CONFIGURATION_AUTO_INIT_EN_AUTO_INIT_EN_FIELD_WIDTH },
    { HASH_GENERAL_CONFIGURATION_AUTO_INIT_EN_AUTO_INIT_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_GENERAL_CONFIGURATION_AUTO_INIT_EN_FIELDS[] =
{
    &HASH_GENERAL_CONFIGURATION_AUTO_INIT_EN_AUTO_INIT_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_GENERAL_CONFIGURATION_AUTO_INIT_EN *****/
const ru_reg_rec HASH_GENERAL_CONFIGURATION_AUTO_INIT_EN_REG =
{
    "GENERAL_CONFIGURATION_AUTO_INIT_EN",
#if RU_INCLUDE_DESC
    "AUTO_INIT_ENABLE Register",
    "Enable auto init of several memories inside the HASH, hash table, aging(MAC), storm (MAC).\nOnce set it will init the BD Ram memories. It will clear when finished\n",
#endif
    { HASH_GENERAL_CONFIGURATION_AUTO_INIT_EN_REG_OFFSET },
    0,
    0,
    1534,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_GENERAL_CONFIGURATION_AUTO_INIT_EN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_GENERAL_CONFIGURATION_STORM_AUTO_INIT_EN, TYPE: Type_HASH_BLK_HASH_GENERAL_CONFIGURATION_STORM_AUTO_INIT_EN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: AUTO_INIT_EN *****/
const ru_field_rec HASH_GENERAL_CONFIGURATION_STORM_AUTO_INIT_EN_AUTO_INIT_EN_FIELD =
{
    "AUTO_INIT_EN",
#if RU_INCLUDE_DESC
    "",
    "Enable auto init of memory  storm (MAC).\nOnce set it will init the BD Ram memories. It will clear when finished\n\n",
#endif
    { HASH_GENERAL_CONFIGURATION_STORM_AUTO_INIT_EN_AUTO_INIT_EN_FIELD_MASK },
    0,
    { HASH_GENERAL_CONFIGURATION_STORM_AUTO_INIT_EN_AUTO_INIT_EN_FIELD_WIDTH },
    { HASH_GENERAL_CONFIGURATION_STORM_AUTO_INIT_EN_AUTO_INIT_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_GENERAL_CONFIGURATION_STORM_AUTO_INIT_EN_FIELDS[] =
{
    &HASH_GENERAL_CONFIGURATION_STORM_AUTO_INIT_EN_AUTO_INIT_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_GENERAL_CONFIGURATION_STORM_AUTO_INIT_EN *****/
const ru_reg_rec HASH_GENERAL_CONFIGURATION_STORM_AUTO_INIT_EN_REG =
{
    "GENERAL_CONFIGURATION_STORM_AUTO_INIT_EN",
#if RU_INCLUDE_DESC
    "STORM_AUTO_INIT_ENABLE Register",
    "Enable auto init of storm memory inside MAC only.\nOnce set it will init the BD Ram memories. It will clear when finished\n",
#endif
    { HASH_GENERAL_CONFIGURATION_STORM_AUTO_INIT_EN_REG_OFFSET },
    0,
    0,
    1535,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_GENERAL_CONFIGURATION_STORM_AUTO_INIT_EN_FIELDS,
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
    1536,
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
    1537,
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
    1538,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_PM_COUNTERS_MISS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_PM_COUNTERS_STORM_WINDOW_SIZE, TYPE: Type_HASH_BLK_HASH_PM_COUNTERS_STORM_WINDOW_SIZE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CNT *****/
const ru_field_rec HASH_PM_COUNTERS_STORM_WINDOW_SIZE_CNT_FIELD =
{
    "CNT",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { HASH_PM_COUNTERS_STORM_WINDOW_SIZE_CNT_FIELD_MASK },
    0,
    { HASH_PM_COUNTERS_STORM_WINDOW_SIZE_CNT_FIELD_WIDTH },
    { HASH_PM_COUNTERS_STORM_WINDOW_SIZE_CNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_PM_COUNTERS_STORM_WINDOW_SIZE_FIELDS[] =
{
    &HASH_PM_COUNTERS_STORM_WINDOW_SIZE_CNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_PM_COUNTERS_STORM_WINDOW_SIZE *****/
const ru_reg_rec HASH_PM_COUNTERS_STORM_WINDOW_SIZE_REG =
{
    "PM_COUNTERS_STORM_WINDOW_SIZE",
#if RU_INCLUDE_DESC
    "STORM_WINDOW_SIZE Register",
    "The STORM_WINDOW is defined as a counter counting down from STORM_WINDOW_SIZE to 0.\n",
#endif
    { HASH_PM_COUNTERS_STORM_WINDOW_SIZE_REG_OFFSET },
    0,
    0,
    1539,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_PM_COUNTERS_STORM_WINDOW_SIZE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_PM_COUNTERS_STORM_CFG, TYPE: Type_HASH_BLK_HASH_PM_COUNTERS_STORM_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: STORM_TH *****/
const ru_field_rec HASH_PM_COUNTERS_STORM_CFG_STORM_TH_FIELD =
{
    "STORM_TH",
#if RU_INCLUDE_DESC
    "",
    "storm trheshold\n",
#endif
    { HASH_PM_COUNTERS_STORM_CFG_STORM_TH_FIELD_MASK },
    0,
    { HASH_PM_COUNTERS_STORM_CFG_STORM_TH_FIELD_WIDTH },
    { HASH_PM_COUNTERS_STORM_CFG_STORM_TH_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: STORM_WINDOW_ENABLE *****/
const ru_field_rec HASH_PM_COUNTERS_STORM_CFG_STORM_WINDOW_ENABLE_FIELD =
{
    "STORM_WINDOW_ENABLE",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { HASH_PM_COUNTERS_STORM_CFG_STORM_WINDOW_ENABLE_FIELD_MASK },
    0,
    { HASH_PM_COUNTERS_STORM_CFG_STORM_WINDOW_ENABLE_FIELD_WIDTH },
    { HASH_PM_COUNTERS_STORM_CFG_STORM_WINDOW_ENABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_PM_COUNTERS_STORM_CFG_FIELDS[] =
{
    &HASH_PM_COUNTERS_STORM_CFG_STORM_TH_FIELD,
    &HASH_PM_COUNTERS_STORM_CFG_STORM_WINDOW_ENABLE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_PM_COUNTERS_STORM_CFG *****/
const ru_reg_rec HASH_PM_COUNTERS_STORM_CFG_REG =
{
    "PM_COUNTERS_STORM_CFG",
#if RU_INCLUDE_DESC
    "STORM_CFG Register",
    "if enabled and If MISS_CNT > STORM_TH then the storm protection becomes active.\n\n",
#endif
    { HASH_PM_COUNTERS_STORM_CFG_REG_OFFSET },
    0,
    0,
    1540,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    HASH_PM_COUNTERS_STORM_CFG_FIELDS,
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
    1541,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_PM_COUNTERS_FRZ_CNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_LKUP_TBL_CFG_CNTXT_CFG, TYPE: Type_HASH_BLK_HASH_LKUP_TBL_CFG_CNTXT_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DIRECT_READ_EN *****/
const ru_field_rec HASH_LKUP_TBL_CFG_CNTXT_CFG_DIRECT_READ_EN_FIELD =
{
    "DIRECT_READ_EN",
#if RU_INCLUDE_DESC
    "",
    "direct read enable\n",
#endif
    { HASH_LKUP_TBL_CFG_CNTXT_CFG_DIRECT_READ_EN_FIELD_MASK },
    0,
    { HASH_LKUP_TBL_CFG_CNTXT_CFG_DIRECT_READ_EN_FIELD_WIDTH },
    { HASH_LKUP_TBL_CFG_CNTXT_CFG_DIRECT_READ_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CAM_EN *****/
const ru_field_rec HASH_LKUP_TBL_CFG_CNTXT_CFG_CAM_EN_FIELD =
{
    "CAM_EN",
#if RU_INCLUDE_DESC
    "",
    "CAM enable in case of miss\n",
#endif
    { HASH_LKUP_TBL_CFG_CNTXT_CFG_CAM_EN_FIELD_MASK },
    0,
    { HASH_LKUP_TBL_CFG_CNTXT_CFG_CAM_EN_FIELD_WIDTH },
    { HASH_LKUP_TBL_CFG_CNTXT_CFG_CAM_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MOVE_EN *****/
const ru_field_rec HASH_LKUP_TBL_CFG_CNTXT_CFG_MOVE_EN_FIELD =
{
    "MOVE_EN",
#if RU_INCLUDE_DESC
    "",
    "enable move enable in MAC table\n",
#endif
    { HASH_LKUP_TBL_CFG_CNTXT_CFG_MOVE_EN_FIELD_MASK },
    0,
    { HASH_LKUP_TBL_CFG_CNTXT_CFG_MOVE_EN_FIELD_WIDTH },
    { HASH_LKUP_TBL_CFG_CNTXT_CFG_MOVE_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: STORM_EN *****/
const ru_field_rec HASH_LKUP_TBL_CFG_CNTXT_CFG_STORM_EN_FIELD =
{
    "STORM_EN",
#if RU_INCLUDE_DESC
    "",
    "in MAC mode only (SA)\nstorm enable\n",
#endif
    { HASH_LKUP_TBL_CFG_CNTXT_CFG_STORM_EN_FIELD_MASK },
    0,
    { HASH_LKUP_TBL_CFG_CNTXT_CFG_STORM_EN_FIELD_WIDTH },
    { HASH_LKUP_TBL_CFG_CNTXT_CFG_STORM_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: AGING_EN *****/
const ru_field_rec HASH_LKUP_TBL_CFG_CNTXT_CFG_AGING_EN_FIELD =
{
    "AGING_EN",
#if RU_INCLUDE_DESC
    "",
    "in MAC mode only (SA)\naging   enable\n",
#endif
    { HASH_LKUP_TBL_CFG_CNTXT_CFG_AGING_EN_FIELD_MASK },
    0,
    { HASH_LKUP_TBL_CFG_CNTXT_CFG_AGING_EN_FIELD_WIDTH },
    { HASH_LKUP_TBL_CFG_CNTXT_CFG_AGING_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RSLT128_EN *****/
const ru_field_rec HASH_LKUP_TBL_CFG_CNTXT_CFG_RSLT128_EN_FIELD =
{
    "RSLT128_EN",
#if RU_INCLUDE_DESC
    "",
    "enable 128 bit only result on BroadBus\nin case we want better performance\n\n",
#endif
    { HASH_LKUP_TBL_CFG_CNTXT_CFG_RSLT128_EN_FIELD_MASK },
    0,
    { HASH_LKUP_TBL_CFG_CNTXT_CFG_RSLT128_EN_FIELD_WIDTH },
    { HASH_LKUP_TBL_CFG_CNTXT_CFG_RSLT128_EN_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MOVE_MASK *****/
const ru_field_rec HASH_LKUP_TBL_CFG_CNTXT_CFG_MOVE_MASK_FIELD =
{
    "MOVE_MASK",
#if RU_INCLUDE_DESC
    "",
    "in MAC mode, configure move bit mask\n\n\n",
#endif
    { HASH_LKUP_TBL_CFG_CNTXT_CFG_MOVE_MASK_FIELD_MASK },
    0,
    { HASH_LKUP_TBL_CFG_CNTXT_CFG_MOVE_MASK_FIELD_WIDTH },
    { HASH_LKUP_TBL_CFG_CNTXT_CFG_MOVE_MASK_FIELD_SHIFT },
    349525,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_LKUP_TBL_CFG_CNTXT_CFG_FIELDS[] =
{
    &HASH_LKUP_TBL_CFG_CNTXT_CFG_DIRECT_READ_EN_FIELD,
    &HASH_LKUP_TBL_CFG_CNTXT_CFG_CAM_EN_FIELD,
    &HASH_LKUP_TBL_CFG_CNTXT_CFG_MOVE_EN_FIELD,
    &HASH_LKUP_TBL_CFG_CNTXT_CFG_STORM_EN_FIELD,
    &HASH_LKUP_TBL_CFG_CNTXT_CFG_AGING_EN_FIELD,
    &HASH_LKUP_TBL_CFG_CNTXT_CFG_RSLT128_EN_FIELD,
    &HASH_LKUP_TBL_CFG_CNTXT_CFG_MOVE_MASK_FIELD,
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
    20,
    1542,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    HASH_LKUP_TBL_CFG_CNTXT_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_LKUP_TBL_CFG_KEY_MASK_HIGHEST, TYPE: Type_HASH_BLK_HASH_LKUP_TBL_CFG_KEY_MASK_HIGHEST
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MASKH *****/
const ru_field_rec HASH_LKUP_TBL_CFG_KEY_MASK_HIGHEST_MASKH_FIELD =
{
    "MASKH",
#if RU_INCLUDE_DESC
    "",
    "MASK High applied on the 32 msb of the current part of key for the current search table.\n\n",
#endif
    { HASH_LKUP_TBL_CFG_KEY_MASK_HIGHEST_MASKH_FIELD_MASK },
    0,
    { HASH_LKUP_TBL_CFG_KEY_MASK_HIGHEST_MASKH_FIELD_WIDTH },
    { HASH_LKUP_TBL_CFG_KEY_MASK_HIGHEST_MASKH_FIELD_SHIFT },
    4294967295,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_LKUP_TBL_CFG_KEY_MASK_HIGHEST_FIELDS[] =
{
    &HASH_LKUP_TBL_CFG_KEY_MASK_HIGHEST_MASKH_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_LKUP_TBL_CFG_KEY_MASK_HIGHEST *****/
const ru_reg_rec HASH_LKUP_TBL_CFG_KEY_MASK_HIGHEST_REG =
{
    "LKUP_TBL_CFG_KEY_MASK_HIGHEST",
#if RU_INCLUDE_DESC
    "KEY_MASK_HIGHEST Register",
    "Look-up table : key Mask on bits\nIPTV KEY:          [127:96]\nFCL, ICL, MAC KEY: [63:32]\nRDWD:              [59:28]\n\n",
#endif
    { HASH_LKUP_TBL_CFG_KEY_MASK_HIGHEST_REG_OFFSET },
    HASH_LKUP_TBL_CFG_KEY_MASK_HIGHEST_REG_RAM_CNT,
    20,
    1543,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_LKUP_TBL_CFG_KEY_MASK_HIGHEST_FIELDS,
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
    "MASK High applied on the 32 msb of the current part of key for the current search table.\n\n",
#endif
    { HASH_LKUP_TBL_CFG_KEY_MASK_HIGH_MASKH_FIELD_MASK },
    0,
    { HASH_LKUP_TBL_CFG_KEY_MASK_HIGH_MASKH_FIELD_WIDTH },
    { HASH_LKUP_TBL_CFG_KEY_MASK_HIGH_MASKH_FIELD_SHIFT },
    4294967295,
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
    "Look-up table : key Mask on bits\nIPTV KEY:          [95:64]\nFCL, ICL, MAC KEY: [31:0]\nRDWD:              [27:0]\n\n",
#endif
    { HASH_LKUP_TBL_CFG_KEY_MASK_HIGH_REG_OFFSET },
    HASH_LKUP_TBL_CFG_KEY_MASK_HIGH_REG_RAM_CNT,
    20,
    1544,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_LKUP_TBL_CFG_KEY_MASK_HIGH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_LKUP_TBL_CFG_KEY_MASK_MID, TYPE: Type_HASH_BLK_HASH_LKUP_TBL_CFG_KEY_MASK_MID
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MASKM *****/
const ru_field_rec HASH_LKUP_TBL_CFG_KEY_MASK_MID_MASKM_FIELD =
{
    "MASKM",
#if RU_INCLUDE_DESC
    "",
    "MASK MID applied on the 32 mid of the current part of key for the current search table.\n\n\n",
#endif
    { HASH_LKUP_TBL_CFG_KEY_MASK_MID_MASKM_FIELD_MASK },
    0,
    { HASH_LKUP_TBL_CFG_KEY_MASK_MID_MASKM_FIELD_WIDTH },
    { HASH_LKUP_TBL_CFG_KEY_MASK_MID_MASKM_FIELD_SHIFT },
    4294967295,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_LKUP_TBL_CFG_KEY_MASK_MID_FIELDS[] =
{
    &HASH_LKUP_TBL_CFG_KEY_MASK_MID_MASKM_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_LKUP_TBL_CFG_KEY_MASK_MID *****/
const ru_reg_rec HASH_LKUP_TBL_CFG_KEY_MASK_MID_REG =
{
    "LKUP_TBL_CFG_KEY_MASK_MID",
#if RU_INCLUDE_DESC
    "KEY_MASK_MID Register",
    "Look-up table key Mask on bits\n\nIPTV KEY: [63:32]\n\n\n\n",
#endif
    { HASH_LKUP_TBL_CFG_KEY_MASK_MID_REG_OFFSET },
    HASH_LKUP_TBL_CFG_KEY_MASK_MID_REG_RAM_CNT,
    20,
    1545,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_LKUP_TBL_CFG_KEY_MASK_MID_FIELDS,
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
    "MASK Low applied on the 32 lsb of the current part of key for the current search table. + CFG [2:0] mask\n\n\n",
#endif
    { HASH_LKUP_TBL_CFG_KEY_MASK_LOW_MASKL_FIELD_MASK },
    0,
    { HASH_LKUP_TBL_CFG_KEY_MASK_LOW_MASKL_FIELD_WIDTH },
    { HASH_LKUP_TBL_CFG_KEY_MASK_LOW_MASKL_FIELD_SHIFT },
    4294967295,
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
    "Look-up table key Mask on bits\n\nBits:\nIPTV key mask [31:3]\nFCL, ICL, MAC, IPTV and RDWD TBL CFG mask\n[2:0]\n\n\n\n\n\n",
#endif
    { HASH_LKUP_TBL_CFG_KEY_MASK_LOW_REG_OFFSET },
    HASH_LKUP_TBL_CFG_KEY_MASK_LOW_REG_RAM_CNT,
    20,
    1546,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_LKUP_TBL_CFG_KEY_MASK_LOW_FIELDS,
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
    1547,
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
    1548,
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
    ".\nCHANGED_BY_HW\n\n",
#endif
    { HASH_CAM_INDIRECT_OP_DONE_VAL_FIELD_MASK },
    0,
    { HASH_CAM_INDIRECT_OP_DONE_VAL_FIELD_WIDTH },
    { HASH_CAM_INDIRECT_OP_DONE_VAL_FIELD_SHIFT },
    1,
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
    1549,
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
/***** Field: KEY_IND *****/
const ru_field_rec HASH_CAM_INDIRECT_ADDR_KEY_IND_FIELD =
{
    "KEY_IND",
#if RU_INCLUDE_DESC
    "",
    "This bit indicate if the operation (RD/WR) is performed on the key High, Med, Low part of the entry\n",
#endif
    { HASH_CAM_INDIRECT_ADDR_KEY_IND_FIELD_MASK },
    0,
    { HASH_CAM_INDIRECT_ADDR_KEY_IND_FIELD_WIDTH },
    { HASH_CAM_INDIRECT_ADDR_KEY_IND_FIELD_SHIFT },
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
    &HASH_CAM_INDIRECT_ADDR_KEY_IND_FIELD,
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
    1550,
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
    "Valid value to be written - this value is relevant during write operation on key0.\n\nIPTV key is 80 bits, CAM has 2 input bit valid. VALID_IN is concatenated\n\nFCL, ICL and MAC - 1 valid bit\n",
#endif
    { HASH_CAM_INDIRECT_VLID_IN_REG_OFFSET },
    0,
    0,
    1551,
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
    ".\nCHANGED_BY_HW\n\n\n\n",
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
    "Valid value read from the CAM - this value is relevant during read operation on key0.\n\nIPTV key is 80 bits so 2 bit valid coming from CAM\n\nFCL, ICL and MAC - 1 valid bit\n",
#endif
    { HASH_CAM_INDIRECT_VLID_OUT_REG_OFFSET },
    0,
    0,
    1552,
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
    1553,
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
    "KEY_IN 0..3 Register",
    "Key to be used in Write/Compare operations.\nThe Key +cfg are 67/128 bit long and are represented by 3/4 registers.\nThe lower address register corresponds to the MSB bits of the key.\n\nIPTV:\n{KEY0[31:0],KEY1[31:0],KEY2[31:0],KEY3 [31:3],KEY3 [2:0]}\n\nTable cfg are in bits [2:0] KEY3\n\n\nFCL,ICL and MAC:\n{KEY0[31:0],KEY1[31:0],KEY2 [15:13]}\n\nTable cfg are in bits [15:13] KEY2\n\nRDWD:\n{KEY0[31:0],KEY1[31:4],KEY2 [15:13]}\n\nTable cfg are in bits [15:13] KEY2\n\n\n\n",
#endif
    { HASH_CAM_INDIRECT_KEY_IN_REG_OFFSET },
    HASH_CAM_INDIRECT_KEY_IN_REG_RAM_CNT,
    4,
    1554,
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
    "KEY_OUT 0..3 Register",
    "Key returned from the CAM in a read operation. The Key +cfg is 67/128bit long and is represented by 3/4 registers. The lower address register correspond to the MSB bits of the key.\n\nIPTV: 125 bit key\n{KEY0[31:0],KEY1[31:0],KEY2[31:0],KEY3 [31:3],KEY3 [2:0]}\n\nTable cfg are in bits [2:0] KEY3\n\n\n\n\nFCl, ICL, MAC- 64 bit key\n\n{KEY0[31:0],KEY1[31:0],KEY2 [15:13]}\n\nTable cfg are in bits [15:13] KEY2\n\n\n\nRDWD- 60 bit key\n\n{KEY0[31:0],KEY1[31:4],KEY2 [15:13]}\n\nTable cfg are in bits [15:13] KEY2\n\n",
#endif
    { HASH_CAM_INDIRECT_KEY_OUT_REG_OFFSET },
    HASH_CAM_INDIRECT_KEY_OUT_REG_RAM_CNT,
    4,
    1555,
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
    ".\nCHANGED_BY_HW\n\n",
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
    1556,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_CAM_BIST_BIST_STATUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_CAM_BIST_BIST_CONTROL, TYPE: Type_HASH_BLK_HASH_CAM_BIST_BIST_CONTROL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALUE *****/
const ru_field_rec HASH_CAM_BIST_BIST_CONTROL_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { HASH_CAM_BIST_BIST_CONTROL_VALUE_FIELD_MASK },
    0,
    { HASH_CAM_BIST_BIST_CONTROL_VALUE_FIELD_WIDTH },
    { HASH_CAM_BIST_BIST_CONTROL_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_BIST_BIST_CONTROL_FIELDS[] =
{
    &HASH_CAM_BIST_BIST_CONTROL_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_CAM_BIST_BIST_CONTROL *****/
const ru_reg_rec HASH_CAM_BIST_BIST_CONTROL_REG =
{
    "CAM_BIST_BIST_CONTROL",
#if RU_INCLUDE_DESC
    "BIST_CONTROL Register",
    ".\n",
#endif
    { HASH_CAM_BIST_BIST_CONTROL_REG_OFFSET },
    0,
    0,
    1557,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_CAM_BIST_BIST_CONTROL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_CAM_BIST_BIST_CACHE_REPAIR, TYPE: Type_HASH_BLK_HASH_CAM_BIST_BIST_CACHE_REPAIR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALUE *****/
const ru_field_rec HASH_CAM_BIST_BIST_CACHE_REPAIR_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { HASH_CAM_BIST_BIST_CACHE_REPAIR_VALUE_FIELD_MASK },
    0,
    { HASH_CAM_BIST_BIST_CACHE_REPAIR_VALUE_FIELD_WIDTH },
    { HASH_CAM_BIST_BIST_CACHE_REPAIR_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_BIST_BIST_CACHE_REPAIR_FIELDS[] =
{
    &HASH_CAM_BIST_BIST_CACHE_REPAIR_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_CAM_BIST_BIST_CACHE_REPAIR *****/
const ru_reg_rec HASH_CAM_BIST_BIST_CACHE_REPAIR_REG =
{
    "CAM_BIST_BIST_CACHE_REPAIR",
#if RU_INCLUDE_DESC
    "BIST_CACHE_REPAIR Register",
    ".\n",
#endif
    { HASH_CAM_BIST_BIST_CACHE_REPAIR_REG_OFFSET },
    0,
    0,
    1558,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_CAM_BIST_BIST_CACHE_REPAIR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_CAM_BIST_BIST_DBG_CONTROL, TYPE: Type_HASH_BLK_HASH_CAM_BIST_BIST_DBG_CONTROL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALUE *****/
const ru_field_rec HASH_CAM_BIST_BIST_DBG_CONTROL_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { HASH_CAM_BIST_BIST_DBG_CONTROL_VALUE_FIELD_MASK },
    0,
    { HASH_CAM_BIST_BIST_DBG_CONTROL_VALUE_FIELD_WIDTH },
    { HASH_CAM_BIST_BIST_DBG_CONTROL_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_BIST_BIST_DBG_CONTROL_FIELDS[] =
{
    &HASH_CAM_BIST_BIST_DBG_CONTROL_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_CAM_BIST_BIST_DBG_CONTROL *****/
const ru_reg_rec HASH_CAM_BIST_BIST_DBG_CONTROL_REG =
{
    "CAM_BIST_BIST_DBG_CONTROL",
#if RU_INCLUDE_DESC
    "BIST_DBG_CONTROL Register",
    ".\n",
#endif
    { HASH_CAM_BIST_BIST_DBG_CONTROL_REG_OFFSET },
    0,
    0,
    1559,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_CAM_BIST_BIST_DBG_CONTROL_FIELDS,
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
    1560,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_CAM_BIST_BIST_DBG_DATA_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_INTR_CTRL_ISR, TYPE: Type_HASH_BLK_HASH_INTR_CTRL_ISR
 ******************************************************************************/

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
static const ru_field_rec *HASH_INTR_CTRL_ISR_FIELDS[] =
{
    &HASH_INTR_CTRL_ISR_MULT_MATCH_FIELD,
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
    1561,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
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
    1562,
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
    "Each bit in the mask controls the corresponding interrupt source in the IER\nCHANGED_BY_HW\n\n",
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
    1563,
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
    "Each bit in the mask tests the corresponding interrupt source in the ISR\nCHANGED_BY_HW\n\n",
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
    1564,
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
    1565,
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
    1566,
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
    1567,
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
    1568,
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
    1569,
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
    1570,
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
    1571,
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
    1572,
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
    1573,
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
    1574,
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
    1575,
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
    1576,
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
    1577,
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
    1578,
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
    1579,
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
    1580,
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
    1581,
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
    1582,
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
    1583,
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
    1584,
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
    1585,
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
    1586,
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
    "AGING 0..4611 Register",
    "MAC HASH only-\nEach bit in the ram represents hash/CAM entry.\n(15*8K hash entries + 128CAM entries)/32= 3844 rows\nBit 0 at the ram corresponds to entry 0 (eng0), Bit 1 at the ram corresponds to entry 1 (eng1), and so on..\n",
#endif
    { HASH_AGING_RAM_AGING_REG_OFFSET },
    HASH_AGING_RAM_AGING_REG_RAM_CNT,
    4,
    1587,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_AGING_RAM_AGING_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_STORM_RAM_STORM, TYPE: Type_HASH_BLK_HASH_STORM_RAM_STORM
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec HASH_STORM_RAM_STORM_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    ".\n",
#endif
    { HASH_STORM_RAM_STORM_DATA_FIELD_MASK },
    0,
    { HASH_STORM_RAM_STORM_DATA_FIELD_WIDTH },
    { HASH_STORM_RAM_STORM_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_STORM_RAM_STORM_FIELDS[] =
{
    &HASH_STORM_RAM_STORM_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_STORM_RAM_STORM *****/
const ru_reg_rec HASH_STORM_RAM_STORM_REG =
{
    "STORM_RAM_STORM",
#if RU_INCLUDE_DESC
    "STORM 0..4095 Register",
    "Exist in MAC HASH only\n\n",
#endif
    { HASH_STORM_RAM_STORM_REG_OFFSET },
    HASH_STORM_RAM_STORM_REG_RAM_CNT,
    4,
    1588,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_STORM_RAM_STORM_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_CAM_CONTEXT_RAM_CONTEXT_CAM, TYPE: Type_HASH_BLK_HASH_CAM_CONTEXT_RAM_CONTEXT_CAM
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec HASH_CAM_CONTEXT_RAM_CONTEXT_CAM_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "MAC - 32 bits\nICL/IPTV - 24 bits - 47:24\nFCL - 32 Bits LSB\n",
#endif
    { HASH_CAM_CONTEXT_RAM_CONTEXT_CAM_DATA_FIELD_MASK },
    0,
    { HASH_CAM_CONTEXT_RAM_CONTEXT_CAM_DATA_FIELD_WIDTH },
    { HASH_CAM_CONTEXT_RAM_CONTEXT_CAM_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CAM_CONTEXT_RAM_CONTEXT_CAM_FIELDS[] =
{
    &HASH_CAM_CONTEXT_RAM_CONTEXT_CAM_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_CAM_CONTEXT_RAM_CONTEXT_CAM *****/
const ru_reg_rec HASH_CAM_CONTEXT_RAM_CONTEXT_CAM_REG =
{
    "CAM_CONTEXT_RAM_CONTEXT_CAM",
#if RU_INCLUDE_DESC
    "CONTEXT_LOW_MID_HIGH 0..511 Register",
    "MAC - 32 bits address 0x0\nICL/IPTV - 48bits - 47:0 address 0x0,4\nFCL - 80 Bits - address 0x0,4,8\n",
#endif
    { HASH_CAM_CONTEXT_RAM_CONTEXT_CAM_REG_OFFSET },
    HASH_CAM_CONTEXT_RAM_CONTEXT_CAM_REG_RAM_CNT,
    4,
    1589,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_CAM_CONTEXT_RAM_CONTEXT_CAM_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_CONTEXT_RAM_CONTEXT, TYPE: Type_HASH_BLK_HASH_CONTEXT_RAM_CONTEXT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec HASH_CONTEXT_RAM_CONTEXT_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "MAC - 32 bits\nICL/IPTV - 24 bits - 47:24\nFCL - 32 Bits LSB\n",
#endif
    { HASH_CONTEXT_RAM_CONTEXT_DATA_FIELD_MASK },
    0,
    { HASH_CONTEXT_RAM_CONTEXT_DATA_FIELD_WIDTH },
    { HASH_CONTEXT_RAM_CONTEXT_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_CONTEXT_RAM_CONTEXT_FIELDS[] =
{
    &HASH_CONTEXT_RAM_CONTEXT_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_CONTEXT_RAM_CONTEXT *****/
const ru_reg_rec HASH_CONTEXT_RAM_CONTEXT_REG =
{
    "CONTEXT_RAM_CONTEXT",
#if RU_INCLUDE_DESC
    "CONTEXT_LOW_MID_HIGH 0..245759 Register",
    "calculate 64440x4\n\nMAC - 32 bits address 0x0\nICL/IPTV - 48bits - 47:0 address 0x0,4\nFCL - 80 Bits - address 0x0,4,8\n",
#endif
    { HASH_CONTEXT_RAM_CONTEXT_REG_OFFSET },
    HASH_CONTEXT_RAM_CONTEXT_REG_RAM_CNT,
    4,
    1590,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_CONTEXT_RAM_CONTEXT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: HASH_RAM_HASH_RAM, TYPE: Type_HASH_BLK_HASH_RAM_HASH_RAM
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec HASH_RAM_HASH_RAM_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "MAC - 32 bits\nICL/IPTV - 24 bits - 47:24\nFCL - 32 Bits LSB\n",
#endif
    { HASH_RAM_HASH_RAM_DATA_FIELD_MASK },
    0,
    { HASH_RAM_HASH_RAM_DATA_FIELD_WIDTH },
    { HASH_RAM_HASH_RAM_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *HASH_RAM_HASH_RAM_FIELDS[] =
{
    &HASH_RAM_HASH_RAM_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: HASH_RAM_HASH_RAM *****/
const ru_reg_rec HASH_RAM_HASH_RAM_REG =
{
    "RAM_HASH_RAM",
#if RU_INCLUDE_DESC
    "HASH_LOW_MID_HIGH 0..589823 Register",
    "FCL - 15 engine, 4096x67 Bits address 0x0,4,8\nICL - 11 engine, 4096x67 bits address 0x0,4,8\nIPTV- 11 engine, 2048x128 bits address 0x0,4,8,c\nMAC - 18 engine, 8192x67 bits address 0x0,4,8\n\n\nMax Nr is calculated on MAC table:\n18 (eng) * 8192 (table size) *4 (4 bytes)\n\n",
#endif
    { HASH_RAM_HASH_RAM_REG_OFFSET },
    HASH_RAM_HASH_RAM_REG_RAM_CNT,
    4,
    1591,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    HASH_RAM_HASH_RAM_FIELDS,
#endif
};

unsigned long HASH_ADDRS[] =
{
    0xB4000000,
};

static const ru_reg_rec *HASH_REGS[] =
{
    &HASH_GENERAL_CONFIGURATION_GEN_CFG_REG,
    &HASH_GENERAL_CONFIGURATION_FIFO_THRESHOLD_REG,
    &HASH_GENERAL_CONFIGURATION_MULT_HIT_ERR_REG,
    &HASH_GENERAL_CONFIGURATION_AUTO_INIT_EN_REG,
    &HASH_GENERAL_CONFIGURATION_STORM_AUTO_INIT_EN_REG,
    &HASH_PM_COUNTERS_HITS_REG,
    &HASH_PM_COUNTERS_SRCHS_REG,
    &HASH_PM_COUNTERS_MISS_REG,
    &HASH_PM_COUNTERS_STORM_WINDOW_SIZE_REG,
    &HASH_PM_COUNTERS_STORM_CFG_REG,
    &HASH_PM_COUNTERS_FRZ_CNT_REG,
    &HASH_LKUP_TBL_CFG_CNTXT_CFG_REG,
    &HASH_LKUP_TBL_CFG_KEY_MASK_HIGHEST_REG,
    &HASH_LKUP_TBL_CFG_KEY_MASK_HIGH_REG,
    &HASH_LKUP_TBL_CFG_KEY_MASK_MID_REG,
    &HASH_LKUP_TBL_CFG_KEY_MASK_LOW_REG,
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
    &HASH_CAM_BIST_BIST_CONTROL_REG,
    &HASH_CAM_BIST_BIST_CACHE_REPAIR_REG,
    &HASH_CAM_BIST_BIST_DBG_CONTROL_REG,
    &HASH_CAM_BIST_BIST_DBG_DATA_REG,
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
    &HASH_STORM_RAM_STORM_REG,
    &HASH_CAM_CONTEXT_RAM_CONTEXT_CAM_REG,
    &HASH_CONTEXT_RAM_CONTEXT_REG,
    &HASH_RAM_HASH_RAM_REG,
};

const ru_block_rec HASH_BLOCK =
{
    "HASH",
#if RU_INCLUDE_SIZE
    { 0x00640000 },
#endif
    HASH_ADDRS,
    1,
    61,
    HASH_REGS,
};
