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
 * Register: XLIF_RX_FLOW_CONTROL_COSMAP_EN
 ******************************************************************************/
const ru_reg_rec XLIF_RX_FLOW_CONTROL_COSMAP_EN_REG = 
{
    "COSMAP_EN",
#if RU_INCLUDE_DESC
    "COSMAP_EN Register",
    ".",
#endif
    XLIF_RX_FLOW_CONTROL_COSMAP_EN_REG_OFFSET,
    0,
    0,
    1122,
};

/******************************************************************************
 * Register: XLIF_RX_FLOW_CONTROL_COSMAP
 ******************************************************************************/
const ru_reg_rec XLIF_RX_FLOW_CONTROL_COSMAP_REG = 
{
    "COSMAP",
#if RU_INCLUDE_DESC
    "COSMAP Register",
    ".",
#endif
    XLIF_RX_FLOW_CONTROL_COSMAP_REG_OFFSET,
    0,
    0,
    1123,
};

/******************************************************************************
 * Block: XLIF_RX_FLOW_CONTROL
 ******************************************************************************/
static const ru_reg_rec *XLIF_RX_FLOW_CONTROL_REGS[] =
{
    &XLIF_RX_FLOW_CONTROL_COSMAP_EN_REG,
    &XLIF_RX_FLOW_CONTROL_COSMAP_REG,
};

unsigned long XLIF_RX_FLOW_CONTROL_ADDRS[] =
{
    0x80147820,
    0x80147a20,
    0x80147c20,
    0x80147e20,
};

const ru_block_rec XLIF_RX_FLOW_CONTROL_BLOCK = 
{
    "XLIF_RX_FLOW_CONTROL",
    XLIF_RX_FLOW_CONTROL_ADDRS,
    4,
    2,
    XLIF_RX_FLOW_CONTROL_REGS
};

/* End of file XRDP_XLIF_RX_FLOW_CONTROL.c */
