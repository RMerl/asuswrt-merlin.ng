/*
   Copyright (c) 2015 Broadcom Corporation
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
 * Field: EPN_ONU_MAC_ADDR_LO_MFGADDRREGLO
 ******************************************************************************/
const ru_field_rec EPN_ONU_MAC_ADDR_LO_MFGADDRREGLO_FIELD =
{
    "MFGADDRREGLO",
#if RU_INCLUDE_DESC
    "",
    "ONU MAC Address i"
    "In Point-To-Point mode; this address is used as the DA[31:24] for"
    "PFC matching.",
#endif
    EPN_ONU_MAC_ADDR_LO_MFGADDRREGLO_FIELD_MASK,
    0,
    EPN_ONU_MAC_ADDR_LO_MFGADDRREGLO_FIELD_WIDTH,
    EPN_ONU_MAC_ADDR_LO_MFGADDRREGLO_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_ONU_MAC_ADDR_LO_ONUADDRREG
 ******************************************************************************/
const ru_field_rec EPN_ONU_MAC_ADDR_LO_ONUADDRREG_FIELD =
{
    "ONUADDRREG",
#if RU_INCLUDE_DESC
    "",
    "ONU MAC Address i"
    "In Point-To-Point mode; this address is used as the DA[23:0] for PFC"
    "matching.",
#endif
    EPN_ONU_MAC_ADDR_LO_ONUADDRREG_FIELD_MASK,
    0,
    EPN_ONU_MAC_ADDR_LO_ONUADDRREG_FIELD_WIDTH,
    EPN_ONU_MAC_ADDR_LO_ONUADDRREG_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_ONU_MAC_ADDR_HI_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_ONU_MAC_ADDR_HI_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_ONU_MAC_ADDR_HI_RESERVED0_FIELD_MASK,
    0,
    EPN_ONU_MAC_ADDR_HI_RESERVED0_FIELD_WIDTH,
    EPN_ONU_MAC_ADDR_HI_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_ONU_MAC_ADDR_HI_MFGADDRREGHI
 ******************************************************************************/
const ru_field_rec EPN_ONU_MAC_ADDR_HI_MFGADDRREGHI_FIELD =
{
    "MFGADDRREGHI",
#if RU_INCLUDE_DESC
    "",
    "ONU MAC Address i"
    "In Point-To-Point mode; this address is used as the DA[47:32] for"
    "PFC matching.",
#endif
    EPN_ONU_MAC_ADDR_HI_MFGADDRREGHI_FIELD_MASK,
    0,
    EPN_ONU_MAC_ADDR_HI_MFGADDRREGHI_FIELD_WIDTH,
    EPN_ONU_MAC_ADDR_HI_MFGADDRREGHI_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: EPN_ONU_MAC_ADDR_LO
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_ONU_MAC_ADDR_LO_FIELDS[] =
{
    &EPN_ONU_MAC_ADDR_LO_MFGADDRREGLO_FIELD,
    &EPN_ONU_MAC_ADDR_LO_ONUADDRREG_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_ONU_MAC_ADDR_LO_REG = 
{
    "LO",
#if RU_INCLUDE_DESC
    "EPN_ONU_MAC_ADDR_31_LO Register",
    "These registers store a MAC address for each bidirectional ONU LLID."
    "These addresses are inserted as the SA in REPORT frames sent upstream."
    "In Point-To-Point mode; this address is used as the DA for PFC"
    "matching."
    "Note: ONU MAC Address registers 8 through 23 are used only in loopback"
    "operation.",
#endif
    EPN_ONU_MAC_ADDR_LO_REG_OFFSET,
    0,
    0,
    19,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_ONU_MAC_ADDR_LO_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_ONU_MAC_ADDR_HI
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_ONU_MAC_ADDR_HI_FIELDS[] =
{
    &EPN_ONU_MAC_ADDR_HI_RESERVED0_FIELD,
    &EPN_ONU_MAC_ADDR_HI_MFGADDRREGHI_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_ONU_MAC_ADDR_HI_REG = 
{
    "HI",
#if RU_INCLUDE_DESC
    "EPN_ONU_MAC_ADDR_31_HI Register",
    "These registers store a MAC address for each bidirectional ONU LLID."
    "These addresses are inserted as the SA in REPORT frames sent upstream."
    "In Point-To-Point mode; this address is used as the DA[47:32] for PFC"
    "matching."
    "Note: ONU MAC Address registers 8 through 23 are used only in loopback"
    "operation.",
#endif
    EPN_ONU_MAC_ADDR_HI_REG_OFFSET,
    0,
    0,
    20,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_ONU_MAC_ADDR_HI_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: EPN_ONU_MAC_ADDR
 ******************************************************************************/
static const ru_reg_rec *EPN_ONU_MAC_ADDR_REGS[] =
{
    &EPN_ONU_MAC_ADDR_LO_REG,
    &EPN_ONU_MAC_ADDR_HI_REG,
};

static unsigned long EPN_ONU_MAC_ADDR_ADDRS[] =
{
    0x828c11f0,
    0x828c11f8,
    0x828c1200,
    0x828c1208,
    0x828c1210,
    0x828c1218,
    0x828c1220,
    0x828c1228,
    0x828c1458,
    0x828c1460,
    0x828c1468,
    0x828c1470,
    0x828c1478,
    0x828c1480,
    0x828c1488,
    0x828c1490,
    0x828c1498,
    0x828c14a0,
    0x828c14a8,
    0x828c14b0,
    0x828c14b8,
    0x828c14c0,
    0x828c14c8,
    0x828c14d0,
    0x828c14d8,
    0x828c14e0,
    0x828c14e8,
    0x828c14f0,
    0x828c14f8,
    0x828c1500,
    0x828c1508,
    0x828c1510,
};

const ru_block_rec EPN_ONU_MAC_ADDR_BLOCK = 
{
    "EPN_ONU_MAC_ADDR",
    EPN_ONU_MAC_ADDR_ADDRS,
    32,
    2,
    EPN_ONU_MAC_ADDR_REGS
};

/* End of file EPON_EPN_ONU_MAC_ADDR.c */
