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
 * Field: SERDES_STATUS_PLL_CTL_PLL0_REFIN_EN
 ******************************************************************************/
const ru_field_rec SERDES_STATUS_PLL_CTL_PLL0_REFIN_EN_FIELD =
{
    "PLL0_REFIN_EN",
#if RU_INCLUDE_DESC
    "PLL0_REFIN_EN",
    "Reference select. 0 - select pad_pll0_refclkp/n. 1 - select"
    "pll0_lcrefp/n."
    "Reset value is 0x0.",
#endif
    SERDES_STATUS_PLL_CTL_PLL0_REFIN_EN_FIELD_MASK,
    0,
    SERDES_STATUS_PLL_CTL_PLL0_REFIN_EN_FIELD_WIDTH,
    SERDES_STATUS_PLL_CTL_PLL0_REFIN_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SERDES_STATUS_PLL_CTL_PLL0_REFOUT_EN
 ******************************************************************************/
const ru_field_rec SERDES_STATUS_PLL_CTL_PLL0_REFOUT_EN_FIELD =
{
    "PLL0_REFOUT_EN",
#if RU_INCLUDE_DESC
    "PLL0_REFOUT_EN",
    "Enables SERDES to drive the pll0_refout pin. 0 - output is hiZ. 1"
    "- output is pad_pll0_refclk."
    "Reset value is 0x0.",
#endif
    SERDES_STATUS_PLL_CTL_PLL0_REFOUT_EN_FIELD_MASK,
    0,
    SERDES_STATUS_PLL_CTL_PLL0_REFOUT_EN_FIELD_WIDTH,
    SERDES_STATUS_PLL_CTL_PLL0_REFOUT_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SERDES_STATUS_PLL_CTL_PLL0_LCREF_SEL
 ******************************************************************************/
const ru_field_rec SERDES_STATUS_PLL_CTL_PLL0_LCREF_SEL_FIELD =
{
    "PLL0_LCREF_SEL",
#if RU_INCLUDE_DESC
    "PLL0_LCREF_SEL",
    "0 - select pll0_lcref. 1 - select pll1_lcref."
    "Reset value is 0x0.",
#endif
    SERDES_STATUS_PLL_CTL_PLL0_LCREF_SEL_FIELD_MASK,
    0,
    SERDES_STATUS_PLL_CTL_PLL0_LCREF_SEL_FIELD_WIDTH,
    SERDES_STATUS_PLL_CTL_PLL0_LCREF_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SERDES_STATUS_PLL_CTL_RESERVED0
 ******************************************************************************/
const ru_field_rec SERDES_STATUS_PLL_CTL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SERDES_STATUS_PLL_CTL_RESERVED0_FIELD_MASK,
    0,
    SERDES_STATUS_PLL_CTL_RESERVED0_FIELD_WIDTH,
    SERDES_STATUS_PLL_CTL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SERDES_STATUS_PLL_CTL_PLL1_REFIN_EN
 ******************************************************************************/
const ru_field_rec SERDES_STATUS_PLL_CTL_PLL1_REFIN_EN_FIELD =
{
    "PLL1_REFIN_EN",
#if RU_INCLUDE_DESC
    "PLL1_REFIN_EN",
    "Reference select. 0 - select pad_pll1_refclkp/n. 1 - select"
    "pll1_lcrefp/n."
    "Reset value is 0x0.",
#endif
    SERDES_STATUS_PLL_CTL_PLL1_REFIN_EN_FIELD_MASK,
    0,
    SERDES_STATUS_PLL_CTL_PLL1_REFIN_EN_FIELD_WIDTH,
    SERDES_STATUS_PLL_CTL_PLL1_REFIN_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SERDES_STATUS_PLL_CTL_PLL1_REFOUT_EN
 ******************************************************************************/
const ru_field_rec SERDES_STATUS_PLL_CTL_PLL1_REFOUT_EN_FIELD =
{
    "PLL1_REFOUT_EN",
#if RU_INCLUDE_DESC
    "PLL1_REFOUT_EN",
    "Enables SERDES to drive the pll1_refout pin. 0 - output is hiZ. 1"
    "- output is pad_pll1_refclk."
    "Reset value is 0x0.",
#endif
    SERDES_STATUS_PLL_CTL_PLL1_REFOUT_EN_FIELD_MASK,
    0,
    SERDES_STATUS_PLL_CTL_PLL1_REFOUT_EN_FIELD_WIDTH,
    SERDES_STATUS_PLL_CTL_PLL1_REFOUT_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SERDES_STATUS_PLL_CTL_PLL1_LCREF_SEL
 ******************************************************************************/
const ru_field_rec SERDES_STATUS_PLL_CTL_PLL1_LCREF_SEL_FIELD =
{
    "PLL1_LCREF_SEL",
#if RU_INCLUDE_DESC
    "PLL1_LCREF_SEL",
    "0 - select pll1_lcref. 1 - select pll0_lcref."
    "Reset value is 0x0.",
#endif
    SERDES_STATUS_PLL_CTL_PLL1_LCREF_SEL_FIELD_MASK,
    0,
    SERDES_STATUS_PLL_CTL_PLL1_LCREF_SEL_FIELD_WIDTH,
    SERDES_STATUS_PLL_CTL_PLL1_LCREF_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SERDES_STATUS_PLL_CTL_RESERVED1
 ******************************************************************************/
const ru_field_rec SERDES_STATUS_PLL_CTL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SERDES_STATUS_PLL_CTL_RESERVED1_FIELD_MASK,
    0,
    SERDES_STATUS_PLL_CTL_RESERVED1_FIELD_WIDTH,
    SERDES_STATUS_PLL_CTL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SERDES_STATUS_TEMP_CTL_WAN_TEMPERATURE_READ
 ******************************************************************************/
const ru_field_rec SERDES_STATUS_TEMP_CTL_WAN_TEMPERATURE_READ_FIELD =
{
    "WAN_TEMPERATURE_READ",
#if RU_INCLUDE_DESC
    "WAN_TEMPERATURE_READ",
    "10-bit temperature data. Please refer to TMON documentation for how"
    "to convert this value to a useful number."
#if defined (CONFIG_BCM96878)
    "Reset value is 0x318.",
#else
    "Reset value is 0x0.",
#endif
#endif
    SERDES_STATUS_TEMP_CTL_WAN_TEMPERATURE_READ_FIELD_MASK,
    0,
    SERDES_STATUS_TEMP_CTL_WAN_TEMPERATURE_READ_FIELD_WIDTH,
    SERDES_STATUS_TEMP_CTL_WAN_TEMPERATURE_READ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
#if defined (CONFIG_BCM96878)
    ru_access_rw
#else
    ru_access_read
#endif
#endif
};

#if defined (CONFIG_BCM96878)
/******************************************************************************
 * Field: SERDES_STATUS_TEMP_CTL_WAN_TEMPERATURE_STROBE
 ******************************************************************************/
const ru_field_rec SERDES_STATUS_TEMP_CTL_WAN_TEMPERATURE_STROBE_FIELD =
{
    "WAN_TEMPERATURE_STROBE",
#if RU_INCLUDE_DESC
    "WAN_TEMPERATURE_STROBE",
    "Reset value is 0x0.",
#endif
    SERDES_STATUS_TEMP_CTL_WAN_TEMPERATURE_STROBE_FIELD_MASK,
    0,
    SERDES_STATUS_TEMP_CTL_WAN_TEMPERATURE_STROBE_FIELD_WIDTH,
    SERDES_STATUS_TEMP_CTL_WAN_TEMPERATURE_STROBE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};
#endif

/******************************************************************************
 * Field: SERDES_STATUS_TEMP_CTL_RESERVED0
 ******************************************************************************/
const ru_field_rec SERDES_STATUS_TEMP_CTL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SERDES_STATUS_TEMP_CTL_RESERVED0_FIELD_MASK,
    0,
    SERDES_STATUS_TEMP_CTL_RESERVED0_FIELD_WIDTH,
    SERDES_STATUS_TEMP_CTL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: SERDES_STATUS_PRAM_CTL_PRAM_ADDRESS
 ******************************************************************************/
const ru_field_rec SERDES_STATUS_PRAM_CTL_PRAM_ADDRESS_FIELD =
{
    "PRAM_ADDRESS",
#if RU_INCLUDE_DESC
    "PRAM_ADDRESS",
    "Program RAM address."
    "Reset value is 0x0.",
#endif
    SERDES_STATUS_PRAM_CTL_PRAM_ADDRESS_FIELD_MASK,
    0,
    SERDES_STATUS_PRAM_CTL_PRAM_ADDRESS_FIELD_WIDTH,
    SERDES_STATUS_PRAM_CTL_PRAM_ADDRESS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SERDES_STATUS_PRAM_CTL_RESERVED0
 ******************************************************************************/
const ru_field_rec SERDES_STATUS_PRAM_CTL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SERDES_STATUS_PRAM_CTL_RESERVED0_FIELD_MASK,
    0,
    SERDES_STATUS_PRAM_CTL_RESERVED0_FIELD_WIDTH,
    SERDES_STATUS_PRAM_CTL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SERDES_STATUS_PRAM_CTL_PRAM_WE
 ******************************************************************************/
const ru_field_rec SERDES_STATUS_PRAM_CTL_PRAM_WE_FIELD =
{
    "PRAM_WE",
#if RU_INCLUDE_DESC
    "PRAM_WE",
    "Program RAM write strobe."
    "Reset value is 0x0.",
#endif
    SERDES_STATUS_PRAM_CTL_PRAM_WE_FIELD_MASK,
    0,
    SERDES_STATUS_PRAM_CTL_PRAM_WE_FIELD_WIDTH,
    SERDES_STATUS_PRAM_CTL_PRAM_WE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SERDES_STATUS_PRAM_CTL_RESERVED1
 ******************************************************************************/
const ru_field_rec SERDES_STATUS_PRAM_CTL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SERDES_STATUS_PRAM_CTL_RESERVED1_FIELD_MASK,
    0,
    SERDES_STATUS_PRAM_CTL_RESERVED1_FIELD_WIDTH,
    SERDES_STATUS_PRAM_CTL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SERDES_STATUS_PRAM_CTL_PRAM_GO
 ******************************************************************************/
const ru_field_rec SERDES_STATUS_PRAM_CTL_PRAM_GO_FIELD =
{
    "PRAM_GO",
#if RU_INCLUDE_DESC
    "PRAM_GO",
    "Perform pRAM operation.  This field is only valid for the B0 or beyond.  Software sets and hardware clears this bit.  Do not write to this register if this bit is set.",
#endif
    SERDES_STATUS_PRAM_CTL_PRAM_GO_FIELD_MASK,
    0,
    SERDES_STATUS_PRAM_CTL_PRAM_GO_FIELD_WIDTH,
    SERDES_STATUS_PRAM_CTL_PRAM_GO_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SERDES_STATUS_PRAM_VAL_LOW_VAL
 ******************************************************************************/
const ru_field_rec SERDES_STATUS_PRAM_VAL_LOW_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "VAL",
    "Data",
#endif
    SERDES_STATUS_PRAM_VAL_LOW_VAL_FIELD_MASK,
    0,
    SERDES_STATUS_PRAM_VAL_LOW_VAL_FIELD_WIDTH,
    SERDES_STATUS_PRAM_VAL_LOW_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SERDES_STATUS_PRAM_VAL_HIGH_VAL
 ******************************************************************************/
const ru_field_rec SERDES_STATUS_PRAM_VAL_HIGH_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "VAL",
    "Data",
#endif
    SERDES_STATUS_PRAM_VAL_HIGH_VAL_FIELD_MASK,
    0,
    SERDES_STATUS_PRAM_VAL_HIGH_VAL_FIELD_WIDTH,
    SERDES_STATUS_PRAM_VAL_HIGH_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SERDES_STATUS_OVERSAMPLE_CTRL_CFG_GPON_RX_CLK
 ******************************************************************************/
const ru_field_rec SERDES_STATUS_OVERSAMPLE_CTRL_CFG_GPON_RX_CLK_FIELD =
{
    "CFG_GPON_RX_CLK",
#if RU_INCLUDE_DESC
    "CFG_GPON_RX_CLK",
    "0: Selects divide-by-2 clock divider for mac_tx_clk. 1: Selects"
    "divide-by-4 clock divider for mac_tx_clk. 2: Selects divide-by-1"
    "clock divider. 3: Unused.",
#endif
    SERDES_STATUS_OVERSAMPLE_CTRL_CFG_GPON_RX_CLK_FIELD_MASK,
    0,
    SERDES_STATUS_OVERSAMPLE_CTRL_CFG_GPON_RX_CLK_FIELD_WIDTH,
    SERDES_STATUS_OVERSAMPLE_CTRL_CFG_GPON_RX_CLK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SERDES_STATUS_OVERSAMPLE_CTRL_TXFIFO_RD_LEGACY_MODE
 ******************************************************************************/
const ru_field_rec SERDES_STATUS_OVERSAMPLE_CTRL_TXFIFO_RD_LEGACY_MODE_FIELD =
{
    "TXFIFO_RD_LEGACY_MODE",
#if RU_INCLUDE_DESC
    "TXFIFO_RD_LEGACY_MODE",
    "GPON gearbox TX path FIFO legacy (subrate) mode",
#endif
    SERDES_STATUS_OVERSAMPLE_CTRL_TXFIFO_RD_LEGACY_MODE_FIELD_MASK,
    0,
    SERDES_STATUS_OVERSAMPLE_CTRL_TXFIFO_RD_LEGACY_MODE_FIELD_WIDTH,
    SERDES_STATUS_OVERSAMPLE_CTRL_TXFIFO_RD_LEGACY_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SERDES_STATUS_OVERSAMPLE_CTRL_TXLBE_SER_EN
 ******************************************************************************/
const ru_field_rec SERDES_STATUS_OVERSAMPLE_CTRL_TXLBE_SER_EN_FIELD =
{
    "TXLBE_SER_EN",
#if RU_INCLUDE_DESC
    "TXLBE_SER_EN",
    "LBE serializer enable"
    ""
    "0 = parallel mode"
    "1 = serial mode",
#endif
    SERDES_STATUS_OVERSAMPLE_CTRL_TXLBE_SER_EN_FIELD_MASK,
    0,
    SERDES_STATUS_OVERSAMPLE_CTRL_TXLBE_SER_EN_FIELD_WIDTH,
    SERDES_STATUS_OVERSAMPLE_CTRL_TXLBE_SER_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SERDES_STATUS_OVERSAMPLE_CTRL_TXLBE_SER_INIT_VAL
 ******************************************************************************/
const ru_field_rec SERDES_STATUS_OVERSAMPLE_CTRL_TXLBE_SER_INIT_VAL_FIELD =
{
    "TXLBE_SER_INIT_VAL",
#if RU_INCLUDE_DESC
    "TXLBE_SER_INIT_VAL",
    "Initial bit position for serializer",
#endif
    SERDES_STATUS_OVERSAMPLE_CTRL_TXLBE_SER_INIT_VAL_FIELD_MASK,
    0,
    SERDES_STATUS_OVERSAMPLE_CTRL_TXLBE_SER_INIT_VAL_FIELD_WIDTH,
    SERDES_STATUS_OVERSAMPLE_CTRL_TXLBE_SER_INIT_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SERDES_STATUS_OVERSAMPLE_CTRL_TXLBE_SER_ORDER
 ******************************************************************************/
const ru_field_rec SERDES_STATUS_OVERSAMPLE_CTRL_TXLBE_SER_ORDER_FIELD =
{
    "TXLBE_SER_ORDER",
#if RU_INCLUDE_DESC
    "TXLBE_SER_ORDER",
    "Serializer direction"
    ""
    "0 = LSB first (increasing)"
    "1 = MSB first (decreasing)",
#endif
    SERDES_STATUS_OVERSAMPLE_CTRL_TXLBE_SER_ORDER_FIELD_MASK,
    0,
    SERDES_STATUS_OVERSAMPLE_CTRL_TXLBE_SER_ORDER_FIELD_WIDTH,
    SERDES_STATUS_OVERSAMPLE_CTRL_TXLBE_SER_ORDER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SERDES_STATUS_OVERSAMPLE_CTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec SERDES_STATUS_OVERSAMPLE_CTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    SERDES_STATUS_OVERSAMPLE_CTRL_RESERVED0_FIELD_MASK,
    0,
    SERDES_STATUS_OVERSAMPLE_CTRL_RESERVED0_FIELD_WIDTH,
    SERDES_STATUS_OVERSAMPLE_CTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: SERDES_STATUS_PLL_CTL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SERDES_STATUS_PLL_CTL_FIELDS[] =
{
    &SERDES_STATUS_PLL_CTL_PLL0_REFIN_EN_FIELD,
    &SERDES_STATUS_PLL_CTL_PLL0_REFOUT_EN_FIELD,
    &SERDES_STATUS_PLL_CTL_PLL0_LCREF_SEL_FIELD,
    &SERDES_STATUS_PLL_CTL_RESERVED0_FIELD,
    &SERDES_STATUS_PLL_CTL_PLL1_REFIN_EN_FIELD,
    &SERDES_STATUS_PLL_CTL_PLL1_REFOUT_EN_FIELD,
    &SERDES_STATUS_PLL_CTL_PLL1_LCREF_SEL_FIELD,
    &SERDES_STATUS_PLL_CTL_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SERDES_STATUS_PLL_CTL_REG = 
{
    "PLL_CTL",
#if RU_INCLUDE_DESC
    "SERDES_PLL_CTL Register",
    "Register used for low configuration of PLL clocks.",
#endif
    SERDES_STATUS_PLL_CTL_REG_OFFSET,
    0,
    0,
    32,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    SERDES_STATUS_PLL_CTL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: SERDES_STATUS_TEMP_CTL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SERDES_STATUS_TEMP_CTL_FIELDS[] =
{
    &SERDES_STATUS_TEMP_CTL_WAN_TEMPERATURE_READ_FIELD,
#if defined (CONFIG_BCM96878)
    &SERDES_STATUS_TEMP_CTL_WAN_TEMPERATURE_STROBE_FIELD,
#endif
    &SERDES_STATUS_TEMP_CTL_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SERDES_STATUS_TEMP_CTL_REG = 
{
    "TEMP_CTL",
#if RU_INCLUDE_DESC
    "SERDES_TEMP_CTL Register",
    "Register used for temperature read.",
#endif
    SERDES_STATUS_TEMP_CTL_REG_OFFSET,
    0,
    0,
    33,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
#if defined (CONFIG_BCM96878)
    3,
#else
    2,
#endif
    SERDES_STATUS_TEMP_CTL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: SERDES_STATUS_PRAM_CTL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SERDES_STATUS_PRAM_CTL_FIELDS[] =
{
    &SERDES_STATUS_PRAM_CTL_PRAM_ADDRESS_FIELD,
    &SERDES_STATUS_PRAM_CTL_RESERVED0_FIELD,
    &SERDES_STATUS_PRAM_CTL_PRAM_WE_FIELD,
    &SERDES_STATUS_PRAM_CTL_RESERVED1_FIELD,
    &SERDES_STATUS_PRAM_CTL_PRAM_GO_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SERDES_STATUS_PRAM_CTL_REG = 
{
    "PRAM_CTL",
#if RU_INCLUDE_DESC
    "SERDES_PRAM_CTL Register",
    "Register used for PRAM control.",
#endif
    SERDES_STATUS_PRAM_CTL_REG_OFFSET,
    0,
    0,
    34,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    SERDES_STATUS_PRAM_CTL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: SERDES_STATUS_PRAM_VAL_LOW
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SERDES_STATUS_PRAM_VAL_LOW_FIELDS[] =
{
    &SERDES_STATUS_PRAM_VAL_LOW_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SERDES_STATUS_PRAM_VAL_LOW_REG = 
{
    "PRAM_VAL_LOW",
#if RU_INCLUDE_DESC
    "SERDES_PRAM_VAL_LOW Register",
    "Bits [31:0] of the 64-bit pRAM write data interface.",
#endif
    SERDES_STATUS_PRAM_VAL_LOW_REG_OFFSET,
    0,
    0,
    35,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    SERDES_STATUS_PRAM_VAL_LOW_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: SERDES_STATUS_PRAM_VAL_HIGH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SERDES_STATUS_PRAM_VAL_HIGH_FIELDS[] =
{
    &SERDES_STATUS_PRAM_VAL_HIGH_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SERDES_STATUS_PRAM_VAL_HIGH_REG = 
{
    "PRAM_VAL_HIGH",
#if RU_INCLUDE_DESC
    "SERDES_PRAM_VAL_HIGH Register",
    "Bits [63:32] of the 64-bit pRAM write data interface.",
#endif
    SERDES_STATUS_PRAM_VAL_HIGH_REG_OFFSET,
    0,
    0,
    36,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    SERDES_STATUS_PRAM_VAL_HIGH_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: SERDES_STATUS_OVERSAMPLE_CTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SERDES_STATUS_OVERSAMPLE_CTRL_FIELDS[] =
{
    &SERDES_STATUS_OVERSAMPLE_CTRL_CFG_GPON_RX_CLK_FIELD,
    &SERDES_STATUS_OVERSAMPLE_CTRL_TXFIFO_RD_LEGACY_MODE_FIELD,
    &SERDES_STATUS_OVERSAMPLE_CTRL_TXLBE_SER_EN_FIELD,
    &SERDES_STATUS_OVERSAMPLE_CTRL_TXLBE_SER_INIT_VAL_FIELD,
    &SERDES_STATUS_OVERSAMPLE_CTRL_TXLBE_SER_ORDER_FIELD,
    &SERDES_STATUS_OVERSAMPLE_CTRL_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SERDES_STATUS_OVERSAMPLE_CTRL_REG = 
{
    "OVERSAMPLE_CTRL",
#if RU_INCLUDE_DESC
    "SERDES_OVERSAMPLE_CTRL Register",
    "Control over SERDES oversample features",
#endif
    SERDES_STATUS_OVERSAMPLE_CTRL_REG_OFFSET,
    0,
    0,
    37,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    SERDES_STATUS_OVERSAMPLE_CTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: SERDES_STATUS
 ******************************************************************************/
static const ru_reg_rec *SERDES_STATUS_REGS[] =
{
    &SERDES_STATUS_PLL_CTL_REG,
    &SERDES_STATUS_TEMP_CTL_REG,
    &SERDES_STATUS_PRAM_CTL_REG,
    &SERDES_STATUS_PRAM_VAL_LOW_REG,
    &SERDES_STATUS_PRAM_VAL_HIGH_REG,
    &SERDES_STATUS_OVERSAMPLE_CTRL_REG,
};

unsigned long SERDES_STATUS_ADDRS[] =
{
    0x82db2050,
};

const ru_block_rec SERDES_STATUS_BLOCK = 
{
    "SERDES_STATUS",
    SERDES_STATUS_ADDRS,
    1,
    6,
    SERDES_STATUS_REGS
};

/* End of file SERDES_STATUS.c */
