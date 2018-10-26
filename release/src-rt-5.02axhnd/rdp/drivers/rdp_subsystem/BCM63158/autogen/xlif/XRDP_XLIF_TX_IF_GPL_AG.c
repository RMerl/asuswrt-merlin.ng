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
 * Register: XLIF_TX_IF_IF_ENABLE
 ******************************************************************************/
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
    1127,
};

/******************************************************************************
 * Register: XLIF_TX_IF_READ_CREDITS
 ******************************************************************************/
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
    1128,
};

/******************************************************************************
 * Register: XLIF_TX_IF_SET_CREDITS
 ******************************************************************************/
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
    1129,
};

/******************************************************************************
 * Register: XLIF_TX_IF_OUT_CTRL
 ******************************************************************************/
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
    1130,
};

/******************************************************************************
 * Register: XLIF_TX_IF_URUN_PORT_ENABLE
 ******************************************************************************/
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
    1131,
};

/******************************************************************************
 * Register: XLIF_TX_IF_TX_THRESHOLD
 ******************************************************************************/
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
    1132,
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
    0x80147840,
    0x80147a40,
    0x80147c40,
    0x80147e40,
};

const ru_block_rec XLIF_TX_IF_BLOCK = 
{
    "XLIF_TX_IF",
    XLIF_TX_IF_ADDRS,
    4,
    6,
    XLIF_TX_IF_REGS
};

/* End of file XRDP_XLIF_TX_IF.c */
