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

#if RU_INCLUDE_FIELD_DB
/******************************************************************************
 * Field: XLIF_Q_OFF_IND_Q_OFF
 ******************************************************************************/
const ru_field_rec XLIF_Q_OFF_IND_Q_OFF_FIELD =
{
    "Q_OFF",
#if RU_INCLUDE_DESC
    "Q_OFF",
    "Q_OFF",
#endif
    XLIF_Q_OFF_IND_Q_OFF_FIELD_MASK,
    0,
    XLIF_Q_OFF_IND_Q_OFF_FIELD_WIDTH,
    XLIF_Q_OFF_IND_Q_OFF_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XLIF_Q_OFF_IND_FAILOVER_ON
 ******************************************************************************/
const ru_field_rec XLIF_Q_OFF_IND_FAILOVER_ON_FIELD =
{
    "FAILOVER_ON",
#if RU_INCLUDE_DESC
    "Failover_on",
    "Failover_on",
#endif
    XLIF_Q_OFF_IND_FAILOVER_ON_FIELD_MASK,
    0,
    XLIF_Q_OFF_IND_FAILOVER_ON_FIELD_WIDTH,
    XLIF_Q_OFF_IND_FAILOVER_ON_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XLIF_Q_OFF_IND_RESERVED0
 ******************************************************************************/
const ru_field_rec XLIF_Q_OFF_IND_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XLIF_Q_OFF_IND_RESERVED0_FIELD_MASK,
    0,
    XLIF_Q_OFF_IND_RESERVED0_FIELD_WIDTH,
    XLIF_Q_OFF_IND_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: XLIF_Q_OFF_IND
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XLIF_Q_OFF_IND_FIELDS[] =
{
    &XLIF_Q_OFF_IND_Q_OFF_FIELD,
    &XLIF_Q_OFF_IND_FAILOVER_ON_FIELD,
    &XLIF_Q_OFF_IND_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XLIF_Q_OFF_IND_REG = 
{
    "IND",
#if RU_INCLUDE_DESC
    "INDICATIONS Register",
    "indications from the XLMAC IF",
#endif
    XLIF_Q_OFF_IND_REG_OFFSET,
    0,
    0,
    1137,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    XLIF_Q_OFF_IND_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Block: XLIF_Q_OFF
 ******************************************************************************/
static const ru_reg_rec *XLIF_Q_OFF_REGS[] =
{
    &XLIF_Q_OFF_IND_REG,
};

unsigned long XLIF_Q_OFF_ADDRS[] =
{
    0x8014787c,
    0x80147a7c,
    0x80147c7c,
    0x80147e7c,
};

const ru_block_rec XLIF_Q_OFF_BLOCK = 
{
    "XLIF_Q_OFF",
    XLIF_Q_OFF_ADDRS,
    4,
    1,
    XLIF_Q_OFF_REGS
};

/* End of file XRDP_Q_OFF.c */
