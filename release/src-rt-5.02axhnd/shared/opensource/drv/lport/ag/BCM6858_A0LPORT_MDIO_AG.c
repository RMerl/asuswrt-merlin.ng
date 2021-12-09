/*
   Copyright (c) 2015 Broadcom Corporation
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
 * Field: LPORT_MDIO_CMD_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_MDIO_CMD_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_MDIO_CMD_RESERVED0_FIELD_MASK,
    0,
    LPORT_MDIO_CMD_RESERVED0_FIELD_WIDTH,
    LPORT_MDIO_CMD_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_MDIO_CMD_START_BUSY
 ******************************************************************************/
const ru_field_rec LPORT_MDIO_CMD_START_BUSY_FIELD =
{
    "START_BUSY",
#if RU_INCLUDE_DESC
    "",
    "START_BUSY, Self-clearing, CPU writes this bit to 1 in order to initiate MDIO transaction. When transaction completes hardware will clear this bit.",
#endif
    LPORT_MDIO_CMD_START_BUSY_FIELD_MASK,
    0,
    LPORT_MDIO_CMD_START_BUSY_FIELD_WIDTH,
    LPORT_MDIO_CMD_START_BUSY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_MDIO_CMD_FAIL
 ******************************************************************************/
const ru_field_rec LPORT_MDIO_CMD_FAIL_FIELD =
{
    "FAIL",
#if RU_INCLUDE_DESC
    "",
    "This bit is set when PHY does not reply to READ command (PHY does not drive 0 on bus turnaround).",
#endif
    LPORT_MDIO_CMD_FAIL_FIELD_MASK,
    0,
    LPORT_MDIO_CMD_FAIL_FIELD_WIDTH,
    LPORT_MDIO_CMD_FAIL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_MDIO_CMD_OP_CODE
 ******************************************************************************/
const ru_field_rec LPORT_MDIO_CMD_OP_CODE_FIELD =
{
    "OP_CODE",
#if RU_INCLUDE_DESC
    "",
    "MDIO command that is OP[1:0]:\n"
    "00 - Address for clause 45.\n"
    "01 - Write.\n"
    "10 - Read increment for clause 45.\n"
    "11 - Read for clause 45.\n"
    "10 - Read for clause 22.\n",
#endif
    LPORT_MDIO_CMD_OP_CODE_FIELD_MASK,
    0,
    LPORT_MDIO_CMD_OP_CODE_FIELD_WIDTH,
    LPORT_MDIO_CMD_OP_CODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_MDIO_CMD_PHY_PRT_ADDR
 ******************************************************************************/
const ru_field_rec LPORT_MDIO_CMD_PHY_PRT_ADDR_FIELD =
{
    "PHY_PRT_ADDR",
#if RU_INCLUDE_DESC
    "",
    "PHY address[4:0] for clause 22, Port address[4:0] for Clause 45.",
#endif
    LPORT_MDIO_CMD_PHY_PRT_ADDR_FIELD_MASK,
    0,
    LPORT_MDIO_CMD_PHY_PRT_ADDR_FIELD_WIDTH,
    LPORT_MDIO_CMD_PHY_PRT_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_MDIO_CMD_REG_DEV_ADDR
 ******************************************************************************/
const ru_field_rec LPORT_MDIO_CMD_REG_DEV_ADDR_FIELD =
{
    "REG_DEV_ADDR",
#if RU_INCLUDE_DESC
    "",
    "Register address[4:0] for clause 22, Device address[4:0] for Clause 45.",
#endif
    LPORT_MDIO_CMD_REG_DEV_ADDR_FIELD_MASK,
    0,
    LPORT_MDIO_CMD_REG_DEV_ADDR_FIELD_WIDTH,
    LPORT_MDIO_CMD_REG_DEV_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_MDIO_CMD_DATA_ADDR
 ******************************************************************************/
const ru_field_rec LPORT_MDIO_CMD_DATA_ADDR_FIELD =
{
    "DATA_ADDR",
#if RU_INCLUDE_DESC
    "",
    "MDIO Read/Write data[15:0], clause 22 and 45 or MDIO address[15:0] for clause 45\".",
#endif
    LPORT_MDIO_CMD_DATA_ADDR_FIELD_MASK,
    0,
    LPORT_MDIO_CMD_DATA_ADDR_FIELD_WIDTH,
    LPORT_MDIO_CMD_DATA_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_MDIO_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_MDIO_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_MDIO_CFG_RESERVED0_FIELD_MASK,
    0,
    LPORT_MDIO_CFG_RESERVED0_FIELD_WIDTH,
    LPORT_MDIO_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_MDIO_CFG_SUPRESS_PREAMBLE
 ******************************************************************************/
const ru_field_rec LPORT_MDIO_CFG_SUPRESS_PREAMBLE_FIELD =
{
    "SUPRESS_PREAMBLE",
#if RU_INCLUDE_DESC
    "",
    "When this bit set, preamble (32 consectutive 1's) is suppressed for MDIO transaction that is MDIO transaction starts with ST.",
#endif
    LPORT_MDIO_CFG_SUPRESS_PREAMBLE_FIELD_MASK,
    0,
    LPORT_MDIO_CFG_SUPRESS_PREAMBLE_FIELD_WIDTH,
    LPORT_MDIO_CFG_SUPRESS_PREAMBLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_MDIO_CFG_FREE_RUN_CLK_ENABLE
 ******************************************************************************/
const ru_field_rec LPORT_MDIO_CFG_FREE_RUN_CLK_ENABLE_FIELD =
{
    "FREE_RUN_CLK_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "When this bit set, MDIO clock is enabled and running.",
#endif
    LPORT_MDIO_CFG_FREE_RUN_CLK_ENABLE_FIELD_MASK,
    0,
    LPORT_MDIO_CFG_FREE_RUN_CLK_ENABLE_FIELD_WIDTH,
    LPORT_MDIO_CFG_FREE_RUN_CLK_ENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_MDIO_CFG_MDIO_CLK_DIVIDER
 ******************************************************************************/
const ru_field_rec LPORT_MDIO_CFG_MDIO_CLK_DIVIDER_FIELD =
{
    "MDIO_CLK_DIVIDER",
#if RU_INCLUDE_DESC
    "",
    "MDIO clock divider[7:0], Reference clock (typically 250 MHz) is divided by 2x(MDIO_CLK_DIVIDER+1) to generate MDIO clock(MDC).",
#endif
    LPORT_MDIO_CFG_MDIO_CLK_DIVIDER_FIELD_MASK,
    0,
    LPORT_MDIO_CFG_MDIO_CLK_DIVIDER_FIELD_WIDTH,
    LPORT_MDIO_CFG_MDIO_CLK_DIVIDER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_MDIO_CFG_RESERVED2
 ******************************************************************************/
const ru_field_rec LPORT_MDIO_CFG_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_MDIO_CFG_RESERVED2_FIELD_MASK,
    0,
    LPORT_MDIO_CFG_RESERVED2_FIELD_WIDTH,
    LPORT_MDIO_CFG_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_MDIO_CFG_MDIO_CLAUSE
 ******************************************************************************/
const ru_field_rec LPORT_MDIO_CFG_MDIO_CLAUSE_FIELD =
{
    "MDIO_CLAUSE",
#if RU_INCLUDE_DESC
    "",
    "0: Clause 45.\n"
    "1: Clause 22.",
#endif
    LPORT_MDIO_CFG_MDIO_CLAUSE_FIELD_MASK,
    0,
    LPORT_MDIO_CFG_MDIO_CLAUSE_FIELD_WIDTH,
    LPORT_MDIO_CFG_MDIO_CLAUSE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: LPORT_MDIO_CMD
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_MDIO_CMD_FIELDS[] =
{
    &LPORT_MDIO_CMD_RESERVED0_FIELD,
    &LPORT_MDIO_CMD_START_BUSY_FIELD,
    &LPORT_MDIO_CMD_FAIL_FIELD,
    &LPORT_MDIO_CMD_OP_CODE_FIELD,
    &LPORT_MDIO_CMD_PHY_PRT_ADDR_FIELD,
    &LPORT_MDIO_CMD_REG_DEV_ADDR_FIELD,
    &LPORT_MDIO_CMD_DATA_ADDR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_MDIO_CMD_REG = 
{
    "CMD",
#if RU_INCLUDE_DESC
    "MDIO Command Register",
    "",
#endif
    LPORT_MDIO_CMD_REG_OFFSET,
    0,
    0,
    269,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    LPORT_MDIO_CMD_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LPORT_MDIO_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_MDIO_CFG_FIELDS[] =
{
    &LPORT_MDIO_CFG_RESERVED0_FIELD,
    &LPORT_MDIO_CFG_FREE_RUN_CLK_ENABLE_FIELD,
    &LPORT_MDIO_CFG_SUPRESS_PREAMBLE_FIELD,
    &LPORT_MDIO_CFG_MDIO_CLK_DIVIDER_FIELD,
    &LPORT_MDIO_CFG_RESERVED2_FIELD,
    &LPORT_MDIO_CFG_MDIO_CLAUSE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_MDIO_CFG_REG = 
{
    "CFG",
#if RU_INCLUDE_DESC
    "MDIO Configuration Register",
    "",
#endif
    LPORT_MDIO_CFG_REG_OFFSET,
    0,
    0,
    270,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    LPORT_MDIO_CFG_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: LPORT_MDIO
 ******************************************************************************/
static const ru_reg_rec *LPORT_MDIO_REGS[] =
{
    &LPORT_MDIO_CMD_REG,
    &LPORT_MDIO_CFG_REG,
};

unsigned long LPORT_MDIO_ADDRS[] =
{
    0x8013d400,
};

const ru_block_rec LPORT_MDIO_BLOCK = 
{
    "LPORT_MDIO",
    LPORT_MDIO_ADDRS,
    1,
    2,
    LPORT_MDIO_REGS
};

/* End of file BCM6858_A0LPORT_MDIO.c */
