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
 * Field: WAN_SERDES_PLL_CTL_RESERVED0
 ******************************************************************************/
const ru_field_rec WAN_SERDES_PLL_CTL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    WAN_SERDES_PLL_CTL_RESERVED0_FIELD_MASK,
    0,
    WAN_SERDES_PLL_CTL_RESERVED0_FIELD_WIDTH,
    WAN_SERDES_PLL_CTL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: WAN_SERDES_PLL_CTL_CFG_PLL1_LCREF_SEL
 ******************************************************************************/
const ru_field_rec WAN_SERDES_PLL_CTL_CFG_PLL1_LCREF_SEL_FIELD =
{
    "CFG_PLL1_LCREF_SEL",
#if RU_INCLUDE_DESC
    "",
    "0 - select pll1_lcref.  1 - select pll0_lcref.",
#endif
    WAN_SERDES_PLL_CTL_CFG_PLL1_LCREF_SEL_FIELD_MASK,
    0,
    WAN_SERDES_PLL_CTL_CFG_PLL1_LCREF_SEL_FIELD_WIDTH,
    WAN_SERDES_PLL_CTL_CFG_PLL1_LCREF_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: WAN_SERDES_PLL_CTL_CFG_PLL1_REFOUT_EN
 ******************************************************************************/
const ru_field_rec WAN_SERDES_PLL_CTL_CFG_PLL1_REFOUT_EN_FIELD =
{
    "CFG_PLL1_REFOUT_EN",
#if RU_INCLUDE_DESC
    "",
    "Enables SERDES to drive the pll1_refout pin.  0 - output is hiZ.  1"
    "- output is pad_pll1_refclk.",
#endif
    WAN_SERDES_PLL_CTL_CFG_PLL1_REFOUT_EN_FIELD_MASK,
    0,
    WAN_SERDES_PLL_CTL_CFG_PLL1_REFOUT_EN_FIELD_WIDTH,
    WAN_SERDES_PLL_CTL_CFG_PLL1_REFOUT_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: WAN_SERDES_PLL_CTL_CFG_PLL1_REFIN_EN
 ******************************************************************************/
const ru_field_rec WAN_SERDES_PLL_CTL_CFG_PLL1_REFIN_EN_FIELD =
{
    "CFG_PLL1_REFIN_EN",
#if RU_INCLUDE_DESC
    "",
    "Reference select. 0 - select pad_pll1_refclkp/n.  1 - select"
    "pll1_lcrefp/n.",
#endif
    WAN_SERDES_PLL_CTL_CFG_PLL1_REFIN_EN_FIELD_MASK,
    0,
    WAN_SERDES_PLL_CTL_CFG_PLL1_REFIN_EN_FIELD_WIDTH,
    WAN_SERDES_PLL_CTL_CFG_PLL1_REFIN_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: WAN_SERDES_PLL_CTL_RESERVED1
 ******************************************************************************/
const ru_field_rec WAN_SERDES_PLL_CTL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    WAN_SERDES_PLL_CTL_RESERVED1_FIELD_MASK,
    0,
    WAN_SERDES_PLL_CTL_RESERVED1_FIELD_WIDTH,
    WAN_SERDES_PLL_CTL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: WAN_SERDES_PLL_CTL_CFG_PLL0_LCREF_SEL
 ******************************************************************************/
const ru_field_rec WAN_SERDES_PLL_CTL_CFG_PLL0_LCREF_SEL_FIELD =
{
    "CFG_PLL0_LCREF_SEL",
#if RU_INCLUDE_DESC
    "",
    "0 - select pll0_lcref.  1 - select pll1_lcref.",
#endif
    WAN_SERDES_PLL_CTL_CFG_PLL0_LCREF_SEL_FIELD_MASK,
    0,
    WAN_SERDES_PLL_CTL_CFG_PLL0_LCREF_SEL_FIELD_WIDTH,
    WAN_SERDES_PLL_CTL_CFG_PLL0_LCREF_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: WAN_SERDES_PLL_CTL_CFG_PLL0_REFOUT_EN
 ******************************************************************************/
const ru_field_rec WAN_SERDES_PLL_CTL_CFG_PLL0_REFOUT_EN_FIELD =
{
    "CFG_PLL0_REFOUT_EN",
#if RU_INCLUDE_DESC
    "",
    "Enables SERDES to drive the pll0_refout pin.  0 - output is hiZ.  1"
    "- output is pad_pll0_refclk.",
#endif
    WAN_SERDES_PLL_CTL_CFG_PLL0_REFOUT_EN_FIELD_MASK,
    0,
    WAN_SERDES_PLL_CTL_CFG_PLL0_REFOUT_EN_FIELD_WIDTH,
    WAN_SERDES_PLL_CTL_CFG_PLL0_REFOUT_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: WAN_SERDES_PLL_CTL_CFG_PLL0_REFIN_EN
 ******************************************************************************/
const ru_field_rec WAN_SERDES_PLL_CTL_CFG_PLL0_REFIN_EN_FIELD =
{
    "CFG_PLL0_REFIN_EN",
#if RU_INCLUDE_DESC
    "",
    "Reference select. 0 - select pad_pll0_refclkp/n.  1 - select"
    "pll0_lcrefp/n.",
#endif
    WAN_SERDES_PLL_CTL_CFG_PLL0_REFIN_EN_FIELD_MASK,
    0,
    WAN_SERDES_PLL_CTL_CFG_PLL0_REFIN_EN_FIELD_WIDTH,
    WAN_SERDES_PLL_CTL_CFG_PLL0_REFIN_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: WAN_SERDES_TEMP_CTL_RESERVED0
 ******************************************************************************/
const ru_field_rec WAN_SERDES_TEMP_CTL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    WAN_SERDES_TEMP_CTL_RESERVED0_FIELD_MASK,
    0,
    WAN_SERDES_TEMP_CTL_RESERVED0_FIELD_WIDTH,
    WAN_SERDES_TEMP_CTL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: WAN_SERDES_TEMP_CTL_WAN_TEMPERATURE_DATA
 ******************************************************************************/
const ru_field_rec WAN_SERDES_TEMP_CTL_WAN_TEMPERATURE_DATA_FIELD =
{
    "WAN_TEMPERATURE_DATA",
#if RU_INCLUDE_DESC
    "",
    "10-bit temperature data. Please refer to TMON documentation for how"
    "to convert this value to a useful number.",
#endif
    WAN_SERDES_TEMP_CTL_WAN_TEMPERATURE_DATA_FIELD_MASK,
    0,
    WAN_SERDES_TEMP_CTL_WAN_TEMPERATURE_DATA_FIELD_WIDTH,
    WAN_SERDES_TEMP_CTL_WAN_TEMPERATURE_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: WAN_SERDES_PRAM_CTL_CFG_PRAM_GO
 ******************************************************************************/
const ru_field_rec WAN_SERDES_PRAM_CTL_CFG_PRAM_GO_FIELD =
{
    "CFG_PRAM_GO",
#if RU_INCLUDE_DESC
    "",
    "Perform pRAM operation.  This field is only valid for the B0 or"
    "beyond.  Software sets and hardware clears this bit.  Do not write"
    "to this register if this bit is set.",
#endif
    WAN_SERDES_PRAM_CTL_CFG_PRAM_GO_FIELD_MASK,
    0,
    WAN_SERDES_PRAM_CTL_CFG_PRAM_GO_FIELD_WIDTH,
    WAN_SERDES_PRAM_CTL_CFG_PRAM_GO_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: WAN_SERDES_PRAM_CTL_RESERVED0
 ******************************************************************************/
const ru_field_rec WAN_SERDES_PRAM_CTL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    WAN_SERDES_PRAM_CTL_RESERVED0_FIELD_MASK,
    0,
    WAN_SERDES_PRAM_CTL_RESERVED0_FIELD_WIDTH,
    WAN_SERDES_PRAM_CTL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: WAN_SERDES_PRAM_CTL_CFG_PRAM_WE
 ******************************************************************************/
const ru_field_rec WAN_SERDES_PRAM_CTL_CFG_PRAM_WE_FIELD =
{
    "CFG_PRAM_WE",
#if RU_INCLUDE_DESC
    "",
    "Program RAM write strobe.",
#endif
    WAN_SERDES_PRAM_CTL_CFG_PRAM_WE_FIELD_MASK,
    0,
    WAN_SERDES_PRAM_CTL_CFG_PRAM_WE_FIELD_WIDTH,
    WAN_SERDES_PRAM_CTL_CFG_PRAM_WE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: WAN_SERDES_PRAM_CTL_CFG_PRAM_CS
 ******************************************************************************/
const ru_field_rec WAN_SERDES_PRAM_CTL_CFG_PRAM_CS_FIELD =
{
    "CFG_PRAM_CS",
#if RU_INCLUDE_DESC
    "",
    "Program RAM chip select. This field is only valid for the A0 version"
    "of the chip.",
#endif
    WAN_SERDES_PRAM_CTL_CFG_PRAM_CS_FIELD_MASK,
    0,
    WAN_SERDES_PRAM_CTL_CFG_PRAM_CS_FIELD_WIDTH,
    WAN_SERDES_PRAM_CTL_CFG_PRAM_CS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: WAN_SERDES_PRAM_CTL_CFG_PRAM_ABILITY
 ******************************************************************************/
const ru_field_rec WAN_SERDES_PRAM_CTL_CFG_PRAM_ABILITY_FIELD =
{
    "CFG_PRAM_ABILITY",
#if RU_INCLUDE_DESC
    "",
    "Ability to support parallel bus interface to access program RAM.  0"
    "- not supported. 1 - supported. This field is only valid for the A0"
    "version of the chip.",
#endif
    WAN_SERDES_PRAM_CTL_CFG_PRAM_ABILITY_FIELD_MASK,
    0,
    WAN_SERDES_PRAM_CTL_CFG_PRAM_ABILITY_FIELD_WIDTH,
    WAN_SERDES_PRAM_CTL_CFG_PRAM_ABILITY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: WAN_SERDES_PRAM_CTL_CFG_PRAM_DATAIN
 ******************************************************************************/
const ru_field_rec WAN_SERDES_PRAM_CTL_CFG_PRAM_DATAIN_FIELD =
{
    "CFG_PRAM_DATAIN",
#if RU_INCLUDE_DESC
    "",
    "Program RAM write data. This field is only valid for the A0 version"
    "of this chip.  For B0 or beyond, use the data field in"
    "WAN_SERDES_PRAM_CTL_2/3.",
#endif
    WAN_SERDES_PRAM_CTL_CFG_PRAM_DATAIN_FIELD_MASK,
    0,
    WAN_SERDES_PRAM_CTL_CFG_PRAM_DATAIN_FIELD_WIDTH,
    WAN_SERDES_PRAM_CTL_CFG_PRAM_DATAIN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: WAN_SERDES_PRAM_CTL_CFG_PRAM_ADDR
 ******************************************************************************/
const ru_field_rec WAN_SERDES_PRAM_CTL_CFG_PRAM_ADDR_FIELD =
{
    "CFG_PRAM_ADDR",
#if RU_INCLUDE_DESC
    "",
    "Program RAM address.",
#endif
    WAN_SERDES_PRAM_CTL_CFG_PRAM_ADDR_FIELD_MASK,
    0,
    WAN_SERDES_PRAM_CTL_CFG_PRAM_ADDR_FIELD_WIDTH,
    WAN_SERDES_PRAM_CTL_CFG_PRAM_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: WAN_SERDES_PRAM_CTL_2_CFG_PRAM_DATAIN_0
 ******************************************************************************/
const ru_field_rec WAN_SERDES_PRAM_CTL_2_CFG_PRAM_DATAIN_0_FIELD =
{
    "CFG_PRAM_DATAIN_0",
#if RU_INCLUDE_DESC
    "",
    "Bits [31:0] of the 64-bit pRAM write data interface.",
#endif
    WAN_SERDES_PRAM_CTL_2_CFG_PRAM_DATAIN_0_FIELD_MASK,
    0,
    WAN_SERDES_PRAM_CTL_2_CFG_PRAM_DATAIN_0_FIELD_WIDTH,
    WAN_SERDES_PRAM_CTL_2_CFG_PRAM_DATAIN_0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: WAN_SERDES_PRAM_CTL_3_CFG_PRAM_DATAIN_1
 ******************************************************************************/
const ru_field_rec WAN_SERDES_PRAM_CTL_3_CFG_PRAM_DATAIN_1_FIELD =
{
    "CFG_PRAM_DATAIN_1",
#if RU_INCLUDE_DESC
    "",
    "Bits [63:32] of the 64-bit pRAM write data interface.",
#endif
    WAN_SERDES_PRAM_CTL_3_CFG_PRAM_DATAIN_1_FIELD_MASK,
    0,
    WAN_SERDES_PRAM_CTL_3_CFG_PRAM_DATAIN_1_FIELD_WIDTH,
    WAN_SERDES_PRAM_CTL_3_CFG_PRAM_DATAIN_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: WAN_SERDES_PLL_CTL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *WAN_SERDES_PLL_CTL_FIELDS[] =
{
    &WAN_SERDES_PLL_CTL_RESERVED0_FIELD,
    &WAN_SERDES_PLL_CTL_CFG_PLL1_LCREF_SEL_FIELD,
    &WAN_SERDES_PLL_CTL_CFG_PLL1_REFOUT_EN_FIELD,
    &WAN_SERDES_PLL_CTL_CFG_PLL1_REFIN_EN_FIELD,
    &WAN_SERDES_PLL_CTL_RESERVED1_FIELD,
    &WAN_SERDES_PLL_CTL_CFG_PLL0_LCREF_SEL_FIELD,
    &WAN_SERDES_PLL_CTL_CFG_PLL0_REFOUT_EN_FIELD,
    &WAN_SERDES_PLL_CTL_CFG_PLL0_REFIN_EN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec WAN_SERDES_PLL_CTL_REG = 
{
    "PLL_CTL",
#if RU_INCLUDE_DESC
    "WAN_SERDES_PLL_CTL Register",
    "Register used for low configuration of PLL clocks.",
#endif
    WAN_SERDES_PLL_CTL_REG_OFFSET,
    0,
    0,
    35,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    WAN_SERDES_PLL_CTL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: WAN_SERDES_TEMP_CTL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *WAN_SERDES_TEMP_CTL_FIELDS[] =
{
    &WAN_SERDES_TEMP_CTL_RESERVED0_FIELD,
    &WAN_SERDES_TEMP_CTL_WAN_TEMPERATURE_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec WAN_SERDES_TEMP_CTL_REG = 
{
    "TEMP_CTL",
#if RU_INCLUDE_DESC
    "WAN_SERDES_TEMP_CTL Register",
    "Register used for temperature read.",
#endif
    WAN_SERDES_TEMP_CTL_REG_OFFSET,
    0,
    0,
    36,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    WAN_SERDES_TEMP_CTL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: WAN_SERDES_PRAM_CTL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *WAN_SERDES_PRAM_CTL_FIELDS[] =
{
    &WAN_SERDES_PRAM_CTL_CFG_PRAM_GO_FIELD,
    &WAN_SERDES_PRAM_CTL_RESERVED0_FIELD,
    &WAN_SERDES_PRAM_CTL_CFG_PRAM_WE_FIELD,
    &WAN_SERDES_PRAM_CTL_CFG_PRAM_CS_FIELD,
    &WAN_SERDES_PRAM_CTL_CFG_PRAM_ABILITY_FIELD,
    &WAN_SERDES_PRAM_CTL_CFG_PRAM_DATAIN_FIELD,
    &WAN_SERDES_PRAM_CTL_CFG_PRAM_ADDR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec WAN_SERDES_PRAM_CTL_REG = 
{
    "PRAM_CTL",
#if RU_INCLUDE_DESC
    "WAN_SERDES_PRAM_CTL Register",
    "Register used for PRAM control.",
#endif
    WAN_SERDES_PRAM_CTL_REG_OFFSET,
    0,
    0,
    37,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    WAN_SERDES_PRAM_CTL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: WAN_SERDES_PRAM_CTL_2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *WAN_SERDES_PRAM_CTL_2_FIELDS[] =
{
    &WAN_SERDES_PRAM_CTL_2_CFG_PRAM_DATAIN_0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec WAN_SERDES_PRAM_CTL_2_REG = 
{
    "PRAM_CTL_2",
#if RU_INCLUDE_DESC
    "WAN_SERDES_PRAM_CTL_2 Register",
    "Register used for PRAM control.",
#endif
    WAN_SERDES_PRAM_CTL_2_REG_OFFSET,
    0,
    0,
    38,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    WAN_SERDES_PRAM_CTL_2_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: WAN_SERDES_PRAM_CTL_3
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *WAN_SERDES_PRAM_CTL_3_FIELDS[] =
{
    &WAN_SERDES_PRAM_CTL_3_CFG_PRAM_DATAIN_1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec WAN_SERDES_PRAM_CTL_3_REG = 
{
    "PRAM_CTL_3",
#if RU_INCLUDE_DESC
    "WAN_SERDES_PRAM_CTL_3 Register",
    "Register used for PRAM control.",
#endif
    WAN_SERDES_PRAM_CTL_3_REG_OFFSET,
    0,
    0,
    39,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    WAN_SERDES_PRAM_CTL_3_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: WAN_SERDES
 ******************************************************************************/
static const ru_reg_rec *WAN_SERDES_REGS[] =
{
    &WAN_SERDES_PLL_CTL_REG,
    &WAN_SERDES_TEMP_CTL_REG,
    &WAN_SERDES_PRAM_CTL_REG,
    &WAN_SERDES_PRAM_CTL_2_REG,
    &WAN_SERDES_PRAM_CTL_3_REG,
};

unsigned long WAN_SERDES_ADDRS[] =
{
    0x80144050,
};

const ru_block_rec WAN_SERDES_BLOCK = 
{
    "WAN_SERDES",
    WAN_SERDES_ADDRS,
    1,
    5,
    WAN_SERDES_REGS
};

/* End of file WAN_SERDES.c */
