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
 * Register: RNR_REGS_CFG_GLOBAL_CTRL
 ******************************************************************************/
const ru_reg_rec RNR_REGS_CFG_GLOBAL_CTRL_REG = 
{
    "CFG_GLOBAL_CTRL",
#if RU_INCLUDE_DESC
    "GLOBAL_CONTROL Register",
    "Global control",
#endif
    RNR_REGS_CFG_GLOBAL_CTRL_REG_OFFSET,
    0,
    0,
    238,
};

/******************************************************************************
 * Register: RNR_REGS_CFG_CPU_WAKEUP
 ******************************************************************************/
const ru_reg_rec RNR_REGS_CFG_CPU_WAKEUP_REG = 
{
    "CFG_CPU_WAKEUP",
#if RU_INCLUDE_DESC
    "CPU_WAKEUP Register",
    "Writing to this register generates a request towards the runner scheduler.",
#endif
    RNR_REGS_CFG_CPU_WAKEUP_REG_OFFSET,
    0,
    0,
    239,
};

/******************************************************************************
 * Register: RNR_REGS_CFG_INT_CTRL
 ******************************************************************************/
const ru_reg_rec RNR_REGS_CFG_INT_CTRL_REG = 
{
    "CFG_INT_CTRL",
#if RU_INCLUDE_DESC
    "INTERRUPT_CONTROL Register",
    "Interrupt control - UNUSED in 6858",
#endif
    RNR_REGS_CFG_INT_CTRL_REG_OFFSET,
    0,
    0,
    240,
};

/******************************************************************************
 * Register: RNR_REGS_CFG_INT_MASK
 ******************************************************************************/
const ru_reg_rec RNR_REGS_CFG_INT_MASK_REG = 
{
    "CFG_INT_MASK",
#if RU_INCLUDE_DESC
    "INTERRUPT_MASK Register",
    "Interrupt mask -  UNUSED in 6858",
#endif
    RNR_REGS_CFG_INT_MASK_REG_OFFSET,
    0,
    0,
    241,
};

/******************************************************************************
 * Register: RNR_REGS_CFG_GEN_CFG
 ******************************************************************************/
const ru_reg_rec RNR_REGS_CFG_GEN_CFG_REG = 
{
    "CFG_GEN_CFG",
#if RU_INCLUDE_DESC
    "GENERAL_CONFIGURATION Register",
    "General configuration",
#endif
    RNR_REGS_CFG_GEN_CFG_REG_OFFSET,
    0,
    0,
    242,
};

/******************************************************************************
 * Register: RNR_REGS_CFG_CAM_CFG
 ******************************************************************************/
const ru_reg_rec RNR_REGS_CFG_CAM_CFG_REG = 
{
    "CFG_CAM_CFG",
#if RU_INCLUDE_DESC
    "CAM_CONFIGURATION Register",
    "CAM configuration",
#endif
    RNR_REGS_CFG_CAM_CFG_REG_OFFSET,
    0,
    0,
    243,
};

/******************************************************************************
 * Register: RNR_REGS_CFG_DDR_CFG
 ******************************************************************************/
const ru_reg_rec RNR_REGS_CFG_DDR_CFG_REG = 
{
    "CFG_DDR_CFG",
#if RU_INCLUDE_DESC
    "DMA_DDR_CONFIG Register",
    "DMA DDR config Register. Contains configurations such as buffer size and ddr base address that are used for DDR address calculations (from buffer number) when DMA instruction addr_calc flag is set.",
#endif
    RNR_REGS_CFG_DDR_CFG_REG_OFFSET,
    0,
    0,
    244,
};

/******************************************************************************
 * Register: RNR_REGS_CFG_PSRAM_CFG
 ******************************************************************************/
const ru_reg_rec RNR_REGS_CFG_PSRAM_CFG_REG = 
{
    "CFG_PSRAM_CFG",
#if RU_INCLUDE_DESC
    "DMA_PSRAM_CONFIG Register",
    "DMA PSRAM config Register. Contains configurations such as buffer size and ddr base address that are used for DDR address calculations (from buffer number) when DMA instruction addr_calc flag is set.",
#endif
    RNR_REGS_CFG_PSRAM_CFG_REG_OFFSET,
    0,
    0,
    245,
};

/******************************************************************************
 * Register: RNR_REGS_CFG_RAMRD_RANGE_MASK_CFG
 ******************************************************************************/
const ru_reg_rec RNR_REGS_CFG_RAMRD_RANGE_MASK_CFG_REG = 
{
    "CFG_RAMRD_RANGE_MASK_CFG",
#if RU_INCLUDE_DESC
    "RAMRD_MASK_CONFIG Register",
    "Ramrd mask for range search. The register holds 2 mask that can be chosen by runner core for range seraches.",
#endif
    RNR_REGS_CFG_RAMRD_RANGE_MASK_CFG_REG_OFFSET,
    0,
    0,
    246,
};

/******************************************************************************
 * Register: RNR_REGS_CFG_SCH_CFG
 ******************************************************************************/
const ru_reg_rec RNR_REGS_CFG_SCH_CFG_REG = 
{
    "CFG_SCH_CFG",
#if RU_INCLUDE_DESC
    "SCHEDULER_CONFIG Register",
    "scheduler configuration",
#endif
    RNR_REGS_CFG_SCH_CFG_REG_OFFSET,
    0,
    0,
    247,
};

/******************************************************************************
 * Register: RNR_REGS_CFG_BKPT_CFG
 ******************************************************************************/
const ru_reg_rec RNR_REGS_CFG_BKPT_CFG_REG = 
{
    "CFG_BKPT_CFG",
#if RU_INCLUDE_DESC
    "BKPT_CFG Register",
    "breakpoint configuration",
#endif
    RNR_REGS_CFG_BKPT_CFG_REG_OFFSET,
    0,
    0,
    248,
};

/******************************************************************************
 * Register: RNR_REGS_CFG_BKPT_IMM
 ******************************************************************************/
const ru_reg_rec RNR_REGS_CFG_BKPT_IMM_REG = 
{
    "CFG_BKPT_IMM",
#if RU_INCLUDE_DESC
    "BKPT_IMMEDIATE Register",
    "break point immediate",
#endif
    RNR_REGS_CFG_BKPT_IMM_REG_OFFSET,
    0,
    0,
    249,
};

/******************************************************************************
 * Register: RNR_REGS_CFG_BKPT_STS
 ******************************************************************************/
const ru_reg_rec RNR_REGS_CFG_BKPT_STS_REG = 
{
    "CFG_BKPT_STS",
#if RU_INCLUDE_DESC
    "BKPT_STS Register",
    "breakpoint status",
#endif
    RNR_REGS_CFG_BKPT_STS_REG_OFFSET,
    0,
    0,
    250,
};

/******************************************************************************
 * Register: RNR_REGS_CFG_PC_STS
 ******************************************************************************/
const ru_reg_rec RNR_REGS_CFG_PC_STS_REG = 
{
    "CFG_PC_STS",
#if RU_INCLUDE_DESC
    "PC_STS Register",
    "Program counterstatus",
#endif
    RNR_REGS_CFG_PC_STS_REG_OFFSET,
    0,
    0,
    251,
};

/******************************************************************************
 * Register: RNR_REGS_CFG_PROFILING_STS
 ******************************************************************************/
const ru_reg_rec RNR_REGS_CFG_PROFILING_STS_REG = 
{
    "CFG_PROFILING_STS",
#if RU_INCLUDE_DESC
    "PROFILING_STS Register",
    "profiling status",
#endif
    RNR_REGS_CFG_PROFILING_STS_REG_OFFSET,
    0,
    0,
    252,
};

/******************************************************************************
 * Register: RNR_REGS_CFG_PROFILING_CFG_0
 ******************************************************************************/
const ru_reg_rec RNR_REGS_CFG_PROFILING_CFG_0_REG = 
{
    "CFG_PROFILING_CFG_0",
#if RU_INCLUDE_DESC
    "PROFILING_CFG_0 Register",
    "profiling confuguration 0",
#endif
    RNR_REGS_CFG_PROFILING_CFG_0_REG_OFFSET,
    0,
    0,
    253,
};

/******************************************************************************
 * Register: RNR_REGS_CFG_PROFILING_CFG_1
 ******************************************************************************/
const ru_reg_rec RNR_REGS_CFG_PROFILING_CFG_1_REG = 
{
    "CFG_PROFILING_CFG_1",
#if RU_INCLUDE_DESC
    "PROFILING_CFG_1 Register",
    "profiling confuguration 1",
#endif
    RNR_REGS_CFG_PROFILING_CFG_1_REG_OFFSET,
    0,
    0,
    254,
};

/******************************************************************************
 * Register: RNR_REGS_CFG_STALL_CNT1
 ******************************************************************************/
const ru_reg_rec RNR_REGS_CFG_STALL_CNT1_REG = 
{
    "CFG_STALL_CNT1",
#if RU_INCLUDE_DESC
    "STALL_CNT1 Register",
    "stall count",
#endif
    RNR_REGS_CFG_STALL_CNT1_REG_OFFSET,
    0,
    0,
    255,
};

/******************************************************************************
 * Register: RNR_REGS_CFG_STALL_CNT2
 ******************************************************************************/
const ru_reg_rec RNR_REGS_CFG_STALL_CNT2_REG = 
{
    "CFG_STALL_CNT2",
#if RU_INCLUDE_DESC
    "STALL_CNT2 Register",
    "stall count",
#endif
    RNR_REGS_CFG_STALL_CNT2_REG_OFFSET,
    0,
    0,
    256,
};

/******************************************************************************
 * Register: RNR_REGS_CFG_IDLE_CNT1
 ******************************************************************************/
const ru_reg_rec RNR_REGS_CFG_IDLE_CNT1_REG = 
{
    "CFG_IDLE_CNT1",
#if RU_INCLUDE_DESC
    "IDLE_CNT1 Register",
    "idle count",
#endif
    RNR_REGS_CFG_IDLE_CNT1_REG_OFFSET,
    0,
    0,
    257,
};

/******************************************************************************
 * Register: RNR_REGS_CFG_JMP_CNT
 ******************************************************************************/
const ru_reg_rec RNR_REGS_CFG_JMP_CNT_REG = 
{
    "CFG_JMP_CNT",
#if RU_INCLUDE_DESC
    "JUMP_CNT Register",
    "Mispredicted jumps count",
#endif
    RNR_REGS_CFG_JMP_CNT_REG_OFFSET,
    0,
    0,
    258,
};

/******************************************************************************
 * Register: RNR_REGS_CFG_METAL_FIX_REG
 ******************************************************************************/
const ru_reg_rec RNR_REGS_CFG_METAL_FIX_REG_REG = 
{
    "CFG_METAL_FIX_REG",
#if RU_INCLUDE_DESC
    "METAL_FIX Register",
    "32 bit register for metal fixes.",
#endif
    RNR_REGS_CFG_METAL_FIX_REG_REG_OFFSET,
    0,
    0,
    259,
};

/******************************************************************************
 * Register: RNR_REGS_DBG_DESIGN_DBG_CTRL
 ******************************************************************************/
const ru_reg_rec RNR_REGS_DBG_DESIGN_DBG_CTRL_REG = 
{
    "DBG_DESIGN_DBG_CTRL",
#if RU_INCLUDE_DESC
    "DESIGN_DEBUG_ADDRESS Register",
    "Design Debug address",
#endif
    RNR_REGS_DBG_DESIGN_DBG_CTRL_REG_OFFSET,
    0,
    0,
    260,
};

/******************************************************************************
 * Register: RNR_REGS_DBG_DESIGN_DBG_DATA
 ******************************************************************************/
const ru_reg_rec RNR_REGS_DBG_DESIGN_DBG_DATA_REG = 
{
    "DBG_DESIGN_DBG_DATA",
#if RU_INCLUDE_DESC
    "DESIGN_DEBUG_DATA Register",
    "Design Debug Data",
#endif
    RNR_REGS_DBG_DESIGN_DBG_DATA_REG_OFFSET,
    0,
    0,
    261,
};

/******************************************************************************
 * Block: RNR_REGS
 ******************************************************************************/
static const ru_reg_rec *RNR_REGS_REGS[] =
{
    &RNR_REGS_CFG_GLOBAL_CTRL_REG,
    &RNR_REGS_CFG_CPU_WAKEUP_REG,
    &RNR_REGS_CFG_INT_CTRL_REG,
    &RNR_REGS_CFG_INT_MASK_REG,
    &RNR_REGS_CFG_GEN_CFG_REG,
    &RNR_REGS_CFG_CAM_CFG_REG,
    &RNR_REGS_CFG_DDR_CFG_REG,
    &RNR_REGS_CFG_PSRAM_CFG_REG,
    &RNR_REGS_CFG_RAMRD_RANGE_MASK_CFG_REG,
    &RNR_REGS_CFG_SCH_CFG_REG,
    &RNR_REGS_CFG_BKPT_CFG_REG,
    &RNR_REGS_CFG_BKPT_IMM_REG,
    &RNR_REGS_CFG_BKPT_STS_REG,
    &RNR_REGS_CFG_PC_STS_REG,
    &RNR_REGS_CFG_PROFILING_STS_REG,
    &RNR_REGS_CFG_PROFILING_CFG_0_REG,
    &RNR_REGS_CFG_PROFILING_CFG_1_REG,
    &RNR_REGS_CFG_STALL_CNT1_REG,
    &RNR_REGS_CFG_STALL_CNT2_REG,
    &RNR_REGS_CFG_IDLE_CNT1_REG,
    &RNR_REGS_CFG_JMP_CNT_REG,
    &RNR_REGS_CFG_METAL_FIX_REG_REG,
    &RNR_REGS_DBG_DESIGN_DBG_CTRL_REG,
    &RNR_REGS_DBG_DESIGN_DBG_DATA_REG,
};

unsigned long RNR_REGS_ADDRS[] =
{
    0x82280000,
    0x82281000,
    0x82282000,
    0x82283000,
    0x82380000,
    0x82381000,
    0x82382000,
    0x82383000,
    0x82480000,
    0x82481000,
    0x82482000,
    0x82483000,
    0x82580000,
    0x82581000,
    0x82582000,
    0x82583000,
};

const ru_block_rec RNR_REGS_BLOCK = 
{
    "RNR_REGS",
    RNR_REGS_ADDRS,
    16,
    24,
    RNR_REGS_REGS
};

/* End of file XRDP_RNR_REGS.c */
