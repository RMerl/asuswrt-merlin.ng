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
 * Field: GPON_GEARBOX_STATUS_GEARBOX_STATUS_CR_RD_DATA_CLX
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_STATUS_GEARBOX_STATUS_CR_RD_DATA_CLX_FIELD =
{
    "CR_RD_DATA_CLX",
#if RU_INCLUDE_DESC
    "",
    "Status indication based on status_sel signals.  If"
    "gpon_gearbox_fifo_status_sel is high, this status will be"
    "gpon_gearbox_fifo_status.  If gpon_gearbox_ptg_status1_sel is high,"
    "this status will be gpon_gearbox_ptg_status1.  If"
    "gpon_gearbox_ptg_status2_sel is high, this status will be"
    "gpon_gearbox_ptg_status2.",
#endif
    GPON_GEARBOX_STATUS_GEARBOX_STATUS_CR_RD_DATA_CLX_FIELD_MASK,
    0,
    GPON_GEARBOX_STATUS_GEARBOX_STATUS_CR_RD_DATA_CLX_FIELD_WIDTH,
    GPON_GEARBOX_STATUS_GEARBOX_STATUS_CR_RD_DATA_CLX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_RESERVED0
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_RESERVED0_FIELD_MASK,
    0,
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_RESERVED0_FIELD_WIDTH,
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_SIG_PRBS_STATUS_CLR
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_SIG_PRBS_STATUS_CLR_FIELD =
{
    "CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_SIG_PRBS_STATUS_CLR",
#if RU_INCLUDE_DESC
    "",
    "TBD",
#endif
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_SIG_PRBS_STATUS_CLR_FIELD_MASK,
    0,
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_SIG_PRBS_STATUS_CLR_FIELD_WIDTH,
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_SIG_PRBS_STATUS_CLR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_INV
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_INV_FIELD =
{
    "CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_INV",
#if RU_INCLUDE_DESC
    "",
    "TBD",
#endif
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_INV_FIELD_MASK,
    0,
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_INV_FIELD_WIDTH,
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_INV_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_OOL_CNT
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_OOL_CNT_FIELD =
{
    "CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_OOL_CNT",
#if RU_INCLUDE_DESC
    "",
    "TBD",
#endif
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_OOL_CNT_FIELD_MASK,
    0,
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_OOL_CNT_FIELD_WIDTH,
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_OOL_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_LOCK_CNT
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_LOCK_CNT_FIELD =
{
    "CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_LOCK_CNT",
#if RU_INCLUDE_DESC
    "",
    "TBD",
#endif
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_LOCK_CNT_FIELD_MASK,
    0,
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_LOCK_CNT_FIELD_WIDTH,
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_LOCK_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_ERR_CNT_BURST_MODE
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_ERR_CNT_BURST_MODE_FIELD =
{
    "CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_ERR_CNT_BURST_MODE",
#if RU_INCLUDE_DESC
    "",
    "TBD",
#endif
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_ERR_CNT_BURST_MODE_FIELD_MASK,
    0,
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_ERR_CNT_BURST_MODE_FIELD_WIDTH,
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_ERR_CNT_BURST_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_MODE_SEL
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_MODE_SEL_FIELD =
{
    "CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_MODE_SEL",
#if RU_INCLUDE_DESC
    "",
    "TBD",
#endif
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_MODE_SEL_FIELD_MASK,
    0,
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_MODE_SEL_FIELD_WIDTH,
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_MODE_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_RESERVED1
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_RESERVED1_FIELD_MASK,
    0,
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_RESERVED1_FIELD_WIDTH,
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_EN_TIMEOUT
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_EN_TIMEOUT_FIELD =
{
    "CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_EN_TIMEOUT",
#if RU_INCLUDE_DESC
    "",
    "TBD",
#endif
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_EN_TIMEOUT_FIELD_MASK,
    0,
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_EN_TIMEOUT_FIELD_WIDTH,
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_EN_TIMEOUT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_EN_TIMER_MODE
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_EN_TIMER_MODE_FIELD =
{
    "CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_EN_TIMER_MODE",
#if RU_INCLUDE_DESC
    "",
    "TBD",
#endif
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_EN_TIMER_MODE_FIELD_MASK,
    0,
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_EN_TIMER_MODE_FIELD_WIDTH,
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_EN_TIMER_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_1_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_1_EN
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_1_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_1_EN_FIELD =
{
    "CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_1_EN",
#if RU_INCLUDE_DESC
    "",
    "TBD",
#endif
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_1_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_1_EN_FIELD_MASK,
    0,
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_1_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_1_EN_FIELD_WIDTH,
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_1_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_1_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_1_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_1_MODE
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_1_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_1_MODE_FIELD =
{
    "CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_1_MODE",
#if RU_INCLUDE_DESC
    "",
    "TBD",
#endif
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_1_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_1_MODE_FIELD_MASK,
    0,
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_1_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_1_MODE_FIELD_WIDTH,
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_1_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_1_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_1_RESERVED0
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_1_RESERVED0_FIELD_MASK,
    0,
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_1_RESERVED0_FIELD_WIDTH,
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_1_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_1_TIMER_VAL
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_1_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_1_TIMER_VAL_FIELD =
{
    "CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_1_TIMER_VAL",
#if RU_INCLUDE_DESC
    "",
    "TBD",
#endif
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_1_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_1_TIMER_VAL_FIELD_MASK,
    0,
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_1_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_1_TIMER_VAL_FIELD_WIDTH,
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_1_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_1_TIMER_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_STATUS_GEARBOX_PRBS_STATUS_0_GPON_GEARBOX_PRBS_STAT_0_VECTOR
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_STATUS_GEARBOX_PRBS_STATUS_0_GPON_GEARBOX_PRBS_STAT_0_VECTOR_FIELD =
{
    "GPON_GEARBOX_PRBS_STAT_0_VECTOR",
#if RU_INCLUDE_DESC
    "",
    "TBD",
#endif
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_STATUS_0_GPON_GEARBOX_PRBS_STAT_0_VECTOR_FIELD_MASK,
    0,
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_STATUS_0_GPON_GEARBOX_PRBS_STAT_0_VECTOR_FIELD_WIDTH,
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_STATUS_0_GPON_GEARBOX_PRBS_STAT_0_VECTOR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_STATUS_GEARBOX_PRBS_STATUS_1_RESERVED0
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_STATUS_GEARBOX_PRBS_STATUS_1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_STATUS_1_RESERVED0_FIELD_MASK,
    0,
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_STATUS_1_RESERVED0_FIELD_WIDTH,
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_STATUS_1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_STATUS_GEARBOX_PRBS_STATUS_1_GPON_GEARBOX_PRBS_STAT_1_VECTOR
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_STATUS_GEARBOX_PRBS_STATUS_1_GPON_GEARBOX_PRBS_STAT_1_VECTOR_FIELD =
{
    "GPON_GEARBOX_PRBS_STAT_1_VECTOR",
#if RU_INCLUDE_DESC
    "",
    "TBD",
#endif
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_STATUS_1_GPON_GEARBOX_PRBS_STAT_1_VECTOR_FIELD_MASK,
    0,
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_STATUS_1_GPON_GEARBOX_PRBS_STAT_1_VECTOR_FIELD_WIDTH,
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_STATUS_1_GPON_GEARBOX_PRBS_STAT_1_VECTOR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: GPON_GEARBOX_STATUS_GEARBOX_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *GPON_GEARBOX_STATUS_GEARBOX_STATUS_FIELDS[] =
{
    &GPON_GEARBOX_STATUS_GEARBOX_STATUS_CR_RD_DATA_CLX_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec GPON_GEARBOX_STATUS_GEARBOX_STATUS_REG = 
{
    "GEARBOX_STATUS",
#if RU_INCLUDE_DESC
    "WAN_TOP_GPON_GEARBOX_STATUS Register",
    "Register used for various WAN status bits.",
#endif
    GPON_GEARBOX_STATUS_GEARBOX_STATUS_REG_OFFSET,
    0,
    0,
    44,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    GPON_GEARBOX_STATUS_GEARBOX_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_FIELDS[] =
{
    &GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_RESERVED0_FIELD,
    &GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_SIG_PRBS_STATUS_CLR_FIELD,
    &GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_INV_FIELD,
    &GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_OOL_CNT_FIELD,
    &GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_LOCK_CNT_FIELD,
    &GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_ERR_CNT_BURST_MODE_FIELD,
    &GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_MODE_SEL_FIELD,
    &GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_RESERVED1_FIELD,
    &GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_EN_TIMEOUT_FIELD,
    &GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_0_EN_TIMER_MODE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_REG = 
{
    "GEARBOX_PRBS_CONTROL_0",
#if RU_INCLUDE_DESC
    "WAN_TOP_GPON_GEARBOX_PRBS_CONTROL_0 Register",
    "Register used to control the GPON gearbox PRBS checker.",
#endif
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_REG_OFFSET,
    0,
    0,
    45,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    10,
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_1_FIELDS[] =
{
    &GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_1_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_1_EN_FIELD,
    &GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_1_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_1_MODE_FIELD,
    &GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_1_RESERVED0_FIELD,
    &GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_1_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_1_TIMER_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_1_REG = 
{
    "GEARBOX_PRBS_CONTROL_1",
#if RU_INCLUDE_DESC
    "WAN_TOP_GPON_GEARBOX_PRBS_CONTROL_1 Register",
    "Register used to control the GPON gearbox PRBS checker.",
#endif
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_1_REG_OFFSET,
    0,
    0,
    46,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: GPON_GEARBOX_STATUS_GEARBOX_PRBS_STATUS_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *GPON_GEARBOX_STATUS_GEARBOX_PRBS_STATUS_0_FIELDS[] =
{
    &GPON_GEARBOX_STATUS_GEARBOX_PRBS_STATUS_0_GPON_GEARBOX_PRBS_STAT_0_VECTOR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec GPON_GEARBOX_STATUS_GEARBOX_PRBS_STATUS_0_REG = 
{
    "GEARBOX_PRBS_STATUS_0",
#if RU_INCLUDE_DESC
    "WAN_TOP_GPON_GEARBOX_PRBS_STATUS_0 Register",
    "Register used to monitor the GPON gearbox PRBS checker.",
#endif
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_STATUS_0_REG_OFFSET,
    0,
    0,
    47,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_STATUS_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: GPON_GEARBOX_STATUS_GEARBOX_PRBS_STATUS_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *GPON_GEARBOX_STATUS_GEARBOX_PRBS_STATUS_1_FIELDS[] =
{
    &GPON_GEARBOX_STATUS_GEARBOX_PRBS_STATUS_1_RESERVED0_FIELD,
    &GPON_GEARBOX_STATUS_GEARBOX_PRBS_STATUS_1_GPON_GEARBOX_PRBS_STAT_1_VECTOR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec GPON_GEARBOX_STATUS_GEARBOX_PRBS_STATUS_1_REG = 
{
    "GEARBOX_PRBS_STATUS_1",
#if RU_INCLUDE_DESC
    "WAN_TOP_GPON_GEARBOX_PRBS_STATUS_1 Register",
    "Register used to monitor the GPON gearbox PRBS checker.",
#endif
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_STATUS_1_REG_OFFSET,
    0,
    0,
    48,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_STATUS_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: GPON_GEARBOX_STATUS
 ******************************************************************************/
static const ru_reg_rec *GPON_GEARBOX_STATUS_REGS[] =
{
    &GPON_GEARBOX_STATUS_GEARBOX_STATUS_REG,
    &GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_REG,
    &GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_1_REG,
    &GPON_GEARBOX_STATUS_GEARBOX_PRBS_STATUS_0_REG,
    &GPON_GEARBOX_STATUS_GEARBOX_PRBS_STATUS_1_REG,
};

unsigned long GPON_GEARBOX_STATUS_ADDRS[] =
{
    0x80144094,
};

const ru_block_rec GPON_GEARBOX_STATUS_BLOCK = 
{
    "GPON_GEARBOX_STATUS",
    GPON_GEARBOX_STATUS_ADDRS,
    1,
    5,
    GPON_GEARBOX_STATUS_REGS
};

/* End of file GPON_GEARBOX_STATUS.c */
