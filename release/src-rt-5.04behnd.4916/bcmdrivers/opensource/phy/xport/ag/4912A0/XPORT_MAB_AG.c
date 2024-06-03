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
 * Field: XPORT_MAB_CONTROL_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_MAB_CONTROL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_MAB_CONTROL_RESERVED0_FIELD_MASK,
    0,
    XPORT_MAB_CONTROL_RESERVED0_FIELD_WIDTH,
    XPORT_MAB_CONTROL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MAB_CONTROL_TX_CREDIT_DISAB
 ******************************************************************************/
const ru_field_rec XPORT_MAB_CONTROL_TX_CREDIT_DISAB_FIELD =
{
    "TX_CREDIT_DISAB",
#if RU_INCLUDE_DESC
    "",
    "When bit <i> is set, MSBUS adaptation TX logic for port<i> will stop sending credits to XLMAC.",
#endif
    XPORT_MAB_CONTROL_TX_CREDIT_DISAB_FIELD_MASK,
    0,
    XPORT_MAB_CONTROL_TX_CREDIT_DISAB_FIELD_WIDTH,
    XPORT_MAB_CONTROL_TX_CREDIT_DISAB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MAB_CONTROL_TX_FIFO_RST
 ******************************************************************************/
const ru_field_rec XPORT_MAB_CONTROL_TX_FIFO_RST_FIELD =
{
    "TX_FIFO_RST",
#if RU_INCLUDE_DESC
    "",
    "When bit <i> is set, MSBUS adaptation TX FIFO for port<i> is initialized.",
#endif
    XPORT_MAB_CONTROL_TX_FIFO_RST_FIELD_MASK,
    0,
    XPORT_MAB_CONTROL_TX_FIFO_RST_FIELD_WIDTH,
    XPORT_MAB_CONTROL_TX_FIFO_RST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MAB_CONTROL_TX_PORT_RST
 ******************************************************************************/
const ru_field_rec XPORT_MAB_CONTROL_TX_PORT_RST_FIELD =
{
    "TX_PORT_RST",
#if RU_INCLUDE_DESC
    "",
    "When bit <i> is set, all MSBUS adaptation TX logic for port<i> is initialized.\n"
    "Although no new credits will be sent to XLMAC for the port (same as when TX_CREDIT_DISAB[i]=1 is set), "
    " the TX TDM scheduling logic will keep scheduling TDM slots for the port to ensure XLMAC is able to drain its remaining MSBUS TX credits for the port.\n"
    "Note that TX_PORT_RST[i]=1 implies, among other things, TX_FIFO_RST[i]=1 and TX_CREDIT_DISAB[i]=1 action, regardless of whether those register bits are set or not.",
#endif
    XPORT_MAB_CONTROL_TX_PORT_RST_FIELD_MASK,
    0,
    XPORT_MAB_CONTROL_TX_PORT_RST_FIELD_WIDTH,
    XPORT_MAB_CONTROL_TX_PORT_RST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MAB_CONTROL_RX_PORT_RST
 ******************************************************************************/
const ru_field_rec XPORT_MAB_CONTROL_RX_PORT_RST_FIELD =
{
    "RX_PORT_RST",
#if RU_INCLUDE_DESC
    "",
    "When bit <i> is set, MSBUS adaptation RX logic for port<i> is initialized.",
#endif
    XPORT_MAB_CONTROL_RX_PORT_RST_FIELD_MASK,
    0,
    XPORT_MAB_CONTROL_RX_PORT_RST_FIELD_WIDTH,
    XPORT_MAB_CONTROL_RX_PORT_RST_FIELD_SHIFT,
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
    "TDM Arbiter/Scheduler Mode.\n"
    "1'b0 - Fixed Mode. TDM slots allocation is not affected by the port activity.\n"
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
    "P0 weight expressed in TDM time slots.",
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
 * Field: XPORT_MAB_TX_THRESHOLD_XGMII3_TX_THRESHOLD
 ******************************************************************************/
const ru_field_rec XPORT_MAB_TX_THRESHOLD_XGMII3_TX_THRESHOLD_FIELD =
{
    "XGMII3_TX_THRESHOLD",
#if RU_INCLUDE_DESC
    "",
    "XGMII P3 asynchronous TX FIFO read depth at which packet dequeue starts.",
#endif
    XPORT_MAB_TX_THRESHOLD_XGMII3_TX_THRESHOLD_FIELD_MASK,
    0,
    XPORT_MAB_TX_THRESHOLD_XGMII3_TX_THRESHOLD_FIELD_WIDTH,
    XPORT_MAB_TX_THRESHOLD_XGMII3_TX_THRESHOLD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MAB_TX_THRESHOLD_XGMII2_TX_THRESHOLD
 ******************************************************************************/
const ru_field_rec XPORT_MAB_TX_THRESHOLD_XGMII2_TX_THRESHOLD_FIELD =
{
    "XGMII2_TX_THRESHOLD",
#if RU_INCLUDE_DESC
    "",
    "XGMII P2 asynchronous TX FIFO read depth at which packet dequeue starts.",
#endif
    XPORT_MAB_TX_THRESHOLD_XGMII2_TX_THRESHOLD_FIELD_MASK,
    0,
    XPORT_MAB_TX_THRESHOLD_XGMII2_TX_THRESHOLD_FIELD_WIDTH,
    XPORT_MAB_TX_THRESHOLD_XGMII2_TX_THRESHOLD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_MAB_TX_THRESHOLD_XGMII1_TX_THRESHOLD
 ******************************************************************************/
const ru_field_rec XPORT_MAB_TX_THRESHOLD_XGMII1_TX_THRESHOLD_FIELD =
{
    "XGMII1_TX_THRESHOLD",
#if RU_INCLUDE_DESC
    "",
    "XGMII P1 asynchronous TX FIFO read depth at which packet dequeue starts.",
#endif
    XPORT_MAB_TX_THRESHOLD_XGMII1_TX_THRESHOLD_FIELD_MASK,
    0,
    XPORT_MAB_TX_THRESHOLD_XGMII1_TX_THRESHOLD_FIELD_WIDTH,
    XPORT_MAB_TX_THRESHOLD_XGMII1_TX_THRESHOLD_FIELD_SHIFT,
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
    "XGMII P0 asynchronous TX FIFO read depth at which packet dequeue starts.",
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
 * Field: XPORT_MAB_STATUS_TX_FRM_UNDERRUN_VECT
 ******************************************************************************/
const ru_field_rec XPORT_MAB_STATUS_TX_FRM_UNDERRUN_VECT_FIELD =
{
    "TX_FRM_UNDERRUN_VECT",
#if RU_INCLUDE_DESC
    "",
    "Port 3:0 TX frame under-run status.",
#endif
    XPORT_MAB_STATUS_TX_FRM_UNDERRUN_VECT_FIELD_MASK,
    0,
    XPORT_MAB_STATUS_TX_FRM_UNDERRUN_VECT_FIELD_WIDTH,
    XPORT_MAB_STATUS_TX_FRM_UNDERRUN_VECT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPORT_MAB_STATUS_TX_OUTSTANDING_CREDITS_CNT_UNDERRUN_VECT
 ******************************************************************************/
const ru_field_rec XPORT_MAB_STATUS_TX_OUTSTANDING_CREDITS_CNT_UNDERRUN_VECT_FIELD =
{
    "TX_OUTSTANDING_CREDITS_CNT_UNDERRUN_VECT",
#if RU_INCLUDE_DESC
    "",
    "Port 3:0 TX outstanding credits counter under-run status.",
#endif
    XPORT_MAB_STATUS_TX_OUTSTANDING_CREDITS_CNT_UNDERRUN_VECT_FIELD_MASK,
    0,
    XPORT_MAB_STATUS_TX_OUTSTANDING_CREDITS_CNT_UNDERRUN_VECT_FIELD_WIDTH,
    XPORT_MAB_STATUS_TX_OUTSTANDING_CREDITS_CNT_UNDERRUN_VECT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPORT_MAB_STATUS_TX_FIFO_OVERRUN_VECT
 ******************************************************************************/
const ru_field_rec XPORT_MAB_STATUS_TX_FIFO_OVERRUN_VECT_FIELD =
{
    "TX_FIFO_OVERRUN_VECT",
#if RU_INCLUDE_DESC
    "",
    "Port 3:0 asynchronous TX FIFO over-run status.",
#endif
    XPORT_MAB_STATUS_TX_FIFO_OVERRUN_VECT_FIELD_MASK,
    0,
    XPORT_MAB_STATUS_TX_FIFO_OVERRUN_VECT_FIELD_WIDTH,
    XPORT_MAB_STATUS_TX_FIFO_OVERRUN_VECT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPORT_MAB_STATUS_RESERVED1
 ******************************************************************************/
const ru_field_rec XPORT_MAB_STATUS_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_MAB_STATUS_RESERVED1_FIELD_MASK,
    0,
    XPORT_MAB_STATUS_RESERVED1_FIELD_WIDTH,
    XPORT_MAB_STATUS_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPORT_MAB_STATUS_RX_FIFO_OVERRUN_VECT
 ******************************************************************************/
const ru_field_rec XPORT_MAB_STATUS_RX_FIFO_OVERRUN_VECT_FIELD =
{
    "RX_FIFO_OVERRUN_VECT",
#if RU_INCLUDE_DESC
    "",
    "Port 3:0 asynchronous RX FIFO over-run status.",
#endif
    XPORT_MAB_STATUS_RX_FIFO_OVERRUN_VECT_FIELD_MASK,
    0,
    XPORT_MAB_STATUS_RX_FIFO_OVERRUN_VECT_FIELD_WIDTH,
    XPORT_MAB_STATUS_RX_FIFO_OVERRUN_VECT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: XPORT_MAB_CONTROL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_MAB_CONTROL_FIELDS[] =
{
    &XPORT_MAB_CONTROL_RESERVED0_FIELD,
    &XPORT_MAB_CONTROL_TX_CREDIT_DISAB_FIELD,
    &XPORT_MAB_CONTROL_TX_FIFO_RST_FIELD,
    &XPORT_MAB_CONTROL_TX_PORT_RST_FIELD,
    &XPORT_MAB_CONTROL_RX_PORT_RST_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_MAB_CONTROL_REG = 
{
    "CONTROL",
#if RU_INCLUDE_DESC
    "MSBUS Adaptation Control Register",
    "",
#endif
    XPORT_MAB_CONTROL_REG_OFFSET,
    0,
    0,
    226,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    XPORT_MAB_CONTROL_FIELDS,
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
    "MSBUS Adaptation TX WRR Arbiter Configuration Register",
    "Note: in 4-port mode, MSBUS clock should be set to >= 4*MAX_PORT_RATE/64b.",
#endif
    XPORT_MAB_TX_WRR_CTRL_REG_OFFSET,
    0,
    0,
    227,
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
    &XPORT_MAB_TX_THRESHOLD_XGMII3_TX_THRESHOLD_FIELD,
    &XPORT_MAB_TX_THRESHOLD_XGMII2_TX_THRESHOLD_FIELD,
    &XPORT_MAB_TX_THRESHOLD_XGMII1_TX_THRESHOLD_FIELD,
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
    228,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    XPORT_MAB_TX_THRESHOLD_FIELDS,
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
    &XPORT_MAB_STATUS_TX_FRM_UNDERRUN_VECT_FIELD,
    &XPORT_MAB_STATUS_TX_OUTSTANDING_CREDITS_CNT_UNDERRUN_VECT_FIELD,
    &XPORT_MAB_STATUS_TX_FIFO_OVERRUN_VECT_FIELD,
    &XPORT_MAB_STATUS_RESERVED1_FIELD,
    &XPORT_MAB_STATUS_RX_FIFO_OVERRUN_VECT_FIELD,
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
    229,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    XPORT_MAB_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: XPORT_MAB
 ******************************************************************************/
static const ru_reg_rec *XPORT_MAB_REGS[] =
{
    &XPORT_MAB_CONTROL_REG,
    &XPORT_MAB_TX_WRR_CTRL_REG,
    &XPORT_MAB_TX_THRESHOLD_REG,
    &XPORT_MAB_STATUS_REG,
};

unsigned long XPORT_MAB_ADDRS[] =
{
    0x837f3300,
    0x837f7300,
};

const ru_block_rec XPORT_MAB_BLOCK = 
{
    "XPORT_MAB",
    XPORT_MAB_ADDRS,
    2,
    4,
    XPORT_MAB_REGS
};

/* End of file XPORT_MAB.c */
