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
 * Register: CNPL_MEMORY_DATA
 ******************************************************************************/
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
    1008,
};

/******************************************************************************
 * Register: CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF
 ******************************************************************************/
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
    1009,
};

/******************************************************************************
 * Register: CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0
 ******************************************************************************/
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
    1010,
};

/******************************************************************************
 * Register: CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1
 ******************************************************************************/
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
    1011,
};

/******************************************************************************
 * Register: CNPL_POLICERS_CONFIGURATIONS_PL_CALC_TYPE
 ******************************************************************************/
const ru_reg_rec CNPL_POLICERS_CONFIGURATIONS_PL_CALC_TYPE_REG = 
{
    "POLICERS_CONFIGURATIONS_PL_CALC_TYPE",
#if RU_INCLUDE_DESC
    "PL_CALC_TYPE %i Register",
    "calculation type register."
    "0:green, yellow, red"
    "1:red, yellow, green"
    ""
    "3 registers support up to 96 policers. Currently we have only 80 policers.",
#endif
    CNPL_POLICERS_CONFIGURATIONS_PL_CALC_TYPE_REG_OFFSET,
    CNPL_POLICERS_CONFIGURATIONS_PL_CALC_TYPE_REG_RAM_CNT,
    4,
    1012,
};

/******************************************************************************
 * Register: CNPL_POLICERS_CONFIGURATIONS_PER_UP
 ******************************************************************************/
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
    1013,
};

/******************************************************************************
 * Register: CNPL_POLICERS_CONFIGURATIONS_PL_SIZE_PROF
 ******************************************************************************/
const ru_reg_rec CNPL_POLICERS_CONFIGURATIONS_PL_SIZE_PROF_REG = 
{
    "POLICERS_CONFIGURATIONS_PL_SIZE_PROF",
#if RU_INCLUDE_DESC
    "PL_SIZE_PROFILE %i Register",
    "8 profiles, 16b each, for calculation of bucket size",
#endif
    CNPL_POLICERS_CONFIGURATIONS_PL_SIZE_PROF_REG_OFFSET,
    CNPL_POLICERS_CONFIGURATIONS_PL_SIZE_PROF_REG_RAM_CNT,
    4,
    1014,
};

/******************************************************************************
 * Register: CNPL_MISC_ARB_PRM
 ******************************************************************************/
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
    1015,
};

/******************************************************************************
 * Register: CNPL_MISC_COL_AWR_EN
 ******************************************************************************/
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
    1016,
};

/******************************************************************************
 * Register: CNPL_SW_IF_SW_CMD
 ******************************************************************************/
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
    1017,
};

/******************************************************************************
 * Register: CNPL_SW_IF_SW_STAT
 ******************************************************************************/
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
    1018,
};

/******************************************************************************
 * Register: CNPL_SW_IF_SW_PL_RSLT
 ******************************************************************************/
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
    1019,
};

/******************************************************************************
 * Register: CNPL_SW_IF_SW_PL_RD
 ******************************************************************************/
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
    1020,
};

/******************************************************************************
 * Register: CNPL_SW_IF_SW_CNT_RD
 ******************************************************************************/
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
    1021,
};

/******************************************************************************
 * Register: CNPL_PM_COUNTERS_ENG_CMDS
 ******************************************************************************/
const ru_reg_rec CNPL_PM_COUNTERS_ENG_CMDS_REG = 
{
    "PM_COUNTERS_ENG_CMDS",
#if RU_INCLUDE_DESC
    "ENG_CMDS_CNTR %i Register",
    "Number of commands that were processed by the engine."
    "order:"
    "0-3: counters"
    "4-7: policers"
    "8: sw counter read"
    "9: sw policer"
    "10:sw policer rd",
#endif
    CNPL_PM_COUNTERS_ENG_CMDS_REG_OFFSET,
    CNPL_PM_COUNTERS_ENG_CMDS_REG_RAM_CNT,
    4,
    1022,
};

/******************************************************************************
 * Register: CNPL_PM_COUNTERS_CMD_WAIT
 ******************************************************************************/
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
    1023,
};

/******************************************************************************
 * Register: CNPL_PM_COUNTERS_TOT_CYC
 ******************************************************************************/
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
    1024,
};

/******************************************************************************
 * Register: CNPL_PM_COUNTERS_GNT_CYC
 ******************************************************************************/
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
    1025,
};

/******************************************************************************
 * Register: CNPL_PM_COUNTERS_ARB_CYC
 ******************************************************************************/
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
    1026,
};

/******************************************************************************
 * Register: CNPL_PM_COUNTERS_PL_UP_ERR
 ******************************************************************************/
const ru_reg_rec CNPL_PM_COUNTERS_PL_UP_ERR_REG = 
{
    "PM_COUNTERS_PL_UP_ERR",
#if RU_INCLUDE_DESC
    "POL_UP_ERR_CNTR %i Register",
    "errors in policer update: the update period finished, and not all policers have been updated yet.",
#endif
    CNPL_PM_COUNTERS_PL_UP_ERR_REG_OFFSET,
    CNPL_PM_COUNTERS_PL_UP_ERR_REG_RAM_CNT,
    4,
    1027,
};

/******************************************************************************
 * Register: CNPL_PM_COUNTERS_GEN_CFG
 ******************************************************************************/
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
    1028,
};

/******************************************************************************
 * Register: CNPL_DEBUG_DBGSEL
 ******************************************************************************/
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
    1029,
};

/******************************************************************************
 * Register: CNPL_DEBUG_DBGBUS
 ******************************************************************************/
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
    1030,
};

/******************************************************************************
 * Register: CNPL_DEBUG_REQ_VEC
 ******************************************************************************/
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
    1031,
};

/******************************************************************************
 * Register: CNPL_DEBUG_POL_UP_ST
 ******************************************************************************/
const ru_reg_rec CNPL_DEBUG_POL_UP_ST_REG = 
{
    "DEBUG_POL_UP_ST",
#if RU_INCLUDE_DESC
    "POLICER_UPDATE_STATUS %i Register",
    "which counter is updated, and where are we in the period cycle",
#endif
    CNPL_DEBUG_POL_UP_ST_REG_OFFSET,
    CNPL_DEBUG_POL_UP_ST_REG_RAM_CNT,
    4,
    1032,
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
    &CNPL_POLICERS_CONFIGURATIONS_PL_SIZE_PROF_REG,
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
    25,
    CNPL_REGS
};

/* End of file XRDP_CNPL.c */
