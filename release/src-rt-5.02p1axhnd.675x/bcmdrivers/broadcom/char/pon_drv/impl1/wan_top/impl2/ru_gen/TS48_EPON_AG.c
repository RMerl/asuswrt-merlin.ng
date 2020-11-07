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
 * Field: TS48_EPON_EPON_MSB_RESERVED0
 ******************************************************************************/
const ru_field_rec TS48_EPON_EPON_MSB_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    TS48_EPON_EPON_MSB_RESERVED0_FIELD_MASK,
    0,
    TS48_EPON_EPON_MSB_RESERVED0_FIELD_WIDTH,
    TS48_EPON_EPON_MSB_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: TS48_EPON_EPON_MSB_TS48_EPON_READ_MSB
 ******************************************************************************/
const ru_field_rec TS48_EPON_EPON_MSB_TS48_EPON_READ_MSB_FIELD =
{
    "TS48_EPON_READ_MSB",
#if RU_INCLUDE_DESC
    "",
    "Upper 16-bits of TS48.",
#endif
    TS48_EPON_EPON_MSB_TS48_EPON_READ_MSB_FIELD_MASK,
    0,
    TS48_EPON_EPON_MSB_TS48_EPON_READ_MSB_FIELD_WIDTH,
    TS48_EPON_EPON_MSB_TS48_EPON_READ_MSB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: TS48_EPON_EPON_LSB_TS48_EPON_READ_LSB
 ******************************************************************************/
const ru_field_rec TS48_EPON_EPON_LSB_TS48_EPON_READ_LSB_FIELD =
{
    "TS48_EPON_READ_LSB",
#if RU_INCLUDE_DESC
    "",
    "Lower 32-bits of TS48.",
#endif
    TS48_EPON_EPON_LSB_TS48_EPON_READ_LSB_FIELD_MASK,
    0,
    TS48_EPON_EPON_LSB_TS48_EPON_READ_LSB_FIELD_WIDTH,
    TS48_EPON_EPON_LSB_TS48_EPON_READ_LSB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: TS48_EPON_EPON_MSB
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TS48_EPON_EPON_MSB_FIELDS[] =
{
    &TS48_EPON_EPON_MSB_RESERVED0_FIELD,
    &TS48_EPON_EPON_MSB_TS48_EPON_READ_MSB_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec TS48_EPON_EPON_MSB_REG = 
{
    "EPON_MSB",
#if RU_INCLUDE_DESC
    "WAN_TOP_TS48_EPON_MSB Register",
    "Register used for 48-bit timestamp Time Of Day (TOD) read back from"
    "EPON/AE block.",
#endif
    TS48_EPON_EPON_MSB_REG_OFFSET,
    0,
    0,
    27,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    TS48_EPON_EPON_MSB_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: TS48_EPON_EPON_LSB
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TS48_EPON_EPON_LSB_FIELDS[] =
{
    &TS48_EPON_EPON_LSB_TS48_EPON_READ_LSB_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec TS48_EPON_EPON_LSB_REG = 
{
    "EPON_LSB",
#if RU_INCLUDE_DESC
    "WAN_TOP_TS48_EPON_LSB Register",
    "Register used for 48-bit timestamp Time Of Day (TOD) read back from"
    "EPON/AE block.",
#endif
    TS48_EPON_EPON_LSB_REG_OFFSET,
    0,
    0,
    28,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TS48_EPON_EPON_LSB_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: TS48_EPON
 ******************************************************************************/
static const ru_reg_rec *TS48_EPON_REGS[] =
{
    &TS48_EPON_EPON_MSB_REG,
    &TS48_EPON_EPON_LSB_REG,
};

unsigned long TS48_EPON_ADDRS[] =
{
    0x801440a0,
};

const ru_block_rec TS48_EPON_BLOCK = 
{
    "TS48_EPON",
    TS48_EPON_ADDRS,
    1,
    2,
    TS48_EPON_REGS
};

/* End of file TS48_EPON.c */
