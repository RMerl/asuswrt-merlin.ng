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
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT BASIC_RATE_LIMITER_TABLE_DS =
{
	16,
	{
		{ dump_RDD_BASIC_RATE_LIMITER_DESCRIPTOR, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_PD_FIFO_TABLE =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT COMPLEX_SCHEDULER_TABLE =
{
	64,
	{
		{ dump_RDD_COMPLEX_SCHEDULER_DESCRIPTOR, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DS_TM_PD_FIFO_TABLE =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT BASIC_SCHEDULER_TABLE_DS =
{
	16,
	{
		{ dump_RDD_BASIC_SCHEDULER_DESCRIPTOR, 0x1600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DS_TM_SCHEDULING_QUEUE_TABLE =
{
	8,
	{
		{ dump_RDD_SCHEDULING_QUEUE_DESCRIPTOR, 0x1800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DS_TM_TM_FLOW_CNTR_TABLE =
{
	1,
	{
		{ dump_RDD_TM_FLOW_CNTR_ENTRY, 0x1a80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_COMPLEX_SCHEDULER_TABLE =
{
	64,
	{
		{ dump_RDD_COMPLEX_SCHEDULER_DESCRIPTOR, 0x1ac0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE =
{
	8,
	{
		{ dump_RDD_SCHEDULING_QUEUE_DESCRIPTOR, 0x1b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, 0x1c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DBG_DUMP_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT REPORTING_QUEUE_DESCRIPTOR_TABLE =
{
	16,
	{
		{ dump_RDD_REPORTING_QUEUE_DESCRIPTOR, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT REPORTING_QUEUE_ACCUMULATED_TABLE =
{
	16,
	{
		{ dump_RDD_QM_QUEUE_COUNTER_DATA, 0x2500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT REPORTING_COUNTER_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x26a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DS_TM_TM_ACTION_PTR_TABLE =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x26c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT BUDGET_ALLOCATION_TIMER_VALUE =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x26e2 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT TM_HW_FLUSH =
{
	4,
	{
		{ dump_RDD_HW_FLUSH_ENTRY, 0x26e4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DS_TM_BBH_TX_QUEUE_ID_TABLE =
{
	4,
	{
		{ dump_RDD_BBH_TX_QUEUE_ID_ENTRY, 0x26e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DS_TM_FLUSH_DISPATCHER_CREDIT_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x26f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT GHOST_REPORTING_GLOBAL_CFG =
{
	4,
	{
		{ dump_RDD_GHOST_REPORTING_GLOBAL_CFG_ENTRY, 0x26fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT NATC_TBL_CFG =
{
	24,
	{
		{ dump_RDD_NATC_TBL_CONFIGURATION, 0x2700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2760 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DS_TM_SCHEDULING_FLUSH_VECTOR =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x27e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DS_TM_BB_DESTINATION_TABLE =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x27f4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_BUDGET_ALLOCATION_TIMER_VALUE =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x27f6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE =
{
	8,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x27f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_TM_ACTION_PTR_TABLE =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT SCHEDULING_FLUSH_GLOBAL_CFG =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2822 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2823 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT TASK_IDX =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2824 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT ACB_PORT_STATUS_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2828 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_DISPATCHER_CREDIT_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2830 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x283c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x283e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT RATE_LIMIT_OVERHEAD =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x283f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT UPDATE_FIFO_TABLE =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x2840 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DS_TM_SCHEDULING_QUEUE_AGING_VECTOR =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2860 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DS_TM_SCHEDULING_GLOBAL_FLUSH_CFG =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2874 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DS_TM_FIRST_QUEUE_MAPPING =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2875 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DS_TM_FLUSH_AGGREGATION_TASK_DISABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2876 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_FIRST_QUEUE_MAPPING =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2877 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT EGRESS_PORT_TO_BROADCOM_SWITCH_PORT_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2878 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_UPDATE_FIFO_TABLE =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x2880 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_SCHEDULING_QUEUE_AGING_VECTOR =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x28a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x28b4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT BBH_TX_INGRESS_COUNTER_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x28b5 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT EGRESS_PORT_TO_IMP_PORT_MAPPING_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x28b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DS_TM_SCHEDULING_AGGREGATION_CONTEXT_VECTOR =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x28c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x28d8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT GHOST_REPORTING_QUEUE_STATUS_BIT_VECTOR_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2900 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT MIRRORING_SCRATCH =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x2918 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT QUEUE_TO_REPORT_BIT_VECTOR =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2920 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT REPORT_BBH_TX_QUEUE_ID_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2938 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT ACB_PACKET_SENT_PD_COPY =
{
	16,
	{
		{ dump_RDD_BYTES_16, 0x2940 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT SPDSVC_BBH_TX_PARAMS_TABLE =
{
	12,
	{
		{ dump_RDD_SPDSVC_BBH_TX_PARAMS, 0x2950 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_RATE_LIMITER_VALID_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2960 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_DISPATCHER_CREDIT_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT RATE_LIMITER_VALID_TABLE_DS =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, 0x2990 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG =
{
	12,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, 0x29a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT XGPON_REPORT_ZERO_SENT_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x29b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DS_TM_BBH_TX_EGRESS_COUNTER_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x29c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT BBH_TX_EGRESS_REPORT_COUNTER_TABLE =
{
	8,
	{
		{ dump_RDD_BBH_TX_EGRESS_COUNTER_ENTRY, 0x29c8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DS_TM_BBH_QUEUE_TABLE =
{
	4,
	{
		{ dump_RDD_BBH_QUEUE_DESCRIPTOR, 0x2a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DS_TM_BBH_TX_ABS_COUNTER_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2a40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DS_TM_EGRESS_PORT_RR_TABLE =
{
	2,
	{
		{ dump_RDD_DS_TM_EGRESS_PORT_RR_ENTRY, 0x2b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_RATE_LIMITER_TABLE =
{
	16,
	{
		{ dump_RDD_BASIC_RATE_LIMITER_DESCRIPTOR, 0x3000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DEBUG_TRACE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DDR_PREFETCH_PINGPONG_BUFFER_1 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT BASIC_RATE_LIMITER_TABLE_US_1 =
{
	16,
	{
		{ dump_RDD_BASIC_RATE_LIMITER_DESCRIPTOR, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DDR_PREFETCH_PD_FIFO_1 =
{
	16,
	{
		{ dump_RDD_BBH_TX_DESCRIPTOR, 0x1800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT COMPLEX_SCHEDULER_TABLE_1 =
{
	64,
	{
		{ dump_RDD_COMPLEX_SCHEDULER_DESCRIPTOR, 0x1c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT US_TM_PD_FIFO_TABLE_1 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT BASIC_SCHEDULER_TABLE_US_1 =
{
	16,
	{
		{ dump_RDD_BASIC_SCHEDULER_DESCRIPTOR, 0x2a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT US_TM_SCHEDULING_QUEUE_TABLE_1 =
{
	8,
	{
		{ dump_RDD_SCHEDULING_QUEUE_DESCRIPTOR, 0x2c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT US_TM_PON_TM_FLOW_CNTR_TABLE_1 =
{
	1,
	{
		{ dump_RDD_TM_FLOW_CNTR_ENTRY, 0x2e80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT PFC_FRAME_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2ec0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT US_TM_DSL_TM_FLOW_CNTR_TABLE_1 =
{
	1,
	{
		{ dump_RDD_TM_FLOW_CNTR_ENTRY, 0x2f00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_1 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, 0x3000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DDR_PREFETCH_SBPM_FIFO_1 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x3200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DBG_DUMP_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT NATC_TBL_CFG_1 =
{
	24,
	{
		{ dump_RDD_NATC_TBL_CONFIGURATION, 0x3500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3560 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x35e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT US_TM_BBH_QUEUE_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BBH_QUEUE_DESCRIPTOR, 0x3600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT US_TM_AE2P5_BBH_TX_QUEUE_ID_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BBH_TX_QUEUE_ID_ENTRY, 0x36c8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DIRECT_PROCESSING_PD_TABLE_1 =
{
	16,
	{
		{ dump_RDD_PROCESSING_RX_DESCRIPTOR, 0x36d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DIRECT_PROCESSING_FLOW_CNTR_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x36e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT BBH_TX_EPON_INGRESS_COUNTER_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT US_TM_AE10_BBH_TX_QUEUE_ID_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BBH_TX_QUEUE_ID_ENTRY, 0x3728 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DDR_PREFETCH_COPIED_FPM_TABLE_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3730 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT US_TM_TM_ACTION_PTR_TABLE_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3740 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT BUDGET_ALLOCATION_TIMER_VALUE_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3762 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT TM_HW_FLUSH_1 =
{
	4,
	{
		{ dump_RDD_HW_FLUSH_ENTRY, 0x3764 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT US_TM_DSL_BBH_TX_QUEUE_ID_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BBH_TX_QUEUE_ID_ENTRY, 0x3768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT OVERALL_RATE_LIMITER_TABLE_1 =
{
	16,
	{
		{ dump_RDD_OVERALL_RATE_LIMITER_DESCRIPTOR, 0x3770 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT UPDATE_FIFO_TABLE_1 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x3780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT PFC_TX_STATUS_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x37a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT EPON_UPDATE_FIFO_TABLE_1 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x37c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT PFC_PD_TABLE_1 =
{
	16,
	{
		{ dump_RDD_PROCESSING_RX_DESCRIPTOR, 0x37e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT US_TM_PON_BBH_TX_EGRESS_COUNTER_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DIRECT_PROCESSING_EPON_CONTROL_SCRATCH_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3820 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT US_TM_BB_DESTINATION_TABLE_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3836 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT US_TM_PON_BBH_TX_QUEUE_ID_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BBH_TX_QUEUE_ID_ENTRY, 0x3838 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT US_TM_PON_BBH_TX_ABS_COUNTER_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3840 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT US_TM_SCHEDULING_FLUSH_VECTOR_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3860 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_1 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, 0x3874 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT US_TM_EPON_BBH_TX_QUEUE_ID_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BBH_TX_QUEUE_ID_ENTRY, 0x3878 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT BBH_TX_EPON_EGRESS_COUNTER_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3880 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT US_TM_SCHEDULING_QUEUE_AGING_VECTOR_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x38a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT TASK_IDX_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x38b4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_1 =
{
	8,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x38b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT US_TM_SCHEDULING_AGGREGATION_CONTEXT_VECTOR_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x38c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT SCHEDULING_FLUSH_GLOBAL_CFG_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x38d4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x38d5 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT TX_TASK_DDR_PREFETCH_FIFO_PTR_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x38d6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x38d8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT SCRATCH_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3900 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT RATE_LIMITER_VALID_TABLE_US_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3920 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT US_TM_FLUSH_DISPATCHER_CREDIT_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3930 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DDR_READ_DDR_PREFETCH_FIFO_PTR_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x393c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT PSRAM_WRITE_DDR_PREFETCH_FIFO_PTR_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x393e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT US_TM_DSL_BBH_TX_EGRESS_COUNTER_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3940 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DISPATCHER_CREDIT_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3950 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DDR_READ_DDR_PREFETCH_PINGPONG_BUFFER_PTR_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x395c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT PSRAM_WRITE_DDR_PREFETCH_PINGPONG_BUFFER_PTR_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x395e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT WAN_TX_DDR_READ_DISPATCHER_CREDIT_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3960 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT WRITE_DDR_PREFETCH_SBPM_FIFO_PTR_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x396c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT READ_DDR_PREFETCH_SBPM_FIFO_PTR_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x396e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT WAN_TX_PSRAM_WRITE_DISPATCHER_CREDIT_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT TX_TASK_DDR_READ_DISP_BB_ID_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x397c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT PFC_TX_ENABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x397e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x397f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT US_TM_DSL_BBH_TX_ABS_COUNTER_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT SPDSVC_BBH_TX_PARAMS_TABLE_1 =
{
	12,
	{
		{ dump_RDD_SPDSVC_BBH_TX_PARAMS, 0x3990 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_1 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x399c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT RATE_LIMIT_OVERHEAD_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x399e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT MAC_TYPE_1 =
{
	1,
	{
		{ dump_RDD_MAC_TYPE_ENTRY, 0x399f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_DISPATCHER_CREDIT_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x39a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT US_TM_SCHEDULING_GLOBAL_FLUSH_CFG_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x39ac },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT US_TM_ETH_BBH_TX_FIFO_SIZE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x39ad },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT US_TM_DSL_BBH_TX_FIFO_SIZE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x39ae },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT US_TM_PON_BBH_TX_FIFO_SIZE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x39af },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_1 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, 0x39b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT MIRRORING_SCRATCH_1 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x39c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT US_TM_ETH_TM_FLOW_CNTR_TABLE_1 =
{
	1,
	{
		{ dump_RDD_TM_FLOW_CNTR_ENTRY, 0x39c8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT US_TM_FIRST_QUEUE_MAPPING_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x39c9 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT US_TM_FLUSH_AGGREGATION_TASK_DISABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x39ca },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DDR_PREFETCH_PD_FIFO_CNTR_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x39cb },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DDR_PREFETCH_WAKEUP_FLAG_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x39cc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x39cd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_1 =
{
	12,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, 0x39d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT US_TM_AE2P5_BBH_TX_EGRESS_COUNTER_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT US_TM_AE10_BBH_TX_EGRESS_COUNTER_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3a40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT US_TM_AE2P5_BBH_TX_ABS_COUNTER_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3a80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT US_TM_AE10_BBH_TX_ABS_COUNTER_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3ac0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DEBUG_TRACE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_TX_SCRATCHPAD_2 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT TCAM_IC_CMD_TABLE_2 =
{
	4,
	{
		{ dump_RDD_TCAM_IC_CMD, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT GLOBAL_DSCP_TO_PBITS_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xa40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_TX_DBG_CNTRS_TABLE_2 =
{
	64,
	{
		{ dump_RDD_CPU_TX_DBG_CNTRS, 0xa80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DSCP_TO_PBITS_MAP_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xb00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT TC_TO_QUEUE_TABLE_2 =
{
	8,
	{
		{ dump_RDD_TC_TO_QUEUE_8, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT IPTV_CONFIGURATION_TABLE_2 =
{
	8,
	{
		{ dump_RDD_HW_IPTV_CONFIGURATION, 0xe08 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT FPM_REPLY_2 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0xe10 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0xe20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_2 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0xe40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_TX_FPM_POOL_NUMBER_MAPPING_TABLE_2 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0xe80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT PON_TX_FLOW_TABLE_2 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, 0xec0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DSL_TX_FLOW_TABLE_2 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, 0xf00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DS_PACKET_BUFFER_2 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_2 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_2 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, 0x2294 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_2 =
{
	8,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x2298 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT VPORT_TO_DSCP_TO_PBITS_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x22a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT VPORT_TX_FLOW_TABLE_2 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, 0x22c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT QM_QUEUE_TO_TX_FLOW_TABLE_PTR_TABLE_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT PBIT_TO_QUEUE_TABLE_2 =
{
	8,
	{
		{ dump_RDD_PBIT_TO_QUEUE_8, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT PROCESSING_QUEUE_DYNAMIC_MNG_TABLE_2 =
{
	8,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x2608 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_INTERRUPT_COALESCING_TABLE_2 =
{
	16,
	{
		{ dump_RDD_CPU_INTERRUPT_COALESCING_ENTRY, 0x2610 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2620 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT PBIT_TO_GEM_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2640 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DUAL_STACK_LITE_TABLE_2 =
{
	64,
	{
		{ dump_RDD_DUAL_STACK_LITE_ENTRY, 0x2680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT GPE_COMMAND_PRIMITIVE_TABLE_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x26c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_2 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, 0x2700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT NATC_COUNTERS_GENERIC_CONTEXT_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_2 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT LAYER2_GRE_TUNNEL_TABLE_2 =
{
	32,
	{
		{ dump_RDD_LAYER2_GRE_TUNNEL_ENTRY, 0x2a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2b80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_2 =
{
	8,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, 0x2c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT TCAM_IC_CFG_TABLE_2 =
{
	16,
	{
		{ dump_RDD_TCAM_IC_CFG, 0x2d40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2dc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT RX_FLOW_CONTEXT_DDR_ADDR_2 =
{
	8,
	{
		{ dump_RDD_DEF_FLOW_CONTEXT_DDR_ADDR, 0x2de8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT SPDSVC_ANALYZER_PARAMS_TABLE_2 =
{
	16,
	{
		{ dump_RDD_SPDSVC_ANALYZER_PARAMS, 0x2df0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_SRAM_PD_FIFO_2 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x2e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DBG_DUMP_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2f00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT NATC_TBL_CFG_2 =
{
	24,
	{
		{ dump_RDD_NATC_TBL_CONFIGURATION, 0x3100 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT BROADCOM_SWITCH_PORT_TO_RX_FLOW_MAPPING_TABLE_2 =
{
	2,
	{
		{ dump_RDD_BROADCOM_SWITCH_PORT_MAPPING, 0x3160 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3172 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3173 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT TASK_IDX_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3174 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT NULL_BUFFER_2 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x3178 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT QUEUE_THRESHOLD_VECTOR_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT ENABLE_VPORT_MASK_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x31a4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_2 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0x31a8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x31b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT FLOW_BASED_ACTION_PTR_TABLE_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x31c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_2 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x31e2 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x31e4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT RATE_LIMIT_OVERHEAD_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x31e5 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT INGRESS_QOS_DONT_DROP_RATIO_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x31e6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_TX_RECYCLE_RING_ID_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x31e7 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_RX_INTERRUPT_ID_DDR_ADDR_2 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0x31e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x31f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_REDIRECT_MODE_2 =
{
	1,
	{
		{ dump_RDD_CPU_REDIRECT_MODE_ENTRY, 0x31fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x31fd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_2 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x31fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x31ff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT VLAN_ACTION_GPE_HANDLER_PTR_TABLE_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3222 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3228 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_TX_RING_DESCRIPTOR_TABLE_2 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x3250 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_TX_INGRESS_DISPATCHER_CREDIT_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3270 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT RING_CPU_TX_DESCRIPTOR_DATA_TABLE_2 =
{
	16,
	{
		{ dump_RDD_RING_CPU_TX_DESCRIPTOR, 0x3280 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_DESCRIPTOR_TABLE_2 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x32a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE_2 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0x32b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_DESCRIPTOR_TABLE_2 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x32c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_INTERRUPT_SCRATCH_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x32d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x32d8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_2 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, 0x32e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_2 =
{
	12,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, 0x32f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_TX_SYNC_FIFO_TABLE_2 =
{
	8,
	{
		{ dump_RDD_CPU_TX_SYNC_FIFO_ENTRY, 0x3300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT MULTICAST_WHITELIST_CFG_TABLE_2 =
{
	12,
	{
		{ dump_RDD_MULTICAST_WHITELIST_CFG, 0x3310 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_2 =
{
	5,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x3320 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE_2 =
{
	8,
	{
		{ dump_RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY, 0x3328 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_TX_RING_INDICES_VALUES_TABLE_2 =
{
	4,
	{
		{ dump_RDD_CPU_TX_RING_INDICES, 0x3330 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_2 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, 0x3338 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_2 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, 0x3340 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_2 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x3348 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_2 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, 0x3350 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_SHADOW_RD_IDX_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3358 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_SHADOW_WR_IDX_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3360 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_2 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, 0x3378 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT FC_FLOW_IP_ADDRESSES_TABLE_2 =
{
	48,
	{
		{ dump_RDD_FC_FLOW_IP_ADDRESSES_ENTRY, 0x3d00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DEBUG_TRACE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_STREAM_TABLE_3 =
{
	240,
	{
		{ dump_RDD_TCPSPDTEST_STREAM, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT GLOBAL_DSCP_TO_PBITS_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_RX_SCRATCHPAD_3 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT LAYER2_GRE_TUNNEL_TABLE_3 =
{
	32,
	{
		{ dump_RDD_LAYER2_GRE_TUNNEL_ENTRY, 0x600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DS_CPU_RX_METER_TABLE_3 =
{
	8,
	{
		{ dump_RDD_CPU_RX_METER_ENTRY, 0x780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT TCAM_IC_CMD_TABLE_3 =
{
	4,
	{
		{ dump_RDD_TCAM_IC_CMD, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_3 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0xa40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT US_CPU_RX_METER_TABLE_3 =
{
	8,
	{
		{ dump_RDD_CPU_RX_METER_ENTRY, 0xa80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DSCP_TO_PBITS_MAP_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xb00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT PKTGEN_TX_STREAM_TABLE_3 =
{
	136,
	{
		{ dump_RDD_PKTGEN_TX_STREAM_ENTRY, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0xe20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT PON_TX_FLOW_TABLE_3 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, 0xe40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_REASON_AND_VPORT_TO_METER_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xe80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT IPTV_CONFIGURATION_TABLE_3 =
{
	8,
	{
		{ dump_RDD_HW_IPTV_CONFIGURATION, 0xef8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DSL_TX_FLOW_TABLE_3 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, 0xf00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DS_PACKET_BUFFER_3 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_3 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT PKTGEN_NO_SBPM_HDRS_CNTR_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2294 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_3 =
{
	8,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x2298 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT VPORT_TO_DSCP_TO_PBITS_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x22a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT VPORT_TX_FLOW_TABLE_3 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, 0x22c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_RING_INTERRUPT_COUNTER_TABLE_3 =
{
	8,
	{
		{ dump_RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY, 0x2300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_INTERRUPT_COALESCING_TABLE_3 =
{
	16,
	{
		{ dump_RDD_CPU_INTERRUPT_COALESCING_ENTRY, 0x2390 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x23a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT PBIT_TO_GEM_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x23c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT TC_TO_QUEUE_TABLE_3 =
{
	8,
	{
		{ dump_RDD_TC_TO_QUEUE_8, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT PROCESSING_QUEUE_DYNAMIC_MNG_TABLE_3 =
{
	8,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x2608 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_RX_COPY_INT_SCRATCHPAD_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2610 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_RSV_TABLE_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2620 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DUAL_STACK_LITE_TABLE_3 =
{
	64,
	{
		{ dump_RDD_DUAL_STACK_LITE_ENTRY, 0x2640 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CSO_CONTEXT_TABLE_3 =
{
	108,
	{
		{ dump_RDD_CSO_CONTEXT_ENTRY, 0x2680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_3 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, 0x26ec },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_RX_PSRAM_GET_NEXT_SCRATCHPAD_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x26f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_RX_COPY_PD_FIFO_TABLE_3 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x2700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT COMMON_REPROCESSING_PD_FIFO_TABLE_3 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x2780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT GPE_COMMAND_PRIMITIVE_TABLE_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x27c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT PBIT_TO_QUEUE_TABLE_3 =
{
	8,
	{
		{ dump_RDD_PBIT_TO_QUEUE_8, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT NATC_COUNTERS_GENERIC_CONTEXT_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2a08 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT RX_FLOW_CONTEXT_DDR_ADDR_3 =
{
	8,
	{
		{ dump_RDD_DEF_FLOW_CONTEXT_DDR_ADDR, 0x2a88 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT SPDSVC_ANALYZER_PARAMS_TABLE_3 =
{
	16,
	{
		{ dump_RDD_SPDSVC_ANALYZER_PARAMS, 0x2a90 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT IPV4_HOST_ADDRESS_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2aa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DS_CPU_REASON_TO_METER_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2ac0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_3 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, 0x2b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT US_CPU_REASON_TO_METER_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2b80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT IPV6_HOST_ADDRESS_CRC_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2bc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_3 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, 0x2c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_3 =
{
	8,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, 0x2e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2f40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT SPDSVC_GEN_PARAMS_TABLE_3 =
{
	60,
	{
		{ dump_RDD_SPDSVC_GEN_PARAMS, 0x2fc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_RX_INTERRUPT_SCRATCH_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2ffc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_RING_DESCRIPTORS_TABLE_3 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x3000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT NATC_TBL_CFG_3 =
{
	24,
	{
		{ dump_RDD_NATC_TBL_CONFIGURATION, 0x3100 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT TCAM_IC_CFG_TABLE_3 =
{
	16,
	{
		{ dump_RDD_TCAM_IC_CFG, 0x3160 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT PKTGEN_FPM_UG_MGMT_3 =
{
	20,
	{
		{ dump_RDD_PKTGEN_FPM_UG_MGMT_ENTRY, 0x31e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT PKTGEN_CURR_SBPM_HDR_PTR_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x31f4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT PKTGEN_NUM_OF_AVAIL_SBPM_HDRS_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x31f6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT NULL_BUFFER_3 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x31f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_SRAM_PD_FIFO_3 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x3200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DBG_DUMP_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT PKTGEN_SBPM_HDR_BNS_3 =
{
	2,
	{
		{ dump_RDD_PKTGEN_SBPM_HDR_BN, 0x3500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT TX_ABS_RECYCLE_COUNTERS_3 =
{
	8,
	{
		{ dump_RDD_TX_ABS_RECYCLE_COUNTERS_ENTRY, 0x3538 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3540 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_3 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0x3568 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3570 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT UDPSPDTEST_STREAM_RX_STAT_TABLE_3 =
{
	48,
	{
		{ dump_RDD_UDPSPDTEST_STREAM_RX_STAT, 0x3580 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_RX_COPY_DISPATCHER_CREDIT_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x35b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT PKTGEN_SBPM_END_PTR_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x35bc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x35be },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x35bf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT QUEUE_THRESHOLD_VECTOR_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x35c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT PKTGEN_BAD_GET_NEXT_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x35e4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_RX_INTERRUPT_ID_DDR_ADDR_3 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0x35e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT SPDSVC_GEN_DISPATCHER_CREDIT_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x35f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT PKTGEN_MAX_UT_PKTS_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x35fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_VPORT_TO_METER_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3628 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT SPDSVC_TCPSPDTEST_COMMON_TABLE_3 =
{
	2,
	{
		{ dump_RDD_SPDSVC_TCPSPDTEST_COMMON_ENTRY, 0x362e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT COMMON_REPROCESSING_DISPATCHER_CREDIT_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3630 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT PKTGEN_UT_TRIGGER_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x363c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT FLOW_BASED_ACTION_PTR_TABLE_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3640 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_INTERRUPT_THRESHOLD_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3662 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT TASK_IDX_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3664 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_3 =
{
	5,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x3668 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x366d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_INTERRUPT_COUNTER_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x366e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_DESCRIPTOR_TABLE_3 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x3670 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT UDPSPDTEST_STREAM_TX_STAT_TABLE_3 =
{
	36,
	{
		{ dump_RDD_UDPSPDTEST_STREAM_TX_STAT, 0x3680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT ENABLE_VPORT_MASK_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x36a4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE_3 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0x36a8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE_3 =
{
	8,
	{
		{ dump_RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY, 0x36b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT VLAN_ACTION_GPE_HANDLER_PTR_TABLE_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x36c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_3 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x36e2 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT RATE_LIMIT_OVERHEAD_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x36e4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT INGRESS_QOS_DONT_DROP_RATIO_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x36e5 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_TX_RECYCLE_RING_ID_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x36e6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_REDIRECT_MODE_3 =
{
	1,
	{
		{ dump_RDD_CPU_REDIRECT_MODE_ENTRY, 0x36e7 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_RX_COPY_LOCAL_SCRATCH_3 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x36e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_INTERRUPT_COALESCING_TABLE_3 =
{
	16,
	{
		{ dump_RDD_CPU_INTERRUPT_COALESCING_ENTRY, 0x36f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT UPDATE_FIFO_TABLE_3 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x3700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_ENGINE_GLOBAL_TABLE_3 =
{
	20,
	{
		{ dump_RDD_TCPSPDTEST_ENGINE_GLOBAL_INFO, 0x3720 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3734 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_3 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x3735 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3736 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3737 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_3 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, 0x3738 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_RX_COPY_UPDATE_FIFO_TABLE_3 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x3740 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT BROADCOM_SWITCH_PORT_TO_RX_FLOW_MAPPING_TABLE_3 =
{
	2,
	{
		{ dump_RDD_BROADCOM_SWITCH_PORT_MAPPING, 0x3760 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_3 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, 0x3778 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT COMMON_REPROCESSING_UPDATE_FIFO_TABLE_3 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x3780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT BCM_SPDSVC_STREAM_RX_TS_TABLE_3 =
{
	12,
	{
		{ dump_RDD_SPDSVC_RX_TS_STAT, 0x37a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_DESCRIPTOR_TABLE_3 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x37b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT PD_FIFO_TABLE_3 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x37c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_REASON_TO_TC_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x37e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT TC_TO_CPU_RXQ_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3820 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT EXC_TC_TO_CPU_RXQ_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3860 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_INTERRUPT_SCRATCH_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x38a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_RX_LOCAL_SCRATCH_3 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x38a8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_3 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x38b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT PKTGEN_SESSION_DATA_3 =
{
	24,
	{
		{ dump_RDD_PKTGEN_TX_PARAMS, 0x38c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x38d8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT VPORT_TO_CPU_OBJ_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3900 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT PKTGEN_SBPM_EXTS_3 =
{
	4,
	{
		{ dump_RDD_PKTGEN_SBPM_EXT, 0x3928 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_3 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, 0x3938 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT PKTGEN_BBMSG_REPLY_SCRATCH_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3940 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_SHADOW_RD_IDX_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3948 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_3 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, 0x3950 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_3 =
{
	12,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, 0x3960 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT MULTICAST_WHITELIST_CFG_TABLE_3 =
{
	12,
	{
		{ dump_RDD_MULTICAST_WHITELIST_CFG, 0x3970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT PKTGEN_TX_STREAM_SCRATCH_TABLE_3 =
{
	8,
	{
		{ dump_RDD_PKTGEN_TX_STREAM_SCRATCH_ENTRY, 0x3980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_SHADOW_WR_IDX_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3988 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3990 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3998 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_3 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, 0x39a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x39a8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT FC_FLOW_IP_ADDRESSES_TABLE_3 =
{
	48,
	{
		{ dump_RDD_FC_FLOW_IP_ADDRESSES_ENTRY, 0x3d00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DEBUG_TRACE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DHD_FLOW_RING_CACHE_CTX_TABLE_4 =
{
	16,
	{
		{ dump_RDD_DHD_FLOW_RING_CACHE_CTX_ENTRY, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DSCP_TO_PBITS_MAP_TABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT TCAM_IC_CMD_TABLE_4 =
{
	4,
	{
		{ dump_RDD_TCAM_IC_CMD, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT GLOBAL_DSCP_TO_PBITS_TABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x640 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DHD_BACKUP_INDEX_CACHE_4 =
{
	32,
	{
		{ dump_RDD_DHD_BACKUP_IDX_CACHE_TABLE, 0x680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x6e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DSL_TX_FLOW_TABLE_4 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, 0x700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT TC_TO_QUEUE_TABLE_4 =
{
	8,
	{
		{ dump_RDD_TC_TO_QUEUE_8, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT WLAN_MCAST_SSID_STATS_TABLE_4 =
{
	8,
	{
		{ dump_RDD_WLAN_MCAST_SSID_STATS_ENTRY, 0xa08 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT IPTV_CONFIGURATION_TABLE_4 =
{
	8,
	{
		{ dump_RDD_HW_IPTV_CONFIGURATION, 0xb88 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DHD_FPM_POOL_NUMBER_MAPPING_TABLE_4 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0xb90 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT VPORT_TO_DSCP_TO_PBITS_TABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xba0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_4 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0xbc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT PBIT_TO_QUEUE_TABLE_4 =
{
	8,
	{
		{ dump_RDD_PBIT_TO_QUEUE_8, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_4 =
{
	8,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0xe08 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_4 =
{
	16,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, 0xe10 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xe20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT PON_TX_FLOW_TABLE_4 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, 0xe40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DHD_L2_HEADER_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xe80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0xec8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT SPDSVC_ANALYZER_PARAMS_TABLE_4 =
{
	16,
	{
		{ dump_RDD_SPDSVC_ANALYZER_PARAMS, 0xef0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_FLOW_RING_BUFFER_4 =
{
	48,
	{
		{ dump_RDD_DHD_TX_POST_DESCRIPTOR, 0xf00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xf90 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT BROADCOM_SWITCH_PORT_TO_RX_FLOW_MAPPING_TABLE_4 =
{
	2,
	{
		{ dump_RDD_BROADCOM_SWITCH_PORT_MAPPING, 0xfa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT SPDSVC_WLAN_TXPOST_PARAMS_TABLE_4 =
{
	2,
	{
		{ dump_RDD_SPDSVC_WLAN_TXPOST_PARAMS, 0xfb2 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_4 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, 0xfb4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT PROCESSING_QUEUE_DYNAMIC_MNG_TABLE_4 =
{
	8,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0xfb8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT VPORT_TX_FLOW_TABLE_4 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, 0xfc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DS_PACKET_BUFFER_4 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_4 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT TASK_IDX_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2294 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT RX_FLOW_CONTEXT_DDR_ADDR_4 =
{
	8,
	{
		{ dump_RDD_DEF_FLOW_CONTEXT_DDR_ADDR, 0x2298 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DHD_MCAST_DISPATCHER_CREDIT_TABLE_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x22a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT ENABLE_VPORT_MASK_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x22ac },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT NULL_BUFFER_4 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x22b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT MIRRORING_SCRATCH_4 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x22b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT PBIT_TO_GEM_TABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x22c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_4 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, 0x2300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DUAL_STACK_LITE_TABLE_4 =
{
	64,
	{
		{ dump_RDD_DUAL_STACK_LITE_ENTRY, 0x2380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT GPE_COMMAND_PRIMITIVE_TABLE_4 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x23c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_4 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT WLAN_MCAST_COPY_SCRATCHPAD_4 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT WLAN_MCAST_DHD_STATION_TABLE_4 =
{
	8,
	{
		{ dump_RDD_WLAN_MCAST_DHD_STATION_ENTRY, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DHD_POST_COMMON_RADIO_DATA_4 =
{
	136,
	{
		{ dump_RDD_DHD_POST_COMMON_RADIO_ENTRY, 0x2a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_4 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0x2b98 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_DESCRIPTOR_TABLE_4 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x2ba0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_4 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, 0x2bb0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT WLAN_MCAST_DHD_STATION_CTX_TABLE_4 =
{
	1,
	{
		{ dump_RDD_WLAN_MCAST_DHD_STATION_CTX_ENTRY, 0x2bc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT LAYER2_GRE_TUNNEL_TABLE_4 =
{
	32,
	{
		{ dump_RDD_LAYER2_GRE_TUNNEL_ENTRY, 0x2c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT NATC_COUNTERS_GENERIC_CONTEXT_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2d80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DHD_BACKUP_INFO_CACHE_TABLE_4 =
{
	8,
	{
		{ dump_RDD_DHD_BACKUP_INFO_CACHE_ENTRY, 0x2e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2f80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT WLAN_MCAST_SSID_MAC_ADDRESS_TABLE_4 =
{
	8,
	{
		{ dump_RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY, 0x3000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT WLAN_MCAST_DFT_LIST_SIZE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT WLAN_MCAST_DFT_LIST_ENTRY_SCRATCH_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x31c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_4 =
{
	8,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, 0x3200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DBG_DUMP_TABLE_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3340 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT TCAM_IC_CFG_TABLE_4 =
{
	16,
	{
		{ dump_RDD_TCAM_IC_CFG, 0x3540 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x35c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DHD_BACKUP_BASE_ADDR_4 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x35e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT WLAN_MCAST_DFT_ADDR_4 =
{
	8,
	{
		{ dump_RDD_PHYS_ADDR_64_PTR, 0x35f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_4 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x35f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x35fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x35ff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_PD_FIFO_TABLE_4 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x3600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT WLAN_MCAST_SCRATCHPAD_4 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x36c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_4 =
{
	5,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x36e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x36ed },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_4 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x36ee },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_4 =
{
	12,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, 0x36f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT RATE_LIMIT_OVERHEAD_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x36fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT INGRESS_QOS_DONT_DROP_RATIO_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x36fd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_REDIRECT_MODE_4 =
{
	1,
	{
		{ dump_RDD_CPU_REDIRECT_MODE_ENTRY, 0x36fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x36ff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT NATC_TBL_CFG_4 =
{
	24,
	{
		{ dump_RDD_NATC_TBL_CONFIGURATION, 0x3700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT MULTICAST_WHITELIST_CFG_TABLE_4 =
{
	12,
	{
		{ dump_RDD_MULTICAST_WHITELIST_CFG, 0x3760 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_4 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x376c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x376d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x376e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DHD_MIRRORING_DISPATCHER_CREDIT_TABLE_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3770 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_4 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, 0x3778 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE_4 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x3780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_4 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, 0x37b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_4 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x37b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT QUEUE_THRESHOLD_VECTOR_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x37c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_4 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, 0x37e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x37f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_4 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x37f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DHD_FLOW_RING_CACHE_LKP_TABLE_4 =
{
	2,
	{
		{ dump_RDD_DHD_FLOW_RING_CACHE_LKP_ENTRY, 0x3800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_4 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, 0x3860 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3868 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT FLOW_BASED_ACTION_PTR_TABLE_4 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3880 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT VLAN_ACTION_GPE_HANDLER_PTR_TABLE_4 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x38c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DHD_DOORBELL_TX_POST_VALUE_4 =
{
	4,
	{
		{ dump_RDD_DHD_DOORBELL, 0x38e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_UPDATE_FIFO_TABLE_4 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x3940 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DHD_MCAST_UPDATE_FIFO_TABLE_4 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x3980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DHD_MCAST_PD_FIFO_TABLE_4 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x39c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT FC_FLOW_IP_ADDRESSES_TABLE_4 =
{
	48,
	{
		{ dump_RDD_FC_FLOW_IP_ADDRESSES_ENTRY, 0x3d00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DEBUG_TRACE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT TCAM_IC_CMD_TABLE_5 =
{
	4,
	{
		{ dump_RDD_TCAM_IC_CMD, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT GLOBAL_DSCP_TO_PBITS_TABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x240 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_FLOW_RING_BUFFER_5 =
{
	32,
	{
		{ dump_RDD_DHD_RX_COMPLETE_DESCRIPTOR, 0x280 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DSCP_TO_PBITS_MAP_TABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT TC_TO_QUEUE_TABLE_5 =
{
	8,
	{
		{ dump_RDD_TC_TO_QUEUE_8, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT IPTV_CONFIGURATION_TABLE_5 =
{
	8,
	{
		{ dump_RDD_HW_IPTV_CONFIGURATION, 0x608 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_5 =
{
	16,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, 0x610 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT VPORT_TO_DSCP_TO_PBITS_TABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x620 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_5 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0x640 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DHD_RX_POST_FLOW_RING_BUFFER_5 =
{
	32,
	{
		{ dump_RDD_DHD_RX_POST_DESCRIPTOR, 0x680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x6e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DSL_TX_FLOW_TABLE_5 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, 0x700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT PBIT_TO_QUEUE_TABLE_5 =
{
	8,
	{
		{ dump_RDD_PBIT_TO_QUEUE_8, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0xa08 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT SPDSVC_ANALYZER_PARAMS_TABLE_5 =
{
	16,
	{
		{ dump_RDD_SPDSVC_ANALYZER_PARAMS, 0xa30 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT PON_TX_FLOW_TABLE_5 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, 0xa40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT VPORT_TX_FLOW_TABLE_5 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, 0xa80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT PBIT_TO_GEM_TABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xac0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_5 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, 0xb00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DUAL_STACK_LITE_TABLE_5 =
{
	64,
	{
		{ dump_RDD_DUAL_STACK_LITE_ENTRY, 0xb80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT GPE_COMMAND_PRIMITIVE_TABLE_5 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0xbc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_5 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT LAYER2_GRE_TUNNEL_TABLE_5 =
{
	32,
	{
		{ dump_RDD_LAYER2_GRE_TUNNEL_ENTRY, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT NATC_COUNTERS_GENERIC_CONTEXT_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DS_PACKET_BUFFER_5 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_5 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_5 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, 0x2294 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_5 =
{
	8,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x2298 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT BROADCOM_SWITCH_PORT_TO_RX_FLOW_MAPPING_TABLE_5 =
{
	2,
	{
		{ dump_RDD_BROADCOM_SWITCH_PORT_MAPPING, 0x22a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x22b2 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x22b3 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT TASK_IDX_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x22b4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT PROCESSING_QUEUE_DYNAMIC_MNG_TABLE_5 =
{
	8,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x22b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_FLOW_RING_BUFFER_5 =
{
	16,
	{
		{ dump_RDD_DHD_TX_COMPLETE_DESCRIPTOR, 0x22c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x22f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT NATC_TBL_CFG_5 =
{
	24,
	{
		{ dump_RDD_NATC_TBL_CONFIGURATION, 0x2300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2360 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_0_DISPATCHER_CREDIT_TABLE_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x23e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT ENABLE_VPORT_MASK_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x23ec },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_1_DISPATCHER_CREDIT_TABLE_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x23f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DHD_RX_POST_RING_SIZE_5 =
{
	2,
	{
		{ dump_RDD_DHD_RING_SIZE, 0x23fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_RING_SIZE_5 =
{
	2,
	{
		{ dump_RDD_DHD_RING_SIZE, 0x23fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DHD_COMPLETE_COMMON_RADIO_DATA_5 =
{
	112,
	{
		{ dump_RDD_DHD_COMPLETE_COMMON_RADIO_ENTRY, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_2_DISPATCHER_CREDIT_TABLE_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2550 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_RING_SIZE_5 =
{
	2,
	{
		{ dump_RDD_DHD_RING_SIZE, 0x255c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_5 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x255e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT TCAM_IC_CFG_TABLE_5 =
{
	16,
	{
		{ dump_RDD_TCAM_IC_CFG, 0x2560 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_0_DISPATCHER_CREDIT_TABLE_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x25e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x25ec },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT RATE_LIMIT_OVERHEAD_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x25ed },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT INGRESS_QOS_DONT_DROP_RATIO_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x25ee },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_REDIRECT_MODE_5 =
{
	1,
	{
		{ dump_RDD_CPU_REDIRECT_MODE_ENTRY, 0x25ef },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_1_DISPATCHER_CREDIT_TABLE_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x25f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x25fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_5 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x25fd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x25fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x25ff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_5 =
{
	8,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DBG_DUMP_TABLE_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2740 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2940 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT RX_FLOW_CONTEXT_DDR_ADDR_5 =
{
	8,
	{
		{ dump_RDD_DEF_FLOW_CONTEXT_DDR_ADDR, 0x2968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_2_DISPATCHER_CREDIT_TABLE_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE_5 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x2980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DHD_DOORBELL_TX_COMPLETE_VALUE_5 =
{
	4,
	{
		{ dump_RDD_DHD_DOORBELL, 0x29b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT QUEUE_THRESHOLD_VECTOR_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x29c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT NULL_BUFFER_5 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x29e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DHD_DOORBELL_RX_COMPLETE_VALUE_5 =
{
	4,
	{
		{ dump_RDD_DHD_DOORBELL, 0x29f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT FLOW_BASED_ACTION_PTR_TABLE_5 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_5 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0x2a28 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DHD_DOORBELL_RX_POST_VALUE_5 =
{
	4,
	{
		{ dump_RDD_DHD_DOORBELL, 0x2a30 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT VLAN_ACTION_GPE_HANDLER_PTR_TABLE_5 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2a40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_5 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2a68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT SPDSVC_WLAN_GEN_PARAMS_TABLE_5 =
{
	10,
	{
		{ dump_RDD_SPDSVC_WLAN_GEN_PARAMS, 0x2a70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DHD_FPM_REPLY_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2a80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_5 =
{
	5,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x2a98 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_DESCRIPTOR_TABLE_5 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x2aa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_5 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, 0x2ab0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_5 =
{
	12,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, 0x2ac0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT MULTICAST_WHITELIST_CFG_TABLE_5 =
{
	12,
	{
		{ dump_RDD_MULTICAST_WHITELIST_CFG, 0x2ad0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_5 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, 0x2ae0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_5 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, 0x2ae8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_5 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x2af0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_5 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, 0x2af8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_5 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2b08 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_5 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, 0x2b10 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2b18 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT FC_FLOW_IP_ADDRESSES_TABLE_5 =
{
	48,
	{
		{ dump_RDD_FC_FLOW_IP_ADDRESSES_ENTRY, 0x3d00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM63158
static DUMP_RUNNERREG_STRUCT DEBUG_TRACE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3e00 },
		{ 0, 0 },
	}
};
#endif

TABLE_STRUCT RUNNER_TABLES[NUMBER_OF_TABLES] =
{
#if defined BCM63158
	{ "BASIC_RATE_LIMITER_TABLE_DS", 1, CORE_0_INDEX, &BASIC_RATE_LIMITER_TABLE_DS, 128, 1, 1 },
#endif
#if defined BCM63158
	{ "SERVICE_QUEUES_PD_FIFO_TABLE", 1, CORE_0_INDEX, &SERVICE_QUEUES_PD_FIFO_TABLE, 64, 1, 1 },
#endif
#if defined BCM63158
	{ "COMPLEX_SCHEDULER_TABLE", 1, CORE_0_INDEX, &COMPLEX_SCHEDULER_TABLE, 16, 1, 1 },
#endif
#if defined BCM63158
	{ "DS_TM_PD_FIFO_TABLE", 1, CORE_0_INDEX, &DS_TM_PD_FIFO_TABLE, 96, 1, 1 },
#endif
#if defined BCM63158
	{ "BASIC_SCHEDULER_TABLE_DS", 1, CORE_0_INDEX, &BASIC_SCHEDULER_TABLE_DS, 32, 1, 1 },
#endif
#if defined BCM63158
	{ "DS_TM_SCHEDULING_QUEUE_TABLE", 1, CORE_0_INDEX, &DS_TM_SCHEDULING_QUEUE_TABLE, 80, 1, 1 },
#endif
#if defined BCM63158
	{ "DS_TM_TM_FLOW_CNTR_TABLE", 1, CORE_0_INDEX, &DS_TM_TM_FLOW_CNTR_TABLE, 64, 1, 1 },
#endif
#if defined BCM63158
	{ "SERVICE_QUEUES_COMPLEX_SCHEDULER_TABLE", 1, CORE_0_INDEX, &SERVICE_QUEUES_COMPLEX_SCHEDULER_TABLE, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE", 1, CORE_0_INDEX, &SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE, 32, 1, 1 },
#endif
#if defined BCM63158
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_0_INDEX, &RUNNER_PROFILING_TRACE_BUFFER, 128, 1, 1 },
#endif
#if defined BCM63158
	{ "DBG_DUMP_TABLE", 1, CORE_0_INDEX, &DBG_DUMP_TABLE, 128, 1, 1 },
#endif
#if defined BCM63158
	{ "REPORTING_QUEUE_DESCRIPTOR_TABLE", 1, CORE_0_INDEX, &REPORTING_QUEUE_DESCRIPTOR_TABLE, 80, 1, 1 },
#endif
#if defined BCM63158
	{ "REPORTING_QUEUE_ACCUMULATED_TABLE", 1, CORE_0_INDEX, &REPORTING_QUEUE_ACCUMULATED_TABLE, 16, 1, 1 },
#endif
#if defined BCM63158
	{ "REPORTING_COUNTER_TABLE", 1, CORE_0_INDEX, &REPORTING_COUNTER_TABLE, 40, 1, 1 },
#endif
#if defined BCM63158
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_0_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT, 8, 1, 1 },
#endif
#if defined BCM63158
	{ "DS_TM_TM_ACTION_PTR_TABLE", 1, CORE_0_INDEX, &DS_TM_TM_ACTION_PTR_TABLE, 17, 1, 1 },
#endif
#if defined BCM63158
	{ "BUDGET_ALLOCATION_TIMER_VALUE", 1, CORE_0_INDEX, &BUDGET_ALLOCATION_TIMER_VALUE, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "TM_HW_FLUSH", 1, CORE_0_INDEX, &TM_HW_FLUSH, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "DS_TM_BBH_TX_QUEUE_ID_TABLE", 1, CORE_0_INDEX, &DS_TM_BBH_TX_QUEUE_ID_TABLE, 2, 1, 1 },
#endif
#if defined BCM63158
	{ "DS_TM_FLUSH_DISPATCHER_CREDIT_TABLE", 1, CORE_0_INDEX, &DS_TM_FLUSH_DISPATCHER_CREDIT_TABLE, 3, 1, 1 },
#endif
#if defined BCM63158
	{ "GHOST_REPORTING_GLOBAL_CFG", 1, CORE_0_INDEX, &GHOST_REPORTING_GLOBAL_CFG, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "NATC_TBL_CFG", 1, CORE_0_INDEX, &NATC_TBL_CFG, 4, 1, 1 },
#endif
#if defined BCM63158
	{ "REGISTERS_BUFFER", 1, CORE_0_INDEX, &REGISTERS_BUFFER, 32, 1, 1 },
#endif
#if defined BCM63158
	{ "DS_TM_SCHEDULING_FLUSH_VECTOR", 1, CORE_0_INDEX, &DS_TM_SCHEDULING_FLUSH_VECTOR, 5, 1, 1 },
#endif
#if defined BCM63158
	{ "DS_TM_BB_DESTINATION_TABLE", 1, CORE_0_INDEX, &DS_TM_BB_DESTINATION_TABLE, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "SERVICE_QUEUES_BUDGET_ALLOCATION_TIMER_VALUE", 1, CORE_0_INDEX, &SERVICE_QUEUES_BUDGET_ALLOCATION_TIMER_VALUE, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_0_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "SERVICE_QUEUES_TM_ACTION_PTR_TABLE", 1, CORE_0_INDEX, &SERVICE_QUEUES_TM_ACTION_PTR_TABLE, 17, 1, 1 },
#endif
#if defined BCM63158
	{ "SCHEDULING_FLUSH_GLOBAL_CFG", 1, CORE_0_INDEX, &SCHEDULING_FLUSH_GLOBAL_CFG, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "CORE_ID_TABLE", 1, CORE_0_INDEX, &CORE_ID_TABLE, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "TASK_IDX", 1, CORE_0_INDEX, &TASK_IDX, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "ACB_PORT_STATUS_TABLE", 1, CORE_0_INDEX, &ACB_PORT_STATUS_TABLE, 8, 1, 1 },
#endif
#if defined BCM63158
	{ "SERVICE_QUEUES_DISPATCHER_CREDIT_TABLE", 1, CORE_0_INDEX, &SERVICE_QUEUES_DISPATCHER_CREDIT_TABLE, 3, 1, 1 },
#endif
#if defined BCM63158
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_0_INDEX, &TX_MIRRORING_CONFIGURATION, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "SRAM_DUMMY_STORE", 1, CORE_0_INDEX, &SRAM_DUMMY_STORE, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "RATE_LIMIT_OVERHEAD", 1, CORE_0_INDEX, &RATE_LIMIT_OVERHEAD, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "UPDATE_FIFO_TABLE", 1, CORE_0_INDEX, &UPDATE_FIFO_TABLE, 8, 1, 1 },
#endif
#if defined BCM63158
	{ "DS_TM_SCHEDULING_QUEUE_AGING_VECTOR", 1, CORE_0_INDEX, &DS_TM_SCHEDULING_QUEUE_AGING_VECTOR, 5, 1, 1 },
#endif
#if defined BCM63158
	{ "DS_TM_SCHEDULING_GLOBAL_FLUSH_CFG", 1, CORE_0_INDEX, &DS_TM_SCHEDULING_GLOBAL_FLUSH_CFG, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "DS_TM_FIRST_QUEUE_MAPPING", 1, CORE_0_INDEX, &DS_TM_FIRST_QUEUE_MAPPING, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "DS_TM_FLUSH_AGGREGATION_TASK_DISABLE", 1, CORE_0_INDEX, &DS_TM_FLUSH_AGGREGATION_TASK_DISABLE, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "SERVICE_QUEUES_FIRST_QUEUE_MAPPING", 1, CORE_0_INDEX, &SERVICE_QUEUES_FIRST_QUEUE_MAPPING, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "EGRESS_PORT_TO_BROADCOM_SWITCH_PORT_TABLE", 1, CORE_0_INDEX, &EGRESS_PORT_TO_BROADCOM_SWITCH_PORT_TABLE, 8, 1, 1 },
#endif
#if defined BCM63158
	{ "SERVICE_QUEUES_UPDATE_FIFO_TABLE", 1, CORE_0_INDEX, &SERVICE_QUEUES_UPDATE_FIFO_TABLE, 8, 1, 1 },
#endif
#if defined BCM63158
	{ "SERVICE_QUEUES_SCHEDULING_QUEUE_AGING_VECTOR", 1, CORE_0_INDEX, &SERVICE_QUEUES_SCHEDULING_QUEUE_AGING_VECTOR, 5, 1, 1 },
#endif
#if defined BCM63158
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_0_INDEX, &DEBUG_PRINT_CORE_LOCK, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "BBH_TX_INGRESS_COUNTER_TABLE", 1, CORE_0_INDEX, &BBH_TX_INGRESS_COUNTER_TABLE, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "EGRESS_PORT_TO_IMP_PORT_MAPPING_TABLE", 1, CORE_0_INDEX, &EGRESS_PORT_TO_IMP_PORT_MAPPING_TABLE, 8, 1, 1 },
#endif
#if defined BCM63158
	{ "DS_TM_SCHEDULING_AGGREGATION_CONTEXT_VECTOR", 1, CORE_0_INDEX, &DS_TM_SCHEDULING_AGGREGATION_CONTEXT_VECTOR, 5, 1, 1 },
#endif
#if defined BCM63158
	{ "DEBUG_SCRATCHPAD", 1, CORE_0_INDEX, &DEBUG_SCRATCHPAD, 10, 1, 1 },
#endif
#if defined BCM63158
	{ "GHOST_REPORTING_QUEUE_STATUS_BIT_VECTOR_TABLE", 1, CORE_0_INDEX, &GHOST_REPORTING_QUEUE_STATUS_BIT_VECTOR_TABLE, 5, 1, 1 },
#endif
#if defined BCM63158
	{ "MIRRORING_SCRATCH", 1, CORE_0_INDEX, &MIRRORING_SCRATCH, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "QUEUE_TO_REPORT_BIT_VECTOR", 1, CORE_0_INDEX, &QUEUE_TO_REPORT_BIT_VECTOR, 5, 1, 1 },
#endif
#if defined BCM63158
	{ "REPORT_BBH_TX_QUEUE_ID_TABLE", 1, CORE_0_INDEX, &REPORT_BBH_TX_QUEUE_ID_TABLE, 2, 1, 1 },
#endif
#if defined BCM63158
	{ "ACB_PACKET_SENT_PD_COPY", 1, CORE_0_INDEX, &ACB_PACKET_SENT_PD_COPY, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "SPDSVC_BBH_TX_PARAMS_TABLE", 1, CORE_0_INDEX, &SPDSVC_BBH_TX_PARAMS_TABLE, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "SERVICE_QUEUES_RATE_LIMITER_VALID_TABLE", 1, CORE_0_INDEX, &SERVICE_QUEUES_RATE_LIMITER_VALID_TABLE, 4, 1, 1 },
#endif
#if defined BCM63158
	{ "TX_MIRRORING_DISPATCHER_CREDIT_TABLE", 1, CORE_0_INDEX, &TX_MIRRORING_DISPATCHER_CREDIT_TABLE, 3, 1, 1 },
#endif
#if defined BCM63158
	{ "RATE_LIMITER_VALID_TABLE_DS", 1, CORE_0_INDEX, &RATE_LIMITER_VALID_TABLE_DS, 4, 1, 1 },
#endif
#if defined BCM63158
	{ "DEBUG_PRINT_TABLE", 1, CORE_0_INDEX, &DEBUG_PRINT_TABLE, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "FPM_GLOBAL_CFG", 1, CORE_0_INDEX, &FPM_GLOBAL_CFG, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "XGPON_REPORT_ZERO_SENT_TABLE", 1, CORE_0_INDEX, &XGPON_REPORT_ZERO_SENT_TABLE, 10, 1, 1 },
#endif
#if defined BCM63158
	{ "DS_TM_BBH_TX_EGRESS_COUNTER_TABLE", 1, CORE_0_INDEX, &DS_TM_BBH_TX_EGRESS_COUNTER_TABLE, 8, 1, 1 },
#endif
#if defined BCM63158
	{ "BBH_TX_EGRESS_REPORT_COUNTER_TABLE", 1, CORE_0_INDEX, &BBH_TX_EGRESS_REPORT_COUNTER_TABLE, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "DS_TM_BBH_QUEUE_TABLE", 1, CORE_0_INDEX, &DS_TM_BBH_QUEUE_TABLE, 8, 1, 1 },
#endif
#if defined BCM63158
	{ "DS_TM_BBH_TX_ABS_COUNTER_TABLE", 1, CORE_0_INDEX, &DS_TM_BBH_TX_ABS_COUNTER_TABLE, 8, 1, 1 },
#endif
#if defined BCM63158
	{ "DS_TM_EGRESS_PORT_RR_TABLE", 1, CORE_0_INDEX, &DS_TM_EGRESS_PORT_RR_TABLE, 3, 1, 1 },
#endif
#if defined BCM63158
	{ "SERVICE_QUEUES_RATE_LIMITER_TABLE", 1, CORE_0_INDEX, &SERVICE_QUEUES_RATE_LIMITER_TABLE, 66, 1, 1 },
#endif
#if defined BCM63158
	{ "DEBUG_TRACE", 1, CORE_0_INDEX, &DEBUG_TRACE, 512, 1, 1 },
#endif
#if defined BCM63158
	{ "DDR_PREFETCH_PINGPONG_BUFFER", 1, CORE_1_INDEX, &DDR_PREFETCH_PINGPONG_BUFFER_1, 512, 1, 1 },
#endif
#if defined BCM63158
	{ "BASIC_RATE_LIMITER_TABLE_US", 1, CORE_1_INDEX, &BASIC_RATE_LIMITER_TABLE_US_1, 128, 1, 1 },
#endif
#if defined BCM63158
	{ "DDR_PREFETCH_PD_FIFO", 1, CORE_1_INDEX, &DDR_PREFETCH_PD_FIFO_1, 64, 1, 1 },
#endif
#if defined BCM63158
	{ "COMPLEX_SCHEDULER_TABLE", 1, CORE_1_INDEX, &COMPLEX_SCHEDULER_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM63158
	{ "US_TM_PD_FIFO_TABLE", 1, CORE_1_INDEX, &US_TM_PD_FIFO_TABLE_1, 160, 1, 1 },
#endif
#if defined BCM63158
	{ "BASIC_SCHEDULER_TABLE_US", 1, CORE_1_INDEX, &BASIC_SCHEDULER_TABLE_US_1, 32, 1, 1 },
#endif
#if defined BCM63158
	{ "US_TM_SCHEDULING_QUEUE_TABLE", 1, CORE_1_INDEX, &US_TM_SCHEDULING_QUEUE_TABLE_1, 80, 1, 1 },
#endif
#if defined BCM63158
	{ "US_TM_PON_TM_FLOW_CNTR_TABLE", 1, CORE_1_INDEX, &US_TM_PON_TM_FLOW_CNTR_TABLE_1, 64, 1, 1 },
#endif
#if defined BCM63158
	{ "PFC_FRAME", 1, CORE_1_INDEX, &PFC_FRAME_1, 64, 1, 1 },
#endif
#if defined BCM63158
	{ "US_TM_DSL_TM_FLOW_CNTR_TABLE", 1, CORE_1_INDEX, &US_TM_DSL_TM_FLOW_CNTR_TABLE_1, 256, 1, 1 },
#endif
#if defined BCM63158
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_1_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_1, 128, 1, 1 },
#endif
#if defined BCM63158
	{ "DDR_PREFETCH_SBPM_FIFO", 1, CORE_1_INDEX, &DDR_PREFETCH_SBPM_FIFO_1, 32, 1, 1 },
#endif
#if defined BCM63158
	{ "DBG_DUMP_TABLE", 1, CORE_1_INDEX, &DBG_DUMP_TABLE_1, 128, 1, 1 },
#endif
#if defined BCM63158
	{ "NATC_TBL_CFG", 1, CORE_1_INDEX, &NATC_TBL_CFG_1, 4, 1, 1 },
#endif
#if defined BCM63158
	{ "REGISTERS_BUFFER", 1, CORE_1_INDEX, &REGISTERS_BUFFER_1, 32, 1, 1 },
#endif
#if defined BCM63158
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_1_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_1, 8, 1, 1 },
#endif
#if defined BCM63158
	{ "US_TM_BBH_QUEUE_TABLE", 1, CORE_1_INDEX, &US_TM_BBH_QUEUE_TABLE_1, 50, 1, 1 },
#endif
#if defined BCM63158
	{ "US_TM_AE2P5_BBH_TX_QUEUE_ID_TABLE", 1, CORE_1_INDEX, &US_TM_AE2P5_BBH_TX_QUEUE_ID_TABLE_1, 2, 1, 1 },
#endif
#if defined BCM63158
	{ "DIRECT_PROCESSING_PD_TABLE", 1, CORE_1_INDEX, &DIRECT_PROCESSING_PD_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "DIRECT_PROCESSING_FLOW_CNTR_TABLE", 1, CORE_1_INDEX, &DIRECT_PROCESSING_FLOW_CNTR_TABLE_1, 32, 1, 1 },
#endif
#if defined BCM63158
	{ "BBH_TX_EPON_INGRESS_COUNTER_TABLE", 1, CORE_1_INDEX, &BBH_TX_EPON_INGRESS_COUNTER_TABLE_1, 40, 1, 1 },
#endif
#if defined BCM63158
	{ "US_TM_AE10_BBH_TX_QUEUE_ID_TABLE", 1, CORE_1_INDEX, &US_TM_AE10_BBH_TX_QUEUE_ID_TABLE_1, 2, 1, 1 },
#endif
#if defined BCM63158
	{ "DDR_PREFETCH_COPIED_FPM_TABLE", 1, CORE_1_INDEX, &DDR_PREFETCH_COPIED_FPM_TABLE_1, 8, 1, 1 },
#endif
#if defined BCM63158
	{ "US_TM_TM_ACTION_PTR_TABLE", 1, CORE_1_INDEX, &US_TM_TM_ACTION_PTR_TABLE_1, 17, 1, 1 },
#endif
#if defined BCM63158
	{ "BUDGET_ALLOCATION_TIMER_VALUE", 1, CORE_1_INDEX, &BUDGET_ALLOCATION_TIMER_VALUE_1, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "TM_HW_FLUSH", 1, CORE_1_INDEX, &TM_HW_FLUSH_1, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "US_TM_DSL_BBH_TX_QUEUE_ID_TABLE", 1, CORE_1_INDEX, &US_TM_DSL_BBH_TX_QUEUE_ID_TABLE_1, 2, 1, 1 },
#endif
#if defined BCM63158
	{ "OVERALL_RATE_LIMITER_TABLE", 1, CORE_1_INDEX, &OVERALL_RATE_LIMITER_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "UPDATE_FIFO_TABLE", 1, CORE_1_INDEX, &UPDATE_FIFO_TABLE_1, 8, 1, 1 },
#endif
#if defined BCM63158
	{ "PFC_TX_STATUS_TABLE", 1, CORE_1_INDEX, &PFC_TX_STATUS_TABLE_1, 8, 1, 1 },
#endif
#if defined BCM63158
	{ "EPON_UPDATE_FIFO_TABLE", 1, CORE_1_INDEX, &EPON_UPDATE_FIFO_TABLE_1, 8, 1, 1 },
#endif
#if defined BCM63158
	{ "PFC_PD_TABLE", 1, CORE_1_INDEX, &PFC_PD_TABLE_1, 2, 1, 1 },
#endif
#if defined BCM63158
	{ "US_TM_PON_BBH_TX_EGRESS_COUNTER_TABLE", 1, CORE_1_INDEX, &US_TM_PON_BBH_TX_EGRESS_COUNTER_TABLE_1, 32, 1, 1 },
#endif
#if defined BCM63158
	{ "DIRECT_PROCESSING_EPON_CONTROL_SCRATCH", 1, CORE_1_INDEX, &DIRECT_PROCESSING_EPON_CONTROL_SCRATCH_1, 22, 1, 1 },
#endif
#if defined BCM63158
	{ "US_TM_BB_DESTINATION_TABLE", 1, CORE_1_INDEX, &US_TM_BB_DESTINATION_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "US_TM_PON_BBH_TX_QUEUE_ID_TABLE", 1, CORE_1_INDEX, &US_TM_PON_BBH_TX_QUEUE_ID_TABLE_1, 2, 1, 1 },
#endif
#if defined BCM63158
	{ "US_TM_PON_BBH_TX_ABS_COUNTER_TABLE", 1, CORE_1_INDEX, &US_TM_PON_BBH_TX_ABS_COUNTER_TABLE_1, 32, 1, 1 },
#endif
#if defined BCM63158
	{ "US_TM_SCHEDULING_FLUSH_VECTOR", 1, CORE_1_INDEX, &US_TM_SCHEDULING_FLUSH_VECTOR_1, 5, 1, 1 },
#endif
#if defined BCM63158
	{ "SPDTEST_GEN_PARAM", 1, CORE_1_INDEX, &SPDTEST_GEN_PARAM_1, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "US_TM_EPON_BBH_TX_QUEUE_ID_TABLE", 1, CORE_1_INDEX, &US_TM_EPON_BBH_TX_QUEUE_ID_TABLE_1, 2, 1, 1 },
#endif
#if defined BCM63158
	{ "BBH_TX_EPON_EGRESS_COUNTER_TABLE", 1, CORE_1_INDEX, &BBH_TX_EPON_EGRESS_COUNTER_TABLE_1, 32, 1, 1 },
#endif
#if defined BCM63158
	{ "US_TM_SCHEDULING_QUEUE_AGING_VECTOR", 1, CORE_1_INDEX, &US_TM_SCHEDULING_QUEUE_AGING_VECTOR_1, 5, 1, 1 },
#endif
#if defined BCM63158
	{ "TASK_IDX", 1, CORE_1_INDEX, &TASK_IDX_1, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_1_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "US_TM_SCHEDULING_AGGREGATION_CONTEXT_VECTOR", 1, CORE_1_INDEX, &US_TM_SCHEDULING_AGGREGATION_CONTEXT_VECTOR_1, 5, 1, 1 },
#endif
#if defined BCM63158
	{ "SCHEDULING_FLUSH_GLOBAL_CFG", 1, CORE_1_INDEX, &SCHEDULING_FLUSH_GLOBAL_CFG_1, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "CORE_ID_TABLE", 1, CORE_1_INDEX, &CORE_ID_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "TX_TASK_DDR_PREFETCH_FIFO_PTR", 1, CORE_1_INDEX, &TX_TASK_DDR_PREFETCH_FIFO_PTR_1, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "DEBUG_SCRATCHPAD", 1, CORE_1_INDEX, &DEBUG_SCRATCHPAD_1, 10, 1, 1 },
#endif
#if defined BCM63158
	{ "SCRATCH", 1, CORE_1_INDEX, &SCRATCH_1, 8, 1, 1 },
#endif
#if defined BCM63158
	{ "RATE_LIMITER_VALID_TABLE_US", 1, CORE_1_INDEX, &RATE_LIMITER_VALID_TABLE_US_1, 4, 1, 1 },
#endif
#if defined BCM63158
	{ "US_TM_FLUSH_DISPATCHER_CREDIT_TABLE", 1, CORE_1_INDEX, &US_TM_FLUSH_DISPATCHER_CREDIT_TABLE_1, 3, 1, 1 },
#endif
#if defined BCM63158
	{ "DDR_READ_DDR_PREFETCH_FIFO_PTR", 1, CORE_1_INDEX, &DDR_READ_DDR_PREFETCH_FIFO_PTR_1, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "PSRAM_WRITE_DDR_PREFETCH_FIFO_PTR", 1, CORE_1_INDEX, &PSRAM_WRITE_DDR_PREFETCH_FIFO_PTR_1, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "US_TM_DSL_BBH_TX_EGRESS_COUNTER_TABLE", 1, CORE_1_INDEX, &US_TM_DSL_BBH_TX_EGRESS_COUNTER_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM63158
	{ "DISPATCHER_CREDIT_TABLE", 1, CORE_1_INDEX, &DISPATCHER_CREDIT_TABLE_1, 3, 1, 1 },
#endif
#if defined BCM63158
	{ "DDR_READ_DDR_PREFETCH_PINGPONG_BUFFER_PTR", 1, CORE_1_INDEX, &DDR_READ_DDR_PREFETCH_PINGPONG_BUFFER_PTR_1, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "PSRAM_WRITE_DDR_PREFETCH_PINGPONG_BUFFER_PTR", 1, CORE_1_INDEX, &PSRAM_WRITE_DDR_PREFETCH_PINGPONG_BUFFER_PTR_1, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "WAN_TX_DDR_READ_DISPATCHER_CREDIT_TABLE", 1, CORE_1_INDEX, &WAN_TX_DDR_READ_DISPATCHER_CREDIT_TABLE_1, 3, 1, 1 },
#endif
#if defined BCM63158
	{ "WRITE_DDR_PREFETCH_SBPM_FIFO_PTR", 1, CORE_1_INDEX, &WRITE_DDR_PREFETCH_SBPM_FIFO_PTR_1, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "READ_DDR_PREFETCH_SBPM_FIFO_PTR", 1, CORE_1_INDEX, &READ_DDR_PREFETCH_SBPM_FIFO_PTR_1, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "WAN_TX_PSRAM_WRITE_DISPATCHER_CREDIT_TABLE", 1, CORE_1_INDEX, &WAN_TX_PSRAM_WRITE_DISPATCHER_CREDIT_TABLE_1, 3, 1, 1 },
#endif
#if defined BCM63158
	{ "TX_TASK_DDR_READ_DISP_BB_ID", 1, CORE_1_INDEX, &TX_TASK_DDR_READ_DISP_BB_ID_1, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "PFC_TX_ENABLE", 1, CORE_1_INDEX, &PFC_TX_ENABLE_1, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "SRAM_DUMMY_STORE", 1, CORE_1_INDEX, &SRAM_DUMMY_STORE_1, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "US_TM_DSL_BBH_TX_ABS_COUNTER_TABLE", 1, CORE_1_INDEX, &US_TM_DSL_BBH_TX_ABS_COUNTER_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM63158
	{ "SPDSVC_BBH_TX_PARAMS_TABLE", 1, CORE_1_INDEX, &SPDSVC_BBH_TX_PARAMS_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_1_INDEX, &TX_MIRRORING_CONFIGURATION_1, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "RATE_LIMIT_OVERHEAD", 1, CORE_1_INDEX, &RATE_LIMIT_OVERHEAD_1, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "MAC_TYPE", 1, CORE_1_INDEX, &MAC_TYPE_1, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "TX_MIRRORING_DISPATCHER_CREDIT_TABLE", 1, CORE_1_INDEX, &TX_MIRRORING_DISPATCHER_CREDIT_TABLE_1, 3, 1, 1 },
#endif
#if defined BCM63158
	{ "US_TM_SCHEDULING_GLOBAL_FLUSH_CFG", 1, CORE_1_INDEX, &US_TM_SCHEDULING_GLOBAL_FLUSH_CFG_1, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "US_TM_ETH_BBH_TX_FIFO_SIZE", 1, CORE_1_INDEX, &US_TM_ETH_BBH_TX_FIFO_SIZE_1, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "US_TM_DSL_BBH_TX_FIFO_SIZE", 1, CORE_1_INDEX, &US_TM_DSL_BBH_TX_FIFO_SIZE_1, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "US_TM_PON_BBH_TX_FIFO_SIZE", 1, CORE_1_INDEX, &US_TM_PON_BBH_TX_FIFO_SIZE_1, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "DEBUG_PRINT_TABLE", 1, CORE_1_INDEX, &DEBUG_PRINT_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "MIRRORING_SCRATCH", 1, CORE_1_INDEX, &MIRRORING_SCRATCH_1, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "US_TM_ETH_TM_FLOW_CNTR_TABLE", 1, CORE_1_INDEX, &US_TM_ETH_TM_FLOW_CNTR_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "US_TM_FIRST_QUEUE_MAPPING", 1, CORE_1_INDEX, &US_TM_FIRST_QUEUE_MAPPING_1, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "US_TM_FLUSH_AGGREGATION_TASK_DISABLE", 1, CORE_1_INDEX, &US_TM_FLUSH_AGGREGATION_TASK_DISABLE_1, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "DDR_PREFETCH_PD_FIFO_CNTR", 1, CORE_1_INDEX, &DDR_PREFETCH_PD_FIFO_CNTR_1, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "DDR_PREFETCH_WAKEUP_FLAG", 1, CORE_1_INDEX, &DDR_PREFETCH_WAKEUP_FLAG_1, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_1_INDEX, &DEBUG_PRINT_CORE_LOCK_1, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "FPM_GLOBAL_CFG", 1, CORE_1_INDEX, &FPM_GLOBAL_CFG_1, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "US_TM_AE2P5_BBH_TX_EGRESS_COUNTER_TABLE", 1, CORE_1_INDEX, &US_TM_AE2P5_BBH_TX_EGRESS_COUNTER_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "US_TM_AE10_BBH_TX_EGRESS_COUNTER_TABLE", 1, CORE_1_INDEX, &US_TM_AE10_BBH_TX_EGRESS_COUNTER_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "US_TM_AE2P5_BBH_TX_ABS_COUNTER_TABLE", 1, CORE_1_INDEX, &US_TM_AE2P5_BBH_TX_ABS_COUNTER_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "US_TM_AE10_BBH_TX_ABS_COUNTER_TABLE", 1, CORE_1_INDEX, &US_TM_AE10_BBH_TX_ABS_COUNTER_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "DEBUG_TRACE", 1, CORE_1_INDEX, &DEBUG_TRACE_1, 512, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_TX_SCRATCHPAD", 1, CORE_2_INDEX, &CPU_TX_SCRATCHPAD_2, 256, 1, 1 },
#endif
#if defined BCM63158
	{ "TCAM_IC_CMD_TABLE", 1, CORE_2_INDEX, &TCAM_IC_CMD_TABLE_2, 9, 16, 1 },
#endif
#if defined BCM63158
	{ "GLOBAL_DSCP_TO_PBITS_TABLE", 1, CORE_2_INDEX, &GLOBAL_DSCP_TO_PBITS_TABLE_2, 1, 64, 1 },
#endif
#if defined BCM63158
	{ "CPU_TX_DBG_CNTRS_TABLE", 1, CORE_2_INDEX, &CPU_TX_DBG_CNTRS_TABLE_2, 2, 1, 1 },
#endif
#if defined BCM63158
	{ "DSCP_TO_PBITS_MAP_TABLE", 1, CORE_2_INDEX, &DSCP_TO_PBITS_MAP_TABLE_2, 4, 64, 1 },
#endif
#if defined BCM63158
	{ "TC_TO_QUEUE_TABLE", 1, CORE_2_INDEX, &TC_TO_QUEUE_TABLE_2, 65, 1, 1 },
#endif
#if defined BCM63158
	{ "IPTV_CONFIGURATION_TABLE", 1, CORE_2_INDEX, &IPTV_CONFIGURATION_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "FPM_REPLY", 1, CORE_2_INDEX, &FPM_REPLY_2, 2, 1, 1 },
#endif
#if defined BCM63158
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_2_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_2, 8, 1, 1 },
#endif
#if defined BCM63158
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_2_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_2, 64, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_TX_FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_2_INDEX, &CPU_TX_FPM_POOL_NUMBER_MAPPING_TABLE_2, 64, 1, 1 },
#endif
#if defined BCM63158
	{ "PON_TX_FLOW_TABLE", 1, CORE_2_INDEX, &PON_TX_FLOW_TABLE_2, 64, 1, 1 },
#endif
#if defined BCM63158
	{ "DSL_TX_FLOW_TABLE", 1, CORE_2_INDEX, &DSL_TX_FLOW_TABLE_2, 256, 1, 1 },
#endif
#if defined BCM63158
	{ "DS_PACKET_BUFFER", 1, CORE_2_INDEX, &DS_PACKET_BUFFER_2, 8, 1, 1 },
#endif
#if defined BCM63158
	{ "RX_FLOW_TABLE", 1, CORE_2_INDEX, &RX_FLOW_TABLE_2, 330, 1, 1 },
#endif
#if defined BCM63158
	{ "SPDTEST_GEN_PARAM", 1, CORE_2_INDEX, &SPDTEST_GEN_PARAM_2, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_2_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "VPORT_TO_DSCP_TO_PBITS_TABLE", 1, CORE_2_INDEX, &VPORT_TO_DSCP_TO_PBITS_TABLE_2, 32, 1, 1 },
#endif
#if defined BCM63158
	{ "VPORT_TX_FLOW_TABLE", 1, CORE_2_INDEX, &VPORT_TX_FLOW_TABLE_2, 64, 1, 1 },
#endif
#if defined BCM63158
	{ "QM_QUEUE_TO_TX_FLOW_TABLE_PTR_TABLE", 1, CORE_2_INDEX, &QM_QUEUE_TO_TX_FLOW_TABLE_PTR_TABLE_2, 128, 1, 1 },
#endif
#if defined BCM63158
	{ "PBIT_TO_QUEUE_TABLE", 1, CORE_2_INDEX, &PBIT_TO_QUEUE_TABLE_2, 65, 1, 1 },
#endif
#if defined BCM63158
	{ "PROCESSING_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_2_INDEX, &PROCESSING_QUEUE_DYNAMIC_MNG_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_RECYCLE_INTERRUPT_COALESCING_TABLE", 1, CORE_2_INDEX, &CPU_RECYCLE_INTERRUPT_COALESCING_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_2_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_2, 32, 1, 1 },
#endif
#if defined BCM63158
	{ "PBIT_TO_GEM_TABLE", 1, CORE_2_INDEX, &PBIT_TO_GEM_TABLE_2, 8, 8, 1 },
#endif
#if defined BCM63158
	{ "DUAL_STACK_LITE_TABLE", 1, CORE_2_INDEX, &DUAL_STACK_LITE_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "GPE_COMMAND_PRIMITIVE_TABLE", 1, CORE_2_INDEX, &GPE_COMMAND_PRIMITIVE_TABLE_2, 32, 1, 1 },
#endif
#if defined BCM63158
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_2_INDEX, &INGRESS_FILTER_PROFILE_TABLE_2, 16, 1, 1 },
#endif
#if defined BCM63158
	{ "NATC_COUNTERS_GENERIC_CONTEXT", 1, CORE_2_INDEX, &NATC_COUNTERS_GENERIC_CONTEXT_2, 128, 1, 1 },
#endif
#if defined BCM63158
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_2_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_2, 128, 1, 1 },
#endif
#if defined BCM63158
	{ "LAYER2_GRE_TUNNEL_TABLE", 1, CORE_2_INDEX, &LAYER2_GRE_TUNNEL_TABLE_2, 12, 1, 1 },
#endif
#if defined BCM63158
	{ "REGISTERS_BUFFER", 1, CORE_2_INDEX, &REGISTERS_BUFFER_2, 32, 1, 1 },
#endif
#if defined BCM63158
	{ "VPORT_CFG_TABLE", 1, CORE_2_INDEX, &VPORT_CFG_TABLE_2, 40, 1, 1 },
#endif
#if defined BCM63158
	{ "TCAM_IC_CFG_TABLE", 1, CORE_2_INDEX, &TCAM_IC_CFG_TABLE_2, 8, 1, 1 },
#endif
#if defined BCM63158
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_2_INDEX, &LOOPBACK_QUEUE_TABLE_2, 40, 1, 1 },
#endif
#if defined BCM63158
	{ "RX_FLOW_CONTEXT_DDR_ADDR", 1, CORE_2_INDEX, &RX_FLOW_CONTEXT_DDR_ADDR_2, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "SPDSVC_ANALYZER_PARAMS_TABLE", 1, CORE_2_INDEX, &SPDSVC_ANALYZER_PARAMS_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_RECYCLE_SRAM_PD_FIFO", 1, CORE_2_INDEX, &CPU_RECYCLE_SRAM_PD_FIFO_2, 16, 1, 1 },
#endif
#if defined BCM63158
	{ "DBG_DUMP_TABLE", 1, CORE_2_INDEX, &DBG_DUMP_TABLE_2, 128, 1, 1 },
#endif
#if defined BCM63158
	{ "NATC_TBL_CFG", 1, CORE_2_INDEX, &NATC_TBL_CFG_2, 4, 1, 1 },
#endif
#if defined BCM63158
	{ "BROADCOM_SWITCH_PORT_TO_RX_FLOW_MAPPING_TABLE", 1, CORE_2_INDEX, &BROADCOM_SWITCH_PORT_TO_RX_FLOW_MAPPING_TABLE_2, 9, 1, 1 },
#endif
#if defined BCM63158
	{ "FORCE_DSCP", 1, CORE_2_INDEX, &FORCE_DSCP_2, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "CORE_ID_TABLE", 1, CORE_2_INDEX, &CORE_ID_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "TASK_IDX", 1, CORE_2_INDEX, &TASK_IDX_2, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "NULL_BUFFER", 1, CORE_2_INDEX, &NULL_BUFFER_2, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "QUEUE_THRESHOLD_VECTOR", 1, CORE_2_INDEX, &QUEUE_THRESHOLD_VECTOR_2, 9, 1, 1 },
#endif
#if defined BCM63158
	{ "ENABLE_VPORT_MASK", 1, CORE_2_INDEX, &ENABLE_VPORT_MASK_2, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR", 1, CORE_2_INDEX, &CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_2, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_2_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_2, 16, 1, 1 },
#endif
#if defined BCM63158
	{ "FLOW_BASED_ACTION_PTR_TABLE", 1, CORE_2_INDEX, &FLOW_BASED_ACTION_PTR_TABLE_2, 17, 1, 1 },
#endif
#if defined BCM63158
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_2_INDEX, &RX_MIRRORING_CONFIGURATION_2, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "SRAM_DUMMY_STORE", 1, CORE_2_INDEX, &SRAM_DUMMY_STORE_2, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "RATE_LIMIT_OVERHEAD", 1, CORE_2_INDEX, &RATE_LIMIT_OVERHEAD_2, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "INGRESS_QOS_DONT_DROP_RATIO", 1, CORE_2_INDEX, &INGRESS_QOS_DONT_DROP_RATIO_2, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_TX_RECYCLE_RING_ID", 1, CORE_2_INDEX, &CPU_TX_RECYCLE_RING_ID_2, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_RX_INTERRUPT_ID_DDR_ADDR", 1, CORE_2_INDEX, &CPU_RX_INTERRUPT_ID_DDR_ADDR_2, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE", 1, CORE_2_INDEX, &CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE_2, 3, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_REDIRECT_MODE", 1, CORE_2_INDEX, &CPU_REDIRECT_MODE_2, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_2_INDEX, &DEBUG_PRINT_CORE_LOCK_2, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_2_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_2_INDEX, &INGRESS_FILTER_1588_CFG_2, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "VLAN_ACTION_GPE_HANDLER_PTR_TABLE", 1, CORE_2_INDEX, &VLAN_ACTION_GPE_HANDLER_PTR_TABLE_2, 17, 1, 1 },
#endif
#if defined BCM63158
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_2_INDEX, &RX_MIRRORING_DIRECT_ENABLE_2, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "DEBUG_SCRATCHPAD", 1, CORE_2_INDEX, &DEBUG_SCRATCHPAD_2, 10, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_TX_RING_DESCRIPTOR_TABLE", 1, CORE_2_INDEX, &CPU_TX_RING_DESCRIPTOR_TABLE_2, 2, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_TX_INGRESS_DISPATCHER_CREDIT_TABLE", 1, CORE_2_INDEX, &CPU_TX_INGRESS_DISPATCHER_CREDIT_TABLE_2, 3, 1, 1 },
#endif
#if defined BCM63158
	{ "RING_CPU_TX_DESCRIPTOR_DATA_TABLE", 1, CORE_2_INDEX, &RING_CPU_TX_DESCRIPTOR_DATA_TABLE_2, 2, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_RECYCLE_RING_DESCRIPTOR_TABLE", 1, CORE_2_INDEX, &CPU_RECYCLE_RING_DESCRIPTOR_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE", 1, CORE_2_INDEX, &CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE_2, 2, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_FEED_RING_DESCRIPTOR_TABLE", 1, CORE_2_INDEX, &CPU_FEED_RING_DESCRIPTOR_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_RECYCLE_INTERRUPT_SCRATCH", 1, CORE_2_INDEX, &CPU_RECYCLE_INTERRUPT_SCRATCH_2, 2, 1, 1 },
#endif
#if defined BCM63158
	{ "DHD_FPM_THRESHOLDS", 1, CORE_2_INDEX, &DHD_FPM_THRESHOLDS_2, 3, 1, 1 },
#endif
#if defined BCM63158
	{ "DEBUG_PRINT_TABLE", 1, CORE_2_INDEX, &DEBUG_PRINT_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "FPM_GLOBAL_CFG", 1, CORE_2_INDEX, &FPM_GLOBAL_CFG_2, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_TX_SYNC_FIFO_TABLE", 1, CORE_2_INDEX, &CPU_TX_SYNC_FIFO_TABLE_2, 2, 1, 1 },
#endif
#if defined BCM63158
	{ "MULTICAST_WHITELIST_CFG_TABLE", 1, CORE_2_INDEX, &MULTICAST_WHITELIST_CFG_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "SYSTEM_CONFIGURATION", 1, CORE_2_INDEX, &SYSTEM_CONFIGURATION_2, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE", 1, CORE_2_INDEX, &CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_TX_RING_INDICES_VALUES_TABLE", 1, CORE_2_INDEX, &CPU_TX_RING_INDICES_VALUES_TABLE_2, 2, 1, 1 },
#endif
#if defined BCM63158
	{ "TUNNELS_PARSING_CFG", 1, CORE_2_INDEX, &TUNNELS_PARSING_CFG_2, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "IPTV_CFG_TABLE", 1, CORE_2_INDEX, &IPTV_CFG_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_2_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_2, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "NAT_CACHE_CFG", 1, CORE_2_INDEX, &NAT_CACHE_CFG_2, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_RECYCLE_SHADOW_RD_IDX", 1, CORE_2_INDEX, &CPU_RECYCLE_SHADOW_RD_IDX_2, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_RECYCLE_SHADOW_WR_IDX", 1, CORE_2_INDEX, &CPU_RECYCLE_SHADOW_WR_IDX_2, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_2_INDEX, &NAT_CACHE_KEY0_MASK_2, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_2_INDEX, &NATC_L2_VLAN_KEY_MASK_2, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "INGRESS_FILTER_CFG", 1, CORE_2_INDEX, &INGRESS_FILTER_CFG_2, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "NATC_L2_TOS_MASK", 1, CORE_2_INDEX, &NATC_L2_TOS_MASK_2, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "FC_FLOW_IP_ADDRESSES_TABLE", 1, CORE_2_INDEX, &FC_FLOW_IP_ADDRESSES_TABLE_2, 4, 1, 1 },
#endif
#if defined BCM63158
	{ "DEBUG_TRACE", 1, CORE_2_INDEX, &DEBUG_TRACE_2, 512, 1, 1 },
#endif
#if defined BCM63158
	{ "TCPSPDTEST_STREAM_TABLE", 1, CORE_3_INDEX, &TCPSPDTEST_STREAM_TABLE_3, 4, 1, 1 },
#endif
#if defined BCM63158
	{ "GLOBAL_DSCP_TO_PBITS_TABLE", 1, CORE_3_INDEX, &GLOBAL_DSCP_TO_PBITS_TABLE_3, 1, 64, 1 },
#endif
#if defined BCM63158
	{ "CPU_RX_SCRATCHPAD", 1, CORE_3_INDEX, &CPU_RX_SCRATCHPAD_3, 64, 1, 1 },
#endif
#if defined BCM63158
	{ "LAYER2_GRE_TUNNEL_TABLE", 1, CORE_3_INDEX, &LAYER2_GRE_TUNNEL_TABLE_3, 12, 1, 1 },
#endif
#if defined BCM63158
	{ "DS_CPU_RX_METER_TABLE", 1, CORE_3_INDEX, &DS_CPU_RX_METER_TABLE_3, 16, 1, 1 },
#endif
#if defined BCM63158
	{ "TCAM_IC_CMD_TABLE", 1, CORE_3_INDEX, &TCAM_IC_CMD_TABLE_3, 9, 16, 1 },
#endif
#if defined BCM63158
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_3_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_3, 64, 1, 1 },
#endif
#if defined BCM63158
	{ "US_CPU_RX_METER_TABLE", 1, CORE_3_INDEX, &US_CPU_RX_METER_TABLE_3, 16, 1, 1 },
#endif
#if defined BCM63158
	{ "DSCP_TO_PBITS_MAP_TABLE", 1, CORE_3_INDEX, &DSCP_TO_PBITS_MAP_TABLE_3, 4, 64, 1 },
#endif
#if defined BCM63158
	{ "PKTGEN_TX_STREAM_TABLE", 1, CORE_3_INDEX, &PKTGEN_TX_STREAM_TABLE_3, 4, 1, 1 },
#endif
#if defined BCM63158
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_3_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_3, 8, 1, 1 },
#endif
#if defined BCM63158
	{ "PON_TX_FLOW_TABLE", 1, CORE_3_INDEX, &PON_TX_FLOW_TABLE_3, 64, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_REASON_AND_VPORT_TO_METER_TABLE", 1, CORE_3_INDEX, &CPU_REASON_AND_VPORT_TO_METER_TABLE_3, 120, 1, 1 },
#endif
#if defined BCM63158
	{ "IPTV_CONFIGURATION_TABLE", 1, CORE_3_INDEX, &IPTV_CONFIGURATION_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "DSL_TX_FLOW_TABLE", 1, CORE_3_INDEX, &DSL_TX_FLOW_TABLE_3, 256, 1, 1 },
#endif
#if defined BCM63158
	{ "DS_PACKET_BUFFER", 1, CORE_3_INDEX, &DS_PACKET_BUFFER_3, 8, 1, 1 },
#endif
#if defined BCM63158
	{ "RX_FLOW_TABLE", 1, CORE_3_INDEX, &RX_FLOW_TABLE_3, 330, 1, 1 },
#endif
#if defined BCM63158
	{ "PKTGEN_NO_SBPM_HDRS_CNTR", 1, CORE_3_INDEX, &PKTGEN_NO_SBPM_HDRS_CNTR_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_3_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "VPORT_TO_DSCP_TO_PBITS_TABLE", 1, CORE_3_INDEX, &VPORT_TO_DSCP_TO_PBITS_TABLE_3, 32, 1, 1 },
#endif
#if defined BCM63158
	{ "VPORT_TX_FLOW_TABLE", 1, CORE_3_INDEX, &VPORT_TX_FLOW_TABLE_3, 64, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_RING_INTERRUPT_COUNTER_TABLE", 1, CORE_3_INDEX, &CPU_RING_INTERRUPT_COUNTER_TABLE_3, 18, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_RECYCLE_INTERRUPT_COALESCING_TABLE", 1, CORE_3_INDEX, &CPU_RECYCLE_INTERRUPT_COALESCING_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_3_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_3, 32, 1, 1 },
#endif
#if defined BCM63158
	{ "PBIT_TO_GEM_TABLE", 1, CORE_3_INDEX, &PBIT_TO_GEM_TABLE_3, 8, 8, 1 },
#endif
#if defined BCM63158
	{ "TC_TO_QUEUE_TABLE", 1, CORE_3_INDEX, &TC_TO_QUEUE_TABLE_3, 65, 1, 1 },
#endif
#if defined BCM63158
	{ "PROCESSING_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_3_INDEX, &PROCESSING_QUEUE_DYNAMIC_MNG_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_RX_COPY_INT_SCRATCHPAD", 1, CORE_3_INDEX, &CPU_RX_COPY_INT_SCRATCHPAD_3, 4, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_FEED_RING_RSV_TABLE", 1, CORE_3_INDEX, &CPU_FEED_RING_RSV_TABLE_3, 16, 1, 1 },
#endif
#if defined BCM63158
	{ "DUAL_STACK_LITE_TABLE", 1, CORE_3_INDEX, &DUAL_STACK_LITE_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "CSO_CONTEXT_TABLE", 1, CORE_3_INDEX, &CSO_CONTEXT_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "SPDTEST_GEN_PARAM", 1, CORE_3_INDEX, &SPDTEST_GEN_PARAM_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_RX_PSRAM_GET_NEXT_SCRATCHPAD", 1, CORE_3_INDEX, &CPU_RX_PSRAM_GET_NEXT_SCRATCHPAD_3, 4, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_RX_COPY_PD_FIFO_TABLE", 1, CORE_3_INDEX, &CPU_RX_COPY_PD_FIFO_TABLE_3, 8, 1, 1 },
#endif
#if defined BCM63158
	{ "COMMON_REPROCESSING_PD_FIFO_TABLE", 1, CORE_3_INDEX, &COMMON_REPROCESSING_PD_FIFO_TABLE_3, 4, 1, 1 },
#endif
#if defined BCM63158
	{ "GPE_COMMAND_PRIMITIVE_TABLE", 1, CORE_3_INDEX, &GPE_COMMAND_PRIMITIVE_TABLE_3, 32, 1, 1 },
#endif
#if defined BCM63158
	{ "PBIT_TO_QUEUE_TABLE", 1, CORE_3_INDEX, &PBIT_TO_QUEUE_TABLE_3, 65, 1, 1 },
#endif
#if defined BCM63158
	{ "NATC_COUNTERS_GENERIC_CONTEXT", 1, CORE_3_INDEX, &NATC_COUNTERS_GENERIC_CONTEXT_3, 128, 1, 1 },
#endif
#if defined BCM63158
	{ "RX_FLOW_CONTEXT_DDR_ADDR", 1, CORE_3_INDEX, &RX_FLOW_CONTEXT_DDR_ADDR_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "SPDSVC_ANALYZER_PARAMS_TABLE", 1, CORE_3_INDEX, &SPDSVC_ANALYZER_PARAMS_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "IPV4_HOST_ADDRESS_TABLE", 1, CORE_3_INDEX, &IPV4_HOST_ADDRESS_TABLE_3, 8, 1, 1 },
#endif
#if defined BCM63158
	{ "DS_CPU_REASON_TO_METER_TABLE", 1, CORE_3_INDEX, &DS_CPU_REASON_TO_METER_TABLE_3, 64, 1, 1 },
#endif
#if defined BCM63158
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_3_INDEX, &INGRESS_FILTER_PROFILE_TABLE_3, 16, 1, 1 },
#endif
#if defined BCM63158
	{ "US_CPU_REASON_TO_METER_TABLE", 1, CORE_3_INDEX, &US_CPU_REASON_TO_METER_TABLE_3, 64, 1, 1 },
#endif
#if defined BCM63158
	{ "IPV6_HOST_ADDRESS_CRC_TABLE", 1, CORE_3_INDEX, &IPV6_HOST_ADDRESS_CRC_TABLE_3, 16, 1, 1 },
#endif
#if defined BCM63158
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_3_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_3, 128, 1, 1 },
#endif
#if defined BCM63158
	{ "VPORT_CFG_TABLE", 1, CORE_3_INDEX, &VPORT_CFG_TABLE_3, 40, 1, 1 },
#endif
#if defined BCM63158
	{ "REGISTERS_BUFFER", 1, CORE_3_INDEX, &REGISTERS_BUFFER_3, 32, 1, 1 },
#endif
#if defined BCM63158
	{ "SPDSVC_GEN_PARAMS_TABLE", 1, CORE_3_INDEX, &SPDSVC_GEN_PARAMS_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_RX_INTERRUPT_SCRATCH", 1, CORE_3_INDEX, &CPU_RX_INTERRUPT_SCRATCH_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_RING_DESCRIPTORS_TABLE", 1, CORE_3_INDEX, &CPU_RING_DESCRIPTORS_TABLE_3, 16, 1, 1 },
#endif
#if defined BCM63158
	{ "NATC_TBL_CFG", 1, CORE_3_INDEX, &NATC_TBL_CFG_3, 4, 1, 1 },
#endif
#if defined BCM63158
	{ "TCAM_IC_CFG_TABLE", 1, CORE_3_INDEX, &TCAM_IC_CFG_TABLE_3, 8, 1, 1 },
#endif
#if defined BCM63158
	{ "PKTGEN_FPM_UG_MGMT", 1, CORE_3_INDEX, &PKTGEN_FPM_UG_MGMT_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "PKTGEN_CURR_SBPM_HDR_PTR", 1, CORE_3_INDEX, &PKTGEN_CURR_SBPM_HDR_PTR_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "PKTGEN_NUM_OF_AVAIL_SBPM_HDRS", 1, CORE_3_INDEX, &PKTGEN_NUM_OF_AVAIL_SBPM_HDRS_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "NULL_BUFFER", 1, CORE_3_INDEX, &NULL_BUFFER_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_RECYCLE_SRAM_PD_FIFO", 1, CORE_3_INDEX, &CPU_RECYCLE_SRAM_PD_FIFO_3, 16, 1, 1 },
#endif
#if defined BCM63158
	{ "DBG_DUMP_TABLE", 1, CORE_3_INDEX, &DBG_DUMP_TABLE_3, 128, 1, 1 },
#endif
#if defined BCM63158
	{ "PKTGEN_SBPM_HDR_BNS", 1, CORE_3_INDEX, &PKTGEN_SBPM_HDR_BNS_3, 28, 1, 1 },
#endif
#if defined BCM63158
	{ "TX_ABS_RECYCLE_COUNTERS", 1, CORE_3_INDEX, &TX_ABS_RECYCLE_COUNTERS_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_3_INDEX, &LOOPBACK_QUEUE_TABLE_3, 40, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR", 1, CORE_3_INDEX, &CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_3_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_3, 16, 1, 1 },
#endif
#if defined BCM63158
	{ "UDPSPDTEST_STREAM_RX_STAT_TABLE", 1, CORE_3_INDEX, &UDPSPDTEST_STREAM_RX_STAT_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_RX_COPY_DISPATCHER_CREDIT_TABLE", 1, CORE_3_INDEX, &CPU_RX_COPY_DISPATCHER_CREDIT_TABLE_3, 3, 1, 1 },
#endif
#if defined BCM63158
	{ "PKTGEN_SBPM_END_PTR", 1, CORE_3_INDEX, &PKTGEN_SBPM_END_PTR_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "FORCE_DSCP", 1, CORE_3_INDEX, &FORCE_DSCP_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "CORE_ID_TABLE", 1, CORE_3_INDEX, &CORE_ID_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "QUEUE_THRESHOLD_VECTOR", 1, CORE_3_INDEX, &QUEUE_THRESHOLD_VECTOR_3, 9, 1, 1 },
#endif
#if defined BCM63158
	{ "PKTGEN_BAD_GET_NEXT", 1, CORE_3_INDEX, &PKTGEN_BAD_GET_NEXT_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_RX_INTERRUPT_ID_DDR_ADDR", 1, CORE_3_INDEX, &CPU_RX_INTERRUPT_ID_DDR_ADDR_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "SPDSVC_GEN_DISPATCHER_CREDIT_TABLE", 1, CORE_3_INDEX, &SPDSVC_GEN_DISPATCHER_CREDIT_TABLE_3, 3, 1, 1 },
#endif
#if defined BCM63158
	{ "PKTGEN_MAX_UT_PKTS", 1, CORE_3_INDEX, &PKTGEN_MAX_UT_PKTS_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_VPORT_TO_METER_TABLE", 1, CORE_3_INDEX, &CPU_VPORT_TO_METER_TABLE_3, 40, 1, 1 },
#endif
#if defined BCM63158
	{ "DHD_FPM_THRESHOLDS", 1, CORE_3_INDEX, &DHD_FPM_THRESHOLDS_3, 3, 1, 1 },
#endif
#if defined BCM63158
	{ "SPDSVC_TCPSPDTEST_COMMON_TABLE", 1, CORE_3_INDEX, &SPDSVC_TCPSPDTEST_COMMON_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "COMMON_REPROCESSING_DISPATCHER_CREDIT_TABLE", 1, CORE_3_INDEX, &COMMON_REPROCESSING_DISPATCHER_CREDIT_TABLE_3, 3, 1, 1 },
#endif
#if defined BCM63158
	{ "PKTGEN_UT_TRIGGER", 1, CORE_3_INDEX, &PKTGEN_UT_TRIGGER_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "FLOW_BASED_ACTION_PTR_TABLE", 1, CORE_3_INDEX, &FLOW_BASED_ACTION_PTR_TABLE_3, 17, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_FEED_RING_INTERRUPT_THRESHOLD", 1, CORE_3_INDEX, &CPU_FEED_RING_INTERRUPT_THRESHOLD_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "TASK_IDX", 1, CORE_3_INDEX, &TASK_IDX_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "SYSTEM_CONFIGURATION", 1, CORE_3_INDEX, &SYSTEM_CONFIGURATION_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "SRAM_DUMMY_STORE", 1, CORE_3_INDEX, &SRAM_DUMMY_STORE_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_FEED_RING_INTERRUPT_COUNTER", 1, CORE_3_INDEX, &CPU_FEED_RING_INTERRUPT_COUNTER_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_RECYCLE_RING_DESCRIPTOR_TABLE", 1, CORE_3_INDEX, &CPU_RECYCLE_RING_DESCRIPTOR_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "UDPSPDTEST_STREAM_TX_STAT_TABLE", 1, CORE_3_INDEX, &UDPSPDTEST_STREAM_TX_STAT_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "ENABLE_VPORT_MASK", 1, CORE_3_INDEX, &ENABLE_VPORT_MASK_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE", 1, CORE_3_INDEX, &CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE_3, 2, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE", 1, CORE_3_INDEX, &CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "VLAN_ACTION_GPE_HANDLER_PTR_TABLE", 1, CORE_3_INDEX, &VLAN_ACTION_GPE_HANDLER_PTR_TABLE_3, 17, 1, 1 },
#endif
#if defined BCM63158
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_3_INDEX, &RX_MIRRORING_CONFIGURATION_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "RATE_LIMIT_OVERHEAD", 1, CORE_3_INDEX, &RATE_LIMIT_OVERHEAD_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "INGRESS_QOS_DONT_DROP_RATIO", 1, CORE_3_INDEX, &INGRESS_QOS_DONT_DROP_RATIO_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_TX_RECYCLE_RING_ID", 1, CORE_3_INDEX, &CPU_TX_RECYCLE_RING_ID_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_REDIRECT_MODE", 1, CORE_3_INDEX, &CPU_REDIRECT_MODE_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_RX_COPY_LOCAL_SCRATCH", 1, CORE_3_INDEX, &CPU_RX_COPY_LOCAL_SCRATCH_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_INTERRUPT_COALESCING_TABLE", 1, CORE_3_INDEX, &CPU_INTERRUPT_COALESCING_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "UPDATE_FIFO_TABLE", 1, CORE_3_INDEX, &UPDATE_FIFO_TABLE_3, 8, 1, 1 },
#endif
#if defined BCM63158
	{ "TCPSPDTEST_ENGINE_GLOBAL_TABLE", 1, CORE_3_INDEX, &TCPSPDTEST_ENGINE_GLOBAL_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_3_INDEX, &DEBUG_PRINT_CORE_LOCK_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_3_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_3_INDEX, &INGRESS_FILTER_1588_CFG_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_3_INDEX, &RX_MIRRORING_DIRECT_ENABLE_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "TUNNELS_PARSING_CFG", 1, CORE_3_INDEX, &TUNNELS_PARSING_CFG_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_RX_COPY_UPDATE_FIFO_TABLE", 1, CORE_3_INDEX, &CPU_RX_COPY_UPDATE_FIFO_TABLE_3, 8, 1, 1 },
#endif
#if defined BCM63158
	{ "BROADCOM_SWITCH_PORT_TO_RX_FLOW_MAPPING_TABLE", 1, CORE_3_INDEX, &BROADCOM_SWITCH_PORT_TO_RX_FLOW_MAPPING_TABLE_3, 9, 1, 1 },
#endif
#if defined BCM63158
	{ "IPTV_CFG_TABLE", 1, CORE_3_INDEX, &IPTV_CFG_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "COMMON_REPROCESSING_UPDATE_FIFO_TABLE", 1, CORE_3_INDEX, &COMMON_REPROCESSING_UPDATE_FIFO_TABLE_3, 8, 1, 1 },
#endif
#if defined BCM63158
	{ "BCM_SPDSVC_STREAM_RX_TS_TABLE", 1, CORE_3_INDEX, &BCM_SPDSVC_STREAM_RX_TS_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_FEED_RING_DESCRIPTOR_TABLE", 1, CORE_3_INDEX, &CPU_FEED_RING_DESCRIPTOR_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "PD_FIFO_TABLE", 1, CORE_3_INDEX, &PD_FIFO_TABLE_3, 2, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_REASON_TO_TC", 1, CORE_3_INDEX, &CPU_REASON_TO_TC_3, 64, 1, 1 },
#endif
#if defined BCM63158
	{ "TC_TO_CPU_RXQ", 1, CORE_3_INDEX, &TC_TO_CPU_RXQ_3, 64, 1, 1 },
#endif
#if defined BCM63158
	{ "EXC_TC_TO_CPU_RXQ", 1, CORE_3_INDEX, &EXC_TC_TO_CPU_RXQ_3, 64, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_RECYCLE_INTERRUPT_SCRATCH", 1, CORE_3_INDEX, &CPU_RECYCLE_INTERRUPT_SCRATCH_3, 2, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_RX_LOCAL_SCRATCH", 1, CORE_3_INDEX, &CPU_RX_LOCAL_SCRATCH_3, 2, 1, 1 },
#endif
#if defined BCM63158
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_3_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "PKTGEN_SESSION_DATA", 1, CORE_3_INDEX, &PKTGEN_SESSION_DATA_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "DEBUG_SCRATCHPAD", 1, CORE_3_INDEX, &DEBUG_SCRATCHPAD_3, 10, 1, 1 },
#endif
#if defined BCM63158
	{ "VPORT_TO_CPU_OBJ", 1, CORE_3_INDEX, &VPORT_TO_CPU_OBJ_3, 40, 1, 1 },
#endif
#if defined BCM63158
	{ "PKTGEN_SBPM_EXTS", 1, CORE_3_INDEX, &PKTGEN_SBPM_EXTS_3, 4, 1, 1 },
#endif
#if defined BCM63158
	{ "NAT_CACHE_CFG", 1, CORE_3_INDEX, &NAT_CACHE_CFG_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "PKTGEN_BBMSG_REPLY_SCRATCH", 1, CORE_3_INDEX, &PKTGEN_BBMSG_REPLY_SCRATCH_3, 2, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_RECYCLE_SHADOW_RD_IDX", 1, CORE_3_INDEX, &CPU_RECYCLE_SHADOW_RD_IDX_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "DEBUG_PRINT_TABLE", 1, CORE_3_INDEX, &DEBUG_PRINT_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "FPM_GLOBAL_CFG", 1, CORE_3_INDEX, &FPM_GLOBAL_CFG_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "MULTICAST_WHITELIST_CFG_TABLE", 1, CORE_3_INDEX, &MULTICAST_WHITELIST_CFG_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "PKTGEN_TX_STREAM_SCRATCH_TABLE", 1, CORE_3_INDEX, &PKTGEN_TX_STREAM_SCRATCH_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_RECYCLE_SHADOW_WR_IDX", 1, CORE_3_INDEX, &CPU_RECYCLE_SHADOW_WR_IDX_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_3_INDEX, &NAT_CACHE_KEY0_MASK_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_3_INDEX, &NATC_L2_VLAN_KEY_MASK_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "INGRESS_FILTER_CFG", 1, CORE_3_INDEX, &INGRESS_FILTER_CFG_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "NATC_L2_TOS_MASK", 1, CORE_3_INDEX, &NATC_L2_TOS_MASK_3, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "FC_FLOW_IP_ADDRESSES_TABLE", 1, CORE_3_INDEX, &FC_FLOW_IP_ADDRESSES_TABLE_3, 4, 1, 1 },
#endif
#if defined BCM63158
	{ "DEBUG_TRACE", 1, CORE_3_INDEX, &DEBUG_TRACE_3, 512, 1, 1 },
#endif
#if defined BCM63158
	{ "DHD_FLOW_RING_CACHE_CTX_TABLE", 1, CORE_4_INDEX, &DHD_FLOW_RING_CACHE_CTX_TABLE_4, 48, 1, 1 },
#endif
#if defined BCM63158
	{ "DSCP_TO_PBITS_MAP_TABLE", 1, CORE_4_INDEX, &DSCP_TO_PBITS_MAP_TABLE_4, 4, 64, 1 },
#endif
#if defined BCM63158
	{ "TCAM_IC_CMD_TABLE", 1, CORE_4_INDEX, &TCAM_IC_CMD_TABLE_4, 9, 16, 1 },
#endif
#if defined BCM63158
	{ "GLOBAL_DSCP_TO_PBITS_TABLE", 1, CORE_4_INDEX, &GLOBAL_DSCP_TO_PBITS_TABLE_4, 1, 64, 1 },
#endif
#if defined BCM63158
	{ "DHD_BACKUP_INDEX_CACHE", 1, CORE_4_INDEX, &DHD_BACKUP_INDEX_CACHE_4, 3, 1, 1 },
#endif
#if defined BCM63158
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_4_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_4, 8, 1, 1 },
#endif
#if defined BCM63158
	{ "DSL_TX_FLOW_TABLE", 1, CORE_4_INDEX, &DSL_TX_FLOW_TABLE_4, 256, 1, 1 },
#endif
#if defined BCM63158
	{ "TC_TO_QUEUE_TABLE", 1, CORE_4_INDEX, &TC_TO_QUEUE_TABLE_4, 65, 1, 1 },
#endif
#if defined BCM63158
	{ "WLAN_MCAST_SSID_STATS_TABLE", 1, CORE_4_INDEX, &WLAN_MCAST_SSID_STATS_TABLE_4, 48, 1, 1 },
#endif
#if defined BCM63158
	{ "IPTV_CONFIGURATION_TABLE", 1, CORE_4_INDEX, &IPTV_CONFIGURATION_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "DHD_FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_4_INDEX, &DHD_FPM_POOL_NUMBER_MAPPING_TABLE_4, 16, 1, 1 },
#endif
#if defined BCM63158
	{ "VPORT_TO_DSCP_TO_PBITS_TABLE", 1, CORE_4_INDEX, &VPORT_TO_DSCP_TO_PBITS_TABLE_4, 32, 1, 1 },
#endif
#if defined BCM63158
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_4_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_4, 64, 1, 1 },
#endif
#if defined BCM63158
	{ "PBIT_TO_QUEUE_TABLE", 1, CORE_4_INDEX, &PBIT_TO_QUEUE_TABLE_4, 65, 1, 1 },
#endif
#if defined BCM63158
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_4_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "DHD_HW_CFG", 1, CORE_4_INDEX, &DHD_HW_CFG_4, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_4_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_4, 32, 1, 1 },
#endif
#if defined BCM63158
	{ "PON_TX_FLOW_TABLE", 1, CORE_4_INDEX, &PON_TX_FLOW_TABLE_4, 64, 1, 1 },
#endif
#if defined BCM63158
	{ "DHD_L2_HEADER", 1, CORE_4_INDEX, &DHD_L2_HEADER_4, 72, 1, 1 },
#endif
#if defined BCM63158
	{ "DEBUG_SCRATCHPAD", 1, CORE_4_INDEX, &DEBUG_SCRATCHPAD_4, 10, 1, 1 },
#endif
#if defined BCM63158
	{ "SPDSVC_ANALYZER_PARAMS_TABLE", 1, CORE_4_INDEX, &SPDSVC_ANALYZER_PARAMS_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "DHD_TX_POST_FLOW_RING_BUFFER", 1, CORE_4_INDEX, &DHD_TX_POST_FLOW_RING_BUFFER_4, 3, 1, 1 },
#endif
#if defined BCM63158
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_4_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_4, 16, 1, 1 },
#endif
#if defined BCM63158
	{ "BROADCOM_SWITCH_PORT_TO_RX_FLOW_MAPPING_TABLE", 1, CORE_4_INDEX, &BROADCOM_SWITCH_PORT_TO_RX_FLOW_MAPPING_TABLE_4, 9, 1, 1 },
#endif
#if defined BCM63158
	{ "SPDSVC_WLAN_TXPOST_PARAMS_TABLE", 1, CORE_4_INDEX, &SPDSVC_WLAN_TXPOST_PARAMS_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "SPDTEST_GEN_PARAM", 1, CORE_4_INDEX, &SPDTEST_GEN_PARAM_4, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "PROCESSING_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_4_INDEX, &PROCESSING_QUEUE_DYNAMIC_MNG_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "VPORT_TX_FLOW_TABLE", 1, CORE_4_INDEX, &VPORT_TX_FLOW_TABLE_4, 64, 1, 1 },
#endif
#if defined BCM63158
	{ "DS_PACKET_BUFFER", 1, CORE_4_INDEX, &DS_PACKET_BUFFER_4, 8, 1, 1 },
#endif
#if defined BCM63158
	{ "RX_FLOW_TABLE", 1, CORE_4_INDEX, &RX_FLOW_TABLE_4, 330, 1, 1 },
#endif
#if defined BCM63158
	{ "TASK_IDX", 1, CORE_4_INDEX, &TASK_IDX_4, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "RX_FLOW_CONTEXT_DDR_ADDR", 1, CORE_4_INDEX, &RX_FLOW_CONTEXT_DDR_ADDR_4, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "DHD_MCAST_DISPATCHER_CREDIT_TABLE", 1, CORE_4_INDEX, &DHD_MCAST_DISPATCHER_CREDIT_TABLE_4, 3, 1, 1 },
#endif
#if defined BCM63158
	{ "ENABLE_VPORT_MASK", 1, CORE_4_INDEX, &ENABLE_VPORT_MASK_4, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "NULL_BUFFER", 1, CORE_4_INDEX, &NULL_BUFFER_4, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "MIRRORING_SCRATCH", 1, CORE_4_INDEX, &MIRRORING_SCRATCH_4, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "PBIT_TO_GEM_TABLE", 1, CORE_4_INDEX, &PBIT_TO_GEM_TABLE_4, 8, 8, 1 },
#endif
#if defined BCM63158
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_4_INDEX, &INGRESS_FILTER_PROFILE_TABLE_4, 16, 1, 1 },
#endif
#if defined BCM63158
	{ "DUAL_STACK_LITE_TABLE", 1, CORE_4_INDEX, &DUAL_STACK_LITE_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "GPE_COMMAND_PRIMITIVE_TABLE", 1, CORE_4_INDEX, &GPE_COMMAND_PRIMITIVE_TABLE_4, 32, 1, 1 },
#endif
#if defined BCM63158
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_4_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_4, 128, 1, 1 },
#endif
#if defined BCM63158
	{ "WLAN_MCAST_COPY_SCRATCHPAD", 1, CORE_4_INDEX, &WLAN_MCAST_COPY_SCRATCHPAD_4, 64, 1, 1 },
#endif
#if defined BCM63158
	{ "WLAN_MCAST_DHD_STATION_TABLE", 1, CORE_4_INDEX, &WLAN_MCAST_DHD_STATION_TABLE_4, 64, 1, 1 },
#endif
#if defined BCM63158
	{ "DHD_POST_COMMON_RADIO_DATA", 1, CORE_4_INDEX, &DHD_POST_COMMON_RADIO_DATA_4, 3, 1, 1 },
#endif
#if defined BCM63158
	{ "DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR", 1, CORE_4_INDEX, &DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_4, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_FEED_RING_DESCRIPTOR_TABLE", 1, CORE_4_INDEX, &CPU_FEED_RING_DESCRIPTOR_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "DEBUG_PRINT_TABLE", 1, CORE_4_INDEX, &DEBUG_PRINT_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "WLAN_MCAST_DHD_STATION_CTX_TABLE", 1, CORE_4_INDEX, &WLAN_MCAST_DHD_STATION_CTX_TABLE_4, 64, 1, 1 },
#endif
#if defined BCM63158
	{ "LAYER2_GRE_TUNNEL_TABLE", 1, CORE_4_INDEX, &LAYER2_GRE_TUNNEL_TABLE_4, 12, 1, 1 },
#endif
#if defined BCM63158
	{ "NATC_COUNTERS_GENERIC_CONTEXT", 1, CORE_4_INDEX, &NATC_COUNTERS_GENERIC_CONTEXT_4, 128, 1, 1 },
#endif
#if defined BCM63158
	{ "DHD_BACKUP_INFO_CACHE_TABLE", 1, CORE_4_INDEX, &DHD_BACKUP_INFO_CACHE_TABLE_4, 48, 1, 1 },
#endif
#if defined BCM63158
	{ "REGISTERS_BUFFER", 1, CORE_4_INDEX, &REGISTERS_BUFFER_4, 32, 1, 1 },
#endif
#if defined BCM63158
	{ "WLAN_MCAST_SSID_MAC_ADDRESS_TABLE", 1, CORE_4_INDEX, &WLAN_MCAST_SSID_MAC_ADDRESS_TABLE_4, 48, 1, 1 },
#endif
#if defined BCM63158
	{ "WLAN_MCAST_DFT_LIST_SIZE", 1, CORE_4_INDEX, &WLAN_MCAST_DFT_LIST_SIZE_4, 64, 1, 1 },
#endif
#if defined BCM63158
	{ "WLAN_MCAST_DFT_LIST_ENTRY_SCRATCH", 1, CORE_4_INDEX, &WLAN_MCAST_DFT_LIST_ENTRY_SCRATCH_4, 64, 1, 1 },
#endif
#if defined BCM63158
	{ "VPORT_CFG_TABLE", 1, CORE_4_INDEX, &VPORT_CFG_TABLE_4, 40, 1, 1 },
#endif
#if defined BCM63158
	{ "DBG_DUMP_TABLE", 1, CORE_4_INDEX, &DBG_DUMP_TABLE_4, 128, 1, 1 },
#endif
#if defined BCM63158
	{ "TCAM_IC_CFG_TABLE", 1, CORE_4_INDEX, &TCAM_IC_CFG_TABLE_4, 8, 1, 1 },
#endif
#if defined BCM63158
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_4_INDEX, &LOOPBACK_QUEUE_TABLE_4, 40, 1, 1 },
#endif
#if defined BCM63158
	{ "DHD_BACKUP_BASE_ADDR", 1, CORE_4_INDEX, &DHD_BACKUP_BASE_ADDR_4, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "WLAN_MCAST_DFT_ADDR", 1, CORE_4_INDEX, &WLAN_MCAST_DFT_ADDR_4, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "DHD_FPM_THRESHOLDS", 1, CORE_4_INDEX, &DHD_FPM_THRESHOLDS_4, 3, 1, 1 },
#endif
#if defined BCM63158
	{ "FORCE_DSCP", 1, CORE_4_INDEX, &FORCE_DSCP_4, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "CORE_ID_TABLE", 1, CORE_4_INDEX, &CORE_ID_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "DHD_TX_POST_PD_FIFO_TABLE", 1, CORE_4_INDEX, &DHD_TX_POST_PD_FIFO_TABLE_4, 12, 1, 1 },
#endif
#if defined BCM63158
	{ "WLAN_MCAST_SCRATCHPAD", 1, CORE_4_INDEX, &WLAN_MCAST_SCRATCHPAD_4, 5, 1, 1 },
#endif
#if defined BCM63158
	{ "SYSTEM_CONFIGURATION", 1, CORE_4_INDEX, &SYSTEM_CONFIGURATION_4, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "SRAM_DUMMY_STORE", 1, CORE_4_INDEX, &SRAM_DUMMY_STORE_4, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_4_INDEX, &RX_MIRRORING_CONFIGURATION_4, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "FPM_GLOBAL_CFG", 1, CORE_4_INDEX, &FPM_GLOBAL_CFG_4, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "RATE_LIMIT_OVERHEAD", 1, CORE_4_INDEX, &RATE_LIMIT_OVERHEAD_4, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "INGRESS_QOS_DONT_DROP_RATIO", 1, CORE_4_INDEX, &INGRESS_QOS_DONT_DROP_RATIO_4, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_REDIRECT_MODE", 1, CORE_4_INDEX, &CPU_REDIRECT_MODE_4, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_4_INDEX, &DEBUG_PRINT_CORE_LOCK_4, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "NATC_TBL_CFG", 1, CORE_4_INDEX, &NATC_TBL_CFG_4, 4, 1, 1 },
#endif
#if defined BCM63158
	{ "MULTICAST_WHITELIST_CFG_TABLE", 1, CORE_4_INDEX, &MULTICAST_WHITELIST_CFG_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_4_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_4_INDEX, &INGRESS_FILTER_1588_CFG_4, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_4_INDEX, &RX_MIRRORING_DIRECT_ENABLE_4, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "DHD_MIRRORING_DISPATCHER_CREDIT_TABLE", 1, CORE_4_INDEX, &DHD_MIRRORING_DISPATCHER_CREDIT_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "TUNNELS_PARSING_CFG", 1, CORE_4_INDEX, &TUNNELS_PARSING_CFG_4, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE", 1, CORE_4_INDEX, &DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE_4, 3, 1, 1 },
#endif
#if defined BCM63158
	{ "IPTV_CFG_TABLE", 1, CORE_4_INDEX, &IPTV_CFG_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_4_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_4, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "QUEUE_THRESHOLD_VECTOR", 1, CORE_4_INDEX, &QUEUE_THRESHOLD_VECTOR_4, 9, 1, 1 },
#endif
#if defined BCM63158
	{ "NAT_CACHE_CFG", 1, CORE_4_INDEX, &NAT_CACHE_CFG_4, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_4_INDEX, &NAT_CACHE_KEY0_MASK_4, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_4_INDEX, &NATC_L2_VLAN_KEY_MASK_4, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "DHD_FLOW_RING_CACHE_LKP_TABLE", 1, CORE_4_INDEX, &DHD_FLOW_RING_CACHE_LKP_TABLE_4, 48, 1, 1 },
#endif
#if defined BCM63158
	{ "INGRESS_FILTER_CFG", 1, CORE_4_INDEX, &INGRESS_FILTER_CFG_4, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "NATC_L2_TOS_MASK", 1, CORE_4_INDEX, &NATC_L2_TOS_MASK_4, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "FLOW_BASED_ACTION_PTR_TABLE", 1, CORE_4_INDEX, &FLOW_BASED_ACTION_PTR_TABLE_4, 17, 1, 1 },
#endif
#if defined BCM63158
	{ "VLAN_ACTION_GPE_HANDLER_PTR_TABLE", 1, CORE_4_INDEX, &VLAN_ACTION_GPE_HANDLER_PTR_TABLE_4, 17, 1, 1 },
#endif
#if defined BCM63158
	{ "DHD_DOORBELL_TX_POST_VALUE", 1, CORE_4_INDEX, &DHD_DOORBELL_TX_POST_VALUE_4, 17, 1, 1 },
#endif
#if defined BCM63158
	{ "DHD_TX_POST_UPDATE_FIFO_TABLE", 1, CORE_4_INDEX, &DHD_TX_POST_UPDATE_FIFO_TABLE_4, 8, 1, 1 },
#endif
#if defined BCM63158
	{ "DHD_MCAST_UPDATE_FIFO_TABLE", 1, CORE_4_INDEX, &DHD_MCAST_UPDATE_FIFO_TABLE_4, 8, 1, 1 },
#endif
#if defined BCM63158
	{ "DHD_MCAST_PD_FIFO_TABLE", 1, CORE_4_INDEX, &DHD_MCAST_PD_FIFO_TABLE_4, 2, 1, 1 },
#endif
#if defined BCM63158
	{ "FC_FLOW_IP_ADDRESSES_TABLE", 1, CORE_4_INDEX, &FC_FLOW_IP_ADDRESSES_TABLE_4, 4, 1, 1 },
#endif
#if defined BCM63158
	{ "DEBUG_TRACE", 1, CORE_4_INDEX, &DEBUG_TRACE_4, 512, 1, 1 },
#endif
#if defined BCM63158
	{ "TCAM_IC_CMD_TABLE", 1, CORE_5_INDEX, &TCAM_IC_CMD_TABLE_5, 9, 16, 1 },
#endif
#if defined BCM63158
	{ "GLOBAL_DSCP_TO_PBITS_TABLE", 1, CORE_5_INDEX, &GLOBAL_DSCP_TO_PBITS_TABLE_5, 1, 64, 1 },
#endif
#if defined BCM63158
	{ "DHD_RX_COMPLETE_FLOW_RING_BUFFER", 1, CORE_5_INDEX, &DHD_RX_COMPLETE_FLOW_RING_BUFFER_5, 3, 1, 1 },
#endif
#if defined BCM63158
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_5_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_5, 8, 1, 1 },
#endif
#if defined BCM63158
	{ "DSCP_TO_PBITS_MAP_TABLE", 1, CORE_5_INDEX, &DSCP_TO_PBITS_MAP_TABLE_5, 4, 64, 1 },
#endif
#if defined BCM63158
	{ "TC_TO_QUEUE_TABLE", 1, CORE_5_INDEX, &TC_TO_QUEUE_TABLE_5, 65, 1, 1 },
#endif
#if defined BCM63158
	{ "IPTV_CONFIGURATION_TABLE", 1, CORE_5_INDEX, &IPTV_CONFIGURATION_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "DHD_HW_CFG", 1, CORE_5_INDEX, &DHD_HW_CFG_5, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "VPORT_TO_DSCP_TO_PBITS_TABLE", 1, CORE_5_INDEX, &VPORT_TO_DSCP_TO_PBITS_TABLE_5, 32, 1, 1 },
#endif
#if defined BCM63158
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_5_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_5, 64, 1, 1 },
#endif
#if defined BCM63158
	{ "DHD_RX_POST_FLOW_RING_BUFFER", 1, CORE_5_INDEX, &DHD_RX_POST_FLOW_RING_BUFFER_5, 3, 1, 1 },
#endif
#if defined BCM63158
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_5_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_5, 32, 1, 1 },
#endif
#if defined BCM63158
	{ "DSL_TX_FLOW_TABLE", 1, CORE_5_INDEX, &DSL_TX_FLOW_TABLE_5, 256, 1, 1 },
#endif
#if defined BCM63158
	{ "PBIT_TO_QUEUE_TABLE", 1, CORE_5_INDEX, &PBIT_TO_QUEUE_TABLE_5, 65, 1, 1 },
#endif
#if defined BCM63158
	{ "DEBUG_SCRATCHPAD", 1, CORE_5_INDEX, &DEBUG_SCRATCHPAD_5, 10, 1, 1 },
#endif
#if defined BCM63158
	{ "SPDSVC_ANALYZER_PARAMS_TABLE", 1, CORE_5_INDEX, &SPDSVC_ANALYZER_PARAMS_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "PON_TX_FLOW_TABLE", 1, CORE_5_INDEX, &PON_TX_FLOW_TABLE_5, 64, 1, 1 },
#endif
#if defined BCM63158
	{ "VPORT_TX_FLOW_TABLE", 1, CORE_5_INDEX, &VPORT_TX_FLOW_TABLE_5, 64, 1, 1 },
#endif
#if defined BCM63158
	{ "PBIT_TO_GEM_TABLE", 1, CORE_5_INDEX, &PBIT_TO_GEM_TABLE_5, 8, 8, 1 },
#endif
#if defined BCM63158
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_5_INDEX, &INGRESS_FILTER_PROFILE_TABLE_5, 16, 1, 1 },
#endif
#if defined BCM63158
	{ "DUAL_STACK_LITE_TABLE", 1, CORE_5_INDEX, &DUAL_STACK_LITE_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "GPE_COMMAND_PRIMITIVE_TABLE", 1, CORE_5_INDEX, &GPE_COMMAND_PRIMITIVE_TABLE_5, 32, 1, 1 },
#endif
#if defined BCM63158
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_5_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_5, 128, 1, 1 },
#endif
#if defined BCM63158
	{ "LAYER2_GRE_TUNNEL_TABLE", 1, CORE_5_INDEX, &LAYER2_GRE_TUNNEL_TABLE_5, 12, 1, 1 },
#endif
#if defined BCM63158
	{ "NATC_COUNTERS_GENERIC_CONTEXT", 1, CORE_5_INDEX, &NATC_COUNTERS_GENERIC_CONTEXT_5, 128, 1, 1 },
#endif
#if defined BCM63158
	{ "DS_PACKET_BUFFER", 1, CORE_5_INDEX, &DS_PACKET_BUFFER_5, 8, 1, 1 },
#endif
#if defined BCM63158
	{ "RX_FLOW_TABLE", 1, CORE_5_INDEX, &RX_FLOW_TABLE_5, 330, 1, 1 },
#endif
#if defined BCM63158
	{ "SPDTEST_GEN_PARAM", 1, CORE_5_INDEX, &SPDTEST_GEN_PARAM_5, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_5_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "BROADCOM_SWITCH_PORT_TO_RX_FLOW_MAPPING_TABLE", 1, CORE_5_INDEX, &BROADCOM_SWITCH_PORT_TO_RX_FLOW_MAPPING_TABLE_5, 9, 1, 1 },
#endif
#if defined BCM63158
	{ "FORCE_DSCP", 1, CORE_5_INDEX, &FORCE_DSCP_5, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "CORE_ID_TABLE", 1, CORE_5_INDEX, &CORE_ID_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "TASK_IDX", 1, CORE_5_INDEX, &TASK_IDX_5, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "PROCESSING_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_5_INDEX, &PROCESSING_QUEUE_DYNAMIC_MNG_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "DHD_TX_COMPLETE_FLOW_RING_BUFFER", 1, CORE_5_INDEX, &DHD_TX_COMPLETE_FLOW_RING_BUFFER_5, 3, 1, 1 },
#endif
#if defined BCM63158
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_5_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_5, 16, 1, 1 },
#endif
#if defined BCM63158
	{ "NATC_TBL_CFG", 1, CORE_5_INDEX, &NATC_TBL_CFG_5, 4, 1, 1 },
#endif
#if defined BCM63158
	{ "REGISTERS_BUFFER", 1, CORE_5_INDEX, &REGISTERS_BUFFER_5, 32, 1, 1 },
#endif
#if defined BCM63158
	{ "DHD_RX_COMPLETE_0_DISPATCHER_CREDIT_TABLE", 1, CORE_5_INDEX, &DHD_RX_COMPLETE_0_DISPATCHER_CREDIT_TABLE_5, 3, 1, 1 },
#endif
#if defined BCM63158
	{ "ENABLE_VPORT_MASK", 1, CORE_5_INDEX, &ENABLE_VPORT_MASK_5, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "DHD_RX_COMPLETE_1_DISPATCHER_CREDIT_TABLE", 1, CORE_5_INDEX, &DHD_RX_COMPLETE_1_DISPATCHER_CREDIT_TABLE_5, 3, 1, 1 },
#endif
#if defined BCM63158
	{ "DHD_RX_POST_RING_SIZE", 1, CORE_5_INDEX, &DHD_RX_POST_RING_SIZE_5, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "DHD_RX_COMPLETE_RING_SIZE", 1, CORE_5_INDEX, &DHD_RX_COMPLETE_RING_SIZE_5, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "DHD_COMPLETE_COMMON_RADIO_DATA", 1, CORE_5_INDEX, &DHD_COMPLETE_COMMON_RADIO_DATA_5, 3, 1, 1 },
#endif
#if defined BCM63158
	{ "DHD_RX_COMPLETE_2_DISPATCHER_CREDIT_TABLE", 1, CORE_5_INDEX, &DHD_RX_COMPLETE_2_DISPATCHER_CREDIT_TABLE_5, 3, 1, 1 },
#endif
#if defined BCM63158
	{ "DHD_TX_COMPLETE_RING_SIZE", 1, CORE_5_INDEX, &DHD_TX_COMPLETE_RING_SIZE_5, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_5_INDEX, &RX_MIRRORING_CONFIGURATION_5, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "TCAM_IC_CFG_TABLE", 1, CORE_5_INDEX, &TCAM_IC_CFG_TABLE_5, 8, 1, 1 },
#endif
#if defined BCM63158
	{ "DHD_TX_COMPLETE_0_DISPATCHER_CREDIT_TABLE", 1, CORE_5_INDEX, &DHD_TX_COMPLETE_0_DISPATCHER_CREDIT_TABLE_5, 3, 1, 1 },
#endif
#if defined BCM63158
	{ "SRAM_DUMMY_STORE", 1, CORE_5_INDEX, &SRAM_DUMMY_STORE_5, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "RATE_LIMIT_OVERHEAD", 1, CORE_5_INDEX, &RATE_LIMIT_OVERHEAD_5, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "INGRESS_QOS_DONT_DROP_RATIO", 1, CORE_5_INDEX, &INGRESS_QOS_DONT_DROP_RATIO_5, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_REDIRECT_MODE", 1, CORE_5_INDEX, &CPU_REDIRECT_MODE_5, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "DHD_TX_COMPLETE_1_DISPATCHER_CREDIT_TABLE", 1, CORE_5_INDEX, &DHD_TX_COMPLETE_1_DISPATCHER_CREDIT_TABLE_5, 3, 1, 1 },
#endif
#if defined BCM63158
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_5_INDEX, &DEBUG_PRINT_CORE_LOCK_5, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_5_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_5_INDEX, &INGRESS_FILTER_1588_CFG_5, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_5_INDEX, &RX_MIRRORING_DIRECT_ENABLE_5, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "VPORT_CFG_TABLE", 1, CORE_5_INDEX, &VPORT_CFG_TABLE_5, 40, 1, 1 },
#endif
#if defined BCM63158
	{ "DBG_DUMP_TABLE", 1, CORE_5_INDEX, &DBG_DUMP_TABLE_5, 128, 1, 1 },
#endif
#if defined BCM63158
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_5_INDEX, &LOOPBACK_QUEUE_TABLE_5, 40, 1, 1 },
#endif
#if defined BCM63158
	{ "RX_FLOW_CONTEXT_DDR_ADDR", 1, CORE_5_INDEX, &RX_FLOW_CONTEXT_DDR_ADDR_5, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "DHD_TX_COMPLETE_2_DISPATCHER_CREDIT_TABLE", 1, CORE_5_INDEX, &DHD_TX_COMPLETE_2_DISPATCHER_CREDIT_TABLE_5, 3, 1, 1 },
#endif
#if defined BCM63158
	{ "DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE", 1, CORE_5_INDEX, &DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE_5, 3, 1, 1 },
#endif
#if defined BCM63158
	{ "DHD_DOORBELL_TX_COMPLETE_VALUE", 1, CORE_5_INDEX, &DHD_DOORBELL_TX_COMPLETE_VALUE_5, 3, 1, 1 },
#endif
#if defined BCM63158
	{ "QUEUE_THRESHOLD_VECTOR", 1, CORE_5_INDEX, &QUEUE_THRESHOLD_VECTOR_5, 9, 1, 1 },
#endif
#if defined BCM63158
	{ "NULL_BUFFER", 1, CORE_5_INDEX, &NULL_BUFFER_5, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "DHD_DOORBELL_RX_COMPLETE_VALUE", 1, CORE_5_INDEX, &DHD_DOORBELL_RX_COMPLETE_VALUE_5, 3, 1, 1 },
#endif
#if defined BCM63158
	{ "FLOW_BASED_ACTION_PTR_TABLE", 1, CORE_5_INDEX, &FLOW_BASED_ACTION_PTR_TABLE_5, 17, 1, 1 },
#endif
#if defined BCM63158
	{ "DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR", 1, CORE_5_INDEX, &DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_5, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "DHD_DOORBELL_RX_POST_VALUE", 1, CORE_5_INDEX, &DHD_DOORBELL_RX_POST_VALUE_5, 3, 1, 1 },
#endif
#if defined BCM63158
	{ "VLAN_ACTION_GPE_HANDLER_PTR_TABLE", 1, CORE_5_INDEX, &VLAN_ACTION_GPE_HANDLER_PTR_TABLE_5, 17, 1, 1 },
#endif
#if defined BCM63158
	{ "DHD_FPM_THRESHOLDS", 1, CORE_5_INDEX, &DHD_FPM_THRESHOLDS_5, 3, 1, 1 },
#endif
#if defined BCM63158
	{ "SPDSVC_WLAN_GEN_PARAMS_TABLE", 1, CORE_5_INDEX, &SPDSVC_WLAN_GEN_PARAMS_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "DHD_FPM_REPLY", 1, CORE_5_INDEX, &DHD_FPM_REPLY_5, 24, 1, 1 },
#endif
#if defined BCM63158
	{ "SYSTEM_CONFIGURATION", 1, CORE_5_INDEX, &SYSTEM_CONFIGURATION_5, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "CPU_FEED_RING_DESCRIPTOR_TABLE", 1, CORE_5_INDEX, &CPU_FEED_RING_DESCRIPTOR_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "DEBUG_PRINT_TABLE", 1, CORE_5_INDEX, &DEBUG_PRINT_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "FPM_GLOBAL_CFG", 1, CORE_5_INDEX, &FPM_GLOBAL_CFG_5, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "MULTICAST_WHITELIST_CFG_TABLE", 1, CORE_5_INDEX, &MULTICAST_WHITELIST_CFG_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "TUNNELS_PARSING_CFG", 1, CORE_5_INDEX, &TUNNELS_PARSING_CFG_5, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "IPTV_CFG_TABLE", 1, CORE_5_INDEX, &IPTV_CFG_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_5_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_5, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "NAT_CACHE_CFG", 1, CORE_5_INDEX, &NAT_CACHE_CFG_5, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_5_INDEX, &NAT_CACHE_KEY0_MASK_5, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_5_INDEX, &NATC_L2_VLAN_KEY_MASK_5, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "INGRESS_FILTER_CFG", 1, CORE_5_INDEX, &INGRESS_FILTER_CFG_5, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "NATC_L2_TOS_MASK", 1, CORE_5_INDEX, &NATC_L2_TOS_MASK_5, 1, 1, 1 },
#endif
#if defined BCM63158
	{ "FC_FLOW_IP_ADDRESSES_TABLE", 1, CORE_5_INDEX, &FC_FLOW_IP_ADDRESSES_TABLE_5, 4, 1, 1 },
#endif
#if defined BCM63158
	{ "DEBUG_TRACE", 1, CORE_5_INDEX, &DEBUG_TRACE_5, 512, 1, 1 },
#endif
};
