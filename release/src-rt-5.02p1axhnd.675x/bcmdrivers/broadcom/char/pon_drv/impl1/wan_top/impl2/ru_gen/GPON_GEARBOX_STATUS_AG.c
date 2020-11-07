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
    "Active high 1 cycle clear signal, use the common clear signal for"
    "the lock_lost_lh and the error counter as counter MSB will be in the"
    "same address as LOCK_LOST_LH.",
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
    "0: Do nothing.  1: Invert all data bits.",
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
    "Specifies the number of consecutive valid clock cycles with 1 or"
    "more bit errors before PRBS checker goes out of PRBS lock state.  0"
    "indicates that PRBS will go out of lock as soon as it gets the first"
    "clock with any error.  Likewise 31 indicates that PRBS will go out"
    "of lock as soon as it gets the 32 consecutive clocks with 1 or more.",
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
    "Specifies the number of consecutive valid clock cycles without any"
    "error before PRBS checker goes into PRBS lock state. Valid values"
    "are 0 to 31 where 0 indicate that PRBS will lock as soon as it gets"
    "the first clock with no error. Likewise 31 indicates that PRBS will"
    "lock as soon as it gets the 32 consecutive clocks with no error.",
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
    "0: will count in bit mode.  1: will count in burst mode.",
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
    "3'd0: PRBS7"
    "3'd1: PRBS9"
    "3'd2: PRBS11"
    "3'd3: PRBS15"
    "3'd4: PRBS23"
    "3'd5: PRBS31"
    "3'd6: PRBS58 (1 + x^39 + x^58)"
    "3'd7: Reserved",
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
    "prbs_chk_en timeout value, range 0 to 31 which maps to 0 to 448.",
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
    "2'b0X: disable prbs_chk_en timer.  2'b10: user heartbeat_toggle_1us."
    "2'b11: use heartbeat_toggle_1ms.  Actually, both heartbeat modes"
    "are the same, as the count of cycles is determined by counter value"
    "field in chk_ctrl_1.",
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
    "1 will enable the PRBS checker.  Also will be used as clock gator"
    "control.",
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
    "2'd0 - self-sync mode w/ hysteresis, polynomial is continuously"
    "seeded with previous received bits"
    "2'd1 - initial seed mode w/ hysteresis, polynomial is seeded with"
    "previous received bits only till lock is acquired. Once locked LFSR"
    "runs independently from the received data"
    "2'd2 - initial seed mode w/o hysteresis, similar to mode 2 above"
    "except once locked it stays locked"
    "2'd3 - reserved",
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
    "Number of cycles (155 clk) for increment of timer count.  0x9B when"
    "timer set to count us.  0x25F80 when timer set to count ms.",
#endif
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_1_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_1_TIMER_VAL_FIELD_MASK,
    0,
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_1_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_1_TIMER_VAL_FIELD_WIDTH,
    GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_1_CR_XGWAN_TOP_WAN_MISC_GPON_GEARBOX_RG_PRBS_CHK_CTRL_1_TIMER_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
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
    32,
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
    33,
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
    34,
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
 * Block: GPON_GEARBOX_STATUS
 ******************************************************************************/
static const ru_reg_rec *GPON_GEARBOX_STATUS_REGS[] =
{
    &GPON_GEARBOX_STATUS_GEARBOX_STATUS_REG,
    &GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_0_REG,
    &GPON_GEARBOX_STATUS_GEARBOX_PRBS_CONTROL_1_REG,
};

unsigned long GPON_GEARBOX_STATUS_ADDRS[] =
{
    0x80144098,
};

const ru_block_rec GPON_GEARBOX_STATUS_BLOCK = 
{
    "GPON_GEARBOX_STATUS",
    GPON_GEARBOX_STATUS_ADDRS,
    1,
    3,
    GPON_GEARBOX_STATUS_REGS
};

/* End of file GPON_GEARBOX_STATUS.c */
