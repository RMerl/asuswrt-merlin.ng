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
 * Field: GPON_GEARBOX_0_SW_RESET_TXPG_RESET
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_0_SW_RESET_TXPG_RESET_FIELD =
{
    "SW_RESET_TXPG_RESET",
#if RU_INCLUDE_DESC
    "SW_RESET_TXPG_RESET",
    "Tx Pattern Generator reset control",
#endif
    GPON_GEARBOX_0_SW_RESET_TXPG_RESET_FIELD_MASK,
    0,
    GPON_GEARBOX_0_SW_RESET_TXPG_RESET_FIELD_WIDTH,
    GPON_GEARBOX_0_SW_RESET_TXPG_RESET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_0_SW_RESET_TXFIFO_RESET
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_0_SW_RESET_TXFIFO_RESET_FIELD =
{
    "SW_RESET_TXFIFO_RESET",
#if RU_INCLUDE_DESC
    "SW_RESET_TXFIFO_RESET",
    "Tx FIFO reset control",
#endif
    GPON_GEARBOX_0_SW_RESET_TXFIFO_RESET_FIELD_MASK,
    0,
    GPON_GEARBOX_0_SW_RESET_TXFIFO_RESET_FIELD_WIDTH,
    GPON_GEARBOX_0_SW_RESET_TXFIFO_RESET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_0_FIFO_CFG_0_CLEAR_TXFIFO_DRIFTED
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_0_FIFO_CFG_0_CLEAR_TXFIFO_DRIFTED_FIELD =
{
    "FIFO_CFG_0_CLEAR_TXFIFO_DRIFTED",
#if RU_INCLUDE_DESC
    "FIFO_CFG_0_CLEAR_TXFIFO_DRIFTED",
    "If 1, the TXFIFO_DRIFTED status bit resets to 0."
    "Reset value is 0.",
#endif
    GPON_GEARBOX_0_FIFO_CFG_0_CLEAR_TXFIFO_DRIFTED_FIELD_MASK,
    0,
    GPON_GEARBOX_0_FIFO_CFG_0_CLEAR_TXFIFO_DRIFTED_FIELD_WIDTH,
    GPON_GEARBOX_0_FIFO_CFG_0_CLEAR_TXFIFO_DRIFTED_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_0_FIFO_CFG_0_LOOPBACK_RX
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_0_FIFO_CFG_0_LOOPBACK_RX_FIELD =
{
    "FIFO_CFG_0_LOOPBACK_RX",
#if RU_INCLUDE_DESC
    "FIFO_CFG_0_LOOPBACK_RX",
    "If 1, the output of Rx FIFO is looped back to the input of Tx FIFO. In this case, the SATA PHY Tx data rate is the same as the Rx data rate regardless of whether Gen2 or Gen3 is selected."
    "Reset value is 0.",
#endif
    GPON_GEARBOX_0_FIFO_CFG_0_LOOPBACK_RX_FIELD_MASK,
    0,
    GPON_GEARBOX_0_FIFO_CFG_0_LOOPBACK_RX_FIELD_WIDTH,
    GPON_GEARBOX_0_FIFO_CFG_0_LOOPBACK_RX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_0_FIFO_CFG_0_CLEAR_TXFIFO_COLLISION
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_0_FIFO_CFG_0_CLEAR_TXFIFO_COLLISION_FIELD =
{
    "FIFO_CFG_0_CLEAR_TXFIFO_COLLISION",
#if RU_INCLUDE_DESC
    "FIFO_CFG_0_CLEAR_TXFIFO_COLLISION",
    "If 1, the TXFIFO_COLLISION status bit resets to 0."
    "Reset value is 0.",
#endif
    GPON_GEARBOX_0_FIFO_CFG_0_CLEAR_TXFIFO_COLLISION_FIELD_MASK,
    0,
    GPON_GEARBOX_0_FIFO_CFG_0_CLEAR_TXFIFO_COLLISION_FIELD_WIDTH,
    GPON_GEARBOX_0_FIFO_CFG_0_CLEAR_TXFIFO_COLLISION_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_0_FIFO_CFG_0_TX_WR_PTR_ADV
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_0_FIFO_CFG_0_TX_WR_PTR_ADV_FIELD =
{
    "FIFO_CFG_0_TX_WR_PTR_ADV",
#if RU_INCLUDE_DESC
    "FIFO_CFG_0_TX_WR_PTR_ADV",
    "Advance Tx FIFO write pointer by 1 location (8 Tx bits). The pointer is adjusted on every 0 to 1 transition in this register field."
    "Reset value is 0.",
#endif
    GPON_GEARBOX_0_FIFO_CFG_0_TX_WR_PTR_ADV_FIELD_MASK,
    0,
    GPON_GEARBOX_0_FIFO_CFG_0_TX_WR_PTR_ADV_FIELD_WIDTH,
    GPON_GEARBOX_0_FIFO_CFG_0_TX_WR_PTR_ADV_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_0_FIFO_CFG_0_TX_WR_PTR_DLY
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_0_FIFO_CFG_0_TX_WR_PTR_DLY_FIELD =
{
    "FIFO_CFG_0_TX_WR_PTR_DLY",
#if RU_INCLUDE_DESC
    "FIFO_CFG_0_TX_WR_PTR_DLY",
    "Delay Tx FIFO write pointer by 1 location (8 Tx bits). The pointer is adjusted on every 0 to 1 transition in this register field."
    "Reset value is 0.",
#endif
    GPON_GEARBOX_0_FIFO_CFG_0_TX_WR_PTR_DLY_FIELD_MASK,
    0,
    GPON_GEARBOX_0_FIFO_CFG_0_TX_WR_PTR_DLY_FIELD_WIDTH,
    GPON_GEARBOX_0_FIFO_CFG_0_TX_WR_PTR_DLY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_0_FIFO_CFG_0_TX_BIT_INV
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_0_FIFO_CFG_0_TX_BIT_INV_FIELD =
{
    "FIFO_CFG_0_TX_BIT_INV",
#if RU_INCLUDE_DESC
    "FIFO_CFG_0_TX_BIT_INV",
    "This bit enables logically inversion of every Tx bit."
    "Reset value is 0.",
#endif
    GPON_GEARBOX_0_FIFO_CFG_0_TX_BIT_INV_FIELD_MASK,
    0,
    GPON_GEARBOX_0_FIFO_CFG_0_TX_BIT_INV_FIELD_WIDTH,
    GPON_GEARBOX_0_FIFO_CFG_0_TX_BIT_INV_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_0_FIFO_CFG_0_TX_PTR_DIST_MAX
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_0_FIFO_CFG_0_TX_PTR_DIST_MAX_FIELD =
{
    "FIFO_CFG_0_TX_PTR_DIST_MAX",
#if RU_INCLUDE_DESC
    "FIFO_CFG_0_TX_PTR_DIST_MAX",
    "Maximum distance allowed between the Tx FIFO write and read pointers. The TXFIFO_DRIFTED status bit is asserted if TX_POINTER_DISTANCE goes above this maximum value."
    "Reset value is 3.",
#endif
    GPON_GEARBOX_0_FIFO_CFG_0_TX_PTR_DIST_MAX_FIELD_MASK,
    0,
    GPON_GEARBOX_0_FIFO_CFG_0_TX_PTR_DIST_MAX_FIELD_WIDTH,
    GPON_GEARBOX_0_FIFO_CFG_0_TX_PTR_DIST_MAX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_0_FIFO_CFG_0_ASYM_LOOPBACK
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_0_FIFO_CFG_0_ASYM_LOOPBACK_FIELD =
{
    "FIFO_CFG_0_ASYM_LOOPBACK",
#if RU_INCLUDE_DESC
    "FIFO_CFG_0_ASYM_LOOPBACK",
    "GPON asymetric loopback",
#endif
    GPON_GEARBOX_0_FIFO_CFG_0_ASYM_LOOPBACK_FIELD_MASK,
    0,
    GPON_GEARBOX_0_FIFO_CFG_0_ASYM_LOOPBACK_FIELD_WIDTH,
    GPON_GEARBOX_0_FIFO_CFG_0_ASYM_LOOPBACK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_0_RESERVED0
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_0_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    GPON_GEARBOX_0_RESERVED0_FIELD_MASK,
    0,
    GPON_GEARBOX_0_RESERVED0_FIELD_WIDTH,
    GPON_GEARBOX_0_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_0_FIFO_CFG_0_TX_PTR_DIST_MIN
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_0_FIFO_CFG_0_TX_PTR_DIST_MIN_FIELD =
{
    "FIFO_CFG_0_TX_PTR_DIST_MIN",
#if RU_INCLUDE_DESC
    "FIFO_CFG_0_TX_PTR_DIST_MIN",
    "Minimum distance allowed between the Tx FIFO write and read pointers. The TXFIFO_DRIFTED status bit is asserted if TX_POINTER_DISTANCE goes below this minimum value."
    "Reset value is 1.",
#endif
    GPON_GEARBOX_0_FIFO_CFG_0_TX_PTR_DIST_MIN_FIELD_MASK,
    0,
    GPON_GEARBOX_0_FIFO_CFG_0_TX_PTR_DIST_MIN_FIELD_WIDTH,
    GPON_GEARBOX_0_FIFO_CFG_0_TX_PTR_DIST_MIN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_0_RESERVED1
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_0_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    GPON_GEARBOX_0_RESERVED1_FIELD_MASK,
    0,
    GPON_GEARBOX_0_RESERVED1_FIELD_WIDTH,
    GPON_GEARBOX_0_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_0_FIFO_CFG_0_TX_20BIT_ORDER
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_0_FIFO_CFG_0_TX_20BIT_ORDER_FIELD =
{
    "FIFO_CFG_0_TX_20BIT_ORDER",
#if RU_INCLUDE_DESC
    "FIFO_CFG_0_TX_20BIT_ORDER",
    "This field changes the bit order of the 20-bit Tx data exiting the Tx FIFO to SATA PHY."
    "0: Bit  0 is transmitted first"
    "1: Bit 19 is transmitted first"
    ""
    "Reset value is 1.",
#endif
    GPON_GEARBOX_0_FIFO_CFG_0_TX_20BIT_ORDER_FIELD_MASK,
    0,
    GPON_GEARBOX_0_FIFO_CFG_0_TX_20BIT_ORDER_FIELD_WIDTH,
    GPON_GEARBOX_0_FIFO_CFG_0_TX_20BIT_ORDER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_0_FIFO_CFG_0_TX_8BIT_ORDER
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_0_FIFO_CFG_0_TX_8BIT_ORDER_FIELD =
{
    "FIFO_CFG_0_TX_8BIT_ORDER",
#if RU_INCLUDE_DESC
    "FIFO_CFG_0_TX_8BIT_ORDER",
    "This field changes the bit order of the 8-bit Tx data entering the Tx FIFO."
    "0: No changes"
    "1: Tx data is reversed from [7:0] to [0:7]"
    ""
    "Reset value is 0.",
#endif
    GPON_GEARBOX_0_FIFO_CFG_0_TX_8BIT_ORDER_FIELD_MASK,
    0,
    GPON_GEARBOX_0_FIFO_CFG_0_TX_8BIT_ORDER_FIELD_WIDTH,
    GPON_GEARBOX_0_FIFO_CFG_0_TX_8BIT_ORDER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_0_FIFO_CFG_0_RX_16BIT_ORDER
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_0_FIFO_CFG_0_RX_16BIT_ORDER_FIELD =
{
    "FIFO_CFG_0_RX_16BIT_ORDER",
#if RU_INCLUDE_DESC
    "FIFO_CFG_0_RX_16BIT_ORDER",
    "This field changes the bit order of the 16-bit Rx data exiting the Rx FIFO to GPON MAC."
    "0: No changes"
    "1: Rx data is reversed from [15:0] to [0:15]"
    ""
    "Reset value is 0.",
#endif
    GPON_GEARBOX_0_FIFO_CFG_0_RX_16BIT_ORDER_FIELD_MASK,
    0,
    GPON_GEARBOX_0_FIFO_CFG_0_RX_16BIT_ORDER_FIELD_WIDTH,
    GPON_GEARBOX_0_FIFO_CFG_0_RX_16BIT_ORDER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_0_FIFO_CFG_0_TXLBE_BIT_ORDER
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_0_FIFO_CFG_0_TXLBE_BIT_ORDER_FIELD =
{
    "FIFO_CFG_0_TXLBE_BIT_ORDER",
#if RU_INCLUDE_DESC
    "FIFO_CFG_0_TXLBE_BIT_ORDER",
    "TBD",
#endif
    GPON_GEARBOX_0_FIFO_CFG_0_TXLBE_BIT_ORDER_FIELD_MASK,
    0,
    GPON_GEARBOX_0_FIFO_CFG_0_TXLBE_BIT_ORDER_FIELD_WIDTH,
    GPON_GEARBOX_0_FIFO_CFG_0_TXLBE_BIT_ORDER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_0_FIFO_STATUS_SEL
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_0_FIFO_STATUS_SEL_FIELD =
{
    "FIFO_STATUS_SEL",
#if RU_INCLUDE_DESC
    "FIFO_STATUS_SEL",
    "TBD",
#endif
    GPON_GEARBOX_0_FIFO_STATUS_SEL_FIELD_MASK,
    0,
    GPON_GEARBOX_0_FIFO_STATUS_SEL_FIELD_WIDTH,
    GPON_GEARBOX_0_FIFO_STATUS_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_0_PTG_STATUS1_SEL
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_0_PTG_STATUS1_SEL_FIELD =
{
    "PTG_STATUS1_SEL",
#if RU_INCLUDE_DESC
    "PTG_STATUS1_SEL",
    "TBD",
#endif
    GPON_GEARBOX_0_PTG_STATUS1_SEL_FIELD_MASK,
    0,
    GPON_GEARBOX_0_PTG_STATUS1_SEL_FIELD_WIDTH,
    GPON_GEARBOX_0_PTG_STATUS1_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_0_PTG_STATUS2_SEL
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_0_PTG_STATUS2_SEL_FIELD =
{
    "PTG_STATUS2_SEL",
#if RU_INCLUDE_DESC
    "PTG_STATUS2_SEL",
    "TBD",
#endif
    GPON_GEARBOX_0_PTG_STATUS2_SEL_FIELD_MASK,
    0,
    GPON_GEARBOX_0_PTG_STATUS2_SEL_FIELD_WIDTH,
    GPON_GEARBOX_0_PTG_STATUS2_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_0_RESERVED2
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_0_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    GPON_GEARBOX_0_RESERVED2_FIELD_MASK,
    0,
    GPON_GEARBOX_0_RESERVED2_FIELD_WIDTH,
    GPON_GEARBOX_0_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_PATTERN_CFG1_PG_MODE
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_PATTERN_CFG1_PG_MODE_FIELD =
{
    "PG_MODE",
#if RU_INCLUDE_DESC
    "PG_MODE",
    "Pattern generator modes:"
    "0: Pattern generator disabled. GPON MAC has control of Tx output and burst enable."
    "1: Generate repetitive Tx bursts. Each burst consists of 1 header byte and"
    "1 or more payload bytes. Filler bytes are placed between Tx bursts."
    "2: Reserved"
    "3: Reserved"
    "4: Generate PRBS7 pattern"
    "5: Generate PRBS15 pattern"
    "6: Generate PRBS23 pattern"
    "7: Generate PRBS31 pattern"
    "Mode 0 is for GPON normal operation.  Mode 1 is for laser burst enable calibration."
    "Reset value is 0.",
#endif
    GPON_GEARBOX_PATTERN_CFG1_PG_MODE_FIELD_MASK,
    0,
    GPON_GEARBOX_PATTERN_CFG1_PG_MODE_FIELD_WIDTH,
    GPON_GEARBOX_PATTERN_CFG1_PG_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_PATTERN_CFG1_RESERVED0
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_PATTERN_CFG1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    GPON_GEARBOX_PATTERN_CFG1_RESERVED0_FIELD_MASK,
    0,
    GPON_GEARBOX_PATTERN_CFG1_RESERVED0_FIELD_WIDTH,
    GPON_GEARBOX_PATTERN_CFG1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_PATTERN_CFG1_HEADER
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_PATTERN_CFG1_HEADER_FIELD =
{
    "HEADER",
#if RU_INCLUDE_DESC
    "HEADER",
    "8-bit pattern to placed at the start of every Tx burst when PG_MODE is 1."
    "Reset value is 0.",
#endif
    GPON_GEARBOX_PATTERN_CFG1_HEADER_FIELD_MASK,
    0,
    GPON_GEARBOX_PATTERN_CFG1_HEADER_FIELD_WIDTH,
    GPON_GEARBOX_PATTERN_CFG1_HEADER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_PATTERN_CFG1_PAYLOAD
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_PATTERN_CFG1_PAYLOAD_FIELD =
{
    "PAYLOAD",
#if RU_INCLUDE_DESC
    "PAYLOAD",
    "8-bit pattern to placed after the HEADER byte in every Tx burst when PG_MODE is 1."
    "Reset value is 0.",
#endif
    GPON_GEARBOX_PATTERN_CFG1_PAYLOAD_FIELD_MASK,
    0,
    GPON_GEARBOX_PATTERN_CFG1_PAYLOAD_FIELD_WIDTH,
    GPON_GEARBOX_PATTERN_CFG1_PAYLOAD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_PATTERN_CFG1_FILLER
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_PATTERN_CFG1_FILLER_FIELD =
{
    "FILLER",
#if RU_INCLUDE_DESC
    "FILLER",
    "8-bit pattern to placed between Tx bursts when PG_MODE is 1."
    "Reset value is 0.",
#endif
    GPON_GEARBOX_PATTERN_CFG1_FILLER_FIELD_MASK,
    0,
    GPON_GEARBOX_PATTERN_CFG1_FILLER_FIELD_WIDTH,
    GPON_GEARBOX_PATTERN_CFG1_FILLER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_PATTERN_CFG2_BURST_SIZE
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_PATTERN_CFG2_BURST_SIZE_FIELD =
{
    "BURST_SIZE",
#if RU_INCLUDE_DESC
    "BURST_SIZE",
    "Total length of Tx burst in bytes when PG_MODE is 1."
    "Reset value is 0.",
#endif
    GPON_GEARBOX_PATTERN_CFG2_BURST_SIZE_FIELD_MASK,
    0,
    GPON_GEARBOX_PATTERN_CFG2_BURST_SIZE_FIELD_WIDTH,
    GPON_GEARBOX_PATTERN_CFG2_BURST_SIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_PATTERN_CFG2_GAP_SIZE
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_PATTERN_CFG2_GAP_SIZE_FIELD =
{
    "GAP_SIZE",
#if RU_INCLUDE_DESC
    "GAP_SIZE",
    "Number of filler bytes to be placed between Tx bursts when PG_MODE is 1."
    "Reset value is 0.",
#endif
    GPON_GEARBOX_PATTERN_CFG2_GAP_SIZE_FIELD_MASK,
    0,
    GPON_GEARBOX_PATTERN_CFG2_GAP_SIZE_FIELD_WIDTH,
    GPON_GEARBOX_PATTERN_CFG2_GAP_SIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_PATTERN_CFG2_RESERVED0
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_PATTERN_CFG2_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    GPON_GEARBOX_PATTERN_CFG2_RESERVED0_FIELD_MASK,
    0,
    GPON_GEARBOX_PATTERN_CFG2_RESERVED0_FIELD_WIDTH,
    GPON_GEARBOX_PATTERN_CFG2_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_2_RESERVED0
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_2_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    GPON_GEARBOX_2_RESERVED0_FIELD_MASK,
    0,
    GPON_GEARBOX_2_RESERVED0_FIELD_WIDTH,
    GPON_GEARBOX_2_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_2_FIFO_CFG_1_TX_RD_POINTER
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_2_FIFO_CFG_1_TX_RD_POINTER_FIELD =
{
    "FIFO_CFG_1_TX_RD_POINTER",
#if RU_INCLUDE_DESC
    "FIFO_CFG_1_TX_RD_POINTER",
    "Initial value to be loaded into Tx FIFO read pointer when"
    "TXFIFO_REET is asserted. Legal values are 0 to 31."
    "Reset value is 0x1c.",
#endif
    GPON_GEARBOX_2_FIFO_CFG_1_TX_RD_POINTER_FIELD_MASK,
    0,
    GPON_GEARBOX_2_FIFO_CFG_1_TX_RD_POINTER_FIELD_WIDTH,
    GPON_GEARBOX_2_FIFO_CFG_1_TX_RD_POINTER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_2_RESERVED1
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_2_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    GPON_GEARBOX_2_RESERVED1_FIELD_MASK,
    0,
    GPON_GEARBOX_2_RESERVED1_FIELD_WIDTH,
    GPON_GEARBOX_2_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_2_FIFO_CFG_1_TX_WR_POINTER
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_2_FIFO_CFG_1_TX_WR_POINTER_FIELD =
{
    "FIFO_CFG_1_TX_WR_POINTER",
#if RU_INCLUDE_DESC
    "FIFO_CFG_1_TX_WR_POINTER",
    "Initial value to be loaded into Tx FIFO write pointer when"
    "TXFIFO_RESET is asserted. Legal values are 0 to 19."
    "Reset value is 0x0.",
#endif
    GPON_GEARBOX_2_FIFO_CFG_1_TX_WR_POINTER_FIELD_MASK,
    0,
    GPON_GEARBOX_2_FIFO_CFG_1_TX_WR_POINTER_FIELD_WIDTH,
    GPON_GEARBOX_2_FIFO_CFG_1_TX_WR_POINTER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_2_TX_VLD_DELAY_CYC
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_2_TX_VLD_DELAY_CYC_FIELD =
{
    "TX_VLD_DELAY_CYC",
#if RU_INCLUDE_DESC
    "TX_VLD_DELAY_CYC",
    "Offset of tx valid signal towards SERDES",
#endif
    GPON_GEARBOX_2_TX_VLD_DELAY_CYC_FIELD_MASK,
    0,
    GPON_GEARBOX_2_TX_VLD_DELAY_CYC_FIELD_WIDTH,
    GPON_GEARBOX_2_TX_VLD_DELAY_CYC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_2_CONFIG_BURST_DELAY_CYC
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_2_CONFIG_BURST_DELAY_CYC_FIELD =
{
    "CONFIG_BURST_DELAY_CYC",
#if RU_INCLUDE_DESC
    "CONFIG_BURST_DELAY_CYC",
    "TBD",
#endif
    GPON_GEARBOX_2_CONFIG_BURST_DELAY_CYC_FIELD_MASK,
    0,
    GPON_GEARBOX_2_CONFIG_BURST_DELAY_CYC_FIELD_WIDTH,
    GPON_GEARBOX_2_CONFIG_BURST_DELAY_CYC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_2_RESERVED2
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_2_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    GPON_GEARBOX_2_RESERVED2_FIELD_MASK,
    0,
    GPON_GEARBOX_2_RESERVED2_FIELD_WIDTH,
    GPON_GEARBOX_2_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: GPON_GEARBOX_STATUS_CR_RD_DATA_CLX
 ******************************************************************************/
const ru_field_rec GPON_GEARBOX_STATUS_CR_RD_DATA_CLX_FIELD =
{
    "CR_RD_DATA_CLX",
#if RU_INCLUDE_DESC
    "CR_RD_DATA_CLX",
    "Status indication based on status_sel signals. If"
    "gpon_gearbox_fifo_status_sel is high, this status will be"
    "gpon_gearbox_fifo_status. If gpon_gearbox_ptg_status1_sel is high,"
    "this status will be gpon_gearbox_ptg_status1. If"
    "gpon_gearbox_ptg_status2_sel is high, this status will be"
    "gpon_gearbox_ptg_status2."
    "Reset value is 0x0.",
#endif
    GPON_GEARBOX_STATUS_CR_RD_DATA_CLX_FIELD_MASK,
    0,
    GPON_GEARBOX_STATUS_CR_RD_DATA_CLX_FIELD_WIDTH,
    GPON_GEARBOX_STATUS_CR_RD_DATA_CLX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: GPON_GEARBOX_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *GPON_GEARBOX_0_FIELDS[] =
{
    &GPON_GEARBOX_0_SW_RESET_TXPG_RESET_FIELD,
    &GPON_GEARBOX_0_SW_RESET_TXFIFO_RESET_FIELD,
    &GPON_GEARBOX_0_FIFO_CFG_0_CLEAR_TXFIFO_DRIFTED_FIELD,
    &GPON_GEARBOX_0_FIFO_CFG_0_LOOPBACK_RX_FIELD,
    &GPON_GEARBOX_0_FIFO_CFG_0_CLEAR_TXFIFO_COLLISION_FIELD,
    &GPON_GEARBOX_0_FIFO_CFG_0_TX_WR_PTR_ADV_FIELD,
    &GPON_GEARBOX_0_FIFO_CFG_0_TX_WR_PTR_DLY_FIELD,
    &GPON_GEARBOX_0_FIFO_CFG_0_TX_BIT_INV_FIELD,
    &GPON_GEARBOX_0_FIFO_CFG_0_TX_PTR_DIST_MAX_FIELD,
    &GPON_GEARBOX_0_FIFO_CFG_0_ASYM_LOOPBACK_FIELD,
    &GPON_GEARBOX_0_RESERVED0_FIELD,
    &GPON_GEARBOX_0_FIFO_CFG_0_TX_PTR_DIST_MIN_FIELD,
    &GPON_GEARBOX_0_RESERVED1_FIELD,
    &GPON_GEARBOX_0_FIFO_CFG_0_TX_20BIT_ORDER_FIELD,
    &GPON_GEARBOX_0_FIFO_CFG_0_TX_8BIT_ORDER_FIELD,
    &GPON_GEARBOX_0_FIFO_CFG_0_RX_16BIT_ORDER_FIELD,
    &GPON_GEARBOX_0_FIFO_CFG_0_TXLBE_BIT_ORDER_FIELD,
    &GPON_GEARBOX_0_FIFO_STATUS_SEL_FIELD,
    &GPON_GEARBOX_0_PTG_STATUS1_SEL_FIELD,
    &GPON_GEARBOX_0_PTG_STATUS2_SEL_FIELD,
    &GPON_GEARBOX_0_RESERVED2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec GPON_GEARBOX_0_REG = 
{
    "GEARBOX_0",
#if RU_INCLUDE_DESC
    "GPON_GEARBOX_0 Register",
    "Configuration for the GPON gearbox.",
#endif
    GPON_GEARBOX_0_REG_OFFSET,
    0,
    0,
    22,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    21,
    GPON_GEARBOX_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: GPON_GEARBOX_PATTERN_CFG1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *GPON_GEARBOX_PATTERN_CFG1_FIELDS[] =
{
    &GPON_GEARBOX_PATTERN_CFG1_PG_MODE_FIELD,
    &GPON_GEARBOX_PATTERN_CFG1_RESERVED0_FIELD,
    &GPON_GEARBOX_PATTERN_CFG1_HEADER_FIELD,
    &GPON_GEARBOX_PATTERN_CFG1_PAYLOAD_FIELD,
    &GPON_GEARBOX_PATTERN_CFG1_FILLER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec GPON_GEARBOX_PATTERN_CFG1_REG = 
{
    "GEARBOX_PATTERN_CFG1",
#if RU_INCLUDE_DESC
    "GPON_GEARBOX_PATTERN_CFG1 Register",
    "GPON Gearbox Pattern Generator Configuration Register 1",
#endif
    GPON_GEARBOX_PATTERN_CFG1_REG_OFFSET,
    0,
    0,
    23,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    GPON_GEARBOX_PATTERN_CFG1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: GPON_GEARBOX_PATTERN_CFG2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *GPON_GEARBOX_PATTERN_CFG2_FIELDS[] =
{
    &GPON_GEARBOX_PATTERN_CFG2_BURST_SIZE_FIELD,
    &GPON_GEARBOX_PATTERN_CFG2_GAP_SIZE_FIELD,
    &GPON_GEARBOX_PATTERN_CFG2_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec GPON_GEARBOX_PATTERN_CFG2_REG = 
{
    "GEARBOX_PATTERN_CFG2",
#if RU_INCLUDE_DESC
    "GPON_GEARBOX_PATTERN_CFG2 Register",
    "GPON Gearbox Pattern Generator Configuration Register 2",
#endif
    GPON_GEARBOX_PATTERN_CFG2_REG_OFFSET,
    0,
    0,
    24,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    GPON_GEARBOX_PATTERN_CFG2_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: GPON_GEARBOX_2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *GPON_GEARBOX_2_FIELDS[] =
{
    &GPON_GEARBOX_2_RESERVED0_FIELD,
    &GPON_GEARBOX_2_FIFO_CFG_1_TX_RD_POINTER_FIELD,
    &GPON_GEARBOX_2_RESERVED1_FIELD,
    &GPON_GEARBOX_2_FIFO_CFG_1_TX_WR_POINTER_FIELD,
    &GPON_GEARBOX_2_TX_VLD_DELAY_CYC_FIELD,
    &GPON_GEARBOX_2_CONFIG_BURST_DELAY_CYC_FIELD,
    &GPON_GEARBOX_2_RESERVED2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec GPON_GEARBOX_2_REG = 
{
    "GEARBOX_2",
#if RU_INCLUDE_DESC
    "GPON_GEARBOX_2 Register",
    "Configuration for the GPON gearbox.",
#endif
    GPON_GEARBOX_2_REG_OFFSET,
    0,
    0,
    25,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    GPON_GEARBOX_2_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: GPON_GEARBOX_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *GPON_GEARBOX_STATUS_FIELDS[] =
{
    &GPON_GEARBOX_STATUS_CR_RD_DATA_CLX_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec GPON_GEARBOX_STATUS_REG = 
{
    "GEARBOX_STATUS",
#if RU_INCLUDE_DESC
    "GPON_GEARBOX_STATUS Register",
    "Register used for various WAN status bits.",
#endif
    GPON_GEARBOX_STATUS_REG_OFFSET,
    0,
    0,
    26,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    GPON_GEARBOX_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: GPON
 ******************************************************************************/
static const ru_reg_rec *GPON_REGS[] =
{
    &GPON_GEARBOX_0_REG,
    &GPON_GEARBOX_PATTERN_CFG1_REG,
    &GPON_GEARBOX_PATTERN_CFG2_REG,
    &GPON_GEARBOX_2_REG,
    &GPON_GEARBOX_STATUS_REG,
};

unsigned long GPON_ADDRS[] =
{
    0x82db2004,
};

const ru_block_rec GPON_BLOCK = 
{
    "GPON",
    GPON_ADDRS,
    1,
    5,
    GPON_REGS
};

/* End of file GPON.c */
