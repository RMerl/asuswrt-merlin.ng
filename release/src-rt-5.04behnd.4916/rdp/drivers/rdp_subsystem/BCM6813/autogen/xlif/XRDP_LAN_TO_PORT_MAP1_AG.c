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

#if RU_INCLUDE_FIELD_DB
/******************************************************************************
 * Field: XLIF1_LAN_TO_PORT_MAP_PORT_MAP_LAN0
 ******************************************************************************/
const ru_field_rec XLIF1_LAN_TO_PORT_MAP_PORT_MAP_LAN0_FIELD =
{
    "LAN0",
#if RU_INCLUDE_DESC
    "LAN0",
    "XLMAC port to be mapped to BBH0",
#endif
    XLIF1_LAN_TO_PORT_MAP_PORT_MAP_LAN0_FIELD_MASK,
    0,
    XLIF1_LAN_TO_PORT_MAP_PORT_MAP_LAN0_FIELD_WIDTH,
    XLIF1_LAN_TO_PORT_MAP_PORT_MAP_LAN0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XLIF1_LAN_TO_PORT_MAP_PORT_MAP_LAN1
 ******************************************************************************/
const ru_field_rec XLIF1_LAN_TO_PORT_MAP_PORT_MAP_LAN1_FIELD =
{
    "LAN1",
#if RU_INCLUDE_DESC
    "LAN1",
    "XLMAC port which is mapped to BBH1",
#endif
    XLIF1_LAN_TO_PORT_MAP_PORT_MAP_LAN1_FIELD_MASK,
    0,
    XLIF1_LAN_TO_PORT_MAP_PORT_MAP_LAN1_FIELD_WIDTH,
    XLIF1_LAN_TO_PORT_MAP_PORT_MAP_LAN1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XLIF1_LAN_TO_PORT_MAP_PORT_MAP_LAN2
 ******************************************************************************/
const ru_field_rec XLIF1_LAN_TO_PORT_MAP_PORT_MAP_LAN2_FIELD =
{
    "LAN2",
#if RU_INCLUDE_DESC
    "LAN2",
    "XLMAC port which is mapped to BBH 2",
#endif
    XLIF1_LAN_TO_PORT_MAP_PORT_MAP_LAN2_FIELD_MASK,
    0,
    XLIF1_LAN_TO_PORT_MAP_PORT_MAP_LAN2_FIELD_WIDTH,
    XLIF1_LAN_TO_PORT_MAP_PORT_MAP_LAN2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XLIF1_LAN_TO_PORT_MAP_PORT_MAP_LAN3
 ******************************************************************************/
const ru_field_rec XLIF1_LAN_TO_PORT_MAP_PORT_MAP_LAN3_FIELD =
{
    "LAN3",
#if RU_INCLUDE_DESC
    "LAN3",
    "XLMAC port which is mapped to BBH 3",
#endif
    XLIF1_LAN_TO_PORT_MAP_PORT_MAP_LAN3_FIELD_MASK,
    0,
    XLIF1_LAN_TO_PORT_MAP_PORT_MAP_LAN3_FIELD_WIDTH,
    XLIF1_LAN_TO_PORT_MAP_PORT_MAP_LAN3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XLIF1_LAN_TO_PORT_MAP_PORT_MAP_RESERVED0
 ******************************************************************************/
const ru_field_rec XLIF1_LAN_TO_PORT_MAP_PORT_MAP_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XLIF1_LAN_TO_PORT_MAP_PORT_MAP_RESERVED0_FIELD_MASK,
    0,
    XLIF1_LAN_TO_PORT_MAP_PORT_MAP_RESERVED0_FIELD_WIDTH,
    XLIF1_LAN_TO_PORT_MAP_PORT_MAP_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: XLIF1_LAN_TO_PORT_MAP_PORT_MAP
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XLIF1_LAN_TO_PORT_MAP_PORT_MAP_FIELDS[] =
{
    &XLIF1_LAN_TO_PORT_MAP_PORT_MAP_LAN0_FIELD,
    &XLIF1_LAN_TO_PORT_MAP_PORT_MAP_LAN1_FIELD,
    &XLIF1_LAN_TO_PORT_MAP_PORT_MAP_LAN2_FIELD,
    &XLIF1_LAN_TO_PORT_MAP_PORT_MAP_LAN3_FIELD,
    &XLIF1_LAN_TO_PORT_MAP_PORT_MAP_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    XLIF1_LAN_TO_PORT_MAP_PORT_MAP_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
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
