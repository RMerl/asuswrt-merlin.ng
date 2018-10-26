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
 * Field: CNPL_MEMORY_DATA_DATA
 ******************************************************************************/
const ru_field_rec CNPL_MEMORY_DATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "data",
    "data",
#endif
    CNPL_MEMORY_DATA_DATA_FIELD_MASK,
    0,
    CNPL_MEMORY_DATA_DATA_FIELD_WIDTH,
    CNPL_MEMORY_DATA_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_BA
 ******************************************************************************/
const ru_field_rec CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_BA_FIELD =
{
    "BA",
#if RU_INCLUDE_DESC
    "base_address",
    "counters group base address (in 8B resolution: 0 is 0x0, 1 is 0x8, ...)",
#endif
    CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_BA_FIELD_MASK,
    0,
    CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_BA_FIELD_WIDTH,
    CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_BA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_CN0_BYTS
 ******************************************************************************/
const ru_field_rec CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_CN0_BYTS_FIELD =
{
    "CN0_BYTS",
#if RU_INCLUDE_DESC
    "cntr0_bytes",
    "number of bytes that will hold each sub-counter."
    "0: 1B"
    "1: 2B"
    "2: 4B",
#endif
    CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_CN0_BYTS_FIELD_MASK,
    0,
    CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_CN0_BYTS_FIELD_WIDTH,
    CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_CN0_BYTS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_DOUBLLE
 ******************************************************************************/
const ru_field_rec CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_DOUBLLE_FIELD =
{
    "DOUBLLE",
#if RU_INCLUDE_DESC
    "doublle",
    "1:each counter of the group is double sub-cntr"
    "0:each counter of the group is single",
#endif
    CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_DOUBLLE_FIELD_MASK,
    0,
    CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_DOUBLLE_FIELD_WIDTH,
    CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_DOUBLLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_WRAP
 ******************************************************************************/
const ru_field_rec CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_WRAP_FIELD =
{
    "WRAP",
#if RU_INCLUDE_DESC
    "wrap",
    "1:wrap at max value"
    "0:freeze at max value",
#endif
    CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_WRAP_FIELD_MASK,
    0,
    CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_WRAP_FIELD_WIDTH,
    CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_WRAP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_CLR
 ******************************************************************************/
const ru_field_rec CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_CLR_FIELD =
{
    "CLR",
#if RU_INCLUDE_DESC
    "clear",
    "1:read clear when reading"
    "0:read no-clear when reading",
#endif
    CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_CLR_FIELD_MASK,
    0,
    CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_CLR_FIELD_WIDTH,
    CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_CLR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_RESERVED0
 ******************************************************************************/
const ru_field_rec CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_RESERVED0_FIELD_MASK,
    0,
    CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_RESERVED0_FIELD_WIDTH,
    CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_BK_BA
 ******************************************************************************/
const ru_field_rec CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_BK_BA_FIELD =
{
    "BK_BA",
#if RU_INCLUDE_DESC
    "bkts_base_address",
    "buckets base address(in 8B resolution: 0 is 0x0, 1 is 0x8, ...)",
#endif
    CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_BK_BA_FIELD_MASK,
    0,
    CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_BK_BA_FIELD_WIDTH,
    CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_BK_BA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_PA_BA
 ******************************************************************************/
const ru_field_rec CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_PA_BA_FIELD =
{
    "PA_BA",
#if RU_INCLUDE_DESC
    "params_base_address",
    "params base address(in 8B resolution: 0 is 0x0, 1 is 0x8, ...)",
#endif
    CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_PA_BA_FIELD_MASK,
    0,
    CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_PA_BA_FIELD_WIDTH,
    CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_PA_BA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_DOUBLLE
 ******************************************************************************/
const ru_field_rec CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_DOUBLLE_FIELD =
{
    "DOUBLLE",
#if RU_INCLUDE_DESC
    "doublle",
    "1:each policer is dual bucket"
    "0:each policer is single bucket",
#endif
    CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_DOUBLLE_FIELD_MASK,
    0,
    CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_DOUBLLE_FIELD_WIDTH,
    CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_DOUBLLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_RESERVED0
 ******************************************************************************/
const ru_field_rec CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_RESERVED0_FIELD_MASK,
    0,
    CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_RESERVED0_FIELD_WIDTH,
    CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_PL_ST
 ******************************************************************************/
const ru_field_rec CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_PL_ST_FIELD =
{
    "PL_ST",
#if RU_INCLUDE_DESC
    "pl_start",
    "Index of 1st policer of the group.",
#endif
    CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_PL_ST_FIELD_MASK,
    0,
    CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_PL_ST_FIELD_WIDTH,
    CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_PL_ST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_PL_END
 ******************************************************************************/
const ru_field_rec CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_PL_END_FIELD =
{
    "PL_END",
#if RU_INCLUDE_DESC
    "pl_end",
    "Index of last policer of the group.",
#endif
    CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_PL_END_FIELD_MASK,
    0,
    CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_PL_END_FIELD_WIDTH,
    CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_PL_END_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_RESERVED0
 ******************************************************************************/
const ru_field_rec CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_RESERVED0_FIELD_MASK,
    0,
    CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_RESERVED0_FIELD_WIDTH,
    CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CNPL_POLICERS_CONFIGURATIONS_PL_CALC_TYPE_VEC
 ******************************************************************************/
const ru_field_rec CNPL_POLICERS_CONFIGURATIONS_PL_CALC_TYPE_VEC_FIELD =
{
    "VEC",
#if RU_INCLUDE_DESC
    "vector",
    "32b vector for 32 policers",
#endif
    CNPL_POLICERS_CONFIGURATIONS_PL_CALC_TYPE_VEC_FIELD_MASK,
    0,
    CNPL_POLICERS_CONFIGURATIONS_PL_CALC_TYPE_VEC_FIELD_WIDTH,
    CNPL_POLICERS_CONFIGURATIONS_PL_CALC_TYPE_VEC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CNPL_POLICERS_CONFIGURATIONS_PER_UP_N
 ******************************************************************************/
const ru_field_rec CNPL_POLICERS_CONFIGURATIONS_PER_UP_N_FIELD =
{
    "N",
#if RU_INCLUDE_DESC
    "N",
    "period in 8k cycles quanta (16.384us for 500MHz)",
#endif
    CNPL_POLICERS_CONFIGURATIONS_PER_UP_N_FIELD_MASK,
    0,
    CNPL_POLICERS_CONFIGURATIONS_PER_UP_N_FIELD_WIDTH,
    CNPL_POLICERS_CONFIGURATIONS_PER_UP_N_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CNPL_POLICERS_CONFIGURATIONS_PER_UP_EN
 ******************************************************************************/
const ru_field_rec CNPL_POLICERS_CONFIGURATIONS_PER_UP_EN_FIELD =
{
    "EN",
#if RU_INCLUDE_DESC
    "enable",
    "1:enable periodic update"
    "0:disable periodic update",
#endif
    CNPL_POLICERS_CONFIGURATIONS_PER_UP_EN_FIELD_MASK,
    0,
    CNPL_POLICERS_CONFIGURATIONS_PER_UP_EN_FIELD_WIDTH,
    CNPL_POLICERS_CONFIGURATIONS_PER_UP_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CNPL_POLICERS_CONFIGURATIONS_PER_UP_RESERVED0
 ******************************************************************************/
const ru_field_rec CNPL_POLICERS_CONFIGURATIONS_PER_UP_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    CNPL_POLICERS_CONFIGURATIONS_PER_UP_RESERVED0_FIELD_MASK,
    0,
    CNPL_POLICERS_CONFIGURATIONS_PER_UP_RESERVED0_FIELD_WIDTH,
    CNPL_POLICERS_CONFIGURATIONS_PER_UP_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CNPL_MISC_ARB_PRM_SW_PRIO
 ******************************************************************************/
const ru_field_rec CNPL_MISC_ARB_PRM_SW_PRIO_FIELD =
{
    "SW_PRIO",
#if RU_INCLUDE_DESC
    "sw_prio",
    "0: fixed lower"
    "1: rr with fw (default)"
    "2: fixed higher",
#endif
    CNPL_MISC_ARB_PRM_SW_PRIO_FIELD_MASK,
    0,
    CNPL_MISC_ARB_PRM_SW_PRIO_FIELD_WIDTH,
    CNPL_MISC_ARB_PRM_SW_PRIO_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CNPL_MISC_ARB_PRM_RESERVED0
 ******************************************************************************/
const ru_field_rec CNPL_MISC_ARB_PRM_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    CNPL_MISC_ARB_PRM_RESERVED0_FIELD_MASK,
    0,
    CNPL_MISC_ARB_PRM_RESERVED0_FIELD_WIDTH,
    CNPL_MISC_ARB_PRM_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CNPL_MISC_COL_AWR_EN_EN
 ******************************************************************************/
const ru_field_rec CNPL_MISC_COL_AWR_EN_EN_FIELD =
{
    "EN",
#if RU_INCLUDE_DESC
    "en",
    "0: dis"
    "1: en",
#endif
    CNPL_MISC_COL_AWR_EN_EN_FIELD_MASK,
    0,
    CNPL_MISC_COL_AWR_EN_EN_FIELD_WIDTH,
    CNPL_MISC_COL_AWR_EN_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CNPL_MISC_COL_AWR_EN_RESERVED0
 ******************************************************************************/
const ru_field_rec CNPL_MISC_COL_AWR_EN_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    CNPL_MISC_COL_AWR_EN_RESERVED0_FIELD_MASK,
    0,
    CNPL_MISC_COL_AWR_EN_RESERVED0_FIELD_WIDTH,
    CNPL_MISC_COL_AWR_EN_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CNPL_SW_IF_SW_CMD_VAL
 ******************************************************************************/
const ru_field_rec CNPL_SW_IF_SW_CMD_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "value",
    "value of register",
#endif
    CNPL_SW_IF_SW_CMD_VAL_FIELD_MASK,
    0,
    CNPL_SW_IF_SW_CMD_VAL_FIELD_WIDTH,
    CNPL_SW_IF_SW_CMD_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CNPL_SW_IF_SW_STAT_CN_RD_ST
 ******************************************************************************/
const ru_field_rec CNPL_SW_IF_SW_STAT_CN_RD_ST_FIELD =
{
    "CN_RD_ST",
#if RU_INCLUDE_DESC
    "cnt_rd_status",
    "0: DONE (ready)"
    "1: PROC(not ready)",
#endif
    CNPL_SW_IF_SW_STAT_CN_RD_ST_FIELD_MASK,
    0,
    CNPL_SW_IF_SW_STAT_CN_RD_ST_FIELD_WIDTH,
    CNPL_SW_IF_SW_STAT_CN_RD_ST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: CNPL_SW_IF_SW_STAT_PL_PLC_ST
 ******************************************************************************/
const ru_field_rec CNPL_SW_IF_SW_STAT_PL_PLC_ST_FIELD =
{
    "PL_PLC_ST",
#if RU_INCLUDE_DESC
    "pl_plc_status",
    "0: DONE (ready)"
    "1: PROC(not ready)",
#endif
    CNPL_SW_IF_SW_STAT_PL_PLC_ST_FIELD_MASK,
    0,
    CNPL_SW_IF_SW_STAT_PL_PLC_ST_FIELD_WIDTH,
    CNPL_SW_IF_SW_STAT_PL_PLC_ST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: CNPL_SW_IF_SW_STAT_PL_RD_ST
 ******************************************************************************/
const ru_field_rec CNPL_SW_IF_SW_STAT_PL_RD_ST_FIELD =
{
    "PL_RD_ST",
#if RU_INCLUDE_DESC
    "pl_rd_status",
    "0: DONE (ready)"
    "1: PROC(not ready)",
#endif
    CNPL_SW_IF_SW_STAT_PL_RD_ST_FIELD_MASK,
    0,
    CNPL_SW_IF_SW_STAT_PL_RD_ST_FIELD_WIDTH,
    CNPL_SW_IF_SW_STAT_PL_RD_ST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: CNPL_SW_IF_SW_STAT_RESERVED0
 ******************************************************************************/
const ru_field_rec CNPL_SW_IF_SW_STAT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    CNPL_SW_IF_SW_STAT_RESERVED0_FIELD_MASK,
    0,
    CNPL_SW_IF_SW_STAT_RESERVED0_FIELD_WIDTH,
    CNPL_SW_IF_SW_STAT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: CNPL_SW_IF_SW_PL_RSLT_RESERVED0
 ******************************************************************************/
const ru_field_rec CNPL_SW_IF_SW_PL_RSLT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    CNPL_SW_IF_SW_PL_RSLT_RESERVED0_FIELD_MASK,
    0,
    CNPL_SW_IF_SW_PL_RSLT_RESERVED0_FIELD_WIDTH,
    CNPL_SW_IF_SW_PL_RSLT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: CNPL_SW_IF_SW_PL_RSLT_COL
 ******************************************************************************/
const ru_field_rec CNPL_SW_IF_SW_PL_RSLT_COL_FIELD =
{
    "COL",
#if RU_INCLUDE_DESC
    "color",
    "red, yellow, green, non-active",
#endif
    CNPL_SW_IF_SW_PL_RSLT_COL_FIELD_MASK,
    0,
    CNPL_SW_IF_SW_PL_RSLT_COL_FIELD_WIDTH,
    CNPL_SW_IF_SW_PL_RSLT_COL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: CNPL_SW_IF_SW_PL_RD_RD
 ******************************************************************************/
const ru_field_rec CNPL_SW_IF_SW_PL_RD_RD_FIELD =
{
    "RD",
#if RU_INCLUDE_DESC
    "read_data",
    "value of read data",
#endif
    CNPL_SW_IF_SW_PL_RD_RD_FIELD_MASK,
    0,
    CNPL_SW_IF_SW_PL_RD_RD_FIELD_WIDTH,
    CNPL_SW_IF_SW_PL_RD_RD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: CNPL_SW_IF_SW_CNT_RD_RD
 ******************************************************************************/
const ru_field_rec CNPL_SW_IF_SW_CNT_RD_RD_FIELD =
{
    "RD",
#if RU_INCLUDE_DESC
    "read_data",
    "value of read data",
#endif
    CNPL_SW_IF_SW_CNT_RD_RD_FIELD_MASK,
    0,
    CNPL_SW_IF_SW_CNT_RD_RD_FIELD_WIDTH,
    CNPL_SW_IF_SW_CNT_RD_RD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: CNPL_PM_COUNTERS_ENG_CMDS_VAL
 ******************************************************************************/
const ru_field_rec CNPL_PM_COUNTERS_ENG_CMDS_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "value",
    "value",
#endif
    CNPL_PM_COUNTERS_ENG_CMDS_VAL_FIELD_MASK,
    0,
    CNPL_PM_COUNTERS_ENG_CMDS_VAL_FIELD_WIDTH,
    CNPL_PM_COUNTERS_ENG_CMDS_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: CNPL_PM_COUNTERS_CMD_WAIT_VAL
 ******************************************************************************/
const ru_field_rec CNPL_PM_COUNTERS_CMD_WAIT_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "value",
    "value",
#endif
    CNPL_PM_COUNTERS_CMD_WAIT_VAL_FIELD_MASK,
    0,
    CNPL_PM_COUNTERS_CMD_WAIT_VAL_FIELD_WIDTH,
    CNPL_PM_COUNTERS_CMD_WAIT_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: CNPL_PM_COUNTERS_TOT_CYC_VAL
 ******************************************************************************/
const ru_field_rec CNPL_PM_COUNTERS_TOT_CYC_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "value",
    "value",
#endif
    CNPL_PM_COUNTERS_TOT_CYC_VAL_FIELD_MASK,
    0,
    CNPL_PM_COUNTERS_TOT_CYC_VAL_FIELD_WIDTH,
    CNPL_PM_COUNTERS_TOT_CYC_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: CNPL_PM_COUNTERS_GNT_CYC_VAL
 ******************************************************************************/
const ru_field_rec CNPL_PM_COUNTERS_GNT_CYC_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "value",
    "value",
#endif
    CNPL_PM_COUNTERS_GNT_CYC_VAL_FIELD_MASK,
    0,
    CNPL_PM_COUNTERS_GNT_CYC_VAL_FIELD_WIDTH,
    CNPL_PM_COUNTERS_GNT_CYC_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: CNPL_PM_COUNTERS_ARB_CYC_VAL
 ******************************************************************************/
const ru_field_rec CNPL_PM_COUNTERS_ARB_CYC_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "value",
    "value",
#endif
    CNPL_PM_COUNTERS_ARB_CYC_VAL_FIELD_MASK,
    0,
    CNPL_PM_COUNTERS_ARB_CYC_VAL_FIELD_WIDTH,
    CNPL_PM_COUNTERS_ARB_CYC_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: CNPL_PM_COUNTERS_PL_UP_ERR_VAL
 ******************************************************************************/
const ru_field_rec CNPL_PM_COUNTERS_PL_UP_ERR_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "value",
    "value",
#endif
    CNPL_PM_COUNTERS_PL_UP_ERR_VAL_FIELD_MASK,
    0,
    CNPL_PM_COUNTERS_PL_UP_ERR_VAL_FIELD_WIDTH,
    CNPL_PM_COUNTERS_PL_UP_ERR_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: CNPL_PM_COUNTERS_GEN_CFG_RD_CLR
 ******************************************************************************/
const ru_field_rec CNPL_PM_COUNTERS_GEN_CFG_RD_CLR_FIELD =
{
    "RD_CLR",
#if RU_INCLUDE_DESC
    "rd_clr",
    "read clear bit",
#endif
    CNPL_PM_COUNTERS_GEN_CFG_RD_CLR_FIELD_MASK,
    0,
    CNPL_PM_COUNTERS_GEN_CFG_RD_CLR_FIELD_WIDTH,
    CNPL_PM_COUNTERS_GEN_CFG_RD_CLR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CNPL_PM_COUNTERS_GEN_CFG_WRAP
 ******************************************************************************/
const ru_field_rec CNPL_PM_COUNTERS_GEN_CFG_WRAP_FIELD =
{
    "WRAP",
#if RU_INCLUDE_DESC
    "wrap",
    "read clear bit",
#endif
    CNPL_PM_COUNTERS_GEN_CFG_WRAP_FIELD_MASK,
    0,
    CNPL_PM_COUNTERS_GEN_CFG_WRAP_FIELD_WIDTH,
    CNPL_PM_COUNTERS_GEN_CFG_WRAP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CNPL_PM_COUNTERS_GEN_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec CNPL_PM_COUNTERS_GEN_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    CNPL_PM_COUNTERS_GEN_CFG_RESERVED0_FIELD_MASK,
    0,
    CNPL_PM_COUNTERS_GEN_CFG_RESERVED0_FIELD_WIDTH,
    CNPL_PM_COUNTERS_GEN_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CNPL_DEBUG_DBGSEL_VS
 ******************************************************************************/
const ru_field_rec CNPL_DEBUG_DBGSEL_VS_FIELD =
{
    "VS",
#if RU_INCLUDE_DESC
    "vector_select",
    "selects th debug vector",
#endif
    CNPL_DEBUG_DBGSEL_VS_FIELD_MASK,
    0,
    CNPL_DEBUG_DBGSEL_VS_FIELD_WIDTH,
    CNPL_DEBUG_DBGSEL_VS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CNPL_DEBUG_DBGSEL_RESERVED0
 ******************************************************************************/
const ru_field_rec CNPL_DEBUG_DBGSEL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    CNPL_DEBUG_DBGSEL_RESERVED0_FIELD_MASK,
    0,
    CNPL_DEBUG_DBGSEL_RESERVED0_FIELD_WIDTH,
    CNPL_DEBUG_DBGSEL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CNPL_DEBUG_DBGBUS_VB
 ******************************************************************************/
const ru_field_rec CNPL_DEBUG_DBGBUS_VB_FIELD =
{
    "VB",
#if RU_INCLUDE_DESC
    "dbg_bus",
    "debug vector",
#endif
    CNPL_DEBUG_DBGBUS_VB_FIELD_MASK,
    0,
    CNPL_DEBUG_DBGBUS_VB_FIELD_WIDTH,
    CNPL_DEBUG_DBGBUS_VB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: CNPL_DEBUG_DBGBUS_RESERVED0
 ******************************************************************************/
const ru_field_rec CNPL_DEBUG_DBGBUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    CNPL_DEBUG_DBGBUS_RESERVED0_FIELD_MASK,
    0,
    CNPL_DEBUG_DBGBUS_RESERVED0_FIELD_WIDTH,
    CNPL_DEBUG_DBGBUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: CNPL_DEBUG_REQ_VEC_REQ
 ******************************************************************************/
const ru_field_rec CNPL_DEBUG_REQ_VEC_REQ_FIELD =
{
    "REQ",
#if RU_INCLUDE_DESC
    "all_requests",
    "still more commands for arbitration",
#endif
    CNPL_DEBUG_REQ_VEC_REQ_FIELD_MASK,
    0,
    CNPL_DEBUG_REQ_VEC_REQ_FIELD_WIDTH,
    CNPL_DEBUG_REQ_VEC_REQ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: CNPL_DEBUG_REQ_VEC_RESERVED0
 ******************************************************************************/
const ru_field_rec CNPL_DEBUG_REQ_VEC_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    CNPL_DEBUG_REQ_VEC_RESERVED0_FIELD_MASK,
    0,
    CNPL_DEBUG_REQ_VEC_RESERVED0_FIELD_WIDTH,
    CNPL_DEBUG_REQ_VEC_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: CNPL_DEBUG_POL_UP_ST_ITR_NUM
 ******************************************************************************/
const ru_field_rec CNPL_DEBUG_POL_UP_ST_ITR_NUM_FIELD =
{
    "ITR_NUM",
#if RU_INCLUDE_DESC
    "itr_num",
    "number of iteration we are(each represent 8192 cycles)",
#endif
    CNPL_DEBUG_POL_UP_ST_ITR_NUM_FIELD_MASK,
    0,
    CNPL_DEBUG_POL_UP_ST_ITR_NUM_FIELD_WIDTH,
    CNPL_DEBUG_POL_UP_ST_ITR_NUM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: CNPL_DEBUG_POL_UP_ST_POL_NUM
 ******************************************************************************/
const ru_field_rec CNPL_DEBUG_POL_UP_ST_POL_NUM_FIELD =
{
    "POL_NUM",
#if RU_INCLUDE_DESC
    "pol_num",
    "number of policer now updated."
    "(80 means we finished updated of all policers for this period)",
#endif
    CNPL_DEBUG_POL_UP_ST_POL_NUM_FIELD_MASK,
    0,
    CNPL_DEBUG_POL_UP_ST_POL_NUM_FIELD_WIDTH,
    CNPL_DEBUG_POL_UP_ST_POL_NUM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: CNPL_DEBUG_POL_UP_ST_RESERVED0
 ******************************************************************************/
const ru_field_rec CNPL_DEBUG_POL_UP_ST_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    CNPL_DEBUG_POL_UP_ST_RESERVED0_FIELD_MASK,
    0,
    CNPL_DEBUG_POL_UP_ST_RESERVED0_FIELD_WIDTH,
    CNPL_DEBUG_POL_UP_ST_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: CNPL_MEMORY_DATA
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_MEMORY_DATA_FIELDS[] =
{
    &CNPL_MEMORY_DATA_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec CNPL_MEMORY_DATA_REG = 
{
    "MEMORY_DATA",
#if RU_INCLUDE_DESC
    "MEM_ENTRY %i Register",
    "mem_entry",
#endif
    CNPL_MEMORY_DATA_REG_OFFSET,
    CNPL_MEMORY_DATA_REG_RAM_CNT,
    4,
    1067,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_MEMORY_DATA_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_FIELDS[] =
{
    &CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_BA_FIELD,
    &CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_CN0_BYTS_FIELD,
    &CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_DOUBLLE_FIELD,
    &CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_WRAP_FIELD,
    &CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_CLR_FIELD,
    &CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_REG = 
{
    "COUNTERS_CONFIGURATIONS_CN_LOC_PROF",
#if RU_INCLUDE_DESC
    "CNT_LOC_PROFILE %i Register",
    "location profiles",
#endif
    CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_REG_OFFSET,
    CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_REG_RAM_CNT,
    4,
    1068,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_FIELDS[] =
{
    &CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_BK_BA_FIELD,
    &CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_PA_BA_FIELD,
    &CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_DOUBLLE_FIELD,
    &CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_REG = 
{
    "POLICERS_CONFIGURATIONS_PL_LOC_PROF0",
#if RU_INCLUDE_DESC
    "PL_LOC_PROFILE0 %i Register",
    "1st reg for location profiles",
#endif
    CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_REG_OFFSET,
    CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_REG_RAM_CNT,
    4,
    1069,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_FIELDS[] =
{
    &CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_PL_ST_FIELD,
    &CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_PL_END_FIELD,
    &CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_REG = 
{
    "POLICERS_CONFIGURATIONS_PL_LOC_PROF1",
#if RU_INCLUDE_DESC
    "PL_LOC_PROFILE1 %i Register",
    "2nd reg for location profiles",
#endif
    CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_REG_OFFSET,
    CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_REG_RAM_CNT,
    4,
    1070,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: CNPL_POLICERS_CONFIGURATIONS_PL_CALC_TYPE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_POLICERS_CONFIGURATIONS_PL_CALC_TYPE_FIELDS[] =
{
    &CNPL_POLICERS_CONFIGURATIONS_PL_CALC_TYPE_VEC_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec CNPL_POLICERS_CONFIGURATIONS_PL_CALC_TYPE_REG = 
{
    "POLICERS_CONFIGURATIONS_PL_CALC_TYPE",
#if RU_INCLUDE_DESC
    "PL_CALC_TYPE %i Register",
    "calculation type register."
    "0:green, yellow, red"
    "1:red, yellow, green",
#endif
    CNPL_POLICERS_CONFIGURATIONS_PL_CALC_TYPE_REG_OFFSET,
    CNPL_POLICERS_CONFIGURATIONS_PL_CALC_TYPE_REG_RAM_CNT,
    4,
    1071,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_POLICERS_CONFIGURATIONS_PL_CALC_TYPE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: CNPL_POLICERS_CONFIGURATIONS_PER_UP
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_POLICERS_CONFIGURATIONS_PER_UP_FIELDS[] =
{
    &CNPL_POLICERS_CONFIGURATIONS_PER_UP_N_FIELD,
    &CNPL_POLICERS_CONFIGURATIONS_PER_UP_EN_FIELD,
    &CNPL_POLICERS_CONFIGURATIONS_PER_UP_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec CNPL_POLICERS_CONFIGURATIONS_PER_UP_REG = 
{
    "POLICERS_CONFIGURATIONS_PER_UP",
#if RU_INCLUDE_DESC
    "PL_PERIODIC_UPDATE Register",
    "periodic update parameters",
#endif
    CNPL_POLICERS_CONFIGURATIONS_PER_UP_REG_OFFSET,
    0,
    0,
    1072,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    CNPL_POLICERS_CONFIGURATIONS_PER_UP_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: CNPL_MISC_ARB_PRM
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_MISC_ARB_PRM_FIELDS[] =
{
    &CNPL_MISC_ARB_PRM_SW_PRIO_FIELD,
    &CNPL_MISC_ARB_PRM_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec CNPL_MISC_ARB_PRM_REG = 
{
    "MISC_ARB_PRM",
#if RU_INCLUDE_DESC
    "ARBITER_PARAM Register",
    "arbiter sw priorities",
#endif
    CNPL_MISC_ARB_PRM_REG_OFFSET,
    0,
    0,
    1073,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    CNPL_MISC_ARB_PRM_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: CNPL_MISC_COL_AWR_EN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_MISC_COL_AWR_EN_FIELDS[] =
{
    &CNPL_MISC_COL_AWR_EN_EN_FIELD,
    &CNPL_MISC_COL_AWR_EN_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec CNPL_MISC_COL_AWR_EN_REG = 
{
    "MISC_COL_AWR_EN",
#if RU_INCLUDE_DESC
    "COLOR_AWARE_ENABLE Register",
    "color aware enable",
#endif
    CNPL_MISC_COL_AWR_EN_REG_OFFSET,
    0,
    0,
    1074,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    CNPL_MISC_COL_AWR_EN_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: CNPL_SW_IF_SW_CMD
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_SW_IF_SW_CMD_FIELDS[] =
{
    &CNPL_SW_IF_SW_CMD_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec CNPL_SW_IF_SW_CMD_REG = 
{
    "SW_IF_SW_CMD",
#if RU_INCLUDE_DESC
    "COMMAND Register",
    "command register",
#endif
    CNPL_SW_IF_SW_CMD_REG_OFFSET,
    0,
    0,
    1075,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_SW_IF_SW_CMD_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: CNPL_SW_IF_SW_STAT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_SW_IF_SW_STAT_FIELDS[] =
{
    &CNPL_SW_IF_SW_STAT_CN_RD_ST_FIELD,
    &CNPL_SW_IF_SW_STAT_PL_PLC_ST_FIELD,
    &CNPL_SW_IF_SW_STAT_PL_RD_ST_FIELD,
    &CNPL_SW_IF_SW_STAT_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec CNPL_SW_IF_SW_STAT_REG = 
{
    "SW_IF_SW_STAT",
#if RU_INCLUDE_DESC
    "STATUS Register",
    "status register",
#endif
    CNPL_SW_IF_SW_STAT_REG_OFFSET,
    0,
    0,
    1076,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    CNPL_SW_IF_SW_STAT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: CNPL_SW_IF_SW_PL_RSLT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_SW_IF_SW_PL_RSLT_FIELDS[] =
{
    &CNPL_SW_IF_SW_PL_RSLT_RESERVED0_FIELD,
    &CNPL_SW_IF_SW_PL_RSLT_COL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec CNPL_SW_IF_SW_PL_RSLT_REG = 
{
    "SW_IF_SW_PL_RSLT",
#if RU_INCLUDE_DESC
    "PL_RSLT Register",
    "rdata register - policer command result",
#endif
    CNPL_SW_IF_SW_PL_RSLT_REG_OFFSET,
    0,
    0,
    1077,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    CNPL_SW_IF_SW_PL_RSLT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: CNPL_SW_IF_SW_PL_RD
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_SW_IF_SW_PL_RD_FIELDS[] =
{
    &CNPL_SW_IF_SW_PL_RD_RD_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec CNPL_SW_IF_SW_PL_RD_REG = 
{
    "SW_IF_SW_PL_RD",
#if RU_INCLUDE_DESC
    "PL_RDX %i Register",
    "rdata register - policer read command result. 2 register for 2 buckets. If the group has only one bucket per policer - the policers are returned in the registers as a full line: the even policers are in reg0 (0,2,4,..), and the odd are in reg1.",
#endif
    CNPL_SW_IF_SW_PL_RD_REG_OFFSET,
    CNPL_SW_IF_SW_PL_RD_REG_RAM_CNT,
    4,
    1078,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_SW_IF_SW_PL_RD_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: CNPL_SW_IF_SW_CNT_RD
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_SW_IF_SW_CNT_RD_FIELDS[] =
{
    &CNPL_SW_IF_SW_CNT_RD_RD_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec CNPL_SW_IF_SW_CNT_RD_REG = 
{
    "SW_IF_SW_CNT_RD",
#if RU_INCLUDE_DESC
    "CNT_RDX %i Register",
    "rdata register - counters read command result. 8 register for 32B batch. In read of single counter (burst size=1) the output will be in reg0 (the 32b where the counter is). In read of burst of counters, the counters are returned in the registers as a full line: addr[2:0]=0 section of line in reg0,2,4,6  and the addr[2:0]=4 are in reg1,3,5,7 (this means that if the start of burst is at addr[2:0]=4 section of line, the wanted output should be from reg1).",
#endif
    CNPL_SW_IF_SW_CNT_RD_REG_OFFSET,
    CNPL_SW_IF_SW_CNT_RD_REG_RAM_CNT,
    4,
    1079,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_SW_IF_SW_CNT_RD_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: CNPL_PM_COUNTERS_ENG_CMDS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_PM_COUNTERS_ENG_CMDS_FIELDS[] =
{
    &CNPL_PM_COUNTERS_ENG_CMDS_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec CNPL_PM_COUNTERS_ENG_CMDS_REG = 
{
    "PM_COUNTERS_ENG_CMDS",
#if RU_INCLUDE_DESC
    "ENG_CMDS_CNTR %i Register",
    "Number of commands that were processed by the engine.",
#endif
    CNPL_PM_COUNTERS_ENG_CMDS_REG_OFFSET,
    CNPL_PM_COUNTERS_ENG_CMDS_REG_RAM_CNT,
    4,
    1080,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_PM_COUNTERS_ENG_CMDS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: CNPL_PM_COUNTERS_CMD_WAIT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_PM_COUNTERS_CMD_WAIT_FIELDS[] =
{
    &CNPL_PM_COUNTERS_CMD_WAIT_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec CNPL_PM_COUNTERS_CMD_WAIT_REG = 
{
    "PM_COUNTERS_CMD_WAIT",
#if RU_INCLUDE_DESC
    "CMD_WAITS_CNTR %i Register",
    "Number of wait cycles that the command waited until there was an idle engine.",
#endif
    CNPL_PM_COUNTERS_CMD_WAIT_REG_OFFSET,
    CNPL_PM_COUNTERS_CMD_WAIT_REG_RAM_CNT,
    4,
    1081,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_PM_COUNTERS_CMD_WAIT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: CNPL_PM_COUNTERS_TOT_CYC
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_PM_COUNTERS_TOT_CYC_FIELDS[] =
{
    &CNPL_PM_COUNTERS_TOT_CYC_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec CNPL_PM_COUNTERS_TOT_CYC_REG = 
{
    "PM_COUNTERS_TOT_CYC",
#if RU_INCLUDE_DESC
    "TOT_CYC_CNTR Register",
    "Number of cycles from last read clear",
#endif
    CNPL_PM_COUNTERS_TOT_CYC_REG_OFFSET,
    0,
    0,
    1082,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_PM_COUNTERS_TOT_CYC_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: CNPL_PM_COUNTERS_GNT_CYC
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_PM_COUNTERS_GNT_CYC_FIELDS[] =
{
    &CNPL_PM_COUNTERS_GNT_CYC_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec CNPL_PM_COUNTERS_GNT_CYC_REG = 
{
    "PM_COUNTERS_GNT_CYC",
#if RU_INCLUDE_DESC
    "GNT_CYC_CNTR Register",
    "Number of cycles that there was gnt from main arbiter",
#endif
    CNPL_PM_COUNTERS_GNT_CYC_REG_OFFSET,
    0,
    0,
    1083,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_PM_COUNTERS_GNT_CYC_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: CNPL_PM_COUNTERS_ARB_CYC
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_PM_COUNTERS_ARB_CYC_FIELDS[] =
{
    &CNPL_PM_COUNTERS_ARB_CYC_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec CNPL_PM_COUNTERS_ARB_CYC_REG = 
{
    "PM_COUNTERS_ARB_CYC",
#if RU_INCLUDE_DESC
    "ARB_CYC_CNTR Register",
    "Number of cycles that there was gnt with request of more than one agent",
#endif
    CNPL_PM_COUNTERS_ARB_CYC_REG_OFFSET,
    0,
    0,
    1084,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_PM_COUNTERS_ARB_CYC_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: CNPL_PM_COUNTERS_PL_UP_ERR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_PM_COUNTERS_PL_UP_ERR_FIELDS[] =
{
    &CNPL_PM_COUNTERS_PL_UP_ERR_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec CNPL_PM_COUNTERS_PL_UP_ERR_REG = 
{
    "PM_COUNTERS_PL_UP_ERR",
#if RU_INCLUDE_DESC
    "POL_UP_ERR_CNTR Register",
    "errors in policer update: the update period finished, and not all policers have been updated yet.",
#endif
    CNPL_PM_COUNTERS_PL_UP_ERR_REG_OFFSET,
    0,
    0,
    1085,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    CNPL_PM_COUNTERS_PL_UP_ERR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: CNPL_PM_COUNTERS_GEN_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_PM_COUNTERS_GEN_CFG_FIELDS[] =
{
    &CNPL_PM_COUNTERS_GEN_CFG_RD_CLR_FIELD,
    &CNPL_PM_COUNTERS_GEN_CFG_WRAP_FIELD,
    &CNPL_PM_COUNTERS_GEN_CFG_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec CNPL_PM_COUNTERS_GEN_CFG_REG = 
{
    "PM_COUNTERS_GEN_CFG",
#if RU_INCLUDE_DESC
    "GENERAL_CONFIG Register",
    "bits rd_clr and wrap for the pm counters(above)",
#endif
    CNPL_PM_COUNTERS_GEN_CFG_REG_OFFSET,
    0,
    0,
    1086,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    CNPL_PM_COUNTERS_GEN_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: CNPL_DEBUG_DBGSEL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_DEBUG_DBGSEL_FIELDS[] =
{
    &CNPL_DEBUG_DBGSEL_VS_FIELD,
    &CNPL_DEBUG_DBGSEL_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec CNPL_DEBUG_DBGSEL_REG = 
{
    "DEBUG_DBGSEL",
#if RU_INCLUDE_DESC
    "DBG_MUX_SEL Register",
    "selects the debug vecore",
#endif
    CNPL_DEBUG_DBGSEL_REG_OFFSET,
    0,
    0,
    1087,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    CNPL_DEBUG_DBGSEL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: CNPL_DEBUG_DBGBUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_DEBUG_DBGBUS_FIELDS[] =
{
    &CNPL_DEBUG_DBGBUS_VB_FIELD,
    &CNPL_DEBUG_DBGBUS_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec CNPL_DEBUG_DBGBUS_REG = 
{
    "DEBUG_DBGBUS",
#if RU_INCLUDE_DESC
    "DBG_BUS Register",
    "the debug bus",
#endif
    CNPL_DEBUG_DBGBUS_REG_OFFSET,
    0,
    0,
    1088,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    CNPL_DEBUG_DBGBUS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: CNPL_DEBUG_REQ_VEC
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_DEBUG_REQ_VEC_FIELDS[] =
{
    &CNPL_DEBUG_REQ_VEC_REQ_FIELD,
    &CNPL_DEBUG_REQ_VEC_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec CNPL_DEBUG_REQ_VEC_REG = 
{
    "DEBUG_REQ_VEC",
#if RU_INCLUDE_DESC
    "REQUEST_VECTOR Register",
    "vector of all the requests of the clients (tx fifo not empty)",
#endif
    CNPL_DEBUG_REQ_VEC_REG_OFFSET,
    0,
    0,
    1089,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    CNPL_DEBUG_REQ_VEC_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: CNPL_DEBUG_POL_UP_ST
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CNPL_DEBUG_POL_UP_ST_FIELDS[] =
{
    &CNPL_DEBUG_POL_UP_ST_ITR_NUM_FIELD,
    &CNPL_DEBUG_POL_UP_ST_POL_NUM_FIELD,
    &CNPL_DEBUG_POL_UP_ST_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec CNPL_DEBUG_POL_UP_ST_REG = 
{
    "DEBUG_POL_UP_ST",
#if RU_INCLUDE_DESC
    "POLICER_UPDATE_STATUS Register",
    "which counter is updated, and where are we in the period cycle",
#endif
    CNPL_DEBUG_POL_UP_ST_REG_OFFSET,
    0,
    0,
    1090,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    CNPL_DEBUG_POL_UP_ST_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Block: CNPL
 ******************************************************************************/
static const ru_reg_rec *CNPL_REGS[] =
{
    &CNPL_MEMORY_DATA_REG,
    &CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_REG,
    &CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_REG,
    &CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_REG,
    &CNPL_POLICERS_CONFIGURATIONS_PL_CALC_TYPE_REG,
    &CNPL_POLICERS_CONFIGURATIONS_PER_UP_REG,
    &CNPL_MISC_ARB_PRM_REG,
    &CNPL_MISC_COL_AWR_EN_REG,
    &CNPL_SW_IF_SW_CMD_REG,
    &CNPL_SW_IF_SW_STAT_REG,
    &CNPL_SW_IF_SW_PL_RSLT_REG,
    &CNPL_SW_IF_SW_PL_RD_REG,
    &CNPL_SW_IF_SW_CNT_RD_REG,
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

unsigned long CNPL_ADDRS[] =
{
    0x82e48000,
};

const ru_block_rec CNPL_BLOCK = 
{
    "CNPL",
    CNPL_ADDRS,
    1,
    24,
    CNPL_REGS
};

/* End of file XRDP_CNPL.c */
