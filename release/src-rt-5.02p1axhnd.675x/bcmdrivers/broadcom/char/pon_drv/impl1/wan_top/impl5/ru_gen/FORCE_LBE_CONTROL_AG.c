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
 * Field: FORCE_LBE_CONTROL_L_CFG_FORCE_LBE
 ******************************************************************************/
const ru_field_rec FORCE_LBE_CONTROL_L_CFG_FORCE_LBE_FIELD =
{
    "CFG_FORCE_LBE",
#if RU_INCLUDE_DESC
    "CFG_FORCE_LBE",
    "0: The MAC controls the LBE signal."
    "1: The LBE signal is forced to cfg_force_lbe_value.",
#endif
    FORCE_LBE_CONTROL_L_CFG_FORCE_LBE_FIELD_MASK,
    0,
    FORCE_LBE_CONTROL_L_CFG_FORCE_LBE_FIELD_WIDTH,
    FORCE_LBE_CONTROL_L_CFG_FORCE_LBE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FORCE_LBE_CONTROL_L_CFG_FORCE_LBE_VALUE
 ******************************************************************************/
const ru_field_rec FORCE_LBE_CONTROL_L_CFG_FORCE_LBE_VALUE_FIELD =
{
    "CFG_FORCE_LBE_VALUE",
#if RU_INCLUDE_DESC
    "CFG_FORCE_LBE_VALUE",
    "This field is only used when cfg_force_lbe is set.  0: LBE is set to 0. 1: LBE is set to 1.",
#endif
    FORCE_LBE_CONTROL_L_CFG_FORCE_LBE_VALUE_FIELD_MASK,
    0,
    FORCE_LBE_CONTROL_L_CFG_FORCE_LBE_VALUE_FIELD_WIDTH,
    FORCE_LBE_CONTROL_L_CFG_FORCE_LBE_VALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FORCE_LBE_CONTROL_L_CFG_FORCE_LBE_OE
 ******************************************************************************/
const ru_field_rec FORCE_LBE_CONTROL_L_CFG_FORCE_LBE_OE_FIELD =
{
    "CFG_FORCE_LBE_OE",
#if RU_INCLUDE_DESC
    "CFG_FORCE_LBE_OE",
    "0: The MAC and cr_xgwan_top_wan_misc_wan_cfg_laser_oe control the LBE output enable signal.  1: The LBE output enable signal is forced to cfg_force_lbe_oe_value.",
#endif
    FORCE_LBE_CONTROL_L_CFG_FORCE_LBE_OE_FIELD_MASK,
    0,
    FORCE_LBE_CONTROL_L_CFG_FORCE_LBE_OE_FIELD_WIDTH,
    FORCE_LBE_CONTROL_L_CFG_FORCE_LBE_OE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FORCE_LBE_CONTROL_L_CFG_FORCE_LBE_OE_VALUE
 ******************************************************************************/
const ru_field_rec FORCE_LBE_CONTROL_L_CFG_FORCE_LBE_OE_VALUE_FIELD =
{
    "CFG_FORCE_LBE_OE_VALUE",
#if RU_INCLUDE_DESC
    "CFG_FORCE_LBE_OE_VALUE",
    "This field is only used when cfg_force_lbe_oe is set.  This signal is then inverted prior to connecting to the OEB pin.  0: LBE output enable is set to 0. 1: LBE output enable is set to 1.",
#endif
    FORCE_LBE_CONTROL_L_CFG_FORCE_LBE_OE_VALUE_FIELD_MASK,
    0,
    FORCE_LBE_CONTROL_L_CFG_FORCE_LBE_OE_VALUE_FIELD_WIDTH,
    FORCE_LBE_CONTROL_L_CFG_FORCE_LBE_OE_VALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: FORCE_LBE_CONTROL_L_RESERVED0
 ******************************************************************************/
const ru_field_rec FORCE_LBE_CONTROL_L_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    FORCE_LBE_CONTROL_L_RESERVED0_FIELD_MASK,
    0,
    FORCE_LBE_CONTROL_L_RESERVED0_FIELD_WIDTH,
    FORCE_LBE_CONTROL_L_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: FORCE_LBE_CONTROL_L
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *FORCE_LBE_CONTROL_L_FIELDS[] =
{
    &FORCE_LBE_CONTROL_L_CFG_FORCE_LBE_FIELD,
    &FORCE_LBE_CONTROL_L_CFG_FORCE_LBE_VALUE_FIELD,
    &FORCE_LBE_CONTROL_L_CFG_FORCE_LBE_OE_FIELD,
    &FORCE_LBE_CONTROL_L_CFG_FORCE_LBE_OE_VALUE_FIELD,
    &FORCE_LBE_CONTROL_L_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec FORCE_LBE_CONTROL_L_REG = 
{
    "L",
#if RU_INCLUDE_DESC
    "FORCE_LBE_CONTROL Register",
    "Register used to force the laser burst enable (LBE) and LBE output enable signals.",
#endif
    FORCE_LBE_CONTROL_L_REG_OFFSET,
    0,
    0,
    27,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    FORCE_LBE_CONTROL_L_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: FORCE_LBE_CONTROL
 ******************************************************************************/
static const ru_reg_rec *FORCE_LBE_CONTROL_REGS[] =
{
    &FORCE_LBE_CONTROL_L_REG,
};

unsigned long FORCE_LBE_CONTROL_ADDRS[] =
{
    0x82db209c,
};

const ru_block_rec FORCE_LBE_CONTROL_BLOCK = 
{
    "FORCE_LBE_CONTROL",
    FORCE_LBE_CONTROL_ADDRS,
    1,
    1,
    FORCE_LBE_CONTROL_REGS
};

/* End of file FORCE_LBE_CONTROL.c */
