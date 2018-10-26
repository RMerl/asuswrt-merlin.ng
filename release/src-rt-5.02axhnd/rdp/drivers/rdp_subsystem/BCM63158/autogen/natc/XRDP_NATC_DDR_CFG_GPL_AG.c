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
 * Register: NATC_DDR_CFG__DDR_SIZE
 ******************************************************************************/
const ru_reg_rec NATC_DDR_CFG__DDR_SIZE_REG = 
{
    "_DDR_SIZE",
#if RU_INCLUDE_DESC
    "NAT Cache DDR Size Register",
    "DDR Size Register",
#endif
    NATC_DDR_CFG__DDR_SIZE_REG_OFFSET,
    0,
    0,
    959,
};

/******************************************************************************
 * Register: NATC_DDR_CFG__DDR_BINS_PER_BUCKET_0
 ******************************************************************************/
const ru_reg_rec NATC_DDR_CFG__DDR_BINS_PER_BUCKET_0_REG = 
{
    "_DDR_BINS_PER_BUCKET_0",
#if RU_INCLUDE_DESC
    "NAT Cache DDR Bins Per Bucket 0 register",
    "DDR Bins Per Bucket Register 0",
#endif
    NATC_DDR_CFG__DDR_BINS_PER_BUCKET_0_REG_OFFSET,
    0,
    0,
    960,
};

/******************************************************************************
 * Register: NATC_DDR_CFG__DDR_BINS_PER_BUCKET_1
 ******************************************************************************/
const ru_reg_rec NATC_DDR_CFG__DDR_BINS_PER_BUCKET_1_REG = 
{
    "_DDR_BINS_PER_BUCKET_1",
#if RU_INCLUDE_DESC
    "NAT Cache DDR Bins Per Bucket 1 Register",
    "DDR Bins Per Bucket Register 1",
#endif
    NATC_DDR_CFG__DDR_BINS_PER_BUCKET_1_REG_OFFSET,
    0,
    0,
    961,
};

/******************************************************************************
 * Register: NATC_DDR_CFG__TOTAL_LEN
 ******************************************************************************/
const ru_reg_rec NATC_DDR_CFG__TOTAL_LEN_REG = 
{
    "_TOTAL_LEN",
#if RU_INCLUDE_DESC
    "NAT Cache Total Length Register",
    "DDR TABLE Total Length Register",
#endif
    NATC_DDR_CFG__TOTAL_LEN_REG_OFFSET,
    0,
    0,
    962,
};

/******************************************************************************
 * Register: NATC_DDR_CFG__SM_STATUS
 ******************************************************************************/
const ru_reg_rec NATC_DDR_CFG__SM_STATUS_REG = 
{
    "_SM_STATUS",
#if RU_INCLUDE_DESC
    "NAT State Machine Status Register",
    "NAT State Machine Status Register",
#endif
    NATC_DDR_CFG__SM_STATUS_REG_OFFSET,
    0,
    0,
    963,
};

/******************************************************************************
 * Block: NATC_DDR_CFG
 ******************************************************************************/
static const ru_reg_rec *NATC_DDR_CFG_REGS[] =
{
    &NATC_DDR_CFG__DDR_SIZE_REG,
    &NATC_DDR_CFG__DDR_BINS_PER_BUCKET_0_REG,
    &NATC_DDR_CFG__DDR_BINS_PER_BUCKET_1_REG,
    &NATC_DDR_CFG__TOTAL_LEN_REG,
    &NATC_DDR_CFG__SM_STATUS_REG,
};

unsigned long NATC_DDR_CFG_ADDRS[] =
{
    0x82e50410,
};

const ru_block_rec NATC_DDR_CFG_BLOCK = 
{
    "NATC_DDR_CFG",
    NATC_DDR_CFG_ADDRS,
    1,
    5,
    NATC_DDR_CFG_REGS
};

/* End of file XRDP_NATC_DDR_CFG.c */
