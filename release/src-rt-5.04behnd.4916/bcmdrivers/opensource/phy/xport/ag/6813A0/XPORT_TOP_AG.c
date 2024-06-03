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
 * Field: XPORT_TOP_CONTROL_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_TOP_CONTROL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_TOP_CONTROL_RESERVED0_FIELD_MASK,
    0,
    XPORT_TOP_CONTROL_RESERVED0_FIELD_WIDTH,
    XPORT_TOP_CONTROL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_TOP_CONTROL_P3_MODE
 ******************************************************************************/
const ru_field_rec XPORT_TOP_CONTROL_P3_MODE_FIELD =
{
    "P3_MODE",
#if RU_INCLUDE_DESC
    "",
    "P3 mode:\n"
    "0 : P3 operates in GMII mode.\n"
    "1 : P3 operates in XGMII mode.",
#endif
    XPORT_TOP_CONTROL_P3_MODE_FIELD_MASK,
    0,
    XPORT_TOP_CONTROL_P3_MODE_FIELD_WIDTH,
    XPORT_TOP_CONTROL_P3_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_TOP_CONTROL_P2_MODE
 ******************************************************************************/
const ru_field_rec XPORT_TOP_CONTROL_P2_MODE_FIELD =
{
    "P2_MODE",
#if RU_INCLUDE_DESC
    "",
    "P2 mode:\n"
    "0 : P2 operates in GMII mode.\n"
    "1 : P2 operates in XGMII mode.",
#endif
    XPORT_TOP_CONTROL_P2_MODE_FIELD_MASK,
    0,
    XPORT_TOP_CONTROL_P2_MODE_FIELD_WIDTH,
    XPORT_TOP_CONTROL_P2_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_TOP_CONTROL_P1_MODE
 ******************************************************************************/
const ru_field_rec XPORT_TOP_CONTROL_P1_MODE_FIELD =
{
    "P1_MODE",
#if RU_INCLUDE_DESC
    "",
    "P1 mode:\n"
    "0 : P1 operates in GMII mode.\n"
    "1 : P1 operates in XGMII mode.",
#endif
    XPORT_TOP_CONTROL_P1_MODE_FIELD_MASK,
    0,
    XPORT_TOP_CONTROL_P1_MODE_FIELD_WIDTH,
    XPORT_TOP_CONTROL_P1_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_TOP_CONTROL_P0_MODE
 ******************************************************************************/
const ru_field_rec XPORT_TOP_CONTROL_P0_MODE_FIELD =
{
    "P0_MODE",
#if RU_INCLUDE_DESC
    "",
    "P0 mode:\n"
    "0 : P0 operates in GMII mode.\n"
    "1 : P0 operates in XGMII mode.",
#endif
    XPORT_TOP_CONTROL_P0_MODE_FIELD_MASK,
    0,
    XPORT_TOP_CONTROL_P0_MODE_FIELD_WIDTH,
    XPORT_TOP_CONTROL_P0_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_TOP_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_TOP_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_TOP_STATUS_RESERVED0_FIELD_MASK,
    0,
    XPORT_TOP_STATUS_RESERVED0_FIELD_WIDTH,
    XPORT_TOP_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPORT_TOP_STATUS_LINK_STATUS
 ******************************************************************************/
const ru_field_rec XPORT_TOP_STATUS_LINK_STATUS_FIELD =
{
    "LINK_STATUS",
#if RU_INCLUDE_DESC
    "",
    "P3:P0 link status, one bit per port.\n"
    "0 : P<i> link is down,\n"
    "1 : P<i> link is up.",
#endif
    XPORT_TOP_STATUS_LINK_STATUS_FIELD_MASK,
    0,
    XPORT_TOP_STATUS_LINK_STATUS_FIELD_WIDTH,
    XPORT_TOP_STATUS_LINK_STATUS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPORT_TOP_REVISION_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_TOP_REVISION_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_TOP_REVISION_RESERVED0_FIELD_MASK,
    0,
    XPORT_TOP_REVISION_RESERVED0_FIELD_WIDTH,
    XPORT_TOP_REVISION_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPORT_TOP_REVISION_XPORT_REV
 ******************************************************************************/
const ru_field_rec XPORT_TOP_REVISION_XPORT_REV_FIELD =
{
    "XPORT_REV",
#if RU_INCLUDE_DESC
    "",
    "XPORT revision.",
#endif
    XPORT_TOP_REVISION_XPORT_REV_FIELD_MASK,
    0,
    XPORT_TOP_REVISION_XPORT_REV_FIELD_WIDTH,
    XPORT_TOP_REVISION_XPORT_REV_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPORT_TOP_SPARE_CNTRL_SPARE_REG
 ******************************************************************************/
const ru_field_rec XPORT_TOP_SPARE_CNTRL_SPARE_REG_FIELD =
{
    "SPARE_REG",
#if RU_INCLUDE_DESC
    "",
    "Spare register. Reserved for future use.",
#endif
    XPORT_TOP_SPARE_CNTRL_SPARE_REG_FIELD_MASK,
    0,
    XPORT_TOP_SPARE_CNTRL_SPARE_REG_FIELD_WIDTH,
    XPORT_TOP_SPARE_CNTRL_SPARE_REG_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: XPORT_TOP_CONTROL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_TOP_CONTROL_FIELDS[] =
{
    &XPORT_TOP_CONTROL_RESERVED0_FIELD,
    &XPORT_TOP_CONTROL_P3_MODE_FIELD,
    &XPORT_TOP_CONTROL_P2_MODE_FIELD,
    &XPORT_TOP_CONTROL_P1_MODE_FIELD,
    &XPORT_TOP_CONTROL_P0_MODE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_TOP_CONTROL_REG = 
{
    "CONTROL",
#if RU_INCLUDE_DESC
    "XPORT Control/Config Register",
    "",
#endif
    XPORT_TOP_CONTROL_REG_OFFSET,
    0,
    0,
    165,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    XPORT_TOP_CONTROL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_TOP_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_TOP_STATUS_FIELDS[] =
{
    &XPORT_TOP_STATUS_RESERVED0_FIELD,
    &XPORT_TOP_STATUS_LINK_STATUS_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_TOP_STATUS_REG = 
{
    "STATUS",
#if RU_INCLUDE_DESC
    "XPORT Status Register",
    "",
#endif
    XPORT_TOP_STATUS_REG_OFFSET,
    0,
    0,
    166,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_TOP_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_TOP_REVISION
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_TOP_REVISION_FIELDS[] =
{
    &XPORT_TOP_REVISION_RESERVED0_FIELD,
    &XPORT_TOP_REVISION_XPORT_REV_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_TOP_REVISION_REG = 
{
    "REVISION",
#if RU_INCLUDE_DESC
    "XPORT Revision Register",
    "",
#endif
    XPORT_TOP_REVISION_REG_OFFSET,
    0,
    0,
    167,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_TOP_REVISION_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_TOP_SPARE_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_TOP_SPARE_CNTRL_FIELDS[] =
{
    &XPORT_TOP_SPARE_CNTRL_SPARE_REG_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_TOP_SPARE_CNTRL_REG = 
{
    "SPARE_CNTRL",
#if RU_INCLUDE_DESC
    "Spare Control Register",
    "",
#endif
    XPORT_TOP_SPARE_CNTRL_REG_OFFSET,
    0,
    0,
    168,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPORT_TOP_SPARE_CNTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: XPORT_TOP
 ******************************************************************************/
static const ru_reg_rec *XPORT_TOP_REGS[] =
{
    &XPORT_TOP_CONTROL_REG,
    &XPORT_TOP_STATUS_REG,
    &XPORT_TOP_REVISION_REG,
    &XPORT_TOP_SPARE_CNTRL_REG,
};

unsigned long XPORT_TOP_ADDRS[] =
{
    0x837f2000,
    0x837f6000,
};

const ru_block_rec XPORT_TOP_BLOCK = 
{
    "XPORT_TOP",
    XPORT_TOP_ADDRS,
    2,
    4,
    XPORT_TOP_REGS
};

/* End of file XPORT_TOP.c */
