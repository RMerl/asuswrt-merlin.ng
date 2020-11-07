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
 * Field: EPN_TX_L1S_SHP_CONFIG_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_TX_L1S_SHP_CONFIG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_TX_L1S_SHP_CONFIG_RESERVED0_FIELD_MASK,
    0,
    EPN_TX_L1S_SHP_CONFIG_RESERVED0_FIELD_WIDTH,
    EPN_TX_L1S_SHP_CONFIG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_TX_L1S_SHP_CONFIG_CFGSHPRATE
 ******************************************************************************/
const ru_field_rec EPN_TX_L1S_SHP_CONFIG_CFGSHPRATE_FIELD =
{
    "CFGSHPRATE",
#if RU_INCLUDE_DESC
    "",
    "Shaper i Rate",
#endif
    EPN_TX_L1S_SHP_CONFIG_CFGSHPRATE_FIELD_MASK,
    0,
    EPN_TX_L1S_SHP_CONFIG_CFGSHPRATE_FIELD_WIDTH,
    EPN_TX_L1S_SHP_CONFIG_CFGSHPRATE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_TX_L1S_SHP_CONFIG_CFGSHPBSTSIZE
 ******************************************************************************/
const ru_field_rec EPN_TX_L1S_SHP_CONFIG_CFGSHPBSTSIZE_FIELD =
{
    "CFGSHPBSTSIZE",
#if RU_INCLUDE_DESC
    "",
    "Shaper i Maximum Burst Size",
#endif
    EPN_TX_L1S_SHP_CONFIG_CFGSHPBSTSIZE_FIELD_MASK,
    0,
    EPN_TX_L1S_SHP_CONFIG_CFGSHPBSTSIZE_FIELD_WIDTH,
    EPN_TX_L1S_SHP_CONFIG_CFGSHPBSTSIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_TX_L1S_SHP_QUE_EN_RESERVED0
 ******************************************************************************/
const ru_field_rec EPN_TX_L1S_SHP_QUE_EN_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPN_TX_L1S_SHP_QUE_EN_RESERVED0_FIELD_MASK,
    0,
    EPN_TX_L1S_SHP_QUE_EN_RESERVED0_FIELD_WIDTH,
    EPN_TX_L1S_SHP_QUE_EN_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPN_TX_L1S_SHP_QUE_EN_CFGSHPEN
 ******************************************************************************/
const ru_field_rec EPN_TX_L1S_SHP_QUE_EN_CFGSHPEN_FIELD =
{
    "CFGSHPEN",
#if RU_INCLUDE_DESC
    "",
    "Set the bit(s) corresponding to the queue(s) this shaper should"
    "police.",
#endif
    EPN_TX_L1S_SHP_QUE_EN_CFGSHPEN_FIELD_MASK,
    0,
    EPN_TX_L1S_SHP_QUE_EN_CFGSHPEN_FIELD_WIDTH,
    EPN_TX_L1S_SHP_QUE_EN_CFGSHPEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: EPN_TX_L1S_SHP_CONFIG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_TX_L1S_SHP_CONFIG_FIELDS[] =
{
    &EPN_TX_L1S_SHP_CONFIG_RESERVED0_FIELD,
    &EPN_TX_L1S_SHP_CONFIG_CFGSHPRATE_FIELD,
    &EPN_TX_L1S_SHP_CONFIG_CFGSHPBSTSIZE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_TX_L1S_SHP_CONFIG_REG = 
{
    "CONFIG",
#if RU_INCLUDE_DESC
    "EPN_TX_L1S_SHP_CONFIG_7 Register",
    "This register configures upstream shaper 0. There are 32 shapers"
    "available for shaping upstream traffic. The cfgShpEn bits are bit wise"
    "selects for enabling shaping of the associated queues. Each shaper can"
    "have shaping enabled on select queues or on all of the queues. If the"
    "bit is set, the corresponding shaper will be used for shaping credits"
    "and control of its associated queue. The cfgShpRate and cfgMaxBstSize"
    "define the shaping. The cfgShpRate value represents the number of bytes"
    "that are added to the shaper's byte credit accumulator each clock"
    "cycle. Given a 125 MHz clock-cycle; the cfgShpRate is in units of 2^-19"
    "Gbps (~1907.34863 bps). The maximum burst size is in units of 256"
    "bytes.",
#endif
    EPN_TX_L1S_SHP_CONFIG_REG_OFFSET,
    0,
    0,
    151,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    EPN_TX_L1S_SHP_CONFIG_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPN_TX_L1S_SHP_QUE_EN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPN_TX_L1S_SHP_QUE_EN_FIELDS[] =
{
    &EPN_TX_L1S_SHP_QUE_EN_RESERVED0_FIELD,
    &EPN_TX_L1S_SHP_QUE_EN_CFGSHPEN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_TX_L1S_SHP_QUE_EN_REG = 
{
    "QUE_EN",
#if RU_INCLUDE_DESC
    "EPN_TX_L1S_SHP_QUE_EN_7 Register",
    "The bit wise selects for enabling shaping of the associated queues."
    "Each shaper can have shaping enabled on select queues or on all of the"
    "queues. If the bit is set, the corresponding shaper will be used for"
    "shaping credits and control of its associated queue.",
#endif
    EPN_TX_L1S_SHP_QUE_EN_REG_OFFSET,
    0,
    0,
    152,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPN_TX_L1S_SHP_QUE_EN_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: EPN_TX_L1S_SHP
 ******************************************************************************/
static const ru_reg_rec *EPN_TX_L1S_SHP_REGS[] =
{
    &EPN_TX_L1S_SHP_CONFIG_REG,
    &EPN_TX_L1S_SHP_QUE_EN_REG,
};

static unsigned long EPN_TX_L1S_SHP_ADDRS[] =
{
    0x82dad238,
    0x82dad240,
    0x82dad248,
    0x82dad250,
    0x82dad258,
    0x82dad260,
    0x82dad268,
    0x82dad270,
};

const ru_block_rec EPN_TX_L1S_SHP_BLOCK = 
{
    "EPN_TX_L1S_SHP",
    EPN_TX_L1S_SHP_ADDRS,
    8,
    2,
    EPN_TX_L1S_SHP_REGS
};

/* End of file EPON_EPN_TX_L1S_SHP.c */
