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
 * Field: TOD_CONFIG_0_CFG_TS48_PRE_SYNC_FIFO_DISABLE
 ******************************************************************************/
const ru_field_rec TOD_CONFIG_0_CFG_TS48_PRE_SYNC_FIFO_DISABLE_FIELD =
{
    "CFG_TS48_PRE_SYNC_FIFO_DISABLE",
#if RU_INCLUDE_DESC
    "",
    "0: New mode. Transfer TS48 using FIFO. 1: Legacy mode.  Transfer"
    "upper TS48 bits between clock domains.",
#endif
    TOD_CONFIG_0_CFG_TS48_PRE_SYNC_FIFO_DISABLE_FIELD_MASK,
    0,
    TOD_CONFIG_0_CFG_TS48_PRE_SYNC_FIFO_DISABLE_FIELD_WIDTH,
    TOD_CONFIG_0_CFG_TS48_PRE_SYNC_FIFO_DISABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TOD_CONFIG_0_CFG_TS48_PRE_SYNC_FIFO_LOAD_RATE
 ******************************************************************************/
const ru_field_rec TOD_CONFIG_0_CFG_TS48_PRE_SYNC_FIFO_LOAD_RATE_FIELD =
{
    "CFG_TS48_PRE_SYNC_FIFO_LOAD_RATE",
#if RU_INCLUDE_DESC
    "",
    "Number of clock ticks between consecutive writes to the TS48 FIFO.",
#endif
    TOD_CONFIG_0_CFG_TS48_PRE_SYNC_FIFO_LOAD_RATE_FIELD_MASK,
    0,
    TOD_CONFIG_0_CFG_TS48_PRE_SYNC_FIFO_LOAD_RATE_FIELD_WIDTH,
    TOD_CONFIG_0_CFG_TS48_PRE_SYNC_FIFO_LOAD_RATE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TOD_CONFIG_0_CFG_TOD_PPS_CLEAR
 ******************************************************************************/
const ru_field_rec TOD_CONFIG_0_CFG_TOD_PPS_CLEAR_FIELD =
{
    "CFG_TOD_PPS_CLEAR",
#if RU_INCLUDE_DESC
    "",
    "Allow 1PPS pulse to clear the counter if set.  If not set, the 1PPS"
    "pulse will have no effect on the TS48.",
#endif
    TOD_CONFIG_0_CFG_TOD_PPS_CLEAR_FIELD_MASK,
    0,
    TOD_CONFIG_0_CFG_TOD_PPS_CLEAR_FIELD_WIDTH,
    TOD_CONFIG_0_CFG_TOD_PPS_CLEAR_FIELD_SHIFT,
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
 * Field: TOD_CONFIG_1_CFG_TOD_LOAD_NS
 ******************************************************************************/
const ru_field_rec TOD_CONFIG_1_CFG_TOD_LOAD_NS_FIELD =
{
    "CFG_TOD_LOAD_NS",
#if RU_INCLUDE_DESC
    "",
    "When this bit is set, hardware will update the internal nanosecond"
    "counter, cfg_tod_ns[31:0], when the local MPCP time equals"
    "cfg_tod_mpcp[31:0]. Software should set this bit and wait until"
    "hardware clears it before setting it again.",
#endif
    TOD_CONFIG_1_CFG_TOD_LOAD_NS_FIELD_MASK,
    0,
    TOD_CONFIG_1_CFG_TOD_LOAD_NS_FIELD_WIDTH,
    TOD_CONFIG_1_CFG_TOD_LOAD_NS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TOD_CONFIG_1_CFG_TOD_EPON_READ
 ******************************************************************************/
const ru_field_rec TOD_CONFIG_1_CFG_TOD_EPON_READ_FIELD =
{
    "CFG_TOD_EPON_READ",
#if RU_INCLUDE_DESC
    "",
    "When this bit is set, hardware will latch the internal ts48, ns, and"
    "seconds counters. Software should set this bit and wait until"
    "hardware clears it before setting it again. Once hardware has"
    "cleared the bit, the timers are available to be read.",
#endif
    TOD_CONFIG_1_CFG_TOD_EPON_READ_FIELD_MASK,
    0,
    TOD_CONFIG_1_CFG_TOD_EPON_READ_FIELD_WIDTH,
    TOD_CONFIG_1_CFG_TOD_EPON_READ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TOD_CONFIG_1_CFG_TOD_EPON_READ_SEL
 ******************************************************************************/
const ru_field_rec TOD_CONFIG_1_CFG_TOD_EPON_READ_SEL_FIELD =
{
    "CFG_TOD_EPON_READ_SEL",
#if RU_INCLUDE_DESC
    "",
    "Select the block to read the timers from.  0: Reserved. 1: 1G EPON."
    "2:10G EPON. 3: AE. This field should not be changed while"
    "cfg_tod_read is set.",
#endif
    TOD_CONFIG_1_CFG_TOD_EPON_READ_SEL_FIELD_MASK,
    0,
    TOD_CONFIG_1_CFG_TOD_EPON_READ_SEL_FIELD_WIDTH,
    TOD_CONFIG_1_CFG_TOD_EPON_READ_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TOD_CONFIG_1_RESERVED0
 ******************************************************************************/
const ru_field_rec TOD_CONFIG_1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    TOD_CONFIG_1_RESERVED0_FIELD_MASK,
    0,
    TOD_CONFIG_1_RESERVED0_FIELD_WIDTH,
    TOD_CONFIG_1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TOD_CONFIG_1_CFG_TOD_LOAD
 ******************************************************************************/
const ru_field_rec TOD_CONFIG_1_CFG_TOD_LOAD_FIELD =
{
    "CFG_TOD_LOAD",
#if RU_INCLUDE_DESC
    "",
    "The rising edge will be latched, and cfg_tod_seconds will be loaded"
    "on the next 1PPS pulse or when the next second rolls over.",
#endif
    TOD_CONFIG_1_CFG_TOD_LOAD_FIELD_MASK,
    0,
    TOD_CONFIG_1_CFG_TOD_LOAD_FIELD_WIDTH,
    TOD_CONFIG_1_CFG_TOD_LOAD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TOD_CONFIG_1_CFG_TOD_SECONDS
 ******************************************************************************/
const ru_field_rec TOD_CONFIG_1_CFG_TOD_SECONDS_FIELD =
{
    "CFG_TOD_SECONDS",
#if RU_INCLUDE_DESC
    "",
    "Number of seconds to be loaded.",
#endif
    TOD_CONFIG_1_CFG_TOD_SECONDS_FIELD_MASK,
    0,
    TOD_CONFIG_1_CFG_TOD_SECONDS_FIELD_WIDTH,
    TOD_CONFIG_1_CFG_TOD_SECONDS_FIELD_SHIFT,
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

/******************************************************************************
 * Field: TOD_NS_CFG_TOD_NS
 ******************************************************************************/
const ru_field_rec TOD_NS_CFG_TOD_NS_FIELD =
{
    "CFG_TOD_NS",
#if RU_INCLUDE_DESC
    "",
    "Value to be loaded when the MPCP time reaches cfg_tod_mpcp. This"
    "field should not be updated while cfg_tod_load_ns is set.",
#endif
    TOD_NS_CFG_TOD_NS_FIELD_MASK,
    0,
    TOD_NS_CFG_TOD_NS_FIELD_WIDTH,
    TOD_NS_CFG_TOD_NS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TOD_MPCP_CFG_TOD_MPCP
 ******************************************************************************/
const ru_field_rec TOD_MPCP_CFG_TOD_MPCP_FIELD =
{
    "CFG_TOD_MPCP",
#if RU_INCLUDE_DESC
    "",
    "MPCP value to wait for before loading cfg_tod_ns.",
#endif
    TOD_MPCP_CFG_TOD_MPCP_FIELD_MASK,
    0,
    TOD_MPCP_CFG_TOD_MPCP_FIELD_WIDTH,
    TOD_MPCP_CFG_TOD_MPCP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
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
    &TOD_CONFIG_0_CFG_TS48_PRE_SYNC_FIFO_DISABLE_FIELD,
    &TOD_CONFIG_0_CFG_TS48_PRE_SYNC_FIFO_LOAD_RATE_FIELD,
    &TOD_CONFIG_0_CFG_TOD_PPS_CLEAR_FIELD,
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
    40,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    TOD_CONFIG_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: TOD_CONFIG_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TOD_CONFIG_1_FIELDS[] =
{
    &TOD_CONFIG_1_CFG_TOD_LOAD_NS_FIELD,
    &TOD_CONFIG_1_CFG_TOD_EPON_READ_FIELD,
    &TOD_CONFIG_1_CFG_TOD_EPON_READ_SEL_FIELD,
    &TOD_CONFIG_1_RESERVED0_FIELD,
    &TOD_CONFIG_1_CFG_TOD_LOAD_FIELD,
    &TOD_CONFIG_1_CFG_TOD_SECONDS_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec TOD_CONFIG_1_REG = 
{
    "CONFIG_1",
#if RU_INCLUDE_DESC
    "WAN_TOP_TOD_CONFIG_1 Register",
    "Register used for 48-bit timestamp Time Of Day (TOD) configuration.",
#endif
    TOD_CONFIG_1_REG_OFFSET,
    0,
    0,
    41,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    TOD_CONFIG_1_FIELDS,
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
    42,
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
    43,
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
    44,
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
    &TOD_CONFIG_3_RESERVED0_FIELD,
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
    45,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
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
    46,
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
    47,
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
    48,
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
 * Register: TOD_NS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TOD_NS_FIELDS[] =
{
    &TOD_NS_CFG_TOD_NS_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec TOD_NS_REG = 
{
    "NS",
#if RU_INCLUDE_DESC
    "WAN_TOP_TOD_NS Register",
    "Register used to load nanosecond counter.",
#endif
    TOD_NS_REG_OFFSET,
    0,
    0,
    49,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TOD_NS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: TOD_MPCP
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TOD_MPCP_FIELDS[] =
{
    &TOD_MPCP_CFG_TOD_MPCP_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec TOD_MPCP_REG = 
{
    "MPCP",
#if RU_INCLUDE_DESC
    "WAN_TOP_TOD_MPCP Register",
    "Register used to hold MPCP value that will be used to determine when"
    "the nanosecond counter is updated.  This field should not be updated"
    "while cfg_tod_load_ns is set.",
#endif
    TOD_MPCP_REG_OFFSET,
    0,
    0,
    50,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TOD_MPCP_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: TOD
 ******************************************************************************/
static const ru_reg_rec *TOD_REGS[] =
{
    &TOD_CONFIG_0_REG,
    &TOD_CONFIG_1_REG,
    &TOD_MSB_REG,
    &TOD_LSB_REG,
    &TOD_CONFIG_2_REG,
    &TOD_CONFIG_3_REG,
    &TOD_STATUS_0_REG,
    &TOD_STATUS_1_REG,
    &TOD_STATUS_2_REG,
    &TOD_NS_REG,
    &TOD_MPCP_REG,
};

unsigned long TOD_ADDRS[] =
{
    0x8014406c,
};

const ru_block_rec TOD_BLOCK = 
{
    "TOD",
    TOD_ADDRS,
    1,
    11,
    TOD_REGS
};

/* End of file TOD.c */
