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
 * Field: XPORT_XLMAC_REG_DIR_ACC_DATA_WRITE_WRITE_DATA
 ******************************************************************************/
const ru_field_rec XPORT_XLMAC_REG_DIR_ACC_DATA_WRITE_WRITE_DATA_FIELD =
{
    "WRITE_DATA",
#if RU_INCLUDE_DESC
    "",
    "Direct register access data write register, bits [63:32]. "
    "Used only for 64-bit register accesses.",
#endif
    XPORT_XLMAC_REG_DIR_ACC_DATA_WRITE_WRITE_DATA_FIELD_MASK,
    0,
    XPORT_XLMAC_REG_DIR_ACC_DATA_WRITE_WRITE_DATA_FIELD_WIDTH,
    XPORT_XLMAC_REG_DIR_ACC_DATA_WRITE_WRITE_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_XLMAC_REG_DIR_ACC_DATA_READ_READ_DATA
 ******************************************************************************/
const ru_field_rec XPORT_XLMAC_REG_DIR_ACC_DATA_READ_READ_DATA_FIELD =
{
    "READ_DATA",
#if RU_INCLUDE_DESC
    "",
    "Direct register access data read register, bits [63:32]. "
    "Used only for 64-bit register accesses.",
#endif
    XPORT_XLMAC_REG_DIR_ACC_DATA_READ_READ_DATA_FIELD_MASK,
    0,
    XPORT_XLMAC_REG_DIR_ACC_DATA_READ_READ_DATA_FIELD_WIDTH,
    XPORT_XLMAC_REG_DIR_ACC_DATA_READ_READ_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_XLMAC_REG_INDIR_ACC_ADDR_0_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_XLMAC_REG_INDIR_ACC_ADDR_0_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_XLMAC_REG_INDIR_ACC_ADDR_0_RESERVED0_FIELD_MASK,
    0,
    XPORT_XLMAC_REG_INDIR_ACC_ADDR_0_RESERVED0_FIELD_WIDTH,
    XPORT_XLMAC_REG_INDIR_ACC_ADDR_0_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_XLMAC_REG_INDIR_ACC_ADDR_0_ERR
 ******************************************************************************/
const ru_field_rec XPORT_XLMAC_REG_INDIR_ACC_ADDR_0_ERR_FIELD =
{
    "ERR",
#if RU_INCLUDE_DESC
    "",
    "Transaction Status. When transaction completes (START_BUSY = 0 after it was set to 1) "
    "and this bit is set it indicates that register transaction completed with error.",
#endif
    XPORT_XLMAC_REG_INDIR_ACC_ADDR_0_ERR_FIELD_MASK,
    0,
    XPORT_XLMAC_REG_INDIR_ACC_ADDR_0_ERR_FIELD_WIDTH,
    XPORT_XLMAC_REG_INDIR_ACC_ADDR_0_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_XLMAC_REG_INDIR_ACC_ADDR_0_START_BUSY
 ******************************************************************************/
const ru_field_rec XPORT_XLMAC_REG_INDIR_ACC_ADDR_0_START_BUSY_FIELD =
{
    "START_BUSY",
#if RU_INCLUDE_DESC
    "",
    "START_BUSY, Self-clearing. CPU writes this bit to 1 in order to initiate indirect register "
    "read/write transaction. When transaction completes hardware clears this bit.",
#endif
    XPORT_XLMAC_REG_INDIR_ACC_ADDR_0_START_BUSY_FIELD_MASK,
    0,
    XPORT_XLMAC_REG_INDIR_ACC_ADDR_0_START_BUSY_FIELD_WIDTH,
    XPORT_XLMAC_REG_INDIR_ACC_ADDR_0_START_BUSY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_XLMAC_REG_INDIR_ACC_ADDR_0_R_W
 ******************************************************************************/
const ru_field_rec XPORT_XLMAC_REG_INDIR_ACC_ADDR_0_R_W_FIELD =
{
    "R_W",
#if RU_INCLUDE_DESC
    "",
    "Register transaction:\n"
    "0 : Register Write.\n'"
    "1 : Register Read.\n",
#endif
    XPORT_XLMAC_REG_INDIR_ACC_ADDR_0_R_W_FIELD_MASK,
    0,
    XPORT_XLMAC_REG_INDIR_ACC_ADDR_0_R_W_FIELD_WIDTH,
    XPORT_XLMAC_REG_INDIR_ACC_ADDR_0_R_W_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_XLMAC_REG_INDIR_ACC_ADDR_0_REG_PORT_ID
 ******************************************************************************/
const ru_field_rec XPORT_XLMAC_REG_INDIR_ACC_ADDR_0_REG_PORT_ID_FIELD =
{
    "REG_PORT_ID",
#if RU_INCLUDE_DESC
    "",
    "Register Port ID.",
#endif
    XPORT_XLMAC_REG_INDIR_ACC_ADDR_0_REG_PORT_ID_FIELD_MASK,
    0,
    XPORT_XLMAC_REG_INDIR_ACC_ADDR_0_REG_PORT_ID_FIELD_WIDTH,
    XPORT_XLMAC_REG_INDIR_ACC_ADDR_0_REG_PORT_ID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_XLMAC_REG_INDIR_ACC_ADDR_0_REG_OFFSET
 ******************************************************************************/
const ru_field_rec XPORT_XLMAC_REG_INDIR_ACC_ADDR_0_REG_OFFSET_FIELD =
{
    "REG_OFFSET",
#if RU_INCLUDE_DESC
    "",
    "Register offset.\n"
    "Note: Bit 7 is ignored by HW. Write it as 0.",
#endif
    XPORT_XLMAC_REG_INDIR_ACC_ADDR_0_REG_OFFSET_FIELD_MASK,
    0,
    XPORT_XLMAC_REG_INDIR_ACC_ADDR_0_REG_OFFSET_FIELD_WIDTH,
    XPORT_XLMAC_REG_INDIR_ACC_ADDR_0_REG_OFFSET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_XLMAC_REG_INDIR_ACC_DATA_LOW_0_DATA_LOW
 ******************************************************************************/
const ru_field_rec XPORT_XLMAC_REG_INDIR_ACC_DATA_LOW_0_DATA_LOW_FIELD =
{
    "DATA_LOW",
#if RU_INCLUDE_DESC
    "",
    "Indirect register access data register, bits [31:0].",
#endif
    XPORT_XLMAC_REG_INDIR_ACC_DATA_LOW_0_DATA_LOW_FIELD_MASK,
    0,
    XPORT_XLMAC_REG_INDIR_ACC_DATA_LOW_0_DATA_LOW_FIELD_WIDTH,
    XPORT_XLMAC_REG_INDIR_ACC_DATA_LOW_0_DATA_LOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_XLMAC_REG_INDIR_ACC_DATA_HIGH_0_DATA_HIGH
 ******************************************************************************/
const ru_field_rec XPORT_XLMAC_REG_INDIR_ACC_DATA_HIGH_0_DATA_HIGH_FIELD =
{
    "DATA_HIGH",
#if RU_INCLUDE_DESC
    "",
    "Indirect register access data register, bits [63:32]. "
    "Used only for 64-bit register accesses.",
#endif
    XPORT_XLMAC_REG_INDIR_ACC_DATA_HIGH_0_DATA_HIGH_FIELD_MASK,
    0,
    XPORT_XLMAC_REG_INDIR_ACC_DATA_HIGH_0_DATA_HIGH_FIELD_WIDTH,
    XPORT_XLMAC_REG_INDIR_ACC_DATA_HIGH_0_DATA_HIGH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_XLMAC_REG_INDIR_ACC_ADDR_1_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_XLMAC_REG_INDIR_ACC_ADDR_1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_XLMAC_REG_INDIR_ACC_ADDR_1_RESERVED0_FIELD_MASK,
    0,
    XPORT_XLMAC_REG_INDIR_ACC_ADDR_1_RESERVED0_FIELD_WIDTH,
    XPORT_XLMAC_REG_INDIR_ACC_ADDR_1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_XLMAC_REG_INDIR_ACC_ADDR_1_ERR
 ******************************************************************************/
const ru_field_rec XPORT_XLMAC_REG_INDIR_ACC_ADDR_1_ERR_FIELD =
{
    "ERR",
#if RU_INCLUDE_DESC
    "",
    "Transaction Status. When transaction completes (START_BUSY = 0 after it was set to 1) "
    "and this bit is set it indicates that register transaction completed with error.",
#endif
    XPORT_XLMAC_REG_INDIR_ACC_ADDR_1_ERR_FIELD_MASK,
    0,
    XPORT_XLMAC_REG_INDIR_ACC_ADDR_1_ERR_FIELD_WIDTH,
    XPORT_XLMAC_REG_INDIR_ACC_ADDR_1_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_XLMAC_REG_INDIR_ACC_ADDR_1_START_BUSY
 ******************************************************************************/
const ru_field_rec XPORT_XLMAC_REG_INDIR_ACC_ADDR_1_START_BUSY_FIELD =
{
    "START_BUSY",
#if RU_INCLUDE_DESC
    "",
    "START_BUSY, Self-clearing. CPU writes this bit to 1 in order to initiate indirect register "
    "read/write transaction. When transaction completes hardware clears this bit.",
#endif
    XPORT_XLMAC_REG_INDIR_ACC_ADDR_1_START_BUSY_FIELD_MASK,
    0,
    XPORT_XLMAC_REG_INDIR_ACC_ADDR_1_START_BUSY_FIELD_WIDTH,
    XPORT_XLMAC_REG_INDIR_ACC_ADDR_1_START_BUSY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_XLMAC_REG_INDIR_ACC_ADDR_1_R_W
 ******************************************************************************/
const ru_field_rec XPORT_XLMAC_REG_INDIR_ACC_ADDR_1_R_W_FIELD =
{
    "R_W",
#if RU_INCLUDE_DESC
    "",
    "Register transaction:\n"
    "0 : Register Write.\n'"
    "1 : Register Read.\n",
#endif
    XPORT_XLMAC_REG_INDIR_ACC_ADDR_1_R_W_FIELD_MASK,
    0,
    XPORT_XLMAC_REG_INDIR_ACC_ADDR_1_R_W_FIELD_WIDTH,
    XPORT_XLMAC_REG_INDIR_ACC_ADDR_1_R_W_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_XLMAC_REG_INDIR_ACC_ADDR_1_REG_PORT_ID
 ******************************************************************************/
const ru_field_rec XPORT_XLMAC_REG_INDIR_ACC_ADDR_1_REG_PORT_ID_FIELD =
{
    "REG_PORT_ID",
#if RU_INCLUDE_DESC
    "",
    "Register Port ID.",
#endif
    XPORT_XLMAC_REG_INDIR_ACC_ADDR_1_REG_PORT_ID_FIELD_MASK,
    0,
    XPORT_XLMAC_REG_INDIR_ACC_ADDR_1_REG_PORT_ID_FIELD_WIDTH,
    XPORT_XLMAC_REG_INDIR_ACC_ADDR_1_REG_PORT_ID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_XLMAC_REG_INDIR_ACC_ADDR_1_REG_OFFSET
 ******************************************************************************/
const ru_field_rec XPORT_XLMAC_REG_INDIR_ACC_ADDR_1_REG_OFFSET_FIELD =
{
    "REG_OFFSET",
#if RU_INCLUDE_DESC
    "",
    "Register offset.\n"
    "Note: Bit 7 is ignored by HW. Write it as 0.",
#endif
    XPORT_XLMAC_REG_INDIR_ACC_ADDR_1_REG_OFFSET_FIELD_MASK,
    0,
    XPORT_XLMAC_REG_INDIR_ACC_ADDR_1_REG_OFFSET_FIELD_WIDTH,
    XPORT_XLMAC_REG_INDIR_ACC_ADDR_1_REG_OFFSET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_XLMAC_REG_INDIR_ACC_DATA_LOW_1_DATA_LOW
 ******************************************************************************/
const ru_field_rec XPORT_XLMAC_REG_INDIR_ACC_DATA_LOW_1_DATA_LOW_FIELD =
{
    "DATA_LOW",
#if RU_INCLUDE_DESC
    "",
    "Indirect register access data register, bits [31:0].",
#endif
    XPORT_XLMAC_REG_INDIR_ACC_DATA_LOW_1_DATA_LOW_FIELD_MASK,
    0,
    XPORT_XLMAC_REG_INDIR_ACC_DATA_LOW_1_DATA_LOW_FIELD_WIDTH,
    XPORT_XLMAC_REG_INDIR_ACC_DATA_LOW_1_DATA_LOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_XLMAC_REG_INDIR_ACC_DATA_HIGH_1_DATA_HIGH
 ******************************************************************************/
const ru_field_rec XPORT_XLMAC_REG_INDIR_ACC_DATA_HIGH_1_DATA_HIGH_FIELD =
{
    "DATA_HIGH",
#if RU_INCLUDE_DESC
    "",
    "Indirect register access data register, bits [63:32]. "
    "Used only for 64-bit register accesses.",
#endif
    XPORT_XLMAC_REG_INDIR_ACC_DATA_HIGH_1_DATA_HIGH_FIELD_MASK,
    0,
    XPORT_XLMAC_REG_INDIR_ACC_DATA_HIGH_1_DATA_HIGH_FIELD_WIDTH,
    XPORT_XLMAC_REG_INDIR_ACC_DATA_HIGH_1_DATA_HIGH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_XLMAC_REG_CONFIG_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_XLMAC_REG_CONFIG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_XLMAC_REG_CONFIG_RESERVED0_FIELD_MASK,
    0,
    XPORT_XLMAC_REG_CONFIG_RESERVED0_FIELD_WIDTH,
    XPORT_XLMAC_REG_CONFIG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_XLMAC_REG_CONFIG_XLMAC_RESET
 ******************************************************************************/
const ru_field_rec XPORT_XLMAC_REG_CONFIG_XLMAC_RESET_FIELD =
{
    "XLMAC_RESET",
#if RU_INCLUDE_DESC
    "",
    "Active high XLMAC hard reset.",
#endif
    XPORT_XLMAC_REG_CONFIG_XLMAC_RESET_FIELD_MASK,
    0,
    XPORT_XLMAC_REG_CONFIG_XLMAC_RESET_FIELD_WIDTH,
    XPORT_XLMAC_REG_CONFIG_XLMAC_RESET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_XLMAC_REG_CONFIG_RX_DUAL_CYCLE_TDM_EN
 ******************************************************************************/
const ru_field_rec XPORT_XLMAC_REG_CONFIG_RX_DUAL_CYCLE_TDM_EN_FIELD =
{
    "RX_DUAL_CYCLE_TDM_EN",
#if RU_INCLUDE_DESC
    "",
    "When set, Rx CDC FIFO read TDM order has same port for 2 consecutive cycles.\n"
    "This is a strap input for the MAC core and should be changed only while hard reset is asserted.",
#endif
    XPORT_XLMAC_REG_CONFIG_RX_DUAL_CYCLE_TDM_EN_FIELD_MASK,
    0,
    XPORT_XLMAC_REG_CONFIG_RX_DUAL_CYCLE_TDM_EN_FIELD_WIDTH,
    XPORT_XLMAC_REG_CONFIG_RX_DUAL_CYCLE_TDM_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_XLMAC_REG_CONFIG_RX_NON_LINEAR_QUAD_TDM_EN
 ******************************************************************************/
const ru_field_rec XPORT_XLMAC_REG_CONFIG_RX_NON_LINEAR_QUAD_TDM_EN_FIELD =
{
    "RX_NON_LINEAR_QUAD_TDM_EN",
#if RU_INCLUDE_DESC
    "",
    "When set, RX CDC FIFO read TDM generation order for quad mode is 0,2,1,3. Otherwise, it is 0,1,2,3.\n"
    "This is a strap input for the MAC core and should be changed only while hard reset is asserted.",
#endif
    XPORT_XLMAC_REG_CONFIG_RX_NON_LINEAR_QUAD_TDM_EN_FIELD_MASK,
    0,
    XPORT_XLMAC_REG_CONFIG_RX_NON_LINEAR_QUAD_TDM_EN_FIELD_WIDTH,
    XPORT_XLMAC_REG_CONFIG_RX_NON_LINEAR_QUAD_TDM_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_XLMAC_REG_CONFIG_RX_FLEX_TDM_ENABLE
 ******************************************************************************/
const ru_field_rec XPORT_XLMAC_REG_CONFIG_RX_FLEX_TDM_ENABLE_FIELD =
{
    "RX_FLEX_TDM_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "Enables non-linear TDM generation on the receive system interface, "
    " based on data availability in Rx FIFOs.\n"
    "0 : Flex TDM Enabled.\n"
    "1 : Flex TDM Disabled.\n",
#endif
    XPORT_XLMAC_REG_CONFIG_RX_FLEX_TDM_ENABLE_FIELD_MASK,
    0,
    XPORT_XLMAC_REG_CONFIG_RX_FLEX_TDM_ENABLE_FIELD_WIDTH,
    XPORT_XLMAC_REG_CONFIG_RX_FLEX_TDM_ENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_XLMAC_REG_CONFIG_MAC_MODE
 ******************************************************************************/
const ru_field_rec XPORT_XLMAC_REG_CONFIG_MAC_MODE_FIELD =
{
    "MAC_MODE",
#if RU_INCLUDE_DESC
    "",
    "Number of ports supported by XLMAC.\n"
    "000 : Quad Port.All ports are used.\n"
    "001 : Tri-Port. Ports 0, 1 and 2 are used.\n"
    "010 : Tri-Port. Ports 0, 2 and 3 are used.\n"
    "011 : Dual Port. Port 0 and 2 are used.\n"
    "1xx : Single Port. Port 0 is used.\n"
    "Note: Valid combinations for 63158 are single Port (P0 active) or Quad Port (P0 and/or P1 active).",
#endif
    XPORT_XLMAC_REG_CONFIG_MAC_MODE_FIELD_MASK,
    0,
    XPORT_XLMAC_REG_CONFIG_MAC_MODE_FIELD_WIDTH,
    XPORT_XLMAC_REG_CONFIG_MAC_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_XLMAC_REG_CONFIG_OSTS_TIMER_DISABLE
 ******************************************************************************/
const ru_field_rec XPORT_XLMAC_REG_CONFIG_OSTS_TIMER_DISABLE_FIELD =
{
    "OSTS_TIMER_DISABLE",
#if RU_INCLUDE_DESC
    "",
    "OSTS time-stamping disable.\n"
    "0 : OSTS Enabled.\n"
    "1 : OSTS Disabled.\n",
#endif
    XPORT_XLMAC_REG_CONFIG_OSTS_TIMER_DISABLE_FIELD_MASK,
    0,
    XPORT_XLMAC_REG_CONFIG_OSTS_TIMER_DISABLE_FIELD_WIDTH,
    XPORT_XLMAC_REG_CONFIG_OSTS_TIMER_DISABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_XLMAC_REG_CONFIG_BYPASS_OSTS
 ******************************************************************************/
const ru_field_rec XPORT_XLMAC_REG_CONFIG_BYPASS_OSTS_FIELD =
{
    "BYPASS_OSTS",
#if RU_INCLUDE_DESC
    "",
    "Bypasses transmit OSTS functionality. When set, reduces Tx path latency.\n"
    "0 : Do not bypass transmit OSTS function.\n"
    "1 : Bypass transmit OSTS function.\n"
    "XLMAC must be reset for this bit to take effect.",
#endif
    XPORT_XLMAC_REG_CONFIG_BYPASS_OSTS_FIELD_MASK,
    0,
    XPORT_XLMAC_REG_CONFIG_BYPASS_OSTS_FIELD_WIDTH,
    XPORT_XLMAC_REG_CONFIG_BYPASS_OSTS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_XLMAC_REG_CONFIG_EGR_1588_TIMESTAMPING_MODE
 ******************************************************************************/
const ru_field_rec XPORT_XLMAC_REG_CONFIG_EGR_1588_TIMESTAMPING_MODE_FIELD =
{
    "EGR_1588_TIMESTAMPING_MODE",
#if RU_INCLUDE_DESC
    "",
    "1588 Egress Time-stamping mode.\n"
    "0 : Legacy, sign extended 32-bit timestamp mode.\n"
    "1 : 48-bit timestamp mode.\n"
    "XLMAC must be reset for this bit to take effect.",
#endif
    XPORT_XLMAC_REG_CONFIG_EGR_1588_TIMESTAMPING_MODE_FIELD_MASK,
    0,
    XPORT_XLMAC_REG_CONFIG_EGR_1588_TIMESTAMPING_MODE_FIELD_WIDTH,
    XPORT_XLMAC_REG_CONFIG_EGR_1588_TIMESTAMPING_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_XLMAC_REG_INTERRUPT_CHECK_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_XLMAC_REG_INTERRUPT_CHECK_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_XLMAC_REG_INTERRUPT_CHECK_RESERVED0_FIELD_MASK,
    0,
    XPORT_XLMAC_REG_INTERRUPT_CHECK_RESERVED0_FIELD_WIDTH,
    XPORT_XLMAC_REG_INTERRUPT_CHECK_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_XLMAC_REG_INTERRUPT_CHECK_XLMAC_INTR_CHECK
 ******************************************************************************/
const ru_field_rec XPORT_XLMAC_REG_INTERRUPT_CHECK_XLMAC_INTR_CHECK_FIELD =
{
    "XLMAC_INTR_CHECK",
#if RU_INCLUDE_DESC
    "",
    "Each bit of this field corresponds to one XLMAC port."
    "SW should write 1 to the corresponding bit(s) of this field any time XLMAC interrupt is in use and "
    "all events obtained by reading XLMAC status register are serviced and "
    "corresponding statuses cleared. Prevents XLMAC interrupt race condition.",
#endif
    XPORT_XLMAC_REG_INTERRUPT_CHECK_XLMAC_INTR_CHECK_FIELD_MASK,
    0,
    XPORT_XLMAC_REG_INTERRUPT_CHECK_XLMAC_INTR_CHECK_FIELD_WIDTH,
    XPORT_XLMAC_REG_INTERRUPT_CHECK_XLMAC_INTR_CHECK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_XLMAC_REG_PORT_0_RXERR_MASK_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_XLMAC_REG_PORT_0_RXERR_MASK_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_XLMAC_REG_PORT_0_RXERR_MASK_RESERVED0_FIELD_MASK,
    0,
    XPORT_XLMAC_REG_PORT_0_RXERR_MASK_RESERVED0_FIELD_WIDTH,
    XPORT_XLMAC_REG_PORT_0_RXERR_MASK_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_XLMAC_REG_PORT_0_RXERR_MASK_RSV_ERR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_XLMAC_REG_PORT_0_RXERR_MASK_RSV_ERR_MASK_FIELD =
{
    "RSV_ERR_MASK",
#if RU_INCLUDE_DESC
    "",
    "The RXERR will be set if both the mask bit & the corresponding statistics bit in RSV[37:16] are set. "
    "RSV[23] which indicates good packet received is excluded from generating RXERR.",
#endif
    XPORT_XLMAC_REG_PORT_0_RXERR_MASK_RSV_ERR_MASK_FIELD_MASK,
    0,
    XPORT_XLMAC_REG_PORT_0_RXERR_MASK_RSV_ERR_MASK_FIELD_WIDTH,
    XPORT_XLMAC_REG_PORT_0_RXERR_MASK_RSV_ERR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_XLMAC_REG_PORT_1_RXERR_MASK_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_XLMAC_REG_PORT_1_RXERR_MASK_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_XLMAC_REG_PORT_1_RXERR_MASK_RESERVED0_FIELD_MASK,
    0,
    XPORT_XLMAC_REG_PORT_1_RXERR_MASK_RESERVED0_FIELD_WIDTH,
    XPORT_XLMAC_REG_PORT_1_RXERR_MASK_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_XLMAC_REG_PORT_1_RXERR_MASK_RSV_ERR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_XLMAC_REG_PORT_1_RXERR_MASK_RSV_ERR_MASK_FIELD =
{
    "RSV_ERR_MASK",
#if RU_INCLUDE_DESC
    "",
    "The RXERR will be set if both the mask bit & the corresponding statistics bit in RSV[37:16] are set. "
    "RSV[23] which indicates good packet received is excluded from generating RXERR.",
#endif
    XPORT_XLMAC_REG_PORT_1_RXERR_MASK_RSV_ERR_MASK_FIELD_MASK,
    0,
    XPORT_XLMAC_REG_PORT_1_RXERR_MASK_RSV_ERR_MASK_FIELD_WIDTH,
    XPORT_XLMAC_REG_PORT_1_RXERR_MASK_RSV_ERR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_XLMAC_REG_PORT_2_RXERR_MASK_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_XLMAC_REG_PORT_2_RXERR_MASK_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_XLMAC_REG_PORT_2_RXERR_MASK_RESERVED0_FIELD_MASK,
    0,
    XPORT_XLMAC_REG_PORT_2_RXERR_MASK_RESERVED0_FIELD_WIDTH,
    XPORT_XLMAC_REG_PORT_2_RXERR_MASK_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_XLMAC_REG_PORT_2_RXERR_MASK_RSV_ERR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_XLMAC_REG_PORT_2_RXERR_MASK_RSV_ERR_MASK_FIELD =
{
    "RSV_ERR_MASK",
#if RU_INCLUDE_DESC
    "",
    "The RXERR will be set if both the mask bit & the corresponding statistics bit in RSV[37:16] are set. "
    "RSV[23] which indicates good packet received is excluded from generating RXERR.",
#endif
    XPORT_XLMAC_REG_PORT_2_RXERR_MASK_RSV_ERR_MASK_FIELD_MASK,
    0,
    XPORT_XLMAC_REG_PORT_2_RXERR_MASK_RSV_ERR_MASK_FIELD_WIDTH,
    XPORT_XLMAC_REG_PORT_2_RXERR_MASK_RSV_ERR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_XLMAC_REG_PORT_3_RXERR_MASK_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_XLMAC_REG_PORT_3_RXERR_MASK_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_XLMAC_REG_PORT_3_RXERR_MASK_RESERVED0_FIELD_MASK,
    0,
    XPORT_XLMAC_REG_PORT_3_RXERR_MASK_RESERVED0_FIELD_WIDTH,
    XPORT_XLMAC_REG_PORT_3_RXERR_MASK_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_XLMAC_REG_PORT_3_RXERR_MASK_RSV_ERR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_XLMAC_REG_PORT_3_RXERR_MASK_RSV_ERR_MASK_FIELD =
{
    "RSV_ERR_MASK",
#if RU_INCLUDE_DESC
    "",
    "The RXERR will be set if both the mask bit & the corresponding statistics bit in RSV[37:16] are set. "
    "RSV[23] which indicates good packet received is excluded from generating RXERR.",
#endif
    XPORT_XLMAC_REG_PORT_3_RXERR_MASK_RSV_ERR_MASK_FIELD_MASK,
    0,
    XPORT_XLMAC_REG_PORT_3_RXERR_MASK_RSV_ERR_MASK_FIELD_WIDTH,
    XPORT_XLMAC_REG_PORT_3_RXERR_MASK_RSV_ERR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_XLMAC_REG_RMT_LPBK_CNTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_XLMAC_REG_RMT_LPBK_CNTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_XLMAC_REG_RMT_LPBK_CNTRL_RESERVED0_FIELD_MASK,
    0,
    XPORT_XLMAC_REG_RMT_LPBK_CNTRL_RESERVED0_FIELD_WIDTH,
    XPORT_XLMAC_REG_RMT_LPBK_CNTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_XLMAC_REG_RMT_LPBK_CNTRL_READ_THRESHOLD
 ******************************************************************************/
const ru_field_rec XPORT_XLMAC_REG_RMT_LPBK_CNTRL_READ_THRESHOLD_FIELD =
{
    "READ_THRESHOLD",
#if RU_INCLUDE_DESC
    "",
    "Remote loopback logic starts reading packet data from the loopback FIFO only "
    "when at least READ_THRESHOLD entries are available in the FIFO. "
    "Used to prevent XLMAC TX underflow. ",
#endif
    XPORT_XLMAC_REG_RMT_LPBK_CNTRL_READ_THRESHOLD_FIELD_MASK,
    0,
    XPORT_XLMAC_REG_RMT_LPBK_CNTRL_READ_THRESHOLD_FIELD_WIDTH,
    XPORT_XLMAC_REG_RMT_LPBK_CNTRL_READ_THRESHOLD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_XLMAC_REG_RMT_LPBK_CNTRL_TX_PORT_ID
 ******************************************************************************/
const ru_field_rec XPORT_XLMAC_REG_RMT_LPBK_CNTRL_TX_PORT_ID_FIELD =
{
    "TX_PORT_ID",
#if RU_INCLUDE_DESC
    "",
    "TX PORT_ID[1:0]. Valid only when TX_PORT_SEL = 1.",
#endif
    XPORT_XLMAC_REG_RMT_LPBK_CNTRL_TX_PORT_ID_FIELD_MASK,
    0,
    XPORT_XLMAC_REG_RMT_LPBK_CNTRL_TX_PORT_ID_FIELD_WIDTH,
    XPORT_XLMAC_REG_RMT_LPBK_CNTRL_TX_PORT_ID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_XLMAC_REG_RMT_LPBK_CNTRL_TX_PORT_SEL
 ******************************************************************************/
const ru_field_rec XPORT_XLMAC_REG_RMT_LPBK_CNTRL_TX_PORT_SEL_FIELD =
{
    "TX_PORT_SEL",
#if RU_INCLUDE_DESC
    "",
    "When set TX PORT_ID[1:0] comes from this registers. "
    "When cleared TX PORT_ID[1:0] equals RX PORT_ID[1:0]. "
    "TX PORT_ID[1:0] is used by remote loopback logic to monitor "
    "EP credits and to indicate outgoing XLMAC port.",
#endif
    XPORT_XLMAC_REG_RMT_LPBK_CNTRL_TX_PORT_SEL_FIELD_MASK,
    0,
    XPORT_XLMAC_REG_RMT_LPBK_CNTRL_TX_PORT_SEL_FIELD_WIDTH,
    XPORT_XLMAC_REG_RMT_LPBK_CNTRL_TX_PORT_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_XLMAC_REG_RMT_LPBK_CNTRL_RXERR_EN
 ******************************************************************************/
const ru_field_rec XPORT_XLMAC_REG_RMT_LPBK_CNTRL_RXERR_EN_FIELD =
{
    "RXERR_EN",
#if RU_INCLUDE_DESC
    "",
    "When set RXERR is propagated to TXERR. When cleared TXERR = 0.",
#endif
    XPORT_XLMAC_REG_RMT_LPBK_CNTRL_RXERR_EN_FIELD_MASK,
    0,
    XPORT_XLMAC_REG_RMT_LPBK_CNTRL_RXERR_EN_FIELD_WIDTH,
    XPORT_XLMAC_REG_RMT_LPBK_CNTRL_RXERR_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_XLMAC_REG_RMT_LPBK_CNTRL_TX_CRC_ERR
 ******************************************************************************/
const ru_field_rec XPORT_XLMAC_REG_RMT_LPBK_CNTRL_TX_CRC_ERR_FIELD =
{
    "TX_CRC_ERR",
#if RU_INCLUDE_DESC
    "",
    "When set CRC is corrupted for the outgoing packet.",
#endif
    XPORT_XLMAC_REG_RMT_LPBK_CNTRL_TX_CRC_ERR_FIELD_MASK,
    0,
    XPORT_XLMAC_REG_RMT_LPBK_CNTRL_TX_CRC_ERR_FIELD_WIDTH,
    XPORT_XLMAC_REG_RMT_LPBK_CNTRL_TX_CRC_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_XLMAC_REG_RMT_LPBK_CNTRL_TX_CRC_MODE
 ******************************************************************************/
const ru_field_rec XPORT_XLMAC_REG_RMT_LPBK_CNTRL_TX_CRC_MODE_FIELD =
{
    "TX_CRC_MODE",
#if RU_INCLUDE_DESC
    "",
    "TX CRC Mode. Encoded as:\n"
    "00 : CRC Append.\n"
    "01 : CRC Forward.\n"
    "10 : CRC Replace.\n"
    "11 : Reserved.\n"
    "CRC Append mode should be enabled only if XLMAC is programmed "
    "to strip off CRC.",
#endif
    XPORT_XLMAC_REG_RMT_LPBK_CNTRL_TX_CRC_MODE_FIELD_MASK,
    0,
    XPORT_XLMAC_REG_RMT_LPBK_CNTRL_TX_CRC_MODE_FIELD_WIDTH,
    XPORT_XLMAC_REG_RMT_LPBK_CNTRL_TX_CRC_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_XLMAC_REG_RMT_LPBK_CNTRL_RMT_LOOPBACK_EN
 ******************************************************************************/
const ru_field_rec XPORT_XLMAC_REG_RMT_LPBK_CNTRL_RMT_LOOPBACK_EN_FIELD =
{
    "RMT_LOOPBACK_EN",
#if RU_INCLUDE_DESC
    "",
    "When set enables XLMAC Remote (RX to TX) loopback. "
    "XLMAC must be kept in reset while remote loopback is "
    "being enabled and released from the reset thereafter.",
#endif
    XPORT_XLMAC_REG_RMT_LPBK_CNTRL_RMT_LOOPBACK_EN_FIELD_MASK,
    0,
    XPORT_XLMAC_REG_RMT_LPBK_CNTRL_RMT_LOOPBACK_EN_FIELD_WIDTH,
    XPORT_XLMAC_REG_RMT_LPBK_CNTRL_RMT_LOOPBACK_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: XPORT_XLMAC_REG_DIR_ACC_DATA_WRITE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_XLMAC_REG_DIR_ACC_DATA_WRITE_FIELDS[] =
{
    &XPORT_XLMAC_REG_DIR_ACC_DATA_WRITE_WRITE_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_XLMAC_REG_DIR_ACC_DATA_WRITE_REG = 
{
    "DIR_ACC_DATA_WRITE",
#if RU_INCLUDE_DESC
    "XLMAC 32-bit Direct Access Data Write Register",
    "",
#endif
    XPORT_XLMAC_REG_DIR_ACC_DATA_WRITE_REG_OFFSET,
    0,
    0,
    183,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPORT_XLMAC_REG_DIR_ACC_DATA_WRITE_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_XLMAC_REG_DIR_ACC_DATA_READ
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_XLMAC_REG_DIR_ACC_DATA_READ_FIELDS[] =
{
    &XPORT_XLMAC_REG_DIR_ACC_DATA_READ_READ_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_XLMAC_REG_DIR_ACC_DATA_READ_REG = 
{
    "DIR_ACC_DATA_READ",
#if RU_INCLUDE_DESC
    "XLMAC 32-bit Direct Access Data Read Register",
    "",
#endif
    XPORT_XLMAC_REG_DIR_ACC_DATA_READ_REG_OFFSET,
    0,
    0,
    184,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPORT_XLMAC_REG_DIR_ACC_DATA_READ_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_XLMAC_REG_INDIR_ACC_ADDR_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_XLMAC_REG_INDIR_ACC_ADDR_0_FIELDS[] =
{
    &XPORT_XLMAC_REG_INDIR_ACC_ADDR_0_RESERVED0_FIELD,
    &XPORT_XLMAC_REG_INDIR_ACC_ADDR_0_ERR_FIELD,
    &XPORT_XLMAC_REG_INDIR_ACC_ADDR_0_START_BUSY_FIELD,
    &XPORT_XLMAC_REG_INDIR_ACC_ADDR_0_R_W_FIELD,
    &XPORT_XLMAC_REG_INDIR_ACC_ADDR_0_REG_PORT_ID_FIELD,
    &XPORT_XLMAC_REG_INDIR_ACC_ADDR_0_REG_OFFSET_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_XLMAC_REG_INDIR_ACC_ADDR_0_REG = 
{
    "INDIR_ACC_ADDR_0",
#if RU_INCLUDE_DESC
    "XLMAC Indirect Access Address Register",
    "",
#endif
    XPORT_XLMAC_REG_INDIR_ACC_ADDR_0_REG_OFFSET,
    0,
    0,
    185,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    XPORT_XLMAC_REG_INDIR_ACC_ADDR_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_XLMAC_REG_INDIR_ACC_DATA_LOW_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_XLMAC_REG_INDIR_ACC_DATA_LOW_0_FIELDS[] =
{
    &XPORT_XLMAC_REG_INDIR_ACC_DATA_LOW_0_DATA_LOW_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_XLMAC_REG_INDIR_ACC_DATA_LOW_0_REG = 
{
    "INDIR_ACC_DATA_LOW_0",
#if RU_INCLUDE_DESC
    "XLMAC Indirect Access Data Low Register",
    "",
#endif
    XPORT_XLMAC_REG_INDIR_ACC_DATA_LOW_0_REG_OFFSET,
    0,
    0,
    186,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPORT_XLMAC_REG_INDIR_ACC_DATA_LOW_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_XLMAC_REG_INDIR_ACC_DATA_HIGH_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_XLMAC_REG_INDIR_ACC_DATA_HIGH_0_FIELDS[] =
{
    &XPORT_XLMAC_REG_INDIR_ACC_DATA_HIGH_0_DATA_HIGH_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_XLMAC_REG_INDIR_ACC_DATA_HIGH_0_REG = 
{
    "INDIR_ACC_DATA_HIGH_0",
#if RU_INCLUDE_DESC
    "XLMAC Indirect Access Data High Register",
    "",
#endif
    XPORT_XLMAC_REG_INDIR_ACC_DATA_HIGH_0_REG_OFFSET,
    0,
    0,
    187,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPORT_XLMAC_REG_INDIR_ACC_DATA_HIGH_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_XLMAC_REG_INDIR_ACC_ADDR_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_XLMAC_REG_INDIR_ACC_ADDR_1_FIELDS[] =
{
    &XPORT_XLMAC_REG_INDIR_ACC_ADDR_1_RESERVED0_FIELD,
    &XPORT_XLMAC_REG_INDIR_ACC_ADDR_1_ERR_FIELD,
    &XPORT_XLMAC_REG_INDIR_ACC_ADDR_1_START_BUSY_FIELD,
    &XPORT_XLMAC_REG_INDIR_ACC_ADDR_1_R_W_FIELD,
    &XPORT_XLMAC_REG_INDIR_ACC_ADDR_1_REG_PORT_ID_FIELD,
    &XPORT_XLMAC_REG_INDIR_ACC_ADDR_1_REG_OFFSET_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_XLMAC_REG_INDIR_ACC_ADDR_1_REG = 
{
    "INDIR_ACC_ADDR_1",
#if RU_INCLUDE_DESC
    "XLMAC Indirect Access Address Register",
    "",
#endif
    XPORT_XLMAC_REG_INDIR_ACC_ADDR_1_REG_OFFSET,
    0,
    0,
    188,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    XPORT_XLMAC_REG_INDIR_ACC_ADDR_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_XLMAC_REG_INDIR_ACC_DATA_LOW_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_XLMAC_REG_INDIR_ACC_DATA_LOW_1_FIELDS[] =
{
    &XPORT_XLMAC_REG_INDIR_ACC_DATA_LOW_1_DATA_LOW_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_XLMAC_REG_INDIR_ACC_DATA_LOW_1_REG = 
{
    "INDIR_ACC_DATA_LOW_1",
#if RU_INCLUDE_DESC
    "XLMAC Indirect Access Data Low Register",
    "",
#endif
    XPORT_XLMAC_REG_INDIR_ACC_DATA_LOW_1_REG_OFFSET,
    0,
    0,
    189,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPORT_XLMAC_REG_INDIR_ACC_DATA_LOW_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_XLMAC_REG_INDIR_ACC_DATA_HIGH_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_XLMAC_REG_INDIR_ACC_DATA_HIGH_1_FIELDS[] =
{
    &XPORT_XLMAC_REG_INDIR_ACC_DATA_HIGH_1_DATA_HIGH_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_XLMAC_REG_INDIR_ACC_DATA_HIGH_1_REG = 
{
    "INDIR_ACC_DATA_HIGH_1",
#if RU_INCLUDE_DESC
    "XLMAC Indirect Access Data High Register",
    "",
#endif
    XPORT_XLMAC_REG_INDIR_ACC_DATA_HIGH_1_REG_OFFSET,
    0,
    0,
    190,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPORT_XLMAC_REG_INDIR_ACC_DATA_HIGH_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_XLMAC_REG_CONFIG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_XLMAC_REG_CONFIG_FIELDS[] =
{
    &XPORT_XLMAC_REG_CONFIG_RESERVED0_FIELD,
    &XPORT_XLMAC_REG_CONFIG_XLMAC_RESET_FIELD,
    &XPORT_XLMAC_REG_CONFIG_RX_DUAL_CYCLE_TDM_EN_FIELD,
    &XPORT_XLMAC_REG_CONFIG_RX_NON_LINEAR_QUAD_TDM_EN_FIELD,
    &XPORT_XLMAC_REG_CONFIG_RX_FLEX_TDM_ENABLE_FIELD,
    &XPORT_XLMAC_REG_CONFIG_MAC_MODE_FIELD,
    &XPORT_XLMAC_REG_CONFIG_OSTS_TIMER_DISABLE_FIELD,
    &XPORT_XLMAC_REG_CONFIG_BYPASS_OSTS_FIELD,
    &XPORT_XLMAC_REG_CONFIG_EGR_1588_TIMESTAMPING_MODE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_XLMAC_REG_CONFIG_REG = 
{
    "CONFIG",
#if RU_INCLUDE_DESC
    "XLMAC Configure Register",
    "",
#endif
    XPORT_XLMAC_REG_CONFIG_REG_OFFSET,
    0,
    0,
    191,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    XPORT_XLMAC_REG_CONFIG_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_XLMAC_REG_INTERRUPT_CHECK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_XLMAC_REG_INTERRUPT_CHECK_FIELDS[] =
{
    &XPORT_XLMAC_REG_INTERRUPT_CHECK_RESERVED0_FIELD,
    &XPORT_XLMAC_REG_INTERRUPT_CHECK_XLMAC_INTR_CHECK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_XLMAC_REG_INTERRUPT_CHECK_REG = 
{
    "INTERRUPT_CHECK",
#if RU_INCLUDE_DESC
    "XLMAC Interrupt Check Register",
    "",
#endif
    XPORT_XLMAC_REG_INTERRUPT_CHECK_REG_OFFSET,
    0,
    0,
    192,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_XLMAC_REG_INTERRUPT_CHECK_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_XLMAC_REG_PORT_0_RXERR_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_XLMAC_REG_PORT_0_RXERR_MASK_FIELDS[] =
{
    &XPORT_XLMAC_REG_PORT_0_RXERR_MASK_RESERVED0_FIELD,
    &XPORT_XLMAC_REG_PORT_0_RXERR_MASK_RSV_ERR_MASK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_XLMAC_REG_PORT_0_RXERR_MASK_REG = 
{
    "PORT_0_RXERR_MASK",
#if RU_INCLUDE_DESC
    "XLMAC Port 3 RXERR Mask Register",
    "",
#endif
    XPORT_XLMAC_REG_PORT_0_RXERR_MASK_REG_OFFSET,
    0,
    0,
    193,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_XLMAC_REG_PORT_0_RXERR_MASK_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_XLMAC_REG_PORT_1_RXERR_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_XLMAC_REG_PORT_1_RXERR_MASK_FIELDS[] =
{
    &XPORT_XLMAC_REG_PORT_1_RXERR_MASK_RESERVED0_FIELD,
    &XPORT_XLMAC_REG_PORT_1_RXERR_MASK_RSV_ERR_MASK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_XLMAC_REG_PORT_1_RXERR_MASK_REG = 
{
    "PORT_1_RXERR_MASK",
#if RU_INCLUDE_DESC
    "XLMAC Port 3 RXERR Mask Register",
    "",
#endif
    XPORT_XLMAC_REG_PORT_1_RXERR_MASK_REG_OFFSET,
    0,
    0,
    194,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_XLMAC_REG_PORT_1_RXERR_MASK_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_XLMAC_REG_PORT_2_RXERR_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_XLMAC_REG_PORT_2_RXERR_MASK_FIELDS[] =
{
    &XPORT_XLMAC_REG_PORT_2_RXERR_MASK_RESERVED0_FIELD,
    &XPORT_XLMAC_REG_PORT_2_RXERR_MASK_RSV_ERR_MASK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_XLMAC_REG_PORT_2_RXERR_MASK_REG = 
{
    "PORT_2_RXERR_MASK",
#if RU_INCLUDE_DESC
    "XLMAC Port 3 RXERR Mask Register",
    "",
#endif
    XPORT_XLMAC_REG_PORT_2_RXERR_MASK_REG_OFFSET,
    0,
    0,
    195,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_XLMAC_REG_PORT_2_RXERR_MASK_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_XLMAC_REG_PORT_3_RXERR_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_XLMAC_REG_PORT_3_RXERR_MASK_FIELDS[] =
{
    &XPORT_XLMAC_REG_PORT_3_RXERR_MASK_RESERVED0_FIELD,
    &XPORT_XLMAC_REG_PORT_3_RXERR_MASK_RSV_ERR_MASK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_XLMAC_REG_PORT_3_RXERR_MASK_REG = 
{
    "PORT_3_RXERR_MASK",
#if RU_INCLUDE_DESC
    "XLMAC Port 3 RXERR Mask Register",
    "",
#endif
    XPORT_XLMAC_REG_PORT_3_RXERR_MASK_REG_OFFSET,
    0,
    0,
    196,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_XLMAC_REG_PORT_3_RXERR_MASK_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_XLMAC_REG_RMT_LPBK_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_XLMAC_REG_RMT_LPBK_CNTRL_FIELDS[] =
{
    &XPORT_XLMAC_REG_RMT_LPBK_CNTRL_RESERVED0_FIELD,
    &XPORT_XLMAC_REG_RMT_LPBK_CNTRL_READ_THRESHOLD_FIELD,
    &XPORT_XLMAC_REG_RMT_LPBK_CNTRL_TX_PORT_ID_FIELD,
    &XPORT_XLMAC_REG_RMT_LPBK_CNTRL_TX_PORT_SEL_FIELD,
    &XPORT_XLMAC_REG_RMT_LPBK_CNTRL_RXERR_EN_FIELD,
    &XPORT_XLMAC_REG_RMT_LPBK_CNTRL_TX_CRC_ERR_FIELD,
    &XPORT_XLMAC_REG_RMT_LPBK_CNTRL_TX_CRC_MODE_FIELD,
    &XPORT_XLMAC_REG_RMT_LPBK_CNTRL_RMT_LOOPBACK_EN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_XLMAC_REG_RMT_LPBK_CNTRL_REG = 
{
    "RMT_LPBK_CNTRL",
#if RU_INCLUDE_DESC
    "XLMAC Remote Loopback Control Register",
    "",
#endif
    XPORT_XLMAC_REG_RMT_LPBK_CNTRL_REG_OFFSET,
    0,
    0,
    197,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    XPORT_XLMAC_REG_RMT_LPBK_CNTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: XPORT_XLMAC_REG
 ******************************************************************************/
static const ru_reg_rec *XPORT_XLMAC_REG_REGS[] =
{
    &XPORT_XLMAC_REG_DIR_ACC_DATA_WRITE_REG,
    &XPORT_XLMAC_REG_DIR_ACC_DATA_READ_REG,
    &XPORT_XLMAC_REG_INDIR_ACC_ADDR_0_REG,
    &XPORT_XLMAC_REG_INDIR_ACC_DATA_LOW_0_REG,
    &XPORT_XLMAC_REG_INDIR_ACC_DATA_HIGH_0_REG,
    &XPORT_XLMAC_REG_INDIR_ACC_ADDR_1_REG,
    &XPORT_XLMAC_REG_INDIR_ACC_DATA_LOW_1_REG,
    &XPORT_XLMAC_REG_INDIR_ACC_DATA_HIGH_1_REG,
    &XPORT_XLMAC_REG_CONFIG_REG,
    &XPORT_XLMAC_REG_INTERRUPT_CHECK_REG,
    &XPORT_XLMAC_REG_PORT_0_RXERR_MASK_REG,
    &XPORT_XLMAC_REG_PORT_1_RXERR_MASK_REG,
    &XPORT_XLMAC_REG_PORT_2_RXERR_MASK_REG,
    &XPORT_XLMAC_REG_PORT_3_RXERR_MASK_REG,
    &XPORT_XLMAC_REG_RMT_LPBK_CNTRL_REG,
};

unsigned long XPORT_XLMAC_REG_ADDRS[] =
{
    0x8013b000,
};

const ru_block_rec XPORT_XLMAC_REG_BLOCK = 
{
    "XPORT_XLMAC_REG",
    XPORT_XLMAC_REG_ADDRS,
    1,
    15,
    XPORT_XLMAC_REG_REGS
};

/* End of file XPORT_XLMAC_REG.c */
