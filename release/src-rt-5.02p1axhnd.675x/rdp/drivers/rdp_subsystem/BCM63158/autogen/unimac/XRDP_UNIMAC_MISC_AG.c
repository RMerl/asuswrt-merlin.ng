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
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_DIRECT_GMII
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_DIRECT_GMII_FIELD =
{
    "DIRECT_GMII",
#if RU_INCLUDE_DESC
    "direct_gmii",
    "Direct GMII mode. If direct set to 0, the only 25MHz clock needs to be fed at ref25, for both 10/100Mbps speeds",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_DIRECT_GMII_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_DIRECT_GMII_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_DIRECT_GMII_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_GPORT_MODE
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_GPORT_MODE_FIELD =
{
    "GPORT_MODE",
#if RU_INCLUDE_DESC
    "gport_mode",
    "GPORT mode to support legacy ENTSW gport/gxport RX FIFO protocol"
    "",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_GPORT_MODE_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_GPORT_MODE_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_GPORT_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_SS_MODE_MII
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_SS_MODE_MII_FIELD =
{
    "SS_MODE_MII",
#if RU_INCLUDE_DESC
    "ss_mode_mii",
    "Indicates whether the unimac should operate in source synchronous mode",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_SS_MODE_MII_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_SS_MODE_MII_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_SS_MODE_MII_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_TXCRCER
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_TXCRCER_FIELD =
{
    "TXCRCER",
#if RU_INCLUDE_DESC
    "txcrcer",
    "This when asserted along with MAC_TXEOP indicates a corrupt CRC of the packet"
    "",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_TXCRCER_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_TXCRCER_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_TXCRCER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_MAC_CRC_FWD
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_MAC_CRC_FWD_FIELD =
{
    "MAC_CRC_FWD",
#if RU_INCLUDE_DESC
    "mac_crc_fwd",
    "Forward CRC",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_MAC_CRC_FWD_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_MAC_CRC_FWD_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_MAC_CRC_FWD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_MAC_CRC_OWRT
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_MAC_CRC_OWRT_FIELD =
{
    "MAC_CRC_OWRT",
#if RU_INCLUDE_DESC
    "mac_crc_owrt",
    "Overwrite CRC",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_MAC_CRC_OWRT_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_MAC_CRC_OWRT_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_MAC_CRC_OWRT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_EXT_TX_FLOW_CONTROL
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_EXT_TX_FLOW_CONTROL_FIELD =
{
    "EXT_TX_FLOW_CONTROL",
#if RU_INCLUDE_DESC
    "ext_tx_flow_control",
    "When this active high signal is asserted and config bit OOB_EFC_EN is set, then data frame transmission is stopped",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_EXT_TX_FLOW_CONTROL_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_EXT_TX_FLOW_CONTROL_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_EXT_TX_FLOW_CONTROL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_LAUNCH_ENABLE
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_LAUNCH_ENABLE_FIELD =
{
    "LAUNCH_ENABLE",
#if RU_INCLUDE_DESC
    "launch_enable",
    "If enabled through register programming, the launch enable rising edge enables the packet transmit on per packet basis",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_LAUNCH_ENABLE_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_LAUNCH_ENABLE_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_LAUNCH_ENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_PP_PSE_EN
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_PP_PSE_EN_FIELD =
{
    "PP_PSE_EN",
#if RU_INCLUDE_DESC
    "pp_pse_en",
    "The corresponding class enable vector of PPP control frame",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_PP_PSE_EN_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_PP_PSE_EN_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_PP_PSE_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_PP_GEN
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_PP_GEN_FIELD =
{
    "PP_GEN",
#if RU_INCLUDE_DESC
    "pp_gen",
    "Instructs MAC to generate a PPP control frame",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_PP_GEN_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_PP_GEN_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_PP_GEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_RESERVED0_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1_MAX_PKT_SIZE
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1_MAX_PKT_SIZE_FIELD =
{
    "MAX_PKT_SIZE",
#if RU_INCLUDE_DESC
    "max_pkt_size",
    "Max Packet Size - Used fro statistics",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1_MAX_PKT_SIZE_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1_MAX_PKT_SIZE_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1_MAX_PKT_SIZE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1_RESERVED0_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1_RXFIFO_CONGESTION_THRESHOLD
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1_RXFIFO_CONGESTION_THRESHOLD_FIELD =
{
    "RXFIFO_CONGESTION_THRESHOLD",
#if RU_INCLUDE_DESC
    "rxfifo_congestion_threshold",
    "RX FIFO Threshold - This is the fifo located between the UNIMAC IP and BBH. Once the threshold is reached, rx_fifo_congestion output is set."
    "This configuration is in 16-byte resolution (the number of bytes in a FIFO line)."
    "Max value is: 0x100.",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1_RXFIFO_CONGESTION_THRESHOLD_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1_RXFIFO_CONGESTION_THRESHOLD_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1_RXFIFO_CONGESTION_THRESHOLD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1_RESERVED1
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1_RESERVED1_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1_RESERVED1_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_RXFIFO_PAUSE_THRESHOLD
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_RXFIFO_PAUSE_THRESHOLD_FIELD =
{
    "RXFIFO_PAUSE_THRESHOLD",
#if RU_INCLUDE_DESC
    "rxfifo_pause_threshold",
    "RX FIFO Threshold - This is the fifo located between the UNIMAC IP and BBH. Once the threshold is reached, pause is sent. This configuration is in 16-byte resolution (the number of bytes in a FIFO line)."
    "Max Value is 0x100.",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_RXFIFO_PAUSE_THRESHOLD_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_RXFIFO_PAUSE_THRESHOLD_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_RXFIFO_PAUSE_THRESHOLD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_RESERVED0_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_BACKPRESSURE_ENABLE_INT
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_BACKPRESSURE_ENABLE_INT_FIELD =
{
    "BACKPRESSURE_ENABLE_INT",
#if RU_INCLUDE_DESC
    "backpressure_enable_int",
    "Backpressure enable for internal unimac",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_BACKPRESSURE_ENABLE_INT_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_BACKPRESSURE_ENABLE_INT_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_BACKPRESSURE_ENABLE_INT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_BACKPRESSURE_ENABLE_EXT
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_BACKPRESSURE_ENABLE_EXT_FIELD =
{
    "BACKPRESSURE_ENABLE_EXT",
#if RU_INCLUDE_DESC
    "backpressure_enable_ext",
    "Backpressure enable for external switch",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_BACKPRESSURE_ENABLE_EXT_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_BACKPRESSURE_ENABLE_EXT_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_BACKPRESSURE_ENABLE_EXT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_FIFO_OVERRUN_CTL_EN
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_FIFO_OVERRUN_CTL_EN_FIELD =
{
    "FIFO_OVERRUN_CTL_EN",
#if RU_INCLUDE_DESC
    "fifo_overrun_ctl_en",
    "Enable the mechanism for always receiving data from UNIMAC IP and dropping overrun words in the unimac_glue FIFO.",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_FIFO_OVERRUN_CTL_EN_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_FIFO_OVERRUN_CTL_EN_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_FIFO_OVERRUN_CTL_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_REMOTE_LOOPBACK_EN
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_REMOTE_LOOPBACK_EN_FIELD =
{
    "REMOTE_LOOPBACK_EN",
#if RU_INCLUDE_DESC
    "remote_loopback_en",
    "When setting this bit, RX recovered clock will also input the clk25/pll_ref_clk in the UNIMAC.",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_REMOTE_LOOPBACK_EN_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_REMOTE_LOOPBACK_EN_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_REMOTE_LOOPBACK_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_RESERVED1
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_RESERVED1_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_RESERVED1_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_UPDATE_MASK_GPORT_STAT_UPDATE_MASK
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_UPDATE_MASK_GPORT_STAT_UPDATE_MASK_FIELD =
{
    "GPORT_STAT_UPDATE_MASK",
#if RU_INCLUDE_DESC
    "gport_stat_update_mask",
    "Mask bits [32:16] of RSV. 1 indicates update disabled for the respective error condition.",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_UPDATE_MASK_GPORT_STAT_UPDATE_MASK_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_UPDATE_MASK_GPORT_STAT_UPDATE_MASK_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_UPDATE_MASK_GPORT_STAT_UPDATE_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_UPDATE_MASK_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_UPDATE_MASK_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_UPDATE_MASK_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_UPDATE_MASK_RESERVED0_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_UPDATE_MASK_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_AUTO_CONFIG_EN
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_AUTO_CONFIG_EN_FIELD =
{
    "AUTO_CONFIG_EN",
#if RU_INCLUDE_DESC
    "auto_config_en",
    "Indicates auto config mode is enabled"
    "",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_AUTO_CONFIG_EN_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_AUTO_CONFIG_EN_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_AUTO_CONFIG_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_ETH_DUPLEX
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_ETH_DUPLEX_FIELD =
{
    "ETH_DUPLEX",
#if RU_INCLUDE_DESC
    "eth_duplex",
    "Indicates current Duplex mode (0 - full duplex, 1 - half duplex)"
    "",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_ETH_DUPLEX_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_ETH_DUPLEX_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_ETH_DUPLEX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_ETH_SPEED
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_ETH_SPEED_FIELD =
{
    "ETH_SPEED",
#if RU_INCLUDE_DESC
    "eth_speed",
    "Indicates current speed (0 - 10Mbps, 1 - 100Mbps, 2 - 1Gbps, 3 - 2.5Gbps)"
    "",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_ETH_SPEED_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_ETH_SPEED_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_ETH_SPEED_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_RX_FIFO_DAT_AVL
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_RX_FIFO_DAT_AVL_FIELD =
{
    "RX_FIFO_DAT_AVL",
#if RU_INCLUDE_DESC
    "rx_fifo_dat_avl",
    "Indicates when onde or more entries in UNIMAC FIFO are full. Used for power management purposes"
    "",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_RX_FIFO_DAT_AVL_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_RX_FIFO_DAT_AVL_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_RX_FIFO_DAT_AVL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_MAC_IN_PAUSE
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_MAC_IN_PAUSE_FIELD =
{
    "MAC_IN_PAUSE",
#if RU_INCLUDE_DESC
    "mac_in_pause",
    "Asserted when MAC_tx is paused",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_MAC_IN_PAUSE_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_MAC_IN_PAUSE_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_MAC_IN_PAUSE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_MAC_TX_EMPTY
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_MAC_TX_EMPTY_FIELD =
{
    "MAC_TX_EMPTY",
#if RU_INCLUDE_DESC
    "mac_tx_empty",
    "Indicates that MAC Transmit Path is Empty"
    "",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_MAC_TX_EMPTY_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_MAC_TX_EMPTY_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_MAC_TX_EMPTY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_LAUNCH_ACK
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_LAUNCH_ACK_FIELD =
{
    "LAUNCH_ACK",
#if RU_INCLUDE_DESC
    "launch_ack",
    "Acknowledges launch enable after the packet starts transmission. Goes low after launch_ena goes low",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_LAUNCH_ACK_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_LAUNCH_ACK_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_LAUNCH_ACK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_LPI_TX_DETECT
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_LPI_TX_DETECT_FIELD =
{
    "LPI_TX_DETECT",
#if RU_INCLUDE_DESC
    "lpi_tx_detect",
    "Transmit LPI State - Set to 1 whenever LPI_IDLES are being Sent out on the TX Line",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_LPI_TX_DETECT_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_LPI_TX_DETECT_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_LPI_TX_DETECT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_LPI_RX_DETECT
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_LPI_RX_DETECT_FIELD =
{
    "LPI_RX_DETECT",
#if RU_INCLUDE_DESC
    "lpi_rx_detect",
    "Receive LPI State - Set to 1 whenever LPI_IDLES are being received on the RX Line",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_LPI_RX_DETECT_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_LPI_RX_DETECT_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_LPI_RX_DETECT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_RX_SOP_OUT
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_RX_SOP_OUT_FIELD =
{
    "RX_SOP_OUT",
#if RU_INCLUDE_DESC
    "rx_sop_out",
    "SOP detected for the received packet"
    "",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_RX_SOP_OUT_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_RX_SOP_OUT_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_RX_SOP_OUT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_RX_SOP_DELETE
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_RX_SOP_DELETE_FIELD =
{
    "RX_SOP_DELETE",
#if RU_INCLUDE_DESC
    "rx_sop_delete",
    "The received packet is dropped"
    "",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_RX_SOP_DELETE_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_RX_SOP_DELETE_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_RX_SOP_DELETE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_MAC_STATS_UPDATE
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_MAC_STATS_UPDATE_FIELD =
{
    "MAC_STATS_UPDATE",
#if RU_INCLUDE_DESC
    "mac_stats_update",
    "MAC STATS need to be updated",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_MAC_STATS_UPDATE_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_MAC_STATS_UPDATE_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_MAC_STATS_UPDATE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_RESERVED0_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_PP_STATS
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_PP_STATS_FIELD =
{
    "PP_STATS",
#if RU_INCLUDE_DESC
    "pp_stats",
    "The XOFF/XON status of the receiving PPP control frame"
    "",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_PP_STATS_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_PP_STATS_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_PP_STATS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_PP_STATS_VALID
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_PP_STATS_VALID_FIELD =
{
    "PP_STATS_VALID",
#if RU_INCLUDE_DESC
    "pp_stats_valid",
    "MAC to indicate a PPP control frame is received"
    "",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_PP_STATS_VALID_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_PP_STATS_VALID_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_PP_STATS_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_RESERVED1
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_RESERVED1_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_RESERVED1_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_DEBUG_DEBUG_SEL
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_DEBUG_DEBUG_SEL_FIELD =
{
    "DEBUG_SEL",
#if RU_INCLUDE_DESC
    "debug_sel",
    "Select which debug bus to output.",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_DEBUG_DEBUG_SEL_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_DEBUG_DEBUG_SEL_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_DEBUG_DEBUG_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_DEBUG_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_DEBUG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_DEBUG_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_DEBUG_RESERVED0_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_DEBUG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RST_UNIMAC_RST
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RST_UNIMAC_RST_FIELD =
{
    "UNIMAC_RST",
#if RU_INCLUDE_DESC
    "unimac_rst",
    "Synchronous UNIMAC reset",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RST_UNIMAC_RST_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RST_UNIMAC_RST_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RST_UNIMAC_RST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RST_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RST_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RST_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RST_RESERVED0_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RST_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RSV_MASK_GPORT_RSV_MASK
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RSV_MASK_GPORT_RSV_MASK_FIELD =
{
    "GPORT_RSV_MASK",
#if RU_INCLUDE_DESC
    "gport_rsv_mask",
    "Mask bits [32:16] of RSV. 1 indicates purge enabled for the respective error condition.",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RSV_MASK_GPORT_RSV_MASK_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RSV_MASK_GPORT_RSV_MASK_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RSV_MASK_GPORT_RSV_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RSV_MASK_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RSV_MASK_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RSV_MASK_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RSV_MASK_RESERVED0_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RSV_MASK_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_OVERRUN_COUNTER_OVERRUN_COUNTER
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_OVERRUN_COUNTER_OVERRUN_COUNTER_FIELD =
{
    "OVERRUN_COUNTER",
#if RU_INCLUDE_DESC
    "overrun_counter",
    "This registers holds the number of 128-bit lines that were received from the UNIMAC IP, but were thrown due to the fact that the unimac_glue FIFO was full. This field stops counting when it reaches 0xff and will be cleared upon read.",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_OVERRUN_COUNTER_OVERRUN_COUNTER_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_OVERRUN_COUNTER_OVERRUN_COUNTER_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_OVERRUN_COUNTER_OVERRUN_COUNTER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_TSI_SIGN_EXT
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_TSI_SIGN_EXT_FIELD =
{
    "TSI_SIGN_EXT",
#if RU_INCLUDE_DESC
    "tsi_sign_ext",
    "The MAC logic will need to evaluate this bit in order to determine how to sign extend the Timestamp (TSe)."
    "If the bit is a zero, the TSe MUST be sign extended with zeros."
    "If the bit is a one, the TSe MUST be sign extended based on the Msbit of the TSe. That is, if the Msbit of the TSe is zero, the TSe is signed extended with zeros. If the Msbit of the TSe is one, the TSe is sign extended with one. (from unimac spec)"
    "",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_TSI_SIGN_EXT_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_TSI_SIGN_EXT_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_TSI_SIGN_EXT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_OSTS_TIMER_DIS
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_OSTS_TIMER_DIS_FIELD =
{
    "OSTS_TIMER_DIS",
#if RU_INCLUDE_DESC
    "osts_timer_disable",
    "This active high signal is used to disable the OSTS time-stamping function in the MAC when asserted during the time the CPU Is initializing the local counters.(from unimac spec)",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_OSTS_TIMER_DIS_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_OSTS_TIMER_DIS_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_OSTS_TIMER_DIS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_EGR_1588_TS_MODE
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_EGR_1588_TS_MODE_FIELD =
{
    "EGR_1588_TS_MODE",
#if RU_INCLUDE_DESC
    "egr_1588_timestamping_mode",
    "When set to 0, enables legacy sign-extension of 32-bit timestamp, when 1, enables 48-bit time-stamping. (from unimac spec)",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_EGR_1588_TS_MODE_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_EGR_1588_TS_MODE_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_EGR_1588_TS_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_RESERVED0_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISR_GEN_INT
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISR_GEN_INT_FIELD =
{
    "GEN_INT",
#if RU_INCLUDE_DESC
    "general_int",
    "general interrupt",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISR_GEN_INT_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISR_GEN_INT_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISR_GEN_INT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISR_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISR_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISR_RESERVED0_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_IER_VALUE
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_IER_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "Value",
    "The bit description is the same like for the ISR.",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_IER_VALUE_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_IER_VALUE_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_IER_VALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_IER_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_IER_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_IER_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_IER_RESERVED0_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_IER_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ITR_VALUE
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ITR_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "Value",
    "The bit description is the same like for the ISR.",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ITR_VALUE_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ITR_VALUE_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ITR_VALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ITR_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ITR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ITR_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ITR_RESERVED0_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ITR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISM_VALUE
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISM_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "Value",
    "The bit description is the same like for the ISR.",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISM_VALUE_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISM_VALUE_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISM_VALUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISM_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISM_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISM_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISM_RESERVED0_FIELD_WIDTH,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISM_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_FIELDS[] =
{
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_DIRECT_GMII_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_GPORT_MODE_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_SS_MODE_MII_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_TXCRCER_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_MAC_CRC_FWD_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_MAC_CRC_OWRT_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_EXT_TX_FLOW_CONTROL_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_LAUNCH_ENABLE_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_PP_PSE_EN_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_PP_GEN_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_REG = 
{
    "UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG",
#if RU_INCLUDE_DESC
    "UNIMAC_CFG Register",
    "Interface static configuration values",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_REG_OFFSET,
    0,
    0,
    792,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    11,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1_FIELDS[] =
{
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1_MAX_PKT_SIZE_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1_RESERVED0_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1_RXFIFO_CONGESTION_THRESHOLD_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1_REG = 
{
    "UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1",
#if RU_INCLUDE_DESC
    "UNIMAC_EXT_CFG1 Register",
    "Configure additional parameters that influence UNIMAC external logic.",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1_REG_OFFSET,
    0,
    0,
    793,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_FIELDS[] =
{
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_RXFIFO_PAUSE_THRESHOLD_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_RESERVED0_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_BACKPRESSURE_ENABLE_INT_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_BACKPRESSURE_ENABLE_EXT_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_FIFO_OVERRUN_CTL_EN_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_REMOTE_LOOPBACK_EN_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_REG = 
{
    "UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2",
#if RU_INCLUDE_DESC
    "UNIMAC_EXT_CFG2 Register",
    "Configure additional parameters that influence UNIMAC external logic.",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_REG_OFFSET,
    0,
    0,
    794,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_UPDATE_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_UPDATE_MASK_FIELDS[] =
{
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_UPDATE_MASK_GPORT_STAT_UPDATE_MASK_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_UPDATE_MASK_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_UPDATE_MASK_REG = 
{
    "UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_UPDATE_MASK",
#if RU_INCLUDE_DESC
    "UNIMAC_STAT_UPDATE_MASK Register",
    "Mask bits [32:16] of RSV. 1 indicates update disabled for the respective error condition.",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_UPDATE_MASK_REG_OFFSET,
    0,
    0,
    795,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_UPDATE_MASK_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_FIELDS[] =
{
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_AUTO_CONFIG_EN_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_ETH_DUPLEX_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_ETH_SPEED_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_RX_FIFO_DAT_AVL_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_MAC_IN_PAUSE_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_MAC_TX_EMPTY_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_LAUNCH_ACK_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_LPI_TX_DETECT_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_LPI_RX_DETECT_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_RX_SOP_OUT_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_RX_SOP_DELETE_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_MAC_STATS_UPDATE_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_RESERVED0_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_PP_STATS_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_PP_STATS_VALID_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_REG = 
{
    "UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT",
#if RU_INCLUDE_DESC
    "UNIMAC_STAT Register",
    "This registers holds status indications of the UNIMAC",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_REG_OFFSET,
    0,
    0,
    796,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    16,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_DEBUG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_DEBUG_FIELDS[] =
{
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_DEBUG_DEBUG_SEL_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_DEBUG_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_DEBUG_REG = 
{
    "UNIMAC_TOP_UNIMAC_MISC_UNIMAC_DEBUG",
#if RU_INCLUDE_DESC
    "UNIMAC_DEBUG Register",
    "This register holds all debug related configurations",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_DEBUG_REG_OFFSET,
    0,
    0,
    797,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_DEBUG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RST
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RST_FIELDS[] =
{
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RST_UNIMAC_RST_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RST_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RST_REG = 
{
    "UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RST",
#if RU_INCLUDE_DESC
    "UNIMAC_RST Register",
    "This is a synchronous reset to the UNIMAC core",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RST_REG_OFFSET,
    0,
    0,
    798,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RST_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RSV_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RSV_MASK_FIELDS[] =
{
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RSV_MASK_GPORT_RSV_MASK_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RSV_MASK_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RSV_MASK_REG = 
{
    "UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RSV_MASK",
#if RU_INCLUDE_DESC
    "UNIMAC_RSV_MASK Register",
    "Mask bits [32:16] of RSV. 1 indicates purge enabled for the respective error condition.",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RSV_MASK_REG_OFFSET,
    0,
    0,
    799,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RSV_MASK_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_OVERRUN_COUNTER
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_OVERRUN_COUNTER_FIELDS[] =
{
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_OVERRUN_COUNTER_OVERRUN_COUNTER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_OVERRUN_COUNTER_REG = 
{
    "UNIMAC_TOP_UNIMAC_MISC_UNIMAC_OVERRUN_COUNTER",
#if RU_INCLUDE_DESC
    "UNIMAC_OVERRUN_COUNTER Register",
    "This registers holds the number of 128-bit lines that were received from the UNIMAC IP, but were thrown due to the fact that the unimac_glue FIFO was full.",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_OVERRUN_COUNTER_REG_OFFSET,
    0,
    0,
    800,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_OVERRUN_COUNTER_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_FIELDS[] =
{
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_TSI_SIGN_EXT_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_OSTS_TIMER_DIS_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_EGR_1588_TS_MODE_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_REG = 
{
    "UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588",
#if RU_INCLUDE_DESC
    "UNIMAC_1588 Register",
    "1588 configurations",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_REG_OFFSET,
    0,
    0,
    801,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISR_FIELDS[] =
{
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISR_GEN_INT_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISR_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISR_REG = 
{
    "UNIMAC_TOP_UNIMAC_INTS_ISR",
#if RU_INCLUDE_DESC
    "ISR Register",
    "This register provides status bits for all the general interrupt sources.",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISR_REG_OFFSET,
    0,
    0,
    802,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_IER
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_IER_FIELDS[] =
{
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_IER_VALUE_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_IER_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_IER_REG = 
{
    "UNIMAC_TOP_UNIMAC_INTS_IER",
#if RU_INCLUDE_DESC
    "IER Register",
    "This register provides enable bits for all the general interrupt sources."
    "The bit description is the same like for the ISR.",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_IER_REG_OFFSET,
    0,
    0,
    803,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_IER_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ITR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ITR_FIELDS[] =
{
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ITR_VALUE_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ITR_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ITR_REG = 
{
    "UNIMAC_TOP_UNIMAC_INTS_ITR",
#if RU_INCLUDE_DESC
    "ITR Register",
    "This register enables alarm interrupt testing. Writing 1 to a bit in this register sets the corresponding bit in the ISR register. Read always returns 0."
    "The bit description is the same like for the ISR.",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ITR_REG_OFFSET,
    0,
    0,
    804,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ITR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISM
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISM_FIELDS[] =
{
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISM_VALUE_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISM_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISM_REG = 
{
    "UNIMAC_TOP_UNIMAC_INTS_ISM",
#if RU_INCLUDE_DESC
    "ISM Register",
    "This read only register provides masked status bits. Each bit corresponds to logical AND operation between the corresponding bits of the ISR and IER registers.The bit description is the same like for the ISR.",
#endif
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISM_REG_OFFSET,
    0,
    0,
    805,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISM_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Block: UNIMAC_MISC
 ******************************************************************************/
static const ru_reg_rec *UNIMAC_MISC_REGS[] =
{
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_REG,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1_REG,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_REG,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_UPDATE_MASK_REG,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_REG,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_DEBUG_REG,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RST_REG,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RSV_MASK_REG,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_OVERRUN_COUNTER_REG,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_REG,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISR_REG,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_IER_REG,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ITR_REG,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISM_REG,
};

unsigned long UNIMAC_MISC_ADDRS[] =
{
    0x82daa000,
    0x82daa400,
    0x82daa800,
};

const ru_block_rec UNIMAC_MISC_BLOCK = 
{
    "UNIMAC_MISC",
    UNIMAC_MISC_ADDRS,
    3,
    14,
    UNIMAC_MISC_REGS
};

/* End of file XRDP_UNIMAC_MISC.c */
