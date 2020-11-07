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
 * Field: RESCAL_CFG_CTRL
 ******************************************************************************/
const ru_field_rec RESCAL_CFG_CTRL_FIELD =
{
    "CTRL",
#if RU_INCLUDE_DESC
    "CTRL",
    "Connects to i_rescal_ctrl."
    "Reset value is 0x0.",
#endif
    RESCAL_CFG_CTRL_FIELD_MASK,
    0,
    RESCAL_CFG_CTRL_FIELD_WIDTH,
    RESCAL_CFG_CTRL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RESCAL_CFG_PWRDN
 ******************************************************************************/
const ru_field_rec RESCAL_CFG_PWRDN_FIELD =
{
    "PWRDN",
#if RU_INCLUDE_DESC
    "PWRDN",
    "Connects to i_pwrdn."
    "Reset value is 0x1.",
#endif
    RESCAL_CFG_PWRDN_FIELD_MASK,
    0,
    RESCAL_CFG_PWRDN_FIELD_WIDTH,
    RESCAL_CFG_PWRDN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RESCAL_CFG_DIAG_ON
 ******************************************************************************/
const ru_field_rec RESCAL_CFG_DIAG_ON_FIELD =
{
    "DIAG_ON",
#if RU_INCLUDE_DESC
    "DIAG_ON",
    "Connects to i_diag_on."
    "Reset value is 0x0.",
#endif
    RESCAL_CFG_DIAG_ON_FIELD_MASK,
    0,
    RESCAL_CFG_DIAG_ON_FIELD_WIDTH,
    RESCAL_CFG_DIAG_ON_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RESCAL_CFG_RSTB
 ******************************************************************************/
const ru_field_rec RESCAL_CFG_RSTB_FIELD =
{
    "RSTB",
#if RU_INCLUDE_DESC
    "RSTB",
    "Connects to i_rstb."
    "Reset value is 0x0."
    "",
#endif
    RESCAL_CFG_RSTB_FIELD_MASK,
    0,
    RESCAL_CFG_RSTB_FIELD_WIDTH,
    RESCAL_CFG_RSTB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RESCAL_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec RESCAL_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RESCAL_CFG_RESERVED0_FIELD_MASK,
    0,
    RESCAL_CFG_RESERVED0_FIELD_WIDTH,
    RESCAL_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RESCAL_STATUS0_VALID
 ******************************************************************************/
const ru_field_rec RESCAL_STATUS0_VALID_FIELD =
{
    "VALID",
#if RU_INCLUDE_DESC
    "VALID",
    "Connects to o_valid."
    "Reset value is 0x0.",
#endif
    RESCAL_STATUS0_VALID_FIELD_MASK,
    0,
    RESCAL_STATUS0_VALID_FIELD_WIDTH,
    RESCAL_STATUS0_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RESCAL_STATUS0_COMP
 ******************************************************************************/
const ru_field_rec RESCAL_STATUS0_COMP_FIELD =
{
    "COMP",
#if RU_INCLUDE_DESC
    "COMP",
    "Connects to o_rescalcomp."
    "Reset value is 0x0.",
#endif
    RESCAL_STATUS0_COMP_FIELD_MASK,
    0,
    RESCAL_STATUS0_COMP_FIELD_WIDTH,
    RESCAL_STATUS0_COMP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RESCAL_STATUS0_STATE
 ******************************************************************************/
const ru_field_rec RESCAL_STATUS0_STATE_FIELD =
{
    "STATE",
#if RU_INCLUDE_DESC
    "STATE",
    "Connects to o_rescal_state."
    "Reset value is 0x0.",
#endif
    RESCAL_STATUS0_STATE_FIELD_MASK,
    0,
    RESCAL_STATUS0_STATE_FIELD_WIDTH,
    RESCAL_STATUS0_STATE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RESCAL_STATUS0_CTRL_DFS
 ******************************************************************************/
const ru_field_rec RESCAL_STATUS0_CTRL_DFS_FIELD =
{
    "CTRL_DFS",
#if RU_INCLUDE_DESC
    "CTRL_DFS",
    "Connects to o_rescal_ctrl_dfs."
    "Reset value is 0x0.",
#endif
    RESCAL_STATUS0_CTRL_DFS_FIELD_MASK,
    0,
    RESCAL_STATUS0_CTRL_DFS_FIELD_WIDTH,
    RESCAL_STATUS0_CTRL_DFS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RESCAL_STATUS0_PREV_COMP_CNT
 ******************************************************************************/
const ru_field_rec RESCAL_STATUS0_PREV_COMP_CNT_FIELD =
{
    "PREV_COMP_CNT",
#if RU_INCLUDE_DESC
    "PREV_COMP_CNT",
    "Connects to o_prev_comp_cnt."
    "Reset value is 0x0.",
#endif
    RESCAL_STATUS0_PREV_COMP_CNT_FIELD_MASK,
    0,
    RESCAL_STATUS0_PREV_COMP_CNT_FIELD_WIDTH,
    RESCAL_STATUS0_PREV_COMP_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RESCAL_STATUS0_PON
 ******************************************************************************/
const ru_field_rec RESCAL_STATUS0_PON_FIELD =
{
    "PON",
#if RU_INCLUDE_DESC
    "PON",
    "Connects to o_pon."
    "Reset value is 0x0.",
#endif
    RESCAL_STATUS0_PON_FIELD_MASK,
    0,
    RESCAL_STATUS0_PON_FIELD_WIDTH,
    RESCAL_STATUS0_PON_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RESCAL_STATUS0_DONE
 ******************************************************************************/
const ru_field_rec RESCAL_STATUS0_DONE_FIELD =
{
    "DONE",
#if RU_INCLUDE_DESC
    "DONE",
    "Connects to o_done."
    "Reset value is 0x0.",
#endif
    RESCAL_STATUS0_DONE_FIELD_MASK,
    0,
    RESCAL_STATUS0_DONE_FIELD_WIDTH,
    RESCAL_STATUS0_DONE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RESCAL_STATUS0_RESERVED0
 ******************************************************************************/
const ru_field_rec RESCAL_STATUS0_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RESCAL_STATUS0_RESERVED0_FIELD_MASK,
    0,
    RESCAL_STATUS0_RESERVED0_FIELD_WIDTH,
    RESCAL_STATUS0_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RESCAL_STATUS1_CURR_COMP_CNT
 ******************************************************************************/
const ru_field_rec RESCAL_STATUS1_CURR_COMP_CNT_FIELD =
{
    "CURR_COMP_CNT",
#if RU_INCLUDE_DESC
    "CURR_COMP_CNT",
    "Connects to o_curr_comp_cnt."
    "Reset value is 0x0."
    "",
#endif
    RESCAL_STATUS1_CURR_COMP_CNT_FIELD_MASK,
    0,
    RESCAL_STATUS1_CURR_COMP_CNT_FIELD_WIDTH,
    RESCAL_STATUS1_CURR_COMP_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RESCAL_STATUS1_RESERVED0
 ******************************************************************************/
const ru_field_rec RESCAL_STATUS1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RESCAL_STATUS1_RESERVED0_FIELD_MASK,
    0,
    RESCAL_STATUS1_RESERVED0_FIELD_WIDTH,
    RESCAL_STATUS1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: RESCAL_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RESCAL_CFG_FIELDS[] =
{
    &RESCAL_CFG_CTRL_FIELD,
    &RESCAL_CFG_PWRDN_FIELD,
    &RESCAL_CFG_DIAG_ON_FIELD,
    &RESCAL_CFG_RSTB_FIELD,
    &RESCAL_CFG_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RESCAL_CFG_REG = 
{
    "CFG",
#if RU_INCLUDE_DESC
    "RESCL_CFG Register",
    "RESCAL configuration",
#endif
    RESCAL_CFG_REG_OFFSET,
    0,
    0,
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    RESCAL_CFG_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: RESCAL_STATUS0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RESCAL_STATUS0_FIELDS[] =
{
    &RESCAL_STATUS0_VALID_FIELD,
    &RESCAL_STATUS0_COMP_FIELD,
    &RESCAL_STATUS0_STATE_FIELD,
    &RESCAL_STATUS0_CTRL_DFS_FIELD,
    &RESCAL_STATUS0_PREV_COMP_CNT_FIELD,
    &RESCAL_STATUS0_PON_FIELD,
    &RESCAL_STATUS0_DONE_FIELD,
    &RESCAL_STATUS0_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RESCAL_STATUS0_REG = 
{
    "STATUS0",
#if RU_INCLUDE_DESC
    "RESCL_STATUS0 Register",
    "Register used for reading RESCAL status.",
#endif
    RESCAL_STATUS0_REG_OFFSET,
    0,
    0,
    3,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    RESCAL_STATUS0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: RESCAL_STATUS1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RESCAL_STATUS1_FIELDS[] =
{
    &RESCAL_STATUS1_CURR_COMP_CNT_FIELD,
    &RESCAL_STATUS1_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RESCAL_STATUS1_REG = 
{
    "STATUS1",
#if RU_INCLUDE_DESC
    "RESCL_STATUS1 Register",
    "Register used for reading RESCAL status.",
#endif
    RESCAL_STATUS1_REG_OFFSET,
    0,
    0,
    4,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RESCAL_STATUS1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: RESCAL
 ******************************************************************************/
static const ru_reg_rec *RESCAL_REGS[] =
{
    &RESCAL_CFG_REG,
    &RESCAL_STATUS0_REG,
    &RESCAL_STATUS1_REG,
};

unsigned long RESCAL_ADDRS[] =
{
    0x82db2034,
};

const ru_block_rec RESCAL_BLOCK = 
{
    "RESCAL",
    RESCAL_ADDRS,
    1,
    3,
    RESCAL_REGS
};

/* End of file RESCAL.c */
