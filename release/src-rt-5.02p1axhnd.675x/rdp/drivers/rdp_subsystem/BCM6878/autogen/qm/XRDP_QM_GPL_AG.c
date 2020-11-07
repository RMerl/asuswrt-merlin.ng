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
 * Register: QM_BACKPRESSURE
 ******************************************************************************/
const ru_reg_rec QM_BACKPRESSURE_REG = 
{
    "BACKPRESSURE",
#if RU_INCLUDE_DESC
    "BACKPRESSURE Register",
    "Back pressure sets the relevant register per bp reason. SW should unset.",
#endif
    QM_BACKPRESSURE_REG_OFFSET,
    0,
    0,
    136,
};

/******************************************************************************
 * Register: QM_GLOBAL_CFG2_BBHTX_FIFO_ADDR
 ******************************************************************************/
const ru_reg_rec QM_GLOBAL_CFG2_BBHTX_FIFO_ADDR_REG = 
{
    "GLOBAL_CFG2_BBHTX_FIFO_ADDR",
#if RU_INCLUDE_DESC
    "BBHTX_FIFO_ADDR Register",
    "QMs BBH TX FIFO address."
    "bit 10:5 of BBs target address to QM. relevant only for project where the BBHTX_SDMA are external to QM",
#endif
    QM_GLOBAL_CFG2_BBHTX_FIFO_ADDR_REG_OFFSET,
    0,
    0,
    137,
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
    138,
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
    &QM_BACKPRESSURE_REG,
    &QM_GLOBAL_CFG2_BBHTX_FIFO_ADDR_REG,
    &QM_DATA_REG,
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
    139,
    QM_REGS
};

/* End of file XRDP_QM.c */
