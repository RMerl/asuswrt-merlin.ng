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
 * Field: DEBUG_BUS_BUS_SEL_SELECT_MODULE
 ******************************************************************************/
const ru_field_rec DEBUG_BUS_BUS_SEL_SELECT_MODULE_FIELD =
{
    "SELECT_MODULE",
#if RU_INCLUDE_DESC
    "Select_Module",
    "RX_TX selection"
    "00 - RX"
    "10 - TX",
#endif
    DEBUG_BUS_BUS_SEL_SELECT_MODULE_FIELD_MASK,
    0,
    DEBUG_BUS_BUS_SEL_SELECT_MODULE_FIELD_WIDTH,
    DEBUG_BUS_BUS_SEL_SELECT_MODULE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DEBUG_BUS_BUS_SEL_SELECT_LANE
 ******************************************************************************/
const ru_field_rec DEBUG_BUS_BUS_SEL_SELECT_LANE_FIELD =
{
    "SELECT_LANE",
#if RU_INCLUDE_DESC
    "Select_lane",
    "Select_lane",
#endif
    DEBUG_BUS_BUS_SEL_SELECT_LANE_FIELD_MASK,
    0,
    DEBUG_BUS_BUS_SEL_SELECT_LANE_FIELD_WIDTH,
    DEBUG_BUS_BUS_SEL_SELECT_LANE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DEBUG_BUS_BUS_SEL_RESERVED0
 ******************************************************************************/
const ru_field_rec DEBUG_BUS_BUS_SEL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DEBUG_BUS_BUS_SEL_RESERVED0_FIELD_MASK,
    0,
    DEBUG_BUS_BUS_SEL_RESERVED0_FIELD_WIDTH,
    DEBUG_BUS_BUS_SEL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: DEBUG_BUS_BUS_SEL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DEBUG_BUS_BUS_SEL_FIELDS[] =
{
    &DEBUG_BUS_BUS_SEL_SELECT_MODULE_FIELD,
    &DEBUG_BUS_BUS_SEL_SELECT_LANE_FIELD,
    &DEBUG_BUS_BUS_SEL_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DEBUG_BUS_BUS_SEL_REG = 
{
    "BUS_SEL",
#if RU_INCLUDE_DESC
    "SEL Register",
    "Select",
#endif
    DEBUG_BUS_BUS_SEL_REG_OFFSET,
    0,
    0,
    1132,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    DEBUG_BUS_BUS_SEL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Block: DEBUG_BUS
 ******************************************************************************/
static const ru_reg_rec *DEBUG_BUS_REGS[] =
{
    &DEBUG_BUS_BUS_SEL_REG,
};

unsigned long DEBUG_BUS_ADDRS[] =
{
    0x80147870,
    0x80147a70,
    0x80147c70,
    0x80147e70,
};

const ru_block_rec DEBUG_BUS_BLOCK = 
{
    "DEBUG_BUS",
    DEBUG_BUS_ADDRS,
    4,
    1,
    DEBUG_BUS_REGS
};

/* End of file XRDP_DEBUG_BUS.c */
