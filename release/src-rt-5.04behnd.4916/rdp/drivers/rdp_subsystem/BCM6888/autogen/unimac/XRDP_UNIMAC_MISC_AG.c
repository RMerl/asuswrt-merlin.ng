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


#include "XRDP_UNIMAC_MISC_AG.h"

/******************************************************************************
 * Register: NAME: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG, TYPE: Type_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MAC_CRC_FWD *****/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_MAC_CRC_FWD_FIELD =
{
    "MAC_CRC_FWD",
#if RU_INCLUDE_DESC
    "",
    "Forward CRC\n",
#endif
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_MAC_CRC_FWD_FIELD_MASK },
    0,
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_MAC_CRC_FWD_FIELD_WIDTH },
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_MAC_CRC_FWD_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MAC_CRC_OWRT *****/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_MAC_CRC_OWRT_FIELD =
{
    "MAC_CRC_OWRT",
#if RU_INCLUDE_DESC
    "",
    "Overwrite CRC\n",
#endif
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_MAC_CRC_OWRT_FIELD_MASK },
    0,
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_MAC_CRC_OWRT_FIELD_WIDTH },
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_MAC_CRC_OWRT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_FIELDS[] =
{
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_MAC_CRC_FWD_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_MAC_CRC_OWRT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG *****/
const ru_reg_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_REG =
{
    "UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG",
#if RU_INCLUDE_DESC
    "UNIMAC_CFG Register",
    "Interface static configuration values\n",
#endif
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_REG_OFFSET },
    0,
    0,
    1110,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1, TYPE: Type_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MAX_PKT_SIZE *****/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1_MAX_PKT_SIZE_FIELD =
{
    "MAX_PKT_SIZE",
#if RU_INCLUDE_DESC
    "",
    "Max Packet Size - Used fro statistics\n",
#endif
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1_MAX_PKT_SIZE_FIELD_MASK },
    0,
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1_MAX_PKT_SIZE_FIELD_WIDTH },
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1_MAX_PKT_SIZE_FIELD_SHIFT },
    1536,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: RXFIFO_CONGESTION_THRESHOLD *****/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1_RXFIFO_CONGESTION_THRESHOLD_FIELD =
{
    "RXFIFO_CONGESTION_THRESHOLD",
#if RU_INCLUDE_DESC
    "",
    "RX FIFO Threshold - This is the fifo located between the UNIMAC IP and BBH. Once the threshold is reached, rx_fifo_congestion output is set.\nThis configuration is in 16-byte resolution (the number of bytes in a FIFO line).\nMax value is: 0x100.\n",
#endif
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1_RXFIFO_CONGESTION_THRESHOLD_FIELD_MASK },
    0,
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1_RXFIFO_CONGESTION_THRESHOLD_FIELD_WIDTH },
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1_RXFIFO_CONGESTION_THRESHOLD_FIELD_SHIFT },
    224,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1_FIELDS[] =
{
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1_MAX_PKT_SIZE_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1_RXFIFO_CONGESTION_THRESHOLD_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1 *****/
const ru_reg_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1_REG =
{
    "UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1",
#if RU_INCLUDE_DESC
    "UNIMAC_EXT_CFG1 Register",
    "Configure additional parameters that influence UNIMAC external logic.\n",
#endif
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1_REG_OFFSET },
    0,
    0,
    1111,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2, TYPE: Type_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: RXFIFO_PAUSE_THRESHOLD *****/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_RXFIFO_PAUSE_THRESHOLD_FIELD =
{
    "RXFIFO_PAUSE_THRESHOLD",
#if RU_INCLUDE_DESC
    "",
    "RX FIFO Threshold - This is the fifo located between the UNIMAC IP and BBH. Once the threshold is reached, pause is sent. This configuration is in 16-byte resolution (the number of bytes in a FIFO line).\nMax Value is 0x100.\n",
#endif
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_RXFIFO_PAUSE_THRESHOLD_FIELD_MASK },
    0,
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_RXFIFO_PAUSE_THRESHOLD_FIELD_WIDTH },
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_RXFIFO_PAUSE_THRESHOLD_FIELD_SHIFT },
    32,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BACKPRESSURE_ENABLE_INT *****/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_BACKPRESSURE_ENABLE_INT_FIELD =
{
    "BACKPRESSURE_ENABLE_INT",
#if RU_INCLUDE_DESC
    "",
    "Backpressure enable for internal unimac\n",
#endif
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_BACKPRESSURE_ENABLE_INT_FIELD_MASK },
    0,
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_BACKPRESSURE_ENABLE_INT_FIELD_WIDTH },
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_BACKPRESSURE_ENABLE_INT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BACKPRESSURE_ENABLE_EXT *****/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_BACKPRESSURE_ENABLE_EXT_FIELD =
{
    "BACKPRESSURE_ENABLE_EXT",
#if RU_INCLUDE_DESC
    "",
    "Backpressure enable for external switch\n",
#endif
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_BACKPRESSURE_ENABLE_EXT_FIELD_MASK },
    0,
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_BACKPRESSURE_ENABLE_EXT_FIELD_WIDTH },
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_BACKPRESSURE_ENABLE_EXT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FIFO_OVERRUN_CTL_EN *****/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_FIFO_OVERRUN_CTL_EN_FIELD =
{
    "FIFO_OVERRUN_CTL_EN",
#if RU_INCLUDE_DESC
    "",
    "Enable the mechanism for always receiving data from UNIMAC IP and dropping overrun words in the unimac_glue FIFO.\n",
#endif
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_FIFO_OVERRUN_CTL_EN_FIELD_MASK },
    0,
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_FIFO_OVERRUN_CTL_EN_FIELD_WIDTH },
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_FIFO_OVERRUN_CTL_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: REMOTE_LOOPBACK_EN *****/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_REMOTE_LOOPBACK_EN_FIELD =
{
    "REMOTE_LOOPBACK_EN",
#if RU_INCLUDE_DESC
    "",
    "When setting this bit, RX recovered clock will also input the clk25/pll_ref_clk in the UNIMAC.\n",
#endif
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_REMOTE_LOOPBACK_EN_FIELD_MASK },
    0,
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_REMOTE_LOOPBACK_EN_FIELD_WIDTH },
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_REMOTE_LOOPBACK_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_FIELDS[] =
{
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_RXFIFO_PAUSE_THRESHOLD_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_BACKPRESSURE_ENABLE_INT_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_BACKPRESSURE_ENABLE_EXT_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_FIFO_OVERRUN_CTL_EN_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_REMOTE_LOOPBACK_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2 *****/
const ru_reg_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_REG =
{
    "UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2",
#if RU_INCLUDE_DESC
    "UNIMAC_EXT_CFG2 Register",
    "Configure additional parameters that influence UNIMAC external logic.\n",
#endif
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_REG_OFFSET },
    0,
    0,
    1112,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT, TYPE: Type_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PORT_RATE *****/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_PORT_RATE_FIELD =
{
    "PORT_RATE",
#if RU_INCLUDE_DESC
    "",
    "Port data rate, to be used in LED block.                                 encoded as:\n3b000:   10Mb/s\n3b001:  100Mb/s\n3b010: 1000Mb/s\n3b011: 2500Mb/s\n3b100:   10Gb/s or higher\n3b101:    5Gb/s\n3b110: reserved\n3b111: reserved\n\n\n",
#endif
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_PORT_RATE_FIELD_MASK },
    0,
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_PORT_RATE_FIELD_WIDTH },
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_PORT_RATE_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: LPI_TX_DETECT *****/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_LPI_TX_DETECT_FIELD =
{
    "LPI_TX_DETECT",
#if RU_INCLUDE_DESC
    "",
    "Transmit LPI State - Set to 1 whenever LPI_IDLES are being Sent out on the TX Line\n",
#endif
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_LPI_TX_DETECT_FIELD_MASK },
    0,
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_LPI_TX_DETECT_FIELD_WIDTH },
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_LPI_TX_DETECT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: LPI_RX_DETECT *****/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_LPI_RX_DETECT_FIELD =
{
    "LPI_RX_DETECT",
#if RU_INCLUDE_DESC
    "",
    "Receive LPI State - Set to 1 whenever LPI_IDLES are being received on the RX Line\n",
#endif
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_LPI_RX_DETECT_FIELD_MASK },
    0,
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_LPI_RX_DETECT_FIELD_WIDTH },
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_LPI_RX_DETECT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PP_STATS *****/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_PP_STATS_FIELD =
{
    "PP_STATS",
#if RU_INCLUDE_DESC
    "",
    "The XOFF/XON status of the receiving PPP control frame\n\n",
#endif
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_PP_STATS_FIELD_MASK },
    0,
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_PP_STATS_FIELD_WIDTH },
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_PP_STATS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: PP_STATS_VALID *****/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_PP_STATS_VALID_FIELD =
{
    "PP_STATS_VALID",
#if RU_INCLUDE_DESC
    "",
    "MAC to indicate a PPP control frame is received\n\n",
#endif
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_PP_STATS_VALID_FIELD_MASK },
    0,
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_PP_STATS_VALID_FIELD_WIDTH },
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_PP_STATS_VALID_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_FIELDS[] =
{
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_PORT_RATE_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_LPI_TX_DETECT_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_LPI_RX_DETECT_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_PP_STATS_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_PP_STATS_VALID_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT *****/
const ru_reg_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_REG =
{
    "UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT",
#if RU_INCLUDE_DESC
    "UNIMAC_STAT Register",
    "This registers holds status indications of the UNIMAC\n",
#endif
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_REG_OFFSET },
    0,
    0,
    1113,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_DEBUG, TYPE: Type_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_DEBUG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DEBUG_SEL *****/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_DEBUG_DEBUG_SEL_FIELD =
{
    "DEBUG_SEL",
#if RU_INCLUDE_DESC
    "",
    "Select which debug bus to output.\n",
#endif
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_DEBUG_DEBUG_SEL_FIELD_MASK },
    0,
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_DEBUG_DEBUG_SEL_FIELD_WIDTH },
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_DEBUG_DEBUG_SEL_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_DEBUG_FIELDS[] =
{
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_DEBUG_DEBUG_SEL_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_DEBUG *****/
const ru_reg_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_DEBUG_REG =
{
    "UNIMAC_TOP_UNIMAC_MISC_UNIMAC_DEBUG",
#if RU_INCLUDE_DESC
    "UNIMAC_DEBUG Register",
    "This register holds all debug related configurations\n",
#endif
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_DEBUG_REG_OFFSET },
    0,
    0,
    1114,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_DEBUG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RST, TYPE: Type_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RST
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: UNIMAC_RST *****/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RST_UNIMAC_RST_FIELD =
{
    "UNIMAC_RST",
#if RU_INCLUDE_DESC
    "",
    "Synchronous UNIMAC reset\n",
#endif
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RST_UNIMAC_RST_FIELD_MASK },
    0,
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RST_UNIMAC_RST_FIELD_WIDTH },
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RST_UNIMAC_RST_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RST_FIELDS[] =
{
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RST_UNIMAC_RST_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RST *****/
const ru_reg_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RST_REG =
{
    "UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RST",
#if RU_INCLUDE_DESC
    "UNIMAC_RST Register",
    "This is a synchronous reset to the UNIMAC core\n",
#endif
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RST_REG_OFFSET },
    0,
    0,
    1115,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RST_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_OVERRUN_COUNTER, TYPE: Type_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_OVERRUN_COUNTER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: OVERRUN_COUNTER *****/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_OVERRUN_COUNTER_OVERRUN_COUNTER_FIELD =
{
    "OVERRUN_COUNTER",
#if RU_INCLUDE_DESC
    "",
    "This registers holds the number of 128-bit lines that were received from the UNIMAC IP, but were thrown due to the fact that the unimac_glue FIFO was full. This field stops counting when it reaches 0xff and will be cleared upon read.\n",
#endif
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_OVERRUN_COUNTER_OVERRUN_COUNTER_FIELD_MASK },
    0,
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_OVERRUN_COUNTER_OVERRUN_COUNTER_FIELD_WIDTH },
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_OVERRUN_COUNTER_OVERRUN_COUNTER_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_OVERRUN_COUNTER_FIELDS[] =
{
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_OVERRUN_COUNTER_OVERRUN_COUNTER_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_OVERRUN_COUNTER *****/
const ru_reg_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_OVERRUN_COUNTER_REG =
{
    "UNIMAC_TOP_UNIMAC_MISC_UNIMAC_OVERRUN_COUNTER",
#if RU_INCLUDE_DESC
    "UNIMAC_OVERRUN_COUNTER Register",
    "This registers holds the number of 128-bit lines that were received from the UNIMAC IP, but were thrown due to the fact that the unimac_glue FIFO was full.\n",
#endif
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_OVERRUN_COUNTER_REG_OFFSET },
    0,
    0,
    1116,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_OVERRUN_COUNTER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588, TYPE: Type_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: TSI_SIGN_EXT *****/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_TSI_SIGN_EXT_FIELD =
{
    "TSI_SIGN_EXT",
#if RU_INCLUDE_DESC
    "",
    "The MAC logic will need to evaluate this bit in order to determine how to sign extend the Timestamp (TSe).\nIf the bit is a zero, the TSe MUST be sign extended with zeros.\nIf the bit is a one, the TSe MUST be sign extended based on the Msbit of the TSe. That is, if the Msbit of the TSe is zero, the TSe is signed extended with zeros. If the Msbit of the TSe is one, the TSe is sign extended with one. (from unimac spec)\n\n",
#endif
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_TSI_SIGN_EXT_FIELD_MASK },
    0,
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_TSI_SIGN_EXT_FIELD_WIDTH },
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_TSI_SIGN_EXT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: OSTS_TIMER_DIS *****/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_OSTS_TIMER_DIS_FIELD =
{
    "OSTS_TIMER_DIS",
#if RU_INCLUDE_DESC
    "",
    "This active high signal is used to disable the OSTS time-stamping function in the MAC when asserted during the time the CPU Is initializing the local counters.(from unimac spec)\n",
#endif
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_OSTS_TIMER_DIS_FIELD_MASK },
    0,
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_OSTS_TIMER_DIS_FIELD_WIDTH },
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_OSTS_TIMER_DIS_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EGR_1588_TS_MODE *****/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_EGR_1588_TS_MODE_FIELD =
{
    "EGR_1588_TS_MODE",
#if RU_INCLUDE_DESC
    "",
    "When set to 0, enables legacy sign-extension of 32-bit timestamp, when 1, enables 48-bit time-stamping. (from unimac spec)\n",
#endif
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_EGR_1588_TS_MODE_FIELD_MASK },
    0,
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_EGR_1588_TS_MODE_FIELD_WIDTH },
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_EGR_1588_TS_MODE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_FIELDS[] =
{
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_TSI_SIGN_EXT_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_OSTS_TIMER_DIS_FIELD,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_EGR_1588_TS_MODE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588 *****/
const ru_reg_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_REG =
{
    "UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588",
#if RU_INCLUDE_DESC
    "UNIMAC_1588 Register",
    "1588 configurations\n",
#endif
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_REG_OFFSET },
    0,
    0,
    1117,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISR, TYPE: Type_UNIMAC_TOP_UNIMAC_INTS_ISR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: GEN_INT *****/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISR_GEN_INT_FIELD =
{
    "GEN_INT",
#if RU_INCLUDE_DESC
    "",
    "general interrupt\n",
#endif
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISR_GEN_INT_FIELD_MASK },
    0,
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISR_GEN_INT_FIELD_WIDTH },
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISR_GEN_INT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISR_FIELDS[] =
{
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISR_GEN_INT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISR *****/
const ru_reg_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISR_REG =
{
    "UNIMAC_TOP_UNIMAC_INTS_ISR",
#if RU_INCLUDE_DESC
    "ISR Register",
    "This register provides status bits for all the general interrupt sources.\n",
#endif
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISR_REG_OFFSET },
    0,
    0,
    1118,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_IER, TYPE: Type_UNIMAC_TOP_UNIMAC_INTS_IER
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALUE *****/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_IER_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "",
    "The bit description is the same like for the ISR.\n",
#endif
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_IER_VALUE_FIELD_MASK },
    0,
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_IER_VALUE_FIELD_WIDTH },
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_IER_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_IER_FIELDS[] =
{
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_IER_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_IER *****/
const ru_reg_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_IER_REG =
{
    "UNIMAC_TOP_UNIMAC_INTS_IER",
#if RU_INCLUDE_DESC
    "IER Register",
    "This register provides enable bits for all the general interrupt sources.\nThe bit description is the same like for the ISR.\n",
#endif
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_IER_REG_OFFSET },
    0,
    0,
    1119,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_IER_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ITR, TYPE: Type_UNIMAC_TOP_UNIMAC_INTS_ITR
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALUE *****/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ITR_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "",
    "The bit description is the same like for the ISR.\n",
#endif
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ITR_VALUE_FIELD_MASK },
    0,
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ITR_VALUE_FIELD_WIDTH },
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ITR_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ITR_FIELDS[] =
{
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ITR_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ITR *****/
const ru_reg_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ITR_REG =
{
    "UNIMAC_TOP_UNIMAC_INTS_ITR",
#if RU_INCLUDE_DESC
    "ITR Register",
    "This register enables alarm interrupt testing. Writing 1 to a bit in this register sets the corresponding bit in the ISR register. Read always returns 0.\nThe bit description is the same like for the ISR.\n",
#endif
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ITR_REG_OFFSET },
    0,
    0,
    1120,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ITR_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISM, TYPE: Type_UNIMAC_TOP_UNIMAC_INTS_ISM
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALUE *****/
const ru_field_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISM_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "",
    "The bit description is the same like for the ISR.\n",
#endif
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISM_VALUE_FIELD_MASK },
    0,
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISM_VALUE_FIELD_WIDTH },
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISM_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISM_FIELDS[] =
{
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISM_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISM *****/
const ru_reg_rec UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISM_REG =
{
    "UNIMAC_TOP_UNIMAC_INTS_ISM",
#if RU_INCLUDE_DESC
    "ISM Register",
    "This read only register provides masked status bits. Each bit corresponds to logical AND operation between the corresponding bits of the ISR and IER registers.The bit description is the same like for the ISR.\n",
#endif
    { UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISM_REG_OFFSET },
    0,
    0,
    1121,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISM_FIELDS,
#endif
};

unsigned long UNIMAC_MISC_ADDRS[] =
{
    0x828B0000,
    0x828B0400,
    0x828B0800,
    0x828B0C00,
    0x828B1000,
};

static const ru_reg_rec *UNIMAC_MISC_REGS[] =
{
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG_REG,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1_REG,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_REG,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_REG,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_DEBUG_REG,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RST_REG,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_OVERRUN_COUNTER_REG,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588_REG,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISR_REG,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_IER_REG,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ITR_REG,
    &UNIMAC_MISC_UNIMAC_TOP_UNIMAC_INTS_ISM_REG,
};

const ru_block_rec UNIMAC_MISC_BLOCK =
{
    "UNIMAC_MISC",
    UNIMAC_MISC_ADDRS,
    5,
    12,
    UNIMAC_MISC_REGS,
};
