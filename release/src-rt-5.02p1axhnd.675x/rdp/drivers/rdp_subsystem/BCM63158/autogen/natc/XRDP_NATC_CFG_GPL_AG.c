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
 * Register: NATC_CFG_DDR_KEY_BASE_ADDRESS_LOWER
 ******************************************************************************/
const ru_reg_rec NATC_CFG_DDR_KEY_BASE_ADDRESS_LOWER_REG = 
{
    "DDR_KEY_BASE_ADDRESS_LOWER",
#if RU_INCLUDE_DESC
    "Lower 32-bit of NAT table 7 key base address NAT table 7 in DDR",
    "Lower 32-bit of the base address of DDR key table"
    "Address must be 64-bit aligned (bit 2 through 0 are zero's)"
    "In order maximum number of key fetches,"
    "for 16-byte key, bit 3 should be 0 to align at 16 byte boundary"
    "for 32-byte key, bit 3 and 4 should be 0 to align at 32 bytes boundary",
#endif
    NATC_CFG_DDR_KEY_BASE_ADDRESS_LOWER_REG_OFFSET,
    0,
    0,
    942,
};

/******************************************************************************
 * Register: NATC_CFG_DDR_KEY_BASE_ADDRESS_UPPER
 ******************************************************************************/
const ru_reg_rec NATC_CFG_DDR_KEY_BASE_ADDRESS_UPPER_REG = 
{
    "DDR_KEY_BASE_ADDRESS_UPPER",
#if RU_INCLUDE_DESC
    "Upper 32-bit of NAT table 7 key base address NAT table 7 in DDR",
    "Upper 8-bit of the base address of DDR key table"
    "For 32-bit system this field should be left as 0",
#endif
    NATC_CFG_DDR_KEY_BASE_ADDRESS_UPPER_REG_OFFSET,
    0,
    0,
    943,
};

/******************************************************************************
 * Register: NATC_CFG_DDR_RESULT_BASE_ADDRESS_LOWER
 ******************************************************************************/
const ru_reg_rec NATC_CFG_DDR_RESULT_BASE_ADDRESS_LOWER_REG = 
{
    "DDR_RESULT_BASE_ADDRESS_LOWER",
#if RU_INCLUDE_DESC
    "Lower 32-bit of NAT table 7 result base address NAT table 7 in DDR",
    "Lower 32-bit of the base address of DDR context table"
    "Address must be 64-bit aligned (bit 2 through 0 are zero's)",
#endif
    NATC_CFG_DDR_RESULT_BASE_ADDRESS_LOWER_REG_OFFSET,
    0,
    0,
    944,
};

/******************************************************************************
 * Register: NATC_CFG_DDR_RESULT_BASE_ADDRESS_UPPER
 ******************************************************************************/
const ru_reg_rec NATC_CFG_DDR_RESULT_BASE_ADDRESS_UPPER_REG = 
{
    "DDR_RESULT_BASE_ADDRESS_UPPER",
#if RU_INCLUDE_DESC
    "Upper 32-bit of NAT table 7 result base address NAT table 7 in DDR",
    "Upper 8-bit of the base address of DDR context table"
    "For 32-bit system this field should be left as 0",
#endif
    NATC_CFG_DDR_RESULT_BASE_ADDRESS_UPPER_REG_OFFSET,
    0,
    0,
    945,
};

/******************************************************************************
 * Block: NATC_CFG
 ******************************************************************************/
static const ru_reg_rec *NATC_CFG_REGS[] =
{
    &NATC_CFG_DDR_KEY_BASE_ADDRESS_LOWER_REG,
    &NATC_CFG_DDR_KEY_BASE_ADDRESS_UPPER_REG,
    &NATC_CFG_DDR_RESULT_BASE_ADDRESS_LOWER_REG,
    &NATC_CFG_DDR_RESULT_BASE_ADDRESS_UPPER_REG,
};

unsigned long NATC_CFG_ADDRS[] =
{
    0x82e502d0,
    0x82e502e0,
    0x82e502f0,
    0x82e50300,
    0x82e50310,
    0x82e50320,
    0x82e50330,
    0x82e50340,
};

const ru_block_rec NATC_CFG_BLOCK = 
{
    "NATC_CFG",
    NATC_CFG_ADDRS,
    8,
    4,
    NATC_CFG_REGS
};

/* End of file XRDP_NATC_CFG.c */
