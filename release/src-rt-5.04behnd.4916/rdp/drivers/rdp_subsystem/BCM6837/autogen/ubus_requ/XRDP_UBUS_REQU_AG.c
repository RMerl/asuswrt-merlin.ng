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


#include "XRDP_UBUS_REQU_AG.h"

/******************************************************************************
 * Register: NAME: UBUS_REQU_XRDP_UBUS_REQUESTER_XRDP_UBUS_RQSTR_EN, TYPE: Type_XRDP_UBUS_REQUESTER_XRDP_UBUS_RQSTR_EN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: EN *****/
const ru_field_rec UBUS_REQU_XRDP_UBUS_REQUESTER_XRDP_UBUS_RQSTR_EN_EN_FIELD =
{
    "EN",
#if RU_INCLUDE_DESC
    "",
    "bridge enable\n",
#endif
    { UBUS_REQU_XRDP_UBUS_REQUESTER_XRDP_UBUS_RQSTR_EN_EN_FIELD_MASK },
    0,
    { UBUS_REQU_XRDP_UBUS_REQUESTER_XRDP_UBUS_RQSTR_EN_EN_FIELD_WIDTH },
    { UBUS_REQU_XRDP_UBUS_REQUESTER_XRDP_UBUS_RQSTR_EN_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_REQU_XRDP_UBUS_REQUESTER_XRDP_UBUS_RQSTR_EN_FIELDS[] =
{
    &UBUS_REQU_XRDP_UBUS_REQUESTER_XRDP_UBUS_RQSTR_EN_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: UBUS_REQU_XRDP_UBUS_REQUESTER_XRDP_UBUS_RQSTR_EN *****/
const ru_reg_rec UBUS_REQU_XRDP_UBUS_REQUESTER_XRDP_UBUS_RQSTR_EN_REG =
{
    "XRDP_UBUS_REQUESTER_XRDP_UBUS_RQSTR_EN",
#if RU_INCLUDE_DESC
    "BRDG_EN Register",
    "bridge enable\n",
#endif
    { UBUS_REQU_XRDP_UBUS_REQUESTER_XRDP_UBUS_RQSTR_EN_REG_OFFSET },
    0,
    0,
    1070,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_REQU_XRDP_UBUS_REQUESTER_XRDP_UBUS_RQSTR_EN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: UBUS_REQU_XRDP_UBUS_REQUESTER_XRDP_UBUS_RQSTR_HYST_CTRL, TYPE: Type_XRDP_UBUS_REQUESTER_XRDP_UBUS_RQSTR_HYST_CTRL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: CMD_SPACE *****/
const ru_field_rec UBUS_REQU_XRDP_UBUS_REQUESTER_XRDP_UBUS_RQSTR_HYST_CTRL_CMD_SPACE_FIELD =
{
    "CMD_SPACE",
#if RU_INCLUDE_DESC
    "",
    "command space indication that controls the ARdy signal.\n\nOnce the HSPACE indication is lower than CMD_SPACE the ARdy will be deasserted\n",
#endif
    { UBUS_REQU_XRDP_UBUS_REQUESTER_XRDP_UBUS_RQSTR_HYST_CTRL_CMD_SPACE_FIELD_MASK },
    0,
    { UBUS_REQU_XRDP_UBUS_REQUESTER_XRDP_UBUS_RQSTR_HYST_CTRL_CMD_SPACE_FIELD_WIDTH },
    { UBUS_REQU_XRDP_UBUS_REQUESTER_XRDP_UBUS_RQSTR_HYST_CTRL_CMD_SPACE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA_SPACE *****/
const ru_field_rec UBUS_REQU_XRDP_UBUS_REQUESTER_XRDP_UBUS_RQSTR_HYST_CTRL_DATA_SPACE_FIELD =
{
    "DATA_SPACE",
#if RU_INCLUDE_DESC
    "",
    "data space indication that controls the ARdy signal.\n\nOnce the DSPACE indication is lower than DATA_SPACE the ARdy will be deasserted\n",
#endif
    { UBUS_REQU_XRDP_UBUS_REQUESTER_XRDP_UBUS_RQSTR_HYST_CTRL_DATA_SPACE_FIELD_MASK },
    0,
    { UBUS_REQU_XRDP_UBUS_REQUESTER_XRDP_UBUS_RQSTR_HYST_CTRL_DATA_SPACE_FIELD_WIDTH },
    { UBUS_REQU_XRDP_UBUS_REQUESTER_XRDP_UBUS_RQSTR_HYST_CTRL_DATA_SPACE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_REQU_XRDP_UBUS_REQUESTER_XRDP_UBUS_RQSTR_HYST_CTRL_FIELDS[] =
{
    &UBUS_REQU_XRDP_UBUS_REQUESTER_XRDP_UBUS_RQSTR_HYST_CTRL_CMD_SPACE_FIELD,
    &UBUS_REQU_XRDP_UBUS_REQUESTER_XRDP_UBUS_RQSTR_HYST_CTRL_DATA_SPACE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: UBUS_REQU_XRDP_UBUS_REQUESTER_XRDP_UBUS_RQSTR_HYST_CTRL *****/
const ru_reg_rec UBUS_REQU_XRDP_UBUS_REQUESTER_XRDP_UBUS_RQSTR_HYST_CTRL_REG =
{
    "XRDP_UBUS_REQUESTER_XRDP_UBUS_RQSTR_HYST_CTRL",
#if RU_INCLUDE_DESC
    "HYST_CTRL Register",
    "control the command / data queue full and empty indications.\n",
#endif
    { UBUS_REQU_XRDP_UBUS_REQUESTER_XRDP_UBUS_RQSTR_HYST_CTRL_REG_OFFSET },
    0,
    0,
    1071,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    UBUS_REQU_XRDP_UBUS_REQUESTER_XRDP_UBUS_RQSTR_HYST_CTRL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: UBUS_REQU_XRDP_UBUS_REQUESTER_XRDP_UBUS_RQSTR_HP, TYPE: Type_XRDP_UBUS_REQUESTER_XRDP_UBUS_RQSTR_HP
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: HP_EN *****/
const ru_field_rec UBUS_REQU_XRDP_UBUS_REQUESTER_XRDP_UBUS_RQSTR_HP_HP_EN_FIELD =
{
    "HP_EN",
#if RU_INCLUDE_DESC
    "",
    "enables the hp mechanism\n",
#endif
    { UBUS_REQU_XRDP_UBUS_REQUESTER_XRDP_UBUS_RQSTR_HP_HP_EN_FIELD_MASK },
    0,
    { UBUS_REQU_XRDP_UBUS_REQUESTER_XRDP_UBUS_RQSTR_HP_HP_EN_FIELD_WIDTH },
    { UBUS_REQU_XRDP_UBUS_REQUESTER_XRDP_UBUS_RQSTR_HP_HP_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_REQU_XRDP_UBUS_REQUESTER_XRDP_UBUS_RQSTR_HP_FIELDS[] =
{
    &UBUS_REQU_XRDP_UBUS_REQUESTER_XRDP_UBUS_RQSTR_HP_HP_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: UBUS_REQU_XRDP_UBUS_REQUESTER_XRDP_UBUS_RQSTR_HP *****/
const ru_reg_rec UBUS_REQU_XRDP_UBUS_REQUESTER_XRDP_UBUS_RQSTR_HP_REG =
{
    "XRDP_UBUS_REQUESTER_XRDP_UBUS_RQSTR_HP",
#if RU_INCLUDE_DESC
    "HIGH_PRIORITY Register",
    "controls the high priority mechanism\n",
#endif
    { UBUS_REQU_XRDP_UBUS_REQUESTER_XRDP_UBUS_RQSTR_HP_REG_OFFSET },
    0,
    0,
    1072,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_REQU_XRDP_UBUS_REQUESTER_XRDP_UBUS_RQSTR_HP_FIELDS,
#endif
};

unsigned long UBUS_REQU_ADDRS[] =
{
    0x8289C000,
    0x8289D000,
    0x8289E000,
    0x8289F000,
};

static const ru_reg_rec *UBUS_REQU_REGS[] =
{
    &UBUS_REQU_XRDP_UBUS_REQUESTER_XRDP_UBUS_RQSTR_EN_REG,
    &UBUS_REQU_XRDP_UBUS_REQUESTER_XRDP_UBUS_RQSTR_HYST_CTRL_REG,
    &UBUS_REQU_XRDP_UBUS_REQUESTER_XRDP_UBUS_RQSTR_HP_REG,
};

const ru_block_rec UBUS_REQU_BLOCK =
{
    "UBUS_REQU",
    UBUS_REQU_ADDRS,
    4,
    3,
    UBUS_REQU_REGS,
};
