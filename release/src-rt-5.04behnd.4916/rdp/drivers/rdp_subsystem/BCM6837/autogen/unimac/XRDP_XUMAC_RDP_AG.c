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


#include "XRDP_XUMAC_RDP_AG.h"

/******************************************************************************
 * Register: NAME: XUMAC_RDP_IPG_HD_BKP_CNTL, TYPE: Type_IPG_HD_BKP_CNTL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: HD_FC_ENA *****/
const ru_field_rec XUMAC_RDP_IPG_HD_BKP_CNTL_HD_FC_ENA_FIELD =
{
    "HD_FC_ENA",
#if RU_INCLUDE_DESC
    "",
    "When set, enables back-pressure in half-duplex mode.\n",
#endif
    { XUMAC_RDP_IPG_HD_BKP_CNTL_HD_FC_ENA_FIELD_MASK },
    0,
    { XUMAC_RDP_IPG_HD_BKP_CNTL_HD_FC_ENA_FIELD_WIDTH },
    { XUMAC_RDP_IPG_HD_BKP_CNTL_HD_FC_ENA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: HD_FC_BKOFF_OK *****/
const ru_field_rec XUMAC_RDP_IPG_HD_BKP_CNTL_HD_FC_BKOFF_OK_FIELD =
{
    "HD_FC_BKOFF_OK",
#if RU_INCLUDE_DESC
    "",
    "Register bit 1 refers to the application of backoff algorithm during HD backpressure.\n",
#endif
    { XUMAC_RDP_IPG_HD_BKP_CNTL_HD_FC_BKOFF_OK_FIELD_MASK },
    0,
    { XUMAC_RDP_IPG_HD_BKP_CNTL_HD_FC_BKOFF_OK_FIELD_WIDTH },
    { XUMAC_RDP_IPG_HD_BKP_CNTL_HD_FC_BKOFF_OK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: IPG_CONFIG_RX *****/
const ru_field_rec XUMAC_RDP_IPG_HD_BKP_CNTL_IPG_CONFIG_RX_FIELD =
{
    "IPG_CONFIG_RX",
#if RU_INCLUDE_DESC
    "",
    "The programmable Rx IPG below which the packets received are dropped graciously. The value is in Bytes for 1/2.5G and Nibbles for 10/100M.\n",
#endif
    { XUMAC_RDP_IPG_HD_BKP_CNTL_IPG_CONFIG_RX_FIELD_MASK },
    0,
    { XUMAC_RDP_IPG_HD_BKP_CNTL_IPG_CONFIG_RX_FIELD_WIDTH },
    { XUMAC_RDP_IPG_HD_BKP_CNTL_IPG_CONFIG_RX_FIELD_SHIFT },
    5,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_IPG_HD_BKP_CNTL_FIELDS[] =
{
    &XUMAC_RDP_IPG_HD_BKP_CNTL_HD_FC_ENA_FIELD,
    &XUMAC_RDP_IPG_HD_BKP_CNTL_HD_FC_BKOFF_OK_FIELD,
    &XUMAC_RDP_IPG_HD_BKP_CNTL_IPG_CONFIG_RX_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_IPG_HD_BKP_CNTL *****/
const ru_reg_rec XUMAC_RDP_IPG_HD_BKP_CNTL_REG =
{
    "IPG_HD_BKP_CNTL",
#if RU_INCLUDE_DESC
    "The control register for HD-BackPressure.",
    "",
#endif
    { XUMAC_RDP_IPG_HD_BKP_CNTL_REG_OFFSET },
    0,
    0,
    1161,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    XUMAC_RDP_IPG_HD_BKP_CNTL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_COMMAND_CONFIG, TYPE: Type_COMMAND_CONFIG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TX_ENA *****/
const ru_field_rec XUMAC_RDP_COMMAND_CONFIG_TX_ENA_FIELD =
{
    "TX_ENA",
#if RU_INCLUDE_DESC
    "",
    "Enable/Disable MAC transmit path for data packets & pause/pfc packets sent in the normal data path.\nPause/pfc packets generated internally are allowed if ignore_tx_pause is not set. When set to '0' (Reset value), the MAC \ntransmit function is disable.  When set to '1', the MAC transmit function is enabled.\n",
#endif
    { XUMAC_RDP_COMMAND_CONFIG_TX_ENA_FIELD_MASK },
    0,
    { XUMAC_RDP_COMMAND_CONFIG_TX_ENA_FIELD_WIDTH },
    { XUMAC_RDP_COMMAND_CONFIG_TX_ENA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RX_ENA *****/
const ru_field_rec XUMAC_RDP_COMMAND_CONFIG_RX_ENA_FIELD =
{
    "RX_ENA",
#if RU_INCLUDE_DESC
    "",
    "Enable/Disable MAC receive path. When set to '0' (Reset value), the MAC \nreceive function is disable.  When set to '1', the MAC receive function is enabled.\n",
#endif
    { XUMAC_RDP_COMMAND_CONFIG_RX_ENA_FIELD_MASK },
    0,
    { XUMAC_RDP_COMMAND_CONFIG_RX_ENA_FIELD_WIDTH },
    { XUMAC_RDP_COMMAND_CONFIG_RX_ENA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ETH_SPEED *****/
const ru_field_rec XUMAC_RDP_COMMAND_CONFIG_ETH_SPEED_FIELD =
{
    "ETH_SPEED",
#if RU_INCLUDE_DESC
    "",
    "Set MAC speed. Bit 1:0 for ETH_SPEED[2:0]. Bit 2 is in ETH_SPEED_BIT2. Ignored when the register bit ENA_EXT_CONFIG is set to '1'.  When the Register bit ENA_EXT_CONFIG is set to '0', used to set the core mode of operation: 000: Enable 10Mbps Ethernet mode 001: Enable 100Mbps Ethernet mode 010: Enable Gigabit Ethernet mode 011: Enable 2.5Gigabit Ethernet mode 101: Enable 5Gigabit Ethernet mode 100 Enable 10Gigabit Ethernet mode\n",
#endif
    { XUMAC_RDP_COMMAND_CONFIG_ETH_SPEED_FIELD_MASK },
    0,
    { XUMAC_RDP_COMMAND_CONFIG_ETH_SPEED_FIELD_WIDTH },
    { XUMAC_RDP_COMMAND_CONFIG_ETH_SPEED_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PROMIS_EN *****/
const ru_field_rec XUMAC_RDP_COMMAND_CONFIG_PROMIS_EN_FIELD =
{
    "PROMIS_EN",
#if RU_INCLUDE_DESC
    "",
    "Enable/Disable MAC promiscuous operation. When asserted (Set to '1'), \nall frames are received without Unicast address filtering.\n",
#endif
    { XUMAC_RDP_COMMAND_CONFIG_PROMIS_EN_FIELD_MASK },
    0,
    { XUMAC_RDP_COMMAND_CONFIG_PROMIS_EN_FIELD_WIDTH },
    { XUMAC_RDP_COMMAND_CONFIG_PROMIS_EN_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PAD_EN *****/
const ru_field_rec XUMAC_RDP_COMMAND_CONFIG_PAD_EN_FIELD =
{
    "PAD_EN",
#if RU_INCLUDE_DESC
    "",
    "Enable/Disable Frame Padding. If enabled (Set to '1'), then padding is removed from the received frame before it is transmitted to the user\napplication. If disabled (set to reset value '0'), then no padding is removed on receive by the MAC. \nThis bit has no effect on Tx padding and hence Transmit always pad runts to guarantee a minimum frame size of 64 octets.\n",
#endif
    { XUMAC_RDP_COMMAND_CONFIG_PAD_EN_FIELD_MASK },
    0,
    { XUMAC_RDP_COMMAND_CONFIG_PAD_EN_FIELD_WIDTH },
    { XUMAC_RDP_COMMAND_CONFIG_PAD_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CRC_FWD *****/
const ru_field_rec XUMAC_RDP_COMMAND_CONFIG_CRC_FWD_FIELD =
{
    "CRC_FWD",
#if RU_INCLUDE_DESC
    "",
    "Terminate/Forward Received CRC. If enabled (1) the CRC field of received \nframes are transmitted to the user application.\nIf disabled (Set to reset value '0') the CRC field is stripped from the frame.\nNote: If padding function (bit PAD_EN set to '1') is enabled. CRC_FWD is \nignored and the CRC field is checked and always terminated and removed.\n",
#endif
    { XUMAC_RDP_COMMAND_CONFIG_CRC_FWD_FIELD_MASK },
    0,
    { XUMAC_RDP_COMMAND_CONFIG_CRC_FWD_FIELD_WIDTH },
    { XUMAC_RDP_COMMAND_CONFIG_CRC_FWD_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PAUSE_FWD *****/
const ru_field_rec XUMAC_RDP_COMMAND_CONFIG_PAUSE_FWD_FIELD =
{
    "PAUSE_FWD",
#if RU_INCLUDE_DESC
    "",
    "Terminate/Forward Pause Frames. If enabled (Set to '1') pause frames are \nforwarded to the user application.  If disabled (Set to reset value '0'), \npause frames are terminated and discarded in the MAC.\n",
#endif
    { XUMAC_RDP_COMMAND_CONFIG_PAUSE_FWD_FIELD_MASK },
    0,
    { XUMAC_RDP_COMMAND_CONFIG_PAUSE_FWD_FIELD_WIDTH },
    { XUMAC_RDP_COMMAND_CONFIG_PAUSE_FWD_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PAUSE_IGNORE *****/
const ru_field_rec XUMAC_RDP_COMMAND_CONFIG_PAUSE_IGNORE_FIELD =
{
    "PAUSE_IGNORE",
#if RU_INCLUDE_DESC
    "",
    "Ignore Pause Frame Quanta. If enabled (Set to '1') received pause frames \nare ignored by the MAC. When disabled (Set to reset value '0') the transmit \nprocess is stopped for the amount of time specified in the pause quanta \nreceived within the pause frame.\n",
#endif
    { XUMAC_RDP_COMMAND_CONFIG_PAUSE_IGNORE_FIELD_MASK },
    0,
    { XUMAC_RDP_COMMAND_CONFIG_PAUSE_IGNORE_FIELD_WIDTH },
    { XUMAC_RDP_COMMAND_CONFIG_PAUSE_IGNORE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TX_ADDR_INS *****/
const ru_field_rec XUMAC_RDP_COMMAND_CONFIG_TX_ADDR_INS_FIELD =
{
    "TX_ADDR_INS",
#if RU_INCLUDE_DESC
    "",
    "Set MAC address on transmit. If enabled (Set to '1') the MAC overwrites \nthe source MAC address with the programmed MAC address in registers MAC_0 \nand MAC_1. If disabled (Set to reset value '0'), the source MAC address \nreceived from the transmit application transmitted is not modified by the MAC.\n",
#endif
    { XUMAC_RDP_COMMAND_CONFIG_TX_ADDR_INS_FIELD_MASK },
    0,
    { XUMAC_RDP_COMMAND_CONFIG_TX_ADDR_INS_FIELD_WIDTH },
    { XUMAC_RDP_COMMAND_CONFIG_TX_ADDR_INS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: HD_ENA *****/
const ru_field_rec XUMAC_RDP_COMMAND_CONFIG_HD_ENA_FIELD =
{
    "HD_ENA",
#if RU_INCLUDE_DESC
    "",
    "Half duplex enable. When set to '1', enables half duplex mode, when set \nto '0', the MAC operates in full duplex mode.\nIgnored at ethernet speeds 1G/2.5G or when the register ENA_EXT_CONFIG is set to '1'.\n",
#endif
    { XUMAC_RDP_COMMAND_CONFIG_HD_ENA_FIELD_MASK },
    0,
    { XUMAC_RDP_COMMAND_CONFIG_HD_ENA_FIELD_WIDTH },
    { XUMAC_RDP_COMMAND_CONFIG_HD_ENA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RX_LOW_LATENCY_EN *****/
const ru_field_rec XUMAC_RDP_COMMAND_CONFIG_RX_LOW_LATENCY_EN_FIELD =
{
    "RX_LOW_LATENCY_EN",
#if RU_INCLUDE_DESC
    "",
    "This works only when runt filter is disabled. It reduces the receive latency by 48 MAC clock time.\n",
#endif
    { XUMAC_RDP_COMMAND_CONFIG_RX_LOW_LATENCY_EN_FIELD_MASK },
    0,
    { XUMAC_RDP_COMMAND_CONFIG_RX_LOW_LATENCY_EN_FIELD_WIDTH },
    { XUMAC_RDP_COMMAND_CONFIG_RX_LOW_LATENCY_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: OVERFLOW_EN *****/
const ru_field_rec XUMAC_RDP_COMMAND_CONFIG_OVERFLOW_EN_FIELD =
{
    "OVERFLOW_EN",
#if RU_INCLUDE_DESC
    "",
    "If set, enables Rx FIFO overflow logic. In this case, the RXFIFO_STAT[1] register bit is not operational (always set to 0).\nIf cleared, disables RX FIFO overflow logic. In this case, the RXFIFO_STAT[1] register bit is operational (Sticky set when overrun occurs, clearable only by SW_Reset).\n",
#endif
    { XUMAC_RDP_COMMAND_CONFIG_OVERFLOW_EN_FIELD_MASK },
    0,
    { XUMAC_RDP_COMMAND_CONFIG_OVERFLOW_EN_FIELD_WIDTH },
    { XUMAC_RDP_COMMAND_CONFIG_OVERFLOW_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SW_RESET *****/
const ru_field_rec XUMAC_RDP_COMMAND_CONFIG_SW_RESET_FIELD =
{
    "SW_RESET",
#if RU_INCLUDE_DESC
    "",
    "Software Reset Command. When asserted, the TX and RX are \ndisabled. Config registers are not affected by sw reset. Write a 0 to de-assert the sw reset.\n",
#endif
    { XUMAC_RDP_COMMAND_CONFIG_SW_RESET_FIELD_MASK },
    0,
    { XUMAC_RDP_COMMAND_CONFIG_SW_RESET_FIELD_WIDTH },
    { XUMAC_RDP_COMMAND_CONFIG_SW_RESET_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FCS_CORRUPT_URUN_EN *****/
const ru_field_rec XUMAC_RDP_COMMAND_CONFIG_FCS_CORRUPT_URUN_EN_FIELD =
{
    "FCS_CORRUPT_URUN_EN",
#if RU_INCLUDE_DESC
    "",
    "Corrupt Tx FCS, on underrun, when set to '1', No FCS corruption when set to '0' (Reset value).\n",
#endif
    { XUMAC_RDP_COMMAND_CONFIG_FCS_CORRUPT_URUN_EN_FIELD_MASK },
    0,
    { XUMAC_RDP_COMMAND_CONFIG_FCS_CORRUPT_URUN_EN_FIELD_WIDTH },
    { XUMAC_RDP_COMMAND_CONFIG_FCS_CORRUPT_URUN_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: LOOP_ENA *****/
const ru_field_rec XUMAC_RDP_COMMAND_CONFIG_LOOP_ENA_FIELD =
{
    "LOOP_ENA",
#if RU_INCLUDE_DESC
    "",
    "Enable GMII/MII loopback (TX to RX) when set to '1', normal operation when set to '0' (Reset value).\n",
#endif
    { XUMAC_RDP_COMMAND_CONFIG_LOOP_ENA_FIELD_MASK },
    0,
    { XUMAC_RDP_COMMAND_CONFIG_LOOP_ENA_FIELD_WIDTH },
    { XUMAC_RDP_COMMAND_CONFIG_LOOP_ENA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MAC_LOOP_CON *****/
const ru_field_rec XUMAC_RDP_COMMAND_CONFIG_MAC_LOOP_CON_FIELD =
{
    "MAC_LOOP_CON",
#if RU_INCLUDE_DESC
    "",
    "Transmit packets to PHY while in MAC local loopback, when set to '1', otherwise transmit to PHY is disabled (normal operation),\nwhen set to '0' (Reset value).\n",
#endif
    { XUMAC_RDP_COMMAND_CONFIG_MAC_LOOP_CON_FIELD_MASK },
    0,
    { XUMAC_RDP_COMMAND_CONFIG_MAC_LOOP_CON_FIELD_WIDTH },
    { XUMAC_RDP_COMMAND_CONFIG_MAC_LOOP_CON_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SW_OVERRIDE_TX *****/
const ru_field_rec XUMAC_RDP_COMMAND_CONFIG_SW_OVERRIDE_TX_FIELD =
{
    "SW_OVERRIDE_TX",
#if RU_INCLUDE_DESC
    "",
    "If set, enables the SW programmed Tx pause capability config bits to overwrite the auto negotiated Tx pause capabilities when ena_ext_config (autoconfig) is set.\nIf cleared, and when ena_ext_config (autoconfig) is set, then SW programmed Tx pause capability config bits has no effect over auto negotiated capabilities.\n",
#endif
    { XUMAC_RDP_COMMAND_CONFIG_SW_OVERRIDE_TX_FIELD_MASK },
    0,
    { XUMAC_RDP_COMMAND_CONFIG_SW_OVERRIDE_TX_FIELD_WIDTH },
    { XUMAC_RDP_COMMAND_CONFIG_SW_OVERRIDE_TX_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SW_OVERRIDE_RX *****/
const ru_field_rec XUMAC_RDP_COMMAND_CONFIG_SW_OVERRIDE_RX_FIELD =
{
    "SW_OVERRIDE_RX",
#if RU_INCLUDE_DESC
    "",
    "If set, enables the SW programmed Rx pause capability config bits to overwrite the auto negotiated Rx pause capabilities when ena_ext_config (autoconfig) is set.\nIf cleared, and when ena_ext_config (autoconfig) is set, then SW programmed Rx pause capability config bits has no effect over auto negotiated capabilities.\n",
#endif
    { XUMAC_RDP_COMMAND_CONFIG_SW_OVERRIDE_RX_FIELD_MASK },
    0,
    { XUMAC_RDP_COMMAND_CONFIG_SW_OVERRIDE_RX_FIELD_WIDTH },
    { XUMAC_RDP_COMMAND_CONFIG_SW_OVERRIDE_RX_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: OOB_EFC_MODE *****/
const ru_field_rec XUMAC_RDP_COMMAND_CONFIG_OOB_EFC_MODE_FIELD =
{
    "OOB_EFC_MODE",
#if RU_INCLUDE_DESC
    "",
    "0=> strict/full OOB egress backpressure mode:\n- pause frames and PFC frames, as well as regular packets, are all affected by Unimac input ext_tx_flow_control, as long as OOB_EFC_DISAB is 0\n- in this mode, OOB backpressure will be active as long as ext_tx_flow_control is asserted and i_oob_efc_disab is 0, regardless of whether the MAC operates in half duplex mode or full duplex mode\n1=> legacy mode:\n- ext_tx_flow_control does not affect (does not prevent) transmission of Pause and PFC frames, i.e. in this mode OOB egress backpressure may only affect transmission of regular packets\n- OOB egress backpressure is fully disabled (ignored) when the MAC operates in half duplex mode.\n",
#endif
    { XUMAC_RDP_COMMAND_CONFIG_OOB_EFC_MODE_FIELD_MASK },
    0,
    { XUMAC_RDP_COMMAND_CONFIG_OOB_EFC_MODE_FIELD_WIDTH },
    { XUMAC_RDP_COMMAND_CONFIG_OOB_EFC_MODE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BYPASS_OOB_EFC_SYNCHRONIZER *****/
const ru_field_rec XUMAC_RDP_COMMAND_CONFIG_BYPASS_OOB_EFC_SYNCHRONIZER_FIELD =
{
    "BYPASS_OOB_EFC_SYNCHRONIZER",
#if RU_INCLUDE_DESC
    "",
    "1=> bypass the OOB external flow control signal synchronizer, to e.g. reduce latency. In this case it is assumed/required that Unimac input ext_tx_flow_control is already in tx_clk clock domain (so there is no need to synchronize it)\n0=> locally synchronize the OOB egress flow control signal to tx_clk. In this case it is assumed/required that ext_tx_flow_control is glitchless (e.g. registered in its native clock domain).\n",
#endif
    { XUMAC_RDP_COMMAND_CONFIG_BYPASS_OOB_EFC_SYNCHRONIZER_FIELD_MASK },
    0,
    { XUMAC_RDP_COMMAND_CONFIG_BYPASS_OOB_EFC_SYNCHRONIZER_FIELD_WIDTH },
    { XUMAC_RDP_COMMAND_CONFIG_BYPASS_OOB_EFC_SYNCHRONIZER_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EN_INTERNAL_TX_CRS *****/
const ru_field_rec XUMAC_RDP_COMMAND_CONFIG_EN_INTERNAL_TX_CRS_FIELD =
{
    "EN_INTERNAL_TX_CRS",
#if RU_INCLUDE_DESC
    "",
    "If enabled, then CRS input to Unimac is ORed with tds[8] (tx data valid output). This is helpful when TX CRS is disabled inside PHY.\n",
#endif
    { XUMAC_RDP_COMMAND_CONFIG_EN_INTERNAL_TX_CRS_FIELD_MASK },
    0,
    { XUMAC_RDP_COMMAND_CONFIG_EN_INTERNAL_TX_CRS_FIELD_WIDTH },
    { XUMAC_RDP_COMMAND_CONFIG_EN_INTERNAL_TX_CRS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ENA_EXT_CONFIG *****/
const ru_field_rec XUMAC_RDP_COMMAND_CONFIG_ENA_EXT_CONFIG_FIELD =
{
    "ENA_EXT_CONFIG",
#if RU_INCLUDE_DESC
    "",
    "Enable Configuration with External Pins. When set to '0' (Reset value) \nthe Core speed and Mode is programmed with the register bits ETH_SPEED(2:0) \nand HD_ENA. When set to '1', the Core is configured with the pins \nset_speed(1:0) and set_duplex.\n",
#endif
    { XUMAC_RDP_COMMAND_CONFIG_ENA_EXT_CONFIG_FIELD_MASK },
    0,
    { XUMAC_RDP_COMMAND_CONFIG_ENA_EXT_CONFIG_FIELD_WIDTH },
    { XUMAC_RDP_COMMAND_CONFIG_ENA_EXT_CONFIG_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CNTL_FRM_ENA *****/
const ru_field_rec XUMAC_RDP_COMMAND_CONFIG_CNTL_FRM_ENA_FIELD =
{
    "CNTL_FRM_ENA",
#if RU_INCLUDE_DESC
    "",
    "MAC Control Frame Enable. When set to '1', MAC Control frames with any \nOpcode other than 0x0001 are accepted and forward to the Client interface. \nWhen set to '0' (Reset value), MAC Control frames with any Opcode other \nthan 0x0001 are silently discarded.\n",
#endif
    { XUMAC_RDP_COMMAND_CONFIG_CNTL_FRM_ENA_FIELD_MASK },
    0,
    { XUMAC_RDP_COMMAND_CONFIG_CNTL_FRM_ENA_FIELD_WIDTH },
    { XUMAC_RDP_COMMAND_CONFIG_CNTL_FRM_ENA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: NO_LGTH_CHECK *****/
const ru_field_rec XUMAC_RDP_COMMAND_CONFIG_NO_LGTH_CHECK_FIELD =
{
    "NO_LGTH_CHECK",
#if RU_INCLUDE_DESC
    "",
    "Payload Length Check Disable. When set to '0', the Core checks the frame's payload length with the Frame\nLength/Type field, when set to '1'(Reset value), the payload length check is disabled.\n",
#endif
    { XUMAC_RDP_COMMAND_CONFIG_NO_LGTH_CHECK_FIELD_MASK },
    0,
    { XUMAC_RDP_COMMAND_CONFIG_NO_LGTH_CHECK_FIELD_WIDTH },
    { XUMAC_RDP_COMMAND_CONFIG_NO_LGTH_CHECK_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: LINE_LOOPBACK *****/
const ru_field_rec XUMAC_RDP_COMMAND_CONFIG_LINE_LOOPBACK_FIELD =
{
    "LINE_LOOPBACK",
#if RU_INCLUDE_DESC
    "",
    "Enable Line Loopback i.e. MAC FIFO side loopback (RX to TX) when set to '1', normal operation when set to '0' (Reset value).\n",
#endif
    { XUMAC_RDP_COMMAND_CONFIG_LINE_LOOPBACK_FIELD_MASK },
    0,
    { XUMAC_RDP_COMMAND_CONFIG_LINE_LOOPBACK_FIELD_WIDTH },
    { XUMAC_RDP_COMMAND_CONFIG_LINE_LOOPBACK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FD_TX_URUN_FIX_EN *****/
const ru_field_rec XUMAC_RDP_COMMAND_CONFIG_FD_TX_URUN_FIX_EN_FIELD =
{
    "FD_TX_URUN_FIX_EN",
#if RU_INCLUDE_DESC
    "",
    "Tx Underflow detection can be improved by accounting for residue bytes in 128b to 8b convertor. The fix is valid only for full duplex mode and can be enabled by setting this bit.\n",
#endif
    { XUMAC_RDP_COMMAND_CONFIG_FD_TX_URUN_FIX_EN_FIELD_MASK },
    0,
    { XUMAC_RDP_COMMAND_CONFIG_FD_TX_URUN_FIX_EN_FIELD_WIDTH },
    { XUMAC_RDP_COMMAND_CONFIG_FD_TX_URUN_FIX_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: IGNORE_TX_PAUSE *****/
const ru_field_rec XUMAC_RDP_COMMAND_CONFIG_IGNORE_TX_PAUSE_FIELD =
{
    "IGNORE_TX_PAUSE",
#if RU_INCLUDE_DESC
    "",
    "Ignores the back pressure signaling from the system and hence no Tx pause generation, when set.\n",
#endif
    { XUMAC_RDP_COMMAND_CONFIG_IGNORE_TX_PAUSE_FIELD_MASK },
    0,
    { XUMAC_RDP_COMMAND_CONFIG_IGNORE_TX_PAUSE_FIELD_WIDTH },
    { XUMAC_RDP_COMMAND_CONFIG_IGNORE_TX_PAUSE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: OOB_EFC_DISAB *****/
const ru_field_rec XUMAC_RDP_COMMAND_CONFIG_OOB_EFC_DISAB_FIELD =
{
    "OOB_EFC_DISAB",
#if RU_INCLUDE_DESC
    "",
    "When this bit is set, out-of-band egress flow control will be disabled. When this bit is 0 (out-of-band egress flow control enabled) and input pin ext_tx_flow_control is 1, frame transmissions may be stopped - see OOB_EFC_MODE for details.\nOut-of-band egress flow control operation is similar to halting the transmit datapath due to reception of a Pause Frame with a non-zero timer value. This bit however has no effect on regular Rx Pause Frame based egress flow control.\n",
#endif
    { XUMAC_RDP_COMMAND_CONFIG_OOB_EFC_DISAB_FIELD_MASK },
    0,
    { XUMAC_RDP_COMMAND_CONFIG_OOB_EFC_DISAB_FIELD_WIDTH },
    { XUMAC_RDP_COMMAND_CONFIG_OOB_EFC_DISAB_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RUNT_FILTER_DIS *****/
const ru_field_rec XUMAC_RDP_COMMAND_CONFIG_RUNT_FILTER_DIS_FIELD =
{
    "RUNT_FILTER_DIS",
#if RU_INCLUDE_DESC
    "",
    "When set, disable runt filtering.\n",
#endif
    { XUMAC_RDP_COMMAND_CONFIG_RUNT_FILTER_DIS_FIELD_MASK },
    0,
    { XUMAC_RDP_COMMAND_CONFIG_RUNT_FILTER_DIS_FIELD_WIDTH },
    { XUMAC_RDP_COMMAND_CONFIG_RUNT_FILTER_DIS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ETH_SPEED_BIT2 *****/
const ru_field_rec XUMAC_RDP_COMMAND_CONFIG_ETH_SPEED_BIT2_FIELD =
{
    "ETH_SPEED_BIT2",
#if RU_INCLUDE_DESC
    "",
    "This is bit 2 for ETH_SPEED. See ETH_SPEED below.\n",
#endif
    { XUMAC_RDP_COMMAND_CONFIG_ETH_SPEED_BIT2_FIELD_MASK },
    0,
    { XUMAC_RDP_COMMAND_CONFIG_ETH_SPEED_BIT2_FIELD_WIDTH },
    { XUMAC_RDP_COMMAND_CONFIG_ETH_SPEED_BIT2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_COMMAND_CONFIG_FIELDS[] =
{
    &XUMAC_RDP_COMMAND_CONFIG_TX_ENA_FIELD,
    &XUMAC_RDP_COMMAND_CONFIG_RX_ENA_FIELD,
    &XUMAC_RDP_COMMAND_CONFIG_ETH_SPEED_FIELD,
    &XUMAC_RDP_COMMAND_CONFIG_PROMIS_EN_FIELD,
    &XUMAC_RDP_COMMAND_CONFIG_PAD_EN_FIELD,
    &XUMAC_RDP_COMMAND_CONFIG_CRC_FWD_FIELD,
    &XUMAC_RDP_COMMAND_CONFIG_PAUSE_FWD_FIELD,
    &XUMAC_RDP_COMMAND_CONFIG_PAUSE_IGNORE_FIELD,
    &XUMAC_RDP_COMMAND_CONFIG_TX_ADDR_INS_FIELD,
    &XUMAC_RDP_COMMAND_CONFIG_HD_ENA_FIELD,
    &XUMAC_RDP_COMMAND_CONFIG_RX_LOW_LATENCY_EN_FIELD,
    &XUMAC_RDP_COMMAND_CONFIG_OVERFLOW_EN_FIELD,
    &XUMAC_RDP_COMMAND_CONFIG_SW_RESET_FIELD,
    &XUMAC_RDP_COMMAND_CONFIG_FCS_CORRUPT_URUN_EN_FIELD,
    &XUMAC_RDP_COMMAND_CONFIG_LOOP_ENA_FIELD,
    &XUMAC_RDP_COMMAND_CONFIG_MAC_LOOP_CON_FIELD,
    &XUMAC_RDP_COMMAND_CONFIG_SW_OVERRIDE_TX_FIELD,
    &XUMAC_RDP_COMMAND_CONFIG_SW_OVERRIDE_RX_FIELD,
    &XUMAC_RDP_COMMAND_CONFIG_OOB_EFC_MODE_FIELD,
    &XUMAC_RDP_COMMAND_CONFIG_BYPASS_OOB_EFC_SYNCHRONIZER_FIELD,
    &XUMAC_RDP_COMMAND_CONFIG_EN_INTERNAL_TX_CRS_FIELD,
    &XUMAC_RDP_COMMAND_CONFIG_ENA_EXT_CONFIG_FIELD,
    &XUMAC_RDP_COMMAND_CONFIG_CNTL_FRM_ENA_FIELD,
    &XUMAC_RDP_COMMAND_CONFIG_NO_LGTH_CHECK_FIELD,
    &XUMAC_RDP_COMMAND_CONFIG_LINE_LOOPBACK_FIELD,
    &XUMAC_RDP_COMMAND_CONFIG_FD_TX_URUN_FIX_EN_FIELD,
    &XUMAC_RDP_COMMAND_CONFIG_IGNORE_TX_PAUSE_FIELD,
    &XUMAC_RDP_COMMAND_CONFIG_OOB_EFC_DISAB_FIELD,
    &XUMAC_RDP_COMMAND_CONFIG_RUNT_FILTER_DIS_FIELD,
    &XUMAC_RDP_COMMAND_CONFIG_ETH_SPEED_BIT2_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_COMMAND_CONFIG *****/
const ru_reg_rec XUMAC_RDP_COMMAND_CONFIG_REG =
{
    "COMMAND_CONFIG",
#if RU_INCLUDE_DESC
    "Command register. Used by the host processor to control and configure the core",
    "",
#endif
    { XUMAC_RDP_COMMAND_CONFIG_REG_OFFSET },
    0,
    0,
    1162,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    30,
    XUMAC_RDP_COMMAND_CONFIG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_MAC_0, TYPE: Type_MAC_0
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MAC_ADDR0 *****/
const ru_field_rec XUMAC_RDP_MAC_0_MAC_ADDR0_FIELD =
{
    "MAC_ADDR0",
#if RU_INCLUDE_DESC
    "",
    "Register bit 0 corresponds to bit 16 of the MAC address, register bit 1 corresponds to bit 17 of the MAC address, and so on.\n",
#endif
    { XUMAC_RDP_MAC_0_MAC_ADDR0_FIELD_MASK },
    0,
    { XUMAC_RDP_MAC_0_MAC_ADDR0_FIELD_WIDTH },
    { XUMAC_RDP_MAC_0_MAC_ADDR0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_MAC_0_FIELDS[] =
{
    &XUMAC_RDP_MAC_0_MAC_ADDR0_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_MAC_0 *****/
const ru_reg_rec XUMAC_RDP_MAC_0_REG =
{
    "MAC_0",
#if RU_INCLUDE_DESC
    "Core MAC address bits 47 to 16.",
    "",
#endif
    { XUMAC_RDP_MAC_0_REG_OFFSET },
    0,
    0,
    1163,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_MAC_0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_MAC_1, TYPE: Type_MAC_1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MAC_ADDR1 *****/
const ru_field_rec XUMAC_RDP_MAC_1_MAC_ADDR1_FIELD =
{
    "MAC_ADDR1",
#if RU_INCLUDE_DESC
    "",
    "Register bit 0 corresponds to bit 0 of the MAC address, register bit 1 corresponds to bit 1 of the MAC address, and so on.\nBits 16 to 31 are reserved.\n",
#endif
    { XUMAC_RDP_MAC_1_MAC_ADDR1_FIELD_MASK },
    0,
    { XUMAC_RDP_MAC_1_MAC_ADDR1_FIELD_WIDTH },
    { XUMAC_RDP_MAC_1_MAC_ADDR1_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_MAC_1_FIELDS[] =
{
    &XUMAC_RDP_MAC_1_MAC_ADDR1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_MAC_1 *****/
const ru_reg_rec XUMAC_RDP_MAC_1_REG =
{
    "MAC_1",
#if RU_INCLUDE_DESC
    "Core MAC address bits 15 to 0.",
    "",
#endif
    { XUMAC_RDP_MAC_1_REG_OFFSET },
    0,
    0,
    1164,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_MAC_1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_FRM_LENGTH, TYPE: Type_FRM_LENGTH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MAXFR *****/
const ru_field_rec XUMAC_RDP_FRM_LENGTH_MAXFR_FIELD =
{
    "MAXFR",
#if RU_INCLUDE_DESC
    "",
    "Defines a 14-bit maximum frame length used by the MAC receive logic to check frames.\n",
#endif
    { XUMAC_RDP_FRM_LENGTH_MAXFR_FIELD_MASK },
    0,
    { XUMAC_RDP_FRM_LENGTH_MAXFR_FIELD_WIDTH },
    { XUMAC_RDP_FRM_LENGTH_MAXFR_FIELD_SHIFT },
    1518,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_FRM_LENGTH_FIELDS[] =
{
    &XUMAC_RDP_FRM_LENGTH_MAXFR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_FRM_LENGTH *****/
const ru_reg_rec XUMAC_RDP_FRM_LENGTH_REG =
{
    "FRM_LENGTH",
#if RU_INCLUDE_DESC
    "Maximum Frame Length.",
    "",
#endif
    { XUMAC_RDP_FRM_LENGTH_REG_OFFSET },
    0,
    0,
    1165,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_FRM_LENGTH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_PAUSE_QUANT, TYPE: Type_PAUSE_QUANT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PAUSE_QUANT *****/
const ru_field_rec XUMAC_RDP_PAUSE_QUANT_PAUSE_QUANT_FIELD =
{
    "PAUSE_QUANT",
#if RU_INCLUDE_DESC
    "",
    "16-bit value, sets, in increments of 512 Ethernet bit times, the pause quanta used in \neach Pause Frame sent to the remote Ethernet device.\n",
#endif
    { XUMAC_RDP_PAUSE_QUANT_PAUSE_QUANT_FIELD_MASK },
    0,
    { XUMAC_RDP_PAUSE_QUANT_PAUSE_QUANT_FIELD_WIDTH },
    { XUMAC_RDP_PAUSE_QUANT_PAUSE_QUANT_FIELD_SHIFT },
    65535,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_PAUSE_QUANT_FIELDS[] =
{
    &XUMAC_RDP_PAUSE_QUANT_PAUSE_QUANT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_PAUSE_QUANT *****/
const ru_reg_rec XUMAC_RDP_PAUSE_QUANT_REG =
{
    "PAUSE_QUANT",
#if RU_INCLUDE_DESC
    "Receive Pause Quanta.",
    "",
#endif
    { XUMAC_RDP_PAUSE_QUANT_REG_OFFSET },
    0,
    0,
    1166,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_PAUSE_QUANT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_TX_TS_SEQ_ID, TYPE: Type_TX_TS_SEQ_ID
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TSTS_SEQ_ID *****/
const ru_field_rec XUMAC_RDP_TX_TS_SEQ_ID_TSTS_SEQ_ID_FIELD =
{
    "TSTS_SEQ_ID",
#if RU_INCLUDE_DESC
    "",
    "Every read of this register will fetch out one seq_id from the transmit FIFO.(One seq_id per one read command on the sbus).\nEvery 49 bit val_bit + seq_id + timestamp is read in two steps, i.e., one read from 0x10f (val_bit + seq_id) followed by another read from 0x1c7 (timestamp).\nTimestamp read without a preceding seq_id read will fetch stale timestamp value.\n",
#endif
    { XUMAC_RDP_TX_TS_SEQ_ID_TSTS_SEQ_ID_FIELD_MASK },
    0,
    { XUMAC_RDP_TX_TS_SEQ_ID_TSTS_SEQ_ID_FIELD_WIDTH },
    { XUMAC_RDP_TX_TS_SEQ_ID_TSTS_SEQ_ID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TSTS_VALID *****/
const ru_field_rec XUMAC_RDP_TX_TS_SEQ_ID_TSTS_VALID_FIELD =
{
    "TSTS_VALID",
#if RU_INCLUDE_DESC
    "",
    "Indicates that a timestamp was captured and is valid. if the cpu reads an empty fifo the VALID bit will be 0.\n",
#endif
    { XUMAC_RDP_TX_TS_SEQ_ID_TSTS_VALID_FIELD_MASK },
    0,
    { XUMAC_RDP_TX_TS_SEQ_ID_TSTS_VALID_FIELD_WIDTH },
    { XUMAC_RDP_TX_TS_SEQ_ID_TSTS_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_TX_TS_SEQ_ID_FIELDS[] =
{
    &XUMAC_RDP_TX_TS_SEQ_ID_TSTS_SEQ_ID_FIELD,
    &XUMAC_RDP_TX_TS_SEQ_ID_TSTS_VALID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_TX_TS_SEQ_ID *****/
const ru_reg_rec XUMAC_RDP_TX_TS_SEQ_ID_REG =
{
    "TX_TS_SEQ_ID",
#if RU_INCLUDE_DESC
    "Transmit Two Step Timestamp Sequence ID",
    "",
#endif
    { XUMAC_RDP_TX_TS_SEQ_ID_REG_OFFSET },
    0,
    0,
    1167,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XUMAC_RDP_TX_TS_SEQ_ID_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_SFD_OFFSET, TYPE: Type_SFD_OFFSET
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SFD_OFFSET *****/
const ru_field_rec XUMAC_RDP_SFD_OFFSET_SFD_OFFSET_FIELD =
{
    "SFD_OFFSET",
#if RU_INCLUDE_DESC
    "",
    "Defines the length of the EFM preamble between 5 and 15 Bytes. When set to 0, 1, 2, 3 or 4,\nthe Preamble EFM length is set to 5 Bytes.\n",
#endif
    { XUMAC_RDP_SFD_OFFSET_SFD_OFFSET_FIELD_MASK },
    0,
    { XUMAC_RDP_SFD_OFFSET_SFD_OFFSET_FIELD_WIDTH },
    { XUMAC_RDP_SFD_OFFSET_SFD_OFFSET_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_SFD_OFFSET_FIELDS[] =
{
    &XUMAC_RDP_SFD_OFFSET_SFD_OFFSET_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_SFD_OFFSET *****/
const ru_reg_rec XUMAC_RDP_SFD_OFFSET_REG =
{
    "SFD_OFFSET",
#if RU_INCLUDE_DESC
    "EFM Preamble Length.",
    "",
#endif
    { XUMAC_RDP_SFD_OFFSET_REG_OFFSET },
    0,
    0,
    1168,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_SFD_OFFSET_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_MAC_MODE, TYPE: Type_MAC_MODE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MAC_SPEED *****/
const ru_field_rec XUMAC_RDP_MAC_MODE_MAC_SPEED_FIELD =
{
    "MAC_SPEED",
#if RU_INCLUDE_DESC
    "",
    "MAC Speed[2:0]. Bit 2 is in MAC_SPEED_BIT2.\n000: 10Mbps Ethernet Mode enabled\n001: 100Mbps Ethernet Mode enabled\n010: Gigabit Ethernet Mode enabled\n011: 2.5Gigabit Ethernet Mode enabled\n101: 5Gigabit Ethernet Mode enabled\n100: 10Gigabit Ethernet Mode enabled\n",
#endif
    { XUMAC_RDP_MAC_MODE_MAC_SPEED_FIELD_MASK },
    0,
    { XUMAC_RDP_MAC_MODE_MAC_SPEED_FIELD_WIDTH },
    { XUMAC_RDP_MAC_MODE_MAC_SPEED_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MAC_DUPLEX *****/
const ru_field_rec XUMAC_RDP_MAC_MODE_MAC_DUPLEX_FIELD =
{
    "MAC_DUPLEX",
#if RU_INCLUDE_DESC
    "",
    "MAC Duplex. \n0: Full Duplex Mode enabled\n1: Half Duplex Mode enabled\n",
#endif
    { XUMAC_RDP_MAC_MODE_MAC_DUPLEX_FIELD_MASK },
    0,
    { XUMAC_RDP_MAC_MODE_MAC_DUPLEX_FIELD_WIDTH },
    { XUMAC_RDP_MAC_MODE_MAC_DUPLEX_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MAC_RX_PAUSE *****/
const ru_field_rec XUMAC_RDP_MAC_MODE_MAC_RX_PAUSE_FIELD =
{
    "MAC_RX_PAUSE",
#if RU_INCLUDE_DESC
    "",
    "MAC Pause Enabled in Receive. \n0: MAC Pause Disabled in Receive\n1: MAC Pause Enabled in Receive\n",
#endif
    { XUMAC_RDP_MAC_MODE_MAC_RX_PAUSE_FIELD_MASK },
    0,
    { XUMAC_RDP_MAC_MODE_MAC_RX_PAUSE_FIELD_WIDTH },
    { XUMAC_RDP_MAC_MODE_MAC_RX_PAUSE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MAC_TX_PAUSE *****/
const ru_field_rec XUMAC_RDP_MAC_MODE_MAC_TX_PAUSE_FIELD =
{
    "MAC_TX_PAUSE",
#if RU_INCLUDE_DESC
    "",
    "MAC Pause Enabled in Transmit. \n0: MAC Pause Disabled in Transmit\n1: MAC Pause Enabled in Transmit\n",
#endif
    { XUMAC_RDP_MAC_MODE_MAC_TX_PAUSE_FIELD_MASK },
    0,
    { XUMAC_RDP_MAC_MODE_MAC_TX_PAUSE_FIELD_WIDTH },
    { XUMAC_RDP_MAC_MODE_MAC_TX_PAUSE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: LINK_STATUS *****/
const ru_field_rec XUMAC_RDP_MAC_MODE_LINK_STATUS_FIELD =
{
    "LINK_STATUS",
#if RU_INCLUDE_DESC
    "",
    "Link Status Indication. Set to '0', when link_status input is low.\nSet to '1', when link_status input is High.\n",
#endif
    { XUMAC_RDP_MAC_MODE_LINK_STATUS_FIELD_MASK },
    0,
    { XUMAC_RDP_MAC_MODE_LINK_STATUS_FIELD_WIDTH },
    { XUMAC_RDP_MAC_MODE_LINK_STATUS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MAC_SPEED_BIT2 *****/
const ru_field_rec XUMAC_RDP_MAC_MODE_MAC_SPEED_BIT2_FIELD =
{
    "MAC_SPEED_BIT2",
#if RU_INCLUDE_DESC
    "",
    "Bit 2 of MAC_SPEED[2:0]\n",
#endif
    { XUMAC_RDP_MAC_MODE_MAC_SPEED_BIT2_FIELD_MASK },
    0,
    { XUMAC_RDP_MAC_MODE_MAC_SPEED_BIT2_FIELD_WIDTH },
    { XUMAC_RDP_MAC_MODE_MAC_SPEED_BIT2_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_MAC_MODE_FIELDS[] =
{
    &XUMAC_RDP_MAC_MODE_MAC_SPEED_FIELD,
    &XUMAC_RDP_MAC_MODE_MAC_DUPLEX_FIELD,
    &XUMAC_RDP_MAC_MODE_MAC_RX_PAUSE_FIELD,
    &XUMAC_RDP_MAC_MODE_MAC_TX_PAUSE_FIELD,
    &XUMAC_RDP_MAC_MODE_LINK_STATUS_FIELD,
    &XUMAC_RDP_MAC_MODE_MAC_SPEED_BIT2_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_MAC_MODE *****/
const ru_reg_rec XUMAC_RDP_MAC_MODE_REG =
{
    "MAC_MODE",
#if RU_INCLUDE_DESC
    "MAC Mode Status. MAC Speed and Duplex Mode configuration from register COMMAND CONFIG, when ENA_EXT_CONFIG is set to '0' or from signals set_speed(1:0), set_duplex, tx_pause_en, rx_pause_en and link_stat.",
    "",
#endif
    { XUMAC_RDP_MAC_MODE_REG_OFFSET },
    0,
    0,
    1169,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    XUMAC_RDP_MAC_MODE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_TAG_0, TYPE: Type_TAG_0
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FRM_TAG_0 *****/
const ru_field_rec XUMAC_RDP_TAG_0_FRM_TAG_0_FIELD =
{
    "FRM_TAG_0",
#if RU_INCLUDE_DESC
    "",
    "Outer tag of the programmable VLAN tag\n",
#endif
    { XUMAC_RDP_TAG_0_FRM_TAG_0_FIELD_MASK },
    0,
    { XUMAC_RDP_TAG_0_FRM_TAG_0_FIELD_WIDTH },
    { XUMAC_RDP_TAG_0_FRM_TAG_0_FIELD_SHIFT },
    33024,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CONFIG_OUTER_TPID_ENABLE *****/
const ru_field_rec XUMAC_RDP_TAG_0_CONFIG_OUTER_TPID_ENABLE_FIELD =
{
    "CONFIG_OUTER_TPID_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "If cleared then disable outer TPID detection\n",
#endif
    { XUMAC_RDP_TAG_0_CONFIG_OUTER_TPID_ENABLE_FIELD_MASK },
    0,
    { XUMAC_RDP_TAG_0_CONFIG_OUTER_TPID_ENABLE_FIELD_WIDTH },
    { XUMAC_RDP_TAG_0_CONFIG_OUTER_TPID_ENABLE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_TAG_0_FIELDS[] =
{
    &XUMAC_RDP_TAG_0_FRM_TAG_0_FIELD,
    &XUMAC_RDP_TAG_0_CONFIG_OUTER_TPID_ENABLE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_TAG_0 *****/
const ru_reg_rec XUMAC_RDP_TAG_0_REG =
{
    "TAG_0",
#if RU_INCLUDE_DESC
    "Programmable VLAN outer tag",
    "",
#endif
    { XUMAC_RDP_TAG_0_REG_OFFSET },
    0,
    0,
    1170,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XUMAC_RDP_TAG_0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_TAG_1, TYPE: Type_TAG_1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FRM_TAG_1 *****/
const ru_field_rec XUMAC_RDP_TAG_1_FRM_TAG_1_FIELD =
{
    "FRM_TAG_1",
#if RU_INCLUDE_DESC
    "",
    "inner tag of the programmable VLAN tag\n",
#endif
    { XUMAC_RDP_TAG_1_FRM_TAG_1_FIELD_MASK },
    0,
    { XUMAC_RDP_TAG_1_FRM_TAG_1_FIELD_WIDTH },
    { XUMAC_RDP_TAG_1_FRM_TAG_1_FIELD_SHIFT },
    33024,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: CONFIG_INNER_TPID_ENABLE *****/
const ru_field_rec XUMAC_RDP_TAG_1_CONFIG_INNER_TPID_ENABLE_FIELD =
{
    "CONFIG_INNER_TPID_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "If cleared then disable inner TPID detection\n",
#endif
    { XUMAC_RDP_TAG_1_CONFIG_INNER_TPID_ENABLE_FIELD_MASK },
    0,
    { XUMAC_RDP_TAG_1_CONFIG_INNER_TPID_ENABLE_FIELD_WIDTH },
    { XUMAC_RDP_TAG_1_CONFIG_INNER_TPID_ENABLE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_TAG_1_FIELDS[] =
{
    &XUMAC_RDP_TAG_1_FRM_TAG_1_FIELD,
    &XUMAC_RDP_TAG_1_CONFIG_INNER_TPID_ENABLE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_TAG_1 *****/
const ru_reg_rec XUMAC_RDP_TAG_1_REG =
{
    "TAG_1",
#if RU_INCLUDE_DESC
    "Programmable VLAN inner tag",
    "",
#endif
    { XUMAC_RDP_TAG_1_REG_OFFSET },
    0,
    0,
    1171,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XUMAC_RDP_TAG_1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_RX_PAUSE_QUANTA_SCALE, TYPE: Type_RX_PAUSE_QUANTA_SCALE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SCALE_VALUE *****/
const ru_field_rec XUMAC_RDP_RX_PAUSE_QUANTA_SCALE_SCALE_VALUE_FIELD =
{
    "SCALE_VALUE",
#if RU_INCLUDE_DESC
    "",
    "The pause timer is loaded with the value obtained after adding or subtracting the scale_value from the received pause quanta.\n",
#endif
    { XUMAC_RDP_RX_PAUSE_QUANTA_SCALE_SCALE_VALUE_FIELD_MASK },
    0,
    { XUMAC_RDP_RX_PAUSE_QUANTA_SCALE_SCALE_VALUE_FIELD_WIDTH },
    { XUMAC_RDP_RX_PAUSE_QUANTA_SCALE_SCALE_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SCALE_CONTROL *****/
const ru_field_rec XUMAC_RDP_RX_PAUSE_QUANTA_SCALE_SCALE_CONTROL_FIELD =
{
    "SCALE_CONTROL",
#if RU_INCLUDE_DESC
    "",
    "If clear, then subtract the scale_value from the received pause quanta. \nIf set, then add the scale_value from the received pause quanta.\n",
#endif
    { XUMAC_RDP_RX_PAUSE_QUANTA_SCALE_SCALE_CONTROL_FIELD_MASK },
    0,
    { XUMAC_RDP_RX_PAUSE_QUANTA_SCALE_SCALE_CONTROL_FIELD_WIDTH },
    { XUMAC_RDP_RX_PAUSE_QUANTA_SCALE_SCALE_CONTROL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SCALE_FIX *****/
const ru_field_rec XUMAC_RDP_RX_PAUSE_QUANTA_SCALE_SCALE_FIX_FIELD =
{
    "SCALE_FIX",
#if RU_INCLUDE_DESC
    "",
    "If set, then receive pause quanta is ignored and a fixed quanta value programmed in SCALE_VALUE is loaded into the pause timer.\nIf set, then SCALE_CONTROL is ignored.\nIf cleared, then SCALE_CONTROL takes into effect.\n",
#endif
    { XUMAC_RDP_RX_PAUSE_QUANTA_SCALE_SCALE_FIX_FIELD_MASK },
    0,
    { XUMAC_RDP_RX_PAUSE_QUANTA_SCALE_SCALE_FIX_FIELD_WIDTH },
    { XUMAC_RDP_RX_PAUSE_QUANTA_SCALE_SCALE_FIX_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_RX_PAUSE_QUANTA_SCALE_FIELDS[] =
{
    &XUMAC_RDP_RX_PAUSE_QUANTA_SCALE_SCALE_VALUE_FIELD,
    &XUMAC_RDP_RX_PAUSE_QUANTA_SCALE_SCALE_CONTROL_FIELD,
    &XUMAC_RDP_RX_PAUSE_QUANTA_SCALE_SCALE_FIX_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_RX_PAUSE_QUANTA_SCALE *****/
const ru_reg_rec XUMAC_RDP_RX_PAUSE_QUANTA_SCALE_REG =
{
    "RX_PAUSE_QUANTA_SCALE",
#if RU_INCLUDE_DESC
    "programmable Rx pause quanta scaler. Static register. Affects Xoff values only",
    "",
#endif
    { XUMAC_RDP_RX_PAUSE_QUANTA_SCALE_REG_OFFSET },
    0,
    0,
    1172,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    XUMAC_RDP_RX_PAUSE_QUANTA_SCALE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_TX_PREAMBLE, TYPE: Type_TX_PREAMBLE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TX_PREAMBLE *****/
const ru_field_rec XUMAC_RDP_TX_PREAMBLE_TX_PREAMBLE_FIELD =
{
    "TX_PREAMBLE",
#if RU_INCLUDE_DESC
    "",
    "Set the transmit preamble excluding SFD to be programmable from min of 2 bytes to the max allowable of 7 bytes, with granularity of 1 byte.\n",
#endif
    { XUMAC_RDP_TX_PREAMBLE_TX_PREAMBLE_FIELD_MASK },
    0,
    { XUMAC_RDP_TX_PREAMBLE_TX_PREAMBLE_FIELD_WIDTH },
    { XUMAC_RDP_TX_PREAMBLE_TX_PREAMBLE_FIELD_SHIFT },
    7,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_TX_PREAMBLE_FIELDS[] =
{
    &XUMAC_RDP_TX_PREAMBLE_TX_PREAMBLE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_TX_PREAMBLE *****/
const ru_reg_rec XUMAC_RDP_TX_PREAMBLE_REG =
{
    "TX_PREAMBLE",
#if RU_INCLUDE_DESC
    "Programmable Preamble at Tx.",
    "",
#endif
    { XUMAC_RDP_TX_PREAMBLE_REG_OFFSET },
    0,
    0,
    1173,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_TX_PREAMBLE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_TX_IPG_LENGTH, TYPE: Type_TX_IPG_LENGTH
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TX_IPG_LENGTH *****/
const ru_field_rec XUMAC_RDP_TX_IPG_LENGTH_TX_IPG_LENGTH_FIELD =
{
    "TX_IPG_LENGTH",
#if RU_INCLUDE_DESC
    "",
    "Set the Transmit minimum IPG from 8 to 64 Byte-times. If a value below 8 or above 64 is\nprogrammed, the minimum IPG is set to 12 byte-times.\n",
#endif
    { XUMAC_RDP_TX_IPG_LENGTH_TX_IPG_LENGTH_FIELD_MASK },
    0,
    { XUMAC_RDP_TX_IPG_LENGTH_TX_IPG_LENGTH_FIELD_WIDTH },
    { XUMAC_RDP_TX_IPG_LENGTH_TX_IPG_LENGTH_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TX_MIN_PKT_SIZE *****/
const ru_field_rec XUMAC_RDP_TX_IPG_LENGTH_TX_MIN_PKT_SIZE_FIELD =
{
    "TX_MIN_PKT_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Min. TX packet size without FCS, also without preamble+SFD.\nPadding will be appended if needed to ensure this size.\nValid values are: 14..125\n",
#endif
    { XUMAC_RDP_TX_IPG_LENGTH_TX_MIN_PKT_SIZE_FIELD_MASK },
    0,
    { XUMAC_RDP_TX_IPG_LENGTH_TX_MIN_PKT_SIZE_FIELD_WIDTH },
    { XUMAC_RDP_TX_IPG_LENGTH_TX_MIN_PKT_SIZE_FIELD_SHIFT },
    60,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_TX_IPG_LENGTH_FIELDS[] =
{
    &XUMAC_RDP_TX_IPG_LENGTH_TX_IPG_LENGTH_FIELD,
    &XUMAC_RDP_TX_IPG_LENGTH_TX_MIN_PKT_SIZE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_TX_IPG_LENGTH *****/
const ru_reg_rec XUMAC_RDP_TX_IPG_LENGTH_REG =
{
    "TX_IPG_LENGTH",
#if RU_INCLUDE_DESC
    "Programmable Inter-Packet-Gap (IPG).",
    "",
#endif
    { XUMAC_RDP_TX_IPG_LENGTH_REG_OFFSET },
    0,
    0,
    1174,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XUMAC_RDP_TX_IPG_LENGTH_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_PFC_XOFF_TIMER, TYPE: Type_PFC_XOFF_TIMER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PFC_XOFF_TIMER *****/
const ru_field_rec XUMAC_RDP_PFC_XOFF_TIMER_PFC_XOFF_TIMER_FIELD =
{
    "PFC_XOFF_TIMER",
#if RU_INCLUDE_DESC
    "",
    "Time value sent in the Timer Field for classes in XOFF state (Unit is 512 bit-times).\n",
#endif
    { XUMAC_RDP_PFC_XOFF_TIMER_PFC_XOFF_TIMER_FIELD_MASK },
    0,
    { XUMAC_RDP_PFC_XOFF_TIMER_PFC_XOFF_TIMER_FIELD_WIDTH },
    { XUMAC_RDP_PFC_XOFF_TIMER_PFC_XOFF_TIMER_FIELD_SHIFT },
    65535,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_PFC_XOFF_TIMER_FIELDS[] =
{
    &XUMAC_RDP_PFC_XOFF_TIMER_PFC_XOFF_TIMER_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_PFC_XOFF_TIMER *****/
const ru_reg_rec XUMAC_RDP_PFC_XOFF_TIMER_REG =
{
    "PFC_XOFF_TIMER",
#if RU_INCLUDE_DESC
    "XOFF Timer value for PFC Tx packet",
    "",
#endif
    { XUMAC_RDP_PFC_XOFF_TIMER_REG_OFFSET },
    0,
    0,
    1175,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_PFC_XOFF_TIMER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_UMAC_EEE_CTRL, TYPE: Type_UMAC_EEE_CTRL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: EEE_EN *****/
const ru_field_rec XUMAC_RDP_UMAC_EEE_CTRL_EEE_EN_FIELD =
{
    "EEE_EN",
#if RU_INCLUDE_DESC
    "",
    "If set, the TX LPI policy control engine is enabled and the MAC inserts LPI_idle codes if the link is idle. The rx_lpi_detect assertion is independent of this configuration. Reset default depends on EEE_en_strap input, which if tied to 1, defaults to enabled, otherwise if tied to 0, defaults to disabled.\n",
#endif
    { XUMAC_RDP_UMAC_EEE_CTRL_EEE_EN_FIELD_MASK },
    0,
    { XUMAC_RDP_UMAC_EEE_CTRL_EEE_EN_FIELD_WIDTH },
    { XUMAC_RDP_UMAC_EEE_CTRL_EEE_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RX_FIFO_CHECK *****/
const ru_field_rec XUMAC_RDP_UMAC_EEE_CTRL_RX_FIFO_CHECK_FIELD =
{
    "RX_FIFO_CHECK",
#if RU_INCLUDE_DESC
    "",
    "If enabled, lpi_rx_detect is set whenever the LPI_IDLES are being received on the RX line and Unimac Rx FIFO is empty.\nBy default, lpi_rx_detect is set only when whenever the LPI_IDLES are being received on the RX line.\n",
#endif
    { XUMAC_RDP_UMAC_EEE_CTRL_RX_FIFO_CHECK_FIELD_MASK },
    0,
    { XUMAC_RDP_UMAC_EEE_CTRL_RX_FIFO_CHECK_FIELD_WIDTH },
    { XUMAC_RDP_UMAC_EEE_CTRL_RX_FIFO_CHECK_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EEE_TXCLK_DIS *****/
const ru_field_rec XUMAC_RDP_UMAC_EEE_CTRL_EEE_TXCLK_DIS_FIELD =
{
    "EEE_TXCLK_DIS",
#if RU_INCLUDE_DESC
    "",
    "If enabled, UNIMAC will shut down TXCLK to PHY, when in LPI state.\n",
#endif
    { XUMAC_RDP_UMAC_EEE_CTRL_EEE_TXCLK_DIS_FIELD_MASK },
    0,
    { XUMAC_RDP_UMAC_EEE_CTRL_EEE_TXCLK_DIS_FIELD_WIDTH },
    { XUMAC_RDP_UMAC_EEE_CTRL_EEE_TXCLK_DIS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DIS_EEE_10M *****/
const ru_field_rec XUMAC_RDP_UMAC_EEE_CTRL_DIS_EEE_10M_FIELD =
{
    "DIS_EEE_10M",
#if RU_INCLUDE_DESC
    "",
    "When this bit is set and link is established at 10Mbps, LPI is not supported (saving is achieved by reduced PHY's output swing). UNIMAC ignores EEE feature on both Tx & Rx in 10Mbps.\nWhen cleared, Unimac doesn't differentiate between speeds for EEE feature.\n",
#endif
    { XUMAC_RDP_UMAC_EEE_CTRL_DIS_EEE_10M_FIELD_MASK },
    0,
    { XUMAC_RDP_UMAC_EEE_CTRL_DIS_EEE_10M_FIELD_WIDTH },
    { XUMAC_RDP_UMAC_EEE_CTRL_DIS_EEE_10M_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: LP_IDLE_PREDICTION_MODE *****/
const ru_field_rec XUMAC_RDP_UMAC_EEE_CTRL_LP_IDLE_PREDICTION_MODE_FIELD =
{
    "LP_IDLE_PREDICTION_MODE",
#if RU_INCLUDE_DESC
    "",
    "When set to 1, enables LP_IDLE Prediction. When set to 0, disables LP_IDLE Prediction.\nIt is an experimental feature and not recommended to use for the production SW.\n",
#endif
    { XUMAC_RDP_UMAC_EEE_CTRL_LP_IDLE_PREDICTION_MODE_FIELD_MASK },
    0,
    { XUMAC_RDP_UMAC_EEE_CTRL_LP_IDLE_PREDICTION_MODE_FIELD_WIDTH },
    { XUMAC_RDP_UMAC_EEE_CTRL_LP_IDLE_PREDICTION_MODE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_UMAC_EEE_CTRL_FIELDS[] =
{
    &XUMAC_RDP_UMAC_EEE_CTRL_EEE_EN_FIELD,
    &XUMAC_RDP_UMAC_EEE_CTRL_RX_FIFO_CHECK_FIELD,
    &XUMAC_RDP_UMAC_EEE_CTRL_EEE_TXCLK_DIS_FIELD,
    &XUMAC_RDP_UMAC_EEE_CTRL_DIS_EEE_10M_FIELD,
    &XUMAC_RDP_UMAC_EEE_CTRL_LP_IDLE_PREDICTION_MODE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_UMAC_EEE_CTRL *****/
const ru_reg_rec XUMAC_RDP_UMAC_EEE_CTRL_REG =
{
    "UMAC_EEE_CTRL",
#if RU_INCLUDE_DESC
    "control configs for EEE feature",
    "",
#endif
    { XUMAC_RDP_UMAC_EEE_CTRL_REG_OFFSET },
    0,
    0,
    1176,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    XUMAC_RDP_UMAC_EEE_CTRL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_MII_EEE_DELAY_ENTRY_TIMER, TYPE: Type_MII_EEE_DELAY_ENTRY_TIMER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MII_EEE_LPI_TIMER *****/
const ru_field_rec XUMAC_RDP_MII_EEE_DELAY_ENTRY_TIMER_MII_EEE_LPI_TIMER_FIELD =
{
    "MII_EEE_LPI_TIMER",
#if RU_INCLUDE_DESC
    "",
    "This is the duration for which condition to move to LPI state must be satisfied, at the end of which MAC transitions to LPI State. The decrement unit is 1 micro-second.\nThis register is meant for 10/100 Mbps speed.\n",
#endif
    { XUMAC_RDP_MII_EEE_DELAY_ENTRY_TIMER_MII_EEE_LPI_TIMER_FIELD_MASK },
    0,
    { XUMAC_RDP_MII_EEE_DELAY_ENTRY_TIMER_MII_EEE_LPI_TIMER_FIELD_WIDTH },
    { XUMAC_RDP_MII_EEE_DELAY_ENTRY_TIMER_MII_EEE_LPI_TIMER_FIELD_SHIFT },
    60,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_MII_EEE_DELAY_ENTRY_TIMER_FIELDS[] =
{
    &XUMAC_RDP_MII_EEE_DELAY_ENTRY_TIMER_MII_EEE_LPI_TIMER_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_MII_EEE_DELAY_ENTRY_TIMER *****/
const ru_reg_rec XUMAC_RDP_MII_EEE_DELAY_ENTRY_TIMER_REG =
{
    "MII_EEE_DELAY_ENTRY_TIMER",
#if RU_INCLUDE_DESC
    "MII_EEE LPI timer",
    "",
#endif
    { XUMAC_RDP_MII_EEE_DELAY_ENTRY_TIMER_REG_OFFSET },
    0,
    0,
    1177,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_MII_EEE_DELAY_ENTRY_TIMER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GMII_EEE_DELAY_ENTRY_TIMER, TYPE: Type_GMII_EEE_DELAY_ENTRY_TIMER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: GMII_EEE_LPI_TIMER *****/
const ru_field_rec XUMAC_RDP_GMII_EEE_DELAY_ENTRY_TIMER_GMII_EEE_LPI_TIMER_FIELD =
{
    "GMII_EEE_LPI_TIMER",
#if RU_INCLUDE_DESC
    "",
    "This is the duration for which condition to move to LPI state must be satisfied, at the end of which MAC transitions to LPI State. The decrement unit is 1 micro-second.\nThis register is meant for 1000 Mbps speed.\n",
#endif
    { XUMAC_RDP_GMII_EEE_DELAY_ENTRY_TIMER_GMII_EEE_LPI_TIMER_FIELD_MASK },
    0,
    { XUMAC_RDP_GMII_EEE_DELAY_ENTRY_TIMER_GMII_EEE_LPI_TIMER_FIELD_WIDTH },
    { XUMAC_RDP_GMII_EEE_DELAY_ENTRY_TIMER_GMII_EEE_LPI_TIMER_FIELD_SHIFT },
    34,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GMII_EEE_DELAY_ENTRY_TIMER_FIELDS[] =
{
    &XUMAC_RDP_GMII_EEE_DELAY_ENTRY_TIMER_GMII_EEE_LPI_TIMER_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GMII_EEE_DELAY_ENTRY_TIMER *****/
const ru_reg_rec XUMAC_RDP_GMII_EEE_DELAY_ENTRY_TIMER_REG =
{
    "GMII_EEE_DELAY_ENTRY_TIMER",
#if RU_INCLUDE_DESC
    "GMII_EEE LPI timer",
    "",
#endif
    { XUMAC_RDP_GMII_EEE_DELAY_ENTRY_TIMER_REG_OFFSET },
    0,
    0,
    1178,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GMII_EEE_DELAY_ENTRY_TIMER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_UMAC_EEE_REF_COUNT, TYPE: Type_UMAC_EEE_REF_COUNT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: EEE_REF_COUNT *****/
const ru_field_rec XUMAC_RDP_UMAC_EEE_REF_COUNT_EEE_REF_COUNT_FIELD =
{
    "EEE_REF_COUNT",
#if RU_INCLUDE_DESC
    "",
    "This field controls clock divider used to generate ~1us reference pulses used by EEE timers. It specifies integer number of timer clock cycles contained within 1us.\n",
#endif
    { XUMAC_RDP_UMAC_EEE_REF_COUNT_EEE_REF_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_UMAC_EEE_REF_COUNT_EEE_REF_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_UMAC_EEE_REF_COUNT_EEE_REF_COUNT_FIELD_SHIFT },
    125,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_UMAC_EEE_REF_COUNT_FIELDS[] =
{
    &XUMAC_RDP_UMAC_EEE_REF_COUNT_EEE_REF_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_UMAC_EEE_REF_COUNT *****/
const ru_reg_rec XUMAC_RDP_UMAC_EEE_REF_COUNT_REG =
{
    "UMAC_EEE_REF_COUNT",
#if RU_INCLUDE_DESC
    "clock divider for 1 us quanta count in EEE",
    "",
#endif
    { XUMAC_RDP_UMAC_EEE_REF_COUNT_REG_OFFSET },
    0,
    0,
    1179,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_UMAC_EEE_REF_COUNT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_UMAC_TIMESTAMP_ADJUST, TYPE: Type_UMAC_TIMESTAMP_ADJUST
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ADJUST *****/
const ru_field_rec XUMAC_RDP_UMAC_TIMESTAMP_ADJUST_ADJUST_FIELD =
{
    "ADJUST",
#if RU_INCLUDE_DESC
    "",
    "Offset adjustment to outgoing TIMESTAMP to adjust for pipeline stalling and/or jitter asymmetry. The value is in 2's compliment format and is of 1ns granularity.\n",
#endif
    { XUMAC_RDP_UMAC_TIMESTAMP_ADJUST_ADJUST_FIELD_MASK },
    0,
    { XUMAC_RDP_UMAC_TIMESTAMP_ADJUST_ADJUST_FIELD_WIDTH },
    { XUMAC_RDP_UMAC_TIMESTAMP_ADJUST_ADJUST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EN_1588 *****/
const ru_field_rec XUMAC_RDP_UMAC_TIMESTAMP_ADJUST_EN_1588_FIELD =
{
    "EN_1588",
#if RU_INCLUDE_DESC
    "",
    "Enables 1588 one step timestamp feature.\n",
#endif
    { XUMAC_RDP_UMAC_TIMESTAMP_ADJUST_EN_1588_FIELD_MASK },
    0,
    { XUMAC_RDP_UMAC_TIMESTAMP_ADJUST_EN_1588_FIELD_WIDTH },
    { XUMAC_RDP_UMAC_TIMESTAMP_ADJUST_EN_1588_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: AUTO_ADJUST *****/
const ru_field_rec XUMAC_RDP_UMAC_TIMESTAMP_ADJUST_AUTO_ADJUST_FIELD =
{
    "AUTO_ADJUST",
#if RU_INCLUDE_DESC
    "",
    "Enables MAC Rx timestamp offset balancing at MAC TX timestamp.\n",
#endif
    { XUMAC_RDP_UMAC_TIMESTAMP_ADJUST_AUTO_ADJUST_FIELD_MASK },
    0,
    { XUMAC_RDP_UMAC_TIMESTAMP_ADJUST_AUTO_ADJUST_FIELD_WIDTH },
    { XUMAC_RDP_UMAC_TIMESTAMP_ADJUST_AUTO_ADJUST_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_UMAC_TIMESTAMP_ADJUST_FIELDS[] =
{
    &XUMAC_RDP_UMAC_TIMESTAMP_ADJUST_ADJUST_FIELD,
    &XUMAC_RDP_UMAC_TIMESTAMP_ADJUST_EN_1588_FIELD,
    &XUMAC_RDP_UMAC_TIMESTAMP_ADJUST_AUTO_ADJUST_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_UMAC_TIMESTAMP_ADJUST *****/
const ru_reg_rec XUMAC_RDP_UMAC_TIMESTAMP_ADJUST_REG =
{
    "UMAC_TIMESTAMP_ADJUST",
#if RU_INCLUDE_DESC
    "1588_one_step_timestamp control",
    "",
#endif
    { XUMAC_RDP_UMAC_TIMESTAMP_ADJUST_REG_OFFSET },
    0,
    0,
    1180,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    XUMAC_RDP_UMAC_TIMESTAMP_ADJUST_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_UMAC_RX_PKT_DROP_STATUS, TYPE: Type_UMAC_RX_PKT_DROP_STATUS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RX_IPG_INVAL *****/
const ru_field_rec XUMAC_RDP_UMAC_RX_PKT_DROP_STATUS_RX_IPG_INVAL_FIELD =
{
    "RX_IPG_INVAL",
#if RU_INCLUDE_DESC
    "",
    "Debug status, set if MAC receives an IPG less than programmed RX IPG or less than four bytes. Sticky bit. Clears when SW writes 0 into the field or by sw_reset.\n",
#endif
    { XUMAC_RDP_UMAC_RX_PKT_DROP_STATUS_RX_IPG_INVAL_FIELD_MASK },
    0,
    { XUMAC_RDP_UMAC_RX_PKT_DROP_STATUS_RX_IPG_INVAL_FIELD_WIDTH },
    { XUMAC_RDP_UMAC_RX_PKT_DROP_STATUS_RX_IPG_INVAL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_UMAC_RX_PKT_DROP_STATUS_FIELDS[] =
{
    &XUMAC_RDP_UMAC_RX_PKT_DROP_STATUS_RX_IPG_INVAL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_UMAC_RX_PKT_DROP_STATUS *****/
const ru_reg_rec XUMAC_RDP_UMAC_RX_PKT_DROP_STATUS_REG =
{
    "UMAC_RX_PKT_DROP_STATUS",
#if RU_INCLUDE_DESC
    "sticky status for Rx packet drop due to invalid IPG",
    "",
#endif
    { XUMAC_RDP_UMAC_RX_PKT_DROP_STATUS_REG_OFFSET },
    0,
    0,
    1181,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_UMAC_RX_PKT_DROP_STATUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_UMAC_SYMMETRIC_IDLE_THRESHOLD, TYPE: Type_UMAC_SYMMETRIC_IDLE_THRESHOLD
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: THRESHOLD_VALUE *****/
const ru_field_rec XUMAC_RDP_UMAC_SYMMETRIC_IDLE_THRESHOLD_THRESHOLD_VALUE_FIELD =
{
    "THRESHOLD_VALUE",
#if RU_INCLUDE_DESC
    "",
    "If LPI_Prediction is enabled then this register defines the number of IDLEs to be received by the UniMAC before allowing LP_IDLE to be sent to Link Partner.\nIt is an experimental feature and not recommended to use for the production SW.\n",
#endif
    { XUMAC_RDP_UMAC_SYMMETRIC_IDLE_THRESHOLD_THRESHOLD_VALUE_FIELD_MASK },
    0,
    { XUMAC_RDP_UMAC_SYMMETRIC_IDLE_THRESHOLD_THRESHOLD_VALUE_FIELD_WIDTH },
    { XUMAC_RDP_UMAC_SYMMETRIC_IDLE_THRESHOLD_THRESHOLD_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_UMAC_SYMMETRIC_IDLE_THRESHOLD_FIELDS[] =
{
    &XUMAC_RDP_UMAC_SYMMETRIC_IDLE_THRESHOLD_THRESHOLD_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_UMAC_SYMMETRIC_IDLE_THRESHOLD *****/
const ru_reg_rec XUMAC_RDP_UMAC_SYMMETRIC_IDLE_THRESHOLD_REG =
{
    "UMAC_SYMMETRIC_IDLE_THRESHOLD",
#if RU_INCLUDE_DESC
    "RX IDLE threshold for LPI prediction",
    "",
#endif
    { XUMAC_RDP_UMAC_SYMMETRIC_IDLE_THRESHOLD_REG_OFFSET },
    0,
    0,
    1182,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_UMAC_SYMMETRIC_IDLE_THRESHOLD_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_MII_EEE_WAKE_TIMER, TYPE: Type_MII_EEE_WAKE_TIMER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MII_EEE_WAKE_TIMER *****/
const ru_field_rec XUMAC_RDP_MII_EEE_WAKE_TIMER_MII_EEE_WAKE_TIMER_FIELD =
{
    "MII_EEE_WAKE_TIMER",
#if RU_INCLUDE_DESC
    "",
    "This is the duration for which MAC must wait to go back to ACTIVE state from LPI state when it receives packet for transmission. The decrement unit is 1 micro-second.\nThis register is meant for 100 Mbps speed.\n",
#endif
    { XUMAC_RDP_MII_EEE_WAKE_TIMER_MII_EEE_WAKE_TIMER_FIELD_MASK },
    0,
    { XUMAC_RDP_MII_EEE_WAKE_TIMER_MII_EEE_WAKE_TIMER_FIELD_WIDTH },
    { XUMAC_RDP_MII_EEE_WAKE_TIMER_MII_EEE_WAKE_TIMER_FIELD_SHIFT },
    30,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_MII_EEE_WAKE_TIMER_FIELDS[] =
{
    &XUMAC_RDP_MII_EEE_WAKE_TIMER_MII_EEE_WAKE_TIMER_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_MII_EEE_WAKE_TIMER *****/
const ru_reg_rec XUMAC_RDP_MII_EEE_WAKE_TIMER_REG =
{
    "MII_EEE_WAKE_TIMER",
#if RU_INCLUDE_DESC
    "MII_EEE Wake timer",
    "",
#endif
    { XUMAC_RDP_MII_EEE_WAKE_TIMER_REG_OFFSET },
    0,
    0,
    1183,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_MII_EEE_WAKE_TIMER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GMII_EEE_WAKE_TIMER, TYPE: Type_GMII_EEE_WAKE_TIMER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: GMII_EEE_WAKE_TIMER *****/
const ru_field_rec XUMAC_RDP_GMII_EEE_WAKE_TIMER_GMII_EEE_WAKE_TIMER_FIELD =
{
    "GMII_EEE_WAKE_TIMER",
#if RU_INCLUDE_DESC
    "",
    "This is the duration for which MAC must wait to go back to ACTIVE state from LPI state when it receives packet for transmission. The decrement unit is 1 micro-second.\nThis register is meant for 1000 Mbps speed.\n",
#endif
    { XUMAC_RDP_GMII_EEE_WAKE_TIMER_GMII_EEE_WAKE_TIMER_FIELD_MASK },
    0,
    { XUMAC_RDP_GMII_EEE_WAKE_TIMER_GMII_EEE_WAKE_TIMER_FIELD_WIDTH },
    { XUMAC_RDP_GMII_EEE_WAKE_TIMER_GMII_EEE_WAKE_TIMER_FIELD_SHIFT },
    17,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GMII_EEE_WAKE_TIMER_FIELDS[] =
{
    &XUMAC_RDP_GMII_EEE_WAKE_TIMER_GMII_EEE_WAKE_TIMER_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GMII_EEE_WAKE_TIMER *****/
const ru_reg_rec XUMAC_RDP_GMII_EEE_WAKE_TIMER_REG =
{
    "GMII_EEE_WAKE_TIMER",
#if RU_INCLUDE_DESC
    "GMII_EEE Wake timer",
    "",
#endif
    { XUMAC_RDP_GMII_EEE_WAKE_TIMER_REG_OFFSET },
    0,
    0,
    1184,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GMII_EEE_WAKE_TIMER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_UMAC_REV_ID, TYPE: Type_UMAC_REV_ID
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PATCH *****/
const ru_field_rec XUMAC_RDP_UMAC_REV_ID_PATCH_FIELD =
{
    "PATCH",
#if RU_INCLUDE_DESC
    "",
    "Unimac revision patch number.\n",
#endif
    { XUMAC_RDP_UMAC_REV_ID_PATCH_FIELD_MASK },
    0,
    { XUMAC_RDP_UMAC_REV_ID_PATCH_FIELD_WIDTH },
    { XUMAC_RDP_UMAC_REV_ID_PATCH_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: REVISION_ID_MINOR *****/
const ru_field_rec XUMAC_RDP_UMAC_REV_ID_REVISION_ID_MINOR_FIELD =
{
    "REVISION_ID_MINOR",
#if RU_INCLUDE_DESC
    "",
    "Unimac version id field after decimal.\n",
#endif
    { XUMAC_RDP_UMAC_REV_ID_REVISION_ID_MINOR_FIELD_MASK },
    0,
    { XUMAC_RDP_UMAC_REV_ID_REVISION_ID_MINOR_FIELD_WIDTH },
    { XUMAC_RDP_UMAC_REV_ID_REVISION_ID_MINOR_FIELD_SHIFT },
    6,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: REVISION_ID_MAJOR *****/
const ru_field_rec XUMAC_RDP_UMAC_REV_ID_REVISION_ID_MAJOR_FIELD =
{
    "REVISION_ID_MAJOR",
#if RU_INCLUDE_DESC
    "",
    "Unimac version id field before decimal.\n",
#endif
    { XUMAC_RDP_UMAC_REV_ID_REVISION_ID_MAJOR_FIELD_MASK },
    0,
    { XUMAC_RDP_UMAC_REV_ID_REVISION_ID_MAJOR_FIELD_WIDTH },
    { XUMAC_RDP_UMAC_REV_ID_REVISION_ID_MAJOR_FIELD_SHIFT },
    3,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_UMAC_REV_ID_FIELDS[] =
{
    &XUMAC_RDP_UMAC_REV_ID_PATCH_FIELD,
    &XUMAC_RDP_UMAC_REV_ID_REVISION_ID_MINOR_FIELD,
    &XUMAC_RDP_UMAC_REV_ID_REVISION_ID_MAJOR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_UMAC_REV_ID *****/
const ru_reg_rec XUMAC_RDP_UMAC_REV_ID_REG =
{
    "UMAC_REV_ID",
#if RU_INCLUDE_DESC
    "UNIMAC_REV_ID",
    "",
#endif
    { XUMAC_RDP_UMAC_REV_ID_REG_OFFSET },
    0,
    0,
    1185,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    XUMAC_RDP_UMAC_REV_ID_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GMII_2P5G_EEE_DELAY_ENTRY_TIMER, TYPE: Type_GMII_2P5G_EEE_DELAY_ENTRY_TIMER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: GMII_2P5G_EEE_LPI_TIMER *****/
const ru_field_rec XUMAC_RDP_GMII_2P5G_EEE_DELAY_ENTRY_TIMER_GMII_2P5G_EEE_LPI_TIMER_FIELD =
{
    "GMII_2P5G_EEE_LPI_TIMER",
#if RU_INCLUDE_DESC
    "",
    "This is the duration for which condition to move to LPI state must be satisfied, at the end of which MAC transitions to LPI State. The decrement unit is 1 micro-second.\nThis register is meant for 2.5 Gbps speed.\n",
#endif
    { XUMAC_RDP_GMII_2P5G_EEE_DELAY_ENTRY_TIMER_GMII_2P5G_EEE_LPI_TIMER_FIELD_MASK },
    0,
    { XUMAC_RDP_GMII_2P5G_EEE_DELAY_ENTRY_TIMER_GMII_2P5G_EEE_LPI_TIMER_FIELD_WIDTH },
    { XUMAC_RDP_GMII_2P5G_EEE_DELAY_ENTRY_TIMER_GMII_2P5G_EEE_LPI_TIMER_FIELD_SHIFT },
    60,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GMII_2P5G_EEE_DELAY_ENTRY_TIMER_FIELDS[] =
{
    &XUMAC_RDP_GMII_2P5G_EEE_DELAY_ENTRY_TIMER_GMII_2P5G_EEE_LPI_TIMER_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GMII_2P5G_EEE_DELAY_ENTRY_TIMER *****/
const ru_reg_rec XUMAC_RDP_GMII_2P5G_EEE_DELAY_ENTRY_TIMER_REG =
{
    "GMII_2P5G_EEE_DELAY_ENTRY_TIMER",
#if RU_INCLUDE_DESC
    "2P5G EEE LPI timer",
    "",
#endif
    { XUMAC_RDP_GMII_2P5G_EEE_DELAY_ENTRY_TIMER_REG_OFFSET },
    0,
    0,
    1186,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GMII_2P5G_EEE_DELAY_ENTRY_TIMER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GMII_5G_EEE_DELAY_ENTRY_TIMER, TYPE: Type_GMII_5G_EEE_DELAY_ENTRY_TIMER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: GMII_5G_EEE_LPI_TIMER *****/
const ru_field_rec XUMAC_RDP_GMII_5G_EEE_DELAY_ENTRY_TIMER_GMII_5G_EEE_LPI_TIMER_FIELD =
{
    "GMII_5G_EEE_LPI_TIMER",
#if RU_INCLUDE_DESC
    "",
    "This is the duration for which condition to move to LPI state must be satisfied, at the end of which MAC transitions to LPI State. The decrement unit is 1 micro-second.\nThis register is meant for 5 Gbps speed.\n",
#endif
    { XUMAC_RDP_GMII_5G_EEE_DELAY_ENTRY_TIMER_GMII_5G_EEE_LPI_TIMER_FIELD_MASK },
    0,
    { XUMAC_RDP_GMII_5G_EEE_DELAY_ENTRY_TIMER_GMII_5G_EEE_LPI_TIMER_FIELD_WIDTH },
    { XUMAC_RDP_GMII_5G_EEE_DELAY_ENTRY_TIMER_GMII_5G_EEE_LPI_TIMER_FIELD_SHIFT },
    30,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GMII_5G_EEE_DELAY_ENTRY_TIMER_FIELDS[] =
{
    &XUMAC_RDP_GMII_5G_EEE_DELAY_ENTRY_TIMER_GMII_5G_EEE_LPI_TIMER_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GMII_5G_EEE_DELAY_ENTRY_TIMER *****/
const ru_reg_rec XUMAC_RDP_GMII_5G_EEE_DELAY_ENTRY_TIMER_REG =
{
    "GMII_5G_EEE_DELAY_ENTRY_TIMER",
#if RU_INCLUDE_DESC
    "5G EEE LPI timer",
    "",
#endif
    { XUMAC_RDP_GMII_5G_EEE_DELAY_ENTRY_TIMER_REG_OFFSET },
    0,
    0,
    1187,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GMII_5G_EEE_DELAY_ENTRY_TIMER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GMII_10G_EEE_DELAY_ENTRY_TIMER, TYPE: Type_GMII_10G_EEE_DELAY_ENTRY_TIMER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: GMII_10G_EEE_LPI_TIMER *****/
const ru_field_rec XUMAC_RDP_GMII_10G_EEE_DELAY_ENTRY_TIMER_GMII_10G_EEE_LPI_TIMER_FIELD =
{
    "GMII_10G_EEE_LPI_TIMER",
#if RU_INCLUDE_DESC
    "",
    "This is the duration for which condition to move to LPI state must be satisfied, at the end of which MAC transitions to LPI State. The decrement unit is 1 micro-second.\nThis register is meant for 10 Gbps speed.\n",
#endif
    { XUMAC_RDP_GMII_10G_EEE_DELAY_ENTRY_TIMER_GMII_10G_EEE_LPI_TIMER_FIELD_MASK },
    0,
    { XUMAC_RDP_GMII_10G_EEE_DELAY_ENTRY_TIMER_GMII_10G_EEE_LPI_TIMER_FIELD_WIDTH },
    { XUMAC_RDP_GMII_10G_EEE_DELAY_ENTRY_TIMER_GMII_10G_EEE_LPI_TIMER_FIELD_SHIFT },
    16,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GMII_10G_EEE_DELAY_ENTRY_TIMER_FIELDS[] =
{
    &XUMAC_RDP_GMII_10G_EEE_DELAY_ENTRY_TIMER_GMII_10G_EEE_LPI_TIMER_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GMII_10G_EEE_DELAY_ENTRY_TIMER *****/
const ru_reg_rec XUMAC_RDP_GMII_10G_EEE_DELAY_ENTRY_TIMER_REG =
{
    "GMII_10G_EEE_DELAY_ENTRY_TIMER",
#if RU_INCLUDE_DESC
    "10G EEE LPI timer",
    "",
#endif
    { XUMAC_RDP_GMII_10G_EEE_DELAY_ENTRY_TIMER_REG_OFFSET },
    0,
    0,
    1188,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GMII_10G_EEE_DELAY_ENTRY_TIMER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GMII_2P5G_EEE_WAKE_TIMER, TYPE: Type_GMII_2P5G_EEE_WAKE_TIMER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: GMII_2P5G_EEE_WAKE_TIMER *****/
const ru_field_rec XUMAC_RDP_GMII_2P5G_EEE_WAKE_TIMER_GMII_2P5G_EEE_WAKE_TIMER_FIELD =
{
    "GMII_2P5G_EEE_WAKE_TIMER",
#if RU_INCLUDE_DESC
    "",
    "This is the duration for which MAC must wait to go back to ACTIVE state from LPI state when it receives packet for transmission. The decrement unit is 1 micro-second.\nThis register is meant for 2.5 Gbps speed.\n",
#endif
    { XUMAC_RDP_GMII_2P5G_EEE_WAKE_TIMER_GMII_2P5G_EEE_WAKE_TIMER_FIELD_MASK },
    0,
    { XUMAC_RDP_GMII_2P5G_EEE_WAKE_TIMER_GMII_2P5G_EEE_WAKE_TIMER_FIELD_WIDTH },
    { XUMAC_RDP_GMII_2P5G_EEE_WAKE_TIMER_GMII_2P5G_EEE_WAKE_TIMER_FIELD_SHIFT },
    30,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GMII_2P5G_EEE_WAKE_TIMER_FIELDS[] =
{
    &XUMAC_RDP_GMII_2P5G_EEE_WAKE_TIMER_GMII_2P5G_EEE_WAKE_TIMER_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GMII_2P5G_EEE_WAKE_TIMER *****/
const ru_reg_rec XUMAC_RDP_GMII_2P5G_EEE_WAKE_TIMER_REG =
{
    "GMII_2P5G_EEE_WAKE_TIMER",
#if RU_INCLUDE_DESC
    "2P5G EEE Wake timer",
    "",
#endif
    { XUMAC_RDP_GMII_2P5G_EEE_WAKE_TIMER_REG_OFFSET },
    0,
    0,
    1189,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GMII_2P5G_EEE_WAKE_TIMER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GMII_5G_EEE_WAKE_TIMER, TYPE: Type_GMII_5G_EEE_WAKE_TIMER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: GMII_5G_EEE_WAKE_TIMER *****/
const ru_field_rec XUMAC_RDP_GMII_5G_EEE_WAKE_TIMER_GMII_5G_EEE_WAKE_TIMER_FIELD =
{
    "GMII_5G_EEE_WAKE_TIMER",
#if RU_INCLUDE_DESC
    "",
    "This is the duration for which MAC must wait to go back to ACTIVE state from LPI state when it receives packet for transmission. The decrement unit is 1 micro-second.\nThis register is meant for 5 Gbps speed.\n",
#endif
    { XUMAC_RDP_GMII_5G_EEE_WAKE_TIMER_GMII_5G_EEE_WAKE_TIMER_FIELD_MASK },
    0,
    { XUMAC_RDP_GMII_5G_EEE_WAKE_TIMER_GMII_5G_EEE_WAKE_TIMER_FIELD_WIDTH },
    { XUMAC_RDP_GMII_5G_EEE_WAKE_TIMER_GMII_5G_EEE_WAKE_TIMER_FIELD_SHIFT },
    15,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GMII_5G_EEE_WAKE_TIMER_FIELDS[] =
{
    &XUMAC_RDP_GMII_5G_EEE_WAKE_TIMER_GMII_5G_EEE_WAKE_TIMER_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GMII_5G_EEE_WAKE_TIMER *****/
const ru_reg_rec XUMAC_RDP_GMII_5G_EEE_WAKE_TIMER_REG =
{
    "GMII_5G_EEE_WAKE_TIMER",
#if RU_INCLUDE_DESC
    "5G EEE Wake timer",
    "",
#endif
    { XUMAC_RDP_GMII_5G_EEE_WAKE_TIMER_REG_OFFSET },
    0,
    0,
    1190,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GMII_5G_EEE_WAKE_TIMER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GMII_10G_EEE_WAKE_TIMER, TYPE: Type_GMII_10G_EEE_WAKE_TIMER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: GMII_10G_EEE_WAKE_TIMER *****/
const ru_field_rec XUMAC_RDP_GMII_10G_EEE_WAKE_TIMER_GMII_10G_EEE_WAKE_TIMER_FIELD =
{
    "GMII_10G_EEE_WAKE_TIMER",
#if RU_INCLUDE_DESC
    "",
    "This is the duration for which MAC must wait to go back to ACTIVE state from LPI state when it receives packet for transmission. The decrement unit is 1 micro-second.\nThis register is meant for 10 Gbps speed.\n",
#endif
    { XUMAC_RDP_GMII_10G_EEE_WAKE_TIMER_GMII_10G_EEE_WAKE_TIMER_FIELD_MASK },
    0,
    { XUMAC_RDP_GMII_10G_EEE_WAKE_TIMER_GMII_10G_EEE_WAKE_TIMER_FIELD_WIDTH },
    { XUMAC_RDP_GMII_10G_EEE_WAKE_TIMER_GMII_10G_EEE_WAKE_TIMER_FIELD_SHIFT },
    8,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GMII_10G_EEE_WAKE_TIMER_FIELDS[] =
{
    &XUMAC_RDP_GMII_10G_EEE_WAKE_TIMER_GMII_10G_EEE_WAKE_TIMER_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GMII_10G_EEE_WAKE_TIMER *****/
const ru_reg_rec XUMAC_RDP_GMII_10G_EEE_WAKE_TIMER_REG =
{
    "GMII_10G_EEE_WAKE_TIMER",
#if RU_INCLUDE_DESC
    "10G EEE Wake timer",
    "",
#endif
    { XUMAC_RDP_GMII_10G_EEE_WAKE_TIMER_REG_OFFSET },
    0,
    0,
    1191,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GMII_10G_EEE_WAKE_TIMER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_ACTIVE_EEE_DELAY_ENTRY_TIMER, TYPE: Type_ACTIVE_EEE_DELAY_ENTRY_TIMER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ACTIVE_EEE_LPI_TIMER *****/
const ru_field_rec XUMAC_RDP_ACTIVE_EEE_DELAY_ENTRY_TIMER_ACTIVE_EEE_LPI_TIMER_FIELD =
{
    "ACTIVE_EEE_LPI_TIMER",
#if RU_INCLUDE_DESC
    "",
    "Currently selected EEE LPI timer.\n",
#endif
    { XUMAC_RDP_ACTIVE_EEE_DELAY_ENTRY_TIMER_ACTIVE_EEE_LPI_TIMER_FIELD_MASK },
    0,
    { XUMAC_RDP_ACTIVE_EEE_DELAY_ENTRY_TIMER_ACTIVE_EEE_LPI_TIMER_FIELD_WIDTH },
    { XUMAC_RDP_ACTIVE_EEE_DELAY_ENTRY_TIMER_ACTIVE_EEE_LPI_TIMER_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_ACTIVE_EEE_DELAY_ENTRY_TIMER_FIELDS[] =
{
    &XUMAC_RDP_ACTIVE_EEE_DELAY_ENTRY_TIMER_ACTIVE_EEE_LPI_TIMER_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_ACTIVE_EEE_DELAY_ENTRY_TIMER *****/
const ru_reg_rec XUMAC_RDP_ACTIVE_EEE_DELAY_ENTRY_TIMER_REG =
{
    "ACTIVE_EEE_DELAY_ENTRY_TIMER",
#if RU_INCLUDE_DESC
    "Active EEE LPI timer",
    "",
#endif
    { XUMAC_RDP_ACTIVE_EEE_DELAY_ENTRY_TIMER_REG_OFFSET },
    0,
    0,
    1192,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_ACTIVE_EEE_DELAY_ENTRY_TIMER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_ACTIVE_EEE_WAKE_TIMER, TYPE: Type_ACTIVE_EEE_WAKE_TIMER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ACTIVE_EEE_WAKE_TIME *****/
const ru_field_rec XUMAC_RDP_ACTIVE_EEE_WAKE_TIMER_ACTIVE_EEE_WAKE_TIME_FIELD =
{
    "ACTIVE_EEE_WAKE_TIME",
#if RU_INCLUDE_DESC
    "",
    "Currently selected wake timer.\n",
#endif
    { XUMAC_RDP_ACTIVE_EEE_WAKE_TIMER_ACTIVE_EEE_WAKE_TIME_FIELD_MASK },
    0,
    { XUMAC_RDP_ACTIVE_EEE_WAKE_TIMER_ACTIVE_EEE_WAKE_TIME_FIELD_WIDTH },
    { XUMAC_RDP_ACTIVE_EEE_WAKE_TIMER_ACTIVE_EEE_WAKE_TIME_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_ACTIVE_EEE_WAKE_TIMER_FIELDS[] =
{
    &XUMAC_RDP_ACTIVE_EEE_WAKE_TIMER_ACTIVE_EEE_WAKE_TIME_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_ACTIVE_EEE_WAKE_TIMER *****/
const ru_reg_rec XUMAC_RDP_ACTIVE_EEE_WAKE_TIMER_REG =
{
    "ACTIVE_EEE_WAKE_TIMER",
#if RU_INCLUDE_DESC
    "Active EEE Wake timer",
    "",
#endif
    { XUMAC_RDP_ACTIVE_EEE_WAKE_TIMER_REG_OFFSET },
    0,
    0,
    1193,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_ACTIVE_EEE_WAKE_TIMER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_MAC_PFC_TYPE, TYPE: Type_MAC_PFC_TYPE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PFC_ETH_TYPE *****/
const ru_field_rec XUMAC_RDP_MAC_PFC_TYPE_PFC_ETH_TYPE_FIELD =
{
    "PFC_ETH_TYPE",
#if RU_INCLUDE_DESC
    "",
    "Ethertype for PFC packets. The default value (0x8808) is the standard value.\n",
#endif
    { XUMAC_RDP_MAC_PFC_TYPE_PFC_ETH_TYPE_FIELD_MASK },
    0,
    { XUMAC_RDP_MAC_PFC_TYPE_PFC_ETH_TYPE_FIELD_WIDTH },
    { XUMAC_RDP_MAC_PFC_TYPE_PFC_ETH_TYPE_FIELD_SHIFT },
    34824,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_MAC_PFC_TYPE_FIELDS[] =
{
    &XUMAC_RDP_MAC_PFC_TYPE_PFC_ETH_TYPE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_MAC_PFC_TYPE *****/
const ru_reg_rec XUMAC_RDP_MAC_PFC_TYPE_REG =
{
    "MAC_PFC_TYPE",
#if RU_INCLUDE_DESC
    "PFC ethertype",
    "",
#endif
    { XUMAC_RDP_MAC_PFC_TYPE_REG_OFFSET },
    0,
    0,
    1194,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_MAC_PFC_TYPE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_MAC_PFC_OPCODE, TYPE: Type_MAC_PFC_OPCODE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PFC_OPCODE *****/
const ru_field_rec XUMAC_RDP_MAC_PFC_OPCODE_PFC_OPCODE_FIELD =
{
    "PFC_OPCODE",
#if RU_INCLUDE_DESC
    "",
    "PFC opcode. The default value (0x0101) is the standard value.\n",
#endif
    { XUMAC_RDP_MAC_PFC_OPCODE_PFC_OPCODE_FIELD_MASK },
    0,
    { XUMAC_RDP_MAC_PFC_OPCODE_PFC_OPCODE_FIELD_WIDTH },
    { XUMAC_RDP_MAC_PFC_OPCODE_PFC_OPCODE_FIELD_SHIFT },
    257,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_MAC_PFC_OPCODE_FIELDS[] =
{
    &XUMAC_RDP_MAC_PFC_OPCODE_PFC_OPCODE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_MAC_PFC_OPCODE *****/
const ru_reg_rec XUMAC_RDP_MAC_PFC_OPCODE_REG =
{
    "MAC_PFC_OPCODE",
#if RU_INCLUDE_DESC
    "PFC opcode",
    "",
#endif
    { XUMAC_RDP_MAC_PFC_OPCODE_REG_OFFSET },
    0,
    0,
    1195,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_MAC_PFC_OPCODE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_MAC_PFC_DA_0, TYPE: Type_MAC_PFC_DA_0
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PFC_MACDA_0 *****/
const ru_field_rec XUMAC_RDP_MAC_PFC_DA_0_PFC_MACDA_0_FIELD =
{
    "PFC_MACDA_0",
#if RU_INCLUDE_DESC
    "",
    "Lower 32 bits of DA for PFC.\n",
#endif
    { XUMAC_RDP_MAC_PFC_DA_0_PFC_MACDA_0_FIELD_MASK },
    0,
    { XUMAC_RDP_MAC_PFC_DA_0_PFC_MACDA_0_FIELD_WIDTH },
    { XUMAC_RDP_MAC_PFC_DA_0_PFC_MACDA_0_FIELD_SHIFT },
    3254779905,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_MAC_PFC_DA_0_FIELDS[] =
{
    &XUMAC_RDP_MAC_PFC_DA_0_PFC_MACDA_0_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_MAC_PFC_DA_0 *****/
const ru_reg_rec XUMAC_RDP_MAC_PFC_DA_0_REG =
{
    "MAC_PFC_DA_0",
#if RU_INCLUDE_DESC
    "lower 32 bits of DA for PFC",
    "",
#endif
    { XUMAC_RDP_MAC_PFC_DA_0_REG_OFFSET },
    0,
    0,
    1196,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_MAC_PFC_DA_0_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_MAC_PFC_DA_1, TYPE: Type_MAC_PFC_DA_1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PFC_MACDA_1 *****/
const ru_field_rec XUMAC_RDP_MAC_PFC_DA_1_PFC_MACDA_1_FIELD =
{
    "PFC_MACDA_1",
#if RU_INCLUDE_DESC
    "",
    "Upper 16 bits of DA for PFC.\n",
#endif
    { XUMAC_RDP_MAC_PFC_DA_1_PFC_MACDA_1_FIELD_MASK },
    0,
    { XUMAC_RDP_MAC_PFC_DA_1_PFC_MACDA_1_FIELD_WIDTH },
    { XUMAC_RDP_MAC_PFC_DA_1_PFC_MACDA_1_FIELD_SHIFT },
    384,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_MAC_PFC_DA_1_FIELDS[] =
{
    &XUMAC_RDP_MAC_PFC_DA_1_PFC_MACDA_1_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_MAC_PFC_DA_1 *****/
const ru_reg_rec XUMAC_RDP_MAC_PFC_DA_1_REG =
{
    "MAC_PFC_DA_1",
#if RU_INCLUDE_DESC
    "upper 16 bits of DA for PFC",
    "",
#endif
    { XUMAC_RDP_MAC_PFC_DA_1_REG_OFFSET },
    0,
    0,
    1197,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_MAC_PFC_DA_1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_MACSEC_PROG_TX_CRC, TYPE: Type_MACSEC_PROG_TX_CRC
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MACSEC_PROG_TX_CRC *****/
const ru_field_rec XUMAC_RDP_MACSEC_PROG_TX_CRC_MACSEC_PROG_TX_CRC_FIELD =
{
    "MACSEC_PROG_TX_CRC",
#if RU_INCLUDE_DESC
    "",
    "The transmitted CRC can be corrupted by replacing the FCS of the transmitted frame by the FCS programmed in this register.\nThis is enabled and controlled by MACSEC_CNTRL register.\n",
#endif
    { XUMAC_RDP_MACSEC_PROG_TX_CRC_MACSEC_PROG_TX_CRC_FIELD_MASK },
    0,
    { XUMAC_RDP_MACSEC_PROG_TX_CRC_MACSEC_PROG_TX_CRC_FIELD_WIDTH },
    { XUMAC_RDP_MACSEC_PROG_TX_CRC_MACSEC_PROG_TX_CRC_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_MACSEC_PROG_TX_CRC_FIELDS[] =
{
    &XUMAC_RDP_MACSEC_PROG_TX_CRC_MACSEC_PROG_TX_CRC_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_MACSEC_PROG_TX_CRC *****/
const ru_reg_rec XUMAC_RDP_MACSEC_PROG_TX_CRC_REG =
{
    "MACSEC_PROG_TX_CRC",
#if RU_INCLUDE_DESC
    "Programmable CRC value to corrupt the Tx CRC to be used in MACSEC",
    "",
#endif
    { XUMAC_RDP_MACSEC_PROG_TX_CRC_REG_OFFSET },
    0,
    0,
    1198,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_MACSEC_PROG_TX_CRC_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_MACSEC_CNTRL, TYPE: Type_MACSEC_CNTRL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TX_LAUNCH_EN *****/
const ru_field_rec XUMAC_RDP_MACSEC_CNTRL_TX_LAUNCH_EN_FIELD =
{
    "TX_LAUNCH_EN",
#if RU_INCLUDE_DESC
    "",
    "Set the bit 0 (Tx_Launch_en) logic 0, if the tx_launch function is to be disabled. If set, then the launch_enable signal assertion/deassertion causes the packet transmit enabled/disabled. The launch_enable is per packet basis.\n",
#endif
    { XUMAC_RDP_MACSEC_CNTRL_TX_LAUNCH_EN_FIELD_MASK },
    0,
    { XUMAC_RDP_MACSEC_CNTRL_TX_LAUNCH_EN_FIELD_WIDTH },
    { XUMAC_RDP_MACSEC_CNTRL_TX_LAUNCH_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TX_CRC_CORUPT_EN *****/
const ru_field_rec XUMAC_RDP_MACSEC_CNTRL_TX_CRC_CORUPT_EN_FIELD =
{
    "TX_CRC_CORUPT_EN",
#if RU_INCLUDE_DESC
    "",
    "Setting this field enables the CRC corruption on the transmitted packets. The options of how to corrupt, depends on\nthe field 2 of this register (TX_CRC_PROGRAM). The CRC corruption happens only on the frames for which TXCRCER is asserted by the system.\n",
#endif
    { XUMAC_RDP_MACSEC_CNTRL_TX_CRC_CORUPT_EN_FIELD_MASK },
    0,
    { XUMAC_RDP_MACSEC_CNTRL_TX_CRC_CORUPT_EN_FIELD_WIDTH },
    { XUMAC_RDP_MACSEC_CNTRL_TX_CRC_CORUPT_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TX_CRC_PROGRAM *****/
const ru_field_rec XUMAC_RDP_MACSEC_CNTRL_TX_CRC_PROGRAM_FIELD =
{
    "TX_CRC_PROGRAM",
#if RU_INCLUDE_DESC
    "",
    "If CRC corruption feature in enabled (TX_CRC_CORUPT_EN set), then in case where this bit when set, replaces the transmitted FCS with the programmed FCS.\nWhen cleared, corrupts the CRC of the transmitted packet internally.\n",
#endif
    { XUMAC_RDP_MACSEC_CNTRL_TX_CRC_PROGRAM_FIELD_MASK },
    0,
    { XUMAC_RDP_MACSEC_CNTRL_TX_CRC_PROGRAM_FIELD_WIDTH },
    { XUMAC_RDP_MACSEC_CNTRL_TX_CRC_PROGRAM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DIS_PAUSE_DATA_VAR_IPG *****/
const ru_field_rec XUMAC_RDP_MACSEC_CNTRL_DIS_PAUSE_DATA_VAR_IPG_FIELD =
{
    "DIS_PAUSE_DATA_VAR_IPG",
#if RU_INCLUDE_DESC
    "",
    "When this bit is 1, IPG between pause and data frame is as per the original design, i.e., 13B or 12B, fixed. It should be noted, that as number of preamble bytes reduces from 7, the IPG also increases. \nWhen this bit is 0, IPG between pause and data frame is variable and equals programmed IPG or programmed IPG + 1.\n",
#endif
    { XUMAC_RDP_MACSEC_CNTRL_DIS_PAUSE_DATA_VAR_IPG_FIELD_MASK },
    0,
    { XUMAC_RDP_MACSEC_CNTRL_DIS_PAUSE_DATA_VAR_IPG_FIELD_WIDTH },
    { XUMAC_RDP_MACSEC_CNTRL_DIS_PAUSE_DATA_VAR_IPG_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_MACSEC_CNTRL_FIELDS[] =
{
    &XUMAC_RDP_MACSEC_CNTRL_TX_LAUNCH_EN_FIELD,
    &XUMAC_RDP_MACSEC_CNTRL_TX_CRC_CORUPT_EN_FIELD,
    &XUMAC_RDP_MACSEC_CNTRL_TX_CRC_PROGRAM_FIELD,
    &XUMAC_RDP_MACSEC_CNTRL_DIS_PAUSE_DATA_VAR_IPG_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_MACSEC_CNTRL *****/
const ru_reg_rec XUMAC_RDP_MACSEC_CNTRL_REG =
{
    "MACSEC_CNTRL",
#if RU_INCLUDE_DESC
    "Miscellaneous control for MACSEC",
    "",
#endif
    { XUMAC_RDP_MACSEC_CNTRL_REG_OFFSET },
    0,
    0,
    1199,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    XUMAC_RDP_MACSEC_CNTRL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_TS_STATUS, TYPE: Type_TS_STATUS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TX_TS_FIFO_FULL *****/
const ru_field_rec XUMAC_RDP_TS_STATUS_TX_TS_FIFO_FULL_FIELD =
{
    "TX_TS_FIFO_FULL",
#if RU_INCLUDE_DESC
    "",
    "Read-only field assertion shows that the transmit timestamp FIFO is full.\n",
#endif
    { XUMAC_RDP_TS_STATUS_TX_TS_FIFO_FULL_FIELD_MASK },
    0,
    { XUMAC_RDP_TS_STATUS_TX_TS_FIFO_FULL_FIELD_WIDTH },
    { XUMAC_RDP_TS_STATUS_TX_TS_FIFO_FULL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TX_TS_FIFO_EMPTY *****/
const ru_field_rec XUMAC_RDP_TS_STATUS_TX_TS_FIFO_EMPTY_FIELD =
{
    "TX_TS_FIFO_EMPTY",
#if RU_INCLUDE_DESC
    "",
    "Read-only field assertion shows that the transmit timestamp FIFO is empty.\n",
#endif
    { XUMAC_RDP_TS_STATUS_TX_TS_FIFO_EMPTY_FIELD_MASK },
    0,
    { XUMAC_RDP_TS_STATUS_TX_TS_FIFO_EMPTY_FIELD_WIDTH },
    { XUMAC_RDP_TS_STATUS_TX_TS_FIFO_EMPTY_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: WORD_AVAIL *****/
const ru_field_rec XUMAC_RDP_TS_STATUS_WORD_AVAIL_FIELD =
{
    "WORD_AVAIL",
#if RU_INCLUDE_DESC
    "",
    "Indicates number of cells filled in the TX timestamp FIFO.\n",
#endif
    { XUMAC_RDP_TS_STATUS_WORD_AVAIL_FIELD_MASK },
    0,
    { XUMAC_RDP_TS_STATUS_WORD_AVAIL_FIELD_WIDTH },
    { XUMAC_RDP_TS_STATUS_WORD_AVAIL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_TS_STATUS_FIELDS[] =
{
    &XUMAC_RDP_TS_STATUS_TX_TS_FIFO_FULL_FIELD,
    &XUMAC_RDP_TS_STATUS_TX_TS_FIFO_EMPTY_FIELD,
    &XUMAC_RDP_TS_STATUS_WORD_AVAIL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_TS_STATUS *****/
const ru_reg_rec XUMAC_RDP_TS_STATUS_REG =
{
    "TS_STATUS",
#if RU_INCLUDE_DESC
    "Timestamp status",
    "",
#endif
    { XUMAC_RDP_TS_STATUS_REG_OFFSET },
    0,
    0,
    1200,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    XUMAC_RDP_TS_STATUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_TX_TS_DATA, TYPE: Type_TX_TS_DATA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TX_TS_DATA *****/
const ru_field_rec XUMAC_RDP_TX_TS_DATA_TX_TS_DATA_FIELD =
{
    "TX_TS_DATA",
#if RU_INCLUDE_DESC
    "",
    "Every read of this register will fetch out one timestamp value corresponding to the preceding seq_id read from the transmit FIFO.\nEvery 49 bit, val_bit + seq_id + timestamp is read in two steps, i.e., one read from 0x10f (val_bit + seq_id) followed by another read from 0x1c7 (timestamp).\nTimestamp read without a preceding seq_id read will fetch stale timestamp value.\n",
#endif
    { XUMAC_RDP_TX_TS_DATA_TX_TS_DATA_FIELD_MASK },
    0,
    { XUMAC_RDP_TX_TS_DATA_TX_TS_DATA_FIELD_WIDTH },
    { XUMAC_RDP_TX_TS_DATA_TX_TS_DATA_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_TX_TS_DATA_FIELDS[] =
{
    &XUMAC_RDP_TX_TS_DATA_TX_TS_DATA_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_TX_TS_DATA *****/
const ru_reg_rec XUMAC_RDP_TX_TS_DATA_REG =
{
    "TX_TS_DATA",
#if RU_INCLUDE_DESC
    "Transmit Timestamp data",
    "",
#endif
    { XUMAC_RDP_TX_TS_DATA_REG_OFFSET },
    0,
    0,
    1201,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_TX_TS_DATA_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_PAUSE_REFRESH_CTRL, TYPE: Type_PAUSE_REFRESH_CTRL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: REFRESH_TIMER *****/
const ru_field_rec XUMAC_RDP_PAUSE_REFRESH_CTRL_REFRESH_TIMER_FIELD =
{
    "REFRESH_TIMER",
#if RU_INCLUDE_DESC
    "",
    "Timer expiry time, represented in 512 bit time units. Note that the actual expiry time depends on the port speed. Values of 0 and 1 are illegal.\n",
#endif
    { XUMAC_RDP_PAUSE_REFRESH_CTRL_REFRESH_TIMER_FIELD_MASK },
    0,
    { XUMAC_RDP_PAUSE_REFRESH_CTRL_REFRESH_TIMER_FIELD_WIDTH },
    { XUMAC_RDP_PAUSE_REFRESH_CTRL_REFRESH_TIMER_FIELD_SHIFT },
    65535,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: ENABLE *****/
const ru_field_rec XUMAC_RDP_PAUSE_REFRESH_CTRL_ENABLE_FIELD =
{
    "ENABLE",
#if RU_INCLUDE_DESC
    "",
    "Enable extra pause frames.\n",
#endif
    { XUMAC_RDP_PAUSE_REFRESH_CTRL_ENABLE_FIELD_MASK },
    0,
    { XUMAC_RDP_PAUSE_REFRESH_CTRL_ENABLE_FIELD_WIDTH },
    { XUMAC_RDP_PAUSE_REFRESH_CTRL_ENABLE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_PAUSE_REFRESH_CTRL_FIELDS[] =
{
    &XUMAC_RDP_PAUSE_REFRESH_CTRL_REFRESH_TIMER_FIELD,
    &XUMAC_RDP_PAUSE_REFRESH_CTRL_ENABLE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_PAUSE_REFRESH_CTRL *****/
const ru_reg_rec XUMAC_RDP_PAUSE_REFRESH_CTRL_REG =
{
    "PAUSE_REFRESH_CTRL",
#if RU_INCLUDE_DESC
    "PAUSE frame refresh timer control register",
    "",
#endif
    { XUMAC_RDP_PAUSE_REFRESH_CTRL_REG_OFFSET },
    0,
    0,
    1202,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XUMAC_RDP_PAUSE_REFRESH_CTRL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_FLUSH_CONTROL, TYPE: Type_FLUSH_CONTROL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: FLUSH *****/
const ru_field_rec XUMAC_RDP_FLUSH_CONTROL_FLUSH_FIELD =
{
    "FLUSH",
#if RU_INCLUDE_DESC
    "",
    "Flush enable bit to drop out all packets in Tx FIFO without egressing any packets when set.\n",
#endif
    { XUMAC_RDP_FLUSH_CONTROL_FLUSH_FIELD_MASK },
    0,
    { XUMAC_RDP_FLUSH_CONTROL_FLUSH_FIELD_WIDTH },
    { XUMAC_RDP_FLUSH_CONTROL_FLUSH_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_FLUSH_CONTROL_FIELDS[] =
{
    &XUMAC_RDP_FLUSH_CONTROL_FLUSH_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_FLUSH_CONTROL *****/
const ru_reg_rec XUMAC_RDP_FLUSH_CONTROL_REG =
{
    "FLUSH_CONTROL",
#if RU_INCLUDE_DESC
    "Flush enable control register",
    "",
#endif
    { XUMAC_RDP_FLUSH_CONTROL_REG_OFFSET },
    0,
    0,
    1203,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_FLUSH_CONTROL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_RXFIFO_STAT, TYPE: Type_RXFIFO_STAT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RXFIFO_UNDERRUN *****/
const ru_field_rec XUMAC_RDP_RXFIFO_STAT_RXFIFO_UNDERRUN_FIELD =
{
    "RXFIFO_UNDERRUN",
#if RU_INCLUDE_DESC
    "",
    "RXFIFO Underrun occurred.\n",
#endif
    { XUMAC_RDP_RXFIFO_STAT_RXFIFO_UNDERRUN_FIELD_MASK },
    0,
    { XUMAC_RDP_RXFIFO_STAT_RXFIFO_UNDERRUN_FIELD_WIDTH },
    { XUMAC_RDP_RXFIFO_STAT_RXFIFO_UNDERRUN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RXFIFO_OVERRUN *****/
const ru_field_rec XUMAC_RDP_RXFIFO_STAT_RXFIFO_OVERRUN_FIELD =
{
    "RXFIFO_OVERRUN",
#if RU_INCLUDE_DESC
    "",
    "RXFIFO Overrun occurred.\n",
#endif
    { XUMAC_RDP_RXFIFO_STAT_RXFIFO_OVERRUN_FIELD_MASK },
    0,
    { XUMAC_RDP_RXFIFO_STAT_RXFIFO_OVERRUN_FIELD_WIDTH },
    { XUMAC_RDP_RXFIFO_STAT_RXFIFO_OVERRUN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_RXFIFO_STAT_FIELDS[] =
{
    &XUMAC_RDP_RXFIFO_STAT_RXFIFO_UNDERRUN_FIELD,
    &XUMAC_RDP_RXFIFO_STAT_RXFIFO_OVERRUN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_RXFIFO_STAT *****/
const ru_reg_rec XUMAC_RDP_RXFIFO_STAT_REG =
{
    "RXFIFO_STAT",
#if RU_INCLUDE_DESC
    "RXFIFO status register",
    "",
#endif
    { XUMAC_RDP_RXFIFO_STAT_REG_OFFSET },
    0,
    0,
    1204,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XUMAC_RDP_RXFIFO_STAT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_TXFIFO_STAT, TYPE: Type_TXFIFO_STAT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TXFIFO_UNDERRUN *****/
const ru_field_rec XUMAC_RDP_TXFIFO_STAT_TXFIFO_UNDERRUN_FIELD =
{
    "TXFIFO_UNDERRUN",
#if RU_INCLUDE_DESC
    "",
    "TXFIFO Underrun occurred.\n",
#endif
    { XUMAC_RDP_TXFIFO_STAT_TXFIFO_UNDERRUN_FIELD_MASK },
    0,
    { XUMAC_RDP_TXFIFO_STAT_TXFIFO_UNDERRUN_FIELD_WIDTH },
    { XUMAC_RDP_TXFIFO_STAT_TXFIFO_UNDERRUN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TXFIFO_OVERRUN *****/
const ru_field_rec XUMAC_RDP_TXFIFO_STAT_TXFIFO_OVERRUN_FIELD =
{
    "TXFIFO_OVERRUN",
#if RU_INCLUDE_DESC
    "",
    "TXFIFO Overrun occurred.\n",
#endif
    { XUMAC_RDP_TXFIFO_STAT_TXFIFO_OVERRUN_FIELD_MASK },
    0,
    { XUMAC_RDP_TXFIFO_STAT_TXFIFO_OVERRUN_FIELD_WIDTH },
    { XUMAC_RDP_TXFIFO_STAT_TXFIFO_OVERRUN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_TXFIFO_STAT_FIELDS[] =
{
    &XUMAC_RDP_TXFIFO_STAT_TXFIFO_UNDERRUN_FIELD,
    &XUMAC_RDP_TXFIFO_STAT_TXFIFO_OVERRUN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_TXFIFO_STAT *****/
const ru_reg_rec XUMAC_RDP_TXFIFO_STAT_REG =
{
    "TXFIFO_STAT",
#if RU_INCLUDE_DESC
    "TXFIFO status register",
    "",
#endif
    { XUMAC_RDP_TXFIFO_STAT_REG_OFFSET },
    0,
    0,
    1205,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XUMAC_RDP_TXFIFO_STAT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_MAC_PFC_CTRL, TYPE: Type_MAC_PFC_CTRL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PFC_TX_ENBL *****/
const ru_field_rec XUMAC_RDP_MAC_PFC_CTRL_PFC_TX_ENBL_FIELD =
{
    "PFC_TX_ENBL",
#if RU_INCLUDE_DESC
    "",
    "Enables the PFC-Tx functionality.\n",
#endif
    { XUMAC_RDP_MAC_PFC_CTRL_PFC_TX_ENBL_FIELD_MASK },
    0,
    { XUMAC_RDP_MAC_PFC_CTRL_PFC_TX_ENBL_FIELD_WIDTH },
    { XUMAC_RDP_MAC_PFC_CTRL_PFC_TX_ENBL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PFC_RX_ENBL *****/
const ru_field_rec XUMAC_RDP_MAC_PFC_CTRL_PFC_RX_ENBL_FIELD =
{
    "PFC_RX_ENBL",
#if RU_INCLUDE_DESC
    "",
    "Enables the PFC-Rx functionality.\n",
#endif
    { XUMAC_RDP_MAC_PFC_CTRL_PFC_RX_ENBL_FIELD_MASK },
    0,
    { XUMAC_RDP_MAC_PFC_CTRL_PFC_RX_ENBL_FIELD_WIDTH },
    { XUMAC_RDP_MAC_PFC_CTRL_PFC_RX_ENBL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FORCE_PFC_XON *****/
const ru_field_rec XUMAC_RDP_MAC_PFC_CTRL_FORCE_PFC_XON_FIELD =
{
    "FORCE_PFC_XON",
#if RU_INCLUDE_DESC
    "",
    "Instructs MAC to send Xon message to all classes of service.\n",
#endif
    { XUMAC_RDP_MAC_PFC_CTRL_FORCE_PFC_XON_FIELD_MASK },
    0,
    { XUMAC_RDP_MAC_PFC_CTRL_FORCE_PFC_XON_FIELD_WIDTH },
    { XUMAC_RDP_MAC_PFC_CTRL_FORCE_PFC_XON_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RX_PASS_PFC_FRM *****/
const ru_field_rec XUMAC_RDP_MAC_PFC_CTRL_RX_PASS_PFC_FRM_FIELD =
{
    "RX_PASS_PFC_FRM",
#if RU_INCLUDE_DESC
    "",
    "When set, MAC pass PFC frame to the system. Otherwise, PFC frame is discarded.\n",
#endif
    { XUMAC_RDP_MAC_PFC_CTRL_RX_PASS_PFC_FRM_FIELD_MASK },
    0,
    { XUMAC_RDP_MAC_PFC_CTRL_RX_PASS_PFC_FRM_FIELD_WIDTH },
    { XUMAC_RDP_MAC_PFC_CTRL_RX_PASS_PFC_FRM_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PFC_STATS_EN *****/
const ru_field_rec XUMAC_RDP_MAC_PFC_CTRL_PFC_STATS_EN_FIELD =
{
    "PFC_STATS_EN",
#if RU_INCLUDE_DESC
    "",
    "When clear, none of PFC related counters should increment. \nOtherwise, PFC counters is in full function. \nNote: it is programming requirement to set this bit when PFC function is enable.\n",
#endif
    { XUMAC_RDP_MAC_PFC_CTRL_PFC_STATS_EN_FIELD_MASK },
    0,
    { XUMAC_RDP_MAC_PFC_CTRL_PFC_STATS_EN_FIELD_WIDTH },
    { XUMAC_RDP_MAC_PFC_CTRL_PFC_STATS_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_MAC_PFC_CTRL_FIELDS[] =
{
    &XUMAC_RDP_MAC_PFC_CTRL_PFC_TX_ENBL_FIELD,
    &XUMAC_RDP_MAC_PFC_CTRL_PFC_RX_ENBL_FIELD,
    &XUMAC_RDP_MAC_PFC_CTRL_FORCE_PFC_XON_FIELD,
    &XUMAC_RDP_MAC_PFC_CTRL_RX_PASS_PFC_FRM_FIELD,
    &XUMAC_RDP_MAC_PFC_CTRL_PFC_STATS_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_MAC_PFC_CTRL *****/
const ru_reg_rec XUMAC_RDP_MAC_PFC_CTRL_REG =
{
    "MAC_PFC_CTRL",
#if RU_INCLUDE_DESC
    "PFC control register",
    "",
#endif
    { XUMAC_RDP_MAC_PFC_CTRL_REG_OFFSET },
    0,
    0,
    1206,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    XUMAC_RDP_MAC_PFC_CTRL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_MAC_PFC_REFRESH_CTRL, TYPE: Type_MAC_PFC_REFRESH_CTRL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PFC_REFRESH_EN *****/
const ru_field_rec XUMAC_RDP_MAC_PFC_REFRESH_CTRL_PFC_REFRESH_EN_FIELD =
{
    "PFC_REFRESH_EN",
#if RU_INCLUDE_DESC
    "",
    "Enables the PFC refresh functionality on the Tx side. When enabled, the MAC sends Xoff message on refresh counter becoming 0\n",
#endif
    { XUMAC_RDP_MAC_PFC_REFRESH_CTRL_PFC_REFRESH_EN_FIELD_MASK },
    0,
    { XUMAC_RDP_MAC_PFC_REFRESH_CTRL_PFC_REFRESH_EN_FIELD_WIDTH },
    { XUMAC_RDP_MAC_PFC_REFRESH_CTRL_PFC_REFRESH_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PFC_REFRESH_TIMER *****/
const ru_field_rec XUMAC_RDP_MAC_PFC_REFRESH_CTRL_PFC_REFRESH_TIMER_FIELD =
{
    "PFC_REFRESH_TIMER",
#if RU_INCLUDE_DESC
    "",
    "PFC refresh counter value.\n",
#endif
    { XUMAC_RDP_MAC_PFC_REFRESH_CTRL_PFC_REFRESH_TIMER_FIELD_MASK },
    0,
    { XUMAC_RDP_MAC_PFC_REFRESH_CTRL_PFC_REFRESH_TIMER_FIELD_WIDTH },
    { XUMAC_RDP_MAC_PFC_REFRESH_CTRL_PFC_REFRESH_TIMER_FIELD_SHIFT },
    32767,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_MAC_PFC_REFRESH_CTRL_FIELDS[] =
{
    &XUMAC_RDP_MAC_PFC_REFRESH_CTRL_PFC_REFRESH_EN_FIELD,
    &XUMAC_RDP_MAC_PFC_REFRESH_CTRL_PFC_REFRESH_TIMER_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_MAC_PFC_REFRESH_CTRL *****/
const ru_reg_rec XUMAC_RDP_MAC_PFC_REFRESH_CTRL_REG =
{
    "MAC_PFC_REFRESH_CTRL",
#if RU_INCLUDE_DESC
    "PFC refresh control register",
    "",
#endif
    { XUMAC_RDP_MAC_PFC_REFRESH_CTRL_REG_OFFSET },
    0,
    0,
    1207,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XUMAC_RDP_MAC_PFC_REFRESH_CTRL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GR64, TYPE: Type_MIB_GR64
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_GR64_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Receive 64 Bytes frame counter.\n",
#endif
    { XUMAC_RDP_GR64_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_GR64_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_GR64_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GR64_FIELDS[] =
{
    &XUMAC_RDP_GR64_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GR64 *****/
const ru_reg_rec XUMAC_RDP_GR64_REG =
{
    "GR64",
#if RU_INCLUDE_DESC
    "Receive 64B Frame Counter",
    "",
#endif
    { XUMAC_RDP_GR64_REG_OFFSET },
    0,
    0,
    1208,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GR64_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GR64_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_GR64_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_GR64_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_GR64_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_GR64_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GR64_UPPER_FIELDS[] =
{
    &XUMAC_RDP_GR64_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GR64_UPPER *****/
const ru_reg_rec XUMAC_RDP_GR64_UPPER_REG =
{
    "GR64_UPPER",
#if RU_INCLUDE_DESC
    "GR64 upper 8 bits",
    "",
#endif
    { XUMAC_RDP_GR64_UPPER_REG_OFFSET },
    0,
    0,
    1209,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GR64_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GR127, TYPE: Type_MIB_GR127
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_GR127_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Receive 65 bytes to 127 bytes frame counter.\n",
#endif
    { XUMAC_RDP_GR127_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_GR127_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_GR127_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GR127_FIELDS[] =
{
    &XUMAC_RDP_GR127_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GR127 *****/
const ru_reg_rec XUMAC_RDP_GR127_REG =
{
    "GR127",
#if RU_INCLUDE_DESC
    "Receive 65B to 127B Frame Counter",
    "",
#endif
    { XUMAC_RDP_GR127_REG_OFFSET },
    0,
    0,
    1210,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GR127_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GR127_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_GR127_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_GR127_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_GR127_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_GR127_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GR127_UPPER_FIELDS[] =
{
    &XUMAC_RDP_GR127_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GR127_UPPER *****/
const ru_reg_rec XUMAC_RDP_GR127_UPPER_REG =
{
    "GR127_UPPER",
#if RU_INCLUDE_DESC
    "GR127 upper 8 bits",
    "",
#endif
    { XUMAC_RDP_GR127_UPPER_REG_OFFSET },
    0,
    0,
    1211,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GR127_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GR255, TYPE: Type_MIB_GR255
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_GR255_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Receive 128 bytes to 255 bytes frame counter.\n",
#endif
    { XUMAC_RDP_GR255_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_GR255_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_GR255_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GR255_FIELDS[] =
{
    &XUMAC_RDP_GR255_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GR255 *****/
const ru_reg_rec XUMAC_RDP_GR255_REG =
{
    "GR255",
#if RU_INCLUDE_DESC
    "Receive 128B to 255B Frame Counter",
    "",
#endif
    { XUMAC_RDP_GR255_REG_OFFSET },
    0,
    0,
    1212,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GR255_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GR255_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_GR255_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_GR255_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_GR255_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_GR255_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GR255_UPPER_FIELDS[] =
{
    &XUMAC_RDP_GR255_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GR255_UPPER *****/
const ru_reg_rec XUMAC_RDP_GR255_UPPER_REG =
{
    "GR255_UPPER",
#if RU_INCLUDE_DESC
    "GR255 upper 8 bits",
    "",
#endif
    { XUMAC_RDP_GR255_UPPER_REG_OFFSET },
    0,
    0,
    1213,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GR255_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GR511, TYPE: Type_MIB_GR511
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_GR511_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Receive 256 bytes to 511 bytes frame counter.\n",
#endif
    { XUMAC_RDP_GR511_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_GR511_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_GR511_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GR511_FIELDS[] =
{
    &XUMAC_RDP_GR511_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GR511 *****/
const ru_reg_rec XUMAC_RDP_GR511_REG =
{
    "GR511",
#if RU_INCLUDE_DESC
    "Receive 256B to 511B Frame Counter",
    "",
#endif
    { XUMAC_RDP_GR511_REG_OFFSET },
    0,
    0,
    1214,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GR511_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GR511_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_GR511_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_GR511_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_GR511_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_GR511_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GR511_UPPER_FIELDS[] =
{
    &XUMAC_RDP_GR511_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GR511_UPPER *****/
const ru_reg_rec XUMAC_RDP_GR511_UPPER_REG =
{
    "GR511_UPPER",
#if RU_INCLUDE_DESC
    "GR511 upper 8 bits",
    "",
#endif
    { XUMAC_RDP_GR511_UPPER_REG_OFFSET },
    0,
    0,
    1215,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GR511_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GR1023, TYPE: Type_MIB_GR1023
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_GR1023_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Receive 512 bytes to 1023 bytes frame counter.\n",
#endif
    { XUMAC_RDP_GR1023_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_GR1023_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_GR1023_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GR1023_FIELDS[] =
{
    &XUMAC_RDP_GR1023_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GR1023 *****/
const ru_reg_rec XUMAC_RDP_GR1023_REG =
{
    "GR1023",
#if RU_INCLUDE_DESC
    "Receive 512B to 1023B Frame Counter",
    "",
#endif
    { XUMAC_RDP_GR1023_REG_OFFSET },
    0,
    0,
    1216,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GR1023_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GR1023_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_GR1023_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_GR1023_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_GR1023_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_GR1023_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GR1023_UPPER_FIELDS[] =
{
    &XUMAC_RDP_GR1023_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GR1023_UPPER *****/
const ru_reg_rec XUMAC_RDP_GR1023_UPPER_REG =
{
    "GR1023_UPPER",
#if RU_INCLUDE_DESC
    "GR1023 upper 8 bits",
    "",
#endif
    { XUMAC_RDP_GR1023_UPPER_REG_OFFSET },
    0,
    0,
    1217,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GR1023_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GR1518, TYPE: Type_MIB_GR1518
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_GR1518_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Receive 1024 bytes to 1518 bytes frame counter.\n",
#endif
    { XUMAC_RDP_GR1518_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_GR1518_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_GR1518_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GR1518_FIELDS[] =
{
    &XUMAC_RDP_GR1518_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GR1518 *****/
const ru_reg_rec XUMAC_RDP_GR1518_REG =
{
    "GR1518",
#if RU_INCLUDE_DESC
    "Receive 1024B to 1518B Frame Counter",
    "",
#endif
    { XUMAC_RDP_GR1518_REG_OFFSET },
    0,
    0,
    1218,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GR1518_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GR1518_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_GR1518_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_GR1518_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_GR1518_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_GR1518_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GR1518_UPPER_FIELDS[] =
{
    &XUMAC_RDP_GR1518_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GR1518_UPPER *****/
const ru_reg_rec XUMAC_RDP_GR1518_UPPER_REG =
{
    "GR1518_UPPER",
#if RU_INCLUDE_DESC
    "GR1518 upper 8 bits",
    "",
#endif
    { XUMAC_RDP_GR1518_UPPER_REG_OFFSET },
    0,
    0,
    1219,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GR1518_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GRMGV, TYPE: Type_MIB_GRMGV
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_GRMGV_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Receive 1519 bytes to 1522 bytes good VLAN frame counter.\n",
#endif
    { XUMAC_RDP_GRMGV_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_GRMGV_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_GRMGV_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GRMGV_FIELDS[] =
{
    &XUMAC_RDP_GRMGV_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GRMGV *****/
const ru_reg_rec XUMAC_RDP_GRMGV_REG =
{
    "GRMGV",
#if RU_INCLUDE_DESC
    "Receive 1519B to 1522B Good VLAN Frame Counter",
    "",
#endif
    { XUMAC_RDP_GRMGV_REG_OFFSET },
    0,
    0,
    1220,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GRMGV_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GRMGV_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_GRMGV_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_GRMGV_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_GRMGV_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_GRMGV_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GRMGV_UPPER_FIELDS[] =
{
    &XUMAC_RDP_GRMGV_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GRMGV_UPPER *****/
const ru_reg_rec XUMAC_RDP_GRMGV_UPPER_REG =
{
    "GRMGV_UPPER",
#if RU_INCLUDE_DESC
    "GRMGV upper 8 bits",
    "",
#endif
    { XUMAC_RDP_GRMGV_UPPER_REG_OFFSET },
    0,
    0,
    1221,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GRMGV_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GR2047, TYPE: Type_MIB_GR2047
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_GR2047_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Receive 1519 bytes to 2047 bytes frame counter.\n",
#endif
    { XUMAC_RDP_GR2047_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_GR2047_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_GR2047_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GR2047_FIELDS[] =
{
    &XUMAC_RDP_GR2047_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GR2047 *****/
const ru_reg_rec XUMAC_RDP_GR2047_REG =
{
    "GR2047",
#if RU_INCLUDE_DESC
    "Receive 1519B to 2047B Frame Counter",
    "",
#endif
    { XUMAC_RDP_GR2047_REG_OFFSET },
    0,
    0,
    1222,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GR2047_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GR2047_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_GR2047_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_GR2047_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_GR2047_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_GR2047_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GR2047_UPPER_FIELDS[] =
{
    &XUMAC_RDP_GR2047_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GR2047_UPPER *****/
const ru_reg_rec XUMAC_RDP_GR2047_UPPER_REG =
{
    "GR2047_UPPER",
#if RU_INCLUDE_DESC
    "GR2047 upper 8 bits",
    "",
#endif
    { XUMAC_RDP_GR2047_UPPER_REG_OFFSET },
    0,
    0,
    1223,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GR2047_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GR4095, TYPE: Type_MIB_GR4095
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_GR4095_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Receive 2048 bytes to 4096 bytes frame counter.\n",
#endif
    { XUMAC_RDP_GR4095_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_GR4095_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_GR4095_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GR4095_FIELDS[] =
{
    &XUMAC_RDP_GR4095_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GR4095 *****/
const ru_reg_rec XUMAC_RDP_GR4095_REG =
{
    "GR4095",
#if RU_INCLUDE_DESC
    "Receive 2048B to 4095B Frame Counter",
    "",
#endif
    { XUMAC_RDP_GR4095_REG_OFFSET },
    0,
    0,
    1224,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GR4095_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GR4095_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_GR4095_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_GR4095_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_GR4095_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_GR4095_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GR4095_UPPER_FIELDS[] =
{
    &XUMAC_RDP_GR4095_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GR4095_UPPER *****/
const ru_reg_rec XUMAC_RDP_GR4095_UPPER_REG =
{
    "GR4095_UPPER",
#if RU_INCLUDE_DESC
    "GR4095 upper 8 bits",
    "",
#endif
    { XUMAC_RDP_GR4095_UPPER_REG_OFFSET },
    0,
    0,
    1225,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GR4095_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GR9216, TYPE: Type_MIB_GR9216
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_GR9216_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Receive 4096 bytes to 9216 bytes frame counter.\n",
#endif
    { XUMAC_RDP_GR9216_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_GR9216_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_GR9216_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GR9216_FIELDS[] =
{
    &XUMAC_RDP_GR9216_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GR9216 *****/
const ru_reg_rec XUMAC_RDP_GR9216_REG =
{
    "GR9216",
#if RU_INCLUDE_DESC
    "Receive 4096B to 9216B Frame Counter",
    "",
#endif
    { XUMAC_RDP_GR9216_REG_OFFSET },
    0,
    0,
    1226,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GR9216_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GR9216_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_GR9216_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_GR9216_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_GR9216_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_GR9216_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GR9216_UPPER_FIELDS[] =
{
    &XUMAC_RDP_GR9216_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GR9216_UPPER *****/
const ru_reg_rec XUMAC_RDP_GR9216_UPPER_REG =
{
    "GR9216_UPPER",
#if RU_INCLUDE_DESC
    "GR9216 upper 8 bits",
    "",
#endif
    { XUMAC_RDP_GR9216_UPPER_REG_OFFSET },
    0,
    0,
    1227,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GR9216_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GRPKT, TYPE: Type_MIB_GRPKT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_GRPKT_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Receive packet counter.\n",
#endif
    { XUMAC_RDP_GRPKT_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_GRPKT_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_GRPKT_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GRPKT_FIELDS[] =
{
    &XUMAC_RDP_GRPKT_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GRPKT *****/
const ru_reg_rec XUMAC_RDP_GRPKT_REG =
{
    "GRPKT",
#if RU_INCLUDE_DESC
    "Receive Packet Counter",
    "",
#endif
    { XUMAC_RDP_GRPKT_REG_OFFSET },
    0,
    0,
    1228,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GRPKT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GRPKT_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_GRPKT_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_GRPKT_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_GRPKT_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_GRPKT_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GRPKT_UPPER_FIELDS[] =
{
    &XUMAC_RDP_GRPKT_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GRPKT_UPPER *****/
const ru_reg_rec XUMAC_RDP_GRPKT_UPPER_REG =
{
    "GRPKT_UPPER",
#if RU_INCLUDE_DESC
    "GRPKT upper 8 bits",
    "",
#endif
    { XUMAC_RDP_GRPKT_UPPER_REG_OFFSET },
    0,
    0,
    1229,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GRPKT_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GRBYT, TYPE: Type_MIB_GRBYT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_GRBYT_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Receive byte counter.\n",
#endif
    { XUMAC_RDP_GRBYT_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_GRBYT_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_GRBYT_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GRBYT_FIELDS[] =
{
    &XUMAC_RDP_GRBYT_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GRBYT *****/
const ru_reg_rec XUMAC_RDP_GRBYT_REG =
{
    "GRBYT",
#if RU_INCLUDE_DESC
    "Receive Byte Counter",
    "",
#endif
    { XUMAC_RDP_GRBYT_REG_OFFSET },
    0,
    0,
    1230,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GRBYT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GRBYT_UPPER, TYPE: Type_UPPER_16BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U16 *****/
const ru_field_rec XUMAC_RDP_GRBYT_UPPER_COUNT_U16_FIELD =
{
    "COUNT_U16",
#if RU_INCLUDE_DESC
    "",
    "Upper 16 bits of 48-bit counter.\n",
#endif
    { XUMAC_RDP_GRBYT_UPPER_COUNT_U16_FIELD_MASK },
    0,
    { XUMAC_RDP_GRBYT_UPPER_COUNT_U16_FIELD_WIDTH },
    { XUMAC_RDP_GRBYT_UPPER_COUNT_U16_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GRBYT_UPPER_FIELDS[] =
{
    &XUMAC_RDP_GRBYT_UPPER_COUNT_U16_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GRBYT_UPPER *****/
const ru_reg_rec XUMAC_RDP_GRBYT_UPPER_REG =
{
    "GRBYT_UPPER",
#if RU_INCLUDE_DESC
    "GRBYT upper 16 bits",
    "",
#endif
    { XUMAC_RDP_GRBYT_UPPER_REG_OFFSET },
    0,
    0,
    1231,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GRBYT_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GRMCA, TYPE: Type_MIB_GRMCA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_GRMCA_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Receive multicast packet counter.\n",
#endif
    { XUMAC_RDP_GRMCA_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_GRMCA_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_GRMCA_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GRMCA_FIELDS[] =
{
    &XUMAC_RDP_GRMCA_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GRMCA *****/
const ru_reg_rec XUMAC_RDP_GRMCA_REG =
{
    "GRMCA",
#if RU_INCLUDE_DESC
    "Receive Multicast Frame Counter",
    "",
#endif
    { XUMAC_RDP_GRMCA_REG_OFFSET },
    0,
    0,
    1232,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GRMCA_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GRMCA_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_GRMCA_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_GRMCA_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_GRMCA_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_GRMCA_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GRMCA_UPPER_FIELDS[] =
{
    &XUMAC_RDP_GRMCA_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GRMCA_UPPER *****/
const ru_reg_rec XUMAC_RDP_GRMCA_UPPER_REG =
{
    "GRMCA_UPPER",
#if RU_INCLUDE_DESC
    "GRMCA upper 8 bits",
    "",
#endif
    { XUMAC_RDP_GRMCA_UPPER_REG_OFFSET },
    0,
    0,
    1233,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GRMCA_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GRBCA, TYPE: Type_MIB_GRBCA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_GRBCA_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Receive broadcast packet counter.\n",
#endif
    { XUMAC_RDP_GRBCA_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_GRBCA_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_GRBCA_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GRBCA_FIELDS[] =
{
    &XUMAC_RDP_GRBCA_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GRBCA *****/
const ru_reg_rec XUMAC_RDP_GRBCA_REG =
{
    "GRBCA",
#if RU_INCLUDE_DESC
    "Receive Broadcast Frame Counter",
    "",
#endif
    { XUMAC_RDP_GRBCA_REG_OFFSET },
    0,
    0,
    1234,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GRBCA_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GRBCA_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_GRBCA_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_GRBCA_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_GRBCA_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_GRBCA_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GRBCA_UPPER_FIELDS[] =
{
    &XUMAC_RDP_GRBCA_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GRBCA_UPPER *****/
const ru_reg_rec XUMAC_RDP_GRBCA_UPPER_REG =
{
    "GRBCA_UPPER",
#if RU_INCLUDE_DESC
    "GRBCA upper 8 bits",
    "",
#endif
    { XUMAC_RDP_GRBCA_UPPER_REG_OFFSET },
    0,
    0,
    1235,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GRBCA_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GRFCS, TYPE: Type_MIB_GRFCS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_GRFCS_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Receive FCS error counter.\n",
#endif
    { XUMAC_RDP_GRFCS_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_GRFCS_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_GRFCS_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GRFCS_FIELDS[] =
{
    &XUMAC_RDP_GRFCS_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GRFCS *****/
const ru_reg_rec XUMAC_RDP_GRFCS_REG =
{
    "GRFCS",
#if RU_INCLUDE_DESC
    "Receive FCS Error Counter",
    "",
#endif
    { XUMAC_RDP_GRFCS_REG_OFFSET },
    0,
    0,
    1236,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GRFCS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GRFCS_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_GRFCS_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_GRFCS_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_GRFCS_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_GRFCS_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GRFCS_UPPER_FIELDS[] =
{
    &XUMAC_RDP_GRFCS_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GRFCS_UPPER *****/
const ru_reg_rec XUMAC_RDP_GRFCS_UPPER_REG =
{
    "GRFCS_UPPER",
#if RU_INCLUDE_DESC
    "GRFCS upper 8 bits",
    "",
#endif
    { XUMAC_RDP_GRFCS_UPPER_REG_OFFSET },
    0,
    0,
    1237,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GRFCS_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GRXCF, TYPE: Type_MIB_GRXCF
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_GRXCF_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Receive control frame packet counter.\n",
#endif
    { XUMAC_RDP_GRXCF_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_GRXCF_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_GRXCF_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GRXCF_FIELDS[] =
{
    &XUMAC_RDP_GRXCF_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GRXCF *****/
const ru_reg_rec XUMAC_RDP_GRXCF_REG =
{
    "GRXCF",
#if RU_INCLUDE_DESC
    "Receive Control Frame Packet Counter",
    "",
#endif
    { XUMAC_RDP_GRXCF_REG_OFFSET },
    0,
    0,
    1238,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GRXCF_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GRXCF_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_GRXCF_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_GRXCF_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_GRXCF_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_GRXCF_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GRXCF_UPPER_FIELDS[] =
{
    &XUMAC_RDP_GRXCF_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GRXCF_UPPER *****/
const ru_reg_rec XUMAC_RDP_GRXCF_UPPER_REG =
{
    "GRXCF_UPPER",
#if RU_INCLUDE_DESC
    "GRXCF upper 8 bits",
    "",
#endif
    { XUMAC_RDP_GRXCF_UPPER_REG_OFFSET },
    0,
    0,
    1239,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GRXCF_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GRXPF, TYPE: Type_MIB_GRXPF
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_GRXPF_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Receive pause frame packet counter.\n",
#endif
    { XUMAC_RDP_GRXPF_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_GRXPF_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_GRXPF_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GRXPF_FIELDS[] =
{
    &XUMAC_RDP_GRXPF_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GRXPF *****/
const ru_reg_rec XUMAC_RDP_GRXPF_REG =
{
    "GRXPF",
#if RU_INCLUDE_DESC
    "Receive Pause Frame Packet Counter",
    "",
#endif
    { XUMAC_RDP_GRXPF_REG_OFFSET },
    0,
    0,
    1240,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GRXPF_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GRXPF_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_GRXPF_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_GRXPF_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_GRXPF_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_GRXPF_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GRXPF_UPPER_FIELDS[] =
{
    &XUMAC_RDP_GRXPF_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GRXPF_UPPER *****/
const ru_reg_rec XUMAC_RDP_GRXPF_UPPER_REG =
{
    "GRXPF_UPPER",
#if RU_INCLUDE_DESC
    "GRXPF upper 8 bits",
    "",
#endif
    { XUMAC_RDP_GRXPF_UPPER_REG_OFFSET },
    0,
    0,
    1241,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GRXPF_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GRXUO, TYPE: Tpye_MIB_GRXUO
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_GRXUO_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Receive unknown op code packet counter.\n",
#endif
    { XUMAC_RDP_GRXUO_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_GRXUO_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_GRXUO_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GRXUO_FIELDS[] =
{
    &XUMAC_RDP_GRXUO_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GRXUO *****/
const ru_reg_rec XUMAC_RDP_GRXUO_REG =
{
    "GRXUO",
#if RU_INCLUDE_DESC
    "Receive Unknown OP Code Packet Counter",
    "",
#endif
    { XUMAC_RDP_GRXUO_REG_OFFSET },
    0,
    0,
    1242,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GRXUO_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GRXUO_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_GRXUO_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_GRXUO_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_GRXUO_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_GRXUO_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GRXUO_UPPER_FIELDS[] =
{
    &XUMAC_RDP_GRXUO_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GRXUO_UPPER *****/
const ru_reg_rec XUMAC_RDP_GRXUO_UPPER_REG =
{
    "GRXUO_UPPER",
#if RU_INCLUDE_DESC
    "GRXUO upper 8 bits",
    "",
#endif
    { XUMAC_RDP_GRXUO_UPPER_REG_OFFSET },
    0,
    0,
    1243,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GRXUO_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GRALN, TYPE: Type_MIB_GRALN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_GRALN_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Receive alignmenet error counter.\n",
#endif
    { XUMAC_RDP_GRALN_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_GRALN_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_GRALN_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GRALN_FIELDS[] =
{
    &XUMAC_RDP_GRALN_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GRALN *****/
const ru_reg_rec XUMAC_RDP_GRALN_REG =
{
    "GRALN",
#if RU_INCLUDE_DESC
    "Receive Alignmenet Error Counter",
    "",
#endif
    { XUMAC_RDP_GRALN_REG_OFFSET },
    0,
    0,
    1244,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GRALN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GRALN_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_GRALN_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_GRALN_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_GRALN_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_GRALN_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GRALN_UPPER_FIELDS[] =
{
    &XUMAC_RDP_GRALN_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GRALN_UPPER *****/
const ru_reg_rec XUMAC_RDP_GRALN_UPPER_REG =
{
    "GRALN_UPPER",
#if RU_INCLUDE_DESC
    "GRALN upper 8 bits",
    "",
#endif
    { XUMAC_RDP_GRALN_UPPER_REG_OFFSET },
    0,
    0,
    1245,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GRALN_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GRFLR, TYPE: Type_MIB_GRFLR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_GRFLR_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Receive frame length out of range counter.\n",
#endif
    { XUMAC_RDP_GRFLR_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_GRFLR_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_GRFLR_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GRFLR_FIELDS[] =
{
    &XUMAC_RDP_GRFLR_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GRFLR *****/
const ru_reg_rec XUMAC_RDP_GRFLR_REG =
{
    "GRFLR",
#if RU_INCLUDE_DESC
    "Receive Frame Length Out Of Range Counter",
    "",
#endif
    { XUMAC_RDP_GRFLR_REG_OFFSET },
    0,
    0,
    1246,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GRFLR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GRFLR_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_GRFLR_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_GRFLR_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_GRFLR_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_GRFLR_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GRFLR_UPPER_FIELDS[] =
{
    &XUMAC_RDP_GRFLR_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GRFLR_UPPER *****/
const ru_reg_rec XUMAC_RDP_GRFLR_UPPER_REG =
{
    "GRFLR_UPPER",
#if RU_INCLUDE_DESC
    "GRFLR upper 8 bits",
    "",
#endif
    { XUMAC_RDP_GRFLR_UPPER_REG_OFFSET },
    0,
    0,
    1247,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GRFLR_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GRCDE, TYPE: Type_MIB_GRCDE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_GRCDE_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Receive code error packet counter.\n",
#endif
    { XUMAC_RDP_GRCDE_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_GRCDE_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_GRCDE_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GRCDE_FIELDS[] =
{
    &XUMAC_RDP_GRCDE_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GRCDE *****/
const ru_reg_rec XUMAC_RDP_GRCDE_REG =
{
    "GRCDE",
#if RU_INCLUDE_DESC
    "Receive Code Error Packet Counter",
    "",
#endif
    { XUMAC_RDP_GRCDE_REG_OFFSET },
    0,
    0,
    1248,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GRCDE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GRCDE_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_GRCDE_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_GRCDE_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_GRCDE_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_GRCDE_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GRCDE_UPPER_FIELDS[] =
{
    &XUMAC_RDP_GRCDE_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GRCDE_UPPER *****/
const ru_reg_rec XUMAC_RDP_GRCDE_UPPER_REG =
{
    "GRCDE_UPPER",
#if RU_INCLUDE_DESC
    "GRCDE upper 8 bits",
    "",
#endif
    { XUMAC_RDP_GRCDE_UPPER_REG_OFFSET },
    0,
    0,
    1249,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GRCDE_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GRFCR, TYPE: Type_MIB_GRFCR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_GRFCR_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Receive carrier sense error packet counter.\n",
#endif
    { XUMAC_RDP_GRFCR_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_GRFCR_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_GRFCR_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GRFCR_FIELDS[] =
{
    &XUMAC_RDP_GRFCR_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GRFCR *****/
const ru_reg_rec XUMAC_RDP_GRFCR_REG =
{
    "GRFCR",
#if RU_INCLUDE_DESC
    "Receive Carrier Sense Error Packet Counter",
    "",
#endif
    { XUMAC_RDP_GRFCR_REG_OFFSET },
    0,
    0,
    1250,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GRFCR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GRFCR_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_GRFCR_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_GRFCR_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_GRFCR_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_GRFCR_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GRFCR_UPPER_FIELDS[] =
{
    &XUMAC_RDP_GRFCR_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GRFCR_UPPER *****/
const ru_reg_rec XUMAC_RDP_GRFCR_UPPER_REG =
{
    "GRFCR_UPPER",
#if RU_INCLUDE_DESC
    "GRFCR upper 8 bits",
    "",
#endif
    { XUMAC_RDP_GRFCR_UPPER_REG_OFFSET },
    0,
    0,
    1251,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GRFCR_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GROVR, TYPE: Type_MIB_GROVR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_GROVR_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Receive oversize packet counter.\n",
#endif
    { XUMAC_RDP_GROVR_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_GROVR_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_GROVR_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GROVR_FIELDS[] =
{
    &XUMAC_RDP_GROVR_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GROVR *****/
const ru_reg_rec XUMAC_RDP_GROVR_REG =
{
    "GROVR",
#if RU_INCLUDE_DESC
    "Receive Oversize Packet Counter",
    "",
#endif
    { XUMAC_RDP_GROVR_REG_OFFSET },
    0,
    0,
    1252,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GROVR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GROVR_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_GROVR_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_GROVR_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_GROVR_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_GROVR_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GROVR_UPPER_FIELDS[] =
{
    &XUMAC_RDP_GROVR_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GROVR_UPPER *****/
const ru_reg_rec XUMAC_RDP_GROVR_UPPER_REG =
{
    "GROVR_UPPER",
#if RU_INCLUDE_DESC
    "GROVR upper 8 bits",
    "",
#endif
    { XUMAC_RDP_GROVR_UPPER_REG_OFFSET },
    0,
    0,
    1253,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GROVR_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GRJBR, TYPE: Type_MIB_GRJBR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_GRJBR_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Receive jabber counter.\n",
#endif
    { XUMAC_RDP_GRJBR_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_GRJBR_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_GRJBR_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GRJBR_FIELDS[] =
{
    &XUMAC_RDP_GRJBR_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GRJBR *****/
const ru_reg_rec XUMAC_RDP_GRJBR_REG =
{
    "GRJBR",
#if RU_INCLUDE_DESC
    "Receive Jabber Counter",
    "",
#endif
    { XUMAC_RDP_GRJBR_REG_OFFSET },
    0,
    0,
    1254,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GRJBR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GRJBR_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_GRJBR_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_GRJBR_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_GRJBR_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_GRJBR_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GRJBR_UPPER_FIELDS[] =
{
    &XUMAC_RDP_GRJBR_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GRJBR_UPPER *****/
const ru_reg_rec XUMAC_RDP_GRJBR_UPPER_REG =
{
    "GRJBR_UPPER",
#if RU_INCLUDE_DESC
    "GRJBR upper 8 bits",
    "",
#endif
    { XUMAC_RDP_GRJBR_UPPER_REG_OFFSET },
    0,
    0,
    1255,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GRJBR_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GRMTUE, TYPE: Type_MIB_GRMTUE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_GRMTUE_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Receive MTU error packet counter.\n",
#endif
    { XUMAC_RDP_GRMTUE_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_GRMTUE_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_GRMTUE_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GRMTUE_FIELDS[] =
{
    &XUMAC_RDP_GRMTUE_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GRMTUE *****/
const ru_reg_rec XUMAC_RDP_GRMTUE_REG =
{
    "GRMTUE",
#if RU_INCLUDE_DESC
    "Receive MTU Error Packet Counter",
    "",
#endif
    { XUMAC_RDP_GRMTUE_REG_OFFSET },
    0,
    0,
    1256,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GRMTUE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GRMTUE_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_GRMTUE_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_GRMTUE_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_GRMTUE_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_GRMTUE_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GRMTUE_UPPER_FIELDS[] =
{
    &XUMAC_RDP_GRMTUE_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GRMTUE_UPPER *****/
const ru_reg_rec XUMAC_RDP_GRMTUE_UPPER_REG =
{
    "GRMTUE_UPPER",
#if RU_INCLUDE_DESC
    "GRMTUE upper 8 bits",
    "",
#endif
    { XUMAC_RDP_GRMTUE_UPPER_REG_OFFSET },
    0,
    0,
    1257,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GRMTUE_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GRPOK, TYPE: Type_MIB_GRPOK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_GRPOK_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Receive good packet counter.\n",
#endif
    { XUMAC_RDP_GRPOK_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_GRPOK_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_GRPOK_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GRPOK_FIELDS[] =
{
    &XUMAC_RDP_GRPOK_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GRPOK *****/
const ru_reg_rec XUMAC_RDP_GRPOK_REG =
{
    "GRPOK",
#if RU_INCLUDE_DESC
    "Receive Good Packet Counter",
    "",
#endif
    { XUMAC_RDP_GRPOK_REG_OFFSET },
    0,
    0,
    1258,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GRPOK_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GRPOK_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_GRPOK_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_GRPOK_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_GRPOK_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_GRPOK_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GRPOK_UPPER_FIELDS[] =
{
    &XUMAC_RDP_GRPOK_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GRPOK_UPPER *****/
const ru_reg_rec XUMAC_RDP_GRPOK_UPPER_REG =
{
    "GRPOK_UPPER",
#if RU_INCLUDE_DESC
    "GRPOK upper 8 bits",
    "",
#endif
    { XUMAC_RDP_GRPOK_UPPER_REG_OFFSET },
    0,
    0,
    1259,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GRPOK_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GRUC, TYPE: Type_MIB_GRUC
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_GRUC_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Received unicast packet counter.\n",
#endif
    { XUMAC_RDP_GRUC_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_GRUC_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_GRUC_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GRUC_FIELDS[] =
{
    &XUMAC_RDP_GRUC_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GRUC *****/
const ru_reg_rec XUMAC_RDP_GRUC_REG =
{
    "GRUC",
#if RU_INCLUDE_DESC
    "Receive Unicast Packet Counter",
    "",
#endif
    { XUMAC_RDP_GRUC_REG_OFFSET },
    0,
    0,
    1260,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GRUC_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GRUC_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_GRUC_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_GRUC_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_GRUC_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_GRUC_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GRUC_UPPER_FIELDS[] =
{
    &XUMAC_RDP_GRUC_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GRUC_UPPER *****/
const ru_reg_rec XUMAC_RDP_GRUC_UPPER_REG =
{
    "GRUC_UPPER",
#if RU_INCLUDE_DESC
    "GRUC upper 8 bits",
    "",
#endif
    { XUMAC_RDP_GRUC_UPPER_REG_OFFSET },
    0,
    0,
    1261,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GRUC_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GRPPP, TYPE: Type_MIB_GRPPP
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_GRPPP_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Receive PPP packet counter.\n",
#endif
    { XUMAC_RDP_GRPPP_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_GRPPP_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_GRPPP_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GRPPP_FIELDS[] =
{
    &XUMAC_RDP_GRPPP_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GRPPP *****/
const ru_reg_rec XUMAC_RDP_GRPPP_REG =
{
    "GRPPP",
#if RU_INCLUDE_DESC
    "Receive PPP Packet Counter",
    "",
#endif
    { XUMAC_RDP_GRPPP_REG_OFFSET },
    0,
    0,
    1262,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GRPPP_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GRPPP_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_GRPPP_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_GRPPP_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_GRPPP_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_GRPPP_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GRPPP_UPPER_FIELDS[] =
{
    &XUMAC_RDP_GRPPP_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GRPPP_UPPER *****/
const ru_reg_rec XUMAC_RDP_GRPPP_UPPER_REG =
{
    "GRPPP_UPPER",
#if RU_INCLUDE_DESC
    "GRPPP upper 8 bits",
    "",
#endif
    { XUMAC_RDP_GRPPP_UPPER_REG_OFFSET },
    0,
    0,
    1263,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GRPPP_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GRCRC, TYPE: Type_MIB_GRCRC
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_GRCRC_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Receive CRC match packet counter.\n",
#endif
    { XUMAC_RDP_GRCRC_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_GRCRC_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_GRCRC_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GRCRC_FIELDS[] =
{
    &XUMAC_RDP_GRCRC_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GRCRC *****/
const ru_reg_rec XUMAC_RDP_GRCRC_REG =
{
    "GRCRC",
#if RU_INCLUDE_DESC
    "Receive CRC Match Packet Counter",
    "",
#endif
    { XUMAC_RDP_GRCRC_REG_OFFSET },
    0,
    0,
    1264,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GRCRC_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GRCRC_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_GRCRC_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_GRCRC_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_GRCRC_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_GRCRC_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GRCRC_UPPER_FIELDS[] =
{
    &XUMAC_RDP_GRCRC_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GRCRC_UPPER *****/
const ru_reg_rec XUMAC_RDP_GRCRC_UPPER_REG =
{
    "GRCRC_UPPER",
#if RU_INCLUDE_DESC
    "GRCRC upper 8 bits",
    "",
#endif
    { XUMAC_RDP_GRCRC_UPPER_REG_OFFSET },
    0,
    0,
    1265,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GRCRC_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_TR64, TYPE: Type_MIB_TR64
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_TR64_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Transmit 64 bytes frame counter.\n",
#endif
    { XUMAC_RDP_TR64_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_TR64_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_TR64_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_TR64_FIELDS[] =
{
    &XUMAC_RDP_TR64_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_TR64 *****/
const ru_reg_rec XUMAC_RDP_TR64_REG =
{
    "TR64",
#if RU_INCLUDE_DESC
    "Transmit 64B Frame Counter",
    "",
#endif
    { XUMAC_RDP_TR64_REG_OFFSET },
    0,
    0,
    1266,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_TR64_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_TR64_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_TR64_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_TR64_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_TR64_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_TR64_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_TR64_UPPER_FIELDS[] =
{
    &XUMAC_RDP_TR64_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_TR64_UPPER *****/
const ru_reg_rec XUMAC_RDP_TR64_UPPER_REG =
{
    "TR64_UPPER",
#if RU_INCLUDE_DESC
    "TR64 upper 8 bits",
    "",
#endif
    { XUMAC_RDP_TR64_UPPER_REG_OFFSET },
    0,
    0,
    1267,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_TR64_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_TR127, TYPE: Type_MIB_TR127
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_TR127_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Transmit 65 bytes to 127 bytes frame counter.\n",
#endif
    { XUMAC_RDP_TR127_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_TR127_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_TR127_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_TR127_FIELDS[] =
{
    &XUMAC_RDP_TR127_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_TR127 *****/
const ru_reg_rec XUMAC_RDP_TR127_REG =
{
    "TR127",
#if RU_INCLUDE_DESC
    "Transmit 65B to 127B Frame Counter",
    "",
#endif
    { XUMAC_RDP_TR127_REG_OFFSET },
    0,
    0,
    1268,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_TR127_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_TR127_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_TR127_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_TR127_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_TR127_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_TR127_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_TR127_UPPER_FIELDS[] =
{
    &XUMAC_RDP_TR127_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_TR127_UPPER *****/
const ru_reg_rec XUMAC_RDP_TR127_UPPER_REG =
{
    "TR127_UPPER",
#if RU_INCLUDE_DESC
    "TR127 upper 8 bits",
    "",
#endif
    { XUMAC_RDP_TR127_UPPER_REG_OFFSET },
    0,
    0,
    1269,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_TR127_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_TR255, TYPE: Type_MIB_TR255
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_TR255_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Transmit 128 bytes to 255 bytes frame counter.\n",
#endif
    { XUMAC_RDP_TR255_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_TR255_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_TR255_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_TR255_FIELDS[] =
{
    &XUMAC_RDP_TR255_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_TR255 *****/
const ru_reg_rec XUMAC_RDP_TR255_REG =
{
    "TR255",
#if RU_INCLUDE_DESC
    "Transmit 128B to 255B Frame Counter",
    "",
#endif
    { XUMAC_RDP_TR255_REG_OFFSET },
    0,
    0,
    1270,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_TR255_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_TR255_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_TR255_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_TR255_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_TR255_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_TR255_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_TR255_UPPER_FIELDS[] =
{
    &XUMAC_RDP_TR255_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_TR255_UPPER *****/
const ru_reg_rec XUMAC_RDP_TR255_UPPER_REG =
{
    "TR255_UPPER",
#if RU_INCLUDE_DESC
    "TR255 upper 8 bits",
    "",
#endif
    { XUMAC_RDP_TR255_UPPER_REG_OFFSET },
    0,
    0,
    1271,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_TR255_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_TR511, TYPE: Type_MIB_TR511
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_TR511_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Transmit 256 bytes to 511 bytes frame counter.\n",
#endif
    { XUMAC_RDP_TR511_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_TR511_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_TR511_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_TR511_FIELDS[] =
{
    &XUMAC_RDP_TR511_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_TR511 *****/
const ru_reg_rec XUMAC_RDP_TR511_REG =
{
    "TR511",
#if RU_INCLUDE_DESC
    "Transmit 256B to 511B Frame Counter",
    "",
#endif
    { XUMAC_RDP_TR511_REG_OFFSET },
    0,
    0,
    1272,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_TR511_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_TR511_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_TR511_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_TR511_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_TR511_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_TR511_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_TR511_UPPER_FIELDS[] =
{
    &XUMAC_RDP_TR511_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_TR511_UPPER *****/
const ru_reg_rec XUMAC_RDP_TR511_UPPER_REG =
{
    "TR511_UPPER",
#if RU_INCLUDE_DESC
    "TR511 upper 8 bits",
    "",
#endif
    { XUMAC_RDP_TR511_UPPER_REG_OFFSET },
    0,
    0,
    1273,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_TR511_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_TR1023, TYPE: Type_MIB_TR1023
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_TR1023_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Transmit 512 bytes to 1023 bytes frame counter.\n",
#endif
    { XUMAC_RDP_TR1023_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_TR1023_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_TR1023_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_TR1023_FIELDS[] =
{
    &XUMAC_RDP_TR1023_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_TR1023 *****/
const ru_reg_rec XUMAC_RDP_TR1023_REG =
{
    "TR1023",
#if RU_INCLUDE_DESC
    "Transmit 512B to 1023B Frame Counter",
    "",
#endif
    { XUMAC_RDP_TR1023_REG_OFFSET },
    0,
    0,
    1274,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_TR1023_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_TR1023_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_TR1023_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_TR1023_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_TR1023_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_TR1023_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_TR1023_UPPER_FIELDS[] =
{
    &XUMAC_RDP_TR1023_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_TR1023_UPPER *****/
const ru_reg_rec XUMAC_RDP_TR1023_UPPER_REG =
{
    "TR1023_UPPER",
#if RU_INCLUDE_DESC
    "TR1023 upper 8 bits",
    "",
#endif
    { XUMAC_RDP_TR1023_UPPER_REG_OFFSET },
    0,
    0,
    1275,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_TR1023_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_TR1518, TYPE: Type_MIB_TR1518
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_TR1518_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Transmit 1024 bytes to 1518 bytes frame counter.\n",
#endif
    { XUMAC_RDP_TR1518_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_TR1518_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_TR1518_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_TR1518_FIELDS[] =
{
    &XUMAC_RDP_TR1518_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_TR1518 *****/
const ru_reg_rec XUMAC_RDP_TR1518_REG =
{
    "TR1518",
#if RU_INCLUDE_DESC
    "Transmit 1024B to 1518B Frame Counter",
    "",
#endif
    { XUMAC_RDP_TR1518_REG_OFFSET },
    0,
    0,
    1276,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_TR1518_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_TR1518_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_TR1518_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_TR1518_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_TR1518_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_TR1518_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_TR1518_UPPER_FIELDS[] =
{
    &XUMAC_RDP_TR1518_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_TR1518_UPPER *****/
const ru_reg_rec XUMAC_RDP_TR1518_UPPER_REG =
{
    "TR1518_UPPER",
#if RU_INCLUDE_DESC
    "TR1518 upper 8 bits",
    "",
#endif
    { XUMAC_RDP_TR1518_UPPER_REG_OFFSET },
    0,
    0,
    1277,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_TR1518_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_TRMGV, TYPE: Type_MIB_TRMGV
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_TRMGV_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Transmit 1519 bytes to 1522 bytes good VLAN frame counter.\n",
#endif
    { XUMAC_RDP_TRMGV_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_TRMGV_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_TRMGV_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_TRMGV_FIELDS[] =
{
    &XUMAC_RDP_TRMGV_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_TRMGV *****/
const ru_reg_rec XUMAC_RDP_TRMGV_REG =
{
    "TRMGV",
#if RU_INCLUDE_DESC
    "Transmit 1519B to 1522B Good VLAN Frame Counter",
    "",
#endif
    { XUMAC_RDP_TRMGV_REG_OFFSET },
    0,
    0,
    1278,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_TRMGV_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_TRMGV_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_TRMGV_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_TRMGV_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_TRMGV_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_TRMGV_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_TRMGV_UPPER_FIELDS[] =
{
    &XUMAC_RDP_TRMGV_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_TRMGV_UPPER *****/
const ru_reg_rec XUMAC_RDP_TRMGV_UPPER_REG =
{
    "TRMGV_UPPER",
#if RU_INCLUDE_DESC
    "TRMGV upper 8 bits",
    "",
#endif
    { XUMAC_RDP_TRMGV_UPPER_REG_OFFSET },
    0,
    0,
    1279,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_TRMGV_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_TR2047, TYPE: Type_MIB_TR2047
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_TR2047_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Transmit 1519 bytes to 2047 bytes Frame Counter.\n",
#endif
    { XUMAC_RDP_TR2047_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_TR2047_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_TR2047_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_TR2047_FIELDS[] =
{
    &XUMAC_RDP_TR2047_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_TR2047 *****/
const ru_reg_rec XUMAC_RDP_TR2047_REG =
{
    "TR2047",
#if RU_INCLUDE_DESC
    "Transmit 1519B to 2047B Frame Counter",
    "",
#endif
    { XUMAC_RDP_TR2047_REG_OFFSET },
    0,
    0,
    1280,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_TR2047_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_TR2047_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_TR2047_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_TR2047_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_TR2047_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_TR2047_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_TR2047_UPPER_FIELDS[] =
{
    &XUMAC_RDP_TR2047_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_TR2047_UPPER *****/
const ru_reg_rec XUMAC_RDP_TR2047_UPPER_REG =
{
    "TR2047_UPPER",
#if RU_INCLUDE_DESC
    "TR2047 upper 8 bits",
    "",
#endif
    { XUMAC_RDP_TR2047_UPPER_REG_OFFSET },
    0,
    0,
    1281,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_TR2047_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_TR4095, TYPE: Type_MIB_TR4095
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_TR4095_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Transmit 2048 bytes to 4095 bytes frame counter.\n",
#endif
    { XUMAC_RDP_TR4095_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_TR4095_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_TR4095_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_TR4095_FIELDS[] =
{
    &XUMAC_RDP_TR4095_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_TR4095 *****/
const ru_reg_rec XUMAC_RDP_TR4095_REG =
{
    "TR4095",
#if RU_INCLUDE_DESC
    "Transmit 2048B to 4095B Frame Counter",
    "",
#endif
    { XUMAC_RDP_TR4095_REG_OFFSET },
    0,
    0,
    1282,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_TR4095_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_TR4095_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_TR4095_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_TR4095_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_TR4095_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_TR4095_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_TR4095_UPPER_FIELDS[] =
{
    &XUMAC_RDP_TR4095_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_TR4095_UPPER *****/
const ru_reg_rec XUMAC_RDP_TR4095_UPPER_REG =
{
    "TR4095_UPPER",
#if RU_INCLUDE_DESC
    "TR4095 upper 8 bits",
    "",
#endif
    { XUMAC_RDP_TR4095_UPPER_REG_OFFSET },
    0,
    0,
    1283,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_TR4095_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_TR9216, TYPE: Type_MIB_TR9216
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_TR9216_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Transmit 4096 bytes to 9216 bytes Frame Counter.\n",
#endif
    { XUMAC_RDP_TR9216_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_TR9216_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_TR9216_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_TR9216_FIELDS[] =
{
    &XUMAC_RDP_TR9216_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_TR9216 *****/
const ru_reg_rec XUMAC_RDP_TR9216_REG =
{
    "TR9216",
#if RU_INCLUDE_DESC
    "Transmit 4096B to 9216B Frame Counter",
    "",
#endif
    { XUMAC_RDP_TR9216_REG_OFFSET },
    0,
    0,
    1284,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_TR9216_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_TR9216_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_TR9216_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_TR9216_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_TR9216_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_TR9216_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_TR9216_UPPER_FIELDS[] =
{
    &XUMAC_RDP_TR9216_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_TR9216_UPPER *****/
const ru_reg_rec XUMAC_RDP_TR9216_UPPER_REG =
{
    "TR9216_UPPER",
#if RU_INCLUDE_DESC
    "TR9216 upper 8 bits",
    "",
#endif
    { XUMAC_RDP_TR9216_UPPER_REG_OFFSET },
    0,
    0,
    1285,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_TR9216_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GTPKT, TYPE: Type_MIB_GTPKT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_GTPKT_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Transmit packet counter.\n",
#endif
    { XUMAC_RDP_GTPKT_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_GTPKT_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_GTPKT_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GTPKT_FIELDS[] =
{
    &XUMAC_RDP_GTPKT_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GTPKT *****/
const ru_reg_rec XUMAC_RDP_GTPKT_REG =
{
    "GTPKT",
#if RU_INCLUDE_DESC
    "Transmit Packet Counter",
    "",
#endif
    { XUMAC_RDP_GTPKT_REG_OFFSET },
    0,
    0,
    1286,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GTPKT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GTPKT_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_GTPKT_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_GTPKT_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_GTPKT_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_GTPKT_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GTPKT_UPPER_FIELDS[] =
{
    &XUMAC_RDP_GTPKT_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GTPKT_UPPER *****/
const ru_reg_rec XUMAC_RDP_GTPKT_UPPER_REG =
{
    "GTPKT_UPPER",
#if RU_INCLUDE_DESC
    "GTPKT upper 8 bits",
    "",
#endif
    { XUMAC_RDP_GTPKT_UPPER_REG_OFFSET },
    0,
    0,
    1287,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GTPKT_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GTMCA, TYPE: Type_MIB_GTMCA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_GTMCA_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Transmit multicast packet counter.\n",
#endif
    { XUMAC_RDP_GTMCA_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_GTMCA_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_GTMCA_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GTMCA_FIELDS[] =
{
    &XUMAC_RDP_GTMCA_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GTMCA *****/
const ru_reg_rec XUMAC_RDP_GTMCA_REG =
{
    "GTMCA",
#if RU_INCLUDE_DESC
    "Transmit Multicast Packet Counter",
    "",
#endif
    { XUMAC_RDP_GTMCA_REG_OFFSET },
    0,
    0,
    1288,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GTMCA_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GTMCA_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_GTMCA_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_GTMCA_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_GTMCA_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_GTMCA_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GTMCA_UPPER_FIELDS[] =
{
    &XUMAC_RDP_GTMCA_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GTMCA_UPPER *****/
const ru_reg_rec XUMAC_RDP_GTMCA_UPPER_REG =
{
    "GTMCA_UPPER",
#if RU_INCLUDE_DESC
    "GTMCA upper 8 bits",
    "",
#endif
    { XUMAC_RDP_GTMCA_UPPER_REG_OFFSET },
    0,
    0,
    1289,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GTMCA_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GTBCA, TYPE: Type_MIB_GTBCA
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_GTBCA_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Transmit broadcast packet counter.\n",
#endif
    { XUMAC_RDP_GTBCA_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_GTBCA_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_GTBCA_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GTBCA_FIELDS[] =
{
    &XUMAC_RDP_GTBCA_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GTBCA *****/
const ru_reg_rec XUMAC_RDP_GTBCA_REG =
{
    "GTBCA",
#if RU_INCLUDE_DESC
    "Transmit Broadcast Packet Counter",
    "",
#endif
    { XUMAC_RDP_GTBCA_REG_OFFSET },
    0,
    0,
    1290,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GTBCA_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GTBCA_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_GTBCA_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_GTBCA_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_GTBCA_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_GTBCA_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GTBCA_UPPER_FIELDS[] =
{
    &XUMAC_RDP_GTBCA_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GTBCA_UPPER *****/
const ru_reg_rec XUMAC_RDP_GTBCA_UPPER_REG =
{
    "GTBCA_UPPER",
#if RU_INCLUDE_DESC
    "GTBCA upper 8 bits",
    "",
#endif
    { XUMAC_RDP_GTBCA_UPPER_REG_OFFSET },
    0,
    0,
    1291,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GTBCA_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GTXPF, TYPE: Type_MIB_GTXPF
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_GTXPF_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Transmit pause frame packet counter.\n",
#endif
    { XUMAC_RDP_GTXPF_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_GTXPF_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_GTXPF_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GTXPF_FIELDS[] =
{
    &XUMAC_RDP_GTXPF_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GTXPF *****/
const ru_reg_rec XUMAC_RDP_GTXPF_REG =
{
    "GTXPF",
#if RU_INCLUDE_DESC
    "Transmit Pause Frame Packet Counter",
    "",
#endif
    { XUMAC_RDP_GTXPF_REG_OFFSET },
    0,
    0,
    1292,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GTXPF_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GTXPF_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_GTXPF_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_GTXPF_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_GTXPF_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_GTXPF_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GTXPF_UPPER_FIELDS[] =
{
    &XUMAC_RDP_GTXPF_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GTXPF_UPPER *****/
const ru_reg_rec XUMAC_RDP_GTXPF_UPPER_REG =
{
    "GTXPF_UPPER",
#if RU_INCLUDE_DESC
    "GTXPF upper 8 bits",
    "",
#endif
    { XUMAC_RDP_GTXPF_UPPER_REG_OFFSET },
    0,
    0,
    1293,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GTXPF_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GTXCF, TYPE: Type_MIB_GTXCF
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_GTXCF_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Transmit control frame packet counter.\n",
#endif
    { XUMAC_RDP_GTXCF_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_GTXCF_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_GTXCF_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GTXCF_FIELDS[] =
{
    &XUMAC_RDP_GTXCF_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GTXCF *****/
const ru_reg_rec XUMAC_RDP_GTXCF_REG =
{
    "GTXCF",
#if RU_INCLUDE_DESC
    "Transmit Control Frame Packet Counter",
    "",
#endif
    { XUMAC_RDP_GTXCF_REG_OFFSET },
    0,
    0,
    1294,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GTXCF_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GTXCF_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_GTXCF_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_GTXCF_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_GTXCF_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_GTXCF_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GTXCF_UPPER_FIELDS[] =
{
    &XUMAC_RDP_GTXCF_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GTXCF_UPPER *****/
const ru_reg_rec XUMAC_RDP_GTXCF_UPPER_REG =
{
    "GTXCF_UPPER",
#if RU_INCLUDE_DESC
    "GTXCF upper 8 bits",
    "",
#endif
    { XUMAC_RDP_GTXCF_UPPER_REG_OFFSET },
    0,
    0,
    1295,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GTXCF_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GTFCS, TYPE: Type_MIB_GTFCS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_GTFCS_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Transmit FCS error counter.\n",
#endif
    { XUMAC_RDP_GTFCS_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_GTFCS_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_GTFCS_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GTFCS_FIELDS[] =
{
    &XUMAC_RDP_GTFCS_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GTFCS *****/
const ru_reg_rec XUMAC_RDP_GTFCS_REG =
{
    "GTFCS",
#if RU_INCLUDE_DESC
    "Transmit FCS Error Counter",
    "",
#endif
    { XUMAC_RDP_GTFCS_REG_OFFSET },
    0,
    0,
    1296,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GTFCS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GTFCS_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_GTFCS_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_GTFCS_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_GTFCS_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_GTFCS_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GTFCS_UPPER_FIELDS[] =
{
    &XUMAC_RDP_GTFCS_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GTFCS_UPPER *****/
const ru_reg_rec XUMAC_RDP_GTFCS_UPPER_REG =
{
    "GTFCS_UPPER",
#if RU_INCLUDE_DESC
    "GTFCS upper 8 bits",
    "",
#endif
    { XUMAC_RDP_GTFCS_UPPER_REG_OFFSET },
    0,
    0,
    1297,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GTFCS_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GTOVR, TYPE: Type_MIB_GTOVR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_GTOVR_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Transmit oversize packet counter.\n",
#endif
    { XUMAC_RDP_GTOVR_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_GTOVR_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_GTOVR_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GTOVR_FIELDS[] =
{
    &XUMAC_RDP_GTOVR_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GTOVR *****/
const ru_reg_rec XUMAC_RDP_GTOVR_REG =
{
    "GTOVR",
#if RU_INCLUDE_DESC
    "Transmit Oversize Packet Counter",
    "",
#endif
    { XUMAC_RDP_GTOVR_REG_OFFSET },
    0,
    0,
    1298,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GTOVR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GTOVR_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_GTOVR_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_GTOVR_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_GTOVR_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_GTOVR_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GTOVR_UPPER_FIELDS[] =
{
    &XUMAC_RDP_GTOVR_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GTOVR_UPPER *****/
const ru_reg_rec XUMAC_RDP_GTOVR_UPPER_REG =
{
    "GTOVR_UPPER",
#if RU_INCLUDE_DESC
    "GTOVR upper 8 bits",
    "",
#endif
    { XUMAC_RDP_GTOVR_UPPER_REG_OFFSET },
    0,
    0,
    1299,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GTOVR_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GTDRF, TYPE: Type_MIB_GTDRF
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_GTDRF_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Transmit deferral packet counter.\n",
#endif
    { XUMAC_RDP_GTDRF_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_GTDRF_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_GTDRF_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GTDRF_FIELDS[] =
{
    &XUMAC_RDP_GTDRF_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GTDRF *****/
const ru_reg_rec XUMAC_RDP_GTDRF_REG =
{
    "GTDRF",
#if RU_INCLUDE_DESC
    "Transmit Deferral Packet Counter",
    "",
#endif
    { XUMAC_RDP_GTDRF_REG_OFFSET },
    0,
    0,
    1300,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GTDRF_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GTDRF_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_GTDRF_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_GTDRF_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_GTDRF_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_GTDRF_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GTDRF_UPPER_FIELDS[] =
{
    &XUMAC_RDP_GTDRF_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GTDRF_UPPER *****/
const ru_reg_rec XUMAC_RDP_GTDRF_UPPER_REG =
{
    "GTDRF_UPPER",
#if RU_INCLUDE_DESC
    "GTDRF upper 8 bits",
    "",
#endif
    { XUMAC_RDP_GTDRF_UPPER_REG_OFFSET },
    0,
    0,
    1301,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GTDRF_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GTEDF, TYPE: Type_MIB_GTEDF
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_GTEDF_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Transmit excessive deferral packet counter.\n",
#endif
    { XUMAC_RDP_GTEDF_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_GTEDF_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_GTEDF_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GTEDF_FIELDS[] =
{
    &XUMAC_RDP_GTEDF_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GTEDF *****/
const ru_reg_rec XUMAC_RDP_GTEDF_REG =
{
    "GTEDF",
#if RU_INCLUDE_DESC
    "Transmit Excessive Deferral Packet Counter",
    "",
#endif
    { XUMAC_RDP_GTEDF_REG_OFFSET },
    0,
    0,
    1302,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GTEDF_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GTEDF_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_GTEDF_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_GTEDF_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_GTEDF_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_GTEDF_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GTEDF_UPPER_FIELDS[] =
{
    &XUMAC_RDP_GTEDF_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GTEDF_UPPER *****/
const ru_reg_rec XUMAC_RDP_GTEDF_UPPER_REG =
{
    "GTEDF_UPPER",
#if RU_INCLUDE_DESC
    "GTEDF upper 8 bits",
    "",
#endif
    { XUMAC_RDP_GTEDF_UPPER_REG_OFFSET },
    0,
    0,
    1303,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GTEDF_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GTSCL, TYPE: Type_MIB_GTSCL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_GTSCL_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Transmit single collision packet counter.\n",
#endif
    { XUMAC_RDP_GTSCL_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_GTSCL_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_GTSCL_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GTSCL_FIELDS[] =
{
    &XUMAC_RDP_GTSCL_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GTSCL *****/
const ru_reg_rec XUMAC_RDP_GTSCL_REG =
{
    "GTSCL",
#if RU_INCLUDE_DESC
    "Transmit Single Collision Packet Counter",
    "",
#endif
    { XUMAC_RDP_GTSCL_REG_OFFSET },
    0,
    0,
    1304,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GTSCL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GTSCL_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_GTSCL_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_GTSCL_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_GTSCL_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_GTSCL_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GTSCL_UPPER_FIELDS[] =
{
    &XUMAC_RDP_GTSCL_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GTSCL_UPPER *****/
const ru_reg_rec XUMAC_RDP_GTSCL_UPPER_REG =
{
    "GTSCL_UPPER",
#if RU_INCLUDE_DESC
    "GTSCL upper 8 bits",
    "",
#endif
    { XUMAC_RDP_GTSCL_UPPER_REG_OFFSET },
    0,
    0,
    1305,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GTSCL_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GTMCL, TYPE: Type_MIB_GTMCL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_GTMCL_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Transmit multiple collision packet counter.\n",
#endif
    { XUMAC_RDP_GTMCL_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_GTMCL_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_GTMCL_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GTMCL_FIELDS[] =
{
    &XUMAC_RDP_GTMCL_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GTMCL *****/
const ru_reg_rec XUMAC_RDP_GTMCL_REG =
{
    "GTMCL",
#if RU_INCLUDE_DESC
    "Transmit Multiple Collision Packet Counter",
    "",
#endif
    { XUMAC_RDP_GTMCL_REG_OFFSET },
    0,
    0,
    1306,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GTMCL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GTMCL_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_GTMCL_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_GTMCL_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_GTMCL_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_GTMCL_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GTMCL_UPPER_FIELDS[] =
{
    &XUMAC_RDP_GTMCL_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GTMCL_UPPER *****/
const ru_reg_rec XUMAC_RDP_GTMCL_UPPER_REG =
{
    "GTMCL_UPPER",
#if RU_INCLUDE_DESC
    "GTMCL upper 8 bits",
    "",
#endif
    { XUMAC_RDP_GTMCL_UPPER_REG_OFFSET },
    0,
    0,
    1307,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GTMCL_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GTLCL, TYPE: Type_MIB_GTLCL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_GTLCL_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Transmit late collision packet counter.\n",
#endif
    { XUMAC_RDP_GTLCL_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_GTLCL_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_GTLCL_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GTLCL_FIELDS[] =
{
    &XUMAC_RDP_GTLCL_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GTLCL *****/
const ru_reg_rec XUMAC_RDP_GTLCL_REG =
{
    "GTLCL",
#if RU_INCLUDE_DESC
    "Transmit Late Collision Packet Counter",
    "",
#endif
    { XUMAC_RDP_GTLCL_REG_OFFSET },
    0,
    0,
    1308,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GTLCL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GTLCL_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_GTLCL_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_GTLCL_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_GTLCL_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_GTLCL_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GTLCL_UPPER_FIELDS[] =
{
    &XUMAC_RDP_GTLCL_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GTLCL_UPPER *****/
const ru_reg_rec XUMAC_RDP_GTLCL_UPPER_REG =
{
    "GTLCL_UPPER",
#if RU_INCLUDE_DESC
    "GTLCL upper 8 bits",
    "",
#endif
    { XUMAC_RDP_GTLCL_UPPER_REG_OFFSET },
    0,
    0,
    1309,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GTLCL_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GTXCL, TYPE: Type_MIB_GTXCL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_GTXCL_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Transmit excessive collision packet counter.\n",
#endif
    { XUMAC_RDP_GTXCL_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_GTXCL_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_GTXCL_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GTXCL_FIELDS[] =
{
    &XUMAC_RDP_GTXCL_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GTXCL *****/
const ru_reg_rec XUMAC_RDP_GTXCL_REG =
{
    "GTXCL",
#if RU_INCLUDE_DESC
    "Transmit Excessive Collision Packet Counter",
    "",
#endif
    { XUMAC_RDP_GTXCL_REG_OFFSET },
    0,
    0,
    1310,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GTXCL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GTXCL_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_GTXCL_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_GTXCL_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_GTXCL_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_GTXCL_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GTXCL_UPPER_FIELDS[] =
{
    &XUMAC_RDP_GTXCL_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GTXCL_UPPER *****/
const ru_reg_rec XUMAC_RDP_GTXCL_UPPER_REG =
{
    "GTXCL_UPPER",
#if RU_INCLUDE_DESC
    "GTXCL upper 8 bits",
    "",
#endif
    { XUMAC_RDP_GTXCL_UPPER_REG_OFFSET },
    0,
    0,
    1311,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GTXCL_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GTFRG, TYPE: Type_MIB_GTFRG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_GTFRG_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Transmit fragments packet counter.\n",
#endif
    { XUMAC_RDP_GTFRG_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_GTFRG_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_GTFRG_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GTFRG_FIELDS[] =
{
    &XUMAC_RDP_GTFRG_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GTFRG *****/
const ru_reg_rec XUMAC_RDP_GTFRG_REG =
{
    "GTFRG",
#if RU_INCLUDE_DESC
    "Transmit Fragments Packet Counter",
    "",
#endif
    { XUMAC_RDP_GTFRG_REG_OFFSET },
    0,
    0,
    1312,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GTFRG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GTFRG_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_GTFRG_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_GTFRG_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_GTFRG_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_GTFRG_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GTFRG_UPPER_FIELDS[] =
{
    &XUMAC_RDP_GTFRG_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GTFRG_UPPER *****/
const ru_reg_rec XUMAC_RDP_GTFRG_UPPER_REG =
{
    "GTFRG_UPPER",
#if RU_INCLUDE_DESC
    "GTFRG upper 8 bits",
    "",
#endif
    { XUMAC_RDP_GTFRG_UPPER_REG_OFFSET },
    0,
    0,
    1313,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GTFRG_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GTNCL, TYPE: Type_MIB_GTNCL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_GTNCL_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Transmit total collision counter.\n",
#endif
    { XUMAC_RDP_GTNCL_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_GTNCL_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_GTNCL_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GTNCL_FIELDS[] =
{
    &XUMAC_RDP_GTNCL_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GTNCL *****/
const ru_reg_rec XUMAC_RDP_GTNCL_REG =
{
    "GTNCL",
#if RU_INCLUDE_DESC
    "Transmit Total Collision Counter",
    "",
#endif
    { XUMAC_RDP_GTNCL_REG_OFFSET },
    0,
    0,
    1314,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GTNCL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GTNCL_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_GTNCL_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_GTNCL_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_GTNCL_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_GTNCL_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GTNCL_UPPER_FIELDS[] =
{
    &XUMAC_RDP_GTNCL_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GTNCL_UPPER *****/
const ru_reg_rec XUMAC_RDP_GTNCL_UPPER_REG =
{
    "GTNCL_UPPER",
#if RU_INCLUDE_DESC
    "GTNCL upper 8 bits",
    "",
#endif
    { XUMAC_RDP_GTNCL_UPPER_REG_OFFSET },
    0,
    0,
    1315,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GTNCL_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GTJBR, TYPE: Type_MIB_GTJBR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_GTJBR_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Transmit jabber counter.\n",
#endif
    { XUMAC_RDP_GTJBR_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_GTJBR_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_GTJBR_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GTJBR_FIELDS[] =
{
    &XUMAC_RDP_GTJBR_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GTJBR *****/
const ru_reg_rec XUMAC_RDP_GTJBR_REG =
{
    "GTJBR",
#if RU_INCLUDE_DESC
    "Transmit Jabber Counter",
    "",
#endif
    { XUMAC_RDP_GTJBR_REG_OFFSET },
    0,
    0,
    1316,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GTJBR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GTJBR_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_GTJBR_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_GTJBR_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_GTJBR_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_GTJBR_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GTJBR_UPPER_FIELDS[] =
{
    &XUMAC_RDP_GTJBR_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GTJBR_UPPER *****/
const ru_reg_rec XUMAC_RDP_GTJBR_UPPER_REG =
{
    "GTJBR_UPPER",
#if RU_INCLUDE_DESC
    "GTJBR upper 8 bits",
    "",
#endif
    { XUMAC_RDP_GTJBR_UPPER_REG_OFFSET },
    0,
    0,
    1317,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GTJBR_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GTBYT, TYPE: Type_MIB_GTBYT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_GTBYT_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Transmit byte counter.\n",
#endif
    { XUMAC_RDP_GTBYT_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_GTBYT_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_GTBYT_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GTBYT_FIELDS[] =
{
    &XUMAC_RDP_GTBYT_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GTBYT *****/
const ru_reg_rec XUMAC_RDP_GTBYT_REG =
{
    "GTBYT",
#if RU_INCLUDE_DESC
    "Transmit Byte Counter",
    "",
#endif
    { XUMAC_RDP_GTBYT_REG_OFFSET },
    0,
    0,
    1318,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GTBYT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GTBYT_UPPER, TYPE: Type_UPPER_16BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U16 *****/
const ru_field_rec XUMAC_RDP_GTBYT_UPPER_COUNT_U16_FIELD =
{
    "COUNT_U16",
#if RU_INCLUDE_DESC
    "",
    "Upper 16 bits of 48-bit counter.\n",
#endif
    { XUMAC_RDP_GTBYT_UPPER_COUNT_U16_FIELD_MASK },
    0,
    { XUMAC_RDP_GTBYT_UPPER_COUNT_U16_FIELD_WIDTH },
    { XUMAC_RDP_GTBYT_UPPER_COUNT_U16_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GTBYT_UPPER_FIELDS[] =
{
    &XUMAC_RDP_GTBYT_UPPER_COUNT_U16_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GTBYT_UPPER *****/
const ru_reg_rec XUMAC_RDP_GTBYT_UPPER_REG =
{
    "GTBYT_UPPER",
#if RU_INCLUDE_DESC
    "GTBYT upper 16 bits",
    "",
#endif
    { XUMAC_RDP_GTBYT_UPPER_REG_OFFSET },
    0,
    0,
    1319,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GTBYT_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GTPOK, TYPE: Type_MIB_GTPOK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_GTPOK_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Transmitted good packets counter.\n",
#endif
    { XUMAC_RDP_GTPOK_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_GTPOK_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_GTPOK_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GTPOK_FIELDS[] =
{
    &XUMAC_RDP_GTPOK_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GTPOK *****/
const ru_reg_rec XUMAC_RDP_GTPOK_REG =
{
    "GTPOK",
#if RU_INCLUDE_DESC
    "Transmit Good Packet Counter",
    "",
#endif
    { XUMAC_RDP_GTPOK_REG_OFFSET },
    0,
    0,
    1320,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GTPOK_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GTPOK_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_GTPOK_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_GTPOK_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_GTPOK_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_GTPOK_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GTPOK_UPPER_FIELDS[] =
{
    &XUMAC_RDP_GTPOK_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GTPOK_UPPER *****/
const ru_reg_rec XUMAC_RDP_GTPOK_UPPER_REG =
{
    "GTPOK_UPPER",
#if RU_INCLUDE_DESC
    "GTPOK upper 8 bits",
    "",
#endif
    { XUMAC_RDP_GTPOK_UPPER_REG_OFFSET },
    0,
    0,
    1321,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GTPOK_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GTUC, TYPE: Type_MIB_GTUC
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_GTUC_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "Transmitted Unicast packets counter.\n",
#endif
    { XUMAC_RDP_GTUC_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_GTUC_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_GTUC_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GTUC_FIELDS[] =
{
    &XUMAC_RDP_GTUC_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GTUC *****/
const ru_reg_rec XUMAC_RDP_GTUC_REG =
{
    "GTUC",
#if RU_INCLUDE_DESC
    "Transmit Unicast Packet Counter",
    "",
#endif
    { XUMAC_RDP_GTUC_REG_OFFSET },
    0,
    0,
    1322,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GTUC_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GTUC_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_GTUC_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_GTUC_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_GTUC_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_GTUC_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GTUC_UPPER_FIELDS[] =
{
    &XUMAC_RDP_GTUC_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GTUC_UPPER *****/
const ru_reg_rec XUMAC_RDP_GTUC_UPPER_REG =
{
    "GTUC_UPPER",
#if RU_INCLUDE_DESC
    "GTUC upper 8 bits",
    "",
#endif
    { XUMAC_RDP_GTUC_UPPER_REG_OFFSET },
    0,
    0,
    1323,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_GTUC_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_RRPKT, TYPE: Type_MIB_RRPKT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_RRPKT_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "RX RUNT packet counter.\n",
#endif
    { XUMAC_RDP_RRPKT_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_RRPKT_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_RRPKT_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_RRPKT_FIELDS[] =
{
    &XUMAC_RDP_RRPKT_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_RRPKT *****/
const ru_reg_rec XUMAC_RDP_RRPKT_REG =
{
    "RRPKT",
#if RU_INCLUDE_DESC
    "Receive RUNT Packet Counter",
    "",
#endif
    { XUMAC_RDP_RRPKT_REG_OFFSET },
    0,
    0,
    1324,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_RRPKT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_RRPKT_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_RRPKT_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_RRPKT_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_RRPKT_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_RRPKT_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_RRPKT_UPPER_FIELDS[] =
{
    &XUMAC_RDP_RRPKT_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_RRPKT_UPPER *****/
const ru_reg_rec XUMAC_RDP_RRPKT_UPPER_REG =
{
    "RRPKT_UPPER",
#if RU_INCLUDE_DESC
    "RRPKT upper 8 bits",
    "",
#endif
    { XUMAC_RDP_RRPKT_UPPER_REG_OFFSET },
    0,
    0,
    1325,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_RRPKT_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_RRUND, TYPE: Type_MIB_RRUND
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_RRUND_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "RX RUNT packet with valid FCS counter.\n",
#endif
    { XUMAC_RDP_RRUND_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_RRUND_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_RRUND_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_RRUND_FIELDS[] =
{
    &XUMAC_RDP_RRUND_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_RRUND *****/
const ru_reg_rec XUMAC_RDP_RRUND_REG =
{
    "RRUND",
#if RU_INCLUDE_DESC
    "Receive RUNT Packet and Contain a Valid FCS",
    "",
#endif
    { XUMAC_RDP_RRUND_REG_OFFSET },
    0,
    0,
    1326,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_RRUND_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_RRUND_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_RRUND_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_RRUND_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_RRUND_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_RRUND_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_RRUND_UPPER_FIELDS[] =
{
    &XUMAC_RDP_RRUND_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_RRUND_UPPER *****/
const ru_reg_rec XUMAC_RDP_RRUND_UPPER_REG =
{
    "RRUND_UPPER",
#if RU_INCLUDE_DESC
    "RRUND upper 8 bits",
    "",
#endif
    { XUMAC_RDP_RRUND_UPPER_REG_OFFSET },
    0,
    0,
    1327,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_RRUND_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_RRFRG, TYPE: Type_MIB_RRFRG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_RRFRG_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "RX RUNT packet with invalid FCS or alignment error counter.\n",
#endif
    { XUMAC_RDP_RRFRG_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_RRFRG_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_RRFRG_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_RRFRG_FIELDS[] =
{
    &XUMAC_RDP_RRFRG_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_RRFRG *****/
const ru_reg_rec XUMAC_RDP_RRFRG_REG =
{
    "RRFRG",
#if RU_INCLUDE_DESC
    "Receive RUNT Packet and Contain Invalid FCS or Alignment Error",
    "",
#endif
    { XUMAC_RDP_RRFRG_REG_OFFSET },
    0,
    0,
    1328,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_RRFRG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_RRFRG_UPPER, TYPE: Type_UPPER_8BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U8 *****/
const ru_field_rec XUMAC_RDP_RRFRG_UPPER_COUNT_U8_FIELD =
{
    "COUNT_U8",
#if RU_INCLUDE_DESC
    "",
    "Upper 8 bits of 40-bit counter.\n",
#endif
    { XUMAC_RDP_RRFRG_UPPER_COUNT_U8_FIELD_MASK },
    0,
    { XUMAC_RDP_RRFRG_UPPER_COUNT_U8_FIELD_WIDTH },
    { XUMAC_RDP_RRFRG_UPPER_COUNT_U8_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_RRFRG_UPPER_FIELDS[] =
{
    &XUMAC_RDP_RRFRG_UPPER_COUNT_U8_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_RRFRG_UPPER *****/
const ru_reg_rec XUMAC_RDP_RRFRG_UPPER_REG =
{
    "RRFRG_UPPER",
#if RU_INCLUDE_DESC
    "RRFRG upper 8 bits",
    "",
#endif
    { XUMAC_RDP_RRFRG_UPPER_REG_OFFSET },
    0,
    0,
    1329,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_RRFRG_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_RRBYT, TYPE: Type_MIB_RRBYT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT *****/
const ru_field_rec XUMAC_RDP_RRBYT_COUNT_FIELD =
{
    "COUNT",
#if RU_INCLUDE_DESC
    "",
    "RX RUNT packet byte counter.\n",
#endif
    { XUMAC_RDP_RRBYT_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_RRBYT_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_RRBYT_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_RRBYT_FIELDS[] =
{
    &XUMAC_RDP_RRBYT_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_RRBYT *****/
const ru_reg_rec XUMAC_RDP_RRBYT_REG =
{
    "RRBYT",
#if RU_INCLUDE_DESC
    "Receive RUNT Packet Byte Counter",
    "",
#endif
    { XUMAC_RDP_RRBYT_REG_OFFSET },
    0,
    0,
    1330,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_RRBYT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_RRBYT_UPPER, TYPE: Type_UPPER_16BITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: COUNT_U16 *****/
const ru_field_rec XUMAC_RDP_RRBYT_UPPER_COUNT_U16_FIELD =
{
    "COUNT_U16",
#if RU_INCLUDE_DESC
    "",
    "Upper 16 bits of 48-bit counter.\n",
#endif
    { XUMAC_RDP_RRBYT_UPPER_COUNT_U16_FIELD_MASK },
    0,
    { XUMAC_RDP_RRBYT_UPPER_COUNT_U16_FIELD_WIDTH },
    { XUMAC_RDP_RRBYT_UPPER_COUNT_U16_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_RRBYT_UPPER_FIELDS[] =
{
    &XUMAC_RDP_RRBYT_UPPER_COUNT_U16_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_RRBYT_UPPER *****/
const ru_reg_rec XUMAC_RDP_RRBYT_UPPER_REG =
{
    "RRBYT_UPPER",
#if RU_INCLUDE_DESC
    "RRBYT upper 16 bits",
    "",
#endif
    { XUMAC_RDP_RRBYT_UPPER_REG_OFFSET },
    0,
    0,
    1331,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_RRBYT_UPPER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_MIB_CNTRL, TYPE: Type_MIB_CNTRL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RX_CNT_RST *****/
const ru_field_rec XUMAC_RDP_MIB_CNTRL_RX_CNT_RST_FIELD =
{
    "RX_CNT_RST",
#if RU_INCLUDE_DESC
    "",
    "Active high. When this bit is set, RX statistics counters will be reseted.\n",
#endif
    { XUMAC_RDP_MIB_CNTRL_RX_CNT_RST_FIELD_MASK },
    0,
    { XUMAC_RDP_MIB_CNTRL_RX_CNT_RST_FIELD_WIDTH },
    { XUMAC_RDP_MIB_CNTRL_RX_CNT_RST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RUNT_CNT_RST *****/
const ru_field_rec XUMAC_RDP_MIB_CNTRL_RUNT_CNT_RST_FIELD =
{
    "RUNT_CNT_RST",
#if RU_INCLUDE_DESC
    "",
    "Active high. When this bit is set, Runt statistics counters will be reseted.\n",
#endif
    { XUMAC_RDP_MIB_CNTRL_RUNT_CNT_RST_FIELD_MASK },
    0,
    { XUMAC_RDP_MIB_CNTRL_RUNT_CNT_RST_FIELD_WIDTH },
    { XUMAC_RDP_MIB_CNTRL_RUNT_CNT_RST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TX_CNT_RST *****/
const ru_field_rec XUMAC_RDP_MIB_CNTRL_TX_CNT_RST_FIELD =
{
    "TX_CNT_RST",
#if RU_INCLUDE_DESC
    "",
    "Active high. When this bit is set, TX statistics counters will be reseted.\n",
#endif
    { XUMAC_RDP_MIB_CNTRL_TX_CNT_RST_FIELD_MASK },
    0,
    { XUMAC_RDP_MIB_CNTRL_TX_CNT_RST_FIELD_WIDTH },
    { XUMAC_RDP_MIB_CNTRL_TX_CNT_RST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_MIB_CNTRL_FIELDS[] =
{
    &XUMAC_RDP_MIB_CNTRL_RX_CNT_RST_FIELD,
    &XUMAC_RDP_MIB_CNTRL_RUNT_CNT_RST_FIELD,
    &XUMAC_RDP_MIB_CNTRL_TX_CNT_RST_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_MIB_CNTRL *****/
const ru_reg_rec XUMAC_RDP_MIB_CNTRL_REG =
{
    "MIB_CNTRL",
#if RU_INCLUDE_DESC
    "MIB Control Register",
    "",
#endif
    { XUMAC_RDP_MIB_CNTRL_REG_OFFSET },
    0,
    0,
    1332,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    XUMAC_RDP_MIB_CNTRL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_MIB_READ_DATA, TYPE: Type_MIB_DATA32
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA32 *****/
const ru_field_rec XUMAC_RDP_MIB_READ_DATA_DATA32_FIELD =
{
    "DATA32",
#if RU_INCLUDE_DESC
    "",
    "32-bit data holder.\n",
#endif
    { XUMAC_RDP_MIB_READ_DATA_DATA32_FIELD_MASK },
    0,
    { XUMAC_RDP_MIB_READ_DATA_DATA32_FIELD_WIDTH },
    { XUMAC_RDP_MIB_READ_DATA_DATA32_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_MIB_READ_DATA_FIELDS[] =
{
    &XUMAC_RDP_MIB_READ_DATA_DATA32_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_MIB_READ_DATA *****/
const ru_reg_rec XUMAC_RDP_MIB_READ_DATA_REG =
{
    "MIB_READ_DATA",
#if RU_INCLUDE_DESC
    "MIB read data",
    "",
#endif
    { XUMAC_RDP_MIB_READ_DATA_REG_OFFSET },
    0,
    0,
    1333,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_MIB_READ_DATA_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_MIB_WRITE_DATA, TYPE: Type_MIB_DATA32
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DATA32 *****/
const ru_field_rec XUMAC_RDP_MIB_WRITE_DATA_DATA32_FIELD =
{
    "DATA32",
#if RU_INCLUDE_DESC
    "",
    "32-bit data holder.\n",
#endif
    { XUMAC_RDP_MIB_WRITE_DATA_DATA32_FIELD_MASK },
    0,
    { XUMAC_RDP_MIB_WRITE_DATA_DATA32_FIELD_WIDTH },
    { XUMAC_RDP_MIB_WRITE_DATA_DATA32_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_MIB_WRITE_DATA_FIELDS[] =
{
    &XUMAC_RDP_MIB_WRITE_DATA_DATA32_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_MIB_WRITE_DATA *****/
const ru_reg_rec XUMAC_RDP_MIB_WRITE_DATA_REG =
{
    "MIB_WRITE_DATA",
#if RU_INCLUDE_DESC
    "MIB write data",
    "",
#endif
    { XUMAC_RDP_MIB_WRITE_DATA_REG_OFFSET },
    0,
    0,
    1334,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_MIB_WRITE_DATA_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_CONTROL, TYPE: Type_CONTROL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MPD_EN *****/
const ru_field_rec XUMAC_RDP_CONTROL_MPD_EN_FIELD =
{
    "MPD_EN",
#if RU_INCLUDE_DESC
    "",
    "When this bit is set, Magic Packet detection is enabled.\n",
#endif
    { XUMAC_RDP_CONTROL_MPD_EN_FIELD_MASK },
    0,
    { XUMAC_RDP_CONTROL_MPD_EN_FIELD_WIDTH },
    { XUMAC_RDP_CONTROL_MPD_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MSEQ_LEN *****/
const ru_field_rec XUMAC_RDP_CONTROL_MSEQ_LEN_FIELD =
{
    "MSEQ_LEN",
#if RU_INCLUDE_DESC
    "",
    "Number of repetitions of MAC Destination Address that must be detected in Magic Packet.\n",
#endif
    { XUMAC_RDP_CONTROL_MSEQ_LEN_FIELD_MASK },
    0,
    { XUMAC_RDP_CONTROL_MSEQ_LEN_FIELD_WIDTH },
    { XUMAC_RDP_CONTROL_MSEQ_LEN_FIELD_SHIFT },
    16,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PSW_EN *****/
const ru_field_rec XUMAC_RDP_CONTROL_PSW_EN_FIELD =
{
    "PSW_EN",
#if RU_INCLUDE_DESC
    "",
    "When set, enable Magic Sequence password check.\n",
#endif
    { XUMAC_RDP_CONTROL_PSW_EN_FIELD_MASK },
    0,
    { XUMAC_RDP_CONTROL_PSW_EN_FIELD_WIDTH },
    { XUMAC_RDP_CONTROL_PSW_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_CONTROL_FIELDS[] =
{
    &XUMAC_RDP_CONTROL_MPD_EN_FIELD,
    &XUMAC_RDP_CONTROL_MSEQ_LEN_FIELD,
    &XUMAC_RDP_CONTROL_PSW_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_CONTROL *****/
const ru_reg_rec XUMAC_RDP_CONTROL_REG =
{
    "CONTROL",
#if RU_INCLUDE_DESC
    "Magic Packet Control Register",
    "",
#endif
    { XUMAC_RDP_CONTROL_REG_OFFSET },
    0,
    0,
    1335,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    XUMAC_RDP_CONTROL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_PSW_MS, TYPE: Type_PSW_MS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PSW_47_32 *****/
const ru_field_rec XUMAC_RDP_PSW_MS_PSW_47_32_FIELD =
{
    "PSW_47_32",
#if RU_INCLUDE_DESC
    "",
    "Magic Packet Optional password bytes 0-1 (password bits [47:32]).Bits [47:40] correspond to password byte 0, which is the first password byte received from the wire.\n",
#endif
    { XUMAC_RDP_PSW_MS_PSW_47_32_FIELD_MASK },
    0,
    { XUMAC_RDP_PSW_MS_PSW_47_32_FIELD_WIDTH },
    { XUMAC_RDP_PSW_MS_PSW_47_32_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_PSW_MS_FIELDS[] =
{
    &XUMAC_RDP_PSW_MS_PSW_47_32_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_PSW_MS *****/
const ru_reg_rec XUMAC_RDP_PSW_MS_REG =
{
    "PSW_MS",
#if RU_INCLUDE_DESC
    "Magic Packet Optional Password bytes 0-1",
    "",
#endif
    { XUMAC_RDP_PSW_MS_REG_OFFSET },
    0,
    0,
    1336,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_PSW_MS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_PSW_LS, TYPE: Type_PSW_LS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PSW_31_0 *****/
const ru_field_rec XUMAC_RDP_PSW_LS_PSW_31_0_FIELD =
{
    "PSW_31_0",
#if RU_INCLUDE_DESC
    "",
    "Magic Packet Optional Password bytes 2-5 (password bits [31:0]).\n",
#endif
    { XUMAC_RDP_PSW_LS_PSW_31_0_FIELD_MASK },
    0,
    { XUMAC_RDP_PSW_LS_PSW_31_0_FIELD_WIDTH },
    { XUMAC_RDP_PSW_LS_PSW_31_0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_PSW_LS_FIELDS[] =
{
    &XUMAC_RDP_PSW_LS_PSW_31_0_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_PSW_LS *****/
const ru_reg_rec XUMAC_RDP_PSW_LS_REG =
{
    "PSW_LS",
#if RU_INCLUDE_DESC
    "Magic Packet Optional Password bytes 2-5",
    "",
#endif
    { XUMAC_RDP_PSW_LS_REG_OFFSET },
    0,
    0,
    1337,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_PSW_LS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_CONTROL_1, TYPE: Type_CONTROL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: XIB_RX_EN *****/
const ru_field_rec XUMAC_RDP_CONTROL_1_XIB_RX_EN_FIELD =
{
    "XIB_RX_EN",
#if RU_INCLUDE_DESC
    "",
    "When set enables receive MAC. When cleared, all data received from PHY are ignored by RX MAC and no data are passed to the system.\n",
#endif
    { XUMAC_RDP_CONTROL_1_XIB_RX_EN_FIELD_MASK },
    0,
    { XUMAC_RDP_CONTROL_1_XIB_RX_EN_FIELD_WIDTH },
    { XUMAC_RDP_CONTROL_1_XIB_RX_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: XIB_TX_EN *****/
const ru_field_rec XUMAC_RDP_CONTROL_1_XIB_TX_EN_FIELD =
{
    "XIB_TX_EN",
#if RU_INCLUDE_DESC
    "",
    "When set enables transmit MAC. When cleared all data received from the system are ignored and no data are transmitted to the wire (IDLEs are transmit instead).\n",
#endif
    { XUMAC_RDP_CONTROL_1_XIB_TX_EN_FIELD_MASK },
    0,
    { XUMAC_RDP_CONTROL_1_XIB_TX_EN_FIELD_WIDTH },
    { XUMAC_RDP_CONTROL_1_XIB_TX_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RX_FLUSH_EN *****/
const ru_field_rec XUMAC_RDP_CONTROL_1_RX_FLUSH_EN_FIELD =
{
    "RX_FLUSH_EN",
#if RU_INCLUDE_DESC
    "",
    "When set MAC receive pipe is flushed (control registers are not affected). Must be cleared by SW.\n",
#endif
    { XUMAC_RDP_CONTROL_1_RX_FLUSH_EN_FIELD_MASK },
    0,
    { XUMAC_RDP_CONTROL_1_RX_FLUSH_EN_FIELD_WIDTH },
    { XUMAC_RDP_CONTROL_1_RX_FLUSH_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TX_FLUSH_EN *****/
const ru_field_rec XUMAC_RDP_CONTROL_1_TX_FLUSH_EN_FIELD =
{
    "TX_FLUSH_EN",
#if RU_INCLUDE_DESC
    "",
    "When set MAC transmit pipe is flushed (control registers are not affected). Must be cleared by SW.\n",
#endif
    { XUMAC_RDP_CONTROL_1_TX_FLUSH_EN_FIELD_MASK },
    0,
    { XUMAC_RDP_CONTROL_1_TX_FLUSH_EN_FIELD_WIDTH },
    { XUMAC_RDP_CONTROL_1_TX_FLUSH_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: LINK_DOWN_RST_EN *****/
const ru_field_rec XUMAC_RDP_CONTROL_1_LINK_DOWN_RST_EN_FIELD =
{
    "LINK_DOWN_RST_EN",
#if RU_INCLUDE_DESC
    "",
    "When set MAC flushes its RX and TX pipe when the loss of link is detected. It stays in reset until valid link status is indicated by PHY.\n",
#endif
    { XUMAC_RDP_CONTROL_1_LINK_DOWN_RST_EN_FIELD_MASK },
    0,
    { XUMAC_RDP_CONTROL_1_LINK_DOWN_RST_EN_FIELD_WIDTH },
    { XUMAC_RDP_CONTROL_1_LINK_DOWN_RST_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: STANDARD_MUX_EN *****/
const ru_field_rec XUMAC_RDP_CONTROL_1_STANDARD_MUX_EN_FIELD =
{
    "STANDARD_MUX_EN",
#if RU_INCLUDE_DESC
    "",
    "When set glitch-less clock muxes behave as a regular clock muxes.\nThis is debug only feature.\n",
#endif
    { XUMAC_RDP_CONTROL_1_STANDARD_MUX_EN_FIELD_MASK },
    0,
    { XUMAC_RDP_CONTROL_1_STANDARD_MUX_EN_FIELD_WIDTH },
    { XUMAC_RDP_CONTROL_1_STANDARD_MUX_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: XGMII_SEL *****/
const ru_field_rec XUMAC_RDP_CONTROL_1_XGMII_SEL_FIELD =
{
    "XGMII_SEL",
#if RU_INCLUDE_DESC
    "",
    "When cleared XIB is by bypassed that is PHY's GMII interfaces is passed through. This bit is valid only when XGMII_SEL_OVRD = 1.\n",
#endif
    { XUMAC_RDP_CONTROL_1_XGMII_SEL_FIELD_MASK },
    0,
    { XUMAC_RDP_CONTROL_1_XGMII_SEL_FIELD_WIDTH },
    { XUMAC_RDP_CONTROL_1_XGMII_SEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DIC_DIS *****/
const ru_field_rec XUMAC_RDP_CONTROL_1_DIC_DIS_FIELD =
{
    "DIC_DIS",
#if RU_INCLUDE_DESC
    "",
    "When cleared GIB TX deploys DIC (Deficit Idle Counter) algorithm. This algorithms maintains 10Gbps data rate by inserting more or less idles than specified via TX_IPG.\nWhen set minimum of TX_IPG IDLEs are insert between packets leading to data rate that is lower than 10Gbps due to the SOP alignment rules.\n",
#endif
    { XUMAC_RDP_CONTROL_1_DIC_DIS_FIELD_MASK },
    0,
    { XUMAC_RDP_CONTROL_1_DIC_DIS_FIELD_WIDTH },
    { XUMAC_RDP_CONTROL_1_DIC_DIS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RX_START_THRESHOLD *****/
const ru_field_rec XUMAC_RDP_CONTROL_1_RX_START_THRESHOLD_FIELD =
{
    "RX_START_THRESHOLD",
#if RU_INCLUDE_DESC
    "",
    "Packet receive in GMII clock domain starts only when at least RX_START_THRESHOLD words are available in XIB RX FIFO.\n",
#endif
    { XUMAC_RDP_CONTROL_1_RX_START_THRESHOLD_FIELD_MASK },
    0,
    { XUMAC_RDP_CONTROL_1_RX_START_THRESHOLD_FIELD_WIDTH },
    { XUMAC_RDP_CONTROL_1_RX_START_THRESHOLD_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GMII_RX_CLK_GATE_EN *****/
const ru_field_rec XUMAC_RDP_CONTROL_1_GMII_RX_CLK_GATE_EN_FIELD =
{
    "GMII_RX_CLK_GATE_EN",
#if RU_INCLUDE_DESC
    "",
    "When set XIB will gate GMII RX clock if RX FIFO becomes empty in the middle of the packet, in order to prevent packet corruption.\n",
#endif
    { XUMAC_RDP_CONTROL_1_GMII_RX_CLK_GATE_EN_FIELD_MASK },
    0,
    { XUMAC_RDP_CONTROL_1_GMII_RX_CLK_GATE_EN_FIELD_WIDTH },
    { XUMAC_RDP_CONTROL_1_GMII_RX_CLK_GATE_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: STRICT_PREAMBLE_DIS *****/
const ru_field_rec XUMAC_RDP_CONTROL_1_STRICT_PREAMBLE_DIS_FIELD =
{
    "STRICT_PREAMBLE_DIS",
#if RU_INCLUDE_DESC
    "",
    "When this bit is set XIB will support packets with shorter or longer than standard (i.e. less or more than 6B of 0x55) preamble. XIB will accept any number of preamble bytes (0x55) until SFD is detected. If SFD is not detected but instead SOP or EFD, current data are discarded and XIB restarts the parsing.\nWhen cleared packets with non-standard preamble are discarded.\n",
#endif
    { XUMAC_RDP_CONTROL_1_STRICT_PREAMBLE_DIS_FIELD_MASK },
    0,
    { XUMAC_RDP_CONTROL_1_STRICT_PREAMBLE_DIS_FIELD_WIDTH },
    { XUMAC_RDP_CONTROL_1_STRICT_PREAMBLE_DIS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TX_IPG *****/
const ru_field_rec XUMAC_RDP_CONTROL_1_TX_IPG_FIELD =
{
    "TX_IPG",
#if RU_INCLUDE_DESC
    "",
    "An average number of XGMII IDLE characters that will be inserted between two packets on TX. The actual number of IDLEs between any two packets can be larger or smaller than this number, depending on the programmed IDLE insertion algorithm. Note that IEEE 802.3 specifies 5B as minimum IPG (TERMINATE + 4B of IDLE).\n",
#endif
    { XUMAC_RDP_CONTROL_1_TX_IPG_FIELD_MASK },
    0,
    { XUMAC_RDP_CONTROL_1_TX_IPG_FIELD_WIDTH },
    { XUMAC_RDP_CONTROL_1_TX_IPG_FIELD_SHIFT },
    12,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MIN_RX_IPG *****/
const ru_field_rec XUMAC_RDP_CONTROL_1_MIN_RX_IPG_FIELD =
{
    "MIN_RX_IPG",
#if RU_INCLUDE_DESC
    "",
    "This value guaranties minimum IPG between any two of received packets. When set to 0 minimum RX IPG is not enforced that is it equals XGMII IPG (plus/minus IDLEs inserted/deleted in clock compensation purposes).\n",
#endif
    { XUMAC_RDP_CONTROL_1_MIN_RX_IPG_FIELD_MASK },
    0,
    { XUMAC_RDP_CONTROL_1_MIN_RX_IPG_FIELD_WIDTH },
    { XUMAC_RDP_CONTROL_1_MIN_RX_IPG_FIELD_SHIFT },
    5,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: XGMII_SEL_OVRD *****/
const ru_field_rec XUMAC_RDP_CONTROL_1_XGMII_SEL_OVRD_FIELD =
{
    "XGMII_SEL_OVRD",
#if RU_INCLUDE_DESC
    "",
    "When set enables XGMII_SEL to select XIB PHY interface. When 0 interface is selected based on the HW pin.\n",
#endif
    { XUMAC_RDP_CONTROL_1_XGMII_SEL_OVRD_FIELD_MASK },
    0,
    { XUMAC_RDP_CONTROL_1_XGMII_SEL_OVRD_FIELD_WIDTH },
    { XUMAC_RDP_CONTROL_1_XGMII_SEL_OVRD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: GMII_TX_CLK_GATE_EN *****/
const ru_field_rec XUMAC_RDP_CONTROL_1_GMII_TX_CLK_GATE_EN_FIELD =
{
    "GMII_TX_CLK_GATE_EN",
#if RU_INCLUDE_DESC
    "",
    "When set XIB will gate GMII TX clock if the TX FIFO occupancy becomes greater or equal to its XOFF threshold. It will re-enable the clock when the TX FIFO occupancy is equal or below its XON threshold.\nShould not be enabled when TX_BACKPRESSURE_EN = 1.\n",
#endif
    { XUMAC_RDP_CONTROL_1_GMII_TX_CLK_GATE_EN_FIELD_MASK },
    0,
    { XUMAC_RDP_CONTROL_1_GMII_TX_CLK_GATE_EN_FIELD_WIDTH },
    { XUMAC_RDP_CONTROL_1_GMII_TX_CLK_GATE_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: AUTOCONFIG_EN *****/
const ru_field_rec XUMAC_RDP_CONTROL_1_AUTOCONFIG_EN_FIELD =
{
    "AUTOCONFIG_EN",
#if RU_INCLUDE_DESC
    "",
    "When set XIB will set N and M for clock swallower automatically based on the XGMII clock/data rate.\nNote: This is only applicable to case when the internal serdes is source of the clock used to generate XIB?s GMII clocks.\nNote: The default is the value to which i_autoconfig_en_strap is tied.\n",
#endif
    { XUMAC_RDP_CONTROL_1_AUTOCONFIG_EN_FIELD_MASK },
    0,
    { XUMAC_RDP_CONTROL_1_AUTOCONFIG_EN_FIELD_WIDTH },
    { XUMAC_RDP_CONTROL_1_AUTOCONFIG_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_CONTROL_1_FIELDS[] =
{
    &XUMAC_RDP_CONTROL_1_XIB_RX_EN_FIELD,
    &XUMAC_RDP_CONTROL_1_XIB_TX_EN_FIELD,
    &XUMAC_RDP_CONTROL_1_RX_FLUSH_EN_FIELD,
    &XUMAC_RDP_CONTROL_1_TX_FLUSH_EN_FIELD,
    &XUMAC_RDP_CONTROL_1_LINK_DOWN_RST_EN_FIELD,
    &XUMAC_RDP_CONTROL_1_STANDARD_MUX_EN_FIELD,
    &XUMAC_RDP_CONTROL_1_XGMII_SEL_FIELD,
    &XUMAC_RDP_CONTROL_1_DIC_DIS_FIELD,
    &XUMAC_RDP_CONTROL_1_RX_START_THRESHOLD_FIELD,
    &XUMAC_RDP_CONTROL_1_GMII_RX_CLK_GATE_EN_FIELD,
    &XUMAC_RDP_CONTROL_1_STRICT_PREAMBLE_DIS_FIELD,
    &XUMAC_RDP_CONTROL_1_TX_IPG_FIELD,
    &XUMAC_RDP_CONTROL_1_MIN_RX_IPG_FIELD,
    &XUMAC_RDP_CONTROL_1_XGMII_SEL_OVRD_FIELD,
    &XUMAC_RDP_CONTROL_1_GMII_TX_CLK_GATE_EN_FIELD,
    &XUMAC_RDP_CONTROL_1_AUTOCONFIG_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_CONTROL_1 *****/
const ru_reg_rec XUMAC_RDP_CONTROL_1_REG =
{
    "CONTROL_1",
#if RU_INCLUDE_DESC
    "XIB Control Register",
    "",
#endif
    { XUMAC_RDP_CONTROL_1_REG_OFFSET },
    0,
    0,
    1338,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    16,
    XUMAC_RDP_CONTROL_1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_EXTENDED_CONTROL, TYPE: Type_EXTENDED_CONTROL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TX_START_THRESHOLD *****/
const ru_field_rec XUMAC_RDP_EXTENDED_CONTROL_TX_START_THRESHOLD_FIELD =
{
    "TX_START_THRESHOLD",
#if RU_INCLUDE_DESC
    "",
    "Packet transmission in XGMII clock domain starts only when at least TX_START_THRESHOLD words are available in XIB TX FIFO. This threshold is applicable to cases where TX FIFO may become empty due to the link down or the other faults.\n",
#endif
    { XUMAC_RDP_EXTENDED_CONTROL_TX_START_THRESHOLD_FIELD_MASK },
    0,
    { XUMAC_RDP_EXTENDED_CONTROL_TX_START_THRESHOLD_FIELD_WIDTH },
    { XUMAC_RDP_EXTENDED_CONTROL_TX_START_THRESHOLD_FIELD_SHIFT },
    3,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TX_XOFF_THRESHOLD *****/
const ru_field_rec XUMAC_RDP_EXTENDED_CONTROL_TX_XOFF_THRESHOLD_FIELD =
{
    "TX_XOFF_THRESHOLD",
#if RU_INCLUDE_DESC
    "",
    "TX XOFF threshold. When TX FIFO depth is equal or larger than this threshold XIB asserts backpressure toward the switch port until FIFO occupancy falls below TX_XON_THERSHOLD.\n",
#endif
    { XUMAC_RDP_EXTENDED_CONTROL_TX_XOFF_THRESHOLD_FIELD_MASK },
    0,
    { XUMAC_RDP_EXTENDED_CONTROL_TX_XOFF_THRESHOLD_FIELD_WIDTH },
    { XUMAC_RDP_EXTENDED_CONTROL_TX_XOFF_THRESHOLD_FIELD_SHIFT },
    12,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TX_XON_THRESHOLD *****/
const ru_field_rec XUMAC_RDP_EXTENDED_CONTROL_TX_XON_THRESHOLD_FIELD =
{
    "TX_XON_THRESHOLD",
#if RU_INCLUDE_DESC
    "",
    "TX XON threshold. When FIFO occupancy drops below this threshold, XIB de-asserts backpressure toward switch port.\n",
#endif
    { XUMAC_RDP_EXTENDED_CONTROL_TX_XON_THRESHOLD_FIELD_MASK },
    0,
    { XUMAC_RDP_EXTENDED_CONTROL_TX_XON_THRESHOLD_FIELD_WIDTH },
    { XUMAC_RDP_EXTENDED_CONTROL_TX_XON_THRESHOLD_FIELD_SHIFT },
    4,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TX_BACKPRESSURE_EN *****/
const ru_field_rec XUMAC_RDP_EXTENDED_CONTROL_TX_BACKPRESSURE_EN_FIELD =
{
    "TX_BACKPRESSURE_EN",
#if RU_INCLUDE_DESC
    "",
    "When set enables asserting backpressure toward MAC (in between packets) when TX XOFF THRESHOLD is crossed.\nShould not be enabled when TX clock gating is enabled.\n",
#endif
    { XUMAC_RDP_EXTENDED_CONTROL_TX_BACKPRESSURE_EN_FIELD_MASK },
    0,
    { XUMAC_RDP_EXTENDED_CONTROL_TX_BACKPRESSURE_EN_FIELD_WIDTH },
    { XUMAC_RDP_EXTENDED_CONTROL_TX_BACKPRESSURE_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_EXTENDED_CONTROL_FIELDS[] =
{
    &XUMAC_RDP_EXTENDED_CONTROL_TX_START_THRESHOLD_FIELD,
    &XUMAC_RDP_EXTENDED_CONTROL_TX_XOFF_THRESHOLD_FIELD,
    &XUMAC_RDP_EXTENDED_CONTROL_TX_XON_THRESHOLD_FIELD,
    &XUMAC_RDP_EXTENDED_CONTROL_TX_BACKPRESSURE_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_EXTENDED_CONTROL *****/
const ru_reg_rec XUMAC_RDP_EXTENDED_CONTROL_REG =
{
    "EXTENDED_CONTROL",
#if RU_INCLUDE_DESC
    "XIB Extended Control Register",
    "",
#endif
    { XUMAC_RDP_EXTENDED_CONTROL_REG_OFFSET },
    0,
    0,
    1339,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    XUMAC_RDP_EXTENDED_CONTROL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_TX_IDLE_STUFFING_CONTROL, TYPE: Type_TX_IDLE_STUFFING_CONTROL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TX_IDLE_STUFFING_CTRL *****/
const ru_field_rec XUMAC_RDP_TX_IDLE_STUFFING_CONTROL_TX_IDLE_STUFFING_CTRL_FIELD =
{
    "TX_IDLE_STUFFING_CTRL",
#if RU_INCLUDE_DESC
    "",
    "This field controls the IDLE time. Idle time is inserted NX times when N equals to the packet size in bytes, including regular idle, preamble, SOP and EOP.\n4'b0000: No IDLE stuffing\nWhen the link speed is 10Gbps, available settings are:\n4'b0001: 1X. This provides 1/2 rate, i.e. 5Gbps\n4'b0010: 3X. This provides 1/4 rate, i.e. 2.5Gbps\n4'b0011: 9X. This provides 1/10 rate, i.e. 1Gbps\n4'b0100: 99X. This provides 1/100 rate, i.e. 100Mbps\n4'b0101: 999X. This provides 1/1000 rate, i.e. 10Mbps\nWhen the link speed is 5Gbps, available settings are:\n4'b0001: 1X. This provides 1/2 rate, i.e. 2.5Gbps\n4'b0110: 4X. This provides 1/5 rate, i.e. 1Gbps\n4'b0111: 49X. This provides 1/50 rate, i.e. 100Mbps\n4'b1000: 499X. This provides 1/500 rate, i.e. 10Mbps\nWhen the link speed is 2.5Gbps, available settings are:\n4'b1001: 1.5X. This provides 1/2.5 rate. i.e. 1Gbps\n4'b1010: 24X. This provides 1/25 rate, i.e. 100Mbps\n4'b1011: 249X. This provides 1/250 rate, i.e. 10Mbps\n4'b1100 - 4'b1111: Reserved\n",
#endif
    { XUMAC_RDP_TX_IDLE_STUFFING_CONTROL_TX_IDLE_STUFFING_CTRL_FIELD_MASK },
    0,
    { XUMAC_RDP_TX_IDLE_STUFFING_CONTROL_TX_IDLE_STUFFING_CTRL_FIELD_WIDTH },
    { XUMAC_RDP_TX_IDLE_STUFFING_CONTROL_TX_IDLE_STUFFING_CTRL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_TX_IDLE_STUFFING_CONTROL_FIELDS[] =
{
    &XUMAC_RDP_TX_IDLE_STUFFING_CONTROL_TX_IDLE_STUFFING_CTRL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_TX_IDLE_STUFFING_CONTROL *****/
const ru_reg_rec XUMAC_RDP_TX_IDLE_STUFFING_CONTROL_REG =
{
    "TX_IDLE_STUFFING_CONTROL",
#if RU_INCLUDE_DESC
    "XIB Tx Idle Stuffing Control Register",
    "",
#endif
    { XUMAC_RDP_TX_IDLE_STUFFING_CONTROL_REG_OFFSET },
    0,
    0,
    1340,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_TX_IDLE_STUFFING_CONTROL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_ACTUAL_DATA_RATE, TYPE: Type_ACTUAL_DATA_RATE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ACTUAL_DATA_RATE *****/
const ru_field_rec XUMAC_RDP_ACTUAL_DATA_RATE_ACTUAL_DATA_RATE_FIELD =
{
    "ACTUAL_DATA_RATE",
#if RU_INCLUDE_DESC
    "",
    "Actual Data Rate. Accounts for IDLE stuffing. Encoded as:\n3'b000 - 10Mbps\n3'b001 - 100Mbps\n3'b010 - 1Gbps\n3'b011 - 2.5Gbps\n3'b100 - 10Gbps\n3'b101 - 5Gbps\n3'b110 - 3'b111 - Reserved\n",
#endif
    { XUMAC_RDP_ACTUAL_DATA_RATE_ACTUAL_DATA_RATE_FIELD_MASK },
    0,
    { XUMAC_RDP_ACTUAL_DATA_RATE_ACTUAL_DATA_RATE_FIELD_WIDTH },
    { XUMAC_RDP_ACTUAL_DATA_RATE_ACTUAL_DATA_RATE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_ACTUAL_DATA_RATE_FIELDS[] =
{
    &XUMAC_RDP_ACTUAL_DATA_RATE_ACTUAL_DATA_RATE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_ACTUAL_DATA_RATE *****/
const ru_reg_rec XUMAC_RDP_ACTUAL_DATA_RATE_REG =
{
    "ACTUAL_DATA_RATE",
#if RU_INCLUDE_DESC
    "XIB Actual Data Rate Register",
    "",
#endif
    { XUMAC_RDP_ACTUAL_DATA_RATE_REG_OFFSET },
    0,
    0,
    1341,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_ACTUAL_DATA_RATE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_GMII_CLOCK_SWALLOWER_CONTROL, TYPE: Type_GMII_CLOCK_SWALLOWER_CONTROL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: NDIV *****/
const ru_field_rec XUMAC_RDP_GMII_CLOCK_SWALLOWER_CONTROL_NDIV_FIELD =
{
    "NDIV",
#if RU_INCLUDE_DESC
    "",
    "GMII clock swallower divisor. GMII clock swallower produces MDIV output clocks for every NDIV input clocks resulting in average MAC GMII RX and TX clock frequency of MDIV/NDIV*INPUT_CLOCK_FREQUENCY. Used only during XGMII to/from GMII conversion.\n",
#endif
    { XUMAC_RDP_GMII_CLOCK_SWALLOWER_CONTROL_NDIV_FIELD_MASK },
    0,
    { XUMAC_RDP_GMII_CLOCK_SWALLOWER_CONTROL_NDIV_FIELD_WIDTH },
    { XUMAC_RDP_GMII_CLOCK_SWALLOWER_CONTROL_NDIV_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MDIV *****/
const ru_field_rec XUMAC_RDP_GMII_CLOCK_SWALLOWER_CONTROL_MDIV_FIELD =
{
    "MDIV",
#if RU_INCLUDE_DESC
    "",
    "GMII clock swallower dividend. Must be <= NDIV. Used only during XGMII to/from GMII conversion.\n",
#endif
    { XUMAC_RDP_GMII_CLOCK_SWALLOWER_CONTROL_MDIV_FIELD_MASK },
    0,
    { XUMAC_RDP_GMII_CLOCK_SWALLOWER_CONTROL_MDIV_FIELD_WIDTH },
    { XUMAC_RDP_GMII_CLOCK_SWALLOWER_CONTROL_MDIV_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_GMII_CLOCK_SWALLOWER_CONTROL_FIELDS[] =
{
    &XUMAC_RDP_GMII_CLOCK_SWALLOWER_CONTROL_NDIV_FIELD,
    &XUMAC_RDP_GMII_CLOCK_SWALLOWER_CONTROL_MDIV_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_GMII_CLOCK_SWALLOWER_CONTROL *****/
const ru_reg_rec XUMAC_RDP_GMII_CLOCK_SWALLOWER_CONTROL_REG =
{
    "GMII_CLOCK_SWALLOWER_CONTROL",
#if RU_INCLUDE_DESC
    "XIB GMII Clock Swallower Control Register",
    "",
#endif
    { XUMAC_RDP_GMII_CLOCK_SWALLOWER_CONTROL_REG_OFFSET },
    0,
    0,
    1342,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XUMAC_RDP_GMII_CLOCK_SWALLOWER_CONTROL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_XGMII_DATA_RATE_STATUS, TYPE: Type_XGMII_DATA_RATE_STATUS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: XGMII_DATA_RATE *****/
const ru_field_rec XUMAC_RDP_XGMII_DATA_RATE_STATUS_XGMII_DATA_RATE_FIELD =
{
    "XGMII_DATA_RATE",
#if RU_INCLUDE_DESC
    "",
    "Indicates data rate over XGMII interface. Used to select GMII operating clocks.\nEncoded as:\n2'b00 - 2.5Gbps\n2'b01 - 5Gbps\n2'b10 - 10Gbps\n2'b11 - Reserved\nVAlid only wehn XGMII_SEL=1.\n",
#endif
    { XUMAC_RDP_XGMII_DATA_RATE_STATUS_XGMII_DATA_RATE_FIELD_MASK },
    0,
    { XUMAC_RDP_XGMII_DATA_RATE_STATUS_XGMII_DATA_RATE_FIELD_WIDTH },
    { XUMAC_RDP_XGMII_DATA_RATE_STATUS_XGMII_DATA_RATE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_XGMII_DATA_RATE_STATUS_FIELDS[] =
{
    &XUMAC_RDP_XGMII_DATA_RATE_STATUS_XGMII_DATA_RATE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_XGMII_DATA_RATE_STATUS *****/
const ru_reg_rec XUMAC_RDP_XGMII_DATA_RATE_STATUS_REG =
{
    "XGMII_DATA_RATE_STATUS",
#if RU_INCLUDE_DESC
    "XIB XGMII Data Rate Status Register",
    "",
#endif
    { XUMAC_RDP_XGMII_DATA_RATE_STATUS_REG_OFFSET },
    0,
    0,
    1343,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_XGMII_DATA_RATE_STATUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_STATUS, TYPE: Type_STATUS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RX_FIFO_OVERRUN *****/
const ru_field_rec XUMAC_RDP_STATUS_RX_FIFO_OVERRUN_FIELD =
{
    "RX_FIFO_OVERRUN",
#if RU_INCLUDE_DESC
    "",
    "When set indicates that RX FIFO had overflow.\nCleared on read.\n",
#endif
    { XUMAC_RDP_STATUS_RX_FIFO_OVERRUN_FIELD_MASK },
    0,
    { XUMAC_RDP_STATUS_RX_FIFO_OVERRUN_FIELD_WIDTH },
    { XUMAC_RDP_STATUS_RX_FIFO_OVERRUN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RX_FIFO_UNDERRUN *****/
const ru_field_rec XUMAC_RDP_STATUS_RX_FIFO_UNDERRUN_FIELD =
{
    "RX_FIFO_UNDERRUN",
#if RU_INCLUDE_DESC
    "",
    "When set indicates that RX FIFO become empty in the middle of the frame.\nCleared on read.\n",
#endif
    { XUMAC_RDP_STATUS_RX_FIFO_UNDERRUN_FIELD_MASK },
    0,
    { XUMAC_RDP_STATUS_RX_FIFO_UNDERRUN_FIELD_WIDTH },
    { XUMAC_RDP_STATUS_RX_FIFO_UNDERRUN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TX_FIFO_UNDERRUN *****/
const ru_field_rec XUMAC_RDP_STATUS_TX_FIFO_UNDERRUN_FIELD =
{
    "TX_FIFO_UNDERRUN",
#if RU_INCLUDE_DESC
    "",
    "When set indicates that TX FIFO become empty in the middle of the frame.\nCleared on read.\n",
#endif
    { XUMAC_RDP_STATUS_TX_FIFO_UNDERRUN_FIELD_MASK },
    0,
    { XUMAC_RDP_STATUS_TX_FIFO_UNDERRUN_FIELD_WIDTH },
    { XUMAC_RDP_STATUS_TX_FIFO_UNDERRUN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: TX_FIFO_OVERRUN *****/
const ru_field_rec XUMAC_RDP_STATUS_TX_FIFO_OVERRUN_FIELD =
{
    "TX_FIFO_OVERRUN",
#if RU_INCLUDE_DESC
    "",
    "When set indicates that TX FIFO had overflow.\nCleared on read.\n",
#endif
    { XUMAC_RDP_STATUS_TX_FIFO_OVERRUN_FIELD_MASK },
    0,
    { XUMAC_RDP_STATUS_TX_FIFO_OVERRUN_FIELD_WIDTH },
    { XUMAC_RDP_STATUS_TX_FIFO_OVERRUN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RX_FAULT_STATUS *****/
const ru_field_rec XUMAC_RDP_STATUS_RX_FAULT_STATUS_FIELD =
{
    "RX_FAULT_STATUS",
#if RU_INCLUDE_DESC
    "",
    "Received faults status encoded as:\n2'b00 - No Fault\n2'b01 - Local Fault\n2'b10 - Remote Fault\n2'b11 - Link Interruption\nCleared on read.\n",
#endif
    { XUMAC_RDP_STATUS_RX_FAULT_STATUS_FIELD_MASK },
    0,
    { XUMAC_RDP_STATUS_RX_FAULT_STATUS_FIELD_WIDTH },
    { XUMAC_RDP_STATUS_RX_FAULT_STATUS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_STATUS_FIELDS[] =
{
    &XUMAC_RDP_STATUS_RX_FIFO_OVERRUN_FIELD,
    &XUMAC_RDP_STATUS_RX_FIFO_UNDERRUN_FIELD,
    &XUMAC_RDP_STATUS_TX_FIFO_UNDERRUN_FIELD,
    &XUMAC_RDP_STATUS_TX_FIFO_OVERRUN_FIELD,
    &XUMAC_RDP_STATUS_RX_FAULT_STATUS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_STATUS *****/
const ru_reg_rec XUMAC_RDP_STATUS_REG =
{
    "STATUS",
#if RU_INCLUDE_DESC
    "XIB Status Register",
    "",
#endif
    { XUMAC_RDP_STATUS_REG_OFFSET },
    0,
    0,
    1344,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    XUMAC_RDP_STATUS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_RX_DISCARD_PACKET_COUNTER, TYPE: Type_RX_DISCARD_PACKET_COUNTER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PKT_COUNT *****/
const ru_field_rec XUMAC_RDP_RX_DISCARD_PACKET_COUNTER_PKT_COUNT_FIELD =
{
    "PKT_COUNT",
#if RU_INCLUDE_DESC
    "",
    "This counter is a free running counter that counts received packets that are discarded by XIB due to framing or other irregularities.\n",
#endif
    { XUMAC_RDP_RX_DISCARD_PACKET_COUNTER_PKT_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_RX_DISCARD_PACKET_COUNTER_PKT_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_RX_DISCARD_PACKET_COUNTER_PKT_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_RX_DISCARD_PACKET_COUNTER_FIELDS[] =
{
    &XUMAC_RDP_RX_DISCARD_PACKET_COUNTER_PKT_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_RX_DISCARD_PACKET_COUNTER *****/
const ru_reg_rec XUMAC_RDP_RX_DISCARD_PACKET_COUNTER_REG =
{
    "RX_DISCARD_PACKET_COUNTER",
#if RU_INCLUDE_DESC
    "XIB Receive Discard Packet Counter Register",
    "",
#endif
    { XUMAC_RDP_RX_DISCARD_PACKET_COUNTER_REG_OFFSET },
    0,
    0,
    1345,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_RX_DISCARD_PACKET_COUNTER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_TX_DISCARD_PACKET_COUNTER, TYPE: Type_TX_DISCARD_PACKET_COUNTER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PKT_COUNT *****/
const ru_field_rec XUMAC_RDP_TX_DISCARD_PACKET_COUNTER_PKT_COUNT_FIELD =
{
    "PKT_COUNT",
#if RU_INCLUDE_DESC
    "",
    "This counter is a free running counter that counts transmitted packets that are discarded by XIB due to framing or other irregularities.\n",
#endif
    { XUMAC_RDP_TX_DISCARD_PACKET_COUNTER_PKT_COUNT_FIELD_MASK },
    0,
    { XUMAC_RDP_TX_DISCARD_PACKET_COUNTER_PKT_COUNT_FIELD_WIDTH },
    { XUMAC_RDP_TX_DISCARD_PACKET_COUNTER_PKT_COUNT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_TX_DISCARD_PACKET_COUNTER_FIELDS[] =
{
    &XUMAC_RDP_TX_DISCARD_PACKET_COUNTER_PKT_COUNT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_TX_DISCARD_PACKET_COUNTER *****/
const ru_reg_rec XUMAC_RDP_TX_DISCARD_PACKET_COUNTER_REG =
{
    "TX_DISCARD_PACKET_COUNTER",
#if RU_INCLUDE_DESC
    "XIB Transmit Discard Packet Counter Register",
    "",
#endif
    { XUMAC_RDP_TX_DISCARD_PACKET_COUNTER_REG_OFFSET },
    0,
    0,
    1346,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_TX_DISCARD_PACKET_COUNTER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_REV, TYPE: Type_REV
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SYS_PORT_REV *****/
const ru_field_rec XUMAC_RDP_REV_SYS_PORT_REV_FIELD =
{
    "SYS_PORT_REV",
#if RU_INCLUDE_DESC
    "",
    "XUMAC revision code.\n",
#endif
    { XUMAC_RDP_REV_SYS_PORT_REV_FIELD_MASK },
    0,
    { XUMAC_RDP_REV_SYS_PORT_REV_FIELD_WIDTH },
    { XUMAC_RDP_REV_SYS_PORT_REV_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_REV_FIELDS[] =
{
    &XUMAC_RDP_REV_SYS_PORT_REV_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_REV *****/
const ru_reg_rec XUMAC_RDP_REV_REG =
{
    "REV",
#if RU_INCLUDE_DESC
    "XUMAC Revision Register",
    "",
#endif
    { XUMAC_RDP_REV_REG_OFFSET },
    0,
    0,
    1347,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_REV_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_UMAC_RXERR_MASK, TYPE: Type_UMAC_RXERR_MASK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MAC_RXERR_MASK *****/
const ru_field_rec XUMAC_RDP_UMAC_RXERR_MASK_MAC_RXERR_MASK_FIELD =
{
    "MAC_RXERR_MASK",
#if RU_INCLUDE_DESC
    "",
    "Mask for RSV[33:16].\nThe effective MAC_RXERR will be: |(RSV[33:16] & UMAC_RXERR_MASK)\n",
#endif
    { XUMAC_RDP_UMAC_RXERR_MASK_MAC_RXERR_MASK_FIELD_MASK },
    0,
    { XUMAC_RDP_UMAC_RXERR_MASK_MAC_RXERR_MASK_FIELD_WIDTH },
    { XUMAC_RDP_UMAC_RXERR_MASK_MAC_RXERR_MASK_FIELD_SHIFT },
    197656,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_UMAC_RXERR_MASK_FIELDS[] =
{
    &XUMAC_RDP_UMAC_RXERR_MASK_MAC_RXERR_MASK_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_UMAC_RXERR_MASK *****/
const ru_reg_rec XUMAC_RDP_UMAC_RXERR_MASK_REG =
{
    "UMAC_RXERR_MASK",
#if RU_INCLUDE_DESC
    "UniMAC RXERR Mask Register",
    "",
#endif
    { XUMAC_RDP_UMAC_RXERR_MASK_REG_OFFSET },
    0,
    0,
    1348,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_UMAC_RXERR_MASK_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XUMAC_RDP_MIB_MAX_PKT_SIZE, TYPE: Type_MIB_MAX_PKT_SIZE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MAX_PKT_SIZE *****/
const ru_field_rec XUMAC_RDP_MIB_MAX_PKT_SIZE_MAX_PKT_SIZE_FIELD =
{
    "MAX_PKT_SIZE",
#if RU_INCLUDE_DESC
    "",
    "This value is used by MIB counters to differentiate regular size packets from oversized packets (used for statistics counting only).\n",
#endif
    { XUMAC_RDP_MIB_MAX_PKT_SIZE_MAX_PKT_SIZE_FIELD_MASK },
    0,
    { XUMAC_RDP_MIB_MAX_PKT_SIZE_MAX_PKT_SIZE_FIELD_WIDTH },
    { XUMAC_RDP_MIB_MAX_PKT_SIZE_MAX_PKT_SIZE_FIELD_SHIFT },
    1522,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XUMAC_RDP_MIB_MAX_PKT_SIZE_FIELDS[] =
{
    &XUMAC_RDP_MIB_MAX_PKT_SIZE_MAX_PKT_SIZE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XUMAC_RDP_MIB_MAX_PKT_SIZE *****/
const ru_reg_rec XUMAC_RDP_MIB_MAX_PKT_SIZE_REG =
{
    "MIB_MAX_PKT_SIZE",
#if RU_INCLUDE_DESC
    "MIB Max Packet Size Register",
    "",
#endif
    { XUMAC_RDP_MIB_MAX_PKT_SIZE_REG_OFFSET },
    0,
    0,
    1349,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XUMAC_RDP_MIB_MAX_PKT_SIZE_FIELDS,
#endif
};

unsigned long XUMAC_RDP_ADDRS[] =
{
    0x828A8000,
    0x828A9000,
    0x828AA000,
    0x828AB000,
};

static const ru_reg_rec *XUMAC_RDP_REGS[] =
{
    &XUMAC_RDP_IPG_HD_BKP_CNTL_REG,
    &XUMAC_RDP_COMMAND_CONFIG_REG,
    &XUMAC_RDP_MAC_0_REG,
    &XUMAC_RDP_MAC_1_REG,
    &XUMAC_RDP_FRM_LENGTH_REG,
    &XUMAC_RDP_PAUSE_QUANT_REG,
    &XUMAC_RDP_TX_TS_SEQ_ID_REG,
    &XUMAC_RDP_SFD_OFFSET_REG,
    &XUMAC_RDP_MAC_MODE_REG,
    &XUMAC_RDP_TAG_0_REG,
    &XUMAC_RDP_TAG_1_REG,
    &XUMAC_RDP_RX_PAUSE_QUANTA_SCALE_REG,
    &XUMAC_RDP_TX_PREAMBLE_REG,
    &XUMAC_RDP_TX_IPG_LENGTH_REG,
    &XUMAC_RDP_PFC_XOFF_TIMER_REG,
    &XUMAC_RDP_UMAC_EEE_CTRL_REG,
    &XUMAC_RDP_MII_EEE_DELAY_ENTRY_TIMER_REG,
    &XUMAC_RDP_GMII_EEE_DELAY_ENTRY_TIMER_REG,
    &XUMAC_RDP_UMAC_EEE_REF_COUNT_REG,
    &XUMAC_RDP_UMAC_TIMESTAMP_ADJUST_REG,
    &XUMAC_RDP_UMAC_RX_PKT_DROP_STATUS_REG,
    &XUMAC_RDP_UMAC_SYMMETRIC_IDLE_THRESHOLD_REG,
    &XUMAC_RDP_MII_EEE_WAKE_TIMER_REG,
    &XUMAC_RDP_GMII_EEE_WAKE_TIMER_REG,
    &XUMAC_RDP_UMAC_REV_ID_REG,
    &XUMAC_RDP_GMII_2P5G_EEE_DELAY_ENTRY_TIMER_REG,
    &XUMAC_RDP_GMII_5G_EEE_DELAY_ENTRY_TIMER_REG,
    &XUMAC_RDP_GMII_10G_EEE_DELAY_ENTRY_TIMER_REG,
    &XUMAC_RDP_GMII_2P5G_EEE_WAKE_TIMER_REG,
    &XUMAC_RDP_GMII_5G_EEE_WAKE_TIMER_REG,
    &XUMAC_RDP_GMII_10G_EEE_WAKE_TIMER_REG,
    &XUMAC_RDP_ACTIVE_EEE_DELAY_ENTRY_TIMER_REG,
    &XUMAC_RDP_ACTIVE_EEE_WAKE_TIMER_REG,
    &XUMAC_RDP_MAC_PFC_TYPE_REG,
    &XUMAC_RDP_MAC_PFC_OPCODE_REG,
    &XUMAC_RDP_MAC_PFC_DA_0_REG,
    &XUMAC_RDP_MAC_PFC_DA_1_REG,
    &XUMAC_RDP_MACSEC_PROG_TX_CRC_REG,
    &XUMAC_RDP_MACSEC_CNTRL_REG,
    &XUMAC_RDP_TS_STATUS_REG,
    &XUMAC_RDP_TX_TS_DATA_REG,
    &XUMAC_RDP_PAUSE_REFRESH_CTRL_REG,
    &XUMAC_RDP_FLUSH_CONTROL_REG,
    &XUMAC_RDP_RXFIFO_STAT_REG,
    &XUMAC_RDP_TXFIFO_STAT_REG,
    &XUMAC_RDP_MAC_PFC_CTRL_REG,
    &XUMAC_RDP_MAC_PFC_REFRESH_CTRL_REG,
    &XUMAC_RDP_GR64_REG,
    &XUMAC_RDP_GR64_UPPER_REG,
    &XUMAC_RDP_GR127_REG,
    &XUMAC_RDP_GR127_UPPER_REG,
    &XUMAC_RDP_GR255_REG,
    &XUMAC_RDP_GR255_UPPER_REG,
    &XUMAC_RDP_GR511_REG,
    &XUMAC_RDP_GR511_UPPER_REG,
    &XUMAC_RDP_GR1023_REG,
    &XUMAC_RDP_GR1023_UPPER_REG,
    &XUMAC_RDP_GR1518_REG,
    &XUMAC_RDP_GR1518_UPPER_REG,
    &XUMAC_RDP_GRMGV_REG,
    &XUMAC_RDP_GRMGV_UPPER_REG,
    &XUMAC_RDP_GR2047_REG,
    &XUMAC_RDP_GR2047_UPPER_REG,
    &XUMAC_RDP_GR4095_REG,
    &XUMAC_RDP_GR4095_UPPER_REG,
    &XUMAC_RDP_GR9216_REG,
    &XUMAC_RDP_GR9216_UPPER_REG,
    &XUMAC_RDP_GRPKT_REG,
    &XUMAC_RDP_GRPKT_UPPER_REG,
    &XUMAC_RDP_GRBYT_REG,
    &XUMAC_RDP_GRBYT_UPPER_REG,
    &XUMAC_RDP_GRMCA_REG,
    &XUMAC_RDP_GRMCA_UPPER_REG,
    &XUMAC_RDP_GRBCA_REG,
    &XUMAC_RDP_GRBCA_UPPER_REG,
    &XUMAC_RDP_GRFCS_REG,
    &XUMAC_RDP_GRFCS_UPPER_REG,
    &XUMAC_RDP_GRXCF_REG,
    &XUMAC_RDP_GRXCF_UPPER_REG,
    &XUMAC_RDP_GRXPF_REG,
    &XUMAC_RDP_GRXPF_UPPER_REG,
    &XUMAC_RDP_GRXUO_REG,
    &XUMAC_RDP_GRXUO_UPPER_REG,
    &XUMAC_RDP_GRALN_REG,
    &XUMAC_RDP_GRALN_UPPER_REG,
    &XUMAC_RDP_GRFLR_REG,
    &XUMAC_RDP_GRFLR_UPPER_REG,
    &XUMAC_RDP_GRCDE_REG,
    &XUMAC_RDP_GRCDE_UPPER_REG,
    &XUMAC_RDP_GRFCR_REG,
    &XUMAC_RDP_GRFCR_UPPER_REG,
    &XUMAC_RDP_GROVR_REG,
    &XUMAC_RDP_GROVR_UPPER_REG,
    &XUMAC_RDP_GRJBR_REG,
    &XUMAC_RDP_GRJBR_UPPER_REG,
    &XUMAC_RDP_GRMTUE_REG,
    &XUMAC_RDP_GRMTUE_UPPER_REG,
    &XUMAC_RDP_GRPOK_REG,
    &XUMAC_RDP_GRPOK_UPPER_REG,
    &XUMAC_RDP_GRUC_REG,
    &XUMAC_RDP_GRUC_UPPER_REG,
    &XUMAC_RDP_GRPPP_REG,
    &XUMAC_RDP_GRPPP_UPPER_REG,
    &XUMAC_RDP_GRCRC_REG,
    &XUMAC_RDP_GRCRC_UPPER_REG,
    &XUMAC_RDP_TR64_REG,
    &XUMAC_RDP_TR64_UPPER_REG,
    &XUMAC_RDP_TR127_REG,
    &XUMAC_RDP_TR127_UPPER_REG,
    &XUMAC_RDP_TR255_REG,
    &XUMAC_RDP_TR255_UPPER_REG,
    &XUMAC_RDP_TR511_REG,
    &XUMAC_RDP_TR511_UPPER_REG,
    &XUMAC_RDP_TR1023_REG,
    &XUMAC_RDP_TR1023_UPPER_REG,
    &XUMAC_RDP_TR1518_REG,
    &XUMAC_RDP_TR1518_UPPER_REG,
    &XUMAC_RDP_TRMGV_REG,
    &XUMAC_RDP_TRMGV_UPPER_REG,
    &XUMAC_RDP_TR2047_REG,
    &XUMAC_RDP_TR2047_UPPER_REG,
    &XUMAC_RDP_TR4095_REG,
    &XUMAC_RDP_TR4095_UPPER_REG,
    &XUMAC_RDP_TR9216_REG,
    &XUMAC_RDP_TR9216_UPPER_REG,
    &XUMAC_RDP_GTPKT_REG,
    &XUMAC_RDP_GTPKT_UPPER_REG,
    &XUMAC_RDP_GTMCA_REG,
    &XUMAC_RDP_GTMCA_UPPER_REG,
    &XUMAC_RDP_GTBCA_REG,
    &XUMAC_RDP_GTBCA_UPPER_REG,
    &XUMAC_RDP_GTXPF_REG,
    &XUMAC_RDP_GTXPF_UPPER_REG,
    &XUMAC_RDP_GTXCF_REG,
    &XUMAC_RDP_GTXCF_UPPER_REG,
    &XUMAC_RDP_GTFCS_REG,
    &XUMAC_RDP_GTFCS_UPPER_REG,
    &XUMAC_RDP_GTOVR_REG,
    &XUMAC_RDP_GTOVR_UPPER_REG,
    &XUMAC_RDP_GTDRF_REG,
    &XUMAC_RDP_GTDRF_UPPER_REG,
    &XUMAC_RDP_GTEDF_REG,
    &XUMAC_RDP_GTEDF_UPPER_REG,
    &XUMAC_RDP_GTSCL_REG,
    &XUMAC_RDP_GTSCL_UPPER_REG,
    &XUMAC_RDP_GTMCL_REG,
    &XUMAC_RDP_GTMCL_UPPER_REG,
    &XUMAC_RDP_GTLCL_REG,
    &XUMAC_RDP_GTLCL_UPPER_REG,
    &XUMAC_RDP_GTXCL_REG,
    &XUMAC_RDP_GTXCL_UPPER_REG,
    &XUMAC_RDP_GTFRG_REG,
    &XUMAC_RDP_GTFRG_UPPER_REG,
    &XUMAC_RDP_GTNCL_REG,
    &XUMAC_RDP_GTNCL_UPPER_REG,
    &XUMAC_RDP_GTJBR_REG,
    &XUMAC_RDP_GTJBR_UPPER_REG,
    &XUMAC_RDP_GTBYT_REG,
    &XUMAC_RDP_GTBYT_UPPER_REG,
    &XUMAC_RDP_GTPOK_REG,
    &XUMAC_RDP_GTPOK_UPPER_REG,
    &XUMAC_RDP_GTUC_REG,
    &XUMAC_RDP_GTUC_UPPER_REG,
    &XUMAC_RDP_RRPKT_REG,
    &XUMAC_RDP_RRPKT_UPPER_REG,
    &XUMAC_RDP_RRUND_REG,
    &XUMAC_RDP_RRUND_UPPER_REG,
    &XUMAC_RDP_RRFRG_REG,
    &XUMAC_RDP_RRFRG_UPPER_REG,
    &XUMAC_RDP_RRBYT_REG,
    &XUMAC_RDP_RRBYT_UPPER_REG,
    &XUMAC_RDP_MIB_CNTRL_REG,
    &XUMAC_RDP_MIB_READ_DATA_REG,
    &XUMAC_RDP_MIB_WRITE_DATA_REG,
    &XUMAC_RDP_CONTROL_REG,
    &XUMAC_RDP_PSW_MS_REG,
    &XUMAC_RDP_PSW_LS_REG,
    &XUMAC_RDP_CONTROL_1_REG,
    &XUMAC_RDP_EXTENDED_CONTROL_REG,
    &XUMAC_RDP_TX_IDLE_STUFFING_CONTROL_REG,
    &XUMAC_RDP_ACTUAL_DATA_RATE_REG,
    &XUMAC_RDP_GMII_CLOCK_SWALLOWER_CONTROL_REG,
    &XUMAC_RDP_XGMII_DATA_RATE_STATUS_REG,
    &XUMAC_RDP_STATUS_REG,
    &XUMAC_RDP_RX_DISCARD_PACKET_COUNTER_REG,
    &XUMAC_RDP_TX_DISCARD_PACKET_COUNTER_REG,
    &XUMAC_RDP_REV_REG,
    &XUMAC_RDP_UMAC_RXERR_MASK_REG,
    &XUMAC_RDP_MIB_MAX_PKT_SIZE_REG,
};

const ru_block_rec XUMAC_RDP_BLOCK =
{
    "XUMAC_RDP",
    XUMAC_RDP_ADDRS,
    4,
    189,
    XUMAC_RDP_REGS,
};
