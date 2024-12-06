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


#include "bdmf_shell.h"
#include "rdd_map_auto.h"
#include "rdd_runner_reg_dump.h"
#include "rdd_runner_reg_dump_addrs.h"
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SQ_TM_PD_FIFO_TABLE =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_RX_SCRATCHPAD =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xa00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_SCRATCHPAD =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0xb68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SQ_TM_RATE_LIMITER_BUDGET_VALID =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0xb70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SQ_TM_RATE_LIMITER_PROFILE_TABLE =
{
	4,
	{
		{ dump_RDD_RATE_LIMITER_PROFILE, 0xb80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0xd68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_INTERRUPT_COALESCING_TABLE =
{
	16,
	{
		{ dump_RDD_CPU_INTERRUPT_COALESCING_ENTRY, 0xd70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_CACHE_TABLE =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0xd80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_RX_INTERRUPT_ID_DDR_ADDR =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0xf68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_DESCRIPTOR_TABLE =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0xf70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_RX_COPY_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT PROCESSING_7_TASKS_PACKET_BUFFER =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT NULL_BUFFER =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x1f68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_RX_AUX_INT_SCRATCHPAD =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1f70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_RXQ_SCRATCH_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1f80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x2168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, 0x2170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER =
{
	8,
	{
		{ dump_RDD_GENERAL_TIMER_ENTRY, 0x2180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE =
{
	8,
	{
		{ dump_RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY, 0x2368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_FLUSH_CFG_FW_TABLE =
{
	16,
	{
		{ dump_RDD_FLUSH_CFG_ENTRY, 0x2370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, 0x2380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_INT_INTERRUPT_SCRATCH =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x23fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_INDEX_DDR_ADDR_TABLE =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0x2568 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_FLUSH_CFG_CURRENT_TABLE =
{
	16,
	{
		{ dump_RDD_FLUSH_CFG_ENTRY, 0x2570 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_REASON_AND_VPORT_TO_METER_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2580 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SQ_TM_RATE_LIMITER_PROFILE_RESIDUE_TABLE =
{
	1,
	{
		{ dump_RDD_RATE_LIMITER_PROFILE_RESIDUE, 0x25e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SQ_TM_SCHEDULING_QUEUE_TABLE =
{
	10,
	{
		{ dump_RDD_SCHEDULING_QUEUE_DESCRIPTOR, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT VPORT_TX_FLOW_TABLE =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, 0x2740 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_RX_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x27d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_RSV_TABLE =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x27e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SQ_TM_RATE_LIMITER_BUDGET_DESCRIPTOR_TABLE =
{
	8,
	{
		{ dump_RDD_RATE_LIMITER_BUDGET_DESCRIPTOR, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SQ_TM_RATE_LIMITER_PARAMS_DESCRIPTOR_TABLE =
{
	8,
	{
		{ dump_RDD_RATE_LIMITER_PARAMS_DESCRIPTOR, 0x2900 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DSL_TX_FLOW_TABLE =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, 0x2b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_RX_METER_TABLE =
{
	8,
	{
		{ dump_RDD_CPU_RX_METER_ENTRY, 0x2c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_RING_INTERRUPT_COUNTER_TABLE =
{
	8,
	{
		{ dump_RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY, 0x2d00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_REASON_TO_METER_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2dc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_SRAM_PD_FIFO =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x2e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, 0x2f00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT AQM_SQ_BITMAP =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2fb4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, 0x2fb8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT IPV6_HOST_ADDRESS_CRC_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2fc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_RING_DESCRIPTORS_TABLE =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x3000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, 0x31c8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SQ_TM_SECONDARY_SCHEDULER_TABLE =
{
	12,
	{
		{ dump_RDD_SECONDARY_SCHEDULER_DESCRIPTOR, 0x31d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x31dc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT IPV4_HOST_ADDRESS_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x31e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_RX_COPY_PD_FIFO_TABLE =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x3200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT AQM_SQ_TABLE =
{
	2,
	{
		{ dump_RDD_AQM_SQ_ENTRY, 0x32c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CSO_CONTEXT_TABLE =
{
	160,
	{
		{ dump_RDD_CSO_CONTEXT_ENTRY, 0x3300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT AQM_ENABLE_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x33a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_TABLE =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, 0x33c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT AQM_NUM_QUEUES =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x33fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, 0x3400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SQ_TM_TX_QUEUE_DROP_TABLE =
{
	8,
	{
		{ dump_RDD_PACKETS_AND_BYTES, 0x34a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x35a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT HOST_TX_TRUNCATE_MIRRORING_TABLE =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, 0x35c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x35fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, 0x3600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SQ_TM_AQM_QUEUE_TABLE =
{
	4,
	{
		{ dump_RDD_AQM_QUEUE_DESCRIPTOR, 0x3680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_ACTION_VEC =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x37be },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, 0x37c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_FLUSH_CFG_ENABLE_TABLE =
{
	2,
	{
		{ dump_RDD_FLUSH_CFG_ENABLE_ENTRY, 0x37fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SQ_TM_PI2_PROBABILITY_CALC_DESCRIPTOR =
{
	52,
	{
		{ dump_RDD_PI2_PROBABILITY_CALC_DESCRIPTOR, 0x3800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3834 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CSO_BAD_IPV4_HDR_CSUM_PACKETS =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3838 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_RX_INTERRUPT_SCRATCH =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x383c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_TABLE =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, 0x3840 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SQ_TM_SCHEDULER_POOL =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3870 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SQ_TM_DISPATCHER_CREDIT_TABLE =
{
	12,
	{
		{ dump_RDD_DISPATCHER_CREDIT_DESCRIPTOR, 0x38b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT TASK_IDX =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x38bc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SQ_TM_UPDATE_FIFO_TABLE =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x38c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0x38e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT UPDATE_FIFO_TABLE =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x3900 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0x3920 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_RX_COPY_UPDATE_FIFO_TABLE =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x3940 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, 0x3960 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_RX_PD_FIFO_TABLE =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x3980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_REASON_TO_TC =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x39a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT TC_TO_CPU_RXQ =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x39e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT EXC_TC_TO_CPU_RXQ =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3a20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SQ_TM_BUFMNG_STATUS_TABLE =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, 0x3a60 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_RXQ_DATA_BUF_TYPE_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3a80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3aa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0x3ac0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3adf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SQ_TM_SCHEDULER_TABLE =
{
	24,
	{
		{ dump_RDD_SCHEDULER_DESCRIPTOR, 0x3ae0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, 0x3af8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_VPORT_TO_METER_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SCHEDULING_FLUSH_GLOBAL_CFG =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3b1f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SQ_TM_AQM_QUEUE_TIMER_TABLE =
{
	1,
	{
		{ dump_RDD_AQM_QUEUE_TIMER_DESCRIPTOR, 0x3b20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3b40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT LLQ_SELECTOR_ECN_MASK =
{
	1,
	{
		{ dump_RDD_LLQ_SELECTOR_ECN, 0x3b5f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3b60 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_AQM_TIMER_DISPATCHER_CREDIT_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3b90 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3b9c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SQ_TM_FIRST_QUEUE_MAPPING =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3b9e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, 0x3ba0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3bbc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT FORCE_DSCP =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3bbd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3bbe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_CACHE_OFFSET =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3bbf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3bc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_RX_COPY_DISPATCHER_CREDIT_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3bd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x3bdc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x3bde },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT BCM_SPDSVC_STREAM_RX_TS_TABLE =
{
	12,
	{
		{ dump_RDD_SPDSVC_RX_TS_STAT, 0x3be0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_REDIRECT_MODE =
{
	1,
	{
		{ dump_RDD_CPU_REDIRECT_MODE_ENTRY, 0x3bec },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CSO_DISABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3bed },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3bee },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, 0x3bef },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x3bf0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3bfc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x3bfd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3bfe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3bff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT VPORT_TO_CPU_OBJ =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3c20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_DESCRIPTOR_TABLE =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x3c30 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0x3c40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_INTERRUPT_COALESCING_TABLE =
{
	16,
	{
		{ dump_RDD_CPU_INTERRUPT_COALESCING_ENTRY, 0x3c50 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_INTERRUPT_SCRATCH =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3c60 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_RX_LOCAL_SCRATCH =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x3c68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x3c78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, 0x3c80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, 0x3c90 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3ca0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, 0x3ca8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_SHADOW_RD_IDX =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3cb0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_SHADOW_WR_IDX =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3cb8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, 0x3cc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3cc8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_NEXT_PTR_TABLE =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3cd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3cd8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, 0x3ce0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3ce8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT AQM_NUM_QUEUES_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_INT_INTERRUPT_SCRATCH_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_1 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0x8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_INTERRUPT_COALESCING_TABLE_1 =
{
	16,
	{
		{ dump_RDD_CPU_INTERRUPT_COALESCING_ENTRY, 0x10 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT AQM_ENABLE_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT VPORT_TX_FLOW_TABLE_1 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, 0x40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_1 =
{
	8,
	{
		{ dump_RDD_GENERAL_TIMER_ENTRY, 0x80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DSL_TX_FLOW_TABLE_1 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, 0x100 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_RX_INTERRUPT_ID_DDR_ADDR_1 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0x368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_1 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, 0x370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_1 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, 0x380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DDR_LATENCY_DBG_USEC_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_1 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT NULL_BUFFER_1 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT BUFFER_ALLOC_REPLY_1 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x770 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SG_DESC_TABLE_1 =
{
	48,
	{
		{ dump_RDD_SG_DESC_ENTRY, 0x780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x7e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_1 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE_1 =
{
	8,
	{
		{ dump_RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY, 0x9c8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x9d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DDR_LATENCY_DBG_USEC_MAX_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x9dc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_1 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0x9e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xa00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0xb68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_TX_RING_INDICES_VALUES_TABLE_1 =
{
	4,
	{
		{ dump_RDD_CPU_TX_RING_INDICES, 0xbe8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_TX_INGRESS_DISPATCHER_CREDIT_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0xbf0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0xbfc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_1 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, 0xd68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_1 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0xd70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0xd7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_TABLE_1 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, 0xd80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0xdbe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT HOST_TX_TRUNCATE_MIRRORING_TABLE_1 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, 0xdc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_ACTION_VEC_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0xdfe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_1 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, 0xf68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0xf70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CSO_BAD_IPV4_HDR_CSUM_PACKETS_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0xf7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0xfbe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_1 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, 0xfc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT LLQ_SELECTOR_ECN_MASK_1 =
{
	1,
	{
		{ dump_RDD_LLQ_SELECTOR_ECN, 0xffe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xfff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER_1 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_TX_SCRATCHPAD_1 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_TX_0_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2d00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_1 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, 0x2f68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_DESCRIPTOR_TABLE_1 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x2f70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_TABLE_1 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, 0x2f80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE_1 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0x2fb0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_1 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0x2fc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_1 =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, 0x2fe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_1 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x3168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_INTERRUPT_SCRATCH_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_1 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, 0x3178 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x317f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT RING_CPU_TX_DESCRIPTOR_DATA_TABLE_1 =
{
	16,
	{
		{ dump_RDD_RING_CPU_TX_DESCRIPTOR, 0x3180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x31a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_1 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0x31c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x31df },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_TX_RING_DESCRIPTOR_TABLE_1 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x31e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_TX_1_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT QM_QUEUE_TO_TX_FLOW_TABLE_PTR_TABLE_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_SRAM_PD_FIFO_1 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x3400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_1 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, 0x3500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT TASK_IDX_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x35b4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_SHADOW_RD_IDX_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x35b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_1 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x35ba },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_1 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x35bc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_REDIRECT_MODE_1 =
{
	1,
	{
		{ dump_RDD_CPU_REDIRECT_MODE_ENTRY, 0x35be },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CSO_DISABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x35bf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x35c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x35df },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_1 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, 0x35e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_1 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, 0x35fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x35fd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_1 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x35fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x35ff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_1 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, 0x3600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x36a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_1 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, 0x36d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_1 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, 0x36e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x36ec },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_SHADOW_WR_IDX_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x36f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x36f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SG_CONTEXT_TABLE_1 =
{
	64,
	{
		{ dump_RDD_SG_CONTEXT_ENTRY, 0x3700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_TX_SYNC_FIFO_TABLE_1 =
{
	8,
	{
		{ dump_RDD_CPU_TX_SYNC_FIFO_ENTRY, 0x3780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_NEXT_PTR_TABLE_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3790 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3798 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x37a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_1 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, 0x37a8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x37b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x37c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_1 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, 0x37e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_1 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, 0x3800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT QUEUE_THRESHOLD_VECTOR_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3880 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_FLOW_RING_CACHE_CTX_TABLE_2 =
{
	16,
	{
		{ dump_RDD_DHD_FLOW_RING_CACHE_CTX_ENTRY, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DSL_TX_FLOW_TABLE_2 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, 0x300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_AUX_INFO_CACHE_TABLE_2 =
{
	16,
	{
		{ dump_RDD_DHD_AUX_INFO_CACHE_ENTRY, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_DOORBELL_TX_POST_VALUE_2 =
{
	4,
	{
		{ dump_RDD_DHD_DOORBELL, 0x700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_POST_COMMON_RADIO_DATA_2 =
{
	200,
	{
		{ dump_RDD_DHD_POST_COMMON_RADIO_ENTRY, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT NULL_BUFFER_2 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0xa58 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT AQM_ENABLE_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0xa60 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_FPM_RINGS_AND_BUFMNG_REFILL_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xa80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT FPM_RING_CFG_TABLE_2 =
{
	24,
	{
		{ dump_RDD_FPM_RING_CFG_ENTRY, 0xb00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT VPORT_TX_FLOW_TABLE_2 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, 0xbc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_2 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_2 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, 0xf68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_2 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, 0xf70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_2 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0xffc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER_2 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT MIRRORING_SCRATCH_2 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x2168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_BACKUP_INDEX_CACHE_2 =
{
	32,
	{
		{ dump_RDD_DHD_BACKUP_IDX_CACHE_TABLE, 0x2180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x21e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_2 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0x2368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_2 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x2370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x237c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_L2_HEADER_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_2 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, 0x23c8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x23d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CSO_BAD_IPV4_HDR_CSUM_PACKETS_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x23dc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_2 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0x23e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2568 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_2 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, 0x25e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_CPU_INT_ID_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x25f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT TASK_IDX_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x25fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_2 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, 0x2768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_MIRRORING_DISPATCHER_CREDIT_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2770 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT AQM_NUM_QUEUES_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x277c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x277e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_TABLE_2 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, 0x2780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x27be },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT HOST_TX_TRUNCATE_MIRRORING_TABLE_2 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, 0x27c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SPDSVC_WLAN_TXPOST_PARAMS_TABLE_2 =
{
	2,
	{
		{ dump_RDD_SPDSVC_WLAN_TXPOST_PARAMS, 0x27fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_2 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x2968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_2 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, 0x2970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x29be },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT LLQ_SELECTOR_ECN_MASK_2 =
{
	1,
	{
		{ dump_RDD_LLQ_SELECTOR_ECN, 0x29bf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_2 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, 0x29c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x29fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x29ff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_2 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, 0x2b68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2b6f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_2 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, 0x2b70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_2 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x2b7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_2 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x2b7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_TABLE_2 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, 0x2b80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2bb0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_REDIRECT_MODE_2 =
{
	1,
	{
		{ dump_RDD_CPU_REDIRECT_MODE_ENTRY, 0x2bb4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CSO_DISABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2bb5 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2bb6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_2 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, 0x2bb7 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2bb8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2bba },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_2 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x2bbb },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2bbc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2bbd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_CODEL_BIAS_SLOPE_TABLE_2 =
{
	4,
	{
		{ dump_RDD_DHD_CODEL_BIAS_SLOPE, 0x2bc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_2 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, 0x2bf0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2bf8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_CPU_TX_POST_RING_DESCRIPTOR_TABLE_2 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x2d80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_UPDATE_FIFO_TABLE_2 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x2dc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_2 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0x2de0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_2 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, 0x2e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_2 =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, 0x2ec0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_TIMER_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2ee0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_2 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, 0x2f00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_UPDATE_FIFO_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2fa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_2 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0x2fc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_2 =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, 0x2fe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_FLOW_RING_BUFFER_2 =
{
	48,
	{
		{ dump_RDD_DHD_TX_POST_DESCRIPTOR, 0x3000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3090 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x30c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_2 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, 0x30e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_0_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3100 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x31a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x31c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_2 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, 0x31e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_1_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_2_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_2 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, 0x3400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE_2 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x3480 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_PD_FIFO_TABLE_2 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x3500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_FLOW_RING_CACHE_LKP_TABLE_2 =
{
	2,
	{
		{ dump_RDD_DHD_FLOW_RING_CACHE_LKP_ENTRY, 0x3600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT US_TM_PD_FIFO_TABLE_3 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT US_TM_SCHEDULING_QUEUE_TABLE_3 =
{
	10,
	{
		{ dump_RDD_SCHEDULING_QUEUE_DESCRIPTOR, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT US_TM_RATE_LIMITER_PROFILE_TABLE_3 =
{
	4,
	{
		{ dump_RDD_RATE_LIMITER_PROFILE, 0xa80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT TX_TASK_US_0_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xb00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT US_TM_SCHEDULER_TABLE_3 =
{
	24,
	{
		{ dump_RDD_SCHEDULER_DESCRIPTOR, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT US_TM_TM_FLOW_CNTR_TABLE_3 =
{
	1,
	{
		{ dump_RDD_TM_FLOW_CNTR_ENTRY, 0xe40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_3 =
{
	8,
	{
		{ dump_RDD_GENERAL_TIMER_ENTRY, 0xe80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT TX_TASK_US_1_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xf00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT PROCESSING_4_TASKS_PACKET_BUFFER_3 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_COMPLETE_COMMON_RADIO_DATA_3 =
{
	192,
	{
		{ dump_RDD_DHD_COMPLETE_COMMON_RADIO_ENTRY, 0x1800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT VPORT_TX_FLOW_TABLE_3 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, 0x1a40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_3 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, 0x1a80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1afc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT US_TM_DSL_TM_FLOW_CNTR_TABLE_3 =
{
	1,
	{
		{ dump_RDD_TM_FLOW_CNTR_ENTRY, 0x1b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT US_TM_RATE_LIMITER_BUDGET_DESCRIPTOR_TABLE_3 =
{
	8,
	{
		{ dump_RDD_RATE_LIMITER_BUDGET_DESCRIPTOR, 0x1c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT US_TM_RATE_LIMITER_PARAMS_DESCRIPTOR_TABLE_3 =
{
	8,
	{
		{ dump_RDD_RATE_LIMITER_PARAMS_DESCRIPTOR, 0x1e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DSL_TM_PD_FIFO_TABLE_3 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT NULL_BUFFER_3 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x2368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT US_TM_RATE_LIMITER_BUDGET_VALID_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_FLOW_RING_BUFFER_3 =
{
	32,
	{
		{ dump_RDD_DHD_RX_COMPLETE_DESCRIPTOR, 0x2380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT US_TM_RATE_LIMITER_PROFILE_RESIDUE_TABLE_3 =
{
	1,
	{
		{ dump_RDD_RATE_LIMITER_PROFILE_RESIDUE, 0x23e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_3 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_3 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x2768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT US_TM_BBH_TX_WAKE_UP_DATA_TABLE_3 =
{
	8,
	{
		{ dump_RDD_BBH_TX_WAKE_UP_DATA_ENTRY, 0x2770 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_RX_POST_FLOW_RING_BUFFER_3 =
{
	32,
	{
		{ dump_RDD_DHD_RX_POST_DESCRIPTOR, 0x2780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT AQM_ENABLE_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x27e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_3 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0x2968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_3 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, 0x2970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT MIRRORING_SCRATCH_3 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x29c8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT US_TM_FLUSH_CFG_CPU_TABLE_3 =
{
	16,
	{
		{ dump_RDD_FLUSH_CFG_ENTRY, 0x29d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x29e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_RX_POST_RING_SIZE_3 =
{
	2,
	{
		{ dump_RDD_DHD_RING_SIZE, 0x2b68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT AQM_NUM_QUEUES_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2b6e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT US_TM_FLUSH_CFG_FW_TABLE_3 =
{
	16,
	{
		{ dump_RDD_FLUSH_CFG_ENTRY, 0x2b70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT US_BUFFER_CONG_MGT_CFG_3 =
{
	68,
	{
		{ dump_RDD_BUFFER_CONG_MGT, 0x2b80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2bc4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_RING_SIZE_3 =
{
	2,
	{
		{ dump_RDD_DHD_RING_SIZE, 0x2bc8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT BUDGET_ALLOCATION_TIMER_VALUE_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2bce },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT US_TM_FLUSH_CFG_CURRENT_TABLE_3 =
{
	16,
	{
		{ dump_RDD_FLUSH_CFG_ENTRY, 0x2bd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_3 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0x2be0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DSL_TX_FLOW_TABLE_3 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, 0x2c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT US_TM_TX_QUEUE_DROP_TABLE_3 =
{
	8,
	{
		{ dump_RDD_PACKETS_AND_BYTES, 0x2d00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT US_TM_AQM_QUEUE_TABLE_3 =
{
	4,
	{
		{ dump_RDD_AQM_QUEUE_DESCRIPTOR, 0x2f00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT US_TM_BBH_QUEUE_TABLE_3 =
{
	8,
	{
		{ dump_RDD_BBH_QUEUE_DESCRIPTOR, 0x3000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT US_TM_AQM_QUEUE_TIMER_TABLE_3 =
{
	1,
	{
		{ dump_RDD_AQM_QUEUE_TIMER_DESCRIPTOR, 0x30c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_3 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, 0x3100 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CSO_BAD_IPV4_HDR_CSUM_PACKETS_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x31b4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_RING_SIZE_3 =
{
	2,
	{
		{ dump_RDD_DHD_RING_SIZE, 0x31b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x31be },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT UPDATE_FIFO_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x31c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_3 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, 0x3200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_3 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0x32a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_0_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x32c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT US_TM_SECONDARY_SCHEDULER_TABLE_3 =
{
	12,
	{
		{ dump_RDD_SECONDARY_SCHEDULER_DESCRIPTOR, 0x3300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT TASK_IDX_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3384 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_3 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, 0x3388 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DIRECT_FLOW_PD_TABLE_3 =
{
	16,
	{
		{ dump_RDD_PROCESSING_RX_DESCRIPTOR, 0x3390 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_3 =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, 0x33a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_1_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x33c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_3 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, 0x3400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT US_TM_SCHEDULER_POOL_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3480 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_2_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3580 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_0_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x35c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_1_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_2_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3640 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT BUFFER_CONG_SCRATCHPAD_3 =
{
	8,
	{
		{ dump_RDD_BUFFER_CONG_Q_OCCUPANCY, 0x3680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_TABLE_3 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, 0x36c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_ACTION_VEC_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x36fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT HOST_TX_TRUNCATE_MIRRORING_TABLE_3 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, 0x3700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT US_TM_FLUSH_CFG_ENABLE_TABLE_3 =
{
	2,
	{
		{ dump_RDD_FLUSH_CFG_ENABLE_ENTRY, 0x373e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT PON_TM_TX_TRUNCATE_MIRRORING_TABLE_3 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, 0x3740 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x377e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x37be },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DIRECT_FLOW_WAN_VIQ_EXCLUSIVE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x37bf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_3 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, 0x37c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT TX_EXCEPTION_3 =
{
	1,
	{
		{ dump_RDD_TX_EXCEPTION_ENTRY, 0x37fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT LLQ_SELECTOR_ECN_MASK_3 =
{
	1,
	{
		{ dump_RDD_LLQ_SELECTOR_ECN, 0x37ff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT US_TM_PI2_PROBABILITY_CALC_DESCRIPTOR_3 =
{
	52,
	{
		{ dump_RDD_PI2_PROBABILITY_CALC_DESCRIPTOR, 0x3800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT BBH_TX_US_WAN_0_FIFO_BYTES_THRESHOLD_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3834 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3836 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3837 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_3 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, 0x3838 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_TABLE_3 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, 0x3840 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3870 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE_3 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x3880 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT US_TM_DISPATCHER_CREDIT_TABLE_3 =
{
	12,
	{
		{ dump_RDD_DISPATCHER_CREDIT_DESCRIPTOR, 0x38b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT US_TM_FIRST_QUEUE_MAPPING_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x38bc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x38be },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_REDIRECT_MODE_3 =
{
	1,
	{
		{ dump_RDD_CPU_REDIRECT_MODE_ENTRY, 0x38bf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_FLOW_RING_BUFFER_3 =
{
	16,
	{
		{ dump_RDD_DHD_TX_COMPLETE_DESCRIPTOR, 0x38c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT US_TM_FLUSH_DISPATCHER_CREDIT_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x38f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_3 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x38fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_3 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x38fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_DISPATCHER_CREDIT_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3900 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_3 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x3930 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CSO_DISABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x393c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x393d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_3 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, 0x393e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT WAN_0_BBH_TX_FIFO_SIZE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x393f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_DISPATCHER_CREDIT_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3940 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x397c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT MAC_TYPE_3 =
{
	1,
	{
		{ dump_RDD_MAC_TYPE_ENTRY, 0x397d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_3 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x397e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x397f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CODEL_BIAS_SLOPE_TABLE_3 =
{
	4,
	{
		{ dump_RDD_CODEL_BIAS_SLOPE, 0x3980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x39ac },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_DOORBELL_TX_COMPLETE_VALUE_3 =
{
	4,
	{
		{ dump_RDD_DHD_DOORBELL, 0x39b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT US_TM_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_TABLE_3 =
{
	40,
	{
		{ dump_RDD_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_ENTRY, 0x39c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_DOORBELL_RX_COMPLETE_VALUE_3 =
{
	4,
	{
		{ dump_RDD_DHD_DOORBELL, 0x3a10 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT BUFFER_CONG_DQM_NOT_EMPTY_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3a20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT UPDATE_FIFO_TABLE_3 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x3a40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT US_TM_TX_OCTETS_COUNTERS_TABLE_3 =
{
	8,
	{
		{ dump_RDD_PACKETS_AND_BYTES, 0x3a60 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT US_TM_SCHEDULING_AGGREGATION_CONTEXT_VECTOR_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3aa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_3 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, 0x3ab8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_3 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0x3ac0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT US_TM_CODEL_DROP_DESCRIPTOR_3 =
{
	20,
	{
		{ dump_RDD_CODEL_DROP_DESCRIPTOR, 0x3ae0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_3 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x3af8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_3 =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, 0x3b20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_3 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, 0x3b38 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT US_TM_BBH_TX_EGRESS_COUNTER_TABLE_3 =
{
	8,
	{
		{ dump_RDD_BBH_TX_EGRESS_COUNTER_ENTRY, 0x3b40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3b58 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3b88 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_DOORBELL_RX_POST_VALUE_3 =
{
	4,
	{
		{ dump_RDD_DHD_DOORBELL, 0x3b90 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_3 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, 0x3ba0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_FPM_REPLY_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3bc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3bd8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DHD_CPU_INT_ID_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3be0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_DISPATCHER_CREDIT_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3bf0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT US_TM_DSL_BBH_TX_ABS_COUNTER_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SPDSVC_WLAN_GEN_PARAMS_TABLE_3 =
{
	10,
	{
		{ dump_RDD_SPDSVC_WLAN_GEN_PARAMS, 0x3c10 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_3 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, 0x3c20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_3 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, 0x3c30 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3c40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_3 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, 0x3c50 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3c58 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3c60 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT US_TM_BBH_TX_ABS_COUNTER_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3c80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_3 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, 0x3ca0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DS_TM_PD_FIFO_TABLE_4 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_STREAM_TABLE_4 =
{
	376,
	{
		{ dump_RDD_TCPSPDTEST_STREAM, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DS_TM_RATE_LIMITER_PROFILE_RESIDUE_TABLE_4 =
{
	1,
	{
		{ dump_RDD_RATE_LIMITER_PROFILE_RESIDUE, 0xde0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DS_TM_RATE_LIMITER_BUDGET_DESCRIPTOR_TABLE_4 =
{
	8,
	{
		{ dump_RDD_RATE_LIMITER_BUDGET_DESCRIPTOR, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DS_TM_SCHEDULING_QUEUE_TABLE_4 =
{
	10,
	{
		{ dump_RDD_SCHEDULING_QUEUE_DESCRIPTOR, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DS_TM_RATE_LIMITER_PROFILE_TABLE_4 =
{
	4,
	{
		{ dump_RDD_RATE_LIMITER_PROFILE, 0x1280 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT TX_TASK_DS_0_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT PKTGEN_TX_STREAM_TABLE_4 =
{
	136,
	{
		{ dump_RDD_PKTGEN_TX_STREAM_ENTRY, 0x1400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT AQM_ENABLE_TABLE_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1620 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DS_TM_TM_FLOW_CNTR_TABLE_4 =
{
	1,
	{
		{ dump_RDD_TM_FLOW_CNTR_ENTRY, 0x1640 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_SCRATCHPAD_4 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x1680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DS_TM_AQM_QUEUE_TABLE_4 =
{
	4,
	{
		{ dump_RDD_AQM_QUEUE_DESCRIPTOR, 0x1700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DS_TM_RATE_LIMITER_PARAMS_DESCRIPTOR_TABLE_4 =
{
	8,
	{
		{ dump_RDD_RATE_LIMITER_PARAMS_DESCRIPTOR, 0x1800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DS_TM_SECONDARY_SCHEDULER_TABLE_4 =
{
	12,
	{
		{ dump_RDD_SECONDARY_SCHEDULER_DESCRIPTOR, 0x1a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CPU_RX_INTERRUPT_ID_DDR_ADDR_4 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0x1b08 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DS_TM_RATE_LIMITER_BUDGET_VALID_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1b10 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1b20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DS_TM_AQM_QUEUE_TIMER_TABLE_4 =
{
	1,
	{
		{ dump_RDD_AQM_QUEUE_TIMER_DESCRIPTOR, 0x1b40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_4 =
{
	8,
	{
		{ dump_RDD_GENERAL_TIMER_ENTRY, 0x1b80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_4 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, 0x1c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DS_TM_TX_QUEUE_DROP_TABLE_4 =
{
	8,
	{
		{ dump_RDD_PACKETS_AND_BYTES, 0x1e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SPDSVC_ANALYZER_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DS_TM_SCHEDULER_TABLE_4 =
{
	24,
	{
		{ dump_RDD_SCHEDULER_DESCRIPTOR, 0x2100 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT UPDATE_FIFO_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x21c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT UDPSPDT_STREAM_RX_STAT_TABLE_4 =
{
	48,
	{
		{ dump_RDD_UDPSPDT_STREAM_RX_STAT, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT COMMON_REPROCESSING_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x22c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SPDSVC_GEN_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_4 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x23c8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DS_TM_BBH_TX_WAKE_UP_DATA_TABLE_4 =
{
	8,
	{
		{ dump_RDD_BBH_TX_WAKE_UP_DATA_ENTRY, 0x23d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_4 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0x23e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT UDPSPDT_STREAM_TX_STAT_TABLE_4 =
{
	40,
	{
		{ dump_RDD_UDPSPDT_STREAM_TX_STAT, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT BUFFER_CONG_DQM_NOT_EMPTY_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x24a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT BUFFER_CONG_SCRATCHPAD_4 =
{
	8,
	{
		{ dump_RDD_BUFFER_CONG_Q_OCCUPANCY, 0x24c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT UDPSPDT_TX_PARAMS_TABLE_4 =
{
	24,
	{
		{ dump_RDD_UDPSPDT_TX_PARAMS, 0x2500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_ENGINE_GLOBAL_TABLE_4 =
{
	20,
	{
		{ dump_RDD_TCPSPDTEST_ENGINE_GLOBAL_INFO, 0x2560 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT PKTGEN_NO_SBPM_HDRS_CNTR_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2574 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT TX_ABS_RECYCLE_COUNTERS_4 =
{
	8,
	{
		{ dump_RDD_TX_ABS_RECYCLE_COUNTERS_ENTRY, 0x2578 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DS_BUFFER_CONG_MGT_CFG_4 =
{
	68,
	{
		{ dump_RDD_BUFFER_CONG_MGT, 0x2580 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x25c4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DS_TM_SCHEDULER_POOL_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x25c8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2648 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT MIRRORING_SCRATCH_4 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x2678 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT COMMON_REPROCESSING_PD_FIFO_TABLE_4 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x2680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT HOST_TX_TRUNCATE_MIRRORING_TABLE_4 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, 0x26c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT AQM_NUM_QUEUES_4 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x26fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SPDSVC_ANALYZER_PD_FIFO_TABLE_4 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x2700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT ETH_TM_TX_TRUNCATE_MIRRORING_TABLE_4 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, 0x2740 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT BUDGET_ALLOCATION_TIMER_VALUE_4 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x277e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_PD_FIFO_TABLE_4 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x2780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SPDSVC_GEN_PARAMS_TABLE_4 =
{
	60,
	{
		{ dump_RDD_SPDSVC_GEN_PARAMS, 0x27c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x27fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DS_TM_BBH_QUEUE_TABLE_4 =
{
	8,
	{
		{ dump_RDD_BBH_QUEUE_DESCRIPTOR, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DS_TM_PI2_PROBABILITY_CALC_DESCRIPTOR_4 =
{
	52,
	{
		{ dump_RDD_PI2_PROBABILITY_CALC_DESCRIPTOR, 0x2840 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT PKTGEN_BAD_GET_NEXT_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2874 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT PKTGEN_CURR_SBPM_HDR_PTR_4 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2878 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT PKTGEN_SBPM_HDR_BNS_4 =
{
	2,
	{
		{ dump_RDD_PKTGEN_SBPM_HDR_BN, 0x2880 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x28c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CODEL_BIAS_SLOPE_TABLE_4 =
{
	4,
	{
		{ dump_RDD_CODEL_BIAS_SLOPE, 0x2940 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT PKTGEN_MAX_UT_PKTS_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x296c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_4 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, 0x2970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DS_TM_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_TABLE_4 =
{
	40,
	{
		{ dump_RDD_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_ENTRY, 0x2980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DS_TM_FLUSH_CFG_CPU_TABLE_4 =
{
	16,
	{
		{ dump_RDD_FLUSH_CFG_ENTRY, 0x29d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DS_TM_SCHEDULING_AGGREGATION_CONTEXT_VECTOR_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x29e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT PKTGEN_UT_TRIGGER_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x29f4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT PKTGEN_NUM_OF_AVAIL_SBPM_HDRS_4 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x29f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT UPDATE_FIFO_TABLE_4 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x2a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DS_TM_CODEL_DROP_DESCRIPTOR_4 =
{
	20,
	{
		{ dump_RDD_CODEL_DROP_DESCRIPTOR, 0x2a20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT TASK_IDX_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2a34 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT PKTGEN_SBPM_END_PTR_4 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2a38 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT COMMON_REPROCESSING_UPDATE_FIFO_TABLE_4 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x2a40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DS_TM_FLUSH_CFG_FW_TABLE_4 =
{
	16,
	{
		{ dump_RDD_FLUSH_CFG_ENTRY, 0x2a60 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DS_TM_FLUSH_CFG_CURRENT_TABLE_4 =
{
	16,
	{
		{ dump_RDD_FLUSH_CFG_ENTRY, 0x2a70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SPDSVC_ANALYZER_UPDATE_FIFO_TABLE_4 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x2a80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SPDSVC_ANALYZER_PARAMS_TABLE_4 =
{
	16,
	{
		{ dump_RDD_SPDSVC_ANALYZER_PARAMS, 0x2aa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DS_TM_FLUSH_DISPATCHER_CREDIT_TABLE_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2ab0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_ACTION_VEC_4 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2abc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DS_TM_FLUSH_CFG_ENABLE_TABLE_4 =
{
	2,
	{
		{ dump_RDD_FLUSH_CFG_ENABLE_ENTRY, 0x2abe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_UPDATE_FIFO_TABLE_4 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x2ac0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_4 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, 0x2ae0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT TX_EXCEPTION_4 =
{
	1,
	{
		{ dump_RDD_TX_EXCEPTION_ENTRY, 0x2afc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2afd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DS_TM_FIRST_QUEUE_MAPPING_4 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2afe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT PKTGEN_SESSION_DATA_4 =
{
	32,
	{
		{ dump_RDD_PKTGEN_TX_PARAMS, 0x2b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SPDSVC_GEN_DISPATCHER_CREDIT_TABLE_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2b20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SPDSVC_TCPSPDTEST_COMMON_TABLE_4 =
{
	1,
	{
		{ dump_RDD_SPDSVC_TCPSPDTEST_COMMON_ENTRY, 0x2b2c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2b2d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_4 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x2b2e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_DISPATCHER_CREDIT_TABLE_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2b30 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_4 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x2b3c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2b3e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2b3f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_4 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0x2b40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT COMMON_REPROCESSING_DISPATCHER_CREDIT_TABLE_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2b60 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_GEN_DISPATCHER_CREDIT_TABLE_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2b70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT PKTGEN_FPM_UG_MGMT_4 =
{
	20,
	{
		{ dump_RDD_PKTGEN_FPM_UG_MGMT_ENTRY, 0x2b80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_4 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x2ba0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_DISPATCHER_CREDIT_TABLE_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2bb0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_4 =
{
	8,
	{
		{ dump_RDD_BBH_TX_EGRESS_COUNTER_ENTRY, 0x2bc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT UDPSPDT_SCRATCH_TABLE_4 =
{
	8,
	{
		{ dump_RDD_UDPSPDT_SCRATCH_IPERF3_RX, 0x2bd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT PKTGEN_SBPM_EXTS_4 =
{
	4,
	{
		{ dump_RDD_PKTGEN_SBPM_EXT, 0x2bd8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT PKTGEN_BBMSG_REPLY_SCRATCH_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2bf0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DS_TM_BBH_TX_ABS_COUNTER_TABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_4 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, 0x2c10 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2c20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63146
static DUMP_RUNNERREG_STRUCT PKTGEN_TX_STREAM_SCRATCH_TABLE_4 =
{
	8,
	{
		{ dump_RDD_PKTGEN_TX_STREAM_SCRATCH_ENTRY, 0x2c80 },
		{ 0, 0 },
	}
};
#endif

TABLE_STRUCT RUNNER_TABLES[NUMBER_OF_TABLES] =
{
#if defined BCM63146
	{ "SQ_TM_PD_FIFO_TABLE", 1, CORE_0_INDEX, &SQ_TM_PD_FIFO_TABLE, 64, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_RX_SCRATCHPAD", 1, CORE_0_INDEX, &CPU_RX_SCRATCHPAD, 128, 1, 1 },
#endif
#if defined BCM63146
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_0_INDEX, &RUNNER_PROFILING_TRACE_BUFFER, 128, 1, 1 },
#endif
#if defined BCM63146
	{ "PROCESSING0_STACK", 1, CORE_0_INDEX, &PROCESSING0_STACK, 360, 1, 1 },
#endif
#if defined BCM63146
	{ "SERVICE_QUEUES_SCRATCHPAD", 1, CORE_0_INDEX, &SERVICE_QUEUES_SCRATCHPAD, 2, 1, 1 },
#endif
#if defined BCM63146
	{ "SQ_TM_RATE_LIMITER_BUDGET_VALID", 1, CORE_0_INDEX, &SQ_TM_RATE_LIMITER_BUDGET_VALID, 4, 1, 1 },
#endif
#if defined BCM63146
	{ "SQ_TM_RATE_LIMITER_PROFILE_TABLE", 1, CORE_0_INDEX, &SQ_TM_RATE_LIMITER_PROFILE_TABLE, 32, 1, 1 },
#endif
#if defined BCM63146
	{ "PROCESSING1_STACK", 1, CORE_0_INDEX, &PROCESSING1_STACK, 360, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR", 1, CORE_0_INDEX, &CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_RECYCLE_INTERRUPT_COALESCING_TABLE", 1, CORE_0_INDEX, &CPU_RECYCLE_INTERRUPT_COALESCING_TABLE, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_FEED_RING_CACHE_TABLE", 1, CORE_0_INDEX, &CPU_FEED_RING_CACHE_TABLE, 16, 1, 1 },
#endif
#if defined BCM63146
	{ "PROCESSING2_STACK", 1, CORE_0_INDEX, &PROCESSING2_STACK, 360, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_RX_INTERRUPT_ID_DDR_ADDR", 1, CORE_0_INDEX, &CPU_RX_INTERRUPT_ID_DDR_ADDR, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_FEED_RING_DESCRIPTOR_TABLE", 1, CORE_0_INDEX, &CPU_FEED_RING_DESCRIPTOR_TABLE, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_RX_COPY_STACK", 1, CORE_0_INDEX, &CPU_RX_COPY_STACK, 128, 1, 1 },
#endif
#if defined BCM63146
	{ "PROCESSING_7_TASKS_PACKET_BUFFER", 1, CORE_0_INDEX, &PROCESSING_7_TASKS_PACKET_BUFFER, 7, 1, 1 },
#endif
#if defined BCM63146
	{ "PROCESSING3_STACK", 1, CORE_0_INDEX, &PROCESSING3_STACK, 360, 1, 1 },
#endif
#if defined BCM63146
	{ "NULL_BUFFER", 1, CORE_0_INDEX, &NULL_BUFFER, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_RX_AUX_INT_SCRATCHPAD", 1, CORE_0_INDEX, &CPU_RX_AUX_INT_SCRATCHPAD, 4, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_RXQ_SCRATCH_TABLE", 1, CORE_0_INDEX, &CPU_RXQ_SCRATCH_TABLE, 128, 1, 1 },
#endif
#if defined BCM63146
	{ "PROCESSING4_STACK", 1, CORE_0_INDEX, &PROCESSING4_STACK, 360, 1, 1 },
#endif
#if defined BCM63146
	{ "SYSTEM_CONFIGURATION", 1, CORE_0_INDEX, &SYSTEM_CONFIGURATION, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "SPDTEST_GEN_PARAM", 1, CORE_0_INDEX, &SPDTEST_GEN_PARAM, 4, 1, 1 },
#endif
#if defined BCM63146
	{ "GENERAL_TIMER", 1, CORE_0_INDEX, &GENERAL_TIMER, 16, 1, 1 },
#endif
#if defined BCM63146
	{ "PROCESSING5_STACK", 1, CORE_0_INDEX, &PROCESSING5_STACK, 360, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE", 1, CORE_0_INDEX, &CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "SERVICE_QUEUES_FLUSH_CFG_FW_TABLE", 1, CORE_0_INDEX, &SERVICE_QUEUES_FLUSH_CFG_FW_TABLE, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "VPORT_CFG_TABLE", 1, CORE_0_INDEX, &VPORT_CFG_TABLE, 31, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_INT_INTERRUPT_SCRATCH", 1, CORE_0_INDEX, &CPU_INT_INTERRUPT_SCRATCH, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "PROCESSING6_STACK", 1, CORE_0_INDEX, &PROCESSING6_STACK, 360, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_FEED_RING_INDEX_DDR_ADDR_TABLE", 1, CORE_0_INDEX, &CPU_FEED_RING_INDEX_DDR_ADDR_TABLE, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "SERVICE_QUEUES_FLUSH_CFG_CURRENT_TABLE", 1, CORE_0_INDEX, &SERVICE_QUEUES_FLUSH_CFG_CURRENT_TABLE, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_REASON_AND_VPORT_TO_METER_TABLE", 1, CORE_0_INDEX, &CPU_REASON_AND_VPORT_TO_METER_TABLE, 96, 1, 1 },
#endif
#if defined BCM63146
	{ "SQ_TM_RATE_LIMITER_PROFILE_RESIDUE_TABLE", 1, CORE_0_INDEX, &SQ_TM_RATE_LIMITER_PROFILE_RESIDUE_TABLE, 32, 1, 1 },
#endif
#if defined BCM63146
	{ "SQ_TM_SCHEDULING_QUEUE_TABLE", 1, CORE_0_INDEX, &SQ_TM_SCHEDULING_QUEUE_TABLE, 32, 1, 1 },
#endif
#if defined BCM63146
	{ "VPORT_TX_FLOW_TABLE", 1, CORE_0_INDEX, &VPORT_TX_FLOW_TABLE, 64, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_RX_STACK", 1, CORE_0_INDEX, &CPU_RX_STACK, 80, 1, 1 },
#endif
#if defined BCM63146
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_0_INDEX, &INGRESS_FILTER_L2_REASON_TABLE, 16, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_FEED_RING_RSV_TABLE", 1, CORE_0_INDEX, &CPU_FEED_RING_RSV_TABLE, 16, 1, 1 },
#endif
#if defined BCM63146
	{ "SQ_TM_RATE_LIMITER_BUDGET_DESCRIPTOR_TABLE", 1, CORE_0_INDEX, &SQ_TM_RATE_LIMITER_BUDGET_DESCRIPTOR_TABLE, 32, 1, 1 },
#endif
#if defined BCM63146
	{ "SQ_TM_RATE_LIMITER_PARAMS_DESCRIPTOR_TABLE", 1, CORE_0_INDEX, &SQ_TM_RATE_LIMITER_PARAMS_DESCRIPTOR_TABLE, 32, 1, 1 },
#endif
#if defined BCM63146
	{ "SERVICE_QUEUES_STACK", 1, CORE_0_INDEX, &SERVICE_QUEUES_STACK, 256, 1, 1 },
#endif
#if defined BCM63146
	{ "DSL_TX_FLOW_TABLE", 1, CORE_0_INDEX, &DSL_TX_FLOW_TABLE, 256, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_RX_METER_TABLE", 1, CORE_0_INDEX, &CPU_RX_METER_TABLE, 32, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_RING_INTERRUPT_COUNTER_TABLE", 1, CORE_0_INDEX, &CPU_RING_INTERRUPT_COUNTER_TABLE, 24, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_REASON_TO_METER_TABLE", 1, CORE_0_INDEX, &CPU_REASON_TO_METER_TABLE, 64, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_RECYCLE_SRAM_PD_FIFO", 1, CORE_0_INDEX, &CPU_RECYCLE_SRAM_PD_FIFO, 16, 1, 1 },
#endif
#if defined BCM63146
	{ "RX_FLOW_TABLE", 1, CORE_0_INDEX, &RX_FLOW_TABLE, 90, 1, 1 },
#endif
#if defined BCM63146
	{ "AQM_SQ_BITMAP", 1, CORE_0_INDEX, &AQM_SQ_BITMAP, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "TCAM_GENERIC_FIELDS", 1, CORE_0_INDEX, &TCAM_GENERIC_FIELDS, 4, 1, 1 },
#endif
#if defined BCM63146
	{ "IPV6_HOST_ADDRESS_CRC_TABLE", 1, CORE_0_INDEX, &IPV6_HOST_ADDRESS_CRC_TABLE, 16, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_RING_DESCRIPTORS_TABLE", 1, CORE_0_INDEX, &CPU_RING_DESCRIPTORS_TABLE, 24, 1, 1 },
#endif
#if defined BCM63146
	{ "GENERAL_TIMER_STACK", 1, CORE_0_INDEX, &GENERAL_TIMER_STACK, 72, 1, 1 },
#endif
#if defined BCM63146
	{ "TUNNELS_PARSING_CFG", 1, CORE_0_INDEX, &TUNNELS_PARSING_CFG, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "SQ_TM_SECONDARY_SCHEDULER_TABLE", 1, CORE_0_INDEX, &SQ_TM_SECONDARY_SCHEDULER_TABLE, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "MUTEX_TABLE", 1, CORE_0_INDEX, &MUTEX_TABLE, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "IPV4_HOST_ADDRESS_TABLE", 1, CORE_0_INDEX, &IPV4_HOST_ADDRESS_TABLE, 8, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_RX_COPY_PD_FIFO_TABLE", 1, CORE_0_INDEX, &CPU_RX_COPY_PD_FIFO_TABLE, 12, 1, 1 },
#endif
#if defined BCM63146
	{ "AQM_SQ_TABLE", 1, CORE_0_INDEX, &AQM_SQ_TABLE, 32, 1, 1 },
#endif
#if defined BCM63146
	{ "CSO_CONTEXT_TABLE", 1, CORE_0_INDEX, &CSO_CONTEXT_TABLE, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "AQM_ENABLE_TABLE", 1, CORE_0_INDEX, &AQM_ENABLE_TABLE, 8, 1, 1 },
#endif
#if defined BCM63146
	{ "RX_MIRRORING_TABLE", 1, CORE_0_INDEX, &RX_MIRRORING_TABLE, 31, 1, 1 },
#endif
#if defined BCM63146
	{ "AQM_NUM_QUEUES", 1, CORE_0_INDEX, &AQM_NUM_QUEUES, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "POLICER_PARAMS_TABLE", 1, CORE_0_INDEX, &POLICER_PARAMS_TABLE, 80, 1, 1 },
#endif
#if defined BCM63146
	{ "SQ_TM_TX_QUEUE_DROP_TABLE", 1, CORE_0_INDEX, &SQ_TM_TX_QUEUE_DROP_TABLE, 32, 1, 1 },
#endif
#if defined BCM63146
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_0_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT, 8, 1, 1 },
#endif
#if defined BCM63146
	{ "HOST_TX_TRUNCATE_MIRRORING_TABLE", 1, CORE_0_INDEX, &HOST_TX_TRUNCATE_MIRRORING_TABLE, 31, 1, 1 },
#endif
#if defined BCM63146
	{ "DOS_DROP_REASONS_CFG", 1, CORE_0_INDEX, &DOS_DROP_REASONS_CFG, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_0_INDEX, &INGRESS_FILTER_PROFILE_TABLE, 16, 1, 1 },
#endif
#if defined BCM63146
	{ "SQ_TM_AQM_QUEUE_TABLE", 1, CORE_0_INDEX, &SQ_TM_AQM_QUEUE_TABLE, 32, 1, 1 },
#endif
#if defined BCM63146
	{ "REGISTERS_BUFFER", 1, CORE_0_INDEX, &REGISTERS_BUFFER, 32, 1, 1 },
#endif
#if defined BCM63146
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_0_INDEX, &LOOPBACK_QUEUE_TABLE, 31, 1, 1 },
#endif
#if defined BCM63146
	{ "GENERAL_TIMER_ACTION_VEC", 1, CORE_0_INDEX, &GENERAL_TIMER_ACTION_VEC, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "VPORT_CFG_EX_TABLE", 1, CORE_0_INDEX, &VPORT_CFG_EX_TABLE, 31, 1, 1 },
#endif
#if defined BCM63146
	{ "SERVICE_QUEUES_FLUSH_CFG_ENABLE_TABLE", 1, CORE_0_INDEX, &SERVICE_QUEUES_FLUSH_CFG_ENABLE_TABLE, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "SQ_TM_PI2_PROBABILITY_CALC_DESCRIPTOR", 1, CORE_0_INDEX, &SQ_TM_PI2_PROBABILITY_CALC_DESCRIPTOR, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_0_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "CSO_BAD_IPV4_HDR_CSUM_PACKETS", 1, CORE_0_INDEX, &CSO_BAD_IPV4_HDR_CSUM_PACKETS, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_RX_INTERRUPT_SCRATCH", 1, CORE_0_INDEX, &CPU_RX_INTERRUPT_SCRATCH, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "TR471_SPDSVC_RX_PKT_ID_TABLE", 1, CORE_0_INDEX, &TR471_SPDSVC_RX_PKT_ID_TABLE, 4, 1, 1 },
#endif
#if defined BCM63146
	{ "SQ_TM_SCHEDULER_POOL", 1, CORE_0_INDEX, &SQ_TM_SCHEDULER_POOL, 64, 1, 1 },
#endif
#if defined BCM63146
	{ "SQ_TM_DISPATCHER_CREDIT_TABLE", 1, CORE_0_INDEX, &SQ_TM_DISPATCHER_CREDIT_TABLE, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "TASK_IDX", 1, CORE_0_INDEX, &TASK_IDX, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "SQ_TM_UPDATE_FIFO_TABLE", 1, CORE_0_INDEX, &SQ_TM_UPDATE_FIFO_TABLE, 8, 1, 1 },
#endif
#if defined BCM63146
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_0_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE, 32, 1, 1 },
#endif
#if defined BCM63146
	{ "UPDATE_FIFO_TABLE", 1, CORE_0_INDEX, &UPDATE_FIFO_TABLE, 8, 1, 1 },
#endif
#if defined BCM63146
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_0_INDEX, &BUFMNG_DESCRIPTOR_TABLE, 16, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_RX_COPY_UPDATE_FIFO_TABLE", 1, CORE_0_INDEX, &CPU_RX_COPY_UPDATE_FIFO_TABLE, 8, 1, 1 },
#endif
#if defined BCM63146
	{ "BUFMNG_STATUS_TABLE", 1, CORE_0_INDEX, &BUFMNG_STATUS_TABLE, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_RX_PD_FIFO_TABLE", 1, CORE_0_INDEX, &CPU_RX_PD_FIFO_TABLE, 2, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_REASON_TO_TC", 1, CORE_0_INDEX, &CPU_REASON_TO_TC, 64, 1, 1 },
#endif
#if defined BCM63146
	{ "TC_TO_CPU_RXQ", 1, CORE_0_INDEX, &TC_TO_CPU_RXQ, 64, 1, 1 },
#endif
#if defined BCM63146
	{ "EXC_TC_TO_CPU_RXQ", 1, CORE_0_INDEX, &EXC_TC_TO_CPU_RXQ, 64, 1, 1 },
#endif
#if defined BCM63146
	{ "SQ_TM_BUFMNG_STATUS_TABLE", 1, CORE_0_INDEX, &SQ_TM_BUFMNG_STATUS_TABLE, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_RXQ_DATA_BUF_TYPE_TABLE", 1, CORE_0_INDEX, &CPU_RXQ_DATA_BUF_TYPE_TABLE, 32, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_RECYCLE_STACK", 1, CORE_0_INDEX, &CPU_RECYCLE_STACK, 32, 1, 1 },
#endif
#if defined BCM63146
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_0_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE, 31, 1, 1 },
#endif
#if defined BCM63146
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_0_INDEX, &SPDTEST_NUM_OF_RX_FLOWS, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "SQ_TM_SCHEDULER_TABLE", 1, CORE_0_INDEX, &SQ_TM_SCHEDULER_TABLE, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "IPTV_CFG_TABLE", 1, CORE_0_INDEX, &IPTV_CFG_TABLE, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_VPORT_TO_METER_TABLE", 1, CORE_0_INDEX, &CPU_VPORT_TO_METER_TABLE, 31, 1, 1 },
#endif
#if defined BCM63146
	{ "SCHEDULING_FLUSH_GLOBAL_CFG", 1, CORE_0_INDEX, &SCHEDULING_FLUSH_GLOBAL_CFG, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "SQ_TM_AQM_QUEUE_TIMER_TABLE", 1, CORE_0_INDEX, &SQ_TM_AQM_QUEUE_TIMER_TABLE, 32, 1, 1 },
#endif
#if defined BCM63146
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_0_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE, 31, 1, 1 },
#endif
#if defined BCM63146
	{ "LLQ_SELECTOR_ECN_MASK", 1, CORE_0_INDEX, &LLQ_SELECTOR_ECN_MASK, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "DEBUG_SCRATCHPAD", 1, CORE_0_INDEX, &DEBUG_SCRATCHPAD, 12, 1, 1 },
#endif
#if defined BCM63146
	{ "SERVICE_QUEUES_AQM_TIMER_DISPATCHER_CREDIT_TABLE", 1, CORE_0_INDEX, &SERVICE_QUEUES_AQM_TIMER_DISPATCHER_CREDIT_TABLE, 3, 1, 1 },
#endif
#if defined BCM63146
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_0_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "SQ_TM_FIRST_QUEUE_MAPPING", 1, CORE_0_INDEX, &SQ_TM_FIRST_QUEUE_MAPPING, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "FPM_GLOBAL_CFG", 1, CORE_0_INDEX, &FPM_GLOBAL_CFG, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_0_INDEX, &LOOPBACK_WAN_FLOW_TABLE, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "FORCE_DSCP", 1, CORE_0_INDEX, &FORCE_DSCP, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "CORE_ID_TABLE", 1, CORE_0_INDEX, &CORE_ID_TABLE, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_FEED_RING_CACHE_OFFSET", 1, CORE_0_INDEX, &CPU_FEED_RING_CACHE_OFFSET, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_0_INDEX, &BITS_CALC_MASKS_TABLE, 4, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_RX_COPY_DISPATCHER_CREDIT_TABLE", 1, CORE_0_INDEX, &CPU_RX_COPY_DISPATCHER_CREDIT_TABLE, 3, 1, 1 },
#endif
#if defined BCM63146
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_0_INDEX, &RX_MIRRORING_CONFIGURATION, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_0_INDEX, &TX_MIRRORING_CONFIGURATION, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "BCM_SPDSVC_STREAM_RX_TS_TABLE", 1, CORE_0_INDEX, &BCM_SPDSVC_STREAM_RX_TS_TABLE, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_REDIRECT_MODE", 1, CORE_0_INDEX, &CPU_REDIRECT_MODE, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "CSO_DISABLE", 1, CORE_0_INDEX, &CSO_DISABLE, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_0_INDEX, &DEBUG_PRINT_CORE_LOCK, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_0_INDEX, &TCAM_TABLE_CFG_TABLE, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_0_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "SRAM_DUMMY_STORE", 1, CORE_0_INDEX, &SRAM_DUMMY_STORE, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_0_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_0_INDEX, &INGRESS_FILTER_1588_CFG, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_0_INDEX, &RX_MIRRORING_DIRECT_ENABLE, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "VPORT_TO_CPU_OBJ", 1, CORE_0_INDEX, &VPORT_TO_CPU_OBJ, 31, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_FPM_THRESHOLDS", 1, CORE_0_INDEX, &DHD_FPM_THRESHOLDS, 3, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_RECYCLE_RING_DESCRIPTOR_TABLE", 1, CORE_0_INDEX, &CPU_RECYCLE_RING_DESCRIPTOR_TABLE, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE", 1, CORE_0_INDEX, &CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE, 2, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_INTERRUPT_COALESCING_TABLE", 1, CORE_0_INDEX, &CPU_INTERRUPT_COALESCING_TABLE, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_RECYCLE_INTERRUPT_SCRATCH", 1, CORE_0_INDEX, &CPU_RECYCLE_INTERRUPT_SCRATCH, 2, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_RX_LOCAL_SCRATCH", 1, CORE_0_INDEX, &CPU_RX_LOCAL_SCRATCH, 2, 1, 1 },
#endif
#if defined BCM63146
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_0_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "DEBUG_PRINT_TABLE", 1, CORE_0_INDEX, &DEBUG_PRINT_TABLE, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "GDX_PARAMS_TABLE", 1, CORE_0_INDEX, &GDX_PARAMS_TABLE, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_0_INDEX, &FW_ERROR_VECTOR_TABLE, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "NAT_CACHE_CFG", 1, CORE_0_INDEX, &NAT_CACHE_CFG, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_RECYCLE_SHADOW_RD_IDX", 1, CORE_0_INDEX, &CPU_RECYCLE_SHADOW_RD_IDX, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_RECYCLE_SHADOW_WR_IDX", 1, CORE_0_INDEX, &CPU_RECYCLE_SHADOW_WR_IDX, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "MULTICAST_KEY_MASK", 1, CORE_0_INDEX, &MULTICAST_KEY_MASK, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_0_INDEX, &NAT_CACHE_KEY0_MASK, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_RECYCLE_NEXT_PTR_TABLE", 1, CORE_0_INDEX, &CPU_RECYCLE_NEXT_PTR_TABLE, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_0_INDEX, &NATC_L2_VLAN_KEY_MASK, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "INGRESS_FILTER_CFG", 1, CORE_0_INDEX, &INGRESS_FILTER_CFG, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "NATC_L2_TOS_MASK", 1, CORE_0_INDEX, &NATC_L2_TOS_MASK, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_1_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "AQM_NUM_QUEUES", 1, CORE_1_INDEX, &AQM_NUM_QUEUES_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_INT_INTERRUPT_SCRATCH", 1, CORE_1_INDEX, &CPU_INT_INTERRUPT_SCRATCH_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR", 1, CORE_1_INDEX, &CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_RECYCLE_INTERRUPT_COALESCING_TABLE", 1, CORE_1_INDEX, &CPU_RECYCLE_INTERRUPT_COALESCING_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "AQM_ENABLE_TABLE", 1, CORE_1_INDEX, &AQM_ENABLE_TABLE_1, 8, 1, 1 },
#endif
#if defined BCM63146
	{ "VPORT_TX_FLOW_TABLE", 1, CORE_1_INDEX, &VPORT_TX_FLOW_TABLE_1, 64, 1, 1 },
#endif
#if defined BCM63146
	{ "GENERAL_TIMER", 1, CORE_1_INDEX, &GENERAL_TIMER_1, 16, 1, 1 },
#endif
#if defined BCM63146
	{ "DSL_TX_FLOW_TABLE", 1, CORE_1_INDEX, &DSL_TX_FLOW_TABLE_1, 256, 1, 1 },
#endif
#if defined BCM63146
	{ "PROCESSING0_STACK", 1, CORE_1_INDEX, &PROCESSING0_STACK_1, 360, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_RX_INTERRUPT_ID_DDR_ADDR", 1, CORE_1_INDEX, &CPU_RX_INTERRUPT_ID_DDR_ADDR_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "SPDTEST_GEN_PARAM", 1, CORE_1_INDEX, &SPDTEST_GEN_PARAM_1, 4, 1, 1 },
#endif
#if defined BCM63146
	{ "VPORT_CFG_TABLE", 1, CORE_1_INDEX, &VPORT_CFG_TABLE_1, 31, 1, 1 },
#endif
#if defined BCM63146
	{ "DDR_LATENCY_DBG_USEC", 1, CORE_1_INDEX, &DDR_LATENCY_DBG_USEC_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_1_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_1, 128, 1, 1 },
#endif
#if defined BCM63146
	{ "PROCESSING1_STACK", 1, CORE_1_INDEX, &PROCESSING1_STACK_1, 360, 1, 1 },
#endif
#if defined BCM63146
	{ "NULL_BUFFER", 1, CORE_1_INDEX, &NULL_BUFFER_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "BUFFER_ALLOC_REPLY", 1, CORE_1_INDEX, &BUFFER_ALLOC_REPLY_1, 2, 1, 1 },
#endif
#if defined BCM63146
	{ "SG_DESC_TABLE", 1, CORE_1_INDEX, &SG_DESC_TABLE_1, 2, 1, 1 },
#endif
#if defined BCM63146
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_1_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_1, 8, 1, 1 },
#endif
#if defined BCM63146
	{ "PROCESSING2_STACK", 1, CORE_1_INDEX, &PROCESSING2_STACK_1, 360, 1, 1 },
#endif
#if defined BCM63146
	{ "SYSTEM_CONFIGURATION", 1, CORE_1_INDEX, &SYSTEM_CONFIGURATION_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_1_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM63146
	{ "GENERAL_TIMER_STACK", 1, CORE_1_INDEX, &GENERAL_TIMER_STACK_1, 72, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE", 1, CORE_1_INDEX, &CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE", 1, CORE_1_INDEX, &CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE_1, 3, 1, 1 },
#endif
#if defined BCM63146
	{ "DDR_LATENCY_DBG_USEC_MAX", 1, CORE_1_INDEX, &DDR_LATENCY_DBG_USEC_MAX_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_1_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_1, 32, 1, 1 },
#endif
#if defined BCM63146
	{ "PROCESSING3_STACK", 1, CORE_1_INDEX, &PROCESSING3_STACK_1, 360, 1, 1 },
#endif
#if defined BCM63146
	{ "REGISTERS_BUFFER", 1, CORE_1_INDEX, &REGISTERS_BUFFER_1, 32, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_TX_RING_INDICES_VALUES_TABLE", 1, CORE_1_INDEX, &CPU_TX_RING_INDICES_VALUES_TABLE_1, 2, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_TX_INGRESS_DISPATCHER_CREDIT_TABLE", 1, CORE_1_INDEX, &CPU_TX_INGRESS_DISPATCHER_CREDIT_TABLE_1, 3, 1, 1 },
#endif
#if defined BCM63146
	{ "MUTEX_TABLE", 1, CORE_1_INDEX, &MUTEX_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "PROCESSING4_STACK", 1, CORE_1_INDEX, &PROCESSING4_STACK_1, 360, 1, 1 },
#endif
#if defined BCM63146
	{ "TCAM_GENERIC_FIELDS", 1, CORE_1_INDEX, &TCAM_GENERIC_FIELDS_1, 4, 1, 1 },
#endif
#if defined BCM63146
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_1_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_1_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "RX_MIRRORING_TABLE", 1, CORE_1_INDEX, &RX_MIRRORING_TABLE_1, 31, 1, 1 },
#endif
#if defined BCM63146
	{ "DOS_DROP_REASONS_CFG", 1, CORE_1_INDEX, &DOS_DROP_REASONS_CFG_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "HOST_TX_TRUNCATE_MIRRORING_TABLE", 1, CORE_1_INDEX, &HOST_TX_TRUNCATE_MIRRORING_TABLE_1, 31, 1, 1 },
#endif
#if defined BCM63146
	{ "GENERAL_TIMER_ACTION_VEC", 1, CORE_1_INDEX, &GENERAL_TIMER_ACTION_VEC_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "PROCESSING5_STACK", 1, CORE_1_INDEX, &PROCESSING5_STACK_1, 360, 1, 1 },
#endif
#if defined BCM63146
	{ "TUNNELS_PARSING_CFG", 1, CORE_1_INDEX, &TUNNELS_PARSING_CFG_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_FPM_THRESHOLDS", 1, CORE_1_INDEX, &DHD_FPM_THRESHOLDS_1, 3, 1, 1 },
#endif
#if defined BCM63146
	{ "CSO_BAD_IPV4_HDR_CSUM_PACKETS", 1, CORE_1_INDEX, &CSO_BAD_IPV4_HDR_CSUM_PACKETS_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_1_INDEX, &LOOPBACK_QUEUE_TABLE_1, 31, 1, 1 },
#endif
#if defined BCM63146
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_1_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "VPORT_CFG_EX_TABLE", 1, CORE_1_INDEX, &VPORT_CFG_EX_TABLE_1, 31, 1, 1 },
#endif
#if defined BCM63146
	{ "LLQ_SELECTOR_ECN_MASK", 1, CORE_1_INDEX, &LLQ_SELECTOR_ECN_MASK_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_1_INDEX, &LOOPBACK_WAN_FLOW_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_1_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER_1, 8, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_TX_SCRATCHPAD", 1, CORE_1_INDEX, &CPU_TX_SCRATCHPAD_1, 416, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_TX_0_STACK", 1, CORE_1_INDEX, &CPU_TX_0_STACK_1, 256, 1, 1 },
#endif
#if defined BCM63146
	{ "PROCESSING6_STACK", 1, CORE_1_INDEX, &PROCESSING6_STACK_1, 360, 1, 1 },
#endif
#if defined BCM63146
	{ "IPTV_CFG_TABLE", 1, CORE_1_INDEX, &IPTV_CFG_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_RECYCLE_RING_DESCRIPTOR_TABLE", 1, CORE_1_INDEX, &CPU_RECYCLE_RING_DESCRIPTOR_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "TR471_SPDSVC_RX_PKT_ID_TABLE", 1, CORE_1_INDEX, &TR471_SPDSVC_RX_PKT_ID_TABLE_1, 4, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE", 1, CORE_1_INDEX, &CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE_1, 2, 1, 1 },
#endif
#if defined BCM63146
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_1_INDEX, &BUFMNG_DESCRIPTOR_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM63146
	{ "BUFMNG_STATUS_TABLE", 1, CORE_1_INDEX, &BUFMNG_STATUS_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "PROCESSING7_STACK", 1, CORE_1_INDEX, &PROCESSING7_STACK_1, 360, 1, 1 },
#endif
#if defined BCM63146
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_1_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_RECYCLE_INTERRUPT_SCRATCH", 1, CORE_1_INDEX, &CPU_RECYCLE_INTERRUPT_SCRATCH_1, 2, 1, 1 },
#endif
#if defined BCM63146
	{ "NAT_CACHE_CFG", 1, CORE_1_INDEX, &NAT_CACHE_CFG_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "FORCE_DSCP", 1, CORE_1_INDEX, &FORCE_DSCP_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "RING_CPU_TX_DESCRIPTOR_DATA_TABLE", 1, CORE_1_INDEX, &RING_CPU_TX_DESCRIPTOR_DATA_TABLE_1, 2, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_RECYCLE_STACK", 1, CORE_1_INDEX, &CPU_RECYCLE_STACK_1, 32, 1, 1 },
#endif
#if defined BCM63146
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_1_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_1, 31, 1, 1 },
#endif
#if defined BCM63146
	{ "CORE_ID_TABLE", 1, CORE_1_INDEX, &CORE_ID_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_TX_RING_DESCRIPTOR_TABLE", 1, CORE_1_INDEX, &CPU_TX_RING_DESCRIPTOR_TABLE_1, 2, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_TX_1_STACK", 1, CORE_1_INDEX, &CPU_TX_1_STACK_1, 256, 1, 1 },
#endif
#if defined BCM63146
	{ "QM_QUEUE_TO_TX_FLOW_TABLE_PTR_TABLE", 1, CORE_1_INDEX, &QM_QUEUE_TO_TX_FLOW_TABLE_PTR_TABLE_1, 128, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_RECYCLE_SRAM_PD_FIFO", 1, CORE_1_INDEX, &CPU_RECYCLE_SRAM_PD_FIFO_1, 16, 1, 1 },
#endif
#if defined BCM63146
	{ "RX_FLOW_TABLE", 1, CORE_1_INDEX, &RX_FLOW_TABLE_1, 90, 1, 1 },
#endif
#if defined BCM63146
	{ "TASK_IDX", 1, CORE_1_INDEX, &TASK_IDX_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_RECYCLE_SHADOW_RD_IDX", 1, CORE_1_INDEX, &CPU_RECYCLE_SHADOW_RD_IDX_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_1_INDEX, &RX_MIRRORING_CONFIGURATION_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_1_INDEX, &TX_MIRRORING_CONFIGURATION_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_REDIRECT_MODE", 1, CORE_1_INDEX, &CPU_REDIRECT_MODE_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "CSO_DISABLE", 1, CORE_1_INDEX, &CSO_DISABLE_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_1_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_1, 31, 1, 1 },
#endif
#if defined BCM63146
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_1_INDEX, &DEBUG_PRINT_CORE_LOCK_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "FPM_GLOBAL_CFG", 1, CORE_1_INDEX, &FPM_GLOBAL_CFG_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_1_INDEX, &TCAM_TABLE_CFG_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "SRAM_DUMMY_STORE", 1, CORE_1_INDEX, &SRAM_DUMMY_STORE_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_1_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_1_INDEX, &INGRESS_FILTER_1588_CFG_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "POLICER_PARAMS_TABLE", 1, CORE_1_INDEX, &POLICER_PARAMS_TABLE_1, 80, 1, 1 },
#endif
#if defined BCM63146
	{ "DEBUG_SCRATCHPAD", 1, CORE_1_INDEX, &DEBUG_SCRATCHPAD_1, 12, 1, 1 },
#endif
#if defined BCM63146
	{ "DEBUG_PRINT_TABLE", 1, CORE_1_INDEX, &DEBUG_PRINT_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "GDX_PARAMS_TABLE", 1, CORE_1_INDEX, &GDX_PARAMS_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_1_INDEX, &RX_MIRRORING_DIRECT_ENABLE_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_RECYCLE_SHADOW_WR_IDX", 1, CORE_1_INDEX, &CPU_RECYCLE_SHADOW_WR_IDX_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_1_INDEX, &NAT_CACHE_KEY0_MASK_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "SG_CONTEXT_TABLE", 1, CORE_1_INDEX, &SG_CONTEXT_TABLE_1, 2, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_TX_SYNC_FIFO_TABLE", 1, CORE_1_INDEX, &CPU_TX_SYNC_FIFO_TABLE_1, 2, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_RECYCLE_NEXT_PTR_TABLE", 1, CORE_1_INDEX, &CPU_RECYCLE_NEXT_PTR_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_1_INDEX, &NATC_L2_VLAN_KEY_MASK_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_1_INDEX, &FW_ERROR_VECTOR_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "INGRESS_FILTER_CFG", 1, CORE_1_INDEX, &INGRESS_FILTER_CFG_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "NATC_L2_TOS_MASK", 1, CORE_1_INDEX, &NATC_L2_TOS_MASK_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_1_INDEX, &BITS_CALC_MASKS_TABLE_1, 4, 1, 1 },
#endif
#if defined BCM63146
	{ "MULTICAST_KEY_MASK", 1, CORE_1_INDEX, &MULTICAST_KEY_MASK_1, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_1_INDEX, &INGRESS_FILTER_PROFILE_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM63146
	{ "QUEUE_THRESHOLD_VECTOR", 1, CORE_1_INDEX, &QUEUE_THRESHOLD_VECTOR_1, 4, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_FLOW_RING_CACHE_CTX_TABLE", 1, CORE_2_INDEX, &DHD_FLOW_RING_CACHE_CTX_TABLE_2, 48, 1, 1 },
#endif
#if defined BCM63146
	{ "DSL_TX_FLOW_TABLE", 1, CORE_2_INDEX, &DSL_TX_FLOW_TABLE_2, 256, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_AUX_INFO_CACHE_TABLE", 1, CORE_2_INDEX, &DHD_AUX_INFO_CACHE_TABLE_2, 48, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_DOORBELL_TX_POST_VALUE", 1, CORE_2_INDEX, &DHD_DOORBELL_TX_POST_VALUE_2, 64, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_POST_COMMON_RADIO_DATA", 1, CORE_2_INDEX, &DHD_POST_COMMON_RADIO_DATA_2, 3, 1, 1 },
#endif
#if defined BCM63146
	{ "NULL_BUFFER", 1, CORE_2_INDEX, &NULL_BUFFER_2, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "AQM_ENABLE_TABLE", 1, CORE_2_INDEX, &AQM_ENABLE_TABLE_2, 8, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_FPM_RINGS_AND_BUFMNG_REFILL_STACK", 1, CORE_2_INDEX, &CPU_FPM_RINGS_AND_BUFMNG_REFILL_STACK_2, 128, 1, 1 },
#endif
#if defined BCM63146
	{ "FPM_RING_CFG_TABLE", 1, CORE_2_INDEX, &FPM_RING_CFG_TABLE_2, 8, 1, 1 },
#endif
#if defined BCM63146
	{ "VPORT_TX_FLOW_TABLE", 1, CORE_2_INDEX, &VPORT_TX_FLOW_TABLE_2, 64, 1, 1 },
#endif
#if defined BCM63146
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_2_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_2, 128, 1, 1 },
#endif
#if defined BCM63146
	{ "PROCESSING0_STACK", 1, CORE_2_INDEX, &PROCESSING0_STACK_2, 360, 1, 1 },
#endif
#if defined BCM63146
	{ "SYSTEM_CONFIGURATION", 1, CORE_2_INDEX, &SYSTEM_CONFIGURATION_2, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "SPDTEST_GEN_PARAM", 1, CORE_2_INDEX, &SPDTEST_GEN_PARAM_2, 4, 1, 1 },
#endif
#if defined BCM63146
	{ "VPORT_CFG_TABLE", 1, CORE_2_INDEX, &VPORT_CFG_TABLE_2, 31, 1, 1 },
#endif
#if defined BCM63146
	{ "MUTEX_TABLE", 1, CORE_2_INDEX, &MUTEX_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_2_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER_2, 8, 1, 1 },
#endif
#if defined BCM63146
	{ "PROCESSING1_STACK", 1, CORE_2_INDEX, &PROCESSING1_STACK_2, 360, 1, 1 },
#endif
#if defined BCM63146
	{ "MIRRORING_SCRATCH", 1, CORE_2_INDEX, &MIRRORING_SCRATCH_2, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_2_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_2, 16, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_BACKUP_INDEX_CACHE", 1, CORE_2_INDEX, &DHD_BACKUP_INDEX_CACHE_2, 3, 1, 1 },
#endif
#if defined BCM63146
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_2_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_2, 8, 1, 1 },
#endif
#if defined BCM63146
	{ "PROCESSING2_STACK", 1, CORE_2_INDEX, &PROCESSING2_STACK_2, 360, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR", 1, CORE_2_INDEX, &DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_2, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_2_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_2_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_L2_HEADER", 1, CORE_2_INDEX, &DHD_L2_HEADER_2, 72, 1, 1 },
#endif
#if defined BCM63146
	{ "TCAM_GENERIC_FIELDS", 1, CORE_2_INDEX, &TCAM_GENERIC_FIELDS_2, 4, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_FPM_THRESHOLDS", 1, CORE_2_INDEX, &DHD_FPM_THRESHOLDS_2, 3, 1, 1 },
#endif
#if defined BCM63146
	{ "CSO_BAD_IPV4_HDR_CSUM_PACKETS", 1, CORE_2_INDEX, &CSO_BAD_IPV4_HDR_CSUM_PACKETS_2, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_2_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_2, 32, 1, 1 },
#endif
#if defined BCM63146
	{ "PROCESSING3_STACK", 1, CORE_2_INDEX, &PROCESSING3_STACK_2, 360, 1, 1 },
#endif
#if defined BCM63146
	{ "REGISTERS_BUFFER", 1, CORE_2_INDEX, &REGISTERS_BUFFER_2, 32, 1, 1 },
#endif
#if defined BCM63146
	{ "TUNNELS_PARSING_CFG", 1, CORE_2_INDEX, &TUNNELS_PARSING_CFG_2, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_CPU_INT_ID", 1, CORE_2_INDEX, &DHD_CPU_INT_ID_2, 3, 1, 1 },
#endif
#if defined BCM63146
	{ "TASK_IDX", 1, CORE_2_INDEX, &TASK_IDX_2, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "PROCESSING4_STACK", 1, CORE_2_INDEX, &PROCESSING4_STACK_2, 360, 1, 1 },
#endif
#if defined BCM63146
	{ "IPTV_CFG_TABLE", 1, CORE_2_INDEX, &IPTV_CFG_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_MIRRORING_DISPATCHER_CREDIT_TABLE", 1, CORE_2_INDEX, &DHD_MIRRORING_DISPATCHER_CREDIT_TABLE_2, 3, 1, 1 },
#endif
#if defined BCM63146
	{ "AQM_NUM_QUEUES", 1, CORE_2_INDEX, &AQM_NUM_QUEUES_2, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "DOS_DROP_REASONS_CFG", 1, CORE_2_INDEX, &DOS_DROP_REASONS_CFG_2, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "RX_MIRRORING_TABLE", 1, CORE_2_INDEX, &RX_MIRRORING_TABLE_2, 31, 1, 1 },
#endif
#if defined BCM63146
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_2_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "HOST_TX_TRUNCATE_MIRRORING_TABLE", 1, CORE_2_INDEX, &HOST_TX_TRUNCATE_MIRRORING_TABLE_2, 31, 1, 1 },
#endif
#if defined BCM63146
	{ "SPDSVC_WLAN_TXPOST_PARAMS_TABLE", 1, CORE_2_INDEX, &SPDSVC_WLAN_TXPOST_PARAMS_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "PROCESSING5_STACK", 1, CORE_2_INDEX, &PROCESSING5_STACK_2, 360, 1, 1 },
#endif
#if defined BCM63146
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_2_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_2, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "DEBUG_PRINT_TABLE", 1, CORE_2_INDEX, &DEBUG_PRINT_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_2_INDEX, &LOOPBACK_QUEUE_TABLE_2, 31, 1, 1 },
#endif
#if defined BCM63146
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_2_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_2, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "LLQ_SELECTOR_ECN_MASK", 1, CORE_2_INDEX, &LLQ_SELECTOR_ECN_MASK_2, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "VPORT_CFG_EX_TABLE", 1, CORE_2_INDEX, &VPORT_CFG_EX_TABLE_2, 31, 1, 1 },
#endif
#if defined BCM63146
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_2_INDEX, &LOOPBACK_WAN_FLOW_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "FORCE_DSCP", 1, CORE_2_INDEX, &FORCE_DSCP_2, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "PROCESSING6_STACK", 1, CORE_2_INDEX, &PROCESSING6_STACK_2, 360, 1, 1 },
#endif
#if defined BCM63146
	{ "NAT_CACHE_CFG", 1, CORE_2_INDEX, &NAT_CACHE_CFG_2, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "CORE_ID_TABLE", 1, CORE_2_INDEX, &CORE_ID_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "GDX_PARAMS_TABLE", 1, CORE_2_INDEX, &GDX_PARAMS_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_2_INDEX, &RX_MIRRORING_CONFIGURATION_2, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_2_INDEX, &TX_MIRRORING_CONFIGURATION_2, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "TR471_SPDSVC_RX_PKT_ID_TABLE", 1, CORE_2_INDEX, &TR471_SPDSVC_RX_PKT_ID_TABLE_2, 4, 1, 1 },
#endif
#if defined BCM63146
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_2_INDEX, &NAT_CACHE_KEY0_MASK_2, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_REDIRECT_MODE", 1, CORE_2_INDEX, &CPU_REDIRECT_MODE_2, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "CSO_DISABLE", 1, CORE_2_INDEX, &CSO_DISABLE_2, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_2_INDEX, &DEBUG_PRINT_CORE_LOCK_2, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_2_INDEX, &TCAM_TABLE_CFG_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_2_INDEX, &NATC_L2_VLAN_KEY_MASK_2, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "SRAM_DUMMY_STORE", 1, CORE_2_INDEX, &SRAM_DUMMY_STORE_2, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_2_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_2_INDEX, &INGRESS_FILTER_1588_CFG_2, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_2_INDEX, &RX_MIRRORING_DIRECT_ENABLE_2, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_CODEL_BIAS_SLOPE_TABLE", 1, CORE_2_INDEX, &DHD_CODEL_BIAS_SLOPE_TABLE_2, 11, 1, 1 },
#endif
#if defined BCM63146
	{ "INGRESS_FILTER_CFG", 1, CORE_2_INDEX, &INGRESS_FILTER_CFG_2, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "NATC_L2_TOS_MASK", 1, CORE_2_INDEX, &NATC_L2_TOS_MASK_2, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "PROCESSING7_STACK", 1, CORE_2_INDEX, &PROCESSING7_STACK_2, 360, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_CPU_TX_POST_RING_DESCRIPTOR_TABLE", 1, CORE_2_INDEX, &DHD_CPU_TX_POST_RING_DESCRIPTOR_TABLE_2, 3, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_TX_POST_UPDATE_FIFO_TABLE", 1, CORE_2_INDEX, &DHD_TX_POST_UPDATE_FIFO_TABLE_2, 8, 1, 1 },
#endif
#if defined BCM63146
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_2_INDEX, &BUFMNG_DESCRIPTOR_TABLE_2, 16, 1, 1 },
#endif
#if defined BCM63146
	{ "RX_FLOW_TABLE", 1, CORE_2_INDEX, &RX_FLOW_TABLE_2, 90, 1, 1 },
#endif
#if defined BCM63146
	{ "BUFMNG_STATUS_TABLE", 1, CORE_2_INDEX, &BUFMNG_STATUS_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_TIMER_STACK", 1, CORE_2_INDEX, &DHD_TIMER_STACK_2, 32, 1, 1 },
#endif
#if defined BCM63146
	{ "POLICER_PARAMS_TABLE", 1, CORE_2_INDEX, &POLICER_PARAMS_TABLE_2, 80, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_TX_POST_UPDATE_FIFO_STACK", 1, CORE_2_INDEX, &DHD_TX_POST_UPDATE_FIFO_STACK_2, 32, 1, 1 },
#endif
#if defined BCM63146
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_2_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_2, 31, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_HW_CFG", 1, CORE_2_INDEX, &DHD_HW_CFG_2, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_TX_POST_FLOW_RING_BUFFER", 1, CORE_2_INDEX, &DHD_TX_POST_FLOW_RING_BUFFER_2, 3, 1, 1 },
#endif
#if defined BCM63146
	{ "DEBUG_SCRATCHPAD", 1, CORE_2_INDEX, &DEBUG_SCRATCHPAD_2, 12, 1, 1 },
#endif
#if defined BCM63146
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_2_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_2, 31, 1, 1 },
#endif
#if defined BCM63146
	{ "FPM_GLOBAL_CFG", 1, CORE_2_INDEX, &FPM_GLOBAL_CFG_2, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_TX_POST_0_STACK", 1, CORE_2_INDEX, &DHD_TX_POST_0_STACK_2, 136, 1, 1 },
#endif
#if defined BCM63146
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_2_INDEX, &FW_ERROR_VECTOR_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_2_INDEX, &BITS_CALC_MASKS_TABLE_2, 4, 1, 1 },
#endif
#if defined BCM63146
	{ "MULTICAST_KEY_MASK", 1, CORE_2_INDEX, &MULTICAST_KEY_MASK_2, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_TX_POST_1_STACK", 1, CORE_2_INDEX, &DHD_TX_POST_1_STACK_2, 136, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_TX_POST_2_STACK", 1, CORE_2_INDEX, &DHD_TX_POST_2_STACK_2, 136, 1, 1 },
#endif
#if defined BCM63146
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_2_INDEX, &INGRESS_FILTER_PROFILE_TABLE_2, 16, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE", 1, CORE_2_INDEX, &DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE_2, 3, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_TX_POST_PD_FIFO_TABLE", 1, CORE_2_INDEX, &DHD_TX_POST_PD_FIFO_TABLE_2, 6, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_FLOW_RING_CACHE_LKP_TABLE", 1, CORE_2_INDEX, &DHD_FLOW_RING_CACHE_LKP_TABLE_2, 48, 1, 1 },
#endif
#if defined BCM63146
	{ "US_TM_PD_FIFO_TABLE", 1, CORE_3_INDEX, &US_TM_PD_FIFO_TABLE_3, 128, 1, 1 },
#endif
#if defined BCM63146
	{ "US_TM_SCHEDULING_QUEUE_TABLE", 1, CORE_3_INDEX, &US_TM_SCHEDULING_QUEUE_TABLE_3, 64, 1, 1 },
#endif
#if defined BCM63146
	{ "US_TM_RATE_LIMITER_PROFILE_TABLE", 1, CORE_3_INDEX, &US_TM_RATE_LIMITER_PROFILE_TABLE_3, 32, 1, 1 },
#endif
#if defined BCM63146
	{ "TX_TASK_US_0_STACK", 1, CORE_3_INDEX, &TX_TASK_US_0_STACK_3, 256, 1, 1 },
#endif
#if defined BCM63146
	{ "US_TM_SCHEDULER_TABLE", 1, CORE_3_INDEX, &US_TM_SCHEDULER_TABLE_3, 24, 1, 1 },
#endif
#if defined BCM63146
	{ "US_TM_TM_FLOW_CNTR_TABLE", 1, CORE_3_INDEX, &US_TM_TM_FLOW_CNTR_TABLE_3, 64, 1, 1 },
#endif
#if defined BCM63146
	{ "GENERAL_TIMER", 1, CORE_3_INDEX, &GENERAL_TIMER_3, 16, 1, 1 },
#endif
#if defined BCM63146
	{ "TX_TASK_US_1_STACK", 1, CORE_3_INDEX, &TX_TASK_US_1_STACK_3, 256, 1, 1 },
#endif
#if defined BCM63146
	{ "PROCESSING_4_TASKS_PACKET_BUFFER", 1, CORE_3_INDEX, &PROCESSING_4_TASKS_PACKET_BUFFER_3, 4, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_COMPLETE_COMMON_RADIO_DATA", 1, CORE_3_INDEX, &DHD_COMPLETE_COMMON_RADIO_DATA_3, 3, 1, 1 },
#endif
#if defined BCM63146
	{ "VPORT_TX_FLOW_TABLE", 1, CORE_3_INDEX, &VPORT_TX_FLOW_TABLE_3, 64, 1, 1 },
#endif
#if defined BCM63146
	{ "VPORT_CFG_TABLE", 1, CORE_3_INDEX, &VPORT_CFG_TABLE_3, 31, 1, 1 },
#endif
#if defined BCM63146
	{ "MUTEX_TABLE", 1, CORE_3_INDEX, &MUTEX_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "US_TM_DSL_TM_FLOW_CNTR_TABLE", 1, CORE_3_INDEX, &US_TM_DSL_TM_FLOW_CNTR_TABLE_3, 256, 1, 1 },
#endif
#if defined BCM63146
	{ "US_TM_RATE_LIMITER_BUDGET_DESCRIPTOR_TABLE", 1, CORE_3_INDEX, &US_TM_RATE_LIMITER_BUDGET_DESCRIPTOR_TABLE_3, 64, 1, 1 },
#endif
#if defined BCM63146
	{ "US_TM_RATE_LIMITER_PARAMS_DESCRIPTOR_TABLE", 1, CORE_3_INDEX, &US_TM_RATE_LIMITER_PARAMS_DESCRIPTOR_TABLE_3, 64, 1, 1 },
#endif
#if defined BCM63146
	{ "DSL_TM_PD_FIFO_TABLE", 1, CORE_3_INDEX, &DSL_TM_PD_FIFO_TABLE_3, 32, 1, 1 },
#endif
#if defined BCM63146
	{ "PROCESSING0_STACK", 1, CORE_3_INDEX, &PROCESSING0_STACK_3, 360, 1, 1 },
#endif
#if defined BCM63146
	{ "NULL_BUFFER", 1, CORE_3_INDEX, &NULL_BUFFER_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "US_TM_RATE_LIMITER_BUDGET_VALID", 1, CORE_3_INDEX, &US_TM_RATE_LIMITER_BUDGET_VALID_3, 4, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_RX_COMPLETE_FLOW_RING_BUFFER", 1, CORE_3_INDEX, &DHD_RX_COMPLETE_FLOW_RING_BUFFER_3, 3, 1, 1 },
#endif
#if defined BCM63146
	{ "US_TM_RATE_LIMITER_PROFILE_RESIDUE_TABLE", 1, CORE_3_INDEX, &US_TM_RATE_LIMITER_PROFILE_RESIDUE_TABLE_3, 32, 1, 1 },
#endif
#if defined BCM63146
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_3_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_3, 128, 1, 1 },
#endif
#if defined BCM63146
	{ "PROCESSING1_STACK", 1, CORE_3_INDEX, &PROCESSING1_STACK_3, 360, 1, 1 },
#endif
#if defined BCM63146
	{ "SYSTEM_CONFIGURATION", 1, CORE_3_INDEX, &SYSTEM_CONFIGURATION_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "US_TM_BBH_TX_WAKE_UP_DATA_TABLE", 1, CORE_3_INDEX, &US_TM_BBH_TX_WAKE_UP_DATA_TABLE_3, 2, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_RX_POST_FLOW_RING_BUFFER", 1, CORE_3_INDEX, &DHD_RX_POST_FLOW_RING_BUFFER_3, 3, 1, 1 },
#endif
#if defined BCM63146
	{ "AQM_ENABLE_TABLE", 1, CORE_3_INDEX, &AQM_ENABLE_TABLE_3, 8, 1, 1 },
#endif
#if defined BCM63146
	{ "PROCESSING2_STACK", 1, CORE_3_INDEX, &PROCESSING2_STACK_3, 360, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR", 1, CORE_3_INDEX, &DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "SPDTEST_GEN_PARAM", 1, CORE_3_INDEX, &SPDTEST_GEN_PARAM_3, 4, 1, 1 },
#endif
#if defined BCM63146
	{ "GENERAL_TIMER_STACK", 1, CORE_3_INDEX, &GENERAL_TIMER_STACK_3, 72, 1, 1 },
#endif
#if defined BCM63146
	{ "MIRRORING_SCRATCH", 1, CORE_3_INDEX, &MIRRORING_SCRATCH_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "US_TM_FLUSH_CFG_CPU_TABLE", 1, CORE_3_INDEX, &US_TM_FLUSH_CFG_CPU_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_3_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_3, 8, 1, 1 },
#endif
#if defined BCM63146
	{ "PROCESSING3_STACK", 1, CORE_3_INDEX, &PROCESSING3_STACK_3, 360, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_RX_POST_RING_SIZE", 1, CORE_3_INDEX, &DHD_RX_POST_RING_SIZE_3, 3, 1, 1 },
#endif
#if defined BCM63146
	{ "AQM_NUM_QUEUES", 1, CORE_3_INDEX, &AQM_NUM_QUEUES_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "US_TM_FLUSH_CFG_FW_TABLE", 1, CORE_3_INDEX, &US_TM_FLUSH_CFG_FW_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "US_BUFFER_CONG_MGT_CFG", 1, CORE_3_INDEX, &US_BUFFER_CONG_MGT_CFG_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_3_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_RX_COMPLETE_RING_SIZE", 1, CORE_3_INDEX, &DHD_RX_COMPLETE_RING_SIZE_3, 3, 1, 1 },
#endif
#if defined BCM63146
	{ "BUDGET_ALLOCATION_TIMER_VALUE", 1, CORE_3_INDEX, &BUDGET_ALLOCATION_TIMER_VALUE_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "US_TM_FLUSH_CFG_CURRENT_TABLE", 1, CORE_3_INDEX, &US_TM_FLUSH_CFG_CURRENT_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_3_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_3, 32, 1, 1 },
#endif
#if defined BCM63146
	{ "DSL_TX_FLOW_TABLE", 1, CORE_3_INDEX, &DSL_TX_FLOW_TABLE_3, 256, 1, 1 },
#endif
#if defined BCM63146
	{ "US_TM_TX_QUEUE_DROP_TABLE", 1, CORE_3_INDEX, &US_TM_TX_QUEUE_DROP_TABLE_3, 64, 1, 1 },
#endif
#if defined BCM63146
	{ "US_TM_AQM_QUEUE_TABLE", 1, CORE_3_INDEX, &US_TM_AQM_QUEUE_TABLE_3, 64, 1, 1 },
#endif
#if defined BCM63146
	{ "US_TM_BBH_QUEUE_TABLE", 1, CORE_3_INDEX, &US_TM_BBH_QUEUE_TABLE_3, 24, 1, 1 },
#endif
#if defined BCM63146
	{ "US_TM_AQM_QUEUE_TIMER_TABLE", 1, CORE_3_INDEX, &US_TM_AQM_QUEUE_TIMER_TABLE_3, 64, 1, 1 },
#endif
#if defined BCM63146
	{ "RX_FLOW_TABLE", 1, CORE_3_INDEX, &RX_FLOW_TABLE_3, 90, 1, 1 },
#endif
#if defined BCM63146
	{ "CSO_BAD_IPV4_HDR_CSUM_PACKETS", 1, CORE_3_INDEX, &CSO_BAD_IPV4_HDR_CSUM_PACKETS_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_TX_COMPLETE_RING_SIZE", 1, CORE_3_INDEX, &DHD_TX_COMPLETE_RING_SIZE_3, 3, 1, 1 },
#endif
#if defined BCM63146
	{ "DOS_DROP_REASONS_CFG", 1, CORE_3_INDEX, &DOS_DROP_REASONS_CFG_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "UPDATE_FIFO_STACK", 1, CORE_3_INDEX, &UPDATE_FIFO_STACK_3, 64, 1, 1 },
#endif
#if defined BCM63146
	{ "POLICER_PARAMS_TABLE", 1, CORE_3_INDEX, &POLICER_PARAMS_TABLE_3, 80, 1, 1 },
#endif
#if defined BCM63146
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_3_INDEX, &BUFMNG_DESCRIPTOR_TABLE_3, 16, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_RX_COMPLETE_0_STACK", 1, CORE_3_INDEX, &DHD_RX_COMPLETE_0_STACK_3, 64, 1, 1 },
#endif
#if defined BCM63146
	{ "US_TM_SECONDARY_SCHEDULER_TABLE", 1, CORE_3_INDEX, &US_TM_SECONDARY_SCHEDULER_TABLE_3, 11, 1, 1 },
#endif
#if defined BCM63146
	{ "TASK_IDX", 1, CORE_3_INDEX, &TASK_IDX_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "TCAM_GENERIC_FIELDS", 1, CORE_3_INDEX, &TCAM_GENERIC_FIELDS_3, 4, 1, 1 },
#endif
#if defined BCM63146
	{ "DIRECT_FLOW_PD_TABLE", 1, CORE_3_INDEX, &DIRECT_FLOW_PD_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "BUFMNG_STATUS_TABLE", 1, CORE_3_INDEX, &BUFMNG_STATUS_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_RX_COMPLETE_1_STACK", 1, CORE_3_INDEX, &DHD_RX_COMPLETE_1_STACK_3, 64, 1, 1 },
#endif
#if defined BCM63146
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_3_INDEX, &INGRESS_FILTER_PROFILE_TABLE_3, 16, 1, 1 },
#endif
#if defined BCM63146
	{ "US_TM_SCHEDULER_POOL", 1, CORE_3_INDEX, &US_TM_SCHEDULER_POOL_3, 128, 1, 1 },
#endif
#if defined BCM63146
	{ "REGISTERS_BUFFER", 1, CORE_3_INDEX, &REGISTERS_BUFFER_3, 32, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_RX_COMPLETE_2_STACK", 1, CORE_3_INDEX, &DHD_RX_COMPLETE_2_STACK_3, 64, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_TX_COMPLETE_0_STACK", 1, CORE_3_INDEX, &DHD_TX_COMPLETE_0_STACK_3, 64, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_TX_COMPLETE_1_STACK", 1, CORE_3_INDEX, &DHD_TX_COMPLETE_1_STACK_3, 64, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_TX_COMPLETE_2_STACK", 1, CORE_3_INDEX, &DHD_TX_COMPLETE_2_STACK_3, 64, 1, 1 },
#endif
#if defined BCM63146
	{ "BUFFER_CONG_SCRATCHPAD", 1, CORE_3_INDEX, &BUFFER_CONG_SCRATCHPAD_3, 8, 1, 1 },
#endif
#if defined BCM63146
	{ "RX_MIRRORING_TABLE", 1, CORE_3_INDEX, &RX_MIRRORING_TABLE_3, 31, 1, 1 },
#endif
#if defined BCM63146
	{ "GENERAL_TIMER_ACTION_VEC", 1, CORE_3_INDEX, &GENERAL_TIMER_ACTION_VEC_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "HOST_TX_TRUNCATE_MIRRORING_TABLE", 1, CORE_3_INDEX, &HOST_TX_TRUNCATE_MIRRORING_TABLE_3, 31, 1, 1 },
#endif
#if defined BCM63146
	{ "US_TM_FLUSH_CFG_ENABLE_TABLE", 1, CORE_3_INDEX, &US_TM_FLUSH_CFG_ENABLE_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "PON_TM_TX_TRUNCATE_MIRRORING_TABLE", 1, CORE_3_INDEX, &PON_TM_TX_TRUNCATE_MIRRORING_TABLE_3, 31, 1, 1 },
#endif
#if defined BCM63146
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_3_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_3_INDEX, &LOOPBACK_QUEUE_TABLE_3, 31, 1, 1 },
#endif
#if defined BCM63146
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_3_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "DIRECT_FLOW_WAN_VIQ_EXCLUSIVE", 1, CORE_3_INDEX, &DIRECT_FLOW_WAN_VIQ_EXCLUSIVE_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "VPORT_CFG_EX_TABLE", 1, CORE_3_INDEX, &VPORT_CFG_EX_TABLE_3, 31, 1, 1 },
#endif
#if defined BCM63146
	{ "TX_EXCEPTION", 1, CORE_3_INDEX, &TX_EXCEPTION_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "LLQ_SELECTOR_ECN_MASK", 1, CORE_3_INDEX, &LLQ_SELECTOR_ECN_MASK_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "US_TM_PI2_PROBABILITY_CALC_DESCRIPTOR", 1, CORE_3_INDEX, &US_TM_PI2_PROBABILITY_CALC_DESCRIPTOR_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "BBH_TX_US_WAN_0_FIFO_BYTES_THRESHOLD", 1, CORE_3_INDEX, &BBH_TX_US_WAN_0_FIFO_BYTES_THRESHOLD_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_3_INDEX, &LOOPBACK_WAN_FLOW_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "FORCE_DSCP", 1, CORE_3_INDEX, &FORCE_DSCP_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "TUNNELS_PARSING_CFG", 1, CORE_3_INDEX, &TUNNELS_PARSING_CFG_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "TR471_SPDSVC_RX_PKT_ID_TABLE", 1, CORE_3_INDEX, &TR471_SPDSVC_RX_PKT_ID_TABLE_3, 4, 1, 1 },
#endif
#if defined BCM63146
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_3_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_3, 16, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE", 1, CORE_3_INDEX, &DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE_3, 3, 1, 1 },
#endif
#if defined BCM63146
	{ "US_TM_DISPATCHER_CREDIT_TABLE", 1, CORE_3_INDEX, &US_TM_DISPATCHER_CREDIT_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "US_TM_FIRST_QUEUE_MAPPING", 1, CORE_3_INDEX, &US_TM_FIRST_QUEUE_MAPPING_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "CORE_ID_TABLE", 1, CORE_3_INDEX, &CORE_ID_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_REDIRECT_MODE", 1, CORE_3_INDEX, &CPU_REDIRECT_MODE_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_TX_COMPLETE_FLOW_RING_BUFFER", 1, CORE_3_INDEX, &DHD_TX_COMPLETE_FLOW_RING_BUFFER_3, 3, 1, 1 },
#endif
#if defined BCM63146
	{ "US_TM_FLUSH_DISPATCHER_CREDIT_TABLE", 1, CORE_3_INDEX, &US_TM_FLUSH_DISPATCHER_CREDIT_TABLE_3, 3, 1, 1 },
#endif
#if defined BCM63146
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_3_INDEX, &RX_MIRRORING_CONFIGURATION_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_3_INDEX, &TX_MIRRORING_CONFIGURATION_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_RX_COMPLETE_DISPATCHER_CREDIT_TABLE", 1, CORE_3_INDEX, &DHD_RX_COMPLETE_DISPATCHER_CREDIT_TABLE_3, 12, 1, 1 },
#endif
#if defined BCM63146
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_3_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "CSO_DISABLE", 1, CORE_3_INDEX, &CSO_DISABLE_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_3_INDEX, &DEBUG_PRINT_CORE_LOCK_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_3_INDEX, &TCAM_TABLE_CFG_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "WAN_0_BBH_TX_FIFO_SIZE", 1, CORE_3_INDEX, &WAN_0_BBH_TX_FIFO_SIZE_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_TX_COMPLETE_DISPATCHER_CREDIT_TABLE", 1, CORE_3_INDEX, &DHD_TX_COMPLETE_DISPATCHER_CREDIT_TABLE_3, 12, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_FPM_THRESHOLDS", 1, CORE_3_INDEX, &DHD_FPM_THRESHOLDS_3, 3, 1, 1 },
#endif
#if defined BCM63146
	{ "SRAM_DUMMY_STORE", 1, CORE_3_INDEX, &SRAM_DUMMY_STORE_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "MAC_TYPE", 1, CORE_3_INDEX, &MAC_TYPE_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_3_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_3_INDEX, &INGRESS_FILTER_1588_CFG_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "CODEL_BIAS_SLOPE_TABLE", 1, CORE_3_INDEX, &CODEL_BIAS_SLOPE_TABLE_3, 11, 1, 1 },
#endif
#if defined BCM63146
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_3_INDEX, &RX_MIRRORING_DIRECT_ENABLE_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_DOORBELL_TX_COMPLETE_VALUE", 1, CORE_3_INDEX, &DHD_DOORBELL_TX_COMPLETE_VALUE_3, 3, 1, 1 },
#endif
#if defined BCM63146
	{ "US_TM_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_TABLE", 1, CORE_3_INDEX, &US_TM_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_TABLE_3, 2, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_DOORBELL_RX_COMPLETE_VALUE", 1, CORE_3_INDEX, &DHD_DOORBELL_RX_COMPLETE_VALUE_3, 3, 1, 1 },
#endif
#if defined BCM63146
	{ "BUFFER_CONG_DQM_NOT_EMPTY", 1, CORE_3_INDEX, &BUFFER_CONG_DQM_NOT_EMPTY_3, 8, 1, 1 },
#endif
#if defined BCM63146
	{ "UPDATE_FIFO_TABLE", 1, CORE_3_INDEX, &UPDATE_FIFO_TABLE_3, 8, 1, 1 },
#endif
#if defined BCM63146
	{ "US_TM_TX_OCTETS_COUNTERS_TABLE", 1, CORE_3_INDEX, &US_TM_TX_OCTETS_COUNTERS_TABLE_3, 8, 1, 1 },
#endif
#if defined BCM63146
	{ "US_TM_SCHEDULING_AGGREGATION_CONTEXT_VECTOR", 1, CORE_3_INDEX, &US_TM_SCHEDULING_AGGREGATION_CONTEXT_VECTOR_3, 5, 1, 1 },
#endif
#if defined BCM63146
	{ "IPTV_CFG_TABLE", 1, CORE_3_INDEX, &IPTV_CFG_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_3_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_3, 31, 1, 1 },
#endif
#if defined BCM63146
	{ "US_TM_CODEL_DROP_DESCRIPTOR", 1, CORE_3_INDEX, &US_TM_CODEL_DROP_DESCRIPTOR_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_3_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_3_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_3, 31, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_HW_CFG", 1, CORE_3_INDEX, &DHD_HW_CFG_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "NAT_CACHE_CFG", 1, CORE_3_INDEX, &NAT_CACHE_CFG_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "US_TM_BBH_TX_EGRESS_COUNTER_TABLE", 1, CORE_3_INDEX, &US_TM_BBH_TX_EGRESS_COUNTER_TABLE_3, 3, 1, 1 },
#endif
#if defined BCM63146
	{ "DEBUG_SCRATCHPAD", 1, CORE_3_INDEX, &DEBUG_SCRATCHPAD_3, 12, 1, 1 },
#endif
#if defined BCM63146
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_3_INDEX, &NAT_CACHE_KEY0_MASK_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_DOORBELL_RX_POST_VALUE", 1, CORE_3_INDEX, &DHD_DOORBELL_RX_POST_VALUE_3, 3, 1, 1 },
#endif
#if defined BCM63146
	{ "FPM_GLOBAL_CFG", 1, CORE_3_INDEX, &FPM_GLOBAL_CFG_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_FPM_REPLY", 1, CORE_3_INDEX, &DHD_FPM_REPLY_3, 24, 1, 1 },
#endif
#if defined BCM63146
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_3_INDEX, &NATC_L2_VLAN_KEY_MASK_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "DHD_CPU_INT_ID", 1, CORE_3_INDEX, &DHD_CPU_INT_ID_3, 3, 1, 1 },
#endif
#if defined BCM63146
	{ "TX_MIRRORING_DISPATCHER_CREDIT_TABLE", 1, CORE_3_INDEX, &TX_MIRRORING_DISPATCHER_CREDIT_TABLE_3, 3, 1, 1 },
#endif
#if defined BCM63146
	{ "US_TM_DSL_BBH_TX_ABS_COUNTER_TABLE", 1, CORE_3_INDEX, &US_TM_DSL_BBH_TX_ABS_COUNTER_TABLE_3, 16, 1, 1 },
#endif
#if defined BCM63146
	{ "SPDSVC_WLAN_GEN_PARAMS_TABLE", 1, CORE_3_INDEX, &SPDSVC_WLAN_GEN_PARAMS_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "DEBUG_PRINT_TABLE", 1, CORE_3_INDEX, &DEBUG_PRINT_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "GDX_PARAMS_TABLE", 1, CORE_3_INDEX, &GDX_PARAMS_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_3_INDEX, &BITS_CALC_MASKS_TABLE_3, 4, 1, 1 },
#endif
#if defined BCM63146
	{ "INGRESS_FILTER_CFG", 1, CORE_3_INDEX, &INGRESS_FILTER_CFG_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "NATC_L2_TOS_MASK", 1, CORE_3_INDEX, &NATC_L2_TOS_MASK_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_3_INDEX, &FW_ERROR_VECTOR_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "US_TM_BBH_TX_ABS_COUNTER_TABLE", 1, CORE_3_INDEX, &US_TM_BBH_TX_ABS_COUNTER_TABLE_3, 8, 1, 1 },
#endif
#if defined BCM63146
	{ "MULTICAST_KEY_MASK", 1, CORE_3_INDEX, &MULTICAST_KEY_MASK_3, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "DS_TM_PD_FIFO_TABLE", 1, CORE_4_INDEX, &DS_TM_PD_FIFO_TABLE_4, 128, 1, 1 },
#endif
#if defined BCM63146
	{ "TCPSPDTEST_STREAM_TABLE", 1, CORE_4_INDEX, &TCPSPDTEST_STREAM_TABLE_4, 4, 1, 1 },
#endif
#if defined BCM63146
	{ "DS_TM_RATE_LIMITER_PROFILE_RESIDUE_TABLE", 1, CORE_4_INDEX, &DS_TM_RATE_LIMITER_PROFILE_RESIDUE_TABLE_4, 32, 1, 1 },
#endif
#if defined BCM63146
	{ "DS_TM_RATE_LIMITER_BUDGET_DESCRIPTOR_TABLE", 1, CORE_4_INDEX, &DS_TM_RATE_LIMITER_BUDGET_DESCRIPTOR_TABLE_4, 64, 1, 1 },
#endif
#if defined BCM63146
	{ "DS_TM_SCHEDULING_QUEUE_TABLE", 1, CORE_4_INDEX, &DS_TM_SCHEDULING_QUEUE_TABLE_4, 64, 1, 1 },
#endif
#if defined BCM63146
	{ "DS_TM_RATE_LIMITER_PROFILE_TABLE", 1, CORE_4_INDEX, &DS_TM_RATE_LIMITER_PROFILE_TABLE_4, 32, 1, 1 },
#endif
#if defined BCM63146
	{ "TX_TASK_DS_0_STACK", 1, CORE_4_INDEX, &TX_TASK_DS_0_STACK_4, 256, 1, 1 },
#endif
#if defined BCM63146
	{ "PKTGEN_TX_STREAM_TABLE", 1, CORE_4_INDEX, &PKTGEN_TX_STREAM_TABLE_4, 4, 1, 1 },
#endif
#if defined BCM63146
	{ "AQM_ENABLE_TABLE", 1, CORE_4_INDEX, &AQM_ENABLE_TABLE_4, 8, 1, 1 },
#endif
#if defined BCM63146
	{ "DS_TM_TM_FLOW_CNTR_TABLE", 1, CORE_4_INDEX, &DS_TM_TM_FLOW_CNTR_TABLE_4, 64, 1, 1 },
#endif
#if defined BCM63146
	{ "TCPSPDTEST_SCRATCHPAD", 1, CORE_4_INDEX, &TCPSPDTEST_SCRATCHPAD_4, 16, 1, 1 },
#endif
#if defined BCM63146
	{ "DS_TM_AQM_QUEUE_TABLE", 1, CORE_4_INDEX, &DS_TM_AQM_QUEUE_TABLE_4, 64, 1, 1 },
#endif
#if defined BCM63146
	{ "DS_TM_RATE_LIMITER_PARAMS_DESCRIPTOR_TABLE", 1, CORE_4_INDEX, &DS_TM_RATE_LIMITER_PARAMS_DESCRIPTOR_TABLE_4, 64, 1, 1 },
#endif
#if defined BCM63146
	{ "DS_TM_SECONDARY_SCHEDULER_TABLE", 1, CORE_4_INDEX, &DS_TM_SECONDARY_SCHEDULER_TABLE_4, 22, 1, 1 },
#endif
#if defined BCM63146
	{ "CPU_RX_INTERRUPT_ID_DDR_ADDR", 1, CORE_4_INDEX, &CPU_RX_INTERRUPT_ID_DDR_ADDR_4, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "DS_TM_RATE_LIMITER_BUDGET_VALID", 1, CORE_4_INDEX, &DS_TM_RATE_LIMITER_BUDGET_VALID_4, 4, 1, 1 },
#endif
#if defined BCM63146
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_4_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_4, 8, 1, 1 },
#endif
#if defined BCM63146
	{ "DS_TM_AQM_QUEUE_TIMER_TABLE", 1, CORE_4_INDEX, &DS_TM_AQM_QUEUE_TIMER_TABLE_4, 64, 1, 1 },
#endif
#if defined BCM63146
	{ "GENERAL_TIMER", 1, CORE_4_INDEX, &GENERAL_TIMER_4, 16, 1, 1 },
#endif
#if defined BCM63146
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_4_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_4, 128, 1, 1 },
#endif
#if defined BCM63146
	{ "DS_TM_TX_QUEUE_DROP_TABLE", 1, CORE_4_INDEX, &DS_TM_TX_QUEUE_DROP_TABLE_4, 64, 1, 1 },
#endif
#if defined BCM63146
	{ "SPDSVC_ANALYZER_STACK", 1, CORE_4_INDEX, &SPDSVC_ANALYZER_STACK_4, 256, 1, 1 },
#endif
#if defined BCM63146
	{ "DS_TM_SCHEDULER_TABLE", 1, CORE_4_INDEX, &DS_TM_SCHEDULER_TABLE_4, 8, 1, 1 },
#endif
#if defined BCM63146
	{ "UPDATE_FIFO_STACK", 1, CORE_4_INDEX, &UPDATE_FIFO_STACK_4, 64, 1, 1 },
#endif
#if defined BCM63146
	{ "UDPSPDT_STREAM_RX_STAT_TABLE", 1, CORE_4_INDEX, &UDPSPDT_STREAM_RX_STAT_TABLE_4, 4, 1, 1 },
#endif
#if defined BCM63146
	{ "COMMON_REPROCESSING_STACK", 1, CORE_4_INDEX, &COMMON_REPROCESSING_STACK_4, 64, 1, 1 },
#endif
#if defined BCM63146
	{ "SPDSVC_GEN_STACK", 1, CORE_4_INDEX, &SPDSVC_GEN_STACK_4, 128, 1, 1 },
#endif
#if defined BCM63146
	{ "GENERAL_TIMER_STACK", 1, CORE_4_INDEX, &GENERAL_TIMER_STACK_4, 72, 1, 1 },
#endif
#if defined BCM63146
	{ "SYSTEM_CONFIGURATION", 1, CORE_4_INDEX, &SYSTEM_CONFIGURATION_4, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "DS_TM_BBH_TX_WAKE_UP_DATA_TABLE", 1, CORE_4_INDEX, &DS_TM_BBH_TX_WAKE_UP_DATA_TABLE_4, 2, 1, 1 },
#endif
#if defined BCM63146
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_4_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_4, 32, 1, 1 },
#endif
#if defined BCM63146
	{ "UDPSPDT_STREAM_TX_STAT_TABLE", 1, CORE_4_INDEX, &UDPSPDT_STREAM_TX_STAT_TABLE_4, 4, 1, 1 },
#endif
#if defined BCM63146
	{ "BUFFER_CONG_DQM_NOT_EMPTY", 1, CORE_4_INDEX, &BUFFER_CONG_DQM_NOT_EMPTY_4, 8, 1, 1 },
#endif
#if defined BCM63146
	{ "BUFFER_CONG_SCRATCHPAD", 1, CORE_4_INDEX, &BUFFER_CONG_SCRATCHPAD_4, 8, 1, 1 },
#endif
#if defined BCM63146
	{ "UDPSPDT_TX_PARAMS_TABLE", 1, CORE_4_INDEX, &UDPSPDT_TX_PARAMS_TABLE_4, 4, 1, 1 },
#endif
#if defined BCM63146
	{ "TCPSPDTEST_ENGINE_GLOBAL_TABLE", 1, CORE_4_INDEX, &TCPSPDTEST_ENGINE_GLOBAL_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "PKTGEN_NO_SBPM_HDRS_CNTR", 1, CORE_4_INDEX, &PKTGEN_NO_SBPM_HDRS_CNTR_4, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "TX_ABS_RECYCLE_COUNTERS", 1, CORE_4_INDEX, &TX_ABS_RECYCLE_COUNTERS_4, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "DS_BUFFER_CONG_MGT_CFG", 1, CORE_4_INDEX, &DS_BUFFER_CONG_MGT_CFG_4, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "MUTEX_TABLE", 1, CORE_4_INDEX, &MUTEX_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "DS_TM_SCHEDULER_POOL", 1, CORE_4_INDEX, &DS_TM_SCHEDULER_POOL_4, 128, 1, 1 },
#endif
#if defined BCM63146
	{ "DEBUG_SCRATCHPAD", 1, CORE_4_INDEX, &DEBUG_SCRATCHPAD_4, 12, 1, 1 },
#endif
#if defined BCM63146
	{ "MIRRORING_SCRATCH", 1, CORE_4_INDEX, &MIRRORING_SCRATCH_4, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "COMMON_REPROCESSING_PD_FIFO_TABLE", 1, CORE_4_INDEX, &COMMON_REPROCESSING_PD_FIFO_TABLE_4, 4, 1, 1 },
#endif
#if defined BCM63146
	{ "HOST_TX_TRUNCATE_MIRRORING_TABLE", 1, CORE_4_INDEX, &HOST_TX_TRUNCATE_MIRRORING_TABLE_4, 31, 1, 1 },
#endif
#if defined BCM63146
	{ "AQM_NUM_QUEUES", 1, CORE_4_INDEX, &AQM_NUM_QUEUES_4, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "SPDSVC_ANALYZER_PD_FIFO_TABLE", 1, CORE_4_INDEX, &SPDSVC_ANALYZER_PD_FIFO_TABLE_4, 4, 1, 1 },
#endif
#if defined BCM63146
	{ "ETH_TM_TX_TRUNCATE_MIRRORING_TABLE", 1, CORE_4_INDEX, &ETH_TM_TX_TRUNCATE_MIRRORING_TABLE_4, 31, 1, 1 },
#endif
#if defined BCM63146
	{ "BUDGET_ALLOCATION_TIMER_VALUE", 1, CORE_4_INDEX, &BUDGET_ALLOCATION_TIMER_VALUE_4, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "TCPSPDTEST_PD_FIFO_TABLE", 1, CORE_4_INDEX, &TCPSPDTEST_PD_FIFO_TABLE_4, 4, 1, 1 },
#endif
#if defined BCM63146
	{ "SPDSVC_GEN_PARAMS_TABLE", 1, CORE_4_INDEX, &SPDSVC_GEN_PARAMS_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_4_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "DS_TM_BBH_QUEUE_TABLE", 1, CORE_4_INDEX, &DS_TM_BBH_QUEUE_TABLE_4, 8, 1, 1 },
#endif
#if defined BCM63146
	{ "DS_TM_PI2_PROBABILITY_CALC_DESCRIPTOR", 1, CORE_4_INDEX, &DS_TM_PI2_PROBABILITY_CALC_DESCRIPTOR_4, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "PKTGEN_BAD_GET_NEXT", 1, CORE_4_INDEX, &PKTGEN_BAD_GET_NEXT_4, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "PKTGEN_CURR_SBPM_HDR_PTR", 1, CORE_4_INDEX, &PKTGEN_CURR_SBPM_HDR_PTR_4, 4, 1, 1 },
#endif
#if defined BCM63146
	{ "PKTGEN_SBPM_HDR_BNS", 1, CORE_4_INDEX, &PKTGEN_SBPM_HDR_BNS_4, 32, 1, 1 },
#endif
#if defined BCM63146
	{ "REGISTERS_BUFFER", 1, CORE_4_INDEX, &REGISTERS_BUFFER_4, 32, 1, 1 },
#endif
#if defined BCM63146
	{ "CODEL_BIAS_SLOPE_TABLE", 1, CORE_4_INDEX, &CODEL_BIAS_SLOPE_TABLE_4, 11, 1, 1 },
#endif
#if defined BCM63146
	{ "PKTGEN_MAX_UT_PKTS", 1, CORE_4_INDEX, &PKTGEN_MAX_UT_PKTS_4, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "SPDTEST_GEN_PARAM", 1, CORE_4_INDEX, &SPDTEST_GEN_PARAM_4, 4, 1, 1 },
#endif
#if defined BCM63146
	{ "DS_TM_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_TABLE", 1, CORE_4_INDEX, &DS_TM_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_TABLE_4, 2, 1, 1 },
#endif
#if defined BCM63146
	{ "DS_TM_FLUSH_CFG_CPU_TABLE", 1, CORE_4_INDEX, &DS_TM_FLUSH_CFG_CPU_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "DS_TM_SCHEDULING_AGGREGATION_CONTEXT_VECTOR", 1, CORE_4_INDEX, &DS_TM_SCHEDULING_AGGREGATION_CONTEXT_VECTOR_4, 5, 1, 1 },
#endif
#if defined BCM63146
	{ "PKTGEN_UT_TRIGGER", 1, CORE_4_INDEX, &PKTGEN_UT_TRIGGER_4, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "PKTGEN_NUM_OF_AVAIL_SBPM_HDRS", 1, CORE_4_INDEX, &PKTGEN_NUM_OF_AVAIL_SBPM_HDRS_4, 4, 1, 1 },
#endif
#if defined BCM63146
	{ "UPDATE_FIFO_TABLE", 1, CORE_4_INDEX, &UPDATE_FIFO_TABLE_4, 8, 1, 1 },
#endif
#if defined BCM63146
	{ "DS_TM_CODEL_DROP_DESCRIPTOR", 1, CORE_4_INDEX, &DS_TM_CODEL_DROP_DESCRIPTOR_4, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "TASK_IDX", 1, CORE_4_INDEX, &TASK_IDX_4, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "PKTGEN_SBPM_END_PTR", 1, CORE_4_INDEX, &PKTGEN_SBPM_END_PTR_4, 4, 1, 1 },
#endif
#if defined BCM63146
	{ "COMMON_REPROCESSING_UPDATE_FIFO_TABLE", 1, CORE_4_INDEX, &COMMON_REPROCESSING_UPDATE_FIFO_TABLE_4, 8, 1, 1 },
#endif
#if defined BCM63146
	{ "DS_TM_FLUSH_CFG_FW_TABLE", 1, CORE_4_INDEX, &DS_TM_FLUSH_CFG_FW_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "DS_TM_FLUSH_CFG_CURRENT_TABLE", 1, CORE_4_INDEX, &DS_TM_FLUSH_CFG_CURRENT_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "SPDSVC_ANALYZER_UPDATE_FIFO_TABLE", 1, CORE_4_INDEX, &SPDSVC_ANALYZER_UPDATE_FIFO_TABLE_4, 8, 1, 1 },
#endif
#if defined BCM63146
	{ "SPDSVC_ANALYZER_PARAMS_TABLE", 1, CORE_4_INDEX, &SPDSVC_ANALYZER_PARAMS_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "DS_TM_FLUSH_DISPATCHER_CREDIT_TABLE", 1, CORE_4_INDEX, &DS_TM_FLUSH_DISPATCHER_CREDIT_TABLE_4, 3, 1, 1 },
#endif
#if defined BCM63146
	{ "GENERAL_TIMER_ACTION_VEC", 1, CORE_4_INDEX, &GENERAL_TIMER_ACTION_VEC_4, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "DS_TM_FLUSH_CFG_ENABLE_TABLE", 1, CORE_4_INDEX, &DS_TM_FLUSH_CFG_ENABLE_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "TCPSPDTEST_UPDATE_FIFO_TABLE", 1, CORE_4_INDEX, &TCPSPDTEST_UPDATE_FIFO_TABLE_4, 8, 1, 1 },
#endif
#if defined BCM63146
	{ "FPM_GLOBAL_CFG", 1, CORE_4_INDEX, &FPM_GLOBAL_CFG_4, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "TX_EXCEPTION", 1, CORE_4_INDEX, &TX_EXCEPTION_4, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "CORE_ID_TABLE", 1, CORE_4_INDEX, &CORE_ID_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "DS_TM_FIRST_QUEUE_MAPPING", 1, CORE_4_INDEX, &DS_TM_FIRST_QUEUE_MAPPING_4, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "PKTGEN_SESSION_DATA", 1, CORE_4_INDEX, &PKTGEN_SESSION_DATA_4, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "SPDSVC_GEN_DISPATCHER_CREDIT_TABLE", 1, CORE_4_INDEX, &SPDSVC_GEN_DISPATCHER_CREDIT_TABLE_4, 3, 1, 1 },
#endif
#if defined BCM63146
	{ "SPDSVC_TCPSPDTEST_COMMON_TABLE", 1, CORE_4_INDEX, &SPDSVC_TCPSPDTEST_COMMON_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_4_INDEX, &DEBUG_PRINT_CORE_LOCK_4, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_4_INDEX, &RX_MIRRORING_CONFIGURATION_4, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "TCPSPDTEST_DISPATCHER_CREDIT_TABLE", 1, CORE_4_INDEX, &TCPSPDTEST_DISPATCHER_CREDIT_TABLE_4, 3, 1, 1 },
#endif
#if defined BCM63146
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_4_INDEX, &TX_MIRRORING_CONFIGURATION_4, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "SRAM_DUMMY_STORE", 1, CORE_4_INDEX, &SRAM_DUMMY_STORE_4, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_4_INDEX, &RX_MIRRORING_DIRECT_ENABLE_4, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_4_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_4, 31, 1, 1 },
#endif
#if defined BCM63146
	{ "COMMON_REPROCESSING_DISPATCHER_CREDIT_TABLE", 1, CORE_4_INDEX, &COMMON_REPROCESSING_DISPATCHER_CREDIT_TABLE_4, 3, 1, 1 },
#endif
#if defined BCM63146
	{ "TCPSPDTEST_GEN_DISPATCHER_CREDIT_TABLE", 1, CORE_4_INDEX, &TCPSPDTEST_GEN_DISPATCHER_CREDIT_TABLE_4, 3, 1, 1 },
#endif
#if defined BCM63146
	{ "PKTGEN_FPM_UG_MGMT", 1, CORE_4_INDEX, &PKTGEN_FPM_UG_MGMT_4, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_4_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "TX_MIRRORING_DISPATCHER_CREDIT_TABLE", 1, CORE_4_INDEX, &TX_MIRRORING_DISPATCHER_CREDIT_TABLE_4, 3, 1, 1 },
#endif
#if defined BCM63146
	{ "DS_TM_BBH_TX_EGRESS_COUNTER_TABLE", 1, CORE_4_INDEX, &DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "UDPSPDT_SCRATCH_TABLE", 1, CORE_4_INDEX, &UDPSPDT_SCRATCH_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "PKTGEN_SBPM_EXTS", 1, CORE_4_INDEX, &PKTGEN_SBPM_EXTS_4, 4, 1, 1 },
#endif
#if defined BCM63146
	{ "PKTGEN_BBMSG_REPLY_SCRATCH", 1, CORE_4_INDEX, &PKTGEN_BBMSG_REPLY_SCRATCH_4, 2, 1, 1 },
#endif
#if defined BCM63146
	{ "DS_TM_BBH_TX_ABS_COUNTER_TABLE", 1, CORE_4_INDEX, &DS_TM_BBH_TX_ABS_COUNTER_TABLE_4, 8, 1, 1 },
#endif
#if defined BCM63146
	{ "DEBUG_PRINT_TABLE", 1, CORE_4_INDEX, &DEBUG_PRINT_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_4_INDEX, &FW_ERROR_VECTOR_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM63146
	{ "PKTGEN_TX_STREAM_SCRATCH_TABLE", 1, CORE_4_INDEX, &PKTGEN_TX_STREAM_SCRATCH_TABLE_4, 1, 1, 1 },
#endif
};

TABLE_STACK_STRUCT RUNNER_STACK_TABLES[NUMBER_OF_STACK_TABLES] =
{
    { "PROCESSING0_STACK", CORE_0_INDEX, 0xa00, 360},
    { "PROCESSING1_STACK", CORE_0_INDEX, 0xc00, 360},
    { "PROCESSING2_STACK", CORE_0_INDEX, 0xe00, 360},
    { "CPU_RX_COPY_STACK", CORE_0_INDEX, 0xf80, 128},
    { "PROCESSING3_STACK", CORE_0_INDEX, 0x1e00, 360},
    { "PROCESSING4_STACK", CORE_0_INDEX, 0x2000, 360},
    { "PROCESSING5_STACK", CORE_0_INDEX, 0x2200, 360},
    { "PROCESSING6_STACK", CORE_0_INDEX, 0x2400, 360},
    { "CPU_RX_STACK", CORE_0_INDEX, 0x2780, 80},
    { "SERVICE_QUEUES_STACK", CORE_0_INDEX, 0x2a00, 256},
    { "GENERAL_TIMER_STACK", CORE_0_INDEX, 0x3180, 72},
    { "CPU_RECYCLE_STACK", CORE_0_INDEX, 0x3aa0, 32},
    { "PROCESSING0_STACK", CORE_1_INDEX, 0x200, 360},
    { "PROCESSING1_STACK", CORE_1_INDEX, 0x600, 360},
    { "PROCESSING2_STACK", CORE_1_INDEX, 0x800, 360},
    { "GENERAL_TIMER_STACK", CORE_1_INDEX, 0x980, 72},
    { "PROCESSING3_STACK", CORE_1_INDEX, 0xa00, 360},
    { "PROCESSING4_STACK", CORE_1_INDEX, 0xc00, 360},
    { "PROCESSING5_STACK", CORE_1_INDEX, 0xe00, 360},
    { "CPU_TX_0_STACK", CORE_1_INDEX, 0x2d00, 256},
    { "PROCESSING6_STACK", CORE_1_INDEX, 0x2e00, 360},
    { "PROCESSING7_STACK", CORE_1_INDEX, 0x3000, 360},
    { "CPU_RECYCLE_STACK", CORE_1_INDEX, 0x31a0, 32},
    { "CPU_TX_1_STACK", CORE_1_INDEX, 0x3200, 256},
    { "CPU_FPM_RINGS_AND_BUFMNG_REFILL_STACK", CORE_2_INDEX, 0xa80, 128},
    { "PROCESSING0_STACK", CORE_2_INDEX, 0xe00, 360},
    { "PROCESSING1_STACK", CORE_2_INDEX, 0x2000, 360},
    { "PROCESSING2_STACK", CORE_2_INDEX, 0x2200, 360},
    { "PROCESSING3_STACK", CORE_2_INDEX, 0x2400, 360},
    { "PROCESSING4_STACK", CORE_2_INDEX, 0x2600, 360},
    { "PROCESSING5_STACK", CORE_2_INDEX, 0x2800, 360},
    { "PROCESSING6_STACK", CORE_2_INDEX, 0x2a00, 360},
    { "PROCESSING7_STACK", CORE_2_INDEX, 0x2c00, 360},
    { "DHD_TIMER_STACK", CORE_2_INDEX, 0x2ee0, 32},
    { "DHD_TX_POST_UPDATE_FIFO_STACK", CORE_2_INDEX, 0x2fa0, 32},
    { "DHD_TX_POST_0_STACK", CORE_2_INDEX, 0x3100, 136},
    { "DHD_TX_POST_1_STACK", CORE_2_INDEX, 0x3200, 136},
    { "DHD_TX_POST_2_STACK", CORE_2_INDEX, 0x3300, 136},
    { "TX_TASK_US_0_STACK", CORE_3_INDEX, 0xb00, 256},
    { "TX_TASK_US_1_STACK", CORE_3_INDEX, 0xf00, 256},
    { "PROCESSING0_STACK", CORE_3_INDEX, 0x2200, 360},
    { "PROCESSING1_STACK", CORE_3_INDEX, 0x2600, 360},
    { "PROCESSING2_STACK", CORE_3_INDEX, 0x2800, 360},
    { "GENERAL_TIMER_STACK", CORE_3_INDEX, 0x2980, 72},
    { "PROCESSING3_STACK", CORE_3_INDEX, 0x2a00, 360},
    { "UPDATE_FIFO_STACK", CORE_3_INDEX, 0x31c0, 64},
    { "DHD_RX_COMPLETE_0_STACK", CORE_3_INDEX, 0x32c0, 64},
    { "DHD_RX_COMPLETE_1_STACK", CORE_3_INDEX, 0x33c0, 64},
    { "DHD_RX_COMPLETE_2_STACK", CORE_3_INDEX, 0x3580, 64},
    { "DHD_TX_COMPLETE_0_STACK", CORE_3_INDEX, 0x35c0, 64},
    { "DHD_TX_COMPLETE_1_STACK", CORE_3_INDEX, 0x3600, 64},
    { "DHD_TX_COMPLETE_2_STACK", CORE_3_INDEX, 0x3640, 64},
    { "TX_TASK_DS_0_STACK", CORE_4_INDEX, 0x1300, 256},
    { "SPDSVC_ANALYZER_STACK", CORE_4_INDEX, 0x2000, 256},
    { "UPDATE_FIFO_STACK", CORE_4_INDEX, 0x21c0, 64},
    { "COMMON_REPROCESSING_STACK", CORE_4_INDEX, 0x22c0, 64},
    { "SPDSVC_GEN_STACK", CORE_4_INDEX, 0x2300, 128},
    { "GENERAL_TIMER_STACK", CORE_4_INDEX, 0x2380, 72}
};
