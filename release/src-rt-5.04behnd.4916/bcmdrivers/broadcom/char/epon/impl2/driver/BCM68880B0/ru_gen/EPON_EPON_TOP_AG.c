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
 * Field: EPON_TOP_SCRATCH_SCRATCH
 ******************************************************************************/
const ru_field_rec EPON_TOP_SCRATCH_SCRATCH_FIELD =
{
    "SCRATCH",
#if RU_INCLUDE_DESC
    "",
    "Scratch pad.",
#endif
    EPON_TOP_SCRATCH_SCRATCH_FIELD_MASK,
    0,
    EPON_TOP_SCRATCH_SCRATCH_FIELD_WIDTH,
    EPON_TOP_SCRATCH_SCRATCH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPON_TOP_RESET_RESERVED0
 ******************************************************************************/
const ru_field_rec EPON_TOP_RESET_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPON_TOP_RESET_RESERVED0_FIELD_MASK,
    0,
    EPON_TOP_RESET_RESERVED0_FIELD_WIDTH,
    EPON_TOP_RESET_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPON_TOP_RESET_XPCSRXRST_N
 ******************************************************************************/
const ru_field_rec EPON_TOP_RESET_XPCSRXRST_N_FIELD =
{
    "XPCSRXRST_N",
#if RU_INCLUDE_DESC
    "",
    "Active low reset for XPcsRx module.",
#endif
    EPON_TOP_RESET_XPCSRXRST_N_FIELD_MASK,
    0,
    EPON_TOP_RESET_XPCSRXRST_N_FIELD_WIDTH,
    EPON_TOP_RESET_XPCSRXRST_N_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPON_TOP_RESET_XPCSTXRST_N
 ******************************************************************************/
const ru_field_rec EPON_TOP_RESET_XPCSTXRST_N_FIELD =
{
    "XPCSTXRST_N",
#if RU_INCLUDE_DESC
    "",
    "Active low reset for XPcsTx module.",
#endif
    EPON_TOP_RESET_XPCSTXRST_N_FIELD_MASK,
    0,
    EPON_TOP_RESET_XPCSTXRST_N_FIELD_WIDTH,
    EPON_TOP_RESET_XPCSTXRST_N_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPON_TOP_RESET_XIFRST_N
 ******************************************************************************/
const ru_field_rec EPON_TOP_RESET_XIFRST_N_FIELD =
{
    "XIFRST_N",
#if RU_INCLUDE_DESC
    "",
    "Active low reset for XIF module.",
#endif
    EPON_TOP_RESET_XIFRST_N_FIELD_MASK,
    0,
    EPON_TOP_RESET_XIFRST_N_FIELD_WIDTH,
    EPON_TOP_RESET_XIFRST_N_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPON_TOP_RESET_TODRST_N
 ******************************************************************************/
const ru_field_rec EPON_TOP_RESET_TODRST_N_FIELD =
{
    "TODRST_N",
#if RU_INCLUDE_DESC
    "",
    "Active low reset for Tod module.",
#endif
    EPON_TOP_RESET_TODRST_N_FIELD_MASK,
    0,
    EPON_TOP_RESET_TODRST_N_FIELD_WIDTH,
    EPON_TOP_RESET_TODRST_N_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPON_TOP_RESET_CLKPRGRST_N
 ******************************************************************************/
const ru_field_rec EPON_TOP_RESET_CLKPRGRST_N_FIELD =
{
    "CLKPRGRST_N",
#if RU_INCLUDE_DESC
    "",
    "Active low reset for ClkPrgSwch module.",
#endif
    EPON_TOP_RESET_CLKPRGRST_N_FIELD_MASK,
    0,
    EPON_TOP_RESET_CLKPRGRST_N_FIELD_WIDTH,
    EPON_TOP_RESET_CLKPRGRST_N_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPON_TOP_RESET_NCORST_N
 ******************************************************************************/
const ru_field_rec EPON_TOP_RESET_NCORST_N_FIELD =
{
    "NCORST_N",
#if RU_INCLUDE_DESC
    "",
    "Active low reset for Nco module.",
#endif
    EPON_TOP_RESET_NCORST_N_FIELD_MASK,
    0,
    EPON_TOP_RESET_NCORST_N_FIELD_WIDTH,
    EPON_TOP_RESET_NCORST_N_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPON_TOP_RESET_LIFRST_N
 ******************************************************************************/
const ru_field_rec EPON_TOP_RESET_LIFRST_N_FIELD =
{
    "LIFRST_N",
#if RU_INCLUDE_DESC
    "",
    "Active low reset for Lif module.",
#endif
    EPON_TOP_RESET_LIFRST_N_FIELD_MASK,
    0,
    EPON_TOP_RESET_LIFRST_N_FIELD_WIDTH,
    EPON_TOP_RESET_LIFRST_N_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPON_TOP_RESET_EPNRST_N
 ******************************************************************************/
const ru_field_rec EPON_TOP_RESET_EPNRST_N_FIELD =
{
    "EPNRST_N",
#if RU_INCLUDE_DESC
    "",
    "Active low reset for Epn module.",
#endif
    EPON_TOP_RESET_EPNRST_N_FIELD_MASK,
    0,
    EPON_TOP_RESET_EPNRST_N_FIELD_WIDTH,
    EPON_TOP_RESET_EPNRST_N_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPON_TOP_INTERRUPT_RESERVED0
 ******************************************************************************/
const ru_field_rec EPON_TOP_INTERRUPT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPON_TOP_INTERRUPT_RESERVED0_FIELD_MASK,
    0,
    EPON_TOP_INTERRUPT_RESERVED0_FIELD_WIDTH,
    EPON_TOP_INTERRUPT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPON_TOP_INTERRUPT_INT_1PPS
 ******************************************************************************/
const ru_field_rec EPON_TOP_INTERRUPT_INT_1PPS_FIELD =
{
    "INT_1PPS",
#if RU_INCLUDE_DESC
    "",
    "Interrupt from 1 pps input.",
#endif
    EPON_TOP_INTERRUPT_INT_1PPS_FIELD_MASK,
    0,
    EPON_TOP_INTERRUPT_INT_1PPS_FIELD_WIDTH,
    EPON_TOP_INTERRUPT_INT_1PPS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPON_TOP_INTERRUPT_INT_XPCS_TX
 ******************************************************************************/
const ru_field_rec EPON_TOP_INTERRUPT_INT_XPCS_TX_FIELD =
{
    "INT_XPCS_TX",
#if RU_INCLUDE_DESC
    "",
    "Interrupt from XPcsTx module.",
#endif
    EPON_TOP_INTERRUPT_INT_XPCS_TX_FIELD_MASK,
    0,
    EPON_TOP_INTERRUPT_INT_XPCS_TX_FIELD_WIDTH,
    EPON_TOP_INTERRUPT_INT_XPCS_TX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPON_TOP_INTERRUPT_INT_XPCS_RX
 ******************************************************************************/
const ru_field_rec EPON_TOP_INTERRUPT_INT_XPCS_RX_FIELD =
{
    "INT_XPCS_RX",
#if RU_INCLUDE_DESC
    "",
    "Interrupt from XPcsRx module.",
#endif
    EPON_TOP_INTERRUPT_INT_XPCS_RX_FIELD_MASK,
    0,
    EPON_TOP_INTERRUPT_INT_XPCS_RX_FIELD_WIDTH,
    EPON_TOP_INTERRUPT_INT_XPCS_RX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPON_TOP_INTERRUPT_INT_XIF
 ******************************************************************************/
const ru_field_rec EPON_TOP_INTERRUPT_INT_XIF_FIELD =
{
    "INT_XIF",
#if RU_INCLUDE_DESC
    "",
    "Interrupt from XIF module.",
#endif
    EPON_TOP_INTERRUPT_INT_XIF_FIELD_MASK,
    0,
    EPON_TOP_INTERRUPT_INT_XIF_FIELD_WIDTH,
    EPON_TOP_INTERRUPT_INT_XIF_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPON_TOP_INTERRUPT_INT_NCO
 ******************************************************************************/
const ru_field_rec EPON_TOP_INTERRUPT_INT_NCO_FIELD =
{
    "INT_NCO",
#if RU_INCLUDE_DESC
    "",
    "Interrupt from NCO module.",
#endif
    EPON_TOP_INTERRUPT_INT_NCO_FIELD_MASK,
    0,
    EPON_TOP_INTERRUPT_INT_NCO_FIELD_WIDTH,
    EPON_TOP_INTERRUPT_INT_NCO_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPON_TOP_INTERRUPT_INT_LIF
 ******************************************************************************/
const ru_field_rec EPON_TOP_INTERRUPT_INT_LIF_FIELD =
{
    "INT_LIF",
#if RU_INCLUDE_DESC
    "",
    "Interrupt from LIF module.",
#endif
    EPON_TOP_INTERRUPT_INT_LIF_FIELD_MASK,
    0,
    EPON_TOP_INTERRUPT_INT_LIF_FIELD_WIDTH,
    EPON_TOP_INTERRUPT_INT_LIF_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPON_TOP_INTERRUPT_RESERVED1
 ******************************************************************************/
const ru_field_rec EPON_TOP_INTERRUPT_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPON_TOP_INTERRUPT_RESERVED1_FIELD_MASK,
    0,
    EPON_TOP_INTERRUPT_RESERVED1_FIELD_WIDTH,
    EPON_TOP_INTERRUPT_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPON_TOP_INTERRUPT_INT_EPN
 ******************************************************************************/
const ru_field_rec EPON_TOP_INTERRUPT_INT_EPN_FIELD =
{
    "INT_EPN",
#if RU_INCLUDE_DESC
    "",
    "Interrupt from EPN module.",
#endif
    EPON_TOP_INTERRUPT_INT_EPN_FIELD_MASK,
    0,
    EPON_TOP_INTERRUPT_INT_EPN_FIELD_WIDTH,
    EPON_TOP_INTERRUPT_INT_EPN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPON_TOP_INTERRUPT_MASK_RESERVED0
 ******************************************************************************/
const ru_field_rec EPON_TOP_INTERRUPT_MASK_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPON_TOP_INTERRUPT_MASK_RESERVED0_FIELD_MASK,
    0,
    EPON_TOP_INTERRUPT_MASK_RESERVED0_FIELD_WIDTH,
    EPON_TOP_INTERRUPT_MASK_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPON_TOP_INTERRUPT_MASK_INT_1PPS_MASK
 ******************************************************************************/
const ru_field_rec EPON_TOP_INTERRUPT_MASK_INT_1PPS_MASK_FIELD =
{
    "INT_1PPS_MASK",
#if RU_INCLUDE_DESC
    "",
    "Mask for interrupt from 1 pps input.",
#endif
    EPON_TOP_INTERRUPT_MASK_INT_1PPS_MASK_FIELD_MASK,
    0,
    EPON_TOP_INTERRUPT_MASK_INT_1PPS_MASK_FIELD_WIDTH,
    EPON_TOP_INTERRUPT_MASK_INT_1PPS_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPON_TOP_INTERRUPT_MASK_INT_XPCS_TX_MASK
 ******************************************************************************/
const ru_field_rec EPON_TOP_INTERRUPT_MASK_INT_XPCS_TX_MASK_FIELD =
{
    "INT_XPCS_TX_MASK",
#if RU_INCLUDE_DESC
    "",
    "Mask for interrupt from XPcsTx module.",
#endif
    EPON_TOP_INTERRUPT_MASK_INT_XPCS_TX_MASK_FIELD_MASK,
    0,
    EPON_TOP_INTERRUPT_MASK_INT_XPCS_TX_MASK_FIELD_WIDTH,
    EPON_TOP_INTERRUPT_MASK_INT_XPCS_TX_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPON_TOP_INTERRUPT_MASK_INT_XPCS_RX_MASK
 ******************************************************************************/
const ru_field_rec EPON_TOP_INTERRUPT_MASK_INT_XPCS_RX_MASK_FIELD =
{
    "INT_XPCS_RX_MASK",
#if RU_INCLUDE_DESC
    "",
    "Mask for interrupt from XPcsRx module.",
#endif
    EPON_TOP_INTERRUPT_MASK_INT_XPCS_RX_MASK_FIELD_MASK,
    0,
    EPON_TOP_INTERRUPT_MASK_INT_XPCS_RX_MASK_FIELD_WIDTH,
    EPON_TOP_INTERRUPT_MASK_INT_XPCS_RX_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPON_TOP_INTERRUPT_MASK_INT_XIF_MASK
 ******************************************************************************/
const ru_field_rec EPON_TOP_INTERRUPT_MASK_INT_XIF_MASK_FIELD =
{
    "INT_XIF_MASK",
#if RU_INCLUDE_DESC
    "",
    "Mask for interrupt from XIF module.",
#endif
    EPON_TOP_INTERRUPT_MASK_INT_XIF_MASK_FIELD_MASK,
    0,
    EPON_TOP_INTERRUPT_MASK_INT_XIF_MASK_FIELD_WIDTH,
    EPON_TOP_INTERRUPT_MASK_INT_XIF_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPON_TOP_INTERRUPT_MASK_INT_NCO_MASK
 ******************************************************************************/
const ru_field_rec EPON_TOP_INTERRUPT_MASK_INT_NCO_MASK_FIELD =
{
    "INT_NCO_MASK",
#if RU_INCLUDE_DESC
    "",
    "Mask for interrupt from NCO module.",
#endif
    EPON_TOP_INTERRUPT_MASK_INT_NCO_MASK_FIELD_MASK,
    0,
    EPON_TOP_INTERRUPT_MASK_INT_NCO_MASK_FIELD_WIDTH,
    EPON_TOP_INTERRUPT_MASK_INT_NCO_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPON_TOP_INTERRUPT_MASK_INT_LIF_MASK
 ******************************************************************************/
const ru_field_rec EPON_TOP_INTERRUPT_MASK_INT_LIF_MASK_FIELD =
{
    "INT_LIF_MASK",
#if RU_INCLUDE_DESC
    "",
    "Mask for interrupt from LIF module.",
#endif
    EPON_TOP_INTERRUPT_MASK_INT_LIF_MASK_FIELD_MASK,
    0,
    EPON_TOP_INTERRUPT_MASK_INT_LIF_MASK_FIELD_WIDTH,
    EPON_TOP_INTERRUPT_MASK_INT_LIF_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPON_TOP_INTERRUPT_MASK_RESERVED1
 ******************************************************************************/
const ru_field_rec EPON_TOP_INTERRUPT_MASK_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPON_TOP_INTERRUPT_MASK_RESERVED1_FIELD_MASK,
    0,
    EPON_TOP_INTERRUPT_MASK_RESERVED1_FIELD_WIDTH,
    EPON_TOP_INTERRUPT_MASK_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPON_TOP_INTERRUPT_MASK_INT_EPN_MASK
 ******************************************************************************/
const ru_field_rec EPON_TOP_INTERRUPT_MASK_INT_EPN_MASK_FIELD =
{
    "INT_EPN_MASK",
#if RU_INCLUDE_DESC
    "",
    "Mask for interrupt from EPN module.",
#endif
    EPON_TOP_INTERRUPT_MASK_INT_EPN_MASK_FIELD_MASK,
    0,
    EPON_TOP_INTERRUPT_MASK_INT_EPN_MASK_FIELD_WIDTH,
    EPON_TOP_INTERRUPT_MASK_INT_EPN_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPON_TOP_CONTROL_RESERVED0
 ******************************************************************************/
const ru_field_rec EPON_TOP_CONTROL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPON_TOP_CONTROL_RESERVED0_FIELD_MASK,
    0,
    EPON_TOP_CONTROL_RESERVED0_FIELD_WIDTH,
    EPON_TOP_CONTROL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPON_TOP_CONTROL_CFGEPONTXCLK125_322
 ******************************************************************************/
const ru_field_rec EPON_TOP_CONTROL_CFGEPONTXCLK125_322_FIELD =
{
    "CFGEPONTXCLK125_322",
#if RU_INCLUDE_DESC
    "",
    "Mux select for divided.  0: 125MHz divided from 644MHz.  1: 125MHz"
    "divided from 322 MHz.",
#endif
    EPON_TOP_CONTROL_CFGEPONTXCLK125_322_FIELD_MASK,
    0,
    EPON_TOP_CONTROL_CFGEPONTXCLK125_322_FIELD_WIDTH,
    EPON_TOP_CONTROL_CFGEPONTXCLK125_322_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPON_TOP_CONTROL_CFGEPONTXCLK125
 ******************************************************************************/
const ru_field_rec EPON_TOP_CONTROL_CFGEPONTXCLK125_FIELD =
{
    "CFGEPONTXCLK125",
#if RU_INCLUDE_DESC
    "",
    "Mux select for 125MHz clock.  0: sds_tx_word_clk_div_5_6.  1:"
    "sds_tx_word_clk_div_2.  2. reserved.  3: sds_tx_word_clk_div_4.",
#endif
    EPON_TOP_CONTROL_CFGEPONTXCLK125_FIELD_MASK,
    0,
    EPON_TOP_CONTROL_CFGEPONTXCLK125_FIELD_WIDTH,
    EPON_TOP_CONTROL_CFGEPONTXCLK125_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPON_TOP_CONTROL_CFGTWOGIGPONDNS
 ******************************************************************************/
const ru_field_rec EPON_TOP_CONTROL_CFGTWOGIGPONDNS_FIELD =
{
    "CFGTWOGIGPONDNS",
#if RU_INCLUDE_DESC
    "",
    "0: 1G downstream mode 1: 2G downstream mode",
#endif
    EPON_TOP_CONTROL_CFGTWOGIGPONDNS_FIELD_MASK,
    0,
    EPON_TOP_CONTROL_CFGTWOGIGPONDNS_FIELD_WIDTH,
    EPON_TOP_CONTROL_CFGTWOGIGPONDNS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPON_TOP_CONTROL_CFGTENGIGPONUP
 ******************************************************************************/
const ru_field_rec EPON_TOP_CONTROL_CFGTENGIGPONUP_FIELD =
{
    "CFGTENGIGPONUP",
#if RU_INCLUDE_DESC
    "",
    "0: 1G uptream mode 1: 10G upstream mode",
#endif
    EPON_TOP_CONTROL_CFGTENGIGPONUP_FIELD_MASK,
    0,
    EPON_TOP_CONTROL_CFGTENGIGPONUP_FIELD_WIDTH,
    EPON_TOP_CONTROL_CFGTENGIGPONUP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPON_TOP_CONTROL_CFGTENGIGDNS
 ******************************************************************************/
const ru_field_rec EPON_TOP_CONTROL_CFGTENGIGDNS_FIELD =
{
    "CFGTENGIGDNS",
#if RU_INCLUDE_DESC
    "",
    "0: 1G downstream mode 1: 10G downstream mode",
#endif
    EPON_TOP_CONTROL_CFGTENGIGDNS_FIELD_MASK,
    0,
    EPON_TOP_CONTROL_CFGTENGIGDNS_FIELD_WIDTH,
    EPON_TOP_CONTROL_CFGTENGIGDNS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPON_TOP_ONE_PPS_MPCP_OFFSET_CFG_1PPS_MPCP_OFFSET
 ******************************************************************************/
const ru_field_rec EPON_TOP_ONE_PPS_MPCP_OFFSET_CFG_1PPS_MPCP_OFFSET_FIELD =
{
    "CFG_1PPS_MPCP_OFFSET",
#if RU_INCLUDE_DESC
    "",
    "Provides additional offset to MPCP sampling due to 1 pps input"
    "assertion.",
#endif
    EPON_TOP_ONE_PPS_MPCP_OFFSET_CFG_1PPS_MPCP_OFFSET_FIELD_MASK,
    0,
    EPON_TOP_ONE_PPS_MPCP_OFFSET_CFG_1PPS_MPCP_OFFSET_FIELD_WIDTH,
    EPON_TOP_ONE_PPS_MPCP_OFFSET_CFG_1PPS_MPCP_OFFSET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPON_TOP_ONE_PPS_CAPTURED_MPCP_TIME_CAPTURE_1PPS_MPCP_TIME
 ******************************************************************************/
const ru_field_rec EPON_TOP_ONE_PPS_CAPTURED_MPCP_TIME_CAPTURE_1PPS_MPCP_TIME_FIELD =
{
    "CAPTURE_1PPS_MPCP_TIME",
#if RU_INCLUDE_DESC
    "",
    "Captured MPCP time due to 1 pps input assertion.",
#endif
    EPON_TOP_ONE_PPS_CAPTURED_MPCP_TIME_CAPTURE_1PPS_MPCP_TIME_FIELD_MASK,
    0,
    EPON_TOP_ONE_PPS_CAPTURED_MPCP_TIME_CAPTURE_1PPS_MPCP_TIME_FIELD_WIDTH,
    EPON_TOP_ONE_PPS_CAPTURED_MPCP_TIME_CAPTURE_1PPS_MPCP_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPON_TOP_TOD_CONFIG_CFG_TOD_LOAD_NS
 ******************************************************************************/
const ru_field_rec EPON_TOP_TOD_CONFIG_CFG_TOD_LOAD_NS_FIELD =
{
    "CFG_TOD_LOAD_NS",
#if RU_INCLUDE_DESC
    "",
    "When this bit is set, hardware will update the internal nanosecond"
    "counter, cfg_tod_ns[31:0], when the local MPCP time equals"
    "cfg_tod_mpcp[31:0]. Software should set this bit and wait until"
    "hardware clears it before setting it again.",
#endif
    EPON_TOP_TOD_CONFIG_CFG_TOD_LOAD_NS_FIELD_MASK,
    0,
    EPON_TOP_TOD_CONFIG_CFG_TOD_LOAD_NS_FIELD_WIDTH,
    EPON_TOP_TOD_CONFIG_CFG_TOD_LOAD_NS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPON_TOP_TOD_CONFIG_RESERVED0
 ******************************************************************************/
const ru_field_rec EPON_TOP_TOD_CONFIG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPON_TOP_TOD_CONFIG_RESERVED0_FIELD_MASK,
    0,
    EPON_TOP_TOD_CONFIG_RESERVED0_FIELD_WIDTH,
    EPON_TOP_TOD_CONFIG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPON_TOP_TOD_CONFIG_CFG_TOD_5G
 ******************************************************************************/
const ru_field_rec EPON_TOP_TOD_CONFIG_CFG_TOD_5G_FIELD =
{
    "CFG_TOD_5G",
#if RU_INCLUDE_DESC
    "",
    "Indicates 5G rate for TS48 generation.",
#endif
    EPON_TOP_TOD_CONFIG_CFG_TOD_5G_FIELD_MASK,
    0,
    EPON_TOP_TOD_CONFIG_CFG_TOD_5G_FIELD_WIDTH,
    EPON_TOP_TOD_CONFIG_CFG_TOD_5G_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPON_TOP_TOD_CONFIG_CFG_TOD_READ
 ******************************************************************************/
const ru_field_rec EPON_TOP_TOD_CONFIG_CFG_TOD_READ_FIELD =
{
    "CFG_TOD_READ",
#if RU_INCLUDE_DESC
    "",
    "When this bit is set, hardware will latch the internal ts48, ns, and"
    "seconds counters. Software should set this bit and wait until"
    "hardware clears it before setting it again. Once hardware has"
    "cleared the bit, the timers are available to be read.",
#endif
    EPON_TOP_TOD_CONFIG_CFG_TOD_READ_FIELD_MASK,
    0,
    EPON_TOP_TOD_CONFIG_CFG_TOD_READ_FIELD_WIDTH,
    EPON_TOP_TOD_CONFIG_CFG_TOD_READ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPON_TOP_TOD_CONFIG_CFG_TOD_READ_SEL
 ******************************************************************************/
const ru_field_rec EPON_TOP_TOD_CONFIG_CFG_TOD_READ_SEL_FIELD =
{
    "CFG_TOD_READ_SEL",
#if RU_INCLUDE_DESC
    "",
    "Select the block to read the timers from.  0: Reserved. 1: 1G EPON."
    "2:10G EPON. 3: AE. This field should not be changed while"
    "cfg_tod_read is set.",
#endif
    EPON_TOP_TOD_CONFIG_CFG_TOD_READ_SEL_FIELD_MASK,
    0,
    EPON_TOP_TOD_CONFIG_CFG_TOD_READ_SEL_FIELD_WIDTH,
    EPON_TOP_TOD_CONFIG_CFG_TOD_READ_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPON_TOP_TOD_CONFIG_CFG_TOD_PPS_CLEAR
 ******************************************************************************/
const ru_field_rec EPON_TOP_TOD_CONFIG_CFG_TOD_PPS_CLEAR_FIELD =
{
    "CFG_TOD_PPS_CLEAR",
#if RU_INCLUDE_DESC
    "",
    "Allow 1PPS pulse to clear the counter if set.  If not set, the 1PPS"
    "pulse will have no effect on the TS48.",
#endif
    EPON_TOP_TOD_CONFIG_CFG_TOD_PPS_CLEAR_FIELD_MASK,
    0,
    EPON_TOP_TOD_CONFIG_CFG_TOD_PPS_CLEAR_FIELD_WIDTH,
    EPON_TOP_TOD_CONFIG_CFG_TOD_PPS_CLEAR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPON_TOP_TOD_CONFIG_CFG_TOD_LOAD
 ******************************************************************************/
const ru_field_rec EPON_TOP_TOD_CONFIG_CFG_TOD_LOAD_FIELD =
{
    "CFG_TOD_LOAD",
#if RU_INCLUDE_DESC
    "",
    "The rising edge will be latched, and cfg_tod_seconds will be loaded"
    "on the next 1PPS pulse or when the next second rolls over.",
#endif
    EPON_TOP_TOD_CONFIG_CFG_TOD_LOAD_FIELD_MASK,
    0,
    EPON_TOP_TOD_CONFIG_CFG_TOD_LOAD_FIELD_WIDTH,
    EPON_TOP_TOD_CONFIG_CFG_TOD_LOAD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPON_TOP_TOD_CONFIG_CFG_TOD_SECONDS
 ******************************************************************************/
const ru_field_rec EPON_TOP_TOD_CONFIG_CFG_TOD_SECONDS_FIELD =
{
    "CFG_TOD_SECONDS",
#if RU_INCLUDE_DESC
    "",
    "Number of seconds to be loaded.",
#endif
    EPON_TOP_TOD_CONFIG_CFG_TOD_SECONDS_FIELD_MASK,
    0,
    EPON_TOP_TOD_CONFIG_CFG_TOD_SECONDS_FIELD_WIDTH,
    EPON_TOP_TOD_CONFIG_CFG_TOD_SECONDS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPON_TOP_TOD_NS_CFG_TOD_NS
 ******************************************************************************/
const ru_field_rec EPON_TOP_TOD_NS_CFG_TOD_NS_FIELD =
{
    "CFG_TOD_NS",
#if RU_INCLUDE_DESC
    "",
    "Value to be loaded when the MPCP time reaches cfg_tod_mpcp. This"
    "field should not be updated while cfg_tod_load_ns is set.",
#endif
    EPON_TOP_TOD_NS_CFG_TOD_NS_FIELD_MASK,
    0,
    EPON_TOP_TOD_NS_CFG_TOD_NS_FIELD_WIDTH,
    EPON_TOP_TOD_NS_CFG_TOD_NS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPON_TOP_TOD_MPCP_CFG_TOD_MPCP
 ******************************************************************************/
const ru_field_rec EPON_TOP_TOD_MPCP_CFG_TOD_MPCP_FIELD =
{
    "CFG_TOD_MPCP",
#if RU_INCLUDE_DESC
    "",
    "MPCP value to wait for before loading cfg_tod_ns.",
#endif
    EPON_TOP_TOD_MPCP_CFG_TOD_MPCP_FIELD_MASK,
    0,
    EPON_TOP_TOD_MPCP_CFG_TOD_MPCP_FIELD_WIDTH,
    EPON_TOP_TOD_MPCP_CFG_TOD_MPCP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: EPON_TOP_TS48_MSB_RESERVED0
 ******************************************************************************/
const ru_field_rec EPON_TOP_TS48_MSB_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPON_TOP_TS48_MSB_RESERVED0_FIELD_MASK,
    0,
    EPON_TOP_TS48_MSB_RESERVED0_FIELD_WIDTH,
    EPON_TOP_TS48_MSB_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPON_TOP_TS48_MSB_TS48_EPON_READ_MSB
 ******************************************************************************/
const ru_field_rec EPON_TOP_TS48_MSB_TS48_EPON_READ_MSB_FIELD =
{
    "TS48_EPON_READ_MSB",
#if RU_INCLUDE_DESC
    "",
    "Upper 16-bits of TS48.",
#endif
    EPON_TOP_TS48_MSB_TS48_EPON_READ_MSB_FIELD_MASK,
    0,
    EPON_TOP_TS48_MSB_TS48_EPON_READ_MSB_FIELD_WIDTH,
    EPON_TOP_TS48_MSB_TS48_EPON_READ_MSB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPON_TOP_TS48_LSB_TS48_EPON_READ_LSB
 ******************************************************************************/
const ru_field_rec EPON_TOP_TS48_LSB_TS48_EPON_READ_LSB_FIELD =
{
    "TS48_EPON_READ_LSB",
#if RU_INCLUDE_DESC
    "",
    "Lower 32-bits of TS48.",
#endif
    EPON_TOP_TS48_LSB_TS48_EPON_READ_LSB_FIELD_MASK,
    0,
    EPON_TOP_TS48_LSB_TS48_EPON_READ_LSB_FIELD_WIDTH,
    EPON_TOP_TS48_LSB_TS48_EPON_READ_LSB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPON_TOP_TSEC_RESERVED0
 ******************************************************************************/
const ru_field_rec EPON_TOP_TSEC_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    EPON_TOP_TSEC_RESERVED0_FIELD_MASK,
    0,
    EPON_TOP_TSEC_RESERVED0_FIELD_WIDTH,
    EPON_TOP_TSEC_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPON_TOP_TSEC_TSEC_EPON_READ
 ******************************************************************************/
const ru_field_rec EPON_TOP_TSEC_TSEC_EPON_READ_FIELD =
{
    "TSEC_EPON_READ",
#if RU_INCLUDE_DESC
    "",
    "Seconds[18:0].",
#endif
    EPON_TOP_TSEC_TSEC_EPON_READ_FIELD_MASK,
    0,
    EPON_TOP_TSEC_TSEC_EPON_READ_FIELD_WIDTH,
    EPON_TOP_TSEC_TSEC_EPON_READ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: EPON_TOP_TNS_EPON_TNS_EPON_READ
 ******************************************************************************/
const ru_field_rec EPON_TOP_TNS_EPON_TNS_EPON_READ_FIELD =
{
    "TNS_EPON_READ",
#if RU_INCLUDE_DESC
    "",
    "Nanoseconds[31:0].",
#endif
    EPON_TOP_TNS_EPON_TNS_EPON_READ_FIELD_MASK,
    0,
    EPON_TOP_TNS_EPON_TNS_EPON_READ_FIELD_WIDTH,
    EPON_TOP_TNS_EPON_TNS_EPON_READ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: EPON_TOP_SCRATCH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPON_TOP_SCRATCH_FIELDS[] =
{
    &EPON_TOP_SCRATCH_SCRATCH_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPON_TOP_SCRATCH_REG = 
{
    "SCRATCH",
#if RU_INCLUDE_DESC
    "EPON_TOP_SCRATCH Register",
    "Register used for testing read and write access into epon_top block.",
#endif
    EPON_TOP_SCRATCH_REG_OFFSET,
    0,
    0,
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    EPON_TOP_SCRATCH_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPON_TOP_RESET
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPON_TOP_RESET_FIELDS[] =
{
    &EPON_TOP_RESET_RESERVED0_FIELD,
    &EPON_TOP_RESET_XPCSRXRST_N_FIELD,
    &EPON_TOP_RESET_XPCSTXRST_N_FIELD,
    &EPON_TOP_RESET_XIFRST_N_FIELD,
    &EPON_TOP_RESET_TODRST_N_FIELD,
    &EPON_TOP_RESET_CLKPRGRST_N_FIELD,
    &EPON_TOP_RESET_NCORST_N_FIELD,
    &EPON_TOP_RESET_LIFRST_N_FIELD,
    &EPON_TOP_RESET_EPNRST_N_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPON_TOP_RESET_REG = 
{
    "RESET",
#if RU_INCLUDE_DESC
    "EPON_TOP_RESET Register",
    "",
#endif
    EPON_TOP_RESET_REG_OFFSET,
    0,
    0,
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    EPON_TOP_RESET_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPON_TOP_INTERRUPT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPON_TOP_INTERRUPT_FIELDS[] =
{
    &EPON_TOP_INTERRUPT_RESERVED0_FIELD,
    &EPON_TOP_INTERRUPT_INT_1PPS_FIELD,
    &EPON_TOP_INTERRUPT_INT_XPCS_TX_FIELD,
    &EPON_TOP_INTERRUPT_INT_XPCS_RX_FIELD,
    &EPON_TOP_INTERRUPT_INT_XIF_FIELD,
    &EPON_TOP_INTERRUPT_INT_NCO_FIELD,
    &EPON_TOP_INTERRUPT_INT_LIF_FIELD,
    &EPON_TOP_INTERRUPT_RESERVED1_FIELD,
    &EPON_TOP_INTERRUPT_INT_EPN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPON_TOP_INTERRUPT_REG = 
{
    "INTERRUPT",
#if RU_INCLUDE_DESC
    "EPON_TOP_INTERRUPT Register",
    "Top level interrupts for all EPON blocks.",
#endif
    EPON_TOP_INTERRUPT_REG_OFFSET,
    0,
    0,
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    EPON_TOP_INTERRUPT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPON_TOP_INTERRUPT_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPON_TOP_INTERRUPT_MASK_FIELDS[] =
{
    &EPON_TOP_INTERRUPT_MASK_RESERVED0_FIELD,
    &EPON_TOP_INTERRUPT_MASK_INT_1PPS_MASK_FIELD,
    &EPON_TOP_INTERRUPT_MASK_INT_XPCS_TX_MASK_FIELD,
    &EPON_TOP_INTERRUPT_MASK_INT_XPCS_RX_MASK_FIELD,
    &EPON_TOP_INTERRUPT_MASK_INT_XIF_MASK_FIELD,
    &EPON_TOP_INTERRUPT_MASK_INT_NCO_MASK_FIELD,
    &EPON_TOP_INTERRUPT_MASK_INT_LIF_MASK_FIELD,
    &EPON_TOP_INTERRUPT_MASK_RESERVED1_FIELD,
    &EPON_TOP_INTERRUPT_MASK_INT_EPN_MASK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPON_TOP_INTERRUPT_MASK_REG = 
{
    "INTERRUPT_MASK",
#if RU_INCLUDE_DESC
    "EPON_TOP_INTERRUPT_MASK Register",
    "Top level interrupts for all EPON blocks. For any bit, a value of 1"
    "will enable the interrupt, and a value of 0 will mask the interrupt."
    "By default, all interrupts are masked.",
#endif
    EPON_TOP_INTERRUPT_MASK_REG_OFFSET,
    0,
    0,
    3,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    EPON_TOP_INTERRUPT_MASK_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPON_TOP_CONTROL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPON_TOP_CONTROL_FIELDS[] =
{
    &EPON_TOP_CONTROL_RESERVED0_FIELD,
    &EPON_TOP_CONTROL_CFGEPONTXCLK125_322_FIELD,
    &EPON_TOP_CONTROL_CFGEPONTXCLK125_FIELD,
    &EPON_TOP_CONTROL_CFGTWOGIGPONDNS_FIELD,
    &EPON_TOP_CONTROL_CFGTENGIGPONUP_FIELD,
    &EPON_TOP_CONTROL_CFGTENGIGDNS_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPON_TOP_CONTROL_REG = 
{
    "CONTROL",
#if RU_INCLUDE_DESC
    "EPON_TOP_CONTROL Register",
    "High level configuration for the EPON block.",
#endif
    EPON_TOP_CONTROL_REG_OFFSET,
    0,
    0,
    4,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    EPON_TOP_CONTROL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPON_TOP_ONE_PPS_MPCP_OFFSET
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPON_TOP_ONE_PPS_MPCP_OFFSET_FIELDS[] =
{
    &EPON_TOP_ONE_PPS_MPCP_OFFSET_CFG_1PPS_MPCP_OFFSET_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPON_TOP_ONE_PPS_MPCP_OFFSET_REG = 
{
    "ONE_PPS_MPCP_OFFSET",
#if RU_INCLUDE_DESC
    "ONE_PPS_MPCP_OFFSET Register",
    "High level configuration for the EPON block.",
#endif
    EPON_TOP_ONE_PPS_MPCP_OFFSET_REG_OFFSET,
    0,
    0,
    5,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    EPON_TOP_ONE_PPS_MPCP_OFFSET_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPON_TOP_ONE_PPS_CAPTURED_MPCP_TIME
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPON_TOP_ONE_PPS_CAPTURED_MPCP_TIME_FIELDS[] =
{
    &EPON_TOP_ONE_PPS_CAPTURED_MPCP_TIME_CAPTURE_1PPS_MPCP_TIME_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPON_TOP_ONE_PPS_CAPTURED_MPCP_TIME_REG = 
{
    "ONE_PPS_CAPTURED_MPCP_TIME",
#if RU_INCLUDE_DESC
    "ONE_PPS_CAPTURED_MPCP_TIME Register",
    "High level configuration for the EPON block.",
#endif
    EPON_TOP_ONE_PPS_CAPTURED_MPCP_TIME_REG_OFFSET,
    0,
    0,
    6,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    EPON_TOP_ONE_PPS_CAPTURED_MPCP_TIME_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPON_TOP_TOD_CONFIG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPON_TOP_TOD_CONFIG_FIELDS[] =
{
    &EPON_TOP_TOD_CONFIG_CFG_TOD_LOAD_NS_FIELD,
    &EPON_TOP_TOD_CONFIG_RESERVED0_FIELD,
    &EPON_TOP_TOD_CONFIG_CFG_TOD_5G_FIELD,
    &EPON_TOP_TOD_CONFIG_CFG_TOD_READ_FIELD,
    &EPON_TOP_TOD_CONFIG_CFG_TOD_READ_SEL_FIELD,
    &EPON_TOP_TOD_CONFIG_CFG_TOD_PPS_CLEAR_FIELD,
    &EPON_TOP_TOD_CONFIG_CFG_TOD_LOAD_FIELD,
    &EPON_TOP_TOD_CONFIG_CFG_TOD_SECONDS_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPON_TOP_TOD_CONFIG_REG = 
{
    "TOD_CONFIG",
#if RU_INCLUDE_DESC
    "EPON_TOP_TOD_CONFIG Register",
    "Register used for 48-bit timestamp Time Of Day (TOD) configuration.",
#endif
    EPON_TOP_TOD_CONFIG_REG_OFFSET,
    0,
    0,
    7,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    EPON_TOP_TOD_CONFIG_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPON_TOP_TOD_NS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPON_TOP_TOD_NS_FIELDS[] =
{
    &EPON_TOP_TOD_NS_CFG_TOD_NS_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPON_TOP_TOD_NS_REG = 
{
    "TOD_NS",
#if RU_INCLUDE_DESC
    "EPON_TOP_TOD_NS Register",
    "Register used to load nanosecond counter.",
#endif
    EPON_TOP_TOD_NS_REG_OFFSET,
    0,
    0,
    8,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    EPON_TOP_TOD_NS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPON_TOP_TOD_MPCP
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPON_TOP_TOD_MPCP_FIELDS[] =
{
    &EPON_TOP_TOD_MPCP_CFG_TOD_MPCP_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPON_TOP_TOD_MPCP_REG = 
{
    "TOD_MPCP",
#if RU_INCLUDE_DESC
    "EPON_TOP_TOD_MPCP Register",
    "Register used to hold MPCP value that will be used to determine when"
    "the nanosecond counter is updated.  This field should not be updated"
    "while cfg_tod_load_ns is set.",
#endif
    EPON_TOP_TOD_MPCP_REG_OFFSET,
    0,
    0,
    9,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    EPON_TOP_TOD_MPCP_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPON_TOP_TS48_MSB
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPON_TOP_TS48_MSB_FIELDS[] =
{
    &EPON_TOP_TS48_MSB_RESERVED0_FIELD,
    &EPON_TOP_TS48_MSB_TS48_EPON_READ_MSB_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPON_TOP_TS48_MSB_REG = 
{
    "TS48_MSB",
#if RU_INCLUDE_DESC
    "EPON_TOP_TS48_MSB Register",
    "Register used for 48-bit timestamp Time Of Day (TOD) read back from"
    "EPON/AE block.",
#endif
    EPON_TOP_TS48_MSB_REG_OFFSET,
    0,
    0,
    10,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPON_TOP_TS48_MSB_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPON_TOP_TS48_LSB
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPON_TOP_TS48_LSB_FIELDS[] =
{
    &EPON_TOP_TS48_LSB_TS48_EPON_READ_LSB_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPON_TOP_TS48_LSB_REG = 
{
    "TS48_LSB",
#if RU_INCLUDE_DESC
    "EPON_TOP_TS48_LSB Register",
    "Register used for 48-bit timestamp Time Of Day (TOD) read back from"
    "EPON/AE block.",
#endif
    EPON_TOP_TS48_LSB_REG_OFFSET,
    0,
    0,
    11,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    EPON_TOP_TS48_LSB_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPON_TOP_TSEC
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPON_TOP_TSEC_FIELDS[] =
{
    &EPON_TOP_TSEC_RESERVED0_FIELD,
    &EPON_TOP_TSEC_TSEC_EPON_READ_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPON_TOP_TSEC_REG = 
{
    "TSEC",
#if RU_INCLUDE_DESC
    "EPON_TOP_TSEC Register",
    "Register used for seconds read back from EPON/AE block.",
#endif
    EPON_TOP_TSEC_REG_OFFSET,
    0,
    0,
    12,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    EPON_TOP_TSEC_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: EPON_TOP_TNS_EPON
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *EPON_TOP_TNS_EPON_FIELDS[] =
{
    &EPON_TOP_TNS_EPON_TNS_EPON_READ_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec EPON_TOP_TNS_EPON_REG = 
{
    "TNS_EPON",
#if RU_INCLUDE_DESC
    "EPON_TOP_TNS_EPON Register",
    "Register used for nanoseconds read back from EPON/AE block.",
#endif
    EPON_TOP_TNS_EPON_REG_OFFSET,
    0,
    0,
    13,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    EPON_TOP_TNS_EPON_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: EPON_TOP
 ******************************************************************************/
static const ru_reg_rec *EPON_TOP_REGS[] =
{
    &EPON_TOP_SCRATCH_REG,
    &EPON_TOP_RESET_REG,
    &EPON_TOP_INTERRUPT_REG,
    &EPON_TOP_INTERRUPT_MASK_REG,
    &EPON_TOP_CONTROL_REG,
    &EPON_TOP_ONE_PPS_MPCP_OFFSET_REG,
    &EPON_TOP_ONE_PPS_CAPTURED_MPCP_TIME_REG,
    &EPON_TOP_TOD_CONFIG_REG,
    &EPON_TOP_TOD_NS_REG,
    &EPON_TOP_TOD_MPCP_REG,
    &EPON_TOP_TS48_MSB_REG,
    &EPON_TOP_TS48_LSB_REG,
    &EPON_TOP_TSEC_REG,
    &EPON_TOP_TNS_EPON_REG,
};

static unsigned long EPON_TOP_ADDRS[] =
{
    0x80100000,
};

const ru_block_rec EPON_TOP_BLOCK = 
{
    "EPON_TOP",
    EPON_TOP_ADDRS,
    1,
    14,
    EPON_TOP_REGS
};

/* End of file EPON_EPON_TOP.c */
