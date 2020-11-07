/*
   Copyright (c) 2015 Broadcom
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
 * Field: NATC_KEY_MASK_KEY_MASK_KEY_MASK
 ******************************************************************************/
const ru_field_rec NATC_KEY_MASK_KEY_MASK_KEY_MASK_FIELD =
{
    "KEY_MASK",
#if RU_INCLUDE_DESC
    "",
    "Specifies the key mask for each byte in the key."
    "each bit corresponds to one byte."
    "0 enables the compare and 1 disables the compare."
    "bit 0 corresponds to byte 0"
    "bit 1 corresponds to byte 1"
    "bit 2 corresponds to byte 2"
    "......................"
    "bit 31 corresponds to byte 31",
#endif
    NATC_KEY_MASK_KEY_MASK_KEY_MASK_FIELD_MASK,
    0,
    NATC_KEY_MASK_KEY_MASK_KEY_MASK_FIELD_WIDTH,
    NATC_KEY_MASK_KEY_MASK_KEY_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: NATC_KEY_MASK_KEY_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NATC_KEY_MASK_KEY_MASK_FIELDS[] =
{
    &NATC_KEY_MASK_KEY_MASK_KEY_MASK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec NATC_KEY_MASK_KEY_MASK_REG = 
{
    "KEY_MASK",
#if RU_INCLUDE_DESC
    "NAT table 7 key mask register",
    "NAT Cache key Mask Register",
#endif
    NATC_KEY_MASK_KEY_MASK_REG_OFFSET,
    0,
    0,
    1053,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    NATC_KEY_MASK_KEY_MASK_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Block: NATC_KEY_MASK
 ******************************************************************************/
static const ru_reg_rec *NATC_KEY_MASK_REGS[] =
{
    &NATC_KEY_MASK_KEY_MASK_REG,
};

unsigned long NATC_KEY_MASK_ADDRS[] =
{
    0x82e50330,
    0x82e50334,
    0x82e50338,
    0x82e5033c,
    0x82e50340,
    0x82e50344,
    0x82e50348,
    0x82e5034c,
};

const ru_block_rec NATC_KEY_MASK_BLOCK = 
{
    "NATC_KEY_MASK",
    NATC_KEY_MASK_ADDRS,
    8,
    1,
    NATC_KEY_MASK_REGS
};

/* End of file XRDP_NATC_KEY_MASK.c */
