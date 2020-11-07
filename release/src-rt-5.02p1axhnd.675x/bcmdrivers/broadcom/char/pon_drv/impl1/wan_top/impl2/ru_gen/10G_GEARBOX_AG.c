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
 * Field: TEN_G_GEARBOX_GEARBOX_RESERVED0
 ******************************************************************************/
const ru_field_rec TEN_G_GEARBOX_GEARBOX_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    TEN_G_GEARBOX_GEARBOX_RESERVED0_FIELD_MASK,
    0,
    TEN_G_GEARBOX_GEARBOX_RESERVED0_FIELD_WIDTH,
    TEN_G_GEARBOX_GEARBOX_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_TX_FIFO_OFF
 ******************************************************************************/
const ru_field_rec TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_TX_FIFO_OFF_FIELD =
{
    "CFG_SGB_PON_10G_EPON_TX_FIFO_OFF",
#if RU_INCLUDE_DESC
    "",
    "TX FIFO offset value.",
#endif
    TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_TX_FIFO_OFF_FIELD_MASK,
    0,
    TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_TX_FIFO_OFF_FIELD_WIDTH,
    TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_TX_FIFO_OFF_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_TX_FIFO_OFF_LD
 ******************************************************************************/
const ru_field_rec TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_TX_FIFO_OFF_LD_FIELD =
{
    "CFG_SGB_PON_10G_EPON_TX_FIFO_OFF_LD",
#if RU_INCLUDE_DESC
    "",
    "0: Normal operation. 1: Load new TX FIFO offset.",
#endif
    TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_TX_FIFO_OFF_LD_FIELD_MASK,
    0,
    TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_TX_FIFO_OFF_LD_FIELD_WIDTH,
    TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_TX_FIFO_OFF_LD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_TX2RX_LOOP_EN
 ******************************************************************************/
const ru_field_rec TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_TX2RX_LOOP_EN_FIELD =
{
    "CFG_SGB_PON_10G_EPON_TX2RX_LOOP_EN",
#if RU_INCLUDE_DESC
    "",
    "0: Normal operation. 1: Enable TX to RX loopback.",
#endif
    TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_TX2RX_LOOP_EN_FIELD_MASK,
    0,
    TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_TX2RX_LOOP_EN_FIELD_WIDTH,
    TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_TX2RX_LOOP_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_RX_DATA_END
 ******************************************************************************/
const ru_field_rec TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_RX_DATA_END_FIELD =
{
    "CFG_SGB_PON_10G_EPON_RX_DATA_END",
#if RU_INCLUDE_DESC
    "",
    "0: Normal operation. 1: Enable endian flip.",
#endif
    TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_RX_DATA_END_FIELD_MASK,
    0,
    TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_RX_DATA_END_FIELD_WIDTH,
    TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_RX_DATA_END_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_CLK_EN
 ******************************************************************************/
const ru_field_rec TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_CLK_EN_FIELD =
{
    "CFG_SGB_PON_10G_EPON_CLK_EN",
#if RU_INCLUDE_DESC
    "",
    "0: Disable clock. 1: Enable clock.",
#endif
    TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_CLK_EN_FIELD_MASK,
    0,
    TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_CLK_EN_FIELD_WIDTH,
    TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_CLK_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_TX_GBOX_RSTN
 ******************************************************************************/
const ru_field_rec TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_TX_GBOX_RSTN_FIELD =
{
    "CFG_SGB_PON_10G_EPON_TX_GBOX_RSTN",
#if RU_INCLUDE_DESC
    "",
    "0: Reset TX gearbox logic. 1: Enable TX gearbox logic.",
#endif
    TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_TX_GBOX_RSTN_FIELD_MASK,
    0,
    TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_TX_GBOX_RSTN_FIELD_WIDTH,
    TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_TX_GBOX_RSTN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_RX_GBOX_RSTN
 ******************************************************************************/
const ru_field_rec TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_RX_GBOX_RSTN_FIELD =
{
    "CFG_SGB_PON_10G_EPON_RX_GBOX_RSTN",
#if RU_INCLUDE_DESC
    "",
    "0: Reset RX gearbox logic. 1: Enable RX gearbox logic.",
#endif
    TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_RX_GBOX_RSTN_FIELD_MASK,
    0,
    TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_RX_GBOX_RSTN_FIELD_WIDTH,
    TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_RX_GBOX_RSTN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_TX_CGEN_RSTN
 ******************************************************************************/
const ru_field_rec TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_TX_CGEN_RSTN_FIELD =
{
    "CFG_SGB_PON_10G_EPON_TX_CGEN_RSTN",
#if RU_INCLUDE_DESC
    "",
    "0: Reset TX clock generator. 1: Enable TX clock generator.",
#endif
    TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_TX_CGEN_RSTN_FIELD_MASK,
    0,
    TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_TX_CGEN_RSTN_FIELD_WIDTH,
    TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_TX_CGEN_RSTN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_RX_CGEN_RSTN
 ******************************************************************************/
const ru_field_rec TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_RX_CGEN_RSTN_FIELD =
{
    "CFG_SGB_PON_10G_EPON_RX_CGEN_RSTN",
#if RU_INCLUDE_DESC
    "",
    "0: Reset RX clock generator. 1: Enable RX clock generator.",
#endif
    TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_RX_CGEN_RSTN_FIELD_MASK,
    0,
    TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_RX_CGEN_RSTN_FIELD_WIDTH,
    TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_RX_CGEN_RSTN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: TEN_G_GEARBOX_GEARBOX
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TEN_G_GEARBOX_GEARBOX_FIELDS[] =
{
    &TEN_G_GEARBOX_GEARBOX_RESERVED0_FIELD,
    &TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_TX_FIFO_OFF_FIELD,
    &TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_TX_FIFO_OFF_LD_FIELD,
    &TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_TX2RX_LOOP_EN_FIELD,
    &TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_RX_DATA_END_FIELD,
    &TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_CLK_EN_FIELD,
    &TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_TX_GBOX_RSTN_FIELD,
    &TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_RX_GBOX_RSTN_FIELD,
    &TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_TX_CGEN_RSTN_FIELD,
    &TEN_G_GEARBOX_GEARBOX_CFG_SGB_PON_10G_EPON_RX_CGEN_RSTN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec TEN_G_GEARBOX_GEARBOX_REG = 
{
    "GEARBOX",
#if RU_INCLUDE_DESC
    "EPON_10G_GEARBOX Register",
    "Configuration for the 10G EPON gearbox.",
#endif
    TEN_G_GEARBOX_GEARBOX_REG_OFFSET,
    0,
    0,
    11,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    10,
    TEN_G_GEARBOX_GEARBOX_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: TEN_G_GEARBOX
 ******************************************************************************/
static const ru_reg_rec *TEN_G_GEARBOX_REGS[] =
{
    &TEN_G_GEARBOX_GEARBOX_REG,
};

unsigned long TEN_G_GEARBOX_ADDRS[] =
{
    0x8014402c,
};

const ru_block_rec TEN_G_GEARBOX_BLOCK = 
{
    "TEN_G_GEARBOX",
    TEN_G_GEARBOX_ADDRS,
    1,
    1,
    TEN_G_GEARBOX_REGS
};

/* End of file 10G_GEARBOX.c */
