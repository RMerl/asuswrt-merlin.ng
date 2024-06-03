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
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DS_TM_PD_FIFO_TABLE =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_PD_FIFO_TABLE =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DS_TM_SCHEDULING_QUEUE_TABLE =
{
	8,
	{
		{ dump_RDD_SCHEDULING_QUEUE_DESCRIPTOR, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT BASIC_SCHEDULER_TABLE_DS =
{
	16,
	{
		{ dump_RDD_BASIC_SCHEDULER_DESCRIPTOR, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING_4_TASKS_PACKET_BUFFER =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT BASIC_RATE_LIMITER_TABLE_DS =
{
	16,
	{
		{ dump_RDD_BASIC_RATE_LIMITER_DESCRIPTOR, 0x1800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, 0x1c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TM_BBH_TX_WAKE_UP_DATA_TABLE =
{
	8,
	{
		{ dump_RDD_BBH_TX_WAKE_UP_DATA_ENTRY, 0x1f68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, 0x1f70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, 0x1f80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FPI_MODE_TO_NATC_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1ffc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_RATE_LIMITER_TABLE =
{
	16,
	{
		{ dump_RDD_BASIC_RATE_LIMITER_DESCRIPTOR, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2420 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DS_TM_TM_FLOW_CNTR_TABLE =
{
	1,
	{
		{ dump_RDD_TM_FLOW_CNTR_ENTRY, 0x2440 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DS_BUFFER_CONG_MGT_CFG =
{
	68,
	{
		{ dump_RDD_BUFFER_CONG_MGT, 0x2480 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FPI_MODE_SEQ_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x24c4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TM_1_BBH_TX_WAKE_UP_DATA_TABLE =
{
	8,
	{
		{ dump_RDD_BBH_TX_WAKE_UP_DATA_ENTRY, 0x24c8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DS_TM_FLUSH_CFG_CPU_TABLE =
{
	16,
	{
		{ dump_RDD_FLUSH_CFG_ENTRY, 0x24d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0x24e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DS_TM_AQM_QUEUE_TABLE =
{
	4,
	{
		{ dump_RDD_AQM_QUEUE_DESCRIPTOR, 0x2500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT NULL_BUFFER =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x2768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DS_TM_FLUSH_CFG_FW_TABLE =
{
	16,
	{
		{ dump_RDD_FLUSH_CFG_ENTRY, 0x2770 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SPDSVC_ANALYZER_PD_FIFO_TABLE =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x2780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT VPORT_TX_FLOW_TABLE =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, 0x27c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x29e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_FLUSH_CFG_FW_TABLE =
{
	16,
	{
		{ dump_RDD_FLUSH_CFG_ENTRY, 0x29f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DS_TM_TX_QUEUE_DROP_TABLE =
{
	8,
	{
		{ dump_RDD_PACKETS_AND_BYTES, 0x2b68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DS_TM_FLUSH_CFG_CURRENT_TABLE =
{
	16,
	{
		{ dump_RDD_FLUSH_CFG_ENTRY, 0x2d70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT AQM_SQ_TABLE =
{
	2,
	{
		{ dump_RDD_AQM_SQ_ENTRY, 0x2d80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT UPDATE_FIFO_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2dc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SPDSVC_ANALYZER_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE =
{
	8,
	{
		{ dump_RDD_SCHEDULING_QUEUE_DESCRIPTOR, 0x2f00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT COMPLEX_SCHEDULER_TABLE_DS =
{
	64,
	{
		{ dump_RDD_COMPLEX_SCHEDULER_DESCRIPTOR, 0x3000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, 0x3100 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x31b4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_SCRATCHPAD =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x31b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_COMPLEX_SCHEDULER_TABLE =
{
	64,
	{
		{ dump_RDD_COMPLEX_SCHEDULER_DESCRIPTOR, 0x31c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT UDPSPDT_STREAM_RX_STAT_TABLE =
{
	48,
	{
		{ dump_RDD_UDPSPDT_STREAM_RX_STAT, 0x3200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT BUFFER_CONG_SCRATCHPAD =
{
	8,
	{
		{ dump_RDD_BUFFER_CONG_Q_OCCUPANCY, 0x32c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, 0x3300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SQ_TX_QUEUE_DROP_TABLE =
{
	8,
	{
		{ dump_RDD_PACKETS_AND_BYTES, 0x33a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0x34a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x34c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT AQM_NUM_QUEUES =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x34fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, 0x3500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_AQM_QUEUE_TABLE =
{
	4,
	{
		{ dump_RDD_AQM_QUEUE_DESCRIPTOR, 0x3580 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT NATC_TBL_CFG =
{
	24,
	{
		{ dump_RDD_NATC_TBL_CONFIGURATION, 0x3600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT BUFFER_CONG_DQM_NOT_EMPTY =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3660 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, 0x3680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT BUDGET_ALLOCATION_TIMER_VALUE =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x36be },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DS_TM_PI2_PROBABILITY_CALC_DESCRIPTOR =
{
	52,
	{
		{ dump_RDD_PI2_PROBABILITY_CALC_DESCRIPTOR, 0x36c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TASK_IDX =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x36f4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT MIRRORING_SCRATCH =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x36f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CODEL_BIAS_SLOPE_TABLE =
{
	4,
	{
		{ dump_RDD_CODEL_BIAS_SLOPE, 0x3700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FPI_CFG_TABLE =
{
	2,
	{
		{ dump_RDD_FPI_CFG, 0x372c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x372e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_FLUSH_CFG_CURRENT_TABLE =
{
	16,
	{
		{ dump_RDD_FLUSH_CFG_ENTRY, 0x3730 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT UPDATE_FIFO_TABLE =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x3740 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DS_TM_SCHEDULING_QUEUE_AGING_VECTOR =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3760 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DS_TM_FLUSH_CFG_ENABLE_TABLE =
{
	2,
	{
		{ dump_RDD_FLUSH_CFG_ENABLE_ENTRY, 0x3774 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_FLUSH_CFG_ENABLE_TABLE =
{
	2,
	{
		{ dump_RDD_FLUSH_CFG_ENABLE_ENTRY, 0x3776 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, 0x3778 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SPDSVC_ANALYZER_UPDATE_FIFO_TABLE =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x3780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_SCHEDULING_QUEUE_AGING_VECTOR =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x37a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x37b4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_BUDGET_ALLOCATION_TIMER_VALUE =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x37b6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, 0x37b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_UPDATE_FIFO_TABLE =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x37c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DS_TM_SCHEDULING_AGGREGATION_CONTEXT_VECTOR =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x37e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x37f4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SCHEDULING_FLUSH_GLOBAL_CFG =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x37f5 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x37f6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FORCE_DSCP =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x37f7 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, 0x37f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DS_TM_BBH_QUEUE_TABLE =
{
	4,
	{
		{ dump_RDD_BBH_QUEUE_DESCRIPTOR, 0x3800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE =
{
	16,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, 0x3830 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0x3840 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x385f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT AQM_ENABLE_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3860 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DS_TM_FIRST_QUEUE_MAPPING =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3874 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_FIRST_QUEUE_MAPPING =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3876 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x3878 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3880 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_REDIRECT_MODE =
{
	1,
	{
		{ dump_RDD_CPU_REDIRECT_MODE_ENTRY, 0x389f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x38a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SQ_TM_BUFMNG_STATUS_TABLE =
{
	16,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, 0x38d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DS_TM_CODEL_DROP_DESCRIPTOR =
{
	20,
	{
		{ dump_RDD_CODEL_DROP_DESCRIPTOR, 0x38e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x38f4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x38f6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT BASIC_SCHEDULER_WAKEUP_CMD_DS =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x38f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x38fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, 0x38fd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x38fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x38ff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_CODEL_DROP_DESCRIPTOR =
{
	20,
	{
		{ dump_RDD_CODEL_DROP_DESCRIPTOR, 0x3900 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3914 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3915 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT COMPLEX_SCHEDULER_WAKEUP_CMD_DS =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3918 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_RATE_LIMITER_VALID_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3920 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SPDSVC_ANALYZER_PARAMS_TABLE =
{
	16,
	{
		{ dump_RDD_SPDSVC_ANALYZER_PARAMS, 0x3930 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3940 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3950 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT RATE_LIMITER_VALID_TABLE_DS =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3960 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, 0x3970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, 0x3980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DS_TM_FLUSH_DISPATCHER_CREDIT_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x39a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_AQM_TIMER_DISPATCHER_CREDIT_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x39b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT IPTV_CONFIGURATION_TABLE =
{
	12,
	{
		{ dump_RDD_HW_IPTV_CONFIGURATION, 0x39c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x39d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x39e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_DISPATCHER_CREDIT_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x39f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SPDSVC_BBH_TX_PARAMS_TABLE =
{
	12,
	{
		{ dump_RDD_SPDSVC_BBH_TX_PARAMS, 0x3a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_DISPATCHER_CREDIT_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3a10 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT UDPSPDT_SCRATCH_TABLE =
{
	8,
	{
		{ dump_RDD_UDPSPDT_SCRATCH_IPERF3_RX, 0x3a20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, 0x3a28 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, 0x3a30 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT AQM_TIMER_DISPATCHER_REPLY =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3a40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3a48 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, 0x3a50 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3a60 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3a68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, 0x3a70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3a78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TM_BBH_TX_EGRESS_COUNTER_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3a80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, 0x3aa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TM_BBH_TX_ABS_COUNTER_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3ac0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TM_1_BBH_TX_EGRESS_COUNTER_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TM_1_BBH_TX_ABS_COUNTER_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3b40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FC_FLOW_IP_ADDRESSES_TABLE =
{
	48,
	{
		{ dump_RDD_FC_FLOW_IP_ADDRESSES_ENTRY, 0x3d00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT US_TM_PD_FIFO_TABLE_1 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT BASIC_RATE_LIMITER_TABLE_US_1 =
{
	16,
	{
		{ dump_RDD_BASIC_RATE_LIMITER_DESCRIPTOR, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT US_TM_SCHEDULING_QUEUE_TABLE_1 =
{
	8,
	{
		{ dump_RDD_SCHEDULING_QUEUE_DESCRIPTOR, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT BASIC_SCHEDULER_TABLE_US_1 =
{
	16,
	{
		{ dump_RDD_BASIC_SCHEDULER_DESCRIPTOR, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING_4_TASKS_PACKET_BUFFER_1 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_1 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, 0x1800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TM_BBH_TX_WAKE_UP_DATA_TABLE_1 =
{
	8,
	{
		{ dump_RDD_BBH_TX_WAKE_UP_DATA_ENTRY, 0x1b68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_1 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, 0x1b70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_1 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, 0x1b80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FPI_MODE_TO_NATC_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1bfc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TM_1_BBH_TX_WAKE_UP_DATA_TABLE_1 =
{
	8,
	{
		{ dump_RDD_BBH_TX_WAKE_UP_DATA_ENTRY, 0x1d68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT US_TM_FLUSH_CFG_CPU_TABLE_1 =
{
	16,
	{
		{ dump_RDD_FLUSH_CFG_ENTRY, 0x1d70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT US_BUFFER_CONG_MGT_CFG_1 =
{
	68,
	{
		{ dump_RDD_BUFFER_CONG_MGT, 0x1d80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FPI_MODE_SEQ_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1dc4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT NULL_BUFFER_1 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x1dc8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT US_TM_FLUSH_CFG_FW_TABLE_1 =
{
	16,
	{
		{ dump_RDD_FLUSH_CFG_ENTRY, 0x1dd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1de0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_1 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x1f68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT US_TM_FLUSH_CFG_CURRENT_TABLE_1 =
{
	16,
	{
		{ dump_RDD_FLUSH_CFG_ENTRY, 0x1f70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT US_TM_TM_FLOW_CNTR_TABLE_1 =
{
	1,
	{
		{ dump_RDD_TM_FLOW_CNTR_ENTRY, 0x1f80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT VPORT_TX_FLOW_TABLE_1 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, 0x1fc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT US_TM_TX_QUEUE_DROP_TABLE_1 =
{
	8,
	{
		{ dump_RDD_PACKETS_AND_BYTES, 0x2168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_1 =
{
	16,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, 0x23f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT US_TM_AQM_QUEUE_TABLE_1 =
{
	4,
	{
		{ dump_RDD_AQM_QUEUE_DESCRIPTOR, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT COMPLEX_SCHEDULER_TABLE_US_1 =
{
	64,
	{
		{ dump_RDD_COMPLEX_SCHEDULER_DESCRIPTOR, 0x2500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_1 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x26b4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT MIRRORING_SCRATCH_1 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x26b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT UPDATE_FIFO_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x26c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_1 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, 0x2700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_1 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0x27a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT BUFFER_CONG_SCRATCHPAD_1 =
{
	8,
	{
		{ dump_RDD_BUFFER_CONG_Q_OCCUPANCY, 0x27c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_1 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2880 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT AQM_NUM_QUEUES_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x28be },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_1 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, 0x28c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT BUDGET_ALLOCATION_TIMER_VALUE_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x28fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT NATC_TBL_CFG_1 =
{
	24,
	{
		{ dump_RDD_NATC_TBL_CONFIGURATION, 0x2900 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_1 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0x2960 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT US_TM_PI2_PROBABILITY_CALC_DESCRIPTOR_1 =
{
	52,
	{
		{ dump_RDD_PI2_PROBABILITY_CALC_DESCRIPTOR, 0x2980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TASK_IDX_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x29b4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_1 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, 0x29b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CODEL_BIAS_SLOPE_TABLE_1 =
{
	4,
	{
		{ dump_RDD_CODEL_BIAS_SLOPE, 0x29c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FPI_CFG_TABLE_1 =
{
	2,
	{
		{ dump_RDD_FPI_CFG, 0x29ec },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x29ee },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DIRECT_FLOW_PD_TABLE_1 =
{
	16,
	{
		{ dump_RDD_PROCESSING_RX_DESCRIPTOR, 0x29f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT US_TM_BBH_QUEUE_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BBH_QUEUE_DESCRIPTOR, 0x2a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2a30 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT UPDATE_FIFO_TABLE_1 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x2a40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT BUFFER_CONG_DQM_NOT_EMPTY_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2a60 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_1 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0x2a80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2a9f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT US_TM_SCHEDULING_QUEUE_AGING_VECTOR_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2aa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT US_TM_FLUSH_CFG_ENABLE_TABLE_1 =
{
	2,
	{
		{ dump_RDD_FLUSH_CFG_ENABLE_ENTRY, 0x2ab4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2ab6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_1 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, 0x2ab8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2ac0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DIRECT_FLOW_WAN_VIQ_EXCLUSIVE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2adf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2ae0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT OVERALL_RATE_LIMITER_TABLE_1 =
{
	16,
	{
		{ dump_RDD_OVERALL_RATE_LIMITER_DESCRIPTOR, 0x2b10 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT US_TM_SCHEDULING_AGGREGATION_CONTEXT_VECTOR_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2b20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2b34 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2b35 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT US_TM_FIRST_QUEUE_MAPPING_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2b36 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_1 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, 0x2b38 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT AQM_ENABLE_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2b40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2b54 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT US_TM_ETH_BBH_TX_FIFO_SIZE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2b55 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_1 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x2b56 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_1 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x2b58 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT US_TM_CODEL_DROP_DESCRIPTOR_1 =
{
	20,
	{
		{ dump_RDD_CODEL_DROP_DESCRIPTOR, 0x2b60 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_1 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x2b74 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT US_TM_ETH_1_BBH_TX_FIFO_SIZE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2b76 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_REDIRECT_MODE_1 =
{
	1,
	{
		{ dump_RDD_CPU_REDIRECT_MODE_ENTRY, 0x2b77 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT BASIC_SCHEDULER_WAKEUP_CMD_US_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2b78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2b7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_1 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, 0x2b7d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2b7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT MAC_TYPE_1 =
{
	1,
	{
		{ dump_RDD_MAC_TYPE_ENTRY, 0x2b7f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2b80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_1 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, 0x2b90 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_1 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x2b9c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2b9d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2b9e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT RATE_LIMITER_VALID_TABLE_US_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2ba0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT US_TM_FLUSH_DISPATCHER_CREDIT_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2bb0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_1 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, 0x2bc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT IPTV_CONFIGURATION_TABLE_1 =
{
	12,
	{
		{ dump_RDD_HW_IPTV_CONFIGURATION, 0x2be0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_1 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x2bf0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SPDSVC_BBH_TX_PARAMS_TABLE_1 =
{
	12,
	{
		{ dump_RDD_SPDSVC_BBH_TX_PARAMS, 0x2c10 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_DISPATCHER_CREDIT_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2c20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_1 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, 0x2c30 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TM_BBH_TX_EGRESS_COUNTER_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2c40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT COMPLEX_SCHEDULER_WAKEUP_CMD_US_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2c48 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_1 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, 0x2c50 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2c60 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_1 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, 0x2c68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2c70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2c78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TM_BBH_TX_ABS_COUNTER_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2c80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_1 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, 0x2c88 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2c90 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_1 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, 0x2ca0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TM_1_BBH_TX_EGRESS_COUNTER_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2cc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TM_1_BBH_TX_ABS_COUNTER_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2d00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FC_FLOW_IP_ADDRESSES_TABLE_1 =
{
	48,
	{
		{ dump_RDD_FC_FLOW_IP_ADDRESSES_ENTRY, 0x3d00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_TX_SCRATCHPAD_2 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT QM_QUEUE_TO_TX_FLOW_TABLE_PTR_TABLE_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0xd00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_2 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0xf68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_INTERRUPT_COALESCING_TABLE_2 =
{
	16,
	{
		{ dump_RDD_CPU_INTERRUPT_COALESCING_ENTRY, 0xf70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_TX_DBG_CNTRS_TABLE_2 =
{
	64,
	{
		{ dump_RDD_CPU_TX_DBG_CNTRS, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER_2 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_2 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_RX_INTERRUPT_ID_DDR_ADDR_2 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0x2368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_2 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, 0x2370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_2 =
{
	8,
	{
		{ dump_RDD_GENERAL_TIMER_ENTRY, 0x2380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT NULL_BUFFER_2 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x2568 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT BUFFER_ALLOC_REPLY_2 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x2570 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_2 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, 0x2580 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_INT_INTERRUPT_SCRATCH_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x25fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_2 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x2768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_2 =
{
	16,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, 0x2770 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SG_DESC_TABLE_2 =
{
	48,
	{
		{ dump_RDD_SG_DESC_ENTRY, 0x2780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x27e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE_2 =
{
	8,
	{
		{ dump_RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY, 0x2968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT VPORT_TX_FLOW_TABLE_2 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, 0x2980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x29c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2b68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_TX_RING_INDICES_VALUES_TABLE_2 =
{
	4,
	{
		{ dump_RDD_CPU_TX_RING_INDICES, 0x2be8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_2 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, 0x2bf0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FPI_MODE_TO_NATC_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2bfc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_2 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, 0x2d68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2d70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FPI_MODE_SEQ_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2d7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2d80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT AQM_NUM_QUEUES_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2dbe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_2 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, 0x2dc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2dfe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_2 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, 0x2f68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_TX_INGRESS_DISPATCHER_CREDIT_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2f70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2f7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT RING_CPU_TX_DESCRIPTOR_DATA_TABLE_2 =
{
	16,
	{
		{ dump_RDD_RING_CPU_TX_DESCRIPTOR, 0x2f80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_2 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0x2fa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_2 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0x2fc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2fe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_TX_0_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_2 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0x3140 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x315f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT AQM_ENABLE_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3160 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TASK_IDX_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3174 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_2 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, 0x3178 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x319f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x31a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_TX_RING_DESCRIPTOR_TABLE_2 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x31d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT IPTV_CONFIGURATION_TABLE_2 =
{
	12,
	{
		{ dump_RDD_HW_IPTV_CONFIGURATION, 0x31f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FPI_CFG_TABLE_2 =
{
	2,
	{
		{ dump_RDD_FPI_CFG, 0x31fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_ACTION_VEC_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x31fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_TX_1_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3340 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_2 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x3350 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x335c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x335e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x335f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_2 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, 0x3360 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_2 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x337c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_REDIRECT_MODE_2 =
{
	1,
	{
		{ dump_RDD_CPU_REDIRECT_MODE_ENTRY, 0x337e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x337f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_TX_SYNC_FIFO_TABLE_2 =
{
	8,
	{
		{ dump_RDD_CPU_TX_SYNC_FIFO_ENTRY, 0x3380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3390 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_2 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, 0x339c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x339d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_2 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x339e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x339f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_DESCRIPTOR_TABLE_2 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x33a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE_2 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0x33b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT QUEUE_THRESHOLD_VECTOR_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x33c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_INTERRUPT_SCRATCH_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x33d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_2 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x33d8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_2 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, 0x33e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_2 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, 0x33f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x33fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_SRAM_PD_FIFO_2 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x3400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_2 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, 0x3500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_2 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, 0x35b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x35c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_SHADOW_RD_IDX_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x35c8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_SHADOW_WR_IDX_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x35d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x35d8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_2 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, 0x35e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x35e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_NEXT_PTR_TABLE_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x35f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_2 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, 0x35f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_2 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, 0x3600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x36a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SG_CONTEXT_TABLE_2 =
{
	64,
	{
		{ dump_RDD_SG_CONTEXT_ENTRY, 0x3700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_2 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, 0x3800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT NATC_TBL_CFG_2 =
{
	24,
	{
		{ dump_RDD_NATC_TBL_CONFIGURATION, 0x3900 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FC_FLOW_IP_ADDRESSES_TABLE_2 =
{
	48,
	{
		{ dump_RDD_FC_FLOW_IP_ADDRESSES_ENTRY, 0x3d00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_3 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_3 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0x368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_INTERRUPT_COALESCING_TABLE_3 =
{
	16,
	{
		{ dump_RDD_CPU_INTERRUPT_COALESCING_ENTRY, 0x370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_CACHE_TABLE_3 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_RX_SCRATCHPAD_3 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_RX_INTERRUPT_ID_DDR_ADDR_3 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0x968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_DESCRIPTOR_TABLE_3 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_RX_METER_TABLE_3 =
{
	8,
	{
		{ dump_RDD_CPU_RX_METER_ENTRY, 0x980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xa00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT NULL_BUFFER_3 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0xb68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_RX_AUX_INT_SCRATCHPAD_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0xb70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_RX_COPY_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xb80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_3 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, 0xd68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_RX_PSRAM_GET_NEXT_SCRATCHPAD_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0xd70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_RXQ_SCRATCH_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xd80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE_3 =
{
	8,
	{
		{ dump_RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY, 0xf68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_3 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, 0xf70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_3 =
{
	8,
	{
		{ dump_RDD_GENERAL_TIMER_ENTRY, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER_3 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_INDEX_DDR_ADDR_TABLE_3 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0x2168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_3 =
{
	16,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, 0x2170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_3 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, 0x2180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_INT_INTERRUPT_SCRATCH_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x21fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_3 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, 0x2368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_REASON_AND_VPORT_TO_METER_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_3 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, 0x23f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FPI_MODE_TO_NATC_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2568 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FPI_MODE_SEQ_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x256c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_3 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, 0x2570 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x257c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_RX_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2580 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_RX_COPY_DISPATCHER_CREDIT_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x25d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_RX_INTERRUPT_SCRATCH_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x25dc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_RSV_TABLE_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x25e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_SRAM_PD_FIFO_3 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_RING_INTERRUPT_COUNTER_TABLE_3 =
{
	8,
	{
		{ dump_RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY, 0x2700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT VPORT_TX_FLOW_TABLE_3 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, 0x27c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_RX_COPY_PD_FIFO_TABLE_3 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_REASON_TO_METER_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x28c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_3 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, 0x2900 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TASK_IDX_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x29b4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_3 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, 0x29b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT IPV6_HOST_ADDRESS_CRC_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x29c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CSO_CONTEXT_TABLE_3 =
{
	176,
	{
		{ dump_RDD_CSO_CONTEXT_ENTRY, 0x2a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT IPTV_CONFIGURATION_TABLE_3 =
{
	12,
	{
		{ dump_RDD_HW_IPTV_CONFIGURATION, 0x2ab0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT AQM_NUM_QUEUES_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2abc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2abe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2ac0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_3 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, 0x2b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT IPV4_HOST_ADDRESS_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2ba0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2bc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_ACTION_VEC_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2bfe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_3 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, 0x2c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2c80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT NATC_TBL_CFG_3 =
{
	24,
	{
		{ dump_RDD_NATC_TBL_CONFIGURATION, 0x2d00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2d60 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_3 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, 0x2d80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2dbe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT UPDATE_FIFO_TABLE_3 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x2dc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_3 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0x2de0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_VPORT_TO_METER_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_3 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x2e28 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_3 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x2e30 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FPI_CFG_TABLE_3 =
{
	2,
	{
		{ dump_RDD_FPI_CFG, 0x2e3c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_INTERRUPT_THRESHOLD_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2e3e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_RX_COPY_UPDATE_FIFO_TABLE_3 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x2e40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_3 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0x2e60 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PD_FIFO_TABLE_3 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x2e80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_REASON_TO_TC_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2ea0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TC_TO_CPU_RXQ_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2ee0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT EXC_TC_TO_CPU_RXQ_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2f20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2f60 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_RXQ_DATA_BUF_TYPE_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2f80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT AQM_ENABLE_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2fa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_INTERRUPT_COUNTER_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2fb4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2fb6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2fb7 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_3 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, 0x2fb8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2fbf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_3 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0x2fc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2fdf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_3 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, 0x2fe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_3 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x2ffc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_CACHE_OFFSET_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2ffe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_REDIRECT_MODE_3 =
{
	1,
	{
		{ dump_RDD_CPU_REDIRECT_MODE_ENTRY, 0x2fff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_RING_DESCRIPTORS_TABLE_3 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x3000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CSO_DISABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x319f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x31a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x31d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x31dc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_3 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, 0x31dd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x31de },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_3 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x31df },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT BCM_SPDSVC_STREAM_RX_TS_TABLE_3 =
{
	12,
	{
		{ dump_RDD_SPDSVC_RX_TS_STAT, 0x31e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x31ec },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x31ed },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_DESCRIPTOR_TABLE_3 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x31f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT VPORT_TO_CPU_OBJ_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE_3 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0x3228 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_SHADOW_RD_IDX_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3238 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3240 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_INTERRUPT_COALESCING_TABLE_3 =
{
	16,
	{
		{ dump_RDD_CPU_INTERRUPT_COALESCING_ENTRY, 0x3250 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_INTERRUPT_SCRATCH_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3260 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_RX_LOCAL_SCRATCH_3 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x3268 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_SHADOW_WR_IDX_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3278 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_3 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, 0x3280 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_3 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, 0x3290 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x32a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x32a8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_NEXT_PTR_TABLE_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x32b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x32b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_3 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, 0x32c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_3 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, 0x32c8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x32d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FC_FLOW_IP_ADDRESSES_TABLE_3 =
{
	48,
	{
		{ dump_RDD_FC_FLOW_IP_ADDRESSES_ENTRY, 0x3d00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_FLOW_RING_CACHE_CTX_TABLE_4 =
{
	16,
	{
		{ dump_RDD_DHD_FLOW_RING_CACHE_CTX_ENTRY, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_DOORBELL_TX_POST_VALUE_4 =
{
	4,
	{
		{ dump_RDD_DHD_DOORBELL, 0x300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_AUX_INFO_CACHE_TABLE_4 =
{
	16,
	{
		{ dump_RDD_DHD_AUX_INFO_CACHE_ENTRY, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_4 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, 0x700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FPI_MODE_TO_NATC_TABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x7b4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT NULL_BUFFER_4 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x7b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT VPORT_TX_FLOW_TABLE_4 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, 0x7c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_POST_COMMON_RADIO_DATA_4 =
{
	184,
	{
		{ dump_RDD_DHD_POST_COMMON_RADIO_ENTRY, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_4 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, 0xa28 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_4 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, 0xa30 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_4 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0xa40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT AQM_NUM_QUEUES_4 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0xa7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_4 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, 0xa80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FPI_MODE_SEQ_TABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xafc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_4 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, 0xb00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0xba0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_4 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, 0xbc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_4 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0xbfe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_4 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT MIRRORING_SCRATCH_4 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0xf68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_4 =
{
	16,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, 0xf70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_BACKUP_INDEX_CACHE_4 =
{
	32,
	{
		{ dump_RDD_DHD_BACKUP_IDX_CACHE_TABLE, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_4 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0xfe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER_4 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_4 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0x2168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_L2_HEADER_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_BACKUP_BASE_ADDR_4 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x21c8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_4 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, 0x21d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x21dc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_4 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0x21e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_4 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, 0x23e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT IPTV_CONFIGURATION_TABLE_4 =
{
	12,
	{
		{ dump_RDD_HW_IPTV_CONFIGURATION, 0x23f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TASK_IDX_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x23fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_4 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, 0x2568 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_4 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x2570 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FPI_CFG_TABLE_4 =
{
	2,
	{
		{ dump_RDD_FPI_CFG, 0x257c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_4 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x257e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_CPU_TX_POST_RING_DESCRIPTOR_TABLE_4 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x2580 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x25b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SPDSVC_WLAN_TXPOST_PARAMS_TABLE_4 =
{
	2,
	{
		{ dump_RDD_SPDSVC_WLAN_TXPOST_PARAMS, 0x25bc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x25be },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x25bf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_CODEL_BIAS_SLOPE_TABLE_4 =
{
	4,
	{
		{ dump_RDD_DHD_CODEL_BIAS_SLOPE, 0x25c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x25ec },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x25ed },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_4 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x25ee },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_CPU_INT_ID_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x25f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_REDIRECT_MODE_4 =
{
	1,
	{
		{ dump_RDD_CPU_REDIRECT_MODE_ENTRY, 0x25fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x25fd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_4 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, 0x25fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x25ff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_4 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, 0x2768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_MIRRORING_DISPATCHER_CREDIT_TABLE_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2770 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_4 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x277c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x277d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x277e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE_4 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x2780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_4 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, 0x27b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_UPDATE_FIFO_TABLE_4 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x27c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_TIMER_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x27e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_4 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x2968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_4 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, 0x2970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_UPDATE_FIFO_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT AQM_ENABLE_TABLE_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x29a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_4 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, 0x29b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_4 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0x29c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_4 =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, 0x29e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x29f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_4 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2b68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_4 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, 0x2b70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2b78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2b80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2ba0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_4 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, 0x2be0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2d80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2da0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_4 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, 0x2dc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_PD_FIFO_TABLE_4 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x2e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_FLOW_RING_BUFFER_4 =
{
	48,
	{
		{ dump_RDD_DHD_TX_POST_DESCRIPTOR, 0x2f00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_0_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_1_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3100 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_2_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_4 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, 0x3300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT NATC_TBL_CFG_4 =
{
	24,
	{
		{ dump_RDD_NATC_TBL_CONFIGURATION, 0x3400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_FLOW_RING_CACHE_LKP_TABLE_4 =
{
	2,
	{
		{ dump_RDD_DHD_FLOW_RING_CACHE_LKP_ENTRY, 0x3500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FC_FLOW_IP_ADDRESSES_TABLE_4 =
{
	48,
	{
		{ dump_RDD_FC_FLOW_IP_ADDRESSES_ENTRY, 0x3d00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_STREAM_TABLE_5 =
{
	376,
	{
		{ dump_RDD_TCPSPDTEST_STREAM, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x5e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_RX_INTERRUPT_ID_DDR_ADDR_5 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0x768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_5 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, 0x770 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_SCRATCHPAD_5 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CRYPTO_SESSION_PARAMS_TABLE_5 =
{
	12,
	{
		{ dump_RDD_CRYPTO_SESSION_PARAMS, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_5 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, 0xb00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PKTGEN_NO_SBPM_HDRS_CNTR_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0xbb4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT NULL_BUFFER_5 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0xbb8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT VPORT_TX_FLOW_TABLE_5 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, 0xbc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PKTGEN_TX_STREAM_TABLE_5 =
{
	136,
	{
		{ dump_RDD_PKTGEN_TX_STREAM_ENTRY, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_5 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0xe20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT COMMON_REPROCESSING_STACK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xe40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SPDSVC_GEN_STACK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xe80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_5 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, 0xf00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_5 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0xfa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SPU_REQUEST_STACK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xfc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER_5 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_5 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_5 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x2368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_5 =
{
	16,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, 0x2370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_5 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, 0x2380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FPI_MODE_TO_NATC_TABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x23fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TX_ABS_RECYCLE_COUNTERS_5 =
{
	8,
	{
		{ dump_RDD_TX_ABS_RECYCLE_COUNTERS_ENTRY, 0x2568 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2570 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT COMMON_REPROCESSING_PD_FIFO_TABLE_5 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x2580 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_5 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x25c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PKTGEN_CURR_SBPM_HDR_PTR_5 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x25fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_5 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, 0x2768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_5 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, 0x2770 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FPI_MODE_SEQ_TABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x277c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_PD_FIFO_TABLE_5 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x2780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_5 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, 0x27c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PKTGEN_NUM_OF_AVAIL_SBPM_HDRS_5 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x27fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_5 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, 0x2968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SPDSVC_GEN_DISPATCHER_CREDIT_TABLE_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x297c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SPU_REQUEST_PD_FIFO_TABLE_5 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x2980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SPDSVC_GEN_PARAMS_TABLE_5 =
{
	60,
	{
		{ dump_RDD_SPDSVC_GEN_PARAMS, 0x29c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PKTGEN_SBPM_END_PTR_5 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x29fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT AQM_NUM_QUEUES_5 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x29fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2b68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_5 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, 0x2be8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_DISPATCHER_CREDIT_TABLE_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2bf0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PKTGEN_BAD_GET_NEXT_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2bfc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_5 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x2d68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT COMMON_REPROCESSING_DISPATCHER_CREDIT_TABLE_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2d70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PKTGEN_MAX_UT_PKTS_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2d7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CRYPTO_SESSION_STATS_SCRATCH_5 =
{
	128,
	{
		{ dump_RDD_CRYPTO_SESSION_SEQ_INFO, 0x2d80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_5 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, 0x2f68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2f6f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_GEN_DISPATCHER_CREDIT_TABLE_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2f70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PKTGEN_UT_TRIGGER_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2f7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PKTGEN_SBPM_HDR_BNS_5 =
{
	2,
	{
		{ dump_RDD_PKTGEN_SBPM_HDR_BN, 0x2f80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TASK_IDX_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2fb8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FPI_CFG_TABLE_5 =
{
	2,
	{
		{ dump_RDD_FPI_CFG, 0x2fbc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_5 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2fbe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SPU_OFFLOAD_PARAMS_TABLE_5 =
{
	40,
	{
		{ dump_RDD_SPU_OFFLOAD_PARAMS, 0x2fc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_5 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2fe8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2fea },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2feb },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2fec },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SPDSVC_TCPSPDTEST_COMMON_TABLE_5 =
{
	1,
	{
		{ dump_RDD_SPDSVC_TCPSPDTEST_COMMON_ENTRY, 0x2fed },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_5 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x2fee },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT IPTV_CONFIGURATION_TABLE_5 =
{
	12,
	{
		{ dump_RDD_HW_IPTV_CONFIGURATION, 0x2ff0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_REDIRECT_MODE_5 =
{
	1,
	{
		{ dump_RDD_CPU_REDIRECT_MODE_ENTRY, 0x2ffc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2ffd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_5 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, 0x2ffe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2fff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT UDPSPDT_STREAM_TX_STAT_TABLE_5 =
{
	40,
	{
		{ dump_RDD_UDPSPDT_STREAM_TX_STAT, 0x3000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SPU_OFFLOAD_SCRATCHPAD_5 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x30a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x30e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_5 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x30ec },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x30ed },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x30ee },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_5 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x30f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_5 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, 0x3100 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT COMMON_REPROCESSING_UPDATE_FIFO_TABLE_5 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x3180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_ENGINE_GLOBAL_TABLE_5 =
{
	20,
	{
		{ dump_RDD_TCPSPDTEST_ENGINE_GLOBAL_INFO, 0x31a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_5 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x31b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_UPDATE_FIFO_TABLE_5 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x31c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT AQM_ENABLE_TABLE_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x31e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_5 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, 0x31f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT UDPSPDT_TX_PARAMS_TABLE_5 =
{
	24,
	{
		{ dump_RDD_UDPSPDT_TX_PARAMS, 0x3200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_5 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, 0x3260 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SPU_REQUEST_UPDATE_FIFO_TABLE_5 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x3280 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x32a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PKTGEN_SBPM_EXTS_5 =
{
	4,
	{
		{ dump_RDD_PKTGEN_SBPM_EXT, 0x32b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PKTGEN_SESSION_DATA_5 =
{
	32,
	{
		{ dump_RDD_PKTGEN_TX_PARAMS, 0x32c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PKTGEN_BBMSG_REPLY_SCRATCH_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x32e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x32e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_5 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, 0x32f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT NATC_TBL_CFG_5 =
{
	24,
	{
		{ dump_RDD_NATC_TBL_CONFIGURATION, 0x3300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_5 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, 0x3360 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_5 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0x3380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x33a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x33c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x33e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_5 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, 0x3420 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PKTGEN_FPM_UG_MGMT_5 =
{
	20,
	{
		{ dump_RDD_PKTGEN_FPM_UG_MGMT_ENTRY, 0x3440 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3480 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PKTGEN_TX_STREAM_SCRATCH_TABLE_5 =
{
	8,
	{
		{ dump_RDD_PKTGEN_TX_STREAM_SCRATCH_ENTRY, 0x3500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FC_FLOW_IP_ADDRESSES_TABLE_5 =
{
	48,
	{
		{ dump_RDD_FC_FLOW_IP_ADDRESSES_ENTRY, 0x3d00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_COMPLETE_COMMON_RADIO_DATA_6 =
{
	192,
	{
		{ dump_RDD_DHD_COMPLETE_COMMON_RADIO_ENTRY, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT VPORT_TX_FLOW_TABLE_6 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, 0x240 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_FPM_RINGS_AND_BUFMNG_REFILL_STACK_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x280 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FPM_RING_CFG_TABLE_6 =
{
	24,
	{
		{ dump_RDD_FPM_RING_CFG_ENTRY, 0x300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_0_STACK_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_6 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_RX_INTERRUPT_ID_DDR_ADDR_6 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0x768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_6 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, 0x770 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SPU_RESPONSE_STACK_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT NULL_BUFFER_6 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_6 =
{
	16,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, 0x970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_6 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, 0x980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FPI_MODE_TO_NATC_TABLE_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x9fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xa00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_6 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, 0xb68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xb70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_FLOW_RING_BUFFER_6 =
{
	32,
	{
		{ dump_RDD_DHD_RX_COMPLETE_DESCRIPTOR, 0xb80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0xbe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_6 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0xd68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_6 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, 0xd70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FPI_MODE_SEQ_TABLE_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xd7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_RX_POST_FLOW_RING_BUFFER_6 =
{
	32,
	{
		{ dump_RDD_DHD_RX_POST_DESCRIPTOR, 0xd80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_6 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0xde0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0xf68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_RX_POST_RING_SIZE_6 =
{
	2,
	{
		{ dump_RDD_DHD_RING_SIZE, 0xfe8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT AQM_NUM_QUEUES_6 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0xfee },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SPU_RESPONSE_DISPATCHER_CREDIT_TABLE_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0xff0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0xffc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER_6 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_RING_SIZE_6 =
{
	2,
	{
		{ dump_RDD_DHD_RING_SIZE, 0x2168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_6 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x216e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT IPTV_CONFIGURATION_TABLE_6 =
{
	12,
	{
		{ dump_RDD_HW_IPTV_CONFIGURATION, 0x2170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TASK_IDX_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x217c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_1_STACK_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_2_STACK_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x21c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_RING_SIZE_6 =
{
	2,
	{
		{ dump_RDD_DHD_RING_SIZE, 0x2368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_6 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x236e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_6 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x2370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FPI_CFG_TABLE_6 =
{
	2,
	{
		{ dump_RDD_FPI_CFG, 0x237c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x237e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x237f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_0_STACK_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_1_STACK_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x23c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_6 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, 0x2568 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2570 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x257c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x257d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_6 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x257e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_2_STACK_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2580 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_6 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x25c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CPU_REDIRECT_MODE_6 =
{
	1,
	{
		{ dump_RDD_CPU_REDIRECT_MODE_ENTRY, 0x25fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x25ff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_6 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_6 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, 0x26b4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x26b5 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_6 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x26b6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x26b7 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_6 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, 0x26b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_6 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, 0x26c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x26fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_6 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, 0x2700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_6 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0x27a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SPU_RESPONSE_PARAMS_TABLE_6 =
{
	56,
	{
		{ dump_RDD_SPU_RESPONSE_PARAMS, 0x27c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_6 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, 0x27f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_6 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT CRYPTO_SESSION_STATS_SCRATCH_6 =
{
	128,
	{
		{ dump_RDD_CRYPTO_SESSION_SEQ_INFO, 0x2880 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT NATC_TBL_CFG_6 =
{
	24,
	{
		{ dump_RDD_NATC_TBL_CONFIGURATION, 0x2900 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT AQM_ENABLE_TABLE_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2960 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_6 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x2978 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE_6 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x2980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_DOORBELL_TX_COMPLETE_VALUE_6 =
{
	4,
	{
		{ dump_RDD_DHD_DOORBELL, 0x29b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_FLOW_RING_BUFFER_6 =
{
	16,
	{
		{ dump_RDD_DHD_TX_COMPLETE_DESCRIPTOR, 0x29c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_DOORBELL_RX_COMPLETE_VALUE_6 =
{
	4,
	{
		{ dump_RDD_DHD_DOORBELL, 0x29f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_DISPATCHER_CREDIT_TABLE_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_DOORBELL_RX_POST_VALUE_6 =
{
	4,
	{
		{ dump_RDD_DHD_DOORBELL, 0x2a30 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_DISPATCHER_CREDIT_TABLE_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2a40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SPU_RESPONSE_SCRATCHPAD_6 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x2a70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_6 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, 0x2ab8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_6 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0x2ac0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_6 =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, 0x2ae0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2af8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2b20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_CPU_INT_ID_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2b50 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_6 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, 0x2b60 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DHD_FPM_REPLY_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2b80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_6 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2b98 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT SPDSVC_WLAN_GEN_PARAMS_TABLE_6 =
{
	10,
	{
		{ dump_RDD_SPDSVC_WLAN_GEN_PARAMS, 0x2ba0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_6 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, 0x2bb0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2bc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_6 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, 0x2bd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2be0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_6 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, 0x2be8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2bf0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_6 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, 0x2c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM4912
static DUMP_RUNNERREG_STRUCT FC_FLOW_IP_ADDRESSES_TABLE_6 =
{
	48,
	{
		{ dump_RDD_FC_FLOW_IP_ADDRESSES_ENTRY, 0x3d00 },
		{ 0, 0 },
	}
};
#endif

TABLE_STRUCT RUNNER_TABLES[NUMBER_OF_TABLES] =
{
#if defined BCM4912
	{ "DS_TM_PD_FIFO_TABLE", 1, CORE_0_INDEX, &DS_TM_PD_FIFO_TABLE, 128, 1, 1 },
#endif
#if defined BCM4912
	{ "SERVICE_QUEUES_PD_FIFO_TABLE", 1, CORE_0_INDEX, &SERVICE_QUEUES_PD_FIFO_TABLE, 64, 1, 1 },
#endif
#if defined BCM4912
	{ "DS_TM_SCHEDULING_QUEUE_TABLE", 1, CORE_0_INDEX, &DS_TM_SCHEDULING_QUEUE_TABLE, 64, 1, 1 },
#endif
#if defined BCM4912
	{ "BASIC_SCHEDULER_TABLE_DS", 1, CORE_0_INDEX, &BASIC_SCHEDULER_TABLE_DS, 32, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING_4_TASKS_PACKET_BUFFER", 1, CORE_0_INDEX, &PROCESSING_4_TASKS_PACKET_BUFFER, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "BASIC_RATE_LIMITER_TABLE_DS", 1, CORE_0_INDEX, &BASIC_RATE_LIMITER_TABLE_DS, 64, 1, 1 },
#endif
#if defined BCM4912
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_0_INDEX, &RUNNER_PROFILING_TRACE_BUFFER, 128, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING0_STACK", 1, CORE_0_INDEX, &PROCESSING0_STACK, 360, 1, 1 },
#endif
#if defined BCM4912
	{ "TM_BBH_TX_WAKE_UP_DATA_TABLE", 1, CORE_0_INDEX, &TM_BBH_TX_WAKE_UP_DATA_TABLE, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "SPDTEST_GEN_PARAM", 1, CORE_0_INDEX, &SPDTEST_GEN_PARAM, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "VPORT_CFG_TABLE", 1, CORE_0_INDEX, &VPORT_CFG_TABLE, 31, 1, 1 },
#endif
#if defined BCM4912
	{ "FPI_MODE_TO_NATC_TABLE", 1, CORE_0_INDEX, &FPI_MODE_TO_NATC_TABLE, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "SERVICE_QUEUES_RATE_LIMITER_TABLE", 1, CORE_0_INDEX, &SERVICE_QUEUES_RATE_LIMITER_TABLE, 66, 1, 1 },
#endif
#if defined BCM4912
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_0_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT, 8, 1, 1 },
#endif
#if defined BCM4912
	{ "DS_TM_TM_FLOW_CNTR_TABLE", 1, CORE_0_INDEX, &DS_TM_TM_FLOW_CNTR_TABLE, 64, 1, 1 },
#endif
#if defined BCM4912
	{ "DS_BUFFER_CONG_MGT_CFG", 1, CORE_0_INDEX, &DS_BUFFER_CONG_MGT_CFG, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "FPI_MODE_SEQ_TABLE", 1, CORE_0_INDEX, &FPI_MODE_SEQ_TABLE, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "TM_1_BBH_TX_WAKE_UP_DATA_TABLE", 1, CORE_0_INDEX, &TM_1_BBH_TX_WAKE_UP_DATA_TABLE, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "DS_TM_FLUSH_CFG_CPU_TABLE", 1, CORE_0_INDEX, &DS_TM_FLUSH_CFG_CPU_TABLE, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_0_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE, 32, 1, 1 },
#endif
#if defined BCM4912
	{ "DS_TM_AQM_QUEUE_TABLE", 1, CORE_0_INDEX, &DS_TM_AQM_QUEUE_TABLE, 64, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING1_STACK", 1, CORE_0_INDEX, &PROCESSING1_STACK, 360, 1, 1 },
#endif
#if defined BCM4912
	{ "NULL_BUFFER", 1, CORE_0_INDEX, &NULL_BUFFER, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "DS_TM_FLUSH_CFG_FW_TABLE", 1, CORE_0_INDEX, &DS_TM_FLUSH_CFG_FW_TABLE, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "SPDSVC_ANALYZER_PD_FIFO_TABLE", 1, CORE_0_INDEX, &SPDSVC_ANALYZER_PD_FIFO_TABLE, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "VPORT_TX_FLOW_TABLE", 1, CORE_0_INDEX, &VPORT_TX_FLOW_TABLE, 64, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING2_STACK", 1, CORE_0_INDEX, &PROCESSING2_STACK, 360, 1, 1 },
#endif
#if defined BCM4912
	{ "REGISTERS_BUFFER", 1, CORE_0_INDEX, &REGISTERS_BUFFER, 32, 1, 1 },
#endif
#if defined BCM4912
	{ "SYSTEM_CONFIGURATION", 1, CORE_0_INDEX, &SYSTEM_CONFIGURATION, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "SERVICE_QUEUES_FLUSH_CFG_FW_TABLE", 1, CORE_0_INDEX, &SERVICE_QUEUES_FLUSH_CFG_FW_TABLE, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING3_STACK", 1, CORE_0_INDEX, &PROCESSING3_STACK, 360, 1, 1 },
#endif
#if defined BCM4912
	{ "DS_TM_TX_QUEUE_DROP_TABLE", 1, CORE_0_INDEX, &DS_TM_TX_QUEUE_DROP_TABLE, 65, 1, 1 },
#endif
#if defined BCM4912
	{ "DS_TM_FLUSH_CFG_CURRENT_TABLE", 1, CORE_0_INDEX, &DS_TM_FLUSH_CFG_CURRENT_TABLE, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "AQM_SQ_TABLE", 1, CORE_0_INDEX, &AQM_SQ_TABLE, 32, 1, 1 },
#endif
#if defined BCM4912
	{ "UPDATE_FIFO_STACK", 1, CORE_0_INDEX, &UPDATE_FIFO_STACK, 64, 1, 1 },
#endif
#if defined BCM4912
	{ "SPDSVC_ANALYZER_STACK", 1, CORE_0_INDEX, &SPDSVC_ANALYZER_STACK, 256, 1, 1 },
#endif
#if defined BCM4912
	{ "SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE", 1, CORE_0_INDEX, &SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE, 32, 1, 1 },
#endif
#if defined BCM4912
	{ "COMPLEX_SCHEDULER_TABLE_DS", 1, CORE_0_INDEX, &COMPLEX_SCHEDULER_TABLE_DS, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "RX_FLOW_TABLE", 1, CORE_0_INDEX, &RX_FLOW_TABLE, 90, 1, 1 },
#endif
#if defined BCM4912
	{ "MUTEX_TABLE", 1, CORE_0_INDEX, &MUTEX_TABLE, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "SERVICE_QUEUES_SCRATCHPAD", 1, CORE_0_INDEX, &SERVICE_QUEUES_SCRATCHPAD, 2, 1, 1 },
#endif
#if defined BCM4912
	{ "SERVICE_QUEUES_COMPLEX_SCHEDULER_TABLE", 1, CORE_0_INDEX, &SERVICE_QUEUES_COMPLEX_SCHEDULER_TABLE, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "UDPSPDT_STREAM_RX_STAT_TABLE", 1, CORE_0_INDEX, &UDPSPDT_STREAM_RX_STAT_TABLE, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "BUFFER_CONG_SCRATCHPAD", 1, CORE_0_INDEX, &BUFFER_CONG_SCRATCHPAD, 8, 1, 1 },
#endif
#if defined BCM4912
	{ "POLICER_PARAMS_TABLE", 1, CORE_0_INDEX, &POLICER_PARAMS_TABLE, 80, 1, 1 },
#endif
#if defined BCM4912
	{ "SQ_TX_QUEUE_DROP_TABLE", 1, CORE_0_INDEX, &SQ_TX_QUEUE_DROP_TABLE, 32, 1, 1 },
#endif
#if defined BCM4912
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_0_INDEX, &BUFMNG_DESCRIPTOR_TABLE, 16, 1, 1 },
#endif
#if defined BCM4912
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_0_INDEX, &LOOPBACK_QUEUE_TABLE, 31, 1, 1 },
#endif
#if defined BCM4912
	{ "AQM_NUM_QUEUES", 1, CORE_0_INDEX, &AQM_NUM_QUEUES, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_0_INDEX, &INGRESS_FILTER_PROFILE_TABLE, 16, 1, 1 },
#endif
#if defined BCM4912
	{ "SERVICE_QUEUES_AQM_QUEUE_TABLE", 1, CORE_0_INDEX, &SERVICE_QUEUES_AQM_QUEUE_TABLE, 32, 1, 1 },
#endif
#if defined BCM4912
	{ "NATC_TBL_CFG", 1, CORE_0_INDEX, &NATC_TBL_CFG, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "BUFFER_CONG_DQM_NOT_EMPTY", 1, CORE_0_INDEX, &BUFFER_CONG_DQM_NOT_EMPTY, 8, 1, 1 },
#endif
#if defined BCM4912
	{ "VPORT_CFG_EX_TABLE", 1, CORE_0_INDEX, &VPORT_CFG_EX_TABLE, 31, 1, 1 },
#endif
#if defined BCM4912
	{ "BUDGET_ALLOCATION_TIMER_VALUE", 1, CORE_0_INDEX, &BUDGET_ALLOCATION_TIMER_VALUE, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "DS_TM_PI2_PROBABILITY_CALC_DESCRIPTOR", 1, CORE_0_INDEX, &DS_TM_PI2_PROBABILITY_CALC_DESCRIPTOR, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "TASK_IDX", 1, CORE_0_INDEX, &TASK_IDX, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "MIRRORING_SCRATCH", 1, CORE_0_INDEX, &MIRRORING_SCRATCH, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "CODEL_BIAS_SLOPE_TABLE", 1, CORE_0_INDEX, &CODEL_BIAS_SLOPE_TABLE, 11, 1, 1 },
#endif
#if defined BCM4912
	{ "FPI_CFG_TABLE", 1, CORE_0_INDEX, &FPI_CFG_TABLE, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "DOS_DROP_REASONS_CFG", 1, CORE_0_INDEX, &DOS_DROP_REASONS_CFG, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "SERVICE_QUEUES_FLUSH_CFG_CURRENT_TABLE", 1, CORE_0_INDEX, &SERVICE_QUEUES_FLUSH_CFG_CURRENT_TABLE, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "UPDATE_FIFO_TABLE", 1, CORE_0_INDEX, &UPDATE_FIFO_TABLE, 8, 1, 1 },
#endif
#if defined BCM4912
	{ "DS_TM_SCHEDULING_QUEUE_AGING_VECTOR", 1, CORE_0_INDEX, &DS_TM_SCHEDULING_QUEUE_AGING_VECTOR, 5, 1, 1 },
#endif
#if defined BCM4912
	{ "DS_TM_FLUSH_CFG_ENABLE_TABLE", 1, CORE_0_INDEX, &DS_TM_FLUSH_CFG_ENABLE_TABLE, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "SERVICE_QUEUES_FLUSH_CFG_ENABLE_TABLE", 1, CORE_0_INDEX, &SERVICE_QUEUES_FLUSH_CFG_ENABLE_TABLE, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "TCAM_GENERIC_FIELDS", 1, CORE_0_INDEX, &TCAM_GENERIC_FIELDS, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "SPDSVC_ANALYZER_UPDATE_FIFO_TABLE", 1, CORE_0_INDEX, &SPDSVC_ANALYZER_UPDATE_FIFO_TABLE, 8, 1, 1 },
#endif
#if defined BCM4912
	{ "SERVICE_QUEUES_SCHEDULING_QUEUE_AGING_VECTOR", 1, CORE_0_INDEX, &SERVICE_QUEUES_SCHEDULING_QUEUE_AGING_VECTOR, 5, 1, 1 },
#endif
#if defined BCM4912
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_0_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "SERVICE_QUEUES_BUDGET_ALLOCATION_TIMER_VALUE", 1, CORE_0_INDEX, &SERVICE_QUEUES_BUDGET_ALLOCATION_TIMER_VALUE, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "TUNNELS_PARSING_CFG", 1, CORE_0_INDEX, &TUNNELS_PARSING_CFG, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "SERVICE_QUEUES_UPDATE_FIFO_TABLE", 1, CORE_0_INDEX, &SERVICE_QUEUES_UPDATE_FIFO_TABLE, 8, 1, 1 },
#endif
#if defined BCM4912
	{ "DS_TM_SCHEDULING_AGGREGATION_CONTEXT_VECTOR", 1, CORE_0_INDEX, &DS_TM_SCHEDULING_AGGREGATION_CONTEXT_VECTOR, 5, 1, 1 },
#endif
#if defined BCM4912
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_0_INDEX, &SPDTEST_NUM_OF_RX_FLOWS, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "SCHEDULING_FLUSH_GLOBAL_CFG", 1, CORE_0_INDEX, &SCHEDULING_FLUSH_GLOBAL_CFG, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_0_INDEX, &LOOPBACK_WAN_FLOW_TABLE, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "FORCE_DSCP", 1, CORE_0_INDEX, &FORCE_DSCP, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "IPTV_CFG_TABLE", 1, CORE_0_INDEX, &IPTV_CFG_TABLE, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "DS_TM_BBH_QUEUE_TABLE", 1, CORE_0_INDEX, &DS_TM_BBH_QUEUE_TABLE, 12, 1, 1 },
#endif
#if defined BCM4912
	{ "BUFMNG_STATUS_TABLE", 1, CORE_0_INDEX, &BUFMNG_STATUS_TABLE, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_0_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE, 31, 1, 1 },
#endif
#if defined BCM4912
	{ "CORE_ID_TABLE", 1, CORE_0_INDEX, &CORE_ID_TABLE, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "AQM_ENABLE_TABLE", 1, CORE_0_INDEX, &AQM_ENABLE_TABLE, 5, 1, 1 },
#endif
#if defined BCM4912
	{ "DS_TM_FIRST_QUEUE_MAPPING", 1, CORE_0_INDEX, &DS_TM_FIRST_QUEUE_MAPPING, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "SERVICE_QUEUES_FIRST_QUEUE_MAPPING", 1, CORE_0_INDEX, &SERVICE_QUEUES_FIRST_QUEUE_MAPPING, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_0_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_0_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE, 31, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_REDIRECT_MODE", 1, CORE_0_INDEX, &CPU_REDIRECT_MODE, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "DEBUG_SCRATCHPAD", 1, CORE_0_INDEX, &DEBUG_SCRATCHPAD, 12, 1, 1 },
#endif
#if defined BCM4912
	{ "SQ_TM_BUFMNG_STATUS_TABLE", 1, CORE_0_INDEX, &SQ_TM_BUFMNG_STATUS_TABLE, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "DS_TM_CODEL_DROP_DESCRIPTOR", 1, CORE_0_INDEX, &DS_TM_CODEL_DROP_DESCRIPTOR, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_0_INDEX, &RX_MIRRORING_CONFIGURATION, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_0_INDEX, &TX_MIRRORING_CONFIGURATION, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "BASIC_SCHEDULER_WAKEUP_CMD_DS", 1, CORE_0_INDEX, &BASIC_SCHEDULER_WAKEUP_CMD_DS, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_0_INDEX, &DEBUG_PRINT_CORE_LOCK, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_0_INDEX, &TCAM_TABLE_CFG_TABLE, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "SRAM_DUMMY_STORE", 1, CORE_0_INDEX, &SRAM_DUMMY_STORE, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_0_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "SERVICE_QUEUES_CODEL_DROP_DESCRIPTOR", 1, CORE_0_INDEX, &SERVICE_QUEUES_CODEL_DROP_DESCRIPTOR, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_0_INDEX, &INGRESS_FILTER_1588_CFG, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_0_INDEX, &RX_MIRRORING_DIRECT_ENABLE, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "COMPLEX_SCHEDULER_WAKEUP_CMD_DS", 1, CORE_0_INDEX, &COMPLEX_SCHEDULER_WAKEUP_CMD_DS, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "SERVICE_QUEUES_RATE_LIMITER_VALID_TABLE", 1, CORE_0_INDEX, &SERVICE_QUEUES_RATE_LIMITER_VALID_TABLE, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "SPDSVC_ANALYZER_PARAMS_TABLE", 1, CORE_0_INDEX, &SPDSVC_ANALYZER_PARAMS_TABLE, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_0_INDEX, &BITS_CALC_MASKS_TABLE, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_0_INDEX, &INGRESS_FILTER_L2_REASON_TABLE, 16, 1, 1 },
#endif
#if defined BCM4912
	{ "RATE_LIMITER_VALID_TABLE_DS", 1, CORE_0_INDEX, &RATE_LIMITER_VALID_TABLE_DS, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "TR471_SPDSVC_RX_PKT_ID", 1, CORE_0_INDEX, &TR471_SPDSVC_RX_PKT_ID, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "FPM_GLOBAL_CFG", 1, CORE_0_INDEX, &FPM_GLOBAL_CFG, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "DS_TM_FLUSH_DISPATCHER_CREDIT_TABLE", 1, CORE_0_INDEX, &DS_TM_FLUSH_DISPATCHER_CREDIT_TABLE, 3, 1, 1 },
#endif
#if defined BCM4912
	{ "SERVICE_QUEUES_AQM_TIMER_DISPATCHER_CREDIT_TABLE", 1, CORE_0_INDEX, &SERVICE_QUEUES_AQM_TIMER_DISPATCHER_CREDIT_TABLE, 3, 1, 1 },
#endif
#if defined BCM4912
	{ "IPTV_CONFIGURATION_TABLE", 1, CORE_0_INDEX, &IPTV_CONFIGURATION_TABLE, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_0_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_FPM_THRESHOLDS", 1, CORE_0_INDEX, &DHD_FPM_THRESHOLDS, 3, 1, 1 },
#endif
#if defined BCM4912
	{ "SERVICE_QUEUES_DISPATCHER_CREDIT_TABLE", 1, CORE_0_INDEX, &SERVICE_QUEUES_DISPATCHER_CREDIT_TABLE, 3, 1, 1 },
#endif
#if defined BCM4912
	{ "SPDSVC_BBH_TX_PARAMS_TABLE", 1, CORE_0_INDEX, &SPDSVC_BBH_TX_PARAMS_TABLE, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "TX_MIRRORING_DISPATCHER_CREDIT_TABLE", 1, CORE_0_INDEX, &TX_MIRRORING_DISPATCHER_CREDIT_TABLE, 3, 1, 1 },
#endif
#if defined BCM4912
	{ "UDPSPDT_SCRATCH_TABLE", 1, CORE_0_INDEX, &UDPSPDT_SCRATCH_TABLE, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "NAT_CACHE_CFG", 1, CORE_0_INDEX, &NAT_CACHE_CFG, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "DEBUG_PRINT_TABLE", 1, CORE_0_INDEX, &DEBUG_PRINT_TABLE, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "AQM_TIMER_DISPATCHER_REPLY", 1, CORE_0_INDEX, &AQM_TIMER_DISPATCHER_REPLY, 2, 1, 1 },
#endif
#if defined BCM4912
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_0_INDEX, &NAT_CACHE_KEY0_MASK, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "GDX_PARAMS_TABLE", 1, CORE_0_INDEX, &GDX_PARAMS_TABLE, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_0_INDEX, &FW_ERROR_VECTOR_TABLE, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_0_INDEX, &NATC_L2_VLAN_KEY_MASK, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "INGRESS_FILTER_CFG", 1, CORE_0_INDEX, &INGRESS_FILTER_CFG, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "NATC_L2_TOS_MASK", 1, CORE_0_INDEX, &NATC_L2_TOS_MASK, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "TM_BBH_TX_EGRESS_COUNTER_TABLE", 1, CORE_0_INDEX, &TM_BBH_TX_EGRESS_COUNTER_TABLE, 6, 1, 1 },
#endif
#if defined BCM4912
	{ "MULTICAST_KEY_MASK", 1, CORE_0_INDEX, &MULTICAST_KEY_MASK, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "TM_BBH_TX_ABS_COUNTER_TABLE", 1, CORE_0_INDEX, &TM_BBH_TX_ABS_COUNTER_TABLE, 6, 1, 1 },
#endif
#if defined BCM4912
	{ "TM_1_BBH_TX_EGRESS_COUNTER_TABLE", 1, CORE_0_INDEX, &TM_1_BBH_TX_EGRESS_COUNTER_TABLE, 5, 1, 1 },
#endif
#if defined BCM4912
	{ "TM_1_BBH_TX_ABS_COUNTER_TABLE", 1, CORE_0_INDEX, &TM_1_BBH_TX_ABS_COUNTER_TABLE, 5, 1, 1 },
#endif
#if defined BCM4912
	{ "FC_FLOW_IP_ADDRESSES_TABLE", 1, CORE_0_INDEX, &FC_FLOW_IP_ADDRESSES_TABLE, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "US_TM_PD_FIFO_TABLE", 1, CORE_1_INDEX, &US_TM_PD_FIFO_TABLE_1, 128, 1, 1 },
#endif
#if defined BCM4912
	{ "BASIC_RATE_LIMITER_TABLE_US", 1, CORE_1_INDEX, &BASIC_RATE_LIMITER_TABLE_US_1, 64, 1, 1 },
#endif
#if defined BCM4912
	{ "US_TM_SCHEDULING_QUEUE_TABLE", 1, CORE_1_INDEX, &US_TM_SCHEDULING_QUEUE_TABLE_1, 64, 1, 1 },
#endif
#if defined BCM4912
	{ "BASIC_SCHEDULER_TABLE_US", 1, CORE_1_INDEX, &BASIC_SCHEDULER_TABLE_US_1, 32, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING_4_TASKS_PACKET_BUFFER", 1, CORE_1_INDEX, &PROCESSING_4_TASKS_PACKET_BUFFER_1, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_1_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_1, 128, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING0_STACK", 1, CORE_1_INDEX, &PROCESSING0_STACK_1, 360, 1, 1 },
#endif
#if defined BCM4912
	{ "TM_BBH_TX_WAKE_UP_DATA_TABLE", 1, CORE_1_INDEX, &TM_BBH_TX_WAKE_UP_DATA_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "SPDTEST_GEN_PARAM", 1, CORE_1_INDEX, &SPDTEST_GEN_PARAM_1, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "VPORT_CFG_TABLE", 1, CORE_1_INDEX, &VPORT_CFG_TABLE_1, 31, 1, 1 },
#endif
#if defined BCM4912
	{ "FPI_MODE_TO_NATC_TABLE", 1, CORE_1_INDEX, &FPI_MODE_TO_NATC_TABLE_1, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING1_STACK", 1, CORE_1_INDEX, &PROCESSING1_STACK_1, 360, 1, 1 },
#endif
#if defined BCM4912
	{ "TM_1_BBH_TX_WAKE_UP_DATA_TABLE", 1, CORE_1_INDEX, &TM_1_BBH_TX_WAKE_UP_DATA_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "US_TM_FLUSH_CFG_CPU_TABLE", 1, CORE_1_INDEX, &US_TM_FLUSH_CFG_CPU_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "US_BUFFER_CONG_MGT_CFG", 1, CORE_1_INDEX, &US_BUFFER_CONG_MGT_CFG_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "FPI_MODE_SEQ_TABLE", 1, CORE_1_INDEX, &FPI_MODE_SEQ_TABLE_1, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "NULL_BUFFER", 1, CORE_1_INDEX, &NULL_BUFFER_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "US_TM_FLUSH_CFG_FW_TABLE", 1, CORE_1_INDEX, &US_TM_FLUSH_CFG_FW_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_1_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_1, 8, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING2_STACK", 1, CORE_1_INDEX, &PROCESSING2_STACK_1, 360, 1, 1 },
#endif
#if defined BCM4912
	{ "SYSTEM_CONFIGURATION", 1, CORE_1_INDEX, &SYSTEM_CONFIGURATION_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "US_TM_FLUSH_CFG_CURRENT_TABLE", 1, CORE_1_INDEX, &US_TM_FLUSH_CFG_CURRENT_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "US_TM_TM_FLOW_CNTR_TABLE", 1, CORE_1_INDEX, &US_TM_TM_FLOW_CNTR_TABLE_1, 64, 1, 1 },
#endif
#if defined BCM4912
	{ "VPORT_TX_FLOW_TABLE", 1, CORE_1_INDEX, &VPORT_TX_FLOW_TABLE_1, 64, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING3_STACK", 1, CORE_1_INDEX, &PROCESSING3_STACK_1, 360, 1, 1 },
#endif
#if defined BCM4912
	{ "US_TM_TX_QUEUE_DROP_TABLE", 1, CORE_1_INDEX, &US_TM_TX_QUEUE_DROP_TABLE_1, 65, 1, 1 },
#endif
#if defined BCM4912
	{ "REGISTERS_BUFFER", 1, CORE_1_INDEX, &REGISTERS_BUFFER_1, 32, 1, 1 },
#endif
#if defined BCM4912
	{ "BUFMNG_STATUS_TABLE", 1, CORE_1_INDEX, &BUFMNG_STATUS_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "US_TM_AQM_QUEUE_TABLE", 1, CORE_1_INDEX, &US_TM_AQM_QUEUE_TABLE_1, 64, 1, 1 },
#endif
#if defined BCM4912
	{ "COMPLEX_SCHEDULER_TABLE_US", 1, CORE_1_INDEX, &COMPLEX_SCHEDULER_TABLE_US_1, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "RX_FLOW_TABLE", 1, CORE_1_INDEX, &RX_FLOW_TABLE_1, 90, 1, 1 },
#endif
#if defined BCM4912
	{ "MUTEX_TABLE", 1, CORE_1_INDEX, &MUTEX_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "MIRRORING_SCRATCH", 1, CORE_1_INDEX, &MIRRORING_SCRATCH_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "UPDATE_FIFO_STACK", 1, CORE_1_INDEX, &UPDATE_FIFO_STACK_1, 64, 1, 1 },
#endif
#if defined BCM4912
	{ "POLICER_PARAMS_TABLE", 1, CORE_1_INDEX, &POLICER_PARAMS_TABLE_1, 80, 1, 1 },
#endif
#if defined BCM4912
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_1_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_1, 32, 1, 1 },
#endif
#if defined BCM4912
	{ "BUFFER_CONG_SCRATCHPAD", 1, CORE_1_INDEX, &BUFFER_CONG_SCRATCHPAD_1, 8, 1, 1 },
#endif
#if defined BCM4912
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_1_INDEX, &INGRESS_FILTER_PROFILE_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM4912
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_1_INDEX, &LOOPBACK_QUEUE_TABLE_1, 31, 1, 1 },
#endif
#if defined BCM4912
	{ "AQM_NUM_QUEUES", 1, CORE_1_INDEX, &AQM_NUM_QUEUES_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "VPORT_CFG_EX_TABLE", 1, CORE_1_INDEX, &VPORT_CFG_EX_TABLE_1, 31, 1, 1 },
#endif
#if defined BCM4912
	{ "BUDGET_ALLOCATION_TIMER_VALUE", 1, CORE_1_INDEX, &BUDGET_ALLOCATION_TIMER_VALUE_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "NATC_TBL_CFG", 1, CORE_1_INDEX, &NATC_TBL_CFG_1, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_1_INDEX, &BUFMNG_DESCRIPTOR_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM4912
	{ "US_TM_PI2_PROBABILITY_CALC_DESCRIPTOR", 1, CORE_1_INDEX, &US_TM_PI2_PROBABILITY_CALC_DESCRIPTOR_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "TASK_IDX", 1, CORE_1_INDEX, &TASK_IDX_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "TCAM_GENERIC_FIELDS", 1, CORE_1_INDEX, &TCAM_GENERIC_FIELDS_1, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "CODEL_BIAS_SLOPE_TABLE", 1, CORE_1_INDEX, &CODEL_BIAS_SLOPE_TABLE_1, 11, 1, 1 },
#endif
#if defined BCM4912
	{ "FPI_CFG_TABLE", 1, CORE_1_INDEX, &FPI_CFG_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "DOS_DROP_REASONS_CFG", 1, CORE_1_INDEX, &DOS_DROP_REASONS_CFG_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "DIRECT_FLOW_PD_TABLE", 1, CORE_1_INDEX, &DIRECT_FLOW_PD_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "US_TM_BBH_QUEUE_TABLE", 1, CORE_1_INDEX, &US_TM_BBH_QUEUE_TABLE_1, 12, 1, 1 },
#endif
#if defined BCM4912
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_1_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM4912
	{ "UPDATE_FIFO_TABLE", 1, CORE_1_INDEX, &UPDATE_FIFO_TABLE_1, 8, 1, 1 },
#endif
#if defined BCM4912
	{ "BUFFER_CONG_DQM_NOT_EMPTY", 1, CORE_1_INDEX, &BUFFER_CONG_DQM_NOT_EMPTY_1, 8, 1, 1 },
#endif
#if defined BCM4912
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_1_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_1, 31, 1, 1 },
#endif
#if defined BCM4912
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_1_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "US_TM_SCHEDULING_QUEUE_AGING_VECTOR", 1, CORE_1_INDEX, &US_TM_SCHEDULING_QUEUE_AGING_VECTOR_1, 5, 1, 1 },
#endif
#if defined BCM4912
	{ "US_TM_FLUSH_CFG_ENABLE_TABLE", 1, CORE_1_INDEX, &US_TM_FLUSH_CFG_ENABLE_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_1_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "TUNNELS_PARSING_CFG", 1, CORE_1_INDEX, &TUNNELS_PARSING_CFG_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_1_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_1, 31, 1, 1 },
#endif
#if defined BCM4912
	{ "DIRECT_FLOW_WAN_VIQ_EXCLUSIVE", 1, CORE_1_INDEX, &DIRECT_FLOW_WAN_VIQ_EXCLUSIVE_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "DEBUG_SCRATCHPAD", 1, CORE_1_INDEX, &DEBUG_SCRATCHPAD_1, 12, 1, 1 },
#endif
#if defined BCM4912
	{ "OVERALL_RATE_LIMITER_TABLE", 1, CORE_1_INDEX, &OVERALL_RATE_LIMITER_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "US_TM_SCHEDULING_AGGREGATION_CONTEXT_VECTOR", 1, CORE_1_INDEX, &US_TM_SCHEDULING_AGGREGATION_CONTEXT_VECTOR_1, 5, 1, 1 },
#endif
#if defined BCM4912
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_1_INDEX, &LOOPBACK_WAN_FLOW_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "FORCE_DSCP", 1, CORE_1_INDEX, &FORCE_DSCP_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "US_TM_FIRST_QUEUE_MAPPING", 1, CORE_1_INDEX, &US_TM_FIRST_QUEUE_MAPPING_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "IPTV_CFG_TABLE", 1, CORE_1_INDEX, &IPTV_CFG_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "AQM_ENABLE_TABLE", 1, CORE_1_INDEX, &AQM_ENABLE_TABLE_1, 5, 1, 1 },
#endif
#if defined BCM4912
	{ "CORE_ID_TABLE", 1, CORE_1_INDEX, &CORE_ID_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "US_TM_ETH_BBH_TX_FIFO_SIZE", 1, CORE_1_INDEX, &US_TM_ETH_BBH_TX_FIFO_SIZE_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_1_INDEX, &RX_MIRRORING_CONFIGURATION_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_1_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "US_TM_CODEL_DROP_DESCRIPTOR", 1, CORE_1_INDEX, &US_TM_CODEL_DROP_DESCRIPTOR_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_1_INDEX, &TX_MIRRORING_CONFIGURATION_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "US_TM_ETH_1_BBH_TX_FIFO_SIZE", 1, CORE_1_INDEX, &US_TM_ETH_1_BBH_TX_FIFO_SIZE_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_REDIRECT_MODE", 1, CORE_1_INDEX, &CPU_REDIRECT_MODE_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "BASIC_SCHEDULER_WAKEUP_CMD_US", 1, CORE_1_INDEX, &BASIC_SCHEDULER_WAKEUP_CMD_US_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_1_INDEX, &DEBUG_PRINT_CORE_LOCK_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_1_INDEX, &TCAM_TABLE_CFG_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "SRAM_DUMMY_STORE", 1, CORE_1_INDEX, &SRAM_DUMMY_STORE_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "MAC_TYPE", 1, CORE_1_INDEX, &MAC_TYPE_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_1_INDEX, &BITS_CALC_MASKS_TABLE_1, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "TR471_SPDSVC_RX_PKT_ID", 1, CORE_1_INDEX, &TR471_SPDSVC_RX_PKT_ID_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_1_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_1_INDEX, &INGRESS_FILTER_1588_CFG_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_1_INDEX, &RX_MIRRORING_DIRECT_ENABLE_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "RATE_LIMITER_VALID_TABLE_US", 1, CORE_1_INDEX, &RATE_LIMITER_VALID_TABLE_US_1, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "US_TM_FLUSH_DISPATCHER_CREDIT_TABLE", 1, CORE_1_INDEX, &US_TM_FLUSH_DISPATCHER_CREDIT_TABLE_1, 3, 1, 1 },
#endif
#if defined BCM4912
	{ "FPM_GLOBAL_CFG", 1, CORE_1_INDEX, &FPM_GLOBAL_CFG_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "IPTV_CONFIGURATION_TABLE", 1, CORE_1_INDEX, &IPTV_CONFIGURATION_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_1_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_FPM_THRESHOLDS", 1, CORE_1_INDEX, &DHD_FPM_THRESHOLDS_1, 3, 1, 1 },
#endif
#if defined BCM4912
	{ "SPDSVC_BBH_TX_PARAMS_TABLE", 1, CORE_1_INDEX, &SPDSVC_BBH_TX_PARAMS_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "TX_MIRRORING_DISPATCHER_CREDIT_TABLE", 1, CORE_1_INDEX, &TX_MIRRORING_DISPATCHER_CREDIT_TABLE_1, 3, 1, 1 },
#endif
#if defined BCM4912
	{ "DEBUG_PRINT_TABLE", 1, CORE_1_INDEX, &DEBUG_PRINT_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "TM_BBH_TX_EGRESS_COUNTER_TABLE", 1, CORE_1_INDEX, &TM_BBH_TX_EGRESS_COUNTER_TABLE_1, 6, 1, 1 },
#endif
#if defined BCM4912
	{ "COMPLEX_SCHEDULER_WAKEUP_CMD_US", 1, CORE_1_INDEX, &COMPLEX_SCHEDULER_WAKEUP_CMD_US_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "GDX_PARAMS_TABLE", 1, CORE_1_INDEX, &GDX_PARAMS_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_1_INDEX, &FW_ERROR_VECTOR_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "NAT_CACHE_CFG", 1, CORE_1_INDEX, &NAT_CACHE_CFG_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_1_INDEX, &NAT_CACHE_KEY0_MASK_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_1_INDEX, &NATC_L2_VLAN_KEY_MASK_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "TM_BBH_TX_ABS_COUNTER_TABLE", 1, CORE_1_INDEX, &TM_BBH_TX_ABS_COUNTER_TABLE_1, 6, 1, 1 },
#endif
#if defined BCM4912
	{ "INGRESS_FILTER_CFG", 1, CORE_1_INDEX, &INGRESS_FILTER_CFG_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "NATC_L2_TOS_MASK", 1, CORE_1_INDEX, &NATC_L2_TOS_MASK_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "MULTICAST_KEY_MASK", 1, CORE_1_INDEX, &MULTICAST_KEY_MASK_1, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "TM_1_BBH_TX_EGRESS_COUNTER_TABLE", 1, CORE_1_INDEX, &TM_1_BBH_TX_EGRESS_COUNTER_TABLE_1, 5, 1, 1 },
#endif
#if defined BCM4912
	{ "TM_1_BBH_TX_ABS_COUNTER_TABLE", 1, CORE_1_INDEX, &TM_1_BBH_TX_ABS_COUNTER_TABLE_1, 5, 1, 1 },
#endif
#if defined BCM4912
	{ "FC_FLOW_IP_ADDRESSES_TABLE", 1, CORE_1_INDEX, &FC_FLOW_IP_ADDRESSES_TABLE_1, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_TX_SCRATCHPAD", 1, CORE_2_INDEX, &CPU_TX_SCRATCHPAD_2, 416, 1, 1 },
#endif
#if defined BCM4912
	{ "QM_QUEUE_TO_TX_FLOW_TABLE_PTR_TABLE", 1, CORE_2_INDEX, &QM_QUEUE_TO_TX_FLOW_TABLE_PTR_TABLE_2, 128, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING0_STACK", 1, CORE_2_INDEX, &PROCESSING0_STACK_2, 360, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR", 1, CORE_2_INDEX, &CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_2, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_RECYCLE_INTERRUPT_COALESCING_TABLE", 1, CORE_2_INDEX, &CPU_RECYCLE_INTERRUPT_COALESCING_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_TX_DBG_CNTRS_TABLE", 1, CORE_2_INDEX, &CPU_TX_DBG_CNTRS_TABLE_2, 2, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_2_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER_2, 8, 1, 1 },
#endif
#if defined BCM4912
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_2_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_2, 128, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING1_STACK", 1, CORE_2_INDEX, &PROCESSING1_STACK_2, 360, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_RX_INTERRUPT_ID_DDR_ADDR", 1, CORE_2_INDEX, &CPU_RX_INTERRUPT_ID_DDR_ADDR_2, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "SPDTEST_GEN_PARAM", 1, CORE_2_INDEX, &SPDTEST_GEN_PARAM_2, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "GENERAL_TIMER", 1, CORE_2_INDEX, &GENERAL_TIMER_2, 16, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING2_STACK", 1, CORE_2_INDEX, &PROCESSING2_STACK_2, 360, 1, 1 },
#endif
#if defined BCM4912
	{ "NULL_BUFFER", 1, CORE_2_INDEX, &NULL_BUFFER_2, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "BUFFER_ALLOC_REPLY", 1, CORE_2_INDEX, &BUFFER_ALLOC_REPLY_2, 2, 1, 1 },
#endif
#if defined BCM4912
	{ "VPORT_CFG_TABLE", 1, CORE_2_INDEX, &VPORT_CFG_TABLE_2, 31, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_INT_INTERRUPT_SCRATCH", 1, CORE_2_INDEX, &CPU_INT_INTERRUPT_SCRATCH_2, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING3_STACK", 1, CORE_2_INDEX, &PROCESSING3_STACK_2, 360, 1, 1 },
#endif
#if defined BCM4912
	{ "SYSTEM_CONFIGURATION", 1, CORE_2_INDEX, &SYSTEM_CONFIGURATION_2, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "BUFMNG_STATUS_TABLE", 1, CORE_2_INDEX, &BUFMNG_STATUS_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "SG_DESC_TABLE", 1, CORE_2_INDEX, &SG_DESC_TABLE_2, 2, 1, 1 },
#endif
#if defined BCM4912
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_2_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_2, 8, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING4_STACK", 1, CORE_2_INDEX, &PROCESSING4_STACK_2, 360, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE", 1, CORE_2_INDEX, &CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_2_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_2, 16, 1, 1 },
#endif
#if defined BCM4912
	{ "VPORT_TX_FLOW_TABLE", 1, CORE_2_INDEX, &VPORT_TX_FLOW_TABLE_2, 64, 1, 1 },
#endif
#if defined BCM4912
	{ "GENERAL_TIMER_STACK", 1, CORE_2_INDEX, &GENERAL_TIMER_STACK_2, 64, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING5_STACK", 1, CORE_2_INDEX, &PROCESSING5_STACK_2, 360, 1, 1 },
#endif
#if defined BCM4912
	{ "REGISTERS_BUFFER", 1, CORE_2_INDEX, &REGISTERS_BUFFER_2, 32, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_TX_RING_INDICES_VALUES_TABLE", 1, CORE_2_INDEX, &CPU_TX_RING_INDICES_VALUES_TABLE_2, 2, 1, 1 },
#endif
#if defined BCM4912
	{ "TR471_SPDSVC_RX_PKT_ID", 1, CORE_2_INDEX, &TR471_SPDSVC_RX_PKT_ID_2, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "FPI_MODE_TO_NATC_TABLE", 1, CORE_2_INDEX, &FPI_MODE_TO_NATC_TABLE_2, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING6_STACK", 1, CORE_2_INDEX, &PROCESSING6_STACK_2, 360, 1, 1 },
#endif
#if defined BCM4912
	{ "TCAM_GENERIC_FIELDS", 1, CORE_2_INDEX, &TCAM_GENERIC_FIELDS_2, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE", 1, CORE_2_INDEX, &CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE_2, 3, 1, 1 },
#endif
#if defined BCM4912
	{ "FPI_MODE_SEQ_TABLE", 1, CORE_2_INDEX, &FPI_MODE_SEQ_TABLE_2, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_2_INDEX, &LOOPBACK_QUEUE_TABLE_2, 31, 1, 1 },
#endif
#if defined BCM4912
	{ "AQM_NUM_QUEUES", 1, CORE_2_INDEX, &AQM_NUM_QUEUES_2, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "VPORT_CFG_EX_TABLE", 1, CORE_2_INDEX, &VPORT_CFG_EX_TABLE_2, 31, 1, 1 },
#endif
#if defined BCM4912
	{ "DOS_DROP_REASONS_CFG", 1, CORE_2_INDEX, &DOS_DROP_REASONS_CFG_2, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING7_STACK", 1, CORE_2_INDEX, &PROCESSING7_STACK_2, 360, 1, 1 },
#endif
#if defined BCM4912
	{ "TUNNELS_PARSING_CFG", 1, CORE_2_INDEX, &TUNNELS_PARSING_CFG_2, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_TX_INGRESS_DISPATCHER_CREDIT_TABLE", 1, CORE_2_INDEX, &CPU_TX_INGRESS_DISPATCHER_CREDIT_TABLE_2, 3, 1, 1 },
#endif
#if defined BCM4912
	{ "MUTEX_TABLE", 1, CORE_2_INDEX, &MUTEX_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "RING_CPU_TX_DESCRIPTOR_DATA_TABLE", 1, CORE_2_INDEX, &RING_CPU_TX_DESCRIPTOR_DATA_TABLE_2, 2, 1, 1 },
#endif
#if defined BCM4912
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_2_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_2, 32, 1, 1 },
#endif
#if defined BCM4912
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_2_INDEX, &BUFMNG_DESCRIPTOR_TABLE_2, 16, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_RECYCLE_STACK", 1, CORE_2_INDEX, &CPU_RECYCLE_STACK_2, 32, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_TX_0_STACK", 1, CORE_2_INDEX, &CPU_TX_0_STACK_2, 320, 1, 1 },
#endif
#if defined BCM4912
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_2_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_2, 31, 1, 1 },
#endif
#if defined BCM4912
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_2_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_2, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "AQM_ENABLE_TABLE", 1, CORE_2_INDEX, &AQM_ENABLE_TABLE_2, 5, 1, 1 },
#endif
#if defined BCM4912
	{ "TASK_IDX", 1, CORE_2_INDEX, &TASK_IDX_2, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "IPTV_CFG_TABLE", 1, CORE_2_INDEX, &IPTV_CFG_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_2_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_2, 31, 1, 1 },
#endif
#if defined BCM4912
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_2_INDEX, &LOOPBACK_WAN_FLOW_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "DEBUG_SCRATCHPAD", 1, CORE_2_INDEX, &DEBUG_SCRATCHPAD_2, 12, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_TX_RING_DESCRIPTOR_TABLE", 1, CORE_2_INDEX, &CPU_TX_RING_DESCRIPTOR_TABLE_2, 2, 1, 1 },
#endif
#if defined BCM4912
	{ "IPTV_CONFIGURATION_TABLE", 1, CORE_2_INDEX, &IPTV_CONFIGURATION_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "FPI_CFG_TABLE", 1, CORE_2_INDEX, &FPI_CFG_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "GENERAL_TIMER_ACTION_VEC", 1, CORE_2_INDEX, &GENERAL_TIMER_ACTION_VEC_2, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_TX_1_STACK", 1, CORE_2_INDEX, &CPU_TX_1_STACK_2, 320, 1, 1 },
#endif
#if defined BCM4912
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_2_INDEX, &BITS_CALC_MASKS_TABLE_2, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_2_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_2_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "FORCE_DSCP", 1, CORE_2_INDEX, &FORCE_DSCP_2, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "CORE_ID_TABLE", 1, CORE_2_INDEX, &CORE_ID_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "FPM_GLOBAL_CFG", 1, CORE_2_INDEX, &FPM_GLOBAL_CFG_2, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_2_INDEX, &RX_MIRRORING_CONFIGURATION_2, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_REDIRECT_MODE", 1, CORE_2_INDEX, &CPU_REDIRECT_MODE_2, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_2_INDEX, &DEBUG_PRINT_CORE_LOCK_2, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_TX_SYNC_FIFO_TABLE", 1, CORE_2_INDEX, &CPU_TX_SYNC_FIFO_TABLE_2, 2, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_FPM_THRESHOLDS", 1, CORE_2_INDEX, &DHD_FPM_THRESHOLDS_2, 3, 1, 1 },
#endif
#if defined BCM4912
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_2_INDEX, &TCAM_TABLE_CFG_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "SRAM_DUMMY_STORE", 1, CORE_2_INDEX, &SRAM_DUMMY_STORE_2, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_2_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_2_INDEX, &INGRESS_FILTER_1588_CFG_2, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_RECYCLE_RING_DESCRIPTOR_TABLE", 1, CORE_2_INDEX, &CPU_RECYCLE_RING_DESCRIPTOR_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE", 1, CORE_2_INDEX, &CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE_2, 2, 1, 1 },
#endif
#if defined BCM4912
	{ "QUEUE_THRESHOLD_VECTOR", 1, CORE_2_INDEX, &QUEUE_THRESHOLD_VECTOR_2, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_RECYCLE_INTERRUPT_SCRATCH", 1, CORE_2_INDEX, &CPU_RECYCLE_INTERRUPT_SCRATCH_2, 2, 1, 1 },
#endif
#if defined BCM4912
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_2_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_2, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "DEBUG_PRINT_TABLE", 1, CORE_2_INDEX, &DEBUG_PRINT_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "GDX_PARAMS_TABLE", 1, CORE_2_INDEX, &GDX_PARAMS_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_2_INDEX, &RX_MIRRORING_DIRECT_ENABLE_2, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_RECYCLE_SRAM_PD_FIFO", 1, CORE_2_INDEX, &CPU_RECYCLE_SRAM_PD_FIFO_2, 16, 1, 1 },
#endif
#if defined BCM4912
	{ "RX_FLOW_TABLE", 1, CORE_2_INDEX, &RX_FLOW_TABLE_2, 90, 1, 1 },
#endif
#if defined BCM4912
	{ "NAT_CACHE_CFG", 1, CORE_2_INDEX, &NAT_CACHE_CFG_2, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_2_INDEX, &FW_ERROR_VECTOR_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_RECYCLE_SHADOW_RD_IDX", 1, CORE_2_INDEX, &CPU_RECYCLE_SHADOW_RD_IDX_2, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_RECYCLE_SHADOW_WR_IDX", 1, CORE_2_INDEX, &CPU_RECYCLE_SHADOW_WR_IDX_2, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_2_INDEX, &NAT_CACHE_KEY0_MASK_2, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "MULTICAST_KEY_MASK", 1, CORE_2_INDEX, &MULTICAST_KEY_MASK_2, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_2_INDEX, &NATC_L2_VLAN_KEY_MASK_2, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_RECYCLE_NEXT_PTR_TABLE", 1, CORE_2_INDEX, &CPU_RECYCLE_NEXT_PTR_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "INGRESS_FILTER_CFG", 1, CORE_2_INDEX, &INGRESS_FILTER_CFG_2, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "POLICER_PARAMS_TABLE", 1, CORE_2_INDEX, &POLICER_PARAMS_TABLE_2, 80, 1, 1 },
#endif
#if defined BCM4912
	{ "NATC_L2_TOS_MASK", 1, CORE_2_INDEX, &NATC_L2_TOS_MASK_2, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "SG_CONTEXT_TABLE", 1, CORE_2_INDEX, &SG_CONTEXT_TABLE_2, 2, 1, 1 },
#endif
#if defined BCM4912
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_2_INDEX, &INGRESS_FILTER_PROFILE_TABLE_2, 16, 1, 1 },
#endif
#if defined BCM4912
	{ "NATC_TBL_CFG", 1, CORE_2_INDEX, &NATC_TBL_CFG_2, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "FC_FLOW_IP_ADDRESSES_TABLE", 1, CORE_2_INDEX, &FC_FLOW_IP_ADDRESSES_TABLE_2, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_3_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_3, 128, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING0_STACK", 1, CORE_3_INDEX, &PROCESSING0_STACK_3, 360, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR", 1, CORE_3_INDEX, &CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_RECYCLE_INTERRUPT_COALESCING_TABLE", 1, CORE_3_INDEX, &CPU_RECYCLE_INTERRUPT_COALESCING_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_FEED_RING_CACHE_TABLE", 1, CORE_3_INDEX, &CPU_FEED_RING_CACHE_TABLE_3, 16, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_RX_SCRATCHPAD", 1, CORE_3_INDEX, &CPU_RX_SCRATCHPAD_3, 128, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING1_STACK", 1, CORE_3_INDEX, &PROCESSING1_STACK_3, 360, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_RX_INTERRUPT_ID_DDR_ADDR", 1, CORE_3_INDEX, &CPU_RX_INTERRUPT_ID_DDR_ADDR_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_FEED_RING_DESCRIPTOR_TABLE", 1, CORE_3_INDEX, &CPU_FEED_RING_DESCRIPTOR_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_RX_METER_TABLE", 1, CORE_3_INDEX, &CPU_RX_METER_TABLE_3, 16, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING2_STACK", 1, CORE_3_INDEX, &PROCESSING2_STACK_3, 360, 1, 1 },
#endif
#if defined BCM4912
	{ "NULL_BUFFER", 1, CORE_3_INDEX, &NULL_BUFFER_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_RX_AUX_INT_SCRATCHPAD", 1, CORE_3_INDEX, &CPU_RX_AUX_INT_SCRATCHPAD_3, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_RX_COPY_STACK", 1, CORE_3_INDEX, &CPU_RX_COPY_STACK_3, 128, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING3_STACK", 1, CORE_3_INDEX, &PROCESSING3_STACK_3, 360, 1, 1 },
#endif
#if defined BCM4912
	{ "SYSTEM_CONFIGURATION", 1, CORE_3_INDEX, &SYSTEM_CONFIGURATION_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_RX_PSRAM_GET_NEXT_SCRATCHPAD", 1, CORE_3_INDEX, &CPU_RX_PSRAM_GET_NEXT_SCRATCHPAD_3, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_RXQ_SCRATCH_TABLE", 1, CORE_3_INDEX, &CPU_RXQ_SCRATCH_TABLE_3, 128, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING4_STACK", 1, CORE_3_INDEX, &PROCESSING4_STACK_3, 360, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE", 1, CORE_3_INDEX, &CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "SPDTEST_GEN_PARAM", 1, CORE_3_INDEX, &SPDTEST_GEN_PARAM_3, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "GENERAL_TIMER", 1, CORE_3_INDEX, &GENERAL_TIMER_3, 16, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_3_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER_3, 8, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING5_STACK", 1, CORE_3_INDEX, &PROCESSING5_STACK_3, 360, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_FEED_RING_INDEX_DDR_ADDR_TABLE", 1, CORE_3_INDEX, &CPU_FEED_RING_INDEX_DDR_ADDR_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "BUFMNG_STATUS_TABLE", 1, CORE_3_INDEX, &BUFMNG_STATUS_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "VPORT_CFG_TABLE", 1, CORE_3_INDEX, &VPORT_CFG_TABLE_3, 31, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_INT_INTERRUPT_SCRATCH", 1, CORE_3_INDEX, &CPU_INT_INTERRUPT_SCRATCH_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING6_STACK", 1, CORE_3_INDEX, &PROCESSING6_STACK_3, 360, 1, 1 },
#endif
#if defined BCM4912
	{ "TCAM_GENERIC_FIELDS", 1, CORE_3_INDEX, &TCAM_GENERIC_FIELDS_3, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_3_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_3, 16, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_REASON_AND_VPORT_TO_METER_TABLE", 1, CORE_3_INDEX, &CPU_REASON_AND_VPORT_TO_METER_TABLE_3, 120, 1, 1 },
#endif
#if defined BCM4912
	{ "TUNNELS_PARSING_CFG", 1, CORE_3_INDEX, &TUNNELS_PARSING_CFG_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING7_STACK", 1, CORE_3_INDEX, &PROCESSING7_STACK_3, 360, 1, 1 },
#endif
#if defined BCM4912
	{ "FPI_MODE_TO_NATC_TABLE", 1, CORE_3_INDEX, &FPI_MODE_TO_NATC_TABLE_3, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "FPI_MODE_SEQ_TABLE", 1, CORE_3_INDEX, &FPI_MODE_SEQ_TABLE_3, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "TR471_SPDSVC_RX_PKT_ID", 1, CORE_3_INDEX, &TR471_SPDSVC_RX_PKT_ID_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "MUTEX_TABLE", 1, CORE_3_INDEX, &MUTEX_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_RX_STACK", 1, CORE_3_INDEX, &CPU_RX_STACK_3, 80, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_RX_COPY_DISPATCHER_CREDIT_TABLE", 1, CORE_3_INDEX, &CPU_RX_COPY_DISPATCHER_CREDIT_TABLE_3, 3, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_RX_INTERRUPT_SCRATCH", 1, CORE_3_INDEX, &CPU_RX_INTERRUPT_SCRATCH_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_FEED_RING_RSV_TABLE", 1, CORE_3_INDEX, &CPU_FEED_RING_RSV_TABLE_3, 16, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_RECYCLE_SRAM_PD_FIFO", 1, CORE_3_INDEX, &CPU_RECYCLE_SRAM_PD_FIFO_3, 16, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_RING_INTERRUPT_COUNTER_TABLE", 1, CORE_3_INDEX, &CPU_RING_INTERRUPT_COUNTER_TABLE_3, 24, 1, 1 },
#endif
#if defined BCM4912
	{ "VPORT_TX_FLOW_TABLE", 1, CORE_3_INDEX, &VPORT_TX_FLOW_TABLE_3, 64, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_RX_COPY_PD_FIFO_TABLE", 1, CORE_3_INDEX, &CPU_RX_COPY_PD_FIFO_TABLE_3, 12, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_REASON_TO_METER_TABLE", 1, CORE_3_INDEX, &CPU_REASON_TO_METER_TABLE_3, 64, 1, 1 },
#endif
#if defined BCM4912
	{ "RX_FLOW_TABLE", 1, CORE_3_INDEX, &RX_FLOW_TABLE_3, 90, 1, 1 },
#endif
#if defined BCM4912
	{ "TASK_IDX", 1, CORE_3_INDEX, &TASK_IDX_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "IPTV_CFG_TABLE", 1, CORE_3_INDEX, &IPTV_CFG_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "IPV6_HOST_ADDRESS_CRC_TABLE", 1, CORE_3_INDEX, &IPV6_HOST_ADDRESS_CRC_TABLE_3, 16, 1, 1 },
#endif
#if defined BCM4912
	{ "CSO_CONTEXT_TABLE", 1, CORE_3_INDEX, &CSO_CONTEXT_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "IPTV_CONFIGURATION_TABLE", 1, CORE_3_INDEX, &IPTV_CONFIGURATION_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "AQM_NUM_QUEUES", 1, CORE_3_INDEX, &AQM_NUM_QUEUES_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "DOS_DROP_REASONS_CFG", 1, CORE_3_INDEX, &DOS_DROP_REASONS_CFG_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "GENERAL_TIMER_STACK", 1, CORE_3_INDEX, &GENERAL_TIMER_STACK_3, 64, 1, 1 },
#endif
#if defined BCM4912
	{ "POLICER_PARAMS_TABLE", 1, CORE_3_INDEX, &POLICER_PARAMS_TABLE_3, 80, 1, 1 },
#endif
#if defined BCM4912
	{ "IPV4_HOST_ADDRESS_TABLE", 1, CORE_3_INDEX, &IPV4_HOST_ADDRESS_TABLE_3, 8, 1, 1 },
#endif
#if defined BCM4912
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_3_INDEX, &LOOPBACK_QUEUE_TABLE_3, 31, 1, 1 },
#endif
#if defined BCM4912
	{ "GENERAL_TIMER_ACTION_VEC", 1, CORE_3_INDEX, &GENERAL_TIMER_ACTION_VEC_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_3_INDEX, &INGRESS_FILTER_PROFILE_TABLE_3, 16, 1, 1 },
#endif
#if defined BCM4912
	{ "REGISTERS_BUFFER", 1, CORE_3_INDEX, &REGISTERS_BUFFER_3, 32, 1, 1 },
#endif
#if defined BCM4912
	{ "NATC_TBL_CFG", 1, CORE_3_INDEX, &NATC_TBL_CFG_3, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_3_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_3, 8, 1, 1 },
#endif
#if defined BCM4912
	{ "VPORT_CFG_EX_TABLE", 1, CORE_3_INDEX, &VPORT_CFG_EX_TABLE_3, 31, 1, 1 },
#endif
#if defined BCM4912
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_3_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "UPDATE_FIFO_TABLE", 1, CORE_3_INDEX, &UPDATE_FIFO_TABLE_3, 8, 1, 1 },
#endif
#if defined BCM4912
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_3_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_3, 32, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_VPORT_TO_METER_TABLE", 1, CORE_3_INDEX, &CPU_VPORT_TO_METER_TABLE_3, 40, 1, 1 },
#endif
#if defined BCM4912
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_3_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_3_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "FPI_CFG_TABLE", 1, CORE_3_INDEX, &FPI_CFG_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_FEED_RING_INTERRUPT_THRESHOLD", 1, CORE_3_INDEX, &CPU_FEED_RING_INTERRUPT_THRESHOLD_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_RX_COPY_UPDATE_FIFO_TABLE", 1, CORE_3_INDEX, &CPU_RX_COPY_UPDATE_FIFO_TABLE_3, 8, 1, 1 },
#endif
#if defined BCM4912
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_3_INDEX, &BUFMNG_DESCRIPTOR_TABLE_3, 16, 1, 1 },
#endif
#if defined BCM4912
	{ "PD_FIFO_TABLE", 1, CORE_3_INDEX, &PD_FIFO_TABLE_3, 2, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_REASON_TO_TC", 1, CORE_3_INDEX, &CPU_REASON_TO_TC_3, 64, 1, 1 },
#endif
#if defined BCM4912
	{ "TC_TO_CPU_RXQ", 1, CORE_3_INDEX, &TC_TO_CPU_RXQ_3, 64, 1, 1 },
#endif
#if defined BCM4912
	{ "EXC_TC_TO_CPU_RXQ", 1, CORE_3_INDEX, &EXC_TC_TO_CPU_RXQ_3, 64, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_RECYCLE_STACK", 1, CORE_3_INDEX, &CPU_RECYCLE_STACK_3, 32, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_RXQ_DATA_BUF_TYPE_TABLE", 1, CORE_3_INDEX, &CPU_RXQ_DATA_BUF_TYPE_TABLE_3, 32, 1, 1 },
#endif
#if defined BCM4912
	{ "AQM_ENABLE_TABLE", 1, CORE_3_INDEX, &AQM_ENABLE_TABLE_3, 5, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_FEED_RING_INTERRUPT_COUNTER", 1, CORE_3_INDEX, &CPU_FEED_RING_INTERRUPT_COUNTER_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_3_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_3_INDEX, &LOOPBACK_WAN_FLOW_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "NAT_CACHE_CFG", 1, CORE_3_INDEX, &NAT_CACHE_CFG_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "FORCE_DSCP", 1, CORE_3_INDEX, &FORCE_DSCP_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_3_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_3, 31, 1, 1 },
#endif
#if defined BCM4912
	{ "CORE_ID_TABLE", 1, CORE_3_INDEX, &CORE_ID_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "FPM_GLOBAL_CFG", 1, CORE_3_INDEX, &FPM_GLOBAL_CFG_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_3_INDEX, &RX_MIRRORING_CONFIGURATION_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_FEED_RING_CACHE_OFFSET", 1, CORE_3_INDEX, &CPU_FEED_RING_CACHE_OFFSET_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_REDIRECT_MODE", 1, CORE_3_INDEX, &CPU_REDIRECT_MODE_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_RING_DESCRIPTORS_TABLE", 1, CORE_3_INDEX, &CPU_RING_DESCRIPTORS_TABLE_3, 24, 1, 1 },
#endif
#if defined BCM4912
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_3_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_3, 31, 1, 1 },
#endif
#if defined BCM4912
	{ "CSO_DISABLE", 1, CORE_3_INDEX, &CSO_DISABLE_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "DEBUG_SCRATCHPAD", 1, CORE_3_INDEX, &DEBUG_SCRATCHPAD_3, 12, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_FPM_THRESHOLDS", 1, CORE_3_INDEX, &DHD_FPM_THRESHOLDS_3, 3, 1, 1 },
#endif
#if defined BCM4912
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_3_INDEX, &DEBUG_PRINT_CORE_LOCK_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_3_INDEX, &TCAM_TABLE_CFG_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "SRAM_DUMMY_STORE", 1, CORE_3_INDEX, &SRAM_DUMMY_STORE_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_3_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "BCM_SPDSVC_STREAM_RX_TS_TABLE", 1, CORE_3_INDEX, &BCM_SPDSVC_STREAM_RX_TS_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_3_INDEX, &INGRESS_FILTER_1588_CFG_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_3_INDEX, &RX_MIRRORING_DIRECT_ENABLE_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_RECYCLE_RING_DESCRIPTOR_TABLE", 1, CORE_3_INDEX, &CPU_RECYCLE_RING_DESCRIPTOR_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "VPORT_TO_CPU_OBJ", 1, CORE_3_INDEX, &VPORT_TO_CPU_OBJ_3, 40, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE", 1, CORE_3_INDEX, &CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE_3, 2, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_RECYCLE_SHADOW_RD_IDX", 1, CORE_3_INDEX, &CPU_RECYCLE_SHADOW_RD_IDX_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_3_INDEX, &BITS_CALC_MASKS_TABLE_3, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_INTERRUPT_COALESCING_TABLE", 1, CORE_3_INDEX, &CPU_INTERRUPT_COALESCING_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_RECYCLE_INTERRUPT_SCRATCH", 1, CORE_3_INDEX, &CPU_RECYCLE_INTERRUPT_SCRATCH_3, 2, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_RX_LOCAL_SCRATCH", 1, CORE_3_INDEX, &CPU_RX_LOCAL_SCRATCH_3, 2, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_RECYCLE_SHADOW_WR_IDX", 1, CORE_3_INDEX, &CPU_RECYCLE_SHADOW_WR_IDX_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "DEBUG_PRINT_TABLE", 1, CORE_3_INDEX, &DEBUG_PRINT_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "GDX_PARAMS_TABLE", 1, CORE_3_INDEX, &GDX_PARAMS_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_3_INDEX, &FW_ERROR_VECTOR_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_3_INDEX, &NAT_CACHE_KEY0_MASK_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_RECYCLE_NEXT_PTR_TABLE", 1, CORE_3_INDEX, &CPU_RECYCLE_NEXT_PTR_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_3_INDEX, &NATC_L2_VLAN_KEY_MASK_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "MULTICAST_KEY_MASK", 1, CORE_3_INDEX, &MULTICAST_KEY_MASK_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "INGRESS_FILTER_CFG", 1, CORE_3_INDEX, &INGRESS_FILTER_CFG_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "NATC_L2_TOS_MASK", 1, CORE_3_INDEX, &NATC_L2_TOS_MASK_3, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "FC_FLOW_IP_ADDRESSES_TABLE", 1, CORE_3_INDEX, &FC_FLOW_IP_ADDRESSES_TABLE_3, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_FLOW_RING_CACHE_CTX_TABLE", 1, CORE_4_INDEX, &DHD_FLOW_RING_CACHE_CTX_TABLE_4, 48, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_DOORBELL_TX_POST_VALUE", 1, CORE_4_INDEX, &DHD_DOORBELL_TX_POST_VALUE_4, 64, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_AUX_INFO_CACHE_TABLE", 1, CORE_4_INDEX, &DHD_AUX_INFO_CACHE_TABLE_4, 48, 1, 1 },
#endif
#if defined BCM4912
	{ "RX_FLOW_TABLE", 1, CORE_4_INDEX, &RX_FLOW_TABLE_4, 90, 1, 1 },
#endif
#if defined BCM4912
	{ "FPI_MODE_TO_NATC_TABLE", 1, CORE_4_INDEX, &FPI_MODE_TO_NATC_TABLE_4, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "NULL_BUFFER", 1, CORE_4_INDEX, &NULL_BUFFER_4, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "VPORT_TX_FLOW_TABLE", 1, CORE_4_INDEX, &VPORT_TX_FLOW_TABLE_4, 64, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_POST_COMMON_RADIO_DATA", 1, CORE_4_INDEX, &DHD_POST_COMMON_RADIO_DATA_4, 3, 1, 1 },
#endif
#if defined BCM4912
	{ "SYSTEM_CONFIGURATION", 1, CORE_4_INDEX, &SYSTEM_CONFIGURATION_4, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "SPDTEST_GEN_PARAM", 1, CORE_4_INDEX, &SPDTEST_GEN_PARAM_4, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_4_INDEX, &LOOPBACK_QUEUE_TABLE_4, 31, 1, 1 },
#endif
#if defined BCM4912
	{ "AQM_NUM_QUEUES", 1, CORE_4_INDEX, &AQM_NUM_QUEUES_4, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "VPORT_CFG_TABLE", 1, CORE_4_INDEX, &VPORT_CFG_TABLE_4, 31, 1, 1 },
#endif
#if defined BCM4912
	{ "FPI_MODE_SEQ_TABLE", 1, CORE_4_INDEX, &FPI_MODE_SEQ_TABLE_4, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "POLICER_PARAMS_TABLE", 1, CORE_4_INDEX, &POLICER_PARAMS_TABLE_4, 80, 1, 1 },
#endif
#if defined BCM4912
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_4_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_4, 8, 1, 1 },
#endif
#if defined BCM4912
	{ "VPORT_CFG_EX_TABLE", 1, CORE_4_INDEX, &VPORT_CFG_EX_TABLE_4, 31, 1, 1 },
#endif
#if defined BCM4912
	{ "DOS_DROP_REASONS_CFG", 1, CORE_4_INDEX, &DOS_DROP_REASONS_CFG_4, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_4_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_4, 128, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING0_STACK", 1, CORE_4_INDEX, &PROCESSING0_STACK_4, 360, 1, 1 },
#endif
#if defined BCM4912
	{ "MIRRORING_SCRATCH", 1, CORE_4_INDEX, &MIRRORING_SCRATCH_4, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "BUFMNG_STATUS_TABLE", 1, CORE_4_INDEX, &BUFMNG_STATUS_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_BACKUP_INDEX_CACHE", 1, CORE_4_INDEX, &DHD_BACKUP_INDEX_CACHE_4, 3, 1, 1 },
#endif
#if defined BCM4912
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_4_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_4, 32, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_4_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER_4, 8, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING1_STACK", 1, CORE_4_INDEX, &PROCESSING1_STACK_4, 360, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR", 1, CORE_4_INDEX, &DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_4, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_4_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_4, 16, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_L2_HEADER", 1, CORE_4_INDEX, &DHD_L2_HEADER_4, 72, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_BACKUP_BASE_ADDR", 1, CORE_4_INDEX, &DHD_BACKUP_BASE_ADDR_4, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "TR471_SPDSVC_RX_PKT_ID", 1, CORE_4_INDEX, &TR471_SPDSVC_RX_PKT_ID_4, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "MUTEX_TABLE", 1, CORE_4_INDEX, &MUTEX_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_4_INDEX, &BUFMNG_DESCRIPTOR_TABLE_4, 16, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING2_STACK", 1, CORE_4_INDEX, &PROCESSING2_STACK_4, 360, 1, 1 },
#endif
#if defined BCM4912
	{ "REGISTERS_BUFFER", 1, CORE_4_INDEX, &REGISTERS_BUFFER_4, 32, 1, 1 },
#endif
#if defined BCM4912
	{ "TCAM_GENERIC_FIELDS", 1, CORE_4_INDEX, &TCAM_GENERIC_FIELDS_4, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "IPTV_CONFIGURATION_TABLE", 1, CORE_4_INDEX, &IPTV_CONFIGURATION_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "TASK_IDX", 1, CORE_4_INDEX, &TASK_IDX_4, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING3_STACK", 1, CORE_4_INDEX, &PROCESSING3_STACK_4, 360, 1, 1 },
#endif
#if defined BCM4912
	{ "TUNNELS_PARSING_CFG", 1, CORE_4_INDEX, &TUNNELS_PARSING_CFG_4, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_4_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "FPI_CFG_TABLE", 1, CORE_4_INDEX, &FPI_CFG_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_4_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_CPU_TX_POST_RING_DESCRIPTOR_TABLE", 1, CORE_4_INDEX, &DHD_CPU_TX_POST_RING_DESCRIPTOR_TABLE_4, 3, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_FPM_THRESHOLDS", 1, CORE_4_INDEX, &DHD_FPM_THRESHOLDS_4, 3, 1, 1 },
#endif
#if defined BCM4912
	{ "SPDSVC_WLAN_TXPOST_PARAMS_TABLE", 1, CORE_4_INDEX, &SPDSVC_WLAN_TXPOST_PARAMS_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_4_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_4, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_4_INDEX, &LOOPBACK_WAN_FLOW_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_CODEL_BIAS_SLOPE_TABLE", 1, CORE_4_INDEX, &DHD_CODEL_BIAS_SLOPE_TABLE_4, 11, 1, 1 },
#endif
#if defined BCM4912
	{ "FORCE_DSCP", 1, CORE_4_INDEX, &FORCE_DSCP_4, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "CORE_ID_TABLE", 1, CORE_4_INDEX, &CORE_ID_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_4_INDEX, &RX_MIRRORING_CONFIGURATION_4, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_CPU_INT_ID", 1, CORE_4_INDEX, &DHD_CPU_INT_ID_4, 3, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_REDIRECT_MODE", 1, CORE_4_INDEX, &CPU_REDIRECT_MODE_4, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_4_INDEX, &DEBUG_PRINT_CORE_LOCK_4, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_4_INDEX, &TCAM_TABLE_CFG_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "SRAM_DUMMY_STORE", 1, CORE_4_INDEX, &SRAM_DUMMY_STORE_4, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING4_STACK", 1, CORE_4_INDEX, &PROCESSING4_STACK_4, 360, 1, 1 },
#endif
#if defined BCM4912
	{ "IPTV_CFG_TABLE", 1, CORE_4_INDEX, &IPTV_CFG_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_MIRRORING_DISPATCHER_CREDIT_TABLE", 1, CORE_4_INDEX, &DHD_MIRRORING_DISPATCHER_CREDIT_TABLE_4, 3, 1, 1 },
#endif
#if defined BCM4912
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_4_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_4_INDEX, &INGRESS_FILTER_1588_CFG_4, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_4_INDEX, &RX_MIRRORING_DIRECT_ENABLE_4, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE", 1, CORE_4_INDEX, &DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE_4, 3, 1, 1 },
#endif
#if defined BCM4912
	{ "DEBUG_PRINT_TABLE", 1, CORE_4_INDEX, &DEBUG_PRINT_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_TX_POST_UPDATE_FIFO_TABLE", 1, CORE_4_INDEX, &DHD_TX_POST_UPDATE_FIFO_TABLE_4, 8, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_TIMER_STACK", 1, CORE_4_INDEX, &DHD_TIMER_STACK_4, 32, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING5_STACK", 1, CORE_4_INDEX, &PROCESSING5_STACK_4, 360, 1, 1 },
#endif
#if defined BCM4912
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_4_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_4, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "GDX_PARAMS_TABLE", 1, CORE_4_INDEX, &GDX_PARAMS_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_TX_POST_UPDATE_FIFO_STACK", 1, CORE_4_INDEX, &DHD_TX_POST_UPDATE_FIFO_STACK_4, 32, 1, 1 },
#endif
#if defined BCM4912
	{ "AQM_ENABLE_TABLE", 1, CORE_4_INDEX, &AQM_ENABLE_TABLE_4, 5, 1, 1 },
#endif
#if defined BCM4912
	{ "NAT_CACHE_CFG", 1, CORE_4_INDEX, &NAT_CACHE_CFG_4, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_4_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_4, 31, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_HW_CFG", 1, CORE_4_INDEX, &DHD_HW_CFG_4, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_4_INDEX, &NAT_CACHE_KEY0_MASK_4, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING6_STACK", 1, CORE_4_INDEX, &PROCESSING6_STACK_4, 360, 1, 1 },
#endif
#if defined BCM4912
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_4_INDEX, &NATC_L2_VLAN_KEY_MASK_4, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "INGRESS_FILTER_CFG", 1, CORE_4_INDEX, &INGRESS_FILTER_CFG_4, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "NATC_L2_TOS_MASK", 1, CORE_4_INDEX, &NATC_L2_TOS_MASK_4, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_4_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_4, 31, 1, 1 },
#endif
#if defined BCM4912
	{ "DEBUG_SCRATCHPAD", 1, CORE_4_INDEX, &DEBUG_SCRATCHPAD_4, 12, 1, 1 },
#endif
#if defined BCM4912
	{ "FPM_GLOBAL_CFG", 1, CORE_4_INDEX, &FPM_GLOBAL_CFG_4, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING7_STACK", 1, CORE_4_INDEX, &PROCESSING7_STACK_4, 360, 1, 1 },
#endif
#if defined BCM4912
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_4_INDEX, &BITS_CALC_MASKS_TABLE_4, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_4_INDEX, &FW_ERROR_VECTOR_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "MULTICAST_KEY_MASK", 1, CORE_4_INDEX, &MULTICAST_KEY_MASK_4, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_TX_POST_PD_FIFO_TABLE", 1, CORE_4_INDEX, &DHD_TX_POST_PD_FIFO_TABLE_4, 12, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_TX_POST_FLOW_RING_BUFFER", 1, CORE_4_INDEX, &DHD_TX_POST_FLOW_RING_BUFFER_4, 3, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_TX_POST_0_STACK", 1, CORE_4_INDEX, &DHD_TX_POST_0_STACK_4, 136, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_TX_POST_1_STACK", 1, CORE_4_INDEX, &DHD_TX_POST_1_STACK_4, 136, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_TX_POST_2_STACK", 1, CORE_4_INDEX, &DHD_TX_POST_2_STACK_4, 136, 1, 1 },
#endif
#if defined BCM4912
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_4_INDEX, &INGRESS_FILTER_PROFILE_TABLE_4, 16, 1, 1 },
#endif
#if defined BCM4912
	{ "NATC_TBL_CFG", 1, CORE_4_INDEX, &NATC_TBL_CFG_4, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_FLOW_RING_CACHE_LKP_TABLE", 1, CORE_4_INDEX, &DHD_FLOW_RING_CACHE_LKP_TABLE_4, 48, 1, 1 },
#endif
#if defined BCM4912
	{ "FC_FLOW_IP_ADDRESSES_TABLE", 1, CORE_4_INDEX, &FC_FLOW_IP_ADDRESSES_TABLE_4, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "TCPSPDTEST_STREAM_TABLE", 1, CORE_5_INDEX, &TCPSPDTEST_STREAM_TABLE_5, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_5_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_5, 8, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING0_STACK", 1, CORE_5_INDEX, &PROCESSING0_STACK_5, 360, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_RX_INTERRUPT_ID_DDR_ADDR", 1, CORE_5_INDEX, &CPU_RX_INTERRUPT_ID_DDR_ADDR_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "SPDTEST_GEN_PARAM", 1, CORE_5_INDEX, &SPDTEST_GEN_PARAM_5, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "TCPSPDTEST_SCRATCHPAD", 1, CORE_5_INDEX, &TCPSPDTEST_SCRATCHPAD_5, 16, 1, 1 },
#endif
#if defined BCM4912
	{ "CRYPTO_SESSION_PARAMS_TABLE", 1, CORE_5_INDEX, &CRYPTO_SESSION_PARAMS_TABLE_5, 64, 1, 1 },
#endif
#if defined BCM4912
	{ "RX_FLOW_TABLE", 1, CORE_5_INDEX, &RX_FLOW_TABLE_5, 90, 1, 1 },
#endif
#if defined BCM4912
	{ "PKTGEN_NO_SBPM_HDRS_CNTR", 1, CORE_5_INDEX, &PKTGEN_NO_SBPM_HDRS_CNTR_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "NULL_BUFFER", 1, CORE_5_INDEX, &NULL_BUFFER_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "VPORT_TX_FLOW_TABLE", 1, CORE_5_INDEX, &VPORT_TX_FLOW_TABLE_5, 64, 1, 1 },
#endif
#if defined BCM4912
	{ "PKTGEN_TX_STREAM_TABLE", 1, CORE_5_INDEX, &PKTGEN_TX_STREAM_TABLE_5, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_5_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_5, 32, 1, 1 },
#endif
#if defined BCM4912
	{ "COMMON_REPROCESSING_STACK", 1, CORE_5_INDEX, &COMMON_REPROCESSING_STACK_5, 64, 1, 1 },
#endif
#if defined BCM4912
	{ "SPDSVC_GEN_STACK", 1, CORE_5_INDEX, &SPDSVC_GEN_STACK_5, 128, 1, 1 },
#endif
#if defined BCM4912
	{ "POLICER_PARAMS_TABLE", 1, CORE_5_INDEX, &POLICER_PARAMS_TABLE_5, 80, 1, 1 },
#endif
#if defined BCM4912
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_5_INDEX, &BUFMNG_DESCRIPTOR_TABLE_5, 16, 1, 1 },
#endif
#if defined BCM4912
	{ "SPU_REQUEST_STACK", 1, CORE_5_INDEX, &SPU_REQUEST_STACK_5, 64, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_5_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER_5, 8, 1, 1 },
#endif
#if defined BCM4912
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_5_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_5, 128, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING1_STACK", 1, CORE_5_INDEX, &PROCESSING1_STACK_5, 360, 1, 1 },
#endif
#if defined BCM4912
	{ "SYSTEM_CONFIGURATION", 1, CORE_5_INDEX, &SYSTEM_CONFIGURATION_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "BUFMNG_STATUS_TABLE", 1, CORE_5_INDEX, &BUFMNG_STATUS_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "VPORT_CFG_TABLE", 1, CORE_5_INDEX, &VPORT_CFG_TABLE_5, 31, 1, 1 },
#endif
#if defined BCM4912
	{ "FPI_MODE_TO_NATC_TABLE", 1, CORE_5_INDEX, &FPI_MODE_TO_NATC_TABLE_5, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING2_STACK", 1, CORE_5_INDEX, &PROCESSING2_STACK_5, 360, 1, 1 },
#endif
#if defined BCM4912
	{ "TX_ABS_RECYCLE_COUNTERS", 1, CORE_5_INDEX, &TX_ABS_RECYCLE_COUNTERS_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_5_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_5, 16, 1, 1 },
#endif
#if defined BCM4912
	{ "COMMON_REPROCESSING_PD_FIFO_TABLE", 1, CORE_5_INDEX, &COMMON_REPROCESSING_PD_FIFO_TABLE_5, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_5_INDEX, &LOOPBACK_QUEUE_TABLE_5, 31, 1, 1 },
#endif
#if defined BCM4912
	{ "PKTGEN_CURR_SBPM_HDR_PTR", 1, CORE_5_INDEX, &PKTGEN_CURR_SBPM_HDR_PTR_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING3_STACK", 1, CORE_5_INDEX, &PROCESSING3_STACK_5, 360, 1, 1 },
#endif
#if defined BCM4912
	{ "TCAM_GENERIC_FIELDS", 1, CORE_5_INDEX, &TCAM_GENERIC_FIELDS_5, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "TR471_SPDSVC_RX_PKT_ID", 1, CORE_5_INDEX, &TR471_SPDSVC_RX_PKT_ID_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "FPI_MODE_SEQ_TABLE", 1, CORE_5_INDEX, &FPI_MODE_SEQ_TABLE_5, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "TCPSPDTEST_PD_FIFO_TABLE", 1, CORE_5_INDEX, &TCPSPDTEST_PD_FIFO_TABLE_5, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "VPORT_CFG_EX_TABLE", 1, CORE_5_INDEX, &VPORT_CFG_EX_TABLE_5, 31, 1, 1 },
#endif
#if defined BCM4912
	{ "PKTGEN_NUM_OF_AVAIL_SBPM_HDRS", 1, CORE_5_INDEX, &PKTGEN_NUM_OF_AVAIL_SBPM_HDRS_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING4_STACK", 1, CORE_5_INDEX, &PROCESSING4_STACK_5, 360, 1, 1 },
#endif
#if defined BCM4912
	{ "TUNNELS_PARSING_CFG", 1, CORE_5_INDEX, &TUNNELS_PARSING_CFG_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "SPDSVC_GEN_DISPATCHER_CREDIT_TABLE", 1, CORE_5_INDEX, &SPDSVC_GEN_DISPATCHER_CREDIT_TABLE_5, 3, 1, 1 },
#endif
#if defined BCM4912
	{ "MUTEX_TABLE", 1, CORE_5_INDEX, &MUTEX_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "SPU_REQUEST_PD_FIFO_TABLE", 1, CORE_5_INDEX, &SPU_REQUEST_PD_FIFO_TABLE_5, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "SPDSVC_GEN_PARAMS_TABLE", 1, CORE_5_INDEX, &SPDSVC_GEN_PARAMS_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "PKTGEN_SBPM_END_PTR", 1, CORE_5_INDEX, &PKTGEN_SBPM_END_PTR_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "AQM_NUM_QUEUES", 1, CORE_5_INDEX, &AQM_NUM_QUEUES_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING5_STACK", 1, CORE_5_INDEX, &PROCESSING5_STACK_5, 360, 1, 1 },
#endif
#if defined BCM4912
	{ "REGISTERS_BUFFER", 1, CORE_5_INDEX, &REGISTERS_BUFFER_5, 32, 1, 1 },
#endif
#if defined BCM4912
	{ "IPTV_CFG_TABLE", 1, CORE_5_INDEX, &IPTV_CFG_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "TCPSPDTEST_DISPATCHER_CREDIT_TABLE", 1, CORE_5_INDEX, &TCPSPDTEST_DISPATCHER_CREDIT_TABLE_5, 3, 1, 1 },
#endif
#if defined BCM4912
	{ "PKTGEN_BAD_GET_NEXT", 1, CORE_5_INDEX, &PKTGEN_BAD_GET_NEXT_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING6_STACK", 1, CORE_5_INDEX, &PROCESSING6_STACK_5, 360, 1, 1 },
#endif
#if defined BCM4912
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_5_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "COMMON_REPROCESSING_DISPATCHER_CREDIT_TABLE", 1, CORE_5_INDEX, &COMMON_REPROCESSING_DISPATCHER_CREDIT_TABLE_5, 3, 1, 1 },
#endif
#if defined BCM4912
	{ "PKTGEN_MAX_UT_PKTS", 1, CORE_5_INDEX, &PKTGEN_MAX_UT_PKTS_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "CRYPTO_SESSION_STATS_SCRATCH", 1, CORE_5_INDEX, &CRYPTO_SESSION_STATS_SCRATCH_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING7_STACK", 1, CORE_5_INDEX, &PROCESSING7_STACK_5, 360, 1, 1 },
#endif
#if defined BCM4912
	{ "NAT_CACHE_CFG", 1, CORE_5_INDEX, &NAT_CACHE_CFG_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_5_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "TCPSPDTEST_GEN_DISPATCHER_CREDIT_TABLE", 1, CORE_5_INDEX, &TCPSPDTEST_GEN_DISPATCHER_CREDIT_TABLE_5, 3, 1, 1 },
#endif
#if defined BCM4912
	{ "PKTGEN_UT_TRIGGER", 1, CORE_5_INDEX, &PKTGEN_UT_TRIGGER_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "PKTGEN_SBPM_HDR_BNS", 1, CORE_5_INDEX, &PKTGEN_SBPM_HDR_BNS_5, 28, 1, 1 },
#endif
#if defined BCM4912
	{ "TASK_IDX", 1, CORE_5_INDEX, &TASK_IDX_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "FPI_CFG_TABLE", 1, CORE_5_INDEX, &FPI_CFG_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "DOS_DROP_REASONS_CFG", 1, CORE_5_INDEX, &DOS_DROP_REASONS_CFG_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "SPU_OFFLOAD_PARAMS_TABLE", 1, CORE_5_INDEX, &SPU_OFFLOAD_PARAMS_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_5_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_5_INDEX, &LOOPBACK_WAN_FLOW_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "FORCE_DSCP", 1, CORE_5_INDEX, &FORCE_DSCP_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "CORE_ID_TABLE", 1, CORE_5_INDEX, &CORE_ID_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "SPDSVC_TCPSPDTEST_COMMON_TABLE", 1, CORE_5_INDEX, &SPDSVC_TCPSPDTEST_COMMON_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_5_INDEX, &RX_MIRRORING_CONFIGURATION_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "IPTV_CONFIGURATION_TABLE", 1, CORE_5_INDEX, &IPTV_CONFIGURATION_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_REDIRECT_MODE", 1, CORE_5_INDEX, &CPU_REDIRECT_MODE_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_5_INDEX, &DEBUG_PRINT_CORE_LOCK_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_5_INDEX, &TCAM_TABLE_CFG_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "SRAM_DUMMY_STORE", 1, CORE_5_INDEX, &SRAM_DUMMY_STORE_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "UDPSPDT_STREAM_TX_STAT_TABLE", 1, CORE_5_INDEX, &UDPSPDT_STREAM_TX_STAT_TABLE_5, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "SPU_OFFLOAD_SCRATCHPAD", 1, CORE_5_INDEX, &SPU_OFFLOAD_SCRATCHPAD_5, 9, 1, 1 },
#endif
#if defined BCM4912
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_5_INDEX, &NAT_CACHE_KEY0_MASK_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_5_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_5_INDEX, &INGRESS_FILTER_1588_CFG_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_5_INDEX, &RX_MIRRORING_DIRECT_ENABLE_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_5_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_5_INDEX, &INGRESS_FILTER_PROFILE_TABLE_5, 16, 1, 1 },
#endif
#if defined BCM4912
	{ "COMMON_REPROCESSING_UPDATE_FIFO_TABLE", 1, CORE_5_INDEX, &COMMON_REPROCESSING_UPDATE_FIFO_TABLE_5, 8, 1, 1 },
#endif
#if defined BCM4912
	{ "TCPSPDTEST_ENGINE_GLOBAL_TABLE", 1, CORE_5_INDEX, &TCPSPDTEST_ENGINE_GLOBAL_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_5_INDEX, &NATC_L2_VLAN_KEY_MASK_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "TCPSPDTEST_UPDATE_FIFO_TABLE", 1, CORE_5_INDEX, &TCPSPDTEST_UPDATE_FIFO_TABLE_5, 8, 1, 1 },
#endif
#if defined BCM4912
	{ "AQM_ENABLE_TABLE", 1, CORE_5_INDEX, &AQM_ENABLE_TABLE_5, 5, 1, 1 },
#endif
#if defined BCM4912
	{ "INGRESS_FILTER_CFG", 1, CORE_5_INDEX, &INGRESS_FILTER_CFG_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "UDPSPDT_TX_PARAMS_TABLE", 1, CORE_5_INDEX, &UDPSPDT_TX_PARAMS_TABLE_5, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "FPM_GLOBAL_CFG", 1, CORE_5_INDEX, &FPM_GLOBAL_CFG_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "SPU_REQUEST_UPDATE_FIFO_TABLE", 1, CORE_5_INDEX, &SPU_REQUEST_UPDATE_FIFO_TABLE_5, 8, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_FPM_THRESHOLDS", 1, CORE_5_INDEX, &DHD_FPM_THRESHOLDS_5, 3, 1, 1 },
#endif
#if defined BCM4912
	{ "PKTGEN_SBPM_EXTS", 1, CORE_5_INDEX, &PKTGEN_SBPM_EXTS_5, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "PKTGEN_SESSION_DATA", 1, CORE_5_INDEX, &PKTGEN_SESSION_DATA_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "PKTGEN_BBMSG_REPLY_SCRATCH", 1, CORE_5_INDEX, &PKTGEN_BBMSG_REPLY_SCRATCH_5, 2, 1, 1 },
#endif
#if defined BCM4912
	{ "NATC_L2_TOS_MASK", 1, CORE_5_INDEX, &NATC_L2_TOS_MASK_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "DEBUG_PRINT_TABLE", 1, CORE_5_INDEX, &DEBUG_PRINT_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "NATC_TBL_CFG", 1, CORE_5_INDEX, &NATC_TBL_CFG_5, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "GDX_PARAMS_TABLE", 1, CORE_5_INDEX, &GDX_PARAMS_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_5_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_5, 31, 1, 1 },
#endif
#if defined BCM4912
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_5_INDEX, &FW_ERROR_VECTOR_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_5_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_5, 31, 1, 1 },
#endif
#if defined BCM4912
	{ "DEBUG_SCRATCHPAD", 1, CORE_5_INDEX, &DEBUG_SCRATCHPAD_5, 12, 1, 1 },
#endif
#if defined BCM4912
	{ "MULTICAST_KEY_MASK", 1, CORE_5_INDEX, &MULTICAST_KEY_MASK_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "PKTGEN_FPM_UG_MGMT", 1, CORE_5_INDEX, &PKTGEN_FPM_UG_MGMT_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_5_INDEX, &BITS_CALC_MASKS_TABLE_5, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "PKTGEN_TX_STREAM_SCRATCH_TABLE", 1, CORE_5_INDEX, &PKTGEN_TX_STREAM_SCRATCH_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "FC_FLOW_IP_ADDRESSES_TABLE", 1, CORE_5_INDEX, &FC_FLOW_IP_ADDRESSES_TABLE_5, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_COMPLETE_COMMON_RADIO_DATA", 1, CORE_6_INDEX, &DHD_COMPLETE_COMMON_RADIO_DATA_6, 3, 1, 1 },
#endif
#if defined BCM4912
	{ "VPORT_TX_FLOW_TABLE", 1, CORE_6_INDEX, &VPORT_TX_FLOW_TABLE_6, 64, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_FPM_RINGS_AND_BUFMNG_REFILL_STACK", 1, CORE_6_INDEX, &CPU_FPM_RINGS_AND_BUFMNG_REFILL_STACK_6, 128, 1, 1 },
#endif
#if defined BCM4912
	{ "FPM_RING_CFG_TABLE", 1, CORE_6_INDEX, &FPM_RING_CFG_TABLE_6, 8, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_RX_COMPLETE_0_STACK", 1, CORE_6_INDEX, &DHD_RX_COMPLETE_0_STACK_6, 64, 1, 1 },
#endif
#if defined BCM4912
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_6_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_6, 128, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING0_STACK", 1, CORE_6_INDEX, &PROCESSING0_STACK_6, 360, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_RX_INTERRUPT_ID_DDR_ADDR", 1, CORE_6_INDEX, &CPU_RX_INTERRUPT_ID_DDR_ADDR_6, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "SPDTEST_GEN_PARAM", 1, CORE_6_INDEX, &SPDTEST_GEN_PARAM_6, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "SPU_RESPONSE_STACK", 1, CORE_6_INDEX, &SPU_RESPONSE_STACK_6, 128, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING1_STACK", 1, CORE_6_INDEX, &PROCESSING1_STACK_6, 360, 1, 1 },
#endif
#if defined BCM4912
	{ "NULL_BUFFER", 1, CORE_6_INDEX, &NULL_BUFFER_6, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "BUFMNG_STATUS_TABLE", 1, CORE_6_INDEX, &BUFMNG_STATUS_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "VPORT_CFG_TABLE", 1, CORE_6_INDEX, &VPORT_CFG_TABLE_6, 31, 1, 1 },
#endif
#if defined BCM4912
	{ "FPI_MODE_TO_NATC_TABLE", 1, CORE_6_INDEX, &FPI_MODE_TO_NATC_TABLE_6, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING2_STACK", 1, CORE_6_INDEX, &PROCESSING2_STACK_6, 360, 1, 1 },
#endif
#if defined BCM4912
	{ "SYSTEM_CONFIGURATION", 1, CORE_6_INDEX, &SYSTEM_CONFIGURATION_6, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_6_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_6, 16, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_RX_COMPLETE_FLOW_RING_BUFFER", 1, CORE_6_INDEX, &DHD_RX_COMPLETE_FLOW_RING_BUFFER_6, 3, 1, 1 },
#endif
#if defined BCM4912
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_6_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_6, 8, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING3_STACK", 1, CORE_6_INDEX, &PROCESSING3_STACK_6, 360, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR", 1, CORE_6_INDEX, &DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_6, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "TR471_SPDSVC_RX_PKT_ID", 1, CORE_6_INDEX, &TR471_SPDSVC_RX_PKT_ID_6, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "FPI_MODE_SEQ_TABLE", 1, CORE_6_INDEX, &FPI_MODE_SEQ_TABLE_6, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_RX_POST_FLOW_RING_BUFFER", 1, CORE_6_INDEX, &DHD_RX_POST_FLOW_RING_BUFFER_6, 3, 1, 1 },
#endif
#if defined BCM4912
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_6_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_6, 32, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING4_STACK", 1, CORE_6_INDEX, &PROCESSING4_STACK_6, 360, 1, 1 },
#endif
#if defined BCM4912
	{ "REGISTERS_BUFFER", 1, CORE_6_INDEX, &REGISTERS_BUFFER_6, 32, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_RX_POST_RING_SIZE", 1, CORE_6_INDEX, &DHD_RX_POST_RING_SIZE_6, 3, 1, 1 },
#endif
#if defined BCM4912
	{ "AQM_NUM_QUEUES", 1, CORE_6_INDEX, &AQM_NUM_QUEUES_6, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "SPU_RESPONSE_DISPATCHER_CREDIT_TABLE", 1, CORE_6_INDEX, &SPU_RESPONSE_DISPATCHER_CREDIT_TABLE_6, 3, 1, 1 },
#endif
#if defined BCM4912
	{ "MUTEX_TABLE", 1, CORE_6_INDEX, &MUTEX_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_6_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER_6, 8, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING5_STACK", 1, CORE_6_INDEX, &PROCESSING5_STACK_6, 360, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_RX_COMPLETE_RING_SIZE", 1, CORE_6_INDEX, &DHD_RX_COMPLETE_RING_SIZE_6, 3, 1, 1 },
#endif
#if defined BCM4912
	{ "DOS_DROP_REASONS_CFG", 1, CORE_6_INDEX, &DOS_DROP_REASONS_CFG_6, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "IPTV_CONFIGURATION_TABLE", 1, CORE_6_INDEX, &IPTV_CONFIGURATION_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "TASK_IDX", 1, CORE_6_INDEX, &TASK_IDX_6, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_RX_COMPLETE_1_STACK", 1, CORE_6_INDEX, &DHD_RX_COMPLETE_1_STACK_6, 64, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_RX_COMPLETE_2_STACK", 1, CORE_6_INDEX, &DHD_RX_COMPLETE_2_STACK_6, 64, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING6_STACK", 1, CORE_6_INDEX, &PROCESSING6_STACK_6, 360, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_TX_COMPLETE_RING_SIZE", 1, CORE_6_INDEX, &DHD_TX_COMPLETE_RING_SIZE_6, 3, 1, 1 },
#endif
#if defined BCM4912
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_6_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_6_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "FPI_CFG_TABLE", 1, CORE_6_INDEX, &FPI_CFG_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_6_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_6, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_6_INDEX, &LOOPBACK_WAN_FLOW_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_TX_COMPLETE_0_STACK", 1, CORE_6_INDEX, &DHD_TX_COMPLETE_0_STACK_6, 64, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_TX_COMPLETE_1_STACK", 1, CORE_6_INDEX, &DHD_TX_COMPLETE_1_STACK_6, 64, 1, 1 },
#endif
#if defined BCM4912
	{ "PROCESSING7_STACK", 1, CORE_6_INDEX, &PROCESSING7_STACK_6, 360, 1, 1 },
#endif
#if defined BCM4912
	{ "TCAM_GENERIC_FIELDS", 1, CORE_6_INDEX, &TCAM_GENERIC_FIELDS_6, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_FPM_THRESHOLDS", 1, CORE_6_INDEX, &DHD_FPM_THRESHOLDS_6, 3, 1, 1 },
#endif
#if defined BCM4912
	{ "FORCE_DSCP", 1, CORE_6_INDEX, &FORCE_DSCP_6, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "CORE_ID_TABLE", 1, CORE_6_INDEX, &CORE_ID_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_6_INDEX, &RX_MIRRORING_CONFIGURATION_6, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_TX_COMPLETE_2_STACK", 1, CORE_6_INDEX, &DHD_TX_COMPLETE_2_STACK_6, 64, 1, 1 },
#endif
#if defined BCM4912
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_6_INDEX, &LOOPBACK_QUEUE_TABLE_6, 31, 1, 1 },
#endif
#if defined BCM4912
	{ "CPU_REDIRECT_MODE", 1, CORE_6_INDEX, &CPU_REDIRECT_MODE_6, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_6_INDEX, &DEBUG_PRINT_CORE_LOCK_6, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "RX_FLOW_TABLE", 1, CORE_6_INDEX, &RX_FLOW_TABLE_6, 90, 1, 1 },
#endif
#if defined BCM4912
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_6_INDEX, &TCAM_TABLE_CFG_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "SRAM_DUMMY_STORE", 1, CORE_6_INDEX, &SRAM_DUMMY_STORE_6, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_6_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_6_INDEX, &INGRESS_FILTER_1588_CFG_6, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "TUNNELS_PARSING_CFG", 1, CORE_6_INDEX, &TUNNELS_PARSING_CFG_6, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "VPORT_CFG_EX_TABLE", 1, CORE_6_INDEX, &VPORT_CFG_EX_TABLE_6, 31, 1, 1 },
#endif
#if defined BCM4912
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_6_INDEX, &RX_MIRRORING_DIRECT_ENABLE_6, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "POLICER_PARAMS_TABLE", 1, CORE_6_INDEX, &POLICER_PARAMS_TABLE_6, 80, 1, 1 },
#endif
#if defined BCM4912
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_6_INDEX, &BUFMNG_DESCRIPTOR_TABLE_6, 16, 1, 1 },
#endif
#if defined BCM4912
	{ "SPU_RESPONSE_PARAMS_TABLE", 1, CORE_6_INDEX, &SPU_RESPONSE_PARAMS_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "IPTV_CFG_TABLE", 1, CORE_6_INDEX, &IPTV_CFG_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_6_INDEX, &INGRESS_FILTER_PROFILE_TABLE_6, 16, 1, 1 },
#endif
#if defined BCM4912
	{ "CRYPTO_SESSION_STATS_SCRATCH", 1, CORE_6_INDEX, &CRYPTO_SESSION_STATS_SCRATCH_6, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "NATC_TBL_CFG", 1, CORE_6_INDEX, &NATC_TBL_CFG_6, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "AQM_ENABLE_TABLE", 1, CORE_6_INDEX, &AQM_ENABLE_TABLE_6, 5, 1, 1 },
#endif
#if defined BCM4912
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_6_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_6, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE", 1, CORE_6_INDEX, &DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE_6, 3, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_DOORBELL_TX_COMPLETE_VALUE", 1, CORE_6_INDEX, &DHD_DOORBELL_TX_COMPLETE_VALUE_6, 3, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_TX_COMPLETE_FLOW_RING_BUFFER", 1, CORE_6_INDEX, &DHD_TX_COMPLETE_FLOW_RING_BUFFER_6, 3, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_DOORBELL_RX_COMPLETE_VALUE", 1, CORE_6_INDEX, &DHD_DOORBELL_RX_COMPLETE_VALUE_6, 3, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_RX_COMPLETE_DISPATCHER_CREDIT_TABLE", 1, CORE_6_INDEX, &DHD_RX_COMPLETE_DISPATCHER_CREDIT_TABLE_6, 12, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_DOORBELL_RX_POST_VALUE", 1, CORE_6_INDEX, &DHD_DOORBELL_RX_POST_VALUE_6, 3, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_TX_COMPLETE_DISPATCHER_CREDIT_TABLE", 1, CORE_6_INDEX, &DHD_TX_COMPLETE_DISPATCHER_CREDIT_TABLE_6, 12, 1, 1 },
#endif
#if defined BCM4912
	{ "SPU_RESPONSE_SCRATCHPAD", 1, CORE_6_INDEX, &SPU_RESPONSE_SCRATCHPAD_6, 9, 1, 1 },
#endif
#if defined BCM4912
	{ "NAT_CACHE_CFG", 1, CORE_6_INDEX, &NAT_CACHE_CFG_6, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_6_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_6, 31, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_HW_CFG", 1, CORE_6_INDEX, &DHD_HW_CFG_6, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_6_INDEX, &NAT_CACHE_KEY0_MASK_6, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_6_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_6, 31, 1, 1 },
#endif
#if defined BCM4912
	{ "DEBUG_SCRATCHPAD", 1, CORE_6_INDEX, &DEBUG_SCRATCHPAD_6, 12, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_CPU_INT_ID", 1, CORE_6_INDEX, &DHD_CPU_INT_ID_6, 3, 1, 1 },
#endif
#if defined BCM4912
	{ "FPM_GLOBAL_CFG", 1, CORE_6_INDEX, &FPM_GLOBAL_CFG_6, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "DHD_FPM_REPLY", 1, CORE_6_INDEX, &DHD_FPM_REPLY_6, 24, 1, 1 },
#endif
#if defined BCM4912
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_6_INDEX, &NATC_L2_VLAN_KEY_MASK_6, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "SPDSVC_WLAN_GEN_PARAMS_TABLE", 1, CORE_6_INDEX, &SPDSVC_WLAN_GEN_PARAMS_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "DEBUG_PRINT_TABLE", 1, CORE_6_INDEX, &DEBUG_PRINT_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_6_INDEX, &BITS_CALC_MASKS_TABLE_6, 4, 1, 1 },
#endif
#if defined BCM4912
	{ "GDX_PARAMS_TABLE", 1, CORE_6_INDEX, &GDX_PARAMS_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_6_INDEX, &FW_ERROR_VECTOR_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "INGRESS_FILTER_CFG", 1, CORE_6_INDEX, &INGRESS_FILTER_CFG_6, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "NATC_L2_TOS_MASK", 1, CORE_6_INDEX, &NATC_L2_TOS_MASK_6, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "MULTICAST_KEY_MASK", 1, CORE_6_INDEX, &MULTICAST_KEY_MASK_6, 1, 1, 1 },
#endif
#if defined BCM4912
	{ "FC_FLOW_IP_ADDRESSES_TABLE", 1, CORE_6_INDEX, &FC_FLOW_IP_ADDRESSES_TABLE_6, 4, 1, 1 },
#endif
};

TABLE_STACK_STRUCT RUNNER_STACK_TABLES[NUMBER_OF_STACK_TABLES] =
{
    { "PROCESSING0_STACK", CORE_0_INDEX, 0x1e00, 360},
    { "PROCESSING1_STACK", CORE_0_INDEX, 0x2600, 360},
    { "PROCESSING2_STACK", CORE_0_INDEX, 0x2800, 360},
    { "PROCESSING3_STACK", CORE_0_INDEX, 0x2a00, 360},
    { "UPDATE_FIFO_STACK", CORE_0_INDEX, 0x2dc0, 64},
    { "SPDSVC_ANALYZER_STACK", CORE_0_INDEX, 0x2e00, 256},
    { "PROCESSING0_STACK", CORE_1_INDEX, 0x1a00, 360},
    { "PROCESSING1_STACK", CORE_1_INDEX, 0x1c00, 360},
    { "PROCESSING2_STACK", CORE_1_INDEX, 0x1e00, 360},
    { "PROCESSING3_STACK", CORE_1_INDEX, 0x2000, 360},
    { "UPDATE_FIFO_STACK", CORE_1_INDEX, 0x26c0, 64},
    { "PROCESSING0_STACK", CORE_2_INDEX, 0xe00, 360},
    { "PROCESSING1_STACK", CORE_2_INDEX, 0x2200, 360},
    { "PROCESSING2_STACK", CORE_2_INDEX, 0x2400, 360},
    { "PROCESSING3_STACK", CORE_2_INDEX, 0x2600, 360},
    { "PROCESSING4_STACK", CORE_2_INDEX, 0x2800, 360},
    { "GENERAL_TIMER_STACK", CORE_2_INDEX, 0x29c0, 64},
    { "PROCESSING5_STACK", CORE_2_INDEX, 0x2a00, 360},
    { "PROCESSING6_STACK", CORE_2_INDEX, 0x2c00, 360},
    { "PROCESSING7_STACK", CORE_2_INDEX, 0x2e00, 360},
    { "CPU_RECYCLE_STACK", CORE_2_INDEX, 0x2fe0, 32},
    { "CPU_TX_0_STACK", CORE_2_INDEX, 0x3000, 320},
    { "CPU_TX_1_STACK", CORE_2_INDEX, 0x3200, 320},
    { "PROCESSING0_STACK", CORE_3_INDEX, 0x200, 360},
    { "PROCESSING1_STACK", CORE_3_INDEX, 0x800, 360},
    { "PROCESSING2_STACK", CORE_3_INDEX, 0xa00, 360},
    { "CPU_RX_COPY_STACK", CORE_3_INDEX, 0xb80, 128},
    { "PROCESSING3_STACK", CORE_3_INDEX, 0xc00, 360},
    { "PROCESSING4_STACK", CORE_3_INDEX, 0xe00, 360},
    { "PROCESSING5_STACK", CORE_3_INDEX, 0x2000, 360},
    { "PROCESSING6_STACK", CORE_3_INDEX, 0x2200, 360},
    { "PROCESSING7_STACK", CORE_3_INDEX, 0x2400, 360},
    { "CPU_RX_STACK", CORE_3_INDEX, 0x2580, 80},
    { "GENERAL_TIMER_STACK", CORE_3_INDEX, 0x2ac0, 64},
    { "CPU_RECYCLE_STACK", CORE_3_INDEX, 0x2f60, 32},
    { "PROCESSING0_STACK", CORE_4_INDEX, 0xe00, 360},
    { "PROCESSING1_STACK", CORE_4_INDEX, 0x2000, 360},
    { "PROCESSING2_STACK", CORE_4_INDEX, 0x2200, 360},
    { "PROCESSING3_STACK", CORE_4_INDEX, 0x2400, 360},
    { "PROCESSING4_STACK", CORE_4_INDEX, 0x2600, 360},
    { "DHD_TIMER_STACK", CORE_4_INDEX, 0x27e0, 32},
    { "PROCESSING5_STACK", CORE_4_INDEX, 0x2800, 360},
    { "DHD_TX_POST_UPDATE_FIFO_STACK", CORE_4_INDEX, 0x2980, 32},
    { "PROCESSING6_STACK", CORE_4_INDEX, 0x2a00, 360},
    { "PROCESSING7_STACK", CORE_4_INDEX, 0x2c00, 360},
    { "DHD_TX_POST_0_STACK", CORE_4_INDEX, 0x3000, 136},
    { "DHD_TX_POST_1_STACK", CORE_4_INDEX, 0x3100, 136},
    { "DHD_TX_POST_2_STACK", CORE_4_INDEX, 0x3200, 136},
    { "PROCESSING0_STACK", CORE_5_INDEX, 0x600, 360},
    { "COMMON_REPROCESSING_STACK", CORE_5_INDEX, 0xe40, 64},
    { "SPDSVC_GEN_STACK", CORE_5_INDEX, 0xe80, 128},
    { "SPU_REQUEST_STACK", CORE_5_INDEX, 0xfc0, 64},
    { "PROCESSING1_STACK", CORE_5_INDEX, 0x2200, 360},
    { "PROCESSING2_STACK", CORE_5_INDEX, 0x2400, 360},
    { "PROCESSING3_STACK", CORE_5_INDEX, 0x2600, 360},
    { "PROCESSING4_STACK", CORE_5_INDEX, 0x2800, 360},
    { "PROCESSING5_STACK", CORE_5_INDEX, 0x2a00, 360},
    { "PROCESSING6_STACK", CORE_5_INDEX, 0x2c00, 360},
    { "PROCESSING7_STACK", CORE_5_INDEX, 0x2e00, 360},
    { "CPU_FPM_RINGS_AND_BUFMNG_REFILL_STACK", CORE_6_INDEX, 0x280, 128},
    { "DHD_RX_COMPLETE_0_STACK", CORE_6_INDEX, 0x3c0, 64},
    { "PROCESSING0_STACK", CORE_6_INDEX, 0x600, 360},
    { "SPU_RESPONSE_STACK", CORE_6_INDEX, 0x780, 128},
    { "PROCESSING1_STACK", CORE_6_INDEX, 0x800, 360},
    { "PROCESSING2_STACK", CORE_6_INDEX, 0xa00, 360},
    { "PROCESSING3_STACK", CORE_6_INDEX, 0xc00, 360},
    { "PROCESSING4_STACK", CORE_6_INDEX, 0xe00, 360},
    { "PROCESSING5_STACK", CORE_6_INDEX, 0x2000, 360},
    { "DHD_RX_COMPLETE_1_STACK", CORE_6_INDEX, 0x2180, 64},
    { "DHD_RX_COMPLETE_2_STACK", CORE_6_INDEX, 0x21c0, 64},
    { "PROCESSING6_STACK", CORE_6_INDEX, 0x2200, 360},
    { "DHD_TX_COMPLETE_0_STACK", CORE_6_INDEX, 0x2380, 64},
    { "DHD_TX_COMPLETE_1_STACK", CORE_6_INDEX, 0x23c0, 64},
    { "PROCESSING7_STACK", CORE_6_INDEX, 0x2400, 360},
    { "DHD_TX_COMPLETE_2_STACK", CORE_6_INDEX, 0x2580, 64}
};
