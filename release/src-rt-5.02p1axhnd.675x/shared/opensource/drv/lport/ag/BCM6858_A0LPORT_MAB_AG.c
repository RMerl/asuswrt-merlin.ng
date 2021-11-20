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
 * Field: LPORT_MAB_CNTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_MAB_CNTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_MAB_CNTRL_RESERVED0_FIELD_MASK,
    0,
    LPORT_MAB_CNTRL_RESERVED0_FIELD_WIDTH,
    LPORT_MAB_CNTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_MAB_CNTRL_LINK_DOWN_RST_EN
 ******************************************************************************/
const ru_field_rec LPORT_MAB_CNTRL_LINK_DOWN_RST_EN_FIELD =
{
    "LINK_DOWN_RST_EN",
#if RU_INCLUDE_DESC
    "",
    "When this bit is set asynchronous RX and TX FIFOs are reset for a port when the link goes down "
    "that is when the local fault is detected.",
#endif
    LPORT_MAB_CNTRL_LINK_DOWN_RST_EN_FIELD_MASK,
    0,
    LPORT_MAB_CNTRL_LINK_DOWN_RST_EN_FIELD_WIDTH,
    LPORT_MAB_CNTRL_LINK_DOWN_RST_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_MAB_CNTRL_RESERVED1
 ******************************************************************************/
const ru_field_rec LPORT_MAB_CNTRL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_MAB_CNTRL_RESERVED1_FIELD_MASK,
    0,
    LPORT_MAB_CNTRL_RESERVED1_FIELD_WIDTH,
    LPORT_MAB_CNTRL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_MAB_CNTRL_XGMII_TX_RST
 ******************************************************************************/
const ru_field_rec LPORT_MAB_CNTRL_XGMII_TX_RST_FIELD =
{
    "XGMII_TX_RST",
#if RU_INCLUDE_DESC
    "",
    "When set resets 10G Port 0 asynchronous TX FIFO and associated logic (such as credit logic).",
#endif
    LPORT_MAB_CNTRL_XGMII_TX_RST_FIELD_MASK,
    0,
    LPORT_MAB_CNTRL_XGMII_TX_RST_FIELD_WIDTH,
    LPORT_MAB_CNTRL_XGMII_TX_RST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_MAB_CNTRL_GMII_TX_RST
 ******************************************************************************/
const ru_field_rec LPORT_MAB_CNTRL_GMII_TX_RST_FIELD =
{
    "GMII_TX_RST",
#if RU_INCLUDE_DESC
    "",
    "When a bit in this vector is set it resets corresponding port (Port 3-0) asynchronous TX FIFO "
    "and associated logic (such as credit logic and byte slicers).",
#endif
    LPORT_MAB_CNTRL_GMII_TX_RST_FIELD_MASK,
    0,
    LPORT_MAB_CNTRL_GMII_TX_RST_FIELD_WIDTH,
    LPORT_MAB_CNTRL_GMII_TX_RST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_MAB_CNTRL_RESERVED2
 ******************************************************************************/
const ru_field_rec LPORT_MAB_CNTRL_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_MAB_CNTRL_RESERVED2_FIELD_MASK,
    0,
    LPORT_MAB_CNTRL_RESERVED2_FIELD_WIDTH,
    LPORT_MAB_CNTRL_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_MAB_CNTRL_XGMII_RX_RST
 ******************************************************************************/
const ru_field_rec LPORT_MAB_CNTRL_XGMII_RX_RST_FIELD =
{
    "XGMII_RX_RST",
#if RU_INCLUDE_DESC
    "",
    "When set resets 10G Port 0 asynchronous RX FIFO and associated logic.",
#endif
    LPORT_MAB_CNTRL_XGMII_RX_RST_FIELD_MASK,
    0,
    LPORT_MAB_CNTRL_XGMII_RX_RST_FIELD_WIDTH,
    LPORT_MAB_CNTRL_XGMII_RX_RST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_MAB_CNTRL_GMII_RX_RST
 ******************************************************************************/
const ru_field_rec LPORT_MAB_CNTRL_GMII_RX_RST_FIELD =
{
    "GMII_RX_RST",
#if RU_INCLUDE_DESC
    "",
    "When a bit in this vector is set it resets corresponding port (Port 3-0) asynchronous RX FIFO "
    "and associated logic (such as byte packers).",
#endif
    LPORT_MAB_CNTRL_GMII_RX_RST_FIELD_MASK,
    0,
    LPORT_MAB_CNTRL_GMII_RX_RST_FIELD_WIDTH,
    LPORT_MAB_CNTRL_GMII_RX_RST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_MAB_TX_WRR_CTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_MAB_TX_WRR_CTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_MAB_TX_WRR_CTRL_RESERVED0_FIELD_MASK,
    0,
    LPORT_MAB_TX_WRR_CTRL_RESERVED0_FIELD_WIDTH,
    LPORT_MAB_TX_WRR_CTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_MAB_TX_WRR_CTRL_ARB_MODE
 ******************************************************************************/
const ru_field_rec LPORT_MAB_TX_WRR_CTRL_ARB_MODE_FIELD =
{
    "ARB_MODE",
#if RU_INCLUDE_DESC
    "",
    "Arbiter Mode\n"
    "1'b0 - Fixed Mode. TDM slots allocated regardless of the port activity.\n"
    "1'b1 - Work-Conserving Mode. TDM slots allocation is affected by the port activity.",
#endif
    LPORT_MAB_TX_WRR_CTRL_ARB_MODE_FIELD_MASK,
    0,
    LPORT_MAB_TX_WRR_CTRL_ARB_MODE_FIELD_WIDTH,
    LPORT_MAB_TX_WRR_CTRL_ARB_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_MAB_TX_WRR_CTRL_P7_WEIGHT
 ******************************************************************************/
const ru_field_rec LPORT_MAB_TX_WRR_CTRL_P7_WEIGHT_FIELD =
{
    "P7_WEIGHT",
#if RU_INCLUDE_DESC
    "",
    "P7 weight expressed in TDM time slots.",
#endif
    LPORT_MAB_TX_WRR_CTRL_P7_WEIGHT_FIELD_MASK,
    0,
    LPORT_MAB_TX_WRR_CTRL_P7_WEIGHT_FIELD_WIDTH,
    LPORT_MAB_TX_WRR_CTRL_P7_WEIGHT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_MAB_TX_WRR_CTRL_P6_WEIGHT
 ******************************************************************************/
const ru_field_rec LPORT_MAB_TX_WRR_CTRL_P6_WEIGHT_FIELD =
{
    "P6_WEIGHT",
#if RU_INCLUDE_DESC
    "",
    "P6 weight expressed in TDM time slots.",
#endif
    LPORT_MAB_TX_WRR_CTRL_P6_WEIGHT_FIELD_MASK,
    0,
    LPORT_MAB_TX_WRR_CTRL_P6_WEIGHT_FIELD_WIDTH,
    LPORT_MAB_TX_WRR_CTRL_P6_WEIGHT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_MAB_TX_WRR_CTRL_P5_WEIGHT
 ******************************************************************************/
const ru_field_rec LPORT_MAB_TX_WRR_CTRL_P5_WEIGHT_FIELD =
{
    "P5_WEIGHT",
#if RU_INCLUDE_DESC
    "",
    "P5 weight expressed in TDM time slots.",
#endif
    LPORT_MAB_TX_WRR_CTRL_P5_WEIGHT_FIELD_MASK,
    0,
    LPORT_MAB_TX_WRR_CTRL_P5_WEIGHT_FIELD_WIDTH,
    LPORT_MAB_TX_WRR_CTRL_P5_WEIGHT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_MAB_TX_WRR_CTRL_P4_WEIGHT
 ******************************************************************************/
const ru_field_rec LPORT_MAB_TX_WRR_CTRL_P4_WEIGHT_FIELD =
{
    "P4_WEIGHT",
#if RU_INCLUDE_DESC
    "",
    "P4 weight expressed in TDM time slots.\n"
    "Allocated port bandwidth is equal to TOTAL_BW*(Px_WEIGHT/SUM(P4_WEIGHT,...,P7_WEIGHT) (where TOTAL_BW is 25.6Gb/as for 400MHz MSBUS clock).",
#endif
    LPORT_MAB_TX_WRR_CTRL_P4_WEIGHT_FIELD_MASK,
    0,
    LPORT_MAB_TX_WRR_CTRL_P4_WEIGHT_FIELD_WIDTH,
    LPORT_MAB_TX_WRR_CTRL_P4_WEIGHT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_MAB_TX_THRESHOLD_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_MAB_TX_THRESHOLD_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_MAB_TX_THRESHOLD_RESERVED0_FIELD_MASK,
    0,
    LPORT_MAB_TX_THRESHOLD_RESERVED0_FIELD_WIDTH,
    LPORT_MAB_TX_THRESHOLD_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_MAB_TX_THRESHOLD_XGMII1_TX_THRESHOLD
 ******************************************************************************/
const ru_field_rec LPORT_MAB_TX_THRESHOLD_XGMII1_TX_THRESHOLD_FIELD =
{
    "XGMII1_TX_THRESHOLD",
#if RU_INCLUDE_DESC
    "",
    "XGMII1 (P4) asynchronous TX FIFO read depth at which packet dequeue starts.",
#endif
    LPORT_MAB_TX_THRESHOLD_XGMII1_TX_THRESHOLD_FIELD_MASK,
    0,
    LPORT_MAB_TX_THRESHOLD_XGMII1_TX_THRESHOLD_FIELD_WIDTH,
    LPORT_MAB_TX_THRESHOLD_XGMII1_TX_THRESHOLD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_MAB_TX_THRESHOLD_GMII7_TX_THRESHOLD
 ******************************************************************************/
const ru_field_rec LPORT_MAB_TX_THRESHOLD_GMII7_TX_THRESHOLD_FIELD =
{
    "GMII7_TX_THRESHOLD",
#if RU_INCLUDE_DESC
    "",
    "GMII P7 asynchronous TX FIFO read depth at which packet dequeue starts.",
#endif
    LPORT_MAB_TX_THRESHOLD_GMII7_TX_THRESHOLD_FIELD_MASK,
    0,
    LPORT_MAB_TX_THRESHOLD_GMII7_TX_THRESHOLD_FIELD_WIDTH,
    LPORT_MAB_TX_THRESHOLD_GMII7_TX_THRESHOLD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_MAB_TX_THRESHOLD_GMII6_TX_THRESHOLD
 ******************************************************************************/
const ru_field_rec LPORT_MAB_TX_THRESHOLD_GMII6_TX_THRESHOLD_FIELD =
{
    "GMII6_TX_THRESHOLD",
#if RU_INCLUDE_DESC
    "",
    "GMII P6 asynchronous TX FIFO read depth at which packet dequeue starts.",
#endif
    LPORT_MAB_TX_THRESHOLD_GMII6_TX_THRESHOLD_FIELD_MASK,
    0,
    LPORT_MAB_TX_THRESHOLD_GMII6_TX_THRESHOLD_FIELD_WIDTH,
    LPORT_MAB_TX_THRESHOLD_GMII6_TX_THRESHOLD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_MAB_TX_THRESHOLD_GMII5_TX_THRESHOLD
 ******************************************************************************/
const ru_field_rec LPORT_MAB_TX_THRESHOLD_GMII5_TX_THRESHOLD_FIELD =
{
    "GMII5_TX_THRESHOLD",
#if RU_INCLUDE_DESC
    "",
    "GMII P5 asynchronous TX FIFO read depth at which packet dequeue starts.",
#endif
    LPORT_MAB_TX_THRESHOLD_GMII5_TX_THRESHOLD_FIELD_MASK,
    0,
    LPORT_MAB_TX_THRESHOLD_GMII5_TX_THRESHOLD_FIELD_WIDTH,
    LPORT_MAB_TX_THRESHOLD_GMII5_TX_THRESHOLD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_MAB_TX_THRESHOLD_GMII4_TX_THRESHOLD
 ******************************************************************************/
const ru_field_rec LPORT_MAB_TX_THRESHOLD_GMII4_TX_THRESHOLD_FIELD =
{
    "GMII4_TX_THRESHOLD",
#if RU_INCLUDE_DESC
    "",
    "GMII P4 asynchronous TX FIFO read depth at which packet dequeue starts.",
#endif
    LPORT_MAB_TX_THRESHOLD_GMII4_TX_THRESHOLD_FIELD_MASK,
    0,
    LPORT_MAB_TX_THRESHOLD_GMII4_TX_THRESHOLD_FIELD_WIDTH,
    LPORT_MAB_TX_THRESHOLD_GMII4_TX_THRESHOLD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_MAB_LINK_DOWN_TX_DATA_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_MAB_LINK_DOWN_TX_DATA_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_MAB_LINK_DOWN_TX_DATA_RESERVED0_FIELD_MASK,
    0,
    LPORT_MAB_LINK_DOWN_TX_DATA_RESERVED0_FIELD_WIDTH,
    LPORT_MAB_LINK_DOWN_TX_DATA_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_MAB_LINK_DOWN_TX_DATA_TXCTL
 ******************************************************************************/
const ru_field_rec LPORT_MAB_LINK_DOWN_TX_DATA_TXCTL_FIELD =
{
    "TXCTL",
#if RU_INCLUDE_DESC
    "",
    "When LINK_DOWN_RST_EN = 1 and link is down content of this register is sent to serdes over XGMII interface. "
    " In GMII mode 0 is sent.",
#endif
    LPORT_MAB_LINK_DOWN_TX_DATA_TXCTL_FIELD_MASK,
    0,
    LPORT_MAB_LINK_DOWN_TX_DATA_TXCTL_FIELD_WIDTH,
    LPORT_MAB_LINK_DOWN_TX_DATA_TXCTL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_MAB_LINK_DOWN_TX_DATA_TXD
 ******************************************************************************/
const ru_field_rec LPORT_MAB_LINK_DOWN_TX_DATA_TXD_FIELD =
{
    "TXD",
#if RU_INCLUDE_DESC
    "",
    "When LINK_DOWN_RST_EN = 1 and link is down content of this register is sent to serdes over XGMII interface. "
    "In GMII mode 0's are sent.",
#endif
    LPORT_MAB_LINK_DOWN_TX_DATA_TXD_FIELD_MASK,
    0,
    LPORT_MAB_LINK_DOWN_TX_DATA_TXD_FIELD_WIDTH,
    LPORT_MAB_LINK_DOWN_TX_DATA_TXD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_MAB_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_MAB_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_MAB_STATUS_RESERVED0_FIELD_MASK,
    0,
    LPORT_MAB_STATUS_RESERVED0_FIELD_WIDTH,
    LPORT_MAB_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_MAB_STATUS_XGMII_RX_AFIFO_OVERRUN
 ******************************************************************************/
const ru_field_rec LPORT_MAB_STATUS_XGMII_RX_AFIFO_OVERRUN_FIELD =
{
    "XGMII_RX_AFIFO_OVERRUN",
#if RU_INCLUDE_DESC
    "",
    "10G Port 0 asynchronous RX FIFO over-run status.",
#endif
    LPORT_MAB_STATUS_XGMII_RX_AFIFO_OVERRUN_FIELD_MASK,
    0,
    LPORT_MAB_STATUS_XGMII_RX_AFIFO_OVERRUN_FIELD_WIDTH,
    LPORT_MAB_STATUS_XGMII_RX_AFIFO_OVERRUN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_MAB_STATUS_GMII_RX_AFIFO_OVERRUN_VECT
 ******************************************************************************/
const ru_field_rec LPORT_MAB_STATUS_GMII_RX_AFIFO_OVERRUN_VECT_FIELD =
{
    "GMII_RX_AFIFO_OVERRUN_VECT",
#if RU_INCLUDE_DESC
    "",
    "Port 3-0 asynchronous RX FIFO over-run status.",
#endif
    LPORT_MAB_STATUS_GMII_RX_AFIFO_OVERRUN_VECT_FIELD_MASK,
    0,
    LPORT_MAB_STATUS_GMII_RX_AFIFO_OVERRUN_VECT_FIELD_WIDTH,
    LPORT_MAB_STATUS_GMII_RX_AFIFO_OVERRUN_VECT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_MAB_STATUS_XGMII_TX_FRM_UNDERRUN
 ******************************************************************************/
const ru_field_rec LPORT_MAB_STATUS_XGMII_TX_FRM_UNDERRUN_FIELD =
{
    "XGMII_TX_FRM_UNDERRUN",
#if RU_INCLUDE_DESC
    "",
    "10G Port 0 TX frame under-run status.",
#endif
    LPORT_MAB_STATUS_XGMII_TX_FRM_UNDERRUN_FIELD_MASK,
    0,
    LPORT_MAB_STATUS_XGMII_TX_FRM_UNDERRUN_FIELD_WIDTH,
    LPORT_MAB_STATUS_XGMII_TX_FRM_UNDERRUN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_MAB_STATUS_XGMII_OUTSTANDING_CREDITS_CNT_UNDERRUN
 ******************************************************************************/
const ru_field_rec LPORT_MAB_STATUS_XGMII_OUTSTANDING_CREDITS_CNT_UNDERRUN_FIELD =
{
    "XGMII_OUTSTANDING_CREDITS_CNT_UNDERRUN",
#if RU_INCLUDE_DESC
    "",
    "10G Port 0 TX credits under-run status.",
#endif
    LPORT_MAB_STATUS_XGMII_OUTSTANDING_CREDITS_CNT_UNDERRUN_FIELD_MASK,
    0,
    LPORT_MAB_STATUS_XGMII_OUTSTANDING_CREDITS_CNT_UNDERRUN_FIELD_WIDTH,
    LPORT_MAB_STATUS_XGMII_OUTSTANDING_CREDITS_CNT_UNDERRUN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_MAB_STATUS_GMII_OUTSTANDING_CREDITS_CNT_UNDERRUN_VECT
 ******************************************************************************/
const ru_field_rec LPORT_MAB_STATUS_GMII_OUTSTANDING_CREDITS_CNT_UNDERRUN_VECT_FIELD =
{
    "GMII_OUTSTANDING_CREDITS_CNT_UNDERRUN_VECT",
#if RU_INCLUDE_DESC
    "",
    "Port 3-0 TX credits under-run status.",
#endif
    LPORT_MAB_STATUS_GMII_OUTSTANDING_CREDITS_CNT_UNDERRUN_VECT_FIELD_MASK,
    0,
    LPORT_MAB_STATUS_GMII_OUTSTANDING_CREDITS_CNT_UNDERRUN_VECT_FIELD_WIDTH,
    LPORT_MAB_STATUS_GMII_OUTSTANDING_CREDITS_CNT_UNDERRUN_VECT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_MAB_STATUS_XGMII_TX_AFIFO_OVERRUN
 ******************************************************************************/
const ru_field_rec LPORT_MAB_STATUS_XGMII_TX_AFIFO_OVERRUN_FIELD =
{
    "XGMII_TX_AFIFO_OVERRUN",
#if RU_INCLUDE_DESC
    "",
    "10G Port 0 asynchronous TX FIFO over-run status.",
#endif
    LPORT_MAB_STATUS_XGMII_TX_AFIFO_OVERRUN_FIELD_MASK,
    0,
    LPORT_MAB_STATUS_XGMII_TX_AFIFO_OVERRUN_FIELD_WIDTH,
    LPORT_MAB_STATUS_XGMII_TX_AFIFO_OVERRUN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: LPORT_MAB_STATUS_GMII_TX_AFIFO_OVERRUN_VECT
 ******************************************************************************/
const ru_field_rec LPORT_MAB_STATUS_GMII_TX_AFIFO_OVERRUN_VECT_FIELD =
{
    "GMII_TX_AFIFO_OVERRUN_VECT",
#if RU_INCLUDE_DESC
    "",
    "Port 3-0 asynchronous TX FIFO over-run status.",
#endif
    LPORT_MAB_STATUS_GMII_TX_AFIFO_OVERRUN_VECT_FIELD_MASK,
    0,
    LPORT_MAB_STATUS_GMII_TX_AFIFO_OVERRUN_VECT_FIELD_WIDTH,
    LPORT_MAB_STATUS_GMII_TX_AFIFO_OVERRUN_VECT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: LPORT_MAB_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_MAB_CNTRL_FIELDS[] =
{
    &LPORT_MAB_CNTRL_RESERVED0_FIELD,
    &LPORT_MAB_CNTRL_LINK_DOWN_RST_EN_FIELD,
    &LPORT_MAB_CNTRL_RESERVED1_FIELD,
    &LPORT_MAB_CNTRL_XGMII_TX_RST_FIELD,
    &LPORT_MAB_CNTRL_GMII_TX_RST_FIELD,
    &LPORT_MAB_CNTRL_RESERVED2_FIELD,
    &LPORT_MAB_CNTRL_XGMII_RX_RST_FIELD,
    &LPORT_MAB_CNTRL_GMII_RX_RST_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_MAB_CNTRL_REG = 
{
    "CNTRL",
#if RU_INCLUDE_DESC
    "MSBUS 1 Adaptation Control Register",
    "",
#endif
    LPORT_MAB_CNTRL_REG_OFFSET,
    0,
    0,
    295,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    LPORT_MAB_CNTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LPORT_MAB_TX_WRR_CTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_MAB_TX_WRR_CTRL_FIELDS[] =
{
    &LPORT_MAB_TX_WRR_CTRL_RESERVED0_FIELD,
    &LPORT_MAB_TX_WRR_CTRL_ARB_MODE_FIELD,
    &LPORT_MAB_TX_WRR_CTRL_P7_WEIGHT_FIELD,
    &LPORT_MAB_TX_WRR_CTRL_P6_WEIGHT_FIELD,
    &LPORT_MAB_TX_WRR_CTRL_P5_WEIGHT_FIELD,
    &LPORT_MAB_TX_WRR_CTRL_P4_WEIGHT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_MAB_TX_WRR_CTRL_REG = 
{
    "TX_WRR_CTRL",
#if RU_INCLUDE_DESC
    "MSBUS 1 Adaptation TX WRR Control Register",
    "",
#endif
    LPORT_MAB_TX_WRR_CTRL_REG_OFFSET,
    0,
    0,
    296,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    LPORT_MAB_TX_WRR_CTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LPORT_MAB_TX_THRESHOLD
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_MAB_TX_THRESHOLD_FIELDS[] =
{
    &LPORT_MAB_TX_THRESHOLD_RESERVED0_FIELD,
    &LPORT_MAB_TX_THRESHOLD_XGMII1_TX_THRESHOLD_FIELD,
    &LPORT_MAB_TX_THRESHOLD_GMII7_TX_THRESHOLD_FIELD,
    &LPORT_MAB_TX_THRESHOLD_GMII6_TX_THRESHOLD_FIELD,
    &LPORT_MAB_TX_THRESHOLD_GMII5_TX_THRESHOLD_FIELD,
    &LPORT_MAB_TX_THRESHOLD_GMII4_TX_THRESHOLD_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_MAB_TX_THRESHOLD_REG = 
{
    "TX_THRESHOLD",
#if RU_INCLUDE_DESC
    "MSBUS 1 Adaptation TX Threshold Register",
    "",
#endif
    LPORT_MAB_TX_THRESHOLD_REG_OFFSET,
    0,
    0,
    297,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    LPORT_MAB_TX_THRESHOLD_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LPORT_MAB_LINK_DOWN_TX_DATA
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_MAB_LINK_DOWN_TX_DATA_FIELDS[] =
{
    &LPORT_MAB_LINK_DOWN_TX_DATA_RESERVED0_FIELD,
    &LPORT_MAB_LINK_DOWN_TX_DATA_TXCTL_FIELD,
    &LPORT_MAB_LINK_DOWN_TX_DATA_TXD_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_MAB_LINK_DOWN_TX_DATA_REG = 
{
    "LINK_DOWN_TX_DATA",
#if RU_INCLUDE_DESC
    "MSBUS 1 Adaptation Link down TX Data Register",
    "",
#endif
    LPORT_MAB_LINK_DOWN_TX_DATA_REG_OFFSET,
    0,
    0,
    298,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    LPORT_MAB_LINK_DOWN_TX_DATA_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LPORT_MAB_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_MAB_STATUS_FIELDS[] =
{
    &LPORT_MAB_STATUS_RESERVED0_FIELD,
    &LPORT_MAB_STATUS_XGMII_RX_AFIFO_OVERRUN_FIELD,
    &LPORT_MAB_STATUS_GMII_RX_AFIFO_OVERRUN_VECT_FIELD,
    &LPORT_MAB_STATUS_XGMII_TX_FRM_UNDERRUN_FIELD,
    &LPORT_MAB_STATUS_XGMII_OUTSTANDING_CREDITS_CNT_UNDERRUN_FIELD,
    &LPORT_MAB_STATUS_GMII_OUTSTANDING_CREDITS_CNT_UNDERRUN_VECT_FIELD,
    &LPORT_MAB_STATUS_XGMII_TX_AFIFO_OVERRUN_FIELD,
    &LPORT_MAB_STATUS_GMII_TX_AFIFO_OVERRUN_VECT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_MAB_STATUS_REG = 
{
    "STATUS",
#if RU_INCLUDE_DESC
    "MSBUS 1 Adaptation Status Register",
    "",
#endif
    LPORT_MAB_STATUS_REG_OFFSET,
    0,
    0,
    299,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    LPORT_MAB_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: LPORT_MAB
 ******************************************************************************/
static const ru_reg_rec *LPORT_MAB_REGS[] =
{
    &LPORT_MAB_CNTRL_REG,
    &LPORT_MAB_TX_WRR_CTRL_REG,
    &LPORT_MAB_TX_THRESHOLD_REG,
    &LPORT_MAB_LINK_DOWN_TX_DATA_REG,
    &LPORT_MAB_STATUS_REG,
};

unsigned long LPORT_MAB_ADDRS[] =
{
    0x8013d700,
    0x8013d800,
};

const ru_block_rec LPORT_MAB_BLOCK = 
{
    "LPORT_MAB",
    LPORT_MAB_ADDRS,
    2,
    5,
    LPORT_MAB_REGS
};

/* End of file BCM6858_A0LPORT_MAB.c */
