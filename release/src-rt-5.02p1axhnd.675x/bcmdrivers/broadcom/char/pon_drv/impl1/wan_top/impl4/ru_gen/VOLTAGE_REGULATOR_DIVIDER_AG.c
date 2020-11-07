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
 * Field: VOLTAGE_REGULATOR_DIVIDER_IDER_RESERVED0
 ******************************************************************************/
const ru_field_rec VOLTAGE_REGULATOR_DIVIDER_IDER_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    VOLTAGE_REGULATOR_DIVIDER_IDER_RESERVED0_FIELD_MASK,
    0,
    VOLTAGE_REGULATOR_DIVIDER_IDER_RESERVED0_FIELD_WIDTH,
    VOLTAGE_REGULATOR_DIVIDER_IDER_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: VOLTAGE_REGULATOR_DIVIDER_IDER_CFG_VREG_CLK_BYPASS
 ******************************************************************************/
const ru_field_rec VOLTAGE_REGULATOR_DIVIDER_IDER_CFG_VREG_CLK_BYPASS_FIELD =
{
    "CFG_VREG_CLK_BYPASS",
#if RU_INCLUDE_DESC
    "",
    "Allows the bypassing of the N divider.",
#endif
    VOLTAGE_REGULATOR_DIVIDER_IDER_CFG_VREG_CLK_BYPASS_FIELD_MASK,
    0,
    VOLTAGE_REGULATOR_DIVIDER_IDER_CFG_VREG_CLK_BYPASS_FIELD_WIDTH,
    VOLTAGE_REGULATOR_DIVIDER_IDER_CFG_VREG_CLK_BYPASS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: VOLTAGE_REGULATOR_DIVIDER_IDER_CFG_VREG_CLK_SRC
 ******************************************************************************/
const ru_field_rec VOLTAGE_REGULATOR_DIVIDER_IDER_CFG_VREG_CLK_SRC_FIELD =
{
    "CFG_VREG_CLK_SRC",
#if RU_INCLUDE_DESC
    "",
    "Specifies the clock source of the voltage regulator sync output : 1"
    "- VDSL PHY; 0 - 50 MHz XTAL clock.",
#endif
    VOLTAGE_REGULATOR_DIVIDER_IDER_CFG_VREG_CLK_SRC_FIELD_MASK,
    0,
    VOLTAGE_REGULATOR_DIVIDER_IDER_CFG_VREG_CLK_SRC_FIELD_WIDTH,
    VOLTAGE_REGULATOR_DIVIDER_IDER_CFG_VREG_CLK_SRC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: VOLTAGE_REGULATOR_DIVIDER_IDER_CFG_VREG_DIV
 ******************************************************************************/
const ru_field_rec VOLTAGE_REGULATOR_DIVIDER_IDER_CFG_VREG_DIV_FIELD =
{
    "CFG_VREG_DIV",
#if RU_INCLUDE_DESC
    "",
    "N divider value of voltage regulator sync output.  Assertive 1 value"
    "will be INT((N/2); and assertive 0, N - INT(N/2).",
#endif
    VOLTAGE_REGULATOR_DIVIDER_IDER_CFG_VREG_DIV_FIELD_MASK,
    0,
    VOLTAGE_REGULATOR_DIVIDER_IDER_CFG_VREG_DIV_FIELD_WIDTH,
    VOLTAGE_REGULATOR_DIVIDER_IDER_CFG_VREG_DIV_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: VOLTAGE_REGULATOR_DIVIDER_IDER
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *VOLTAGE_REGULATOR_DIVIDER_IDER_FIELDS[] =
{
    &VOLTAGE_REGULATOR_DIVIDER_IDER_RESERVED0_FIELD,
    &VOLTAGE_REGULATOR_DIVIDER_IDER_CFG_VREG_CLK_BYPASS_FIELD,
    &VOLTAGE_REGULATOR_DIVIDER_IDER_CFG_VREG_CLK_SRC_FIELD,
    &VOLTAGE_REGULATOR_DIVIDER_IDER_CFG_VREG_DIV_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec VOLTAGE_REGULATOR_DIVIDER_IDER_REG = 
{
    "IDER",
#if RU_INCLUDE_DESC
    "WAN_VOLTAGE_REGULATOR_DIVIDER Register",
    "Provides the divider for the voltage regulator sync output.",
#endif
    VOLTAGE_REGULATOR_DIVIDER_IDER_REG_OFFSET,
    0,
    0,
    50,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    VOLTAGE_REGULATOR_DIVIDER_IDER_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: VOLTAGE_REGULATOR_DIVIDER
 ******************************************************************************/
static const ru_reg_rec *VOLTAGE_REGULATOR_DIVIDER_REGS[] =
{
    &VOLTAGE_REGULATOR_DIVIDER_IDER_REG,
};

unsigned long VOLTAGE_REGULATOR_DIVIDER_ADDRS[] =
{
    0x801440c8,
};

const ru_block_rec VOLTAGE_REGULATOR_DIVIDER_BLOCK = 
{
    "VOLTAGE_REGULATOR_DIVIDER",
    VOLTAGE_REGULATOR_DIVIDER_ADDRS,
    1,
    1,
    VOLTAGE_REGULATOR_DIVIDER_REGS
};

/* End of file VOLTAGE_REGULATOR_DIVIDER.c */
