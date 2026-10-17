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
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_COMPLETE_COMMON_RADIO_DATA =
{
	320,
	{
		{ dump_RDD_DHD_COMPLETE_COMMON_RADIO_ENTRY, set_RDD_DHD_COMPLETE_COMMON_RADIO_ENTRY, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, set_RDD_RX_FLOW_ENTRY, 0x500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, set_RDD_POLICER_PARAMS_ENTRY, 0x600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xa00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SCORE_CFG_TABLE =
{
	8,
	{
		{ dump_RDD_SCORE_CFG_ENTRY, set_RDD_SCORE_CFG_ENTRY, 0xb68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, set_RDD_SPDTEST_GEN_CFG, 0xb70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SCORE_DATA_COLLECTION_PROFILE_TABLE =
{
	16,
	{
		{ dump_RDD_SCORE_DATA_COLLECTION_PROFILE, set_RDD_SCORE_DATA_COLLECTION_PROFILE, 0xb80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0xd68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_DOORBELL_TX_COMPLETE_VALUE =
{
	4,
	{
		{ dump_RDD_DHD_DOORBELL, set_RDD_DHD_DOORBELL, 0xd70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_FLOW_RING_BUFFER =
{
	32,
	{
		{ dump_RDD_DHD_RX_COMPLETE_DESCRIPTOR, set_RDD_DHD_RX_COMPLETE_DESCRIPTOR, 0xd80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, set_RDD_DDR_ADDRESS, 0xf68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_DOORBELL_RX_COMPLETE_VALUE =
{
	4,
	{
		{ dump_RDD_DHD_DOORBELL, set_RDD_DHD_DOORBELL, 0xf70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_RX_POST_FLOW_RING_BUFFER =
{
	32,
	{
		{ dump_RDD_DHD_RX_POST_DESCRIPTOR, set_RDD_DHD_RX_POST_DESCRIPTOR, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, set_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_RX_POST_RING_SIZE =
{
	2,
	{
		{ dump_RDD_DHD_RING_SIZE, set_RDD_DHD_RING_SIZE, 0x2168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_DOORBELL_RX_POST_VALUE =
{
	4,
	{
		{ dump_RDD_DHD_DOORBELL, set_RDD_DHD_DOORBELL, 0x2170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_0_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_RING_SIZE =
{
	2,
	{
		{ dump_RDD_DHD_RING_SIZE, set_RDD_DHD_RING_SIZE, 0x2368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_CPU_INT_ID =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_1_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_RING_SIZE =
{
	2,
	{
		{ dump_RDD_DHD_RING_SIZE, set_RDD_DHD_RING_SIZE, 0x2568 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_PCAP_CFG =
{
	4,
	{
		{ dump_RDD_DHD_PCAP_CONFIG, set_RDD_DHD_PCAP_CONFIG, 0x2570 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_2_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2580 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SCORE_COLLECT_DROP_BITMAP_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x276c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2770 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, set_RDD_VPORT_CFG_ENTRY, 0x2780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x27fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, set_RDD_TUNNELS_PARSING_CFG, 0x29e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x29f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CSO_BAD_IPV4_HDR_CSUM_PACKETS =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x29fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_MONITOR_PKT_INFO_TABLE =
{
	56,
	{
		{ dump_RDD_DHD_MONITOR_PKT_INFO, set_RDD_DHD_MONITOR_PKT_INFO, 0x2a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2ae0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0x2b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TASK_IDX =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2bd4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_MIRRORING_MINIFPM_PARAMS =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2bd8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0x2be0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, set_RDD_INGRESS_FILTER_CTRL, 0x2c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_PCAP_TX_RING_DESCRIPTOR_TABLE =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, set_RDD_CPU_RING_DESCRIPTOR, 0x2c80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_DISPATCHER_CREDIT_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2cc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_PCAP_RX_RING_DESCRIPTOR_TABLE =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, set_RDD_CPU_RING_DESCRIPTOR, 0x2d00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_DISPATCHER_CREDIT_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2d40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, set_RDD_CPU_RING_DESCRIPTOR, 0x2d80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_0_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2dc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_FLOW_RING_BUFFER =
{
	16,
	{
		{ dump_RDD_DHD_TX_COMPLETE_DESCRIPTOR, set_RDD_DHD_TX_COMPLETE_DESCRIPTOR, 0x2e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_1_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2e40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_2_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2e80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_TABLE =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, set_RDD_MIRRORING_TRUNCATE_ENTRY, 0x2ec0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2efe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT HOST_TX_TRUNCATE_MIRRORING_TABLE =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, set_RDD_MIRRORING_TRUNCATE_ENTRY, 0x2f00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2f3e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2f40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2f7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2f7f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, set_RDD_VPORT_CFG_EX_ENTRY, 0x2f80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FORCE_DSCP =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2fbe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2fbf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_TABLE =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, set_RDD_TR471_SPDSVC_RX_PKT_ID, 0x2fc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2ff0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT LLQ_SELECTOR_ECN_MASK =
{
	1,
	{
		{ dump_RDD_LLQ_SELECTOR_ECN, set_RDD_LLQ_SELECTOR_ECN, 0x2ffc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2ffd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x2ffe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, set_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0x3000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, set_RDD_BUFMNG_STATUS_ENTRY, 0x3020 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_FPM_REPLY =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3040 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, set_RDD_DHD_HW_CONFIGURATION, 0x3060 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x3074 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3076 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CSO_DISABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3077 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, set_RDD_IPTV_CFG, 0x3078 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0x3080 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, set_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x309f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0x30a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x30bc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x30bd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x30c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x30e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SPDSVC_WLAN_GEN_PARAMS_TABLE =
{
	10,
	{
		{ dump_RDD_SPDSVC_WLAN_GEN_PARAMS, set_RDD_SPDSVC_WLAN_GEN_PARAMS, 0x3110 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0x3120 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, set_RDD_TCAM_GENERIC, 0x3130 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3140 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, set_RDD_GDX_PARAMS, 0x3150 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3160 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, set_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x3168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, set_RDD_NAT_CACHE_CFG, 0x3170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3178 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, set_RDD_MULTICAST_KEY_MASK_ENTRY, 0x3180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, set_RDD_TCAM_TABLE_CFG, 0x3188 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x3190 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, set_RDD_INGRESS_FILTER_CFG, 0x3198 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x31a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_FLOW_RING_CACHE_CTX_TABLE_1 =
{
	16,
	{
		{ dump_RDD_DHD_FLOW_RING_CACHE_CTX_ENTRY, set_RDD_DHD_FLOW_RING_CACHE_CTX_ENTRY, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_AUX_INFO_CACHE_TABLE_1 =
{
	16,
	{
		{ dump_RDD_DHD_AUX_INFO_CACHE_ENTRY, set_RDD_DHD_AUX_INFO_CACHE_ENTRY, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_POST_COMMON_RADIO_DATA_1 =
{
	200,
	{
		{ dump_RDD_DHD_POST_COMMON_RADIO_ENTRY, set_RDD_DHD_POST_COMMON_RADIO_ENTRY, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xb20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_TABLE_1 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, set_RDD_MIRRORING_TRUNCATE_ENTRY, 0xb40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0xb7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SCORE_DATA_COLLECTION_PROFILE_TABLE_1 =
{
	16,
	{
		{ dump_RDD_SCORE_DATA_COLLECTION_PROFILE, set_RDD_SCORE_DATA_COLLECTION_PROFILE, 0xb80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_1 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, set_RDD_POLICER_PARAMS_ENTRY, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_DSTAT_CFG_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_MONITOR_BQ_INFO_TABLE_1 =
{
	32,
	{
		{ dump_RDD_DHD_MONITOR_BQ_INFO, set_RDD_DHD_MONITOR_BQ_INFO, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER_1 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, set_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_1 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SCORE_CFG_TABLE_1 =
{
	8,
	{
		{ dump_RDD_SCORE_CFG_ENTRY, set_RDD_SCORE_CFG_ENTRY, 0x2368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_1 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, set_RDD_SPDTEST_GEN_CFG, 0x2370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_BACKUP_INDEX_CACHE_1 =
{
	32,
	{
		{ dump_RDD_DHD_BACKUP_IDX_CACHE_TABLE, set_RDD_DHD_BACKUP_IDX_CACHE_TABLE, 0x2380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_1 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x2568 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_CPU_INT_ID_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2570 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_1 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, set_RDD_VPORT_CFG_ENTRY, 0x2580 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SCORE_COLLECT_DROP_BITMAP_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x25fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT MIRRORING_SCRATCH_1 =
{
	8,
	{
		{ dump_RDD_BYTES_8, set_RDD_BYTES_8, 0x2768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_MIRRORING_DISPATCHER_CREDIT_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2770 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_L2_HEADER_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_1 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0x27e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_1 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, set_RDD_DDR_ADDRESS, 0x29e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x29f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_1 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, set_RDD_TUNNELS_PARSING_CFG, 0x2b68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_1 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x2b70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2b7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_CPU_TX_POST_RING_DESCRIPTOR_TABLE_1 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, set_RDD_CPU_RING_DESCRIPTOR, 0x2b80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT HOST_TX_TRUNCATE_MIRRORING_TABLE_1 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, set_RDD_MIRRORING_TRUNCATE_ENTRY, 0x2bc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2bfe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_MIRRORING_MINIFPM_PARAMS_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2d68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2d70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2d7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE_1 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, set_RDD_CPU_RING_DESCRIPTOR, 0x2d80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2dc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SPDSVC_WLAN_TXPOST_PARAMS_TABLE_1 =
{
	2,
	{
		{ dump_RDD_SPDSVC_WLAN_TXPOST_PARAMS, set_RDD_SPDSVC_WLAN_TXPOST_PARAMS, 0x2dfe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CSO_BAD_IPV4_HDR_CSUM_PACKETS_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2f68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TASK_IDX_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2f6c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_1 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0x2f70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_1 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, set_RDD_VPORT_CFG_EX_ENTRY, 0x2f80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2fbe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2fbf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_TABLE_1 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, set_RDD_TR471_SPDSVC_RX_PKT_ID, 0x2fc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_1 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, set_RDD_TCAM_GENERIC, 0x2ff0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_1 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, set_RDD_IPTV_CFG, 0x3168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_1 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, set_RDD_GDX_PARAMS, 0x3170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x317c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x317d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT LLQ_SELECTOR_ECN_MASK_1 =
{
	1,
	{
		{ dump_RDD_LLQ_SELECTOR_ECN, set_RDD_LLQ_SELECTOR_ECN, 0x317e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x317f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_1 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, set_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0x3180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_1 =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, set_RDD_BUFMNG_STATUS_ENTRY, 0x31a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_UPDATE_FIFO_TABLE_1 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, set_RDD_UPDATE_FIFO_ENTRY, 0x31c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_TIMER_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x31e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_PD_FIFO_TABLE_1 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, set_RDD_PROCESSING_TX_DESCRIPTOR, 0x3200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_1 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, set_RDD_RX_FLOW_ENTRY, 0x3300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_DOORBELL_TX_POST_VALUE_1 =
{
	4,
	{
		{ dump_RDD_DHD_DOORBELL, set_RDD_DHD_DOORBELL, 0x3400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_1 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0x3500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_1 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x35d4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_1 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x35d6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_1 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, set_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x35d8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_UPDATE_FIFO_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x35e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_FLOW_RING_BUFFER_1 =
{
	48,
	{
		{ dump_RDD_DHD_TX_POST_DESCRIPTOR, set_RDD_DHD_TX_POST_DESCRIPTOR, 0x3600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_1 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0x36c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x36df },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_1 =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, set_RDD_DHD_HW_CONFIGURATION, 0x36e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CSO_DISABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x36f4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_1 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, set_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x36f5 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x36f6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x36f7 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_1 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, set_RDD_NAT_CACHE_CFG, 0x36f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_0_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x37a8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_1 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, set_RDD_TCAM_TABLE_CFG, 0x37b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x37b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x37c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_1 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0x37e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_1_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x38a8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_1 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, set_RDD_INGRESS_FILTER_CFG, 0x38d8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x38e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x38e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_2_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3900 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x39c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_1 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, set_RDD_MULTICAST_KEY_MASK_ENTRY, 0x39e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_FLOW_RING_CACHE_LKP_TABLE_1 =
{
	2,
	{
		{ dump_RDD_DHD_FLOW_RING_CACHE_LKP_ENTRY, set_RDD_DHD_FLOW_RING_CACHE_LKP_ENTRY, 0x3a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_1 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, set_RDD_INGRESS_FILTER_CTRL, 0x3b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_2 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, set_RDD_POLICER_PARAMS_ENTRY, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SCORE_CFG_TABLE_2 =
{
	8,
	{
		{ dump_RDD_SCORE_CFG_ENTRY, set_RDD_SCORE_CFG_ENTRY, 0x368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_2 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, set_RDD_SPDTEST_GEN_CFG, 0x370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SCORE_DATA_COLLECTION_PROFILE_TABLE_2 =
{
	16,
	{
		{ dump_RDD_SCORE_DATA_COLLECTION_PROFILE, set_RDD_SCORE_DATA_COLLECTION_PROFILE, 0x380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CPU_RX_SCRATCHPAD_2 =
{
	8,
	{
		{ dump_RDD_BYTES_8, set_RDD_BYTES_8, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_2 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xa00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_2 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0xb68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CPU_RX_AUX_INT_SCRATCHPAD_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xb70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_2 =
{
	8,
	{
		{ dump_RDD_GENERAL_TIMER_ENTRY, set_RDD_GENERAL_TIMER_ENTRY, 0xb80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CPU_RX_INTERRUPT_ID_DDR_ADDR_2 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, set_RDD_DDR_ADDRESS, 0xd68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xd70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CPU_RX_CSO_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xd80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SCORE_COLLECT_DROP_BITMAP_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xf68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xf6c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CPU_RX_CSO_DISPATCHER_CREDIT_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xf70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xf7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CPU_RXQ_SCRATCH_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER_2 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, set_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_2 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, set_RDD_TUNNELS_PARSING_CFG, 0x2168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_2 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x2170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CSO_BAD_IPV4_HDR_CSUM_PACKETS_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x217c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_2 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, set_RDD_VPORT_CFG_ENTRY, 0x2180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TASK_IDX_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x21fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RNR_WR_TEST_ADDR_2 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, set_RDD_DDR_ADDRESS, 0x2368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x237c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_ACTION_VEC_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x237e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CPU_REASON_AND_VPORT_TO_METER_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x23e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_2 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, set_RDD_IPTV_CFG, 0x2568 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_2 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0x2570 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2580 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_2 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, set_RDD_TCAM_GENERIC, 0x25c8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_2 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, set_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x25d8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_2 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0x25e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_2 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, set_RDD_NAT_CACHE_CFG, 0x27e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x27ef },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CPU_INTERRUPT_COALESCING_TABLE_2 =
{
	16,
	{
		{ dump_RDD_CPU_INTERRUPT_COALESCING_ENTRY, set_RDD_CPU_INTERRUPT_COALESCING_ENTRY, 0x27f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_2 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, set_RDD_RX_FLOW_ENTRY, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CPU_RX_METER_TABLE_2 =
{
	8,
	{
		{ dump_RDD_CPU_RX_METER_ENTRY, set_RDD_CPU_RX_METER_ENTRY, 0x2900 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_2 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0x2a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2ad4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2ad6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2ad7 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CPU_RX_INTERRUPT_SCRATCH_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2ad8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RNR_WR_TEST_FROM_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2adc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_2 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, set_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0x2ae0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CPU_RING_INTERRUPT_COUNTER_TABLE_2 =
{
	8,
	{
		{ dump_RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY, set_RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY, 0x2b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CPU_REASON_TO_METER_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2bc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CPU_RX_XPM_PD_FIFO_TABLE_2 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, set_RDD_PROCESSING_TX_DESCRIPTOR, 0x2c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT IPV6_HOST_ADDRESS_CRC_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2cc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CSO_CONTEXT_TABLE_2 =
{
	176,
	{
		{ dump_RDD_CSO_CONTEXT_ENTRY, set_RDD_CSO_CONTEXT_ENTRY, 0x2d00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CPU_RX_LOCAL_SCRATCH_2 =
{
	8,
	{
		{ dump_RDD_BYTES_8, set_RDD_BYTES_8, 0x2db0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_TABLE_2 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, set_RDD_MIRRORING_TRUNCATE_ENTRY, 0x2dc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2dfe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT LLQ_SELECTOR_ECN_MASK_2 =
{
	1,
	{
		{ dump_RDD_LLQ_SELECTOR_ECN, set_RDD_LLQ_SELECTOR_ECN, 0x2dff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CPU_RX_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RNR_WR_TEST_TO_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2e88 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_2 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x2e8c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_2 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x2e8e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_2 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, set_RDD_GDX_PARAMS, 0x2e90 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2e9c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2e9d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CSO_DISABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2e9e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RNR_WR_TEST_START_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2e9f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_2 =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, set_RDD_BUFMNG_STATUS_ENTRY, 0x2ea0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT HOST_TX_TRUNCATE_MIRRORING_TABLE_2 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, set_RDD_MIRRORING_TRUNCATE_ENTRY, 0x2ec0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_2 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, set_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x2efe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2eff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_2 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, set_RDD_INGRESS_FILTER_CTRL, 0x2f00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2f80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2fbe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_2 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, set_RDD_VPORT_CFG_EX_ENTRY, 0x2fc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CPU_RING_DESCRIPTORS_TABLE_2 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, set_RDD_CPU_RING_DESCRIPTOR, 0x3000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_TABLE_2 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, set_RDD_TR471_SPDSVC_RX_PKT_ID, 0x3180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CPU_RX_TELEMETRY_BUFFER_LOCATION_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x31b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x31b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CPU_RX_CSO_UPDATE_FIFO_TABLE_2 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, set_RDD_UPDATE_FIFO_ENTRY, 0x31c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT IPV4_HOST_ADDRESS_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x31e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CPU_RX_CSO_PD_FIFO_TABLE_2 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, set_RDD_PROCESSING_TX_DESCRIPTOR, 0x3200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CPU_RX_XPM_UPDATE_FIFO_TABLE_2 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, set_RDD_UPDATE_FIFO_ENTRY, 0x3280 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CPU_REASON_TO_TC_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x32a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TC_TO_CPU_RXQ_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x32e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT EXC_TC_TO_CPU_RXQ_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3320 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_2 =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, set_RDD_DHD_HW_CONFIGURATION, 0x3360 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_2 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, set_RDD_TCAM_TABLE_CFG, 0x3378 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CPU_RXQ_DATA_BUF_TYPE_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_2 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0x33a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_2 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0x33c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x33e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x33e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_2 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, set_RDD_INGRESS_FILTER_CFG, 0x33f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x33f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3420 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_2 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, set_RDD_MULTICAST_KEY_MASK_ENTRY, 0x3460 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CPU_VPORT_TO_METER_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3480 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x34c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_TO_CPU_OBJ_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CPU_TX_SCRATCHPAD_3 =
{
	8,
	{
		{ dump_RDD_BYTES_8, set_RDD_BYTES_8, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_3 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, set_RDD_POLICER_PARAMS_ENTRY, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SCORE_CFG_TABLE_3 =
{
	8,
	{
		{ dump_RDD_SCORE_CFG_ENTRY, set_RDD_SCORE_CFG_ENTRY, 0x768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_3 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, set_RDD_SPDTEST_GEN_CFG, 0x770 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SCORE_DATA_COLLECTION_PROFILE_TABLE_3 =
{
	16,
	{
		{ dump_RDD_SCORE_DATA_COLLECTION_PROFILE, set_RDD_SCORE_DATA_COLLECTION_PROFILE, 0x780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_3 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xa00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_3 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0xb68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFFER_ALLOC_REPLY_3 =
{
	8,
	{
		{ dump_RDD_BYTES_8, set_RDD_BYTES_8, 0xb70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_3 =
{
	8,
	{
		{ dump_RDD_GENERAL_TIMER_ENTRY, set_RDD_GENERAL_TIMER_ENTRY, 0xb80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_3 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, set_RDD_DDR_ADDRESS, 0xd68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_INTERRUPT_COALESCING_TABLE_3 =
{
	16,
	{
		{ dump_RDD_CPU_INTERRUPT_COALESCING_ENTRY, set_RDD_CPU_INTERRUPT_COALESCING_ENTRY, 0xd70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_3 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, set_RDD_VPORT_CFG_ENTRY, 0xd80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DDR_LATENCY_DBG_USEC_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xdfc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CPU_RX_INTERRUPT_ID_DDR_ADDR_3 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, set_RDD_DDR_ADDRESS, 0xf68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SG_DESC_TABLE_3 =
{
	48,
	{
		{ dump_RDD_SG_DESC_ENTRY, set_RDD_SG_DESC_ENTRY, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xfe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER_3 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, set_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CPU_TX_RING_INDICES_VALUES_TABLE_3 =
{
	4,
	{
		{ dump_RDD_CPU_TX_RING_INDICES, set_RDD_CPU_TX_RING_INDICES, 0x2168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DDR_LATENCY_DBG_USEC_MAX_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x217c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SCORE_COLLECT_DROP_BITMAP_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x21c8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x21cc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CPU_TX_INGRESS_DISPATCHER_CREDIT_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x21d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x21dc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_3 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0x21e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_3 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, set_RDD_TUNNELS_PARSING_CFG, 0x23e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_3 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x23f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CSO_BAD_IPV4_HDR_CSUM_PACKETS_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x23fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE_3 =
{
	8,
	{
		{ dump_RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY, set_RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY, 0x2568 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2570 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CPU_INT_INTERRUPT_SCRATCH_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x257c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_TABLE_3 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, set_RDD_MIRRORING_TRUNCATE_ENTRY, 0x2580 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x25be },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT HOST_TX_TRUNCATE_MIRRORING_TABLE_3 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, set_RDD_MIRRORING_TRUNCATE_ENTRY, 0x25c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_ACTION_VEC_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x25fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_3 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, set_RDD_IPTV_CFG, 0x2768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_3 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0x2770 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x27be },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_3 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, set_RDD_VPORT_CFG_EX_ENTRY, 0x27c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x27fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x27ff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_3 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, set_RDD_RX_FLOW_ENTRY, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CPU_TX_0_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2900 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_3 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, set_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0x29e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_SRAM_PD_FIFO_3 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, set_RDD_PROCESSING_TX_DESCRIPTOR, 0x2a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CPU_TX_1_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_3 =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, set_RDD_BUFMNG_STATUS_ENTRY, 0x2be0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_3 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0x2c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TASK_IDX_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2cd4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_3 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, set_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x2cd8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2ce0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SG_CONTEXT_TABLE_3 =
{
	64,
	{
		{ dump_RDD_SG_CONTEXT_ENTRY, set_RDD_SG_CONTEXT_ENTRY, 0x2d00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT QUEUE_THRESHOLD_VECTOR_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2d80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_3 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, set_RDD_NAT_CACHE_CFG, 0x2db8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2dbf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_TABLE_3 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, set_RDD_TR471_SPDSVC_RX_PKT_ID, 0x2dc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_3 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, set_RDD_TCAM_GENERIC, 0x2df0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_3 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, set_RDD_INGRESS_FILTER_CTRL, 0x2e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RING_CPU_TX_DESCRIPTOR_DATA_TABLE_3 =
{
	16,
	{
		{ dump_RDD_RING_CPU_TX_DESCRIPTOR, set_RDD_RING_CPU_TX_DESCRIPTOR, 0x2e80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_3 =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, set_RDD_DHD_HW_CONFIGURATION, 0x2ea0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2eb4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT LLQ_SELECTOR_ECN_MASK_3 =
{
	1,
	{
		{ dump_RDD_LLQ_SELECTOR_ECN, set_RDD_LLQ_SELECTOR_ECN, 0x2eb5 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_3 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x2eb6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_SHADOW_RD_IDX_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2eb8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_3 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x2eba },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2ebc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2ebd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CSO_DISABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2ebe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_3 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, set_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x2ebf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_3 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0x2ec0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2edf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CPU_TX_RING_DESCRIPTOR_TABLE_3 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, set_RDD_CPU_RING_DESCRIPTOR, 0x2ee0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2f00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2f1f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2f20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_DESCRIPTOR_TABLE_3 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, set_RDD_CPU_RING_DESCRIPTOR, 0x2f50 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_3 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0x2f60 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CPU_TX_SYNC_FIFO_TABLE_3 =
{
	8,
	{
		{ dump_RDD_CPU_TX_SYNC_FIFO_ENTRY, set_RDD_CPU_TX_SYNC_FIFO_ENTRY, 0x2f80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE_3 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, set_RDD_DDR_ADDRESS, 0x2f90 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_INTERRUPT_SCRATCH_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2fa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_SHADOW_WR_IDX_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2fa8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_3 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, set_RDD_GDX_PARAMS, 0x2fb0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2fc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_NEXT_PTR_TABLE_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2fd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2fd8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2fe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_3 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, set_RDD_TCAM_TABLE_CFG, 0x2fe8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2ff0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_3 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, set_RDD_INGRESS_FILTER_CFG, 0x2ff8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_3 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, set_RDD_MULTICAST_KEY_MASK_ENTRY, 0x3000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3008 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SQ_TM_PD_FIFO_TABLE_4 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, set_RDD_PROCESSING_TX_DESCRIPTOR, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CRYPTO_SESSION_PARAMS_TABLE_4 =
{
	12,
	{
		{ dump_RDD_CRYPTO_SESSION_PARAMS, set_RDD_CRYPTO_SESSION_PARAMS, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SQ_TM_RATE_LIMITER_BUDGET_DESCRIPTOR_TABLE_4 =
{
	8,
	{
		{ dump_RDD_RATE_LIMITER_BUDGET_DESCRIPTOR, set_RDD_RATE_LIMITER_BUDGET_DESCRIPTOR, 0x700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_4 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, set_RDD_POLICER_PARAMS_ENTRY, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xa00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_SCRATCHPAD_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xb68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SQ_TM_RATE_LIMITER_BUDGET_VALID_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xb70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SQ_TM_RATE_LIMITER_PROFILE_TABLE_4 =
{
	4,
	{
		{ dump_RDD_RATE_LIMITER_PROFILE, set_RDD_RATE_LIMITER_PROFILE, 0xb80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_4 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SCORE_CFG_TABLE_4 =
{
	8,
	{
		{ dump_RDD_SCORE_CFG_ENTRY, set_RDD_SCORE_CFG_ENTRY, 0xf68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_4 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, set_RDD_SPDTEST_GEN_CFG, 0xf70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER_4 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, set_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_4 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x2168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SQ_TM_DCL_IMG_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CPU_RX_INTERRUPT_ID_DDR_ADDR_4 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, set_RDD_DDR_ADDRESS, 0x2368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SQ_TM_SECONDARY_SCHEDULER_TABLE_4 =
{
	12,
	{
		{ dump_RDD_SECONDARY_SCHEDULER_DESCRIPTOR, set_RDD_SECONDARY_SCHEDULER_DESCRIPTOR, 0x2370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SCORE_COLLECT_DROP_BITMAP_TABLE_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x237c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SCORE_DATA_COLLECTION_PROFILE_TABLE_4 =
{
	16,
	{
		{ dump_RDD_SCORE_DATA_COLLECTION_PROFILE, set_RDD_SCORE_DATA_COLLECTION_PROFILE, 0x2380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_4 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, set_RDD_TUNNELS_PARSING_CFG, 0x2568 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SQ_TM_DISPATCHER_CREDIT_TABLE_4 =
{
	12,
	{
		{ dump_RDD_DISPATCHER_CREDIT_DESCRIPTOR, set_RDD_DISPATCHER_CREDIT_DESCRIPTOR, 0x2570 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x257c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_4 =
{
	8,
	{
		{ dump_RDD_GENERAL_TIMER_ENTRY, set_RDD_GENERAL_TIMER_ENTRY, 0x2580 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CSO_BAD_IPV4_HDR_CSUM_PACKETS_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x276c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_4 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x2770 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SQ_TM_UPDATE_FIFO_START_INDEX_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x277c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_4 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, set_RDD_VPORT_CFG_ENTRY, 0x2780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TASK_IDX_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x27fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_4 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, set_RDD_IPTV_CFG, 0x2968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_4 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x297c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_ACTION_VEC_4 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x297e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_4 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, set_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x29c8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_4 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0x29d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SQ_TM_RATE_LIMITER_PROFILE_RESIDUE_TABLE_4 =
{
	1,
	{
		{ dump_RDD_RATE_LIMITER_PROFILE_RESIDUE, set_RDD_RATE_LIMITER_PROFILE_RESIDUE, 0x29e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SQ_TM_AQM_QUEUE_TABLE_4 =
{
	4,
	{
		{ dump_RDD_AQM_QUEUE_DESCRIPTOR, set_RDD_AQM_QUEUE_DESCRIPTOR, 0x2b68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_4 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, set_RDD_TCAM_GENERIC, 0x2be8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_4 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, set_RDD_NAT_CACHE_CFG, 0x2bf8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2bff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SQ_TM_SCHEDULING_QUEUE_TABLE_4 =
{
	10,
	{
		{ dump_RDD_SCHEDULING_QUEUE_DESCRIPTOR, set_RDD_SCHEDULING_QUEUE_DESCRIPTOR, 0x2c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2d40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SPU_REQUEST_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2dc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SQ_TM_RATE_LIMITER_PARAMS_DESCRIPTOR_TABLE_4 =
{
	8,
	{
		{ dump_RDD_RATE_LIMITER_PARAMS_DESCRIPTOR, set_RDD_RATE_LIMITER_PARAMS_DESCRIPTOR, 0x2e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_4 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, set_RDD_RX_FLOW_ENTRY, 0x2f00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_4 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0x3000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_4 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x30d4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SQ_TM_FIRST_QUEUE_MAPPING_4 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x30d6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x30d8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x30dc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x30dd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x30de },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT LLQ_SELECTOR_ECN_MASK_4 =
{
	1,
	{
		{ dump_RDD_LLQ_SELECTOR_ECN, set_RDD_LLQ_SELECTOR_ECN, 0x30df },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x30e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_4 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, set_RDD_INGRESS_FILTER_CTRL, 0x3100 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SPU_REQUEST_PD_FIFO_TABLE_4 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, set_RDD_PROCESSING_TX_DESCRIPTOR, 0x3180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SPU_REQUEST_PREPEND_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x31c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFFER_CONG_SCRATCHPAD_4 =
{
	8,
	{
		{ dump_RDD_BUFFER_CONG_Q_OCCUPANCY, set_RDD_BUFFER_CONG_Q_OCCUPANCY, 0x3240 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CRYPTO_SESSION_STATS_SCRATCH_4 =
{
	128,
	{
		{ dump_RDD_CRYPTO_SESSION_SEQ_INFO, set_RDD_CRYPTO_SESSION_SEQ_INFO, 0x3280 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_TABLE_4 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, set_RDD_MIRRORING_TRUNCATE_ENTRY, 0x3300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_4 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x333e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT HOST_TX_TRUNCATE_MIRRORING_TABLE_4 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, set_RDD_MIRRORING_TRUNCATE_ENTRY, 0x3340 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_4 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x337e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_4 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x3380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x33be },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x33bf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_4 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, set_RDD_VPORT_CFG_EX_ENTRY, 0x33c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CSO_DISABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x33fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_4 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, set_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x33ff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFFER_CONG_DQM_NOT_EMPTY_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3434 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3435 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_4 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, set_RDD_TCAM_TABLE_CFG, 0x3438 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_TABLE_4 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, set_RDD_TR471_SPDSVC_RX_PKT_ID, 0x3440 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_4 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, set_RDD_GDX_PARAMS, 0x3470 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SQ_BUFFER_CONG_MGT_CFG_4 =
{
	48,
	{
		{ dump_RDD_BUFFER_CONG_MGT, set_RDD_BUFFER_CONG_MGT, 0x3480 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_4 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x34b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_4 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, set_RDD_INGRESS_FILTER_CFG, 0x34b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SPU_OFFLOAD_PARAMS_TABLE_4 =
{
	48,
	{
		{ dump_RDD_SPU_OFFLOAD_PARAMS, set_RDD_SPU_OFFLOAD_PARAMS, 0x34c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SPU_OFFLOAD_SCRATCHPAD_4 =
{
	8,
	{
		{ dump_RDD_BYTES_8, set_RDD_BYTES_8, 0x34f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3538 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SQ_TM_ENABLED_QUEUES_TABLE_4 =
{
	34,
	{
		{ dump_RDD_ENABLED_QUEUES_ENTRY, set_RDD_ENABLED_QUEUES_ENTRY, 0x3540 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SQ_TM_UPDATE_FIFO_TABLE_4 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, set_RDD_UPDATE_FIFO_ENTRY, 0x3580 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_4 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0x35a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_4 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, set_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0x35c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_4 =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, set_RDD_BUFMNG_STATUS_ENTRY, 0x35e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SQ_TM_DCL_DATA_BASE_4 =
{
	32,
	{
		{ dump_RDD_DCL_DATA, set_RDD_DCL_DATA, 0x3600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SQ_TM_BUFMNG_STATUS_TABLE_4 =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, set_RDD_BUFMNG_STATUS_ENTRY, 0x3620 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SPU_REQUEST_UPDATE_FIFO_TABLE_4 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, set_RDD_UPDATE_FIFO_ENTRY, 0x3640 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SQ_TM_FW_FLUSH_TABLE_4 =
{
	28,
	{
		{ dump_RDD_FW_FLUSH_ENTRY, set_RDD_FW_FLUSH_ENTRY, 0x3660 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_4 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0x3680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SQ_TM_SCHEDULER_TABLE_4 =
{
	24,
	{
		{ dump_RDD_SCHEDULER_DESCRIPTOR, set_RDD_SCHEDULER_DESCRIPTOR, 0x36a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x36c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x36e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_4 =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, set_RDD_DHD_HW_CONFIGURATION, 0x3720 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SQ_TM_SCHEDULER_POOL_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3738 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SQ_TM_AQM_QUEUE_TIMER_TABLE_4 =
{
	1,
	{
		{ dump_RDD_AQM_QUEUE_TIMER_DESCRIPTOR, set_RDD_AQM_QUEUE_TIMER_DESCRIPTOR, 0x3758 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SQ_TM_PI2_PROBABILITY_CALC_DESCRIPTOR_4 =
{
	20,
	{
		{ dump_RDD_PI2_PROBABILITY_CALC_DESCRIPTOR, set_RDD_PI2_PROBABILITY_CALC_DESCRIPTOR, 0x3780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_4 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0x37a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x37c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x37e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_4 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, set_RDD_MULTICAST_KEY_MASK_ENTRY, 0x3800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_STREAM_TABLE_5 =
{
	376,
	{
		{ dump_RDD_TCPSPDTEST_STREAM, set_RDD_TCPSPDTEST_STREAM, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x5e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_5 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, set_RDD_POLICER_PARAMS_ENTRY, 0x600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PKTGEN_TX_STREAM_TABLE_5 =
{
	136,
	{
		{ dump_RDD_PKTGEN_TX_STREAM_ENTRY, set_RDD_PKTGEN_TX_STREAM_ENTRY, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_5 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0xa20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT COMMON_REPROCESSING_STACK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xa40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SCORE_DATA_COLLECTION_PROFILE_TABLE_5 =
{
	16,
	{
		{ dump_RDD_SCORE_DATA_COLLECTION_PROFILE, set_RDD_SCORE_DATA_COLLECTION_PROFILE, 0xa80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_5 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, set_RDD_RX_FLOW_ENTRY, 0xb00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_5 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SCORE_CFG_TABLE_5 =
{
	8,
	{
		{ dump_RDD_SCORE_CFG_ENTRY, set_RDD_SCORE_CFG_ENTRY, 0xf68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_5 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, set_RDD_SPDTEST_GEN_CFG, 0xf70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_SCRATCHPAD_5 =
{
	8,
	{
		{ dump_RDD_BYTES_8, set_RDD_BYTES_8, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER_5 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, set_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_5 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x2168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SPDSVC_GEN_STACK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TX_ABS_RECYCLE_COUNTERS_5 =
{
	8,
	{
		{ dump_RDD_TX_ABS_RECYCLE_COUNTERS_ENTRY, set_RDD_TX_ABS_RECYCLE_COUNTERS_ENTRY, 0x2368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SPDSVC_GEN_DISPATCHER_CREDIT_TABLE_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SCORE_COLLECT_DROP_BITMAP_TABLE_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x237c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_5 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, set_RDD_VPORT_CFG_ENTRY, 0x2380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PKTGEN_NO_SBPM_HDRS_CNTR_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x23fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CPU_RX_INTERRUPT_ID_DDR_ADDR_5 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, set_RDD_DDR_ADDRESS, 0x2568 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_DISPATCHER_CREDIT_TABLE_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2570 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x257c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PKTGEN_SBPM_HDR_BNS_5 =
{
	2,
	{
		{ dump_RDD_PKTGEN_SBPM_HDR_BN, set_RDD_PKTGEN_SBPM_HDR_BN, 0x2580 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_TABLE_5 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, set_RDD_MIRRORING_TRUNCATE_ENTRY, 0x25c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_5 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x25fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PKTGEN_CURR_SBPM_HDR_PTR_5 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x27e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT COMMON_REPROCESSING_DISPATCHER_CREDIT_TABLE_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x27f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x27fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PKTGEN_NUM_OF_AVAIL_SBPM_HDRS_5 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_GEN_DISPATCHER_CREDIT_TABLE_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CSO_BAD_IPV4_HDR_CSUM_PACKETS_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x297c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT COMMON_REPROCESSING_PD_FIFO_TABLE_5 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, set_RDD_PROCESSING_TX_DESCRIPTOR, 0x2980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT HOST_TX_TRUNCATE_MIRRORING_TABLE_5 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, set_RDD_MIRRORING_TRUNCATE_ENTRY, 0x29c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_5 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x29fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PKTGEN_SBPM_END_PTR_5 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2b68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_5 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x2b70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PKTGEN_BAD_GET_NEXT_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2b7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_PD_FIFO_TABLE_5 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, set_RDD_PROCESSING_TX_DESCRIPTOR, 0x2b80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_5 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2bc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2bfe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2bff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_5 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, set_RDD_TUNNELS_PARSING_CFG, 0x2d68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2d70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PKTGEN_MAX_UT_PKTS_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2d7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_5 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, set_RDD_VPORT_CFG_EX_ENTRY, 0x2d80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2dbe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2dbf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SPDSVC_GEN_PARAMS_TABLE_5 =
{
	60,
	{
		{ dump_RDD_SPDSVC_GEN_PARAMS, set_RDD_SPDSVC_GEN_PARAMS, 0x2dc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PKTGEN_UT_TRIGGER_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2dfc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_DOWNLOAD_STACK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PKTGEN_SBPM_EXTS_5 =
{
	4,
	{
		{ dump_RDD_PKTGEN_SBPM_EXT, set_RDD_PKTGEN_SBPM_EXT, 0x2ef0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_5 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0x2f00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TASK_IDX_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2fd4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_5 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, set_RDD_IPTV_CFG, 0x2fd8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_5 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, set_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0x2fe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT UDPSPDT_STREAM_TX_STAT_TABLE_5 =
{
	40,
	{
		{ dump_RDD_UDPSPDT_STREAM_TX_STAT, set_RDD_UDPSPDT_STREAM_TX_STAT, 0x3000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_5 =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, set_RDD_BUFMNG_STATUS_ENTRY, 0x30a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_TABLE_5 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, set_RDD_TR471_SPDSVC_RX_PKT_ID, 0x30c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PKTGEN_BBMSG_REPLY_SCRATCH_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x30f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_5 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, set_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x30f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_5 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, set_RDD_INGRESS_FILTER_CTRL, 0x3100 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PKTGEN_SESSION_DATA_5 =
{
	32,
	{
		{ dump_RDD_PKTGEN_TX_PARAMS, set_RDD_PKTGEN_TX_PARAMS, 0x3180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_ENGINE_GLOBAL_TABLE_5 =
{
	20,
	{
		{ dump_RDD_TCPSPDTEST_ENGINE_GLOBAL_INFO, set_RDD_TCPSPDTEST_ENGINE_GLOBAL_INFO, 0x31a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SPDSVC_TCPSPDTEST_COMMON_TABLE_5 =
{
	1,
	{
		{ dump_RDD_SPDSVC_TCPSPDTEST_COMMON_ENTRY, set_RDD_SPDSVC_TCPSPDTEST_COMMON_ENTRY, 0x31b4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT LLQ_SELECTOR_ECN_MASK_5 =
{
	1,
	{
		{ dump_RDD_LLQ_SELECTOR_ECN, set_RDD_LLQ_SELECTOR_ECN, 0x31b5 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_5 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x31b6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_5 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, set_RDD_NAT_CACHE_CFG, 0x31b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x31bf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT COMMON_REPROCESSING_UPDATE_FIFO_TABLE_5 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, set_RDD_UPDATE_FIFO_ENTRY, 0x31c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_5 =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, set_RDD_DHD_HW_CONFIGURATION, 0x31e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_5 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x31f4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x31f6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CSO_DISABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x31f7 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x31f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_5 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, set_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x31fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x31fd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x31fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT UDPSPDT_TX_PARAMS_TABLE_5 =
{
	24,
	{
		{ dump_RDD_UDPSPDT_TX_PARAMS, set_RDD_UDPSPDT_TX_PARAMS, 0x3200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_5 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0x3260 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_UPDATE_FIFO_TABLE_5 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, set_RDD_UPDATE_FIFO_ENTRY, 0x3280 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_5 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0x32a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_5 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, set_RDD_TCAM_GENERIC, 0x32b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_5 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0x32c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_5 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, set_RDD_GDX_PARAMS, 0x32e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_5 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, set_RDD_TCAM_TABLE_CFG, 0x32f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_5 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x32f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3320 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_5 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, set_RDD_INGRESS_FILTER_CFG, 0x3350 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3358 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3360 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PKTGEN_FPM_UG_MGMT_5 =
{
	20,
	{
		{ dump_RDD_PKTGEN_FPM_UG_MGMT_ENTRY, set_RDD_PKTGEN_FPM_UG_MGMT_ENTRY, 0x3380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_5 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, set_RDD_MULTICAST_KEY_MASK_ENTRY, 0x33a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x33c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PKTGEN_TX_STREAM_SCRATCH_TABLE_5 =
{
	8,
	{
		{ dump_RDD_PKTGEN_TX_STREAM_SCRATCH_ENTRY, set_RDD_PKTGEN_TX_STREAM_SCRATCH_ENTRY, 0x3400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_6 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, set_RDD_POLICER_PARAMS_ENTRY, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SCORE_CFG_TABLE_6 =
{
	8,
	{
		{ dump_RDD_SCORE_CFG_ENTRY, set_RDD_SCORE_CFG_ENTRY, 0x368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_6 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, set_RDD_SPDTEST_GEN_CFG, 0x370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SCORE_DATA_COLLECTION_PROFILE_TABLE_6 =
{
	16,
	{
		{ dump_RDD_SCORE_DATA_COLLECTION_PROFILE, set_RDD_SCORE_DATA_COLLECTION_PROFILE, 0x380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_6 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_6 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x770 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_6 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, set_RDD_VPORT_CFG_ENTRY, 0x780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SCORE_COLLECT_DROP_BITMAP_TABLE_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x7fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_6 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, set_RDD_TUNNELS_PARSING_CFG, 0x9e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_6 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x9f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x9fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xa00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xb68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CSO_BAD_IPV4_HDR_CSUM_PACKETS_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xb6c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xb70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TASK_IDX_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xb7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_TABLE_6 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, set_RDD_MIRRORING_TRUNCATE_ENTRY, 0xb80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_6 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0xbbe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT HOST_TX_TRUNCATE_MIRRORING_TABLE_6 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, set_RDD_MIRRORING_TRUNCATE_ENTRY, 0xbc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_6 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0xbfe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_6 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, set_RDD_IPTV_CFG, 0xd68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_6 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0xd70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_6 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0xd80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xdbe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xdbf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_6 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, set_RDD_VPORT_CFG_EX_ENTRY, 0xdc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xdfe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xdff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_6 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, set_RDD_TCAM_GENERIC, 0xf68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_6 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, set_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0xf78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_TABLE_6 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, set_RDD_TR471_SPDSVC_RX_PKT_ID, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_6 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, set_RDD_GDX_PARAMS, 0xfb0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT LLQ_SELECTOR_ECN_MASK_6 =
{
	1,
	{
		{ dump_RDD_LLQ_SELECTOR_ECN, set_RDD_LLQ_SELECTOR_ECN, 0xfbc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xfbd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_6 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0xfbe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xfc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_6 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0xfe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER_6 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, set_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_6 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, set_RDD_NAT_CACHE_CFG, 0x2168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x216f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_6 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x2174 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CSO_DISABLE_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2176 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_6 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, set_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x2177 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_6 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, set_RDD_TCAM_TABLE_CFG, 0x2178 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x217a },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x217b },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_6 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, set_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0x2180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_6 =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, set_RDD_BUFMNG_STATUS_ENTRY, 0x21a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_6 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0x21c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_6 =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, set_RDD_DHD_HW_CONFIGURATION, 0x21e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_6 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x21f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_6 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, set_RDD_INGRESS_FILTER_CFG, 0x2368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x23a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_6 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0x23e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_6 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, set_RDD_RX_FLOW_ENTRY, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_6 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0x2500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x25e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_6 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, set_RDD_INGRESS_FILTER_CTRL, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_6 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, set_RDD_MULTICAST_KEY_MASK_ENTRY, 0x26a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_7 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, set_RDD_POLICER_PARAMS_ENTRY, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SCORE_CFG_TABLE_7 =
{
	8,
	{
		{ dump_RDD_SCORE_CFG_ENTRY, set_RDD_SCORE_CFG_ENTRY, 0x368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_7 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, set_RDD_SPDTEST_GEN_CFG, 0x370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SCORE_DATA_COLLECTION_PROFILE_TABLE_7 =
{
	16,
	{
		{ dump_RDD_SCORE_DATA_COLLECTION_PROFILE, set_RDD_SCORE_DATA_COLLECTION_PROFILE, 0x380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_7 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_7 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x770 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_7 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, set_RDD_VPORT_CFG_ENTRY, 0x780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SCORE_COLLECT_DROP_BITMAP_TABLE_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x7fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_7 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, set_RDD_TUNNELS_PARSING_CFG, 0x9e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_7 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x9f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x9fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xa00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xb68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CSO_BAD_IPV4_HDR_CSUM_PACKETS_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xb6c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xb70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TASK_IDX_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xb7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_TABLE_7 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, set_RDD_MIRRORING_TRUNCATE_ENTRY, 0xb80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_7 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0xbbe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT HOST_TX_TRUNCATE_MIRRORING_TABLE_7 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, set_RDD_MIRRORING_TRUNCATE_ENTRY, 0xbc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_7 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0xbfe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_7 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, set_RDD_IPTV_CFG, 0xd68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_7 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0xd70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_7 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0xd80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xdbe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xdbf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_7 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, set_RDD_VPORT_CFG_EX_ENTRY, 0xdc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xdfe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xdff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_7 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, set_RDD_TCAM_GENERIC, 0xf68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_7 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, set_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0xf78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_TABLE_7 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, set_RDD_TR471_SPDSVC_RX_PKT_ID, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_7 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, set_RDD_GDX_PARAMS, 0xfb0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT LLQ_SELECTOR_ECN_MASK_7 =
{
	1,
	{
		{ dump_RDD_LLQ_SELECTOR_ECN, set_RDD_LLQ_SELECTOR_ECN, 0xfbc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xfbd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_7 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0xfbe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xfc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_7 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0xfe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER_7 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, set_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_7 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, set_RDD_NAT_CACHE_CFG, 0x2168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x216f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_7 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x2174 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CSO_DISABLE_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2176 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_7 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, set_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x2177 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_7 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, set_RDD_TCAM_TABLE_CFG, 0x2178 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x217a },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x217b },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_7 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, set_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0x2180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_7 =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, set_RDD_BUFMNG_STATUS_ENTRY, 0x21a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_7 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0x21c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_7 =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, set_RDD_DHD_HW_CONFIGURATION, 0x21e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_7 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x21f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_7 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, set_RDD_INGRESS_FILTER_CFG, 0x2368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x23a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_7 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0x23e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_7 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, set_RDD_RX_FLOW_ENTRY, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_7 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0x2500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x25e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_7 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, set_RDD_INGRESS_FILTER_CTRL, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_7 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, set_RDD_MULTICAST_KEY_MASK_ENTRY, 0x26a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_8 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, set_RDD_POLICER_PARAMS_ENTRY, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SCORE_CFG_TABLE_8 =
{
	8,
	{
		{ dump_RDD_SCORE_CFG_ENTRY, set_RDD_SCORE_CFG_ENTRY, 0x368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_8 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, set_RDD_SPDTEST_GEN_CFG, 0x370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SCORE_DATA_COLLECTION_PROFILE_TABLE_8 =
{
	16,
	{
		{ dump_RDD_SCORE_DATA_COLLECTION_PROFILE, set_RDD_SCORE_DATA_COLLECTION_PROFILE, 0x380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_8 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_8 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SPDSVC_ANALYZER_PARAMS_TABLE_8 =
{
	16,
	{
		{ dump_RDD_SPDSVC_ANALYZER_PARAMS, set_RDD_SPDSVC_ANALYZER_PARAMS, 0x770 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_8 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, set_RDD_VPORT_CFG_ENTRY, 0x780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SCORE_COLLECT_DROP_BITMAP_TABLE_8 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x7fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_8 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, set_RDD_TUNNELS_PARSING_CFG, 0x968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SPDSVC_ANALYZER_PD_FIFO_TABLE_8 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, set_RDD_PROCESSING_TX_DESCRIPTOR, 0x980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_TABLE_8 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, set_RDD_MIRRORING_TRUNCATE_ENTRY, 0x9c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_8 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x9fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xa00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_8 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xb68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_8 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xbe8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_8 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xbec },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_8 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0xbf0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CSO_BAD_IPV4_HDR_CSUM_PACKETS_8 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xbfc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_8 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, set_RDD_IPTV_CFG, 0xd68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_8 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xd70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TASK_IDX_8 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xd7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT HOST_TX_TRUNCATE_MIRRORING_TABLE_8 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, set_RDD_MIRRORING_TRUNCATE_ENTRY, 0xd80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_8 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0xdbe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_8 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0xdc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xdfe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xdff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_8 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, set_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0xf68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT UDPSPDT_SCRATCH_TABLE_8 =
{
	8,
	{
		{ dump_RDD_UDPSPDT_SCRATCH_IPERF3_RX, set_RDD_UDPSPDT_SCRATCH_IPERF3_RX, 0xf70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_8 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, set_RDD_NAT_CACHE_CFG, 0xf78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_8 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, set_RDD_VPORT_CFG_EX_ENTRY, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xfbe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT LLQ_SELECTOR_ECN_MASK_8 =
{
	1,
	{
		{ dump_RDD_LLQ_SELECTOR_ECN, set_RDD_LLQ_SELECTOR_ECN, 0xfbf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_TABLE_8 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, set_RDD_TR471_SPDSVC_RX_PKT_ID, 0xfc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_8 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0xff0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER_8 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, set_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_8 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, set_RDD_TCAM_GENERIC, 0x2168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_8 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2178 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_8 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x217c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_8 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x217e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_8 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_8 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0x21a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_8 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, set_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0x21c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_8 =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, set_RDD_BUFMNG_STATUS_ENTRY, 0x21e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_8 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, set_RDD_TCAM_TABLE_CFG, 0x2368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x236a },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x236b },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CSO_DISABLE_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x236c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_8 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, set_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x236d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x236e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x236f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_8 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, set_RDD_GDX_PARAMS, 0x2370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SPDSVC_ANALYZER_UPDATE_FIFO_TABLE_8 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, set_RDD_UPDATE_FIFO_ENTRY, 0x2380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_8 =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, set_RDD_DHD_HW_CONFIGURATION, 0x23a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_8 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x23b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_8 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0x23c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_8 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0x23e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_8 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, set_RDD_RX_FLOW_ENTRY, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_8 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0x2500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_8 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, set_RDD_INGRESS_FILTER_CFG, 0x25d8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_8 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x25e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x25e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT UDPSPDT_STREAM_RX_STAT_TABLE_8 =
{
	48,
	{
		{ dump_RDD_UDPSPDT_STREAM_RX_STAT, set_RDD_UDPSPDT_STREAM_RX_STAT, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x26c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_8 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, set_RDD_MULTICAST_KEY_MASK_ENTRY, 0x26e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SPDSVC_ANALYZER_STACK_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_8 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x27b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_8 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, set_RDD_INGRESS_FILTER_CTRL, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_8 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2880 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_9 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, set_RDD_POLICER_PARAMS_ENTRY, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SCORE_INTERRUPT_ID_DDR_ADDR_9 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, set_RDD_DDR_ADDRESS, 0x368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_9 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, set_RDD_SPDTEST_GEN_CFG, 0x370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DATA_TRACE_SCRATCHPAD_9 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_9 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SCORE_CFG_TABLE_9 =
{
	8,
	{
		{ dump_RDD_SCORE_CFG_ENTRY, set_RDD_SCORE_CFG_ENTRY, 0x768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x770 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SCORE_DATA_COLLECTION_PROFILE_TABLE_9 =
{
	16,
	{
		{ dump_RDD_SCORE_DATA_COLLECTION_PROFILE, set_RDD_SCORE_DATA_COLLECTION_PROFILE, 0x780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_9 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SPU_RESPONSE_DISPATCHER_CREDIT_TABLE_9 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SCORE_QSTAT_SEQ_NUM_9 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x97c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_9 =
{
	8,
	{
		{ dump_RDD_GENERAL_TIMER_ENTRY, set_RDD_GENERAL_TIMER_ENTRY, 0x980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xa00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CPU_RX_INTERRUPT_ID_DDR_ADDR_9 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, set_RDD_DDR_ADDRESS, 0xb68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_9 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0xb70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SCORE_COLLECT_DROP_BITMAP_TABLE_9 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xb7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_9 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, set_RDD_VPORT_CFG_ENTRY, 0xb80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_9 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xbfc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SCORE_COALESCING_TABLE_9 =
{
	6,
	{
		{ dump_RDD_SCORE_COALESCING, set_RDD_SCORE_COALESCING, 0xd68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_9 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0xd6e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_9 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xd70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_9 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xd7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SPU_RESPONSE_STACK_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xd80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_9 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xde0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_9 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, set_RDD_TUNNELS_PARSING_CFG, 0xf68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_9 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0xf70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_STACK_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_9 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, set_RDD_TCAM_GENERIC, 0xfc8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CSO_BAD_IPV4_HDR_CSUM_PACKETS_9 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xfd8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TASK_IDX_9 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xfdc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_9 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0xfe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER_9 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, set_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_9 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_9 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, set_RDD_IPTV_CFG, 0x21e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_9 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, set_RDD_GDX_PARAMS, 0x21f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_ACTION_VEC_9 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x21fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_9 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x21fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_9 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, set_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x2368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_9 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, set_RDD_NAT_CACHE_CFG, 0x2370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2377 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_9 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2378 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x237c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x237d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x237e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT LLQ_SELECTOR_ECN_MASK_9 =
{
	1,
	{
		{ dump_RDD_LLQ_SELECTOR_ECN, set_RDD_LLQ_SELECTOR_ECN, 0x237f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CRYPTO_SESSION_STATS_SCRATCH_9 =
{
	128,
	{
		{ dump_RDD_CRYPTO_SESSION_SEQ_INFO, set_RDD_CRYPTO_SESSION_SEQ_INFO, 0x2380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_9 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, set_RDD_RX_FLOW_ENTRY, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SCORE_SDR_DB_TABLE_9 =
{
	32,
	{
		{ dump_RDD_SCORE_SDR_DB_ENTRY, set_RDD_SCORE_SDR_DB_ENTRY, 0x2500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_9 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_9 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x26d4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_9 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x26d6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_9 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, set_RDD_TCAM_TABLE_CFG, 0x26d8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x26da },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x26db },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CSO_DISABLE_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x26dc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_9 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, set_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x26dd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x26de },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x26df },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_9 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, set_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0x26e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_9 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, set_RDD_INGRESS_FILTER_CTRL, 0x2700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_TABLE_9 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, set_RDD_MIRRORING_TRUNCATE_ENTRY, 0x2780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT HOST_TX_TRUNCATE_MIRRORING_TABLE_9 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, set_RDD_MIRRORING_TRUNCATE_ENTRY, 0x27c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_9 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_9 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, set_RDD_VPORT_CFG_EX_ENTRY, 0x2840 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SPU_RESPONSE_PARAMS_TABLE_9 =
{
	56,
	{
		{ dump_RDD_SPU_RESPONSE_PARAMS, set_RDD_SPU_RESPONSE_PARAMS, 0x2880 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_9 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x28b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_TABLE_9 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, set_RDD_TR471_SPDSVC_RX_PKT_ID, 0x28c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_9 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, set_RDD_INGRESS_FILTER_CFG, 0x28f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x28f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DATA_TRACE_STACK_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2900 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SPU_RESPONSE_SCRATCHPAD_9 =
{
	8,
	{
		{ dump_RDD_BYTES_8, set_RDD_BYTES_8, 0x2928 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_9 =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, set_RDD_BUFMNG_STATUS_ENTRY, 0x2980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_9 =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, set_RDD_DHD_HW_CONFIGURATION, 0x29a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DATA_TRACE_UPDATE_FIFO_TABLE_9 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, set_RDD_UPDATE_FIFO_ENTRY, 0x29c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_9 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0x29e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DATA_TRACE_PD_FIFO_TABLE_9 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, set_RDD_PROCESSING_TX_DESCRIPTOR, 0x2a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_9 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2a20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_9 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0x2a40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_9 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, set_RDD_MULTICAST_KEY_MASK_ENTRY, 0x2a60 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2a80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_9 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2aa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SCORE_MSG_RING_DESCRIPTOR_TABLE_9 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, set_RDD_CPU_RING_DESCRIPTOR, 0x2b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SCORE_QSTAT_RING_DESCRIPTOR_TABLE_9 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, set_RDD_CPU_RING_DESCRIPTOR, 0x2b40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SCORE_BNL_RING_DESCRIPTOR_TABLE_9 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, set_RDD_CPU_RING_DESCRIPTOR, 0x2b80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_9 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2bc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DS_TM_PD_FIFO_TABLE_10 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, set_RDD_PROCESSING_TX_DESCRIPTOR, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_10 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, set_RDD_POLICER_PARAMS_ENTRY, 0xa00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_SCHEDULING_QUEUE_TABLE_10 =
{
	10,
	{
		{ dump_RDD_SCHEDULING_QUEUE_DESCRIPTOR, set_RDD_SCHEDULING_QUEUE_DESCRIPTOR, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_RATE_LIMITER_PROFILE_RESIDUE_TABLE_10 =
{
	1,
	{
		{ dump_RDD_RATE_LIMITER_PROFILE_RESIDUE, set_RDD_RATE_LIMITER_PROFILE_RESIDUE, 0xf20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DS_TM_TM_FLOW_CNTR_TABLE_10 =
{
	1,
	{
		{ dump_RDD_TM_FLOW_CNTR_ENTRY, set_RDD_TM_FLOW_CNTR_ENTRY, 0xf40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TX_TASK_0_STACK_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING_4_TASKS_PACKET_BUFFER_10 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, set_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_RATE_LIMITER_BUDGET_DESCRIPTOR_TABLE_10 =
{
	8,
	{
		{ dump_RDD_RATE_LIMITER_BUDGET_DESCRIPTOR, set_RDD_RATE_LIMITER_BUDGET_DESCRIPTOR, 0x1800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_RATE_LIMITER_PROFILE_TABLE_10 =
{
	4,
	{
		{ dump_RDD_RATE_LIMITER_PROFILE, set_RDD_RATE_LIMITER_PROFILE, 0x1a80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_10 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, set_RDD_RX_FLOW_ENTRY, 0x1b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_RATE_LIMITER_PARAMS_DESCRIPTOR_TABLE_10 =
{
	8,
	{
		{ dump_RDD_RATE_LIMITER_PARAMS_DESCRIPTOR, set_RDD_RATE_LIMITER_PARAMS_DESCRIPTOR, 0x1c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_DCL_IMG_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x1e80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_10 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0x1f00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SCORE_COLLECT_DROP_BITMAP_TABLE_10 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x1fd4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DS_TM_BBH_TX_WAKE_UP_DATA_TABLE_10 =
{
	8,
	{
		{ dump_RDD_BBH_TX_WAKE_UP_DATA_ENTRY, set_RDD_BBH_TX_WAKE_UP_DATA_ENTRY, 0x1fd8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_10 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x1fe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_10 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SCORE_CFG_TABLE_10 =
{
	8,
	{
		{ dump_RDD_SCORE_CFG_ENTRY, set_RDD_SCORE_CFG_ENTRY, 0x2368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_RATE_LIMITER_BUDGET_VALID_10 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SCORE_DATA_COLLECTION_PROFILE_TABLE_10 =
{
	16,
	{
		{ dump_RDD_SCORE_DATA_COLLECTION_PROFILE, set_RDD_SCORE_DATA_COLLECTION_PROFILE, 0x2380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_10 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x2568 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_10 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, set_RDD_SPDTEST_GEN_CFG, 0x2570 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_10 =
{
	8,
	{
		{ dump_RDD_GENERAL_TIMER_ENTRY, set_RDD_GENERAL_TIMER_ENTRY, 0x2580 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT MIRRORING_SCRATCH_10 =
{
	8,
	{
		{ dump_RDD_BYTES_8, set_RDD_BYTES_8, 0x2768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2770 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_10 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, set_RDD_VPORT_CFG_ENTRY, 0x2780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_10 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x27fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_10 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, set_RDD_TUNNELS_PARSING_CFG, 0x2968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_10 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x2970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_10 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x297c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_SECONDARY_SCHEDULER_TABLE_10 =
{
	12,
	{
		{ dump_RDD_SECONDARY_SCHEDULER_DESCRIPTOR, set_RDD_SECONDARY_SCHEDULER_DESCRIPTOR, 0x2980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CSO_BAD_IPV4_HDR_CSUM_PACKETS_10 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x29d4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_10 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, set_RDD_IPTV_CFG, 0x29d8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_10 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0x29e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_AQM_QUEUE_TABLE_10 =
{
	4,
	{
		{ dump_RDD_AQM_QUEUE_DESCRIPTOR, set_RDD_AQM_QUEUE_DESCRIPTOR, 0x2a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_SCHEDULER_POOL_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2b40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_10 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, set_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0x2be0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_SCHEDULER_TABLE_10 =
{
	24,
	{
		{ dump_RDD_SCHEDULER_DESCRIPTOR, set_RDD_SCHEDULER_DESCRIPTOR, 0x2c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_10 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, set_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x2ca8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_10 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2cb0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_DCL_MULTIPLE_CNTR_10 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2cbc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT UPDATE_FIFO_STACK_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2cc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_10 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, set_RDD_INGRESS_FILTER_CTRL, 0x2d00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_AQM_QUEUE_TIMER_TABLE_10 =
{
	1,
	{
		{ dump_RDD_AQM_QUEUE_TIMER_DESCRIPTOR, set_RDD_AQM_QUEUE_TIMER_DESCRIPTOR, 0x2d80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_DISPATCHER_CREDIT_TABLE_10 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2dd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TASK_IDX_10 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2ddc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_10 =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, set_RDD_BUFMNG_STATUS_ENTRY, 0x2de0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_STACK_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_10 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2e48 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DS_TM_CPU_TX_ABS_COUNTERS_10 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2ec8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_10 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, set_RDD_NAT_CACHE_CFG, 0x2f48 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2f4f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_10 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0x2f50 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_FW_FLUSH_TABLE_10 =
{
	28,
	{
		{ dump_RDD_FW_FLUSH_ENTRY, set_RDD_FW_FLUSH_ENTRY, 0x2f60 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUDGET_ALLOCATION_TIMER_VALUE_10 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2f7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_10 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2f7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFFER_CONG_SCRATCHPAD_10 =
{
	8,
	{
		{ dump_RDD_BUFFER_CONG_Q_OCCUPANCY, set_RDD_BUFFER_CONG_Q_OCCUPANCY, 0x2f80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_TABLE_10 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, set_RDD_MIRRORING_TRUNCATE_ENTRY, 0x2fc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_ACTION_VEC_10 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2ffe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT HOST_TX_TRUNCATE_MIRRORING_TABLE_10 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, set_RDD_MIRRORING_TRUNCATE_ENTRY, 0x3000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_10 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x303e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_TX_TRUNCATE_MIRRORING_TABLE_10 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, set_RDD_MIRRORING_TRUNCATE_ENTRY, 0x3040 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DS_TM_BB_DESTINATION_TABLE_10 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x307e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_10 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x3080 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x30be },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x30bf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_10 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, set_RDD_VPORT_CFG_EX_ENTRY, 0x30c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DS_TM_FIRST_QUEUE_MAPPING_10 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x30fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFFER_CONG_DQM_NOT_EMPTY_10 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3100 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3134 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT LLQ_SELECTOR_ECN_MASK_10 =
{
	1,
	{
		{ dump_RDD_LLQ_SELECTOR_ECN, set_RDD_LLQ_SELECTOR_ECN, 0x3135 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TX_EXCEPTION_10 =
{
	1,
	{
		{ dump_RDD_TX_EXCEPTION_ENTRY, set_RDD_TX_EXCEPTION_ENTRY, 0x3136 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3137 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_10 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3138 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_10 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x313c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_10 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x313e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_TABLE_10 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, set_RDD_TR471_SPDSVC_RX_PKT_ID, 0x3140 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_10 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, set_RDD_TCAM_GENERIC, 0x3170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DS_BUFFER_CONG_MGT_CFG_10 =
{
	48,
	{
		{ dump_RDD_BUFFER_CONG_MGT, set_RDD_BUFFER_CONG_MGT, 0x3180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_10 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, set_RDD_GDX_PARAMS, 0x31b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x31bc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BACKUP_BBH_INGRESS_COUNTERS_TABLE_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x31bd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BACKUP_BBH_EGRESS_COUNTERS_TABLE_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x31be },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CSO_DISABLE_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x31bf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_ENABLED_QUEUES_TABLE_10 =
{
	34,
	{
		{ dump_RDD_ENABLED_QUEUES_ENTRY, set_RDD_ENABLED_QUEUES_ENTRY, 0x31c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_10 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, set_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x31e2 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x31e3 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x31e4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_10 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, set_RDD_TCAM_TABLE_CFG, 0x31e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_10 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x31f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_10 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, set_RDD_INGRESS_FILTER_CFG, 0x31f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DS_TM_BBH_QUEUE_TABLE_10 =
{
	8,
	{
		{ dump_RDD_BBH_QUEUE_DESCRIPTOR, set_RDD_BBH_QUEUE_DESCRIPTOR, 0x3200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3238 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DS_TM_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_TABLE_10 =
{
	40,
	{
		{ dump_RDD_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_ENTRY, set_RDD_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_ENTRY, 0x3240 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_10 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3290 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT UPDATE_FIFO_TABLE_10 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, set_RDD_UPDATE_FIFO_ENTRY, 0x32c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DS_TM_PI2_PROBABILITY_CALC_DESCRIPTOR_10 =
{
	20,
	{
		{ dump_RDD_PI2_PROBABILITY_CALC_DESCRIPTOR, set_RDD_PI2_PROBABILITY_CALC_DESCRIPTOR, 0x32e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_DCL_DATA_BASE_10 =
{
	32,
	{
		{ dump_RDD_DCL_DATA, set_RDD_DCL_DATA, 0x3300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_10 =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, set_RDD_DHD_HW_CONFIGURATION, 0x3320 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_10 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0x3340 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_10 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0x3360 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_10 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x33a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_10 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x33c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_10 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, set_RDD_MULTICAST_KEY_MASK_ENTRY, 0x33e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DS_TM_PD_FIFO_TABLE_11 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, set_RDD_PROCESSING_TX_DESCRIPTOR, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_11 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, set_RDD_POLICER_PARAMS_ENTRY, 0xa00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_SCHEDULING_QUEUE_TABLE_11 =
{
	10,
	{
		{ dump_RDD_SCHEDULING_QUEUE_DESCRIPTOR, set_RDD_SCHEDULING_QUEUE_DESCRIPTOR, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_RATE_LIMITER_PROFILE_RESIDUE_TABLE_11 =
{
	1,
	{
		{ dump_RDD_RATE_LIMITER_PROFILE_RESIDUE, set_RDD_RATE_LIMITER_PROFILE_RESIDUE, 0xf20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DS_TM_TM_FLOW_CNTR_TABLE_11 =
{
	1,
	{
		{ dump_RDD_TM_FLOW_CNTR_ENTRY, set_RDD_TM_FLOW_CNTR_ENTRY, 0xf40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TX_TASK_0_STACK_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING_4_TASKS_PACKET_BUFFER_11 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, set_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_RATE_LIMITER_BUDGET_DESCRIPTOR_TABLE_11 =
{
	8,
	{
		{ dump_RDD_RATE_LIMITER_BUDGET_DESCRIPTOR, set_RDD_RATE_LIMITER_BUDGET_DESCRIPTOR, 0x1800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_RATE_LIMITER_PROFILE_TABLE_11 =
{
	4,
	{
		{ dump_RDD_RATE_LIMITER_PROFILE, set_RDD_RATE_LIMITER_PROFILE, 0x1a80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_11 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, set_RDD_RX_FLOW_ENTRY, 0x1b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_RATE_LIMITER_PARAMS_DESCRIPTOR_TABLE_11 =
{
	8,
	{
		{ dump_RDD_RATE_LIMITER_PARAMS_DESCRIPTOR, set_RDD_RATE_LIMITER_PARAMS_DESCRIPTOR, 0x1c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_DCL_IMG_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x1e80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_11 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0x1f00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SCORE_COLLECT_DROP_BITMAP_TABLE_11 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x1fd4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DS_TM_BBH_TX_WAKE_UP_DATA_TABLE_11 =
{
	8,
	{
		{ dump_RDD_BBH_TX_WAKE_UP_DATA_ENTRY, set_RDD_BBH_TX_WAKE_UP_DATA_ENTRY, 0x1fd8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_11 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x1fe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_11 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SCORE_CFG_TABLE_11 =
{
	8,
	{
		{ dump_RDD_SCORE_CFG_ENTRY, set_RDD_SCORE_CFG_ENTRY, 0x2368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_RATE_LIMITER_BUDGET_VALID_11 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SCORE_DATA_COLLECTION_PROFILE_TABLE_11 =
{
	16,
	{
		{ dump_RDD_SCORE_DATA_COLLECTION_PROFILE, set_RDD_SCORE_DATA_COLLECTION_PROFILE, 0x2380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_11 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x2568 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_11 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, set_RDD_SPDTEST_GEN_CFG, 0x2570 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_11 =
{
	8,
	{
		{ dump_RDD_GENERAL_TIMER_ENTRY, set_RDD_GENERAL_TIMER_ENTRY, 0x2580 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT MIRRORING_SCRATCH_11 =
{
	8,
	{
		{ dump_RDD_BYTES_8, set_RDD_BYTES_8, 0x2768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2770 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_11 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, set_RDD_VPORT_CFG_ENTRY, 0x2780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_11 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x27fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_11 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, set_RDD_TUNNELS_PARSING_CFG, 0x2968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_11 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x2970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_11 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x297c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_SECONDARY_SCHEDULER_TABLE_11 =
{
	12,
	{
		{ dump_RDD_SECONDARY_SCHEDULER_DESCRIPTOR, set_RDD_SECONDARY_SCHEDULER_DESCRIPTOR, 0x2980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CSO_BAD_IPV4_HDR_CSUM_PACKETS_11 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x29d4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_11 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, set_RDD_IPTV_CFG, 0x29d8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_11 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0x29e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_AQM_QUEUE_TABLE_11 =
{
	4,
	{
		{ dump_RDD_AQM_QUEUE_DESCRIPTOR, set_RDD_AQM_QUEUE_DESCRIPTOR, 0x2a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_SCHEDULER_POOL_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2b40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_11 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, set_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0x2be0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_SCHEDULER_TABLE_11 =
{
	24,
	{
		{ dump_RDD_SCHEDULER_DESCRIPTOR, set_RDD_SCHEDULER_DESCRIPTOR, 0x2c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_11 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, set_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x2ca8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_11 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2cb0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_DCL_MULTIPLE_CNTR_11 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2cbc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT UPDATE_FIFO_STACK_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2cc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_11 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, set_RDD_INGRESS_FILTER_CTRL, 0x2d00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_AQM_QUEUE_TIMER_TABLE_11 =
{
	1,
	{
		{ dump_RDD_AQM_QUEUE_TIMER_DESCRIPTOR, set_RDD_AQM_QUEUE_TIMER_DESCRIPTOR, 0x2d80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_DISPATCHER_CREDIT_TABLE_11 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2dd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TASK_IDX_11 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2ddc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_11 =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, set_RDD_BUFMNG_STATUS_ENTRY, 0x2de0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_STACK_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_11 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2e48 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DS_TM_CPU_TX_ABS_COUNTERS_11 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2ec8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_11 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, set_RDD_NAT_CACHE_CFG, 0x2f48 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2f4f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_11 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0x2f50 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_FW_FLUSH_TABLE_11 =
{
	28,
	{
		{ dump_RDD_FW_FLUSH_ENTRY, set_RDD_FW_FLUSH_ENTRY, 0x2f60 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUDGET_ALLOCATION_TIMER_VALUE_11 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2f7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_11 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2f7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFFER_CONG_SCRATCHPAD_11 =
{
	8,
	{
		{ dump_RDD_BUFFER_CONG_Q_OCCUPANCY, set_RDD_BUFFER_CONG_Q_OCCUPANCY, 0x2f80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_TABLE_11 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, set_RDD_MIRRORING_TRUNCATE_ENTRY, 0x2fc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_ACTION_VEC_11 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2ffe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT HOST_TX_TRUNCATE_MIRRORING_TABLE_11 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, set_RDD_MIRRORING_TRUNCATE_ENTRY, 0x3000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_11 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x303e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_TX_TRUNCATE_MIRRORING_TABLE_11 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, set_RDD_MIRRORING_TRUNCATE_ENTRY, 0x3040 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DS_TM_BB_DESTINATION_TABLE_11 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x307e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_11 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x3080 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x30be },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x30bf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_11 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, set_RDD_VPORT_CFG_EX_ENTRY, 0x30c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DS_TM_FIRST_QUEUE_MAPPING_11 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x30fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFFER_CONG_DQM_NOT_EMPTY_11 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3100 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3134 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT LLQ_SELECTOR_ECN_MASK_11 =
{
	1,
	{
		{ dump_RDD_LLQ_SELECTOR_ECN, set_RDD_LLQ_SELECTOR_ECN, 0x3135 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TX_EXCEPTION_11 =
{
	1,
	{
		{ dump_RDD_TX_EXCEPTION_ENTRY, set_RDD_TX_EXCEPTION_ENTRY, 0x3136 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3137 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_11 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3138 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_11 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x313c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_11 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x313e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_TABLE_11 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, set_RDD_TR471_SPDSVC_RX_PKT_ID, 0x3140 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_11 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, set_RDD_TCAM_GENERIC, 0x3170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DS_BUFFER_CONG_MGT_CFG_11 =
{
	48,
	{
		{ dump_RDD_BUFFER_CONG_MGT, set_RDD_BUFFER_CONG_MGT, 0x3180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_11 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, set_RDD_GDX_PARAMS, 0x31b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x31bc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BACKUP_BBH_INGRESS_COUNTERS_TABLE_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x31bd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BACKUP_BBH_EGRESS_COUNTERS_TABLE_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x31be },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CSO_DISABLE_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x31bf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_ENABLED_QUEUES_TABLE_11 =
{
	34,
	{
		{ dump_RDD_ENABLED_QUEUES_ENTRY, set_RDD_ENABLED_QUEUES_ENTRY, 0x31c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_11 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, set_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x31e2 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x31e3 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x31e4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_11 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, set_RDD_TCAM_TABLE_CFG, 0x31e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_11 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x31f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_11 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, set_RDD_INGRESS_FILTER_CFG, 0x31f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DS_TM_BBH_QUEUE_TABLE_11 =
{
	8,
	{
		{ dump_RDD_BBH_QUEUE_DESCRIPTOR, set_RDD_BBH_QUEUE_DESCRIPTOR, 0x3200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3238 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DS_TM_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_TABLE_11 =
{
	40,
	{
		{ dump_RDD_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_ENTRY, set_RDD_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_ENTRY, 0x3240 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_11 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3290 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT UPDATE_FIFO_TABLE_11 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, set_RDD_UPDATE_FIFO_ENTRY, 0x32c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DS_TM_PI2_PROBABILITY_CALC_DESCRIPTOR_11 =
{
	20,
	{
		{ dump_RDD_PI2_PROBABILITY_CALC_DESCRIPTOR, set_RDD_PI2_PROBABILITY_CALC_DESCRIPTOR, 0x32e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_DCL_DATA_BASE_11 =
{
	32,
	{
		{ dump_RDD_DCL_DATA, set_RDD_DCL_DATA, 0x3300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_11 =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, set_RDD_DHD_HW_CONFIGURATION, 0x3320 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_11 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0x3340 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_11 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0x3360 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_11 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x33a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_11 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x33c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_11 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, set_RDD_MULTICAST_KEY_MASK_ENTRY, 0x33e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DS_TM_PD_FIFO_TABLE_12 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, set_RDD_PROCESSING_TX_DESCRIPTOR, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_12 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, set_RDD_POLICER_PARAMS_ENTRY, 0xa00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_SCHEDULING_QUEUE_TABLE_12 =
{
	10,
	{
		{ dump_RDD_SCHEDULING_QUEUE_DESCRIPTOR, set_RDD_SCHEDULING_QUEUE_DESCRIPTOR, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_RATE_LIMITER_PROFILE_RESIDUE_TABLE_12 =
{
	1,
	{
		{ dump_RDD_RATE_LIMITER_PROFILE_RESIDUE, set_RDD_RATE_LIMITER_PROFILE_RESIDUE, 0xf20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DS_TM_TM_FLOW_CNTR_TABLE_12 =
{
	1,
	{
		{ dump_RDD_TM_FLOW_CNTR_ENTRY, set_RDD_TM_FLOW_CNTR_ENTRY, 0xf40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TX_TASK_0_STACK_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING_4_TASKS_PACKET_BUFFER_12 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, set_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT REPORTING_QUEUE_DESCRIPTOR_TABLE_12 =
{
	8,
	{
		{ dump_RDD_REPORTING_QUEUE_DESCRIPTOR, set_RDD_REPORTING_QUEUE_DESCRIPTOR, 0x1800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_12 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x1c20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT UPDATE_FIFO_STACK_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x1c40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_RATE_LIMITER_PROFILE_TABLE_12 =
{
	4,
	{
		{ dump_RDD_RATE_LIMITER_PROFILE, set_RDD_RATE_LIMITER_PROFILE, 0x1c80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_12 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, set_RDD_RX_FLOW_ENTRY, 0x1d00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x1e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DS_TM_BBH_TX_WAKE_UP_DATA_TABLE_12 =
{
	8,
	{
		{ dump_RDD_BBH_TX_WAKE_UP_DATA_ENTRY, set_RDD_BBH_TX_WAKE_UP_DATA_ENTRY, 0x1f68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_RATE_LIMITER_BUDGET_VALID_12 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x1f70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_DCL_IMG_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x1f80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_RATE_LIMITER_BUDGET_DESCRIPTOR_TABLE_12 =
{
	8,
	{
		{ dump_RDD_RATE_LIMITER_BUDGET_DESCRIPTOR, set_RDD_RATE_LIMITER_BUDGET_DESCRIPTOR, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SCORE_DATA_COLLECTION_PROFILE_TABLE_12 =
{
	16,
	{
		{ dump_RDD_SCORE_DATA_COLLECTION_PROFILE, set_RDD_SCORE_DATA_COLLECTION_PROFILE, 0x2280 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT REPORTING_QUEUE_ACCUMULATED_TABLE_12 =
{
	16,
	{
		{ dump_RDD_QM_QUEUE_COUNTER_DATA, set_RDD_QM_QUEUE_COUNTER_DATA, 0x2300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_RATE_LIMITER_PARAMS_DESCRIPTOR_TABLE_12 =
{
	8,
	{
		{ dump_RDD_RATE_LIMITER_PARAMS_DESCRIPTOR, set_RDD_RATE_LIMITER_PARAMS_DESCRIPTOR, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_12 =
{
	8,
	{
		{ dump_RDD_GENERAL_TIMER_ENTRY, set_RDD_GENERAL_TIMER_ENTRY, 0x2680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_12 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0x2700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SCORE_COLLECT_DROP_BITMAP_TABLE_12 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x27d4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SCORE_CFG_TABLE_12 =
{
	8,
	{
		{ dump_RDD_SCORE_CFG_ENTRY, set_RDD_SCORE_CFG_ENTRY, 0x27d8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_12 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0x27e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_12 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_12 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x2b68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_12 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, set_RDD_SPDTEST_GEN_CFG, 0x2b70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_12 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, set_RDD_VPORT_CFG_ENTRY, 0x2b80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_12 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2bfc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT MIRRORING_SCRATCH_12 =
{
	8,
	{
		{ dump_RDD_BYTES_8, set_RDD_BYTES_8, 0x2d68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2d70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_SECONDARY_SCHEDULER_TABLE_12 =
{
	12,
	{
		{ dump_RDD_SECONDARY_SCHEDULER_DESCRIPTOR, set_RDD_SECONDARY_SCHEDULER_DESCRIPTOR, 0x2d80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_12 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2dd4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT REPORT_BBH_TX_QUEUE_ID_TABLE_12 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2dd8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_12 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, set_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0x2de0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_12 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, set_RDD_TUNNELS_PARSING_CFG, 0x2f68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_12 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x2f70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CSO_BAD_IPV4_HDR_CSUM_PACKETS_12 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2f7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_AQM_QUEUE_TIMER_TABLE_12 =
{
	1,
	{
		{ dump_RDD_AQM_QUEUE_TIMER_DESCRIPTOR, set_RDD_AQM_QUEUE_TIMER_DESCRIPTOR, 0x2f80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_12 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2fd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GHOST_REPORTING_GLOBAL_CFG_12 =
{
	4,
	{
		{ dump_RDD_GHOST_REPORTING_GLOBAL_CFG_ENTRY, set_RDD_GHOST_REPORTING_GLOBAL_CFG_ENTRY, 0x2fdc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_12 =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, set_RDD_BUFMNG_STATUS_ENTRY, 0x2fe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_AQM_QUEUE_TABLE_12 =
{
	4,
	{
		{ dump_RDD_AQM_QUEUE_DESCRIPTOR, set_RDD_AQM_QUEUE_DESCRIPTOR, 0x3000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_SCHEDULER_POOL_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3140 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT REPORTING_STACK_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x31e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT REPORTING_QUEUE_COUNTER_TABLE_12 =
{
	2,
	{
		{ dump_RDD_REPORTING_QUEUE_COUNTER, set_RDD_REPORTING_QUEUE_COUNTER, 0x3200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_12 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, set_RDD_IPTV_CFG, 0x3308 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_DISPATCHER_CREDIT_TABLE_12 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3310 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_DCL_MULTIPLE_CNTR_12 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x331c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_FW_FLUSH_TABLE_12 =
{
	28,
	{
		{ dump_RDD_FW_FLUSH_ENTRY, set_RDD_FW_FLUSH_ENTRY, 0x3320 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TASK_IDX_12 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x333c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFFER_CONG_SCRATCHPAD_12 =
{
	8,
	{
		{ dump_RDD_BUFFER_CONG_Q_OCCUPANCY, set_RDD_BUFFER_CONG_Q_OCCUPANCY, 0x3340 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_STACK_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_12 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x33c8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_12 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, set_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x33f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_SCHEDULER_TABLE_12 =
{
	24,
	{
		{ dump_RDD_SCHEDULER_DESCRIPTOR, set_RDD_SCHEDULER_DESCRIPTOR, 0x3400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BBH_TX_EGRESS_REPORT_COUNTER_TABLE_12 =
{
	8,
	{
		{ dump_RDD_BBH_TX_EGRESS_COUNTER_ENTRY, set_RDD_BBH_TX_EGRESS_COUNTER_ENTRY, 0x34a8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_12 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0x34b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_TABLE_12 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, set_RDD_MIRRORING_TRUNCATE_ENTRY, 0x34c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUDGET_ALLOCATION_TIMER_VALUE_12 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x34fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT REPORTING_COUNTER_TABLE_12 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DS_TM_PI2_PROBABILITY_CALC_DESCRIPTOR_12 =
{
	20,
	{
		{ dump_RDD_PI2_PROBABILITY_CALC_DESCRIPTOR, set_RDD_PI2_PROBABILITY_CALC_DESCRIPTOR, 0x35a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_12 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x35b4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_ACTION_VEC_12 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x35b6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BBH_TX_INGRESS_COUNTER_TABLE_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x35b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT HOST_TX_TRUNCATE_MIRRORING_TABLE_12 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, set_RDD_MIRRORING_TRUNCATE_ENTRY, 0x35c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_12 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x35fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_12 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, set_RDD_INGRESS_FILTER_CTRL, 0x3600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_12 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DS_TM_CPU_TX_ABS_COUNTERS_12 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_TX_TRUNCATE_MIRRORING_TABLE_12 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, set_RDD_MIRRORING_TRUNCATE_ENTRY, 0x3780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DS_TM_BB_DESTINATION_TABLE_12 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x37be },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_12 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x37c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x37fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x37ff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_12 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, set_RDD_VPORT_CFG_EX_ENTRY, 0x3800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DS_TM_FIRST_QUEUE_MAPPING_12 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x383e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFFER_CONG_DQM_NOT_EMPTY_12 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3840 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3874 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3875 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT LLQ_SELECTOR_ECN_MASK_12 =
{
	1,
	{
		{ dump_RDD_LLQ_SELECTOR_ECN, set_RDD_LLQ_SELECTOR_ECN, 0x3876 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TX_EXCEPTION_12 =
{
	1,
	{
		{ dump_RDD_TX_EXCEPTION_ENTRY, set_RDD_TX_EXCEPTION_ENTRY, 0x3877 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_12 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, set_RDD_NAT_CACHE_CFG, 0x3878 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x387f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_TABLE_12 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, set_RDD_TR471_SPDSVC_RX_PKT_ID, 0x3880 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_12 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, set_RDD_TCAM_GENERIC, 0x38b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DS_BUFFER_CONG_MGT_CFG_12 =
{
	48,
	{
		{ dump_RDD_BUFFER_CONG_MGT, set_RDD_BUFFER_CONG_MGT, 0x38c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_12 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, set_RDD_GDX_PARAMS, 0x38f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_12 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x38fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_12 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x38fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DS_TM_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_TABLE_12 =
{
	40,
	{
		{ dump_RDD_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_ENTRY, set_RDD_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_ENTRY, 0x3900 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT XGPON_REPORT_ZERO_SENT_TABLE_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3950 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x395a },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BACKUP_BBH_INGRESS_COUNTERS_TABLE_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x395b },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BACKUP_BBH_EGRESS_COUNTERS_TABLE_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x395c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CSO_DISABLE_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x395d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_12 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, set_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x395e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x395f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_12 =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, set_RDD_DHD_HW_CONFIGURATION, 0x3960 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3974 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_12 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3978 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_ENABLED_QUEUES_TABLE_12 =
{
	34,
	{
		{ dump_RDD_ENABLED_QUEUES_ENTRY, set_RDD_ENABLED_QUEUES_ENTRY, 0x3980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_12 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, set_RDD_TCAM_TABLE_CFG, 0x39a8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_12 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x39b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_12 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, set_RDD_INGRESS_FILTER_CFG, 0x39b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT UPDATE_FIFO_TABLE_12 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, set_RDD_UPDATE_FIFO_ENTRY, 0x39c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GHOST_REPORTING_QUEUE_STATUS_BIT_VECTOR_TABLE_12 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x39e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x39f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DS_TM_BBH_QUEUE_TABLE_12 =
{
	8,
	{
		{ dump_RDD_BBH_QUEUE_DESCRIPTOR, set_RDD_BBH_QUEUE_DESCRIPTOR, 0x3a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_12 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0x3a40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT QUEUE_TO_REPORT_BIT_VECTOR_12 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3a60 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT ETH_TM_DCL_DATA_BASE_12 =
{
	32,
	{
		{ dump_RDD_DCL_DATA, set_RDD_DCL_DATA, 0x3a80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_12 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0x3aa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3ac0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_12 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3ae0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_12 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_12 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, set_RDD_MULTICAST_KEY_MASK_ENTRY, 0x3b20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3b40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT US_TM_PD_FIFO_TABLE_13 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, set_RDD_PROCESSING_TX_DESCRIPTOR, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PON_TM_SCHEDULER_POOL_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x1480 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_13 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x15b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT UPDATE_FIFO_STACK_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x15c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PON_TM_SECONDARY_SCHEDULER_TABLE_13 =
{
	12,
	{
		{ dump_RDD_SECONDARY_SCHEDULER_DESCRIPTOR, set_RDD_SECONDARY_SCHEDULER_DESCRIPTOR, 0x1600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT WAN_STACK_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x1780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PON_TM_SCHEDULING_QUEUE_TABLE_13 =
{
	10,
	{
		{ dump_RDD_SCHEDULING_QUEUE_DESCRIPTOR, set_RDD_SCHEDULING_QUEUE_DESCRIPTOR, 0x1800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT US_TM_WAN_0_BBH_TX_WAKE_UP_DATA_TABLE_13 =
{
	8,
	{
		{ dump_RDD_BBH_TX_WAKE_UP_DATA_ENTRY, set_RDD_BBH_TX_WAKE_UP_DATA_ENTRY, 0x1d28 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PON_TM_RATE_LIMITER_BUDGET_VALID_13 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x1d30 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFFER_CONG_SCRATCHPAD_13 =
{
	8,
	{
		{ dump_RDD_BUFFER_CONG_Q_OCCUPANCY, set_RDD_BUFFER_CONG_Q_OCCUPANCY, 0x1d40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT WAN_EPON_STACK_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x1d80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT US_TM_BBH_QUEUE_TABLE_13 =
{
	8,
	{
		{ dump_RDD_BBH_QUEUE_DESCRIPTOR, set_RDD_BBH_QUEUE_DESCRIPTOR, 0x1e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT HOST_TX_TRUNCATE_MIRRORING_TABLE_13 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, set_RDD_MIRRORING_TRUNCATE_ENTRY, 0x1f40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PON_TM_TX_TRUNCATE_MIRRORING_TABLE_13 =
{
	2,
	{
		{ dump_RDD_MIRRORING_TRUNCATE_ENTRY, set_RDD_MIRRORING_TRUNCATE_ENTRY, 0x1f7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PON_TM_RATE_LIMITER_PROFILE_TABLE_13 =
{
	4,
	{
		{ dump_RDD_RATE_LIMITER_PROFILE, set_RDD_RATE_LIMITER_PROFILE, 0x1f80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PON_TM_RATE_LIMITER_BUDGET_DESCRIPTOR_TABLE_13 =
{
	8,
	{
		{ dump_RDD_RATE_LIMITER_BUDGET_DESCRIPTOR, set_RDD_RATE_LIMITER_BUDGET_DESCRIPTOR, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PON_TM_RATE_LIMITER_PROFILE_RESIDUE_TABLE_13 =
{
	1,
	{
		{ dump_RDD_RATE_LIMITER_PROFILE_RESIDUE, set_RDD_RATE_LIMITER_PROFILE_RESIDUE, 0x2420 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_13 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, set_RDD_VPORT_CFG_EX_ENTRY, 0x2440 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUDGET_ALLOCATION_TIMER_VALUE_13 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x247e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PON_TM_DCL_IMG_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2480 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DIRECT_FLOW_RX_MIRRORING_SCRATCHPAD_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BBH_TX_EPON_WAKE_UP_DATA_TABLE_13 =
{
	8,
	{
		{ dump_RDD_BBH_TX_WAKE_UP_DATA_ENTRY, set_RDD_BBH_TX_WAKE_UP_DATA_ENTRY, 0x2588 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT NULL_BUFFER_13 =
{
	8,
	{
		{ dump_RDD_BYTES_8, set_RDD_BYTES_8, 0x2590 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_13 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x25a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFFER_CONG_DQM_NOT_EMPTY_13 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x25c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_13 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x25f4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT MIRRORING_SCRATCH_13 =
{
	8,
	{
		{ dump_RDD_BYTES_8, set_RDD_BYTES_8, 0x25f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FPM_RING_BUFMGT_CNT_TABLE_13 =
{
	264,
	{
		{ dump_RDD_FPM_RING_BUFMNG_CNT, set_RDD_FPM_RING_BUFMNG_CNT, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_13 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2708 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TASK_IDX_13 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x270c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DIRECT_FLOW_PD_TABLE_13 =
{
	16,
	{
		{ dump_RDD_PROCESSING_RX_DESCRIPTOR, set_RDD_PROCESSING_RX_DESCRIPTOR, 0x2710 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_13 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0x2720 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT US_BUFFER_CONG_MGT_CFG_13 =
{
	48,
	{
		{ dump_RDD_BUFFER_CONG_MGT, set_RDD_BUFFER_CONG_MGT, 0x2740 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PON_TM_DISPATCHER_CREDIT_TABLE_13 =
{
	12,
	{
		{ dump_RDD_DISPATCHER_CREDIT_DESCRIPTOR, set_RDD_DISPATCHER_CREDIT_DESCRIPTOR, 0x2770 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_ACTION_VEC_13 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x277c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DIRECT_FLOW_PAUSE_QUANTA_13 =
{
	2,
	{
		{ dump_RDD_PAUSE_QUANTA_ENTRY, set_RDD_PAUSE_QUANTA_ENTRY, 0x277e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT US_TM_TM_FLOW_CNTR_TABLE_13 =
{
	1,
	{
		{ dump_RDD_TM_FLOW_CNTR_ENTRY, set_RDD_TM_FLOW_CNTR_ENTRY, 0x2780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PON_TM_RATE_LIMITER_PARAMS_DESCRIPTOR_TABLE_13 =
{
	8,
	{
		{ dump_RDD_RATE_LIMITER_PARAMS_DESCRIPTOR, set_RDD_RATE_LIMITER_PARAMS_DESCRIPTOR, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT EPON_UPDATE_FIFO_STACK_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2c20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT US_TM_WAN_0_BBH_TX_EGRESS_COUNTER_TABLE_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2c40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT US_TM_BBH_TX_WAN_0_FIFO_BYTES_USED_13 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2c68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT US_TM_WAN_0_BB_DESTINATION_TABLE_13 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2c6c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT QEMU_SYNC_MEM_13 =
{
	1,
	{
		{ dump_RDD_QEMU_DATA, set_RDD_QEMU_DATA, 0x2c6e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DIRECT_FLOW_WAN_VIQ_EXCLUSIVE_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2c6f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DISPATCHER_CREDIT_TABLE_13 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2c70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PON_TM_VPORT_TO_RL_OVERHEAD_TABLE_13 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0x2c7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2c7d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT US_TM_FIRST_QUEUE_MAPPING_13 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2c7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_13 =
{
	8,
	{
		{ dump_RDD_GENERAL_TIMER_ENTRY, set_RDD_GENERAL_TIMER_ENTRY, 0x2c80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PON_TM_AQM_QUEUE_TIMER_TABLE_13 =
{
	1,
	{
		{ dump_RDD_AQM_QUEUE_TIMER_DESCRIPTOR, set_RDD_AQM_QUEUE_TIMER_DESCRIPTOR, 0x2d00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT US_TM_TX_PAUSE_NACK_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2d84 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TX_EXCEPTION_13 =
{
	1,
	{
		{ dump_RDD_TX_EXCEPTION_ENTRY, set_RDD_TX_EXCEPTION_ENTRY, 0x2d85 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BBH_TX_US_WAN_0_FIFO_BYTES_THRESHOLD_13 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2d86 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_13 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x2d88 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_13 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x2d8a },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2d8c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2d8d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT MAC_TYPE_13 =
{
	1,
	{
		{ dump_RDD_MAC_TYPE_ENTRY, set_RDD_MAC_TYPE_ENTRY, 0x2d8e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT WAN_0_BBH_TX_FIFO_SIZE_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2d8f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_13 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x2d90 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2d9c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DIRECT_FLOW_CNTR_TABLE_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2da0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BBH_TX_EPON_INGRESS_COUNTER_TABLE_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2dc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DIRECT_FLOW_PAUSE_DEBUG_13 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2df0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT CPU_FPM_BUFMNG_RELEASE_STACK_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DIRECT_FLOW_STACK_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2e80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_13 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, set_RDD_VPORT_CFG_ENTRY, 0x2f00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_STACK_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2f80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_DISPATCHER_CREDIT_TABLE_13 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2fd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PON_TM_FW_FLUSH_TABLE_13 =
{
	28,
	{
		{ dump_RDD_FW_FLUSH_ENTRY, set_RDD_FW_FLUSH_ENTRY, 0x2fe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PON_TM_SCHEDULER_TABLE_13 =
{
	24,
	{
		{ dump_RDD_SCHEDULER_DESCRIPTOR, set_RDD_SCHEDULER_DESCRIPTOR, 0x3000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PON_TM_ENABLED_QUEUES_TABLE_13 =
{
	34,
	{
		{ dump_RDD_ENABLED_QUEUES_ENTRY, set_RDD_ENABLED_QUEUES_ENTRY, 0x33c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_13 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0x33f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PON_TM_AQM_QUEUE_TABLE_13 =
{
	4,
	{
		{ dump_RDD_AQM_QUEUE_DESCRIPTOR, set_RDD_AQM_QUEUE_DESCRIPTOR, 0x3400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_13 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3610 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT US_TM_CPU_TX_ABS_COUNTERS_13 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3690 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_13 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3710 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT BBH_TX_EPON_EGRESS_COUNTER_TABLE_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3740 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT DIRECT_FLOW_EPON_CONTROL_SCRATCH_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3760 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT PON_TM_DCL_DATA_BASE_13 =
{
	32,
	{
		{ dump_RDD_DCL_DATA, set_RDD_DCL_DATA, 0x3780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT US_TM_PI2_PROBABILITY_CALC_DESCRIPTOR_13 =
{
	20,
	{
		{ dump_RDD_PI2_PROBABILITY_CALC_DESCRIPTOR, set_RDD_PI2_PROBABILITY_CALC_DESCRIPTOR, 0x37a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT UPDATE_FIFO_TABLE_13 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, set_RDD_UPDATE_FIFO_ENTRY, 0x37c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_13 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0x37e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_13 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0x3800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT EPON_UPDATE_FIFO_TABLE_13 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, set_RDD_UPDATE_FIFO_ENTRY, 0x3a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT US_TM_TX_OCTETS_COUNTERS_TABLE_13 =
{
	8,
	{
		{ dump_RDD_PACKETS_AND_BYTES, set_RDD_PACKETS_AND_BYTES, 0x3a20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_13 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3a60 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM68375
static DUMP_RUNNERREG_STRUCT US_TM_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_TABLE_13 =
{
	40,
	{
		{ dump_RDD_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_ENTRY, set_RDD_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_ENTRY, 0x3a80 },
		{ 0, 0 },
	}
};
#endif

TABLE_STRUCT RUNNER_TABLES[NUMBER_OF_TABLES] =
{
#if defined BCM68375
	{ "DHD_COMPLETE_COMMON_RADIO_DATA", 1, CORE_0_INDEX, &DHD_COMPLETE_COMMON_RADIO_DATA, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_FLOW_TABLE", 1, CORE_0_INDEX, &RX_FLOW_TABLE, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "POLICER_PARAMS_TABLE", 1, CORE_0_INDEX, &POLICER_PARAMS_TABLE, 256, 1, 1 },
#endif
#if defined BCM68375
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_0_INDEX, &RUNNER_PROFILING_TRACE_BUFFER, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING0_STACK", 1, CORE_0_INDEX, &PROCESSING0_STACK, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "SCORE_CFG_TABLE", 1, CORE_0_INDEX, &SCORE_CFG_TABLE, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SPDTEST_GEN_PARAM", 1, CORE_0_INDEX, &SPDTEST_GEN_PARAM, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "SCORE_DATA_COLLECTION_PROFILE_TABLE", 1, CORE_0_INDEX, &SCORE_DATA_COLLECTION_PROFILE_TABLE, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING1_STACK", 1, CORE_0_INDEX, &PROCESSING1_STACK, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "SYSTEM_CONFIGURATION", 1, CORE_0_INDEX, &SYSTEM_CONFIGURATION, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_DOORBELL_TX_COMPLETE_VALUE", 1, CORE_0_INDEX, &DHD_DOORBELL_TX_COMPLETE_VALUE, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_RX_COMPLETE_FLOW_RING_BUFFER", 1, CORE_0_INDEX, &DHD_RX_COMPLETE_FLOW_RING_BUFFER, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING2_STACK", 1, CORE_0_INDEX, &PROCESSING2_STACK, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR", 1, CORE_0_INDEX, &DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_DOORBELL_RX_COMPLETE_VALUE", 1, CORE_0_INDEX, &DHD_DOORBELL_RX_COMPLETE_VALUE, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_RX_POST_FLOW_RING_BUFFER", 1, CORE_0_INDEX, &DHD_RX_POST_FLOW_RING_BUFFER, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_0_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING3_STACK", 1, CORE_0_INDEX, &PROCESSING3_STACK, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_RX_POST_RING_SIZE", 1, CORE_0_INDEX, &DHD_RX_POST_RING_SIZE, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_DOORBELL_RX_POST_VALUE", 1, CORE_0_INDEX, &DHD_DOORBELL_RX_POST_VALUE, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_RX_COMPLETE_0_STACK", 1, CORE_0_INDEX, &DHD_RX_COMPLETE_0_STACK, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING4_STACK", 1, CORE_0_INDEX, &PROCESSING4_STACK, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_RX_COMPLETE_RING_SIZE", 1, CORE_0_INDEX, &DHD_RX_COMPLETE_RING_SIZE, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_CPU_INT_ID", 1, CORE_0_INDEX, &DHD_CPU_INT_ID, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_RX_COMPLETE_1_STACK", 1, CORE_0_INDEX, &DHD_RX_COMPLETE_1_STACK, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING5_STACK", 1, CORE_0_INDEX, &PROCESSING5_STACK, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_TX_COMPLETE_RING_SIZE", 1, CORE_0_INDEX, &DHD_TX_COMPLETE_RING_SIZE, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_PCAP_CFG", 1, CORE_0_INDEX, &DHD_PCAP_CFG, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_RX_COMPLETE_2_STACK", 1, CORE_0_INDEX, &DHD_RX_COMPLETE_2_STACK, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING6_STACK", 1, CORE_0_INDEX, &PROCESSING6_STACK, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "SCORE_COLLECT_DROP_BITMAP_TABLE", 1, CORE_0_INDEX, &SCORE_COLLECT_DROP_BITMAP_TABLE, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "MUTEX_TABLE", 1, CORE_0_INDEX, &MUTEX_TABLE, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_0_INDEX, &INGRESS_FILTER_L2_REASON_TABLE, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_CFG_TABLE", 1, CORE_0_INDEX, &VPORT_CFG_TABLE, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_0_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING7_STACK", 1, CORE_0_INDEX, &PROCESSING7_STACK, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "REGISTERS_BUFFER", 1, CORE_0_INDEX, &REGISTERS_BUFFER, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "TUNNELS_PARSING_CFG", 1, CORE_0_INDEX, &TUNNELS_PARSING_CFG, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_0_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "CSO_BAD_IPV4_HDR_CSUM_PACKETS", 1, CORE_0_INDEX, &CSO_BAD_IPV4_HDR_CSUM_PACKETS, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_MONITOR_PKT_INFO_TABLE", 1, CORE_0_INDEX, &DHD_MONITOR_PKT_INFO_TABLE, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_0_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "TX_FLOW_TABLE", 1, CORE_0_INDEX, &TX_FLOW_TABLE, 212, 1, 1 },
#endif
#if defined BCM68375
	{ "TASK_IDX", 1, CORE_0_INDEX, &TASK_IDX, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_MIRRORING_MINIFPM_PARAMS", 1, CORE_0_INDEX, &DHD_MIRRORING_MINIFPM_PARAMS, 2, 1, 1 },
#endif
#if defined BCM68375
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_0_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_0_INDEX, &INGRESS_FILTER_PROFILE_TABLE, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_PCAP_TX_RING_DESCRIPTOR_TABLE", 1, CORE_0_INDEX, &DHD_PCAP_TX_RING_DESCRIPTOR_TABLE, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_RX_COMPLETE_DISPATCHER_CREDIT_TABLE", 1, CORE_0_INDEX, &DHD_RX_COMPLETE_DISPATCHER_CREDIT_TABLE, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_PCAP_RX_RING_DESCRIPTOR_TABLE", 1, CORE_0_INDEX, &DHD_PCAP_RX_RING_DESCRIPTOR_TABLE, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_TX_COMPLETE_DISPATCHER_CREDIT_TABLE", 1, CORE_0_INDEX, &DHD_TX_COMPLETE_DISPATCHER_CREDIT_TABLE, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE", 1, CORE_0_INDEX, &DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_TX_COMPLETE_0_STACK", 1, CORE_0_INDEX, &DHD_TX_COMPLETE_0_STACK, 64, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_TX_COMPLETE_FLOW_RING_BUFFER", 1, CORE_0_INDEX, &DHD_TX_COMPLETE_FLOW_RING_BUFFER, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_TX_COMPLETE_1_STACK", 1, CORE_0_INDEX, &DHD_TX_COMPLETE_1_STACK, 64, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_TX_COMPLETE_2_STACK", 1, CORE_0_INDEX, &DHD_TX_COMPLETE_2_STACK, 64, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_MIRRORING_TABLE", 1, CORE_0_INDEX, &RX_MIRRORING_TABLE, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "DOS_DROP_REASONS_CFG", 1, CORE_0_INDEX, &DOS_DROP_REASONS_CFG, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "HOST_TX_TRUNCATE_MIRRORING_TABLE", 1, CORE_0_INDEX, &HOST_TX_TRUNCATE_MIRRORING_TABLE, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_0_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_0_INDEX, &LOOPBACK_QUEUE_TABLE, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_0_INDEX, &SPDTEST_NUM_OF_RX_FLOWS, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_0_INDEX, &LOOPBACK_WAN_FLOW_TABLE, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_CFG_EX_TABLE", 1, CORE_0_INDEX, &VPORT_CFG_EX_TABLE, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "FORCE_DSCP", 1, CORE_0_INDEX, &FORCE_DSCP, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "CORE_ID_TABLE", 1, CORE_0_INDEX, &CORE_ID_TABLE, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TR471_SPDSVC_RX_PKT_ID_TABLE", 1, CORE_0_INDEX, &TR471_SPDSVC_RX_PKT_ID_TABLE, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_FPM_THRESHOLDS", 1, CORE_0_INDEX, &DHD_FPM_THRESHOLDS, 3, 1, 1 },
#endif
#if defined BCM68375
	{ "LLQ_SELECTOR_ECN_MASK", 1, CORE_0_INDEX, &LLQ_SELECTOR_ECN_MASK, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_0_INDEX, &DEBUG_PRINT_CORE_LOCK, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_0_INDEX, &RX_MIRRORING_CONFIGURATION, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_0_INDEX, &BUFMNG_DESCRIPTOR_TABLE, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFMNG_STATUS_TABLE", 1, CORE_0_INDEX, &BUFMNG_STATUS_TABLE, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_FPM_REPLY", 1, CORE_0_INDEX, &DHD_FPM_REPLY, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_HW_CFG", 1, CORE_0_INDEX, &DHD_HW_CFG, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_0_INDEX, &TX_MIRRORING_CONFIGURATION, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SRAM_DUMMY_STORE", 1, CORE_0_INDEX, &SRAM_DUMMY_STORE, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "CSO_DISABLE", 1, CORE_0_INDEX, &CSO_DISABLE, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "IPTV_CFG_TABLE", 1, CORE_0_INDEX, &IPTV_CFG_TABLE, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_0_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_0_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "FPM_GLOBAL_CFG", 1, CORE_0_INDEX, &FPM_GLOBAL_CFG, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_0_INDEX, &INGRESS_FILTER_1588_CFG, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_0_INDEX, &RX_MIRRORING_DIRECT_ENABLE, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_0_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "DEBUG_SCRATCHPAD", 1, CORE_0_INDEX, &DEBUG_SCRATCHPAD, 12, 1, 1 },
#endif
#if defined BCM68375
	{ "SPDSVC_WLAN_GEN_PARAMS_TABLE", 1, CORE_0_INDEX, &SPDSVC_WLAN_GEN_PARAMS_TABLE, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DEBUG_PRINT_TABLE", 1, CORE_0_INDEX, &DEBUG_PRINT_TABLE, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TCAM_GENERIC_FIELDS", 1, CORE_0_INDEX, &TCAM_GENERIC_FIELDS, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_0_INDEX, &BITS_CALC_MASKS_TABLE, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "GDX_PARAMS_TABLE", 1, CORE_0_INDEX, &GDX_PARAMS_TABLE, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_0_INDEX, &FW_ERROR_VECTOR_TABLE, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_0_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "NAT_CACHE_CFG", 1, CORE_0_INDEX, &NAT_CACHE_CFG, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_0_INDEX, &NAT_CACHE_KEY0_MASK, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "MULTICAST_KEY_MASK", 1, CORE_0_INDEX, &MULTICAST_KEY_MASK, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_0_INDEX, &TCAM_TABLE_CFG_TABLE, 2, 1, 1 },
#endif
#if defined BCM68375
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_0_INDEX, &NATC_L2_VLAN_KEY_MASK, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_CFG", 1, CORE_0_INDEX, &INGRESS_FILTER_CFG, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "NATC_L2_TOS_MASK", 1, CORE_0_INDEX, &NATC_L2_TOS_MASK, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_FLOW_RING_CACHE_CTX_TABLE", 1, CORE_1_INDEX, &DHD_FLOW_RING_CACHE_CTX_TABLE_1, 64, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_AUX_INFO_CACHE_TABLE", 1, CORE_1_INDEX, &DHD_AUX_INFO_CACHE_TABLE_1, 64, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_POST_COMMON_RADIO_DATA", 1, CORE_1_INDEX, &DHD_POST_COMMON_RADIO_DATA_1, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_1_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_1, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_MIRRORING_TABLE", 1, CORE_1_INDEX, &RX_MIRRORING_TABLE_1, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "DOS_DROP_REASONS_CFG", 1, CORE_1_INDEX, &DOS_DROP_REASONS_CFG_1, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SCORE_DATA_COLLECTION_PROFILE_TABLE", 1, CORE_1_INDEX, &SCORE_DATA_COLLECTION_PROFILE_TABLE_1, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "POLICER_PARAMS_TABLE", 1, CORE_1_INDEX, &POLICER_PARAMS_TABLE_1, 256, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_DSTAT_CFG", 1, CORE_1_INDEX, &DHD_DSTAT_CFG_1, 384, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_MONITOR_BQ_INFO_TABLE", 1, CORE_1_INDEX, &DHD_MONITOR_BQ_INFO_TABLE_1, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_1_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER_1, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_1_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_1, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING0_STACK", 1, CORE_1_INDEX, &PROCESSING0_STACK_1, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "SCORE_CFG_TABLE", 1, CORE_1_INDEX, &SCORE_CFG_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SPDTEST_GEN_PARAM", 1, CORE_1_INDEX, &SPDTEST_GEN_PARAM_1, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_BACKUP_INDEX_CACHE", 1, CORE_1_INDEX, &DHD_BACKUP_INDEX_CACHE_1, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING1_STACK", 1, CORE_1_INDEX, &PROCESSING1_STACK_1, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "SYSTEM_CONFIGURATION", 1, CORE_1_INDEX, &SYSTEM_CONFIGURATION_1, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_CPU_INT_ID", 1, CORE_1_INDEX, &DHD_CPU_INT_ID_1, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_CFG_TABLE", 1, CORE_1_INDEX, &VPORT_CFG_TABLE_1, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "SCORE_COLLECT_DROP_BITMAP_TABLE", 1, CORE_1_INDEX, &SCORE_COLLECT_DROP_BITMAP_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING2_STACK", 1, CORE_1_INDEX, &PROCESSING2_STACK_1, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "MIRRORING_SCRATCH", 1, CORE_1_INDEX, &MIRRORING_SCRATCH_1, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_MIRRORING_DISPATCHER_CREDIT_TABLE", 1, CORE_1_INDEX, &DHD_MIRRORING_DISPATCHER_CREDIT_TABLE_1, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_L2_HEADER", 1, CORE_1_INDEX, &DHD_L2_HEADER_1, 96, 1, 1 },
#endif
#if defined BCM68375
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_1_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_1, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING3_STACK", 1, CORE_1_INDEX, &PROCESSING3_STACK_1, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "REGISTERS_BUFFER", 1, CORE_1_INDEX, &REGISTERS_BUFFER_1, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR", 1, CORE_1_INDEX, &DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_1, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_1_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING4_STACK", 1, CORE_1_INDEX, &PROCESSING4_STACK_1, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "TUNNELS_PARSING_CFG", 1, CORE_1_INDEX, &TUNNELS_PARSING_CFG_1, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_1_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "MUTEX_TABLE", 1, CORE_1_INDEX, &MUTEX_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_CPU_TX_POST_RING_DESCRIPTOR_TABLE", 1, CORE_1_INDEX, &DHD_CPU_TX_POST_RING_DESCRIPTOR_TABLE_1, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "HOST_TX_TRUNCATE_MIRRORING_TABLE", 1, CORE_1_INDEX, &HOST_TX_TRUNCATE_MIRRORING_TABLE_1, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_1_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING5_STACK", 1, CORE_1_INDEX, &PROCESSING5_STACK_1, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_MIRRORING_MINIFPM_PARAMS", 1, CORE_1_INDEX, &DHD_MIRRORING_MINIFPM_PARAMS_1, 2, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_FPM_THRESHOLDS", 1, CORE_1_INDEX, &DHD_FPM_THRESHOLDS_1, 3, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_1_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE", 1, CORE_1_INDEX, &DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE_1, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_1_INDEX, &LOOPBACK_QUEUE_TABLE_1, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "SPDSVC_WLAN_TXPOST_PARAMS_TABLE", 1, CORE_1_INDEX, &SPDSVC_WLAN_TXPOST_PARAMS_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING6_STACK", 1, CORE_1_INDEX, &PROCESSING6_STACK_1, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "CSO_BAD_IPV4_HDR_CSUM_PACKETS", 1, CORE_1_INDEX, &CSO_BAD_IPV4_HDR_CSUM_PACKETS_1, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TASK_IDX", 1, CORE_1_INDEX, &TASK_IDX_1, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DEBUG_PRINT_TABLE", 1, CORE_1_INDEX, &DEBUG_PRINT_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_CFG_EX_TABLE", 1, CORE_1_INDEX, &VPORT_CFG_EX_TABLE_1, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_1_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_1, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_1_INDEX, &LOOPBACK_WAN_FLOW_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TR471_SPDSVC_RX_PKT_ID_TABLE", 1, CORE_1_INDEX, &TR471_SPDSVC_RX_PKT_ID_TABLE_1, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "TCAM_GENERIC_FIELDS", 1, CORE_1_INDEX, &TCAM_GENERIC_FIELDS_1, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING7_STACK", 1, CORE_1_INDEX, &PROCESSING7_STACK_1, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "IPTV_CFG_TABLE", 1, CORE_1_INDEX, &IPTV_CFG_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "GDX_PARAMS_TABLE", 1, CORE_1_INDEX, &GDX_PARAMS_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "FORCE_DSCP", 1, CORE_1_INDEX, &FORCE_DSCP_1, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "CORE_ID_TABLE", 1, CORE_1_INDEX, &CORE_ID_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "LLQ_SELECTOR_ECN_MASK", 1, CORE_1_INDEX, &LLQ_SELECTOR_ECN_MASK_1, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_1_INDEX, &DEBUG_PRINT_CORE_LOCK_1, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_1_INDEX, &BUFMNG_DESCRIPTOR_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFMNG_STATUS_TABLE", 1, CORE_1_INDEX, &BUFMNG_STATUS_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_TX_POST_UPDATE_FIFO_TABLE", 1, CORE_1_INDEX, &DHD_TX_POST_UPDATE_FIFO_TABLE_1, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_TIMER_STACK", 1, CORE_1_INDEX, &DHD_TIMER_STACK_1, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_TX_POST_PD_FIFO_TABLE", 1, CORE_1_INDEX, &DHD_TX_POST_PD_FIFO_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_FLOW_TABLE", 1, CORE_1_INDEX, &RX_FLOW_TABLE_1, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_DOORBELL_TX_POST_VALUE", 1, CORE_1_INDEX, &DHD_DOORBELL_TX_POST_VALUE_1, 64, 1, 1 },
#endif
#if defined BCM68375
	{ "TX_FLOW_TABLE", 1, CORE_1_INDEX, &TX_FLOW_TABLE_1, 212, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_1_INDEX, &RX_MIRRORING_CONFIGURATION_1, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_1_INDEX, &TX_MIRRORING_CONFIGURATION_1, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_1_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_1, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_TX_POST_UPDATE_FIFO_STACK", 1, CORE_1_INDEX, &DHD_TX_POST_UPDATE_FIFO_STACK_1, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_TX_POST_FLOW_RING_BUFFER", 1, CORE_1_INDEX, &DHD_TX_POST_FLOW_RING_BUFFER_1, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_1_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_1, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "SRAM_DUMMY_STORE", 1, CORE_1_INDEX, &SRAM_DUMMY_STORE_1, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_HW_CFG", 1, CORE_1_INDEX, &DHD_HW_CFG_1, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "CSO_DISABLE", 1, CORE_1_INDEX, &CSO_DISABLE_1, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_1_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_1_INDEX, &INGRESS_FILTER_1588_CFG_1, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_1_INDEX, &RX_MIRRORING_DIRECT_ENABLE_1, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "NAT_CACHE_CFG", 1, CORE_1_INDEX, &NAT_CACHE_CFG_1, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_TX_POST_0_STACK", 1, CORE_1_INDEX, &DHD_TX_POST_0_STACK_1, 168, 1, 1 },
#endif
#if defined BCM68375
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_1_INDEX, &NAT_CACHE_KEY0_MASK_1, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_1_INDEX, &TCAM_TABLE_CFG_TABLE_1, 2, 1, 1 },
#endif
#if defined BCM68375
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_1_INDEX, &NATC_L2_VLAN_KEY_MASK_1, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_1_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_1, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "FPM_GLOBAL_CFG", 1, CORE_1_INDEX, &FPM_GLOBAL_CFG_1, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_TX_POST_1_STACK", 1, CORE_1_INDEX, &DHD_TX_POST_1_STACK_1, 168, 1, 1 },
#endif
#if defined BCM68375
	{ "DEBUG_SCRATCHPAD", 1, CORE_1_INDEX, &DEBUG_SCRATCHPAD_1, 12, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_CFG", 1, CORE_1_INDEX, &INGRESS_FILTER_CFG_1, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_1_INDEX, &FW_ERROR_VECTOR_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "NATC_L2_TOS_MASK", 1, CORE_1_INDEX, &NATC_L2_TOS_MASK_1, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_TX_POST_2_STACK", 1, CORE_1_INDEX, &DHD_TX_POST_2_STACK_1, 168, 1, 1 },
#endif
#if defined BCM68375
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_1_INDEX, &BITS_CALC_MASKS_TABLE_1, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "MULTICAST_KEY_MASK", 1, CORE_1_INDEX, &MULTICAST_KEY_MASK_1, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_FLOW_RING_CACHE_LKP_TABLE", 1, CORE_1_INDEX, &DHD_FLOW_RING_CACHE_LKP_TABLE_1, 64, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_1_INDEX, &INGRESS_FILTER_PROFILE_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "POLICER_PARAMS_TABLE", 1, CORE_2_INDEX, &POLICER_PARAMS_TABLE_2, 256, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING0_STACK", 1, CORE_2_INDEX, &PROCESSING0_STACK_2, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "SCORE_CFG_TABLE", 1, CORE_2_INDEX, &SCORE_CFG_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SPDTEST_GEN_PARAM", 1, CORE_2_INDEX, &SPDTEST_GEN_PARAM_2, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "SCORE_DATA_COLLECTION_PROFILE_TABLE", 1, CORE_2_INDEX, &SCORE_DATA_COLLECTION_PROFILE_TABLE_2, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "CPU_RX_SCRATCHPAD", 1, CORE_2_INDEX, &CPU_RX_SCRATCHPAD_2, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_2_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_2, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING1_STACK", 1, CORE_2_INDEX, &PROCESSING1_STACK_2, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "SYSTEM_CONFIGURATION", 1, CORE_2_INDEX, &SYSTEM_CONFIGURATION_2, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "CPU_RX_AUX_INT_SCRATCHPAD", 1, CORE_2_INDEX, &CPU_RX_AUX_INT_SCRATCHPAD_2, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "GENERAL_TIMER", 1, CORE_2_INDEX, &GENERAL_TIMER_2, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING2_STACK", 1, CORE_2_INDEX, &PROCESSING2_STACK_2, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "CPU_RX_INTERRUPT_ID_DDR_ADDR", 1, CORE_2_INDEX, &CPU_RX_INTERRUPT_ID_DDR_ADDR_2, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_2_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_2, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "CPU_RX_CSO_STACK", 1, CORE_2_INDEX, &CPU_RX_CSO_STACK_2, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING3_STACK", 1, CORE_2_INDEX, &PROCESSING3_STACK_2, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "SCORE_COLLECT_DROP_BITMAP_TABLE", 1, CORE_2_INDEX, &SCORE_COLLECT_DROP_BITMAP_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "MUTEX_TABLE", 1, CORE_2_INDEX, &MUTEX_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "CPU_RX_CSO_DISPATCHER_CREDIT_TABLE", 1, CORE_2_INDEX, &CPU_RX_CSO_DISPATCHER_CREDIT_TABLE_2, 3, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_2_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "CPU_RXQ_SCRATCH_TABLE", 1, CORE_2_INDEX, &CPU_RXQ_SCRATCH_TABLE_2, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_2_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER_2, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING4_STACK", 1, CORE_2_INDEX, &PROCESSING4_STACK_2, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "TUNNELS_PARSING_CFG", 1, CORE_2_INDEX, &TUNNELS_PARSING_CFG_2, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_2_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "CSO_BAD_IPV4_HDR_CSUM_PACKETS", 1, CORE_2_INDEX, &CSO_BAD_IPV4_HDR_CSUM_PACKETS_2, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_CFG_TABLE", 1, CORE_2_INDEX, &VPORT_CFG_TABLE_2, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "TASK_IDX", 1, CORE_2_INDEX, &TASK_IDX_2, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING5_STACK", 1, CORE_2_INDEX, &PROCESSING5_STACK_2, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "RNR_WR_TEST_ADDR", 1, CORE_2_INDEX, &RNR_WR_TEST_ADDR_2, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_FPM_THRESHOLDS", 1, CORE_2_INDEX, &DHD_FPM_THRESHOLDS_2, 3, 1, 1 },
#endif
#if defined BCM68375
	{ "DOS_DROP_REASONS_CFG", 1, CORE_2_INDEX, &DOS_DROP_REASONS_CFG_2, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "GENERAL_TIMER_ACTION_VEC", 1, CORE_2_INDEX, &GENERAL_TIMER_ACTION_VEC_2, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "CPU_REASON_AND_VPORT_TO_METER_TABLE", 1, CORE_2_INDEX, &CPU_REASON_AND_VPORT_TO_METER_TABLE_2, 96, 1, 1 },
#endif
#if defined BCM68375
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_2_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_2, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING6_STACK", 1, CORE_2_INDEX, &PROCESSING6_STACK_2, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "IPTV_CFG_TABLE", 1, CORE_2_INDEX, &IPTV_CFG_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DEBUG_PRINT_TABLE", 1, CORE_2_INDEX, &DEBUG_PRINT_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "GENERAL_TIMER_STACK", 1, CORE_2_INDEX, &GENERAL_TIMER_STACK_2, 72, 1, 1 },
#endif
#if defined BCM68375
	{ "TCAM_GENERIC_FIELDS", 1, CORE_2_INDEX, &TCAM_GENERIC_FIELDS_2, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_2_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_2, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_2_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_2, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING7_STACK", 1, CORE_2_INDEX, &PROCESSING7_STACK_2, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "REGISTERS_BUFFER", 1, CORE_2_INDEX, &REGISTERS_BUFFER_2, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "NAT_CACHE_CFG", 1, CORE_2_INDEX, &NAT_CACHE_CFG_2, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_2_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_2, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "CPU_INTERRUPT_COALESCING_TABLE", 1, CORE_2_INDEX, &CPU_INTERRUPT_COALESCING_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_FLOW_TABLE", 1, CORE_2_INDEX, &RX_FLOW_TABLE_2, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "CPU_RX_METER_TABLE", 1, CORE_2_INDEX, &CPU_RX_METER_TABLE_2, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "TX_FLOW_TABLE", 1, CORE_2_INDEX, &TX_FLOW_TABLE_2, 212, 1, 1 },
#endif
#if defined BCM68375
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_2_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_2_INDEX, &LOOPBACK_WAN_FLOW_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "FORCE_DSCP", 1, CORE_2_INDEX, &FORCE_DSCP_2, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "CPU_RX_INTERRUPT_SCRATCH", 1, CORE_2_INDEX, &CPU_RX_INTERRUPT_SCRATCH_2, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "RNR_WR_TEST_FROM", 1, CORE_2_INDEX, &RNR_WR_TEST_FROM_2, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_2_INDEX, &BUFMNG_DESCRIPTOR_TABLE_2, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "CPU_RING_INTERRUPT_COUNTER_TABLE", 1, CORE_2_INDEX, &CPU_RING_INTERRUPT_COUNTER_TABLE_2, 24, 1, 1 },
#endif
#if defined BCM68375
	{ "CPU_REASON_TO_METER_TABLE", 1, CORE_2_INDEX, &CPU_REASON_TO_METER_TABLE_2, 64, 1, 1 },
#endif
#if defined BCM68375
	{ "CPU_RX_XPM_PD_FIFO_TABLE", 1, CORE_2_INDEX, &CPU_RX_XPM_PD_FIFO_TABLE_2, 12, 1, 1 },
#endif
#if defined BCM68375
	{ "IPV6_HOST_ADDRESS_CRC_TABLE", 1, CORE_2_INDEX, &IPV6_HOST_ADDRESS_CRC_TABLE_2, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "CSO_CONTEXT_TABLE", 1, CORE_2_INDEX, &CSO_CONTEXT_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "CPU_RX_LOCAL_SCRATCH", 1, CORE_2_INDEX, &CPU_RX_LOCAL_SCRATCH_2, 2, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_MIRRORING_TABLE", 1, CORE_2_INDEX, &RX_MIRRORING_TABLE_2, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "CORE_ID_TABLE", 1, CORE_2_INDEX, &CORE_ID_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "LLQ_SELECTOR_ECN_MASK", 1, CORE_2_INDEX, &LLQ_SELECTOR_ECN_MASK_2, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "CPU_RX_STACK", 1, CORE_2_INDEX, &CPU_RX_STACK_2, 136, 1, 1 },
#endif
#if defined BCM68375
	{ "RNR_WR_TEST_TO", 1, CORE_2_INDEX, &RNR_WR_TEST_TO_2, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_2_INDEX, &RX_MIRRORING_CONFIGURATION_2, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_2_INDEX, &TX_MIRRORING_CONFIGURATION_2, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "GDX_PARAMS_TABLE", 1, CORE_2_INDEX, &GDX_PARAMS_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_2_INDEX, &DEBUG_PRINT_CORE_LOCK_2, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SRAM_DUMMY_STORE", 1, CORE_2_INDEX, &SRAM_DUMMY_STORE_2, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "CSO_DISABLE", 1, CORE_2_INDEX, &CSO_DISABLE_2, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "RNR_WR_TEST_START", 1, CORE_2_INDEX, &RNR_WR_TEST_START_2, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFMNG_STATUS_TABLE", 1, CORE_2_INDEX, &BUFMNG_STATUS_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "HOST_TX_TRUNCATE_MIRRORING_TABLE", 1, CORE_2_INDEX, &HOST_TX_TRUNCATE_MIRRORING_TABLE_2, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_2_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_2_INDEX, &INGRESS_FILTER_1588_CFG_2, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_2_INDEX, &INGRESS_FILTER_PROFILE_TABLE_2, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_2_INDEX, &LOOPBACK_QUEUE_TABLE_2, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_2_INDEX, &RX_MIRRORING_DIRECT_ENABLE_2, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_CFG_EX_TABLE", 1, CORE_2_INDEX, &VPORT_CFG_EX_TABLE_2, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "CPU_RING_DESCRIPTORS_TABLE", 1, CORE_2_INDEX, &CPU_RING_DESCRIPTORS_TABLE_2, 24, 1, 1 },
#endif
#if defined BCM68375
	{ "TR471_SPDSVC_RX_PKT_ID_TABLE", 1, CORE_2_INDEX, &TR471_SPDSVC_RX_PKT_ID_TABLE_2, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "CPU_RX_TELEMETRY_BUFFER_LOCATION", 1, CORE_2_INDEX, &CPU_RX_TELEMETRY_BUFFER_LOCATION_2, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_2_INDEX, &NAT_CACHE_KEY0_MASK_2, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "CPU_RX_CSO_UPDATE_FIFO_TABLE", 1, CORE_2_INDEX, &CPU_RX_CSO_UPDATE_FIFO_TABLE_2, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "IPV4_HOST_ADDRESS_TABLE", 1, CORE_2_INDEX, &IPV4_HOST_ADDRESS_TABLE_2, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "CPU_RX_CSO_PD_FIFO_TABLE", 1, CORE_2_INDEX, &CPU_RX_CSO_PD_FIFO_TABLE_2, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "CPU_RX_XPM_UPDATE_FIFO_TABLE", 1, CORE_2_INDEX, &CPU_RX_XPM_UPDATE_FIFO_TABLE_2, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "CPU_REASON_TO_TC", 1, CORE_2_INDEX, &CPU_REASON_TO_TC_2, 64, 1, 1 },
#endif
#if defined BCM68375
	{ "TC_TO_CPU_RXQ", 1, CORE_2_INDEX, &TC_TO_CPU_RXQ_2, 64, 1, 1 },
#endif
#if defined BCM68375
	{ "EXC_TC_TO_CPU_RXQ", 1, CORE_2_INDEX, &EXC_TC_TO_CPU_RXQ_2, 64, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_HW_CFG", 1, CORE_2_INDEX, &DHD_HW_CFG_2, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_2_INDEX, &TCAM_TABLE_CFG_TABLE_2, 2, 1, 1 },
#endif
#if defined BCM68375
	{ "CPU_RXQ_DATA_BUF_TYPE_TABLE", 1, CORE_2_INDEX, &CPU_RXQ_DATA_BUF_TYPE_TABLE_2, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "FPM_GLOBAL_CFG", 1, CORE_2_INDEX, &FPM_GLOBAL_CFG_2, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_2_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_2, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_2_INDEX, &FW_ERROR_VECTOR_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_2_INDEX, &NATC_L2_VLAN_KEY_MASK_2, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_CFG", 1, CORE_2_INDEX, &INGRESS_FILTER_CFG_2, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "NATC_L2_TOS_MASK", 1, CORE_2_INDEX, &NATC_L2_TOS_MASK_2, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_2_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_2, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "DEBUG_SCRATCHPAD", 1, CORE_2_INDEX, &DEBUG_SCRATCHPAD_2, 12, 1, 1 },
#endif
#if defined BCM68375
	{ "MULTICAST_KEY_MASK", 1, CORE_2_INDEX, &MULTICAST_KEY_MASK_2, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "CPU_VPORT_TO_METER_TABLE", 1, CORE_2_INDEX, &CPU_VPORT_TO_METER_TABLE_2, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_2_INDEX, &BITS_CALC_MASKS_TABLE_2, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_TO_CPU_OBJ", 1, CORE_2_INDEX, &VPORT_TO_CPU_OBJ_2, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "CPU_TX_SCRATCHPAD", 1, CORE_3_INDEX, &CPU_TX_SCRATCHPAD_3, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "POLICER_PARAMS_TABLE", 1, CORE_3_INDEX, &POLICER_PARAMS_TABLE_3, 256, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING0_STACK", 1, CORE_3_INDEX, &PROCESSING0_STACK_3, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "SCORE_CFG_TABLE", 1, CORE_3_INDEX, &SCORE_CFG_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SPDTEST_GEN_PARAM", 1, CORE_3_INDEX, &SPDTEST_GEN_PARAM_3, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "SCORE_DATA_COLLECTION_PROFILE_TABLE", 1, CORE_3_INDEX, &SCORE_DATA_COLLECTION_PROFILE_TABLE_3, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_3_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_3, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING1_STACK", 1, CORE_3_INDEX, &PROCESSING1_STACK_3, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "SYSTEM_CONFIGURATION", 1, CORE_3_INDEX, &SYSTEM_CONFIGURATION_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFFER_ALLOC_REPLY", 1, CORE_3_INDEX, &BUFFER_ALLOC_REPLY_3, 2, 1, 1 },
#endif
#if defined BCM68375
	{ "GENERAL_TIMER", 1, CORE_3_INDEX, &GENERAL_TIMER_3, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING2_STACK", 1, CORE_3_INDEX, &PROCESSING2_STACK_3, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR", 1, CORE_3_INDEX, &CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "CPU_RECYCLE_INTERRUPT_COALESCING_TABLE", 1, CORE_3_INDEX, &CPU_RECYCLE_INTERRUPT_COALESCING_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_CFG_TABLE", 1, CORE_3_INDEX, &VPORT_CFG_TABLE_3, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "DDR_LATENCY_DBG_USEC", 1, CORE_3_INDEX, &DDR_LATENCY_DBG_USEC_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING3_STACK", 1, CORE_3_INDEX, &PROCESSING3_STACK_3, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "CPU_RX_INTERRUPT_ID_DDR_ADDR", 1, CORE_3_INDEX, &CPU_RX_INTERRUPT_ID_DDR_ADDR_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_3_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_3, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "SG_DESC_TABLE", 1, CORE_3_INDEX, &SG_DESC_TABLE_3, 2, 1, 1 },
#endif
#if defined BCM68375
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_3_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_3, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_3_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER_3, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING4_STACK", 1, CORE_3_INDEX, &PROCESSING4_STACK_3, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "CPU_TX_RING_INDICES_VALUES_TABLE", 1, CORE_3_INDEX, &CPU_TX_RING_INDICES_VALUES_TABLE_3, 2, 1, 1 },
#endif
#if defined BCM68375
	{ "CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE", 1, CORE_3_INDEX, &CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE_3, 3, 1, 1 },
#endif
#if defined BCM68375
	{ "DDR_LATENCY_DBG_USEC_MAX", 1, CORE_3_INDEX, &DDR_LATENCY_DBG_USEC_MAX_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "GENERAL_TIMER_STACK", 1, CORE_3_INDEX, &GENERAL_TIMER_STACK_3, 72, 1, 1 },
#endif
#if defined BCM68375
	{ "SCORE_COLLECT_DROP_BITMAP_TABLE", 1, CORE_3_INDEX, &SCORE_COLLECT_DROP_BITMAP_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "MUTEX_TABLE", 1, CORE_3_INDEX, &MUTEX_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "CPU_TX_INGRESS_DISPATCHER_CREDIT_TABLE", 1, CORE_3_INDEX, &CPU_TX_INGRESS_DISPATCHER_CREDIT_TABLE_3, 3, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_3_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_3_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_3, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING5_STACK", 1, CORE_3_INDEX, &PROCESSING5_STACK_3, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "REGISTERS_BUFFER", 1, CORE_3_INDEX, &REGISTERS_BUFFER_3, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "TUNNELS_PARSING_CFG", 1, CORE_3_INDEX, &TUNNELS_PARSING_CFG_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_3_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "CSO_BAD_IPV4_HDR_CSUM_PACKETS", 1, CORE_3_INDEX, &CSO_BAD_IPV4_HDR_CSUM_PACKETS_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING6_STACK", 1, CORE_3_INDEX, &PROCESSING6_STACK_3, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE", 1, CORE_3_INDEX, &CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_FPM_THRESHOLDS", 1, CORE_3_INDEX, &DHD_FPM_THRESHOLDS_3, 3, 1, 1 },
#endif
#if defined BCM68375
	{ "CPU_INT_INTERRUPT_SCRATCH", 1, CORE_3_INDEX, &CPU_INT_INTERRUPT_SCRATCH_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_MIRRORING_TABLE", 1, CORE_3_INDEX, &RX_MIRRORING_TABLE_3, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "DOS_DROP_REASONS_CFG", 1, CORE_3_INDEX, &DOS_DROP_REASONS_CFG_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "HOST_TX_TRUNCATE_MIRRORING_TABLE", 1, CORE_3_INDEX, &HOST_TX_TRUNCATE_MIRRORING_TABLE_3, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "GENERAL_TIMER_ACTION_VEC", 1, CORE_3_INDEX, &GENERAL_TIMER_ACTION_VEC_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING7_STACK", 1, CORE_3_INDEX, &PROCESSING7_STACK_3, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "IPTV_CFG_TABLE", 1, CORE_3_INDEX, &IPTV_CFG_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DEBUG_PRINT_TABLE", 1, CORE_3_INDEX, &DEBUG_PRINT_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_3_INDEX, &LOOPBACK_QUEUE_TABLE_3, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_3_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_CFG_EX_TABLE", 1, CORE_3_INDEX, &VPORT_CFG_EX_TABLE_3, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_3_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_3_INDEX, &LOOPBACK_WAN_FLOW_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_FLOW_TABLE", 1, CORE_3_INDEX, &RX_FLOW_TABLE_3, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "CPU_TX_0_STACK", 1, CORE_3_INDEX, &CPU_TX_0_STACK_3, 224, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_3_INDEX, &BUFMNG_DESCRIPTOR_TABLE_3, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "CPU_RECYCLE_SRAM_PD_FIFO", 1, CORE_3_INDEX, &CPU_RECYCLE_SRAM_PD_FIFO_3, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "CPU_TX_1_STACK", 1, CORE_3_INDEX, &CPU_TX_1_STACK_3, 224, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFMNG_STATUS_TABLE", 1, CORE_3_INDEX, &BUFMNG_STATUS_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TX_FLOW_TABLE", 1, CORE_3_INDEX, &TX_FLOW_TABLE_3, 212, 1, 1 },
#endif
#if defined BCM68375
	{ "TASK_IDX", 1, CORE_3_INDEX, &TASK_IDX_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_3_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "CPU_RECYCLE_STACK", 1, CORE_3_INDEX, &CPU_RECYCLE_STACK_3, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "SG_CONTEXT_TABLE", 1, CORE_3_INDEX, &SG_CONTEXT_TABLE_3, 2, 1, 1 },
#endif
#if defined BCM68375
	{ "QUEUE_THRESHOLD_VECTOR", 1, CORE_3_INDEX, &QUEUE_THRESHOLD_VECTOR_3, 14, 1, 1 },
#endif
#if defined BCM68375
	{ "NAT_CACHE_CFG", 1, CORE_3_INDEX, &NAT_CACHE_CFG_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "FORCE_DSCP", 1, CORE_3_INDEX, &FORCE_DSCP_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TR471_SPDSVC_RX_PKT_ID_TABLE", 1, CORE_3_INDEX, &TR471_SPDSVC_RX_PKT_ID_TABLE_3, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "TCAM_GENERIC_FIELDS", 1, CORE_3_INDEX, &TCAM_GENERIC_FIELDS_3, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_3_INDEX, &INGRESS_FILTER_PROFILE_TABLE_3, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "RING_CPU_TX_DESCRIPTOR_DATA_TABLE", 1, CORE_3_INDEX, &RING_CPU_TX_DESCRIPTOR_DATA_TABLE_3, 2, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_HW_CFG", 1, CORE_3_INDEX, &DHD_HW_CFG_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "CORE_ID_TABLE", 1, CORE_3_INDEX, &CORE_ID_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "LLQ_SELECTOR_ECN_MASK", 1, CORE_3_INDEX, &LLQ_SELECTOR_ECN_MASK_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_3_INDEX, &RX_MIRRORING_CONFIGURATION_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "CPU_RECYCLE_SHADOW_RD_IDX", 1, CORE_3_INDEX, &CPU_RECYCLE_SHADOW_RD_IDX_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_3_INDEX, &TX_MIRRORING_CONFIGURATION_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_3_INDEX, &DEBUG_PRINT_CORE_LOCK_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SRAM_DUMMY_STORE", 1, CORE_3_INDEX, &SRAM_DUMMY_STORE_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "CSO_DISABLE", 1, CORE_3_INDEX, &CSO_DISABLE_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_3_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_3_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_3, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_3_INDEX, &INGRESS_FILTER_1588_CFG_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "CPU_TX_RING_DESCRIPTOR_TABLE", 1, CORE_3_INDEX, &CPU_TX_RING_DESCRIPTOR_TABLE_3, 2, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_3_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_3, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_3_INDEX, &RX_MIRRORING_DIRECT_ENABLE_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DEBUG_SCRATCHPAD", 1, CORE_3_INDEX, &DEBUG_SCRATCHPAD_3, 12, 1, 1 },
#endif
#if defined BCM68375
	{ "CPU_RECYCLE_RING_DESCRIPTOR_TABLE", 1, CORE_3_INDEX, &CPU_RECYCLE_RING_DESCRIPTOR_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "FPM_GLOBAL_CFG", 1, CORE_3_INDEX, &FPM_GLOBAL_CFG_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "CPU_TX_SYNC_FIFO_TABLE", 1, CORE_3_INDEX, &CPU_TX_SYNC_FIFO_TABLE_3, 2, 1, 1 },
#endif
#if defined BCM68375
	{ "CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE", 1, CORE_3_INDEX, &CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE_3, 2, 1, 1 },
#endif
#if defined BCM68375
	{ "CPU_RECYCLE_INTERRUPT_SCRATCH", 1, CORE_3_INDEX, &CPU_RECYCLE_INTERRUPT_SCRATCH_3, 2, 1, 1 },
#endif
#if defined BCM68375
	{ "CPU_RECYCLE_SHADOW_WR_IDX", 1, CORE_3_INDEX, &CPU_RECYCLE_SHADOW_WR_IDX_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "GDX_PARAMS_TABLE", 1, CORE_3_INDEX, &GDX_PARAMS_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_3_INDEX, &BITS_CALC_MASKS_TABLE_3, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "CPU_RECYCLE_NEXT_PTR_TABLE", 1, CORE_3_INDEX, &CPU_RECYCLE_NEXT_PTR_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_3_INDEX, &NAT_CACHE_KEY0_MASK_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_3_INDEX, &FW_ERROR_VECTOR_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_3_INDEX, &TCAM_TABLE_CFG_TABLE_3, 2, 1, 1 },
#endif
#if defined BCM68375
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_3_INDEX, &NATC_L2_VLAN_KEY_MASK_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_CFG", 1, CORE_3_INDEX, &INGRESS_FILTER_CFG_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "MULTICAST_KEY_MASK", 1, CORE_3_INDEX, &MULTICAST_KEY_MASK_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "NATC_L2_TOS_MASK", 1, CORE_3_INDEX, &NATC_L2_TOS_MASK_3, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SQ_TM_PD_FIFO_TABLE", 1, CORE_4_INDEX, &SQ_TM_PD_FIFO_TABLE_4, 64, 1, 1 },
#endif
#if defined BCM68375
	{ "CRYPTO_SESSION_PARAMS_TABLE", 1, CORE_4_INDEX, &CRYPTO_SESSION_PARAMS_TABLE_4, 64, 1, 1 },
#endif
#if defined BCM68375
	{ "SQ_TM_RATE_LIMITER_BUDGET_DESCRIPTOR_TABLE", 1, CORE_4_INDEX, &SQ_TM_RATE_LIMITER_BUDGET_DESCRIPTOR_TABLE_4, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "POLICER_PARAMS_TABLE", 1, CORE_4_INDEX, &POLICER_PARAMS_TABLE_4, 256, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING0_STACK", 1, CORE_4_INDEX, &PROCESSING0_STACK_4, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "SERVICE_QUEUES_SCRATCHPAD", 1, CORE_4_INDEX, &SERVICE_QUEUES_SCRATCHPAD_4, 2, 1, 1 },
#endif
#if defined BCM68375
	{ "SQ_TM_RATE_LIMITER_BUDGET_VALID", 1, CORE_4_INDEX, &SQ_TM_RATE_LIMITER_BUDGET_VALID_4, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "SQ_TM_RATE_LIMITER_PROFILE_TABLE", 1, CORE_4_INDEX, &SQ_TM_RATE_LIMITER_PROFILE_TABLE_4, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_4_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_4, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING1_STACK", 1, CORE_4_INDEX, &PROCESSING1_STACK_4, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "SCORE_CFG_TABLE", 1, CORE_4_INDEX, &SCORE_CFG_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SPDTEST_GEN_PARAM", 1, CORE_4_INDEX, &SPDTEST_GEN_PARAM_4, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "SERVICE_QUEUES_STACK", 1, CORE_4_INDEX, &SERVICE_QUEUES_STACK_4, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_4_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER_4, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING2_STACK", 1, CORE_4_INDEX, &PROCESSING2_STACK_4, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "SYSTEM_CONFIGURATION", 1, CORE_4_INDEX, &SYSTEM_CONFIGURATION_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_4_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_4, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "SQ_TM_DCL_IMG", 1, CORE_4_INDEX, &SQ_TM_DCL_IMG_4, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING3_STACK", 1, CORE_4_INDEX, &PROCESSING3_STACK_4, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "CPU_RX_INTERRUPT_ID_DDR_ADDR", 1, CORE_4_INDEX, &CPU_RX_INTERRUPT_ID_DDR_ADDR_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SQ_TM_SECONDARY_SCHEDULER_TABLE", 1, CORE_4_INDEX, &SQ_TM_SECONDARY_SCHEDULER_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SCORE_COLLECT_DROP_BITMAP_TABLE", 1, CORE_4_INDEX, &SCORE_COLLECT_DROP_BITMAP_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SCORE_DATA_COLLECTION_PROFILE_TABLE", 1, CORE_4_INDEX, &SCORE_DATA_COLLECTION_PROFILE_TABLE_4, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING4_STACK", 1, CORE_4_INDEX, &PROCESSING4_STACK_4, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "TUNNELS_PARSING_CFG", 1, CORE_4_INDEX, &TUNNELS_PARSING_CFG_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SQ_TM_DISPATCHER_CREDIT_TABLE", 1, CORE_4_INDEX, &SQ_TM_DISPATCHER_CREDIT_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "MUTEX_TABLE", 1, CORE_4_INDEX, &MUTEX_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "GENERAL_TIMER", 1, CORE_4_INDEX, &GENERAL_TIMER_4, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING5_STACK", 1, CORE_4_INDEX, &PROCESSING5_STACK_4, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_4_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "CSO_BAD_IPV4_HDR_CSUM_PACKETS", 1, CORE_4_INDEX, &CSO_BAD_IPV4_HDR_CSUM_PACKETS_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_4_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SQ_TM_UPDATE_FIFO_START_INDEX", 1, CORE_4_INDEX, &SQ_TM_UPDATE_FIFO_START_INDEX_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_CFG_TABLE", 1, CORE_4_INDEX, &VPORT_CFG_TABLE_4, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "TASK_IDX", 1, CORE_4_INDEX, &TASK_IDX_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING6_STACK", 1, CORE_4_INDEX, &PROCESSING6_STACK_4, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "IPTV_CFG_TABLE", 1, CORE_4_INDEX, &IPTV_CFG_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_FPM_THRESHOLDS", 1, CORE_4_INDEX, &DHD_FPM_THRESHOLDS_4, 3, 1, 1 },
#endif
#if defined BCM68375
	{ "DOS_DROP_REASONS_CFG", 1, CORE_4_INDEX, &DOS_DROP_REASONS_CFG_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "GENERAL_TIMER_ACTION_VEC", 1, CORE_4_INDEX, &GENERAL_TIMER_ACTION_VEC_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "GENERAL_TIMER_STACK", 1, CORE_4_INDEX, &GENERAL_TIMER_STACK_4, 72, 1, 1 },
#endif
#if defined BCM68375
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_4_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DEBUG_PRINT_TABLE", 1, CORE_4_INDEX, &DEBUG_PRINT_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SQ_TM_RATE_LIMITER_PROFILE_RESIDUE_TABLE", 1, CORE_4_INDEX, &SQ_TM_RATE_LIMITER_PROFILE_RESIDUE_TABLE_4, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING7_STACK", 1, CORE_4_INDEX, &PROCESSING7_STACK_4, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "SQ_TM_AQM_QUEUE_TABLE", 1, CORE_4_INDEX, &SQ_TM_AQM_QUEUE_TABLE_4, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "TCAM_GENERIC_FIELDS", 1, CORE_4_INDEX, &TCAM_GENERIC_FIELDS_4, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "NAT_CACHE_CFG", 1, CORE_4_INDEX, &NAT_CACHE_CFG_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_4_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SQ_TM_SCHEDULING_QUEUE_TABLE", 1, CORE_4_INDEX, &SQ_TM_SCHEDULING_QUEUE_TABLE_4, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "REGISTERS_BUFFER", 1, CORE_4_INDEX, &REGISTERS_BUFFER_4, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "SPU_REQUEST_STACK", 1, CORE_4_INDEX, &SPU_REQUEST_STACK_4, 64, 1, 1 },
#endif
#if defined BCM68375
	{ "SQ_TM_RATE_LIMITER_PARAMS_DESCRIPTOR_TABLE", 1, CORE_4_INDEX, &SQ_TM_RATE_LIMITER_PARAMS_DESCRIPTOR_TABLE_4, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_FLOW_TABLE", 1, CORE_4_INDEX, &RX_FLOW_TABLE_4, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "TX_FLOW_TABLE", 1, CORE_4_INDEX, &TX_FLOW_TABLE_4, 212, 1, 1 },
#endif
#if defined BCM68375
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_4_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SQ_TM_FIRST_QUEUE_MAPPING", 1, CORE_4_INDEX, &SQ_TM_FIRST_QUEUE_MAPPING_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_4_INDEX, &NAT_CACHE_KEY0_MASK_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_4_INDEX, &LOOPBACK_WAN_FLOW_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "FORCE_DSCP", 1, CORE_4_INDEX, &FORCE_DSCP_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "CORE_ID_TABLE", 1, CORE_4_INDEX, &CORE_ID_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "LLQ_SELECTOR_ECN_MASK", 1, CORE_4_INDEX, &LLQ_SELECTOR_ECN_MASK_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_4_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_4, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_4_INDEX, &INGRESS_FILTER_PROFILE_TABLE_4, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "SPU_REQUEST_PD_FIFO_TABLE", 1, CORE_4_INDEX, &SPU_REQUEST_PD_FIFO_TABLE_4, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "SPU_REQUEST_PREPEND", 1, CORE_4_INDEX, &SPU_REQUEST_PREPEND_4, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFFER_CONG_SCRATCHPAD", 1, CORE_4_INDEX, &BUFFER_CONG_SCRATCHPAD_4, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "CRYPTO_SESSION_STATS_SCRATCH", 1, CORE_4_INDEX, &CRYPTO_SESSION_STATS_SCRATCH_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_MIRRORING_TABLE", 1, CORE_4_INDEX, &RX_MIRRORING_TABLE_4, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_4_INDEX, &RX_MIRRORING_CONFIGURATION_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "HOST_TX_TRUNCATE_MIRRORING_TABLE", 1, CORE_4_INDEX, &HOST_TX_TRUNCATE_MIRRORING_TABLE_4, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_4_INDEX, &TX_MIRRORING_CONFIGURATION_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_4_INDEX, &LOOPBACK_QUEUE_TABLE_4, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_4_INDEX, &DEBUG_PRINT_CORE_LOCK_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SRAM_DUMMY_STORE", 1, CORE_4_INDEX, &SRAM_DUMMY_STORE_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_CFG_EX_TABLE", 1, CORE_4_INDEX, &VPORT_CFG_EX_TABLE_4, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "CSO_DISABLE", 1, CORE_4_INDEX, &CSO_DISABLE_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_4_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFFER_CONG_DQM_NOT_EMPTY", 1, CORE_4_INDEX, &BUFFER_CONG_DQM_NOT_EMPTY_4, 13, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_4_INDEX, &INGRESS_FILTER_1588_CFG_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_4_INDEX, &RX_MIRRORING_DIRECT_ENABLE_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_4_INDEX, &TCAM_TABLE_CFG_TABLE_4, 2, 1, 1 },
#endif
#if defined BCM68375
	{ "TR471_SPDSVC_RX_PKT_ID_TABLE", 1, CORE_4_INDEX, &TR471_SPDSVC_RX_PKT_ID_TABLE_4, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "GDX_PARAMS_TABLE", 1, CORE_4_INDEX, &GDX_PARAMS_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SQ_BUFFER_CONG_MGT_CFG", 1, CORE_4_INDEX, &SQ_BUFFER_CONG_MGT_CFG_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_4_INDEX, &NATC_L2_VLAN_KEY_MASK_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_CFG", 1, CORE_4_INDEX, &INGRESS_FILTER_CFG_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SPU_OFFLOAD_PARAMS_TABLE", 1, CORE_4_INDEX, &SPU_OFFLOAD_PARAMS_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SPU_OFFLOAD_SCRATCHPAD", 1, CORE_4_INDEX, &SPU_OFFLOAD_SCRATCHPAD_4, 9, 1, 1 },
#endif
#if defined BCM68375
	{ "NATC_L2_TOS_MASK", 1, CORE_4_INDEX, &NATC_L2_TOS_MASK_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SQ_TM_ENABLED_QUEUES_TABLE", 1, CORE_4_INDEX, &SQ_TM_ENABLED_QUEUES_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SQ_TM_UPDATE_FIFO_TABLE", 1, CORE_4_INDEX, &SQ_TM_UPDATE_FIFO_TABLE_4, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_4_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_4, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_4_INDEX, &BUFMNG_DESCRIPTOR_TABLE_4, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFMNG_STATUS_TABLE", 1, CORE_4_INDEX, &BUFMNG_STATUS_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SQ_TM_DCL_DATA_BASE", 1, CORE_4_INDEX, &SQ_TM_DCL_DATA_BASE_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SQ_TM_BUFMNG_STATUS_TABLE", 1, CORE_4_INDEX, &SQ_TM_BUFMNG_STATUS_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SPU_REQUEST_UPDATE_FIFO_TABLE", 1, CORE_4_INDEX, &SPU_REQUEST_UPDATE_FIFO_TABLE_4, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "SQ_TM_FW_FLUSH_TABLE", 1, CORE_4_INDEX, &SQ_TM_FW_FLUSH_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_4_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_4, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "SQ_TM_SCHEDULER_TABLE", 1, CORE_4_INDEX, &SQ_TM_SCHEDULER_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_4_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_4, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "DEBUG_SCRATCHPAD", 1, CORE_4_INDEX, &DEBUG_SCRATCHPAD_4, 12, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_HW_CFG", 1, CORE_4_INDEX, &DHD_HW_CFG_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SQ_TM_SCHEDULER_POOL", 1, CORE_4_INDEX, &SQ_TM_SCHEDULER_POOL_4, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "SQ_TM_AQM_QUEUE_TIMER_TABLE", 1, CORE_4_INDEX, &SQ_TM_AQM_QUEUE_TIMER_TABLE_4, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "SQ_TM_PI2_PROBABILITY_CALC_DESCRIPTOR", 1, CORE_4_INDEX, &SQ_TM_PI2_PROBABILITY_CALC_DESCRIPTOR_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "FPM_GLOBAL_CFG", 1, CORE_4_INDEX, &FPM_GLOBAL_CFG_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_4_INDEX, &BITS_CALC_MASKS_TABLE_4, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_4_INDEX, &FW_ERROR_VECTOR_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "MULTICAST_KEY_MASK", 1, CORE_4_INDEX, &MULTICAST_KEY_MASK_4, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TCPSPDTEST_STREAM_TABLE", 1, CORE_5_INDEX, &TCPSPDTEST_STREAM_TABLE_5, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_5_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_5, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "POLICER_PARAMS_TABLE", 1, CORE_5_INDEX, &POLICER_PARAMS_TABLE_5, 256, 1, 1 },
#endif
#if defined BCM68375
	{ "PKTGEN_TX_STREAM_TABLE", 1, CORE_5_INDEX, &PKTGEN_TX_STREAM_TABLE_5, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_5_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_5, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "COMMON_REPROCESSING_STACK", 1, CORE_5_INDEX, &COMMON_REPROCESSING_STACK_5, 64, 1, 1 },
#endif
#if defined BCM68375
	{ "SCORE_DATA_COLLECTION_PROFILE_TABLE", 1, CORE_5_INDEX, &SCORE_DATA_COLLECTION_PROFILE_TABLE_5, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_FLOW_TABLE", 1, CORE_5_INDEX, &RX_FLOW_TABLE_5, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_5_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_5, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING0_STACK", 1, CORE_5_INDEX, &PROCESSING0_STACK_5, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "SCORE_CFG_TABLE", 1, CORE_5_INDEX, &SCORE_CFG_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SPDTEST_GEN_PARAM", 1, CORE_5_INDEX, &SPDTEST_GEN_PARAM_5, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "TCPSPDTEST_SCRATCHPAD", 1, CORE_5_INDEX, &TCPSPDTEST_SCRATCHPAD_5, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_5_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER_5, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING1_STACK", 1, CORE_5_INDEX, &PROCESSING1_STACK_5, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "SYSTEM_CONFIGURATION", 1, CORE_5_INDEX, &SYSTEM_CONFIGURATION_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_5_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_5, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "SPDSVC_GEN_STACK", 1, CORE_5_INDEX, &SPDSVC_GEN_STACK_5, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING2_STACK", 1, CORE_5_INDEX, &PROCESSING2_STACK_5, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "TX_ABS_RECYCLE_COUNTERS", 1, CORE_5_INDEX, &TX_ABS_RECYCLE_COUNTERS_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SPDSVC_GEN_DISPATCHER_CREDIT_TABLE", 1, CORE_5_INDEX, &SPDSVC_GEN_DISPATCHER_CREDIT_TABLE_5, 3, 1, 1 },
#endif
#if defined BCM68375
	{ "SCORE_COLLECT_DROP_BITMAP_TABLE", 1, CORE_5_INDEX, &SCORE_COLLECT_DROP_BITMAP_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_CFG_TABLE", 1, CORE_5_INDEX, &VPORT_CFG_TABLE_5, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "PKTGEN_NO_SBPM_HDRS_CNTR", 1, CORE_5_INDEX, &PKTGEN_NO_SBPM_HDRS_CNTR_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING3_STACK", 1, CORE_5_INDEX, &PROCESSING3_STACK_5, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "CPU_RX_INTERRUPT_ID_DDR_ADDR", 1, CORE_5_INDEX, &CPU_RX_INTERRUPT_ID_DDR_ADDR_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TCPSPDTEST_DISPATCHER_CREDIT_TABLE", 1, CORE_5_INDEX, &TCPSPDTEST_DISPATCHER_CREDIT_TABLE_5, 3, 1, 1 },
#endif
#if defined BCM68375
	{ "MUTEX_TABLE", 1, CORE_5_INDEX, &MUTEX_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "PKTGEN_SBPM_HDR_BNS", 1, CORE_5_INDEX, &PKTGEN_SBPM_HDR_BNS_5, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_MIRRORING_TABLE", 1, CORE_5_INDEX, &RX_MIRRORING_TABLE_5, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "DOS_DROP_REASONS_CFG", 1, CORE_5_INDEX, &DOS_DROP_REASONS_CFG_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING4_STACK", 1, CORE_5_INDEX, &PROCESSING4_STACK_5, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "REGISTERS_BUFFER", 1, CORE_5_INDEX, &REGISTERS_BUFFER_5, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "PKTGEN_CURR_SBPM_HDR_PTR", 1, CORE_5_INDEX, &PKTGEN_CURR_SBPM_HDR_PTR_5, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "COMMON_REPROCESSING_DISPATCHER_CREDIT_TABLE", 1, CORE_5_INDEX, &COMMON_REPROCESSING_DISPATCHER_CREDIT_TABLE_5, 3, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_5_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING5_STACK", 1, CORE_5_INDEX, &PROCESSING5_STACK_5, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "PKTGEN_NUM_OF_AVAIL_SBPM_HDRS", 1, CORE_5_INDEX, &PKTGEN_NUM_OF_AVAIL_SBPM_HDRS_5, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "TCPSPDTEST_GEN_DISPATCHER_CREDIT_TABLE", 1, CORE_5_INDEX, &TCPSPDTEST_GEN_DISPATCHER_CREDIT_TABLE_5, 3, 1, 1 },
#endif
#if defined BCM68375
	{ "CSO_BAD_IPV4_HDR_CSUM_PACKETS", 1, CORE_5_INDEX, &CSO_BAD_IPV4_HDR_CSUM_PACKETS_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "COMMON_REPROCESSING_PD_FIFO_TABLE", 1, CORE_5_INDEX, &COMMON_REPROCESSING_PD_FIFO_TABLE_5, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "HOST_TX_TRUNCATE_MIRRORING_TABLE", 1, CORE_5_INDEX, &HOST_TX_TRUNCATE_MIRRORING_TABLE_5, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_5_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING6_STACK", 1, CORE_5_INDEX, &PROCESSING6_STACK_5, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "PKTGEN_SBPM_END_PTR", 1, CORE_5_INDEX, &PKTGEN_SBPM_END_PTR_5, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_5_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "PKTGEN_BAD_GET_NEXT", 1, CORE_5_INDEX, &PKTGEN_BAD_GET_NEXT_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TCPSPDTEST_PD_FIFO_TABLE", 1, CORE_5_INDEX, &TCPSPDTEST_PD_FIFO_TABLE_5, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_5_INDEX, &LOOPBACK_QUEUE_TABLE_5, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_5_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_5_INDEX, &LOOPBACK_WAN_FLOW_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING7_STACK", 1, CORE_5_INDEX, &PROCESSING7_STACK_5, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "TUNNELS_PARSING_CFG", 1, CORE_5_INDEX, &TUNNELS_PARSING_CFG_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_FPM_THRESHOLDS", 1, CORE_5_INDEX, &DHD_FPM_THRESHOLDS_5, 3, 1, 1 },
#endif
#if defined BCM68375
	{ "PKTGEN_MAX_UT_PKTS", 1, CORE_5_INDEX, &PKTGEN_MAX_UT_PKTS_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_CFG_EX_TABLE", 1, CORE_5_INDEX, &VPORT_CFG_EX_TABLE_5, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "FORCE_DSCP", 1, CORE_5_INDEX, &FORCE_DSCP_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "CORE_ID_TABLE", 1, CORE_5_INDEX, &CORE_ID_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SPDSVC_GEN_PARAMS_TABLE", 1, CORE_5_INDEX, &SPDSVC_GEN_PARAMS_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "PKTGEN_UT_TRIGGER", 1, CORE_5_INDEX, &PKTGEN_UT_TRIGGER_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TCPSPDTEST_DOWNLOAD_STACK", 1, CORE_5_INDEX, &TCPSPDTEST_DOWNLOAD_STACK_5, 240, 1, 1 },
#endif
#if defined BCM68375
	{ "PKTGEN_SBPM_EXTS", 1, CORE_5_INDEX, &PKTGEN_SBPM_EXTS_5, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "TX_FLOW_TABLE", 1, CORE_5_INDEX, &TX_FLOW_TABLE_5, 212, 1, 1 },
#endif
#if defined BCM68375
	{ "TASK_IDX", 1, CORE_5_INDEX, &TASK_IDX_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "IPTV_CFG_TABLE", 1, CORE_5_INDEX, &IPTV_CFG_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_5_INDEX, &BUFMNG_DESCRIPTOR_TABLE_5, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "UDPSPDT_STREAM_TX_STAT_TABLE", 1, CORE_5_INDEX, &UDPSPDT_STREAM_TX_STAT_TABLE_5, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFMNG_STATUS_TABLE", 1, CORE_5_INDEX, &BUFMNG_STATUS_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TR471_SPDSVC_RX_PKT_ID_TABLE", 1, CORE_5_INDEX, &TR471_SPDSVC_RX_PKT_ID_TABLE_5, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "PKTGEN_BBMSG_REPLY_SCRATCH", 1, CORE_5_INDEX, &PKTGEN_BBMSG_REPLY_SCRATCH_5, 2, 1, 1 },
#endif
#if defined BCM68375
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_5_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_5_INDEX, &INGRESS_FILTER_PROFILE_TABLE_5, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "PKTGEN_SESSION_DATA", 1, CORE_5_INDEX, &PKTGEN_SESSION_DATA_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TCPSPDTEST_ENGINE_GLOBAL_TABLE", 1, CORE_5_INDEX, &TCPSPDTEST_ENGINE_GLOBAL_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SPDSVC_TCPSPDTEST_COMMON_TABLE", 1, CORE_5_INDEX, &SPDSVC_TCPSPDTEST_COMMON_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "LLQ_SELECTOR_ECN_MASK", 1, CORE_5_INDEX, &LLQ_SELECTOR_ECN_MASK_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_5_INDEX, &RX_MIRRORING_CONFIGURATION_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "NAT_CACHE_CFG", 1, CORE_5_INDEX, &NAT_CACHE_CFG_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_5_INDEX, &DEBUG_PRINT_CORE_LOCK_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "COMMON_REPROCESSING_UPDATE_FIFO_TABLE", 1, CORE_5_INDEX, &COMMON_REPROCESSING_UPDATE_FIFO_TABLE_5, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_HW_CFG", 1, CORE_5_INDEX, &DHD_HW_CFG_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_5_INDEX, &TX_MIRRORING_CONFIGURATION_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SRAM_DUMMY_STORE", 1, CORE_5_INDEX, &SRAM_DUMMY_STORE_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "CSO_DISABLE", 1, CORE_5_INDEX, &CSO_DISABLE_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_5_INDEX, &NAT_CACHE_KEY0_MASK_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_5_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_5_INDEX, &INGRESS_FILTER_1588_CFG_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_5_INDEX, &RX_MIRRORING_DIRECT_ENABLE_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "UDPSPDT_TX_PARAMS_TABLE", 1, CORE_5_INDEX, &UDPSPDT_TX_PARAMS_TABLE_5, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "FPM_GLOBAL_CFG", 1, CORE_5_INDEX, &FPM_GLOBAL_CFG_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TCPSPDTEST_UPDATE_FIFO_TABLE", 1, CORE_5_INDEX, &TCPSPDTEST_UPDATE_FIFO_TABLE_5, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "DEBUG_PRINT_TABLE", 1, CORE_5_INDEX, &DEBUG_PRINT_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TCAM_GENERIC_FIELDS", 1, CORE_5_INDEX, &TCAM_GENERIC_FIELDS_5, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_5_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_5, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "GDX_PARAMS_TABLE", 1, CORE_5_INDEX, &GDX_PARAMS_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_5_INDEX, &TCAM_TABLE_CFG_TABLE_5, 2, 1, 1 },
#endif
#if defined BCM68375
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_5_INDEX, &NATC_L2_VLAN_KEY_MASK_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_5_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_5, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "DEBUG_SCRATCHPAD", 1, CORE_5_INDEX, &DEBUG_SCRATCHPAD_5, 12, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_CFG", 1, CORE_5_INDEX, &INGRESS_FILTER_CFG_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "NATC_L2_TOS_MASK", 1, CORE_5_INDEX, &NATC_L2_TOS_MASK_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_5_INDEX, &FW_ERROR_VECTOR_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "PKTGEN_FPM_UG_MGMT", 1, CORE_5_INDEX, &PKTGEN_FPM_UG_MGMT_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "MULTICAST_KEY_MASK", 1, CORE_5_INDEX, &MULTICAST_KEY_MASK_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_5_INDEX, &BITS_CALC_MASKS_TABLE_5, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "PKTGEN_TX_STREAM_SCRATCH_TABLE", 1, CORE_5_INDEX, &PKTGEN_TX_STREAM_SCRATCH_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "POLICER_PARAMS_TABLE", 1, CORE_6_INDEX, &POLICER_PARAMS_TABLE_6, 256, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING0_STACK", 1, CORE_6_INDEX, &PROCESSING0_STACK_6, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "SCORE_CFG_TABLE", 1, CORE_6_INDEX, &SCORE_CFG_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SPDTEST_GEN_PARAM", 1, CORE_6_INDEX, &SPDTEST_GEN_PARAM_6, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "SCORE_DATA_COLLECTION_PROFILE_TABLE", 1, CORE_6_INDEX, &SCORE_DATA_COLLECTION_PROFILE_TABLE_6, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_6_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_6, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING1_STACK", 1, CORE_6_INDEX, &PROCESSING1_STACK_6, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "SYSTEM_CONFIGURATION", 1, CORE_6_INDEX, &SYSTEM_CONFIGURATION_6, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_6_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_6, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_CFG_TABLE", 1, CORE_6_INDEX, &VPORT_CFG_TABLE_6, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "SCORE_COLLECT_DROP_BITMAP_TABLE", 1, CORE_6_INDEX, &SCORE_COLLECT_DROP_BITMAP_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING2_STACK", 1, CORE_6_INDEX, &PROCESSING2_STACK_6, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "REGISTERS_BUFFER", 1, CORE_6_INDEX, &REGISTERS_BUFFER_6, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "TUNNELS_PARSING_CFG", 1, CORE_6_INDEX, &TUNNELS_PARSING_CFG_6, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_6_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "MUTEX_TABLE", 1, CORE_6_INDEX, &MUTEX_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING3_STACK", 1, CORE_6_INDEX, &PROCESSING3_STACK_6, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_6_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "CSO_BAD_IPV4_HDR_CSUM_PACKETS", 1, CORE_6_INDEX, &CSO_BAD_IPV4_HDR_CSUM_PACKETS_6, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_FPM_THRESHOLDS", 1, CORE_6_INDEX, &DHD_FPM_THRESHOLDS_6, 3, 1, 1 },
#endif
#if defined BCM68375
	{ "TASK_IDX", 1, CORE_6_INDEX, &TASK_IDX_6, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_MIRRORING_TABLE", 1, CORE_6_INDEX, &RX_MIRRORING_TABLE_6, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "DOS_DROP_REASONS_CFG", 1, CORE_6_INDEX, &DOS_DROP_REASONS_CFG_6, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "HOST_TX_TRUNCATE_MIRRORING_TABLE", 1, CORE_6_INDEX, &HOST_TX_TRUNCATE_MIRRORING_TABLE_6, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_6_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING4_STACK", 1, CORE_6_INDEX, &PROCESSING4_STACK_6, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "IPTV_CFG_TABLE", 1, CORE_6_INDEX, &IPTV_CFG_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DEBUG_PRINT_TABLE", 1, CORE_6_INDEX, &DEBUG_PRINT_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_6_INDEX, &LOOPBACK_QUEUE_TABLE_6, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_6_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_6, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_6_INDEX, &LOOPBACK_WAN_FLOW_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_CFG_EX_TABLE", 1, CORE_6_INDEX, &VPORT_CFG_EX_TABLE_6, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "FORCE_DSCP", 1, CORE_6_INDEX, &FORCE_DSCP_6, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "CORE_ID_TABLE", 1, CORE_6_INDEX, &CORE_ID_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING5_STACK", 1, CORE_6_INDEX, &PROCESSING5_STACK_6, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "TCAM_GENERIC_FIELDS", 1, CORE_6_INDEX, &TCAM_GENERIC_FIELDS_6, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_6_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_6, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TR471_SPDSVC_RX_PKT_ID_TABLE", 1, CORE_6_INDEX, &TR471_SPDSVC_RX_PKT_ID_TABLE_6, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "GDX_PARAMS_TABLE", 1, CORE_6_INDEX, &GDX_PARAMS_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "LLQ_SELECTOR_ECN_MASK", 1, CORE_6_INDEX, &LLQ_SELECTOR_ECN_MASK_6, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_6_INDEX, &DEBUG_PRINT_CORE_LOCK_6, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_6_INDEX, &RX_MIRRORING_CONFIGURATION_6, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_6_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_6, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_6_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_6, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_6_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER_6, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING6_STACK", 1, CORE_6_INDEX, &PROCESSING6_STACK_6, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "NAT_CACHE_CFG", 1, CORE_6_INDEX, &NAT_CACHE_CFG_6, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SRAM_DUMMY_STORE", 1, CORE_6_INDEX, &SRAM_DUMMY_STORE_6, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_6_INDEX, &NAT_CACHE_KEY0_MASK_6, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_6_INDEX, &TX_MIRRORING_CONFIGURATION_6, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "CSO_DISABLE", 1, CORE_6_INDEX, &CSO_DISABLE_6, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_6_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_6_INDEX, &TCAM_TABLE_CFG_TABLE_6, 2, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_6_INDEX, &INGRESS_FILTER_1588_CFG_6, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_6_INDEX, &RX_MIRRORING_DIRECT_ENABLE_6, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_6_INDEX, &BUFMNG_DESCRIPTOR_TABLE_6, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFMNG_STATUS_TABLE", 1, CORE_6_INDEX, &BUFMNG_STATUS_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_6_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_6, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_HW_CFG", 1, CORE_6_INDEX, &DHD_HW_CFG_6, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_6_INDEX, &NATC_L2_VLAN_KEY_MASK_6, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING7_STACK", 1, CORE_6_INDEX, &PROCESSING7_STACK_6, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_CFG", 1, CORE_6_INDEX, &INGRESS_FILTER_CFG_6, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "NATC_L2_TOS_MASK", 1, CORE_6_INDEX, &NATC_L2_TOS_MASK_6, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_6_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_6, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "DEBUG_SCRATCHPAD", 1, CORE_6_INDEX, &DEBUG_SCRATCHPAD_6, 12, 1, 1 },
#endif
#if defined BCM68375
	{ "FPM_GLOBAL_CFG", 1, CORE_6_INDEX, &FPM_GLOBAL_CFG_6, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_FLOW_TABLE", 1, CORE_6_INDEX, &RX_FLOW_TABLE_6, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "TX_FLOW_TABLE", 1, CORE_6_INDEX, &TX_FLOW_TABLE_6, 212, 1, 1 },
#endif
#if defined BCM68375
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_6_INDEX, &FW_ERROR_VECTOR_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_6_INDEX, &INGRESS_FILTER_PROFILE_TABLE_6, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_6_INDEX, &BITS_CALC_MASKS_TABLE_6, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "MULTICAST_KEY_MASK", 1, CORE_6_INDEX, &MULTICAST_KEY_MASK_6, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "POLICER_PARAMS_TABLE", 1, CORE_7_INDEX, &POLICER_PARAMS_TABLE_7, 256, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING0_STACK", 1, CORE_7_INDEX, &PROCESSING0_STACK_7, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "SCORE_CFG_TABLE", 1, CORE_7_INDEX, &SCORE_CFG_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SPDTEST_GEN_PARAM", 1, CORE_7_INDEX, &SPDTEST_GEN_PARAM_7, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "SCORE_DATA_COLLECTION_PROFILE_TABLE", 1, CORE_7_INDEX, &SCORE_DATA_COLLECTION_PROFILE_TABLE_7, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_7_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_7, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING1_STACK", 1, CORE_7_INDEX, &PROCESSING1_STACK_7, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "SYSTEM_CONFIGURATION", 1, CORE_7_INDEX, &SYSTEM_CONFIGURATION_7, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_7_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_7, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_CFG_TABLE", 1, CORE_7_INDEX, &VPORT_CFG_TABLE_7, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "SCORE_COLLECT_DROP_BITMAP_TABLE", 1, CORE_7_INDEX, &SCORE_COLLECT_DROP_BITMAP_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING2_STACK", 1, CORE_7_INDEX, &PROCESSING2_STACK_7, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "REGISTERS_BUFFER", 1, CORE_7_INDEX, &REGISTERS_BUFFER_7, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "TUNNELS_PARSING_CFG", 1, CORE_7_INDEX, &TUNNELS_PARSING_CFG_7, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_7_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "MUTEX_TABLE", 1, CORE_7_INDEX, &MUTEX_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING3_STACK", 1, CORE_7_INDEX, &PROCESSING3_STACK_7, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_7_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "CSO_BAD_IPV4_HDR_CSUM_PACKETS", 1, CORE_7_INDEX, &CSO_BAD_IPV4_HDR_CSUM_PACKETS_7, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_FPM_THRESHOLDS", 1, CORE_7_INDEX, &DHD_FPM_THRESHOLDS_7, 3, 1, 1 },
#endif
#if defined BCM68375
	{ "TASK_IDX", 1, CORE_7_INDEX, &TASK_IDX_7, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_MIRRORING_TABLE", 1, CORE_7_INDEX, &RX_MIRRORING_TABLE_7, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "DOS_DROP_REASONS_CFG", 1, CORE_7_INDEX, &DOS_DROP_REASONS_CFG_7, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "HOST_TX_TRUNCATE_MIRRORING_TABLE", 1, CORE_7_INDEX, &HOST_TX_TRUNCATE_MIRRORING_TABLE_7, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_7_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING4_STACK", 1, CORE_7_INDEX, &PROCESSING4_STACK_7, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "IPTV_CFG_TABLE", 1, CORE_7_INDEX, &IPTV_CFG_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DEBUG_PRINT_TABLE", 1, CORE_7_INDEX, &DEBUG_PRINT_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_7_INDEX, &LOOPBACK_QUEUE_TABLE_7, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_7_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_7, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_7_INDEX, &LOOPBACK_WAN_FLOW_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_CFG_EX_TABLE", 1, CORE_7_INDEX, &VPORT_CFG_EX_TABLE_7, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "FORCE_DSCP", 1, CORE_7_INDEX, &FORCE_DSCP_7, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "CORE_ID_TABLE", 1, CORE_7_INDEX, &CORE_ID_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING5_STACK", 1, CORE_7_INDEX, &PROCESSING5_STACK_7, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "TCAM_GENERIC_FIELDS", 1, CORE_7_INDEX, &TCAM_GENERIC_FIELDS_7, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_7_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_7, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TR471_SPDSVC_RX_PKT_ID_TABLE", 1, CORE_7_INDEX, &TR471_SPDSVC_RX_PKT_ID_TABLE_7, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "GDX_PARAMS_TABLE", 1, CORE_7_INDEX, &GDX_PARAMS_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "LLQ_SELECTOR_ECN_MASK", 1, CORE_7_INDEX, &LLQ_SELECTOR_ECN_MASK_7, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_7_INDEX, &DEBUG_PRINT_CORE_LOCK_7, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_7_INDEX, &RX_MIRRORING_CONFIGURATION_7, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_7_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_7, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_7_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_7, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_7_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER_7, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING6_STACK", 1, CORE_7_INDEX, &PROCESSING6_STACK_7, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "NAT_CACHE_CFG", 1, CORE_7_INDEX, &NAT_CACHE_CFG_7, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SRAM_DUMMY_STORE", 1, CORE_7_INDEX, &SRAM_DUMMY_STORE_7, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_7_INDEX, &NAT_CACHE_KEY0_MASK_7, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_7_INDEX, &TX_MIRRORING_CONFIGURATION_7, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "CSO_DISABLE", 1, CORE_7_INDEX, &CSO_DISABLE_7, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_7_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_7_INDEX, &TCAM_TABLE_CFG_TABLE_7, 2, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_7_INDEX, &INGRESS_FILTER_1588_CFG_7, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_7_INDEX, &RX_MIRRORING_DIRECT_ENABLE_7, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_7_INDEX, &BUFMNG_DESCRIPTOR_TABLE_7, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFMNG_STATUS_TABLE", 1, CORE_7_INDEX, &BUFMNG_STATUS_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_7_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_7, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_HW_CFG", 1, CORE_7_INDEX, &DHD_HW_CFG_7, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_7_INDEX, &NATC_L2_VLAN_KEY_MASK_7, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING7_STACK", 1, CORE_7_INDEX, &PROCESSING7_STACK_7, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_CFG", 1, CORE_7_INDEX, &INGRESS_FILTER_CFG_7, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "NATC_L2_TOS_MASK", 1, CORE_7_INDEX, &NATC_L2_TOS_MASK_7, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_7_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_7, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "DEBUG_SCRATCHPAD", 1, CORE_7_INDEX, &DEBUG_SCRATCHPAD_7, 12, 1, 1 },
#endif
#if defined BCM68375
	{ "FPM_GLOBAL_CFG", 1, CORE_7_INDEX, &FPM_GLOBAL_CFG_7, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_FLOW_TABLE", 1, CORE_7_INDEX, &RX_FLOW_TABLE_7, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "TX_FLOW_TABLE", 1, CORE_7_INDEX, &TX_FLOW_TABLE_7, 212, 1, 1 },
#endif
#if defined BCM68375
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_7_INDEX, &FW_ERROR_VECTOR_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_7_INDEX, &INGRESS_FILTER_PROFILE_TABLE_7, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_7_INDEX, &BITS_CALC_MASKS_TABLE_7, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "MULTICAST_KEY_MASK", 1, CORE_7_INDEX, &MULTICAST_KEY_MASK_7, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "POLICER_PARAMS_TABLE", 1, CORE_8_INDEX, &POLICER_PARAMS_TABLE_8, 256, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING0_STACK", 1, CORE_8_INDEX, &PROCESSING0_STACK_8, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "SCORE_CFG_TABLE", 1, CORE_8_INDEX, &SCORE_CFG_TABLE_8, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SPDTEST_GEN_PARAM", 1, CORE_8_INDEX, &SPDTEST_GEN_PARAM_8, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "SCORE_DATA_COLLECTION_PROFILE_TABLE", 1, CORE_8_INDEX, &SCORE_DATA_COLLECTION_PROFILE_TABLE_8, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_8_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_8, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING1_STACK", 1, CORE_8_INDEX, &PROCESSING1_STACK_8, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "SYSTEM_CONFIGURATION", 1, CORE_8_INDEX, &SYSTEM_CONFIGURATION_8, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SPDSVC_ANALYZER_PARAMS_TABLE", 1, CORE_8_INDEX, &SPDSVC_ANALYZER_PARAMS_TABLE_8, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_CFG_TABLE", 1, CORE_8_INDEX, &VPORT_CFG_TABLE_8, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "SCORE_COLLECT_DROP_BITMAP_TABLE", 1, CORE_8_INDEX, &SCORE_COLLECT_DROP_BITMAP_TABLE_8, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING2_STACK", 1, CORE_8_INDEX, &PROCESSING2_STACK_8, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "TUNNELS_PARSING_CFG", 1, CORE_8_INDEX, &TUNNELS_PARSING_CFG_8, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_8_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_8, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "SPDSVC_ANALYZER_PD_FIFO_TABLE", 1, CORE_8_INDEX, &SPDSVC_ANALYZER_PD_FIFO_TABLE_8, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_MIRRORING_TABLE", 1, CORE_8_INDEX, &RX_MIRRORING_TABLE_8, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "DOS_DROP_REASONS_CFG", 1, CORE_8_INDEX, &DOS_DROP_REASONS_CFG_8, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING3_STACK", 1, CORE_8_INDEX, &PROCESSING3_STACK_8, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "REGISTERS_BUFFER", 1, CORE_8_INDEX, &REGISTERS_BUFFER_8, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "MUTEX_TABLE", 1, CORE_8_INDEX, &MUTEX_TABLE_8, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_8_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_8, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_8_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_8, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "CSO_BAD_IPV4_HDR_CSUM_PACKETS", 1, CORE_8_INDEX, &CSO_BAD_IPV4_HDR_CSUM_PACKETS_8, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING4_STACK", 1, CORE_8_INDEX, &PROCESSING4_STACK_8, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "IPTV_CFG_TABLE", 1, CORE_8_INDEX, &IPTV_CFG_TABLE_8, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_FPM_THRESHOLDS", 1, CORE_8_INDEX, &DHD_FPM_THRESHOLDS_8, 3, 1, 1 },
#endif
#if defined BCM68375
	{ "TASK_IDX", 1, CORE_8_INDEX, &TASK_IDX_8, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "HOST_TX_TRUNCATE_MIRRORING_TABLE", 1, CORE_8_INDEX, &HOST_TX_TRUNCATE_MIRRORING_TABLE_8, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_8_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_8, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_8_INDEX, &LOOPBACK_QUEUE_TABLE_8, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_8_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_8, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_8_INDEX, &LOOPBACK_WAN_FLOW_TABLE_8, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING5_STACK", 1, CORE_8_INDEX, &PROCESSING5_STACK_8, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_8_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_8, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "UDPSPDT_SCRATCH_TABLE", 1, CORE_8_INDEX, &UDPSPDT_SCRATCH_TABLE_8, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "NAT_CACHE_CFG", 1, CORE_8_INDEX, &NAT_CACHE_CFG_8, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "FORCE_DSCP", 1, CORE_8_INDEX, &FORCE_DSCP_8, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_CFG_EX_TABLE", 1, CORE_8_INDEX, &VPORT_CFG_EX_TABLE_8, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "CORE_ID_TABLE", 1, CORE_8_INDEX, &CORE_ID_TABLE_8, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "LLQ_SELECTOR_ECN_MASK", 1, CORE_8_INDEX, &LLQ_SELECTOR_ECN_MASK_8, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TR471_SPDSVC_RX_PKT_ID_TABLE", 1, CORE_8_INDEX, &TR471_SPDSVC_RX_PKT_ID_TABLE_8, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "DEBUG_PRINT_TABLE", 1, CORE_8_INDEX, &DEBUG_PRINT_TABLE_8, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_8_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER_8, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING6_STACK", 1, CORE_8_INDEX, &PROCESSING6_STACK_8, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "TCAM_GENERIC_FIELDS", 1, CORE_8_INDEX, &TCAM_GENERIC_FIELDS_8, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_8_INDEX, &NAT_CACHE_KEY0_MASK_8, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_8_INDEX, &RX_MIRRORING_CONFIGURATION_8, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_8_INDEX, &TX_MIRRORING_CONFIGURATION_8, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_8_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_8, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_8_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_8, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_8_INDEX, &BUFMNG_DESCRIPTOR_TABLE_8, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFMNG_STATUS_TABLE", 1, CORE_8_INDEX, &BUFMNG_STATUS_TABLE_8, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING7_STACK", 1, CORE_8_INDEX, &PROCESSING7_STACK_8, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_8_INDEX, &TCAM_TABLE_CFG_TABLE_8, 2, 1, 1 },
#endif
#if defined BCM68375
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_8_INDEX, &DEBUG_PRINT_CORE_LOCK_8, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SRAM_DUMMY_STORE", 1, CORE_8_INDEX, &SRAM_DUMMY_STORE_8, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "CSO_DISABLE", 1, CORE_8_INDEX, &CSO_DISABLE_8, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_8_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_8, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_8_INDEX, &INGRESS_FILTER_1588_CFG_8, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_8_INDEX, &RX_MIRRORING_DIRECT_ENABLE_8, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "GDX_PARAMS_TABLE", 1, CORE_8_INDEX, &GDX_PARAMS_TABLE_8, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SPDSVC_ANALYZER_UPDATE_FIFO_TABLE", 1, CORE_8_INDEX, &SPDSVC_ANALYZER_UPDATE_FIFO_TABLE_8, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_HW_CFG", 1, CORE_8_INDEX, &DHD_HW_CFG_8, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_8_INDEX, &NATC_L2_VLAN_KEY_MASK_8, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_8_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_8, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "FPM_GLOBAL_CFG", 1, CORE_8_INDEX, &FPM_GLOBAL_CFG_8, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_FLOW_TABLE", 1, CORE_8_INDEX, &RX_FLOW_TABLE_8, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "TX_FLOW_TABLE", 1, CORE_8_INDEX, &TX_FLOW_TABLE_8, 212, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_CFG", 1, CORE_8_INDEX, &INGRESS_FILTER_CFG_8, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_8_INDEX, &FW_ERROR_VECTOR_TABLE_8, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "NATC_L2_TOS_MASK", 1, CORE_8_INDEX, &NATC_L2_TOS_MASK_8, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "UDPSPDT_STREAM_RX_STAT_TABLE", 1, CORE_8_INDEX, &UDPSPDT_STREAM_RX_STAT_TABLE_8, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_8_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_8, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "MULTICAST_KEY_MASK", 1, CORE_8_INDEX, &MULTICAST_KEY_MASK_8, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SPDSVC_ANALYZER_STACK", 1, CORE_8_INDEX, &SPDSVC_ANALYZER_STACK_8, 176, 1, 1 },
#endif
#if defined BCM68375
	{ "DEBUG_SCRATCHPAD", 1, CORE_8_INDEX, &DEBUG_SCRATCHPAD_8, 12, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_8_INDEX, &INGRESS_FILTER_PROFILE_TABLE_8, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_8_INDEX, &BITS_CALC_MASKS_TABLE_8, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "POLICER_PARAMS_TABLE", 1, CORE_9_INDEX, &POLICER_PARAMS_TABLE_9, 256, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING0_STACK", 1, CORE_9_INDEX, &PROCESSING0_STACK_9, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "SCORE_INTERRUPT_ID_DDR_ADDR", 1, CORE_9_INDEX, &SCORE_INTERRUPT_ID_DDR_ADDR_9, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SPDTEST_GEN_PARAM", 1, CORE_9_INDEX, &SPDTEST_GEN_PARAM_9, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "DATA_TRACE_SCRATCHPAD", 1, CORE_9_INDEX, &DATA_TRACE_SCRATCHPAD_9, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_9_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_9, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING1_STACK", 1, CORE_9_INDEX, &PROCESSING1_STACK_9, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "SCORE_CFG_TABLE", 1, CORE_9_INDEX, &SCORE_CFG_TABLE_9, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_9_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_9, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "SCORE_DATA_COLLECTION_PROFILE_TABLE", 1, CORE_9_INDEX, &SCORE_DATA_COLLECTION_PROFILE_TABLE_9, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING2_STACK", 1, CORE_9_INDEX, &PROCESSING2_STACK_9, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "SYSTEM_CONFIGURATION", 1, CORE_9_INDEX, &SYSTEM_CONFIGURATION_9, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SPU_RESPONSE_DISPATCHER_CREDIT_TABLE", 1, CORE_9_INDEX, &SPU_RESPONSE_DISPATCHER_CREDIT_TABLE_9, 3, 1, 1 },
#endif
#if defined BCM68375
	{ "SCORE_QSTAT_SEQ_NUM", 1, CORE_9_INDEX, &SCORE_QSTAT_SEQ_NUM_9, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "GENERAL_TIMER", 1, CORE_9_INDEX, &GENERAL_TIMER_9, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING3_STACK", 1, CORE_9_INDEX, &PROCESSING3_STACK_9, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "CPU_RX_INTERRUPT_ID_DDR_ADDR", 1, CORE_9_INDEX, &CPU_RX_INTERRUPT_ID_DDR_ADDR_9, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_9_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_9, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SCORE_COLLECT_DROP_BITMAP_TABLE", 1, CORE_9_INDEX, &SCORE_COLLECT_DROP_BITMAP_TABLE_9, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_CFG_TABLE", 1, CORE_9_INDEX, &VPORT_CFG_TABLE_9, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "MUTEX_TABLE", 1, CORE_9_INDEX, &MUTEX_TABLE_9, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING4_STACK", 1, CORE_9_INDEX, &PROCESSING4_STACK_9, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "SCORE_COALESCING_TABLE", 1, CORE_9_INDEX, &SCORE_COALESCING_TABLE_9, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DOS_DROP_REASONS_CFG", 1, CORE_9_INDEX, &DOS_DROP_REASONS_CFG_9, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_FPM_THRESHOLDS", 1, CORE_9_INDEX, &DHD_FPM_THRESHOLDS_9, 3, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_9_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_9, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SPU_RESPONSE_STACK", 1, CORE_9_INDEX, &SPU_RESPONSE_STACK_9, 96, 1, 1 },
#endif
#if defined BCM68375
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_9_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_9, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING5_STACK", 1, CORE_9_INDEX, &PROCESSING5_STACK_9, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "TUNNELS_PARSING_CFG", 1, CORE_9_INDEX, &TUNNELS_PARSING_CFG_9, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DEBUG_PRINT_TABLE", 1, CORE_9_INDEX, &DEBUG_PRINT_TABLE_9, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "GENERAL_TIMER_STACK", 1, CORE_9_INDEX, &GENERAL_TIMER_STACK_9, 72, 1, 1 },
#endif
#if defined BCM68375
	{ "TCAM_GENERIC_FIELDS", 1, CORE_9_INDEX, &TCAM_GENERIC_FIELDS_9, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "CSO_BAD_IPV4_HDR_CSUM_PACKETS", 1, CORE_9_INDEX, &CSO_BAD_IPV4_HDR_CSUM_PACKETS_9, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TASK_IDX", 1, CORE_9_INDEX, &TASK_IDX_9, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_9_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_9, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_9_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER_9, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING6_STACK", 1, CORE_9_INDEX, &PROCESSING6_STACK_9, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "REGISTERS_BUFFER", 1, CORE_9_INDEX, &REGISTERS_BUFFER_9, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "IPTV_CFG_TABLE", 1, CORE_9_INDEX, &IPTV_CFG_TABLE_9, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "GDX_PARAMS_TABLE", 1, CORE_9_INDEX, &GDX_PARAMS_TABLE_9, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "GENERAL_TIMER_ACTION_VEC", 1, CORE_9_INDEX, &GENERAL_TIMER_ACTION_VEC_9, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_9_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_9, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING7_STACK", 1, CORE_9_INDEX, &PROCESSING7_STACK_9, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_9_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_9, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "NAT_CACHE_CFG", 1, CORE_9_INDEX, &NAT_CACHE_CFG_9, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_9_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_9, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_9_INDEX, &NAT_CACHE_KEY0_MASK_9, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_9_INDEX, &LOOPBACK_WAN_FLOW_TABLE_9, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "FORCE_DSCP", 1, CORE_9_INDEX, &FORCE_DSCP_9, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "CORE_ID_TABLE", 1, CORE_9_INDEX, &CORE_ID_TABLE_9, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "LLQ_SELECTOR_ECN_MASK", 1, CORE_9_INDEX, &LLQ_SELECTOR_ECN_MASK_9, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "CRYPTO_SESSION_STATS_SCRATCH", 1, CORE_9_INDEX, &CRYPTO_SESSION_STATS_SCRATCH_9, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_FLOW_TABLE", 1, CORE_9_INDEX, &RX_FLOW_TABLE_9, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "SCORE_SDR_DB_TABLE", 1, CORE_9_INDEX, &SCORE_SDR_DB_TABLE_9, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "TX_FLOW_TABLE", 1, CORE_9_INDEX, &TX_FLOW_TABLE_9, 212, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_9_INDEX, &RX_MIRRORING_CONFIGURATION_9, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_9_INDEX, &TX_MIRRORING_CONFIGURATION_9, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_9_INDEX, &TCAM_TABLE_CFG_TABLE_9, 2, 1, 1 },
#endif
#if defined BCM68375
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_9_INDEX, &DEBUG_PRINT_CORE_LOCK_9, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SRAM_DUMMY_STORE", 1, CORE_9_INDEX, &SRAM_DUMMY_STORE_9, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "CSO_DISABLE", 1, CORE_9_INDEX, &CSO_DISABLE_9, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_9_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_9, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_9_INDEX, &INGRESS_FILTER_1588_CFG_9, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_9_INDEX, &RX_MIRRORING_DIRECT_ENABLE_9, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_9_INDEX, &BUFMNG_DESCRIPTOR_TABLE_9, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_9_INDEX, &INGRESS_FILTER_PROFILE_TABLE_9, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_MIRRORING_TABLE", 1, CORE_9_INDEX, &RX_MIRRORING_TABLE_9, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "HOST_TX_TRUNCATE_MIRRORING_TABLE", 1, CORE_9_INDEX, &HOST_TX_TRUNCATE_MIRRORING_TABLE_9, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_9_INDEX, &LOOPBACK_QUEUE_TABLE_9, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_CFG_EX_TABLE", 1, CORE_9_INDEX, &VPORT_CFG_EX_TABLE_9, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "SPU_RESPONSE_PARAMS_TABLE", 1, CORE_9_INDEX, &SPU_RESPONSE_PARAMS_TABLE_9, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_9_INDEX, &NATC_L2_VLAN_KEY_MASK_9, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TR471_SPDSVC_RX_PKT_ID_TABLE", 1, CORE_9_INDEX, &TR471_SPDSVC_RX_PKT_ID_TABLE_9, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_CFG", 1, CORE_9_INDEX, &INGRESS_FILTER_CFG_9, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "NATC_L2_TOS_MASK", 1, CORE_9_INDEX, &NATC_L2_TOS_MASK_9, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DATA_TRACE_STACK", 1, CORE_9_INDEX, &DATA_TRACE_STACK_9, 40, 1, 1 },
#endif
#if defined BCM68375
	{ "SPU_RESPONSE_SCRATCHPAD", 1, CORE_9_INDEX, &SPU_RESPONSE_SCRATCHPAD_9, 9, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFMNG_STATUS_TABLE", 1, CORE_9_INDEX, &BUFMNG_STATUS_TABLE_9, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_HW_CFG", 1, CORE_9_INDEX, &DHD_HW_CFG_9, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DATA_TRACE_UPDATE_FIFO_TABLE", 1, CORE_9_INDEX, &DATA_TRACE_UPDATE_FIFO_TABLE_9, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "FPM_GLOBAL_CFG", 1, CORE_9_INDEX, &FPM_GLOBAL_CFG_9, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DATA_TRACE_PD_FIFO_TABLE", 1, CORE_9_INDEX, &DATA_TRACE_PD_FIFO_TABLE_9, 2, 1, 1 },
#endif
#if defined BCM68375
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_9_INDEX, &FW_ERROR_VECTOR_TABLE_9, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_9_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_9, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "MULTICAST_KEY_MASK", 1, CORE_9_INDEX, &MULTICAST_KEY_MASK_9, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_9_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_9, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "DEBUG_SCRATCHPAD", 1, CORE_9_INDEX, &DEBUG_SCRATCHPAD_9, 12, 1, 1 },
#endif
#if defined BCM68375
	{ "SCORE_MSG_RING_DESCRIPTOR_TABLE", 1, CORE_9_INDEX, &SCORE_MSG_RING_DESCRIPTOR_TABLE_9, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SCORE_QSTAT_RING_DESCRIPTOR_TABLE", 1, CORE_9_INDEX, &SCORE_QSTAT_RING_DESCRIPTOR_TABLE_9, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SCORE_BNL_RING_DESCRIPTOR_TABLE", 1, CORE_9_INDEX, &SCORE_BNL_RING_DESCRIPTOR_TABLE_9, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_9_INDEX, &BITS_CALC_MASKS_TABLE_9, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "DS_TM_PD_FIFO_TABLE", 1, CORE_10_INDEX, &DS_TM_PD_FIFO_TABLE_10, 160, 1, 1 },
#endif
#if defined BCM68375
	{ "POLICER_PARAMS_TABLE", 1, CORE_10_INDEX, &POLICER_PARAMS_TABLE_10, 256, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_SCHEDULING_QUEUE_TABLE", 1, CORE_10_INDEX, &ETH_TM_SCHEDULING_QUEUE_TABLE_10, 80, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_RATE_LIMITER_PROFILE_RESIDUE_TABLE", 1, CORE_10_INDEX, &ETH_TM_RATE_LIMITER_PROFILE_RESIDUE_TABLE_10, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "DS_TM_TM_FLOW_CNTR_TABLE", 1, CORE_10_INDEX, &DS_TM_TM_FLOW_CNTR_TABLE_10, 64, 1, 1 },
#endif
#if defined BCM68375
	{ "TX_TASK_0_STACK", 1, CORE_10_INDEX, &TX_TASK_0_STACK_10, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING_4_TASKS_PACKET_BUFFER", 1, CORE_10_INDEX, &PROCESSING_4_TASKS_PACKET_BUFFER_10, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_RATE_LIMITER_BUDGET_DESCRIPTOR_TABLE", 1, CORE_10_INDEX, &ETH_TM_RATE_LIMITER_BUDGET_DESCRIPTOR_TABLE_10, 80, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_RATE_LIMITER_PROFILE_TABLE", 1, CORE_10_INDEX, &ETH_TM_RATE_LIMITER_PROFILE_TABLE_10, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_FLOW_TABLE", 1, CORE_10_INDEX, &RX_FLOW_TABLE_10, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_RATE_LIMITER_PARAMS_DESCRIPTOR_TABLE", 1, CORE_10_INDEX, &ETH_TM_RATE_LIMITER_PARAMS_DESCRIPTOR_TABLE_10, 80, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_DCL_IMG", 1, CORE_10_INDEX, &ETH_TM_DCL_IMG_10, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "TX_FLOW_TABLE", 1, CORE_10_INDEX, &TX_FLOW_TABLE_10, 212, 1, 1 },
#endif
#if defined BCM68375
	{ "SCORE_COLLECT_DROP_BITMAP_TABLE", 1, CORE_10_INDEX, &SCORE_COLLECT_DROP_BITMAP_TABLE_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DS_TM_BBH_TX_WAKE_UP_DATA_TABLE", 1, CORE_10_INDEX, &DS_TM_BBH_TX_WAKE_UP_DATA_TABLE_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_10_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_10, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_10_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_10, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING0_STACK", 1, CORE_10_INDEX, &PROCESSING0_STACK_10, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "SCORE_CFG_TABLE", 1, CORE_10_INDEX, &SCORE_CFG_TABLE_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_RATE_LIMITER_BUDGET_VALID", 1, CORE_10_INDEX, &ETH_TM_RATE_LIMITER_BUDGET_VALID_10, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "SCORE_DATA_COLLECTION_PROFILE_TABLE", 1, CORE_10_INDEX, &SCORE_DATA_COLLECTION_PROFILE_TABLE_10, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING1_STACK", 1, CORE_10_INDEX, &PROCESSING1_STACK_10, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "SYSTEM_CONFIGURATION", 1, CORE_10_INDEX, &SYSTEM_CONFIGURATION_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SPDTEST_GEN_PARAM", 1, CORE_10_INDEX, &SPDTEST_GEN_PARAM_10, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "GENERAL_TIMER", 1, CORE_10_INDEX, &GENERAL_TIMER_10, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING2_STACK", 1, CORE_10_INDEX, &PROCESSING2_STACK_10, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "MIRRORING_SCRATCH", 1, CORE_10_INDEX, &MIRRORING_SCRATCH_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_10_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_10, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_CFG_TABLE", 1, CORE_10_INDEX, &VPORT_CFG_TABLE_10, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "MUTEX_TABLE", 1, CORE_10_INDEX, &MUTEX_TABLE_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING3_STACK", 1, CORE_10_INDEX, &PROCESSING3_STACK_10, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "TUNNELS_PARSING_CFG", 1, CORE_10_INDEX, &TUNNELS_PARSING_CFG_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_10_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_10_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_SECONDARY_SCHEDULER_TABLE", 1, CORE_10_INDEX, &ETH_TM_SECONDARY_SCHEDULER_TABLE_10, 7, 1, 1 },
#endif
#if defined BCM68375
	{ "CSO_BAD_IPV4_HDR_CSUM_PACKETS", 1, CORE_10_INDEX, &CSO_BAD_IPV4_HDR_CSUM_PACKETS_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "IPTV_CFG_TABLE", 1, CORE_10_INDEX, &IPTV_CFG_TABLE_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_10_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_10, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_AQM_QUEUE_TABLE", 1, CORE_10_INDEX, &ETH_TM_AQM_QUEUE_TABLE_10, 80, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_SCHEDULER_POOL", 1, CORE_10_INDEX, &ETH_TM_SCHEDULER_POOL_10, 160, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_10_INDEX, &BUFMNG_DESCRIPTOR_TABLE_10, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_SCHEDULER_TABLE", 1, CORE_10_INDEX, &ETH_TM_SCHEDULER_TABLE_10, 7, 1, 1 },
#endif
#if defined BCM68375
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_10_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_FPM_THRESHOLDS", 1, CORE_10_INDEX, &DHD_FPM_THRESHOLDS_10, 3, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_DCL_MULTIPLE_CNTR", 1, CORE_10_INDEX, &ETH_TM_DCL_MULTIPLE_CNTR_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "UPDATE_FIFO_STACK", 1, CORE_10_INDEX, &UPDATE_FIFO_STACK_10, 64, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_10_INDEX, &INGRESS_FILTER_PROFILE_TABLE_10, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_AQM_QUEUE_TIMER_TABLE", 1, CORE_10_INDEX, &ETH_TM_AQM_QUEUE_TIMER_TABLE_10, 80, 1, 1 },
#endif
#if defined BCM68375
	{ "TX_MIRRORING_DISPATCHER_CREDIT_TABLE", 1, CORE_10_INDEX, &TX_MIRRORING_DISPATCHER_CREDIT_TABLE_10, 3, 1, 1 },
#endif
#if defined BCM68375
	{ "TASK_IDX", 1, CORE_10_INDEX, &TASK_IDX_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFMNG_STATUS_TABLE", 1, CORE_10_INDEX, &BUFMNG_STATUS_TABLE_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "GENERAL_TIMER_STACK", 1, CORE_10_INDEX, &GENERAL_TIMER_STACK_10, 72, 1, 1 },
#endif
#if defined BCM68375
	{ "REGISTERS_BUFFER", 1, CORE_10_INDEX, &REGISTERS_BUFFER_10, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "DS_TM_CPU_TX_ABS_COUNTERS", 1, CORE_10_INDEX, &DS_TM_CPU_TX_ABS_COUNTERS_10, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "NAT_CACHE_CFG", 1, CORE_10_INDEX, &NAT_CACHE_CFG_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_10_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DEBUG_PRINT_TABLE", 1, CORE_10_INDEX, &DEBUG_PRINT_TABLE_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_FW_FLUSH_TABLE", 1, CORE_10_INDEX, &ETH_TM_FW_FLUSH_TABLE_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "BUDGET_ALLOCATION_TIMER_VALUE", 1, CORE_10_INDEX, &BUDGET_ALLOCATION_TIMER_VALUE_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DOS_DROP_REASONS_CFG", 1, CORE_10_INDEX, &DOS_DROP_REASONS_CFG_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFFER_CONG_SCRATCHPAD", 1, CORE_10_INDEX, &BUFFER_CONG_SCRATCHPAD_10, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_MIRRORING_TABLE", 1, CORE_10_INDEX, &RX_MIRRORING_TABLE_10, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "GENERAL_TIMER_ACTION_VEC", 1, CORE_10_INDEX, &GENERAL_TIMER_ACTION_VEC_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "HOST_TX_TRUNCATE_MIRRORING_TABLE", 1, CORE_10_INDEX, &HOST_TX_TRUNCATE_MIRRORING_TABLE_10, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_10_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_TX_TRUNCATE_MIRRORING_TABLE", 1, CORE_10_INDEX, &ETH_TM_TX_TRUNCATE_MIRRORING_TABLE_10, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "DS_TM_BB_DESTINATION_TABLE", 1, CORE_10_INDEX, &DS_TM_BB_DESTINATION_TABLE_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_10_INDEX, &LOOPBACK_QUEUE_TABLE_10, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_10_INDEX, &LOOPBACK_WAN_FLOW_TABLE_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "FORCE_DSCP", 1, CORE_10_INDEX, &FORCE_DSCP_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_CFG_EX_TABLE", 1, CORE_10_INDEX, &VPORT_CFG_EX_TABLE_10, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "DS_TM_FIRST_QUEUE_MAPPING", 1, CORE_10_INDEX, &DS_TM_FIRST_QUEUE_MAPPING_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFFER_CONG_DQM_NOT_EMPTY", 1, CORE_10_INDEX, &BUFFER_CONG_DQM_NOT_EMPTY_10, 13, 1, 1 },
#endif
#if defined BCM68375
	{ "CORE_ID_TABLE", 1, CORE_10_INDEX, &CORE_ID_TABLE_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "LLQ_SELECTOR_ECN_MASK", 1, CORE_10_INDEX, &LLQ_SELECTOR_ECN_MASK_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TX_EXCEPTION", 1, CORE_10_INDEX, &TX_EXCEPTION_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_10_INDEX, &DEBUG_PRINT_CORE_LOCK_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_10_INDEX, &NAT_CACHE_KEY0_MASK_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_10_INDEX, &RX_MIRRORING_CONFIGURATION_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_10_INDEX, &TX_MIRRORING_CONFIGURATION_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TR471_SPDSVC_RX_PKT_ID_TABLE", 1, CORE_10_INDEX, &TR471_SPDSVC_RX_PKT_ID_TABLE_10, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "TCAM_GENERIC_FIELDS", 1, CORE_10_INDEX, &TCAM_GENERIC_FIELDS_10, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "DS_BUFFER_CONG_MGT_CFG", 1, CORE_10_INDEX, &DS_BUFFER_CONG_MGT_CFG_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "GDX_PARAMS_TABLE", 1, CORE_10_INDEX, &GDX_PARAMS_TABLE_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SRAM_DUMMY_STORE", 1, CORE_10_INDEX, &SRAM_DUMMY_STORE_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "BACKUP_BBH_INGRESS_COUNTERS_TABLE", 1, CORE_10_INDEX, &BACKUP_BBH_INGRESS_COUNTERS_TABLE_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "BACKUP_BBH_EGRESS_COUNTERS_TABLE", 1, CORE_10_INDEX, &BACKUP_BBH_EGRESS_COUNTERS_TABLE_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "CSO_DISABLE", 1, CORE_10_INDEX, &CSO_DISABLE_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_ENABLED_QUEUES_TABLE", 1, CORE_10_INDEX, &ETH_TM_ENABLED_QUEUES_TABLE_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_10_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_10_INDEX, &INGRESS_FILTER_1588_CFG_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_10_INDEX, &RX_MIRRORING_DIRECT_ENABLE_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_10_INDEX, &TCAM_TABLE_CFG_TABLE_10, 2, 1, 1 },
#endif
#if defined BCM68375
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_10_INDEX, &NATC_L2_VLAN_KEY_MASK_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_CFG", 1, CORE_10_INDEX, &INGRESS_FILTER_CFG_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DS_TM_BBH_QUEUE_TABLE", 1, CORE_10_INDEX, &DS_TM_BBH_QUEUE_TABLE_10, 7, 1, 1 },
#endif
#if defined BCM68375
	{ "NATC_L2_TOS_MASK", 1, CORE_10_INDEX, &NATC_L2_TOS_MASK_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DS_TM_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_TABLE", 1, CORE_10_INDEX, &DS_TM_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_TABLE_10, 2, 1, 1 },
#endif
#if defined BCM68375
	{ "DEBUG_SCRATCHPAD", 1, CORE_10_INDEX, &DEBUG_SCRATCHPAD_10, 12, 1, 1 },
#endif
#if defined BCM68375
	{ "UPDATE_FIFO_TABLE", 1, CORE_10_INDEX, &UPDATE_FIFO_TABLE_10, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "DS_TM_PI2_PROBABILITY_CALC_DESCRIPTOR", 1, CORE_10_INDEX, &DS_TM_PI2_PROBABILITY_CALC_DESCRIPTOR_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_DCL_DATA_BASE", 1, CORE_10_INDEX, &ETH_TM_DCL_DATA_BASE_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_HW_CFG", 1, CORE_10_INDEX, &DHD_HW_CFG_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_10_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_10, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "FPM_GLOBAL_CFG", 1, CORE_10_INDEX, &FPM_GLOBAL_CFG_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_10_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_10, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_10_INDEX, &FW_ERROR_VECTOR_TABLE_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_10_INDEX, &BITS_CALC_MASKS_TABLE_10, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "MULTICAST_KEY_MASK", 1, CORE_10_INDEX, &MULTICAST_KEY_MASK_10, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DS_TM_BBH_TX_EGRESS_COUNTER_TABLE", 1, CORE_10_INDEX, &DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_10, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "DS_TM_PD_FIFO_TABLE", 1, CORE_11_INDEX, &DS_TM_PD_FIFO_TABLE_11, 160, 1, 1 },
#endif
#if defined BCM68375
	{ "POLICER_PARAMS_TABLE", 1, CORE_11_INDEX, &POLICER_PARAMS_TABLE_11, 256, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_SCHEDULING_QUEUE_TABLE", 1, CORE_11_INDEX, &ETH_TM_SCHEDULING_QUEUE_TABLE_11, 80, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_RATE_LIMITER_PROFILE_RESIDUE_TABLE", 1, CORE_11_INDEX, &ETH_TM_RATE_LIMITER_PROFILE_RESIDUE_TABLE_11, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "DS_TM_TM_FLOW_CNTR_TABLE", 1, CORE_11_INDEX, &DS_TM_TM_FLOW_CNTR_TABLE_11, 64, 1, 1 },
#endif
#if defined BCM68375
	{ "TX_TASK_0_STACK", 1, CORE_11_INDEX, &TX_TASK_0_STACK_11, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING_4_TASKS_PACKET_BUFFER", 1, CORE_11_INDEX, &PROCESSING_4_TASKS_PACKET_BUFFER_11, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_RATE_LIMITER_BUDGET_DESCRIPTOR_TABLE", 1, CORE_11_INDEX, &ETH_TM_RATE_LIMITER_BUDGET_DESCRIPTOR_TABLE_11, 80, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_RATE_LIMITER_PROFILE_TABLE", 1, CORE_11_INDEX, &ETH_TM_RATE_LIMITER_PROFILE_TABLE_11, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_FLOW_TABLE", 1, CORE_11_INDEX, &RX_FLOW_TABLE_11, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_RATE_LIMITER_PARAMS_DESCRIPTOR_TABLE", 1, CORE_11_INDEX, &ETH_TM_RATE_LIMITER_PARAMS_DESCRIPTOR_TABLE_11, 80, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_DCL_IMG", 1, CORE_11_INDEX, &ETH_TM_DCL_IMG_11, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "TX_FLOW_TABLE", 1, CORE_11_INDEX, &TX_FLOW_TABLE_11, 212, 1, 1 },
#endif
#if defined BCM68375
	{ "SCORE_COLLECT_DROP_BITMAP_TABLE", 1, CORE_11_INDEX, &SCORE_COLLECT_DROP_BITMAP_TABLE_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DS_TM_BBH_TX_WAKE_UP_DATA_TABLE", 1, CORE_11_INDEX, &DS_TM_BBH_TX_WAKE_UP_DATA_TABLE_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_11_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_11, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_11_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_11, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING0_STACK", 1, CORE_11_INDEX, &PROCESSING0_STACK_11, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "SCORE_CFG_TABLE", 1, CORE_11_INDEX, &SCORE_CFG_TABLE_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_RATE_LIMITER_BUDGET_VALID", 1, CORE_11_INDEX, &ETH_TM_RATE_LIMITER_BUDGET_VALID_11, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "SCORE_DATA_COLLECTION_PROFILE_TABLE", 1, CORE_11_INDEX, &SCORE_DATA_COLLECTION_PROFILE_TABLE_11, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING1_STACK", 1, CORE_11_INDEX, &PROCESSING1_STACK_11, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "SYSTEM_CONFIGURATION", 1, CORE_11_INDEX, &SYSTEM_CONFIGURATION_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SPDTEST_GEN_PARAM", 1, CORE_11_INDEX, &SPDTEST_GEN_PARAM_11, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "GENERAL_TIMER", 1, CORE_11_INDEX, &GENERAL_TIMER_11, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING2_STACK", 1, CORE_11_INDEX, &PROCESSING2_STACK_11, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "MIRRORING_SCRATCH", 1, CORE_11_INDEX, &MIRRORING_SCRATCH_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_11_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_11, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_CFG_TABLE", 1, CORE_11_INDEX, &VPORT_CFG_TABLE_11, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "MUTEX_TABLE", 1, CORE_11_INDEX, &MUTEX_TABLE_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING3_STACK", 1, CORE_11_INDEX, &PROCESSING3_STACK_11, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "TUNNELS_PARSING_CFG", 1, CORE_11_INDEX, &TUNNELS_PARSING_CFG_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_11_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_11_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_SECONDARY_SCHEDULER_TABLE", 1, CORE_11_INDEX, &ETH_TM_SECONDARY_SCHEDULER_TABLE_11, 7, 1, 1 },
#endif
#if defined BCM68375
	{ "CSO_BAD_IPV4_HDR_CSUM_PACKETS", 1, CORE_11_INDEX, &CSO_BAD_IPV4_HDR_CSUM_PACKETS_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "IPTV_CFG_TABLE", 1, CORE_11_INDEX, &IPTV_CFG_TABLE_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_11_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_11, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_AQM_QUEUE_TABLE", 1, CORE_11_INDEX, &ETH_TM_AQM_QUEUE_TABLE_11, 80, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_SCHEDULER_POOL", 1, CORE_11_INDEX, &ETH_TM_SCHEDULER_POOL_11, 160, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_11_INDEX, &BUFMNG_DESCRIPTOR_TABLE_11, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_SCHEDULER_TABLE", 1, CORE_11_INDEX, &ETH_TM_SCHEDULER_TABLE_11, 7, 1, 1 },
#endif
#if defined BCM68375
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_11_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_FPM_THRESHOLDS", 1, CORE_11_INDEX, &DHD_FPM_THRESHOLDS_11, 3, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_DCL_MULTIPLE_CNTR", 1, CORE_11_INDEX, &ETH_TM_DCL_MULTIPLE_CNTR_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "UPDATE_FIFO_STACK", 1, CORE_11_INDEX, &UPDATE_FIFO_STACK_11, 64, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_11_INDEX, &INGRESS_FILTER_PROFILE_TABLE_11, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_AQM_QUEUE_TIMER_TABLE", 1, CORE_11_INDEX, &ETH_TM_AQM_QUEUE_TIMER_TABLE_11, 80, 1, 1 },
#endif
#if defined BCM68375
	{ "TX_MIRRORING_DISPATCHER_CREDIT_TABLE", 1, CORE_11_INDEX, &TX_MIRRORING_DISPATCHER_CREDIT_TABLE_11, 3, 1, 1 },
#endif
#if defined BCM68375
	{ "TASK_IDX", 1, CORE_11_INDEX, &TASK_IDX_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFMNG_STATUS_TABLE", 1, CORE_11_INDEX, &BUFMNG_STATUS_TABLE_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "GENERAL_TIMER_STACK", 1, CORE_11_INDEX, &GENERAL_TIMER_STACK_11, 72, 1, 1 },
#endif
#if defined BCM68375
	{ "REGISTERS_BUFFER", 1, CORE_11_INDEX, &REGISTERS_BUFFER_11, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "DS_TM_CPU_TX_ABS_COUNTERS", 1, CORE_11_INDEX, &DS_TM_CPU_TX_ABS_COUNTERS_11, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "NAT_CACHE_CFG", 1, CORE_11_INDEX, &NAT_CACHE_CFG_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_11_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DEBUG_PRINT_TABLE", 1, CORE_11_INDEX, &DEBUG_PRINT_TABLE_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_FW_FLUSH_TABLE", 1, CORE_11_INDEX, &ETH_TM_FW_FLUSH_TABLE_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "BUDGET_ALLOCATION_TIMER_VALUE", 1, CORE_11_INDEX, &BUDGET_ALLOCATION_TIMER_VALUE_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DOS_DROP_REASONS_CFG", 1, CORE_11_INDEX, &DOS_DROP_REASONS_CFG_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFFER_CONG_SCRATCHPAD", 1, CORE_11_INDEX, &BUFFER_CONG_SCRATCHPAD_11, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_MIRRORING_TABLE", 1, CORE_11_INDEX, &RX_MIRRORING_TABLE_11, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "GENERAL_TIMER_ACTION_VEC", 1, CORE_11_INDEX, &GENERAL_TIMER_ACTION_VEC_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "HOST_TX_TRUNCATE_MIRRORING_TABLE", 1, CORE_11_INDEX, &HOST_TX_TRUNCATE_MIRRORING_TABLE_11, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_11_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_TX_TRUNCATE_MIRRORING_TABLE", 1, CORE_11_INDEX, &ETH_TM_TX_TRUNCATE_MIRRORING_TABLE_11, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "DS_TM_BB_DESTINATION_TABLE", 1, CORE_11_INDEX, &DS_TM_BB_DESTINATION_TABLE_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_11_INDEX, &LOOPBACK_QUEUE_TABLE_11, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_11_INDEX, &LOOPBACK_WAN_FLOW_TABLE_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "FORCE_DSCP", 1, CORE_11_INDEX, &FORCE_DSCP_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_CFG_EX_TABLE", 1, CORE_11_INDEX, &VPORT_CFG_EX_TABLE_11, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "DS_TM_FIRST_QUEUE_MAPPING", 1, CORE_11_INDEX, &DS_TM_FIRST_QUEUE_MAPPING_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFFER_CONG_DQM_NOT_EMPTY", 1, CORE_11_INDEX, &BUFFER_CONG_DQM_NOT_EMPTY_11, 13, 1, 1 },
#endif
#if defined BCM68375
	{ "CORE_ID_TABLE", 1, CORE_11_INDEX, &CORE_ID_TABLE_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "LLQ_SELECTOR_ECN_MASK", 1, CORE_11_INDEX, &LLQ_SELECTOR_ECN_MASK_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TX_EXCEPTION", 1, CORE_11_INDEX, &TX_EXCEPTION_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_11_INDEX, &DEBUG_PRINT_CORE_LOCK_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_11_INDEX, &NAT_CACHE_KEY0_MASK_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_11_INDEX, &RX_MIRRORING_CONFIGURATION_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_11_INDEX, &TX_MIRRORING_CONFIGURATION_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TR471_SPDSVC_RX_PKT_ID_TABLE", 1, CORE_11_INDEX, &TR471_SPDSVC_RX_PKT_ID_TABLE_11, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "TCAM_GENERIC_FIELDS", 1, CORE_11_INDEX, &TCAM_GENERIC_FIELDS_11, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "DS_BUFFER_CONG_MGT_CFG", 1, CORE_11_INDEX, &DS_BUFFER_CONG_MGT_CFG_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "GDX_PARAMS_TABLE", 1, CORE_11_INDEX, &GDX_PARAMS_TABLE_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SRAM_DUMMY_STORE", 1, CORE_11_INDEX, &SRAM_DUMMY_STORE_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "BACKUP_BBH_INGRESS_COUNTERS_TABLE", 1, CORE_11_INDEX, &BACKUP_BBH_INGRESS_COUNTERS_TABLE_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "BACKUP_BBH_EGRESS_COUNTERS_TABLE", 1, CORE_11_INDEX, &BACKUP_BBH_EGRESS_COUNTERS_TABLE_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "CSO_DISABLE", 1, CORE_11_INDEX, &CSO_DISABLE_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_ENABLED_QUEUES_TABLE", 1, CORE_11_INDEX, &ETH_TM_ENABLED_QUEUES_TABLE_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_11_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_11_INDEX, &INGRESS_FILTER_1588_CFG_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_11_INDEX, &RX_MIRRORING_DIRECT_ENABLE_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_11_INDEX, &TCAM_TABLE_CFG_TABLE_11, 2, 1, 1 },
#endif
#if defined BCM68375
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_11_INDEX, &NATC_L2_VLAN_KEY_MASK_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_CFG", 1, CORE_11_INDEX, &INGRESS_FILTER_CFG_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DS_TM_BBH_QUEUE_TABLE", 1, CORE_11_INDEX, &DS_TM_BBH_QUEUE_TABLE_11, 7, 1, 1 },
#endif
#if defined BCM68375
	{ "NATC_L2_TOS_MASK", 1, CORE_11_INDEX, &NATC_L2_TOS_MASK_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DS_TM_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_TABLE", 1, CORE_11_INDEX, &DS_TM_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_TABLE_11, 2, 1, 1 },
#endif
#if defined BCM68375
	{ "DEBUG_SCRATCHPAD", 1, CORE_11_INDEX, &DEBUG_SCRATCHPAD_11, 12, 1, 1 },
#endif
#if defined BCM68375
	{ "UPDATE_FIFO_TABLE", 1, CORE_11_INDEX, &UPDATE_FIFO_TABLE_11, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "DS_TM_PI2_PROBABILITY_CALC_DESCRIPTOR", 1, CORE_11_INDEX, &DS_TM_PI2_PROBABILITY_CALC_DESCRIPTOR_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_DCL_DATA_BASE", 1, CORE_11_INDEX, &ETH_TM_DCL_DATA_BASE_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_HW_CFG", 1, CORE_11_INDEX, &DHD_HW_CFG_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_11_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_11, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "FPM_GLOBAL_CFG", 1, CORE_11_INDEX, &FPM_GLOBAL_CFG_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_11_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_11, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_11_INDEX, &FW_ERROR_VECTOR_TABLE_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_11_INDEX, &BITS_CALC_MASKS_TABLE_11, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "MULTICAST_KEY_MASK", 1, CORE_11_INDEX, &MULTICAST_KEY_MASK_11, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DS_TM_BBH_TX_EGRESS_COUNTER_TABLE", 1, CORE_11_INDEX, &DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_11, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "DS_TM_PD_FIFO_TABLE", 1, CORE_12_INDEX, &DS_TM_PD_FIFO_TABLE_12, 160, 1, 1 },
#endif
#if defined BCM68375
	{ "POLICER_PARAMS_TABLE", 1, CORE_12_INDEX, &POLICER_PARAMS_TABLE_12, 256, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_SCHEDULING_QUEUE_TABLE", 1, CORE_12_INDEX, &ETH_TM_SCHEDULING_QUEUE_TABLE_12, 80, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_RATE_LIMITER_PROFILE_RESIDUE_TABLE", 1, CORE_12_INDEX, &ETH_TM_RATE_LIMITER_PROFILE_RESIDUE_TABLE_12, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "DS_TM_TM_FLOW_CNTR_TABLE", 1, CORE_12_INDEX, &DS_TM_TM_FLOW_CNTR_TABLE_12, 64, 1, 1 },
#endif
#if defined BCM68375
	{ "TX_TASK_0_STACK", 1, CORE_12_INDEX, &TX_TASK_0_STACK_12, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING_4_TASKS_PACKET_BUFFER", 1, CORE_12_INDEX, &PROCESSING_4_TASKS_PACKET_BUFFER_12, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "REPORTING_QUEUE_DESCRIPTOR_TABLE", 1, CORE_12_INDEX, &REPORTING_QUEUE_DESCRIPTOR_TABLE_12, 132, 1, 1 },
#endif
#if defined BCM68375
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_12_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_12, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "UPDATE_FIFO_STACK", 1, CORE_12_INDEX, &UPDATE_FIFO_STACK_12, 64, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_RATE_LIMITER_PROFILE_TABLE", 1, CORE_12_INDEX, &ETH_TM_RATE_LIMITER_PROFILE_TABLE_12, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_FLOW_TABLE", 1, CORE_12_INDEX, &RX_FLOW_TABLE_12, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING0_STACK", 1, CORE_12_INDEX, &PROCESSING0_STACK_12, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "DS_TM_BBH_TX_WAKE_UP_DATA_TABLE", 1, CORE_12_INDEX, &DS_TM_BBH_TX_WAKE_UP_DATA_TABLE_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_RATE_LIMITER_BUDGET_VALID", 1, CORE_12_INDEX, &ETH_TM_RATE_LIMITER_BUDGET_VALID_12, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_DCL_IMG", 1, CORE_12_INDEX, &ETH_TM_DCL_IMG_12, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_RATE_LIMITER_BUDGET_DESCRIPTOR_TABLE", 1, CORE_12_INDEX, &ETH_TM_RATE_LIMITER_BUDGET_DESCRIPTOR_TABLE_12, 80, 1, 1 },
#endif
#if defined BCM68375
	{ "SCORE_DATA_COLLECTION_PROFILE_TABLE", 1, CORE_12_INDEX, &SCORE_DATA_COLLECTION_PROFILE_TABLE_12, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "REPORTING_QUEUE_ACCUMULATED_TABLE", 1, CORE_12_INDEX, &REPORTING_QUEUE_ACCUMULATED_TABLE_12, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_RATE_LIMITER_PARAMS_DESCRIPTOR_TABLE", 1, CORE_12_INDEX, &ETH_TM_RATE_LIMITER_PARAMS_DESCRIPTOR_TABLE_12, 80, 1, 1 },
#endif
#if defined BCM68375
	{ "GENERAL_TIMER", 1, CORE_12_INDEX, &GENERAL_TIMER_12, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "TX_FLOW_TABLE", 1, CORE_12_INDEX, &TX_FLOW_TABLE_12, 212, 1, 1 },
#endif
#if defined BCM68375
	{ "SCORE_COLLECT_DROP_BITMAP_TABLE", 1, CORE_12_INDEX, &SCORE_COLLECT_DROP_BITMAP_TABLE_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SCORE_CFG_TABLE", 1, CORE_12_INDEX, &SCORE_CFG_TABLE_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_12_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_12, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_12_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_12, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING1_STACK", 1, CORE_12_INDEX, &PROCESSING1_STACK_12, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "SYSTEM_CONFIGURATION", 1, CORE_12_INDEX, &SYSTEM_CONFIGURATION_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SPDTEST_GEN_PARAM", 1, CORE_12_INDEX, &SPDTEST_GEN_PARAM_12, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_CFG_TABLE", 1, CORE_12_INDEX, &VPORT_CFG_TABLE_12, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "MUTEX_TABLE", 1, CORE_12_INDEX, &MUTEX_TABLE_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING2_STACK", 1, CORE_12_INDEX, &PROCESSING2_STACK_12, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "MIRRORING_SCRATCH", 1, CORE_12_INDEX, &MIRRORING_SCRATCH_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_12_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_12, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_SECONDARY_SCHEDULER_TABLE", 1, CORE_12_INDEX, &ETH_TM_SECONDARY_SCHEDULER_TABLE_12, 7, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_12_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "REPORT_BBH_TX_QUEUE_ID_TABLE", 1, CORE_12_INDEX, &REPORT_BBH_TX_QUEUE_ID_TABLE_12, 2, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_12_INDEX, &BUFMNG_DESCRIPTOR_TABLE_12, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "PROCESSING3_STACK", 1, CORE_12_INDEX, &PROCESSING3_STACK_12, 360, 1, 1 },
#endif
#if defined BCM68375
	{ "TUNNELS_PARSING_CFG", 1, CORE_12_INDEX, &TUNNELS_PARSING_CFG_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_12_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "CSO_BAD_IPV4_HDR_CSUM_PACKETS", 1, CORE_12_INDEX, &CSO_BAD_IPV4_HDR_CSUM_PACKETS_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_AQM_QUEUE_TIMER_TABLE", 1, CORE_12_INDEX, &ETH_TM_AQM_QUEUE_TIMER_TABLE_12, 80, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_FPM_THRESHOLDS", 1, CORE_12_INDEX, &DHD_FPM_THRESHOLDS_12, 3, 1, 1 },
#endif
#if defined BCM68375
	{ "GHOST_REPORTING_GLOBAL_CFG", 1, CORE_12_INDEX, &GHOST_REPORTING_GLOBAL_CFG_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFMNG_STATUS_TABLE", 1, CORE_12_INDEX, &BUFMNG_STATUS_TABLE_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_AQM_QUEUE_TABLE", 1, CORE_12_INDEX, &ETH_TM_AQM_QUEUE_TABLE_12, 80, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_SCHEDULER_POOL", 1, CORE_12_INDEX, &ETH_TM_SCHEDULER_POOL_12, 160, 1, 1 },
#endif
#if defined BCM68375
	{ "REPORTING_STACK", 1, CORE_12_INDEX, &REPORTING_STACK_12, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "REPORTING_QUEUE_COUNTER_TABLE", 1, CORE_12_INDEX, &REPORTING_QUEUE_COUNTER_TABLE_12, 132, 1, 1 },
#endif
#if defined BCM68375
	{ "IPTV_CFG_TABLE", 1, CORE_12_INDEX, &IPTV_CFG_TABLE_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TX_MIRRORING_DISPATCHER_CREDIT_TABLE", 1, CORE_12_INDEX, &TX_MIRRORING_DISPATCHER_CREDIT_TABLE_12, 3, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_DCL_MULTIPLE_CNTR", 1, CORE_12_INDEX, &ETH_TM_DCL_MULTIPLE_CNTR_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_FW_FLUSH_TABLE", 1, CORE_12_INDEX, &ETH_TM_FW_FLUSH_TABLE_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TASK_IDX", 1, CORE_12_INDEX, &TASK_IDX_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFFER_CONG_SCRATCHPAD", 1, CORE_12_INDEX, &BUFFER_CONG_SCRATCHPAD_12, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "GENERAL_TIMER_STACK", 1, CORE_12_INDEX, &GENERAL_TIMER_STACK_12, 72, 1, 1 },
#endif
#if defined BCM68375
	{ "DEBUG_SCRATCHPAD", 1, CORE_12_INDEX, &DEBUG_SCRATCHPAD_12, 12, 1, 1 },
#endif
#if defined BCM68375
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_12_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_SCHEDULER_TABLE", 1, CORE_12_INDEX, &ETH_TM_SCHEDULER_TABLE_12, 7, 1, 1 },
#endif
#if defined BCM68375
	{ "BBH_TX_EGRESS_REPORT_COUNTER_TABLE", 1, CORE_12_INDEX, &BBH_TX_EGRESS_REPORT_COUNTER_TABLE_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DEBUG_PRINT_TABLE", 1, CORE_12_INDEX, &DEBUG_PRINT_TABLE_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_MIRRORING_TABLE", 1, CORE_12_INDEX, &RX_MIRRORING_TABLE_12, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "BUDGET_ALLOCATION_TIMER_VALUE", 1, CORE_12_INDEX, &BUDGET_ALLOCATION_TIMER_VALUE_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "REPORTING_COUNTER_TABLE", 1, CORE_12_INDEX, &REPORTING_COUNTER_TABLE_12, 40, 1, 1 },
#endif
#if defined BCM68375
	{ "DS_TM_PI2_PROBABILITY_CALC_DESCRIPTOR", 1, CORE_12_INDEX, &DS_TM_PI2_PROBABILITY_CALC_DESCRIPTOR_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DOS_DROP_REASONS_CFG", 1, CORE_12_INDEX, &DOS_DROP_REASONS_CFG_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "GENERAL_TIMER_ACTION_VEC", 1, CORE_12_INDEX, &GENERAL_TIMER_ACTION_VEC_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "BBH_TX_INGRESS_COUNTER_TABLE", 1, CORE_12_INDEX, &BBH_TX_INGRESS_COUNTER_TABLE_12, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "HOST_TX_TRUNCATE_MIRRORING_TABLE", 1, CORE_12_INDEX, &HOST_TX_TRUNCATE_MIRRORING_TABLE_12, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_12_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_12_INDEX, &INGRESS_FILTER_PROFILE_TABLE_12, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "REGISTERS_BUFFER", 1, CORE_12_INDEX, &REGISTERS_BUFFER_12, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "DS_TM_CPU_TX_ABS_COUNTERS", 1, CORE_12_INDEX, &DS_TM_CPU_TX_ABS_COUNTERS_12, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_TX_TRUNCATE_MIRRORING_TABLE", 1, CORE_12_INDEX, &ETH_TM_TX_TRUNCATE_MIRRORING_TABLE_12, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "DS_TM_BB_DESTINATION_TABLE", 1, CORE_12_INDEX, &DS_TM_BB_DESTINATION_TABLE_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_12_INDEX, &LOOPBACK_QUEUE_TABLE_12, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_12_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_12_INDEX, &LOOPBACK_WAN_FLOW_TABLE_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_CFG_EX_TABLE", 1, CORE_12_INDEX, &VPORT_CFG_EX_TABLE_12, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "DS_TM_FIRST_QUEUE_MAPPING", 1, CORE_12_INDEX, &DS_TM_FIRST_QUEUE_MAPPING_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFFER_CONG_DQM_NOT_EMPTY", 1, CORE_12_INDEX, &BUFFER_CONG_DQM_NOT_EMPTY_12, 13, 1, 1 },
#endif
#if defined BCM68375
	{ "FORCE_DSCP", 1, CORE_12_INDEX, &FORCE_DSCP_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "CORE_ID_TABLE", 1, CORE_12_INDEX, &CORE_ID_TABLE_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "LLQ_SELECTOR_ECN_MASK", 1, CORE_12_INDEX, &LLQ_SELECTOR_ECN_MASK_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TX_EXCEPTION", 1, CORE_12_INDEX, &TX_EXCEPTION_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "NAT_CACHE_CFG", 1, CORE_12_INDEX, &NAT_CACHE_CFG_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_12_INDEX, &DEBUG_PRINT_CORE_LOCK_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TR471_SPDSVC_RX_PKT_ID_TABLE", 1, CORE_12_INDEX, &TR471_SPDSVC_RX_PKT_ID_TABLE_12, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "TCAM_GENERIC_FIELDS", 1, CORE_12_INDEX, &TCAM_GENERIC_FIELDS_12, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "DS_BUFFER_CONG_MGT_CFG", 1, CORE_12_INDEX, &DS_BUFFER_CONG_MGT_CFG_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "GDX_PARAMS_TABLE", 1, CORE_12_INDEX, &GDX_PARAMS_TABLE_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_12_INDEX, &RX_MIRRORING_CONFIGURATION_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_12_INDEX, &TX_MIRRORING_CONFIGURATION_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DS_TM_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_TABLE", 1, CORE_12_INDEX, &DS_TM_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_TABLE_12, 2, 1, 1 },
#endif
#if defined BCM68375
	{ "XGPON_REPORT_ZERO_SENT_TABLE", 1, CORE_12_INDEX, &XGPON_REPORT_ZERO_SENT_TABLE_12, 10, 1, 1 },
#endif
#if defined BCM68375
	{ "SRAM_DUMMY_STORE", 1, CORE_12_INDEX, &SRAM_DUMMY_STORE_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "BACKUP_BBH_INGRESS_COUNTERS_TABLE", 1, CORE_12_INDEX, &BACKUP_BBH_INGRESS_COUNTERS_TABLE_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "BACKUP_BBH_EGRESS_COUNTERS_TABLE", 1, CORE_12_INDEX, &BACKUP_BBH_EGRESS_COUNTERS_TABLE_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "CSO_DISABLE", 1, CORE_12_INDEX, &CSO_DISABLE_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_12_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_12_INDEX, &INGRESS_FILTER_1588_CFG_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DHD_HW_CFG", 1, CORE_12_INDEX, &DHD_HW_CFG_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_12_INDEX, &RX_MIRRORING_DIRECT_ENABLE_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_12_INDEX, &NAT_CACHE_KEY0_MASK_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_ENABLED_QUEUES_TABLE", 1, CORE_12_INDEX, &ETH_TM_ENABLED_QUEUES_TABLE_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_12_INDEX, &TCAM_TABLE_CFG_TABLE_12, 2, 1, 1 },
#endif
#if defined BCM68375
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_12_INDEX, &NATC_L2_VLAN_KEY_MASK_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "INGRESS_FILTER_CFG", 1, CORE_12_INDEX, &INGRESS_FILTER_CFG_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "UPDATE_FIFO_TABLE", 1, CORE_12_INDEX, &UPDATE_FIFO_TABLE_12, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "GHOST_REPORTING_QUEUE_STATUS_BIT_VECTOR_TABLE", 1, CORE_12_INDEX, &GHOST_REPORTING_QUEUE_STATUS_BIT_VECTOR_TABLE_12, 5, 1, 1 },
#endif
#if defined BCM68375
	{ "NATC_L2_TOS_MASK", 1, CORE_12_INDEX, &NATC_L2_TOS_MASK_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DS_TM_BBH_QUEUE_TABLE", 1, CORE_12_INDEX, &DS_TM_BBH_QUEUE_TABLE_12, 7, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_12_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_12, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "QUEUE_TO_REPORT_BIT_VECTOR", 1, CORE_12_INDEX, &QUEUE_TO_REPORT_BIT_VECTOR_12, 5, 1, 1 },
#endif
#if defined BCM68375
	{ "ETH_TM_DCL_DATA_BASE", 1, CORE_12_INDEX, &ETH_TM_DCL_DATA_BASE_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "FPM_GLOBAL_CFG", 1, CORE_12_INDEX, &FPM_GLOBAL_CFG_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_12_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_12, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_12_INDEX, &FW_ERROR_VECTOR_TABLE_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_12_INDEX, &BITS_CALC_MASKS_TABLE_12, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "MULTICAST_KEY_MASK", 1, CORE_12_INDEX, &MULTICAST_KEY_MASK_12, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DS_TM_BBH_TX_EGRESS_COUNTER_TABLE", 1, CORE_12_INDEX, &DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_12, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "US_TM_PD_FIFO_TABLE", 1, CORE_13_INDEX, &US_TM_PD_FIFO_TABLE_13, 328, 1, 1 },
#endif
#if defined BCM68375
	{ "PON_TM_SCHEDULER_POOL", 1, CORE_13_INDEX, &PON_TM_SCHEDULER_POOL_13, 312, 1, 1 },
#endif
#if defined BCM68375
	{ "SYSTEM_CONFIGURATION", 1, CORE_13_INDEX, &SYSTEM_CONFIGURATION_13, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "UPDATE_FIFO_STACK", 1, CORE_13_INDEX, &UPDATE_FIFO_STACK_13, 64, 1, 1 },
#endif
#if defined BCM68375
	{ "PON_TM_SECONDARY_SCHEDULER_TABLE", 1, CORE_13_INDEX, &PON_TM_SECONDARY_SCHEDULER_TABLE_13, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "WAN_STACK", 1, CORE_13_INDEX, &WAN_STACK_13, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "PON_TM_SCHEDULING_QUEUE_TABLE", 1, CORE_13_INDEX, &PON_TM_SCHEDULING_QUEUE_TABLE_13, 132, 1, 1 },
#endif
#if defined BCM68375
	{ "US_TM_WAN_0_BBH_TX_WAKE_UP_DATA_TABLE", 1, CORE_13_INDEX, &US_TM_WAN_0_BBH_TX_WAKE_UP_DATA_TABLE_13, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "PON_TM_RATE_LIMITER_BUDGET_VALID", 1, CORE_13_INDEX, &PON_TM_RATE_LIMITER_BUDGET_VALID_13, 4, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFFER_CONG_SCRATCHPAD", 1, CORE_13_INDEX, &BUFFER_CONG_SCRATCHPAD_13, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "WAN_EPON_STACK", 1, CORE_13_INDEX, &WAN_EPON_STACK_13, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "US_TM_BBH_QUEUE_TABLE", 1, CORE_13_INDEX, &US_TM_BBH_QUEUE_TABLE_13, 40, 1, 1 },
#endif
#if defined BCM68375
	{ "HOST_TX_TRUNCATE_MIRRORING_TABLE", 1, CORE_13_INDEX, &HOST_TX_TRUNCATE_MIRRORING_TABLE_13, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "PON_TM_TX_TRUNCATE_MIRRORING_TABLE", 1, CORE_13_INDEX, &PON_TM_TX_TRUNCATE_MIRRORING_TABLE_13, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "PON_TM_RATE_LIMITER_PROFILE_TABLE", 1, CORE_13_INDEX, &PON_TM_RATE_LIMITER_PROFILE_TABLE_13, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "PON_TM_RATE_LIMITER_BUDGET_DESCRIPTOR_TABLE", 1, CORE_13_INDEX, &PON_TM_RATE_LIMITER_BUDGET_DESCRIPTOR_TABLE_13, 132, 1, 1 },
#endif
#if defined BCM68375
	{ "PON_TM_RATE_LIMITER_PROFILE_RESIDUE_TABLE", 1, CORE_13_INDEX, &PON_TM_RATE_LIMITER_PROFILE_RESIDUE_TABLE_13, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_CFG_EX_TABLE", 1, CORE_13_INDEX, &VPORT_CFG_EX_TABLE_13, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "BUDGET_ALLOCATION_TIMER_VALUE", 1, CORE_13_INDEX, &BUDGET_ALLOCATION_TIMER_VALUE_13, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "PON_TM_DCL_IMG", 1, CORE_13_INDEX, &PON_TM_DCL_IMG_13, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "DIRECT_FLOW_RX_MIRRORING_SCRATCHPAD", 1, CORE_13_INDEX, &DIRECT_FLOW_RX_MIRRORING_SCRATCHPAD_13, 136, 1, 1 },
#endif
#if defined BCM68375
	{ "BBH_TX_EPON_WAKE_UP_DATA_TABLE", 1, CORE_13_INDEX, &BBH_TX_EPON_WAKE_UP_DATA_TABLE_13, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "NULL_BUFFER", 1, CORE_13_INDEX, &NULL_BUFFER_13, 2, 1, 1 },
#endif
#if defined BCM68375
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_13_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_13, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFFER_CONG_DQM_NOT_EMPTY", 1, CORE_13_INDEX, &BUFFER_CONG_DQM_NOT_EMPTY_13, 13, 1, 1 },
#endif
#if defined BCM68375
	{ "MUTEX_TABLE", 1, CORE_13_INDEX, &MUTEX_TABLE_13, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "MIRRORING_SCRATCH", 1, CORE_13_INDEX, &MIRRORING_SCRATCH_13, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "FPM_RING_BUFMGT_CNT_TABLE", 1, CORE_13_INDEX, &FPM_RING_BUFMGT_CNT_TABLE_13, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_13_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_13, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TASK_IDX", 1, CORE_13_INDEX, &TASK_IDX_13, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DIRECT_FLOW_PD_TABLE", 1, CORE_13_INDEX, &DIRECT_FLOW_PD_TABLE_13, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_13_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_13, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "US_BUFFER_CONG_MGT_CFG", 1, CORE_13_INDEX, &US_BUFFER_CONG_MGT_CFG_13, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "PON_TM_DISPATCHER_CREDIT_TABLE", 1, CORE_13_INDEX, &PON_TM_DISPATCHER_CREDIT_TABLE_13, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "GENERAL_TIMER_ACTION_VEC", 1, CORE_13_INDEX, &GENERAL_TIMER_ACTION_VEC_13, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DIRECT_FLOW_PAUSE_QUANTA", 1, CORE_13_INDEX, &DIRECT_FLOW_PAUSE_QUANTA_13, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "US_TM_TM_FLOW_CNTR_TABLE", 1, CORE_13_INDEX, &US_TM_TM_FLOW_CNTR_TABLE_13, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "PON_TM_RATE_LIMITER_PARAMS_DESCRIPTOR_TABLE", 1, CORE_13_INDEX, &PON_TM_RATE_LIMITER_PARAMS_DESCRIPTOR_TABLE_13, 132, 1, 1 },
#endif
#if defined BCM68375
	{ "EPON_UPDATE_FIFO_STACK", 1, CORE_13_INDEX, &EPON_UPDATE_FIFO_STACK_13, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "US_TM_WAN_0_BBH_TX_EGRESS_COUNTER_TABLE", 1, CORE_13_INDEX, &US_TM_WAN_0_BBH_TX_EGRESS_COUNTER_TABLE_13, 40, 1, 1 },
#endif
#if defined BCM68375
	{ "US_TM_BBH_TX_WAN_0_FIFO_BYTES_USED", 1, CORE_13_INDEX, &US_TM_BBH_TX_WAN_0_FIFO_BYTES_USED_13, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "US_TM_WAN_0_BB_DESTINATION_TABLE", 1, CORE_13_INDEX, &US_TM_WAN_0_BB_DESTINATION_TABLE_13, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "QEMU_SYNC_MEM", 1, CORE_13_INDEX, &QEMU_SYNC_MEM_13, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DIRECT_FLOW_WAN_VIQ_EXCLUSIVE", 1, CORE_13_INDEX, &DIRECT_FLOW_WAN_VIQ_EXCLUSIVE_13, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DISPATCHER_CREDIT_TABLE", 1, CORE_13_INDEX, &DISPATCHER_CREDIT_TABLE_13, 3, 1, 1 },
#endif
#if defined BCM68375
	{ "PON_TM_VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_13_INDEX, &PON_TM_VPORT_TO_RL_OVERHEAD_TABLE_13, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "CORE_ID_TABLE", 1, CORE_13_INDEX, &CORE_ID_TABLE_13, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "US_TM_FIRST_QUEUE_MAPPING", 1, CORE_13_INDEX, &US_TM_FIRST_QUEUE_MAPPING_13, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "GENERAL_TIMER", 1, CORE_13_INDEX, &GENERAL_TIMER_13, 16, 1, 1 },
#endif
#if defined BCM68375
	{ "PON_TM_AQM_QUEUE_TIMER_TABLE", 1, CORE_13_INDEX, &PON_TM_AQM_QUEUE_TIMER_TABLE_13, 132, 1, 1 },
#endif
#if defined BCM68375
	{ "US_TM_TX_PAUSE_NACK", 1, CORE_13_INDEX, &US_TM_TX_PAUSE_NACK_13, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TX_EXCEPTION", 1, CORE_13_INDEX, &TX_EXCEPTION_13, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "BBH_TX_US_WAN_0_FIFO_BYTES_THRESHOLD", 1, CORE_13_INDEX, &BBH_TX_US_WAN_0_FIFO_BYTES_THRESHOLD_13, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_13_INDEX, &RX_MIRRORING_CONFIGURATION_13, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_13_INDEX, &TX_MIRRORING_CONFIGURATION_13, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_13_INDEX, &DEBUG_PRINT_CORE_LOCK_13, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "SRAM_DUMMY_STORE", 1, CORE_13_INDEX, &SRAM_DUMMY_STORE_13, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "MAC_TYPE", 1, CORE_13_INDEX, &MAC_TYPE_13, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "WAN_0_BBH_TX_FIFO_SIZE", 1, CORE_13_INDEX, &WAN_0_BBH_TX_FIFO_SIZE_13, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_13_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_13, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_13_INDEX, &RX_MIRRORING_DIRECT_ENABLE_13, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DIRECT_FLOW_CNTR_TABLE", 1, CORE_13_INDEX, &DIRECT_FLOW_CNTR_TABLE_13, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "BBH_TX_EPON_INGRESS_COUNTER_TABLE", 1, CORE_13_INDEX, &BBH_TX_EPON_INGRESS_COUNTER_TABLE_13, 40, 1, 1 },
#endif
#if defined BCM68375
	{ "DIRECT_FLOW_PAUSE_DEBUG", 1, CORE_13_INDEX, &DIRECT_FLOW_PAUSE_DEBUG_13, 3, 1, 1 },
#endif
#if defined BCM68375
	{ "CPU_FPM_BUFMNG_RELEASE_STACK", 1, CORE_13_INDEX, &CPU_FPM_BUFMNG_RELEASE_STACK_13, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "DIRECT_FLOW_STACK", 1, CORE_13_INDEX, &DIRECT_FLOW_STACK_13, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "VPORT_CFG_TABLE", 1, CORE_13_INDEX, &VPORT_CFG_TABLE_13, 31, 1, 1 },
#endif
#if defined BCM68375
	{ "GENERAL_TIMER_STACK", 1, CORE_13_INDEX, &GENERAL_TIMER_STACK_13, 72, 1, 1 },
#endif
#if defined BCM68375
	{ "TX_MIRRORING_DISPATCHER_CREDIT_TABLE", 1, CORE_13_INDEX, &TX_MIRRORING_DISPATCHER_CREDIT_TABLE_13, 3, 1, 1 },
#endif
#if defined BCM68375
	{ "PON_TM_FW_FLUSH_TABLE", 1, CORE_13_INDEX, &PON_TM_FW_FLUSH_TABLE_13, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "PON_TM_SCHEDULER_TABLE", 1, CORE_13_INDEX, &PON_TM_SCHEDULER_TABLE_13, 40, 1, 1 },
#endif
#if defined BCM68375
	{ "PON_TM_ENABLED_QUEUES_TABLE", 1, CORE_13_INDEX, &PON_TM_ENABLED_QUEUES_TABLE_13, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "DEBUG_PRINT_TABLE", 1, CORE_13_INDEX, &DEBUG_PRINT_TABLE_13, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "PON_TM_AQM_QUEUE_TABLE", 1, CORE_13_INDEX, &PON_TM_AQM_QUEUE_TABLE_13, 132, 1, 1 },
#endif
#if defined BCM68375
	{ "REGISTERS_BUFFER", 1, CORE_13_INDEX, &REGISTERS_BUFFER_13, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "US_TM_CPU_TX_ABS_COUNTERS", 1, CORE_13_INDEX, &US_TM_CPU_TX_ABS_COUNTERS_13, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "DEBUG_SCRATCHPAD", 1, CORE_13_INDEX, &DEBUG_SCRATCHPAD_13, 12, 1, 1 },
#endif
#if defined BCM68375
	{ "BBH_TX_EPON_EGRESS_COUNTER_TABLE", 1, CORE_13_INDEX, &BBH_TX_EPON_EGRESS_COUNTER_TABLE_13, 32, 1, 1 },
#endif
#if defined BCM68375
	{ "DIRECT_FLOW_EPON_CONTROL_SCRATCH", 1, CORE_13_INDEX, &DIRECT_FLOW_EPON_CONTROL_SCRATCH_13, 22, 1, 1 },
#endif
#if defined BCM68375
	{ "PON_TM_DCL_DATA_BASE", 1, CORE_13_INDEX, &PON_TM_DCL_DATA_BASE_13, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "US_TM_PI2_PROBABILITY_CALC_DESCRIPTOR", 1, CORE_13_INDEX, &US_TM_PI2_PROBABILITY_CALC_DESCRIPTOR_13, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "UPDATE_FIFO_TABLE", 1, CORE_13_INDEX, &UPDATE_FIFO_TABLE_13, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "FPM_GLOBAL_CFG", 1, CORE_13_INDEX, &FPM_GLOBAL_CFG_13, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_13_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_13, 128, 1, 1 },
#endif
#if defined BCM68375
	{ "EPON_UPDATE_FIFO_TABLE", 1, CORE_13_INDEX, &EPON_UPDATE_FIFO_TABLE_13, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "US_TM_TX_OCTETS_COUNTERS_TABLE", 1, CORE_13_INDEX, &US_TM_TX_OCTETS_COUNTERS_TABLE_13, 8, 1, 1 },
#endif
#if defined BCM68375
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_13_INDEX, &FW_ERROR_VECTOR_TABLE_13, 1, 1, 1 },
#endif
#if defined BCM68375
	{ "US_TM_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_TABLE", 1, CORE_13_INDEX, &US_TM_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_TABLE_13, 1, 1, 1 },
#endif
};

TABLE_STACK_STRUCT RUNNER_STACK_TABLES[NUMBER_OF_STACK_TABLES] =
{
    { "PROCESSING0_STACK", CORE_0_INDEX, 0xa00, 360},
    { "PROCESSING1_STACK", CORE_0_INDEX, 0xc00, 360},
    { "PROCESSING2_STACK", CORE_0_INDEX, 0xe00, 360},
    { "PROCESSING3_STACK", CORE_0_INDEX, 0x2000, 360},
    { "DHD_RX_COMPLETE_0_STACK", CORE_0_INDEX, 0x2180, 128},
    { "PROCESSING4_STACK", CORE_0_INDEX, 0x2200, 360},
    { "DHD_RX_COMPLETE_1_STACK", CORE_0_INDEX, 0x2380, 128},
    { "PROCESSING5_STACK", CORE_0_INDEX, 0x2400, 360},
    { "DHD_RX_COMPLETE_2_STACK", CORE_0_INDEX, 0x2580, 128},
    { "PROCESSING6_STACK", CORE_0_INDEX, 0x2600, 360},
    { "PROCESSING7_STACK", CORE_0_INDEX, 0x2800, 360},
    { "DHD_TX_COMPLETE_0_STACK", CORE_0_INDEX, 0x2dc0, 64},
    { "DHD_TX_COMPLETE_1_STACK", CORE_0_INDEX, 0x2e40, 64},
    { "DHD_TX_COMPLETE_2_STACK", CORE_0_INDEX, 0x2e80, 64},
    { "PROCESSING0_STACK", CORE_1_INDEX, 0x2200, 360},
    { "PROCESSING1_STACK", CORE_1_INDEX, 0x2400, 360},
    { "PROCESSING2_STACK", CORE_1_INDEX, 0x2600, 360},
    { "PROCESSING3_STACK", CORE_1_INDEX, 0x2800, 360},
    { "PROCESSING4_STACK", CORE_1_INDEX, 0x2a00, 360},
    { "PROCESSING5_STACK", CORE_1_INDEX, 0x2c00, 360},
    { "PROCESSING6_STACK", CORE_1_INDEX, 0x2e00, 360},
    { "PROCESSING7_STACK", CORE_1_INDEX, 0x3000, 360},
    { "DHD_TIMER_STACK", CORE_1_INDEX, 0x31e0, 32},
    { "DHD_TX_POST_UPDATE_FIFO_STACK", CORE_1_INDEX, 0x35e0, 32},
    { "DHD_TX_POST_0_STACK", CORE_1_INDEX, 0x3700, 168},
    { "DHD_TX_POST_1_STACK", CORE_1_INDEX, 0x3800, 168},
    { "DHD_TX_POST_2_STACK", CORE_1_INDEX, 0x3900, 168},
    { "PROCESSING0_STACK", CORE_2_INDEX, 0x200, 360},
    { "PROCESSING1_STACK", CORE_2_INDEX, 0xa00, 360},
    { "PROCESSING2_STACK", CORE_2_INDEX, 0xc00, 360},
    { "CPU_RX_CSO_STACK", CORE_2_INDEX, 0xd80, 128},
    { "PROCESSING3_STACK", CORE_2_INDEX, 0xe00, 360},
    { "PROCESSING4_STACK", CORE_2_INDEX, 0x2000, 360},
    { "PROCESSING5_STACK", CORE_2_INDEX, 0x2200, 360},
    { "PROCESSING6_STACK", CORE_2_INDEX, 0x2400, 360},
    { "GENERAL_TIMER_STACK", CORE_2_INDEX, 0x2580, 72},
    { "PROCESSING7_STACK", CORE_2_INDEX, 0x2600, 360},
    { "CPU_RX_STACK", CORE_2_INDEX, 0x2e00, 136},
    { "PROCESSING0_STACK", CORE_3_INDEX, 0x600, 360},
    { "PROCESSING1_STACK", CORE_3_INDEX, 0xa00, 360},
    { "PROCESSING2_STACK", CORE_3_INDEX, 0xc00, 360},
    { "PROCESSING3_STACK", CORE_3_INDEX, 0xe00, 360},
    { "PROCESSING4_STACK", CORE_3_INDEX, 0x2000, 360},
    { "GENERAL_TIMER_STACK", CORE_3_INDEX, 0x2180, 72},
    { "PROCESSING5_STACK", CORE_3_INDEX, 0x2200, 360},
    { "PROCESSING6_STACK", CORE_3_INDEX, 0x2400, 360},
    { "PROCESSING7_STACK", CORE_3_INDEX, 0x2600, 360},
    { "CPU_TX_0_STACK", CORE_3_INDEX, 0x2900, 224},
    { "CPU_TX_1_STACK", CORE_3_INDEX, 0x2b00, 224},
    { "CPU_RECYCLE_STACK", CORE_3_INDEX, 0x2ce0, 32},
    { "PROCESSING0_STACK", CORE_4_INDEX, 0xa00, 360},
    { "PROCESSING1_STACK", CORE_4_INDEX, 0xe00, 360},
    { "SERVICE_QUEUES_STACK", CORE_4_INDEX, 0xf80, 128},
    { "PROCESSING2_STACK", CORE_4_INDEX, 0x2000, 360},
    { "PROCESSING3_STACK", CORE_4_INDEX, 0x2200, 360},
    { "PROCESSING4_STACK", CORE_4_INDEX, 0x2400, 360},
    { "PROCESSING5_STACK", CORE_4_INDEX, 0x2600, 360},
    { "PROCESSING6_STACK", CORE_4_INDEX, 0x2800, 360},
    { "GENERAL_TIMER_STACK", CORE_4_INDEX, 0x2980, 72},
    { "PROCESSING7_STACK", CORE_4_INDEX, 0x2a00, 360},
    { "SPU_REQUEST_STACK", CORE_4_INDEX, 0x2dc0, 64},
    { "COMMON_REPROCESSING_STACK", CORE_5_INDEX, 0xa40, 64},
    { "PROCESSING0_STACK", CORE_5_INDEX, 0xe00, 360},
    { "PROCESSING1_STACK", CORE_5_INDEX, 0x2000, 360},
    { "SPDSVC_GEN_STACK", CORE_5_INDEX, 0x2180, 128},
    { "PROCESSING2_STACK", CORE_5_INDEX, 0x2200, 360},
    { "PROCESSING3_STACK", CORE_5_INDEX, 0x2400, 360},
    { "PROCESSING4_STACK", CORE_5_INDEX, 0x2600, 360},
    { "PROCESSING5_STACK", CORE_5_INDEX, 0x2800, 360},
    { "PROCESSING6_STACK", CORE_5_INDEX, 0x2a00, 360},
    { "PROCESSING7_STACK", CORE_5_INDEX, 0x2c00, 360},
    { "TCPSPDTEST_DOWNLOAD_STACK", CORE_5_INDEX, 0x2e00, 240},
    { "PROCESSING0_STACK", CORE_6_INDEX, 0x200, 360},
    { "PROCESSING1_STACK", CORE_6_INDEX, 0x600, 360},
    { "PROCESSING2_STACK", CORE_6_INDEX, 0x800, 360},
    { "PROCESSING3_STACK", CORE_6_INDEX, 0xa00, 360},
    { "PROCESSING4_STACK", CORE_6_INDEX, 0xc00, 360},
    { "PROCESSING5_STACK", CORE_6_INDEX, 0xe00, 360},
    { "PROCESSING6_STACK", CORE_6_INDEX, 0x2000, 360},
    { "PROCESSING7_STACK", CORE_6_INDEX, 0x2200, 360},
    { "PROCESSING0_STACK", CORE_7_INDEX, 0x200, 360},
    { "PROCESSING1_STACK", CORE_7_INDEX, 0x600, 360},
    { "PROCESSING2_STACK", CORE_7_INDEX, 0x800, 360},
    { "PROCESSING3_STACK", CORE_7_INDEX, 0xa00, 360},
    { "PROCESSING4_STACK", CORE_7_INDEX, 0xc00, 360},
    { "PROCESSING5_STACK", CORE_7_INDEX, 0xe00, 360},
    { "PROCESSING6_STACK", CORE_7_INDEX, 0x2000, 360},
    { "PROCESSING7_STACK", CORE_7_INDEX, 0x2200, 360},
    { "PROCESSING0_STACK", CORE_8_INDEX, 0x200, 360},
    { "PROCESSING1_STACK", CORE_8_INDEX, 0x600, 360},
    { "PROCESSING2_STACK", CORE_8_INDEX, 0x800, 360},
    { "PROCESSING3_STACK", CORE_8_INDEX, 0xa00, 360},
    { "PROCESSING4_STACK", CORE_8_INDEX, 0xc00, 360},
    { "PROCESSING5_STACK", CORE_8_INDEX, 0xe00, 360},
    { "PROCESSING6_STACK", CORE_8_INDEX, 0x2000, 360},
    { "PROCESSING7_STACK", CORE_8_INDEX, 0x2200, 360},
    { "SPDSVC_ANALYZER_STACK", CORE_8_INDEX, 0x2700, 176},
    { "PROCESSING0_STACK", CORE_9_INDEX, 0x200, 360},
    { "PROCESSING1_STACK", CORE_9_INDEX, 0x600, 360},
    { "PROCESSING2_STACK", CORE_9_INDEX, 0x800, 360},
    { "PROCESSING3_STACK", CORE_9_INDEX, 0xa00, 360},
    { "PROCESSING4_STACK", CORE_9_INDEX, 0xc00, 360},
    { "SPU_RESPONSE_STACK", CORE_9_INDEX, 0xd80, 96},
    { "PROCESSING5_STACK", CORE_9_INDEX, 0xe00, 360},
    { "GENERAL_TIMER_STACK", CORE_9_INDEX, 0xf80, 72},
    { "PROCESSING6_STACK", CORE_9_INDEX, 0x2000, 360},
    { "PROCESSING7_STACK", CORE_9_INDEX, 0x2200, 360},
    { "DATA_TRACE_STACK", CORE_9_INDEX, 0x2900, 40},
    { "TX_TASK_0_STACK", CORE_10_INDEX, 0xf80, 128},
    { "PROCESSING0_STACK", CORE_10_INDEX, 0x2200, 360},
    { "PROCESSING1_STACK", CORE_10_INDEX, 0x2400, 360},
    { "PROCESSING2_STACK", CORE_10_INDEX, 0x2600, 360},
    { "PROCESSING3_STACK", CORE_10_INDEX, 0x2800, 360},
    { "UPDATE_FIFO_STACK", CORE_10_INDEX, 0x2cc0, 64},
    { "GENERAL_TIMER_STACK", CORE_10_INDEX, 0x2e00, 72},
    { "TX_TASK_0_STACK", CORE_11_INDEX, 0xf80, 128},
    { "PROCESSING0_STACK", CORE_11_INDEX, 0x2200, 360},
    { "PROCESSING1_STACK", CORE_11_INDEX, 0x2400, 360},
    { "PROCESSING2_STACK", CORE_11_INDEX, 0x2600, 360},
    { "PROCESSING3_STACK", CORE_11_INDEX, 0x2800, 360},
    { "UPDATE_FIFO_STACK", CORE_11_INDEX, 0x2cc0, 64},
    { "GENERAL_TIMER_STACK", CORE_11_INDEX, 0x2e00, 72},
    { "TX_TASK_0_STACK", CORE_12_INDEX, 0xf80, 128},
    { "UPDATE_FIFO_STACK", CORE_12_INDEX, 0x1c40, 64},
    { "PROCESSING0_STACK", CORE_12_INDEX, 0x1e00, 360},
    { "PROCESSING1_STACK", CORE_12_INDEX, 0x2a00, 360},
    { "PROCESSING2_STACK", CORE_12_INDEX, 0x2c00, 360},
    { "PROCESSING3_STACK", CORE_12_INDEX, 0x2e00, 360},
    { "REPORTING_STACK", CORE_12_INDEX, 0x31e0, 32},
    { "GENERAL_TIMER_STACK", CORE_12_INDEX, 0x3380, 72},
    { "UPDATE_FIFO_STACK", CORE_13_INDEX, 0x15c0, 64},
    { "WAN_STACK", CORE_13_INDEX, 0x1780, 128},
    { "WAN_EPON_STACK", CORE_13_INDEX, 0x1d80, 128},
    { "EPON_UPDATE_FIFO_STACK", CORE_13_INDEX, 0x2c20, 32},
    { "CPU_FPM_BUFMNG_RELEASE_STACK", CORE_13_INDEX, 0x2e00, 128},
    { "DIRECT_FLOW_STACK", CORE_13_INDEX, 0x2e80, 128},
    { "GENERAL_TIMER_STACK", CORE_13_INDEX, 0x2f80, 72}
};
