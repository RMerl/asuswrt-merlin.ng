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
 * Register: DEBUG_BUS_BUS_SEL
 ******************************************************************************/
const ru_reg_rec DEBUG_BUS_BUS_SEL_REG = 
{
    "BUS_SEL",
#if RU_INCLUDE_DESC
    "SEL Register",
    "Select",
#endif
    DEBUG_BUS_BUS_SEL_REG_OFFSET,
    0,
    0,
    613,
};

/******************************************************************************
 * Block: DEBUG_BUS
 ******************************************************************************/
static const ru_reg_rec *DEBUG_BUS_REGS[] =
{
    &DEBUG_BUS_BUS_SEL_REG,
};

unsigned long DEBUG_BUS_ADDRS[] =
{
    0x82d2b070,
    0x82d2b270,
    0x82d2b470,
    0x82d2b670,
    0x82d2b870,
    0x82d2ba70,
    0x82d2bc70,
    0x82d2be70,
};

const ru_block_rec DEBUG_BUS_BLOCK = 
{
    "DEBUG_BUS",
    DEBUG_BUS_ADDRS,
    8,
    1,
    DEBUG_BUS_REGS
};

/* End of file XRDP_DEBUG_BUS.c */
