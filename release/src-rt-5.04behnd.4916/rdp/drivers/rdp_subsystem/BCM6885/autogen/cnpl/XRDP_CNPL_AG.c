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
 * Register: NAME: CNPL_MAIN_MEMORY_MAIN_DATA, TYPE: Type_CNPL_BLOCK_CNPL_MAIN_MEMORY_MAIN_DATA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec CNPL_MAIN_MEMORY_MAIN_DATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "data\n",
#endif
    { CNPL_MAIN_MEMORY_MAIN_DATA_DATA_FIELD_MASK },
    0,
    { CNPL_MAIN_MEMORY_MAIN_DATA_DATA_FIELD_WIDTH },
    { CNPL_MAIN_MEMORY_MAIN_DATA_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_MAIN_MEMORY_MAIN_DATA_FIELDS[] =
{
    &CNPL_MAIN_MEMORY_MAIN_DATA_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_MAIN_MEMORY_MAIN_DATA *****/
const ru_reg_rec CNPL_MAIN_MEMORY_MAIN_DATA_REG =
{
    "MAIN_MEMORY_MAIN_DATA",
#if RU_INCLUDE_DESC
    "MEM_ENTRY 0..196607 Register",
    "mem_entry.\n\n",
#endif
    { CNPL_MAIN_MEMORY_MAIN_DATA_REG_OFFSET },
    CNPL_MAIN_MEMORY_MAIN_DATA_REG_RAM_CNT,
    4,
    305,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_MAIN_MEMORY_MAIN_DATA_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CNPL_NUM64_MEMORY_B64_DATA, TYPE: Type_CNPL_BLOCK_CNPL_64_MEMORY_B64_DATA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec CNPL_NUM64_MEMORY_B64_DATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "data\n",
#endif
    { CNPL_NUM64_MEMORY_B64_DATA_DATA_FIELD_MASK },
    0,
    { CNPL_NUM64_MEMORY_B64_DATA_DATA_FIELD_WIDTH },
    { CNPL_NUM64_MEMORY_B64_DATA_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_NUM64_MEMORY_B64_DATA_FIELDS[] =
{
    &CNPL_NUM64_MEMORY_B64_DATA_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_NUM64_MEMORY_B64_DATA *****/
const ru_reg_rec CNPL_NUM64_MEMORY_B64_DATA_REG =
{
    "NUM64_MEMORY_B64_DATA",
#if RU_INCLUDE_DESC
    "MEM_ENTRY 0..383 Register",
    "mem_entry.\n\n",
#endif
    { CNPL_NUM64_MEMORY_B64_DATA_REG_OFFSET },
    CNPL_NUM64_MEMORY_B64_DATA_REG_RAM_CNT,
    4,
    306,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_NUM64_MEMORY_B64_DATA_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CNPL_PLC_PROF_SEL_MEM_PLC_PROF_SEL_DATA, TYPE: Type_CNPL_BLOCK_CNPL_PLC_PROF_SEL_MEM_PLC_PROF_SEL_DATA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec CNPL_PLC_PROF_SEL_MEM_PLC_PROF_SEL_DATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "data\n",
#endif
    { CNPL_PLC_PROF_SEL_MEM_PLC_PROF_SEL_DATA_DATA_FIELD_MASK },
    0,
    { CNPL_PLC_PROF_SEL_MEM_PLC_PROF_SEL_DATA_DATA_FIELD_WIDTH },
    { CNPL_PLC_PROF_SEL_MEM_PLC_PROF_SEL_DATA_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_PLC_PROF_SEL_MEM_PLC_PROF_SEL_DATA_FIELDS[] =
{
    &CNPL_PLC_PROF_SEL_MEM_PLC_PROF_SEL_DATA_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_PLC_PROF_SEL_MEM_PLC_PROF_SEL_DATA *****/
const ru_reg_rec CNPL_PLC_PROF_SEL_MEM_PLC_PROF_SEL_DATA_REG =
{
    "PLC_PROF_SEL_MEM_PLC_PROF_SEL_DATA",
#if RU_INCLUDE_DESC
    "MEM_ENTRY 0..8191 Register",
    "mem_entry.\n\n",
#endif
    { CNPL_PLC_PROF_SEL_MEM_PLC_PROF_SEL_DATA_REG_OFFSET },
    CNPL_PLC_PROF_SEL_MEM_PLC_PROF_SEL_DATA_REG_RAM_CNT,
    4,
    307,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_PLC_PROF_SEL_MEM_PLC_PROF_SEL_DATA_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CNPL_PLC_PROFILES_MEM_PLC_PROFILES_DATA, TYPE: Type_CNPL_BLOCK_CNPL_PLC_PROFILES_MEM_PLC_PROFILES_DATA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA *****/
const ru_field_rec CNPL_PLC_PROFILES_MEM_PLC_PROFILES_DATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "",
    "data\n",
#endif
    { CNPL_PLC_PROFILES_MEM_PLC_PROFILES_DATA_DATA_FIELD_MASK },
    0,
    { CNPL_PLC_PROFILES_MEM_PLC_PROFILES_DATA_DATA_FIELD_WIDTH },
    { CNPL_PLC_PROFILES_MEM_PLC_PROFILES_DATA_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_PLC_PROFILES_MEM_PLC_PROFILES_DATA_FIELDS[] =
{
    &CNPL_PLC_PROFILES_MEM_PLC_PROFILES_DATA_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_PLC_PROFILES_MEM_PLC_PROFILES_DATA *****/
const ru_reg_rec CNPL_PLC_PROFILES_MEM_PLC_PROFILES_DATA_REG =
{
    "PLC_PROFILES_MEM_PLC_PROFILES_DATA",
#if RU_INCLUDE_DESC
    "MEM_ENTRY 0..1023 Register",
    "mem_entry.\n\n",
#endif
    { CNPL_PLC_PROFILES_MEM_PLC_PROFILES_DATA_REG_OFFSET },
    CNPL_PLC_PROFILES_MEM_PLC_PROFILES_DATA_REG_RAM_CNT,
    4,
    308,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_PLC_PROFILES_MEM_PLC_PROFILES_DATA_FIELDS,
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
    "counters group base address (in 8B resolution: 0 is 0x0, 1 is 0x8, ...)\n17b for 32K counters x3 (4B, dbl), and last bit for support of small bank_64 after the 32K.\n",
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
/***** Field: INC1_1 *****/
const ru_field_rec CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_INC1_1_FIELD =
{
    "INC1_1",
#if RU_INCLUDE_DESC
    "",
    "1:inc1 is fixed to 1 (to save register of runner)\n0:inc1 from bb input\nrelevant only for dbl(doublle=1) counters.\n",
#endif
    { CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_INC1_1_FIELD_MASK },
    0,
    { CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_INC1_1_FIELD_WIDTH },
    { CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_INC1_1_FIELD_SHIFT },
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
/***** Field: DBL_4BYTE_MOD *****/
const ru_field_rec CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_DBL_4BYTE_MOD_FIELD =
{
    "DBL_4BYTE_MOD",
#if RU_INCLUDE_DESC
    "",
    "0:28b+36b\n1:32b+32b\n",
#endif
    { CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_DBL_4BYTE_MOD_FIELD_MASK },
    0,
    { CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_DBL_4BYTE_MOD_FIELD_WIDTH },
    { CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_DBL_4BYTE_MOD_FIELD_SHIFT },
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
    &CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_DOUBLLE_FIELD,
    &CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_WRAP_FIELD,
    &CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_CLR_FIELD,
    &CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_INC1_1_FIELD,
    &CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_CN0_BYTS_FIELD,
    &CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_DBL_4BYTE_MOD_FIELD,
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
    309,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CNPL_COUNTERS_CONFIGURATIONS_CN_GLB_CTRL, TYPE: Type_CNPL_BLOCK_CNPL_COUNTERS_CONFIGURATIONS_CN_GLB_CTRL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MODE *****/
const ru_field_rec CNPL_COUNTERS_CONFIGURATIONS_CN_GLB_CTRL_MODE_FIELD =
{
    "MODE",
#if RU_INCLUDE_DESC
    "",
    "mode:\n0: normal cnpl counters\n1: QM counters\n2: QM watermark counters\nIn watermark counter -\n we get a counter index and a value in the command\n the engine reads current value\n if new value > current value, then write it to the memory\n clear is done like for normal counters through the sw_rd_cntr engine.\n\nthe parsing for QM counters/watermark counters is:\nQM 128b = {4b0, 4b0, cntr[14:0], byte_size[31:0], 1b0, pkt_size[31:0], 40b0 }\n\n",
#endif
    { CNPL_COUNTERS_CONFIGURATIONS_CN_GLB_CTRL_MODE_FIELD_MASK },
    0,
    { CNPL_COUNTERS_CONFIGURATIONS_CN_GLB_CTRL_MODE_FIELD_WIDTH },
    { CNPL_COUNTERS_CONFIGURATIONS_CN_GLB_CTRL_MODE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_COUNTERS_CONFIGURATIONS_CN_GLB_CTRL_FIELDS[] =
{
    &CNPL_COUNTERS_CONFIGURATIONS_CN_GLB_CTRL_MODE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_COUNTERS_CONFIGURATIONS_CN_GLB_CTRL *****/
const ru_reg_rec CNPL_COUNTERS_CONFIGURATIONS_CN_GLB_CTRL_REG =
{
    "COUNTERS_CONFIGURATIONS_CN_GLB_CTRL",
#if RU_INCLUDE_DESC
    "CNT_GLB_CTRL Register",
    "global control for counters\n",
#endif
    { CNPL_COUNTERS_CONFIGURATIONS_CN_GLB_CTRL_REG_OFFSET },
    0,
    0,
    310,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_COUNTERS_CONFIGURATIONS_CN_GLB_CTRL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_A, TYPE: Type_CNPL_BLOCK_CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_A
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: BK_BA *****/
const ru_field_rec CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_A_BK_BA_FIELD =
{
    "BK_BA",
#if RU_INCLUDE_DESC
    "",
    "buckets base address(in 16B resolution: 0 is 0x0, 1 is 0x10, ...), each addr correspnds to a memory entry.\n15b for 32K policers(4B, dbl), and last bit for support of small bank_64 after the 32K.\n",
#endif
    { CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_A_BK_BA_FIELD_MASK },
    0,
    { CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_A_BK_BA_FIELD_WIDTH },
    { CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_A_BK_BA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PA_BA *****/
const ru_field_rec CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_A_PA_BA_FIELD =
{
    "PA_BA",
#if RU_INCLUDE_DESC
    "",
    "params base address(in 8B resolution: 0 is 0x0, 1 is 0x8, ...).\nNot valid for lotus (since params are in 256 entry params mem)\n",
#endif
    { CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_A_PA_BA_FIELD_MASK },
    0,
    { CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_A_PA_BA_FIELD_WIDTH },
    { CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_A_PA_BA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DOUBLLE *****/
const ru_field_rec CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_A_DOUBLLE_FIELD =
{
    "DOUBLLE",
#if RU_INCLUDE_DESC
    "",
    "1:each policer is dual bucket\n0:each policer is single bucket\n",
#endif
    { CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_A_DOUBLLE_FIELD_MASK },
    0,
    { CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_A_DOUBLLE_FIELD_WIDTH },
    { CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_A_DOUBLLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FC *****/
const ru_field_rec CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_A_FC_FIELD =
{
    "FC",
#if RU_INCLUDE_DESC
    "",
    "1:the policers are flow control type\n0:the policers are normal\n\nWhen it is fc, the double field has no effect.\n",
#endif
    { CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_A_FC_FIELD_MASK },
    0,
    { CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_A_FC_FIELD_WIDTH },
    { CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_A_FC_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_A_FIELDS[] =
{
    &CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_A_BK_BA_FIELD,
    &CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_A_PA_BA_FIELD,
    &CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_A_DOUBLLE_FIELD,
    &CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_A_FC_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_A *****/
const ru_reg_rec CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_A_REG =
{
    "POLICERS_CONFIGURATIONS_PL_LOC_PROF_A",
#if RU_INCLUDE_DESC
    "PL_LOC_PROFILE_A 0..7 Register",
    "1st reg for location profiles\n",
#endif
    { CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_A_REG_OFFSET },
    CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_A_REG_RAM_CNT,
    4,
    311,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_A_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_B, TYPE: Type_CNPL_BLOCK_CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_B
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PL_ST *****/
const ru_field_rec CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_B_PL_ST_FIELD =
{
    "PL_ST",
#if RU_INCLUDE_DESC
    "",
    "Index of 1st policer of the group.\nIn default: pol_start>pol_end, so the group is not valid\n\nNote: we have total of 32K + 64(fc) policers, so 16b are needed.\n",
#endif
    { CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_B_PL_ST_FIELD_MASK },
    0,
    { CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_B_PL_ST_FIELD_WIDTH },
    { CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_B_PL_ST_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PL_END *****/
const ru_field_rec CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_B_PL_END_FIELD =
{
    "PL_END",
#if RU_INCLUDE_DESC
    "",
    "Index of last policer of the group.\nIn default: pol_start>pol_end, so the group is not valid\n\nNote: we have total of 32K + 64(fc) policers, so 16b are needed.\n",
#endif
    { CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_B_PL_END_FIELD_MASK },
    0,
    { CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_B_PL_END_FIELD_WIDTH },
    { CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_B_PL_END_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_B_FIELDS[] =
{
    &CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_B_PL_ST_FIELD,
    &CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_B_PL_END_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_B *****/
const ru_reg_rec CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_B_REG =
{
    "POLICERS_CONFIGURATIONS_PL_LOC_PROF_B",
#if RU_INCLUDE_DESC
    "PL_LOC_PROFILE_B 0..7 Register",
    "2nd reg for location profiles\n",
#endif
    { CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_B_REG_OFFSET },
    CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_B_REG_RAM_CNT,
    4,
    312,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_B_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_C, TYPE: Type_CNPL_BLOCK_CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_C
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: N *****/
const ru_field_rec CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_C_N_FIELD =
{
    "N",
#if RU_INCLUDE_DESC
    "",
    "shift left of 16k cycles quanta. 4b lsb only are valid.\n",
#endif
    { CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_C_N_FIELD_MASK },
    0,
    { CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_C_N_FIELD_WIDTH },
    { CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_C_N_FIELD_SHIFT },
    6,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_C_FIELDS[] =
{
    &CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_C_N_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_C *****/
const ru_reg_rec CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_C_REG =
{
    "POLICERS_CONFIGURATIONS_PL_LOC_PROF_C",
#if RU_INCLUDE_DESC
    "PL_LOC_PROFILE_C 0..7 Register",
    "3rd reg for location profiles\n",
#endif
    { CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_C_REG_OFFSET },
    CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_C_REG_RAM_CNT,
    4,
    313,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_C_FIELDS,
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
    "NOT USED in OLT.\n\nusing instead 1b in each entry of policer bucket mem.\n\ncalculation type register.\n0:green, yellow, red\n1:red, yellow, green\n\n8 registers support up to 256 policers.\n",
#endif
    { CNPL_POLICERS_CONFIGURATIONS_PL_CALC_TYPE_REG_OFFSET },
    CNPL_POLICERS_CONFIGURATIONS_PL_CALC_TYPE_REG_RAM_CNT,
    4,
    314,
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
    315,
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
    316,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    CNPL_POLICERS_CONFIGURATIONS_PER_UP_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CNPL_POLICERS_FC_THR_FC_THR_L, TYPE: Type_CNPL_BLOCK_CNPL_POLICERS_FC_THR_FC_THR_L
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec CNPL_POLICERS_FC_THR_FC_THR_L_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "32b value\n",
#endif
    { CNPL_POLICERS_FC_THR_FC_THR_L_VAL_FIELD_MASK },
    0,
    { CNPL_POLICERS_FC_THR_FC_THR_L_VAL_FIELD_WIDTH },
    { CNPL_POLICERS_FC_THR_FC_THR_L_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_POLICERS_FC_THR_FC_THR_L_FIELDS[] =
{
    &CNPL_POLICERS_FC_THR_FC_THR_L_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_POLICERS_FC_THR_FC_THR_L *****/
const ru_reg_rec CNPL_POLICERS_FC_THR_FC_THR_L_REG =
{
    "POLICERS_FC_THR_FC_THR_L",
#if RU_INCLUDE_DESC
    "FC_THR_L Register",
    "flow control thr 36b value.\nreg0 - LSB (32b)\nreg1 - MSB (4b)\n",
#endif
    { CNPL_POLICERS_FC_THR_FC_THR_L_REG_OFFSET },
    CNPL_POLICERS_FC_THR_FC_THR_L_REG_RAM_CNT,
    8,
    317,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_POLICERS_FC_THR_FC_THR_L_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CNPL_POLICERS_FC_THR_FC_THR_M, TYPE: Type_CNPL_BLOCK_CNPL_POLICERS_FC_THR_FC_THR_M
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec CNPL_POLICERS_FC_THR_FC_THR_M_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "32b value\n",
#endif
    { CNPL_POLICERS_FC_THR_FC_THR_M_VAL_FIELD_MASK },
    0,
    { CNPL_POLICERS_FC_THR_FC_THR_M_VAL_FIELD_WIDTH },
    { CNPL_POLICERS_FC_THR_FC_THR_M_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_POLICERS_FC_THR_FC_THR_M_FIELDS[] =
{
    &CNPL_POLICERS_FC_THR_FC_THR_M_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_POLICERS_FC_THR_FC_THR_M *****/
const ru_reg_rec CNPL_POLICERS_FC_THR_FC_THR_M_REG =
{
    "POLICERS_FC_THR_FC_THR_M",
#if RU_INCLUDE_DESC
    "FC_THR_M Register",
    "flow control thr 36b value.\nreg0 - LSB (32b)\nreg1 - MSB (4b)\n",
#endif
    { CNPL_POLICERS_FC_THR_FC_THR_M_REG_OFFSET },
    CNPL_POLICERS_FC_THR_FC_THR_M_REG_RAM_CNT,
    8,
    318,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_POLICERS_FC_THR_FC_THR_M_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CNPL_SW_IF_SW_CMD_H, TYPE: Type_CNPL_BLOCK_CNPL_SW_IF_SW_CMD_H
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec CNPL_SW_IF_SW_CMD_H_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "value of register\nTRIGGERS_HW\n\n",
#endif
    { CNPL_SW_IF_SW_CMD_H_VAL_FIELD_MASK },
    0,
    { CNPL_SW_IF_SW_CMD_H_VAL_FIELD_WIDTH },
    { CNPL_SW_IF_SW_CMD_H_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_SW_IF_SW_CMD_H_FIELDS[] =
{
    &CNPL_SW_IF_SW_CMD_H_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_SW_IF_SW_CMD_H *****/
const ru_reg_rec CNPL_SW_IF_SW_CMD_H_REG =
{
    "SW_IF_SW_CMD_H",
#if RU_INCLUDE_DESC
    "COMMAND_HIGH Register",
    "command register, high bytes.\nIf the cmd is short - less that 32b, only the high section should be configured.\nIf bigger - remaining data is configured in low register.\nNote:in this case (long cmd) CMD_L should be written first, and then CMD_H (trigerring the HW)\n\nsee detailed sw commands in cnpl spec: https://docs.google.com/document/d/1EqMmomFN7tTGASRFM6N6P9jDDb5y6ay-/edit?pli=1#heading=h.3whwml4 , sections 1.2.1-1.2.3\n",
#endif
    { CNPL_SW_IF_SW_CMD_H_REG_OFFSET },
    0,
    0,
    319,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_SW_IF_SW_CMD_H_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CNPL_SW_IF_SW_CMD_L, TYPE: Type_CNPL_BLOCK_CNPL_SW_IF_SW_CMD_L
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec CNPL_SW_IF_SW_CMD_L_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "value of register\nTRIGGERS_HW\n\n",
#endif
    { CNPL_SW_IF_SW_CMD_L_VAL_FIELD_MASK },
    0,
    { CNPL_SW_IF_SW_CMD_L_VAL_FIELD_WIDTH },
    { CNPL_SW_IF_SW_CMD_L_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_SW_IF_SW_CMD_L_FIELDS[] =
{
    &CNPL_SW_IF_SW_CMD_L_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_SW_IF_SW_CMD_L *****/
const ru_reg_rec CNPL_SW_IF_SW_CMD_L_REG =
{
    "SW_IF_SW_CMD_L",
#if RU_INCLUDE_DESC
    "COMMAND_LOW Register",
    "command register, low bytes.\nIf the cmd is short - less that 32b, only the high section should be configured.\nIf bigger - remaining data is configured in low register.\nNote:in this case (long cmd) CMD_L should be written first, and then CMD_H (trigerring the HW)\n\nsee detailed sw commands in cnpl spec: https://docs.google.com/document/d/1EqMmomFN7tTGASRFM6N6P9jDDb5y6ay-/edit?pli=1#heading=h.3whwml4 , sections 1.2.1-1.2.3\n",
#endif
    { CNPL_SW_IF_SW_CMD_L_REG_OFFSET },
    0,
    0,
    320,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_SW_IF_SW_CMD_L_FIELDS,
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
    "0: DONE (ready)\n1: PROC(not ready)\nCHANGED_BY_HW\n\n",
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
    "0: DONE (ready)\n1: PROC(not ready)\nCHANGED_BY_HW\n\n",
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
    "0: DONE (ready)\n1: PROC(not ready)\nCHANGED_BY_HW\n\n",
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
    321,
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
    322,
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
    "PL_RDX 0..2 Register",
    "rdata register - policer read command result. 3 register for 75 bits of memory line.\nreg0 holds {      bits[31: 0]},\nreg1 holds {      bits[63:32]},\nreg2 holds {210, bits[74:64]},\nnote - bit74 is the calc_type\n\nreminder:\nbkt0(active in single/dual/fc) is in bits [73:37],\nbkt1(active in dual) is in bits [36:0]\n\n\n",
#endif
    { CNPL_SW_IF_SW_PL_RD_REG_OFFSET },
    CNPL_SW_IF_SW_PL_RD_REG_RAM_CNT,
    4,
    323,
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
    "rdata register - counters read command result. 8 register for 32B batch.\nEach 64b memory line is captured by 2 regs, e.g. :\nline0: {reg1, reg0},\nline1: {reg3, reg2}, ...\n\nreminder:\nfor dual cntrs, the format is:\n{bytes, pkts}\nfor single:\n{cntr0, cntr1}\n",
#endif
    { CNPL_SW_IF_SW_CNT_RD_REG_OFFSET },
    CNPL_SW_IF_SW_CNT_RD_REG_RAM_CNT,
    4,
    324,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_SW_IF_SW_CNT_RD_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CNPL_SW_IF_SW_OVFLW_F_RD_USED, TYPE: Type_CNPL_BLOCK_CNPL_SW_IF_SW_OVFLW_F_RD_USED
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RD *****/
const ru_field_rec CNPL_SW_IF_SW_OVFLW_F_RD_USED_RD_FIELD =
{
    "RD",
#if RU_INCLUDE_DESC
    "",
    "value of read data\n",
#endif
    { CNPL_SW_IF_SW_OVFLW_F_RD_USED_RD_FIELD_MASK },
    0,
    { CNPL_SW_IF_SW_OVFLW_F_RD_USED_RD_FIELD_WIDTH },
    { CNPL_SW_IF_SW_OVFLW_F_RD_USED_RD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_SW_IF_SW_OVFLW_F_RD_USED_FIELDS[] =
{
    &CNPL_SW_IF_SW_OVFLW_F_RD_USED_RD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_SW_IF_SW_OVFLW_F_RD_USED *****/
const ru_reg_rec CNPL_SW_IF_SW_OVFLW_F_RD_USED_REG =
{
    "SW_IF_SW_OVFLW_F_RD_USED",
#if RU_INCLUDE_DESC
    "OVFLW_FIFO_RD_USED Register",
    "overflow fifo read used words\n",
#endif
    { CNPL_SW_IF_SW_OVFLW_F_RD_USED_REG_OFFSET },
    0,
    0,
    325,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_SW_IF_SW_OVFLW_F_RD_USED_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CNPL_SW_IF_SW_OVFLW_F_POP, TYPE: Type_CNPL_BLOCK_CNPL_SW_IF_SW_OVFLW_F_POP
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RD *****/
const ru_field_rec CNPL_SW_IF_SW_OVFLW_F_POP_RD_FIELD =
{
    "RD",
#if RU_INCLUDE_DESC
    "",
    "value of read data\n",
#endif
    { CNPL_SW_IF_SW_OVFLW_F_POP_RD_FIELD_MASK },
    0,
    { CNPL_SW_IF_SW_OVFLW_F_POP_RD_FIELD_WIDTH },
    { CNPL_SW_IF_SW_OVFLW_F_POP_RD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_SW_IF_SW_OVFLW_F_POP_FIELDS[] =
{
    &CNPL_SW_IF_SW_OVFLW_F_POP_RD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_SW_IF_SW_OVFLW_F_POP *****/
const ru_reg_rec CNPL_SW_IF_SW_OVFLW_F_POP_REG =
{
    "SW_IF_SW_OVFLW_F_POP",
#if RU_INCLUDE_DESC
    "OVFLW_FIFO_POP Register",
    "overflow fifo pop\n",
#endif
    { CNPL_SW_IF_SW_OVFLW_F_POP_REG_OFFSET },
    0,
    0,
    326,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_SW_IF_SW_OVFLW_F_POP_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CNPL_SW_IF_SW_OVFLW_F_THR, TYPE: Type_CNPL_BLOCK_CNPL_SW_IF_SW_OVFLW_F_THR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec CNPL_SW_IF_SW_OVFLW_F_THR_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "val\n",
#endif
    { CNPL_SW_IF_SW_OVFLW_F_THR_VAL_FIELD_MASK },
    0,
    { CNPL_SW_IF_SW_OVFLW_F_THR_VAL_FIELD_WIDTH },
    { CNPL_SW_IF_SW_OVFLW_F_THR_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_SW_IF_SW_OVFLW_F_THR_FIELDS[] =
{
    &CNPL_SW_IF_SW_OVFLW_F_THR_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_SW_IF_SW_OVFLW_F_THR *****/
const ru_reg_rec CNPL_SW_IF_SW_OVFLW_F_THR_REG =
{
    "SW_IF_SW_OVFLW_F_THR",
#if RU_INCLUDE_DESC
    "OVFLW_FIFO_THR Register",
    "overflow fifo threshold. When crossed, an interrupt is triggered.\n",
#endif
    { CNPL_SW_IF_SW_OVFLW_F_THR_REG_OFFSET },
    0,
    0,
    327,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_SW_IF_SW_OVFLW_F_THR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CNPL_SW_IF_SW_OVFLW_F_FULL_MOD, TYPE: Type_CNPL_BLOCK_CNPL_SW_IF_SW_OVFLW_F_FULL_MOD
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec CNPL_SW_IF_SW_OVFLW_F_FULL_MOD_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "val\n",
#endif
    { CNPL_SW_IF_SW_OVFLW_F_FULL_MOD_VAL_FIELD_MASK },
    0,
    { CNPL_SW_IF_SW_OVFLW_F_FULL_MOD_VAL_FIELD_WIDTH },
    { CNPL_SW_IF_SW_OVFLW_F_FULL_MOD_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_SW_IF_SW_OVFLW_F_FULL_MOD_FIELDS[] =
{
    &CNPL_SW_IF_SW_OVFLW_F_FULL_MOD_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_SW_IF_SW_OVFLW_F_FULL_MOD *****/
const ru_reg_rec CNPL_SW_IF_SW_OVFLW_F_FULL_MOD_REG =
{
    "SW_IF_SW_OVFLW_F_FULL_MOD",
#if RU_INCLUDE_DESC
    "OVFLW_FIFO_FULL_MODE Register",
    "Behavior when there is a push to the overflow fifo, which is already full:\n0 - backpressure\n1 - drop new entry\n\nIn any case, an error counter will be incremented.\n",
#endif
    { CNPL_SW_IF_SW_OVFLW_F_FULL_MOD_REG_OFFSET },
    0,
    0,
    328,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_SW_IF_SW_OVFLW_F_FULL_MOD_FIELDS,
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
    "0: fixed lower\n1: rr with fw (default)\n2: fixed higher\nTRIGGERS_HW\n\n",
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
    "11 possible options to select which address bit decodes to which cnpl memory the transaction goes.\n(32KB mem = 15b addr(in Bytes), 12b addr (in 8B words), 11 options to select 2 bits)\nValue of 0xF is kept to disable this feature (legacy mode), and all accesses will be directed to memory0.\nsince we have a few sizes of cnpl, please use values of [3..1], so -\nbits [0..8] are valid values for all cnpl types.\nNote: if mem_addr_bit_sel (in CNPL_BLOCK.CNPL_Misc.ARB_PRM) > 8, only half the address space is accessible.\nTRIGGERS_HW\n\n",
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
    329,
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
    330,
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
/***** Field: MAIN_INIT *****/
const ru_field_rec CNPL_MISC_MEM_INIT_MAIN_INIT_FIELD =
{
    "MAIN_INIT",
#if RU_INCLUDE_DESC
    "",
    "init for main mem (all banks)\nTRIGGERS_HW\n\n",
#endif
    { CNPL_MISC_MEM_INIT_MAIN_INIT_FIELD_MASK },
    0,
    { CNPL_MISC_MEM_INIT_MAIN_INIT_FIELD_WIDTH },
    { CNPL_MISC_MEM_INIT_MAIN_INIT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PLC_PROF_SEL_INIT *****/
const ru_field_rec CNPL_MISC_MEM_INIT_PLC_PROF_SEL_INIT_FIELD =
{
    "PLC_PROF_SEL_INIT",
#if RU_INCLUDE_DESC
    "",
    "init for policers profiles sel mem (all banks)\nTRIGGERS_HW\nNot relevant for counters cnpl\n\n",
#endif
    { CNPL_MISC_MEM_INIT_PLC_PROF_SEL_INIT_FIELD_MASK },
    0,
    { CNPL_MISC_MEM_INIT_PLC_PROF_SEL_INIT_FIELD_WIDTH },
    { CNPL_MISC_MEM_INIT_PLC_PROF_SEL_INIT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PLC_PROFILES_INIT *****/
const ru_field_rec CNPL_MISC_MEM_INIT_PLC_PROFILES_INIT_FIELD =
{
    "PLC_PROFILES_INIT",
#if RU_INCLUDE_DESC
    "",
    "init for policers profiles mem (all banks)\nTRIGGERS_HW\nNot relevant for counters cnpl\n\n",
#endif
    { CNPL_MISC_MEM_INIT_PLC_PROFILES_INIT_FIELD_MASK },
    0,
    { CNPL_MISC_MEM_INIT_PLC_PROFILES_INIT_FIELD_WIDTH },
    { CNPL_MISC_MEM_INIT_PLC_PROFILES_INIT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_MISC_MEM_INIT_FIELDS[] =
{
    &CNPL_MISC_MEM_INIT_MAIN_INIT_FIELD,
    &CNPL_MISC_MEM_INIT_PLC_PROF_SEL_INIT_FIELD,
    &CNPL_MISC_MEM_INIT_PLC_PROFILES_INIT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_MISC_MEM_INIT *****/
const ru_reg_rec CNPL_MISC_MEM_INIT_REG =
{
    "MISC_MEM_INIT",
#if RU_INCLUDE_DESC
    "MEM_INIT Register",
    "initialization for the various memories.\nwr 1 to start init, poll for 0 for done.\nPlease dont check rd value in reg test, since it changes automatically after memory is initialized.\n",
#endif
    { CNPL_MISC_MEM_INIT_REG_OFFSET },
    0,
    0,
    331,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
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
    332,
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
    333,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_PM_COUNTERS_CMD_WAIT_FIELDS,
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
    "POL_UP_ERR_CNTR 0..7 Register",
    "errors in policer update: the update period finished, and not all policers have been updated yet.\n",
#endif
    { CNPL_PM_COUNTERS_PL_UP_ERR_REG_OFFSET },
    CNPL_PM_COUNTERS_PL_UP_ERR_REG_RAM_CNT,
    4,
    334,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_PM_COUNTERS_PL_UP_ERR_FIELDS,
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
    "TOT_CYC_CNTR 0..20 Register",
    "Number of cycles from last read clear.\nreg 1(0x0): mem0\nreg 2(0x4): mem1\nreg 3(0x8): mem2\nreg 4(0xC): mem3\n...\n",
#endif
    { CNPL_PM_COUNTERS_TOT_CYC_REG_OFFSET },
    CNPL_PM_COUNTERS_TOT_CYC_REG_RAM_CNT,
    4,
    335,
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
    "GNT_CYC_CNTR 0..20 Register",
    "Number of cycles that there was gnt from memX arbiter.\nreg 1(0x0): mem0\nreg 2(0x4): mem1\nreg 3(0x8): mem2\nreg 4(0xC): mem3\n...\n\n\n",
#endif
    { CNPL_PM_COUNTERS_GNT_CYC_REG_OFFSET },
    CNPL_PM_COUNTERS_GNT_CYC_REG_RAM_CNT,
    4,
    336,
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
    "ARB_CYC_CNTR 0..20 Register",
    "Number of cycles that there was gnt with request of more than one agent.\nreg 1(0x0): mem0\nreg 2(0x4): mem1\nreg 3(0x8): mem2\nreg 4(0xC): mem3\n...\n",
#endif
    { CNPL_PM_COUNTERS_ARB_CYC_REG_OFFSET },
    CNPL_PM_COUNTERS_ARB_CYC_REG_RAM_CNT,
    4,
    337,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_PM_COUNTERS_ARB_CYC_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CNPL_PM_COUNTERS_ERR_BAD_ENTRY, TYPE: Type_CNPL_BLOCK_CNPL_PM_COUNTERS_ERR_BAD_ENTRY
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec CNPL_PM_COUNTERS_ERR_BAD_ENTRY_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "value\n",
#endif
    { CNPL_PM_COUNTERS_ERR_BAD_ENTRY_VAL_FIELD_MASK },
    0,
    { CNPL_PM_COUNTERS_ERR_BAD_ENTRY_VAL_FIELD_WIDTH },
    { CNPL_PM_COUNTERS_ERR_BAD_ENTRY_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_PM_COUNTERS_ERR_BAD_ENTRY_FIELDS[] =
{
    &CNPL_PM_COUNTERS_ERR_BAD_ENTRY_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_PM_COUNTERS_ERR_BAD_ENTRY *****/
const ru_reg_rec CNPL_PM_COUNTERS_ERR_BAD_ENTRY_REG =
{
    "PM_COUNTERS_ERR_BAD_ENTRY",
#if RU_INCLUDE_DESC
    "ERR_BAD_ENTRY Register",
    "counts when BA+offset exceeds the max memory entry (e.g.: 32K+64)\n",
#endif
    { CNPL_PM_COUNTERS_ERR_BAD_ENTRY_REG_OFFSET },
    0,
    0,
    338,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_PM_COUNTERS_ERR_BAD_ENTRY_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CNPL_PM_COUNTERS_OVFLW_F_PUSH_ERR, TYPE: Type_CNPL_BLOCK_CNPL_PM_COUNTERS_OVFLW_F_PUSH_ERR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VAL *****/
const ru_field_rec CNPL_PM_COUNTERS_OVFLW_F_PUSH_ERR_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "",
    "value\n",
#endif
    { CNPL_PM_COUNTERS_OVFLW_F_PUSH_ERR_VAL_FIELD_MASK },
    0,
    { CNPL_PM_COUNTERS_OVFLW_F_PUSH_ERR_VAL_FIELD_WIDTH },
    { CNPL_PM_COUNTERS_OVFLW_F_PUSH_ERR_VAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_PM_COUNTERS_OVFLW_F_PUSH_ERR_FIELDS[] =
{
    &CNPL_PM_COUNTERS_OVFLW_F_PUSH_ERR_VAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_PM_COUNTERS_OVFLW_F_PUSH_ERR *****/
const ru_reg_rec CNPL_PM_COUNTERS_OVFLW_F_PUSH_ERR_REG =
{
    "PM_COUNTERS_OVFLW_F_PUSH_ERR",
#if RU_INCLUDE_DESC
    "OVFLW_F_PUSH_ERR Register",
    "counts when hw has a push to counter-overflow fifo and it was full(should not happen)\n",
#endif
    { CNPL_PM_COUNTERS_OVFLW_F_PUSH_ERR_REG_OFFSET },
    0,
    0,
    339,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_PM_COUNTERS_OVFLW_F_PUSH_ERR_FIELDS,
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
    340,
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
    341,
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
    342,
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
    "vector of all the requests of the engines\n",
#endif
    { CNPL_DEBUG_REQ_VEC_REG_OFFSET },
    0,
    0,
    343,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_DEBUG_REQ_VEC_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CNPL_DEBUG_ERR_BAD_ENTRY_DATA, TYPE: Type_CNPL_BLOCK_CNPL_DEBUG_ERR_BAD_ENTRY_DATA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: REQ *****/
const ru_field_rec CNPL_DEBUG_ERR_BAD_ENTRY_DATA_REQ_FIELD =
{
    "REQ",
#if RU_INCLUDE_DESC
    "",
    "still more commands for arbitration\n",
#endif
    { CNPL_DEBUG_ERR_BAD_ENTRY_DATA_REQ_FIELD_MASK },
    0,
    { CNPL_DEBUG_ERR_BAD_ENTRY_DATA_REQ_FIELD_WIDTH },
    { CNPL_DEBUG_ERR_BAD_ENTRY_DATA_REQ_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_DEBUG_ERR_BAD_ENTRY_DATA_FIELDS[] =
{
    &CNPL_DEBUG_ERR_BAD_ENTRY_DATA_REQ_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_DEBUG_ERR_BAD_ENTRY_DATA *****/
const ru_reg_rec CNPL_DEBUG_ERR_BAD_ENTRY_DATA_REG =
{
    "DEBUG_ERR_BAD_ENTRY_DATA",
#if RU_INCLUDE_DESC
    "ERR_BAD_ENTRY_DATA Register",
    "err_bad_entry data bus:\n{ loc_prof_ba[15:4],\ncntr_offset,\ncntr_group\t  };\n\nHold info for last error\n",
#endif
    { CNPL_DEBUG_ERR_BAD_ENTRY_DATA_REG_OFFSET },
    0,
    0,
    344,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_DEBUG_ERR_BAD_ENTRY_DATA_FIELDS,
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
    "POLICER_UPDATE_STATUS 0..7 Register",
    "which counter is updated, and where are we in the period cycle\n",
#endif
    { CNPL_DEBUG_POL_UP_ST_REG_OFFSET },
    CNPL_DEBUG_POL_UP_ST_REG_RAM_CNT,
    4,
    345,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    CNPL_DEBUG_POL_UP_ST_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CNPL_INTR_CTRL_ISR, TYPE: Type_CNPL_BLOCK_CNPL_INTR_CTRL_ISR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MULT_MATCH *****/
const ru_field_rec CNPL_INTR_CTRL_ISR_MULT_MATCH_FIELD =
{
    "MULT_MATCH",
#if RU_INCLUDE_DESC
    "",
    "During the search process same key was found a valid in multiple engines.\n",
#endif
    { CNPL_INTR_CTRL_ISR_MULT_MATCH_FIELD_MASK },
    0,
    { CNPL_INTR_CTRL_ISR_MULT_MATCH_FIELD_WIDTH },
    { CNPL_INTR_CTRL_ISR_MULT_MATCH_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_INTR_CTRL_ISR_FIELDS[] =
{
    &CNPL_INTR_CTRL_ISR_MULT_MATCH_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_INTR_CTRL_ISR *****/
const ru_reg_rec CNPL_INTR_CTRL_ISR_REG =
{
    "INTR_CTRL_ISR",
#if RU_INCLUDE_DESC
    "INTERRUPT_STATUS_REGISTER Register",
    "This register contains the current active hash interrupts. Each asserted bit represents an active interrupt source. The interrupt remains active until the software clears it by writing 1 to the corresponding bit.\n",
#endif
    { CNPL_INTR_CTRL_ISR_REG_OFFSET },
    0,
    0,
    346,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_INTR_CTRL_ISR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CNPL_INTR_CTRL_ISM, TYPE: Type_CNPL_BLOCK_CNPL_INTR_CTRL_ISM
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ISM *****/
const ru_field_rec CNPL_INTR_CTRL_ISM_ISM_FIELD =
{
    "ISM",
#if RU_INCLUDE_DESC
    "",
    "Status Masked of corresponding interrupt source in the ISR\n",
#endif
    { CNPL_INTR_CTRL_ISM_ISM_FIELD_MASK },
    0,
    { CNPL_INTR_CTRL_ISM_ISM_FIELD_WIDTH },
    { CNPL_INTR_CTRL_ISM_ISM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_INTR_CTRL_ISM_FIELDS[] =
{
    &CNPL_INTR_CTRL_ISM_ISM_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_INTR_CTRL_ISM *****/
const ru_reg_rec CNPL_INTR_CTRL_ISM_REG =
{
    "INTR_CTRL_ISM",
#if RU_INCLUDE_DESC
    "INTERRUPT_STATUS_MASKED_REGISTER Register",
    "This register provides only the  enabled interrupts for each of the interrupt sources depicted in the ISR register.\n",
#endif
    { CNPL_INTR_CTRL_ISM_REG_OFFSET },
    0,
    0,
    347,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_INTR_CTRL_ISM_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CNPL_INTR_CTRL_IER, TYPE: Type_CNPL_BLOCK_CNPL_INTR_CTRL_IER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: IEM *****/
const ru_field_rec CNPL_INTR_CTRL_IER_IEM_FIELD =
{
    "IEM",
#if RU_INCLUDE_DESC
    "",
    "Each bit in the mask controls the corresponding interrupt source in the IER\nCHANGED_BY_HW\n\n",
#endif
    { CNPL_INTR_CTRL_IER_IEM_FIELD_MASK },
    0,
    { CNPL_INTR_CTRL_IER_IEM_FIELD_WIDTH },
    { CNPL_INTR_CTRL_IER_IEM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_INTR_CTRL_IER_FIELDS[] =
{
    &CNPL_INTR_CTRL_IER_IEM_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_INTR_CTRL_IER *****/
const ru_reg_rec CNPL_INTR_CTRL_IER_REG =
{
    "INTR_CTRL_IER",
#if RU_INCLUDE_DESC
    "INTERRUPT_ENABLE_REGISTER Register",
    "This register provides an enable mask for each of the interrupt sources depicted in the ISR register.\n",
#endif
    { CNPL_INTR_CTRL_IER_REG_OFFSET },
    0,
    0,
    348,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_INTR_CTRL_IER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: CNPL_INTR_CTRL_ITR, TYPE: Type_CNPL_BLOCK_CNPL_INTR_CTRL_ITR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: IST *****/
const ru_field_rec CNPL_INTR_CTRL_ITR_IST_FIELD =
{
    "IST",
#if RU_INCLUDE_DESC
    "",
    "Each bit in the mask tests the corresponding interrupt source in the ISR\nCHANGED_BY_HW\n\n",
#endif
    { CNPL_INTR_CTRL_ITR_IST_FIELD_MASK },
    0,
    { CNPL_INTR_CTRL_ITR_IST_FIELD_WIDTH },
    { CNPL_INTR_CTRL_ITR_IST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_INTR_CTRL_ITR_FIELDS[] =
{
    &CNPL_INTR_CTRL_ITR_IST_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: CNPL_INTR_CTRL_ITR *****/
const ru_reg_rec CNPL_INTR_CTRL_ITR_REG =
{
    "INTR_CTRL_ITR",
#if RU_INCLUDE_DESC
    "INTERRUPT_TEST_REGISTER Register",
    "This register enables testing by simulating interrupt sources. When the software sets a bit in the ITR, the corresponding bit in the ISR shows an active interrupt. The interrupt remains active until software clears the bit in the ITR\n",
#endif
    { CNPL_INTR_CTRL_ITR_REG_OFFSET },
    0,
    0,
    349,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_INTR_CTRL_ITR_FIELDS,
#endif
};

unsigned long CNPL_ADDRS[] =
{
    0xB4800000,
    0xB4900000,
    0xB4A00000,
};

static const ru_reg_rec *CNPL_REGS[] =
{
    &CNPL_MAIN_MEMORY_MAIN_DATA_REG,
    &CNPL_NUM64_MEMORY_B64_DATA_REG,
    &CNPL_PLC_PROF_SEL_MEM_PLC_PROF_SEL_DATA_REG,
    &CNPL_PLC_PROFILES_MEM_PLC_PROFILES_DATA_REG,
    &CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_REG,
    &CNPL_COUNTERS_CONFIGURATIONS_CN_GLB_CTRL_REG,
    &CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_A_REG,
    &CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_B_REG,
    &CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF_C_REG,
    &CNPL_POLICERS_CONFIGURATIONS_PL_CALC_TYPE_REG,
    &CNPL_POLICERS_CONFIGURATIONS_PL_SIZE_PROF_REG,
    &CNPL_POLICERS_CONFIGURATIONS_PER_UP_REG,
    &CNPL_POLICERS_FC_THR_FC_THR_L_REG,
    &CNPL_POLICERS_FC_THR_FC_THR_M_REG,
    &CNPL_SW_IF_SW_CMD_H_REG,
    &CNPL_SW_IF_SW_CMD_L_REG,
    &CNPL_SW_IF_SW_STAT_REG,
    &CNPL_SW_IF_SW_PL_RSLT_REG,
    &CNPL_SW_IF_SW_PL_RD_REG,
    &CNPL_SW_IF_SW_CNT_RD_REG,
    &CNPL_SW_IF_SW_OVFLW_F_RD_USED_REG,
    &CNPL_SW_IF_SW_OVFLW_F_POP_REG,
    &CNPL_SW_IF_SW_OVFLW_F_THR_REG,
    &CNPL_SW_IF_SW_OVFLW_F_FULL_MOD_REG,
    &CNPL_MISC_ARB_PRM_REG,
    &CNPL_MISC_COL_AWR_EN_REG,
    &CNPL_MISC_MEM_INIT_REG,
    &CNPL_PM_COUNTERS_ENG_CMDS_REG,
    &CNPL_PM_COUNTERS_CMD_WAIT_REG,
    &CNPL_PM_COUNTERS_PL_UP_ERR_REG,
    &CNPL_PM_COUNTERS_TOT_CYC_REG,
    &CNPL_PM_COUNTERS_GNT_CYC_REG,
    &CNPL_PM_COUNTERS_ARB_CYC_REG,
    &CNPL_PM_COUNTERS_ERR_BAD_ENTRY_REG,
    &CNPL_PM_COUNTERS_OVFLW_F_PUSH_ERR_REG,
    &CNPL_PM_COUNTERS_GEN_CFG_REG,
    &CNPL_DEBUG_DBGSEL_REG,
    &CNPL_DEBUG_DBGBUS_REG,
    &CNPL_DEBUG_REQ_VEC_REG,
    &CNPL_DEBUG_ERR_BAD_ENTRY_DATA_REG,
    &CNPL_DEBUG_POL_UP_ST_REG,
    &CNPL_INTR_CTRL_ISR_REG,
    &CNPL_INTR_CTRL_ISM_REG,
    &CNPL_INTR_CTRL_IER_REG,
    &CNPL_INTR_CTRL_ITR_REG,
};

const ru_block_rec CNPL_BLOCK =
{
    "CNPL",
#if RU_INCLUDE_SIZE
    { 0x000FFFF0 },
#endif
    CNPL_ADDRS,
    3,
    45,
    CNPL_REGS,
};
