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
 * Field: AE_GEARBOX_CONTROL_0__0_RESERVED0
 ******************************************************************************/
const ru_field_rec AE_GEARBOX_CONTROL_0__0_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    AE_GEARBOX_CONTROL_0__0_RESERVED0_FIELD_MASK,
    0,
    AE_GEARBOX_CONTROL_0__0_RESERVED0_FIELD_WIDTH,
    AE_GEARBOX_CONTROL_0__0_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: AE_GEARBOX_CONTROL_0__0_CR_WAN_TOP_AE_GEARBOX_TX_FIFO_OFFSET
 ******************************************************************************/
const ru_field_rec AE_GEARBOX_CONTROL_0__0_CR_WAN_TOP_AE_GEARBOX_TX_FIFO_OFFSET_FIELD =
{
    "CR_WAN_TOP_AE_GEARBOX_TX_FIFO_OFFSET",
#if RU_INCLUDE_DESC
    "",
    "Set to 1 then 0 to load initial offset value determined by"
    "cr_wan_top_ae_gearbox_tx_fifo_offset reg",
#endif
    AE_GEARBOX_CONTROL_0__0_CR_WAN_TOP_AE_GEARBOX_TX_FIFO_OFFSET_FIELD_MASK,
    0,
    AE_GEARBOX_CONTROL_0__0_CR_WAN_TOP_AE_GEARBOX_TX_FIFO_OFFSET_FIELD_WIDTH,
    AE_GEARBOX_CONTROL_0__0_CR_WAN_TOP_AE_GEARBOX_TX_FIFO_OFFSET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: AE_GEARBOX_CONTROL_0__0_RESERVED1
 ******************************************************************************/
const ru_field_rec AE_GEARBOX_CONTROL_0__0_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    AE_GEARBOX_CONTROL_0__0_RESERVED1_FIELD_MASK,
    0,
    AE_GEARBOX_CONTROL_0__0_RESERVED1_FIELD_WIDTH,
    AE_GEARBOX_CONTROL_0__0_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: AE_GEARBOX_CONTROL_0__0_CR_WAN_TOP_AE_GEARBOX_TX_FIFO_OFFSET_LD
 ******************************************************************************/
const ru_field_rec AE_GEARBOX_CONTROL_0__0_CR_WAN_TOP_AE_GEARBOX_TX_FIFO_OFFSET_LD_FIELD =
{
    "CR_WAN_TOP_AE_GEARBOX_TX_FIFO_OFFSET_LD",
#if RU_INCLUDE_DESC
    "",
    "Set to 1 then 0 to load initial offset value determined by"
    "cr_wan_top_ae_gearbox_tx_fifo_offset reg",
#endif
    AE_GEARBOX_CONTROL_0__0_CR_WAN_TOP_AE_GEARBOX_TX_FIFO_OFFSET_LD_FIELD_MASK,
    0,
    AE_GEARBOX_CONTROL_0__0_CR_WAN_TOP_AE_GEARBOX_TX_FIFO_OFFSET_LD_FIELD_WIDTH,
    AE_GEARBOX_CONTROL_0__0_CR_WAN_TOP_AE_GEARBOX_TX_FIFO_OFFSET_LD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: AE_GEARBOX_CONTROL_0__0_CR_WAN_TOP_AE_GEARBOX_FULL_RATE_SERDES_MODE
 ******************************************************************************/
const ru_field_rec AE_GEARBOX_CONTROL_0__0_CR_WAN_TOP_AE_GEARBOX_FULL_RATE_SERDES_MODE_FIELD =
{
    "CR_WAN_TOP_AE_GEARBOX_FULL_RATE_SERDES_MODE",
#if RU_INCLUDE_DESC
    "",
    "0 = compatible with sub rate serdes mode"
    "1 = compatible with full rate serdes mode."
    ""
    "100FX - sub rate only."
    "1G  - sub/full rate"
    "2.5G  - sub/full rate"
    "10G  - full rate only.",
#endif
    AE_GEARBOX_CONTROL_0__0_CR_WAN_TOP_AE_GEARBOX_FULL_RATE_SERDES_MODE_FIELD_MASK,
    0,
    AE_GEARBOX_CONTROL_0__0_CR_WAN_TOP_AE_GEARBOX_FULL_RATE_SERDES_MODE_FIELD_WIDTH,
    AE_GEARBOX_CONTROL_0__0_CR_WAN_TOP_AE_GEARBOX_FULL_RATE_SERDES_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: AE_GEARBOX_CONTROL_0__0_CR_WAN_TOP_AE_GEARBOX_WIDTH_MODE
 ******************************************************************************/
const ru_field_rec AE_GEARBOX_CONTROL_0__0_CR_WAN_TOP_AE_GEARBOX_WIDTH_MODE_FIELD =
{
    "CR_WAN_TOP_AE_GEARBOX_WIDTH_MODE",
#if RU_INCLUDE_DESC
    "",
    "0 = 10b mode"
    "1 = 20b mode"
    ""
    "100FX - 10b mode"
    "1G  - 10b mode"
    "2.5G  - 10b mode"
    "10G  - 20b mode",
#endif
    AE_GEARBOX_CONTROL_0__0_CR_WAN_TOP_AE_GEARBOX_WIDTH_MODE_FIELD_MASK,
    0,
    AE_GEARBOX_CONTROL_0__0_CR_WAN_TOP_AE_GEARBOX_WIDTH_MODE_FIELD_WIDTH,
    AE_GEARBOX_CONTROL_0__0_CR_WAN_TOP_AE_GEARBOX_WIDTH_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: AE_GEARBOX_CONTROL_0__0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *AE_GEARBOX_CONTROL_0__0_FIELDS[] =
{
    &AE_GEARBOX_CONTROL_0__0_RESERVED0_FIELD,
    &AE_GEARBOX_CONTROL_0__0_CR_WAN_TOP_AE_GEARBOX_TX_FIFO_OFFSET_FIELD,
    &AE_GEARBOX_CONTROL_0__0_RESERVED1_FIELD,
    &AE_GEARBOX_CONTROL_0__0_CR_WAN_TOP_AE_GEARBOX_TX_FIFO_OFFSET_LD_FIELD,
    &AE_GEARBOX_CONTROL_0__0_CR_WAN_TOP_AE_GEARBOX_FULL_RATE_SERDES_MODE_FIELD,
    &AE_GEARBOX_CONTROL_0__0_CR_WAN_TOP_AE_GEARBOX_WIDTH_MODE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec AE_GEARBOX_CONTROL_0__0_REG = 
{
    "_0",
#if RU_INCLUDE_DESC
    "WAN_TOP_AE_GEARBOX_CONTROL_0 Register",
    "Register used to control AE Gearbox",
#endif
    AE_GEARBOX_CONTROL_0__0_REG_OFFSET,
    0,
    0,
    49,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    AE_GEARBOX_CONTROL_0__0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: AE_GEARBOX_CONTROL_0
 ******************************************************************************/
static const ru_reg_rec *AE_GEARBOX_CONTROL_0_REGS[] =
{
    &AE_GEARBOX_CONTROL_0__0_REG,
};

unsigned long AE_GEARBOX_CONTROL_0_ADDRS[] =
{
    0x801440c4,
};

const ru_block_rec AE_GEARBOX_CONTROL_0_BLOCK = 
{
    "AE_GEARBOX_CONTROL_0",
    AE_GEARBOX_CONTROL_0_ADDRS,
    1,
    1,
    AE_GEARBOX_CONTROL_0_REGS
};

/* End of file AE_GEARBOX_CONTROL_0.c */
