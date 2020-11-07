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
 * Field: SYNCE_PLL_CONFIG_IG_CFG_SYNCE_PLL_NDIV_INT
 ******************************************************************************/
const ru_field_rec SYNCE_PLL_CONFIG_IG_CFG_SYNCE_PLL_NDIV_INT_FIELD =
{
    "CFG_SYNCE_PLL_NDIV_INT",
#if RU_INCLUDE_DESC
    "",
    "Integer divider.",
#endif
    SYNCE_PLL_CONFIG_IG_CFG_SYNCE_PLL_NDIV_INT_FIELD_MASK,
    0,
    SYNCE_PLL_CONFIG_IG_CFG_SYNCE_PLL_NDIV_INT_FIELD_WIDTH,
    SYNCE_PLL_CONFIG_IG_CFG_SYNCE_PLL_NDIV_INT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: SYNCE_PLL_CONFIG_IG_CFG_SYNCE_PLL_NDIV_FRAC
 ******************************************************************************/
const ru_field_rec SYNCE_PLL_CONFIG_IG_CFG_SYNCE_PLL_NDIV_FRAC_FIELD =
{
    "CFG_SYNCE_PLL_NDIV_FRAC",
#if RU_INCLUDE_DESC
    "",
    "Fractional divider.",
#endif
    SYNCE_PLL_CONFIG_IG_CFG_SYNCE_PLL_NDIV_FRAC_FIELD_MASK,
    0,
    SYNCE_PLL_CONFIG_IG_CFG_SYNCE_PLL_NDIV_FRAC_FIELD_WIDTH,
    SYNCE_PLL_CONFIG_IG_CFG_SYNCE_PLL_NDIV_FRAC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: SYNCE_PLL_CONFIG_IG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *SYNCE_PLL_CONFIG_IG_FIELDS[] =
{
    &SYNCE_PLL_CONFIG_IG_CFG_SYNCE_PLL_NDIV_INT_FIELD,
    &SYNCE_PLL_CONFIG_IG_CFG_SYNCE_PLL_NDIV_FRAC_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec SYNCE_PLL_CONFIG_IG_REG = 
{
    "IG",
#if RU_INCLUDE_DESC
    "WAN_SYNCE_PLL_CONFIG Register",
    "Specifies the SyncE_PLL integer/fractional dividers, applicable when"
    "cfg_en_pbi_wr_2_syncE_pll is set.",
#endif
    SYNCE_PLL_CONFIG_IG_REG_OFFSET,
    0,
    0,
    42,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    SYNCE_PLL_CONFIG_IG_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: SYNCE_PLL_CONFIG
 ******************************************************************************/
static const ru_reg_rec *SYNCE_PLL_CONFIG_REGS[] =
{
    &SYNCE_PLL_CONFIG_IG_REG,
};

unsigned long SYNCE_PLL_CONFIG_ADDRS[] =
{
    0x801440ac,
};

const ru_block_rec SYNCE_PLL_CONFIG_BLOCK = 
{
    "SYNCE_PLL_CONFIG",
    SYNCE_PLL_CONFIG_ADDRS,
    1,
    1,
    SYNCE_PLL_CONFIG_REGS
};

/* End of file SYNCE_PLL_CONFIG.c */
