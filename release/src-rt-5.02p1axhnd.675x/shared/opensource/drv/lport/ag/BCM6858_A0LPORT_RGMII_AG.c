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
 * Field: LPORT_RGMII_CNTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_CNTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_RGMII_CNTRL_RESERVED0_FIELD_MASK,
    0,
    LPORT_RGMII_CNTRL_RESERVED0_FIELD_WIDTH,
    LPORT_RGMII_CNTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_CNTRL_COL_CRS_MASK
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_CNTRL_COL_CRS_MASK_FIELD =
{
    "COL_CRS_MASK",
#if RU_INCLUDE_DESC
    "",
    "When this bit is set to 1'''b1, COL signal toward the MAC is 1'''b0 and CRS signal toward the MAC is 1'''b1.\n"
    "Applicable to MII/rvMII interfaces and used in case where link partner does not support COL/CRS or the link is full-duplex. "
    "Note that as per IEEE 802.3 MACs ignore COL/CRS in full-duplex mode and therefore it is not necessary required to set this bit.",
#endif
    LPORT_RGMII_CNTRL_COL_CRS_MASK_FIELD_MASK,
    0,
    LPORT_RGMII_CNTRL_COL_CRS_MASK_FIELD_WIDTH,
    LPORT_RGMII_CNTRL_COL_CRS_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_CNTRL_RX_ERR_MASK
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_CNTRL_RX_ERR_MASK_FIELD =
{
    "RX_ERR_MASK",
#if RU_INCLUDE_DESC
    "",
    "When this bit is set to 1'''b1, RX_ERR signal toward the MAC is 1'''b0 (i.e. no error). "
    "Applicable to MII/rvMII interfaces and used in case where link partner does not support RX_ERR.",
#endif
    LPORT_RGMII_CNTRL_RX_ERR_MASK_FIELD_MASK,
    0,
    LPORT_RGMII_CNTRL_RX_ERR_MASK_FIELD_WIDTH,
    LPORT_RGMII_CNTRL_RX_ERR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_CNTRL_LPI_COUNT
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_CNTRL_LPI_COUNT_FIELD =
{
    "LPI_COUNT",
#if RU_INCLUDE_DESC
    "",
    "Specifies number of cycles after which TX_CLK will be stopped (after LPI is asserted), "
    "if the clock stopping is enabled.",
#endif
    LPORT_RGMII_CNTRL_LPI_COUNT_FIELD_MASK,
    0,
    LPORT_RGMII_CNTRL_LPI_COUNT_FIELD_WIDTH,
    LPORT_RGMII_CNTRL_LPI_COUNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_CNTRL_TX_CLK_STOP_EN
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_CNTRL_TX_CLK_STOP_EN_FIELD =
{
    "TX_CLK_STOP_EN",
#if RU_INCLUDE_DESC
    "",
    "When set enables stopping TX_CLK after LPI is asserted. "
    "This bit should be set only when the connected EEE PHY supports it.",
#endif
    LPORT_RGMII_CNTRL_TX_CLK_STOP_EN_FIELD_MASK,
    0,
    LPORT_RGMII_CNTRL_TX_CLK_STOP_EN_FIELD_WIDTH,
    LPORT_RGMII_CNTRL_TX_CLK_STOP_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_CNTRL_TX_PAUSE_EN
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_CNTRL_TX_PAUSE_EN_FIELD =
{
    "TX_PAUSE_EN",
#if RU_INCLUDE_DESC
    "",
    "Tx Pause as negotiated by the attached PHY. Obtained by SW via MDIO.",
#endif
    LPORT_RGMII_CNTRL_TX_PAUSE_EN_FIELD_MASK,
    0,
    LPORT_RGMII_CNTRL_TX_PAUSE_EN_FIELD_WIDTH,
    LPORT_RGMII_CNTRL_TX_PAUSE_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_CNTRL_RX_PAUSE_EN
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_CNTRL_RX_PAUSE_EN_FIELD =
{
    "RX_PAUSE_EN",
#if RU_INCLUDE_DESC
    "",
    "Rx Pause as negotiated by the attached PHY. Obtained by SW via MDIO.",
#endif
    LPORT_RGMII_CNTRL_RX_PAUSE_EN_FIELD_MASK,
    0,
    LPORT_RGMII_CNTRL_RX_PAUSE_EN_FIELD_WIDTH,
    LPORT_RGMII_CNTRL_RX_PAUSE_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_CNTRL_RVMII_REF_SEL
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_CNTRL_RVMII_REF_SEL_FIELD =
{
    "RVMII_REF_SEL",
#if RU_INCLUDE_DESC
    "",
    "Selects clock in RvMII mode.\n"
    "0 : RvMII reference clock is 50MHz.\n"
    "1 : RvMII reference clock is 25MHz.",
#endif
    LPORT_RGMII_CNTRL_RVMII_REF_SEL_FIELD_MASK,
    0,
    LPORT_RGMII_CNTRL_RVMII_REF_SEL_FIELD_WIDTH,
    LPORT_RGMII_CNTRL_RVMII_REF_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_CNTRL_PORT_MODE
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_CNTRL_PORT_MODE_FIELD =
{
    "PORT_MODE",
#if RU_INCLUDE_DESC
    "",
    "Port Mode encoded as:\n"
    "000 : Internal EPHY (MII).\n"
    "001 : Internal GPHY (GMII/MII).\n"
    "010 : External EPHY (MII).\n"
    "011 : External GPHY (RGMII).\n"
    "100 : External RvMII.\n"
    "Not all combinations are applicable to all chips.",
#endif
    LPORT_RGMII_CNTRL_PORT_MODE_FIELD_MASK,
    0,
    LPORT_RGMII_CNTRL_PORT_MODE_FIELD_WIDTH,
    LPORT_RGMII_CNTRL_PORT_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_CNTRL_ID_MODE_DIS
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_CNTRL_ID_MODE_DIS_FIELD =
{
    "ID_MODE_DIS",
#if RU_INCLUDE_DESC
    "",
    "RGMII Internal Delay (ID) mode disable.\n"
    "When set RGMII transmit clock edges are aligned with the data.\n"
    "When cleared RGMII transmit clock edges are centered in the middle of (transmit) data valid window.",
#endif
    LPORT_RGMII_CNTRL_ID_MODE_DIS_FIELD_MASK,
    0,
    LPORT_RGMII_CNTRL_ID_MODE_DIS_FIELD_WIDTH,
    LPORT_RGMII_CNTRL_ID_MODE_DIS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_CNTRL_RGMII_MODE_EN
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_CNTRL_RGMII_MODE_EN_FIELD =
{
    "RGMII_MODE_EN",
#if RU_INCLUDE_DESC
    "",
    "When set this bit enables RGMII interface. "
    "This bit acts as a reset for RGMII block abd therefore it can be used to reset RGMII block when needed.",
#endif
    LPORT_RGMII_CNTRL_RGMII_MODE_EN_FIELD_MASK,
    0,
    LPORT_RGMII_CNTRL_RGMII_MODE_EN_FIELD_WIDTH,
    LPORT_RGMII_CNTRL_RGMII_MODE_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_IB_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_IB_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_RGMII_IB_STATUS_RESERVED0_FIELD_MASK,
    0,
    LPORT_RGMII_IB_STATUS_RESERVED0_FIELD_WIDTH,
    LPORT_RGMII_IB_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_IB_STATUS_IB_STATUS_OVRD
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_IB_STATUS_IB_STATUS_OVRD_FIELD =
{
    "IB_STATUS_OVRD",
#if RU_INCLUDE_DESC
    "",
    "When this bit is set, RGMII in-band status can be overridden by bits [3:0] of this register by SW.",
#endif
    LPORT_RGMII_IB_STATUS_IB_STATUS_OVRD_FIELD_MASK,
    0,
    LPORT_RGMII_IB_STATUS_IB_STATUS_OVRD_FIELD_WIDTH,
    LPORT_RGMII_IB_STATUS_IB_STATUS_OVRD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_IB_STATUS_LINK_DECODE
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_IB_STATUS_LINK_DECODE_FIELD =
{
    "LINK_DECODE",
#if RU_INCLUDE_DESC
    "",
    "RGMII link indication as extracted from in-band signaling.\n"
    "1 : Link Up.\n"
    "0 : Link Down.",
#endif
    LPORT_RGMII_IB_STATUS_LINK_DECODE_FIELD_MASK,
    0,
    LPORT_RGMII_IB_STATUS_LINK_DECODE_FIELD_WIDTH,
    LPORT_RGMII_IB_STATUS_LINK_DECODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_IB_STATUS_DUPLEX_DECODE
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_IB_STATUS_DUPLEX_DECODE_FIELD =
{
    "DUPLEX_DECODE",
#if RU_INCLUDE_DESC
    "",
    "RGMII duplex mode as extracted from in-band signaling.\n"
    "1 : Full Duplex.\n"
    "0 : Half Duplex.",
#endif
    LPORT_RGMII_IB_STATUS_DUPLEX_DECODE_FIELD_MASK,
    0,
    LPORT_RGMII_IB_STATUS_DUPLEX_DECODE_FIELD_WIDTH,
    LPORT_RGMII_IB_STATUS_DUPLEX_DECODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_IB_STATUS_SPEED_DECODE
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_IB_STATUS_SPEED_DECODE_FIELD =
{
    "SPEED_DECODE",
#if RU_INCLUDE_DESC
    "",
    "RGMII operating speed as extracted from in-band signaling.\n"
    "00 : 10Mbp/s.\n"
    "01 : 100Mbp/s.\n"
    "10 : 1000Mbp/s.\n"
    "11 : reserved.",
#endif
    LPORT_RGMII_IB_STATUS_SPEED_DECODE_FIELD_MASK,
    0,
    LPORT_RGMII_IB_STATUS_SPEED_DECODE_FIELD_WIDTH,
    LPORT_RGMII_IB_STATUS_SPEED_DECODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_RESERVED0_FIELD_MASK,
    0,
    LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_RESERVED0_FIELD_WIDTH,
    LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_RESET
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_RESET_FIELD =
{
    "RESET",
#if RU_INCLUDE_DESC
    "",
    "When set it resets 2ns delay line.",
#endif
    LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_RESET_FIELD_MASK,
    0,
    LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_RESET_FIELD_WIDTH,
    LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_RESET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_DLY_OVERRIDE
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_DLY_OVERRIDE_FIELD =
{
    "DLY_OVERRIDE",
#if RU_INCLUDE_DESC
    "",
    "Overrides HW selected delay.",
#endif
    LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_DLY_OVERRIDE_FIELD_MASK,
    0,
    LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_DLY_OVERRIDE_FIELD_WIDTH,
    LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_DLY_OVERRIDE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_DLY_SEL
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_DLY_SEL_FIELD =
{
    "DLY_SEL",
#if RU_INCLUDE_DESC
    "",
    "When set delay line delay is ~2ns and when cleared delay line is > 2.2ns. Valid only when DLY_OVERRIDE bit is set.",
#endif
    LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_DLY_SEL_FIELD_MASK,
    0,
    LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_DLY_SEL_FIELD_WIDTH,
    LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_DLY_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_BYPASS
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_BYPASS_FIELD =
{
    "BYPASS",
#if RU_INCLUDE_DESC
    "",
    "When set it puts 2ns delay line in bypass mode (default). This bit should be cleared only in non-ID mode.",
#endif
    LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_BYPASS_FIELD_MASK,
    0,
    LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_BYPASS_FIELD_WIDTH,
    LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_BYPASS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_IDDQ
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_IDDQ_FIELD =
{
    "IDDQ",
#if RU_INCLUDE_DESC
    "",
    "When set puts 2ns delay line in IDDQ mode."
    "Requires HW reset (see bit 8 of this register) to bring 2ns delay line from power down.",
#endif
    LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_IDDQ_FIELD_MASK,
    0,
    LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_IDDQ_FIELD_WIDTH,
    LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_IDDQ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_DRNG
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_DRNG_FIELD =
{
    "DRNG",
#if RU_INCLUDE_DESC
    "",
    "VCDL control. Contact BRCM for more information",
#endif
    LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_DRNG_FIELD_MASK,
    0,
    LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_DRNG_FIELD_WIDTH,
    LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_DRNG_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_CTRI
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_CTRI_FIELD =
{
    "CTRI",
#if RU_INCLUDE_DESC
    "",
    "Charge pump current control. Contact BRCM for more information",
#endif
    LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_CTRI_FIELD_MASK,
    0,
    LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_CTRI_FIELD_WIDTH,
    LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_CTRI_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_ATE_RX_CNTRL_EXP_DATA_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_ATE_RX_CNTRL_EXP_DATA_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_RGMII_ATE_RX_CNTRL_EXP_DATA_RESERVED0_FIELD_MASK,
    0,
    LPORT_RGMII_ATE_RX_CNTRL_EXP_DATA_RESERVED0_FIELD_WIDTH,
    LPORT_RGMII_ATE_RX_CNTRL_EXP_DATA_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_ATE_RX_CNTRL_EXP_DATA_ATE_EN
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_ATE_RX_CNTRL_EXP_DATA_ATE_EN_FIELD =
{
    "ATE_EN",
#if RU_INCLUDE_DESC
    "",
    "When set enables ATE testing",
#endif
    LPORT_RGMII_ATE_RX_CNTRL_EXP_DATA_ATE_EN_FIELD_MASK,
    0,
    LPORT_RGMII_ATE_RX_CNTRL_EXP_DATA_ATE_EN_FIELD_WIDTH,
    LPORT_RGMII_ATE_RX_CNTRL_EXP_DATA_ATE_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_ATE_RX_CNTRL_EXP_DATA_PKT_COUNT_RST
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_ATE_RX_CNTRL_EXP_DATA_PKT_COUNT_RST_FIELD =
{
    "PKT_COUNT_RST",
#if RU_INCLUDE_DESC
    "",
    "When set resets received packets counter. Used only in packet generation mode (PKT_GEN_MODE bit is set).",
#endif
    LPORT_RGMII_ATE_RX_CNTRL_EXP_DATA_PKT_COUNT_RST_FIELD_MASK,
    0,
    LPORT_RGMII_ATE_RX_CNTRL_EXP_DATA_PKT_COUNT_RST_FIELD_WIDTH,
    LPORT_RGMII_ATE_RX_CNTRL_EXP_DATA_PKT_COUNT_RST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_ATE_RX_CNTRL_EXP_DATA_GOOD_COUNT
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_ATE_RX_CNTRL_EXP_DATA_GOOD_COUNT_FIELD =
{
    "GOOD_COUNT",
#if RU_INCLUDE_DESC
    "",
    "Count that specifies how many consecutive {EXPECTED_DATA_0, EXPECTED_DATA_1, EXPECTED_DATA_2, EXPECTED_DATA_3 } patterns should be received before RX_OK signal is asserted.\n"
    " In packet generation mode it specifies number of expected packets.",
#endif
    LPORT_RGMII_ATE_RX_CNTRL_EXP_DATA_GOOD_COUNT_FIELD_MASK,
    0,
    LPORT_RGMII_ATE_RX_CNTRL_EXP_DATA_GOOD_COUNT_FIELD_WIDTH,
    LPORT_RGMII_ATE_RX_CNTRL_EXP_DATA_GOOD_COUNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_ATE_RX_CNTRL_EXP_DATA_EXPECTED_DATA_1
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_ATE_RX_CNTRL_EXP_DATA_EXPECTED_DATA_1_FIELD =
{
    "EXPECTED_DATA_1",
#if RU_INCLUDE_DESC
    "",
    "Data expected on the odd rising edge of the RXC clock on the RGMII Rx interface. Bits[12:9] of this register are used only in MII modes and they represent RXD[3:0]. Bit 17 corresponds RX_ER.\n"
    "Not used in Packet Generation mode.",
#endif
    LPORT_RGMII_ATE_RX_CNTRL_EXP_DATA_EXPECTED_DATA_1_FIELD_MASK,
    0,
    LPORT_RGMII_ATE_RX_CNTRL_EXP_DATA_EXPECTED_DATA_1_FIELD_WIDTH,
    LPORT_RGMII_ATE_RX_CNTRL_EXP_DATA_EXPECTED_DATA_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_ATE_RX_CNTRL_EXP_DATA_EXPECTED_DATA_0
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_ATE_RX_CNTRL_EXP_DATA_EXPECTED_DATA_0_FIELD =
{
    "EXPECTED_DATA_0",
#if RU_INCLUDE_DESC
    "",
    "Data expected on the even rising edge of the RXC clock on the RGMII Rx interface. Bits[3:0] of this register are used only in MII modes and they represent RXD[3:0]. Bit 8 corresponds RX_ER.\n"
    "Not used in Packet Generation mode.",
#endif
    LPORT_RGMII_ATE_RX_CNTRL_EXP_DATA_EXPECTED_DATA_0_FIELD_MASK,
    0,
    LPORT_RGMII_ATE_RX_CNTRL_EXP_DATA_EXPECTED_DATA_0_FIELD_WIDTH,
    LPORT_RGMII_ATE_RX_CNTRL_EXP_DATA_EXPECTED_DATA_0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_ATE_RX_EXP_DATA_1_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_ATE_RX_EXP_DATA_1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_RGMII_ATE_RX_EXP_DATA_1_RESERVED0_FIELD_MASK,
    0,
    LPORT_RGMII_ATE_RX_EXP_DATA_1_RESERVED0_FIELD_WIDTH,
    LPORT_RGMII_ATE_RX_EXP_DATA_1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_ATE_RX_EXP_DATA_1_EXPECTED_DATA_3
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_ATE_RX_EXP_DATA_1_EXPECTED_DATA_3_FIELD =
{
    "EXPECTED_DATA_3",
#if RU_INCLUDE_DESC
    "",
    "Data expected on the odd rising edge of the RXC clock on the RGMII Rx interface. Bits[12:9] of this register are used only in MII modes and they represent RXD[3:0]. Bit 17 corresponds RX_ER.\n"
    "Not used in Packet Generation mode.",
#endif
    LPORT_RGMII_ATE_RX_EXP_DATA_1_EXPECTED_DATA_3_FIELD_MASK,
    0,
    LPORT_RGMII_ATE_RX_EXP_DATA_1_EXPECTED_DATA_3_FIELD_WIDTH,
    LPORT_RGMII_ATE_RX_EXP_DATA_1_EXPECTED_DATA_3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_ATE_RX_EXP_DATA_1_EXPECTED_DATA_2
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_ATE_RX_EXP_DATA_1_EXPECTED_DATA_2_FIELD =
{
    "EXPECTED_DATA_2",
#if RU_INCLUDE_DESC
    "",
    "Data expected on the even rising edge of the RXC clock on the RGMII Rx interface. Bits[3:0] of this register are used only in MII modes and they represent RXD[3:0]. Bit 8 corresponds RX_ER.\n"
    "Not used in Packet Generation mode.",
#endif
    LPORT_RGMII_ATE_RX_EXP_DATA_1_EXPECTED_DATA_2_FIELD_MASK,
    0,
    LPORT_RGMII_ATE_RX_EXP_DATA_1_EXPECTED_DATA_2_FIELD_WIDTH,
    LPORT_RGMII_ATE_RX_EXP_DATA_1_EXPECTED_DATA_2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_ATE_RX_STATUS_0_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_ATE_RX_STATUS_0_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_RGMII_ATE_RX_STATUS_0_RESERVED0_FIELD_MASK,
    0,
    LPORT_RGMII_ATE_RX_STATUS_0_RESERVED0_FIELD_WIDTH,
    LPORT_RGMII_ATE_RX_STATUS_0_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_ATE_RX_STATUS_0_RX_OK
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_ATE_RX_STATUS_0_RX_OK_FIELD =
{
    "RX_OK",
#if RU_INCLUDE_DESC
    "",
    "Test Status. This bit is cleared by HW on the rising edge of RX_CTL and asserted if GOOD_COUNT consective expected patterns are detected.\n"
    "In packet generation mode this bit is cleared when PKT_COUNT_RST bit is set and set when received packet count = GOOD_COUNT. ",
#endif
    LPORT_RGMII_ATE_RX_STATUS_0_RX_OK_FIELD_MASK,
    0,
    LPORT_RGMII_ATE_RX_STATUS_0_RX_OK_FIELD_WIDTH,
    LPORT_RGMII_ATE_RX_STATUS_0_RX_OK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_ATE_RX_STATUS_0_RECEIVED_DATA_1
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_ATE_RX_STATUS_0_RECEIVED_DATA_1_FIELD =
{
    "RECEIVED_DATA_1",
#if RU_INCLUDE_DESC
    "",
    "Data received on the odd rising edge of the RXC clock on the RGMII Rx interface. In MII modes, only Bits[12:9] of this register are used only for RXD[3:0]. Bit[17]: RX_ER\n"
    "In Packet Generation mode bits [7:0] are 2nd received byte after SOF.",
#endif
    LPORT_RGMII_ATE_RX_STATUS_0_RECEIVED_DATA_1_FIELD_MASK,
    0,
    LPORT_RGMII_ATE_RX_STATUS_0_RECEIVED_DATA_1_FIELD_WIDTH,
    LPORT_RGMII_ATE_RX_STATUS_0_RECEIVED_DATA_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_ATE_RX_STATUS_0_RECEIVED_DATA_0
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_ATE_RX_STATUS_0_RECEIVED_DATA_0_FIELD =
{
    "RECEIVED_DATA_0",
#if RU_INCLUDE_DESC
    "",
    "Data received on the even rising edge of the RXC clock on the RGMII Rx interface. In MII modes, only Bits[3:0] of this register are used only for RXD[3:0]. Bit[8]: RX_ER\n"
    "In Packet Generation mode bits [7:0] are 1st received byte after SOF.",
#endif
    LPORT_RGMII_ATE_RX_STATUS_0_RECEIVED_DATA_0_FIELD_MASK,
    0,
    LPORT_RGMII_ATE_RX_STATUS_0_RECEIVED_DATA_0_FIELD_WIDTH,
    LPORT_RGMII_ATE_RX_STATUS_0_RECEIVED_DATA_0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_ATE_RX_STATUS_1_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_ATE_RX_STATUS_1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_RGMII_ATE_RX_STATUS_1_RESERVED0_FIELD_MASK,
    0,
    LPORT_RGMII_ATE_RX_STATUS_1_RESERVED0_FIELD_WIDTH,
    LPORT_RGMII_ATE_RX_STATUS_1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_ATE_RX_STATUS_1_RECEIVED_DATA_3
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_ATE_RX_STATUS_1_RECEIVED_DATA_3_FIELD =
{
    "RECEIVED_DATA_3",
#if RU_INCLUDE_DESC
    "",
    "Data received on the odd rising edge of the RXC clock on the RGMII Rx interface. In MII modes, only Bits[12:9] of this register are used only for RXD[3:0]. Bit[17]: RX_ER\n"
    "In Packet Generation mode bits [7:0] are 4th received byte after SOF.",
#endif
    LPORT_RGMII_ATE_RX_STATUS_1_RECEIVED_DATA_3_FIELD_MASK,
    0,
    LPORT_RGMII_ATE_RX_STATUS_1_RECEIVED_DATA_3_FIELD_WIDTH,
    LPORT_RGMII_ATE_RX_STATUS_1_RECEIVED_DATA_3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_ATE_RX_STATUS_1_RECEIVED_DATA_2
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_ATE_RX_STATUS_1_RECEIVED_DATA_2_FIELD =
{
    "RECEIVED_DATA_2",
#if RU_INCLUDE_DESC
    "",
    "Data received on the even rising edge of the RXC clock on the RGMII Rx interface. In MII modes, only Bits[3:0] of this register are used only for RXD[3:0]. Bit[8]: RX_ER\n"
    "In Packet Generation mode bits [7:0] are 3rd received byte after SOF.",
#endif
    LPORT_RGMII_ATE_RX_STATUS_1_RECEIVED_DATA_2_FIELD_MASK,
    0,
    LPORT_RGMII_ATE_RX_STATUS_1_RECEIVED_DATA_2_FIELD_WIDTH,
    LPORT_RGMII_ATE_RX_STATUS_1_RECEIVED_DATA_2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_ATE_TX_CNTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_ATE_TX_CNTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_RGMII_ATE_TX_CNTRL_RESERVED0_FIELD_MASK,
    0,
    LPORT_RGMII_ATE_TX_CNTRL_RESERVED0_FIELD_WIDTH,
    LPORT_RGMII_ATE_TX_CNTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_ATE_TX_CNTRL_PKT_IPG
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_ATE_TX_CNTRL_PKT_IPG_FIELD =
{
    "PKT_IPG",
#if RU_INCLUDE_DESC
    "",
    "Inter-packet gap in packet generation mode.",
#endif
    LPORT_RGMII_ATE_TX_CNTRL_PKT_IPG_FIELD_MASK,
    0,
    LPORT_RGMII_ATE_TX_CNTRL_PKT_IPG_FIELD_WIDTH,
    LPORT_RGMII_ATE_TX_CNTRL_PKT_IPG_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_ATE_TX_CNTRL_PAYLOAD_LENGTH
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_ATE_TX_CNTRL_PAYLOAD_LENGTH_FIELD =
{
    "PAYLOAD_LENGTH",
#if RU_INCLUDE_DESC
    "",
    "Generated packet payload in bytes. Must be between 46B and 1500B.",
#endif
    LPORT_RGMII_ATE_TX_CNTRL_PAYLOAD_LENGTH_FIELD_MASK,
    0,
    LPORT_RGMII_ATE_TX_CNTRL_PAYLOAD_LENGTH_FIELD_WIDTH,
    LPORT_RGMII_ATE_TX_CNTRL_PAYLOAD_LENGTH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_ATE_TX_CNTRL_PKT_CNT
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_ATE_TX_CNTRL_PKT_CNT_FIELD =
{
    "PKT_CNT",
#if RU_INCLUDE_DESC
    "",
    "Number of packets generated when START_STOP bit is set.  When program to 0 it means infinite number of packets will be transmit (i.e. until START_STOP is cleared). ",
#endif
    LPORT_RGMII_ATE_TX_CNTRL_PKT_CNT_FIELD_MASK,
    0,
    LPORT_RGMII_ATE_TX_CNTRL_PKT_CNT_FIELD_WIDTH,
    LPORT_RGMII_ATE_TX_CNTRL_PKT_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_ATE_TX_CNTRL_PKT_GEN_EN
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_ATE_TX_CNTRL_PKT_GEN_EN_FIELD =
{
    "PKT_GEN_EN",
#if RU_INCLUDE_DESC
    "",
    "When this bit is set ATE test logic operates in the packet generation mode.",
#endif
    LPORT_RGMII_ATE_TX_CNTRL_PKT_GEN_EN_FIELD_MASK,
    0,
    LPORT_RGMII_ATE_TX_CNTRL_PKT_GEN_EN_FIELD_WIDTH,
    LPORT_RGMII_ATE_TX_CNTRL_PKT_GEN_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_ATE_TX_CNTRL_START_STOP
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_ATE_TX_CNTRL_START_STOP_FIELD =
{
    "START_STOP",
#if RU_INCLUDE_DESC
    "",
    "start_stop. When set transmit state matchin starts outputing programmed pattern over RGMII TX interface. When cleared transmit state machine stops outputting data.",
#endif
    LPORT_RGMII_ATE_TX_CNTRL_START_STOP_FIELD_MASK,
    0,
    LPORT_RGMII_ATE_TX_CNTRL_START_STOP_FIELD_WIDTH,
    LPORT_RGMII_ATE_TX_CNTRL_START_STOP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_ATE_TX_CNTRL_START_STOP_OVRD
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_ATE_TX_CNTRL_START_STOP_OVRD_FIELD =
{
    "START_STOP_OVRD",
#if RU_INCLUDE_DESC
    "",
    "START_STOP override. When this bit is set, transmit state machine will be controlled by START_STOP bit of this register instead of the chip pin.",
#endif
    LPORT_RGMII_ATE_TX_CNTRL_START_STOP_OVRD_FIELD_MASK,
    0,
    LPORT_RGMII_ATE_TX_CNTRL_START_STOP_OVRD_FIELD_WIDTH,
    LPORT_RGMII_ATE_TX_CNTRL_START_STOP_OVRD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_ATE_TX_DATA_0_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_ATE_TX_DATA_0_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_RGMII_ATE_TX_DATA_0_RESERVED0_FIELD_MASK,
    0,
    LPORT_RGMII_ATE_TX_DATA_0_RESERVED0_FIELD_WIDTH,
    LPORT_RGMII_ATE_TX_DATA_0_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_ATE_TX_DATA_0_TX_DATA_1
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_ATE_TX_DATA_0_TX_DATA_1_FIELD =
{
    "TX_DATA_1",
#if RU_INCLUDE_DESC
    "",
    "Data transmitted on the odd rising edge of the TXC clock on the RGMII Tx interface. In case of MII, only bit[12:9] are used to transmit TXD[3:0]. Bit 17: TX_ER\n"
    "In Packet Generation mode bits [7:0] are 2nd byte of MAC DA.",
#endif
    LPORT_RGMII_ATE_TX_DATA_0_TX_DATA_1_FIELD_MASK,
    0,
    LPORT_RGMII_ATE_TX_DATA_0_TX_DATA_1_FIELD_WIDTH,
    LPORT_RGMII_ATE_TX_DATA_0_TX_DATA_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_ATE_TX_DATA_0_TX_DATA_0
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_ATE_TX_DATA_0_TX_DATA_0_FIELD =
{
    "TX_DATA_0",
#if RU_INCLUDE_DESC
    "",
    "Data transmitted on the even rising edge of the TXC clock on the RGMII Tx interface. In case of MII, only bit[3:0] are used to transmit TXD[3:0]. Bit 8: TX_ER\n"
    "In Packet Generation mode bits [7:0] are 1st byte of MAC DA.",
#endif
    LPORT_RGMII_ATE_TX_DATA_0_TX_DATA_0_FIELD_MASK,
    0,
    LPORT_RGMII_ATE_TX_DATA_0_TX_DATA_0_FIELD_WIDTH,
    LPORT_RGMII_ATE_TX_DATA_0_TX_DATA_0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_ATE_TX_DATA_1_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_ATE_TX_DATA_1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_RGMII_ATE_TX_DATA_1_RESERVED0_FIELD_MASK,
    0,
    LPORT_RGMII_ATE_TX_DATA_1_RESERVED0_FIELD_WIDTH,
    LPORT_RGMII_ATE_TX_DATA_1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_ATE_TX_DATA_1_TX_DATA_3
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_ATE_TX_DATA_1_TX_DATA_3_FIELD =
{
    "TX_DATA_3",
#if RU_INCLUDE_DESC
    "",
    "Data transmitted on the odd rising edge of the TXC clock on the RGMII Tx interface. In case of MII, only bit[12:9] are used to transmit TXD[3:0]. Bit 17: TX_ER\n"
    "In Packet Generation mode bits [7:0] are 4th byte of MAC DA.",
#endif
    LPORT_RGMII_ATE_TX_DATA_1_TX_DATA_3_FIELD_MASK,
    0,
    LPORT_RGMII_ATE_TX_DATA_1_TX_DATA_3_FIELD_WIDTH,
    LPORT_RGMII_ATE_TX_DATA_1_TX_DATA_3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_ATE_TX_DATA_1_TX_DATA_2
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_ATE_TX_DATA_1_TX_DATA_2_FIELD =
{
    "TX_DATA_2",
#if RU_INCLUDE_DESC
    "",
    "Data transmitted on the even rising edge of the TXC clock on the RGMII Tx interface. In case of MII, only bit[3:0] are used to transmit TXD[3:0]. Bit 8: TX_ER\n"
    "In Packet Generation mode bits [7:0] are 3rd byte of MAC DA.",
#endif
    LPORT_RGMII_ATE_TX_DATA_1_TX_DATA_2_FIELD_MASK,
    0,
    LPORT_RGMII_ATE_TX_DATA_1_TX_DATA_2_FIELD_WIDTH,
    LPORT_RGMII_ATE_TX_DATA_1_TX_DATA_2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_ATE_TX_DATA_2_ETHER_TYPE
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_ATE_TX_DATA_2_ETHER_TYPE_FIELD =
{
    "ETHER_TYPE",
#if RU_INCLUDE_DESC
    "",
    "Generated packet Ethertype",
#endif
    LPORT_RGMII_ATE_TX_DATA_2_ETHER_TYPE_FIELD_MASK,
    0,
    LPORT_RGMII_ATE_TX_DATA_2_ETHER_TYPE_FIELD_WIDTH,
    LPORT_RGMII_ATE_TX_DATA_2_ETHER_TYPE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_ATE_TX_DATA_2_TX_DATA_5
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_ATE_TX_DATA_2_TX_DATA_5_FIELD =
{
    "TX_DATA_5",
#if RU_INCLUDE_DESC
    "",
    "In Packet Generation mode bits [7:0] are 6th byte of MAC DA",
#endif
    LPORT_RGMII_ATE_TX_DATA_2_TX_DATA_5_FIELD_MASK,
    0,
    LPORT_RGMII_ATE_TX_DATA_2_TX_DATA_5_FIELD_WIDTH,
    LPORT_RGMII_ATE_TX_DATA_2_TX_DATA_5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_ATE_TX_DATA_2_TX_DATA_4
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_ATE_TX_DATA_2_TX_DATA_4_FIELD =
{
    "TX_DATA_4",
#if RU_INCLUDE_DESC
    "",
    "In Packet Generation mode bits [7:0] are 5th byte of MAC DA.",
#endif
    LPORT_RGMII_ATE_TX_DATA_2_TX_DATA_4_FIELD_MASK,
    0,
    LPORT_RGMII_ATE_TX_DATA_2_TX_DATA_4_FIELD_WIDTH,
    LPORT_RGMII_ATE_TX_DATA_2_TX_DATA_4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_TX_DELAY_CNTRL_0_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_TX_DELAY_CNTRL_0_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_RGMII_TX_DELAY_CNTRL_0_RESERVED0_FIELD_MASK,
    0,
    LPORT_RGMII_TX_DELAY_CNTRL_0_RESERVED0_FIELD_WIDTH,
    LPORT_RGMII_TX_DELAY_CNTRL_0_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_TX_DELAY_CNTRL_0_TXD3_DEL_OVRD_EN
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_TX_DELAY_CNTRL_0_TXD3_DEL_OVRD_EN_FIELD =
{
    "TXD3_DEL_OVRD_EN",
#if RU_INCLUDE_DESC
    "",
    "txd3 CKTAP delay override enable. When set enables CKTAP delay to be controlled from this register.",
#endif
    LPORT_RGMII_TX_DELAY_CNTRL_0_TXD3_DEL_OVRD_EN_FIELD_MASK,
    0,
    LPORT_RGMII_TX_DELAY_CNTRL_0_TXD3_DEL_OVRD_EN_FIELD_WIDTH,
    LPORT_RGMII_TX_DELAY_CNTRL_0_TXD3_DEL_OVRD_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_TX_DELAY_CNTRL_0_TXD3_DEL_SEL
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_TX_DELAY_CNTRL_0_TXD3_DEL_SEL_FIELD =
{
    "TXD3_DEL_SEL",
#if RU_INCLUDE_DESC
    "",
    "txd3 CKTAP delay control. Refer to the CKTAP datasheet for programming.",
#endif
    LPORT_RGMII_TX_DELAY_CNTRL_0_TXD3_DEL_SEL_FIELD_MASK,
    0,
    LPORT_RGMII_TX_DELAY_CNTRL_0_TXD3_DEL_SEL_FIELD_WIDTH,
    LPORT_RGMII_TX_DELAY_CNTRL_0_TXD3_DEL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_TX_DELAY_CNTRL_0_TXD2_DEL_OVRD_EN
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_TX_DELAY_CNTRL_0_TXD2_DEL_OVRD_EN_FIELD =
{
    "TXD2_DEL_OVRD_EN",
#if RU_INCLUDE_DESC
    "",
    "txd2 CKTAP delay override enable. When set enables CKTAP delay to be controlled from this register.",
#endif
    LPORT_RGMII_TX_DELAY_CNTRL_0_TXD2_DEL_OVRD_EN_FIELD_MASK,
    0,
    LPORT_RGMII_TX_DELAY_CNTRL_0_TXD2_DEL_OVRD_EN_FIELD_WIDTH,
    LPORT_RGMII_TX_DELAY_CNTRL_0_TXD2_DEL_OVRD_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_TX_DELAY_CNTRL_0_TXD2_DEL_SEL
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_TX_DELAY_CNTRL_0_TXD2_DEL_SEL_FIELD =
{
    "TXD2_DEL_SEL",
#if RU_INCLUDE_DESC
    "",
    "txd2 CKTAP delay control. Refer to the CKTAP datasheet for programming.",
#endif
    LPORT_RGMII_TX_DELAY_CNTRL_0_TXD2_DEL_SEL_FIELD_MASK,
    0,
    LPORT_RGMII_TX_DELAY_CNTRL_0_TXD2_DEL_SEL_FIELD_WIDTH,
    LPORT_RGMII_TX_DELAY_CNTRL_0_TXD2_DEL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_TX_DELAY_CNTRL_0_TXD1_DEL_OVRD_EN
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_TX_DELAY_CNTRL_0_TXD1_DEL_OVRD_EN_FIELD =
{
    "TXD1_DEL_OVRD_EN",
#if RU_INCLUDE_DESC
    "",
    "txd1 CKTAP delay override enable. When set enables CKTAP delay to be controlled from this register.",
#endif
    LPORT_RGMII_TX_DELAY_CNTRL_0_TXD1_DEL_OVRD_EN_FIELD_MASK,
    0,
    LPORT_RGMII_TX_DELAY_CNTRL_0_TXD1_DEL_OVRD_EN_FIELD_WIDTH,
    LPORT_RGMII_TX_DELAY_CNTRL_0_TXD1_DEL_OVRD_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_TX_DELAY_CNTRL_0_TXD1_DEL_SEL
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_TX_DELAY_CNTRL_0_TXD1_DEL_SEL_FIELD =
{
    "TXD1_DEL_SEL",
#if RU_INCLUDE_DESC
    "",
    "txd1 CKTAP delay control. Refer to the CKTAP datasheet for programming.",
#endif
    LPORT_RGMII_TX_DELAY_CNTRL_0_TXD1_DEL_SEL_FIELD_MASK,
    0,
    LPORT_RGMII_TX_DELAY_CNTRL_0_TXD1_DEL_SEL_FIELD_WIDTH,
    LPORT_RGMII_TX_DELAY_CNTRL_0_TXD1_DEL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_TX_DELAY_CNTRL_0_TXD0_DEL_OVRD_EN
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_TX_DELAY_CNTRL_0_TXD0_DEL_OVRD_EN_FIELD =
{
    "TXD0_DEL_OVRD_EN",
#if RU_INCLUDE_DESC
    "",
    "txd0 CKTAP delay override enable. When set enables CKTAP delay to be controlled from this register.",
#endif
    LPORT_RGMII_TX_DELAY_CNTRL_0_TXD0_DEL_OVRD_EN_FIELD_MASK,
    0,
    LPORT_RGMII_TX_DELAY_CNTRL_0_TXD0_DEL_OVRD_EN_FIELD_WIDTH,
    LPORT_RGMII_TX_DELAY_CNTRL_0_TXD0_DEL_OVRD_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_TX_DELAY_CNTRL_0_TXD0_DEL_SEL
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_TX_DELAY_CNTRL_0_TXD0_DEL_SEL_FIELD =
{
    "TXD0_DEL_SEL",
#if RU_INCLUDE_DESC
    "",
    "txd0 CKTAP delay control. Refer to the CKTAP datasheet for programming.",
#endif
    LPORT_RGMII_TX_DELAY_CNTRL_0_TXD0_DEL_SEL_FIELD_MASK,
    0,
    LPORT_RGMII_TX_DELAY_CNTRL_0_TXD0_DEL_SEL_FIELD_WIDTH,
    LPORT_RGMII_TX_DELAY_CNTRL_0_TXD0_DEL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_TX_DELAY_CNTRL_1_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_TX_DELAY_CNTRL_1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_RGMII_TX_DELAY_CNTRL_1_RESERVED0_FIELD_MASK,
    0,
    LPORT_RGMII_TX_DELAY_CNTRL_1_RESERVED0_FIELD_WIDTH,
    LPORT_RGMII_TX_DELAY_CNTRL_1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_TX_DELAY_CNTRL_1_TXCLK_ID_DEL_OVRD_EN
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_TX_DELAY_CNTRL_1_TXCLK_ID_DEL_OVRD_EN_FIELD =
{
    "TXCLK_ID_DEL_OVRD_EN",
#if RU_INCLUDE_DESC
    "",
    "txclk ID mode CKTAP delay override enable. When set enables CKTAP delay to be controlled from this register.",
#endif
    LPORT_RGMII_TX_DELAY_CNTRL_1_TXCLK_ID_DEL_OVRD_EN_FIELD_MASK,
    0,
    LPORT_RGMII_TX_DELAY_CNTRL_1_TXCLK_ID_DEL_OVRD_EN_FIELD_WIDTH,
    LPORT_RGMII_TX_DELAY_CNTRL_1_TXCLK_ID_DEL_OVRD_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_TX_DELAY_CNTRL_1_TXCLK_ID_DEL_SEL
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_TX_DELAY_CNTRL_1_TXCLK_ID_DEL_SEL_FIELD =
{
    "TXCLK_ID_DEL_SEL",
#if RU_INCLUDE_DESC
    "",
    "txclk ID mode CKTAP delay control. Refer to the CKTAP datasheet for programming.",
#endif
    LPORT_RGMII_TX_DELAY_CNTRL_1_TXCLK_ID_DEL_SEL_FIELD_MASK,
    0,
    LPORT_RGMII_TX_DELAY_CNTRL_1_TXCLK_ID_DEL_SEL_FIELD_WIDTH,
    LPORT_RGMII_TX_DELAY_CNTRL_1_TXCLK_ID_DEL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_TX_DELAY_CNTRL_1_TXCLK_DEL_OVRD_EN
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_TX_DELAY_CNTRL_1_TXCLK_DEL_OVRD_EN_FIELD =
{
    "TXCLK_DEL_OVRD_EN",
#if RU_INCLUDE_DESC
    "",
    "txclk NON_ID mode CKTAP delay override enable. When set enables CKTAP delay to be controlled from this register.",
#endif
    LPORT_RGMII_TX_DELAY_CNTRL_1_TXCLK_DEL_OVRD_EN_FIELD_MASK,
    0,
    LPORT_RGMII_TX_DELAY_CNTRL_1_TXCLK_DEL_OVRD_EN_FIELD_WIDTH,
    LPORT_RGMII_TX_DELAY_CNTRL_1_TXCLK_DEL_OVRD_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_TX_DELAY_CNTRL_1_TXCLK_DEL_SEL
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_TX_DELAY_CNTRL_1_TXCLK_DEL_SEL_FIELD =
{
    "TXCLK_DEL_SEL",
#if RU_INCLUDE_DESC
    "",
    "txclk NON-ID mode CKTAP delay control. Refer to the CKTAP datasheet for programming.",
#endif
    LPORT_RGMII_TX_DELAY_CNTRL_1_TXCLK_DEL_SEL_FIELD_MASK,
    0,
    LPORT_RGMII_TX_DELAY_CNTRL_1_TXCLK_DEL_SEL_FIELD_WIDTH,
    LPORT_RGMII_TX_DELAY_CNTRL_1_TXCLK_DEL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_TX_DELAY_CNTRL_1_TXCTL_DEL_OVRD_EN
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_TX_DELAY_CNTRL_1_TXCTL_DEL_OVRD_EN_FIELD =
{
    "TXCTL_DEL_OVRD_EN",
#if RU_INCLUDE_DESC
    "",
    "txctl CKTAP delay override enable. When set enables CKTAP delay to be controlled from this register.",
#endif
    LPORT_RGMII_TX_DELAY_CNTRL_1_TXCTL_DEL_OVRD_EN_FIELD_MASK,
    0,
    LPORT_RGMII_TX_DELAY_CNTRL_1_TXCTL_DEL_OVRD_EN_FIELD_WIDTH,
    LPORT_RGMII_TX_DELAY_CNTRL_1_TXCTL_DEL_OVRD_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_TX_DELAY_CNTRL_1_TXCTL_DEL_SEL
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_TX_DELAY_CNTRL_1_TXCTL_DEL_SEL_FIELD =
{
    "TXCTL_DEL_SEL",
#if RU_INCLUDE_DESC
    "",
    "txctl CKTAP delay control. Refer to the CKTAP datasheet for programming.",
#endif
    LPORT_RGMII_TX_DELAY_CNTRL_1_TXCTL_DEL_SEL_FIELD_MASK,
    0,
    LPORT_RGMII_TX_DELAY_CNTRL_1_TXCTL_DEL_SEL_FIELD_WIDTH,
    LPORT_RGMII_TX_DELAY_CNTRL_1_TXCTL_DEL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_RX_DELAY_CNTRL_0_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_RX_DELAY_CNTRL_0_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_RGMII_RX_DELAY_CNTRL_0_RESERVED0_FIELD_MASK,
    0,
    LPORT_RGMII_RX_DELAY_CNTRL_0_RESERVED0_FIELD_WIDTH,
    LPORT_RGMII_RX_DELAY_CNTRL_0_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_RX_DELAY_CNTRL_0_RXD3_DEL_OVRD_EN
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_RX_DELAY_CNTRL_0_RXD3_DEL_OVRD_EN_FIELD =
{
    "RXD3_DEL_OVRD_EN",
#if RU_INCLUDE_DESC
    "",
    "rxd3 CKTAP delay override enable. When set enables CKTAP delay to be controlled from this register.",
#endif
    LPORT_RGMII_RX_DELAY_CNTRL_0_RXD3_DEL_OVRD_EN_FIELD_MASK,
    0,
    LPORT_RGMII_RX_DELAY_CNTRL_0_RXD3_DEL_OVRD_EN_FIELD_WIDTH,
    LPORT_RGMII_RX_DELAY_CNTRL_0_RXD3_DEL_OVRD_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_RX_DELAY_CNTRL_0_RXD3_DEL_SEL
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_RX_DELAY_CNTRL_0_RXD3_DEL_SEL_FIELD =
{
    "RXD3_DEL_SEL",
#if RU_INCLUDE_DESC
    "",
    "rxd3 CKTAP delay control. Refer to the CKTAP datasheet for programming.",
#endif
    LPORT_RGMII_RX_DELAY_CNTRL_0_RXD3_DEL_SEL_FIELD_MASK,
    0,
    LPORT_RGMII_RX_DELAY_CNTRL_0_RXD3_DEL_SEL_FIELD_WIDTH,
    LPORT_RGMII_RX_DELAY_CNTRL_0_RXD3_DEL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_RX_DELAY_CNTRL_0_RXD2_DEL_OVRD_EN
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_RX_DELAY_CNTRL_0_RXD2_DEL_OVRD_EN_FIELD =
{
    "RXD2_DEL_OVRD_EN",
#if RU_INCLUDE_DESC
    "",
    "rxd2 CKTAP delay override enable. When set enables CKTAP delay to be controlled from this register.",
#endif
    LPORT_RGMII_RX_DELAY_CNTRL_0_RXD2_DEL_OVRD_EN_FIELD_MASK,
    0,
    LPORT_RGMII_RX_DELAY_CNTRL_0_RXD2_DEL_OVRD_EN_FIELD_WIDTH,
    LPORT_RGMII_RX_DELAY_CNTRL_0_RXD2_DEL_OVRD_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_RX_DELAY_CNTRL_0_RXD2_DEL_SEL
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_RX_DELAY_CNTRL_0_RXD2_DEL_SEL_FIELD =
{
    "RXD2_DEL_SEL",
#if RU_INCLUDE_DESC
    "",
    "rxd2 CKTAP delay control. Refer to the CKTAP datasheet for programming.",
#endif
    LPORT_RGMII_RX_DELAY_CNTRL_0_RXD2_DEL_SEL_FIELD_MASK,
    0,
    LPORT_RGMII_RX_DELAY_CNTRL_0_RXD2_DEL_SEL_FIELD_WIDTH,
    LPORT_RGMII_RX_DELAY_CNTRL_0_RXD2_DEL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_RX_DELAY_CNTRL_0_RXD1_DEL_OVRD_EN
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_RX_DELAY_CNTRL_0_RXD1_DEL_OVRD_EN_FIELD =
{
    "RXD1_DEL_OVRD_EN",
#if RU_INCLUDE_DESC
    "",
    "rxd1 CKTAP delay override enable. When set enables CKTAP delay to be controlled from this register.",
#endif
    LPORT_RGMII_RX_DELAY_CNTRL_0_RXD1_DEL_OVRD_EN_FIELD_MASK,
    0,
    LPORT_RGMII_RX_DELAY_CNTRL_0_RXD1_DEL_OVRD_EN_FIELD_WIDTH,
    LPORT_RGMII_RX_DELAY_CNTRL_0_RXD1_DEL_OVRD_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_RX_DELAY_CNTRL_0_RXD1_DEL_SEL
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_RX_DELAY_CNTRL_0_RXD1_DEL_SEL_FIELD =
{
    "RXD1_DEL_SEL",
#if RU_INCLUDE_DESC
    "",
    "rxd1 CKTAP delay control. Refer to the CKTAP datasheet for programming.",
#endif
    LPORT_RGMII_RX_DELAY_CNTRL_0_RXD1_DEL_SEL_FIELD_MASK,
    0,
    LPORT_RGMII_RX_DELAY_CNTRL_0_RXD1_DEL_SEL_FIELD_WIDTH,
    LPORT_RGMII_RX_DELAY_CNTRL_0_RXD1_DEL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_RX_DELAY_CNTRL_0_RXD0_DEL_OVRD_EN
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_RX_DELAY_CNTRL_0_RXD0_DEL_OVRD_EN_FIELD =
{
    "RXD0_DEL_OVRD_EN",
#if RU_INCLUDE_DESC
    "",
    "rxd0 CKTAP delay override enable. When set enables CKTAP delay to be controlled from this register.",
#endif
    LPORT_RGMII_RX_DELAY_CNTRL_0_RXD0_DEL_OVRD_EN_FIELD_MASK,
    0,
    LPORT_RGMII_RX_DELAY_CNTRL_0_RXD0_DEL_OVRD_EN_FIELD_WIDTH,
    LPORT_RGMII_RX_DELAY_CNTRL_0_RXD0_DEL_OVRD_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_RX_DELAY_CNTRL_0_RXD0_DEL_SEL
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_RX_DELAY_CNTRL_0_RXD0_DEL_SEL_FIELD =
{
    "RXD0_DEL_SEL",
#if RU_INCLUDE_DESC
    "",
    "rxd0 CKTAP delay control. Refer to the CKTAP datasheet for programming.",
#endif
    LPORT_RGMII_RX_DELAY_CNTRL_0_RXD0_DEL_SEL_FIELD_MASK,
    0,
    LPORT_RGMII_RX_DELAY_CNTRL_0_RXD0_DEL_SEL_FIELD_WIDTH,
    LPORT_RGMII_RX_DELAY_CNTRL_0_RXD0_DEL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_RX_DELAY_CNTRL_1_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_RX_DELAY_CNTRL_1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_RGMII_RX_DELAY_CNTRL_1_RESERVED0_FIELD_MASK,
    0,
    LPORT_RGMII_RX_DELAY_CNTRL_1_RESERVED0_FIELD_WIDTH,
    LPORT_RGMII_RX_DELAY_CNTRL_1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_RX_DELAY_CNTRL_1_RXD7_DEL_OVRD_EN
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_RX_DELAY_CNTRL_1_RXD7_DEL_OVRD_EN_FIELD =
{
    "RXD7_DEL_OVRD_EN",
#if RU_INCLUDE_DESC
    "",
    "rxd7 CKTAP delay override enable. When set enables CKTAP delay to be controlled from this register.",
#endif
    LPORT_RGMII_RX_DELAY_CNTRL_1_RXD7_DEL_OVRD_EN_FIELD_MASK,
    0,
    LPORT_RGMII_RX_DELAY_CNTRL_1_RXD7_DEL_OVRD_EN_FIELD_WIDTH,
    LPORT_RGMII_RX_DELAY_CNTRL_1_RXD7_DEL_OVRD_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_RX_DELAY_CNTRL_1_RXD7_DEL_SEL
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_RX_DELAY_CNTRL_1_RXD7_DEL_SEL_FIELD =
{
    "RXD7_DEL_SEL",
#if RU_INCLUDE_DESC
    "",
    "rxd7 CKTAP delay control. Refer to the CKTAP datasheet for programming.",
#endif
    LPORT_RGMII_RX_DELAY_CNTRL_1_RXD7_DEL_SEL_FIELD_MASK,
    0,
    LPORT_RGMII_RX_DELAY_CNTRL_1_RXD7_DEL_SEL_FIELD_WIDTH,
    LPORT_RGMII_RX_DELAY_CNTRL_1_RXD7_DEL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_RX_DELAY_CNTRL_1_RXD6_DEL_OVRD_EN
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_RX_DELAY_CNTRL_1_RXD6_DEL_OVRD_EN_FIELD =
{
    "RXD6_DEL_OVRD_EN",
#if RU_INCLUDE_DESC
    "",
    "rxd6 CKTAP delay override enable. When set enables CKTAP delay to be controlled from this register.",
#endif
    LPORT_RGMII_RX_DELAY_CNTRL_1_RXD6_DEL_OVRD_EN_FIELD_MASK,
    0,
    LPORT_RGMII_RX_DELAY_CNTRL_1_RXD6_DEL_OVRD_EN_FIELD_WIDTH,
    LPORT_RGMII_RX_DELAY_CNTRL_1_RXD6_DEL_OVRD_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_RX_DELAY_CNTRL_1_RXD6_DEL_SEL
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_RX_DELAY_CNTRL_1_RXD6_DEL_SEL_FIELD =
{
    "RXD6_DEL_SEL",
#if RU_INCLUDE_DESC
    "",
    "rxd6 CKTAP delay control. Refer to the CKTAP datasheet for programming.",
#endif
    LPORT_RGMII_RX_DELAY_CNTRL_1_RXD6_DEL_SEL_FIELD_MASK,
    0,
    LPORT_RGMII_RX_DELAY_CNTRL_1_RXD6_DEL_SEL_FIELD_WIDTH,
    LPORT_RGMII_RX_DELAY_CNTRL_1_RXD6_DEL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_RX_DELAY_CNTRL_1_RXD5_DEL_OVRD_EN
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_RX_DELAY_CNTRL_1_RXD5_DEL_OVRD_EN_FIELD =
{
    "RXD5_DEL_OVRD_EN",
#if RU_INCLUDE_DESC
    "",
    "rxd5 CKTAP delay override enable. When set enables CKTAP delay to be controlled from this register.",
#endif
    LPORT_RGMII_RX_DELAY_CNTRL_1_RXD5_DEL_OVRD_EN_FIELD_MASK,
    0,
    LPORT_RGMII_RX_DELAY_CNTRL_1_RXD5_DEL_OVRD_EN_FIELD_WIDTH,
    LPORT_RGMII_RX_DELAY_CNTRL_1_RXD5_DEL_OVRD_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_RX_DELAY_CNTRL_1_RXD5_DEL_SEL
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_RX_DELAY_CNTRL_1_RXD5_DEL_SEL_FIELD =
{
    "RXD5_DEL_SEL",
#if RU_INCLUDE_DESC
    "",
    "rxd5 CKTAP delay control. Refer to the CKTAP datasheet for programming.",
#endif
    LPORT_RGMII_RX_DELAY_CNTRL_1_RXD5_DEL_SEL_FIELD_MASK,
    0,
    LPORT_RGMII_RX_DELAY_CNTRL_1_RXD5_DEL_SEL_FIELD_WIDTH,
    LPORT_RGMII_RX_DELAY_CNTRL_1_RXD5_DEL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_RX_DELAY_CNTRL_1_RXD4_DEL_OVRD_EN
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_RX_DELAY_CNTRL_1_RXD4_DEL_OVRD_EN_FIELD =
{
    "RXD4_DEL_OVRD_EN",
#if RU_INCLUDE_DESC
    "",
    "rxd4 CKTAP delay override enable. When set enables CKTAP delay to be controlled from this register.",
#endif
    LPORT_RGMII_RX_DELAY_CNTRL_1_RXD4_DEL_OVRD_EN_FIELD_MASK,
    0,
    LPORT_RGMII_RX_DELAY_CNTRL_1_RXD4_DEL_OVRD_EN_FIELD_WIDTH,
    LPORT_RGMII_RX_DELAY_CNTRL_1_RXD4_DEL_OVRD_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_RX_DELAY_CNTRL_1_RXD4_DEL_SEL
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_RX_DELAY_CNTRL_1_RXD4_DEL_SEL_FIELD =
{
    "RXD4_DEL_SEL",
#if RU_INCLUDE_DESC
    "",
    "rxd4 CKTAP delay control. Refer to the CKTAP datasheet for programming.",
#endif
    LPORT_RGMII_RX_DELAY_CNTRL_1_RXD4_DEL_SEL_FIELD_MASK,
    0,
    LPORT_RGMII_RX_DELAY_CNTRL_1_RXD4_DEL_SEL_FIELD_WIDTH,
    LPORT_RGMII_RX_DELAY_CNTRL_1_RXD4_DEL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_RX_DELAY_CNTRL_2_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_RX_DELAY_CNTRL_2_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_RGMII_RX_DELAY_CNTRL_2_RESERVED0_FIELD_MASK,
    0,
    LPORT_RGMII_RX_DELAY_CNTRL_2_RESERVED0_FIELD_WIDTH,
    LPORT_RGMII_RX_DELAY_CNTRL_2_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_RX_DELAY_CNTRL_2_RXCLK_DEL_OVRD_EN
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_RX_DELAY_CNTRL_2_RXCLK_DEL_OVRD_EN_FIELD =
{
    "RXCLK_DEL_OVRD_EN",
#if RU_INCLUDE_DESC
    "",
    "rxclk CKTAP delay override enable. When set enables CKTAP delay to be controlled from this register.",
#endif
    LPORT_RGMII_RX_DELAY_CNTRL_2_RXCLK_DEL_OVRD_EN_FIELD_MASK,
    0,
    LPORT_RGMII_RX_DELAY_CNTRL_2_RXCLK_DEL_OVRD_EN_FIELD_WIDTH,
    LPORT_RGMII_RX_DELAY_CNTRL_2_RXCLK_DEL_OVRD_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_RX_DELAY_CNTRL_2_RXCLK_DEL_SEL
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_RX_DELAY_CNTRL_2_RXCLK_DEL_SEL_FIELD =
{
    "RXCLK_DEL_SEL",
#if RU_INCLUDE_DESC
    "",
    "rxclk CKTAP delay control. Refer to the CKTAP datasheet for programming.",
#endif
    LPORT_RGMII_RX_DELAY_CNTRL_2_RXCLK_DEL_SEL_FIELD_MASK,
    0,
    LPORT_RGMII_RX_DELAY_CNTRL_2_RXCLK_DEL_SEL_FIELD_WIDTH,
    LPORT_RGMII_RX_DELAY_CNTRL_2_RXCLK_DEL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_RX_DELAY_CNTRL_2_RXCTL_NEG_DEL_OVRD_EN
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_RX_DELAY_CNTRL_2_RXCTL_NEG_DEL_OVRD_EN_FIELD =
{
    "RXCTL_NEG_DEL_OVRD_EN",
#if RU_INCLUDE_DESC
    "",
    "rxctl_neg CKTAP delay override enable. When set enables CKTAP delay to be controlled from this register.",
#endif
    LPORT_RGMII_RX_DELAY_CNTRL_2_RXCTL_NEG_DEL_OVRD_EN_FIELD_MASK,
    0,
    LPORT_RGMII_RX_DELAY_CNTRL_2_RXCTL_NEG_DEL_OVRD_EN_FIELD_WIDTH,
    LPORT_RGMII_RX_DELAY_CNTRL_2_RXCTL_NEG_DEL_OVRD_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_RX_DELAY_CNTRL_2_RXCTL_NEG_DEL_SEL
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_RX_DELAY_CNTRL_2_RXCTL_NEG_DEL_SEL_FIELD =
{
    "RXCTL_NEG_DEL_SEL",
#if RU_INCLUDE_DESC
    "",
    "rxctl_neg CKTAP delay control. Refer to the CKTAP datasheet for programming.",
#endif
    LPORT_RGMII_RX_DELAY_CNTRL_2_RXCTL_NEG_DEL_SEL_FIELD_MASK,
    0,
    LPORT_RGMII_RX_DELAY_CNTRL_2_RXCTL_NEG_DEL_SEL_FIELD_WIDTH,
    LPORT_RGMII_RX_DELAY_CNTRL_2_RXCTL_NEG_DEL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_RX_DELAY_CNTRL_2_RXCTL_POS_DEL_OVRD_EN
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_RX_DELAY_CNTRL_2_RXCTL_POS_DEL_OVRD_EN_FIELD =
{
    "RXCTL_POS_DEL_OVRD_EN",
#if RU_INCLUDE_DESC
    "",
    "rxctl_pos CKTAP delay override enable. When set enables CKTAP delay to be controlled from this register.",
#endif
    LPORT_RGMII_RX_DELAY_CNTRL_2_RXCTL_POS_DEL_OVRD_EN_FIELD_MASK,
    0,
    LPORT_RGMII_RX_DELAY_CNTRL_2_RXCTL_POS_DEL_OVRD_EN_FIELD_WIDTH,
    LPORT_RGMII_RX_DELAY_CNTRL_2_RXCTL_POS_DEL_OVRD_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_RGMII_RX_DELAY_CNTRL_2_RXCTL_POS_DEL_SEL
 ******************************************************************************/
const ru_field_rec LPORT_RGMII_RX_DELAY_CNTRL_2_RXCTL_POS_DEL_SEL_FIELD =
{
    "RXCTL_POS_DEL_SEL",
#if RU_INCLUDE_DESC
    "",
    "rxctl_pos CKTAP delay control. Refer to the CKTAP datasheet for programming.",
#endif
    LPORT_RGMII_RX_DELAY_CNTRL_2_RXCTL_POS_DEL_SEL_FIELD_MASK,
    0,
    LPORT_RGMII_RX_DELAY_CNTRL_2_RXCTL_POS_DEL_SEL_FIELD_WIDTH,
    LPORT_RGMII_RX_DELAY_CNTRL_2_RXCTL_POS_DEL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: LPORT_RGMII_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_RGMII_CNTRL_FIELDS[] =
{
    &LPORT_RGMII_CNTRL_RESERVED0_FIELD,
    &LPORT_RGMII_CNTRL_COL_CRS_MASK_FIELD,
    &LPORT_RGMII_CNTRL_RX_ERR_MASK_FIELD,
    &LPORT_RGMII_CNTRL_LPI_COUNT_FIELD,
    &LPORT_RGMII_CNTRL_TX_CLK_STOP_EN_FIELD,
    &LPORT_RGMII_CNTRL_TX_PAUSE_EN_FIELD,
    &LPORT_RGMII_CNTRL_RX_PAUSE_EN_FIELD,
    &LPORT_RGMII_CNTRL_RVMII_REF_SEL_FIELD,
    &LPORT_RGMII_CNTRL_PORT_MODE_FIELD,
    &LPORT_RGMII_CNTRL_ID_MODE_DIS_FIELD,
    &LPORT_RGMII_CNTRL_RGMII_MODE_EN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_RGMII_CNTRL_REG = 
{
    "CNTRL",
#if RU_INCLUDE_DESC
    "RGMII port 2 Control Register",
    "",
#endif
    LPORT_RGMII_CNTRL_REG_OFFSET,
    0,
    0,
    213,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    11,
    LPORT_RGMII_CNTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LPORT_RGMII_IB_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_RGMII_IB_STATUS_FIELDS[] =
{
    &LPORT_RGMII_IB_STATUS_RESERVED0_FIELD,
    &LPORT_RGMII_IB_STATUS_IB_STATUS_OVRD_FIELD,
    &LPORT_RGMII_IB_STATUS_LINK_DECODE_FIELD,
    &LPORT_RGMII_IB_STATUS_DUPLEX_DECODE_FIELD,
    &LPORT_RGMII_IB_STATUS_SPEED_DECODE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_RGMII_IB_STATUS_REG = 
{
    "IB_STATUS",
#if RU_INCLUDE_DESC
    "RGMII port 2 InBand Status Register",
    "",
#endif
    LPORT_RGMII_IB_STATUS_REG_OFFSET,
    0,
    0,
    214,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    LPORT_RGMII_IB_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LPORT_RGMII_RX_CLOCK_DELAY_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_FIELDS[] =
{
    &LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_RESERVED0_FIELD,
    &LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_RESET_FIELD,
    &LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_DLY_OVERRIDE_FIELD,
    &LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_DLY_SEL_FIELD,
    &LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_BYPASS_FIELD,
    &LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_IDDQ_FIELD,
    &LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_DRNG_FIELD,
    &LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_CTRI_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_REG = 
{
    "RX_CLOCK_DELAY_CNTRL",
#if RU_INCLUDE_DESC
    "RGMII port 2 RX Clock Delay Control Register",
    "",
#endif
    LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_REG_OFFSET,
    0,
    0,
    215,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LPORT_RGMII_ATE_RX_CNTRL_EXP_DATA
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_RGMII_ATE_RX_CNTRL_EXP_DATA_FIELDS[] =
{
    &LPORT_RGMII_ATE_RX_CNTRL_EXP_DATA_RESERVED0_FIELD,
    &LPORT_RGMII_ATE_RX_CNTRL_EXP_DATA_ATE_EN_FIELD,
    &LPORT_RGMII_ATE_RX_CNTRL_EXP_DATA_PKT_COUNT_RST_FIELD,
    &LPORT_RGMII_ATE_RX_CNTRL_EXP_DATA_GOOD_COUNT_FIELD,
    &LPORT_RGMII_ATE_RX_CNTRL_EXP_DATA_EXPECTED_DATA_1_FIELD,
    &LPORT_RGMII_ATE_RX_CNTRL_EXP_DATA_EXPECTED_DATA_0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_RGMII_ATE_RX_CNTRL_EXP_DATA_REG = 
{
    "ATE_RX_CNTRL_EXP_DATA",
#if RU_INCLUDE_DESC
    "RGMII port 2 ATE RX Control and Expected Data Register",
    "",
#endif
    LPORT_RGMII_ATE_RX_CNTRL_EXP_DATA_REG_OFFSET,
    0,
    0,
    216,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    LPORT_RGMII_ATE_RX_CNTRL_EXP_DATA_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LPORT_RGMII_ATE_RX_EXP_DATA_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_RGMII_ATE_RX_EXP_DATA_1_FIELDS[] =
{
    &LPORT_RGMII_ATE_RX_EXP_DATA_1_RESERVED0_FIELD,
    &LPORT_RGMII_ATE_RX_EXP_DATA_1_EXPECTED_DATA_3_FIELD,
    &LPORT_RGMII_ATE_RX_EXP_DATA_1_EXPECTED_DATA_2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_RGMII_ATE_RX_EXP_DATA_1_REG = 
{
    "ATE_RX_EXP_DATA_1",
#if RU_INCLUDE_DESC
    "RGMII port 2 ATE RX Expected Data 1 Register",
    "",
#endif
    LPORT_RGMII_ATE_RX_EXP_DATA_1_REG_OFFSET,
    0,
    0,
    217,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    LPORT_RGMII_ATE_RX_EXP_DATA_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LPORT_RGMII_ATE_RX_STATUS_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_RGMII_ATE_RX_STATUS_0_FIELDS[] =
{
    &LPORT_RGMII_ATE_RX_STATUS_0_RESERVED0_FIELD,
    &LPORT_RGMII_ATE_RX_STATUS_0_RX_OK_FIELD,
    &LPORT_RGMII_ATE_RX_STATUS_0_RECEIVED_DATA_1_FIELD,
    &LPORT_RGMII_ATE_RX_STATUS_0_RECEIVED_DATA_0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_RGMII_ATE_RX_STATUS_0_REG = 
{
    "ATE_RX_STATUS_0",
#if RU_INCLUDE_DESC
    "RGMII port 2 ATE RX Status 0 Register",
    "",
#endif
    LPORT_RGMII_ATE_RX_STATUS_0_REG_OFFSET,
    0,
    0,
    218,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    LPORT_RGMII_ATE_RX_STATUS_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LPORT_RGMII_ATE_RX_STATUS_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_RGMII_ATE_RX_STATUS_1_FIELDS[] =
{
    &LPORT_RGMII_ATE_RX_STATUS_1_RESERVED0_FIELD,
    &LPORT_RGMII_ATE_RX_STATUS_1_RECEIVED_DATA_3_FIELD,
    &LPORT_RGMII_ATE_RX_STATUS_1_RECEIVED_DATA_2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_RGMII_ATE_RX_STATUS_1_REG = 
{
    "ATE_RX_STATUS_1",
#if RU_INCLUDE_DESC
    "RGMII port 2 ATE RX Status 1 Register",
    "",
#endif
    LPORT_RGMII_ATE_RX_STATUS_1_REG_OFFSET,
    0,
    0,
    219,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    LPORT_RGMII_ATE_RX_STATUS_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LPORT_RGMII_ATE_TX_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_RGMII_ATE_TX_CNTRL_FIELDS[] =
{
    &LPORT_RGMII_ATE_TX_CNTRL_RESERVED0_FIELD,
    &LPORT_RGMII_ATE_TX_CNTRL_PKT_IPG_FIELD,
    &LPORT_RGMII_ATE_TX_CNTRL_PAYLOAD_LENGTH_FIELD,
    &LPORT_RGMII_ATE_TX_CNTRL_PKT_CNT_FIELD,
    &LPORT_RGMII_ATE_TX_CNTRL_PKT_GEN_EN_FIELD,
    &LPORT_RGMII_ATE_TX_CNTRL_START_STOP_FIELD,
    &LPORT_RGMII_ATE_TX_CNTRL_START_STOP_OVRD_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_RGMII_ATE_TX_CNTRL_REG = 
{
    "ATE_TX_CNTRL",
#if RU_INCLUDE_DESC
    "RGMII port 2 ATE TX Control Register",
    "",
#endif
    LPORT_RGMII_ATE_TX_CNTRL_REG_OFFSET,
    0,
    0,
    220,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    LPORT_RGMII_ATE_TX_CNTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LPORT_RGMII_ATE_TX_DATA_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_RGMII_ATE_TX_DATA_0_FIELDS[] =
{
    &LPORT_RGMII_ATE_TX_DATA_0_RESERVED0_FIELD,
    &LPORT_RGMII_ATE_TX_DATA_0_TX_DATA_1_FIELD,
    &LPORT_RGMII_ATE_TX_DATA_0_TX_DATA_0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_RGMII_ATE_TX_DATA_0_REG = 
{
    "ATE_TX_DATA_0",
#if RU_INCLUDE_DESC
    "RGMII port 2 ATE TX Data 0 Register",
    "",
#endif
    LPORT_RGMII_ATE_TX_DATA_0_REG_OFFSET,
    0,
    0,
    221,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    LPORT_RGMII_ATE_TX_DATA_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LPORT_RGMII_ATE_TX_DATA_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_RGMII_ATE_TX_DATA_1_FIELDS[] =
{
    &LPORT_RGMII_ATE_TX_DATA_1_RESERVED0_FIELD,
    &LPORT_RGMII_ATE_TX_DATA_1_TX_DATA_3_FIELD,
    &LPORT_RGMII_ATE_TX_DATA_1_TX_DATA_2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_RGMII_ATE_TX_DATA_1_REG = 
{
    "ATE_TX_DATA_1",
#if RU_INCLUDE_DESC
    "RGMII port 2 ATE TX Data 1 Register",
    "",
#endif
    LPORT_RGMII_ATE_TX_DATA_1_REG_OFFSET,
    0,
    0,
    222,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    LPORT_RGMII_ATE_TX_DATA_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LPORT_RGMII_ATE_TX_DATA_2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_RGMII_ATE_TX_DATA_2_FIELDS[] =
{
    &LPORT_RGMII_ATE_TX_DATA_2_ETHER_TYPE_FIELD,
    &LPORT_RGMII_ATE_TX_DATA_2_TX_DATA_5_FIELD,
    &LPORT_RGMII_ATE_TX_DATA_2_TX_DATA_4_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_RGMII_ATE_TX_DATA_2_REG = 
{
    "ATE_TX_DATA_2",
#if RU_INCLUDE_DESC
    "RGMII port 2 ATE TX Data 2 Register",
    "",
#endif
    LPORT_RGMII_ATE_TX_DATA_2_REG_OFFSET,
    0,
    0,
    223,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    LPORT_RGMII_ATE_TX_DATA_2_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LPORT_RGMII_TX_DELAY_CNTRL_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_RGMII_TX_DELAY_CNTRL_0_FIELDS[] =
{
    &LPORT_RGMII_TX_DELAY_CNTRL_0_RESERVED0_FIELD,
    &LPORT_RGMII_TX_DELAY_CNTRL_0_TXD3_DEL_OVRD_EN_FIELD,
    &LPORT_RGMII_TX_DELAY_CNTRL_0_TXD3_DEL_SEL_FIELD,
    &LPORT_RGMII_TX_DELAY_CNTRL_0_TXD2_DEL_OVRD_EN_FIELD,
    &LPORT_RGMII_TX_DELAY_CNTRL_0_TXD2_DEL_SEL_FIELD,
    &LPORT_RGMII_TX_DELAY_CNTRL_0_TXD1_DEL_OVRD_EN_FIELD,
    &LPORT_RGMII_TX_DELAY_CNTRL_0_TXD1_DEL_SEL_FIELD,
    &LPORT_RGMII_TX_DELAY_CNTRL_0_TXD0_DEL_OVRD_EN_FIELD,
    &LPORT_RGMII_TX_DELAY_CNTRL_0_TXD0_DEL_SEL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_RGMII_TX_DELAY_CNTRL_0_REG = 
{
    "TX_DELAY_CNTRL_0",
#if RU_INCLUDE_DESC
    "RGMII Append 2 TX Delay Control 0 Register",
    "",
#endif
    LPORT_RGMII_TX_DELAY_CNTRL_0_REG_OFFSET,
    0,
    0,
    224,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    LPORT_RGMII_TX_DELAY_CNTRL_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LPORT_RGMII_TX_DELAY_CNTRL_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_RGMII_TX_DELAY_CNTRL_1_FIELDS[] =
{
    &LPORT_RGMII_TX_DELAY_CNTRL_1_RESERVED0_FIELD,
    &LPORT_RGMII_TX_DELAY_CNTRL_1_TXCLK_ID_DEL_OVRD_EN_FIELD,
    &LPORT_RGMII_TX_DELAY_CNTRL_1_TXCLK_ID_DEL_SEL_FIELD,
    &LPORT_RGMII_TX_DELAY_CNTRL_1_TXCLK_DEL_OVRD_EN_FIELD,
    &LPORT_RGMII_TX_DELAY_CNTRL_1_TXCLK_DEL_SEL_FIELD,
    &LPORT_RGMII_TX_DELAY_CNTRL_1_TXCTL_DEL_OVRD_EN_FIELD,
    &LPORT_RGMII_TX_DELAY_CNTRL_1_TXCTL_DEL_SEL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_RGMII_TX_DELAY_CNTRL_1_REG = 
{
    "TX_DELAY_CNTRL_1",
#if RU_INCLUDE_DESC
    "RGMII Append 2 TX Delay Control 1 Register",
    "",
#endif
    LPORT_RGMII_TX_DELAY_CNTRL_1_REG_OFFSET,
    0,
    0,
    225,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    LPORT_RGMII_TX_DELAY_CNTRL_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LPORT_RGMII_RX_DELAY_CNTRL_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_RGMII_RX_DELAY_CNTRL_0_FIELDS[] =
{
    &LPORT_RGMII_RX_DELAY_CNTRL_0_RESERVED0_FIELD,
    &LPORT_RGMII_RX_DELAY_CNTRL_0_RXD3_DEL_OVRD_EN_FIELD,
    &LPORT_RGMII_RX_DELAY_CNTRL_0_RXD3_DEL_SEL_FIELD,
    &LPORT_RGMII_RX_DELAY_CNTRL_0_RXD2_DEL_OVRD_EN_FIELD,
    &LPORT_RGMII_RX_DELAY_CNTRL_0_RXD2_DEL_SEL_FIELD,
    &LPORT_RGMII_RX_DELAY_CNTRL_0_RXD1_DEL_OVRD_EN_FIELD,
    &LPORT_RGMII_RX_DELAY_CNTRL_0_RXD1_DEL_SEL_FIELD,
    &LPORT_RGMII_RX_DELAY_CNTRL_0_RXD0_DEL_OVRD_EN_FIELD,
    &LPORT_RGMII_RX_DELAY_CNTRL_0_RXD0_DEL_SEL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_RGMII_RX_DELAY_CNTRL_0_REG = 
{
    "RX_DELAY_CNTRL_0",
#if RU_INCLUDE_DESC
    "RGMII Append 2 RX Delay Control 0 Register",
    "",
#endif
    LPORT_RGMII_RX_DELAY_CNTRL_0_REG_OFFSET,
    0,
    0,
    226,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    LPORT_RGMII_RX_DELAY_CNTRL_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LPORT_RGMII_RX_DELAY_CNTRL_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_RGMII_RX_DELAY_CNTRL_1_FIELDS[] =
{
    &LPORT_RGMII_RX_DELAY_CNTRL_1_RESERVED0_FIELD,
    &LPORT_RGMII_RX_DELAY_CNTRL_1_RXD7_DEL_OVRD_EN_FIELD,
    &LPORT_RGMII_RX_DELAY_CNTRL_1_RXD7_DEL_SEL_FIELD,
    &LPORT_RGMII_RX_DELAY_CNTRL_1_RXD6_DEL_OVRD_EN_FIELD,
    &LPORT_RGMII_RX_DELAY_CNTRL_1_RXD6_DEL_SEL_FIELD,
    &LPORT_RGMII_RX_DELAY_CNTRL_1_RXD5_DEL_OVRD_EN_FIELD,
    &LPORT_RGMII_RX_DELAY_CNTRL_1_RXD5_DEL_SEL_FIELD,
    &LPORT_RGMII_RX_DELAY_CNTRL_1_RXD4_DEL_OVRD_EN_FIELD,
    &LPORT_RGMII_RX_DELAY_CNTRL_1_RXD4_DEL_SEL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_RGMII_RX_DELAY_CNTRL_1_REG = 
{
    "RX_DELAY_CNTRL_1",
#if RU_INCLUDE_DESC
    "RGMII Append 2 RX Delay Control 1 Register",
    "",
#endif
    LPORT_RGMII_RX_DELAY_CNTRL_1_REG_OFFSET,
    0,
    0,
    227,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    LPORT_RGMII_RX_DELAY_CNTRL_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LPORT_RGMII_RX_DELAY_CNTRL_2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_RGMII_RX_DELAY_CNTRL_2_FIELDS[] =
{
    &LPORT_RGMII_RX_DELAY_CNTRL_2_RESERVED0_FIELD,
    &LPORT_RGMII_RX_DELAY_CNTRL_2_RXCLK_DEL_OVRD_EN_FIELD,
    &LPORT_RGMII_RX_DELAY_CNTRL_2_RXCLK_DEL_SEL_FIELD,
    &LPORT_RGMII_RX_DELAY_CNTRL_2_RXCTL_NEG_DEL_OVRD_EN_FIELD,
    &LPORT_RGMII_RX_DELAY_CNTRL_2_RXCTL_NEG_DEL_SEL_FIELD,
    &LPORT_RGMII_RX_DELAY_CNTRL_2_RXCTL_POS_DEL_OVRD_EN_FIELD,
    &LPORT_RGMII_RX_DELAY_CNTRL_2_RXCTL_POS_DEL_SEL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_RGMII_RX_DELAY_CNTRL_2_REG = 
{
    "RX_DELAY_CNTRL_2",
#if RU_INCLUDE_DESC
    "RGMII Append 2 RX Delay Control 2 Register",
    "",
#endif
    LPORT_RGMII_RX_DELAY_CNTRL_2_REG_OFFSET,
    0,
    0,
    228,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    LPORT_RGMII_RX_DELAY_CNTRL_2_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: LPORT_RGMII
 ******************************************************************************/
static const ru_reg_rec *LPORT_RGMII_REGS[] =
{
    &LPORT_RGMII_CNTRL_REG,
    &LPORT_RGMII_IB_STATUS_REG,
    &LPORT_RGMII_RX_CLOCK_DELAY_CNTRL_REG,
    &LPORT_RGMII_ATE_RX_CNTRL_EXP_DATA_REG,
    &LPORT_RGMII_ATE_RX_EXP_DATA_1_REG,
    &LPORT_RGMII_ATE_RX_STATUS_0_REG,
    &LPORT_RGMII_ATE_RX_STATUS_1_REG,
    &LPORT_RGMII_ATE_TX_CNTRL_REG,
    &LPORT_RGMII_ATE_TX_DATA_0_REG,
    &LPORT_RGMII_ATE_TX_DATA_1_REG,
    &LPORT_RGMII_ATE_TX_DATA_2_REG,
    &LPORT_RGMII_TX_DELAY_CNTRL_0_REG,
    &LPORT_RGMII_TX_DELAY_CNTRL_1_REG,
    &LPORT_RGMII_RX_DELAY_CNTRL_0_REG,
    &LPORT_RGMII_RX_DELAY_CNTRL_1_REG,
    &LPORT_RGMII_RX_DELAY_CNTRL_2_REG,
};

unsigned long LPORT_RGMII_ADDRS[] =
{
    0x8013c100,
    0x8013c10c,
    0x8013c118,
};

const ru_block_rec LPORT_RGMII_BLOCK = 
{
    "LPORT_RGMII",
    LPORT_RGMII_ADDRS,
    3,
    16,
    LPORT_RGMII_REGS
};

/* End of file BCM6858_A0LPORT_RGMII.c */
