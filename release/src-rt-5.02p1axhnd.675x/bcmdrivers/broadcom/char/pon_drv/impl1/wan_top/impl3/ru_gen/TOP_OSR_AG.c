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
 * Field: TOP_OSR_CONTROL_RESERVED0
 ******************************************************************************/
const ru_field_rec TOP_OSR_CONTROL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    TOP_OSR_CONTROL_RESERVED0_FIELD_MASK,
    0,
    TOP_OSR_CONTROL_RESERVED0_FIELD_WIDTH,
    TOP_OSR_CONTROL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TOP_OSR_CONTROL_CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_TXLBE_SER_ORDER
 ******************************************************************************/
const ru_field_rec TOP_OSR_CONTROL_CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_TXLBE_SER_ORDER_FIELD =
{
    "CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_TXLBE_SER_ORDER",
#if RU_INCLUDE_DESC
    "",
    "Serializer direction.  0: LSB first (increasing).  1: MSB first"
    "(decreasing).",
#endif
    TOP_OSR_CONTROL_CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_TXLBE_SER_ORDER_FIELD_MASK,
    0,
    TOP_OSR_CONTROL_CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_TXLBE_SER_ORDER_FIELD_WIDTH,
    TOP_OSR_CONTROL_CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_TXLBE_SER_ORDER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TOP_OSR_CONTROL_CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_TXLBE_SER_INIT_VAL
 ******************************************************************************/
const ru_field_rec TOP_OSR_CONTROL_CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_TXLBE_SER_INIT_VAL_FIELD =
{
    "CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_TXLBE_SER_INIT_VAL",
#if RU_INCLUDE_DESC
    "",
    "Initial bit position for serializer.",
#endif
    TOP_OSR_CONTROL_CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_TXLBE_SER_INIT_VAL_FIELD_MASK,
    0,
    TOP_OSR_CONTROL_CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_TXLBE_SER_INIT_VAL_FIELD_WIDTH,
    TOP_OSR_CONTROL_CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_TXLBE_SER_INIT_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TOP_OSR_CONTROL_CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_TXLBE_SER_EN
 ******************************************************************************/
const ru_field_rec TOP_OSR_CONTROL_CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_TXLBE_SER_EN_FIELD =
{
    "CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_TXLBE_SER_EN",
#if RU_INCLUDE_DESC
    "",
    "LBE serializer enable.  0: parallel mode. 1: serial mode.",
#endif
    TOP_OSR_CONTROL_CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_TXLBE_SER_EN_FIELD_MASK,
    0,
    TOP_OSR_CONTROL_CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_TXLBE_SER_EN_FIELD_WIDTH,
    TOP_OSR_CONTROL_CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_TXLBE_SER_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TOP_OSR_CONTROL_CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_TXFIFO_RD_LEGACY_MODE
 ******************************************************************************/
const ru_field_rec TOP_OSR_CONTROL_CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_TXFIFO_RD_LEGACY_MODE_FIELD =
{
    "CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_TXFIFO_RD_LEGACY_MODE",
#if RU_INCLUDE_DESC
    "",
    "0: New oversample mode.  1: Legacy mode.",
#endif
    TOP_OSR_CONTROL_CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_TXFIFO_RD_LEGACY_MODE_FIELD_MASK,
    0,
    TOP_OSR_CONTROL_CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_TXFIFO_RD_LEGACY_MODE_FIELD_WIDTH,
    TOP_OSR_CONTROL_CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_TXFIFO_RD_LEGACY_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TOP_OSR_CONTROL_CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_CFG_GPON_RX_CLK
 ******************************************************************************/
const ru_field_rec TOP_OSR_CONTROL_CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_CFG_GPON_RX_CLK_FIELD =
{
    "CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_CFG_GPON_RX_CLK",
#if RU_INCLUDE_DESC
    "",
    "0: Select div2 clock.  1: Select div4 clock.  2: Select legacy mode"
    "clocking. 3: Reserved.",
#endif
    TOP_OSR_CONTROL_CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_CFG_GPON_RX_CLK_FIELD_MASK,
    0,
    TOP_OSR_CONTROL_CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_CFG_GPON_RX_CLK_FIELD_WIDTH,
    TOP_OSR_CONTROL_CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_CFG_GPON_RX_CLK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: TOP_OSR_CONTROL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TOP_OSR_CONTROL_FIELDS[] =
{
    &TOP_OSR_CONTROL_RESERVED0_FIELD,
    &TOP_OSR_CONTROL_CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_TXLBE_SER_ORDER_FIELD,
    &TOP_OSR_CONTROL_CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_TXLBE_SER_INIT_VAL_FIELD,
    &TOP_OSR_CONTROL_CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_TXLBE_SER_EN_FIELD,
    &TOP_OSR_CONTROL_CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_TXFIFO_RD_LEGACY_MODE_FIELD,
    &TOP_OSR_CONTROL_CR_WAN_TOP_WAN_MISC_SERDES_OVERSAMPLE_CTRL_CFG_GPON_RX_CLK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec TOP_OSR_CONTROL_REG = 
{
    "CONTROL",
#if RU_INCLUDE_DESC
    "WAN_TOP_OSR_CONTROL Register",
    "Register used to control the oversample mode of the SERDES gearboxes.",
#endif
    TOP_OSR_CONTROL_REG_OFFSET,
    0,
    0,
    33,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    TOP_OSR_CONTROL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: TOP_OSR
 ******************************************************************************/
static const ru_reg_rec *TOP_OSR_REGS[] =
{
    &TOP_OSR_CONTROL_REG,
};

unsigned long TOP_OSR_ADDRS[] =
{
#if defined(CONFIG_BCM963158)
    0x801440b4,
#else
    #error "TOP_OSR base address not defined"
#endif
};

const ru_block_rec TOP_OSR_BLOCK = 
{
    "TOP_OSR",
    TOP_OSR_ADDRS,
    1,
    1,
    TOP_OSR_REGS
};

/* End of file TOP_OSR.c */
