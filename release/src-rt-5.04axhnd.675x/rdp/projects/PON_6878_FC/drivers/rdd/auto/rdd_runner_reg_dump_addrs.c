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
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT US_TM_PD_FIFO_TABLE =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x820 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CODEL_SQ_TABLE =
{
	2,
	{
		{ dump_RDD_CODEL_SQ_ENTRY, 0x840 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT US_TM_TM_FLOW_CNTR_TABLE =
{
	1,
	{
		{ dump_RDD_TM_FLOW_CNTR_ENTRY, 0x880 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT REPORTING_QUEUE_ACCUMULATED_TABLE =
{
	16,
	{
		{ dump_RDD_QM_QUEUE_COUNTER_DATA, 0x900 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_RATE_LIMITER_TABLE =
{
	16,
	{
		{ dump_RDD_BASIC_RATE_LIMITER_DESCRIPTOR, 0xa00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DIRECT_FLOW_CNTR_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xb20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE =
{
	8,
	{
		{ dump_RDD_SCHEDULING_QUEUE_DESCRIPTOR, 0xb40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT BASIC_SCHEDULER_TABLE_DS =
{
	16,
	{
		{ dump_RDD_BASIC_SCHEDULER_DESCRIPTOR, 0xb80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DIRECT_FLOW_PD_TABLE =
{
	16,
	{
		{ dump_RDD_PROCESSING_RX_DESCRIPTOR, 0xbd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT BUFFER_CONG_DQM_NOT_EMPTY =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0xbe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT BASIC_SCHEDULER_TABLE_US =
{
	16,
	{
		{ dump_RDD_BASIC_SCHEDULER_DESCRIPTOR, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT US_TM_CPU_TX_ABS_COUNTERS =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0xe10 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT US_TM_FLUSH_CFG_CPU_TABLE =
{
	16,
	{
		{ dump_RDD_FLUSH_CFG_ENTRY, 0xe90 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CODEL_ENABLE_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0xea0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT US_TM_WAN_0_BBH_TX_WAKE_UP_DATA_TABLE =
{
	8,
	{
		{ dump_RDD_BBH_TX_WAKE_UP_DATA_ENTRY, 0xeb8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_COMPLEX_SCHEDULER_TABLE =
{
	64,
	{
		{ dump_RDD_COMPLEX_SCHEDULER_DESCRIPTOR, 0xec0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT COMPLEX_SCHEDULER_TABLE =
{
	64,
	{
		{ dump_RDD_COMPLEX_SCHEDULER_DESCRIPTOR, 0xf00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DS_TM_PD_FIFO_TABLE =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT REPORTING_COUNTER_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DIRECT_FLOW_EPON_CONTROL_SCRATCH =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x15a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CODEL_NUM_QUEUES =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x15b6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DS_TM_BBH_TX_WAKE_UP_DATA_TABLE =
{
	8,
	{
		{ dump_RDD_BBH_TX_WAKE_UP_DATA_ENTRY, 0x15b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT US_BUFFER_CONG_MGT_CFG =
{
	64,
	{
		{ dump_RDD_BUFFER_CONG_MGT, 0x15c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT US_TM_CODEL_QUEUE_TABLE =
{
	4,
	{
		{ dump_RDD_CODEL_QUEUE_DESCRIPTOR, 0x1600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1704 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DS_TM_CPU_TX_ABS_COUNTERS =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1708 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1788 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT BBH_TX_EPON_WAKE_UP_DATA_TABLE =
{
	8,
	{
		{ dump_RDD_BBH_TX_WAKE_UP_DATA_ENTRY, 0x17b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DS_BUFFER_CONG_MGT_CFG =
{
	64,
	{
		{ dump_RDD_BUFFER_CONG_MGT, 0x17c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT BASIC_RATE_LIMITER_TABLE_DS =
{
	16,
	{
		{ dump_RDD_BASIC_RATE_LIMITER_DESCRIPTOR, 0x1800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1a80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DIRECT_FLOW_RX_MIRRORING_SCRATCHPAD =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE =
{
	8,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x1b88 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DS_TM_FLUSH_CFG_CPU_TABLE =
{
	16,
	{
		{ dump_RDD_FLUSH_CFG_ENTRY, 0x1b90 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT US_TM_SCHEDULING_QUEUE_AGING_VECTOR =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1ba0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT GHOST_REPORTING_GLOBAL_CFG =
{
	4,
	{
		{ dump_RDD_GHOST_REPORTING_GLOBAL_CFG_ENTRY, 0x1bb4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT NULL_BUFFER =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x1bb8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT BUFFER_CONG_SCRATCHPAD =
{
	8,
	{
		{ dump_RDD_BUFFER_CONG_Q_OCCUPANCY, 0x1bc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT US_TM_SCHEDULING_QUEUE_TABLE =
{
	8,
	{
		{ dump_RDD_SCHEDULING_QUEUE_DESCRIPTOR, 0x1c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT MIRRORING_SCRATCH =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x1e08 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT US_TM_FLUSH_CFG_FW_TABLE =
{
	16,
	{
		{ dump_RDD_FLUSH_CFG_ENTRY, 0x1e10 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DS_TM_SCHEDULING_QUEUE_AGING_VECTOR =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1e20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT ONE_VALUE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1e34 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT REPORT_BBH_TX_QUEUE_ID_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1e38 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CODEL_BIAS_SLOPE_TABLE =
{
	4,
	{
		{ dump_RDD_CODEL_BIAS_SLOPE, 0x1e40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DIRECT_FLOW_PAUSE_QUANTA =
{
	2,
	{
		{ dump_RDD_PAUSE_QUANTA_ENTRY, 0x1e6c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT BUDGET_ALLOCATION_TIMER_VALUE =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x1e6e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DS_TM_FLUSH_CFG_FW_TABLE =
{
	16,
	{
		{ dump_RDD_FLUSH_CFG_ENTRY, 0x1e70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT US_TM_WAN_0_BBH_TX_EGRESS_COUNTER_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1e80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DS_TM_TM_FLOW_CNTR_TABLE =
{
	1,
	{
		{ dump_RDD_TM_FLOW_CNTR_ENTRY, 0x1ea8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT US_TM_WAN_0_BB_DESTINATION_TABLE =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x1eae },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_FLUSH_CFG_FW_TABLE =
{
	16,
	{
		{ dump_RDD_FLUSH_CFG_ENTRY, 0x1eb0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT BBH_TX_EPON_INGRESS_COUNTER_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1ec0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT BBH_TX_EGRESS_REPORT_COUNTER_TABLE =
{
	8,
	{
		{ dump_RDD_BBH_TX_EGRESS_COUNTER_ENTRY, 0x1ee8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT US_TM_FLUSH_CFG_CURRENT_TABLE =
{
	16,
	{
		{ dump_RDD_FLUSH_CFG_ENTRY, 0x1ef0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT REPORTING_QUEUE_COUNTER_TABLE =
{
	2,
	{
		{ dump_RDD_REPORTING_QUEUE_COUNTER, 0x1f00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DS_TM_BB_DESTINATION_TABLE =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x1f82 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT US_TM_BBH_TX_WAN_0_FIFO_BYTES_USED =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1f84 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT US_TM_FLUSH_CFG_ENABLE_TABLE =
{
	2,
	{
		{ dump_RDD_FLUSH_CFG_ENABLE_ENTRY, 0x1f88 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_BUDGET_ALLOCATION_TIMER_VALUE =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x1f8a },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT TASK_IDX =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1f8c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DS_TM_FLUSH_CFG_CURRENT_TABLE =
{
	16,
	{
		{ dump_RDD_FLUSH_CFG_ENTRY, 0x1f90 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_SCHEDULING_QUEUE_AGING_VECTOR =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1fa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_INTERRUPT_COUNTER_MAX =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x1fb4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DIRECT_FLOW_WAN_VIQ_EXCLUSIVE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1fb6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT US_TM_TX_PAUSE_NACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1fb7 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DS_TM_FLUSH_CFG_ENABLE_TABLE =
{
	2,
	{
		{ dump_RDD_FLUSH_CFG_ENABLE_ENTRY, 0x1fb8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT BBH_TX_US_WAN_0_FIFO_BYTES_THRESHOLD =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x1fba },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT SCHEDULING_FLUSH_GLOBAL_CFG =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1fbc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT TM_VLAN_STATS_ENABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1fbd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT TX_EXCEPTION =
{
	1,
	{
		{ dump_RDD_TX_EXCEPTION_ENTRY, 0x1fbe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1fbf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_TM_ACTION_PTR_TABLE =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x1fc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x1fe2 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x1fe4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT US_TM_FLUSH_AGGREGATION_TASK_DISABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1fe6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DS_TM_FLUSH_AGGREGATION_TASK_DISABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1fe7 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_FLUSH_CFG_ENABLE_TABLE =
{
	2,
	{
		{ dump_RDD_FLUSH_CFG_ENABLE_ENTRY, 0x1fe8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1fea },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT RATE_LIMIT_OVERHEAD =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1feb },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT MAC_TYPE =
{
	1,
	{
		{ dump_RDD_MAC_TYPE_ENTRY, 0x1fec },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT WAN_0_BBH_TX_FIFO_SIZE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1fed },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT US_TM_FIRST_QUEUE_MAPPING =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1fee },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DS_TM_FIRST_QUEUE_MAPPING =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1fef },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_FLUSH_CFG_CURRENT_TABLE =
{
	16,
	{
		{ dump_RDD_FLUSH_CFG_ENTRY, 0x1ff0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT BASIC_RATE_LIMITER_TABLE_US =
{
	16,
	{
		{ dump_RDD_BASIC_RATE_LIMITER_DESCRIPTOR, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT US_TM_TX_QUEUE_DROP_TABLE =
{
	8,
	{
		{ dump_RDD_PACKETS_AND_BYTES, 0x2410 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DBG_DUMP_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2618 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT BACKUP_BBH_INGRESS_COUNTERS_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2718 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT BACKUP_BBH_EGRESS_COUNTERS_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2719 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x271a },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_FIRST_QUEUE_MAPPING =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x271b },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x271c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT BBH_TX_INGRESS_COUNTER_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x271d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT SCHEDULING_AGGREGATION_CONTEXT_VECTOR =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2720 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT US_TM_UPDATE_FIFO_TABLE =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x2740 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT GHOST_REPORTING_QUEUE_STATUS_BIT_VECTOR_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2760 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DS_TM_UPDATE_FIFO_TABLE =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x2780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT QUEUE_TO_REPORT_BIT_VECTOR =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x27a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT EPON_UPDATE_FIFO_TABLE =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x27c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_RATE_LIMITER_VALID_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x27e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT OVERALL_RATE_LIMITER_TABLE =
{
	16,
	{
		{ dump_RDD_OVERALL_RATE_LIMITER_DESCRIPTOR, 0x27f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT REPORTING_QUEUE_DESCRIPTOR_TABLE =
{
	8,
	{
		{ dump_RDD_REPORTING_QUEUE_DESCRIPTOR, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT US_TM_TX_OCTETS_COUNTERS_TABLE =
{
	8,
	{
		{ dump_RDD_PACKETS_AND_BYTES, 0x2a08 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_CODEL_QUEUE_TABLE =
{
	4,
	{
		{ dump_RDD_CODEL_QUEUE_DESCRIPTOR, 0x2a48 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT FLUSH_DISPATCHER_CREDIT_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2a70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_UPDATE_FIFO_TABLE =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x2a80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT SQ_TX_QUEUE_DROP_TABLE =
{
	8,
	{
		{ dump_RDD_PACKETS_AND_BYTES, 0x2aa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT RATE_LIMITER_VALID_TABLE_US =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2ae0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT US_TM_CODEL_DROP_DESCRIPTOR =
{
	12,
	{
		{ dump_RDD_CODEL_DROP_DESCRIPTOR, 0x2af0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT NATC_TBL_CFG =
{
	24,
	{
		{ dump_RDD_NATC_TBL_CONFIGURATION, 0x2b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DS_TM_CODEL_DROP_DESCRIPTOR =
{
	12,
	{
		{ dump_RDD_CODEL_DROP_DESCRIPTOR, 0x2b50 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT RATE_LIMITER_VALID_TABLE_DS =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2b60 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DIRECT_FLOW_PAUSE_DEBUG =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2b70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DISPATCHER_CREDIT_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2b80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_DISPATCHER_CREDIT_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2b90 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_CODEL_DROP_DESCRIPTOR =
{
	12,
	{
		{ dump_RDD_CODEL_DROP_DESCRIPTOR, 0x2ba0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_DISPATCHER_CREDIT_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2bb0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT BBH_TX_EPON_EGRESS_COUNTER_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2bc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG =
{
	16,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, 0x2bd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, 0x2be0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT XGPON_REPORT_ZERO_SENT_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2bf0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, 0x2c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_PD_FIFO_TABLE =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x2e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DS_TM_BBH_TX_EGRESS_COUNTER_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2f00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DS_TM_SCHEDULING_QUEUE_TABLE =
{
	8,
	{
		{ dump_RDD_SCHEDULING_QUEUE_DESCRIPTOR, 0x3000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DS_TM_TX_QUEUE_DROP_TABLE =
{
	8,
	{
		{ dump_RDD_PACKETS_AND_BYTES, 0x3140 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT US_TM_BBH_QUEUE_TABLE =
{
	4,
	{
		{ dump_RDD_BBH_QUEUE_DESCRIPTOR, 0x3400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DS_TM_CODEL_QUEUE_TABLE =
{
	4,
	{
		{ dump_RDD_CODEL_QUEUE_DESCRIPTOR, 0x3600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DS_TM_BBH_QUEUE_TABLE =
{
	4,
	{
		{ dump_RDD_BBH_QUEUE_DESCRIPTOR, 0x3800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT SCT_FILTER =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3ffc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_1 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_1 =
{
	8,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, 0x280 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_1 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, 0x300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT TUNNEL_TABLE_1 =
{
	16,
	{
		{ dump_RDD_TUNNEL_ENTRY, 0x3c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_RX_SCRATCHPAD_1 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_SRAM_PD_FIFO_1 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_1 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, 0x700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT VPORT_TO_DSCP_TO_PBITS_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x790 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT FLOODING_HEADER_FPM_POOL_NUMBER_MAPPING_TABLE_1 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0x7a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DSCP_TO_PBITS_MAP_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x7c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_STREAM_TABLE_1 =
{
	384,
	{
		{ dump_RDD_TCPSPDTEST_STREAM, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT UDPSPDT_STREAM_RX_STAT_TABLE_1 =
{
	48,
	{
		{ dump_RDD_UDPSPDT_STREAM_RX_STAT, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT GLOBAL_DSCP_TO_PBITS_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xec0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_RING_INTERRUPT_COUNTER_TABLE_1 =
{
	8,
	{
		{ dump_RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY, 0xf00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_1 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0xf90 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xfa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT TUNNEL_DST_IP_LKP_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0xfc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DS_PACKET_BUFFER_1 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT TCAM_IC_CMD_TABLE_1 =
{
	4,
	{
		{ dump_RDD_TCAM_IC_CMD, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DS_CPU_REASON_TO_METER_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2240 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_CACHE_TABLE_1 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x2280 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT TC_TO_QUEUE_TABLE_1 =
{
	8,
	{
		{ dump_RDD_TC_TO_QUEUE_8, 0x2300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT MCAST_FPM_TOKENS_READ_FLAG_1 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x2388 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2390 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT VPORT_TO_FLOW_IDX_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x23a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT US_CPU_REASON_TO_METER_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x23c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT PKTGEN_TX_STREAM_TABLE_1 =
{
	136,
	{
		{ dump_RDD_PKTGEN_TX_STREAM_ENTRY, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_RSV_TABLE_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2620 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT IPV6_HOST_ADDRESS_CRC_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2640 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DS_CPU_RX_METER_TABLE_1 =
{
	8,
	{
		{ dump_RDD_CPU_RX_METER_ENTRY, 0x2680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT PBIT_TO_QUEUE_TABLE_1 =
{
	8,
	{
		{ dump_RDD_PBIT_TO_QUEUE_8, 0x2700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_1 =
{
	8,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x2788 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_INTERRUPT_COALESCING_TABLE_1 =
{
	16,
	{
		{ dump_RDD_CPU_INTERRUPT_COALESCING_ENTRY, 0x2790 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT IPV4_HOST_ADDRESS_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x27a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT SPDSVC_GEN_PARAMS_TABLE_1 =
{
	60,
	{
		{ dump_RDD_SPDSVC_GEN_PARAMS, 0x27c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x27fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_1 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT UDPSPDT_STREAM_TX_STAT_TABLE_1 =
{
	40,
	{
		{ dump_RDD_UDPSPDT_STREAM_TX_STAT, 0x2a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CODEL_ENABLE_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2aa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT NULL_BUFFER_1 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x2ab8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_REASON_AND_VPORT_TO_METER_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2ac0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_RX_COPY_INT_SCRATCHPAD_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2af0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_RX_COPY_PD_FIFO_TABLE_1 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x2b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DBG_DUMP_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2b80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT US_CPU_RX_METER_TABLE_1 =
{
	8,
	{
		{ dump_RDD_CPU_RX_METER_ENTRY, 0x2c80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_1 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, 0x2d00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT LAYER2_HEADER_COPY_MAPPING_TABLE_1 =
{
	4,
	{
		{ dump_RDD_LAYER2_HEADER_COPY_MAPPING_ENTRY, 0x2d80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT TX_ABS_RECYCLE_COUNTERS_1 =
{
	8,
	{
		{ dump_RDD_TX_ABS_RECYCLE_COUNTERS_ENTRY, 0x2df8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CSO_CONTEXT_TABLE_1 =
{
	108,
	{
		{ dump_RDD_CSO_CONTEXT_ENTRY, 0x2e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT PKTGEN_NO_SBPM_HDRS_CNTR_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2e6c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_RX_PSRAM_GET_NEXT_SCRATCHPAD_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2e70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT MAX_PKT_LEN_TABLE_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2e80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT VLAN_ACTION_GPE_HANDLER_PTR_TABLE_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2ec0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CODEL_NUM_QUEUES_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2ee2 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT ONE_VALUE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2ee4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_1 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0x2ee8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_1 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, 0x2ef0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT UDPSPDT_TX_PARAMS_TABLE_1 =
{
	24,
	{
		{ dump_RDD_UDPSPDT_TX_PARAMS, 0x2f00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_ENGINE_GLOBAL_TABLE_1 =
{
	20,
	{
		{ dump_RDD_TCPSPDTEST_ENGINE_GLOBAL_INFO, 0x2f60 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_RX_INTERRUPT_SCRATCH_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2f74 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_RX_INTERRUPT_ID_DDR_ADDR_1 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0x2f78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT COMMON_REPROCESSING_PD_FIFO_TABLE_1 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x2f80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT UPDATE_FIFO_TABLE_1 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x2fc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT VPORT_PBIT_TO_DISCARD_PRIO_VECTOR_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2fe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT SPDSVC_ANALYZER_PARAMS_TABLE_1 =
{
	16,
	{
		{ dump_RDD_SPDSVC_ANALYZER_PARAMS, 0x2ff0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_RING_DESCRIPTORS_TABLE_1 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x3000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT EMAC_FLOW_CTRL_1 =
{
	16,
	{
		{ dump_RDD_EMAC_FLOW_CTRL_ENTRY, 0x3100 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3150 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_1 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, 0x3180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_RX_COPY_UPDATE_FIFO_TABLE_1 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x31c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT BRIDGE_CFG_TABLE_1 =
{
	8,
	{
		{ dump_RDD_BRIDGE_CFG, 0x31e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x31f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT NATC_TBL_CFG_1 =
{
	24,
	{
		{ dump_RDD_NATC_TBL_CONFIGURATION, 0x3200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT NATC_COUNTERS_GENERIC_CONTEXT_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3248 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x32c8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_1 =
{
	5,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x3348 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x334d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT PKTGEN_CURR_SBPM_HDR_PTR_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x334e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_RX_COPY_DISPATCHER_CREDIT_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3350 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT PKTGEN_NUM_OF_AVAIL_SBPM_HDRS_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x335c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT PKTGEN_SBPM_END_PTR_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x335e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT TCAM_IC_CFG_TABLE_1 =
{
	16,
	{
		{ dump_RDD_TCAM_IC_CFG, 0x3360 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT SPDSVC_GEN_DISPATCHER_CREDIT_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x33e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT PKTGEN_BAD_GET_NEXT_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x33ec },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT COMMON_REPROCESSING_DISPATCHER_CREDIT_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x33f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT PKTGEN_MAX_UT_PKTS_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x33fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT PKTGEN_SBPM_HDR_BNS_1 =
{
	2,
	{
		{ dump_RDD_PKTGEN_SBPM_HDR_BN, 0x3400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT INGRESS_PACKET_BASED_MAPPING_1 =
{
	4,
	{
		{ dump_RDD_INGRESS_PACKET_BASED_MAPPING_ENTRY, 0x3438 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT PKTGEN_UT_TRIGGER_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x343c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT COMMON_REPROCESSING_UPDATE_FIFO_TABLE_1 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x3440 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT IPTV_CONFIGURATION_TABLE_1 =
{
	12,
	{
		{ dump_RDD_HW_IPTV_CONFIGURATION, 0x3460 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT TASK_IDX_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x346c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3470 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x347c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x347d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT SPDSVC_TCPSPDTEST_COMMON_TABLE_1 =
{
	1,
	{
		{ dump_RDD_SPDSVC_TCPSPDTEST_COMMON_ENTRY, 0x347e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_UDP_RX_FLOWS_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x347f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_VPORT_TO_METER_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3480 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE_1 =
{
	8,
	{
		{ dump_RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY, 0x34a8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_1 =
{
	16,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, 0x34b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT PD_FIFO_TABLE_1 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x34c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT BCM_SPDSVC_STREAM_RX_TS_TABLE_1 =
{
	12,
	{
		{ dump_RDD_SPDSVC_RX_TS_STAT, 0x34e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_INTERRUPT_THRESHOLD_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x34ec },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_INTERRUPT_COUNTER_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x34ee },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_DESCRIPTOR_TABLE_1 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x34f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DEFAULT_VLAN_VID_TABLE_1 =
{
	2,
	{
		{ dump_RDD_DEFAULT_VLAN_VID_ENTRY, 0x3500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_REASON_TO_TC_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3520 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT TC_TO_CPU_RXQ_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3560 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT EXC_TC_TO_CPU_RXQ_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x35a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE_1 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0x35e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_INTERRUPT_COALESCING_TABLE_1 =
{
	16,
	{
		{ dump_RDD_CPU_INTERRUPT_COALESCING_ENTRY, 0x35f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT PKTGEN_SESSION_DATA_1 =
{
	32,
	{
		{ dump_RDD_PKTGEN_TX_PARAMS, 0x3600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_DESCRIPTOR_TABLE_1 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x3620 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_INTERRUPT_SCRATCH_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3630 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_INDEX_DDR_ADDR_TABLE_1 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0x3638 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT VPORT_TO_CPU_OBJ_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3640 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_RX_LOCAL_SCRATCH_1 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x3668 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_1 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, 0x3678 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT PKTGEN_FPM_UG_MGMT_1 =
{
	20,
	{
		{ dump_RDD_PKTGEN_FPM_UG_MGMT_ENTRY, 0x3680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_1 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x3694 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT ECN_IPV6_REMARK_TABLE_1 =
{
	1,
	{
		{ dump_RDD_ECN_IPV6_REMARK_ENTRY, 0x3696 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT MULTI_FLOW_FLAG_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3697 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT PKTGEN_SBPM_EXTS_1 =
{
	4,
	{
		{ dump_RDD_PKTGEN_SBPM_EXT, 0x3698 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_1 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, 0x36a8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT PKTGEN_BBMSG_REPLY_SCRATCH_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x36b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_1 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x36b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x36c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_1 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, 0x36d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT TUNNEL_HEADER_PSRAM_BUFFER_1 =
{
	4,
	{
		{ dump_RDD_TUNNEL_HEADER_PSRAM_BUFFER, 0x36e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x36e4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT RATE_LIMIT_OVERHEAD_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x36e5 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT INGRESS_QOS_DONT_DROP_RATIO_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x36e6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT INGRESS_QOS_WAN_UNTAGGED_PRIORITY_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x36e7 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_1 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, 0x36e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_CACHE_OFFSET_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x36ef },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x36f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_REDIRECT_MODE_1 =
{
	1,
	{
		{ dump_RDD_CPU_REDIRECT_MODE_ENTRY, 0x36f4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CSO_DISABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x36f5 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x36f6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_1 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x36f7 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_SHADOW_RD_IDX_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x36f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x36fa },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x36fb },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT PKTGEN_TX_STREAM_SCRATCH_TABLE_1 =
{
	8,
	{
		{ dump_RDD_PKTGEN_TX_STREAM_SCRATCH_ENTRY, 0x3700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_SHADOW_WR_IDX_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3708 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_NEXT_PTR_TABLE_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3710 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3718 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_1 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, 0x3720 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_1 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, 0x3728 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3730 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT SCT_FILTER_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3ffc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_TX_SCRATCHPAD_2 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_2 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_2 =
{
	8,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, 0x680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT FW_POLICER_BUDGET_2 =
{
	2,
	{
		{ dump_RDD_FW_POLICER_BUDGET_ENTRY, 0x700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x7a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT TUNNEL_TABLE_2 =
{
	16,
	{
		{ dump_RDD_TUNNEL_ENTRY, 0x7c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT TCAM_IC_CMD_TABLE_2 =
{
	4,
	{
		{ dump_RDD_TCAM_IC_CMD, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DSCP_TO_PBITS_MAP_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xa40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_TX_DBG_CNTRS_TABLE_2 =
{
	64,
	{
		{ dump_RDD_CPU_TX_DBG_CNTRS, 0xa80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_2 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, 0xb00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT FLOODING_HEADER_FPM_POOL_NUMBER_MAPPING_TABLE_2 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0xba0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT GLOBAL_DSCP_TO_PBITS_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xbc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_2 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT FW_POLICER_CBS_2 =
{
	4,
	{
		{ dump_RDD_CBS_ENTRY, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT TUNNEL_DST_IP_LKP_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0xf40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT LAYER2_HEADER_COPY_MAPPING_TABLE_2 =
{
	4,
	{
		{ dump_RDD_LAYER2_HEADER_COPY_MAPPING_ENTRY, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT MCAST_FPM_TOKENS_READ_FLAG_2 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0xff8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DS_PACKET_BUFFER_2 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_SRAM_PD_FIFO_2 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_2 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, 0x2100 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT VPORT_TO_DSCP_TO_PBITS_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2190 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x21a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DHD_CODEL_BIAS_SLOPE_TABLE_2 =
{
	4,
	{
		{ dump_RDD_DHD_CODEL_BIAS_SLOPE, 0x21c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x21ec },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_2 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0x21f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_PD_FIFO_TABLE_2 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT QUEUE_THRESHOLD_VECTOR_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x22c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT ONE_VALUE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x22e4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_2 =
{
	8,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x22e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DHD_FPM_POOL_NUMBER_MAPPING_TABLE_2 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0x22f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT TC_TO_QUEUE_TABLE_2 =
{
	8,
	{
		{ dump_RDD_TC_TO_QUEUE_8, 0x2300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT FW_POLICER_BUDGET_REMAINDER_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2388 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT TIMER_COMMON_CTR_REP_2 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x23d8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CODEL_ENABLE_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x23e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT NULL_BUFFER_2 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x23f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DHD_FLOW_RING_CACHE_CTX_TABLE_2 =
{
	16,
	{
		{ dump_RDD_DHD_FLOW_RING_CACHE_CTX_ENTRY, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT PBIT_TO_QUEUE_TABLE_2 =
{
	8,
	{
		{ dump_RDD_PBIT_TO_QUEUE_8, 0x2500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2588 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_2 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0x25b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT VLAN_ACTION_GPE_HANDLER_PTR_TABLE_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x25c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CODEL_NUM_QUEUES_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x25e2 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT TIMER_COMMON_TIMER_VALUE_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x25e4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT SPDSVC_WLAN_TXPOST_PARAMS_TABLE_2 =
{
	2,
	{
		{ dump_RDD_SPDSVC_WLAN_TXPOST_PARAMS, 0x25e6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_RX_INTERRUPT_ID_DDR_ADDR_2 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0x25e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT BUFFER_ALLOC_REPLY_2 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x25f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DHD_POST_COMMON_RADIO_DATA_2 =
{
	152,
	{
		{ dump_RDD_DHD_POST_COMMON_RADIO_ENTRY, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DBG_DUMP_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2698 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT MIRRORING_SCRATCH_2 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x2798 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT VPORT_PBIT_TO_DISCARD_PRIO_VECTOR_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x27a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x27b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_UPDATE_FIFO_TABLE_2 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x27c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_INTERRUPT_COALESCING_TABLE_2 =
{
	16,
	{
		{ dump_RDD_CPU_INTERRUPT_COALESCING_ENTRY, 0x27e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_2 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, 0x27f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DHD_AUX_INFO_CACHE_TABLE_2 =
{
	16,
	{
		{ dump_RDD_DHD_AUX_INFO_CACHE_ENTRY, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_2 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, 0x2900 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT MAX_PKT_LEN_TABLE_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DHD_DOORBELL_TX_POST_VALUE_2 =
{
	4,
	{
		{ dump_RDD_DHD_DOORBELL, 0x29c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DHD_COMPLETE_COMMON_RADIO_DATA_2 =
{
	120,
	{
		{ dump_RDD_DHD_COMPLETE_COMMON_RADIO_ENTRY, 0x2a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_2 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0x2a78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_2 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, 0x2a80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT EMAC_FLOW_CTRL_BUDGET_2 =
{
	4,
	{
		{ dump_RDD_EMAC_FLOW_CTRL_BUDGET_ENTRY, 0x2ac0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT TASK_IDX_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2ad4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DHD_BACKUP_BASE_ADDR_2 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x2ad8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_TX_RING_DESCRIPTOR_TABLE_2 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x2ae0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT EMAC_FLOW_CTRL_2 =
{
	16,
	{
		{ dump_RDD_EMAC_FLOW_CTRL_ENTRY, 0x2b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT NATC_COUNTERS_GENERIC_CONTEXT_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2b50 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_2 =
{
	16,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, 0x2bd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT SPDSVC_ANALYZER_PARAMS_TABLE_2 =
{
	16,
	{
		{ dump_RDD_SPDSVC_ANALYZER_PARAMS, 0x2be0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT BRIDGE_CFG_TABLE_2 =
{
	8,
	{
		{ dump_RDD_BRIDGE_CFG, 0x2bf0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT NATC_TBL_CFG_2 =
{
	24,
	{
		{ dump_RDD_NATC_TBL_CONFIGURATION, 0x2c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2c48 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_2 =
{
	5,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x2cc8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT EMAC_FLOW_CTRL_VECTOR_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2ccd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2cce },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2ccf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2cd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT TCAM_IC_CFG_TABLE_2 =
{
	16,
	{
		{ dump_RDD_TCAM_IC_CFG, 0x2ce0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2d60 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_TX_VLAN_STATS_ENABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2d6c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2d6d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_UDP_RX_FLOWS_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2d6e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT ECN_IPV6_REMARK_TABLE_2 =
{
	1,
	{
		{ dump_RDD_ECN_IPV6_REMARK_ENTRY, 0x2d6f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_TX_INGRESS_DISPATCHER_CREDIT_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2d70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DHD_RX_POST_RING_SIZE_2 =
{
	2,
	{
		{ dump_RDD_DHD_RING_SIZE, 0x2d7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_RING_SIZE_2 =
{
	2,
	{
		{ dump_RDD_DHD_RING_SIZE, 0x2d7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DEFAULT_VLAN_VID_TABLE_2 =
{
	2,
	{
		{ dump_RDD_DEFAULT_VLAN_VID_ENTRY, 0x2d80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT IPTV_CONFIGURATION_TABLE_2 =
{
	12,
	{
		{ dump_RDD_HW_IPTV_CONFIGURATION, 0x2da0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_RING_SIZE_2 =
{
	2,
	{
		{ dump_RDD_DHD_RING_SIZE, 0x2dac },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_2 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x2dae },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT FW_POLICER_VECTOR_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2db0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT MULTI_FLOW_FLAG_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2dbc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2dbd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT RATE_LIMIT_OVERHEAD_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2dbe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT INGRESS_QOS_DONT_DROP_RATIO_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2dbf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2dc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2dd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT INGRESS_QOS_WAN_UNTAGGED_PRIORITY_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2ddc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_REDIRECT_MODE_2 =
{
	1,
	{
		{ dump_RDD_CPU_REDIRECT_MODE_ENTRY, 0x2ddd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2dde },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_2 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x2ddf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_DISPATCHER_CREDIT_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2de0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2dec },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2ded },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_DISPATCHER_CREDIT_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2df0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_FLOW_RING_BUFFER_2 =
{
	48,
	{
		{ dump_RDD_DHD_TX_POST_DESCRIPTOR, 0x2e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DHD_MIRRORING_DISPATCHER_CREDIT_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2e30 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_FLOW_RING_BUFFER_2 =
{
	16,
	{
		{ dump_RDD_DHD_TX_COMPLETE_DESCRIPTOR, 0x2e40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT SPDSVC_WLAN_GEN_PARAMS_TABLE_2 =
{
	10,
	{
		{ dump_RDD_SPDSVC_WLAN_GEN_PARAMS, 0x2e50 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_2 =
{
	16,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, 0x2e60 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_DESCRIPTOR_TABLE_2 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x2e70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT RING_CPU_TX_DESCRIPTOR_DATA_TABLE_2 =
{
	16,
	{
		{ dump_RDD_RING_CPU_TX_DESCRIPTOR, 0x2e80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE_2 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0x2ea0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_DESCRIPTOR_TABLE_2 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x2eb0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_INTERRUPT_SCRATCH_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2ec0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT INGRESS_PACKET_BASED_MAPPING_2 =
{
	4,
	{
		{ dump_RDD_INGRESS_PACKET_BASED_MAPPING_ENTRY, 0x2ec8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_2 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, 0x2ed0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT TUNNEL_HEADER_PSRAM_BUFFER_2 =
{
	4,
	{
		{ dump_RDD_TUNNEL_HEADER_PSRAM_BUFFER, 0x2ee0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE_2 =
{
	8,
	{
		{ dump_RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY, 0x2ee8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_TX_RING_INDICES_VALUES_TABLE_2 =
{
	4,
	{
		{ dump_RDD_CPU_TX_RING_INDICES, 0x2ef0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_2 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, 0x2ef8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DHD_FLOW_RING_CACHE_LKP_TABLE_2 =
{
	2,
	{
		{ dump_RDD_DHD_FLOW_RING_CACHE_LKP_ENTRY, 0x2f00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_2 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, 0x2f20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_2 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, 0x2f28 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DHD_DOORBELL_TX_COMPLETE_VALUE_2 =
{
	4,
	{
		{ dump_RDD_DHD_DOORBELL, 0x2f30 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_2 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x2f38 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DHD_FPM_REPLY_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2f40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_2 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, 0x2f48 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DHD_DOORBELL_RX_COMPLETE_VALUE_2 =
{
	4,
	{
		{ dump_RDD_DHD_DOORBELL, 0x2f50 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT EMAC_FLOW_CTRL_BUDGET_REMAINDER_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2f58 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DHD_DOORBELL_RX_POST_VALUE_2 =
{
	4,
	{
		{ dump_RDD_DHD_DOORBELL, 0x2f60 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2f68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DHD_CPU_INT_ID_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2f70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_SHADOW_RD_IDX_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2f78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DHD_BACKUP_INDEX_CACHE_2 =
{
	32,
	{
		{ dump_RDD_DHD_BACKUP_IDX_CACHE_TABLE, 0x2f80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_SHADOW_WR_IDX_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2fa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2fa8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_NEXT_PTR_TABLE_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2fb0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_2 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, 0x2fb8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2fc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_FLOW_RING_BUFFER_2 =
{
	32,
	{
		{ dump_RDD_DHD_RX_COMPLETE_DESCRIPTOR, 0x3000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DHD_RX_POST_FLOW_RING_BUFFER_2 =
{
	32,
	{
		{ dump_RDD_DHD_RX_POST_DESCRIPTOR, 0x3080 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DHD_L2_HEADER_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3100 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT CPU_TX_SYNC_FIFO_TABLE_2 =
{
	8,
	{
		{ dump_RDD_CPU_TX_SYNC_FIFO_ENTRY, 0x3180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE_2 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x3200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6878
static DUMP_RUNNERREG_STRUCT SCT_FILTER_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3ffc },
		{ 0, 0 },
	}
};
#endif

TABLE_STRUCT RUNNER_TABLES[NUMBER_OF_TABLES] =
{
#if defined BCM6878
	{ "US_TM_PD_FIFO_TABLE", 1, CORE_0_INDEX, &US_TM_PD_FIFO_TABLE, 130, 1, 1 },
#endif
#if defined BCM6878
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_0_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT, 8, 1, 1 },
#endif
#if defined BCM6878
	{ "CODEL_SQ_TABLE", 1, CORE_0_INDEX, &CODEL_SQ_TABLE, 32, 1, 1 },
#endif
#if defined BCM6878
	{ "US_TM_TM_FLOW_CNTR_TABLE", 1, CORE_0_INDEX, &US_TM_TM_FLOW_CNTR_TABLE, 128, 1, 1 },
#endif
#if defined BCM6878
	{ "REPORTING_QUEUE_ACCUMULATED_TABLE", 1, CORE_0_INDEX, &REPORTING_QUEUE_ACCUMULATED_TABLE, 16, 1, 1 },
#endif
#if defined BCM6878
	{ "SERVICE_QUEUES_RATE_LIMITER_TABLE", 1, CORE_0_INDEX, &SERVICE_QUEUES_RATE_LIMITER_TABLE, 18, 1, 1 },
#endif
#if defined BCM6878
	{ "DIRECT_FLOW_CNTR_TABLE", 1, CORE_0_INDEX, &DIRECT_FLOW_CNTR_TABLE, 32, 1, 1 },
#endif
#if defined BCM6878
	{ "SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE", 1, CORE_0_INDEX, &SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE, 8, 1, 1 },
#endif
#if defined BCM6878
	{ "BASIC_SCHEDULER_TABLE_DS", 1, CORE_0_INDEX, &BASIC_SCHEDULER_TABLE_DS, 5, 1, 1 },
#endif
#if defined BCM6878
	{ "DIRECT_FLOW_PD_TABLE", 1, CORE_0_INDEX, &DIRECT_FLOW_PD_TABLE, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "BUFFER_CONG_DQM_NOT_EMPTY", 1, CORE_0_INDEX, &BUFFER_CONG_DQM_NOT_EMPTY, 8, 1, 1 },
#endif
#if defined BCM6878
	{ "BASIC_SCHEDULER_TABLE_US", 1, CORE_0_INDEX, &BASIC_SCHEDULER_TABLE_US, 33, 1, 1 },
#endif
#if defined BCM6878
	{ "US_TM_CPU_TX_ABS_COUNTERS", 1, CORE_0_INDEX, &US_TM_CPU_TX_ABS_COUNTERS, 32, 1, 1 },
#endif
#if defined BCM6878
	{ "US_TM_FLUSH_CFG_CPU_TABLE", 1, CORE_0_INDEX, &US_TM_FLUSH_CFG_CPU_TABLE, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "CODEL_ENABLE_TABLE", 1, CORE_0_INDEX, &CODEL_ENABLE_TABLE, 6, 1, 1 },
#endif
#if defined BCM6878
	{ "US_TM_WAN_0_BBH_TX_WAKE_UP_DATA_TABLE", 1, CORE_0_INDEX, &US_TM_WAN_0_BBH_TX_WAKE_UP_DATA_TABLE, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "SERVICE_QUEUES_COMPLEX_SCHEDULER_TABLE", 1, CORE_0_INDEX, &SERVICE_QUEUES_COMPLEX_SCHEDULER_TABLE, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "COMPLEX_SCHEDULER_TABLE", 1, CORE_0_INDEX, &COMPLEX_SCHEDULER_TABLE, 4, 1, 1 },
#endif
#if defined BCM6878
	{ "DS_TM_PD_FIFO_TABLE", 1, CORE_0_INDEX, &DS_TM_PD_FIFO_TABLE, 80, 1, 1 },
#endif
#if defined BCM6878
	{ "REPORTING_COUNTER_TABLE", 1, CORE_0_INDEX, &REPORTING_COUNTER_TABLE, 40, 1, 1 },
#endif
#if defined BCM6878
	{ "DIRECT_FLOW_EPON_CONTROL_SCRATCH", 1, CORE_0_INDEX, &DIRECT_FLOW_EPON_CONTROL_SCRATCH, 22, 1, 1 },
#endif
#if defined BCM6878
	{ "CODEL_NUM_QUEUES", 1, CORE_0_INDEX, &CODEL_NUM_QUEUES, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "DS_TM_BBH_TX_WAKE_UP_DATA_TABLE", 1, CORE_0_INDEX, &DS_TM_BBH_TX_WAKE_UP_DATA_TABLE, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "US_BUFFER_CONG_MGT_CFG", 1, CORE_0_INDEX, &US_BUFFER_CONG_MGT_CFG, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "US_TM_CODEL_QUEUE_TABLE", 1, CORE_0_INDEX, &US_TM_CODEL_QUEUE_TABLE, 65, 1, 1 },
#endif
#if defined BCM6878
	{ "MUTEX_TABLE", 1, CORE_0_INDEX, &MUTEX_TABLE, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "DS_TM_CPU_TX_ABS_COUNTERS", 1, CORE_0_INDEX, &DS_TM_CPU_TX_ABS_COUNTERS, 32, 1, 1 },
#endif
#if defined BCM6878
	{ "DEBUG_SCRATCHPAD", 1, CORE_0_INDEX, &DEBUG_SCRATCHPAD, 12, 1, 1 },
#endif
#if defined BCM6878
	{ "BBH_TX_EPON_WAKE_UP_DATA_TABLE", 1, CORE_0_INDEX, &BBH_TX_EPON_WAKE_UP_DATA_TABLE, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "DS_BUFFER_CONG_MGT_CFG", 1, CORE_0_INDEX, &DS_BUFFER_CONG_MGT_CFG, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "BASIC_RATE_LIMITER_TABLE_DS", 1, CORE_0_INDEX, &BASIC_RATE_LIMITER_TABLE_DS, 40, 1, 1 },
#endif
#if defined BCM6878
	{ "REGISTERS_BUFFER", 1, CORE_0_INDEX, &REGISTERS_BUFFER, 32, 1, 1 },
#endif
#if defined BCM6878
	{ "DIRECT_FLOW_RX_MIRRORING_SCRATCHPAD", 1, CORE_0_INDEX, &DIRECT_FLOW_RX_MIRRORING_SCRATCHPAD, 136, 1, 1 },
#endif
#if defined BCM6878
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_0_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "DS_TM_FLUSH_CFG_CPU_TABLE", 1, CORE_0_INDEX, &DS_TM_FLUSH_CFG_CPU_TABLE, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "US_TM_SCHEDULING_QUEUE_AGING_VECTOR", 1, CORE_0_INDEX, &US_TM_SCHEDULING_QUEUE_AGING_VECTOR, 5, 1, 1 },
#endif
#if defined BCM6878
	{ "GHOST_REPORTING_GLOBAL_CFG", 1, CORE_0_INDEX, &GHOST_REPORTING_GLOBAL_CFG, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "NULL_BUFFER", 1, CORE_0_INDEX, &NULL_BUFFER, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "BUFFER_CONG_SCRATCHPAD", 1, CORE_0_INDEX, &BUFFER_CONG_SCRATCHPAD, 8, 1, 1 },
#endif
#if defined BCM6878
	{ "US_TM_SCHEDULING_QUEUE_TABLE", 1, CORE_0_INDEX, &US_TM_SCHEDULING_QUEUE_TABLE, 65, 1, 1 },
#endif
#if defined BCM6878
	{ "MIRRORING_SCRATCH", 1, CORE_0_INDEX, &MIRRORING_SCRATCH, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "US_TM_FLUSH_CFG_FW_TABLE", 1, CORE_0_INDEX, &US_TM_FLUSH_CFG_FW_TABLE, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "DS_TM_SCHEDULING_QUEUE_AGING_VECTOR", 1, CORE_0_INDEX, &DS_TM_SCHEDULING_QUEUE_AGING_VECTOR, 5, 1, 1 },
#endif
#if defined BCM6878
	{ "ONE_VALUE", 1, CORE_0_INDEX, &ONE_VALUE, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "REPORT_BBH_TX_QUEUE_ID_TABLE", 1, CORE_0_INDEX, &REPORT_BBH_TX_QUEUE_ID_TABLE, 2, 1, 1 },
#endif
#if defined BCM6878
	{ "CODEL_BIAS_SLOPE_TABLE", 1, CORE_0_INDEX, &CODEL_BIAS_SLOPE_TABLE, 11, 1, 1 },
#endif
#if defined BCM6878
	{ "DIRECT_FLOW_PAUSE_QUANTA", 1, CORE_0_INDEX, &DIRECT_FLOW_PAUSE_QUANTA, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "BUDGET_ALLOCATION_TIMER_VALUE", 1, CORE_0_INDEX, &BUDGET_ALLOCATION_TIMER_VALUE, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "DS_TM_FLUSH_CFG_FW_TABLE", 1, CORE_0_INDEX, &DS_TM_FLUSH_CFG_FW_TABLE, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "US_TM_WAN_0_BBH_TX_EGRESS_COUNTER_TABLE", 1, CORE_0_INDEX, &US_TM_WAN_0_BBH_TX_EGRESS_COUNTER_TABLE, 40, 1, 1 },
#endif
#if defined BCM6878
	{ "DS_TM_TM_FLOW_CNTR_TABLE", 1, CORE_0_INDEX, &DS_TM_TM_FLOW_CNTR_TABLE, 6, 1, 1 },
#endif
#if defined BCM6878
	{ "US_TM_WAN_0_BB_DESTINATION_TABLE", 1, CORE_0_INDEX, &US_TM_WAN_0_BB_DESTINATION_TABLE, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "SERVICE_QUEUES_FLUSH_CFG_FW_TABLE", 1, CORE_0_INDEX, &SERVICE_QUEUES_FLUSH_CFG_FW_TABLE, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "BBH_TX_EPON_INGRESS_COUNTER_TABLE", 1, CORE_0_INDEX, &BBH_TX_EPON_INGRESS_COUNTER_TABLE, 40, 1, 1 },
#endif
#if defined BCM6878
	{ "BBH_TX_EGRESS_REPORT_COUNTER_TABLE", 1, CORE_0_INDEX, &BBH_TX_EGRESS_REPORT_COUNTER_TABLE, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "US_TM_FLUSH_CFG_CURRENT_TABLE", 1, CORE_0_INDEX, &US_TM_FLUSH_CFG_CURRENT_TABLE, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "REPORTING_QUEUE_COUNTER_TABLE", 1, CORE_0_INDEX, &REPORTING_QUEUE_COUNTER_TABLE, 65, 1, 1 },
#endif
#if defined BCM6878
	{ "DS_TM_BB_DESTINATION_TABLE", 1, CORE_0_INDEX, &DS_TM_BB_DESTINATION_TABLE, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "US_TM_BBH_TX_WAN_0_FIFO_BYTES_USED", 1, CORE_0_INDEX, &US_TM_BBH_TX_WAN_0_FIFO_BYTES_USED, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "US_TM_FLUSH_CFG_ENABLE_TABLE", 1, CORE_0_INDEX, &US_TM_FLUSH_CFG_ENABLE_TABLE, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "SERVICE_QUEUES_BUDGET_ALLOCATION_TIMER_VALUE", 1, CORE_0_INDEX, &SERVICE_QUEUES_BUDGET_ALLOCATION_TIMER_VALUE, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "TASK_IDX", 1, CORE_0_INDEX, &TASK_IDX, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "DS_TM_FLUSH_CFG_CURRENT_TABLE", 1, CORE_0_INDEX, &DS_TM_FLUSH_CFG_CURRENT_TABLE, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "SERVICE_QUEUES_SCHEDULING_QUEUE_AGING_VECTOR", 1, CORE_0_INDEX, &SERVICE_QUEUES_SCHEDULING_QUEUE_AGING_VECTOR, 5, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_FEED_RING_INTERRUPT_COUNTER_MAX", 1, CORE_0_INDEX, &CPU_FEED_RING_INTERRUPT_COUNTER_MAX, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "DIRECT_FLOW_WAN_VIQ_EXCLUSIVE", 1, CORE_0_INDEX, &DIRECT_FLOW_WAN_VIQ_EXCLUSIVE, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "US_TM_TX_PAUSE_NACK", 1, CORE_0_INDEX, &US_TM_TX_PAUSE_NACK, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "DS_TM_FLUSH_CFG_ENABLE_TABLE", 1, CORE_0_INDEX, &DS_TM_FLUSH_CFG_ENABLE_TABLE, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "BBH_TX_US_WAN_0_FIFO_BYTES_THRESHOLD", 1, CORE_0_INDEX, &BBH_TX_US_WAN_0_FIFO_BYTES_THRESHOLD, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "SCHEDULING_FLUSH_GLOBAL_CFG", 1, CORE_0_INDEX, &SCHEDULING_FLUSH_GLOBAL_CFG, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "TM_VLAN_STATS_ENABLE", 1, CORE_0_INDEX, &TM_VLAN_STATS_ENABLE, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "TX_EXCEPTION", 1, CORE_0_INDEX, &TX_EXCEPTION, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "CORE_ID_TABLE", 1, CORE_0_INDEX, &CORE_ID_TABLE, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "SERVICE_QUEUES_TM_ACTION_PTR_TABLE", 1, CORE_0_INDEX, &SERVICE_QUEUES_TM_ACTION_PTR_TABLE, 17, 1, 1 },
#endif
#if defined BCM6878
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_0_INDEX, &RX_MIRRORING_CONFIGURATION, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_0_INDEX, &TX_MIRRORING_CONFIGURATION, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "US_TM_FLUSH_AGGREGATION_TASK_DISABLE", 1, CORE_0_INDEX, &US_TM_FLUSH_AGGREGATION_TASK_DISABLE, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "DS_TM_FLUSH_AGGREGATION_TASK_DISABLE", 1, CORE_0_INDEX, &DS_TM_FLUSH_AGGREGATION_TASK_DISABLE, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "SERVICE_QUEUES_FLUSH_CFG_ENABLE_TABLE", 1, CORE_0_INDEX, &SERVICE_QUEUES_FLUSH_CFG_ENABLE_TABLE, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "SRAM_DUMMY_STORE", 1, CORE_0_INDEX, &SRAM_DUMMY_STORE, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "RATE_LIMIT_OVERHEAD", 1, CORE_0_INDEX, &RATE_LIMIT_OVERHEAD, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "MAC_TYPE", 1, CORE_0_INDEX, &MAC_TYPE, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "WAN_0_BBH_TX_FIFO_SIZE", 1, CORE_0_INDEX, &WAN_0_BBH_TX_FIFO_SIZE, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "US_TM_FIRST_QUEUE_MAPPING", 1, CORE_0_INDEX, &US_TM_FIRST_QUEUE_MAPPING, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "DS_TM_FIRST_QUEUE_MAPPING", 1, CORE_0_INDEX, &DS_TM_FIRST_QUEUE_MAPPING, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "SERVICE_QUEUES_FLUSH_CFG_CURRENT_TABLE", 1, CORE_0_INDEX, &SERVICE_QUEUES_FLUSH_CFG_CURRENT_TABLE, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "BASIC_RATE_LIMITER_TABLE_US", 1, CORE_0_INDEX, &BASIC_RATE_LIMITER_TABLE_US, 65, 1, 1 },
#endif
#if defined BCM6878
	{ "US_TM_TX_QUEUE_DROP_TABLE", 1, CORE_0_INDEX, &US_TM_TX_QUEUE_DROP_TABLE, 65, 1, 1 },
#endif
#if defined BCM6878
	{ "DBG_DUMP_TABLE", 1, CORE_0_INDEX, &DBG_DUMP_TABLE, 64, 1, 1 },
#endif
#if defined BCM6878
	{ "BACKUP_BBH_INGRESS_COUNTERS_TABLE", 1, CORE_0_INDEX, &BACKUP_BBH_INGRESS_COUNTERS_TABLE, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "BACKUP_BBH_EGRESS_COUNTERS_TABLE", 1, CORE_0_INDEX, &BACKUP_BBH_EGRESS_COUNTERS_TABLE, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_0_INDEX, &DEBUG_PRINT_CORE_LOCK, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "SERVICE_QUEUES_FIRST_QUEUE_MAPPING", 1, CORE_0_INDEX, &SERVICE_QUEUES_FIRST_QUEUE_MAPPING, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_0_INDEX, &RX_MIRRORING_DIRECT_ENABLE, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "BBH_TX_INGRESS_COUNTER_TABLE", 1, CORE_0_INDEX, &BBH_TX_INGRESS_COUNTER_TABLE, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "SCHEDULING_AGGREGATION_CONTEXT_VECTOR", 1, CORE_0_INDEX, &SCHEDULING_AGGREGATION_CONTEXT_VECTOR, 5, 1, 1 },
#endif
#if defined BCM6878
	{ "US_TM_UPDATE_FIFO_TABLE", 1, CORE_0_INDEX, &US_TM_UPDATE_FIFO_TABLE, 8, 1, 1 },
#endif
#if defined BCM6878
	{ "GHOST_REPORTING_QUEUE_STATUS_BIT_VECTOR_TABLE", 1, CORE_0_INDEX, &GHOST_REPORTING_QUEUE_STATUS_BIT_VECTOR_TABLE, 5, 1, 1 },
#endif
#if defined BCM6878
	{ "DS_TM_UPDATE_FIFO_TABLE", 1, CORE_0_INDEX, &DS_TM_UPDATE_FIFO_TABLE, 8, 1, 1 },
#endif
#if defined BCM6878
	{ "QUEUE_TO_REPORT_BIT_VECTOR", 1, CORE_0_INDEX, &QUEUE_TO_REPORT_BIT_VECTOR, 5, 1, 1 },
#endif
#if defined BCM6878
	{ "EPON_UPDATE_FIFO_TABLE", 1, CORE_0_INDEX, &EPON_UPDATE_FIFO_TABLE, 8, 1, 1 },
#endif
#if defined BCM6878
	{ "SERVICE_QUEUES_RATE_LIMITER_VALID_TABLE", 1, CORE_0_INDEX, &SERVICE_QUEUES_RATE_LIMITER_VALID_TABLE, 4, 1, 1 },
#endif
#if defined BCM6878
	{ "OVERALL_RATE_LIMITER_TABLE", 1, CORE_0_INDEX, &OVERALL_RATE_LIMITER_TABLE, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "REPORTING_QUEUE_DESCRIPTOR_TABLE", 1, CORE_0_INDEX, &REPORTING_QUEUE_DESCRIPTOR_TABLE, 65, 1, 1 },
#endif
#if defined BCM6878
	{ "US_TM_TX_OCTETS_COUNTERS_TABLE", 1, CORE_0_INDEX, &US_TM_TX_OCTETS_COUNTERS_TABLE, 8, 1, 1 },
#endif
#if defined BCM6878
	{ "SERVICE_QUEUES_CODEL_QUEUE_TABLE", 1, CORE_0_INDEX, &SERVICE_QUEUES_CODEL_QUEUE_TABLE, 8, 1, 1 },
#endif
#if defined BCM6878
	{ "FLUSH_DISPATCHER_CREDIT_TABLE", 1, CORE_0_INDEX, &FLUSH_DISPATCHER_CREDIT_TABLE, 3, 1, 1 },
#endif
#if defined BCM6878
	{ "SERVICE_QUEUES_UPDATE_FIFO_TABLE", 1, CORE_0_INDEX, &SERVICE_QUEUES_UPDATE_FIFO_TABLE, 8, 1, 1 },
#endif
#if defined BCM6878
	{ "SQ_TX_QUEUE_DROP_TABLE", 1, CORE_0_INDEX, &SQ_TX_QUEUE_DROP_TABLE, 8, 1, 1 },
#endif
#if defined BCM6878
	{ "RATE_LIMITER_VALID_TABLE_US", 1, CORE_0_INDEX, &RATE_LIMITER_VALID_TABLE_US, 4, 1, 1 },
#endif
#if defined BCM6878
	{ "US_TM_CODEL_DROP_DESCRIPTOR", 1, CORE_0_INDEX, &US_TM_CODEL_DROP_DESCRIPTOR, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "NATC_TBL_CFG", 1, CORE_0_INDEX, &NATC_TBL_CFG, 3, 1, 1 },
#endif
#if defined BCM6878
	{ "DS_TM_CODEL_DROP_DESCRIPTOR", 1, CORE_0_INDEX, &DS_TM_CODEL_DROP_DESCRIPTOR, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "RATE_LIMITER_VALID_TABLE_DS", 1, CORE_0_INDEX, &RATE_LIMITER_VALID_TABLE_DS, 4, 1, 1 },
#endif
#if defined BCM6878
	{ "DIRECT_FLOW_PAUSE_DEBUG", 1, CORE_0_INDEX, &DIRECT_FLOW_PAUSE_DEBUG, 3, 1, 1 },
#endif
#if defined BCM6878
	{ "DISPATCHER_CREDIT_TABLE", 1, CORE_0_INDEX, &DISPATCHER_CREDIT_TABLE, 3, 1, 1 },
#endif
#if defined BCM6878
	{ "SERVICE_QUEUES_DISPATCHER_CREDIT_TABLE", 1, CORE_0_INDEX, &SERVICE_QUEUES_DISPATCHER_CREDIT_TABLE, 3, 1, 1 },
#endif
#if defined BCM6878
	{ "SERVICE_QUEUES_CODEL_DROP_DESCRIPTOR", 1, CORE_0_INDEX, &SERVICE_QUEUES_CODEL_DROP_DESCRIPTOR, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "TX_MIRRORING_DISPATCHER_CREDIT_TABLE", 1, CORE_0_INDEX, &TX_MIRRORING_DISPATCHER_CREDIT_TABLE, 3, 1, 1 },
#endif
#if defined BCM6878
	{ "BBH_TX_EPON_EGRESS_COUNTER_TABLE", 1, CORE_0_INDEX, &BBH_TX_EPON_EGRESS_COUNTER_TABLE, 8, 1, 1 },
#endif
#if defined BCM6878
	{ "FPM_GLOBAL_CFG", 1, CORE_0_INDEX, &FPM_GLOBAL_CFG, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "DEBUG_PRINT_TABLE", 1, CORE_0_INDEX, &DEBUG_PRINT_TABLE, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "XGPON_REPORT_ZERO_SENT_TABLE", 1, CORE_0_INDEX, &XGPON_REPORT_ZERO_SENT_TABLE, 10, 1, 1 },
#endif
#if defined BCM6878
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_0_INDEX, &RUNNER_PROFILING_TRACE_BUFFER, 128, 1, 1 },
#endif
#if defined BCM6878
	{ "SERVICE_QUEUES_PD_FIFO_TABLE", 1, CORE_0_INDEX, &SERVICE_QUEUES_PD_FIFO_TABLE, 16, 1, 1 },
#endif
#if defined BCM6878
	{ "DS_TM_BBH_TX_EGRESS_COUNTER_TABLE", 1, CORE_0_INDEX, &DS_TM_BBH_TX_EGRESS_COUNTER_TABLE, 5, 1, 1 },
#endif
#if defined BCM6878
	{ "DS_TM_SCHEDULING_QUEUE_TABLE", 1, CORE_0_INDEX, &DS_TM_SCHEDULING_QUEUE_TABLE, 40, 1, 1 },
#endif
#if defined BCM6878
	{ "DS_TM_TX_QUEUE_DROP_TABLE", 1, CORE_0_INDEX, &DS_TM_TX_QUEUE_DROP_TABLE, 65, 1, 1 },
#endif
#if defined BCM6878
	{ "US_TM_BBH_QUEUE_TABLE", 1, CORE_0_INDEX, &US_TM_BBH_QUEUE_TABLE, 40, 1, 1 },
#endif
#if defined BCM6878
	{ "DS_TM_CODEL_QUEUE_TABLE", 1, CORE_0_INDEX, &DS_TM_CODEL_QUEUE_TABLE, 40, 1, 1 },
#endif
#if defined BCM6878
	{ "DS_TM_BBH_QUEUE_TABLE", 1, CORE_0_INDEX, &DS_TM_BBH_QUEUE_TABLE, 5, 1, 1 },
#endif
#if defined BCM6878
	{ "SCT_FILTER", 1, CORE_0_INDEX, &SCT_FILTER, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "RX_FLOW_TABLE", 1, CORE_1_INDEX, &RX_FLOW_TABLE_1, 320, 1, 1 },
#endif
#if defined BCM6878
	{ "VPORT_CFG_TABLE", 1, CORE_1_INDEX, &VPORT_CFG_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM6878
	{ "POLICER_PARAMS_TABLE", 1, CORE_1_INDEX, &POLICER_PARAMS_TABLE_1, 80, 1, 1 },
#endif
#if defined BCM6878
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_1_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_1, 8, 1, 1 },
#endif
#if defined BCM6878
	{ "TUNNEL_TABLE", 1, CORE_1_INDEX, &TUNNEL_TABLE_1, 4, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_RX_SCRATCHPAD", 1, CORE_1_INDEX, &CPU_RX_SCRATCHPAD_1, 64, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_RECYCLE_SRAM_PD_FIFO", 1, CORE_1_INDEX, &CPU_RECYCLE_SRAM_PD_FIFO_1, 16, 1, 1 },
#endif
#if defined BCM6878
	{ "TX_FLOW_TABLE", 1, CORE_1_INDEX, &TX_FLOW_TABLE_1, 144, 1, 1 },
#endif
#if defined BCM6878
	{ "VPORT_TO_DSCP_TO_PBITS_TABLE", 1, CORE_1_INDEX, &VPORT_TO_DSCP_TO_PBITS_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM6878
	{ "FLOODING_HEADER_FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_1_INDEX, &FLOODING_HEADER_FPM_POOL_NUMBER_MAPPING_TABLE_1, 32, 1, 1 },
#endif
#if defined BCM6878
	{ "DSCP_TO_PBITS_MAP_TABLE", 1, CORE_1_INDEX, &DSCP_TO_PBITS_MAP_TABLE_1, 1, 64, 1 },
#endif
#if defined BCM6878
	{ "TCPSPDTEST_STREAM_TABLE", 1, CORE_1_INDEX, &TCPSPDTEST_STREAM_TABLE_1, 4, 1, 1 },
#endif
#if defined BCM6878
	{ "UDPSPDT_STREAM_RX_STAT_TABLE", 1, CORE_1_INDEX, &UDPSPDT_STREAM_RX_STAT_TABLE_1, 4, 1, 1 },
#endif
#if defined BCM6878
	{ "GLOBAL_DSCP_TO_PBITS_TABLE", 1, CORE_1_INDEX, &GLOBAL_DSCP_TO_PBITS_TABLE_1, 1, 64, 1 },
#endif
#if defined BCM6878
	{ "CPU_RING_INTERRUPT_COUNTER_TABLE", 1, CORE_1_INDEX, &CPU_RING_INTERRUPT_COUNTER_TABLE_1, 18, 1, 1 },
#endif
#if defined BCM6878
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_1_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM6878
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_1_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_1, 32, 1, 1 },
#endif
#if defined BCM6878
	{ "TUNNEL_DST_IP_LKP_TABLE", 1, CORE_1_INDEX, &TUNNEL_DST_IP_LKP_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM6878
	{ "DS_PACKET_BUFFER", 1, CORE_1_INDEX, &DS_PACKET_BUFFER_1, 8, 1, 1 },
#endif
#if defined BCM6878
	{ "TCAM_IC_CMD_TABLE", 1, CORE_1_INDEX, &TCAM_IC_CMD_TABLE_1, 9, 16, 1 },
#endif
#if defined BCM6878
	{ "DS_CPU_REASON_TO_METER_TABLE", 1, CORE_1_INDEX, &DS_CPU_REASON_TO_METER_TABLE_1, 64, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_FEED_RING_CACHE_TABLE", 1, CORE_1_INDEX, &CPU_FEED_RING_CACHE_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM6878
	{ "TC_TO_QUEUE_TABLE", 1, CORE_1_INDEX, &TC_TO_QUEUE_TABLE_1, 17, 1, 1 },
#endif
#if defined BCM6878
	{ "MCAST_FPM_TOKENS_READ_FLAG", 1, CORE_1_INDEX, &MCAST_FPM_TOKENS_READ_FLAG_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_1_INDEX, &LOOPBACK_QUEUE_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM6878
	{ "VPORT_TO_FLOW_IDX", 1, CORE_1_INDEX, &VPORT_TO_FLOW_IDX_1, 32, 1, 1 },
#endif
#if defined BCM6878
	{ "US_CPU_REASON_TO_METER_TABLE", 1, CORE_1_INDEX, &US_CPU_REASON_TO_METER_TABLE_1, 64, 1, 1 },
#endif
#if defined BCM6878
	{ "PKTGEN_TX_STREAM_TABLE", 1, CORE_1_INDEX, &PKTGEN_TX_STREAM_TABLE_1, 4, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_FEED_RING_RSV_TABLE", 1, CORE_1_INDEX, &CPU_FEED_RING_RSV_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM6878
	{ "IPV6_HOST_ADDRESS_CRC_TABLE", 1, CORE_1_INDEX, &IPV6_HOST_ADDRESS_CRC_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM6878
	{ "DS_CPU_RX_METER_TABLE", 1, CORE_1_INDEX, &DS_CPU_RX_METER_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM6878
	{ "PBIT_TO_QUEUE_TABLE", 1, CORE_1_INDEX, &PBIT_TO_QUEUE_TABLE_1, 17, 1, 1 },
#endif
#if defined BCM6878
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_1_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_RECYCLE_INTERRUPT_COALESCING_TABLE", 1, CORE_1_INDEX, &CPU_RECYCLE_INTERRUPT_COALESCING_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "IPV4_HOST_ADDRESS_TABLE", 1, CORE_1_INDEX, &IPV4_HOST_ADDRESS_TABLE_1, 8, 1, 1 },
#endif
#if defined BCM6878
	{ "SPDSVC_GEN_PARAMS_TABLE", 1, CORE_1_INDEX, &SPDSVC_GEN_PARAMS_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "MUTEX_TABLE", 1, CORE_1_INDEX, &MUTEX_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_1_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_1, 128, 1, 1 },
#endif
#if defined BCM6878
	{ "UDPSPDT_STREAM_TX_STAT_TABLE", 1, CORE_1_INDEX, &UDPSPDT_STREAM_TX_STAT_TABLE_1, 4, 1, 1 },
#endif
#if defined BCM6878
	{ "CODEL_ENABLE_TABLE", 1, CORE_1_INDEX, &CODEL_ENABLE_TABLE_1, 6, 1, 1 },
#endif
#if defined BCM6878
	{ "NULL_BUFFER", 1, CORE_1_INDEX, &NULL_BUFFER_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_REASON_AND_VPORT_TO_METER_TABLE", 1, CORE_1_INDEX, &CPU_REASON_AND_VPORT_TO_METER_TABLE_1, 48, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_RX_COPY_INT_SCRATCHPAD", 1, CORE_1_INDEX, &CPU_RX_COPY_INT_SCRATCHPAD_1, 4, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_RX_COPY_PD_FIFO_TABLE", 1, CORE_1_INDEX, &CPU_RX_COPY_PD_FIFO_TABLE_1, 8, 1, 1 },
#endif
#if defined BCM6878
	{ "DBG_DUMP_TABLE", 1, CORE_1_INDEX, &DBG_DUMP_TABLE_1, 64, 1, 1 },
#endif
#if defined BCM6878
	{ "US_CPU_RX_METER_TABLE", 1, CORE_1_INDEX, &US_CPU_RX_METER_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM6878
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_1_INDEX, &INGRESS_FILTER_PROFILE_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM6878
	{ "LAYER2_HEADER_COPY_MAPPING_TABLE", 1, CORE_1_INDEX, &LAYER2_HEADER_COPY_MAPPING_TABLE_1, 30, 1, 1 },
#endif
#if defined BCM6878
	{ "TX_ABS_RECYCLE_COUNTERS", 1, CORE_1_INDEX, &TX_ABS_RECYCLE_COUNTERS_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "CSO_CONTEXT_TABLE", 1, CORE_1_INDEX, &CSO_CONTEXT_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "PKTGEN_NO_SBPM_HDRS_CNTR", 1, CORE_1_INDEX, &PKTGEN_NO_SBPM_HDRS_CNTR_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_RX_PSRAM_GET_NEXT_SCRATCHPAD", 1, CORE_1_INDEX, &CPU_RX_PSRAM_GET_NEXT_SCRATCHPAD_1, 4, 1, 1 },
#endif
#if defined BCM6878
	{ "MAX_PKT_LEN_TABLE", 1, CORE_1_INDEX, &MAX_PKT_LEN_TABLE_1, 32, 1, 1 },
#endif
#if defined BCM6878
	{ "VLAN_ACTION_GPE_HANDLER_PTR_TABLE", 1, CORE_1_INDEX, &VLAN_ACTION_GPE_HANDLER_PTR_TABLE_1, 17, 1, 1 },
#endif
#if defined BCM6878
	{ "CODEL_NUM_QUEUES", 1, CORE_1_INDEX, &CODEL_NUM_QUEUES_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "ONE_VALUE", 1, CORE_1_INDEX, &ONE_VALUE_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR", 1, CORE_1_INDEX, &CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "SPDTEST_GEN_PARAM", 1, CORE_1_INDEX, &SPDTEST_GEN_PARAM_1, 4, 1, 1 },
#endif
#if defined BCM6878
	{ "UDPSPDT_TX_PARAMS_TABLE", 1, CORE_1_INDEX, &UDPSPDT_TX_PARAMS_TABLE_1, 4, 1, 1 },
#endif
#if defined BCM6878
	{ "TCPSPDTEST_ENGINE_GLOBAL_TABLE", 1, CORE_1_INDEX, &TCPSPDTEST_ENGINE_GLOBAL_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_RX_INTERRUPT_SCRATCH", 1, CORE_1_INDEX, &CPU_RX_INTERRUPT_SCRATCH_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_RX_INTERRUPT_ID_DDR_ADDR", 1, CORE_1_INDEX, &CPU_RX_INTERRUPT_ID_DDR_ADDR_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "COMMON_REPROCESSING_PD_FIFO_TABLE", 1, CORE_1_INDEX, &COMMON_REPROCESSING_PD_FIFO_TABLE_1, 4, 1, 1 },
#endif
#if defined BCM6878
	{ "UPDATE_FIFO_TABLE", 1, CORE_1_INDEX, &UPDATE_FIFO_TABLE_1, 8, 1, 1 },
#endif
#if defined BCM6878
	{ "VPORT_PBIT_TO_DISCARD_PRIO_VECTOR", 1, CORE_1_INDEX, &VPORT_PBIT_TO_DISCARD_PRIO_VECTOR_1, 16, 1, 1 },
#endif
#if defined BCM6878
	{ "SPDSVC_ANALYZER_PARAMS_TABLE", 1, CORE_1_INDEX, &SPDSVC_ANALYZER_PARAMS_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_RING_DESCRIPTORS_TABLE", 1, CORE_1_INDEX, &CPU_RING_DESCRIPTORS_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM6878
	{ "EMAC_FLOW_CTRL", 1, CORE_1_INDEX, &EMAC_FLOW_CTRL_1, 5, 1, 1 },
#endif
#if defined BCM6878
	{ "DEBUG_SCRATCHPAD", 1, CORE_1_INDEX, &DEBUG_SCRATCHPAD_1, 12, 1, 1 },
#endif
#if defined BCM6878
	{ "VPORT_CFG_EX_TABLE", 1, CORE_1_INDEX, &VPORT_CFG_EX_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_RX_COPY_UPDATE_FIFO_TABLE", 1, CORE_1_INDEX, &CPU_RX_COPY_UPDATE_FIFO_TABLE_1, 8, 1, 1 },
#endif
#if defined BCM6878
	{ "BRIDGE_CFG_TABLE", 1, CORE_1_INDEX, &BRIDGE_CFG_TABLE_1, 2, 1, 1 },
#endif
#if defined BCM6878
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_1_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM6878
	{ "NATC_TBL_CFG", 1, CORE_1_INDEX, &NATC_TBL_CFG_1, 3, 1, 1 },
#endif
#if defined BCM6878
	{ "NATC_COUNTERS_GENERIC_CONTEXT", 1, CORE_1_INDEX, &NATC_COUNTERS_GENERIC_CONTEXT_1, 128, 1, 1 },
#endif
#if defined BCM6878
	{ "REGISTERS_BUFFER", 1, CORE_1_INDEX, &REGISTERS_BUFFER_1, 32, 1, 1 },
#endif
#if defined BCM6878
	{ "SYSTEM_CONFIGURATION", 1, CORE_1_INDEX, &SYSTEM_CONFIGURATION_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_1_INDEX, &LOOPBACK_WAN_FLOW_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "PKTGEN_CURR_SBPM_HDR_PTR", 1, CORE_1_INDEX, &PKTGEN_CURR_SBPM_HDR_PTR_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_RX_COPY_DISPATCHER_CREDIT_TABLE", 1, CORE_1_INDEX, &CPU_RX_COPY_DISPATCHER_CREDIT_TABLE_1, 3, 1, 1 },
#endif
#if defined BCM6878
	{ "PKTGEN_NUM_OF_AVAIL_SBPM_HDRS", 1, CORE_1_INDEX, &PKTGEN_NUM_OF_AVAIL_SBPM_HDRS_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "PKTGEN_SBPM_END_PTR", 1, CORE_1_INDEX, &PKTGEN_SBPM_END_PTR_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "TCAM_IC_CFG_TABLE", 1, CORE_1_INDEX, &TCAM_IC_CFG_TABLE_1, 8, 1, 1 },
#endif
#if defined BCM6878
	{ "SPDSVC_GEN_DISPATCHER_CREDIT_TABLE", 1, CORE_1_INDEX, &SPDSVC_GEN_DISPATCHER_CREDIT_TABLE_1, 3, 1, 1 },
#endif
#if defined BCM6878
	{ "PKTGEN_BAD_GET_NEXT", 1, CORE_1_INDEX, &PKTGEN_BAD_GET_NEXT_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "COMMON_REPROCESSING_DISPATCHER_CREDIT_TABLE", 1, CORE_1_INDEX, &COMMON_REPROCESSING_DISPATCHER_CREDIT_TABLE_1, 3, 1, 1 },
#endif
#if defined BCM6878
	{ "PKTGEN_MAX_UT_PKTS", 1, CORE_1_INDEX, &PKTGEN_MAX_UT_PKTS_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "PKTGEN_SBPM_HDR_BNS", 1, CORE_1_INDEX, &PKTGEN_SBPM_HDR_BNS_1, 28, 1, 1 },
#endif
#if defined BCM6878
	{ "INGRESS_PACKET_BASED_MAPPING", 1, CORE_1_INDEX, &INGRESS_PACKET_BASED_MAPPING_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "PKTGEN_UT_TRIGGER", 1, CORE_1_INDEX, &PKTGEN_UT_TRIGGER_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "COMMON_REPROCESSING_UPDATE_FIFO_TABLE", 1, CORE_1_INDEX, &COMMON_REPROCESSING_UPDATE_FIFO_TABLE_1, 8, 1, 1 },
#endif
#if defined BCM6878
	{ "IPTV_CONFIGURATION_TABLE", 1, CORE_1_INDEX, &IPTV_CONFIGURATION_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "TASK_IDX", 1, CORE_1_INDEX, &TASK_IDX_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "DHD_FPM_THRESHOLDS", 1, CORE_1_INDEX, &DHD_FPM_THRESHOLDS_1, 3, 1, 1 },
#endif
#if defined BCM6878
	{ "FORCE_DSCP", 1, CORE_1_INDEX, &FORCE_DSCP_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "CORE_ID_TABLE", 1, CORE_1_INDEX, &CORE_ID_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "SPDSVC_TCPSPDTEST_COMMON_TABLE", 1, CORE_1_INDEX, &SPDSVC_TCPSPDTEST_COMMON_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "SPDTEST_NUM_OF_UDP_RX_FLOWS", 1, CORE_1_INDEX, &SPDTEST_NUM_OF_UDP_RX_FLOWS_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_VPORT_TO_METER_TABLE", 1, CORE_1_INDEX, &CPU_VPORT_TO_METER_TABLE_1, 40, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE", 1, CORE_1_INDEX, &CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "FPM_GLOBAL_CFG", 1, CORE_1_INDEX, &FPM_GLOBAL_CFG_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "PD_FIFO_TABLE", 1, CORE_1_INDEX, &PD_FIFO_TABLE_1, 2, 1, 1 },
#endif
#if defined BCM6878
	{ "BCM_SPDSVC_STREAM_RX_TS_TABLE", 1, CORE_1_INDEX, &BCM_SPDSVC_STREAM_RX_TS_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_FEED_RING_INTERRUPT_THRESHOLD", 1, CORE_1_INDEX, &CPU_FEED_RING_INTERRUPT_THRESHOLD_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_FEED_RING_INTERRUPT_COUNTER", 1, CORE_1_INDEX, &CPU_FEED_RING_INTERRUPT_COUNTER_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_RECYCLE_RING_DESCRIPTOR_TABLE", 1, CORE_1_INDEX, &CPU_RECYCLE_RING_DESCRIPTOR_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "DEFAULT_VLAN_VID_TABLE", 1, CORE_1_INDEX, &DEFAULT_VLAN_VID_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_REASON_TO_TC", 1, CORE_1_INDEX, &CPU_REASON_TO_TC_1, 64, 1, 1 },
#endif
#if defined BCM6878
	{ "TC_TO_CPU_RXQ", 1, CORE_1_INDEX, &TC_TO_CPU_RXQ_1, 64, 1, 1 },
#endif
#if defined BCM6878
	{ "EXC_TC_TO_CPU_RXQ", 1, CORE_1_INDEX, &EXC_TC_TO_CPU_RXQ_1, 64, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE", 1, CORE_1_INDEX, &CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE_1, 2, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_INTERRUPT_COALESCING_TABLE", 1, CORE_1_INDEX, &CPU_INTERRUPT_COALESCING_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "PKTGEN_SESSION_DATA", 1, CORE_1_INDEX, &PKTGEN_SESSION_DATA_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_FEED_RING_DESCRIPTOR_TABLE", 1, CORE_1_INDEX, &CPU_FEED_RING_DESCRIPTOR_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_RECYCLE_INTERRUPT_SCRATCH", 1, CORE_1_INDEX, &CPU_RECYCLE_INTERRUPT_SCRATCH_1, 2, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_FEED_RING_INDEX_DDR_ADDR_TABLE", 1, CORE_1_INDEX, &CPU_FEED_RING_INDEX_DDR_ADDR_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "VPORT_TO_CPU_OBJ", 1, CORE_1_INDEX, &VPORT_TO_CPU_OBJ_1, 40, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_RX_LOCAL_SCRATCH", 1, CORE_1_INDEX, &CPU_RX_LOCAL_SCRATCH_1, 2, 1, 1 },
#endif
#if defined BCM6878
	{ "TUNNELS_PARSING_CFG", 1, CORE_1_INDEX, &TUNNELS_PARSING_CFG_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "PKTGEN_FPM_UG_MGMT", 1, CORE_1_INDEX, &PKTGEN_FPM_UG_MGMT_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_1_INDEX, &RX_MIRRORING_CONFIGURATION_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "ECN_IPV6_REMARK_TABLE", 1, CORE_1_INDEX, &ECN_IPV6_REMARK_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "MULTI_FLOW_FLAG_TABLE", 1, CORE_1_INDEX, &MULTI_FLOW_FLAG_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "PKTGEN_SBPM_EXTS", 1, CORE_1_INDEX, &PKTGEN_SBPM_EXTS_1, 4, 1, 1 },
#endif
#if defined BCM6878
	{ "IPTV_CFG_TABLE", 1, CORE_1_INDEX, &IPTV_CFG_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "PKTGEN_BBMSG_REPLY_SCRATCH", 1, CORE_1_INDEX, &PKTGEN_BBMSG_REPLY_SCRATCH_1, 2, 1, 1 },
#endif
#if defined BCM6878
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_1_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_1_INDEX, &BITS_CALC_MASKS_TABLE_1, 4, 1, 1 },
#endif
#if defined BCM6878
	{ "DEBUG_PRINT_TABLE", 1, CORE_1_INDEX, &DEBUG_PRINT_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "TUNNEL_HEADER_PSRAM_BUFFER", 1, CORE_1_INDEX, &TUNNEL_HEADER_PSRAM_BUFFER_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "SRAM_DUMMY_STORE", 1, CORE_1_INDEX, &SRAM_DUMMY_STORE_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "RATE_LIMIT_OVERHEAD", 1, CORE_1_INDEX, &RATE_LIMIT_OVERHEAD_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "INGRESS_QOS_DONT_DROP_RATIO", 1, CORE_1_INDEX, &INGRESS_QOS_DONT_DROP_RATIO_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "INGRESS_QOS_WAN_UNTAGGED_PRIORITY", 1, CORE_1_INDEX, &INGRESS_QOS_WAN_UNTAGGED_PRIORITY_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "NAT_CACHE_CFG", 1, CORE_1_INDEX, &NAT_CACHE_CFG_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_FEED_RING_CACHE_OFFSET", 1, CORE_1_INDEX, &CPU_FEED_RING_CACHE_OFFSET_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_1_INDEX, &NAT_CACHE_KEY0_MASK_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_REDIRECT_MODE", 1, CORE_1_INDEX, &CPU_REDIRECT_MODE_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "CSO_DISABLE", 1, CORE_1_INDEX, &CSO_DISABLE_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_1_INDEX, &DEBUG_PRINT_CORE_LOCK_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_1_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_RECYCLE_SHADOW_RD_IDX", 1, CORE_1_INDEX, &CPU_RECYCLE_SHADOW_RD_IDX_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_1_INDEX, &INGRESS_FILTER_1588_CFG_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_1_INDEX, &RX_MIRRORING_DIRECT_ENABLE_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "PKTGEN_TX_STREAM_SCRATCH_TABLE", 1, CORE_1_INDEX, &PKTGEN_TX_STREAM_SCRATCH_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_RECYCLE_SHADOW_WR_IDX", 1, CORE_1_INDEX, &CPU_RECYCLE_SHADOW_WR_IDX_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_RECYCLE_NEXT_PTR_TABLE", 1, CORE_1_INDEX, &CPU_RECYCLE_NEXT_PTR_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_1_INDEX, &NATC_L2_VLAN_KEY_MASK_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "MULTICAST_KEY_MASK", 1, CORE_1_INDEX, &MULTICAST_KEY_MASK_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "INGRESS_FILTER_CFG", 1, CORE_1_INDEX, &INGRESS_FILTER_CFG_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "NATC_L2_TOS_MASK", 1, CORE_1_INDEX, &NATC_L2_TOS_MASK_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "SCT_FILTER", 1, CORE_1_INDEX, &SCT_FILTER_1, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_TX_SCRATCHPAD", 1, CORE_2_INDEX, &CPU_TX_SCRATCHPAD_2, 128, 1, 1 },
#endif
#if defined BCM6878
	{ "RX_FLOW_TABLE", 1, CORE_2_INDEX, &RX_FLOW_TABLE_2, 320, 1, 1 },
#endif
#if defined BCM6878
	{ "VPORT_CFG_TABLE", 1, CORE_2_INDEX, &VPORT_CFG_TABLE_2, 16, 1, 1 },
#endif
#if defined BCM6878
	{ "FW_POLICER_BUDGET", 1, CORE_2_INDEX, &FW_POLICER_BUDGET_2, 80, 1, 1 },
#endif
#if defined BCM6878
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_2_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_2, 8, 1, 1 },
#endif
#if defined BCM6878
	{ "TUNNEL_TABLE", 1, CORE_2_INDEX, &TUNNEL_TABLE_2, 4, 1, 1 },
#endif
#if defined BCM6878
	{ "TCAM_IC_CMD_TABLE", 1, CORE_2_INDEX, &TCAM_IC_CMD_TABLE_2, 9, 16, 1 },
#endif
#if defined BCM6878
	{ "DSCP_TO_PBITS_MAP_TABLE", 1, CORE_2_INDEX, &DSCP_TO_PBITS_MAP_TABLE_2, 1, 64, 1 },
#endif
#if defined BCM6878
	{ "CPU_TX_DBG_CNTRS_TABLE", 1, CORE_2_INDEX, &CPU_TX_DBG_CNTRS_TABLE_2, 2, 1, 1 },
#endif
#if defined BCM6878
	{ "POLICER_PARAMS_TABLE", 1, CORE_2_INDEX, &POLICER_PARAMS_TABLE_2, 80, 1, 1 },
#endif
#if defined BCM6878
	{ "FLOODING_HEADER_FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_2_INDEX, &FLOODING_HEADER_FPM_POOL_NUMBER_MAPPING_TABLE_2, 32, 1, 1 },
#endif
#if defined BCM6878
	{ "GLOBAL_DSCP_TO_PBITS_TABLE", 1, CORE_2_INDEX, &GLOBAL_DSCP_TO_PBITS_TABLE_2, 1, 64, 1 },
#endif
#if defined BCM6878
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_2_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_2, 128, 1, 1 },
#endif
#if defined BCM6878
	{ "FW_POLICER_CBS", 1, CORE_2_INDEX, &FW_POLICER_CBS_2, 80, 1, 1 },
#endif
#if defined BCM6878
	{ "TUNNEL_DST_IP_LKP_TABLE", 1, CORE_2_INDEX, &TUNNEL_DST_IP_LKP_TABLE_2, 16, 1, 1 },
#endif
#if defined BCM6878
	{ "LAYER2_HEADER_COPY_MAPPING_TABLE", 1, CORE_2_INDEX, &LAYER2_HEADER_COPY_MAPPING_TABLE_2, 30, 1, 1 },
#endif
#if defined BCM6878
	{ "MCAST_FPM_TOKENS_READ_FLAG", 1, CORE_2_INDEX, &MCAST_FPM_TOKENS_READ_FLAG_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "DS_PACKET_BUFFER", 1, CORE_2_INDEX, &DS_PACKET_BUFFER_2, 8, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_RECYCLE_SRAM_PD_FIFO", 1, CORE_2_INDEX, &CPU_RECYCLE_SRAM_PD_FIFO_2, 16, 1, 1 },
#endif
#if defined BCM6878
	{ "TX_FLOW_TABLE", 1, CORE_2_INDEX, &TX_FLOW_TABLE_2, 144, 1, 1 },
#endif
#if defined BCM6878
	{ "VPORT_TO_DSCP_TO_PBITS_TABLE", 1, CORE_2_INDEX, &VPORT_TO_DSCP_TO_PBITS_TABLE_2, 16, 1, 1 },
#endif
#if defined BCM6878
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_2_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_2, 32, 1, 1 },
#endif
#if defined BCM6878
	{ "DHD_CODEL_BIAS_SLOPE_TABLE", 1, CORE_2_INDEX, &DHD_CODEL_BIAS_SLOPE_TABLE_2, 11, 1, 1 },
#endif
#if defined BCM6878
	{ "MUTEX_TABLE", 1, CORE_2_INDEX, &MUTEX_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_2_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_2, 16, 1, 1 },
#endif
#if defined BCM6878
	{ "DHD_TX_POST_PD_FIFO_TABLE", 1, CORE_2_INDEX, &DHD_TX_POST_PD_FIFO_TABLE_2, 12, 1, 1 },
#endif
#if defined BCM6878
	{ "QUEUE_THRESHOLD_VECTOR", 1, CORE_2_INDEX, &QUEUE_THRESHOLD_VECTOR_2, 9, 1, 1 },
#endif
#if defined BCM6878
	{ "ONE_VALUE", 1, CORE_2_INDEX, &ONE_VALUE_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_2_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "DHD_FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_2_INDEX, &DHD_FPM_POOL_NUMBER_MAPPING_TABLE_2, 16, 1, 1 },
#endif
#if defined BCM6878
	{ "TC_TO_QUEUE_TABLE", 1, CORE_2_INDEX, &TC_TO_QUEUE_TABLE_2, 17, 1, 1 },
#endif
#if defined BCM6878
	{ "FW_POLICER_BUDGET_REMAINDER", 1, CORE_2_INDEX, &FW_POLICER_BUDGET_REMAINDER_2, 80, 1, 1 },
#endif
#if defined BCM6878
	{ "TIMER_COMMON_CTR_REP", 1, CORE_2_INDEX, &TIMER_COMMON_CTR_REP_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "CODEL_ENABLE_TABLE", 1, CORE_2_INDEX, &CODEL_ENABLE_TABLE_2, 6, 1, 1 },
#endif
#if defined BCM6878
	{ "NULL_BUFFER", 1, CORE_2_INDEX, &NULL_BUFFER_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "DHD_FLOW_RING_CACHE_CTX_TABLE", 1, CORE_2_INDEX, &DHD_FLOW_RING_CACHE_CTX_TABLE_2, 16, 1, 1 },
#endif
#if defined BCM6878
	{ "PBIT_TO_QUEUE_TABLE", 1, CORE_2_INDEX, &PBIT_TO_QUEUE_TABLE_2, 17, 1, 1 },
#endif
#if defined BCM6878
	{ "DEBUG_SCRATCHPAD", 1, CORE_2_INDEX, &DEBUG_SCRATCHPAD_2, 12, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR", 1, CORE_2_INDEX, &CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "VLAN_ACTION_GPE_HANDLER_PTR_TABLE", 1, CORE_2_INDEX, &VLAN_ACTION_GPE_HANDLER_PTR_TABLE_2, 17, 1, 1 },
#endif
#if defined BCM6878
	{ "CODEL_NUM_QUEUES", 1, CORE_2_INDEX, &CODEL_NUM_QUEUES_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "TIMER_COMMON_TIMER_VALUE", 1, CORE_2_INDEX, &TIMER_COMMON_TIMER_VALUE_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "SPDSVC_WLAN_TXPOST_PARAMS_TABLE", 1, CORE_2_INDEX, &SPDSVC_WLAN_TXPOST_PARAMS_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_RX_INTERRUPT_ID_DDR_ADDR", 1, CORE_2_INDEX, &CPU_RX_INTERRUPT_ID_DDR_ADDR_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "BUFFER_ALLOC_REPLY", 1, CORE_2_INDEX, &BUFFER_ALLOC_REPLY_2, 2, 1, 1 },
#endif
#if defined BCM6878
	{ "DHD_POST_COMMON_RADIO_DATA", 1, CORE_2_INDEX, &DHD_POST_COMMON_RADIO_DATA_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "DBG_DUMP_TABLE", 1, CORE_2_INDEX, &DBG_DUMP_TABLE_2, 64, 1, 1 },
#endif
#if defined BCM6878
	{ "MIRRORING_SCRATCH", 1, CORE_2_INDEX, &MIRRORING_SCRATCH_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "VPORT_PBIT_TO_DISCARD_PRIO_VECTOR", 1, CORE_2_INDEX, &VPORT_PBIT_TO_DISCARD_PRIO_VECTOR_2, 16, 1, 1 },
#endif
#if defined BCM6878
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_2_INDEX, &LOOPBACK_QUEUE_TABLE_2, 16, 1, 1 },
#endif
#if defined BCM6878
	{ "DHD_TX_POST_UPDATE_FIFO_TABLE", 1, CORE_2_INDEX, &DHD_TX_POST_UPDATE_FIFO_TABLE_2, 8, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_RECYCLE_INTERRUPT_COALESCING_TABLE", 1, CORE_2_INDEX, &CPU_RECYCLE_INTERRUPT_COALESCING_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "SPDTEST_GEN_PARAM", 1, CORE_2_INDEX, &SPDTEST_GEN_PARAM_2, 4, 1, 1 },
#endif
#if defined BCM6878
	{ "DHD_AUX_INFO_CACHE_TABLE", 1, CORE_2_INDEX, &DHD_AUX_INFO_CACHE_TABLE_2, 16, 1, 1 },
#endif
#if defined BCM6878
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_2_INDEX, &INGRESS_FILTER_PROFILE_TABLE_2, 16, 1, 1 },
#endif
#if defined BCM6878
	{ "MAX_PKT_LEN_TABLE", 1, CORE_2_INDEX, &MAX_PKT_LEN_TABLE_2, 32, 1, 1 },
#endif
#if defined BCM6878
	{ "DHD_DOORBELL_TX_POST_VALUE", 1, CORE_2_INDEX, &DHD_DOORBELL_TX_POST_VALUE_2, 16, 1, 1 },
#endif
#if defined BCM6878
	{ "DHD_COMPLETE_COMMON_RADIO_DATA", 1, CORE_2_INDEX, &DHD_COMPLETE_COMMON_RADIO_DATA_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR", 1, CORE_2_INDEX, &DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "VPORT_CFG_EX_TABLE", 1, CORE_2_INDEX, &VPORT_CFG_EX_TABLE_2, 16, 1, 1 },
#endif
#if defined BCM6878
	{ "EMAC_FLOW_CTRL_BUDGET", 1, CORE_2_INDEX, &EMAC_FLOW_CTRL_BUDGET_2, 5, 1, 1 },
#endif
#if defined BCM6878
	{ "TASK_IDX", 1, CORE_2_INDEX, &TASK_IDX_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "DHD_BACKUP_BASE_ADDR", 1, CORE_2_INDEX, &DHD_BACKUP_BASE_ADDR_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_TX_RING_DESCRIPTOR_TABLE", 1, CORE_2_INDEX, &CPU_TX_RING_DESCRIPTOR_TABLE_2, 2, 1, 1 },
#endif
#if defined BCM6878
	{ "EMAC_FLOW_CTRL", 1, CORE_2_INDEX, &EMAC_FLOW_CTRL_2, 5, 1, 1 },
#endif
#if defined BCM6878
	{ "NATC_COUNTERS_GENERIC_CONTEXT", 1, CORE_2_INDEX, &NATC_COUNTERS_GENERIC_CONTEXT_2, 128, 1, 1 },
#endif
#if defined BCM6878
	{ "DHD_HW_CFG", 1, CORE_2_INDEX, &DHD_HW_CFG_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "SPDSVC_ANALYZER_PARAMS_TABLE", 1, CORE_2_INDEX, &SPDSVC_ANALYZER_PARAMS_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "BRIDGE_CFG_TABLE", 1, CORE_2_INDEX, &BRIDGE_CFG_TABLE_2, 2, 1, 1 },
#endif
#if defined BCM6878
	{ "NATC_TBL_CFG", 1, CORE_2_INDEX, &NATC_TBL_CFG_2, 3, 1, 1 },
#endif
#if defined BCM6878
	{ "REGISTERS_BUFFER", 1, CORE_2_INDEX, &REGISTERS_BUFFER_2, 32, 1, 1 },
#endif
#if defined BCM6878
	{ "SYSTEM_CONFIGURATION", 1, CORE_2_INDEX, &SYSTEM_CONFIGURATION_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "EMAC_FLOW_CTRL_VECTOR", 1, CORE_2_INDEX, &EMAC_FLOW_CTRL_VECTOR_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_2_INDEX, &LOOPBACK_WAN_FLOW_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "FORCE_DSCP", 1, CORE_2_INDEX, &FORCE_DSCP_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_2_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_2, 16, 1, 1 },
#endif
#if defined BCM6878
	{ "TCAM_IC_CFG_TABLE", 1, CORE_2_INDEX, &TCAM_IC_CFG_TABLE_2, 8, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE", 1, CORE_2_INDEX, &CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE_2, 3, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_TX_VLAN_STATS_ENABLE", 1, CORE_2_INDEX, &CPU_TX_VLAN_STATS_ENABLE_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "CORE_ID_TABLE", 1, CORE_2_INDEX, &CORE_ID_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "SPDTEST_NUM_OF_UDP_RX_FLOWS", 1, CORE_2_INDEX, &SPDTEST_NUM_OF_UDP_RX_FLOWS_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "ECN_IPV6_REMARK_TABLE", 1, CORE_2_INDEX, &ECN_IPV6_REMARK_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_TX_INGRESS_DISPATCHER_CREDIT_TABLE", 1, CORE_2_INDEX, &CPU_TX_INGRESS_DISPATCHER_CREDIT_TABLE_2, 3, 1, 1 },
#endif
#if defined BCM6878
	{ "DHD_RX_POST_RING_SIZE", 1, CORE_2_INDEX, &DHD_RX_POST_RING_SIZE_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "DHD_RX_COMPLETE_RING_SIZE", 1, CORE_2_INDEX, &DHD_RX_COMPLETE_RING_SIZE_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "DEFAULT_VLAN_VID_TABLE", 1, CORE_2_INDEX, &DEFAULT_VLAN_VID_TABLE_2, 16, 1, 1 },
#endif
#if defined BCM6878
	{ "IPTV_CONFIGURATION_TABLE", 1, CORE_2_INDEX, &IPTV_CONFIGURATION_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "DHD_TX_COMPLETE_RING_SIZE", 1, CORE_2_INDEX, &DHD_TX_COMPLETE_RING_SIZE_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_2_INDEX, &RX_MIRRORING_CONFIGURATION_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "FW_POLICER_VECTOR", 1, CORE_2_INDEX, &FW_POLICER_VECTOR_2, 3, 1, 1 },
#endif
#if defined BCM6878
	{ "MULTI_FLOW_FLAG_TABLE", 1, CORE_2_INDEX, &MULTI_FLOW_FLAG_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "SRAM_DUMMY_STORE", 1, CORE_2_INDEX, &SRAM_DUMMY_STORE_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "RATE_LIMIT_OVERHEAD", 1, CORE_2_INDEX, &RATE_LIMIT_OVERHEAD_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "INGRESS_QOS_DONT_DROP_RATIO", 1, CORE_2_INDEX, &INGRESS_QOS_DONT_DROP_RATIO_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_2_INDEX, &BITS_CALC_MASKS_TABLE_2, 4, 1, 1 },
#endif
#if defined BCM6878
	{ "DHD_FPM_THRESHOLDS", 1, CORE_2_INDEX, &DHD_FPM_THRESHOLDS_2, 3, 1, 1 },
#endif
#if defined BCM6878
	{ "INGRESS_QOS_WAN_UNTAGGED_PRIORITY", 1, CORE_2_INDEX, &INGRESS_QOS_WAN_UNTAGGED_PRIORITY_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_REDIRECT_MODE", 1, CORE_2_INDEX, &CPU_REDIRECT_MODE_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_2_INDEX, &DEBUG_PRINT_CORE_LOCK_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_2_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "DHD_RX_COMPLETE_DISPATCHER_CREDIT_TABLE", 1, CORE_2_INDEX, &DHD_RX_COMPLETE_DISPATCHER_CREDIT_TABLE_2, 3, 1, 1 },
#endif
#if defined BCM6878
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_2_INDEX, &INGRESS_FILTER_1588_CFG_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_2_INDEX, &RX_MIRRORING_DIRECT_ENABLE_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "DHD_TX_COMPLETE_DISPATCHER_CREDIT_TABLE", 1, CORE_2_INDEX, &DHD_TX_COMPLETE_DISPATCHER_CREDIT_TABLE_2, 3, 1, 1 },
#endif
#if defined BCM6878
	{ "DHD_TX_POST_FLOW_RING_BUFFER", 1, CORE_2_INDEX, &DHD_TX_POST_FLOW_RING_BUFFER_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "DHD_MIRRORING_DISPATCHER_CREDIT_TABLE", 1, CORE_2_INDEX, &DHD_MIRRORING_DISPATCHER_CREDIT_TABLE_2, 3, 1, 1 },
#endif
#if defined BCM6878
	{ "DHD_TX_COMPLETE_FLOW_RING_BUFFER", 1, CORE_2_INDEX, &DHD_TX_COMPLETE_FLOW_RING_BUFFER_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "SPDSVC_WLAN_GEN_PARAMS_TABLE", 1, CORE_2_INDEX, &SPDSVC_WLAN_GEN_PARAMS_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "FPM_GLOBAL_CFG", 1, CORE_2_INDEX, &FPM_GLOBAL_CFG_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_RECYCLE_RING_DESCRIPTOR_TABLE", 1, CORE_2_INDEX, &CPU_RECYCLE_RING_DESCRIPTOR_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "RING_CPU_TX_DESCRIPTOR_DATA_TABLE", 1, CORE_2_INDEX, &RING_CPU_TX_DESCRIPTOR_DATA_TABLE_2, 2, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE", 1, CORE_2_INDEX, &CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE_2, 2, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_FEED_RING_DESCRIPTOR_TABLE", 1, CORE_2_INDEX, &CPU_FEED_RING_DESCRIPTOR_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_RECYCLE_INTERRUPT_SCRATCH", 1, CORE_2_INDEX, &CPU_RECYCLE_INTERRUPT_SCRATCH_2, 2, 1, 1 },
#endif
#if defined BCM6878
	{ "INGRESS_PACKET_BASED_MAPPING", 1, CORE_2_INDEX, &INGRESS_PACKET_BASED_MAPPING_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "DEBUG_PRINT_TABLE", 1, CORE_2_INDEX, &DEBUG_PRINT_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "TUNNEL_HEADER_PSRAM_BUFFER", 1, CORE_2_INDEX, &TUNNEL_HEADER_PSRAM_BUFFER_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE", 1, CORE_2_INDEX, &CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_TX_RING_INDICES_VALUES_TABLE", 1, CORE_2_INDEX, &CPU_TX_RING_INDICES_VALUES_TABLE_2, 2, 1, 1 },
#endif
#if defined BCM6878
	{ "TUNNELS_PARSING_CFG", 1, CORE_2_INDEX, &TUNNELS_PARSING_CFG_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "DHD_FLOW_RING_CACHE_LKP_TABLE", 1, CORE_2_INDEX, &DHD_FLOW_RING_CACHE_LKP_TABLE_2, 16, 1, 1 },
#endif
#if defined BCM6878
	{ "MULTICAST_KEY_MASK", 1, CORE_2_INDEX, &MULTICAST_KEY_MASK_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "IPTV_CFG_TABLE", 1, CORE_2_INDEX, &IPTV_CFG_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "DHD_DOORBELL_TX_COMPLETE_VALUE", 1, CORE_2_INDEX, &DHD_DOORBELL_TX_COMPLETE_VALUE_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_2_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "DHD_FPM_REPLY", 1, CORE_2_INDEX, &DHD_FPM_REPLY_2, 8, 1, 1 },
#endif
#if defined BCM6878
	{ "NAT_CACHE_CFG", 1, CORE_2_INDEX, &NAT_CACHE_CFG_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "DHD_DOORBELL_RX_COMPLETE_VALUE", 1, CORE_2_INDEX, &DHD_DOORBELL_RX_COMPLETE_VALUE_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "EMAC_FLOW_CTRL_BUDGET_REMAINDER", 1, CORE_2_INDEX, &EMAC_FLOW_CTRL_BUDGET_REMAINDER_2, 5, 1, 1 },
#endif
#if defined BCM6878
	{ "DHD_DOORBELL_RX_POST_VALUE", 1, CORE_2_INDEX, &DHD_DOORBELL_RX_POST_VALUE_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_2_INDEX, &NAT_CACHE_KEY0_MASK_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "DHD_CPU_INT_ID", 1, CORE_2_INDEX, &DHD_CPU_INT_ID_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_RECYCLE_SHADOW_RD_IDX", 1, CORE_2_INDEX, &CPU_RECYCLE_SHADOW_RD_IDX_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "DHD_BACKUP_INDEX_CACHE", 1, CORE_2_INDEX, &DHD_BACKUP_INDEX_CACHE_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_RECYCLE_SHADOW_WR_IDX", 1, CORE_2_INDEX, &CPU_RECYCLE_SHADOW_WR_IDX_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_2_INDEX, &NATC_L2_VLAN_KEY_MASK_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_RECYCLE_NEXT_PTR_TABLE", 1, CORE_2_INDEX, &CPU_RECYCLE_NEXT_PTR_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "INGRESS_FILTER_CFG", 1, CORE_2_INDEX, &INGRESS_FILTER_CFG_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "NATC_L2_TOS_MASK", 1, CORE_2_INDEX, &NATC_L2_TOS_MASK_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "DHD_RX_COMPLETE_FLOW_RING_BUFFER", 1, CORE_2_INDEX, &DHD_RX_COMPLETE_FLOW_RING_BUFFER_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "DHD_RX_POST_FLOW_RING_BUFFER", 1, CORE_2_INDEX, &DHD_RX_POST_FLOW_RING_BUFFER_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "DHD_L2_HEADER", 1, CORE_2_INDEX, &DHD_L2_HEADER_2, 24, 1, 1 },
#endif
#if defined BCM6878
	{ "CPU_TX_SYNC_FIFO_TABLE", 1, CORE_2_INDEX, &CPU_TX_SYNC_FIFO_TABLE_2, 2, 1, 1 },
#endif
#if defined BCM6878
	{ "DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE", 1, CORE_2_INDEX, &DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6878
	{ "SCT_FILTER", 1, CORE_2_INDEX, &SCT_FILTER_2, 1, 1, 1 },
#endif
};

TABLE_STACK_STRUCT RUNNER_STACK_TABLES[NUMBER_OF_STACK_TABLES] =
{

};
