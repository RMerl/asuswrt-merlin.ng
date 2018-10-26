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
 * Register: XLIF_EEE_IND
 ******************************************************************************/
const ru_reg_rec XLIF_EEE_IND_REG = 
{
    "IND",
#if RU_INCLUDE_DESC
    "INDICATIONS Register",
    "eee indications from the XLMAC interface",
#endif
    XLIF_EEE_IND_REG_OFFSET,
    0,
    0,
    614,
};

/******************************************************************************
 * Block: XLIF_EEE
 ******************************************************************************/
static const ru_reg_rec *XLIF_EEE_REGS[] =
{
    &XLIF_EEE_IND_REG,
};

unsigned long XLIF_EEE_ADDRS[] =
{
    0x82d2b078,
    0x82d2b278,
    0x82d2b478,
    0x82d2b678,
    0x82d2b878,
    0x82d2ba78,
    0x82d2bc78,
    0x82d2be78,
};

const ru_block_rec XLIF_EEE_BLOCK = 
{
    "XLIF_EEE",
    XLIF_EEE_ADDRS,
    8,
    1,
    XLIF_EEE_REGS
};

/* End of file XRDP_XLIF_EEE.c */
