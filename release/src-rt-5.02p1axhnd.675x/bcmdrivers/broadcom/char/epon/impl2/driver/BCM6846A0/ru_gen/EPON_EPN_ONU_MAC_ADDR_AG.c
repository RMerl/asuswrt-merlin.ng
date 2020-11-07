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
 * Field: EPN_ONU_MAC_ADDR_LO_MFGADDRREGLO
 ******************************************************************************/
const ru_field_rec EPN_ONU_MAC_ADDR_LO_MFGADDRREGLO_FIELD =
{
    "MFGADDRREGLO",
#if RU_INCLUDE_DESC
    "",
    "ONU MAC Address i",
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
    "ONU MAC Address i",
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
    "ONU MAC Address i",
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
    "EPN_ONU_MAC_ADDR_7_LO Register",
    "These registers store a MAC address for each bidirectional ONU LLID."
    "These addresses are inserted as the SA in REPORT frames sent upstream."
    "Note: ONU MAC Address registers 8 through 23 are used only in loopback"
    "operation.",
#endif
    EPN_ONU_MAC_ADDR_LO_REG_OFFSET,
    0,
    0,
    149,
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
    "EPN_ONU_MAC_ADDR_7_HI Register",
    "These registers store a MAC address for each bidirectional ONU LLID."
    "These addresses are inserted as the SA in REPORT frames sent upstream."
    "Note: ONU MAC Address registers 8 through 23 are used only in loopback"
    "operation.",
#endif
    EPN_ONU_MAC_ADDR_HI_REG_OFFSET,
    0,
    0,
    150,
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
    0x82dad1f0,
    0x82dad1f8,
    0x82dad200,
    0x82dad208,
    0x82dad210,
    0x82dad218,
    0x82dad220,
    0x82dad228,
};

const ru_block_rec EPN_ONU_MAC_ADDR_BLOCK = 
{
    "EPN_ONU_MAC_ADDR",
    EPN_ONU_MAC_ADDR_ADDRS,
    8,
    2,
    EPN_ONU_MAC_ADDR_REGS
};

/* End of file EPON_EPN_ONU_MAC_ADDR.c */
