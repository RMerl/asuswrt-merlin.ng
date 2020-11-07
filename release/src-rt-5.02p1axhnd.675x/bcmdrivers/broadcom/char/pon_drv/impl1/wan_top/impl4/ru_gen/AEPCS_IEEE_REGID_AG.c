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
 * Field: AEPCS_IEEE_REGID_ID_CFG_AEPCS_IEEE_REGID
 ******************************************************************************/
const ru_field_rec AEPCS_IEEE_REGID_ID_CFG_AEPCS_IEEE_REGID_FIELD =
{
    "CFG_AEPCS_IEEE_REGID",
#if RU_INCLUDE_DESC
    "",
    "The AE PCS IEEE device ID.",
#endif
    AEPCS_IEEE_REGID_ID_CFG_AEPCS_IEEE_REGID_FIELD_MASK,
    0,
    AEPCS_IEEE_REGID_ID_CFG_AEPCS_IEEE_REGID_FIELD_WIDTH,
    AEPCS_IEEE_REGID_ID_CFG_AEPCS_IEEE_REGID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: AEPCS_IEEE_REGID_ID
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *AEPCS_IEEE_REGID_ID_FIELDS[] =
{
    &AEPCS_IEEE_REGID_ID_CFG_AEPCS_IEEE_REGID_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec AEPCS_IEEE_REGID_ID_REG = 
{
    "ID",
#if RU_INCLUDE_DESC
    "WAN_AEPCS_IEEE_REGID Register",
    "Provides the configuration for AE PCS IEEE device ID register.",
#endif
    AEPCS_IEEE_REGID_ID_REG_OFFSET,
    0,
    0,
    52,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    AEPCS_IEEE_REGID_ID_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: AEPCS_IEEE_REGID
 ******************************************************************************/
static const ru_reg_rec *AEPCS_IEEE_REGID_REGS[] =
{
    &AEPCS_IEEE_REGID_ID_REG,
};

unsigned long AEPCS_IEEE_REGID_ADDRS[] =
{
    0x801440d0,
};

const ru_block_rec AEPCS_IEEE_REGID_BLOCK = 
{
    "AEPCS_IEEE_REGID",
    AEPCS_IEEE_REGID_ADDRS,
    1,
    1,
    AEPCS_IEEE_REGID_REGS
};

/* End of file AEPCS_IEEE_REGID.c */
