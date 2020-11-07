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
 * Field: TOD_CONFIG_0_RESERVED0
 ******************************************************************************/
const ru_field_rec TOD_CONFIG_0_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    TOD_CONFIG_0_RESERVED0_FIELD_MASK,
    0,
    TOD_CONFIG_0_RESERVED0_FIELD_WIDTH,
    TOD_CONFIG_0_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TOD_CONFIG_0_CFG_TS48_READ
 ******************************************************************************/
const ru_field_rec TOD_CONFIG_0_CFG_TS48_READ_FIELD =
{
    "CFG_TS48_READ",
#if RU_INCLUDE_DESC
    "",
    "The TS48 will be captured on the rising edge of this signal.",
#endif
    TOD_CONFIG_0_CFG_TS48_READ_FIELD_MASK,
    0,
    TOD_CONFIG_0_CFG_TS48_READ_FIELD_WIDTH,
    TOD_CONFIG_0_CFG_TS48_READ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TOD_CONFIG_0_CFG_TS48_OFFSET
 ******************************************************************************/
const ru_field_rec TOD_CONFIG_0_CFG_TS48_OFFSET_FIELD =
{
    "CFG_TS48_OFFSET",
#if RU_INCLUDE_DESC
    "",
    "The lower 10-bits of the timestamp, to be applied after"
    "synchronizing to the 250 MHz domain.",
#endif
    TOD_CONFIG_0_CFG_TS48_OFFSET_FIELD_MASK,
    0,
    TOD_CONFIG_0_CFG_TS48_OFFSET_FIELD_WIDTH,
    TOD_CONFIG_0_CFG_TS48_OFFSET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TOD_CONFIG_0_CFG_TS48_ENABLE
 ******************************************************************************/
const ru_field_rec TOD_CONFIG_0_CFG_TS48_ENABLE_FIELD =
{
    "CFG_TS48_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "All TS48 config signals will be sampled on the rising edge of this"
    "signal.  To change any of the other configuration fields, software"
    "must clear and assert this bit again.",
#endif
    TOD_CONFIG_0_CFG_TS48_ENABLE_FIELD_MASK,
    0,
    TOD_CONFIG_0_CFG_TS48_ENABLE_FIELD_WIDTH,
    TOD_CONFIG_0_CFG_TS48_ENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TOD_CONFIG_0_CFG_TS48_MAC_SELECT
 ******************************************************************************/
const ru_field_rec TOD_CONFIG_0_CFG_TS48_MAC_SELECT_FIELD =
{
    "CFG_TS48_MAC_SELECT",
#if RU_INCLUDE_DESC
    "",
    "This field selects the MAC that the timestamp comes from."
    "0: EPON"
    "1: 10G EPON"
    "2: GPON"
    "3: NGPON"
    "4: Active Ethernet"
    "5-7: Reserved",
#endif
    TOD_CONFIG_0_CFG_TS48_MAC_SELECT_FIELD_MASK,
    0,
    TOD_CONFIG_0_CFG_TS48_MAC_SELECT_FIELD_WIDTH,
    TOD_CONFIG_0_CFG_TS48_MAC_SELECT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TOD_MSB_RESERVED0
 ******************************************************************************/
const ru_field_rec TOD_MSB_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    TOD_MSB_RESERVED0_FIELD_MASK,
    0,
    TOD_MSB_RESERVED0_FIELD_WIDTH,
    TOD_MSB_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: TOD_MSB_TS48_WAN_READ_MSB
 ******************************************************************************/
const ru_field_rec TOD_MSB_TS48_WAN_READ_MSB_FIELD =
{
    "TS48_WAN_READ_MSB",
#if RU_INCLUDE_DESC
    "",
    "Upper 16-bits of TS48.",
#endif
    TOD_MSB_TS48_WAN_READ_MSB_FIELD_MASK,
    0,
    TOD_MSB_TS48_WAN_READ_MSB_FIELD_WIDTH,
    TOD_MSB_TS48_WAN_READ_MSB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: TOD_LSB_TS48_WAN_READ_LSB
 ******************************************************************************/
const ru_field_rec TOD_LSB_TS48_WAN_READ_LSB_FIELD =
{
    "TS48_WAN_READ_LSB",
#if RU_INCLUDE_DESC
    "",
    "Lower 32-bits of TS48.",
#endif
    TOD_LSB_TS48_WAN_READ_LSB_FIELD_MASK,
    0,
    TOD_LSB_TS48_WAN_READ_LSB_FIELD_WIDTH,
    TOD_LSB_TS48_WAN_READ_LSB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: TOD_CONFIG_2_RESERVED0
 ******************************************************************************/
const ru_field_rec TOD_CONFIG_2_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    TOD_CONFIG_2_RESERVED0_FIELD_MASK,
    0,
    TOD_CONFIG_2_RESERVED0_FIELD_WIDTH,
    TOD_CONFIG_2_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TOD_CONFIG_2_CFG_TX_OFFSET
 ******************************************************************************/
const ru_field_rec TOD_CONFIG_2_CFG_TX_OFFSET_FIELD =
{
    "CFG_TX_OFFSET",
#if RU_INCLUDE_DESC
    "",
    "The lower 10-bits of the TX clock timestamp, to be applied after"
    "synchronizing to the 250 MHz domain.",
#endif
    TOD_CONFIG_2_CFG_TX_OFFSET_FIELD_MASK,
    0,
    TOD_CONFIG_2_CFG_TX_OFFSET_FIELD_WIDTH,
    TOD_CONFIG_2_CFG_TX_OFFSET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TOD_CONFIG_2_RESERVED1
 ******************************************************************************/
const ru_field_rec TOD_CONFIG_2_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    TOD_CONFIG_2_RESERVED1_FIELD_MASK,
    0,
    TOD_CONFIG_2_RESERVED1_FIELD_WIDTH,
    TOD_CONFIG_2_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TOD_CONFIG_2_CFG_RX_OFFSET
 ******************************************************************************/
const ru_field_rec TOD_CONFIG_2_CFG_RX_OFFSET_FIELD =
{
    "CFG_RX_OFFSET",
#if RU_INCLUDE_DESC
    "",
    "The lower 10-bits of the RX clock timestamp, to be applied after"
    "synchronizing to the 250 MHz domain.",
#endif
    TOD_CONFIG_2_CFG_RX_OFFSET_FIELD_MASK,
    0,
    TOD_CONFIG_2_CFG_RX_OFFSET_FIELD_WIDTH,
    TOD_CONFIG_2_CFG_RX_OFFSET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TOD_CONFIG_3_CFG_TS48_FIFO_DIS
 ******************************************************************************/
const ru_field_rec TOD_CONFIG_3_CFG_TS48_FIFO_DIS_FIELD =
{
    "CFG_TS48_FIFO_DIS",
#if RU_INCLUDE_DESC
    "",
    "cfg_ts48_fifo_dis",
#endif
    TOD_CONFIG_3_CFG_TS48_FIFO_DIS_FIELD_MASK,
    0,
    TOD_CONFIG_3_CFG_TS48_FIFO_DIS_FIELD_WIDTH,
    TOD_CONFIG_3_CFG_TS48_FIFO_DIS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TOD_CONFIG_3_RESERVED0
 ******************************************************************************/
const ru_field_rec TOD_CONFIG_3_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    TOD_CONFIG_3_RESERVED0_FIELD_MASK,
    0,
    TOD_CONFIG_3_RESERVED0_FIELD_WIDTH,
    TOD_CONFIG_3_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TOD_CONFIG_3_CFG_TS48_FIFO_LD_RATE
 ******************************************************************************/
const ru_field_rec TOD_CONFIG_3_CFG_TS48_FIFO_LD_RATE_FIELD =
{
    "CFG_TS48_FIFO_LD_RATE",
#if RU_INCLUDE_DESC
    "",
    "cfg_ts48_fifo_ld_rate",
#endif
    TOD_CONFIG_3_CFG_TS48_FIFO_LD_RATE_FIELD_MASK,
    0,
    TOD_CONFIG_3_CFG_TS48_FIFO_LD_RATE_FIELD_WIDTH,
    TOD_CONFIG_3_CFG_TS48_FIFO_LD_RATE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TOD_CONFIG_3_RESERVED1
 ******************************************************************************/
const ru_field_rec TOD_CONFIG_3_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    TOD_CONFIG_3_RESERVED1_FIELD_MASK,
    0,
    TOD_CONFIG_3_RESERVED1_FIELD_WIDTH,
    TOD_CONFIG_3_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TOD_CONFIG_3_CFG_REF_OFFSET
 ******************************************************************************/
const ru_field_rec TOD_CONFIG_3_CFG_REF_OFFSET_FIELD =
{
    "CFG_REF_OFFSET",
#if RU_INCLUDE_DESC
    "",
    "The lower 10-bits of the REF clock timestamp, to be applied after"
    "synchronizing to the 250 MHz domain.",
#endif
    TOD_CONFIG_3_CFG_REF_OFFSET_FIELD_MASK,
    0,
    TOD_CONFIG_3_CFG_REF_OFFSET_FIELD_WIDTH,
    TOD_CONFIG_3_CFG_REF_OFFSET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TOD_STATUS_0_TS16_TX_READ
 ******************************************************************************/
const ru_field_rec TOD_STATUS_0_TS16_TX_READ_FIELD =
{
    "TS16_TX_READ",
#if RU_INCLUDE_DESC
    "",
    "TX clock timestamp.",
#endif
    TOD_STATUS_0_TS16_TX_READ_FIELD_MASK,
    0,
    TOD_STATUS_0_TS16_TX_READ_FIELD_WIDTH,
    TOD_STATUS_0_TS16_TX_READ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: TOD_STATUS_0_TS16_RX_READ
 ******************************************************************************/
const ru_field_rec TOD_STATUS_0_TS16_RX_READ_FIELD =
{
    "TS16_RX_READ",
#if RU_INCLUDE_DESC
    "",
    "RX clock timestamp.",
#endif
    TOD_STATUS_0_TS16_RX_READ_FIELD_MASK,
    0,
    TOD_STATUS_0_TS16_RX_READ_FIELD_WIDTH,
    TOD_STATUS_0_TS16_RX_READ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: TOD_STATUS_1_TS16_RX_SYNCE_READ
 ******************************************************************************/
const ru_field_rec TOD_STATUS_1_TS16_RX_SYNCE_READ_FIELD =
{
    "TS16_RX_SYNCE_READ",
#if RU_INCLUDE_DESC
    "",
    "RX clock timestamp.",
#endif
    TOD_STATUS_1_TS16_RX_SYNCE_READ_FIELD_MASK,
    0,
    TOD_STATUS_1_TS16_RX_SYNCE_READ_FIELD_WIDTH,
    TOD_STATUS_1_TS16_RX_SYNCE_READ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: TOD_STATUS_1_TS16_REF_SYNCE_READ
 ******************************************************************************/
const ru_field_rec TOD_STATUS_1_TS16_REF_SYNCE_READ_FIELD =
{
    "TS16_REF_SYNCE_READ",
#if RU_INCLUDE_DESC
    "",
    "REF clock timestamp.",
#endif
    TOD_STATUS_1_TS16_REF_SYNCE_READ_FIELD_MASK,
    0,
    TOD_STATUS_1_TS16_REF_SYNCE_READ_FIELD_WIDTH,
    TOD_STATUS_1_TS16_REF_SYNCE_READ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: TOD_STATUS_2_TS16_MAC_TX_READ
 ******************************************************************************/
const ru_field_rec TOD_STATUS_2_TS16_MAC_TX_READ_FIELD =
{
    "TS16_MAC_TX_READ",
#if RU_INCLUDE_DESC
    "",
    "TX MAC clock timestamp.",
#endif
    TOD_STATUS_2_TS16_MAC_TX_READ_FIELD_MASK,
    0,
    TOD_STATUS_2_TS16_MAC_TX_READ_FIELD_WIDTH,
    TOD_STATUS_2_TS16_MAC_TX_READ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: TOD_STATUS_2_TS16_MAC_RX_READ
 ******************************************************************************/
const ru_field_rec TOD_STATUS_2_TS16_MAC_RX_READ_FIELD =
{
    "TS16_MAC_RX_READ",
#if RU_INCLUDE_DESC
    "",
    "RX MAC clock timestamp.",
#endif
    TOD_STATUS_2_TS16_MAC_RX_READ_FIELD_MASK,
    0,
    TOD_STATUS_2_TS16_MAC_RX_READ_FIELD_WIDTH,
    TOD_STATUS_2_TS16_MAC_RX_READ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: TOD_CONFIG_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TOD_CONFIG_0_FIELDS[] =
{
    &TOD_CONFIG_0_RESERVED0_FIELD,
    &TOD_CONFIG_0_CFG_TS48_READ_FIELD,
    &TOD_CONFIG_0_CFG_TS48_OFFSET_FIELD,
    &TOD_CONFIG_0_CFG_TS48_ENABLE_FIELD,
    &TOD_CONFIG_0_CFG_TS48_MAC_SELECT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec TOD_CONFIG_0_REG = 
{
    "CONFIG_0",
#if RU_INCLUDE_DESC
    "WAN_TOP_TOD_CONFIG_0 Register",
    "Register used for 48-bit timestamp Time Of Day (TOD) configuration.",
#endif
    TOD_CONFIG_0_REG_OFFSET,
    0,
    0,
    17,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    TOD_CONFIG_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: TOD_MSB
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TOD_MSB_FIELDS[] =
{
    &TOD_MSB_RESERVED0_FIELD,
    &TOD_MSB_TS48_WAN_READ_MSB_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec TOD_MSB_REG = 
{
    "MSB",
#if RU_INCLUDE_DESC
    "WAN_TOP_TOD_MSB Register",
    "Register used for 48-bit timestamp Time Of Day (TOD) read back.",
#endif
    TOD_MSB_REG_OFFSET,
    0,
    0,
    18,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    TOD_MSB_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: TOD_LSB
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TOD_LSB_FIELDS[] =
{
    &TOD_LSB_TS48_WAN_READ_LSB_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec TOD_LSB_REG = 
{
    "LSB",
#if RU_INCLUDE_DESC
    "WAN_TOP_TOD_LSB Register",
    "Register used for 48-bit timestamp Time Of Day (TOD) read back.",
#endif
    TOD_LSB_REG_OFFSET,
    0,
    0,
    19,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TOD_LSB_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: TOD_CONFIG_2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TOD_CONFIG_2_FIELDS[] =
{
    &TOD_CONFIG_2_RESERVED0_FIELD,
    &TOD_CONFIG_2_CFG_TX_OFFSET_FIELD,
    &TOD_CONFIG_2_RESERVED1_FIELD,
    &TOD_CONFIG_2_CFG_RX_OFFSET_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec TOD_CONFIG_2_REG = 
{
    "CONFIG_2",
#if RU_INCLUDE_DESC
    "WAN_TOP_TOD_CONFIG_2 Register",
    "Register used for 16-bit timestamp configuration.",
#endif
    TOD_CONFIG_2_REG_OFFSET,
    0,
    0,
    20,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    TOD_CONFIG_2_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: TOD_CONFIG_3
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TOD_CONFIG_3_FIELDS[] =
{
    &TOD_CONFIG_3_CFG_TS48_FIFO_DIS_FIELD,
    &TOD_CONFIG_3_RESERVED0_FIELD,
    &TOD_CONFIG_3_CFG_TS48_FIFO_LD_RATE_FIELD,
    &TOD_CONFIG_3_RESERVED1_FIELD,
    &TOD_CONFIG_3_CFG_REF_OFFSET_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec TOD_CONFIG_3_REG = 
{
    "CONFIG_3",
#if RU_INCLUDE_DESC
    "WAN_TOP_TOD_CONFIG_3 Register",
    "Register used for 16-bit timestamp configuration.",
#endif
    TOD_CONFIG_3_REG_OFFSET,
    0,
    0,
    21,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    TOD_CONFIG_3_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: TOD_STATUS_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TOD_STATUS_0_FIELDS[] =
{
    &TOD_STATUS_0_TS16_TX_READ_FIELD,
    &TOD_STATUS_0_TS16_RX_READ_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec TOD_STATUS_0_REG = 
{
    "STATUS_0",
#if RU_INCLUDE_DESC
    "WAN_TOP_TOD_STATUS_0 Register",
    "Register used for 16-bit timestamp read back.",
#endif
    TOD_STATUS_0_REG_OFFSET,
    0,
    0,
    22,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    TOD_STATUS_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: TOD_STATUS_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TOD_STATUS_1_FIELDS[] =
{
    &TOD_STATUS_1_TS16_RX_SYNCE_READ_FIELD,
    &TOD_STATUS_1_TS16_REF_SYNCE_READ_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec TOD_STATUS_1_REG = 
{
    "STATUS_1",
#if RU_INCLUDE_DESC
    "WAN_TOP_TOD_STATUS_1 Register",
    "Register used for 16-bit timestamp read back.",
#endif
    TOD_STATUS_1_REG_OFFSET,
    0,
    0,
    23,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    TOD_STATUS_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: TOD_STATUS_2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TOD_STATUS_2_FIELDS[] =
{
    &TOD_STATUS_2_TS16_MAC_TX_READ_FIELD,
    &TOD_STATUS_2_TS16_MAC_RX_READ_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec TOD_STATUS_2_REG = 
{
    "STATUS_2",
#if RU_INCLUDE_DESC
    "WAN_TOP_TOD_STATUS_2 Register",
    "Register used for 16-bit timestamp read back.",
#endif
    TOD_STATUS_2_REG_OFFSET,
    0,
    0,
    24,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    TOD_STATUS_2_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: TOD
 ******************************************************************************/
static const ru_reg_rec *TOD_REGS[] =
{
    &TOD_CONFIG_0_REG,
    &TOD_MSB_REG,
    &TOD_LSB_REG,
    &TOD_CONFIG_2_REG,
    &TOD_CONFIG_3_REG,
    &TOD_STATUS_0_REG,
    &TOD_STATUS_1_REG,
    &TOD_STATUS_2_REG,
};

unsigned long TOD_ADDRS[] =
{
#if defined(CONFIG_BCM963158)
    0x80144060,
#else
    #error "TOD base address not defined"
#endif
};

const ru_block_rec TOD_BLOCK = 
{
    "TOD",
    TOD_ADDRS,
    1,
    8,
    TOD_REGS
};

/* End of file TOD.c */
