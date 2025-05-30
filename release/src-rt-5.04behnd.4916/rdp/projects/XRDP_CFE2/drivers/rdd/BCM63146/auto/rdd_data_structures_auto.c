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



/* This is an automated file. Do not edit its contents. */


#include "packing.h"
#include "rdd.h"
#include "ru_types.h"

#include "rdd_data_structures_auto.h"

/* >>>RDD_SRAM_PD_FIFO_ADDRESS_ARR */
uint32_t RDD_SRAM_PD_FIFO_ADDRESS_ARR[NUM_OF_RUNNER_CORES] = {
	0x0,
};
/* <<<RDD_SRAM_PD_FIFO_ADDRESS_ARR */


/* >>>RDD_US_TM_BBH_TX_EGRESS_COUNTER_TABLE_ADDRESS_ARR */
uint32_t RDD_US_TM_BBH_TX_EGRESS_COUNTER_TABLE_ADDRESS_ARR[NUM_OF_RUNNER_CORES] = {
	0x100,
};
/* <<<RDD_US_TM_BBH_TX_EGRESS_COUNTER_TABLE_ADDRESS_ARR */


/* >>>RDD_DIRECT_PROCESSING_PD_TABLE_ADDRESS_ARR */
uint32_t RDD_DIRECT_PROCESSING_PD_TABLE_ADDRESS_ARR[NUM_OF_RUNNER_CORES] = {
	0x140,
};
/* <<<RDD_DIRECT_PROCESSING_PD_TABLE_ADDRESS_ARR */


/* >>>RDD_CPU_RX_SCRATCHPAD_ADDRESS_ARR */
uint32_t RDD_CPU_RX_SCRATCHPAD_ADDRESS_ARR[NUM_OF_RUNNER_CORES] = {
	0x160,
};
/* <<<RDD_CPU_RX_SCRATCHPAD_ADDRESS_ARR */


/* >>>RDD_CPU_TX_SCRATCHPAD_ADDRESS_ARR */
uint32_t RDD_CPU_TX_SCRATCHPAD_ADDRESS_ARR[NUM_OF_RUNNER_CORES] = {
	0x760,
};
/* <<<RDD_CPU_TX_SCRATCHPAD_ADDRESS_ARR */


/* >>>RDD_BBH_TX_RING_TABLE_ADDRESS_ARR */
uint32_t RDD_BBH_TX_RING_TABLE_ADDRESS_ARR[NUM_OF_RUNNER_CORES] = {
	0xd60,
};
/* <<<RDD_BBH_TX_RING_TABLE_ADDRESS_ARR */


/* >>>RDD_CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE_ADDRESS_ARR */
uint32_t RDD_CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE_ADDRESS_ARR[NUM_OF_RUNNER_CORES] = {
	0xd70,
};
/* <<<RDD_CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE_ADDRESS_ARR */


/* >>>RDD_BBH_TX_BB_DESTINATION_TABLE_ADDRESS_ARR */
uint32_t RDD_BBH_TX_BB_DESTINATION_TABLE_ADDRESS_ARR[NUM_OF_RUNNER_CORES] = {
	0xd7c,
};
/* <<<RDD_BBH_TX_BB_DESTINATION_TABLE_ADDRESS_ARR */


/* >>>RDD_CPU_RX_CFE_SRAM_COUNTERS_ADDRESS_ARR */
uint32_t RDD_CPU_RX_CFE_SRAM_COUNTERS_ADDRESS_ARR[NUM_OF_RUNNER_CORES] = {
	0xd80,
};
/* <<<RDD_CPU_RX_CFE_SRAM_COUNTERS_ADDRESS_ARR */


/* >>>RDD_CPU_TX_INGRESS_DISPATCHER_CREDIT_TABLE_ADDRESS_ARR */
uint32_t RDD_CPU_TX_INGRESS_DISPATCHER_CREDIT_TABLE_ADDRESS_ARR[NUM_OF_RUNNER_CORES] = {
	0xdd0,
};
/* <<<RDD_CPU_TX_INGRESS_DISPATCHER_CREDIT_TABLE_ADDRESS_ARR */


/* >>>RDD_TASK_IDX_ADDRESS_ARR */
uint32_t RDD_TASK_IDX_ADDRESS_ARR[NUM_OF_RUNNER_CORES] = {
	0xddc,
};
/* <<<RDD_TASK_IDX_ADDRESS_ARR */


/* >>>RDD_CPU_RX_INTERRUPT_SCRATCH_ADDRESS_ARR */
uint32_t RDD_CPU_RX_INTERRUPT_SCRATCH_ADDRESS_ARR[NUM_OF_RUNNER_CORES] = {
	0xde0,
};
/* <<<RDD_CPU_RX_INTERRUPT_SCRATCH_ADDRESS_ARR */


/* >>>RDD_CPU_RX_INTERRUPT_ID_DDR_ADDR_ADDRESS_ARR */
uint32_t RDD_CPU_RX_INTERRUPT_ID_DDR_ADDR_ADDRESS_ARR[NUM_OF_RUNNER_CORES] = {
	0xde8,
};
/* <<<RDD_CPU_RX_INTERRUPT_ID_DDR_ADDR_ADDRESS_ARR */


/* >>>RDD_PKT_BUFFER_ALLOC_MAP_TABLE_ADDRESS_ARR */
uint32_t RDD_PKT_BUFFER_ALLOC_MAP_TABLE_ADDRESS_ARR[NUM_OF_RUNNER_CORES] = {
	0xdf0,
};
/* <<<RDD_PKT_BUFFER_ALLOC_MAP_TABLE_ADDRESS_ARR */


/* >>>RDD_CPU_RX_LAST_READ_INDEX_ADDRESS_ARR */
uint32_t RDD_CPU_RX_LAST_READ_INDEX_ADDRESS_ARR[NUM_OF_RUNNER_CORES] = {
	0xdfc,
};
/* <<<RDD_CPU_RX_LAST_READ_INDEX_ADDRESS_ARR */


/* >>>RDD_SRAM_DUMMY_STORE_ADDRESS_ARR */
uint32_t RDD_SRAM_DUMMY_STORE_ADDRESS_ARR[NUM_OF_RUNNER_CORES] = {
	0xdfe,
};
/* <<<RDD_SRAM_DUMMY_STORE_ADDRESS_ARR */


/* >>>RDD_CPU_TX_CFE_SRAM_COUNTERS_ADDRESS_ARR */
uint32_t RDD_CPU_TX_CFE_SRAM_COUNTERS_ADDRESS_ARR[NUM_OF_RUNNER_CORES] = {
	0xe00,
};
/* <<<RDD_CPU_TX_CFE_SRAM_COUNTERS_ADDRESS_ARR */


/* >>>RDD_CPU_RX_BB_REPLY_ADDR_TABLE_ADDRESS_ARR */
uint32_t RDD_CPU_RX_BB_REPLY_ADDR_TABLE_ADDRESS_ARR[NUM_OF_RUNNER_CORES] = {
	0xe50,
};
/* <<<RDD_CPU_RX_BB_REPLY_ADDR_TABLE_ADDRESS_ARR */


/* >>>RDD_CPU_TX_BB_REPLY_ADDR_TABLE_ADDRESS_ARR */
uint32_t RDD_CPU_TX_BB_REPLY_ADDR_TABLE_ADDRESS_ARR[NUM_OF_RUNNER_CORES] = {
	0xe58,
};
/* <<<RDD_CPU_TX_BB_REPLY_ADDR_TABLE_ADDRESS_ARR */


/* >>>RDD_DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_ADDRESS_ARR */
uint32_t RDD_DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_ADDRESS_ARR[NUM_OF_RUNNER_CORES] = {
	0xe80,
};
/* <<<RDD_DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_ADDRESS_ARR */


/* >>>RDD_REGISTERS_BUFFER_ADDRESS_ARR */
uint32_t RDD_REGISTERS_BUFFER_ADDRESS_ARR[NUM_OF_RUNNER_CORES] = {
	0xec0,
};
/* <<<RDD_REGISTERS_BUFFER_ADDRESS_ARR */


/* >>>RDD_RX_FLOW_TABLE_ADDRESS_ARR */
uint32_t RDD_RX_FLOW_TABLE_ADDRESS_ARR[NUM_OF_RUNNER_CORES] = {
	0x1000,
};
/* <<<RDD_RX_FLOW_TABLE_ADDRESS_ARR */

