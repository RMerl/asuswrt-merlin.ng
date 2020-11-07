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
 * Field: TOP_SCRATCH_ATCH_SCRATCH
 ******************************************************************************/
const ru_field_rec TOP_SCRATCH_ATCH_SCRATCH_FIELD =
{
    "SCRATCH",
#if RU_INCLUDE_DESC
    "",
    "Scratch pad.",
#endif
    TOP_SCRATCH_ATCH_SCRATCH_FIELD_MASK,
    0,
    TOP_SCRATCH_ATCH_SCRATCH_FIELD_WIDTH,
    TOP_SCRATCH_ATCH_SCRATCH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: TOP_SCRATCH_ATCH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TOP_SCRATCH_ATCH_FIELDS[] =
{
    &TOP_SCRATCH_ATCH_SCRATCH_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec TOP_SCRATCH_ATCH_REG = 
{
    "ATCH",
#if RU_INCLUDE_DESC
    "WAN_TOP_SCRATCH Register",
    "Register used for testing read and write access into wan_top block.",
#endif
    TOP_SCRATCH_ATCH_REG_OFFSET,
    0,
    0,
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TOP_SCRATCH_ATCH_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: TOP_SCRATCH
 ******************************************************************************/
static const ru_reg_rec *TOP_SCRATCH_REGS[] =
{
    &TOP_SCRATCH_ATCH_REG,
};

unsigned long TOP_SCRATCH_ADDRS[] =
{
#if defined(CONFIG_BCM963158)
    0x80144000,
#else
    #error "TOP_SCRATCH base address not defined"
#endif
};

const ru_block_rec TOP_SCRATCH_BLOCK = 
{
    "TOP_SCRATCH",
    TOP_SCRATCH_ADDRS,
    1,
    1,
    TOP_SCRATCH_REGS
};

/* End of file TOP_SCRATCH.c */
