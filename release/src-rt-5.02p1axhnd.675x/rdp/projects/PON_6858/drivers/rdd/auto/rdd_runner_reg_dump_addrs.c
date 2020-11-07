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
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DS_TM_PD_FIFO_TABLE =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT COMPLEX_SCHEDULER_TABLE =
{
	64,
	{
		{ dump_RDD_COMPLEX_SCHEDULER_DESCRIPTOR, 0x1400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DS_TM_SCHEDULING_QUEUE_TABLE =
{
	8,
	{
		{ dump_RDD_SCHEDULING_QUEUE_DESCRIPTOR, 0x1800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_TBL_CFG =
{
	24,
	{
		{ dump_RDD_NATC_TBL_CONFIGURATION, 0x1d00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DS_TM_BBH_TX_WAKE_UP_DATA_TABLE =
{
	8,
	{
		{ dump_RDD_BBH_TX_WAKE_UP_DATA_ENTRY, 0x1d48 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DS_TM_FLUSH_DISPATCHER_CREDIT_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1d50 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TM_HW_FLUSH =
{
	4,
	{
		{ dump_RDD_HW_FLUSH_ENTRY, 0x1d5c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1d60 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DS_TM_BBH_TX_EGRESS_COUNTER_TABLE =
{
	8,
	{
		{ dump_RDD_BBH_TX_EGRESS_COUNTER_ENTRY, 0x1d80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DS_TM_TM_FLOW_CNTR_TABLE =
{
	1,
	{
		{ dump_RDD_TM_FLOW_CNTR_ENTRY, 0x1dc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT BASIC_SCHEDULER_TABLE_DS =
{
	16,
	{
		{ dump_RDD_BASIC_SCHEDULER_DESCRIPTOR, 0x1e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT BASIC_RATE_LIMITER_TABLE_DS =
{
	16,
	{
		{ dump_RDD_BASIC_RATE_LIMITER_DESCRIPTOR, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VLAN_TX_COUNTERS =
{
	8,
	{
		{ dump_RDD_PACKETS_AND_BYTES, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DBG_DUMP_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3008 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DS_TM_CPU_TX_ABS_COUNTERS =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3208 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3288 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE =
{
	8,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x3308 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_DISPATCHER_CREDIT_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3310 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT ZERO_VALUE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x331c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DS_TM_SCHEDULING_FLUSH_VECTOR =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3320 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT ONE_VALUE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3334 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MIRRORING_SCRATCH =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x3338 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DS_TM_TM_ACTION_PTR_TABLE =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3340 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT BUDGET_ALLOCATION_TIMER_VALUE =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3362 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DS_TM_BB_DESTINATION_TABLE =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3364 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DS_TM_SCHEDULING_GLOBAL_FLUSH_CFG =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3366 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SCHEDULING_FLUSH_GLOBAL_CFG =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3367 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TASK_IDX =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TM_VLAN_STATS_ENABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x336c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TX_EXCEPTION =
{
	1,
	{
		{ dump_RDD_TX_EXCEPTION_ENTRY, 0x336d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x336e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DS_TM_FLUSH_AGGREGATION_TASK_DISABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x336f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, 0x3370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT UPDATE_FIFO_TABLE =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x3380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DS_TM_SCHEDULING_QUEUE_AGING_VECTOR =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x33a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x33b4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x33b6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RATE_LIMIT_OVERHEAD =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x33b7 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DS_TM_FIRST_QUEUE_MAPPING =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x33b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT BACKUP_BBH_INGRESS_COUNTERS_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x33b9 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT BACKUP_BBH_EGRESS_COUNTERS_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x33ba },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x33bb },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DS_TM_SCHEDULING_AGGREGATION_CONTEXT_VECTOR =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x33c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x33d8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, 0x3400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DS_TM_BBH_QUEUE_TABLE =
{
	4,
	{
		{ dump_RDD_BBH_QUEUE_DESCRIPTOR, 0x3600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RATE_LIMITER_VALID_TABLE_DS =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3620 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG =
{
	12,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, 0x3630 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_STREAM_TABLE_1 =
{
	240,
	{
		{ dump_RDD_TCPSPDTEST_STREAM, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DS_CPU_REASON_TO_METER_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_RX_SCRATCHPAD_1 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT WLAN_MCAST_COPY_SCRATCHPAD_1 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_FLOW_RING_CACHE_CTX_TABLE_1 =
{
	16,
	{
		{ dump_RDD_DHD_FLOW_RING_CACHE_CTX_ENTRY, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_RX_TASK_STACK_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0xb00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PKTGEN_TX_STREAM_TABLE_1 =
{
	136,
	{
		{ dump_RDD_PKTGEN_TX_STREAM_ENTRY, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT WLAN_MCAST_SSID_STATS_TABLE_1 =
{
	8,
	{
		{ dump_RDD_WLAN_MCAST_SSID_STATS_ENTRY, 0xe20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0xfa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT US_CPU_REASON_TO_METER_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xfc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_1 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_POST_COMMON_RADIO_DATA_1 =
{
	136,
	{
		{ dump_RDD_DHD_POST_COMMON_RADIO_ENTRY, 0x1200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_1 =
{
	8,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x1398 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_TO_FLOW_IDX_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x13a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPV6_HOST_ADDRESS_CRC_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x13c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT WLAN_MCAST_DHD_STATION_TABLE_1 =
{
	8,
	{
		{ dump_RDD_WLAN_MCAST_DHD_STATION_ENTRY, 0x1400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_BACKUP_INFO_CACHE_TABLE_1 =
{
	8,
	{
		{ dump_RDD_DHD_BACKUP_INFO_CACHE_ENTRY, 0x1600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DS_CPU_RX_METER_TABLE_1 =
{
	8,
	{
		{ dump_RDD_CPU_RX_METER_ENTRY, 0x1780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT WLAN_MCAST_SSID_MAC_ADDRESS_TABLE_1 =
{
	8,
	{
		{ dump_RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY, 0x1800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT US_CPU_RX_METER_TABLE_1 =
{
	8,
	{
		{ dump_RDD_CPU_RX_METER_ENTRY, 0x1980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_SRAM_PD_FIFO_1 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x1a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DBG_DUMP_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_RING_INTERRUPT_COUNTER_TABLE_1 =
{
	8,
	{
		{ dump_RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY, 0x1d00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_FPM_POOL_NUMBER_MAPPING_TABLE_1 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0x1d90 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_RSV_TABLE_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x1da0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT WLAN_MCAST_DHD_STATION_CTX_TABLE_1 =
{
	1,
	{
		{ dump_RDD_WLAN_MCAST_DHD_STATION_CTX_ENTRY, 0x1dc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_PD_FIFO_TABLE_1 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x1e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT WLAN_MCAST_DFT_LIST_ENTRY_SCRATCH_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1ec0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_FLOW_RING_BUFFER_1 =
{
	48,
	{
		{ dump_RDD_DHD_TX_POST_DESCRIPTOR, 0x1f00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_INTERRUPT_COALESCING_TABLE_1 =
{
	16,
	{
		{ dump_RDD_CPU_INTERRUPT_COALESCING_ENTRY, 0x1f90 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPV4_HOST_ADDRESS_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1fa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SPDSVC_GEN_PARAMS_TABLE_1 =
{
	60,
	{
		{ dump_RDD_SPDSVC_GEN_PARAMS, 0x1fc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PKTGEN_NO_SBPM_HDRS_CNTR_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1ffc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_RX_COPY_PD_FIFO_TABLE_1 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_REASON_AND_VPORT_TO_METER_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2080 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TX_ABS_RECYCLE_COUNTERS_1 =
{
	8,
	{
		{ dump_RDD_TX_ABS_RECYCLE_COUNTERS_ENTRY, 0x20f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CSO_CONTEXT_TABLE_1 =
{
	108,
	{
		{ dump_RDD_CSO_CONTEXT_ENTRY, 0x2100 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_1 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, 0x216c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_RX_COPY_INT_SCRATCHPAD_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_BACKUP_INDEX_CACHE_1 =
{
	32,
	{
		{ dump_RDD_DHD_BACKUP_IDX_CACHE_TABLE, 0x2180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PKTGEN_FPM_UG_MGMT_1 =
{
	20,
	{
		{ dump_RDD_PKTGEN_FPM_UG_MGMT_ENTRY, 0x21e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT ZERO_VALUE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x21f4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_1 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0x21f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_FLOW_RING_CACHE_LKP_TABLE_1 =
{
	2,
	{
		{ dump_RDD_DHD_FLOW_RING_CACHE_LKP_ENTRY, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_ENGINE_GLOBAL_TABLE_1 =
{
	20,
	{
		{ dump_RDD_TCPSPDTEST_ENGINE_GLOBAL_INFO, 0x2260 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT ONE_VALUE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2274 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_RX_INTERRUPT_ID_DDR_ADDR_1 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0x2278 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_L2_HEADER_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2280 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x22c8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_RX_PSRAM_GET_NEXT_SCRATCHPAD_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x22f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_TBL_CFG_1 =
{
	24,
	{
		{ dump_RDD_NATC_TBL_CONFIGURATION, 0x2300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MIRRORING_SCRATCH_1 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x2348 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_1 =
{
	16,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, 0x2350 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_MCAST_DISPATCHER_CREDIT_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2360 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_RX_INTERRUPT_SCRATCH_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x236c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_MIRRORING_DISPATCHER_CREDIT_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PKTGEN_CURR_SBPM_HDR_PTR_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x237c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PKTGEN_NUM_OF_AVAIL_SBPM_HDRS_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x237e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT COMMON_REPROCESSING_PD_FIFO_TABLE_1 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x2380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x23c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT WLAN_MCAST_SCRATCHPAD_1 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x2440 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_1 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0x2468 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_RX_COPY_DISPATCHER_CREDIT_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2470 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PKTGEN_SBPM_END_PTR_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x247c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SPDSVC_WLAN_TXPOST_PARAMS_TABLE_1 =
{
	2,
	{
		{ dump_RDD_SPDSVC_WLAN_TXPOST_PARAMS, 0x247e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT WLAN_MCAST_DFT_LIST_SIZE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2480 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT UPDATE_FIFO_TABLE_1 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x24c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT WAN_LOOPBACK_DISPATCHER_CREDIT_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x24e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PKTGEN_BAD_GET_NEXT_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x24ec },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SPDSVC_GEN_DISPATCHER_CREDIT_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x24f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PKTGEN_MAX_UT_PKTS_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x24fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PKTGEN_SBPM_HDR_BNS_1 =
{
	2,
	{
		{ dump_RDD_PKTGEN_SBPM_HDR_BN, 0x2500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_DOORBELL_TX_POST_VALUE_1 =
{
	4,
	{
		{ dump_RDD_DHD_DOORBELL, 0x2538 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PKTGEN_UT_TRIGGER_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x257c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT UDPSPDTEST_STREAM_RX_STAT_TABLE_1 =
{
	48,
	{
		{ dump_RDD_UDPSPDTEST_STREAM_RX_STAT, 0x2580 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT COMMON_REPROCESSING_DISPATCHER_CREDIT_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x25b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TASK_IDX_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x25bc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_UPDATE_FIFO_TABLE_1 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x25c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT BCM_SPDSVC_STREAM_RX_TS_TABLE_1 =
{
	12,
	{
		{ dump_RDD_SPDSVC_RX_TS_STAT, 0x25e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x25ec },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SPDSVC_TCPSPDTEST_COMMON_TABLE_1 =
{
	1,
	{
		{ dump_RDD_SPDSVC_TCPSPDTEST_COMMON_ENTRY, 0x25ed },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_INTERRUPT_THRESHOLD_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x25ee },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_DESCRIPTOR_TABLE_1 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x25f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE_1 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE_1 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0x2630 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_MCAST_UPDATE_FIFO_TABLE_1 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x2640 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_INTERRUPT_COALESCING_TABLE_1 =
{
	16,
	{
		{ dump_RDD_CPU_INTERRUPT_COALESCING_ENTRY, 0x2660 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_DESCRIPTOR_TABLE_1 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x2670 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_VPORT_TO_METER_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_BACKUP_BASE_ADDR_1 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x26a8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_INTERRUPT_SCRATCH_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x26b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT WLAN_MCAST_DFT_ADDR_1 =
{
	8,
	{
		{ dump_RDD_PHYS_ADDR_64_PTR, 0x26b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_RX_COPY_UPDATE_FIFO_TABLE_1 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x26c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_RX_LOCAL_SCRATCH_1 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x26e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PKTGEN_SBPM_EXTS_1 =
{
	4,
	{
		{ dump_RDD_PKTGEN_SBPM_EXT, 0x26f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT UDPSPDTEST_STREAM_TX_STAT_TABLE_1 =
{
	36,
	{
		{ dump_RDD_UDPSPDTEST_STREAM_TX_STAT, 0x2700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_INTERRUPT_COUNTER_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2724 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2726 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2727 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE_1 =
{
	8,
	{
		{ dump_RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY, 0x2728 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PKTGEN_BBMSG_REPLY_SCRATCH_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2730 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_RX_COPY_LOCAL_SCRATCH_1 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x2738 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT COMMON_REPROCESSING_UPDATE_FIFO_TABLE_1 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x2740 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_1 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, 0x2760 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_1 =
{
	12,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, 0x2770 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PD_FIFO_TABLE_1 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x2780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_SHADOW_RD_IDX_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x27a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_SHADOW_WR_IDX_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x27a8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_MCAST_PD_FIFO_TABLE_1 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x27c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_REASON_TO_TC_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x27e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TC_TO_CPU_RXQ_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2820 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT EXC_TC_TO_CPU_RXQ_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2860 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PKTGEN_SESSION_DATA_1 =
{
	24,
	{
		{ dump_RDD_PKTGEN_TX_PARAMS, 0x28c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_TO_CPU_OBJ_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2900 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PKTGEN_TX_STREAM_SCRATCH_TABLE_1 =
{
	8,
	{
		{ dump_RDD_PKTGEN_TX_STREAM_SCRATCH_ENTRY, 0x2980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_RING_DESCRIPTORS_TABLE_1 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x3000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_TX_SCRATCHPAD_2 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT REPORTING_QUEUE_DESCRIPTOR_TABLE_2 =
{
	16,
	{
		{ dump_RDD_REPORTING_QUEUE_DESCRIPTOR, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_COMPLETE_COMMON_RADIO_DATA_2 =
{
	112,
	{
		{ dump_RDD_DHD_COMPLETE_COMMON_RADIO_ENTRY, 0x1a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_TX_FPM_POOL_NUMBER_MAPPING_TABLE_2 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0x1b50 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1b60 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_TX_DBG_CNTRS_TABLE_2 =
{
	64,
	{
		{ dump_RDD_CPU_TX_DBG_CNTRS, 0x1b80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_2 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, 0x1c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FW_POLICER_CBS_2 =
{
	4,
	{
		{ dump_RDD_CBS_ENTRY, 0x1e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_COMPLEX_SCHEDULER_TABLE_2 =
{
	64,
	{
		{ dump_RDD_COMPLEX_SCHEDULER_DESCRIPTOR, 0x1f40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT REPORTING_QUEUE_ACCUMULATED_TABLE_2 =
{
	8,
	{
		{ dump_RDD_REPORTING_ACCUMULATED_DATA, 0x1f80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_RATE_LIMITER_TABLE_2 =
{
	16,
	{
		{ dump_RDD_BASIC_RATE_LIMITER_DESCRIPTOR, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_SCHEDULING_QUEUE_AGING_VECTOR_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2420 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_2 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, 0x2434 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_2 =
{
	8,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x2438 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_FLOW_RING_BUFFER_2 =
{
	16,
	{
		{ dump_RDD_DHD_TX_COMPLETE_DESCRIPTOR, 0x2440 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FPM_REPLY_2 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x2470 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_FLOW_RING_BUFFER_2 =
{
	32,
	{
		{ dump_RDD_DHD_RX_COMPLETE_DESCRIPTOR, 0x2480 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT GHOST_REPORTING_QUEUE_STATUS_BIT_VECTOR_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x24e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT GHOST_REPORTING_GLOBAL_CFG_2 =
{
	4,
	{
		{ dump_RDD_GHOST_REPORTING_GLOBAL_CFG_ENTRY, 0x24f4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TIMER_COMMON_CTR_REP_2 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x24f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE_2 =
{
	8,
	{
		{ dump_RDD_SCHEDULING_QUEUE_DESCRIPTOR, 0x2500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_SRAM_PD_FIFO_2 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_2 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, 0x2700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT QUEUE_THRESHOLD_VECTOR_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x27c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT ZERO_VALUE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x27e4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_2 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0x27e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_INTERRUPT_COALESCING_TABLE_2 =
{
	16,
	{
		{ dump_RDD_CPU_INTERRUPT_COALESCING_ENTRY, 0x27f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_PD_FIFO_TABLE_2 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DBG_DUMP_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FW_POLICER_BUDGET_2 =
{
	2,
	{
		{ dump_RDD_FW_POLICER_BUDGET_ENTRY, 0x2e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FW_POLICER_BUDGET_REMAINDER_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2ea0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_2 =
{
	16,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, 0x2ef0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT REPORTING_COUNTER_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2f00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT QUEUE_TO_REPORT_BIT_VECTOR_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2fa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT ONE_VALUE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2fb4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_RX_INTERRUPT_ID_DDR_ADDR_2 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0x2fb8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_TM_ACTION_PTR_TABLE_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2fc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TIMER_COMMON_TIMER_VALUE_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2fe2 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TASK_IDX_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2fe4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_2 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0x2fe8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2ff0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_BUDGET_ALLOCATION_TIMER_VALUE_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2ffc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT EMAC_FLOW_CTRL_VECTOR_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2ffe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_TX_VLAN_STATS_ENABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2fff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_RX_POST_FLOW_RING_BUFFER_2 =
{
	32,
	{
		{ dump_RDD_DHD_RX_POST_DESCRIPTOR, 0x3000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3060 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_TX_RING_DESCRIPTOR_TABLE_2 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x30e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_TBL_CFG_2 =
{
	24,
	{
		{ dump_RDD_NATC_TBL_CONFIGURATION, 0x3100 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3148 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_TX_INGRESS_DISPATCHER_CREDIT_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x317c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RATE_LIMIT_OVERHEAD_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x317d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_RX_POST_RING_SIZE_2 =
{
	2,
	{
		{ dump_RDD_DHD_RING_SIZE, 0x317e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE_2 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x3180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_0_DISPATCHER_CREDIT_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x31b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_RING_SIZE_2 =
{
	2,
	{
		{ dump_RDD_DHD_RING_SIZE, 0x31bc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_RING_SIZE_2 =
{
	2,
	{
		{ dump_RDD_DHD_RING_SIZE, 0x31be },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT EMAC_FLOW_CTRL_BUDGET_2 =
{
	4,
	{
		{ dump_RDD_EMAC_FLOW_CTRL_BUDGET_ENTRY, 0x31c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_RATE_LIMITER_VALID_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x31e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_1_DISPATCHER_CREDIT_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x31f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x31fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x31fd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_FIRST_QUEUE_MAPPING_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x31fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT BBH_TX_INGRESS_COUNTER_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x31ff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RING_CPU_TX_DESCRIPTOR_DATA_TABLE_2 =
{
	16,
	{
		{ dump_RDD_RING_CPU_TX_DESCRIPTOR, 0x3200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_2_DISPATCHER_CREDIT_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3220 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_0_DISPATCHER_CREDIT_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3230 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_UPDATE_FIFO_TABLE_2 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x3240 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_1_DISPATCHER_CREDIT_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3260 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_2_DISPATCHER_CREDIT_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3270 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_FPM_REPLY_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3280 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT REPORT_BBH_TX_QUEUE_ID_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3298 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FW_POLICER_VECTOR_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x32a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_DOORBELL_TX_COMPLETE_VALUE_2 =
{
	4,
	{
		{ dump_RDD_DHD_DOORBELL, 0x32b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_DOORBELL_RX_COMPLETE_VALUE_2 =
{
	4,
	{
		{ dump_RDD_DHD_DOORBELL, 0x32c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_DOORBELL_RX_POST_VALUE_2 =
{
	4,
	{
		{ dump_RDD_DHD_DOORBELL, 0x32d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_DISPATCHER_CREDIT_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x32e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SPDSVC_WLAN_GEN_PARAMS_TABLE_2 =
{
	10,
	{
		{ dump_RDD_SPDSVC_WLAN_GEN_PARAMS, 0x32f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_TX_SYNC_FIFO_TABLE_2 =
{
	8,
	{
		{ dump_RDD_CPU_TX_SYNC_FIFO_ENTRY, 0x3300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_DESCRIPTOR_TABLE_2 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x3310 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE_2 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0x3320 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_INTERRUPT_SCRATCH_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3330 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT EMAC_FLOW_CTRL_BUDGET_REMAINDER_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3338 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_2 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, 0x3340 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_2 =
{
	12,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, 0x3350 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT XGPON_REPORT_ZERO_SENT_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3360 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE_2 =
{
	8,
	{
		{ dump_RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY, 0x3370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_TX_RING_INDICES_VALUES_TABLE_2 =
{
	4,
	{
		{ dump_RDD_CPU_TX_RING_INDICES, 0x3378 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT BBH_TX_EGRESS_REPORT_COUNTER_TABLE_2 =
{
	8,
	{
		{ dump_RDD_BBH_TX_EGRESS_COUNTER_ENTRY, 0x3380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_SHADOW_RD_IDX_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3388 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_SHADOW_WR_IDX_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3390 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT US_TM_PD_FIFO_TABLE_3 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT COMPLEX_SCHEDULER_TABLE_3 =
{
	64,
	{
		{ dump_RDD_COMPLEX_SCHEDULER_DESCRIPTOR, 0x1400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT US_TM_SCHEDULING_QUEUE_TABLE_3 =
{
	8,
	{
		{ dump_RDD_SCHEDULING_QUEUE_DESCRIPTOR, 0x1800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DBG_DUMP_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1d00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DIRECT_PROCESSING_RX_MIRRORING_SCRATCHPAD_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1f00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT US_TM_WAN_0_BBH_TX_WAKE_UP_DATA_TABLE_3 =
{
	8,
	{
		{ dump_RDD_BBH_TX_WAKE_UP_DATA_ENTRY, 0x1f88 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DIRECT_PROCESSING_PD_TABLE_3 =
{
	16,
	{
		{ dump_RDD_PROCESSING_RX_DESCRIPTOR, 0x1f90 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1fa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT US_TM_WAN_0_BBH_TX_EGRESS_COUNTER_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1fc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT BBH_TX_EPON_WAKE_UP_DATA_TABLE_3 =
{
	8,
	{
		{ dump_RDD_BBH_TX_WAKE_UP_DATA_ENTRY, 0x1fe8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT OVERALL_RATE_LIMITER_TABLE_3 =
{
	16,
	{
		{ dump_RDD_OVERALL_RATE_LIMITER_DESCRIPTOR, 0x1ff0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT BASIC_RATE_LIMITER_TABLE_US_3 =
{
	16,
	{
		{ dump_RDD_BASIC_RATE_LIMITER_DESCRIPTOR, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VLAN_TX_COUNTERS_3 =
{
	8,
	{
		{ dump_RDD_PACKETS_AND_BYTES, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_3 =
{
	8,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x3008 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT US_TM_FLUSH_DISPATCHER_CREDIT_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3010 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TM_HW_FLUSH_3 =
{
	4,
	{
		{ dump_RDD_HW_FLUSH_ENTRY, 0x301c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DIRECT_PROCESSING_FLOW_CNTR_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3020 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT BBH_TX_EPON_INGRESS_COUNTER_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3040 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MIRRORING_SCRATCH_3 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x3068 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DISPATCHER_CREDIT_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3070 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT ZERO_VALUE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x307c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT US_TM_TM_FLOW_CNTR_TABLE_3 =
{
	1,
	{
		{ dump_RDD_TM_FLOW_CNTR_ENTRY, 0x3080 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_TBL_CFG_3 =
{
	24,
	{
		{ dump_RDD_NATC_TBL_CONFIGURATION, 0x3100 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT US_TM_CPU_TX_ABS_COUNTERS_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3148 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT ONE_VALUE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x31c8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DIRECT_PROCESSING_PAUSE_QUANTA_3 =
{
	2,
	{
		{ dump_RDD_PAUSE_QUANTA_ENTRY, 0x31cc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT BUDGET_ALLOCATION_TIMER_VALUE_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x31ce },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_DISPATCHER_CREDIT_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x31d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT US_TM_BBH_TX_WAN_0_FIFO_BYTES_USED_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x31dc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DIRECT_PROCESSING_EPON_CONTROL_SCRATCH_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x31e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT US_TM_WAN_0_BB_DESTINATION_TABLE_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x31f6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TASK_IDX_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x31f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT US_TM_SCHEDULING_GLOBAL_FLUSH_CFG_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x31fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT US_TM_TX_PAUSE_NACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x31fd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT BBH_TX_US_WAN_0_FIFO_BYTES_THRESHOLD_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x31fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT US_TM_BBH_QUEUE_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BBH_QUEUE_DESCRIPTOR, 0x3200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x32a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT US_TM_SCHEDULING_FLUSH_VECTOR_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3320 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SCHEDULING_FLUSH_GLOBAL_CFG_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3334 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TM_VLAN_STATS_ENABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3335 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TX_EXCEPTION_3 =
{
	1,
	{
		{ dump_RDD_TX_EXCEPTION_ENTRY, 0x3336 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3337 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_3 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x3338 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_3 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x333a },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT US_TM_FLUSH_AGGREGATION_TASK_DISABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x333c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x333d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RATE_LIMIT_OVERHEAD_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x333e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MAC_TYPE_3 =
{
	1,
	{
		{ dump_RDD_MAC_TYPE_ENTRY, 0x333f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT US_TM_TM_ACTION_PTR_TABLE_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3340 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT WAN_0_BBH_TX_FIFO_SIZE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3362 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT US_TM_FIRST_QUEUE_MAPPING_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3363 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3364 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3365 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_3 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, 0x3370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT BBH_TX_EPON_EGRESS_COUNTER_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT US_TM_SCHEDULING_QUEUE_AGING_VECTOR_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x33a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT UPDATE_FIFO_TABLE_3 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x33c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT US_TM_SCHEDULING_AGGREGATION_CONTEXT_VECTOR_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x33e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT BASIC_SCHEDULER_TABLE_US_3 =
{
	16,
	{
		{ dump_RDD_BASIC_SCHEDULER_DESCRIPTOR, 0x3400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3610 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT EPON_UPDATE_FIFO_TABLE_3 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x3640 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT US_TM_TX_OCTETS_COUNTERS_TABLE_3 =
{
	8,
	{
		{ dump_RDD_PACKETS_AND_BYTES, 0x3660 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RATE_LIMITER_VALID_TABLE_US_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x36a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_3 =
{
	12,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, 0x36b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_3 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, 0x3800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DS_PACKET_BUFFER_4 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_4 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_4 =
{
	8,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, 0x1280 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DSCP_TO_PBITS_MAP_TABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TCAM_IC_CMD_TABLE_4 =
{
	4,
	{
		{ dump_RDD_TCAM_IC_CMD, 0x1400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_PROCESSING_TASKS_REPLY_4 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x1640 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PBIT_TO_GEM_TABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_4 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, 0x1700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_PROCESSING_TASKS_LOCK_4 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x17c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TC_TO_QUEUE_TABLE_4 =
{
	8,
	{
		{ dump_RDD_TC_TO_QUEUE_8, 0x1800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_FPM_TOKENS_READ_FLAG_4 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x1a08 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_TO_DSCP_TO_PBITS_TABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1a10 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1a20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_BBH_OVERRUN_TASKS_LIMIT_4 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x1a40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT LAYER2_HEADER_COPY_MAPPING_TABLE_4 =
{
	4,
	{
		{ dump_RDD_LAYER2_HEADER_COPY_MAPPING_ENTRY, 0x1a80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_4 =
{
	8,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x1af8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT EMAC_FLOW_CTRL_4 =
{
	16,
	{
		{ dump_RDD_EMAC_FLOW_CTRL_ENTRY, 0x1b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_4 =
{
	1,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, 0x1b80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_4 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0x1bd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FLOODING_HEADER_FPM_POOL_NUMBER_MAPPING_TABLE_4 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0x1be0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PBIT_TO_QUEUE_TABLE_4 =
{
	8,
	{
		{ dump_RDD_PBIT_TO_QUEUE_8, 0x1c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1e08 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1e30 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT GLOBAL_DSCP_TO_PBITS_TABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1e40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_4 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, 0x1e80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PORT_MAC_4 =
{
	8,
	{
		{ dump_RDD_MAC_ADDRESS, 0x1ec0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_4 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, 0x1f00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DUAL_STACK_LITE_TABLE_4 =
{
	64,
	{
		{ dump_RDD_DUAL_STACK_LITE_ENTRY, 0x1f80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT QUEUE_THRESHOLD_VECTOR_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1fc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_PACKET_BASED_MAPPING_4 =
{
	4,
	{
		{ dump_RDD_INGRESS_PACKET_BASED_MAPPING_ENTRY, 0x1fe4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PROCESSING_QUEUE_DYNAMIC_MNG_TABLE_4 =
{
	8,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x1fe8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SPDSVC_ANALYZER_PARAMS_TABLE_4 =
{
	16,
	{
		{ dump_RDD_SPDSVC_ANALYZER_PARAMS, 0x1ff0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_4 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DBG_DUMP_TABLE_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_TBL_CFG_4 =
{
	24,
	{
		{ dump_RDD_NATC_TBL_CONFIGURATION, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_COUNTERS_GENERIC_CONTEXT_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2448 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_FLOW_CONTEXT_DDR_ADDR_4 =
{
	8,
	{
		{ dump_RDD_DEF_FLOW_CONTEXT_DDR_ADDR, 0x24c8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT BRIDGE_CFG_TABLE_4 =
{
	8,
	{
		{ dump_RDD_BRIDGE_CFG, 0x24d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_PBIT_TO_DISCARD_PRIO_VECTOR_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x24e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x24f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MAX_PKT_LEN_TABLE_4 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2540 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TCAM_IC_CFG_TABLE_4 =
{
	16,
	{
		{ dump_RDD_TCAM_IC_CFG, 0x25c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VLAN_ACTION_GPE_HANDLER_PTR_TABLE_4 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2640 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_BBH_OVERRUN_MAX_TASKS_LIMIT_4 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2662 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_4 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, 0x2664 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NULL_BUFFER_4 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x2668 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPTV_CONFIGURATION_TABLE_4 =
{
	12,
	{
		{ dump_RDD_HW_IPTV_CONFIGURATION, 0x2670 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT ZERO_VALUE_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x267c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TUNNEL_TABLE_4 =
{
	16,
	{
		{ dump_RDD_TUNNEL_ENTRY, 0x2680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_DESCRIPTOR_TABLE_4 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x26a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_4 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, 0x26b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_4 =
{
	12,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, 0x26c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT ONE_VALUE_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x26cc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_4 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x26d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_BBH_OVERRUN_MIN_TASKS_LIMIT_4 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x26d6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_4 =
{
	5,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x26d8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x26dd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x26de },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x26df },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TUNNEL_HEADER_PSRAM_BUFFER_4 =
{
	4,
	{
		{ dump_RDD_TUNNEL_HEADER_PSRAM_BUFFER, 0x26e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TASK_IDX_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x26e4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_4 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, 0x26e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_4 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, 0x26f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_4 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x26f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEFAULT_VLAN_VID_TABLE_4 =
{
	2,
	{
		{ dump_RDD_DEFAULT_VLAN_VID_ENTRY, 0x2700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_4 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, 0x2720 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2727 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2728 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_4 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x272c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RATE_LIMIT_OVERHEAD_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x272e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_QOS_DONT_DROP_RATIO_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x272f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_4 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2730 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_QOS_WAN_UNTAGGED_PRIORITY_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2732 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IC_MCAST_ENABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2733 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT ECN_IPV6_REMARK_TABLE_4 =
{
	1,
	{
		{ dump_RDD_ECN_IPV6_REMARK_ENTRY, 0x2734 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_REDIRECT_MODE_4 =
{
	1,
	{
		{ dump_RDD_CPU_REDIRECT_MODE_ENTRY, 0x2735 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2736 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_4 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x2737 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_4 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, 0x2738 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x273a },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x273b },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2740 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEFAULT_VLAN_VID_TO_TX_VLAN_CNTR_TABLE_4 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DS_PACKET_BUFFER_5 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_5 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_5 =
{
	8,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, 0x1280 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DSCP_TO_PBITS_MAP_TABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TCAM_IC_CMD_TABLE_5 =
{
	4,
	{
		{ dump_RDD_TCAM_IC_CMD, 0x1400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_PROCESSING_TASKS_REPLY_5 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x1640 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PBIT_TO_GEM_TABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_5 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, 0x1700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_PROCESSING_TASKS_LOCK_5 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x17c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TC_TO_QUEUE_TABLE_5 =
{
	8,
	{
		{ dump_RDD_TC_TO_QUEUE_8, 0x1800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_FPM_TOKENS_READ_FLAG_5 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x1a08 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_TO_DSCP_TO_PBITS_TABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1a10 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1a20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_BBH_OVERRUN_TASKS_LIMIT_5 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x1a40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT LAYER2_HEADER_COPY_MAPPING_TABLE_5 =
{
	4,
	{
		{ dump_RDD_LAYER2_HEADER_COPY_MAPPING_ENTRY, 0x1a80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_5 =
{
	8,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x1af8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT EMAC_FLOW_CTRL_5 =
{
	16,
	{
		{ dump_RDD_EMAC_FLOW_CTRL_ENTRY, 0x1b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_5 =
{
	1,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, 0x1b80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_5 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0x1bd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FLOODING_HEADER_FPM_POOL_NUMBER_MAPPING_TABLE_5 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0x1be0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PBIT_TO_QUEUE_TABLE_5 =
{
	8,
	{
		{ dump_RDD_PBIT_TO_QUEUE_8, 0x1c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1e08 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1e30 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT GLOBAL_DSCP_TO_PBITS_TABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1e40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_5 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, 0x1e80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PORT_MAC_5 =
{
	8,
	{
		{ dump_RDD_MAC_ADDRESS, 0x1ec0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_5 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, 0x1f00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DUAL_STACK_LITE_TABLE_5 =
{
	64,
	{
		{ dump_RDD_DUAL_STACK_LITE_ENTRY, 0x1f80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT QUEUE_THRESHOLD_VECTOR_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1fc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_PACKET_BASED_MAPPING_5 =
{
	4,
	{
		{ dump_RDD_INGRESS_PACKET_BASED_MAPPING_ENTRY, 0x1fe4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PROCESSING_QUEUE_DYNAMIC_MNG_TABLE_5 =
{
	8,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x1fe8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SPDSVC_ANALYZER_PARAMS_TABLE_5 =
{
	16,
	{
		{ dump_RDD_SPDSVC_ANALYZER_PARAMS, 0x1ff0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_5 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DBG_DUMP_TABLE_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_TBL_CFG_5 =
{
	24,
	{
		{ dump_RDD_NATC_TBL_CONFIGURATION, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_COUNTERS_GENERIC_CONTEXT_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2448 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_FLOW_CONTEXT_DDR_ADDR_5 =
{
	8,
	{
		{ dump_RDD_DEF_FLOW_CONTEXT_DDR_ADDR, 0x24c8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT BRIDGE_CFG_TABLE_5 =
{
	8,
	{
		{ dump_RDD_BRIDGE_CFG, 0x24d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_PBIT_TO_DISCARD_PRIO_VECTOR_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x24e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x24f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MAX_PKT_LEN_TABLE_5 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2540 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TCAM_IC_CFG_TABLE_5 =
{
	16,
	{
		{ dump_RDD_TCAM_IC_CFG, 0x25c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VLAN_ACTION_GPE_HANDLER_PTR_TABLE_5 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2640 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_BBH_OVERRUN_MAX_TASKS_LIMIT_5 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2662 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_5 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, 0x2664 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NULL_BUFFER_5 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x2668 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPTV_CONFIGURATION_TABLE_5 =
{
	12,
	{
		{ dump_RDD_HW_IPTV_CONFIGURATION, 0x2670 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT ZERO_VALUE_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x267c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TUNNEL_TABLE_5 =
{
	16,
	{
		{ dump_RDD_TUNNEL_ENTRY, 0x2680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_DESCRIPTOR_TABLE_5 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x26a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_5 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, 0x26b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_5 =
{
	12,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, 0x26c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT ONE_VALUE_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x26cc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_5 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x26d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_BBH_OVERRUN_MIN_TASKS_LIMIT_5 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x26d6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_5 =
{
	5,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x26d8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x26dd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x26de },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x26df },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TUNNEL_HEADER_PSRAM_BUFFER_5 =
{
	4,
	{
		{ dump_RDD_TUNNEL_HEADER_PSRAM_BUFFER, 0x26e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TASK_IDX_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x26e4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_5 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, 0x26e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_5 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, 0x26f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_5 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x26f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEFAULT_VLAN_VID_TABLE_5 =
{
	2,
	{
		{ dump_RDD_DEFAULT_VLAN_VID_ENTRY, 0x2700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_5 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, 0x2720 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2727 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2728 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_5 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x272c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RATE_LIMIT_OVERHEAD_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x272e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_QOS_DONT_DROP_RATIO_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x272f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_5 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2730 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_QOS_WAN_UNTAGGED_PRIORITY_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2732 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IC_MCAST_ENABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2733 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT ECN_IPV6_REMARK_TABLE_5 =
{
	1,
	{
		{ dump_RDD_ECN_IPV6_REMARK_ENTRY, 0x2734 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_REDIRECT_MODE_5 =
{
	1,
	{
		{ dump_RDD_CPU_REDIRECT_MODE_ENTRY, 0x2735 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2736 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_5 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x2737 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_5 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, 0x2738 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x273a },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x273b },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2740 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEFAULT_VLAN_VID_TO_TX_VLAN_CNTR_TABLE_5 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DS_PACKET_BUFFER_6 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_6 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_6 =
{
	8,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, 0x1280 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DSCP_TO_PBITS_MAP_TABLE_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TCAM_IC_CMD_TABLE_6 =
{
	4,
	{
		{ dump_RDD_TCAM_IC_CMD, 0x1400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_PROCESSING_TASKS_REPLY_6 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x1640 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PBIT_TO_GEM_TABLE_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_6 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, 0x1700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_PROCESSING_TASKS_LOCK_6 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x17c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TC_TO_QUEUE_TABLE_6 =
{
	8,
	{
		{ dump_RDD_TC_TO_QUEUE_8, 0x1800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_FPM_TOKENS_READ_FLAG_6 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x1a08 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_TO_DSCP_TO_PBITS_TABLE_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1a10 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1a20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_BBH_OVERRUN_TASKS_LIMIT_6 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x1a40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT LAYER2_HEADER_COPY_MAPPING_TABLE_6 =
{
	4,
	{
		{ dump_RDD_LAYER2_HEADER_COPY_MAPPING_ENTRY, 0x1a80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_6 =
{
	8,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x1af8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT EMAC_FLOW_CTRL_6 =
{
	16,
	{
		{ dump_RDD_EMAC_FLOW_CTRL_ENTRY, 0x1b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_6 =
{
	1,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, 0x1b80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_6 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0x1bd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FLOODING_HEADER_FPM_POOL_NUMBER_MAPPING_TABLE_6 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0x1be0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PBIT_TO_QUEUE_TABLE_6 =
{
	8,
	{
		{ dump_RDD_PBIT_TO_QUEUE_8, 0x1c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1e08 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1e30 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT GLOBAL_DSCP_TO_PBITS_TABLE_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1e40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_6 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, 0x1e80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PORT_MAC_6 =
{
	8,
	{
		{ dump_RDD_MAC_ADDRESS, 0x1ec0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_6 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, 0x1f00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DUAL_STACK_LITE_TABLE_6 =
{
	64,
	{
		{ dump_RDD_DUAL_STACK_LITE_ENTRY, 0x1f80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT QUEUE_THRESHOLD_VECTOR_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1fc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_PACKET_BASED_MAPPING_6 =
{
	4,
	{
		{ dump_RDD_INGRESS_PACKET_BASED_MAPPING_ENTRY, 0x1fe4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PROCESSING_QUEUE_DYNAMIC_MNG_TABLE_6 =
{
	8,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x1fe8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SPDSVC_ANALYZER_PARAMS_TABLE_6 =
{
	16,
	{
		{ dump_RDD_SPDSVC_ANALYZER_PARAMS, 0x1ff0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_6 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DBG_DUMP_TABLE_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_TBL_CFG_6 =
{
	24,
	{
		{ dump_RDD_NATC_TBL_CONFIGURATION, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_COUNTERS_GENERIC_CONTEXT_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2448 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_FLOW_CONTEXT_DDR_ADDR_6 =
{
	8,
	{
		{ dump_RDD_DEF_FLOW_CONTEXT_DDR_ADDR, 0x24c8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT BRIDGE_CFG_TABLE_6 =
{
	8,
	{
		{ dump_RDD_BRIDGE_CFG, 0x24d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_PBIT_TO_DISCARD_PRIO_VECTOR_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x24e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x24f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MAX_PKT_LEN_TABLE_6 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2540 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TCAM_IC_CFG_TABLE_6 =
{
	16,
	{
		{ dump_RDD_TCAM_IC_CFG, 0x25c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VLAN_ACTION_GPE_HANDLER_PTR_TABLE_6 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2640 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_BBH_OVERRUN_MAX_TASKS_LIMIT_6 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2662 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_6 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, 0x2664 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NULL_BUFFER_6 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x2668 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPTV_CONFIGURATION_TABLE_6 =
{
	12,
	{
		{ dump_RDD_HW_IPTV_CONFIGURATION, 0x2670 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT ZERO_VALUE_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x267c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TUNNEL_TABLE_6 =
{
	16,
	{
		{ dump_RDD_TUNNEL_ENTRY, 0x2680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_DESCRIPTOR_TABLE_6 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x26a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_6 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, 0x26b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_6 =
{
	12,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, 0x26c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT ONE_VALUE_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x26cc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_6 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x26d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_BBH_OVERRUN_MIN_TASKS_LIMIT_6 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x26d6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_6 =
{
	5,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x26d8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x26dd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x26de },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x26df },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TUNNEL_HEADER_PSRAM_BUFFER_6 =
{
	4,
	{
		{ dump_RDD_TUNNEL_HEADER_PSRAM_BUFFER, 0x26e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TASK_IDX_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x26e4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_6 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, 0x26e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_6 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, 0x26f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_6 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x26f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEFAULT_VLAN_VID_TABLE_6 =
{
	2,
	{
		{ dump_RDD_DEFAULT_VLAN_VID_ENTRY, 0x2700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_6 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, 0x2720 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2727 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2728 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_6 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x272c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RATE_LIMIT_OVERHEAD_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x272e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_QOS_DONT_DROP_RATIO_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x272f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_6 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2730 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_QOS_WAN_UNTAGGED_PRIORITY_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2732 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IC_MCAST_ENABLE_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2733 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT ECN_IPV6_REMARK_TABLE_6 =
{
	1,
	{
		{ dump_RDD_ECN_IPV6_REMARK_ENTRY, 0x2734 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_REDIRECT_MODE_6 =
{
	1,
	{
		{ dump_RDD_CPU_REDIRECT_MODE_ENTRY, 0x2735 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2736 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_6 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x2737 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_6 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, 0x2738 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x273a },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x273b },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2740 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEFAULT_VLAN_VID_TO_TX_VLAN_CNTR_TABLE_6 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DS_PACKET_BUFFER_7 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_7 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_7 =
{
	8,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, 0x1280 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DSCP_TO_PBITS_MAP_TABLE_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TCAM_IC_CMD_TABLE_7 =
{
	4,
	{
		{ dump_RDD_TCAM_IC_CMD, 0x1400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_PROCESSING_TASKS_REPLY_7 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x1640 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PBIT_TO_GEM_TABLE_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_7 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, 0x1700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_PROCESSING_TASKS_LOCK_7 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x17c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TC_TO_QUEUE_TABLE_7 =
{
	8,
	{
		{ dump_RDD_TC_TO_QUEUE_8, 0x1800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_FPM_TOKENS_READ_FLAG_7 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x1a08 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_TO_DSCP_TO_PBITS_TABLE_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1a10 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1a20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_BBH_OVERRUN_TASKS_LIMIT_7 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x1a40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT LAYER2_HEADER_COPY_MAPPING_TABLE_7 =
{
	4,
	{
		{ dump_RDD_LAYER2_HEADER_COPY_MAPPING_ENTRY, 0x1a80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_7 =
{
	8,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x1af8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT EMAC_FLOW_CTRL_7 =
{
	16,
	{
		{ dump_RDD_EMAC_FLOW_CTRL_ENTRY, 0x1b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_7 =
{
	1,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, 0x1b80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_7 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0x1bd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FLOODING_HEADER_FPM_POOL_NUMBER_MAPPING_TABLE_7 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0x1be0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PBIT_TO_QUEUE_TABLE_7 =
{
	8,
	{
		{ dump_RDD_PBIT_TO_QUEUE_8, 0x1c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1e08 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1e30 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT GLOBAL_DSCP_TO_PBITS_TABLE_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1e40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_7 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, 0x1e80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PORT_MAC_7 =
{
	8,
	{
		{ dump_RDD_MAC_ADDRESS, 0x1ec0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_7 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, 0x1f00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DUAL_STACK_LITE_TABLE_7 =
{
	64,
	{
		{ dump_RDD_DUAL_STACK_LITE_ENTRY, 0x1f80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT QUEUE_THRESHOLD_VECTOR_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1fc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_PACKET_BASED_MAPPING_7 =
{
	4,
	{
		{ dump_RDD_INGRESS_PACKET_BASED_MAPPING_ENTRY, 0x1fe4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PROCESSING_QUEUE_DYNAMIC_MNG_TABLE_7 =
{
	8,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x1fe8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SPDSVC_ANALYZER_PARAMS_TABLE_7 =
{
	16,
	{
		{ dump_RDD_SPDSVC_ANALYZER_PARAMS, 0x1ff0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_7 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DBG_DUMP_TABLE_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_TBL_CFG_7 =
{
	24,
	{
		{ dump_RDD_NATC_TBL_CONFIGURATION, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_COUNTERS_GENERIC_CONTEXT_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2448 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_FLOW_CONTEXT_DDR_ADDR_7 =
{
	8,
	{
		{ dump_RDD_DEF_FLOW_CONTEXT_DDR_ADDR, 0x24c8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT BRIDGE_CFG_TABLE_7 =
{
	8,
	{
		{ dump_RDD_BRIDGE_CFG, 0x24d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_PBIT_TO_DISCARD_PRIO_VECTOR_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x24e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x24f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MAX_PKT_LEN_TABLE_7 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2540 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TCAM_IC_CFG_TABLE_7 =
{
	16,
	{
		{ dump_RDD_TCAM_IC_CFG, 0x25c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VLAN_ACTION_GPE_HANDLER_PTR_TABLE_7 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2640 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_BBH_OVERRUN_MAX_TASKS_LIMIT_7 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2662 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_7 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, 0x2664 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NULL_BUFFER_7 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x2668 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPTV_CONFIGURATION_TABLE_7 =
{
	12,
	{
		{ dump_RDD_HW_IPTV_CONFIGURATION, 0x2670 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT ZERO_VALUE_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x267c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TUNNEL_TABLE_7 =
{
	16,
	{
		{ dump_RDD_TUNNEL_ENTRY, 0x2680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_DESCRIPTOR_TABLE_7 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x26a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_7 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, 0x26b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_7 =
{
	12,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, 0x26c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT ONE_VALUE_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x26cc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_7 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x26d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_BBH_OVERRUN_MIN_TASKS_LIMIT_7 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x26d6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_7 =
{
	5,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x26d8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x26dd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x26de },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x26df },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TUNNEL_HEADER_PSRAM_BUFFER_7 =
{
	4,
	{
		{ dump_RDD_TUNNEL_HEADER_PSRAM_BUFFER, 0x26e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TASK_IDX_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x26e4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_7 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, 0x26e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_7 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, 0x26f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_7 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x26f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEFAULT_VLAN_VID_TABLE_7 =
{
	2,
	{
		{ dump_RDD_DEFAULT_VLAN_VID_ENTRY, 0x2700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_7 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, 0x2720 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2727 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2728 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_7 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x272c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RATE_LIMIT_OVERHEAD_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x272e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_QOS_DONT_DROP_RATIO_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x272f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_7 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2730 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_QOS_WAN_UNTAGGED_PRIORITY_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2732 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IC_MCAST_ENABLE_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2733 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT ECN_IPV6_REMARK_TABLE_7 =
{
	1,
	{
		{ dump_RDD_ECN_IPV6_REMARK_ENTRY, 0x2734 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_REDIRECT_MODE_7 =
{
	1,
	{
		{ dump_RDD_CPU_REDIRECT_MODE_ENTRY, 0x2735 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2736 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_7 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x2737 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_7 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, 0x2738 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x273a },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x273b },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2740 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEFAULT_VLAN_VID_TO_TX_VLAN_CNTR_TABLE_7 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DS_PACKET_BUFFER_8 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_8 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_8 =
{
	8,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, 0x1280 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DSCP_TO_PBITS_MAP_TABLE_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TCAM_IC_CMD_TABLE_8 =
{
	4,
	{
		{ dump_RDD_TCAM_IC_CMD, 0x1400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_PROCESSING_TASKS_REPLY_8 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x1640 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PBIT_TO_GEM_TABLE_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_8 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, 0x1700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_PROCESSING_TASKS_LOCK_8 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x17c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TC_TO_QUEUE_TABLE_8 =
{
	8,
	{
		{ dump_RDD_TC_TO_QUEUE_8, 0x1800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_FPM_TOKENS_READ_FLAG_8 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x1a08 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_TO_DSCP_TO_PBITS_TABLE_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1a10 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_8 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1a20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_BBH_OVERRUN_TASKS_LIMIT_8 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x1a40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT LAYER2_HEADER_COPY_MAPPING_TABLE_8 =
{
	4,
	{
		{ dump_RDD_LAYER2_HEADER_COPY_MAPPING_ENTRY, 0x1a80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_8 =
{
	8,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x1af8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT EMAC_FLOW_CTRL_8 =
{
	16,
	{
		{ dump_RDD_EMAC_FLOW_CTRL_ENTRY, 0x1b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_8 =
{
	1,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, 0x1b80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_8 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0x1bd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FLOODING_HEADER_FPM_POOL_NUMBER_MAPPING_TABLE_8 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0x1be0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PBIT_TO_QUEUE_TABLE_8 =
{
	8,
	{
		{ dump_RDD_PBIT_TO_QUEUE_8, 0x1c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_8 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1e08 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1e30 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT GLOBAL_DSCP_TO_PBITS_TABLE_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1e40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_8 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, 0x1e80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PORT_MAC_8 =
{
	8,
	{
		{ dump_RDD_MAC_ADDRESS, 0x1ec0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_8 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, 0x1f00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DUAL_STACK_LITE_TABLE_8 =
{
	64,
	{
		{ dump_RDD_DUAL_STACK_LITE_ENTRY, 0x1f80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT QUEUE_THRESHOLD_VECTOR_8 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1fc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_PACKET_BASED_MAPPING_8 =
{
	4,
	{
		{ dump_RDD_INGRESS_PACKET_BASED_MAPPING_ENTRY, 0x1fe4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PROCESSING_QUEUE_DYNAMIC_MNG_TABLE_8 =
{
	8,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x1fe8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SPDSVC_ANALYZER_PARAMS_TABLE_8 =
{
	16,
	{
		{ dump_RDD_SPDSVC_ANALYZER_PARAMS, 0x1ff0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_8 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DBG_DUMP_TABLE_8 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_TBL_CFG_8 =
{
	24,
	{
		{ dump_RDD_NATC_TBL_CONFIGURATION, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_COUNTERS_GENERIC_CONTEXT_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2448 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_FLOW_CONTEXT_DDR_ADDR_8 =
{
	8,
	{
		{ dump_RDD_DEF_FLOW_CONTEXT_DDR_ADDR, 0x24c8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT BRIDGE_CFG_TABLE_8 =
{
	8,
	{
		{ dump_RDD_BRIDGE_CFG, 0x24d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_PBIT_TO_DISCARD_PRIO_VECTOR_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x24e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x24f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MAX_PKT_LEN_TABLE_8 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_8 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2540 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TCAM_IC_CFG_TABLE_8 =
{
	16,
	{
		{ dump_RDD_TCAM_IC_CFG, 0x25c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VLAN_ACTION_GPE_HANDLER_PTR_TABLE_8 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2640 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_BBH_OVERRUN_MAX_TASKS_LIMIT_8 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2662 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_8 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, 0x2664 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NULL_BUFFER_8 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x2668 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPTV_CONFIGURATION_TABLE_8 =
{
	12,
	{
		{ dump_RDD_HW_IPTV_CONFIGURATION, 0x2670 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT ZERO_VALUE_8 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x267c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TUNNEL_TABLE_8 =
{
	16,
	{
		{ dump_RDD_TUNNEL_ENTRY, 0x2680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_DESCRIPTOR_TABLE_8 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x26a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_8 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, 0x26b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_8 =
{
	12,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, 0x26c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT ONE_VALUE_8 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x26cc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_8 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x26d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_BBH_OVERRUN_MIN_TASKS_LIMIT_8 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x26d6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_8 =
{
	5,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x26d8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x26dd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x26de },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x26df },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TUNNEL_HEADER_PSRAM_BUFFER_8 =
{
	4,
	{
		{ dump_RDD_TUNNEL_HEADER_PSRAM_BUFFER, 0x26e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TASK_IDX_8 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x26e4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_8 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, 0x26e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_8 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, 0x26f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_8 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x26f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEFAULT_VLAN_VID_TABLE_8 =
{
	2,
	{
		{ dump_RDD_DEFAULT_VLAN_VID_ENTRY, 0x2700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_8 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, 0x2720 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2727 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_8 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2728 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_8 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x272c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RATE_LIMIT_OVERHEAD_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x272e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_QOS_DONT_DROP_RATIO_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x272f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_8 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2730 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_QOS_WAN_UNTAGGED_PRIORITY_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2732 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IC_MCAST_ENABLE_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2733 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT ECN_IPV6_REMARK_TABLE_8 =
{
	1,
	{
		{ dump_RDD_ECN_IPV6_REMARK_ENTRY, 0x2734 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_REDIRECT_MODE_8 =
{
	1,
	{
		{ dump_RDD_CPU_REDIRECT_MODE_ENTRY, 0x2735 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2736 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_8 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x2737 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_8 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, 0x2738 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x273a },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x273b },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2740 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEFAULT_VLAN_VID_TO_TX_VLAN_CNTR_TABLE_8 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DS_PACKET_BUFFER_9 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_9 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_9 =
{
	8,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, 0x1280 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DSCP_TO_PBITS_MAP_TABLE_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TCAM_IC_CMD_TABLE_9 =
{
	4,
	{
		{ dump_RDD_TCAM_IC_CMD, 0x1400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_PROCESSING_TASKS_REPLY_9 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x1640 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PBIT_TO_GEM_TABLE_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_9 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, 0x1700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_PROCESSING_TASKS_LOCK_9 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x17c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TC_TO_QUEUE_TABLE_9 =
{
	8,
	{
		{ dump_RDD_TC_TO_QUEUE_8, 0x1800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_FPM_TOKENS_READ_FLAG_9 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x1a08 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_TO_DSCP_TO_PBITS_TABLE_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1a10 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_9 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1a20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_BBH_OVERRUN_TASKS_LIMIT_9 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x1a40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT LAYER2_HEADER_COPY_MAPPING_TABLE_9 =
{
	4,
	{
		{ dump_RDD_LAYER2_HEADER_COPY_MAPPING_ENTRY, 0x1a80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_9 =
{
	8,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x1af8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT EMAC_FLOW_CTRL_9 =
{
	16,
	{
		{ dump_RDD_EMAC_FLOW_CTRL_ENTRY, 0x1b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_9 =
{
	1,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, 0x1b80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_9 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0x1bd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FLOODING_HEADER_FPM_POOL_NUMBER_MAPPING_TABLE_9 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0x1be0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PBIT_TO_QUEUE_TABLE_9 =
{
	8,
	{
		{ dump_RDD_PBIT_TO_QUEUE_8, 0x1c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_9 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1e08 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1e30 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT GLOBAL_DSCP_TO_PBITS_TABLE_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1e40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_9 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, 0x1e80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PORT_MAC_9 =
{
	8,
	{
		{ dump_RDD_MAC_ADDRESS, 0x1ec0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_9 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, 0x1f00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DUAL_STACK_LITE_TABLE_9 =
{
	64,
	{
		{ dump_RDD_DUAL_STACK_LITE_ENTRY, 0x1f80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT QUEUE_THRESHOLD_VECTOR_9 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1fc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_PACKET_BASED_MAPPING_9 =
{
	4,
	{
		{ dump_RDD_INGRESS_PACKET_BASED_MAPPING_ENTRY, 0x1fe4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PROCESSING_QUEUE_DYNAMIC_MNG_TABLE_9 =
{
	8,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x1fe8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SPDSVC_ANALYZER_PARAMS_TABLE_9 =
{
	16,
	{
		{ dump_RDD_SPDSVC_ANALYZER_PARAMS, 0x1ff0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_9 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DBG_DUMP_TABLE_9 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_TBL_CFG_9 =
{
	24,
	{
		{ dump_RDD_NATC_TBL_CONFIGURATION, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_COUNTERS_GENERIC_CONTEXT_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2448 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_FLOW_CONTEXT_DDR_ADDR_9 =
{
	8,
	{
		{ dump_RDD_DEF_FLOW_CONTEXT_DDR_ADDR, 0x24c8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT BRIDGE_CFG_TABLE_9 =
{
	8,
	{
		{ dump_RDD_BRIDGE_CFG, 0x24d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_PBIT_TO_DISCARD_PRIO_VECTOR_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x24e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x24f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MAX_PKT_LEN_TABLE_9 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_9 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2540 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TCAM_IC_CFG_TABLE_9 =
{
	16,
	{
		{ dump_RDD_TCAM_IC_CFG, 0x25c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VLAN_ACTION_GPE_HANDLER_PTR_TABLE_9 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2640 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_BBH_OVERRUN_MAX_TASKS_LIMIT_9 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2662 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_9 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, 0x2664 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NULL_BUFFER_9 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x2668 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPTV_CONFIGURATION_TABLE_9 =
{
	12,
	{
		{ dump_RDD_HW_IPTV_CONFIGURATION, 0x2670 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT ZERO_VALUE_9 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x267c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TUNNEL_TABLE_9 =
{
	16,
	{
		{ dump_RDD_TUNNEL_ENTRY, 0x2680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_DESCRIPTOR_TABLE_9 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x26a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_9 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, 0x26b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_9 =
{
	12,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, 0x26c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT ONE_VALUE_9 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x26cc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_9 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x26d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_BBH_OVERRUN_MIN_TASKS_LIMIT_9 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x26d6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_9 =
{
	5,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x26d8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x26dd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x26de },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x26df },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TUNNEL_HEADER_PSRAM_BUFFER_9 =
{
	4,
	{
		{ dump_RDD_TUNNEL_HEADER_PSRAM_BUFFER, 0x26e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TASK_IDX_9 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x26e4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_9 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, 0x26e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_9 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, 0x26f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_9 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x26f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEFAULT_VLAN_VID_TABLE_9 =
{
	2,
	{
		{ dump_RDD_DEFAULT_VLAN_VID_ENTRY, 0x2700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_9 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, 0x2720 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2727 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_9 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2728 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_9 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x272c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RATE_LIMIT_OVERHEAD_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x272e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_QOS_DONT_DROP_RATIO_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x272f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_9 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2730 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_QOS_WAN_UNTAGGED_PRIORITY_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2732 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IC_MCAST_ENABLE_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2733 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT ECN_IPV6_REMARK_TABLE_9 =
{
	1,
	{
		{ dump_RDD_ECN_IPV6_REMARK_ENTRY, 0x2734 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_REDIRECT_MODE_9 =
{
	1,
	{
		{ dump_RDD_CPU_REDIRECT_MODE_ENTRY, 0x2735 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2736 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_9 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x2737 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_9 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, 0x2738 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x273a },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x273b },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2740 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEFAULT_VLAN_VID_TO_TX_VLAN_CNTR_TABLE_9 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DS_PACKET_BUFFER_10 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_10 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_10 =
{
	8,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, 0x1280 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DSCP_TO_PBITS_MAP_TABLE_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TCAM_IC_CMD_TABLE_10 =
{
	4,
	{
		{ dump_RDD_TCAM_IC_CMD, 0x1400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_PROCESSING_TASKS_REPLY_10 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x1640 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PBIT_TO_GEM_TABLE_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_10 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, 0x1700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_PROCESSING_TASKS_LOCK_10 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x17c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TC_TO_QUEUE_TABLE_10 =
{
	8,
	{
		{ dump_RDD_TC_TO_QUEUE_8, 0x1800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_FPM_TOKENS_READ_FLAG_10 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x1a08 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_TO_DSCP_TO_PBITS_TABLE_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1a10 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_10 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1a20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_BBH_OVERRUN_TASKS_LIMIT_10 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x1a40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT LAYER2_HEADER_COPY_MAPPING_TABLE_10 =
{
	4,
	{
		{ dump_RDD_LAYER2_HEADER_COPY_MAPPING_ENTRY, 0x1a80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_10 =
{
	8,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x1af8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT EMAC_FLOW_CTRL_10 =
{
	16,
	{
		{ dump_RDD_EMAC_FLOW_CTRL_ENTRY, 0x1b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_10 =
{
	1,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, 0x1b80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_10 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0x1bd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FLOODING_HEADER_FPM_POOL_NUMBER_MAPPING_TABLE_10 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0x1be0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PBIT_TO_QUEUE_TABLE_10 =
{
	8,
	{
		{ dump_RDD_PBIT_TO_QUEUE_8, 0x1c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_10 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1e08 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1e30 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT GLOBAL_DSCP_TO_PBITS_TABLE_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1e40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_10 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, 0x1e80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PORT_MAC_10 =
{
	8,
	{
		{ dump_RDD_MAC_ADDRESS, 0x1ec0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_10 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, 0x1f00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DUAL_STACK_LITE_TABLE_10 =
{
	64,
	{
		{ dump_RDD_DUAL_STACK_LITE_ENTRY, 0x1f80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT QUEUE_THRESHOLD_VECTOR_10 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1fc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_PACKET_BASED_MAPPING_10 =
{
	4,
	{
		{ dump_RDD_INGRESS_PACKET_BASED_MAPPING_ENTRY, 0x1fe4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PROCESSING_QUEUE_DYNAMIC_MNG_TABLE_10 =
{
	8,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x1fe8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SPDSVC_ANALYZER_PARAMS_TABLE_10 =
{
	16,
	{
		{ dump_RDD_SPDSVC_ANALYZER_PARAMS, 0x1ff0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_10 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DBG_DUMP_TABLE_10 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_TBL_CFG_10 =
{
	24,
	{
		{ dump_RDD_NATC_TBL_CONFIGURATION, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_COUNTERS_GENERIC_CONTEXT_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2448 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_FLOW_CONTEXT_DDR_ADDR_10 =
{
	8,
	{
		{ dump_RDD_DEF_FLOW_CONTEXT_DDR_ADDR, 0x24c8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT BRIDGE_CFG_TABLE_10 =
{
	8,
	{
		{ dump_RDD_BRIDGE_CFG, 0x24d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_PBIT_TO_DISCARD_PRIO_VECTOR_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x24e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x24f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MAX_PKT_LEN_TABLE_10 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_10 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2540 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TCAM_IC_CFG_TABLE_10 =
{
	16,
	{
		{ dump_RDD_TCAM_IC_CFG, 0x25c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VLAN_ACTION_GPE_HANDLER_PTR_TABLE_10 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2640 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_BBH_OVERRUN_MAX_TASKS_LIMIT_10 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2662 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_10 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, 0x2664 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NULL_BUFFER_10 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x2668 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPTV_CONFIGURATION_TABLE_10 =
{
	12,
	{
		{ dump_RDD_HW_IPTV_CONFIGURATION, 0x2670 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT ZERO_VALUE_10 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x267c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TUNNEL_TABLE_10 =
{
	16,
	{
		{ dump_RDD_TUNNEL_ENTRY, 0x2680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_DESCRIPTOR_TABLE_10 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x26a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_10 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, 0x26b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_10 =
{
	12,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, 0x26c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT ONE_VALUE_10 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x26cc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_10 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x26d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_BBH_OVERRUN_MIN_TASKS_LIMIT_10 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x26d6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_10 =
{
	5,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x26d8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x26dd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x26de },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x26df },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TUNNEL_HEADER_PSRAM_BUFFER_10 =
{
	4,
	{
		{ dump_RDD_TUNNEL_HEADER_PSRAM_BUFFER, 0x26e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TASK_IDX_10 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x26e4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_10 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, 0x26e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_10 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, 0x26f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_10 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x26f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEFAULT_VLAN_VID_TABLE_10 =
{
	2,
	{
		{ dump_RDD_DEFAULT_VLAN_VID_ENTRY, 0x2700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_10 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, 0x2720 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2727 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_10 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2728 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_10 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x272c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RATE_LIMIT_OVERHEAD_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x272e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_QOS_DONT_DROP_RATIO_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x272f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_10 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2730 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_QOS_WAN_UNTAGGED_PRIORITY_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2732 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IC_MCAST_ENABLE_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2733 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT ECN_IPV6_REMARK_TABLE_10 =
{
	1,
	{
		{ dump_RDD_ECN_IPV6_REMARK_ENTRY, 0x2734 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_REDIRECT_MODE_10 =
{
	1,
	{
		{ dump_RDD_CPU_REDIRECT_MODE_ENTRY, 0x2735 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2736 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_10 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x2737 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_10 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, 0x2738 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x273a },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x273b },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2740 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEFAULT_VLAN_VID_TO_TX_VLAN_CNTR_TABLE_10 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DS_PACKET_BUFFER_11 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_11 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_11 =
{
	8,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, 0x1280 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DSCP_TO_PBITS_MAP_TABLE_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TCAM_IC_CMD_TABLE_11 =
{
	4,
	{
		{ dump_RDD_TCAM_IC_CMD, 0x1400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_PROCESSING_TASKS_REPLY_11 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x1640 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PBIT_TO_GEM_TABLE_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_11 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, 0x1700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_PROCESSING_TASKS_LOCK_11 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x17c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TC_TO_QUEUE_TABLE_11 =
{
	8,
	{
		{ dump_RDD_TC_TO_QUEUE_8, 0x1800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_FPM_TOKENS_READ_FLAG_11 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x1a08 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_TO_DSCP_TO_PBITS_TABLE_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1a10 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_11 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1a20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_BBH_OVERRUN_TASKS_LIMIT_11 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x1a40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT LAYER2_HEADER_COPY_MAPPING_TABLE_11 =
{
	4,
	{
		{ dump_RDD_LAYER2_HEADER_COPY_MAPPING_ENTRY, 0x1a80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_11 =
{
	8,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x1af8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT EMAC_FLOW_CTRL_11 =
{
	16,
	{
		{ dump_RDD_EMAC_FLOW_CTRL_ENTRY, 0x1b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_11 =
{
	1,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, 0x1b80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_11 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0x1bd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FLOODING_HEADER_FPM_POOL_NUMBER_MAPPING_TABLE_11 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0x1be0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PBIT_TO_QUEUE_TABLE_11 =
{
	8,
	{
		{ dump_RDD_PBIT_TO_QUEUE_8, 0x1c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_11 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1e08 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1e30 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT GLOBAL_DSCP_TO_PBITS_TABLE_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1e40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_11 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, 0x1e80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PORT_MAC_11 =
{
	8,
	{
		{ dump_RDD_MAC_ADDRESS, 0x1ec0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_11 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, 0x1f00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DUAL_STACK_LITE_TABLE_11 =
{
	64,
	{
		{ dump_RDD_DUAL_STACK_LITE_ENTRY, 0x1f80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT QUEUE_THRESHOLD_VECTOR_11 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1fc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_PACKET_BASED_MAPPING_11 =
{
	4,
	{
		{ dump_RDD_INGRESS_PACKET_BASED_MAPPING_ENTRY, 0x1fe4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PROCESSING_QUEUE_DYNAMIC_MNG_TABLE_11 =
{
	8,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x1fe8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SPDSVC_ANALYZER_PARAMS_TABLE_11 =
{
	16,
	{
		{ dump_RDD_SPDSVC_ANALYZER_PARAMS, 0x1ff0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_11 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DBG_DUMP_TABLE_11 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_TBL_CFG_11 =
{
	24,
	{
		{ dump_RDD_NATC_TBL_CONFIGURATION, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_COUNTERS_GENERIC_CONTEXT_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2448 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_FLOW_CONTEXT_DDR_ADDR_11 =
{
	8,
	{
		{ dump_RDD_DEF_FLOW_CONTEXT_DDR_ADDR, 0x24c8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT BRIDGE_CFG_TABLE_11 =
{
	8,
	{
		{ dump_RDD_BRIDGE_CFG, 0x24d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_PBIT_TO_DISCARD_PRIO_VECTOR_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x24e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x24f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MAX_PKT_LEN_TABLE_11 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_11 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2540 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TCAM_IC_CFG_TABLE_11 =
{
	16,
	{
		{ dump_RDD_TCAM_IC_CFG, 0x25c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VLAN_ACTION_GPE_HANDLER_PTR_TABLE_11 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2640 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_BBH_OVERRUN_MAX_TASKS_LIMIT_11 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2662 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_11 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, 0x2664 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NULL_BUFFER_11 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x2668 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPTV_CONFIGURATION_TABLE_11 =
{
	12,
	{
		{ dump_RDD_HW_IPTV_CONFIGURATION, 0x2670 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT ZERO_VALUE_11 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x267c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TUNNEL_TABLE_11 =
{
	16,
	{
		{ dump_RDD_TUNNEL_ENTRY, 0x2680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_DESCRIPTOR_TABLE_11 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x26a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_11 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, 0x26b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_11 =
{
	12,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, 0x26c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT ONE_VALUE_11 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x26cc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_11 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x26d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_BBH_OVERRUN_MIN_TASKS_LIMIT_11 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x26d6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_11 =
{
	5,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x26d8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x26dd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x26de },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x26df },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TUNNEL_HEADER_PSRAM_BUFFER_11 =
{
	4,
	{
		{ dump_RDD_TUNNEL_HEADER_PSRAM_BUFFER, 0x26e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TASK_IDX_11 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x26e4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_11 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, 0x26e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_11 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, 0x26f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_11 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x26f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEFAULT_VLAN_VID_TABLE_11 =
{
	2,
	{
		{ dump_RDD_DEFAULT_VLAN_VID_ENTRY, 0x2700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_11 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, 0x2720 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2727 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_11 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2728 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_11 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x272c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RATE_LIMIT_OVERHEAD_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x272e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_QOS_DONT_DROP_RATIO_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x272f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_11 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2730 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_QOS_WAN_UNTAGGED_PRIORITY_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2732 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IC_MCAST_ENABLE_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2733 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT ECN_IPV6_REMARK_TABLE_11 =
{
	1,
	{
		{ dump_RDD_ECN_IPV6_REMARK_ENTRY, 0x2734 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_REDIRECT_MODE_11 =
{
	1,
	{
		{ dump_RDD_CPU_REDIRECT_MODE_ENTRY, 0x2735 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2736 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_11 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x2737 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_11 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, 0x2738 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x273a },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x273b },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2740 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEFAULT_VLAN_VID_TO_TX_VLAN_CNTR_TABLE_11 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DS_PACKET_BUFFER_12 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_12 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_12 =
{
	8,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, 0x1280 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DSCP_TO_PBITS_MAP_TABLE_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TCAM_IC_CMD_TABLE_12 =
{
	4,
	{
		{ dump_RDD_TCAM_IC_CMD, 0x1400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_PROCESSING_TASKS_REPLY_12 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x1640 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PBIT_TO_GEM_TABLE_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_12 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, 0x1700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_PROCESSING_TASKS_LOCK_12 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x17c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TC_TO_QUEUE_TABLE_12 =
{
	8,
	{
		{ dump_RDD_TC_TO_QUEUE_8, 0x1800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_FPM_TOKENS_READ_FLAG_12 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x1a08 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_TO_DSCP_TO_PBITS_TABLE_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1a10 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_12 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1a20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_BBH_OVERRUN_TASKS_LIMIT_12 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x1a40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT LAYER2_HEADER_COPY_MAPPING_TABLE_12 =
{
	4,
	{
		{ dump_RDD_LAYER2_HEADER_COPY_MAPPING_ENTRY, 0x1a80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_12 =
{
	8,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x1af8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT EMAC_FLOW_CTRL_12 =
{
	16,
	{
		{ dump_RDD_EMAC_FLOW_CTRL_ENTRY, 0x1b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_12 =
{
	1,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, 0x1b80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_12 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0x1bd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FLOODING_HEADER_FPM_POOL_NUMBER_MAPPING_TABLE_12 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0x1be0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PBIT_TO_QUEUE_TABLE_12 =
{
	8,
	{
		{ dump_RDD_PBIT_TO_QUEUE_8, 0x1c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_12 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1e08 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1e30 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT GLOBAL_DSCP_TO_PBITS_TABLE_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1e40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_12 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, 0x1e80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PORT_MAC_12 =
{
	8,
	{
		{ dump_RDD_MAC_ADDRESS, 0x1ec0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_12 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, 0x1f00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DUAL_STACK_LITE_TABLE_12 =
{
	64,
	{
		{ dump_RDD_DUAL_STACK_LITE_ENTRY, 0x1f80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT QUEUE_THRESHOLD_VECTOR_12 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1fc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_PACKET_BASED_MAPPING_12 =
{
	4,
	{
		{ dump_RDD_INGRESS_PACKET_BASED_MAPPING_ENTRY, 0x1fe4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PROCESSING_QUEUE_DYNAMIC_MNG_TABLE_12 =
{
	8,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x1fe8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SPDSVC_ANALYZER_PARAMS_TABLE_12 =
{
	16,
	{
		{ dump_RDD_SPDSVC_ANALYZER_PARAMS, 0x1ff0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_12 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DBG_DUMP_TABLE_12 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_TBL_CFG_12 =
{
	24,
	{
		{ dump_RDD_NATC_TBL_CONFIGURATION, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_COUNTERS_GENERIC_CONTEXT_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2448 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_FLOW_CONTEXT_DDR_ADDR_12 =
{
	8,
	{
		{ dump_RDD_DEF_FLOW_CONTEXT_DDR_ADDR, 0x24c8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT BRIDGE_CFG_TABLE_12 =
{
	8,
	{
		{ dump_RDD_BRIDGE_CFG, 0x24d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_PBIT_TO_DISCARD_PRIO_VECTOR_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x24e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x24f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MAX_PKT_LEN_TABLE_12 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_12 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2540 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TCAM_IC_CFG_TABLE_12 =
{
	16,
	{
		{ dump_RDD_TCAM_IC_CFG, 0x25c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VLAN_ACTION_GPE_HANDLER_PTR_TABLE_12 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2640 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_BBH_OVERRUN_MAX_TASKS_LIMIT_12 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2662 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_12 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, 0x2664 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NULL_BUFFER_12 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x2668 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPTV_CONFIGURATION_TABLE_12 =
{
	12,
	{
		{ dump_RDD_HW_IPTV_CONFIGURATION, 0x2670 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT ZERO_VALUE_12 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x267c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TUNNEL_TABLE_12 =
{
	16,
	{
		{ dump_RDD_TUNNEL_ENTRY, 0x2680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_DESCRIPTOR_TABLE_12 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x26a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_12 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, 0x26b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_12 =
{
	12,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, 0x26c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT ONE_VALUE_12 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x26cc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_12 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x26d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_BBH_OVERRUN_MIN_TASKS_LIMIT_12 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x26d6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_12 =
{
	5,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x26d8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x26dd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x26de },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x26df },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TUNNEL_HEADER_PSRAM_BUFFER_12 =
{
	4,
	{
		{ dump_RDD_TUNNEL_HEADER_PSRAM_BUFFER, 0x26e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TASK_IDX_12 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x26e4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_12 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, 0x26e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_12 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, 0x26f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_12 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x26f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEFAULT_VLAN_VID_TABLE_12 =
{
	2,
	{
		{ dump_RDD_DEFAULT_VLAN_VID_ENTRY, 0x2700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_12 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, 0x2720 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2727 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_12 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2728 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_12 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x272c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RATE_LIMIT_OVERHEAD_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x272e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_QOS_DONT_DROP_RATIO_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x272f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_12 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2730 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_QOS_WAN_UNTAGGED_PRIORITY_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2732 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IC_MCAST_ENABLE_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2733 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT ECN_IPV6_REMARK_TABLE_12 =
{
	1,
	{
		{ dump_RDD_ECN_IPV6_REMARK_ENTRY, 0x2734 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_REDIRECT_MODE_12 =
{
	1,
	{
		{ dump_RDD_CPU_REDIRECT_MODE_ENTRY, 0x2735 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2736 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_12 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x2737 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_12 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, 0x2738 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x273a },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x273b },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2740 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEFAULT_VLAN_VID_TO_TX_VLAN_CNTR_TABLE_12 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DS_PACKET_BUFFER_13 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_13 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_13 =
{
	8,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, 0x1280 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DSCP_TO_PBITS_MAP_TABLE_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TCAM_IC_CMD_TABLE_13 =
{
	4,
	{
		{ dump_RDD_TCAM_IC_CMD, 0x1400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_PROCESSING_TASKS_REPLY_13 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x1640 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PBIT_TO_GEM_TABLE_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_13 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, 0x1700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_PROCESSING_TASKS_LOCK_13 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x17c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TC_TO_QUEUE_TABLE_13 =
{
	8,
	{
		{ dump_RDD_TC_TO_QUEUE_8, 0x1800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_FPM_TOKENS_READ_FLAG_13 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x1a08 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_TO_DSCP_TO_PBITS_TABLE_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1a10 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_13 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1a20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_BBH_OVERRUN_TASKS_LIMIT_13 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x1a40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT LAYER2_HEADER_COPY_MAPPING_TABLE_13 =
{
	4,
	{
		{ dump_RDD_LAYER2_HEADER_COPY_MAPPING_ENTRY, 0x1a80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_13 =
{
	8,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x1af8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT EMAC_FLOW_CTRL_13 =
{
	16,
	{
		{ dump_RDD_EMAC_FLOW_CTRL_ENTRY, 0x1b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_13 =
{
	1,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, 0x1b80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_13 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0x1bd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FLOODING_HEADER_FPM_POOL_NUMBER_MAPPING_TABLE_13 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0x1be0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PBIT_TO_QUEUE_TABLE_13 =
{
	8,
	{
		{ dump_RDD_PBIT_TO_QUEUE_8, 0x1c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_13 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1e08 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1e30 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT GLOBAL_DSCP_TO_PBITS_TABLE_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1e40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_13 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, 0x1e80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PORT_MAC_13 =
{
	8,
	{
		{ dump_RDD_MAC_ADDRESS, 0x1ec0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_13 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, 0x1f00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DUAL_STACK_LITE_TABLE_13 =
{
	64,
	{
		{ dump_RDD_DUAL_STACK_LITE_ENTRY, 0x1f80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT QUEUE_THRESHOLD_VECTOR_13 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1fc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_PACKET_BASED_MAPPING_13 =
{
	4,
	{
		{ dump_RDD_INGRESS_PACKET_BASED_MAPPING_ENTRY, 0x1fe4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PROCESSING_QUEUE_DYNAMIC_MNG_TABLE_13 =
{
	8,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x1fe8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SPDSVC_ANALYZER_PARAMS_TABLE_13 =
{
	16,
	{
		{ dump_RDD_SPDSVC_ANALYZER_PARAMS, 0x1ff0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_13 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DBG_DUMP_TABLE_13 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_TBL_CFG_13 =
{
	24,
	{
		{ dump_RDD_NATC_TBL_CONFIGURATION, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_COUNTERS_GENERIC_CONTEXT_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2448 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_FLOW_CONTEXT_DDR_ADDR_13 =
{
	8,
	{
		{ dump_RDD_DEF_FLOW_CONTEXT_DDR_ADDR, 0x24c8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT BRIDGE_CFG_TABLE_13 =
{
	8,
	{
		{ dump_RDD_BRIDGE_CFG, 0x24d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_PBIT_TO_DISCARD_PRIO_VECTOR_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x24e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x24f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MAX_PKT_LEN_TABLE_13 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_13 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2540 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TCAM_IC_CFG_TABLE_13 =
{
	16,
	{
		{ dump_RDD_TCAM_IC_CFG, 0x25c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VLAN_ACTION_GPE_HANDLER_PTR_TABLE_13 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2640 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_BBH_OVERRUN_MAX_TASKS_LIMIT_13 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2662 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_13 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, 0x2664 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NULL_BUFFER_13 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x2668 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPTV_CONFIGURATION_TABLE_13 =
{
	12,
	{
		{ dump_RDD_HW_IPTV_CONFIGURATION, 0x2670 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT ZERO_VALUE_13 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x267c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TUNNEL_TABLE_13 =
{
	16,
	{
		{ dump_RDD_TUNNEL_ENTRY, 0x2680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_DESCRIPTOR_TABLE_13 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x26a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_13 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, 0x26b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_13 =
{
	12,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, 0x26c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT ONE_VALUE_13 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x26cc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_13 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x26d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_BBH_OVERRUN_MIN_TASKS_LIMIT_13 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x26d6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_13 =
{
	5,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x26d8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x26dd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x26de },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x26df },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TUNNEL_HEADER_PSRAM_BUFFER_13 =
{
	4,
	{
		{ dump_RDD_TUNNEL_HEADER_PSRAM_BUFFER, 0x26e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TASK_IDX_13 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x26e4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_13 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, 0x26e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_13 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, 0x26f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_13 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x26f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEFAULT_VLAN_VID_TABLE_13 =
{
	2,
	{
		{ dump_RDD_DEFAULT_VLAN_VID_ENTRY, 0x2700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_13 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, 0x2720 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2727 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_13 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2728 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_13 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x272c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RATE_LIMIT_OVERHEAD_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x272e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_QOS_DONT_DROP_RATIO_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x272f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_13 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2730 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_QOS_WAN_UNTAGGED_PRIORITY_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2732 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IC_MCAST_ENABLE_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2733 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT ECN_IPV6_REMARK_TABLE_13 =
{
	1,
	{
		{ dump_RDD_ECN_IPV6_REMARK_ENTRY, 0x2734 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_REDIRECT_MODE_13 =
{
	1,
	{
		{ dump_RDD_CPU_REDIRECT_MODE_ENTRY, 0x2735 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2736 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_13 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x2737 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_13 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, 0x2738 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x273a },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x273b },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2740 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEFAULT_VLAN_VID_TO_TX_VLAN_CNTR_TABLE_13 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DS_PACKET_BUFFER_14 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_14 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_14 =
{
	8,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, 0x1280 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DSCP_TO_PBITS_MAP_TABLE_14 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TCAM_IC_CMD_TABLE_14 =
{
	4,
	{
		{ dump_RDD_TCAM_IC_CMD, 0x1400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_PROCESSING_TASKS_REPLY_14 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x1640 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PBIT_TO_GEM_TABLE_14 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_14 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, 0x1700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_PROCESSING_TASKS_LOCK_14 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x17c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TC_TO_QUEUE_TABLE_14 =
{
	8,
	{
		{ dump_RDD_TC_TO_QUEUE_8, 0x1800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_FPM_TOKENS_READ_FLAG_14 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x1a08 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_TO_DSCP_TO_PBITS_TABLE_14 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1a10 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_14 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1a20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_BBH_OVERRUN_TASKS_LIMIT_14 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x1a40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT LAYER2_HEADER_COPY_MAPPING_TABLE_14 =
{
	4,
	{
		{ dump_RDD_LAYER2_HEADER_COPY_MAPPING_ENTRY, 0x1a80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_14 =
{
	8,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x1af8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT EMAC_FLOW_CTRL_14 =
{
	16,
	{
		{ dump_RDD_EMAC_FLOW_CTRL_ENTRY, 0x1b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_14 =
{
	1,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, 0x1b80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_14 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0x1bd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FLOODING_HEADER_FPM_POOL_NUMBER_MAPPING_TABLE_14 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0x1be0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PBIT_TO_QUEUE_TABLE_14 =
{
	8,
	{
		{ dump_RDD_PBIT_TO_QUEUE_8, 0x1c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_14 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1e08 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_14 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1e30 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT GLOBAL_DSCP_TO_PBITS_TABLE_14 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1e40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_14 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, 0x1e80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PORT_MAC_14 =
{
	8,
	{
		{ dump_RDD_MAC_ADDRESS, 0x1ec0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_14 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, 0x1f00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DUAL_STACK_LITE_TABLE_14 =
{
	64,
	{
		{ dump_RDD_DUAL_STACK_LITE_ENTRY, 0x1f80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT QUEUE_THRESHOLD_VECTOR_14 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1fc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_PACKET_BASED_MAPPING_14 =
{
	4,
	{
		{ dump_RDD_INGRESS_PACKET_BASED_MAPPING_ENTRY, 0x1fe4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PROCESSING_QUEUE_DYNAMIC_MNG_TABLE_14 =
{
	8,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x1fe8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SPDSVC_ANALYZER_PARAMS_TABLE_14 =
{
	16,
	{
		{ dump_RDD_SPDSVC_ANALYZER_PARAMS, 0x1ff0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_14 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DBG_DUMP_TABLE_14 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_TBL_CFG_14 =
{
	24,
	{
		{ dump_RDD_NATC_TBL_CONFIGURATION, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_COUNTERS_GENERIC_CONTEXT_14 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2448 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_FLOW_CONTEXT_DDR_ADDR_14 =
{
	8,
	{
		{ dump_RDD_DEF_FLOW_CONTEXT_DDR_ADDR, 0x24c8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT BRIDGE_CFG_TABLE_14 =
{
	8,
	{
		{ dump_RDD_BRIDGE_CFG, 0x24d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_PBIT_TO_DISCARD_PRIO_VECTOR_14 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x24e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_14 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x24f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MAX_PKT_LEN_TABLE_14 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_14 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2540 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TCAM_IC_CFG_TABLE_14 =
{
	16,
	{
		{ dump_RDD_TCAM_IC_CFG, 0x25c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VLAN_ACTION_GPE_HANDLER_PTR_TABLE_14 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2640 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_BBH_OVERRUN_MAX_TASKS_LIMIT_14 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2662 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_14 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, 0x2664 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NULL_BUFFER_14 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x2668 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPTV_CONFIGURATION_TABLE_14 =
{
	12,
	{
		{ dump_RDD_HW_IPTV_CONFIGURATION, 0x2670 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT ZERO_VALUE_14 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x267c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TUNNEL_TABLE_14 =
{
	16,
	{
		{ dump_RDD_TUNNEL_ENTRY, 0x2680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_DESCRIPTOR_TABLE_14 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x26a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_14 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, 0x26b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_14 =
{
	12,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, 0x26c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT ONE_VALUE_14 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x26cc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_14 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x26d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_BBH_OVERRUN_MIN_TASKS_LIMIT_14 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x26d6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_14 =
{
	5,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x26d8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_14 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x26dd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_14 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x26de },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_14 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x26df },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TUNNEL_HEADER_PSRAM_BUFFER_14 =
{
	4,
	{
		{ dump_RDD_TUNNEL_HEADER_PSRAM_BUFFER, 0x26e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TASK_IDX_14 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x26e4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_14 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, 0x26e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_14 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, 0x26f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_14 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x26f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEFAULT_VLAN_VID_TABLE_14 =
{
	2,
	{
		{ dump_RDD_DEFAULT_VLAN_VID_ENTRY, 0x2700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_14 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, 0x2720 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_14 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2727 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_14 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2728 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_14 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x272c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RATE_LIMIT_OVERHEAD_14 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x272e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_QOS_DONT_DROP_RATIO_14 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x272f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_14 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2730 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_QOS_WAN_UNTAGGED_PRIORITY_14 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2732 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IC_MCAST_ENABLE_14 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2733 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT ECN_IPV6_REMARK_TABLE_14 =
{
	1,
	{
		{ dump_RDD_ECN_IPV6_REMARK_ENTRY, 0x2734 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_REDIRECT_MODE_14 =
{
	1,
	{
		{ dump_RDD_CPU_REDIRECT_MODE_ENTRY, 0x2735 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_14 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2736 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_14 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x2737 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_14 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, 0x2738 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_14 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x273a },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_14 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x273b },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_14 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2740 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEFAULT_VLAN_VID_TO_TX_VLAN_CNTR_TABLE_14 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DS_PACKET_BUFFER_15 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_15 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_15 =
{
	8,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, 0x1280 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DSCP_TO_PBITS_MAP_TABLE_15 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TCAM_IC_CMD_TABLE_15 =
{
	4,
	{
		{ dump_RDD_TCAM_IC_CMD, 0x1400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_PROCESSING_TASKS_REPLY_15 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x1640 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PBIT_TO_GEM_TABLE_15 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_15 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, 0x1700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_PROCESSING_TASKS_LOCK_15 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x17c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TC_TO_QUEUE_TABLE_15 =
{
	8,
	{
		{ dump_RDD_TC_TO_QUEUE_8, 0x1800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_FPM_TOKENS_READ_FLAG_15 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x1a08 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_TO_DSCP_TO_PBITS_TABLE_15 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1a10 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_15 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1a20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_BBH_OVERRUN_TASKS_LIMIT_15 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x1a40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT LAYER2_HEADER_COPY_MAPPING_TABLE_15 =
{
	4,
	{
		{ dump_RDD_LAYER2_HEADER_COPY_MAPPING_ENTRY, 0x1a80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_15 =
{
	8,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x1af8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT EMAC_FLOW_CTRL_15 =
{
	16,
	{
		{ dump_RDD_EMAC_FLOW_CTRL_ENTRY, 0x1b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_15 =
{
	1,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, 0x1b80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_15 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0x1bd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FLOODING_HEADER_FPM_POOL_NUMBER_MAPPING_TABLE_15 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0x1be0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PBIT_TO_QUEUE_TABLE_15 =
{
	8,
	{
		{ dump_RDD_PBIT_TO_QUEUE_8, 0x1c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_15 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1e08 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_15 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1e30 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT GLOBAL_DSCP_TO_PBITS_TABLE_15 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1e40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_15 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, 0x1e80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PORT_MAC_15 =
{
	8,
	{
		{ dump_RDD_MAC_ADDRESS, 0x1ec0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_15 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, 0x1f00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DUAL_STACK_LITE_TABLE_15 =
{
	64,
	{
		{ dump_RDD_DUAL_STACK_LITE_ENTRY, 0x1f80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT QUEUE_THRESHOLD_VECTOR_15 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1fc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_PACKET_BASED_MAPPING_15 =
{
	4,
	{
		{ dump_RDD_INGRESS_PACKET_BASED_MAPPING_ENTRY, 0x1fe4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT PROCESSING_QUEUE_DYNAMIC_MNG_TABLE_15 =
{
	8,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x1fe8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SPDSVC_ANALYZER_PARAMS_TABLE_15 =
{
	16,
	{
		{ dump_RDD_SPDSVC_ANALYZER_PARAMS, 0x1ff0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_15 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DBG_DUMP_TABLE_15 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_TBL_CFG_15 =
{
	24,
	{
		{ dump_RDD_NATC_TBL_CONFIGURATION, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_COUNTERS_GENERIC_CONTEXT_15 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2448 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_FLOW_CONTEXT_DDR_ADDR_15 =
{
	8,
	{
		{ dump_RDD_DEF_FLOW_CONTEXT_DDR_ADDR, 0x24c8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT BRIDGE_CFG_TABLE_15 =
{
	8,
	{
		{ dump_RDD_BRIDGE_CFG, 0x24d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VPORT_PBIT_TO_DISCARD_PRIO_VECTOR_15 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x24e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_15 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x24f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MAX_PKT_LEN_TABLE_15 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_15 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2540 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TCAM_IC_CFG_TABLE_15 =
{
	16,
	{
		{ dump_RDD_TCAM_IC_CFG, 0x25c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT VLAN_ACTION_GPE_HANDLER_PTR_TABLE_15 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2640 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_BBH_OVERRUN_MAX_TASKS_LIMIT_15 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2662 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_15 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, 0x2664 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NULL_BUFFER_15 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x2668 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPTV_CONFIGURATION_TABLE_15 =
{
	12,
	{
		{ dump_RDD_HW_IPTV_CONFIGURATION, 0x2670 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT ZERO_VALUE_15 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x267c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TUNNEL_TABLE_15 =
{
	16,
	{
		{ dump_RDD_TUNNEL_ENTRY, 0x2680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_DESCRIPTOR_TABLE_15 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x26a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_15 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, 0x26b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_15 =
{
	12,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, 0x26c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT ONE_VALUE_15 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x26cc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_15 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x26d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT MCAST_BBH_OVERRUN_MIN_TASKS_LIMIT_15 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x26d6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_15 =
{
	5,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x26d8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_15 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x26dd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_15 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x26de },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_15 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x26df },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TUNNEL_HEADER_PSRAM_BUFFER_15 =
{
	4,
	{
		{ dump_RDD_TUNNEL_HEADER_PSRAM_BUFFER, 0x26e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TASK_IDX_15 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x26e4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_15 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, 0x26e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_15 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, 0x26f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_15 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x26f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEFAULT_VLAN_VID_TABLE_15 =
{
	2,
	{
		{ dump_RDD_DEFAULT_VLAN_VID_ENTRY, 0x2700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_15 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, 0x2720 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_15 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2727 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_15 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2728 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_15 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x272c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RATE_LIMIT_OVERHEAD_15 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x272e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_QOS_DONT_DROP_RATIO_15 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x272f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_15 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2730 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_QOS_WAN_UNTAGGED_PRIORITY_15 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2732 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IC_MCAST_ENABLE_15 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2733 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT ECN_IPV6_REMARK_TABLE_15 =
{
	1,
	{
		{ dump_RDD_ECN_IPV6_REMARK_ENTRY, 0x2734 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT CPU_REDIRECT_MODE_15 =
{
	1,
	{
		{ dump_RDD_CPU_REDIRECT_MODE_ENTRY, 0x2735 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_15 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2736 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_15 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x2737 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_15 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, 0x2738 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_15 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x273a },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_15 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x273b },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_15 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2740 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6858
static DUMP_RUNNERREG_STRUCT DEFAULT_VLAN_VID_TO_TX_VLAN_CNTR_TABLE_15 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2780 },
		{ 0, 0 },
	}
};
#endif

TABLE_STRUCT RUNNER_TABLES[NUMBER_OF_TABLES] =
{
#if defined BCM6858
	{ "DS_TM_PD_FIFO_TABLE", 1, CORE_0_INDEX, &DS_TM_PD_FIFO_TABLE, 320, 1, 1 },
#endif
#if defined BCM6858
	{ "COMPLEX_SCHEDULER_TABLE", 1, CORE_0_INDEX, &COMPLEX_SCHEDULER_TABLE, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "DS_TM_SCHEDULING_QUEUE_TABLE", 1, CORE_0_INDEX, &DS_TM_SCHEDULING_QUEUE_TABLE, 160, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_TBL_CFG", 1, CORE_0_INDEX, &NATC_TBL_CFG, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "DS_TM_BBH_TX_WAKE_UP_DATA_TABLE", 1, CORE_0_INDEX, &DS_TM_BBH_TX_WAKE_UP_DATA_TABLE, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DS_TM_FLUSH_DISPATCHER_CREDIT_TABLE", 1, CORE_0_INDEX, &DS_TM_FLUSH_DISPATCHER_CREDIT_TABLE, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "TM_HW_FLUSH", 1, CORE_0_INDEX, &TM_HW_FLUSH, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_0_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "DS_TM_BBH_TX_EGRESS_COUNTER_TABLE", 1, CORE_0_INDEX, &DS_TM_BBH_TX_EGRESS_COUNTER_TABLE, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "DS_TM_TM_FLOW_CNTR_TABLE", 1, CORE_0_INDEX, &DS_TM_TM_FLOW_CNTR_TABLE, 64, 1, 1 },
#endif
#if defined BCM6858
	{ "BASIC_SCHEDULER_TABLE_DS", 1, CORE_0_INDEX, &BASIC_SCHEDULER_TABLE_DS, 32, 1, 1 },
#endif
#if defined BCM6858
	{ "BASIC_RATE_LIMITER_TABLE_DS", 1, CORE_0_INDEX, &BASIC_RATE_LIMITER_TABLE_DS, 128, 1, 1 },
#endif
#if defined BCM6858
	{ "VLAN_TX_COUNTERS", 1, CORE_0_INDEX, &VLAN_TX_COUNTERS, 257, 1, 1 },
#endif
#if defined BCM6858
	{ "DBG_DUMP_TABLE", 1, CORE_0_INDEX, &DBG_DUMP_TABLE, 128, 1, 1 },
#endif
#if defined BCM6858
	{ "DS_TM_CPU_TX_ABS_COUNTERS", 1, CORE_0_INDEX, &DS_TM_CPU_TX_ABS_COUNTERS, 32, 1, 1 },
#endif
#if defined BCM6858
	{ "REGISTERS_BUFFER", 1, CORE_0_INDEX, &REGISTERS_BUFFER, 32, 1, 1 },
#endif
#if defined BCM6858
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_0_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TX_MIRRORING_DISPATCHER_CREDIT_TABLE", 1, CORE_0_INDEX, &TX_MIRRORING_DISPATCHER_CREDIT_TABLE, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "ZERO_VALUE", 1, CORE_0_INDEX, &ZERO_VALUE, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DS_TM_SCHEDULING_FLUSH_VECTOR", 1, CORE_0_INDEX, &DS_TM_SCHEDULING_FLUSH_VECTOR, 5, 1, 1 },
#endif
#if defined BCM6858
	{ "ONE_VALUE", 1, CORE_0_INDEX, &ONE_VALUE, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "MIRRORING_SCRATCH", 1, CORE_0_INDEX, &MIRRORING_SCRATCH, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DS_TM_TM_ACTION_PTR_TABLE", 1, CORE_0_INDEX, &DS_TM_TM_ACTION_PTR_TABLE, 17, 1, 1 },
#endif
#if defined BCM6858
	{ "BUDGET_ALLOCATION_TIMER_VALUE", 1, CORE_0_INDEX, &BUDGET_ALLOCATION_TIMER_VALUE, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DS_TM_BB_DESTINATION_TABLE", 1, CORE_0_INDEX, &DS_TM_BB_DESTINATION_TABLE, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DS_TM_SCHEDULING_GLOBAL_FLUSH_CFG", 1, CORE_0_INDEX, &DS_TM_SCHEDULING_GLOBAL_FLUSH_CFG, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SCHEDULING_FLUSH_GLOBAL_CFG", 1, CORE_0_INDEX, &SCHEDULING_FLUSH_GLOBAL_CFG, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TASK_IDX", 1, CORE_0_INDEX, &TASK_IDX, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TM_VLAN_STATS_ENABLE", 1, CORE_0_INDEX, &TM_VLAN_STATS_ENABLE, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TX_EXCEPTION", 1, CORE_0_INDEX, &TX_EXCEPTION, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CORE_ID_TABLE", 1, CORE_0_INDEX, &CORE_ID_TABLE, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DS_TM_FLUSH_AGGREGATION_TASK_DISABLE", 1, CORE_0_INDEX, &DS_TM_FLUSH_AGGREGATION_TASK_DISABLE, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEBUG_PRINT_TABLE", 1, CORE_0_INDEX, &DEBUG_PRINT_TABLE, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "UPDATE_FIFO_TABLE", 1, CORE_0_INDEX, &UPDATE_FIFO_TABLE, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "DS_TM_SCHEDULING_QUEUE_AGING_VECTOR", 1, CORE_0_INDEX, &DS_TM_SCHEDULING_QUEUE_AGING_VECTOR, 5, 1, 1 },
#endif
#if defined BCM6858
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_0_INDEX, &TX_MIRRORING_CONFIGURATION, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SRAM_DUMMY_STORE", 1, CORE_0_INDEX, &SRAM_DUMMY_STORE, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RATE_LIMIT_OVERHEAD", 1, CORE_0_INDEX, &RATE_LIMIT_OVERHEAD, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DS_TM_FIRST_QUEUE_MAPPING", 1, CORE_0_INDEX, &DS_TM_FIRST_QUEUE_MAPPING, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "BACKUP_BBH_INGRESS_COUNTERS_TABLE", 1, CORE_0_INDEX, &BACKUP_BBH_INGRESS_COUNTERS_TABLE, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "BACKUP_BBH_EGRESS_COUNTERS_TABLE", 1, CORE_0_INDEX, &BACKUP_BBH_EGRESS_COUNTERS_TABLE, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_0_INDEX, &DEBUG_PRINT_CORE_LOCK, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DS_TM_SCHEDULING_AGGREGATION_CONTEXT_VECTOR", 1, CORE_0_INDEX, &DS_TM_SCHEDULING_AGGREGATION_CONTEXT_VECTOR, 5, 1, 1 },
#endif
#if defined BCM6858
	{ "DEBUG_SCRATCHPAD", 1, CORE_0_INDEX, &DEBUG_SCRATCHPAD, 10, 1, 1 },
#endif
#if defined BCM6858
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_0_INDEX, &RUNNER_PROFILING_TRACE_BUFFER, 128, 1, 1 },
#endif
#if defined BCM6858
	{ "DS_TM_BBH_QUEUE_TABLE", 1, CORE_0_INDEX, &DS_TM_BBH_QUEUE_TABLE, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "RATE_LIMITER_VALID_TABLE_DS", 1, CORE_0_INDEX, &RATE_LIMITER_VALID_TABLE_DS, 4, 1, 1 },
#endif
#if defined BCM6858
	{ "FPM_GLOBAL_CFG", 1, CORE_0_INDEX, &FPM_GLOBAL_CFG, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TCPSPDTEST_STREAM_TABLE", 1, CORE_1_INDEX, &TCPSPDTEST_STREAM_TABLE_1, 4, 1, 1 },
#endif
#if defined BCM6858
	{ "DS_CPU_REASON_TO_METER_TABLE", 1, CORE_1_INDEX, &DS_CPU_REASON_TO_METER_TABLE_1, 64, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_RX_SCRATCHPAD", 1, CORE_1_INDEX, &CPU_RX_SCRATCHPAD_1, 64, 1, 1 },
#endif
#if defined BCM6858
	{ "WLAN_MCAST_COPY_SCRATCHPAD", 1, CORE_1_INDEX, &WLAN_MCAST_COPY_SCRATCHPAD_1, 64, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_FLOW_RING_CACHE_CTX_TABLE", 1, CORE_1_INDEX, &DHD_FLOW_RING_CACHE_CTX_TABLE_1, 48, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_RX_TASK_STACK", 1, CORE_1_INDEX, &CPU_RX_TASK_STACK_1, 64, 1, 1 },
#endif
#if defined BCM6858
	{ "PKTGEN_TX_STREAM_TABLE", 1, CORE_1_INDEX, &PKTGEN_TX_STREAM_TABLE_1, 4, 1, 1 },
#endif
#if defined BCM6858
	{ "WLAN_MCAST_SSID_STATS_TABLE", 1, CORE_1_INDEX, &WLAN_MCAST_SSID_STATS_TABLE_1, 48, 1, 1 },
#endif
#if defined BCM6858
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_1_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_1, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "US_CPU_REASON_TO_METER_TABLE", 1, CORE_1_INDEX, &US_CPU_REASON_TO_METER_TABLE_1, 64, 1, 1 },
#endif
#if defined BCM6858
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_1_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_1, 128, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_POST_COMMON_RADIO_DATA", 1, CORE_1_INDEX, &DHD_POST_COMMON_RADIO_DATA_1, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_1_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "VPORT_TO_FLOW_IDX", 1, CORE_1_INDEX, &VPORT_TO_FLOW_IDX_1, 32, 1, 1 },
#endif
#if defined BCM6858
	{ "IPV6_HOST_ADDRESS_CRC_TABLE", 1, CORE_1_INDEX, &IPV6_HOST_ADDRESS_CRC_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "WLAN_MCAST_DHD_STATION_TABLE", 1, CORE_1_INDEX, &WLAN_MCAST_DHD_STATION_TABLE_1, 64, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_BACKUP_INFO_CACHE_TABLE", 1, CORE_1_INDEX, &DHD_BACKUP_INFO_CACHE_TABLE_1, 48, 1, 1 },
#endif
#if defined BCM6858
	{ "DS_CPU_RX_METER_TABLE", 1, CORE_1_INDEX, &DS_CPU_RX_METER_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "WLAN_MCAST_SSID_MAC_ADDRESS_TABLE", 1, CORE_1_INDEX, &WLAN_MCAST_SSID_MAC_ADDRESS_TABLE_1, 48, 1, 1 },
#endif
#if defined BCM6858
	{ "US_CPU_RX_METER_TABLE", 1, CORE_1_INDEX, &US_CPU_RX_METER_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_RECYCLE_SRAM_PD_FIFO", 1, CORE_1_INDEX, &CPU_RECYCLE_SRAM_PD_FIFO_1, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "DBG_DUMP_TABLE", 1, CORE_1_INDEX, &DBG_DUMP_TABLE_1, 128, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_RING_INTERRUPT_COUNTER_TABLE", 1, CORE_1_INDEX, &CPU_RING_INTERRUPT_COUNTER_TABLE_1, 18, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_1_INDEX, &DHD_FPM_POOL_NUMBER_MAPPING_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_FEED_RING_RSV_TABLE", 1, CORE_1_INDEX, &CPU_FEED_RING_RSV_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "WLAN_MCAST_DHD_STATION_CTX_TABLE", 1, CORE_1_INDEX, &WLAN_MCAST_DHD_STATION_CTX_TABLE_1, 64, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_TX_POST_PD_FIFO_TABLE", 1, CORE_1_INDEX, &DHD_TX_POST_PD_FIFO_TABLE_1, 12, 1, 1 },
#endif
#if defined BCM6858
	{ "WLAN_MCAST_DFT_LIST_ENTRY_SCRATCH", 1, CORE_1_INDEX, &WLAN_MCAST_DFT_LIST_ENTRY_SCRATCH_1, 64, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_TX_POST_FLOW_RING_BUFFER", 1, CORE_1_INDEX, &DHD_TX_POST_FLOW_RING_BUFFER_1, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_RECYCLE_INTERRUPT_COALESCING_TABLE", 1, CORE_1_INDEX, &CPU_RECYCLE_INTERRUPT_COALESCING_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPV4_HOST_ADDRESS_TABLE", 1, CORE_1_INDEX, &IPV4_HOST_ADDRESS_TABLE_1, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "SPDSVC_GEN_PARAMS_TABLE", 1, CORE_1_INDEX, &SPDSVC_GEN_PARAMS_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "PKTGEN_NO_SBPM_HDRS_CNTR", 1, CORE_1_INDEX, &PKTGEN_NO_SBPM_HDRS_CNTR_1, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_RX_COPY_PD_FIFO_TABLE", 1, CORE_1_INDEX, &CPU_RX_COPY_PD_FIFO_TABLE_1, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_REASON_AND_VPORT_TO_METER_TABLE", 1, CORE_1_INDEX, &CPU_REASON_AND_VPORT_TO_METER_TABLE_1, 120, 1, 1 },
#endif
#if defined BCM6858
	{ "TX_ABS_RECYCLE_COUNTERS", 1, CORE_1_INDEX, &TX_ABS_RECYCLE_COUNTERS_1, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CSO_CONTEXT_TABLE", 1, CORE_1_INDEX, &CSO_CONTEXT_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SPDTEST_GEN_PARAM", 1, CORE_1_INDEX, &SPDTEST_GEN_PARAM_1, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_RX_COPY_INT_SCRATCHPAD", 1, CORE_1_INDEX, &CPU_RX_COPY_INT_SCRATCHPAD_1, 4, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_BACKUP_INDEX_CACHE", 1, CORE_1_INDEX, &DHD_BACKUP_INDEX_CACHE_1, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "PKTGEN_FPM_UG_MGMT", 1, CORE_1_INDEX, &PKTGEN_FPM_UG_MGMT_1, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "ZERO_VALUE", 1, CORE_1_INDEX, &ZERO_VALUE_1, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR", 1, CORE_1_INDEX, &CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_1, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_FLOW_RING_CACHE_LKP_TABLE", 1, CORE_1_INDEX, &DHD_FLOW_RING_CACHE_LKP_TABLE_1, 48, 1, 1 },
#endif
#if defined BCM6858
	{ "TCPSPDTEST_ENGINE_GLOBAL_TABLE", 1, CORE_1_INDEX, &TCPSPDTEST_ENGINE_GLOBAL_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "ONE_VALUE", 1, CORE_1_INDEX, &ONE_VALUE_1, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_RX_INTERRUPT_ID_DDR_ADDR", 1, CORE_1_INDEX, &CPU_RX_INTERRUPT_ID_DDR_ADDR_1, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_L2_HEADER", 1, CORE_1_INDEX, &DHD_L2_HEADER_1, 72, 1, 1 },
#endif
#if defined BCM6858
	{ "DEBUG_SCRATCHPAD", 1, CORE_1_INDEX, &DEBUG_SCRATCHPAD_1, 10, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_RX_PSRAM_GET_NEXT_SCRATCHPAD", 1, CORE_1_INDEX, &CPU_RX_PSRAM_GET_NEXT_SCRATCHPAD_1, 4, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_TBL_CFG", 1, CORE_1_INDEX, &NATC_TBL_CFG_1, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "MIRRORING_SCRATCH", 1, CORE_1_INDEX, &MIRRORING_SCRATCH_1, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_HW_CFG", 1, CORE_1_INDEX, &DHD_HW_CFG_1, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_MCAST_DISPATCHER_CREDIT_TABLE", 1, CORE_1_INDEX, &DHD_MCAST_DISPATCHER_CREDIT_TABLE_1, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_RX_INTERRUPT_SCRATCH", 1, CORE_1_INDEX, &CPU_RX_INTERRUPT_SCRATCH_1, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_MIRRORING_DISPATCHER_CREDIT_TABLE", 1, CORE_1_INDEX, &DHD_MIRRORING_DISPATCHER_CREDIT_TABLE_1, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "PKTGEN_CURR_SBPM_HDR_PTR", 1, CORE_1_INDEX, &PKTGEN_CURR_SBPM_HDR_PTR_1, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "PKTGEN_NUM_OF_AVAIL_SBPM_HDRS", 1, CORE_1_INDEX, &PKTGEN_NUM_OF_AVAIL_SBPM_HDRS_1, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "COMMON_REPROCESSING_PD_FIFO_TABLE", 1, CORE_1_INDEX, &COMMON_REPROCESSING_PD_FIFO_TABLE_1, 4, 1, 1 },
#endif
#if defined BCM6858
	{ "REGISTERS_BUFFER", 1, CORE_1_INDEX, &REGISTERS_BUFFER_1, 32, 1, 1 },
#endif
#if defined BCM6858
	{ "WLAN_MCAST_SCRATCHPAD", 1, CORE_1_INDEX, &WLAN_MCAST_SCRATCHPAD_1, 5, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR", 1, CORE_1_INDEX, &DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_1, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_RX_COPY_DISPATCHER_CREDIT_TABLE", 1, CORE_1_INDEX, &CPU_RX_COPY_DISPATCHER_CREDIT_TABLE_1, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "PKTGEN_SBPM_END_PTR", 1, CORE_1_INDEX, &PKTGEN_SBPM_END_PTR_1, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SPDSVC_WLAN_TXPOST_PARAMS_TABLE", 1, CORE_1_INDEX, &SPDSVC_WLAN_TXPOST_PARAMS_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "WLAN_MCAST_DFT_LIST_SIZE", 1, CORE_1_INDEX, &WLAN_MCAST_DFT_LIST_SIZE_1, 64, 1, 1 },
#endif
#if defined BCM6858
	{ "UPDATE_FIFO_TABLE", 1, CORE_1_INDEX, &UPDATE_FIFO_TABLE_1, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "WAN_LOOPBACK_DISPATCHER_CREDIT_TABLE", 1, CORE_1_INDEX, &WAN_LOOPBACK_DISPATCHER_CREDIT_TABLE_1, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "PKTGEN_BAD_GET_NEXT", 1, CORE_1_INDEX, &PKTGEN_BAD_GET_NEXT_1, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SPDSVC_GEN_DISPATCHER_CREDIT_TABLE", 1, CORE_1_INDEX, &SPDSVC_GEN_DISPATCHER_CREDIT_TABLE_1, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "PKTGEN_MAX_UT_PKTS", 1, CORE_1_INDEX, &PKTGEN_MAX_UT_PKTS_1, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "PKTGEN_SBPM_HDR_BNS", 1, CORE_1_INDEX, &PKTGEN_SBPM_HDR_BNS_1, 28, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_DOORBELL_TX_POST_VALUE", 1, CORE_1_INDEX, &DHD_DOORBELL_TX_POST_VALUE_1, 17, 1, 1 },
#endif
#if defined BCM6858
	{ "PKTGEN_UT_TRIGGER", 1, CORE_1_INDEX, &PKTGEN_UT_TRIGGER_1, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "UDPSPDTEST_STREAM_RX_STAT_TABLE", 1, CORE_1_INDEX, &UDPSPDTEST_STREAM_RX_STAT_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "COMMON_REPROCESSING_DISPATCHER_CREDIT_TABLE", 1, CORE_1_INDEX, &COMMON_REPROCESSING_DISPATCHER_CREDIT_TABLE_1, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "TASK_IDX", 1, CORE_1_INDEX, &TASK_IDX_1, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_TX_POST_UPDATE_FIFO_TABLE", 1, CORE_1_INDEX, &DHD_TX_POST_UPDATE_FIFO_TABLE_1, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "BCM_SPDSVC_STREAM_RX_TS_TABLE", 1, CORE_1_INDEX, &BCM_SPDSVC_STREAM_RX_TS_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CORE_ID_TABLE", 1, CORE_1_INDEX, &CORE_ID_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SPDSVC_TCPSPDTEST_COMMON_TABLE", 1, CORE_1_INDEX, &SPDSVC_TCPSPDTEST_COMMON_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_FEED_RING_INTERRUPT_THRESHOLD", 1, CORE_1_INDEX, &CPU_FEED_RING_INTERRUPT_THRESHOLD_1, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_RECYCLE_RING_DESCRIPTOR_TABLE", 1, CORE_1_INDEX, &CPU_RECYCLE_RING_DESCRIPTOR_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE", 1, CORE_1_INDEX, &DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE_1, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE", 1, CORE_1_INDEX, &CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE_1, 2, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_MCAST_UPDATE_FIFO_TABLE", 1, CORE_1_INDEX, &DHD_MCAST_UPDATE_FIFO_TABLE_1, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_INTERRUPT_COALESCING_TABLE", 1, CORE_1_INDEX, &CPU_INTERRUPT_COALESCING_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_FEED_RING_DESCRIPTOR_TABLE", 1, CORE_1_INDEX, &CPU_FEED_RING_DESCRIPTOR_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_VPORT_TO_METER_TABLE", 1, CORE_1_INDEX, &CPU_VPORT_TO_METER_TABLE_1, 40, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_BACKUP_BASE_ADDR", 1, CORE_1_INDEX, &DHD_BACKUP_BASE_ADDR_1, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_RECYCLE_INTERRUPT_SCRATCH", 1, CORE_1_INDEX, &CPU_RECYCLE_INTERRUPT_SCRATCH_1, 2, 1, 1 },
#endif
#if defined BCM6858
	{ "WLAN_MCAST_DFT_ADDR", 1, CORE_1_INDEX, &WLAN_MCAST_DFT_ADDR_1, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_RX_COPY_UPDATE_FIFO_TABLE", 1, CORE_1_INDEX, &CPU_RX_COPY_UPDATE_FIFO_TABLE_1, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_RX_LOCAL_SCRATCH", 1, CORE_1_INDEX, &CPU_RX_LOCAL_SCRATCH_1, 2, 1, 1 },
#endif
#if defined BCM6858
	{ "PKTGEN_SBPM_EXTS", 1, CORE_1_INDEX, &PKTGEN_SBPM_EXTS_1, 4, 1, 1 },
#endif
#if defined BCM6858
	{ "UDPSPDTEST_STREAM_TX_STAT_TABLE", 1, CORE_1_INDEX, &UDPSPDTEST_STREAM_TX_STAT_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_FEED_RING_INTERRUPT_COUNTER", 1, CORE_1_INDEX, &CPU_FEED_RING_INTERRUPT_COUNTER_1, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SRAM_DUMMY_STORE", 1, CORE_1_INDEX, &SRAM_DUMMY_STORE_1, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_1_INDEX, &DEBUG_PRINT_CORE_LOCK_1, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE", 1, CORE_1_INDEX, &CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "PKTGEN_BBMSG_REPLY_SCRATCH", 1, CORE_1_INDEX, &PKTGEN_BBMSG_REPLY_SCRATCH_1, 2, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_RX_COPY_LOCAL_SCRATCH", 1, CORE_1_INDEX, &CPU_RX_COPY_LOCAL_SCRATCH_1, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "COMMON_REPROCESSING_UPDATE_FIFO_TABLE", 1, CORE_1_INDEX, &COMMON_REPROCESSING_UPDATE_FIFO_TABLE_1, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "DEBUG_PRINT_TABLE", 1, CORE_1_INDEX, &DEBUG_PRINT_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "FPM_GLOBAL_CFG", 1, CORE_1_INDEX, &FPM_GLOBAL_CFG_1, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "PD_FIFO_TABLE", 1, CORE_1_INDEX, &PD_FIFO_TABLE_1, 2, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_RECYCLE_SHADOW_RD_IDX", 1, CORE_1_INDEX, &CPU_RECYCLE_SHADOW_RD_IDX_1, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_RECYCLE_SHADOW_WR_IDX", 1, CORE_1_INDEX, &CPU_RECYCLE_SHADOW_WR_IDX_1, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_MCAST_PD_FIFO_TABLE", 1, CORE_1_INDEX, &DHD_MCAST_PD_FIFO_TABLE_1, 2, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_REASON_TO_TC", 1, CORE_1_INDEX, &CPU_REASON_TO_TC_1, 64, 1, 1 },
#endif
#if defined BCM6858
	{ "TC_TO_CPU_RXQ", 1, CORE_1_INDEX, &TC_TO_CPU_RXQ_1, 64, 1, 1 },
#endif
#if defined BCM6858
	{ "EXC_TC_TO_CPU_RXQ", 1, CORE_1_INDEX, &EXC_TC_TO_CPU_RXQ_1, 64, 1, 1 },
#endif
#if defined BCM6858
	{ "PKTGEN_SESSION_DATA", 1, CORE_1_INDEX, &PKTGEN_SESSION_DATA_1, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "VPORT_TO_CPU_OBJ", 1, CORE_1_INDEX, &VPORT_TO_CPU_OBJ_1, 40, 1, 1 },
#endif
#if defined BCM6858
	{ "PKTGEN_TX_STREAM_SCRATCH_TABLE", 1, CORE_1_INDEX, &PKTGEN_TX_STREAM_SCRATCH_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_RING_DESCRIPTORS_TABLE", 1, CORE_1_INDEX, &CPU_RING_DESCRIPTORS_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_TX_SCRATCHPAD", 1, CORE_2_INDEX, &CPU_TX_SCRATCHPAD_2, 512, 1, 1 },
#endif
#if defined BCM6858
	{ "REPORTING_QUEUE_DESCRIPTOR_TABLE", 1, CORE_2_INDEX, &REPORTING_QUEUE_DESCRIPTOR_TABLE_2, 160, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_COMPLETE_COMMON_RADIO_DATA", 1, CORE_2_INDEX, &DHD_COMPLETE_COMMON_RADIO_DATA_2, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_TX_FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_2_INDEX, &CPU_TX_FPM_POOL_NUMBER_MAPPING_TABLE_2, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_2_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_2, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_TX_DBG_CNTRS_TABLE", 1, CORE_2_INDEX, &CPU_TX_DBG_CNTRS_TABLE_2, 2, 1, 1 },
#endif
#if defined BCM6858
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_2_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_2, 128, 1, 1 },
#endif
#if defined BCM6858
	{ "FW_POLICER_CBS", 1, CORE_2_INDEX, &FW_POLICER_CBS_2, 80, 1, 1 },
#endif
#if defined BCM6858
	{ "SERVICE_QUEUES_COMPLEX_SCHEDULER_TABLE", 1, CORE_2_INDEX, &SERVICE_QUEUES_COMPLEX_SCHEDULER_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "REPORTING_QUEUE_ACCUMULATED_TABLE", 1, CORE_2_INDEX, &REPORTING_QUEUE_ACCUMULATED_TABLE_2, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "SERVICE_QUEUES_RATE_LIMITER_TABLE", 1, CORE_2_INDEX, &SERVICE_QUEUES_RATE_LIMITER_TABLE_2, 66, 1, 1 },
#endif
#if defined BCM6858
	{ "SERVICE_QUEUES_SCHEDULING_QUEUE_AGING_VECTOR", 1, CORE_2_INDEX, &SERVICE_QUEUES_SCHEDULING_QUEUE_AGING_VECTOR_2, 5, 1, 1 },
#endif
#if defined BCM6858
	{ "SPDTEST_GEN_PARAM", 1, CORE_2_INDEX, &SPDTEST_GEN_PARAM_2, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_2_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_TX_COMPLETE_FLOW_RING_BUFFER", 1, CORE_2_INDEX, &DHD_TX_COMPLETE_FLOW_RING_BUFFER_2, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "FPM_REPLY", 1, CORE_2_INDEX, &FPM_REPLY_2, 2, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_RX_COMPLETE_FLOW_RING_BUFFER", 1, CORE_2_INDEX, &DHD_RX_COMPLETE_FLOW_RING_BUFFER_2, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "GHOST_REPORTING_QUEUE_STATUS_BIT_VECTOR_TABLE", 1, CORE_2_INDEX, &GHOST_REPORTING_QUEUE_STATUS_BIT_VECTOR_TABLE_2, 5, 1, 1 },
#endif
#if defined BCM6858
	{ "GHOST_REPORTING_GLOBAL_CFG", 1, CORE_2_INDEX, &GHOST_REPORTING_GLOBAL_CFG_2, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TIMER_COMMON_CTR_REP", 1, CORE_2_INDEX, &TIMER_COMMON_CTR_REP_2, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE", 1, CORE_2_INDEX, &SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE_2, 32, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_RECYCLE_SRAM_PD_FIFO", 1, CORE_2_INDEX, &CPU_RECYCLE_SRAM_PD_FIFO_2, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "TX_FLOW_TABLE", 1, CORE_2_INDEX, &TX_FLOW_TABLE_2, 192, 1, 1 },
#endif
#if defined BCM6858
	{ "QUEUE_THRESHOLD_VECTOR", 1, CORE_2_INDEX, &QUEUE_THRESHOLD_VECTOR_2, 9, 1, 1 },
#endif
#if defined BCM6858
	{ "ZERO_VALUE", 1, CORE_2_INDEX, &ZERO_VALUE_2, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR", 1, CORE_2_INDEX, &CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_2, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_RECYCLE_INTERRUPT_COALESCING_TABLE", 1, CORE_2_INDEX, &CPU_RECYCLE_INTERRUPT_COALESCING_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SERVICE_QUEUES_PD_FIFO_TABLE", 1, CORE_2_INDEX, &SERVICE_QUEUES_PD_FIFO_TABLE_2, 64, 1, 1 },
#endif
#if defined BCM6858
	{ "DBG_DUMP_TABLE", 1, CORE_2_INDEX, &DBG_DUMP_TABLE_2, 128, 1, 1 },
#endif
#if defined BCM6858
	{ "FW_POLICER_BUDGET", 1, CORE_2_INDEX, &FW_POLICER_BUDGET_2, 80, 1, 1 },
#endif
#if defined BCM6858
	{ "FW_POLICER_BUDGET_REMAINDER", 1, CORE_2_INDEX, &FW_POLICER_BUDGET_REMAINDER_2, 80, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_HW_CFG", 1, CORE_2_INDEX, &DHD_HW_CFG_2, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "REPORTING_COUNTER_TABLE", 1, CORE_2_INDEX, &REPORTING_COUNTER_TABLE_2, 40, 1, 1 },
#endif
#if defined BCM6858
	{ "QUEUE_TO_REPORT_BIT_VECTOR", 1, CORE_2_INDEX, &QUEUE_TO_REPORT_BIT_VECTOR_2, 5, 1, 1 },
#endif
#if defined BCM6858
	{ "ONE_VALUE", 1, CORE_2_INDEX, &ONE_VALUE_2, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_RX_INTERRUPT_ID_DDR_ADDR", 1, CORE_2_INDEX, &CPU_RX_INTERRUPT_ID_DDR_ADDR_2, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SERVICE_QUEUES_TM_ACTION_PTR_TABLE", 1, CORE_2_INDEX, &SERVICE_QUEUES_TM_ACTION_PTR_TABLE_2, 17, 1, 1 },
#endif
#if defined BCM6858
	{ "TIMER_COMMON_TIMER_VALUE", 1, CORE_2_INDEX, &TIMER_COMMON_TIMER_VALUE_2, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TASK_IDX", 1, CORE_2_INDEX, &TASK_IDX_2, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR", 1, CORE_2_INDEX, &DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_2, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE", 1, CORE_2_INDEX, &CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE_2, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "SERVICE_QUEUES_BUDGET_ALLOCATION_TIMER_VALUE", 1, CORE_2_INDEX, &SERVICE_QUEUES_BUDGET_ALLOCATION_TIMER_VALUE_2, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "EMAC_FLOW_CTRL_VECTOR", 1, CORE_2_INDEX, &EMAC_FLOW_CTRL_VECTOR_2, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_TX_VLAN_STATS_ENABLE", 1, CORE_2_INDEX, &CPU_TX_VLAN_STATS_ENABLE_2, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_RX_POST_FLOW_RING_BUFFER", 1, CORE_2_INDEX, &DHD_RX_POST_FLOW_RING_BUFFER_2, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "REGISTERS_BUFFER", 1, CORE_2_INDEX, &REGISTERS_BUFFER_2, 32, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_TX_RING_DESCRIPTOR_TABLE", 1, CORE_2_INDEX, &CPU_TX_RING_DESCRIPTOR_TABLE_2, 2, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_TBL_CFG", 1, CORE_2_INDEX, &NATC_TBL_CFG_2, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "DEBUG_SCRATCHPAD", 1, CORE_2_INDEX, &DEBUG_SCRATCHPAD_2, 10, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_TX_INGRESS_DISPATCHER_CREDIT_TABLE", 1, CORE_2_INDEX, &CPU_TX_INGRESS_DISPATCHER_CREDIT_TABLE_2, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "CORE_ID_TABLE", 1, CORE_2_INDEX, &CORE_ID_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RATE_LIMIT_OVERHEAD", 1, CORE_2_INDEX, &RATE_LIMIT_OVERHEAD_2, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_RX_POST_RING_SIZE", 1, CORE_2_INDEX, &DHD_RX_POST_RING_SIZE_2, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE", 1, CORE_2_INDEX, &DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE_2, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_RX_COMPLETE_0_DISPATCHER_CREDIT_TABLE", 1, CORE_2_INDEX, &DHD_RX_COMPLETE_0_DISPATCHER_CREDIT_TABLE_2, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_RX_COMPLETE_RING_SIZE", 1, CORE_2_INDEX, &DHD_RX_COMPLETE_RING_SIZE_2, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_TX_COMPLETE_RING_SIZE", 1, CORE_2_INDEX, &DHD_TX_COMPLETE_RING_SIZE_2, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "EMAC_FLOW_CTRL_BUDGET", 1, CORE_2_INDEX, &EMAC_FLOW_CTRL_BUDGET_2, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "SERVICE_QUEUES_RATE_LIMITER_VALID_TABLE", 1, CORE_2_INDEX, &SERVICE_QUEUES_RATE_LIMITER_VALID_TABLE_2, 4, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_RX_COMPLETE_1_DISPATCHER_CREDIT_TABLE", 1, CORE_2_INDEX, &DHD_RX_COMPLETE_1_DISPATCHER_CREDIT_TABLE_2, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "SRAM_DUMMY_STORE", 1, CORE_2_INDEX, &SRAM_DUMMY_STORE_2, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_2_INDEX, &DEBUG_PRINT_CORE_LOCK_2, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SERVICE_QUEUES_FIRST_QUEUE_MAPPING", 1, CORE_2_INDEX, &SERVICE_QUEUES_FIRST_QUEUE_MAPPING_2, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "BBH_TX_INGRESS_COUNTER_TABLE", 1, CORE_2_INDEX, &BBH_TX_INGRESS_COUNTER_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RING_CPU_TX_DESCRIPTOR_DATA_TABLE", 1, CORE_2_INDEX, &RING_CPU_TX_DESCRIPTOR_DATA_TABLE_2, 2, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_RX_COMPLETE_2_DISPATCHER_CREDIT_TABLE", 1, CORE_2_INDEX, &DHD_RX_COMPLETE_2_DISPATCHER_CREDIT_TABLE_2, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_TX_COMPLETE_0_DISPATCHER_CREDIT_TABLE", 1, CORE_2_INDEX, &DHD_TX_COMPLETE_0_DISPATCHER_CREDIT_TABLE_2, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "SERVICE_QUEUES_UPDATE_FIFO_TABLE", 1, CORE_2_INDEX, &SERVICE_QUEUES_UPDATE_FIFO_TABLE_2, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_TX_COMPLETE_1_DISPATCHER_CREDIT_TABLE", 1, CORE_2_INDEX, &DHD_TX_COMPLETE_1_DISPATCHER_CREDIT_TABLE_2, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_TX_COMPLETE_2_DISPATCHER_CREDIT_TABLE", 1, CORE_2_INDEX, &DHD_TX_COMPLETE_2_DISPATCHER_CREDIT_TABLE_2, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_FPM_REPLY", 1, CORE_2_INDEX, &DHD_FPM_REPLY_2, 24, 1, 1 },
#endif
#if defined BCM6858
	{ "REPORT_BBH_TX_QUEUE_ID_TABLE", 1, CORE_2_INDEX, &REPORT_BBH_TX_QUEUE_ID_TABLE_2, 2, 1, 1 },
#endif
#if defined BCM6858
	{ "FW_POLICER_VECTOR", 1, CORE_2_INDEX, &FW_POLICER_VECTOR_2, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_DOORBELL_TX_COMPLETE_VALUE", 1, CORE_2_INDEX, &DHD_DOORBELL_TX_COMPLETE_VALUE_2, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_DOORBELL_RX_COMPLETE_VALUE", 1, CORE_2_INDEX, &DHD_DOORBELL_RX_COMPLETE_VALUE_2, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_DOORBELL_RX_POST_VALUE", 1, CORE_2_INDEX, &DHD_DOORBELL_RX_POST_VALUE_2, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "SERVICE_QUEUES_DISPATCHER_CREDIT_TABLE", 1, CORE_2_INDEX, &SERVICE_QUEUES_DISPATCHER_CREDIT_TABLE_2, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "SPDSVC_WLAN_GEN_PARAMS_TABLE", 1, CORE_2_INDEX, &SPDSVC_WLAN_GEN_PARAMS_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_TX_SYNC_FIFO_TABLE", 1, CORE_2_INDEX, &CPU_TX_SYNC_FIFO_TABLE_2, 2, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_RECYCLE_RING_DESCRIPTOR_TABLE", 1, CORE_2_INDEX, &CPU_RECYCLE_RING_DESCRIPTOR_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE", 1, CORE_2_INDEX, &CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE_2, 2, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_RECYCLE_INTERRUPT_SCRATCH", 1, CORE_2_INDEX, &CPU_RECYCLE_INTERRUPT_SCRATCH_2, 2, 1, 1 },
#endif
#if defined BCM6858
	{ "EMAC_FLOW_CTRL_BUDGET_REMAINDER", 1, CORE_2_INDEX, &EMAC_FLOW_CTRL_BUDGET_REMAINDER_2, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "DEBUG_PRINT_TABLE", 1, CORE_2_INDEX, &DEBUG_PRINT_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "FPM_GLOBAL_CFG", 1, CORE_2_INDEX, &FPM_GLOBAL_CFG_2, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "XGPON_REPORT_ZERO_SENT_TABLE", 1, CORE_2_INDEX, &XGPON_REPORT_ZERO_SENT_TABLE_2, 10, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE", 1, CORE_2_INDEX, &CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_TX_RING_INDICES_VALUES_TABLE", 1, CORE_2_INDEX, &CPU_TX_RING_INDICES_VALUES_TABLE_2, 2, 1, 1 },
#endif
#if defined BCM6858
	{ "BBH_TX_EGRESS_REPORT_COUNTER_TABLE", 1, CORE_2_INDEX, &BBH_TX_EGRESS_REPORT_COUNTER_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_RECYCLE_SHADOW_RD_IDX", 1, CORE_2_INDEX, &CPU_RECYCLE_SHADOW_RD_IDX_2, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_RECYCLE_SHADOW_WR_IDX", 1, CORE_2_INDEX, &CPU_RECYCLE_SHADOW_WR_IDX_2, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "US_TM_PD_FIFO_TABLE", 1, CORE_3_INDEX, &US_TM_PD_FIFO_TABLE_3, 320, 1, 1 },
#endif
#if defined BCM6858
	{ "COMPLEX_SCHEDULER_TABLE", 1, CORE_3_INDEX, &COMPLEX_SCHEDULER_TABLE_3, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "US_TM_SCHEDULING_QUEUE_TABLE", 1, CORE_3_INDEX, &US_TM_SCHEDULING_QUEUE_TABLE_3, 160, 1, 1 },
#endif
#if defined BCM6858
	{ "DBG_DUMP_TABLE", 1, CORE_3_INDEX, &DBG_DUMP_TABLE_3, 128, 1, 1 },
#endif
#if defined BCM6858
	{ "DIRECT_PROCESSING_RX_MIRRORING_SCRATCHPAD", 1, CORE_3_INDEX, &DIRECT_PROCESSING_RX_MIRRORING_SCRATCHPAD_3, 136, 1, 1 },
#endif
#if defined BCM6858
	{ "US_TM_WAN_0_BBH_TX_WAKE_UP_DATA_TABLE", 1, CORE_3_INDEX, &US_TM_WAN_0_BBH_TX_WAKE_UP_DATA_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DIRECT_PROCESSING_PD_TABLE", 1, CORE_3_INDEX, &DIRECT_PROCESSING_PD_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_3_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_3, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "US_TM_WAN_0_BBH_TX_EGRESS_COUNTER_TABLE", 1, CORE_3_INDEX, &US_TM_WAN_0_BBH_TX_EGRESS_COUNTER_TABLE_3, 40, 1, 1 },
#endif
#if defined BCM6858
	{ "BBH_TX_EPON_WAKE_UP_DATA_TABLE", 1, CORE_3_INDEX, &BBH_TX_EPON_WAKE_UP_DATA_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "OVERALL_RATE_LIMITER_TABLE", 1, CORE_3_INDEX, &OVERALL_RATE_LIMITER_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "BASIC_RATE_LIMITER_TABLE_US", 1, CORE_3_INDEX, &BASIC_RATE_LIMITER_TABLE_US_3, 128, 1, 1 },
#endif
#if defined BCM6858
	{ "VLAN_TX_COUNTERS", 1, CORE_3_INDEX, &VLAN_TX_COUNTERS_3, 257, 1, 1 },
#endif
#if defined BCM6858
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_3_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "US_TM_FLUSH_DISPATCHER_CREDIT_TABLE", 1, CORE_3_INDEX, &US_TM_FLUSH_DISPATCHER_CREDIT_TABLE_3, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "TM_HW_FLUSH", 1, CORE_3_INDEX, &TM_HW_FLUSH_3, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DIRECT_PROCESSING_FLOW_CNTR_TABLE", 1, CORE_3_INDEX, &DIRECT_PROCESSING_FLOW_CNTR_TABLE_3, 32, 1, 1 },
#endif
#if defined BCM6858
	{ "BBH_TX_EPON_INGRESS_COUNTER_TABLE", 1, CORE_3_INDEX, &BBH_TX_EPON_INGRESS_COUNTER_TABLE_3, 40, 1, 1 },
#endif
#if defined BCM6858
	{ "MIRRORING_SCRATCH", 1, CORE_3_INDEX, &MIRRORING_SCRATCH_3, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DISPATCHER_CREDIT_TABLE", 1, CORE_3_INDEX, &DISPATCHER_CREDIT_TABLE_3, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "ZERO_VALUE", 1, CORE_3_INDEX, &ZERO_VALUE_3, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "US_TM_TM_FLOW_CNTR_TABLE", 1, CORE_3_INDEX, &US_TM_TM_FLOW_CNTR_TABLE_3, 128, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_TBL_CFG", 1, CORE_3_INDEX, &NATC_TBL_CFG_3, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "US_TM_CPU_TX_ABS_COUNTERS", 1, CORE_3_INDEX, &US_TM_CPU_TX_ABS_COUNTERS_3, 32, 1, 1 },
#endif
#if defined BCM6858
	{ "ONE_VALUE", 1, CORE_3_INDEX, &ONE_VALUE_3, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DIRECT_PROCESSING_PAUSE_QUANTA", 1, CORE_3_INDEX, &DIRECT_PROCESSING_PAUSE_QUANTA_3, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "BUDGET_ALLOCATION_TIMER_VALUE", 1, CORE_3_INDEX, &BUDGET_ALLOCATION_TIMER_VALUE_3, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TX_MIRRORING_DISPATCHER_CREDIT_TABLE", 1, CORE_3_INDEX, &TX_MIRRORING_DISPATCHER_CREDIT_TABLE_3, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "US_TM_BBH_TX_WAN_0_FIFO_BYTES_USED", 1, CORE_3_INDEX, &US_TM_BBH_TX_WAN_0_FIFO_BYTES_USED_3, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DIRECT_PROCESSING_EPON_CONTROL_SCRATCH", 1, CORE_3_INDEX, &DIRECT_PROCESSING_EPON_CONTROL_SCRATCH_3, 22, 1, 1 },
#endif
#if defined BCM6858
	{ "US_TM_WAN_0_BB_DESTINATION_TABLE", 1, CORE_3_INDEX, &US_TM_WAN_0_BB_DESTINATION_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TASK_IDX", 1, CORE_3_INDEX, &TASK_IDX_3, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "US_TM_SCHEDULING_GLOBAL_FLUSH_CFG", 1, CORE_3_INDEX, &US_TM_SCHEDULING_GLOBAL_FLUSH_CFG_3, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "US_TM_TX_PAUSE_NACK", 1, CORE_3_INDEX, &US_TM_TX_PAUSE_NACK_3, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "BBH_TX_US_WAN_0_FIFO_BYTES_THRESHOLD", 1, CORE_3_INDEX, &BBH_TX_US_WAN_0_FIFO_BYTES_THRESHOLD_3, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "US_TM_BBH_QUEUE_TABLE", 1, CORE_3_INDEX, &US_TM_BBH_QUEUE_TABLE_3, 40, 1, 1 },
#endif
#if defined BCM6858
	{ "REGISTERS_BUFFER", 1, CORE_3_INDEX, &REGISTERS_BUFFER_3, 32, 1, 1 },
#endif
#if defined BCM6858
	{ "US_TM_SCHEDULING_FLUSH_VECTOR", 1, CORE_3_INDEX, &US_TM_SCHEDULING_FLUSH_VECTOR_3, 5, 1, 1 },
#endif
#if defined BCM6858
	{ "SCHEDULING_FLUSH_GLOBAL_CFG", 1, CORE_3_INDEX, &SCHEDULING_FLUSH_GLOBAL_CFG_3, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TM_VLAN_STATS_ENABLE", 1, CORE_3_INDEX, &TM_VLAN_STATS_ENABLE_3, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TX_EXCEPTION", 1, CORE_3_INDEX, &TX_EXCEPTION_3, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CORE_ID_TABLE", 1, CORE_3_INDEX, &CORE_ID_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_3_INDEX, &RX_MIRRORING_CONFIGURATION_3, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_3_INDEX, &TX_MIRRORING_CONFIGURATION_3, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "US_TM_FLUSH_AGGREGATION_TASK_DISABLE", 1, CORE_3_INDEX, &US_TM_FLUSH_AGGREGATION_TASK_DISABLE_3, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SRAM_DUMMY_STORE", 1, CORE_3_INDEX, &SRAM_DUMMY_STORE_3, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RATE_LIMIT_OVERHEAD", 1, CORE_3_INDEX, &RATE_LIMIT_OVERHEAD_3, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "MAC_TYPE", 1, CORE_3_INDEX, &MAC_TYPE_3, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "US_TM_TM_ACTION_PTR_TABLE", 1, CORE_3_INDEX, &US_TM_TM_ACTION_PTR_TABLE_3, 17, 1, 1 },
#endif
#if defined BCM6858
	{ "WAN_0_BBH_TX_FIFO_SIZE", 1, CORE_3_INDEX, &WAN_0_BBH_TX_FIFO_SIZE_3, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "US_TM_FIRST_QUEUE_MAPPING", 1, CORE_3_INDEX, &US_TM_FIRST_QUEUE_MAPPING_3, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_3_INDEX, &DEBUG_PRINT_CORE_LOCK_3, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_3_INDEX, &RX_MIRRORING_DIRECT_ENABLE_3, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEBUG_PRINT_TABLE", 1, CORE_3_INDEX, &DEBUG_PRINT_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "BBH_TX_EPON_EGRESS_COUNTER_TABLE", 1, CORE_3_INDEX, &BBH_TX_EPON_EGRESS_COUNTER_TABLE_3, 32, 1, 1 },
#endif
#if defined BCM6858
	{ "US_TM_SCHEDULING_QUEUE_AGING_VECTOR", 1, CORE_3_INDEX, &US_TM_SCHEDULING_QUEUE_AGING_VECTOR_3, 5, 1, 1 },
#endif
#if defined BCM6858
	{ "UPDATE_FIFO_TABLE", 1, CORE_3_INDEX, &UPDATE_FIFO_TABLE_3, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "US_TM_SCHEDULING_AGGREGATION_CONTEXT_VECTOR", 1, CORE_3_INDEX, &US_TM_SCHEDULING_AGGREGATION_CONTEXT_VECTOR_3, 5, 1, 1 },
#endif
#if defined BCM6858
	{ "BASIC_SCHEDULER_TABLE_US", 1, CORE_3_INDEX, &BASIC_SCHEDULER_TABLE_US_3, 33, 1, 1 },
#endif
#if defined BCM6858
	{ "DEBUG_SCRATCHPAD", 1, CORE_3_INDEX, &DEBUG_SCRATCHPAD_3, 10, 1, 1 },
#endif
#if defined BCM6858
	{ "EPON_UPDATE_FIFO_TABLE", 1, CORE_3_INDEX, &EPON_UPDATE_FIFO_TABLE_3, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "US_TM_TX_OCTETS_COUNTERS_TABLE", 1, CORE_3_INDEX, &US_TM_TX_OCTETS_COUNTERS_TABLE_3, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "RATE_LIMITER_VALID_TABLE_US", 1, CORE_3_INDEX, &RATE_LIMITER_VALID_TABLE_US_3, 4, 1, 1 },
#endif
#if defined BCM6858
	{ "FPM_GLOBAL_CFG", 1, CORE_3_INDEX, &FPM_GLOBAL_CFG_3, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_3_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_3, 128, 1, 1 },
#endif
#if defined BCM6858
	{ "DS_PACKET_BUFFER", 1, CORE_4_INDEX, &DS_PACKET_BUFFER_4, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_FLOW_TABLE", 1, CORE_4_INDEX, &RX_FLOW_TABLE_4, 320, 1, 1 },
#endif
#if defined BCM6858
	{ "VPORT_CFG_TABLE", 1, CORE_4_INDEX, &VPORT_CFG_TABLE_4, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "DSCP_TO_PBITS_MAP_TABLE", 1, CORE_4_INDEX, &DSCP_TO_PBITS_MAP_TABLE_4, 4, 64, 1 },
#endif
#if defined BCM6858
	{ "TCAM_IC_CMD_TABLE", 1, CORE_4_INDEX, &TCAM_IC_CMD_TABLE_4, 9, 16, 1 },
#endif
#if defined BCM6858
	{ "MCAST_PROCESSING_TASKS_REPLY", 1, CORE_4_INDEX, &MCAST_PROCESSING_TASKS_REPLY_4, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "PBIT_TO_GEM_TABLE", 1, CORE_4_INDEX, &PBIT_TO_GEM_TABLE_4, 16, 8, 1 },
#endif
#if defined BCM6858
	{ "TX_FLOW_TABLE", 1, CORE_4_INDEX, &TX_FLOW_TABLE_4, 192, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_PROCESSING_TASKS_LOCK", 1, CORE_4_INDEX, &MCAST_PROCESSING_TASKS_LOCK_4, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "TC_TO_QUEUE_TABLE", 1, CORE_4_INDEX, &TC_TO_QUEUE_TABLE_4, 65, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_FPM_TOKENS_READ_FLAG", 1, CORE_4_INDEX, &MCAST_FPM_TOKENS_READ_FLAG_4, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "VPORT_TO_DSCP_TO_PBITS_TABLE", 1, CORE_4_INDEX, &VPORT_TO_DSCP_TO_PBITS_TABLE_4, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_4_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_4, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_BBH_OVERRUN_TASKS_LIMIT", 1, CORE_4_INDEX, &MCAST_BBH_OVERRUN_TASKS_LIMIT_4, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "LAYER2_HEADER_COPY_MAPPING_TABLE", 1, CORE_4_INDEX, &LAYER2_HEADER_COPY_MAPPING_TABLE_4, 30, 1, 1 },
#endif
#if defined BCM6858
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_4_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "EMAC_FLOW_CTRL", 1, CORE_4_INDEX, &EMAC_FLOW_CTRL_4, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "POLICER_PARAMS_TABLE", 1, CORE_4_INDEX, &POLICER_PARAMS_TABLE_4, 80, 1, 1 },
#endif
#if defined BCM6858
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_4_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_4, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "FLOODING_HEADER_FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_4_INDEX, &FLOODING_HEADER_FPM_POOL_NUMBER_MAPPING_TABLE_4, 32, 1, 1 },
#endif
#if defined BCM6858
	{ "PBIT_TO_QUEUE_TABLE", 1, CORE_4_INDEX, &PBIT_TO_QUEUE_TABLE_4, 65, 1, 1 },
#endif
#if defined BCM6858
	{ "DEBUG_SCRATCHPAD", 1, CORE_4_INDEX, &DEBUG_SCRATCHPAD_4, 10, 1, 1 },
#endif
#if defined BCM6858
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_4_INDEX, &LOOPBACK_QUEUE_TABLE_4, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "GLOBAL_DSCP_TO_PBITS_TABLE", 1, CORE_4_INDEX, &GLOBAL_DSCP_TO_PBITS_TABLE_4, 1, 64, 1 },
#endif
#if defined BCM6858
	{ "VPORT_CFG_EX_TABLE", 1, CORE_4_INDEX, &VPORT_CFG_EX_TABLE_4, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "PORT_MAC", 1, CORE_4_INDEX, &PORT_MAC_4, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_4_INDEX, &INGRESS_FILTER_PROFILE_TABLE_4, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "DUAL_STACK_LITE_TABLE", 1, CORE_4_INDEX, &DUAL_STACK_LITE_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "QUEUE_THRESHOLD_VECTOR", 1, CORE_4_INDEX, &QUEUE_THRESHOLD_VECTOR_4, 9, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_PACKET_BASED_MAPPING", 1, CORE_4_INDEX, &INGRESS_PACKET_BASED_MAPPING_4, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "PROCESSING_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_4_INDEX, &PROCESSING_QUEUE_DYNAMIC_MNG_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SPDSVC_ANALYZER_PARAMS_TABLE", 1, CORE_4_INDEX, &SPDSVC_ANALYZER_PARAMS_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_4_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_4, 128, 1, 1 },
#endif
#if defined BCM6858
	{ "DBG_DUMP_TABLE", 1, CORE_4_INDEX, &DBG_DUMP_TABLE_4, 128, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_TBL_CFG", 1, CORE_4_INDEX, &NATC_TBL_CFG_4, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_COUNTERS_GENERIC_CONTEXT", 1, CORE_4_INDEX, &NATC_COUNTERS_GENERIC_CONTEXT_4, 128, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_FLOW_CONTEXT_DDR_ADDR", 1, CORE_4_INDEX, &RX_FLOW_CONTEXT_DDR_ADDR_4, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "BRIDGE_CFG_TABLE", 1, CORE_4_INDEX, &BRIDGE_CFG_TABLE_4, 2, 1, 1 },
#endif
#if defined BCM6858
	{ "VPORT_PBIT_TO_DISCARD_PRIO_VECTOR", 1, CORE_4_INDEX, &VPORT_PBIT_TO_DISCARD_PRIO_VECTOR_4, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_4_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_4, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "MAX_PKT_LEN_TABLE", 1, CORE_4_INDEX, &MAX_PKT_LEN_TABLE_4, 32, 1, 1 },
#endif
#if defined BCM6858
	{ "REGISTERS_BUFFER", 1, CORE_4_INDEX, &REGISTERS_BUFFER_4, 32, 1, 1 },
#endif
#if defined BCM6858
	{ "TCAM_IC_CFG_TABLE", 1, CORE_4_INDEX, &TCAM_IC_CFG_TABLE_4, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "VLAN_ACTION_GPE_HANDLER_PTR_TABLE", 1, CORE_4_INDEX, &VLAN_ACTION_GPE_HANDLER_PTR_TABLE_4, 17, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_BBH_OVERRUN_MAX_TASKS_LIMIT", 1, CORE_4_INDEX, &MCAST_BBH_OVERRUN_MAX_TASKS_LIMIT_4, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SPDTEST_GEN_PARAM", 1, CORE_4_INDEX, &SPDTEST_GEN_PARAM_4, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "NULL_BUFFER", 1, CORE_4_INDEX, &NULL_BUFFER_4, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPTV_CONFIGURATION_TABLE", 1, CORE_4_INDEX, &IPTV_CONFIGURATION_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "ZERO_VALUE", 1, CORE_4_INDEX, &ZERO_VALUE_4, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TUNNEL_TABLE", 1, CORE_4_INDEX, &TUNNEL_TABLE_4, 2, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_FEED_RING_DESCRIPTOR_TABLE", 1, CORE_4_INDEX, &CPU_FEED_RING_DESCRIPTOR_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEBUG_PRINT_TABLE", 1, CORE_4_INDEX, &DEBUG_PRINT_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "FPM_GLOBAL_CFG", 1, CORE_4_INDEX, &FPM_GLOBAL_CFG_4, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "ONE_VALUE", 1, CORE_4_INDEX, &ONE_VALUE_4, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_FPM_THRESHOLDS", 1, CORE_4_INDEX, &DHD_FPM_THRESHOLDS_4, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_BBH_OVERRUN_MIN_TASKS_LIMIT", 1, CORE_4_INDEX, &MCAST_BBH_OVERRUN_MIN_TASKS_LIMIT_4, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SYSTEM_CONFIGURATION", 1, CORE_4_INDEX, &SYSTEM_CONFIGURATION_4, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_4_INDEX, &LOOPBACK_WAN_FLOW_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "FORCE_DSCP", 1, CORE_4_INDEX, &FORCE_DSCP_4, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CORE_ID_TABLE", 1, CORE_4_INDEX, &CORE_ID_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TUNNEL_HEADER_PSRAM_BUFFER", 1, CORE_4_INDEX, &TUNNEL_HEADER_PSRAM_BUFFER_4, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TASK_IDX", 1, CORE_4_INDEX, &TASK_IDX_4, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TUNNELS_PARSING_CFG", 1, CORE_4_INDEX, &TUNNELS_PARSING_CFG_4, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPTV_CFG_TABLE", 1, CORE_4_INDEX, &IPTV_CFG_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_4_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_4, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEFAULT_VLAN_VID_TABLE", 1, CORE_4_INDEX, &DEFAULT_VLAN_VID_TABLE_4, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "NAT_CACHE_CFG", 1, CORE_4_INDEX, &NAT_CACHE_CFG_4, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SRAM_DUMMY_STORE", 1, CORE_4_INDEX, &SRAM_DUMMY_STORE_4, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_4_INDEX, &NAT_CACHE_KEY0_MASK_4, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_4_INDEX, &RX_MIRRORING_CONFIGURATION_4, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RATE_LIMIT_OVERHEAD", 1, CORE_4_INDEX, &RATE_LIMIT_OVERHEAD_4, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_QOS_DONT_DROP_RATIO", 1, CORE_4_INDEX, &INGRESS_QOS_DONT_DROP_RATIO_4, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_4_INDEX, &NATC_L2_VLAN_KEY_MASK_4, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_QOS_WAN_UNTAGGED_PRIORITY", 1, CORE_4_INDEX, &INGRESS_QOS_WAN_UNTAGGED_PRIORITY_4, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IC_MCAST_ENABLE", 1, CORE_4_INDEX, &IC_MCAST_ENABLE_4, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "ECN_IPV6_REMARK_TABLE", 1, CORE_4_INDEX, &ECN_IPV6_REMARK_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_REDIRECT_MODE", 1, CORE_4_INDEX, &CPU_REDIRECT_MODE_4, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_4_INDEX, &DEBUG_PRINT_CORE_LOCK_4, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_4_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_FILTER_CFG", 1, CORE_4_INDEX, &INGRESS_FILTER_CFG_4, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_4_INDEX, &INGRESS_FILTER_1588_CFG_4, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_4_INDEX, &RX_MIRRORING_DIRECT_ENABLE_4, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_L2_TOS_MASK", 1, CORE_4_INDEX, &NATC_L2_TOS_MASK_4, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEFAULT_VLAN_VID_TO_TX_VLAN_CNTR_TABLE", 1, CORE_4_INDEX, &DEFAULT_VLAN_VID_TO_TX_VLAN_CNTR_TABLE_4, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "DS_PACKET_BUFFER", 1, CORE_5_INDEX, &DS_PACKET_BUFFER_5, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_FLOW_TABLE", 1, CORE_5_INDEX, &RX_FLOW_TABLE_5, 320, 1, 1 },
#endif
#if defined BCM6858
	{ "VPORT_CFG_TABLE", 1, CORE_5_INDEX, &VPORT_CFG_TABLE_5, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "DSCP_TO_PBITS_MAP_TABLE", 1, CORE_5_INDEX, &DSCP_TO_PBITS_MAP_TABLE_5, 4, 64, 1 },
#endif
#if defined BCM6858
	{ "TCAM_IC_CMD_TABLE", 1, CORE_5_INDEX, &TCAM_IC_CMD_TABLE_5, 9, 16, 1 },
#endif
#if defined BCM6858
	{ "MCAST_PROCESSING_TASKS_REPLY", 1, CORE_5_INDEX, &MCAST_PROCESSING_TASKS_REPLY_5, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "PBIT_TO_GEM_TABLE", 1, CORE_5_INDEX, &PBIT_TO_GEM_TABLE_5, 16, 8, 1 },
#endif
#if defined BCM6858
	{ "TX_FLOW_TABLE", 1, CORE_5_INDEX, &TX_FLOW_TABLE_5, 192, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_PROCESSING_TASKS_LOCK", 1, CORE_5_INDEX, &MCAST_PROCESSING_TASKS_LOCK_5, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "TC_TO_QUEUE_TABLE", 1, CORE_5_INDEX, &TC_TO_QUEUE_TABLE_5, 65, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_FPM_TOKENS_READ_FLAG", 1, CORE_5_INDEX, &MCAST_FPM_TOKENS_READ_FLAG_5, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "VPORT_TO_DSCP_TO_PBITS_TABLE", 1, CORE_5_INDEX, &VPORT_TO_DSCP_TO_PBITS_TABLE_5, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_5_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_5, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_BBH_OVERRUN_TASKS_LIMIT", 1, CORE_5_INDEX, &MCAST_BBH_OVERRUN_TASKS_LIMIT_5, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "LAYER2_HEADER_COPY_MAPPING_TABLE", 1, CORE_5_INDEX, &LAYER2_HEADER_COPY_MAPPING_TABLE_5, 30, 1, 1 },
#endif
#if defined BCM6858
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_5_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "EMAC_FLOW_CTRL", 1, CORE_5_INDEX, &EMAC_FLOW_CTRL_5, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "POLICER_PARAMS_TABLE", 1, CORE_5_INDEX, &POLICER_PARAMS_TABLE_5, 80, 1, 1 },
#endif
#if defined BCM6858
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_5_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_5, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "FLOODING_HEADER_FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_5_INDEX, &FLOODING_HEADER_FPM_POOL_NUMBER_MAPPING_TABLE_5, 32, 1, 1 },
#endif
#if defined BCM6858
	{ "PBIT_TO_QUEUE_TABLE", 1, CORE_5_INDEX, &PBIT_TO_QUEUE_TABLE_5, 65, 1, 1 },
#endif
#if defined BCM6858
	{ "DEBUG_SCRATCHPAD", 1, CORE_5_INDEX, &DEBUG_SCRATCHPAD_5, 10, 1, 1 },
#endif
#if defined BCM6858
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_5_INDEX, &LOOPBACK_QUEUE_TABLE_5, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "GLOBAL_DSCP_TO_PBITS_TABLE", 1, CORE_5_INDEX, &GLOBAL_DSCP_TO_PBITS_TABLE_5, 1, 64, 1 },
#endif
#if defined BCM6858
	{ "VPORT_CFG_EX_TABLE", 1, CORE_5_INDEX, &VPORT_CFG_EX_TABLE_5, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "PORT_MAC", 1, CORE_5_INDEX, &PORT_MAC_5, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_5_INDEX, &INGRESS_FILTER_PROFILE_TABLE_5, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "DUAL_STACK_LITE_TABLE", 1, CORE_5_INDEX, &DUAL_STACK_LITE_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "QUEUE_THRESHOLD_VECTOR", 1, CORE_5_INDEX, &QUEUE_THRESHOLD_VECTOR_5, 9, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_PACKET_BASED_MAPPING", 1, CORE_5_INDEX, &INGRESS_PACKET_BASED_MAPPING_5, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "PROCESSING_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_5_INDEX, &PROCESSING_QUEUE_DYNAMIC_MNG_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SPDSVC_ANALYZER_PARAMS_TABLE", 1, CORE_5_INDEX, &SPDSVC_ANALYZER_PARAMS_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_5_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_5, 128, 1, 1 },
#endif
#if defined BCM6858
	{ "DBG_DUMP_TABLE", 1, CORE_5_INDEX, &DBG_DUMP_TABLE_5, 128, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_TBL_CFG", 1, CORE_5_INDEX, &NATC_TBL_CFG_5, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_COUNTERS_GENERIC_CONTEXT", 1, CORE_5_INDEX, &NATC_COUNTERS_GENERIC_CONTEXT_5, 128, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_FLOW_CONTEXT_DDR_ADDR", 1, CORE_5_INDEX, &RX_FLOW_CONTEXT_DDR_ADDR_5, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "BRIDGE_CFG_TABLE", 1, CORE_5_INDEX, &BRIDGE_CFG_TABLE_5, 2, 1, 1 },
#endif
#if defined BCM6858
	{ "VPORT_PBIT_TO_DISCARD_PRIO_VECTOR", 1, CORE_5_INDEX, &VPORT_PBIT_TO_DISCARD_PRIO_VECTOR_5, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_5_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_5, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "MAX_PKT_LEN_TABLE", 1, CORE_5_INDEX, &MAX_PKT_LEN_TABLE_5, 32, 1, 1 },
#endif
#if defined BCM6858
	{ "REGISTERS_BUFFER", 1, CORE_5_INDEX, &REGISTERS_BUFFER_5, 32, 1, 1 },
#endif
#if defined BCM6858
	{ "TCAM_IC_CFG_TABLE", 1, CORE_5_INDEX, &TCAM_IC_CFG_TABLE_5, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "VLAN_ACTION_GPE_HANDLER_PTR_TABLE", 1, CORE_5_INDEX, &VLAN_ACTION_GPE_HANDLER_PTR_TABLE_5, 17, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_BBH_OVERRUN_MAX_TASKS_LIMIT", 1, CORE_5_INDEX, &MCAST_BBH_OVERRUN_MAX_TASKS_LIMIT_5, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SPDTEST_GEN_PARAM", 1, CORE_5_INDEX, &SPDTEST_GEN_PARAM_5, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "NULL_BUFFER", 1, CORE_5_INDEX, &NULL_BUFFER_5, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPTV_CONFIGURATION_TABLE", 1, CORE_5_INDEX, &IPTV_CONFIGURATION_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "ZERO_VALUE", 1, CORE_5_INDEX, &ZERO_VALUE_5, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TUNNEL_TABLE", 1, CORE_5_INDEX, &TUNNEL_TABLE_5, 2, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_FEED_RING_DESCRIPTOR_TABLE", 1, CORE_5_INDEX, &CPU_FEED_RING_DESCRIPTOR_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEBUG_PRINT_TABLE", 1, CORE_5_INDEX, &DEBUG_PRINT_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "FPM_GLOBAL_CFG", 1, CORE_5_INDEX, &FPM_GLOBAL_CFG_5, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "ONE_VALUE", 1, CORE_5_INDEX, &ONE_VALUE_5, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_FPM_THRESHOLDS", 1, CORE_5_INDEX, &DHD_FPM_THRESHOLDS_5, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_BBH_OVERRUN_MIN_TASKS_LIMIT", 1, CORE_5_INDEX, &MCAST_BBH_OVERRUN_MIN_TASKS_LIMIT_5, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SYSTEM_CONFIGURATION", 1, CORE_5_INDEX, &SYSTEM_CONFIGURATION_5, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_5_INDEX, &LOOPBACK_WAN_FLOW_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "FORCE_DSCP", 1, CORE_5_INDEX, &FORCE_DSCP_5, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CORE_ID_TABLE", 1, CORE_5_INDEX, &CORE_ID_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TUNNEL_HEADER_PSRAM_BUFFER", 1, CORE_5_INDEX, &TUNNEL_HEADER_PSRAM_BUFFER_5, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TASK_IDX", 1, CORE_5_INDEX, &TASK_IDX_5, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TUNNELS_PARSING_CFG", 1, CORE_5_INDEX, &TUNNELS_PARSING_CFG_5, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPTV_CFG_TABLE", 1, CORE_5_INDEX, &IPTV_CFG_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_5_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_5, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEFAULT_VLAN_VID_TABLE", 1, CORE_5_INDEX, &DEFAULT_VLAN_VID_TABLE_5, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "NAT_CACHE_CFG", 1, CORE_5_INDEX, &NAT_CACHE_CFG_5, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SRAM_DUMMY_STORE", 1, CORE_5_INDEX, &SRAM_DUMMY_STORE_5, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_5_INDEX, &NAT_CACHE_KEY0_MASK_5, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_5_INDEX, &RX_MIRRORING_CONFIGURATION_5, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RATE_LIMIT_OVERHEAD", 1, CORE_5_INDEX, &RATE_LIMIT_OVERHEAD_5, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_QOS_DONT_DROP_RATIO", 1, CORE_5_INDEX, &INGRESS_QOS_DONT_DROP_RATIO_5, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_5_INDEX, &NATC_L2_VLAN_KEY_MASK_5, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_QOS_WAN_UNTAGGED_PRIORITY", 1, CORE_5_INDEX, &INGRESS_QOS_WAN_UNTAGGED_PRIORITY_5, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IC_MCAST_ENABLE", 1, CORE_5_INDEX, &IC_MCAST_ENABLE_5, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "ECN_IPV6_REMARK_TABLE", 1, CORE_5_INDEX, &ECN_IPV6_REMARK_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_REDIRECT_MODE", 1, CORE_5_INDEX, &CPU_REDIRECT_MODE_5, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_5_INDEX, &DEBUG_PRINT_CORE_LOCK_5, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_5_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_FILTER_CFG", 1, CORE_5_INDEX, &INGRESS_FILTER_CFG_5, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_5_INDEX, &INGRESS_FILTER_1588_CFG_5, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_5_INDEX, &RX_MIRRORING_DIRECT_ENABLE_5, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_L2_TOS_MASK", 1, CORE_5_INDEX, &NATC_L2_TOS_MASK_5, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEFAULT_VLAN_VID_TO_TX_VLAN_CNTR_TABLE", 1, CORE_5_INDEX, &DEFAULT_VLAN_VID_TO_TX_VLAN_CNTR_TABLE_5, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "DS_PACKET_BUFFER", 1, CORE_6_INDEX, &DS_PACKET_BUFFER_6, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_FLOW_TABLE", 1, CORE_6_INDEX, &RX_FLOW_TABLE_6, 320, 1, 1 },
#endif
#if defined BCM6858
	{ "VPORT_CFG_TABLE", 1, CORE_6_INDEX, &VPORT_CFG_TABLE_6, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "DSCP_TO_PBITS_MAP_TABLE", 1, CORE_6_INDEX, &DSCP_TO_PBITS_MAP_TABLE_6, 4, 64, 1 },
#endif
#if defined BCM6858
	{ "TCAM_IC_CMD_TABLE", 1, CORE_6_INDEX, &TCAM_IC_CMD_TABLE_6, 9, 16, 1 },
#endif
#if defined BCM6858
	{ "MCAST_PROCESSING_TASKS_REPLY", 1, CORE_6_INDEX, &MCAST_PROCESSING_TASKS_REPLY_6, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "PBIT_TO_GEM_TABLE", 1, CORE_6_INDEX, &PBIT_TO_GEM_TABLE_6, 16, 8, 1 },
#endif
#if defined BCM6858
	{ "TX_FLOW_TABLE", 1, CORE_6_INDEX, &TX_FLOW_TABLE_6, 192, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_PROCESSING_TASKS_LOCK", 1, CORE_6_INDEX, &MCAST_PROCESSING_TASKS_LOCK_6, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "TC_TO_QUEUE_TABLE", 1, CORE_6_INDEX, &TC_TO_QUEUE_TABLE_6, 65, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_FPM_TOKENS_READ_FLAG", 1, CORE_6_INDEX, &MCAST_FPM_TOKENS_READ_FLAG_6, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "VPORT_TO_DSCP_TO_PBITS_TABLE", 1, CORE_6_INDEX, &VPORT_TO_DSCP_TO_PBITS_TABLE_6, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_6_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_6, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_BBH_OVERRUN_TASKS_LIMIT", 1, CORE_6_INDEX, &MCAST_BBH_OVERRUN_TASKS_LIMIT_6, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "LAYER2_HEADER_COPY_MAPPING_TABLE", 1, CORE_6_INDEX, &LAYER2_HEADER_COPY_MAPPING_TABLE_6, 30, 1, 1 },
#endif
#if defined BCM6858
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_6_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "EMAC_FLOW_CTRL", 1, CORE_6_INDEX, &EMAC_FLOW_CTRL_6, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "POLICER_PARAMS_TABLE", 1, CORE_6_INDEX, &POLICER_PARAMS_TABLE_6, 80, 1, 1 },
#endif
#if defined BCM6858
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_6_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_6, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "FLOODING_HEADER_FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_6_INDEX, &FLOODING_HEADER_FPM_POOL_NUMBER_MAPPING_TABLE_6, 32, 1, 1 },
#endif
#if defined BCM6858
	{ "PBIT_TO_QUEUE_TABLE", 1, CORE_6_INDEX, &PBIT_TO_QUEUE_TABLE_6, 65, 1, 1 },
#endif
#if defined BCM6858
	{ "DEBUG_SCRATCHPAD", 1, CORE_6_INDEX, &DEBUG_SCRATCHPAD_6, 10, 1, 1 },
#endif
#if defined BCM6858
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_6_INDEX, &LOOPBACK_QUEUE_TABLE_6, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "GLOBAL_DSCP_TO_PBITS_TABLE", 1, CORE_6_INDEX, &GLOBAL_DSCP_TO_PBITS_TABLE_6, 1, 64, 1 },
#endif
#if defined BCM6858
	{ "VPORT_CFG_EX_TABLE", 1, CORE_6_INDEX, &VPORT_CFG_EX_TABLE_6, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "PORT_MAC", 1, CORE_6_INDEX, &PORT_MAC_6, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_6_INDEX, &INGRESS_FILTER_PROFILE_TABLE_6, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "DUAL_STACK_LITE_TABLE", 1, CORE_6_INDEX, &DUAL_STACK_LITE_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "QUEUE_THRESHOLD_VECTOR", 1, CORE_6_INDEX, &QUEUE_THRESHOLD_VECTOR_6, 9, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_PACKET_BASED_MAPPING", 1, CORE_6_INDEX, &INGRESS_PACKET_BASED_MAPPING_6, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "PROCESSING_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_6_INDEX, &PROCESSING_QUEUE_DYNAMIC_MNG_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SPDSVC_ANALYZER_PARAMS_TABLE", 1, CORE_6_INDEX, &SPDSVC_ANALYZER_PARAMS_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_6_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_6, 128, 1, 1 },
#endif
#if defined BCM6858
	{ "DBG_DUMP_TABLE", 1, CORE_6_INDEX, &DBG_DUMP_TABLE_6, 128, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_TBL_CFG", 1, CORE_6_INDEX, &NATC_TBL_CFG_6, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_COUNTERS_GENERIC_CONTEXT", 1, CORE_6_INDEX, &NATC_COUNTERS_GENERIC_CONTEXT_6, 128, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_FLOW_CONTEXT_DDR_ADDR", 1, CORE_6_INDEX, &RX_FLOW_CONTEXT_DDR_ADDR_6, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "BRIDGE_CFG_TABLE", 1, CORE_6_INDEX, &BRIDGE_CFG_TABLE_6, 2, 1, 1 },
#endif
#if defined BCM6858
	{ "VPORT_PBIT_TO_DISCARD_PRIO_VECTOR", 1, CORE_6_INDEX, &VPORT_PBIT_TO_DISCARD_PRIO_VECTOR_6, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_6_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_6, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "MAX_PKT_LEN_TABLE", 1, CORE_6_INDEX, &MAX_PKT_LEN_TABLE_6, 32, 1, 1 },
#endif
#if defined BCM6858
	{ "REGISTERS_BUFFER", 1, CORE_6_INDEX, &REGISTERS_BUFFER_6, 32, 1, 1 },
#endif
#if defined BCM6858
	{ "TCAM_IC_CFG_TABLE", 1, CORE_6_INDEX, &TCAM_IC_CFG_TABLE_6, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "VLAN_ACTION_GPE_HANDLER_PTR_TABLE", 1, CORE_6_INDEX, &VLAN_ACTION_GPE_HANDLER_PTR_TABLE_6, 17, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_BBH_OVERRUN_MAX_TASKS_LIMIT", 1, CORE_6_INDEX, &MCAST_BBH_OVERRUN_MAX_TASKS_LIMIT_6, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SPDTEST_GEN_PARAM", 1, CORE_6_INDEX, &SPDTEST_GEN_PARAM_6, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "NULL_BUFFER", 1, CORE_6_INDEX, &NULL_BUFFER_6, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPTV_CONFIGURATION_TABLE", 1, CORE_6_INDEX, &IPTV_CONFIGURATION_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "ZERO_VALUE", 1, CORE_6_INDEX, &ZERO_VALUE_6, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TUNNEL_TABLE", 1, CORE_6_INDEX, &TUNNEL_TABLE_6, 2, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_FEED_RING_DESCRIPTOR_TABLE", 1, CORE_6_INDEX, &CPU_FEED_RING_DESCRIPTOR_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEBUG_PRINT_TABLE", 1, CORE_6_INDEX, &DEBUG_PRINT_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "FPM_GLOBAL_CFG", 1, CORE_6_INDEX, &FPM_GLOBAL_CFG_6, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "ONE_VALUE", 1, CORE_6_INDEX, &ONE_VALUE_6, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_FPM_THRESHOLDS", 1, CORE_6_INDEX, &DHD_FPM_THRESHOLDS_6, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_BBH_OVERRUN_MIN_TASKS_LIMIT", 1, CORE_6_INDEX, &MCAST_BBH_OVERRUN_MIN_TASKS_LIMIT_6, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SYSTEM_CONFIGURATION", 1, CORE_6_INDEX, &SYSTEM_CONFIGURATION_6, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_6_INDEX, &LOOPBACK_WAN_FLOW_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "FORCE_DSCP", 1, CORE_6_INDEX, &FORCE_DSCP_6, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CORE_ID_TABLE", 1, CORE_6_INDEX, &CORE_ID_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TUNNEL_HEADER_PSRAM_BUFFER", 1, CORE_6_INDEX, &TUNNEL_HEADER_PSRAM_BUFFER_6, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TASK_IDX", 1, CORE_6_INDEX, &TASK_IDX_6, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TUNNELS_PARSING_CFG", 1, CORE_6_INDEX, &TUNNELS_PARSING_CFG_6, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPTV_CFG_TABLE", 1, CORE_6_INDEX, &IPTV_CFG_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_6_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_6, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEFAULT_VLAN_VID_TABLE", 1, CORE_6_INDEX, &DEFAULT_VLAN_VID_TABLE_6, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "NAT_CACHE_CFG", 1, CORE_6_INDEX, &NAT_CACHE_CFG_6, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SRAM_DUMMY_STORE", 1, CORE_6_INDEX, &SRAM_DUMMY_STORE_6, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_6_INDEX, &NAT_CACHE_KEY0_MASK_6, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_6_INDEX, &RX_MIRRORING_CONFIGURATION_6, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RATE_LIMIT_OVERHEAD", 1, CORE_6_INDEX, &RATE_LIMIT_OVERHEAD_6, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_QOS_DONT_DROP_RATIO", 1, CORE_6_INDEX, &INGRESS_QOS_DONT_DROP_RATIO_6, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_6_INDEX, &NATC_L2_VLAN_KEY_MASK_6, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_QOS_WAN_UNTAGGED_PRIORITY", 1, CORE_6_INDEX, &INGRESS_QOS_WAN_UNTAGGED_PRIORITY_6, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IC_MCAST_ENABLE", 1, CORE_6_INDEX, &IC_MCAST_ENABLE_6, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "ECN_IPV6_REMARK_TABLE", 1, CORE_6_INDEX, &ECN_IPV6_REMARK_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_REDIRECT_MODE", 1, CORE_6_INDEX, &CPU_REDIRECT_MODE_6, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_6_INDEX, &DEBUG_PRINT_CORE_LOCK_6, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_6_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_FILTER_CFG", 1, CORE_6_INDEX, &INGRESS_FILTER_CFG_6, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_6_INDEX, &INGRESS_FILTER_1588_CFG_6, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_6_INDEX, &RX_MIRRORING_DIRECT_ENABLE_6, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_L2_TOS_MASK", 1, CORE_6_INDEX, &NATC_L2_TOS_MASK_6, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEFAULT_VLAN_VID_TO_TX_VLAN_CNTR_TABLE", 1, CORE_6_INDEX, &DEFAULT_VLAN_VID_TO_TX_VLAN_CNTR_TABLE_6, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "DS_PACKET_BUFFER", 1, CORE_7_INDEX, &DS_PACKET_BUFFER_7, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_FLOW_TABLE", 1, CORE_7_INDEX, &RX_FLOW_TABLE_7, 320, 1, 1 },
#endif
#if defined BCM6858
	{ "VPORT_CFG_TABLE", 1, CORE_7_INDEX, &VPORT_CFG_TABLE_7, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "DSCP_TO_PBITS_MAP_TABLE", 1, CORE_7_INDEX, &DSCP_TO_PBITS_MAP_TABLE_7, 4, 64, 1 },
#endif
#if defined BCM6858
	{ "TCAM_IC_CMD_TABLE", 1, CORE_7_INDEX, &TCAM_IC_CMD_TABLE_7, 9, 16, 1 },
#endif
#if defined BCM6858
	{ "MCAST_PROCESSING_TASKS_REPLY", 1, CORE_7_INDEX, &MCAST_PROCESSING_TASKS_REPLY_7, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "PBIT_TO_GEM_TABLE", 1, CORE_7_INDEX, &PBIT_TO_GEM_TABLE_7, 16, 8, 1 },
#endif
#if defined BCM6858
	{ "TX_FLOW_TABLE", 1, CORE_7_INDEX, &TX_FLOW_TABLE_7, 192, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_PROCESSING_TASKS_LOCK", 1, CORE_7_INDEX, &MCAST_PROCESSING_TASKS_LOCK_7, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "TC_TO_QUEUE_TABLE", 1, CORE_7_INDEX, &TC_TO_QUEUE_TABLE_7, 65, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_FPM_TOKENS_READ_FLAG", 1, CORE_7_INDEX, &MCAST_FPM_TOKENS_READ_FLAG_7, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "VPORT_TO_DSCP_TO_PBITS_TABLE", 1, CORE_7_INDEX, &VPORT_TO_DSCP_TO_PBITS_TABLE_7, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_7_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_7, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_BBH_OVERRUN_TASKS_LIMIT", 1, CORE_7_INDEX, &MCAST_BBH_OVERRUN_TASKS_LIMIT_7, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "LAYER2_HEADER_COPY_MAPPING_TABLE", 1, CORE_7_INDEX, &LAYER2_HEADER_COPY_MAPPING_TABLE_7, 30, 1, 1 },
#endif
#if defined BCM6858
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_7_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "EMAC_FLOW_CTRL", 1, CORE_7_INDEX, &EMAC_FLOW_CTRL_7, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "POLICER_PARAMS_TABLE", 1, CORE_7_INDEX, &POLICER_PARAMS_TABLE_7, 80, 1, 1 },
#endif
#if defined BCM6858
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_7_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_7, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "FLOODING_HEADER_FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_7_INDEX, &FLOODING_HEADER_FPM_POOL_NUMBER_MAPPING_TABLE_7, 32, 1, 1 },
#endif
#if defined BCM6858
	{ "PBIT_TO_QUEUE_TABLE", 1, CORE_7_INDEX, &PBIT_TO_QUEUE_TABLE_7, 65, 1, 1 },
#endif
#if defined BCM6858
	{ "DEBUG_SCRATCHPAD", 1, CORE_7_INDEX, &DEBUG_SCRATCHPAD_7, 10, 1, 1 },
#endif
#if defined BCM6858
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_7_INDEX, &LOOPBACK_QUEUE_TABLE_7, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "GLOBAL_DSCP_TO_PBITS_TABLE", 1, CORE_7_INDEX, &GLOBAL_DSCP_TO_PBITS_TABLE_7, 1, 64, 1 },
#endif
#if defined BCM6858
	{ "VPORT_CFG_EX_TABLE", 1, CORE_7_INDEX, &VPORT_CFG_EX_TABLE_7, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "PORT_MAC", 1, CORE_7_INDEX, &PORT_MAC_7, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_7_INDEX, &INGRESS_FILTER_PROFILE_TABLE_7, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "DUAL_STACK_LITE_TABLE", 1, CORE_7_INDEX, &DUAL_STACK_LITE_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "QUEUE_THRESHOLD_VECTOR", 1, CORE_7_INDEX, &QUEUE_THRESHOLD_VECTOR_7, 9, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_PACKET_BASED_MAPPING", 1, CORE_7_INDEX, &INGRESS_PACKET_BASED_MAPPING_7, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "PROCESSING_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_7_INDEX, &PROCESSING_QUEUE_DYNAMIC_MNG_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SPDSVC_ANALYZER_PARAMS_TABLE", 1, CORE_7_INDEX, &SPDSVC_ANALYZER_PARAMS_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_7_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_7, 128, 1, 1 },
#endif
#if defined BCM6858
	{ "DBG_DUMP_TABLE", 1, CORE_7_INDEX, &DBG_DUMP_TABLE_7, 128, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_TBL_CFG", 1, CORE_7_INDEX, &NATC_TBL_CFG_7, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_COUNTERS_GENERIC_CONTEXT", 1, CORE_7_INDEX, &NATC_COUNTERS_GENERIC_CONTEXT_7, 128, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_FLOW_CONTEXT_DDR_ADDR", 1, CORE_7_INDEX, &RX_FLOW_CONTEXT_DDR_ADDR_7, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "BRIDGE_CFG_TABLE", 1, CORE_7_INDEX, &BRIDGE_CFG_TABLE_7, 2, 1, 1 },
#endif
#if defined BCM6858
	{ "VPORT_PBIT_TO_DISCARD_PRIO_VECTOR", 1, CORE_7_INDEX, &VPORT_PBIT_TO_DISCARD_PRIO_VECTOR_7, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_7_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_7, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "MAX_PKT_LEN_TABLE", 1, CORE_7_INDEX, &MAX_PKT_LEN_TABLE_7, 32, 1, 1 },
#endif
#if defined BCM6858
	{ "REGISTERS_BUFFER", 1, CORE_7_INDEX, &REGISTERS_BUFFER_7, 32, 1, 1 },
#endif
#if defined BCM6858
	{ "TCAM_IC_CFG_TABLE", 1, CORE_7_INDEX, &TCAM_IC_CFG_TABLE_7, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "VLAN_ACTION_GPE_HANDLER_PTR_TABLE", 1, CORE_7_INDEX, &VLAN_ACTION_GPE_HANDLER_PTR_TABLE_7, 17, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_BBH_OVERRUN_MAX_TASKS_LIMIT", 1, CORE_7_INDEX, &MCAST_BBH_OVERRUN_MAX_TASKS_LIMIT_7, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SPDTEST_GEN_PARAM", 1, CORE_7_INDEX, &SPDTEST_GEN_PARAM_7, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "NULL_BUFFER", 1, CORE_7_INDEX, &NULL_BUFFER_7, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPTV_CONFIGURATION_TABLE", 1, CORE_7_INDEX, &IPTV_CONFIGURATION_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "ZERO_VALUE", 1, CORE_7_INDEX, &ZERO_VALUE_7, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TUNNEL_TABLE", 1, CORE_7_INDEX, &TUNNEL_TABLE_7, 2, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_FEED_RING_DESCRIPTOR_TABLE", 1, CORE_7_INDEX, &CPU_FEED_RING_DESCRIPTOR_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEBUG_PRINT_TABLE", 1, CORE_7_INDEX, &DEBUG_PRINT_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "FPM_GLOBAL_CFG", 1, CORE_7_INDEX, &FPM_GLOBAL_CFG_7, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "ONE_VALUE", 1, CORE_7_INDEX, &ONE_VALUE_7, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_FPM_THRESHOLDS", 1, CORE_7_INDEX, &DHD_FPM_THRESHOLDS_7, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_BBH_OVERRUN_MIN_TASKS_LIMIT", 1, CORE_7_INDEX, &MCAST_BBH_OVERRUN_MIN_TASKS_LIMIT_7, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SYSTEM_CONFIGURATION", 1, CORE_7_INDEX, &SYSTEM_CONFIGURATION_7, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_7_INDEX, &LOOPBACK_WAN_FLOW_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "FORCE_DSCP", 1, CORE_7_INDEX, &FORCE_DSCP_7, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CORE_ID_TABLE", 1, CORE_7_INDEX, &CORE_ID_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TUNNEL_HEADER_PSRAM_BUFFER", 1, CORE_7_INDEX, &TUNNEL_HEADER_PSRAM_BUFFER_7, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TASK_IDX", 1, CORE_7_INDEX, &TASK_IDX_7, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TUNNELS_PARSING_CFG", 1, CORE_7_INDEX, &TUNNELS_PARSING_CFG_7, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPTV_CFG_TABLE", 1, CORE_7_INDEX, &IPTV_CFG_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_7_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_7, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEFAULT_VLAN_VID_TABLE", 1, CORE_7_INDEX, &DEFAULT_VLAN_VID_TABLE_7, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "NAT_CACHE_CFG", 1, CORE_7_INDEX, &NAT_CACHE_CFG_7, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SRAM_DUMMY_STORE", 1, CORE_7_INDEX, &SRAM_DUMMY_STORE_7, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_7_INDEX, &NAT_CACHE_KEY0_MASK_7, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_7_INDEX, &RX_MIRRORING_CONFIGURATION_7, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RATE_LIMIT_OVERHEAD", 1, CORE_7_INDEX, &RATE_LIMIT_OVERHEAD_7, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_QOS_DONT_DROP_RATIO", 1, CORE_7_INDEX, &INGRESS_QOS_DONT_DROP_RATIO_7, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_7_INDEX, &NATC_L2_VLAN_KEY_MASK_7, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_QOS_WAN_UNTAGGED_PRIORITY", 1, CORE_7_INDEX, &INGRESS_QOS_WAN_UNTAGGED_PRIORITY_7, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IC_MCAST_ENABLE", 1, CORE_7_INDEX, &IC_MCAST_ENABLE_7, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "ECN_IPV6_REMARK_TABLE", 1, CORE_7_INDEX, &ECN_IPV6_REMARK_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_REDIRECT_MODE", 1, CORE_7_INDEX, &CPU_REDIRECT_MODE_7, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_7_INDEX, &DEBUG_PRINT_CORE_LOCK_7, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_7_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_FILTER_CFG", 1, CORE_7_INDEX, &INGRESS_FILTER_CFG_7, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_7_INDEX, &INGRESS_FILTER_1588_CFG_7, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_7_INDEX, &RX_MIRRORING_DIRECT_ENABLE_7, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_L2_TOS_MASK", 1, CORE_7_INDEX, &NATC_L2_TOS_MASK_7, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEFAULT_VLAN_VID_TO_TX_VLAN_CNTR_TABLE", 1, CORE_7_INDEX, &DEFAULT_VLAN_VID_TO_TX_VLAN_CNTR_TABLE_7, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "DS_PACKET_BUFFER", 1, CORE_8_INDEX, &DS_PACKET_BUFFER_8, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_FLOW_TABLE", 1, CORE_8_INDEX, &RX_FLOW_TABLE_8, 320, 1, 1 },
#endif
#if defined BCM6858
	{ "VPORT_CFG_TABLE", 1, CORE_8_INDEX, &VPORT_CFG_TABLE_8, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "DSCP_TO_PBITS_MAP_TABLE", 1, CORE_8_INDEX, &DSCP_TO_PBITS_MAP_TABLE_8, 4, 64, 1 },
#endif
#if defined BCM6858
	{ "TCAM_IC_CMD_TABLE", 1, CORE_8_INDEX, &TCAM_IC_CMD_TABLE_8, 9, 16, 1 },
#endif
#if defined BCM6858
	{ "MCAST_PROCESSING_TASKS_REPLY", 1, CORE_8_INDEX, &MCAST_PROCESSING_TASKS_REPLY_8, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "PBIT_TO_GEM_TABLE", 1, CORE_8_INDEX, &PBIT_TO_GEM_TABLE_8, 16, 8, 1 },
#endif
#if defined BCM6858
	{ "TX_FLOW_TABLE", 1, CORE_8_INDEX, &TX_FLOW_TABLE_8, 192, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_PROCESSING_TASKS_LOCK", 1, CORE_8_INDEX, &MCAST_PROCESSING_TASKS_LOCK_8, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "TC_TO_QUEUE_TABLE", 1, CORE_8_INDEX, &TC_TO_QUEUE_TABLE_8, 65, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_FPM_TOKENS_READ_FLAG", 1, CORE_8_INDEX, &MCAST_FPM_TOKENS_READ_FLAG_8, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "VPORT_TO_DSCP_TO_PBITS_TABLE", 1, CORE_8_INDEX, &VPORT_TO_DSCP_TO_PBITS_TABLE_8, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_8_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_8, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_BBH_OVERRUN_TASKS_LIMIT", 1, CORE_8_INDEX, &MCAST_BBH_OVERRUN_TASKS_LIMIT_8, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "LAYER2_HEADER_COPY_MAPPING_TABLE", 1, CORE_8_INDEX, &LAYER2_HEADER_COPY_MAPPING_TABLE_8, 30, 1, 1 },
#endif
#if defined BCM6858
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_8_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_8, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "EMAC_FLOW_CTRL", 1, CORE_8_INDEX, &EMAC_FLOW_CTRL_8, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "POLICER_PARAMS_TABLE", 1, CORE_8_INDEX, &POLICER_PARAMS_TABLE_8, 80, 1, 1 },
#endif
#if defined BCM6858
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_8_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_8, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "FLOODING_HEADER_FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_8_INDEX, &FLOODING_HEADER_FPM_POOL_NUMBER_MAPPING_TABLE_8, 32, 1, 1 },
#endif
#if defined BCM6858
	{ "PBIT_TO_QUEUE_TABLE", 1, CORE_8_INDEX, &PBIT_TO_QUEUE_TABLE_8, 65, 1, 1 },
#endif
#if defined BCM6858
	{ "DEBUG_SCRATCHPAD", 1, CORE_8_INDEX, &DEBUG_SCRATCHPAD_8, 10, 1, 1 },
#endif
#if defined BCM6858
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_8_INDEX, &LOOPBACK_QUEUE_TABLE_8, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "GLOBAL_DSCP_TO_PBITS_TABLE", 1, CORE_8_INDEX, &GLOBAL_DSCP_TO_PBITS_TABLE_8, 1, 64, 1 },
#endif
#if defined BCM6858
	{ "VPORT_CFG_EX_TABLE", 1, CORE_8_INDEX, &VPORT_CFG_EX_TABLE_8, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "PORT_MAC", 1, CORE_8_INDEX, &PORT_MAC_8, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_8_INDEX, &INGRESS_FILTER_PROFILE_TABLE_8, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "DUAL_STACK_LITE_TABLE", 1, CORE_8_INDEX, &DUAL_STACK_LITE_TABLE_8, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "QUEUE_THRESHOLD_VECTOR", 1, CORE_8_INDEX, &QUEUE_THRESHOLD_VECTOR_8, 9, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_PACKET_BASED_MAPPING", 1, CORE_8_INDEX, &INGRESS_PACKET_BASED_MAPPING_8, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "PROCESSING_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_8_INDEX, &PROCESSING_QUEUE_DYNAMIC_MNG_TABLE_8, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SPDSVC_ANALYZER_PARAMS_TABLE", 1, CORE_8_INDEX, &SPDSVC_ANALYZER_PARAMS_TABLE_8, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_8_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_8, 128, 1, 1 },
#endif
#if defined BCM6858
	{ "DBG_DUMP_TABLE", 1, CORE_8_INDEX, &DBG_DUMP_TABLE_8, 128, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_TBL_CFG", 1, CORE_8_INDEX, &NATC_TBL_CFG_8, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_COUNTERS_GENERIC_CONTEXT", 1, CORE_8_INDEX, &NATC_COUNTERS_GENERIC_CONTEXT_8, 128, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_FLOW_CONTEXT_DDR_ADDR", 1, CORE_8_INDEX, &RX_FLOW_CONTEXT_DDR_ADDR_8, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "BRIDGE_CFG_TABLE", 1, CORE_8_INDEX, &BRIDGE_CFG_TABLE_8, 2, 1, 1 },
#endif
#if defined BCM6858
	{ "VPORT_PBIT_TO_DISCARD_PRIO_VECTOR", 1, CORE_8_INDEX, &VPORT_PBIT_TO_DISCARD_PRIO_VECTOR_8, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_8_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_8, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "MAX_PKT_LEN_TABLE", 1, CORE_8_INDEX, &MAX_PKT_LEN_TABLE_8, 32, 1, 1 },
#endif
#if defined BCM6858
	{ "REGISTERS_BUFFER", 1, CORE_8_INDEX, &REGISTERS_BUFFER_8, 32, 1, 1 },
#endif
#if defined BCM6858
	{ "TCAM_IC_CFG_TABLE", 1, CORE_8_INDEX, &TCAM_IC_CFG_TABLE_8, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "VLAN_ACTION_GPE_HANDLER_PTR_TABLE", 1, CORE_8_INDEX, &VLAN_ACTION_GPE_HANDLER_PTR_TABLE_8, 17, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_BBH_OVERRUN_MAX_TASKS_LIMIT", 1, CORE_8_INDEX, &MCAST_BBH_OVERRUN_MAX_TASKS_LIMIT_8, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SPDTEST_GEN_PARAM", 1, CORE_8_INDEX, &SPDTEST_GEN_PARAM_8, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "NULL_BUFFER", 1, CORE_8_INDEX, &NULL_BUFFER_8, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPTV_CONFIGURATION_TABLE", 1, CORE_8_INDEX, &IPTV_CONFIGURATION_TABLE_8, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "ZERO_VALUE", 1, CORE_8_INDEX, &ZERO_VALUE_8, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TUNNEL_TABLE", 1, CORE_8_INDEX, &TUNNEL_TABLE_8, 2, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_FEED_RING_DESCRIPTOR_TABLE", 1, CORE_8_INDEX, &CPU_FEED_RING_DESCRIPTOR_TABLE_8, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEBUG_PRINT_TABLE", 1, CORE_8_INDEX, &DEBUG_PRINT_TABLE_8, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "FPM_GLOBAL_CFG", 1, CORE_8_INDEX, &FPM_GLOBAL_CFG_8, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "ONE_VALUE", 1, CORE_8_INDEX, &ONE_VALUE_8, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_FPM_THRESHOLDS", 1, CORE_8_INDEX, &DHD_FPM_THRESHOLDS_8, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_BBH_OVERRUN_MIN_TASKS_LIMIT", 1, CORE_8_INDEX, &MCAST_BBH_OVERRUN_MIN_TASKS_LIMIT_8, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SYSTEM_CONFIGURATION", 1, CORE_8_INDEX, &SYSTEM_CONFIGURATION_8, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_8_INDEX, &LOOPBACK_WAN_FLOW_TABLE_8, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "FORCE_DSCP", 1, CORE_8_INDEX, &FORCE_DSCP_8, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CORE_ID_TABLE", 1, CORE_8_INDEX, &CORE_ID_TABLE_8, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TUNNEL_HEADER_PSRAM_BUFFER", 1, CORE_8_INDEX, &TUNNEL_HEADER_PSRAM_BUFFER_8, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TASK_IDX", 1, CORE_8_INDEX, &TASK_IDX_8, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TUNNELS_PARSING_CFG", 1, CORE_8_INDEX, &TUNNELS_PARSING_CFG_8, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPTV_CFG_TABLE", 1, CORE_8_INDEX, &IPTV_CFG_TABLE_8, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_8_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_8, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEFAULT_VLAN_VID_TABLE", 1, CORE_8_INDEX, &DEFAULT_VLAN_VID_TABLE_8, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "NAT_CACHE_CFG", 1, CORE_8_INDEX, &NAT_CACHE_CFG_8, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SRAM_DUMMY_STORE", 1, CORE_8_INDEX, &SRAM_DUMMY_STORE_8, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_8_INDEX, &NAT_CACHE_KEY0_MASK_8, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_8_INDEX, &RX_MIRRORING_CONFIGURATION_8, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RATE_LIMIT_OVERHEAD", 1, CORE_8_INDEX, &RATE_LIMIT_OVERHEAD_8, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_QOS_DONT_DROP_RATIO", 1, CORE_8_INDEX, &INGRESS_QOS_DONT_DROP_RATIO_8, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_8_INDEX, &NATC_L2_VLAN_KEY_MASK_8, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_QOS_WAN_UNTAGGED_PRIORITY", 1, CORE_8_INDEX, &INGRESS_QOS_WAN_UNTAGGED_PRIORITY_8, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IC_MCAST_ENABLE", 1, CORE_8_INDEX, &IC_MCAST_ENABLE_8, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "ECN_IPV6_REMARK_TABLE", 1, CORE_8_INDEX, &ECN_IPV6_REMARK_TABLE_8, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_REDIRECT_MODE", 1, CORE_8_INDEX, &CPU_REDIRECT_MODE_8, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_8_INDEX, &DEBUG_PRINT_CORE_LOCK_8, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_8_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_8, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_FILTER_CFG", 1, CORE_8_INDEX, &INGRESS_FILTER_CFG_8, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_8_INDEX, &INGRESS_FILTER_1588_CFG_8, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_8_INDEX, &RX_MIRRORING_DIRECT_ENABLE_8, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_L2_TOS_MASK", 1, CORE_8_INDEX, &NATC_L2_TOS_MASK_8, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEFAULT_VLAN_VID_TO_TX_VLAN_CNTR_TABLE", 1, CORE_8_INDEX, &DEFAULT_VLAN_VID_TO_TX_VLAN_CNTR_TABLE_8, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "DS_PACKET_BUFFER", 1, CORE_9_INDEX, &DS_PACKET_BUFFER_9, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_FLOW_TABLE", 1, CORE_9_INDEX, &RX_FLOW_TABLE_9, 320, 1, 1 },
#endif
#if defined BCM6858
	{ "VPORT_CFG_TABLE", 1, CORE_9_INDEX, &VPORT_CFG_TABLE_9, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "DSCP_TO_PBITS_MAP_TABLE", 1, CORE_9_INDEX, &DSCP_TO_PBITS_MAP_TABLE_9, 4, 64, 1 },
#endif
#if defined BCM6858
	{ "TCAM_IC_CMD_TABLE", 1, CORE_9_INDEX, &TCAM_IC_CMD_TABLE_9, 9, 16, 1 },
#endif
#if defined BCM6858
	{ "MCAST_PROCESSING_TASKS_REPLY", 1, CORE_9_INDEX, &MCAST_PROCESSING_TASKS_REPLY_9, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "PBIT_TO_GEM_TABLE", 1, CORE_9_INDEX, &PBIT_TO_GEM_TABLE_9, 16, 8, 1 },
#endif
#if defined BCM6858
	{ "TX_FLOW_TABLE", 1, CORE_9_INDEX, &TX_FLOW_TABLE_9, 192, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_PROCESSING_TASKS_LOCK", 1, CORE_9_INDEX, &MCAST_PROCESSING_TASKS_LOCK_9, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "TC_TO_QUEUE_TABLE", 1, CORE_9_INDEX, &TC_TO_QUEUE_TABLE_9, 65, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_FPM_TOKENS_READ_FLAG", 1, CORE_9_INDEX, &MCAST_FPM_TOKENS_READ_FLAG_9, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "VPORT_TO_DSCP_TO_PBITS_TABLE", 1, CORE_9_INDEX, &VPORT_TO_DSCP_TO_PBITS_TABLE_9, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_9_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_9, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_BBH_OVERRUN_TASKS_LIMIT", 1, CORE_9_INDEX, &MCAST_BBH_OVERRUN_TASKS_LIMIT_9, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "LAYER2_HEADER_COPY_MAPPING_TABLE", 1, CORE_9_INDEX, &LAYER2_HEADER_COPY_MAPPING_TABLE_9, 30, 1, 1 },
#endif
#if defined BCM6858
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_9_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_9, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "EMAC_FLOW_CTRL", 1, CORE_9_INDEX, &EMAC_FLOW_CTRL_9, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "POLICER_PARAMS_TABLE", 1, CORE_9_INDEX, &POLICER_PARAMS_TABLE_9, 80, 1, 1 },
#endif
#if defined BCM6858
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_9_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_9, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "FLOODING_HEADER_FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_9_INDEX, &FLOODING_HEADER_FPM_POOL_NUMBER_MAPPING_TABLE_9, 32, 1, 1 },
#endif
#if defined BCM6858
	{ "PBIT_TO_QUEUE_TABLE", 1, CORE_9_INDEX, &PBIT_TO_QUEUE_TABLE_9, 65, 1, 1 },
#endif
#if defined BCM6858
	{ "DEBUG_SCRATCHPAD", 1, CORE_9_INDEX, &DEBUG_SCRATCHPAD_9, 10, 1, 1 },
#endif
#if defined BCM6858
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_9_INDEX, &LOOPBACK_QUEUE_TABLE_9, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "GLOBAL_DSCP_TO_PBITS_TABLE", 1, CORE_9_INDEX, &GLOBAL_DSCP_TO_PBITS_TABLE_9, 1, 64, 1 },
#endif
#if defined BCM6858
	{ "VPORT_CFG_EX_TABLE", 1, CORE_9_INDEX, &VPORT_CFG_EX_TABLE_9, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "PORT_MAC", 1, CORE_9_INDEX, &PORT_MAC_9, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_9_INDEX, &INGRESS_FILTER_PROFILE_TABLE_9, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "DUAL_STACK_LITE_TABLE", 1, CORE_9_INDEX, &DUAL_STACK_LITE_TABLE_9, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "QUEUE_THRESHOLD_VECTOR", 1, CORE_9_INDEX, &QUEUE_THRESHOLD_VECTOR_9, 9, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_PACKET_BASED_MAPPING", 1, CORE_9_INDEX, &INGRESS_PACKET_BASED_MAPPING_9, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "PROCESSING_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_9_INDEX, &PROCESSING_QUEUE_DYNAMIC_MNG_TABLE_9, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SPDSVC_ANALYZER_PARAMS_TABLE", 1, CORE_9_INDEX, &SPDSVC_ANALYZER_PARAMS_TABLE_9, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_9_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_9, 128, 1, 1 },
#endif
#if defined BCM6858
	{ "DBG_DUMP_TABLE", 1, CORE_9_INDEX, &DBG_DUMP_TABLE_9, 128, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_TBL_CFG", 1, CORE_9_INDEX, &NATC_TBL_CFG_9, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_COUNTERS_GENERIC_CONTEXT", 1, CORE_9_INDEX, &NATC_COUNTERS_GENERIC_CONTEXT_9, 128, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_FLOW_CONTEXT_DDR_ADDR", 1, CORE_9_INDEX, &RX_FLOW_CONTEXT_DDR_ADDR_9, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "BRIDGE_CFG_TABLE", 1, CORE_9_INDEX, &BRIDGE_CFG_TABLE_9, 2, 1, 1 },
#endif
#if defined BCM6858
	{ "VPORT_PBIT_TO_DISCARD_PRIO_VECTOR", 1, CORE_9_INDEX, &VPORT_PBIT_TO_DISCARD_PRIO_VECTOR_9, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_9_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_9, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "MAX_PKT_LEN_TABLE", 1, CORE_9_INDEX, &MAX_PKT_LEN_TABLE_9, 32, 1, 1 },
#endif
#if defined BCM6858
	{ "REGISTERS_BUFFER", 1, CORE_9_INDEX, &REGISTERS_BUFFER_9, 32, 1, 1 },
#endif
#if defined BCM6858
	{ "TCAM_IC_CFG_TABLE", 1, CORE_9_INDEX, &TCAM_IC_CFG_TABLE_9, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "VLAN_ACTION_GPE_HANDLER_PTR_TABLE", 1, CORE_9_INDEX, &VLAN_ACTION_GPE_HANDLER_PTR_TABLE_9, 17, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_BBH_OVERRUN_MAX_TASKS_LIMIT", 1, CORE_9_INDEX, &MCAST_BBH_OVERRUN_MAX_TASKS_LIMIT_9, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SPDTEST_GEN_PARAM", 1, CORE_9_INDEX, &SPDTEST_GEN_PARAM_9, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "NULL_BUFFER", 1, CORE_9_INDEX, &NULL_BUFFER_9, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPTV_CONFIGURATION_TABLE", 1, CORE_9_INDEX, &IPTV_CONFIGURATION_TABLE_9, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "ZERO_VALUE", 1, CORE_9_INDEX, &ZERO_VALUE_9, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TUNNEL_TABLE", 1, CORE_9_INDEX, &TUNNEL_TABLE_9, 2, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_FEED_RING_DESCRIPTOR_TABLE", 1, CORE_9_INDEX, &CPU_FEED_RING_DESCRIPTOR_TABLE_9, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEBUG_PRINT_TABLE", 1, CORE_9_INDEX, &DEBUG_PRINT_TABLE_9, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "FPM_GLOBAL_CFG", 1, CORE_9_INDEX, &FPM_GLOBAL_CFG_9, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "ONE_VALUE", 1, CORE_9_INDEX, &ONE_VALUE_9, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_FPM_THRESHOLDS", 1, CORE_9_INDEX, &DHD_FPM_THRESHOLDS_9, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_BBH_OVERRUN_MIN_TASKS_LIMIT", 1, CORE_9_INDEX, &MCAST_BBH_OVERRUN_MIN_TASKS_LIMIT_9, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SYSTEM_CONFIGURATION", 1, CORE_9_INDEX, &SYSTEM_CONFIGURATION_9, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_9_INDEX, &LOOPBACK_WAN_FLOW_TABLE_9, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "FORCE_DSCP", 1, CORE_9_INDEX, &FORCE_DSCP_9, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CORE_ID_TABLE", 1, CORE_9_INDEX, &CORE_ID_TABLE_9, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TUNNEL_HEADER_PSRAM_BUFFER", 1, CORE_9_INDEX, &TUNNEL_HEADER_PSRAM_BUFFER_9, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TASK_IDX", 1, CORE_9_INDEX, &TASK_IDX_9, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TUNNELS_PARSING_CFG", 1, CORE_9_INDEX, &TUNNELS_PARSING_CFG_9, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPTV_CFG_TABLE", 1, CORE_9_INDEX, &IPTV_CFG_TABLE_9, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_9_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_9, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEFAULT_VLAN_VID_TABLE", 1, CORE_9_INDEX, &DEFAULT_VLAN_VID_TABLE_9, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "NAT_CACHE_CFG", 1, CORE_9_INDEX, &NAT_CACHE_CFG_9, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SRAM_DUMMY_STORE", 1, CORE_9_INDEX, &SRAM_DUMMY_STORE_9, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_9_INDEX, &NAT_CACHE_KEY0_MASK_9, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_9_INDEX, &RX_MIRRORING_CONFIGURATION_9, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RATE_LIMIT_OVERHEAD", 1, CORE_9_INDEX, &RATE_LIMIT_OVERHEAD_9, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_QOS_DONT_DROP_RATIO", 1, CORE_9_INDEX, &INGRESS_QOS_DONT_DROP_RATIO_9, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_9_INDEX, &NATC_L2_VLAN_KEY_MASK_9, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_QOS_WAN_UNTAGGED_PRIORITY", 1, CORE_9_INDEX, &INGRESS_QOS_WAN_UNTAGGED_PRIORITY_9, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IC_MCAST_ENABLE", 1, CORE_9_INDEX, &IC_MCAST_ENABLE_9, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "ECN_IPV6_REMARK_TABLE", 1, CORE_9_INDEX, &ECN_IPV6_REMARK_TABLE_9, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_REDIRECT_MODE", 1, CORE_9_INDEX, &CPU_REDIRECT_MODE_9, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_9_INDEX, &DEBUG_PRINT_CORE_LOCK_9, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_9_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_9, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_FILTER_CFG", 1, CORE_9_INDEX, &INGRESS_FILTER_CFG_9, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_9_INDEX, &INGRESS_FILTER_1588_CFG_9, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_9_INDEX, &RX_MIRRORING_DIRECT_ENABLE_9, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_L2_TOS_MASK", 1, CORE_9_INDEX, &NATC_L2_TOS_MASK_9, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEFAULT_VLAN_VID_TO_TX_VLAN_CNTR_TABLE", 1, CORE_9_INDEX, &DEFAULT_VLAN_VID_TO_TX_VLAN_CNTR_TABLE_9, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "DS_PACKET_BUFFER", 1, CORE_10_INDEX, &DS_PACKET_BUFFER_10, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_FLOW_TABLE", 1, CORE_10_INDEX, &RX_FLOW_TABLE_10, 320, 1, 1 },
#endif
#if defined BCM6858
	{ "VPORT_CFG_TABLE", 1, CORE_10_INDEX, &VPORT_CFG_TABLE_10, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "DSCP_TO_PBITS_MAP_TABLE", 1, CORE_10_INDEX, &DSCP_TO_PBITS_MAP_TABLE_10, 4, 64, 1 },
#endif
#if defined BCM6858
	{ "TCAM_IC_CMD_TABLE", 1, CORE_10_INDEX, &TCAM_IC_CMD_TABLE_10, 9, 16, 1 },
#endif
#if defined BCM6858
	{ "MCAST_PROCESSING_TASKS_REPLY", 1, CORE_10_INDEX, &MCAST_PROCESSING_TASKS_REPLY_10, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "PBIT_TO_GEM_TABLE", 1, CORE_10_INDEX, &PBIT_TO_GEM_TABLE_10, 16, 8, 1 },
#endif
#if defined BCM6858
	{ "TX_FLOW_TABLE", 1, CORE_10_INDEX, &TX_FLOW_TABLE_10, 192, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_PROCESSING_TASKS_LOCK", 1, CORE_10_INDEX, &MCAST_PROCESSING_TASKS_LOCK_10, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "TC_TO_QUEUE_TABLE", 1, CORE_10_INDEX, &TC_TO_QUEUE_TABLE_10, 65, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_FPM_TOKENS_READ_FLAG", 1, CORE_10_INDEX, &MCAST_FPM_TOKENS_READ_FLAG_10, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "VPORT_TO_DSCP_TO_PBITS_TABLE", 1, CORE_10_INDEX, &VPORT_TO_DSCP_TO_PBITS_TABLE_10, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_10_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_10, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_BBH_OVERRUN_TASKS_LIMIT", 1, CORE_10_INDEX, &MCAST_BBH_OVERRUN_TASKS_LIMIT_10, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "LAYER2_HEADER_COPY_MAPPING_TABLE", 1, CORE_10_INDEX, &LAYER2_HEADER_COPY_MAPPING_TABLE_10, 30, 1, 1 },
#endif
#if defined BCM6858
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_10_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_10, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "EMAC_FLOW_CTRL", 1, CORE_10_INDEX, &EMAC_FLOW_CTRL_10, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "POLICER_PARAMS_TABLE", 1, CORE_10_INDEX, &POLICER_PARAMS_TABLE_10, 80, 1, 1 },
#endif
#if defined BCM6858
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_10_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_10, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "FLOODING_HEADER_FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_10_INDEX, &FLOODING_HEADER_FPM_POOL_NUMBER_MAPPING_TABLE_10, 32, 1, 1 },
#endif
#if defined BCM6858
	{ "PBIT_TO_QUEUE_TABLE", 1, CORE_10_INDEX, &PBIT_TO_QUEUE_TABLE_10, 65, 1, 1 },
#endif
#if defined BCM6858
	{ "DEBUG_SCRATCHPAD", 1, CORE_10_INDEX, &DEBUG_SCRATCHPAD_10, 10, 1, 1 },
#endif
#if defined BCM6858
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_10_INDEX, &LOOPBACK_QUEUE_TABLE_10, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "GLOBAL_DSCP_TO_PBITS_TABLE", 1, CORE_10_INDEX, &GLOBAL_DSCP_TO_PBITS_TABLE_10, 1, 64, 1 },
#endif
#if defined BCM6858
	{ "VPORT_CFG_EX_TABLE", 1, CORE_10_INDEX, &VPORT_CFG_EX_TABLE_10, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "PORT_MAC", 1, CORE_10_INDEX, &PORT_MAC_10, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_10_INDEX, &INGRESS_FILTER_PROFILE_TABLE_10, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "DUAL_STACK_LITE_TABLE", 1, CORE_10_INDEX, &DUAL_STACK_LITE_TABLE_10, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "QUEUE_THRESHOLD_VECTOR", 1, CORE_10_INDEX, &QUEUE_THRESHOLD_VECTOR_10, 9, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_PACKET_BASED_MAPPING", 1, CORE_10_INDEX, &INGRESS_PACKET_BASED_MAPPING_10, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "PROCESSING_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_10_INDEX, &PROCESSING_QUEUE_DYNAMIC_MNG_TABLE_10, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SPDSVC_ANALYZER_PARAMS_TABLE", 1, CORE_10_INDEX, &SPDSVC_ANALYZER_PARAMS_TABLE_10, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_10_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_10, 128, 1, 1 },
#endif
#if defined BCM6858
	{ "DBG_DUMP_TABLE", 1, CORE_10_INDEX, &DBG_DUMP_TABLE_10, 128, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_TBL_CFG", 1, CORE_10_INDEX, &NATC_TBL_CFG_10, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_COUNTERS_GENERIC_CONTEXT", 1, CORE_10_INDEX, &NATC_COUNTERS_GENERIC_CONTEXT_10, 128, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_FLOW_CONTEXT_DDR_ADDR", 1, CORE_10_INDEX, &RX_FLOW_CONTEXT_DDR_ADDR_10, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "BRIDGE_CFG_TABLE", 1, CORE_10_INDEX, &BRIDGE_CFG_TABLE_10, 2, 1, 1 },
#endif
#if defined BCM6858
	{ "VPORT_PBIT_TO_DISCARD_PRIO_VECTOR", 1, CORE_10_INDEX, &VPORT_PBIT_TO_DISCARD_PRIO_VECTOR_10, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_10_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_10, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "MAX_PKT_LEN_TABLE", 1, CORE_10_INDEX, &MAX_PKT_LEN_TABLE_10, 32, 1, 1 },
#endif
#if defined BCM6858
	{ "REGISTERS_BUFFER", 1, CORE_10_INDEX, &REGISTERS_BUFFER_10, 32, 1, 1 },
#endif
#if defined BCM6858
	{ "TCAM_IC_CFG_TABLE", 1, CORE_10_INDEX, &TCAM_IC_CFG_TABLE_10, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "VLAN_ACTION_GPE_HANDLER_PTR_TABLE", 1, CORE_10_INDEX, &VLAN_ACTION_GPE_HANDLER_PTR_TABLE_10, 17, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_BBH_OVERRUN_MAX_TASKS_LIMIT", 1, CORE_10_INDEX, &MCAST_BBH_OVERRUN_MAX_TASKS_LIMIT_10, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SPDTEST_GEN_PARAM", 1, CORE_10_INDEX, &SPDTEST_GEN_PARAM_10, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "NULL_BUFFER", 1, CORE_10_INDEX, &NULL_BUFFER_10, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPTV_CONFIGURATION_TABLE", 1, CORE_10_INDEX, &IPTV_CONFIGURATION_TABLE_10, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "ZERO_VALUE", 1, CORE_10_INDEX, &ZERO_VALUE_10, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TUNNEL_TABLE", 1, CORE_10_INDEX, &TUNNEL_TABLE_10, 2, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_FEED_RING_DESCRIPTOR_TABLE", 1, CORE_10_INDEX, &CPU_FEED_RING_DESCRIPTOR_TABLE_10, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEBUG_PRINT_TABLE", 1, CORE_10_INDEX, &DEBUG_PRINT_TABLE_10, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "FPM_GLOBAL_CFG", 1, CORE_10_INDEX, &FPM_GLOBAL_CFG_10, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "ONE_VALUE", 1, CORE_10_INDEX, &ONE_VALUE_10, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_FPM_THRESHOLDS", 1, CORE_10_INDEX, &DHD_FPM_THRESHOLDS_10, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_BBH_OVERRUN_MIN_TASKS_LIMIT", 1, CORE_10_INDEX, &MCAST_BBH_OVERRUN_MIN_TASKS_LIMIT_10, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SYSTEM_CONFIGURATION", 1, CORE_10_INDEX, &SYSTEM_CONFIGURATION_10, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_10_INDEX, &LOOPBACK_WAN_FLOW_TABLE_10, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "FORCE_DSCP", 1, CORE_10_INDEX, &FORCE_DSCP_10, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CORE_ID_TABLE", 1, CORE_10_INDEX, &CORE_ID_TABLE_10, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TUNNEL_HEADER_PSRAM_BUFFER", 1, CORE_10_INDEX, &TUNNEL_HEADER_PSRAM_BUFFER_10, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TASK_IDX", 1, CORE_10_INDEX, &TASK_IDX_10, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TUNNELS_PARSING_CFG", 1, CORE_10_INDEX, &TUNNELS_PARSING_CFG_10, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPTV_CFG_TABLE", 1, CORE_10_INDEX, &IPTV_CFG_TABLE_10, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_10_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_10, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEFAULT_VLAN_VID_TABLE", 1, CORE_10_INDEX, &DEFAULT_VLAN_VID_TABLE_10, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "NAT_CACHE_CFG", 1, CORE_10_INDEX, &NAT_CACHE_CFG_10, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SRAM_DUMMY_STORE", 1, CORE_10_INDEX, &SRAM_DUMMY_STORE_10, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_10_INDEX, &NAT_CACHE_KEY0_MASK_10, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_10_INDEX, &RX_MIRRORING_CONFIGURATION_10, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RATE_LIMIT_OVERHEAD", 1, CORE_10_INDEX, &RATE_LIMIT_OVERHEAD_10, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_QOS_DONT_DROP_RATIO", 1, CORE_10_INDEX, &INGRESS_QOS_DONT_DROP_RATIO_10, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_10_INDEX, &NATC_L2_VLAN_KEY_MASK_10, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_QOS_WAN_UNTAGGED_PRIORITY", 1, CORE_10_INDEX, &INGRESS_QOS_WAN_UNTAGGED_PRIORITY_10, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IC_MCAST_ENABLE", 1, CORE_10_INDEX, &IC_MCAST_ENABLE_10, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "ECN_IPV6_REMARK_TABLE", 1, CORE_10_INDEX, &ECN_IPV6_REMARK_TABLE_10, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_REDIRECT_MODE", 1, CORE_10_INDEX, &CPU_REDIRECT_MODE_10, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_10_INDEX, &DEBUG_PRINT_CORE_LOCK_10, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_10_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_10, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_FILTER_CFG", 1, CORE_10_INDEX, &INGRESS_FILTER_CFG_10, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_10_INDEX, &INGRESS_FILTER_1588_CFG_10, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_10_INDEX, &RX_MIRRORING_DIRECT_ENABLE_10, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_L2_TOS_MASK", 1, CORE_10_INDEX, &NATC_L2_TOS_MASK_10, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEFAULT_VLAN_VID_TO_TX_VLAN_CNTR_TABLE", 1, CORE_10_INDEX, &DEFAULT_VLAN_VID_TO_TX_VLAN_CNTR_TABLE_10, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "DS_PACKET_BUFFER", 1, CORE_11_INDEX, &DS_PACKET_BUFFER_11, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_FLOW_TABLE", 1, CORE_11_INDEX, &RX_FLOW_TABLE_11, 320, 1, 1 },
#endif
#if defined BCM6858
	{ "VPORT_CFG_TABLE", 1, CORE_11_INDEX, &VPORT_CFG_TABLE_11, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "DSCP_TO_PBITS_MAP_TABLE", 1, CORE_11_INDEX, &DSCP_TO_PBITS_MAP_TABLE_11, 4, 64, 1 },
#endif
#if defined BCM6858
	{ "TCAM_IC_CMD_TABLE", 1, CORE_11_INDEX, &TCAM_IC_CMD_TABLE_11, 9, 16, 1 },
#endif
#if defined BCM6858
	{ "MCAST_PROCESSING_TASKS_REPLY", 1, CORE_11_INDEX, &MCAST_PROCESSING_TASKS_REPLY_11, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "PBIT_TO_GEM_TABLE", 1, CORE_11_INDEX, &PBIT_TO_GEM_TABLE_11, 16, 8, 1 },
#endif
#if defined BCM6858
	{ "TX_FLOW_TABLE", 1, CORE_11_INDEX, &TX_FLOW_TABLE_11, 192, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_PROCESSING_TASKS_LOCK", 1, CORE_11_INDEX, &MCAST_PROCESSING_TASKS_LOCK_11, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "TC_TO_QUEUE_TABLE", 1, CORE_11_INDEX, &TC_TO_QUEUE_TABLE_11, 65, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_FPM_TOKENS_READ_FLAG", 1, CORE_11_INDEX, &MCAST_FPM_TOKENS_READ_FLAG_11, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "VPORT_TO_DSCP_TO_PBITS_TABLE", 1, CORE_11_INDEX, &VPORT_TO_DSCP_TO_PBITS_TABLE_11, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_11_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_11, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_BBH_OVERRUN_TASKS_LIMIT", 1, CORE_11_INDEX, &MCAST_BBH_OVERRUN_TASKS_LIMIT_11, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "LAYER2_HEADER_COPY_MAPPING_TABLE", 1, CORE_11_INDEX, &LAYER2_HEADER_COPY_MAPPING_TABLE_11, 30, 1, 1 },
#endif
#if defined BCM6858
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_11_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_11, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "EMAC_FLOW_CTRL", 1, CORE_11_INDEX, &EMAC_FLOW_CTRL_11, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "POLICER_PARAMS_TABLE", 1, CORE_11_INDEX, &POLICER_PARAMS_TABLE_11, 80, 1, 1 },
#endif
#if defined BCM6858
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_11_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_11, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "FLOODING_HEADER_FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_11_INDEX, &FLOODING_HEADER_FPM_POOL_NUMBER_MAPPING_TABLE_11, 32, 1, 1 },
#endif
#if defined BCM6858
	{ "PBIT_TO_QUEUE_TABLE", 1, CORE_11_INDEX, &PBIT_TO_QUEUE_TABLE_11, 65, 1, 1 },
#endif
#if defined BCM6858
	{ "DEBUG_SCRATCHPAD", 1, CORE_11_INDEX, &DEBUG_SCRATCHPAD_11, 10, 1, 1 },
#endif
#if defined BCM6858
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_11_INDEX, &LOOPBACK_QUEUE_TABLE_11, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "GLOBAL_DSCP_TO_PBITS_TABLE", 1, CORE_11_INDEX, &GLOBAL_DSCP_TO_PBITS_TABLE_11, 1, 64, 1 },
#endif
#if defined BCM6858
	{ "VPORT_CFG_EX_TABLE", 1, CORE_11_INDEX, &VPORT_CFG_EX_TABLE_11, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "PORT_MAC", 1, CORE_11_INDEX, &PORT_MAC_11, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_11_INDEX, &INGRESS_FILTER_PROFILE_TABLE_11, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "DUAL_STACK_LITE_TABLE", 1, CORE_11_INDEX, &DUAL_STACK_LITE_TABLE_11, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "QUEUE_THRESHOLD_VECTOR", 1, CORE_11_INDEX, &QUEUE_THRESHOLD_VECTOR_11, 9, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_PACKET_BASED_MAPPING", 1, CORE_11_INDEX, &INGRESS_PACKET_BASED_MAPPING_11, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "PROCESSING_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_11_INDEX, &PROCESSING_QUEUE_DYNAMIC_MNG_TABLE_11, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SPDSVC_ANALYZER_PARAMS_TABLE", 1, CORE_11_INDEX, &SPDSVC_ANALYZER_PARAMS_TABLE_11, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_11_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_11, 128, 1, 1 },
#endif
#if defined BCM6858
	{ "DBG_DUMP_TABLE", 1, CORE_11_INDEX, &DBG_DUMP_TABLE_11, 128, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_TBL_CFG", 1, CORE_11_INDEX, &NATC_TBL_CFG_11, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_COUNTERS_GENERIC_CONTEXT", 1, CORE_11_INDEX, &NATC_COUNTERS_GENERIC_CONTEXT_11, 128, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_FLOW_CONTEXT_DDR_ADDR", 1, CORE_11_INDEX, &RX_FLOW_CONTEXT_DDR_ADDR_11, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "BRIDGE_CFG_TABLE", 1, CORE_11_INDEX, &BRIDGE_CFG_TABLE_11, 2, 1, 1 },
#endif
#if defined BCM6858
	{ "VPORT_PBIT_TO_DISCARD_PRIO_VECTOR", 1, CORE_11_INDEX, &VPORT_PBIT_TO_DISCARD_PRIO_VECTOR_11, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_11_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_11, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "MAX_PKT_LEN_TABLE", 1, CORE_11_INDEX, &MAX_PKT_LEN_TABLE_11, 32, 1, 1 },
#endif
#if defined BCM6858
	{ "REGISTERS_BUFFER", 1, CORE_11_INDEX, &REGISTERS_BUFFER_11, 32, 1, 1 },
#endif
#if defined BCM6858
	{ "TCAM_IC_CFG_TABLE", 1, CORE_11_INDEX, &TCAM_IC_CFG_TABLE_11, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "VLAN_ACTION_GPE_HANDLER_PTR_TABLE", 1, CORE_11_INDEX, &VLAN_ACTION_GPE_HANDLER_PTR_TABLE_11, 17, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_BBH_OVERRUN_MAX_TASKS_LIMIT", 1, CORE_11_INDEX, &MCAST_BBH_OVERRUN_MAX_TASKS_LIMIT_11, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SPDTEST_GEN_PARAM", 1, CORE_11_INDEX, &SPDTEST_GEN_PARAM_11, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "NULL_BUFFER", 1, CORE_11_INDEX, &NULL_BUFFER_11, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPTV_CONFIGURATION_TABLE", 1, CORE_11_INDEX, &IPTV_CONFIGURATION_TABLE_11, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "ZERO_VALUE", 1, CORE_11_INDEX, &ZERO_VALUE_11, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TUNNEL_TABLE", 1, CORE_11_INDEX, &TUNNEL_TABLE_11, 2, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_FEED_RING_DESCRIPTOR_TABLE", 1, CORE_11_INDEX, &CPU_FEED_RING_DESCRIPTOR_TABLE_11, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEBUG_PRINT_TABLE", 1, CORE_11_INDEX, &DEBUG_PRINT_TABLE_11, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "FPM_GLOBAL_CFG", 1, CORE_11_INDEX, &FPM_GLOBAL_CFG_11, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "ONE_VALUE", 1, CORE_11_INDEX, &ONE_VALUE_11, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_FPM_THRESHOLDS", 1, CORE_11_INDEX, &DHD_FPM_THRESHOLDS_11, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_BBH_OVERRUN_MIN_TASKS_LIMIT", 1, CORE_11_INDEX, &MCAST_BBH_OVERRUN_MIN_TASKS_LIMIT_11, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SYSTEM_CONFIGURATION", 1, CORE_11_INDEX, &SYSTEM_CONFIGURATION_11, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_11_INDEX, &LOOPBACK_WAN_FLOW_TABLE_11, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "FORCE_DSCP", 1, CORE_11_INDEX, &FORCE_DSCP_11, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CORE_ID_TABLE", 1, CORE_11_INDEX, &CORE_ID_TABLE_11, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TUNNEL_HEADER_PSRAM_BUFFER", 1, CORE_11_INDEX, &TUNNEL_HEADER_PSRAM_BUFFER_11, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TASK_IDX", 1, CORE_11_INDEX, &TASK_IDX_11, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TUNNELS_PARSING_CFG", 1, CORE_11_INDEX, &TUNNELS_PARSING_CFG_11, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPTV_CFG_TABLE", 1, CORE_11_INDEX, &IPTV_CFG_TABLE_11, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_11_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_11, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEFAULT_VLAN_VID_TABLE", 1, CORE_11_INDEX, &DEFAULT_VLAN_VID_TABLE_11, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "NAT_CACHE_CFG", 1, CORE_11_INDEX, &NAT_CACHE_CFG_11, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SRAM_DUMMY_STORE", 1, CORE_11_INDEX, &SRAM_DUMMY_STORE_11, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_11_INDEX, &NAT_CACHE_KEY0_MASK_11, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_11_INDEX, &RX_MIRRORING_CONFIGURATION_11, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RATE_LIMIT_OVERHEAD", 1, CORE_11_INDEX, &RATE_LIMIT_OVERHEAD_11, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_QOS_DONT_DROP_RATIO", 1, CORE_11_INDEX, &INGRESS_QOS_DONT_DROP_RATIO_11, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_11_INDEX, &NATC_L2_VLAN_KEY_MASK_11, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_QOS_WAN_UNTAGGED_PRIORITY", 1, CORE_11_INDEX, &INGRESS_QOS_WAN_UNTAGGED_PRIORITY_11, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IC_MCAST_ENABLE", 1, CORE_11_INDEX, &IC_MCAST_ENABLE_11, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "ECN_IPV6_REMARK_TABLE", 1, CORE_11_INDEX, &ECN_IPV6_REMARK_TABLE_11, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_REDIRECT_MODE", 1, CORE_11_INDEX, &CPU_REDIRECT_MODE_11, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_11_INDEX, &DEBUG_PRINT_CORE_LOCK_11, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_11_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_11, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_FILTER_CFG", 1, CORE_11_INDEX, &INGRESS_FILTER_CFG_11, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_11_INDEX, &INGRESS_FILTER_1588_CFG_11, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_11_INDEX, &RX_MIRRORING_DIRECT_ENABLE_11, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_L2_TOS_MASK", 1, CORE_11_INDEX, &NATC_L2_TOS_MASK_11, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEFAULT_VLAN_VID_TO_TX_VLAN_CNTR_TABLE", 1, CORE_11_INDEX, &DEFAULT_VLAN_VID_TO_TX_VLAN_CNTR_TABLE_11, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "DS_PACKET_BUFFER", 1, CORE_12_INDEX, &DS_PACKET_BUFFER_12, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_FLOW_TABLE", 1, CORE_12_INDEX, &RX_FLOW_TABLE_12, 320, 1, 1 },
#endif
#if defined BCM6858
	{ "VPORT_CFG_TABLE", 1, CORE_12_INDEX, &VPORT_CFG_TABLE_12, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "DSCP_TO_PBITS_MAP_TABLE", 1, CORE_12_INDEX, &DSCP_TO_PBITS_MAP_TABLE_12, 4, 64, 1 },
#endif
#if defined BCM6858
	{ "TCAM_IC_CMD_TABLE", 1, CORE_12_INDEX, &TCAM_IC_CMD_TABLE_12, 9, 16, 1 },
#endif
#if defined BCM6858
	{ "MCAST_PROCESSING_TASKS_REPLY", 1, CORE_12_INDEX, &MCAST_PROCESSING_TASKS_REPLY_12, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "PBIT_TO_GEM_TABLE", 1, CORE_12_INDEX, &PBIT_TO_GEM_TABLE_12, 16, 8, 1 },
#endif
#if defined BCM6858
	{ "TX_FLOW_TABLE", 1, CORE_12_INDEX, &TX_FLOW_TABLE_12, 192, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_PROCESSING_TASKS_LOCK", 1, CORE_12_INDEX, &MCAST_PROCESSING_TASKS_LOCK_12, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "TC_TO_QUEUE_TABLE", 1, CORE_12_INDEX, &TC_TO_QUEUE_TABLE_12, 65, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_FPM_TOKENS_READ_FLAG", 1, CORE_12_INDEX, &MCAST_FPM_TOKENS_READ_FLAG_12, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "VPORT_TO_DSCP_TO_PBITS_TABLE", 1, CORE_12_INDEX, &VPORT_TO_DSCP_TO_PBITS_TABLE_12, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_12_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_12, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_BBH_OVERRUN_TASKS_LIMIT", 1, CORE_12_INDEX, &MCAST_BBH_OVERRUN_TASKS_LIMIT_12, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "LAYER2_HEADER_COPY_MAPPING_TABLE", 1, CORE_12_INDEX, &LAYER2_HEADER_COPY_MAPPING_TABLE_12, 30, 1, 1 },
#endif
#if defined BCM6858
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_12_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_12, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "EMAC_FLOW_CTRL", 1, CORE_12_INDEX, &EMAC_FLOW_CTRL_12, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "POLICER_PARAMS_TABLE", 1, CORE_12_INDEX, &POLICER_PARAMS_TABLE_12, 80, 1, 1 },
#endif
#if defined BCM6858
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_12_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_12, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "FLOODING_HEADER_FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_12_INDEX, &FLOODING_HEADER_FPM_POOL_NUMBER_MAPPING_TABLE_12, 32, 1, 1 },
#endif
#if defined BCM6858
	{ "PBIT_TO_QUEUE_TABLE", 1, CORE_12_INDEX, &PBIT_TO_QUEUE_TABLE_12, 65, 1, 1 },
#endif
#if defined BCM6858
	{ "DEBUG_SCRATCHPAD", 1, CORE_12_INDEX, &DEBUG_SCRATCHPAD_12, 10, 1, 1 },
#endif
#if defined BCM6858
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_12_INDEX, &LOOPBACK_QUEUE_TABLE_12, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "GLOBAL_DSCP_TO_PBITS_TABLE", 1, CORE_12_INDEX, &GLOBAL_DSCP_TO_PBITS_TABLE_12, 1, 64, 1 },
#endif
#if defined BCM6858
	{ "VPORT_CFG_EX_TABLE", 1, CORE_12_INDEX, &VPORT_CFG_EX_TABLE_12, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "PORT_MAC", 1, CORE_12_INDEX, &PORT_MAC_12, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_12_INDEX, &INGRESS_FILTER_PROFILE_TABLE_12, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "DUAL_STACK_LITE_TABLE", 1, CORE_12_INDEX, &DUAL_STACK_LITE_TABLE_12, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "QUEUE_THRESHOLD_VECTOR", 1, CORE_12_INDEX, &QUEUE_THRESHOLD_VECTOR_12, 9, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_PACKET_BASED_MAPPING", 1, CORE_12_INDEX, &INGRESS_PACKET_BASED_MAPPING_12, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "PROCESSING_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_12_INDEX, &PROCESSING_QUEUE_DYNAMIC_MNG_TABLE_12, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SPDSVC_ANALYZER_PARAMS_TABLE", 1, CORE_12_INDEX, &SPDSVC_ANALYZER_PARAMS_TABLE_12, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_12_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_12, 128, 1, 1 },
#endif
#if defined BCM6858
	{ "DBG_DUMP_TABLE", 1, CORE_12_INDEX, &DBG_DUMP_TABLE_12, 128, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_TBL_CFG", 1, CORE_12_INDEX, &NATC_TBL_CFG_12, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_COUNTERS_GENERIC_CONTEXT", 1, CORE_12_INDEX, &NATC_COUNTERS_GENERIC_CONTEXT_12, 128, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_FLOW_CONTEXT_DDR_ADDR", 1, CORE_12_INDEX, &RX_FLOW_CONTEXT_DDR_ADDR_12, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "BRIDGE_CFG_TABLE", 1, CORE_12_INDEX, &BRIDGE_CFG_TABLE_12, 2, 1, 1 },
#endif
#if defined BCM6858
	{ "VPORT_PBIT_TO_DISCARD_PRIO_VECTOR", 1, CORE_12_INDEX, &VPORT_PBIT_TO_DISCARD_PRIO_VECTOR_12, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_12_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_12, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "MAX_PKT_LEN_TABLE", 1, CORE_12_INDEX, &MAX_PKT_LEN_TABLE_12, 32, 1, 1 },
#endif
#if defined BCM6858
	{ "REGISTERS_BUFFER", 1, CORE_12_INDEX, &REGISTERS_BUFFER_12, 32, 1, 1 },
#endif
#if defined BCM6858
	{ "TCAM_IC_CFG_TABLE", 1, CORE_12_INDEX, &TCAM_IC_CFG_TABLE_12, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "VLAN_ACTION_GPE_HANDLER_PTR_TABLE", 1, CORE_12_INDEX, &VLAN_ACTION_GPE_HANDLER_PTR_TABLE_12, 17, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_BBH_OVERRUN_MAX_TASKS_LIMIT", 1, CORE_12_INDEX, &MCAST_BBH_OVERRUN_MAX_TASKS_LIMIT_12, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SPDTEST_GEN_PARAM", 1, CORE_12_INDEX, &SPDTEST_GEN_PARAM_12, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "NULL_BUFFER", 1, CORE_12_INDEX, &NULL_BUFFER_12, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPTV_CONFIGURATION_TABLE", 1, CORE_12_INDEX, &IPTV_CONFIGURATION_TABLE_12, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "ZERO_VALUE", 1, CORE_12_INDEX, &ZERO_VALUE_12, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TUNNEL_TABLE", 1, CORE_12_INDEX, &TUNNEL_TABLE_12, 2, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_FEED_RING_DESCRIPTOR_TABLE", 1, CORE_12_INDEX, &CPU_FEED_RING_DESCRIPTOR_TABLE_12, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEBUG_PRINT_TABLE", 1, CORE_12_INDEX, &DEBUG_PRINT_TABLE_12, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "FPM_GLOBAL_CFG", 1, CORE_12_INDEX, &FPM_GLOBAL_CFG_12, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "ONE_VALUE", 1, CORE_12_INDEX, &ONE_VALUE_12, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_FPM_THRESHOLDS", 1, CORE_12_INDEX, &DHD_FPM_THRESHOLDS_12, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_BBH_OVERRUN_MIN_TASKS_LIMIT", 1, CORE_12_INDEX, &MCAST_BBH_OVERRUN_MIN_TASKS_LIMIT_12, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SYSTEM_CONFIGURATION", 1, CORE_12_INDEX, &SYSTEM_CONFIGURATION_12, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_12_INDEX, &LOOPBACK_WAN_FLOW_TABLE_12, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "FORCE_DSCP", 1, CORE_12_INDEX, &FORCE_DSCP_12, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CORE_ID_TABLE", 1, CORE_12_INDEX, &CORE_ID_TABLE_12, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TUNNEL_HEADER_PSRAM_BUFFER", 1, CORE_12_INDEX, &TUNNEL_HEADER_PSRAM_BUFFER_12, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TASK_IDX", 1, CORE_12_INDEX, &TASK_IDX_12, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TUNNELS_PARSING_CFG", 1, CORE_12_INDEX, &TUNNELS_PARSING_CFG_12, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPTV_CFG_TABLE", 1, CORE_12_INDEX, &IPTV_CFG_TABLE_12, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_12_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_12, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEFAULT_VLAN_VID_TABLE", 1, CORE_12_INDEX, &DEFAULT_VLAN_VID_TABLE_12, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "NAT_CACHE_CFG", 1, CORE_12_INDEX, &NAT_CACHE_CFG_12, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SRAM_DUMMY_STORE", 1, CORE_12_INDEX, &SRAM_DUMMY_STORE_12, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_12_INDEX, &NAT_CACHE_KEY0_MASK_12, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_12_INDEX, &RX_MIRRORING_CONFIGURATION_12, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RATE_LIMIT_OVERHEAD", 1, CORE_12_INDEX, &RATE_LIMIT_OVERHEAD_12, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_QOS_DONT_DROP_RATIO", 1, CORE_12_INDEX, &INGRESS_QOS_DONT_DROP_RATIO_12, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_12_INDEX, &NATC_L2_VLAN_KEY_MASK_12, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_QOS_WAN_UNTAGGED_PRIORITY", 1, CORE_12_INDEX, &INGRESS_QOS_WAN_UNTAGGED_PRIORITY_12, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IC_MCAST_ENABLE", 1, CORE_12_INDEX, &IC_MCAST_ENABLE_12, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "ECN_IPV6_REMARK_TABLE", 1, CORE_12_INDEX, &ECN_IPV6_REMARK_TABLE_12, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_REDIRECT_MODE", 1, CORE_12_INDEX, &CPU_REDIRECT_MODE_12, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_12_INDEX, &DEBUG_PRINT_CORE_LOCK_12, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_12_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_12, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_FILTER_CFG", 1, CORE_12_INDEX, &INGRESS_FILTER_CFG_12, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_12_INDEX, &INGRESS_FILTER_1588_CFG_12, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_12_INDEX, &RX_MIRRORING_DIRECT_ENABLE_12, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_L2_TOS_MASK", 1, CORE_12_INDEX, &NATC_L2_TOS_MASK_12, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEFAULT_VLAN_VID_TO_TX_VLAN_CNTR_TABLE", 1, CORE_12_INDEX, &DEFAULT_VLAN_VID_TO_TX_VLAN_CNTR_TABLE_12, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "DS_PACKET_BUFFER", 1, CORE_13_INDEX, &DS_PACKET_BUFFER_13, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_FLOW_TABLE", 1, CORE_13_INDEX, &RX_FLOW_TABLE_13, 320, 1, 1 },
#endif
#if defined BCM6858
	{ "VPORT_CFG_TABLE", 1, CORE_13_INDEX, &VPORT_CFG_TABLE_13, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "DSCP_TO_PBITS_MAP_TABLE", 1, CORE_13_INDEX, &DSCP_TO_PBITS_MAP_TABLE_13, 4, 64, 1 },
#endif
#if defined BCM6858
	{ "TCAM_IC_CMD_TABLE", 1, CORE_13_INDEX, &TCAM_IC_CMD_TABLE_13, 9, 16, 1 },
#endif
#if defined BCM6858
	{ "MCAST_PROCESSING_TASKS_REPLY", 1, CORE_13_INDEX, &MCAST_PROCESSING_TASKS_REPLY_13, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "PBIT_TO_GEM_TABLE", 1, CORE_13_INDEX, &PBIT_TO_GEM_TABLE_13, 16, 8, 1 },
#endif
#if defined BCM6858
	{ "TX_FLOW_TABLE", 1, CORE_13_INDEX, &TX_FLOW_TABLE_13, 192, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_PROCESSING_TASKS_LOCK", 1, CORE_13_INDEX, &MCAST_PROCESSING_TASKS_LOCK_13, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "TC_TO_QUEUE_TABLE", 1, CORE_13_INDEX, &TC_TO_QUEUE_TABLE_13, 65, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_FPM_TOKENS_READ_FLAG", 1, CORE_13_INDEX, &MCAST_FPM_TOKENS_READ_FLAG_13, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "VPORT_TO_DSCP_TO_PBITS_TABLE", 1, CORE_13_INDEX, &VPORT_TO_DSCP_TO_PBITS_TABLE_13, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_13_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_13, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_BBH_OVERRUN_TASKS_LIMIT", 1, CORE_13_INDEX, &MCAST_BBH_OVERRUN_TASKS_LIMIT_13, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "LAYER2_HEADER_COPY_MAPPING_TABLE", 1, CORE_13_INDEX, &LAYER2_HEADER_COPY_MAPPING_TABLE_13, 30, 1, 1 },
#endif
#if defined BCM6858
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_13_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_13, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "EMAC_FLOW_CTRL", 1, CORE_13_INDEX, &EMAC_FLOW_CTRL_13, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "POLICER_PARAMS_TABLE", 1, CORE_13_INDEX, &POLICER_PARAMS_TABLE_13, 80, 1, 1 },
#endif
#if defined BCM6858
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_13_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_13, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "FLOODING_HEADER_FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_13_INDEX, &FLOODING_HEADER_FPM_POOL_NUMBER_MAPPING_TABLE_13, 32, 1, 1 },
#endif
#if defined BCM6858
	{ "PBIT_TO_QUEUE_TABLE", 1, CORE_13_INDEX, &PBIT_TO_QUEUE_TABLE_13, 65, 1, 1 },
#endif
#if defined BCM6858
	{ "DEBUG_SCRATCHPAD", 1, CORE_13_INDEX, &DEBUG_SCRATCHPAD_13, 10, 1, 1 },
#endif
#if defined BCM6858
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_13_INDEX, &LOOPBACK_QUEUE_TABLE_13, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "GLOBAL_DSCP_TO_PBITS_TABLE", 1, CORE_13_INDEX, &GLOBAL_DSCP_TO_PBITS_TABLE_13, 1, 64, 1 },
#endif
#if defined BCM6858
	{ "VPORT_CFG_EX_TABLE", 1, CORE_13_INDEX, &VPORT_CFG_EX_TABLE_13, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "PORT_MAC", 1, CORE_13_INDEX, &PORT_MAC_13, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_13_INDEX, &INGRESS_FILTER_PROFILE_TABLE_13, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "DUAL_STACK_LITE_TABLE", 1, CORE_13_INDEX, &DUAL_STACK_LITE_TABLE_13, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "QUEUE_THRESHOLD_VECTOR", 1, CORE_13_INDEX, &QUEUE_THRESHOLD_VECTOR_13, 9, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_PACKET_BASED_MAPPING", 1, CORE_13_INDEX, &INGRESS_PACKET_BASED_MAPPING_13, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "PROCESSING_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_13_INDEX, &PROCESSING_QUEUE_DYNAMIC_MNG_TABLE_13, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SPDSVC_ANALYZER_PARAMS_TABLE", 1, CORE_13_INDEX, &SPDSVC_ANALYZER_PARAMS_TABLE_13, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_13_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_13, 128, 1, 1 },
#endif
#if defined BCM6858
	{ "DBG_DUMP_TABLE", 1, CORE_13_INDEX, &DBG_DUMP_TABLE_13, 128, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_TBL_CFG", 1, CORE_13_INDEX, &NATC_TBL_CFG_13, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_COUNTERS_GENERIC_CONTEXT", 1, CORE_13_INDEX, &NATC_COUNTERS_GENERIC_CONTEXT_13, 128, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_FLOW_CONTEXT_DDR_ADDR", 1, CORE_13_INDEX, &RX_FLOW_CONTEXT_DDR_ADDR_13, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "BRIDGE_CFG_TABLE", 1, CORE_13_INDEX, &BRIDGE_CFG_TABLE_13, 2, 1, 1 },
#endif
#if defined BCM6858
	{ "VPORT_PBIT_TO_DISCARD_PRIO_VECTOR", 1, CORE_13_INDEX, &VPORT_PBIT_TO_DISCARD_PRIO_VECTOR_13, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_13_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_13, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "MAX_PKT_LEN_TABLE", 1, CORE_13_INDEX, &MAX_PKT_LEN_TABLE_13, 32, 1, 1 },
#endif
#if defined BCM6858
	{ "REGISTERS_BUFFER", 1, CORE_13_INDEX, &REGISTERS_BUFFER_13, 32, 1, 1 },
#endif
#if defined BCM6858
	{ "TCAM_IC_CFG_TABLE", 1, CORE_13_INDEX, &TCAM_IC_CFG_TABLE_13, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "VLAN_ACTION_GPE_HANDLER_PTR_TABLE", 1, CORE_13_INDEX, &VLAN_ACTION_GPE_HANDLER_PTR_TABLE_13, 17, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_BBH_OVERRUN_MAX_TASKS_LIMIT", 1, CORE_13_INDEX, &MCAST_BBH_OVERRUN_MAX_TASKS_LIMIT_13, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SPDTEST_GEN_PARAM", 1, CORE_13_INDEX, &SPDTEST_GEN_PARAM_13, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "NULL_BUFFER", 1, CORE_13_INDEX, &NULL_BUFFER_13, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPTV_CONFIGURATION_TABLE", 1, CORE_13_INDEX, &IPTV_CONFIGURATION_TABLE_13, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "ZERO_VALUE", 1, CORE_13_INDEX, &ZERO_VALUE_13, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TUNNEL_TABLE", 1, CORE_13_INDEX, &TUNNEL_TABLE_13, 2, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_FEED_RING_DESCRIPTOR_TABLE", 1, CORE_13_INDEX, &CPU_FEED_RING_DESCRIPTOR_TABLE_13, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEBUG_PRINT_TABLE", 1, CORE_13_INDEX, &DEBUG_PRINT_TABLE_13, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "FPM_GLOBAL_CFG", 1, CORE_13_INDEX, &FPM_GLOBAL_CFG_13, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "ONE_VALUE", 1, CORE_13_INDEX, &ONE_VALUE_13, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_FPM_THRESHOLDS", 1, CORE_13_INDEX, &DHD_FPM_THRESHOLDS_13, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_BBH_OVERRUN_MIN_TASKS_LIMIT", 1, CORE_13_INDEX, &MCAST_BBH_OVERRUN_MIN_TASKS_LIMIT_13, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SYSTEM_CONFIGURATION", 1, CORE_13_INDEX, &SYSTEM_CONFIGURATION_13, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_13_INDEX, &LOOPBACK_WAN_FLOW_TABLE_13, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "FORCE_DSCP", 1, CORE_13_INDEX, &FORCE_DSCP_13, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CORE_ID_TABLE", 1, CORE_13_INDEX, &CORE_ID_TABLE_13, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TUNNEL_HEADER_PSRAM_BUFFER", 1, CORE_13_INDEX, &TUNNEL_HEADER_PSRAM_BUFFER_13, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TASK_IDX", 1, CORE_13_INDEX, &TASK_IDX_13, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TUNNELS_PARSING_CFG", 1, CORE_13_INDEX, &TUNNELS_PARSING_CFG_13, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPTV_CFG_TABLE", 1, CORE_13_INDEX, &IPTV_CFG_TABLE_13, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_13_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_13, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEFAULT_VLAN_VID_TABLE", 1, CORE_13_INDEX, &DEFAULT_VLAN_VID_TABLE_13, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "NAT_CACHE_CFG", 1, CORE_13_INDEX, &NAT_CACHE_CFG_13, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SRAM_DUMMY_STORE", 1, CORE_13_INDEX, &SRAM_DUMMY_STORE_13, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_13_INDEX, &NAT_CACHE_KEY0_MASK_13, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_13_INDEX, &RX_MIRRORING_CONFIGURATION_13, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RATE_LIMIT_OVERHEAD", 1, CORE_13_INDEX, &RATE_LIMIT_OVERHEAD_13, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_QOS_DONT_DROP_RATIO", 1, CORE_13_INDEX, &INGRESS_QOS_DONT_DROP_RATIO_13, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_13_INDEX, &NATC_L2_VLAN_KEY_MASK_13, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_QOS_WAN_UNTAGGED_PRIORITY", 1, CORE_13_INDEX, &INGRESS_QOS_WAN_UNTAGGED_PRIORITY_13, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IC_MCAST_ENABLE", 1, CORE_13_INDEX, &IC_MCAST_ENABLE_13, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "ECN_IPV6_REMARK_TABLE", 1, CORE_13_INDEX, &ECN_IPV6_REMARK_TABLE_13, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_REDIRECT_MODE", 1, CORE_13_INDEX, &CPU_REDIRECT_MODE_13, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_13_INDEX, &DEBUG_PRINT_CORE_LOCK_13, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_13_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_13, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_FILTER_CFG", 1, CORE_13_INDEX, &INGRESS_FILTER_CFG_13, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_13_INDEX, &INGRESS_FILTER_1588_CFG_13, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_13_INDEX, &RX_MIRRORING_DIRECT_ENABLE_13, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_L2_TOS_MASK", 1, CORE_13_INDEX, &NATC_L2_TOS_MASK_13, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEFAULT_VLAN_VID_TO_TX_VLAN_CNTR_TABLE", 1, CORE_13_INDEX, &DEFAULT_VLAN_VID_TO_TX_VLAN_CNTR_TABLE_13, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "DS_PACKET_BUFFER", 1, CORE_14_INDEX, &DS_PACKET_BUFFER_14, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_FLOW_TABLE", 1, CORE_14_INDEX, &RX_FLOW_TABLE_14, 320, 1, 1 },
#endif
#if defined BCM6858
	{ "VPORT_CFG_TABLE", 1, CORE_14_INDEX, &VPORT_CFG_TABLE_14, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "DSCP_TO_PBITS_MAP_TABLE", 1, CORE_14_INDEX, &DSCP_TO_PBITS_MAP_TABLE_14, 4, 64, 1 },
#endif
#if defined BCM6858
	{ "TCAM_IC_CMD_TABLE", 1, CORE_14_INDEX, &TCAM_IC_CMD_TABLE_14, 9, 16, 1 },
#endif
#if defined BCM6858
	{ "MCAST_PROCESSING_TASKS_REPLY", 1, CORE_14_INDEX, &MCAST_PROCESSING_TASKS_REPLY_14, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "PBIT_TO_GEM_TABLE", 1, CORE_14_INDEX, &PBIT_TO_GEM_TABLE_14, 16, 8, 1 },
#endif
#if defined BCM6858
	{ "TX_FLOW_TABLE", 1, CORE_14_INDEX, &TX_FLOW_TABLE_14, 192, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_PROCESSING_TASKS_LOCK", 1, CORE_14_INDEX, &MCAST_PROCESSING_TASKS_LOCK_14, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "TC_TO_QUEUE_TABLE", 1, CORE_14_INDEX, &TC_TO_QUEUE_TABLE_14, 65, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_FPM_TOKENS_READ_FLAG", 1, CORE_14_INDEX, &MCAST_FPM_TOKENS_READ_FLAG_14, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "VPORT_TO_DSCP_TO_PBITS_TABLE", 1, CORE_14_INDEX, &VPORT_TO_DSCP_TO_PBITS_TABLE_14, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_14_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_14, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_BBH_OVERRUN_TASKS_LIMIT", 1, CORE_14_INDEX, &MCAST_BBH_OVERRUN_TASKS_LIMIT_14, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "LAYER2_HEADER_COPY_MAPPING_TABLE", 1, CORE_14_INDEX, &LAYER2_HEADER_COPY_MAPPING_TABLE_14, 30, 1, 1 },
#endif
#if defined BCM6858
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_14_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_14, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "EMAC_FLOW_CTRL", 1, CORE_14_INDEX, &EMAC_FLOW_CTRL_14, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "POLICER_PARAMS_TABLE", 1, CORE_14_INDEX, &POLICER_PARAMS_TABLE_14, 80, 1, 1 },
#endif
#if defined BCM6858
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_14_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_14, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "FLOODING_HEADER_FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_14_INDEX, &FLOODING_HEADER_FPM_POOL_NUMBER_MAPPING_TABLE_14, 32, 1, 1 },
#endif
#if defined BCM6858
	{ "PBIT_TO_QUEUE_TABLE", 1, CORE_14_INDEX, &PBIT_TO_QUEUE_TABLE_14, 65, 1, 1 },
#endif
#if defined BCM6858
	{ "DEBUG_SCRATCHPAD", 1, CORE_14_INDEX, &DEBUG_SCRATCHPAD_14, 10, 1, 1 },
#endif
#if defined BCM6858
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_14_INDEX, &LOOPBACK_QUEUE_TABLE_14, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "GLOBAL_DSCP_TO_PBITS_TABLE", 1, CORE_14_INDEX, &GLOBAL_DSCP_TO_PBITS_TABLE_14, 1, 64, 1 },
#endif
#if defined BCM6858
	{ "VPORT_CFG_EX_TABLE", 1, CORE_14_INDEX, &VPORT_CFG_EX_TABLE_14, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "PORT_MAC", 1, CORE_14_INDEX, &PORT_MAC_14, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_14_INDEX, &INGRESS_FILTER_PROFILE_TABLE_14, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "DUAL_STACK_LITE_TABLE", 1, CORE_14_INDEX, &DUAL_STACK_LITE_TABLE_14, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "QUEUE_THRESHOLD_VECTOR", 1, CORE_14_INDEX, &QUEUE_THRESHOLD_VECTOR_14, 9, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_PACKET_BASED_MAPPING", 1, CORE_14_INDEX, &INGRESS_PACKET_BASED_MAPPING_14, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "PROCESSING_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_14_INDEX, &PROCESSING_QUEUE_DYNAMIC_MNG_TABLE_14, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SPDSVC_ANALYZER_PARAMS_TABLE", 1, CORE_14_INDEX, &SPDSVC_ANALYZER_PARAMS_TABLE_14, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_14_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_14, 128, 1, 1 },
#endif
#if defined BCM6858
	{ "DBG_DUMP_TABLE", 1, CORE_14_INDEX, &DBG_DUMP_TABLE_14, 128, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_TBL_CFG", 1, CORE_14_INDEX, &NATC_TBL_CFG_14, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_COUNTERS_GENERIC_CONTEXT", 1, CORE_14_INDEX, &NATC_COUNTERS_GENERIC_CONTEXT_14, 128, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_FLOW_CONTEXT_DDR_ADDR", 1, CORE_14_INDEX, &RX_FLOW_CONTEXT_DDR_ADDR_14, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "BRIDGE_CFG_TABLE", 1, CORE_14_INDEX, &BRIDGE_CFG_TABLE_14, 2, 1, 1 },
#endif
#if defined BCM6858
	{ "VPORT_PBIT_TO_DISCARD_PRIO_VECTOR", 1, CORE_14_INDEX, &VPORT_PBIT_TO_DISCARD_PRIO_VECTOR_14, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_14_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_14, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "MAX_PKT_LEN_TABLE", 1, CORE_14_INDEX, &MAX_PKT_LEN_TABLE_14, 32, 1, 1 },
#endif
#if defined BCM6858
	{ "REGISTERS_BUFFER", 1, CORE_14_INDEX, &REGISTERS_BUFFER_14, 32, 1, 1 },
#endif
#if defined BCM6858
	{ "TCAM_IC_CFG_TABLE", 1, CORE_14_INDEX, &TCAM_IC_CFG_TABLE_14, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "VLAN_ACTION_GPE_HANDLER_PTR_TABLE", 1, CORE_14_INDEX, &VLAN_ACTION_GPE_HANDLER_PTR_TABLE_14, 17, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_BBH_OVERRUN_MAX_TASKS_LIMIT", 1, CORE_14_INDEX, &MCAST_BBH_OVERRUN_MAX_TASKS_LIMIT_14, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SPDTEST_GEN_PARAM", 1, CORE_14_INDEX, &SPDTEST_GEN_PARAM_14, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "NULL_BUFFER", 1, CORE_14_INDEX, &NULL_BUFFER_14, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPTV_CONFIGURATION_TABLE", 1, CORE_14_INDEX, &IPTV_CONFIGURATION_TABLE_14, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "ZERO_VALUE", 1, CORE_14_INDEX, &ZERO_VALUE_14, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TUNNEL_TABLE", 1, CORE_14_INDEX, &TUNNEL_TABLE_14, 2, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_FEED_RING_DESCRIPTOR_TABLE", 1, CORE_14_INDEX, &CPU_FEED_RING_DESCRIPTOR_TABLE_14, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEBUG_PRINT_TABLE", 1, CORE_14_INDEX, &DEBUG_PRINT_TABLE_14, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "FPM_GLOBAL_CFG", 1, CORE_14_INDEX, &FPM_GLOBAL_CFG_14, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "ONE_VALUE", 1, CORE_14_INDEX, &ONE_VALUE_14, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_FPM_THRESHOLDS", 1, CORE_14_INDEX, &DHD_FPM_THRESHOLDS_14, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_BBH_OVERRUN_MIN_TASKS_LIMIT", 1, CORE_14_INDEX, &MCAST_BBH_OVERRUN_MIN_TASKS_LIMIT_14, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SYSTEM_CONFIGURATION", 1, CORE_14_INDEX, &SYSTEM_CONFIGURATION_14, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_14_INDEX, &LOOPBACK_WAN_FLOW_TABLE_14, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "FORCE_DSCP", 1, CORE_14_INDEX, &FORCE_DSCP_14, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CORE_ID_TABLE", 1, CORE_14_INDEX, &CORE_ID_TABLE_14, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TUNNEL_HEADER_PSRAM_BUFFER", 1, CORE_14_INDEX, &TUNNEL_HEADER_PSRAM_BUFFER_14, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TASK_IDX", 1, CORE_14_INDEX, &TASK_IDX_14, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TUNNELS_PARSING_CFG", 1, CORE_14_INDEX, &TUNNELS_PARSING_CFG_14, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPTV_CFG_TABLE", 1, CORE_14_INDEX, &IPTV_CFG_TABLE_14, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_14_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_14, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEFAULT_VLAN_VID_TABLE", 1, CORE_14_INDEX, &DEFAULT_VLAN_VID_TABLE_14, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "NAT_CACHE_CFG", 1, CORE_14_INDEX, &NAT_CACHE_CFG_14, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SRAM_DUMMY_STORE", 1, CORE_14_INDEX, &SRAM_DUMMY_STORE_14, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_14_INDEX, &NAT_CACHE_KEY0_MASK_14, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_14_INDEX, &RX_MIRRORING_CONFIGURATION_14, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RATE_LIMIT_OVERHEAD", 1, CORE_14_INDEX, &RATE_LIMIT_OVERHEAD_14, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_QOS_DONT_DROP_RATIO", 1, CORE_14_INDEX, &INGRESS_QOS_DONT_DROP_RATIO_14, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_14_INDEX, &NATC_L2_VLAN_KEY_MASK_14, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_QOS_WAN_UNTAGGED_PRIORITY", 1, CORE_14_INDEX, &INGRESS_QOS_WAN_UNTAGGED_PRIORITY_14, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IC_MCAST_ENABLE", 1, CORE_14_INDEX, &IC_MCAST_ENABLE_14, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "ECN_IPV6_REMARK_TABLE", 1, CORE_14_INDEX, &ECN_IPV6_REMARK_TABLE_14, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_REDIRECT_MODE", 1, CORE_14_INDEX, &CPU_REDIRECT_MODE_14, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_14_INDEX, &DEBUG_PRINT_CORE_LOCK_14, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_14_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_14, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_FILTER_CFG", 1, CORE_14_INDEX, &INGRESS_FILTER_CFG_14, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_14_INDEX, &INGRESS_FILTER_1588_CFG_14, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_14_INDEX, &RX_MIRRORING_DIRECT_ENABLE_14, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_L2_TOS_MASK", 1, CORE_14_INDEX, &NATC_L2_TOS_MASK_14, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEFAULT_VLAN_VID_TO_TX_VLAN_CNTR_TABLE", 1, CORE_14_INDEX, &DEFAULT_VLAN_VID_TO_TX_VLAN_CNTR_TABLE_14, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "DS_PACKET_BUFFER", 1, CORE_15_INDEX, &DS_PACKET_BUFFER_15, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_FLOW_TABLE", 1, CORE_15_INDEX, &RX_FLOW_TABLE_15, 320, 1, 1 },
#endif
#if defined BCM6858
	{ "VPORT_CFG_TABLE", 1, CORE_15_INDEX, &VPORT_CFG_TABLE_15, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "DSCP_TO_PBITS_MAP_TABLE", 1, CORE_15_INDEX, &DSCP_TO_PBITS_MAP_TABLE_15, 4, 64, 1 },
#endif
#if defined BCM6858
	{ "TCAM_IC_CMD_TABLE", 1, CORE_15_INDEX, &TCAM_IC_CMD_TABLE_15, 9, 16, 1 },
#endif
#if defined BCM6858
	{ "MCAST_PROCESSING_TASKS_REPLY", 1, CORE_15_INDEX, &MCAST_PROCESSING_TASKS_REPLY_15, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "PBIT_TO_GEM_TABLE", 1, CORE_15_INDEX, &PBIT_TO_GEM_TABLE_15, 16, 8, 1 },
#endif
#if defined BCM6858
	{ "TX_FLOW_TABLE", 1, CORE_15_INDEX, &TX_FLOW_TABLE_15, 192, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_PROCESSING_TASKS_LOCK", 1, CORE_15_INDEX, &MCAST_PROCESSING_TASKS_LOCK_15, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "TC_TO_QUEUE_TABLE", 1, CORE_15_INDEX, &TC_TO_QUEUE_TABLE_15, 65, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_FPM_TOKENS_READ_FLAG", 1, CORE_15_INDEX, &MCAST_FPM_TOKENS_READ_FLAG_15, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "VPORT_TO_DSCP_TO_PBITS_TABLE", 1, CORE_15_INDEX, &VPORT_TO_DSCP_TO_PBITS_TABLE_15, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_15_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_15, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_BBH_OVERRUN_TASKS_LIMIT", 1, CORE_15_INDEX, &MCAST_BBH_OVERRUN_TASKS_LIMIT_15, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "LAYER2_HEADER_COPY_MAPPING_TABLE", 1, CORE_15_INDEX, &LAYER2_HEADER_COPY_MAPPING_TABLE_15, 30, 1, 1 },
#endif
#if defined BCM6858
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_15_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_15, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "EMAC_FLOW_CTRL", 1, CORE_15_INDEX, &EMAC_FLOW_CTRL_15, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "POLICER_PARAMS_TABLE", 1, CORE_15_INDEX, &POLICER_PARAMS_TABLE_15, 80, 1, 1 },
#endif
#if defined BCM6858
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_15_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_15, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "FLOODING_HEADER_FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_15_INDEX, &FLOODING_HEADER_FPM_POOL_NUMBER_MAPPING_TABLE_15, 32, 1, 1 },
#endif
#if defined BCM6858
	{ "PBIT_TO_QUEUE_TABLE", 1, CORE_15_INDEX, &PBIT_TO_QUEUE_TABLE_15, 65, 1, 1 },
#endif
#if defined BCM6858
	{ "DEBUG_SCRATCHPAD", 1, CORE_15_INDEX, &DEBUG_SCRATCHPAD_15, 10, 1, 1 },
#endif
#if defined BCM6858
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_15_INDEX, &LOOPBACK_QUEUE_TABLE_15, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "GLOBAL_DSCP_TO_PBITS_TABLE", 1, CORE_15_INDEX, &GLOBAL_DSCP_TO_PBITS_TABLE_15, 1, 64, 1 },
#endif
#if defined BCM6858
	{ "VPORT_CFG_EX_TABLE", 1, CORE_15_INDEX, &VPORT_CFG_EX_TABLE_15, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "PORT_MAC", 1, CORE_15_INDEX, &PORT_MAC_15, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_15_INDEX, &INGRESS_FILTER_PROFILE_TABLE_15, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "DUAL_STACK_LITE_TABLE", 1, CORE_15_INDEX, &DUAL_STACK_LITE_TABLE_15, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "QUEUE_THRESHOLD_VECTOR", 1, CORE_15_INDEX, &QUEUE_THRESHOLD_VECTOR_15, 9, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_PACKET_BASED_MAPPING", 1, CORE_15_INDEX, &INGRESS_PACKET_BASED_MAPPING_15, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "PROCESSING_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_15_INDEX, &PROCESSING_QUEUE_DYNAMIC_MNG_TABLE_15, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SPDSVC_ANALYZER_PARAMS_TABLE", 1, CORE_15_INDEX, &SPDSVC_ANALYZER_PARAMS_TABLE_15, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_15_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_15, 128, 1, 1 },
#endif
#if defined BCM6858
	{ "DBG_DUMP_TABLE", 1, CORE_15_INDEX, &DBG_DUMP_TABLE_15, 128, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_TBL_CFG", 1, CORE_15_INDEX, &NATC_TBL_CFG_15, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_COUNTERS_GENERIC_CONTEXT", 1, CORE_15_INDEX, &NATC_COUNTERS_GENERIC_CONTEXT_15, 128, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_FLOW_CONTEXT_DDR_ADDR", 1, CORE_15_INDEX, &RX_FLOW_CONTEXT_DDR_ADDR_15, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "BRIDGE_CFG_TABLE", 1, CORE_15_INDEX, &BRIDGE_CFG_TABLE_15, 2, 1, 1 },
#endif
#if defined BCM6858
	{ "VPORT_PBIT_TO_DISCARD_PRIO_VECTOR", 1, CORE_15_INDEX, &VPORT_PBIT_TO_DISCARD_PRIO_VECTOR_15, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_15_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_15, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "MAX_PKT_LEN_TABLE", 1, CORE_15_INDEX, &MAX_PKT_LEN_TABLE_15, 32, 1, 1 },
#endif
#if defined BCM6858
	{ "REGISTERS_BUFFER", 1, CORE_15_INDEX, &REGISTERS_BUFFER_15, 32, 1, 1 },
#endif
#if defined BCM6858
	{ "TCAM_IC_CFG_TABLE", 1, CORE_15_INDEX, &TCAM_IC_CFG_TABLE_15, 8, 1, 1 },
#endif
#if defined BCM6858
	{ "VLAN_ACTION_GPE_HANDLER_PTR_TABLE", 1, CORE_15_INDEX, &VLAN_ACTION_GPE_HANDLER_PTR_TABLE_15, 17, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_BBH_OVERRUN_MAX_TASKS_LIMIT", 1, CORE_15_INDEX, &MCAST_BBH_OVERRUN_MAX_TASKS_LIMIT_15, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SPDTEST_GEN_PARAM", 1, CORE_15_INDEX, &SPDTEST_GEN_PARAM_15, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "NULL_BUFFER", 1, CORE_15_INDEX, &NULL_BUFFER_15, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPTV_CONFIGURATION_TABLE", 1, CORE_15_INDEX, &IPTV_CONFIGURATION_TABLE_15, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "ZERO_VALUE", 1, CORE_15_INDEX, &ZERO_VALUE_15, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TUNNEL_TABLE", 1, CORE_15_INDEX, &TUNNEL_TABLE_15, 2, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_FEED_RING_DESCRIPTOR_TABLE", 1, CORE_15_INDEX, &CPU_FEED_RING_DESCRIPTOR_TABLE_15, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEBUG_PRINT_TABLE", 1, CORE_15_INDEX, &DEBUG_PRINT_TABLE_15, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "FPM_GLOBAL_CFG", 1, CORE_15_INDEX, &FPM_GLOBAL_CFG_15, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "ONE_VALUE", 1, CORE_15_INDEX, &ONE_VALUE_15, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DHD_FPM_THRESHOLDS", 1, CORE_15_INDEX, &DHD_FPM_THRESHOLDS_15, 3, 1, 1 },
#endif
#if defined BCM6858
	{ "MCAST_BBH_OVERRUN_MIN_TASKS_LIMIT", 1, CORE_15_INDEX, &MCAST_BBH_OVERRUN_MIN_TASKS_LIMIT_15, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SYSTEM_CONFIGURATION", 1, CORE_15_INDEX, &SYSTEM_CONFIGURATION_15, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_15_INDEX, &LOOPBACK_WAN_FLOW_TABLE_15, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "FORCE_DSCP", 1, CORE_15_INDEX, &FORCE_DSCP_15, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CORE_ID_TABLE", 1, CORE_15_INDEX, &CORE_ID_TABLE_15, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TUNNEL_HEADER_PSRAM_BUFFER", 1, CORE_15_INDEX, &TUNNEL_HEADER_PSRAM_BUFFER_15, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TASK_IDX", 1, CORE_15_INDEX, &TASK_IDX_15, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "TUNNELS_PARSING_CFG", 1, CORE_15_INDEX, &TUNNELS_PARSING_CFG_15, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPTV_CFG_TABLE", 1, CORE_15_INDEX, &IPTV_CFG_TABLE_15, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_15_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_15, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEFAULT_VLAN_VID_TABLE", 1, CORE_15_INDEX, &DEFAULT_VLAN_VID_TABLE_15, 16, 1, 1 },
#endif
#if defined BCM6858
	{ "NAT_CACHE_CFG", 1, CORE_15_INDEX, &NAT_CACHE_CFG_15, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "SRAM_DUMMY_STORE", 1, CORE_15_INDEX, &SRAM_DUMMY_STORE_15, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_15_INDEX, &NAT_CACHE_KEY0_MASK_15, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_15_INDEX, &RX_MIRRORING_CONFIGURATION_15, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RATE_LIMIT_OVERHEAD", 1, CORE_15_INDEX, &RATE_LIMIT_OVERHEAD_15, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_QOS_DONT_DROP_RATIO", 1, CORE_15_INDEX, &INGRESS_QOS_DONT_DROP_RATIO_15, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_15_INDEX, &NATC_L2_VLAN_KEY_MASK_15, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_QOS_WAN_UNTAGGED_PRIORITY", 1, CORE_15_INDEX, &INGRESS_QOS_WAN_UNTAGGED_PRIORITY_15, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IC_MCAST_ENABLE", 1, CORE_15_INDEX, &IC_MCAST_ENABLE_15, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "ECN_IPV6_REMARK_TABLE", 1, CORE_15_INDEX, &ECN_IPV6_REMARK_TABLE_15, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "CPU_REDIRECT_MODE", 1, CORE_15_INDEX, &CPU_REDIRECT_MODE_15, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_15_INDEX, &DEBUG_PRINT_CORE_LOCK_15, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_15_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_15, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_FILTER_CFG", 1, CORE_15_INDEX, &INGRESS_FILTER_CFG_15, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_15_INDEX, &INGRESS_FILTER_1588_CFG_15, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_15_INDEX, &RX_MIRRORING_DIRECT_ENABLE_15, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "NATC_L2_TOS_MASK", 1, CORE_15_INDEX, &NATC_L2_TOS_MASK_15, 1, 1, 1 },
#endif
#if defined BCM6858
	{ "DEFAULT_VLAN_VID_TO_TX_VLAN_CNTR_TABLE", 1, CORE_15_INDEX, &DEFAULT_VLAN_VID_TO_TX_VLAN_CNTR_TABLE_15, 16, 1, 1 },
#endif
};
