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
 * Field: INT_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec INT_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    INT_STATUS_RESERVED0_FIELD_MASK,
    0,
    INT_STATUS_RESERVED0_FIELD_WIDTH,
    INT_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: INT_STATUS_CLK_SAMPLE_INT
 ******************************************************************************/
const ru_field_rec INT_STATUS_CLK_SAMPLE_INT_FIELD =
{
    "CLK_SAMPLE_INT",
#if RU_INCLUDE_DESC
    "",
    "Indicates the sampling of clock counter, as specified by"
    "cfg_pll_smpl_prd sampling period.",
#endif
    INT_STATUS_CLK_SAMPLE_INT_FIELD_MASK,
    0,
    INT_STATUS_CLK_SAMPLE_INT_FIELD_WIDTH,
    INT_STATUS_CLK_SAMPLE_INT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: INT_MASK_RESERVED0
 ******************************************************************************/
const ru_field_rec INT_MASK_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    INT_MASK_RESERVED0_FIELD_MASK,
    0,
    INT_MASK_RESERVED0_FIELD_WIDTH,
    INT_MASK_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: INT_MASK_MSK_CLK_SAMPLE_INT
 ******************************************************************************/
const ru_field_rec INT_MASK_MSK_CLK_SAMPLE_INT_FIELD =
{
    "MSK_CLK_SAMPLE_INT",
#if RU_INCLUDE_DESC
    "",
    "Interrupt mask, active low.",
#endif
    INT_MASK_MSK_CLK_SAMPLE_INT_FIELD_MASK,
    0,
    INT_MASK_MSK_CLK_SAMPLE_INT_FIELD_WIDTH,
    INT_MASK_MSK_CLK_SAMPLE_INT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: INT_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *INT_STATUS_FIELDS[] =
{
    &INT_STATUS_RESERVED0_FIELD,
    &INT_STATUS_CLK_SAMPLE_INT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec INT_STATUS_REG = 
{
    "STATUS",
#if RU_INCLUDE_DESC
    "WAN_INT_STATUS Register",
    "Interrupts.",
#endif
    INT_STATUS_REG_OFFSET,
    0,
    0,
    37,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    INT_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: INT_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *INT_MASK_FIELDS[] =
{
    &INT_MASK_RESERVED0_FIELD,
    &INT_MASK_MSK_CLK_SAMPLE_INT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec INT_MASK_REG = 
{
    "MASK",
#if RU_INCLUDE_DESC
    "WAN_INT_MASK Register",
    "Interrupt masks.",
#endif
    INT_MASK_REG_OFFSET,
    0,
    0,
    38,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    INT_MASK_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: INT
 ******************************************************************************/
static const ru_reg_rec *INT_REGS[] =
{
    &INT_STATUS_REG,
    &INT_MASK_REG,
};

unsigned long INT_ADDRS[] =
{
    0x80144098,
};

const ru_block_rec INT_BLOCK = 
{
    "INT",
    INT_ADDRS,
    1,
    2,
    INT_REGS
};

/* End of file INT.c */
