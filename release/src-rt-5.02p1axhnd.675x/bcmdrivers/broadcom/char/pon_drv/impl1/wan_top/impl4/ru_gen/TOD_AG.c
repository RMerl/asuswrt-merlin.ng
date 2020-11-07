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
 * Field: TOD_CONFIG_0_TOD_READ_BUSY
 ******************************************************************************/
const ru_field_rec TOD_CONFIG_0_TOD_READ_BUSY_FIELD =
{
    "TOD_READ_BUSY",
#if RU_INCLUDE_DESC
    "",
    "Indicates TOD read is in progress.  Deassertive value indicates"
    "valid values at WAN_TOD_TS48/WAN_TOD_TS64 registers.",
#endif
    TOD_CONFIG_0_TOD_READ_BUSY_FIELD_MASK,
    0,
    TOD_CONFIG_0_TOD_READ_BUSY_FIELD_WIDTH,
    TOD_CONFIG_0_TOD_READ_BUSY_FIELD_SHIFT,
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
    "Allows 1PPS pulse to load cfg_tod_1pps_ns_offset into nanosecond"
    "counter.  If not set, the 1PPS pulse will have no effect on the"
    "TS48.",
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
 * Field: TOD_CONFIG_0_CFG_TOD_READ
 ******************************************************************************/
const ru_field_rec TOD_CONFIG_0_CFG_TOD_READ_FIELD =
{
    "CFG_TOD_READ",
#if RU_INCLUDE_DESC
    "",
    "Arm the reading of the TS48/TS64 timestamps.  Values are valid at"
    "the deassertion of tod_read_busy",
#endif
    TOD_CONFIG_0_CFG_TOD_READ_FIELD_MASK,
    0,
    TOD_CONFIG_0_CFG_TOD_READ_FIELD_WIDTH,
    TOD_CONFIG_0_CFG_TOD_READ_FIELD_SHIFT,
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
    "The TS48 offset value."
    ""
    "In legacy, GPON mode (cfg_ts48_pre_sync_fifo_disable = 1), the"
    "rising edge of"
    "TS48/TS64's bit[9] loads cfg_ts48_offset[8:0] into the lower 9 bits"
    "of the synchronized TS48/TS64."
    "In the new mode, the timestamp is transfer to the 250 MHz clock"
    "domain via an asynchronous"
    "FIFO.  The offset is added to the output of the FIFO.  The"
    "cfg_ts48_offset[8] is the sign"
    "bit, allowing +/- adjustment to the timestamp value. It is sign"
    "extended to make the"
    "offset 48-bits."
    ""
    "In AE mode, the offset is added to the current TS48 value and"
    "loading it back into AE TS48."
    "Loading is accomplished by setting the cfg_tod_load_ts48_offset bit."
    "The sign extension of"
    "cfg_ts48_offset[8] also applies.",
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
 * Field: TOD_CONFIG_0_RESERVED1
 ******************************************************************************/
const ru_field_rec TOD_CONFIG_0_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    TOD_CONFIG_0_RESERVED1_FIELD_MASK,
    0,
    TOD_CONFIG_0_RESERVED1_FIELD_WIDTH,
    TOD_CONFIG_0_RESERVED1_FIELD_SHIFT,
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
    "2: GPON"
    "4: Active Ethernet"
    "0,1,3,5,6,7: Reserved",
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
 * Field: TOD_CONFIG_1_CFG_TOD_LOAD_TS48_OFFSET
 ******************************************************************************/
const ru_field_rec TOD_CONFIG_1_CFG_TOD_LOAD_TS48_OFFSET_FIELD =
{
    "CFG_TOD_LOAD_TS48_OFFSET",
#if RU_INCLUDE_DESC
    "",
    "The rising edge will load the cfg_ts48_offset into AE TS48, subject"
    "to a lockout window of 1us before and after rollover.",
#endif
    TOD_CONFIG_1_CFG_TOD_LOAD_TS48_OFFSET_FIELD_MASK,
    0,
    TOD_CONFIG_1_CFG_TOD_LOAD_TS48_OFFSET_FIELD_WIDTH,
    TOD_CONFIG_1_CFG_TOD_LOAD_TS48_OFFSET_FIELD_SHIFT,
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
    "into AE TS48 on the next 1PPS pulse or when the next second rolls"
    "over.",
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
    "Number of seconds to be loaded into AE TS48.",
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
    "The TS48 offset value for TX clock timestamp."
    ""
    "In legacy mode (cfg_ts48_pre_sync_fifo_disable = 1), the rising edge"
    "of cfg_ts48_offset[9]"
    "loads the lower 9 bits into the 16-bits timestamp.  In the new mode,"
    "the timestamp is"
    "transfer to the 250 MHz clock domain via an asynchronous FIFO.  The"
    "offset is added to the"
    "output of the FIFO. The cfg_ts48_offset[8] is the sign bit,"
    "allowing +/- adjustment to"
    "the timestamp value. It is sign extended to make the offset 48-bits.",
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
    "The TS48 offset value for RX clock timestamp."
    ""
    "In legacy mode (cfg_ts48_pre_sync_fifo_disable = 1), the rising edge"
    "of cfg_ts48_offset[9]"
    "loads the lower 9 bits into the 16-bits timestamp.  In the new mode,"
    "the timestamp is"
    "transfer to the 250 MHz clock domain via an asynchronous FIFO.  The"
    "offset is added to the"
    "output of the FIFO. The cfg_ts48_offset[8] is the sign bit,"
    "allowing +/- adjustment to"
    "the timestamp value. It is sign extended to make the offset 48-bits.",
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
    "The TS48 offset value for RX clock timestamp."
    ""
    "In legacy mode (cfg_ts48_pre_sync_fifo_disable = 1), the rising edge"
    "of cfg_ts48_offset[9]"
    "loads the lower 9 bits into the 16-bits timestamp.  In the new mode,"
    "the timestamp is"
    "transfer to the 250 MHz clock domain via an asynchronous FIFO.  The"
    "offset is added to the"
    "output of the FIFO. The cfg_ts48_offset[8] is the sign bit,"
    "allowing +/- adjustment to"
    "the timestamp value. It is sign extended to make the offset 48-bits.",
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
 * Field: TOD_CONFIG_4_RESERVED0
 ******************************************************************************/
const ru_field_rec TOD_CONFIG_4_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    TOD_CONFIG_4_RESERVED0_FIELD_MASK,
    0,
    TOD_CONFIG_4_RESERVED0_FIELD_WIDTH,
    TOD_CONFIG_4_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TOD_CONFIG_4_CFG_TOD_1PPS_NS_OFFSET
 ******************************************************************************/
const ru_field_rec TOD_CONFIG_4_CFG_TOD_1PPS_NS_OFFSET_FIELD =
{
    "CFG_TOD_1PPS_NS_OFFSET",
#if RU_INCLUDE_DESC
    "",
    "Value to be loaded into nanosecond counter by 1PPS pulse, provided"
    "cfg_tod_pps_clear is set.",
#endif
    TOD_CONFIG_4_CFG_TOD_1PPS_NS_OFFSET_FIELD_MASK,
    0,
    TOD_CONFIG_4_CFG_TOD_1PPS_NS_OFFSET_FIELD_WIDTH,
    TOD_CONFIG_4_CFG_TOD_1PPS_NS_OFFSET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TOD_CONFIG_5_RESERVED0
 ******************************************************************************/
const ru_field_rec TOD_CONFIG_5_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    TOD_CONFIG_5_RESERVED0_FIELD_MASK,
    0,
    TOD_CONFIG_5_RESERVED0_FIELD_WIDTH,
    TOD_CONFIG_5_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TOD_CONFIG_5_CFG_TOD_LOAD_NS_OFFSET
 ******************************************************************************/
const ru_field_rec TOD_CONFIG_5_CFG_TOD_LOAD_NS_OFFSET_FIELD =
{
    "CFG_TOD_LOAD_NS_OFFSET",
#if RU_INCLUDE_DESC
    "",
    "Rising edge immediately load cfg_tod_ns_offset into nanosecond"
    "counter.  This is mainly utilized for debugging.",
#endif
    TOD_CONFIG_5_CFG_TOD_LOAD_NS_OFFSET_FIELD_MASK,
    0,
    TOD_CONFIG_5_CFG_TOD_LOAD_NS_OFFSET_FIELD_WIDTH,
    TOD_CONFIG_5_CFG_TOD_LOAD_NS_OFFSET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TOD_CONFIG_5_CFG_TOD_NS_OFFSET
 ******************************************************************************/
const ru_field_rec TOD_CONFIG_5_CFG_TOD_NS_OFFSET_FIELD =
{
    "CFG_TOD_NS_OFFSET",
#if RU_INCLUDE_DESC
    "",
    "Value to be loaded into nanosecond counter. The rollover value is at"
    "0x3B9ACA00.",
#endif
    TOD_CONFIG_5_CFG_TOD_NS_OFFSET_FIELD_MASK,
    0,
    TOD_CONFIG_5_CFG_TOD_NS_OFFSET_FIELD_WIDTH,
    TOD_CONFIG_5_CFG_TOD_NS_OFFSET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: TOD_TS48_MSB_RESERVED0
 ******************************************************************************/
const ru_field_rec TOD_TS48_MSB_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    TOD_TS48_MSB_RESERVED0_FIELD_MASK,
    0,
    TOD_TS48_MSB_RESERVED0_FIELD_WIDTH,
    TOD_TS48_MSB_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: TOD_TS48_MSB_TS48_WAN_READ_MSB
 ******************************************************************************/
const ru_field_rec TOD_TS48_MSB_TS48_WAN_READ_MSB_FIELD =
{
    "TS48_WAN_READ_MSB",
#if RU_INCLUDE_DESC
    "",
    "Upper 16-bits of TS48.",
#endif
    TOD_TS48_MSB_TS48_WAN_READ_MSB_FIELD_MASK,
    0,
    TOD_TS48_MSB_TS48_WAN_READ_MSB_FIELD_WIDTH,
    TOD_TS48_MSB_TS48_WAN_READ_MSB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: TOD_TS48_LSB_TS48_WAN_READ_LSB
 ******************************************************************************/
const ru_field_rec TOD_TS48_LSB_TS48_WAN_READ_LSB_FIELD =
{
    "TS48_WAN_READ_LSB",
#if RU_INCLUDE_DESC
    "",
    "Lower 32-bits of TS48.",
#endif
    TOD_TS48_LSB_TS48_WAN_READ_LSB_FIELD_MASK,
    0,
    TOD_TS48_LSB_TS48_WAN_READ_LSB_FIELD_WIDTH,
    TOD_TS48_LSB_TS48_WAN_READ_LSB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: TOD_TS64_MSB_TS64_WAN_READ_MSB
 ******************************************************************************/
const ru_field_rec TOD_TS64_MSB_TS64_WAN_READ_MSB_FIELD =
{
    "TS64_WAN_READ_MSB",
#if RU_INCLUDE_DESC
    "",
    "Upper value of TS64 :"
    "AE - second = ts64_wan_read_msb[18:0]"
    "GPON - second[33:2] = ts64_wan_read_msb[31:0]",
#endif
    TOD_TS64_MSB_TS64_WAN_READ_MSB_FIELD_MASK,
    0,
    TOD_TS64_MSB_TS64_WAN_READ_MSB_FIELD_WIDTH,
    TOD_TS64_MSB_TS64_WAN_READ_MSB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: TOD_TS64_LSB_TS64_WAN_READ_LSB
 ******************************************************************************/
const ru_field_rec TOD_TS64_LSB_TS64_WAN_READ_LSB_FIELD =
{
    "TS64_WAN_READ_LSB",
#if RU_INCLUDE_DESC
    "",
    "Lower value of TS64 :"
    "AE - nanosecond    = ts64_wan_read_lsb[31:0]"
    "GPON - second[1:0] = ts64_wan_read_lsb[31:30]; nanosecond ="
    "ts64_wan_read_lsb[29:0];",
#endif
    TOD_TS64_LSB_TS64_WAN_READ_LSB_FIELD_MASK,
    0,
    TOD_TS64_LSB_TS64_WAN_READ_LSB_FIELD_WIDTH,
    TOD_TS64_LSB_TS64_WAN_READ_LSB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: TOD_STATUS_0_RESERVED0
 ******************************************************************************/
const ru_field_rec TOD_STATUS_0_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    TOD_STATUS_0_RESERVED0_FIELD_MASK,
    0,
    TOD_STATUS_0_RESERVED0_FIELD_WIDTH,
    TOD_STATUS_0_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: TOD_STATUS_0_TS16_REF_SYNCE_READ
 ******************************************************************************/
const ru_field_rec TOD_STATUS_0_TS16_REF_SYNCE_READ_FIELD =
{
    "TS16_REF_SYNCE_READ",
#if RU_INCLUDE_DESC
    "",
    "REF clock timestamp.",
#endif
    TOD_STATUS_0_TS16_REF_SYNCE_READ_FIELD_MASK,
    0,
    TOD_STATUS_0_TS16_REF_SYNCE_READ_FIELD_WIDTH,
    TOD_STATUS_0_TS16_REF_SYNCE_READ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: TOD_STATUS_1_TS16_MAC_TX_READ
 ******************************************************************************/
const ru_field_rec TOD_STATUS_1_TS16_MAC_TX_READ_FIELD =
{
    "TS16_MAC_TX_READ",
#if RU_INCLUDE_DESC
    "",
    "TX MAC clock timestamp.",
#endif
    TOD_STATUS_1_TS16_MAC_TX_READ_FIELD_MASK,
    0,
    TOD_STATUS_1_TS16_MAC_TX_READ_FIELD_WIDTH,
    TOD_STATUS_1_TS16_MAC_TX_READ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: TOD_STATUS_1_TS16_MAC_RX_READ
 ******************************************************************************/
const ru_field_rec TOD_STATUS_1_TS16_MAC_RX_READ_FIELD =
{
    "TS16_MAC_RX_READ",
#if RU_INCLUDE_DESC
    "",
    "RX MAC clock timestamp.",
#endif
    TOD_STATUS_1_TS16_MAC_RX_READ_FIELD_MASK,
    0,
    TOD_STATUS_1_TS16_MAC_RX_READ_FIELD_WIDTH,
    TOD_STATUS_1_TS16_MAC_RX_READ_FIELD_SHIFT,
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
    &TOD_CONFIG_0_TOD_READ_BUSY_FIELD,
    &TOD_CONFIG_0_CFG_TS48_PRE_SYNC_FIFO_DISABLE_FIELD,
    &TOD_CONFIG_0_CFG_TS48_PRE_SYNC_FIFO_LOAD_RATE_FIELD,
    &TOD_CONFIG_0_CFG_TOD_PPS_CLEAR_FIELD,
    &TOD_CONFIG_0_CFG_TOD_READ_FIELD,
    &TOD_CONFIG_0_CFG_TS48_OFFSET_FIELD,
    &TOD_CONFIG_0_RESERVED1_FIELD,
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
    24,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
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
    &TOD_CONFIG_1_RESERVED0_FIELD,
    &TOD_CONFIG_1_CFG_TOD_LOAD_TS48_OFFSET_FIELD,
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
    25,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    TOD_CONFIG_1_FIELDS,
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
    26,
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
    27,
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
 * Register: TOD_CONFIG_4
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TOD_CONFIG_4_FIELDS[] =
{
    &TOD_CONFIG_4_RESERVED0_FIELD,
    &TOD_CONFIG_4_CFG_TOD_1PPS_NS_OFFSET_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec TOD_CONFIG_4_REG = 
{
    "CONFIG_4",
#if RU_INCLUDE_DESC
    "WAN_TOP_TOD_CONFIG_4 Register",
    "Offset for 1pps loading.",
#endif
    TOD_CONFIG_4_REG_OFFSET,
    0,
    0,
    28,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    TOD_CONFIG_4_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: TOD_CONFIG_5
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TOD_CONFIG_5_FIELDS[] =
{
    &TOD_CONFIG_5_RESERVED0_FIELD,
    &TOD_CONFIG_5_CFG_TOD_LOAD_NS_OFFSET_FIELD,
    &TOD_CONFIG_5_CFG_TOD_NS_OFFSET_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec TOD_CONFIG_5_REG = 
{
    "CONFIG_5",
#if RU_INCLUDE_DESC
    "WAN_TOP_TOD_CONFIG_5 Register",
    "Debug register, used for loading TOD nanosecond counte for rollover"
    "testing.",
#endif
    TOD_CONFIG_5_REG_OFFSET,
    0,
    0,
    29,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    TOD_CONFIG_5_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: TOD_TS48_MSB
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TOD_TS48_MSB_FIELDS[] =
{
    &TOD_TS48_MSB_RESERVED0_FIELD,
    &TOD_TS48_MSB_TS48_WAN_READ_MSB_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec TOD_TS48_MSB_REG = 
{
    "TS48_MSB",
#if RU_INCLUDE_DESC
    "WAN_TOD_TS48_MSB Register",
    "Register used for 48-bit timestamp Time Of Day (TOD) read back.",
#endif
    TOD_TS48_MSB_REG_OFFSET,
    0,
    0,
    30,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    TOD_TS48_MSB_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: TOD_TS48_LSB
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TOD_TS48_LSB_FIELDS[] =
{
    &TOD_TS48_LSB_TS48_WAN_READ_LSB_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec TOD_TS48_LSB_REG = 
{
    "TS48_LSB",
#if RU_INCLUDE_DESC
    "WAN_TOD_TS48_LSB Register",
    "Register used for 48-bit timestamp Time Of Day (TOD) read back.",
#endif
    TOD_TS48_LSB_REG_OFFSET,
    0,
    0,
    31,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TOD_TS48_LSB_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: TOD_TS64_MSB
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TOD_TS64_MSB_FIELDS[] =
{
    &TOD_TS64_MSB_TS64_WAN_READ_MSB_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec TOD_TS64_MSB_REG = 
{
    "TS64_MSB",
#if RU_INCLUDE_DESC
    "WAN_TOD_TS64_MSB Register",
    "Register used for 64-bit timestamp Time Of Day (TOD) read back.",
#endif
    TOD_TS64_MSB_REG_OFFSET,
    0,
    0,
    32,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TOD_TS64_MSB_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: TOD_TS64_LSB
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TOD_TS64_LSB_FIELDS[] =
{
    &TOD_TS64_LSB_TS64_WAN_READ_LSB_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec TOD_TS64_LSB_REG = 
{
    "TS64_LSB",
#if RU_INCLUDE_DESC
    "WAN_TOD_TS64_LSB Register",
    "Register used for 64-bit timestamp Time Of Day (TOD) read back.",
#endif
    TOD_TS64_LSB_REG_OFFSET,
    0,
    0,
    33,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    TOD_TS64_LSB_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: TOD_STATUS_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *TOD_STATUS_0_FIELDS[] =
{
    &TOD_STATUS_0_RESERVED0_FIELD,
    &TOD_STATUS_0_TS16_REF_SYNCE_READ_FIELD,
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
    34,
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
    &TOD_STATUS_1_TS16_MAC_TX_READ_FIELD,
    &TOD_STATUS_1_TS16_MAC_RX_READ_FIELD,
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
    35,
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
 * Block: TOD
 ******************************************************************************/
static const ru_reg_rec *TOD_REGS[] =
{
    &TOD_CONFIG_0_REG,
    &TOD_CONFIG_1_REG,
    &TOD_CONFIG_2_REG,
    &TOD_CONFIG_3_REG,
    &TOD_CONFIG_4_REG,
    &TOD_CONFIG_5_REG,
    &TOD_TS48_MSB_REG,
    &TOD_TS48_LSB_REG,
    &TOD_TS64_MSB_REG,
    &TOD_TS64_LSB_REG,
    &TOD_STATUS_0_REG,
    &TOD_STATUS_1_REG,
};

unsigned long TOD_ADDRS[] =
{
    0x80144060,
};

const ru_block_rec TOD_BLOCK = 
{
    "TOD",
    TOD_ADDRS,
    1,
    12,
    TOD_REGS
};

/* End of file TOD.c */
