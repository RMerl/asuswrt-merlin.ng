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
 * Field: CLOCK_SYNC_CONFIG_IG_RESERVED0
 ******************************************************************************/
const ru_field_rec CLOCK_SYNC_CONFIG_IG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    CLOCK_SYNC_CONFIG_IG_RESERVED0_FIELD_MASK,
    0,
    CLOCK_SYNC_CONFIG_IG_RESERVED0_FIELD_WIDTH,
    CLOCK_SYNC_CONFIG_IG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CLOCK_SYNC_CONFIG_IG_CFG_GPIO_1PPS_OEB
 ******************************************************************************/
const ru_field_rec CLOCK_SYNC_CONFIG_IG_CFG_GPIO_1PPS_OEB_FIELD =
{
    "CFG_GPIO_1PPS_OEB",
#if RU_INCLUDE_DESC
    "",
    "Output enable for 1PPS output to GPIO, applicable only if"
    "cfg_gpio_1pps_src is cleared.",
#endif
    CLOCK_SYNC_CONFIG_IG_CFG_GPIO_1PPS_OEB_FIELD_MASK,
    0,
    CLOCK_SYNC_CONFIG_IG_CFG_GPIO_1PPS_OEB_FIELD_WIDTH,
    CLOCK_SYNC_CONFIG_IG_CFG_GPIO_1PPS_OEB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CLOCK_SYNC_CONFIG_IG_CFG_GPIO_1PPS_SRC
 ******************************************************************************/
const ru_field_rec CLOCK_SYNC_CONFIG_IG_CFG_GPIO_1PPS_SRC_FIELD =
{
    "CFG_GPIO_1PPS_SRC",
#if RU_INCLUDE_DESC
    "",
    "Selects the source of 1PPS output to GPIO : 0 - NCO; 1 - Switch.",
#endif
    CLOCK_SYNC_CONFIG_IG_CFG_GPIO_1PPS_SRC_FIELD_MASK,
    0,
    CLOCK_SYNC_CONFIG_IG_CFG_GPIO_1PPS_SRC_FIELD_WIDTH,
    CLOCK_SYNC_CONFIG_IG_CFG_GPIO_1PPS_SRC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: CLOCK_SYNC_CONFIG_IG_CFG_SWITCH_SYNCIN_SRC
 ******************************************************************************/
const ru_field_rec CLOCK_SYNC_CONFIG_IG_CFG_SWITCH_SYNCIN_SRC_FIELD =
{
    "CFG_SWITCH_SYNCIN_SRC",
#if RU_INCLUDE_DESC
    "",
    "Selects the source of sync pulse output to switch : 0 - 1PPS; 1 -"
    "recovered PHY/SerDes clock.",
#endif
    CLOCK_SYNC_CONFIG_IG_CFG_SWITCH_SYNCIN_SRC_FIELD_MASK,
    0,
    CLOCK_SYNC_CONFIG_IG_CFG_SWITCH_SYNCIN_SRC_FIELD_WIDTH,
    CLOCK_SYNC_CONFIG_IG_CFG_SWITCH_SYNCIN_SRC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: CLOCK_SYNC_CONFIG_IG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *CLOCK_SYNC_CONFIG_IG_FIELDS[] =
{
    &CLOCK_SYNC_CONFIG_IG_RESERVED0_FIELD,
    &CLOCK_SYNC_CONFIG_IG_CFG_GPIO_1PPS_OEB_FIELD,
    &CLOCK_SYNC_CONFIG_IG_CFG_GPIO_1PPS_SRC_FIELD,
    &CLOCK_SYNC_CONFIG_IG_CFG_SWITCH_SYNCIN_SRC_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec CLOCK_SYNC_CONFIG_IG_REG = 
{
    "IG",
#if RU_INCLUDE_DESC
    "WAN_CLOCK_SYNC_CONFIG Register",
    "Provides the configuration for clock syncing.",
#endif
    CLOCK_SYNC_CONFIG_IG_REG_OFFSET,
    0,
    0,
    51,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    CLOCK_SYNC_CONFIG_IG_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: CLOCK_SYNC_CONFIG
 ******************************************************************************/
static const ru_reg_rec *CLOCK_SYNC_CONFIG_REGS[] =
{
    &CLOCK_SYNC_CONFIG_IG_REG,
};

unsigned long CLOCK_SYNC_CONFIG_ADDRS[] =
{
    0x801440cc,
};

const ru_block_rec CLOCK_SYNC_CONFIG_BLOCK = 
{
    "CLOCK_SYNC_CONFIG",
    CLOCK_SYNC_CONFIG_ADDRS,
    1,
    1,
    CLOCK_SYNC_CONFIG_REGS
};

/* End of file CLOCK_SYNC_CONFIG.c */
