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
 * Field: XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT_PFC_EN
 ******************************************************************************/
const ru_field_rec XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT_PFC_EN_FIELD =
{
    "PFC_EN",
#if RU_INCLUDE_DESC
    "PFC_en",
    "PFC_EN",
#endif
    XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT_PFC_EN_FIELD_MASK,
    0,
    XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT_PFC_EN_FIELD_WIDTH,
    XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT_PFC_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT_RESERVED0
 ******************************************************************************/
const ru_field_rec XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT_RESERVED0_FIELD_MASK,
    0,
    XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT_RESERVED0_FIELD_WIDTH,
    XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT_LLFC_EN
 ******************************************************************************/
const ru_field_rec XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT_LLFC_EN_FIELD =
{
    "LLFC_EN",
#if RU_INCLUDE_DESC
    "LLFC_en",
    "LLFC_en",
#endif
    XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT_LLFC_EN_FIELD_MASK,
    0,
    XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT_LLFC_EN_FIELD_WIDTH,
    XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT_LLFC_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT_RESERVED1
 ******************************************************************************/
const ru_field_rec XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT_RESERVED1_FIELD_MASK,
    0,
    XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT_RESERVED1_FIELD_WIDTH,
    XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XLIF_TX_FLOW_CONTROL_COSMAP_STAT_VALUE
 ******************************************************************************/
const ru_field_rec XLIF_TX_FLOW_CONTROL_COSMAP_STAT_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "Value",
    "value",
#endif
    XLIF_TX_FLOW_CONTROL_COSMAP_STAT_VALUE_FIELD_MASK,
    0,
    XLIF_TX_FLOW_CONTROL_COSMAP_STAT_VALUE_FIELD_WIDTH,
    XLIF_TX_FLOW_CONTROL_COSMAP_STAT_VALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XLIF_TX_FLOW_CONTROL_COSMAP_STAT_RESERVED0
 ******************************************************************************/
const ru_field_rec XLIF_TX_FLOW_CONTROL_COSMAP_STAT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XLIF_TX_FLOW_CONTROL_COSMAP_STAT_RESERVED0_FIELD_MASK,
    0,
    XLIF_TX_FLOW_CONTROL_COSMAP_STAT_RESERVED0_FIELD_WIDTH,
    XLIF_TX_FLOW_CONTROL_COSMAP_STAT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT_FIELDS[] =
{
    &XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT_PFC_EN_FIELD,
    &XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT_RESERVED0_FIELD,
    &XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT_LLFC_EN_FIELD,
    &XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT_REG = 
{
    "COSMAP_EN_STAT",
#if RU_INCLUDE_DESC
    "COSMAP_EN_STATUS Register",
    "cosmap_en indications from the XLMAC Interface",
#endif
    XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT_REG_OFFSET,
    0,
    0,
    1133,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: XLIF_TX_FLOW_CONTROL_COSMAP_STAT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XLIF_TX_FLOW_CONTROL_COSMAP_STAT_FIELDS[] =
{
    &XLIF_TX_FLOW_CONTROL_COSMAP_STAT_VALUE_FIELD,
    &XLIF_TX_FLOW_CONTROL_COSMAP_STAT_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XLIF_TX_FLOW_CONTROL_COSMAP_STAT_REG = 
{
    "COSMAP_STAT",
#if RU_INCLUDE_DESC
    "COSMAP_STATUS Register",
    "cosmap_status from the XLMAC Interface",
#endif
    XLIF_TX_FLOW_CONTROL_COSMAP_STAT_REG_OFFSET,
    0,
    0,
    1134,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XLIF_TX_FLOW_CONTROL_COSMAP_STAT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Block: XLIF_TX_FLOW_CONTROL
 ******************************************************************************/
static const ru_reg_rec *XLIF_TX_FLOW_CONTROL_REGS[] =
{
    &XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT_REG,
    &XLIF_TX_FLOW_CONTROL_COSMAP_STAT_REG,
};

unsigned long XLIF_TX_FLOW_CONTROL_ADDRS[] =
{
    0x80147860,
    0x80147a60,
    0x80147c60,
    0x80147e60,
};

const ru_block_rec XLIF_TX_FLOW_CONTROL_BLOCK = 
{
    "XLIF_TX_FLOW_CONTROL",
    XLIF_TX_FLOW_CONTROL_ADDRS,
    4,
    2,
    XLIF_TX_FLOW_CONTROL_REGS
};

/* End of file XRDP_XLIF_TX_FLOW_CONTROL.c */
