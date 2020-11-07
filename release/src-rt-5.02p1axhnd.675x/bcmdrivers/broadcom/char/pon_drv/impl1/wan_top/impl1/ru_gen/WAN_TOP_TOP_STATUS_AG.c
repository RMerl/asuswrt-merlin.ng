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
 * Field: TOP_STATUS_US_RESERVED0
 ******************************************************************************/
const ru_field_rec TOP_STATUS_US_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    TOP_STATUS_US_RESERVED0_FIELD_MASK,
    0,
    TOP_STATUS_US_RESERVED0_FIELD_WIDTH,
    TOP_STATUS_US_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: TOP_STATUS_US_CLK_8KHZ_ALIGNED
 ******************************************************************************/
const ru_field_rec TOP_STATUS_US_CLK_8KHZ_ALIGNED_FIELD =
{
    "CLK_8KHZ_ALIGNED",
#if RU_INCLUDE_DESC
    "",
    "Status indication whether clk_8khz is aligned to the incoming frames"
    "or being adjusted.",
#endif
    TOP_STATUS_US_CLK_8KHZ_ALIGNED_FIELD_MASK,
    0,
    TOP_STATUS_US_CLK_8KHZ_ALIGNED_FIELD_WIDTH,
    TOP_STATUS_US_CLK_8KHZ_ALIGNED_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: TOP_STATUS_US
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TOP_STATUS_US_FIELDS[] =
{
    &TOP_STATUS_US_RESERVED0_FIELD,
    &TOP_STATUS_US_CLK_8KHZ_ALIGNED_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec TOP_STATUS_US_REG = 
{
    "US",
#if RU_INCLUDE_DESC
    "WAN_TOP_STATUS Register",
    "Register used for various WAN status bits.",
#endif
    TOP_STATUS_US_REG_OFFSET,
    0,
    0,
    36,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    TOP_STATUS_US_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: TOP_STATUS
 ******************************************************************************/
static const ru_reg_rec *TOP_STATUS_REGS[] =
{
    &TOP_STATUS_US_REG,
};

static unsigned long TOP_STATUS_ADDRS[] =
{
    0x80144090,
};

const ru_block_rec TOP_STATUS_BLOCK = 
{
    "TOP_STATUS",
    TOP_STATUS_ADDRS,
    1,
    1,
    TOP_STATUS_REGS
};

/* End of file BCM6858_A0_TOP_STATUS.c */
