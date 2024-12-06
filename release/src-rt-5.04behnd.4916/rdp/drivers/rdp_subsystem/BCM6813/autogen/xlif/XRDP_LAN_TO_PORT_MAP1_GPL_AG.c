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
 * Register: XLIF1_LAN_TO_PORT_MAP_PORT_MAP
 ******************************************************************************/
const ru_reg_rec XLIF1_LAN_TO_PORT_MAP_PORT_MAP_REG = 
{
    "PORT_MAP",
#if RU_INCLUDE_DESC
    "PORT_MAP Register",
    "PORT_MAP",
#endif
    XLIF1_LAN_TO_PORT_MAP_PORT_MAP_REG_OFFSET,
    0,
    0,
    1091,
};

/******************************************************************************
 * Block: XLIF1_LAN_TO_PORT_MAP
 ******************************************************************************/
static const ru_reg_rec *XLIF1_LAN_TO_PORT_MAP_REGS[] =
{
    &XLIF1_LAN_TO_PORT_MAP_PORT_MAP_REG,
};

unsigned long XLIF1_LAN_TO_PORT_MAP_ADDRS[] =
{
    0x828b2880,
    0x828b2a80,
    0x828b2c80,
    0x828b2e80,
};

const ru_block_rec XLIF1_LAN_TO_PORT_MAP_BLOCK = 
{
    "XLIF1_LAN_TO_PORT_MAP",
    XLIF1_LAN_TO_PORT_MAP_ADDRS,
    4,
    1,
    XLIF1_LAN_TO_PORT_MAP_REGS
};

/* End of file XRDP_LAN_TO_PORT_MAP1.c */
