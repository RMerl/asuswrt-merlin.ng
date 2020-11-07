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
 * Field: UBUS_MSTR_EN_EN
 ******************************************************************************/
const ru_field_rec UBUS_MSTR_EN_EN_FIELD =
{
    "EN",
#if RU_INCLUDE_DESC
    "BRDG_ENABLE",
    "bridge enable",
#endif
    UBUS_MSTR_EN_EN_FIELD_MASK,
    0,
    UBUS_MSTR_EN_EN_FIELD_WIDTH,
    UBUS_MSTR_EN_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_MSTR_EN_RESERVED0
 ******************************************************************************/
const ru_field_rec UBUS_MSTR_EN_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UBUS_MSTR_EN_RESERVED0_FIELD_MASK,
    0,
    UBUS_MSTR_EN_RESERVED0_FIELD_WIDTH,
    UBUS_MSTR_EN_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_MSTR_HYST_CTRL_CMD_SPACE
 ******************************************************************************/
const ru_field_rec UBUS_MSTR_HYST_CTRL_CMD_SPACE_FIELD =
{
    "CMD_SPACE",
#if RU_INCLUDE_DESC
    "CMD_SPACE",
    "command space indication that controls the ARdy signal."
    ""
    "Once the HSPACE indication is lower than CMD_SPACE the ARdy will be deasserted",
#endif
    UBUS_MSTR_HYST_CTRL_CMD_SPACE_FIELD_MASK,
    0,
    UBUS_MSTR_HYST_CTRL_CMD_SPACE_FIELD_WIDTH,
    UBUS_MSTR_HYST_CTRL_CMD_SPACE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_MSTR_HYST_CTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec UBUS_MSTR_HYST_CTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UBUS_MSTR_HYST_CTRL_RESERVED0_FIELD_MASK,
    0,
    UBUS_MSTR_HYST_CTRL_RESERVED0_FIELD_WIDTH,
    UBUS_MSTR_HYST_CTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_MSTR_HYST_CTRL_DATA_SPACE
 ******************************************************************************/
const ru_field_rec UBUS_MSTR_HYST_CTRL_DATA_SPACE_FIELD =
{
    "DATA_SPACE",
#if RU_INCLUDE_DESC
    "DATA_SPACE",
    "data space indication that controls the ARdy signal."
    ""
    "Once the DSPACE indication is lower than DATA_SPACE the ARdy will be deasserted",
#endif
    UBUS_MSTR_HYST_CTRL_DATA_SPACE_FIELD_MASK,
    0,
    UBUS_MSTR_HYST_CTRL_DATA_SPACE_FIELD_WIDTH,
    UBUS_MSTR_HYST_CTRL_DATA_SPACE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_MSTR_HYST_CTRL_RESERVED1
 ******************************************************************************/
const ru_field_rec UBUS_MSTR_HYST_CTRL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UBUS_MSTR_HYST_CTRL_RESERVED1_FIELD_MASK,
    0,
    UBUS_MSTR_HYST_CTRL_RESERVED1_FIELD_WIDTH,
    UBUS_MSTR_HYST_CTRL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_MSTR_HP_HP_EN
 ******************************************************************************/
const ru_field_rec UBUS_MSTR_HP_HP_EN_FIELD =
{
    "HP_EN",
#if RU_INCLUDE_DESC
    "hp_en",
    "enables the hp mechanism",
#endif
    UBUS_MSTR_HP_HP_EN_FIELD_MASK,
    0,
    UBUS_MSTR_HP_HP_EN_FIELD_WIDTH,
    UBUS_MSTR_HP_HP_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_MSTR_HP_RESERVED0
 ******************************************************************************/
const ru_field_rec UBUS_MSTR_HP_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UBUS_MSTR_HP_RESERVED0_FIELD_MASK,
    0,
    UBUS_MSTR_HP_RESERVED0_FIELD_WIDTH,
    UBUS_MSTR_HP_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: UBUS_MSTR_EN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_MSTR_EN_FIELDS[] =
{
    &UBUS_MSTR_EN_EN_FIELD,
    &UBUS_MSTR_EN_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UBUS_MSTR_EN_REG = 
{
    "EN",
#if RU_INCLUDE_DESC
    "BRDG_EN Register",
    "bridge enable",
#endif
    UBUS_MSTR_EN_REG_OFFSET,
    0,
    0,
    623,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    UBUS_MSTR_EN_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_MSTR_HYST_CTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_MSTR_HYST_CTRL_FIELDS[] =
{
    &UBUS_MSTR_HYST_CTRL_CMD_SPACE_FIELD,
    &UBUS_MSTR_HYST_CTRL_RESERVED0_FIELD,
    &UBUS_MSTR_HYST_CTRL_DATA_SPACE_FIELD,
    &UBUS_MSTR_HYST_CTRL_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UBUS_MSTR_HYST_CTRL_REG = 
{
    "HYST_CTRL",
#if RU_INCLUDE_DESC
    "HYST_CTRL Register",
    "control the command / data queue full and empty indications.",
#endif
    UBUS_MSTR_HYST_CTRL_REG_OFFSET,
    0,
    0,
    624,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    UBUS_MSTR_HYST_CTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_MSTR_HP
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_MSTR_HP_FIELDS[] =
{
    &UBUS_MSTR_HP_HP_EN_FIELD,
    &UBUS_MSTR_HP_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UBUS_MSTR_HP_REG = 
{
    "HP",
#if RU_INCLUDE_DESC
    "HIGH_PRIORITY Register",
    "controls the high priority mechanism",
#endif
    UBUS_MSTR_HP_REG_OFFSET,
    0,
    0,
    625,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    UBUS_MSTR_HP_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Block: UBUS_MSTR
 ******************************************************************************/
static const ru_reg_rec *UBUS_MSTR_REGS[] =
{
    &UBUS_MSTR_EN_REG,
    &UBUS_MSTR_HYST_CTRL_REG,
    &UBUS_MSTR_HP_REG,
};

unsigned long UBUS_MSTR_ADDRS[] =
{
    0x82d96000,
};

const ru_block_rec UBUS_MSTR_BLOCK = 
{
    "UBUS_MSTR",
    UBUS_MSTR_ADDRS,
    1,
    3,
    UBUS_MSTR_REGS
};

/* End of file XRDP_UBUS_MSTR.c */
