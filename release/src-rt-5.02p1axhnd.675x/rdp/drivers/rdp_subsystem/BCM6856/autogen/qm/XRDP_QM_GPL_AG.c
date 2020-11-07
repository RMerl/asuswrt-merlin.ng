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

/******************************************************************************
 * Register: QM_GLOBAL_CFG_QM_ENABLE_CTRL
 ******************************************************************************/
const ru_reg_rec QM_GLOBAL_CFG_QM_ENABLE_CTRL_REG = 
{
    "GLOBAL_CFG_QM_ENABLE_CTRL",
#if RU_INCLUDE_DESC
    "QM_ENABLE_CTRL Register",
    "QM Enable register",
#endif
    QM_GLOBAL_CFG_QM_ENABLE_CTRL_REG_OFFSET,
    0,
    0,
    0,
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG_QM_SW_RST_CTRL
 ******************************************************************************/
const ru_reg_rec QM_GLOBAL_CFG_QM_SW_RST_CTRL_REG = 
{
    "GLOBAL_CFG_QM_SW_RST_CTRL",
#if RU_INCLUDE_DESC
    "QM_SW_RST_CTRL Register",
    "QM soft reset register",
#endif
    QM_GLOBAL_CFG_QM_SW_RST_CTRL_REG_OFFSET,
    0,
    0,
    1,
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG_QM_GENERAL_CTRL
 ******************************************************************************/
const ru_reg_rec QM_GLOBAL_CFG_QM_GENERAL_CTRL_REG = 
{
    "GLOBAL_CFG_QM_GENERAL_CTRL",
#if RU_INCLUDE_DESC
    "QM_GENERAL_CTRL Register",
    "QM Enable register",
#endif
    QM_GLOBAL_CFG_QM_GENERAL_CTRL_REG_OFFSET,
    0,
    0,
    2,
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG_FPM_CONTROL
 ******************************************************************************/
const ru_reg_rec QM_GLOBAL_CFG_FPM_CONTROL_REG = 
{
    "GLOBAL_CFG_FPM_CONTROL",
#if RU_INCLUDE_DESC
    "FPM_CONTROL Register",
    "FPM Control Register",
#endif
    QM_GLOBAL_CFG_FPM_CONTROL_REG_OFFSET,
    0,
    0,
    3,
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_CONTROL
 ******************************************************************************/
const ru_reg_rec QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_CONTROL_REG = 
{
    "GLOBAL_CFG_DDR_BYTE_CONGESTION_CONTROL",
#if RU_INCLUDE_DESC
    "DDR_BYTE_CONGESTION_CONTROL Register",
    "DDR Byte Congestion Control Register",
#endif
    QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_CONTROL_REG_OFFSET,
    0,
    0,
    4,
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_LOWER_THR
 ******************************************************************************/
const ru_reg_rec QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_LOWER_THR_REG = 
{
    "GLOBAL_CFG_DDR_BYTE_CONGESTION_LOWER_THR",
#if RU_INCLUDE_DESC
    "DDR_BYTE_CONGESTION_LOWER_THR Register",
    "DDR Byte Congestion Lower Threshold",
#endif
    QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_LOWER_THR_REG_OFFSET,
    0,
    0,
    5,
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_MID_THR
 ******************************************************************************/
const ru_reg_rec QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_MID_THR_REG = 
{
    "GLOBAL_CFG_DDR_BYTE_CONGESTION_MID_THR",
#if RU_INCLUDE_DESC
    "DDR_BYTE_CONGESTION_MID_THR Register",
    "DDR Byte Congestion Middle Threshold",
#endif
    QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_MID_THR_REG_OFFSET,
    0,
    0,
    6,
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_HIGHER_THR
 ******************************************************************************/
const ru_reg_rec QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_HIGHER_THR_REG = 
{
    "GLOBAL_CFG_DDR_BYTE_CONGESTION_HIGHER_THR",
#if RU_INCLUDE_DESC
    "DDR_BYTE_CONGESTION_HIGHER_THR Register",
    "DDR Byte Congestion Higher Threshold",
#endif
    QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_HIGHER_THR_REG_OFFSET,
    0,
    0,
    7,
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL
 ******************************************************************************/
const ru_reg_rec QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_REG = 
{
    "GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL",
#if RU_INCLUDE_DESC
    "DDR_PD_CONGESTION_CONTROL Register",
    "DDR PD Congestion Control Register",
#endif
    QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_REG_OFFSET,
    0,
    0,
    8,
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG_QM_PD_CONGESTION_CONTROL
 ******************************************************************************/
const ru_reg_rec QM_GLOBAL_CFG_QM_PD_CONGESTION_CONTROL_REG = 
{
    "GLOBAL_CFG_QM_PD_CONGESTION_CONTROL",
#if RU_INCLUDE_DESC
    "QM_PD_CONGESTION_CONTROL Register",
    "QM PD Congestion Control Register",
#endif
    QM_GLOBAL_CFG_QM_PD_CONGESTION_CONTROL_REG_OFFSET,
    0,
    0,
    9,
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG_ABS_DROP_QUEUE
 ******************************************************************************/
const ru_reg_rec QM_GLOBAL_CFG_ABS_DROP_QUEUE_REG = 
{
    "GLOBAL_CFG_ABS_DROP_QUEUE",
#if RU_INCLUDE_DESC
    "ABS_DROP_QUEUE Register",
    "Absolute Adress drop queue",
#endif
    QM_GLOBAL_CFG_ABS_DROP_QUEUE_REG_OFFSET,
    0,
    0,
    10,
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG_AGGREGATION_CTRL
 ******************************************************************************/
const ru_reg_rec QM_GLOBAL_CFG_AGGREGATION_CTRL_REG = 
{
    "GLOBAL_CFG_AGGREGATION_CTRL",
#if RU_INCLUDE_DESC
    "AGGREGATION_CTRL Register",
    "Aggregation Control register",
#endif
    QM_GLOBAL_CFG_AGGREGATION_CTRL_REG_OFFSET,
    0,
    0,
    11,
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG_FPM_BASE_ADDR
 ******************************************************************************/
const ru_reg_rec QM_GLOBAL_CFG_FPM_BASE_ADDR_REG = 
{
    "GLOBAL_CFG_FPM_BASE_ADDR",
#if RU_INCLUDE_DESC
    "FPM_BASE_ADDR Register",
    "FPM Base Address",
#endif
    QM_GLOBAL_CFG_FPM_BASE_ADDR_REG_OFFSET,
    0,
    0,
    12,
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG_FPM_COHERENT_BASE_ADDR
 ******************************************************************************/
const ru_reg_rec QM_GLOBAL_CFG_FPM_COHERENT_BASE_ADDR_REG = 
{
    "GLOBAL_CFG_FPM_COHERENT_BASE_ADDR",
#if RU_INCLUDE_DESC
    "FPM_COHERENT_BASE_ADDR Register",
    "FPM Base Address for PDs that have the coherent bit set",
#endif
    QM_GLOBAL_CFG_FPM_COHERENT_BASE_ADDR_REG_OFFSET,
    0,
    0,
    13,
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG_DDR_SOP_OFFSET
 ******************************************************************************/
const ru_reg_rec QM_GLOBAL_CFG_DDR_SOP_OFFSET_REG = 
{
    "GLOBAL_CFG_DDR_SOP_OFFSET",
#if RU_INCLUDE_DESC
    "DDR_SOP_OFFSET Register",
    "DDR SOP Offset options",
#endif
    QM_GLOBAL_CFG_DDR_SOP_OFFSET_REG_OFFSET,
    0,
    0,
    14,
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL
 ******************************************************************************/
const ru_reg_rec QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_REG = 
{
    "GLOBAL_CFG_EPON_OVERHEAD_CTRL",
#if RU_INCLUDE_DESC
    "EPON_OVERHEAD_CTRL Register",
    "EPON Ghost reporting configuration",
#endif
    QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_REG_OFFSET,
    0,
    0,
    15,
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG_DQM_FULL
 ******************************************************************************/
const ru_reg_rec QM_GLOBAL_CFG_DQM_FULL_REG = 
{
    "GLOBAL_CFG_DQM_FULL",
#if RU_INCLUDE_DESC
    "DQM_FULL %i Register",
    "Queue Full indication"
    "Each register includes a batch of 32 queues non-empty indication."
    "9 Batches are need for 288 queues."
    "First Batch is for queues 31-0 and so on until the last batch representing queues 287-256.",
#endif
    QM_GLOBAL_CFG_DQM_FULL_REG_OFFSET,
    QM_GLOBAL_CFG_DQM_FULL_REG_RAM_CNT,
    4,
    16,
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG_DQM_NOT_EMPTY
 ******************************************************************************/
const ru_reg_rec QM_GLOBAL_CFG_DQM_NOT_EMPTY_REG = 
{
    "GLOBAL_CFG_DQM_NOT_EMPTY",
#if RU_INCLUDE_DESC
    "DQM_NOT_EMPTY %i Register",
    "Queue Not Empty indication"
    "Each register includes a batch of 32 queues non-empty indication."
    "9 Batches are need for 288 queues."
    "First Batch is for queues 31-0 and so on until the last batch representing queues 287-256.",
#endif
    QM_GLOBAL_CFG_DQM_NOT_EMPTY_REG_OFFSET,
    QM_GLOBAL_CFG_DQM_NOT_EMPTY_REG_RAM_CNT,
    4,
    17,
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG_DQM_POP_READY
 ******************************************************************************/
const ru_reg_rec QM_GLOBAL_CFG_DQM_POP_READY_REG = 
{
    "GLOBAL_CFG_DQM_POP_READY",
#if RU_INCLUDE_DESC
    "DQM_POP_READY %i Register",
    "Queue pop ready indication (Some queues may be non-empty, but due to PD offload they are not immediatly ready to be popped. Pop can be issued, but in this case the result could be delayed)."
    "Each register includes a batch of 32 queues non-empty indication."
    "9 Batches are need for 288 queues."
    "First Batch is for queues 31-0 and so on until the last batch representing queues 287-256.",
#endif
    QM_GLOBAL_CFG_DQM_POP_READY_REG_OFFSET,
    QM_GLOBAL_CFG_DQM_POP_READY_REG_RAM_CNT,
    4,
    18,
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG_AGGREGATION_CONTEXT_VALID
 ******************************************************************************/
const ru_reg_rec QM_GLOBAL_CFG_AGGREGATION_CONTEXT_VALID_REG = 
{
    "GLOBAL_CFG_AGGREGATION_CONTEXT_VALID",
#if RU_INCLUDE_DESC
    "AGGREGATION_CONTEXT_VALID %i Register",
    "Aggregation context valid."
    "This indicates that the queue is in the process of packet aggregation."
    "Each register includes a batch of 32 queues aggregation valid indication."
    "9 Batches are need for 288 queues."
    "First Batch is for queues 31-0 and so on until the last batch representing queues 287-256.",
#endif
    QM_GLOBAL_CFG_AGGREGATION_CONTEXT_VALID_REG_OFFSET,
    QM_GLOBAL_CFG_AGGREGATION_CONTEXT_VALID_REG_RAM_CNT,
    4,
    19,
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL
 ******************************************************************************/
const ru_reg_rec QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_REG = 
{
    "GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL",
#if RU_INCLUDE_DESC
    "QM_AGGREGATION_TIMER_CTRL Register",
    "Open aggregation will be forced to close after internal timer expiration."
    "The first byte (0-7bits) controls the granularity of the internal counter (valid value 0x0-0x3)"
    "The second byte (8-15bits) controls the timout value (valid values 0x0-0x7), which is counted according to granularity cycles."
    "the 16bit is enable for the mechanism",
#endif
    QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_REG_OFFSET,
    0,
    0,
    20,
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG_QM_FPM_UG_GBL_CNT
 ******************************************************************************/
const ru_reg_rec QM_GLOBAL_CFG_QM_FPM_UG_GBL_CNT_REG = 
{
    "GLOBAL_CFG_QM_FPM_UG_GBL_CNT",
#if RU_INCLUDE_DESC
    "QM_FPM_UG_GBL_CNT Register",
    "FPM global user group counter:"
    "UG0-3 + UG7"
    ""
    "",
#endif
    QM_GLOBAL_CFG_QM_FPM_UG_GBL_CNT_REG_OFFSET,
    0,
    0,
    21,
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE
 ******************************************************************************/
const ru_reg_rec QM_GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE_REG = 
{
    "GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE",
#if RU_INCLUDE_DESC
    "QM_EGRESS_FLUSH_QUEUE Register",
    "0-8b: queue to flush"
    "9b:   enable flush"
    ""
    ""
    ""
    "",
#endif
    QM_GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE_REG_OFFSET,
    0,
    0,
    22,
};

/******************************************************************************
 * Register: QM_FPM_POOLS_THR
 ******************************************************************************/
const ru_reg_rec QM_FPM_POOLS_THR_REG = 
{
    "FPM_POOLS_THR",
#if RU_INCLUDE_DESC
    "THR Register",
    "Hold 2 thresholds per FPM pool for priority management",
#endif
    QM_FPM_POOLS_THR_REG_OFFSET,
    QM_FPM_POOLS_THR_REG_RAM_CNT,
    32,
    23,
};

/******************************************************************************
 * Register: QM_FPM_USR_GRP_LOWER_THR
 ******************************************************************************/
const ru_reg_rec QM_FPM_USR_GRP_LOWER_THR_REG = 
{
    "FPM_USR_GRP_LOWER_THR",
#if RU_INCLUDE_DESC
    "LOWER_THR Register",
    "Holds FPM user group lower threshold.",
#endif
    QM_FPM_USR_GRP_LOWER_THR_REG_OFFSET,
    QM_FPM_USR_GRP_LOWER_THR_REG_RAM_CNT,
    32,
    24,
};

/******************************************************************************
 * Register: QM_FPM_USR_GRP_MID_THR
 ******************************************************************************/
const ru_reg_rec QM_FPM_USR_GRP_MID_THR_REG = 
{
    "FPM_USR_GRP_MID_THR",
#if RU_INCLUDE_DESC
    "MID_THR Register",
    "Holds FPM user group middle threshold."
    "*IMPORTANT* if buffer reservations is enabled, the following should be honored:"
    "HIGHER_THR-MID_THR > 16",
#endif
    QM_FPM_USR_GRP_MID_THR_REG_OFFSET,
    QM_FPM_USR_GRP_MID_THR_REG_RAM_CNT,
    32,
    25,
};

/******************************************************************************
 * Register: QM_FPM_USR_GRP_HIGHER_THR
 ******************************************************************************/
const ru_reg_rec QM_FPM_USR_GRP_HIGHER_THR_REG = 
{
    "FPM_USR_GRP_HIGHER_THR",
#if RU_INCLUDE_DESC
    "HIGHER_THR Register",
    "Holds FPM user group higher threshold."
    "*IMPORTANT* if buffer reservations is enabled, the following should be honored:"
    "HIGHER_THR-MID_THR > 16",
#endif
    QM_FPM_USR_GRP_HIGHER_THR_REG_OFFSET,
    QM_FPM_USR_GRP_HIGHER_THR_REG_RAM_CNT,
    32,
    26,
};

/******************************************************************************
 * Register: QM_FPM_USR_GRP_CNT
 ******************************************************************************/
const ru_reg_rec QM_FPM_USR_GRP_CNT_REG = 
{
    "FPM_USR_GRP_CNT",
#if RU_INCLUDE_DESC
    "CNT Register",
    "FPM user group buffer counter",
#endif
    QM_FPM_USR_GRP_CNT_REG_OFFSET,
    QM_FPM_USR_GRP_CNT_REG_RAM_CNT,
    32,
    27,
};

/******************************************************************************
 * Register: QM_RUNNER_GRP_RNR_CONFIG
 ******************************************************************************/
const ru_reg_rec QM_RUNNER_GRP_RNR_CONFIG_REG = 
{
    "RUNNER_GRP_RNR_CONFIG",
#if RU_INCLUDE_DESC
    "RNR_CONFIG Register",
    "Runners Configurations",
#endif
    QM_RUNNER_GRP_RNR_CONFIG_REG_OFFSET,
    QM_RUNNER_GRP_RNR_CONFIG_REG_RAM_CNT,
    16,
    28,
};

/******************************************************************************
 * Register: QM_RUNNER_GRP_QUEUE_CONFIG
 ******************************************************************************/
const ru_reg_rec QM_RUNNER_GRP_QUEUE_CONFIG_REG = 
{
    "RUNNER_GRP_QUEUE_CONFIG",
#if RU_INCLUDE_DESC
    "QUEUE_CONFIG Register",
    "Consecutive queues which are associated with this runner",
#endif
    QM_RUNNER_GRP_QUEUE_CONFIG_REG_OFFSET,
    QM_RUNNER_GRP_QUEUE_CONFIG_REG_RAM_CNT,
    16,
    29,
};

/******************************************************************************
 * Register: QM_RUNNER_GRP_PDFIFO_CONFIG
 ******************************************************************************/
const ru_reg_rec QM_RUNNER_GRP_PDFIFO_CONFIG_REG = 
{
    "RUNNER_GRP_PDFIFO_CONFIG",
#if RU_INCLUDE_DESC
    "PDFIFO_CONFIG Register",
    "head of the queue PD FIFO attributes",
#endif
    QM_RUNNER_GRP_PDFIFO_CONFIG_REG_OFFSET,
    QM_RUNNER_GRP_PDFIFO_CONFIG_REG_RAM_CNT,
    16,
    30,
};

/******************************************************************************
 * Register: QM_RUNNER_GRP_UPDATE_FIFO_CONFIG
 ******************************************************************************/
const ru_reg_rec QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_REG = 
{
    "RUNNER_GRP_UPDATE_FIFO_CONFIG",
#if RU_INCLUDE_DESC
    "UPDATE_FIFO_CONFIG Register",
    "Update FIFO attributes",
#endif
    QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_REG_OFFSET,
    QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_REG_RAM_CNT,
    16,
    31,
};

/******************************************************************************
 * Register: QM_INTR_CTRL_ISR
 ******************************************************************************/
const ru_reg_rec QM_INTR_CTRL_ISR_REG = 
{
    "INTR_CTRL_ISR",
#if RU_INCLUDE_DESC
    "INTERRUPT_STATUS_REGISTER Register",
    "This register contains the current active QM interrupts. Each asserted bit represents an active interrupt source. The interrupt remains active until the software clears it by writing 1 to the corresponding bit.",
#endif
    QM_INTR_CTRL_ISR_REG_OFFSET,
    0,
    0,
    32,
};

/******************************************************************************
 * Register: QM_INTR_CTRL_ISM
 ******************************************************************************/
const ru_reg_rec QM_INTR_CTRL_ISM_REG = 
{
    "INTR_CTRL_ISM",
#if RU_INCLUDE_DESC
    "INTERRUPT_STATUS_MASKED_REGISTER Register",
    "This register provides only the  enabled interrupts for each of the interrupt sources depicted in the ISR register.",
#endif
    QM_INTR_CTRL_ISM_REG_OFFSET,
    0,
    0,
    33,
};

/******************************************************************************
 * Register: QM_INTR_CTRL_IER
 ******************************************************************************/
const ru_reg_rec QM_INTR_CTRL_IER_REG = 
{
    "INTR_CTRL_IER",
#if RU_INCLUDE_DESC
    "INTERRUPT_ENABLE_REGISTER Register",
    "This register provides an enable mask for each of the interrupt sources depicted in the ISR register.",
#endif
    QM_INTR_CTRL_IER_REG_OFFSET,
    0,
    0,
    34,
};

/******************************************************************************
 * Register: QM_INTR_CTRL_ITR
 ******************************************************************************/
const ru_reg_rec QM_INTR_CTRL_ITR_REG = 
{
    "INTR_CTRL_ITR",
#if RU_INCLUDE_DESC
    "INTERRUPT_TEST_REGISTER Register",
    "This register enables testing by simulating interrupt sources. When the software sets a bit in the ITR, the corresponding bit in the ISR shows an active interrupt. The interrupt remains active until software clears the bit in the ITR",
#endif
    QM_INTR_CTRL_ITR_REG_OFFSET,
    0,
    0,
    35,
};

/******************************************************************************
 * Register: QM_CLK_GATE_CLK_GATE_CNTRL
 ******************************************************************************/
const ru_reg_rec QM_CLK_GATE_CLK_GATE_CNTRL_REG = 
{
    "CLK_GATE_CLK_GATE_CNTRL",
#if RU_INCLUDE_DESC
    "CLOCK_GATE_CONTROL Register",
    "Clock Gate control register including timer config and bypass control",
#endif
    QM_CLK_GATE_CLK_GATE_CNTRL_REG_OFFSET,
    0,
    0,
    36,
};

/******************************************************************************
 * Register: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL
 ******************************************************************************/
const ru_reg_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_REG = 
{
    "CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL",
#if RU_INCLUDE_DESC
    "CPU_PD_INDIRECT_CTRL Register",
    "CPU PD Indirect Access Control",
#endif
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_REG_OFFSET,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_REG_RAM_CNT,
    64,
    37,
};

/******************************************************************************
 * Register: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_0
 ******************************************************************************/
const ru_reg_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_0_REG = 
{
    "CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_0",
#if RU_INCLUDE_DESC
    "CPU_PD_INDIRECT_WR_DATA %i Register",
    "CPU PD Indirect Write data to DQM."
    "First entry represents PD[127:96] and so on until the last entry representing PD[31:0].",
#endif
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_0_REG_OFFSET,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_0_REG_RAM_CNT,
    64,
    38,
};

/******************************************************************************
 * Register: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_1
 ******************************************************************************/
const ru_reg_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_1_REG = 
{
    "CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_1",
#if RU_INCLUDE_DESC
    "CPU_PD_INDIRECT_WR_DATA %i Register",
    "CPU PD Indirect Write data to DQM."
    "First entry represents PD[127:96] and so on until the last entry representing PD[31:0].",
#endif
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_1_REG_OFFSET,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_1_REG_RAM_CNT,
    64,
    39,
};

/******************************************************************************
 * Register: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_2
 ******************************************************************************/
const ru_reg_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_2_REG = 
{
    "CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_2",
#if RU_INCLUDE_DESC
    "CPU_PD_INDIRECT_WR_DATA %i Register",
    "CPU PD Indirect Write data to DQM."
    "First entry represents PD[127:96] and so on until the last entry representing PD[31:0].",
#endif
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_2_REG_OFFSET,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_2_REG_RAM_CNT,
    64,
    40,
};

/******************************************************************************
 * Register: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_3
 ******************************************************************************/
const ru_reg_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_3_REG = 
{
    "CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_3",
#if RU_INCLUDE_DESC
    "CPU_PD_INDIRECT_WR_DATA %i Register",
    "CPU PD Indirect Write data to DQM."
    "First entry represents PD[127:96] and so on until the last entry representing PD[31:0].",
#endif
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_3_REG_OFFSET,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_3_REG_RAM_CNT,
    64,
    41,
};

/******************************************************************************
 * Register: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_0
 ******************************************************************************/
const ru_reg_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_0_REG = 
{
    "CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_0",
#if RU_INCLUDE_DESC
    "CPU_PD_INDIRECT_RD_DATA %i Register",
    "CPU PD Indirect Read data from DQM."
    "First entry represents PD[127:96] and so on until the last entry representing PD[31:0].",
#endif
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_0_REG_OFFSET,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_0_REG_RAM_CNT,
    64,
    42,
};

/******************************************************************************
 * Register: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_1
 ******************************************************************************/
const ru_reg_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_1_REG = 
{
    "CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_1",
#if RU_INCLUDE_DESC
    "CPU_PD_INDIRECT_RD_DATA %i Register",
    "CPU PD Indirect Read data from DQM."
    "First entry represents PD[127:96] and so on until the last entry representing PD[31:0].",
#endif
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_1_REG_OFFSET,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_1_REG_RAM_CNT,
    64,
    43,
};

/******************************************************************************
 * Register: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_2
 ******************************************************************************/
const ru_reg_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_2_REG = 
{
    "CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_2",
#if RU_INCLUDE_DESC
    "CPU_PD_INDIRECT_RD_DATA %i Register",
    "CPU PD Indirect Read data from DQM."
    "First entry represents PD[127:96] and so on until the last entry representing PD[31:0].",
#endif
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_2_REG_OFFSET,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_2_REG_RAM_CNT,
    64,
    44,
};

/******************************************************************************
 * Register: QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_3
 ******************************************************************************/
const ru_reg_rec QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_3_REG = 
{
    "CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_3",
#if RU_INCLUDE_DESC
    "CPU_PD_INDIRECT_RD_DATA %i Register",
    "CPU PD Indirect Read data from DQM."
    "First entry represents PD[127:96] and so on until the last entry representing PD[31:0].",
#endif
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_3_REG_OFFSET,
    QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_3_REG_RAM_CNT,
    64,
    45,
};

/******************************************************************************
 * Register: QM_QUEUE_CONTEXT_CONTEXT
 ******************************************************************************/
const ru_reg_rec QM_QUEUE_CONTEXT_CONTEXT_REG = 
{
    "QUEUE_CONTEXT_CONTEXT",
#if RU_INCLUDE_DESC
    "QUEUE_CONTEXT %i Register",
    "This RAM holds all queue attributes."
    "Not all of the 32-bits in the address space are implemented."
    "WRED Profile            3:0"
    "Copy decision profile 6:4"
    "Copy to DDR         7"
    "DDR copy disable 8"
    "Aggregation Disable 9"
    "FPM User Group         11:10"
    "Exclusive Priority 12"
    "802.1AE                 13"
    "SCI                 14"
    "FEC Enable         15"
    "",
#endif
    QM_QUEUE_CONTEXT_CONTEXT_REG_OFFSET,
    QM_QUEUE_CONTEXT_CONTEXT_REG_RAM_CNT,
    4,
    46,
};

/******************************************************************************
 * Register: QM_WRED_PROFILE_COLOR_MIN_THR_0
 ******************************************************************************/
const ru_reg_rec QM_WRED_PROFILE_COLOR_MIN_THR_0_REG = 
{
    "WRED_PROFILE_COLOR_MIN_THR_0",
#if RU_INCLUDE_DESC
    "COLOR_MIN_THR %i Register",
    "WRED Color min thresholds",
#endif
    QM_WRED_PROFILE_COLOR_MIN_THR_0_REG_OFFSET,
    QM_WRED_PROFILE_COLOR_MIN_THR_0_REG_RAM_CNT,
    48,
    47,
};

/******************************************************************************
 * Register: QM_WRED_PROFILE_COLOR_MIN_THR_1
 ******************************************************************************/
const ru_reg_rec QM_WRED_PROFILE_COLOR_MIN_THR_1_REG = 
{
    "WRED_PROFILE_COLOR_MIN_THR_1",
#if RU_INCLUDE_DESC
    "COLOR_MIN_THR %i Register",
    "WRED Color min thresholds",
#endif
    QM_WRED_PROFILE_COLOR_MIN_THR_1_REG_OFFSET,
    QM_WRED_PROFILE_COLOR_MIN_THR_1_REG_RAM_CNT,
    48,
    48,
};

/******************************************************************************
 * Register: QM_WRED_PROFILE_COLOR_MAX_THR_0
 ******************************************************************************/
const ru_reg_rec QM_WRED_PROFILE_COLOR_MAX_THR_0_REG = 
{
    "WRED_PROFILE_COLOR_MAX_THR_0",
#if RU_INCLUDE_DESC
    "COLOR_MAX_THR %i Register",
    "WRED Color max thresholds",
#endif
    QM_WRED_PROFILE_COLOR_MAX_THR_0_REG_OFFSET,
    QM_WRED_PROFILE_COLOR_MAX_THR_0_REG_RAM_CNT,
    48,
    49,
};

/******************************************************************************
 * Register: QM_WRED_PROFILE_COLOR_MAX_THR_1
 ******************************************************************************/
const ru_reg_rec QM_WRED_PROFILE_COLOR_MAX_THR_1_REG = 
{
    "WRED_PROFILE_COLOR_MAX_THR_1",
#if RU_INCLUDE_DESC
    "COLOR_MAX_THR %i Register",
    "WRED Color max thresholds",
#endif
    QM_WRED_PROFILE_COLOR_MAX_THR_1_REG_OFFSET,
    QM_WRED_PROFILE_COLOR_MAX_THR_1_REG_RAM_CNT,
    48,
    50,
};

/******************************************************************************
 * Register: QM_WRED_PROFILE_COLOR_SLOPE_0
 ******************************************************************************/
const ru_reg_rec QM_WRED_PROFILE_COLOR_SLOPE_0_REG = 
{
    "WRED_PROFILE_COLOR_SLOPE_0",
#if RU_INCLUDE_DESC
    "COLOR_SLOPE %i Register",
    "WRED Color slopes",
#endif
    QM_WRED_PROFILE_COLOR_SLOPE_0_REG_OFFSET,
    QM_WRED_PROFILE_COLOR_SLOPE_0_REG_RAM_CNT,
    48,
    51,
};

/******************************************************************************
 * Register: QM_WRED_PROFILE_COLOR_SLOPE_1
 ******************************************************************************/
const ru_reg_rec QM_WRED_PROFILE_COLOR_SLOPE_1_REG = 
{
    "WRED_PROFILE_COLOR_SLOPE_1",
#if RU_INCLUDE_DESC
    "COLOR_SLOPE %i Register",
    "WRED Color slopes",
#endif
    QM_WRED_PROFILE_COLOR_SLOPE_1_REG_OFFSET,
    QM_WRED_PROFILE_COLOR_SLOPE_1_REG_RAM_CNT,
    48,
    52,
};

/******************************************************************************
 * Register: QM_COPY_DECISION_PROFILE_THR
 ******************************************************************************/
const ru_reg_rec QM_COPY_DECISION_PROFILE_THR_REG = 
{
    "COPY_DECISION_PROFILE_THR",
#if RU_INCLUDE_DESC
    "THR Register",
    "DDR Pipe and PSRAM threshold configurations for DDR copy decision logic",
#endif
    QM_COPY_DECISION_PROFILE_THR_REG_OFFSET,
    QM_COPY_DECISION_PROFILE_THR_REG_RAM_CNT,
    32,
    53,
};

/******************************************************************************
 * Register: QM_TOTAL_VALID_COUNTER_COUNTER
 ******************************************************************************/
const ru_reg_rec QM_TOTAL_VALID_COUNTER_COUNTER_REG = 
{
    "TOTAL_VALID_COUNTER_COUNTER",
#if RU_INCLUDE_DESC
    "COUNTER %i Register",
    "Counter."
    "word0:{15`b0,pkt_cnt[16:0]}"
    "word1:{2`b0,byte_cnt[29:0]}"
    "word2:{15b0,res_cnt[16:0]}"
    "word3: reserved"
    ""
    "There are three words per queue starting at queue0 up to queue 159/287.",
#endif
    QM_TOTAL_VALID_COUNTER_COUNTER_REG_OFFSET,
    QM_TOTAL_VALID_COUNTER_COUNTER_REG_RAM_CNT,
    4,
    54,
};

/******************************************************************************
 * Register: QM_DQM_VALID_COUNTER_COUNTER
 ******************************************************************************/
const ru_reg_rec QM_DQM_VALID_COUNTER_COUNTER_REG = 
{
    "DQM_VALID_COUNTER_COUNTER",
#if RU_INCLUDE_DESC
    "COUNTER %i Register",
    "Counter."
    "word0:{15`b0,pkt_cnt[16:0]}"
    "word1:{2`b0,byte_cnt[29:0]}"
    ""
    "There are two words per queue starting at queue0 up to queue 287.",
#endif
    QM_DQM_VALID_COUNTER_COUNTER_REG_OFFSET,
    QM_DQM_VALID_COUNTER_COUNTER_REG_RAM_CNT,
    4,
    55,
};

/******************************************************************************
 * Register: QM_DROP_COUNTER_COUNTER
 ******************************************************************************/
const ru_reg_rec QM_DROP_COUNTER_COUNTER_REG = 
{
    "DROP_COUNTER_COUNTER",
#if RU_INCLUDE_DESC
    "COUNTER %i Register",
    "Counter."
    "word0:{6`b0,pkt_cnt[25:0]}"
    "word1:{byte_cnt[31:0]}"
    ""
    "in WRED drop mode:"
    "word0 - color1 dropped packets"
    "word1 - color0 dropped packets"
    ""
    ""
    "There are two words per queue starting at queue0 up to queue 287.",
#endif
    QM_DROP_COUNTER_COUNTER_REG_OFFSET,
    QM_DROP_COUNTER_COUNTER_REG_RAM_CNT,
    4,
    56,
};

/******************************************************************************
 * Register: QM_EPON_RPT_CNT_COUNTER
 ******************************************************************************/
const ru_reg_rec QM_EPON_RPT_CNT_COUNTER_REG = 
{
    "EPON_RPT_CNT_COUNTER",
#if RU_INCLUDE_DESC
    "COUNTER %i Register",
    "Counter - For each of the 32-queues in a batch, this counter stores a 32-bit accumulated and overhead byte counter per queue."
    "word0: {accumulated_bytes[31:0]}"
    "word1: {accumulated_overhead[31:0}"
    ""
    "There are two words per queue starting at queue0 up to queue 127.",
#endif
    QM_EPON_RPT_CNT_COUNTER_REG_OFFSET,
    QM_EPON_RPT_CNT_COUNTER_REG_RAM_CNT,
    4,
    57,
};

/******************************************************************************
 * Register: QM_EPON_RPT_CNT_QUEUE_STATUS
 ******************************************************************************/
const ru_reg_rec QM_EPON_RPT_CNT_QUEUE_STATUS_REG = 
{
    "EPON_RPT_CNT_QUEUE_STATUS",
#if RU_INCLUDE_DESC
    "QUEUE_STATUS %i Register",
    "Status bit vector - For each of the 32-queues in a batch, this status indicates which queue counter has been updated.",
#endif
    QM_EPON_RPT_CNT_QUEUE_STATUS_REG_OFFSET,
    QM_EPON_RPT_CNT_QUEUE_STATUS_REG_RAM_CNT,
    4,
    58,
};

/******************************************************************************
 * Register: QM_RD_DATA_POOL0
 ******************************************************************************/
const ru_reg_rec QM_RD_DATA_POOL0_REG = 
{
    "RD_DATA_POOL0",
#if RU_INCLUDE_DESC
    "RD_DATA_POOL0 Register",
    "Read the head of the FIFO",
#endif
    QM_RD_DATA_POOL0_REG_OFFSET,
    0,
    0,
    59,
};

/******************************************************************************
 * Register: QM_RD_DATA_POOL1
 ******************************************************************************/
const ru_reg_rec QM_RD_DATA_POOL1_REG = 
{
    "RD_DATA_POOL1",
#if RU_INCLUDE_DESC
    "RD_DATA_POOL1 Register",
    "Read the head of the FIFO",
#endif
    QM_RD_DATA_POOL1_REG_OFFSET,
    0,
    0,
    60,
};

/******************************************************************************
 * Register: QM_RD_DATA_POOL2
 ******************************************************************************/
const ru_reg_rec QM_RD_DATA_POOL2_REG = 
{
    "RD_DATA_POOL2",
#if RU_INCLUDE_DESC
    "RD_DATA_POOL2 Register",
    "Read the head of the FIFO",
#endif
    QM_RD_DATA_POOL2_REG_OFFSET,
    0,
    0,
    61,
};

/******************************************************************************
 * Register: QM_RD_DATA_POOL3
 ******************************************************************************/
const ru_reg_rec QM_RD_DATA_POOL3_REG = 
{
    "RD_DATA_POOL3",
#if RU_INCLUDE_DESC
    "RD_DATA_POOL3 Register",
    "Read the head of the FIFO",
#endif
    QM_RD_DATA_POOL3_REG_OFFSET,
    0,
    0,
    62,
};

/******************************************************************************
 * Register: QM_PDFIFO_PTR
 ******************************************************************************/
const ru_reg_rec QM_PDFIFO_PTR_REG = 
{
    "PDFIFO_PTR",
#if RU_INCLUDE_DESC
    "PDFIFO_PTR %i Register",
    "PDFIFO per queue rd/wr pointers",
#endif
    QM_PDFIFO_PTR_REG_OFFSET,
    QM_PDFIFO_PTR_REG_RAM_CNT,
    4,
    63,
};

/******************************************************************************
 * Register: QM_UPDATE_FIFO_PTR
 ******************************************************************************/
const ru_reg_rec QM_UPDATE_FIFO_PTR_REG = 
{
    "UPDATE_FIFO_PTR",
#if RU_INCLUDE_DESC
    "UPDATE_FIFO_PTR %i Register",
    "Update FIFO rd/wr pointers",
#endif
    QM_UPDATE_FIFO_PTR_REG_OFFSET,
    QM_UPDATE_FIFO_PTR_REG_RAM_CNT,
    4,
    64,
};

/******************************************************************************
 * Register: QM_RD_DATA
 ******************************************************************************/
const ru_reg_rec QM_RD_DATA_REG = 
{
    "RD_DATA",
#if RU_INCLUDE_DESC
    "RD_DATA %i Register",
    "Debug - Read the head of the FIFO",
#endif
    QM_RD_DATA_REG_OFFSET,
    QM_RD_DATA_REG_RAM_CNT,
    4,
    65,
};

/******************************************************************************
 * Register: QM_POP
 ******************************************************************************/
const ru_reg_rec QM_POP_REG = 
{
    "POP",
#if RU_INCLUDE_DESC
    "POP Register",
    "Pop an entry in the FIFO",
#endif
    QM_POP_REG_OFFSET,
    0,
    0,
    66,
};

/******************************************************************************
 * Register: QM_CM_COMMON_INPUT_FIFO_DATA
 ******************************************************************************/
const ru_reg_rec QM_CM_COMMON_INPUT_FIFO_DATA_REG = 
{
    "CM_COMMON_INPUT_FIFO_DATA",
#if RU_INCLUDE_DESC
    "DATA %i Register",
    "CM Common Input FIFO - debug access",
#endif
    QM_CM_COMMON_INPUT_FIFO_DATA_REG_OFFSET,
    QM_CM_COMMON_INPUT_FIFO_DATA_REG_RAM_CNT,
    4,
    67,
};

/******************************************************************************
 * Register: QM_NORMAL_RMT_FIFO_DATA
 ******************************************************************************/
const ru_reg_rec QM_NORMAL_RMT_FIFO_DATA_REG = 
{
    "NORMAL_RMT_FIFO_DATA",
#if RU_INCLUDE_DESC
    "DATA %i Register",
    "Normal Remote FIFO - debug access",
#endif
    QM_NORMAL_RMT_FIFO_DATA_REG_OFFSET,
    QM_NORMAL_RMT_FIFO_DATA_REG_RAM_CNT,
    4,
    68,
};

/******************************************************************************
 * Register: QM_NON_DELAYED_RMT_FIFO_DATA
 ******************************************************************************/
const ru_reg_rec QM_NON_DELAYED_RMT_FIFO_DATA_REG = 
{
    "NON_DELAYED_RMT_FIFO_DATA",
#if RU_INCLUDE_DESC
    "DATA %i Register",
    "Non-delayed Remote FIFO - debug access",
#endif
    QM_NON_DELAYED_RMT_FIFO_DATA_REG_OFFSET,
    QM_NON_DELAYED_RMT_FIFO_DATA_REG_RAM_CNT,
    4,
    69,
};

/******************************************************************************
 * Register: QM_EGRESS_DATA_FIFO_DATA
 ******************************************************************************/
const ru_reg_rec QM_EGRESS_DATA_FIFO_DATA_REG = 
{
    "EGRESS_DATA_FIFO_DATA",
#if RU_INCLUDE_DESC
    "DATA %i Register",
    "Egress data FIFO - debug access",
#endif
    QM_EGRESS_DATA_FIFO_DATA_REG_OFFSET,
    QM_EGRESS_DATA_FIFO_DATA_REG_RAM_CNT,
    4,
    70,
};

/******************************************************************************
 * Register: QM_EGRESS_RR_FIFO_DATA
 ******************************************************************************/
const ru_reg_rec QM_EGRESS_RR_FIFO_DATA_REG = 
{
    "EGRESS_RR_FIFO_DATA",
#if RU_INCLUDE_DESC
    "DATA %i Register",
    "Egress RR FIFO - debug access",
#endif
    QM_EGRESS_RR_FIFO_DATA_REG_OFFSET,
    QM_EGRESS_RR_FIFO_DATA_REG_RAM_CNT,
    4,
    71,
};

/******************************************************************************
 * Register: QM_EGRESS_BB_INPUT_FIFO_DATA
 ******************************************************************************/
const ru_reg_rec QM_EGRESS_BB_INPUT_FIFO_DATA_REG = 
{
    "EGRESS_BB_INPUT_FIFO_DATA",
#if RU_INCLUDE_DESC
    "DATA %i Register",
    "Egress BB Input FIFO - debug access",
#endif
    QM_EGRESS_BB_INPUT_FIFO_DATA_REG_OFFSET,
    QM_EGRESS_BB_INPUT_FIFO_DATA_REG_RAM_CNT,
    4,
    72,
};

/******************************************************************************
 * Register: QM_EGRESS_BB_OUTPUT_FIFO_DATA
 ******************************************************************************/
const ru_reg_rec QM_EGRESS_BB_OUTPUT_FIFO_DATA_REG = 
{
    "EGRESS_BB_OUTPUT_FIFO_DATA",
#if RU_INCLUDE_DESC
    "DATA %i Register",
    "Egress BB Output FIFO - debug access",
#endif
    QM_EGRESS_BB_OUTPUT_FIFO_DATA_REG_OFFSET,
    QM_EGRESS_BB_OUTPUT_FIFO_DATA_REG_RAM_CNT,
    4,
    73,
};

/******************************************************************************
 * Register: QM_BB_OUTPUT_FIFO_DATA
 ******************************************************************************/
const ru_reg_rec QM_BB_OUTPUT_FIFO_DATA_REG = 
{
    "BB_OUTPUT_FIFO_DATA",
#if RU_INCLUDE_DESC
    "DATA %i Register",
    "QM BB Output FIFO - debug access",
#endif
    QM_BB_OUTPUT_FIFO_DATA_REG_OFFSET,
    QM_BB_OUTPUT_FIFO_DATA_REG_RAM_CNT,
    4,
    74,
};

/******************************************************************************
 * Register: QM_NON_DELAYED_OUT_FIFO_DATA
 ******************************************************************************/
const ru_reg_rec QM_NON_DELAYED_OUT_FIFO_DATA_REG = 
{
    "NON_DELAYED_OUT_FIFO_DATA",
#if RU_INCLUDE_DESC
    "DATA %i Register",
    "Non delayed output FIFO - debug access",
#endif
    QM_NON_DELAYED_OUT_FIFO_DATA_REG_OFFSET,
    QM_NON_DELAYED_OUT_FIFO_DATA_REG_RAM_CNT,
    4,
    75,
};

/******************************************************************************
 * Register: QM_CONTEXT_DATA
 ******************************************************************************/
const ru_reg_rec QM_CONTEXT_DATA_REG = 
{
    "CONTEXT_DATA",
#if RU_INCLUDE_DESC
    "DATA %i Register",
    "Aggregation context - debug access",
#endif
    QM_CONTEXT_DATA_REG_OFFSET,
    QM_CONTEXT_DATA_REG_RAM_CNT,
    4,
    76,
};

/******************************************************************************
 * Register: QM_FPM_BUFFER_RESERVATION_DATA
 ******************************************************************************/
const ru_reg_rec QM_FPM_BUFFER_RESERVATION_DATA_REG = 
{
    "FPM_BUFFER_RESERVATION_DATA",
#if RU_INCLUDE_DESC
    "PROFILE %i Register",
    "Reserved FPM buffers in units of min. FPM buffer."
    "entry0 -> profile0"
    "..."
    "entry7 -> profile7",
#endif
    QM_FPM_BUFFER_RESERVATION_DATA_REG_OFFSET,
    QM_FPM_BUFFER_RESERVATION_DATA_REG_RAM_CNT,
    4,
    77,
};

/******************************************************************************
 * Register: QM_FLOW_CTRL_UG_CTRL
 ******************************************************************************/
const ru_reg_rec QM_FLOW_CTRL_UG_CTRL_REG = 
{
    "FLOW_CTRL_UG_CTRL",
#if RU_INCLUDE_DESC
    "UG_CTRL Register",
    "FPM user group ctrl",
#endif
    QM_FLOW_CTRL_UG_CTRL_REG_OFFSET,
    0,
    0,
    78,
};

/******************************************************************************
 * Register: QM_FLOW_CTRL_STATUS
 ******************************************************************************/
const ru_reg_rec QM_FLOW_CTRL_STATUS_REG = 
{
    "FLOW_CTRL_STATUS",
#if RU_INCLUDE_DESC
    "STATUS Register",
    "Keeps status vector of user group + wred are under flow control."
    "3:0 - {ug3,ug2,ug1,ug0}, 4 - OR on wred_source"
    ""
    "for UG -"
    "queues which passed the mid. thr. is set to 1. the user groups indication is de-asserted when the occupancy reaches the low thr."
    "4bits - bit for each user group."
    ""
    "for WRED/occupancy -"
    "If one of the queues which pass color1 min occupancy marked as 1. when the occupancy is reduced to below than color0 min occupancy in all queues the bit is set to 0."
    "FW can set/reset the value, it will updated when flow control which is relevant to the corresponding bit takes place.",
#endif
    QM_FLOW_CTRL_STATUS_REG_OFFSET,
    0,
    0,
    79,
};

/******************************************************************************
 * Register: QM_FLOW_CTRL_WRED_SOURCE
 ******************************************************************************/
const ru_reg_rec QM_FLOW_CTRL_WRED_SOURCE_REG = 
{
    "FLOW_CTRL_WRED_SOURCE",
#if RU_INCLUDE_DESC
    "WRED_SOURCE %i Register",
    "Keeps status vector of queues which are under flow control:"
    "queues which passed the color1 low threshold is set to 1. the queue indication is de-asserted when the queue byte occupancy reaches the color0 low threshold."
    "320bits - bit for each queue number (up to 320 queues).",
#endif
    QM_FLOW_CTRL_WRED_SOURCE_REG_OFFSET,
    QM_FLOW_CTRL_WRED_SOURCE_REG_RAM_CNT,
    4,
    80,
};

/******************************************************************************
 * Register: QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG
 ******************************************************************************/
const ru_reg_rec QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_REG = 
{
    "FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG",
#if RU_INCLUDE_DESC
    "QM_FLOW_CTRL_RNR_CFG Register",
    "lossless flow control configuration"
    ""
    "",
#endif
    QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_REG_OFFSET,
    0,
    0,
    81,
};

/******************************************************************************
 * Register: QM_DEBUG_SEL
 ******************************************************************************/
const ru_reg_rec QM_DEBUG_SEL_REG = 
{
    "DEBUG_SEL",
#if RU_INCLUDE_DESC
    "DEBUG_SEL Register",
    "Controls Debug bus select:"
    ""
    "5h1:  qm_dbg_bus = qm_bb_input_dbg_bus;"
    "5h2:  qm_dbg_bus = qm_bb_output_dbg_bus;"
    "5h3:  qm_dbg_bus = qm_cm_dbg_bus;"
    "5h4:  qm_dbg_bus = qm_ddr_write_dbg_bus;"
    "5h5:  qm_dbg_bus = qm_counters_dbg_bus;"
    "5h6:  qm_dbg_bus = qm_cpu_if_dbg_bus;"
    "5h7:  qm_dbg_bus = qm_dqm_push_dbg_bus;"
    "5h8:  qm_dbg_bus = qm_egress_dbg_bus;"
    "5h9:  qm_dbg_bus = qm_fpm_prefetch_dbg_bus;"
    "5ha:  qm_dbg_bus = qm_ingress_dbg_bus;"
    "5hb:  qm_dbg_bus = qm_rmt_fifos_dbg_bus;"
    "5hc:  qm_dbg_bus = {19b0,bbh_debug_0};"
    "5hd:  qm_dbg_bus = {19b0,bbh_debug_1};"
    "5he:  qm_dbg_bus = {19b0,bbh_debug_2};"
    "5hf:  qm_dbg_bus = {19b0,bbh_debug_3};"
    "5h10: qm_dbg_bus = {19b0,bbh_debug_4};"
    "5h11: qm_dbg_bus = {19b0,bbh_debug_5};"
    "5h12: qm_dbg_bus = {19b0,bbh_debug_6};"
    "5h13: qm_dbg_bus = {19b0,bbh_debug_7};"
    "5h14: qm_dbg_bus = {19b0,bbh_debug_8};"
    "5h15: qm_dbg_bus = {19b0,bbh_debug_9};"
    "5h16: qm_dbg_bus = {19b0,dma_debug_vec};"
    "5h17: qm_dbg_bus = {8b0,dqm_diag_r};"
    "",
#endif
    QM_DEBUG_SEL_REG_OFFSET,
    0,
    0,
    82,
};

/******************************************************************************
 * Register: QM_DEBUG_BUS_LSB
 ******************************************************************************/
const ru_reg_rec QM_DEBUG_BUS_LSB_REG = 
{
    "DEBUG_BUS_LSB",
#if RU_INCLUDE_DESC
    "DEBUG_BUS_LSB Register",
    "Debug Bus sampling",
#endif
    QM_DEBUG_BUS_LSB_REG_OFFSET,
    0,
    0,
    83,
};

/******************************************************************************
 * Register: QM_DEBUG_BUS_MSB
 ******************************************************************************/
const ru_reg_rec QM_DEBUG_BUS_MSB_REG = 
{
    "DEBUG_BUS_MSB",
#if RU_INCLUDE_DESC
    "DEBUG_BUS_MSB Register",
    "Debug Bus sampling",
#endif
    QM_DEBUG_BUS_MSB_REG_OFFSET,
    0,
    0,
    84,
};

/******************************************************************************
 * Register: QM_QM_SPARE_CONFIG
 ******************************************************************************/
const ru_reg_rec QM_QM_SPARE_CONFIG_REG = 
{
    "QM_SPARE_CONFIG",
#if RU_INCLUDE_DESC
    "QM_SPARE_CONFIG Register",
    "Spare configuration for ECO purposes",
#endif
    QM_QM_SPARE_CONFIG_REG_OFFSET,
    0,
    0,
    85,
};

/******************************************************************************
 * Register: QM_GOOD_LVL1_PKTS_CNT
 ******************************************************************************/
const ru_reg_rec QM_GOOD_LVL1_PKTS_CNT_REG = 
{
    "GOOD_LVL1_PKTS_CNT",
#if RU_INCLUDE_DESC
    "GOOD_LVL1_PKTS_CNT Register",
    "Counts the total number of non-dropped and non-reprocessing packets from all queues",
#endif
    QM_GOOD_LVL1_PKTS_CNT_REG_OFFSET,
    0,
    0,
    86,
};

/******************************************************************************
 * Register: QM_GOOD_LVL1_BYTES_CNT
 ******************************************************************************/
const ru_reg_rec QM_GOOD_LVL1_BYTES_CNT_REG = 
{
    "GOOD_LVL1_BYTES_CNT",
#if RU_INCLUDE_DESC
    "GOOD_LVL1_BYTES_CNT Register",
    "Counts the total number of non-dropped and non-reprocessing bytes from all queues",
#endif
    QM_GOOD_LVL1_BYTES_CNT_REG_OFFSET,
    0,
    0,
    87,
};

/******************************************************************************
 * Register: QM_GOOD_LVL2_PKTS_CNT
 ******************************************************************************/
const ru_reg_rec QM_GOOD_LVL2_PKTS_CNT_REG = 
{
    "GOOD_LVL2_PKTS_CNT",
#if RU_INCLUDE_DESC
    "GOOD_LVL2_PKTS_CNT Register",
    "Counts the total number of non-dropped and reprocessing packets from all queues",
#endif
    QM_GOOD_LVL2_PKTS_CNT_REG_OFFSET,
    0,
    0,
    88,
};

/******************************************************************************
 * Register: QM_GOOD_LVL2_BYTES_CNT
 ******************************************************************************/
const ru_reg_rec QM_GOOD_LVL2_BYTES_CNT_REG = 
{
    "GOOD_LVL2_BYTES_CNT",
#if RU_INCLUDE_DESC
    "GOOD_LVL2_BYTES_CNT Register",
    "Counts the total number of non-dropped and reprocessing bytes from all queues",
#endif
    QM_GOOD_LVL2_BYTES_CNT_REG_OFFSET,
    0,
    0,
    89,
};

/******************************************************************************
 * Register: QM_COPIED_PKTS_CNT
 ******************************************************************************/
const ru_reg_rec QM_COPIED_PKTS_CNT_REG = 
{
    "COPIED_PKTS_CNT",
#if RU_INCLUDE_DESC
    "COPIED_PKTS_CNT Register",
    "Counts the total number of copied packets to the DDR from all queues",
#endif
    QM_COPIED_PKTS_CNT_REG_OFFSET,
    0,
    0,
    90,
};

/******************************************************************************
 * Register: QM_COPIED_BYTES_CNT
 ******************************************************************************/
const ru_reg_rec QM_COPIED_BYTES_CNT_REG = 
{
    "COPIED_BYTES_CNT",
#if RU_INCLUDE_DESC
    "COPIED_BYTES_CNT Register",
    "Counts the total number of copied bytes to the DDR from all queues",
#endif
    QM_COPIED_BYTES_CNT_REG_OFFSET,
    0,
    0,
    91,
};

/******************************************************************************
 * Register: QM_AGG_PKTS_CNT
 ******************************************************************************/
const ru_reg_rec QM_AGG_PKTS_CNT_REG = 
{
    "AGG_PKTS_CNT",
#if RU_INCLUDE_DESC
    "AGG_PKTS_CNT Register",
    "Counts the total number of aggregated packets from all queues",
#endif
    QM_AGG_PKTS_CNT_REG_OFFSET,
    0,
    0,
    92,
};

/******************************************************************************
 * Register: QM_AGG_BYTES_CNT
 ******************************************************************************/
const ru_reg_rec QM_AGG_BYTES_CNT_REG = 
{
    "AGG_BYTES_CNT",
#if RU_INCLUDE_DESC
    "AGG_BYTES_CNT Register",
    "Counts the total number of aggregated bytes from all queues",
#endif
    QM_AGG_BYTES_CNT_REG_OFFSET,
    0,
    0,
    93,
};

/******************************************************************************
 * Register: QM_AGG_1_PKTS_CNT
 ******************************************************************************/
const ru_reg_rec QM_AGG_1_PKTS_CNT_REG = 
{
    "AGG_1_PKTS_CNT",
#if RU_INCLUDE_DESC
    "AGG_1_PKTS_CNT Register",
    "Counts the total number of packets aggregated in a 1-packet PD from all queues",
#endif
    QM_AGG_1_PKTS_CNT_REG_OFFSET,
    0,
    0,
    94,
};

/******************************************************************************
 * Register: QM_AGG_2_PKTS_CNT
 ******************************************************************************/
const ru_reg_rec QM_AGG_2_PKTS_CNT_REG = 
{
    "AGG_2_PKTS_CNT",
#if RU_INCLUDE_DESC
    "AGG_2_PKTS_CNT Register",
    "Counts the total number of packets aggregated in a 2-packet PD from all queues",
#endif
    QM_AGG_2_PKTS_CNT_REG_OFFSET,
    0,
    0,
    95,
};

/******************************************************************************
 * Register: QM_AGG_3_PKTS_CNT
 ******************************************************************************/
const ru_reg_rec QM_AGG_3_PKTS_CNT_REG = 
{
    "AGG_3_PKTS_CNT",
#if RU_INCLUDE_DESC
    "AGG_3_PKTS_CNT Register",
    "Counts the total number of packets aggregated in a 3-packet PD from all queues",
#endif
    QM_AGG_3_PKTS_CNT_REG_OFFSET,
    0,
    0,
    96,
};

/******************************************************************************
 * Register: QM_AGG_4_PKTS_CNT
 ******************************************************************************/
const ru_reg_rec QM_AGG_4_PKTS_CNT_REG = 
{
    "AGG_4_PKTS_CNT",
#if RU_INCLUDE_DESC
    "AGG_4_PKTS_CNT Register",
    "Counts the total number of packets aggregated in a 4-packet PD from all queues",
#endif
    QM_AGG_4_PKTS_CNT_REG_OFFSET,
    0,
    0,
    97,
};

/******************************************************************************
 * Register: QM_WRED_DROP_CNT
 ******************************************************************************/
const ru_reg_rec QM_WRED_DROP_CNT_REG = 
{
    "WRED_DROP_CNT",
#if RU_INCLUDE_DESC
    "WRED_DROP_CNT Register",
    "Counts the total number of packets dropped from all queues due to WRED",
#endif
    QM_WRED_DROP_CNT_REG_OFFSET,
    0,
    0,
    98,
};

/******************************************************************************
 * Register: QM_FPM_CONGESTION_DROP_CNT
 ******************************************************************************/
const ru_reg_rec QM_FPM_CONGESTION_DROP_CNT_REG = 
{
    "FPM_CONGESTION_DROP_CNT",
#if RU_INCLUDE_DESC
    "FPM_CONGESTION_DROP_CNT Register",
    "Counts the total number of packets dropped from all queues due to FPM congestion indication",
#endif
    QM_FPM_CONGESTION_DROP_CNT_REG_OFFSET,
    0,
    0,
    99,
};

/******************************************************************************
 * Register: QM_DDR_PD_CONGESTION_DROP_CNT
 ******************************************************************************/
const ru_reg_rec QM_DDR_PD_CONGESTION_DROP_CNT_REG = 
{
    "DDR_PD_CONGESTION_DROP_CNT",
#if RU_INCLUDE_DESC
    "DDR_PD_CONGESTION_DROP_CNT Register",
    "Counts the total number of packets dropped from all queues due to DDR PD congestion",
#endif
    QM_DDR_PD_CONGESTION_DROP_CNT_REG_OFFSET,
    0,
    0,
    100,
};

/******************************************************************************
 * Register: QM_DDR_BYTE_CONGESTION_DROP_CNT
 ******************************************************************************/
const ru_reg_rec QM_DDR_BYTE_CONGESTION_DROP_CNT_REG = 
{
    "DDR_BYTE_CONGESTION_DROP_CNT",
#if RU_INCLUDE_DESC
    "DDR_BYTE_CONGESTION_DROP_CNT Register",
    "Counts the total number of packets dropped from all queues due to DDR byte congestion (number of bytes waiting to be copied exceeded the thresholds)",
#endif
    QM_DDR_BYTE_CONGESTION_DROP_CNT_REG_OFFSET,
    0,
    0,
    101,
};

/******************************************************************************
 * Register: QM_QM_PD_CONGESTION_DROP_CNT
 ******************************************************************************/
const ru_reg_rec QM_QM_PD_CONGESTION_DROP_CNT_REG = 
{
    "QM_PD_CONGESTION_DROP_CNT",
#if RU_INCLUDE_DESC
    "QM_PD_CONGESTION_DROP_CNT Register",
    "Counts the total number of packets dropped from all queues due to QM PD congestion (this value is limited by the DQM)",
#endif
    QM_QM_PD_CONGESTION_DROP_CNT_REG_OFFSET,
    0,
    0,
    102,
};

/******************************************************************************
 * Register: QM_QM_ABS_REQUEUE_CNT
 ******************************************************************************/
const ru_reg_rec QM_QM_ABS_REQUEUE_CNT_REG = 
{
    "QM_ABS_REQUEUE_CNT",
#if RU_INCLUDE_DESC
    "QM_ABS_REQUEUE_CNT Register",
    "Counts the total number of packets requeued due to absolute address drops from all queues",
#endif
    QM_QM_ABS_REQUEUE_CNT_REG_OFFSET,
    0,
    0,
    103,
};

/******************************************************************************
 * Register: QM_FPM_PREFETCH_FIFO0_STATUS
 ******************************************************************************/
const ru_reg_rec QM_FPM_PREFETCH_FIFO0_STATUS_REG = 
{
    "FPM_PREFETCH_FIFO0_STATUS",
#if RU_INCLUDE_DESC
    "FPM_PREFETCH_FIFO0_STATUS Register",
    "Holds the FIFO Status",
#endif
    QM_FPM_PREFETCH_FIFO0_STATUS_REG_OFFSET,
    0,
    0,
    104,
};

/******************************************************************************
 * Register: QM_FPM_PREFETCH_FIFO1_STATUS
 ******************************************************************************/
const ru_reg_rec QM_FPM_PREFETCH_FIFO1_STATUS_REG = 
{
    "FPM_PREFETCH_FIFO1_STATUS",
#if RU_INCLUDE_DESC
    "FPM_PREFETCH_FIFO1_STATUS Register",
    "Holds the FIFO Status",
#endif
    QM_FPM_PREFETCH_FIFO1_STATUS_REG_OFFSET,
    0,
    0,
    105,
};

/******************************************************************************
 * Register: QM_FPM_PREFETCH_FIFO2_STATUS
 ******************************************************************************/
const ru_reg_rec QM_FPM_PREFETCH_FIFO2_STATUS_REG = 
{
    "FPM_PREFETCH_FIFO2_STATUS",
#if RU_INCLUDE_DESC
    "FPM_PREFETCH_FIFO2_STATUS Register",
    "Holds the FIFO Status",
#endif
    QM_FPM_PREFETCH_FIFO2_STATUS_REG_OFFSET,
    0,
    0,
    106,
};

/******************************************************************************
 * Register: QM_FPM_PREFETCH_FIFO3_STATUS
 ******************************************************************************/
const ru_reg_rec QM_FPM_PREFETCH_FIFO3_STATUS_REG = 
{
    "FPM_PREFETCH_FIFO3_STATUS",
#if RU_INCLUDE_DESC
    "FPM_PREFETCH_FIFO3_STATUS Register",
    "Holds the FIFO Status",
#endif
    QM_FPM_PREFETCH_FIFO3_STATUS_REG_OFFSET,
    0,
    0,
    107,
};

/******************************************************************************
 * Register: QM_NORMAL_RMT_FIFO_STATUS
 ******************************************************************************/
const ru_reg_rec QM_NORMAL_RMT_FIFO_STATUS_REG = 
{
    "NORMAL_RMT_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "NORMAL_RMT_FIFO_STATUS Register",
    "Holds the FIFO Status",
#endif
    QM_NORMAL_RMT_FIFO_STATUS_REG_OFFSET,
    0,
    0,
    108,
};

/******************************************************************************
 * Register: QM_NON_DELAYED_RMT_FIFO_STATUS
 ******************************************************************************/
const ru_reg_rec QM_NON_DELAYED_RMT_FIFO_STATUS_REG = 
{
    "NON_DELAYED_RMT_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "NON_DELAYED_RMT_FIFO_STATUS Register",
    "Holds the FIFO Status",
#endif
    QM_NON_DELAYED_RMT_FIFO_STATUS_REG_OFFSET,
    0,
    0,
    109,
};

/******************************************************************************
 * Register: QM_NON_DELAYED_OUT_FIFO_STATUS
 ******************************************************************************/
const ru_reg_rec QM_NON_DELAYED_OUT_FIFO_STATUS_REG = 
{
    "NON_DELAYED_OUT_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "NON_DELAYED_OUT_FIFO_STATUS Register",
    "Holds the FIFO Status",
#endif
    QM_NON_DELAYED_OUT_FIFO_STATUS_REG_OFFSET,
    0,
    0,
    110,
};

/******************************************************************************
 * Register: QM_PRE_CM_FIFO_STATUS
 ******************************************************************************/
const ru_reg_rec QM_PRE_CM_FIFO_STATUS_REG = 
{
    "PRE_CM_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "PRE_CM_FIFO_STATUS Register",
    "Holds the FIFO Status",
#endif
    QM_PRE_CM_FIFO_STATUS_REG_OFFSET,
    0,
    0,
    111,
};

/******************************************************************************
 * Register: QM_CM_RD_PD_FIFO_STATUS
 ******************************************************************************/
const ru_reg_rec QM_CM_RD_PD_FIFO_STATUS_REG = 
{
    "CM_RD_PD_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "CM_RD_PD_FIFO_STATUS Register",
    "Holds the FIFO Status",
#endif
    QM_CM_RD_PD_FIFO_STATUS_REG_OFFSET,
    0,
    0,
    112,
};

/******************************************************************************
 * Register: QM_CM_WR_PD_FIFO_STATUS
 ******************************************************************************/
const ru_reg_rec QM_CM_WR_PD_FIFO_STATUS_REG = 
{
    "CM_WR_PD_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "CM_WR_PD_FIFO_STATUS Register",
    "Holds the FIFO Status",
#endif
    QM_CM_WR_PD_FIFO_STATUS_REG_OFFSET,
    0,
    0,
    113,
};

/******************************************************************************
 * Register: QM_CM_COMMON_INPUT_FIFO_STATUS
 ******************************************************************************/
const ru_reg_rec QM_CM_COMMON_INPUT_FIFO_STATUS_REG = 
{
    "CM_COMMON_INPUT_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "CM_COMMON_INPUT_FIFO_STATUS Register",
    "Holds the FIFO Status",
#endif
    QM_CM_COMMON_INPUT_FIFO_STATUS_REG_OFFSET,
    0,
    0,
    114,
};

/******************************************************************************
 * Register: QM_BB0_OUTPUT_FIFO_STATUS
 ******************************************************************************/
const ru_reg_rec QM_BB0_OUTPUT_FIFO_STATUS_REG = 
{
    "BB0_OUTPUT_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "BB0_OUTPUT_FIFO_STATUS Register",
    "Holds the FIFO Status",
#endif
    QM_BB0_OUTPUT_FIFO_STATUS_REG_OFFSET,
    0,
    0,
    115,
};

/******************************************************************************
 * Register: QM_BB1_OUTPUT_FIFO_STATUS
 ******************************************************************************/
const ru_reg_rec QM_BB1_OUTPUT_FIFO_STATUS_REG = 
{
    "BB1_OUTPUT_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "BB1_OUTPUT_FIFO_STATUS Register",
    "Holds the FIFO Status",
#endif
    QM_BB1_OUTPUT_FIFO_STATUS_REG_OFFSET,
    0,
    0,
    116,
};

/******************************************************************************
 * Register: QM_BB1_INPUT_FIFO_STATUS
 ******************************************************************************/
const ru_reg_rec QM_BB1_INPUT_FIFO_STATUS_REG = 
{
    "BB1_INPUT_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "BB1_INPUT_FIFO_STATUS Register",
    "Holds the FIFO Status",
#endif
    QM_BB1_INPUT_FIFO_STATUS_REG_OFFSET,
    0,
    0,
    117,
};

/******************************************************************************
 * Register: QM_EGRESS_DATA_FIFO_STATUS
 ******************************************************************************/
const ru_reg_rec QM_EGRESS_DATA_FIFO_STATUS_REG = 
{
    "EGRESS_DATA_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "EGRESS_DATA_FIFO_STATUS Register",
    "Holds the FIFO Status",
#endif
    QM_EGRESS_DATA_FIFO_STATUS_REG_OFFSET,
    0,
    0,
    118,
};

/******************************************************************************
 * Register: QM_EGRESS_RR_FIFO_STATUS
 ******************************************************************************/
const ru_reg_rec QM_EGRESS_RR_FIFO_STATUS_REG = 
{
    "EGRESS_RR_FIFO_STATUS",
#if RU_INCLUDE_DESC
    "EGRESS_RR_FIFO_STATUS Register",
    "Holds the FIFO Status",
#endif
    QM_EGRESS_RR_FIFO_STATUS_REG_OFFSET,
    0,
    0,
    119,
};

/******************************************************************************
 * Register: QM_BB_ROUTE_OVR
 ******************************************************************************/
const ru_reg_rec QM_BB_ROUTE_OVR_REG = 
{
    "BB_ROUTE_OVR",
#if RU_INCLUDE_DESC
    "BB_ROUTE_OVR %i Register",
    "BB ROUTE Override:"
    "0 - for QM_TOP"
    "1 - for RNR_GRID",
#endif
    QM_BB_ROUTE_OVR_REG_OFFSET,
    QM_BB_ROUTE_OVR_REG_RAM_CNT,
    4,
    120,
};

/******************************************************************************
 * Register: QM_QM_INGRESS_STAT
 ******************************************************************************/
const ru_reg_rec QM_QM_INGRESS_STAT_REG = 
{
    "QM_INGRESS_STAT",
#if RU_INCLUDE_DESC
    "QM_INGRESS_STAT Register",
    "Holds the Ingress Status",
#endif
    QM_QM_INGRESS_STAT_REG_OFFSET,
    0,
    0,
    121,
};

/******************************************************************************
 * Register: QM_QM_EGRESS_STAT
 ******************************************************************************/
const ru_reg_rec QM_QM_EGRESS_STAT_REG = 
{
    "QM_EGRESS_STAT",
#if RU_INCLUDE_DESC
    "QM_EGRESS_STAT Register",
    "Holds the Egress Status",
#endif
    QM_QM_EGRESS_STAT_REG_OFFSET,
    0,
    0,
    122,
};

/******************************************************************************
 * Register: QM_QM_CM_STAT
 ******************************************************************************/
const ru_reg_rec QM_QM_CM_STAT_REG = 
{
    "QM_CM_STAT",
#if RU_INCLUDE_DESC
    "QM_CM_STAT Register",
    "Holds the CM Status",
#endif
    QM_QM_CM_STAT_REG_OFFSET,
    0,
    0,
    123,
};

/******************************************************************************
 * Register: QM_QM_FPM_PREFETCH_STAT
 ******************************************************************************/
const ru_reg_rec QM_QM_FPM_PREFETCH_STAT_REG = 
{
    "QM_FPM_PREFETCH_STAT",
#if RU_INCLUDE_DESC
    "QM_FPM_PREFETCH_STAT Register",
    "Holds the FPM Prefetch Status",
#endif
    QM_QM_FPM_PREFETCH_STAT_REG_OFFSET,
    0,
    0,
    124,
};

/******************************************************************************
 * Register: QM_QM_CONNECT_ACK_COUNTER
 ******************************************************************************/
const ru_reg_rec QM_QM_CONNECT_ACK_COUNTER_REG = 
{
    "QM_CONNECT_ACK_COUNTER",
#if RU_INCLUDE_DESC
    "QM_CONNECT_ACK_COUNTER Register",
    "QM connect ack counter",
#endif
    QM_QM_CONNECT_ACK_COUNTER_REG_OFFSET,
    0,
    0,
    125,
};

/******************************************************************************
 * Register: QM_QM_DDR_WR_REPLY_COUNTER
 ******************************************************************************/
const ru_reg_rec QM_QM_DDR_WR_REPLY_COUNTER_REG = 
{
    "QM_DDR_WR_REPLY_COUNTER",
#if RU_INCLUDE_DESC
    "QM_DDR_WR_REPLY_COUNTER Register",
    "QM DDR WR reply Counter",
#endif
    QM_QM_DDR_WR_REPLY_COUNTER_REG_OFFSET,
    0,
    0,
    126,
};

/******************************************************************************
 * Register: QM_QM_DDR_PIPE_BYTE_COUNTER
 ******************************************************************************/
const ru_reg_rec QM_QM_DDR_PIPE_BYTE_COUNTER_REG = 
{
    "QM_DDR_PIPE_BYTE_COUNTER",
#if RU_INCLUDE_DESC
    "QM_DDR_PIPE_BYTE_COUNTER Register",
    "QM DDR pipe byte counter",
#endif
    QM_QM_DDR_PIPE_BYTE_COUNTER_REG_OFFSET,
    0,
    0,
    127,
};

/******************************************************************************
 * Register: QM_QM_ABS_REQUEUE_VALID_COUNTER
 ******************************************************************************/
const ru_reg_rec QM_QM_ABS_REQUEUE_VALID_COUNTER_REG = 
{
    "QM_ABS_REQUEUE_VALID_COUNTER",
#if RU_INCLUDE_DESC
    "QM_ABS_REQUEUE_VALID_COUNTER Register",
    "Indicates the number of PDs currently in the Absolute address drop queue.",
#endif
    QM_QM_ABS_REQUEUE_VALID_COUNTER_REG_OFFSET,
    0,
    0,
    128,
};

/******************************************************************************
 * Register: QM_QM_ILLEGAL_PD_CAPTURE
 ******************************************************************************/
const ru_reg_rec QM_QM_ILLEGAL_PD_CAPTURE_REG = 
{
    "QM_ILLEGAL_PD_CAPTURE",
#if RU_INCLUDE_DESC
    "QM_ILLEGAL_PD_CAPTURE %i Register",
    "PD captured when an illegal PD was detected and the relevant interrupt was generated.",
#endif
    QM_QM_ILLEGAL_PD_CAPTURE_REG_OFFSET,
    QM_QM_ILLEGAL_PD_CAPTURE_REG_RAM_CNT,
    4,
    129,
};

/******************************************************************************
 * Register: QM_QM_INGRESS_PROCESSED_PD_CAPTURE
 ******************************************************************************/
const ru_reg_rec QM_QM_INGRESS_PROCESSED_PD_CAPTURE_REG = 
{
    "QM_INGRESS_PROCESSED_PD_CAPTURE",
#if RU_INCLUDE_DESC
    "QM_INGRESS_PROCESSED_PD_CAPTURE %i Register",
    "Last ingress processed PD capture",
#endif
    QM_QM_INGRESS_PROCESSED_PD_CAPTURE_REG_OFFSET,
    QM_QM_INGRESS_PROCESSED_PD_CAPTURE_REG_RAM_CNT,
    4,
    130,
};

/******************************************************************************
 * Register: QM_QM_CM_PROCESSED_PD_CAPTURE
 ******************************************************************************/
const ru_reg_rec QM_QM_CM_PROCESSED_PD_CAPTURE_REG = 
{
    "QM_CM_PROCESSED_PD_CAPTURE",
#if RU_INCLUDE_DESC
    "QM_CM_PROCESSED_PD_CAPTURE %i Register",
    "Last copy machine processed PD capture",
#endif
    QM_QM_CM_PROCESSED_PD_CAPTURE_REG_OFFSET,
    QM_QM_CM_PROCESSED_PD_CAPTURE_REG_RAM_CNT,
    4,
    131,
};

/******************************************************************************
 * Register: QM_FPM_POOL_DROP_CNT
 ******************************************************************************/
const ru_reg_rec QM_FPM_POOL_DROP_CNT_REG = 
{
    "FPM_POOL_DROP_CNT",
#if RU_INCLUDE_DESC
    "FPM_POOL_DROP_CNT %i Register",
    "Counts the total number of packets dropped for all queues due to FPM pool priority thresholds. Counter per pool",
#endif
    QM_FPM_POOL_DROP_CNT_REG_OFFSET,
    QM_FPM_POOL_DROP_CNT_REG_RAM_CNT,
    4,
    132,
};

/******************************************************************************
 * Register: QM_FPM_GRP_DROP_CNT
 ******************************************************************************/
const ru_reg_rec QM_FPM_GRP_DROP_CNT_REG = 
{
    "FPM_GRP_DROP_CNT",
#if RU_INCLUDE_DESC
    "FPM_GRP_DROP_CNT %i Register",
    "Counts the total number of packets dropped from all queues due to FPM user group priority thresholds. Counter per UG (0-3)",
#endif
    QM_FPM_GRP_DROP_CNT_REG_OFFSET,
    QM_FPM_GRP_DROP_CNT_REG_RAM_CNT,
    4,
    133,
};

/******************************************************************************
 * Register: QM_FPM_BUFFER_RES_DROP_CNT
 ******************************************************************************/
const ru_reg_rec QM_FPM_BUFFER_RES_DROP_CNT_REG = 
{
    "FPM_BUFFER_RES_DROP_CNT",
#if RU_INCLUDE_DESC
    "FPM_BUFFER_RES_DROP_CNT Register",
    "Counts the total number of packets dropped from all queues due to buffer reservation mechanism.",
#endif
    QM_FPM_BUFFER_RES_DROP_CNT_REG_OFFSET,
    0,
    0,
    134,
};

/******************************************************************************
 * Register: QM_PSRAM_EGRESS_CONG_DRP_CNT
 ******************************************************************************/
const ru_reg_rec QM_PSRAM_EGRESS_CONG_DRP_CNT_REG = 
{
    "PSRAM_EGRESS_CONG_DRP_CNT",
#if RU_INCLUDE_DESC
    "PSRAM_EGRESS_CONG_DRP_CNT Register",
    "Counts the total number of packets dropped from all queues due to psram egress congestion.",
#endif
    QM_PSRAM_EGRESS_CONG_DRP_CNT_REG_OFFSET,
    0,
    0,
    135,
};

/******************************************************************************
 * Register: QM_DATA
 ******************************************************************************/
const ru_reg_rec QM_DATA_REG = 
{
    "DATA",
#if RU_INCLUDE_DESC
    "DATA %i Register",
    "CM Residue - debug access",
#endif
    QM_DATA_REG_OFFSET,
    QM_DATA_REG_RAM_CNT,
    4,
    136,
};

/******************************************************************************
 * Register: QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_VPB_BASE
 ******************************************************************************/
const ru_reg_rec QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_VPB_BASE_REG = 
{
    "XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_VPB_BASE",
#if RU_INCLUDE_DESC
    "VPB_BASE Register",
    "VPB Base address",
#endif
    QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_VPB_BASE_REG_OFFSET,
    0,
    0,
    137,
};

/******************************************************************************
 * Register: QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_VPB_MASK
 ******************************************************************************/
const ru_reg_rec QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_VPB_MASK_REG = 
{
    "XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_VPB_MASK",
#if RU_INCLUDE_DESC
    "VPB_MASK Register",
    "VPB mask address",
#endif
    QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_VPB_MASK_REG_OFFSET,
    0,
    0,
    138,
};

/******************************************************************************
 * Register: QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_APB_BASE
 ******************************************************************************/
const ru_reg_rec QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_APB_BASE_REG = 
{
    "XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_APB_BASE",
#if RU_INCLUDE_DESC
    "APB_BASE Register",
    "APB Base address",
#endif
    QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_APB_BASE_REG_OFFSET,
    0,
    0,
    139,
};

/******************************************************************************
 * Register: QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_APB_MASK
 ******************************************************************************/
const ru_reg_rec QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_APB_MASK_REG = 
{
    "XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_APB_MASK",
#if RU_INCLUDE_DESC
    "APB_MASK Register",
    "APB mask address",
#endif
    QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_APB_MASK_REG_OFFSET,
    0,
    0,
    140,
};

/******************************************************************************
 * Register: QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_DQM_BASE
 ******************************************************************************/
const ru_reg_rec QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_DQM_BASE_REG = 
{
    "XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_DQM_BASE",
#if RU_INCLUDE_DESC
    "DQM_BASE Register",
    "DQM Base address",
#endif
    QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_DQM_BASE_REG_OFFSET,
    0,
    0,
    141,
};

/******************************************************************************
 * Register: QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_DQM_MASK
 ******************************************************************************/
const ru_reg_rec QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_DQM_MASK_REG = 
{
    "XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_DQM_MASK",
#if RU_INCLUDE_DESC
    "DQM_MASK Register",
    "DQM mask address",
#endif
    QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_DQM_MASK_REG_OFFSET,
    0,
    0,
    142,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_MACTYPE
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_MACTYPE_REG = 
{
    "BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_MACTYPE",
#if RU_INCLUDE_DESC
    "MAC_TYPE Register",
    "The BBH supports working with different MAC types. Each MAC requires different interface and features. This register defines the type of MAC the BBH works with.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_MACTYPE_REG_OFFSET,
    0,
    0,
    143,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_REG = 
{
    "BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX",
#if RU_INCLUDE_DESC
    "BB_CFG_1 Register",
    "Each BBH unit has its own position on the BB tree. This position defines the Route address when approaching the Runner, S/DMA or S/BPM. The route is determined by a dedicated generic logic which uses the source id of the destination.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_REG_OFFSET,
    0,
    0,
    144,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_REG = 
{
    "BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX",
#if RU_INCLUDE_DESC
    "BB_CFG_2 Register",
    "Each BBH unit has its own position on the BB tree. This position defines the Route address when approaching the Runner, S/DMA or S/BPM. The route is determined by a dedicated generic logic which uses the source id of the destination.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_REG_OFFSET,
    0,
    0,
    145,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_REG = 
{
    "BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX",
#if RU_INCLUDE_DESC
    "RD_ADDR_CFG Register",
    "Configurations for determining the address to read from the DDR/PSRAm",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_REG_OFFSET,
    0,
    0,
    146,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_1
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_1_REG = 
{
    "BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_1",
#if RU_INCLUDE_DESC
    "PD_RNR_CFG_1 %i Register",
    "Queue index address:"
    "The BBH requests a Packet descriptor from the Runner. The BBH writes the queue number in a predefined address at the Runner SRAM. The message serves also as a wake-up request to the Runner."
    "This register defines the queue index address within the Runner address space."
    "SKB address:"
    "When the packet is transmitted from absolute address, then, instead of releasing the BN, the BBH writes a 6 bits read counter into the Runner SRAM. It writes it into a pre-defined address + TCONT_NUM (for Ethernet TCONT_NUM = 0)."
    "This register defines the SKB free base address within the Runner address."
    ""
    "Note: all addresses are in 8 byte resolution. As the Runner memory is limited to 12 bits address, use the 12 lsb bits."
    "",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_1_REG_OFFSET,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_1_REG_RAM_CNT,
    4,
    147,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_2
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_2_REG = 
{
    "BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_2",
#if RU_INCLUDE_DESC
    "PD_RNR_CFG_2 %i Register",
    "PD transfer process:"
    "-The Runner wont ACK the BBH; therefore the BBH wont wake the TX task."
    "-The Runner will push the PDs into the BBH (without any wakeup from the BBH)."
    "-Each time that the BBH reads a PD from the PD FIFO, it will write the read pointer into a pre-defined address in the Runner. The pointer is 6 bits width (one bit larger than needed to distinguish between full and empty)."
    "-The Runner should manage the congestion over the PD FIFO (in the BBH) by reading the BBH read pointer prior to each PD write."
    "-PD drop should be done by the Runner only. The BBH will drop PD when the FIFO is full and will count each drop. The BBH wont release the BN in this case."
    "-There will be a full threshold, which can be smaller than the actual size of the FIFO. When the BBH will move from full to not full state, the BBH will wakeup the Runner."
    ""
    "Note: all addresses are in 8 byte resolution. As the Runner memory is limited to 12 bits address, use the 12 lsb bits."
    "",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_2_REG_OFFSET,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_2_REG_RAM_CNT,
    4,
    148,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_REG = 
{
    "BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX",
#if RU_INCLUDE_DESC
    "DMA_CFG Register",
    "The BBH reads the packet data from the DDR in chunks (with a maximal size of 128 bytes)."
    "For each chunk the BBH writes a read request (descriptor) into the DMA memory space. The read descriptors are arranged in a predefined space in the DMA memory and managed in a cyclic FIFO style."
    "A special configuration limits the maximum number of read requests.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_REG_OFFSET,
    0,
    0,
    149,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_REG = 
{
    "BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX",
#if RU_INCLUDE_DESC
    "SDMA_CFG Register",
    "The BBH reads the packet data from the PSRAM in chunks (with a maximal size of 128 bytes)."
    "For each chunk the BBH writes a read request (descriptor) into the SDMA memory space. The read descriptors are arranged in a predefined space in the SDMA memory and managed in a cyclic FIFO style."
    "A special configuration limits the maximum number of read requests.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_REG_OFFSET,
    0,
    0,
    150,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG_REG = 
{
    "BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG",
#if RU_INCLUDE_DESC
    "SBPM_CFG Register",
    "When packet transmission is done, the BBH releases the SBPM buffers."
    "This register defines which release command is used:"
    "1. Normal free with context"
    "2. Special free with context"
    "3. free without context",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG_REG_OFFSET,
    0,
    0,
    151,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEL
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEL_REG = 
{
    "BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEL",
#if RU_INCLUDE_DESC
    "DDR_TM_BASE_LOW %i Register",
    "The BBH calculate the DDR physical address according to the Buffer number and buffer size and then adds the DDR TM base."
    ""
    "The DDR TM address space is divided to two - coherent and non coherent."
    ""
    "The first register in this array defines the base address of the non coherent space and the second is for the coherent."
    ""
    "The value of this register should match the relevant registers value in the BBH RX, QM and the Runner."
    "",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEL_REG_OFFSET,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEL_REG_RAM_CNT,
    4,
    152,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEH
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEH_REG = 
{
    "BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEH",
#if RU_INCLUDE_DESC
    "DDR_TM_BASE_HIGH %i Register",
    "The BBH calculate the DDR physical address according to the Buffer number and buffer size and then adds the DDR TM base."
    ""
    "The DDR TM address space is divided to two - coherent and non coherent."
    ""
    "The first register in this array defines the base address of the non coherent space and the second is for the coherent."
    ""
    "The value of this register should match the relevant registers value in the BBH RX, QM and the Runner."
    "",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEH_REG_OFFSET,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEH_REG_RAM_CNT,
    4,
    153,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DFIFOCTRL
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DFIFOCTRL_REG = 
{
    "BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DFIFOCTRL",
#if RU_INCLUDE_DESC
    "DATA_FIFO_CTRL Register",
    "The BBH orders data both from DDR and PSRAM. The returned data is stored in two FIFOs for reordering. The two FIFOs are implemented in a single RAM. This register defines the division of the RAM to two FIFOs.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DFIFOCTRL_REG_OFFSET,
    0,
    0,
    154,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_ARB_CFG
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_ARB_CFG_REG = 
{
    "BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_ARB_CFG",
#if RU_INCLUDE_DESC
    "ARB_CFG Register",
    "configurations related to different arbitration processes (ordering PDs, ordering data)",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_ARB_CFG_REG_OFFSET,
    0,
    0,
    155,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBROUTE
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBROUTE_REG = 
{
    "BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBROUTE",
#if RU_INCLUDE_DESC
    "BB_ROUTE_OVERRIDE Register",
    "override configuration for the route of one of the peripherals (DMA/SDMMA/FPM/SBPM?Runners)",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBROUTE_REG_OFFSET,
    0,
    0,
    156,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_Q2RNR
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_Q2RNR_REG = 
{
    "BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_Q2RNR",
#if RU_INCLUDE_DESC
    "Q_TO_RNR %i Register",
    "configuration which queue is managed by each of the two runners."
    ""
    "Each register in this array configures 2 queues.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_Q2RNR_REG_OFFSET,
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_Q2RNR_REG_RAM_CNT,
    4,
    157,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_REG = 
{
    "BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK",
#if RU_INCLUDE_DESC
    "PER_Q_TASK Register",
    "which task in the runner to wake-up when requesting a PD for a certain q."
    ""
    "This register holds the task number of the first 8 queues."
    ""
    "For queues 8-40 (if they exist) the task that will be waking is the one appearing in the PD_RNR_CFG regs, depending on which runner this queue is associated with.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_REG_OFFSET,
    0,
    0,
    158,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_REG = 
{
    "BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD",
#if RU_INCLUDE_DESC
    "TX_RESET_COMMAND Register",
    "This register enables reset of internal units (for possible WA purposes).",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_REG_OFFSET,
    0,
    0,
    159,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DBGSEL
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DBGSEL_REG = 
{
    "BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DBGSEL",
#if RU_INCLUDE_DESC
    "DEBUG_SELECT Register",
    "This register selects 1 of 8 debug vectors."
    "The selected vector is reflected to DBGOUTREG.",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DBGSEL_REG_OFFSET,
    0,
    0,
    160,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_REG = 
{
    "BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL",
#if RU_INCLUDE_DESC
    "CLOCK_GATE_CONTROL Register",
    "Clock Gate control register including timer config and bypass control",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_REG_OFFSET,
    0,
    0,
    161,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_GPR
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_GPR_REG = 
{
    "BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_GPR",
#if RU_INCLUDE_DESC
    "GENERAL_PURPOSE_REGISTER Register",
    "general purpose register",
#endif
    QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_GPR_REG_OFFSET,
    0,
    0,
    162,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDBASE
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDBASE_REG = 
{
    "BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDBASE",
#if RU_INCLUDE_DESC
    "PD_FIFO_BASE Register",
    "The BBH manages 40 queues for GPON or 32 queus for EPON (1 for each TCONT/LLID). For each queue it manages a PD FIFO."
    "A total of 256 PDs are available for all queues."
    "For each Queue the SW configures the base and the size within these 256 PDs."
    ""
    "The size of the 1st BN FIFO and get-next FIFO is the same as the size of the PD FIFO of each queue."
    ""
    "each register in this array defines the PD FIFO base of 2 queues."
    "",
#endif
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDBASE_REG_OFFSET,
    0,
    0,
    163,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDSIZE
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDSIZE_REG = 
{
    "BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDSIZE",
#if RU_INCLUDE_DESC
    "PD_FIFO_SIZE Register",
    "The BBH manages 40 queues for GPON and 32 queues for EPON (FIFO per TCONT/LLID). For each queue it manages a PD FIFO."
    "A total of 256 PDs are available for all queues."
    "For each Queue the SW configures the base and the size within these."
    "each register in this array defines the PD FIFO size of 2 queues."
    "",
#endif
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDSIZE_REG_OFFSET,
    0,
    0,
    164,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDWKUPH
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDWKUPH_REG = 
{
    "BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDWKUPH",
#if RU_INCLUDE_DESC
    "PD_WKUP_THRESH Register",
    "When a FIFO occupancy is above this wakeup threshold, the BBH will not wake-up the Runner for sending a new PD. This threshold does not represent the actual size of the FIFO. If a PD will arrive from the Runner when the FIFO is above the threshold, it will not be dropped unless the FIFO is actually full."
    "Each register defines the threshold of 2 queues."
    "",
#endif
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDWKUPH_REG_OFFSET,
    0,
    0,
    165,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_REG = 
{
    "BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH",
#if RU_INCLUDE_DESC
    "PD_BYTES_THRESHOLD Register",
    "The BBH requests PDs from the Runner and maintains a pre-fetch PDs FIFO."
    "The PDs pre fetch is limited either by the PD FIFO configurable size or according to the total number of bytes (deducting bytes already requested/transmitted) for preventing HOL. Full configuration for the first 8 TCONT and one configuration for the rest (TCONTs 8-39)."
    "Each register in this array defines the threshold of 2 queues.",
#endif
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_REG_OFFSET,
    0,
    0,
    166,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN_REG = 
{
    "BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN",
#if RU_INCLUDE_DESC
    "PD_BYTES_THRESHOLD_EN Register",
    "The BBH requests PDs from the Runner and maintains a pre-fetch PDs FIFO."
    "The PDs pre fetch is limited either by the PD FIFO configurable size or according to the total number of bytes (deducting bytes already requested/transmitted) for preventing HOL. Full configuration for the first 8 TCONT and one configuration per group of 8 TCONTs for the rest.",
#endif
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN_REG_OFFSET,
    0,
    0,
    167,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDEMPTY
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDEMPTY_REG = 
{
    "BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDEMPTY",
#if RU_INCLUDE_DESC
    "PD_EMPTY_THRESHOLD Register",
    "The BBH manages 32 queues for EPON (FIFO per LLID). For each queue it manages a PD FIFO. Usually, the BBH orders PDs from the Runner in RR between all queues. In EPON BBH, if a FIFO occupancy is below this threshold, the queue will have higher priority in PD ordering arbitration (with RR between all the empty queues)."
    "This configuration is global for all queues."
    "Relevant only for EPON BBH.",
#endif
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDEMPTY_REG_OFFSET,
    0,
    0,
    168,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TXTHRESH
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TXTHRESH_REG = 
{
    "BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TXTHRESH",
#if RU_INCLUDE_DESC
    "TX_THRESHOLD Register",
    "Transmit threshold in 8 bytes resolution."
    "The BBH TX will not start to transmit data towards the XLMAC until the amount of data in the TX FIFO is larger than the threshold or if there is a complete packet in the FIFO.",
#endif
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TXTHRESH_REG_OFFSET,
    0,
    0,
    169,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_EEE
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_EEE_REG = 
{
    "BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_EEE",
#if RU_INCLUDE_DESC
    "EEE Register",
    "The BBH is responsible for indicating the XLMAC that no traffic is about to arrive so the XLMAC may try to enter power saving mode."
    ""
    "This register is used to enable this feature.",
#endif
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_EEE_REG_OFFSET,
    0,
    0,
    170,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TS
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TS_REG = 
{
    "BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TS",
#if RU_INCLUDE_DESC
    "TS Register",
    "The BBH is responsible for indicating the XLMAC that it should and calculate timestamp for the current packet that is being transmitted. The BBH gets the timestamping parameters in the PD and forward it to the XLMAC."
    ""
    "This register is used to enable this feature.",
#endif
    QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TS_REG_OFFSET,
    0,
    0,
    171,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMPD
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMPD_REG = 
{
    "BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMPD",
#if RU_INCLUDE_DESC
    "SRAM_PD_COUNTER Register",
    "This counter counts the number of received PD for packets to be transmitted from the SRAM."
    "It counts the packets for all queues together."
    "This counter is cleared when read and freezes when maximum value is reached."
    "",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMPD_REG_OFFSET,
    0,
    0,
    172,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRPD
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRPD_REG = 
{
    "BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRPD",
#if RU_INCLUDE_DESC
    "DDR_PD_COUNTER Register",
    "This counter counts the number of received PDs for packets to be transmitted from the DDR."
    "It counts the packets for all queues together."
    "This counter is cleared when read and freezes when maximum value is reached.",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRPD_REG_OFFSET,
    0,
    0,
    173,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_PDDROP
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_PDDROP_REG = 
{
    "BBH_TX_QM_BBHTX_DEBUG_COUNTERS_PDDROP",
#if RU_INCLUDE_DESC
    "PD_DROP_COUNTER Register",
    "This counter counts the number of PDs which were dropped due to PD FIFO full."
    "It counts the packets for all queues together."
    "This counter is cleared when read and freezes when maximum value is reached."
    "",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_PDDROP_REG_OFFSET,
    0,
    0,
    174,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSCNT
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSCNT_REG = 
{
    "BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSCNT",
#if RU_INCLUDE_DESC
    "STS_COUNTER Register",
    "This counter counts the number of STS messages which were received from Runner."
    "It counts the packets for all queues together."
    "This counter is cleared when read and freezes when maximum value is reached."
    "",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSCNT_REG_OFFSET,
    0,
    0,
    175,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSDROP
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSDROP_REG = 
{
    "BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSDROP",
#if RU_INCLUDE_DESC
    "STS_DROP_COUNTER Register",
    "This counter counts the number of STS which were dropped due to PD FIFO full."
    "It counts the packets for all queues together."
    "This counter is cleared when read and freezes when maximum value is reached."
    "",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSDROP_REG_OFFSET,
    0,
    0,
    176,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGCNT
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGCNT_REG = 
{
    "BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGCNT",
#if RU_INCLUDE_DESC
    "MSG_COUNTER Register",
    "This counter counts the number of MSG (DBR/Ghost) messages which were received from Runner."
    "It counts the packets for all queues together."
    "This counter is cleared when read and freezes when maximum value is reached."
    "",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGCNT_REG_OFFSET,
    0,
    0,
    177,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGDROP
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGDROP_REG = 
{
    "BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGDROP",
#if RU_INCLUDE_DESC
    "MSG_DROP_COUNTER Register",
    "This counter counts the number of MSG which were dropped due to PD FIFO full."
    "It counts the packets for all queues together."
    "This counter is cleared when read and freezes when maximum value is reached."
    "",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGDROP_REG_OFFSET,
    0,
    0,
    178,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_GETNEXTNULL
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_GETNEXTNULL_REG = 
{
    "BBH_TX_QM_BBHTX_DEBUG_COUNTERS_GETNEXTNULL",
#if RU_INCLUDE_DESC
    "GET_NEXT_IS_NULL_COUNTER Register",
    "This counter counts the number Get next responses with a null BN."
    "It counts the packets for all TCONTs together."
    "This counter is cleared when read and freezes when maximum value is reached."
    "This counter is relevant for Ethernet only.",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_GETNEXTNULL_REG_OFFSET,
    0,
    0,
    179,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_FLUSHPKTS
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_FLUSHPKTS_REG = 
{
    "BBH_TX_QM_BBHTX_DEBUG_COUNTERS_FLUSHPKTS",
#if RU_INCLUDE_DESC
    "FLUSHED_PACKETS_COUNTER Register",
    "This counter counts the number of packets that were flushed (bn was released without sending the data to the EPON MAC) due to flush request."
    "The counter is global for all queues."
    "The counter is read clear.",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_FLUSHPKTS_REG_OFFSET,
    0,
    0,
    180,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_LENERR
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_LENERR_REG = 
{
    "BBH_TX_QM_BBHTX_DEBUG_COUNTERS_LENERR",
#if RU_INCLUDE_DESC
    "REQ_LENGTH_ERROR_COUNTER Register",
    "This counter counts the number of times a length error (mismatch between a request from the MAC and a PD from the Runner) occured."
    "This counter is cleared when read and freezes when maximum value is reached.",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_LENERR_REG_OFFSET,
    0,
    0,
    181,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_AGGRLENERR
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_AGGRLENERR_REG = 
{
    "BBH_TX_QM_BBHTX_DEBUG_COUNTERS_AGGRLENERR",
#if RU_INCLUDE_DESC
    "AGGREGATION_LENGTH_ERROR_COUNTER Register",
    "This counter Counts aggregation length error events."
    "If one or more of the packets in an aggregated PD is shorter than 60 bytes, this counter will be incremented by 1."
    "This counter is cleared when read and freezes when maximum value is reached.",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_AGGRLENERR_REG_OFFSET,
    0,
    0,
    182,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMPKT
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMPKT_REG = 
{
    "BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMPKT",
#if RU_INCLUDE_DESC
    "SRAM_PKT_COUNTER Register",
    "This counter counts the number of received packets to be transmitted from the SRAM."
    "It counts the packets for all queues together."
    "This counter is cleared when read and freezes when maximum value is reached."
    "",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMPKT_REG_OFFSET,
    0,
    0,
    183,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRPKT
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRPKT_REG = 
{
    "BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRPKT",
#if RU_INCLUDE_DESC
    "DDR_PKT_COUNTER Register",
    "This counter counts the number of received packets to be transmitted from the DDR."
    "It counts the packets for all queues together."
    "This counter is cleared when read and freezes when maximum value is reached."
    "",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRPKT_REG_OFFSET,
    0,
    0,
    184,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMBYTE
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMBYTE_REG = 
{
    "BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMBYTE",
#if RU_INCLUDE_DESC
    "SRAM_BYTE_COUNTER Register",
    "This counter counts the number of transmitted bytes from the SRAM."
    "It counts the bytes for all queues together."
    "This counter is cleared when read and freezes when maximum value is reached."
    "",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMBYTE_REG_OFFSET,
    0,
    0,
    185,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRBYTE
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRBYTE_REG = 
{
    "BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRBYTE",
#if RU_INCLUDE_DESC
    "DDR_BYTE_COUNTER Register",
    "This counter counts the number of transmitted bytes from the DDR."
    "It counts the bytes for all queues together."
    "This counter is cleared when read and freezes when maximum value is reached."
    "",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRBYTE_REG_OFFSET,
    0,
    0,
    186,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_REG = 
{
    "BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN",
#if RU_INCLUDE_DESC
    "SW_RD_EN Register",
    "writing to this register creates a rd_en pulse to the selected array the SW wants to access."
    ""
    "Each bit in the register represents one of the arrays the SW can access."
    ""
    "The address inside the array is determined in the previous register (sw_rd_address)."
    ""
    "When writing to this register the SW should assert only one bit. If more than one is asserted, The HW will return the value read from the lsb selected array.",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_REG_OFFSET,
    0,
    0,
    187,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDADDR
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDADDR_REG = 
{
    "BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDADDR",
#if RU_INCLUDE_DESC
    "SW_RD_ADDR Register",
    "the address inside the array the SW wants to read",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDADDR_REG_OFFSET,
    0,
    0,
    188,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDDATA
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDDATA_REG = 
{
    "BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDDATA",
#if RU_INCLUDE_DESC
    "SW_RD_DATA Register",
    "indirect memories and arrays read data",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDDATA_REG_OFFSET,
    0,
    0,
    189,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_UNIFIEDPKT
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_UNIFIEDPKT_REG = 
{
    "BBH_TX_QM_BBHTX_DEBUG_COUNTERS_UNIFIEDPKT",
#if RU_INCLUDE_DESC
    "UNIFIED_PKT_COUNTER %i Register",
    "This counter array counts the number of transmitted packets through each interface in the unified BBH."
    ""
    "This counter is cleared when read and freezes when maximum value is reached."
    "",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_UNIFIEDPKT_REG_OFFSET,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_UNIFIEDPKT_REG_RAM_CNT,
    4,
    190,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_UNIFIEDBYTE
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_UNIFIEDBYTE_REG = 
{
    "BBH_TX_QM_BBHTX_DEBUG_COUNTERS_UNIFIEDBYTE",
#if RU_INCLUDE_DESC
    "UNIFIED_BYTE_COUNTER %i Register",
    "This counter array counts the number of transmitted bytes through each interface in the unified BBH."
    ""
    "This counter is cleared when read and freezes when maximum value is reached."
    "",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_UNIFIEDBYTE_REG_OFFSET,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_UNIFIEDBYTE_REG_RAM_CNT,
    4,
    191,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DBGOUTREG
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DBGOUTREG_REG = 
{
    "BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DBGOUTREG",
#if RU_INCLUDE_DESC
    "DEBUG_OUT_REG %i Register",
    "an array including all the debug vectors of the BBH TX."
    "entries 30 and 31 are DSL debug.",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DBGOUTREG_REG_OFFSET,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DBGOUTREG_REG_RAM_CNT,
    4,
    192,
};

/******************************************************************************
 * Register: QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_IN_SEGMENTATION
 ******************************************************************************/
const ru_reg_rec QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_IN_SEGMENTATION_REG = 
{
    "BBH_TX_QM_BBHTX_DEBUG_COUNTERS_IN_SEGMENTATION",
#if RU_INCLUDE_DESC
    "IN_SEGMENTATION %i Register",
    "40 bit vector in which each bit represents if the segmentation SM is currently handling a PD of a certain TCONT."
    ""
    "first address is for TCONTS [31:0]"
    "second is for TCONTS [39:32]",
#endif
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_IN_SEGMENTATION_REG_OFFSET,
    QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_IN_SEGMENTATION_REG_RAM_CNT,
    4,
    193,
};

/******************************************************************************
 * Register: QM_DMA_QM_DMA_CONFIG_BBROUTEOVRD
 ******************************************************************************/
const ru_reg_rec QM_DMA_QM_DMA_CONFIG_BBROUTEOVRD_REG = 
{
    "DMA_QM_DMA_CONFIG_BBROUTEOVRD",
#if RU_INCLUDE_DESC
    "BB_ROUTE_OVERRIDE Register",
    "Broadbus route override",
#endif
    QM_DMA_QM_DMA_CONFIG_BBROUTEOVRD_REG_OFFSET,
    0,
    0,
    194,
};

/******************************************************************************
 * Register: QM_DMA_QM_DMA_CONFIG_NUM_OF_WRITES
 ******************************************************************************/
const ru_reg_rec QM_DMA_QM_DMA_CONFIG_NUM_OF_WRITES_REG = 
{
    "DMA_QM_DMA_CONFIG_NUM_OF_WRITES",
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
    QM_DMA_QM_DMA_CONFIG_NUM_OF_WRITES_REG_OFFSET,
    QM_DMA_QM_DMA_CONFIG_NUM_OF_WRITES_REG_RAM_CNT,
    4,
    195,
};

/******************************************************************************
 * Register: QM_DMA_QM_DMA_CONFIG_NUM_OF_READS
 ******************************************************************************/
const ru_reg_rec QM_DMA_QM_DMA_CONFIG_NUM_OF_READS_REG = 
{
    "DMA_QM_DMA_CONFIG_NUM_OF_READS",
#if RU_INCLUDE_DESC
    "NUM_OF_READ_REQ %i Register",
    "This array of registers controls the number of read requests of each peripheral within the read requests RAM."
    "total of 64 requests are divided between peripherals."
    "Base address of peripheral 0 is 0, base of peripheral 1 is 0 + periph0_num_of_read_requests and so on.",
#endif
    QM_DMA_QM_DMA_CONFIG_NUM_OF_READS_REG_OFFSET,
    QM_DMA_QM_DMA_CONFIG_NUM_OF_READS_REG_RAM_CNT,
    4,
    196,
};

/******************************************************************************
 * Register: QM_DMA_QM_DMA_CONFIG_U_THRESH
 ******************************************************************************/
const ru_reg_rec QM_DMA_QM_DMA_CONFIG_U_THRESH_REG = 
{
    "DMA_QM_DMA_CONFIG_U_THRESH",
#if RU_INCLUDE_DESC
    "URGENT_THRESHOLDS %i Register",
    "the in/out of urgent thresholds mark the number of write requests in the queue in which the peripherals priority is changed. The two thresholds should create hysteresis."
    "The moving into urgent threshold must always be greater than the moving out of urgent threshold.",
#endif
    QM_DMA_QM_DMA_CONFIG_U_THRESH_REG_OFFSET,
    QM_DMA_QM_DMA_CONFIG_U_THRESH_REG_RAM_CNT,
    4,
    197,
};

/******************************************************************************
 * Register: QM_DMA_QM_DMA_CONFIG_PRI
 ******************************************************************************/
const ru_reg_rec QM_DMA_QM_DMA_CONFIG_PRI_REG = 
{
    "DMA_QM_DMA_CONFIG_PRI",
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
    QM_DMA_QM_DMA_CONFIG_PRI_REG_OFFSET,
    QM_DMA_QM_DMA_CONFIG_PRI_REG_RAM_CNT,
    4,
    198,
};

/******************************************************************************
 * Register: QM_DMA_QM_DMA_CONFIG_PERIPH_SOURCE
 ******************************************************************************/
const ru_reg_rec QM_DMA_QM_DMA_CONFIG_PERIPH_SOURCE_REG = 
{
    "DMA_QM_DMA_CONFIG_PERIPH_SOURCE",
#if RU_INCLUDE_DESC
    "BB_SOURCE_DMA_PERIPH %i Register",
    "Broadbus source address of the DMA peripherals. Register per peripheral (rx and tx). The source is used to determine the route address to the different peripherals.",
#endif
    QM_DMA_QM_DMA_CONFIG_PERIPH_SOURCE_REG_OFFSET,
    QM_DMA_QM_DMA_CONFIG_PERIPH_SOURCE_REG_RAM_CNT,
    4,
    199,
};

/******************************************************************************
 * Register: QM_DMA_QM_DMA_CONFIG_WEIGHT
 ******************************************************************************/
const ru_reg_rec QM_DMA_QM_DMA_CONFIG_WEIGHT_REG = 
{
    "DMA_QM_DMA_CONFIG_WEIGHT",
#if RU_INCLUDE_DESC
    "WEIGHT_OF_ROUND_ROBIN %i Register",
    "The second phase of the arbitration between requests is weighted round robin between requests of peripherals with the same priority."
    "This array of registers allow configurtion of the weight of each peripheral (rx and tx). The actual weight will be weight + 1, meaning configuration of 0 is actual weight of 1.",
#endif
    QM_DMA_QM_DMA_CONFIG_WEIGHT_REG_OFFSET,
    QM_DMA_QM_DMA_CONFIG_WEIGHT_REG_RAM_CNT,
    4,
    200,
};

/******************************************************************************
 * Register: QM_DMA_QM_DMA_CONFIG_PTRRST
 ******************************************************************************/
const ru_reg_rec QM_DMA_QM_DMA_CONFIG_PTRRST_REG = 
{
    "DMA_QM_DMA_CONFIG_PTRRST",
#if RU_INCLUDE_DESC
    "POINTERS_RESET Register",
    "Resets the pointers of the peripherals FIFOs within the DMA. Bit per peripheral side (rx and tx)."
    "For rx side resets the data and CD FIFOs."
    "For tx side resets the read requests FIFO.",
#endif
    QM_DMA_QM_DMA_CONFIG_PTRRST_REG_OFFSET,
    0,
    0,
    201,
};

/******************************************************************************
 * Register: QM_DMA_QM_DMA_CONFIG_MAX_OTF
 ******************************************************************************/
const ru_reg_rec QM_DMA_QM_DMA_CONFIG_MAX_OTF_REG = 
{
    "DMA_QM_DMA_CONFIG_MAX_OTF",
#if RU_INCLUDE_DESC
    "MAX_ON_THE_FLY Register",
    "max number of on the fly read commands the DMA may issue to DDR before receiving any data.",
#endif
    QM_DMA_QM_DMA_CONFIG_MAX_OTF_REG_OFFSET,
    0,
    0,
    202,
};

/******************************************************************************
 * Register: QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL
 ******************************************************************************/
const ru_reg_rec QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_REG = 
{
    "DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL",
#if RU_INCLUDE_DESC
    "CLOCK_GATE_CONTROL Register",
    "Clock Gate control register including timer config and bypass control",
#endif
    QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_REG_OFFSET,
    0,
    0,
    203,
};

/******************************************************************************
 * Register: QM_DMA_QM_DMA_CONFIG_DBG_SEL
 ******************************************************************************/
const ru_reg_rec QM_DMA_QM_DMA_CONFIG_DBG_SEL_REG = 
{
    "DMA_QM_DMA_CONFIG_DBG_SEL",
#if RU_INCLUDE_DESC
    "DBG_SEL Register",
    "debug bus select",
#endif
    QM_DMA_QM_DMA_CONFIG_DBG_SEL_REG_OFFSET,
    0,
    0,
    204,
};

/******************************************************************************
 * Register: QM_DMA_QM_DMA_DEBUG_NEMPTY
 ******************************************************************************/
const ru_reg_rec QM_DMA_QM_DMA_DEBUG_NEMPTY_REG = 
{
    "DMA_QM_DMA_DEBUG_NEMPTY",
#if RU_INCLUDE_DESC
    "NOT_EMPTY_VECTOR Register",
    "Each peripheral is represented in a bit on the not empty vector."
    "LSB is for rx peripherals, MSB for tx peripherals."
    "If the bit is asserted, the requests queue of the relevant peripheral is not empty."
    "The not empty vector is used by the DMA scheduler to determine which peripheral is the next to be served.",
#endif
    QM_DMA_QM_DMA_DEBUG_NEMPTY_REG_OFFSET,
    0,
    0,
    205,
};

/******************************************************************************
 * Register: QM_DMA_QM_DMA_DEBUG_URGNT
 ******************************************************************************/
const ru_reg_rec QM_DMA_QM_DMA_DEBUG_URGNT_REG = 
{
    "DMA_QM_DMA_DEBUG_URGNT",
#if RU_INCLUDE_DESC
    "URGENT_VECTOR Register",
    "Each peripheral, a is represented in a bit on the urgent vector. 8 LSB are rx peripherlas, 8 MSB are tx peripherals."
    "If the bit is asserted, the requests queue of the relevant peripheral is in urgent state."
    "The urgent vector is used by the DMA scheduler to determine which peripheral is the next to be served.",
#endif
    QM_DMA_QM_DMA_DEBUG_URGNT_REG_OFFSET,
    0,
    0,
    206,
};

/******************************************************************************
 * Register: QM_DMA_QM_DMA_DEBUG_SELSRC
 ******************************************************************************/
const ru_reg_rec QM_DMA_QM_DMA_DEBUG_SELSRC_REG = 
{
    "DMA_QM_DMA_DEBUG_SELSRC",
#if RU_INCLUDE_DESC
    "SELECTED_SOURCE_NUM Register",
    "The decision of the dma schedule rand the next peripheral to be served, represented by its source address",
#endif
    QM_DMA_QM_DMA_DEBUG_SELSRC_REG_OFFSET,
    0,
    0,
    207,
};

/******************************************************************************
 * Register: QM_DMA_QM_DMA_DEBUG_REQ_CNT_RX
 ******************************************************************************/
const ru_reg_rec QM_DMA_QM_DMA_DEBUG_REQ_CNT_RX_REG = 
{
    "DMA_QM_DMA_DEBUG_REQ_CNT_RX",
#if RU_INCLUDE_DESC
    "REQUEST_COUNTERS_RX %i Register",
    "the number of write requests currently pending for each rx peripheral.",
#endif
    QM_DMA_QM_DMA_DEBUG_REQ_CNT_RX_REG_OFFSET,
    QM_DMA_QM_DMA_DEBUG_REQ_CNT_RX_REG_RAM_CNT,
    4,
    208,
};

/******************************************************************************
 * Register: QM_DMA_QM_DMA_DEBUG_REQ_CNT_TX
 ******************************************************************************/
const ru_reg_rec QM_DMA_QM_DMA_DEBUG_REQ_CNT_TX_REG = 
{
    "DMA_QM_DMA_DEBUG_REQ_CNT_TX",
#if RU_INCLUDE_DESC
    "REQUEST_COUNTERS_TX %i Register",
    "the number of read requestscurrently pending for each TX peripheral.",
#endif
    QM_DMA_QM_DMA_DEBUG_REQ_CNT_TX_REG_OFFSET,
    QM_DMA_QM_DMA_DEBUG_REQ_CNT_TX_REG_RAM_CNT,
    4,
    209,
};

/******************************************************************************
 * Register: QM_DMA_QM_DMA_DEBUG_REQ_CNT_RX_ACC
 ******************************************************************************/
const ru_reg_rec QM_DMA_QM_DMA_DEBUG_REQ_CNT_RX_ACC_REG = 
{
    "DMA_QM_DMA_DEBUG_REQ_CNT_RX_ACC",
#if RU_INCLUDE_DESC
    "ACC_REQUEST_COUNTERS_RX %i Register",
    "the accumulated number of write requests served so far for each peripheral. Wrap around on max value, not read clear.",
#endif
    QM_DMA_QM_DMA_DEBUG_REQ_CNT_RX_ACC_REG_OFFSET,
    QM_DMA_QM_DMA_DEBUG_REQ_CNT_RX_ACC_REG_RAM_CNT,
    4,
    210,
};

/******************************************************************************
 * Register: QM_DMA_QM_DMA_DEBUG_REQ_CNT_TX_ACC
 ******************************************************************************/
const ru_reg_rec QM_DMA_QM_DMA_DEBUG_REQ_CNT_TX_ACC_REG = 
{
    "DMA_QM_DMA_DEBUG_REQ_CNT_TX_ACC",
#if RU_INCLUDE_DESC
    "ACC_REQUEST_COUNTERS_TX %i Register",
    "the accumulated number of read requests served so far for each peripheral. Wrap around on max value, not read clear.",
#endif
    QM_DMA_QM_DMA_DEBUG_REQ_CNT_TX_ACC_REG_OFFSET,
    QM_DMA_QM_DMA_DEBUG_REQ_CNT_TX_ACC_REG_RAM_CNT,
    4,
    211,
};

/******************************************************************************
 * Register: QM_DMA_QM_DMA_DEBUG_RDADD
 ******************************************************************************/
const ru_reg_rec QM_DMA_QM_DMA_DEBUG_RDADD_REG = 
{
    "DMA_QM_DMA_DEBUG_RDADD",
#if RU_INCLUDE_DESC
    "RAM_ADDRES Register",
    "the address and cs of the ram the user wishes to read using the indirect access read mechanism.",
#endif
    QM_DMA_QM_DMA_DEBUG_RDADD_REG_OFFSET,
    0,
    0,
    212,
};

/******************************************************************************
 * Register: QM_DMA_QM_DMA_DEBUG_RDVALID
 ******************************************************************************/
const ru_reg_rec QM_DMA_QM_DMA_DEBUG_RDVALID_REG = 
{
    "DMA_QM_DMA_DEBUG_RDVALID",
#if RU_INCLUDE_DESC
    "INDIRECT_READ_REQUEST_VALID Register",
    "After determining the address and cs, the user should assert this bit for indicating that the address and cs are valid.",
#endif
    QM_DMA_QM_DMA_DEBUG_RDVALID_REG_OFFSET,
    0,
    0,
    213,
};

/******************************************************************************
 * Register: QM_DMA_QM_DMA_DEBUG_RDDATA
 ******************************************************************************/
const ru_reg_rec QM_DMA_QM_DMA_DEBUG_RDDATA_REG = 
{
    "DMA_QM_DMA_DEBUG_RDDATA",
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
    QM_DMA_QM_DMA_DEBUG_RDDATA_REG_OFFSET,
    QM_DMA_QM_DMA_DEBUG_RDDATA_REG_RAM_CNT,
    4,
    214,
};

/******************************************************************************
 * Register: QM_DMA_QM_DMA_DEBUG_RDDATARDY
 ******************************************************************************/
const ru_reg_rec QM_DMA_QM_DMA_DEBUG_RDDATARDY_REG = 
{
    "DMA_QM_DMA_DEBUG_RDDATARDY",
#if RU_INCLUDE_DESC
    "READ_DATA_READY Register",
    "When assertd indicats that the data in the previous array is valid.Willremain asserted until the user deasserts the valid bit in regiser RDVALID.",
#endif
    QM_DMA_QM_DMA_DEBUG_RDDATARDY_REG_OFFSET,
    0,
    0,
    215,
};

/******************************************************************************
 * Block: QM
 ******************************************************************************/
static const ru_reg_rec *QM_REGS[] =
{
    &QM_GLOBAL_CFG_QM_ENABLE_CTRL_REG,
    &QM_GLOBAL_CFG_QM_SW_RST_CTRL_REG,
    &QM_GLOBAL_CFG_QM_GENERAL_CTRL_REG,
    &QM_GLOBAL_CFG_FPM_CONTROL_REG,
    &QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_CONTROL_REG,
    &QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_LOWER_THR_REG,
    &QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_MID_THR_REG,
    &QM_GLOBAL_CFG_DDR_BYTE_CONGESTION_HIGHER_THR_REG,
    &QM_GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL_REG,
    &QM_GLOBAL_CFG_QM_PD_CONGESTION_CONTROL_REG,
    &QM_GLOBAL_CFG_ABS_DROP_QUEUE_REG,
    &QM_GLOBAL_CFG_AGGREGATION_CTRL_REG,
    &QM_GLOBAL_CFG_FPM_BASE_ADDR_REG,
    &QM_GLOBAL_CFG_FPM_COHERENT_BASE_ADDR_REG,
    &QM_GLOBAL_CFG_DDR_SOP_OFFSET_REG,
    &QM_GLOBAL_CFG_EPON_OVERHEAD_CTRL_REG,
    &QM_GLOBAL_CFG_DQM_FULL_REG,
    &QM_GLOBAL_CFG_DQM_NOT_EMPTY_REG,
    &QM_GLOBAL_CFG_DQM_POP_READY_REG,
    &QM_GLOBAL_CFG_AGGREGATION_CONTEXT_VALID_REG,
    &QM_GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL_REG,
    &QM_GLOBAL_CFG_QM_FPM_UG_GBL_CNT_REG,
    &QM_GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE_REG,
    &QM_FPM_POOLS_THR_REG,
    &QM_FPM_USR_GRP_LOWER_THR_REG,
    &QM_FPM_USR_GRP_MID_THR_REG,
    &QM_FPM_USR_GRP_HIGHER_THR_REG,
    &QM_FPM_USR_GRP_CNT_REG,
    &QM_RUNNER_GRP_RNR_CONFIG_REG,
    &QM_RUNNER_GRP_QUEUE_CONFIG_REG,
    &QM_RUNNER_GRP_PDFIFO_CONFIG_REG,
    &QM_RUNNER_GRP_UPDATE_FIFO_CONFIG_REG,
    &QM_INTR_CTRL_ISR_REG,
    &QM_INTR_CTRL_ISM_REG,
    &QM_INTR_CTRL_IER_REG,
    &QM_INTR_CTRL_ITR_REG,
    &QM_CLK_GATE_CLK_GATE_CNTRL_REG,
    &QM_CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL_REG,
    &QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_0_REG,
    &QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_1_REG,
    &QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_2_REG,
    &QM_CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_3_REG,
    &QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_0_REG,
    &QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_1_REG,
    &QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_2_REG,
    &QM_CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_3_REG,
    &QM_QUEUE_CONTEXT_CONTEXT_REG,
    &QM_WRED_PROFILE_COLOR_MIN_THR_0_REG,
    &QM_WRED_PROFILE_COLOR_MIN_THR_1_REG,
    &QM_WRED_PROFILE_COLOR_MAX_THR_0_REG,
    &QM_WRED_PROFILE_COLOR_MAX_THR_1_REG,
    &QM_WRED_PROFILE_COLOR_SLOPE_0_REG,
    &QM_WRED_PROFILE_COLOR_SLOPE_1_REG,
    &QM_COPY_DECISION_PROFILE_THR_REG,
    &QM_TOTAL_VALID_COUNTER_COUNTER_REG,
    &QM_DQM_VALID_COUNTER_COUNTER_REG,
    &QM_DROP_COUNTER_COUNTER_REG,
    &QM_EPON_RPT_CNT_COUNTER_REG,
    &QM_EPON_RPT_CNT_QUEUE_STATUS_REG,
    &QM_RD_DATA_POOL0_REG,
    &QM_RD_DATA_POOL1_REG,
    &QM_RD_DATA_POOL2_REG,
    &QM_RD_DATA_POOL3_REG,
    &QM_PDFIFO_PTR_REG,
    &QM_UPDATE_FIFO_PTR_REG,
    &QM_RD_DATA_REG,
    &QM_POP_REG,
    &QM_CM_COMMON_INPUT_FIFO_DATA_REG,
    &QM_NORMAL_RMT_FIFO_DATA_REG,
    &QM_NON_DELAYED_RMT_FIFO_DATA_REG,
    &QM_EGRESS_DATA_FIFO_DATA_REG,
    &QM_EGRESS_RR_FIFO_DATA_REG,
    &QM_EGRESS_BB_INPUT_FIFO_DATA_REG,
    &QM_EGRESS_BB_OUTPUT_FIFO_DATA_REG,
    &QM_BB_OUTPUT_FIFO_DATA_REG,
    &QM_NON_DELAYED_OUT_FIFO_DATA_REG,
    &QM_CONTEXT_DATA_REG,
    &QM_FPM_BUFFER_RESERVATION_DATA_REG,
    &QM_FLOW_CTRL_UG_CTRL_REG,
    &QM_FLOW_CTRL_STATUS_REG,
    &QM_FLOW_CTRL_WRED_SOURCE_REG,
    &QM_FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG_REG,
    &QM_DEBUG_SEL_REG,
    &QM_DEBUG_BUS_LSB_REG,
    &QM_DEBUG_BUS_MSB_REG,
    &QM_QM_SPARE_CONFIG_REG,
    &QM_GOOD_LVL1_PKTS_CNT_REG,
    &QM_GOOD_LVL1_BYTES_CNT_REG,
    &QM_GOOD_LVL2_PKTS_CNT_REG,
    &QM_GOOD_LVL2_BYTES_CNT_REG,
    &QM_COPIED_PKTS_CNT_REG,
    &QM_COPIED_BYTES_CNT_REG,
    &QM_AGG_PKTS_CNT_REG,
    &QM_AGG_BYTES_CNT_REG,
    &QM_AGG_1_PKTS_CNT_REG,
    &QM_AGG_2_PKTS_CNT_REG,
    &QM_AGG_3_PKTS_CNT_REG,
    &QM_AGG_4_PKTS_CNT_REG,
    &QM_WRED_DROP_CNT_REG,
    &QM_FPM_CONGESTION_DROP_CNT_REG,
    &QM_DDR_PD_CONGESTION_DROP_CNT_REG,
    &QM_DDR_BYTE_CONGESTION_DROP_CNT_REG,
    &QM_QM_PD_CONGESTION_DROP_CNT_REG,
    &QM_QM_ABS_REQUEUE_CNT_REG,
    &QM_FPM_PREFETCH_FIFO0_STATUS_REG,
    &QM_FPM_PREFETCH_FIFO1_STATUS_REG,
    &QM_FPM_PREFETCH_FIFO2_STATUS_REG,
    &QM_FPM_PREFETCH_FIFO3_STATUS_REG,
    &QM_NORMAL_RMT_FIFO_STATUS_REG,
    &QM_NON_DELAYED_RMT_FIFO_STATUS_REG,
    &QM_NON_DELAYED_OUT_FIFO_STATUS_REG,
    &QM_PRE_CM_FIFO_STATUS_REG,
    &QM_CM_RD_PD_FIFO_STATUS_REG,
    &QM_CM_WR_PD_FIFO_STATUS_REG,
    &QM_CM_COMMON_INPUT_FIFO_STATUS_REG,
    &QM_BB0_OUTPUT_FIFO_STATUS_REG,
    &QM_BB1_OUTPUT_FIFO_STATUS_REG,
    &QM_BB1_INPUT_FIFO_STATUS_REG,
    &QM_EGRESS_DATA_FIFO_STATUS_REG,
    &QM_EGRESS_RR_FIFO_STATUS_REG,
    &QM_BB_ROUTE_OVR_REG,
    &QM_QM_INGRESS_STAT_REG,
    &QM_QM_EGRESS_STAT_REG,
    &QM_QM_CM_STAT_REG,
    &QM_QM_FPM_PREFETCH_STAT_REG,
    &QM_QM_CONNECT_ACK_COUNTER_REG,
    &QM_QM_DDR_WR_REPLY_COUNTER_REG,
    &QM_QM_DDR_PIPE_BYTE_COUNTER_REG,
    &QM_QM_ABS_REQUEUE_VALID_COUNTER_REG,
    &QM_QM_ILLEGAL_PD_CAPTURE_REG,
    &QM_QM_INGRESS_PROCESSED_PD_CAPTURE_REG,
    &QM_QM_CM_PROCESSED_PD_CAPTURE_REG,
    &QM_FPM_POOL_DROP_CNT_REG,
    &QM_FPM_GRP_DROP_CNT_REG,
    &QM_FPM_BUFFER_RES_DROP_CNT_REG,
    &QM_PSRAM_EGRESS_CONG_DRP_CNT_REG,
    &QM_DATA_REG,
    &QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_VPB_BASE_REG,
    &QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_VPB_MASK_REG,
    &QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_APB_BASE_REG,
    &QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_APB_MASK_REG,
    &QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_DQM_BASE_REG,
    &QM_XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_DQM_MASK_REG,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_MACTYPE_REG,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX_REG,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX_REG,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX_REG,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_1_REG,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_2_REG,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX_REG,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX_REG,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG_REG,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEL_REG,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEH_REG,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DFIFOCTRL_REG,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_ARB_CFG_REG,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBROUTE_REG,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_Q2RNR_REG,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK_REG,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD_REG,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DBGSEL_REG,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL_REG,
    &QM_BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_GPR_REG,
    &QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDBASE_REG,
    &QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDSIZE_REG,
    &QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDWKUPH_REG,
    &QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_REG,
    &QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN_REG,
    &QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDEMPTY_REG,
    &QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TXTHRESH_REG,
    &QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_EEE_REG,
    &QM_BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TS_REG,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMPD_REG,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRPD_REG,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_PDDROP_REG,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSCNT_REG,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSDROP_REG,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGCNT_REG,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGDROP_REG,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_GETNEXTNULL_REG,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_FLUSHPKTS_REG,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_LENERR_REG,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_AGGRLENERR_REG,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMPKT_REG,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRPKT_REG,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMBYTE_REG,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRBYTE_REG,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN_REG,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDADDR_REG,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDDATA_REG,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_UNIFIEDPKT_REG,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_UNIFIEDBYTE_REG,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DBGOUTREG_REG,
    &QM_BBH_TX_QM_BBHTX_DEBUG_COUNTERS_IN_SEGMENTATION_REG,
    &QM_DMA_QM_DMA_CONFIG_BBROUTEOVRD_REG,
    &QM_DMA_QM_DMA_CONFIG_NUM_OF_WRITES_REG,
    &QM_DMA_QM_DMA_CONFIG_NUM_OF_READS_REG,
    &QM_DMA_QM_DMA_CONFIG_U_THRESH_REG,
    &QM_DMA_QM_DMA_CONFIG_PRI_REG,
    &QM_DMA_QM_DMA_CONFIG_PERIPH_SOURCE_REG,
    &QM_DMA_QM_DMA_CONFIG_WEIGHT_REG,
    &QM_DMA_QM_DMA_CONFIG_PTRRST_REG,
    &QM_DMA_QM_DMA_CONFIG_MAX_OTF_REG,
    &QM_DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL_REG,
    &QM_DMA_QM_DMA_CONFIG_DBG_SEL_REG,
    &QM_DMA_QM_DMA_DEBUG_NEMPTY_REG,
    &QM_DMA_QM_DMA_DEBUG_URGNT_REG,
    &QM_DMA_QM_DMA_DEBUG_SELSRC_REG,
    &QM_DMA_QM_DMA_DEBUG_REQ_CNT_RX_REG,
    &QM_DMA_QM_DMA_DEBUG_REQ_CNT_TX_REG,
    &QM_DMA_QM_DMA_DEBUG_REQ_CNT_RX_ACC_REG,
    &QM_DMA_QM_DMA_DEBUG_REQ_CNT_TX_ACC_REG,
    &QM_DMA_QM_DMA_DEBUG_RDADD_REG,
    &QM_DMA_QM_DMA_DEBUG_RDVALID_REG,
    &QM_DMA_QM_DMA_DEBUG_RDDATA_REG,
    &QM_DMA_QM_DMA_DEBUG_RDDATARDY_REG,
};

unsigned long QM_ADDRS[] =
{
    0x82100000,
};

const ru_block_rec QM_BLOCK = 
{
    "QM",
    QM_ADDRS,
    1,
    216,
    QM_REGS
};

/* End of file XRDP_QM.c */
