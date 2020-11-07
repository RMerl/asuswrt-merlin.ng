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
 * Field: MISC_0_ONU2G_PMD_STATUS_SEL
 ******************************************************************************/
const ru_field_rec MISC_0_ONU2G_PMD_STATUS_SEL_FIELD =
{
    "ONU2G_PMD_STATUS_SEL",
#if RU_INCLUDE_DESC
    "ONU2G_PMD_STATUS_SEL",
    "Not used in this chip."
    "Reset value is 0x0.",
#endif
    MISC_0_ONU2G_PMD_STATUS_SEL_FIELD_MASK,
    0,
    MISC_0_ONU2G_PMD_STATUS_SEL_FIELD_WIDTH,
    MISC_0_ONU2G_PMD_STATUS_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_0_REFIN_EN
 ******************************************************************************/
const ru_field_rec MISC_0_REFIN_EN_FIELD =
{
    "REFIN_EN",
#if RU_INCLUDE_DESC
    "REFIN_EN",
    "Not used in this chip."
    "Reset value is 0x0.",
#endif
    MISC_0_REFIN_EN_FIELD_MASK,
    0,
    MISC_0_REFIN_EN_FIELD_WIDTH,
    MISC_0_REFIN_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_0_REFOUT_EN
 ******************************************************************************/
const ru_field_rec MISC_0_REFOUT_EN_FIELD =
{
    "REFOUT_EN",
#if RU_INCLUDE_DESC
    "REFOUT_EN",
    "Not used in this chip."
    "Reset value is 0x0.",
#endif
    MISC_0_REFOUT_EN_FIELD_MASK,
    0,
    MISC_0_REFOUT_EN_FIELD_WIDTH,
    MISC_0_REFOUT_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_0_MDIO_MODE
 ******************************************************************************/
const ru_field_rec MISC_0_MDIO_MODE_FIELD =
{
    "MDIO_MODE",
#if RU_INCLUDE_DESC
    "MDIO_MODE",
    "MDIO transaction indicator needs to be asserted in a configuration"
    "where an external mdio controller is trying to access the internal"
    "PMD registers directly."
    "Reset value is 0x0.",
#endif
    MISC_0_MDIO_MODE_FIELD_MASK,
    0,
    MISC_0_MDIO_MODE_FIELD_WIDTH,
    MISC_0_MDIO_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_0_MDIO_FAST_MODE
 ******************************************************************************/
const ru_field_rec MISC_0_MDIO_FAST_MODE_FIELD =
{
    "MDIO_FAST_MODE",
#if RU_INCLUDE_DESC
    "MDIO_FAST_MODE",
    "Debug bit that disables the 32-bit preamble to allow mdio frames to"
    "run at 2x speed."
    "Reset value is 0x0.",
#endif
    MISC_0_MDIO_FAST_MODE_FIELD_MASK,
    0,
    MISC_0_MDIO_FAST_MODE_FIELD_WIDTH,
    MISC_0_MDIO_FAST_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_0_EPON_TX_FIFO_OFF_LD
 ******************************************************************************/
const ru_field_rec MISC_0_EPON_TX_FIFO_OFF_LD_FIELD =
{
    "EPON_TX_FIFO_OFF_LD",
#if RU_INCLUDE_DESC
    "EPON_TX_FIFO_OFF_LD",
    "Load vlue in cr_xgwan_top_wan_misc_epon_tx_fifo_off.",
#endif
    MISC_0_EPON_TX_FIFO_OFF_LD_FIELD_MASK,
    0,
    MISC_0_EPON_TX_FIFO_OFF_LD_FIELD_WIDTH,
    MISC_0_EPON_TX_FIFO_OFF_LD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_0_ONU2G_PHYA
 ******************************************************************************/
const ru_field_rec MISC_0_ONU2G_PHYA_FIELD =
{
    "ONU2G_PHYA",
#if RU_INCLUDE_DESC
    "ONU2G_PHYA",
    "Strap input for selecting the port address to decode on the mdio"
    "transaction."
    "Reset value is 0x0.",
#endif
    MISC_0_ONU2G_PHYA_FIELD_MASK,
    0,
    MISC_0_ONU2G_PHYA_FIELD_WIDTH,
    MISC_0_ONU2G_PHYA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_0_EPON_GBOX_RX_WIDTH_MODE
 ******************************************************************************/
const ru_field_rec MISC_0_EPON_GBOX_RX_WIDTH_MODE_FIELD =
{
    "EPON_GBOX_RX_WIDTH_MODE",
#if RU_INCLUDE_DESC
    "EPON_GBOX_RX_WIDTH_MODE",
    "cr_wan_top_wan_misc_wan_top_misc_0_epon_gbox_pon_rx_width_mode",
#endif
    MISC_0_EPON_GBOX_RX_WIDTH_MODE_FIELD_MASK,
    0,
    MISC_0_EPON_GBOX_RX_WIDTH_MODE_FIELD_WIDTH,
    MISC_0_EPON_GBOX_RX_WIDTH_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_0_EPON_AE_2P5_FULL_RATE_MODE
 ******************************************************************************/
const ru_field_rec MISC_0_EPON_AE_2P5_FULL_RATE_MODE_FIELD =
{
    "EPON_AE_2P5_FULL_RATE_MODE",
#if RU_INCLUDE_DESC
    "EPON_AE_2P5_FULL_RATE_MODE",
    "cr_wan_top_wan_misc_wan_top_misc_0_epon_ae_2p5_full_rate_mode",
#endif
    MISC_0_EPON_AE_2P5_FULL_RATE_MODE_FIELD_MASK,
    0,
    MISC_0_EPON_AE_2P5_FULL_RATE_MODE_FIELD_WIDTH,
    MISC_0_EPON_AE_2P5_FULL_RATE_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_0_RESERVED0
 ******************************************************************************/
const ru_field_rec MISC_0_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    MISC_0_RESERVED0_FIELD_MASK,
    0,
    MISC_0_RESERVED0_FIELD_WIDTH,
    MISC_0_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_0_PMD_LANE_MODE
 ******************************************************************************/
const ru_field_rec MISC_0_PMD_LANE_MODE_FIELD =
{
    "PMD_LANE_MODE",
#if RU_INCLUDE_DESC
    "PMD_LANE_MODE",
    "Reserved mode bus for lane. Mode bus for lane used by the PCS to"
    "communicate lane info to PMD. This bus should only be written to"
    "when the lane is in reset since the firmware will only read this"
    "after coming out of reset. This signal will be latched to a lane"
    "based register during core_dp_rstb. Asynchronous signal to the PMD"
    "Reset value is 0x0.",
#endif
    MISC_0_PMD_LANE_MODE_FIELD_MASK,
    0,
    MISC_0_PMD_LANE_MODE_FIELD_WIDTH,
    MISC_0_PMD_LANE_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_1_PMD_CORE_0_MODE
 ******************************************************************************/
const ru_field_rec MISC_1_PMD_CORE_0_MODE_FIELD =
{
    "PMD_CORE_0_MODE",
#if RU_INCLUDE_DESC
    "PMD_CORE_0_MODE",
    "Reserved mode bus for the entire core. Mode bus for core used by"
    "the PCS to communicate core info to PMD . This bus should only be"
    "written to when the core is in reset since the firmware will only"
    "read this after coming out of reset. This signal will be latched to"
    "a core register during core_dp_rstb. Asynchronous signal to the PMD"
    "Reset value is 0x0.",
#endif
    MISC_1_PMD_CORE_0_MODE_FIELD_MASK,
    0,
    MISC_1_PMD_CORE_0_MODE_FIELD_WIDTH,
    MISC_1_PMD_CORE_0_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_1_PMD_CORE_1_MODE
 ******************************************************************************/
const ru_field_rec MISC_1_PMD_CORE_1_MODE_FIELD =
{
    "PMD_CORE_1_MODE",
#if RU_INCLUDE_DESC
    "PMD_CORE_1_MODE",
    "Reserved mode bus for the entire core. Mode bus for core used by"
    "the PCS to communicate core info to PMD . This bus should only be"
    "written to when the core is in reset since the firmware will only"
    "read this after coming out of reset. This signal will be latched to"
    "a core register during core_dp_rstb. Asynchronous signal to the PMD"
    "Reset value is 0x0.",
#endif
    MISC_1_PMD_CORE_1_MODE_FIELD_MASK,
    0,
    MISC_1_PMD_CORE_1_MODE_FIELD_WIDTH,
    MISC_1_PMD_CORE_1_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_2_PMD_RX_MODE
 ******************************************************************************/
const ru_field_rec MISC_2_PMD_RX_MODE_FIELD =
{
    "PMD_RX_MODE",
#if RU_INCLUDE_DESC
    "PMD_RX_MODE",
    "EEE rx mode function for lane. Asynchronous signal to the PMD"
    "Reset value is 0x0.",
#endif
    MISC_2_PMD_RX_MODE_FIELD_MASK,
    0,
    MISC_2_PMD_RX_MODE_FIELD_WIDTH,
    MISC_2_PMD_RX_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_2_PMD_LN_DP_H_RSTB
 ******************************************************************************/
const ru_field_rec MISC_2_PMD_LN_DP_H_RSTB_FIELD =
{
    "PMD_LN_DP_H_RSTB",
#if RU_INCLUDE_DESC
    "PMD_LN_DP_H_RSTB",
    "Lane datapath reset, does not reset registers. Active Low. Minimum"
    "assertion time: 25 comclk period."
    "Reset value is 0x0.",
#endif
    MISC_2_PMD_LN_DP_H_RSTB_FIELD_MASK,
    0,
    MISC_2_PMD_LN_DP_H_RSTB_FIELD_WIDTH,
    MISC_2_PMD_LN_DP_H_RSTB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_2_PMD_LN_H_RSTB
 ******************************************************************************/
const ru_field_rec MISC_2_PMD_LN_H_RSTB_FIELD =
{
    "PMD_LN_H_RSTB",
#if RU_INCLUDE_DESC
    "PMD_LN_H_RSTB",
    "Lane reset registers and data path. Active Low. Minimum assertion"
    "time: 25 comclk period."
    "Reset value is 0x0.",
#endif
    MISC_2_PMD_LN_H_RSTB_FIELD_MASK,
    0,
    MISC_2_PMD_LN_H_RSTB_FIELD_WIDTH,
    MISC_2_PMD_LN_H_RSTB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_2_PMD_CORE_0_DP_H_RSTB
 ******************************************************************************/
const ru_field_rec MISC_2_PMD_CORE_0_DP_H_RSTB_FIELD =
{
    "PMD_CORE_0_DP_H_RSTB",
#if RU_INCLUDE_DESC
    "PMD_CORE_0_DP_H_RSTB",
    "Core reset for datapath for all lanes and corresponding PLL. Does"
    "not reset registers. Active Low. Minimum assertion time: 25 comclk"
    "period."
    "Reset value is 0x0.",
#endif
    MISC_2_PMD_CORE_0_DP_H_RSTB_FIELD_MASK,
    0,
    MISC_2_PMD_CORE_0_DP_H_RSTB_FIELD_WIDTH,
    MISC_2_PMD_CORE_0_DP_H_RSTB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_2_PMD_CORE_1_DP_H_RSTB
 ******************************************************************************/
const ru_field_rec MISC_2_PMD_CORE_1_DP_H_RSTB_FIELD =
{
    "PMD_CORE_1_DP_H_RSTB",
#if RU_INCLUDE_DESC
    "PMD_CORE_1_DP_H_RSTB",
    "Core reset for datapath for all lanes and corresponding PLL. Does"
    "not reset registers. Active Low. Minimum assertion time: 25 comclk"
    "period."
    "Reset value is 0x0.",
#endif
    MISC_2_PMD_CORE_1_DP_H_RSTB_FIELD_MASK,
    0,
    MISC_2_PMD_CORE_1_DP_H_RSTB_FIELD_WIDTH,
    MISC_2_PMD_CORE_1_DP_H_RSTB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_2_PMD_POR_H_RSTB
 ******************************************************************************/
const ru_field_rec MISC_2_PMD_POR_H_RSTB_FIELD =
{
    "PMD_POR_H_RSTB",
#if RU_INCLUDE_DESC
    "PMD_POR_H_RSTB",
    "PMD main reset, resets registers, data path for entire core"
    "including all lanes. Active Low. Minimum assertion time: 25 comclk"
    "period."
    "Reset value is 0x0.",
#endif
    MISC_2_PMD_POR_H_RSTB_FIELD_MASK,
    0,
    MISC_2_PMD_POR_H_RSTB_FIELD_WIDTH,
    MISC_2_PMD_POR_H_RSTB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_2_PMD_EXT_LOS
 ******************************************************************************/
const ru_field_rec MISC_2_PMD_EXT_LOS_FIELD =
{
    "PMD_EXT_LOS",
#if RU_INCLUDE_DESC
    "PMD_EXT_LOS",
    "External Loss of signal. LOS = 1. Signal presence = 0."
    "Reset value is 0x0.",
#endif
    MISC_2_PMD_EXT_LOS_FIELD_MASK,
    0,
    MISC_2_PMD_EXT_LOS_FIELD_WIDTH,
    MISC_2_PMD_EXT_LOS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_2_PMD_LN_TX_H_PWRDN
 ******************************************************************************/
const ru_field_rec MISC_2_PMD_LN_TX_H_PWRDN_FIELD =
{
    "PMD_LN_TX_H_PWRDN",
#if RU_INCLUDE_DESC
    "PMD_LN_TX_H_PWRDN",
    "Lane TX power down. Minimum assertion time: 25 comclk period."
    "Reset value is 0x0.",
#endif
    MISC_2_PMD_LN_TX_H_PWRDN_FIELD_MASK,
    0,
    MISC_2_PMD_LN_TX_H_PWRDN_FIELD_WIDTH,
    MISC_2_PMD_LN_TX_H_PWRDN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_2_PMD_LN_RX_H_PWRDN
 ******************************************************************************/
const ru_field_rec MISC_2_PMD_LN_RX_H_PWRDN_FIELD =
{
    "PMD_LN_RX_H_PWRDN",
#if RU_INCLUDE_DESC
    "PMD_LN_RX_H_PWRDN",
    "Lane RX power down. Minimum assertion time: 25 comclk period."
    "Reset value is 0x0.",
#endif
    MISC_2_PMD_LN_RX_H_PWRDN_FIELD_MASK,
    0,
    MISC_2_PMD_LN_RX_H_PWRDN_FIELD_WIDTH,
    MISC_2_PMD_LN_RX_H_PWRDN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_2_PMD_TX_DISABLE
 ******************************************************************************/
const ru_field_rec MISC_2_PMD_TX_DISABLE_FIELD =
{
    "PMD_TX_DISABLE",
#if RU_INCLUDE_DESC
    "PMD_TX_DISABLE",
    "Pmd_tx_disable is asserted to squelch the transmit signal for lane."
    "Reset value is 0x0.",
#endif
    MISC_2_PMD_TX_DISABLE_FIELD_MASK,
    0,
    MISC_2_PMD_TX_DISABLE_FIELD_WIDTH,
    MISC_2_PMD_TX_DISABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_2_RESERVED0
 ******************************************************************************/
const ru_field_rec MISC_2_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    MISC_2_RESERVED0_FIELD_MASK,
    0,
    MISC_2_RESERVED0_FIELD_WIDTH,
    MISC_2_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_2_PMD_TX_OSR_MODE
 ******************************************************************************/
const ru_field_rec MISC_2_PMD_TX_OSR_MODE_FIELD =
{
    "PMD_TX_OSR_MODE",
#if RU_INCLUDE_DESC
    "PMD_TX_OSR_MODE",
    "Oversample mode for tx lane. Asynchronous signal to the PMD. 0 -"
    "OSR1. 1 - OSR2."
    "Reset value is 0x0.",
#endif
    MISC_2_PMD_TX_OSR_MODE_FIELD_MASK,
    0,
    MISC_2_PMD_TX_OSR_MODE_FIELD_WIDTH,
    MISC_2_PMD_TX_OSR_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_2_PMD_TX_MODE
 ******************************************************************************/
const ru_field_rec MISC_2_PMD_TX_MODE_FIELD =
{
    "PMD_TX_MODE",
#if RU_INCLUDE_DESC
    "PMD_TX_MODE",
    "EEE tx mode function for lane. Asynchronous signal to the PMD."
    "Reset value is 0x0.",
#endif
    MISC_2_PMD_TX_MODE_FIELD_MASK,
    0,
    MISC_2_PMD_TX_MODE_FIELD_WIDTH,
    MISC_2_PMD_TX_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_2_RESERVED1
 ******************************************************************************/
const ru_field_rec MISC_2_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    MISC_2_RESERVED1_FIELD_MASK,
    0,
    MISC_2_RESERVED1_FIELD_WIDTH,
    MISC_2_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_2_PMD_RX_OSR_MODE
 ******************************************************************************/
const ru_field_rec MISC_2_PMD_RX_OSR_MODE_FIELD =
{
    "PMD_RX_OSR_MODE",
#if RU_INCLUDE_DESC
    "PMD_RX_OSR_MODE",
    "Oversample mode for rx lane. Asynchronous signal to the PMD. 0 -"
    "OSR1. 1 - OSR2."
    "Reset value is 0x0.",
#endif
    MISC_2_PMD_RX_OSR_MODE_FIELD_MASK,
    0,
    MISC_2_PMD_RX_OSR_MODE_FIELD_WIDTH,
    MISC_2_PMD_RX_OSR_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_2_CFGACTIVEETHERNET2P5
 ******************************************************************************/
const ru_field_rec MISC_2_CFGACTIVEETHERNET2P5_FIELD =
{
    "CFGACTIVEETHERNET2P5",
#if RU_INCLUDE_DESC
    "cfgActiveEthernet2p5",
    "0: Selects divide-by-2 clock divider for clkRbc125. 1: Selects"
    "divide-by-1 clock divider for clkRbc125."
    "Reset value is 0x0.",
#endif
    MISC_2_CFGACTIVEETHERNET2P5_FIELD_MASK,
    0,
    MISC_2_CFGACTIVEETHERNET2P5_FIELD_WIDTH,
    MISC_2_CFGACTIVEETHERNET2P5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_2_CFGNGPONTXCLK
 ******************************************************************************/
const ru_field_rec MISC_2_CFGNGPONTXCLK_FIELD =
{
    "CFGNGPONTXCLK",
#if RU_INCLUDE_DESC
    "cfgNgponTxClk",
    "0: Selects divide-by-2 clock divider for mac_tx_clk. 1: Selects"
    "divide-by-4 clock divider for mac_tx_clk. 2: Selects divide-by-1"
    "clock divider. 3: Unused."
    "Reset value is 0x0."
    "",
#endif
    MISC_2_CFGNGPONTXCLK_FIELD_MASK,
    0,
    MISC_2_CFGNGPONTXCLK_FIELD_WIDTH,
    MISC_2_CFGNGPONTXCLK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_2_CFGNGPONRXCLK
 ******************************************************************************/
const ru_field_rec MISC_2_CFGNGPONRXCLK_FIELD =
{
    "CFGNGPONRXCLK",
#if RU_INCLUDE_DESC
    "cfgNgponRxClk",
    "0: Selects divide-by-2 clock divider for mac_rx_clk. 1: Selects"
    "divide-by-4 clock divider for mac_rx_clk. 2: Selects divide-by-1"
    "clock divider. 3: Unused."
    "Reset value is 0x0.",
#endif
    MISC_2_CFGNGPONRXCLK_FIELD_MASK,
    0,
    MISC_2_CFGNGPONRXCLK_FIELD_WIDTH,
    MISC_2_CFGNGPONRXCLK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_2_CFG_APM_MUX_SEL_0
 ******************************************************************************/
const ru_field_rec MISC_2_CFG_APM_MUX_SEL_0_FIELD =
{
    "CFG_APM_MUX_SEL_0",
#if RU_INCLUDE_DESC
    "CFG_APM_MUX_SEL_0",
    "0: Select ncoProgClk for MUX 0 output. 1: Select ncoClk8KHz for MUX"
    "0 output."
    "Reset value is 0x0.",
#endif
    MISC_2_CFG_APM_MUX_SEL_0_FIELD_MASK,
    0,
    MISC_2_CFG_APM_MUX_SEL_0_FIELD_WIDTH,
    MISC_2_CFG_APM_MUX_SEL_0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_2_CFG_APM_MUX_SEL_1
 ******************************************************************************/
const ru_field_rec MISC_2_CFG_APM_MUX_SEL_1_FIELD =
{
    "CFG_APM_MUX_SEL_1",
#if RU_INCLUDE_DESC
    "CFG_APM_MUX_SEL_1",
    "0: Select MUX 0 output for wan_rbc_for_apm. 1: ntr_sync_pulse for"
    "wan_rbc_for_apm."
    "Reset value is 0x0.",
#endif
    MISC_2_CFG_APM_MUX_SEL_1_FIELD_MASK,
    0,
    MISC_2_CFG_APM_MUX_SEL_1_FIELD_WIDTH,
    MISC_2_CFG_APM_MUX_SEL_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_2_RESERVED2
 ******************************************************************************/
const ru_field_rec MISC_2_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    MISC_2_RESERVED2_FIELD_MASK,
    0,
    MISC_2_RESERVED2_FIELD_WIDTH,
    MISC_2_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_3_MEM_REB
 ******************************************************************************/
const ru_field_rec MISC_3_MEM_REB_FIELD =
{
    "MEM_REB",
#if RU_INCLUDE_DESC
    "MEM_REB",
    "REB going to WAN memories"
    "0 - REB asserted"
    "1 - REB de-asserted",
#endif
    MISC_3_MEM_REB_FIELD_MASK,
    0,
    MISC_3_MEM_REB_FIELD_WIDTH,
    MISC_3_MEM_REB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_3_LASER_INVERT
 ******************************************************************************/
const ru_field_rec MISC_3_LASER_INVERT_FIELD =
{
    "LASER_INVERT",
#if RU_INCLUDE_DESC
    "laser_invert",
    "Selects laser mode"
    "0 - Do not invert laser_en"
    "1 - Invert laser_en",
#endif
    MISC_3_LASER_INVERT_FIELD_MASK,
    0,
    MISC_3_LASER_INVERT_FIELD_WIDTH,
    MISC_3_LASER_INVERT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_3_LASER_MODE
 ******************************************************************************/
const ru_field_rec MISC_3_LASER_MODE_FIELD =
{
    "LASER_MODE",
#if RU_INCLUDE_DESC
    "laser_mode",
    "Selects laser mode"
    "0 - EPON laser_en"
    "1 - Reserved"
    "2 - GPON laser_en"
    "3 - Reserved",
#endif
    MISC_3_LASER_MODE_FIELD_MASK,
    0,
    MISC_3_LASER_MODE_FIELD_WIDTH,
    MISC_3_LASER_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_3_RESERVED0
 ******************************************************************************/
const ru_field_rec MISC_3_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    MISC_3_RESERVED0_FIELD_MASK,
    0,
    MISC_3_RESERVED0_FIELD_WIDTH,
    MISC_3_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_3_WAN_INTERFACE_SELECT
 ******************************************************************************/
const ru_field_rec MISC_3_WAN_INTERFACE_SELECT_FIELD =
{
    "WAN_INTERFACE_SELECT",
#if RU_INCLUDE_DESC
    "wan_interface_select",
    "Selects which WAN interface will be used."
    "0 - EPON"
    "1 - GPON"
    "",
#endif
    MISC_3_WAN_INTERFACE_SELECT_FIELD_MASK,
    0,
    MISC_3_WAN_INTERFACE_SELECT_FIELD_WIDTH,
    MISC_3_WAN_INTERFACE_SELECT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_3_LASER_OE
 ******************************************************************************/
const ru_field_rec MISC_3_LASER_OE_FIELD =
{
    "LASER_OE",
#if RU_INCLUDE_DESC
    "laser_oe",
    "Laser output enable"
    "0 - Output not enabled"
    "1 - Output enabled",
#endif
    MISC_3_LASER_OE_FIELD_MASK,
    0,
    MISC_3_LASER_OE_FIELD_WIDTH,
    MISC_3_LASER_OE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_3_NTR_SYNC_PERIOD_SEL
 ******************************************************************************/
const ru_field_rec MISC_3_NTR_SYNC_PERIOD_SEL_FIELD =
{
    "NTR_SYNC_PERIOD_SEL",
#if RU_INCLUDE_DESC
    "ntr_sync_period_sel",
    "Selects APM clock generation division",
#endif
    MISC_3_NTR_SYNC_PERIOD_SEL_FIELD_MASK,
    0,
    MISC_3_NTR_SYNC_PERIOD_SEL_FIELD_WIDTH,
    MISC_3_NTR_SYNC_PERIOD_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_3_WAN_DEBUG_SEL
 ******************************************************************************/
const ru_field_rec MISC_3_WAN_DEBUG_SEL_FIELD =
{
    "WAN_DEBUG_SEL",
#if RU_INCLUDE_DESC
    "WAN_DEBUG_SEL",
    "Selects WAN debug bus output",
#endif
    MISC_3_WAN_DEBUG_SEL_FIELD_MASK,
    0,
    MISC_3_WAN_DEBUG_SEL_FIELD_WIDTH,
    MISC_3_WAN_DEBUG_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_3_EPON_DEBUG_SEL
 ******************************************************************************/
const ru_field_rec MISC_3_EPON_DEBUG_SEL_FIELD =
{
    "EPON_DEBUG_SEL",
#if RU_INCLUDE_DESC
    "EPON_DEBUG_SEL",
    "Selects EPON debug bus output",
#endif
    MISC_3_EPON_DEBUG_SEL_FIELD_MASK,
    0,
    MISC_3_EPON_DEBUG_SEL_FIELD_WIDTH,
    MISC_3_EPON_DEBUG_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_3_EPON_TX_FIFO_OFF
 ******************************************************************************/
const ru_field_rec MISC_3_EPON_TX_FIFO_OFF_FIELD =
{
    "EPON_TX_FIFO_OFF",
#if RU_INCLUDE_DESC
    "EPON_TX_FIFO_OFF",
    "TXFIFO OFF LOAD signal for EPONs gearbox."
    "Reset value is 0x0.",
#endif
    MISC_3_EPON_TX_FIFO_OFF_FIELD_MASK,
    0,
    MISC_3_EPON_TX_FIFO_OFF_FIELD_WIDTH,
    MISC_3_EPON_TX_FIFO_OFF_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_3_RESERVED1
 ******************************************************************************/
const ru_field_rec MISC_3_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    MISC_3_RESERVED1_FIELD_MASK,
    0,
    MISC_3_RESERVED1_FIELD_WIDTH,
    MISC_3_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: MISC_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *MISC_0_FIELDS[] =
{
    &MISC_0_ONU2G_PMD_STATUS_SEL_FIELD,
    &MISC_0_REFIN_EN_FIELD,
    &MISC_0_REFOUT_EN_FIELD,
    &MISC_0_MDIO_MODE_FIELD,
    &MISC_0_MDIO_FAST_MODE_FIELD,
    &MISC_0_EPON_TX_FIFO_OFF_LD_FIELD,
    &MISC_0_ONU2G_PHYA_FIELD,
    &MISC_0_EPON_GBOX_RX_WIDTH_MODE_FIELD,
    &MISC_0_EPON_AE_2P5_FULL_RATE_MODE_FIELD,
    &MISC_0_RESERVED0_FIELD,
    &MISC_0_PMD_LANE_MODE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec MISC_0_REG = 
{
    "0",
#if RU_INCLUDE_DESC
    "WAN_TOP_MISC_0 Register",
    "Register used for wan_top configuration.",
#endif
    MISC_0_REG_OFFSET,
    0,
    0,
    5,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    11,
    MISC_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: MISC_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *MISC_1_FIELDS[] =
{
    &MISC_1_PMD_CORE_0_MODE_FIELD,
    &MISC_1_PMD_CORE_1_MODE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec MISC_1_REG = 
{
    "1",
#if RU_INCLUDE_DESC
    "WAN_TOP_MISC_1 Register",
    "Register used for wan_top configuration.",
#endif
    MISC_1_REG_OFFSET,
    0,
    0,
    6,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    MISC_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: MISC_2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *MISC_2_FIELDS[] =
{
    &MISC_2_PMD_RX_MODE_FIELD,
    &MISC_2_PMD_LN_DP_H_RSTB_FIELD,
    &MISC_2_PMD_LN_H_RSTB_FIELD,
    &MISC_2_PMD_CORE_0_DP_H_RSTB_FIELD,
    &MISC_2_PMD_CORE_1_DP_H_RSTB_FIELD,
    &MISC_2_PMD_POR_H_RSTB_FIELD,
    &MISC_2_PMD_EXT_LOS_FIELD,
    &MISC_2_PMD_LN_TX_H_PWRDN_FIELD,
    &MISC_2_PMD_LN_RX_H_PWRDN_FIELD,
    &MISC_2_PMD_TX_DISABLE_FIELD,
    &MISC_2_RESERVED0_FIELD,
    &MISC_2_PMD_TX_OSR_MODE_FIELD,
    &MISC_2_PMD_TX_MODE_FIELD,
    &MISC_2_RESERVED1_FIELD,
    &MISC_2_PMD_RX_OSR_MODE_FIELD,
    &MISC_2_CFGACTIVEETHERNET2P5_FIELD,
    &MISC_2_CFGNGPONTXCLK_FIELD,
    &MISC_2_CFGNGPONRXCLK_FIELD,
    &MISC_2_CFG_APM_MUX_SEL_0_FIELD,
    &MISC_2_CFG_APM_MUX_SEL_1_FIELD,
    &MISC_2_RESERVED2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec MISC_2_REG = 
{
    "2",
#if RU_INCLUDE_DESC
    "WAN_TOP_MISC_2 Register",
    "Register used for wan_top configuration.",
#endif
    MISC_2_REG_OFFSET,
    0,
    0,
    7,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    21,
    MISC_2_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: MISC_3
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *MISC_3_FIELDS[] =
{
    &MISC_3_MEM_REB_FIELD,
    &MISC_3_LASER_INVERT_FIELD,
    &MISC_3_LASER_MODE_FIELD,
    &MISC_3_RESERVED0_FIELD,
    &MISC_3_WAN_INTERFACE_SELECT_FIELD,
    &MISC_3_LASER_OE_FIELD,
    &MISC_3_NTR_SYNC_PERIOD_SEL_FIELD,
    &MISC_3_WAN_DEBUG_SEL_FIELD,
    &MISC_3_EPON_DEBUG_SEL_FIELD,
    &MISC_3_EPON_TX_FIFO_OFF_FIELD,
    &MISC_3_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec MISC_3_REG = 
{
    "3",
#if RU_INCLUDE_DESC
    "WAN_TOP_MISC_3 Register",
    "Register used for wan_top configuration.",
#endif
    MISC_3_REG_OFFSET,
    0,
    0,
    8,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    11,
    MISC_3_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: MISC
 ******************************************************************************/
static const ru_reg_rec *MISC_REGS[] =
{
    &MISC_0_REG,
    &MISC_1_REG,
    &MISC_2_REG,
    &MISC_3_REG,
};

unsigned long MISC_ADDRS[] =
{
    0x82db2040,
};

const ru_block_rec MISC_BLOCK = 
{
    "MISC",
    MISC_ADDRS,
    1,
    4,
    MISC_REGS
};

/* End of file MISC.c */
