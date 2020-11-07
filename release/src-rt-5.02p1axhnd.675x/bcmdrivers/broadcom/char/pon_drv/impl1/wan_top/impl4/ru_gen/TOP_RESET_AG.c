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
 * Field: TOP_RESET_T_RESERVED0
 ******************************************************************************/
const ru_field_rec TOP_RESET_T_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    TOP_RESET_T_RESERVED0_FIELD_MASK,
    0,
    TOP_RESET_T_RESERVED0_FIELD_WIDTH,
    TOP_RESET_T_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TOP_RESET_T_CFG_PCS_RESET_N
 ******************************************************************************/
const ru_field_rec TOP_RESET_T_CFG_PCS_RESET_N_FIELD =
{
    "CFG_PCS_RESET_N",
#if RU_INCLUDE_DESC
    "",
    "Active low PCS reset. Set to 1 for normal operation",
#endif
    TOP_RESET_T_CFG_PCS_RESET_N_FIELD_MASK,
    0,
    TOP_RESET_T_CFG_PCS_RESET_N_FIELD_WIDTH,
    TOP_RESET_T_CFG_PCS_RESET_N_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: TOP_RESET_T
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TOP_RESET_T_FIELDS[] =
{
    &TOP_RESET_T_RESERVED0_FIELD,
    &TOP_RESET_T_CFG_PCS_RESET_N_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec TOP_RESET_T_REG = 
{
    "T",
#if RU_INCLUDE_DESC
    "WAN_TOP_RESET Register",
    "Various resets to be applied within wan_top.",
#endif
    TOP_RESET_T_REG_OFFSET,
    0,
    0,
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    TOP_RESET_T_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: TOP_RESET
 ******************************************************************************/
static const ru_reg_rec *TOP_RESET_REGS[] =
{
    &TOP_RESET_T_REG,
};

unsigned long TOP_RESET_ADDRS[] =
{
    0x80144004,
};

const ru_block_rec TOP_RESET_BLOCK = 
{
    "TOP_RESET",
    TOP_RESET_ADDRS,
    1,
    1,
    TOP_RESET_REGS
};

/* End of file TOP_RESET.c */
