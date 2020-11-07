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
    "EPN_TX_L1S_SHP_CONFIG_31 Register",
    "This register configures upstream shaper i. There are 32 shapers"
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
    20,
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
    &EPN_TX_L1S_SHP_QUE_EN_CFGSHPEN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPN_TX_L1S_SHP_QUE_EN_REG = 
{
    "QUE_EN",
#if RU_INCLUDE_DESC
    "EPN_TX_L1S_SHP_QUE_EN_31 Register",
    "The bit wise selects for enabling shaping of the associated queues."
    "Each shaper can have shaping enabled on select queues or on all of the"
    "queues. If the bit is set, the corresponding shaper will be used for"
    "shaping credits and control of its associated queue.",
#endif
    EPN_TX_L1S_SHP_QUE_EN_REG_OFFSET,
    0,
    0,
    21,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
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
    0x80141238,
    0x80141240,
    0x80141248,
    0x80141250,
    0x80141258,
    0x80141260,
    0x80141268,
    0x80141270,
    0x80141518,
    0x80141520,
    0x80141528,
    0x80141530,
    0x80141538,
    0x80141540,
    0x80141548,
    0x80141550,
    0x80141558,
    0x80141560,
    0x80141568,
    0x80141570,
    0x80141578,
    0x80141580,
    0x80141588,
    0x80141590,
    0x80141598,
    0x801415a0,
    0x801415a8,
    0x801415b0,
    0x801415b8,
    0x801415c0,
    0x801415c8,
    0x801415d0,
};

const ru_block_rec EPN_TX_L1S_SHP_BLOCK = 
{
    "EPN_TX_L1S_SHP",
    EPN_TX_L1S_SHP_ADDRS,
    32,
    2,
    EPN_TX_L1S_SHP_REGS
};

/* End of file EPON_EPN_TX_L1S_SHP.c */
