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
 * Register: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG
 ******************************************************************************/
const ru_reg_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_REG = 
{
    "UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG",
#if RU_INCLUDE_DESC
    "UNIMAC_CFG Register",
    "Interface static configuration values",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_REG_OFFSET,
    0,
    0,
    949,
};

/******************************************************************************
 * Register: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1
 ******************************************************************************/
const ru_reg_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1_REG = 
{
    "UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1",
#if RU_INCLUDE_DESC
    "UNIMAC_EXT_CFG1 Register",
    "Configure additional parameters that influence UNIMAC external logic.",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1_REG_OFFSET,
    0,
    0,
    950,
};

/******************************************************************************
 * Register: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2
 ******************************************************************************/
const ru_reg_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_REG = 
{
    "UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2",
#if RU_INCLUDE_DESC
    "UNIMAC_EXT_CFG2 Register",
    "Configure additional parameters that influence UNIMAC external logic.",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_REG_OFFSET,
    0,
    0,
    951,
};

/******************************************************************************
 * Register: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_UPDATE_MASK
 ******************************************************************************/
const ru_reg_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_UPDATE_MASK_REG = 
{
    "UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_UPDATE_MASK",
#if RU_INCLUDE_DESC
    "UNIMAC_STAT_UPDATE_MASK Register",
    "Mask bits [32:16] of RSV. 1 indicates update disabled for the respective error condition.",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_UPDATE_MASK_REG_OFFSET,
    0,
    0,
    952,
};

/******************************************************************************
 * Register: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT
 ******************************************************************************/
const ru_reg_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_REG = 
{
    "UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT",
#if RU_INCLUDE_DESC
    "UNIMAC_STAT Register",
    "This registers holds status indications of the UNIMAC",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_REG_OFFSET,
    0,
    0,
    953,
};

/******************************************************************************
 * Register: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_DEBUG
 ******************************************************************************/
const ru_reg_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_DEBUG_REG = 
{
    "UNIMAC_TOP_UNIMAC_MISC_UNIMAC_DEBUG",
#if RU_INCLUDE_DESC
    "UNIMAC_DEBUG Register",
    "This register holds all debug related configurations",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_DEBUG_REG_OFFSET,
    0,
    0,
    954,
};

/******************************************************************************
 * Register: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RST
 ******************************************************************************/
const ru_reg_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RST_REG = 
{
    "UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RST",
#if RU_INCLUDE_DESC
    "UNIMAC_RST Register",
    "This is a synchronous reset to the UNIMAC core",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RST_REG_OFFSET,
    0,
    0,
    955,
};

/******************************************************************************
 * Register: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RSV_MASK
 ******************************************************************************/
const ru_reg_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RSV_MASK_REG = 
{
    "UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RSV_MASK",
#if RU_INCLUDE_DESC
    "UNIMAC_RSV_MASK Register",
    "Mask bits [32:16] of RSV. 1 indicates purge enabled for the respective error condition.",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RSV_MASK_REG_OFFSET,
    0,
    0,
    956,
};

/******************************************************************************
 * Register: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_OVERRUN_COUNTER
 ******************************************************************************/
const ru_reg_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_OVERRUN_COUNTER_REG = 
{
    "UNIMAC_TOP_UNIMAC_MISC_UNIMAC_OVERRUN_COUNTER",
#if RU_INCLUDE_DESC
    "UNIMAC_OVERRUN_COUNTER Register",
    "This registers holds the number of 128-bit lines that were received from the UNIMAC IP, but were thrown due to the fact that the unimac_glue FIFO was full.",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_OVERRUN_COUNTER_REG_OFFSET,
    0,
    0,
    957,
};

/******************************************************************************
 * Register: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588
 ******************************************************************************/
const ru_reg_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_REG = 
{
    "UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588",
#if RU_INCLUDE_DESC
    "UNIMAC_1588 Register",
    "1588 configurations",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_REG_OFFSET,
    0,
    0,
    958,
};

/******************************************************************************
 * Register: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISR
 ******************************************************************************/
const ru_reg_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISR_REG = 
{
    "UNIMAC_TOP_UNIMAC_INTS_ISR",
#if RU_INCLUDE_DESC
    "ISR Register",
    "This register provides status bits for all the general interrupt sources.",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISR_REG_OFFSET,
    0,
    0,
    959,
};

/******************************************************************************
 * Register: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_IER
 ******************************************************************************/
const ru_reg_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_IER_REG = 
{
    "UNIMAC_TOP_UNIMAC_INTS_IER",
#if RU_INCLUDE_DESC
    "IER Register",
    "This register provides enable bits for all the general interrupt sources."
    "The bit description is the same like for the ISR.",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_IER_REG_OFFSET,
    0,
    0,
    960,
};

/******************************************************************************
 * Register: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ITR
 ******************************************************************************/
const ru_reg_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ITR_REG = 
{
    "UNIMAC_TOP_UNIMAC_INTS_ITR",
#if RU_INCLUDE_DESC
    "ITR Register",
    "This register enables alarm interrupt testing. Writing 1 to a bit in this register sets the corresponding bit in the ISR register. Read always returns 0."
    "The bit description is the same like for the ISR.",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ITR_REG_OFFSET,
    0,
    0,
    961,
};

/******************************************************************************
 * Register: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISM
 ******************************************************************************/
const ru_reg_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISM_REG = 
{
    "UNIMAC_TOP_UNIMAC_INTS_ISM",
#if RU_INCLUDE_DESC
    "ISM Register",
    "This read only register provides masked status bits. Each bit corresponds to logical AND operation between the corresponding bits of the ISR and IER registers.The bit description is the same like for the ISR.",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISM_REG_OFFSET,
    0,
    0,
    962,
};

/******************************************************************************
 * Block: UNIMAC_MISC
 ******************************************************************************/
static const ru_reg_rec *UNIMAC_MISC_REGS[] =
{
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_REG,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1_REG,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_REG,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_UPDATE_MASK_REG,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_REG,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_DEBUG_REG,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RST_REG,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RSV_MASK_REG,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_OVERRUN_COUNTER_REG,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_REG,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISR_REG,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_IER_REG,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ITR_REG,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISM_REG,
};

unsigned long UNIMAC_MISC_ADDRS[] =
{
    0x82daa000,
    0x82daa400,
    0x82daa800,
    0x82daac00,
    0x82dab000,
    0x82dab400,
};

const ru_block_rec UNIMAC_MISC_BLOCK = 
{
    "UNIMAC_MISC",
    UNIMAC_MISC_ADDRS,
    6,
    14,
    UNIMAC_MISC_REGS
};

/* End of file XRDP_UNIMAC_MISC.c */
