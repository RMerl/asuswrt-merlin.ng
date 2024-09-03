/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2, as published by
    the Free Software Foundation (the "GPL").
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    
    A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
    writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    
:>
*/

#include "ru.h"

/******************************************************************************
 * Register: XLIF0_LAN_TO_PORT_MAP_PORT_MAP
 ******************************************************************************/
const ru_reg_rec XLIF0_LAN_TO_PORT_MAP_PORT_MAP_REG = 
{
    "PORT_MAP",
#if RU_INCLUDE_DESC
    "PORT_MAP Register",
    "PORT_MAP",
#endif
    XLIF0_LAN_TO_PORT_MAP_PORT_MAP_REG_OFFSET,
    0,
    0,
    1074,
};

/******************************************************************************
 * Block: XLIF0_LAN_TO_PORT_MAP
 ******************************************************************************/
static const ru_reg_rec *XLIF0_LAN_TO_PORT_MAP_REGS[] =
{
    &XLIF0_LAN_TO_PORT_MAP_PORT_MAP_REG,
};

unsigned long XLIF0_LAN_TO_PORT_MAP_ADDRS[] =
{
    0x828b2080,
    0x828b2280,
    0x828b2480,
    0x828b2680,
};

const ru_block_rec XLIF0_LAN_TO_PORT_MAP_BLOCK = 
{
    "XLIF0_LAN_TO_PORT_MAP",
    XLIF0_LAN_TO_PORT_MAP_ADDRS,
    4,
    1,
    XLIF0_LAN_TO_PORT_MAP_REGS
};

/* End of file XRDP_LAN_TO_PORT_MAP0.c */
