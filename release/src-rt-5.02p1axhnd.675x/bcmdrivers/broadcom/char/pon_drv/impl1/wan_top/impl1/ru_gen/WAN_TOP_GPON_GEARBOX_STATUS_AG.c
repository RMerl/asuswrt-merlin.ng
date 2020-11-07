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
    38,
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
 * Block: GPON_GEARBOX_STATUS
 ******************************************************************************/
static const ru_reg_rec *GPON_GEARBOX_STATUS_REGS[] =
{
    &GPON_GEARBOX_STATUS_GEARBOX_STATUS_REG,
};

static unsigned long GPON_GEARBOX_STATUS_ADDRS[] =
{
    0x80144098,
};

const ru_block_rec GPON_GEARBOX_STATUS_BLOCK = 
{
    "GPON_GEARBOX_STATUS",
    GPON_GEARBOX_STATUS_ADDRS,
    1,
    1,
    GPON_GEARBOX_STATUS_REGS
};

/* End of file BCM6858_A0_GPON_GEARBOX_STATUS.c */
