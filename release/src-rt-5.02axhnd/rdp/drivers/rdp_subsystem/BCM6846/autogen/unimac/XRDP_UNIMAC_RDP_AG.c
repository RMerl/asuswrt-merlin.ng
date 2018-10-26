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

#if RU_INCLUDE_FIELD_DB
/******************************************************************************
 * Field: UNIMAC_RDP_IPG_HD_BKP_CNTL_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_IPG_HD_BKP_CNTL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_IPG_HD_BKP_CNTL_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_IPG_HD_BKP_CNTL_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_IPG_HD_BKP_CNTL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_IPG_HD_BKP_CNTL_IPG_CONFIG_RX
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_IPG_HD_BKP_CNTL_IPG_CONFIG_RX_FIELD =
{
    "IPG_CONFIG_RX",
#if RU_INCLUDE_DESC
    "",
    "The programmable Rx IPG below which the packets received are dropped graciously. The value is in Bytes for 1/2.5G and Nibbles for 10/100M.",
#endif
    UNIMAC_RDP_IPG_HD_BKP_CNTL_IPG_CONFIG_RX_FIELD_MASK,
    0,
    UNIMAC_RDP_IPG_HD_BKP_CNTL_IPG_CONFIG_RX_FIELD_WIDTH,
    UNIMAC_RDP_IPG_HD_BKP_CNTL_IPG_CONFIG_RX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_IPG_HD_BKP_CNTL_HD_FC_BKOFF_OK
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_IPG_HD_BKP_CNTL_HD_FC_BKOFF_OK_FIELD =
{
    "HD_FC_BKOFF_OK",
#if RU_INCLUDE_DESC
    "",
    "Register bit 1 refers to the application of backoff algorithm during HD backpressure.",
#endif
    UNIMAC_RDP_IPG_HD_BKP_CNTL_HD_FC_BKOFF_OK_FIELD_MASK,
    0,
    UNIMAC_RDP_IPG_HD_BKP_CNTL_HD_FC_BKOFF_OK_FIELD_WIDTH,
    UNIMAC_RDP_IPG_HD_BKP_CNTL_HD_FC_BKOFF_OK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_IPG_HD_BKP_CNTL_HD_FC_ENA
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_IPG_HD_BKP_CNTL_HD_FC_ENA_FIELD =
{
    "HD_FC_ENA",
#if RU_INCLUDE_DESC
    "",
    "When set, enables back-pressure in half-duplex mode.",
#endif
    UNIMAC_RDP_IPG_HD_BKP_CNTL_HD_FC_ENA_FIELD_MASK,
    0,
    UNIMAC_RDP_IPG_HD_BKP_CNTL_HD_FC_ENA_FIELD_WIDTH,
    UNIMAC_RDP_IPG_HD_BKP_CNTL_HD_FC_ENA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_COMMAND_CONFIG_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_COMMAND_CONFIG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_COMMAND_CONFIG_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_COMMAND_CONFIG_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_COMMAND_CONFIG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_COMMAND_CONFIG_RUNT_FILTER_DIS
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_COMMAND_CONFIG_RUNT_FILTER_DIS_FIELD =
{
    "RUNT_FILTER_DIS",
#if RU_INCLUDE_DESC
    "",
    "When set, disable runt filtering.",
#endif
    UNIMAC_RDP_COMMAND_CONFIG_RUNT_FILTER_DIS_FIELD_MASK,
    0,
    UNIMAC_RDP_COMMAND_CONFIG_RUNT_FILTER_DIS_FIELD_WIDTH,
    UNIMAC_RDP_COMMAND_CONFIG_RUNT_FILTER_DIS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_COMMAND_CONFIG_OOB_EFC_EN
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_COMMAND_CONFIG_OOB_EFC_EN_FIELD =
{
    "OOB_EFC_EN",
#if RU_INCLUDE_DESC
    "",
    "If set then out-of-band egress flow control is enabled. When this bit is set and input pin ext_tx_flow_control is enabled then data frame trasmission is stopped, whereas Pause & PFC frames are transmitted normally. This operation is similar to halting the transmit datapath due to the reception of a Pause Frame with non-zero timer value, and is used in applications where the flow control information is exchanged out of band. Enabling or disabling this bit has no effect on regular Rx_pause pkt based egress flow control.",
#endif
    UNIMAC_RDP_COMMAND_CONFIG_OOB_EFC_EN_FIELD_MASK,
    0,
    UNIMAC_RDP_COMMAND_CONFIG_OOB_EFC_EN_FIELD_WIDTH,
    UNIMAC_RDP_COMMAND_CONFIG_OOB_EFC_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_COMMAND_CONFIG_IGNORE_TX_PAUSE
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_COMMAND_CONFIG_IGNORE_TX_PAUSE_FIELD =
{
    "IGNORE_TX_PAUSE",
#if RU_INCLUDE_DESC
    "",
    "Ignores the back pressure signaling from the system and hence no Tx pause generation, when set.",
#endif
    UNIMAC_RDP_COMMAND_CONFIG_IGNORE_TX_PAUSE_FIELD_MASK,
    0,
    UNIMAC_RDP_COMMAND_CONFIG_IGNORE_TX_PAUSE_FIELD_WIDTH,
    UNIMAC_RDP_COMMAND_CONFIG_IGNORE_TX_PAUSE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_COMMAND_CONFIG_FD_TX_URUN_FIX_EN
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_COMMAND_CONFIG_FD_TX_URUN_FIX_EN_FIELD =
{
    "FD_TX_URUN_FIX_EN",
#if RU_INCLUDE_DESC
    "",
    "Tx Underflow detection can be improved by accounting for residue bytes in 128b to 8b convertor. The fix is valid only for full duplex mode and can be enabled by setting this bit.",
#endif
    UNIMAC_RDP_COMMAND_CONFIG_FD_TX_URUN_FIX_EN_FIELD_MASK,
    0,
    UNIMAC_RDP_COMMAND_CONFIG_FD_TX_URUN_FIX_EN_FIELD_WIDTH,
    UNIMAC_RDP_COMMAND_CONFIG_FD_TX_URUN_FIX_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_COMMAND_CONFIG_RESERVED1
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_COMMAND_CONFIG_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_COMMAND_CONFIG_RESERVED1_FIELD_MASK,
    0,
    UNIMAC_RDP_COMMAND_CONFIG_RESERVED1_FIELD_WIDTH,
    UNIMAC_RDP_COMMAND_CONFIG_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_COMMAND_CONFIG_LINE_LOOPBACK
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_COMMAND_CONFIG_LINE_LOOPBACK_FIELD =
{
    "LINE_LOOPBACK",
#if RU_INCLUDE_DESC
    "",
    "Enable Line Loopback i.e. MAC FIFO side loopback (RX to TX) when set to '1', normal operation when set to '0' (Reset value).",
#endif
    UNIMAC_RDP_COMMAND_CONFIG_LINE_LOOPBACK_FIELD_MASK,
    0,
    UNIMAC_RDP_COMMAND_CONFIG_LINE_LOOPBACK_FIELD_WIDTH,
    UNIMAC_RDP_COMMAND_CONFIG_LINE_LOOPBACK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_COMMAND_CONFIG_NO_LGTH_CHECK
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_COMMAND_CONFIG_NO_LGTH_CHECK_FIELD =
{
    "NO_LGTH_CHECK",
#if RU_INCLUDE_DESC
    "",
    "Payload Length Check Disable. When set to '0', the Core checks the frame's payload length with the Frame\n"
    "Length/Type field, when set to '1'(Reset value), the payload length check is disabled.",
#endif
    UNIMAC_RDP_COMMAND_CONFIG_NO_LGTH_CHECK_FIELD_MASK,
    0,
    UNIMAC_RDP_COMMAND_CONFIG_NO_LGTH_CHECK_FIELD_WIDTH,
    UNIMAC_RDP_COMMAND_CONFIG_NO_LGTH_CHECK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_COMMAND_CONFIG_CNTL_FRM_ENA
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_COMMAND_CONFIG_CNTL_FRM_ENA_FIELD =
{
    "CNTL_FRM_ENA",
#if RU_INCLUDE_DESC
    "",
    "MAC Control Frame Enable. When set to '1', MAC Control frames with any \n"
    "Opcode other than 0x0001 are accepted and forward to the Client interface. \n"
    "When set to '0' (Reset value), MAC Control frames with any Opcode other \n"
    "than 0x0001 are silently discarded.",
#endif
    UNIMAC_RDP_COMMAND_CONFIG_CNTL_FRM_ENA_FIELD_MASK,
    0,
    UNIMAC_RDP_COMMAND_CONFIG_CNTL_FRM_ENA_FIELD_WIDTH,
    UNIMAC_RDP_COMMAND_CONFIG_CNTL_FRM_ENA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_COMMAND_CONFIG_ENA_EXT_CONFIG
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_COMMAND_CONFIG_ENA_EXT_CONFIG_FIELD =
{
    "ENA_EXT_CONFIG",
#if RU_INCLUDE_DESC
    "",
    "Enable Configuration with External Pins. When set to '0' (Reset value) \n"
    "the Core speed and Mode is programmed with the register bits ETH_SPEED(1:0) \n"
    "and HD_ENA. When set to '1', the Core is configured with the pins \n"
    "set_speed(1:0) and set_duplex.",
#endif
    UNIMAC_RDP_COMMAND_CONFIG_ENA_EXT_CONFIG_FIELD_MASK,
    0,
    UNIMAC_RDP_COMMAND_CONFIG_ENA_EXT_CONFIG_FIELD_WIDTH,
    UNIMAC_RDP_COMMAND_CONFIG_ENA_EXT_CONFIG_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_COMMAND_CONFIG_EN_INTERNAL_TX_CRS
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_COMMAND_CONFIG_EN_INTERNAL_TX_CRS_FIELD =
{
    "EN_INTERNAL_TX_CRS",
#if RU_INCLUDE_DESC
    "",
    "If enabled, then CRS input to Unimac is ORed with tds[8] (tx data valid output). This is helpful when TX CRS is disabled inside PHY.",
#endif
    UNIMAC_RDP_COMMAND_CONFIG_EN_INTERNAL_TX_CRS_FIELD_MASK,
    0,
    UNIMAC_RDP_COMMAND_CONFIG_EN_INTERNAL_TX_CRS_FIELD_WIDTH,
    UNIMAC_RDP_COMMAND_CONFIG_EN_INTERNAL_TX_CRS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_COMMAND_CONFIG_RESERVED2
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_COMMAND_CONFIG_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_COMMAND_CONFIG_RESERVED2_FIELD_MASK,
    0,
    UNIMAC_RDP_COMMAND_CONFIG_RESERVED2_FIELD_WIDTH,
    UNIMAC_RDP_COMMAND_CONFIG_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_COMMAND_CONFIG_SW_OVERRIDE_RX
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_COMMAND_CONFIG_SW_OVERRIDE_RX_FIELD =
{
    "SW_OVERRIDE_RX",
#if RU_INCLUDE_DESC
    "",
    "If set, enables the SW programmed Rx pause capability config bits to overwrite the auto negotiated Rx pause capabilities when ena_ext_config (autoconfig) is set.\n"
    "If cleared, and when ena_ext_config (autoconfig) is set, then SW programmed Rx pause capability config bits has no effect over auto negotiated capabilities.",
#endif
    UNIMAC_RDP_COMMAND_CONFIG_SW_OVERRIDE_RX_FIELD_MASK,
    0,
    UNIMAC_RDP_COMMAND_CONFIG_SW_OVERRIDE_RX_FIELD_WIDTH,
    UNIMAC_RDP_COMMAND_CONFIG_SW_OVERRIDE_RX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_COMMAND_CONFIG_SW_OVERRIDE_TX
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_COMMAND_CONFIG_SW_OVERRIDE_TX_FIELD =
{
    "SW_OVERRIDE_TX",
#if RU_INCLUDE_DESC
    "",
    "If set, enables the SW programmed Tx pause capability config bits to overwrite the auto negotiated Tx pause capabilities when ena_ext_config (autoconfig) is set.\n"
    "If cleared, and when ena_ext_config (autoconfig) is set, then SW programmed Tx pause capability config bits has no effect over auto negotiated capabilities.",
#endif
    UNIMAC_RDP_COMMAND_CONFIG_SW_OVERRIDE_TX_FIELD_MASK,
    0,
    UNIMAC_RDP_COMMAND_CONFIG_SW_OVERRIDE_TX_FIELD_WIDTH,
    UNIMAC_RDP_COMMAND_CONFIG_SW_OVERRIDE_TX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_COMMAND_CONFIG_MAC_LOOP_CON
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_COMMAND_CONFIG_MAC_LOOP_CON_FIELD =
{
    "MAC_LOOP_CON",
#if RU_INCLUDE_DESC
    "",
    "Transmit packets to PHY while in MAC local loopback, when set to '1', otherwise transmit to PHY is disabled (normal operation),\n"
    "when set to '0' (Reset value).",
#endif
    UNIMAC_RDP_COMMAND_CONFIG_MAC_LOOP_CON_FIELD_MASK,
    0,
    UNIMAC_RDP_COMMAND_CONFIG_MAC_LOOP_CON_FIELD_WIDTH,
    UNIMAC_RDP_COMMAND_CONFIG_MAC_LOOP_CON_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_COMMAND_CONFIG_LOOP_ENA
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_COMMAND_CONFIG_LOOP_ENA_FIELD =
{
    "LOOP_ENA",
#if RU_INCLUDE_DESC
    "",
    "Enable GMII/MII loopback (TX to RX) when set to '1', normal operation when set to '0' (Reset value).",
#endif
    UNIMAC_RDP_COMMAND_CONFIG_LOOP_ENA_FIELD_MASK,
    0,
    UNIMAC_RDP_COMMAND_CONFIG_LOOP_ENA_FIELD_WIDTH,
    UNIMAC_RDP_COMMAND_CONFIG_LOOP_ENA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_COMMAND_CONFIG_FCS_CORRUPT_URUN_EN
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_COMMAND_CONFIG_FCS_CORRUPT_URUN_EN_FIELD =
{
    "FCS_CORRUPT_URUN_EN",
#if RU_INCLUDE_DESC
    "",
    "Corrupt Tx FCS, on underrun, when set to '1', No FCS corruption when set to '0' (Reset value).",
#endif
    UNIMAC_RDP_COMMAND_CONFIG_FCS_CORRUPT_URUN_EN_FIELD_MASK,
    0,
    UNIMAC_RDP_COMMAND_CONFIG_FCS_CORRUPT_URUN_EN_FIELD_WIDTH,
    UNIMAC_RDP_COMMAND_CONFIG_FCS_CORRUPT_URUN_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_COMMAND_CONFIG_SW_RESET
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_COMMAND_CONFIG_SW_RESET_FIELD =
{
    "SW_RESET",
#if RU_INCLUDE_DESC
    "",
    "Software Reset Command. When asserted, the TX and RX are \n"
    "disabled. Config registers are not affected by sw reset. Write a 0 to de-assert the sw reset.",
#endif
    UNIMAC_RDP_COMMAND_CONFIG_SW_RESET_FIELD_MASK,
    0,
    UNIMAC_RDP_COMMAND_CONFIG_SW_RESET_FIELD_WIDTH,
    UNIMAC_RDP_COMMAND_CONFIG_SW_RESET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_COMMAND_CONFIG_OVERFLOW_EN
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_COMMAND_CONFIG_OVERFLOW_EN_FIELD =
{
    "OVERFLOW_EN",
#if RU_INCLUDE_DESC
    "",
    "If set, enables Rx FIFO overflow logic. In this case, the RXFIFO_STAT[1] register bit is not operational (always set to 0).\n"
    "If cleared, disables RX FIFO overflow logic. In this case, the RXFIFO_STAT[1] register bit is operational (Sticky set when overrun occurs, clearable only by SW_Reset).",
#endif
    UNIMAC_RDP_COMMAND_CONFIG_OVERFLOW_EN_FIELD_MASK,
    0,
    UNIMAC_RDP_COMMAND_CONFIG_OVERFLOW_EN_FIELD_WIDTH,
    UNIMAC_RDP_COMMAND_CONFIG_OVERFLOW_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_COMMAND_CONFIG_RX_LOW_LATENCY_EN
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_COMMAND_CONFIG_RX_LOW_LATENCY_EN_FIELD =
{
    "RX_LOW_LATENCY_EN",
#if RU_INCLUDE_DESC
    "",
    "This works only when runt filter is disabled. It reduces the receive latency by 48 MAC clock time.",
#endif
    UNIMAC_RDP_COMMAND_CONFIG_RX_LOW_LATENCY_EN_FIELD_MASK,
    0,
    UNIMAC_RDP_COMMAND_CONFIG_RX_LOW_LATENCY_EN_FIELD_WIDTH,
    UNIMAC_RDP_COMMAND_CONFIG_RX_LOW_LATENCY_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_COMMAND_CONFIG_HD_ENA
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_COMMAND_CONFIG_HD_ENA_FIELD =
{
    "HD_ENA",
#if RU_INCLUDE_DESC
    "",
    "Half duplex enable. When set to '1', enables half duplex mode, when set \n"
    "to '0', the MAC operates in full duplex mode.\n"
    "Ignored at ethernet speeds 1G/2.5G or when the register ENA_EXT_CONFIG is set to '1'.",
#endif
    UNIMAC_RDP_COMMAND_CONFIG_HD_ENA_FIELD_MASK,
    0,
    UNIMAC_RDP_COMMAND_CONFIG_HD_ENA_FIELD_WIDTH,
    UNIMAC_RDP_COMMAND_CONFIG_HD_ENA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_COMMAND_CONFIG_TX_ADDR_INS
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_COMMAND_CONFIG_TX_ADDR_INS_FIELD =
{
    "TX_ADDR_INS",
#if RU_INCLUDE_DESC
    "",
    "Set MAC address on transmit. If enabled (Set to '1') the MAC overwrites \n"
    "the source MAC address with the programmed MAC address in registers MAC_0 \n"
    "and MAC_1. If disabled (Set to reset value '0'), the source MAC address \n"
    "received from the transmit application transmitted is not modified by the MAC.",
#endif
    UNIMAC_RDP_COMMAND_CONFIG_TX_ADDR_INS_FIELD_MASK,
    0,
    UNIMAC_RDP_COMMAND_CONFIG_TX_ADDR_INS_FIELD_WIDTH,
    UNIMAC_RDP_COMMAND_CONFIG_TX_ADDR_INS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_COMMAND_CONFIG_PAUSE_IGNORE
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_COMMAND_CONFIG_PAUSE_IGNORE_FIELD =
{
    "PAUSE_IGNORE",
#if RU_INCLUDE_DESC
    "",
    "Ignore Pause Frame Quanta. If enabled (Set to '1') received pause frames \n"
    "are ignored by the MAC. When disabled (Set to reset value '0') the transmit \n"
    "process is stopped for the amount of time specified in the pause quanta \n"
    "received within the pause frame.",
#endif
    UNIMAC_RDP_COMMAND_CONFIG_PAUSE_IGNORE_FIELD_MASK,
    0,
    UNIMAC_RDP_COMMAND_CONFIG_PAUSE_IGNORE_FIELD_WIDTH,
    UNIMAC_RDP_COMMAND_CONFIG_PAUSE_IGNORE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_COMMAND_CONFIG_PAUSE_FWD
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_COMMAND_CONFIG_PAUSE_FWD_FIELD =
{
    "PAUSE_FWD",
#if RU_INCLUDE_DESC
    "",
    "Terminate/Forward Pause Frames. If enabled (Set to '1') pause frames are \n"
    "forwarded to the user application.  If disabled (Set to reset value '0'), \n"
    "pause frames are terminated and discarded in the MAC.",
#endif
    UNIMAC_RDP_COMMAND_CONFIG_PAUSE_FWD_FIELD_MASK,
    0,
    UNIMAC_RDP_COMMAND_CONFIG_PAUSE_FWD_FIELD_WIDTH,
    UNIMAC_RDP_COMMAND_CONFIG_PAUSE_FWD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_COMMAND_CONFIG_CRC_FWD
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_COMMAND_CONFIG_CRC_FWD_FIELD =
{
    "CRC_FWD",
#if RU_INCLUDE_DESC
    "",
    "Terminate/Forward Received CRC. If enabled (1) the CRC field of received \n"
    "frames are transmitted to the user application.\n"
    "If disabled (Set to reset value '0') the CRC field is stripped from the frame.\n"
    "Note: If padding function (bit PAD_EN set to '1') is enabled. CRC_FWD is \n"
    "ignored and the CRC field is checked and always terminated and removed.",
#endif
    UNIMAC_RDP_COMMAND_CONFIG_CRC_FWD_FIELD_MASK,
    0,
    UNIMAC_RDP_COMMAND_CONFIG_CRC_FWD_FIELD_WIDTH,
    UNIMAC_RDP_COMMAND_CONFIG_CRC_FWD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_COMMAND_CONFIG_PAD_EN
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_COMMAND_CONFIG_PAD_EN_FIELD =
{
    "PAD_EN",
#if RU_INCLUDE_DESC
    "",
    "Enable/Disable Frame Padding. If enabled (Set to '1'), then padding is removed from the received frame before it is transmitted to the user\n"
    "application. If disabled (set to reset value '0'), then no padding is removed on receive by the MAC. \n"
    "This bit has no effect on Tx padding and hence Transmit always pad runts to guarantee a minimum frame size of 64 octets.",
#endif
    UNIMAC_RDP_COMMAND_CONFIG_PAD_EN_FIELD_MASK,
    0,
    UNIMAC_RDP_COMMAND_CONFIG_PAD_EN_FIELD_WIDTH,
    UNIMAC_RDP_COMMAND_CONFIG_PAD_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_COMMAND_CONFIG_PROMIS_EN
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_COMMAND_CONFIG_PROMIS_EN_FIELD =
{
    "PROMIS_EN",
#if RU_INCLUDE_DESC
    "",
    "Enable/Disable MAC promiscuous operation. When asserted (Set to '1'), \n"
    "all frames are received without Unicast address filtering.",
#endif
    UNIMAC_RDP_COMMAND_CONFIG_PROMIS_EN_FIELD_MASK,
    0,
    UNIMAC_RDP_COMMAND_CONFIG_PROMIS_EN_FIELD_WIDTH,
    UNIMAC_RDP_COMMAND_CONFIG_PROMIS_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_COMMAND_CONFIG_ETH_SPEED
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_COMMAND_CONFIG_ETH_SPEED_FIELD =
{
    "ETH_SPEED",
#if RU_INCLUDE_DESC
    "",
    "Set MAC speed. Ignored when the register bit ENA_EXT_CONFIG is set to '1'.  When the Register bit ENA_EXT_CONFIG is set to '0', used to set the core mode of operation: 00: Enable 10Mbps Ethernet mode 01: Enable 100Mbps Ethernet mode 10: Enable Gigabit Ethernet mode 11: Enable 2.5Gigabit Ethernet mode",
#endif
    UNIMAC_RDP_COMMAND_CONFIG_ETH_SPEED_FIELD_MASK,
    0,
    UNIMAC_RDP_COMMAND_CONFIG_ETH_SPEED_FIELD_WIDTH,
    UNIMAC_RDP_COMMAND_CONFIG_ETH_SPEED_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_COMMAND_CONFIG_RX_ENA
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_COMMAND_CONFIG_RX_ENA_FIELD =
{
    "RX_ENA",
#if RU_INCLUDE_DESC
    "",
    "Enable/Disable MAC receive path. When set to '0' (Reset value), the MAC \n"
    "receive function is disable.  When set to '1', the MAC receive function is enabled.",
#endif
    UNIMAC_RDP_COMMAND_CONFIG_RX_ENA_FIELD_MASK,
    0,
    UNIMAC_RDP_COMMAND_CONFIG_RX_ENA_FIELD_WIDTH,
    UNIMAC_RDP_COMMAND_CONFIG_RX_ENA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_COMMAND_CONFIG_TX_ENA
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_COMMAND_CONFIG_TX_ENA_FIELD =
{
    "TX_ENA",
#if RU_INCLUDE_DESC
    "",
    "Enable/Disable MAC transmit path for data packets & pause/pfc packets sent in the normal data path.\n"
    "Pause/pfc packets generated internally are allowed if ignore_tx_pause is not set. When set to '0' (Reset value), the MAC \n"
    "transmit function is disable.  When set to '1', the MAC transmit function is enabled.",
#endif
    UNIMAC_RDP_COMMAND_CONFIG_TX_ENA_FIELD_MASK,
    0,
    UNIMAC_RDP_COMMAND_CONFIG_TX_ENA_FIELD_WIDTH,
    UNIMAC_RDP_COMMAND_CONFIG_TX_ENA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MAC_0_MAC_ADDR0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MAC_0_MAC_ADDR0_FIELD =
{
    "MAC_ADDR0",
#if RU_INCLUDE_DESC
    "",
    "Register bit 0 corresponds to bit 16 of the MAC address, register bit 1 corresponds to bit 17 of the MAC address, and so on.",
#endif
    UNIMAC_RDP_MAC_0_MAC_ADDR0_FIELD_MASK,
    0,
    UNIMAC_RDP_MAC_0_MAC_ADDR0_FIELD_WIDTH,
    UNIMAC_RDP_MAC_0_MAC_ADDR0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MAC_1_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MAC_1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_MAC_1_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_MAC_1_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_MAC_1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MAC_1_MAC_ADDR1
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MAC_1_MAC_ADDR1_FIELD =
{
    "MAC_ADDR1",
#if RU_INCLUDE_DESC
    "",
    "Register bit 0 corresponds to bit 0 of the MAC address, register bit 1 corresponds to bit 1 of the MAC address, and so on.\n"
    "Bits 16 to 31 are reserved.",
#endif
    UNIMAC_RDP_MAC_1_MAC_ADDR1_FIELD_MASK,
    0,
    UNIMAC_RDP_MAC_1_MAC_ADDR1_FIELD_WIDTH,
    UNIMAC_RDP_MAC_1_MAC_ADDR1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_FRM_LENGTH_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_FRM_LENGTH_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_FRM_LENGTH_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_FRM_LENGTH_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_FRM_LENGTH_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_FRM_LENGTH_MAXFR
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_FRM_LENGTH_MAXFR_FIELD =
{
    "MAXFR",
#if RU_INCLUDE_DESC
    "",
    "Defines a 14-bit maximum frame length used by the MAC receive logic to check frames.",
#endif
    UNIMAC_RDP_FRM_LENGTH_MAXFR_FIELD_MASK,
    0,
    UNIMAC_RDP_FRM_LENGTH_MAXFR_FIELD_WIDTH,
    UNIMAC_RDP_FRM_LENGTH_MAXFR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_PAUSE_QUANT_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_PAUSE_QUANT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_PAUSE_QUANT_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_PAUSE_QUANT_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_PAUSE_QUANT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_PAUSE_QUANT_PAUSE_QUANT
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_PAUSE_QUANT_PAUSE_QUANT_FIELD =
{
    "PAUSE_QUANT",
#if RU_INCLUDE_DESC
    "",
    "16-bit value, sets, in increments of 512 Ethernet bit times, the pause quanta used in \n"
    "each Pause Frame sent to the remote Ethernet device.",
#endif
    UNIMAC_RDP_PAUSE_QUANT_PAUSE_QUANT_FIELD_MASK,
    0,
    UNIMAC_RDP_PAUSE_QUANT_PAUSE_QUANT_FIELD_WIDTH,
    UNIMAC_RDP_PAUSE_QUANT_PAUSE_QUANT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_TX_TS_SEQ_ID_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_TX_TS_SEQ_ID_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_TX_TS_SEQ_ID_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_TX_TS_SEQ_ID_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_TX_TS_SEQ_ID_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_TX_TS_SEQ_ID_TSTS_VALID
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_TX_TS_SEQ_ID_TSTS_VALID_FIELD =
{
    "TSTS_VALID",
#if RU_INCLUDE_DESC
    "",
    "Indicates that a timestamp was captured and is valid. if the cpu reads an empty fifo the VALID bit will be 0.",
#endif
    UNIMAC_RDP_TX_TS_SEQ_ID_TSTS_VALID_FIELD_MASK,
    0,
    UNIMAC_RDP_TX_TS_SEQ_ID_TSTS_VALID_FIELD_WIDTH,
    UNIMAC_RDP_TX_TS_SEQ_ID_TSTS_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_TX_TS_SEQ_ID_TSTS_SEQ_ID
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_TX_TS_SEQ_ID_TSTS_SEQ_ID_FIELD =
{
    "TSTS_SEQ_ID",
#if RU_INCLUDE_DESC
    "",
    "Every read of this register will fetch out one seq_id from the transmit FIFO.(One seq_id per one read command on the sbus).\n"
    "Every 49 bit val_bit + seq_id + timestamp is read in two steps, i.e., one read from 0x10f (val_bit + seq_id) followed by another read from 0x1c7 (timestamp).\n"
    "Timestamp read without a preceding seq_id read will fetch stale timestamp value.",
#endif
    UNIMAC_RDP_TX_TS_SEQ_ID_TSTS_SEQ_ID_FIELD_MASK,
    0,
    UNIMAC_RDP_TX_TS_SEQ_ID_TSTS_SEQ_ID_FIELD_WIDTH,
    UNIMAC_RDP_TX_TS_SEQ_ID_TSTS_SEQ_ID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_SFD_OFFSET_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_SFD_OFFSET_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_SFD_OFFSET_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_SFD_OFFSET_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_SFD_OFFSET_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_SFD_OFFSET_SFD_OFFSET
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_SFD_OFFSET_SFD_OFFSET_FIELD =
{
    "SFD_OFFSET",
#if RU_INCLUDE_DESC
    "",
    "Defines the length of the EFM preamble between 5 and 15 Bytes. When set to 0, 1, 2, 3 or 4,\n"
    "the Preamble EFM length is set to 5 Bytes.",
#endif
    UNIMAC_RDP_SFD_OFFSET_SFD_OFFSET_FIELD_MASK,
    0,
    UNIMAC_RDP_SFD_OFFSET_SFD_OFFSET_FIELD_WIDTH,
    UNIMAC_RDP_SFD_OFFSET_SFD_OFFSET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MAC_MODE_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MAC_MODE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_MAC_MODE_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_MAC_MODE_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_MAC_MODE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MAC_MODE_LINK_STATUS
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MAC_MODE_LINK_STATUS_FIELD =
{
    "LINK_STATUS",
#if RU_INCLUDE_DESC
    "",
    "Link Status Indication. Set to '0', when link_status input is low.\n"
    "Set to '1', when link_status input is High.",
#endif
    UNIMAC_RDP_MAC_MODE_LINK_STATUS_FIELD_MASK,
    0,
    UNIMAC_RDP_MAC_MODE_LINK_STATUS_FIELD_WIDTH,
    UNIMAC_RDP_MAC_MODE_LINK_STATUS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MAC_MODE_MAC_TX_PAUSE
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MAC_MODE_MAC_TX_PAUSE_FIELD =
{
    "MAC_TX_PAUSE",
#if RU_INCLUDE_DESC
    "",
    "MAC Pause Enabled in Transmit. \n"
    "0: MAC Pause Disabled in Transmit\n"
    "1: MAC Pause Enabled in Transmit",
#endif
    UNIMAC_RDP_MAC_MODE_MAC_TX_PAUSE_FIELD_MASK,
    0,
    UNIMAC_RDP_MAC_MODE_MAC_TX_PAUSE_FIELD_WIDTH,
    UNIMAC_RDP_MAC_MODE_MAC_TX_PAUSE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MAC_MODE_MAC_RX_PAUSE
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MAC_MODE_MAC_RX_PAUSE_FIELD =
{
    "MAC_RX_PAUSE",
#if RU_INCLUDE_DESC
    "",
    "MAC Pause Enabled in Receive. \n"
    "0: MAC Pause Disabled in Receive\n"
    "1: MAC Pause Enabled in Receive",
#endif
    UNIMAC_RDP_MAC_MODE_MAC_RX_PAUSE_FIELD_MASK,
    0,
    UNIMAC_RDP_MAC_MODE_MAC_RX_PAUSE_FIELD_WIDTH,
    UNIMAC_RDP_MAC_MODE_MAC_RX_PAUSE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MAC_MODE_MAC_DUPLEX
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MAC_MODE_MAC_DUPLEX_FIELD =
{
    "MAC_DUPLEX",
#if RU_INCLUDE_DESC
    "",
    "MAC Duplex. \n"
    "0: Full Duplex Mode enabled\n"
    "1: Half Duplex Mode enabled",
#endif
    UNIMAC_RDP_MAC_MODE_MAC_DUPLEX_FIELD_MASK,
    0,
    UNIMAC_RDP_MAC_MODE_MAC_DUPLEX_FIELD_WIDTH,
    UNIMAC_RDP_MAC_MODE_MAC_DUPLEX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MAC_MODE_MAC_SPEED
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MAC_MODE_MAC_SPEED_FIELD =
{
    "MAC_SPEED",
#if RU_INCLUDE_DESC
    "",
    "MAC Speed. \n"
    "00: 10Mbps Ethernet Mode enabled\n"
    "01: 100Mbps Ethernet Mode enabled\n"
    "10: Gigabit Ethernet Mode enabled\n"
    "11: 2.5Gigabit Ethernet Mode enabled",
#endif
    UNIMAC_RDP_MAC_MODE_MAC_SPEED_FIELD_MASK,
    0,
    UNIMAC_RDP_MAC_MODE_MAC_SPEED_FIELD_WIDTH,
    UNIMAC_RDP_MAC_MODE_MAC_SPEED_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_TAG_0_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_TAG_0_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_TAG_0_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_TAG_0_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_TAG_0_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_TAG_0_CONFIG_OUTER_TPID_ENABLE
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_TAG_0_CONFIG_OUTER_TPID_ENABLE_FIELD =
{
    "CONFIG_OUTER_TPID_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "If cleared then disable outer TPID detection",
#endif
    UNIMAC_RDP_TAG_0_CONFIG_OUTER_TPID_ENABLE_FIELD_MASK,
    0,
    UNIMAC_RDP_TAG_0_CONFIG_OUTER_TPID_ENABLE_FIELD_WIDTH,
    UNIMAC_RDP_TAG_0_CONFIG_OUTER_TPID_ENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_TAG_0_FRM_TAG_0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_TAG_0_FRM_TAG_0_FIELD =
{
    "FRM_TAG_0",
#if RU_INCLUDE_DESC
    "",
    "Outer tag of the programmable VLAN tag",
#endif
    UNIMAC_RDP_TAG_0_FRM_TAG_0_FIELD_MASK,
    0,
    UNIMAC_RDP_TAG_0_FRM_TAG_0_FIELD_WIDTH,
    UNIMAC_RDP_TAG_0_FRM_TAG_0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_TAG_1_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_TAG_1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_TAG_1_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_TAG_1_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_TAG_1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_TAG_1_CONFIG_INNER_TPID_ENABLE
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_TAG_1_CONFIG_INNER_TPID_ENABLE_FIELD =
{
    "CONFIG_INNER_TPID_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "If cleared then disable inner TPID detection",
#endif
    UNIMAC_RDP_TAG_1_CONFIG_INNER_TPID_ENABLE_FIELD_MASK,
    0,
    UNIMAC_RDP_TAG_1_CONFIG_INNER_TPID_ENABLE_FIELD_WIDTH,
    UNIMAC_RDP_TAG_1_CONFIG_INNER_TPID_ENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_TAG_1_FRM_TAG_1
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_TAG_1_FRM_TAG_1_FIELD =
{
    "FRM_TAG_1",
#if RU_INCLUDE_DESC
    "",
    "inner tag of the programmable VLAN tag",
#endif
    UNIMAC_RDP_TAG_1_FRM_TAG_1_FIELD_MASK,
    0,
    UNIMAC_RDP_TAG_1_FRM_TAG_1_FIELD_WIDTH,
    UNIMAC_RDP_TAG_1_FRM_TAG_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_RX_PAUSE_QUANTA_SCALE_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_RX_PAUSE_QUANTA_SCALE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_RX_PAUSE_QUANTA_SCALE_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_RX_PAUSE_QUANTA_SCALE_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_RX_PAUSE_QUANTA_SCALE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_RX_PAUSE_QUANTA_SCALE_SCALE_FIX
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_RX_PAUSE_QUANTA_SCALE_SCALE_FIX_FIELD =
{
    "SCALE_FIX",
#if RU_INCLUDE_DESC
    "",
    "If set, then receive pause quanta is ignored and a fixed quanta value programmed in SCALE_VALUE is loaded into the pause timer.\n"
    "If set, then SCALE_CONTROL is ignored.\n"
    "If cleared, then SCALE_CONTROL takes into effect.",
#endif
    UNIMAC_RDP_RX_PAUSE_QUANTA_SCALE_SCALE_FIX_FIELD_MASK,
    0,
    UNIMAC_RDP_RX_PAUSE_QUANTA_SCALE_SCALE_FIX_FIELD_WIDTH,
    UNIMAC_RDP_RX_PAUSE_QUANTA_SCALE_SCALE_FIX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_RX_PAUSE_QUANTA_SCALE_SCALE_CONTROL
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_RX_PAUSE_QUANTA_SCALE_SCALE_CONTROL_FIELD =
{
    "SCALE_CONTROL",
#if RU_INCLUDE_DESC
    "",
    "If clear, then subtract the scale_value from the received pause quanta. \n"
    "If set, then add the scale_value from the received pause quanta.",
#endif
    UNIMAC_RDP_RX_PAUSE_QUANTA_SCALE_SCALE_CONTROL_FIELD_MASK,
    0,
    UNIMAC_RDP_RX_PAUSE_QUANTA_SCALE_SCALE_CONTROL_FIELD_WIDTH,
    UNIMAC_RDP_RX_PAUSE_QUANTA_SCALE_SCALE_CONTROL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_RX_PAUSE_QUANTA_SCALE_SCALE_VALUE
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_RX_PAUSE_QUANTA_SCALE_SCALE_VALUE_FIELD =
{
    "SCALE_VALUE",
#if RU_INCLUDE_DESC
    "",
    "The pause timer is loaded with the value obtained after adding or subtracting the scale_value from the received pause quanta.",
#endif
    UNIMAC_RDP_RX_PAUSE_QUANTA_SCALE_SCALE_VALUE_FIELD_MASK,
    0,
    UNIMAC_RDP_RX_PAUSE_QUANTA_SCALE_SCALE_VALUE_FIELD_WIDTH,
    UNIMAC_RDP_RX_PAUSE_QUANTA_SCALE_SCALE_VALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_TX_PREAMBLE_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_TX_PREAMBLE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_TX_PREAMBLE_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_TX_PREAMBLE_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_TX_PREAMBLE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_TX_PREAMBLE_TX_PREAMBLE
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_TX_PREAMBLE_TX_PREAMBLE_FIELD =
{
    "TX_PREAMBLE",
#if RU_INCLUDE_DESC
    "",
    "Set the transmit preamble excluding SFD to be programmable from min of 2 bytes to the max allowable of 7 bytes, with granularity of 1 byte.",
#endif
    UNIMAC_RDP_TX_PREAMBLE_TX_PREAMBLE_FIELD_MASK,
    0,
    UNIMAC_RDP_TX_PREAMBLE_TX_PREAMBLE_FIELD_WIDTH,
    UNIMAC_RDP_TX_PREAMBLE_TX_PREAMBLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_TX_IPG_LENGTH_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_TX_IPG_LENGTH_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_TX_IPG_LENGTH_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_TX_IPG_LENGTH_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_TX_IPG_LENGTH_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_TX_IPG_LENGTH_TX_MIN_PKT_SIZE
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_TX_IPG_LENGTH_TX_MIN_PKT_SIZE_FIELD =
{
    "TX_MIN_PKT_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Min. TX packet size without FCS, also without preamble+SFD.\n"
    "Padding will be appended if needed to ensure this size.\n"
    "Valid values are: 14..125",
#endif
    UNIMAC_RDP_TX_IPG_LENGTH_TX_MIN_PKT_SIZE_FIELD_MASK,
    0,
    UNIMAC_RDP_TX_IPG_LENGTH_TX_MIN_PKT_SIZE_FIELD_WIDTH,
    UNIMAC_RDP_TX_IPG_LENGTH_TX_MIN_PKT_SIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_TX_IPG_LENGTH_RESERVED1
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_TX_IPG_LENGTH_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_TX_IPG_LENGTH_RESERVED1_FIELD_MASK,
    0,
    UNIMAC_RDP_TX_IPG_LENGTH_RESERVED1_FIELD_WIDTH,
    UNIMAC_RDP_TX_IPG_LENGTH_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_TX_IPG_LENGTH_TX_IPG_LENGTH
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_TX_IPG_LENGTH_TX_IPG_LENGTH_FIELD =
{
    "TX_IPG_LENGTH",
#if RU_INCLUDE_DESC
    "",
    "Set the Transmit minimum IPG from 8 to 64 Byte-times. If a value below 8 or above 64 is\n"
    "programmed, the minimum IPG is set to 12 byte-times.",
#endif
    UNIMAC_RDP_TX_IPG_LENGTH_TX_IPG_LENGTH_FIELD_MASK,
    0,
    UNIMAC_RDP_TX_IPG_LENGTH_TX_IPG_LENGTH_FIELD_WIDTH,
    UNIMAC_RDP_TX_IPG_LENGTH_TX_IPG_LENGTH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_PFC_XOFF_TIMER_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_PFC_XOFF_TIMER_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_PFC_XOFF_TIMER_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_PFC_XOFF_TIMER_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_PFC_XOFF_TIMER_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_PFC_XOFF_TIMER_PFC_XOFF_TIMER
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_PFC_XOFF_TIMER_PFC_XOFF_TIMER_FIELD =
{
    "PFC_XOFF_TIMER",
#if RU_INCLUDE_DESC
    "",
    "Time value sent in the Timer Field for classes in XOFF state (Unit is 512 bit-times).",
#endif
    UNIMAC_RDP_PFC_XOFF_TIMER_PFC_XOFF_TIMER_FIELD_MASK,
    0,
    UNIMAC_RDP_PFC_XOFF_TIMER_PFC_XOFF_TIMER_FIELD_WIDTH,
    UNIMAC_RDP_PFC_XOFF_TIMER_PFC_XOFF_TIMER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_UMAC_EEE_CTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_UMAC_EEE_CTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_UMAC_EEE_CTRL_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_UMAC_EEE_CTRL_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_UMAC_EEE_CTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_UMAC_EEE_CTRL_LP_IDLE_PREDICTION_MODE
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_UMAC_EEE_CTRL_LP_IDLE_PREDICTION_MODE_FIELD =
{
    "LP_IDLE_PREDICTION_MODE",
#if RU_INCLUDE_DESC
    "",
    "When set to 1, enables LP_IDLE Prediction. When set to 0, disables LP_IDLE Prediction.",
#endif
    UNIMAC_RDP_UMAC_EEE_CTRL_LP_IDLE_PREDICTION_MODE_FIELD_MASK,
    0,
    UNIMAC_RDP_UMAC_EEE_CTRL_LP_IDLE_PREDICTION_MODE_FIELD_WIDTH,
    UNIMAC_RDP_UMAC_EEE_CTRL_LP_IDLE_PREDICTION_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_UMAC_EEE_CTRL_DIS_EEE_10M
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_UMAC_EEE_CTRL_DIS_EEE_10M_FIELD =
{
    "DIS_EEE_10M",
#if RU_INCLUDE_DESC
    "",
    "When this bit is set and link is established at 10Mbps, LPI is not supported (saving is achieved by reduced PHY's output swing). UNIMAC ignores EEE feature on both Tx & Rx in 10Mbps.\n"
    "When cleared, Unimac doesn't differentiate between speeds for EEE feature.",
#endif
    UNIMAC_RDP_UMAC_EEE_CTRL_DIS_EEE_10M_FIELD_MASK,
    0,
    UNIMAC_RDP_UMAC_EEE_CTRL_DIS_EEE_10M_FIELD_WIDTH,
    UNIMAC_RDP_UMAC_EEE_CTRL_DIS_EEE_10M_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_UMAC_EEE_CTRL_EEE_TXCLK_DIS
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_UMAC_EEE_CTRL_EEE_TXCLK_DIS_FIELD =
{
    "EEE_TXCLK_DIS",
#if RU_INCLUDE_DESC
    "",
    "If enabled, UNIMAC will shut down TXCLK to PHY, when in LPI state.",
#endif
    UNIMAC_RDP_UMAC_EEE_CTRL_EEE_TXCLK_DIS_FIELD_MASK,
    0,
    UNIMAC_RDP_UMAC_EEE_CTRL_EEE_TXCLK_DIS_FIELD_WIDTH,
    UNIMAC_RDP_UMAC_EEE_CTRL_EEE_TXCLK_DIS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_UMAC_EEE_CTRL_RX_FIFO_CHECK
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_UMAC_EEE_CTRL_RX_FIFO_CHECK_FIELD =
{
    "RX_FIFO_CHECK",
#if RU_INCLUDE_DESC
    "",
    "If enabled, lpi_rx_detect is set whenever the LPI_IDLES are being received on the RX line and Unimac Rx FIFO is empty.\n"
    "By default, lpi_rx_detect is set only when whenever the LPI_IDLES are being received on the RX line.",
#endif
    UNIMAC_RDP_UMAC_EEE_CTRL_RX_FIFO_CHECK_FIELD_MASK,
    0,
    UNIMAC_RDP_UMAC_EEE_CTRL_RX_FIFO_CHECK_FIELD_WIDTH,
    UNIMAC_RDP_UMAC_EEE_CTRL_RX_FIFO_CHECK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_UMAC_EEE_CTRL_EEE_EN
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_UMAC_EEE_CTRL_EEE_EN_FIELD =
{
    "EEE_EN",
#if RU_INCLUDE_DESC
    "",
    "If set, the TX LPI policy control engine is enabled and the MAC inserts LPI_idle codes if the link is idle. The rx_lpi_detect assertion is independent of this configuration. Reset default depends on EEE_en_strap input, which if tied to 1, defaults to enabled, otherwise if tied to 0, defaults to disabled.",
#endif
    UNIMAC_RDP_UMAC_EEE_CTRL_EEE_EN_FIELD_MASK,
    0,
    UNIMAC_RDP_UMAC_EEE_CTRL_EEE_EN_FIELD_WIDTH,
    UNIMAC_RDP_UMAC_EEE_CTRL_EEE_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_UMAC_EEE_CTRL_RESERVED1
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_UMAC_EEE_CTRL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_UMAC_EEE_CTRL_RESERVED1_FIELD_MASK,
    0,
    UNIMAC_RDP_UMAC_EEE_CTRL_RESERVED1_FIELD_WIDTH,
    UNIMAC_RDP_UMAC_EEE_CTRL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MII_EEE_DELAY_ENTRY_TIMER_MII_EEE_LPI_TIMER
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MII_EEE_DELAY_ENTRY_TIMER_MII_EEE_LPI_TIMER_FIELD =
{
    "MII_EEE_LPI_TIMER",
#if RU_INCLUDE_DESC
    "",
    "This is the duration for which condition to move to LPI state must be satisfied, at the end of which MAC transitions to LPI State. The decrement unit is 1 micro-second.\n"
    "This register is meant for 10/100 Mbps speed.",
#endif
    UNIMAC_RDP_MII_EEE_DELAY_ENTRY_TIMER_MII_EEE_LPI_TIMER_FIELD_MASK,
    0,
    UNIMAC_RDP_MII_EEE_DELAY_ENTRY_TIMER_MII_EEE_LPI_TIMER_FIELD_WIDTH,
    UNIMAC_RDP_MII_EEE_DELAY_ENTRY_TIMER_MII_EEE_LPI_TIMER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_GMII_EEE_DELAY_ENTRY_TIMER_GMII_EEE_LPI_TIMER
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_GMII_EEE_DELAY_ENTRY_TIMER_GMII_EEE_LPI_TIMER_FIELD =
{
    "GMII_EEE_LPI_TIMER",
#if RU_INCLUDE_DESC
    "",
    "This is the duration for which condition to move to LPI state must be satisfied, at the end of which MAC transitions to LPI State. The decrement unit is 1 micro-second.\n"
    "This register is meant for 1000 Mbps speed.",
#endif
    UNIMAC_RDP_GMII_EEE_DELAY_ENTRY_TIMER_GMII_EEE_LPI_TIMER_FIELD_MASK,
    0,
    UNIMAC_RDP_GMII_EEE_DELAY_ENTRY_TIMER_GMII_EEE_LPI_TIMER_FIELD_WIDTH,
    UNIMAC_RDP_GMII_EEE_DELAY_ENTRY_TIMER_GMII_EEE_LPI_TIMER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_UMAC_EEE_REF_COUNT_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_UMAC_EEE_REF_COUNT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_UMAC_EEE_REF_COUNT_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_UMAC_EEE_REF_COUNT_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_UMAC_EEE_REF_COUNT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_UMAC_EEE_REF_COUNT_EEE_REF_COUNT
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_UMAC_EEE_REF_COUNT_EEE_REF_COUNT_FIELD =
{
    "EEE_REF_COUNT",
#if RU_INCLUDE_DESC
    "",
    "This field controls clock divider used to generate ~1us reference pulses used by EEE timers. It specifies integer number of timer clock cycles contained within 1us.",
#endif
    UNIMAC_RDP_UMAC_EEE_REF_COUNT_EEE_REF_COUNT_FIELD_MASK,
    0,
    UNIMAC_RDP_UMAC_EEE_REF_COUNT_EEE_REF_COUNT_FIELD_WIDTH,
    UNIMAC_RDP_UMAC_EEE_REF_COUNT_EEE_REF_COUNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_UMAC_TIMESTAMP_ADJUST_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_UMAC_TIMESTAMP_ADJUST_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_UMAC_TIMESTAMP_ADJUST_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_UMAC_TIMESTAMP_ADJUST_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_UMAC_TIMESTAMP_ADJUST_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_UMAC_TIMESTAMP_ADJUST_AUTO_ADJUST
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_UMAC_TIMESTAMP_ADJUST_AUTO_ADJUST_FIELD =
{
    "AUTO_ADJUST",
#if RU_INCLUDE_DESC
    "",
    "Enables MAC Rx timestamp offset balancing at MAC TX timestamp.",
#endif
    UNIMAC_RDP_UMAC_TIMESTAMP_ADJUST_AUTO_ADJUST_FIELD_MASK,
    0,
    UNIMAC_RDP_UMAC_TIMESTAMP_ADJUST_AUTO_ADJUST_FIELD_WIDTH,
    UNIMAC_RDP_UMAC_TIMESTAMP_ADJUST_AUTO_ADJUST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_UMAC_TIMESTAMP_ADJUST_EN_1588
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_UMAC_TIMESTAMP_ADJUST_EN_1588_FIELD =
{
    "EN_1588",
#if RU_INCLUDE_DESC
    "",
    "Enables 1588 one step timestamp feature.",
#endif
    UNIMAC_RDP_UMAC_TIMESTAMP_ADJUST_EN_1588_FIELD_MASK,
    0,
    UNIMAC_RDP_UMAC_TIMESTAMP_ADJUST_EN_1588_FIELD_WIDTH,
    UNIMAC_RDP_UMAC_TIMESTAMP_ADJUST_EN_1588_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_UMAC_TIMESTAMP_ADJUST_ADJUST
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_UMAC_TIMESTAMP_ADJUST_ADJUST_FIELD =
{
    "ADJUST",
#if RU_INCLUDE_DESC
    "",
    "Offset adjustment to outgoing TIMESTAMP to adjust for pipeline stalling and/or jitter asymmetry. The value is in 2's compliment format and is of 1ns granularity.",
#endif
    UNIMAC_RDP_UMAC_TIMESTAMP_ADJUST_ADJUST_FIELD_MASK,
    0,
    UNIMAC_RDP_UMAC_TIMESTAMP_ADJUST_ADJUST_FIELD_WIDTH,
    UNIMAC_RDP_UMAC_TIMESTAMP_ADJUST_ADJUST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_UMAC_RX_PKT_DROP_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_UMAC_RX_PKT_DROP_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_UMAC_RX_PKT_DROP_STATUS_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_UMAC_RX_PKT_DROP_STATUS_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_UMAC_RX_PKT_DROP_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_UMAC_RX_PKT_DROP_STATUS_RX_IPG_INVAL
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_UMAC_RX_PKT_DROP_STATUS_RX_IPG_INVAL_FIELD =
{
    "RX_IPG_INVAL",
#if RU_INCLUDE_DESC
    "",
    "Debug status, set if MAC receives an IPG less than programmed RX IPG or less than four bytes. Sticky bit. Clears when SW writes 0 into the field or by sw_reset.",
#endif
    UNIMAC_RDP_UMAC_RX_PKT_DROP_STATUS_RX_IPG_INVAL_FIELD_MASK,
    0,
    UNIMAC_RDP_UMAC_RX_PKT_DROP_STATUS_RX_IPG_INVAL_FIELD_WIDTH,
    UNIMAC_RDP_UMAC_RX_PKT_DROP_STATUS_RX_IPG_INVAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_UMAC_SYMMETRIC_IDLE_THRESHOLD_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_UMAC_SYMMETRIC_IDLE_THRESHOLD_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_UMAC_SYMMETRIC_IDLE_THRESHOLD_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_UMAC_SYMMETRIC_IDLE_THRESHOLD_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_UMAC_SYMMETRIC_IDLE_THRESHOLD_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_UMAC_SYMMETRIC_IDLE_THRESHOLD_THRESHOLD_VALUE
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_UMAC_SYMMETRIC_IDLE_THRESHOLD_THRESHOLD_VALUE_FIELD =
{
    "THRESHOLD_VALUE",
#if RU_INCLUDE_DESC
    "",
    "If LPI_Prediction is enabled then this register defines the number of IDLEs to be received by the UniMAC before allowing LP_IDLE to be sent to Link Partner.",
#endif
    UNIMAC_RDP_UMAC_SYMMETRIC_IDLE_THRESHOLD_THRESHOLD_VALUE_FIELD_MASK,
    0,
    UNIMAC_RDP_UMAC_SYMMETRIC_IDLE_THRESHOLD_THRESHOLD_VALUE_FIELD_WIDTH,
    UNIMAC_RDP_UMAC_SYMMETRIC_IDLE_THRESHOLD_THRESHOLD_VALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MII_EEE_WAKE_TIMER_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MII_EEE_WAKE_TIMER_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_MII_EEE_WAKE_TIMER_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_MII_EEE_WAKE_TIMER_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_MII_EEE_WAKE_TIMER_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MII_EEE_WAKE_TIMER_MII_EEE_WAKE_TIMER
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MII_EEE_WAKE_TIMER_MII_EEE_WAKE_TIMER_FIELD =
{
    "MII_EEE_WAKE_TIMER",
#if RU_INCLUDE_DESC
    "",
    "This is the duration for which MAC must wait to go back to ACTIVE state from LPI state when it receives packet for transmission. The decrement unit is 1 micro-second.\n"
    "This register is meant for 100 Mbps speed.",
#endif
    UNIMAC_RDP_MII_EEE_WAKE_TIMER_MII_EEE_WAKE_TIMER_FIELD_MASK,
    0,
    UNIMAC_RDP_MII_EEE_WAKE_TIMER_MII_EEE_WAKE_TIMER_FIELD_WIDTH,
    UNIMAC_RDP_MII_EEE_WAKE_TIMER_MII_EEE_WAKE_TIMER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_GMII_EEE_WAKE_TIMER_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_GMII_EEE_WAKE_TIMER_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_GMII_EEE_WAKE_TIMER_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_GMII_EEE_WAKE_TIMER_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_GMII_EEE_WAKE_TIMER_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_GMII_EEE_WAKE_TIMER_GMII_EEE_WAKE_TIMER
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_GMII_EEE_WAKE_TIMER_GMII_EEE_WAKE_TIMER_FIELD =
{
    "GMII_EEE_WAKE_TIMER",
#if RU_INCLUDE_DESC
    "",
    "This is the duration for which MAC must wait to go back to ACTIVE state from LPI state when it receives packet for transmission. The decrement unit is 1 micro-second.\n"
    "This register is meant for 1000 Mbps speed.",
#endif
    UNIMAC_RDP_GMII_EEE_WAKE_TIMER_GMII_EEE_WAKE_TIMER_FIELD_MASK,
    0,
    UNIMAC_RDP_GMII_EEE_WAKE_TIMER_GMII_EEE_WAKE_TIMER_FIELD_WIDTH,
    UNIMAC_RDP_GMII_EEE_WAKE_TIMER_GMII_EEE_WAKE_TIMER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_UMAC_REV_ID_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_UMAC_REV_ID_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_UMAC_REV_ID_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_UMAC_REV_ID_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_UMAC_REV_ID_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_UMAC_REV_ID_REVISION_ID_MAJOR
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_UMAC_REV_ID_REVISION_ID_MAJOR_FIELD =
{
    "REVISION_ID_MAJOR",
#if RU_INCLUDE_DESC
    "",
    "Unimac version id field before decimal.",
#endif
    UNIMAC_RDP_UMAC_REV_ID_REVISION_ID_MAJOR_FIELD_MASK,
    0,
    UNIMAC_RDP_UMAC_REV_ID_REVISION_ID_MAJOR_FIELD_WIDTH,
    UNIMAC_RDP_UMAC_REV_ID_REVISION_ID_MAJOR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_UMAC_REV_ID_REVISION_ID_MINOR
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_UMAC_REV_ID_REVISION_ID_MINOR_FIELD =
{
    "REVISION_ID_MINOR",
#if RU_INCLUDE_DESC
    "",
    "Unimac version id field after decimal.",
#endif
    UNIMAC_RDP_UMAC_REV_ID_REVISION_ID_MINOR_FIELD_MASK,
    0,
    UNIMAC_RDP_UMAC_REV_ID_REVISION_ID_MINOR_FIELD_WIDTH,
    UNIMAC_RDP_UMAC_REV_ID_REVISION_ID_MINOR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_UMAC_REV_ID_PATCH
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_UMAC_REV_ID_PATCH_FIELD =
{
    "PATCH",
#if RU_INCLUDE_DESC
    "",
    "Unimac revision patch number.",
#endif
    UNIMAC_RDP_UMAC_REV_ID_PATCH_FIELD_MASK,
    0,
    UNIMAC_RDP_UMAC_REV_ID_PATCH_FIELD_WIDTH,
    UNIMAC_RDP_UMAC_REV_ID_PATCH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MAC_PFC_TYPE_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MAC_PFC_TYPE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_MAC_PFC_TYPE_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_MAC_PFC_TYPE_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_MAC_PFC_TYPE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MAC_PFC_TYPE_PFC_ETH_TYPE
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MAC_PFC_TYPE_PFC_ETH_TYPE_FIELD =
{
    "PFC_ETH_TYPE",
#if RU_INCLUDE_DESC
    "",
    "Ethertype for PFC packets. The default value (0x8808) is the standard value.",
#endif
    UNIMAC_RDP_MAC_PFC_TYPE_PFC_ETH_TYPE_FIELD_MASK,
    0,
    UNIMAC_RDP_MAC_PFC_TYPE_PFC_ETH_TYPE_FIELD_WIDTH,
    UNIMAC_RDP_MAC_PFC_TYPE_PFC_ETH_TYPE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MAC_PFC_OPCODE_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MAC_PFC_OPCODE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_MAC_PFC_OPCODE_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_MAC_PFC_OPCODE_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_MAC_PFC_OPCODE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MAC_PFC_OPCODE_PFC_OPCODE
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MAC_PFC_OPCODE_PFC_OPCODE_FIELD =
{
    "PFC_OPCODE",
#if RU_INCLUDE_DESC
    "",
    "PFC opcode. The default value (0x0101) is the standard value.",
#endif
    UNIMAC_RDP_MAC_PFC_OPCODE_PFC_OPCODE_FIELD_MASK,
    0,
    UNIMAC_RDP_MAC_PFC_OPCODE_PFC_OPCODE_FIELD_WIDTH,
    UNIMAC_RDP_MAC_PFC_OPCODE_PFC_OPCODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MAC_PFC_DA_0_PFC_MACDA_0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MAC_PFC_DA_0_PFC_MACDA_0_FIELD =
{
    "PFC_MACDA_0",
#if RU_INCLUDE_DESC
    "",
    "Lower 32 bits of DA for PFC.",
#endif
    UNIMAC_RDP_MAC_PFC_DA_0_PFC_MACDA_0_FIELD_MASK,
    0,
    UNIMAC_RDP_MAC_PFC_DA_0_PFC_MACDA_0_FIELD_WIDTH,
    UNIMAC_RDP_MAC_PFC_DA_0_PFC_MACDA_0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MAC_PFC_DA_1_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MAC_PFC_DA_1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_MAC_PFC_DA_1_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_MAC_PFC_DA_1_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_MAC_PFC_DA_1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MAC_PFC_DA_1_PFC_MACDA_1
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MAC_PFC_DA_1_PFC_MACDA_1_FIELD =
{
    "PFC_MACDA_1",
#if RU_INCLUDE_DESC
    "",
    "Upper 16 bits of DA for PFC.",
#endif
    UNIMAC_RDP_MAC_PFC_DA_1_PFC_MACDA_1_FIELD_MASK,
    0,
    UNIMAC_RDP_MAC_PFC_DA_1_PFC_MACDA_1_FIELD_WIDTH,
    UNIMAC_RDP_MAC_PFC_DA_1_PFC_MACDA_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MACSEC_PROG_TX_CRC_MACSEC_PROG_TX_CRC
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MACSEC_PROG_TX_CRC_MACSEC_PROG_TX_CRC_FIELD =
{
    "MACSEC_PROG_TX_CRC",
#if RU_INCLUDE_DESC
    "",
    "The transmitted CRC can be corrupted by replacing the FCS of the transmitted frame by the FCS programmed in this register.\n"
    "This is enabled and controlled by MACSEC_CNTRL register.",
#endif
    UNIMAC_RDP_MACSEC_PROG_TX_CRC_MACSEC_PROG_TX_CRC_FIELD_MASK,
    0,
    UNIMAC_RDP_MACSEC_PROG_TX_CRC_MACSEC_PROG_TX_CRC_FIELD_WIDTH,
    UNIMAC_RDP_MACSEC_PROG_TX_CRC_MACSEC_PROG_TX_CRC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MACSEC_CNTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MACSEC_CNTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_MACSEC_CNTRL_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_MACSEC_CNTRL_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_MACSEC_CNTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MACSEC_CNTRL_DIS_PAUSE_DATA_VAR_IPG
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MACSEC_CNTRL_DIS_PAUSE_DATA_VAR_IPG_FIELD =
{
    "DIS_PAUSE_DATA_VAR_IPG",
#if RU_INCLUDE_DESC
    "",
    "When this bit is 1, IPG between pause and data frame is as per the original design, i.e., 13B or 12B, fixed. It should be noted, that as number of preamble bytes reduces from 7, the IPG also increases. \n"
    "When this bit is 0, IPG between pause and data frame is variable and equals programmed IPG or programmed IPG + 1.",
#endif
    UNIMAC_RDP_MACSEC_CNTRL_DIS_PAUSE_DATA_VAR_IPG_FIELD_MASK,
    0,
    UNIMAC_RDP_MACSEC_CNTRL_DIS_PAUSE_DATA_VAR_IPG_FIELD_WIDTH,
    UNIMAC_RDP_MACSEC_CNTRL_DIS_PAUSE_DATA_VAR_IPG_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MACSEC_CNTRL_TX_CRC_PROGRAM
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MACSEC_CNTRL_TX_CRC_PROGRAM_FIELD =
{
    "TX_CRC_PROGRAM",
#if RU_INCLUDE_DESC
    "",
    "If CRC corruption feature in enabled (TX_CRC_CORUPT_EN set), then in case where this bit when set, replaces the transmitted FCS with the programmed FCS.\n"
    "When cleared, corrupts the CRC of the transmitted packet internally.",
#endif
    UNIMAC_RDP_MACSEC_CNTRL_TX_CRC_PROGRAM_FIELD_MASK,
    0,
    UNIMAC_RDP_MACSEC_CNTRL_TX_CRC_PROGRAM_FIELD_WIDTH,
    UNIMAC_RDP_MACSEC_CNTRL_TX_CRC_PROGRAM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MACSEC_CNTRL_TX_CRC_CORUPT_EN
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MACSEC_CNTRL_TX_CRC_CORUPT_EN_FIELD =
{
    "TX_CRC_CORUPT_EN",
#if RU_INCLUDE_DESC
    "",
    "Setting this field enables the CRC corruption on the transmitted packets. The options of how to corrupt, depends on\n"
    "the field 2 of this register (TX_CRC_PROGRAM). The CRC corruption happens only on the frames for which TXCRCER is asserted by the system.",
#endif
    UNIMAC_RDP_MACSEC_CNTRL_TX_CRC_CORUPT_EN_FIELD_MASK,
    0,
    UNIMAC_RDP_MACSEC_CNTRL_TX_CRC_CORUPT_EN_FIELD_WIDTH,
    UNIMAC_RDP_MACSEC_CNTRL_TX_CRC_CORUPT_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MACSEC_CNTRL_TX_LAUNCH_EN
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MACSEC_CNTRL_TX_LAUNCH_EN_FIELD =
{
    "TX_LAUNCH_EN",
#if RU_INCLUDE_DESC
    "",
    "Set the bit 0 (Tx_Launch_en) logic 0, if the tx_launch function is to be disabled. If set, then the launch_enable signal assertion/deassertion causes the packet transmit enabled/disabled. The launch_enable is per packet basis.",
#endif
    UNIMAC_RDP_MACSEC_CNTRL_TX_LAUNCH_EN_FIELD_MASK,
    0,
    UNIMAC_RDP_MACSEC_CNTRL_TX_LAUNCH_EN_FIELD_WIDTH,
    UNIMAC_RDP_MACSEC_CNTRL_TX_LAUNCH_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_TS_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_TS_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_TS_STATUS_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_TS_STATUS_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_TS_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_TS_STATUS_WORD_AVAIL
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_TS_STATUS_WORD_AVAIL_FIELD =
{
    "WORD_AVAIL",
#if RU_INCLUDE_DESC
    "",
    "Indicates number of cells filled in the TX timestamp FIFO.",
#endif
    UNIMAC_RDP_TS_STATUS_WORD_AVAIL_FIELD_MASK,
    0,
    UNIMAC_RDP_TS_STATUS_WORD_AVAIL_FIELD_WIDTH,
    UNIMAC_RDP_TS_STATUS_WORD_AVAIL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_TS_STATUS_TX_TS_FIFO_EMPTY
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_TS_STATUS_TX_TS_FIFO_EMPTY_FIELD =
{
    "TX_TS_FIFO_EMPTY",
#if RU_INCLUDE_DESC
    "",
    "Read-only field assertion shows that the transmit timestamp FIFO is empty.",
#endif
    UNIMAC_RDP_TS_STATUS_TX_TS_FIFO_EMPTY_FIELD_MASK,
    0,
    UNIMAC_RDP_TS_STATUS_TX_TS_FIFO_EMPTY_FIELD_WIDTH,
    UNIMAC_RDP_TS_STATUS_TX_TS_FIFO_EMPTY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_TS_STATUS_TX_TS_FIFO_FULL
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_TS_STATUS_TX_TS_FIFO_FULL_FIELD =
{
    "TX_TS_FIFO_FULL",
#if RU_INCLUDE_DESC
    "",
    "Read-only field assertion shows that the transmit timestamp FIFO is full.",
#endif
    UNIMAC_RDP_TS_STATUS_TX_TS_FIFO_FULL_FIELD_MASK,
    0,
    UNIMAC_RDP_TS_STATUS_TX_TS_FIFO_FULL_FIELD_WIDTH,
    UNIMAC_RDP_TS_STATUS_TX_TS_FIFO_FULL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_TX_TS_DATA_TX_TS_DATA
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_TX_TS_DATA_TX_TS_DATA_FIELD =
{
    "TX_TS_DATA",
#if RU_INCLUDE_DESC
    "",
    "Every read of this register will fetch out one timestamp value corresponding to the preceding seq_id read from the transmit FIFO.\n"
    "Every 49 bit, val_bit + seq_id + timestamp is read in two steps, i.e., one read from 0x10f (val_bit + seq_id) followed by another read from 0x1c7 (timestamp).\n"
    "Timestamp read without a preceding seq_id read will fetch stale timestamp value.",
#endif
    UNIMAC_RDP_TX_TS_DATA_TX_TS_DATA_FIELD_MASK,
    0,
    UNIMAC_RDP_TX_TS_DATA_TX_TS_DATA_FIELD_WIDTH,
    UNIMAC_RDP_TX_TS_DATA_TX_TS_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_PAUSE_REFRESH_CTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_PAUSE_REFRESH_CTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_PAUSE_REFRESH_CTRL_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_PAUSE_REFRESH_CTRL_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_PAUSE_REFRESH_CTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_PAUSE_REFRESH_CTRL_ENABLE
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_PAUSE_REFRESH_CTRL_ENABLE_FIELD =
{
    "ENABLE",
#if RU_INCLUDE_DESC
    "",
    "Enable extra pause frames.",
#endif
    UNIMAC_RDP_PAUSE_REFRESH_CTRL_ENABLE_FIELD_MASK,
    0,
    UNIMAC_RDP_PAUSE_REFRESH_CTRL_ENABLE_FIELD_WIDTH,
    UNIMAC_RDP_PAUSE_REFRESH_CTRL_ENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_PAUSE_REFRESH_CTRL_REFRESH_TIMER
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_PAUSE_REFRESH_CTRL_REFRESH_TIMER_FIELD =
{
    "REFRESH_TIMER",
#if RU_INCLUDE_DESC
    "",
    "Timer expiry time, represented in 512 bit time units. Note that the actual expiry time depends on the port speed. Values of 0 and 1 are illegal.",
#endif
    UNIMAC_RDP_PAUSE_REFRESH_CTRL_REFRESH_TIMER_FIELD_MASK,
    0,
    UNIMAC_RDP_PAUSE_REFRESH_CTRL_REFRESH_TIMER_FIELD_WIDTH,
    UNIMAC_RDP_PAUSE_REFRESH_CTRL_REFRESH_TIMER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_FLUSH_CONTROL_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_FLUSH_CONTROL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_FLUSH_CONTROL_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_FLUSH_CONTROL_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_FLUSH_CONTROL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_FLUSH_CONTROL_FLUSH
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_FLUSH_CONTROL_FLUSH_FIELD =
{
    "FLUSH",
#if RU_INCLUDE_DESC
    "",
    "Flush enable bit to drop out all packets in Tx FIFO without egressing any packets when set.",
#endif
    UNIMAC_RDP_FLUSH_CONTROL_FLUSH_FIELD_MASK,
    0,
    UNIMAC_RDP_FLUSH_CONTROL_FLUSH_FIELD_WIDTH,
    UNIMAC_RDP_FLUSH_CONTROL_FLUSH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_RXFIFO_STAT_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_RXFIFO_STAT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_RXFIFO_STAT_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_RXFIFO_STAT_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_RXFIFO_STAT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_RXFIFO_STAT_RXFIFO_OVERRUN
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_RXFIFO_STAT_RXFIFO_OVERRUN_FIELD =
{
    "RXFIFO_OVERRUN",
#if RU_INCLUDE_DESC
    "",
    "RXFIFO Overrun occurred.",
#endif
    UNIMAC_RDP_RXFIFO_STAT_RXFIFO_OVERRUN_FIELD_MASK,
    0,
    UNIMAC_RDP_RXFIFO_STAT_RXFIFO_OVERRUN_FIELD_WIDTH,
    UNIMAC_RDP_RXFIFO_STAT_RXFIFO_OVERRUN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_RXFIFO_STAT_RXFIFO_UNDERRUN
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_RXFIFO_STAT_RXFIFO_UNDERRUN_FIELD =
{
    "RXFIFO_UNDERRUN",
#if RU_INCLUDE_DESC
    "",
    "RXFIFO Underrun occurred.",
#endif
    UNIMAC_RDP_RXFIFO_STAT_RXFIFO_UNDERRUN_FIELD_MASK,
    0,
    UNIMAC_RDP_RXFIFO_STAT_RXFIFO_UNDERRUN_FIELD_WIDTH,
    UNIMAC_RDP_RXFIFO_STAT_RXFIFO_UNDERRUN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_TXFIFO_STAT_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_TXFIFO_STAT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_TXFIFO_STAT_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_TXFIFO_STAT_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_TXFIFO_STAT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_TXFIFO_STAT_TXFIFO_OVERRUN
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_TXFIFO_STAT_TXFIFO_OVERRUN_FIELD =
{
    "TXFIFO_OVERRUN",
#if RU_INCLUDE_DESC
    "",
    "TXFIFO Overrun occurred.",
#endif
    UNIMAC_RDP_TXFIFO_STAT_TXFIFO_OVERRUN_FIELD_MASK,
    0,
    UNIMAC_RDP_TXFIFO_STAT_TXFIFO_OVERRUN_FIELD_WIDTH,
    UNIMAC_RDP_TXFIFO_STAT_TXFIFO_OVERRUN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_TXFIFO_STAT_TXFIFO_UNDERRUN
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_TXFIFO_STAT_TXFIFO_UNDERRUN_FIELD =
{
    "TXFIFO_UNDERRUN",
#if RU_INCLUDE_DESC
    "",
    "TXFIFO Underrun occurred.",
#endif
    UNIMAC_RDP_TXFIFO_STAT_TXFIFO_UNDERRUN_FIELD_MASK,
    0,
    UNIMAC_RDP_TXFIFO_STAT_TXFIFO_UNDERRUN_FIELD_WIDTH,
    UNIMAC_RDP_TXFIFO_STAT_TXFIFO_UNDERRUN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MAC_PFC_CTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MAC_PFC_CTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_MAC_PFC_CTRL_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_MAC_PFC_CTRL_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_MAC_PFC_CTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MAC_PFC_CTRL_PFC_STATS_EN
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MAC_PFC_CTRL_PFC_STATS_EN_FIELD =
{
    "PFC_STATS_EN",
#if RU_INCLUDE_DESC
    "",
    "When clear, none of PFC related counters should increment. \n"
    "Otherwise, PFC counters is in full function. \n"
    "Note: it is programming requirement to set this bit when PFC function is enable.",
#endif
    UNIMAC_RDP_MAC_PFC_CTRL_PFC_STATS_EN_FIELD_MASK,
    0,
    UNIMAC_RDP_MAC_PFC_CTRL_PFC_STATS_EN_FIELD_WIDTH,
    UNIMAC_RDP_MAC_PFC_CTRL_PFC_STATS_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MAC_PFC_CTRL_RX_PASS_PFC_FRM
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MAC_PFC_CTRL_RX_PASS_PFC_FRM_FIELD =
{
    "RX_PASS_PFC_FRM",
#if RU_INCLUDE_DESC
    "",
    "When set, MAC pass PFC frame to the system. Otherwise, PFC frame is discarded.",
#endif
    UNIMAC_RDP_MAC_PFC_CTRL_RX_PASS_PFC_FRM_FIELD_MASK,
    0,
    UNIMAC_RDP_MAC_PFC_CTRL_RX_PASS_PFC_FRM_FIELD_WIDTH,
    UNIMAC_RDP_MAC_PFC_CTRL_RX_PASS_PFC_FRM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MAC_PFC_CTRL_RESERVED1
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MAC_PFC_CTRL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_MAC_PFC_CTRL_RESERVED1_FIELD_MASK,
    0,
    UNIMAC_RDP_MAC_PFC_CTRL_RESERVED1_FIELD_WIDTH,
    UNIMAC_RDP_MAC_PFC_CTRL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MAC_PFC_CTRL_FORCE_PFC_XON
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MAC_PFC_CTRL_FORCE_PFC_XON_FIELD =
{
    "FORCE_PFC_XON",
#if RU_INCLUDE_DESC
    "",
    "Instructs MAC to send Xon message to all classes of service.",
#endif
    UNIMAC_RDP_MAC_PFC_CTRL_FORCE_PFC_XON_FIELD_MASK,
    0,
    UNIMAC_RDP_MAC_PFC_CTRL_FORCE_PFC_XON_FIELD_WIDTH,
    UNIMAC_RDP_MAC_PFC_CTRL_FORCE_PFC_XON_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MAC_PFC_CTRL_PFC_RX_ENBL
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MAC_PFC_CTRL_PFC_RX_ENBL_FIELD =
{
    "PFC_RX_ENBL",
#if RU_INCLUDE_DESC
    "",
    "Enables the PFC-Rx functionality.",
#endif
    UNIMAC_RDP_MAC_PFC_CTRL_PFC_RX_ENBL_FIELD_MASK,
    0,
    UNIMAC_RDP_MAC_PFC_CTRL_PFC_RX_ENBL_FIELD_WIDTH,
    UNIMAC_RDP_MAC_PFC_CTRL_PFC_RX_ENBL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MAC_PFC_CTRL_PFC_TX_ENBL
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MAC_PFC_CTRL_PFC_TX_ENBL_FIELD =
{
    "PFC_TX_ENBL",
#if RU_INCLUDE_DESC
    "",
    "Enables the PFC-Tx functionality.",
#endif
    UNIMAC_RDP_MAC_PFC_CTRL_PFC_TX_ENBL_FIELD_MASK,
    0,
    UNIMAC_RDP_MAC_PFC_CTRL_PFC_TX_ENBL_FIELD_WIDTH,
    UNIMAC_RDP_MAC_PFC_CTRL_PFC_TX_ENBL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MAC_PFC_REFRESH_CTRL_PFC_REFRESH_TIMER
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MAC_PFC_REFRESH_CTRL_PFC_REFRESH_TIMER_FIELD =
{
    "PFC_REFRESH_TIMER",
#if RU_INCLUDE_DESC
    "",
    "PFC refresh counter value.",
#endif
    UNIMAC_RDP_MAC_PFC_REFRESH_CTRL_PFC_REFRESH_TIMER_FIELD_MASK,
    0,
    UNIMAC_RDP_MAC_PFC_REFRESH_CTRL_PFC_REFRESH_TIMER_FIELD_WIDTH,
    UNIMAC_RDP_MAC_PFC_REFRESH_CTRL_PFC_REFRESH_TIMER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MAC_PFC_REFRESH_CTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MAC_PFC_REFRESH_CTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_MAC_PFC_REFRESH_CTRL_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_MAC_PFC_REFRESH_CTRL_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_MAC_PFC_REFRESH_CTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MAC_PFC_REFRESH_CTRL_PFC_REFRESH_EN
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MAC_PFC_REFRESH_CTRL_PFC_REFRESH_EN_FIELD =
{
    "PFC_REFRESH_EN",
#if RU_INCLUDE_DESC
    "",
    "Enables the PFC refresh functionality on the Tx side. When enabled, the MAC sends Xoff message on refresh counter becoming 0",
#endif
    UNIMAC_RDP_MAC_PFC_REFRESH_CTRL_PFC_REFRESH_EN_FIELD_MASK,
    0,
    UNIMAC_RDP_MAC_PFC_REFRESH_CTRL_PFC_REFRESH_EN_FIELD_WIDTH,
    UNIMAC_RDP_MAC_PFC_REFRESH_CTRL_PFC_REFRESH_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: UNIMAC_RDP_IPG_HD_BKP_CNTL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_IPG_HD_BKP_CNTL_FIELDS[] =
{
    &UNIMAC_RDP_IPG_HD_BKP_CNTL_RESERVED0_FIELD,
    &UNIMAC_RDP_IPG_HD_BKP_CNTL_IPG_CONFIG_RX_FIELD,
    &UNIMAC_RDP_IPG_HD_BKP_CNTL_HD_FC_BKOFF_OK_FIELD,
    &UNIMAC_RDP_IPG_HD_BKP_CNTL_HD_FC_ENA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    845,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    UNIMAC_RDP_IPG_HD_BKP_CNTL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_COMMAND_CONFIG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_COMMAND_CONFIG_FIELDS[] =
{
    &UNIMAC_RDP_COMMAND_CONFIG_RESERVED0_FIELD,
    &UNIMAC_RDP_COMMAND_CONFIG_RUNT_FILTER_DIS_FIELD,
    &UNIMAC_RDP_COMMAND_CONFIG_OOB_EFC_EN_FIELD,
    &UNIMAC_RDP_COMMAND_CONFIG_IGNORE_TX_PAUSE_FIELD,
    &UNIMAC_RDP_COMMAND_CONFIG_FD_TX_URUN_FIX_EN_FIELD,
    &UNIMAC_RDP_COMMAND_CONFIG_RESERVED1_FIELD,
    &UNIMAC_RDP_COMMAND_CONFIG_LINE_LOOPBACK_FIELD,
    &UNIMAC_RDP_COMMAND_CONFIG_NO_LGTH_CHECK_FIELD,
    &UNIMAC_RDP_COMMAND_CONFIG_CNTL_FRM_ENA_FIELD,
    &UNIMAC_RDP_COMMAND_CONFIG_ENA_EXT_CONFIG_FIELD,
    &UNIMAC_RDP_COMMAND_CONFIG_EN_INTERNAL_TX_CRS_FIELD,
    &UNIMAC_RDP_COMMAND_CONFIG_RESERVED2_FIELD,
    &UNIMAC_RDP_COMMAND_CONFIG_SW_OVERRIDE_RX_FIELD,
    &UNIMAC_RDP_COMMAND_CONFIG_SW_OVERRIDE_TX_FIELD,
    &UNIMAC_RDP_COMMAND_CONFIG_MAC_LOOP_CON_FIELD,
    &UNIMAC_RDP_COMMAND_CONFIG_LOOP_ENA_FIELD,
    &UNIMAC_RDP_COMMAND_CONFIG_FCS_CORRUPT_URUN_EN_FIELD,
    &UNIMAC_RDP_COMMAND_CONFIG_SW_RESET_FIELD,
    &UNIMAC_RDP_COMMAND_CONFIG_OVERFLOW_EN_FIELD,
    &UNIMAC_RDP_COMMAND_CONFIG_RX_LOW_LATENCY_EN_FIELD,
    &UNIMAC_RDP_COMMAND_CONFIG_HD_ENA_FIELD,
    &UNIMAC_RDP_COMMAND_CONFIG_TX_ADDR_INS_FIELD,
    &UNIMAC_RDP_COMMAND_CONFIG_PAUSE_IGNORE_FIELD,
    &UNIMAC_RDP_COMMAND_CONFIG_PAUSE_FWD_FIELD,
    &UNIMAC_RDP_COMMAND_CONFIG_CRC_FWD_FIELD,
    &UNIMAC_RDP_COMMAND_CONFIG_PAD_EN_FIELD,
    &UNIMAC_RDP_COMMAND_CONFIG_PROMIS_EN_FIELD,
    &UNIMAC_RDP_COMMAND_CONFIG_ETH_SPEED_FIELD,
    &UNIMAC_RDP_COMMAND_CONFIG_RX_ENA_FIELD,
    &UNIMAC_RDP_COMMAND_CONFIG_TX_ENA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    846,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    30,
    UNIMAC_RDP_COMMAND_CONFIG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_MAC_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_MAC_0_FIELDS[] =
{
    &UNIMAC_RDP_MAC_0_MAC_ADDR0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    847,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_RDP_MAC_0_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_MAC_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_MAC_1_FIELDS[] =
{
    &UNIMAC_RDP_MAC_1_RESERVED0_FIELD,
    &UNIMAC_RDP_MAC_1_MAC_ADDR1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    848,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    UNIMAC_RDP_MAC_1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_FRM_LENGTH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_FRM_LENGTH_FIELDS[] =
{
    &UNIMAC_RDP_FRM_LENGTH_RESERVED0_FIELD,
    &UNIMAC_RDP_FRM_LENGTH_MAXFR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    849,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    UNIMAC_RDP_FRM_LENGTH_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_PAUSE_QUANT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_PAUSE_QUANT_FIELDS[] =
{
    &UNIMAC_RDP_PAUSE_QUANT_RESERVED0_FIELD,
    &UNIMAC_RDP_PAUSE_QUANT_PAUSE_QUANT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    850,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    UNIMAC_RDP_PAUSE_QUANT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_TX_TS_SEQ_ID
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_TX_TS_SEQ_ID_FIELDS[] =
{
    &UNIMAC_RDP_TX_TS_SEQ_ID_RESERVED0_FIELD,
    &UNIMAC_RDP_TX_TS_SEQ_ID_TSTS_VALID_FIELD,
    &UNIMAC_RDP_TX_TS_SEQ_ID_TSTS_SEQ_ID_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    851,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    UNIMAC_RDP_TX_TS_SEQ_ID_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_SFD_OFFSET
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_SFD_OFFSET_FIELDS[] =
{
    &UNIMAC_RDP_SFD_OFFSET_RESERVED0_FIELD,
    &UNIMAC_RDP_SFD_OFFSET_SFD_OFFSET_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    852,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    UNIMAC_RDP_SFD_OFFSET_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_MAC_MODE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_MAC_MODE_FIELDS[] =
{
    &UNIMAC_RDP_MAC_MODE_RESERVED0_FIELD,
    &UNIMAC_RDP_MAC_MODE_LINK_STATUS_FIELD,
    &UNIMAC_RDP_MAC_MODE_MAC_TX_PAUSE_FIELD,
    &UNIMAC_RDP_MAC_MODE_MAC_RX_PAUSE_FIELD,
    &UNIMAC_RDP_MAC_MODE_MAC_DUPLEX_FIELD,
    &UNIMAC_RDP_MAC_MODE_MAC_SPEED_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    853,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    UNIMAC_RDP_MAC_MODE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_TAG_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_TAG_0_FIELDS[] =
{
    &UNIMAC_RDP_TAG_0_RESERVED0_FIELD,
    &UNIMAC_RDP_TAG_0_CONFIG_OUTER_TPID_ENABLE_FIELD,
    &UNIMAC_RDP_TAG_0_FRM_TAG_0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    854,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    UNIMAC_RDP_TAG_0_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_TAG_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_TAG_1_FIELDS[] =
{
    &UNIMAC_RDP_TAG_1_RESERVED0_FIELD,
    &UNIMAC_RDP_TAG_1_CONFIG_INNER_TPID_ENABLE_FIELD,
    &UNIMAC_RDP_TAG_1_FRM_TAG_1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    855,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    UNIMAC_RDP_TAG_1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_RX_PAUSE_QUANTA_SCALE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_RX_PAUSE_QUANTA_SCALE_FIELDS[] =
{
    &UNIMAC_RDP_RX_PAUSE_QUANTA_SCALE_RESERVED0_FIELD,
    &UNIMAC_RDP_RX_PAUSE_QUANTA_SCALE_SCALE_FIX_FIELD,
    &UNIMAC_RDP_RX_PAUSE_QUANTA_SCALE_SCALE_CONTROL_FIELD,
    &UNIMAC_RDP_RX_PAUSE_QUANTA_SCALE_SCALE_VALUE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    856,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    UNIMAC_RDP_RX_PAUSE_QUANTA_SCALE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_TX_PREAMBLE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_TX_PREAMBLE_FIELDS[] =
{
    &UNIMAC_RDP_TX_PREAMBLE_RESERVED0_FIELD,
    &UNIMAC_RDP_TX_PREAMBLE_TX_PREAMBLE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    857,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    UNIMAC_RDP_TX_PREAMBLE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_TX_IPG_LENGTH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_TX_IPG_LENGTH_FIELDS[] =
{
    &UNIMAC_RDP_TX_IPG_LENGTH_RESERVED0_FIELD,
    &UNIMAC_RDP_TX_IPG_LENGTH_TX_MIN_PKT_SIZE_FIELD,
    &UNIMAC_RDP_TX_IPG_LENGTH_RESERVED1_FIELD,
    &UNIMAC_RDP_TX_IPG_LENGTH_TX_IPG_LENGTH_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    858,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    UNIMAC_RDP_TX_IPG_LENGTH_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_PFC_XOFF_TIMER
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_PFC_XOFF_TIMER_FIELDS[] =
{
    &UNIMAC_RDP_PFC_XOFF_TIMER_RESERVED0_FIELD,
    &UNIMAC_RDP_PFC_XOFF_TIMER_PFC_XOFF_TIMER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    859,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    UNIMAC_RDP_PFC_XOFF_TIMER_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_UMAC_EEE_CTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_UMAC_EEE_CTRL_FIELDS[] =
{
    &UNIMAC_RDP_UMAC_EEE_CTRL_RESERVED0_FIELD,
    &UNIMAC_RDP_UMAC_EEE_CTRL_LP_IDLE_PREDICTION_MODE_FIELD,
    &UNIMAC_RDP_UMAC_EEE_CTRL_DIS_EEE_10M_FIELD,
    &UNIMAC_RDP_UMAC_EEE_CTRL_EEE_TXCLK_DIS_FIELD,
    &UNIMAC_RDP_UMAC_EEE_CTRL_RX_FIFO_CHECK_FIELD,
    &UNIMAC_RDP_UMAC_EEE_CTRL_EEE_EN_FIELD,
    &UNIMAC_RDP_UMAC_EEE_CTRL_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    860,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    UNIMAC_RDP_UMAC_EEE_CTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_MII_EEE_DELAY_ENTRY_TIMER
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_MII_EEE_DELAY_ENTRY_TIMER_FIELDS[] =
{
    &UNIMAC_RDP_MII_EEE_DELAY_ENTRY_TIMER_MII_EEE_LPI_TIMER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    861,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_RDP_MII_EEE_DELAY_ENTRY_TIMER_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_GMII_EEE_DELAY_ENTRY_TIMER
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_GMII_EEE_DELAY_ENTRY_TIMER_FIELDS[] =
{
    &UNIMAC_RDP_GMII_EEE_DELAY_ENTRY_TIMER_GMII_EEE_LPI_TIMER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    862,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_RDP_GMII_EEE_DELAY_ENTRY_TIMER_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_UMAC_EEE_REF_COUNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_UMAC_EEE_REF_COUNT_FIELDS[] =
{
    &UNIMAC_RDP_UMAC_EEE_REF_COUNT_RESERVED0_FIELD,
    &UNIMAC_RDP_UMAC_EEE_REF_COUNT_EEE_REF_COUNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    863,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    UNIMAC_RDP_UMAC_EEE_REF_COUNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_UMAC_TIMESTAMP_ADJUST
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_UMAC_TIMESTAMP_ADJUST_FIELDS[] =
{
    &UNIMAC_RDP_UMAC_TIMESTAMP_ADJUST_RESERVED0_FIELD,
    &UNIMAC_RDP_UMAC_TIMESTAMP_ADJUST_AUTO_ADJUST_FIELD,
    &UNIMAC_RDP_UMAC_TIMESTAMP_ADJUST_EN_1588_FIELD,
    &UNIMAC_RDP_UMAC_TIMESTAMP_ADJUST_ADJUST_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    864,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    UNIMAC_RDP_UMAC_TIMESTAMP_ADJUST_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_UMAC_RX_PKT_DROP_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_UMAC_RX_PKT_DROP_STATUS_FIELDS[] =
{
    &UNIMAC_RDP_UMAC_RX_PKT_DROP_STATUS_RESERVED0_FIELD,
    &UNIMAC_RDP_UMAC_RX_PKT_DROP_STATUS_RX_IPG_INVAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    865,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    UNIMAC_RDP_UMAC_RX_PKT_DROP_STATUS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_UMAC_SYMMETRIC_IDLE_THRESHOLD
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_UMAC_SYMMETRIC_IDLE_THRESHOLD_FIELDS[] =
{
    &UNIMAC_RDP_UMAC_SYMMETRIC_IDLE_THRESHOLD_RESERVED0_FIELD,
    &UNIMAC_RDP_UMAC_SYMMETRIC_IDLE_THRESHOLD_THRESHOLD_VALUE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    866,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    UNIMAC_RDP_UMAC_SYMMETRIC_IDLE_THRESHOLD_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_MII_EEE_WAKE_TIMER
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_MII_EEE_WAKE_TIMER_FIELDS[] =
{
    &UNIMAC_RDP_MII_EEE_WAKE_TIMER_RESERVED0_FIELD,
    &UNIMAC_RDP_MII_EEE_WAKE_TIMER_MII_EEE_WAKE_TIMER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    867,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    UNIMAC_RDP_MII_EEE_WAKE_TIMER_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_GMII_EEE_WAKE_TIMER
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_GMII_EEE_WAKE_TIMER_FIELDS[] =
{
    &UNIMAC_RDP_GMII_EEE_WAKE_TIMER_RESERVED0_FIELD,
    &UNIMAC_RDP_GMII_EEE_WAKE_TIMER_GMII_EEE_WAKE_TIMER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    868,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    UNIMAC_RDP_GMII_EEE_WAKE_TIMER_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_UMAC_REV_ID
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_UMAC_REV_ID_FIELDS[] =
{
    &UNIMAC_RDP_UMAC_REV_ID_RESERVED0_FIELD,
    &UNIMAC_RDP_UMAC_REV_ID_REVISION_ID_MAJOR_FIELD,
    &UNIMAC_RDP_UMAC_REV_ID_REVISION_ID_MINOR_FIELD,
    &UNIMAC_RDP_UMAC_REV_ID_PATCH_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    869,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    UNIMAC_RDP_UMAC_REV_ID_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_MAC_PFC_TYPE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_MAC_PFC_TYPE_FIELDS[] =
{
    &UNIMAC_RDP_MAC_PFC_TYPE_RESERVED0_FIELD,
    &UNIMAC_RDP_MAC_PFC_TYPE_PFC_ETH_TYPE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    870,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    UNIMAC_RDP_MAC_PFC_TYPE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_MAC_PFC_OPCODE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_MAC_PFC_OPCODE_FIELDS[] =
{
    &UNIMAC_RDP_MAC_PFC_OPCODE_RESERVED0_FIELD,
    &UNIMAC_RDP_MAC_PFC_OPCODE_PFC_OPCODE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    871,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    UNIMAC_RDP_MAC_PFC_OPCODE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_MAC_PFC_DA_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_MAC_PFC_DA_0_FIELDS[] =
{
    &UNIMAC_RDP_MAC_PFC_DA_0_PFC_MACDA_0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    872,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_RDP_MAC_PFC_DA_0_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_MAC_PFC_DA_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_MAC_PFC_DA_1_FIELDS[] =
{
    &UNIMAC_RDP_MAC_PFC_DA_1_RESERVED0_FIELD,
    &UNIMAC_RDP_MAC_PFC_DA_1_PFC_MACDA_1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    873,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    UNIMAC_RDP_MAC_PFC_DA_1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_MACSEC_PROG_TX_CRC
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_MACSEC_PROG_TX_CRC_FIELDS[] =
{
    &UNIMAC_RDP_MACSEC_PROG_TX_CRC_MACSEC_PROG_TX_CRC_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    874,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_RDP_MACSEC_PROG_TX_CRC_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_MACSEC_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_MACSEC_CNTRL_FIELDS[] =
{
    &UNIMAC_RDP_MACSEC_CNTRL_RESERVED0_FIELD,
    &UNIMAC_RDP_MACSEC_CNTRL_DIS_PAUSE_DATA_VAR_IPG_FIELD,
    &UNIMAC_RDP_MACSEC_CNTRL_TX_CRC_PROGRAM_FIELD,
    &UNIMAC_RDP_MACSEC_CNTRL_TX_CRC_CORUPT_EN_FIELD,
    &UNIMAC_RDP_MACSEC_CNTRL_TX_LAUNCH_EN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    875,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    UNIMAC_RDP_MACSEC_CNTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_TS_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_TS_STATUS_FIELDS[] =
{
    &UNIMAC_RDP_TS_STATUS_RESERVED0_FIELD,
    &UNIMAC_RDP_TS_STATUS_WORD_AVAIL_FIELD,
    &UNIMAC_RDP_TS_STATUS_TX_TS_FIFO_EMPTY_FIELD,
    &UNIMAC_RDP_TS_STATUS_TX_TS_FIFO_FULL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    876,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    UNIMAC_RDP_TS_STATUS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_TX_TS_DATA
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_TX_TS_DATA_FIELDS[] =
{
    &UNIMAC_RDP_TX_TS_DATA_TX_TS_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    877,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_RDP_TX_TS_DATA_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_PAUSE_REFRESH_CTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_PAUSE_REFRESH_CTRL_FIELDS[] =
{
    &UNIMAC_RDP_PAUSE_REFRESH_CTRL_RESERVED0_FIELD,
    &UNIMAC_RDP_PAUSE_REFRESH_CTRL_ENABLE_FIELD,
    &UNIMAC_RDP_PAUSE_REFRESH_CTRL_REFRESH_TIMER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    878,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    UNIMAC_RDP_PAUSE_REFRESH_CTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_FLUSH_CONTROL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_FLUSH_CONTROL_FIELDS[] =
{
    &UNIMAC_RDP_FLUSH_CONTROL_RESERVED0_FIELD,
    &UNIMAC_RDP_FLUSH_CONTROL_FLUSH_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    879,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    UNIMAC_RDP_FLUSH_CONTROL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_RXFIFO_STAT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_RXFIFO_STAT_FIELDS[] =
{
    &UNIMAC_RDP_RXFIFO_STAT_RESERVED0_FIELD,
    &UNIMAC_RDP_RXFIFO_STAT_RXFIFO_OVERRUN_FIELD,
    &UNIMAC_RDP_RXFIFO_STAT_RXFIFO_UNDERRUN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    880,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    UNIMAC_RDP_RXFIFO_STAT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_TXFIFO_STAT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_TXFIFO_STAT_FIELDS[] =
{
    &UNIMAC_RDP_TXFIFO_STAT_RESERVED0_FIELD,
    &UNIMAC_RDP_TXFIFO_STAT_TXFIFO_OVERRUN_FIELD,
    &UNIMAC_RDP_TXFIFO_STAT_TXFIFO_UNDERRUN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    881,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    UNIMAC_RDP_TXFIFO_STAT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_MAC_PFC_CTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_MAC_PFC_CTRL_FIELDS[] =
{
    &UNIMAC_RDP_MAC_PFC_CTRL_RESERVED0_FIELD,
    &UNIMAC_RDP_MAC_PFC_CTRL_PFC_STATS_EN_FIELD,
    &UNIMAC_RDP_MAC_PFC_CTRL_RX_PASS_PFC_FRM_FIELD,
    &UNIMAC_RDP_MAC_PFC_CTRL_RESERVED1_FIELD,
    &UNIMAC_RDP_MAC_PFC_CTRL_FORCE_PFC_XON_FIELD,
    &UNIMAC_RDP_MAC_PFC_CTRL_PFC_RX_ENBL_FIELD,
    &UNIMAC_RDP_MAC_PFC_CTRL_PFC_TX_ENBL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    882,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    UNIMAC_RDP_MAC_PFC_CTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_MAC_PFC_REFRESH_CTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_MAC_PFC_REFRESH_CTRL_FIELDS[] =
{
    &UNIMAC_RDP_MAC_PFC_REFRESH_CTRL_PFC_REFRESH_TIMER_FIELD,
    &UNIMAC_RDP_MAC_PFC_REFRESH_CTRL_RESERVED0_FIELD,
    &UNIMAC_RDP_MAC_PFC_REFRESH_CTRL_PFC_REFRESH_EN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

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
    883,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    UNIMAC_RDP_MAC_PFC_REFRESH_CTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
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
