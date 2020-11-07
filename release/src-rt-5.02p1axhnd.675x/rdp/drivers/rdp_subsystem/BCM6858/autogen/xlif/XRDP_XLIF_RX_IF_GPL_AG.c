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
 * Register: XLIF_RX_IF_IF_DIS
 ******************************************************************************/
const ru_reg_rec XLIF_RX_IF_IF_DIS_REG = 
{
    "IF_DIS",
#if RU_INCLUDE_DESC
    "INTERFACE_DISABLE Register",
    "Interface_Disable",
#endif
    XLIF_RX_IF_IF_DIS_REG_OFFSET,
    0,
    0,
    600,
};

/******************************************************************************
 * Register: XLIF_RX_IF_OFLW_FLAG
 ******************************************************************************/
const ru_reg_rec XLIF_RX_IF_OFLW_FLAG_REG = 
{
    "OFLW_FLAG",
#if RU_INCLUDE_DESC
    "OVRFLOW_FLAG Register",
    "Indicate an overflow event (data valid while FIFO is full)."
    "read clear",
#endif
    XLIF_RX_IF_OFLW_FLAG_REG_OFFSET,
    0,
    0,
    601,
};

/******************************************************************************
 * Register: XLIF_RX_IF_ERR_FLAG
 ******************************************************************************/
const ru_reg_rec XLIF_RX_IF_ERR_FLAG_REG = 
{
    "ERR_FLAG",
#if RU_INCLUDE_DESC
    "PROTOCOL_ERR Register",
    "Indicate RX protocol Error."
    "read clear",
#endif
    XLIF_RX_IF_ERR_FLAG_REG_OFFSET,
    0,
    0,
    602,
};

/******************************************************************************
 * Block: XLIF_RX_IF
 ******************************************************************************/
static const ru_reg_rec *XLIF_RX_IF_REGS[] =
{
    &XLIF_RX_IF_IF_DIS_REG,
    &XLIF_RX_IF_OFLW_FLAG_REG,
    &XLIF_RX_IF_ERR_FLAG_REG,
};

unsigned long XLIF_RX_IF_ADDRS[] =
{
    0x82d2b000,
    0x82d2b200,
    0x82d2b400,
    0x82d2b600,
    0x82d2b800,
    0x82d2ba00,
    0x82d2bc00,
    0x82d2be00,
};

const ru_block_rec XLIF_RX_IF_BLOCK = 
{
    "XLIF_RX_IF",
    XLIF_RX_IF_ADDRS,
    8,
    3,
    XLIF_RX_IF_REGS
};

/* End of file XRDP_XLIF_RX_IF.c */
