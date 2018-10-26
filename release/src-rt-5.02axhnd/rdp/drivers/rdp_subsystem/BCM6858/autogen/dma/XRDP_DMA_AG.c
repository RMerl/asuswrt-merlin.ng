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
 * Field: DMA_CONFIG_BBROUTEOVRD_DEST
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_BBROUTEOVRD_DEST_FIELD =
{
    "DEST",
#if RU_INCLUDE_DESC
    "dest_id",
    "destination ID",
#endif
    DMA_CONFIG_BBROUTEOVRD_DEST_FIELD_MASK,
    0,
    DMA_CONFIG_BBROUTEOVRD_DEST_FIELD_WIDTH,
    DMA_CONFIG_BBROUTEOVRD_DEST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_BBROUTEOVRD_RESERVED0
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_BBROUTEOVRD_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DMA_CONFIG_BBROUTEOVRD_RESERVED0_FIELD_MASK,
    0,
    DMA_CONFIG_BBROUTEOVRD_RESERVED0_FIELD_WIDTH,
    DMA_CONFIG_BBROUTEOVRD_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_BBROUTEOVRD_ROUTE
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_BBROUTEOVRD_ROUTE_FIELD =
{
    "ROUTE",
#if RU_INCLUDE_DESC
    "route_override",
    "the route to be used (override the default route)",
#endif
    DMA_CONFIG_BBROUTEOVRD_ROUTE_FIELD_MASK,
    0,
    DMA_CONFIG_BBROUTEOVRD_ROUTE_FIELD_WIDTH,
    DMA_CONFIG_BBROUTEOVRD_ROUTE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_BBROUTEOVRD_RESERVED1
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_BBROUTEOVRD_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DMA_CONFIG_BBROUTEOVRD_RESERVED1_FIELD_MASK,
    0,
    DMA_CONFIG_BBROUTEOVRD_RESERVED1_FIELD_WIDTH,
    DMA_CONFIG_BBROUTEOVRD_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_BBROUTEOVRD_OVRD
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_BBROUTEOVRD_OVRD_FIELD =
{
    "OVRD",
#if RU_INCLUDE_DESC
    "OVRD_EN",
    "override enable",
#endif
    DMA_CONFIG_BBROUTEOVRD_OVRD_FIELD_MASK,
    0,
    DMA_CONFIG_BBROUTEOVRD_OVRD_FIELD_WIDTH,
    DMA_CONFIG_BBROUTEOVRD_OVRD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_BBROUTEOVRD_RESERVED2
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_BBROUTEOVRD_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DMA_CONFIG_BBROUTEOVRD_RESERVED2_FIELD_MASK,
    0,
    DMA_CONFIG_BBROUTEOVRD_RESERVED2_FIELD_WIDTH,
    DMA_CONFIG_BBROUTEOVRD_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_NUM_OF_WRITES_NUMOFBUFF
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_NUM_OF_WRITES_NUMOFBUFF_FIELD =
{
    "NUMOFBUFF",
#if RU_INCLUDE_DESC
    "number_of_buffers",
    "the number of 128bytes buffers allocated to the peripheral."
    "",
#endif
    DMA_CONFIG_NUM_OF_WRITES_NUMOFBUFF_FIELD_MASK,
    0,
    DMA_CONFIG_NUM_OF_WRITES_NUMOFBUFF_FIELD_WIDTH,
    DMA_CONFIG_NUM_OF_WRITES_NUMOFBUFF_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_NUM_OF_WRITES_RESERVED0
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_NUM_OF_WRITES_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DMA_CONFIG_NUM_OF_WRITES_RESERVED0_FIELD_MASK,
    0,
    DMA_CONFIG_NUM_OF_WRITES_RESERVED0_FIELD_WIDTH,
    DMA_CONFIG_NUM_OF_WRITES_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_NUM_OF_READS_RR_NUM
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_NUM_OF_READS_RR_NUM_FIELD =
{
    "RR_NUM",
#if RU_INCLUDE_DESC
    "NUM_OF_READ_REQ",
    "number of read requests",
#endif
    DMA_CONFIG_NUM_OF_READS_RR_NUM_FIELD_MASK,
    0,
    DMA_CONFIG_NUM_OF_READS_RR_NUM_FIELD_WIDTH,
    DMA_CONFIG_NUM_OF_READS_RR_NUM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_NUM_OF_READS_RESERVED0
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_NUM_OF_READS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DMA_CONFIG_NUM_OF_READS_RESERVED0_FIELD_MASK,
    0,
    DMA_CONFIG_NUM_OF_READS_RESERVED0_FIELD_WIDTH,
    DMA_CONFIG_NUM_OF_READS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_U_THRESH_INTO_U
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_U_THRESH_INTO_U_FIELD =
{
    "INTO_U",
#if RU_INCLUDE_DESC
    "into_urgent_threshold",
    "moving into urgent threshold",
#endif
    DMA_CONFIG_U_THRESH_INTO_U_FIELD_MASK,
    0,
    DMA_CONFIG_U_THRESH_INTO_U_FIELD_WIDTH,
    DMA_CONFIG_U_THRESH_INTO_U_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_U_THRESH_RESERVED0
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_U_THRESH_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DMA_CONFIG_U_THRESH_RESERVED0_FIELD_MASK,
    0,
    DMA_CONFIG_U_THRESH_RESERVED0_FIELD_WIDTH,
    DMA_CONFIG_U_THRESH_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_U_THRESH_OUT_OF_U
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_U_THRESH_OUT_OF_U_FIELD =
{
    "OUT_OF_U",
#if RU_INCLUDE_DESC
    "out_of_urgent_threshold",
    "moving out ot urgent threshold",
#endif
    DMA_CONFIG_U_THRESH_OUT_OF_U_FIELD_MASK,
    0,
    DMA_CONFIG_U_THRESH_OUT_OF_U_FIELD_WIDTH,
    DMA_CONFIG_U_THRESH_OUT_OF_U_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_U_THRESH_RESERVED1
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_U_THRESH_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DMA_CONFIG_U_THRESH_RESERVED1_FIELD_MASK,
    0,
    DMA_CONFIG_U_THRESH_RESERVED1_FIELD_WIDTH,
    DMA_CONFIG_U_THRESH_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_PRI_RXPRI
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_PRI_RXPRI_FIELD =
{
    "RXPRI",
#if RU_INCLUDE_DESC
    "priority_of_rx_side",
    "priority of rx side (upload) of the peripheral",
#endif
    DMA_CONFIG_PRI_RXPRI_FIELD_MASK,
    0,
    DMA_CONFIG_PRI_RXPRI_FIELD_WIDTH,
    DMA_CONFIG_PRI_RXPRI_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_PRI_TXPRI
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_PRI_TXPRI_FIELD =
{
    "TXPRI",
#if RU_INCLUDE_DESC
    "priority_of_tx_side",
    "priority of tx side (download) of the peripheral",
#endif
    DMA_CONFIG_PRI_TXPRI_FIELD_MASK,
    0,
    DMA_CONFIG_PRI_TXPRI_FIELD_WIDTH,
    DMA_CONFIG_PRI_TXPRI_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_PRI_RESERVED0
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_PRI_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DMA_CONFIG_PRI_RESERVED0_FIELD_MASK,
    0,
    DMA_CONFIG_PRI_RESERVED0_FIELD_WIDTH,
    DMA_CONFIG_PRI_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_PERIPH_SOURCE_RXSOURCE
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_PERIPH_SOURCE_RXSOURCE_FIELD =
{
    "RXSOURCE",
#if RU_INCLUDE_DESC
    "bb_source_rx_side",
    "bb source of rx side (upload) of the peripheral",
#endif
    DMA_CONFIG_PERIPH_SOURCE_RXSOURCE_FIELD_MASK,
    0,
    DMA_CONFIG_PERIPH_SOURCE_RXSOURCE_FIELD_WIDTH,
    DMA_CONFIG_PERIPH_SOURCE_RXSOURCE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_PERIPH_SOURCE_RESERVED0
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_PERIPH_SOURCE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DMA_CONFIG_PERIPH_SOURCE_RESERVED0_FIELD_MASK,
    0,
    DMA_CONFIG_PERIPH_SOURCE_RESERVED0_FIELD_WIDTH,
    DMA_CONFIG_PERIPH_SOURCE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_PERIPH_SOURCE_TXSOURCE
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_PERIPH_SOURCE_TXSOURCE_FIELD =
{
    "TXSOURCE",
#if RU_INCLUDE_DESC
    "bb_source_tx_side",
    "bb source of tx side (download) of the peripheral",
#endif
    DMA_CONFIG_PERIPH_SOURCE_TXSOURCE_FIELD_MASK,
    0,
    DMA_CONFIG_PERIPH_SOURCE_TXSOURCE_FIELD_WIDTH,
    DMA_CONFIG_PERIPH_SOURCE_TXSOURCE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_PERIPH_SOURCE_RESERVED1
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_PERIPH_SOURCE_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DMA_CONFIG_PERIPH_SOURCE_RESERVED1_FIELD_MASK,
    0,
    DMA_CONFIG_PERIPH_SOURCE_RESERVED1_FIELD_WIDTH,
    DMA_CONFIG_PERIPH_SOURCE_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_WEIGHT_RXWEIGHT
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_WEIGHT_RXWEIGHT_FIELD =
{
    "RXWEIGHT",
#if RU_INCLUDE_DESC
    "weight_of_rx_side",
    "weight of rx side (upload) of the peripheral",
#endif
    DMA_CONFIG_WEIGHT_RXWEIGHT_FIELD_MASK,
    0,
    DMA_CONFIG_WEIGHT_RXWEIGHT_FIELD_WIDTH,
    DMA_CONFIG_WEIGHT_RXWEIGHT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_WEIGHT_RESERVED0
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_WEIGHT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DMA_CONFIG_WEIGHT_RESERVED0_FIELD_MASK,
    0,
    DMA_CONFIG_WEIGHT_RESERVED0_FIELD_WIDTH,
    DMA_CONFIG_WEIGHT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_WEIGHT_TXWEIGHT
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_WEIGHT_TXWEIGHT_FIELD =
{
    "TXWEIGHT",
#if RU_INCLUDE_DESC
    "weight_of_tx_side",
    "weight of tx side (download) of the peripheral",
#endif
    DMA_CONFIG_WEIGHT_TXWEIGHT_FIELD_MASK,
    0,
    DMA_CONFIG_WEIGHT_TXWEIGHT_FIELD_WIDTH,
    DMA_CONFIG_WEIGHT_TXWEIGHT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_WEIGHT_RESERVED1
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_WEIGHT_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DMA_CONFIG_WEIGHT_RESERVED1_FIELD_MASK,
    0,
    DMA_CONFIG_WEIGHT_RESERVED1_FIELD_WIDTH,
    DMA_CONFIG_WEIGHT_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_PTRRST_RSTVEC
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_PTRRST_RSTVEC_FIELD =
{
    "RSTVEC",
#if RU_INCLUDE_DESC
    "reset_vector",
    "vector in which each bit represents a peripheral."
    "LSB represent RX peripherals and MSB represent TX peripherals."
    "When asserted, the relevant FIFOS of the selected peripheral will be reset to zero",
#endif
    DMA_CONFIG_PTRRST_RSTVEC_FIELD_MASK,
    0,
    DMA_CONFIG_PTRRST_RSTVEC_FIELD_WIDTH,
    DMA_CONFIG_PTRRST_RSTVEC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_PTRRST_RESERVED0
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_PTRRST_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DMA_CONFIG_PTRRST_RESERVED0_FIELD_MASK,
    0,
    DMA_CONFIG_PTRRST_RESERVED0_FIELD_WIDTH,
    DMA_CONFIG_PTRRST_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_MAX_OTF_MAX
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_MAX_OTF_MAX_FIELD =
{
    "MAX",
#if RU_INCLUDE_DESC
    "max_on_the_fly",
    "max on the fly",
#endif
    DMA_CONFIG_MAX_OTF_MAX_FIELD_MASK,
    0,
    DMA_CONFIG_MAX_OTF_MAX_FIELD_WIDTH,
    DMA_CONFIG_MAX_OTF_MAX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_MAX_OTF_RESERVED0
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_MAX_OTF_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DMA_CONFIG_MAX_OTF_RESERVED0_FIELD_MASK,
    0,
    DMA_CONFIG_MAX_OTF_RESERVED0_FIELD_WIDTH,
    DMA_CONFIG_MAX_OTF_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_CLK_GATE_CNTRL_BYPASS_CLK_GATE
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD =
{
    "BYPASS_CLK_GATE",
#if RU_INCLUDE_DESC
    "BYPASS_CLOCK_GATE",
    "If set to 1b1 will disable the clock gate logic such to always enable the clock",
#endif
    DMA_CONFIG_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_MASK,
    0,
    DMA_CONFIG_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_WIDTH,
    DMA_CONFIG_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_CLK_GATE_CNTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_CLK_GATE_CNTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DMA_CONFIG_CLK_GATE_CNTRL_RESERVED0_FIELD_MASK,
    0,
    DMA_CONFIG_CLK_GATE_CNTRL_RESERVED0_FIELD_WIDTH,
    DMA_CONFIG_CLK_GATE_CNTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_CLK_GATE_CNTRL_TIMER_VAL
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_CLK_GATE_CNTRL_TIMER_VAL_FIELD =
{
    "TIMER_VAL",
#if RU_INCLUDE_DESC
    "TIMER_VALUE",
    "For how long should the clock stay active once all conditions for clock disable are met."
    ""
    "",
#endif
    DMA_CONFIG_CLK_GATE_CNTRL_TIMER_VAL_FIELD_MASK,
    0,
    DMA_CONFIG_CLK_GATE_CNTRL_TIMER_VAL_FIELD_WIDTH,
    DMA_CONFIG_CLK_GATE_CNTRL_TIMER_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_EN
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD =
{
    "KEEP_ALIVE_EN",
#if RU_INCLUDE_DESC
    "KEEP_ALIVE_ENABLE",
    "Enables the keep alive logic which will periodically enable the clock to assure that no deadlock of clock being removed completely will occur",
#endif
    DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_MASK,
    0,
    DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_WIDTH,
    DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_CLK_GATE_CNTRL_RESERVED1
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_CLK_GATE_CNTRL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DMA_CONFIG_CLK_GATE_CNTRL_RESERVED1_FIELD_MASK,
    0,
    DMA_CONFIG_CLK_GATE_CNTRL_RESERVED1_FIELD_WIDTH,
    DMA_CONFIG_CLK_GATE_CNTRL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD =
{
    "KEEP_ALIVE_INTRVL",
#if RU_INCLUDE_DESC
    "KEEP_ALIVE_INTERVAL",
    "If the KEEP alive option is enabled the field will determine for how many cycles should the clock be active",
#endif
    DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_MASK,
    0,
    DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_WIDTH,
    DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_CLK_GATE_CNTRL_RESERVED2
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_CLK_GATE_CNTRL_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DMA_CONFIG_CLK_GATE_CNTRL_RESERVED2_FIELD_MASK,
    0,
    DMA_CONFIG_CLK_GATE_CNTRL_RESERVED2_FIELD_WIDTH,
    DMA_CONFIG_CLK_GATE_CNTRL_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_CYC
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD =
{
    "KEEP_ALIVE_CYC",
#if RU_INCLUDE_DESC
    "KEEP_ALIVE_CYCLE",
    "If the KEEP alive option is enabled this field will determine for how many cycles should the clock be disabled (minus the KEEP_ALIVE_INTERVAL)"
    ""
    "So KEEP_ALIVE_CYCLE must be larger than KEEP_ALIVE_INTERVAL.",
#endif
    DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_MASK,
    0,
    DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_WIDTH,
    DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_DBG_SEL_DBGSEL
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_DBG_SEL_DBGSEL_FIELD =
{
    "DBGSEL",
#if RU_INCLUDE_DESC
    "select",
    "select",
#endif
    DMA_CONFIG_DBG_SEL_DBGSEL_FIELD_MASK,
    0,
    DMA_CONFIG_DBG_SEL_DBGSEL_FIELD_WIDTH,
    DMA_CONFIG_DBG_SEL_DBGSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_DBG_SEL_RESERVED0
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_DBG_SEL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DMA_CONFIG_DBG_SEL_RESERVED0_FIELD_MASK,
    0,
    DMA_CONFIG_DBG_SEL_RESERVED0_FIELD_WIDTH,
    DMA_CONFIG_DBG_SEL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_DEBUG_NEMPTY_NEMPTY
 ******************************************************************************/
const ru_field_rec DMA_DEBUG_NEMPTY_NEMPTY_FIELD =
{
    "NEMPTY",
#if RU_INCLUDE_DESC
    "not_empty_indications",
    "indication of the queue state",
#endif
    DMA_DEBUG_NEMPTY_NEMPTY_FIELD_MASK,
    0,
    DMA_DEBUG_NEMPTY_NEMPTY_FIELD_WIDTH,
    DMA_DEBUG_NEMPTY_NEMPTY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DMA_DEBUG_NEMPTY_RESERVED0
 ******************************************************************************/
const ru_field_rec DMA_DEBUG_NEMPTY_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DMA_DEBUG_NEMPTY_RESERVED0_FIELD_MASK,
    0,
    DMA_DEBUG_NEMPTY_RESERVED0_FIELD_WIDTH,
    DMA_DEBUG_NEMPTY_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DMA_DEBUG_URGNT_URGNT
 ******************************************************************************/
const ru_field_rec DMA_DEBUG_URGNT_URGNT_FIELD =
{
    "URGNT",
#if RU_INCLUDE_DESC
    "urgent",
    "indication whether the queue is in urgent state or not",
#endif
    DMA_DEBUG_URGNT_URGNT_FIELD_MASK,
    0,
    DMA_DEBUG_URGNT_URGNT_FIELD_WIDTH,
    DMA_DEBUG_URGNT_URGNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DMA_DEBUG_URGNT_RESERVED0
 ******************************************************************************/
const ru_field_rec DMA_DEBUG_URGNT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DMA_DEBUG_URGNT_RESERVED0_FIELD_MASK,
    0,
    DMA_DEBUG_URGNT_RESERVED0_FIELD_WIDTH,
    DMA_DEBUG_URGNT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DMA_DEBUG_SELSRC_SEL_SRC
 ******************************************************************************/
const ru_field_rec DMA_DEBUG_SELSRC_SEL_SRC_FIELD =
{
    "SEL_SRC",
#if RU_INCLUDE_DESC
    "selected_source",
    "the next peripheral to be served by the dma",
#endif
    DMA_DEBUG_SELSRC_SEL_SRC_FIELD_MASK,
    0,
    DMA_DEBUG_SELSRC_SEL_SRC_FIELD_WIDTH,
    DMA_DEBUG_SELSRC_SEL_SRC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DMA_DEBUG_SELSRC_RESERVED0
 ******************************************************************************/
const ru_field_rec DMA_DEBUG_SELSRC_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DMA_DEBUG_SELSRC_RESERVED0_FIELD_MASK,
    0,
    DMA_DEBUG_SELSRC_RESERVED0_FIELD_WIDTH,
    DMA_DEBUG_SELSRC_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DMA_DEBUG_REQ_CNT_RX_REQ_CNT
 ******************************************************************************/
const ru_field_rec DMA_DEBUG_REQ_CNT_RX_REQ_CNT_FIELD =
{
    "REQ_CNT",
#if RU_INCLUDE_DESC
    "write_requests_counter",
    "the number of pending write requests",
#endif
    DMA_DEBUG_REQ_CNT_RX_REQ_CNT_FIELD_MASK,
    0,
    DMA_DEBUG_REQ_CNT_RX_REQ_CNT_FIELD_WIDTH,
    DMA_DEBUG_REQ_CNT_RX_REQ_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DMA_DEBUG_REQ_CNT_RX_RESERVED0
 ******************************************************************************/
const ru_field_rec DMA_DEBUG_REQ_CNT_RX_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DMA_DEBUG_REQ_CNT_RX_RESERVED0_FIELD_MASK,
    0,
    DMA_DEBUG_REQ_CNT_RX_RESERVED0_FIELD_WIDTH,
    DMA_DEBUG_REQ_CNT_RX_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DMA_DEBUG_REQ_CNT_TX_REQ_CNT
 ******************************************************************************/
const ru_field_rec DMA_DEBUG_REQ_CNT_TX_REQ_CNT_FIELD =
{
    "REQ_CNT",
#if RU_INCLUDE_DESC
    "read_requests_counter",
    "the number of pending read requests",
#endif
    DMA_DEBUG_REQ_CNT_TX_REQ_CNT_FIELD_MASK,
    0,
    DMA_DEBUG_REQ_CNT_TX_REQ_CNT_FIELD_WIDTH,
    DMA_DEBUG_REQ_CNT_TX_REQ_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DMA_DEBUG_REQ_CNT_TX_RESERVED0
 ******************************************************************************/
const ru_field_rec DMA_DEBUG_REQ_CNT_TX_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DMA_DEBUG_REQ_CNT_TX_RESERVED0_FIELD_MASK,
    0,
    DMA_DEBUG_REQ_CNT_TX_RESERVED0_FIELD_WIDTH,
    DMA_DEBUG_REQ_CNT_TX_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DMA_DEBUG_REQ_CNT_RX_ACC_REQ_CNT
 ******************************************************************************/
const ru_field_rec DMA_DEBUG_REQ_CNT_RX_ACC_REQ_CNT_FIELD =
{
    "REQ_CNT",
#if RU_INCLUDE_DESC
    "write_requests_counter",
    "the number of pending write requests",
#endif
    DMA_DEBUG_REQ_CNT_RX_ACC_REQ_CNT_FIELD_MASK,
    0,
    DMA_DEBUG_REQ_CNT_RX_ACC_REQ_CNT_FIELD_WIDTH,
    DMA_DEBUG_REQ_CNT_RX_ACC_REQ_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DMA_DEBUG_REQ_CNT_TX_ACC_REQ_CNT
 ******************************************************************************/
const ru_field_rec DMA_DEBUG_REQ_CNT_TX_ACC_REQ_CNT_FIELD =
{
    "REQ_CNT",
#if RU_INCLUDE_DESC
    "write_requests_counter",
    "the number of pending write requests",
#endif
    DMA_DEBUG_REQ_CNT_TX_ACC_REQ_CNT_FIELD_MASK,
    0,
    DMA_DEBUG_REQ_CNT_TX_ACC_REQ_CNT_FIELD_WIDTH,
    DMA_DEBUG_REQ_CNT_TX_ACC_REQ_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DMA_DEBUG_RDADD_ADDRESS
 ******************************************************************************/
const ru_field_rec DMA_DEBUG_RDADD_ADDRESS_FIELD =
{
    "ADDRESS",
#if RU_INCLUDE_DESC
    "address",
    "address within the ram",
#endif
    DMA_DEBUG_RDADD_ADDRESS_FIELD_MASK,
    0,
    DMA_DEBUG_RDADD_ADDRESS_FIELD_WIDTH,
    DMA_DEBUG_RDADD_ADDRESS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_DEBUG_RDADD_RESERVED0
 ******************************************************************************/
const ru_field_rec DMA_DEBUG_RDADD_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DMA_DEBUG_RDADD_RESERVED0_FIELD_MASK,
    0,
    DMA_DEBUG_RDADD_RESERVED0_FIELD_WIDTH,
    DMA_DEBUG_RDADD_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_DEBUG_RDADD_DATACS
 ******************************************************************************/
const ru_field_rec DMA_DEBUG_RDADD_DATACS_FIELD =
{
    "DATACS",
#if RU_INCLUDE_DESC
    "data_ram_cs",
    "chip select for write data ram",
#endif
    DMA_DEBUG_RDADD_DATACS_FIELD_MASK,
    0,
    DMA_DEBUG_RDADD_DATACS_FIELD_WIDTH,
    DMA_DEBUG_RDADD_DATACS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_DEBUG_RDADD_CDCS
 ******************************************************************************/
const ru_field_rec DMA_DEBUG_RDADD_CDCS_FIELD =
{
    "CDCS",
#if RU_INCLUDE_DESC
    "cd_ram_cs",
    "chip select for chunk descriptors ram",
#endif
    DMA_DEBUG_RDADD_CDCS_FIELD_MASK,
    0,
    DMA_DEBUG_RDADD_CDCS_FIELD_WIDTH,
    DMA_DEBUG_RDADD_CDCS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_DEBUG_RDADD_RRCS
 ******************************************************************************/
const ru_field_rec DMA_DEBUG_RDADD_RRCS_FIELD =
{
    "RRCS",
#if RU_INCLUDE_DESC
    "rr_ram_cd",
    "chip select for read requests ram",
#endif
    DMA_DEBUG_RDADD_RRCS_FIELD_MASK,
    0,
    DMA_DEBUG_RDADD_RRCS_FIELD_WIDTH,
    DMA_DEBUG_RDADD_RRCS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_DEBUG_RDADD_RESERVED1
 ******************************************************************************/
const ru_field_rec DMA_DEBUG_RDADD_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DMA_DEBUG_RDADD_RESERVED1_FIELD_MASK,
    0,
    DMA_DEBUG_RDADD_RESERVED1_FIELD_WIDTH,
    DMA_DEBUG_RDADD_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_DEBUG_RDVALID_VALID
 ******************************************************************************/
const ru_field_rec DMA_DEBUG_RDVALID_VALID_FIELD =
{
    "VALID",
#if RU_INCLUDE_DESC
    "valid",
    "indirect read request is valid",
#endif
    DMA_DEBUG_RDVALID_VALID_FIELD_MASK,
    0,
    DMA_DEBUG_RDVALID_VALID_FIELD_WIDTH,
    DMA_DEBUG_RDVALID_VALID_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_DEBUG_RDVALID_RESERVED0
 ******************************************************************************/
const ru_field_rec DMA_DEBUG_RDVALID_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DMA_DEBUG_RDVALID_RESERVED0_FIELD_MASK,
    0,
    DMA_DEBUG_RDVALID_RESERVED0_FIELD_WIDTH,
    DMA_DEBUG_RDVALID_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_DEBUG_RDDATA_DATA
 ******************************************************************************/
const ru_field_rec DMA_DEBUG_RDDATA_DATA_FIELD =
{
    "DATA",
#if RU_INCLUDE_DESC
    "data",
    "read data from ram",
#endif
    DMA_DEBUG_RDDATA_DATA_FIELD_MASK,
    0,
    DMA_DEBUG_RDDATA_DATA_FIELD_WIDTH,
    DMA_DEBUG_RDDATA_DATA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DMA_DEBUG_RDDATARDY_READY
 ******************************************************************************/
const ru_field_rec DMA_DEBUG_RDDATARDY_READY_FIELD =
{
    "READY",
#if RU_INCLUDE_DESC
    "ready",
    "read data ready",
#endif
    DMA_DEBUG_RDDATARDY_READY_FIELD_MASK,
    0,
    DMA_DEBUG_RDDATARDY_READY_FIELD_WIDTH,
    DMA_DEBUG_RDDATARDY_READY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DMA_DEBUG_RDDATARDY_RESERVED0
 ******************************************************************************/
const ru_field_rec DMA_DEBUG_RDDATARDY_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    DMA_DEBUG_RDDATARDY_RESERVED0_FIELD_MASK,
    0,
    DMA_DEBUG_RDDATARDY_RESERVED0_FIELD_WIDTH,
    DMA_DEBUG_RDDATARDY_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: DMA_CONFIG_BBROUTEOVRD
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_CONFIG_BBROUTEOVRD_FIELDS[] =
{
    &DMA_CONFIG_BBROUTEOVRD_DEST_FIELD,
    &DMA_CONFIG_BBROUTEOVRD_RESERVED0_FIELD,
    &DMA_CONFIG_BBROUTEOVRD_ROUTE_FIELD,
    &DMA_CONFIG_BBROUTEOVRD_RESERVED1_FIELD,
    &DMA_CONFIG_BBROUTEOVRD_OVRD_FIELD,
    &DMA_CONFIG_BBROUTEOVRD_RESERVED2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DMA_CONFIG_BBROUTEOVRD_REG = 
{
    "CONFIG_BBROUTEOVRD",
#if RU_INCLUDE_DESC
    "BB_ROUTE_OVERRIDE Register",
    "Broadbus route override",
#endif
    DMA_CONFIG_BBROUTEOVRD_REG_OFFSET,
    0,
    0,
    667,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    DMA_CONFIG_BBROUTEOVRD_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DMA_CONFIG_NUM_OF_WRITES
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_CONFIG_NUM_OF_WRITES_FIELDS[] =
{
    &DMA_CONFIG_NUM_OF_WRITES_NUMOFBUFF_FIELD,
    &DMA_CONFIG_NUM_OF_WRITES_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DMA_CONFIG_NUM_OF_WRITES_REG = 
{
    "CONFIG_NUM_OF_WRITES",
#if RU_INCLUDE_DESC
    "NUM_OF_WRITE_REQ %i Register",
    "This array of registers defines the memory allocation for the peripherals, for upstream."
    "The allocation is of number of 128byte buffers out of the total 48 buffers for both sdma and dma."
    ""
    "The allocation is done by defining a only the number of allocated buffers. base address is calculated by HW, when base of peripheral 0 is 0."
    "Note that the memory allocation should not contain wrap around."
    "The number of allocated CDs is the same of data buffers."
    "",
#endif
    DMA_CONFIG_NUM_OF_WRITES_REG_OFFSET,
    DMA_CONFIG_NUM_OF_WRITES_REG_RAM_CNT,
    4,
    668,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DMA_CONFIG_NUM_OF_WRITES_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DMA_CONFIG_NUM_OF_READS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_CONFIG_NUM_OF_READS_FIELDS[] =
{
    &DMA_CONFIG_NUM_OF_READS_RR_NUM_FIELD,
    &DMA_CONFIG_NUM_OF_READS_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DMA_CONFIG_NUM_OF_READS_REG = 
{
    "CONFIG_NUM_OF_READS",
#if RU_INCLUDE_DESC
    "NUM_OF_READ_REQ %i Register",
    "This array of registers controls the number of read requests of each peripheral within the read requests RAM."
    "total of 64 requests are divided between peripherals."
    "Base address of peripheral 0 is 0, base of peripheral 1 is 0 + periph0_num_of_read_requests and so on.",
#endif
    DMA_CONFIG_NUM_OF_READS_REG_OFFSET,
    DMA_CONFIG_NUM_OF_READS_REG_RAM_CNT,
    4,
    669,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DMA_CONFIG_NUM_OF_READS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DMA_CONFIG_U_THRESH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_CONFIG_U_THRESH_FIELDS[] =
{
    &DMA_CONFIG_U_THRESH_INTO_U_FIELD,
    &DMA_CONFIG_U_THRESH_RESERVED0_FIELD,
    &DMA_CONFIG_U_THRESH_OUT_OF_U_FIELD,
    &DMA_CONFIG_U_THRESH_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DMA_CONFIG_U_THRESH_REG = 
{
    "CONFIG_U_THRESH",
#if RU_INCLUDE_DESC
    "URGENT_THRESHOLDS %i Register",
    "the in/out of urgent thresholds mark the number of write requests in the queue in which the peripherals priority is changed. The two thresholds should create hysteresis."
    "The moving into urgent threshold must always be greater than the moving out of urgent threshold.",
#endif
    DMA_CONFIG_U_THRESH_REG_OFFSET,
    DMA_CONFIG_U_THRESH_REG_RAM_CNT,
    4,
    670,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    DMA_CONFIG_U_THRESH_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DMA_CONFIG_PRI
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_CONFIG_PRI_FIELDS[] =
{
    &DMA_CONFIG_PRI_RXPRI_FIELD,
    &DMA_CONFIG_PRI_TXPRI_FIELD,
    &DMA_CONFIG_PRI_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DMA_CONFIG_PRI_REG = 
{
    "CONFIG_PRI",
#if RU_INCLUDE_DESC
    "STRICT_PRIORITY %i Register",
    "The arbitration between the requests of the different peripherals is done in two stages:"
    "1. Strict priority - chooses the peripherals with the highest priority among all perpherals who have a request pending."
    "2. Weighted Round-Robin between all peripherals with the same priority."
    ""
    "This array of registers allow configuration of the priority of each peripheral (both rx and tx) in the following manner:"
    "There are 4 levels of priorities, when each bit in the register represents a different level of priority. One should assert the relevant bit according to the desired priority -"
    "For the lowest  - 0001"
    "For the highest - 1000",
#endif
    DMA_CONFIG_PRI_REG_OFFSET,
    DMA_CONFIG_PRI_REG_RAM_CNT,
    4,
    671,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    DMA_CONFIG_PRI_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DMA_CONFIG_PERIPH_SOURCE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_CONFIG_PERIPH_SOURCE_FIELDS[] =
{
    &DMA_CONFIG_PERIPH_SOURCE_RXSOURCE_FIELD,
    &DMA_CONFIG_PERIPH_SOURCE_RESERVED0_FIELD,
    &DMA_CONFIG_PERIPH_SOURCE_TXSOURCE_FIELD,
    &DMA_CONFIG_PERIPH_SOURCE_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DMA_CONFIG_PERIPH_SOURCE_REG = 
{
    "CONFIG_PERIPH_SOURCE",
#if RU_INCLUDE_DESC
    "BB_SOURCE_DMA_PERIPH %i Register",
    "Broadbus source address of the DMA peripherals. Register per peripheral (rx and tx). The source is used to determine the route address to the different peripherals.",
#endif
    DMA_CONFIG_PERIPH_SOURCE_REG_OFFSET,
    DMA_CONFIG_PERIPH_SOURCE_REG_RAM_CNT,
    4,
    672,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    DMA_CONFIG_PERIPH_SOURCE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DMA_CONFIG_WEIGHT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_CONFIG_WEIGHT_FIELDS[] =
{
    &DMA_CONFIG_WEIGHT_RXWEIGHT_FIELD,
    &DMA_CONFIG_WEIGHT_RESERVED0_FIELD,
    &DMA_CONFIG_WEIGHT_TXWEIGHT_FIELD,
    &DMA_CONFIG_WEIGHT_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DMA_CONFIG_WEIGHT_REG = 
{
    "CONFIG_WEIGHT",
#if RU_INCLUDE_DESC
    "WEIGHT_OF_ROUND_ROBIN %i Register",
    "The second phase of the arbitration between requests is weighted round robin between requests of peripherals with the same priority."
    "This array of registers allow configurtion of the weight of each peripheral (rx and tx). The actual weight will be weight + 1, meaning configuration of 0 is actual weight of 1.",
#endif
    DMA_CONFIG_WEIGHT_REG_OFFSET,
    DMA_CONFIG_WEIGHT_REG_RAM_CNT,
    4,
    673,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    DMA_CONFIG_WEIGHT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DMA_CONFIG_PTRRST
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_CONFIG_PTRRST_FIELDS[] =
{
    &DMA_CONFIG_PTRRST_RSTVEC_FIELD,
    &DMA_CONFIG_PTRRST_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DMA_CONFIG_PTRRST_REG = 
{
    "CONFIG_PTRRST",
#if RU_INCLUDE_DESC
    "POINTERS_RESET Register",
    "Resets the pointers of the peripherals FIFOs within the DMA. Bit per peripheral side (rx and tx)."
    "For rx side resets the data and CD FIFOs."
    "For tx side resets the read requests FIFO.",
#endif
    DMA_CONFIG_PTRRST_REG_OFFSET,
    0,
    0,
    674,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DMA_CONFIG_PTRRST_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DMA_CONFIG_MAX_OTF
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_CONFIG_MAX_OTF_FIELDS[] =
{
    &DMA_CONFIG_MAX_OTF_MAX_FIELD,
    &DMA_CONFIG_MAX_OTF_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DMA_CONFIG_MAX_OTF_REG = 
{
    "CONFIG_MAX_OTF",
#if RU_INCLUDE_DESC
    "MAX_ON_THE_FLY Register",
    "max number of on the fly read commands the DMA may issue to DDR before receiving any data.",
#endif
    DMA_CONFIG_MAX_OTF_REG_OFFSET,
    0,
    0,
    675,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DMA_CONFIG_MAX_OTF_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DMA_CONFIG_CLK_GATE_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_CONFIG_CLK_GATE_CNTRL_FIELDS[] =
{
    &DMA_CONFIG_CLK_GATE_CNTRL_BYPASS_CLK_GATE_FIELD,
    &DMA_CONFIG_CLK_GATE_CNTRL_RESERVED0_FIELD,
    &DMA_CONFIG_CLK_GATE_CNTRL_TIMER_VAL_FIELD,
    &DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_EN_FIELD,
    &DMA_CONFIG_CLK_GATE_CNTRL_RESERVED1_FIELD,
    &DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_INTRVL_FIELD,
    &DMA_CONFIG_CLK_GATE_CNTRL_RESERVED2_FIELD,
    &DMA_CONFIG_CLK_GATE_CNTRL_KEEP_ALIVE_CYC_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DMA_CONFIG_CLK_GATE_CNTRL_REG = 
{
    "CONFIG_CLK_GATE_CNTRL",
#if RU_INCLUDE_DESC
    "CLOCK_GATE_CONTROL Register",
    "Clock Gate control register including timer config and bypass control",
#endif
    DMA_CONFIG_CLK_GATE_CNTRL_REG_OFFSET,
    0,
    0,
    676,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    DMA_CONFIG_CLK_GATE_CNTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DMA_CONFIG_DBG_SEL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_CONFIG_DBG_SEL_FIELDS[] =
{
    &DMA_CONFIG_DBG_SEL_DBGSEL_FIELD,
    &DMA_CONFIG_DBG_SEL_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DMA_CONFIG_DBG_SEL_REG = 
{
    "CONFIG_DBG_SEL",
#if RU_INCLUDE_DESC
    "DBG_SEL Register",
    "debug bus select",
#endif
    DMA_CONFIG_DBG_SEL_REG_OFFSET,
    0,
    0,
    677,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DMA_CONFIG_DBG_SEL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DMA_DEBUG_NEMPTY
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_DEBUG_NEMPTY_FIELDS[] =
{
    &DMA_DEBUG_NEMPTY_NEMPTY_FIELD,
    &DMA_DEBUG_NEMPTY_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DMA_DEBUG_NEMPTY_REG = 
{
    "DEBUG_NEMPTY",
#if RU_INCLUDE_DESC
    "NOT_EMPTY_VECTOR Register",
    "Each peripheral is represented in a bit on the not empty vector."
    "LSB is for rx peripherals, MSB for tx peripherals."
    "If the bit is asserted, the requests queue of the relevant peripheral is not empty."
    "The not empty vector is used by the DMA scheduler to determine which peripheral is the next to be served.",
#endif
    DMA_DEBUG_NEMPTY_REG_OFFSET,
    0,
    0,
    678,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DMA_DEBUG_NEMPTY_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DMA_DEBUG_URGNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_DEBUG_URGNT_FIELDS[] =
{
    &DMA_DEBUG_URGNT_URGNT_FIELD,
    &DMA_DEBUG_URGNT_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DMA_DEBUG_URGNT_REG = 
{
    "DEBUG_URGNT",
#if RU_INCLUDE_DESC
    "URGENT_VECTOR Register",
    "Each peripheral, a is represented in a bit on the urgent vector. 8 LSB are rx peripherlas, 8 MSB are tx peripherals."
    "If the bit is asserted, the requests queue of the relevant peripheral is in urgent state."
    "The urgent vector is used by the DMA scheduler to determine which peripheral is the next to be served.",
#endif
    DMA_DEBUG_URGNT_REG_OFFSET,
    0,
    0,
    679,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DMA_DEBUG_URGNT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DMA_DEBUG_SELSRC
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_DEBUG_SELSRC_FIELDS[] =
{
    &DMA_DEBUG_SELSRC_SEL_SRC_FIELD,
    &DMA_DEBUG_SELSRC_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DMA_DEBUG_SELSRC_REG = 
{
    "DEBUG_SELSRC",
#if RU_INCLUDE_DESC
    "SELECTED_SOURCE_NUM Register",
    "The decision of the dma schedule rand the next peripheral to be served, represented by its source address",
#endif
    DMA_DEBUG_SELSRC_REG_OFFSET,
    0,
    0,
    680,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DMA_DEBUG_SELSRC_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DMA_DEBUG_REQ_CNT_RX
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_DEBUG_REQ_CNT_RX_FIELDS[] =
{
    &DMA_DEBUG_REQ_CNT_RX_REQ_CNT_FIELD,
    &DMA_DEBUG_REQ_CNT_RX_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DMA_DEBUG_REQ_CNT_RX_REG = 
{
    "DEBUG_REQ_CNT_RX",
#if RU_INCLUDE_DESC
    "REQUEST_COUNTERS_RX %i Register",
    "the number of write requests currently pending for each rx peripheral.",
#endif
    DMA_DEBUG_REQ_CNT_RX_REG_OFFSET,
    DMA_DEBUG_REQ_CNT_RX_REG_RAM_CNT,
    4,
    681,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DMA_DEBUG_REQ_CNT_RX_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DMA_DEBUG_REQ_CNT_TX
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_DEBUG_REQ_CNT_TX_FIELDS[] =
{
    &DMA_DEBUG_REQ_CNT_TX_REQ_CNT_FIELD,
    &DMA_DEBUG_REQ_CNT_TX_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DMA_DEBUG_REQ_CNT_TX_REG = 
{
    "DEBUG_REQ_CNT_TX",
#if RU_INCLUDE_DESC
    "REQUEST_COUNTERS_TX %i Register",
    "the number of read requestscurrently pending for each TX peripheral.",
#endif
    DMA_DEBUG_REQ_CNT_TX_REG_OFFSET,
    DMA_DEBUG_REQ_CNT_TX_REG_RAM_CNT,
    4,
    682,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DMA_DEBUG_REQ_CNT_TX_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DMA_DEBUG_REQ_CNT_RX_ACC
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_DEBUG_REQ_CNT_RX_ACC_FIELDS[] =
{
    &DMA_DEBUG_REQ_CNT_RX_ACC_REQ_CNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DMA_DEBUG_REQ_CNT_RX_ACC_REG = 
{
    "DEBUG_REQ_CNT_RX_ACC",
#if RU_INCLUDE_DESC
    "ACC_REQUEST_COUNTERS_RX %i Register",
    "the accumulated number of write requests served so far for each peripheral. Wrap around on max value, not read clear.",
#endif
    DMA_DEBUG_REQ_CNT_RX_ACC_REG_OFFSET,
    DMA_DEBUG_REQ_CNT_RX_ACC_REG_RAM_CNT,
    4,
    683,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DMA_DEBUG_REQ_CNT_RX_ACC_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DMA_DEBUG_REQ_CNT_TX_ACC
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_DEBUG_REQ_CNT_TX_ACC_FIELDS[] =
{
    &DMA_DEBUG_REQ_CNT_TX_ACC_REQ_CNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DMA_DEBUG_REQ_CNT_TX_ACC_REG = 
{
    "DEBUG_REQ_CNT_TX_ACC",
#if RU_INCLUDE_DESC
    "ACC_REQUEST_COUNTERS_TX %i Register",
    "the accumulated number of read requests served so far for each peripheral. Wrap around on max value, not read clear.",
#endif
    DMA_DEBUG_REQ_CNT_TX_ACC_REG_OFFSET,
    DMA_DEBUG_REQ_CNT_TX_ACC_REG_RAM_CNT,
    4,
    684,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DMA_DEBUG_REQ_CNT_TX_ACC_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DMA_DEBUG_RDADD
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_DEBUG_RDADD_FIELDS[] =
{
    &DMA_DEBUG_RDADD_ADDRESS_FIELD,
    &DMA_DEBUG_RDADD_RESERVED0_FIELD,
    &DMA_DEBUG_RDADD_DATACS_FIELD,
    &DMA_DEBUG_RDADD_CDCS_FIELD,
    &DMA_DEBUG_RDADD_RRCS_FIELD,
    &DMA_DEBUG_RDADD_RESERVED1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DMA_DEBUG_RDADD_REG = 
{
    "DEBUG_RDADD",
#if RU_INCLUDE_DESC
    "RAM_ADDRES Register",
    "the address and cs of the ram the user wishes to read using the indirect access read mechanism.",
#endif
    DMA_DEBUG_RDADD_REG_OFFSET,
    0,
    0,
    685,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    DMA_DEBUG_RDADD_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DMA_DEBUG_RDVALID
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_DEBUG_RDVALID_FIELDS[] =
{
    &DMA_DEBUG_RDVALID_VALID_FIELD,
    &DMA_DEBUG_RDVALID_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DMA_DEBUG_RDVALID_REG = 
{
    "DEBUG_RDVALID",
#if RU_INCLUDE_DESC
    "INDIRECT_READ_REQUEST_VALID Register",
    "After determining the address and cs, the user should assert this bit for indicating that the address and cs are valid.",
#endif
    DMA_DEBUG_RDVALID_REG_OFFSET,
    0,
    0,
    686,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DMA_DEBUG_RDVALID_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DMA_DEBUG_RDDATA
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_DEBUG_RDDATA_FIELDS[] =
{
    &DMA_DEBUG_RDDATA_DATA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DMA_DEBUG_RDDATA_REG = 
{
    "DEBUG_RDDATA",
#if RU_INCLUDE_DESC
    "INDIRECT_READ_DATA %i Register",
    "The returned read data from the selected RAM. Array of 4 registers (128 bits total)."
    "The width of the different memories is as follows:"
    "write data - 128 bits"
    "chunk descriptors - 36 bits"
    "read requests - 42 bits"
    "read data - 64 bits"
    ""
    "The the memories with width smaller than 128, the data will appear in the first registers of the array, for example:"
    "data from the cd RAM will appear in - {reg1[5:0], reg0[31:0]}.",
#endif
    DMA_DEBUG_RDDATA_REG_OFFSET,
    DMA_DEBUG_RDDATA_REG_RAM_CNT,
    4,
    687,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    DMA_DEBUG_RDDATA_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DMA_DEBUG_RDDATARDY
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_DEBUG_RDDATARDY_FIELDS[] =
{
    &DMA_DEBUG_RDDATARDY_READY_FIELD,
    &DMA_DEBUG_RDDATARDY_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DMA_DEBUG_RDDATARDY_REG = 
{
    "DEBUG_RDDATARDY",
#if RU_INCLUDE_DESC
    "READ_DATA_READY Register",
    "When assertd indicats that the data in the previous array is valid.Willremain asserted until the user deasserts the valid bit in regiser RDVALID.",
#endif
    DMA_DEBUG_RDDATARDY_REG_OFFSET,
    0,
    0,
    688,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DMA_DEBUG_RDDATARDY_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Block: DMA
 ******************************************************************************/
static const ru_reg_rec *DMA_REGS[] =
{
    &DMA_CONFIG_BBROUTEOVRD_REG,
    &DMA_CONFIG_NUM_OF_WRITES_REG,
    &DMA_CONFIG_NUM_OF_READS_REG,
    &DMA_CONFIG_U_THRESH_REG,
    &DMA_CONFIG_PRI_REG,
    &DMA_CONFIG_PERIPH_SOURCE_REG,
    &DMA_CONFIG_WEIGHT_REG,
    &DMA_CONFIG_PTRRST_REG,
    &DMA_CONFIG_MAX_OTF_REG,
    &DMA_CONFIG_CLK_GATE_CNTRL_REG,
    &DMA_CONFIG_DBG_SEL_REG,
    &DMA_DEBUG_NEMPTY_REG,
    &DMA_DEBUG_URGNT_REG,
    &DMA_DEBUG_SELSRC_REG,
    &DMA_DEBUG_REQ_CNT_RX_REG,
    &DMA_DEBUG_REQ_CNT_TX_REG,
    &DMA_DEBUG_REQ_CNT_RX_ACC_REG,
    &DMA_DEBUG_REQ_CNT_TX_ACC_REG,
    &DMA_DEBUG_RDADD_REG,
    &DMA_DEBUG_RDVALID_REG,
    &DMA_DEBUG_RDDATA_REG,
    &DMA_DEBUG_RDDATARDY_REG,
};

unsigned long DMA_ADDRS[] =
{
    0x82d2c800,
    0x82d2cc00,
    0x82d2d000,
    0x82d2d400,
};

const ru_block_rec DMA_BLOCK = 
{
    "DMA",
    DMA_ADDRS,
    4,
    22,
    DMA_REGS
};

/* End of file XRDP_DMA.c */
