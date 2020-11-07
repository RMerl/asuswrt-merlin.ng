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
 * Register: RNR_CNTXT_MEM_ENTRY
 ******************************************************************************/
const ru_reg_rec RNR_CNTXT_MEM_ENTRY_REG = 
{
    "MEM_ENTRY",
#if RU_INCLUDE_DESC
    "CONTEXT_MEM_ENTRY Register",
    "Context mem entry",
#endif
    RNR_CNTXT_MEM_ENTRY_REG_OFFSET,
    RNR_CNTXT_MEM_ENTRY_REG_RAM_CNT,
    4,
    308,
};

/******************************************************************************
 * Block: RNR_CNTXT
 ******************************************************************************/
static const ru_reg_rec *RNR_CNTXT_REGS[] =
{
    &RNR_CNTXT_MEM_ENTRY_REG,
};

unsigned long RNR_CNTXT_ADDRS[] =
{
    0x82c18000,
    0x82c38000,
    0x82c58000,
    0x82c78000,
    0x82c98000,
    0x82cb8000,
};

const ru_block_rec RNR_CNTXT_BLOCK = 
{
    "RNR_CNTXT",
    RNR_CNTXT_ADDRS,
    6,
    1,
    RNR_CNTXT_REGS
};

/* End of file XRDP_RNR_CNTXT.c */
