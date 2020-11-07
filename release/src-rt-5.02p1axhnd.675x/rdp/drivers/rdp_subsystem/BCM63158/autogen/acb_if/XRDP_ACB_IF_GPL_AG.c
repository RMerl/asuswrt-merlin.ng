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
 * Register: ACB_IF_ACBIF_BLOCK_ACBIF_CONFIG_CONF0
 ******************************************************************************/
const ru_reg_rec ACB_IF_ACBIF_BLOCK_ACBIF_CONFIG_CONF0_REG = 
{
    "ACBIF_BLOCK_ACBIF_CONFIG_CONF0",
#if RU_INCLUDE_DESC
    "CONFIG0 Register",
    "misc configs 0",
#endif
    ACB_IF_ACBIF_BLOCK_ACBIF_CONFIG_CONF0_REG_OFFSET,
    0,
    0,
    961,
};

/******************************************************************************
 * Register: ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_CMD_TYPE
 ******************************************************************************/
const ru_reg_rec ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_CMD_TYPE_REG = 
{
    "ACBIF_BLOCK_ACBIF_PM_COUNTERS_CMD_TYPE",
#if RU_INCLUDE_DESC
    "CMD_TYPE_CNTR %i Register",
    "Number of commands that were processed for each command type (order - intend, sent, stat).",
#endif
    ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_CMD_TYPE_REG_OFFSET,
    ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_CMD_TYPE_REG_RAM_CNT,
    4,
    962,
};

/******************************************************************************
 * Register: ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_CMD_IMP
 ******************************************************************************/
const ru_reg_rec ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_CMD_IMP_REG = 
{
    "ACBIF_BLOCK_ACBIF_PM_COUNTERS_CMD_IMP",
#if RU_INCLUDE_DESC
    "CMD_IMP_CNTR %i Register",
    "Number of commands that were processed for each IMP(0,1,2).e.",
#endif
    ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_CMD_IMP_REG_OFFSET,
    ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_CMD_IMP_REG_RAM_CNT,
    4,
    963,
};

/******************************************************************************
 * Register: ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_AGG
 ******************************************************************************/
const ru_reg_rec ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_AGG_REG = 
{
    "ACBIF_BLOCK_ACBIF_PM_COUNTERS_AGG",
#if RU_INCLUDE_DESC
    "AGG_CNTR %i Register",
    "Number of commands (for each of - intend, sent) that were for aggregated packets.",
#endif
    ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_AGG_REG_OFFSET,
    ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_AGG_REG_RAM_CNT,
    4,
    964,
};

/******************************************************************************
 * Register: ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_BUFFS
 ******************************************************************************/
const ru_reg_rec ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_BUFFS_REG = 
{
    "ACBIF_BLOCK_ACBIF_PM_COUNTERS_BUFFS",
#if RU_INCLUDE_DESC
    "BUFS_NUM_CNTR %i Register",
    "Number of buffers that were counted for each command(order - intend, sent).",
#endif
    ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_BUFFS_REG_OFFSET,
    ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_BUFFS_REG_RAM_CNT,
    4,
    965,
};

/******************************************************************************
 * Register: ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_GEN_CFG
 ******************************************************************************/
const ru_reg_rec ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_GEN_CFG_REG = 
{
    "ACBIF_BLOCK_ACBIF_PM_COUNTERS_GEN_CFG",
#if RU_INCLUDE_DESC
    "GENERAL_CONFIG Register",
    "bits rd_clr and wrap for the pm counters(above)",
#endif
    ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_GEN_CFG_REG_OFFSET,
    0,
    0,
    966,
};

/******************************************************************************
 * Register: ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_DBGSEL
 ******************************************************************************/
const ru_reg_rec ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_DBGSEL_REG = 
{
    "ACBIF_BLOCK_ACBIF_DEBUG_DBGSEL",
#if RU_INCLUDE_DESC
    "DBG_MUX_SEL Register",
    "selects the debug vecore",
#endif
    ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_DBGSEL_REG_OFFSET,
    0,
    0,
    967,
};

/******************************************************************************
 * Register: ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_DBGBUS
 ******************************************************************************/
const ru_reg_rec ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_DBGBUS_REG = 
{
    "ACBIF_BLOCK_ACBIF_DEBUG_DBGBUS",
#if RU_INCLUDE_DESC
    "DBG_BUS Register",
    "the debug bus",
#endif
    ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_DBGBUS_REG_OFFSET,
    0,
    0,
    968,
};

/******************************************************************************
 * Register: ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_STAT
 ******************************************************************************/
const ru_reg_rec ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_STAT_REG = 
{
    "ACBIF_BLOCK_ACBIF_DEBUG_STAT",
#if RU_INCLUDE_DESC
    "STATUS %i Register",
    "status register (msb, lsb)",
#endif
    ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_STAT_REG_OFFSET,
    ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_STAT_REG_RAM_CNT,
    4,
    969,
};

/******************************************************************************
 * Block: ACB_IF
 ******************************************************************************/
static const ru_reg_rec *ACB_IF_REGS[] =
{
    &ACB_IF_ACBIF_BLOCK_ACBIF_CONFIG_CONF0_REG,
    &ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_CMD_TYPE_REG,
    &ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_CMD_IMP_REG,
    &ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_AGG_REG,
    &ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_BUFFS_REG,
    &ACB_IF_ACBIF_BLOCK_ACBIF_PM_COUNTERS_GEN_CFG_REG,
    &ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_DBGSEL_REG,
    &ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_DBGBUS_REG,
    &ACB_IF_ACBIF_BLOCK_ACBIF_DEBUG_STAT_REG,
};

unsigned long ACB_IF_ADDRS[] =
{
    0x82e50800,
};

const ru_block_rec ACB_IF_BLOCK = 
{
    "ACB_IF",
    ACB_IF_ADDRS,
    1,
    9,
    ACB_IF_REGS
};

/* End of file XRDP_ACB_IF.c */
