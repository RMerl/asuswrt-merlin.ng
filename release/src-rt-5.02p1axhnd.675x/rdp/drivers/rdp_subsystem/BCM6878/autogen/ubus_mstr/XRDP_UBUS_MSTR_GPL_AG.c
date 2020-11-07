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
 * Register: UBUS_MSTR_EN
 ******************************************************************************/
const ru_reg_rec UBUS_MSTR_EN_REG = 
{
    "EN",
#if RU_INCLUDE_DESC
    "BRDG_EN Register",
    "bridge enable",
#endif
    UBUS_MSTR_EN_REG_OFFSET,
    0,
    0,
    623,
};

/******************************************************************************
 * Register: UBUS_MSTR_HYST_CTRL
 ******************************************************************************/
const ru_reg_rec UBUS_MSTR_HYST_CTRL_REG = 
{
    "HYST_CTRL",
#if RU_INCLUDE_DESC
    "HYST_CTRL Register",
    "control the command / data queue full and empty indications.",
#endif
    UBUS_MSTR_HYST_CTRL_REG_OFFSET,
    0,
    0,
    624,
};

/******************************************************************************
 * Register: UBUS_MSTR_HP
 ******************************************************************************/
const ru_reg_rec UBUS_MSTR_HP_REG = 
{
    "HP",
#if RU_INCLUDE_DESC
    "HIGH_PRIORITY Register",
    "controls the high priority mechanism",
#endif
    UBUS_MSTR_HP_REG_OFFSET,
    0,
    0,
    625,
};

/******************************************************************************
 * Block: UBUS_MSTR
 ******************************************************************************/
static const ru_reg_rec *UBUS_MSTR_REGS[] =
{
    &UBUS_MSTR_EN_REG,
    &UBUS_MSTR_HYST_CTRL_REG,
    &UBUS_MSTR_HP_REG,
};

unsigned long UBUS_MSTR_ADDRS[] =
{
    0x82d96000,
};

const ru_block_rec UBUS_MSTR_BLOCK = 
{
    "UBUS_MSTR",
    UBUS_MSTR_ADDRS,
    1,
    3,
    UBUS_MSTR_REGS
};

/* End of file XRDP_UBUS_MSTR.c */
