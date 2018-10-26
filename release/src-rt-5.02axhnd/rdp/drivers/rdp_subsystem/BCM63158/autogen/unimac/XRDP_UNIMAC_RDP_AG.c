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
 * Field: UNIMAC_RDP_UMAC_DUMMY_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_UMAC_DUMMY_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_UMAC_DUMMY_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_UMAC_DUMMY_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_UMAC_DUMMY_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_UMAC_DUMMY_UMAC_DUMMY
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_UMAC_DUMMY_UMAC_DUMMY_FIELD =
{
    "UMAC_DUMMY",
#if RU_INCLUDE_DESC
    "",
    "A dummy read only register.",
#endif
    UNIMAC_RDP_UMAC_DUMMY_UMAC_DUMMY_FIELD_MASK,
    0,
    UNIMAC_RDP_UMAC_DUMMY_UMAC_DUMMY_FIELD_WIDTH,
    UNIMAC_RDP_UMAC_DUMMY_UMAC_DUMMY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_HD_BKP_CNTL_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_HD_BKP_CNTL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_HD_BKP_CNTL_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_HD_BKP_CNTL_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_HD_BKP_CNTL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_HD_BKP_CNTL_IPG_CONFIG_RX
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_HD_BKP_CNTL_IPG_CONFIG_RX_FIELD =
{
    "IPG_CONFIG_RX",
#if RU_INCLUDE_DESC
    "",
    "The programmable value in bytes in case of 1/2.5G and in nibbles in case of 10/100M on the RX side"
    "indicates that the packets received with IPG less than the programmed value are dropped."
    "This value must be varied between 4B and 11B (8 and 22 in case of Nibbles), through the register"
    "can take other values in the range of 0 to 31. The default value is 5 in case of 1G/2.5G and 10 in"
    "case of 10M/100M, indicating that"
    "(a) the packets that have IPG greater than 4B, must be passed."
    "(b) the packets that have IPG less than or equal to 4B must be dropped.",
#endif
    UNIMAC_RDP_HD_BKP_CNTL_IPG_CONFIG_RX_FIELD_MASK,
    0,
    UNIMAC_RDP_HD_BKP_CNTL_IPG_CONFIG_RX_FIELD_WIDTH,
    UNIMAC_RDP_HD_BKP_CNTL_IPG_CONFIG_RX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_HD_BKP_CNTL_HD_FC_BKOFF_OK
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_HD_BKP_CNTL_HD_FC_BKOFF_OK_FIELD =
{
    "HD_FC_BKOFF_OK",
#if RU_INCLUDE_DESC
    "",
    "1: Enables exponential back-off algorithm for half-duplex back-pressure mechanism."
    "0: TBD",
#endif
    UNIMAC_RDP_HD_BKP_CNTL_HD_FC_BKOFF_OK_FIELD_MASK,
    0,
    UNIMAC_RDP_HD_BKP_CNTL_HD_FC_BKOFF_OK_FIELD_WIDTH,
    UNIMAC_RDP_HD_BKP_CNTL_HD_FC_BKOFF_OK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_HD_BKP_CNTL_HD_FC_ENA
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_HD_BKP_CNTL_HD_FC_ENA_FIELD =
{
    "HD_FC_ENA",
#if RU_INCLUDE_DESC
    "",
    "1: Enables the half-duplex flow control, so the MAC hogs the line with preamble."
    "0: Disabled.",
#endif
    UNIMAC_RDP_HD_BKP_CNTL_HD_FC_ENA_FIELD_MASK,
    0,
    UNIMAC_RDP_HD_BKP_CNTL_HD_FC_ENA_FIELD_WIDTH,
    UNIMAC_RDP_HD_BKP_CNTL_HD_FC_ENA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_CMD_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_CMD_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_CMD_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_CMD_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_CMD_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_CMD_RUNT_FILTER_DIS
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_CMD_RUNT_FILTER_DIS_FIELD =
{
    "RUNT_FILTER_DIS",
#if RU_INCLUDE_DESC
    "",
    "1: Disable RX side RUNT filtering."
    "0: Enable  RUNT filtering.",
#endif
    UNIMAC_RDP_CMD_RUNT_FILTER_DIS_FIELD_MASK,
    0,
    UNIMAC_RDP_CMD_RUNT_FILTER_DIS_FIELD_WIDTH,
    UNIMAC_RDP_CMD_RUNT_FILTER_DIS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_CMD_TXRX_EN_CONFIG
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_CMD_TXRX_EN_CONFIG_FIELD =
{
    "TXRX_EN_CONFIG",
#if RU_INCLUDE_DESC
    "",
    "This mode only works in auto-config mode:"
    "0: After auto-config, TX_ENA and RX_ENA bits are set to 1."
    "1: After auto-config, TX_ENA and RX_ENA bits are set to 0,"
    "meaning SW will have to come in and enable TX and RX.",
#endif
    UNIMAC_RDP_CMD_TXRX_EN_CONFIG_FIELD_MASK,
    0,
    UNIMAC_RDP_CMD_TXRX_EN_CONFIG_FIELD_WIDTH,
    UNIMAC_RDP_CMD_TXRX_EN_CONFIG_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_CMD_TX_PAUSE_IGNORE
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_CMD_TX_PAUSE_IGNORE_FIELD =
{
    "TX_PAUSE_IGNORE",
#if RU_INCLUDE_DESC
    "",
    "Ignore TX PAUSE frame transmit request.",
#endif
    UNIMAC_RDP_CMD_TX_PAUSE_IGNORE_FIELD_MASK,
    0,
    UNIMAC_RDP_CMD_TX_PAUSE_IGNORE_FIELD_WIDTH,
    UNIMAC_RDP_CMD_TX_PAUSE_IGNORE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_CMD_PRBL_ENA
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_CMD_PRBL_ENA_FIELD =
{
    "PRBL_ENA",
#if RU_INCLUDE_DESC
    "",
    "1: Enable extract/insert of EFM headers.",
#endif
    UNIMAC_RDP_CMD_PRBL_ENA_FIELD_MASK,
    0,
    UNIMAC_RDP_CMD_PRBL_ENA_FIELD_WIDTH,
    UNIMAC_RDP_CMD_PRBL_ENA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_CMD_RX_ERR_DISC
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_CMD_RX_ERR_DISC_FIELD =
{
    "RX_ERR_DISC",
#if RU_INCLUDE_DESC
    "",
    "This bit currently not used.",
#endif
    UNIMAC_RDP_CMD_RX_ERR_DISC_FIELD_MASK,
    0,
    UNIMAC_RDP_CMD_RX_ERR_DISC_FIELD_WIDTH,
    UNIMAC_RDP_CMD_RX_ERR_DISC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_CMD_RMT_LOOP_ENA
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_CMD_RMT_LOOP_ENA_FIELD =
{
    "RMT_LOOP_ENA",
#if RU_INCLUDE_DESC
    "",
    "1: Enable remote loopback at the fifo system side."
    "0: Normal operation.",
#endif
    UNIMAC_RDP_CMD_RMT_LOOP_ENA_FIELD_MASK,
    0,
    UNIMAC_RDP_CMD_RMT_LOOP_ENA_FIELD_WIDTH,
    UNIMAC_RDP_CMD_RMT_LOOP_ENA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_CMD_NO_LGTH_CHECK
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_CMD_NO_LGTH_CHECK_FIELD =
{
    "NO_LGTH_CHECK",
#if RU_INCLUDE_DESC
    "",
    "Payload length check."
    "0: Check payload length with Length/Type field."
    "1: Check disabled.",
#endif
    UNIMAC_RDP_CMD_NO_LGTH_CHECK_FIELD_MASK,
    0,
    UNIMAC_RDP_CMD_NO_LGTH_CHECK_FIELD_WIDTH,
    UNIMAC_RDP_CMD_NO_LGTH_CHECK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_CMD_CNTL_FRM_ENA
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_CMD_CNTL_FRM_ENA_FIELD =
{
    "CNTL_FRM_ENA",
#if RU_INCLUDE_DESC
    "",
    "MAC control frame enable."
    "1: MAC control frames with opcode other than 0x0001 are accepted and forwarded to the client interface."
    "0: MAC control frames with opcode other than 0x0000 and 0x0001 are silently discarded.",
#endif
    UNIMAC_RDP_CMD_CNTL_FRM_ENA_FIELD_MASK,
    0,
    UNIMAC_RDP_CMD_CNTL_FRM_ENA_FIELD_WIDTH,
    UNIMAC_RDP_CMD_CNTL_FRM_ENA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_CMD_ENA_EXT_CONFIG
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_CMD_ENA_EXT_CONFIG_FIELD =
{
    "ENA_EXT_CONFIG",
#if RU_INCLUDE_DESC
    "",
    "Enable/Disable auto-configuration."
    "1: Enable "
    "0: Disable ",
#endif
    UNIMAC_RDP_CMD_ENA_EXT_CONFIG_FIELD_MASK,
    0,
    UNIMAC_RDP_CMD_ENA_EXT_CONFIG_FIELD_WIDTH,
    UNIMAC_RDP_CMD_ENA_EXT_CONFIG_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_CMD_RESERVED1
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_CMD_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_CMD_RESERVED1_FIELD_MASK,
    0,
    UNIMAC_RDP_CMD_RESERVED1_FIELD_WIDTH,
    UNIMAC_RDP_CMD_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_CMD_LCL_LOOP_ENA
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_CMD_LCL_LOOP_ENA_FIELD =
{
    "LCL_LOOP_ENA",
#if RU_INCLUDE_DESC
    "",
    "Enable GMII/MII loopback"
    "1: Loopback enabled."
    "0: Normal operation.",
#endif
    UNIMAC_RDP_CMD_LCL_LOOP_ENA_FIELD_MASK,
    0,
    UNIMAC_RDP_CMD_LCL_LOOP_ENA_FIELD_WIDTH,
    UNIMAC_RDP_CMD_LCL_LOOP_ENA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_CMD_RESERVED2
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_CMD_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_CMD_RESERVED2_FIELD_MASK,
    0,
    UNIMAC_RDP_CMD_RESERVED2_FIELD_WIDTH,
    UNIMAC_RDP_CMD_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_CMD_SW_RESET
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_CMD_SW_RESET_FIELD =
{
    "SW_RESET",
#if RU_INCLUDE_DESC
    "",
    "1: RX and RX engines are put in reset."
    "0: come out of SW reset.",
#endif
    UNIMAC_RDP_CMD_SW_RESET_FIELD_MASK,
    0,
    UNIMAC_RDP_CMD_SW_RESET_FIELD_WIDTH,
    UNIMAC_RDP_CMD_SW_RESET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_CMD_RESERVED3
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_CMD_RESERVED3_FIELD =
{
    "RESERVED3",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_CMD_RESERVED3_FIELD_MASK,
    0,
    UNIMAC_RDP_CMD_RESERVED3_FIELD_WIDTH,
    UNIMAC_RDP_CMD_RESERVED3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_CMD_HD_ENA
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_CMD_HD_ENA_FIELD =
{
    "HD_ENA",
#if RU_INCLUDE_DESC
    "",
    "Ignored when RTH_SPEED[1]==1, gigabit mode."
    "1: half duplex"
    "0: full duplex",
#endif
    UNIMAC_RDP_CMD_HD_ENA_FIELD_MASK,
    0,
    UNIMAC_RDP_CMD_HD_ENA_FIELD_WIDTH,
    UNIMAC_RDP_CMD_HD_ENA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_CMD_TX_ADDR_INS
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_CMD_TX_ADDR_INS_FIELD =
{
    "TX_ADDR_INS",
#if RU_INCLUDE_DESC
    "",
    "1: The MAC overwrites the source MAC address with a programmed MAC address in register MAC_0 and MAC_1."
    "0: Not modify the source address received from the transmit application client.",
#endif
    UNIMAC_RDP_CMD_TX_ADDR_INS_FIELD_MASK,
    0,
    UNIMAC_RDP_CMD_TX_ADDR_INS_FIELD_WIDTH,
    UNIMAC_RDP_CMD_TX_ADDR_INS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_CMD_RX_PAUSE_IGNORE
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_CMD_RX_PAUSE_IGNORE_FIELD =
{
    "RX_PAUSE_IGNORE",
#if RU_INCLUDE_DESC
    "",
    "1: Receive PAUSE frames are ignored by the MAC."
    "0: The tramsmit process is stiooed for the amount of time specified in the pause wuanta received within the PAUSE frame.",
#endif
    UNIMAC_RDP_CMD_RX_PAUSE_IGNORE_FIELD_MASK,
    0,
    UNIMAC_RDP_CMD_RX_PAUSE_IGNORE_FIELD_WIDTH,
    UNIMAC_RDP_CMD_RX_PAUSE_IGNORE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_CMD_PAUSE_FWD
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_CMD_PAUSE_FWD_FIELD =
{
    "PAUSE_FWD",
#if RU_INCLUDE_DESC
    "",
    "1: PAUSE frames are forwarded to the user application."
    "0: The PAUSE frames are terminated and discarded in the MAC.",
#endif
    UNIMAC_RDP_CMD_PAUSE_FWD_FIELD_MASK,
    0,
    UNIMAC_RDP_CMD_PAUSE_FWD_FIELD_WIDTH,
    UNIMAC_RDP_CMD_PAUSE_FWD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_CMD_CRC_FWD
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_CMD_CRC_FWD_FIELD =
{
    "CRC_FWD",
#if RU_INCLUDE_DESC
    "",
    "1: The CRC field of received frames is transmitted to the user application."
    "0: The CRC field is stripped from the frame.",
#endif
    UNIMAC_RDP_CMD_CRC_FWD_FIELD_MASK,
    0,
    UNIMAC_RDP_CMD_CRC_FWD_FIELD_WIDTH,
    UNIMAC_RDP_CMD_CRC_FWD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_CMD_PAD_EN
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_CMD_PAD_EN_FIELD =
{
    "PAD_EN",
#if RU_INCLUDE_DESC
    "",
    "1: Padding is removed along with crc field before the frame is sent to the user application."
    "0: No padding is removed by the MAC.",
#endif
    UNIMAC_RDP_CMD_PAD_EN_FIELD_MASK,
    0,
    UNIMAC_RDP_CMD_PAD_EN_FIELD_WIDTH,
    UNIMAC_RDP_CMD_PAD_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_CMD_PROMIS_EN
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_CMD_PROMIS_EN_FIELD =
{
    "PROMIS_EN",
#if RU_INCLUDE_DESC
    "",
    "1: All frames are received without Unicast address filtering."
    "0: ",
#endif
    UNIMAC_RDP_CMD_PROMIS_EN_FIELD_MASK,
    0,
    UNIMAC_RDP_CMD_PROMIS_EN_FIELD_WIDTH,
    UNIMAC_RDP_CMD_PROMIS_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_CMD_ETH_SPEED
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_CMD_ETH_SPEED_FIELD =
{
    "ETH_SPEED",
#if RU_INCLUDE_DESC
    "",
    "00: 10Mbps, 01: 100Mbps, 10: 1000Mbps, 11: 2500Mbps",
#endif
    UNIMAC_RDP_CMD_ETH_SPEED_FIELD_MASK,
    0,
    UNIMAC_RDP_CMD_ETH_SPEED_FIELD_WIDTH,
    UNIMAC_RDP_CMD_ETH_SPEED_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_CMD_RX_ENA
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_CMD_RX_ENA_FIELD =
{
    "RX_ENA",
#if RU_INCLUDE_DESC
    "",
    "1: The MAC receive function is enabled."
    "0: The MAC receive function is disabled."
    "The enable works on packet boundary meaning that only on the assertion on the bit during every 0->1 transition of rx_dv.",
#endif
    UNIMAC_RDP_CMD_RX_ENA_FIELD_MASK,
    0,
    UNIMAC_RDP_CMD_RX_ENA_FIELD_WIDTH,
    UNIMAC_RDP_CMD_RX_ENA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_CMD_TX_ENA
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_CMD_TX_ENA_FIELD =
{
    "TX_ENA",
#if RU_INCLUDE_DESC
    "",
    "1: The MAC transmit function is enabled."
    "0: The MAC transmit function is disabled."
    "The enable works on packet boundary meaning that only on the assertion of the bit during every SOP.",
#endif
    UNIMAC_RDP_CMD_TX_ENA_FIELD_MASK,
    0,
    UNIMAC_RDP_CMD_TX_ENA_FIELD_WIDTH,
    UNIMAC_RDP_CMD_TX_ENA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MAC0_MAC_0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MAC0_MAC_0_FIELD =
{
    "MAC_0",
#if RU_INCLUDE_DESC
    "",
    "SA[47:16]",
#endif
    UNIMAC_RDP_MAC0_MAC_0_FIELD_MASK,
    0,
    UNIMAC_RDP_MAC0_MAC_0_FIELD_WIDTH,
    UNIMAC_RDP_MAC0_MAC_0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MAC1_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MAC1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_MAC1_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_MAC1_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_MAC1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MAC1_MAC_1
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MAC1_MAC_1_FIELD =
{
    "MAC_1",
#if RU_INCLUDE_DESC
    "",
    "SA[15:0]",
#endif
    UNIMAC_RDP_MAC1_MAC_1_FIELD_MASK,
    0,
    UNIMAC_RDP_MAC1_MAC_1_FIELD_WIDTH,
    UNIMAC_RDP_MAC1_MAC_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_FRM_LEN_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_FRM_LEN_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_FRM_LEN_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_FRM_LEN_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_FRM_LEN_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_FRM_LEN_FRAME_LENGTH
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_FRM_LEN_FRAME_LENGTH_FIELD =
{
    "FRAME_LENGTH",
#if RU_INCLUDE_DESC
    "",
    "Max. frame length"
    "Also used on the TX side as the upper threshold to determine the jumbo frame,"
    "(length > 1518 and length <= the config. frame_length).",
#endif
    UNIMAC_RDP_FRM_LEN_FRAME_LENGTH_FIELD_MASK,
    0,
    UNIMAC_RDP_FRM_LEN_FRAME_LENGTH_FIELD_WIDTH,
    UNIMAC_RDP_FRM_LEN_FRAME_LENGTH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_PAUSE_QUNAT_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_PAUSE_QUNAT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_PAUSE_QUNAT_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_PAUSE_QUNAT_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_PAUSE_QUNAT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_PAUSE_QUNAT_PAUSE_QUANT
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_PAUSE_QUNAT_PAUSE_QUANT_FIELD =
{
    "PAUSE_QUANT",
#if RU_INCLUDE_DESC
    "",
    "Value of ethernet bit time, the pause quanta used in each PAUSE frame sent to the remote Ethernet device.",
#endif
    UNIMAC_RDP_PAUSE_QUNAT_PAUSE_QUANT_FIELD_MASK,
    0,
    UNIMAC_RDP_PAUSE_QUNAT_PAUSE_QUANT_FIELD_WIDTH,
    UNIMAC_RDP_PAUSE_QUNAT_PAUSE_QUANT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_SFD_OFFSET_TEMP
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_SFD_OFFSET_TEMP_FIELD =
{
    "TEMP",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_SFD_OFFSET_TEMP_FIELD_MASK,
    0,
    UNIMAC_RDP_SFD_OFFSET_TEMP_FIELD_WIDTH,
    UNIMAC_RDP_SFD_OFFSET_TEMP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MODE_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MODE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_MODE_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_MODE_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_MODE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MODE_MAC_LINK_STAT
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MODE_MAC_LINK_STAT_FIELD =
{
    "MAC_LINK_STAT",
#if RU_INCLUDE_DESC
    "",
    "Link status indication.",
#endif
    UNIMAC_RDP_MODE_MAC_LINK_STAT_FIELD_MASK,
    0,
    UNIMAC_RDP_MODE_MAC_LINK_STAT_FIELD_WIDTH,
    UNIMAC_RDP_MODE_MAC_LINK_STAT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MODE_MAC_TX_PAUSE
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MODE_MAC_TX_PAUSE_FIELD =
{
    "MAC_TX_PAUSE",
#if RU_INCLUDE_DESC
    "",
    "1: MAC Tx pause enabled."
    "0: MAC Tx pause disabled.",
#endif
    UNIMAC_RDP_MODE_MAC_TX_PAUSE_FIELD_MASK,
    0,
    UNIMAC_RDP_MODE_MAC_TX_PAUSE_FIELD_WIDTH,
    UNIMAC_RDP_MODE_MAC_TX_PAUSE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MODE_MAC_RX_PAUSE
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MODE_MAC_RX_PAUSE_FIELD =
{
    "MAC_RX_PAUSE",
#if RU_INCLUDE_DESC
    "",
    "1: MAC Rx pause enabled."
    "0: MAC Rx pause disabled.",
#endif
    UNIMAC_RDP_MODE_MAC_RX_PAUSE_FIELD_MASK,
    0,
    UNIMAC_RDP_MODE_MAC_RX_PAUSE_FIELD_WIDTH,
    UNIMAC_RDP_MODE_MAC_RX_PAUSE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MODE_MAC_DUPLEX
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MODE_MAC_DUPLEX_FIELD =
{
    "MAC_DUPLEX",
#if RU_INCLUDE_DESC
    "",
    "1: Half duplex."
    "0: Full duplex.",
#endif
    UNIMAC_RDP_MODE_MAC_DUPLEX_FIELD_MASK,
    0,
    UNIMAC_RDP_MODE_MAC_DUPLEX_FIELD_WIDTH,
    UNIMAC_RDP_MODE_MAC_DUPLEX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MODE_MAC_SPEED
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MODE_MAC_SPEED_FIELD =
{
    "MAC_SPEED",
#if RU_INCLUDE_DESC
    "",
    "00: 10Mbps, 01: 100Mbps, 10: 1Gbps, 11: 2.5Gbps",
#endif
    UNIMAC_RDP_MODE_MAC_SPEED_FIELD_MASK,
    0,
    UNIMAC_RDP_MODE_MAC_SPEED_FIELD_WIDTH,
    UNIMAC_RDP_MODE_MAC_SPEED_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_FRM_TAG0_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_FRM_TAG0_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_FRM_TAG0_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_FRM_TAG0_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_FRM_TAG0_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_FRM_TAG0_OUTER_TAG
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_FRM_TAG0_OUTER_TAG_FIELD =
{
    "OUTER_TAG",
#if RU_INCLUDE_DESC
    "",
    "Programmable outer tag.",
#endif
    UNIMAC_RDP_FRM_TAG0_OUTER_TAG_FIELD_MASK,
    0,
    UNIMAC_RDP_FRM_TAG0_OUTER_TAG_FIELD_WIDTH,
    UNIMAC_RDP_FRM_TAG0_OUTER_TAG_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_FRM_TAG1_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_FRM_TAG1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_FRM_TAG1_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_FRM_TAG1_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_FRM_TAG1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_FRM_TAG1_INNER_TAG
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_FRM_TAG1_INNER_TAG_FIELD =
{
    "INNER_TAG",
#if RU_INCLUDE_DESC
    "",
    "Programmable inner tag.",
#endif
    UNIMAC_RDP_FRM_TAG1_INNER_TAG_FIELD_MASK,
    0,
    UNIMAC_RDP_FRM_TAG1_INNER_TAG_FIELD_WIDTH,
    UNIMAC_RDP_FRM_TAG1_INNER_TAG_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_TX_IPG_LEN_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_TX_IPG_LEN_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_TX_IPG_LEN_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_TX_IPG_LEN_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_TX_IPG_LEN_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_TX_IPG_LEN_TX_IPG_LEN
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_TX_IPG_LEN_TX_IPG_LEN_FIELD =
{
    "TX_IPG_LEN",
#if RU_INCLUDE_DESC
    "",
    "Programmble inter-packet Gap. Set the transmit minimum IPG from 8 to 26 byte-times."
    "If a value below 8 or above 26 is programmed, the min. IPG is set to 12 byte_times.",
#endif
    UNIMAC_RDP_TX_IPG_LEN_TX_IPG_LEN_FIELD_MASK,
    0,
    UNIMAC_RDP_TX_IPG_LEN_TX_IPG_LEN_FIELD_WIDTH,
    UNIMAC_RDP_TX_IPG_LEN_TX_IPG_LEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_EEE_CTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_EEE_CTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_EEE_CTRL_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_EEE_CTRL_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_EEE_CTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_EEE_CTRL_LP_IDLE_PREDICTION_MODE
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_EEE_CTRL_LP_IDLE_PREDICTION_MODE_FIELD =
{
    "LP_IDLE_PREDICTION_MODE",
#if RU_INCLUDE_DESC
    "",
    "1: Enable LP_IDLE prediction. \n"
    "0: Disable LP_IDLE prediction",
#endif
    UNIMAC_RDP_EEE_CTRL_LP_IDLE_PREDICTION_MODE_FIELD_MASK,
    0,
    UNIMAC_RDP_EEE_CTRL_LP_IDLE_PREDICTION_MODE_FIELD_WIDTH,
    UNIMAC_RDP_EEE_CTRL_LP_IDLE_PREDICTION_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_EEE_CTRL_DIS_EEE_10M
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_EEE_CTRL_DIS_EEE_10M_FIELD =
{
    "DIS_EEE_10M",
#if RU_INCLUDE_DESC
    "",
    "1: When this bit is set and link is established at 10Mbp, LPI is not supported(saving is achieved by reduced PHY's output swing). UNIMAC ignores EEE feature on both Tx & Rx in 10Mbps. \n"
    "0: When cleared, Unimac doesn't differentiate between speeds for EEE features.",
#endif
    UNIMAC_RDP_EEE_CTRL_DIS_EEE_10M_FIELD_MASK,
    0,
    UNIMAC_RDP_EEE_CTRL_DIS_EEE_10M_FIELD_WIDTH,
    UNIMAC_RDP_EEE_CTRL_DIS_EEE_10M_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_EEE_CTRL_EEE_TXCLK_DIS
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_EEE_CTRL_EEE_TXCLK_DIS_FIELD =
{
    "EEE_TXCLK_DIS",
#if RU_INCLUDE_DESC
    "",
    "If enabled, UNIMAC will shut down TXCLK to PHY, when in LPI state.",
#endif
    UNIMAC_RDP_EEE_CTRL_EEE_TXCLK_DIS_FIELD_MASK,
    0,
    UNIMAC_RDP_EEE_CTRL_EEE_TXCLK_DIS_FIELD_WIDTH,
    UNIMAC_RDP_EEE_CTRL_EEE_TXCLK_DIS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_EEE_CTRL_RX_FIFO_CHECK
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_EEE_CTRL_RX_FIFO_CHECK_FIELD =
{
    "RX_FIFO_CHECK",
#if RU_INCLUDE_DESC
    "",
    "If enabled, lpi_rx_detect is set whenever the LPI_IDELS are being received on the RX line and Unimac Rx FIFO is empty.\n"
    "By default, lpi_rx_detect is set only whenever the LPI_IDLES are being received on the RX line.",
#endif
    UNIMAC_RDP_EEE_CTRL_RX_FIFO_CHECK_FIELD_MASK,
    0,
    UNIMAC_RDP_EEE_CTRL_RX_FIFO_CHECK_FIELD_WIDTH,
    UNIMAC_RDP_EEE_CTRL_RX_FIFO_CHECK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_EEE_CTRL_EEE_EN
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_EEE_CTRL_EEE_EN_FIELD =
{
    "EEE_EN",
#if RU_INCLUDE_DESC
    "",
    "If set, the TX LPI policy control engine is enabled and the MAC inserts LPI_idle codes if the link is idle. The rx_lpi_detect assertion is independent of this configuration. \n"
    "Reset default depends EEE_en_strap_input, which if tied to 1, defaults to enabled, otherwise if tied to 0, defaults to disabled.",
#endif
    UNIMAC_RDP_EEE_CTRL_EEE_EN_FIELD_MASK,
    0,
    UNIMAC_RDP_EEE_CTRL_EEE_EN_FIELD_WIDTH,
    UNIMAC_RDP_EEE_CTRL_EEE_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_EEE_CTRL_EN_LPI_TX_PAUSE
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_EEE_CTRL_EN_LPI_TX_PAUSE_FIELD =
{
    "EN_LPI_TX_PAUSE",
#if RU_INCLUDE_DESC
    "",
    "Enable LPI Tx Pause.",
#endif
    UNIMAC_RDP_EEE_CTRL_EN_LPI_TX_PAUSE_FIELD_MASK,
    0,
    UNIMAC_RDP_EEE_CTRL_EN_LPI_TX_PAUSE_FIELD_WIDTH,
    UNIMAC_RDP_EEE_CTRL_EN_LPI_TX_PAUSE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_EEE_CTRL_EN_LPI_TX_PFC
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_EEE_CTRL_EN_LPI_TX_PFC_FIELD =
{
    "EN_LPI_TX_PFC",
#if RU_INCLUDE_DESC
    "",
    "Enable LPI Tx PFC.",
#endif
    UNIMAC_RDP_EEE_CTRL_EN_LPI_TX_PFC_FIELD_MASK,
    0,
    UNIMAC_RDP_EEE_CTRL_EN_LPI_TX_PFC_FIELD_WIDTH,
    UNIMAC_RDP_EEE_CTRL_EN_LPI_TX_PFC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_EEE_CTRL_EN_LPI_RX_PAUSE
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_EEE_CTRL_EN_LPI_RX_PAUSE_FIELD =
{
    "EN_LPI_RX_PAUSE",
#if RU_INCLUDE_DESC
    "",
    "Enable LPI Rx Pause.",
#endif
    UNIMAC_RDP_EEE_CTRL_EN_LPI_RX_PAUSE_FIELD_MASK,
    0,
    UNIMAC_RDP_EEE_CTRL_EN_LPI_RX_PAUSE_FIELD_WIDTH,
    UNIMAC_RDP_EEE_CTRL_EN_LPI_RX_PAUSE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_EEE_LPI_TIMER_EEE_LPI_TIMER
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_EEE_LPI_TIMER_EEE_LPI_TIMER_FIELD =
{
    "EEE_LPI_TIMER",
#if RU_INCLUDE_DESC
    "",
    "EEE LPI Timer.",
#endif
    UNIMAC_RDP_EEE_LPI_TIMER_EEE_LPI_TIMER_FIELD_MASK,
    0,
    UNIMAC_RDP_EEE_LPI_TIMER_EEE_LPI_TIMER_FIELD_WIDTH,
    UNIMAC_RDP_EEE_LPI_TIMER_EEE_LPI_TIMER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_EEE_WAKE_TIMER_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_EEE_WAKE_TIMER_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_EEE_WAKE_TIMER_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_EEE_WAKE_TIMER_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_EEE_WAKE_TIMER_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_EEE_WAKE_TIMER_EEE_WAKE_TIMER
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_EEE_WAKE_TIMER_EEE_WAKE_TIMER_FIELD =
{
    "EEE_WAKE_TIMER",
#if RU_INCLUDE_DESC
    "",
    "EEE Wake Timer.",
#endif
    UNIMAC_RDP_EEE_WAKE_TIMER_EEE_WAKE_TIMER_FIELD_MASK,
    0,
    UNIMAC_RDP_EEE_WAKE_TIMER_EEE_WAKE_TIMER_FIELD_WIDTH,
    UNIMAC_RDP_EEE_WAKE_TIMER_EEE_WAKE_TIMER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_EEE_REF_COUNT_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_EEE_REF_COUNT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_EEE_REF_COUNT_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_EEE_REF_COUNT_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_EEE_REF_COUNT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_EEE_REF_COUNT_EEE_REFERENCE_COUNT
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_EEE_REF_COUNT_EEE_REFERENCE_COUNT_FIELD =
{
    "EEE_REFERENCE_COUNT",
#if RU_INCLUDE_DESC
    "",
    "Clock divider for 1 us quanta count in EEE. This field controls clock divider used to generate ~1us reference pulses used by EEE timers.\n"
    "It specifies integer number of system clock cycles contained within 1us.",
#endif
    UNIMAC_RDP_EEE_REF_COUNT_EEE_REFERENCE_COUNT_FIELD_MASK,
    0,
    UNIMAC_RDP_EEE_REF_COUNT_EEE_REFERENCE_COUNT_FIELD_WIDTH,
    UNIMAC_RDP_EEE_REF_COUNT_EEE_REFERENCE_COUNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_RX_PKT_DROP_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_RX_PKT_DROP_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_RX_PKT_DROP_STATUS_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_RX_PKT_DROP_STATUS_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_RX_PKT_DROP_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_RX_PKT_DROP_STATUS_RX_IPG_INVALID
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_RX_PKT_DROP_STATUS_RX_IPG_INVALID_FIELD =
{
    "RX_IPG_INVALID",
#if RU_INCLUDE_DESC
    "",
    "Debug status, set if MAC receives an IPG less than programmed RX IPG or less than four bytes. \n"
    "Sticky bit. Clears when SW writes 0 into the file or by sw_reset\n",
#endif
    UNIMAC_RDP_RX_PKT_DROP_STATUS_RX_IPG_INVALID_FIELD_MASK,
    0,
    UNIMAC_RDP_RX_PKT_DROP_STATUS_RX_IPG_INVALID_FIELD_WIDTH,
    UNIMAC_RDP_RX_PKT_DROP_STATUS_RX_IPG_INVALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_SYMMETRIC_IDLE_THRESHOLD_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_SYMMETRIC_IDLE_THRESHOLD_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_SYMMETRIC_IDLE_THRESHOLD_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_SYMMETRIC_IDLE_THRESHOLD_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_SYMMETRIC_IDLE_THRESHOLD_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_SYMMETRIC_IDLE_THRESHOLD_THRESHOLD_VALUE
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_SYMMETRIC_IDLE_THRESHOLD_THRESHOLD_VALUE_FIELD =
{
    "THRESHOLD_VALUE",
#if RU_INCLUDE_DESC
    "",
    "If LPI_Prediction is enabled, then this register defines the number of IDLEs to received by the Unimac.\n"
    "before allowing LP_IDLE to be sent to Link Partner.",
#endif
    UNIMAC_RDP_SYMMETRIC_IDLE_THRESHOLD_THRESHOLD_VALUE_FIELD_MASK,
    0,
    UNIMAC_RDP_SYMMETRIC_IDLE_THRESHOLD_THRESHOLD_VALUE_FIELD_WIDTH,
    UNIMAC_RDP_SYMMETRIC_IDLE_THRESHOLD_THRESHOLD_VALUE_FIELD_SHIFT,
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
    "Programmable CRC value to corrupt the TX CRC to be used in MACSEC. The transmitted crc can be corrupted by replacing"
    "the FCS of the transmitted frame by the FCS programmed in this register.",
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
 * Field: UNIMAC_RDP_MACSEC_CNTRL_TX_CRC_PROGRAM
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MACSEC_CNTRL_TX_CRC_PROGRAM_FIELD =
{
    "TX_CRC_PROGRAM",
#if RU_INCLUDE_DESC
    "",
    "1: Replaces the transmitted FCS with the programmed FCS."
    "0: Corrupt the CRC of the transmitted packet internally.",
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
 * Field: UNIMAC_RDP_MACSEC_CNTRL_TX_CRC_CORRUPT_EN
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MACSEC_CNTRL_TX_CRC_CORRUPT_EN_FIELD =
{
    "TX_CRC_CORRUPT_EN",
#if RU_INCLUDE_DESC
    "",
    "1: Enables the CRC corruption on the transmitted packets."
    "The CRC corruption happens only on the frames for which TXERR is asserted by the system.",
#endif
    UNIMAC_RDP_MACSEC_CNTRL_TX_CRC_CORRUPT_EN_FIELD_MASK,
    0,
    UNIMAC_RDP_MACSEC_CNTRL_TX_CRC_CORRUPT_EN_FIELD_WIDTH,
    UNIMAC_RDP_MACSEC_CNTRL_TX_CRC_CORRUPT_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MACSEC_CNTRL_TX_LANUCH_EN
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MACSEC_CNTRL_TX_LANUCH_EN_FIELD =
{
    "TX_LANUCH_EN",
#if RU_INCLUDE_DESC
    "",
    "1: The launch_enable signal assertion/deassertion causes the packet transmit enable/disabled"
    "0: Disable TX launch function.",
#endif
    UNIMAC_RDP_MACSEC_CNTRL_TX_LANUCH_EN_FIELD_MASK,
    0,
    UNIMAC_RDP_MACSEC_CNTRL_TX_LANUCH_EN_FIELD_WIDTH,
    UNIMAC_RDP_MACSEC_CNTRL_TX_LANUCH_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_TS_STATUS_CNTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_TS_STATUS_CNTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_TS_STATUS_CNTRL_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_TS_STATUS_CNTRL_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_TS_STATUS_CNTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_TS_STATUS_CNTRL_WORD_AVAIL
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_TS_STATUS_CNTRL_WORD_AVAIL_FIELD =
{
    "WORD_AVAIL",
#if RU_INCLUDE_DESC
    "",
    "The number of FIFO cells where data is available.",
#endif
    UNIMAC_RDP_TS_STATUS_CNTRL_WORD_AVAIL_FIELD_MASK,
    0,
    UNIMAC_RDP_TS_STATUS_CNTRL_WORD_AVAIL_FIELD_WIDTH,
    UNIMAC_RDP_TS_STATUS_CNTRL_WORD_AVAIL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_TS_STATUS_CNTRL_TX_TS_FIFO_EMPTY
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_TS_STATUS_CNTRL_TX_TS_FIFO_EMPTY_FIELD =
{
    "TX_TS_FIFO_EMPTY",
#if RU_INCLUDE_DESC
    "",
    "Indicated that the transmit timestamp FIFO is empty.",
#endif
    UNIMAC_RDP_TS_STATUS_CNTRL_TX_TS_FIFO_EMPTY_FIELD_MASK,
    0,
    UNIMAC_RDP_TS_STATUS_CNTRL_TX_TS_FIFO_EMPTY_FIELD_WIDTH,
    UNIMAC_RDP_TS_STATUS_CNTRL_TX_TS_FIFO_EMPTY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_TS_STATUS_CNTRL_TX_TS_FIFO_FULL
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_TS_STATUS_CNTRL_TX_TS_FIFO_FULL_FIELD =
{
    "TX_TS_FIFO_FULL",
#if RU_INCLUDE_DESC
    "",
    "Indicated that the transmit timestamp FIFO is full.",
#endif
    UNIMAC_RDP_TS_STATUS_CNTRL_TX_TS_FIFO_FULL_FIELD_MASK,
    0,
    UNIMAC_RDP_TS_STATUS_CNTRL_TX_TS_FIFO_FULL_FIELD_WIDTH,
    UNIMAC_RDP_TS_STATUS_CNTRL_TX_TS_FIFO_FULL_FIELD_SHIFT,
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
    "Every read of this register will fetch out one timestamp value from the transmit FIFO."
    "(One timsstamp value per one read command) (RO)",
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
 * Field: UNIMAC_RDP_PAUSE_CNTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_PAUSE_CNTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_PAUSE_CNTRL_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_PAUSE_CNTRL_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_PAUSE_CNTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_PAUSE_CNTRL_PAUSE_CONTROL_EN
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_PAUSE_CNTRL_PAUSE_CONTROL_EN_FIELD =
{
    "PAUSE_CONTROL_EN",
#if RU_INCLUDE_DESC
    "",
    "Repetitive pause frame send enable. When enabled and back pressure asserted. pauses are tranmitted when PAUSE_TIMER value expires.",
#endif
    UNIMAC_RDP_PAUSE_CNTRL_PAUSE_CONTROL_EN_FIELD_MASK,
    0,
    UNIMAC_RDP_PAUSE_CNTRL_PAUSE_CONTROL_EN_FIELD_WIDTH,
    UNIMAC_RDP_PAUSE_CNTRL_PAUSE_CONTROL_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_PAUSE_CNTRL_PAUSE_TIMER
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_PAUSE_CNTRL_PAUSE_TIMER_FIELD =
{
    "PAUSE_TIMER",
#if RU_INCLUDE_DESC
    "",
    "Pause timer value for repetitive pause frame.",
#endif
    UNIMAC_RDP_PAUSE_CNTRL_PAUSE_TIMER_FIELD_MASK,
    0,
    UNIMAC_RDP_PAUSE_CNTRL_PAUSE_TIMER_FIELD_WIDTH,
    UNIMAC_RDP_PAUSE_CNTRL_PAUSE_TIMER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_TXFIFO_FLUSH_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_TXFIFO_FLUSH_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_TXFIFO_FLUSH_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_TXFIFO_FLUSH_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_TXFIFO_FLUSH_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_TXFIFO_FLUSH_TX_FLUSH
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_TXFIFO_FLUSH_TX_FLUSH_FIELD =
{
    "TX_FLUSH",
#if RU_INCLUDE_DESC
    "",
    "1: TXFIFO_FULL is de-asserted and MAC flushes all the data from the system.Flush happens at packet boundaries."
    "0: MAC resumes normal operation.",
#endif
    UNIMAC_RDP_TXFIFO_FLUSH_TX_FLUSH_FIELD_MASK,
    0,
    UNIMAC_RDP_TXFIFO_FLUSH_TX_FLUSH_FIELD_WIDTH,
    UNIMAC_RDP_TXFIFO_FLUSH_TX_FLUSH_FIELD_SHIFT,
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
 * Field: UNIMAC_RDP_RXFIFO_STAT_RXFIFO_STATUS
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_RXFIFO_STAT_RXFIFO_STATUS_FIELD =
{
    "RXFIFO_STATUS",
#if RU_INCLUDE_DESC
    "",
    "RXFIFO status, only cleared by HW or SW reset.",
#endif
    UNIMAC_RDP_RXFIFO_STAT_RXFIFO_STATUS_FIELD_MASK,
    0,
    UNIMAC_RDP_RXFIFO_STAT_RXFIFO_STATUS_FIELD_WIDTH,
    UNIMAC_RDP_RXFIFO_STAT_RXFIFO_STATUS_FIELD_SHIFT,
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
    "Indicates TXFIFO overrun has occured. Sticky bit only resetable by HW/SW resets.",
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
    "Indicates TXFIFO underrun has occured. Sticky bit only resetable by HW/SW resets.",
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
 * Field: UNIMAC_RDP_PPP_CNTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_PPP_CNTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_PPP_CNTRL_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_PPP_CNTRL_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_PPP_CNTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_PPP_CNTRL_PFC_STATS_EN
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_PPP_CNTRL_PFC_STATS_EN_FIELD =
{
    "PFC_STATS_EN",
#if RU_INCLUDE_DESC
    "",
    "When clear, none of PFC related counters should increment. Otherwise, PFC counters is in full function. \n"
    "Note: it is programming requirement to set this bit when PFC function is enable. ",
#endif
    UNIMAC_RDP_PPP_CNTRL_PFC_STATS_EN_FIELD_MASK,
    0,
    UNIMAC_RDP_PPP_CNTRL_PFC_STATS_EN_FIELD_WIDTH,
    UNIMAC_RDP_PPP_CNTRL_PFC_STATS_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_PPP_CNTRL_RX_PASS_PFC_FRM
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_PPP_CNTRL_RX_PASS_PFC_FRM_FIELD =
{
    "RX_PASS_PFC_FRM",
#if RU_INCLUDE_DESC
    "",
    "When set, MAC pass PFC frame to the system. Otherwise, PFC frame is discarded.",
#endif
    UNIMAC_RDP_PPP_CNTRL_RX_PASS_PFC_FRM_FIELD_MASK,
    0,
    UNIMAC_RDP_PPP_CNTRL_RX_PASS_PFC_FRM_FIELD_WIDTH,
    UNIMAC_RDP_PPP_CNTRL_RX_PASS_PFC_FRM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_PPP_CNTRL_RESERVED1
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_PPP_CNTRL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_PPP_CNTRL_RESERVED1_FIELD_MASK,
    0,
    UNIMAC_RDP_PPP_CNTRL_RESERVED1_FIELD_WIDTH,
    UNIMAC_RDP_PPP_CNTRL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_PPP_CNTRL_FORCE_PPP_XON
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_PPP_CNTRL_FORCE_PPP_XON_FIELD =
{
    "FORCE_PPP_XON",
#if RU_INCLUDE_DESC
    "",
    "Instructs the MAC to send xon for all classes of service.",
#endif
    UNIMAC_RDP_PPP_CNTRL_FORCE_PPP_XON_FIELD_MASK,
    0,
    UNIMAC_RDP_PPP_CNTRL_FORCE_PPP_XON_FIELD_WIDTH,
    UNIMAC_RDP_PPP_CNTRL_FORCE_PPP_XON_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_PPP_CNTRL_PPP_EN_RX
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_PPP_CNTRL_PPP_EN_RX_FIELD =
{
    "PPP_EN_RX",
#if RU_INCLUDE_DESC
    "",
    "Enable MAC PPP_RX function.",
#endif
    UNIMAC_RDP_PPP_CNTRL_PPP_EN_RX_FIELD_MASK,
    0,
    UNIMAC_RDP_PPP_CNTRL_PPP_EN_RX_FIELD_WIDTH,
    UNIMAC_RDP_PPP_CNTRL_PPP_EN_RX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_PPP_CNTRL_PPP_EN_TX
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_PPP_CNTRL_PPP_EN_TX_FIELD =
{
    "PPP_EN_TX",
#if RU_INCLUDE_DESC
    "",
    "Enable MAC PPP_TX function.",
#endif
    UNIMAC_RDP_PPP_CNTRL_PPP_EN_TX_FIELD_MASK,
    0,
    UNIMAC_RDP_PPP_CNTRL_PPP_EN_TX_FIELD_WIDTH,
    UNIMAC_RDP_PPP_CNTRL_PPP_EN_TX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_PPP_REFRESH_CNTRL_PPP_REFRESH_TIMER
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_PPP_REFRESH_CNTRL_PPP_REFRESH_TIMER_FIELD =
{
    "PPP_REFRESH_TIMER",
#if RU_INCLUDE_DESC
    "",
    "The threshold used to load priority Pause refresh time counter every time a priority Pause control frame is transmitted."
    "A refresh priority Pause control frame will be generated and transmitted if the counter reaches 0.",
#endif
    UNIMAC_RDP_PPP_REFRESH_CNTRL_PPP_REFRESH_TIMER_FIELD_MASK,
    0,
    UNIMAC_RDP_PPP_REFRESH_CNTRL_PPP_REFRESH_TIMER_FIELD_WIDTH,
    UNIMAC_RDP_PPP_REFRESH_CNTRL_PPP_REFRESH_TIMER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_PPP_REFRESH_CNTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_PPP_REFRESH_CNTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_PPP_REFRESH_CNTRL_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_PPP_REFRESH_CNTRL_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_PPP_REFRESH_CNTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_PPP_REFRESH_CNTRL_PPP_REFRESH_EN
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_PPP_REFRESH_CNTRL_PPP_REFRESH_EN_FIELD =
{
    "PPP_REFRESH_EN",
#if RU_INCLUDE_DESC
    "",
    "1: Enable MAC PPP refresh transmission function."
    "0: Disabled. ",
#endif
    UNIMAC_RDP_PPP_REFRESH_CNTRL_PPP_REFRESH_EN_FIELD_MASK,
    0,
    UNIMAC_RDP_PPP_REFRESH_CNTRL_PPP_REFRESH_EN_FIELD_WIDTH,
    UNIMAC_RDP_PPP_REFRESH_CNTRL_PPP_REFRESH_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_TX_PAUSE_PREL0_TX_PAUSE_PRB0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_TX_PAUSE_PREL0_TX_PAUSE_PRB0_FIELD =
{
    "TX_PAUSE_PRB0",
#if RU_INCLUDE_DESC
    "",
    "TX_PAUSE_PRBL[31:0], always return 0 when read",
#endif
    UNIMAC_RDP_TX_PAUSE_PREL0_TX_PAUSE_PRB0_FIELD_MASK,
    0,
    UNIMAC_RDP_TX_PAUSE_PREL0_TX_PAUSE_PRB0_FIELD_WIDTH,
    UNIMAC_RDP_TX_PAUSE_PREL0_TX_PAUSE_PRB0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_TX_PAUSE_PREL1_TX_PAUSE_PRB1
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_TX_PAUSE_PREL1_TX_PAUSE_PRB1_FIELD =
{
    "TX_PAUSE_PRB1",
#if RU_INCLUDE_DESC
    "",
    "TX_PAUSE_PRBL[63:32], always return 0 when read",
#endif
    UNIMAC_RDP_TX_PAUSE_PREL1_TX_PAUSE_PRB1_FIELD_MASK,
    0,
    UNIMAC_RDP_TX_PAUSE_PREL1_TX_PAUSE_PRB1_FIELD_WIDTH,
    UNIMAC_RDP_TX_PAUSE_PREL1_TX_PAUSE_PRB1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_TX_PAUSE_PREL2_TX_PAUSE_PRB2
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_TX_PAUSE_PREL2_TX_PAUSE_PRB2_FIELD =
{
    "TX_PAUSE_PRB2",
#if RU_INCLUDE_DESC
    "",
    "TX_PAUSE_PRBL[95:64], always return 0 when read",
#endif
    UNIMAC_RDP_TX_PAUSE_PREL2_TX_PAUSE_PRB2_FIELD_MASK,
    0,
    UNIMAC_RDP_TX_PAUSE_PREL2_TX_PAUSE_PRB2_FIELD_WIDTH,
    UNIMAC_RDP_TX_PAUSE_PREL2_TX_PAUSE_PRB2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_TX_PAUSE_PREL3_TX_PAUSE_PRB3
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_TX_PAUSE_PREL3_TX_PAUSE_PRB3_FIELD =
{
    "TX_PAUSE_PRB3",
#if RU_INCLUDE_DESC
    "",
    "TX_PAUSE_PRBL[127:96], always return 0 when read",
#endif
    UNIMAC_RDP_TX_PAUSE_PREL3_TX_PAUSE_PRB3_FIELD_MASK,
    0,
    UNIMAC_RDP_TX_PAUSE_PREL3_TX_PAUSE_PRB3_FIELD_WIDTH,
    UNIMAC_RDP_TX_PAUSE_PREL3_TX_PAUSE_PRB3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_RX_PAUSE_PREL0_RX_PAUSE_PRB0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_RX_PAUSE_PREL0_RX_PAUSE_PRB0_FIELD =
{
    "RX_PAUSE_PRB0",
#if RU_INCLUDE_DESC
    "",
    "RX_PAUSE_PRBL[31:0], always return 0 when read",
#endif
    UNIMAC_RDP_RX_PAUSE_PREL0_RX_PAUSE_PRB0_FIELD_MASK,
    0,
    UNIMAC_RDP_RX_PAUSE_PREL0_RX_PAUSE_PRB0_FIELD_WIDTH,
    UNIMAC_RDP_RX_PAUSE_PREL0_RX_PAUSE_PRB0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_RX_PAUSE_PREL1_RX_PAUSE_PRB1
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_RX_PAUSE_PREL1_RX_PAUSE_PRB1_FIELD =
{
    "RX_PAUSE_PRB1",
#if RU_INCLUDE_DESC
    "",
    "RX_PAUSE_PRBL[63:32], always return 0 when read",
#endif
    UNIMAC_RDP_RX_PAUSE_PREL1_RX_PAUSE_PRB1_FIELD_MASK,
    0,
    UNIMAC_RDP_RX_PAUSE_PREL1_RX_PAUSE_PRB1_FIELD_WIDTH,
    UNIMAC_RDP_RX_PAUSE_PREL1_RX_PAUSE_PRB1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_RX_PAUSE_PREL2_RX_PAUSE_PRB2
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_RX_PAUSE_PREL2_RX_PAUSE_PRB2_FIELD =
{
    "RX_PAUSE_PRB2",
#if RU_INCLUDE_DESC
    "",
    "RX_PAUSE_PRBL[95:64], always return 0 when read",
#endif
    UNIMAC_RDP_RX_PAUSE_PREL2_RX_PAUSE_PRB2_FIELD_MASK,
    0,
    UNIMAC_RDP_RX_PAUSE_PREL2_RX_PAUSE_PRB2_FIELD_WIDTH,
    UNIMAC_RDP_RX_PAUSE_PREL2_RX_PAUSE_PRB2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_RX_PAUSE_PREL3_RX_PAUSE_PRB3
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_RX_PAUSE_PREL3_RX_PAUSE_PRB3_FIELD =
{
    "RX_PAUSE_PRB3",
#if RU_INCLUDE_DESC
    "",
    "RX_PAUSE_PRBL[127:96], always return 0 when read",
#endif
    UNIMAC_RDP_RX_PAUSE_PREL3_RX_PAUSE_PRB3_FIELD_MASK,
    0,
    UNIMAC_RDP_RX_PAUSE_PREL3_RX_PAUSE_PRB3_FIELD_WIDTH,
    UNIMAC_RDP_RX_PAUSE_PREL3_RX_PAUSE_PRB3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_RXERR_MASK_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_RXERR_MASK_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_RXERR_MASK_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_RXERR_MASK_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_RXERR_MASK_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_RXERR_MASK_MAC_RXERR_MASK
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_RXERR_MASK_MAC_RXERR_MASK_FIELD =
{
    "MAC_RXERR_MASK",
#if RU_INCLUDE_DESC
    "",
    "Mask for RSV[33:16]"
    "MAC_RXERR := |(RSV[33:16] & MAC_RXERR_MASK) ",
#endif
    UNIMAC_RDP_RXERR_MASK_MAC_RXERR_MASK_FIELD_MASK,
    0,
    UNIMAC_RDP_RXERR_MASK_MAC_RXERR_MASK_FIELD_WIDTH,
    UNIMAC_RDP_RXERR_MASK_MAC_RXERR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_RX_MAX_PKT_SIZE_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_RX_MAX_PKT_SIZE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_RX_MAX_PKT_SIZE_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_RX_MAX_PKT_SIZE_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_RX_MAX_PKT_SIZE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_RX_MAX_PKT_SIZE_MAX_PKT_SIZE
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_RX_MAX_PKT_SIZE_MAX_PKT_SIZE_FIELD =
{
    "MAX_PKT_SIZE",
#if RU_INCLUDE_DESC
    "",
    "This is maximum packet size of that is accepted by the UniMAC. Default is 1518B.",
#endif
    UNIMAC_RDP_RX_MAX_PKT_SIZE_MAX_PKT_SIZE_FIELD_MASK,
    0,
    UNIMAC_RDP_RX_MAX_PKT_SIZE_MAX_PKT_SIZE_FIELD_WIDTH,
    UNIMAC_RDP_RX_MAX_PKT_SIZE_MAX_PKT_SIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MDIO_CMD_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MDIO_CMD_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_MDIO_CMD_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_MDIO_CMD_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_MDIO_CMD_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MDIO_CMD_MDIO_BUSY
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MDIO_CMD_MDIO_BUSY_FIELD =
{
    "MDIO_BUSY",
#if RU_INCLUDE_DESC
    "",
    "CPU writes this bit to 1 in order to initiate MCIO transaction. When transaction completes hardware will clear this bit.",
#endif
    UNIMAC_RDP_MDIO_CMD_MDIO_BUSY_FIELD_MASK,
    0,
    UNIMAC_RDP_MDIO_CMD_MDIO_BUSY_FIELD_WIDTH,
    UNIMAC_RDP_MDIO_CMD_MDIO_BUSY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MDIO_CMD_FAIL
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MDIO_CMD_FAIL_FIELD =
{
    "FAIL",
#if RU_INCLUDE_DESC
    "",
    "This bit is set when PHY does not reply to READ command (PHY does not drive 0 on bus turnaround).",
#endif
    UNIMAC_RDP_MDIO_CMD_FAIL_FIELD_MASK,
    0,
    UNIMAC_RDP_MDIO_CMD_FAIL_FIELD_WIDTH,
    UNIMAC_RDP_MDIO_CMD_FAIL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MDIO_CMD_OP_CODE
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MDIO_CMD_OP_CODE_FIELD =
{
    "OP_CODE",
#if RU_INCLUDE_DESC
    "",
    "MDIO command that is OP[1:0]: "
    "00 - Address for clause 45 "
    "01 - Write "
    "10 - Read increment for clause 45 "
    "11 - Read for clause 45 "
    "10 - Read for clause 22 ",
#endif
    UNIMAC_RDP_MDIO_CMD_OP_CODE_FIELD_MASK,
    0,
    UNIMAC_RDP_MDIO_CMD_OP_CODE_FIELD_WIDTH,
    UNIMAC_RDP_MDIO_CMD_OP_CODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MDIO_CMD_PHY_PRT_ADDR
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MDIO_CMD_PHY_PRT_ADDR_FIELD =
{
    "PHY_PRT_ADDR",
#if RU_INCLUDE_DESC
    "",
    "PHY address[4:0] for clause 22, Port address[4:0] for Clause 45.",
#endif
    UNIMAC_RDP_MDIO_CMD_PHY_PRT_ADDR_FIELD_MASK,
    0,
    UNIMAC_RDP_MDIO_CMD_PHY_PRT_ADDR_FIELD_WIDTH,
    UNIMAC_RDP_MDIO_CMD_PHY_PRT_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MDIO_CMD_REG_DEC_ADDR
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MDIO_CMD_REG_DEC_ADDR_FIELD =
{
    "REG_DEC_ADDR",
#if RU_INCLUDE_DESC
    "",
    "Register address[4:0] for clause 22, Device address[4:0] for Clause 45.",
#endif
    UNIMAC_RDP_MDIO_CMD_REG_DEC_ADDR_FIELD_MASK,
    0,
    UNIMAC_RDP_MDIO_CMD_REG_DEC_ADDR_FIELD_WIDTH,
    UNIMAC_RDP_MDIO_CMD_REG_DEC_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MDIO_CMD_DATA_ADDR
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MDIO_CMD_DATA_ADDR_FIELD =
{
    "DATA_ADDR",
#if RU_INCLUDE_DESC
    "",
    "MDIO Read/Write data[15:0], clause 22 and 45 or MDIO address[15:0] for clause 45\".",
#endif
    UNIMAC_RDP_MDIO_CMD_DATA_ADDR_FIELD_MASK,
    0,
    UNIMAC_RDP_MDIO_CMD_DATA_ADDR_FIELD_WIDTH,
    UNIMAC_RDP_MDIO_CMD_DATA_ADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MDIO_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MDIO_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_MDIO_CFG_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_MDIO_CFG_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_MDIO_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MDIO_CFG_MDIO_CLK_DIVIDER
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MDIO_CFG_MDIO_CLK_DIVIDER_FIELD =
{
    "MDIO_CLK_DIVIDER",
#if RU_INCLUDE_DESC
    "",
    "MDIO clock divider[5:0], system clock is divided by 2x(MDIO_CLK_DIVIDER+1) to generate MDIO clock(MDC) = 10.8Mhz; system clock = 108Mhz",
#endif
    UNIMAC_RDP_MDIO_CFG_MDIO_CLK_DIVIDER_FIELD_MASK,
    0,
    UNIMAC_RDP_MDIO_CFG_MDIO_CLK_DIVIDER_FIELD_WIDTH,
    UNIMAC_RDP_MDIO_CFG_MDIO_CLK_DIVIDER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MDIO_CFG_RESERVED1
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MDIO_CFG_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_MDIO_CFG_RESERVED1_FIELD_MASK,
    0,
    UNIMAC_RDP_MDIO_CFG_RESERVED1_FIELD_WIDTH,
    UNIMAC_RDP_MDIO_CFG_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MDIO_CFG_MDIO_CLAUSE
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MDIO_CFG_MDIO_CLAUSE_FIELD =
{
    "MDIO_CLAUSE",
#if RU_INCLUDE_DESC
    "",
    "0: Clause 45 "
    "1: Clause 22 ",
#endif
    UNIMAC_RDP_MDIO_CFG_MDIO_CLAUSE_FIELD_MASK,
    0,
    UNIMAC_RDP_MDIO_CFG_MDIO_CLAUSE_FIELD_WIDTH,
    UNIMAC_RDP_MDIO_CFG_MDIO_CLAUSE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_MDF_CNT_MDF_PACKET_COUNTER
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_MDF_CNT_MDF_PACKET_COUNTER_FIELD =
{
    "MDF_PACKET_COUNTER",
#if RU_INCLUDE_DESC
    "",
    "Counter of MDF packet discarded by RBUF."
    "Saturates at 0xFFFFFFFF",
#endif
    UNIMAC_RDP_MDF_CNT_MDF_PACKET_COUNTER_FIELD_MASK,
    0,
    UNIMAC_RDP_MDF_CNT_MDF_PACKET_COUNTER_FIELD_WIDTH,
    UNIMAC_RDP_MDF_CNT_MDF_PACKET_COUNTER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_DIAG_SEL_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_DIAG_SEL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_DIAG_SEL_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_RDP_DIAG_SEL_RESERVED0_FIELD_WIDTH,
    UNIMAC_RDP_DIAG_SEL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_DIAG_SEL_DIAG_HI_SELECT
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_DIAG_SEL_DIAG_HI_SELECT_FIELD =
{
    "DIAG_HI_SELECT",
#if RU_INCLUDE_DESC
    "",
    "Select Diag Bus[31:15].",
#endif
    UNIMAC_RDP_DIAG_SEL_DIAG_HI_SELECT_FIELD_MASK,
    0,
    UNIMAC_RDP_DIAG_SEL_DIAG_HI_SELECT_FIELD_WIDTH,
    UNIMAC_RDP_DIAG_SEL_DIAG_HI_SELECT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_DIAG_SEL_RESERVED1
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_DIAG_SEL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_RDP_DIAG_SEL_RESERVED1_FIELD_MASK,
    0,
    UNIMAC_RDP_DIAG_SEL_RESERVED1_FIELD_WIDTH,
    UNIMAC_RDP_DIAG_SEL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_RDP_DIAG_SEL_DIAG_LO_SELECT
 ******************************************************************************/
const ru_field_rec UNIMAC_RDP_DIAG_SEL_DIAG_LO_SELECT_FIELD =
{
    "DIAG_LO_SELECT",
#if RU_INCLUDE_DESC
    "",
    "Select Diag Bus[15:0].",
#endif
    UNIMAC_RDP_DIAG_SEL_DIAG_LO_SELECT_FIELD_MASK,
    0,
    UNIMAC_RDP_DIAG_SEL_DIAG_LO_SELECT_FIELD_WIDTH,
    UNIMAC_RDP_DIAG_SEL_DIAG_LO_SELECT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: UNIMAC_RDP_UMAC_DUMMY
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_UMAC_DUMMY_FIELDS[] =
{
    &UNIMAC_RDP_UMAC_DUMMY_RESERVED0_FIELD,
    &UNIMAC_RDP_UMAC_DUMMY_UMAC_DUMMY_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_RDP_UMAC_DUMMY_REG = 
{
    "UMAC_DUMMY",
#if RU_INCLUDE_DESC
    "UniMAC Dummy Register",
    "",
#endif
    UNIMAC_RDP_UMAC_DUMMY_REG_OFFSET,
    0,
    0,
    694,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    UNIMAC_RDP_UMAC_DUMMY_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_HD_BKP_CNTL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_HD_BKP_CNTL_FIELDS[] =
{
    &UNIMAC_RDP_HD_BKP_CNTL_RESERVED0_FIELD,
    &UNIMAC_RDP_HD_BKP_CNTL_IPG_CONFIG_RX_FIELD,
    &UNIMAC_RDP_HD_BKP_CNTL_HD_FC_BKOFF_OK_FIELD,
    &UNIMAC_RDP_HD_BKP_CNTL_HD_FC_ENA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_RDP_HD_BKP_CNTL_REG = 
{
    "HD_BKP_CNTL",
#if RU_INCLUDE_DESC
    "UniMAC Half Duplex Backpressure Control Register",
    "",
#endif
    UNIMAC_RDP_HD_BKP_CNTL_REG_OFFSET,
    0,
    0,
    695,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    UNIMAC_RDP_HD_BKP_CNTL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_CMD
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_CMD_FIELDS[] =
{
    &UNIMAC_RDP_CMD_RESERVED0_FIELD,
    &UNIMAC_RDP_CMD_RUNT_FILTER_DIS_FIELD,
    &UNIMAC_RDP_CMD_TXRX_EN_CONFIG_FIELD,
    &UNIMAC_RDP_CMD_TX_PAUSE_IGNORE_FIELD,
    &UNIMAC_RDP_CMD_PRBL_ENA_FIELD,
    &UNIMAC_RDP_CMD_RX_ERR_DISC_FIELD,
    &UNIMAC_RDP_CMD_RMT_LOOP_ENA_FIELD,
    &UNIMAC_RDP_CMD_NO_LGTH_CHECK_FIELD,
    &UNIMAC_RDP_CMD_CNTL_FRM_ENA_FIELD,
    &UNIMAC_RDP_CMD_ENA_EXT_CONFIG_FIELD,
    &UNIMAC_RDP_CMD_RESERVED1_FIELD,
    &UNIMAC_RDP_CMD_LCL_LOOP_ENA_FIELD,
    &UNIMAC_RDP_CMD_RESERVED2_FIELD,
    &UNIMAC_RDP_CMD_SW_RESET_FIELD,
    &UNIMAC_RDP_CMD_RESERVED3_FIELD,
    &UNIMAC_RDP_CMD_HD_ENA_FIELD,
    &UNIMAC_RDP_CMD_TX_ADDR_INS_FIELD,
    &UNIMAC_RDP_CMD_RX_PAUSE_IGNORE_FIELD,
    &UNIMAC_RDP_CMD_PAUSE_FWD_FIELD,
    &UNIMAC_RDP_CMD_CRC_FWD_FIELD,
    &UNIMAC_RDP_CMD_PAD_EN_FIELD,
    &UNIMAC_RDP_CMD_PROMIS_EN_FIELD,
    &UNIMAC_RDP_CMD_ETH_SPEED_FIELD,
    &UNIMAC_RDP_CMD_RX_ENA_FIELD,
    &UNIMAC_RDP_CMD_TX_ENA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_RDP_CMD_REG = 
{
    "CMD",
#if RU_INCLUDE_DESC
    "UniMAC Command Register",
    "",
#endif
    UNIMAC_RDP_CMD_REG_OFFSET,
    0,
    0,
    696,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    25,
    UNIMAC_RDP_CMD_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_MAC0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_MAC0_FIELDS[] =
{
    &UNIMAC_RDP_MAC0_MAC_0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_RDP_MAC0_REG = 
{
    "MAC0",
#if RU_INCLUDE_DESC
    "UniMAC MAC 0",
    "",
#endif
    UNIMAC_RDP_MAC0_REG_OFFSET,
    0,
    0,
    697,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_RDP_MAC0_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_MAC1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_MAC1_FIELDS[] =
{
    &UNIMAC_RDP_MAC1_RESERVED0_FIELD,
    &UNIMAC_RDP_MAC1_MAC_1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_RDP_MAC1_REG = 
{
    "MAC1",
#if RU_INCLUDE_DESC
    "UniMAC MAC 1",
    "",
#endif
    UNIMAC_RDP_MAC1_REG_OFFSET,
    0,
    0,
    698,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    UNIMAC_RDP_MAC1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_FRM_LEN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_FRM_LEN_FIELDS[] =
{
    &UNIMAC_RDP_FRM_LEN_RESERVED0_FIELD,
    &UNIMAC_RDP_FRM_LEN_FRAME_LENGTH_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_RDP_FRM_LEN_REG = 
{
    "FRM_LEN",
#if RU_INCLUDE_DESC
    "UniMAC Frame Length",
    "",
#endif
    UNIMAC_RDP_FRM_LEN_REG_OFFSET,
    0,
    0,
    699,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    UNIMAC_RDP_FRM_LEN_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_PAUSE_QUNAT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_PAUSE_QUNAT_FIELDS[] =
{
    &UNIMAC_RDP_PAUSE_QUNAT_RESERVED0_FIELD,
    &UNIMAC_RDP_PAUSE_QUNAT_PAUSE_QUANT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_RDP_PAUSE_QUNAT_REG = 
{
    "PAUSE_QUNAT",
#if RU_INCLUDE_DESC
    "UniMAC Pause Quanta",
    "",
#endif
    UNIMAC_RDP_PAUSE_QUNAT_REG_OFFSET,
    0,
    0,
    700,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    UNIMAC_RDP_PAUSE_QUNAT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_SFD_OFFSET
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_SFD_OFFSET_FIELDS[] =
{
    &UNIMAC_RDP_SFD_OFFSET_TEMP_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_RDP_SFD_OFFSET_REG = 
{
    "SFD_OFFSET",
#if RU_INCLUDE_DESC
    "UniMAC EFM Preamble Length",
    "",
#endif
    UNIMAC_RDP_SFD_OFFSET_REG_OFFSET,
    0,
    0,
    701,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_RDP_SFD_OFFSET_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_MODE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_MODE_FIELDS[] =
{
    &UNIMAC_RDP_MODE_RESERVED0_FIELD,
    &UNIMAC_RDP_MODE_MAC_LINK_STAT_FIELD,
    &UNIMAC_RDP_MODE_MAC_TX_PAUSE_FIELD,
    &UNIMAC_RDP_MODE_MAC_RX_PAUSE_FIELD,
    &UNIMAC_RDP_MODE_MAC_DUPLEX_FIELD,
    &UNIMAC_RDP_MODE_MAC_SPEED_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_RDP_MODE_REG = 
{
    "MODE",
#if RU_INCLUDE_DESC
    "UniMAC Mode",
    "",
#endif
    UNIMAC_RDP_MODE_REG_OFFSET,
    0,
    0,
    702,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    UNIMAC_RDP_MODE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_FRM_TAG0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_FRM_TAG0_FIELDS[] =
{
    &UNIMAC_RDP_FRM_TAG0_RESERVED0_FIELD,
    &UNIMAC_RDP_FRM_TAG0_OUTER_TAG_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_RDP_FRM_TAG0_REG = 
{
    "FRM_TAG0",
#if RU_INCLUDE_DESC
    "UniMAC Preamble Outer TAG 0",
    "",
#endif
    UNIMAC_RDP_FRM_TAG0_REG_OFFSET,
    0,
    0,
    703,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    UNIMAC_RDP_FRM_TAG0_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_FRM_TAG1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_FRM_TAG1_FIELDS[] =
{
    &UNIMAC_RDP_FRM_TAG1_RESERVED0_FIELD,
    &UNIMAC_RDP_FRM_TAG1_INNER_TAG_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_RDP_FRM_TAG1_REG = 
{
    "FRM_TAG1",
#if RU_INCLUDE_DESC
    "UniMAC Preamble Outer TAG 1",
    "",
#endif
    UNIMAC_RDP_FRM_TAG1_REG_OFFSET,
    0,
    0,
    704,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    UNIMAC_RDP_FRM_TAG1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_TX_IPG_LEN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_TX_IPG_LEN_FIELDS[] =
{
    &UNIMAC_RDP_TX_IPG_LEN_RESERVED0_FIELD,
    &UNIMAC_RDP_TX_IPG_LEN_TX_IPG_LEN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_RDP_TX_IPG_LEN_REG = 
{
    "TX_IPG_LEN",
#if RU_INCLUDE_DESC
    "UniMAC Inter Packet Gap",
    "",
#endif
    UNIMAC_RDP_TX_IPG_LEN_REG_OFFSET,
    0,
    0,
    705,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    UNIMAC_RDP_TX_IPG_LEN_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_EEE_CTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_EEE_CTRL_FIELDS[] =
{
    &UNIMAC_RDP_EEE_CTRL_RESERVED0_FIELD,
    &UNIMAC_RDP_EEE_CTRL_LP_IDLE_PREDICTION_MODE_FIELD,
    &UNIMAC_RDP_EEE_CTRL_DIS_EEE_10M_FIELD,
    &UNIMAC_RDP_EEE_CTRL_EEE_TXCLK_DIS_FIELD,
    &UNIMAC_RDP_EEE_CTRL_RX_FIFO_CHECK_FIELD,
    &UNIMAC_RDP_EEE_CTRL_EEE_EN_FIELD,
    &UNIMAC_RDP_EEE_CTRL_EN_LPI_TX_PAUSE_FIELD,
    &UNIMAC_RDP_EEE_CTRL_EN_LPI_TX_PFC_FIELD,
    &UNIMAC_RDP_EEE_CTRL_EN_LPI_RX_PAUSE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_RDP_EEE_CTRL_REG = 
{
    "EEE_CTRL",
#if RU_INCLUDE_DESC
    "UniMAC EEE Control",
    "",
#endif
    UNIMAC_RDP_EEE_CTRL_REG_OFFSET,
    0,
    0,
    706,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    UNIMAC_RDP_EEE_CTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_EEE_LPI_TIMER
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_EEE_LPI_TIMER_FIELDS[] =
{
    &UNIMAC_RDP_EEE_LPI_TIMER_EEE_LPI_TIMER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_RDP_EEE_LPI_TIMER_REG = 
{
    "EEE_LPI_TIMER",
#if RU_INCLUDE_DESC
    "UniMAC EEE  LPI Timer",
    "",
#endif
    UNIMAC_RDP_EEE_LPI_TIMER_REG_OFFSET,
    0,
    0,
    707,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_RDP_EEE_LPI_TIMER_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_EEE_WAKE_TIMER
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_EEE_WAKE_TIMER_FIELDS[] =
{
    &UNIMAC_RDP_EEE_WAKE_TIMER_RESERVED0_FIELD,
    &UNIMAC_RDP_EEE_WAKE_TIMER_EEE_WAKE_TIMER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_RDP_EEE_WAKE_TIMER_REG = 
{
    "EEE_WAKE_TIMER",
#if RU_INCLUDE_DESC
    "UniMAC EEE  Wake Timer",
    "",
#endif
    UNIMAC_RDP_EEE_WAKE_TIMER_REG_OFFSET,
    0,
    0,
    708,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    UNIMAC_RDP_EEE_WAKE_TIMER_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_EEE_REF_COUNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_EEE_REF_COUNT_FIELDS[] =
{
    &UNIMAC_RDP_EEE_REF_COUNT_RESERVED0_FIELD,
    &UNIMAC_RDP_EEE_REF_COUNT_EEE_REFERENCE_COUNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_RDP_EEE_REF_COUNT_REG = 
{
    "EEE_REF_COUNT",
#if RU_INCLUDE_DESC
    "UniMAC EEE Reference Count",
    "",
#endif
    UNIMAC_RDP_EEE_REF_COUNT_REG_OFFSET,
    0,
    0,
    709,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    UNIMAC_RDP_EEE_REF_COUNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_RX_PKT_DROP_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_RX_PKT_DROP_STATUS_FIELDS[] =
{
    &UNIMAC_RDP_RX_PKT_DROP_STATUS_RESERVED0_FIELD,
    &UNIMAC_RDP_RX_PKT_DROP_STATUS_RX_IPG_INVALID_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_RDP_RX_PKT_DROP_STATUS_REG = 
{
    "RX_PKT_DROP_STATUS",
#if RU_INCLUDE_DESC
    "UniMAC Rx Pkt Drop Status",
    "",
#endif
    UNIMAC_RDP_RX_PKT_DROP_STATUS_REG_OFFSET,
    0,
    0,
    710,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    UNIMAC_RDP_RX_PKT_DROP_STATUS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_SYMMETRIC_IDLE_THRESHOLD
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_SYMMETRIC_IDLE_THRESHOLD_FIELDS[] =
{
    &UNIMAC_RDP_SYMMETRIC_IDLE_THRESHOLD_RESERVED0_FIELD,
    &UNIMAC_RDP_SYMMETRIC_IDLE_THRESHOLD_THRESHOLD_VALUE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_RDP_SYMMETRIC_IDLE_THRESHOLD_REG = 
{
    "SYMMETRIC_IDLE_THRESHOLD",
#if RU_INCLUDE_DESC
    "UniMAC Symmetric Idle Threshold",
    "",
#endif
    UNIMAC_RDP_SYMMETRIC_IDLE_THRESHOLD_REG_OFFSET,
    0,
    0,
    711,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    UNIMAC_RDP_SYMMETRIC_IDLE_THRESHOLD_FIELDS
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
    "UniMAC Programmable CRC value",
    "",
#endif
    UNIMAC_RDP_MACSEC_PROG_TX_CRC_REG_OFFSET,
    0,
    0,
    712,
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
    &UNIMAC_RDP_MACSEC_CNTRL_TX_CRC_PROGRAM_FIELD,
    &UNIMAC_RDP_MACSEC_CNTRL_TX_CRC_CORRUPT_EN_FIELD,
    &UNIMAC_RDP_MACSEC_CNTRL_TX_LANUCH_EN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_RDP_MACSEC_CNTRL_REG = 
{
    "MACSEC_CNTRL",
#if RU_INCLUDE_DESC
    "UniMAC Misc. MACSEC Control Register",
    "",
#endif
    UNIMAC_RDP_MACSEC_CNTRL_REG_OFFSET,
    0,
    0,
    713,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    UNIMAC_RDP_MACSEC_CNTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_TS_STATUS_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_TS_STATUS_CNTRL_FIELDS[] =
{
    &UNIMAC_RDP_TS_STATUS_CNTRL_RESERVED0_FIELD,
    &UNIMAC_RDP_TS_STATUS_CNTRL_WORD_AVAIL_FIELD,
    &UNIMAC_RDP_TS_STATUS_CNTRL_TX_TS_FIFO_EMPTY_FIELD,
    &UNIMAC_RDP_TS_STATUS_CNTRL_TX_TS_FIFO_FULL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_RDP_TS_STATUS_CNTRL_REG = 
{
    "TS_STATUS_CNTRL",
#if RU_INCLUDE_DESC
    "UniMAC Timestamp Status",
    "",
#endif
    UNIMAC_RDP_TS_STATUS_CNTRL_REG_OFFSET,
    0,
    0,
    714,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    UNIMAC_RDP_TS_STATUS_CNTRL_FIELDS
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
    "UniMAC Timestamp Data",
    "",
#endif
    UNIMAC_RDP_TX_TS_DATA_REG_OFFSET,
    0,
    0,
    715,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_RDP_TX_TS_DATA_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_PAUSE_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_PAUSE_CNTRL_FIELDS[] =
{
    &UNIMAC_RDP_PAUSE_CNTRL_RESERVED0_FIELD,
    &UNIMAC_RDP_PAUSE_CNTRL_PAUSE_CONTROL_EN_FIELD,
    &UNIMAC_RDP_PAUSE_CNTRL_PAUSE_TIMER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_RDP_PAUSE_CNTRL_REG = 
{
    "PAUSE_CNTRL",
#if RU_INCLUDE_DESC
    "UniMAC Repetitive Pause Control in TX direction",
    "",
#endif
    UNIMAC_RDP_PAUSE_CNTRL_REG_OFFSET,
    0,
    0,
    716,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    UNIMAC_RDP_PAUSE_CNTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_TXFIFO_FLUSH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_TXFIFO_FLUSH_FIELDS[] =
{
    &UNIMAC_RDP_TXFIFO_FLUSH_RESERVED0_FIELD,
    &UNIMAC_RDP_TXFIFO_FLUSH_TX_FLUSH_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_RDP_TXFIFO_FLUSH_REG = 
{
    "TXFIFO_FLUSH",
#if RU_INCLUDE_DESC
    "UniMAC TX Flush",
    "",
#endif
    UNIMAC_RDP_TXFIFO_FLUSH_REG_OFFSET,
    0,
    0,
    717,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    UNIMAC_RDP_TXFIFO_FLUSH_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_RXFIFO_STAT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_RXFIFO_STAT_FIELDS[] =
{
    &UNIMAC_RDP_RXFIFO_STAT_RESERVED0_FIELD,
    &UNIMAC_RDP_RXFIFO_STAT_RXFIFO_STATUS_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_RDP_RXFIFO_STAT_REG = 
{
    "RXFIFO_STAT",
#if RU_INCLUDE_DESC
    "UniMAC RX FIFO Status",
    "",
#endif
    UNIMAC_RDP_RXFIFO_STAT_REG_OFFSET,
    0,
    0,
    718,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
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
    "UniMAC TX FIFO Status",
    "",
#endif
    UNIMAC_RDP_TXFIFO_STAT_REG_OFFSET,
    0,
    0,
    719,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    UNIMAC_RDP_TXFIFO_STAT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_PPP_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_PPP_CNTRL_FIELDS[] =
{
    &UNIMAC_RDP_PPP_CNTRL_RESERVED0_FIELD,
    &UNIMAC_RDP_PPP_CNTRL_PFC_STATS_EN_FIELD,
    &UNIMAC_RDP_PPP_CNTRL_RX_PASS_PFC_FRM_FIELD,
    &UNIMAC_RDP_PPP_CNTRL_RESERVED1_FIELD,
    &UNIMAC_RDP_PPP_CNTRL_FORCE_PPP_XON_FIELD,
    &UNIMAC_RDP_PPP_CNTRL_PPP_EN_RX_FIELD,
    &UNIMAC_RDP_PPP_CNTRL_PPP_EN_TX_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_RDP_PPP_CNTRL_REG = 
{
    "PPP_CNTRL",
#if RU_INCLUDE_DESC
    "UniMAC PPP Control",
    "",
#endif
    UNIMAC_RDP_PPP_CNTRL_REG_OFFSET,
    0,
    0,
    720,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    UNIMAC_RDP_PPP_CNTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_PPP_REFRESH_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_PPP_REFRESH_CNTRL_FIELDS[] =
{
    &UNIMAC_RDP_PPP_REFRESH_CNTRL_PPP_REFRESH_TIMER_FIELD,
    &UNIMAC_RDP_PPP_REFRESH_CNTRL_RESERVED0_FIELD,
    &UNIMAC_RDP_PPP_REFRESH_CNTRL_PPP_REFRESH_EN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_RDP_PPP_REFRESH_CNTRL_REG = 
{
    "PPP_REFRESH_CNTRL",
#if RU_INCLUDE_DESC
    "UniMAC Refresh Control",
    "",
#endif
    UNIMAC_RDP_PPP_REFRESH_CNTRL_REG_OFFSET,
    0,
    0,
    721,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    UNIMAC_RDP_PPP_REFRESH_CNTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_TX_PAUSE_PREL0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_TX_PAUSE_PREL0_FIELDS[] =
{
    &UNIMAC_RDP_TX_PAUSE_PREL0_TX_PAUSE_PRB0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_RDP_TX_PAUSE_PREL0_REG = 
{
    "TX_PAUSE_PREL0",
#if RU_INCLUDE_DESC
    "UniMAC TX Pause PRBL[31:0]",
    "",
#endif
    UNIMAC_RDP_TX_PAUSE_PREL0_REG_OFFSET,
    0,
    0,
    722,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_RDP_TX_PAUSE_PREL0_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_TX_PAUSE_PREL1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_TX_PAUSE_PREL1_FIELDS[] =
{
    &UNIMAC_RDP_TX_PAUSE_PREL1_TX_PAUSE_PRB1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_RDP_TX_PAUSE_PREL1_REG = 
{
    "TX_PAUSE_PREL1",
#if RU_INCLUDE_DESC
    "UniMAC TX Pause PRBL[63:32]",
    "",
#endif
    UNIMAC_RDP_TX_PAUSE_PREL1_REG_OFFSET,
    0,
    0,
    723,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_RDP_TX_PAUSE_PREL1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_TX_PAUSE_PREL2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_TX_PAUSE_PREL2_FIELDS[] =
{
    &UNIMAC_RDP_TX_PAUSE_PREL2_TX_PAUSE_PRB2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_RDP_TX_PAUSE_PREL2_REG = 
{
    "TX_PAUSE_PREL2",
#if RU_INCLUDE_DESC
    "UniMAC TX Pause PRBL[95:64]",
    "",
#endif
    UNIMAC_RDP_TX_PAUSE_PREL2_REG_OFFSET,
    0,
    0,
    724,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_RDP_TX_PAUSE_PREL2_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_TX_PAUSE_PREL3
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_TX_PAUSE_PREL3_FIELDS[] =
{
    &UNIMAC_RDP_TX_PAUSE_PREL3_TX_PAUSE_PRB3_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_RDP_TX_PAUSE_PREL3_REG = 
{
    "TX_PAUSE_PREL3",
#if RU_INCLUDE_DESC
    "UniMAC TX Pause PRBL[127:96]",
    "",
#endif
    UNIMAC_RDP_TX_PAUSE_PREL3_REG_OFFSET,
    0,
    0,
    725,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_RDP_TX_PAUSE_PREL3_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_RX_PAUSE_PREL0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_RX_PAUSE_PREL0_FIELDS[] =
{
    &UNIMAC_RDP_RX_PAUSE_PREL0_RX_PAUSE_PRB0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_RDP_RX_PAUSE_PREL0_REG = 
{
    "RX_PAUSE_PREL0",
#if RU_INCLUDE_DESC
    "UniMAC RX Pause PRBL[31:0]",
    "",
#endif
    UNIMAC_RDP_RX_PAUSE_PREL0_REG_OFFSET,
    0,
    0,
    726,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_RDP_RX_PAUSE_PREL0_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_RX_PAUSE_PREL1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_RX_PAUSE_PREL1_FIELDS[] =
{
    &UNIMAC_RDP_RX_PAUSE_PREL1_RX_PAUSE_PRB1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_RDP_RX_PAUSE_PREL1_REG = 
{
    "RX_PAUSE_PREL1",
#if RU_INCLUDE_DESC
    "UniMAC RX Pause PRBL[63:32]",
    "",
#endif
    UNIMAC_RDP_RX_PAUSE_PREL1_REG_OFFSET,
    0,
    0,
    727,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_RDP_RX_PAUSE_PREL1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_RX_PAUSE_PREL2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_RX_PAUSE_PREL2_FIELDS[] =
{
    &UNIMAC_RDP_RX_PAUSE_PREL2_RX_PAUSE_PRB2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_RDP_RX_PAUSE_PREL2_REG = 
{
    "RX_PAUSE_PREL2",
#if RU_INCLUDE_DESC
    "UniMAC RX Pause PRBL[95:64]",
    "",
#endif
    UNIMAC_RDP_RX_PAUSE_PREL2_REG_OFFSET,
    0,
    0,
    728,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_RDP_RX_PAUSE_PREL2_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_RX_PAUSE_PREL3
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_RX_PAUSE_PREL3_FIELDS[] =
{
    &UNIMAC_RDP_RX_PAUSE_PREL3_RX_PAUSE_PRB3_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_RDP_RX_PAUSE_PREL3_REG = 
{
    "RX_PAUSE_PREL3",
#if RU_INCLUDE_DESC
    "UniMAC RX Pause PRBL[127:96]",
    "",
#endif
    UNIMAC_RDP_RX_PAUSE_PREL3_REG_OFFSET,
    0,
    0,
    729,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_RDP_RX_PAUSE_PREL3_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_RXERR_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_RXERR_MASK_FIELDS[] =
{
    &UNIMAC_RDP_RXERR_MASK_RESERVED0_FIELD,
    &UNIMAC_RDP_RXERR_MASK_MAC_RXERR_MASK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_RDP_RXERR_MASK_REG = 
{
    "RXERR_MASK",
#if RU_INCLUDE_DESC
    "UniMAC RXERR Mask Register",
    "",
#endif
    UNIMAC_RDP_RXERR_MASK_REG_OFFSET,
    0,
    0,
    730,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    UNIMAC_RDP_RXERR_MASK_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_RX_MAX_PKT_SIZE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_RX_MAX_PKT_SIZE_FIELDS[] =
{
    &UNIMAC_RDP_RX_MAX_PKT_SIZE_RESERVED0_FIELD,
    &UNIMAC_RDP_RX_MAX_PKT_SIZE_MAX_PKT_SIZE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_RDP_RX_MAX_PKT_SIZE_REG = 
{
    "RX_MAX_PKT_SIZE",
#if RU_INCLUDE_DESC
    "UniMAC RX MAX packet Size Register",
    "",
#endif
    UNIMAC_RDP_RX_MAX_PKT_SIZE_REG_OFFSET,
    0,
    0,
    731,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    UNIMAC_RDP_RX_MAX_PKT_SIZE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_MDIO_CMD
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_MDIO_CMD_FIELDS[] =
{
    &UNIMAC_RDP_MDIO_CMD_RESERVED0_FIELD,
    &UNIMAC_RDP_MDIO_CMD_MDIO_BUSY_FIELD,
    &UNIMAC_RDP_MDIO_CMD_FAIL_FIELD,
    &UNIMAC_RDP_MDIO_CMD_OP_CODE_FIELD,
    &UNIMAC_RDP_MDIO_CMD_PHY_PRT_ADDR_FIELD,
    &UNIMAC_RDP_MDIO_CMD_REG_DEC_ADDR_FIELD,
    &UNIMAC_RDP_MDIO_CMD_DATA_ADDR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_RDP_MDIO_CMD_REG = 
{
    "MDIO_CMD",
#if RU_INCLUDE_DESC
    "MDIO Command Register",
    "",
#endif
    UNIMAC_RDP_MDIO_CMD_REG_OFFSET,
    0,
    0,
    732,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    UNIMAC_RDP_MDIO_CMD_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_MDIO_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_MDIO_CFG_FIELDS[] =
{
    &UNIMAC_RDP_MDIO_CFG_RESERVED0_FIELD,
    &UNIMAC_RDP_MDIO_CFG_MDIO_CLK_DIVIDER_FIELD,
    &UNIMAC_RDP_MDIO_CFG_RESERVED1_FIELD,
    &UNIMAC_RDP_MDIO_CFG_MDIO_CLAUSE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_RDP_MDIO_CFG_REG = 
{
    "MDIO_CFG",
#if RU_INCLUDE_DESC
    "MDIO Configuration Register",
    "",
#endif
    UNIMAC_RDP_MDIO_CFG_REG_OFFSET,
    0,
    0,
    733,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    UNIMAC_RDP_MDIO_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_MDF_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_MDF_CNT_FIELDS[] =
{
    &UNIMAC_RDP_MDF_CNT_MDF_PACKET_COUNTER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_RDP_MDF_CNT_REG = 
{
    "MDF_CNT",
#if RU_INCLUDE_DESC
    "MDF Discard Packet Counter",
    "",
#endif
    UNIMAC_RDP_MDF_CNT_REG_OFFSET,
    0,
    0,
    734,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_RDP_MDF_CNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_RDP_DIAG_SEL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_RDP_DIAG_SEL_FIELDS[] =
{
    &UNIMAC_RDP_DIAG_SEL_RESERVED0_FIELD,
    &UNIMAC_RDP_DIAG_SEL_DIAG_HI_SELECT_FIELD,
    &UNIMAC_RDP_DIAG_SEL_RESERVED1_FIELD,
    &UNIMAC_RDP_DIAG_SEL_DIAG_LO_SELECT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_RDP_DIAG_SEL_REG = 
{
    "DIAG_SEL",
#if RU_INCLUDE_DESC
    "Diag Select Register",
    "",
#endif
    UNIMAC_RDP_DIAG_SEL_REG_OFFSET,
    0,
    0,
    735,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    UNIMAC_RDP_DIAG_SEL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Block: UNIMAC_RDP
 ******************************************************************************/
static const ru_reg_rec *UNIMAC_RDP_REGS[] =
{
    &UNIMAC_RDP_UMAC_DUMMY_REG,
    &UNIMAC_RDP_HD_BKP_CNTL_REG,
    &UNIMAC_RDP_CMD_REG,
    &UNIMAC_RDP_MAC0_REG,
    &UNIMAC_RDP_MAC1_REG,
    &UNIMAC_RDP_FRM_LEN_REG,
    &UNIMAC_RDP_PAUSE_QUNAT_REG,
    &UNIMAC_RDP_SFD_OFFSET_REG,
    &UNIMAC_RDP_MODE_REG,
    &UNIMAC_RDP_FRM_TAG0_REG,
    &UNIMAC_RDP_FRM_TAG1_REG,
    &UNIMAC_RDP_TX_IPG_LEN_REG,
    &UNIMAC_RDP_EEE_CTRL_REG,
    &UNIMAC_RDP_EEE_LPI_TIMER_REG,
    &UNIMAC_RDP_EEE_WAKE_TIMER_REG,
    &UNIMAC_RDP_EEE_REF_COUNT_REG,
    &UNIMAC_RDP_RX_PKT_DROP_STATUS_REG,
    &UNIMAC_RDP_SYMMETRIC_IDLE_THRESHOLD_REG,
    &UNIMAC_RDP_MACSEC_PROG_TX_CRC_REG,
    &UNIMAC_RDP_MACSEC_CNTRL_REG,
    &UNIMAC_RDP_TS_STATUS_CNTRL_REG,
    &UNIMAC_RDP_TX_TS_DATA_REG,
    &UNIMAC_RDP_PAUSE_CNTRL_REG,
    &UNIMAC_RDP_TXFIFO_FLUSH_REG,
    &UNIMAC_RDP_RXFIFO_STAT_REG,
    &UNIMAC_RDP_TXFIFO_STAT_REG,
    &UNIMAC_RDP_PPP_CNTRL_REG,
    &UNIMAC_RDP_PPP_REFRESH_CNTRL_REG,
    &UNIMAC_RDP_TX_PAUSE_PREL0_REG,
    &UNIMAC_RDP_TX_PAUSE_PREL1_REG,
    &UNIMAC_RDP_TX_PAUSE_PREL2_REG,
    &UNIMAC_RDP_TX_PAUSE_PREL3_REG,
    &UNIMAC_RDP_RX_PAUSE_PREL0_REG,
    &UNIMAC_RDP_RX_PAUSE_PREL1_REG,
    &UNIMAC_RDP_RX_PAUSE_PREL2_REG,
    &UNIMAC_RDP_RX_PAUSE_PREL3_REG,
    &UNIMAC_RDP_RXERR_MASK_REG,
    &UNIMAC_RDP_RX_MAX_PKT_SIZE_REG,
    &UNIMAC_RDP_MDIO_CMD_REG,
    &UNIMAC_RDP_MDIO_CFG_REG,
    &UNIMAC_RDP_MDF_CNT_REG,
    &UNIMAC_RDP_DIAG_SEL_REG,
};

unsigned long UNIMAC_RDP_ADDRS[] =
{
    0x82da0000,
    0x82da1000,
    0x82da2000,
};

const ru_block_rec UNIMAC_RDP_BLOCK = 
{
    "UNIMAC_RDP",
    UNIMAC_RDP_ADDRS,
    3,
    42,
    UNIMAC_RDP_REGS
};

/* End of file XRDP_UNIMAC_RDP.c */
