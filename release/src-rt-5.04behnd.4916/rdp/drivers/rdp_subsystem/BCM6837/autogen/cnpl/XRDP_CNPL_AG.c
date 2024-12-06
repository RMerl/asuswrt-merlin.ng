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


#include "XRDP_CNPL_AG.h"

/******************************************************************************
 * Register: NAME: CNPL_MEMORY_DATA, TYPE: Type_CNPL_BLOCK_CNPL_MEMORY_DATA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec CNPL_MEMORY_DATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "data\n",
#endif
    { CNPL_MEMORY_DATA_DATA_FIELD_MASK },
    0,
    { CNPL_MEMORY_DATA_DATA_FIELD_WIDTH },
    { CNPL_MEMORY_DATA_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_MEMORY_DATA_FIELDS[] =
{
    &CNPL_MEMORY_DATA_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_MEMORY_DATA *****/
const ru_reg_rec CNPL_MEMORY_DATA_REG =
{
    "MEMORY_DATA",
#if RU_INCLUDE_DESC
    "MEM_ENTRY 0..5119 Register",
    "mem_entry.\nNote: if mem_addr_bit_sel (in CNPL_BLOCK.CNPL_Misc.ARB_PRM) > 9 (default is 15), only half the address space is accessible (10MB out of 20MB).\n",
#endif
    { CNPL_MEMORY_DATA_REG_OFFSET },
    CNPL_MEMORY_DATA_REG_RAM_CNT,
    4,
    226,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_MEMORY_DATA_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF, TYPE: Type_CNPL_BLOCK_CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BA *****/
const ru_field_rec CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_BA_FIELD =
{
    "BA",
#if RU_INCLUDE_DESC
    "",
    "counters group base address (in 8B resolution: 0 is 0x0, 1 is 0x8, ...)\n",
#endif
    { CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_BA_FIELD_MASK },
    0,
    { CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_BA_FIELD_WIDTH },
    { CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_BA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CN0_BYTS *****/
const ru_field_rec CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_CN0_BYTS_FIELD =
{
    "CN0_BYTS",
#if RU_INCLUDE_DESC
    "",
    "number of bytes that will hold each sub-counter.\n0: 1B\n1: 2B\n2: 4B\n3: 8B (only single, no double supported)\n",
#endif
    { CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_CN0_BYTS_FIELD_MASK },
    0,
    { CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_CN0_BYTS_FIELD_WIDTH },
    { CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_CN0_BYTS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DOUBLLE *****/
const ru_field_rec CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_DOUBLLE_FIELD =
{
    "DOUBLLE",
#if RU_INCLUDE_DESC
    "",
    "1:each counter of the group is double sub-cntr (for 4B-double the format is 36b+28b)\n0:each counter of the group is single\n",
#endif
    { CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_DOUBLLE_FIELD_MASK },
    0,
    { CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_DOUBLLE_FIELD_WIDTH },
    { CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_DOUBLLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: WRAP *****/
const ru_field_rec CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_WRAP_FIELD =
{
    "WRAP",
#if RU_INCLUDE_DESC
    "",
    "1:wrap at max value\n0:freeze at max value\n",
#endif
    { CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_WRAP_FIELD_MASK },
    0,
    { CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_WRAP_FIELD_WIDTH },
    { CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_WRAP_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CLR *****/
const ru_field_rec CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_CLR_FIELD =
{
    "CLR",
#if RU_INCLUDE_DESC
    "",
    "1:read clear when reading\n0:read no-clear when reading\n",
#endif
    { CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_CLR_FIELD_MASK },
    0,
    { CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_CLR_FIELD_WIDTH },
    { CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_CLR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_FIELDS[] =
{
    &CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_BA_FIELD,
    &CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_CN0_BYTS_FIELD,
    &CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_DOUBLLE_FIELD,
    &CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_WRAP_FIELD,
    &CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_CLR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF *****/
const ru_reg_rec CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_REG =
{
    "COUNTERS_CONFIGURATIONS_CN_LOC_PROF",
#if RU_INCLUDE_DESC
    "CNT_LOC_PROFILE 0..15 Register",
    "location profiles\n",
#endif
    { CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_REG_OFFSET },
    CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_REG_RAM_CNT,
    4,
    227,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0, TYPE: Type_CNPL_BLOCK_CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BK_BA *****/
const ru_field_rec CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_BK_BA_FIELD =
{
    "BK_BA",
#if RU_INCLUDE_DESC
    "",
    "buckets base address(in 8B resolution: 0 is 0x0, 1 is 0x8, ...)\n",
#endif
    { CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_BK_BA_FIELD_MASK },
    0,
    { CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_BK_BA_FIELD_WIDTH },
    { CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_BK_BA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PA_BA *****/
const ru_field_rec CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_PA_BA_FIELD =
{
    "PA_BA",
#if RU_INCLUDE_DESC
    "",
    "params base address(in 8B resolution: 0 is 0x0, 1 is 0x8, ...)\n",
#endif
    { CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_PA_BA_FIELD_MASK },
    0,
    { CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_PA_BA_FIELD_WIDTH },
    { CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_PA_BA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DOUBLLE *****/
const ru_field_rec CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_DOUBLLE_FIELD =
{
    "DOUBLLE",
#if RU_INCLUDE_DESC
    "",
    "1:each policer is dual bucket\n0:each policer is single bucket\n",
#endif
    { CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_DOUBLLE_FIELD_MASK },
    0,
    { CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_DOUBLLE_FIELD_WIDTH },
    { CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_DOUBLLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FC *****/
const ru_field_rec CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_FC_FIELD =
{
    "FC",
#if RU_INCLUDE_DESC
    "",
    "1:the policers are flow control type\n0:the policers are normal\n\nWhen it is fc, the double field has no effect.\n",
#endif
    { CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_FC_FIELD_MASK },
    0,
    { CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_FC_FIELD_WIDTH },
    { CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_FC_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_FIELDS[] =
{
    &CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_BK_BA_FIELD,
    &CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_PA_BA_FIELD,
    &CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_DOUBLLE_FIELD,
    &CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_FC_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0 *****/
const ru_reg_rec CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_REG =
{
    "POLICERS_CONFIGURATIONS_PL_LOC_PROF0",
#if RU_INCLUDE_DESC
    "PL_LOC_PROFILE0 0..3 Register",
    "1st reg for location profiles\n",
#endif
    { CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_REG_OFFSET },
    CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_REG_RAM_CNT,
    4,
    228,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1, TYPE: Type_CNPL_BLOCK_CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PL_ST *****/
const ru_field_rec CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_PL_ST_FIELD =
{
    "PL_ST",
#if RU_INCLUDE_DESC
    "",
    "Index of 1st policer of the group.\nIn default: pol_start>pol_end, so the group is not valid\n",
#endif
    { CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_PL_ST_FIELD_MASK },
    0,
    { CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_PL_ST_FIELD_WIDTH },
    { CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_PL_ST_FIELD_SHIFT },
    255,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PL_END *****/
const ru_field_rec CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_PL_END_FIELD =
{
    "PL_END",
#if RU_INCLUDE_DESC
    "",
    "Index of last policer of the group.\nIn default: pol_start>pol_end, so the group is not valid\n",
#endif
    { CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_PL_END_FIELD_MASK },
    0,
    { CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_PL_END_FIELD_WIDTH },
    { CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_PL_END_FIELD_SHIFT },
    254,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: N *****/
const ru_field_rec CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_N_FIELD =
{
    "N",
#if RU_INCLUDE_DESC
    "",
    "shift left of 8k cycles quanta. 4b lsb only are valid.\n",
#endif
    { CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_N_FIELD_MASK },
    0,
    { CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_N_FIELD_WIDTH },
    { CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_N_FIELD_SHIFT },
    6,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_FIELDS[] =
{
    &CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_PL_ST_FIELD,
    &CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_PL_END_FIELD,
    &CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_N_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1 *****/
const ru_reg_rec CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_REG =
{
    "POLICERS_CONFIGURATIONS_PL_LOC_PROF1",
#if RU_INCLUDE_DESC
    "PL_LOC_PROFILE1 0..3 Register",
    "2nd reg for location profiles\n",
#endif
    { CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_REG_OFFSET },
    CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_REG_RAM_CNT,
    4,
    229,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CNPL_POLICERS_CONFIGURATIONS_PL_CALC_TYPE, TYPE: Type_CNPL_BLOCK_CNPL_POLICERS_CONFIGURATIONS_PL_CALC_TYPE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VEC *****/
const ru_field_rec CNPL_POLICERS_CONFIGURATIONS_PL_CALC_TYPE_VEC_FIELD =
{
    "VEC",
#if RU_INCLUDE_DESC
    "",
    "32b vector for 32 policers\n",
#endif
    { CNPL_POLICERS_CONFIGURATIONS_PL_CALC_TYPE_VEC_FIELD_MASK },
    0,
    { CNPL_POLICERS_CONFIGURATIONS_PL_CALC_TYPE_VEC_FIELD_WIDTH },
    { CNPL_POLICERS_CONFIGURATIONS_PL_CALC_TYPE_VEC_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_POLICERS_CONFIGURATIONS_PL_CALC_TYPE_FIELDS[] =
{
    &CNPL_POLICERS_CONFIGURATIONS_PL_CALC_TYPE_VEC_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_POLICERS_CONFIGURATIONS_PL_CALC_TYPE *****/
const ru_reg_rec CNPL_POLICERS_CONFIGURATIONS_PL_CALC_TYPE_REG =
{
    "POLICERS_CONFIGURATIONS_PL_CALC_TYPE",
#if RU_INCLUDE_DESC
    "PL_CALC_TYPE 0..7 Register",
    "calculation type register.\n0:green, yellow, red\n1:red, yellow, green\n\n8 registers support up to 256 policers.\n",
#endif
    { CNPL_POLICERS_CONFIGURATIONS_PL_CALC_TYPE_REG_OFFSET },
    CNPL_POLICERS_CONFIGURATIONS_PL_CALC_TYPE_REG_RAM_CNT,
    4,
    230,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_POLICERS_CONFIGURATIONS_PL_CALC_TYPE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CNPL_POLICERS_CONFIGURATIONS_PL_SIZE_PROF, TYPE: Type_CNPL_BLOCK_CNPL_POLICERS_CONFIGURATIONS_PL_SIZE_PROF
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PRF0 *****/
const ru_field_rec CNPL_POLICERS_CONFIGURATIONS_PL_SIZE_PROF_PRF0_FIELD =
{
    "PRF0",
#if RU_INCLUDE_DESC
    "",
    "16b profile\n",
#endif
    { CNPL_POLICERS_CONFIGURATIONS_PL_SIZE_PROF_PRF0_FIELD_MASK },
    0,
    { CNPL_POLICERS_CONFIGURATIONS_PL_SIZE_PROF_PRF0_FIELD_WIDTH },
    { CNPL_POLICERS_CONFIGURATIONS_PL_SIZE_PROF_PRF0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PRF1 *****/
const ru_field_rec CNPL_POLICERS_CONFIGURATIONS_PL_SIZE_PROF_PRF1_FIELD =
{
    "PRF1",
#if RU_INCLUDE_DESC
    "",
    "16b profile\n",
#endif
    { CNPL_POLICERS_CONFIGURATIONS_PL_SIZE_PROF_PRF1_FIELD_MASK },
    0,
    { CNPL_POLICERS_CONFIGURATIONS_PL_SIZE_PROF_PRF1_FIELD_WIDTH },
    { CNPL_POLICERS_CONFIGURATIONS_PL_SIZE_PROF_PRF1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_POLICERS_CONFIGURATIONS_PL_SIZE_PROF_FIELDS[] =
{
    &CNPL_POLICERS_CONFIGURATIONS_PL_SIZE_PROF_PRF0_FIELD,
    &CNPL_POLICERS_CONFIGURATIONS_PL_SIZE_PROF_PRF1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_POLICERS_CONFIGURATIONS_PL_SIZE_PROF *****/
const ru_reg_rec CNPL_POLICERS_CONFIGURATIONS_PL_SIZE_PROF_REG =
{
    "POLICERS_CONFIGURATIONS_PL_SIZE_PROF",
#if RU_INCLUDE_DESC
    "PL_SIZE_PROFILE 0..3 Register",
    "8 profiles, 16b each, for calculation of bucket size\n",
#endif
    { CNPL_POLICERS_CONFIGURATIONS_PL_SIZE_PROF_REG_OFFSET },
    CNPL_POLICERS_CONFIGURATIONS_PL_SIZE_PROF_REG_RAM_CNT,
    4,
    231,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    CNPL_POLICERS_CONFIGURATIONS_PL_SIZE_PROF_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CNPL_POLICERS_CONFIGURATIONS_PER_UP, TYPE: Type_CNPL_BLOCK_CNPL_POLICERS_CONFIGURATIONS_PER_UP
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: EN *****/
const ru_field_rec CNPL_POLICERS_CONFIGURATIONS_PER_UP_EN_FIELD =
{
    "EN",
#if RU_INCLUDE_DESC
    "",
    "1:enable periodic update\n0:disable periodic update\n",
#endif
    { CNPL_POLICERS_CONFIGURATIONS_PER_UP_EN_FIELD_MASK },
    0,
    { CNPL_POLICERS_CONFIGURATIONS_PER_UP_EN_FIELD_WIDTH },
    { CNPL_POLICERS_CONFIGURATIONS_PER_UP_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MTU *****/
const ru_field_rec CNPL_POLICERS_CONFIGURATIONS_PER_UP_MTU_FIELD =
{
    "MTU",
#if RU_INCLUDE_DESC
    "",
    "MTU for calculation of bucket size\n",
#endif
    { CNPL_POLICERS_CONFIGURATIONS_PER_UP_MTU_FIELD_MASK },
    0,
    { CNPL_POLICERS_CONFIGURATIONS_PER_UP_MTU_FIELD_WIDTH },
    { CNPL_POLICERS_CONFIGURATIONS_PER_UP_MTU_FIELD_SHIFT },
    1518,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_POLICERS_CONFIGURATIONS_PER_UP_FIELDS[] =
{
    &CNPL_POLICERS_CONFIGURATIONS_PER_UP_EN_FIELD,
    &CNPL_POLICERS_CONFIGURATIONS_PER_UP_MTU_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_POLICERS_CONFIGURATIONS_PER_UP *****/
const ru_reg_rec CNPL_POLICERS_CONFIGURATIONS_PER_UP_REG =
{
    "POLICERS_CONFIGURATIONS_PER_UP",
#if RU_INCLUDE_DESC
    "PL_PERIODIC_UPDATE Register",
    "periodic update parameters\n",
#endif
    { CNPL_POLICERS_CONFIGURATIONS_PER_UP_REG_OFFSET },
    0,
    0,
    232,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    CNPL_POLICERS_CONFIGURATIONS_PER_UP_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CNPL_SW_IF_SW_CMD, TYPE: Type_CNPL_BLOCK_CNPL_SW_IF_SW_CMD
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec CNPL_SW_IF_SW_CMD_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "value of register\n",
#endif
    { CNPL_SW_IF_SW_CMD_VAL_FIELD_MASK },
    0,
    { CNPL_SW_IF_SW_CMD_VAL_FIELD_WIDTH },
    { CNPL_SW_IF_SW_CMD_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_SW_IF_SW_CMD_FIELDS[] =
{
    &CNPL_SW_IF_SW_CMD_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_SW_IF_SW_CMD *****/
const ru_reg_rec CNPL_SW_IF_SW_CMD_REG =
{
    "SW_IF_SW_CMD",
#if RU_INCLUDE_DESC
    "COMMAND Register",
    "command register\n",
#endif
    { CNPL_SW_IF_SW_CMD_REG_OFFSET },
    0,
    0,
    233,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_SW_IF_SW_CMD_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CNPL_SW_IF_SW_STAT, TYPE: Type_CNPL_BLOCK_CNPL_SW_IF_SW_STAT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CN_RD_ST *****/
const ru_field_rec CNPL_SW_IF_SW_STAT_CN_RD_ST_FIELD =
{
    "CN_RD_ST",
#if RU_INCLUDE_DESC
    "",
    "0: DONE (ready)\n1: PROC(not ready)\n",
#endif
    { CNPL_SW_IF_SW_STAT_CN_RD_ST_FIELD_MASK },
    0,
    { CNPL_SW_IF_SW_STAT_CN_RD_ST_FIELD_WIDTH },
    { CNPL_SW_IF_SW_STAT_CN_RD_ST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PL_PLC_ST *****/
const ru_field_rec CNPL_SW_IF_SW_STAT_PL_PLC_ST_FIELD =
{
    "PL_PLC_ST",
#if RU_INCLUDE_DESC
    "",
    "0: DONE (ready)\n1: PROC(not ready)\n",
#endif
    { CNPL_SW_IF_SW_STAT_PL_PLC_ST_FIELD_MASK },
    0,
    { CNPL_SW_IF_SW_STAT_PL_PLC_ST_FIELD_WIDTH },
    { CNPL_SW_IF_SW_STAT_PL_PLC_ST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PL_RD_ST *****/
const ru_field_rec CNPL_SW_IF_SW_STAT_PL_RD_ST_FIELD =
{
    "PL_RD_ST",
#if RU_INCLUDE_DESC
    "",
    "0: DONE (ready)\n1: PROC(not ready)\n",
#endif
    { CNPL_SW_IF_SW_STAT_PL_RD_ST_FIELD_MASK },
    0,
    { CNPL_SW_IF_SW_STAT_PL_RD_ST_FIELD_WIDTH },
    { CNPL_SW_IF_SW_STAT_PL_RD_ST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_SW_IF_SW_STAT_FIELDS[] =
{
    &CNPL_SW_IF_SW_STAT_CN_RD_ST_FIELD,
    &CNPL_SW_IF_SW_STAT_PL_PLC_ST_FIELD,
    &CNPL_SW_IF_SW_STAT_PL_RD_ST_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_SW_IF_SW_STAT *****/
const ru_reg_rec CNPL_SW_IF_SW_STAT_REG =
{
    "SW_IF_SW_STAT",
#if RU_INCLUDE_DESC
    "STATUS Register",
    "status register\n",
#endif
    { CNPL_SW_IF_SW_STAT_REG_OFFSET },
    0,
    0,
    234,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    CNPL_SW_IF_SW_STAT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CNPL_SW_IF_SW_PL_RSLT, TYPE: Type_CNPL_BLOCK_CNPL_SW_IF_SW_PL_RSLT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COL *****/
const ru_field_rec CNPL_SW_IF_SW_PL_RSLT_COL_FIELD =
{
    "COL",
#if RU_INCLUDE_DESC
    "",
    "red, yellow, green, non-active\n",
#endif
    { CNPL_SW_IF_SW_PL_RSLT_COL_FIELD_MASK },
    0,
    { CNPL_SW_IF_SW_PL_RSLT_COL_FIELD_WIDTH },
    { CNPL_SW_IF_SW_PL_RSLT_COL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_SW_IF_SW_PL_RSLT_FIELDS[] =
{
    &CNPL_SW_IF_SW_PL_RSLT_COL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_SW_IF_SW_PL_RSLT *****/
const ru_reg_rec CNPL_SW_IF_SW_PL_RSLT_REG =
{
    "SW_IF_SW_PL_RSLT",
#if RU_INCLUDE_DESC
    "PL_RSLT Register",
    "rdata register - policer command result\n",
#endif
    { CNPL_SW_IF_SW_PL_RSLT_REG_OFFSET },
    0,
    0,
    235,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_SW_IF_SW_PL_RSLT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CNPL_SW_IF_SW_PL_RD, TYPE: Type_CNPL_BLOCK_CNPL_SW_IF_SW_PL_RD
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RD *****/
const ru_field_rec CNPL_SW_IF_SW_PL_RD_RD_FIELD =
{
    "RD",
#if RU_INCLUDE_DESC
    "",
    "value of read data\n",
#endif
    { CNPL_SW_IF_SW_PL_RD_RD_FIELD_MASK },
    0,
    { CNPL_SW_IF_SW_PL_RD_RD_FIELD_WIDTH },
    { CNPL_SW_IF_SW_PL_RD_RD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_SW_IF_SW_PL_RD_FIELDS[] =
{
    &CNPL_SW_IF_SW_PL_RD_RD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_SW_IF_SW_PL_RD *****/
const ru_reg_rec CNPL_SW_IF_SW_PL_RD_REG =
{
    "SW_IF_SW_PL_RD",
#if RU_INCLUDE_DESC
    "PL_RDX 0..1 Register",
    "rdata register - policer read command result. 2 register for 2 buckets. If the group has only one bucket per policer - the policers are returned in the registers as a full line: the even policers are in reg0 (0,2,4,..), and the odd are in reg1.\n",
#endif
    { CNPL_SW_IF_SW_PL_RD_REG_OFFSET },
    CNPL_SW_IF_SW_PL_RD_REG_RAM_CNT,
    4,
    236,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_SW_IF_SW_PL_RD_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CNPL_SW_IF_SW_CNT_RD, TYPE: Type_CNPL_BLOCK_CNPL_SW_IF_SW_CNT_RD
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RD *****/
const ru_field_rec CNPL_SW_IF_SW_CNT_RD_RD_FIELD =
{
    "RD",
#if RU_INCLUDE_DESC
    "",
    "value of read data\n",
#endif
    { CNPL_SW_IF_SW_CNT_RD_RD_FIELD_MASK },
    0,
    { CNPL_SW_IF_SW_CNT_RD_RD_FIELD_WIDTH },
    { CNPL_SW_IF_SW_CNT_RD_RD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_SW_IF_SW_CNT_RD_FIELDS[] =
{
    &CNPL_SW_IF_SW_CNT_RD_RD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_SW_IF_SW_CNT_RD *****/
const ru_reg_rec CNPL_SW_IF_SW_CNT_RD_REG =
{
    "SW_IF_SW_CNT_RD",
#if RU_INCLUDE_DESC
    "CNT_RDX 0..7 Register",
    "rdata register - counters read command result. 8 register for 32B batch. In read of single counter (burst size=1) the output will be in reg0 (the 32b where the counter is). In read of burst of counters, the counters are returned in the registers as a full line: addr[2:0]=0 section of line in reg0,2,4,6  and the addr[2:0]=4 are in reg1,3,5,7 (this means that if the start of burst is at addr[2:0]=4 section of line, the wanted output should be from reg1).\n",
#endif
    { CNPL_SW_IF_SW_CNT_RD_REG_OFFSET },
    CNPL_SW_IF_SW_CNT_RD_REG_RAM_CNT,
    4,
    237,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_SW_IF_SW_CNT_RD_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CNPL_MISC_ARB_PRM, TYPE: Type_CNPL_BLOCK_CNPL_MISC_ARB_PRM
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SW_PRIO *****/
const ru_field_rec CNPL_MISC_ARB_PRM_SW_PRIO_FIELD =
{
    "SW_PRIO",
#if RU_INCLUDE_DESC
    "",
    "0: fixed lower\n1: rr with fw (default)\n2: fixed higher\n",
#endif
    { CNPL_MISC_ARB_PRM_SW_PRIO_FIELD_MASK },
    0,
    { CNPL_MISC_ARB_PRM_SW_PRIO_FIELD_WIDTH },
    { CNPL_MISC_ARB_PRM_SW_PRIO_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MEM_ADDR_BIT_SEL *****/
const ru_field_rec CNPL_MISC_ARB_PRM_MEM_ADDR_BIT_SEL_FIELD =
{
    "MEM_ADDR_BIT_SEL",
#if RU_INCLUDE_DESC
    "",
    "15 possible options to select which address bit decodes to which cnpl memory the transaction goes.\nCurrently only 9 bits are needed (for 20K memory).\nValue of 0xF is kept to disable this feature (legacy mode), and all accesses will be directed to memory0.\nBits [0..8] are valid values for 20K memory.\nNote: if mem_addr_bit_sel (in CNPL_BLOCK.CNPL_Misc.ARB_PRM) > 8, only half the address space is accessible (10MB out of 20MB).\n",
#endif
    { CNPL_MISC_ARB_PRM_MEM_ADDR_BIT_SEL_FIELD_MASK },
    0,
    { CNPL_MISC_ARB_PRM_MEM_ADDR_BIT_SEL_FIELD_WIDTH },
    { CNPL_MISC_ARB_PRM_MEM_ADDR_BIT_SEL_FIELD_SHIFT },
    15,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_MISC_ARB_PRM_FIELDS[] =
{
    &CNPL_MISC_ARB_PRM_SW_PRIO_FIELD,
    &CNPL_MISC_ARB_PRM_MEM_ADDR_BIT_SEL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_MISC_ARB_PRM *****/
const ru_reg_rec CNPL_MISC_ARB_PRM_REG =
{
    "MISC_ARB_PRM",
#if RU_INCLUDE_DESC
    "ARBITER_PARAM Register",
    "arbiter sw priorities\n",
#endif
    { CNPL_MISC_ARB_PRM_REG_OFFSET },
    0,
    0,
    238,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    CNPL_MISC_ARB_PRM_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CNPL_MISC_COL_AWR_EN, TYPE: Type_CNPL_BLOCK_CNPL_MISC_COL_AWR_EN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: EN *****/
const ru_field_rec CNPL_MISC_COL_AWR_EN_EN_FIELD =
{
    "EN",
#if RU_INCLUDE_DESC
    "",
    "0: dis\n1: en\n",
#endif
    { CNPL_MISC_COL_AWR_EN_EN_FIELD_MASK },
    0,
    { CNPL_MISC_COL_AWR_EN_EN_FIELD_WIDTH },
    { CNPL_MISC_COL_AWR_EN_EN_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_MISC_COL_AWR_EN_FIELDS[] =
{
    &CNPL_MISC_COL_AWR_EN_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_MISC_COL_AWR_EN *****/
const ru_reg_rec CNPL_MISC_COL_AWR_EN_REG =
{
    "MISC_COL_AWR_EN",
#if RU_INCLUDE_DESC
    "COLOR_AWARE_ENABLE Register",
    "color aware enable\n",
#endif
    { CNPL_MISC_COL_AWR_EN_REG_OFFSET },
    0,
    0,
    239,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_MISC_COL_AWR_EN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CNPL_MISC_MEM_INIT, TYPE: Type_CNPL_BLOCK_CNPL_MISC_MEM_INIT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: INIT0 *****/
const ru_field_rec CNPL_MISC_MEM_INIT_INIT0_FIELD =
{
    "INIT0",
#if RU_INCLUDE_DESC
    "",
    "init for mem 0\n",
#endif
    { CNPL_MISC_MEM_INIT_INIT0_FIELD_MASK },
    0,
    { CNPL_MISC_MEM_INIT_INIT0_FIELD_WIDTH },
    { CNPL_MISC_MEM_INIT_INIT0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: INIT1 *****/
const ru_field_rec CNPL_MISC_MEM_INIT_INIT1_FIELD =
{
    "INIT1",
#if RU_INCLUDE_DESC
    "",
    "init for mem 1\n",
#endif
    { CNPL_MISC_MEM_INIT_INIT1_FIELD_MASK },
    0,
    { CNPL_MISC_MEM_INIT_INIT1_FIELD_WIDTH },
    { CNPL_MISC_MEM_INIT_INIT1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_MISC_MEM_INIT_FIELDS[] =
{
    &CNPL_MISC_MEM_INIT_INIT0_FIELD,
    &CNPL_MISC_MEM_INIT_INIT1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_MISC_MEM_INIT *****/
const ru_reg_rec CNPL_MISC_MEM_INIT_REG =
{
    "MISC_MEM_INIT",
#if RU_INCLUDE_DESC
    "MEM_INIT Register",
    "initialization for the 2 memories(2 bits).\nwr 1 to start init, poll for 0 for done.\nPlease dont check rd value in reg test, since it changes automatically after memory is initialized.\n",
#endif
    { CNPL_MISC_MEM_INIT_REG_OFFSET },
    0,
    0,
    240,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    CNPL_MISC_MEM_INIT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CNPL_PM_COUNTERS_ENG_CMDS, TYPE: Type_CNPL_BLOCK_CNPL_PM_COUNTERS_ENG_CMDS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec CNPL_PM_COUNTERS_ENG_CMDS_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "value\n",
#endif
    { CNPL_PM_COUNTERS_ENG_CMDS_VAL_FIELD_MASK },
    0,
    { CNPL_PM_COUNTERS_ENG_CMDS_VAL_FIELD_WIDTH },
    { CNPL_PM_COUNTERS_ENG_CMDS_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_PM_COUNTERS_ENG_CMDS_FIELDS[] =
{
    &CNPL_PM_COUNTERS_ENG_CMDS_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_PM_COUNTERS_ENG_CMDS *****/
const ru_reg_rec CNPL_PM_COUNTERS_ENG_CMDS_REG =
{
    "PM_COUNTERS_ENG_CMDS",
#if RU_INCLUDE_DESC
    "ENG_CMDS_CNTR 0..22 Register",
    "Number of commands that were processed by the engine.\norder:\n0-9: counters\n10-12: policers\n13-14: reserved\n(15: buf-mng cntr. not used in 68880)\n16-19: reserved\n20: sw counter read\n21: sw policer\n22:sw policer rd\n\n",
#endif
    { CNPL_PM_COUNTERS_ENG_CMDS_REG_OFFSET },
    CNPL_PM_COUNTERS_ENG_CMDS_REG_RAM_CNT,
    4,
    241,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_PM_COUNTERS_ENG_CMDS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CNPL_PM_COUNTERS_CMD_WAIT, TYPE: Type_CNPL_BLOCK_CNPL_PM_COUNTERS_CMD_WAIT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec CNPL_PM_COUNTERS_CMD_WAIT_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "value\n",
#endif
    { CNPL_PM_COUNTERS_CMD_WAIT_VAL_FIELD_MASK },
    0,
    { CNPL_PM_COUNTERS_CMD_WAIT_VAL_FIELD_WIDTH },
    { CNPL_PM_COUNTERS_CMD_WAIT_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_PM_COUNTERS_CMD_WAIT_FIELDS[] =
{
    &CNPL_PM_COUNTERS_CMD_WAIT_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_PM_COUNTERS_CMD_WAIT *****/
const ru_reg_rec CNPL_PM_COUNTERS_CMD_WAIT_REG =
{
    "PM_COUNTERS_CMD_WAIT",
#if RU_INCLUDE_DESC
    "CMD_WAITS_CNTR 0..1 Register",
    "Number of wait cycles that the command waited until there was an idle engine.\nidx0: cntr cmds\nidx1: pol  cmds\n",
#endif
    { CNPL_PM_COUNTERS_CMD_WAIT_REG_OFFSET },
    CNPL_PM_COUNTERS_CMD_WAIT_REG_RAM_CNT,
    4,
    242,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_PM_COUNTERS_CMD_WAIT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CNPL_PM_COUNTERS_TOT_CYC, TYPE: Type_CNPL_BLOCK_CNPL_PM_COUNTERS_TOT_CYC
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec CNPL_PM_COUNTERS_TOT_CYC_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "value\n",
#endif
    { CNPL_PM_COUNTERS_TOT_CYC_VAL_FIELD_MASK },
    0,
    { CNPL_PM_COUNTERS_TOT_CYC_VAL_FIELD_WIDTH },
    { CNPL_PM_COUNTERS_TOT_CYC_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_PM_COUNTERS_TOT_CYC_FIELDS[] =
{
    &CNPL_PM_COUNTERS_TOT_CYC_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_PM_COUNTERS_TOT_CYC *****/
const ru_reg_rec CNPL_PM_COUNTERS_TOT_CYC_REG =
{
    "PM_COUNTERS_TOT_CYC",
#if RU_INCLUDE_DESC
    "TOT_CYC_CNTR 0..2 Register",
    "Number of cycles from last read clear.\nreg 1(0x0): mem0\nreg 2(0x4): mem1\nreg 3(0x8): both are active\n",
#endif
    { CNPL_PM_COUNTERS_TOT_CYC_REG_OFFSET },
    CNPL_PM_COUNTERS_TOT_CYC_REG_RAM_CNT,
    4,
    243,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_PM_COUNTERS_TOT_CYC_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CNPL_PM_COUNTERS_GNT_CYC, TYPE: Type_CNPL_BLOCK_CNPL_PM_COUNTERS_GNT_CYC
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec CNPL_PM_COUNTERS_GNT_CYC_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "value\n",
#endif
    { CNPL_PM_COUNTERS_GNT_CYC_VAL_FIELD_MASK },
    0,
    { CNPL_PM_COUNTERS_GNT_CYC_VAL_FIELD_WIDTH },
    { CNPL_PM_COUNTERS_GNT_CYC_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_PM_COUNTERS_GNT_CYC_FIELDS[] =
{
    &CNPL_PM_COUNTERS_GNT_CYC_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_PM_COUNTERS_GNT_CYC *****/
const ru_reg_rec CNPL_PM_COUNTERS_GNT_CYC_REG =
{
    "PM_COUNTERS_GNT_CYC",
#if RU_INCLUDE_DESC
    "GNT_CYC_CNTR 0..2 Register",
    "Number of cycles that there was gnt from memX arbiter.\nreg 1(0x0): mem0\nreg 2(0x4): mem1\nreg 3(0x8): both had gnts\n",
#endif
    { CNPL_PM_COUNTERS_GNT_CYC_REG_OFFSET },
    CNPL_PM_COUNTERS_GNT_CYC_REG_RAM_CNT,
    4,
    244,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_PM_COUNTERS_GNT_CYC_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CNPL_PM_COUNTERS_ARB_CYC, TYPE: Type_CNPL_BLOCK_CNPL_PM_COUNTERS_ARB_CYC
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec CNPL_PM_COUNTERS_ARB_CYC_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "value\n",
#endif
    { CNPL_PM_COUNTERS_ARB_CYC_VAL_FIELD_MASK },
    0,
    { CNPL_PM_COUNTERS_ARB_CYC_VAL_FIELD_WIDTH },
    { CNPL_PM_COUNTERS_ARB_CYC_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_PM_COUNTERS_ARB_CYC_FIELDS[] =
{
    &CNPL_PM_COUNTERS_ARB_CYC_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_PM_COUNTERS_ARB_CYC *****/
const ru_reg_rec CNPL_PM_COUNTERS_ARB_CYC_REG =
{
    "PM_COUNTERS_ARB_CYC",
#if RU_INCLUDE_DESC
    "ARB_CYC_CNTR 0..2 Register",
    "Number of cycles that there was gnt with request of more than one agent.\nreg 1(0x0): mem0\nreg 2(0x4): mem1\nreg 3(0x8): both are active\n",
#endif
    { CNPL_PM_COUNTERS_ARB_CYC_REG_OFFSET },
    CNPL_PM_COUNTERS_ARB_CYC_REG_RAM_CNT,
    4,
    245,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_PM_COUNTERS_ARB_CYC_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CNPL_PM_COUNTERS_PL_UP_ERR, TYPE: Type_CNPL_BLOCK_CNPL_PM_COUNTERS_PL_UP_ERR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec CNPL_PM_COUNTERS_PL_UP_ERR_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "value\n",
#endif
    { CNPL_PM_COUNTERS_PL_UP_ERR_VAL_FIELD_MASK },
    0,
    { CNPL_PM_COUNTERS_PL_UP_ERR_VAL_FIELD_WIDTH },
    { CNPL_PM_COUNTERS_PL_UP_ERR_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_PM_COUNTERS_PL_UP_ERR_FIELDS[] =
{
    &CNPL_PM_COUNTERS_PL_UP_ERR_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_PM_COUNTERS_PL_UP_ERR *****/
const ru_reg_rec CNPL_PM_COUNTERS_PL_UP_ERR_REG =
{
    "PM_COUNTERS_PL_UP_ERR",
#if RU_INCLUDE_DESC
    "POL_UP_ERR_CNTR 0..3 Register",
    "errors in policer update: the update period finished, and not all policers have been updated yet.\n",
#endif
    { CNPL_PM_COUNTERS_PL_UP_ERR_REG_OFFSET },
    CNPL_PM_COUNTERS_PL_UP_ERR_REG_RAM_CNT,
    4,
    246,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_PM_COUNTERS_PL_UP_ERR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CNPL_PM_COUNTERS_GEN_CFG, TYPE: Type_CNPL_BLOCK_CNPL_PM_COUNTERS_GEN_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RD_CLR *****/
const ru_field_rec CNPL_PM_COUNTERS_GEN_CFG_RD_CLR_FIELD =
{
    "RD_CLR",
#if RU_INCLUDE_DESC
    "",
    "read clear bit\n",
#endif
    { CNPL_PM_COUNTERS_GEN_CFG_RD_CLR_FIELD_MASK },
    0,
    { CNPL_PM_COUNTERS_GEN_CFG_RD_CLR_FIELD_WIDTH },
    { CNPL_PM_COUNTERS_GEN_CFG_RD_CLR_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: WRAP *****/
const ru_field_rec CNPL_PM_COUNTERS_GEN_CFG_WRAP_FIELD =
{
    "WRAP",
#if RU_INCLUDE_DESC
    "",
    "read clear bit\n",
#endif
    { CNPL_PM_COUNTERS_GEN_CFG_WRAP_FIELD_MASK },
    0,
    { CNPL_PM_COUNTERS_GEN_CFG_WRAP_FIELD_WIDTH },
    { CNPL_PM_COUNTERS_GEN_CFG_WRAP_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_PM_COUNTERS_GEN_CFG_FIELDS[] =
{
    &CNPL_PM_COUNTERS_GEN_CFG_RD_CLR_FIELD,
    &CNPL_PM_COUNTERS_GEN_CFG_WRAP_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_PM_COUNTERS_GEN_CFG *****/
const ru_reg_rec CNPL_PM_COUNTERS_GEN_CFG_REG =
{
    "PM_COUNTERS_GEN_CFG",
#if RU_INCLUDE_DESC
    "GENERAL_CONFIG Register",
    "bits rd_clr and wrap for the pm counters(above)\n",
#endif
    { CNPL_PM_COUNTERS_GEN_CFG_REG_OFFSET },
    0,
    0,
    247,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    CNPL_PM_COUNTERS_GEN_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CNPL_DEBUG_DBGSEL, TYPE: Type_CNPL_BLOCK_CNPL_DEBUG_DBGSEL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VS *****/
const ru_field_rec CNPL_DEBUG_DBGSEL_VS_FIELD =
{
    "VS",
#if RU_INCLUDE_DESC
    "",
    "selects th debug vector\n",
#endif
    { CNPL_DEBUG_DBGSEL_VS_FIELD_MASK },
    0,
    { CNPL_DEBUG_DBGSEL_VS_FIELD_WIDTH },
    { CNPL_DEBUG_DBGSEL_VS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_DEBUG_DBGSEL_FIELDS[] =
{
    &CNPL_DEBUG_DBGSEL_VS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_DEBUG_DBGSEL *****/
const ru_reg_rec CNPL_DEBUG_DBGSEL_REG =
{
    "DEBUG_DBGSEL",
#if RU_INCLUDE_DESC
    "DBG_MUX_SEL Register",
    "selects the debug vecore\n",
#endif
    { CNPL_DEBUG_DBGSEL_REG_OFFSET },
    0,
    0,
    248,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_DEBUG_DBGSEL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CNPL_DEBUG_DBGBUS, TYPE: Type_CNPL_BLOCK_CNPL_DEBUG_DBGBUS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VB *****/
const ru_field_rec CNPL_DEBUG_DBGBUS_VB_FIELD =
{
    "VB",
#if RU_INCLUDE_DESC
    "",
    "debug vector\n",
#endif
    { CNPL_DEBUG_DBGBUS_VB_FIELD_MASK },
    0,
    { CNPL_DEBUG_DBGBUS_VB_FIELD_WIDTH },
    { CNPL_DEBUG_DBGBUS_VB_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_DEBUG_DBGBUS_FIELDS[] =
{
    &CNPL_DEBUG_DBGBUS_VB_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_DEBUG_DBGBUS *****/
const ru_reg_rec CNPL_DEBUG_DBGBUS_REG =
{
    "DEBUG_DBGBUS",
#if RU_INCLUDE_DESC
    "DBG_BUS Register",
    "the debug bus\n",
#endif
    { CNPL_DEBUG_DBGBUS_REG_OFFSET },
    0,
    0,
    249,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_DEBUG_DBGBUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CNPL_DEBUG_REQ_VEC, TYPE: Type_CNPL_BLOCK_CNPL_DEBUG_REQ_VEC
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: REQ *****/
const ru_field_rec CNPL_DEBUG_REQ_VEC_REQ_FIELD =
{
    "REQ",
#if RU_INCLUDE_DESC
    "",
    "still more commands for arbitration\n",
#endif
    { CNPL_DEBUG_REQ_VEC_REQ_FIELD_MASK },
    0,
    { CNPL_DEBUG_REQ_VEC_REQ_FIELD_WIDTH },
    { CNPL_DEBUG_REQ_VEC_REQ_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_DEBUG_REQ_VEC_FIELDS[] =
{
    &CNPL_DEBUG_REQ_VEC_REQ_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_DEBUG_REQ_VEC *****/
const ru_reg_rec CNPL_DEBUG_REQ_VEC_REG =
{
    "DEBUG_REQ_VEC",
#if RU_INCLUDE_DESC
    "REQUEST_VECTOR Register",
    "vector of all the requests of the clients (tx fifo not empty)\n",
#endif
    { CNPL_DEBUG_REQ_VEC_REG_OFFSET },
    0,
    0,
    250,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_DEBUG_REQ_VEC_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CNPL_DEBUG_POL_UP_ST, TYPE: Type_CNPL_BLOCK_CNPL_DEBUG_POL_UP_ST
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ITR_NUM *****/
const ru_field_rec CNPL_DEBUG_POL_UP_ST_ITR_NUM_FIELD =
{
    "ITR_NUM",
#if RU_INCLUDE_DESC
    "",
    "number of iteration we are(each represent 8192 cycles)\n",
#endif
    { CNPL_DEBUG_POL_UP_ST_ITR_NUM_FIELD_MASK },
    0,
    { CNPL_DEBUG_POL_UP_ST_ITR_NUM_FIELD_WIDTH },
    { CNPL_DEBUG_POL_UP_ST_ITR_NUM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: POL_NUM *****/
const ru_field_rec CNPL_DEBUG_POL_UP_ST_POL_NUM_FIELD =
{
    "POL_NUM",
#if RU_INCLUDE_DESC
    "",
    "number of policer now updated.\n(80 means we finished updated of all policers for this period)\n",
#endif
    { CNPL_DEBUG_POL_UP_ST_POL_NUM_FIELD_MASK },
    0,
    { CNPL_DEBUG_POL_UP_ST_POL_NUM_FIELD_WIDTH },
    { CNPL_DEBUG_POL_UP_ST_POL_NUM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_DEBUG_POL_UP_ST_FIELDS[] =
{
    &CNPL_DEBUG_POL_UP_ST_ITR_NUM_FIELD,
    &CNPL_DEBUG_POL_UP_ST_POL_NUM_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_DEBUG_POL_UP_ST *****/
const ru_reg_rec CNPL_DEBUG_POL_UP_ST_REG =
{
    "DEBUG_POL_UP_ST",
#if RU_INCLUDE_DESC
    "POLICER_UPDATE_STATUS 0..3 Register",
    "which counter is updated, and where are we in the period cycle\n",
#endif
    { CNPL_DEBUG_POL_UP_ST_REG_OFFSET },
    CNPL_DEBUG_POL_UP_ST_REG_RAM_CNT,
    4,
    251,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    CNPL_DEBUG_POL_UP_ST_FIELDS,
#endif
};

unsigned long CNPL_ADDRS[] =
{
    0x82948000,
};

static const ru_reg_rec *CNPL_REGS[] =
{
    &CNPL_MEMORY_DATA_REG,
    &CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_REG,
    &CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_REG,
    &CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_REG,
    &CNPL_POLICERS_CONFIGURATIONS_PL_CALC_TYPE_REG,
    &CNPL_POLICERS_CONFIGURATIONS_PL_SIZE_PROF_REG,
    &CNPL_POLICERS_CONFIGURATIONS_PER_UP_REG,
    &CNPL_SW_IF_SW_CMD_REG,
    &CNPL_SW_IF_SW_STAT_REG,
    &CNPL_SW_IF_SW_PL_RSLT_REG,
    &CNPL_SW_IF_SW_PL_RD_REG,
    &CNPL_SW_IF_SW_CNT_RD_REG,
    &CNPL_MISC_ARB_PRM_REG,
    &CNPL_MISC_COL_AWR_EN_REG,
    &CNPL_MISC_MEM_INIT_REG,
    &CNPL_PM_COUNTERS_ENG_CMDS_REG,
    &CNPL_PM_COUNTERS_CMD_WAIT_REG,
    &CNPL_PM_COUNTERS_TOT_CYC_REG,
    &CNPL_PM_COUNTERS_GNT_CYC_REG,
    &CNPL_PM_COUNTERS_ARB_CYC_REG,
    &CNPL_PM_COUNTERS_PL_UP_ERR_REG,
    &CNPL_PM_COUNTERS_GEN_CFG_REG,
    &CNPL_DEBUG_DBGSEL_REG,
    &CNPL_DEBUG_DBGBUS_REG,
    &CNPL_DEBUG_REQ_VEC_REG,
    &CNPL_DEBUG_POL_UP_ST_REG,
};

const ru_block_rec CNPL_BLOCK =
{
    "CNPL",
    CNPL_ADDRS,
    1,
    26,
    CNPL_REGS,
};
