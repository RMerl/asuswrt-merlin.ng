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
 * Field: LPORT_XLMAC_CTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_CTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_CTRL_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_CTRL_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_CTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_CTRL_EXTENDED_HIG2_EN
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_CTRL_EXTENDED_HIG2_EN_FIELD =
{
    "EXTENDED_HIG2_EN",
#if RU_INCLUDE_DESC
    "",
    "Extended Higig 2 header is also known as sirius header. Setting this bit to 0 will disable parsing for the extended header bit(5th bit of 8th header byte) in HG2 header and hence all the Higig 2 packets will be treated as normal Higig2 packets irrespective of extended header bit value. Default value of this field is 1 which will enable parsing extended header bit in every Higig 2 header.",
#endif
    LPORT_XLMAC_CTRL_EXTENDED_HIG2_EN_FIELD_MASK,
    0,
    LPORT_XLMAC_CTRL_EXTENDED_HIG2_EN_FIELD_WIDTH,
    LPORT_XLMAC_CTRL_EXTENDED_HIG2_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_CTRL_LINK_STATUS_SELECT
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_CTRL_LINK_STATUS_SELECT_FIELD =
{
    "LINK_STATUS_SELECT",
#if RU_INCLUDE_DESC
    "",
    "This configuration chooses between link status indication from software (SW_LINK_STATUS) or the hardware link status (hw_link_status)indication from the TSC. If reset, it selects the software link status",
#endif
    LPORT_XLMAC_CTRL_LINK_STATUS_SELECT_FIELD_MASK,
    0,
    LPORT_XLMAC_CTRL_LINK_STATUS_SELECT_FIELD_WIDTH,
    LPORT_XLMAC_CTRL_LINK_STATUS_SELECT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_CTRL_SW_LINK_STATUS
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_CTRL_SW_LINK_STATUS_FIELD =
{
    "SW_LINK_STATUS",
#if RU_INCLUDE_DESC
    "",
    "Link status indication from Software. If set, indicates that link is active.",
#endif
    LPORT_XLMAC_CTRL_SW_LINK_STATUS_FIELD_MASK,
    0,
    LPORT_XLMAC_CTRL_SW_LINK_STATUS_FIELD_WIDTH,
    LPORT_XLMAC_CTRL_SW_LINK_STATUS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_CTRL_XGMII_IPG_CHECK_DISABLE
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_CTRL_XGMII_IPG_CHECK_DISABLE_FIELD =
{
    "XGMII_IPG_CHECK_DISABLE",
#if RU_INCLUDE_DESC
    "",
    "If set, this will override the one column idle/sequence ordered set check before SOP in XGMII mode - effectively supporting  reception of packets with 1 byte IPG in XGMII mode",
#endif
    LPORT_XLMAC_CTRL_XGMII_IPG_CHECK_DISABLE_FIELD_MASK,
    0,
    LPORT_XLMAC_CTRL_XGMII_IPG_CHECK_DISABLE_FIELD_WIDTH,
    LPORT_XLMAC_CTRL_XGMII_IPG_CHECK_DISABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_CTRL_RS_SOFT_RESET
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_CTRL_RS_SOFT_RESET_FIELD =
{
    "RS_SOFT_RESET",
#if RU_INCLUDE_DESC
    "",
    "Resets the RS layer functionality - Fault detection and related responses are disabled and IDLEs are sent on line",
#endif
    LPORT_XLMAC_CTRL_RS_SOFT_RESET_FIELD_MASK,
    0,
    LPORT_XLMAC_CTRL_RS_SOFT_RESET_FIELD_WIDTH,
    LPORT_XLMAC_CTRL_RS_SOFT_RESET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_CTRL_RSVD_5
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_CTRL_RSVD_5_FIELD =
{
    "RSVD_5",
#if RU_INCLUDE_DESC
    "",
    "Reserved",
#endif
    LPORT_XLMAC_CTRL_RSVD_5_FIELD_MASK,
    0,
    LPORT_XLMAC_CTRL_RSVD_5_FIELD_WIDTH,
    LPORT_XLMAC_CTRL_RSVD_5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_CTRL_LOCAL_LPBK_LEAK_ENB
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_CTRL_LOCAL_LPBK_LEAK_ENB_FIELD =
{
    "LOCAL_LPBK_LEAK_ENB",
#if RU_INCLUDE_DESC
    "",
    "If set, during the local loopback mode, the transmit packets are also sent to the transmit line interface, apart from the loopback operation",
#endif
    LPORT_XLMAC_CTRL_LOCAL_LPBK_LEAK_ENB_FIELD_MASK,
    0,
    LPORT_XLMAC_CTRL_LOCAL_LPBK_LEAK_ENB_FIELD_WIDTH,
    LPORT_XLMAC_CTRL_LOCAL_LPBK_LEAK_ENB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_CTRL_RSVD_4
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_CTRL_RSVD_4_FIELD =
{
    "RSVD_4",
#if RU_INCLUDE_DESC
    "",
    "Reserved",
#endif
    LPORT_XLMAC_CTRL_RSVD_4_FIELD_MASK,
    0,
    LPORT_XLMAC_CTRL_RSVD_4_FIELD_WIDTH,
    LPORT_XLMAC_CTRL_RSVD_4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_CTRL_SOFT_RESET
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_CTRL_SOFT_RESET_FIELD =
{
    "SOFT_RESET",
#if RU_INCLUDE_DESC
    "",
    "If set, disables the corresponding port logic and status registers only. Packet data and flow control logic is disabled. Fault handling is active and the MAC will continue to respond to credits from TSC. When the soft reset is cleared MAC will issue a fresh set of credits to EP in transmit side.",
#endif
    LPORT_XLMAC_CTRL_SOFT_RESET_FIELD_MASK,
    0,
    LPORT_XLMAC_CTRL_SOFT_RESET_FIELD_WIDTH,
    LPORT_XLMAC_CTRL_SOFT_RESET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_CTRL_LAG_FAILOVER_EN
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_CTRL_LAG_FAILOVER_EN_FIELD =
{
    "LAG_FAILOVER_EN",
#if RU_INCLUDE_DESC
    "",
    "If set, enable LAG Failover. This bit has priority over LOCAL_LPBK. The lag failover kicks in when the link status selected by LINK_STATUS_SELECT transitions from 1 to 0. TSC clock and TSC credits must be active for lag failover.",
#endif
    LPORT_XLMAC_CTRL_LAG_FAILOVER_EN_FIELD_MASK,
    0,
    LPORT_XLMAC_CTRL_LAG_FAILOVER_EN_FIELD_WIDTH,
    LPORT_XLMAC_CTRL_LAG_FAILOVER_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_CTRL_REMOVE_FAILOVER_LPBK
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_CTRL_REMOVE_FAILOVER_LPBK_FIELD =
{
    "REMOVE_FAILOVER_LPBK",
#if RU_INCLUDE_DESC
    "",
    "If set, XLMAC will move from lag failover state to normal operation. This bit should be set after link is up.",
#endif
    LPORT_XLMAC_CTRL_REMOVE_FAILOVER_LPBK_FIELD_MASK,
    0,
    LPORT_XLMAC_CTRL_REMOVE_FAILOVER_LPBK_FIELD_WIDTH,
    LPORT_XLMAC_CTRL_REMOVE_FAILOVER_LPBK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_CTRL_RSVD_1
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_CTRL_RSVD_1_FIELD =
{
    "RSVD_1",
#if RU_INCLUDE_DESC
    "",
    "Reserved",
#endif
    LPORT_XLMAC_CTRL_RSVD_1_FIELD_MASK,
    0,
    LPORT_XLMAC_CTRL_RSVD_1_FIELD_WIDTH,
    LPORT_XLMAC_CTRL_RSVD_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_CTRL_LOCAL_LPBK
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_CTRL_LOCAL_LPBK_FIELD =
{
    "LOCAL_LPBK",
#if RU_INCLUDE_DESC
    "",
    "If set, enables local loopback from TX to RX. This loopback is on the line side after clock domain crossing - from the last TX pipeline stage to the first RX pipeline stage. Hence, TSC clock and TSC credits must be active for loopback. LAG_FAILOVER_EN should be disabled for this bit to work.",
#endif
    LPORT_XLMAC_CTRL_LOCAL_LPBK_FIELD_MASK,
    0,
    LPORT_XLMAC_CTRL_LOCAL_LPBK_FIELD_WIDTH,
    LPORT_XLMAC_CTRL_LOCAL_LPBK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_CTRL_RX_EN
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_CTRL_RX_EN_FIELD =
{
    "RX_EN",
#if RU_INCLUDE_DESC
    "",
    "If set, enables MAC receive datapath and flowcontrol logic.",
#endif
    LPORT_XLMAC_CTRL_RX_EN_FIELD_MASK,
    0,
    LPORT_XLMAC_CTRL_RX_EN_FIELD_WIDTH,
    LPORT_XLMAC_CTRL_RX_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_CTRL_TX_EN
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_CTRL_TX_EN_FIELD =
{
    "TX_EN",
#if RU_INCLUDE_DESC
    "",
    "If set, enables MAC transmit datapath and flowcontrol logic. When disabled, MAC will respond to TSC credits with IDLE codewords.",
#endif
    LPORT_XLMAC_CTRL_TX_EN_FIELD_MASK,
    0,
    LPORT_XLMAC_CTRL_TX_EN_FIELD_WIDTH,
    LPORT_XLMAC_CTRL_TX_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_MODE_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_MODE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_MODE_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_MODE_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_MODE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_MODE_SPEED_MODE
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_MODE_SPEED_MODE_FIELD =
{
    "SPEED_MODE",
#if RU_INCLUDE_DESC
    "",
    "Port Speed, used for LED indications and internal buffer sizing.",
#endif
    LPORT_XLMAC_MODE_SPEED_MODE_FIELD_MASK,
    0,
    LPORT_XLMAC_MODE_SPEED_MODE_FIELD_WIDTH,
    LPORT_XLMAC_MODE_SPEED_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_MODE_NO_SOP_FOR_CRC_HG
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_MODE_NO_SOP_FOR_CRC_HG_FIELD =
{
    "NO_SOP_FOR_CRC_HG",
#if RU_INCLUDE_DESC
    "",
    "If set, excludes the SOP byte for CRC calculation in HIGIG modes",
#endif
    LPORT_XLMAC_MODE_NO_SOP_FOR_CRC_HG_FIELD_MASK,
    0,
    LPORT_XLMAC_MODE_NO_SOP_FOR_CRC_HG_FIELD_WIDTH,
    LPORT_XLMAC_MODE_NO_SOP_FOR_CRC_HG_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_MODE_HDR_MODE
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_MODE_HDR_MODE_FIELD =
{
    "HDR_MODE",
#if RU_INCLUDE_DESC
    "",
    "Packet Header mode.",
#endif
    LPORT_XLMAC_MODE_HDR_MODE_FIELD_MASK,
    0,
    LPORT_XLMAC_MODE_HDR_MODE_FIELD_WIDTH,
    LPORT_XLMAC_MODE_HDR_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_SPARE0_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_SPARE0_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_SPARE0_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_SPARE0_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_SPARE0_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_SPARE0_RSVD
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_SPARE0_RSVD_FIELD =
{
    "RSVD",
#if RU_INCLUDE_DESC
    "",
    "SPARE REGISTER 0",
#endif
    LPORT_XLMAC_SPARE0_RSVD_FIELD_MASK,
    0,
    LPORT_XLMAC_SPARE0_RSVD_FIELD_WIDTH,
    LPORT_XLMAC_SPARE0_RSVD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_SPARE1_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_SPARE1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_SPARE1_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_SPARE1_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_SPARE1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_SPARE1_RSVD
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_SPARE1_RSVD_FIELD =
{
    "RSVD",
#if RU_INCLUDE_DESC
    "",
    "SPARE REGISTER 1",
#endif
    LPORT_XLMAC_SPARE1_RSVD_FIELD_MASK,
    0,
    LPORT_XLMAC_SPARE1_RSVD_FIELD_WIDTH,
    LPORT_XLMAC_SPARE1_RSVD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TX_CTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TX_CTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_TX_CTRL_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_TX_CTRL_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_TX_CTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TX_CTRL_TX_THRESHOLD
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TX_CTRL_TX_THRESHOLD_FIELD =
{
    "TX_THRESHOLD",
#if RU_INCLUDE_DESC
    "",
    "Indicates the number of 16-byte cells that are buffered per packet in the Tx CDC FIFO, before starting transmission of the packet on the line side.\n"
    "This setting is useful to prevent underflow issues if the EP logic pumps in data at port rate, rather than bursting at full rate.\n"
    "This mode will increase the overall latency.\n"
    "In quad port mode, this field should be set >= 1 and <= 4 for each port.\n"
    "In single port mode, this field should be set >= 1 and <= 16 for the four lane port (port0).\n"
    "In dual port mode, this field should be set >= 1 and <= 8 for each two lane port (port0 and port2).\n"
    "In tri1/tri2, this field should be set >= 1 and <= 4 for each single lane port, and >= 1 and <= 8 for the two lane port.",
#endif
    LPORT_XLMAC_TX_CTRL_TX_THRESHOLD_FIELD_MASK,
    0,
    LPORT_XLMAC_TX_CTRL_TX_THRESHOLD_FIELD_WIDTH,
    LPORT_XLMAC_TX_CTRL_TX_THRESHOLD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TX_CTRL_EP_DISCARD
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TX_CTRL_EP_DISCARD_FIELD =
{
    "EP_DISCARD",
#if RU_INCLUDE_DESC
    "",
    "If set, MAC accepts packets from the EP but does not write to the CDC FIFO and discards them on the core side without updating the statistics.",
#endif
    LPORT_XLMAC_TX_CTRL_EP_DISCARD_FIELD_MASK,
    0,
    LPORT_XLMAC_TX_CTRL_EP_DISCARD_FIELD_WIDTH,
    LPORT_XLMAC_TX_CTRL_EP_DISCARD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TX_CTRL_TX_PREAMBLE_LENGTH
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TX_CTRL_TX_PREAMBLE_LENGTH_FIELD =
{
    "TX_PREAMBLE_LENGTH",
#if RU_INCLUDE_DESC
    "",
    "Number of preamble bytes for transmit IEEE packets, this value should include the K.SOP & SFD character as well",
#endif
    LPORT_XLMAC_TX_CTRL_TX_PREAMBLE_LENGTH_FIELD_MASK,
    0,
    LPORT_XLMAC_TX_CTRL_TX_PREAMBLE_LENGTH_FIELD_WIDTH,
    LPORT_XLMAC_TX_CTRL_TX_PREAMBLE_LENGTH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TX_CTRL_THROT_DENOM
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TX_CTRL_THROT_DENOM_FIELD =
{
    "THROT_DENOM",
#if RU_INCLUDE_DESC
    "",
    "Number of bytes to transmit before adding THROT_NUM bytes to the IPG.  This configuration is used for WAN IPG throttling. Refer MAC specs for more details.",
#endif
    LPORT_XLMAC_TX_CTRL_THROT_DENOM_FIELD_MASK,
    0,
    LPORT_XLMAC_TX_CTRL_THROT_DENOM_FIELD_WIDTH,
    LPORT_XLMAC_TX_CTRL_THROT_DENOM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TX_CTRL_THROT_NUM
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TX_CTRL_THROT_NUM_FIELD =
{
    "THROT_NUM",
#if RU_INCLUDE_DESC
    "",
    "Number of bytes of extra IPG added whenever THROT_DENOM bytes have been transmitted. This configuration is used for WAN IPG throttling. Refer MAC specs for more details.",
#endif
    LPORT_XLMAC_TX_CTRL_THROT_NUM_FIELD_MASK,
    0,
    LPORT_XLMAC_TX_CTRL_THROT_NUM_FIELD_WIDTH,
    LPORT_XLMAC_TX_CTRL_THROT_NUM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TX_CTRL_AVERAGE_IPG
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TX_CTRL_AVERAGE_IPG_FIELD =
{
    "AVERAGE_IPG",
#if RU_INCLUDE_DESC
    "",
    "Average interpacket gap. Must be programmed >= 8. Per packet IPG will vary based on DIC for 10G+ speeds.",
#endif
    LPORT_XLMAC_TX_CTRL_AVERAGE_IPG_FIELD_MASK,
    0,
    LPORT_XLMAC_TX_CTRL_AVERAGE_IPG_FIELD_WIDTH,
    LPORT_XLMAC_TX_CTRL_AVERAGE_IPG_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TX_CTRL_PAD_THRESHOLD
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TX_CTRL_PAD_THRESHOLD_FIELD =
{
    "PAD_THRESHOLD",
#if RU_INCLUDE_DESC
    "",
    "If padding is enabled, packets smaller than PAD_THRESHOLD are padded to this size. This must be set to a value >= 17 (decimal)",
#endif
    LPORT_XLMAC_TX_CTRL_PAD_THRESHOLD_FIELD_MASK,
    0,
    LPORT_XLMAC_TX_CTRL_PAD_THRESHOLD_FIELD_WIDTH,
    LPORT_XLMAC_TX_CTRL_PAD_THRESHOLD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TX_CTRL_PAD_EN
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TX_CTRL_PAD_EN_FIELD =
{
    "PAD_EN",
#if RU_INCLUDE_DESC
    "",
    "If set, enable XLMAC to pad packets smaller than PAD_THRESHOLD on the Tx",
#endif
    LPORT_XLMAC_TX_CTRL_PAD_EN_FIELD_MASK,
    0,
    LPORT_XLMAC_TX_CTRL_PAD_EN_FIELD_WIDTH,
    LPORT_XLMAC_TX_CTRL_PAD_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TX_CTRL_TX_ANY_START
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TX_CTRL_TX_ANY_START_FIELD =
{
    "TX_ANY_START",
#if RU_INCLUDE_DESC
    "",
    "If reset, MAC forces the first byte of a packet to be /S/ character (0xFB) irrespective of incoming EP data at SOP location in HIGIG modes",
#endif
    LPORT_XLMAC_TX_CTRL_TX_ANY_START_FIELD_MASK,
    0,
    LPORT_XLMAC_TX_CTRL_TX_ANY_START_FIELD_WIDTH,
    LPORT_XLMAC_TX_CTRL_TX_ANY_START_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TX_CTRL_DISCARD
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TX_CTRL_DISCARD_FIELD =
{
    "DISCARD",
#if RU_INCLUDE_DESC
    "",
    "If set, MAC accepts packets from the EP and discards them on the line side.  The statistics are updated.",
#endif
    LPORT_XLMAC_TX_CTRL_DISCARD_FIELD_MASK,
    0,
    LPORT_XLMAC_TX_CTRL_DISCARD_FIELD_WIDTH,
    LPORT_XLMAC_TX_CTRL_DISCARD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TX_CTRL_CRC_MODE
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TX_CTRL_CRC_MODE_FIELD =
{
    "CRC_MODE",
#if RU_INCLUDE_DESC
    "",
    "CRC mode for Transmit Side",
#endif
    LPORT_XLMAC_TX_CTRL_CRC_MODE_FIELD_MASK,
    0,
    LPORT_XLMAC_TX_CTRL_CRC_MODE_FIELD_WIDTH,
    LPORT_XLMAC_TX_CTRL_CRC_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TX_CTRL_OVERLAY_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TX_CTRL_OVERLAY_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_TX_CTRL_OVERLAY_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_TX_CTRL_OVERLAY_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_TX_CTRL_OVERLAY_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TX_CTRL_OVERLAY_XLMAC_TX_CTRL_HI
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TX_CTRL_OVERLAY_XLMAC_TX_CTRL_HI_FIELD =
{
    "XLMAC_TX_CTRL_HI",
#if RU_INCLUDE_DESC
    "",
    "10 upper bits of the TX CTRL Register",
#endif
    LPORT_XLMAC_TX_CTRL_OVERLAY_XLMAC_TX_CTRL_HI_FIELD_MASK,
    0,
    LPORT_XLMAC_TX_CTRL_OVERLAY_XLMAC_TX_CTRL_HI_FIELD_WIDTH,
    LPORT_XLMAC_TX_CTRL_OVERLAY_XLMAC_TX_CTRL_HI_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TX_CTRL_OVERLAY_XLMAC_TX_CTRL_LO
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TX_CTRL_OVERLAY_XLMAC_TX_CTRL_LO_FIELD =
{
    "XLMAC_TX_CTRL_LO",
#if RU_INCLUDE_DESC
    "",
    "32 lower bits of the TX CTRL register",
#endif
    LPORT_XLMAC_TX_CTRL_OVERLAY_XLMAC_TX_CTRL_LO_FIELD_MASK,
    0,
    LPORT_XLMAC_TX_CTRL_OVERLAY_XLMAC_TX_CTRL_LO_FIELD_WIDTH,
    LPORT_XLMAC_TX_CTRL_OVERLAY_XLMAC_TX_CTRL_LO_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TX_MAC_SA_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TX_MAC_SA_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_TX_MAC_SA_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_TX_MAC_SA_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_TX_MAC_SA_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TX_MAC_SA_CTRL_SA
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TX_MAC_SA_CTRL_SA_FIELD =
{
    "CTRL_SA",
#if RU_INCLUDE_DESC
    "",
    "Source Address for PAUSE/PFC packets generated by the MAC",
#endif
    LPORT_XLMAC_TX_MAC_SA_CTRL_SA_FIELD_MASK,
    0,
    LPORT_XLMAC_TX_MAC_SA_CTRL_SA_FIELD_WIDTH,
    LPORT_XLMAC_TX_MAC_SA_CTRL_SA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TX_MAC_SA_OVERLAY_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TX_MAC_SA_OVERLAY_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_TX_MAC_SA_OVERLAY_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_TX_MAC_SA_OVERLAY_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_TX_MAC_SA_OVERLAY_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TX_MAC_SA_OVERLAY_SA_HI
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TX_MAC_SA_OVERLAY_SA_HI_FIELD =
{
    "SA_HI",
#if RU_INCLUDE_DESC
    "",
    "16 upper bits of the SA",
#endif
    LPORT_XLMAC_TX_MAC_SA_OVERLAY_SA_HI_FIELD_MASK,
    0,
    LPORT_XLMAC_TX_MAC_SA_OVERLAY_SA_HI_FIELD_WIDTH,
    LPORT_XLMAC_TX_MAC_SA_OVERLAY_SA_HI_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TX_MAC_SA_OVERLAY_SA_LO
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TX_MAC_SA_OVERLAY_SA_LO_FIELD =
{
    "SA_LO",
#if RU_INCLUDE_DESC
    "",
    "32 lower bits of the SA",
#endif
    LPORT_XLMAC_TX_MAC_SA_OVERLAY_SA_LO_FIELD_MASK,
    0,
    LPORT_XLMAC_TX_MAC_SA_OVERLAY_SA_LO_FIELD_WIDTH,
    LPORT_XLMAC_TX_MAC_SA_OVERLAY_SA_LO_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_RX_CTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_RX_CTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_RX_CTRL_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_RX_CTRL_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_RX_CTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_RX_CTRL_RX_PASS_PFC
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_RX_CTRL_RX_PASS_PFC_FIELD =
{
    "RX_PASS_PFC",
#if RU_INCLUDE_DESC
    "",
    "This configuration is used to pass or drop pfc packetw when pfc_ether_type is not equal to 0x8808.              \n"
    "If set, PFC frames are passed to system side. Otherwise, PFC frames are dropped in XLMAC.\n"
    "This configuration is used in Rx CDC mode only.",
#endif
    LPORT_XLMAC_RX_CTRL_RX_PASS_PFC_FIELD_MASK,
    0,
    LPORT_XLMAC_RX_CTRL_RX_PASS_PFC_FIELD_WIDTH,
    LPORT_XLMAC_RX_CTRL_RX_PASS_PFC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_RX_CTRL_RX_PASS_PAUSE
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_RX_CTRL_RX_PASS_PAUSE_FIELD =
{
    "RX_PASS_PAUSE",
#if RU_INCLUDE_DESC
    "",
    "If set, PAUSE frames are passed to sytem side. Otherwise, PAUSE frames are dropped in XLMAC \n"
    "This configuration is used in Rx CDC mode only.",
#endif
    LPORT_XLMAC_RX_CTRL_RX_PASS_PAUSE_FIELD_MASK,
    0,
    LPORT_XLMAC_RX_CTRL_RX_PASS_PAUSE_FIELD_WIDTH,
    LPORT_XLMAC_RX_CTRL_RX_PASS_PAUSE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_RX_CTRL_RX_PASS_CTRL
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_RX_CTRL_RX_PASS_CTRL_FIELD =
{
    "RX_PASS_CTRL",
#if RU_INCLUDE_DESC
    "",
    "This configuration is used to drop or pass all control frames (with ether type 0x8808) except pause packets.\n"
    "If set, all control frames are passed to system side. \n"
    "Otherwise, control frames (including pfc frames wih ether type 0x8808) are dropped in XLMAC.\n"
    "This configuration is used in Rx CDC mode only.",
#endif
    LPORT_XLMAC_RX_CTRL_RX_PASS_CTRL_FIELD_MASK,
    0,
    LPORT_XLMAC_RX_CTRL_RX_PASS_CTRL_FIELD_WIDTH,
    LPORT_XLMAC_RX_CTRL_RX_PASS_CTRL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_RX_CTRL_RSVD_3
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_RX_CTRL_RSVD_3_FIELD =
{
    "RSVD_3",
#if RU_INCLUDE_DESC
    "",
    "Reserved",
#endif
    LPORT_XLMAC_RX_CTRL_RSVD_3_FIELD_MASK,
    0,
    LPORT_XLMAC_RX_CTRL_RSVD_3_FIELD_WIDTH,
    LPORT_XLMAC_RX_CTRL_RSVD_3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_RX_CTRL_RSVD_2
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_RX_CTRL_RSVD_2_FIELD =
{
    "RSVD_2",
#if RU_INCLUDE_DESC
    "",
    "Reserved",
#endif
    LPORT_XLMAC_RX_CTRL_RSVD_2_FIELD_MASK,
    0,
    LPORT_XLMAC_RX_CTRL_RSVD_2_FIELD_WIDTH,
    LPORT_XLMAC_RX_CTRL_RSVD_2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_RX_CTRL_RUNT_THRESHOLD
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_RX_CTRL_RUNT_THRESHOLD_FIELD =
{
    "RUNT_THRESHOLD",
#if RU_INCLUDE_DESC
    "",
    "The runt threshold, below which the packets are dropped (CDC mode) or marked as runt (Low latency mode). Should be programmed < 96 bytes (decimal)",
#endif
    LPORT_XLMAC_RX_CTRL_RUNT_THRESHOLD_FIELD_MASK,
    0,
    LPORT_XLMAC_RX_CTRL_RUNT_THRESHOLD_FIELD_WIDTH,
    LPORT_XLMAC_RX_CTRL_RUNT_THRESHOLD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_RX_CTRL_STRICT_PREAMBLE
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_RX_CTRL_STRICT_PREAMBLE_FIELD =
{
    "STRICT_PREAMBLE",
#if RU_INCLUDE_DESC
    "",
    "If set, MAC checks for IEEE Ethernet preamble - K.SOP +  6 \"0x55\" preamble bytes + \"0xD5\" SFD character - if this sequence is missing it is treated as an errored packet",
#endif
    LPORT_XLMAC_RX_CTRL_STRICT_PREAMBLE_FIELD_MASK,
    0,
    LPORT_XLMAC_RX_CTRL_STRICT_PREAMBLE_FIELD_WIDTH,
    LPORT_XLMAC_RX_CTRL_STRICT_PREAMBLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_RX_CTRL_STRIP_CRC
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_RX_CTRL_STRIP_CRC_FIELD =
{
    "STRIP_CRC",
#if RU_INCLUDE_DESC
    "",
    "If set, CRC is stripped from the received packet",
#endif
    LPORT_XLMAC_RX_CTRL_STRIP_CRC_FIELD_MASK,
    0,
    LPORT_XLMAC_RX_CTRL_STRIP_CRC_FIELD_WIDTH,
    LPORT_XLMAC_RX_CTRL_STRIP_CRC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_RX_CTRL_RX_ANY_START
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_RX_CTRL_RX_ANY_START_FIELD =
{
    "RX_ANY_START",
#if RU_INCLUDE_DESC
    "",
    "If set, MAC allows any undefined control character to start a packet",
#endif
    LPORT_XLMAC_RX_CTRL_RX_ANY_START_FIELD_MASK,
    0,
    LPORT_XLMAC_RX_CTRL_RX_ANY_START_FIELD_WIDTH,
    LPORT_XLMAC_RX_CTRL_RX_ANY_START_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_RX_CTRL_RSVD_1
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_RX_CTRL_RSVD_1_FIELD =
{
    "RSVD_1",
#if RU_INCLUDE_DESC
    "",
    "Reserved",
#endif
    LPORT_XLMAC_RX_CTRL_RSVD_1_FIELD_MASK,
    0,
    LPORT_XLMAC_RX_CTRL_RSVD_1_FIELD_WIDTH,
    LPORT_XLMAC_RX_CTRL_RSVD_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_RX_MAC_SA_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_RX_MAC_SA_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_RX_MAC_SA_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_RX_MAC_SA_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_RX_MAC_SA_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_RX_MAC_SA_RX_SA
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_RX_MAC_SA_RX_SA_FIELD =
{
    "RX_SA",
#if RU_INCLUDE_DESC
    "",
    "Source Address recognized for MAC control packets in addition to the standard 0x0180C2000001",
#endif
    LPORT_XLMAC_RX_MAC_SA_RX_SA_FIELD_MASK,
    0,
    LPORT_XLMAC_RX_MAC_SA_RX_SA_FIELD_WIDTH,
    LPORT_XLMAC_RX_MAC_SA_RX_SA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_RX_MAC_SA_OVERLAY_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_RX_MAC_SA_OVERLAY_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_RX_MAC_SA_OVERLAY_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_RX_MAC_SA_OVERLAY_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_RX_MAC_SA_OVERLAY_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_RX_MAC_SA_OVERLAY_SA_HI
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_RX_MAC_SA_OVERLAY_SA_HI_FIELD =
{
    "SA_HI",
#if RU_INCLUDE_DESC
    "",
    "16 upper bits of the receive SA",
#endif
    LPORT_XLMAC_RX_MAC_SA_OVERLAY_SA_HI_FIELD_MASK,
    0,
    LPORT_XLMAC_RX_MAC_SA_OVERLAY_SA_HI_FIELD_WIDTH,
    LPORT_XLMAC_RX_MAC_SA_OVERLAY_SA_HI_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_RX_MAC_SA_OVERLAY_SA_LO
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_RX_MAC_SA_OVERLAY_SA_LO_FIELD =
{
    "SA_LO",
#if RU_INCLUDE_DESC
    "",
    "32 lower bits of the receive SA",
#endif
    LPORT_XLMAC_RX_MAC_SA_OVERLAY_SA_LO_FIELD_MASK,
    0,
    LPORT_XLMAC_RX_MAC_SA_OVERLAY_SA_LO_FIELD_WIDTH,
    LPORT_XLMAC_RX_MAC_SA_OVERLAY_SA_LO_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_RX_MAX_SIZE_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_RX_MAX_SIZE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_RX_MAX_SIZE_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_RX_MAX_SIZE_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_RX_MAX_SIZE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_RX_MAX_SIZE_RX_MAX_SIZE
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_RX_MAX_SIZE_RX_MAX_SIZE_FIELD =
{
    "RX_MAX_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Maximum packet size in receive direction, exclusive of preamble & CRC in strip mode. Packets greater than this size are truncated to this value.",
#endif
    LPORT_XLMAC_RX_MAX_SIZE_RX_MAX_SIZE_FIELD_MASK,
    0,
    LPORT_XLMAC_RX_MAX_SIZE_RX_MAX_SIZE_FIELD_WIDTH,
    LPORT_XLMAC_RX_MAX_SIZE_RX_MAX_SIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_RX_VLAN_TAG_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_RX_VLAN_TAG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_RX_VLAN_TAG_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_RX_VLAN_TAG_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_RX_VLAN_TAG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_RX_VLAN_TAG_OUTER_VLAN_TAG_ENABLE
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_RX_VLAN_TAG_OUTER_VLAN_TAG_ENABLE_FIELD =
{
    "OUTER_VLAN_TAG_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "If set, MAC enables VLAN tag detection using the OUTER_VLAN_TAG",
#endif
    LPORT_XLMAC_RX_VLAN_TAG_OUTER_VLAN_TAG_ENABLE_FIELD_MASK,
    0,
    LPORT_XLMAC_RX_VLAN_TAG_OUTER_VLAN_TAG_ENABLE_FIELD_WIDTH,
    LPORT_XLMAC_RX_VLAN_TAG_OUTER_VLAN_TAG_ENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_RX_VLAN_TAG_INNER_VLAN_TAG_ENABLE
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_RX_VLAN_TAG_INNER_VLAN_TAG_ENABLE_FIELD =
{
    "INNER_VLAN_TAG_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "If set, MAC enables VLAN tag detection using the INNER_VLAN_TAG",
#endif
    LPORT_XLMAC_RX_VLAN_TAG_INNER_VLAN_TAG_ENABLE_FIELD_MASK,
    0,
    LPORT_XLMAC_RX_VLAN_TAG_INNER_VLAN_TAG_ENABLE_FIELD_WIDTH,
    LPORT_XLMAC_RX_VLAN_TAG_INNER_VLAN_TAG_ENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_RX_VLAN_TAG_OUTER_VLAN_TAG
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_RX_VLAN_TAG_OUTER_VLAN_TAG_FIELD =
{
    "OUTER_VLAN_TAG",
#if RU_INCLUDE_DESC
    "",
    "TPID field for Outer VLAN tag",
#endif
    LPORT_XLMAC_RX_VLAN_TAG_OUTER_VLAN_TAG_FIELD_MASK,
    0,
    LPORT_XLMAC_RX_VLAN_TAG_OUTER_VLAN_TAG_FIELD_WIDTH,
    LPORT_XLMAC_RX_VLAN_TAG_OUTER_VLAN_TAG_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_RX_VLAN_TAG_INNER_VLAN_TAG
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_RX_VLAN_TAG_INNER_VLAN_TAG_FIELD =
{
    "INNER_VLAN_TAG",
#if RU_INCLUDE_DESC
    "",
    "TPID field for Inner VLAN tag",
#endif
    LPORT_XLMAC_RX_VLAN_TAG_INNER_VLAN_TAG_FIELD_MASK,
    0,
    LPORT_XLMAC_RX_VLAN_TAG_INNER_VLAN_TAG_FIELD_WIDTH,
    LPORT_XLMAC_RX_VLAN_TAG_INNER_VLAN_TAG_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_RX_LSS_CTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_RX_LSS_CTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_RX_LSS_CTRL_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_RX_LSS_CTRL_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_RX_LSS_CTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_RX_LSS_CTRL_RESET_FLOW_CONTROL_TIMERS_ON_LINK_DOWN
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_RX_LSS_CTRL_RESET_FLOW_CONTROL_TIMERS_ON_LINK_DOWN_FIELD =
{
    "RESET_FLOW_CONTROL_TIMERS_ON_LINK_DOWN",
#if RU_INCLUDE_DESC
    "",
    "If set, the Receive Pause, PFC & LLFC timers are reset whenever the link status is down, or local or remote faults are received.",
#endif
    LPORT_XLMAC_RX_LSS_CTRL_RESET_FLOW_CONTROL_TIMERS_ON_LINK_DOWN_FIELD_MASK,
    0,
    LPORT_XLMAC_RX_LSS_CTRL_RESET_FLOW_CONTROL_TIMERS_ON_LINK_DOWN_FIELD_WIDTH,
    LPORT_XLMAC_RX_LSS_CTRL_RESET_FLOW_CONTROL_TIMERS_ON_LINK_DOWN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_RX_LSS_CTRL_DROP_TX_DATA_ON_LINK_INTERRUPT
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_RX_LSS_CTRL_DROP_TX_DATA_ON_LINK_INTERRUPT_FIELD =
{
    "DROP_TX_DATA_ON_LINK_INTERRUPT",
#if RU_INCLUDE_DESC
    "",
    "This bit determines the way MAC handles data during link interruption state, if LINK_INTERRUPTION_DISABLE is reset.\n"
    "If set, during link interruption state, MAC drops transmit-data (statistics are updated) and sends IDLEs on the wire.\n"
    "If reset, transmit data is stalled in the internal FIFO under link interruption state.",
#endif
    LPORT_XLMAC_RX_LSS_CTRL_DROP_TX_DATA_ON_LINK_INTERRUPT_FIELD_MASK,
    0,
    LPORT_XLMAC_RX_LSS_CTRL_DROP_TX_DATA_ON_LINK_INTERRUPT_FIELD_WIDTH,
    LPORT_XLMAC_RX_LSS_CTRL_DROP_TX_DATA_ON_LINK_INTERRUPT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_RX_LSS_CTRL_DROP_TX_DATA_ON_REMOTE_FAULT
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_RX_LSS_CTRL_DROP_TX_DATA_ON_REMOTE_FAULT_FIELD =
{
    "DROP_TX_DATA_ON_REMOTE_FAULT",
#if RU_INCLUDE_DESC
    "",
    "This bit determines the way MAC handles data during remote fault state, if REMOTE_FAULT_DISABLE is reset.\n"
    "If set, during remote fault state, MAC drops transmit-data (statistics are updated) and sends IDLEs on the wire.\n"
    "If reset, transmit data is stalled in the internal FIFO under remote fault state.",
#endif
    LPORT_XLMAC_RX_LSS_CTRL_DROP_TX_DATA_ON_REMOTE_FAULT_FIELD_MASK,
    0,
    LPORT_XLMAC_RX_LSS_CTRL_DROP_TX_DATA_ON_REMOTE_FAULT_FIELD_WIDTH,
    LPORT_XLMAC_RX_LSS_CTRL_DROP_TX_DATA_ON_REMOTE_FAULT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_RX_LSS_CTRL_DROP_TX_DATA_ON_LOCAL_FAULT
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_RX_LSS_CTRL_DROP_TX_DATA_ON_LOCAL_FAULT_FIELD =
{
    "DROP_TX_DATA_ON_LOCAL_FAULT",
#if RU_INCLUDE_DESC
    "",
    "This bit determines the way MAC handles data during local fault state, if LOCAL_FAULT_DISABLE is reset.\n"
    "If set, during local fault state, MAC drops transmit-data (statistics are updated) and sends remote faults on the wire.\n"
    "If reset, transmit data is stalled in the internal FIFO under local fault state.",
#endif
    LPORT_XLMAC_RX_LSS_CTRL_DROP_TX_DATA_ON_LOCAL_FAULT_FIELD_MASK,
    0,
    LPORT_XLMAC_RX_LSS_CTRL_DROP_TX_DATA_ON_LOCAL_FAULT_FIELD_WIDTH,
    LPORT_XLMAC_RX_LSS_CTRL_DROP_TX_DATA_ON_LOCAL_FAULT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_RX_LSS_CTRL_LINK_INTERRUPTION_DISABLE
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_RX_LSS_CTRL_LINK_INTERRUPTION_DISABLE_FIELD =
{
    "LINK_INTERRUPTION_DISABLE",
#if RU_INCLUDE_DESC
    "",
    "This bit determines the transmit response during link interruption state. The LINK_INTERRUPTION_STATUS bit is always updated irrespective of this configuration.\n"
    "If set, MAC will continue to transmit data irrespective of LINK_INTERRUPTION_STATUS.\n"
    "If reset, MAC transmit behavior is governed by DROP_TX_DATA_ON_LINK_INTERRUPT configuration.",
#endif
    LPORT_XLMAC_RX_LSS_CTRL_LINK_INTERRUPTION_DISABLE_FIELD_MASK,
    0,
    LPORT_XLMAC_RX_LSS_CTRL_LINK_INTERRUPTION_DISABLE_FIELD_WIDTH,
    LPORT_XLMAC_RX_LSS_CTRL_LINK_INTERRUPTION_DISABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_RX_LSS_CTRL_USE_EXTERNAL_FAULTS_FOR_TX
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_RX_LSS_CTRL_USE_EXTERNAL_FAULTS_FOR_TX_FIELD =
{
    "USE_EXTERNAL_FAULTS_FOR_TX",
#if RU_INCLUDE_DESC
    "",
    "If set, the transmit fault responses are determined from input pins rather than internal receive status. \n"
    "In this mode, input fault from pins (from a peer MAC) is directly relayed on the transmit side of this MAC.\n"
    "See specification document for more details.",
#endif
    LPORT_XLMAC_RX_LSS_CTRL_USE_EXTERNAL_FAULTS_FOR_TX_FIELD_MASK,
    0,
    LPORT_XLMAC_RX_LSS_CTRL_USE_EXTERNAL_FAULTS_FOR_TX_FIELD_WIDTH,
    LPORT_XLMAC_RX_LSS_CTRL_USE_EXTERNAL_FAULTS_FOR_TX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_RX_LSS_CTRL_REMOTE_FAULT_DISABLE
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_RX_LSS_CTRL_REMOTE_FAULT_DISABLE_FIELD =
{
    "REMOTE_FAULT_DISABLE",
#if RU_INCLUDE_DESC
    "",
    "This bit determines the transmit response during remote fault state. The REMOTE_FAULT_STATUS bit is always updated irrespective of this configuration.\n"
    "If set, MAC will continue to transmit data irrespective of REMOTE_FAULT_STATUS.\n"
    "If reset, MAC transmit behavior is governed by DROP_TX_DATA_ON_REMOTE_FAULT configuration.",
#endif
    LPORT_XLMAC_RX_LSS_CTRL_REMOTE_FAULT_DISABLE_FIELD_MASK,
    0,
    LPORT_XLMAC_RX_LSS_CTRL_REMOTE_FAULT_DISABLE_FIELD_WIDTH,
    LPORT_XLMAC_RX_LSS_CTRL_REMOTE_FAULT_DISABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_RX_LSS_CTRL_LOCAL_FAULT_DISABLE
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_RX_LSS_CTRL_LOCAL_FAULT_DISABLE_FIELD =
{
    "LOCAL_FAULT_DISABLE",
#if RU_INCLUDE_DESC
    "",
    "This bit determines the transmit response during local fault state. The LOCAL_FAULT_STATUS bit is always updated irrespective of this configuration.\n"
    "If set, MAC will continue to transmit data irrespective of LOCAL_FAULT_STATUS.\n"
    "If reset, MAC transmit behavior is governed by DROP_TX_DATA_ON_LOCAL_FAULT configuration.",
#endif
    LPORT_XLMAC_RX_LSS_CTRL_LOCAL_FAULT_DISABLE_FIELD_MASK,
    0,
    LPORT_XLMAC_RX_LSS_CTRL_LOCAL_FAULT_DISABLE_FIELD_WIDTH,
    LPORT_XLMAC_RX_LSS_CTRL_LOCAL_FAULT_DISABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_RX_LSS_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_RX_LSS_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_RX_LSS_STATUS_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_RX_LSS_STATUS_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_RX_LSS_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_RX_LSS_STATUS_LINK_INTERRUPTION_STATUS
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_RX_LSS_STATUS_LINK_INTERRUPTION_STATUS_FIELD =
{
    "LINK_INTERRUPTION_STATUS",
#if RU_INCLUDE_DESC
    "",
    "True when link interruption state is detected as per RS layer state machine. Sticky bit is cleared by CLEAR_LINK_INTERRUPTION_STATUS.",
#endif
    LPORT_XLMAC_RX_LSS_STATUS_LINK_INTERRUPTION_STATUS_FIELD_MASK,
    0,
    LPORT_XLMAC_RX_LSS_STATUS_LINK_INTERRUPTION_STATUS_FIELD_WIDTH,
    LPORT_XLMAC_RX_LSS_STATUS_LINK_INTERRUPTION_STATUS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_RX_LSS_STATUS_REMOTE_FAULT_STATUS
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_RX_LSS_STATUS_REMOTE_FAULT_STATUS_FIELD =
{
    "REMOTE_FAULT_STATUS",
#if RU_INCLUDE_DESC
    "",
    "True when remote fault state is detected as per RS layer state machine. Sticky bit is cleared by CLEAR_REMOTE_FAULT_STATUS.",
#endif
    LPORT_XLMAC_RX_LSS_STATUS_REMOTE_FAULT_STATUS_FIELD_MASK,
    0,
    LPORT_XLMAC_RX_LSS_STATUS_REMOTE_FAULT_STATUS_FIELD_WIDTH,
    LPORT_XLMAC_RX_LSS_STATUS_REMOTE_FAULT_STATUS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_RX_LSS_STATUS_LOCAL_FAULT_STATUS
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_RX_LSS_STATUS_LOCAL_FAULT_STATUS_FIELD =
{
    "LOCAL_FAULT_STATUS",
#if RU_INCLUDE_DESC
    "",
    "True when local fault state is detected as per RS layer state machine. Sticky bit is cleared by CLEAR_LOCAL_FAULT_STATUS",
#endif
    LPORT_XLMAC_RX_LSS_STATUS_LOCAL_FAULT_STATUS_FIELD_MASK,
    0,
    LPORT_XLMAC_RX_LSS_STATUS_LOCAL_FAULT_STATUS_FIELD_WIDTH,
    LPORT_XLMAC_RX_LSS_STATUS_LOCAL_FAULT_STATUS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_CLEAR_RX_LSS_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_CLEAR_RX_LSS_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_CLEAR_RX_LSS_STATUS_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_CLEAR_RX_LSS_STATUS_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_CLEAR_RX_LSS_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_CLEAR_RX_LSS_STATUS_CLEAR_LINK_INTERRUPTION_STATUS
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_CLEAR_RX_LSS_STATUS_CLEAR_LINK_INTERRUPTION_STATUS_FIELD =
{
    "CLEAR_LINK_INTERRUPTION_STATUS",
#if RU_INCLUDE_DESC
    "",
    "A rising edge on this register bit (0->1), clears the sticky LINK_INTERRUPTION_STATUS bit",
#endif
    LPORT_XLMAC_CLEAR_RX_LSS_STATUS_CLEAR_LINK_INTERRUPTION_STATUS_FIELD_MASK,
    0,
    LPORT_XLMAC_CLEAR_RX_LSS_STATUS_CLEAR_LINK_INTERRUPTION_STATUS_FIELD_WIDTH,
    LPORT_XLMAC_CLEAR_RX_LSS_STATUS_CLEAR_LINK_INTERRUPTION_STATUS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_CLEAR_RX_LSS_STATUS_CLEAR_REMOTE_FAULT_STATUS
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_CLEAR_RX_LSS_STATUS_CLEAR_REMOTE_FAULT_STATUS_FIELD =
{
    "CLEAR_REMOTE_FAULT_STATUS",
#if RU_INCLUDE_DESC
    "",
    "A rising edge on this register bit (0->1), clears the sticky REMOTE_FAULT_STATUS bit",
#endif
    LPORT_XLMAC_CLEAR_RX_LSS_STATUS_CLEAR_REMOTE_FAULT_STATUS_FIELD_MASK,
    0,
    LPORT_XLMAC_CLEAR_RX_LSS_STATUS_CLEAR_REMOTE_FAULT_STATUS_FIELD_WIDTH,
    LPORT_XLMAC_CLEAR_RX_LSS_STATUS_CLEAR_REMOTE_FAULT_STATUS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_CLEAR_RX_LSS_STATUS_CLEAR_LOCAL_FAULT_STATUS
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_CLEAR_RX_LSS_STATUS_CLEAR_LOCAL_FAULT_STATUS_FIELD =
{
    "CLEAR_LOCAL_FAULT_STATUS",
#if RU_INCLUDE_DESC
    "",
    "A rising edge on this register bit (0->1), clears the sticky LOCAL_FAULT_STATUS bit",
#endif
    LPORT_XLMAC_CLEAR_RX_LSS_STATUS_CLEAR_LOCAL_FAULT_STATUS_FIELD_MASK,
    0,
    LPORT_XLMAC_CLEAR_RX_LSS_STATUS_CLEAR_LOCAL_FAULT_STATUS_FIELD_WIDTH,
    LPORT_XLMAC_CLEAR_RX_LSS_STATUS_CLEAR_LOCAL_FAULT_STATUS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_PAUSE_CTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_PAUSE_CTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_PAUSE_CTRL_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_PAUSE_CTRL_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_PAUSE_CTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_PAUSE_CTRL_PAUSE_XOFF_TIMER
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_PAUSE_CTRL_PAUSE_XOFF_TIMER_FIELD =
{
    "PAUSE_XOFF_TIMER",
#if RU_INCLUDE_DESC
    "",
    "Pause time value sent in the timer field for XOFF state (unit is 512 bit-times)",
#endif
    LPORT_XLMAC_PAUSE_CTRL_PAUSE_XOFF_TIMER_FIELD_MASK,
    0,
    LPORT_XLMAC_PAUSE_CTRL_PAUSE_XOFF_TIMER_FIELD_WIDTH,
    LPORT_XLMAC_PAUSE_CTRL_PAUSE_XOFF_TIMER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_PAUSE_CTRL_RSVD_2
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_PAUSE_CTRL_RSVD_2_FIELD =
{
    "RSVD_2",
#if RU_INCLUDE_DESC
    "",
    "Reserved",
#endif
    LPORT_XLMAC_PAUSE_CTRL_RSVD_2_FIELD_MASK,
    0,
    LPORT_XLMAC_PAUSE_CTRL_RSVD_2_FIELD_WIDTH,
    LPORT_XLMAC_PAUSE_CTRL_RSVD_2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_PAUSE_CTRL_RSVD_1
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_PAUSE_CTRL_RSVD_1_FIELD =
{
    "RSVD_1",
#if RU_INCLUDE_DESC
    "",
    "Reserved",
#endif
    LPORT_XLMAC_PAUSE_CTRL_RSVD_1_FIELD_MASK,
    0,
    LPORT_XLMAC_PAUSE_CTRL_RSVD_1_FIELD_WIDTH,
    LPORT_XLMAC_PAUSE_CTRL_RSVD_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_PAUSE_CTRL_RX_PAUSE_EN
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_PAUSE_CTRL_RX_PAUSE_EN_FIELD =
{
    "RX_PAUSE_EN",
#if RU_INCLUDE_DESC
    "",
    "When set, enables detection of pause frames in the receive direction and pause/resume the transmit data path",
#endif
    LPORT_XLMAC_PAUSE_CTRL_RX_PAUSE_EN_FIELD_MASK,
    0,
    LPORT_XLMAC_PAUSE_CTRL_RX_PAUSE_EN_FIELD_WIDTH,
    LPORT_XLMAC_PAUSE_CTRL_RX_PAUSE_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_PAUSE_CTRL_TX_PAUSE_EN
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_PAUSE_CTRL_TX_PAUSE_EN_FIELD =
{
    "TX_PAUSE_EN",
#if RU_INCLUDE_DESC
    "",
    "When set, enables the transmission of pause frames whenever there is a transition on txbkp input to MAC from MMU",
#endif
    LPORT_XLMAC_PAUSE_CTRL_TX_PAUSE_EN_FIELD_MASK,
    0,
    LPORT_XLMAC_PAUSE_CTRL_TX_PAUSE_EN_FIELD_WIDTH,
    LPORT_XLMAC_PAUSE_CTRL_TX_PAUSE_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_PAUSE_CTRL_PAUSE_REFRESH_EN
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_PAUSE_CTRL_PAUSE_REFRESH_EN_FIELD =
{
    "PAUSE_REFRESH_EN",
#if RU_INCLUDE_DESC
    "",
    "When set, enables the periodic re-generation of XOFF pause frames based on the interval specified in PAUSE_REFRESH_TIMER",
#endif
    LPORT_XLMAC_PAUSE_CTRL_PAUSE_REFRESH_EN_FIELD_MASK,
    0,
    LPORT_XLMAC_PAUSE_CTRL_PAUSE_REFRESH_EN_FIELD_WIDTH,
    LPORT_XLMAC_PAUSE_CTRL_PAUSE_REFRESH_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_PAUSE_CTRL_PAUSE_REFRESH_TIMER
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_PAUSE_CTRL_PAUSE_REFRESH_TIMER_FIELD =
{
    "PAUSE_REFRESH_TIMER",
#if RU_INCLUDE_DESC
    "",
    "This field specifies the interval at which pause frames are re-generated during XOFF state, provided PAUSE_REFRESH_EN is set (unit is 512 bit-times)",
#endif
    LPORT_XLMAC_PAUSE_CTRL_PAUSE_REFRESH_TIMER_FIELD_MASK,
    0,
    LPORT_XLMAC_PAUSE_CTRL_PAUSE_REFRESH_TIMER_FIELD_WIDTH,
    LPORT_XLMAC_PAUSE_CTRL_PAUSE_REFRESH_TIMER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_PAUSE_CTRL_OVERLAY_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_PAUSE_CTRL_OVERLAY_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_PAUSE_CTRL_OVERLAY_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_PAUSE_CTRL_OVERLAY_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_PAUSE_CTRL_OVERLAY_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_PAUSE_CTRL_OVERLAY_XLMAC_PAUSE_CTRL_HI
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_PAUSE_CTRL_OVERLAY_XLMAC_PAUSE_CTRL_HI_FIELD =
{
    "XLMAC_PAUSE_CTRL_HI",
#if RU_INCLUDE_DESC
    "",
    "5 upper bits of the XLMAC Pause CTRL register",
#endif
    LPORT_XLMAC_PAUSE_CTRL_OVERLAY_XLMAC_PAUSE_CTRL_HI_FIELD_MASK,
    0,
    LPORT_XLMAC_PAUSE_CTRL_OVERLAY_XLMAC_PAUSE_CTRL_HI_FIELD_WIDTH,
    LPORT_XLMAC_PAUSE_CTRL_OVERLAY_XLMAC_PAUSE_CTRL_HI_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_PAUSE_CTRL_OVERLAY_XLMAC_PAUSE_CTRL_LO
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_PAUSE_CTRL_OVERLAY_XLMAC_PAUSE_CTRL_LO_FIELD =
{
    "XLMAC_PAUSE_CTRL_LO",
#if RU_INCLUDE_DESC
    "",
    "32 lower bits of the XLMAC Pause CTRL register",
#endif
    LPORT_XLMAC_PAUSE_CTRL_OVERLAY_XLMAC_PAUSE_CTRL_LO_FIELD_MASK,
    0,
    LPORT_XLMAC_PAUSE_CTRL_OVERLAY_XLMAC_PAUSE_CTRL_LO_FIELD_WIDTH,
    LPORT_XLMAC_PAUSE_CTRL_OVERLAY_XLMAC_PAUSE_CTRL_LO_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_PFC_CTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_PFC_CTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_PFC_CTRL_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_PFC_CTRL_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_PFC_CTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_PFC_CTRL_TX_PFC_EN
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_PFC_CTRL_TX_PFC_EN_FIELD =
{
    "TX_PFC_EN",
#if RU_INCLUDE_DESC
    "",
    "When set, enables the transmission of PFC frames",
#endif
    LPORT_XLMAC_PFC_CTRL_TX_PFC_EN_FIELD_MASK,
    0,
    LPORT_XLMAC_PFC_CTRL_TX_PFC_EN_FIELD_WIDTH,
    LPORT_XLMAC_PFC_CTRL_TX_PFC_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_PFC_CTRL_RX_PFC_EN
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_PFC_CTRL_RX_PFC_EN_FIELD =
{
    "RX_PFC_EN",
#if RU_INCLUDE_DESC
    "",
    "When set, enables detection of PFC frames in the receive direction and generation of COSMAPs to MMU based on incoming timer values",
#endif
    LPORT_XLMAC_PFC_CTRL_RX_PFC_EN_FIELD_MASK,
    0,
    LPORT_XLMAC_PFC_CTRL_RX_PFC_EN_FIELD_WIDTH,
    LPORT_XLMAC_PFC_CTRL_RX_PFC_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_PFC_CTRL_PFC_STATS_EN
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_PFC_CTRL_PFC_STATS_EN_FIELD =
{
    "PFC_STATS_EN",
#if RU_INCLUDE_DESC
    "",
    "When set, enables the generation of receive and transmit PFC events into the corresponding statistics vectors (RSV and TSV)",
#endif
    LPORT_XLMAC_PFC_CTRL_PFC_STATS_EN_FIELD_MASK,
    0,
    LPORT_XLMAC_PFC_CTRL_PFC_STATS_EN_FIELD_WIDTH,
    LPORT_XLMAC_PFC_CTRL_PFC_STATS_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_PFC_CTRL_RSVD
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_PFC_CTRL_RSVD_FIELD =
{
    "RSVD",
#if RU_INCLUDE_DESC
    "",
    "Reserved",
#endif
    LPORT_XLMAC_PFC_CTRL_RSVD_FIELD_MASK,
    0,
    LPORT_XLMAC_PFC_CTRL_RSVD_FIELD_WIDTH,
    LPORT_XLMAC_PFC_CTRL_RSVD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_PFC_CTRL_FORCE_PFC_XON
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_PFC_CTRL_FORCE_PFC_XON_FIELD =
{
    "FORCE_PFC_XON",
#if RU_INCLUDE_DESC
    "",
    "When set, forces the MAC to generate an XON indication to the MMU for all classes of service in the receive direction",
#endif
    LPORT_XLMAC_PFC_CTRL_FORCE_PFC_XON_FIELD_MASK,
    0,
    LPORT_XLMAC_PFC_CTRL_FORCE_PFC_XON_FIELD_WIDTH,
    LPORT_XLMAC_PFC_CTRL_FORCE_PFC_XON_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_PFC_CTRL_PFC_REFRESH_EN
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_PFC_CTRL_PFC_REFRESH_EN_FIELD =
{
    "PFC_REFRESH_EN",
#if RU_INCLUDE_DESC
    "",
    "When set, enables the periodic re-generation of PFC frames based on the interval specified in PFC_REFRESH_TIMER",
#endif
    LPORT_XLMAC_PFC_CTRL_PFC_REFRESH_EN_FIELD_MASK,
    0,
    LPORT_XLMAC_PFC_CTRL_PFC_REFRESH_EN_FIELD_WIDTH,
    LPORT_XLMAC_PFC_CTRL_PFC_REFRESH_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_PFC_CTRL_PFC_XOFF_TIMER
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_PFC_CTRL_PFC_XOFF_TIMER_FIELD =
{
    "PFC_XOFF_TIMER",
#if RU_INCLUDE_DESC
    "",
    "Pause time value sent in the timer field for classes in XOFF state (unit is 512 bit-times)",
#endif
    LPORT_XLMAC_PFC_CTRL_PFC_XOFF_TIMER_FIELD_MASK,
    0,
    LPORT_XLMAC_PFC_CTRL_PFC_XOFF_TIMER_FIELD_WIDTH,
    LPORT_XLMAC_PFC_CTRL_PFC_XOFF_TIMER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_PFC_CTRL_PFC_REFRESH_TIMER
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_PFC_CTRL_PFC_REFRESH_TIMER_FIELD =
{
    "PFC_REFRESH_TIMER",
#if RU_INCLUDE_DESC
    "",
    "This field specifies the interval at which PFC frames are re-generated for a class of service in XOFF state, provided PFC_REFRESH_EN is set (unit is 512 bit-times)",
#endif
    LPORT_XLMAC_PFC_CTRL_PFC_REFRESH_TIMER_FIELD_MASK,
    0,
    LPORT_XLMAC_PFC_CTRL_PFC_REFRESH_TIMER_FIELD_WIDTH,
    LPORT_XLMAC_PFC_CTRL_PFC_REFRESH_TIMER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_PFC_CTRL_OVERLAY_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_PFC_CTRL_OVERLAY_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_PFC_CTRL_OVERLAY_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_PFC_CTRL_OVERLAY_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_PFC_CTRL_OVERLAY_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_PFC_CTRL_OVERLAY_LLFC_REFRESH_EN
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_PFC_CTRL_OVERLAY_LLFC_REFRESH_EN_FIELD =
{
    "LLFC_REFRESH_EN",
#if RU_INCLUDE_DESC
    "",
    "When set, enables the periodic re-generation of LLFC frames based on the interval specified in LLFC_REFRESH_TIMER",
#endif
    LPORT_XLMAC_PFC_CTRL_OVERLAY_LLFC_REFRESH_EN_FIELD_MASK,
    0,
    LPORT_XLMAC_PFC_CTRL_OVERLAY_LLFC_REFRESH_EN_FIELD_WIDTH,
    LPORT_XLMAC_PFC_CTRL_OVERLAY_LLFC_REFRESH_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_PFC_CTRL_OVERLAY_RESERVED1
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_PFC_CTRL_OVERLAY_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_PFC_CTRL_OVERLAY_RESERVED1_FIELD_MASK,
    0,
    LPORT_XLMAC_PFC_CTRL_OVERLAY_RESERVED1_FIELD_WIDTH,
    LPORT_XLMAC_PFC_CTRL_OVERLAY_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_PFC_CTRL_OVERLAY_LLFC_REFRESH_TIMER
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_PFC_CTRL_OVERLAY_LLFC_REFRESH_TIMER_FIELD =
{
    "LLFC_REFRESH_TIMER",
#if RU_INCLUDE_DESC
    "",
    "This field specifies the interval at which LLFC frames are re-generated for a class of service in XOFF state, provided LLFC_REFRESH_EN is set (unit is 512 bit-times)",
#endif
    LPORT_XLMAC_PFC_CTRL_OVERLAY_LLFC_REFRESH_TIMER_FIELD_MASK,
    0,
    LPORT_XLMAC_PFC_CTRL_OVERLAY_LLFC_REFRESH_TIMER_FIELD_WIDTH,
    LPORT_XLMAC_PFC_CTRL_OVERLAY_LLFC_REFRESH_TIMER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_PFC_TYPE_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_PFC_TYPE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_PFC_TYPE_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_PFC_TYPE_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_PFC_TYPE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_PFC_TYPE_PFC_ETH_TYPE
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_PFC_TYPE_PFC_ETH_TYPE_FIELD =
{
    "PFC_ETH_TYPE",
#if RU_INCLUDE_DESC
    "",
    "This field is used in the ETHERTYPE field of the PFC frame that is generated and transmitted by the MAC and also used for detection in the receive direction",
#endif
    LPORT_XLMAC_PFC_TYPE_PFC_ETH_TYPE_FIELD_MASK,
    0,
    LPORT_XLMAC_PFC_TYPE_PFC_ETH_TYPE_FIELD_WIDTH,
    LPORT_XLMAC_PFC_TYPE_PFC_ETH_TYPE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_PFC_OPCODE_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_PFC_OPCODE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_PFC_OPCODE_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_PFC_OPCODE_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_PFC_OPCODE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_PFC_OPCODE_PFC_OPCODE
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_PFC_OPCODE_PFC_OPCODE_FIELD =
{
    "PFC_OPCODE",
#if RU_INCLUDE_DESC
    "",
    "This field is used in the OPCODE field of the PFC frame that is generated and transmitted by the MAC and also used for detection in the receive direction",
#endif
    LPORT_XLMAC_PFC_OPCODE_PFC_OPCODE_FIELD_MASK,
    0,
    LPORT_XLMAC_PFC_OPCODE_PFC_OPCODE_FIELD_WIDTH,
    LPORT_XLMAC_PFC_OPCODE_PFC_OPCODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_PFC_DA_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_PFC_DA_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_PFC_DA_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_PFC_DA_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_PFC_DA_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_PFC_DA_PFC_MACDA
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_PFC_DA_PFC_MACDA_FIELD =
{
    "PFC_MACDA",
#if RU_INCLUDE_DESC
    "",
    "This field is used in the destination-address field of the PFC frame that is generated and transmitted by the MAC and also used for detection in the receive direction",
#endif
    LPORT_XLMAC_PFC_DA_PFC_MACDA_FIELD_MASK,
    0,
    LPORT_XLMAC_PFC_DA_PFC_MACDA_FIELD_WIDTH,
    LPORT_XLMAC_PFC_DA_PFC_MACDA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_PFC_DA_OVERLAY_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_PFC_DA_OVERLAY_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_PFC_DA_OVERLAY_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_PFC_DA_OVERLAY_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_PFC_DA_OVERLAY_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_PFC_DA_OVERLAY_PFC_MACDA_HI
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_PFC_DA_OVERLAY_PFC_MACDA_HI_FIELD =
{
    "PFC_MACDA_HI",
#if RU_INCLUDE_DESC
    "",
    "16 upper bits of the PFC DA",
#endif
    LPORT_XLMAC_PFC_DA_OVERLAY_PFC_MACDA_HI_FIELD_MASK,
    0,
    LPORT_XLMAC_PFC_DA_OVERLAY_PFC_MACDA_HI_FIELD_WIDTH,
    LPORT_XLMAC_PFC_DA_OVERLAY_PFC_MACDA_HI_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_PFC_DA_OVERLAY_PFC_MACDA_LO
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_PFC_DA_OVERLAY_PFC_MACDA_LO_FIELD =
{
    "PFC_MACDA_LO",
#if RU_INCLUDE_DESC
    "",
    "32 lower bits of the PFC DA",
#endif
    LPORT_XLMAC_PFC_DA_OVERLAY_PFC_MACDA_LO_FIELD_MASK,
    0,
    LPORT_XLMAC_PFC_DA_OVERLAY_PFC_MACDA_LO_FIELD_WIDTH,
    LPORT_XLMAC_PFC_DA_OVERLAY_PFC_MACDA_LO_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_LLFC_CTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_LLFC_CTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_LLFC_CTRL_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_LLFC_CTRL_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_LLFC_CTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_LLFC_CTRL_LLFC_IMG
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_LLFC_CTRL_LLFC_IMG_FIELD =
{
    "LLFC_IMG",
#if RU_INCLUDE_DESC
    "",
    "This field indicates the minimum Inter Message Gap that is enforced by the MAC between 2 LLFC messages in the transmit direction (unit is 1 credit)",
#endif
    LPORT_XLMAC_LLFC_CTRL_LLFC_IMG_FIELD_MASK,
    0,
    LPORT_XLMAC_LLFC_CTRL_LLFC_IMG_FIELD_WIDTH,
    LPORT_XLMAC_LLFC_CTRL_LLFC_IMG_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_LLFC_CTRL_NO_SOM_FOR_CRC_LLFC
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_LLFC_CTRL_NO_SOM_FOR_CRC_LLFC_FIELD =
{
    "NO_SOM_FOR_CRC_LLFC",
#if RU_INCLUDE_DESC
    "",
    "When set, LLFC CRC computation does not include the SOM character",
#endif
    LPORT_XLMAC_LLFC_CTRL_NO_SOM_FOR_CRC_LLFC_FIELD_MASK,
    0,
    LPORT_XLMAC_LLFC_CTRL_NO_SOM_FOR_CRC_LLFC_FIELD_WIDTH,
    LPORT_XLMAC_LLFC_CTRL_NO_SOM_FOR_CRC_LLFC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_LLFC_CTRL_LLFC_CRC_IGNORE
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_LLFC_CTRL_LLFC_CRC_IGNORE_FIELD =
{
    "LLFC_CRC_IGNORE",
#if RU_INCLUDE_DESC
    "",
    "When set, disables the CRC check for LLFC messages in the receive direction",
#endif
    LPORT_XLMAC_LLFC_CTRL_LLFC_CRC_IGNORE_FIELD_MASK,
    0,
    LPORT_XLMAC_LLFC_CTRL_LLFC_CRC_IGNORE_FIELD_WIDTH,
    LPORT_XLMAC_LLFC_CTRL_LLFC_CRC_IGNORE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_LLFC_CTRL_LLFC_CUT_THROUGH_MODE
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_LLFC_CTRL_LLFC_CUT_THROUGH_MODE_FIELD =
{
    "LLFC_CUT_THROUGH_MODE",
#if RU_INCLUDE_DESC
    "",
    "When LLFC_IN_IPG_ONLY is reset, the mode of transmission of LLFC messages is controlled by this bit depending upon whether the LLFC message is XON or XOFF\n"
    "When LLFC_CUT_THROUGH_MODE is reset, all LLFC messages are transmitted pre-emptively (within a packet)\n"
    "When LLFC_CUT_THROUGH_MODE is set, only XOFF LLFC messages are transmitted pre-emptively, XON LLFC messages are transmitted during IPG",
#endif
    LPORT_XLMAC_LLFC_CTRL_LLFC_CUT_THROUGH_MODE_FIELD_MASK,
    0,
    LPORT_XLMAC_LLFC_CTRL_LLFC_CUT_THROUGH_MODE_FIELD_WIDTH,
    LPORT_XLMAC_LLFC_CTRL_LLFC_CUT_THROUGH_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_LLFC_CTRL_LLFC_IN_IPG_ONLY
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_LLFC_CTRL_LLFC_IN_IPG_ONLY_FIELD =
{
    "LLFC_IN_IPG_ONLY",
#if RU_INCLUDE_DESC
    "",
    "When set, all LLFC messages are transmitted during IPG\n"
    "When reset, the mode of insertion of LLFC messages is controlled by LLFC_CUT_THROUGH_MODE",
#endif
    LPORT_XLMAC_LLFC_CTRL_LLFC_IN_IPG_ONLY_FIELD_MASK,
    0,
    LPORT_XLMAC_LLFC_CTRL_LLFC_IN_IPG_ONLY_FIELD_WIDTH,
    LPORT_XLMAC_LLFC_CTRL_LLFC_IN_IPG_ONLY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_LLFC_CTRL_RX_LLFC_EN
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_LLFC_CTRL_RX_LLFC_EN_FIELD =
{
    "RX_LLFC_EN",
#if RU_INCLUDE_DESC
    "",
    "When set, enables processing of LLFC frames in the receive direction and generation of COSMAPs to MMU",
#endif
    LPORT_XLMAC_LLFC_CTRL_RX_LLFC_EN_FIELD_MASK,
    0,
    LPORT_XLMAC_LLFC_CTRL_RX_LLFC_EN_FIELD_WIDTH,
    LPORT_XLMAC_LLFC_CTRL_RX_LLFC_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_LLFC_CTRL_TX_LLFC_EN
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_LLFC_CTRL_TX_LLFC_EN_FIELD =
{
    "TX_LLFC_EN",
#if RU_INCLUDE_DESC
    "",
    "When set, enables the generation and transmission of LLFC frames in the transmit direction",
#endif
    LPORT_XLMAC_LLFC_CTRL_TX_LLFC_EN_FIELD_MASK,
    0,
    LPORT_XLMAC_LLFC_CTRL_TX_LLFC_EN_FIELD_WIDTH,
    LPORT_XLMAC_LLFC_CTRL_TX_LLFC_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TX_LLFC_MSG_FIELDS_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TX_LLFC_MSG_FIELDS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_TX_LLFC_MSG_FIELDS_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_TX_LLFC_MSG_FIELDS_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_TX_LLFC_MSG_FIELDS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TX_LLFC_MSG_FIELDS_LLFC_XOFF_TIME
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TX_LLFC_MSG_FIELDS_LLFC_XOFF_TIME_FIELD =
{
    "LLFC_XOFF_TIME",
#if RU_INCLUDE_DESC
    "",
    "Pause time value sent in the XOFF_TIME field of the outgoing LLFC message",
#endif
    LPORT_XLMAC_TX_LLFC_MSG_FIELDS_LLFC_XOFF_TIME_FIELD_MASK,
    0,
    LPORT_XLMAC_TX_LLFC_MSG_FIELDS_LLFC_XOFF_TIME_FIELD_WIDTH,
    LPORT_XLMAC_TX_LLFC_MSG_FIELDS_LLFC_XOFF_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TX_LLFC_MSG_FIELDS_TX_LLFC_FC_OBJ_LOGICAL
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TX_LLFC_MSG_FIELDS_TX_LLFC_FC_OBJ_LOGICAL_FIELD =
{
    "TX_LLFC_FC_OBJ_LOGICAL",
#if RU_INCLUDE_DESC
    "",
    "This field is used in the FC_OBJ_LOGICAL field of the outgoing LLFC message",
#endif
    LPORT_XLMAC_TX_LLFC_MSG_FIELDS_TX_LLFC_FC_OBJ_LOGICAL_FIELD_MASK,
    0,
    LPORT_XLMAC_TX_LLFC_MSG_FIELDS_TX_LLFC_FC_OBJ_LOGICAL_FIELD_WIDTH,
    LPORT_XLMAC_TX_LLFC_MSG_FIELDS_TX_LLFC_FC_OBJ_LOGICAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TX_LLFC_MSG_FIELDS_TX_LLFC_MSG_TYPE_LOGICAL
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TX_LLFC_MSG_FIELDS_TX_LLFC_MSG_TYPE_LOGICAL_FIELD =
{
    "TX_LLFC_MSG_TYPE_LOGICAL",
#if RU_INCLUDE_DESC
    "",
    "This field is used in the MSG_TYPE_LOGICAL field of the outgoing LLFC message",
#endif
    LPORT_XLMAC_TX_LLFC_MSG_FIELDS_TX_LLFC_MSG_TYPE_LOGICAL_FIELD_MASK,
    0,
    LPORT_XLMAC_TX_LLFC_MSG_FIELDS_TX_LLFC_MSG_TYPE_LOGICAL_FIELD_WIDTH,
    LPORT_XLMAC_TX_LLFC_MSG_FIELDS_TX_LLFC_MSG_TYPE_LOGICAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_RX_LLFC_MSG_FIELDS_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_RX_LLFC_MSG_FIELDS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_RX_LLFC_MSG_FIELDS_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_RX_LLFC_MSG_FIELDS_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_RX_LLFC_MSG_FIELDS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_RX_LLFC_MSG_FIELDS_RX_LLFC_FC_OBJ_PHYSICAL
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_RX_LLFC_MSG_FIELDS_RX_LLFC_FC_OBJ_PHYSICAL_FIELD =
{
    "RX_LLFC_FC_OBJ_PHYSICAL",
#if RU_INCLUDE_DESC
    "",
    "This value is compared against the FC_OBJ_PHYSICAL field of an incoming LLFC message in order to decode the message",
#endif
    LPORT_XLMAC_RX_LLFC_MSG_FIELDS_RX_LLFC_FC_OBJ_PHYSICAL_FIELD_MASK,
    0,
    LPORT_XLMAC_RX_LLFC_MSG_FIELDS_RX_LLFC_FC_OBJ_PHYSICAL_FIELD_WIDTH,
    LPORT_XLMAC_RX_LLFC_MSG_FIELDS_RX_LLFC_FC_OBJ_PHYSICAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_RX_LLFC_MSG_FIELDS_RX_LLFC_MSG_TYPE_PHYSICAL
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_RX_LLFC_MSG_FIELDS_RX_LLFC_MSG_TYPE_PHYSICAL_FIELD =
{
    "RX_LLFC_MSG_TYPE_PHYSICAL",
#if RU_INCLUDE_DESC
    "",
    "This value is compared against the MSG_TYPE_PHYSICAL field of an incoming LLFC message in order to decode the message",
#endif
    LPORT_XLMAC_RX_LLFC_MSG_FIELDS_RX_LLFC_MSG_TYPE_PHYSICAL_FIELD_MASK,
    0,
    LPORT_XLMAC_RX_LLFC_MSG_FIELDS_RX_LLFC_MSG_TYPE_PHYSICAL_FIELD_WIDTH,
    LPORT_XLMAC_RX_LLFC_MSG_FIELDS_RX_LLFC_MSG_TYPE_PHYSICAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_RX_LLFC_MSG_FIELDS_RX_LLFC_FC_OBJ_LOGICAL
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_RX_LLFC_MSG_FIELDS_RX_LLFC_FC_OBJ_LOGICAL_FIELD =
{
    "RX_LLFC_FC_OBJ_LOGICAL",
#if RU_INCLUDE_DESC
    "",
    "This value is compared against the FC_OBJ_LOGICAL field of an incoming LLFC message in order to decode the message",
#endif
    LPORT_XLMAC_RX_LLFC_MSG_FIELDS_RX_LLFC_FC_OBJ_LOGICAL_FIELD_MASK,
    0,
    LPORT_XLMAC_RX_LLFC_MSG_FIELDS_RX_LLFC_FC_OBJ_LOGICAL_FIELD_WIDTH,
    LPORT_XLMAC_RX_LLFC_MSG_FIELDS_RX_LLFC_FC_OBJ_LOGICAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_RX_LLFC_MSG_FIELDS_RX_LLFC_MSG_TYPE_LOGICAL
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_RX_LLFC_MSG_FIELDS_RX_LLFC_MSG_TYPE_LOGICAL_FIELD =
{
    "RX_LLFC_MSG_TYPE_LOGICAL",
#if RU_INCLUDE_DESC
    "",
    "This value is compared against the MSG_TYPE_LOGICAL field of an incoming LLFC message in order to decode the message",
#endif
    LPORT_XLMAC_RX_LLFC_MSG_FIELDS_RX_LLFC_MSG_TYPE_LOGICAL_FIELD_MASK,
    0,
    LPORT_XLMAC_RX_LLFC_MSG_FIELDS_RX_LLFC_MSG_TYPE_LOGICAL_FIELD_WIDTH,
    LPORT_XLMAC_RX_LLFC_MSG_FIELDS_RX_LLFC_MSG_TYPE_LOGICAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TX_TIMESTAMP_FIFO_DATA_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TX_TIMESTAMP_FIFO_DATA_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_TX_TIMESTAMP_FIFO_DATA_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_TX_TIMESTAMP_FIFO_DATA_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_TX_TIMESTAMP_FIFO_DATA_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TX_TIMESTAMP_FIFO_DATA_TS_ENTRY_VALID
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TX_TIMESTAMP_FIFO_DATA_TS_ENTRY_VALID_FIELD =
{
    "TS_ENTRY_VALID",
#if RU_INCLUDE_DESC
    "",
    "Active high qualifier for the TimeStamp & SEQUENCE_ID fields.",
#endif
    LPORT_XLMAC_TX_TIMESTAMP_FIFO_DATA_TS_ENTRY_VALID_FIELD_MASK,
    0,
    LPORT_XLMAC_TX_TIMESTAMP_FIFO_DATA_TS_ENTRY_VALID_FIELD_WIDTH,
    LPORT_XLMAC_TX_TIMESTAMP_FIFO_DATA_TS_ENTRY_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TX_TIMESTAMP_FIFO_DATA_SEQUENCE_ID
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TX_TIMESTAMP_FIFO_DATA_SEQUENCE_ID_FIELD =
{
    "SEQUENCE_ID",
#if RU_INCLUDE_DESC
    "",
    "The Sequence Identifier extracted from the Timesync packet based on the header offset",
#endif
    LPORT_XLMAC_TX_TIMESTAMP_FIFO_DATA_SEQUENCE_ID_FIELD_MASK,
    0,
    LPORT_XLMAC_TX_TIMESTAMP_FIFO_DATA_SEQUENCE_ID_FIELD_WIDTH,
    LPORT_XLMAC_TX_TIMESTAMP_FIFO_DATA_SEQUENCE_ID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TX_TIMESTAMP_FIFO_DATA_TIME_STAMP
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TX_TIMESTAMP_FIFO_DATA_TIME_STAMP_FIELD =
{
    "TIME_STAMP",
#if RU_INCLUDE_DESC
    "",
    "The TimeStamp value of the Tx two-step enabled packet.",
#endif
    LPORT_XLMAC_TX_TIMESTAMP_FIFO_DATA_TIME_STAMP_FIELD_MASK,
    0,
    LPORT_XLMAC_TX_TIMESTAMP_FIFO_DATA_TIME_STAMP_FIELD_WIDTH,
    LPORT_XLMAC_TX_TIMESTAMP_FIFO_DATA_TIME_STAMP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TX_TIMESTAMP_FIFO_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TX_TIMESTAMP_FIFO_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_TX_TIMESTAMP_FIFO_STATUS_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_TX_TIMESTAMP_FIFO_STATUS_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_TX_TIMESTAMP_FIFO_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TX_TIMESTAMP_FIFO_STATUS_ENTRY_COUNT
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TX_TIMESTAMP_FIFO_STATUS_ENTRY_COUNT_FIELD =
{
    "ENTRY_COUNT",
#if RU_INCLUDE_DESC
    "",
    "Number of TX time stamps currently buffered in TX Time Stamp FIFO. A valid entry is popped out whenever XLMAC_TX_TIMESTMAP_FIFO_DATA is read",
#endif
    LPORT_XLMAC_TX_TIMESTAMP_FIFO_STATUS_ENTRY_COUNT_FIELD_MASK,
    0,
    LPORT_XLMAC_TX_TIMESTAMP_FIFO_STATUS_ENTRY_COUNT_FIELD_WIDTH,
    LPORT_XLMAC_TX_TIMESTAMP_FIFO_STATUS_ENTRY_COUNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_FIFO_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_FIFO_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_FIFO_STATUS_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_FIFO_STATUS_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_FIFO_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_FIFO_STATUS_LINK_STATUS
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_FIFO_STATUS_LINK_STATUS_FIELD =
{
    "LINK_STATUS",
#if RU_INCLUDE_DESC
    "",
    "This bit indicates the link status used by XLMAC EEE and lag-failover state machines. This reflects the live status of the link as seen by the MAC. If set, indicates that link is active.",
#endif
    LPORT_XLMAC_FIFO_STATUS_LINK_STATUS_FIELD_MASK,
    0,
    LPORT_XLMAC_FIFO_STATUS_LINK_STATUS_FIELD_WIDTH,
    LPORT_XLMAC_FIFO_STATUS_LINK_STATUS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_FIFO_STATUS_RX_PKT_OVERFLOW
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_FIFO_STATUS_RX_PKT_OVERFLOW_FIELD =
{
    "RX_PKT_OVERFLOW",
#if RU_INCLUDE_DESC
    "",
    "If set, indicates RX packet fifo overflow",
#endif
    LPORT_XLMAC_FIFO_STATUS_RX_PKT_OVERFLOW_FIELD_MASK,
    0,
    LPORT_XLMAC_FIFO_STATUS_RX_PKT_OVERFLOW_FIELD_WIDTH,
    LPORT_XLMAC_FIFO_STATUS_RX_PKT_OVERFLOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_FIFO_STATUS_TX_TS_FIFO_OVERFLOW
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_FIFO_STATUS_TX_TS_FIFO_OVERFLOW_FIELD =
{
    "TX_TS_FIFO_OVERFLOW",
#if RU_INCLUDE_DESC
    "",
    "If set, indicates overflow occurred in TX two-step Time Stamp FIFO",
#endif
    LPORT_XLMAC_FIFO_STATUS_TX_TS_FIFO_OVERFLOW_FIELD_MASK,
    0,
    LPORT_XLMAC_FIFO_STATUS_TX_TS_FIFO_OVERFLOW_FIELD_WIDTH,
    LPORT_XLMAC_FIFO_STATUS_TX_TS_FIFO_OVERFLOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_FIFO_STATUS_TX_LLFC_MSG_OVERFLOW
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_FIFO_STATUS_TX_LLFC_MSG_OVERFLOW_FIELD =
{
    "TX_LLFC_MSG_OVERFLOW",
#if RU_INCLUDE_DESC
    "",
    "If set, indicates TX LLFC message fifo overflow",
#endif
    LPORT_XLMAC_FIFO_STATUS_TX_LLFC_MSG_OVERFLOW_FIELD_MASK,
    0,
    LPORT_XLMAC_FIFO_STATUS_TX_LLFC_MSG_OVERFLOW_FIELD_WIDTH,
    LPORT_XLMAC_FIFO_STATUS_TX_LLFC_MSG_OVERFLOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_FIFO_STATUS_RSVD_2
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_FIFO_STATUS_RSVD_2_FIELD =
{
    "RSVD_2",
#if RU_INCLUDE_DESC
    "",
    "Reserved",
#endif
    LPORT_XLMAC_FIFO_STATUS_RSVD_2_FIELD_MASK,
    0,
    LPORT_XLMAC_FIFO_STATUS_RSVD_2_FIELD_WIDTH,
    LPORT_XLMAC_FIFO_STATUS_RSVD_2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_FIFO_STATUS_TX_PKT_OVERFLOW
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_FIFO_STATUS_TX_PKT_OVERFLOW_FIELD =
{
    "TX_PKT_OVERFLOW",
#if RU_INCLUDE_DESC
    "",
    "If set, indicates tx packet fifo overflow",
#endif
    LPORT_XLMAC_FIFO_STATUS_TX_PKT_OVERFLOW_FIELD_MASK,
    0,
    LPORT_XLMAC_FIFO_STATUS_TX_PKT_OVERFLOW_FIELD_WIDTH,
    LPORT_XLMAC_FIFO_STATUS_TX_PKT_OVERFLOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_FIFO_STATUS_TX_PKT_UNDERFLOW
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_FIFO_STATUS_TX_PKT_UNDERFLOW_FIELD =
{
    "TX_PKT_UNDERFLOW",
#if RU_INCLUDE_DESC
    "",
    "If set, indicates tx packet fifo underflow",
#endif
    LPORT_XLMAC_FIFO_STATUS_TX_PKT_UNDERFLOW_FIELD_MASK,
    0,
    LPORT_XLMAC_FIFO_STATUS_TX_PKT_UNDERFLOW_FIELD_WIDTH,
    LPORT_XLMAC_FIFO_STATUS_TX_PKT_UNDERFLOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_FIFO_STATUS_RX_MSG_OVERFLOW
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_FIFO_STATUS_RX_MSG_OVERFLOW_FIELD =
{
    "RX_MSG_OVERFLOW",
#if RU_INCLUDE_DESC
    "",
    "If set, indicates rx message fifo overflow",
#endif
    LPORT_XLMAC_FIFO_STATUS_RX_MSG_OVERFLOW_FIELD_MASK,
    0,
    LPORT_XLMAC_FIFO_STATUS_RX_MSG_OVERFLOW_FIELD_WIDTH,
    LPORT_XLMAC_FIFO_STATUS_RX_MSG_OVERFLOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_FIFO_STATUS_RSVD_1
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_FIFO_STATUS_RSVD_1_FIELD =
{
    "RSVD_1",
#if RU_INCLUDE_DESC
    "",
    "Reserved",
#endif
    LPORT_XLMAC_FIFO_STATUS_RSVD_1_FIELD_MASK,
    0,
    LPORT_XLMAC_FIFO_STATUS_RSVD_1_FIELD_WIDTH,
    LPORT_XLMAC_FIFO_STATUS_RSVD_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_CLEAR_FIFO_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_CLEAR_FIFO_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_CLEAR_FIFO_STATUS_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_CLEAR_FIFO_STATUS_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_CLEAR_FIFO_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_CLEAR_FIFO_STATUS_CLEAR_RX_PKT_OVERFLOW
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_CLEAR_FIFO_STATUS_CLEAR_RX_PKT_OVERFLOW_FIELD =
{
    "CLEAR_RX_PKT_OVERFLOW",
#if RU_INCLUDE_DESC
    "",
    "A rising edge on this register bit (0->1), clears the sticky RX_PKT_OVERFLOW status bit.",
#endif
    LPORT_XLMAC_CLEAR_FIFO_STATUS_CLEAR_RX_PKT_OVERFLOW_FIELD_MASK,
    0,
    LPORT_XLMAC_CLEAR_FIFO_STATUS_CLEAR_RX_PKT_OVERFLOW_FIELD_WIDTH,
    LPORT_XLMAC_CLEAR_FIFO_STATUS_CLEAR_RX_PKT_OVERFLOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_CLEAR_FIFO_STATUS_CLEAR_TX_TS_FIFO_OVERFLOW
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_CLEAR_FIFO_STATUS_CLEAR_TX_TS_FIFO_OVERFLOW_FIELD =
{
    "CLEAR_TX_TS_FIFO_OVERFLOW",
#if RU_INCLUDE_DESC
    "",
    "A rising edge on this register bit (0->1), clears the sticky TX_TS_FIFO_OVERFLOW status bit.",
#endif
    LPORT_XLMAC_CLEAR_FIFO_STATUS_CLEAR_TX_TS_FIFO_OVERFLOW_FIELD_MASK,
    0,
    LPORT_XLMAC_CLEAR_FIFO_STATUS_CLEAR_TX_TS_FIFO_OVERFLOW_FIELD_WIDTH,
    LPORT_XLMAC_CLEAR_FIFO_STATUS_CLEAR_TX_TS_FIFO_OVERFLOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_CLEAR_FIFO_STATUS_CLEAR_TX_LLFC_MSG_OVERFLOW
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_CLEAR_FIFO_STATUS_CLEAR_TX_LLFC_MSG_OVERFLOW_FIELD =
{
    "CLEAR_TX_LLFC_MSG_OVERFLOW",
#if RU_INCLUDE_DESC
    "",
    "A rising edge on this register bit (0->1), clears the sticky TX_LLFC_MSG_OVERFLOW status bit.",
#endif
    LPORT_XLMAC_CLEAR_FIFO_STATUS_CLEAR_TX_LLFC_MSG_OVERFLOW_FIELD_MASK,
    0,
    LPORT_XLMAC_CLEAR_FIFO_STATUS_CLEAR_TX_LLFC_MSG_OVERFLOW_FIELD_WIDTH,
    LPORT_XLMAC_CLEAR_FIFO_STATUS_CLEAR_TX_LLFC_MSG_OVERFLOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_CLEAR_FIFO_STATUS_RSVD_2
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_CLEAR_FIFO_STATUS_RSVD_2_FIELD =
{
    "RSVD_2",
#if RU_INCLUDE_DESC
    "",
    "Reserved",
#endif
    LPORT_XLMAC_CLEAR_FIFO_STATUS_RSVD_2_FIELD_MASK,
    0,
    LPORT_XLMAC_CLEAR_FIFO_STATUS_RSVD_2_FIELD_WIDTH,
    LPORT_XLMAC_CLEAR_FIFO_STATUS_RSVD_2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_CLEAR_FIFO_STATUS_CLEAR_TX_PKT_OVERFLOW
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_CLEAR_FIFO_STATUS_CLEAR_TX_PKT_OVERFLOW_FIELD =
{
    "CLEAR_TX_PKT_OVERFLOW",
#if RU_INCLUDE_DESC
    "",
    "A rising edge on this register bit (0->1), clears the sticky TX_PKT_OVERFLOW status bit.",
#endif
    LPORT_XLMAC_CLEAR_FIFO_STATUS_CLEAR_TX_PKT_OVERFLOW_FIELD_MASK,
    0,
    LPORT_XLMAC_CLEAR_FIFO_STATUS_CLEAR_TX_PKT_OVERFLOW_FIELD_WIDTH,
    LPORT_XLMAC_CLEAR_FIFO_STATUS_CLEAR_TX_PKT_OVERFLOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_CLEAR_FIFO_STATUS_CLEAR_TX_PKT_UNDERFLOW
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_CLEAR_FIFO_STATUS_CLEAR_TX_PKT_UNDERFLOW_FIELD =
{
    "CLEAR_TX_PKT_UNDERFLOW",
#if RU_INCLUDE_DESC
    "",
    "A rising edge on this register bit (0->1), clears the sticky TX_PKT_UNDERFLOW status bit",
#endif
    LPORT_XLMAC_CLEAR_FIFO_STATUS_CLEAR_TX_PKT_UNDERFLOW_FIELD_MASK,
    0,
    LPORT_XLMAC_CLEAR_FIFO_STATUS_CLEAR_TX_PKT_UNDERFLOW_FIELD_WIDTH,
    LPORT_XLMAC_CLEAR_FIFO_STATUS_CLEAR_TX_PKT_UNDERFLOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_CLEAR_FIFO_STATUS_CLEAR_RX_MSG_OVERFLOW
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_CLEAR_FIFO_STATUS_CLEAR_RX_MSG_OVERFLOW_FIELD =
{
    "CLEAR_RX_MSG_OVERFLOW",
#if RU_INCLUDE_DESC
    "",
    "A rising edge on this register bit (0->1), clears the sticky RX_MSG_OVERFLOW status bit",
#endif
    LPORT_XLMAC_CLEAR_FIFO_STATUS_CLEAR_RX_MSG_OVERFLOW_FIELD_MASK,
    0,
    LPORT_XLMAC_CLEAR_FIFO_STATUS_CLEAR_RX_MSG_OVERFLOW_FIELD_WIDTH,
    LPORT_XLMAC_CLEAR_FIFO_STATUS_CLEAR_RX_MSG_OVERFLOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_CLEAR_FIFO_STATUS_RSVD_1
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_CLEAR_FIFO_STATUS_RSVD_1_FIELD =
{
    "RSVD_1",
#if RU_INCLUDE_DESC
    "",
    "Reserved",
#endif
    LPORT_XLMAC_CLEAR_FIFO_STATUS_RSVD_1_FIELD_MASK,
    0,
    LPORT_XLMAC_CLEAR_FIFO_STATUS_RSVD_1_FIELD_WIDTH,
    LPORT_XLMAC_CLEAR_FIFO_STATUS_RSVD_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_LAG_FAILOVER_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_LAG_FAILOVER_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_LAG_FAILOVER_STATUS_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_LAG_FAILOVER_STATUS_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_LAG_FAILOVER_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_LAG_FAILOVER_STATUS_RSVD
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_LAG_FAILOVER_STATUS_RSVD_FIELD =
{
    "RSVD",
#if RU_INCLUDE_DESC
    "",
    "Reserved",
#endif
    LPORT_XLMAC_LAG_FAILOVER_STATUS_RSVD_FIELD_MASK,
    0,
    LPORT_XLMAC_LAG_FAILOVER_STATUS_RSVD_FIELD_WIDTH,
    LPORT_XLMAC_LAG_FAILOVER_STATUS_RSVD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_LAG_FAILOVER_STATUS_LAG_FAILOVER_LOOPBACK
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_LAG_FAILOVER_STATUS_LAG_FAILOVER_LOOPBACK_FIELD =
{
    "LAG_FAILOVER_LOOPBACK",
#if RU_INCLUDE_DESC
    "",
    "Set when XLMAC is in lag failover state",
#endif
    LPORT_XLMAC_LAG_FAILOVER_STATUS_LAG_FAILOVER_LOOPBACK_FIELD_MASK,
    0,
    LPORT_XLMAC_LAG_FAILOVER_STATUS_LAG_FAILOVER_LOOPBACK_FIELD_WIDTH,
    LPORT_XLMAC_LAG_FAILOVER_STATUS_LAG_FAILOVER_LOOPBACK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_EEE_CTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_EEE_CTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_EEE_CTRL_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_EEE_CTRL_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_EEE_CTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_EEE_CTRL_RSVD
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_EEE_CTRL_RSVD_FIELD =
{
    "RSVD",
#if RU_INCLUDE_DESC
    "",
    "Reserved",
#endif
    LPORT_XLMAC_EEE_CTRL_RSVD_FIELD_MASK,
    0,
    LPORT_XLMAC_EEE_CTRL_RSVD_FIELD_WIDTH,
    LPORT_XLMAC_EEE_CTRL_RSVD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_EEE_CTRL_EEE_EN
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_EEE_CTRL_EEE_EN_FIELD =
{
    "EEE_EN",
#if RU_INCLUDE_DESC
    "",
    "When set, enables EEE state machine in the transmit direction and LPI detection/prediction in the receive direction",
#endif
    LPORT_XLMAC_EEE_CTRL_EEE_EN_FIELD_MASK,
    0,
    LPORT_XLMAC_EEE_CTRL_EEE_EN_FIELD_WIDTH,
    LPORT_XLMAC_EEE_CTRL_EEE_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_EEE_TIMERS_EEE_REF_COUNT
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_EEE_TIMERS_EEE_REF_COUNT_FIELD =
{
    "EEE_REF_COUNT",
#if RU_INCLUDE_DESC
    "",
    "This field controls clock divider used to generate ~1us reference pulses used by EEE timers. It specifies integer number of clock cycles for 1us reference using tsc_clk",
#endif
    LPORT_XLMAC_EEE_TIMERS_EEE_REF_COUNT_FIELD_MASK,
    0,
    LPORT_XLMAC_EEE_TIMERS_EEE_REF_COUNT_FIELD_WIDTH,
    LPORT_XLMAC_EEE_TIMERS_EEE_REF_COUNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_EEE_TIMERS_EEE_WAKE_TIMER
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_EEE_TIMERS_EEE_WAKE_TIMER_FIELD =
{
    "EEE_WAKE_TIMER",
#if RU_INCLUDE_DESC
    "",
    "This is the duration for which MAC must wait to go back to ACTIVE state from LPI state when it receives packet/flow-control frames for transmission. Unit is micro seconds",
#endif
    LPORT_XLMAC_EEE_TIMERS_EEE_WAKE_TIMER_FIELD_MASK,
    0,
    LPORT_XLMAC_EEE_TIMERS_EEE_WAKE_TIMER_FIELD_WIDTH,
    LPORT_XLMAC_EEE_TIMERS_EEE_WAKE_TIMER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_EEE_TIMERS_EEE_DELAY_ENTRY_TIMER
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_EEE_TIMERS_EEE_DELAY_ENTRY_TIMER_FIELD =
{
    "EEE_DELAY_ENTRY_TIMER",
#if RU_INCLUDE_DESC
    "",
    "This is the duration for which the MAC must wait in EMPTY state before transitioning to LPI state. Unit is micro seconds",
#endif
    LPORT_XLMAC_EEE_TIMERS_EEE_DELAY_ENTRY_TIMER_FIELD_MASK,
    0,
    LPORT_XLMAC_EEE_TIMERS_EEE_DELAY_ENTRY_TIMER_FIELD_WIDTH,
    LPORT_XLMAC_EEE_TIMERS_EEE_DELAY_ENTRY_TIMER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_EEE_TIMERS_OVERLAY_XLMAC_EEE_TIMERS_HI
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_EEE_TIMERS_OVERLAY_XLMAC_EEE_TIMERS_HI_FIELD =
{
    "XLMAC_EEE_TIMERS_HI",
#if RU_INCLUDE_DESC
    "",
    "32 upper bits of the EEE timer register",
#endif
    LPORT_XLMAC_EEE_TIMERS_OVERLAY_XLMAC_EEE_TIMERS_HI_FIELD_MASK,
    0,
    LPORT_XLMAC_EEE_TIMERS_OVERLAY_XLMAC_EEE_TIMERS_HI_FIELD_WIDTH,
    LPORT_XLMAC_EEE_TIMERS_OVERLAY_XLMAC_EEE_TIMERS_HI_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_EEE_TIMERS_OVERLAY_XLMAC_EEE_TIMERS_LO
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_EEE_TIMERS_OVERLAY_XLMAC_EEE_TIMERS_LO_FIELD =
{
    "XLMAC_EEE_TIMERS_LO",
#if RU_INCLUDE_DESC
    "",
    "32 lower bits of the EEE Timer",
#endif
    LPORT_XLMAC_EEE_TIMERS_OVERLAY_XLMAC_EEE_TIMERS_LO_FIELD_MASK,
    0,
    LPORT_XLMAC_EEE_TIMERS_OVERLAY_XLMAC_EEE_TIMERS_LO_FIELD_WIDTH,
    LPORT_XLMAC_EEE_TIMERS_OVERLAY_XLMAC_EEE_TIMERS_LO_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_EEE_1_SEC_LINK_STATUS_TIMER_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_EEE_1_SEC_LINK_STATUS_TIMER_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_EEE_1_SEC_LINK_STATUS_TIMER_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_EEE_1_SEC_LINK_STATUS_TIMER_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_EEE_1_SEC_LINK_STATUS_TIMER_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_EEE_1_SEC_LINK_STATUS_TIMER_ONE_SECOND_TIMER
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_EEE_1_SEC_LINK_STATUS_TIMER_ONE_SECOND_TIMER_FIELD =
{
    "ONE_SECOND_TIMER",
#if RU_INCLUDE_DESC
    "",
    "This is the duration for which EEE FSM must wait when Link status becomes active before transitioning to ACTIVE state. Unit is micro seconds",
#endif
    LPORT_XLMAC_EEE_1_SEC_LINK_STATUS_TIMER_ONE_SECOND_TIMER_FIELD_MASK,
    0,
    LPORT_XLMAC_EEE_1_SEC_LINK_STATUS_TIMER_ONE_SECOND_TIMER_FIELD_WIDTH,
    LPORT_XLMAC_EEE_1_SEC_LINK_STATUS_TIMER_ONE_SECOND_TIMER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_HIGIG_HDR_0_HIGIG_HDR_0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_HIGIG_HDR_0_HIGIG_HDR_0_FIELD =
{
    "HIGIG_HDR_0",
#if RU_INCLUDE_DESC
    "",
    "In HiGig2 mode, this register contains bits 127:64 of 16-byte HiGig2 header. \n"
    "In HiGig+ mode, bits 31:0 of this register contains bits 95:64 of 12-byte HiGig+ header.\n"
    "This field is used for constructing the module header for HiGig2/HiGig+ pause and PFC frames in the transmit direction.",
#endif
    LPORT_XLMAC_HIGIG_HDR_0_HIGIG_HDR_0_FIELD_MASK,
    0,
    LPORT_XLMAC_HIGIG_HDR_0_HIGIG_HDR_0_FIELD_WIDTH,
    LPORT_XLMAC_HIGIG_HDR_0_HIGIG_HDR_0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_HIGIG_HDR_0_OVERLAY_HIGIG_HDR_0_HI
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_HIGIG_HDR_0_OVERLAY_HIGIG_HDR_0_HI_FIELD =
{
    "HIGIG_HDR_0_HI",
#if RU_INCLUDE_DESC
    "",
    "32 upper bits of the HIGIG_HDR_0",
#endif
    LPORT_XLMAC_HIGIG_HDR_0_OVERLAY_HIGIG_HDR_0_HI_FIELD_MASK,
    0,
    LPORT_XLMAC_HIGIG_HDR_0_OVERLAY_HIGIG_HDR_0_HI_FIELD_WIDTH,
    LPORT_XLMAC_HIGIG_HDR_0_OVERLAY_HIGIG_HDR_0_HI_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_HIGIG_HDR_0_OVERLAY_HIGIG_HDR_0_LO
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_HIGIG_HDR_0_OVERLAY_HIGIG_HDR_0_LO_FIELD =
{
    "HIGIG_HDR_0_LO",
#if RU_INCLUDE_DESC
    "",
    "32 lower bits of the HIGIG_HDR_0",
#endif
    LPORT_XLMAC_HIGIG_HDR_0_OVERLAY_HIGIG_HDR_0_LO_FIELD_MASK,
    0,
    LPORT_XLMAC_HIGIG_HDR_0_OVERLAY_HIGIG_HDR_0_LO_FIELD_WIDTH,
    LPORT_XLMAC_HIGIG_HDR_0_OVERLAY_HIGIG_HDR_0_LO_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_HIGIG_HDR_1_HIGIG_HDR_1
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_HIGIG_HDR_1_HIGIG_HDR_1_FIELD =
{
    "HIGIG_HDR_1",
#if RU_INCLUDE_DESC
    "",
    "In HiGig2 mode, this register contains bits 63:0 of 16-byte HiGig2 header. \n"
    "In HiGig+ mode, this register contains bits 63:0 of 12-byte HiGig+ header.\n"
    "This field is used for constructing the module header for HiGig2/HiGig+ pause and PFC frames in the transmit direction.",
#endif
    LPORT_XLMAC_HIGIG_HDR_1_HIGIG_HDR_1_FIELD_MASK,
    0,
    LPORT_XLMAC_HIGIG_HDR_1_HIGIG_HDR_1_FIELD_WIDTH,
    LPORT_XLMAC_HIGIG_HDR_1_HIGIG_HDR_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_HIGIG_HDR_1_OVERLAY_HIGIG_HDR_1_HI
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_HIGIG_HDR_1_OVERLAY_HIGIG_HDR_1_HI_FIELD =
{
    "HIGIG_HDR_1_HI",
#if RU_INCLUDE_DESC
    "",
    "32 upper bits of the HIGIG_HDR_1",
#endif
    LPORT_XLMAC_HIGIG_HDR_1_OVERLAY_HIGIG_HDR_1_HI_FIELD_MASK,
    0,
    LPORT_XLMAC_HIGIG_HDR_1_OVERLAY_HIGIG_HDR_1_HI_FIELD_WIDTH,
    LPORT_XLMAC_HIGIG_HDR_1_OVERLAY_HIGIG_HDR_1_HI_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_HIGIG_HDR_1_OVERLAY_HIGIG_HDR_1_LO
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_HIGIG_HDR_1_OVERLAY_HIGIG_HDR_1_LO_FIELD =
{
    "HIGIG_HDR_1_LO",
#if RU_INCLUDE_DESC
    "",
    "32 lower bits of the HIGIG_HDR_1",
#endif
    LPORT_XLMAC_HIGIG_HDR_1_OVERLAY_HIGIG_HDR_1_LO_FIELD_MASK,
    0,
    LPORT_XLMAC_HIGIG_HDR_1_OVERLAY_HIGIG_HDR_1_LO_FIELD_WIDTH,
    LPORT_XLMAC_HIGIG_HDR_1_OVERLAY_HIGIG_HDR_1_LO_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_GMII_EEE_CTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_GMII_EEE_CTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_GMII_EEE_CTRL_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_GMII_EEE_CTRL_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_GMII_EEE_CTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_GMII_EEE_CTRL_GMII_LPI_PREDICT_MODE_EN
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_GMII_EEE_CTRL_GMII_LPI_PREDICT_MODE_EN_FIELD =
{
    "GMII_LPI_PREDICT_MODE_EN",
#if RU_INCLUDE_DESC
    "",
    "When set, enables LPI prediction",
#endif
    LPORT_XLMAC_GMII_EEE_CTRL_GMII_LPI_PREDICT_MODE_EN_FIELD_MASK,
    0,
    LPORT_XLMAC_GMII_EEE_CTRL_GMII_LPI_PREDICT_MODE_EN_FIELD_WIDTH,
    LPORT_XLMAC_GMII_EEE_CTRL_GMII_LPI_PREDICT_MODE_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_GMII_EEE_CTRL_GMII_LPI_PREDICT_THRESHOLD
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_GMII_EEE_CTRL_GMII_LPI_PREDICT_THRESHOLD_FIELD =
{
    "GMII_LPI_PREDICT_THRESHOLD",
#if RU_INCLUDE_DESC
    "",
    "If GMII_LPI_PREDICT_MODE_EN is set then this field defines the number of IDLEs to be received before allowing LPIs to be sent to Link Partner",
#endif
    LPORT_XLMAC_GMII_EEE_CTRL_GMII_LPI_PREDICT_THRESHOLD_FIELD_MASK,
    0,
    LPORT_XLMAC_GMII_EEE_CTRL_GMII_LPI_PREDICT_THRESHOLD_FIELD_WIDTH,
    LPORT_XLMAC_GMII_EEE_CTRL_GMII_LPI_PREDICT_THRESHOLD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TIMESTAMP_ADJUST_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TIMESTAMP_ADJUST_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_TIMESTAMP_ADJUST_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_TIMESTAMP_ADJUST_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_TIMESTAMP_ADJUST_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TIMESTAMP_ADJUST_TS_USE_CS_OFFSET
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TIMESTAMP_ADJUST_TS_USE_CS_OFFSET_FIELD =
{
    "TS_USE_CS_OFFSET",
#if RU_INCLUDE_DESC
    "",
    "When set, indicates that the checksum offset is referenced by input port checksumoffset, else checksum offset is referenced by txtsoffset",
#endif
    LPORT_XLMAC_TIMESTAMP_ADJUST_TS_USE_CS_OFFSET_FIELD_MASK,
    0,
    LPORT_XLMAC_TIMESTAMP_ADJUST_TS_USE_CS_OFFSET_FIELD_WIDTH,
    LPORT_XLMAC_TIMESTAMP_ADJUST_TS_USE_CS_OFFSET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TIMESTAMP_ADJUST_TS_TSTS_ADJUST
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TIMESTAMP_ADJUST_TS_TSTS_ADJUST_FIELD =
{
    "TS_TSTS_ADJUST",
#if RU_INCLUDE_DESC
    "",
    "This is an unsigned value to account for synchronization delay of TS timer from TS clk to TSC_CLK domain. Unit is 1ns.\n"
    "The latency is [2.5 TSC_CLK period + 1 TS_CLK period].",
#endif
    LPORT_XLMAC_TIMESTAMP_ADJUST_TS_TSTS_ADJUST_FIELD_MASK,
    0,
    LPORT_XLMAC_TIMESTAMP_ADJUST_TS_TSTS_ADJUST_FIELD_WIDTH,
    LPORT_XLMAC_TIMESTAMP_ADJUST_TS_TSTS_ADJUST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TIMESTAMP_ADJUST_TS_OSTS_ADJUST
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TIMESTAMP_ADJUST_TS_OSTS_ADJUST_FIELD =
{
    "TS_OSTS_ADJUST",
#if RU_INCLUDE_DESC
    "",
    "This is a signed value which is 2s complement added to synchronized timestamp to account for MAC pipeline delay in OSTS. Unit is 1ns\n"
    "The latency is [6 TSC_CLK period + 1 TS_CLK period ].",
#endif
    LPORT_XLMAC_TIMESTAMP_ADJUST_TS_OSTS_ADJUST_FIELD_MASK,
    0,
    LPORT_XLMAC_TIMESTAMP_ADJUST_TS_OSTS_ADJUST_FIELD_WIDTH,
    LPORT_XLMAC_TIMESTAMP_ADJUST_TS_OSTS_ADJUST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TIMESTAMP_BYTE_ADJUST_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TIMESTAMP_BYTE_ADJUST_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_TIMESTAMP_BYTE_ADJUST_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_TIMESTAMP_BYTE_ADJUST_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_TIMESTAMP_BYTE_ADJUST_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TIMESTAMP_BYTE_ADJUST_RX_TIMER_BYTE_ADJUST_EN
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TIMESTAMP_BYTE_ADJUST_RX_TIMER_BYTE_ADJUST_EN_FIELD =
{
    "RX_TIMER_BYTE_ADJUST_EN",
#if RU_INCLUDE_DESC
    "",
    "When set, enables byte based adjustment for receive timestamp capture. This should be enabled in GMII/MII modes only.",
#endif
    LPORT_XLMAC_TIMESTAMP_BYTE_ADJUST_RX_TIMER_BYTE_ADJUST_EN_FIELD_MASK,
    0,
    LPORT_XLMAC_TIMESTAMP_BYTE_ADJUST_RX_TIMER_BYTE_ADJUST_EN_FIELD_WIDTH,
    LPORT_XLMAC_TIMESTAMP_BYTE_ADJUST_RX_TIMER_BYTE_ADJUST_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TIMESTAMP_BYTE_ADJUST_RX_TIMER_BYTE_ADJUST
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TIMESTAMP_BYTE_ADJUST_RX_TIMER_BYTE_ADJUST_FIELD =
{
    "RX_TIMER_BYTE_ADJUST",
#if RU_INCLUDE_DESC
    "",
    "This is a per byte unsigned value which is subtracted from sampled timestamp to account for timestamp jitter due to wider MSBUS interface. Unit is 1ns",
#endif
    LPORT_XLMAC_TIMESTAMP_BYTE_ADJUST_RX_TIMER_BYTE_ADJUST_FIELD_MASK,
    0,
    LPORT_XLMAC_TIMESTAMP_BYTE_ADJUST_RX_TIMER_BYTE_ADJUST_FIELD_WIDTH,
    LPORT_XLMAC_TIMESTAMP_BYTE_ADJUST_RX_TIMER_BYTE_ADJUST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TIMESTAMP_BYTE_ADJUST_TX_TIMER_BYTE_ADJUST_EN
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TIMESTAMP_BYTE_ADJUST_TX_TIMER_BYTE_ADJUST_EN_FIELD =
{
    "TX_TIMER_BYTE_ADJUST_EN",
#if RU_INCLUDE_DESC
    "",
    "When set, enables byte based adjustment for transmit timestamp capture (OSTS and TSTS). This should be enabled in GMII/MII modes only.",
#endif
    LPORT_XLMAC_TIMESTAMP_BYTE_ADJUST_TX_TIMER_BYTE_ADJUST_EN_FIELD_MASK,
    0,
    LPORT_XLMAC_TIMESTAMP_BYTE_ADJUST_TX_TIMER_BYTE_ADJUST_EN_FIELD_WIDTH,
    LPORT_XLMAC_TIMESTAMP_BYTE_ADJUST_TX_TIMER_BYTE_ADJUST_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TIMESTAMP_BYTE_ADJUST_TX_TIMER_BYTE_ADJUST
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TIMESTAMP_BYTE_ADJUST_TX_TIMER_BYTE_ADJUST_FIELD =
{
    "TX_TIMER_BYTE_ADJUST",
#if RU_INCLUDE_DESC
    "",
    "This is a per byte unsigned value which is added to sampled timestamp to account for timestamp jitter due to wider MSBUS interface. Unit is 1ns",
#endif
    LPORT_XLMAC_TIMESTAMP_BYTE_ADJUST_TX_TIMER_BYTE_ADJUST_FIELD_MASK,
    0,
    LPORT_XLMAC_TIMESTAMP_BYTE_ADJUST_TX_TIMER_BYTE_ADJUST_FIELD_WIDTH,
    LPORT_XLMAC_TIMESTAMP_BYTE_ADJUST_TX_TIMER_BYTE_ADJUST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_PROG_TX_CRC
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_PROG_TX_CRC_FIELD =
{
    "PROG_TX_CRC",
#if RU_INCLUDE_DESC
    "",
    "Programmable CRC value used to corrupt the Tx CRC. The computed CRC is replaced by this programmed CRC value based on TX_CRC_CORRUPTION_MODE",
#endif
    LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_PROG_TX_CRC_FIELD_MASK,
    0,
    LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_PROG_TX_CRC_FIELD_WIDTH,
    LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_PROG_TX_CRC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_TX_CRC_CORRUPTION_MODE
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_TX_CRC_CORRUPTION_MODE_FIELD =
{
    "TX_CRC_CORRUPTION_MODE",
#if RU_INCLUDE_DESC
    "",
    "When set, the computed CRC is replaced with PROG_TX_CRC, else computed CRC is inverted",
#endif
    LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_TX_CRC_CORRUPTION_MODE_FIELD_MASK,
    0,
    LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_TX_CRC_CORRUPTION_MODE_FIELD_WIDTH,
    LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_TX_CRC_CORRUPTION_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_TX_CRC_CORRUPT_EN
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_TX_CRC_CORRUPT_EN_FIELD =
{
    "TX_CRC_CORRUPT_EN",
#if RU_INCLUDE_DESC
    "",
    "When set, MAC enables the CRC corruption on the transmitted packets. Mode of corruption is determined by TX_CRC_CORRUPTION_MODE",
#endif
    LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_TX_CRC_CORRUPT_EN_FIELD_MASK,
    0,
    LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_TX_CRC_CORRUPT_EN_FIELD_WIDTH,
    LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_TX_CRC_CORRUPT_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_TX_ERR_CORRUPTS_CRC
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_TX_ERR_CORRUPTS_CRC_FIELD =
{
    "TX_ERR_CORRUPTS_CRC",
#if RU_INCLUDE_DESC
    "",
    "When set, this bit causes packets with TXERR to corrupt the CRC of the packet when it is transmitted.\n"
    "When reset, packets with TXERR are transmitted with /E/ termination character (/T/ is not enforced); packet CRC is unaffected",
#endif
    LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_TX_ERR_CORRUPTS_CRC_FIELD_MASK,
    0,
    LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_TX_ERR_CORRUPTS_CRC_FIELD_WIDTH,
    LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_TX_ERR_CORRUPTS_CRC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_OVERLAY_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_OVERLAY_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_OVERLAY_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_OVERLAY_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_OVERLAY_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_OVERLAY_XLMAC_TX_CRC_CORRUPT_CTRL_HI
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_OVERLAY_XLMAC_TX_CRC_CORRUPT_CTRL_HI_FIELD =
{
    "XLMAC_TX_CRC_CORRUPT_CTRL_HI",
#if RU_INCLUDE_DESC
    "",
    "3 Upper bits of the Tx CRC corrupt control register",
#endif
    LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_OVERLAY_XLMAC_TX_CRC_CORRUPT_CTRL_HI_FIELD_MASK,
    0,
    LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_OVERLAY_XLMAC_TX_CRC_CORRUPT_CTRL_HI_FIELD_WIDTH,
    LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_OVERLAY_XLMAC_TX_CRC_CORRUPT_CTRL_HI_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_OVERLAY_XLMAC_TX_CRC_CORRUPT_CTRL_LO
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_OVERLAY_XLMAC_TX_CRC_CORRUPT_CTRL_LO_FIELD =
{
    "XLMAC_TX_CRC_CORRUPT_CTRL_LO",
#if RU_INCLUDE_DESC
    "",
    "32 lower bits of the Tx CRC corrupt control register",
#endif
    LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_OVERLAY_XLMAC_TX_CRC_CORRUPT_CTRL_LO_FIELD_MASK,
    0,
    LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_OVERLAY_XLMAC_TX_CRC_CORRUPT_CTRL_LO_FIELD_WIDTH,
    LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_OVERLAY_XLMAC_TX_CRC_CORRUPT_CTRL_LO_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_E2E_CTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_E2E_CTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_E2E_CTRL_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_E2E_CTRL_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_E2E_CTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_E2E_CTRL_E2EFC_DUAL_MODID_EN
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_E2E_CTRL_E2EFC_DUAL_MODID_EN_FIELD =
{
    "E2EFC_DUAL_MODID_EN",
#if RU_INCLUDE_DESC
    "",
    "When set, dual modid is enabled for E2EFC (Only 32 ports IBP is sent out). When reset, single modid is enabled for E2EFC (64 ports IBP is sent)",
#endif
    LPORT_XLMAC_E2E_CTRL_E2EFC_DUAL_MODID_EN_FIELD_MASK,
    0,
    LPORT_XLMAC_E2E_CTRL_E2EFC_DUAL_MODID_EN_FIELD_WIDTH,
    LPORT_XLMAC_E2E_CTRL_E2EFC_DUAL_MODID_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_E2E_CTRL_E2ECC_LEGACY_IMP_EN
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_E2E_CTRL_E2ECC_LEGACY_IMP_EN_FIELD =
{
    "E2ECC_LEGACY_IMP_EN",
#if RU_INCLUDE_DESC
    "",
    "When set, legacy E2ECC stage2 loading enabled (single stage2 buffer for all ports). When reset, new E2ECC stage2 loading enabled (per port stage2 buffer)",
#endif
    LPORT_XLMAC_E2E_CTRL_E2ECC_LEGACY_IMP_EN_FIELD_MASK,
    0,
    LPORT_XLMAC_E2E_CTRL_E2ECC_LEGACY_IMP_EN_FIELD_WIDTH,
    LPORT_XLMAC_E2E_CTRL_E2ECC_LEGACY_IMP_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_E2E_CTRL_E2ECC_DUAL_MODID_EN
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_E2E_CTRL_E2ECC_DUAL_MODID_EN_FIELD =
{
    "E2ECC_DUAL_MODID_EN",
#if RU_INCLUDE_DESC
    "",
    "When set, dual modid is enabled for E2ECC. When reset, single modid is enabled for E2ECC",
#endif
    LPORT_XLMAC_E2E_CTRL_E2ECC_DUAL_MODID_EN_FIELD_MASK,
    0,
    LPORT_XLMAC_E2E_CTRL_E2ECC_DUAL_MODID_EN_FIELD_WIDTH,
    LPORT_XLMAC_E2E_CTRL_E2ECC_DUAL_MODID_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_E2E_CTRL_HONOR_PAUSE_FOR_E2E
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_E2E_CTRL_HONOR_PAUSE_FOR_E2E_FIELD =
{
    "HONOR_PAUSE_FOR_E2E",
#if RU_INCLUDE_DESC
    "",
    "When set, E2ECC/FC frames are not transmitted during pause state. When reset, E2ECC/FC frames are transmitted even during pause state similar to other flow control frames.",
#endif
    LPORT_XLMAC_E2E_CTRL_HONOR_PAUSE_FOR_E2E_FIELD_MASK,
    0,
    LPORT_XLMAC_E2E_CTRL_HONOR_PAUSE_FOR_E2E_FIELD_WIDTH,
    LPORT_XLMAC_E2E_CTRL_HONOR_PAUSE_FOR_E2E_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_E2E_CTRL_E2E_ENABLE
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_E2E_CTRL_E2E_ENABLE_FIELD =
{
    "E2E_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "When set, MAC enables E2EFC/E2ECC frame generation and transmission.",
#endif
    LPORT_XLMAC_E2E_CTRL_E2E_ENABLE_FIELD_MASK,
    0,
    LPORT_XLMAC_E2E_CTRL_E2E_ENABLE_FIELD_WIDTH,
    LPORT_XLMAC_E2E_CTRL_E2E_ENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_E2ECC_MODULE_HDR_0_E2ECC_MODULE_HDR_0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_E2ECC_MODULE_HDR_0_E2ECC_MODULE_HDR_0_FIELD =
{
    "E2ECC_MODULE_HDR_0",
#if RU_INCLUDE_DESC
    "",
    "In HiGig2 mode, this register contains bits 127:64 of 16-byte HiGig2 header. \n"
    "In HiGig+ mode, this register contains bits 95:32 of 12-byte HiGig+ header.\n"
    "This field is used for constructing the module header for HiGig2/HiGig+ E2ECC frames in the transmit direction.",
#endif
    LPORT_XLMAC_E2ECC_MODULE_HDR_0_E2ECC_MODULE_HDR_0_FIELD_MASK,
    0,
    LPORT_XLMAC_E2ECC_MODULE_HDR_0_E2ECC_MODULE_HDR_0_FIELD_WIDTH,
    LPORT_XLMAC_E2ECC_MODULE_HDR_0_E2ECC_MODULE_HDR_0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_E2ECC_MODULE_HDR_0_OVERLAY_E2ECC_MODULE_HDR_0_HI
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_E2ECC_MODULE_HDR_0_OVERLAY_E2ECC_MODULE_HDR_0_HI_FIELD =
{
    "E2ECC_MODULE_HDR_0_HI",
#if RU_INCLUDE_DESC
    "",
    "32 upper bits of the E2ECC_MODULE_HDR_0",
#endif
    LPORT_XLMAC_E2ECC_MODULE_HDR_0_OVERLAY_E2ECC_MODULE_HDR_0_HI_FIELD_MASK,
    0,
    LPORT_XLMAC_E2ECC_MODULE_HDR_0_OVERLAY_E2ECC_MODULE_HDR_0_HI_FIELD_WIDTH,
    LPORT_XLMAC_E2ECC_MODULE_HDR_0_OVERLAY_E2ECC_MODULE_HDR_0_HI_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_E2ECC_MODULE_HDR_0_OVERLAY_E2ECC_MODULE_HDR_0_LO
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_E2ECC_MODULE_HDR_0_OVERLAY_E2ECC_MODULE_HDR_0_LO_FIELD =
{
    "E2ECC_MODULE_HDR_0_LO",
#if RU_INCLUDE_DESC
    "",
    "32 lower bits of the E2ECC_MODULE_HDR_0",
#endif
    LPORT_XLMAC_E2ECC_MODULE_HDR_0_OVERLAY_E2ECC_MODULE_HDR_0_LO_FIELD_MASK,
    0,
    LPORT_XLMAC_E2ECC_MODULE_HDR_0_OVERLAY_E2ECC_MODULE_HDR_0_LO_FIELD_WIDTH,
    LPORT_XLMAC_E2ECC_MODULE_HDR_0_OVERLAY_E2ECC_MODULE_HDR_0_LO_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_E2ECC_MODULE_HDR_1_E2ECC_MODULE_HDR_1
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_E2ECC_MODULE_HDR_1_E2ECC_MODULE_HDR_1_FIELD =
{
    "E2ECC_MODULE_HDR_1",
#if RU_INCLUDE_DESC
    "",
    "In HiGig2 mode, this register contains bits 63:0 of 16-byte HiGig2 header.\n"
    "In HiGig+ mode, bits 63:32 of this register contains bits 31:0 of 12-byte HiGig+ header.\n"
    "This field is used for constructing the module header for HiGig2/HiGig+ E2ECC frames in the transmit direction.",
#endif
    LPORT_XLMAC_E2ECC_MODULE_HDR_1_E2ECC_MODULE_HDR_1_FIELD_MASK,
    0,
    LPORT_XLMAC_E2ECC_MODULE_HDR_1_E2ECC_MODULE_HDR_1_FIELD_WIDTH,
    LPORT_XLMAC_E2ECC_MODULE_HDR_1_E2ECC_MODULE_HDR_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_E2ECC_MODULE_HDR_1_OVERLAY_E2ECC_MODULE_HDR_1_HI
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_E2ECC_MODULE_HDR_1_OVERLAY_E2ECC_MODULE_HDR_1_HI_FIELD =
{
    "E2ECC_MODULE_HDR_1_HI",
#if RU_INCLUDE_DESC
    "",
    "32 upper bits of the E2ECC_MODULE_HDR_1",
#endif
    LPORT_XLMAC_E2ECC_MODULE_HDR_1_OVERLAY_E2ECC_MODULE_HDR_1_HI_FIELD_MASK,
    0,
    LPORT_XLMAC_E2ECC_MODULE_HDR_1_OVERLAY_E2ECC_MODULE_HDR_1_HI_FIELD_WIDTH,
    LPORT_XLMAC_E2ECC_MODULE_HDR_1_OVERLAY_E2ECC_MODULE_HDR_1_HI_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_E2ECC_MODULE_HDR_1_OVERLAY_E2ECC_MODULE_HDR_1_LO
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_E2ECC_MODULE_HDR_1_OVERLAY_E2ECC_MODULE_HDR_1_LO_FIELD =
{
    "E2ECC_MODULE_HDR_1_LO",
#if RU_INCLUDE_DESC
    "",
    "32 lower bits of the E2ECC_MODULE_HDR_1",
#endif
    LPORT_XLMAC_E2ECC_MODULE_HDR_1_OVERLAY_E2ECC_MODULE_HDR_1_LO_FIELD_MASK,
    0,
    LPORT_XLMAC_E2ECC_MODULE_HDR_1_OVERLAY_E2ECC_MODULE_HDR_1_LO_FIELD_WIDTH,
    LPORT_XLMAC_E2ECC_MODULE_HDR_1_OVERLAY_E2ECC_MODULE_HDR_1_LO_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_E2ECC_DATA_HDR_0_E2ECC_DATA_HDR_0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_E2ECC_DATA_HDR_0_E2ECC_DATA_HDR_0_FIELD =
{
    "E2ECC_DATA_HDR_0",
#if RU_INCLUDE_DESC
    "",
    "This register contains bits 127:64 of 16-byte IEEE header (DA + SA + Length/Type + Opcode).\n"
    "This field is used for constructing the Ethernet header for HiGig2/HiGig+ E2ECC frames in the transmit direction.",
#endif
    LPORT_XLMAC_E2ECC_DATA_HDR_0_E2ECC_DATA_HDR_0_FIELD_MASK,
    0,
    LPORT_XLMAC_E2ECC_DATA_HDR_0_E2ECC_DATA_HDR_0_FIELD_WIDTH,
    LPORT_XLMAC_E2ECC_DATA_HDR_0_E2ECC_DATA_HDR_0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_E2ECC_DATA_HDR_0_OVERLAY_E2ECC_DATA_HDR_0_HI
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_E2ECC_DATA_HDR_0_OVERLAY_E2ECC_DATA_HDR_0_HI_FIELD =
{
    "E2ECC_DATA_HDR_0_HI",
#if RU_INCLUDE_DESC
    "",
    "32 upper bits of the E2ECC_DATA_HDR_0",
#endif
    LPORT_XLMAC_E2ECC_DATA_HDR_0_OVERLAY_E2ECC_DATA_HDR_0_HI_FIELD_MASK,
    0,
    LPORT_XLMAC_E2ECC_DATA_HDR_0_OVERLAY_E2ECC_DATA_HDR_0_HI_FIELD_WIDTH,
    LPORT_XLMAC_E2ECC_DATA_HDR_0_OVERLAY_E2ECC_DATA_HDR_0_HI_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_E2ECC_DATA_HDR_0_OVERLAY_E2ECC_DATA_HDR_0_LO
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_E2ECC_DATA_HDR_0_OVERLAY_E2ECC_DATA_HDR_0_LO_FIELD =
{
    "E2ECC_DATA_HDR_0_LO",
#if RU_INCLUDE_DESC
    "",
    "32 lower bits of the E2ECC_DATA_HDR_0",
#endif
    LPORT_XLMAC_E2ECC_DATA_HDR_0_OVERLAY_E2ECC_DATA_HDR_0_LO_FIELD_MASK,
    0,
    LPORT_XLMAC_E2ECC_DATA_HDR_0_OVERLAY_E2ECC_DATA_HDR_0_LO_FIELD_WIDTH,
    LPORT_XLMAC_E2ECC_DATA_HDR_0_OVERLAY_E2ECC_DATA_HDR_0_LO_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_E2ECC_DATA_HDR_1_E2ECC_DATA_HDR_1
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_E2ECC_DATA_HDR_1_E2ECC_DATA_HDR_1_FIELD =
{
    "E2ECC_DATA_HDR_1",
#if RU_INCLUDE_DESC
    "",
    "This register contains bits 63:0 of 16-byte IEEE header (DA + SA + Length/Type + Opcode).\n"
    "This field is used for constructing the Ethernet header for HiGig2/HiGig+ E2ECC frames in the transmit direction.",
#endif
    LPORT_XLMAC_E2ECC_DATA_HDR_1_E2ECC_DATA_HDR_1_FIELD_MASK,
    0,
    LPORT_XLMAC_E2ECC_DATA_HDR_1_E2ECC_DATA_HDR_1_FIELD_WIDTH,
    LPORT_XLMAC_E2ECC_DATA_HDR_1_E2ECC_DATA_HDR_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_E2ECC_DATA_HDR_1_OVERLAY_E2ECC_DATA_HDR_1_HI
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_E2ECC_DATA_HDR_1_OVERLAY_E2ECC_DATA_HDR_1_HI_FIELD =
{
    "E2ECC_DATA_HDR_1_HI",
#if RU_INCLUDE_DESC
    "",
    "32 upper bits of the E2ECC_DATA_HDR_1",
#endif
    LPORT_XLMAC_E2ECC_DATA_HDR_1_OVERLAY_E2ECC_DATA_HDR_1_HI_FIELD_MASK,
    0,
    LPORT_XLMAC_E2ECC_DATA_HDR_1_OVERLAY_E2ECC_DATA_HDR_1_HI_FIELD_WIDTH,
    LPORT_XLMAC_E2ECC_DATA_HDR_1_OVERLAY_E2ECC_DATA_HDR_1_HI_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_E2ECC_DATA_HDR_1_OVERLAY_E2ECC_DATA_HDR_1_LO
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_E2ECC_DATA_HDR_1_OVERLAY_E2ECC_DATA_HDR_1_LO_FIELD =
{
    "E2ECC_DATA_HDR_1_LO",
#if RU_INCLUDE_DESC
    "",
    "32 lower bits of the E2ECC_DATA_HDR_1",
#endif
    LPORT_XLMAC_E2ECC_DATA_HDR_1_OVERLAY_E2ECC_DATA_HDR_1_LO_FIELD_MASK,
    0,
    LPORT_XLMAC_E2ECC_DATA_HDR_1_OVERLAY_E2ECC_DATA_HDR_1_LO_FIELD_WIDTH,
    LPORT_XLMAC_E2ECC_DATA_HDR_1_OVERLAY_E2ECC_DATA_HDR_1_LO_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_E2EFC_MODULE_HDR_0_E2EFC_MODULE_HDR_0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_E2EFC_MODULE_HDR_0_E2EFC_MODULE_HDR_0_FIELD =
{
    "E2EFC_MODULE_HDR_0",
#if RU_INCLUDE_DESC
    "",
    "In HiGig2 mode, this register contains bits 127:64 of 16-byte HiGig2 header. \n"
    "In HiGig+ mode, this register contains bits 95:32 of 12-byte HiGig+ header.\n"
    "This field is used for constructing the module header for HiGig2/HiGig+ E2EFC frames in the transmit direction.",
#endif
    LPORT_XLMAC_E2EFC_MODULE_HDR_0_E2EFC_MODULE_HDR_0_FIELD_MASK,
    0,
    LPORT_XLMAC_E2EFC_MODULE_HDR_0_E2EFC_MODULE_HDR_0_FIELD_WIDTH,
    LPORT_XLMAC_E2EFC_MODULE_HDR_0_E2EFC_MODULE_HDR_0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_E2EFC_MODULE_HDR_0_OVERLAY_E2EFC_MODULE_HDR_0_HI
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_E2EFC_MODULE_HDR_0_OVERLAY_E2EFC_MODULE_HDR_0_HI_FIELD =
{
    "E2EFC_MODULE_HDR_0_HI",
#if RU_INCLUDE_DESC
    "",
    "32 upper bits of the E2EFC_MODULE_HDR_0",
#endif
    LPORT_XLMAC_E2EFC_MODULE_HDR_0_OVERLAY_E2EFC_MODULE_HDR_0_HI_FIELD_MASK,
    0,
    LPORT_XLMAC_E2EFC_MODULE_HDR_0_OVERLAY_E2EFC_MODULE_HDR_0_HI_FIELD_WIDTH,
    LPORT_XLMAC_E2EFC_MODULE_HDR_0_OVERLAY_E2EFC_MODULE_HDR_0_HI_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_E2EFC_MODULE_HDR_0_OVERLAY_E2EFC_MODULE_HDR_0_LO
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_E2EFC_MODULE_HDR_0_OVERLAY_E2EFC_MODULE_HDR_0_LO_FIELD =
{
    "E2EFC_MODULE_HDR_0_LO",
#if RU_INCLUDE_DESC
    "",
    "32 lower bits of the E2EFC_MODULE_HDR_0",
#endif
    LPORT_XLMAC_E2EFC_MODULE_HDR_0_OVERLAY_E2EFC_MODULE_HDR_0_LO_FIELD_MASK,
    0,
    LPORT_XLMAC_E2EFC_MODULE_HDR_0_OVERLAY_E2EFC_MODULE_HDR_0_LO_FIELD_WIDTH,
    LPORT_XLMAC_E2EFC_MODULE_HDR_0_OVERLAY_E2EFC_MODULE_HDR_0_LO_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_E2EFC_MODULE_HDR_1_E2EFC_MODULE_HDR_1
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_E2EFC_MODULE_HDR_1_E2EFC_MODULE_HDR_1_FIELD =
{
    "E2EFC_MODULE_HDR_1",
#if RU_INCLUDE_DESC
    "",
    "In HiGig2 mode, this register contains bits 63:0 of 16-byte HiGig2 header.\n"
    "In HiGig+ mode, bits 63:32 of this register contains bits 31:0 of 12-byte HiGig+ header.\n"
    "This field is used for constructing the module header for HiGig2/HiGig+ E2EFC frames in the transmit direction.",
#endif
    LPORT_XLMAC_E2EFC_MODULE_HDR_1_E2EFC_MODULE_HDR_1_FIELD_MASK,
    0,
    LPORT_XLMAC_E2EFC_MODULE_HDR_1_E2EFC_MODULE_HDR_1_FIELD_WIDTH,
    LPORT_XLMAC_E2EFC_MODULE_HDR_1_E2EFC_MODULE_HDR_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_E2EFC_MODULE_HDR_1_OVERLAY_E2EFC_MODULE_HDR_1_HI
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_E2EFC_MODULE_HDR_1_OVERLAY_E2EFC_MODULE_HDR_1_HI_FIELD =
{
    "E2EFC_MODULE_HDR_1_HI",
#if RU_INCLUDE_DESC
    "",
    "32 upper bits of the E2EFC_MODULE_HDR_1",
#endif
    LPORT_XLMAC_E2EFC_MODULE_HDR_1_OVERLAY_E2EFC_MODULE_HDR_1_HI_FIELD_MASK,
    0,
    LPORT_XLMAC_E2EFC_MODULE_HDR_1_OVERLAY_E2EFC_MODULE_HDR_1_HI_FIELD_WIDTH,
    LPORT_XLMAC_E2EFC_MODULE_HDR_1_OVERLAY_E2EFC_MODULE_HDR_1_HI_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_E2EFC_MODULE_HDR_1_OVERLAY_E2EFC_MODULE_HDR_1_LO
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_E2EFC_MODULE_HDR_1_OVERLAY_E2EFC_MODULE_HDR_1_LO_FIELD =
{
    "E2EFC_MODULE_HDR_1_LO",
#if RU_INCLUDE_DESC
    "",
    "32 lower bits of the E2EFC_MODULE_HDR_1",
#endif
    LPORT_XLMAC_E2EFC_MODULE_HDR_1_OVERLAY_E2EFC_MODULE_HDR_1_LO_FIELD_MASK,
    0,
    LPORT_XLMAC_E2EFC_MODULE_HDR_1_OVERLAY_E2EFC_MODULE_HDR_1_LO_FIELD_WIDTH,
    LPORT_XLMAC_E2EFC_MODULE_HDR_1_OVERLAY_E2EFC_MODULE_HDR_1_LO_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_E2EFC_DATA_HDR_0_E2EFC_DATA_HDR_0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_E2EFC_DATA_HDR_0_E2EFC_DATA_HDR_0_FIELD =
{
    "E2EFC_DATA_HDR_0",
#if RU_INCLUDE_DESC
    "",
    "This register contains bits 127:64 of 16-byte IEEE header (DA + SA + Length/Type + Opcode).\n"
    "This field is used for constructing the Ethernet header for HiGig2/HiGig+ E2EFC frames in the transmit direction.",
#endif
    LPORT_XLMAC_E2EFC_DATA_HDR_0_E2EFC_DATA_HDR_0_FIELD_MASK,
    0,
    LPORT_XLMAC_E2EFC_DATA_HDR_0_E2EFC_DATA_HDR_0_FIELD_WIDTH,
    LPORT_XLMAC_E2EFC_DATA_HDR_0_E2EFC_DATA_HDR_0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_E2EFC_DATA_HDR_0_OVERLAY_E2EFC_DATA_HDR_0_HI
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_E2EFC_DATA_HDR_0_OVERLAY_E2EFC_DATA_HDR_0_HI_FIELD =
{
    "E2EFC_DATA_HDR_0_HI",
#if RU_INCLUDE_DESC
    "",
    "32 upper bits of the E2EFC_DATA_HDR_0",
#endif
    LPORT_XLMAC_E2EFC_DATA_HDR_0_OVERLAY_E2EFC_DATA_HDR_0_HI_FIELD_MASK,
    0,
    LPORT_XLMAC_E2EFC_DATA_HDR_0_OVERLAY_E2EFC_DATA_HDR_0_HI_FIELD_WIDTH,
    LPORT_XLMAC_E2EFC_DATA_HDR_0_OVERLAY_E2EFC_DATA_HDR_0_HI_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_E2EFC_DATA_HDR_0_OVERLAY_E2EFC_DATA_HDR_0_LO
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_E2EFC_DATA_HDR_0_OVERLAY_E2EFC_DATA_HDR_0_LO_FIELD =
{
    "E2EFC_DATA_HDR_0_LO",
#if RU_INCLUDE_DESC
    "",
    "32 lower bits of the E2EFC_DATA_HDR_0",
#endif
    LPORT_XLMAC_E2EFC_DATA_HDR_0_OVERLAY_E2EFC_DATA_HDR_0_LO_FIELD_MASK,
    0,
    LPORT_XLMAC_E2EFC_DATA_HDR_0_OVERLAY_E2EFC_DATA_HDR_0_LO_FIELD_WIDTH,
    LPORT_XLMAC_E2EFC_DATA_HDR_0_OVERLAY_E2EFC_DATA_HDR_0_LO_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_E2EFC_DATA_HDR_1_E2EFC_DATA_HDR_1
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_E2EFC_DATA_HDR_1_E2EFC_DATA_HDR_1_FIELD =
{
    "E2EFC_DATA_HDR_1",
#if RU_INCLUDE_DESC
    "",
    "This register contains bits 63:0 of 16-byte IEEE header (DA + SA + Length/Type + Opcode).\n"
    "This field is used for constructing the Ethernet header for HiGig2/HiGig+ E2EFC frames in the transmit direction.",
#endif
    LPORT_XLMAC_E2EFC_DATA_HDR_1_E2EFC_DATA_HDR_1_FIELD_MASK,
    0,
    LPORT_XLMAC_E2EFC_DATA_HDR_1_E2EFC_DATA_HDR_1_FIELD_WIDTH,
    LPORT_XLMAC_E2EFC_DATA_HDR_1_E2EFC_DATA_HDR_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_E2EFC_DATA_HDR_1_OVERLAY_E2EFC_DATA_HDR_1_HI
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_E2EFC_DATA_HDR_1_OVERLAY_E2EFC_DATA_HDR_1_HI_FIELD =
{
    "E2EFC_DATA_HDR_1_HI",
#if RU_INCLUDE_DESC
    "",
    "32 upper bits of the E2EFC_DATA_HDR_1",
#endif
    LPORT_XLMAC_E2EFC_DATA_HDR_1_OVERLAY_E2EFC_DATA_HDR_1_HI_FIELD_MASK,
    0,
    LPORT_XLMAC_E2EFC_DATA_HDR_1_OVERLAY_E2EFC_DATA_HDR_1_HI_FIELD_WIDTH,
    LPORT_XLMAC_E2EFC_DATA_HDR_1_OVERLAY_E2EFC_DATA_HDR_1_HI_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_E2EFC_DATA_HDR_1_OVERLAY_E2EFC_DATA_HDR_1_LO
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_E2EFC_DATA_HDR_1_OVERLAY_E2EFC_DATA_HDR_1_LO_FIELD =
{
    "E2EFC_DATA_HDR_1_LO",
#if RU_INCLUDE_DESC
    "",
    "32 lower bits of the E2EFC_DATA_HDR_1",
#endif
    LPORT_XLMAC_E2EFC_DATA_HDR_1_OVERLAY_E2EFC_DATA_HDR_1_LO_FIELD_MASK,
    0,
    LPORT_XLMAC_E2EFC_DATA_HDR_1_OVERLAY_E2EFC_DATA_HDR_1_LO_FIELD_WIDTH,
    LPORT_XLMAC_E2EFC_DATA_HDR_1_OVERLAY_E2EFC_DATA_HDR_1_LO_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TXFIFO_CELL_CNT_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TXFIFO_CELL_CNT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_TXFIFO_CELL_CNT_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_TXFIFO_CELL_CNT_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_TXFIFO_CELL_CNT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TXFIFO_CELL_CNT_CELL_CNT
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TXFIFO_CELL_CNT_CELL_CNT_FIELD =
{
    "CELL_CNT",
#if RU_INCLUDE_DESC
    "",
    "Number of cell counts in XLMAC TX FIFO.\n"
    "Should range from 0 to 32 for XLMAC core in single port mode, or 0 to 16 if XLMAC core is in dual port mode, or 0 to 8 if XLMAC core is in quad port mode during traffic.\n"
    "This should reset to 0 after the traffic has stopped for all port modes.",
#endif
    LPORT_XLMAC_TXFIFO_CELL_CNT_CELL_CNT_FIELD_MASK,
    0,
    LPORT_XLMAC_TXFIFO_CELL_CNT_CELL_CNT_FIELD_WIDTH,
    LPORT_XLMAC_TXFIFO_CELL_CNT_CELL_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TXFIFO_CELL_REQ_CNT_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TXFIFO_CELL_REQ_CNT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_TXFIFO_CELL_REQ_CNT_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_TXFIFO_CELL_REQ_CNT_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_TXFIFO_CELL_REQ_CNT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TXFIFO_CELL_REQ_CNT_REQ_CNT
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TXFIFO_CELL_REQ_CNT_REQ_CNT_FIELD =
{
    "REQ_CNT",
#if RU_INCLUDE_DESC
    "",
    "Number of cell requests made to Egress Pipeline. \n"
    "Should range from 0 to 32 for XLMAC core in single port mode, or 0 to 16 if XLMAC core is in dual port mode, or 0 to 8 if XLMAC core is in quad port mode during traffic.\n"
    "This should saturate at the maximum value for the corresponding port mode after traffic has stopped.",
#endif
    LPORT_XLMAC_TXFIFO_CELL_REQ_CNT_REQ_CNT_FIELD_MASK,
    0,
    LPORT_XLMAC_TXFIFO_CELL_REQ_CNT_REQ_CNT_FIELD_WIDTH,
    LPORT_XLMAC_TXFIFO_CELL_REQ_CNT_REQ_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_MEM_CTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_MEM_CTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_MEM_CTRL_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_MEM_CTRL_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_MEM_CTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_MEM_CTRL_TX_CDC_MEM_CTRL_TM
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_MEM_CTRL_TX_CDC_MEM_CTRL_TM_FIELD =
{
    "TX_CDC_MEM_CTRL_TM",
#if RU_INCLUDE_DESC
    "",
    "Test mode configuration of Tx CDC Memory",
#endif
    LPORT_XLMAC_MEM_CTRL_TX_CDC_MEM_CTRL_TM_FIELD_MASK,
    0,
    LPORT_XLMAC_MEM_CTRL_TX_CDC_MEM_CTRL_TM_FIELD_WIDTH,
    LPORT_XLMAC_MEM_CTRL_TX_CDC_MEM_CTRL_TM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_MEM_CTRL_RX_CDC_MEM_CTRL_TM
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_MEM_CTRL_RX_CDC_MEM_CTRL_TM_FIELD =
{
    "RX_CDC_MEM_CTRL_TM",
#if RU_INCLUDE_DESC
    "",
    "Test mode configuration of Rx CDC Memory.",
#endif
    LPORT_XLMAC_MEM_CTRL_RX_CDC_MEM_CTRL_TM_FIELD_MASK,
    0,
    LPORT_XLMAC_MEM_CTRL_RX_CDC_MEM_CTRL_TM_FIELD_WIDTH,
    LPORT_XLMAC_MEM_CTRL_RX_CDC_MEM_CTRL_TM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_ECC_CTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_ECC_CTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_ECC_CTRL_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_ECC_CTRL_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_ECC_CTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_ECC_CTRL_TX_CDC_ECC_CTRL_EN
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_ECC_CTRL_TX_CDC_ECC_CTRL_EN_FIELD =
{
    "TX_CDC_ECC_CTRL_EN",
#if RU_INCLUDE_DESC
    "",
    "When set, MAC enables Tx CDC memory ECC logic",
#endif
    LPORT_XLMAC_ECC_CTRL_TX_CDC_ECC_CTRL_EN_FIELD_MASK,
    0,
    LPORT_XLMAC_ECC_CTRL_TX_CDC_ECC_CTRL_EN_FIELD_WIDTH,
    LPORT_XLMAC_ECC_CTRL_TX_CDC_ECC_CTRL_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_ECC_CTRL_RX_CDC_ECC_CTRL_EN
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_ECC_CTRL_RX_CDC_ECC_CTRL_EN_FIELD =
{
    "RX_CDC_ECC_CTRL_EN",
#if RU_INCLUDE_DESC
    "",
    "When set, MAC enables Rx CDC memory ECC logic",
#endif
    LPORT_XLMAC_ECC_CTRL_RX_CDC_ECC_CTRL_EN_FIELD_MASK,
    0,
    LPORT_XLMAC_ECC_CTRL_RX_CDC_ECC_CTRL_EN_FIELD_WIDTH,
    LPORT_XLMAC_ECC_CTRL_RX_CDC_ECC_CTRL_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_ECC_FORCE_DOUBLE_BIT_ERR_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_ECC_FORCE_DOUBLE_BIT_ERR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_ECC_FORCE_DOUBLE_BIT_ERR_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_ECC_FORCE_DOUBLE_BIT_ERR_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_ECC_FORCE_DOUBLE_BIT_ERR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_ECC_FORCE_DOUBLE_BIT_ERR_TX_CDC_FORCE_DOUBLE_BIT_ERR
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_ECC_FORCE_DOUBLE_BIT_ERR_TX_CDC_FORCE_DOUBLE_BIT_ERR_FIELD =
{
    "TX_CDC_FORCE_DOUBLE_BIT_ERR",
#if RU_INCLUDE_DESC
    "",
    "Tx CDC memory force double bit error enable. The LSB 2 bits will be inverted at the next memory read.\n"
    "This should never be asserted simultaneously with TX_CDC_FORCE_SINGLE_BIT_ERR.\n"
    "In order to inject double bit error again, this bit needs to be written to 0 before being re-written to 1.",
#endif
    LPORT_XLMAC_ECC_FORCE_DOUBLE_BIT_ERR_TX_CDC_FORCE_DOUBLE_BIT_ERR_FIELD_MASK,
    0,
    LPORT_XLMAC_ECC_FORCE_DOUBLE_BIT_ERR_TX_CDC_FORCE_DOUBLE_BIT_ERR_FIELD_WIDTH,
    LPORT_XLMAC_ECC_FORCE_DOUBLE_BIT_ERR_TX_CDC_FORCE_DOUBLE_BIT_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_ECC_FORCE_DOUBLE_BIT_ERR_RX_CDC_FORCE_DOUBLE_BIT_ERR
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_ECC_FORCE_DOUBLE_BIT_ERR_RX_CDC_FORCE_DOUBLE_BIT_ERR_FIELD =
{
    "RX_CDC_FORCE_DOUBLE_BIT_ERR",
#if RU_INCLUDE_DESC
    "",
    "Rx CDC memory force double bit error enable. The LSB 2 bits will be inverted at the next memory read.\n"
    "This should never be asserted simultaneously with force RX_CDC_FORCE_SINGLE_BIT_ERR.\n"
    "In order to inject double bit error again, this bit needs to be written to 0 before being re-written to 1.",
#endif
    LPORT_XLMAC_ECC_FORCE_DOUBLE_BIT_ERR_RX_CDC_FORCE_DOUBLE_BIT_ERR_FIELD_MASK,
    0,
    LPORT_XLMAC_ECC_FORCE_DOUBLE_BIT_ERR_RX_CDC_FORCE_DOUBLE_BIT_ERR_FIELD_WIDTH,
    LPORT_XLMAC_ECC_FORCE_DOUBLE_BIT_ERR_RX_CDC_FORCE_DOUBLE_BIT_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_ECC_FORCE_SINGLE_BIT_ERR_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_ECC_FORCE_SINGLE_BIT_ERR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_ECC_FORCE_SINGLE_BIT_ERR_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_ECC_FORCE_SINGLE_BIT_ERR_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_ECC_FORCE_SINGLE_BIT_ERR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_ECC_FORCE_SINGLE_BIT_ERR_TX_CDC_FORCE_SINGLE_BIT_ERR
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_ECC_FORCE_SINGLE_BIT_ERR_TX_CDC_FORCE_SINGLE_BIT_ERR_FIELD =
{
    "TX_CDC_FORCE_SINGLE_BIT_ERR",
#if RU_INCLUDE_DESC
    "",
    "Tx CDC memory force single bit error enable. The LSB 1 bit will be inverted at the next memory read.\n"
    "This should never be asserted simultaneously with TX_CDC_FORCE_DOUBLE_BIT_ERR.\n"
    "In order to inject single bit error again, this bit needs to be written to 0 before being re-written to 1.",
#endif
    LPORT_XLMAC_ECC_FORCE_SINGLE_BIT_ERR_TX_CDC_FORCE_SINGLE_BIT_ERR_FIELD_MASK,
    0,
    LPORT_XLMAC_ECC_FORCE_SINGLE_BIT_ERR_TX_CDC_FORCE_SINGLE_BIT_ERR_FIELD_WIDTH,
    LPORT_XLMAC_ECC_FORCE_SINGLE_BIT_ERR_TX_CDC_FORCE_SINGLE_BIT_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_ECC_FORCE_SINGLE_BIT_ERR_RX_CDC_FORCE_SINGLE_BIT_ERR
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_ECC_FORCE_SINGLE_BIT_ERR_RX_CDC_FORCE_SINGLE_BIT_ERR_FIELD =
{
    "RX_CDC_FORCE_SINGLE_BIT_ERR",
#if RU_INCLUDE_DESC
    "",
    "Rx CDC memory force single bit error enable. The LSB 1 bit will be inverted at the next memory read.\n"
    "This should never be asserted simultaneously with RX_CDC_FORCE_DOUBLE_BIT_ERR.\n"
    "In order to inject single bit error again, this bit needs to be written to 0 before being re-written to 1.",
#endif
    LPORT_XLMAC_ECC_FORCE_SINGLE_BIT_ERR_RX_CDC_FORCE_SINGLE_BIT_ERR_FIELD_MASK,
    0,
    LPORT_XLMAC_ECC_FORCE_SINGLE_BIT_ERR_RX_CDC_FORCE_SINGLE_BIT_ERR_FIELD_WIDTH,
    LPORT_XLMAC_ECC_FORCE_SINGLE_BIT_ERR_RX_CDC_FORCE_SINGLE_BIT_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_RX_CDC_ECC_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_RX_CDC_ECC_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_RX_CDC_ECC_STATUS_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_RX_CDC_ECC_STATUS_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_RX_CDC_ECC_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_RX_CDC_ECC_STATUS_RX_CDC_DOUBLE_BIT_ERR
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_RX_CDC_ECC_STATUS_RX_CDC_DOUBLE_BIT_ERR_FIELD =
{
    "RX_CDC_DOUBLE_BIT_ERR",
#if RU_INCLUDE_DESC
    "",
    "This status bit indicates a double bit error occurred in the Rx CDC memory",
#endif
    LPORT_XLMAC_RX_CDC_ECC_STATUS_RX_CDC_DOUBLE_BIT_ERR_FIELD_MASK,
    0,
    LPORT_XLMAC_RX_CDC_ECC_STATUS_RX_CDC_DOUBLE_BIT_ERR_FIELD_WIDTH,
    LPORT_XLMAC_RX_CDC_ECC_STATUS_RX_CDC_DOUBLE_BIT_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_RX_CDC_ECC_STATUS_RX_CDC_SINGLE_BIT_ERR
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_RX_CDC_ECC_STATUS_RX_CDC_SINGLE_BIT_ERR_FIELD =
{
    "RX_CDC_SINGLE_BIT_ERR",
#if RU_INCLUDE_DESC
    "",
    "This status bit indicates a single bit error occurred in the Rx CDC memory",
#endif
    LPORT_XLMAC_RX_CDC_ECC_STATUS_RX_CDC_SINGLE_BIT_ERR_FIELD_MASK,
    0,
    LPORT_XLMAC_RX_CDC_ECC_STATUS_RX_CDC_SINGLE_BIT_ERR_FIELD_WIDTH,
    LPORT_XLMAC_RX_CDC_ECC_STATUS_RX_CDC_SINGLE_BIT_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TX_CDC_ECC_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TX_CDC_ECC_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_TX_CDC_ECC_STATUS_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_TX_CDC_ECC_STATUS_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_TX_CDC_ECC_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TX_CDC_ECC_STATUS_TX_CDC_DOUBLE_BIT_ERR
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TX_CDC_ECC_STATUS_TX_CDC_DOUBLE_BIT_ERR_FIELD =
{
    "TX_CDC_DOUBLE_BIT_ERR",
#if RU_INCLUDE_DESC
    "",
    "This status bit indicates a double bit error occurred in the Tx CDC memory",
#endif
    LPORT_XLMAC_TX_CDC_ECC_STATUS_TX_CDC_DOUBLE_BIT_ERR_FIELD_MASK,
    0,
    LPORT_XLMAC_TX_CDC_ECC_STATUS_TX_CDC_DOUBLE_BIT_ERR_FIELD_WIDTH,
    LPORT_XLMAC_TX_CDC_ECC_STATUS_TX_CDC_DOUBLE_BIT_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_TX_CDC_ECC_STATUS_TX_CDC_SINGLE_BIT_ERR
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_TX_CDC_ECC_STATUS_TX_CDC_SINGLE_BIT_ERR_FIELD =
{
    "TX_CDC_SINGLE_BIT_ERR",
#if RU_INCLUDE_DESC
    "",
    "This status bit indicates a single bit error occurred in the Tx CDC memory",
#endif
    LPORT_XLMAC_TX_CDC_ECC_STATUS_TX_CDC_SINGLE_BIT_ERR_FIELD_MASK,
    0,
    LPORT_XLMAC_TX_CDC_ECC_STATUS_TX_CDC_SINGLE_BIT_ERR_FIELD_WIDTH,
    LPORT_XLMAC_TX_CDC_ECC_STATUS_TX_CDC_SINGLE_BIT_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_CLEAR_ECC_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_CLEAR_ECC_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_CLEAR_ECC_STATUS_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_CLEAR_ECC_STATUS_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_CLEAR_ECC_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_CLEAR_ECC_STATUS_CLEAR_TX_CDC_DOUBLE_BIT_ERR
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_CLEAR_ECC_STATUS_CLEAR_TX_CDC_DOUBLE_BIT_ERR_FIELD =
{
    "CLEAR_TX_CDC_DOUBLE_BIT_ERR",
#if RU_INCLUDE_DESC
    "",
    "A rising edge on this register bit (0->1), clears the sticky TX_CDC_DOUBLE_BIT_ERR status bit",
#endif
    LPORT_XLMAC_CLEAR_ECC_STATUS_CLEAR_TX_CDC_DOUBLE_BIT_ERR_FIELD_MASK,
    0,
    LPORT_XLMAC_CLEAR_ECC_STATUS_CLEAR_TX_CDC_DOUBLE_BIT_ERR_FIELD_WIDTH,
    LPORT_XLMAC_CLEAR_ECC_STATUS_CLEAR_TX_CDC_DOUBLE_BIT_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_CLEAR_ECC_STATUS_CLEAR_TX_CDC_SINGLE_BIT_ERR
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_CLEAR_ECC_STATUS_CLEAR_TX_CDC_SINGLE_BIT_ERR_FIELD =
{
    "CLEAR_TX_CDC_SINGLE_BIT_ERR",
#if RU_INCLUDE_DESC
    "",
    "A rising edge on this register bit (0->1), clears the sticky TX_CDC_SINGLE_BIT_ERR status bit",
#endif
    LPORT_XLMAC_CLEAR_ECC_STATUS_CLEAR_TX_CDC_SINGLE_BIT_ERR_FIELD_MASK,
    0,
    LPORT_XLMAC_CLEAR_ECC_STATUS_CLEAR_TX_CDC_SINGLE_BIT_ERR_FIELD_WIDTH,
    LPORT_XLMAC_CLEAR_ECC_STATUS_CLEAR_TX_CDC_SINGLE_BIT_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_CLEAR_ECC_STATUS_CLEAR_RX_CDC_DOUBLE_BIT_ERR
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_CLEAR_ECC_STATUS_CLEAR_RX_CDC_DOUBLE_BIT_ERR_FIELD =
{
    "CLEAR_RX_CDC_DOUBLE_BIT_ERR",
#if RU_INCLUDE_DESC
    "",
    "A rising edge on this register bit (0->1), clears the sticky RX_CDC_DOUBLE_BIT_ERR status bit",
#endif
    LPORT_XLMAC_CLEAR_ECC_STATUS_CLEAR_RX_CDC_DOUBLE_BIT_ERR_FIELD_MASK,
    0,
    LPORT_XLMAC_CLEAR_ECC_STATUS_CLEAR_RX_CDC_DOUBLE_BIT_ERR_FIELD_WIDTH,
    LPORT_XLMAC_CLEAR_ECC_STATUS_CLEAR_RX_CDC_DOUBLE_BIT_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_CLEAR_ECC_STATUS_CLEAR_RX_CDC_SINGLE_BIT_ERR
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_CLEAR_ECC_STATUS_CLEAR_RX_CDC_SINGLE_BIT_ERR_FIELD =
{
    "CLEAR_RX_CDC_SINGLE_BIT_ERR",
#if RU_INCLUDE_DESC
    "",
    "A rising edge on this register bit (0->1), clears the sticky RX_CDC_SINGLE_BIT_ERR status bit",
#endif
    LPORT_XLMAC_CLEAR_ECC_STATUS_CLEAR_RX_CDC_SINGLE_BIT_ERR_FIELD_MASK,
    0,
    LPORT_XLMAC_CLEAR_ECC_STATUS_CLEAR_RX_CDC_SINGLE_BIT_ERR_FIELD_WIDTH,
    LPORT_XLMAC_CLEAR_ECC_STATUS_CLEAR_RX_CDC_SINGLE_BIT_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_INTR_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_INTR_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_INTR_STATUS_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_INTR_STATUS_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_INTR_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_INTR_STATUS_SUM_TS_ENTRY_VALID
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_INTR_STATUS_SUM_TS_ENTRY_VALID_FIELD =
{
    "SUM_TS_ENTRY_VALID",
#if RU_INCLUDE_DESC
    "",
    "Active high qualifier for the TimeStamp & SEQUENCE_ID fields.",
#endif
    LPORT_XLMAC_INTR_STATUS_SUM_TS_ENTRY_VALID_FIELD_MASK,
    0,
    LPORT_XLMAC_INTR_STATUS_SUM_TS_ENTRY_VALID_FIELD_WIDTH,
    LPORT_XLMAC_INTR_STATUS_SUM_TS_ENTRY_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_INTR_STATUS_SUM_LINK_INTERRUPTION_STATUS
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_INTR_STATUS_SUM_LINK_INTERRUPTION_STATUS_FIELD =
{
    "SUM_LINK_INTERRUPTION_STATUS",
#if RU_INCLUDE_DESC
    "",
    "True when link interruption state is detected as per RS layer state machine. Sticky bit is cleared by CLEAR_LINK_INTERRUPTION_STATUS.",
#endif
    LPORT_XLMAC_INTR_STATUS_SUM_LINK_INTERRUPTION_STATUS_FIELD_MASK,
    0,
    LPORT_XLMAC_INTR_STATUS_SUM_LINK_INTERRUPTION_STATUS_FIELD_WIDTH,
    LPORT_XLMAC_INTR_STATUS_SUM_LINK_INTERRUPTION_STATUS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_INTR_STATUS_SUM_REMOTE_FAULT_STATUS
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_INTR_STATUS_SUM_REMOTE_FAULT_STATUS_FIELD =
{
    "SUM_REMOTE_FAULT_STATUS",
#if RU_INCLUDE_DESC
    "",
    "True when remote fault state is detected as per RS layer state machine. Sticky bit is cleared by CLEAR_REMOTE_FAULT_STATUS.",
#endif
    LPORT_XLMAC_INTR_STATUS_SUM_REMOTE_FAULT_STATUS_FIELD_MASK,
    0,
    LPORT_XLMAC_INTR_STATUS_SUM_REMOTE_FAULT_STATUS_FIELD_WIDTH,
    LPORT_XLMAC_INTR_STATUS_SUM_REMOTE_FAULT_STATUS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_INTR_STATUS_SUM_LOCAL_FAULT_STATUS
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_INTR_STATUS_SUM_LOCAL_FAULT_STATUS_FIELD =
{
    "SUM_LOCAL_FAULT_STATUS",
#if RU_INCLUDE_DESC
    "",
    "True when local fault state is detected as per RS layer state machine. Sticky bit is cleared by CLEAR_LOCAL_FAULT_STATUS",
#endif
    LPORT_XLMAC_INTR_STATUS_SUM_LOCAL_FAULT_STATUS_FIELD_MASK,
    0,
    LPORT_XLMAC_INTR_STATUS_SUM_LOCAL_FAULT_STATUS_FIELD_WIDTH,
    LPORT_XLMAC_INTR_STATUS_SUM_LOCAL_FAULT_STATUS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_INTR_STATUS_SUM_RX_CDC_DOUBLE_BIT_ERR
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_INTR_STATUS_SUM_RX_CDC_DOUBLE_BIT_ERR_FIELD =
{
    "SUM_RX_CDC_DOUBLE_BIT_ERR",
#if RU_INCLUDE_DESC
    "",
    "This status bit indicates a double bit error occurred in the Rx CDC memory",
#endif
    LPORT_XLMAC_INTR_STATUS_SUM_RX_CDC_DOUBLE_BIT_ERR_FIELD_MASK,
    0,
    LPORT_XLMAC_INTR_STATUS_SUM_RX_CDC_DOUBLE_BIT_ERR_FIELD_WIDTH,
    LPORT_XLMAC_INTR_STATUS_SUM_RX_CDC_DOUBLE_BIT_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_INTR_STATUS_SUM_RX_CDC_SINGLE_BIT_ERR
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_INTR_STATUS_SUM_RX_CDC_SINGLE_BIT_ERR_FIELD =
{
    "SUM_RX_CDC_SINGLE_BIT_ERR",
#if RU_INCLUDE_DESC
    "",
    "This status bit indicates a single bit error occurred in the Rx CDC memory",
#endif
    LPORT_XLMAC_INTR_STATUS_SUM_RX_CDC_SINGLE_BIT_ERR_FIELD_MASK,
    0,
    LPORT_XLMAC_INTR_STATUS_SUM_RX_CDC_SINGLE_BIT_ERR_FIELD_WIDTH,
    LPORT_XLMAC_INTR_STATUS_SUM_RX_CDC_SINGLE_BIT_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_INTR_STATUS_SUM_TX_CDC_DOUBLE_BIT_ERR
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_INTR_STATUS_SUM_TX_CDC_DOUBLE_BIT_ERR_FIELD =
{
    "SUM_TX_CDC_DOUBLE_BIT_ERR",
#if RU_INCLUDE_DESC
    "",
    "This status bit indicates a double bit error occurred in the Tx CDC memory",
#endif
    LPORT_XLMAC_INTR_STATUS_SUM_TX_CDC_DOUBLE_BIT_ERR_FIELD_MASK,
    0,
    LPORT_XLMAC_INTR_STATUS_SUM_TX_CDC_DOUBLE_BIT_ERR_FIELD_WIDTH,
    LPORT_XLMAC_INTR_STATUS_SUM_TX_CDC_DOUBLE_BIT_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_INTR_STATUS_SUM_TX_CDC_SINGLE_BIT_ERR
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_INTR_STATUS_SUM_TX_CDC_SINGLE_BIT_ERR_FIELD =
{
    "SUM_TX_CDC_SINGLE_BIT_ERR",
#if RU_INCLUDE_DESC
    "",
    "This status bit indicates a single bit error occurred in the Tx CDC memory",
#endif
    LPORT_XLMAC_INTR_STATUS_SUM_TX_CDC_SINGLE_BIT_ERR_FIELD_MASK,
    0,
    LPORT_XLMAC_INTR_STATUS_SUM_TX_CDC_SINGLE_BIT_ERR_FIELD_WIDTH,
    LPORT_XLMAC_INTR_STATUS_SUM_TX_CDC_SINGLE_BIT_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_INTR_STATUS_SUM_RX_MSG_OVERFLOW
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_INTR_STATUS_SUM_RX_MSG_OVERFLOW_FIELD =
{
    "SUM_RX_MSG_OVERFLOW",
#if RU_INCLUDE_DESC
    "",
    "If set, indicates rx message fifo overflow",
#endif
    LPORT_XLMAC_INTR_STATUS_SUM_RX_MSG_OVERFLOW_FIELD_MASK,
    0,
    LPORT_XLMAC_INTR_STATUS_SUM_RX_MSG_OVERFLOW_FIELD_WIDTH,
    LPORT_XLMAC_INTR_STATUS_SUM_RX_MSG_OVERFLOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_INTR_STATUS_SUM_RX_PKT_OVERFLOW
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_INTR_STATUS_SUM_RX_PKT_OVERFLOW_FIELD =
{
    "SUM_RX_PKT_OVERFLOW",
#if RU_INCLUDE_DESC
    "",
    "If set, indicates RX packet fifo overflow",
#endif
    LPORT_XLMAC_INTR_STATUS_SUM_RX_PKT_OVERFLOW_FIELD_MASK,
    0,
    LPORT_XLMAC_INTR_STATUS_SUM_RX_PKT_OVERFLOW_FIELD_WIDTH,
    LPORT_XLMAC_INTR_STATUS_SUM_RX_PKT_OVERFLOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_INTR_STATUS_SUM_TX_TS_FIFO_OVERFLOW
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_INTR_STATUS_SUM_TX_TS_FIFO_OVERFLOW_FIELD =
{
    "SUM_TX_TS_FIFO_OVERFLOW",
#if RU_INCLUDE_DESC
    "",
    "If set, indicates overflow occurred in TX two-step Time Stamp FIFO",
#endif
    LPORT_XLMAC_INTR_STATUS_SUM_TX_TS_FIFO_OVERFLOW_FIELD_MASK,
    0,
    LPORT_XLMAC_INTR_STATUS_SUM_TX_TS_FIFO_OVERFLOW_FIELD_WIDTH,
    LPORT_XLMAC_INTR_STATUS_SUM_TX_TS_FIFO_OVERFLOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_INTR_STATUS_SUM_TX_LLFC_MSG_OVERFLOW
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_INTR_STATUS_SUM_TX_LLFC_MSG_OVERFLOW_FIELD =
{
    "SUM_TX_LLFC_MSG_OVERFLOW",
#if RU_INCLUDE_DESC
    "",
    "If set, indicates TX LLFC message fifo overflow",
#endif
    LPORT_XLMAC_INTR_STATUS_SUM_TX_LLFC_MSG_OVERFLOW_FIELD_MASK,
    0,
    LPORT_XLMAC_INTR_STATUS_SUM_TX_LLFC_MSG_OVERFLOW_FIELD_WIDTH,
    LPORT_XLMAC_INTR_STATUS_SUM_TX_LLFC_MSG_OVERFLOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_INTR_STATUS_SUM_TX_PKT_OVERFLOW
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_INTR_STATUS_SUM_TX_PKT_OVERFLOW_FIELD =
{
    "SUM_TX_PKT_OVERFLOW",
#if RU_INCLUDE_DESC
    "",
    "If set, indicates tx packet fifo overflow",
#endif
    LPORT_XLMAC_INTR_STATUS_SUM_TX_PKT_OVERFLOW_FIELD_MASK,
    0,
    LPORT_XLMAC_INTR_STATUS_SUM_TX_PKT_OVERFLOW_FIELD_WIDTH,
    LPORT_XLMAC_INTR_STATUS_SUM_TX_PKT_OVERFLOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_INTR_STATUS_SUM_TX_PKT_UNDERFLOW
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_INTR_STATUS_SUM_TX_PKT_UNDERFLOW_FIELD =
{
    "SUM_TX_PKT_UNDERFLOW",
#if RU_INCLUDE_DESC
    "",
    "If set, indicates tx packet fifo underflow",
#endif
    LPORT_XLMAC_INTR_STATUS_SUM_TX_PKT_UNDERFLOW_FIELD_MASK,
    0,
    LPORT_XLMAC_INTR_STATUS_SUM_TX_PKT_UNDERFLOW_FIELD_WIDTH,
    LPORT_XLMAC_INTR_STATUS_SUM_TX_PKT_UNDERFLOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_INTR_ENABLE_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_INTR_ENABLE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_INTR_ENABLE_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_INTR_ENABLE_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_INTR_ENABLE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_INTR_ENABLE_EN_TS_ENTRY_VALID
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_INTR_ENABLE_EN_TS_ENTRY_VALID_FIELD =
{
    "EN_TS_ENTRY_VALID",
#if RU_INCLUDE_DESC
    "",
    "If set, SUM_TS_ENTRY_VALID can set mac interrupt.",
#endif
    LPORT_XLMAC_INTR_ENABLE_EN_TS_ENTRY_VALID_FIELD_MASK,
    0,
    LPORT_XLMAC_INTR_ENABLE_EN_TS_ENTRY_VALID_FIELD_WIDTH,
    LPORT_XLMAC_INTR_ENABLE_EN_TS_ENTRY_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_INTR_ENABLE_EN_LINK_INTERRUPTION_STATUS
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_INTR_ENABLE_EN_LINK_INTERRUPTION_STATUS_FIELD =
{
    "EN_LINK_INTERRUPTION_STATUS",
#if RU_INCLUDE_DESC
    "",
    "If set, SUM_LINK_INTERRUPTION_STATUS can set mac interrupt.",
#endif
    LPORT_XLMAC_INTR_ENABLE_EN_LINK_INTERRUPTION_STATUS_FIELD_MASK,
    0,
    LPORT_XLMAC_INTR_ENABLE_EN_LINK_INTERRUPTION_STATUS_FIELD_WIDTH,
    LPORT_XLMAC_INTR_ENABLE_EN_LINK_INTERRUPTION_STATUS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_INTR_ENABLE_EN_REMOTE_FAULT_STATUS
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_INTR_ENABLE_EN_REMOTE_FAULT_STATUS_FIELD =
{
    "EN_REMOTE_FAULT_STATUS",
#if RU_INCLUDE_DESC
    "",
    "If set, SUM_REMOTE_FAULT_STATUS can set mac interrupt.",
#endif
    LPORT_XLMAC_INTR_ENABLE_EN_REMOTE_FAULT_STATUS_FIELD_MASK,
    0,
    LPORT_XLMAC_INTR_ENABLE_EN_REMOTE_FAULT_STATUS_FIELD_WIDTH,
    LPORT_XLMAC_INTR_ENABLE_EN_REMOTE_FAULT_STATUS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_INTR_ENABLE_EN_LOCAL_FAULT_STATUS
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_INTR_ENABLE_EN_LOCAL_FAULT_STATUS_FIELD =
{
    "EN_LOCAL_FAULT_STATUS",
#if RU_INCLUDE_DESC
    "",
    "If set, SUM_LOCAL_FAULT_STATUS can set mac interrupt.",
#endif
    LPORT_XLMAC_INTR_ENABLE_EN_LOCAL_FAULT_STATUS_FIELD_MASK,
    0,
    LPORT_XLMAC_INTR_ENABLE_EN_LOCAL_FAULT_STATUS_FIELD_WIDTH,
    LPORT_XLMAC_INTR_ENABLE_EN_LOCAL_FAULT_STATUS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_INTR_ENABLE_EN_RX_CDC_DOUBLE_BIT_ERR
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_INTR_ENABLE_EN_RX_CDC_DOUBLE_BIT_ERR_FIELD =
{
    "EN_RX_CDC_DOUBLE_BIT_ERR",
#if RU_INCLUDE_DESC
    "",
    "If set, SUM_RX_CDC_DOUBLE_BIT_ERR can set mac interrupt.",
#endif
    LPORT_XLMAC_INTR_ENABLE_EN_RX_CDC_DOUBLE_BIT_ERR_FIELD_MASK,
    0,
    LPORT_XLMAC_INTR_ENABLE_EN_RX_CDC_DOUBLE_BIT_ERR_FIELD_WIDTH,
    LPORT_XLMAC_INTR_ENABLE_EN_RX_CDC_DOUBLE_BIT_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_INTR_ENABLE_EN_RX_CDC_SINGLE_BIT_ERR
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_INTR_ENABLE_EN_RX_CDC_SINGLE_BIT_ERR_FIELD =
{
    "EN_RX_CDC_SINGLE_BIT_ERR",
#if RU_INCLUDE_DESC
    "",
    "If set, SUM_RX_CDC_SINGLE_BIT_ERR can set mac interrupt.",
#endif
    LPORT_XLMAC_INTR_ENABLE_EN_RX_CDC_SINGLE_BIT_ERR_FIELD_MASK,
    0,
    LPORT_XLMAC_INTR_ENABLE_EN_RX_CDC_SINGLE_BIT_ERR_FIELD_WIDTH,
    LPORT_XLMAC_INTR_ENABLE_EN_RX_CDC_SINGLE_BIT_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_INTR_ENABLE_EN_TX_CDC_DOUBLE_BIT_ERR
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_INTR_ENABLE_EN_TX_CDC_DOUBLE_BIT_ERR_FIELD =
{
    "EN_TX_CDC_DOUBLE_BIT_ERR",
#if RU_INCLUDE_DESC
    "",
    "If set, SUM_TX_CDC_DOUBLE_BIT_ERR can set mac interrupt.",
#endif
    LPORT_XLMAC_INTR_ENABLE_EN_TX_CDC_DOUBLE_BIT_ERR_FIELD_MASK,
    0,
    LPORT_XLMAC_INTR_ENABLE_EN_TX_CDC_DOUBLE_BIT_ERR_FIELD_WIDTH,
    LPORT_XLMAC_INTR_ENABLE_EN_TX_CDC_DOUBLE_BIT_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_INTR_ENABLE_EN_TX_CDC_SINGLE_BIT_ERR
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_INTR_ENABLE_EN_TX_CDC_SINGLE_BIT_ERR_FIELD =
{
    "EN_TX_CDC_SINGLE_BIT_ERR",
#if RU_INCLUDE_DESC
    "",
    "If set, SUM_TX_CDC_SINGLE_BIT_ERR can set mac interrupt.",
#endif
    LPORT_XLMAC_INTR_ENABLE_EN_TX_CDC_SINGLE_BIT_ERR_FIELD_MASK,
    0,
    LPORT_XLMAC_INTR_ENABLE_EN_TX_CDC_SINGLE_BIT_ERR_FIELD_WIDTH,
    LPORT_XLMAC_INTR_ENABLE_EN_TX_CDC_SINGLE_BIT_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_INTR_ENABLE_EN_RX_MSG_OVERFLOW
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_INTR_ENABLE_EN_RX_MSG_OVERFLOW_FIELD =
{
    "EN_RX_MSG_OVERFLOW",
#if RU_INCLUDE_DESC
    "",
    "If set, SUM_RX_MSG_OVERFLOW can set mac interrupt.",
#endif
    LPORT_XLMAC_INTR_ENABLE_EN_RX_MSG_OVERFLOW_FIELD_MASK,
    0,
    LPORT_XLMAC_INTR_ENABLE_EN_RX_MSG_OVERFLOW_FIELD_WIDTH,
    LPORT_XLMAC_INTR_ENABLE_EN_RX_MSG_OVERFLOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_INTR_ENABLE_EN_RX_PKT_OVERFLOW
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_INTR_ENABLE_EN_RX_PKT_OVERFLOW_FIELD =
{
    "EN_RX_PKT_OVERFLOW",
#if RU_INCLUDE_DESC
    "",
    "If set, SUM_RX_PKT_OVERFLOW can set mac interrupt.",
#endif
    LPORT_XLMAC_INTR_ENABLE_EN_RX_PKT_OVERFLOW_FIELD_MASK,
    0,
    LPORT_XLMAC_INTR_ENABLE_EN_RX_PKT_OVERFLOW_FIELD_WIDTH,
    LPORT_XLMAC_INTR_ENABLE_EN_RX_PKT_OVERFLOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_INTR_ENABLE_EN_TX_TS_FIFO_OVERFLOW
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_INTR_ENABLE_EN_TX_TS_FIFO_OVERFLOW_FIELD =
{
    "EN_TX_TS_FIFO_OVERFLOW",
#if RU_INCLUDE_DESC
    "",
    "If set, SUM_TX_TS_FIFO_OVERFLOW can set mac interrupt.",
#endif
    LPORT_XLMAC_INTR_ENABLE_EN_TX_TS_FIFO_OVERFLOW_FIELD_MASK,
    0,
    LPORT_XLMAC_INTR_ENABLE_EN_TX_TS_FIFO_OVERFLOW_FIELD_WIDTH,
    LPORT_XLMAC_INTR_ENABLE_EN_TX_TS_FIFO_OVERFLOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_INTR_ENABLE_EN_TX_LLFC_MSG_OVERFLOW
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_INTR_ENABLE_EN_TX_LLFC_MSG_OVERFLOW_FIELD =
{
    "EN_TX_LLFC_MSG_OVERFLOW",
#if RU_INCLUDE_DESC
    "",
    "If set, SUM_TX_LLFC_MSG_OVERFLOW can set mac interrupt.",
#endif
    LPORT_XLMAC_INTR_ENABLE_EN_TX_LLFC_MSG_OVERFLOW_FIELD_MASK,
    0,
    LPORT_XLMAC_INTR_ENABLE_EN_TX_LLFC_MSG_OVERFLOW_FIELD_WIDTH,
    LPORT_XLMAC_INTR_ENABLE_EN_TX_LLFC_MSG_OVERFLOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_INTR_ENABLE_EN_TX_PKT_OVERFLOW
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_INTR_ENABLE_EN_TX_PKT_OVERFLOW_FIELD =
{
    "EN_TX_PKT_OVERFLOW",
#if RU_INCLUDE_DESC
    "",
    "If set, SUM_TX_PKT_OVERFLOW can set mac interrupt.",
#endif
    LPORT_XLMAC_INTR_ENABLE_EN_TX_PKT_OVERFLOW_FIELD_MASK,
    0,
    LPORT_XLMAC_INTR_ENABLE_EN_TX_PKT_OVERFLOW_FIELD_WIDTH,
    LPORT_XLMAC_INTR_ENABLE_EN_TX_PKT_OVERFLOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_INTR_ENABLE_EN_TX_PKT_UNDERFLOW
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_INTR_ENABLE_EN_TX_PKT_UNDERFLOW_FIELD =
{
    "EN_TX_PKT_UNDERFLOW",
#if RU_INCLUDE_DESC
    "",
    "If set, SUM_TX_PKT_UNDERFLOW can set mac interrupt.",
#endif
    LPORT_XLMAC_INTR_ENABLE_EN_TX_PKT_UNDERFLOW_FIELD_MASK,
    0,
    LPORT_XLMAC_INTR_ENABLE_EN_TX_PKT_UNDERFLOW_FIELD_WIDTH,
    LPORT_XLMAC_INTR_ENABLE_EN_TX_PKT_UNDERFLOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_VERSION_ID_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_VERSION_ID_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_XLMAC_VERSION_ID_RESERVED0_FIELD_MASK,
    0,
    LPORT_XLMAC_VERSION_ID_RESERVED0_FIELD_WIDTH,
    LPORT_XLMAC_VERSION_ID_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_XLMAC_VERSION_ID_XLMAC_VERSION
 ******************************************************************************/
const ru_field_rec LPORT_XLMAC_VERSION_ID_XLMAC_VERSION_FIELD =
{
    "XLMAC_VERSION",
#if RU_INCLUDE_DESC
    "",
    "XLMAC IP Version ID - corresponds to RTL/DV label",
#endif
    LPORT_XLMAC_VERSION_ID_XLMAC_VERSION_FIELD_MASK,
    0,
    LPORT_XLMAC_VERSION_ID_XLMAC_VERSION_FIELD_WIDTH,
    LPORT_XLMAC_VERSION_ID_XLMAC_VERSION_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: LPORT_XLMAC_CTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_CTRL_FIELDS[] =
{
    &LPORT_XLMAC_CTRL_RESERVED0_FIELD,
    &LPORT_XLMAC_CTRL_EXTENDED_HIG2_EN_FIELD,
    &LPORT_XLMAC_CTRL_LINK_STATUS_SELECT_FIELD,
    &LPORT_XLMAC_CTRL_SW_LINK_STATUS_FIELD,
    &LPORT_XLMAC_CTRL_XGMII_IPG_CHECK_DISABLE_FIELD,
    &LPORT_XLMAC_CTRL_RS_SOFT_RESET_FIELD,
    &LPORT_XLMAC_CTRL_RSVD_5_FIELD,
    &LPORT_XLMAC_CTRL_LOCAL_LPBK_LEAK_ENB_FIELD,
    &LPORT_XLMAC_CTRL_RSVD_4_FIELD,
    &LPORT_XLMAC_CTRL_SOFT_RESET_FIELD,
    &LPORT_XLMAC_CTRL_LAG_FAILOVER_EN_FIELD,
    &LPORT_XLMAC_CTRL_REMOVE_FAILOVER_LPBK_FIELD,
    &LPORT_XLMAC_CTRL_RSVD_1_FIELD,
    &LPORT_XLMAC_CTRL_LOCAL_LPBK_FIELD,
    &LPORT_XLMAC_CTRL_RX_EN_FIELD,
    &LPORT_XLMAC_CTRL_TX_EN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_CTRL_REG = 
{
    "CTRL",
#if RU_INCLUDE_DESC
    "MAC control for XLMAC0/port0 (LPORT port0)",
    "MAC control.",
#endif
    LPORT_XLMAC_CTRL_REG_OFFSET,
    0,
    0,
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    16,
    LPORT_XLMAC_CTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_MODE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_MODE_FIELDS[] =
{
    &LPORT_XLMAC_MODE_RESERVED0_FIELD,
    &LPORT_XLMAC_MODE_SPEED_MODE_FIELD,
    &LPORT_XLMAC_MODE_NO_SOP_FOR_CRC_HG_FIELD,
    &LPORT_XLMAC_MODE_HDR_MODE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_MODE_REG = 
{
    "MODE",
#if RU_INCLUDE_DESC
    "XLMAC Mode register for XLMAC0/port0 (LPORT port0)",
    "XLMAC Mode register",
#endif
    LPORT_XLMAC_MODE_REG_OFFSET,
    0,
    0,
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    LPORT_XLMAC_MODE_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_SPARE0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_SPARE0_FIELDS[] =
{
    &LPORT_XLMAC_SPARE0_RESERVED0_FIELD,
    &LPORT_XLMAC_SPARE0_RSVD_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_SPARE0_REG = 
{
    "SPARE0",
#if RU_INCLUDE_DESC
    "Spare reg 0 for ECO for XLMAC0/port0 (LPORT port0)",
    "Spare reg 0 for ECO",
#endif
    LPORT_XLMAC_SPARE0_REG_OFFSET,
    0,
    0,
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LPORT_XLMAC_SPARE0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_SPARE1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_SPARE1_FIELDS[] =
{
    &LPORT_XLMAC_SPARE1_RESERVED0_FIELD,
    &LPORT_XLMAC_SPARE1_RSVD_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_SPARE1_REG = 
{
    "SPARE1",
#if RU_INCLUDE_DESC
    "Spare reg 1 for ECO for XLMAC0/port0 (LPORT port0)",
    "Spare reg 1 for ECO",
#endif
    LPORT_XLMAC_SPARE1_REG_OFFSET,
    0,
    0,
    3,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LPORT_XLMAC_SPARE1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_TX_CTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_TX_CTRL_FIELDS[] =
{
    &LPORT_XLMAC_TX_CTRL_RESERVED0_FIELD,
    &LPORT_XLMAC_TX_CTRL_TX_THRESHOLD_FIELD,
    &LPORT_XLMAC_TX_CTRL_EP_DISCARD_FIELD,
    &LPORT_XLMAC_TX_CTRL_TX_PREAMBLE_LENGTH_FIELD,
    &LPORT_XLMAC_TX_CTRL_THROT_DENOM_FIELD,
    &LPORT_XLMAC_TX_CTRL_THROT_NUM_FIELD,
    &LPORT_XLMAC_TX_CTRL_AVERAGE_IPG_FIELD,
    &LPORT_XLMAC_TX_CTRL_PAD_THRESHOLD_FIELD,
    &LPORT_XLMAC_TX_CTRL_PAD_EN_FIELD,
    &LPORT_XLMAC_TX_CTRL_TX_ANY_START_FIELD,
    &LPORT_XLMAC_TX_CTRL_DISCARD_FIELD,
    &LPORT_XLMAC_TX_CTRL_CRC_MODE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_TX_CTRL_REG = 
{
    "TX_CTRL",
#if RU_INCLUDE_DESC
    "Transmit control for XLMAC0/port0 (LPORT port0)",
    "Transmit control.\n"
    "\n"
    "This XLMAC core register is 42 bits wide in hardware. LPORT register reads and writes are however 32 bits per transaction. When reading from this address, higher XLMAC register bits are copied to XLMAC0 32-bit Direct Access Data Read Register, and can subsequently be obtained by reading from that register. Similarly, when writing to this address, higher XLMAC register bits are taken from XLMAC0 32-bit Direct Access Data Write Register, so it is important to ensure proper value for the higher bits is present in that register prior to writing to this (...XLMAC_TX_CTRL) register.\n"
    "Alternatively, Indirect Access mechanism can be used to access XLMAC core registers (see XLMACx Indirect Access registers).\n"
    "\n"
    "NOTE: THIS REGISTER HAS AN ALTERNATIVE/OVERLAY FIELD LAYOUT VIEW",
#endif
    LPORT_XLMAC_TX_CTRL_REG_OFFSET,
    0,
    0,
    4,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    12,
    LPORT_XLMAC_TX_CTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_TX_CTRL_OVERLAY
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_TX_CTRL_OVERLAY_FIELDS[] =
{
    &LPORT_XLMAC_TX_CTRL_OVERLAY_RESERVED0_FIELD,
    &LPORT_XLMAC_TX_CTRL_OVERLAY_XLMAC_TX_CTRL_HI_FIELD,
    &LPORT_XLMAC_TX_CTRL_OVERLAY_XLMAC_TX_CTRL_LO_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_TX_CTRL_OVERLAY_REG = 
{
    "TX_CTRL_OVERLAY",
#if RU_INCLUDE_DESC
    "Transmit control for XLMAC0/port0 (LPORT port0) - OVERLAY VIEW.",
    "Transmit control.\n"
    "\n"
    "This is not a separate register - this is the same register (same address/same hardware) as ...XLMAC_TX_CTRL, here just presented with an alternative/overlay field layout view.\n"
    "\n"
    "This XLMAC core register, in this \"overlay\" view, has 42 bits. LPORT register reads and writes are however 32 bits per transaction. When reading from this address, higher XLMAC register bits are copied to XLMAC0 32-bit Direct Access Data Read Register, and can subsequently be obtained by reading from that register. Similarly, when writing to this address, higher XLMAC register bits are taken from XLMAC0 32-bit Direct Access Data Write Register, so it is important to ensure proper value for the higher bits is present in that register prior to writing to this (...XLMAC_TX_CTRL) register.\n"
    "Alternatively, Indirect Access mechanism can be used to access XLMAC core registers (see XLMACx Indirect Access registers).",
#endif
    LPORT_XLMAC_TX_CTRL_OVERLAY_REG_OFFSET,
    0,
    0,
    5,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    LPORT_XLMAC_TX_CTRL_OVERLAY_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_TX_MAC_SA
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_TX_MAC_SA_FIELDS[] =
{
    &LPORT_XLMAC_TX_MAC_SA_RESERVED0_FIELD,
    &LPORT_XLMAC_TX_MAC_SA_CTRL_SA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_TX_MAC_SA_REG = 
{
    "TX_MAC_SA",
#if RU_INCLUDE_DESC
    "Transmit Source Address for XLMAC0/port0 (LPORT port0)",
    "Transmit Source Address.\n"
    "\n"
    "This XLMAC core register is 48 bits wide in hardware. LPORT register reads and writes are however 32 bits per transaction. When reading from this address, higher XLMAC register bits are copied to XLMAC0 32-bit Direct Access Data Read Register, and can subsequently be obtained by reading from that register. Similarly, when writing to this address, higher XLMAC register bits are taken from XLMAC0 32-bit Direct Access Data Write Register, so it is important to ensure proper value for the higher bits is present in that register prior to writing to this (...XLMAC_TX_MAC_SA) register.\n"
    "Alternatively, Indirect Access mechanism can be used to access XLMAC core registers (see XLMACx Indirect Access registers).\n"
    "\n"
    "NOTE: THIS REGISTER HAS AN ALTERNATIVE/OVERLAY FIELD LAYOUT VIEW",
#endif
    LPORT_XLMAC_TX_MAC_SA_REG_OFFSET,
    0,
    0,
    6,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LPORT_XLMAC_TX_MAC_SA_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_TX_MAC_SA_OVERLAY
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_TX_MAC_SA_OVERLAY_FIELDS[] =
{
    &LPORT_XLMAC_TX_MAC_SA_OVERLAY_RESERVED0_FIELD,
    &LPORT_XLMAC_TX_MAC_SA_OVERLAY_SA_HI_FIELD,
    &LPORT_XLMAC_TX_MAC_SA_OVERLAY_SA_LO_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_TX_MAC_SA_OVERLAY_REG = 
{
    "TX_MAC_SA_OVERLAY",
#if RU_INCLUDE_DESC
    "Transmit Source Address for XLMAC0/port0 (LPORT port0) - OVERLAY VIEW.",
    "Transmit Source Address.\n"
    "\n"
    "This is not a separate register - this is the same register (same address/same hardware) as ...XLMAC_TX_MAC_SA, here just presented with an alternative/overlay field layout view.\n"
    "\n"
    "This XLMAC core register, in this \"overlay\" view, has 48 bits. LPORT register reads and writes are however 32 bits per transaction. When reading from this address, higher XLMAC register bits are copied to XLMAC0 32-bit Direct Access Data Read Register, and can subsequently be obtained by reading from that register. Similarly, when writing to this address, higher XLMAC register bits are taken from XLMAC0 32-bit Direct Access Data Write Register, so it is important to ensure proper value for the higher bits is present in that register prior to writing to this (...XLMAC_TX_MAC_SA) register.\n"
    "Alternatively, Indirect Access mechanism can be used to access XLMAC core registers (see XLMACx Indirect Access registers).",
#endif
    LPORT_XLMAC_TX_MAC_SA_OVERLAY_REG_OFFSET,
    0,
    0,
    7,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    LPORT_XLMAC_TX_MAC_SA_OVERLAY_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_RX_CTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_RX_CTRL_FIELDS[] =
{
    &LPORT_XLMAC_RX_CTRL_RESERVED0_FIELD,
    &LPORT_XLMAC_RX_CTRL_RX_PASS_PFC_FIELD,
    &LPORT_XLMAC_RX_CTRL_RX_PASS_PAUSE_FIELD,
    &LPORT_XLMAC_RX_CTRL_RX_PASS_CTRL_FIELD,
    &LPORT_XLMAC_RX_CTRL_RSVD_3_FIELD,
    &LPORT_XLMAC_RX_CTRL_RSVD_2_FIELD,
    &LPORT_XLMAC_RX_CTRL_RUNT_THRESHOLD_FIELD,
    &LPORT_XLMAC_RX_CTRL_STRICT_PREAMBLE_FIELD,
    &LPORT_XLMAC_RX_CTRL_STRIP_CRC_FIELD,
    &LPORT_XLMAC_RX_CTRL_RX_ANY_START_FIELD,
    &LPORT_XLMAC_RX_CTRL_RSVD_1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_RX_CTRL_REG = 
{
    "RX_CTRL",
#if RU_INCLUDE_DESC
    "Receive control for XLMAC0/port0 (LPORT port0)",
    "Receive control.",
#endif
    LPORT_XLMAC_RX_CTRL_REG_OFFSET,
    0,
    0,
    8,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    11,
    LPORT_XLMAC_RX_CTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_RX_MAC_SA
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_RX_MAC_SA_FIELDS[] =
{
    &LPORT_XLMAC_RX_MAC_SA_RESERVED0_FIELD,
    &LPORT_XLMAC_RX_MAC_SA_RX_SA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_RX_MAC_SA_REG = 
{
    "RX_MAC_SA",
#if RU_INCLUDE_DESC
    "Receive source address for XLMAC0/port0 (LPORT port0)",
    "Receive source address.\n"
    "\n"
    "This XLMAC core register is 48 bits wide in hardware. LPORT register reads and writes are however 32 bits per transaction. When reading from this address, higher XLMAC register bits are copied to XLMAC0 32-bit Direct Access Data Read Register, and can subsequently be obtained by reading from that register. Similarly, when writing to this address, higher XLMAC register bits are taken from XLMAC0 32-bit Direct Access Data Write Register, so it is important to ensure proper value for the higher bits is present in that register prior to writing to this (...XLMAC_RX_MAC_SA) register.\n"
    "Alternatively, Indirect Access mechanism can be used to access XLMAC core registers (see XLMACx Indirect Access registers).\n"
    "\n"
    "NOTE: THIS REGISTER HAS AN ALTERNATIVE/OVERLAY FIELD LAYOUT VIEW",
#endif
    LPORT_XLMAC_RX_MAC_SA_REG_OFFSET,
    0,
    0,
    9,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LPORT_XLMAC_RX_MAC_SA_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_RX_MAC_SA_OVERLAY
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_RX_MAC_SA_OVERLAY_FIELDS[] =
{
    &LPORT_XLMAC_RX_MAC_SA_OVERLAY_RESERVED0_FIELD,
    &LPORT_XLMAC_RX_MAC_SA_OVERLAY_SA_HI_FIELD,
    &LPORT_XLMAC_RX_MAC_SA_OVERLAY_SA_LO_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_RX_MAC_SA_OVERLAY_REG = 
{
    "RX_MAC_SA_OVERLAY",
#if RU_INCLUDE_DESC
    "Receive source address for XLMAC0/port0 (LPORT port0) - OVERLAY VIEW.",
    "Receive source address.\n"
    "\n"
    "This is not a separate register - this is the same register (same address/same hardware) as ...XLMAC_RX_MAC_SA, here just presented with an alternative/overlay field layout view.\n"
    "\n"
    "This XLMAC core register, in this \"overlay\" view, has 48 bits. LPORT register reads and writes are however 32 bits per transaction. When reading from this address, higher XLMAC register bits are copied to XLMAC0 32-bit Direct Access Data Read Register, and can subsequently be obtained by reading from that register. Similarly, when writing to this address, higher XLMAC register bits are taken from XLMAC0 32-bit Direct Access Data Write Register, so it is important to ensure proper value for the higher bits is present in that register prior to writing to this (...XLMAC_RX_MAC_SA) register.\n"
    "Alternatively, Indirect Access mechanism can be used to access XLMAC core registers (see XLMACx Indirect Access registers).",
#endif
    LPORT_XLMAC_RX_MAC_SA_OVERLAY_REG_OFFSET,
    0,
    0,
    10,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    LPORT_XLMAC_RX_MAC_SA_OVERLAY_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_RX_MAX_SIZE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_RX_MAX_SIZE_FIELDS[] =
{
    &LPORT_XLMAC_RX_MAX_SIZE_RESERVED0_FIELD,
    &LPORT_XLMAC_RX_MAX_SIZE_RX_MAX_SIZE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_RX_MAX_SIZE_REG = 
{
    "RX_MAX_SIZE",
#if RU_INCLUDE_DESC
    "Receive maximum packet size for XLMAC0/port0 (LPORT port0)",
    "Receive maximum packet size.",
#endif
    LPORT_XLMAC_RX_MAX_SIZE_REG_OFFSET,
    0,
    0,
    11,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LPORT_XLMAC_RX_MAX_SIZE_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_RX_VLAN_TAG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_RX_VLAN_TAG_FIELDS[] =
{
    &LPORT_XLMAC_RX_VLAN_TAG_RESERVED0_FIELD,
    &LPORT_XLMAC_RX_VLAN_TAG_OUTER_VLAN_TAG_ENABLE_FIELD,
    &LPORT_XLMAC_RX_VLAN_TAG_INNER_VLAN_TAG_ENABLE_FIELD,
    &LPORT_XLMAC_RX_VLAN_TAG_OUTER_VLAN_TAG_FIELD,
    &LPORT_XLMAC_RX_VLAN_TAG_INNER_VLAN_TAG_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_RX_VLAN_TAG_REG = 
{
    "RX_VLAN_TAG",
#if RU_INCLUDE_DESC
    "Inner and Outer VLAN tag fields for XLMAC0/port0 (LPORT port0)",
    "Inner and Outer VLAN tag fields\n"
    "\n"
    "This XLMAC core register is 34 bits wide in hardware. LPORT register reads and writes are however 32 bits per transaction. When reading from this address, higher XLMAC register bits are copied to XLMAC0 32-bit Direct Access Data Read Register, and can subsequently be obtained by reading from that register. Similarly, when writing to this address, higher XLMAC register bits are taken from XLMAC0 32-bit Direct Access Data Write Register, so it is important to ensure proper value for the higher bits is present in that register prior to writing to this (...XLMAC_RX_VLAN_TAG) register.\n"
    "Alternatively, Indirect Access mechanism can be used to access XLMAC core registers (see XLMACx Indirect Access registers).",
#endif
    LPORT_XLMAC_RX_VLAN_TAG_REG_OFFSET,
    0,
    0,
    12,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    LPORT_XLMAC_RX_VLAN_TAG_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_RX_LSS_CTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_RX_LSS_CTRL_FIELDS[] =
{
    &LPORT_XLMAC_RX_LSS_CTRL_RESERVED0_FIELD,
    &LPORT_XLMAC_RX_LSS_CTRL_RESET_FLOW_CONTROL_TIMERS_ON_LINK_DOWN_FIELD,
    &LPORT_XLMAC_RX_LSS_CTRL_DROP_TX_DATA_ON_LINK_INTERRUPT_FIELD,
    &LPORT_XLMAC_RX_LSS_CTRL_DROP_TX_DATA_ON_REMOTE_FAULT_FIELD,
    &LPORT_XLMAC_RX_LSS_CTRL_DROP_TX_DATA_ON_LOCAL_FAULT_FIELD,
    &LPORT_XLMAC_RX_LSS_CTRL_LINK_INTERRUPTION_DISABLE_FIELD,
    &LPORT_XLMAC_RX_LSS_CTRL_USE_EXTERNAL_FAULTS_FOR_TX_FIELD,
    &LPORT_XLMAC_RX_LSS_CTRL_REMOTE_FAULT_DISABLE_FIELD,
    &LPORT_XLMAC_RX_LSS_CTRL_LOCAL_FAULT_DISABLE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_RX_LSS_CTRL_REG = 
{
    "RX_LSS_CTRL",
#if RU_INCLUDE_DESC
    "Control for LSS (ordered set) messages for XLMAC0/port0 (LPORT port0)",
    "Control for LSS (ordered set) messages",
#endif
    LPORT_XLMAC_RX_LSS_CTRL_REG_OFFSET,
    0,
    0,
    13,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    LPORT_XLMAC_RX_LSS_CTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_RX_LSS_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_RX_LSS_STATUS_FIELDS[] =
{
    &LPORT_XLMAC_RX_LSS_STATUS_RESERVED0_FIELD,
    &LPORT_XLMAC_RX_LSS_STATUS_LINK_INTERRUPTION_STATUS_FIELD,
    &LPORT_XLMAC_RX_LSS_STATUS_REMOTE_FAULT_STATUS_FIELD,
    &LPORT_XLMAC_RX_LSS_STATUS_LOCAL_FAULT_STATUS_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_RX_LSS_STATUS_REG = 
{
    "RX_LSS_STATUS",
#if RU_INCLUDE_DESC
    "Status for RS layerThese bits are sticky by nature, and can be cleared by writing to the clear register for XLMAC0/port0 (LPORT port0)",
    "Status for RS layer. These bits are sticky by nature, and can be cleared by writing to the clear register",
#endif
    LPORT_XLMAC_RX_LSS_STATUS_REG_OFFSET,
    0,
    0,
    14,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    LPORT_XLMAC_RX_LSS_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_CLEAR_RX_LSS_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_CLEAR_RX_LSS_STATUS_FIELDS[] =
{
    &LPORT_XLMAC_CLEAR_RX_LSS_STATUS_RESERVED0_FIELD,
    &LPORT_XLMAC_CLEAR_RX_LSS_STATUS_CLEAR_LINK_INTERRUPTION_STATUS_FIELD,
    &LPORT_XLMAC_CLEAR_RX_LSS_STATUS_CLEAR_REMOTE_FAULT_STATUS_FIELD,
    &LPORT_XLMAC_CLEAR_RX_LSS_STATUS_CLEAR_LOCAL_FAULT_STATUS_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_CLEAR_RX_LSS_STATUS_REG = 
{
    "CLEAR_RX_LSS_STATUS",
#if RU_INCLUDE_DESC
    "Clear the XLMAC_RX_LSS_STATUS register, used for resetting the sticky status bits for XLMAC0/port0 (LPORT port0)",
    "Clear the XLMAC_RX_LSS_STATUS register, used for resetting the sticky status bits",
#endif
    LPORT_XLMAC_CLEAR_RX_LSS_STATUS_REG_OFFSET,
    0,
    0,
    15,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    LPORT_XLMAC_CLEAR_RX_LSS_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_PAUSE_CTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_PAUSE_CTRL_FIELDS[] =
{
    &LPORT_XLMAC_PAUSE_CTRL_RESERVED0_FIELD,
    &LPORT_XLMAC_PAUSE_CTRL_PAUSE_XOFF_TIMER_FIELD,
    &LPORT_XLMAC_PAUSE_CTRL_RSVD_2_FIELD,
    &LPORT_XLMAC_PAUSE_CTRL_RSVD_1_FIELD,
    &LPORT_XLMAC_PAUSE_CTRL_RX_PAUSE_EN_FIELD,
    &LPORT_XLMAC_PAUSE_CTRL_TX_PAUSE_EN_FIELD,
    &LPORT_XLMAC_PAUSE_CTRL_PAUSE_REFRESH_EN_FIELD,
    &LPORT_XLMAC_PAUSE_CTRL_PAUSE_REFRESH_TIMER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_PAUSE_CTRL_REG = 
{
    "PAUSE_CTRL",
#if RU_INCLUDE_DESC
    "PAUSE control register for XLMAC0/port0 (LPORT port0)",
    "PAUSE control register\n"
    "\n"
    "This XLMAC core register is 37 bits wide in hardware. LPORT register reads and writes are however 32 bits per transaction. When reading from this address, higher XLMAC register bits are copied to XLMAC0 32-bit Direct Access Data Read Register, and can subsequently be obtained by reading from that register. Similarly, when writing to this address, higher XLMAC register bits are taken from XLMAC0 32-bit Direct Access Data Write Register, so it is important to ensure proper value for the higher bits is present in that register prior to writing to this (...XLMAC_PAUSE_CTRL) register.\n"
    "Alternatively, Indirect Access mechanism can be used to access XLMAC core registers (see XLMACx Indirect Access registers).\n"
    "\n"
    "NOTE: THIS REGISTER HAS AN ALTERNATIVE/OVERLAY FIELD LAYOUT VIEW",
#endif
    LPORT_XLMAC_PAUSE_CTRL_REG_OFFSET,
    0,
    0,
    16,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    LPORT_XLMAC_PAUSE_CTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_PAUSE_CTRL_OVERLAY
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_PAUSE_CTRL_OVERLAY_FIELDS[] =
{
    &LPORT_XLMAC_PAUSE_CTRL_OVERLAY_RESERVED0_FIELD,
    &LPORT_XLMAC_PAUSE_CTRL_OVERLAY_XLMAC_PAUSE_CTRL_HI_FIELD,
    &LPORT_XLMAC_PAUSE_CTRL_OVERLAY_XLMAC_PAUSE_CTRL_LO_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_PAUSE_CTRL_OVERLAY_REG = 
{
    "PAUSE_CTRL_OVERLAY",
#if RU_INCLUDE_DESC
    "PAUSE control register for XLMAC0/port0 (LPORT port0) - OVERLAY VIEW.",
    "PAUSE control register\n"
    "\n"
    "This is not a separate register - this is the same register (same address/same hardware) as ...XLMAC_PAUSE_CTRL, here just presented with an alternative/overlay field layout view.\n"
    "\n"
    "This XLMAC core register, in this \"overlay\" view, has 37 bits. LPORT register reads and writes are however 32 bits per transaction. When reading from this address, higher XLMAC register bits are copied to XLMAC0 32-bit Direct Access Data Read Register, and can subsequently be obtained by reading from that register. Similarly, when writing to this address, higher XLMAC register bits are taken from XLMAC0 32-bit Direct Access Data Write Register, so it is important to ensure proper value for the higher bits is present in that register prior to writing to this (...XLMAC_PAUSE_CTRL) register.\n"
    "Alternatively, Indirect Access mechanism can be used to access XLMAC core registers (see XLMACx Indirect Access registers).",
#endif
    LPORT_XLMAC_PAUSE_CTRL_OVERLAY_REG_OFFSET,
    0,
    0,
    17,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    LPORT_XLMAC_PAUSE_CTRL_OVERLAY_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_PFC_CTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_PFC_CTRL_FIELDS[] =
{
    &LPORT_XLMAC_PFC_CTRL_RESERVED0_FIELD,
    &LPORT_XLMAC_PFC_CTRL_TX_PFC_EN_FIELD,
    &LPORT_XLMAC_PFC_CTRL_RX_PFC_EN_FIELD,
    &LPORT_XLMAC_PFC_CTRL_PFC_STATS_EN_FIELD,
    &LPORT_XLMAC_PFC_CTRL_RSVD_FIELD,
    &LPORT_XLMAC_PFC_CTRL_FORCE_PFC_XON_FIELD,
    &LPORT_XLMAC_PFC_CTRL_PFC_REFRESH_EN_FIELD,
    &LPORT_XLMAC_PFC_CTRL_PFC_XOFF_TIMER_FIELD,
    &LPORT_XLMAC_PFC_CTRL_PFC_REFRESH_TIMER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_PFC_CTRL_REG = 
{
    "PFC_CTRL",
#if RU_INCLUDE_DESC
    "PFC control register for XLMAC0/port0 (LPORT port0)",
    "PFC control register\n"
    "\n"
    "This XLMAC core register is 38 bits wide in hardware. LPORT register reads and writes are however 32 bits per transaction. When reading from this address, higher XLMAC register bits are copied to XLMAC0 32-bit Direct Access Data Read Register, and can subsequently be obtained by reading from that register. Similarly, when writing to this address, higher XLMAC register bits are taken from XLMAC0 32-bit Direct Access Data Write Register, so it is important to ensure proper value for the higher bits is present in that register prior to writing to this (...XLMAC_PFC_CTRL) register.\n"
    "Alternatively, Indirect Access mechanism can be used to access XLMAC core registers (see XLMACx Indirect Access registers).\n"
    "\n"
    "NOTE: THIS REGISTER HAS AN ALTERNATIVE/OVERLAY FIELD LAYOUT VIEW",
#endif
    LPORT_XLMAC_PFC_CTRL_REG_OFFSET,
    0,
    0,
    18,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    LPORT_XLMAC_PFC_CTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_PFC_CTRL_OVERLAY
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_PFC_CTRL_OVERLAY_FIELDS[] =
{
    &LPORT_XLMAC_PFC_CTRL_OVERLAY_RESERVED0_FIELD,
    &LPORT_XLMAC_PFC_CTRL_OVERLAY_LLFC_REFRESH_EN_FIELD,
    &LPORT_XLMAC_PFC_CTRL_OVERLAY_RESERVED1_FIELD,
    &LPORT_XLMAC_PFC_CTRL_OVERLAY_LLFC_REFRESH_TIMER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_PFC_CTRL_OVERLAY_REG = 
{
    "PFC_CTRL_OVERLAY",
#if RU_INCLUDE_DESC
    "PFC control register for XLMAC0/port0 (LPORT port0) - OVERLAY VIEW.",
    "PFC control register\n"
    "\n"
    "This is not a separate register - this is the same register (same address/same hardware) as ...XLMAC_PFC_CTRL, here just presented with an alternative/overlay field layout view.\n"
    "\n"
    "This XLMAC core register, in this \"overlay\" view, has 33 bits. LPORT register reads and writes are however 32 bits per transaction. When reading from this address, higher XLMAC register bits are copied to XLMAC0 32-bit Direct Access Data Read Register, and can subsequently be obtained by reading from that register. Similarly, when writing to this address, higher XLMAC register bits are taken from XLMAC0 32-bit Direct Access Data Write Register, so it is important to ensure proper value for the higher bits is present in that register prior to writing to this (...XLMAC_PFC_CTRL) register.\n"
    "Alternatively, Indirect Access mechanism can be used to access XLMAC core registers (see XLMACx Indirect Access registers).",
#endif
    LPORT_XLMAC_PFC_CTRL_OVERLAY_REG_OFFSET,
    0,
    0,
    19,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    LPORT_XLMAC_PFC_CTRL_OVERLAY_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_PFC_TYPE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_PFC_TYPE_FIELDS[] =
{
    &LPORT_XLMAC_PFC_TYPE_RESERVED0_FIELD,
    &LPORT_XLMAC_PFC_TYPE_PFC_ETH_TYPE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_PFC_TYPE_REG = 
{
    "PFC_TYPE",
#if RU_INCLUDE_DESC
    "PFC Ethertype for XLMAC0/port0 (LPORT port0)",
    "PFC Ethertype",
#endif
    LPORT_XLMAC_PFC_TYPE_REG_OFFSET,
    0,
    0,
    20,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LPORT_XLMAC_PFC_TYPE_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_PFC_OPCODE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_PFC_OPCODE_FIELDS[] =
{
    &LPORT_XLMAC_PFC_OPCODE_RESERVED0_FIELD,
    &LPORT_XLMAC_PFC_OPCODE_PFC_OPCODE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_PFC_OPCODE_REG = 
{
    "PFC_OPCODE",
#if RU_INCLUDE_DESC
    "PFC Opcode for XLMAC0/port0 (LPORT port0)",
    "PFC Opcode",
#endif
    LPORT_XLMAC_PFC_OPCODE_REG_OFFSET,
    0,
    0,
    21,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LPORT_XLMAC_PFC_OPCODE_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_PFC_DA
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_PFC_DA_FIELDS[] =
{
    &LPORT_XLMAC_PFC_DA_RESERVED0_FIELD,
    &LPORT_XLMAC_PFC_DA_PFC_MACDA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_PFC_DA_REG = 
{
    "PFC_DA",
#if RU_INCLUDE_DESC
    "PFC Destination Address for XLMAC0/port0 (LPORT port0)",
    "PFC Destination Address.\n"
    "\n"
    "This XLMAC core register is 48 bits wide in hardware. LPORT register reads and writes are however 32 bits per transaction. When reading from this address, higher XLMAC register bits are copied to XLMAC0 32-bit Direct Access Data Read Register, and can subsequently be obtained by reading from that register. Similarly, when writing to this address, higher XLMAC register bits are taken from XLMAC0 32-bit Direct Access Data Write Register, so it is important to ensure proper value for the higher bits is present in that register prior to writing to this (...XLMAC_PFC_DA) register.\n"
    "Alternatively, Indirect Access mechanism can be used to access XLMAC core registers (see XLMACx Indirect Access registers).\n"
    "\n"
    "NOTE: THIS REGISTER HAS AN ALTERNATIVE/OVERLAY FIELD LAYOUT VIEW",
#endif
    LPORT_XLMAC_PFC_DA_REG_OFFSET,
    0,
    0,
    22,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LPORT_XLMAC_PFC_DA_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_PFC_DA_OVERLAY
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_PFC_DA_OVERLAY_FIELDS[] =
{
    &LPORT_XLMAC_PFC_DA_OVERLAY_RESERVED0_FIELD,
    &LPORT_XLMAC_PFC_DA_OVERLAY_PFC_MACDA_HI_FIELD,
    &LPORT_XLMAC_PFC_DA_OVERLAY_PFC_MACDA_LO_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_PFC_DA_OVERLAY_REG = 
{
    "PFC_DA_OVERLAY",
#if RU_INCLUDE_DESC
    "PFC Destination Address for XLMAC0/port0 (LPORT port0) - OVERLAY VIEW.",
    "PFC Destination Address.\n"
    "\n"
    "This is not a separate register - this is the same register (same address/same hardware) as ...XLMAC_PFC_DA, here just presented with an alternative/overlay field layout view.\n"
    "\n"
    "This XLMAC core register, in this \"overlay\" view, has 48 bits. LPORT register reads and writes are however 32 bits per transaction. When reading from this address, higher XLMAC register bits are copied to XLMAC0 32-bit Direct Access Data Read Register, and can subsequently be obtained by reading from that register. Similarly, when writing to this address, higher XLMAC register bits are taken from XLMAC0 32-bit Direct Access Data Write Register, so it is important to ensure proper value for the higher bits is present in that register prior to writing to this (...XLMAC_PFC_DA) register.\n"
    "Alternatively, Indirect Access mechanism can be used to access XLMAC core registers (see XLMACx Indirect Access registers).",
#endif
    LPORT_XLMAC_PFC_DA_OVERLAY_REG_OFFSET,
    0,
    0,
    23,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    LPORT_XLMAC_PFC_DA_OVERLAY_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_LLFC_CTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_LLFC_CTRL_FIELDS[] =
{
    &LPORT_XLMAC_LLFC_CTRL_RESERVED0_FIELD,
    &LPORT_XLMAC_LLFC_CTRL_LLFC_IMG_FIELD,
    &LPORT_XLMAC_LLFC_CTRL_NO_SOM_FOR_CRC_LLFC_FIELD,
    &LPORT_XLMAC_LLFC_CTRL_LLFC_CRC_IGNORE_FIELD,
    &LPORT_XLMAC_LLFC_CTRL_LLFC_CUT_THROUGH_MODE_FIELD,
    &LPORT_XLMAC_LLFC_CTRL_LLFC_IN_IPG_ONLY_FIELD,
    &LPORT_XLMAC_LLFC_CTRL_RX_LLFC_EN_FIELD,
    &LPORT_XLMAC_LLFC_CTRL_TX_LLFC_EN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_LLFC_CTRL_REG = 
{
    "LLFC_CTRL",
#if RU_INCLUDE_DESC
    "LLFC Control Register for XLMAC0/port0 (LPORT port0)",
    "LLFC Control Register",
#endif
    LPORT_XLMAC_LLFC_CTRL_REG_OFFSET,
    0,
    0,
    24,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    LPORT_XLMAC_LLFC_CTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_TX_LLFC_MSG_FIELDS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_TX_LLFC_MSG_FIELDS_FIELDS[] =
{
    &LPORT_XLMAC_TX_LLFC_MSG_FIELDS_RESERVED0_FIELD,
    &LPORT_XLMAC_TX_LLFC_MSG_FIELDS_LLFC_XOFF_TIME_FIELD,
    &LPORT_XLMAC_TX_LLFC_MSG_FIELDS_TX_LLFC_FC_OBJ_LOGICAL_FIELD,
    &LPORT_XLMAC_TX_LLFC_MSG_FIELDS_TX_LLFC_MSG_TYPE_LOGICAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_TX_LLFC_MSG_FIELDS_REG = 
{
    "TX_LLFC_MSG_FIELDS",
#if RU_INCLUDE_DESC
    "Programmable TX LLFC Message fields for XLMAC0/port0 (LPORT port0)",
    "Programmable TX LLFC Message fields.",
#endif
    LPORT_XLMAC_TX_LLFC_MSG_FIELDS_REG_OFFSET,
    0,
    0,
    25,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    LPORT_XLMAC_TX_LLFC_MSG_FIELDS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_RX_LLFC_MSG_FIELDS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_RX_LLFC_MSG_FIELDS_FIELDS[] =
{
    &LPORT_XLMAC_RX_LLFC_MSG_FIELDS_RESERVED0_FIELD,
    &LPORT_XLMAC_RX_LLFC_MSG_FIELDS_RX_LLFC_FC_OBJ_PHYSICAL_FIELD,
    &LPORT_XLMAC_RX_LLFC_MSG_FIELDS_RX_LLFC_MSG_TYPE_PHYSICAL_FIELD,
    &LPORT_XLMAC_RX_LLFC_MSG_FIELDS_RX_LLFC_FC_OBJ_LOGICAL_FIELD,
    &LPORT_XLMAC_RX_LLFC_MSG_FIELDS_RX_LLFC_MSG_TYPE_LOGICAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_RX_LLFC_MSG_FIELDS_REG = 
{
    "RX_LLFC_MSG_FIELDS",
#if RU_INCLUDE_DESC
    "Programmable RX LLFC Message fields for XLMAC0/port0 (LPORT port0)",
    "Programmable RX LLFC Message fields",
#endif
    LPORT_XLMAC_RX_LLFC_MSG_FIELDS_REG_OFFSET,
    0,
    0,
    26,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    LPORT_XLMAC_RX_LLFC_MSG_FIELDS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_TX_TIMESTAMP_FIFO_DATA
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_TX_TIMESTAMP_FIFO_DATA_FIELDS[] =
{
    &LPORT_XLMAC_TX_TIMESTAMP_FIFO_DATA_RESERVED0_FIELD,
    &LPORT_XLMAC_TX_TIMESTAMP_FIFO_DATA_TS_ENTRY_VALID_FIELD,
    &LPORT_XLMAC_TX_TIMESTAMP_FIFO_DATA_SEQUENCE_ID_FIELD,
    &LPORT_XLMAC_TX_TIMESTAMP_FIFO_DATA_TIME_STAMP_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_TX_TIMESTAMP_FIFO_DATA_REG = 
{
    "TX_TIMESTAMP_FIFO_DATA",
#if RU_INCLUDE_DESC
    "The TimeStamp value of the Tx two-step packets for XLMAC0/port0 (LPORT port0)",
    "The TimeStamp value of the Tx two-step packets.\n"
    "\n"
    "This XLMAC core register is 49 bits wide in hardware. LPORT register reads and writes are however 32 bits per transaction. When reading from this address, higher XLMAC register bits are copied to XLMAC0 32-bit Direct Access Data Read Register, and can subsequently be obtained by reading from that register. Similarly, when writing to this address, higher XLMAC register bits are taken from XLMAC0 32-bit Direct Access Data Write Register, so it is important to ensure proper value for the higher bits is present in that register prior to writing to this (...XLMAC_TX_TIMESTAMP_FIFO_DATA) register.\n"
    "Alternatively, Indirect Access mechanism can be used to access XLMAC core registers (see XLMACx Indirect Access registers).",
#endif
    LPORT_XLMAC_TX_TIMESTAMP_FIFO_DATA_REG_OFFSET,
    0,
    0,
    27,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    LPORT_XLMAC_TX_TIMESTAMP_FIFO_DATA_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_TX_TIMESTAMP_FIFO_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_TX_TIMESTAMP_FIFO_STATUS_FIELDS[] =
{
    &LPORT_XLMAC_TX_TIMESTAMP_FIFO_STATUS_RESERVED0_FIELD,
    &LPORT_XLMAC_TX_TIMESTAMP_FIFO_STATUS_ENTRY_COUNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_TX_TIMESTAMP_FIFO_STATUS_REG = 
{
    "TX_TIMESTAMP_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "Tx TimeStamp FIFO Status for XLMAC0/port0 (LPORT port0)",
    "Tx TimeStamp FIFO Status.",
#endif
    LPORT_XLMAC_TX_TIMESTAMP_FIFO_STATUS_REG_OFFSET,
    0,
    0,
    28,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LPORT_XLMAC_TX_TIMESTAMP_FIFO_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_FIFO_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_FIFO_STATUS_FIELDS[] =
{
    &LPORT_XLMAC_FIFO_STATUS_RESERVED0_FIELD,
    &LPORT_XLMAC_FIFO_STATUS_LINK_STATUS_FIELD,
    &LPORT_XLMAC_FIFO_STATUS_RX_PKT_OVERFLOW_FIELD,
    &LPORT_XLMAC_FIFO_STATUS_TX_TS_FIFO_OVERFLOW_FIELD,
    &LPORT_XLMAC_FIFO_STATUS_TX_LLFC_MSG_OVERFLOW_FIELD,
    &LPORT_XLMAC_FIFO_STATUS_RSVD_2_FIELD,
    &LPORT_XLMAC_FIFO_STATUS_TX_PKT_OVERFLOW_FIELD,
    &LPORT_XLMAC_FIFO_STATUS_TX_PKT_UNDERFLOW_FIELD,
    &LPORT_XLMAC_FIFO_STATUS_RX_MSG_OVERFLOW_FIELD,
    &LPORT_XLMAC_FIFO_STATUS_RSVD_1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_FIFO_STATUS_REG = 
{
    "FIFO_STATUS",
#if RU_INCLUDE_DESC
    "FIFO status registerThese bits (except LINK_STATUS) are sticky by nature, and can be cleared by writing to the clear register. for XLMAC0/port0 (LPORT port0)",
    "FIFO status register. These bits (except LINK_STATUS) are sticky by nature, and can be cleared by writing to the clear register.",
#endif
    LPORT_XLMAC_FIFO_STATUS_REG_OFFSET,
    0,
    0,
    29,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    10,
    LPORT_XLMAC_FIFO_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_CLEAR_FIFO_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_CLEAR_FIFO_STATUS_FIELDS[] =
{
    &LPORT_XLMAC_CLEAR_FIFO_STATUS_RESERVED0_FIELD,
    &LPORT_XLMAC_CLEAR_FIFO_STATUS_CLEAR_RX_PKT_OVERFLOW_FIELD,
    &LPORT_XLMAC_CLEAR_FIFO_STATUS_CLEAR_TX_TS_FIFO_OVERFLOW_FIELD,
    &LPORT_XLMAC_CLEAR_FIFO_STATUS_CLEAR_TX_LLFC_MSG_OVERFLOW_FIELD,
    &LPORT_XLMAC_CLEAR_FIFO_STATUS_RSVD_2_FIELD,
    &LPORT_XLMAC_CLEAR_FIFO_STATUS_CLEAR_TX_PKT_OVERFLOW_FIELD,
    &LPORT_XLMAC_CLEAR_FIFO_STATUS_CLEAR_TX_PKT_UNDERFLOW_FIELD,
    &LPORT_XLMAC_CLEAR_FIFO_STATUS_CLEAR_RX_MSG_OVERFLOW_FIELD,
    &LPORT_XLMAC_CLEAR_FIFO_STATUS_RSVD_1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_CLEAR_FIFO_STATUS_REG = 
{
    "CLEAR_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "Clear XLMAC_FIFO_STATUS register, used for resetting the sticky status bits for XLMAC0/port0 (LPORT port0)",
    "Clear XLMAC_FIFO_STATUS register, used for resetting the sticky status bits",
#endif
    LPORT_XLMAC_CLEAR_FIFO_STATUS_REG_OFFSET,
    0,
    0,
    30,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    LPORT_XLMAC_CLEAR_FIFO_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_LAG_FAILOVER_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_LAG_FAILOVER_STATUS_FIELDS[] =
{
    &LPORT_XLMAC_LAG_FAILOVER_STATUS_RESERVED0_FIELD,
    &LPORT_XLMAC_LAG_FAILOVER_STATUS_RSVD_FIELD,
    &LPORT_XLMAC_LAG_FAILOVER_STATUS_LAG_FAILOVER_LOOPBACK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_LAG_FAILOVER_STATUS_REG = 
{
    "LAG_FAILOVER_STATUS",
#if RU_INCLUDE_DESC
    "Lag Failover Status for XLMAC0/port0 (LPORT port0)",
    "Lag Failover Status.",
#endif
    LPORT_XLMAC_LAG_FAILOVER_STATUS_REG_OFFSET,
    0,
    0,
    31,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    LPORT_XLMAC_LAG_FAILOVER_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_EEE_CTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_EEE_CTRL_FIELDS[] =
{
    &LPORT_XLMAC_EEE_CTRL_RESERVED0_FIELD,
    &LPORT_XLMAC_EEE_CTRL_RSVD_FIELD,
    &LPORT_XLMAC_EEE_CTRL_EEE_EN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_EEE_CTRL_REG = 
{
    "EEE_CTRL",
#if RU_INCLUDE_DESC
    "Register for EEE Control  for XLMAC0/port0 (LPORT port0)",
    "Register for EEE Control",
#endif
    LPORT_XLMAC_EEE_CTRL_REG_OFFSET,
    0,
    0,
    32,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    LPORT_XLMAC_EEE_CTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_EEE_TIMERS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_EEE_TIMERS_FIELDS[] =
{
    &LPORT_XLMAC_EEE_TIMERS_EEE_REF_COUNT_FIELD,
    &LPORT_XLMAC_EEE_TIMERS_EEE_WAKE_TIMER_FIELD,
    &LPORT_XLMAC_EEE_TIMERS_EEE_DELAY_ENTRY_TIMER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_EEE_TIMERS_REG = 
{
    "EEE_TIMERS",
#if RU_INCLUDE_DESC
    "EEE Timers for XLMAC0/port0 (LPORT port0)",
    "EEE Timers\n"
    "\n"
    "This XLMAC core register is 64 bits wide in hardware. LPORT register reads and writes are however 32 bits per transaction. When reading from this address, higher XLMAC register bits are copied to XLMAC0 32-bit Direct Access Data Read Register, and can subsequently be obtained by reading from that register. Similarly, when writing to this address, higher XLMAC register bits are taken from XLMAC0 32-bit Direct Access Data Write Register, so it is important to ensure proper value for the higher bits is present in that register prior to writing to this (...XLMAC_EEE_TIMERS) register.\n"
    "Alternatively, Indirect Access mechanism can be used to access XLMAC core registers (see XLMACx Indirect Access registers).\n"
    "\n"
    "NOTE: THIS REGISTER HAS AN ALTERNATIVE/OVERLAY FIELD LAYOUT VIEW",
#endif
    LPORT_XLMAC_EEE_TIMERS_REG_OFFSET,
    0,
    0,
    33,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    LPORT_XLMAC_EEE_TIMERS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_EEE_TIMERS_OVERLAY
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_EEE_TIMERS_OVERLAY_FIELDS[] =
{
    &LPORT_XLMAC_EEE_TIMERS_OVERLAY_XLMAC_EEE_TIMERS_HI_FIELD,
    &LPORT_XLMAC_EEE_TIMERS_OVERLAY_XLMAC_EEE_TIMERS_LO_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_EEE_TIMERS_OVERLAY_REG = 
{
    "EEE_TIMERS_OVERLAY",
#if RU_INCLUDE_DESC
    "EEE Timers for XLMAC0/port0 (LPORT port0) - OVERLAY VIEW.",
    "EEE Timers\n"
    "\n"
    "This is not a separate register - this is the same register (same address/same hardware) as ...XLMAC_EEE_TIMERS, here just presented with an alternative/overlay field layout view.\n"
    "\n"
    "This XLMAC core register, in this \"overlay\" view, has 64 bits. LPORT register reads and writes are however 32 bits per transaction. When reading from this address, higher XLMAC register bits are copied to XLMAC0 32-bit Direct Access Data Read Register, and can subsequently be obtained by reading from that register. Similarly, when writing to this address, higher XLMAC register bits are taken from XLMAC0 32-bit Direct Access Data Write Register, so it is important to ensure proper value for the higher bits is present in that register prior to writing to this (...XLMAC_EEE_TIMERS) register.\n"
    "Alternatively, Indirect Access mechanism can be used to access XLMAC core registers (see XLMACx Indirect Access registers).",
#endif
    LPORT_XLMAC_EEE_TIMERS_OVERLAY_REG_OFFSET,
    0,
    0,
    34,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LPORT_XLMAC_EEE_TIMERS_OVERLAY_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_EEE_1_SEC_LINK_STATUS_TIMER
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_EEE_1_SEC_LINK_STATUS_TIMER_FIELDS[] =
{
    &LPORT_XLMAC_EEE_1_SEC_LINK_STATUS_TIMER_RESERVED0_FIELD,
    &LPORT_XLMAC_EEE_1_SEC_LINK_STATUS_TIMER_ONE_SECOND_TIMER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_EEE_1_SEC_LINK_STATUS_TIMER_REG = 
{
    "EEE_1_SEC_LINK_STATUS_TIMER",
#if RU_INCLUDE_DESC
    "EEE One Second Link Status Timer for XLMAC0/port0 (LPORT port0)",
    "EEE One Second Link Status Timer",
#endif
    LPORT_XLMAC_EEE_1_SEC_LINK_STATUS_TIMER_REG_OFFSET,
    0,
    0,
    35,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LPORT_XLMAC_EEE_1_SEC_LINK_STATUS_TIMER_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_HIGIG_HDR_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_HIGIG_HDR_0_FIELDS[] =
{
    &LPORT_XLMAC_HIGIG_HDR_0_HIGIG_HDR_0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_HIGIG_HDR_0_REG = 
{
    "HIGIG_HDR_0",
#if RU_INCLUDE_DESC
    "HiGig2 and HiHig+ header register - MS bytes for XLMAC0/port0 (LPORT port0)",
    "HiGig2 and HiHig+ header register - MS bytes\n"
    "\n"
    "This XLMAC core register is 64 bits wide in hardware. LPORT register reads and writes are however 32 bits per transaction. When reading from this address, higher XLMAC register bits are copied to XLMAC0 32-bit Direct Access Data Read Register, and can subsequently be obtained by reading from that register. Similarly, when writing to this address, higher XLMAC register bits are taken from XLMAC0 32-bit Direct Access Data Write Register, so it is important to ensure proper value for the higher bits is present in that register prior to writing to this (...XLMAC_HIGIG_HDR_0) register.\n"
    "Alternatively, Indirect Access mechanism can be used to access XLMAC core registers (see XLMACx Indirect Access registers).\n"
    "\n"
    "NOTE: THIS REGISTER HAS AN ALTERNATIVE/OVERLAY FIELD LAYOUT VIEW",
#endif
    LPORT_XLMAC_HIGIG_HDR_0_REG_OFFSET,
    0,
    0,
    36,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LPORT_XLMAC_HIGIG_HDR_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_HIGIG_HDR_0_OVERLAY
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_HIGIG_HDR_0_OVERLAY_FIELDS[] =
{
    &LPORT_XLMAC_HIGIG_HDR_0_OVERLAY_HIGIG_HDR_0_HI_FIELD,
    &LPORT_XLMAC_HIGIG_HDR_0_OVERLAY_HIGIG_HDR_0_LO_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_HIGIG_HDR_0_OVERLAY_REG = 
{
    "HIGIG_HDR_0_OVERLAY",
#if RU_INCLUDE_DESC
    "HiGig2 and HiHig+ header register - MS bytes for XLMAC0/port0 (LPORT port0) - OVERLAY VIEW.",
    "HiGig2 and HiHig+ header register - MS bytes\n"
    "\n"
    "This is not a separate register - this is the same register (same address/same hardware) as ...XLMAC_HIGIG_HDR_0, here just presented with an alternative/overlay field layout view.\n"
    "\n"
    "This XLMAC core register, in this \"overlay\" view, has 64 bits. LPORT register reads and writes are however 32 bits per transaction. When reading from this address, higher XLMAC register bits are copied to XLMAC0 32-bit Direct Access Data Read Register, and can subsequently be obtained by reading from that register. Similarly, when writing to this address, higher XLMAC register bits are taken from XLMAC0 32-bit Direct Access Data Write Register, so it is important to ensure proper value for the higher bits is present in that register prior to writing to this (...XLMAC_HIGIG_HDR_0) register.\n"
    "Alternatively, Indirect Access mechanism can be used to access XLMAC core registers (see XLMACx Indirect Access registers).",
#endif
    LPORT_XLMAC_HIGIG_HDR_0_OVERLAY_REG_OFFSET,
    0,
    0,
    37,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LPORT_XLMAC_HIGIG_HDR_0_OVERLAY_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_HIGIG_HDR_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_HIGIG_HDR_1_FIELDS[] =
{
    &LPORT_XLMAC_HIGIG_HDR_1_HIGIG_HDR_1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_HIGIG_HDR_1_REG = 
{
    "HIGIG_HDR_1",
#if RU_INCLUDE_DESC
    "HiGig2 and HiHig+ header register - LS bytes for XLMAC0/port0 (LPORT port0)",
    "HiGig2 and HiHig+ header register - LS bytes\n"
    "\n"
    "This XLMAC core register is 64 bits wide in hardware. LPORT register reads and writes are however 32 bits per transaction. When reading from this address, higher XLMAC register bits are copied to XLMAC0 32-bit Direct Access Data Read Register, and can subsequently be obtained by reading from that register. Similarly, when writing to this address, higher XLMAC register bits are taken from XLMAC0 32-bit Direct Access Data Write Register, so it is important to ensure proper value for the higher bits is present in that register prior to writing to this (...XLMAC_HIGIG_HDR_1) register.\n"
    "Alternatively, Indirect Access mechanism can be used to access XLMAC core registers (see XLMACx Indirect Access registers).\n"
    "\n"
    "NOTE: THIS REGISTER HAS AN ALTERNATIVE/OVERLAY FIELD LAYOUT VIEW",
#endif
    LPORT_XLMAC_HIGIG_HDR_1_REG_OFFSET,
    0,
    0,
    38,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LPORT_XLMAC_HIGIG_HDR_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_HIGIG_HDR_1_OVERLAY
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_HIGIG_HDR_1_OVERLAY_FIELDS[] =
{
    &LPORT_XLMAC_HIGIG_HDR_1_OVERLAY_HIGIG_HDR_1_HI_FIELD,
    &LPORT_XLMAC_HIGIG_HDR_1_OVERLAY_HIGIG_HDR_1_LO_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_HIGIG_HDR_1_OVERLAY_REG = 
{
    "HIGIG_HDR_1_OVERLAY",
#if RU_INCLUDE_DESC
    "HiGig2 and HiHig+ header register - LS bytes for XLMAC0/port0 (LPORT port0) - OVERLAY VIEW.",
    "HiGig2 and HiHig+ header register - LS bytes\n"
    "\n"
    "This is not a separate register - this is the same register (same address/same hardware) as ...XLMAC_HIGIG_HDR_1, here just presented with an alternative/overlay field layout view.\n"
    "\n"
    "This XLMAC core register, in this \"overlay\" view, has 64 bits. LPORT register reads and writes are however 32 bits per transaction. When reading from this address, higher XLMAC register bits are copied to XLMAC0 32-bit Direct Access Data Read Register, and can subsequently be obtained by reading from that register. Similarly, when writing to this address, higher XLMAC register bits are taken from XLMAC0 32-bit Direct Access Data Write Register, so it is important to ensure proper value for the higher bits is present in that register prior to writing to this (...XLMAC_HIGIG_HDR_1) register.\n"
    "Alternatively, Indirect Access mechanism can be used to access XLMAC core registers (see XLMACx Indirect Access registers).",
#endif
    LPORT_XLMAC_HIGIG_HDR_1_OVERLAY_REG_OFFSET,
    0,
    0,
    39,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LPORT_XLMAC_HIGIG_HDR_1_OVERLAY_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_GMII_EEE_CTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_GMII_EEE_CTRL_FIELDS[] =
{
    &LPORT_XLMAC_GMII_EEE_CTRL_RESERVED0_FIELD,
    &LPORT_XLMAC_GMII_EEE_CTRL_GMII_LPI_PREDICT_MODE_EN_FIELD,
    &LPORT_XLMAC_GMII_EEE_CTRL_GMII_LPI_PREDICT_THRESHOLD_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_GMII_EEE_CTRL_REG = 
{
    "GMII_EEE_CTRL",
#if RU_INCLUDE_DESC
    "MAC EEE control in GMII mode for XLMAC0/port0 (LPORT port0)",
    "MAC EEE control in GMII mode.",
#endif
    LPORT_XLMAC_GMII_EEE_CTRL_REG_OFFSET,
    0,
    0,
    40,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    LPORT_XLMAC_GMII_EEE_CTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_TIMESTAMP_ADJUST
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_TIMESTAMP_ADJUST_FIELDS[] =
{
    &LPORT_XLMAC_TIMESTAMP_ADJUST_RESERVED0_FIELD,
    &LPORT_XLMAC_TIMESTAMP_ADJUST_TS_USE_CS_OFFSET_FIELD,
    &LPORT_XLMAC_TIMESTAMP_ADJUST_TS_TSTS_ADJUST_FIELD,
    &LPORT_XLMAC_TIMESTAMP_ADJUST_TS_OSTS_ADJUST_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_TIMESTAMP_ADJUST_REG = 
{
    "TIMESTAMP_ADJUST",
#if RU_INCLUDE_DESC
    "Timestamp Adjust registerRefer specification document for more details for XLMAC0/port0 (LPORT port0)",
    "Timestamp Adjust register. Refer specification document for more details",
#endif
    LPORT_XLMAC_TIMESTAMP_ADJUST_REG_OFFSET,
    0,
    0,
    41,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    LPORT_XLMAC_TIMESTAMP_ADJUST_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_TIMESTAMP_BYTE_ADJUST
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_TIMESTAMP_BYTE_ADJUST_FIELDS[] =
{
    &LPORT_XLMAC_TIMESTAMP_BYTE_ADJUST_RESERVED0_FIELD,
    &LPORT_XLMAC_TIMESTAMP_BYTE_ADJUST_RX_TIMER_BYTE_ADJUST_EN_FIELD,
    &LPORT_XLMAC_TIMESTAMP_BYTE_ADJUST_RX_TIMER_BYTE_ADJUST_FIELD,
    &LPORT_XLMAC_TIMESTAMP_BYTE_ADJUST_TX_TIMER_BYTE_ADJUST_EN_FIELD,
    &LPORT_XLMAC_TIMESTAMP_BYTE_ADJUST_TX_TIMER_BYTE_ADJUST_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_TIMESTAMP_BYTE_ADJUST_REG = 
{
    "TIMESTAMP_BYTE_ADJUST",
#if RU_INCLUDE_DESC
    "Timestamp Byte Adjust registerRefer specification document for more details for XLMAC0/port0 (LPORT port0)",
    "Timestamp Byte Adjust register. Refer specification document for more details",
#endif
    LPORT_XLMAC_TIMESTAMP_BYTE_ADJUST_REG_OFFSET,
    0,
    0,
    42,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    LPORT_XLMAC_TIMESTAMP_BYTE_ADJUST_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_TX_CRC_CORRUPT_CTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_FIELDS[] =
{
    &LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_RESERVED0_FIELD,
    &LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_PROG_TX_CRC_FIELD,
    &LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_TX_CRC_CORRUPTION_MODE_FIELD,
    &LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_TX_CRC_CORRUPT_EN_FIELD,
    &LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_TX_ERR_CORRUPTS_CRC_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_REG = 
{
    "TX_CRC_CORRUPT_CTRL",
#if RU_INCLUDE_DESC
    "Tx CRC corrupt control register for XLMAC0/port0 (LPORT port0)",
    "Tx CRC corrupt control register\n"
    "\n"
    "This XLMAC core register is 35 bits wide in hardware. LPORT register reads and writes are however 32 bits per transaction. When reading from this address, higher XLMAC register bits are copied to XLMAC0 32-bit Direct Access Data Read Register, and can subsequently be obtained by reading from that register. Similarly, when writing to this address, higher XLMAC register bits are taken from XLMAC0 32-bit Direct Access Data Write Register, so it is important to ensure proper value for the higher bits is present in that register prior to writing to this (...XLMAC_TX_CRC_CORRUPT_CTRL) register.\n"
    "Alternatively, Indirect Access mechanism can be used to access XLMAC core registers (see XLMACx Indirect Access registers).\n"
    "\n"
    "NOTE: THIS REGISTER HAS AN ALTERNATIVE/OVERLAY FIELD LAYOUT VIEW",
#endif
    LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_REG_OFFSET,
    0,
    0,
    43,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_OVERLAY
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_OVERLAY_FIELDS[] =
{
    &LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_OVERLAY_RESERVED0_FIELD,
    &LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_OVERLAY_XLMAC_TX_CRC_CORRUPT_CTRL_HI_FIELD,
    &LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_OVERLAY_XLMAC_TX_CRC_CORRUPT_CTRL_LO_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_OVERLAY_REG = 
{
    "TX_CRC_CORRUPT_CTRL_OVERLAY",
#if RU_INCLUDE_DESC
    "Tx CRC corrupt control register for XLMAC0/port0 (LPORT port0) - OVERLAY VIEW.",
    "Tx CRC corrupt control register\n"
    "\n"
    "This is not a separate register - this is the same register (same address/same hardware) as ...XLMAC_TX_CRC_CORRUPT_CTRL, here just presented with an alternative/overlay field layout view.\n"
    "\n"
    "This XLMAC core register, in this \"overlay\" view, has 35 bits. LPORT register reads and writes are however 32 bits per transaction. When reading from this address, higher XLMAC register bits are copied to XLMAC0 32-bit Direct Access Data Read Register, and can subsequently be obtained by reading from that register. Similarly, when writing to this address, higher XLMAC register bits are taken from XLMAC0 32-bit Direct Access Data Write Register, so it is important to ensure proper value for the higher bits is present in that register prior to writing to this (...XLMAC_TX_CRC_CORRUPT_CTRL) register.\n"
    "Alternatively, Indirect Access mechanism can be used to access XLMAC core registers (see XLMACx Indirect Access registers).",
#endif
    LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_OVERLAY_REG_OFFSET,
    0,
    0,
    44,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_OVERLAY_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_E2E_CTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_E2E_CTRL_FIELDS[] =
{
    &LPORT_XLMAC_E2E_CTRL_RESERVED0_FIELD,
    &LPORT_XLMAC_E2E_CTRL_E2EFC_DUAL_MODID_EN_FIELD,
    &LPORT_XLMAC_E2E_CTRL_E2ECC_LEGACY_IMP_EN_FIELD,
    &LPORT_XLMAC_E2E_CTRL_E2ECC_DUAL_MODID_EN_FIELD,
    &LPORT_XLMAC_E2E_CTRL_HONOR_PAUSE_FOR_E2E_FIELD,
    &LPORT_XLMAC_E2E_CTRL_E2E_ENABLE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_E2E_CTRL_REG = 
{
    "E2E_CTRL",
#if RU_INCLUDE_DESC
    "Transmit E2EFC/E2ECC control register for XLMAC0/port0 (LPORT port0)",
    "Transmit E2EFC/E2ECC control register",
#endif
    LPORT_XLMAC_E2E_CTRL_REG_OFFSET,
    0,
    0,
    45,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    LPORT_XLMAC_E2E_CTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_E2ECC_MODULE_HDR_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_E2ECC_MODULE_HDR_0_FIELDS[] =
{
    &LPORT_XLMAC_E2ECC_MODULE_HDR_0_E2ECC_MODULE_HDR_0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_E2ECC_MODULE_HDR_0_REG = 
{
    "E2ECC_MODULE_HDR_0",
#if RU_INCLUDE_DESC
    "E2ECC module header register - MS bytes for XLMAC0/port0 (LPORT port0)",
    "E2ECC module header register - MS bytes\n"
    "\n"
    "This XLMAC core register is 64 bits wide in hardware. LPORT register reads and writes are however 32 bits per transaction. When reading from this address, higher XLMAC register bits are copied to XLMAC0 32-bit Direct Access Data Read Register, and can subsequently be obtained by reading from that register. Similarly, when writing to this address, higher XLMAC register bits are taken from XLMAC0 32-bit Direct Access Data Write Register, so it is important to ensure proper value for the higher bits is present in that register prior to writing to this (...XLMAC_E2ECC_MODULE_HDR_0) register.\n"
    "Alternatively, Indirect Access mechanism can be used to access XLMAC core registers (see XLMACx Indirect Access registers).\n"
    "\n"
    "NOTE: THIS REGISTER HAS AN ALTERNATIVE/OVERLAY FIELD LAYOUT VIEW",
#endif
    LPORT_XLMAC_E2ECC_MODULE_HDR_0_REG_OFFSET,
    0,
    0,
    46,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LPORT_XLMAC_E2ECC_MODULE_HDR_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_E2ECC_MODULE_HDR_0_OVERLAY
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_E2ECC_MODULE_HDR_0_OVERLAY_FIELDS[] =
{
    &LPORT_XLMAC_E2ECC_MODULE_HDR_0_OVERLAY_E2ECC_MODULE_HDR_0_HI_FIELD,
    &LPORT_XLMAC_E2ECC_MODULE_HDR_0_OVERLAY_E2ECC_MODULE_HDR_0_LO_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_E2ECC_MODULE_HDR_0_OVERLAY_REG = 
{
    "E2ECC_MODULE_HDR_0_OVERLAY",
#if RU_INCLUDE_DESC
    "E2ECC module header register - MS bytes for XLMAC0/port0 (LPORT port0) - OVERLAY VIEW.",
    "E2ECC module header register - MS bytes\n"
    "\n"
    "This is not a separate register - this is the same register (same address/same hardware) as ...XLMAC_E2ECC_MODULE_HDR_0, here just presented with an alternative/overlay field layout view.\n"
    "\n"
    "This XLMAC core register, in this \"overlay\" view, has 64 bits. LPORT register reads and writes are however 32 bits per transaction. When reading from this address, higher XLMAC register bits are copied to XLMAC0 32-bit Direct Access Data Read Register, and can subsequently be obtained by reading from that register. Similarly, when writing to this address, higher XLMAC register bits are taken from XLMAC0 32-bit Direct Access Data Write Register, so it is important to ensure proper value for the higher bits is present in that register prior to writing to this (...XLMAC_E2ECC_MODULE_HDR_0) register.\n"
    "Alternatively, Indirect Access mechanism can be used to access XLMAC core registers (see XLMACx Indirect Access registers).",
#endif
    LPORT_XLMAC_E2ECC_MODULE_HDR_0_OVERLAY_REG_OFFSET,
    0,
    0,
    47,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LPORT_XLMAC_E2ECC_MODULE_HDR_0_OVERLAY_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_E2ECC_MODULE_HDR_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_E2ECC_MODULE_HDR_1_FIELDS[] =
{
    &LPORT_XLMAC_E2ECC_MODULE_HDR_1_E2ECC_MODULE_HDR_1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_E2ECC_MODULE_HDR_1_REG = 
{
    "E2ECC_MODULE_HDR_1",
#if RU_INCLUDE_DESC
    "E2ECC module header register - LS bytes for XLMAC0/port0 (LPORT port0)",
    "E2ECC module header register - LS bytes\n"
    "\n"
    "This XLMAC core register is 64 bits wide in hardware. LPORT register reads and writes are however 32 bits per transaction. When reading from this address, higher XLMAC register bits are copied to XLMAC0 32-bit Direct Access Data Read Register, and can subsequently be obtained by reading from that register. Similarly, when writing to this address, higher XLMAC register bits are taken from XLMAC0 32-bit Direct Access Data Write Register, so it is important to ensure proper value for the higher bits is present in that register prior to writing to this (...XLMAC_E2ECC_MODULE_HDR_1) register.\n"
    "Alternatively, Indirect Access mechanism can be used to access XLMAC core registers (see XLMACx Indirect Access registers).\n"
    "\n"
    "NOTE: THIS REGISTER HAS AN ALTERNATIVE/OVERLAY FIELD LAYOUT VIEW",
#endif
    LPORT_XLMAC_E2ECC_MODULE_HDR_1_REG_OFFSET,
    0,
    0,
    48,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LPORT_XLMAC_E2ECC_MODULE_HDR_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_E2ECC_MODULE_HDR_1_OVERLAY
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_E2ECC_MODULE_HDR_1_OVERLAY_FIELDS[] =
{
    &LPORT_XLMAC_E2ECC_MODULE_HDR_1_OVERLAY_E2ECC_MODULE_HDR_1_HI_FIELD,
    &LPORT_XLMAC_E2ECC_MODULE_HDR_1_OVERLAY_E2ECC_MODULE_HDR_1_LO_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_E2ECC_MODULE_HDR_1_OVERLAY_REG = 
{
    "E2ECC_MODULE_HDR_1_OVERLAY",
#if RU_INCLUDE_DESC
    "E2ECC module header register - LS bytes for XLMAC0/port0 (LPORT port0) - OVERLAY VIEW.",
    "E2ECC module header register - LS bytes\n"
    "\n"
    "This is not a separate register - this is the same register (same address/same hardware) as ...XLMAC_E2ECC_MODULE_HDR_1, here just presented with an alternative/overlay field layout view.\n"
    "\n"
    "This XLMAC core register, in this \"overlay\" view, has 64 bits. LPORT register reads and writes are however 32 bits per transaction. When reading from this address, higher XLMAC register bits are copied to XLMAC0 32-bit Direct Access Data Read Register, and can subsequently be obtained by reading from that register. Similarly, when writing to this address, higher XLMAC register bits are taken from XLMAC0 32-bit Direct Access Data Write Register, so it is important to ensure proper value for the higher bits is present in that register prior to writing to this (...XLMAC_E2ECC_MODULE_HDR_1) register.\n"
    "Alternatively, Indirect Access mechanism can be used to access XLMAC core registers (see XLMACx Indirect Access registers).",
#endif
    LPORT_XLMAC_E2ECC_MODULE_HDR_1_OVERLAY_REG_OFFSET,
    0,
    0,
    49,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LPORT_XLMAC_E2ECC_MODULE_HDR_1_OVERLAY_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_E2ECC_DATA_HDR_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_E2ECC_DATA_HDR_0_FIELDS[] =
{
    &LPORT_XLMAC_E2ECC_DATA_HDR_0_E2ECC_DATA_HDR_0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_E2ECC_DATA_HDR_0_REG = 
{
    "E2ECC_DATA_HDR_0",
#if RU_INCLUDE_DESC
    "E2ECC Ethernet header register - MS bytes for XLMAC0/port0 (LPORT port0)",
    "E2ECC Ethernet header register - MS bytes\n"
    "\n"
    "This XLMAC core register is 64 bits wide in hardware. LPORT register reads and writes are however 32 bits per transaction. When reading from this address, higher XLMAC register bits are copied to XLMAC0 32-bit Direct Access Data Read Register, and can subsequently be obtained by reading from that register. Similarly, when writing to this address, higher XLMAC register bits are taken from XLMAC0 32-bit Direct Access Data Write Register, so it is important to ensure proper value for the higher bits is present in that register prior to writing to this (...XLMAC_E2ECC_DATA_HDR_0) register.\n"
    "Alternatively, Indirect Access mechanism can be used to access XLMAC core registers (see XLMACx Indirect Access registers).\n"
    "\n"
    "NOTE: THIS REGISTER HAS AN ALTERNATIVE/OVERLAY FIELD LAYOUT VIEW",
#endif
    LPORT_XLMAC_E2ECC_DATA_HDR_0_REG_OFFSET,
    0,
    0,
    50,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LPORT_XLMAC_E2ECC_DATA_HDR_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_E2ECC_DATA_HDR_0_OVERLAY
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_E2ECC_DATA_HDR_0_OVERLAY_FIELDS[] =
{
    &LPORT_XLMAC_E2ECC_DATA_HDR_0_OVERLAY_E2ECC_DATA_HDR_0_HI_FIELD,
    &LPORT_XLMAC_E2ECC_DATA_HDR_0_OVERLAY_E2ECC_DATA_HDR_0_LO_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_E2ECC_DATA_HDR_0_OVERLAY_REG = 
{
    "E2ECC_DATA_HDR_0_OVERLAY",
#if RU_INCLUDE_DESC
    "E2ECC Ethernet header register - MS bytes for XLMAC0/port0 (LPORT port0) - OVERLAY VIEW.",
    "E2ECC Ethernet header register - MS bytes\n"
    "\n"
    "This is not a separate register - this is the same register (same address/same hardware) as ...XLMAC_E2ECC_DATA_HDR_0, here just presented with an alternative/overlay field layout view.\n"
    "\n"
    "This XLMAC core register, in this \"overlay\" view, has 64 bits. LPORT register reads and writes are however 32 bits per transaction. When reading from this address, higher XLMAC register bits are copied to XLMAC0 32-bit Direct Access Data Read Register, and can subsequently be obtained by reading from that register. Similarly, when writing to this address, higher XLMAC register bits are taken from XLMAC0 32-bit Direct Access Data Write Register, so it is important to ensure proper value for the higher bits is present in that register prior to writing to this (...XLMAC_E2ECC_DATA_HDR_0) register.\n"
    "Alternatively, Indirect Access mechanism can be used to access XLMAC core registers (see XLMACx Indirect Access registers).",
#endif
    LPORT_XLMAC_E2ECC_DATA_HDR_0_OVERLAY_REG_OFFSET,
    0,
    0,
    51,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LPORT_XLMAC_E2ECC_DATA_HDR_0_OVERLAY_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_E2ECC_DATA_HDR_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_E2ECC_DATA_HDR_1_FIELDS[] =
{
    &LPORT_XLMAC_E2ECC_DATA_HDR_1_E2ECC_DATA_HDR_1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_E2ECC_DATA_HDR_1_REG = 
{
    "E2ECC_DATA_HDR_1",
#if RU_INCLUDE_DESC
    "E2ECC Ethernet header register - LS bytes for XLMAC0/port0 (LPORT port0)",
    "E2ECC Ethernet header register - LS bytes\n"
    "\n"
    "This XLMAC core register is 64 bits wide in hardware. LPORT register reads and writes are however 32 bits per transaction. When reading from this address, higher XLMAC register bits are copied to XLMAC0 32-bit Direct Access Data Read Register, and can subsequently be obtained by reading from that register. Similarly, when writing to this address, higher XLMAC register bits are taken from XLMAC0 32-bit Direct Access Data Write Register, so it is important to ensure proper value for the higher bits is present in that register prior to writing to this (...XLMAC_E2ECC_DATA_HDR_1) register.\n"
    "Alternatively, Indirect Access mechanism can be used to access XLMAC core registers (see XLMACx Indirect Access registers).\n"
    "\n"
    "NOTE: THIS REGISTER HAS AN ALTERNATIVE/OVERLAY FIELD LAYOUT VIEW",
#endif
    LPORT_XLMAC_E2ECC_DATA_HDR_1_REG_OFFSET,
    0,
    0,
    52,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LPORT_XLMAC_E2ECC_DATA_HDR_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_E2ECC_DATA_HDR_1_OVERLAY
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_E2ECC_DATA_HDR_1_OVERLAY_FIELDS[] =
{
    &LPORT_XLMAC_E2ECC_DATA_HDR_1_OVERLAY_E2ECC_DATA_HDR_1_HI_FIELD,
    &LPORT_XLMAC_E2ECC_DATA_HDR_1_OVERLAY_E2ECC_DATA_HDR_1_LO_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_E2ECC_DATA_HDR_1_OVERLAY_REG = 
{
    "E2ECC_DATA_HDR_1_OVERLAY",
#if RU_INCLUDE_DESC
    "E2ECC Ethernet header register - LS bytes for XLMAC0/port0 (LPORT port0) - OVERLAY VIEW.",
    "E2ECC Ethernet header register - LS bytes\n"
    "\n"
    "This is not a separate register - this is the same register (same address/same hardware) as ...XLMAC_E2ECC_DATA_HDR_1, here just presented with an alternative/overlay field layout view.\n"
    "\n"
    "This XLMAC core register, in this \"overlay\" view, has 64 bits. LPORT register reads and writes are however 32 bits per transaction. When reading from this address, higher XLMAC register bits are copied to XLMAC0 32-bit Direct Access Data Read Register, and can subsequently be obtained by reading from that register. Similarly, when writing to this address, higher XLMAC register bits are taken from XLMAC0 32-bit Direct Access Data Write Register, so it is important to ensure proper value for the higher bits is present in that register prior to writing to this (...XLMAC_E2ECC_DATA_HDR_1) register.\n"
    "Alternatively, Indirect Access mechanism can be used to access XLMAC core registers (see XLMACx Indirect Access registers).",
#endif
    LPORT_XLMAC_E2ECC_DATA_HDR_1_OVERLAY_REG_OFFSET,
    0,
    0,
    53,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LPORT_XLMAC_E2ECC_DATA_HDR_1_OVERLAY_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_E2EFC_MODULE_HDR_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_E2EFC_MODULE_HDR_0_FIELDS[] =
{
    &LPORT_XLMAC_E2EFC_MODULE_HDR_0_E2EFC_MODULE_HDR_0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_E2EFC_MODULE_HDR_0_REG = 
{
    "E2EFC_MODULE_HDR_0",
#if RU_INCLUDE_DESC
    "E2EFC module header register - MS bytes for XLMAC0/port0 (LPORT port0)",
    "E2EFC module header register - MS bytes\n"
    "\n"
    "This XLMAC core register is 64 bits wide in hardware. LPORT register reads and writes are however 32 bits per transaction. When reading from this address, higher XLMAC register bits are copied to XLMAC0 32-bit Direct Access Data Read Register, and can subsequently be obtained by reading from that register. Similarly, when writing to this address, higher XLMAC register bits are taken from XLMAC0 32-bit Direct Access Data Write Register, so it is important to ensure proper value for the higher bits is present in that register prior to writing to this (...XLMAC_E2EFC_MODULE_HDR_0) register.\n"
    "Alternatively, Indirect Access mechanism can be used to access XLMAC core registers (see XLMACx Indirect Access registers).\n"
    "\n"
    "NOTE: THIS REGISTER HAS AN ALTERNATIVE/OVERLAY FIELD LAYOUT VIEW",
#endif
    LPORT_XLMAC_E2EFC_MODULE_HDR_0_REG_OFFSET,
    0,
    0,
    54,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LPORT_XLMAC_E2EFC_MODULE_HDR_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_E2EFC_MODULE_HDR_0_OVERLAY
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_E2EFC_MODULE_HDR_0_OVERLAY_FIELDS[] =
{
    &LPORT_XLMAC_E2EFC_MODULE_HDR_0_OVERLAY_E2EFC_MODULE_HDR_0_HI_FIELD,
    &LPORT_XLMAC_E2EFC_MODULE_HDR_0_OVERLAY_E2EFC_MODULE_HDR_0_LO_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_E2EFC_MODULE_HDR_0_OVERLAY_REG = 
{
    "E2EFC_MODULE_HDR_0_OVERLAY",
#if RU_INCLUDE_DESC
    "E2EFC module header register - MS bytes for XLMAC0/port0 (LPORT port0) - OVERLAY VIEW.",
    "E2EFC module header register - MS bytes\n"
    "\n"
    "This is not a separate register - this is the same register (same address/same hardware) as ...XLMAC_E2EFC_MODULE_HDR_0, here just presented with an alternative/overlay field layout view.\n"
    "\n"
    "This XLMAC core register, in this \"overlay\" view, has 64 bits. LPORT register reads and writes are however 32 bits per transaction. When reading from this address, higher XLMAC register bits are copied to XLMAC0 32-bit Direct Access Data Read Register, and can subsequently be obtained by reading from that register. Similarly, when writing to this address, higher XLMAC register bits are taken from XLMAC0 32-bit Direct Access Data Write Register, so it is important to ensure proper value for the higher bits is present in that register prior to writing to this (...XLMAC_E2EFC_MODULE_HDR_0) register.\n"
    "Alternatively, Indirect Access mechanism can be used to access XLMAC core registers (see XLMACx Indirect Access registers).",
#endif
    LPORT_XLMAC_E2EFC_MODULE_HDR_0_OVERLAY_REG_OFFSET,
    0,
    0,
    55,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LPORT_XLMAC_E2EFC_MODULE_HDR_0_OVERLAY_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_E2EFC_MODULE_HDR_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_E2EFC_MODULE_HDR_1_FIELDS[] =
{
    &LPORT_XLMAC_E2EFC_MODULE_HDR_1_E2EFC_MODULE_HDR_1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_E2EFC_MODULE_HDR_1_REG = 
{
    "E2EFC_MODULE_HDR_1",
#if RU_INCLUDE_DESC
    "E2EFC module header register - LS bytes for XLMAC0/port0 (LPORT port0)",
    "E2EFC module header register - LS bytes\n"
    "\n"
    "This XLMAC core register is 64 bits wide in hardware. LPORT register reads and writes are however 32 bits per transaction. When reading from this address, higher XLMAC register bits are copied to XLMAC0 32-bit Direct Access Data Read Register, and can subsequently be obtained by reading from that register. Similarly, when writing to this address, higher XLMAC register bits are taken from XLMAC0 32-bit Direct Access Data Write Register, so it is important to ensure proper value for the higher bits is present in that register prior to writing to this (...XLMAC_E2EFC_MODULE_HDR_1) register.\n"
    "Alternatively, Indirect Access mechanism can be used to access XLMAC core registers (see XLMACx Indirect Access registers).\n"
    "\n"
    "NOTE: THIS REGISTER HAS AN ALTERNATIVE/OVERLAY FIELD LAYOUT VIEW",
#endif
    LPORT_XLMAC_E2EFC_MODULE_HDR_1_REG_OFFSET,
    0,
    0,
    56,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LPORT_XLMAC_E2EFC_MODULE_HDR_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_E2EFC_MODULE_HDR_1_OVERLAY
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_E2EFC_MODULE_HDR_1_OVERLAY_FIELDS[] =
{
    &LPORT_XLMAC_E2EFC_MODULE_HDR_1_OVERLAY_E2EFC_MODULE_HDR_1_HI_FIELD,
    &LPORT_XLMAC_E2EFC_MODULE_HDR_1_OVERLAY_E2EFC_MODULE_HDR_1_LO_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_E2EFC_MODULE_HDR_1_OVERLAY_REG = 
{
    "E2EFC_MODULE_HDR_1_OVERLAY",
#if RU_INCLUDE_DESC
    "E2EFC module header register - LS bytes for XLMAC0/port0 (LPORT port0) - OVERLAY VIEW.",
    "E2EFC module header register - LS bytes\n"
    "\n"
    "This is not a separate register - this is the same register (same address/same hardware) as ...XLMAC_E2EFC_MODULE_HDR_1, here just presented with an alternative/overlay field layout view.\n"
    "\n"
    "This XLMAC core register, in this \"overlay\" view, has 64 bits. LPORT register reads and writes are however 32 bits per transaction. When reading from this address, higher XLMAC register bits are copied to XLMAC0 32-bit Direct Access Data Read Register, and can subsequently be obtained by reading from that register. Similarly, when writing to this address, higher XLMAC register bits are taken from XLMAC0 32-bit Direct Access Data Write Register, so it is important to ensure proper value for the higher bits is present in that register prior to writing to this (...XLMAC_E2EFC_MODULE_HDR_1) register.\n"
    "Alternatively, Indirect Access mechanism can be used to access XLMAC core registers (see XLMACx Indirect Access registers).",
#endif
    LPORT_XLMAC_E2EFC_MODULE_HDR_1_OVERLAY_REG_OFFSET,
    0,
    0,
    57,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LPORT_XLMAC_E2EFC_MODULE_HDR_1_OVERLAY_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_E2EFC_DATA_HDR_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_E2EFC_DATA_HDR_0_FIELDS[] =
{
    &LPORT_XLMAC_E2EFC_DATA_HDR_0_E2EFC_DATA_HDR_0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_E2EFC_DATA_HDR_0_REG = 
{
    "E2EFC_DATA_HDR_0",
#if RU_INCLUDE_DESC
    "E2EFC Ethernet header register - MS bytes for XLMAC0/port0 (LPORT port0)",
    "E2EFC Ethernet header register - MS bytes\n"
    "\n"
    "This XLMAC core register is 64 bits wide in hardware. LPORT register reads and writes are however 32 bits per transaction. When reading from this address, higher XLMAC register bits are copied to XLMAC0 32-bit Direct Access Data Read Register, and can subsequently be obtained by reading from that register. Similarly, when writing to this address, higher XLMAC register bits are taken from XLMAC0 32-bit Direct Access Data Write Register, so it is important to ensure proper value for the higher bits is present in that register prior to writing to this (...XLMAC_E2EFC_DATA_HDR_0) register.\n"
    "Alternatively, Indirect Access mechanism can be used to access XLMAC core registers (see XLMACx Indirect Access registers).\n"
    "\n"
    "NOTE: THIS REGISTER HAS AN ALTERNATIVE/OVERLAY FIELD LAYOUT VIEW",
#endif
    LPORT_XLMAC_E2EFC_DATA_HDR_0_REG_OFFSET,
    0,
    0,
    58,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LPORT_XLMAC_E2EFC_DATA_HDR_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_E2EFC_DATA_HDR_0_OVERLAY
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_E2EFC_DATA_HDR_0_OVERLAY_FIELDS[] =
{
    &LPORT_XLMAC_E2EFC_DATA_HDR_0_OVERLAY_E2EFC_DATA_HDR_0_HI_FIELD,
    &LPORT_XLMAC_E2EFC_DATA_HDR_0_OVERLAY_E2EFC_DATA_HDR_0_LO_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_E2EFC_DATA_HDR_0_OVERLAY_REG = 
{
    "E2EFC_DATA_HDR_0_OVERLAY",
#if RU_INCLUDE_DESC
    "E2EFC Ethernet header register - MS bytes for XLMAC0/port0 (LPORT port0) - OVERLAY VIEW.",
    "E2EFC Ethernet header register - MS bytes\n"
    "\n"
    "This is not a separate register - this is the same register (same address/same hardware) as ...XLMAC_E2EFC_DATA_HDR_0, here just presented with an alternative/overlay field layout view.\n"
    "\n"
    "This XLMAC core register, in this \"overlay\" view, has 64 bits. LPORT register reads and writes are however 32 bits per transaction. When reading from this address, higher XLMAC register bits are copied to XLMAC0 32-bit Direct Access Data Read Register, and can subsequently be obtained by reading from that register. Similarly, when writing to this address, higher XLMAC register bits are taken from XLMAC0 32-bit Direct Access Data Write Register, so it is important to ensure proper value for the higher bits is present in that register prior to writing to this (...XLMAC_E2EFC_DATA_HDR_0) register.\n"
    "Alternatively, Indirect Access mechanism can be used to access XLMAC core registers (see XLMACx Indirect Access registers).",
#endif
    LPORT_XLMAC_E2EFC_DATA_HDR_0_OVERLAY_REG_OFFSET,
    0,
    0,
    59,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LPORT_XLMAC_E2EFC_DATA_HDR_0_OVERLAY_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_E2EFC_DATA_HDR_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_E2EFC_DATA_HDR_1_FIELDS[] =
{
    &LPORT_XLMAC_E2EFC_DATA_HDR_1_E2EFC_DATA_HDR_1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_E2EFC_DATA_HDR_1_REG = 
{
    "E2EFC_DATA_HDR_1",
#if RU_INCLUDE_DESC
    "E2EFC Ethernet header register - LS bytes for XLMAC0/port0 (LPORT port0)",
    "E2EFC Ethernet header register - LS bytes\n"
    "\n"
    "This XLMAC core register is 64 bits wide in hardware. LPORT register reads and writes are however 32 bits per transaction. When reading from this address, higher XLMAC register bits are copied to XLMAC0 32-bit Direct Access Data Read Register, and can subsequently be obtained by reading from that register. Similarly, when writing to this address, higher XLMAC register bits are taken from XLMAC0 32-bit Direct Access Data Write Register, so it is important to ensure proper value for the higher bits is present in that register prior to writing to this (...XLMAC_E2EFC_DATA_HDR_1) register.\n"
    "Alternatively, Indirect Access mechanism can be used to access XLMAC core registers (see XLMACx Indirect Access registers).\n"
    "\n"
    "NOTE: THIS REGISTER HAS AN ALTERNATIVE/OVERLAY FIELD LAYOUT VIEW",
#endif
    LPORT_XLMAC_E2EFC_DATA_HDR_1_REG_OFFSET,
    0,
    0,
    60,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    LPORT_XLMAC_E2EFC_DATA_HDR_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_E2EFC_DATA_HDR_1_OVERLAY
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_E2EFC_DATA_HDR_1_OVERLAY_FIELDS[] =
{
    &LPORT_XLMAC_E2EFC_DATA_HDR_1_OVERLAY_E2EFC_DATA_HDR_1_HI_FIELD,
    &LPORT_XLMAC_E2EFC_DATA_HDR_1_OVERLAY_E2EFC_DATA_HDR_1_LO_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_E2EFC_DATA_HDR_1_OVERLAY_REG = 
{
    "E2EFC_DATA_HDR_1_OVERLAY",
#if RU_INCLUDE_DESC
    "E2EFC Ethernet header register - LS bytes for XLMAC0/port0 (LPORT port0) - OVERLAY VIEW.",
    "E2EFC Ethernet header register - LS bytes\n"
    "\n"
    "This is not a separate register - this is the same register (same address/same hardware) as ...XLMAC_E2EFC_DATA_HDR_1, here just presented with an alternative/overlay field layout view.\n"
    "\n"
    "This XLMAC core register, in this \"overlay\" view, has 64 bits. LPORT register reads and writes are however 32 bits per transaction. When reading from this address, higher XLMAC register bits are copied to XLMAC0 32-bit Direct Access Data Read Register, and can subsequently be obtained by reading from that register. Similarly, when writing to this address, higher XLMAC register bits are taken from XLMAC0 32-bit Direct Access Data Write Register, so it is important to ensure proper value for the higher bits is present in that register prior to writing to this (...XLMAC_E2EFC_DATA_HDR_1) register.\n"
    "Alternatively, Indirect Access mechanism can be used to access XLMAC core registers (see XLMACx Indirect Access registers).",
#endif
    LPORT_XLMAC_E2EFC_DATA_HDR_1_OVERLAY_REG_OFFSET,
    0,
    0,
    61,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LPORT_XLMAC_E2EFC_DATA_HDR_1_OVERLAY_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_TXFIFO_CELL_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_TXFIFO_CELL_CNT_FIELDS[] =
{
    &LPORT_XLMAC_TXFIFO_CELL_CNT_RESERVED0_FIELD,
    &LPORT_XLMAC_TXFIFO_CELL_CNT_CELL_CNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_TXFIFO_CELL_CNT_REG = 
{
    "TXFIFO_CELL_CNT",
#if RU_INCLUDE_DESC
    "XLMAC TX FIFO Cell Count register for XLMAC0/port0 (LPORT port0)",
    "XLMAC TX FIFO Cell Count register",
#endif
    LPORT_XLMAC_TXFIFO_CELL_CNT_REG_OFFSET,
    0,
    0,
    62,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LPORT_XLMAC_TXFIFO_CELL_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_TXFIFO_CELL_REQ_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_TXFIFO_CELL_REQ_CNT_FIELDS[] =
{
    &LPORT_XLMAC_TXFIFO_CELL_REQ_CNT_RESERVED0_FIELD,
    &LPORT_XLMAC_TXFIFO_CELL_REQ_CNT_REQ_CNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_TXFIFO_CELL_REQ_CNT_REG = 
{
    "TXFIFO_CELL_REQ_CNT",
#if RU_INCLUDE_DESC
    "XLMAC TX FIFO Cell Request Count Register for XLMAC0/port0 (LPORT port0)",
    "XLMAC TX FIFO Cell Request Count Register",
#endif
    LPORT_XLMAC_TXFIFO_CELL_REQ_CNT_REG_OFFSET,
    0,
    0,
    63,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LPORT_XLMAC_TXFIFO_CELL_REQ_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_MEM_CTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_MEM_CTRL_FIELDS[] =
{
    &LPORT_XLMAC_MEM_CTRL_RESERVED0_FIELD,
    &LPORT_XLMAC_MEM_CTRL_TX_CDC_MEM_CTRL_TM_FIELD,
    &LPORT_XLMAC_MEM_CTRL_RX_CDC_MEM_CTRL_TM_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_MEM_CTRL_REG = 
{
    "MEM_CTRL",
#if RU_INCLUDE_DESC
    "Memory Control register for XLMAC0/port0 (LPORT port0)",
    "Memory Control register",
#endif
    LPORT_XLMAC_MEM_CTRL_REG_OFFSET,
    0,
    0,
    64,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    LPORT_XLMAC_MEM_CTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_ECC_CTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_ECC_CTRL_FIELDS[] =
{
    &LPORT_XLMAC_ECC_CTRL_RESERVED0_FIELD,
    &LPORT_XLMAC_ECC_CTRL_TX_CDC_ECC_CTRL_EN_FIELD,
    &LPORT_XLMAC_ECC_CTRL_RX_CDC_ECC_CTRL_EN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_ECC_CTRL_REG = 
{
    "ECC_CTRL",
#if RU_INCLUDE_DESC
    "XLMAC memories ECC control register for XLMAC0/port0 (LPORT port0)",
    "XLMAC memories ECC control register",
#endif
    LPORT_XLMAC_ECC_CTRL_REG_OFFSET,
    0,
    0,
    65,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    LPORT_XLMAC_ECC_CTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_ECC_FORCE_DOUBLE_BIT_ERR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_ECC_FORCE_DOUBLE_BIT_ERR_FIELDS[] =
{
    &LPORT_XLMAC_ECC_FORCE_DOUBLE_BIT_ERR_RESERVED0_FIELD,
    &LPORT_XLMAC_ECC_FORCE_DOUBLE_BIT_ERR_TX_CDC_FORCE_DOUBLE_BIT_ERR_FIELD,
    &LPORT_XLMAC_ECC_FORCE_DOUBLE_BIT_ERR_RX_CDC_FORCE_DOUBLE_BIT_ERR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_ECC_FORCE_DOUBLE_BIT_ERR_REG = 
{
    "ECC_FORCE_DOUBLE_BIT_ERR",
#if RU_INCLUDE_DESC
    "XLMAC memories double bit error control register for XLMAC0/port0 (LPORT port0)",
    "XLMAC memories double bit error control register",
#endif
    LPORT_XLMAC_ECC_FORCE_DOUBLE_BIT_ERR_REG_OFFSET,
    0,
    0,
    66,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    LPORT_XLMAC_ECC_FORCE_DOUBLE_BIT_ERR_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_ECC_FORCE_SINGLE_BIT_ERR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_ECC_FORCE_SINGLE_BIT_ERR_FIELDS[] =
{
    &LPORT_XLMAC_ECC_FORCE_SINGLE_BIT_ERR_RESERVED0_FIELD,
    &LPORT_XLMAC_ECC_FORCE_SINGLE_BIT_ERR_TX_CDC_FORCE_SINGLE_BIT_ERR_FIELD,
    &LPORT_XLMAC_ECC_FORCE_SINGLE_BIT_ERR_RX_CDC_FORCE_SINGLE_BIT_ERR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_ECC_FORCE_SINGLE_BIT_ERR_REG = 
{
    "ECC_FORCE_SINGLE_BIT_ERR",
#if RU_INCLUDE_DESC
    "XLMAC memories single bit error control register for XLMAC0/port0 (LPORT port0)",
    "XLMAC memories single bit error control register",
#endif
    LPORT_XLMAC_ECC_FORCE_SINGLE_BIT_ERR_REG_OFFSET,
    0,
    0,
    67,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    LPORT_XLMAC_ECC_FORCE_SINGLE_BIT_ERR_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_RX_CDC_ECC_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_RX_CDC_ECC_STATUS_FIELDS[] =
{
    &LPORT_XLMAC_RX_CDC_ECC_STATUS_RESERVED0_FIELD,
    &LPORT_XLMAC_RX_CDC_ECC_STATUS_RX_CDC_DOUBLE_BIT_ERR_FIELD,
    &LPORT_XLMAC_RX_CDC_ECC_STATUS_RX_CDC_SINGLE_BIT_ERR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_RX_CDC_ECC_STATUS_REG = 
{
    "RX_CDC_ECC_STATUS",
#if RU_INCLUDE_DESC
    "Rx CDC memory ECC status registerThese bits are sticky by nature, and can be cleared by writing to the clear register. for XLMAC0/port0 (LPORT port0)",
    "Rx CDC memory ECC status register. These bits are sticky by nature, and can be cleared by writing to the clear register.",
#endif
    LPORT_XLMAC_RX_CDC_ECC_STATUS_REG_OFFSET,
    0,
    0,
    68,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    LPORT_XLMAC_RX_CDC_ECC_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_TX_CDC_ECC_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_TX_CDC_ECC_STATUS_FIELDS[] =
{
    &LPORT_XLMAC_TX_CDC_ECC_STATUS_RESERVED0_FIELD,
    &LPORT_XLMAC_TX_CDC_ECC_STATUS_TX_CDC_DOUBLE_BIT_ERR_FIELD,
    &LPORT_XLMAC_TX_CDC_ECC_STATUS_TX_CDC_SINGLE_BIT_ERR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_TX_CDC_ECC_STATUS_REG = 
{
    "TX_CDC_ECC_STATUS",
#if RU_INCLUDE_DESC
    "Tx CDC memory ECC status registerThese bits are sticky by nature, and can be cleared by writing to the clear register. for XLMAC0/port0 (LPORT port0)",
    "Tx CDC memory ECC status register. These bits are sticky by nature, and can be cleared by writing to the clear register.",
#endif
    LPORT_XLMAC_TX_CDC_ECC_STATUS_REG_OFFSET,
    0,
    0,
    69,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    LPORT_XLMAC_TX_CDC_ECC_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_CLEAR_ECC_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_CLEAR_ECC_STATUS_FIELDS[] =
{
    &LPORT_XLMAC_CLEAR_ECC_STATUS_RESERVED0_FIELD,
    &LPORT_XLMAC_CLEAR_ECC_STATUS_CLEAR_TX_CDC_DOUBLE_BIT_ERR_FIELD,
    &LPORT_XLMAC_CLEAR_ECC_STATUS_CLEAR_TX_CDC_SINGLE_BIT_ERR_FIELD,
    &LPORT_XLMAC_CLEAR_ECC_STATUS_CLEAR_RX_CDC_DOUBLE_BIT_ERR_FIELD,
    &LPORT_XLMAC_CLEAR_ECC_STATUS_CLEAR_RX_CDC_SINGLE_BIT_ERR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_CLEAR_ECC_STATUS_REG = 
{
    "CLEAR_ECC_STATUS",
#if RU_INCLUDE_DESC
    "Clear ECC status register, used to reset the sticky status bits for XLMAC0/port0 (LPORT port0)",
    "Clear ECC status register, used to reset the sticky status bits",
#endif
    LPORT_XLMAC_CLEAR_ECC_STATUS_REG_OFFSET,
    0,
    0,
    70,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    LPORT_XLMAC_CLEAR_ECC_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_INTR_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_INTR_STATUS_FIELDS[] =
{
    &LPORT_XLMAC_INTR_STATUS_RESERVED0_FIELD,
    &LPORT_XLMAC_INTR_STATUS_SUM_TS_ENTRY_VALID_FIELD,
    &LPORT_XLMAC_INTR_STATUS_SUM_LINK_INTERRUPTION_STATUS_FIELD,
    &LPORT_XLMAC_INTR_STATUS_SUM_REMOTE_FAULT_STATUS_FIELD,
    &LPORT_XLMAC_INTR_STATUS_SUM_LOCAL_FAULT_STATUS_FIELD,
    &LPORT_XLMAC_INTR_STATUS_SUM_RX_CDC_DOUBLE_BIT_ERR_FIELD,
    &LPORT_XLMAC_INTR_STATUS_SUM_RX_CDC_SINGLE_BIT_ERR_FIELD,
    &LPORT_XLMAC_INTR_STATUS_SUM_TX_CDC_DOUBLE_BIT_ERR_FIELD,
    &LPORT_XLMAC_INTR_STATUS_SUM_TX_CDC_SINGLE_BIT_ERR_FIELD,
    &LPORT_XLMAC_INTR_STATUS_SUM_RX_MSG_OVERFLOW_FIELD,
    &LPORT_XLMAC_INTR_STATUS_SUM_RX_PKT_OVERFLOW_FIELD,
    &LPORT_XLMAC_INTR_STATUS_SUM_TX_TS_FIFO_OVERFLOW_FIELD,
    &LPORT_XLMAC_INTR_STATUS_SUM_TX_LLFC_MSG_OVERFLOW_FIELD,
    &LPORT_XLMAC_INTR_STATUS_SUM_TX_PKT_OVERFLOW_FIELD,
    &LPORT_XLMAC_INTR_STATUS_SUM_TX_PKT_UNDERFLOW_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_INTR_STATUS_REG = 
{
    "INTR_STATUS",
#if RU_INCLUDE_DESC
    "XLMAC interrupt status register for XLMAC0/port0 (LPORT port0)",
    "XLMAC interrupt status register",
#endif
    LPORT_XLMAC_INTR_STATUS_REG_OFFSET,
    0,
    0,
    71,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    15,
    LPORT_XLMAC_INTR_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_INTR_ENABLE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_INTR_ENABLE_FIELDS[] =
{
    &LPORT_XLMAC_INTR_ENABLE_RESERVED0_FIELD,
    &LPORT_XLMAC_INTR_ENABLE_EN_TS_ENTRY_VALID_FIELD,
    &LPORT_XLMAC_INTR_ENABLE_EN_LINK_INTERRUPTION_STATUS_FIELD,
    &LPORT_XLMAC_INTR_ENABLE_EN_REMOTE_FAULT_STATUS_FIELD,
    &LPORT_XLMAC_INTR_ENABLE_EN_LOCAL_FAULT_STATUS_FIELD,
    &LPORT_XLMAC_INTR_ENABLE_EN_RX_CDC_DOUBLE_BIT_ERR_FIELD,
    &LPORT_XLMAC_INTR_ENABLE_EN_RX_CDC_SINGLE_BIT_ERR_FIELD,
    &LPORT_XLMAC_INTR_ENABLE_EN_TX_CDC_DOUBLE_BIT_ERR_FIELD,
    &LPORT_XLMAC_INTR_ENABLE_EN_TX_CDC_SINGLE_BIT_ERR_FIELD,
    &LPORT_XLMAC_INTR_ENABLE_EN_RX_MSG_OVERFLOW_FIELD,
    &LPORT_XLMAC_INTR_ENABLE_EN_RX_PKT_OVERFLOW_FIELD,
    &LPORT_XLMAC_INTR_ENABLE_EN_TX_TS_FIFO_OVERFLOW_FIELD,
    &LPORT_XLMAC_INTR_ENABLE_EN_TX_LLFC_MSG_OVERFLOW_FIELD,
    &LPORT_XLMAC_INTR_ENABLE_EN_TX_PKT_OVERFLOW_FIELD,
    &LPORT_XLMAC_INTR_ENABLE_EN_TX_PKT_UNDERFLOW_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_INTR_ENABLE_REG = 
{
    "INTR_ENABLE",
#if RU_INCLUDE_DESC
    "XLMAC interrupt enable register for XLMAC0/port0 (LPORT port0)",
    "XLMAC interrupt enable register",
#endif
    LPORT_XLMAC_INTR_ENABLE_REG_OFFSET,
    0,
    0,
    72,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    15,
    LPORT_XLMAC_INTR_ENABLE_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Register: LPORT_XLMAC_VERSION_ID
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_XLMAC_VERSION_ID_FIELDS[] =
{
    &LPORT_XLMAC_VERSION_ID_RESERVED0_FIELD,
    &LPORT_XLMAC_VERSION_ID_XLMAC_VERSION_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_XLMAC_VERSION_ID_REG = 
{
    "VERSION_ID",
#if RU_INCLUDE_DESC
    "Version ID register for XLMAC0/port0 (LPORT port0)",
    "Version ID register",
#endif
    LPORT_XLMAC_VERSION_ID_REG_OFFSET,
    0,
    0,
    73,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LPORT_XLMAC_VERSION_ID_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_64
};

/******************************************************************************
 * Block: LPORT_XLMAC
 ******************************************************************************/
static const ru_reg_rec *LPORT_XLMAC_REGS[] =
{
    &LPORT_XLMAC_CTRL_REG,
    &LPORT_XLMAC_MODE_REG,
    &LPORT_XLMAC_SPARE0_REG,
    &LPORT_XLMAC_SPARE1_REG,
    &LPORT_XLMAC_TX_CTRL_REG,
    &LPORT_XLMAC_TX_CTRL_OVERLAY_REG,
    &LPORT_XLMAC_TX_MAC_SA_REG,
    &LPORT_XLMAC_TX_MAC_SA_OVERLAY_REG,
    &LPORT_XLMAC_RX_CTRL_REG,
    &LPORT_XLMAC_RX_MAC_SA_REG,
    &LPORT_XLMAC_RX_MAC_SA_OVERLAY_REG,
    &LPORT_XLMAC_RX_MAX_SIZE_REG,
    &LPORT_XLMAC_RX_VLAN_TAG_REG,
    &LPORT_XLMAC_RX_LSS_CTRL_REG,
    &LPORT_XLMAC_RX_LSS_STATUS_REG,
    &LPORT_XLMAC_CLEAR_RX_LSS_STATUS_REG,
    &LPORT_XLMAC_PAUSE_CTRL_REG,
    &LPORT_XLMAC_PAUSE_CTRL_OVERLAY_REG,
    &LPORT_XLMAC_PFC_CTRL_REG,
    &LPORT_XLMAC_PFC_CTRL_OVERLAY_REG,
    &LPORT_XLMAC_PFC_TYPE_REG,
    &LPORT_XLMAC_PFC_OPCODE_REG,
    &LPORT_XLMAC_PFC_DA_REG,
    &LPORT_XLMAC_PFC_DA_OVERLAY_REG,
    &LPORT_XLMAC_LLFC_CTRL_REG,
    &LPORT_XLMAC_TX_LLFC_MSG_FIELDS_REG,
    &LPORT_XLMAC_RX_LLFC_MSG_FIELDS_REG,
    &LPORT_XLMAC_TX_TIMESTAMP_FIFO_DATA_REG,
    &LPORT_XLMAC_TX_TIMESTAMP_FIFO_STATUS_REG,
    &LPORT_XLMAC_FIFO_STATUS_REG,
    &LPORT_XLMAC_CLEAR_FIFO_STATUS_REG,
    &LPORT_XLMAC_LAG_FAILOVER_STATUS_REG,
    &LPORT_XLMAC_EEE_CTRL_REG,
    &LPORT_XLMAC_EEE_TIMERS_REG,
    &LPORT_XLMAC_EEE_TIMERS_OVERLAY_REG,
    &LPORT_XLMAC_EEE_1_SEC_LINK_STATUS_TIMER_REG,
    &LPORT_XLMAC_HIGIG_HDR_0_REG,
    &LPORT_XLMAC_HIGIG_HDR_0_OVERLAY_REG,
    &LPORT_XLMAC_HIGIG_HDR_1_REG,
    &LPORT_XLMAC_HIGIG_HDR_1_OVERLAY_REG,
    &LPORT_XLMAC_GMII_EEE_CTRL_REG,
    &LPORT_XLMAC_TIMESTAMP_ADJUST_REG,
    &LPORT_XLMAC_TIMESTAMP_BYTE_ADJUST_REG,
    &LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_REG,
    &LPORT_XLMAC_TX_CRC_CORRUPT_CTRL_OVERLAY_REG,
    &LPORT_XLMAC_E2E_CTRL_REG,
    &LPORT_XLMAC_E2ECC_MODULE_HDR_0_REG,
    &LPORT_XLMAC_E2ECC_MODULE_HDR_0_OVERLAY_REG,
    &LPORT_XLMAC_E2ECC_MODULE_HDR_1_REG,
    &LPORT_XLMAC_E2ECC_MODULE_HDR_1_OVERLAY_REG,
    &LPORT_XLMAC_E2ECC_DATA_HDR_0_REG,
    &LPORT_XLMAC_E2ECC_DATA_HDR_0_OVERLAY_REG,
    &LPORT_XLMAC_E2ECC_DATA_HDR_1_REG,
    &LPORT_XLMAC_E2ECC_DATA_HDR_1_OVERLAY_REG,
    &LPORT_XLMAC_E2EFC_MODULE_HDR_0_REG,
    &LPORT_XLMAC_E2EFC_MODULE_HDR_0_OVERLAY_REG,
    &LPORT_XLMAC_E2EFC_MODULE_HDR_1_REG,
    &LPORT_XLMAC_E2EFC_MODULE_HDR_1_OVERLAY_REG,
    &LPORT_XLMAC_E2EFC_DATA_HDR_0_REG,
    &LPORT_XLMAC_E2EFC_DATA_HDR_0_OVERLAY_REG,
    &LPORT_XLMAC_E2EFC_DATA_HDR_1_REG,
    &LPORT_XLMAC_E2EFC_DATA_HDR_1_OVERLAY_REG,
    &LPORT_XLMAC_TXFIFO_CELL_CNT_REG,
    &LPORT_XLMAC_TXFIFO_CELL_REQ_CNT_REG,
    &LPORT_XLMAC_MEM_CTRL_REG,
    &LPORT_XLMAC_ECC_CTRL_REG,
    &LPORT_XLMAC_ECC_FORCE_DOUBLE_BIT_ERR_REG,
    &LPORT_XLMAC_ECC_FORCE_SINGLE_BIT_ERR_REG,
    &LPORT_XLMAC_RX_CDC_ECC_STATUS_REG,
    &LPORT_XLMAC_TX_CDC_ECC_STATUS_REG,
    &LPORT_XLMAC_CLEAR_ECC_STATUS_REG,
    &LPORT_XLMAC_INTR_STATUS_REG,
    &LPORT_XLMAC_INTR_ENABLE_REG,
    &LPORT_XLMAC_VERSION_ID_REG,
};

unsigned long LPORT_XLMAC_ADDRS[] =
{
    0x80138000,
    0x80138400,
    0x80138800,
    0x80138c00,
    0x8013a000,
    0x8013a400,
    0x8013a800,
    0x8013ac00,
};

const ru_block_rec LPORT_XLMAC_BLOCK = 
{
    "LPORT_XLMAC",
    LPORT_XLMAC_ADDRS,
    8,
    74,
    LPORT_XLMAC_REGS
};

/* End of file BCM6858_A0LPORT_XLMAC.c */
