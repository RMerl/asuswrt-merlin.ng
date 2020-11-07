/*
   Copyright (c) 2015 Broadcom
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

/******************************************************************************
 * Register: UNIMAC_RDP_IPG_HD_BKP_CNTL
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_IPG_HD_BKP_CNTL_REG = 
{
    "IPG_HD_BKP_CNTL",
#if RU_INCLUDE_DESC
    "The control register for HD-BackPressure.",
    "",
#endif
    UNIMAC_RDP_IPG_HD_BKP_CNTL_REG_OFFSET,
    0,
    0,
    788,
};

/******************************************************************************
 * Register: UNIMAC_RDP_COMMAND_CONFIG
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_COMMAND_CONFIG_REG = 
{
    "COMMAND_CONFIG",
#if RU_INCLUDE_DESC
    "Command register. Used by the host processor to control and configure the core",
    "",
#endif
    UNIMAC_RDP_COMMAND_CONFIG_REG_OFFSET,
    0,
    0,
    789,
};

/******************************************************************************
 * Register: UNIMAC_RDP_MAC_0
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_MAC_0_REG = 
{
    "MAC_0",
#if RU_INCLUDE_DESC
    "Core MAC address bits 47 to 16.",
    "",
#endif
    UNIMAC_RDP_MAC_0_REG_OFFSET,
    0,
    0,
    790,
};

/******************************************************************************
 * Register: UNIMAC_RDP_MAC_1
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_MAC_1_REG = 
{
    "MAC_1",
#if RU_INCLUDE_DESC
    "Core MAC address bits 15 to 0.",
    "",
#endif
    UNIMAC_RDP_MAC_1_REG_OFFSET,
    0,
    0,
    791,
};

/******************************************************************************
 * Register: UNIMAC_RDP_FRM_LENGTH
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_FRM_LENGTH_REG = 
{
    "FRM_LENGTH",
#if RU_INCLUDE_DESC
    "Maximum Frame Length.",
    "",
#endif
    UNIMAC_RDP_FRM_LENGTH_REG_OFFSET,
    0,
    0,
    792,
};

/******************************************************************************
 * Register: UNIMAC_RDP_PAUSE_QUANT
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_PAUSE_QUANT_REG = 
{
    "PAUSE_QUANT",
#if RU_INCLUDE_DESC
    "Receive Pause Quanta.",
    "",
#endif
    UNIMAC_RDP_PAUSE_QUANT_REG_OFFSET,
    0,
    0,
    793,
};

/******************************************************************************
 * Register: UNIMAC_RDP_TX_TS_SEQ_ID
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_TX_TS_SEQ_ID_REG = 
{
    "TX_TS_SEQ_ID",
#if RU_INCLUDE_DESC
    "Transmit Two Step Timestamp Sequence ID",
    "",
#endif
    UNIMAC_RDP_TX_TS_SEQ_ID_REG_OFFSET,
    0,
    0,
    794,
};

/******************************************************************************
 * Register: UNIMAC_RDP_SFD_OFFSET
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_SFD_OFFSET_REG = 
{
    "SFD_OFFSET",
#if RU_INCLUDE_DESC
    "EFM Preamble Length.",
    "",
#endif
    UNIMAC_RDP_SFD_OFFSET_REG_OFFSET,
    0,
    0,
    795,
};

/******************************************************************************
 * Register: UNIMAC_RDP_MAC_MODE
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_MAC_MODE_REG = 
{
    "MAC_MODE",
#if RU_INCLUDE_DESC
    "MAC Mode Status. MAC Speed and Duplex Mode configuration from register COMMAND CONFIG, when ENA_EXT_CONFIG is set to '0' or from signals set_speed(1:0), set_duplex, tx_pause_en, rx_pause_en and link_stat.",
    "",
#endif
    UNIMAC_RDP_MAC_MODE_REG_OFFSET,
    0,
    0,
    796,
};

/******************************************************************************
 * Register: UNIMAC_RDP_TAG_0
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_TAG_0_REG = 
{
    "TAG_0",
#if RU_INCLUDE_DESC
    "Programmable VLAN outer tag",
    "",
#endif
    UNIMAC_RDP_TAG_0_REG_OFFSET,
    0,
    0,
    797,
};

/******************************************************************************
 * Register: UNIMAC_RDP_TAG_1
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_TAG_1_REG = 
{
    "TAG_1",
#if RU_INCLUDE_DESC
    "Programmable VLAN inner tag",
    "",
#endif
    UNIMAC_RDP_TAG_1_REG_OFFSET,
    0,
    0,
    798,
};

/******************************************************************************
 * Register: UNIMAC_RDP_RX_PAUSE_QUANTA_SCALE
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_RX_PAUSE_QUANTA_SCALE_REG = 
{
    "RX_PAUSE_QUANTA_SCALE",
#if RU_INCLUDE_DESC
    "programmable Rx pause quanta scaler. Static register. Affects Xoff values only",
    "",
#endif
    UNIMAC_RDP_RX_PAUSE_QUANTA_SCALE_REG_OFFSET,
    0,
    0,
    799,
};

/******************************************************************************
 * Register: UNIMAC_RDP_TX_PREAMBLE
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_TX_PREAMBLE_REG = 
{
    "TX_PREAMBLE",
#if RU_INCLUDE_DESC
    "Programmable Preamble at Tx.",
    "",
#endif
    UNIMAC_RDP_TX_PREAMBLE_REG_OFFSET,
    0,
    0,
    800,
};

/******************************************************************************
 * Register: UNIMAC_RDP_TX_IPG_LENGTH
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_TX_IPG_LENGTH_REG = 
{
    "TX_IPG_LENGTH",
#if RU_INCLUDE_DESC
    "Programmable Inter-Packet-Gap (IPG).",
    "",
#endif
    UNIMAC_RDP_TX_IPG_LENGTH_REG_OFFSET,
    0,
    0,
    801,
};

/******************************************************************************
 * Register: UNIMAC_RDP_PFC_XOFF_TIMER
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_PFC_XOFF_TIMER_REG = 
{
    "PFC_XOFF_TIMER",
#if RU_INCLUDE_DESC
    "XOFF Timer value for PFC Tx packet",
    "",
#endif
    UNIMAC_RDP_PFC_XOFF_TIMER_REG_OFFSET,
    0,
    0,
    802,
};

/******************************************************************************
 * Register: UNIMAC_RDP_UMAC_EEE_CTRL
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_UMAC_EEE_CTRL_REG = 
{
    "UMAC_EEE_CTRL",
#if RU_INCLUDE_DESC
    "control configs for EEE feature",
    "",
#endif
    UNIMAC_RDP_UMAC_EEE_CTRL_REG_OFFSET,
    0,
    0,
    803,
};

/******************************************************************************
 * Register: UNIMAC_RDP_MII_EEE_DELAY_ENTRY_TIMER
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_MII_EEE_DELAY_ENTRY_TIMER_REG = 
{
    "MII_EEE_DELAY_ENTRY_TIMER",
#if RU_INCLUDE_DESC
    "MII_EEE LPI timer",
    "",
#endif
    UNIMAC_RDP_MII_EEE_DELAY_ENTRY_TIMER_REG_OFFSET,
    0,
    0,
    804,
};

/******************************************************************************
 * Register: UNIMAC_RDP_GMII_EEE_DELAY_ENTRY_TIMER
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_GMII_EEE_DELAY_ENTRY_TIMER_REG = 
{
    "GMII_EEE_DELAY_ENTRY_TIMER",
#if RU_INCLUDE_DESC
    "GMII_EEE LPI timer",
    "",
#endif
    UNIMAC_RDP_GMII_EEE_DELAY_ENTRY_TIMER_REG_OFFSET,
    0,
    0,
    805,
};

/******************************************************************************
 * Register: UNIMAC_RDP_UMAC_EEE_REF_COUNT
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_UMAC_EEE_REF_COUNT_REG = 
{
    "UMAC_EEE_REF_COUNT",
#if RU_INCLUDE_DESC
    "clock divider for 1 us quanta count in EEE",
    "",
#endif
    UNIMAC_RDP_UMAC_EEE_REF_COUNT_REG_OFFSET,
    0,
    0,
    806,
};

/******************************************************************************
 * Register: UNIMAC_RDP_UMAC_TIMESTAMP_ADJUST
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_UMAC_TIMESTAMP_ADJUST_REG = 
{
    "UMAC_TIMESTAMP_ADJUST",
#if RU_INCLUDE_DESC
    "1588_one_step_timestamp control",
    "",
#endif
    UNIMAC_RDP_UMAC_TIMESTAMP_ADJUST_REG_OFFSET,
    0,
    0,
    807,
};

/******************************************************************************
 * Register: UNIMAC_RDP_UMAC_RX_PKT_DROP_STATUS
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_UMAC_RX_PKT_DROP_STATUS_REG = 
{
    "UMAC_RX_PKT_DROP_STATUS",
#if RU_INCLUDE_DESC
    "sticky status for Rx packet drop due to invalid IPG",
    "",
#endif
    UNIMAC_RDP_UMAC_RX_PKT_DROP_STATUS_REG_OFFSET,
    0,
    0,
    808,
};

/******************************************************************************
 * Register: UNIMAC_RDP_UMAC_SYMMETRIC_IDLE_THRESHOLD
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_UMAC_SYMMETRIC_IDLE_THRESHOLD_REG = 
{
    "UMAC_SYMMETRIC_IDLE_THRESHOLD",
#if RU_INCLUDE_DESC
    "RX IDLE threshold for LPI prediction",
    "",
#endif
    UNIMAC_RDP_UMAC_SYMMETRIC_IDLE_THRESHOLD_REG_OFFSET,
    0,
    0,
    809,
};

/******************************************************************************
 * Register: UNIMAC_RDP_MII_EEE_WAKE_TIMER
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_MII_EEE_WAKE_TIMER_REG = 
{
    "MII_EEE_WAKE_TIMER",
#if RU_INCLUDE_DESC
    "MII_EEE Wake timer",
    "",
#endif
    UNIMAC_RDP_MII_EEE_WAKE_TIMER_REG_OFFSET,
    0,
    0,
    810,
};

/******************************************************************************
 * Register: UNIMAC_RDP_GMII_EEE_WAKE_TIMER
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_GMII_EEE_WAKE_TIMER_REG = 
{
    "GMII_EEE_WAKE_TIMER",
#if RU_INCLUDE_DESC
    "GMII_EEE Wake timer",
    "",
#endif
    UNIMAC_RDP_GMII_EEE_WAKE_TIMER_REG_OFFSET,
    0,
    0,
    811,
};

/******************************************************************************
 * Register: UNIMAC_RDP_UMAC_REV_ID
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_UMAC_REV_ID_REG = 
{
    "UMAC_REV_ID",
#if RU_INCLUDE_DESC
    "UNIMAC_REV_ID",
    "",
#endif
    UNIMAC_RDP_UMAC_REV_ID_REG_OFFSET,
    0,
    0,
    812,
};

/******************************************************************************
 * Register: UNIMAC_RDP_MAC_PFC_TYPE
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_MAC_PFC_TYPE_REG = 
{
    "MAC_PFC_TYPE",
#if RU_INCLUDE_DESC
    "PFC ethertype",
    "",
#endif
    UNIMAC_RDP_MAC_PFC_TYPE_REG_OFFSET,
    0,
    0,
    813,
};

/******************************************************************************
 * Register: UNIMAC_RDP_MAC_PFC_OPCODE
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_MAC_PFC_OPCODE_REG = 
{
    "MAC_PFC_OPCODE",
#if RU_INCLUDE_DESC
    "PFC opcode",
    "",
#endif
    UNIMAC_RDP_MAC_PFC_OPCODE_REG_OFFSET,
    0,
    0,
    814,
};

/******************************************************************************
 * Register: UNIMAC_RDP_MAC_PFC_DA_0
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_MAC_PFC_DA_0_REG = 
{
    "MAC_PFC_DA_0",
#if RU_INCLUDE_DESC
    "lower 32 bits of DA for PFC",
    "",
#endif
    UNIMAC_RDP_MAC_PFC_DA_0_REG_OFFSET,
    0,
    0,
    815,
};

/******************************************************************************
 * Register: UNIMAC_RDP_MAC_PFC_DA_1
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_MAC_PFC_DA_1_REG = 
{
    "MAC_PFC_DA_1",
#if RU_INCLUDE_DESC
    "upper 16 bits of DA for PFC",
    "",
#endif
    UNIMAC_RDP_MAC_PFC_DA_1_REG_OFFSET,
    0,
    0,
    816,
};

/******************************************************************************
 * Register: UNIMAC_RDP_MACSEC_PROG_TX_CRC
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_MACSEC_PROG_TX_CRC_REG = 
{
    "MACSEC_PROG_TX_CRC",
#if RU_INCLUDE_DESC
    "Programmable CRC value to corrupt the Tx CRC to be used in MACSEC",
    "",
#endif
    UNIMAC_RDP_MACSEC_PROG_TX_CRC_REG_OFFSET,
    0,
    0,
    817,
};

/******************************************************************************
 * Register: UNIMAC_RDP_MACSEC_CNTRL
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_MACSEC_CNTRL_REG = 
{
    "MACSEC_CNTRL",
#if RU_INCLUDE_DESC
    "Miscellaneous control for MACSEC",
    "",
#endif
    UNIMAC_RDP_MACSEC_CNTRL_REG_OFFSET,
    0,
    0,
    818,
};

/******************************************************************************
 * Register: UNIMAC_RDP_TS_STATUS
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_TS_STATUS_REG = 
{
    "TS_STATUS",
#if RU_INCLUDE_DESC
    "Timestamp status",
    "",
#endif
    UNIMAC_RDP_TS_STATUS_REG_OFFSET,
    0,
    0,
    819,
};

/******************************************************************************
 * Register: UNIMAC_RDP_TX_TS_DATA
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_TX_TS_DATA_REG = 
{
    "TX_TS_DATA",
#if RU_INCLUDE_DESC
    "Transmit Timestamp data",
    "",
#endif
    UNIMAC_RDP_TX_TS_DATA_REG_OFFSET,
    0,
    0,
    820,
};

/******************************************************************************
 * Register: UNIMAC_RDP_PAUSE_REFRESH_CTRL
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_PAUSE_REFRESH_CTRL_REG = 
{
    "PAUSE_REFRESH_CTRL",
#if RU_INCLUDE_DESC
    "PAUSE frame refresh timer control register",
    "",
#endif
    UNIMAC_RDP_PAUSE_REFRESH_CTRL_REG_OFFSET,
    0,
    0,
    821,
};

/******************************************************************************
 * Register: UNIMAC_RDP_FLUSH_CONTROL
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_FLUSH_CONTROL_REG = 
{
    "FLUSH_CONTROL",
#if RU_INCLUDE_DESC
    "Flush enable control register",
    "",
#endif
    UNIMAC_RDP_FLUSH_CONTROL_REG_OFFSET,
    0,
    0,
    822,
};

/******************************************************************************
 * Register: UNIMAC_RDP_RXFIFO_STAT
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_RXFIFO_STAT_REG = 
{
    "RXFIFO_STAT",
#if RU_INCLUDE_DESC
    "RXFIFO status register",
    "",
#endif
    UNIMAC_RDP_RXFIFO_STAT_REG_OFFSET,
    0,
    0,
    823,
};

/******************************************************************************
 * Register: UNIMAC_RDP_TXFIFO_STAT
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_TXFIFO_STAT_REG = 
{
    "TXFIFO_STAT",
#if RU_INCLUDE_DESC
    "TXFIFO status register",
    "",
#endif
    UNIMAC_RDP_TXFIFO_STAT_REG_OFFSET,
    0,
    0,
    824,
};

/******************************************************************************
 * Register: UNIMAC_RDP_MAC_PFC_CTRL
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_MAC_PFC_CTRL_REG = 
{
    "MAC_PFC_CTRL",
#if RU_INCLUDE_DESC
    "PFC control register",
    "",
#endif
    UNIMAC_RDP_MAC_PFC_CTRL_REG_OFFSET,
    0,
    0,
    825,
};

/******************************************************************************
 * Register: UNIMAC_RDP_MAC_PFC_REFRESH_CTRL
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_MAC_PFC_REFRESH_CTRL_REG = 
{
    "MAC_PFC_REFRESH_CTRL",
#if RU_INCLUDE_DESC
    "PFC refresh control register",
    "",
#endif
    UNIMAC_RDP_MAC_PFC_REFRESH_CTRL_REG_OFFSET,
    0,
    0,
    826,
};

/******************************************************************************
 * Block: UNIMAC_RDP
 ******************************************************************************/
static const ru_reg_rec *UNIMAC_RDP_REGS[] =
{
    &UNIMAC_RDP_IPG_HD_BKP_CNTL_REG,
    &UNIMAC_RDP_COMMAND_CONFIG_REG,
    &UNIMAC_RDP_MAC_0_REG,
    &UNIMAC_RDP_MAC_1_REG,
    &UNIMAC_RDP_FRM_LENGTH_REG,
    &UNIMAC_RDP_PAUSE_QUANT_REG,
    &UNIMAC_RDP_TX_TS_SEQ_ID_REG,
    &UNIMAC_RDP_SFD_OFFSET_REG,
    &UNIMAC_RDP_MAC_MODE_REG,
    &UNIMAC_RDP_TAG_0_REG,
    &UNIMAC_RDP_TAG_1_REG,
    &UNIMAC_RDP_RX_PAUSE_QUANTA_SCALE_REG,
    &UNIMAC_RDP_TX_PREAMBLE_REG,
    &UNIMAC_RDP_TX_IPG_LENGTH_REG,
    &UNIMAC_RDP_PFC_XOFF_TIMER_REG,
    &UNIMAC_RDP_UMAC_EEE_CTRL_REG,
    &UNIMAC_RDP_MII_EEE_DELAY_ENTRY_TIMER_REG,
    &UNIMAC_RDP_GMII_EEE_DELAY_ENTRY_TIMER_REG,
    &UNIMAC_RDP_UMAC_EEE_REF_COUNT_REG,
    &UNIMAC_RDP_UMAC_TIMESTAMP_ADJUST_REG,
    &UNIMAC_RDP_UMAC_RX_PKT_DROP_STATUS_REG,
    &UNIMAC_RDP_UMAC_SYMMETRIC_IDLE_THRESHOLD_REG,
    &UNIMAC_RDP_MII_EEE_WAKE_TIMER_REG,
    &UNIMAC_RDP_GMII_EEE_WAKE_TIMER_REG,
    &UNIMAC_RDP_UMAC_REV_ID_REG,
    &UNIMAC_RDP_MAC_PFC_TYPE_REG,
    &UNIMAC_RDP_MAC_PFC_OPCODE_REG,
    &UNIMAC_RDP_MAC_PFC_DA_0_REG,
    &UNIMAC_RDP_MAC_PFC_DA_1_REG,
    &UNIMAC_RDP_MACSEC_PROG_TX_CRC_REG,
    &UNIMAC_RDP_MACSEC_CNTRL_REG,
    &UNIMAC_RDP_TS_STATUS_REG,
    &UNIMAC_RDP_TX_TS_DATA_REG,
    &UNIMAC_RDP_PAUSE_REFRESH_CTRL_REG,
    &UNIMAC_RDP_FLUSH_CONTROL_REG,
    &UNIMAC_RDP_RXFIFO_STAT_REG,
    &UNIMAC_RDP_TXFIFO_STAT_REG,
    &UNIMAC_RDP_MAC_PFC_CTRL_REG,
    &UNIMAC_RDP_MAC_PFC_REFRESH_CTRL_REG,
};

unsigned long UNIMAC_RDP_ADDRS[] =
{
    0x82da0004,
    0x82da1004,
    0x82da2004,
    0x82da3004,
    0x82da4004,
};

const ru_block_rec UNIMAC_RDP_BLOCK = 
{
    "UNIMAC_RDP",
    UNIMAC_RDP_ADDRS,
    5,
    39,
    UNIMAC_RDP_REGS
};

/* End of file XRDP_UNIMAC_RDP.c */
