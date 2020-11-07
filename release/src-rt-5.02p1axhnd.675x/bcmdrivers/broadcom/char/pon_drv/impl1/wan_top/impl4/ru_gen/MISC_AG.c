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
 * Field: MISC_0_CR_XGWAN_TOP_WAN_MISC_PMD_LANE_MODE
 ******************************************************************************/
const ru_field_rec MISC_0_CR_XGWAN_TOP_WAN_MISC_PMD_LANE_MODE_FIELD =
{
    "CR_XGWAN_TOP_WAN_MISC_PMD_LANE_MODE",
#if RU_INCLUDE_DESC
    "",
    "Reserved mode bus for lane. Mode bus for lane used by the PCS to"
    "communicate lane info to PMD. This bus should only be written to"
    "when the lane is in reset since the firmware will only read this"
    "after coming out of reset.  This signal will be latched to a lane"
    "based register during core_dp_rstb. Asynchronous signal to the PMD",
#endif
    MISC_0_CR_XGWAN_TOP_WAN_MISC_PMD_LANE_MODE_FIELD_MASK,
    0,
    MISC_0_CR_XGWAN_TOP_WAN_MISC_PMD_LANE_MODE_FIELD_WIDTH,
    MISC_0_CR_XGWAN_TOP_WAN_MISC_PMD_LANE_MODE_FIELD_SHIFT,
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
 * Field: MISC_0_CR_XGWAN_TOP_WAN_MISC_EPON_GBOX_AE_2P5_FULL_RATE_MODE
 ******************************************************************************/
const ru_field_rec MISC_0_CR_XGWAN_TOP_WAN_MISC_EPON_GBOX_AE_2P5_FULL_RATE_MODE_FIELD =
{
    "CR_XGWAN_TOP_WAN_MISC_EPON_GBOX_AE_2P5_FULL_RATE_MODE",
#if RU_INCLUDE_DESC
    "",
    "0: All other modes 1: Explicit 2.5G Full Rate Serdes Mode",
#endif
    MISC_0_CR_XGWAN_TOP_WAN_MISC_EPON_GBOX_AE_2P5_FULL_RATE_MODE_FIELD_MASK,
    0,
    MISC_0_CR_XGWAN_TOP_WAN_MISC_EPON_GBOX_AE_2P5_FULL_RATE_MODE_FIELD_WIDTH,
    MISC_0_CR_XGWAN_TOP_WAN_MISC_EPON_GBOX_AE_2P5_FULL_RATE_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_0_CR_XGWAN_TOP_WAN_MISC_EPON_GBOX_PON_RX_WIDTH_MODE
 ******************************************************************************/
const ru_field_rec MISC_0_CR_XGWAN_TOP_WAN_MISC_EPON_GBOX_PON_RX_WIDTH_MODE_FIELD =
{
    "CR_XGWAN_TOP_WAN_MISC_EPON_GBOX_PON_RX_WIDTH_MODE",
#if RU_INCLUDE_DESC
    "",
    "0: 10b mode, 1G Operation 1: 20b mode, 2G Operation",
#endif
    MISC_0_CR_XGWAN_TOP_WAN_MISC_EPON_GBOX_PON_RX_WIDTH_MODE_FIELD_MASK,
    0,
    MISC_0_CR_XGWAN_TOP_WAN_MISC_EPON_GBOX_PON_RX_WIDTH_MODE_FIELD_WIDTH,
    MISC_0_CR_XGWAN_TOP_WAN_MISC_EPON_GBOX_PON_RX_WIDTH_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_0_CR_XGWAN_TOP_WAN_MISC_ONU2G_PHYA
 ******************************************************************************/
const ru_field_rec MISC_0_CR_XGWAN_TOP_WAN_MISC_ONU2G_PHYA_FIELD =
{
    "CR_XGWAN_TOP_WAN_MISC_ONU2G_PHYA",
#if RU_INCLUDE_DESC
    "",
    "Strap input for selecting the port address to decode on the mdio"
    "transaction.",
#endif
    MISC_0_CR_XGWAN_TOP_WAN_MISC_ONU2G_PHYA_FIELD_MASK,
    0,
    MISC_0_CR_XGWAN_TOP_WAN_MISC_ONU2G_PHYA_FIELD_WIDTH,
    MISC_0_CR_XGWAN_TOP_WAN_MISC_ONU2G_PHYA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_0_CR_XGWAN_TOP_WAN_MISC_EPON_TX_FIFO_OFF_LD
 ******************************************************************************/
const ru_field_rec MISC_0_CR_XGWAN_TOP_WAN_MISC_EPON_TX_FIFO_OFF_LD_FIELD =
{
    "CR_XGWAN_TOP_WAN_MISC_EPON_TX_FIFO_OFF_LD",
#if RU_INCLUDE_DESC
    "",
    "Load vlue in cr_xgwan_top_wan_misc_epon_tx_fifo_off.",
#endif
    MISC_0_CR_XGWAN_TOP_WAN_MISC_EPON_TX_FIFO_OFF_LD_FIELD_MASK,
    0,
    MISC_0_CR_XGWAN_TOP_WAN_MISC_EPON_TX_FIFO_OFF_LD_FIELD_WIDTH,
    MISC_0_CR_XGWAN_TOP_WAN_MISC_EPON_TX_FIFO_OFF_LD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_0_CR_XGWAN_TOP_WAN_MISC_MDIO_FAST_MODE
 ******************************************************************************/
const ru_field_rec MISC_0_CR_XGWAN_TOP_WAN_MISC_MDIO_FAST_MODE_FIELD =
{
    "CR_XGWAN_TOP_WAN_MISC_MDIO_FAST_MODE",
#if RU_INCLUDE_DESC
    "",
    "Debug bit that disables the 32-bit preamble to allow mdio frames to"
    "run at 2x speed.",
#endif
    MISC_0_CR_XGWAN_TOP_WAN_MISC_MDIO_FAST_MODE_FIELD_MASK,
    0,
    MISC_0_CR_XGWAN_TOP_WAN_MISC_MDIO_FAST_MODE_FIELD_WIDTH,
    MISC_0_CR_XGWAN_TOP_WAN_MISC_MDIO_FAST_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_0_CR_XGWAN_TOP_WAN_MISC_MDIO_MODE
 ******************************************************************************/
const ru_field_rec MISC_0_CR_XGWAN_TOP_WAN_MISC_MDIO_MODE_FIELD =
{
    "CR_XGWAN_TOP_WAN_MISC_MDIO_MODE",
#if RU_INCLUDE_DESC
    "",
    "MDIO transaction indicator needs to be asserted in a configuration"
    "where an external mdio controller is trying to access the internal"
    "PMD registers directly.",
#endif
    MISC_0_CR_XGWAN_TOP_WAN_MISC_MDIO_MODE_FIELD_MASK,
    0,
    MISC_0_CR_XGWAN_TOP_WAN_MISC_MDIO_MODE_FIELD_WIDTH,
    MISC_0_CR_XGWAN_TOP_WAN_MISC_MDIO_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_0_CR_XGWAN_TOP_WAN_MISC_REFOUT_EN
 ******************************************************************************/
const ru_field_rec MISC_0_CR_XGWAN_TOP_WAN_MISC_REFOUT_EN_FIELD =
{
    "CR_XGWAN_TOP_WAN_MISC_REFOUT_EN",
#if RU_INCLUDE_DESC
    "",
    "Not used in this chip.",
#endif
    MISC_0_CR_XGWAN_TOP_WAN_MISC_REFOUT_EN_FIELD_MASK,
    0,
    MISC_0_CR_XGWAN_TOP_WAN_MISC_REFOUT_EN_FIELD_WIDTH,
    MISC_0_CR_XGWAN_TOP_WAN_MISC_REFOUT_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_0_CR_XGWAN_TOP_WAN_MISC_REFIN_EN
 ******************************************************************************/
const ru_field_rec MISC_0_CR_XGWAN_TOP_WAN_MISC_REFIN_EN_FIELD =
{
    "CR_XGWAN_TOP_WAN_MISC_REFIN_EN",
#if RU_INCLUDE_DESC
    "",
    "Not used in this chip.",
#endif
    MISC_0_CR_XGWAN_TOP_WAN_MISC_REFIN_EN_FIELD_MASK,
    0,
    MISC_0_CR_XGWAN_TOP_WAN_MISC_REFIN_EN_FIELD_WIDTH,
    MISC_0_CR_XGWAN_TOP_WAN_MISC_REFIN_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_0_CR_XGWAN_TOP_WAN_MISC_ONU2G_PMD_STATUS_SEL
 ******************************************************************************/
const ru_field_rec MISC_0_CR_XGWAN_TOP_WAN_MISC_ONU2G_PMD_STATUS_SEL_FIELD =
{
    "CR_XGWAN_TOP_WAN_MISC_ONU2G_PMD_STATUS_SEL",
#if RU_INCLUDE_DESC
    "",
    "Not used in this chip.",
#endif
    MISC_0_CR_XGWAN_TOP_WAN_MISC_ONU2G_PMD_STATUS_SEL_FIELD_MASK,
    0,
    MISC_0_CR_XGWAN_TOP_WAN_MISC_ONU2G_PMD_STATUS_SEL_FIELD_WIDTH,
    MISC_0_CR_XGWAN_TOP_WAN_MISC_ONU2G_PMD_STATUS_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_1_CR_XGWAN_TOP_WAN_MISC_PMD_CORE_1_MODE
 ******************************************************************************/
const ru_field_rec MISC_1_CR_XGWAN_TOP_WAN_MISC_PMD_CORE_1_MODE_FIELD =
{
    "CR_XGWAN_TOP_WAN_MISC_PMD_CORE_1_MODE",
#if RU_INCLUDE_DESC
    "",
    "Reserved mode bus for the entire core.  Mode bus for core used by"
    "the PCS to communicate core info to PMD . This bus should only be"
    "written to when the core is in reset since the firmware will only"
    "read this after coming out of reset.  This signal will be latched to"
    "a core register during core_dp_rstb. Asynchronous signal to the PMD",
#endif
    MISC_1_CR_XGWAN_TOP_WAN_MISC_PMD_CORE_1_MODE_FIELD_MASK,
    0,
    MISC_1_CR_XGWAN_TOP_WAN_MISC_PMD_CORE_1_MODE_FIELD_WIDTH,
    MISC_1_CR_XGWAN_TOP_WAN_MISC_PMD_CORE_1_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_1_CR_XGWAN_TOP_WAN_MISC_PMD_CORE_0_MODE
 ******************************************************************************/
const ru_field_rec MISC_1_CR_XGWAN_TOP_WAN_MISC_PMD_CORE_0_MODE_FIELD =
{
    "CR_XGWAN_TOP_WAN_MISC_PMD_CORE_0_MODE",
#if RU_INCLUDE_DESC
    "",
    "Reserved mode bus for the entire core.  Mode bus for core used by"
    "the PCS to communicate core info to PMD . This bus should only be"
    "written to when the core is in reset since the firmware will only"
    "read this after coming out of reset.  This signal will be latched to"
    "a core register during core_dp_rstb. Asynchronous signal to the PMD",
#endif
    MISC_1_CR_XGWAN_TOP_WAN_MISC_PMD_CORE_0_MODE_FIELD_MASK,
    0,
    MISC_1_CR_XGWAN_TOP_WAN_MISC_PMD_CORE_0_MODE_FIELD_WIDTH,
    MISC_1_CR_XGWAN_TOP_WAN_MISC_PMD_CORE_0_MODE_FIELD_SHIFT,
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
 * Field: MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_RX_OSR_MODE
 ******************************************************************************/
const ru_field_rec MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_RX_OSR_MODE_FIELD =
{
    "CR_XGWAN_TOP_WAN_MISC_PMD_RX_OSR_MODE",
#if RU_INCLUDE_DESC
    "",
    "Oversample mode for rx lane. Asynchronous signal to the PMD. 0 -"
    "OSR1. 1 - OSR2.",
#endif
    MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_RX_OSR_MODE_FIELD_MASK,
    0,
    MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_RX_OSR_MODE_FIELD_WIDTH,
    MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_RX_OSR_MODE_FIELD_SHIFT,
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
 * Field: MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_TX_MODE
 ******************************************************************************/
const ru_field_rec MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_TX_MODE_FIELD =
{
    "CR_XGWAN_TOP_WAN_MISC_PMD_TX_MODE",
#if RU_INCLUDE_DESC
    "",
    "EEE tx mode function for lane. Asynchronous signal to the PMD.",
#endif
    MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_TX_MODE_FIELD_MASK,
    0,
    MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_TX_MODE_FIELD_WIDTH,
    MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_TX_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_TX_OSR_MODE
 ******************************************************************************/
const ru_field_rec MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_TX_OSR_MODE_FIELD =
{
    "CR_XGWAN_TOP_WAN_MISC_PMD_TX_OSR_MODE",
#if RU_INCLUDE_DESC
    "",
    "Oversample mode for tx lane. Asynchronous signal to the PMD. 0 -"
    "OSR1.  1 - OSR2.",
#endif
    MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_TX_OSR_MODE_FIELD_MASK,
    0,
    MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_TX_OSR_MODE_FIELD_WIDTH,
    MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_TX_OSR_MODE_FIELD_SHIFT,
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
 * Field: MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_TX_DISABLE
 ******************************************************************************/
const ru_field_rec MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_TX_DISABLE_FIELD =
{
    "CR_XGWAN_TOP_WAN_MISC_PMD_TX_DISABLE",
#if RU_INCLUDE_DESC
    "",
    "Pmd_tx_disable is asserted to squelch the transmit signal for lane.",
#endif
    MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_TX_DISABLE_FIELD_MASK,
    0,
    MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_TX_DISABLE_FIELD_WIDTH,
    MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_TX_DISABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_LN_RX_H_PWRDN
 ******************************************************************************/
const ru_field_rec MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_LN_RX_H_PWRDN_FIELD =
{
    "CR_XGWAN_TOP_WAN_MISC_PMD_LN_RX_H_PWRDN",
#if RU_INCLUDE_DESC
    "",
    "Lane RX power down. Minimum assertion time: 25 comclk period.",
#endif
    MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_LN_RX_H_PWRDN_FIELD_MASK,
    0,
    MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_LN_RX_H_PWRDN_FIELD_WIDTH,
    MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_LN_RX_H_PWRDN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_LN_TX_H_PWRDN
 ******************************************************************************/
const ru_field_rec MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_LN_TX_H_PWRDN_FIELD =
{
    "CR_XGWAN_TOP_WAN_MISC_PMD_LN_TX_H_PWRDN",
#if RU_INCLUDE_DESC
    "",
    "Lane TX power down. Minimum assertion time: 25 comclk period.",
#endif
    MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_LN_TX_H_PWRDN_FIELD_MASK,
    0,
    MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_LN_TX_H_PWRDN_FIELD_WIDTH,
    MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_LN_TX_H_PWRDN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_EXT_LOS
 ******************************************************************************/
const ru_field_rec MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_EXT_LOS_FIELD =
{
    "CR_XGWAN_TOP_WAN_MISC_PMD_EXT_LOS",
#if RU_INCLUDE_DESC
    "",
    "External Loss of signal. LOS = 1. Signal presence = 0.",
#endif
    MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_EXT_LOS_FIELD_MASK,
    0,
    MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_EXT_LOS_FIELD_WIDTH,
    MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_EXT_LOS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_POR_H_RSTB
 ******************************************************************************/
const ru_field_rec MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_POR_H_RSTB_FIELD =
{
    "CR_XGWAN_TOP_WAN_MISC_PMD_POR_H_RSTB",
#if RU_INCLUDE_DESC
    "",
    "PMD main reset, resets registers, data path for entire core"
    "including all lanes. Active Low. Minimum assertion time: 25 comclk"
    "period.",
#endif
    MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_POR_H_RSTB_FIELD_MASK,
    0,
    MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_POR_H_RSTB_FIELD_WIDTH,
    MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_POR_H_RSTB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_CORE_1_DP_H_RSTB
 ******************************************************************************/
const ru_field_rec MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_CORE_1_DP_H_RSTB_FIELD =
{
    "CR_XGWAN_TOP_WAN_MISC_PMD_CORE_1_DP_H_RSTB",
#if RU_INCLUDE_DESC
    "",
    "Core reset for datapath for all lanes and corresponding PLL. Does"
    "not reset registers. Active Low. Minimum assertion time: 25 comclk"
    "period.",
#endif
    MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_CORE_1_DP_H_RSTB_FIELD_MASK,
    0,
    MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_CORE_1_DP_H_RSTB_FIELD_WIDTH,
    MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_CORE_1_DP_H_RSTB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_CORE_0_DP_H_RSTB
 ******************************************************************************/
const ru_field_rec MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_CORE_0_DP_H_RSTB_FIELD =
{
    "CR_XGWAN_TOP_WAN_MISC_PMD_CORE_0_DP_H_RSTB",
#if RU_INCLUDE_DESC
    "",
    "Core reset for datapath for all lanes and corresponding PLL. Does"
    "not reset registers. Active Low. Minimum assertion time: 25 comclk"
    "period.",
#endif
    MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_CORE_0_DP_H_RSTB_FIELD_MASK,
    0,
    MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_CORE_0_DP_H_RSTB_FIELD_WIDTH,
    MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_CORE_0_DP_H_RSTB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_LN_H_RSTB
 ******************************************************************************/
const ru_field_rec MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_LN_H_RSTB_FIELD =
{
    "CR_XGWAN_TOP_WAN_MISC_PMD_LN_H_RSTB",
#if RU_INCLUDE_DESC
    "",
    "Lane reset registers and data path. Active Low. Minimum assertion"
    "time: 25 comclk period.",
#endif
    MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_LN_H_RSTB_FIELD_MASK,
    0,
    MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_LN_H_RSTB_FIELD_WIDTH,
    MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_LN_H_RSTB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_LN_DP_H_RSTB
 ******************************************************************************/
const ru_field_rec MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_LN_DP_H_RSTB_FIELD =
{
    "CR_XGWAN_TOP_WAN_MISC_PMD_LN_DP_H_RSTB",
#if RU_INCLUDE_DESC
    "",
    "Lane datapath reset, does not reset registers. Active Low. Minimum"
    "assertion time: 25 comclk period.",
#endif
    MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_LN_DP_H_RSTB_FIELD_MASK,
    0,
    MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_LN_DP_H_RSTB_FIELD_WIDTH,
    MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_LN_DP_H_RSTB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_RX_MODE
 ******************************************************************************/
const ru_field_rec MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_RX_MODE_FIELD =
{
    "CR_XGWAN_TOP_WAN_MISC_PMD_RX_MODE",
#if RU_INCLUDE_DESC
    "",
    "EEE rx mode function for lane. Asynchronous signal to the PMD",
#endif
    MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_RX_MODE_FIELD_MASK,
    0,
    MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_RX_MODE_FIELD_WIDTH,
    MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_RX_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_4_CFG_NTR_PULSE_WIDTH
 ******************************************************************************/
const ru_field_rec MISC_4_CFG_NTR_PULSE_WIDTH_FIELD =
{
    "CFG_NTR_PULSE_WIDTH",
#if RU_INCLUDE_DESC
    "",
    "Specifies the width of NTR pulse, in unit of 4 ns.",
#endif
    MISC_4_CFG_NTR_PULSE_WIDTH_FIELD_MASK,
    0,
    MISC_4_CFG_NTR_PULSE_WIDTH_FIELD_WIDTH,
    MISC_4_CFG_NTR_PULSE_WIDTH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_3_CR_XGWAN_TOP_WAN_MISC_WAN_CFG_EPON_TX_FIFO_OFF
 ******************************************************************************/
const ru_field_rec MISC_3_CR_XGWAN_TOP_WAN_MISC_WAN_CFG_EPON_TX_FIFO_OFF_FIELD =
{
    "CR_XGWAN_TOP_WAN_MISC_WAN_CFG_EPON_TX_FIFO_OFF",
#if RU_INCLUDE_DESC
    "",
    "TXFIFO OFF LOAD signal for EPON's gearbox.",
#endif
    MISC_3_CR_XGWAN_TOP_WAN_MISC_WAN_CFG_EPON_TX_FIFO_OFF_FIELD_MASK,
    0,
    MISC_3_CR_XGWAN_TOP_WAN_MISC_WAN_CFG_EPON_TX_FIFO_OFF_FIELD_WIDTH,
    MISC_3_CR_XGWAN_TOP_WAN_MISC_WAN_CFG_EPON_TX_FIFO_OFF_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_3_CR_XGWAN_TOP_WAN_MISC_WAN_CFG_WAN_DEBUG_SEL
 ******************************************************************************/
const ru_field_rec MISC_3_CR_XGWAN_TOP_WAN_MISC_WAN_CFG_WAN_DEBUG_SEL_FIELD =
{
    "CR_XGWAN_TOP_WAN_MISC_WAN_CFG_WAN_DEBUG_SEL",
#if RU_INCLUDE_DESC
    "",
    "This field selects the block that the 40-bit debug bus comes"
    "from."
    "0: GPON"
    "1: XPORT_0 BBH RX"
    "2: XPORT_0 BBH TX"
    "3: XPORT_1 BBH RX"
    "4: XPORT_1 BBH TX"
    "5: SERDES"
    "6: XLIF"
    "7: GPON BBH RX"
    "8: GPON BBH TX"
    "9: SAR BBH RX"
    "10: SAR BBH TX"
    "11: XPORT"
    "12: SAR",
#endif
    MISC_3_CR_XGWAN_TOP_WAN_MISC_WAN_CFG_WAN_DEBUG_SEL_FIELD_MASK,
    0,
    MISC_3_CR_XGWAN_TOP_WAN_MISC_WAN_CFG_WAN_DEBUG_SEL_FIELD_WIDTH,
    MISC_3_CR_XGWAN_TOP_WAN_MISC_WAN_CFG_WAN_DEBUG_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_3_CFG_NTR_PERIPH_PULSE_BYPASS
 ******************************************************************************/
const ru_field_rec MISC_3_CFG_NTR_PERIPH_PULSE_BYPASS_FIELD =
{
    "CFG_NTR_PERIPH_PULSE_BYPASS",
#if RU_INCLUDE_DESC
    "",
    "Allows the bypassing of the NTR double sync and pulse generator.",
#endif
    MISC_3_CFG_NTR_PERIPH_PULSE_BYPASS_FIELD_MASK,
    0,
    MISC_3_CFG_NTR_PERIPH_PULSE_BYPASS_FIELD_WIDTH,
    MISC_3_CFG_NTR_PERIPH_PULSE_BYPASS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_3_CFG_NTR_GPIO_PULSE_BYPASS
 ******************************************************************************/
const ru_field_rec MISC_3_CFG_NTR_GPIO_PULSE_BYPASS_FIELD =
{
    "CFG_NTR_GPIO_PULSE_BYPASS",
#if RU_INCLUDE_DESC
    "",
    "Allows the bypassing of the NTR double sync and pulse generator.",
#endif
    MISC_3_CFG_NTR_GPIO_PULSE_BYPASS_FIELD_MASK,
    0,
    MISC_3_CFG_NTR_GPIO_PULSE_BYPASS_FIELD_WIDTH,
    MISC_3_CFG_NTR_GPIO_PULSE_BYPASS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_3_CFG_NTR_SRC
 ******************************************************************************/
const ru_field_rec MISC_3_CFG_NTR_SRC_FIELD =
{
    "CFG_NTR_SRC",
#if RU_INCLUDE_DESC
    "",
    "Selects the source of NTR pulse : 0 - programmable clock; 1 - GPON;"
    "2 - GPIO, 3 - NCO.",
#endif
    MISC_3_CFG_NTR_SRC_FIELD_MASK,
    0,
    MISC_3_CFG_NTR_SRC_FIELD_WIDTH,
    MISC_3_CFG_NTR_SRC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_3_CR_XGWAN_TOP_WAN_MISC_WAN_CFG_LASER_OE
 ******************************************************************************/
const ru_field_rec MISC_3_CR_XGWAN_TOP_WAN_MISC_WAN_CFG_LASER_OE_FIELD =
{
    "CR_XGWAN_TOP_WAN_MISC_WAN_CFG_LASER_OE",
#if RU_INCLUDE_DESC
    "",
    "0: Output enable for laser is disabled.  1: Output enable for laser"
    "is enabled.",
#endif
    MISC_3_CR_XGWAN_TOP_WAN_MISC_WAN_CFG_LASER_OE_FIELD_MASK,
    0,
    MISC_3_CR_XGWAN_TOP_WAN_MISC_WAN_CFG_LASER_OE_FIELD_WIDTH,
    MISC_3_CR_XGWAN_TOP_WAN_MISC_WAN_CFG_LASER_OE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_3_CR_XGWAN_TOP_WAN_MISC_WAN_CFG_WAN_INTERFACE_SELECT
 ******************************************************************************/
const ru_field_rec MISC_3_CR_XGWAN_TOP_WAN_MISC_WAN_CFG_WAN_INTERFACE_SELECT_FIELD =
{
    "CR_XGWAN_TOP_WAN_MISC_WAN_CFG_WAN_INTERFACE_SELECT",
#if RU_INCLUDE_DESC
    "",
    "Selects between various speed AE modes and GPON mode   0: 100M AE;"
    "1: 1G AE; 2. 2.5G AE; 3: 10G AE; 4: GPON; 5-7: Reserve.",
#endif
    MISC_3_CR_XGWAN_TOP_WAN_MISC_WAN_CFG_WAN_INTERFACE_SELECT_FIELD_MASK,
    0,
    MISC_3_CR_XGWAN_TOP_WAN_MISC_WAN_CFG_WAN_INTERFACE_SELECT_FIELD_WIDTH,
    MISC_3_CR_XGWAN_TOP_WAN_MISC_WAN_CFG_WAN_INTERFACE_SELECT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_3_CR_XGWAN_TOP_WAN_MISC_WAN_CFG_LASER_MODE
 ******************************************************************************/
const ru_field_rec MISC_3_CR_XGWAN_TOP_WAN_MISC_WAN_CFG_LASER_MODE_FIELD =
{
    "CR_XGWAN_TOP_WAN_MISC_WAN_CFG_LASER_MODE",
#if RU_INCLUDE_DESC
    "",
    "Bit 0 selects the speed, and bit 1 selects the technology.  2: GPON;"
    "0,1,3 : Disable laser.",
#endif
    MISC_3_CR_XGWAN_TOP_WAN_MISC_WAN_CFG_LASER_MODE_FIELD_MASK,
    0,
    MISC_3_CR_XGWAN_TOP_WAN_MISC_WAN_CFG_LASER_MODE_FIELD_WIDTH,
    MISC_3_CR_XGWAN_TOP_WAN_MISC_WAN_CFG_LASER_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_3_CR_XGWAN_TOP_WAN_MISC_WAN_CFG_LASER_INVERT
 ******************************************************************************/
const ru_field_rec MISC_3_CR_XGWAN_TOP_WAN_MISC_WAN_CFG_LASER_INVERT_FIELD =
{
    "CR_XGWAN_TOP_WAN_MISC_WAN_CFG_LASER_INVERT",
#if RU_INCLUDE_DESC
    "",
    "0: Normal operation.  1: Invert laser enable.",
#endif
    MISC_3_CR_XGWAN_TOP_WAN_MISC_WAN_CFG_LASER_INVERT_FIELD_MASK,
    0,
    MISC_3_CR_XGWAN_TOP_WAN_MISC_WAN_CFG_LASER_INVERT_FIELD_WIDTH,
    MISC_3_CR_XGWAN_TOP_WAN_MISC_WAN_CFG_LASER_INVERT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: MISC_3_CR_XGWAN_TOP_WAN_MISC_WAN_CFG_MEM_REB
 ******************************************************************************/
const ru_field_rec MISC_3_CR_XGWAN_TOP_WAN_MISC_WAN_CFG_MEM_REB_FIELD =
{
    "CR_XGWAN_TOP_WAN_MISC_WAN_CFG_MEM_REB",
#if RU_INCLUDE_DESC
    "",
    "REB going to WAN memories.",
#endif
    MISC_3_CR_XGWAN_TOP_WAN_MISC_WAN_CFG_MEM_REB_FIELD_MASK,
    0,
    MISC_3_CR_XGWAN_TOP_WAN_MISC_WAN_CFG_MEM_REB_FIELD_WIDTH,
    MISC_3_CR_XGWAN_TOP_WAN_MISC_WAN_CFG_MEM_REB_FIELD_SHIFT,
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
    &MISC_0_CR_XGWAN_TOP_WAN_MISC_PMD_LANE_MODE_FIELD,
    &MISC_0_RESERVED0_FIELD,
    &MISC_0_CR_XGWAN_TOP_WAN_MISC_EPON_GBOX_AE_2P5_FULL_RATE_MODE_FIELD,
    &MISC_0_CR_XGWAN_TOP_WAN_MISC_EPON_GBOX_PON_RX_WIDTH_MODE_FIELD,
    &MISC_0_CR_XGWAN_TOP_WAN_MISC_ONU2G_PHYA_FIELD,
    &MISC_0_CR_XGWAN_TOP_WAN_MISC_EPON_TX_FIFO_OFF_LD_FIELD,
    &MISC_0_CR_XGWAN_TOP_WAN_MISC_MDIO_FAST_MODE_FIELD,
    &MISC_0_CR_XGWAN_TOP_WAN_MISC_MDIO_MODE_FIELD,
    &MISC_0_CR_XGWAN_TOP_WAN_MISC_REFOUT_EN_FIELD,
    &MISC_0_CR_XGWAN_TOP_WAN_MISC_REFIN_EN_FIELD,
    &MISC_0_CR_XGWAN_TOP_WAN_MISC_ONU2G_PMD_STATUS_SEL_FIELD,
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
    10,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
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
    &MISC_1_CR_XGWAN_TOP_WAN_MISC_PMD_CORE_1_MODE_FIELD,
    &MISC_1_CR_XGWAN_TOP_WAN_MISC_PMD_CORE_0_MODE_FIELD,
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
    11,
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
    &MISC_2_RESERVED0_FIELD,
    &MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_RX_OSR_MODE_FIELD,
    &MISC_2_RESERVED1_FIELD,
    &MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_TX_MODE_FIELD,
    &MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_TX_OSR_MODE_FIELD,
    &MISC_2_RESERVED2_FIELD,
    &MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_TX_DISABLE_FIELD,
    &MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_LN_RX_H_PWRDN_FIELD,
    &MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_LN_TX_H_PWRDN_FIELD,
    &MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_EXT_LOS_FIELD,
    &MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_POR_H_RSTB_FIELD,
    &MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_CORE_1_DP_H_RSTB_FIELD,
    &MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_CORE_0_DP_H_RSTB_FIELD,
    &MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_LN_H_RSTB_FIELD,
    &MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_LN_DP_H_RSTB_FIELD,
    &MISC_2_CR_XGWAN_TOP_WAN_MISC_PMD_RX_MODE_FIELD,
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
    12,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    16,
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
    &MISC_3_CR_XGWAN_TOP_WAN_MISC_WAN_CFG_EPON_TX_FIFO_OFF_FIELD,
    &MISC_3_CR_XGWAN_TOP_WAN_MISC_WAN_CFG_WAN_DEBUG_SEL_FIELD,
    &MISC_3_CFG_NTR_PERIPH_PULSE_BYPASS_FIELD,
    &MISC_3_CFG_NTR_GPIO_PULSE_BYPASS_FIELD,
    &MISC_3_CFG_NTR_SRC_FIELD,
    &MISC_3_CR_XGWAN_TOP_WAN_MISC_WAN_CFG_LASER_OE_FIELD,
    &MISC_3_CR_XGWAN_TOP_WAN_MISC_WAN_CFG_WAN_INTERFACE_SELECT_FIELD,
    &MISC_3_CR_XGWAN_TOP_WAN_MISC_WAN_CFG_LASER_MODE_FIELD,
    &MISC_3_CR_XGWAN_TOP_WAN_MISC_WAN_CFG_LASER_INVERT_FIELD,
    &MISC_3_CR_XGWAN_TOP_WAN_MISC_WAN_CFG_MEM_REB_FIELD,
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
    13,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    10,
    MISC_3_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *MISC_4_FIELDS[] =
{
    &MISC_4_CFG_NTR_PULSE_WIDTH_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec MISC_4_REG = 
{
    "4",
#if RU_INCLUDE_DESC
    "WAN_TOP_MISC_4 Register",
    "Register used for wan_top configuration.",
#endif
    MISC_4_REG_OFFSET,
    0,
    0,
    61,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    MISC_4_FIELDS,
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
    &MISC_4_REG,
};

unsigned long MISC_ADDRS[] =
{
    0x80144028,
};

const ru_block_rec MISC_BLOCK = 
{
    "MISC",
    MISC_ADDRS,
    1,
    5,
    MISC_REGS
};

/* End of file MISC.c */
