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
 * Field: TSEC_EPON_EPON_RESERVED0
 ******************************************************************************/
const ru_field_rec TSEC_EPON_EPON_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    TSEC_EPON_EPON_RESERVED0_FIELD_MASK,
    0,
    TSEC_EPON_EPON_RESERVED0_FIELD_WIDTH,
    TSEC_EPON_EPON_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: TSEC_EPON_EPON_TSEC_EPON_READ
 ******************************************************************************/
const ru_field_rec TSEC_EPON_EPON_TSEC_EPON_READ_FIELD =
{
    "TSEC_EPON_READ",
#if RU_INCLUDE_DESC
    "",
    "Seconds[18:0].",
#endif
    TSEC_EPON_EPON_TSEC_EPON_READ_FIELD_MASK,
    0,
    TSEC_EPON_EPON_TSEC_EPON_READ_FIELD_WIDTH,
    TSEC_EPON_EPON_TSEC_EPON_READ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: TSEC_EPON_EPON
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TSEC_EPON_EPON_FIELDS[] =
{
    &TSEC_EPON_EPON_RESERVED0_FIELD,
    &TSEC_EPON_EPON_TSEC_EPON_READ_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec TSEC_EPON_EPON_REG = 
{
    "EPON",
#if RU_INCLUDE_DESC
    "WAN_TOP_TSEC_EPON Register",
    "Register used for seconds read back from EPON/AE block.",
#endif
    TSEC_EPON_EPON_REG_OFFSET,
    0,
    0,
    29,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    TSEC_EPON_EPON_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: TSEC_EPON
 ******************************************************************************/
static const ru_reg_rec *TSEC_EPON_REGS[] =
{
    &TSEC_EPON_EPON_REG,
};

unsigned long TSEC_EPON_ADDRS[] =
{
    0x801440a8,
};

const ru_block_rec TSEC_EPON_BLOCK = 
{
    "TSEC_EPON",
    TSEC_EPON_ADDRS,
    1,
    1,
    TSEC_EPON_REGS
};

/* End of file TSEC_EPON.c */
