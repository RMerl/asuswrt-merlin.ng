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

#if RU_INCLUDE_FIELD_DB
/******************************************************************************
 * Field: XLIF_TX_IF_IF_ENABLE_DISABLE_WITH_CREDITS
 ******************************************************************************/
const ru_field_rec XLIF_TX_IF_IF_ENABLE_DISABLE_WITH_CREDITS_FIELD =
{
    "DISABLE_WITH_CREDITS",
#if RU_INCLUDE_DESC
    "Disable_With_Credits",
    "Disable_With_Credits",
#endif
    XLIF_TX_IF_IF_ENABLE_DISABLE_WITH_CREDITS_FIELD_MASK,
    0,
    XLIF_TX_IF_IF_ENABLE_DISABLE_WITH_CREDITS_FIELD_WIDTH,
    XLIF_TX_IF_IF_ENABLE_DISABLE_WITH_CREDITS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XLIF_TX_IF_IF_ENABLE_DISABLE_WO_CREDITS
 ******************************************************************************/
const ru_field_rec XLIF_TX_IF_IF_ENABLE_DISABLE_WO_CREDITS_FIELD =
{
    "DISABLE_WO_CREDITS",
#if RU_INCLUDE_DESC
    "Disable_WO_Credits",
    "Disable_WO_Credits",
#endif
    XLIF_TX_IF_IF_ENABLE_DISABLE_WO_CREDITS_FIELD_MASK,
    0,
    XLIF_TX_IF_IF_ENABLE_DISABLE_WO_CREDITS_FIELD_WIDTH,
    XLIF_TX_IF_IF_ENABLE_DISABLE_WO_CREDITS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XLIF_TX_IF_IF_ENABLE_RESERVED0
 ******************************************************************************/
const ru_field_rec XLIF_TX_IF_IF_ENABLE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XLIF_TX_IF_IF_ENABLE_RESERVED0_FIELD_MASK,
    0,
    XLIF_TX_IF_IF_ENABLE_RESERVED0_FIELD_WIDTH,
    XLIF_TX_IF_IF_ENABLE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XLIF_TX_IF_READ_CREDITS_VALUE
 ******************************************************************************/
const ru_field_rec XLIF_TX_IF_READ_CREDITS_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "Value",
    "Value",
#endif
    XLIF_TX_IF_READ_CREDITS_VALUE_FIELD_MASK,
    0,
    XLIF_TX_IF_READ_CREDITS_VALUE_FIELD_WIDTH,
    XLIF_TX_IF_READ_CREDITS_VALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XLIF_TX_IF_READ_CREDITS_RESERVED0
 ******************************************************************************/
const ru_field_rec XLIF_TX_IF_READ_CREDITS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XLIF_TX_IF_READ_CREDITS_RESERVED0_FIELD_MASK,
    0,
    XLIF_TX_IF_READ_CREDITS_RESERVED0_FIELD_WIDTH,
    XLIF_TX_IF_READ_CREDITS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XLIF_TX_IF_SET_CREDITS_VALUE
 ******************************************************************************/
const ru_field_rec XLIF_TX_IF_SET_CREDITS_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "Value",
    "Value",
#endif
    XLIF_TX_IF_SET_CREDITS_VALUE_FIELD_MASK,
    0,
    XLIF_TX_IF_SET_CREDITS_VALUE_FIELD_WIDTH,
    XLIF_TX_IF_SET_CREDITS_VALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XLIF_TX_IF_SET_CREDITS_RESERVED0
 ******************************************************************************/
const ru_field_rec XLIF_TX_IF_SET_CREDITS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XLIF_TX_IF_SET_CREDITS_RESERVED0_FIELD_MASK,
    0,
    XLIF_TX_IF_SET_CREDITS_RESERVED0_FIELD_WIDTH,
    XLIF_TX_IF_SET_CREDITS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XLIF_TX_IF_SET_CREDITS_EN
 ******************************************************************************/
const ru_field_rec XLIF_TX_IF_SET_CREDITS_EN_FIELD =
{
    "EN",
#if RU_INCLUDE_DESC
    "enable",
    "enable",
#endif
    XLIF_TX_IF_SET_CREDITS_EN_FIELD_MASK,
    0,
    XLIF_TX_IF_SET_CREDITS_EN_FIELD_WIDTH,
    XLIF_TX_IF_SET_CREDITS_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XLIF_TX_IF_SET_CREDITS_RESERVED1
 ******************************************************************************/
const ru_field_rec XLIF_TX_IF_SET_CREDITS_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XLIF_TX_IF_SET_CREDITS_RESERVED1_FIELD_MASK,
    0,
    XLIF_TX_IF_SET_CREDITS_RESERVED1_FIELD_WIDTH,
    XLIF_TX_IF_SET_CREDITS_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XLIF_TX_IF_OUT_CTRL_MAC_TXERR
 ******************************************************************************/
const ru_field_rec XLIF_TX_IF_OUT_CTRL_MAC_TXERR_FIELD =
{
    "MAC_TXERR",
#if RU_INCLUDE_DESC
    "mac_txerr",
    "mac_txerr",
#endif
    XLIF_TX_IF_OUT_CTRL_MAC_TXERR_FIELD_MASK,
    0,
    XLIF_TX_IF_OUT_CTRL_MAC_TXERR_FIELD_WIDTH,
    XLIF_TX_IF_OUT_CTRL_MAC_TXERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XLIF_TX_IF_OUT_CTRL_MAC_TXCRCERR
 ******************************************************************************/
const ru_field_rec XLIF_TX_IF_OUT_CTRL_MAC_TXCRCERR_FIELD =
{
    "MAC_TXCRCERR",
#if RU_INCLUDE_DESC
    "mac_txcrcerr",
    "mac_txcrcerr",
#endif
    XLIF_TX_IF_OUT_CTRL_MAC_TXCRCERR_FIELD_MASK,
    0,
    XLIF_TX_IF_OUT_CTRL_MAC_TXCRCERR_FIELD_WIDTH,
    XLIF_TX_IF_OUT_CTRL_MAC_TXCRCERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XLIF_TX_IF_OUT_CTRL_MAC_TXOSTS_SINEXT
 ******************************************************************************/
const ru_field_rec XLIF_TX_IF_OUT_CTRL_MAC_TXOSTS_SINEXT_FIELD =
{
    "MAC_TXOSTS_SINEXT",
#if RU_INCLUDE_DESC
    "mac_txosts_sinext",
    "mac_txosts_sinext",
#endif
    XLIF_TX_IF_OUT_CTRL_MAC_TXOSTS_SINEXT_FIELD_MASK,
    0,
    XLIF_TX_IF_OUT_CTRL_MAC_TXOSTS_SINEXT_FIELD_WIDTH,
    XLIF_TX_IF_OUT_CTRL_MAC_TXOSTS_SINEXT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XLIF_TX_IF_OUT_CTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec XLIF_TX_IF_OUT_CTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XLIF_TX_IF_OUT_CTRL_RESERVED0_FIELD_MASK,
    0,
    XLIF_TX_IF_OUT_CTRL_RESERVED0_FIELD_WIDTH,
    XLIF_TX_IF_OUT_CTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XLIF_TX_IF_OUT_CTRL_MAC_TXCRCMODE
 ******************************************************************************/
const ru_field_rec XLIF_TX_IF_OUT_CTRL_MAC_TXCRCMODE_FIELD =
{
    "MAC_TXCRCMODE",
#if RU_INCLUDE_DESC
    "mac_txcrcmode",
    "mac_txcrcmode",
#endif
    XLIF_TX_IF_OUT_CTRL_MAC_TXCRCMODE_FIELD_MASK,
    0,
    XLIF_TX_IF_OUT_CTRL_MAC_TXCRCMODE_FIELD_WIDTH,
    XLIF_TX_IF_OUT_CTRL_MAC_TXCRCMODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XLIF_TX_IF_OUT_CTRL_RESERVED1
 ******************************************************************************/
const ru_field_rec XLIF_TX_IF_OUT_CTRL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XLIF_TX_IF_OUT_CTRL_RESERVED1_FIELD_MASK,
    0,
    XLIF_TX_IF_OUT_CTRL_RESERVED1_FIELD_WIDTH,
    XLIF_TX_IF_OUT_CTRL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XLIF_TX_IF_URUN_PORT_ENABLE_ENABLE
 ******************************************************************************/
const ru_field_rec XLIF_TX_IF_URUN_PORT_ENABLE_ENABLE_FIELD =
{
    "ENABLE",
#if RU_INCLUDE_DESC
    "Enable",
    "Enable",
#endif
    XLIF_TX_IF_URUN_PORT_ENABLE_ENABLE_FIELD_MASK,
    0,
    XLIF_TX_IF_URUN_PORT_ENABLE_ENABLE_FIELD_WIDTH,
    XLIF_TX_IF_URUN_PORT_ENABLE_ENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XLIF_TX_IF_URUN_PORT_ENABLE_RESERVED0
 ******************************************************************************/
const ru_field_rec XLIF_TX_IF_URUN_PORT_ENABLE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XLIF_TX_IF_URUN_PORT_ENABLE_RESERVED0_FIELD_MASK,
    0,
    XLIF_TX_IF_URUN_PORT_ENABLE_RESERVED0_FIELD_WIDTH,
    XLIF_TX_IF_URUN_PORT_ENABLE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XLIF_TX_IF_TX_THRESHOLD_VALUE
 ******************************************************************************/
const ru_field_rec XLIF_TX_IF_TX_THRESHOLD_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "Value",
    "Value",
#endif
    XLIF_TX_IF_TX_THRESHOLD_VALUE_FIELD_MASK,
    0,
    XLIF_TX_IF_TX_THRESHOLD_VALUE_FIELD_WIDTH,
    XLIF_TX_IF_TX_THRESHOLD_VALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XLIF_TX_IF_TX_THRESHOLD_RESERVED0
 ******************************************************************************/
const ru_field_rec XLIF_TX_IF_TX_THRESHOLD_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XLIF_TX_IF_TX_THRESHOLD_RESERVED0_FIELD_MASK,
    0,
    XLIF_TX_IF_TX_THRESHOLD_RESERVED0_FIELD_WIDTH,
    XLIF_TX_IF_TX_THRESHOLD_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: XLIF_TX_IF_IF_ENABLE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XLIF_TX_IF_IF_ENABLE_FIELDS[] =
{
    &XLIF_TX_IF_IF_ENABLE_DISABLE_WITH_CREDITS_FIELD,
    &XLIF_TX_IF_IF_ENABLE_DISABLE_WO_CREDITS_FIELD,
    &XLIF_TX_IF_IF_ENABLE_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XLIF_TX_IF_IF_ENABLE_REG = 
{
    "IF_ENABLE",
#if RU_INCLUDE_DESC
    "INTERFACE_ENABLE Register",
    "Interface_Enable",
#endif
    XLIF_TX_IF_IF_ENABLE_REG_OFFSET,
    0,
    0,
    605,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    XLIF_TX_IF_IF_ENABLE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: XLIF_TX_IF_READ_CREDITS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XLIF_TX_IF_READ_CREDITS_FIELDS[] =
{
    &XLIF_TX_IF_READ_CREDITS_VALUE_FIELD,
    &XLIF_TX_IF_READ_CREDITS_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XLIF_TX_IF_READ_CREDITS_REG = 
{
    "READ_CREDITS",
#if RU_INCLUDE_DESC
    "READ_CREDITS Register",
    "Read_Credits",
#endif
    XLIF_TX_IF_READ_CREDITS_REG_OFFSET,
    0,
    0,
    606,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XLIF_TX_IF_READ_CREDITS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: XLIF_TX_IF_SET_CREDITS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XLIF_TX_IF_SET_CREDITS_FIELDS[] =
{
    &XLIF_TX_IF_SET_CREDITS_VALUE_FIELD,
    &XLIF_TX_IF_SET_CREDITS_RESERVED0_FIELD,
    &XLIF_TX_IF_SET_CREDITS_EN_FIELD,
    &XLIF_TX_IF_SET_CREDITS_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XLIF_TX_IF_SET_CREDITS_REG = 
{
    "SET_CREDITS",
#if RU_INCLUDE_DESC
    "SET_CREDITS Register",
    "Set_Credits"
    "The enable bit and the new value can be set together. Then, the enable bit must be turned off, while the new value remain stable.",
#endif
    XLIF_TX_IF_SET_CREDITS_REG_OFFSET,
    0,
    0,
    607,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    XLIF_TX_IF_SET_CREDITS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: XLIF_TX_IF_OUT_CTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XLIF_TX_IF_OUT_CTRL_FIELDS[] =
{
    &XLIF_TX_IF_OUT_CTRL_MAC_TXERR_FIELD,
    &XLIF_TX_IF_OUT_CTRL_MAC_TXCRCERR_FIELD,
    &XLIF_TX_IF_OUT_CTRL_MAC_TXOSTS_SINEXT_FIELD,
    &XLIF_TX_IF_OUT_CTRL_RESERVED0_FIELD,
    &XLIF_TX_IF_OUT_CTRL_MAC_TXCRCMODE_FIELD,
    &XLIF_TX_IF_OUT_CTRL_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XLIF_TX_IF_OUT_CTRL_REG = 
{
    "OUT_CTRL",
#if RU_INCLUDE_DESC
    "OUTPUTS_CONTROL Register",
    "Control the values of several output signals on the XRDP -> XLMAC interface.",
#endif
    XLIF_TX_IF_OUT_CTRL_REG_OFFSET,
    0,
    0,
    608,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    XLIF_TX_IF_OUT_CTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: XLIF_TX_IF_URUN_PORT_ENABLE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XLIF_TX_IF_URUN_PORT_ENABLE_FIELDS[] =
{
    &XLIF_TX_IF_URUN_PORT_ENABLE_ENABLE_FIELD,
    &XLIF_TX_IF_URUN_PORT_ENABLE_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XLIF_TX_IF_URUN_PORT_ENABLE_REG = 
{
    "URUN_PORT_ENABLE",
#if RU_INCLUDE_DESC
    "UNDERRUN_PROTECTION_ENABLE Register",
    "Underrun_Protection_Enable",
#endif
    XLIF_TX_IF_URUN_PORT_ENABLE_REG_OFFSET,
    0,
    0,
    609,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XLIF_TX_IF_URUN_PORT_ENABLE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: XLIF_TX_IF_TX_THRESHOLD
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XLIF_TX_IF_TX_THRESHOLD_FIELDS[] =
{
    &XLIF_TX_IF_TX_THRESHOLD_VALUE_FIELD,
    &XLIF_TX_IF_TX_THRESHOLD_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XLIF_TX_IF_TX_THRESHOLD_REG = 
{
    "TX_THRESHOLD",
#if RU_INCLUDE_DESC
    "TX_THRESHOLD Register",
    "TX threshold for the TX CDC FIFO in units of 128 bit."
    "The TX CDC FIFO is depth is 16 entries."
    "",
#endif
    XLIF_TX_IF_TX_THRESHOLD_REG_OFFSET,
    0,
    0,
    610,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XLIF_TX_IF_TX_THRESHOLD_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Block: XLIF_TX_IF
 ******************************************************************************/
static const ru_reg_rec *XLIF_TX_IF_REGS[] =
{
    &XLIF_TX_IF_IF_ENABLE_REG,
    &XLIF_TX_IF_READ_CREDITS_REG,
    &XLIF_TX_IF_SET_CREDITS_REG,
    &XLIF_TX_IF_OUT_CTRL_REG,
    &XLIF_TX_IF_URUN_PORT_ENABLE_REG,
    &XLIF_TX_IF_TX_THRESHOLD_REG,
};

unsigned long XLIF_TX_IF_ADDRS[] =
{
    0x82d2b040,
    0x82d2b240,
    0x82d2b440,
    0x82d2b640,
    0x82d2b840,
    0x82d2ba40,
    0x82d2bc40,
    0x82d2be40,
};

const ru_block_rec XLIF_TX_IF_BLOCK = 
{
    "XLIF_TX_IF",
    XLIF_TX_IF_ADDRS,
    8,
    6,
    XLIF_TX_IF_REGS
};

/* End of file XRDP_XLIF_TX_IF.c */
