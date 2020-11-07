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

/******************************************************************************
 * Register: NATC_KEY_MASK_KEY_MASK
 ******************************************************************************/
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
    1109,
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
    0x82e503b0,
    0x82e503b4,
    0x82e503b8,
    0x82e503bc,
    0x82e503c0,
    0x82e503c4,
    0x82e503c8,
    0x82e503cc,
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
