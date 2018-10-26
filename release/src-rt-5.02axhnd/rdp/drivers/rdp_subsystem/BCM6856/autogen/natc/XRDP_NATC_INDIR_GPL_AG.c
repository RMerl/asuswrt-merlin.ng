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
 * Register: NATC_INDIR_C_INDIR_ADDR_REG
 ******************************************************************************/
const ru_reg_rec NATC_INDIR_C_INDIR_ADDR_REG_REG = 
{
    "C_INDIR_ADDR_REG",
#if RU_INCLUDE_DESC
    "NATC Indirect Address Register",
    "",
#endif
    NATC_INDIR_C_INDIR_ADDR_REG_REG_OFFSET,
    0,
    0,
    1117,
};

/******************************************************************************
 * Register: NATC_INDIR_C_INDIR_DATA_REG
 ******************************************************************************/
const ru_reg_rec NATC_INDIR_C_INDIR_DATA_REG_REG = 
{
    "C_INDIR_DATA_REG",
#if RU_INCLUDE_DESC
    "MATC Indirect Data Register",
    "",
#endif
    NATC_INDIR_C_INDIR_DATA_REG_REG_OFFSET,
    NATC_INDIR_C_INDIR_DATA_REG_REG_RAM_CNT,
    4,
    1118,
};

/******************************************************************************
 * Block: NATC_INDIR
 ******************************************************************************/
static const ru_reg_rec *NATC_INDIR_REGS[] =
{
    &NATC_INDIR_C_INDIR_ADDR_REG_REG,
    &NATC_INDIR_C_INDIR_DATA_REG_REG,
};

unsigned long NATC_INDIR_ADDRS[] =
{
    0x82e50400,
};

const ru_block_rec NATC_INDIR_BLOCK = 
{
    "NATC_INDIR",
    NATC_INDIR_ADDRS,
    1,
    2,
    NATC_INDIR_REGS
};

/* End of file XRDP_NATC_INDIR.c */
