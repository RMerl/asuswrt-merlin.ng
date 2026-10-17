/*
   Copyright (c) 2025 Broadcom
   All Rights Reserved

    <:label-BRCM:2025:DUAL/GPL:standard

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
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SQ_TM_PD_FIFO_TABLE =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, set_RDD_PROCESSING_TX_DESCRIPTOR, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_COMPLETE_COMMON_RADIO_DATA =
{
	320,
	{
		{ dump_RDD_DHD_COMPLETE_COMMON_RADIO_ENTRY, set_RDD_DHD_COMPLETE_COMMON_RADIO_ENTRY, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT VPORT_TX_FLOW_TABLE =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0x7c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, set_RDD_RX_FLOW_ENTRY, 0xa00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT AQM_SQ_BITMAP =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xb94 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_SCRATCHPAD =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xb98 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SQ_TM_RATE_LIMITER_PROFILE_RESIDUE_TABLE =
{
	1,
	{
		{ dump_RDD_RATE_LIMITER_PROFILE_RESIDUE, set_RDD_RATE_LIMITER_PROFILE_RESIDUE, 0xba0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT AQM_SQ_TABLE =
{
	2,
	{
		{ dump_RDD_AQM_SQ_ENTRY, set_RDD_AQM_SQ_ENTRY, 0xbc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT NULL_BUFFER =
{
	8,
	{
		{ dump_RDD_BYTES_8, set_RDD_BYTES_8, 0xd68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SQ_TM_RATE_LIMITER_BUDGET_VALID =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xd70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SQ_TM_RATE_LIMITER_PROFILE_TABLE =
{
	4,
	{
		{ dump_RDD_RATE_LIMITER_PROFILE, set_RDD_RATE_LIMITER_PROFILE, 0xd80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0xf68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, set_RDD_SPDTEST_GEN_CFG, 0xf70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, set_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SCORE_CFG_TABLE =
{
	8,
	{
		{ dump_RDD_SCORE_CFG_ENTRY, set_RDD_SCORE_CFG_ENTRY, 0x2168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SQ_TM_DCL_IMG =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, set_RDD_DDR_ADDRESS, 0x2368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SQ_TM_SECONDARY_SCHEDULER_TABLE =
{
	12,
	{
		{ dump_RDD_SECONDARY_SCHEDULER_DESCRIPTOR, set_RDD_SECONDARY_SCHEDULER_DESCRIPTOR, 0x2370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x237c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER =
{
	8,
	{
		{ dump_RDD_GENERAL_TIMER_ENTRY, set_RDD_GENERAL_TIMER_ENTRY, 0x2380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_RX_POST_RING_SIZE =
{
	2,
	{
		{ dump_RDD_DHD_RING_SIZE, set_RDD_DHD_RING_SIZE, 0x2568 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT AQM_NUM_QUEUES =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x256e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SQ_TM_DISPATCHER_CREDIT_TABLE =
{
	12,
	{
		{ dump_RDD_DISPATCHER_CREDIT_DESCRIPTOR, set_RDD_DISPATCHER_CREDIT_DESCRIPTOR, 0x2570 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x257c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SCORE_DATA_COLLECTION_PROFILE_TABLE =
{
	16,
	{
		{ dump_RDD_SCORE_DATA_COLLECTION_PROFILE, set_RDD_SCORE_DATA_COLLECTION_PROFILE, 0x2580 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_RING_SIZE =
{
	2,
	{
		{ dump_RDD_DHD_RING_SIZE, set_RDD_DHD_RING_SIZE, 0x2768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x276e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_AQM_TIMER_DISPATCHER_CREDIT_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2770 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CSO_BAD_IPV4_HDR_CSUM_PACKETS =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x277c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_0_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_RING_SIZE =
{
	2,
	{
		{ dump_RDD_DHD_RING_SIZE, set_RDD_DHD_RING_SIZE, 0x2968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_ACTION_VEC =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x296e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x2970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SCORE_COLLECT_DROP_BITMAP_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x297c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_1_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, set_RDD_TUNNELS_PARSING_CFG, 0x2b68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2b70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SQ_TM_UPDATE_FIFO_START_INDEX =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2b7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_2_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2b80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SQ_TM_SCHEDULING_QUEUE_TABLE =
{
	10,
	{
		{ dump_RDD_SCHEDULING_QUEUE_DESCRIPTOR, set_RDD_SCHEDULING_QUEUE_DESCRIPTOR, 0x2c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_0_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2d40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, set_RDD_VPORT_CFG_ENTRY, 0x2d80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TASK_IDX =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2dfc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SQ_TM_RATE_LIMITER_BUDGET_DESCRIPTOR_TABLE =
{
	8,
	{
		{ dump_RDD_RATE_LIMITER_BUDGET_DESCRIPTOR, set_RDD_RATE_LIMITER_BUDGET_DESCRIPTOR, 0x2e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SQ_TM_RATE_LIMITER_PARAMS_DESCRIPTOR_TABLE =
{
	8,
	{
		{ dump_RDD_RATE_LIMITER_PARAMS_DESCRIPTOR, set_RDD_RATE_LIMITER_PARAMS_DESCRIPTOR, 0x2f00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PON_TX_FLOW_TABLE =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0x3000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_MONITOR_PKT_INFO_TABLE =
{
	56,
	{
		{ dump_RDD_DHD_MONITOR_PKT_INFO, set_RDD_DHD_MONITOR_PKT_INFO, 0x3100 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT AQM_ENABLE_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x31e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, set_RDD_POLICER_PARAMS_ENTRY, 0x3200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SQ_TM_TX_QUEUE_DROP_TABLE =
{
	8,
	{
		{ dump_RDD_PACKETS_AND_BYTES, set_RDD_PACKETS_AND_BYTES, 0x32a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x33a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_1_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x33c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, set_RDD_INGRESS_FILTER_CTRL, 0x3400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_FLOW_RING_BUFFER =
{
	32,
	{
		{ dump_RDD_DHD_RX_COMPLETE_DESCRIPTOR, set_RDD_DHD_RX_COMPLETE_DESCRIPTOR, 0x3480 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0x34e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_RX_POST_FLOW_RING_BUFFER =
{
	32,
	{
		{ dump_RDD_DHD_RX_POST_DESCRIPTOR, set_RDD_DHD_RX_POST_DESCRIPTOR, 0x3500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, set_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0x3560 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3580 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SQ_TM_AQM_QUEUE_TABLE =
{
	4,
	{
		{ dump_RDD_AQM_QUEUE_DESCRIPTOR, set_RDD_AQM_QUEUE_DESCRIPTOR, 0x35c8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3648 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, set_RDD_IPTV_CFG, 0x36c8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_DOORBELL_TX_COMPLETE_VALUE =
{
	4,
	{
		{ dump_RDD_DHD_DOORBELL, set_RDD_DHD_DOORBELL, 0x36d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x36dc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SQ_TM_FIRST_QUEUE_MAPPING =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x36de },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, set_RDD_BUFMNG_STATUS_ENTRY, 0x36e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_2_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT BUFFER_CONG_SCRATCHPAD =
{
	8,
	{
		{ dump_RDD_BUFFER_CONG_Q_OCCUPANCY, set_RDD_BUFFER_CONG_Q_OCCUPANCY, 0x3740 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_TABLE =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, set_RDD_MIRRORING_TRUNCATE_ENTRY, 0x3780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x37be },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x37bf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT HOST_TX_TRUNCATE_MIRRORING_TABLE =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, set_RDD_MIRRORING_TRUNCATE_ENTRY, 0x37c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT FORCE_DSCP =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x37fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x37ff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x3800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x383e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, set_RDD_VPORT_CFG_EX_ENTRY, 0x3840 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x387e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT BUFFER_CONG_DQM_NOT_EMPTY =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3880 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CSO_DISABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x38b4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x38b5 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x38b6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, set_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x38b7 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, set_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x38b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_TABLE =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, set_RDD_TR471_SPDSVC_RX_PKT_ID, 0x38c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_DOORBELL_RX_COMPLETE_VALUE =
{
	4,
	{
		{ dump_RDD_DHD_DOORBELL, set_RDD_DHD_DOORBELL, 0x38f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x38fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x38fd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_PCAP_TX_RING_DESCRIPTOR_TABLE =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, set_RDD_CPU_RING_DESCRIPTOR, 0x3900 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_DOORBELL_RX_POST_VALUE =
{
	4,
	{
		{ dump_RDD_DHD_DOORBELL, set_RDD_DHD_DOORBELL, 0x3930 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_FLOW_RING_BUFFER =
{
	16,
	{
		{ dump_RDD_DHD_TX_COMPLETE_DESCRIPTOR, set_RDD_DHD_TX_COMPLETE_DESCRIPTOR, 0x3940 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_CPU_INT_ID =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_PCAP_RX_RING_DESCRIPTOR_TABLE =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, set_RDD_CPU_RING_DESCRIPTOR, 0x3980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_PCAP_CFG =
{
	4,
	{
		{ dump_RDD_DHD_PCAP_CONFIG, set_RDD_DHD_PCAP_CONFIG, 0x39b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_DISPATCHER_CREDIT_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x39c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SPDSVC_WLAN_GEN_PARAMS_TABLE =
{
	10,
	{
		{ dump_RDD_SPDSVC_WLAN_GEN_PARAMS, set_RDD_SPDSVC_WLAN_GEN_PARAMS, 0x39f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, set_RDD_CPU_RING_DESCRIPTOR, 0x3a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0x3a30 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_DISPATCHER_CREDIT_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3a40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, set_RDD_TCAM_GENERIC, 0x3a70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SQ_BUFFER_CONG_MGT_CFG =
{
	48,
	{
		{ dump_RDD_BUFFER_CONG_MGT, set_RDD_BUFFER_CONG_MGT, 0x3a80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, set_RDD_GDX_PARAMS, 0x3ab0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SQ_TM_ENABLED_QUEUES_TABLE =
{
	34,
	{
		{ dump_RDD_ENABLED_QUEUES_ENTRY, set_RDD_ENABLED_QUEUES_ENTRY, 0x3ac0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SQ_TM_SCHEDULER_POOL =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3ae8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, set_RDD_NAT_CACHE_CFG, 0x3b28 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3b30 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, set_RDD_TCAM_TABLE_CFG, 0x3b38 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SQ_TM_UPDATE_FIFO_TABLE =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, set_RDD_UPDATE_FIFO_ENTRY, 0x3b40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SQ_TM_BUFMNG_STATUS_TABLE =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, set_RDD_BUFMNG_STATUS_ENTRY, 0x3b60 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SQ_TM_DCL_DATA_BASE =
{
	32,
	{
		{ dump_RDD_DCL_DATA, set_RDD_DCL_DATA, 0x3b80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SQ_TM_FW_FLUSH_TABLE =
{
	28,
	{
		{ dump_RDD_FW_FLUSH_ENTRY, set_RDD_FW_FLUSH_ENTRY, 0x3ba0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0x3bc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SQ_TM_SCHEDULER_TABLE =
{
	24,
	{
		{ dump_RDD_SCHEDULER_DESCRIPTOR, set_RDD_SCHEDULER_DESCRIPTOR, 0x3be0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x3bf8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3c20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, set_RDD_INGRESS_FILTER_CFG, 0x3c50 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3c58 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, set_RDD_DHD_HW_CONFIGURATION, 0x3c60 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_FPM_REPLY =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3c80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SQ_TM_AQM_QUEUE_TIMER_TABLE =
{
	1,
	{
		{ dump_RDD_AQM_QUEUE_TIMER_DESCRIPTOR, set_RDD_AQM_QUEUE_TIMER_DESCRIPTOR, 0x3c98 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3cc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0x3ce0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SQ_TM_PI2_PROBABILITY_CALC_DESCRIPTOR =
{
	20,
	{
		{ dump_RDD_PI2_PROBABILITY_CALC_DESCRIPTOR, set_RDD_PI2_PROBABILITY_CALC_DESCRIPTOR, 0x3d00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3d20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, set_RDD_MULTICAST_KEY_MASK_ENTRY, 0x3d40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_STREAM_TABLE_1 =
{
	376,
	{
		{ dump_RDD_TCPSPDTEST_STREAM, set_RDD_TCPSPDTEST_STREAM, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT AQM_ENABLE_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x5e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_1 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, set_RDD_RX_FLOW_ENTRY, 0x600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PKTGEN_NO_SBPM_HDRS_CNTR_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x794 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_RX_INTERRUPT_ID_DDR_ADDR_1 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, set_RDD_DDR_ADDRESS, 0x798 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x7a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT VPORT_TX_FLOW_TABLE_1 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0x7c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PKTGEN_TX_STREAM_TABLE_1 =
{
	136,
	{
		{ dump_RDD_PKTGEN_TX_STREAM_ENTRY, set_RDD_PKTGEN_TX_STREAM_ENTRY, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_1 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0xa20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT COMMON_REPROCESSING_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xa40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_SCRATCHPAD_1 =
{
	8,
	{
		{ dump_RDD_BYTES_8, set_RDD_BYTES_8, 0xa80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PON_TX_FLOW_TABLE_1 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0xb00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_1 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT NULL_BUFFER_1 =
{
	8,
	{
		{ dump_RDD_BYTES_8, set_RDD_BYTES_8, 0xf68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_1 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, set_RDD_SPDTEST_GEN_CFG, 0xf70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_1 =
{
	8,
	{
		{ dump_RDD_GENERAL_TIMER_ENTRY, set_RDD_GENERAL_TIMER_ENTRY, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER_1 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, set_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_1 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x2168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SPDSVC_GEN_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TX_ABS_RECYCLE_COUNTERS_1 =
{
	8,
	{
		{ dump_RDD_TX_ABS_RECYCLE_COUNTERS_ENTRY, set_RDD_TX_ABS_RECYCLE_COUNTERS_ENTRY, 0x2368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SPDSVC_GEN_DISPATCHER_CREDIT_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x237c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SCORE_DATA_COLLECTION_PROFILE_TABLE_1 =
{
	16,
	{
		{ dump_RDD_SCORE_DATA_COLLECTION_PROFILE, set_RDD_SCORE_DATA_COLLECTION_PROFILE, 0x2380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SCORE_CFG_TABLE_1 =
{
	8,
	{
		{ dump_RDD_SCORE_CFG_ENTRY, set_RDD_SCORE_CFG_ENTRY, 0x2568 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_DISPATCHER_CREDIT_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2570 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x257c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_1 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, set_RDD_VPORT_CFG_ENTRY, 0x2580 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CSO_BAD_IPV4_HDR_CSUM_PACKETS_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x25fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PKTGEN_CURR_SBPM_HDR_PTR_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT COMMON_REPROCESSING_DISPATCHER_CREDIT_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2770 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SCORE_COLLECT_DROP_BITMAP_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x277c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PKTGEN_NUM_OF_AVAIL_SBPM_HDRS_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x27c8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_GEN_DISPATCHER_CREDIT_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x27d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PKTGEN_BAD_GET_NEXT_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x27dc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_1 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, set_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0x27e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PKTGEN_SBPM_END_PTR_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_1 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x2970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PKTGEN_MAX_UT_PKTS_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x297c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT COMMON_REPROCESSING_PD_FIFO_TABLE_1 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, set_RDD_PROCESSING_TX_DESCRIPTOR, 0x2980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_TABLE_1 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, set_RDD_MIRRORING_TRUNCATE_ENTRY, 0x29c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT AQM_NUM_QUEUES_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x29fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_1 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, set_RDD_TUNNELS_PARSING_CFG, 0x2b68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2b70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PKTGEN_UT_TRIGGER_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2b7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_PD_FIFO_TABLE_1 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, set_RDD_PROCESSING_TX_DESCRIPTOR, 0x2b80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT HOST_TX_TRUNCATE_MIRRORING_TABLE_1 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, set_RDD_MIRRORING_TRUNCATE_ENTRY, 0x2bc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2bfe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PKTGEN_SBPM_EXTS_1 =
{
	4,
	{
		{ dump_RDD_PKTGEN_SBPM_EXT, set_RDD_PKTGEN_SBPM_EXT, 0x2d68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_1 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, set_RDD_IPTV_CFG, 0x2d78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PKTGEN_SBPM_HDR_BNS_1 =
{
	2,
	{
		{ dump_RDD_PKTGEN_SBPM_HDR_BN, set_RDD_PKTGEN_SBPM_HDR_BN, 0x2d80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2dc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_ACTION_VEC_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2dfe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT UDPSPDT_STREAM_TX_STAT_TABLE_1 =
{
	40,
	{
		{ dump_RDD_UDPSPDT_STREAM_TX_STAT, set_RDD_UDPSPDT_STREAM_TX_STAT, 0x2e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_1 =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, set_RDD_BUFMNG_STATUS_ENTRY, 0x2ea0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_1 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, set_RDD_VPORT_CFG_EX_ENTRY, 0x2ec0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2efe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_1 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, set_RDD_POLICER_PARAMS_ENTRY, 0x2f00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_ENGINE_GLOBAL_TABLE_1 =
{
	20,
	{
		{ dump_RDD_TCPSPDTEST_ENGINE_GLOBAL_INFO, set_RDD_TCPSPDTEST_ENGINE_GLOBAL_INFO, 0x2fa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TASK_IDX_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2fb4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_1 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, set_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x2fb8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SPDSVC_GEN_PARAMS_TABLE_1 =
{
	60,
	{
		{ dump_RDD_SPDSVC_GEN_PARAMS, set_RDD_SPDSVC_GEN_PARAMS, 0x2fc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2ffc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2ffd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2ffe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2fff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_1 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, set_RDD_INGRESS_FILTER_CTRL, 0x3000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3080 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT UDPSPDT_TX_PARAMS_TABLE_1 =
{
	24,
	{
		{ dump_RDD_UDPSPDT_TX_PARAMS, set_RDD_UDPSPDT_TX_PARAMS, 0x3100 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_1 =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, set_RDD_DHD_HW_CONFIGURATION, 0x3160 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SPDSVC_TCPSPDTEST_COMMON_TABLE_1 =
{
	1,
	{
		{ dump_RDD_SPDSVC_TCPSPDTEST_COMMON_ENTRY, set_RDD_SPDSVC_TCPSPDTEST_COMMON_ENTRY, 0x3174 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CSO_DISABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3175 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_1 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x3176 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_1 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, set_RDD_NAT_CACHE_CFG, 0x3178 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x317f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_TABLE_1 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, set_RDD_TR471_SPDSVC_RX_PKT_ID, 0x3180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PKTGEN_BBMSG_REPLY_SCRATCH_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x31b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x31b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_1 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x31bc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x31be },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_1 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, set_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x31bf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT COMMON_REPROCESSING_UPDATE_FIFO_TABLE_1 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, set_RDD_UPDATE_FIFO_ENTRY, 0x31c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_1 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0x31e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x31fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x31fd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_UPDATE_FIFO_TABLE_1 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, set_RDD_UPDATE_FIFO_ENTRY, 0x3200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_1 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0x3220 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_1 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, set_RDD_TCAM_GENERIC, 0x3230 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PKTGEN_SESSION_DATA_1 =
{
	32,
	{
		{ dump_RDD_PKTGEN_TX_PARAMS, set_RDD_PKTGEN_TX_PARAMS, 0x3240 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_1 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, set_RDD_GDX_PARAMS, 0x3260 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_1 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, set_RDD_TCAM_TABLE_CFG, 0x3270 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x3278 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_1 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0x3280 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x32a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_1 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, set_RDD_INGRESS_FILTER_CFG, 0x32a8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x32b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x32c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x32e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_1 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, set_RDD_MULTICAST_KEY_MASK_ENTRY, 0x3320 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PKTGEN_FPM_UG_MGMT_1 =
{
	20,
	{
		{ dump_RDD_PKTGEN_FPM_UG_MGMT_ENTRY, set_RDD_PKTGEN_FPM_UG_MGMT_ENTRY, 0x3340 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PKTGEN_TX_STREAM_SCRATCH_TABLE_1 =
{
	8,
	{
		{ dump_RDD_PKTGEN_TX_STREAM_SCRATCH_ENTRY, set_RDD_PKTGEN_TX_STREAM_SCRATCH_ENTRY, 0x3400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_TX_SCRATCHPAD_2 =
{
	8,
	{
		{ dump_RDD_BYTES_8, set_RDD_BYTES_8, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PON_TX_FLOW_TABLE_2 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0xd00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_2 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, set_RDD_RX_FLOW_ENTRY, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_INT_INTERRUPT_SCRATCH_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xf94 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_2 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, set_RDD_DDR_ADDRESS, 0xf98 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT AQM_ENABLE_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xfa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT VPORT_TX_FLOW_TABLE_2 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0xfc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER_2 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, set_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_2 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_RX_INTERRUPT_ID_DDR_ADDR_2 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, set_RDD_DDR_ADDRESS, 0x2368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_INTERRUPT_COALESCING_TABLE_2 =
{
	16,
	{
		{ dump_RDD_CPU_INTERRUPT_COALESCING_ENTRY, set_RDD_CPU_INTERRUPT_COALESCING_ENTRY, 0x2370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_2 =
{
	8,
	{
		{ dump_RDD_GENERAL_TIMER_ENTRY, set_RDD_GENERAL_TIMER_ENTRY, 0x2380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT NULL_BUFFER_2 =
{
	8,
	{
		{ dump_RDD_BYTES_8, set_RDD_BYTES_8, 0x2568 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_2 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, set_RDD_SPDTEST_GEN_CFG, 0x2570 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SCORE_DATA_COLLECTION_PROFILE_TABLE_2 =
{
	16,
	{
		{ dump_RDD_SCORE_DATA_COLLECTION_PROFILE, set_RDD_SCORE_DATA_COLLECTION_PROFILE, 0x2580 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_2 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x2768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT BUFFER_ALLOC_REPLY_2 =
{
	8,
	{
		{ dump_RDD_BYTES_8, set_RDD_BYTES_8, 0x2770 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_2 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, set_RDD_VPORT_CFG_ENTRY, 0x2780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x27fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SCORE_CFG_TABLE_2 =
{
	8,
	{
		{ dump_RDD_SCORE_CFG_ENTRY, set_RDD_SCORE_CFG_ENTRY, 0x2968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SG_DESC_TABLE_2 =
{
	48,
	{
		{ dump_RDD_SG_DESC_ENTRY, set_RDD_SG_DESC_ENTRY, 0x2980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x29e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE_2 =
{
	8,
	{
		{ dump_RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY, set_RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY, 0x2b68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2b70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2b7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2b80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_TX_RING_INDICES_VALUES_TABLE_2 =
{
	4,
	{
		{ dump_RDD_CPU_TX_RING_INDICES, set_RDD_CPU_TX_RING_INDICES, 0x2bc8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_TX_INGRESS_DISPATCHER_CREDIT_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2bd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CSO_BAD_IPV4_HDR_CSUM_PACKETS_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2bdc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_2 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0x2be0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_2 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, set_RDD_TUNNELS_PARSING_CFG, 0x2d68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_2 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x2d70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SCORE_COLLECT_DROP_BITMAP_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2d7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_TX_RNR_CTR_REPLY_TABLE_2 =
{
	32,
	{
		{ dump_RDD_RNR_COUNTER_REPLY_ENTRY, set_RDD_RNR_COUNTER_REPLY_ENTRY, 0x2d80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_TABLE_2 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, set_RDD_MIRRORING_TRUNCATE_ENTRY, 0x2dc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT AQM_NUM_QUEUES_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2dfe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2f68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_2 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, set_RDD_IPTV_CFG, 0x2fe8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2ff0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TASK_IDX_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2ffc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_2 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, set_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x3168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_DESCRIPTOR_TABLE_2 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, set_RDD_CPU_RING_DESCRIPTOR, 0x3170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT HOST_TX_TRUNCATE_MIRRORING_TABLE_2 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, set_RDD_MIRRORING_TRUNCATE_ENTRY, 0x3180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x31be },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x31c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_ACTION_VEC_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x31fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_TX_0_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_2 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, set_RDD_VPORT_CFG_EX_ENTRY, 0x3340 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x337e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_TABLE_2 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, set_RDD_TR471_SPDSVC_RX_PKT_ID, 0x3380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE_2 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, set_RDD_DDR_ADDRESS, 0x33b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_2 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, set_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0x33c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_2 =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, set_RDD_BUFMNG_STATUS_ENTRY, 0x33e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_TX_1_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3540 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_2 =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, set_RDD_DHD_HW_CONFIGURATION, 0x3560 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3574 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3575 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3576 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3577 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_2 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, set_RDD_NAT_CACHE_CFG, 0x3578 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CSO_DISABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x357f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RING_CPU_TX_DESCRIPTOR_DATA_TABLE_2 =
{
	16,
	{
		{ dump_RDD_RING_CPU_TX_DESCRIPTOR, set_RDD_RING_CPU_TX_DESCRIPTOR, 0x3580 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_TX_RING_DESCRIPTOR_TABLE_2 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, set_RDD_CPU_RING_DESCRIPTOR, 0x35a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_2 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0x35c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x35df },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_2 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0x35e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_2 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x35fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_2 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x35fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT QM_QUEUE_TO_TX_FLOW_TABLE_PTR_TABLE_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x3600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3740 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x375f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3760 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_INTERRUPT_SCRATCH_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3790 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_SHADOW_RD_IDX_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x3798 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_2 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, set_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x379a },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x379b },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x379c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_2 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0x37a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_2 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, set_RDD_TCAM_GENERIC, 0x37b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT QUEUE_THRESHOLD_VECTOR_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x37c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_SHADOW_WR_IDX_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x37d8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_2 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, set_RDD_GDX_PARAMS, 0x37e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_NEXT_PTR_TABLE_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x37f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x37f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_SRAM_PD_FIFO_2 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, set_RDD_PROCESSING_TX_DESCRIPTOR, 0x3800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_2 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, set_RDD_POLICER_PARAMS_ENTRY, 0x3900 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x39a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_2 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, set_RDD_TCAM_TABLE_CFG, 0x39a8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x39b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_2 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, set_RDD_INGRESS_FILTER_CFG, 0x39b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x39c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x39d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_2 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, set_RDD_MULTICAST_KEY_MASK_ENTRY, 0x39e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SG_CONTEXT_TABLE_2 =
{
	64,
	{
		{ dump_RDD_SG_CONTEXT_ENTRY, set_RDD_SG_CONTEXT_ENTRY, 0x3a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_TX_SYNC_FIFO_TABLE_2 =
{
	8,
	{
		{ dump_RDD_CPU_TX_SYNC_FIFO_ENTRY, set_RDD_CPU_TX_SYNC_FIFO_ENTRY, 0x3a80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_2 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, set_RDD_INGRESS_FILTER_CTRL, 0x3b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CRYPTO_SESSION_PARAMS_TABLE_3 =
{
	12,
	{
		{ dump_RDD_CRYPTO_SESSION_PARAMS, set_RDD_CRYPTO_SESSION_PARAMS, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PON_TX_FLOW_TABLE_3 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0x300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_RX_SCRATCHPAD_3 =
{
	8,
	{
		{ dump_RDD_BYTES_8, set_RDD_BYTES_8, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_3 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_3 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, set_RDD_RX_FLOW_ENTRY, 0xa00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xb94 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_RX_INTERRUPT_ID_DDR_ADDR_3 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, set_RDD_DDR_ADDRESS, 0xb98 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT IPV4_HOST_ADDRESS_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xba0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT VPORT_TX_FLOW_TABLE_3 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0xbc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT NULL_BUFFER_3 =
{
	8,
	{
		{ dump_RDD_BYTES_8, set_RDD_BYTES_8, 0xd68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_RX_AUX_INT_SCRATCHPAD_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xd70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_RX_METER_TABLE_3 =
{
	8,
	{
		{ dump_RDD_CPU_RX_METER_ENTRY, set_RDD_CPU_RX_METER_ENTRY, 0xd80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_3 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0xf68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_3 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, set_RDD_SPDTEST_GEN_CFG, 0xf70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_RXQ_SCRATCH_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER_3 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, set_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SCORE_CFG_TABLE_3 =
{
	8,
	{
		{ dump_RDD_SCORE_CFG_ENTRY, set_RDD_SCORE_CFG_ENTRY, 0x2168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SPDSVC_ANALYZER_PARAMS_TABLE_3 =
{
	16,
	{
		{ dump_RDD_SPDSVC_ANALYZER_PARAMS, set_RDD_SPDSVC_ANALYZER_PARAMS, 0x2170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_3 =
{
	8,
	{
		{ dump_RDD_GENERAL_TIMER_ENTRY, set_RDD_GENERAL_TIMER_ENTRY, 0x2180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RNR_WR_TEST_ADDR_3 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, set_RDD_DDR_ADDRESS, 0x2368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_RX_CSO_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_3 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, set_RDD_TUNNELS_PARSING_CFG, 0x2568 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_RX_CSO_DISPATCHER_CREDIT_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2570 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x257c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SCORE_DATA_COLLECTION_PROFILE_TABLE_3 =
{
	16,
	{
		{ dump_RDD_SCORE_DATA_COLLECTION_PROFILE, set_RDD_SCORE_DATA_COLLECTION_PROFILE, 0x2580 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CSO_BAD_IPV4_HDR_CSUM_PACKETS_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SCORE_COLLECT_DROP_BITMAP_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x276c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_3 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x2770 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_RX_INTERRUPT_SCRATCH_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x277c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_3 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, set_RDD_VPORT_CFG_ENTRY, 0x2780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RNR_WR_TEST_FROM_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x27fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_3 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, set_RDD_IPTV_CFG, 0x2968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RNR_WR_TEST_TO_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x297c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_REASON_AND_VPORT_TO_METER_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT AQM_ENABLE_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x29e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_3 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, set_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x2b68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_INTERRUPT_COALESCING_TABLE_3 =
{
	16,
	{
		{ dump_RDD_CPU_INTERRUPT_COALESCING_ENTRY, set_RDD_CPU_INTERRUPT_COALESCING_ENTRY, 0x2b70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2b80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_RX_LOCAL_SCRATCH_3 =
{
	8,
	{
		{ dump_RDD_BYTES_8, set_RDD_BYTES_8, 0x2bc8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_3 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, set_RDD_NAT_CACHE_CFG, 0x2bd8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2bdf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2be0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_RX_XPM_PD_FIFO_TABLE_3 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, set_RDD_PROCESSING_TX_DESCRIPTOR, 0x2c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_REASON_TO_METER_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2cc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_RING_INTERRUPT_COUNTER_TABLE_3 =
{
	8,
	{
		{ dump_RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY, set_RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY, 0x2d00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT IPV6_HOST_ADDRESS_CRC_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2dc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT UDPSPDT_STREAM_RX_STAT_TABLE_3 =
{
	48,
	{
		{ dump_RDD_UDPSPDT_STREAM_RX_STAT, set_RDD_UDPSPDT_STREAM_RX_STAT, 0x2e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SPU_REQUEST_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2ec0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CSO_CONTEXT_TABLE_3 =
{
	176,
	{
		{ dump_RDD_CSO_CONTEXT_ENTRY, set_RDD_CSO_CONTEXT_ENTRY, 0x2f00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT UDPSPDT_SCRATCH_TABLE_3 =
{
	8,
	{
		{ dump_RDD_UDPSPDT_SCRATCH_IPERF3_RX, set_RDD_UDPSPDT_SCRATCH_IPERF3_RX, 0x2fb0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TASK_IDX_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2fb8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT AQM_NUM_QUEUES_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2fbc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2fbe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_TABLE_3 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, set_RDD_MIRRORING_TRUNCATE_ENTRY, 0x2fc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_ACTION_VEC_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2ffe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_RING_DESCRIPTORS_TABLE_3 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, set_RDD_CPU_RING_DESCRIPTOR, 0x3000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SPDSVC_ANALYZER_PD_FIFO_TABLE_3 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, set_RDD_PROCESSING_TX_DESCRIPTOR, 0x3180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT HOST_TX_TRUNCATE_MIRRORING_TABLE_3 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, set_RDD_MIRRORING_TRUNCATE_ENTRY, 0x31c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x31fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SPDSVC_ANALYZER_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_3 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0x32b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x32c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x32fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x32ff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_3 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, set_RDD_POLICER_PARAMS_ENTRY, 0x3300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_3 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0x33a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_3 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, set_RDD_VPORT_CFG_EX_ENTRY, 0x33c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x33fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CSO_DISABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x33ff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_RX_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_3 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, set_RDD_TCAM_GENERIC, 0x3488 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3498 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_3 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x349c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_3 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x349e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_3 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, set_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0x34a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_TABLE_3 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, set_RDD_TR471_SPDSVC_RX_PKT_ID, 0x34c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_3 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, set_RDD_GDX_PARAMS, 0x34f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RNR_WR_TEST_START_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x34fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x34fd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x34fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_3 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, set_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x34ff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_3 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, set_RDD_INGRESS_FILTER_CTRL, 0x3500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SPU_REQUEST_PD_FIFO_TABLE_3 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, set_RDD_PROCESSING_TX_DESCRIPTOR, 0x3580 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SPU_OFFLOAD_PARAMS_TABLE_3 =
{
	48,
	{
		{ dump_RDD_SPU_OFFLOAD_PARAMS, set_RDD_SPU_OFFLOAD_PARAMS, 0x35c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_RX_TELEMETRY_BUFFER_LOCATION_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x35f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x35f2 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x35f3 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_3 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, set_RDD_TCAM_TABLE_CFG, 0x35f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_RX_CSO_PD_FIFO_TABLE_3 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, set_RDD_PROCESSING_TX_DESCRIPTOR, 0x3600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SPU_REQUEST_PREPEND_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CRYPTO_SESSION_STATS_SCRATCH_3 =
{
	128,
	{
		{ dump_RDD_CRYPTO_SESSION_SEQ_INFO, set_RDD_CRYPTO_SESSION_SEQ_INFO, 0x3780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SPU_OFFLOAD_SCRATCHPAD_3 =
{
	8,
	{
		{ dump_RDD_BYTES_8, set_RDD_BYTES_8, 0x3800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x3848 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_3 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, set_RDD_INGRESS_FILTER_CFG, 0x3850 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3858 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_3 =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, set_RDD_BUFMNG_STATUS_ENTRY, 0x3860 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_RX_XPM_UPDATE_FIFO_TABLE_3 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, set_RDD_UPDATE_FIFO_ENTRY, 0x3880 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_3 =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, set_RDD_DHD_HW_CONFIGURATION, 0x38a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_RX_CSO_UPDATE_FIFO_TABLE_3 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, set_RDD_UPDATE_FIFO_ENTRY, 0x38c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_3 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0x38e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SPDSVC_ANALYZER_UPDATE_FIFO_TABLE_3 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, set_RDD_UPDATE_FIFO_ENTRY, 0x3900 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT BCM_SPDSVC_STREAM_RX_TS_TABLE_3 =
{
	12,
	{
		{ dump_RDD_SPDSVC_RX_TS_STAT, set_RDD_SPDSVC_RX_TS_STAT, 0x3920 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SPU_REQUEST_UPDATE_FIFO_TABLE_3 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, set_RDD_UPDATE_FIFO_ENTRY, 0x3940 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_REASON_TO_TC_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3960 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TC_TO_CPU_RXQ_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x39a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT EXC_TC_TO_CPU_RXQ_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x39e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3a20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_RXQ_DATA_BUF_TYPE_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3a40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_3 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, set_RDD_MULTICAST_KEY_MASK_ENTRY, 0x3a60 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_VPORT_TO_METER_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3a80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_3 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0x3ac0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3b20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3b80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT VPORT_TO_CPU_OBJ_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3bc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_FLOW_RING_CACHE_CTX_TABLE_4 =
{
	16,
	{
		{ dump_RDD_DHD_FLOW_RING_CACHE_CTX_ENTRY, set_RDD_DHD_FLOW_RING_CACHE_CTX_ENTRY, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PON_TX_FLOW_TABLE_4 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0x300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_AUX_INFO_CACHE_TABLE_4 =
{
	16,
	{
		{ dump_RDD_DHD_AUX_INFO_CACHE_ENTRY, set_RDD_DHD_AUX_INFO_CACHE_ENTRY, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_DOORBELL_TX_POST_VALUE_4 =
{
	4,
	{
		{ dump_RDD_DHD_DOORBELL, set_RDD_DHD_DOORBELL, 0x700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_POST_COMMON_RADIO_DATA_4 =
{
	200,
	{
		{ dump_RDD_DHD_POST_COMMON_RADIO_ENTRY, set_RDD_DHD_POST_COMMON_RADIO_ENTRY, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_RX_INTERRUPT_ID_DDR_ADDR_4 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, set_RDD_DDR_ADDRESS, 0xa58 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT AQM_ENABLE_TABLE_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xa60 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_FPM_BUFMNG_RELEASE_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xa80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_4 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, set_RDD_POLICER_PARAMS_ENTRY, 0xb00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xba0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT VPORT_TX_FLOW_TABLE_4 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0xbc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_4 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_4 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, set_RDD_RX_FLOW_ENTRY, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xf94 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT NULL_BUFFER_4 =
{
	8,
	{
		{ dump_RDD_BYTES_8, set_RDD_BYTES_8, 0xf98 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_4 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0xfa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_TABLE_4 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, set_RDD_MIRRORING_TRUNCATE_ENTRY, 0xfc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT AQM_NUM_QUEUES_4 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0xffe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER_4 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, set_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_DSTAT_CFG_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SCORE_DATA_COLLECTION_PROFILE_TABLE_4 =
{
	16,
	{
		{ dump_RDD_SCORE_DATA_COLLECTION_PROFILE, set_RDD_SCORE_DATA_COLLECTION_PROFILE, 0x2180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_4 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x2368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_4 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, set_RDD_SPDTEST_GEN_CFG, 0x2370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_MONITOR_BQ_INFO_TABLE_4 =
{
	32,
	{
		{ dump_RDD_DHD_MONITOR_BQ_INFO, set_RDD_DHD_MONITOR_BQ_INFO, 0x2380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SCORE_CFG_TABLE_4 =
{
	8,
	{
		{ dump_RDD_SCORE_CFG_ENTRY, set_RDD_SCORE_CFG_ENTRY, 0x2568 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2570 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_4 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, set_RDD_VPORT_CFG_ENTRY, 0x2580 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x25fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT MIRRORING_SCRATCH_4 =
{
	8,
	{
		{ dump_RDD_BYTES_8, set_RDD_BYTES_8, 0x2768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SPU_RESPONSE_DISPATCHER_CREDIT_TABLE_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2770 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CSO_BAD_IPV4_HDR_CSUM_PACKETS_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x277c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_BACKUP_INDEX_CACHE_4 =
{
	32,
	{
		{ dump_RDD_DHD_BACKUP_IDX_CACHE_TABLE, set_RDD_DHD_BACKUP_IDX_CACHE_TABLE, 0x2780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_4 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, set_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0x27e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_4 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, set_RDD_DDR_ADDRESS, 0x2968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_4 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x2970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SCORE_COLLECT_DROP_BITMAP_TABLE_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x297c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SPU_RESPONSE_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_4 =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, set_RDD_BUFMNG_STATUS_ENTRY, 0x29e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_4 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, set_RDD_TUNNELS_PARSING_CFG, 0x2b68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2b70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TASK_IDX_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2b7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_L2_HEADER_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2b80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_4 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, set_RDD_IPTV_CFG, 0x2bc8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_CPU_INT_ID_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2bd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_4 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2bdc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_4 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2bde },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_TIMER_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2be0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2d68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_4 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, set_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x2de8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_MIRRORING_DISPATCHER_CREDIT_TABLE_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2df0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SPDSVC_WLAN_TXPOST_PARAMS_TABLE_4 =
{
	2,
	{
		{ dump_RDD_SPDSVC_WLAN_TXPOST_PARAMS, set_RDD_SPDSVC_WLAN_TXPOST_PARAMS, 0x2dfc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2dfe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2dff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_4 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, set_RDD_NAT_CACHE_CFG, 0x2f68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2f6f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_4 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0x2f70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CRYPTO_SESSION_STATS_SCRATCH_4 =
{
	128,
	{
		{ dump_RDD_CRYPTO_SESSION_SEQ_INFO, set_RDD_CRYPTO_SESSION_SEQ_INFO, 0x2f80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_4 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, set_RDD_TCAM_GENERIC, 0x3168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3178 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x317c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CSO_DISABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x317d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_4 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x317e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT HOST_TX_TRUNCATE_MIRRORING_TABLE_4 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, set_RDD_MIRRORING_TRUNCATE_ENTRY, 0x3180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_4 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x31be },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_4 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x31c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x31fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x31ff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT FPM_RING_BUFMGT_CNT_TABLE_4 =
{
	264,
	{
		{ dump_RDD_FPM_RING_BUFMNG_CNT, set_RDD_FPM_RING_BUFMNG_CNT, 0x3200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_4 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, set_RDD_TCAM_TABLE_CFG, 0x3308 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_4 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, set_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x330a },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x330b },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x330c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_4 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, set_RDD_GDX_PARAMS, 0x3310 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_UPDATE_FIFO_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3320 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_4 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, set_RDD_VPORT_CFG_EX_ENTRY, 0x3340 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SPU_RESPONSE_PARAMS_TABLE_4 =
{
	56,
	{
		{ dump_RDD_SPU_RESPONSE_PARAMS, set_RDD_SPU_RESPONSE_PARAMS, 0x3380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_4 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x33b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_TABLE_4 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, set_RDD_TR471_SPDSVC_RX_PKT_ID, 0x33c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_4 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, set_RDD_INGRESS_FILTER_CFG, 0x33f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x33f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_FLOW_RING_BUFFER_4 =
{
	48,
	{
		{ dump_RDD_DHD_TX_POST_DESCRIPTOR, set_RDD_DHD_TX_POST_DESCRIPTOR, 0x3400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SPU_RESPONSE_SCRATCHPAD_4 =
{
	8,
	{
		{ dump_RDD_BYTES_8, set_RDD_BYTES_8, 0x3490 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_4 =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, set_RDD_DHD_HW_CONFIGURATION, 0x34e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_0_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3588 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_UPDATE_FIFO_TABLE_4 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, set_RDD_UPDATE_FIFO_ENTRY, 0x35c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_4 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0x35e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_1_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x36a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_4 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0x36c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_4 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, set_RDD_MULTICAST_KEY_MASK_ENTRY, 0x36e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_2_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x37c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_4 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, set_RDD_INGRESS_FILTER_CTRL, 0x3800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_CPU_TX_POST_RING_DESCRIPTOR_TABLE_4 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, set_RDD_CPU_RING_DESCRIPTOR, 0x3880 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x38c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_PD_FIFO_TABLE_4 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, set_RDD_PROCESSING_TX_DESCRIPTOR, 0x3900 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE_4 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, set_RDD_CPU_RING_DESCRIPTOR, 0x3980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_FLOW_RING_CACHE_LKP_TABLE_4 =
{
	2,
	{
		{ dump_RDD_DHD_FLOW_RING_CACHE_LKP_ENTRY, set_RDD_DHD_FLOW_RING_CACHE_LKP_ENTRY, 0x3a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_TX_MCORE_SCRATCHPAD_5 =
{
	8,
	{
		{ dump_RDD_BYTES_8, set_RDD_BYTES_8, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DIRECT_FLOW_STACK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PON_TX_FLOW_TABLE_5 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0x700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT REPORTING_QUEUE_DESCRIPTOR_TABLE_5 =
{
	8,
	{
		{ dump_RDD_REPORTING_QUEUE_DESCRIPTOR, set_RDD_REPORTING_QUEUE_DESCRIPTOR, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_5 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, set_RDD_DDR_ADDRESS, 0xc08 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_INTERRUPT_COALESCING_TABLE_5 =
{
	16,
	{
		{ dump_RDD_CPU_INTERRUPT_COALESCING_ENTRY, set_RDD_CPU_INTERRUPT_COALESCING_ENTRY, 0xc10 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DIRECT_FLOW_CNTR_TABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xc20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT VPORT_TX_FLOW_TABLE_5 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0xc40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_5 =
{
	8,
	{
		{ dump_RDD_GENERAL_TIMER_ENTRY, set_RDD_GENERAL_TIMER_ENTRY, 0xc80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT REPORTING_QUEUE_ACCUMULATED_TABLE_5 =
{
	16,
	{
		{ dump_RDD_QM_QUEUE_COUNTER_DATA, set_RDD_QM_QUEUE_COUNTER_DATA, 0xd00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_5 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, set_RDD_RX_FLOW_ENTRY, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_INT_INTERRUPT_SCRATCH_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xf94 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT NULL_BUFFER_5 =
{
	8,
	{
		{ dump_RDD_BYTES_8, set_RDD_BYTES_8, 0xf98 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT REPORTING_STACK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xfa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_TABLE_5 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, set_RDD_MIRRORING_TRUNCATE_ENTRY, 0xfc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DIRECT_FLOW_PAUSE_QUANTA_5 =
{
	2,
	{
		{ dump_RDD_PAUSE_QUANTA_ENTRY, set_RDD_PAUSE_QUANTA_ENTRY, 0xffe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER_5 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, set_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_5 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_5 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x2368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_5 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, set_RDD_SPDTEST_GEN_CFG, 0x2370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SCORE_DATA_COLLECTION_PROFILE_TABLE_5 =
{
	16,
	{
		{ dump_RDD_SCORE_DATA_COLLECTION_PROFILE, set_RDD_SCORE_DATA_COLLECTION_PROFILE, 0x2380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SCORE_CFG_TABLE_5 =
{
	8,
	{
		{ dump_RDD_SCORE_CFG_ENTRY, set_RDD_SCORE_CFG_ENTRY, 0x2568 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DIRECT_FLOW_PD_TABLE_5 =
{
	16,
	{
		{ dump_RDD_PROCESSING_RX_DESCRIPTOR, set_RDD_PROCESSING_RX_DESCRIPTOR, 0x2570 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_5 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, set_RDD_VPORT_CFG_ENTRY, 0x2580 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x25fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT REPORT_BBH_TX_QUEUE_ID_TABLE_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2770 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_STACK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE_5 =
{
	8,
	{
		{ dump_RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY, set_RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY, 0x27c8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DIRECT_FLOW_PAUSE_DEBUG_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x27d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x27dc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT AQM_ENABLE_TABLE_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x27e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_5 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, set_RDD_TUNNELS_PARSING_CFG, 0x29e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x29f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CSO_BAD_IPV4_HDR_CSUM_PACKETS_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x29fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SCORE_COLLECT_DROP_BITMAP_TABLE_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2b68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT GHOST_REPORTING_GLOBAL_CFG_5 =
{
	4,
	{
		{ dump_RDD_GHOST_REPORTING_GLOBAL_CFG_ENTRY, set_RDD_GHOST_REPORTING_GLOBAL_CFG_ENTRY, 0x2b6c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_5 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x2b70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_TX_MCORE_RING_INDICES_VALUES_TABLE_5 =
{
	4,
	{
		{ dump_RDD_CPU_TX_RING_INDICES, set_RDD_CPU_TX_RING_INDICES, 0x2b7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SG_CONTEXT_MCORE_TABLE_5 =
{
	64,
	{
		{ dump_RDD_SG_CONTEXT_ENTRY, set_RDD_SG_CONTEXT_ENTRY, 0x2b80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT HOST_TX_TRUNCATE_MIRRORING_TABLE_5 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, set_RDD_MIRRORING_TRUNCATE_ENTRY, 0x2bc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT AQM_NUM_QUEUES_5 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2bfe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_5 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, set_RDD_IPTV_CFG, 0x2d68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2d70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TASK_IDX_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2d7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_5 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2d80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_5 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2dbe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_5 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, set_RDD_VPORT_CFG_EX_ENTRY, 0x2dc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_ACTION_VEC_5 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2dfe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_5 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, set_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x2f68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_DESCRIPTOR_TABLE_5 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, set_RDD_CPU_RING_DESCRIPTOR, 0x2f70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_TABLE_5 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, set_RDD_TR471_SPDSVC_RX_PKT_ID, 0x2f80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE_5 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, set_RDD_DDR_ADDRESS, 0x2fb0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SG_DESC_MCORE_TABLE_5 =
{
	48,
	{
		{ dump_RDD_SG_DESC_ENTRY, set_RDD_SG_DESC_ENTRY, 0x2fc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_INTERRUPT_SCRATCH_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2ff0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT BBH_TX_EGRESS_REPORT_COUNTER_TABLE_5 =
{
	8,
	{
		{ dump_RDD_BBH_TX_EGRESS_COUNTER_ENTRY, set_RDD_BBH_TX_EGRESS_COUNTER_ENTRY, 0x2ff8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT BBH_TX_INGRESS_COUNTER_TABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_TX_MCORE_RING_DESCRIPTOR_TABLE_5 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, set_RDD_CPU_RING_DESCRIPTOR, 0x3170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_TX_MCORE_RNR_CTR_REPLY_TABLE_5 =
{
	32,
	{
		{ dump_RDD_RNR_COUNTER_REPLY_ENTRY, set_RDD_RNR_COUNTER_REPLY_ENTRY, 0x3180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x31a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_5 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0x31c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_5 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, set_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0x31e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_TX_MCORE_STACK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_5 =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, set_RDD_BUFMNG_STATUS_ENTRY, 0x3340 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_STACK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3360 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_5 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0x3380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT US_TM_TX_PAUSE_NACK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x339f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DIRECT_FLOW_EPON_CONTROL_SCRATCH_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x33a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_5 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x33b6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_5 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, set_RDD_NAT_CACHE_CFG, 0x33b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x33bf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x33c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DIRECT_FLOW_WAN_VIQ_EXCLUSIVE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x33df },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_5 =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, set_RDD_DHD_HW_CONFIGURATION, 0x33e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x33f4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x33f5 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x33f6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CSO_DISABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x33f7 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_SHADOW_RD_IDX_5 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x33f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_5 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x33fa },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_5 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x33fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x33fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x33ff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT REPORTING_QUEUE_COUNTER_TABLE_5 =
{
	2,
	{
		{ dump_RDD_REPORTING_QUEUE_COUNTER, set_RDD_REPORTING_QUEUE_COUNTER, 0x3400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT MAC_TYPE_5 =
{
	1,
	{
		{ dump_RDD_MAC_TYPE_ENTRY, set_RDD_MAC_TYPE_ENTRY, 0x3502 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_5 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, set_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x3503 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3504 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3505 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3508 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_SHADOW_WR_IDX_5 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x3538 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT QUEUE_THRESHOLD_VECTOR_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3540 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3558 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT GHOST_REPORTING_QUEUE_STATUS_BIT_VECTOR_TABLE_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3560 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_5 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, set_RDD_TCAM_TABLE_CFG, 0x3578 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT QUEUE_TO_REPORT_BIT_VECTOR_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3580 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_5 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x3598 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_5 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0x35a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x35c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_5 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0x35d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_5 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, set_RDD_TCAM_GENERIC, 0x35e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_5 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, set_RDD_GDX_PARAMS, 0x35f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_SRAM_PD_FIFO_5 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, set_RDD_PROCESSING_TX_DESCRIPTOR, 0x3600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_5 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, set_RDD_POLICER_PARAMS_ENTRY, 0x3700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT XGPON_REPORT_ZERO_SENT_TABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x37a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_NEXT_PTR_TABLE_5 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x37b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_5 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, set_RDD_INGRESS_FILTER_CFG, 0x37b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x37c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x37c8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_5 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, set_RDD_MULTICAST_KEY_MASK_ENTRY, 0x37e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT REPORTING_COUNTER_TABLE_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DIRECT_FLOW_RX_MIRRORING_SCRATCHPAD_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3900 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_5 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, set_RDD_INGRESS_FILTER_CTRL, 0x3a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RING_CPU_TX_MCORE_DESCRIPTOR_DATA_TABLE_5 =
{
	16,
	{
		{ dump_RDD_RING_CPU_TX_DESCRIPTOR, set_RDD_RING_CPU_TX_DESCRIPTOR, 0x3a80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT US_TM_PD_FIFO_TABLE_6 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, set_RDD_PROCESSING_TX_DESCRIPTOR, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT US_TM_SECONDARY_SCHEDULER_TABLE_6 =
{
	12,
	{
		{ dump_RDD_SECONDARY_SCHEDULER_DESCRIPTOR, set_RDD_SECONDARY_SCHEDULER_DESCRIPTOR, 0xa00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TX_TASK_US_0_STACK_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT US_TM_SCHEDULING_QUEUE_TABLE_6 =
{
	10,
	{
		{ dump_RDD_SCHEDULING_QUEUE_DESCRIPTOR, set_RDD_SCHEDULING_QUEUE_DESCRIPTOR, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT US_TM_RATE_LIMITER_PROFILE_RESIDUE_TABLE_6 =
{
	1,
	{
		{ dump_RDD_RATE_LIMITER_PROFILE_RESIDUE, set_RDD_RATE_LIMITER_PROFILE_RESIDUE, 0xf20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT US_TM_TM_FLOW_CNTR_TABLE_6 =
{
	1,
	{
		{ dump_RDD_TM_FLOW_CNTR_ENTRY, set_RDD_TM_FLOW_CNTR_ENTRY, 0xf40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TX_TASK_US_1_STACK_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT US_TM_SCHEDULER_TABLE_6 =
{
	24,
	{
		{ dump_RDD_SCHEDULER_DESCRIPTOR, set_RDD_SCHEDULER_DESCRIPTOR, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_6 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x14c8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT US_TM_RATE_LIMITER_BUDGET_VALID_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x14d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x14e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT US_TM_PON_TM_FLOW_CNTR_TABLE_6 =
{
	1,
	{
		{ dump_RDD_TM_FLOW_CNTR_ENTRY, set_RDD_TM_FLOW_CNTR_ENTRY, 0x1500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT US_TM_BBH_QUEUE_TABLE_6 =
{
	8,
	{
		{ dump_RDD_BBH_QUEUE_DESCRIPTOR, set_RDD_BBH_QUEUE_DESCRIPTOR, 0x1600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SCORE_INTERRUPT_ID_DDR_ADDR_6 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, set_RDD_DDR_ADDRESS, 0x1768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT US_TM_DISPATCHER_CREDIT_TABLE_6 =
{
	12,
	{
		{ dump_RDD_DISPATCHER_CREDIT_DESCRIPTOR, set_RDD_DISPATCHER_CREDIT_DESCRIPTOR, 0x1770 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x177c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TX_TASK_PON_STACK_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x1780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT US_TM_RATE_LIMITER_BUDGET_DESCRIPTOR_TABLE_6 =
{
	8,
	{
		{ dump_RDD_RATE_LIMITER_BUDGET_DESCRIPTOR, set_RDD_RATE_LIMITER_BUDGET_DESCRIPTOR, 0x1800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT US_TM_RATE_LIMITER_PROFILE_TABLE_6 =
{
	4,
	{
		{ dump_RDD_RATE_LIMITER_PROFILE, set_RDD_RATE_LIMITER_PROFILE, 0x1a80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SCORE_SDR_DB_TABLE_6 =
{
	32,
	{
		{ dump_RDD_SCORE_SDR_DB_ENTRY, set_RDD_SCORE_SDR_DB_ENTRY, 0x1b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT US_TM_RATE_LIMITER_PARAMS_DESCRIPTOR_TABLE_6 =
{
	8,
	{
		{ dump_RDD_RATE_LIMITER_PARAMS_DESCRIPTOR, set_RDD_RATE_LIMITER_PARAMS_DESCRIPTOR, 0x1c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT US_TM_SCHEDULER_POOL_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x1e80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SCORE_CFG_TABLE_6 =
{
	8,
	{
		{ dump_RDD_SCORE_CFG_ENTRY, set_RDD_SCORE_CFG_ENTRY, 0x1fb8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT VPORT_TX_FLOW_TABLE_6 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0x1fc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_6 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT US_TM_TX_QUEUE_DROP_TABLE_6 =
{
	8,
	{
		{ dump_RDD_PACKETS_AND_BYTES, set_RDD_PACKETS_AND_BYTES, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT US_TM_DCL_IMG_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2480 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_6 =
{
	8,
	{
		{ dump_RDD_GENERAL_TIMER_ENTRY, set_RDD_GENERAL_TIMER_ENTRY, 0x2500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DATA_TRACE_SCRATCHPAD_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2580 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT US_TM_AQM_QUEUE_TABLE_6 =
{
	4,
	{
		{ dump_RDD_AQM_QUEUE_DESCRIPTOR, set_RDD_AQM_QUEUE_DESCRIPTOR, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT UPDATE_FIFO_STACK_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2740 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT US_TM_AQM_QUEUE_TIMER_TABLE_6 =
{
	1,
	{
		{ dump_RDD_AQM_QUEUE_TIMER_DESCRIPTOR, set_RDD_AQM_QUEUE_TIMER_DESCRIPTOR, 0x2780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DISPATCHER_CREDIT_TABLE_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x27d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x27dc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_6 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0x27e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_STACK_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2848 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT MIRRORING_SCRATCH_6 =
{
	8,
	{
		{ dump_RDD_BYTES_8, set_RDD_BYTES_8, 0x28c8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_6 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x28d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SCORE_QSTAT_SEQ_NUM_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x28dc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PON_TM_FW_FLUSH_TABLE_6 =
{
	28,
	{
		{ dump_RDD_FW_FLUSH_ENTRY, set_RDD_FW_FLUSH_ENTRY, 0x28e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT US_TM_DCL_MULTIPLE_CNTR_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x28fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT BUFFER_CONG_SCRATCHPAD_6 =
{
	8,
	{
		{ dump_RDD_BUFFER_CONG_Q_OCCUPANCY, set_RDD_BUFFER_CONG_Q_OCCUPANCY, 0x2900 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PON_TM_TX_TRUNCATE_MIRRORING_TABLE_6 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, set_RDD_MIRRORING_TRUNCATE_ENTRY, 0x2940 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT BUDGET_ALLOCATION_TIMER_VALUE_6 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x297e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT US_TM_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_TABLE_6 =
{
	40,
	{
		{ dump_RDD_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_ENTRY, set_RDD_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_ENTRY, 0x2980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SCORE_COALESCING_TABLE_6 =
{
	6,
	{
		{ dump_RDD_SCORE_COALESCING, set_RDD_SCORE_COALESCING, 0x29f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_ACTION_VEC_6 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x29fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT US_TM_BBH_TX_EGRESS_COUNTER_TABLE_6 =
{
	8,
	{
		{ dump_RDD_BBH_TX_EGRESS_COUNTER_ENTRY, set_RDD_BBH_TX_EGRESS_COUNTER_ENTRY, 0x2a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TASK_IDX_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2a38 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TX_EXCEPTION_6 =
{
	1,
	{
		{ dump_RDD_TX_EXCEPTION_ENTRY, set_RDD_TX_EXCEPTION_ENTRY, 0x2a3c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2a3d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT BBH_TX_US_WAN_0_FIFO_BYTES_THRESHOLD_6 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2a3e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT BUFFER_CONG_DQM_NOT_EMPTY_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2a40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT US_TM_FIRST_QUEUE_MAPPING_6 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2a74 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_6 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x2a76 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT US_TM_ETH_BBH_TX_FIFO_SIZE_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2a78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT US_TM_ETH_1_BBH_TX_FIFO_SIZE_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2a79 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT US_TM_PON_BBH_TX_FIFO_SIZE_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2a7a },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2a7b },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT WAN_0_BBH_TX_FIFO_SIZE_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2a7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2a7d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT US_BUFFER_CONG_MGT_CFG_6 =
{
	48,
	{
		{ dump_RDD_BUFFER_CONG_MGT, set_RDD_BUFFER_CONG_MGT, 0x2a80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_DISPATCHER_CREDIT_TABLE_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2ab0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT US_TM_PON_BBH_TX_ABS_COUNTER_TABLE_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2ac0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_6 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0x2af0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DATA_TRACE_STACK_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PON_TM_ENABLED_QUEUES_TABLE_6 =
{
	34,
	{
		{ dump_RDD_ENABLED_QUEUES_ENTRY, set_RDD_ENABLED_QUEUES_ENTRY, 0x2b40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT US_TM_DCL_DATA_BASE_6 =
{
	32,
	{
		{ dump_RDD_DCL_DATA, set_RDD_DCL_DATA, 0x2b80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT US_TM_BBH_TX_WAKE_UP_DATA_TABLE_6 =
{
	8,
	{
		{ dump_RDD_BBH_TX_WAKE_UP_DATA_ENTRY, set_RDD_BBH_TX_WAKE_UP_DATA_ENTRY, 0x2ba0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT UPDATE_FIFO_TABLE_6 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, set_RDD_UPDATE_FIFO_ENTRY, 0x2bc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT US_TM_SCHEDULING_AGGREGATION_CONTEXT_VECTOR_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2be0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DATA_TRACE_UPDATE_FIFO_TABLE_6 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, set_RDD_UPDATE_FIFO_ENTRY, 0x2c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT US_TM_PI2_PROBABILITY_CALC_DESCRIPTOR_6 =
{
	20,
	{
		{ dump_RDD_PI2_PROBABILITY_CALC_DESCRIPTOR, set_RDD_PI2_PROBABILITY_CALC_DESCRIPTOR, 0x2c20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DATA_TRACE_PD_FIFO_TABLE_6 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, set_RDD_PROCESSING_TX_DESCRIPTOR, 0x2c40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT US_TM_TX_OCTETS_COUNTERS_TABLE_6 =
{
	8,
	{
		{ dump_RDD_PACKETS_AND_BYTES, set_RDD_PACKETS_AND_BYTES, 0x2c60 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_6 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0x2ca0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_6 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0x2cc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2ce0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2d20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SCORE_MSG_RING_DESCRIPTOR_TABLE_6 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, set_RDD_CPU_RING_DESCRIPTOR, 0x2d40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SCORE_QSTAT_RING_DESCRIPTOR_TABLE_6 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, set_RDD_CPU_RING_DESCRIPTOR, 0x2d80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SCORE_BNL_RING_DESCRIPTOR_TABLE_6 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, set_RDD_CPU_RING_DESCRIPTOR, 0x2dc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT US_TM_BBH_TX_ABS_COUNTER_TABLE_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT US_TM_1_BBH_TX_ABS_COUNTER_TABLE_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2e40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DS_TM_PD_FIFO_TABLE_7 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, set_RDD_PROCESSING_TX_DESCRIPTOR, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DS_TM_SCHEDULING_QUEUE_TABLE_7 =
{
	10,
	{
		{ dump_RDD_SCHEDULING_QUEUE_DESCRIPTOR, set_RDD_SCHEDULING_QUEUE_DESCRIPTOR, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TX_TASK_DS_0_STACK_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xa80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PON_TX_FLOW_TABLE_7 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0xb00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DS_TM_RATE_LIMITER_BUDGET_DESCRIPTOR_TABLE_7 =
{
	8,
	{
		{ dump_RDD_RATE_LIMITER_BUDGET_DESCRIPTOR, set_RDD_RATE_LIMITER_BUDGET_DESCRIPTOR, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DS_TM_RATE_LIMITER_PARAMS_DESCRIPTOR_TABLE_7 =
{
	8,
	{
		{ dump_RDD_RATE_LIMITER_PARAMS_DESCRIPTOR, set_RDD_RATE_LIMITER_PARAMS_DESCRIPTOR, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING_4_TASKS_PACKET_BUFFER_7 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, set_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_7 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0x1800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_7 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, set_RDD_RX_FLOW_ENTRY, 0x1a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x1b94 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT NULL_BUFFER_7 =
{
	8,
	{
		{ dump_RDD_BYTES_8, set_RDD_BYTES_8, 0x1b98 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DS_TM_RATE_LIMITER_PROFILE_RESIDUE_TABLE_7 =
{
	1,
	{
		{ dump_RDD_RATE_LIMITER_PROFILE_RESIDUE, set_RDD_RATE_LIMITER_PROFILE_RESIDUE, 0x1ba0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DS_TM_TM_FLOW_CNTR_TABLE_7 =
{
	1,
	{
		{ dump_RDD_TM_FLOW_CNTR_ENTRY, set_RDD_TM_FLOW_CNTR_ENTRY, 0x1bc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x1c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_7 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x1d68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DS_TM_RATE_LIMITER_BUDGET_VALID_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x1d70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TX_TASK_DS_1_STACK_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x1d80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x1e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SCORE_CFG_TABLE_7 =
{
	8,
	{
		{ dump_RDD_SCORE_CFG_ENTRY, set_RDD_SCORE_CFG_ENTRY, 0x1f68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DS_TM_BBH_TX_WAKE_UP_DATA_TABLE_7 =
{
	8,
	{
		{ dump_RDD_BBH_TX_WAKE_UP_DATA_ENTRY, set_RDD_BBH_TX_WAKE_UP_DATA_ENTRY, 0x1f70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DS_TM_RATE_LIMITER_PROFILE_TABLE_7 =
{
	4,
	{
		{ dump_RDD_RATE_LIMITER_PROFILE, set_RDD_RATE_LIMITER_PROFILE, 0x1f80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT MIRRORING_SCRATCH_7 =
{
	8,
	{
		{ dump_RDD_BYTES_8, set_RDD_BYTES_8, 0x2168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_7 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, set_RDD_SPDTEST_GEN_CFG, 0x2170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DS_TM_DCL_IMG_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_7 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, set_RDD_TUNNELS_PARSING_CFG, 0x2368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_7 =
{
	8,
	{
		{ dump_RDD_GENERAL_TIMER_ENTRY, set_RDD_GENERAL_TIMER_ENTRY, 0x2380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DS_TM_SCHEDULER_TABLE_7 =
{
	24,
	{
		{ dump_RDD_SCHEDULER_DESCRIPTOR, set_RDD_SCHEDULER_DESCRIPTOR, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DS_TM_TX_QUEUE_DROP_TABLE_7 =
{
	8,
	{
		{ dump_RDD_PACKETS_AND_BYTES, set_RDD_PACKETS_AND_BYTES, 0x2508 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2708 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CSO_BAD_IPV4_HDR_CSUM_PACKETS_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x270c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_7 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x2710 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SCORE_COLLECT_DROP_BITMAP_TABLE_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x271c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT AQM_ENABLE_TABLE_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2720 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT VPORT_TX_FLOW_TABLE_7 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0x2740 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SCORE_DATA_COLLECTION_PROFILE_TABLE_7 =
{
	16,
	{
		{ dump_RDD_SCORE_DATA_COLLECTION_PROFILE, set_RDD_SCORE_DATA_COLLECTION_PROFILE, 0x2780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DS_TM_AQM_QUEUE_TABLE_7 =
{
	4,
	{
		{ dump_RDD_AQM_QUEUE_DESCRIPTOR, set_RDD_AQM_QUEUE_DESCRIPTOR, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_7 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, set_RDD_POLICER_PARAMS_ENTRY, 0x2900 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x29a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DS_TM_AQM_QUEUE_TIMER_TABLE_7 =
{
	1,
	{
		{ dump_RDD_AQM_QUEUE_TIMER_DESCRIPTOR, set_RDD_AQM_QUEUE_TIMER_DESCRIPTOR, 0x29c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_7 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, set_RDD_INGRESS_FILTER_CTRL, 0x2a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_7 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, set_RDD_VPORT_CFG_ENTRY, 0x2a80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DS_TM_DCL_MULTIPLE_CNTR_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2afc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DS_TM_SECONDARY_SCHEDULER_TABLE_7 =
{
	12,
	{
		{ dump_RDD_SECONDARY_SCHEDULER_DESCRIPTOR, set_RDD_SECONDARY_SCHEDULER_DESCRIPTOR, 0x2b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DS_TM_SCHEDULER_POOL_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2b60 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_7 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0x2c20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT UPDATE_FIFO_STACK_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2c40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_STACK_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2c80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2cc8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_7 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, set_RDD_IPTV_CFG, 0x2d48 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2d50 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TASK_IDX_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2d5c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_7 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, set_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0x2d60 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT BUFFER_CONG_SCRATCHPAD_7 =
{
	8,
	{
		{ dump_RDD_BUFFER_CONG_Q_OCCUPANCY, set_RDD_BUFFER_CONG_Q_OCCUPANCY, 0x2d80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_TABLE_7 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, set_RDD_MIRRORING_TRUNCATE_ENTRY, 0x2dc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT AQM_NUM_QUEUES_7 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2dfe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DS_TM_BBH_QUEUE_TABLE_7 =
{
	8,
	{
		{ dump_RDD_BBH_QUEUE_DESCRIPTOR, set_RDD_BBH_QUEUE_DESCRIPTOR, 0x2e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_7 =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, set_RDD_BUFMNG_STATUS_ENTRY, 0x2e60 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT HOST_TX_TRUNCATE_MIRRORING_TABLE_7 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, set_RDD_MIRRORING_TRUNCATE_ENTRY, 0x2e80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT BUDGET_ALLOCATION_TIMER_VALUE_7 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2ebe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT ETH_TM_TX_TRUNCATE_MIRRORING_TABLE_7 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, set_RDD_MIRRORING_TRUNCATE_ENTRY, 0x2ec0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_7 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2efe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_7 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2f00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_ACTION_VEC_7 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2f3e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_7 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, set_RDD_VPORT_CFG_EX_ENTRY, 0x2f40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_7 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2f7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT BUFFER_CONG_DQM_NOT_EMPTY_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2f80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2fb4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TX_EXCEPTION_7 =
{
	1,
	{
		{ dump_RDD_TX_EXCEPTION_ENTRY, set_RDD_TX_EXCEPTION_ENTRY, 0x2fb5 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2fb6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2fb7 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_7 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, set_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x2fb8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_TABLE_7 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, set_RDD_TR471_SPDSVC_RX_PKT_ID, 0x2fc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_DISPATCHER_CREDIT_TABLE_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2ff0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DS_TM_FIRST_QUEUE_MAPPING_7 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2ffc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2ffe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT CSO_DISABLE_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2fff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DS_BUFFER_CONG_MGT_CFG_7 =
{
	48,
	{
		{ dump_RDD_BUFFER_CONG_MGT, set_RDD_BUFFER_CONG_MGT, 0x3000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_7 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0x3030 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DS_TM_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_TABLE_7 =
{
	40,
	{
		{ dump_RDD_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_ENTRY, set_RDD_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_ENTRY, 0x3040 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_7 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, set_RDD_TCAM_GENERIC, 0x3090 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT ETH_TM_FW_FLUSH_TABLE_7 =
{
	28,
	{
		{ dump_RDD_FW_FLUSH_ENTRY, set_RDD_FW_FLUSH_ENTRY, 0x30a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_7 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x30bc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_7 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x30be },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT ETH_TM_ENABLED_QUEUES_TABLE_7 =
{
	34,
	{
		{ dump_RDD_ENABLED_QUEUES_ENTRY, set_RDD_ENABLED_QUEUES_ENTRY, 0x30c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x30e2 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x30e3 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_7 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, set_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x30e4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x30e5 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x30e6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_7 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, set_RDD_NAT_CACHE_CFG, 0x30e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_7 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, set_RDD_GDX_PARAMS, 0x30f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DS_TM_DCL_DATA_BASE_7 =
{
	32,
	{
		{ dump_RDD_DCL_DATA, set_RDD_DCL_DATA, 0x3100 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DS_TM_SCHEDULING_AGGREGATION_CONTEXT_VECTOR_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3120 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3138 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT UPDATE_FIFO_TABLE_7 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, set_RDD_UPDATE_FIFO_ENTRY, 0x3140 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DS_TM_PI2_PROBABILITY_CALC_DESCRIPTOR_7 =
{
	20,
	{
		{ dump_RDD_PI2_PROBABILITY_CALC_DESCRIPTOR, set_RDD_PI2_PROBABILITY_CALC_DESCRIPTOR, 0x3160 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_7 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, set_RDD_TCAM_TABLE_CFG, 0x3178 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_7 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0x3180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_7 =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, set_RDD_DHD_HW_CONFIGURATION, 0x31a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_7 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x31b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x31c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x31e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_7 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, set_RDD_INGRESS_FILTER_CFG, 0x3210 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3218 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_7 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0x3220 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3240 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3260 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_7 =
{
	8,
	{
		{ dump_RDD_BBH_TX_EGRESS_COUNTER_ENTRY, set_RDD_BBH_TX_EGRESS_COUNTER_ENTRY, 0x3280 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_7 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, set_RDD_MULTICAST_KEY_MASK_ENTRY, 0x32a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DS_TM_BBH_TX_ABS_COUNTER_TABLE_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x32c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6813
static DUMP_RUNNERREG_STRUCT DS_TM_1_BBH_TX_ABS_COUNTER_TABLE_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3300 },
		{ 0, 0 },
	}
};
#endif

TABLE_STRUCT RUNNER_TABLES[NUMBER_OF_TABLES] =
{
#if defined BCM6813
	{ "SQ_TM_PD_FIFO_TABLE", 1, CORE_0_INDEX, &SQ_TM_PD_FIFO_TABLE, 64, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_COMPLETE_COMMON_RADIO_DATA", 1, CORE_0_INDEX, &DHD_COMPLETE_COMMON_RADIO_DATA, 3, 1, 1 },
#endif
#if defined BCM6813
	{ "VPORT_TX_FLOW_TABLE", 1, CORE_0_INDEX, &VPORT_TX_FLOW_TABLE, 64, 1, 1 },
#endif
#if defined BCM6813
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_0_INDEX, &RUNNER_PROFILING_TRACE_BUFFER, 128, 1, 1 },
#endif
#if defined BCM6813
	{ "RX_FLOW_TABLE", 1, CORE_0_INDEX, &RX_FLOW_TABLE, 202, 1, 1 },
#endif
#if defined BCM6813
	{ "AQM_SQ_BITMAP", 1, CORE_0_INDEX, &AQM_SQ_BITMAP, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SERVICE_QUEUES_SCRATCHPAD", 1, CORE_0_INDEX, &SERVICE_QUEUES_SCRATCHPAD, 2, 1, 1 },
#endif
#if defined BCM6813
	{ "SQ_TM_RATE_LIMITER_PROFILE_RESIDUE_TABLE", 1, CORE_0_INDEX, &SQ_TM_RATE_LIMITER_PROFILE_RESIDUE_TABLE, 32, 1, 1 },
#endif
#if defined BCM6813
	{ "AQM_SQ_TABLE", 1, CORE_0_INDEX, &AQM_SQ_TABLE, 32, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING0_STACK", 1, CORE_0_INDEX, &PROCESSING0_STACK, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "NULL_BUFFER", 1, CORE_0_INDEX, &NULL_BUFFER, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SQ_TM_RATE_LIMITER_BUDGET_VALID", 1, CORE_0_INDEX, &SQ_TM_RATE_LIMITER_BUDGET_VALID, 4, 1, 1 },
#endif
#if defined BCM6813
	{ "SQ_TM_RATE_LIMITER_PROFILE_TABLE", 1, CORE_0_INDEX, &SQ_TM_RATE_LIMITER_PROFILE_TABLE, 32, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING1_STACK", 1, CORE_0_INDEX, &PROCESSING1_STACK, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "SYSTEM_CONFIGURATION", 1, CORE_0_INDEX, &SYSTEM_CONFIGURATION, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SPDTEST_GEN_PARAM", 1, CORE_0_INDEX, &SPDTEST_GEN_PARAM, 4, 1, 1 },
#endif
#if defined BCM6813
	{ "SERVICE_QUEUES_STACK", 1, CORE_0_INDEX, &SERVICE_QUEUES_STACK, 128, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_0_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING2_STACK", 1, CORE_0_INDEX, &PROCESSING2_STACK, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "SCORE_CFG_TABLE", 1, CORE_0_INDEX, &SCORE_CFG_TABLE, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_0_INDEX, &INGRESS_FILTER_L2_REASON_TABLE, 16, 1, 1 },
#endif
#if defined BCM6813
	{ "SQ_TM_DCL_IMG", 1, CORE_0_INDEX, &SQ_TM_DCL_IMG, 128, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING3_STACK", 1, CORE_0_INDEX, &PROCESSING3_STACK, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR", 1, CORE_0_INDEX, &DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SQ_TM_SECONDARY_SCHEDULER_TABLE", 1, CORE_0_INDEX, &SQ_TM_SECONDARY_SCHEDULER_TABLE, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "MUTEX_TABLE", 1, CORE_0_INDEX, &MUTEX_TABLE, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "GENERAL_TIMER", 1, CORE_0_INDEX, &GENERAL_TIMER, 16, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING4_STACK", 1, CORE_0_INDEX, &PROCESSING4_STACK, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_RX_POST_RING_SIZE", 1, CORE_0_INDEX, &DHD_RX_POST_RING_SIZE, 3, 1, 1 },
#endif
#if defined BCM6813
	{ "AQM_NUM_QUEUES", 1, CORE_0_INDEX, &AQM_NUM_QUEUES, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SQ_TM_DISPATCHER_CREDIT_TABLE", 1, CORE_0_INDEX, &SQ_TM_DISPATCHER_CREDIT_TABLE, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_0_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SCORE_DATA_COLLECTION_PROFILE_TABLE", 1, CORE_0_INDEX, &SCORE_DATA_COLLECTION_PROFILE_TABLE, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING5_STACK", 1, CORE_0_INDEX, &PROCESSING5_STACK, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_RX_COMPLETE_RING_SIZE", 1, CORE_0_INDEX, &DHD_RX_COMPLETE_RING_SIZE, 3, 1, 1 },
#endif
#if defined BCM6813
	{ "DOS_DROP_REASONS_CFG", 1, CORE_0_INDEX, &DOS_DROP_REASONS_CFG, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SERVICE_QUEUES_AQM_TIMER_DISPATCHER_CREDIT_TABLE", 1, CORE_0_INDEX, &SERVICE_QUEUES_AQM_TIMER_DISPATCHER_CREDIT_TABLE, 3, 1, 1 },
#endif
#if defined BCM6813
	{ "CSO_BAD_IPV4_HDR_CSUM_PACKETS", 1, CORE_0_INDEX, &CSO_BAD_IPV4_HDR_CSUM_PACKETS, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_RX_COMPLETE_0_STACK", 1, CORE_0_INDEX, &DHD_RX_COMPLETE_0_STACK, 128, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING6_STACK", 1, CORE_0_INDEX, &PROCESSING6_STACK, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_TX_COMPLETE_RING_SIZE", 1, CORE_0_INDEX, &DHD_TX_COMPLETE_RING_SIZE, 3, 1, 1 },
#endif
#if defined BCM6813
	{ "GENERAL_TIMER_ACTION_VEC", 1, CORE_0_INDEX, &GENERAL_TIMER_ACTION_VEC, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_0_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SCORE_COLLECT_DROP_BITMAP_TABLE", 1, CORE_0_INDEX, &SCORE_COLLECT_DROP_BITMAP_TABLE, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_RX_COMPLETE_1_STACK", 1, CORE_0_INDEX, &DHD_RX_COMPLETE_1_STACK, 128, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING7_STACK", 1, CORE_0_INDEX, &PROCESSING7_STACK, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "TUNNELS_PARSING_CFG", 1, CORE_0_INDEX, &TUNNELS_PARSING_CFG, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_FPM_THRESHOLDS", 1, CORE_0_INDEX, &DHD_FPM_THRESHOLDS, 3, 1, 1 },
#endif
#if defined BCM6813
	{ "SQ_TM_UPDATE_FIFO_START_INDEX", 1, CORE_0_INDEX, &SQ_TM_UPDATE_FIFO_START_INDEX, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_RX_COMPLETE_2_STACK", 1, CORE_0_INDEX, &DHD_RX_COMPLETE_2_STACK, 128, 1, 1 },
#endif
#if defined BCM6813
	{ "SQ_TM_SCHEDULING_QUEUE_TABLE", 1, CORE_0_INDEX, &SQ_TM_SCHEDULING_QUEUE_TABLE, 32, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_TX_COMPLETE_0_STACK", 1, CORE_0_INDEX, &DHD_TX_COMPLETE_0_STACK, 64, 1, 1 },
#endif
#if defined BCM6813
	{ "VPORT_CFG_TABLE", 1, CORE_0_INDEX, &VPORT_CFG_TABLE, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "TASK_IDX", 1, CORE_0_INDEX, &TASK_IDX, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SQ_TM_RATE_LIMITER_BUDGET_DESCRIPTOR_TABLE", 1, CORE_0_INDEX, &SQ_TM_RATE_LIMITER_BUDGET_DESCRIPTOR_TABLE, 32, 1, 1 },
#endif
#if defined BCM6813
	{ "SQ_TM_RATE_LIMITER_PARAMS_DESCRIPTOR_TABLE", 1, CORE_0_INDEX, &SQ_TM_RATE_LIMITER_PARAMS_DESCRIPTOR_TABLE, 32, 1, 1 },
#endif
#if defined BCM6813
	{ "PON_TX_FLOW_TABLE", 1, CORE_0_INDEX, &PON_TX_FLOW_TABLE, 256, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_MONITOR_PKT_INFO_TABLE", 1, CORE_0_INDEX, &DHD_MONITOR_PKT_INFO_TABLE, 4, 1, 1 },
#endif
#if defined BCM6813
	{ "AQM_ENABLE_TABLE", 1, CORE_0_INDEX, &AQM_ENABLE_TABLE, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "POLICER_PARAMS_TABLE", 1, CORE_0_INDEX, &POLICER_PARAMS_TABLE, 80, 1, 1 },
#endif
#if defined BCM6813
	{ "SQ_TM_TX_QUEUE_DROP_TABLE", 1, CORE_0_INDEX, &SQ_TM_TX_QUEUE_DROP_TABLE, 32, 1, 1 },
#endif
#if defined BCM6813
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_0_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_TX_COMPLETE_1_STACK", 1, CORE_0_INDEX, &DHD_TX_COMPLETE_1_STACK, 64, 1, 1 },
#endif
#if defined BCM6813
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_0_INDEX, &INGRESS_FILTER_PROFILE_TABLE, 16, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_RX_COMPLETE_FLOW_RING_BUFFER", 1, CORE_0_INDEX, &DHD_RX_COMPLETE_FLOW_RING_BUFFER, 3, 1, 1 },
#endif
#if defined BCM6813
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_0_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE, 32, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_RX_POST_FLOW_RING_BUFFER", 1, CORE_0_INDEX, &DHD_RX_POST_FLOW_RING_BUFFER, 3, 1, 1 },
#endif
#if defined BCM6813
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_0_INDEX, &BUFMNG_DESCRIPTOR_TABLE, 16, 1, 1 },
#endif
#if defined BCM6813
	{ "GENERAL_TIMER_STACK", 1, CORE_0_INDEX, &GENERAL_TIMER_STACK, 72, 1, 1 },
#endif
#if defined BCM6813
	{ "SQ_TM_AQM_QUEUE_TABLE", 1, CORE_0_INDEX, &SQ_TM_AQM_QUEUE_TABLE, 32, 1, 1 },
#endif
#if defined BCM6813
	{ "REGISTERS_BUFFER", 1, CORE_0_INDEX, &REGISTERS_BUFFER, 32, 1, 1 },
#endif
#if defined BCM6813
	{ "IPTV_CFG_TABLE", 1, CORE_0_INDEX, &IPTV_CFG_TABLE, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_DOORBELL_TX_COMPLETE_VALUE", 1, CORE_0_INDEX, &DHD_DOORBELL_TX_COMPLETE_VALUE, 3, 1, 1 },
#endif
#if defined BCM6813
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_0_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SQ_TM_FIRST_QUEUE_MAPPING", 1, CORE_0_INDEX, &SQ_TM_FIRST_QUEUE_MAPPING, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "BUFMNG_STATUS_TABLE", 1, CORE_0_INDEX, &BUFMNG_STATUS_TABLE, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_TX_COMPLETE_2_STACK", 1, CORE_0_INDEX, &DHD_TX_COMPLETE_2_STACK, 64, 1, 1 },
#endif
#if defined BCM6813
	{ "BUFFER_CONG_SCRATCHPAD", 1, CORE_0_INDEX, &BUFFER_CONG_SCRATCHPAD, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "RX_MIRRORING_TABLE", 1, CORE_0_INDEX, &RX_MIRRORING_TABLE, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_0_INDEX, &SPDTEST_NUM_OF_RX_FLOWS, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_0_INDEX, &LOOPBACK_WAN_FLOW_TABLE, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "HOST_TX_TRUNCATE_MIRRORING_TABLE", 1, CORE_0_INDEX, &HOST_TX_TRUNCATE_MIRRORING_TABLE, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "FORCE_DSCP", 1, CORE_0_INDEX, &FORCE_DSCP, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "CORE_ID_TABLE", 1, CORE_0_INDEX, &CORE_ID_TABLE, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_0_INDEX, &LOOPBACK_QUEUE_TABLE, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_0_INDEX, &RX_MIRRORING_CONFIGURATION, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "VPORT_CFG_EX_TABLE", 1, CORE_0_INDEX, &VPORT_CFG_EX_TABLE, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_0_INDEX, &TX_MIRRORING_CONFIGURATION, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "BUFFER_CONG_DQM_NOT_EMPTY", 1, CORE_0_INDEX, &BUFFER_CONG_DQM_NOT_EMPTY, 13, 1, 1 },
#endif
#if defined BCM6813
	{ "CSO_DISABLE", 1, CORE_0_INDEX, &CSO_DISABLE, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_0_INDEX, &DEBUG_PRINT_CORE_LOCK, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SRAM_DUMMY_STORE", 1, CORE_0_INDEX, &SRAM_DUMMY_STORE, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_0_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_0_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "TR471_SPDSVC_RX_PKT_ID_TABLE", 1, CORE_0_INDEX, &TR471_SPDSVC_RX_PKT_ID_TABLE, 4, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_DOORBELL_RX_COMPLETE_VALUE", 1, CORE_0_INDEX, &DHD_DOORBELL_RX_COMPLETE_VALUE, 3, 1, 1 },
#endif
#if defined BCM6813
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_0_INDEX, &INGRESS_FILTER_1588_CFG, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_0_INDEX, &RX_MIRRORING_DIRECT_ENABLE, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_PCAP_TX_RING_DESCRIPTOR_TABLE", 1, CORE_0_INDEX, &DHD_PCAP_TX_RING_DESCRIPTOR_TABLE, 3, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_DOORBELL_RX_POST_VALUE", 1, CORE_0_INDEX, &DHD_DOORBELL_RX_POST_VALUE, 3, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_TX_COMPLETE_FLOW_RING_BUFFER", 1, CORE_0_INDEX, &DHD_TX_COMPLETE_FLOW_RING_BUFFER, 3, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_CPU_INT_ID", 1, CORE_0_INDEX, &DHD_CPU_INT_ID, 3, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_PCAP_RX_RING_DESCRIPTOR_TABLE", 1, CORE_0_INDEX, &DHD_PCAP_RX_RING_DESCRIPTOR_TABLE, 3, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_PCAP_CFG", 1, CORE_0_INDEX, &DHD_PCAP_CFG, 3, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_RX_COMPLETE_DISPATCHER_CREDIT_TABLE", 1, CORE_0_INDEX, &DHD_RX_COMPLETE_DISPATCHER_CREDIT_TABLE, 12, 1, 1 },
#endif
#if defined BCM6813
	{ "SPDSVC_WLAN_GEN_PARAMS_TABLE", 1, CORE_0_INDEX, &SPDSVC_WLAN_GEN_PARAMS_TABLE, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE", 1, CORE_0_INDEX, &DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE, 3, 1, 1 },
#endif
#if defined BCM6813
	{ "DEBUG_PRINT_TABLE", 1, CORE_0_INDEX, &DEBUG_PRINT_TABLE, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_TX_COMPLETE_DISPATCHER_CREDIT_TABLE", 1, CORE_0_INDEX, &DHD_TX_COMPLETE_DISPATCHER_CREDIT_TABLE, 12, 1, 1 },
#endif
#if defined BCM6813
	{ "TCAM_GENERIC_FIELDS", 1, CORE_0_INDEX, &TCAM_GENERIC_FIELDS, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "SQ_BUFFER_CONG_MGT_CFG", 1, CORE_0_INDEX, &SQ_BUFFER_CONG_MGT_CFG, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "GDX_PARAMS_TABLE", 1, CORE_0_INDEX, &GDX_PARAMS_TABLE, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SQ_TM_ENABLED_QUEUES_TABLE", 1, CORE_0_INDEX, &SQ_TM_ENABLED_QUEUES_TABLE, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SQ_TM_SCHEDULER_POOL", 1, CORE_0_INDEX, &SQ_TM_SCHEDULER_POOL, 64, 1, 1 },
#endif
#if defined BCM6813
	{ "NAT_CACHE_CFG", 1, CORE_0_INDEX, &NAT_CACHE_CFG, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_0_INDEX, &NAT_CACHE_KEY0_MASK, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_0_INDEX, &TCAM_TABLE_CFG_TABLE, 2, 1, 1 },
#endif
#if defined BCM6813
	{ "SQ_TM_UPDATE_FIFO_TABLE", 1, CORE_0_INDEX, &SQ_TM_UPDATE_FIFO_TABLE, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "SQ_TM_BUFMNG_STATUS_TABLE", 1, CORE_0_INDEX, &SQ_TM_BUFMNG_STATUS_TABLE, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SQ_TM_DCL_DATA_BASE", 1, CORE_0_INDEX, &SQ_TM_DCL_DATA_BASE, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SQ_TM_FW_FLUSH_TABLE", 1, CORE_0_INDEX, &SQ_TM_FW_FLUSH_TABLE, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_0_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "SQ_TM_SCHEDULER_TABLE", 1, CORE_0_INDEX, &SQ_TM_SCHEDULER_TABLE, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_0_INDEX, &NATC_L2_VLAN_KEY_MASK, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_0_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "DEBUG_SCRATCHPAD", 1, CORE_0_INDEX, &DEBUG_SCRATCHPAD, 12, 1, 1 },
#endif
#if defined BCM6813
	{ "INGRESS_FILTER_CFG", 1, CORE_0_INDEX, &INGRESS_FILTER_CFG, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "NATC_L2_TOS_MASK", 1, CORE_0_INDEX, &NATC_L2_TOS_MASK, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_HW_CFG", 1, CORE_0_INDEX, &DHD_HW_CFG, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_FPM_REPLY", 1, CORE_0_INDEX, &DHD_FPM_REPLY, 24, 1, 1 },
#endif
#if defined BCM6813
	{ "SQ_TM_AQM_QUEUE_TIMER_TABLE", 1, CORE_0_INDEX, &SQ_TM_AQM_QUEUE_TIMER_TABLE, 32, 1, 1 },
#endif
#if defined BCM6813
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_0_INDEX, &BITS_CALC_MASKS_TABLE, 4, 1, 1 },
#endif
#if defined BCM6813
	{ "FPM_GLOBAL_CFG", 1, CORE_0_INDEX, &FPM_GLOBAL_CFG, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SQ_TM_PI2_PROBABILITY_CALC_DESCRIPTOR", 1, CORE_0_INDEX, &SQ_TM_PI2_PROBABILITY_CALC_DESCRIPTOR, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_0_INDEX, &FW_ERROR_VECTOR_TABLE, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "MULTICAST_KEY_MASK", 1, CORE_0_INDEX, &MULTICAST_KEY_MASK, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "TCPSPDTEST_STREAM_TABLE", 1, CORE_1_INDEX, &TCPSPDTEST_STREAM_TABLE_1, 4, 1, 1 },
#endif
#if defined BCM6813
	{ "AQM_ENABLE_TABLE", 1, CORE_1_INDEX, &AQM_ENABLE_TABLE_1, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "RX_FLOW_TABLE", 1, CORE_1_INDEX, &RX_FLOW_TABLE_1, 202, 1, 1 },
#endif
#if defined BCM6813
	{ "PKTGEN_NO_SBPM_HDRS_CNTR", 1, CORE_1_INDEX, &PKTGEN_NO_SBPM_HDRS_CNTR_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_RX_INTERRUPT_ID_DDR_ADDR", 1, CORE_1_INDEX, &CPU_RX_INTERRUPT_ID_DDR_ADDR_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_1_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_1, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "VPORT_TX_FLOW_TABLE", 1, CORE_1_INDEX, &VPORT_TX_FLOW_TABLE_1, 64, 1, 1 },
#endif
#if defined BCM6813
	{ "PKTGEN_TX_STREAM_TABLE", 1, CORE_1_INDEX, &PKTGEN_TX_STREAM_TABLE_1, 4, 1, 1 },
#endif
#if defined BCM6813
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_1_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_1, 32, 1, 1 },
#endif
#if defined BCM6813
	{ "COMMON_REPROCESSING_STACK", 1, CORE_1_INDEX, &COMMON_REPROCESSING_STACK_1, 64, 1, 1 },
#endif
#if defined BCM6813
	{ "TCPSPDTEST_SCRATCHPAD", 1, CORE_1_INDEX, &TCPSPDTEST_SCRATCHPAD_1, 16, 1, 1 },
#endif
#if defined BCM6813
	{ "PON_TX_FLOW_TABLE", 1, CORE_1_INDEX, &PON_TX_FLOW_TABLE_1, 256, 1, 1 },
#endif
#if defined BCM6813
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_1_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_1, 128, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING0_STACK", 1, CORE_1_INDEX, &PROCESSING0_STACK_1, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "NULL_BUFFER", 1, CORE_1_INDEX, &NULL_BUFFER_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SPDTEST_GEN_PARAM", 1, CORE_1_INDEX, &SPDTEST_GEN_PARAM_1, 4, 1, 1 },
#endif
#if defined BCM6813
	{ "GENERAL_TIMER", 1, CORE_1_INDEX, &GENERAL_TIMER_1, 16, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_1_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER_1, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING1_STACK", 1, CORE_1_INDEX, &PROCESSING1_STACK_1, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "SYSTEM_CONFIGURATION", 1, CORE_1_INDEX, &SYSTEM_CONFIGURATION_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_1_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM6813
	{ "SPDSVC_GEN_STACK", 1, CORE_1_INDEX, &SPDSVC_GEN_STACK_1, 128, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING2_STACK", 1, CORE_1_INDEX, &PROCESSING2_STACK_1, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "TX_ABS_RECYCLE_COUNTERS", 1, CORE_1_INDEX, &TX_ABS_RECYCLE_COUNTERS_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SPDSVC_GEN_DISPATCHER_CREDIT_TABLE", 1, CORE_1_INDEX, &SPDSVC_GEN_DISPATCHER_CREDIT_TABLE_1, 3, 1, 1 },
#endif
#if defined BCM6813
	{ "MUTEX_TABLE", 1, CORE_1_INDEX, &MUTEX_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SCORE_DATA_COLLECTION_PROFILE_TABLE", 1, CORE_1_INDEX, &SCORE_DATA_COLLECTION_PROFILE_TABLE_1, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING3_STACK", 1, CORE_1_INDEX, &PROCESSING3_STACK_1, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "SCORE_CFG_TABLE", 1, CORE_1_INDEX, &SCORE_CFG_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "TCPSPDTEST_DISPATCHER_CREDIT_TABLE", 1, CORE_1_INDEX, &TCPSPDTEST_DISPATCHER_CREDIT_TABLE_1, 3, 1, 1 },
#endif
#if defined BCM6813
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_1_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "VPORT_CFG_TABLE", 1, CORE_1_INDEX, &VPORT_CFG_TABLE_1, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "CSO_BAD_IPV4_HDR_CSUM_PACKETS", 1, CORE_1_INDEX, &CSO_BAD_IPV4_HDR_CSUM_PACKETS_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING4_STACK", 1, CORE_1_INDEX, &PROCESSING4_STACK_1, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "PKTGEN_CURR_SBPM_HDR_PTR", 1, CORE_1_INDEX, &PKTGEN_CURR_SBPM_HDR_PTR_1, 4, 1, 1 },
#endif
#if defined BCM6813
	{ "COMMON_REPROCESSING_DISPATCHER_CREDIT_TABLE", 1, CORE_1_INDEX, &COMMON_REPROCESSING_DISPATCHER_CREDIT_TABLE_1, 3, 1, 1 },
#endif
#if defined BCM6813
	{ "SCORE_COLLECT_DROP_BITMAP_TABLE", 1, CORE_1_INDEX, &SCORE_COLLECT_DROP_BITMAP_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "GENERAL_TIMER_STACK", 1, CORE_1_INDEX, &GENERAL_TIMER_STACK_1, 72, 1, 1 },
#endif
#if defined BCM6813
	{ "PKTGEN_NUM_OF_AVAIL_SBPM_HDRS", 1, CORE_1_INDEX, &PKTGEN_NUM_OF_AVAIL_SBPM_HDRS_1, 4, 1, 1 },
#endif
#if defined BCM6813
	{ "TCPSPDTEST_GEN_DISPATCHER_CREDIT_TABLE", 1, CORE_1_INDEX, &TCPSPDTEST_GEN_DISPATCHER_CREDIT_TABLE_1, 3, 1, 1 },
#endif
#if defined BCM6813
	{ "PKTGEN_BAD_GET_NEXT", 1, CORE_1_INDEX, &PKTGEN_BAD_GET_NEXT_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_1_INDEX, &BUFMNG_DESCRIPTOR_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING5_STACK", 1, CORE_1_INDEX, &PROCESSING5_STACK_1, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "PKTGEN_SBPM_END_PTR", 1, CORE_1_INDEX, &PKTGEN_SBPM_END_PTR_1, 4, 1, 1 },
#endif
#if defined BCM6813
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_1_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "PKTGEN_MAX_UT_PKTS", 1, CORE_1_INDEX, &PKTGEN_MAX_UT_PKTS_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "COMMON_REPROCESSING_PD_FIFO_TABLE", 1, CORE_1_INDEX, &COMMON_REPROCESSING_PD_FIFO_TABLE_1, 4, 1, 1 },
#endif
#if defined BCM6813
	{ "RX_MIRRORING_TABLE", 1, CORE_1_INDEX, &RX_MIRRORING_TABLE_1, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "AQM_NUM_QUEUES", 1, CORE_1_INDEX, &AQM_NUM_QUEUES_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING6_STACK", 1, CORE_1_INDEX, &PROCESSING6_STACK_1, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "TUNNELS_PARSING_CFG", 1, CORE_1_INDEX, &TUNNELS_PARSING_CFG_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_FPM_THRESHOLDS", 1, CORE_1_INDEX, &DHD_FPM_THRESHOLDS_1, 3, 1, 1 },
#endif
#if defined BCM6813
	{ "PKTGEN_UT_TRIGGER", 1, CORE_1_INDEX, &PKTGEN_UT_TRIGGER_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "TCPSPDTEST_PD_FIFO_TABLE", 1, CORE_1_INDEX, &TCPSPDTEST_PD_FIFO_TABLE_1, 4, 1, 1 },
#endif
#if defined BCM6813
	{ "HOST_TX_TRUNCATE_MIRRORING_TABLE", 1, CORE_1_INDEX, &HOST_TX_TRUNCATE_MIRRORING_TABLE_1, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "DOS_DROP_REASONS_CFG", 1, CORE_1_INDEX, &DOS_DROP_REASONS_CFG_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING7_STACK", 1, CORE_1_INDEX, &PROCESSING7_STACK_1, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "PKTGEN_SBPM_EXTS", 1, CORE_1_INDEX, &PKTGEN_SBPM_EXTS_1, 4, 1, 1 },
#endif
#if defined BCM6813
	{ "IPTV_CFG_TABLE", 1, CORE_1_INDEX, &IPTV_CFG_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "PKTGEN_SBPM_HDR_BNS", 1, CORE_1_INDEX, &PKTGEN_SBPM_HDR_BNS_1, 32, 1, 1 },
#endif
#if defined BCM6813
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_1_INDEX, &LOOPBACK_QUEUE_TABLE_1, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "GENERAL_TIMER_ACTION_VEC", 1, CORE_1_INDEX, &GENERAL_TIMER_ACTION_VEC_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "UDPSPDT_STREAM_TX_STAT_TABLE", 1, CORE_1_INDEX, &UDPSPDT_STREAM_TX_STAT_TABLE_1, 4, 1, 1 },
#endif
#if defined BCM6813
	{ "BUFMNG_STATUS_TABLE", 1, CORE_1_INDEX, &BUFMNG_STATUS_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "VPORT_CFG_EX_TABLE", 1, CORE_1_INDEX, &VPORT_CFG_EX_TABLE_1, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_1_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "POLICER_PARAMS_TABLE", 1, CORE_1_INDEX, &POLICER_PARAMS_TABLE_1, 80, 1, 1 },
#endif
#if defined BCM6813
	{ "TCPSPDTEST_ENGINE_GLOBAL_TABLE", 1, CORE_1_INDEX, &TCPSPDTEST_ENGINE_GLOBAL_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "TASK_IDX", 1, CORE_1_INDEX, &TASK_IDX_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_1_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SPDSVC_GEN_PARAMS_TABLE", 1, CORE_1_INDEX, &SPDSVC_GEN_PARAMS_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_1_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_1_INDEX, &LOOPBACK_WAN_FLOW_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "FORCE_DSCP", 1, CORE_1_INDEX, &FORCE_DSCP_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "CORE_ID_TABLE", 1, CORE_1_INDEX, &CORE_ID_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_1_INDEX, &INGRESS_FILTER_PROFILE_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM6813
	{ "REGISTERS_BUFFER", 1, CORE_1_INDEX, &REGISTERS_BUFFER_1, 32, 1, 1 },
#endif
#if defined BCM6813
	{ "UDPSPDT_TX_PARAMS_TABLE", 1, CORE_1_INDEX, &UDPSPDT_TX_PARAMS_TABLE_1, 4, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_HW_CFG", 1, CORE_1_INDEX, &DHD_HW_CFG_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SPDSVC_TCPSPDTEST_COMMON_TABLE", 1, CORE_1_INDEX, &SPDSVC_TCPSPDTEST_COMMON_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "CSO_DISABLE", 1, CORE_1_INDEX, &CSO_DISABLE_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_1_INDEX, &RX_MIRRORING_CONFIGURATION_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "NAT_CACHE_CFG", 1, CORE_1_INDEX, &NAT_CACHE_CFG_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_1_INDEX, &DEBUG_PRINT_CORE_LOCK_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "TR471_SPDSVC_RX_PKT_ID_TABLE", 1, CORE_1_INDEX, &TR471_SPDSVC_RX_PKT_ID_TABLE_1, 4, 1, 1 },
#endif
#if defined BCM6813
	{ "PKTGEN_BBMSG_REPLY_SCRATCH", 1, CORE_1_INDEX, &PKTGEN_BBMSG_REPLY_SCRATCH_1, 2, 1, 1 },
#endif
#if defined BCM6813
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_1_INDEX, &NAT_CACHE_KEY0_MASK_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_1_INDEX, &TX_MIRRORING_CONFIGURATION_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SRAM_DUMMY_STORE", 1, CORE_1_INDEX, &SRAM_DUMMY_STORE_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_1_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "COMMON_REPROCESSING_UPDATE_FIFO_TABLE", 1, CORE_1_INDEX, &COMMON_REPROCESSING_UPDATE_FIFO_TABLE_1, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "FPM_GLOBAL_CFG", 1, CORE_1_INDEX, &FPM_GLOBAL_CFG_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_1_INDEX, &INGRESS_FILTER_1588_CFG_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_1_INDEX, &RX_MIRRORING_DIRECT_ENABLE_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "TCPSPDTEST_UPDATE_FIFO_TABLE", 1, CORE_1_INDEX, &TCPSPDTEST_UPDATE_FIFO_TABLE_1, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "DEBUG_PRINT_TABLE", 1, CORE_1_INDEX, &DEBUG_PRINT_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "TCAM_GENERIC_FIELDS", 1, CORE_1_INDEX, &TCAM_GENERIC_FIELDS_1, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "PKTGEN_SESSION_DATA", 1, CORE_1_INDEX, &PKTGEN_SESSION_DATA_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "GDX_PARAMS_TABLE", 1, CORE_1_INDEX, &GDX_PARAMS_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_1_INDEX, &TCAM_TABLE_CFG_TABLE_1, 2, 1, 1 },
#endif
#if defined BCM6813
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_1_INDEX, &NATC_L2_VLAN_KEY_MASK_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_1_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_1, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_1_INDEX, &FW_ERROR_VECTOR_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "INGRESS_FILTER_CFG", 1, CORE_1_INDEX, &INGRESS_FILTER_CFG_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "NATC_L2_TOS_MASK", 1, CORE_1_INDEX, &NATC_L2_TOS_MASK_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_1_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_1, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "DEBUG_SCRATCHPAD", 1, CORE_1_INDEX, &DEBUG_SCRATCHPAD_1, 12, 1, 1 },
#endif
#if defined BCM6813
	{ "MULTICAST_KEY_MASK", 1, CORE_1_INDEX, &MULTICAST_KEY_MASK_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "PKTGEN_FPM_UG_MGMT", 1, CORE_1_INDEX, &PKTGEN_FPM_UG_MGMT_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_1_INDEX, &BITS_CALC_MASKS_TABLE_1, 4, 1, 1 },
#endif
#if defined BCM6813
	{ "PKTGEN_TX_STREAM_SCRATCH_TABLE", 1, CORE_1_INDEX, &PKTGEN_TX_STREAM_SCRATCH_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_TX_SCRATCHPAD", 1, CORE_2_INDEX, &CPU_TX_SCRATCHPAD_2, 416, 1, 1 },
#endif
#if defined BCM6813
	{ "PON_TX_FLOW_TABLE", 1, CORE_2_INDEX, &PON_TX_FLOW_TABLE_2, 256, 1, 1 },
#endif
#if defined BCM6813
	{ "RX_FLOW_TABLE", 1, CORE_2_INDEX, &RX_FLOW_TABLE_2, 202, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_INT_INTERRUPT_SCRATCH", 1, CORE_2_INDEX, &CPU_INT_INTERRUPT_SCRATCH_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR", 1, CORE_2_INDEX, &CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "AQM_ENABLE_TABLE", 1, CORE_2_INDEX, &AQM_ENABLE_TABLE_2, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "VPORT_TX_FLOW_TABLE", 1, CORE_2_INDEX, &VPORT_TX_FLOW_TABLE_2, 64, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_2_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER_2, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_2_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_2, 128, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING0_STACK", 1, CORE_2_INDEX, &PROCESSING0_STACK_2, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_RX_INTERRUPT_ID_DDR_ADDR", 1, CORE_2_INDEX, &CPU_RX_INTERRUPT_ID_DDR_ADDR_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_RECYCLE_INTERRUPT_COALESCING_TABLE", 1, CORE_2_INDEX, &CPU_RECYCLE_INTERRUPT_COALESCING_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "GENERAL_TIMER", 1, CORE_2_INDEX, &GENERAL_TIMER_2, 16, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING1_STACK", 1, CORE_2_INDEX, &PROCESSING1_STACK_2, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "NULL_BUFFER", 1, CORE_2_INDEX, &NULL_BUFFER_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SPDTEST_GEN_PARAM", 1, CORE_2_INDEX, &SPDTEST_GEN_PARAM_2, 4, 1, 1 },
#endif
#if defined BCM6813
	{ "SCORE_DATA_COLLECTION_PROFILE_TABLE", 1, CORE_2_INDEX, &SCORE_DATA_COLLECTION_PROFILE_TABLE_2, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING2_STACK", 1, CORE_2_INDEX, &PROCESSING2_STACK_2, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "SYSTEM_CONFIGURATION", 1, CORE_2_INDEX, &SYSTEM_CONFIGURATION_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "BUFFER_ALLOC_REPLY", 1, CORE_2_INDEX, &BUFFER_ALLOC_REPLY_2, 2, 1, 1 },
#endif
#if defined BCM6813
	{ "VPORT_CFG_TABLE", 1, CORE_2_INDEX, &VPORT_CFG_TABLE_2, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "MUTEX_TABLE", 1, CORE_2_INDEX, &MUTEX_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING3_STACK", 1, CORE_2_INDEX, &PROCESSING3_STACK_2, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "SCORE_CFG_TABLE", 1, CORE_2_INDEX, &SCORE_CFG_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_2_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_2, 16, 1, 1 },
#endif
#if defined BCM6813
	{ "SG_DESC_TABLE", 1, CORE_2_INDEX, &SG_DESC_TABLE_2, 2, 1, 1 },
#endif
#if defined BCM6813
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_2_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_2, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING4_STACK", 1, CORE_2_INDEX, &PROCESSING4_STACK_2, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE", 1, CORE_2_INDEX, &CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE", 1, CORE_2_INDEX, &CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE_2, 3, 1, 1 },
#endif
#if defined BCM6813
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_2_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "GENERAL_TIMER_STACK", 1, CORE_2_INDEX, &GENERAL_TIMER_STACK_2, 72, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_TX_RING_INDICES_VALUES_TABLE", 1, CORE_2_INDEX, &CPU_TX_RING_INDICES_VALUES_TABLE_2, 2, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_TX_INGRESS_DISPATCHER_CREDIT_TABLE", 1, CORE_2_INDEX, &CPU_TX_INGRESS_DISPATCHER_CREDIT_TABLE_2, 3, 1, 1 },
#endif
#if defined BCM6813
	{ "CSO_BAD_IPV4_HDR_CSUM_PACKETS", 1, CORE_2_INDEX, &CSO_BAD_IPV4_HDR_CSUM_PACKETS_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_2_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_2, 32, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING5_STACK", 1, CORE_2_INDEX, &PROCESSING5_STACK_2, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "TUNNELS_PARSING_CFG", 1, CORE_2_INDEX, &TUNNELS_PARSING_CFG_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_2_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SCORE_COLLECT_DROP_BITMAP_TABLE", 1, CORE_2_INDEX, &SCORE_COLLECT_DROP_BITMAP_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_TX_RNR_CTR_REPLY_TABLE", 1, CORE_2_INDEX, &CPU_TX_RNR_CTR_REPLY_TABLE_2, 2, 1, 1 },
#endif
#if defined BCM6813
	{ "RX_MIRRORING_TABLE", 1, CORE_2_INDEX, &RX_MIRRORING_TABLE_2, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "AQM_NUM_QUEUES", 1, CORE_2_INDEX, &AQM_NUM_QUEUES_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING6_STACK", 1, CORE_2_INDEX, &PROCESSING6_STACK_2, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "REGISTERS_BUFFER", 1, CORE_2_INDEX, &REGISTERS_BUFFER_2, 32, 1, 1 },
#endif
#if defined BCM6813
	{ "IPTV_CFG_TABLE", 1, CORE_2_INDEX, &IPTV_CFG_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_FPM_THRESHOLDS", 1, CORE_2_INDEX, &DHD_FPM_THRESHOLDS_2, 3, 1, 1 },
#endif
#if defined BCM6813
	{ "TASK_IDX", 1, CORE_2_INDEX, &TASK_IDX_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING7_STACK", 1, CORE_2_INDEX, &PROCESSING7_STACK_2, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_2_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_RECYCLE_RING_DESCRIPTOR_TABLE", 1, CORE_2_INDEX, &CPU_RECYCLE_RING_DESCRIPTOR_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "HOST_TX_TRUNCATE_MIRRORING_TABLE", 1, CORE_2_INDEX, &HOST_TX_TRUNCATE_MIRRORING_TABLE_2, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "DOS_DROP_REASONS_CFG", 1, CORE_2_INDEX, &DOS_DROP_REASONS_CFG_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_2_INDEX, &LOOPBACK_QUEUE_TABLE_2, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "GENERAL_TIMER_ACTION_VEC", 1, CORE_2_INDEX, &GENERAL_TIMER_ACTION_VEC_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_TX_0_STACK", 1, CORE_2_INDEX, &CPU_TX_0_STACK_2, 320, 1, 1 },
#endif
#if defined BCM6813
	{ "VPORT_CFG_EX_TABLE", 1, CORE_2_INDEX, &VPORT_CFG_EX_TABLE_2, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_2_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "TR471_SPDSVC_RX_PKT_ID_TABLE", 1, CORE_2_INDEX, &TR471_SPDSVC_RX_PKT_ID_TABLE_2, 4, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE", 1, CORE_2_INDEX, &CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE_2, 2, 1, 1 },
#endif
#if defined BCM6813
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_2_INDEX, &BUFMNG_DESCRIPTOR_TABLE_2, 16, 1, 1 },
#endif
#if defined BCM6813
	{ "BUFMNG_STATUS_TABLE", 1, CORE_2_INDEX, &BUFMNG_STATUS_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_TX_1_STACK", 1, CORE_2_INDEX, &CPU_TX_1_STACK_2, 320, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_RECYCLE_STACK", 1, CORE_2_INDEX, &CPU_RECYCLE_STACK_2, 32, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_HW_CFG", 1, CORE_2_INDEX, &DHD_HW_CFG_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_2_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_2_INDEX, &LOOPBACK_WAN_FLOW_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "FORCE_DSCP", 1, CORE_2_INDEX, &FORCE_DSCP_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "CORE_ID_TABLE", 1, CORE_2_INDEX, &CORE_ID_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "NAT_CACHE_CFG", 1, CORE_2_INDEX, &NAT_CACHE_CFG_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "CSO_DISABLE", 1, CORE_2_INDEX, &CSO_DISABLE_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "RING_CPU_TX_DESCRIPTOR_DATA_TABLE", 1, CORE_2_INDEX, &RING_CPU_TX_DESCRIPTOR_DATA_TABLE_2, 2, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_TX_RING_DESCRIPTOR_TABLE", 1, CORE_2_INDEX, &CPU_TX_RING_DESCRIPTOR_TABLE_2, 2, 1, 1 },
#endif
#if defined BCM6813
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_2_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_2, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_2_INDEX, &DEBUG_PRINT_CORE_LOCK_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "FPM_GLOBAL_CFG", 1, CORE_2_INDEX, &FPM_GLOBAL_CFG_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_2_INDEX, &RX_MIRRORING_CONFIGURATION_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_2_INDEX, &TX_MIRRORING_CONFIGURATION_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "QM_QUEUE_TO_TX_FLOW_TABLE_PTR_TABLE", 1, CORE_2_INDEX, &QM_QUEUE_TO_TX_FLOW_TABLE_PTR_TABLE_2, 160, 1, 1 },
#endif
#if defined BCM6813
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_2_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_2, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "SRAM_DUMMY_STORE", 1, CORE_2_INDEX, &SRAM_DUMMY_STORE_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DEBUG_SCRATCHPAD", 1, CORE_2_INDEX, &DEBUG_SCRATCHPAD_2, 12, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_RECYCLE_INTERRUPT_SCRATCH", 1, CORE_2_INDEX, &CPU_RECYCLE_INTERRUPT_SCRATCH_2, 2, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_RECYCLE_SHADOW_RD_IDX", 1, CORE_2_INDEX, &CPU_RECYCLE_SHADOW_RD_IDX_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_2_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_2_INDEX, &INGRESS_FILTER_1588_CFG_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_2_INDEX, &RX_MIRRORING_DIRECT_ENABLE_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DEBUG_PRINT_TABLE", 1, CORE_2_INDEX, &DEBUG_PRINT_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "TCAM_GENERIC_FIELDS", 1, CORE_2_INDEX, &TCAM_GENERIC_FIELDS_2, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "QUEUE_THRESHOLD_VECTOR", 1, CORE_2_INDEX, &QUEUE_THRESHOLD_VECTOR_2, 5, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_RECYCLE_SHADOW_WR_IDX", 1, CORE_2_INDEX, &CPU_RECYCLE_SHADOW_WR_IDX_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "GDX_PARAMS_TABLE", 1, CORE_2_INDEX, &GDX_PARAMS_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_RECYCLE_NEXT_PTR_TABLE", 1, CORE_2_INDEX, &CPU_RECYCLE_NEXT_PTR_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_2_INDEX, &NAT_CACHE_KEY0_MASK_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_RECYCLE_SRAM_PD_FIFO", 1, CORE_2_INDEX, &CPU_RECYCLE_SRAM_PD_FIFO_2, 16, 1, 1 },
#endif
#if defined BCM6813
	{ "POLICER_PARAMS_TABLE", 1, CORE_2_INDEX, &POLICER_PARAMS_TABLE_2, 80, 1, 1 },
#endif
#if defined BCM6813
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_2_INDEX, &FW_ERROR_VECTOR_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_2_INDEX, &TCAM_TABLE_CFG_TABLE_2, 2, 1, 1 },
#endif
#if defined BCM6813
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_2_INDEX, &NATC_L2_VLAN_KEY_MASK_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "INGRESS_FILTER_CFG", 1, CORE_2_INDEX, &INGRESS_FILTER_CFG_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_2_INDEX, &BITS_CALC_MASKS_TABLE_2, 4, 1, 1 },
#endif
#if defined BCM6813
	{ "NATC_L2_TOS_MASK", 1, CORE_2_INDEX, &NATC_L2_TOS_MASK_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "MULTICAST_KEY_MASK", 1, CORE_2_INDEX, &MULTICAST_KEY_MASK_2, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SG_CONTEXT_TABLE", 1, CORE_2_INDEX, &SG_CONTEXT_TABLE_2, 2, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_TX_SYNC_FIFO_TABLE", 1, CORE_2_INDEX, &CPU_TX_SYNC_FIFO_TABLE_2, 2, 1, 1 },
#endif
#if defined BCM6813
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_2_INDEX, &INGRESS_FILTER_PROFILE_TABLE_2, 16, 1, 1 },
#endif
#if defined BCM6813
	{ "CRYPTO_SESSION_PARAMS_TABLE", 1, CORE_3_INDEX, &CRYPTO_SESSION_PARAMS_TABLE_3, 64, 1, 1 },
#endif
#if defined BCM6813
	{ "PON_TX_FLOW_TABLE", 1, CORE_3_INDEX, &PON_TX_FLOW_TABLE_3, 256, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_RX_SCRATCHPAD", 1, CORE_3_INDEX, &CPU_RX_SCRATCHPAD_3, 128, 1, 1 },
#endif
#if defined BCM6813
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_3_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_3, 128, 1, 1 },
#endif
#if defined BCM6813
	{ "RX_FLOW_TABLE", 1, CORE_3_INDEX, &RX_FLOW_TABLE_3, 202, 1, 1 },
#endif
#if defined BCM6813
	{ "MUTEX_TABLE", 1, CORE_3_INDEX, &MUTEX_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_RX_INTERRUPT_ID_DDR_ADDR", 1, CORE_3_INDEX, &CPU_RX_INTERRUPT_ID_DDR_ADDR_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "IPV4_HOST_ADDRESS_TABLE", 1, CORE_3_INDEX, &IPV4_HOST_ADDRESS_TABLE_3, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "VPORT_TX_FLOW_TABLE", 1, CORE_3_INDEX, &VPORT_TX_FLOW_TABLE_3, 64, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING0_STACK", 1, CORE_3_INDEX, &PROCESSING0_STACK_3, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "NULL_BUFFER", 1, CORE_3_INDEX, &NULL_BUFFER_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_RX_AUX_INT_SCRATCHPAD", 1, CORE_3_INDEX, &CPU_RX_AUX_INT_SCRATCHPAD_3, 4, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_RX_METER_TABLE", 1, CORE_3_INDEX, &CPU_RX_METER_TABLE_3, 16, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING1_STACK", 1, CORE_3_INDEX, &PROCESSING1_STACK_3, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "SYSTEM_CONFIGURATION", 1, CORE_3_INDEX, &SYSTEM_CONFIGURATION_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SPDTEST_GEN_PARAM", 1, CORE_3_INDEX, &SPDTEST_GEN_PARAM_3, 4, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_RXQ_SCRATCH_TABLE", 1, CORE_3_INDEX, &CPU_RXQ_SCRATCH_TABLE_3, 128, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_3_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER_3, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING2_STACK", 1, CORE_3_INDEX, &PROCESSING2_STACK_3, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "SCORE_CFG_TABLE", 1, CORE_3_INDEX, &SCORE_CFG_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SPDSVC_ANALYZER_PARAMS_TABLE", 1, CORE_3_INDEX, &SPDSVC_ANALYZER_PARAMS_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "GENERAL_TIMER", 1, CORE_3_INDEX, &GENERAL_TIMER_3, 16, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING3_STACK", 1, CORE_3_INDEX, &PROCESSING3_STACK_3, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "RNR_WR_TEST_ADDR", 1, CORE_3_INDEX, &RNR_WR_TEST_ADDR_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_3_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_3, 16, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_RX_CSO_STACK", 1, CORE_3_INDEX, &CPU_RX_CSO_STACK_3, 128, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING4_STACK", 1, CORE_3_INDEX, &PROCESSING4_STACK_3, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "TUNNELS_PARSING_CFG", 1, CORE_3_INDEX, &TUNNELS_PARSING_CFG_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_RX_CSO_DISPATCHER_CREDIT_TABLE", 1, CORE_3_INDEX, &CPU_RX_CSO_DISPATCHER_CREDIT_TABLE_3, 3, 1, 1 },
#endif
#if defined BCM6813
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_3_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SCORE_DATA_COLLECTION_PROFILE_TABLE", 1, CORE_3_INDEX, &SCORE_DATA_COLLECTION_PROFILE_TABLE_3, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING5_STACK", 1, CORE_3_INDEX, &PROCESSING5_STACK_3, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "CSO_BAD_IPV4_HDR_CSUM_PACKETS", 1, CORE_3_INDEX, &CSO_BAD_IPV4_HDR_CSUM_PACKETS_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SCORE_COLLECT_DROP_BITMAP_TABLE", 1, CORE_3_INDEX, &SCORE_COLLECT_DROP_BITMAP_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_3_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_RX_INTERRUPT_SCRATCH", 1, CORE_3_INDEX, &CPU_RX_INTERRUPT_SCRATCH_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "VPORT_CFG_TABLE", 1, CORE_3_INDEX, &VPORT_CFG_TABLE_3, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "RNR_WR_TEST_FROM", 1, CORE_3_INDEX, &RNR_WR_TEST_FROM_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING6_STACK", 1, CORE_3_INDEX, &PROCESSING6_STACK_3, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "IPTV_CFG_TABLE", 1, CORE_3_INDEX, &IPTV_CFG_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_FPM_THRESHOLDS", 1, CORE_3_INDEX, &DHD_FPM_THRESHOLDS_3, 3, 1, 1 },
#endif
#if defined BCM6813
	{ "RNR_WR_TEST_TO", 1, CORE_3_INDEX, &RNR_WR_TEST_TO_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_REASON_AND_VPORT_TO_METER_TABLE", 1, CORE_3_INDEX, &CPU_REASON_AND_VPORT_TO_METER_TABLE_3, 96, 1, 1 },
#endif
#if defined BCM6813
	{ "AQM_ENABLE_TABLE", 1, CORE_3_INDEX, &AQM_ENABLE_TABLE_3, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING7_STACK", 1, CORE_3_INDEX, &PROCESSING7_STACK_3, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_3_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_INTERRUPT_COALESCING_TABLE", 1, CORE_3_INDEX, &CPU_INTERRUPT_COALESCING_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "GENERAL_TIMER_STACK", 1, CORE_3_INDEX, &GENERAL_TIMER_STACK_3, 72, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_RX_LOCAL_SCRATCH", 1, CORE_3_INDEX, &CPU_RX_LOCAL_SCRATCH_3, 2, 1, 1 },
#endif
#if defined BCM6813
	{ "NAT_CACHE_CFG", 1, CORE_3_INDEX, &NAT_CACHE_CFG_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_3_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_3_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_3, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_RX_XPM_PD_FIFO_TABLE", 1, CORE_3_INDEX, &CPU_RX_XPM_PD_FIFO_TABLE_3, 12, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_REASON_TO_METER_TABLE", 1, CORE_3_INDEX, &CPU_REASON_TO_METER_TABLE_3, 64, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_RING_INTERRUPT_COUNTER_TABLE", 1, CORE_3_INDEX, &CPU_RING_INTERRUPT_COUNTER_TABLE_3, 24, 1, 1 },
#endif
#if defined BCM6813
	{ "IPV6_HOST_ADDRESS_CRC_TABLE", 1, CORE_3_INDEX, &IPV6_HOST_ADDRESS_CRC_TABLE_3, 16, 1, 1 },
#endif
#if defined BCM6813
	{ "UDPSPDT_STREAM_RX_STAT_TABLE", 1, CORE_3_INDEX, &UDPSPDT_STREAM_RX_STAT_TABLE_3, 4, 1, 1 },
#endif
#if defined BCM6813
	{ "SPU_REQUEST_STACK", 1, CORE_3_INDEX, &SPU_REQUEST_STACK_3, 64, 1, 1 },
#endif
#if defined BCM6813
	{ "CSO_CONTEXT_TABLE", 1, CORE_3_INDEX, &CSO_CONTEXT_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "UDPSPDT_SCRATCH_TABLE", 1, CORE_3_INDEX, &UDPSPDT_SCRATCH_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "TASK_IDX", 1, CORE_3_INDEX, &TASK_IDX_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "AQM_NUM_QUEUES", 1, CORE_3_INDEX, &AQM_NUM_QUEUES_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DOS_DROP_REASONS_CFG", 1, CORE_3_INDEX, &DOS_DROP_REASONS_CFG_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "RX_MIRRORING_TABLE", 1, CORE_3_INDEX, &RX_MIRRORING_TABLE_3, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "GENERAL_TIMER_ACTION_VEC", 1, CORE_3_INDEX, &GENERAL_TIMER_ACTION_VEC_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_RING_DESCRIPTORS_TABLE", 1, CORE_3_INDEX, &CPU_RING_DESCRIPTORS_TABLE_3, 24, 1, 1 },
#endif
#if defined BCM6813
	{ "SPDSVC_ANALYZER_PD_FIFO_TABLE", 1, CORE_3_INDEX, &SPDSVC_ANALYZER_PD_FIFO_TABLE_3, 4, 1, 1 },
#endif
#if defined BCM6813
	{ "HOST_TX_TRUNCATE_MIRRORING_TABLE", 1, CORE_3_INDEX, &HOST_TX_TRUNCATE_MIRRORING_TABLE_3, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_3_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SPDSVC_ANALYZER_STACK", 1, CORE_3_INDEX, &SPDSVC_ANALYZER_STACK_3, 176, 1, 1 },
#endif
#if defined BCM6813
	{ "DEBUG_PRINT_TABLE", 1, CORE_3_INDEX, &DEBUG_PRINT_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_3_INDEX, &LOOPBACK_QUEUE_TABLE_3, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_3_INDEX, &LOOPBACK_WAN_FLOW_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "FORCE_DSCP", 1, CORE_3_INDEX, &FORCE_DSCP_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "POLICER_PARAMS_TABLE", 1, CORE_3_INDEX, &POLICER_PARAMS_TABLE_3, 80, 1, 1 },
#endif
#if defined BCM6813
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_3_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_3, 32, 1, 1 },
#endif
#if defined BCM6813
	{ "VPORT_CFG_EX_TABLE", 1, CORE_3_INDEX, &VPORT_CFG_EX_TABLE_3, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "CORE_ID_TABLE", 1, CORE_3_INDEX, &CORE_ID_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "CSO_DISABLE", 1, CORE_3_INDEX, &CSO_DISABLE_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_RX_STACK", 1, CORE_3_INDEX, &CPU_RX_STACK_3, 136, 1, 1 },
#endif
#if defined BCM6813
	{ "TCAM_GENERIC_FIELDS", 1, CORE_3_INDEX, &TCAM_GENERIC_FIELDS_3, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_3_INDEX, &NAT_CACHE_KEY0_MASK_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_3_INDEX, &RX_MIRRORING_CONFIGURATION_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_3_INDEX, &TX_MIRRORING_CONFIGURATION_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_3_INDEX, &BUFMNG_DESCRIPTOR_TABLE_3, 16, 1, 1 },
#endif
#if defined BCM6813
	{ "TR471_SPDSVC_RX_PKT_ID_TABLE", 1, CORE_3_INDEX, &TR471_SPDSVC_RX_PKT_ID_TABLE_3, 4, 1, 1 },
#endif
#if defined BCM6813
	{ "GDX_PARAMS_TABLE", 1, CORE_3_INDEX, &GDX_PARAMS_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "RNR_WR_TEST_START", 1, CORE_3_INDEX, &RNR_WR_TEST_START_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_3_INDEX, &DEBUG_PRINT_CORE_LOCK_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SRAM_DUMMY_STORE", 1, CORE_3_INDEX, &SRAM_DUMMY_STORE_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_3_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_3_INDEX, &INGRESS_FILTER_PROFILE_TABLE_3, 16, 1, 1 },
#endif
#if defined BCM6813
	{ "SPU_REQUEST_PD_FIFO_TABLE", 1, CORE_3_INDEX, &SPU_REQUEST_PD_FIFO_TABLE_3, 4, 1, 1 },
#endif
#if defined BCM6813
	{ "SPU_OFFLOAD_PARAMS_TABLE", 1, CORE_3_INDEX, &SPU_OFFLOAD_PARAMS_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_RX_TELEMETRY_BUFFER_LOCATION", 1, CORE_3_INDEX, &CPU_RX_TELEMETRY_BUFFER_LOCATION_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_3_INDEX, &INGRESS_FILTER_1588_CFG_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_3_INDEX, &RX_MIRRORING_DIRECT_ENABLE_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_3_INDEX, &TCAM_TABLE_CFG_TABLE_3, 2, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_RX_CSO_PD_FIFO_TABLE", 1, CORE_3_INDEX, &CPU_RX_CSO_PD_FIFO_TABLE_3, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "REGISTERS_BUFFER", 1, CORE_3_INDEX, &REGISTERS_BUFFER_3, 32, 1, 1 },
#endif
#if defined BCM6813
	{ "SPU_REQUEST_PREPEND", 1, CORE_3_INDEX, &SPU_REQUEST_PREPEND_3, 32, 1, 1 },
#endif
#if defined BCM6813
	{ "CRYPTO_SESSION_STATS_SCRATCH", 1, CORE_3_INDEX, &CRYPTO_SESSION_STATS_SCRATCH_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SPU_OFFLOAD_SCRATCHPAD", 1, CORE_3_INDEX, &SPU_OFFLOAD_SCRATCHPAD_3, 9, 1, 1 },
#endif
#if defined BCM6813
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_3_INDEX, &NATC_L2_VLAN_KEY_MASK_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "INGRESS_FILTER_CFG", 1, CORE_3_INDEX, &INGRESS_FILTER_CFG_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "NATC_L2_TOS_MASK", 1, CORE_3_INDEX, &NATC_L2_TOS_MASK_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "BUFMNG_STATUS_TABLE", 1, CORE_3_INDEX, &BUFMNG_STATUS_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_RX_XPM_UPDATE_FIFO_TABLE", 1, CORE_3_INDEX, &CPU_RX_XPM_UPDATE_FIFO_TABLE_3, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_HW_CFG", 1, CORE_3_INDEX, &DHD_HW_CFG_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_RX_CSO_UPDATE_FIFO_TABLE", 1, CORE_3_INDEX, &CPU_RX_CSO_UPDATE_FIFO_TABLE_3, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "FPM_GLOBAL_CFG", 1, CORE_3_INDEX, &FPM_GLOBAL_CFG_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SPDSVC_ANALYZER_UPDATE_FIFO_TABLE", 1, CORE_3_INDEX, &SPDSVC_ANALYZER_UPDATE_FIFO_TABLE_3, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "BCM_SPDSVC_STREAM_RX_TS_TABLE", 1, CORE_3_INDEX, &BCM_SPDSVC_STREAM_RX_TS_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SPU_REQUEST_UPDATE_FIFO_TABLE", 1, CORE_3_INDEX, &SPU_REQUEST_UPDATE_FIFO_TABLE_3, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_REASON_TO_TC", 1, CORE_3_INDEX, &CPU_REASON_TO_TC_3, 64, 1, 1 },
#endif
#if defined BCM6813
	{ "TC_TO_CPU_RXQ", 1, CORE_3_INDEX, &TC_TO_CPU_RXQ_3, 64, 1, 1 },
#endif
#if defined BCM6813
	{ "EXC_TC_TO_CPU_RXQ", 1, CORE_3_INDEX, &EXC_TC_TO_CPU_RXQ_3, 64, 1, 1 },
#endif
#if defined BCM6813
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_3_INDEX, &FW_ERROR_VECTOR_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_RXQ_DATA_BUF_TYPE_TABLE", 1, CORE_3_INDEX, &CPU_RXQ_DATA_BUF_TYPE_TABLE_3, 32, 1, 1 },
#endif
#if defined BCM6813
	{ "MULTICAST_KEY_MASK", 1, CORE_3_INDEX, &MULTICAST_KEY_MASK_3, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_VPORT_TO_METER_TABLE", 1, CORE_3_INDEX, &CPU_VPORT_TO_METER_TABLE_3, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_3_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_3, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_3_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_3, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "DEBUG_SCRATCHPAD", 1, CORE_3_INDEX, &DEBUG_SCRATCHPAD_3, 12, 1, 1 },
#endif
#if defined BCM6813
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_3_INDEX, &BITS_CALC_MASKS_TABLE_3, 4, 1, 1 },
#endif
#if defined BCM6813
	{ "VPORT_TO_CPU_OBJ", 1, CORE_3_INDEX, &VPORT_TO_CPU_OBJ_3, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_FLOW_RING_CACHE_CTX_TABLE", 1, CORE_4_INDEX, &DHD_FLOW_RING_CACHE_CTX_TABLE_4, 48, 1, 1 },
#endif
#if defined BCM6813
	{ "PON_TX_FLOW_TABLE", 1, CORE_4_INDEX, &PON_TX_FLOW_TABLE_4, 256, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_AUX_INFO_CACHE_TABLE", 1, CORE_4_INDEX, &DHD_AUX_INFO_CACHE_TABLE_4, 48, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_DOORBELL_TX_POST_VALUE", 1, CORE_4_INDEX, &DHD_DOORBELL_TX_POST_VALUE_4, 64, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_POST_COMMON_RADIO_DATA", 1, CORE_4_INDEX, &DHD_POST_COMMON_RADIO_DATA_4, 3, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_RX_INTERRUPT_ID_DDR_ADDR", 1, CORE_4_INDEX, &CPU_RX_INTERRUPT_ID_DDR_ADDR_4, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "AQM_ENABLE_TABLE", 1, CORE_4_INDEX, &AQM_ENABLE_TABLE_4, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_FPM_BUFMNG_RELEASE_STACK", 1, CORE_4_INDEX, &CPU_FPM_BUFMNG_RELEASE_STACK_4, 128, 1, 1 },
#endif
#if defined BCM6813
	{ "POLICER_PARAMS_TABLE", 1, CORE_4_INDEX, &POLICER_PARAMS_TABLE_4, 80, 1, 1 },
#endif
#if defined BCM6813
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_4_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_4, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "VPORT_TX_FLOW_TABLE", 1, CORE_4_INDEX, &VPORT_TX_FLOW_TABLE_4, 64, 1, 1 },
#endif
#if defined BCM6813
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_4_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_4, 128, 1, 1 },
#endif
#if defined BCM6813
	{ "RX_FLOW_TABLE", 1, CORE_4_INDEX, &RX_FLOW_TABLE_4, 202, 1, 1 },
#endif
#if defined BCM6813
	{ "MUTEX_TABLE", 1, CORE_4_INDEX, &MUTEX_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "NULL_BUFFER", 1, CORE_4_INDEX, &NULL_BUFFER_4, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_4_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_4, 32, 1, 1 },
#endif
#if defined BCM6813
	{ "RX_MIRRORING_TABLE", 1, CORE_4_INDEX, &RX_MIRRORING_TABLE_4, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "AQM_NUM_QUEUES", 1, CORE_4_INDEX, &AQM_NUM_QUEUES_4, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_4_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER_4, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_DSTAT_CFG", 1, CORE_4_INDEX, &DHD_DSTAT_CFG_4, 384, 1, 1 },
#endif
#if defined BCM6813
	{ "SCORE_DATA_COLLECTION_PROFILE_TABLE", 1, CORE_4_INDEX, &SCORE_DATA_COLLECTION_PROFILE_TABLE_4, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING0_STACK", 1, CORE_4_INDEX, &PROCESSING0_STACK_4, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "SYSTEM_CONFIGURATION", 1, CORE_4_INDEX, &SYSTEM_CONFIGURATION_4, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SPDTEST_GEN_PARAM", 1, CORE_4_INDEX, &SPDTEST_GEN_PARAM_4, 4, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_MONITOR_BQ_INFO_TABLE", 1, CORE_4_INDEX, &DHD_MONITOR_BQ_INFO_TABLE_4, 4, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING1_STACK", 1, CORE_4_INDEX, &PROCESSING1_STACK_4, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "SCORE_CFG_TABLE", 1, CORE_4_INDEX, &SCORE_CFG_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_4_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_4, 16, 1, 1 },
#endif
#if defined BCM6813
	{ "VPORT_CFG_TABLE", 1, CORE_4_INDEX, &VPORT_CFG_TABLE_4, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_4_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING2_STACK", 1, CORE_4_INDEX, &PROCESSING2_STACK_4, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "MIRRORING_SCRATCH", 1, CORE_4_INDEX, &MIRRORING_SCRATCH_4, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SPU_RESPONSE_DISPATCHER_CREDIT_TABLE", 1, CORE_4_INDEX, &SPU_RESPONSE_DISPATCHER_CREDIT_TABLE_4, 3, 1, 1 },
#endif
#if defined BCM6813
	{ "CSO_BAD_IPV4_HDR_CSUM_PACKETS", 1, CORE_4_INDEX, &CSO_BAD_IPV4_HDR_CSUM_PACKETS_4, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_BACKUP_INDEX_CACHE", 1, CORE_4_INDEX, &DHD_BACKUP_INDEX_CACHE_4, 3, 1, 1 },
#endif
#if defined BCM6813
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_4_INDEX, &BUFMNG_DESCRIPTOR_TABLE_4, 16, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING3_STACK", 1, CORE_4_INDEX, &PROCESSING3_STACK_4, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR", 1, CORE_4_INDEX, &DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_4, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_4_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SCORE_COLLECT_DROP_BITMAP_TABLE", 1, CORE_4_INDEX, &SCORE_COLLECT_DROP_BITMAP_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SPU_RESPONSE_STACK", 1, CORE_4_INDEX, &SPU_RESPONSE_STACK_4, 96, 1, 1 },
#endif
#if defined BCM6813
	{ "BUFMNG_STATUS_TABLE", 1, CORE_4_INDEX, &BUFMNG_STATUS_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING4_STACK", 1, CORE_4_INDEX, &PROCESSING4_STACK_4, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "TUNNELS_PARSING_CFG", 1, CORE_4_INDEX, &TUNNELS_PARSING_CFG_4, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_FPM_THRESHOLDS", 1, CORE_4_INDEX, &DHD_FPM_THRESHOLDS_4, 3, 1, 1 },
#endif
#if defined BCM6813
	{ "TASK_IDX", 1, CORE_4_INDEX, &TASK_IDX_4, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_L2_HEADER", 1, CORE_4_INDEX, &DHD_L2_HEADER_4, 72, 1, 1 },
#endif
#if defined BCM6813
	{ "IPTV_CFG_TABLE", 1, CORE_4_INDEX, &IPTV_CFG_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_CPU_INT_ID", 1, CORE_4_INDEX, &DHD_CPU_INT_ID_4, 3, 1, 1 },
#endif
#if defined BCM6813
	{ "DOS_DROP_REASONS_CFG", 1, CORE_4_INDEX, &DOS_DROP_REASONS_CFG_4, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_4_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_TIMER_STACK", 1, CORE_4_INDEX, &DHD_TIMER_STACK_4, 32, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING5_STACK", 1, CORE_4_INDEX, &PROCESSING5_STACK_4, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "REGISTERS_BUFFER", 1, CORE_4_INDEX, &REGISTERS_BUFFER_4, 32, 1, 1 },
#endif
#if defined BCM6813
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_4_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_4, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_MIRRORING_DISPATCHER_CREDIT_TABLE", 1, CORE_4_INDEX, &DHD_MIRRORING_DISPATCHER_CREDIT_TABLE_4, 3, 1, 1 },
#endif
#if defined BCM6813
	{ "SPDSVC_WLAN_TXPOST_PARAMS_TABLE", 1, CORE_4_INDEX, &SPDSVC_WLAN_TXPOST_PARAMS_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_4_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_4, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_4_INDEX, &LOOPBACK_WAN_FLOW_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING6_STACK", 1, CORE_4_INDEX, &PROCESSING6_STACK_4, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "NAT_CACHE_CFG", 1, CORE_4_INDEX, &NAT_CACHE_CFG_4, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "FORCE_DSCP", 1, CORE_4_INDEX, &FORCE_DSCP_4, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DEBUG_PRINT_TABLE", 1, CORE_4_INDEX, &DEBUG_PRINT_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "CRYPTO_SESSION_STATS_SCRATCH", 1, CORE_4_INDEX, &CRYPTO_SESSION_STATS_SCRATCH_4, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING7_STACK", 1, CORE_4_INDEX, &PROCESSING7_STACK_4, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "TCAM_GENERIC_FIELDS", 1, CORE_4_INDEX, &TCAM_GENERIC_FIELDS_4, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_4_INDEX, &NAT_CACHE_KEY0_MASK_4, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "CORE_ID_TABLE", 1, CORE_4_INDEX, &CORE_ID_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "CSO_DISABLE", 1, CORE_4_INDEX, &CSO_DISABLE_4, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_4_INDEX, &RX_MIRRORING_CONFIGURATION_4, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "HOST_TX_TRUNCATE_MIRRORING_TABLE", 1, CORE_4_INDEX, &HOST_TX_TRUNCATE_MIRRORING_TABLE_4, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_4_INDEX, &TX_MIRRORING_CONFIGURATION_4, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_4_INDEX, &LOOPBACK_QUEUE_TABLE_4, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_4_INDEX, &DEBUG_PRINT_CORE_LOCK_4, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SRAM_DUMMY_STORE", 1, CORE_4_INDEX, &SRAM_DUMMY_STORE_4, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "FPM_RING_BUFMGT_CNT_TABLE", 1, CORE_4_INDEX, &FPM_RING_BUFMGT_CNT_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_4_INDEX, &TCAM_TABLE_CFG_TABLE_4, 2, 1, 1 },
#endif
#if defined BCM6813
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_4_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_4_INDEX, &INGRESS_FILTER_1588_CFG_4, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_4_INDEX, &RX_MIRRORING_DIRECT_ENABLE_4, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "GDX_PARAMS_TABLE", 1, CORE_4_INDEX, &GDX_PARAMS_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_TX_POST_UPDATE_FIFO_STACK", 1, CORE_4_INDEX, &DHD_TX_POST_UPDATE_FIFO_STACK_4, 32, 1, 1 },
#endif
#if defined BCM6813
	{ "VPORT_CFG_EX_TABLE", 1, CORE_4_INDEX, &VPORT_CFG_EX_TABLE_4, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "SPU_RESPONSE_PARAMS_TABLE", 1, CORE_4_INDEX, &SPU_RESPONSE_PARAMS_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_4_INDEX, &NATC_L2_VLAN_KEY_MASK_4, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "TR471_SPDSVC_RX_PKT_ID_TABLE", 1, CORE_4_INDEX, &TR471_SPDSVC_RX_PKT_ID_TABLE_4, 4, 1, 1 },
#endif
#if defined BCM6813
	{ "INGRESS_FILTER_CFG", 1, CORE_4_INDEX, &INGRESS_FILTER_CFG_4, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "NATC_L2_TOS_MASK", 1, CORE_4_INDEX, &NATC_L2_TOS_MASK_4, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_TX_POST_FLOW_RING_BUFFER", 1, CORE_4_INDEX, &DHD_TX_POST_FLOW_RING_BUFFER_4, 3, 1, 1 },
#endif
#if defined BCM6813
	{ "SPU_RESPONSE_SCRATCHPAD", 1, CORE_4_INDEX, &SPU_RESPONSE_SCRATCHPAD_4, 9, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_HW_CFG", 1, CORE_4_INDEX, &DHD_HW_CFG_4, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_TX_POST_0_STACK", 1, CORE_4_INDEX, &DHD_TX_POST_0_STACK_4, 136, 1, 1 },
#endif
#if defined BCM6813
	{ "DEBUG_SCRATCHPAD", 1, CORE_4_INDEX, &DEBUG_SCRATCHPAD_4, 12, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_TX_POST_UPDATE_FIFO_TABLE", 1, CORE_4_INDEX, &DHD_TX_POST_UPDATE_FIFO_TABLE_4, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "FPM_GLOBAL_CFG", 1, CORE_4_INDEX, &FPM_GLOBAL_CFG_4, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_TX_POST_1_STACK", 1, CORE_4_INDEX, &DHD_TX_POST_1_STACK_4, 136, 1, 1 },
#endif
#if defined BCM6813
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_4_INDEX, &FW_ERROR_VECTOR_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_4_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_4, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "MULTICAST_KEY_MASK", 1, CORE_4_INDEX, &MULTICAST_KEY_MASK_4, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_TX_POST_2_STACK", 1, CORE_4_INDEX, &DHD_TX_POST_2_STACK_4, 136, 1, 1 },
#endif
#if defined BCM6813
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_4_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_4, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_4_INDEX, &INGRESS_FILTER_PROFILE_TABLE_4, 16, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_CPU_TX_POST_RING_DESCRIPTOR_TABLE", 1, CORE_4_INDEX, &DHD_CPU_TX_POST_RING_DESCRIPTOR_TABLE_4, 3, 1, 1 },
#endif
#if defined BCM6813
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_4_INDEX, &BITS_CALC_MASKS_TABLE_4, 4, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_TX_POST_PD_FIFO_TABLE", 1, CORE_4_INDEX, &DHD_TX_POST_PD_FIFO_TABLE_4, 6, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE", 1, CORE_4_INDEX, &DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE_4, 3, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_FLOW_RING_CACHE_LKP_TABLE", 1, CORE_4_INDEX, &DHD_FLOW_RING_CACHE_LKP_TABLE_4, 48, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_TX_MCORE_SCRATCHPAD", 1, CORE_5_INDEX, &CPU_TX_MCORE_SCRATCHPAD_5, 208, 1, 1 },
#endif
#if defined BCM6813
	{ "DIRECT_FLOW_STACK", 1, CORE_5_INDEX, &DIRECT_FLOW_STACK_5, 128, 1, 1 },
#endif
#if defined BCM6813
	{ "PON_TX_FLOW_TABLE", 1, CORE_5_INDEX, &PON_TX_FLOW_TABLE_5, 256, 1, 1 },
#endif
#if defined BCM6813
	{ "REPORTING_QUEUE_DESCRIPTOR_TABLE", 1, CORE_5_INDEX, &REPORTING_QUEUE_DESCRIPTOR_TABLE_5, 129, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR", 1, CORE_5_INDEX, &CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_RECYCLE_INTERRUPT_COALESCING_TABLE", 1, CORE_5_INDEX, &CPU_RECYCLE_INTERRUPT_COALESCING_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DIRECT_FLOW_CNTR_TABLE", 1, CORE_5_INDEX, &DIRECT_FLOW_CNTR_TABLE_5, 32, 1, 1 },
#endif
#if defined BCM6813
	{ "VPORT_TX_FLOW_TABLE", 1, CORE_5_INDEX, &VPORT_TX_FLOW_TABLE_5, 64, 1, 1 },
#endif
#if defined BCM6813
	{ "GENERAL_TIMER", 1, CORE_5_INDEX, &GENERAL_TIMER_5, 16, 1, 1 },
#endif
#if defined BCM6813
	{ "REPORTING_QUEUE_ACCUMULATED_TABLE", 1, CORE_5_INDEX, &REPORTING_QUEUE_ACCUMULATED_TABLE_5, 16, 1, 1 },
#endif
#if defined BCM6813
	{ "RX_FLOW_TABLE", 1, CORE_5_INDEX, &RX_FLOW_TABLE_5, 202, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_INT_INTERRUPT_SCRATCH", 1, CORE_5_INDEX, &CPU_INT_INTERRUPT_SCRATCH_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "NULL_BUFFER", 1, CORE_5_INDEX, &NULL_BUFFER_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "REPORTING_STACK", 1, CORE_5_INDEX, &REPORTING_STACK_5, 32, 1, 1 },
#endif
#if defined BCM6813
	{ "RX_MIRRORING_TABLE", 1, CORE_5_INDEX, &RX_MIRRORING_TABLE_5, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "DIRECT_FLOW_PAUSE_QUANTA", 1, CORE_5_INDEX, &DIRECT_FLOW_PAUSE_QUANTA_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_5_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER_5, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_5_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_5, 128, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING0_STACK", 1, CORE_5_INDEX, &PROCESSING0_STACK_5, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "SYSTEM_CONFIGURATION", 1, CORE_5_INDEX, &SYSTEM_CONFIGURATION_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SPDTEST_GEN_PARAM", 1, CORE_5_INDEX, &SPDTEST_GEN_PARAM_5, 4, 1, 1 },
#endif
#if defined BCM6813
	{ "SCORE_DATA_COLLECTION_PROFILE_TABLE", 1, CORE_5_INDEX, &SCORE_DATA_COLLECTION_PROFILE_TABLE_5, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING1_STACK", 1, CORE_5_INDEX, &PROCESSING1_STACK_5, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "SCORE_CFG_TABLE", 1, CORE_5_INDEX, &SCORE_CFG_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DIRECT_FLOW_PD_TABLE", 1, CORE_5_INDEX, &DIRECT_FLOW_PD_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "VPORT_CFG_TABLE", 1, CORE_5_INDEX, &VPORT_CFG_TABLE_5, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "MUTEX_TABLE", 1, CORE_5_INDEX, &MUTEX_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING2_STACK", 1, CORE_5_INDEX, &PROCESSING2_STACK_5, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "REPORT_BBH_TX_QUEUE_ID_TABLE", 1, CORE_5_INDEX, &REPORT_BBH_TX_QUEUE_ID_TABLE_5, 2, 1, 1 },
#endif
#if defined BCM6813
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_5_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_5, 16, 1, 1 },
#endif
#if defined BCM6813
	{ "GENERAL_TIMER_STACK", 1, CORE_5_INDEX, &GENERAL_TIMER_STACK_5, 72, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE", 1, CORE_5_INDEX, &CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DIRECT_FLOW_PAUSE_DEBUG", 1, CORE_5_INDEX, &DIRECT_FLOW_PAUSE_DEBUG_5, 3, 1, 1 },
#endif
#if defined BCM6813
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_5_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "AQM_ENABLE_TABLE", 1, CORE_5_INDEX, &AQM_ENABLE_TABLE_5, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING3_STACK", 1, CORE_5_INDEX, &PROCESSING3_STACK_5, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "REGISTERS_BUFFER", 1, CORE_5_INDEX, &REGISTERS_BUFFER_5, 32, 1, 1 },
#endif
#if defined BCM6813
	{ "TUNNELS_PARSING_CFG", 1, CORE_5_INDEX, &TUNNELS_PARSING_CFG_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE", 1, CORE_5_INDEX, &CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE_5, 3, 1, 1 },
#endif
#if defined BCM6813
	{ "CSO_BAD_IPV4_HDR_CSUM_PACKETS", 1, CORE_5_INDEX, &CSO_BAD_IPV4_HDR_CSUM_PACKETS_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING4_STACK", 1, CORE_5_INDEX, &PROCESSING4_STACK_5, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "SCORE_COLLECT_DROP_BITMAP_TABLE", 1, CORE_5_INDEX, &SCORE_COLLECT_DROP_BITMAP_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "GHOST_REPORTING_GLOBAL_CFG", 1, CORE_5_INDEX, &GHOST_REPORTING_GLOBAL_CFG_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_5_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_TX_MCORE_RING_INDICES_VALUES_TABLE", 1, CORE_5_INDEX, &CPU_TX_MCORE_RING_INDICES_VALUES_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SG_CONTEXT_MCORE_TABLE", 1, CORE_5_INDEX, &SG_CONTEXT_MCORE_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "HOST_TX_TRUNCATE_MIRRORING_TABLE", 1, CORE_5_INDEX, &HOST_TX_TRUNCATE_MIRRORING_TABLE_5, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "AQM_NUM_QUEUES", 1, CORE_5_INDEX, &AQM_NUM_QUEUES_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING5_STACK", 1, CORE_5_INDEX, &PROCESSING5_STACK_5, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "IPTV_CFG_TABLE", 1, CORE_5_INDEX, &IPTV_CFG_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_FPM_THRESHOLDS", 1, CORE_5_INDEX, &DHD_FPM_THRESHOLDS_5, 3, 1, 1 },
#endif
#if defined BCM6813
	{ "TASK_IDX", 1, CORE_5_INDEX, &TASK_IDX_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_5_INDEX, &LOOPBACK_QUEUE_TABLE_5, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "DOS_DROP_REASONS_CFG", 1, CORE_5_INDEX, &DOS_DROP_REASONS_CFG_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "VPORT_CFG_EX_TABLE", 1, CORE_5_INDEX, &VPORT_CFG_EX_TABLE_5, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "GENERAL_TIMER_ACTION_VEC", 1, CORE_5_INDEX, &GENERAL_TIMER_ACTION_VEC_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING6_STACK", 1, CORE_5_INDEX, &PROCESSING6_STACK_5, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_5_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_RECYCLE_RING_DESCRIPTOR_TABLE", 1, CORE_5_INDEX, &CPU_RECYCLE_RING_DESCRIPTOR_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "TR471_SPDSVC_RX_PKT_ID_TABLE", 1, CORE_5_INDEX, &TR471_SPDSVC_RX_PKT_ID_TABLE_5, 4, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE", 1, CORE_5_INDEX, &CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE_5, 2, 1, 1 },
#endif
#if defined BCM6813
	{ "SG_DESC_MCORE_TABLE", 1, CORE_5_INDEX, &SG_DESC_MCORE_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_RECYCLE_INTERRUPT_SCRATCH", 1, CORE_5_INDEX, &CPU_RECYCLE_INTERRUPT_SCRATCH_5, 2, 1, 1 },
#endif
#if defined BCM6813
	{ "BBH_TX_EGRESS_REPORT_COUNTER_TABLE", 1, CORE_5_INDEX, &BBH_TX_EGRESS_REPORT_COUNTER_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING7_STACK", 1, CORE_5_INDEX, &PROCESSING7_STACK_5, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "BBH_TX_INGRESS_COUNTER_TABLE", 1, CORE_5_INDEX, &BBH_TX_INGRESS_COUNTER_TABLE_5, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_TX_MCORE_RING_DESCRIPTOR_TABLE", 1, CORE_5_INDEX, &CPU_TX_MCORE_RING_DESCRIPTOR_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_TX_MCORE_RNR_CTR_REPLY_TABLE", 1, CORE_5_INDEX, &CPU_TX_MCORE_RNR_CTR_REPLY_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_5_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_5, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_5_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_5, 32, 1, 1 },
#endif
#if defined BCM6813
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_5_INDEX, &BUFMNG_DESCRIPTOR_TABLE_5, 16, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_TX_MCORE_STACK", 1, CORE_5_INDEX, &CPU_TX_MCORE_STACK_5, 320, 1, 1 },
#endif
#if defined BCM6813
	{ "BUFMNG_STATUS_TABLE", 1, CORE_5_INDEX, &BUFMNG_STATUS_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_RECYCLE_STACK", 1, CORE_5_INDEX, &CPU_RECYCLE_STACK_5, 32, 1, 1 },
#endif
#if defined BCM6813
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_5_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_5, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "US_TM_TX_PAUSE_NACK", 1, CORE_5_INDEX, &US_TM_TX_PAUSE_NACK_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DIRECT_FLOW_EPON_CONTROL_SCRATCH", 1, CORE_5_INDEX, &DIRECT_FLOW_EPON_CONTROL_SCRATCH_5, 22, 1, 1 },
#endif
#if defined BCM6813
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_5_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "NAT_CACHE_CFG", 1, CORE_5_INDEX, &NAT_CACHE_CFG_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_5_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_5_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_5, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "DIRECT_FLOW_WAN_VIQ_EXCLUSIVE", 1, CORE_5_INDEX, &DIRECT_FLOW_WAN_VIQ_EXCLUSIVE_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_HW_CFG", 1, CORE_5_INDEX, &DHD_HW_CFG_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_5_INDEX, &LOOPBACK_WAN_FLOW_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "FORCE_DSCP", 1, CORE_5_INDEX, &FORCE_DSCP_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "CORE_ID_TABLE", 1, CORE_5_INDEX, &CORE_ID_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "CSO_DISABLE", 1, CORE_5_INDEX, &CSO_DISABLE_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_RECYCLE_SHADOW_RD_IDX", 1, CORE_5_INDEX, &CPU_RECYCLE_SHADOW_RD_IDX_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_5_INDEX, &RX_MIRRORING_CONFIGURATION_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_5_INDEX, &TX_MIRRORING_CONFIGURATION_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_5_INDEX, &DEBUG_PRINT_CORE_LOCK_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SRAM_DUMMY_STORE", 1, CORE_5_INDEX, &SRAM_DUMMY_STORE_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "REPORTING_QUEUE_COUNTER_TABLE", 1, CORE_5_INDEX, &REPORTING_QUEUE_COUNTER_TABLE_5, 129, 1, 1 },
#endif
#if defined BCM6813
	{ "MAC_TYPE", 1, CORE_5_INDEX, &MAC_TYPE_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_5_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_5_INDEX, &INGRESS_FILTER_1588_CFG_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_5_INDEX, &RX_MIRRORING_DIRECT_ENABLE_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DEBUG_SCRATCHPAD", 1, CORE_5_INDEX, &DEBUG_SCRATCHPAD_5, 12, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_RECYCLE_SHADOW_WR_IDX", 1, CORE_5_INDEX, &CPU_RECYCLE_SHADOW_WR_IDX_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "QUEUE_THRESHOLD_VECTOR", 1, CORE_5_INDEX, &QUEUE_THRESHOLD_VECTOR_5, 5, 1, 1 },
#endif
#if defined BCM6813
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_5_INDEX, &NAT_CACHE_KEY0_MASK_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "GHOST_REPORTING_QUEUE_STATUS_BIT_VECTOR_TABLE", 1, CORE_5_INDEX, &GHOST_REPORTING_QUEUE_STATUS_BIT_VECTOR_TABLE_5, 5, 1, 1 },
#endif
#if defined BCM6813
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_5_INDEX, &TCAM_TABLE_CFG_TABLE_5, 2, 1, 1 },
#endif
#if defined BCM6813
	{ "QUEUE_TO_REPORT_BIT_VECTOR", 1, CORE_5_INDEX, &QUEUE_TO_REPORT_BIT_VECTOR_5, 5, 1, 1 },
#endif
#if defined BCM6813
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_5_INDEX, &NATC_L2_VLAN_KEY_MASK_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "FPM_GLOBAL_CFG", 1, CORE_5_INDEX, &FPM_GLOBAL_CFG_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_5_INDEX, &BITS_CALC_MASKS_TABLE_5, 4, 1, 1 },
#endif
#if defined BCM6813
	{ "DEBUG_PRINT_TABLE", 1, CORE_5_INDEX, &DEBUG_PRINT_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "TCAM_GENERIC_FIELDS", 1, CORE_5_INDEX, &TCAM_GENERIC_FIELDS_5, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "GDX_PARAMS_TABLE", 1, CORE_5_INDEX, &GDX_PARAMS_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_RECYCLE_SRAM_PD_FIFO", 1, CORE_5_INDEX, &CPU_RECYCLE_SRAM_PD_FIFO_5, 16, 1, 1 },
#endif
#if defined BCM6813
	{ "POLICER_PARAMS_TABLE", 1, CORE_5_INDEX, &POLICER_PARAMS_TABLE_5, 80, 1, 1 },
#endif
#if defined BCM6813
	{ "XGPON_REPORT_ZERO_SENT_TABLE", 1, CORE_5_INDEX, &XGPON_REPORT_ZERO_SENT_TABLE_5, 10, 1, 1 },
#endif
#if defined BCM6813
	{ "CPU_RECYCLE_NEXT_PTR_TABLE", 1, CORE_5_INDEX, &CPU_RECYCLE_NEXT_PTR_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "INGRESS_FILTER_CFG", 1, CORE_5_INDEX, &INGRESS_FILTER_CFG_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_5_INDEX, &FW_ERROR_VECTOR_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "NATC_L2_TOS_MASK", 1, CORE_5_INDEX, &NATC_L2_TOS_MASK_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "MULTICAST_KEY_MASK", 1, CORE_5_INDEX, &MULTICAST_KEY_MASK_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "REPORTING_COUNTER_TABLE", 1, CORE_5_INDEX, &REPORTING_COUNTER_TABLE_5, 40, 1, 1 },
#endif
#if defined BCM6813
	{ "DIRECT_FLOW_RX_MIRRORING_SCRATCHPAD", 1, CORE_5_INDEX, &DIRECT_FLOW_RX_MIRRORING_SCRATCHPAD_5, 136, 1, 1 },
#endif
#if defined BCM6813
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_5_INDEX, &INGRESS_FILTER_PROFILE_TABLE_5, 16, 1, 1 },
#endif
#if defined BCM6813
	{ "RING_CPU_TX_MCORE_DESCRIPTOR_DATA_TABLE", 1, CORE_5_INDEX, &RING_CPU_TX_MCORE_DESCRIPTOR_DATA_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "US_TM_PD_FIFO_TABLE", 1, CORE_6_INDEX, &US_TM_PD_FIFO_TABLE_6, 160, 1, 1 },
#endif
#if defined BCM6813
	{ "US_TM_SECONDARY_SCHEDULER_TABLE", 1, CORE_6_INDEX, &US_TM_SECONDARY_SCHEDULER_TABLE_6, 32, 1, 1 },
#endif
#if defined BCM6813
	{ "TX_TASK_US_0_STACK", 1, CORE_6_INDEX, &TX_TASK_US_0_STACK_6, 128, 1, 1 },
#endif
#if defined BCM6813
	{ "US_TM_SCHEDULING_QUEUE_TABLE", 1, CORE_6_INDEX, &US_TM_SCHEDULING_QUEUE_TABLE_6, 80, 1, 1 },
#endif
#if defined BCM6813
	{ "US_TM_RATE_LIMITER_PROFILE_RESIDUE_TABLE", 1, CORE_6_INDEX, &US_TM_RATE_LIMITER_PROFILE_RESIDUE_TABLE_6, 32, 1, 1 },
#endif
#if defined BCM6813
	{ "US_TM_TM_FLOW_CNTR_TABLE", 1, CORE_6_INDEX, &US_TM_TM_FLOW_CNTR_TABLE_6, 64, 1, 1 },
#endif
#if defined BCM6813
	{ "TX_TASK_US_1_STACK", 1, CORE_6_INDEX, &TX_TASK_US_1_STACK_6, 128, 1, 1 },
#endif
#if defined BCM6813
	{ "US_TM_SCHEDULER_TABLE", 1, CORE_6_INDEX, &US_TM_SCHEDULER_TABLE_6, 51, 1, 1 },
#endif
#if defined BCM6813
	{ "SYSTEM_CONFIGURATION", 1, CORE_6_INDEX, &SYSTEM_CONFIGURATION_6, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "US_TM_RATE_LIMITER_BUDGET_VALID", 1, CORE_6_INDEX, &US_TM_RATE_LIMITER_BUDGET_VALID_6, 4, 1, 1 },
#endif
#if defined BCM6813
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_6_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_6, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "US_TM_PON_TM_FLOW_CNTR_TABLE", 1, CORE_6_INDEX, &US_TM_PON_TM_FLOW_CNTR_TABLE_6, 256, 1, 1 },
#endif
#if defined BCM6813
	{ "US_TM_BBH_QUEUE_TABLE", 1, CORE_6_INDEX, &US_TM_BBH_QUEUE_TABLE_6, 45, 1, 1 },
#endif
#if defined BCM6813
	{ "SCORE_INTERRUPT_ID_DDR_ADDR", 1, CORE_6_INDEX, &SCORE_INTERRUPT_ID_DDR_ADDR_6, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "US_TM_DISPATCHER_CREDIT_TABLE", 1, CORE_6_INDEX, &US_TM_DISPATCHER_CREDIT_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "MUTEX_TABLE", 1, CORE_6_INDEX, &MUTEX_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "TX_TASK_PON_STACK", 1, CORE_6_INDEX, &TX_TASK_PON_STACK_6, 128, 1, 1 },
#endif
#if defined BCM6813
	{ "US_TM_RATE_LIMITER_BUDGET_DESCRIPTOR_TABLE", 1, CORE_6_INDEX, &US_TM_RATE_LIMITER_BUDGET_DESCRIPTOR_TABLE_6, 80, 1, 1 },
#endif
#if defined BCM6813
	{ "US_TM_RATE_LIMITER_PROFILE_TABLE", 1, CORE_6_INDEX, &US_TM_RATE_LIMITER_PROFILE_TABLE_6, 32, 1, 1 },
#endif
#if defined BCM6813
	{ "SCORE_SDR_DB_TABLE", 1, CORE_6_INDEX, &SCORE_SDR_DB_TABLE_6, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "US_TM_RATE_LIMITER_PARAMS_DESCRIPTOR_TABLE", 1, CORE_6_INDEX, &US_TM_RATE_LIMITER_PARAMS_DESCRIPTOR_TABLE_6, 80, 1, 1 },
#endif
#if defined BCM6813
	{ "US_TM_SCHEDULER_POOL", 1, CORE_6_INDEX, &US_TM_SCHEDULER_POOL_6, 312, 1, 1 },
#endif
#if defined BCM6813
	{ "SCORE_CFG_TABLE", 1, CORE_6_INDEX, &SCORE_CFG_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "VPORT_TX_FLOW_TABLE", 1, CORE_6_INDEX, &VPORT_TX_FLOW_TABLE_6, 64, 1, 1 },
#endif
#if defined BCM6813
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_6_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_6, 128, 1, 1 },
#endif
#if defined BCM6813
	{ "US_TM_TX_QUEUE_DROP_TABLE", 1, CORE_6_INDEX, &US_TM_TX_QUEUE_DROP_TABLE_6, 80, 1, 1 },
#endif
#if defined BCM6813
	{ "US_TM_DCL_IMG", 1, CORE_6_INDEX, &US_TM_DCL_IMG_6, 128, 1, 1 },
#endif
#if defined BCM6813
	{ "GENERAL_TIMER", 1, CORE_6_INDEX, &GENERAL_TIMER_6, 16, 1, 1 },
#endif
#if defined BCM6813
	{ "DATA_TRACE_SCRATCHPAD", 1, CORE_6_INDEX, &DATA_TRACE_SCRATCHPAD_6, 32, 1, 1 },
#endif
#if defined BCM6813
	{ "US_TM_AQM_QUEUE_TABLE", 1, CORE_6_INDEX, &US_TM_AQM_QUEUE_TABLE_6, 80, 1, 1 },
#endif
#if defined BCM6813
	{ "UPDATE_FIFO_STACK", 1, CORE_6_INDEX, &UPDATE_FIFO_STACK_6, 64, 1, 1 },
#endif
#if defined BCM6813
	{ "US_TM_AQM_QUEUE_TIMER_TABLE", 1, CORE_6_INDEX, &US_TM_AQM_QUEUE_TIMER_TABLE_6, 80, 1, 1 },
#endif
#if defined BCM6813
	{ "DISPATCHER_CREDIT_TABLE", 1, CORE_6_INDEX, &DISPATCHER_CREDIT_TABLE_6, 3, 1, 1 },
#endif
#if defined BCM6813
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_6_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_6_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_6, 32, 1, 1 },
#endif
#if defined BCM6813
	{ "GENERAL_TIMER_STACK", 1, CORE_6_INDEX, &GENERAL_TIMER_STACK_6, 72, 1, 1 },
#endif
#if defined BCM6813
	{ "REGISTERS_BUFFER", 1, CORE_6_INDEX, &REGISTERS_BUFFER_6, 32, 1, 1 },
#endif
#if defined BCM6813
	{ "MIRRORING_SCRATCH", 1, CORE_6_INDEX, &MIRRORING_SCRATCH_6, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_6_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SCORE_QSTAT_SEQ_NUM", 1, CORE_6_INDEX, &SCORE_QSTAT_SEQ_NUM_6, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "PON_TM_FW_FLUSH_TABLE", 1, CORE_6_INDEX, &PON_TM_FW_FLUSH_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "US_TM_DCL_MULTIPLE_CNTR", 1, CORE_6_INDEX, &US_TM_DCL_MULTIPLE_CNTR_6, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "BUFFER_CONG_SCRATCHPAD", 1, CORE_6_INDEX, &BUFFER_CONG_SCRATCHPAD_6, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "PON_TM_TX_TRUNCATE_MIRRORING_TABLE", 1, CORE_6_INDEX, &PON_TM_TX_TRUNCATE_MIRRORING_TABLE_6, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "BUDGET_ALLOCATION_TIMER_VALUE", 1, CORE_6_INDEX, &BUDGET_ALLOCATION_TIMER_VALUE_6, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "US_TM_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_TABLE", 1, CORE_6_INDEX, &US_TM_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_TABLE_6, 3, 1, 1 },
#endif
#if defined BCM6813
	{ "SCORE_COALESCING_TABLE", 1, CORE_6_INDEX, &SCORE_COALESCING_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "GENERAL_TIMER_ACTION_VEC", 1, CORE_6_INDEX, &GENERAL_TIMER_ACTION_VEC_6, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "US_TM_BBH_TX_EGRESS_COUNTER_TABLE", 1, CORE_6_INDEX, &US_TM_BBH_TX_EGRESS_COUNTER_TABLE_6, 7, 1, 1 },
#endif
#if defined BCM6813
	{ "TASK_IDX", 1, CORE_6_INDEX, &TASK_IDX_6, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "TX_EXCEPTION", 1, CORE_6_INDEX, &TX_EXCEPTION_6, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "CORE_ID_TABLE", 1, CORE_6_INDEX, &CORE_ID_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "BBH_TX_US_WAN_0_FIFO_BYTES_THRESHOLD", 1, CORE_6_INDEX, &BBH_TX_US_WAN_0_FIFO_BYTES_THRESHOLD_6, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "BUFFER_CONG_DQM_NOT_EMPTY", 1, CORE_6_INDEX, &BUFFER_CONG_DQM_NOT_EMPTY_6, 13, 1, 1 },
#endif
#if defined BCM6813
	{ "US_TM_FIRST_QUEUE_MAPPING", 1, CORE_6_INDEX, &US_TM_FIRST_QUEUE_MAPPING_6, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_6_INDEX, &TX_MIRRORING_CONFIGURATION_6, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "US_TM_ETH_BBH_TX_FIFO_SIZE", 1, CORE_6_INDEX, &US_TM_ETH_BBH_TX_FIFO_SIZE_6, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "US_TM_ETH_1_BBH_TX_FIFO_SIZE", 1, CORE_6_INDEX, &US_TM_ETH_1_BBH_TX_FIFO_SIZE_6, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "US_TM_PON_BBH_TX_FIFO_SIZE", 1, CORE_6_INDEX, &US_TM_PON_BBH_TX_FIFO_SIZE_6, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_6_INDEX, &DEBUG_PRINT_CORE_LOCK_6, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "WAN_0_BBH_TX_FIFO_SIZE", 1, CORE_6_INDEX, &WAN_0_BBH_TX_FIFO_SIZE_6, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SRAM_DUMMY_STORE", 1, CORE_6_INDEX, &SRAM_DUMMY_STORE_6, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "US_BUFFER_CONG_MGT_CFG", 1, CORE_6_INDEX, &US_BUFFER_CONG_MGT_CFG_6, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "TX_MIRRORING_DISPATCHER_CREDIT_TABLE", 1, CORE_6_INDEX, &TX_MIRRORING_DISPATCHER_CREDIT_TABLE_6, 3, 1, 1 },
#endif
#if defined BCM6813
	{ "US_TM_PON_BBH_TX_ABS_COUNTER_TABLE", 1, CORE_6_INDEX, &US_TM_PON_BBH_TX_ABS_COUNTER_TABLE_6, 40, 1, 1 },
#endif
#if defined BCM6813
	{ "DEBUG_PRINT_TABLE", 1, CORE_6_INDEX, &DEBUG_PRINT_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DATA_TRACE_STACK", 1, CORE_6_INDEX, &DATA_TRACE_STACK_6, 40, 1, 1 },
#endif
#if defined BCM6813
	{ "PON_TM_ENABLED_QUEUES_TABLE", 1, CORE_6_INDEX, &PON_TM_ENABLED_QUEUES_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "US_TM_DCL_DATA_BASE", 1, CORE_6_INDEX, &US_TM_DCL_DATA_BASE_6, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "US_TM_BBH_TX_WAKE_UP_DATA_TABLE", 1, CORE_6_INDEX, &US_TM_BBH_TX_WAKE_UP_DATA_TABLE_6, 3, 1, 1 },
#endif
#if defined BCM6813
	{ "UPDATE_FIFO_TABLE", 1, CORE_6_INDEX, &UPDATE_FIFO_TABLE_6, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "US_TM_SCHEDULING_AGGREGATION_CONTEXT_VECTOR", 1, CORE_6_INDEX, &US_TM_SCHEDULING_AGGREGATION_CONTEXT_VECTOR_6, 5, 1, 1 },
#endif
#if defined BCM6813
	{ "DATA_TRACE_UPDATE_FIFO_TABLE", 1, CORE_6_INDEX, &DATA_TRACE_UPDATE_FIFO_TABLE_6, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "US_TM_PI2_PROBABILITY_CALC_DESCRIPTOR", 1, CORE_6_INDEX, &US_TM_PI2_PROBABILITY_CALC_DESCRIPTOR_6, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DATA_TRACE_PD_FIFO_TABLE", 1, CORE_6_INDEX, &DATA_TRACE_PD_FIFO_TABLE_6, 2, 1, 1 },
#endif
#if defined BCM6813
	{ "US_TM_TX_OCTETS_COUNTERS_TABLE", 1, CORE_6_INDEX, &US_TM_TX_OCTETS_COUNTERS_TABLE_6, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "FPM_GLOBAL_CFG", 1, CORE_6_INDEX, &FPM_GLOBAL_CFG_6, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_6_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_6, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "DEBUG_SCRATCHPAD", 1, CORE_6_INDEX, &DEBUG_SCRATCHPAD_6, 12, 1, 1 },
#endif
#if defined BCM6813
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_6_INDEX, &FW_ERROR_VECTOR_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SCORE_MSG_RING_DESCRIPTOR_TABLE", 1, CORE_6_INDEX, &SCORE_MSG_RING_DESCRIPTOR_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SCORE_QSTAT_RING_DESCRIPTOR_TABLE", 1, CORE_6_INDEX, &SCORE_QSTAT_RING_DESCRIPTOR_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SCORE_BNL_RING_DESCRIPTOR_TABLE", 1, CORE_6_INDEX, &SCORE_BNL_RING_DESCRIPTOR_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "US_TM_BBH_TX_ABS_COUNTER_TABLE", 1, CORE_6_INDEX, &US_TM_BBH_TX_ABS_COUNTER_TABLE_6, 6, 1, 1 },
#endif
#if defined BCM6813
	{ "US_TM_1_BBH_TX_ABS_COUNTER_TABLE", 1, CORE_6_INDEX, &US_TM_1_BBH_TX_ABS_COUNTER_TABLE_6, 5, 1, 1 },
#endif
#if defined BCM6813
	{ "DS_TM_PD_FIFO_TABLE", 1, CORE_7_INDEX, &DS_TM_PD_FIFO_TABLE_7, 128, 1, 1 },
#endif
#if defined BCM6813
	{ "DS_TM_SCHEDULING_QUEUE_TABLE", 1, CORE_7_INDEX, &DS_TM_SCHEDULING_QUEUE_TABLE_7, 64, 1, 1 },
#endif
#if defined BCM6813
	{ "TX_TASK_DS_0_STACK", 1, CORE_7_INDEX, &TX_TASK_DS_0_STACK_7, 128, 1, 1 },
#endif
#if defined BCM6813
	{ "PON_TX_FLOW_TABLE", 1, CORE_7_INDEX, &PON_TX_FLOW_TABLE_7, 256, 1, 1 },
#endif
#if defined BCM6813
	{ "DS_TM_RATE_LIMITER_BUDGET_DESCRIPTOR_TABLE", 1, CORE_7_INDEX, &DS_TM_RATE_LIMITER_BUDGET_DESCRIPTOR_TABLE_7, 64, 1, 1 },
#endif
#if defined BCM6813
	{ "DS_TM_RATE_LIMITER_PARAMS_DESCRIPTOR_TABLE", 1, CORE_7_INDEX, &DS_TM_RATE_LIMITER_PARAMS_DESCRIPTOR_TABLE_7, 64, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING_4_TASKS_PACKET_BUFFER", 1, CORE_7_INDEX, &PROCESSING_4_TASKS_PACKET_BUFFER_7, 4, 1, 1 },
#endif
#if defined BCM6813
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_7_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_7, 128, 1, 1 },
#endif
#if defined BCM6813
	{ "RX_FLOW_TABLE", 1, CORE_7_INDEX, &RX_FLOW_TABLE_7, 202, 1, 1 },
#endif
#if defined BCM6813
	{ "MUTEX_TABLE", 1, CORE_7_INDEX, &MUTEX_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "NULL_BUFFER", 1, CORE_7_INDEX, &NULL_BUFFER_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DS_TM_RATE_LIMITER_PROFILE_RESIDUE_TABLE", 1, CORE_7_INDEX, &DS_TM_RATE_LIMITER_PROFILE_RESIDUE_TABLE_7, 32, 1, 1 },
#endif
#if defined BCM6813
	{ "DS_TM_TM_FLOW_CNTR_TABLE", 1, CORE_7_INDEX, &DS_TM_TM_FLOW_CNTR_TABLE_7, 64, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING0_STACK", 1, CORE_7_INDEX, &PROCESSING0_STACK_7, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "SYSTEM_CONFIGURATION", 1, CORE_7_INDEX, &SYSTEM_CONFIGURATION_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DS_TM_RATE_LIMITER_BUDGET_VALID", 1, CORE_7_INDEX, &DS_TM_RATE_LIMITER_BUDGET_VALID_7, 4, 1, 1 },
#endif
#if defined BCM6813
	{ "TX_TASK_DS_1_STACK", 1, CORE_7_INDEX, &TX_TASK_DS_1_STACK_7, 128, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING1_STACK", 1, CORE_7_INDEX, &PROCESSING1_STACK_7, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "SCORE_CFG_TABLE", 1, CORE_7_INDEX, &SCORE_CFG_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DS_TM_BBH_TX_WAKE_UP_DATA_TABLE", 1, CORE_7_INDEX, &DS_TM_BBH_TX_WAKE_UP_DATA_TABLE_7, 2, 1, 1 },
#endif
#if defined BCM6813
	{ "DS_TM_RATE_LIMITER_PROFILE_TABLE", 1, CORE_7_INDEX, &DS_TM_RATE_LIMITER_PROFILE_TABLE_7, 32, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING2_STACK", 1, CORE_7_INDEX, &PROCESSING2_STACK_7, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "MIRRORING_SCRATCH", 1, CORE_7_INDEX, &MIRRORING_SCRATCH_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SPDTEST_GEN_PARAM", 1, CORE_7_INDEX, &SPDTEST_GEN_PARAM_7, 4, 1, 1 },
#endif
#if defined BCM6813
	{ "DS_TM_DCL_IMG", 1, CORE_7_INDEX, &DS_TM_DCL_IMG_7, 128, 1, 1 },
#endif
#if defined BCM6813
	{ "PROCESSING3_STACK", 1, CORE_7_INDEX, &PROCESSING3_STACK_7, 360, 1, 1 },
#endif
#if defined BCM6813
	{ "TUNNELS_PARSING_CFG", 1, CORE_7_INDEX, &TUNNELS_PARSING_CFG_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_7_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_7, 16, 1, 1 },
#endif
#if defined BCM6813
	{ "GENERAL_TIMER", 1, CORE_7_INDEX, &GENERAL_TIMER_7, 16, 1, 1 },
#endif
#if defined BCM6813
	{ "DS_TM_SCHEDULER_TABLE", 1, CORE_7_INDEX, &DS_TM_SCHEDULER_TABLE_7, 11, 1, 1 },
#endif
#if defined BCM6813
	{ "DS_TM_TX_QUEUE_DROP_TABLE", 1, CORE_7_INDEX, &DS_TM_TX_QUEUE_DROP_TABLE_7, 64, 1, 1 },
#endif
#if defined BCM6813
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_7_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "CSO_BAD_IPV4_HDR_CSUM_PACKETS", 1, CORE_7_INDEX, &CSO_BAD_IPV4_HDR_CSUM_PACKETS_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_7_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SCORE_COLLECT_DROP_BITMAP_TABLE", 1, CORE_7_INDEX, &SCORE_COLLECT_DROP_BITMAP_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "AQM_ENABLE_TABLE", 1, CORE_7_INDEX, &AQM_ENABLE_TABLE_7, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "VPORT_TX_FLOW_TABLE", 1, CORE_7_INDEX, &VPORT_TX_FLOW_TABLE_7, 64, 1, 1 },
#endif
#if defined BCM6813
	{ "SCORE_DATA_COLLECTION_PROFILE_TABLE", 1, CORE_7_INDEX, &SCORE_DATA_COLLECTION_PROFILE_TABLE_7, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "DS_TM_AQM_QUEUE_TABLE", 1, CORE_7_INDEX, &DS_TM_AQM_QUEUE_TABLE_7, 64, 1, 1 },
#endif
#if defined BCM6813
	{ "POLICER_PARAMS_TABLE", 1, CORE_7_INDEX, &POLICER_PARAMS_TABLE_7, 80, 1, 1 },
#endif
#if defined BCM6813
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_7_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_7, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "DS_TM_AQM_QUEUE_TIMER_TABLE", 1, CORE_7_INDEX, &DS_TM_AQM_QUEUE_TIMER_TABLE_7, 64, 1, 1 },
#endif
#if defined BCM6813
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_7_INDEX, &INGRESS_FILTER_PROFILE_TABLE_7, 16, 1, 1 },
#endif
#if defined BCM6813
	{ "VPORT_CFG_TABLE", 1, CORE_7_INDEX, &VPORT_CFG_TABLE_7, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "DS_TM_DCL_MULTIPLE_CNTR", 1, CORE_7_INDEX, &DS_TM_DCL_MULTIPLE_CNTR_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DS_TM_SECONDARY_SCHEDULER_TABLE", 1, CORE_7_INDEX, &DS_TM_SECONDARY_SCHEDULER_TABLE_7, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "DS_TM_SCHEDULER_POOL", 1, CORE_7_INDEX, &DS_TM_SCHEDULER_POOL_7, 192, 1, 1 },
#endif
#if defined BCM6813
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_7_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_7, 32, 1, 1 },
#endif
#if defined BCM6813
	{ "UPDATE_FIFO_STACK", 1, CORE_7_INDEX, &UPDATE_FIFO_STACK_7, 64, 1, 1 },
#endif
#if defined BCM6813
	{ "GENERAL_TIMER_STACK", 1, CORE_7_INDEX, &GENERAL_TIMER_STACK_7, 72, 1, 1 },
#endif
#if defined BCM6813
	{ "REGISTERS_BUFFER", 1, CORE_7_INDEX, &REGISTERS_BUFFER_7, 32, 1, 1 },
#endif
#if defined BCM6813
	{ "IPTV_CFG_TABLE", 1, CORE_7_INDEX, &IPTV_CFG_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_FPM_THRESHOLDS", 1, CORE_7_INDEX, &DHD_FPM_THRESHOLDS_7, 3, 1, 1 },
#endif
#if defined BCM6813
	{ "TASK_IDX", 1, CORE_7_INDEX, &TASK_IDX_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_7_INDEX, &BUFMNG_DESCRIPTOR_TABLE_7, 16, 1, 1 },
#endif
#if defined BCM6813
	{ "BUFFER_CONG_SCRATCHPAD", 1, CORE_7_INDEX, &BUFFER_CONG_SCRATCHPAD_7, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "RX_MIRRORING_TABLE", 1, CORE_7_INDEX, &RX_MIRRORING_TABLE_7, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "AQM_NUM_QUEUES", 1, CORE_7_INDEX, &AQM_NUM_QUEUES_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DS_TM_BBH_QUEUE_TABLE", 1, CORE_7_INDEX, &DS_TM_BBH_QUEUE_TABLE_7, 12, 1, 1 },
#endif
#if defined BCM6813
	{ "BUFMNG_STATUS_TABLE", 1, CORE_7_INDEX, &BUFMNG_STATUS_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "HOST_TX_TRUNCATE_MIRRORING_TABLE", 1, CORE_7_INDEX, &HOST_TX_TRUNCATE_MIRRORING_TABLE_7, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "BUDGET_ALLOCATION_TIMER_VALUE", 1, CORE_7_INDEX, &BUDGET_ALLOCATION_TIMER_VALUE_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "ETH_TM_TX_TRUNCATE_MIRRORING_TABLE", 1, CORE_7_INDEX, &ETH_TM_TX_TRUNCATE_MIRRORING_TABLE_7, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "DOS_DROP_REASONS_CFG", 1, CORE_7_INDEX, &DOS_DROP_REASONS_CFG_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_7_INDEX, &LOOPBACK_QUEUE_TABLE_7, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "GENERAL_TIMER_ACTION_VEC", 1, CORE_7_INDEX, &GENERAL_TIMER_ACTION_VEC_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "VPORT_CFG_EX_TABLE", 1, CORE_7_INDEX, &VPORT_CFG_EX_TABLE_7, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_7_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "BUFFER_CONG_DQM_NOT_EMPTY", 1, CORE_7_INDEX, &BUFFER_CONG_DQM_NOT_EMPTY_7, 13, 1, 1 },
#endif
#if defined BCM6813
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_7_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "TX_EXCEPTION", 1, CORE_7_INDEX, &TX_EXCEPTION_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_7_INDEX, &LOOPBACK_WAN_FLOW_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "FORCE_DSCP", 1, CORE_7_INDEX, &FORCE_DSCP_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_7_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "TR471_SPDSVC_RX_PKT_ID_TABLE", 1, CORE_7_INDEX, &TR471_SPDSVC_RX_PKT_ID_TABLE_7, 4, 1, 1 },
#endif
#if defined BCM6813
	{ "TX_MIRRORING_DISPATCHER_CREDIT_TABLE", 1, CORE_7_INDEX, &TX_MIRRORING_DISPATCHER_CREDIT_TABLE_7, 3, 1, 1 },
#endif
#if defined BCM6813
	{ "DS_TM_FIRST_QUEUE_MAPPING", 1, CORE_7_INDEX, &DS_TM_FIRST_QUEUE_MAPPING_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "CORE_ID_TABLE", 1, CORE_7_INDEX, &CORE_ID_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "CSO_DISABLE", 1, CORE_7_INDEX, &CSO_DISABLE_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DS_BUFFER_CONG_MGT_CFG", 1, CORE_7_INDEX, &DS_BUFFER_CONG_MGT_CFG_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DEBUG_PRINT_TABLE", 1, CORE_7_INDEX, &DEBUG_PRINT_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DS_TM_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_TABLE", 1, CORE_7_INDEX, &DS_TM_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_TABLE_7, 2, 1, 1 },
#endif
#if defined BCM6813
	{ "TCAM_GENERIC_FIELDS", 1, CORE_7_INDEX, &TCAM_GENERIC_FIELDS_7, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "ETH_TM_FW_FLUSH_TABLE", 1, CORE_7_INDEX, &ETH_TM_FW_FLUSH_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_7_INDEX, &RX_MIRRORING_CONFIGURATION_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_7_INDEX, &TX_MIRRORING_CONFIGURATION_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "ETH_TM_ENABLED_QUEUES_TABLE", 1, CORE_7_INDEX, &ETH_TM_ENABLED_QUEUES_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_7_INDEX, &DEBUG_PRINT_CORE_LOCK_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "SRAM_DUMMY_STORE", 1, CORE_7_INDEX, &SRAM_DUMMY_STORE_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_7_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_7_INDEX, &INGRESS_FILTER_1588_CFG_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_7_INDEX, &RX_MIRRORING_DIRECT_ENABLE_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "NAT_CACHE_CFG", 1, CORE_7_INDEX, &NAT_CACHE_CFG_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "GDX_PARAMS_TABLE", 1, CORE_7_INDEX, &GDX_PARAMS_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DS_TM_DCL_DATA_BASE", 1, CORE_7_INDEX, &DS_TM_DCL_DATA_BASE_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DS_TM_SCHEDULING_AGGREGATION_CONTEXT_VECTOR", 1, CORE_7_INDEX, &DS_TM_SCHEDULING_AGGREGATION_CONTEXT_VECTOR_7, 5, 1, 1 },
#endif
#if defined BCM6813
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_7_INDEX, &NAT_CACHE_KEY0_MASK_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "UPDATE_FIFO_TABLE", 1, CORE_7_INDEX, &UPDATE_FIFO_TABLE_7, 8, 1, 1 },
#endif
#if defined BCM6813
	{ "DS_TM_PI2_PROBABILITY_CALC_DESCRIPTOR", 1, CORE_7_INDEX, &DS_TM_PI2_PROBABILITY_CALC_DESCRIPTOR_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_7_INDEX, &TCAM_TABLE_CFG_TABLE_7, 2, 1, 1 },
#endif
#if defined BCM6813
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_7_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_7, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "DHD_HW_CFG", 1, CORE_7_INDEX, &DHD_HW_CFG_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_7_INDEX, &NATC_L2_VLAN_KEY_MASK_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_7_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_7, 31, 1, 1 },
#endif
#if defined BCM6813
	{ "DEBUG_SCRATCHPAD", 1, CORE_7_INDEX, &DEBUG_SCRATCHPAD_7, 12, 1, 1 },
#endif
#if defined BCM6813
	{ "INGRESS_FILTER_CFG", 1, CORE_7_INDEX, &INGRESS_FILTER_CFG_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "NATC_L2_TOS_MASK", 1, CORE_7_INDEX, &NATC_L2_TOS_MASK_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "FPM_GLOBAL_CFG", 1, CORE_7_INDEX, &FPM_GLOBAL_CFG_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_7_INDEX, &BITS_CALC_MASKS_TABLE_7, 4, 1, 1 },
#endif
#if defined BCM6813
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_7_INDEX, &FW_ERROR_VECTOR_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DS_TM_BBH_TX_EGRESS_COUNTER_TABLE", 1, CORE_7_INDEX, &DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_7, 2, 1, 1 },
#endif
#if defined BCM6813
	{ "MULTICAST_KEY_MASK", 1, CORE_7_INDEX, &MULTICAST_KEY_MASK_7, 1, 1, 1 },
#endif
#if defined BCM6813
	{ "DS_TM_BBH_TX_ABS_COUNTER_TABLE", 1, CORE_7_INDEX, &DS_TM_BBH_TX_ABS_COUNTER_TABLE_7, 6, 1, 1 },
#endif
#if defined BCM6813
	{ "DS_TM_1_BBH_TX_ABS_COUNTER_TABLE", 1, CORE_7_INDEX, &DS_TM_1_BBH_TX_ABS_COUNTER_TABLE_7, 5, 1, 1 },
#endif
};

TABLE_STACK_STRUCT RUNNER_STACK_TABLES[NUMBER_OF_STACK_TABLES] =
{
    { "PROCESSING0_STACK", CORE_0_INDEX, 0xc00, 360},
    { "PROCESSING1_STACK", CORE_0_INDEX, 0xe00, 360},
    { "SERVICE_QUEUES_STACK", CORE_0_INDEX, 0xf80, 128},
    { "PROCESSING2_STACK", CORE_0_INDEX, 0x2000, 360},
    { "PROCESSING3_STACK", CORE_0_INDEX, 0x2200, 360},
    { "PROCESSING4_STACK", CORE_0_INDEX, 0x2400, 360},
    { "PROCESSING5_STACK", CORE_0_INDEX, 0x2600, 360},
    { "DHD_RX_COMPLETE_0_STACK", CORE_0_INDEX, 0x2780, 128},
    { "PROCESSING6_STACK", CORE_0_INDEX, 0x2800, 360},
    { "DHD_RX_COMPLETE_1_STACK", CORE_0_INDEX, 0x2980, 128},
    { "PROCESSING7_STACK", CORE_0_INDEX, 0x2a00, 360},
    { "DHD_RX_COMPLETE_2_STACK", CORE_0_INDEX, 0x2b80, 128},
    { "DHD_TX_COMPLETE_0_STACK", CORE_0_INDEX, 0x2d40, 64},
    { "DHD_TX_COMPLETE_1_STACK", CORE_0_INDEX, 0x33c0, 64},
    { "GENERAL_TIMER_STACK", CORE_0_INDEX, 0x3580, 72},
    { "DHD_TX_COMPLETE_2_STACK", CORE_0_INDEX, 0x3700, 64},
    { "COMMON_REPROCESSING_STACK", CORE_1_INDEX, 0xa40, 64},
    { "PROCESSING0_STACK", CORE_1_INDEX, 0xe00, 360},
    { "PROCESSING1_STACK", CORE_1_INDEX, 0x2000, 360},
    { "SPDSVC_GEN_STACK", CORE_1_INDEX, 0x2180, 128},
    { "PROCESSING2_STACK", CORE_1_INDEX, 0x2200, 360},
    { "PROCESSING3_STACK", CORE_1_INDEX, 0x2400, 360},
    { "PROCESSING4_STACK", CORE_1_INDEX, 0x2600, 360},
    { "GENERAL_TIMER_STACK", CORE_1_INDEX, 0x2780, 72},
    { "PROCESSING5_STACK", CORE_1_INDEX, 0x2800, 360},
    { "PROCESSING6_STACK", CORE_1_INDEX, 0x2a00, 360},
    { "PROCESSING7_STACK", CORE_1_INDEX, 0x2c00, 360},
    { "PROCESSING0_STACK", CORE_2_INDEX, 0x2200, 360},
    { "PROCESSING1_STACK", CORE_2_INDEX, 0x2400, 360},
    { "PROCESSING2_STACK", CORE_2_INDEX, 0x2600, 360},
    { "PROCESSING3_STACK", CORE_2_INDEX, 0x2800, 360},
    { "PROCESSING4_STACK", CORE_2_INDEX, 0x2a00, 360},
    { "GENERAL_TIMER_STACK", CORE_2_INDEX, 0x2b80, 72},
    { "PROCESSING5_STACK", CORE_2_INDEX, 0x2c00, 360},
    { "PROCESSING6_STACK", CORE_2_INDEX, 0x2e00, 360},
    { "PROCESSING7_STACK", CORE_2_INDEX, 0x3000, 360},
    { "CPU_TX_0_STACK", CORE_2_INDEX, 0x3200, 320},
    { "CPU_TX_1_STACK", CORE_2_INDEX, 0x3400, 320},
    { "CPU_RECYCLE_STACK", CORE_2_INDEX, 0x3540, 32},
    { "PROCESSING0_STACK", CORE_3_INDEX, 0xc00, 360},
    { "PROCESSING1_STACK", CORE_3_INDEX, 0xe00, 360},
    { "PROCESSING2_STACK", CORE_3_INDEX, 0x2000, 360},
    { "PROCESSING3_STACK", CORE_3_INDEX, 0x2200, 360},
    { "CPU_RX_CSO_STACK", CORE_3_INDEX, 0x2380, 128},
    { "PROCESSING4_STACK", CORE_3_INDEX, 0x2400, 360},
    { "PROCESSING5_STACK", CORE_3_INDEX, 0x2600, 360},
    { "PROCESSING6_STACK", CORE_3_INDEX, 0x2800, 360},
    { "PROCESSING7_STACK", CORE_3_INDEX, 0x2a00, 360},
    { "GENERAL_TIMER_STACK", CORE_3_INDEX, 0x2b80, 72},
    { "SPU_REQUEST_STACK", CORE_3_INDEX, 0x2ec0, 64},
    { "SPDSVC_ANALYZER_STACK", CORE_3_INDEX, 0x3200, 176},
    { "CPU_RX_STACK", CORE_3_INDEX, 0x3400, 136},
    { "CPU_FPM_BUFMNG_RELEASE_STACK", CORE_4_INDEX, 0xa80, 128},
    { "PROCESSING0_STACK", CORE_4_INDEX, 0x2200, 360},
    { "PROCESSING1_STACK", CORE_4_INDEX, 0x2400, 360},
    { "PROCESSING2_STACK", CORE_4_INDEX, 0x2600, 360},
    { "PROCESSING3_STACK", CORE_4_INDEX, 0x2800, 360},
    { "SPU_RESPONSE_STACK", CORE_4_INDEX, 0x2980, 96},
    { "PROCESSING4_STACK", CORE_4_INDEX, 0x2a00, 360},
    { "DHD_TIMER_STACK", CORE_4_INDEX, 0x2be0, 32},
    { "PROCESSING5_STACK", CORE_4_INDEX, 0x2c00, 360},
    { "PROCESSING6_STACK", CORE_4_INDEX, 0x2e00, 360},
    { "PROCESSING7_STACK", CORE_4_INDEX, 0x3000, 360},
    { "DHD_TX_POST_UPDATE_FIFO_STACK", CORE_4_INDEX, 0x3320, 32},
    { "DHD_TX_POST_0_STACK", CORE_4_INDEX, 0x3500, 136},
    { "DHD_TX_POST_1_STACK", CORE_4_INDEX, 0x3600, 136},
    { "DHD_TX_POST_2_STACK", CORE_4_INDEX, 0x3700, 136},
    { "DIRECT_FLOW_STACK", CORE_5_INDEX, 0x680, 128},
    { "REPORTING_STACK", CORE_5_INDEX, 0xfa0, 32},
    { "PROCESSING0_STACK", CORE_5_INDEX, 0x2200, 360},
    { "PROCESSING1_STACK", CORE_5_INDEX, 0x2400, 360},
    { "PROCESSING2_STACK", CORE_5_INDEX, 0x2600, 360},
    { "GENERAL_TIMER_STACK", CORE_5_INDEX, 0x2780, 72},
    { "PROCESSING3_STACK", CORE_5_INDEX, 0x2800, 360},
    { "PROCESSING4_STACK", CORE_5_INDEX, 0x2a00, 360},
    { "PROCESSING5_STACK", CORE_5_INDEX, 0x2c00, 360},
    { "PROCESSING6_STACK", CORE_5_INDEX, 0x2e00, 360},
    { "PROCESSING7_STACK", CORE_5_INDEX, 0x3000, 360},
    { "CPU_TX_MCORE_STACK", CORE_5_INDEX, 0x3200, 320},
    { "CPU_RECYCLE_STACK", CORE_5_INDEX, 0x3360, 32},
    { "TX_TASK_US_0_STACK", CORE_6_INDEX, 0xb80, 128},
    { "TX_TASK_US_1_STACK", CORE_6_INDEX, 0xf80, 128},
    { "TX_TASK_PON_STACK", CORE_6_INDEX, 0x1780, 128},
    { "UPDATE_FIFO_STACK", CORE_6_INDEX, 0x2740, 64},
    { "GENERAL_TIMER_STACK", CORE_6_INDEX, 0x2800, 72},
    { "DATA_TRACE_STACK", CORE_6_INDEX, 0x2b00, 40},
    { "TX_TASK_DS_0_STACK", CORE_7_INDEX, 0xa80, 128},
    { "PROCESSING0_STACK", CORE_7_INDEX, 0x1c00, 360},
    { "TX_TASK_DS_1_STACK", CORE_7_INDEX, 0x1d80, 128},
    { "PROCESSING1_STACK", CORE_7_INDEX, 0x1e00, 360},
    { "PROCESSING2_STACK", CORE_7_INDEX, 0x2000, 360},
    { "PROCESSING3_STACK", CORE_7_INDEX, 0x2200, 360},
    { "UPDATE_FIFO_STACK", CORE_7_INDEX, 0x2c40, 64},
    { "GENERAL_TIMER_STACK", CORE_7_INDEX, 0x2c80, 72}
};
