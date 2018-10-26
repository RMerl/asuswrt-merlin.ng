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
 * Field: XPORT_MAB_CNTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_MAB_CNTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_MAB_CNTRL_RESERVED0_FIELD_MASK,
    0,
    XPORT_MAB_CNTRL_RESERVED0_FIELD_WIDTH,
    XPORT_MAB_CNTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MAB_CNTRL_LINK_DOWN_RST_EN
 ******************************************************************************/
const ru_field_rec XPORT_MAB_CNTRL_LINK_DOWN_RST_EN_FIELD =
{
    "LINK_DOWN_RST_EN",
#if RU_INCLUDE_DESC
    "",
    "When this bit is set asynchronous RX and TX FIFOs are reset for a port when the link goes down "
    "that is when the local fault is detected.",
#endif
    XPORT_MAB_CNTRL_LINK_DOWN_RST_EN_FIELD_MASK,
    0,
    XPORT_MAB_CNTRL_LINK_DOWN_RST_EN_FIELD_WIDTH,
    XPORT_MAB_CNTRL_LINK_DOWN_RST_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MAB_CNTRL_RESERVED1
 ******************************************************************************/
const ru_field_rec XPORT_MAB_CNTRL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_MAB_CNTRL_RESERVED1_FIELD_MASK,
    0,
    XPORT_MAB_CNTRL_RESERVED1_FIELD_WIDTH,
    XPORT_MAB_CNTRL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MAB_CNTRL_XGMII_TX_RST
 ******************************************************************************/
const ru_field_rec XPORT_MAB_CNTRL_XGMII_TX_RST_FIELD =
{
    "XGMII_TX_RST",
#if RU_INCLUDE_DESC
    "",
    "When set resets 10G Port 0 asynchronous TX FIFO and associated logic (such as credit logic).",
#endif
    XPORT_MAB_CNTRL_XGMII_TX_RST_FIELD_MASK,
    0,
    XPORT_MAB_CNTRL_XGMII_TX_RST_FIELD_WIDTH,
    XPORT_MAB_CNTRL_XGMII_TX_RST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MAB_CNTRL_GMII_TX_RST
 ******************************************************************************/
const ru_field_rec XPORT_MAB_CNTRL_GMII_TX_RST_FIELD =
{
    "GMII_TX_RST",
#if RU_INCLUDE_DESC
    "",
    "When a bit in this vector is set it resets corresponding port (Port 3-0) asynchronous TX FIFO "
    "and associated logic (such as credit logic and byte slicers).",
#endif
    XPORT_MAB_CNTRL_GMII_TX_RST_FIELD_MASK,
    0,
    XPORT_MAB_CNTRL_GMII_TX_RST_FIELD_WIDTH,
    XPORT_MAB_CNTRL_GMII_TX_RST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MAB_CNTRL_RESERVED2
 ******************************************************************************/
const ru_field_rec XPORT_MAB_CNTRL_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_MAB_CNTRL_RESERVED2_FIELD_MASK,
    0,
    XPORT_MAB_CNTRL_RESERVED2_FIELD_WIDTH,
    XPORT_MAB_CNTRL_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MAB_CNTRL_XGMII_RX_RST
 ******************************************************************************/
const ru_field_rec XPORT_MAB_CNTRL_XGMII_RX_RST_FIELD =
{
    "XGMII_RX_RST",
#if RU_INCLUDE_DESC
    "",
    "When set resets 10G Port 0 asynchronous RX FIFO and associated logic.",
#endif
    XPORT_MAB_CNTRL_XGMII_RX_RST_FIELD_MASK,
    0,
    XPORT_MAB_CNTRL_XGMII_RX_RST_FIELD_WIDTH,
    XPORT_MAB_CNTRL_XGMII_RX_RST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MAB_CNTRL_GMII_RX_RST
 ******************************************************************************/
const ru_field_rec XPORT_MAB_CNTRL_GMII_RX_RST_FIELD =
{
    "GMII_RX_RST",
#if RU_INCLUDE_DESC
    "",
    "When a bit in this vector is set it resets corresponding port (Port 3-0) asynchronous RX FIFO "
    "and associated logic (such as byte packers).",
#endif
    XPORT_MAB_CNTRL_GMII_RX_RST_FIELD_MASK,
    0,
    XPORT_MAB_CNTRL_GMII_RX_RST_FIELD_WIDTH,
    XPORT_MAB_CNTRL_GMII_RX_RST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MAB_TX_WRR_CTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_MAB_TX_WRR_CTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_MAB_TX_WRR_CTRL_RESERVED0_FIELD_MASK,
    0,
    XPORT_MAB_TX_WRR_CTRL_RESERVED0_FIELD_WIDTH,
    XPORT_MAB_TX_WRR_CTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MAB_TX_WRR_CTRL_ARB_MODE
 ******************************************************************************/
const ru_field_rec XPORT_MAB_TX_WRR_CTRL_ARB_MODE_FIELD =
{
    "ARB_MODE",
#if RU_INCLUDE_DESC
    "",
    "Arbiter Mode\n"
    "1'b0 - Fixed Mode. TDM slots allocated regardless of the port activity.\n"
    "1'b1 - Work-Conserving Mode. TDM slots allocation is affected by the port activity.",
#endif
    XPORT_MAB_TX_WRR_CTRL_ARB_MODE_FIELD_MASK,
    0,
    XPORT_MAB_TX_WRR_CTRL_ARB_MODE_FIELD_WIDTH,
    XPORT_MAB_TX_WRR_CTRL_ARB_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MAB_TX_WRR_CTRL_P3_WEIGHT
 ******************************************************************************/
const ru_field_rec XPORT_MAB_TX_WRR_CTRL_P3_WEIGHT_FIELD =
{
    "P3_WEIGHT",
#if RU_INCLUDE_DESC
    "",
    "P3 weight expressed in TDM time slots.",
#endif
    XPORT_MAB_TX_WRR_CTRL_P3_WEIGHT_FIELD_MASK,
    0,
    XPORT_MAB_TX_WRR_CTRL_P3_WEIGHT_FIELD_WIDTH,
    XPORT_MAB_TX_WRR_CTRL_P3_WEIGHT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MAB_TX_WRR_CTRL_P2_WEIGHT
 ******************************************************************************/
const ru_field_rec XPORT_MAB_TX_WRR_CTRL_P2_WEIGHT_FIELD =
{
    "P2_WEIGHT",
#if RU_INCLUDE_DESC
    "",
    "P2 weight expressed in TDM time slots.",
#endif
    XPORT_MAB_TX_WRR_CTRL_P2_WEIGHT_FIELD_MASK,
    0,
    XPORT_MAB_TX_WRR_CTRL_P2_WEIGHT_FIELD_WIDTH,
    XPORT_MAB_TX_WRR_CTRL_P2_WEIGHT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MAB_TX_WRR_CTRL_P1_WEIGHT
 ******************************************************************************/
const ru_field_rec XPORT_MAB_TX_WRR_CTRL_P1_WEIGHT_FIELD =
{
    "P1_WEIGHT",
#if RU_INCLUDE_DESC
    "",
    "P1 weight expressed in TDM time slots.",
#endif
    XPORT_MAB_TX_WRR_CTRL_P1_WEIGHT_FIELD_MASK,
    0,
    XPORT_MAB_TX_WRR_CTRL_P1_WEIGHT_FIELD_WIDTH,
    XPORT_MAB_TX_WRR_CTRL_P1_WEIGHT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MAB_TX_WRR_CTRL_P0_WEIGHT
 ******************************************************************************/
const ru_field_rec XPORT_MAB_TX_WRR_CTRL_P0_WEIGHT_FIELD =
{
    "P0_WEIGHT",
#if RU_INCLUDE_DESC
    "",
    "P0 weight expressed in TDM time slots.\n"
    "Note: Arbitration weights should not be changed from their default values due to XLMAC implementation specifics. "
    "In 4-port mode MSBUS clock should be set to 4*MAX_PORT_RATE/64.",
#endif
    XPORT_MAB_TX_WRR_CTRL_P0_WEIGHT_FIELD_MASK,
    0,
    XPORT_MAB_TX_WRR_CTRL_P0_WEIGHT_FIELD_WIDTH,
    XPORT_MAB_TX_WRR_CTRL_P0_WEIGHT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MAB_TX_THRESHOLD_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_MAB_TX_THRESHOLD_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_MAB_TX_THRESHOLD_RESERVED0_FIELD_MASK,
    0,
    XPORT_MAB_TX_THRESHOLD_RESERVED0_FIELD_WIDTH,
    XPORT_MAB_TX_THRESHOLD_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MAB_TX_THRESHOLD_XGMII0_TX_THRESHOLD
 ******************************************************************************/
const ru_field_rec XPORT_MAB_TX_THRESHOLD_XGMII0_TX_THRESHOLD_FIELD =
{
    "XGMII0_TX_THRESHOLD",
#if RU_INCLUDE_DESC
    "",
    "XGMII0 (P0) asynchronous TX FIFO read depth at which packet dequeue starts.",
#endif
    XPORT_MAB_TX_THRESHOLD_XGMII0_TX_THRESHOLD_FIELD_MASK,
    0,
    XPORT_MAB_TX_THRESHOLD_XGMII0_TX_THRESHOLD_FIELD_WIDTH,
    XPORT_MAB_TX_THRESHOLD_XGMII0_TX_THRESHOLD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MAB_TX_THRESHOLD_GMII3_TX_THRESHOLD
 ******************************************************************************/
const ru_field_rec XPORT_MAB_TX_THRESHOLD_GMII3_TX_THRESHOLD_FIELD =
{
    "GMII3_TX_THRESHOLD",
#if RU_INCLUDE_DESC
    "",
    "GMII P3 asynchronous TX FIFO read depth at which packet dequeue starts.",
#endif
    XPORT_MAB_TX_THRESHOLD_GMII3_TX_THRESHOLD_FIELD_MASK,
    0,
    XPORT_MAB_TX_THRESHOLD_GMII3_TX_THRESHOLD_FIELD_WIDTH,
    XPORT_MAB_TX_THRESHOLD_GMII3_TX_THRESHOLD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MAB_TX_THRESHOLD_GMII2_TX_THRESHOLD
 ******************************************************************************/
const ru_field_rec XPORT_MAB_TX_THRESHOLD_GMII2_TX_THRESHOLD_FIELD =
{
    "GMII2_TX_THRESHOLD",
#if RU_INCLUDE_DESC
    "",
    "GMII P2 asynchronous TX FIFO read depth at which packet dequeue starts.",
#endif
    XPORT_MAB_TX_THRESHOLD_GMII2_TX_THRESHOLD_FIELD_MASK,
    0,
    XPORT_MAB_TX_THRESHOLD_GMII2_TX_THRESHOLD_FIELD_WIDTH,
    XPORT_MAB_TX_THRESHOLD_GMII2_TX_THRESHOLD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MAB_TX_THRESHOLD_GMII1_TX_THRESHOLD
 ******************************************************************************/
const ru_field_rec XPORT_MAB_TX_THRESHOLD_GMII1_TX_THRESHOLD_FIELD =
{
    "GMII1_TX_THRESHOLD",
#if RU_INCLUDE_DESC
    "",
    "GMII P1 asynchronous TX FIFO read depth at which packet dequeue starts.",
#endif
    XPORT_MAB_TX_THRESHOLD_GMII1_TX_THRESHOLD_FIELD_MASK,
    0,
    XPORT_MAB_TX_THRESHOLD_GMII1_TX_THRESHOLD_FIELD_WIDTH,
    XPORT_MAB_TX_THRESHOLD_GMII1_TX_THRESHOLD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MAB_TX_THRESHOLD_GMII0_TX_THRESHOLD
 ******************************************************************************/
const ru_field_rec XPORT_MAB_TX_THRESHOLD_GMII0_TX_THRESHOLD_FIELD =
{
    "GMII0_TX_THRESHOLD",
#if RU_INCLUDE_DESC
    "",
    "GMII P0 asynchronous TX FIFO read depth at which packet dequeue starts.",
#endif
    XPORT_MAB_TX_THRESHOLD_GMII0_TX_THRESHOLD_FIELD_MASK,
    0,
    XPORT_MAB_TX_THRESHOLD_GMII0_TX_THRESHOLD_FIELD_WIDTH,
    XPORT_MAB_TX_THRESHOLD_GMII0_TX_THRESHOLD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MAB_LINK_DOWN_TX_DATA_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_MAB_LINK_DOWN_TX_DATA_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_MAB_LINK_DOWN_TX_DATA_RESERVED0_FIELD_MASK,
    0,
    XPORT_MAB_LINK_DOWN_TX_DATA_RESERVED0_FIELD_WIDTH,
    XPORT_MAB_LINK_DOWN_TX_DATA_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MAB_LINK_DOWN_TX_DATA_TXCTL
 ******************************************************************************/
const ru_field_rec XPORT_MAB_LINK_DOWN_TX_DATA_TXCTL_FIELD =
{
    "TXCTL",
#if RU_INCLUDE_DESC
    "",
    "When LINK_DOWN_RST_EN = 1 and link is down content of this register is sent to serdes over XGMII interface. "
    " In GMII mode 0 is sent.",
#endif
    XPORT_MAB_LINK_DOWN_TX_DATA_TXCTL_FIELD_MASK,
    0,
    XPORT_MAB_LINK_DOWN_TX_DATA_TXCTL_FIELD_WIDTH,
    XPORT_MAB_LINK_DOWN_TX_DATA_TXCTL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MAB_LINK_DOWN_TX_DATA_TXD
 ******************************************************************************/
const ru_field_rec XPORT_MAB_LINK_DOWN_TX_DATA_TXD_FIELD =
{
    "TXD",
#if RU_INCLUDE_DESC
    "",
    "When LINK_DOWN_RST_EN = 1 and link is down content of this register is sent to serdes over XGMII interface. "
    "In GMII mode 0's are sent.",
#endif
    XPORT_MAB_LINK_DOWN_TX_DATA_TXD_FIELD_MASK,
    0,
    XPORT_MAB_LINK_DOWN_TX_DATA_TXD_FIELD_WIDTH,
    XPORT_MAB_LINK_DOWN_TX_DATA_TXD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MAB_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_MAB_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_MAB_STATUS_RESERVED0_FIELD_MASK,
    0,
    XPORT_MAB_STATUS_RESERVED0_FIELD_WIDTH,
    XPORT_MAB_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPORT_MAB_STATUS_XGMII_RX_AFIFO_OVERRUN
 ******************************************************************************/
const ru_field_rec XPORT_MAB_STATUS_XGMII_RX_AFIFO_OVERRUN_FIELD =
{
    "XGMII_RX_AFIFO_OVERRUN",
#if RU_INCLUDE_DESC
    "",
    "10G Port 0 asynchronous RX FIFO over-run status.",
#endif
    XPORT_MAB_STATUS_XGMII_RX_AFIFO_OVERRUN_FIELD_MASK,
    0,
    XPORT_MAB_STATUS_XGMII_RX_AFIFO_OVERRUN_FIELD_WIDTH,
    XPORT_MAB_STATUS_XGMII_RX_AFIFO_OVERRUN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPORT_MAB_STATUS_GMII_RX_AFIFO_OVERRUN_VECT
 ******************************************************************************/
const ru_field_rec XPORT_MAB_STATUS_GMII_RX_AFIFO_OVERRUN_VECT_FIELD =
{
    "GMII_RX_AFIFO_OVERRUN_VECT",
#if RU_INCLUDE_DESC
    "",
    "Port 3-0 asynchronous RX FIFO over-run status.",
#endif
    XPORT_MAB_STATUS_GMII_RX_AFIFO_OVERRUN_VECT_FIELD_MASK,
    0,
    XPORT_MAB_STATUS_GMII_RX_AFIFO_OVERRUN_VECT_FIELD_WIDTH,
    XPORT_MAB_STATUS_GMII_RX_AFIFO_OVERRUN_VECT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPORT_MAB_STATUS_XGMII_TX_FRM_UNDERRUN
 ******************************************************************************/
const ru_field_rec XPORT_MAB_STATUS_XGMII_TX_FRM_UNDERRUN_FIELD =
{
    "XGMII_TX_FRM_UNDERRUN",
#if RU_INCLUDE_DESC
    "",
    "10G Port 0 TX frame under-run status.",
#endif
    XPORT_MAB_STATUS_XGMII_TX_FRM_UNDERRUN_FIELD_MASK,
    0,
    XPORT_MAB_STATUS_XGMII_TX_FRM_UNDERRUN_FIELD_WIDTH,
    XPORT_MAB_STATUS_XGMII_TX_FRM_UNDERRUN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPORT_MAB_STATUS_XGMII_OUTSTANDING_CREDITS_CNT_UNDERRUN
 ******************************************************************************/
const ru_field_rec XPORT_MAB_STATUS_XGMII_OUTSTANDING_CREDITS_CNT_UNDERRUN_FIELD =
{
    "XGMII_OUTSTANDING_CREDITS_CNT_UNDERRUN",
#if RU_INCLUDE_DESC
    "",
    "10G Port 0 TX credits under-run status.",
#endif
    XPORT_MAB_STATUS_XGMII_OUTSTANDING_CREDITS_CNT_UNDERRUN_FIELD_MASK,
    0,
    XPORT_MAB_STATUS_XGMII_OUTSTANDING_CREDITS_CNT_UNDERRUN_FIELD_WIDTH,
    XPORT_MAB_STATUS_XGMII_OUTSTANDING_CREDITS_CNT_UNDERRUN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPORT_MAB_STATUS_GMII_OUTSTANDING_CREDITS_CNT_UNDERRUN_VECT
 ******************************************************************************/
const ru_field_rec XPORT_MAB_STATUS_GMII_OUTSTANDING_CREDITS_CNT_UNDERRUN_VECT_FIELD =
{
    "GMII_OUTSTANDING_CREDITS_CNT_UNDERRUN_VECT",
#if RU_INCLUDE_DESC
    "",
    "Port 3-0 TX credits under-run status.",
#endif
    XPORT_MAB_STATUS_GMII_OUTSTANDING_CREDITS_CNT_UNDERRUN_VECT_FIELD_MASK,
    0,
    XPORT_MAB_STATUS_GMII_OUTSTANDING_CREDITS_CNT_UNDERRUN_VECT_FIELD_WIDTH,
    XPORT_MAB_STATUS_GMII_OUTSTANDING_CREDITS_CNT_UNDERRUN_VECT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPORT_MAB_STATUS_XGMII_TX_AFIFO_OVERRUN
 ******************************************************************************/
const ru_field_rec XPORT_MAB_STATUS_XGMII_TX_AFIFO_OVERRUN_FIELD =
{
    "XGMII_TX_AFIFO_OVERRUN",
#if RU_INCLUDE_DESC
    "",
    "10G Port 0 asynchronous TX FIFO over-run status.",
#endif
    XPORT_MAB_STATUS_XGMII_TX_AFIFO_OVERRUN_FIELD_MASK,
    0,
    XPORT_MAB_STATUS_XGMII_TX_AFIFO_OVERRUN_FIELD_WIDTH,
    XPORT_MAB_STATUS_XGMII_TX_AFIFO_OVERRUN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPORT_MAB_STATUS_GMII_TX_AFIFO_OVERRUN_VECT
 ******************************************************************************/
const ru_field_rec XPORT_MAB_STATUS_GMII_TX_AFIFO_OVERRUN_VECT_FIELD =
{
    "GMII_TX_AFIFO_OVERRUN_VECT",
#if RU_INCLUDE_DESC
    "",
    "Port 3-0 asynchronous TX FIFO over-run status.",
#endif
    XPORT_MAB_STATUS_GMII_TX_AFIFO_OVERRUN_VECT_FIELD_MASK,
    0,
    XPORT_MAB_STATUS_GMII_TX_AFIFO_OVERRUN_VECT_FIELD_WIDTH,
    XPORT_MAB_STATUS_GMII_TX_AFIFO_OVERRUN_VECT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: XPORT_MAB_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_MAB_CNTRL_FIELDS[] =
{
    &XPORT_MAB_CNTRL_RESERVED0_FIELD,
    &XPORT_MAB_CNTRL_LINK_DOWN_RST_EN_FIELD,
    &XPORT_MAB_CNTRL_RESERVED1_FIELD,
    &XPORT_MAB_CNTRL_XGMII_TX_RST_FIELD,
    &XPORT_MAB_CNTRL_GMII_TX_RST_FIELD,
    &XPORT_MAB_CNTRL_RESERVED2_FIELD,
    &XPORT_MAB_CNTRL_XGMII_RX_RST_FIELD,
    &XPORT_MAB_CNTRL_GMII_RX_RST_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_MAB_CNTRL_REG = 
{
    "CNTRL",
#if RU_INCLUDE_DESC
    "MSBUS Adaptation Control Register",
    "",
#endif
    XPORT_MAB_CNTRL_REG_OFFSET,
    0,
    0,
    234,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    XPORT_MAB_CNTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_MAB_TX_WRR_CTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_MAB_TX_WRR_CTRL_FIELDS[] =
{
    &XPORT_MAB_TX_WRR_CTRL_RESERVED0_FIELD,
    &XPORT_MAB_TX_WRR_CTRL_ARB_MODE_FIELD,
    &XPORT_MAB_TX_WRR_CTRL_P3_WEIGHT_FIELD,
    &XPORT_MAB_TX_WRR_CTRL_P2_WEIGHT_FIELD,
    &XPORT_MAB_TX_WRR_CTRL_P1_WEIGHT_FIELD,
    &XPORT_MAB_TX_WRR_CTRL_P0_WEIGHT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_MAB_TX_WRR_CTRL_REG = 
{
    "TX_WRR_CTRL",
#if RU_INCLUDE_DESC
    "MSBUS Adaptation TX WRR Control Register",
    "",
#endif
    XPORT_MAB_TX_WRR_CTRL_REG_OFFSET,
    0,
    0,
    235,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    XPORT_MAB_TX_WRR_CTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_MAB_TX_THRESHOLD
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_MAB_TX_THRESHOLD_FIELDS[] =
{
    &XPORT_MAB_TX_THRESHOLD_RESERVED0_FIELD,
    &XPORT_MAB_TX_THRESHOLD_XGMII0_TX_THRESHOLD_FIELD,
    &XPORT_MAB_TX_THRESHOLD_GMII3_TX_THRESHOLD_FIELD,
    &XPORT_MAB_TX_THRESHOLD_GMII2_TX_THRESHOLD_FIELD,
    &XPORT_MAB_TX_THRESHOLD_GMII1_TX_THRESHOLD_FIELD,
    &XPORT_MAB_TX_THRESHOLD_GMII0_TX_THRESHOLD_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_MAB_TX_THRESHOLD_REG = 
{
    "TX_THRESHOLD",
#if RU_INCLUDE_DESC
    "MSBUS Adaptation TX Threshold Register",
    "",
#endif
    XPORT_MAB_TX_THRESHOLD_REG_OFFSET,
    0,
    0,
    236,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    XPORT_MAB_TX_THRESHOLD_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_MAB_LINK_DOWN_TX_DATA
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_MAB_LINK_DOWN_TX_DATA_FIELDS[] =
{
    &XPORT_MAB_LINK_DOWN_TX_DATA_RESERVED0_FIELD,
    &XPORT_MAB_LINK_DOWN_TX_DATA_TXCTL_FIELD,
    &XPORT_MAB_LINK_DOWN_TX_DATA_TXD_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_MAB_LINK_DOWN_TX_DATA_REG = 
{
    "LINK_DOWN_TX_DATA",
#if RU_INCLUDE_DESC
    "MSBUS Adaptation Link down TX Data Register",
    "",
#endif
    XPORT_MAB_LINK_DOWN_TX_DATA_REG_OFFSET,
    0,
    0,
    237,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    XPORT_MAB_LINK_DOWN_TX_DATA_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_MAB_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_MAB_STATUS_FIELDS[] =
{
    &XPORT_MAB_STATUS_RESERVED0_FIELD,
    &XPORT_MAB_STATUS_XGMII_RX_AFIFO_OVERRUN_FIELD,
    &XPORT_MAB_STATUS_GMII_RX_AFIFO_OVERRUN_VECT_FIELD,
    &XPORT_MAB_STATUS_XGMII_TX_FRM_UNDERRUN_FIELD,
    &XPORT_MAB_STATUS_XGMII_OUTSTANDING_CREDITS_CNT_UNDERRUN_FIELD,
    &XPORT_MAB_STATUS_GMII_OUTSTANDING_CREDITS_CNT_UNDERRUN_VECT_FIELD,
    &XPORT_MAB_STATUS_XGMII_TX_AFIFO_OVERRUN_FIELD,
    &XPORT_MAB_STATUS_GMII_TX_AFIFO_OVERRUN_VECT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_MAB_STATUS_REG = 
{
    "STATUS",
#if RU_INCLUDE_DESC
    "MSBUS Adaptation Status Register",
    "",
#endif
    XPORT_MAB_STATUS_REG_OFFSET,
    0,
    0,
    238,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    XPORT_MAB_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: XPORT_MAB
 ******************************************************************************/
static const ru_reg_rec *XPORT_MAB_REGS[] =
{
    &XPORT_MAB_CNTRL_REG,
    &XPORT_MAB_TX_WRR_CTRL_REG,
    &XPORT_MAB_TX_THRESHOLD_REG,
    &XPORT_MAB_LINK_DOWN_TX_DATA_REG,
    &XPORT_MAB_STATUS_REG,
};

unsigned long XPORT_MAB_ADDRS[] =
{
    0x8013b300,
};

const ru_block_rec XPORT_MAB_BLOCK = 
{
    "XPORT_MAB",
    XPORT_MAB_ADDRS,
    1,
    5,
    XPORT_MAB_REGS
};

/* End of file XPORT_MAB.c */
