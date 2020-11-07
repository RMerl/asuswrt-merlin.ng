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
 * Field: RESCAL_CFG_CFG_WAN_RESCAL_RSTB
 ******************************************************************************/
const ru_field_rec RESCAL_CFG_CFG_WAN_RESCAL_RSTB_FIELD =
{
    "CFG_WAN_RESCAL_RSTB",
#if RU_INCLUDE_DESC
    "",
    "Connects to i_rstb.",
#endif
    RESCAL_CFG_CFG_WAN_RESCAL_RSTB_FIELD_MASK,
    0,
    RESCAL_CFG_CFG_WAN_RESCAL_RSTB_FIELD_WIDTH,
    RESCAL_CFG_CFG_WAN_RESCAL_RSTB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RESCAL_CFG_CFG_WAN_RESCAL_DIAG_ON
 ******************************************************************************/
const ru_field_rec RESCAL_CFG_CFG_WAN_RESCAL_DIAG_ON_FIELD =
{
    "CFG_WAN_RESCAL_DIAG_ON",
#if RU_INCLUDE_DESC
    "",
    "Connects to i_diag_on.",
#endif
    RESCAL_CFG_CFG_WAN_RESCAL_DIAG_ON_FIELD_MASK,
    0,
    RESCAL_CFG_CFG_WAN_RESCAL_DIAG_ON_FIELD_WIDTH,
    RESCAL_CFG_CFG_WAN_RESCAL_DIAG_ON_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RESCAL_CFG_CFG_WAN_RESCAL_PWRDN
 ******************************************************************************/
const ru_field_rec RESCAL_CFG_CFG_WAN_RESCAL_PWRDN_FIELD =
{
    "CFG_WAN_RESCAL_PWRDN",
#if RU_INCLUDE_DESC
    "",
    "Connects to i_pwrdn.",
#endif
    RESCAL_CFG_CFG_WAN_RESCAL_PWRDN_FIELD_MASK,
    0,
    RESCAL_CFG_CFG_WAN_RESCAL_PWRDN_FIELD_WIDTH,
    RESCAL_CFG_CFG_WAN_RESCAL_PWRDN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RESCAL_CFG_CFG_WAN_RESCAL_CTRL
 ******************************************************************************/
const ru_field_rec RESCAL_CFG_CFG_WAN_RESCAL_CTRL_FIELD =
{
    "CFG_WAN_RESCAL_CTRL",
#if RU_INCLUDE_DESC
    "",
    "Connects to i_rescal_ctrl.",
#endif
    RESCAL_CFG_CFG_WAN_RESCAL_CTRL_FIELD_MASK,
    0,
    RESCAL_CFG_CFG_WAN_RESCAL_CTRL_FIELD_WIDTH,
    RESCAL_CFG_CFG_WAN_RESCAL_CTRL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: RESCAL_STATUS_0_RESERVED0
 ******************************************************************************/
const ru_field_rec RESCAL_STATUS_0_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RESCAL_STATUS_0_RESERVED0_FIELD_MASK,
    0,
    RESCAL_STATUS_0_RESERVED0_FIELD_WIDTH,
    RESCAL_STATUS_0_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RESCAL_STATUS_0_WAN_RESCAL_DONE
 ******************************************************************************/
const ru_field_rec RESCAL_STATUS_0_WAN_RESCAL_DONE_FIELD =
{
    "WAN_RESCAL_DONE",
#if RU_INCLUDE_DESC
    "",
    "Connects to o_done.",
#endif
    RESCAL_STATUS_0_WAN_RESCAL_DONE_FIELD_MASK,
    0,
    RESCAL_STATUS_0_WAN_RESCAL_DONE_FIELD_WIDTH,
    RESCAL_STATUS_0_WAN_RESCAL_DONE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RESCAL_STATUS_0_WAN_RESCAL_PON
 ******************************************************************************/
const ru_field_rec RESCAL_STATUS_0_WAN_RESCAL_PON_FIELD =
{
    "WAN_RESCAL_PON",
#if RU_INCLUDE_DESC
    "",
    "Connects to o_pon.",
#endif
    RESCAL_STATUS_0_WAN_RESCAL_PON_FIELD_MASK,
    0,
    RESCAL_STATUS_0_WAN_RESCAL_PON_FIELD_WIDTH,
    RESCAL_STATUS_0_WAN_RESCAL_PON_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RESCAL_STATUS_0_WAN_RESCAL_PREV_COMP_CNT
 ******************************************************************************/
const ru_field_rec RESCAL_STATUS_0_WAN_RESCAL_PREV_COMP_CNT_FIELD =
{
    "WAN_RESCAL_PREV_COMP_CNT",
#if RU_INCLUDE_DESC
    "",
    "Connects to o_prev_comp_cnt.",
#endif
    RESCAL_STATUS_0_WAN_RESCAL_PREV_COMP_CNT_FIELD_MASK,
    0,
    RESCAL_STATUS_0_WAN_RESCAL_PREV_COMP_CNT_FIELD_WIDTH,
    RESCAL_STATUS_0_WAN_RESCAL_PREV_COMP_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RESCAL_STATUS_0_WAN_RESCAL_CTRL_DFS
 ******************************************************************************/
const ru_field_rec RESCAL_STATUS_0_WAN_RESCAL_CTRL_DFS_FIELD =
{
    "WAN_RESCAL_CTRL_DFS",
#if RU_INCLUDE_DESC
    "",
    "Connects to o_rescal_ctrl_dfs.",
#endif
    RESCAL_STATUS_0_WAN_RESCAL_CTRL_DFS_FIELD_MASK,
    0,
    RESCAL_STATUS_0_WAN_RESCAL_CTRL_DFS_FIELD_WIDTH,
    RESCAL_STATUS_0_WAN_RESCAL_CTRL_DFS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RESCAL_STATUS_0_WAN_RESCAL_STATE
 ******************************************************************************/
const ru_field_rec RESCAL_STATUS_0_WAN_RESCAL_STATE_FIELD =
{
    "WAN_RESCAL_STATE",
#if RU_INCLUDE_DESC
    "",
    "Connects to o_rescal_state.",
#endif
    RESCAL_STATUS_0_WAN_RESCAL_STATE_FIELD_MASK,
    0,
    RESCAL_STATUS_0_WAN_RESCAL_STATE_FIELD_WIDTH,
    RESCAL_STATUS_0_WAN_RESCAL_STATE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RESCAL_STATUS_0_WAN_RESCAL_COMP
 ******************************************************************************/
const ru_field_rec RESCAL_STATUS_0_WAN_RESCAL_COMP_FIELD =
{
    "WAN_RESCAL_COMP",
#if RU_INCLUDE_DESC
    "",
    "Connects to o_rescalcomp.",
#endif
    RESCAL_STATUS_0_WAN_RESCAL_COMP_FIELD_MASK,
    0,
    RESCAL_STATUS_0_WAN_RESCAL_COMP_FIELD_WIDTH,
    RESCAL_STATUS_0_WAN_RESCAL_COMP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RESCAL_STATUS_0_WAN_RESCAL_VALID
 ******************************************************************************/
const ru_field_rec RESCAL_STATUS_0_WAN_RESCAL_VALID_FIELD =
{
    "WAN_RESCAL_VALID",
#if RU_INCLUDE_DESC
    "",
    "Connects to o_valid.",
#endif
    RESCAL_STATUS_0_WAN_RESCAL_VALID_FIELD_MASK,
    0,
    RESCAL_STATUS_0_WAN_RESCAL_VALID_FIELD_WIDTH,
    RESCAL_STATUS_0_WAN_RESCAL_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RESCAL_STATUS_1_RESERVED0
 ******************************************************************************/
const ru_field_rec RESCAL_STATUS_1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    RESCAL_STATUS_1_RESERVED0_FIELD_MASK,
    0,
    RESCAL_STATUS_1_RESERVED0_FIELD_WIDTH,
    RESCAL_STATUS_1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: RESCAL_STATUS_1_WAN_RESCAL_CURR_COMP_CNT
 ******************************************************************************/
const ru_field_rec RESCAL_STATUS_1_WAN_RESCAL_CURR_COMP_CNT_FIELD =
{
    "WAN_RESCAL_CURR_COMP_CNT",
#if RU_INCLUDE_DESC
    "",
    "Connects to o_curr_comp_cnt.",
#endif
    RESCAL_STATUS_1_WAN_RESCAL_CURR_COMP_CNT_FIELD_MASK,
    0,
    RESCAL_STATUS_1_WAN_RESCAL_CURR_COMP_CNT_FIELD_WIDTH,
    RESCAL_STATUS_1_WAN_RESCAL_CURR_COMP_CNT_FIELD_SHIFT,
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
    &RESCAL_CFG_RESERVED0_FIELD,
    &RESCAL_CFG_CFG_WAN_RESCAL_RSTB_FIELD,
    &RESCAL_CFG_CFG_WAN_RESCAL_DIAG_ON_FIELD,
    &RESCAL_CFG_CFG_WAN_RESCAL_PWRDN_FIELD,
    &RESCAL_CFG_CFG_WAN_RESCAL_CTRL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RESCAL_CFG_REG = 
{
    "CFG",
#if RU_INCLUDE_DESC
    "WAN_TOP_RESCAL_CFG Register",
    "Register used for configuring the RESCAL.",
#endif
    RESCAL_CFG_REG_OFFSET,
    0,
    0,
    6,
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
 * Register: RESCAL_STATUS_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RESCAL_STATUS_0_FIELDS[] =
{
    &RESCAL_STATUS_0_RESERVED0_FIELD,
    &RESCAL_STATUS_0_WAN_RESCAL_DONE_FIELD,
    &RESCAL_STATUS_0_WAN_RESCAL_PON_FIELD,
    &RESCAL_STATUS_0_WAN_RESCAL_PREV_COMP_CNT_FIELD,
    &RESCAL_STATUS_0_WAN_RESCAL_CTRL_DFS_FIELD,
    &RESCAL_STATUS_0_WAN_RESCAL_STATE_FIELD,
    &RESCAL_STATUS_0_WAN_RESCAL_COMP_FIELD,
    &RESCAL_STATUS_0_WAN_RESCAL_VALID_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RESCAL_STATUS_0_REG = 
{
    "STATUS_0",
#if RU_INCLUDE_DESC
    "WAN_TOP_RESCAL_STATUS_0 Register",
    "Register used for reading RESCAL status.",
#endif
    RESCAL_STATUS_0_REG_OFFSET,
    0,
    0,
    7,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    RESCAL_STATUS_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: RESCAL_STATUS_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *RESCAL_STATUS_1_FIELDS[] =
{
    &RESCAL_STATUS_1_RESERVED0_FIELD,
    &RESCAL_STATUS_1_WAN_RESCAL_CURR_COMP_CNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec RESCAL_STATUS_1_REG = 
{
    "STATUS_1",
#if RU_INCLUDE_DESC
    "WAN_TOP_RESCAL_STATUS_1 Register",
    "Register used for reading RESCAL status.",
#endif
    RESCAL_STATUS_1_REG_OFFSET,
    0,
    0,
    8,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    RESCAL_STATUS_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: RESCAL
 ******************************************************************************/
static const ru_reg_rec *RESCAL_REGS[] =
{
    &RESCAL_CFG_REG,
    &RESCAL_STATUS_0_REG,
    &RESCAL_STATUS_1_REG,
};

unsigned long RESCAL_ADDRS[] =
{
#if defined(CONFIG_BCM963158)
    0x8014401c,
#else
    #error "RESCAL base address not defined"
#endif
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
