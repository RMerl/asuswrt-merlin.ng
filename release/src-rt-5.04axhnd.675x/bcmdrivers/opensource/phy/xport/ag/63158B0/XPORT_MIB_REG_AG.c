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
 * Field: XPORT_MIB_REG_DIR_ACC_DATA_WRITE_WRITE_DATA
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_DIR_ACC_DATA_WRITE_WRITE_DATA_FIELD =
{
    "WRITE_DATA",
#if RU_INCLUDE_DESC
    "",
    "Direct register access data write register, bits [63:32]. "
    "Used only for 64-bit register accesses.",
#endif
    XPORT_MIB_REG_DIR_ACC_DATA_WRITE_WRITE_DATA_FIELD_MASK,
    0,
    XPORT_MIB_REG_DIR_ACC_DATA_WRITE_WRITE_DATA_FIELD_WIDTH,
    XPORT_MIB_REG_DIR_ACC_DATA_WRITE_WRITE_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_DIR_ACC_DATA_READ_READ_DATA
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_DIR_ACC_DATA_READ_READ_DATA_FIELD =
{
    "READ_DATA",
#if RU_INCLUDE_DESC
    "",
    "Direct register access data read register, bits [63:32]. "
    "Used only for 64-bit register accesses.",
#endif
    XPORT_MIB_REG_DIR_ACC_DATA_READ_READ_DATA_FIELD_MASK,
    0,
    XPORT_MIB_REG_DIR_ACC_DATA_READ_READ_DATA_FIELD_WIDTH,
    XPORT_MIB_REG_DIR_ACC_DATA_READ_READ_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_INDIR_ACC_ADDR_0_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_INDIR_ACC_ADDR_0_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_MIB_REG_INDIR_ACC_ADDR_0_RESERVED0_FIELD_MASK,
    0,
    XPORT_MIB_REG_INDIR_ACC_ADDR_0_RESERVED0_FIELD_WIDTH,
    XPORT_MIB_REG_INDIR_ACC_ADDR_0_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_INDIR_ACC_ADDR_0_ERR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_INDIR_ACC_ADDR_0_ERR_FIELD =
{
    "ERR",
#if RU_INCLUDE_DESC
    "",
    "Transaction Status. When transaction completes (START_BUSY = 0 after it was set to 1) "
    "and this bit is set it indicates that register transaction completed with error.",
#endif
    XPORT_MIB_REG_INDIR_ACC_ADDR_0_ERR_FIELD_MASK,
    0,
    XPORT_MIB_REG_INDIR_ACC_ADDR_0_ERR_FIELD_WIDTH,
    XPORT_MIB_REG_INDIR_ACC_ADDR_0_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_INDIR_ACC_ADDR_0_START_BUSY
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_INDIR_ACC_ADDR_0_START_BUSY_FIELD =
{
    "START_BUSY",
#if RU_INCLUDE_DESC
    "",
    "START_BUSY, Self-clearing. CPU writes this bit to 1 in order to initiate indirect register "
    "read/write transaction. When transaction completes hardware clears this bit.",
#endif
    XPORT_MIB_REG_INDIR_ACC_ADDR_0_START_BUSY_FIELD_MASK,
    0,
    XPORT_MIB_REG_INDIR_ACC_ADDR_0_START_BUSY_FIELD_WIDTH,
    XPORT_MIB_REG_INDIR_ACC_ADDR_0_START_BUSY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_INDIR_ACC_ADDR_0_R_W
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_INDIR_ACC_ADDR_0_R_W_FIELD =
{
    "R_W",
#if RU_INCLUDE_DESC
    "",
    "Register transaction:\n"
    "0 : Register Write.\n'"
    "1 : Register Read.\n",
#endif
    XPORT_MIB_REG_INDIR_ACC_ADDR_0_R_W_FIELD_MASK,
    0,
    XPORT_MIB_REG_INDIR_ACC_ADDR_0_R_W_FIELD_WIDTH,
    XPORT_MIB_REG_INDIR_ACC_ADDR_0_R_W_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_INDIR_ACC_ADDR_0_REG_PORT_ID
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_INDIR_ACC_ADDR_0_REG_PORT_ID_FIELD =
{
    "REG_PORT_ID",
#if RU_INCLUDE_DESC
    "",
    "Register Port ID.",
#endif
    XPORT_MIB_REG_INDIR_ACC_ADDR_0_REG_PORT_ID_FIELD_MASK,
    0,
    XPORT_MIB_REG_INDIR_ACC_ADDR_0_REG_PORT_ID_FIELD_WIDTH,
    XPORT_MIB_REG_INDIR_ACC_ADDR_0_REG_PORT_ID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_INDIR_ACC_ADDR_0_REG_OFFSET
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_INDIR_ACC_ADDR_0_REG_OFFSET_FIELD =
{
    "REG_OFFSET",
#if RU_INCLUDE_DESC
    "",
    "Register offset.\n"
    "Note: Bit 7 is ignored by HW. Write it as 0.",
#endif
    XPORT_MIB_REG_INDIR_ACC_ADDR_0_REG_OFFSET_FIELD_MASK,
    0,
    XPORT_MIB_REG_INDIR_ACC_ADDR_0_REG_OFFSET_FIELD_WIDTH,
    XPORT_MIB_REG_INDIR_ACC_ADDR_0_REG_OFFSET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_INDIR_ACC_DATA_LOW_0_DATA_LOW
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_INDIR_ACC_DATA_LOW_0_DATA_LOW_FIELD =
{
    "DATA_LOW",
#if RU_INCLUDE_DESC
    "",
    "Indirect register access data register, bits [31:0].",
#endif
    XPORT_MIB_REG_INDIR_ACC_DATA_LOW_0_DATA_LOW_FIELD_MASK,
    0,
    XPORT_MIB_REG_INDIR_ACC_DATA_LOW_0_DATA_LOW_FIELD_WIDTH,
    XPORT_MIB_REG_INDIR_ACC_DATA_LOW_0_DATA_LOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_INDIR_ACC_DATA_HIGH_0_DATA_HIGH
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_INDIR_ACC_DATA_HIGH_0_DATA_HIGH_FIELD =
{
    "DATA_HIGH",
#if RU_INCLUDE_DESC
    "",
    "Indirect register access data register, bits [63:32]. "
    "Used only for 64-bit register accesses.",
#endif
    XPORT_MIB_REG_INDIR_ACC_DATA_HIGH_0_DATA_HIGH_FIELD_MASK,
    0,
    XPORT_MIB_REG_INDIR_ACC_DATA_HIGH_0_DATA_HIGH_FIELD_WIDTH,
    XPORT_MIB_REG_INDIR_ACC_DATA_HIGH_0_DATA_HIGH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_INDIR_ACC_ADDR_1_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_INDIR_ACC_ADDR_1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_MIB_REG_INDIR_ACC_ADDR_1_RESERVED0_FIELD_MASK,
    0,
    XPORT_MIB_REG_INDIR_ACC_ADDR_1_RESERVED0_FIELD_WIDTH,
    XPORT_MIB_REG_INDIR_ACC_ADDR_1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_INDIR_ACC_ADDR_1_ERR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_INDIR_ACC_ADDR_1_ERR_FIELD =
{
    "ERR",
#if RU_INCLUDE_DESC
    "",
    "Transaction Status. When transaction completes (START_BUSY = 0 after it was set to 1) "
    "and this bit is set it indicates that register transaction completed with error.",
#endif
    XPORT_MIB_REG_INDIR_ACC_ADDR_1_ERR_FIELD_MASK,
    0,
    XPORT_MIB_REG_INDIR_ACC_ADDR_1_ERR_FIELD_WIDTH,
    XPORT_MIB_REG_INDIR_ACC_ADDR_1_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_INDIR_ACC_ADDR_1_START_BUSY
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_INDIR_ACC_ADDR_1_START_BUSY_FIELD =
{
    "START_BUSY",
#if RU_INCLUDE_DESC
    "",
    "START_BUSY, Self-clearing. CPU writes this bit to 1 in order to initiate indirect register "
    "read/write transaction. When transaction completes hardware clears this bit.",
#endif
    XPORT_MIB_REG_INDIR_ACC_ADDR_1_START_BUSY_FIELD_MASK,
    0,
    XPORT_MIB_REG_INDIR_ACC_ADDR_1_START_BUSY_FIELD_WIDTH,
    XPORT_MIB_REG_INDIR_ACC_ADDR_1_START_BUSY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_INDIR_ACC_ADDR_1_R_W
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_INDIR_ACC_ADDR_1_R_W_FIELD =
{
    "R_W",
#if RU_INCLUDE_DESC
    "",
    "Register transaction:\n"
    "0 : Register Write.\n'"
    "1 : Register Read.\n",
#endif
    XPORT_MIB_REG_INDIR_ACC_ADDR_1_R_W_FIELD_MASK,
    0,
    XPORT_MIB_REG_INDIR_ACC_ADDR_1_R_W_FIELD_WIDTH,
    XPORT_MIB_REG_INDIR_ACC_ADDR_1_R_W_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_INDIR_ACC_ADDR_1_REG_PORT_ID
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_INDIR_ACC_ADDR_1_REG_PORT_ID_FIELD =
{
    "REG_PORT_ID",
#if RU_INCLUDE_DESC
    "",
    "Register Port ID.",
#endif
    XPORT_MIB_REG_INDIR_ACC_ADDR_1_REG_PORT_ID_FIELD_MASK,
    0,
    XPORT_MIB_REG_INDIR_ACC_ADDR_1_REG_PORT_ID_FIELD_WIDTH,
    XPORT_MIB_REG_INDIR_ACC_ADDR_1_REG_PORT_ID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_INDIR_ACC_ADDR_1_REG_OFFSET
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_INDIR_ACC_ADDR_1_REG_OFFSET_FIELD =
{
    "REG_OFFSET",
#if RU_INCLUDE_DESC
    "",
    "Register offset.\n"
    "Note: Bit 7 is ignored by HW. Write it as 0.",
#endif
    XPORT_MIB_REG_INDIR_ACC_ADDR_1_REG_OFFSET_FIELD_MASK,
    0,
    XPORT_MIB_REG_INDIR_ACC_ADDR_1_REG_OFFSET_FIELD_WIDTH,
    XPORT_MIB_REG_INDIR_ACC_ADDR_1_REG_OFFSET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_INDIR_ACC_DATA_LOW_1_DATA_LOW
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_INDIR_ACC_DATA_LOW_1_DATA_LOW_FIELD =
{
    "DATA_LOW",
#if RU_INCLUDE_DESC
    "",
    "Indirect register access data register, bits [31:0].",
#endif
    XPORT_MIB_REG_INDIR_ACC_DATA_LOW_1_DATA_LOW_FIELD_MASK,
    0,
    XPORT_MIB_REG_INDIR_ACC_DATA_LOW_1_DATA_LOW_FIELD_WIDTH,
    XPORT_MIB_REG_INDIR_ACC_DATA_LOW_1_DATA_LOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_INDIR_ACC_DATA_HIGH_1_DATA_HIGH
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_INDIR_ACC_DATA_HIGH_1_DATA_HIGH_FIELD =
{
    "DATA_HIGH",
#if RU_INCLUDE_DESC
    "",
    "Indirect register access data register, bits [63:32]. "
    "Used only for 64-bit register accesses.",
#endif
    XPORT_MIB_REG_INDIR_ACC_DATA_HIGH_1_DATA_HIGH_FIELD_MASK,
    0,
    XPORT_MIB_REG_INDIR_ACC_DATA_HIGH_1_DATA_HIGH_FIELD_WIDTH,
    XPORT_MIB_REG_INDIR_ACC_DATA_HIGH_1_DATA_HIGH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_CNTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_CNTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_MIB_REG_CNTRL_RESERVED0_FIELD_MASK,
    0,
    XPORT_MIB_REG_CNTRL_RESERVED0_FIELD_WIDTH,
    XPORT_MIB_REG_CNTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_CNTRL_EEE_CNT_MODE
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_CNTRL_EEE_CNT_MODE_FIELD =
{
    "EEE_CNT_MODE",
#if RU_INCLUDE_DESC
    "",
    "RX and TX EEE Duration Counter Behavior\n"
    "0 : Counter behavior is asymmetric mode (100Base-TX, for example).\n"
    "1 : Counter behavior is symmetric mode (1000Base-T, for example).",
#endif
    XPORT_MIB_REG_CNTRL_EEE_CNT_MODE_FIELD_MASK,
    0,
    XPORT_MIB_REG_CNTRL_EEE_CNT_MODE_FIELD_WIDTH,
    XPORT_MIB_REG_CNTRL_EEE_CNT_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_CNTRL_SATURATE_EN
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_CNTRL_SATURATE_EN_FIELD =
{
    "SATURATE_EN",
#if RU_INCLUDE_DESC
    "",
    "When a bit in this vector is set corresponding XLMAC port statistic counters "
    "saturate at their respective maximum values.",
#endif
    XPORT_MIB_REG_CNTRL_SATURATE_EN_FIELD_MASK,
    0,
    XPORT_MIB_REG_CNTRL_SATURATE_EN_FIELD_WIDTH,
    XPORT_MIB_REG_CNTRL_SATURATE_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_CNTRL_COR_EN
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_CNTRL_COR_EN_FIELD =
{
    "COR_EN",
#if RU_INCLUDE_DESC
    "",
    "When a bit in this vector is set corresponding XLMAC port statistic counters are clear-on-read.",
#endif
    XPORT_MIB_REG_CNTRL_COR_EN_FIELD_MASK,
    0,
    XPORT_MIB_REG_CNTRL_COR_EN_FIELD_WIDTH,
    XPORT_MIB_REG_CNTRL_COR_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_CNTRL_CNT_RST
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_CNTRL_CNT_RST_FIELD =
{
    "CNT_RST",
#if RU_INCLUDE_DESC
    "",
    "When a bit in this vector is set corresponding XLMAC port statistic counters are reset.",
#endif
    XPORT_MIB_REG_CNTRL_CNT_RST_FIELD_MASK,
    0,
    XPORT_MIB_REG_CNTRL_CNT_RST_FIELD_WIDTH,
    XPORT_MIB_REG_CNTRL_CNT_RST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_EEE_PULSE_DURATION_CNTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_EEE_PULSE_DURATION_CNTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_MIB_REG_EEE_PULSE_DURATION_CNTRL_RESERVED0_FIELD_MASK,
    0,
    XPORT_MIB_REG_EEE_PULSE_DURATION_CNTRL_RESERVED0_FIELD_WIDTH,
    XPORT_MIB_REG_EEE_PULSE_DURATION_CNTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_EEE_PULSE_DURATION_CNTRL_CNT
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_EEE_PULSE_DURATION_CNTRL_CNT_FIELD =
{
    "CNT",
#if RU_INCLUDE_DESC
    "",
    "Timer to generate 10us pulse based on 25MHz refclk. Using LFSR to count up to 250 value.",
#endif
    XPORT_MIB_REG_EEE_PULSE_DURATION_CNTRL_CNT_FIELD_MASK,
    0,
    XPORT_MIB_REG_EEE_PULSE_DURATION_CNTRL_CNT_FIELD_WIDTH,
    XPORT_MIB_REG_EEE_PULSE_DURATION_CNTRL_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_GPORT0_MAX_PKT_SIZE_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_GPORT0_MAX_PKT_SIZE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_MIB_REG_GPORT0_MAX_PKT_SIZE_RESERVED0_FIELD_MASK,
    0,
    XPORT_MIB_REG_GPORT0_MAX_PKT_SIZE_RESERVED0_FIELD_WIDTH,
    XPORT_MIB_REG_GPORT0_MAX_PKT_SIZE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_GPORT0_MAX_PKT_SIZE_MAX_PKT_SIZE
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_GPORT0_MAX_PKT_SIZE_MAX_PKT_SIZE_FIELD =
{
    "MAX_PKT_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Maximum Packet Size, defaults to 1518B. Packets over this size are counted by MIB as oversized.",
#endif
    XPORT_MIB_REG_GPORT0_MAX_PKT_SIZE_MAX_PKT_SIZE_FIELD_MASK,
    0,
    XPORT_MIB_REG_GPORT0_MAX_PKT_SIZE_MAX_PKT_SIZE_FIELD_WIDTH,
    XPORT_MIB_REG_GPORT0_MAX_PKT_SIZE_MAX_PKT_SIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_GPORT1_MAX_PKT_SIZE_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_GPORT1_MAX_PKT_SIZE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_MIB_REG_GPORT1_MAX_PKT_SIZE_RESERVED0_FIELD_MASK,
    0,
    XPORT_MIB_REG_GPORT1_MAX_PKT_SIZE_RESERVED0_FIELD_WIDTH,
    XPORT_MIB_REG_GPORT1_MAX_PKT_SIZE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_GPORT1_MAX_PKT_SIZE_MAX_PKT_SIZE
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_GPORT1_MAX_PKT_SIZE_MAX_PKT_SIZE_FIELD =
{
    "MAX_PKT_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Maximum Packet Size, defaults to 1518B. Packets over this size are counted by MIB as oversized.",
#endif
    XPORT_MIB_REG_GPORT1_MAX_PKT_SIZE_MAX_PKT_SIZE_FIELD_MASK,
    0,
    XPORT_MIB_REG_GPORT1_MAX_PKT_SIZE_MAX_PKT_SIZE_FIELD_WIDTH,
    XPORT_MIB_REG_GPORT1_MAX_PKT_SIZE_MAX_PKT_SIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_GPORT2_MAX_PKT_SIZE_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_GPORT2_MAX_PKT_SIZE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_MIB_REG_GPORT2_MAX_PKT_SIZE_RESERVED0_FIELD_MASK,
    0,
    XPORT_MIB_REG_GPORT2_MAX_PKT_SIZE_RESERVED0_FIELD_WIDTH,
    XPORT_MIB_REG_GPORT2_MAX_PKT_SIZE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_GPORT2_MAX_PKT_SIZE_MAX_PKT_SIZE
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_GPORT2_MAX_PKT_SIZE_MAX_PKT_SIZE_FIELD =
{
    "MAX_PKT_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Maximum Packet Size, defaults to 1518B. Packets over this size are counted by MIB as oversized.",
#endif
    XPORT_MIB_REG_GPORT2_MAX_PKT_SIZE_MAX_PKT_SIZE_FIELD_MASK,
    0,
    XPORT_MIB_REG_GPORT2_MAX_PKT_SIZE_MAX_PKT_SIZE_FIELD_WIDTH,
    XPORT_MIB_REG_GPORT2_MAX_PKT_SIZE_MAX_PKT_SIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_GPORT3_MAX_PKT_SIZE_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_GPORT3_MAX_PKT_SIZE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_MIB_REG_GPORT3_MAX_PKT_SIZE_RESERVED0_FIELD_MASK,
    0,
    XPORT_MIB_REG_GPORT3_MAX_PKT_SIZE_RESERVED0_FIELD_WIDTH,
    XPORT_MIB_REG_GPORT3_MAX_PKT_SIZE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_GPORT3_MAX_PKT_SIZE_MAX_PKT_SIZE
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_GPORT3_MAX_PKT_SIZE_MAX_PKT_SIZE_FIELD =
{
    "MAX_PKT_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Maximum Packet Size, defaults to 1518B. Packets over this size are counted by MIB as oversized.",
#endif
    XPORT_MIB_REG_GPORT3_MAX_PKT_SIZE_MAX_PKT_SIZE_FIELD_MASK,
    0,
    XPORT_MIB_REG_GPORT3_MAX_PKT_SIZE_MAX_PKT_SIZE_FIELD_WIDTH,
    XPORT_MIB_REG_GPORT3_MAX_PKT_SIZE_MAX_PKT_SIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_ECC_CNTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_ECC_CNTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_MIB_REG_ECC_CNTRL_RESERVED0_FIELD_MASK,
    0,
    XPORT_MIB_REG_ECC_CNTRL_RESERVED0_FIELD_WIDTH,
    XPORT_MIB_REG_ECC_CNTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_ECC_CNTRL_TX_MIB_ECC_EN
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_ECC_CNTRL_TX_MIB_ECC_EN_FIELD =
{
    "TX_MIB_ECC_EN",
#if RU_INCLUDE_DESC
    "",
    "ECC enable for Tx MIB memories.",
#endif
    XPORT_MIB_REG_ECC_CNTRL_TX_MIB_ECC_EN_FIELD_MASK,
    0,
    XPORT_MIB_REG_ECC_CNTRL_TX_MIB_ECC_EN_FIELD_WIDTH,
    XPORT_MIB_REG_ECC_CNTRL_TX_MIB_ECC_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_ECC_CNTRL_RX_MIB_ECC_EN
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_ECC_CNTRL_RX_MIB_ECC_EN_FIELD =
{
    "RX_MIB_ECC_EN",
#if RU_INCLUDE_DESC
    "",
    "ECC enable for Rx MIB memories.",
#endif
    XPORT_MIB_REG_ECC_CNTRL_RX_MIB_ECC_EN_FIELD_MASK,
    0,
    XPORT_MIB_REG_ECC_CNTRL_RX_MIB_ECC_EN_FIELD_WIDTH,
    XPORT_MIB_REG_ECC_CNTRL_RX_MIB_ECC_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_FORCE_SB_ECC_ERR_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_FORCE_SB_ECC_ERR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_MIB_REG_FORCE_SB_ECC_ERR_RESERVED0_FIELD_MASK,
    0,
    XPORT_MIB_REG_FORCE_SB_ECC_ERR_RESERVED0_FIELD_WIDTH,
    XPORT_MIB_REG_FORCE_SB_ECC_ERR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_TX_MEM3_SERR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_TX_MEM3_SERR_FIELD =
{
    "FORCE_TX_MEM3_SERR",
#if RU_INCLUDE_DESC
    "",
    "Self-clearing. "
    "Force Tx MIB memory instance 3 single bit ECC error. Do not assert together with force double bit ECC error.",
#endif
    XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_TX_MEM3_SERR_FIELD_MASK,
    0,
    XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_TX_MEM3_SERR_FIELD_WIDTH,
    XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_TX_MEM3_SERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_TX_MEM2_SERR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_TX_MEM2_SERR_FIELD =
{
    "FORCE_TX_MEM2_SERR",
#if RU_INCLUDE_DESC
    "",
    "Self-clearing. "
    "Force Tx MIB memory instance 2 single bit ECC error. Do not assert together with force double bit ECC error.",
#endif
    XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_TX_MEM2_SERR_FIELD_MASK,
    0,
    XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_TX_MEM2_SERR_FIELD_WIDTH,
    XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_TX_MEM2_SERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_TX_MEM1_SERR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_TX_MEM1_SERR_FIELD =
{
    "FORCE_TX_MEM1_SERR",
#if RU_INCLUDE_DESC
    "",
    "Self-clearing. "
    "Force Tx MIB memory instance 1 single bit ECC error. Do not assert together with force double bit ECC error.",
#endif
    XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_TX_MEM1_SERR_FIELD_MASK,
    0,
    XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_TX_MEM1_SERR_FIELD_WIDTH,
    XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_TX_MEM1_SERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_TX_MEM0_SERR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_TX_MEM0_SERR_FIELD =
{
    "FORCE_TX_MEM0_SERR",
#if RU_INCLUDE_DESC
    "",
    "Self-clearing. "
    "Force Tx MIB memory instance 0 single bit ECC error. Do not assert together with force double bit ECC error.",
#endif
    XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_TX_MEM0_SERR_FIELD_MASK,
    0,
    XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_TX_MEM0_SERR_FIELD_WIDTH,
    XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_TX_MEM0_SERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_RX_MEM4_SERR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_RX_MEM4_SERR_FIELD =
{
    "FORCE_RX_MEM4_SERR",
#if RU_INCLUDE_DESC
    "",
    "Self-clearing. "
    "Force Rx MIB memory instance 4 single bit ECC error. Do not assert together with force double bit ECC error.",
#endif
    XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_RX_MEM4_SERR_FIELD_MASK,
    0,
    XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_RX_MEM4_SERR_FIELD_WIDTH,
    XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_RX_MEM4_SERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_RX_MEM3_SERR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_RX_MEM3_SERR_FIELD =
{
    "FORCE_RX_MEM3_SERR",
#if RU_INCLUDE_DESC
    "",
    "Self-clearing. "
    "Force Rx MIB memory instance 3 single bit ECC error. Do not assert together with force double bit ECC error.",
#endif
    XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_RX_MEM3_SERR_FIELD_MASK,
    0,
    XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_RX_MEM3_SERR_FIELD_WIDTH,
    XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_RX_MEM3_SERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_RX_MEM2_SERR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_RX_MEM2_SERR_FIELD =
{
    "FORCE_RX_MEM2_SERR",
#if RU_INCLUDE_DESC
    "",
    "Self-clearing. "
    "Force Rx MIB memory instance 2 single bit ECC error. Do not assert together with force double bit ECC error.",
#endif
    XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_RX_MEM2_SERR_FIELD_MASK,
    0,
    XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_RX_MEM2_SERR_FIELD_WIDTH,
    XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_RX_MEM2_SERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_RX_MEM1_SERR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_RX_MEM1_SERR_FIELD =
{
    "FORCE_RX_MEM1_SERR",
#if RU_INCLUDE_DESC
    "",
    "Self-clearing. "
    "Force Rx MIB memory instance 1 single bit ECC error. Do not assert together with force double bit ECC error.",
#endif
    XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_RX_MEM1_SERR_FIELD_MASK,
    0,
    XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_RX_MEM1_SERR_FIELD_WIDTH,
    XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_RX_MEM1_SERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_RX_MEM0_SERR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_RX_MEM0_SERR_FIELD =
{
    "FORCE_RX_MEM0_SERR",
#if RU_INCLUDE_DESC
    "",
    "Self-clearing. "
    "Force Rx MIB memory instance 0 single bit ECC error. Do not assert together with force double bit ECC error.",
#endif
    XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_RX_MEM0_SERR_FIELD_MASK,
    0,
    XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_RX_MEM0_SERR_FIELD_WIDTH,
    XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_RX_MEM0_SERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_FORCE_DB_ECC_ERR_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_FORCE_DB_ECC_ERR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_MIB_REG_FORCE_DB_ECC_ERR_RESERVED0_FIELD_MASK,
    0,
    XPORT_MIB_REG_FORCE_DB_ECC_ERR_RESERVED0_FIELD_WIDTH,
    XPORT_MIB_REG_FORCE_DB_ECC_ERR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_TX_MEM3_DERR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_TX_MEM3_DERR_FIELD =
{
    "FORCE_TX_MEM3_DERR",
#if RU_INCLUDE_DESC
    "",
    "Self-clearing. "
    "Force Tx MIB memory instance 3 double bit ECC error. Do not assert together with force single bit ECC error.",
#endif
    XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_TX_MEM3_DERR_FIELD_MASK,
    0,
    XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_TX_MEM3_DERR_FIELD_WIDTH,
    XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_TX_MEM3_DERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_TX_MEM2_DERR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_TX_MEM2_DERR_FIELD =
{
    "FORCE_TX_MEM2_DERR",
#if RU_INCLUDE_DESC
    "",
    "Self-clearing. "
    "Force Tx MIB memory instance 2 double bit ECC error. Do not assert together with force single bit ECC error.",
#endif
    XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_TX_MEM2_DERR_FIELD_MASK,
    0,
    XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_TX_MEM2_DERR_FIELD_WIDTH,
    XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_TX_MEM2_DERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_TX_MEM1_DERR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_TX_MEM1_DERR_FIELD =
{
    "FORCE_TX_MEM1_DERR",
#if RU_INCLUDE_DESC
    "",
    "Self-clearing. "
    "Force Tx MIB memory instance 1 double bit ECC error. Do not assert together with force single bit ECC error.",
#endif
    XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_TX_MEM1_DERR_FIELD_MASK,
    0,
    XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_TX_MEM1_DERR_FIELD_WIDTH,
    XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_TX_MEM1_DERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_TX_MEM0_DERR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_TX_MEM0_DERR_FIELD =
{
    "FORCE_TX_MEM0_DERR",
#if RU_INCLUDE_DESC
    "",
    "Self-clearing. "
    "Force Tx MIB memory instance 0 double bit ECC error. Do not assert together with force single bit ECC error.",
#endif
    XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_TX_MEM0_DERR_FIELD_MASK,
    0,
    XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_TX_MEM0_DERR_FIELD_WIDTH,
    XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_TX_MEM0_DERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_RX_MEM4_DERR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_RX_MEM4_DERR_FIELD =
{
    "FORCE_RX_MEM4_DERR",
#if RU_INCLUDE_DESC
    "",
    "Self-clearing. "
    "Force Rx MIB memory instance 4 double bit ECC error. Do not assert together with force single bit ECC error.",
#endif
    XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_RX_MEM4_DERR_FIELD_MASK,
    0,
    XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_RX_MEM4_DERR_FIELD_WIDTH,
    XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_RX_MEM4_DERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_RX_MEM3_DERR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_RX_MEM3_DERR_FIELD =
{
    "FORCE_RX_MEM3_DERR",
#if RU_INCLUDE_DESC
    "",
    "Self-clearing. "
    "Force Rx MIB memory instance 3 double bit ECC error. Do not assert together with force single bit ECC error.",
#endif
    XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_RX_MEM3_DERR_FIELD_MASK,
    0,
    XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_RX_MEM3_DERR_FIELD_WIDTH,
    XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_RX_MEM3_DERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_RX_MEM2_DERR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_RX_MEM2_DERR_FIELD =
{
    "FORCE_RX_MEM2_DERR",
#if RU_INCLUDE_DESC
    "",
    "Self-clearing. "
    "Force Rx MIB memory instance 2 double bit ECC error. Do not assert together with force single bit ECC error.",
#endif
    XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_RX_MEM2_DERR_FIELD_MASK,
    0,
    XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_RX_MEM2_DERR_FIELD_WIDTH,
    XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_RX_MEM2_DERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_RX_MEM1_DERR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_RX_MEM1_DERR_FIELD =
{
    "FORCE_RX_MEM1_DERR",
#if RU_INCLUDE_DESC
    "",
    "Self-clearing. "
    "Force Rx MIB memory instance 1 double bit ECC error. Do not assert together with force single bit ECC error.",
#endif
    XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_RX_MEM1_DERR_FIELD_MASK,
    0,
    XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_RX_MEM1_DERR_FIELD_WIDTH,
    XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_RX_MEM1_DERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_RX_MEM0_DERR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_RX_MEM0_DERR_FIELD =
{
    "FORCE_RX_MEM0_DERR",
#if RU_INCLUDE_DESC
    "",
    "Self-clearing. "
    "Force Rx MIB memory instance 0 double bit ECC error. Do not assert together with force single bit ECC error.",
#endif
    XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_RX_MEM0_DERR_FIELD_MASK,
    0,
    XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_RX_MEM0_DERR_FIELD_WIDTH,
    XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_RX_MEM0_DERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_RX_MEM0_ECC_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_RX_MEM0_ECC_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_MIB_REG_RX_MEM0_ECC_STATUS_RESERVED0_FIELD_MASK,
    0,
    XPORT_MIB_REG_RX_MEM0_ECC_STATUS_RESERVED0_FIELD_WIDTH,
    XPORT_MIB_REG_RX_MEM0_ECC_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_RX_MEM0_ECC_STATUS_MEM_ADDR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_RX_MEM0_ECC_STATUS_MEM_ADDR_FIELD =
{
    "MEM_ADDR",
#if RU_INCLUDE_DESC
    "",
    "First memory address in which single bit error or double bit error is detected.",
#endif
    XPORT_MIB_REG_RX_MEM0_ECC_STATUS_MEM_ADDR_FIELD_MASK,
    0,
    XPORT_MIB_REG_RX_MEM0_ECC_STATUS_MEM_ADDR_FIELD_WIDTH,
    XPORT_MIB_REG_RX_MEM0_ECC_STATUS_MEM_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_RX_MEM0_ECC_STATUS_DOUBLE_BIT_ECC_ERR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_RX_MEM0_ECC_STATUS_DOUBLE_BIT_ECC_ERR_FIELD =
{
    "DOUBLE_BIT_ECC_ERR",
#if RU_INCLUDE_DESC
    "",
    "Double Bit Error indicates an uncorrectable error occurred.",
#endif
    XPORT_MIB_REG_RX_MEM0_ECC_STATUS_DOUBLE_BIT_ECC_ERR_FIELD_MASK,
    0,
    XPORT_MIB_REG_RX_MEM0_ECC_STATUS_DOUBLE_BIT_ECC_ERR_FIELD_WIDTH,
    XPORT_MIB_REG_RX_MEM0_ECC_STATUS_DOUBLE_BIT_ECC_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_RX_MEM0_ECC_STATUS_MULTI_ECC_ERR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_RX_MEM0_ECC_STATUS_MULTI_ECC_ERR_FIELD =
{
    "MULTI_ECC_ERR",
#if RU_INCLUDE_DESC
    "",
    "Indicates more than one single bit error or double bit error are detected.",
#endif
    XPORT_MIB_REG_RX_MEM0_ECC_STATUS_MULTI_ECC_ERR_FIELD_MASK,
    0,
    XPORT_MIB_REG_RX_MEM0_ECC_STATUS_MULTI_ECC_ERR_FIELD_WIDTH,
    XPORT_MIB_REG_RX_MEM0_ECC_STATUS_MULTI_ECC_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_RX_MEM0_ECC_STATUS_ECC_ERR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_RX_MEM0_ECC_STATUS_ECC_ERR_FIELD =
{
    "ECC_ERR",
#if RU_INCLUDE_DESC
    "",
    "Single Bit Error (correctable) or Double Bit Error (Uncorrectable) occurred.",
#endif
    XPORT_MIB_REG_RX_MEM0_ECC_STATUS_ECC_ERR_FIELD_MASK,
    0,
    XPORT_MIB_REG_RX_MEM0_ECC_STATUS_ECC_ERR_FIELD_WIDTH,
    XPORT_MIB_REG_RX_MEM0_ECC_STATUS_ECC_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_RX_MEM1_ECC_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_RX_MEM1_ECC_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_MIB_REG_RX_MEM1_ECC_STATUS_RESERVED0_FIELD_MASK,
    0,
    XPORT_MIB_REG_RX_MEM1_ECC_STATUS_RESERVED0_FIELD_WIDTH,
    XPORT_MIB_REG_RX_MEM1_ECC_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_RX_MEM1_ECC_STATUS_MEM_ADDR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_RX_MEM1_ECC_STATUS_MEM_ADDR_FIELD =
{
    "MEM_ADDR",
#if RU_INCLUDE_DESC
    "",
    "First memory address in which single bit error or double bit error is detected.",
#endif
    XPORT_MIB_REG_RX_MEM1_ECC_STATUS_MEM_ADDR_FIELD_MASK,
    0,
    XPORT_MIB_REG_RX_MEM1_ECC_STATUS_MEM_ADDR_FIELD_WIDTH,
    XPORT_MIB_REG_RX_MEM1_ECC_STATUS_MEM_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_RX_MEM1_ECC_STATUS_DOUBLE_BIT_ECC_ERR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_RX_MEM1_ECC_STATUS_DOUBLE_BIT_ECC_ERR_FIELD =
{
    "DOUBLE_BIT_ECC_ERR",
#if RU_INCLUDE_DESC
    "",
    "Double Bit Error indicates an uncorrectable error occurred.",
#endif
    XPORT_MIB_REG_RX_MEM1_ECC_STATUS_DOUBLE_BIT_ECC_ERR_FIELD_MASK,
    0,
    XPORT_MIB_REG_RX_MEM1_ECC_STATUS_DOUBLE_BIT_ECC_ERR_FIELD_WIDTH,
    XPORT_MIB_REG_RX_MEM1_ECC_STATUS_DOUBLE_BIT_ECC_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_RX_MEM1_ECC_STATUS_MULTI_ECC_ERR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_RX_MEM1_ECC_STATUS_MULTI_ECC_ERR_FIELD =
{
    "MULTI_ECC_ERR",
#if RU_INCLUDE_DESC
    "",
    "Indicates more than one single bit error or double bit error are detected.",
#endif
    XPORT_MIB_REG_RX_MEM1_ECC_STATUS_MULTI_ECC_ERR_FIELD_MASK,
    0,
    XPORT_MIB_REG_RX_MEM1_ECC_STATUS_MULTI_ECC_ERR_FIELD_WIDTH,
    XPORT_MIB_REG_RX_MEM1_ECC_STATUS_MULTI_ECC_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_RX_MEM1_ECC_STATUS_ECC_ERR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_RX_MEM1_ECC_STATUS_ECC_ERR_FIELD =
{
    "ECC_ERR",
#if RU_INCLUDE_DESC
    "",
    "Single Bit Error (correctable) or Double Bit Error (Uncorrectable) occurred.",
#endif
    XPORT_MIB_REG_RX_MEM1_ECC_STATUS_ECC_ERR_FIELD_MASK,
    0,
    XPORT_MIB_REG_RX_MEM1_ECC_STATUS_ECC_ERR_FIELD_WIDTH,
    XPORT_MIB_REG_RX_MEM1_ECC_STATUS_ECC_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_RX_MEM2_ECC_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_RX_MEM2_ECC_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_MIB_REG_RX_MEM2_ECC_STATUS_RESERVED0_FIELD_MASK,
    0,
    XPORT_MIB_REG_RX_MEM2_ECC_STATUS_RESERVED0_FIELD_WIDTH,
    XPORT_MIB_REG_RX_MEM2_ECC_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_RX_MEM2_ECC_STATUS_MEM_ADDR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_RX_MEM2_ECC_STATUS_MEM_ADDR_FIELD =
{
    "MEM_ADDR",
#if RU_INCLUDE_DESC
    "",
    "First memory address in which single bit error or double bit error is detected.",
#endif
    XPORT_MIB_REG_RX_MEM2_ECC_STATUS_MEM_ADDR_FIELD_MASK,
    0,
    XPORT_MIB_REG_RX_MEM2_ECC_STATUS_MEM_ADDR_FIELD_WIDTH,
    XPORT_MIB_REG_RX_MEM2_ECC_STATUS_MEM_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_RX_MEM2_ECC_STATUS_DOUBLE_BIT_ECC_ERR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_RX_MEM2_ECC_STATUS_DOUBLE_BIT_ECC_ERR_FIELD =
{
    "DOUBLE_BIT_ECC_ERR",
#if RU_INCLUDE_DESC
    "",
    "Double Bit Error indicates an uncorrectable error occurred.",
#endif
    XPORT_MIB_REG_RX_MEM2_ECC_STATUS_DOUBLE_BIT_ECC_ERR_FIELD_MASK,
    0,
    XPORT_MIB_REG_RX_MEM2_ECC_STATUS_DOUBLE_BIT_ECC_ERR_FIELD_WIDTH,
    XPORT_MIB_REG_RX_MEM2_ECC_STATUS_DOUBLE_BIT_ECC_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_RX_MEM2_ECC_STATUS_MULTI_ECC_ERR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_RX_MEM2_ECC_STATUS_MULTI_ECC_ERR_FIELD =
{
    "MULTI_ECC_ERR",
#if RU_INCLUDE_DESC
    "",
    "Indicates more than one single bit error or double bit error are detected.",
#endif
    XPORT_MIB_REG_RX_MEM2_ECC_STATUS_MULTI_ECC_ERR_FIELD_MASK,
    0,
    XPORT_MIB_REG_RX_MEM2_ECC_STATUS_MULTI_ECC_ERR_FIELD_WIDTH,
    XPORT_MIB_REG_RX_MEM2_ECC_STATUS_MULTI_ECC_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_RX_MEM2_ECC_STATUS_ECC_ERR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_RX_MEM2_ECC_STATUS_ECC_ERR_FIELD =
{
    "ECC_ERR",
#if RU_INCLUDE_DESC
    "",
    "Single Bit Error (correctable) or Double Bit Error (Uncorrectable) occurred.",
#endif
    XPORT_MIB_REG_RX_MEM2_ECC_STATUS_ECC_ERR_FIELD_MASK,
    0,
    XPORT_MIB_REG_RX_MEM2_ECC_STATUS_ECC_ERR_FIELD_WIDTH,
    XPORT_MIB_REG_RX_MEM2_ECC_STATUS_ECC_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_RX_MEM3_ECC_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_RX_MEM3_ECC_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_MIB_REG_RX_MEM3_ECC_STATUS_RESERVED0_FIELD_MASK,
    0,
    XPORT_MIB_REG_RX_MEM3_ECC_STATUS_RESERVED0_FIELD_WIDTH,
    XPORT_MIB_REG_RX_MEM3_ECC_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_RX_MEM3_ECC_STATUS_MEM_ADDR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_RX_MEM3_ECC_STATUS_MEM_ADDR_FIELD =
{
    "MEM_ADDR",
#if RU_INCLUDE_DESC
    "",
    "First memory address in which single bit error or double bit error is detected.",
#endif
    XPORT_MIB_REG_RX_MEM3_ECC_STATUS_MEM_ADDR_FIELD_MASK,
    0,
    XPORT_MIB_REG_RX_MEM3_ECC_STATUS_MEM_ADDR_FIELD_WIDTH,
    XPORT_MIB_REG_RX_MEM3_ECC_STATUS_MEM_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_RX_MEM3_ECC_STATUS_DOUBLE_BIT_ECC_ERR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_RX_MEM3_ECC_STATUS_DOUBLE_BIT_ECC_ERR_FIELD =
{
    "DOUBLE_BIT_ECC_ERR",
#if RU_INCLUDE_DESC
    "",
    "Double Bit Error indicates an uncorrectable error occurred.",
#endif
    XPORT_MIB_REG_RX_MEM3_ECC_STATUS_DOUBLE_BIT_ECC_ERR_FIELD_MASK,
    0,
    XPORT_MIB_REG_RX_MEM3_ECC_STATUS_DOUBLE_BIT_ECC_ERR_FIELD_WIDTH,
    XPORT_MIB_REG_RX_MEM3_ECC_STATUS_DOUBLE_BIT_ECC_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_RX_MEM3_ECC_STATUS_MULTI_ECC_ERR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_RX_MEM3_ECC_STATUS_MULTI_ECC_ERR_FIELD =
{
    "MULTI_ECC_ERR",
#if RU_INCLUDE_DESC
    "",
    "Indicates more than one single bit error or double bit error are detected.",
#endif
    XPORT_MIB_REG_RX_MEM3_ECC_STATUS_MULTI_ECC_ERR_FIELD_MASK,
    0,
    XPORT_MIB_REG_RX_MEM3_ECC_STATUS_MULTI_ECC_ERR_FIELD_WIDTH,
    XPORT_MIB_REG_RX_MEM3_ECC_STATUS_MULTI_ECC_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_RX_MEM3_ECC_STATUS_ECC_ERR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_RX_MEM3_ECC_STATUS_ECC_ERR_FIELD =
{
    "ECC_ERR",
#if RU_INCLUDE_DESC
    "",
    "Single Bit Error (correctable) or Double Bit Error (Uncorrectable) occurred.",
#endif
    XPORT_MIB_REG_RX_MEM3_ECC_STATUS_ECC_ERR_FIELD_MASK,
    0,
    XPORT_MIB_REG_RX_MEM3_ECC_STATUS_ECC_ERR_FIELD_WIDTH,
    XPORT_MIB_REG_RX_MEM3_ECC_STATUS_ECC_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_RX_MEM4_ECC_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_RX_MEM4_ECC_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_MIB_REG_RX_MEM4_ECC_STATUS_RESERVED0_FIELD_MASK,
    0,
    XPORT_MIB_REG_RX_MEM4_ECC_STATUS_RESERVED0_FIELD_WIDTH,
    XPORT_MIB_REG_RX_MEM4_ECC_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_RX_MEM4_ECC_STATUS_MEM_ADDR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_RX_MEM4_ECC_STATUS_MEM_ADDR_FIELD =
{
    "MEM_ADDR",
#if RU_INCLUDE_DESC
    "",
    "First memory address in which single bit error or double bit error is detected.",
#endif
    XPORT_MIB_REG_RX_MEM4_ECC_STATUS_MEM_ADDR_FIELD_MASK,
    0,
    XPORT_MIB_REG_RX_MEM4_ECC_STATUS_MEM_ADDR_FIELD_WIDTH,
    XPORT_MIB_REG_RX_MEM4_ECC_STATUS_MEM_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_RX_MEM4_ECC_STATUS_DOUBLE_BIT_ECC_ERR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_RX_MEM4_ECC_STATUS_DOUBLE_BIT_ECC_ERR_FIELD =
{
    "DOUBLE_BIT_ECC_ERR",
#if RU_INCLUDE_DESC
    "",
    "Double Bit Error indicates an uncorrectable error occurred.",
#endif
    XPORT_MIB_REG_RX_MEM4_ECC_STATUS_DOUBLE_BIT_ECC_ERR_FIELD_MASK,
    0,
    XPORT_MIB_REG_RX_MEM4_ECC_STATUS_DOUBLE_BIT_ECC_ERR_FIELD_WIDTH,
    XPORT_MIB_REG_RX_MEM4_ECC_STATUS_DOUBLE_BIT_ECC_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_RX_MEM4_ECC_STATUS_MULTI_ECC_ERR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_RX_MEM4_ECC_STATUS_MULTI_ECC_ERR_FIELD =
{
    "MULTI_ECC_ERR",
#if RU_INCLUDE_DESC
    "",
    "Indicates more than one single bit error or double bit error are detected.",
#endif
    XPORT_MIB_REG_RX_MEM4_ECC_STATUS_MULTI_ECC_ERR_FIELD_MASK,
    0,
    XPORT_MIB_REG_RX_MEM4_ECC_STATUS_MULTI_ECC_ERR_FIELD_WIDTH,
    XPORT_MIB_REG_RX_MEM4_ECC_STATUS_MULTI_ECC_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_RX_MEM4_ECC_STATUS_ECC_ERR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_RX_MEM4_ECC_STATUS_ECC_ERR_FIELD =
{
    "ECC_ERR",
#if RU_INCLUDE_DESC
    "",
    "Single Bit Error (correctable) or Double Bit Error (Uncorrectable) occurred.",
#endif
    XPORT_MIB_REG_RX_MEM4_ECC_STATUS_ECC_ERR_FIELD_MASK,
    0,
    XPORT_MIB_REG_RX_MEM4_ECC_STATUS_ECC_ERR_FIELD_WIDTH,
    XPORT_MIB_REG_RX_MEM4_ECC_STATUS_ECC_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_TX_MEM0_ECC_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_TX_MEM0_ECC_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_MIB_REG_TX_MEM0_ECC_STATUS_RESERVED0_FIELD_MASK,
    0,
    XPORT_MIB_REG_TX_MEM0_ECC_STATUS_RESERVED0_FIELD_WIDTH,
    XPORT_MIB_REG_TX_MEM0_ECC_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_TX_MEM0_ECC_STATUS_MEM_ADDR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_TX_MEM0_ECC_STATUS_MEM_ADDR_FIELD =
{
    "MEM_ADDR",
#if RU_INCLUDE_DESC
    "",
    "First memory address in which single bit error or double bit error is detected.",
#endif
    XPORT_MIB_REG_TX_MEM0_ECC_STATUS_MEM_ADDR_FIELD_MASK,
    0,
    XPORT_MIB_REG_TX_MEM0_ECC_STATUS_MEM_ADDR_FIELD_WIDTH,
    XPORT_MIB_REG_TX_MEM0_ECC_STATUS_MEM_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_TX_MEM0_ECC_STATUS_DOUBLE_BIT_ECC_ERR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_TX_MEM0_ECC_STATUS_DOUBLE_BIT_ECC_ERR_FIELD =
{
    "DOUBLE_BIT_ECC_ERR",
#if RU_INCLUDE_DESC
    "",
    "Double Bit Error indicates an uncorrectable error occurred.",
#endif
    XPORT_MIB_REG_TX_MEM0_ECC_STATUS_DOUBLE_BIT_ECC_ERR_FIELD_MASK,
    0,
    XPORT_MIB_REG_TX_MEM0_ECC_STATUS_DOUBLE_BIT_ECC_ERR_FIELD_WIDTH,
    XPORT_MIB_REG_TX_MEM0_ECC_STATUS_DOUBLE_BIT_ECC_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_TX_MEM0_ECC_STATUS_MULTI_ECC_ERR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_TX_MEM0_ECC_STATUS_MULTI_ECC_ERR_FIELD =
{
    "MULTI_ECC_ERR",
#if RU_INCLUDE_DESC
    "",
    "Indicates more than one single bit error or double bit error are detected.",
#endif
    XPORT_MIB_REG_TX_MEM0_ECC_STATUS_MULTI_ECC_ERR_FIELD_MASK,
    0,
    XPORT_MIB_REG_TX_MEM0_ECC_STATUS_MULTI_ECC_ERR_FIELD_WIDTH,
    XPORT_MIB_REG_TX_MEM0_ECC_STATUS_MULTI_ECC_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_TX_MEM0_ECC_STATUS_ECC_ERR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_TX_MEM0_ECC_STATUS_ECC_ERR_FIELD =
{
    "ECC_ERR",
#if RU_INCLUDE_DESC
    "",
    "Single Bit Error (correctable) or Double Bit Error (Uncorrectable) occurred.",
#endif
    XPORT_MIB_REG_TX_MEM0_ECC_STATUS_ECC_ERR_FIELD_MASK,
    0,
    XPORT_MIB_REG_TX_MEM0_ECC_STATUS_ECC_ERR_FIELD_WIDTH,
    XPORT_MIB_REG_TX_MEM0_ECC_STATUS_ECC_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_TX_MEM1_ECC_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_TX_MEM1_ECC_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_MIB_REG_TX_MEM1_ECC_STATUS_RESERVED0_FIELD_MASK,
    0,
    XPORT_MIB_REG_TX_MEM1_ECC_STATUS_RESERVED0_FIELD_WIDTH,
    XPORT_MIB_REG_TX_MEM1_ECC_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_TX_MEM1_ECC_STATUS_MEM_ADDR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_TX_MEM1_ECC_STATUS_MEM_ADDR_FIELD =
{
    "MEM_ADDR",
#if RU_INCLUDE_DESC
    "",
    "First memory address in which single bit error or double bit error is detected.",
#endif
    XPORT_MIB_REG_TX_MEM1_ECC_STATUS_MEM_ADDR_FIELD_MASK,
    0,
    XPORT_MIB_REG_TX_MEM1_ECC_STATUS_MEM_ADDR_FIELD_WIDTH,
    XPORT_MIB_REG_TX_MEM1_ECC_STATUS_MEM_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_TX_MEM1_ECC_STATUS_DOUBLE_BIT_ECC_ERR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_TX_MEM1_ECC_STATUS_DOUBLE_BIT_ECC_ERR_FIELD =
{
    "DOUBLE_BIT_ECC_ERR",
#if RU_INCLUDE_DESC
    "",
    "Double Bit Error indicates an uncorrectable error occurred.",
#endif
    XPORT_MIB_REG_TX_MEM1_ECC_STATUS_DOUBLE_BIT_ECC_ERR_FIELD_MASK,
    0,
    XPORT_MIB_REG_TX_MEM1_ECC_STATUS_DOUBLE_BIT_ECC_ERR_FIELD_WIDTH,
    XPORT_MIB_REG_TX_MEM1_ECC_STATUS_DOUBLE_BIT_ECC_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_TX_MEM1_ECC_STATUS_MULTI_ECC_ERR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_TX_MEM1_ECC_STATUS_MULTI_ECC_ERR_FIELD =
{
    "MULTI_ECC_ERR",
#if RU_INCLUDE_DESC
    "",
    "Indicates more than one single bit error or double bit error are detected.",
#endif
    XPORT_MIB_REG_TX_MEM1_ECC_STATUS_MULTI_ECC_ERR_FIELD_MASK,
    0,
    XPORT_MIB_REG_TX_MEM1_ECC_STATUS_MULTI_ECC_ERR_FIELD_WIDTH,
    XPORT_MIB_REG_TX_MEM1_ECC_STATUS_MULTI_ECC_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_TX_MEM1_ECC_STATUS_ECC_ERR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_TX_MEM1_ECC_STATUS_ECC_ERR_FIELD =
{
    "ECC_ERR",
#if RU_INCLUDE_DESC
    "",
    "Single Bit Error (correctable) or Double Bit Error (Uncorrectable) occurred.",
#endif
    XPORT_MIB_REG_TX_MEM1_ECC_STATUS_ECC_ERR_FIELD_MASK,
    0,
    XPORT_MIB_REG_TX_MEM1_ECC_STATUS_ECC_ERR_FIELD_WIDTH,
    XPORT_MIB_REG_TX_MEM1_ECC_STATUS_ECC_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_TX_MEM2_ECC_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_TX_MEM2_ECC_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_MIB_REG_TX_MEM2_ECC_STATUS_RESERVED0_FIELD_MASK,
    0,
    XPORT_MIB_REG_TX_MEM2_ECC_STATUS_RESERVED0_FIELD_WIDTH,
    XPORT_MIB_REG_TX_MEM2_ECC_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_TX_MEM2_ECC_STATUS_MEM_ADDR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_TX_MEM2_ECC_STATUS_MEM_ADDR_FIELD =
{
    "MEM_ADDR",
#if RU_INCLUDE_DESC
    "",
    "First memory address in which single bit error or double bit error is detected.",
#endif
    XPORT_MIB_REG_TX_MEM2_ECC_STATUS_MEM_ADDR_FIELD_MASK,
    0,
    XPORT_MIB_REG_TX_MEM2_ECC_STATUS_MEM_ADDR_FIELD_WIDTH,
    XPORT_MIB_REG_TX_MEM2_ECC_STATUS_MEM_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_TX_MEM2_ECC_STATUS_DOUBLE_BIT_ECC_ERR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_TX_MEM2_ECC_STATUS_DOUBLE_BIT_ECC_ERR_FIELD =
{
    "DOUBLE_BIT_ECC_ERR",
#if RU_INCLUDE_DESC
    "",
    "Double Bit Error indicates an uncorrectable error occurred.",
#endif
    XPORT_MIB_REG_TX_MEM2_ECC_STATUS_DOUBLE_BIT_ECC_ERR_FIELD_MASK,
    0,
    XPORT_MIB_REG_TX_MEM2_ECC_STATUS_DOUBLE_BIT_ECC_ERR_FIELD_WIDTH,
    XPORT_MIB_REG_TX_MEM2_ECC_STATUS_DOUBLE_BIT_ECC_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_TX_MEM2_ECC_STATUS_MULTI_ECC_ERR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_TX_MEM2_ECC_STATUS_MULTI_ECC_ERR_FIELD =
{
    "MULTI_ECC_ERR",
#if RU_INCLUDE_DESC
    "",
    "Indicates more than one single bit error or double bit error are detected.",
#endif
    XPORT_MIB_REG_TX_MEM2_ECC_STATUS_MULTI_ECC_ERR_FIELD_MASK,
    0,
    XPORT_MIB_REG_TX_MEM2_ECC_STATUS_MULTI_ECC_ERR_FIELD_WIDTH,
    XPORT_MIB_REG_TX_MEM2_ECC_STATUS_MULTI_ECC_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_TX_MEM2_ECC_STATUS_ECC_ERR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_TX_MEM2_ECC_STATUS_ECC_ERR_FIELD =
{
    "ECC_ERR",
#if RU_INCLUDE_DESC
    "",
    "Single Bit Error (correctable) or Double Bit Error (Uncorrectable) occurred.",
#endif
    XPORT_MIB_REG_TX_MEM2_ECC_STATUS_ECC_ERR_FIELD_MASK,
    0,
    XPORT_MIB_REG_TX_MEM2_ECC_STATUS_ECC_ERR_FIELD_WIDTH,
    XPORT_MIB_REG_TX_MEM2_ECC_STATUS_ECC_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_TX_MEM3_ECC_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_TX_MEM3_ECC_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_MIB_REG_TX_MEM3_ECC_STATUS_RESERVED0_FIELD_MASK,
    0,
    XPORT_MIB_REG_TX_MEM3_ECC_STATUS_RESERVED0_FIELD_WIDTH,
    XPORT_MIB_REG_TX_MEM3_ECC_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_TX_MEM3_ECC_STATUS_MEM_ADDR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_TX_MEM3_ECC_STATUS_MEM_ADDR_FIELD =
{
    "MEM_ADDR",
#if RU_INCLUDE_DESC
    "",
    "First memory address in which single bit error or double bit error is detected.",
#endif
    XPORT_MIB_REG_TX_MEM3_ECC_STATUS_MEM_ADDR_FIELD_MASK,
    0,
    XPORT_MIB_REG_TX_MEM3_ECC_STATUS_MEM_ADDR_FIELD_WIDTH,
    XPORT_MIB_REG_TX_MEM3_ECC_STATUS_MEM_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_TX_MEM3_ECC_STATUS_DOUBLE_BIT_ECC_ERR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_TX_MEM3_ECC_STATUS_DOUBLE_BIT_ECC_ERR_FIELD =
{
    "DOUBLE_BIT_ECC_ERR",
#if RU_INCLUDE_DESC
    "",
    "Double Bit Error indicates an uncorrectable error occurred.",
#endif
    XPORT_MIB_REG_TX_MEM3_ECC_STATUS_DOUBLE_BIT_ECC_ERR_FIELD_MASK,
    0,
    XPORT_MIB_REG_TX_MEM3_ECC_STATUS_DOUBLE_BIT_ECC_ERR_FIELD_WIDTH,
    XPORT_MIB_REG_TX_MEM3_ECC_STATUS_DOUBLE_BIT_ECC_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_TX_MEM3_ECC_STATUS_MULTI_ECC_ERR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_TX_MEM3_ECC_STATUS_MULTI_ECC_ERR_FIELD =
{
    "MULTI_ECC_ERR",
#if RU_INCLUDE_DESC
    "",
    "Indicates more than one single bit error or double bit error are detected.",
#endif
    XPORT_MIB_REG_TX_MEM3_ECC_STATUS_MULTI_ECC_ERR_FIELD_MASK,
    0,
    XPORT_MIB_REG_TX_MEM3_ECC_STATUS_MULTI_ECC_ERR_FIELD_WIDTH,
    XPORT_MIB_REG_TX_MEM3_ECC_STATUS_MULTI_ECC_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MIB_REG_TX_MEM3_ECC_STATUS_ECC_ERR
 ******************************************************************************/
const ru_field_rec XPORT_MIB_REG_TX_MEM3_ECC_STATUS_ECC_ERR_FIELD =
{
    "ECC_ERR",
#if RU_INCLUDE_DESC
    "",
    "Single Bit Error (correctable) or Double Bit Error (Uncorrectable) occurred.",
#endif
    XPORT_MIB_REG_TX_MEM3_ECC_STATUS_ECC_ERR_FIELD_MASK,
    0,
    XPORT_MIB_REG_TX_MEM3_ECC_STATUS_ECC_ERR_FIELD_WIDTH,
    XPORT_MIB_REG_TX_MEM3_ECC_STATUS_ECC_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: XPORT_MIB_REG_DIR_ACC_DATA_WRITE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_MIB_REG_DIR_ACC_DATA_WRITE_FIELDS[] =
{
    &XPORT_MIB_REG_DIR_ACC_DATA_WRITE_WRITE_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_MIB_REG_DIR_ACC_DATA_WRITE_REG = 
{
    "DIR_ACC_DATA_WRITE",
#if RU_INCLUDE_DESC
    "MIB 32-bit Direct Access Data Write Register",
    "",
#endif
    XPORT_MIB_REG_DIR_ACC_DATA_WRITE_REG_OFFSET,
    0,
    0,
    198,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPORT_MIB_REG_DIR_ACC_DATA_WRITE_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_MIB_REG_DIR_ACC_DATA_READ
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_MIB_REG_DIR_ACC_DATA_READ_FIELDS[] =
{
    &XPORT_MIB_REG_DIR_ACC_DATA_READ_READ_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_MIB_REG_DIR_ACC_DATA_READ_REG = 
{
    "DIR_ACC_DATA_READ",
#if RU_INCLUDE_DESC
    "MIB 32-bit Direct Access Data Read Register",
    "",
#endif
    XPORT_MIB_REG_DIR_ACC_DATA_READ_REG_OFFSET,
    0,
    0,
    199,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPORT_MIB_REG_DIR_ACC_DATA_READ_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_MIB_REG_INDIR_ACC_ADDR_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_MIB_REG_INDIR_ACC_ADDR_0_FIELDS[] =
{
    &XPORT_MIB_REG_INDIR_ACC_ADDR_0_RESERVED0_FIELD,
    &XPORT_MIB_REG_INDIR_ACC_ADDR_0_ERR_FIELD,
    &XPORT_MIB_REG_INDIR_ACC_ADDR_0_START_BUSY_FIELD,
    &XPORT_MIB_REG_INDIR_ACC_ADDR_0_R_W_FIELD,
    &XPORT_MIB_REG_INDIR_ACC_ADDR_0_REG_PORT_ID_FIELD,
    &XPORT_MIB_REG_INDIR_ACC_ADDR_0_REG_OFFSET_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_MIB_REG_INDIR_ACC_ADDR_0_REG = 
{
    "INDIR_ACC_ADDR_0",
#if RU_INCLUDE_DESC
    "MIB Indirect Access Address Register",
    "",
#endif
    XPORT_MIB_REG_INDIR_ACC_ADDR_0_REG_OFFSET,
    0,
    0,
    200,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    XPORT_MIB_REG_INDIR_ACC_ADDR_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_MIB_REG_INDIR_ACC_DATA_LOW_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_MIB_REG_INDIR_ACC_DATA_LOW_0_FIELDS[] =
{
    &XPORT_MIB_REG_INDIR_ACC_DATA_LOW_0_DATA_LOW_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_MIB_REG_INDIR_ACC_DATA_LOW_0_REG = 
{
    "INDIR_ACC_DATA_LOW_0",
#if RU_INCLUDE_DESC
    "MIB Indirect Access Data Low Register",
    "",
#endif
    XPORT_MIB_REG_INDIR_ACC_DATA_LOW_0_REG_OFFSET,
    0,
    0,
    201,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPORT_MIB_REG_INDIR_ACC_DATA_LOW_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_MIB_REG_INDIR_ACC_DATA_HIGH_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_MIB_REG_INDIR_ACC_DATA_HIGH_0_FIELDS[] =
{
    &XPORT_MIB_REG_INDIR_ACC_DATA_HIGH_0_DATA_HIGH_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_MIB_REG_INDIR_ACC_DATA_HIGH_0_REG = 
{
    "INDIR_ACC_DATA_HIGH_0",
#if RU_INCLUDE_DESC
    "MIB Indirect Access Data High Register",
    "",
#endif
    XPORT_MIB_REG_INDIR_ACC_DATA_HIGH_0_REG_OFFSET,
    0,
    0,
    202,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPORT_MIB_REG_INDIR_ACC_DATA_HIGH_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_MIB_REG_INDIR_ACC_ADDR_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_MIB_REG_INDIR_ACC_ADDR_1_FIELDS[] =
{
    &XPORT_MIB_REG_INDIR_ACC_ADDR_1_RESERVED0_FIELD,
    &XPORT_MIB_REG_INDIR_ACC_ADDR_1_ERR_FIELD,
    &XPORT_MIB_REG_INDIR_ACC_ADDR_1_START_BUSY_FIELD,
    &XPORT_MIB_REG_INDIR_ACC_ADDR_1_R_W_FIELD,
    &XPORT_MIB_REG_INDIR_ACC_ADDR_1_REG_PORT_ID_FIELD,
    &XPORT_MIB_REG_INDIR_ACC_ADDR_1_REG_OFFSET_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_MIB_REG_INDIR_ACC_ADDR_1_REG = 
{
    "INDIR_ACC_ADDR_1",
#if RU_INCLUDE_DESC
    "MIB Indirect Access Address Register",
    "",
#endif
    XPORT_MIB_REG_INDIR_ACC_ADDR_1_REG_OFFSET,
    0,
    0,
    203,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    XPORT_MIB_REG_INDIR_ACC_ADDR_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_MIB_REG_INDIR_ACC_DATA_LOW_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_MIB_REG_INDIR_ACC_DATA_LOW_1_FIELDS[] =
{
    &XPORT_MIB_REG_INDIR_ACC_DATA_LOW_1_DATA_LOW_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_MIB_REG_INDIR_ACC_DATA_LOW_1_REG = 
{
    "INDIR_ACC_DATA_LOW_1",
#if RU_INCLUDE_DESC
    "MIB Indirect Access Data Low Register",
    "",
#endif
    XPORT_MIB_REG_INDIR_ACC_DATA_LOW_1_REG_OFFSET,
    0,
    0,
    204,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPORT_MIB_REG_INDIR_ACC_DATA_LOW_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_MIB_REG_INDIR_ACC_DATA_HIGH_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_MIB_REG_INDIR_ACC_DATA_HIGH_1_FIELDS[] =
{
    &XPORT_MIB_REG_INDIR_ACC_DATA_HIGH_1_DATA_HIGH_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_MIB_REG_INDIR_ACC_DATA_HIGH_1_REG = 
{
    "INDIR_ACC_DATA_HIGH_1",
#if RU_INCLUDE_DESC
    "MIB Indirect Access Data High Register",
    "",
#endif
    XPORT_MIB_REG_INDIR_ACC_DATA_HIGH_1_REG_OFFSET,
    0,
    0,
    205,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPORT_MIB_REG_INDIR_ACC_DATA_HIGH_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_MIB_REG_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_MIB_REG_CNTRL_FIELDS[] =
{
    &XPORT_MIB_REG_CNTRL_RESERVED0_FIELD,
    &XPORT_MIB_REG_CNTRL_EEE_CNT_MODE_FIELD,
    &XPORT_MIB_REG_CNTRL_SATURATE_EN_FIELD,
    &XPORT_MIB_REG_CNTRL_COR_EN_FIELD,
    &XPORT_MIB_REG_CNTRL_CNT_RST_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_MIB_REG_CNTRL_REG = 
{
    "CNTRL",
#if RU_INCLUDE_DESC
    "MIB Control Register",
    "",
#endif
    XPORT_MIB_REG_CNTRL_REG_OFFSET,
    0,
    0,
    206,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    XPORT_MIB_REG_CNTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_MIB_REG_EEE_PULSE_DURATION_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_MIB_REG_EEE_PULSE_DURATION_CNTRL_FIELDS[] =
{
    &XPORT_MIB_REG_EEE_PULSE_DURATION_CNTRL_RESERVED0_FIELD,
    &XPORT_MIB_REG_EEE_PULSE_DURATION_CNTRL_CNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_MIB_REG_EEE_PULSE_DURATION_CNTRL_REG = 
{
    "EEE_PULSE_DURATION_CNTRL",
#if RU_INCLUDE_DESC
    "MIB EEE Pulse Duration Control Register",
    "",
#endif
    XPORT_MIB_REG_EEE_PULSE_DURATION_CNTRL_REG_OFFSET,
    0,
    0,
    207,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_MIB_REG_EEE_PULSE_DURATION_CNTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_MIB_REG_GPORT0_MAX_PKT_SIZE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_MIB_REG_GPORT0_MAX_PKT_SIZE_FIELDS[] =
{
    &XPORT_MIB_REG_GPORT0_MAX_PKT_SIZE_RESERVED0_FIELD,
    &XPORT_MIB_REG_GPORT0_MAX_PKT_SIZE_MAX_PKT_SIZE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_MIB_REG_GPORT0_MAX_PKT_SIZE_REG = 
{
    "GPORT0_MAX_PKT_SIZE",
#if RU_INCLUDE_DESC
    "MIB Max Packet Size Register",
    "",
#endif
    XPORT_MIB_REG_GPORT0_MAX_PKT_SIZE_REG_OFFSET,
    0,
    0,
    208,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_MIB_REG_GPORT0_MAX_PKT_SIZE_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_MIB_REG_GPORT1_MAX_PKT_SIZE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_MIB_REG_GPORT1_MAX_PKT_SIZE_FIELDS[] =
{
    &XPORT_MIB_REG_GPORT1_MAX_PKT_SIZE_RESERVED0_FIELD,
    &XPORT_MIB_REG_GPORT1_MAX_PKT_SIZE_MAX_PKT_SIZE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_MIB_REG_GPORT1_MAX_PKT_SIZE_REG = 
{
    "GPORT1_MAX_PKT_SIZE",
#if RU_INCLUDE_DESC
    "MIB Max Packet Size Register",
    "",
#endif
    XPORT_MIB_REG_GPORT1_MAX_PKT_SIZE_REG_OFFSET,
    0,
    0,
    209,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_MIB_REG_GPORT1_MAX_PKT_SIZE_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_MIB_REG_GPORT2_MAX_PKT_SIZE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_MIB_REG_GPORT2_MAX_PKT_SIZE_FIELDS[] =
{
    &XPORT_MIB_REG_GPORT2_MAX_PKT_SIZE_RESERVED0_FIELD,
    &XPORT_MIB_REG_GPORT2_MAX_PKT_SIZE_MAX_PKT_SIZE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_MIB_REG_GPORT2_MAX_PKT_SIZE_REG = 
{
    "GPORT2_MAX_PKT_SIZE",
#if RU_INCLUDE_DESC
    "MIB Max Packet Size Register",
    "",
#endif
    XPORT_MIB_REG_GPORT2_MAX_PKT_SIZE_REG_OFFSET,
    0,
    0,
    210,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_MIB_REG_GPORT2_MAX_PKT_SIZE_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_MIB_REG_GPORT3_MAX_PKT_SIZE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_MIB_REG_GPORT3_MAX_PKT_SIZE_FIELDS[] =
{
    &XPORT_MIB_REG_GPORT3_MAX_PKT_SIZE_RESERVED0_FIELD,
    &XPORT_MIB_REG_GPORT3_MAX_PKT_SIZE_MAX_PKT_SIZE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_MIB_REG_GPORT3_MAX_PKT_SIZE_REG = 
{
    "GPORT3_MAX_PKT_SIZE",
#if RU_INCLUDE_DESC
    "MIB Max Packet Size Register",
    "",
#endif
    XPORT_MIB_REG_GPORT3_MAX_PKT_SIZE_REG_OFFSET,
    0,
    0,
    211,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_MIB_REG_GPORT3_MAX_PKT_SIZE_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_MIB_REG_ECC_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_MIB_REG_ECC_CNTRL_FIELDS[] =
{
    &XPORT_MIB_REG_ECC_CNTRL_RESERVED0_FIELD,
    &XPORT_MIB_REG_ECC_CNTRL_TX_MIB_ECC_EN_FIELD,
    &XPORT_MIB_REG_ECC_CNTRL_RX_MIB_ECC_EN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_MIB_REG_ECC_CNTRL_REG = 
{
    "ECC_CNTRL",
#if RU_INCLUDE_DESC
    "MIB ECC Control Register",
    "",
#endif
    XPORT_MIB_REG_ECC_CNTRL_REG_OFFSET,
    0,
    0,
    212,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    XPORT_MIB_REG_ECC_CNTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_MIB_REG_FORCE_SB_ECC_ERR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_MIB_REG_FORCE_SB_ECC_ERR_FIELDS[] =
{
    &XPORT_MIB_REG_FORCE_SB_ECC_ERR_RESERVED0_FIELD,
    &XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_TX_MEM3_SERR_FIELD,
    &XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_TX_MEM2_SERR_FIELD,
    &XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_TX_MEM1_SERR_FIELD,
    &XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_TX_MEM0_SERR_FIELD,
    &XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_RX_MEM4_SERR_FIELD,
    &XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_RX_MEM3_SERR_FIELD,
    &XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_RX_MEM2_SERR_FIELD,
    &XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_RX_MEM1_SERR_FIELD,
    &XPORT_MIB_REG_FORCE_SB_ECC_ERR_FORCE_RX_MEM0_SERR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_MIB_REG_FORCE_SB_ECC_ERR_REG = 
{
    "FORCE_SB_ECC_ERR",
#if RU_INCLUDE_DESC
    "MIB Force Single Bit ECC Error Register",
    "",
#endif
    XPORT_MIB_REG_FORCE_SB_ECC_ERR_REG_OFFSET,
    0,
    0,
    213,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    10,
    XPORT_MIB_REG_FORCE_SB_ECC_ERR_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_MIB_REG_FORCE_DB_ECC_ERR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_MIB_REG_FORCE_DB_ECC_ERR_FIELDS[] =
{
    &XPORT_MIB_REG_FORCE_DB_ECC_ERR_RESERVED0_FIELD,
    &XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_TX_MEM3_DERR_FIELD,
    &XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_TX_MEM2_DERR_FIELD,
    &XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_TX_MEM1_DERR_FIELD,
    &XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_TX_MEM0_DERR_FIELD,
    &XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_RX_MEM4_DERR_FIELD,
    &XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_RX_MEM3_DERR_FIELD,
    &XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_RX_MEM2_DERR_FIELD,
    &XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_RX_MEM1_DERR_FIELD,
    &XPORT_MIB_REG_FORCE_DB_ECC_ERR_FORCE_RX_MEM0_DERR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_MIB_REG_FORCE_DB_ECC_ERR_REG = 
{
    "FORCE_DB_ECC_ERR",
#if RU_INCLUDE_DESC
    "MIB Force Double Bit ECC Error Register",
    "",
#endif
    XPORT_MIB_REG_FORCE_DB_ECC_ERR_REG_OFFSET,
    0,
    0,
    214,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    10,
    XPORT_MIB_REG_FORCE_DB_ECC_ERR_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_MIB_REG_RX_MEM0_ECC_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_MIB_REG_RX_MEM0_ECC_STATUS_FIELDS[] =
{
    &XPORT_MIB_REG_RX_MEM0_ECC_STATUS_RESERVED0_FIELD,
    &XPORT_MIB_REG_RX_MEM0_ECC_STATUS_MEM_ADDR_FIELD,
    &XPORT_MIB_REG_RX_MEM0_ECC_STATUS_DOUBLE_BIT_ECC_ERR_FIELD,
    &XPORT_MIB_REG_RX_MEM0_ECC_STATUS_MULTI_ECC_ERR_FIELD,
    &XPORT_MIB_REG_RX_MEM0_ECC_STATUS_ECC_ERR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_MIB_REG_RX_MEM0_ECC_STATUS_REG = 
{
    "RX_MEM0_ECC_STATUS",
#if RU_INCLUDE_DESC
    "MIB TX MEM3 ECC Status Register",
    "",
#endif
    XPORT_MIB_REG_RX_MEM0_ECC_STATUS_REG_OFFSET,
    0,
    0,
    215,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    XPORT_MIB_REG_RX_MEM0_ECC_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_MIB_REG_RX_MEM1_ECC_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_MIB_REG_RX_MEM1_ECC_STATUS_FIELDS[] =
{
    &XPORT_MIB_REG_RX_MEM1_ECC_STATUS_RESERVED0_FIELD,
    &XPORT_MIB_REG_RX_MEM1_ECC_STATUS_MEM_ADDR_FIELD,
    &XPORT_MIB_REG_RX_MEM1_ECC_STATUS_DOUBLE_BIT_ECC_ERR_FIELD,
    &XPORT_MIB_REG_RX_MEM1_ECC_STATUS_MULTI_ECC_ERR_FIELD,
    &XPORT_MIB_REG_RX_MEM1_ECC_STATUS_ECC_ERR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_MIB_REG_RX_MEM1_ECC_STATUS_REG = 
{
    "RX_MEM1_ECC_STATUS",
#if RU_INCLUDE_DESC
    "MIB TX MEM3 ECC Status Register",
    "",
#endif
    XPORT_MIB_REG_RX_MEM1_ECC_STATUS_REG_OFFSET,
    0,
    0,
    216,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    XPORT_MIB_REG_RX_MEM1_ECC_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_MIB_REG_RX_MEM2_ECC_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_MIB_REG_RX_MEM2_ECC_STATUS_FIELDS[] =
{
    &XPORT_MIB_REG_RX_MEM2_ECC_STATUS_RESERVED0_FIELD,
    &XPORT_MIB_REG_RX_MEM2_ECC_STATUS_MEM_ADDR_FIELD,
    &XPORT_MIB_REG_RX_MEM2_ECC_STATUS_DOUBLE_BIT_ECC_ERR_FIELD,
    &XPORT_MIB_REG_RX_MEM2_ECC_STATUS_MULTI_ECC_ERR_FIELD,
    &XPORT_MIB_REG_RX_MEM2_ECC_STATUS_ECC_ERR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_MIB_REG_RX_MEM2_ECC_STATUS_REG = 
{
    "RX_MEM2_ECC_STATUS",
#if RU_INCLUDE_DESC
    "MIB TX MEM3 ECC Status Register",
    "",
#endif
    XPORT_MIB_REG_RX_MEM2_ECC_STATUS_REG_OFFSET,
    0,
    0,
    217,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    XPORT_MIB_REG_RX_MEM2_ECC_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_MIB_REG_RX_MEM3_ECC_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_MIB_REG_RX_MEM3_ECC_STATUS_FIELDS[] =
{
    &XPORT_MIB_REG_RX_MEM3_ECC_STATUS_RESERVED0_FIELD,
    &XPORT_MIB_REG_RX_MEM3_ECC_STATUS_MEM_ADDR_FIELD,
    &XPORT_MIB_REG_RX_MEM3_ECC_STATUS_DOUBLE_BIT_ECC_ERR_FIELD,
    &XPORT_MIB_REG_RX_MEM3_ECC_STATUS_MULTI_ECC_ERR_FIELD,
    &XPORT_MIB_REG_RX_MEM3_ECC_STATUS_ECC_ERR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_MIB_REG_RX_MEM3_ECC_STATUS_REG = 
{
    "RX_MEM3_ECC_STATUS",
#if RU_INCLUDE_DESC
    "MIB TX MEM3 ECC Status Register",
    "",
#endif
    XPORT_MIB_REG_RX_MEM3_ECC_STATUS_REG_OFFSET,
    0,
    0,
    218,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    XPORT_MIB_REG_RX_MEM3_ECC_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_MIB_REG_RX_MEM4_ECC_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_MIB_REG_RX_MEM4_ECC_STATUS_FIELDS[] =
{
    &XPORT_MIB_REG_RX_MEM4_ECC_STATUS_RESERVED0_FIELD,
    &XPORT_MIB_REG_RX_MEM4_ECC_STATUS_MEM_ADDR_FIELD,
    &XPORT_MIB_REG_RX_MEM4_ECC_STATUS_DOUBLE_BIT_ECC_ERR_FIELD,
    &XPORT_MIB_REG_RX_MEM4_ECC_STATUS_MULTI_ECC_ERR_FIELD,
    &XPORT_MIB_REG_RX_MEM4_ECC_STATUS_ECC_ERR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_MIB_REG_RX_MEM4_ECC_STATUS_REG = 
{
    "RX_MEM4_ECC_STATUS",
#if RU_INCLUDE_DESC
    "MIB TX MEM3 ECC Status Register",
    "",
#endif
    XPORT_MIB_REG_RX_MEM4_ECC_STATUS_REG_OFFSET,
    0,
    0,
    219,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    XPORT_MIB_REG_RX_MEM4_ECC_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_MIB_REG_TX_MEM0_ECC_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_MIB_REG_TX_MEM0_ECC_STATUS_FIELDS[] =
{
    &XPORT_MIB_REG_TX_MEM0_ECC_STATUS_RESERVED0_FIELD,
    &XPORT_MIB_REG_TX_MEM0_ECC_STATUS_MEM_ADDR_FIELD,
    &XPORT_MIB_REG_TX_MEM0_ECC_STATUS_DOUBLE_BIT_ECC_ERR_FIELD,
    &XPORT_MIB_REG_TX_MEM0_ECC_STATUS_MULTI_ECC_ERR_FIELD,
    &XPORT_MIB_REG_TX_MEM0_ECC_STATUS_ECC_ERR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_MIB_REG_TX_MEM0_ECC_STATUS_REG = 
{
    "TX_MEM0_ECC_STATUS",
#if RU_INCLUDE_DESC
    "MIB TX MEM3 ECC Status Register",
    "",
#endif
    XPORT_MIB_REG_TX_MEM0_ECC_STATUS_REG_OFFSET,
    0,
    0,
    220,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    XPORT_MIB_REG_TX_MEM0_ECC_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_MIB_REG_TX_MEM1_ECC_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_MIB_REG_TX_MEM1_ECC_STATUS_FIELDS[] =
{
    &XPORT_MIB_REG_TX_MEM1_ECC_STATUS_RESERVED0_FIELD,
    &XPORT_MIB_REG_TX_MEM1_ECC_STATUS_MEM_ADDR_FIELD,
    &XPORT_MIB_REG_TX_MEM1_ECC_STATUS_DOUBLE_BIT_ECC_ERR_FIELD,
    &XPORT_MIB_REG_TX_MEM1_ECC_STATUS_MULTI_ECC_ERR_FIELD,
    &XPORT_MIB_REG_TX_MEM1_ECC_STATUS_ECC_ERR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_MIB_REG_TX_MEM1_ECC_STATUS_REG = 
{
    "TX_MEM1_ECC_STATUS",
#if RU_INCLUDE_DESC
    "MIB TX MEM3 ECC Status Register",
    "",
#endif
    XPORT_MIB_REG_TX_MEM1_ECC_STATUS_REG_OFFSET,
    0,
    0,
    221,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    XPORT_MIB_REG_TX_MEM1_ECC_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_MIB_REG_TX_MEM2_ECC_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_MIB_REG_TX_MEM2_ECC_STATUS_FIELDS[] =
{
    &XPORT_MIB_REG_TX_MEM2_ECC_STATUS_RESERVED0_FIELD,
    &XPORT_MIB_REG_TX_MEM2_ECC_STATUS_MEM_ADDR_FIELD,
    &XPORT_MIB_REG_TX_MEM2_ECC_STATUS_DOUBLE_BIT_ECC_ERR_FIELD,
    &XPORT_MIB_REG_TX_MEM2_ECC_STATUS_MULTI_ECC_ERR_FIELD,
    &XPORT_MIB_REG_TX_MEM2_ECC_STATUS_ECC_ERR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_MIB_REG_TX_MEM2_ECC_STATUS_REG = 
{
    "TX_MEM2_ECC_STATUS",
#if RU_INCLUDE_DESC
    "MIB TX MEM3 ECC Status Register",
    "",
#endif
    XPORT_MIB_REG_TX_MEM2_ECC_STATUS_REG_OFFSET,
    0,
    0,
    222,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    XPORT_MIB_REG_TX_MEM2_ECC_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_MIB_REG_TX_MEM3_ECC_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_MIB_REG_TX_MEM3_ECC_STATUS_FIELDS[] =
{
    &XPORT_MIB_REG_TX_MEM3_ECC_STATUS_RESERVED0_FIELD,
    &XPORT_MIB_REG_TX_MEM3_ECC_STATUS_MEM_ADDR_FIELD,
    &XPORT_MIB_REG_TX_MEM3_ECC_STATUS_DOUBLE_BIT_ECC_ERR_FIELD,
    &XPORT_MIB_REG_TX_MEM3_ECC_STATUS_MULTI_ECC_ERR_FIELD,
    &XPORT_MIB_REG_TX_MEM3_ECC_STATUS_ECC_ERR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_MIB_REG_TX_MEM3_ECC_STATUS_REG = 
{
    "TX_MEM3_ECC_STATUS",
#if RU_INCLUDE_DESC
    "MIB TX MEM3 ECC Status Register",
    "",
#endif
    XPORT_MIB_REG_TX_MEM3_ECC_STATUS_REG_OFFSET,
    0,
    0,
    223,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    XPORT_MIB_REG_TX_MEM3_ECC_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: XPORT_MIB_REG
 ******************************************************************************/
static const ru_reg_rec *XPORT_MIB_REG_REGS[] =
{
    &XPORT_MIB_REG_DIR_ACC_DATA_WRITE_REG,
    &XPORT_MIB_REG_DIR_ACC_DATA_READ_REG,
    &XPORT_MIB_REG_INDIR_ACC_ADDR_0_REG,
    &XPORT_MIB_REG_INDIR_ACC_DATA_LOW_0_REG,
    &XPORT_MIB_REG_INDIR_ACC_DATA_HIGH_0_REG,
    &XPORT_MIB_REG_INDIR_ACC_ADDR_1_REG,
    &XPORT_MIB_REG_INDIR_ACC_DATA_LOW_1_REG,
    &XPORT_MIB_REG_INDIR_ACC_DATA_HIGH_1_REG,
    &XPORT_MIB_REG_CNTRL_REG,
    &XPORT_MIB_REG_EEE_PULSE_DURATION_CNTRL_REG,
    &XPORT_MIB_REG_GPORT0_MAX_PKT_SIZE_REG,
    &XPORT_MIB_REG_GPORT1_MAX_PKT_SIZE_REG,
    &XPORT_MIB_REG_GPORT2_MAX_PKT_SIZE_REG,
    &XPORT_MIB_REG_GPORT3_MAX_PKT_SIZE_REG,
    &XPORT_MIB_REG_ECC_CNTRL_REG,
    &XPORT_MIB_REG_FORCE_SB_ECC_ERR_REG,
    &XPORT_MIB_REG_FORCE_DB_ECC_ERR_REG,
    &XPORT_MIB_REG_RX_MEM0_ECC_STATUS_REG,
    &XPORT_MIB_REG_RX_MEM1_ECC_STATUS_REG,
    &XPORT_MIB_REG_RX_MEM2_ECC_STATUS_REG,
    &XPORT_MIB_REG_RX_MEM3_ECC_STATUS_REG,
    &XPORT_MIB_REG_RX_MEM4_ECC_STATUS_REG,
    &XPORT_MIB_REG_TX_MEM0_ECC_STATUS_REG,
    &XPORT_MIB_REG_TX_MEM1_ECC_STATUS_REG,
    &XPORT_MIB_REG_TX_MEM2_ECC_STATUS_REG,
    &XPORT_MIB_REG_TX_MEM3_ECC_STATUS_REG,
};

unsigned long XPORT_MIB_REG_ADDRS[] =
{
    0x8013b100,
};

const ru_block_rec XPORT_MIB_REG_BLOCK = 
{
    "XPORT_MIB_REG",
    XPORT_MIB_REG_ADDRS,
    1,
    26,
    XPORT_MIB_REG_REGS
};

/* End of file XPORT_MIB_REG.c */
