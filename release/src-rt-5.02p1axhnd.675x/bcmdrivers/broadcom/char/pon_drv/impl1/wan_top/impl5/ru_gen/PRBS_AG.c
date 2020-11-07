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
 * Field: PRBS_CHK_CTRL_0_EN_TIMER_MODE
 ******************************************************************************/
const ru_field_rec PRBS_CHK_CTRL_0_EN_TIMER_MODE_FIELD =
{
    "EN_TIMER_MODE",
#if RU_INCLUDE_DESC
    "EN_TIMER_MODE",
    "0x - disable prbs_chk_en timer                                                    10 - use heartbeat_toggle_1us                                                       11 - use heartbeat_toggle_1ms"
    ""
    "Actually both heartbeat modes are the same, as the count of cycles is determined by counter value field in chk_ctrl_1 register.",
#endif
    PRBS_CHK_CTRL_0_EN_TIMER_MODE_FIELD_MASK,
    0,
    PRBS_CHK_CTRL_0_EN_TIMER_MODE_FIELD_WIDTH,
    PRBS_CHK_CTRL_0_EN_TIMER_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PRBS_CHK_CTRL_0_EN_TIMEOUT
 ******************************************************************************/
const ru_field_rec PRBS_CHK_CTRL_0_EN_TIMEOUT_FIELD =
{
    "EN_TIMEOUT",
#if RU_INCLUDE_DESC
    "EN_TIMEOUT",
    "prbs_chk_en timeout value, range 0 to 31 which maps to 0 to 448",
#endif
    PRBS_CHK_CTRL_0_EN_TIMEOUT_FIELD_MASK,
    0,
    PRBS_CHK_CTRL_0_EN_TIMEOUT_FIELD_WIDTH,
    PRBS_CHK_CTRL_0_EN_TIMEOUT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PRBS_CHK_CTRL_0_RESERVED0
 ******************************************************************************/
const ru_field_rec PRBS_CHK_CTRL_0_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    PRBS_CHK_CTRL_0_RESERVED0_FIELD_MASK,
    0,
    PRBS_CHK_CTRL_0_RESERVED0_FIELD_WIDTH,
    PRBS_CHK_CTRL_0_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PRBS_CHK_CTRL_0_MODE_SEL
 ******************************************************************************/
const ru_field_rec PRBS_CHK_CTRL_0_MODE_SEL_FIELD =
{
    "MODE_SEL",
#if RU_INCLUDE_DESC
    "MODE_SEL",
    "3d0 -> PRBS 7"
    "3d1 -> PRBS 9                                                         3d2 -> PRBS 11                                                         3d3 -> PRBS 15                                                         3d4 -> PRBS 23                                                         3d5 -> PRBS 31                                                    3d6 -> PRBS 58 (1 + x^39 + x^58)                                                   3d7 -> reserved"
    ""
    "",
#endif
    PRBS_CHK_CTRL_0_MODE_SEL_FIELD_MASK,
    0,
    PRBS_CHK_CTRL_0_MODE_SEL_FIELD_WIDTH,
    PRBS_CHK_CTRL_0_MODE_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PRBS_CHK_CTRL_0_ERR_CNT_BURST_MODE
 ******************************************************************************/
const ru_field_rec PRBS_CHK_CTRL_0_ERR_CNT_BURST_MODE_FIELD =
{
    "ERR_CNT_BURST_MODE",
#if RU_INCLUDE_DESC
    "ERR_CNT_BURST_MODE",
    "0 will count in bit mode (default)"
    "1 will count in burst mode"
    "",
#endif
    PRBS_CHK_CTRL_0_ERR_CNT_BURST_MODE_FIELD_MASK,
    0,
    PRBS_CHK_CTRL_0_ERR_CNT_BURST_MODE_FIELD_WIDTH,
    PRBS_CHK_CTRL_0_ERR_CNT_BURST_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PRBS_CHK_CTRL_0_LOCK_CNT
 ******************************************************************************/
const ru_field_rec PRBS_CHK_CTRL_0_LOCK_CNT_FIELD =
{
    "LOCK_CNT",
#if RU_INCLUDE_DESC
    "LOCK_CNT",
    "Specifies the number of consecutive valid clock cycles without any error before PRBS checker goes into PRBS lock state. Valid values are 0 to 31 where 0 indicate that PRBS will lock as soon as it gets the first clock with no error. Likewise 31 indicates that PRBS will lock as soon as it gets the 32 consecutive clocks with no error."
    "",
#endif
    PRBS_CHK_CTRL_0_LOCK_CNT_FIELD_MASK,
    0,
    PRBS_CHK_CTRL_0_LOCK_CNT_FIELD_WIDTH,
    PRBS_CHK_CTRL_0_LOCK_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PRBS_CHK_CTRL_0_OOL_CNT
 ******************************************************************************/
const ru_field_rec PRBS_CHK_CTRL_0_OOL_CNT_FIELD =
{
    "OOL_CNT",
#if RU_INCLUDE_DESC
    "OOL_CNT",
    "Specifies the number of consecutive valid clock cycles with 1 or more bit errors before PRBS checker goes out of PRBS lock state."
    "0 indicate that PRBS will go out of lock as soon as it gets the first clock with any error."
    "Likewise 31 indicates that PRBS will go out of lock as soon as it gets the 32 consecutive clocks with 1 or more",
#endif
    PRBS_CHK_CTRL_0_OOL_CNT_FIELD_MASK,
    0,
    PRBS_CHK_CTRL_0_OOL_CNT_FIELD_WIDTH,
    PRBS_CHK_CTRL_0_OOL_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PRBS_CHK_CTRL_0_INV
 ******************************************************************************/
const ru_field_rec PRBS_CHK_CTRL_0_INV_FIELD =
{
    "INV",
#if RU_INCLUDE_DESC
    "INV",
    "1 will invert all the data bits."
    "",
#endif
    PRBS_CHK_CTRL_0_INV_FIELD_MASK,
    0,
    PRBS_CHK_CTRL_0_INV_FIELD_WIDTH,
    PRBS_CHK_CTRL_0_INV_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PRBS_CHK_CTRL_0_SIG_PRBS_STATUS_CLR
 ******************************************************************************/
const ru_field_rec PRBS_CHK_CTRL_0_SIG_PRBS_STATUS_CLR_FIELD =
{
    "SIG_PRBS_STATUS_CLR",
#if RU_INCLUDE_DESC
    "SIG_PRBS_STATUS_CLR",
    "Active high 1 cycle clear signal, use the common clear signal for the lock_lost_lh and the error counter as counter MSB will be in the same address as LOCK_LOST_LH"
    "",
#endif
    PRBS_CHK_CTRL_0_SIG_PRBS_STATUS_CLR_FIELD_MASK,
    0,
    PRBS_CHK_CTRL_0_SIG_PRBS_STATUS_CLR_FIELD_WIDTH,
    PRBS_CHK_CTRL_0_SIG_PRBS_STATUS_CLR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PRBS_CHK_CTRL_0_RESERVED1
 ******************************************************************************/
const ru_field_rec PRBS_CHK_CTRL_0_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    PRBS_CHK_CTRL_0_RESERVED1_FIELD_MASK,
    0,
    PRBS_CHK_CTRL_0_RESERVED1_FIELD_WIDTH,
    PRBS_CHK_CTRL_0_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PRBS_CHK_CTRL_1_PRBS_TIMER_VAL
 ******************************************************************************/
const ru_field_rec PRBS_CHK_CTRL_1_PRBS_TIMER_VAL_FIELD =
{
    "PRBS_TIMER_VAL",
#if RU_INCLUDE_DESC
    "PRBS_TIMER_VAL",
    "Number of cycles (155 clk) for increment of timer count."
    "0x9B when timer set to count us (default value)."
    "0x25f80 when timer set to count ms."
    "",
#endif
    PRBS_CHK_CTRL_1_PRBS_TIMER_VAL_FIELD_MASK,
    0,
    PRBS_CHK_CTRL_1_PRBS_TIMER_VAL_FIELD_WIDTH,
    PRBS_CHK_CTRL_1_PRBS_TIMER_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PRBS_CHK_CTRL_1_RESERVED0
 ******************************************************************************/
const ru_field_rec PRBS_CHK_CTRL_1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    PRBS_CHK_CTRL_1_RESERVED0_FIELD_MASK,
    0,
    PRBS_CHK_CTRL_1_RESERVED0_FIELD_WIDTH,
    PRBS_CHK_CTRL_1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PRBS_CHK_CTRL_1_PRBS_CHK_MODE
 ******************************************************************************/
const ru_field_rec PRBS_CHK_CTRL_1_PRBS_CHK_MODE_FIELD =
{
    "PRBS_CHK_MODE",
#if RU_INCLUDE_DESC
    "PRBS_CHK_MODE",
    "2d0  - self-sync mode w/ hysteresis, polynomial is continuously seeded with previous received bits"
    "2d1  - initial seed mode w/ hysteresis, polynomial is seeded with previous received bits only till lock is acquired. Once locked LFSR runs independently from the received data"
    "2d2  - initial seed mode w/o hysteresis, similar to mode 2 above except once locked it stays locked"
    "2d3  - reserved"
    "",
#endif
    PRBS_CHK_CTRL_1_PRBS_CHK_MODE_FIELD_MASK,
    0,
    PRBS_CHK_CTRL_1_PRBS_CHK_MODE_FIELD_WIDTH,
    PRBS_CHK_CTRL_1_PRBS_CHK_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PRBS_CHK_CTRL_1_PRBS_CHK_EN
 ******************************************************************************/
const ru_field_rec PRBS_CHK_CTRL_1_PRBS_CHK_EN_FIELD =
{
    "PRBS_CHK_EN",
#if RU_INCLUDE_DESC
    "PRBS_CHK_EN",
    "1 will enable the PRBS checker. Also will be used as clock gator control"
    "",
#endif
    PRBS_CHK_CTRL_1_PRBS_CHK_EN_FIELD_MASK,
    0,
    PRBS_CHK_CTRL_1_PRBS_CHK_EN_FIELD_WIDTH,
    PRBS_CHK_CTRL_1_PRBS_CHK_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: PRBS_STATUS_0_ERR_CNT
 ******************************************************************************/
const ru_field_rec PRBS_STATUS_0_ERR_CNT_FIELD =
{
    "ERR_CNT",
#if RU_INCLUDE_DESC
    "ERR_CNT",
    "Error counter. Toggle SIG_PRBS_STATUS_CLR to latch.",
#endif
    PRBS_STATUS_0_ERR_CNT_FIELD_MASK,
    0,
    PRBS_STATUS_0_ERR_CNT_FIELD_WIDTH,
    PRBS_STATUS_0_ERR_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: PRBS_STATUS_0_LOCK_LOST_LH
 ******************************************************************************/
const ru_field_rec PRBS_STATUS_0_LOCK_LOST_LH_FIELD =
{
    "LOCK_LOST_LH",
#if RU_INCLUDE_DESC
    "LOCK_LOST_LH",
    "Indicates latched high indication of lock_lost"
    "",
#endif
    PRBS_STATUS_0_LOCK_LOST_LH_FIELD_MASK,
    0,
    PRBS_STATUS_0_LOCK_LOST_LH_FIELD_WIDTH,
    PRBS_STATUS_0_LOCK_LOST_LH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: PRBS_STATUS_1_LOCK
 ******************************************************************************/
const ru_field_rec PRBS_STATUS_1_LOCK_FIELD =
{
    "LOCK",
#if RU_INCLUDE_DESC
    "LOCK",
    "live indication"
    "",
#endif
    PRBS_STATUS_1_LOCK_FIELD_MASK,
    0,
    PRBS_STATUS_1_LOCK_FIELD_WIDTH,
    PRBS_STATUS_1_LOCK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: PRBS_STATUS_1_ANY_ERR
 ******************************************************************************/
const ru_field_rec PRBS_STATUS_1_ANY_ERR_FIELD =
{
    "ANY_ERR",
#if RU_INCLUDE_DESC
    "ANY_ERR",
    "live indication, indicates if any errors found in a particular clock cycle"
    "",
#endif
    PRBS_STATUS_1_ANY_ERR_FIELD_MASK,
    0,
    PRBS_STATUS_1_ANY_ERR_FIELD_WIDTH,
    PRBS_STATUS_1_ANY_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: PRBS_STATUS_1_RESERVED0
 ******************************************************************************/
const ru_field_rec PRBS_STATUS_1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    PRBS_STATUS_1_RESERVED0_FIELD_MASK,
    0,
    PRBS_STATUS_1_RESERVED0_FIELD_WIDTH,
    PRBS_STATUS_1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: PRBS_CHK_CTRL_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PRBS_CHK_CTRL_0_FIELDS[] =
{
    &PRBS_CHK_CTRL_0_EN_TIMER_MODE_FIELD,
    &PRBS_CHK_CTRL_0_EN_TIMEOUT_FIELD,
    &PRBS_CHK_CTRL_0_RESERVED0_FIELD,
    &PRBS_CHK_CTRL_0_MODE_SEL_FIELD,
    &PRBS_CHK_CTRL_0_ERR_CNT_BURST_MODE_FIELD,
    &PRBS_CHK_CTRL_0_LOCK_CNT_FIELD,
    &PRBS_CHK_CTRL_0_OOL_CNT_FIELD,
    &PRBS_CHK_CTRL_0_INV_FIELD,
    &PRBS_CHK_CTRL_0_SIG_PRBS_STATUS_CLR_FIELD,
    &PRBS_CHK_CTRL_0_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec PRBS_CHK_CTRL_0_REG = 
{
    "CHK_CTRL_0",
#if RU_INCLUDE_DESC
    "PRBS_CHK_CTRL_0 Register",
    "Control over PRBS checker",
#endif
    PRBS_CHK_CTRL_0_REG_OFFSET,
    0,
    0,
    28,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    10,
    PRBS_CHK_CTRL_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: PRBS_CHK_CTRL_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PRBS_CHK_CTRL_1_FIELDS[] =
{
    &PRBS_CHK_CTRL_1_PRBS_TIMER_VAL_FIELD,
    &PRBS_CHK_CTRL_1_RESERVED0_FIELD,
    &PRBS_CHK_CTRL_1_PRBS_CHK_MODE_FIELD,
    &PRBS_CHK_CTRL_1_PRBS_CHK_EN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec PRBS_CHK_CTRL_1_REG = 
{
    "CHK_CTRL_1",
#if RU_INCLUDE_DESC
    "PRBS_CHK_CTRL_1 Register",
    "Control over PRBS checker (2nd register)",
#endif
    PRBS_CHK_CTRL_1_REG_OFFSET,
    0,
    0,
    29,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    PRBS_CHK_CTRL_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: PRBS_STATUS_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PRBS_STATUS_0_FIELDS[] =
{
    &PRBS_STATUS_0_ERR_CNT_FIELD,
    &PRBS_STATUS_0_LOCK_LOST_LH_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec PRBS_STATUS_0_REG = 
{
    "STATUS_0",
#if RU_INCLUDE_DESC
    "PRBS_STATUS_0 Register",
    "PRBS checker status 0",
#endif
    PRBS_STATUS_0_REG_OFFSET,
    0,
    0,
    30,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    PRBS_STATUS_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: PRBS_STATUS_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *PRBS_STATUS_1_FIELDS[] =
{
    &PRBS_STATUS_1_LOCK_FIELD,
    &PRBS_STATUS_1_ANY_ERR_FIELD,
    &PRBS_STATUS_1_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec PRBS_STATUS_1_REG = 
{
    "STATUS_1",
#if RU_INCLUDE_DESC
    "PRBS_STATUS_1 Register",
    "PRBS checker status 1",
#endif
    PRBS_STATUS_1_REG_OFFSET,
    0,
    0,
    31,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    PRBS_STATUS_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: PRBS
 ******************************************************************************/
static const ru_reg_rec *PRBS_REGS[] =
{
    &PRBS_CHK_CTRL_0_REG,
    &PRBS_CHK_CTRL_1_REG,
    &PRBS_STATUS_0_REG,
    &PRBS_STATUS_1_REG,
};

unsigned long PRBS_ADDRS[] =
{
    0x82db20a8,
};

const ru_block_rec PRBS_BLOCK = 
{
    "PRBS",
    PRBS_ADDRS,
    1,
    4,
    PRBS_REGS
};

/* End of file PRBS.c */
