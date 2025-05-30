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


#ifndef _XRDP_CNPL_AG_H_
#define _XRDP_CNPL_AG_H_

#include "ru_types.h"

#define CNPL_MEMORY_DATA_DATA_FIELD_MASK 0xFFFFFFFF
#define CNPL_MEMORY_DATA_DATA_FIELD_WIDTH 32
#define CNPL_MEMORY_DATA_DATA_FIELD_SHIFT 0
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec CNPL_MEMORY_DATA_DATA_FIELD;
#endif
extern const ru_reg_rec CNPL_MEMORY_DATA_REG;
#define CNPL_MEMORY_DATA_REG_OFFSET 0x00000000
#define CNPL_MEMORY_DATA_REG_RAM_CNT 5120

#define CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_BA_FIELD_MASK 0x00000FFF
#define CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_BA_FIELD_WIDTH 12
#define CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_BA_FIELD_SHIFT 0
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_BA_FIELD;
#endif
#define CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_CN0_BYTS_FIELD_MASK 0x00003000
#define CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_CN0_BYTS_FIELD_WIDTH 2
#define CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_CN0_BYTS_FIELD_SHIFT 12
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_CN0_BYTS_FIELD;
#endif
#define CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_DOUBLLE_FIELD_MASK 0x00004000
#define CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_DOUBLLE_FIELD_WIDTH 1
#define CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_DOUBLLE_FIELD_SHIFT 14
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_DOUBLLE_FIELD;
#endif
#define CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_WRAP_FIELD_MASK 0x00008000
#define CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_WRAP_FIELD_WIDTH 1
#define CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_WRAP_FIELD_SHIFT 15
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_WRAP_FIELD;
#endif
#define CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_CLR_FIELD_MASK 0x00010000
#define CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_CLR_FIELD_WIDTH 1
#define CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_CLR_FIELD_SHIFT 16
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_CLR_FIELD;
#endif
extern const ru_reg_rec CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_REG;
#define CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_REG_OFFSET 0x00005000
#define CNPL_COUNTERS_CONFIGURATIONS_CN_LOC_PROF_REG_RAM_CNT 16

#define CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_BK_BA_FIELD_MASK 0x00000FFF
#define CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_BK_BA_FIELD_WIDTH 12
#define CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_BK_BA_FIELD_SHIFT 0
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_BK_BA_FIELD;
#endif
#define CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_PA_BA_FIELD_MASK 0x00FFF000
#define CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_PA_BA_FIELD_WIDTH 12
#define CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_PA_BA_FIELD_SHIFT 12
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_PA_BA_FIELD;
#endif
#define CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_DOUBLLE_FIELD_MASK 0x01000000
#define CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_DOUBLLE_FIELD_WIDTH 1
#define CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_DOUBLLE_FIELD_SHIFT 24
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_DOUBLLE_FIELD;
#endif
#define CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_FC_FIELD_MASK 0x02000000
#define CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_FC_FIELD_WIDTH 1
#define CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_FC_FIELD_SHIFT 25
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_FC_FIELD;
#endif
extern const ru_reg_rec CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_REG;
#define CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_REG_OFFSET 0x00005100
#define CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF0_REG_RAM_CNT 4

#define CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_PL_ST_FIELD_MASK 0x000000FF
#define CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_PL_ST_FIELD_WIDTH 8
#define CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_PL_ST_FIELD_SHIFT 0
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_PL_ST_FIELD;
#endif
#define CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_PL_END_FIELD_MASK 0x0000FF00
#define CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_PL_END_FIELD_WIDTH 8
#define CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_PL_END_FIELD_SHIFT 8
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_PL_END_FIELD;
#endif
#define CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_N_FIELD_MASK 0x00FF0000
#define CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_N_FIELD_WIDTH 8
#define CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_N_FIELD_SHIFT 16
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_N_FIELD;
#endif
extern const ru_reg_rec CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_REG;
#define CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_REG_OFFSET 0x00005110
#define CNPL_POLICERS_CONFIGURATIONS_PL_LOC_PROF1_REG_RAM_CNT 4

#define CNPL_POLICERS_CONFIGURATIONS_PL_CALC_TYPE_VEC_FIELD_MASK 0xFFFFFFFF
#define CNPL_POLICERS_CONFIGURATIONS_PL_CALC_TYPE_VEC_FIELD_WIDTH 32
#define CNPL_POLICERS_CONFIGURATIONS_PL_CALC_TYPE_VEC_FIELD_SHIFT 0
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec CNPL_POLICERS_CONFIGURATIONS_PL_CALC_TYPE_VEC_FIELD;
#endif
extern const ru_reg_rec CNPL_POLICERS_CONFIGURATIONS_PL_CALC_TYPE_REG;
#define CNPL_POLICERS_CONFIGURATIONS_PL_CALC_TYPE_REG_OFFSET 0x00005120
#define CNPL_POLICERS_CONFIGURATIONS_PL_CALC_TYPE_REG_RAM_CNT 8

#define CNPL_POLICERS_CONFIGURATIONS_PL_SIZE_PROF_PRF0_FIELD_MASK 0x0000FFFF
#define CNPL_POLICERS_CONFIGURATIONS_PL_SIZE_PROF_PRF0_FIELD_WIDTH 16
#define CNPL_POLICERS_CONFIGURATIONS_PL_SIZE_PROF_PRF0_FIELD_SHIFT 0
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec CNPL_POLICERS_CONFIGURATIONS_PL_SIZE_PROF_PRF0_FIELD;
#endif
#define CNPL_POLICERS_CONFIGURATIONS_PL_SIZE_PROF_PRF1_FIELD_MASK 0xFFFF0000
#define CNPL_POLICERS_CONFIGURATIONS_PL_SIZE_PROF_PRF1_FIELD_WIDTH 16
#define CNPL_POLICERS_CONFIGURATIONS_PL_SIZE_PROF_PRF1_FIELD_SHIFT 16
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec CNPL_POLICERS_CONFIGURATIONS_PL_SIZE_PROF_PRF1_FIELD;
#endif
extern const ru_reg_rec CNPL_POLICERS_CONFIGURATIONS_PL_SIZE_PROF_REG;
#define CNPL_POLICERS_CONFIGURATIONS_PL_SIZE_PROF_REG_OFFSET 0x00005140
#define CNPL_POLICERS_CONFIGURATIONS_PL_SIZE_PROF_REG_RAM_CNT 4

#define CNPL_POLICERS_CONFIGURATIONS_PER_UP_EN_FIELD_MASK 0x00000001
#define CNPL_POLICERS_CONFIGURATIONS_PER_UP_EN_FIELD_WIDTH 1
#define CNPL_POLICERS_CONFIGURATIONS_PER_UP_EN_FIELD_SHIFT 0
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec CNPL_POLICERS_CONFIGURATIONS_PER_UP_EN_FIELD;
#endif
#define CNPL_POLICERS_CONFIGURATIONS_PER_UP_MTU_FIELD_MASK 0x00007FFE
#define CNPL_POLICERS_CONFIGURATIONS_PER_UP_MTU_FIELD_WIDTH 14
#define CNPL_POLICERS_CONFIGURATIONS_PER_UP_MTU_FIELD_SHIFT 1
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec CNPL_POLICERS_CONFIGURATIONS_PER_UP_MTU_FIELD;
#endif
extern const ru_reg_rec CNPL_POLICERS_CONFIGURATIONS_PER_UP_REG;
#define CNPL_POLICERS_CONFIGURATIONS_PER_UP_REG_OFFSET 0x00005150

#define CNPL_SW_IF_SW_CMD_VAL_FIELD_MASK 0xFFFFFFFF
#define CNPL_SW_IF_SW_CMD_VAL_FIELD_WIDTH 32
#define CNPL_SW_IF_SW_CMD_VAL_FIELD_SHIFT 0
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec CNPL_SW_IF_SW_CMD_VAL_FIELD;
#endif
extern const ru_reg_rec CNPL_SW_IF_SW_CMD_REG;
#define CNPL_SW_IF_SW_CMD_REG_OFFSET 0x00005400

#define CNPL_SW_IF_SW_STAT_CN_RD_ST_FIELD_MASK 0x00000001
#define CNPL_SW_IF_SW_STAT_CN_RD_ST_FIELD_WIDTH 1
#define CNPL_SW_IF_SW_STAT_CN_RD_ST_FIELD_SHIFT 0
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec CNPL_SW_IF_SW_STAT_CN_RD_ST_FIELD;
#endif
#define CNPL_SW_IF_SW_STAT_PL_PLC_ST_FIELD_MASK 0x00000002
#define CNPL_SW_IF_SW_STAT_PL_PLC_ST_FIELD_WIDTH 1
#define CNPL_SW_IF_SW_STAT_PL_PLC_ST_FIELD_SHIFT 1
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec CNPL_SW_IF_SW_STAT_PL_PLC_ST_FIELD;
#endif
#define CNPL_SW_IF_SW_STAT_PL_RD_ST_FIELD_MASK 0x00000004
#define CNPL_SW_IF_SW_STAT_PL_RD_ST_FIELD_WIDTH 1
#define CNPL_SW_IF_SW_STAT_PL_RD_ST_FIELD_SHIFT 2
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec CNPL_SW_IF_SW_STAT_PL_RD_ST_FIELD;
#endif
extern const ru_reg_rec CNPL_SW_IF_SW_STAT_REG;
#define CNPL_SW_IF_SW_STAT_REG_OFFSET 0x00005404

#define CNPL_SW_IF_SW_PL_RSLT_COL_FIELD_MASK 0xC0000000
#define CNPL_SW_IF_SW_PL_RSLT_COL_FIELD_WIDTH 2
#define CNPL_SW_IF_SW_PL_RSLT_COL_FIELD_SHIFT 30
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec CNPL_SW_IF_SW_PL_RSLT_COL_FIELD;
#endif
extern const ru_reg_rec CNPL_SW_IF_SW_PL_RSLT_REG;
#define CNPL_SW_IF_SW_PL_RSLT_REG_OFFSET 0x00005410

#define CNPL_SW_IF_SW_PL_RD_RD_FIELD_MASK 0xFFFFFFFF
#define CNPL_SW_IF_SW_PL_RD_RD_FIELD_WIDTH 32
#define CNPL_SW_IF_SW_PL_RD_RD_FIELD_SHIFT 0
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec CNPL_SW_IF_SW_PL_RD_RD_FIELD;
#endif
extern const ru_reg_rec CNPL_SW_IF_SW_PL_RD_REG;
#define CNPL_SW_IF_SW_PL_RD_REG_OFFSET 0x00005418
#define CNPL_SW_IF_SW_PL_RD_REG_RAM_CNT 2

#define CNPL_SW_IF_SW_CNT_RD_RD_FIELD_MASK 0xFFFFFFFF
#define CNPL_SW_IF_SW_CNT_RD_RD_FIELD_WIDTH 32
#define CNPL_SW_IF_SW_CNT_RD_RD_FIELD_SHIFT 0
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec CNPL_SW_IF_SW_CNT_RD_RD_FIELD;
#endif
extern const ru_reg_rec CNPL_SW_IF_SW_CNT_RD_REG;
#define CNPL_SW_IF_SW_CNT_RD_REG_OFFSET 0x00005420
#define CNPL_SW_IF_SW_CNT_RD_REG_RAM_CNT 8

#define CNPL_MISC_ARB_PRM_SW_PRIO_FIELD_MASK 0x00000003
#define CNPL_MISC_ARB_PRM_SW_PRIO_FIELD_WIDTH 2
#define CNPL_MISC_ARB_PRM_SW_PRIO_FIELD_SHIFT 0
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec CNPL_MISC_ARB_PRM_SW_PRIO_FIELD;
#endif
#define CNPL_MISC_ARB_PRM_MEM_ADDR_BIT_SEL_FIELD_MASK 0x000000F0
#define CNPL_MISC_ARB_PRM_MEM_ADDR_BIT_SEL_FIELD_WIDTH 4
#define CNPL_MISC_ARB_PRM_MEM_ADDR_BIT_SEL_FIELD_SHIFT 4
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec CNPL_MISC_ARB_PRM_MEM_ADDR_BIT_SEL_FIELD;
#endif
extern const ru_reg_rec CNPL_MISC_ARB_PRM_REG;
#define CNPL_MISC_ARB_PRM_REG_OFFSET 0x00005500

#define CNPL_MISC_COL_AWR_EN_EN_FIELD_MASK 0x00000001
#define CNPL_MISC_COL_AWR_EN_EN_FIELD_WIDTH 1
#define CNPL_MISC_COL_AWR_EN_EN_FIELD_SHIFT 0
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec CNPL_MISC_COL_AWR_EN_EN_FIELD;
#endif
extern const ru_reg_rec CNPL_MISC_COL_AWR_EN_REG;
#define CNPL_MISC_COL_AWR_EN_REG_OFFSET 0x00005504

#define CNPL_MISC_MEM_INIT_INIT0_FIELD_MASK 0x00000001
#define CNPL_MISC_MEM_INIT_INIT0_FIELD_WIDTH 1
#define CNPL_MISC_MEM_INIT_INIT0_FIELD_SHIFT 0
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec CNPL_MISC_MEM_INIT_INIT0_FIELD;
#endif
#define CNPL_MISC_MEM_INIT_INIT1_FIELD_MASK 0x00000002
#define CNPL_MISC_MEM_INIT_INIT1_FIELD_WIDTH 1
#define CNPL_MISC_MEM_INIT_INIT1_FIELD_SHIFT 1
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec CNPL_MISC_MEM_INIT_INIT1_FIELD;
#endif
extern const ru_reg_rec CNPL_MISC_MEM_INIT_REG;
#define CNPL_MISC_MEM_INIT_REG_OFFSET 0x00005508

#define CNPL_PM_COUNTERS_ENG_CMDS_VAL_FIELD_MASK 0xFFFFFFFF
#define CNPL_PM_COUNTERS_ENG_CMDS_VAL_FIELD_WIDTH 32
#define CNPL_PM_COUNTERS_ENG_CMDS_VAL_FIELD_SHIFT 0
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec CNPL_PM_COUNTERS_ENG_CMDS_VAL_FIELD;
#endif
extern const ru_reg_rec CNPL_PM_COUNTERS_ENG_CMDS_REG;
#define CNPL_PM_COUNTERS_ENG_CMDS_REG_OFFSET 0x00005600
#define CNPL_PM_COUNTERS_ENG_CMDS_REG_RAM_CNT 23

#define CNPL_PM_COUNTERS_CMD_WAIT_VAL_FIELD_MASK 0xFFFFFFFF
#define CNPL_PM_COUNTERS_CMD_WAIT_VAL_FIELD_WIDTH 32
#define CNPL_PM_COUNTERS_CMD_WAIT_VAL_FIELD_SHIFT 0
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec CNPL_PM_COUNTERS_CMD_WAIT_VAL_FIELD;
#endif
extern const ru_reg_rec CNPL_PM_COUNTERS_CMD_WAIT_REG;
#define CNPL_PM_COUNTERS_CMD_WAIT_REG_OFFSET 0x00005680
#define CNPL_PM_COUNTERS_CMD_WAIT_REG_RAM_CNT 2

#define CNPL_PM_COUNTERS_TOT_CYC_VAL_FIELD_MASK 0xFFFFFFFF
#define CNPL_PM_COUNTERS_TOT_CYC_VAL_FIELD_WIDTH 32
#define CNPL_PM_COUNTERS_TOT_CYC_VAL_FIELD_SHIFT 0
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec CNPL_PM_COUNTERS_TOT_CYC_VAL_FIELD;
#endif
extern const ru_reg_rec CNPL_PM_COUNTERS_TOT_CYC_REG;
#define CNPL_PM_COUNTERS_TOT_CYC_REG_OFFSET 0x00005690
#define CNPL_PM_COUNTERS_TOT_CYC_REG_RAM_CNT 3

#define CNPL_PM_COUNTERS_GNT_CYC_VAL_FIELD_MASK 0xFFFFFFFF
#define CNPL_PM_COUNTERS_GNT_CYC_VAL_FIELD_WIDTH 32
#define CNPL_PM_COUNTERS_GNT_CYC_VAL_FIELD_SHIFT 0
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec CNPL_PM_COUNTERS_GNT_CYC_VAL_FIELD;
#endif
extern const ru_reg_rec CNPL_PM_COUNTERS_GNT_CYC_REG;
#define CNPL_PM_COUNTERS_GNT_CYC_REG_OFFSET 0x000056A0
#define CNPL_PM_COUNTERS_GNT_CYC_REG_RAM_CNT 3

#define CNPL_PM_COUNTERS_ARB_CYC_VAL_FIELD_MASK 0xFFFFFFFF
#define CNPL_PM_COUNTERS_ARB_CYC_VAL_FIELD_WIDTH 32
#define CNPL_PM_COUNTERS_ARB_CYC_VAL_FIELD_SHIFT 0
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec CNPL_PM_COUNTERS_ARB_CYC_VAL_FIELD;
#endif
extern const ru_reg_rec CNPL_PM_COUNTERS_ARB_CYC_REG;
#define CNPL_PM_COUNTERS_ARB_CYC_REG_OFFSET 0x000056B0
#define CNPL_PM_COUNTERS_ARB_CYC_REG_RAM_CNT 3

#define CNPL_PM_COUNTERS_PL_UP_ERR_VAL_FIELD_MASK 0xFFFFFFFF
#define CNPL_PM_COUNTERS_PL_UP_ERR_VAL_FIELD_WIDTH 32
#define CNPL_PM_COUNTERS_PL_UP_ERR_VAL_FIELD_SHIFT 0
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec CNPL_PM_COUNTERS_PL_UP_ERR_VAL_FIELD;
#endif
extern const ru_reg_rec CNPL_PM_COUNTERS_PL_UP_ERR_REG;
#define CNPL_PM_COUNTERS_PL_UP_ERR_REG_OFFSET 0x000056C0
#define CNPL_PM_COUNTERS_PL_UP_ERR_REG_RAM_CNT 4

#define CNPL_PM_COUNTERS_GEN_CFG_RD_CLR_FIELD_MASK 0x00000001
#define CNPL_PM_COUNTERS_GEN_CFG_RD_CLR_FIELD_WIDTH 1
#define CNPL_PM_COUNTERS_GEN_CFG_RD_CLR_FIELD_SHIFT 0
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec CNPL_PM_COUNTERS_GEN_CFG_RD_CLR_FIELD;
#endif
#define CNPL_PM_COUNTERS_GEN_CFG_WRAP_FIELD_MASK 0x00000002
#define CNPL_PM_COUNTERS_GEN_CFG_WRAP_FIELD_WIDTH 1
#define CNPL_PM_COUNTERS_GEN_CFG_WRAP_FIELD_SHIFT 1
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec CNPL_PM_COUNTERS_GEN_CFG_WRAP_FIELD;
#endif
extern const ru_reg_rec CNPL_PM_COUNTERS_GEN_CFG_REG;
#define CNPL_PM_COUNTERS_GEN_CFG_REG_OFFSET 0x000056FC

#define CNPL_DEBUG_DBGSEL_VS_FIELD_MASK 0x0000007F
#define CNPL_DEBUG_DBGSEL_VS_FIELD_WIDTH 7
#define CNPL_DEBUG_DBGSEL_VS_FIELD_SHIFT 0
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec CNPL_DEBUG_DBGSEL_VS_FIELD;
#endif
extern const ru_reg_rec CNPL_DEBUG_DBGSEL_REG;
#define CNPL_DEBUG_DBGSEL_REG_OFFSET 0x00005700

#define CNPL_DEBUG_DBGBUS_VB_FIELD_MASK 0x001FFFFF
#define CNPL_DEBUG_DBGBUS_VB_FIELD_WIDTH 21
#define CNPL_DEBUG_DBGBUS_VB_FIELD_SHIFT 0
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec CNPL_DEBUG_DBGBUS_VB_FIELD;
#endif
extern const ru_reg_rec CNPL_DEBUG_DBGBUS_REG;
#define CNPL_DEBUG_DBGBUS_REG_OFFSET 0x00005704

#define CNPL_DEBUG_REQ_VEC_REQ_FIELD_MASK 0x0000007F
#define CNPL_DEBUG_REQ_VEC_REQ_FIELD_WIDTH 7
#define CNPL_DEBUG_REQ_VEC_REQ_FIELD_SHIFT 0
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec CNPL_DEBUG_REQ_VEC_REQ_FIELD;
#endif
extern const ru_reg_rec CNPL_DEBUG_REQ_VEC_REG;
#define CNPL_DEBUG_REQ_VEC_REG_OFFSET 0x00005708

#define CNPL_DEBUG_POL_UP_ST_ITR_NUM_FIELD_MASK 0x000000FF
#define CNPL_DEBUG_POL_UP_ST_ITR_NUM_FIELD_WIDTH 8
#define CNPL_DEBUG_POL_UP_ST_ITR_NUM_FIELD_SHIFT 0
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec CNPL_DEBUG_POL_UP_ST_ITR_NUM_FIELD;
#endif
#define CNPL_DEBUG_POL_UP_ST_POL_NUM_FIELD_MASK 0x0000FF00
#define CNPL_DEBUG_POL_UP_ST_POL_NUM_FIELD_WIDTH 8
#define CNPL_DEBUG_POL_UP_ST_POL_NUM_FIELD_SHIFT 8
#if RU_INCLUDE_FIELD_DB
extern const ru_field_rec CNPL_DEBUG_POL_UP_ST_POL_NUM_FIELD;
#endif
extern const ru_reg_rec CNPL_DEBUG_POL_UP_ST_REG;
#define CNPL_DEBUG_POL_UP_ST_REG_OFFSET 0x00005710
#define CNPL_DEBUG_POL_UP_ST_REG_RAM_CNT 4

extern const ru_block_rec CNPL_BLOCK;

#endif
