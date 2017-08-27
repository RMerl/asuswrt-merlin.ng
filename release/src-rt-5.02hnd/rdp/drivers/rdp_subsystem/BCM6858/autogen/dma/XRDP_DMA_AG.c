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
 * Field: DMA_CONFIG_PTRRST_ETH0RX
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_PTRRST_ETH0RX_FIELD =
{
    "ETH0RX",
#if RU_INCLUDE_DESC
    "ethernet_0_rx_reset",
    "resets the pointers of ethernet 0 rx",
#endif
    DMA_CONFIG_PTRRST_ETH0RX_FIELD_MASK,
    0,
    DMA_CONFIG_PTRRST_ETH0RX_FIELD_WIDTH,
    DMA_CONFIG_PTRRST_ETH0RX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_PTRRST_ETH0TX
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_PTRRST_ETH0TX_FIELD =
{
    "ETH0TX",
#if RU_INCLUDE_DESC
    "ethernet_0_tx_reset",
    "resets the pointers of ethernet 0 tx",
#endif
    DMA_CONFIG_PTRRST_ETH0TX_FIELD_MASK,
    0,
    DMA_CONFIG_PTRRST_ETH0TX_FIELD_WIDTH,
    DMA_CONFIG_PTRRST_ETH0TX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_PTRRST_ETH1RX
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_PTRRST_ETH1RX_FIELD =
{
    "ETH1RX",
#if RU_INCLUDE_DESC
    "ethernet_1_rx_reset",
    "resets the pointers of ethernet 1 rx",
#endif
    DMA_CONFIG_PTRRST_ETH1RX_FIELD_MASK,
    0,
    DMA_CONFIG_PTRRST_ETH1RX_FIELD_WIDTH,
    DMA_CONFIG_PTRRST_ETH1RX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_PTRRST_ETH1TX
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_PTRRST_ETH1TX_FIELD =
{
    "ETH1TX",
#if RU_INCLUDE_DESC
    "ethernet_1_tx_reset",
    "resets the pointers of ethernet 1 tx",
#endif
    DMA_CONFIG_PTRRST_ETH1TX_FIELD_MASK,
    0,
    DMA_CONFIG_PTRRST_ETH1TX_FIELD_WIDTH,
    DMA_CONFIG_PTRRST_ETH1TX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_PTRRST_ETH2RX
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_PTRRST_ETH2RX_FIELD =
{
    "ETH2RX",
#if RU_INCLUDE_DESC
    "ethernet_2_rx_reset",
    "resets the pointers of ethernet 2 rx",
#endif
    DMA_CONFIG_PTRRST_ETH2RX_FIELD_MASK,
    0,
    DMA_CONFIG_PTRRST_ETH2RX_FIELD_WIDTH,
    DMA_CONFIG_PTRRST_ETH2RX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_PTRRST_ETH2TX
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_PTRRST_ETH2TX_FIELD =
{
    "ETH2TX",
#if RU_INCLUDE_DESC
    "ethernet_2_tx_reset",
    "resets the pointers of ethernet 2 tx",
#endif
    DMA_CONFIG_PTRRST_ETH2TX_FIELD_MASK,
    0,
    DMA_CONFIG_PTRRST_ETH2TX_FIELD_WIDTH,
    DMA_CONFIG_PTRRST_ETH2TX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_PTRRST_ETH3RX
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_PTRRST_ETH3RX_FIELD =
{
    "ETH3RX",
#if RU_INCLUDE_DESC
    "ethernet_3_rx_reset",
    "resets the pointers of ethernet 3 rx",
#endif
    DMA_CONFIG_PTRRST_ETH3RX_FIELD_MASK,
    0,
    DMA_CONFIG_PTRRST_ETH3RX_FIELD_WIDTH,
    DMA_CONFIG_PTRRST_ETH3RX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_PTRRST_ETH3TX
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_PTRRST_ETH3TX_FIELD =
{
    "ETH3TX",
#if RU_INCLUDE_DESC
    "ethernet_3_tx_reset",
    "resets the pointers of ethernet 3 tx",
#endif
    DMA_CONFIG_PTRRST_ETH3TX_FIELD_MASK,
    0,
    DMA_CONFIG_PTRRST_ETH3TX_FIELD_WIDTH,
    DMA_CONFIG_PTRRST_ETH3TX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_PTRRST_ETH4RX
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_PTRRST_ETH4RX_FIELD =
{
    "ETH4RX",
#if RU_INCLUDE_DESC
    "ethernet_4_rx_reset",
    "resets the pointers of ethernet 4 rx",
#endif
    DMA_CONFIG_PTRRST_ETH4RX_FIELD_MASK,
    0,
    DMA_CONFIG_PTRRST_ETH4RX_FIELD_WIDTH,
    DMA_CONFIG_PTRRST_ETH4RX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_PTRRST_ETH4TX
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_PTRRST_ETH4TX_FIELD =
{
    "ETH4TX",
#if RU_INCLUDE_DESC
    "ethernet_4_tx_reset",
    "resets the pointers of ethernet 4 tx",
#endif
    DMA_CONFIG_PTRRST_ETH4TX_FIELD_MASK,
    0,
    DMA_CONFIG_PTRRST_ETH4TX_FIELD_WIDTH,
    DMA_CONFIG_PTRRST_ETH4TX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_PTRRST_GPONRX
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_PTRRST_GPONRX_FIELD =
{
    "GPONRX",
#if RU_INCLUDE_DESC
    "gpon_rx_reset",
    "resets the pointers of gpon rx",
#endif
    DMA_CONFIG_PTRRST_GPONRX_FIELD_MASK,
    0,
    DMA_CONFIG_PTRRST_GPONRX_FIELD_WIDTH,
    DMA_CONFIG_PTRRST_GPONRX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: DMA_CONFIG_PTRRST_GPONTX
 ******************************************************************************/
const ru_field_rec DMA_CONFIG_PTRRST_GPONTX_FIELD =
{
    "GPONTX",
#if RU_INCLUDE_DESC
    "gpon_tx_reset",
    "resets the pointers of gpon tx",
#endif
    DMA_CONFIG_PTRRST_GPONTX_FIELD_MASK,
    0,
    DMA_CONFIG_PTRRST_GPONTX_FIELD_WIDTH,
    DMA_CONFIG_PTRRST_GPONTX_FIELD_SHIFT,
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
 * Field: DMA_DEBUG_NEMPTY_ETH0RXNE
 ******************************************************************************/
const ru_field_rec DMA_DEBUG_NEMPTY_ETH0RXNE_FIELD =
{
    "ETH0RXNE",
#if RU_INCLUDE_DESC
    "Ethernet0_RX_not_empty_indications",
    "indication of the queue state",
#endif
    DMA_DEBUG_NEMPTY_ETH0RXNE_FIELD_MASK,
    0,
    DMA_DEBUG_NEMPTY_ETH0RXNE_FIELD_WIDTH,
    DMA_DEBUG_NEMPTY_ETH0RXNE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DMA_DEBUG_NEMPTY_ETH1RXNE
 ******************************************************************************/
const ru_field_rec DMA_DEBUG_NEMPTY_ETH1RXNE_FIELD =
{
    "ETH1RXNE",
#if RU_INCLUDE_DESC
    "Ethernet1_RX_not_empty_indications",
    "indication of the queue state",
#endif
    DMA_DEBUG_NEMPTY_ETH1RXNE_FIELD_MASK,
    0,
    DMA_DEBUG_NEMPTY_ETH1RXNE_FIELD_WIDTH,
    DMA_DEBUG_NEMPTY_ETH1RXNE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DMA_DEBUG_NEMPTY_ETH2RXNE
 ******************************************************************************/
const ru_field_rec DMA_DEBUG_NEMPTY_ETH2RXNE_FIELD =
{
    "ETH2RXNE",
#if RU_INCLUDE_DESC
    "Ethernet2_RX_not_empty_indications",
    "indication of the queue state",
#endif
    DMA_DEBUG_NEMPTY_ETH2RXNE_FIELD_MASK,
    0,
    DMA_DEBUG_NEMPTY_ETH2RXNE_FIELD_WIDTH,
    DMA_DEBUG_NEMPTY_ETH2RXNE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DMA_DEBUG_NEMPTY_ETH3RXNE
 ******************************************************************************/
const ru_field_rec DMA_DEBUG_NEMPTY_ETH3RXNE_FIELD =
{
    "ETH3RXNE",
#if RU_INCLUDE_DESC
    "Ethernet3_RX_not_empty_indications",
    "indication of the queue state",
#endif
    DMA_DEBUG_NEMPTY_ETH3RXNE_FIELD_MASK,
    0,
    DMA_DEBUG_NEMPTY_ETH3RXNE_FIELD_WIDTH,
    DMA_DEBUG_NEMPTY_ETH3RXNE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DMA_DEBUG_NEMPTY_ETH4RXNE
 ******************************************************************************/
const ru_field_rec DMA_DEBUG_NEMPTY_ETH4RXNE_FIELD =
{
    "ETH4RXNE",
#if RU_INCLUDE_DESC
    "Ethernet4_RX_not_empty_indications",
    "indication of the queue state",
#endif
    DMA_DEBUG_NEMPTY_ETH4RXNE_FIELD_MASK,
    0,
    DMA_DEBUG_NEMPTY_ETH4RXNE_FIELD_WIDTH,
    DMA_DEBUG_NEMPTY_ETH4RXNE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DMA_DEBUG_NEMPTY_GPONRXNE
 ******************************************************************************/
const ru_field_rec DMA_DEBUG_NEMPTY_GPONRXNE_FIELD =
{
    "GPONRXNE",
#if RU_INCLUDE_DESC
    "GPON_RX_not_empty_indications",
    "indication of the queue state",
#endif
    DMA_DEBUG_NEMPTY_GPONRXNE_FIELD_MASK,
    0,
    DMA_DEBUG_NEMPTY_GPONRXNE_FIELD_WIDTH,
    DMA_DEBUG_NEMPTY_GPONRXNE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DMA_DEBUG_NEMPTY_ETH0TXNE
 ******************************************************************************/
const ru_field_rec DMA_DEBUG_NEMPTY_ETH0TXNE_FIELD =
{
    "ETH0TXNE",
#if RU_INCLUDE_DESC
    "Ethernet0_TX_not_empty_indications",
    "indication of the queue state",
#endif
    DMA_DEBUG_NEMPTY_ETH0TXNE_FIELD_MASK,
    0,
    DMA_DEBUG_NEMPTY_ETH0TXNE_FIELD_WIDTH,
    DMA_DEBUG_NEMPTY_ETH0TXNE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DMA_DEBUG_NEMPTY_ETH1TXNE
 ******************************************************************************/
const ru_field_rec DMA_DEBUG_NEMPTY_ETH1TXNE_FIELD =
{
    "ETH1TXNE",
#if RU_INCLUDE_DESC
    "Ethernet1_TX_not_empty_indications",
    "indication of the queue state",
#endif
    DMA_DEBUG_NEMPTY_ETH1TXNE_FIELD_MASK,
    0,
    DMA_DEBUG_NEMPTY_ETH1TXNE_FIELD_WIDTH,
    DMA_DEBUG_NEMPTY_ETH1TXNE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DMA_DEBUG_NEMPTY_ETH2TXNE
 ******************************************************************************/
const ru_field_rec DMA_DEBUG_NEMPTY_ETH2TXNE_FIELD =
{
    "ETH2TXNE",
#if RU_INCLUDE_DESC
    "Ethernet2_TX_not_empty_indications",
    "indication of the queue state",
#endif
    DMA_DEBUG_NEMPTY_ETH2TXNE_FIELD_MASK,
    0,
    DMA_DEBUG_NEMPTY_ETH2TXNE_FIELD_WIDTH,
    DMA_DEBUG_NEMPTY_ETH2TXNE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DMA_DEBUG_NEMPTY_ETH3TXNE
 ******************************************************************************/
const ru_field_rec DMA_DEBUG_NEMPTY_ETH3TXNE_FIELD =
{
    "ETH3TXNE",
#if RU_INCLUDE_DESC
    "Ethernet3_TX_not_empty_indications",
    "indication of the queue state",
#endif
    DMA_DEBUG_NEMPTY_ETH3TXNE_FIELD_MASK,
    0,
    DMA_DEBUG_NEMPTY_ETH3TXNE_FIELD_WIDTH,
    DMA_DEBUG_NEMPTY_ETH3TXNE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DMA_DEBUG_NEMPTY_ETH4TXNE
 ******************************************************************************/
const ru_field_rec DMA_DEBUG_NEMPTY_ETH4TXNE_FIELD =
{
    "ETH4TXNE",
#if RU_INCLUDE_DESC
    "Ethernet4_TX_not_empty_indications",
    "indication of the queue state",
#endif
    DMA_DEBUG_NEMPTY_ETH4TXNE_FIELD_MASK,
    0,
    DMA_DEBUG_NEMPTY_ETH4TXNE_FIELD_WIDTH,
    DMA_DEBUG_NEMPTY_ETH4TXNE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DMA_DEBUG_NEMPTY_GPONTXNE
 ******************************************************************************/
const ru_field_rec DMA_DEBUG_NEMPTY_GPONTXNE_FIELD =
{
    "GPONTXNE",
#if RU_INCLUDE_DESC
    "GPON_TX_not_empty_indications",
    "indication of the queue state",
#endif
    DMA_DEBUG_NEMPTY_GPONTXNE_FIELD_MASK,
    0,
    DMA_DEBUG_NEMPTY_GPONTXNE_FIELD_WIDTH,
    DMA_DEBUG_NEMPTY_GPONTXNE_FIELD_SHIFT,
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
 * Field: DMA_DEBUG_URGNT_ETH0RXU
 ******************************************************************************/
const ru_field_rec DMA_DEBUG_URGNT_ETH0RXU_FIELD =
{
    "ETH0RXU",
#if RU_INCLUDE_DESC
    "Ethernet0_RX_urgent_indication",
    "indication whether the queue is in urgent state or not",
#endif
    DMA_DEBUG_URGNT_ETH0RXU_FIELD_MASK,
    0,
    DMA_DEBUG_URGNT_ETH0RXU_FIELD_WIDTH,
    DMA_DEBUG_URGNT_ETH0RXU_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DMA_DEBUG_URGNT_ETH1RXU
 ******************************************************************************/
const ru_field_rec DMA_DEBUG_URGNT_ETH1RXU_FIELD =
{
    "ETH1RXU",
#if RU_INCLUDE_DESC
    "Ethernet1_RX_urgent_indication",
    "indication whether the queue is in urgent state or not",
#endif
    DMA_DEBUG_URGNT_ETH1RXU_FIELD_MASK,
    0,
    DMA_DEBUG_URGNT_ETH1RXU_FIELD_WIDTH,
    DMA_DEBUG_URGNT_ETH1RXU_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DMA_DEBUG_URGNT_ETH2RXU
 ******************************************************************************/
const ru_field_rec DMA_DEBUG_URGNT_ETH2RXU_FIELD =
{
    "ETH2RXU",
#if RU_INCLUDE_DESC
    "Ethernet2_RX_urgent_indication",
    "indication whether the queue is in urgent state or not",
#endif
    DMA_DEBUG_URGNT_ETH2RXU_FIELD_MASK,
    0,
    DMA_DEBUG_URGNT_ETH2RXU_FIELD_WIDTH,
    DMA_DEBUG_URGNT_ETH2RXU_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DMA_DEBUG_URGNT_ETH3RXU
 ******************************************************************************/
const ru_field_rec DMA_DEBUG_URGNT_ETH3RXU_FIELD =
{
    "ETH3RXU",
#if RU_INCLUDE_DESC
    "Ethernet3_RX_urgent_indication",
    "indication whether the queue is in urgent state or not",
#endif
    DMA_DEBUG_URGNT_ETH3RXU_FIELD_MASK,
    0,
    DMA_DEBUG_URGNT_ETH3RXU_FIELD_WIDTH,
    DMA_DEBUG_URGNT_ETH3RXU_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DMA_DEBUG_URGNT_ETH4RXU
 ******************************************************************************/
const ru_field_rec DMA_DEBUG_URGNT_ETH4RXU_FIELD =
{
    "ETH4RXU",
#if RU_INCLUDE_DESC
    "Ethernet4_RX_urgent_indication",
    "indication whether the queue is in urgent state or not",
#endif
    DMA_DEBUG_URGNT_ETH4RXU_FIELD_MASK,
    0,
    DMA_DEBUG_URGNT_ETH4RXU_FIELD_WIDTH,
    DMA_DEBUG_URGNT_ETH4RXU_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DMA_DEBUG_URGNT_GPONRXU
 ******************************************************************************/
const ru_field_rec DMA_DEBUG_URGNT_GPONRXU_FIELD =
{
    "GPONRXU",
#if RU_INCLUDE_DESC
    "GPON_RX_urgent_indication",
    "indication whether the queue is in urgent state or not",
#endif
    DMA_DEBUG_URGNT_GPONRXU_FIELD_MASK,
    0,
    DMA_DEBUG_URGNT_GPONRXU_FIELD_WIDTH,
    DMA_DEBUG_URGNT_GPONRXU_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DMA_DEBUG_URGNT_ETH0TXU
 ******************************************************************************/
const ru_field_rec DMA_DEBUG_URGNT_ETH0TXU_FIELD =
{
    "ETH0TXU",
#if RU_INCLUDE_DESC
    "Ethernet0_TX_urgent_indication",
    "indication whether the queue is in urgent state or not",
#endif
    DMA_DEBUG_URGNT_ETH0TXU_FIELD_MASK,
    0,
    DMA_DEBUG_URGNT_ETH0TXU_FIELD_WIDTH,
    DMA_DEBUG_URGNT_ETH0TXU_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DMA_DEBUG_URGNT_ETH1TXU
 ******************************************************************************/
const ru_field_rec DMA_DEBUG_URGNT_ETH1TXU_FIELD =
{
    "ETH1TXU",
#if RU_INCLUDE_DESC
    "Ethernet1_TX_urgent_indication",
    "indication whether the queue is in urgent state or not",
#endif
    DMA_DEBUG_URGNT_ETH1TXU_FIELD_MASK,
    0,
    DMA_DEBUG_URGNT_ETH1TXU_FIELD_WIDTH,
    DMA_DEBUG_URGNT_ETH1TXU_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DMA_DEBUG_URGNT_ETH2TXU
 ******************************************************************************/
const ru_field_rec DMA_DEBUG_URGNT_ETH2TXU_FIELD =
{
    "ETH2TXU",
#if RU_INCLUDE_DESC
    "Ethernet2_TX_urgent_indication",
    "indication whether the queue is in urgent state or not",
#endif
    DMA_DEBUG_URGNT_ETH2TXU_FIELD_MASK,
    0,
    DMA_DEBUG_URGNT_ETH2TXU_FIELD_WIDTH,
    DMA_DEBUG_URGNT_ETH2TXU_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DMA_DEBUG_URGNT_ETH3TXU
 ******************************************************************************/
const ru_field_rec DMA_DEBUG_URGNT_ETH3TXU_FIELD =
{
    "ETH3TXU",
#if RU_INCLUDE_DESC
    "Ethernet3_TX_urgent_indication",
    "indication whether the queue is in urgent state or not",
#endif
    DMA_DEBUG_URGNT_ETH3TXU_FIELD_MASK,
    0,
    DMA_DEBUG_URGNT_ETH3TXU_FIELD_WIDTH,
    DMA_DEBUG_URGNT_ETH3TXU_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DMA_DEBUG_URGNT_ETH4TXU
 ******************************************************************************/
const ru_field_rec DMA_DEBUG_URGNT_ETH4TXU_FIELD =
{
    "ETH4TXU",
#if RU_INCLUDE_DESC
    "Ethernet4_TX_urgent_indication",
    "indication whether the queue is in urgent state or not",
#endif
    DMA_DEBUG_URGNT_ETH4TXU_FIELD_MASK,
    0,
    DMA_DEBUG_URGNT_ETH4TXU_FIELD_WIDTH,
    DMA_DEBUG_URGNT_ETH4TXU_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: DMA_DEBUG_URGNT_GPONTXU
 ******************************************************************************/
const ru_field_rec DMA_DEBUG_URGNT_GPONTXU_FIELD =
{
    "GPONTXU",
#if RU_INCLUDE_DESC
    "GPON_TX_urgent_indication",
    "indication whether the queue is in urgent state or not",
#endif
    DMA_DEBUG_URGNT_GPONTXU_FIELD_MASK,
    0,
    DMA_DEBUG_URGNT_GPONTXU_FIELD_WIDTH,
    DMA_DEBUG_URGNT_GPONTXU_FIELD_SHIFT,
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
    687,
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
    "The allocation is of number of 128byte buffers out of the total 32 buffers for sdma or 96 buffers in dma  in the upload data RAM."
    "For the DMA, the buffers are divided between 2 physical RAMs 964 in the first, 32 in the second). The decision which clients FIFO is located in which memory is done by the register in address 0x98."
    "The allocation is done by defining a base address (aligned to 128 bytes) and the number of allocated buffers."
    "Note that the memory allocation should not contain wrap around. For example, if three buffers are needed, do not allocate buffers 30, 31 and 0."
    "The number of allocated CDs is the same of data buffers - one chunk descriptor per buffer, therefore allocation in CD RAM is defined only by offset address."
    ""
    "The order of peripherals within the array is:"
    "Ethernet 0"
    "Ethernet 1"
    "Ethernet 2"
    "Ethernet 3"
    "Ethernet 4"
    "GPON/EPON",
#endif
    DMA_CONFIG_NUM_OF_WRITES_REG_OFFSET,
    DMA_CONFIG_NUM_OF_WRITES_REG_RAM_CNT,
    4,
    688,
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
    689,
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
    690,
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
    "There are 8 levels of priorities, when each bit in the register represents a different level of priority. One should assert the relevant bit according to the desired priority -"
    "For the lowest  - 00000001"
    "For the highest - 10000000",
#endif
    DMA_CONFIG_PRI_REG_OFFSET,
    DMA_CONFIG_PRI_REG_RAM_CNT,
    4,
    691,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    DMA_CONFIG_PRI_FIELDS
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
    692,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    DMA_CONFIG_WEIGHT_FIELDS
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
    693,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    DMA_CONFIG_PERIPH_SOURCE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DMA_CONFIG_PTRRST
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_CONFIG_PTRRST_FIELDS[] =
{
    &DMA_CONFIG_PTRRST_ETH0RX_FIELD,
    &DMA_CONFIG_PTRRST_ETH0TX_FIELD,
    &DMA_CONFIG_PTRRST_ETH1RX_FIELD,
    &DMA_CONFIG_PTRRST_ETH1TX_FIELD,
    &DMA_CONFIG_PTRRST_ETH2RX_FIELD,
    &DMA_CONFIG_PTRRST_ETH2TX_FIELD,
    &DMA_CONFIG_PTRRST_ETH3RX_FIELD,
    &DMA_CONFIG_PTRRST_ETH3TX_FIELD,
    &DMA_CONFIG_PTRRST_ETH4RX_FIELD,
    &DMA_CONFIG_PTRRST_ETH4TX_FIELD,
    &DMA_CONFIG_PTRRST_GPONRX_FIELD,
    &DMA_CONFIG_PTRRST_GPONTX_FIELD,
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
    694,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    13,
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
    695,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    DMA_CONFIG_MAX_OTF_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DMA_DEBUG_NEMPTY
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_DEBUG_NEMPTY_FIELDS[] =
{
    &DMA_DEBUG_NEMPTY_ETH0RXNE_FIELD,
    &DMA_DEBUG_NEMPTY_ETH1RXNE_FIELD,
    &DMA_DEBUG_NEMPTY_ETH2RXNE_FIELD,
    &DMA_DEBUG_NEMPTY_ETH3RXNE_FIELD,
    &DMA_DEBUG_NEMPTY_ETH4RXNE_FIELD,
    &DMA_DEBUG_NEMPTY_GPONRXNE_FIELD,
    &DMA_DEBUG_NEMPTY_ETH0TXNE_FIELD,
    &DMA_DEBUG_NEMPTY_ETH1TXNE_FIELD,
    &DMA_DEBUG_NEMPTY_ETH2TXNE_FIELD,
    &DMA_DEBUG_NEMPTY_ETH3TXNE_FIELD,
    &DMA_DEBUG_NEMPTY_ETH4TXNE_FIELD,
    &DMA_DEBUG_NEMPTY_GPONTXNE_FIELD,
    &DMA_DEBUG_NEMPTY_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DMA_DEBUG_NEMPTY_REG = 
{
    "DEBUG_NEMPTY",
#if RU_INCLUDE_DESC
    "NOT_EMPTY_VECTOR Register",
    "Each peripheral, according to its source address, is represented in a bit on the not empty vector."
    "If the bit is asserted, the requests queue of the relevant peripheral is not empty."
    "The not empty vector is used by the DMA scheduler to determine which peripheral is the next to be served.",
#endif
    DMA_DEBUG_NEMPTY_REG_OFFSET,
    0,
    0,
    696,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    13,
    DMA_DEBUG_NEMPTY_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: DMA_DEBUG_URGNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *DMA_DEBUG_URGNT_FIELDS[] =
{
    &DMA_DEBUG_URGNT_ETH0RXU_FIELD,
    &DMA_DEBUG_URGNT_ETH1RXU_FIELD,
    &DMA_DEBUG_URGNT_ETH2RXU_FIELD,
    &DMA_DEBUG_URGNT_ETH3RXU_FIELD,
    &DMA_DEBUG_URGNT_ETH4RXU_FIELD,
    &DMA_DEBUG_URGNT_GPONRXU_FIELD,
    &DMA_DEBUG_URGNT_ETH0TXU_FIELD,
    &DMA_DEBUG_URGNT_ETH1TXU_FIELD,
    &DMA_DEBUG_URGNT_ETH2TXU_FIELD,
    &DMA_DEBUG_URGNT_ETH3TXU_FIELD,
    &DMA_DEBUG_URGNT_ETH4TXU_FIELD,
    &DMA_DEBUG_URGNT_GPONTXU_FIELD,
    &DMA_DEBUG_URGNT_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec DMA_DEBUG_URGNT_REG = 
{
    "DEBUG_URGNT",
#if RU_INCLUDE_DESC
    "URGENT_VECTOR Register",
    "Each peripheral, according to its source address, is represented in a bit on the urgent vector."
    "If the bit is asserted, the requests queue of the relevant peripheral is in urgent state."
    "The urgent vector is used by the DMA scheduler to determine which peripheral is the next to be served.",
#endif
    DMA_DEBUG_URGNT_REG_OFFSET,
    0,
    0,
    697,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    13,
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
    698,
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
    699,
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
    700,
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
    701,
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
    702,
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
    703,
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
    704,
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
    705,
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
    706,
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
    &DMA_CONFIG_WEIGHT_REG,
    &DMA_CONFIG_PERIPH_SOURCE_REG,
    &DMA_CONFIG_PTRRST_REG,
    &DMA_CONFIG_MAX_OTF_REG,
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
    20,
    DMA_REGS
};

/* End of file XRDP_DMA.c */
