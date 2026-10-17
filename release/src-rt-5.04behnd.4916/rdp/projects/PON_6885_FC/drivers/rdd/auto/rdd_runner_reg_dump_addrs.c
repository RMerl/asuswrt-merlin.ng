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
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, set_RDD_POLICER_PARAMS_ENTRY, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, set_RDD_SPDTEST_GEN_CFG, 0x370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, set_RDD_VPORT_CFG_ENTRY, 0x380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, set_RDD_TUNNELS_PARSING_CFG, 0x7e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x7f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_BAD_IPV4_HDR_CSUM_PACKETS =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x96c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TASK_IDX =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x97c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x9be },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, set_RDD_VPORT_CFG_EX_ENTRY, 0x9c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x9fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xa00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, set_RDD_IPTV_CFG, 0xb68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xb70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FORCE_DSCP =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_TABLE =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, set_RDD_TR471_SPDSVC_RX_PKT_ID, 0xb80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0xbb0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xbc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0xbe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, set_RDD_TCAM_GENERIC, 0xd68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, set_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0xd78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, set_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0xd80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, set_RDD_BUFMNG_STATUS_ENTRY, 0xda0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0xdc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xddf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, set_RDD_DHD_HW_CONFIGURATION, 0xde0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0xdf4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0xdf6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, set_RDD_NAT_CACHE_CFG, 0xdf8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xdff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CHANNEL_ID_TO_SBPM_BB_ID_TABLE =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0xf68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xf78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_DISABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, set_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0xf7d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xfa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, set_RDD_GDX_PARAMS, 0xfd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0xfe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, set_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, set_RDD_TCAM_TABLE_CFG, 0x2168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, set_RDD_INGRESS_FILTER_CFG, 0x2178 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2190 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x21a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, set_RDD_MULTICAST_KEY_MASK_ENTRY, 0x21c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, set_RDD_RX_FLOW_ENTRY, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0x2500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PARSER_KEY_BUILD_TABLE =
{
	36,
	{
		{ dump_RDD_PARSER_KEY_BUILD_ENTRY, set_RDD_PARSER_KEY_BUILD_ENTRY, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, set_RDD_INGRESS_FILTER_CTRL, 0x2700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_1 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, set_RDD_POLICER_PARAMS_ENTRY, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_1 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_1 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, set_RDD_SPDTEST_GEN_CFG, 0x370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_1 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, set_RDD_VPORT_CFG_ENTRY, 0x380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_1 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_1 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, set_RDD_TUNNELS_PARSING_CFG, 0x7e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x7f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_BAD_IPV4_HDR_CSUM_PACKETS_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x96c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_1 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TASK_IDX_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x97c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x9be },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_1 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, set_RDD_VPORT_CFG_EX_ENTRY, 0x9c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x9fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xa00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_1 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, set_RDD_IPTV_CFG, 0xb68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xb70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_TABLE_1 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, set_RDD_TR471_SPDSVC_RX_PKT_ID, 0xb80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_1 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0xbb0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xbc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_1 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0xbe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_1 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, set_RDD_TCAM_GENERIC, 0xd68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_1 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, set_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0xd78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_1 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, set_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0xd80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_1 =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, set_RDD_BUFMNG_STATUS_ENTRY, 0xda0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_1 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0xdc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xddf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_1 =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, set_RDD_DHD_HW_CONFIGURATION, 0xde0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_1 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0xdf4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_1 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0xdf6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_1 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, set_RDD_NAT_CACHE_CFG, 0xdf8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xdff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CHANNEL_ID_TO_SBPM_BB_ID_TABLE_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0xf68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xf78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_DISABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_1 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, set_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0xf7d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xfa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_1 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, set_RDD_GDX_PARAMS, 0xfd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_1 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0xfe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER_1 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, set_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_1 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, set_RDD_TCAM_TABLE_CFG, 0x2168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_1 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, set_RDD_INGRESS_FILTER_CFG, 0x2178 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2190 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x21a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_1 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, set_RDD_MULTICAST_KEY_MASK_ENTRY, 0x21c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_1 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, set_RDD_RX_FLOW_ENTRY, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_1 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0x2500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PARSER_KEY_BUILD_TABLE_1 =
{
	36,
	{
		{ dump_RDD_PARSER_KEY_BUILD_ENTRY, set_RDD_PARSER_KEY_BUILD_ENTRY, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_1 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, set_RDD_INGRESS_FILTER_CTRL, 0x2700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_2 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, set_RDD_POLICER_PARAMS_ENTRY, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_2 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_2 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, set_RDD_SPDTEST_GEN_CFG, 0x370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_2 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, set_RDD_VPORT_CFG_ENTRY, 0x380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_2 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_2 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, set_RDD_TUNNELS_PARSING_CFG, 0x7e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x7f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_BAD_IPV4_HDR_CSUM_PACKETS_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x96c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_2 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TASK_IDX_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x97c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x9be },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_2 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, set_RDD_VPORT_CFG_EX_ENTRY, 0x9c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x9fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xa00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_2 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, set_RDD_IPTV_CFG, 0xb68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xb70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_TABLE_2 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, set_RDD_TR471_SPDSVC_RX_PKT_ID, 0xb80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_2 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0xbb0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xbc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_2 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0xbe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_2 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, set_RDD_TCAM_GENERIC, 0xd68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_2 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, set_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0xd78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_2 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, set_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0xd80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_2 =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, set_RDD_BUFMNG_STATUS_ENTRY, 0xda0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_2 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0xdc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xddf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_2 =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, set_RDD_DHD_HW_CONFIGURATION, 0xde0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_2 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0xdf4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_2 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0xdf6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_2 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, set_RDD_NAT_CACHE_CFG, 0xdf8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xdff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CHANNEL_ID_TO_SBPM_BB_ID_TABLE_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0xf68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xf78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_DISABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_2 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, set_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0xf7d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xfa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_2 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, set_RDD_GDX_PARAMS, 0xfd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_2 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0xfe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER_2 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, set_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_2 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, set_RDD_TCAM_TABLE_CFG, 0x2168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_2 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, set_RDD_INGRESS_FILTER_CFG, 0x2178 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2190 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x21a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_2 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, set_RDD_MULTICAST_KEY_MASK_ENTRY, 0x21c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_2 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, set_RDD_RX_FLOW_ENTRY, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_2 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0x2500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PARSER_KEY_BUILD_TABLE_2 =
{
	36,
	{
		{ dump_RDD_PARSER_KEY_BUILD_ENTRY, set_RDD_PARSER_KEY_BUILD_ENTRY, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_2 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, set_RDD_INGRESS_FILTER_CTRL, 0x2700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_3 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, set_RDD_POLICER_PARAMS_ENTRY, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_3 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_3 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, set_RDD_SPDTEST_GEN_CFG, 0x370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_3 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, set_RDD_VPORT_CFG_ENTRY, 0x380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_3 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_3 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, set_RDD_TUNNELS_PARSING_CFG, 0x7e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x7f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_BAD_IPV4_HDR_CSUM_PACKETS_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x96c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_3 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TASK_IDX_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x97c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x9be },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_3 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, set_RDD_VPORT_CFG_EX_ENTRY, 0x9c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x9fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xa00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_3 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, set_RDD_IPTV_CFG, 0xb68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xb70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_TABLE_3 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, set_RDD_TR471_SPDSVC_RX_PKT_ID, 0xb80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_3 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0xbb0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xbc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_3 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0xbe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_3 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, set_RDD_TCAM_GENERIC, 0xd68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_3 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, set_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0xd78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_3 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, set_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0xd80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_3 =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, set_RDD_BUFMNG_STATUS_ENTRY, 0xda0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_3 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0xdc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xddf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_3 =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, set_RDD_DHD_HW_CONFIGURATION, 0xde0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_3 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0xdf4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_3 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0xdf6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_3 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, set_RDD_NAT_CACHE_CFG, 0xdf8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xdff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CHANNEL_ID_TO_SBPM_BB_ID_TABLE_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0xf68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xf78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_DISABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_3 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, set_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0xf7d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xfa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_3 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, set_RDD_GDX_PARAMS, 0xfd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_3 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0xfe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER_3 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, set_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_3 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, set_RDD_TCAM_TABLE_CFG, 0x2168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_3 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, set_RDD_INGRESS_FILTER_CFG, 0x2178 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2190 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x21a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_3 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, set_RDD_MULTICAST_KEY_MASK_ENTRY, 0x21c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_3 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, set_RDD_RX_FLOW_ENTRY, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_3 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0x2500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PARSER_KEY_BUILD_TABLE_3 =
{
	36,
	{
		{ dump_RDD_PARSER_KEY_BUILD_ENTRY, set_RDD_PARSER_KEY_BUILD_ENTRY, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_3 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, set_RDD_INGRESS_FILTER_CTRL, 0x2700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_4 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, set_RDD_POLICER_PARAMS_ENTRY, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_4 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_4 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, set_RDD_SPDTEST_GEN_CFG, 0x370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_4 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, set_RDD_VPORT_CFG_ENTRY, 0x380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_4 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_4 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, set_RDD_TUNNELS_PARSING_CFG, 0x7e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x7f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_BAD_IPV4_HDR_CSUM_PACKETS_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x96c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_4 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TASK_IDX_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x97c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_4 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_4 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x9be },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_4 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, set_RDD_VPORT_CFG_EX_ENTRY, 0x9c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_4 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x9fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xa00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_4 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, set_RDD_IPTV_CFG, 0xb68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xb70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_TABLE_4 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, set_RDD_TR471_SPDSVC_RX_PKT_ID, 0xb80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_4 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0xbb0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xbc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_4 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0xbe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_4 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, set_RDD_TCAM_GENERIC, 0xd68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_4 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, set_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0xd78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_4 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, set_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0xd80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_4 =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, set_RDD_BUFMNG_STATUS_ENTRY, 0xda0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_4 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0xdc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xddf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_4 =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, set_RDD_DHD_HW_CONFIGURATION, 0xde0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_4 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0xdf4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_4 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0xdf6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_4 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, set_RDD_NAT_CACHE_CFG, 0xdf8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xdff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CHANNEL_ID_TO_SBPM_BB_ID_TABLE_4 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0xf68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xf78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_DISABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_4 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, set_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0xf7d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xfa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_4 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, set_RDD_GDX_PARAMS, 0xfd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_4 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0xfe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER_4 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, set_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_4 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, set_RDD_TCAM_TABLE_CFG, 0x2168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_4 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_4 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, set_RDD_INGRESS_FILTER_CFG, 0x2178 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2190 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x21a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_4 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, set_RDD_MULTICAST_KEY_MASK_ENTRY, 0x21c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_4 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, set_RDD_RX_FLOW_ENTRY, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_4 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0x2500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PARSER_KEY_BUILD_TABLE_4 =
{
	36,
	{
		{ dump_RDD_PARSER_KEY_BUILD_ENTRY, set_RDD_PARSER_KEY_BUILD_ENTRY, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_4 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, set_RDD_INGRESS_FILTER_CTRL, 0x2700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_5 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, set_RDD_POLICER_PARAMS_ENTRY, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_5 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_5 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, set_RDD_SPDTEST_GEN_CFG, 0x370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_5 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, set_RDD_VPORT_CFG_ENTRY, 0x380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_5 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_5 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, set_RDD_TUNNELS_PARSING_CFG, 0x7e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x7f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_BAD_IPV4_HDR_CSUM_PACKETS_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x96c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_5 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TASK_IDX_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x97c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_5 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_5 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x9be },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_5 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, set_RDD_VPORT_CFG_EX_ENTRY, 0x9c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_5 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x9fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xa00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_5 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, set_RDD_IPTV_CFG, 0xb68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xb70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_TABLE_5 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, set_RDD_TR471_SPDSVC_RX_PKT_ID, 0xb80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_5 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0xbb0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xbc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_5 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0xbe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_5 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, set_RDD_TCAM_GENERIC, 0xd68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_5 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, set_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0xd78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_5 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, set_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0xd80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_5 =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, set_RDD_BUFMNG_STATUS_ENTRY, 0xda0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_5 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0xdc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xddf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_5 =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, set_RDD_DHD_HW_CONFIGURATION, 0xde0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_5 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0xdf4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_5 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0xdf6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_5 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, set_RDD_NAT_CACHE_CFG, 0xdf8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xdff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CHANNEL_ID_TO_SBPM_BB_ID_TABLE_5 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0xf68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xf78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_DISABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_5 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, set_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0xf7d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xfa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_5 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, set_RDD_GDX_PARAMS, 0xfd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_5 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0xfe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER_5 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, set_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_5 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, set_RDD_TCAM_TABLE_CFG, 0x2168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_5 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_5 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, set_RDD_INGRESS_FILTER_CFG, 0x2178 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2190 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x21a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_5 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, set_RDD_MULTICAST_KEY_MASK_ENTRY, 0x21c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_5 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, set_RDD_RX_FLOW_ENTRY, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_5 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0x2500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PARSER_KEY_BUILD_TABLE_5 =
{
	36,
	{
		{ dump_RDD_PARSER_KEY_BUILD_ENTRY, set_RDD_PARSER_KEY_BUILD_ENTRY, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_5 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, set_RDD_INGRESS_FILTER_CTRL, 0x2700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_6 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, set_RDD_POLICER_PARAMS_ENTRY, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_6 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_6 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, set_RDD_SPDTEST_GEN_CFG, 0x370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_6 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, set_RDD_VPORT_CFG_ENTRY, 0x380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_6 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_6 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, set_RDD_TUNNELS_PARSING_CFG, 0x7e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x7f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_BAD_IPV4_HDR_CSUM_PACKETS_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x96c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_6 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TASK_IDX_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x97c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_6 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_6 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x9be },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_6 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, set_RDD_VPORT_CFG_EX_ENTRY, 0x9c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_6 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x9fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xa00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_6 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, set_RDD_IPTV_CFG, 0xb68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xb70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_TABLE_6 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, set_RDD_TR471_SPDSVC_RX_PKT_ID, 0xb80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_6 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0xbb0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xbc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_6 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0xbe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_6 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, set_RDD_TCAM_GENERIC, 0xd68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_6 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, set_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0xd78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_6 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, set_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0xd80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_6 =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, set_RDD_BUFMNG_STATUS_ENTRY, 0xda0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_6 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0xdc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xddf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_6 =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, set_RDD_DHD_HW_CONFIGURATION, 0xde0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_6 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0xdf4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_6 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0xdf6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_6 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, set_RDD_NAT_CACHE_CFG, 0xdf8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xdff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CHANNEL_ID_TO_SBPM_BB_ID_TABLE_6 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0xf68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xf78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_DISABLE_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_6 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, set_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0xf7d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xfa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_6 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, set_RDD_GDX_PARAMS, 0xfd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_6 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0xfe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER_6 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, set_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_6 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, set_RDD_TCAM_TABLE_CFG, 0x2168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_6 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_6 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, set_RDD_INGRESS_FILTER_CFG, 0x2178 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2190 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_6 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x21a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_6 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, set_RDD_MULTICAST_KEY_MASK_ENTRY, 0x21c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK_6 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_6 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, set_RDD_RX_FLOW_ENTRY, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_6 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0x2500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PARSER_KEY_BUILD_TABLE_6 =
{
	36,
	{
		{ dump_RDD_PARSER_KEY_BUILD_ENTRY, set_RDD_PARSER_KEY_BUILD_ENTRY, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_6 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, set_RDD_INGRESS_FILTER_CTRL, 0x2700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_7 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, set_RDD_POLICER_PARAMS_ENTRY, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_7 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_7 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, set_RDD_SPDTEST_GEN_CFG, 0x370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_7 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, set_RDD_VPORT_CFG_ENTRY, 0x380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_7 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_7 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, set_RDD_TUNNELS_PARSING_CFG, 0x7e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x7f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_BAD_IPV4_HDR_CSUM_PACKETS_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x96c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_7 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TASK_IDX_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x97c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_7 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_7 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x9be },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_7 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, set_RDD_VPORT_CFG_EX_ENTRY, 0x9c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_7 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x9fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xa00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_7 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, set_RDD_IPTV_CFG, 0xb68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xb70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_TABLE_7 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, set_RDD_TR471_SPDSVC_RX_PKT_ID, 0xb80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_7 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0xbb0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xbc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_7 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0xbe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_7 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, set_RDD_TCAM_GENERIC, 0xd68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_7 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, set_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0xd78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_7 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, set_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0xd80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_7 =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, set_RDD_BUFMNG_STATUS_ENTRY, 0xda0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_7 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0xdc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xddf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_7 =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, set_RDD_DHD_HW_CONFIGURATION, 0xde0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_7 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0xdf4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_7 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0xdf6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_7 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, set_RDD_NAT_CACHE_CFG, 0xdf8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xdff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CHANNEL_ID_TO_SBPM_BB_ID_TABLE_7 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0xf68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xf78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_DISABLE_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_7 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, set_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0xf7d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xfa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_7 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, set_RDD_GDX_PARAMS, 0xfd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_7 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0xfe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER_7 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, set_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_7 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, set_RDD_TCAM_TABLE_CFG, 0x2168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_7 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_7 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, set_RDD_INGRESS_FILTER_CFG, 0x2178 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2190 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_7 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x21a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_7 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, set_RDD_MULTICAST_KEY_MASK_ENTRY, 0x21c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK_7 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_7 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, set_RDD_RX_FLOW_ENTRY, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_7 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0x2500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PARSER_KEY_BUILD_TABLE_7 =
{
	36,
	{
		{ dump_RDD_PARSER_KEY_BUILD_ENTRY, set_RDD_PARSER_KEY_BUILD_ENTRY, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_7 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, set_RDD_INGRESS_FILTER_CTRL, 0x2700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_8 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, set_RDD_POLICER_PARAMS_ENTRY, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_8 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_8 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, set_RDD_SPDTEST_GEN_CFG, 0x370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_8 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, set_RDD_VPORT_CFG_ENTRY, 0x380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_8 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_8 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_8 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_8 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, set_RDD_TUNNELS_PARSING_CFG, 0x7e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x7f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_8 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_BAD_IPV4_HDR_CSUM_PACKETS_8 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x96c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_8 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TASK_IDX_8 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x97c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_8 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_8 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x9be },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_8 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, set_RDD_VPORT_CFG_EX_ENTRY, 0x9c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_8 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x9fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xa00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_8 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, set_RDD_IPTV_CFG, 0xb68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_8 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xb70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_TABLE_8 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, set_RDD_TR471_SPDSVC_RX_PKT_ID, 0xb80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_8 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0xbb0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_8 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xbc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_8 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0xbe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_8 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, set_RDD_TCAM_GENERIC, 0xd68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_8 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, set_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0xd78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_8 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, set_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0xd80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_8 =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, set_RDD_BUFMNG_STATUS_ENTRY, 0xda0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_8 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0xdc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xddf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_8 =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, set_RDD_DHD_HW_CONFIGURATION, 0xde0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_8 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0xdf4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_8 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0xdf6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_8 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, set_RDD_NAT_CACHE_CFG, 0xdf8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xdff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CHANNEL_ID_TO_SBPM_BB_ID_TABLE_8 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0xf68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_8 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xf78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_DISABLE_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_8 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, set_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0xf7d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_8 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xfa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_8 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, set_RDD_GDX_PARAMS, 0xfd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_8 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0xfe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER_8 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, set_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_8 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, set_RDD_TCAM_TABLE_CFG, 0x2168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_8 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_8 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, set_RDD_INGRESS_FILTER_CFG, 0x2178 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_8 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2190 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_8 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x21a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_8 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, set_RDD_MULTICAST_KEY_MASK_ENTRY, 0x21c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK_8 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_8 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, set_RDD_RX_FLOW_ENTRY, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_8 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0x2500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PARSER_KEY_BUILD_TABLE_8 =
{
	36,
	{
		{ dump_RDD_PARSER_KEY_BUILD_ENTRY, set_RDD_PARSER_KEY_BUILD_ENTRY, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_8 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, set_RDD_INGRESS_FILTER_CTRL, 0x2700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_9 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, set_RDD_POLICER_PARAMS_ENTRY, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_9 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_9 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, set_RDD_SPDTEST_GEN_CFG, 0x370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_9 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, set_RDD_VPORT_CFG_ENTRY, 0x380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_9 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_9 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_9 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_9 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, set_RDD_TUNNELS_PARSING_CFG, 0x7e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x7f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_9 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_BAD_IPV4_HDR_CSUM_PACKETS_9 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x96c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_9 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TASK_IDX_9 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x97c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_9 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_9 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x9be },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_9 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, set_RDD_VPORT_CFG_EX_ENTRY, 0x9c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_9 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x9fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xa00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_9 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, set_RDD_IPTV_CFG, 0xb68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_9 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xb70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_TABLE_9 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, set_RDD_TR471_SPDSVC_RX_PKT_ID, 0xb80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_9 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0xbb0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_9 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xbc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_9 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0xbe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_9 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, set_RDD_TCAM_GENERIC, 0xd68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_9 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, set_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0xd78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_9 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, set_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0xd80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_9 =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, set_RDD_BUFMNG_STATUS_ENTRY, 0xda0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_9 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0xdc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xddf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_9 =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, set_RDD_DHD_HW_CONFIGURATION, 0xde0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_9 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0xdf4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_9 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0xdf6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_9 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, set_RDD_NAT_CACHE_CFG, 0xdf8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xdff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CHANNEL_ID_TO_SBPM_BB_ID_TABLE_9 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0xf68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_9 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xf78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_DISABLE_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_9 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, set_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0xf7d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_9 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xfa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_9 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, set_RDD_GDX_PARAMS, 0xfd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_9 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0xfe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER_9 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, set_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_9 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, set_RDD_TCAM_TABLE_CFG, 0x2168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_9 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_9 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, set_RDD_INGRESS_FILTER_CFG, 0x2178 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_9 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2190 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_9 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x21a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_9 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, set_RDD_MULTICAST_KEY_MASK_ENTRY, 0x21c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK_9 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_9 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, set_RDD_RX_FLOW_ENTRY, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_9 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0x2500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PARSER_KEY_BUILD_TABLE_9 =
{
	36,
	{
		{ dump_RDD_PARSER_KEY_BUILD_ENTRY, set_RDD_PARSER_KEY_BUILD_ENTRY, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_9 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, set_RDD_INGRESS_FILTER_CTRL, 0x2700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_10 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, set_RDD_POLICER_PARAMS_ENTRY, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_10 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_10 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, set_RDD_SPDTEST_GEN_CFG, 0x370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_10 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, set_RDD_VPORT_CFG_ENTRY, 0x380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_10 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_10 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_10 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_10 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, set_RDD_TUNNELS_PARSING_CFG, 0x7e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x7f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_10 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_BAD_IPV4_HDR_CSUM_PACKETS_10 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x96c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_10 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TASK_IDX_10 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x97c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_10 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_10 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x9be },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_10 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, set_RDD_VPORT_CFG_EX_ENTRY, 0x9c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_10 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x9fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xa00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_10 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, set_RDD_IPTV_CFG, 0xb68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_10 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xb70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_TABLE_10 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, set_RDD_TR471_SPDSVC_RX_PKT_ID, 0xb80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_10 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0xbb0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_10 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xbc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_10 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0xbe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_10 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, set_RDD_TCAM_GENERIC, 0xd68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_10 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, set_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0xd78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_10 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, set_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0xd80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_10 =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, set_RDD_BUFMNG_STATUS_ENTRY, 0xda0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_10 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0xdc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xddf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_10 =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, set_RDD_DHD_HW_CONFIGURATION, 0xde0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_10 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0xdf4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_10 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0xdf6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_10 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, set_RDD_NAT_CACHE_CFG, 0xdf8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xdff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CHANNEL_ID_TO_SBPM_BB_ID_TABLE_10 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0xf68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_10 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xf78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_DISABLE_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_10 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, set_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0xf7d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_10 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xfa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_10 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, set_RDD_GDX_PARAMS, 0xfd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_10 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0xfe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER_10 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, set_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_10 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, set_RDD_TCAM_TABLE_CFG, 0x2168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_10 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_10 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, set_RDD_INGRESS_FILTER_CFG, 0x2178 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_10 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2190 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_10 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x21a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_10 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, set_RDD_MULTICAST_KEY_MASK_ENTRY, 0x21c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK_10 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_10 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, set_RDD_RX_FLOW_ENTRY, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_10 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0x2500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PARSER_KEY_BUILD_TABLE_10 =
{
	36,
	{
		{ dump_RDD_PARSER_KEY_BUILD_ENTRY, set_RDD_PARSER_KEY_BUILD_ENTRY, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_10 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, set_RDD_INGRESS_FILTER_CTRL, 0x2700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_11 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, set_RDD_POLICER_PARAMS_ENTRY, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_11 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_11 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, set_RDD_SPDTEST_GEN_CFG, 0x370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_11 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, set_RDD_VPORT_CFG_ENTRY, 0x380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_11 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_11 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_11 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_11 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, set_RDD_TUNNELS_PARSING_CFG, 0x7e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x7f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_11 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_BAD_IPV4_HDR_CSUM_PACKETS_11 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x96c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_11 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TASK_IDX_11 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x97c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_11 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_11 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x9be },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_11 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, set_RDD_VPORT_CFG_EX_ENTRY, 0x9c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_11 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x9fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xa00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_11 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, set_RDD_IPTV_CFG, 0xb68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_11 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xb70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_TABLE_11 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, set_RDD_TR471_SPDSVC_RX_PKT_ID, 0xb80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_11 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0xbb0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_11 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xbc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_11 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0xbe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_11 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, set_RDD_TCAM_GENERIC, 0xd68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_11 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, set_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0xd78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_11 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, set_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0xd80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_11 =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, set_RDD_BUFMNG_STATUS_ENTRY, 0xda0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_11 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0xdc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xddf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_11 =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, set_RDD_DHD_HW_CONFIGURATION, 0xde0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_11 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0xdf4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_11 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0xdf6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_11 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, set_RDD_NAT_CACHE_CFG, 0xdf8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xdff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CHANNEL_ID_TO_SBPM_BB_ID_TABLE_11 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0xf68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_11 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xf78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_DISABLE_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_11 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, set_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0xf7d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_11 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xfa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_11 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, set_RDD_GDX_PARAMS, 0xfd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_11 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0xfe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER_11 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, set_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_11 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, set_RDD_TCAM_TABLE_CFG, 0x2168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_11 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_11 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, set_RDD_INGRESS_FILTER_CFG, 0x2178 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_11 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2190 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_11 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x21a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_11 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, set_RDD_MULTICAST_KEY_MASK_ENTRY, 0x21c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK_11 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_11 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, set_RDD_RX_FLOW_ENTRY, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_11 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0x2500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PARSER_KEY_BUILD_TABLE_11 =
{
	36,
	{
		{ dump_RDD_PARSER_KEY_BUILD_ENTRY, set_RDD_PARSER_KEY_BUILD_ENTRY, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_11 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, set_RDD_INGRESS_FILTER_CTRL, 0x2700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_12 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, set_RDD_POLICER_PARAMS_ENTRY, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_12 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_12 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, set_RDD_SPDTEST_GEN_CFG, 0x370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_12 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, set_RDD_VPORT_CFG_ENTRY, 0x380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_12 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_12 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_12 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_12 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, set_RDD_TUNNELS_PARSING_CFG, 0x7e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x7f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_12 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_BAD_IPV4_HDR_CSUM_PACKETS_12 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x96c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_12 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TASK_IDX_12 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x97c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_12 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_12 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x9be },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_12 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, set_RDD_VPORT_CFG_EX_ENTRY, 0x9c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_12 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x9fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xa00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_12 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, set_RDD_IPTV_CFG, 0xb68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_12 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xb70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_TABLE_12 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, set_RDD_TR471_SPDSVC_RX_PKT_ID, 0xb80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_12 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0xbb0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_12 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xbc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_12 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0xbe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_12 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, set_RDD_TCAM_GENERIC, 0xd68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_12 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, set_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0xd78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_12 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, set_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0xd80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_12 =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, set_RDD_BUFMNG_STATUS_ENTRY, 0xda0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_12 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0xdc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xddf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_12 =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, set_RDD_DHD_HW_CONFIGURATION, 0xde0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_12 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0xdf4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_12 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0xdf6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_12 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, set_RDD_NAT_CACHE_CFG, 0xdf8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xdff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CHANNEL_ID_TO_SBPM_BB_ID_TABLE_12 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0xf68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_12 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xf78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_DISABLE_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_12 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, set_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0xf7d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_12 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xfa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_12 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, set_RDD_GDX_PARAMS, 0xfd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_12 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0xfe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER_12 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, set_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_12 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, set_RDD_TCAM_TABLE_CFG, 0x2168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_12 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_12 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, set_RDD_INGRESS_FILTER_CFG, 0x2178 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_12 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2190 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_12 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x21a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_12 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, set_RDD_MULTICAST_KEY_MASK_ENTRY, 0x21c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK_12 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_12 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, set_RDD_RX_FLOW_ENTRY, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_12 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0x2500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PARSER_KEY_BUILD_TABLE_12 =
{
	36,
	{
		{ dump_RDD_PARSER_KEY_BUILD_ENTRY, set_RDD_PARSER_KEY_BUILD_ENTRY, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_12 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, set_RDD_INGRESS_FILTER_CTRL, 0x2700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_13 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, set_RDD_POLICER_PARAMS_ENTRY, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_13 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_13 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, set_RDD_SPDTEST_GEN_CFG, 0x370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_13 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, set_RDD_VPORT_CFG_ENTRY, 0x380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_13 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_13 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_13 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_13 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, set_RDD_TUNNELS_PARSING_CFG, 0x7e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x7f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_13 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_BAD_IPV4_HDR_CSUM_PACKETS_13 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x96c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_13 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TASK_IDX_13 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x97c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_13 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_13 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x9be },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_13 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, set_RDD_VPORT_CFG_EX_ENTRY, 0x9c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_13 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x9fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xa00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_13 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, set_RDD_IPTV_CFG, 0xb68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_13 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xb70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_TABLE_13 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, set_RDD_TR471_SPDSVC_RX_PKT_ID, 0xb80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_13 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0xbb0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_13 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xbc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_13 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0xbe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_13 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, set_RDD_TCAM_GENERIC, 0xd68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_13 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, set_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0xd78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_13 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, set_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0xd80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_13 =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, set_RDD_BUFMNG_STATUS_ENTRY, 0xda0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_13 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0xdc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xddf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_13 =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, set_RDD_DHD_HW_CONFIGURATION, 0xde0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_13 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0xdf4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_13 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0xdf6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_13 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, set_RDD_NAT_CACHE_CFG, 0xdf8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xdff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CHANNEL_ID_TO_SBPM_BB_ID_TABLE_13 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0xf68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_13 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xf78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_DISABLE_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_13 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, set_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0xf7d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_13 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xfa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_13 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, set_RDD_GDX_PARAMS, 0xfd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_13 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0xfe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER_13 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, set_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_13 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, set_RDD_TCAM_TABLE_CFG, 0x2168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_13 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_13 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, set_RDD_INGRESS_FILTER_CFG, 0x2178 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_13 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2190 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_13 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x21a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_13 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, set_RDD_MULTICAST_KEY_MASK_ENTRY, 0x21c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK_13 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_13 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, set_RDD_RX_FLOW_ENTRY, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_13 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0x2500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PARSER_KEY_BUILD_TABLE_13 =
{
	36,
	{
		{ dump_RDD_PARSER_KEY_BUILD_ENTRY, set_RDD_PARSER_KEY_BUILD_ENTRY, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_13 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, set_RDD_INGRESS_FILTER_CTRL, 0x2700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_14 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, set_RDD_POLICER_PARAMS_ENTRY, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_14 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_14 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_14 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, set_RDD_SPDTEST_GEN_CFG, 0x370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DIRECT_FLOW_STACK_14 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_14 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_14 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_14 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, set_RDD_TUNNELS_PARSING_CFG, 0x768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DIRECT_FLOW_PD_TABLE_14 =
{
	16,
	{
		{ dump_RDD_PROCESSING_RX_DESCRIPTOR, set_RDD_PROCESSING_RX_DESCRIPTOR, 0x770 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_14 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, set_RDD_VPORT_CFG_ENTRY, 0x780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_14 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x7fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_14 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_14 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_14 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x9e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_BAD_IPV4_HDR_CSUM_PACKETS_14 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x9ec },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_14 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x9f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_14 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xa00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_14 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, set_RDD_IPTV_CFG, 0xb68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_14 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0xb70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TASK_IDX_14 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xb7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_14 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0xb80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_14 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0xbbe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_14 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, set_RDD_VPORT_CFG_EX_ENTRY, 0xbc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_14 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0xbfe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK_14 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_14 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, set_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0xd68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DIRECT_FLOW_PAUSE_DEBUG_14 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xd70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DIRECT_FLOW_PAUSE_QUANTA_14 =
{
	2,
	{
		{ dump_RDD_PAUSE_QUANTA_ENTRY, set_RDD_PAUSE_QUANTA_ENTRY, 0xd7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DIRECT_FLOW_WAN_VIQ_EXCLUSIVE_14 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xd7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_14 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xd7f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_TABLE_14 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, set_RDD_TR471_SPDSVC_RX_PKT_ID, 0xd80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_14 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xdb0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_14 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xdbc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_14 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xdbd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_14 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xdbe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT US_TM_TX_PAUSE_NACK_14 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xdbf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_14 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xdc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_14 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0xde0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK_14 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_14 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, set_RDD_NAT_CACHE_CFG, 0xf68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_14 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf6f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_14 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0xf70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_14 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, set_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_14 =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, set_RDD_BUFMNG_STATUS_ENTRY, 0xfa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DIRECT_FLOW_CNTR_TABLE_14 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xfc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DIRECT_FLOW_EPON_CONTROL_SCRATCH_14 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xfe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_14 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0xff6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_14 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xff8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_14 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0xffc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_14 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xffe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MAC_TYPE_14 =
{
	1,
	{
		{ dump_RDD_MAC_TYPE_ENTRY, set_RDD_MAC_TYPE_ENTRY, 0xfff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER_14 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, set_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK_14 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_14 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, set_RDD_TCAM_GENERIC, 0x2168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_14 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, set_RDD_TCAM_TABLE_CFG, 0x2178 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_DISABLE_14 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x217a },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_14 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, set_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x217b },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_14 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x217c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_14 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x217d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_14 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0x2180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_14 =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, set_RDD_DHD_HW_CONFIGURATION, 0x21a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_14 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x21b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_14 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x21c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_14 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0x21e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK_14 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_14 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CHANNEL_ID_TO_SBPM_BB_ID_TABLE_14 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2398 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_14 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, set_RDD_INGRESS_FILTER_CFG, 0x23a8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_14 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, set_RDD_GDX_PARAMS, 0x23b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_14 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x23c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_14 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x23d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_14 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x23e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_14 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, set_RDD_RX_FLOW_ENTRY, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_14 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0x2500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_14 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, set_RDD_MULTICAST_KEY_MASK_ENTRY, 0x25e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PARSER_KEY_BUILD_TABLE_14 =
{
	36,
	{
		{ dump_RDD_PARSER_KEY_BUILD_ENTRY, set_RDD_PARSER_KEY_BUILD_ENTRY, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DIRECT_FLOW_RX_MIRRORING_SCRATCHPAD_14 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_14 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, set_RDD_INGRESS_FILTER_CTRL, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_15 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, set_RDD_POLICER_PARAMS_ENTRY, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_15 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_15 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_15 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, set_RDD_SPDTEST_GEN_CFG, 0x370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_15 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, set_RDD_VPORT_CFG_ENTRY, 0x380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_15 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_15 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_15 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_15 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_15 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, set_RDD_TUNNELS_PARSING_CFG, 0x7e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_15 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x7f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_15 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_15 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_BAD_IPV4_HDR_CSUM_PACKETS_15 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x96c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_15 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TASK_IDX_15 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x97c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_15 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_15 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x9be },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_15 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, set_RDD_VPORT_CFG_EX_ENTRY, 0x9c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_15 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x9fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_15 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xa00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_15 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, set_RDD_IPTV_CFG, 0xb68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_15 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xb70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_15 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_15 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_15 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_15 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_TABLE_15 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, set_RDD_TR471_SPDSVC_RX_PKT_ID, 0xb80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_15 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0xbb0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_15 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xbc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_15 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0xbe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK_15 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_15 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, set_RDD_TCAM_GENERIC, 0xd68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_15 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, set_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0xd78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_15 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, set_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0xd80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_15 =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, set_RDD_BUFMNG_STATUS_ENTRY, 0xda0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_15 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0xdc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_15 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xddf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_15 =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, set_RDD_DHD_HW_CONFIGURATION, 0xde0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_15 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0xdf4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_15 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0xdf6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_15 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, set_RDD_NAT_CACHE_CFG, 0xdf8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_15 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xdff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK_15 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CHANNEL_ID_TO_SBPM_BB_ID_TABLE_15 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0xf68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_15 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xf78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_DISABLE_15 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_15 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, set_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0xf7d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_15 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_15 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_15 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_15 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xfa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_15 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, set_RDD_GDX_PARAMS, 0xfd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_15 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0xfe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER_15 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, set_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK_15 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_15 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, set_RDD_TCAM_TABLE_CFG, 0x2168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_15 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_15 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, set_RDD_INGRESS_FILTER_CFG, 0x2178 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_15 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_15 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2190 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_15 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x21a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_15 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, set_RDD_MULTICAST_KEY_MASK_ENTRY, 0x21c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK_15 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_15 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, set_RDD_RX_FLOW_ENTRY, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_15 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0x2500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PARSER_KEY_BUILD_TABLE_15 =
{
	36,
	{
		{ dump_RDD_PARSER_KEY_BUILD_ENTRY, set_RDD_PARSER_KEY_BUILD_ENTRY, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_15 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, set_RDD_INGRESS_FILTER_CTRL, 0x2700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_STREAM_TABLE_16 =
{
	376,
	{
		{ dump_RDD_TCPSPDTEST_STREAM, set_RDD_TCPSPDTEST_STREAM, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_16 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x5e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_16 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, set_RDD_POLICER_PARAMS_ENTRY, 0x600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_16 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_16 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xa00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_16 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0xb68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_16 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, set_RDD_SPDTEST_GEN_CFG, 0xb70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_SCRATCHPAD_16 =
{
	8,
	{
		{ dump_RDD_BYTES_8, set_RDD_BYTES_8, 0xb80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_16 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_16 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, set_RDD_TUNNELS_PARSING_CFG, 0xd68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDSVC_ANALYZER_PARAMS_TABLE_16 =
{
	16,
	{
		{ dump_RDD_SPDSVC_ANALYZER_PARAMS, set_RDD_SPDSVC_ANALYZER_PARAMS, 0xd70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_UPLOAD_STACK_16 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xd80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_16 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_16 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xf68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_16 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xf6c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_16 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_16 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, set_RDD_VPORT_CFG_ENTRY, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_BAD_IPV4_HDR_CSUM_PACKETS_16 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xffc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER_16 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, set_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_16 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_16 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_16 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, set_RDD_IPTV_CFG, 0x21e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_DOWNLOAD_DISPATCHER_CREDIT_TABLE_16 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x21f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TASK_IDX_16 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x21fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK_16 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_16 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, set_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x2368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_UPLOAD_DISPATCHER_CREDIT_TABLE_16 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_16 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x237c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDSVC_TCPSPDTEST_COMMON_TABLE_16 =
{
	2,
	{
		{ dump_RDD_SPDSVC_TCPSPDTEST_COMMON_ENTRY, set_RDD_SPDSVC_TCPSPDTEST_COMMON_ENTRY, 0x237e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_UPLOAD_TIMER_STACK_16 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_16 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x23c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_16 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x23fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK_16 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_16 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, set_RDD_NAT_CACHE_CFG, 0x2568 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_16 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x256f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_UPLOAD_TIMER_DISPATCHER_CREDIT_TABLE_16 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2570 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_16 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x257c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_16 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x257d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_16 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x257e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_16 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x257f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_DOWNLOAD_PD_FIFO_TABLE_16 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, set_RDD_PROCESSING_TX_DESCRIPTOR, 0x2580 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_16 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, set_RDD_VPORT_CFG_EX_ENTRY, 0x25c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_16 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x25fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK_16 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_16 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_16 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x276c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_16 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x276e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_DISABLE_16 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x276f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_16 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x2770 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_16 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, set_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x277c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_16 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x277d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_16 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x277e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_UPLOAD_PD_FIFO_TABLE_16 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, set_RDD_PROCESSING_TX_DESCRIPTOR, 0x2780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDSVC_GEN_PARAMS_TABLE_16 =
{
	60,
	{
		{ dump_RDD_SPDSVC_GEN_PARAMS, set_RDD_SPDSVC_GEN_PARAMS, 0x27c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK_16 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_16 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, set_RDD_TCAM_TABLE_CFG, 0x2968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_16 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_TABLE_16 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, set_RDD_TR471_SPDSVC_RX_PKT_ID, 0x2980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT UDPSPDT_SCRATCH_TABLE_16 =
{
	8,
	{
		{ dump_RDD_UDPSPDT_SCRATCH_IPERF3_RX, set_RDD_UDPSPDT_SCRATCH_IPERF3_RX, 0x29b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_16 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x29b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_16 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0x29c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_16 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, set_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0x29e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_16 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, set_RDD_RX_FLOW_ENTRY, 0x2a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_DOWNLOAD_STACK_16 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_16 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0x2bf0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_16 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0x2c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_16 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, set_RDD_INGRESS_FILTER_CFG, 0x2cd8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_16 =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, set_RDD_BUFMNG_STATUS_ENTRY, 0x2ce0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDSVC_ANALYZER_STACK_16 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2d00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_16 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, set_RDD_TCAM_GENERIC, 0x2db0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDSVC_ANALYZER_UPDATE_FIFO_TABLE_16 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, set_RDD_UPDATE_FIFO_ENTRY, 0x2dc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_ENGINE_GLOBAL_TABLE_16 =
{
	20,
	{
		{ dump_RDD_TCPSPDTEST_ENGINE_GLOBAL_INFO, set_RDD_TCPSPDTEST_ENGINE_GLOBAL_INFO, 0x2de0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_16 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2df8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT UDPSPDT_STREAM_RX_STAT_TABLE_16 =
{
	48,
	{
		{ dump_RDD_UDPSPDT_STREAM_RX_STAT, set_RDD_UDPSPDT_STREAM_RX_STAT, 0x2e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_DOWNLOAD_UPDATE_FIFO_TABLE_16 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, set_RDD_UPDATE_FIFO_ENTRY, 0x2ec0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_16 =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, set_RDD_DHD_HW_CONFIGURATION, 0x2ee0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PARSER_KEY_BUILD_TABLE_16 =
{
	36,
	{
		{ dump_RDD_PARSER_KEY_BUILD_ENTRY, set_RDD_PARSER_KEY_BUILD_ENTRY, 0x2f00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_16 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2f90 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_UPLOAD_UPDATE_FIFO_TABLE_16 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, set_RDD_UPDATE_FIFO_ENTRY, 0x2fc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_16 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0x2fe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT UDPSPDT_STREAM_TX_STAT_TABLE_16 =
{
	40,
	{
		{ dump_RDD_UDPSPDT_STREAM_TX_STAT, set_RDD_UDPSPDT_STREAM_TX_STAT, 0x3000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CHANNEL_ID_TO_SBPM_BB_ID_TABLE_16 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x30a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_16 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, set_RDD_GDX_PARAMS, 0x30b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_16 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0x30c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_16 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x30e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_16 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, set_RDD_INGRESS_FILTER_CTRL, 0x3100 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_16 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_16 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, set_RDD_MULTICAST_KEY_MASK_ENTRY, 0x31a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_16 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x31c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT UDPSPDT_TX_PARAMS_TABLE_16 =
{
	24,
	{
		{ dump_RDD_UDPSPDT_TX_PARAMS, set_RDD_UDPSPDT_TX_PARAMS, 0x3200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CPU_TX_SCRATCHPAD_17 =
{
	8,
	{
		{ dump_RDD_BYTES_8, set_RDD_BYTES_8, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_17 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, set_RDD_POLICER_PARAMS_ENTRY, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_17 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_17 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_17 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, set_RDD_SPDTEST_GEN_CFG, 0x770 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_17 =
{
	8,
	{
		{ dump_RDD_GENERAL_TIMER_ENTRY, set_RDD_GENERAL_TIMER_ENTRY, 0x780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_17 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_17 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xa00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_17 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, set_RDD_DDR_ADDRESS, 0xb68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFFER_ALLOC_REPLY_17 =
{
	8,
	{
		{ dump_RDD_BYTES_8, set_RDD_BYTES_8, 0xb70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_17 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, set_RDD_VPORT_CFG_ENTRY, 0xb80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DDR_LATENCY_DBG_USEC_17 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xbfc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_17 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CPU_RX_INTERRUPT_ID_DDR_ADDR_17 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, set_RDD_DDR_ADDRESS, 0xd68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_INTERRUPT_COALESCING_TABLE_17 =
{
	16,
	{
		{ dump_RDD_CPU_INTERRUPT_COALESCING_ENTRY, set_RDD_CPU_INTERRUPT_COALESCING_ENTRY, 0xd70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SG_DESC_TABLE_17 =
{
	48,
	{
		{ dump_RDD_SG_DESC_ENTRY, set_RDD_SG_DESC_ENTRY, 0xd80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_17 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xde0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_17 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CPU_TX_RING_INDICES_VALUES_TABLE_17 =
{
	4,
	{
		{ dump_RDD_CPU_TX_RING_INDICES, set_RDD_CPU_TX_RING_INDICES, 0xf68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_17 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_STACK_17 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DDR_LATENCY_DBG_USEC_MAX_17 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xfc8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_17 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xfcc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE_17 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xfd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_17 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xfdc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_17 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0xfe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER_17 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, set_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK_17 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_17 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_17 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, set_RDD_TUNNELS_PARSING_CFG, 0x21e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CPU_TX_INGRESS_DISPATCHER_CREDIT_TABLE_17 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x21f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_BAD_IPV4_HDR_CSUM_PACKETS_17 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x21fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK_17 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE_17 =
{
	8,
	{
		{ dump_RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY, set_RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY, 0x2368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_17 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x2370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CPU_INT_INTERRUPT_SCRATCH_17 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x237c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_17 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_17 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x23be },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_17 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, set_RDD_VPORT_CFG_EX_ENTRY, 0x23c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_ACTION_VEC_17 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x23fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK_17 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_17 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, set_RDD_IPTV_CFG, 0x2568 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_17 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2570 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TASK_IDX_17 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x257c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT QUEUE_THRESHOLD_VECTOR_17 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2580 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_17 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, set_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x25b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_TABLE_17 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, set_RDD_TR471_SPDSVC_RX_PKT_ID, 0x25c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_17 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0x25f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK_17 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_17 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, set_RDD_TCAM_GENERIC, 0x2768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_17 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, set_RDD_NAT_CACHE_CFG, 0x2778 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_17 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x277f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RING_CPU_TX_DESCRIPTOR_DATA_TABLE_17 =
{
	16,
	{
		{ dump_RDD_RING_CPU_TX_DESCRIPTOR, set_RDD_RING_CPU_TX_DESCRIPTOR, 0x2780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_17 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, set_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0x27a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_17 =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, set_RDD_BUFMNG_STATUS_ENTRY, 0x27c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_STACK_17 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x27e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_17 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, set_RDD_RX_FLOW_ENTRY, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CPU_TX_0_STACK_17 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2900 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_17 =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, set_RDD_DHD_HW_CONFIGURATION, 0x29e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_17 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x29f4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_17 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x29f6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_17 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x29f7 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_SHADOW_RD_IDX_17 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x29f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_17 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x29fa },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_17 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x29fb },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_17 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x29fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_17 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x29fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_SRAM_PD_FIFO_17 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, set_RDD_PROCESSING_TX_DESCRIPTOR, 0x2a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CPU_TX_1_STACK_17 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CPU_TX_RING_DESCRIPTOR_TABLE_17 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, set_RDD_CPU_RING_DESCRIPTOR, 0x2be0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_17 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0x2c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_17 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2cd4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_DISABLE_17 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2cd5 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_17 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, set_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x2cd6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_17 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2cd7 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_SHADOW_WR_IDX_17 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2cd8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_17 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2cda },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_17 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0x2ce0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PARSER_KEY_BUILD_TABLE_17 =
{
	36,
	{
		{ dump_RDD_PARSER_KEY_BUILD_ENTRY, set_RDD_PARSER_KEY_BUILD_ENTRY, 0x2d00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_17 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2d90 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_17 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0x2dc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CHANNEL_ID_TO_SBPM_BB_ID_TABLE_17 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2de0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_DESCRIPTOR_TABLE_17 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, set_RDD_CPU_RING_DESCRIPTOR, 0x2df0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SG_CONTEXT_TABLE_17 =
{
	64,
	{
		{ dump_RDD_SG_CONTEXT_ENTRY, set_RDD_SG_CONTEXT_ENTRY, 0x2e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_17 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2e80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE_17 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, set_RDD_DDR_ADDRESS, 0x2ea0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_INTERRUPT_SCRATCH_17 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2eb0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_17 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2eb8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_17 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2ec0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_17 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, set_RDD_GDX_PARAMS, 0x2ed0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_17 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2ee0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_17 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, set_RDD_TCAM_TABLE_CFG, 0x2ee8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_NEXT_PTR_TABLE_17 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2ef0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_17 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2ef8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_17 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, set_RDD_INGRESS_FILTER_CTRL, 0x2f00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CPU_TX_SYNC_FIFO_TABLE_17 =
{
	8,
	{
		{ dump_RDD_CPU_TX_SYNC_FIFO_ENTRY, set_RDD_CPU_TX_SYNC_FIFO_ENTRY, 0x2f80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_17 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, set_RDD_INGRESS_FILTER_CFG, 0x2f90 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_17 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2f98 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_17 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, set_RDD_MULTICAST_KEY_MASK_ENTRY, 0x2fa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_COMPLETE_COMMON_RADIO_DATA_18 =
{
	320,
	{
		{ dump_RDD_DHD_COMPLETE_COMMON_RADIO_ENTRY, set_RDD_DHD_COMPLETE_COMMON_RADIO_ENTRY, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_18 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, set_RDD_RX_FLOW_ENTRY, 0x500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_18 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, set_RDD_POLICER_PARAMS_ENTRY, 0x600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_18 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_18 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xa00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_18 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0xb68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_18 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, set_RDD_SPDTEST_GEN_CFG, 0xb70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_FLOW_RING_BUFFER_18 =
{
	32,
	{
		{ dump_RDD_DHD_RX_COMPLETE_DESCRIPTOR, set_RDD_DHD_RX_COMPLETE_DESCRIPTOR, 0xb80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_18 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_18 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, set_RDD_DDR_ADDRESS, 0xd68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_DOORBELL_TX_COMPLETE_VALUE_18 =
{
	4,
	{
		{ dump_RDD_DHD_DOORBELL, set_RDD_DHD_DOORBELL, 0xd70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_RX_POST_FLOW_RING_BUFFER_18 =
{
	32,
	{
		{ dump_RDD_DHD_RX_POST_DESCRIPTOR, set_RDD_DHD_RX_POST_DESCRIPTOR, 0xd80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_18 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_RX_POST_RING_SIZE_18 =
{
	2,
	{
		{ dump_RDD_DHD_RING_SIZE, set_RDD_DHD_RING_SIZE, 0xf68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_DOORBELL_RX_COMPLETE_VALUE_18 =
{
	4,
	{
		{ dump_RDD_DHD_DOORBELL, set_RDD_DHD_DOORBELL, 0xf70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_0_STACK_18 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER_18 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, set_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_18 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_RING_SIZE_18 =
{
	2,
	{
		{ dump_RDD_DHD_RING_SIZE, set_RDD_DHD_RING_SIZE, 0x2168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_DOORBELL_RX_POST_VALUE_18 =
{
	4,
	{
		{ dump_RDD_DHD_DOORBELL, set_RDD_DHD_DOORBELL, 0x2170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_1_STACK_18 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK_18 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_RING_SIZE_18 =
{
	2,
	{
		{ dump_RDD_DHD_RING_SIZE, set_RDD_DHD_RING_SIZE, 0x2368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_CPU_INT_ID_18 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_2_STACK_18 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK_18 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_18 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, set_RDD_TUNNELS_PARSING_CFG, 0x2568 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_PCAP_CFG_18 =
{
	4,
	{
		{ dump_RDD_DHD_PCAP_CONFIG, set_RDD_DHD_PCAP_CONFIG, 0x2570 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_18 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, set_RDD_VPORT_CFG_ENTRY, 0x2580 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_18 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x25fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK_18 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_18 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_MIRRORING_MINIFPM_PARAMS_18 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x27e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_18 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x27f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK_18 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_18 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_BAD_IPV4_HDR_CSUM_PACKETS_18 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x296c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_18 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x2970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TASK_IDX_18 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x297c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_PCAP_TX_RING_DESCRIPTOR_TABLE_18 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, set_RDD_CPU_RING_DESCRIPTOR, 0x2980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_DISPATCHER_CREDIT_TABLE_18 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x29c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_MONITOR_PKT_INFO_TABLE_18 =
{
	56,
	{
		{ dump_RDD_DHD_MONITOR_PKT_INFO, set_RDD_DHD_MONITOR_PKT_INFO, 0x2a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_18 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2ae0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_18 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0x2b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_18 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2bd4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_18 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2bd6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_18 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, set_RDD_IPTV_CFG, 0x2bd8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_18 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0x2be0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PARSER_KEY_BUILD_TABLE_18 =
{
	36,
	{
		{ dump_RDD_PARSER_KEY_BUILD_ENTRY, set_RDD_PARSER_KEY_BUILD_ENTRY, 0x2c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_18 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2c90 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_18 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2c9c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_18 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2c9d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_18 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2c9e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_18 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2c9f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_18 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, set_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0x2ca0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_DISPATCHER_CREDIT_TABLE_18 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2cc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_18 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, set_RDD_INGRESS_FILTER_CTRL, 0x2d00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_PCAP_RX_RING_DESCRIPTOR_TABLE_18 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, set_RDD_CPU_RING_DESCRIPTOR, 0x2d80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_0_STACK_18 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2dc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE_18 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, set_RDD_CPU_RING_DESCRIPTOR, 0x2e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_1_STACK_18 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2e40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_FLOW_RING_BUFFER_18 =
{
	16,
	{
		{ dump_RDD_DHD_TX_COMPLETE_DESCRIPTOR, set_RDD_DHD_TX_COMPLETE_DESCRIPTOR, 0x2e80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_2_STACK_18 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2ec0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_18 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2f00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_18 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x2f3e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_18 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, set_RDD_VPORT_CFG_EX_ENTRY, 0x2f40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_18 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x2f7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_TABLE_18 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, set_RDD_TR471_SPDSVC_RX_PKT_ID, 0x2f80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_18 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0x2fb0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_18 =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, set_RDD_BUFMNG_STATUS_ENTRY, 0x2fc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_18 =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, set_RDD_DHD_HW_CONFIGURATION, 0x2fe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_18 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2ff4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_18 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2ff5 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_DISABLE_18 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2ff6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_18 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, set_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x2ff7 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_18 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, set_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x2ff8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_FPM_REPLY_18 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_18 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0x3020 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_18 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x303c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_18 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x303d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_18 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0x3040 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_18 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, set_RDD_TCAM_GENERIC, 0x3060 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CHANNEL_ID_TO_SBPM_BB_ID_TABLE_18 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x3070 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_18 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3080 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_18 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x30a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_18 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, set_RDD_GDX_PARAMS, 0x30d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_18 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x30e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_18 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, set_RDD_NAT_CACHE_CFG, 0x30e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_18 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x30f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_18 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, set_RDD_TCAM_TABLE_CFG, 0x30f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_18 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3100 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_18 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x3110 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_18 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, set_RDD_INGRESS_FILTER_CFG, 0x3118 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_18 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, set_RDD_MULTICAST_KEY_MASK_ENTRY, 0x3120 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_18 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3128 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_COMPLETE_COMMON_RADIO_DATA_19 =
{
	320,
	{
		{ dump_RDD_DHD_COMPLETE_COMMON_RADIO_ENTRY, set_RDD_DHD_COMPLETE_COMMON_RADIO_ENTRY, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_19 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, set_RDD_RX_FLOW_ENTRY, 0x500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_19 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, set_RDD_POLICER_PARAMS_ENTRY, 0x600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_19 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_19 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xa00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_19 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0xb68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_19 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, set_RDD_SPDTEST_GEN_CFG, 0xb70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_FLOW_RING_BUFFER_19 =
{
	32,
	{
		{ dump_RDD_DHD_RX_COMPLETE_DESCRIPTOR, set_RDD_DHD_RX_COMPLETE_DESCRIPTOR, 0xb80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_19 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_19 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, set_RDD_DDR_ADDRESS, 0xd68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_DOORBELL_TX_COMPLETE_VALUE_19 =
{
	4,
	{
		{ dump_RDD_DHD_DOORBELL, set_RDD_DHD_DOORBELL, 0xd70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_RX_POST_FLOW_RING_BUFFER_19 =
{
	32,
	{
		{ dump_RDD_DHD_RX_POST_DESCRIPTOR, set_RDD_DHD_RX_POST_DESCRIPTOR, 0xd80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_19 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_RX_POST_RING_SIZE_19 =
{
	2,
	{
		{ dump_RDD_DHD_RING_SIZE, set_RDD_DHD_RING_SIZE, 0xf68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_DOORBELL_RX_COMPLETE_VALUE_19 =
{
	4,
	{
		{ dump_RDD_DHD_DOORBELL, set_RDD_DHD_DOORBELL, 0xf70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_0_STACK_19 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER_19 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, set_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_19 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_RING_SIZE_19 =
{
	2,
	{
		{ dump_RDD_DHD_RING_SIZE, set_RDD_DHD_RING_SIZE, 0x2168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_DOORBELL_RX_POST_VALUE_19 =
{
	4,
	{
		{ dump_RDD_DHD_DOORBELL, set_RDD_DHD_DOORBELL, 0x2170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_1_STACK_19 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK_19 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_RING_SIZE_19 =
{
	2,
	{
		{ dump_RDD_DHD_RING_SIZE, set_RDD_DHD_RING_SIZE, 0x2368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_CPU_INT_ID_19 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_2_STACK_19 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK_19 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_19 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, set_RDD_TUNNELS_PARSING_CFG, 0x2568 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_PCAP_CFG_19 =
{
	4,
	{
		{ dump_RDD_DHD_PCAP_CONFIG, set_RDD_DHD_PCAP_CONFIG, 0x2570 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_19 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, set_RDD_VPORT_CFG_ENTRY, 0x2580 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_19 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x25fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK_19 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_19 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_MIRRORING_MINIFPM_PARAMS_19 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x27e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_19 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x27f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK_19 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_19 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_BAD_IPV4_HDR_CSUM_PACKETS_19 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x296c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_19 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x2970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TASK_IDX_19 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x297c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_PCAP_TX_RING_DESCRIPTOR_TABLE_19 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, set_RDD_CPU_RING_DESCRIPTOR, 0x2980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_DISPATCHER_CREDIT_TABLE_19 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x29c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_MONITOR_PKT_INFO_TABLE_19 =
{
	56,
	{
		{ dump_RDD_DHD_MONITOR_PKT_INFO, set_RDD_DHD_MONITOR_PKT_INFO, 0x2a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_19 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2ae0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_19 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0x2b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_19 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2bd4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_19 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2bd6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_19 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, set_RDD_IPTV_CFG, 0x2bd8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_19 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0x2be0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PARSER_KEY_BUILD_TABLE_19 =
{
	36,
	{
		{ dump_RDD_PARSER_KEY_BUILD_ENTRY, set_RDD_PARSER_KEY_BUILD_ENTRY, 0x2c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_19 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2c90 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_19 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2c9c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_19 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2c9d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_19 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2c9e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_19 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2c9f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_19 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, set_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0x2ca0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_DISPATCHER_CREDIT_TABLE_19 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2cc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_19 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, set_RDD_INGRESS_FILTER_CTRL, 0x2d00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_PCAP_RX_RING_DESCRIPTOR_TABLE_19 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, set_RDD_CPU_RING_DESCRIPTOR, 0x2d80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_0_STACK_19 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2dc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE_19 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, set_RDD_CPU_RING_DESCRIPTOR, 0x2e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_1_STACK_19 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2e40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_FLOW_RING_BUFFER_19 =
{
	16,
	{
		{ dump_RDD_DHD_TX_COMPLETE_DESCRIPTOR, set_RDD_DHD_TX_COMPLETE_DESCRIPTOR, 0x2e80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_2_STACK_19 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2ec0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_19 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2f00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_19 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x2f3e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_19 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, set_RDD_VPORT_CFG_EX_ENTRY, 0x2f40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_19 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0x2f7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_TABLE_19 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, set_RDD_TR471_SPDSVC_RX_PKT_ID, 0x2f80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_19 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0x2fb0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_19 =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, set_RDD_BUFMNG_STATUS_ENTRY, 0x2fc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_19 =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, set_RDD_DHD_HW_CONFIGURATION, 0x2fe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_19 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2ff4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_19 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2ff5 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_DISABLE_19 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2ff6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_19 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, set_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x2ff7 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_19 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, set_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x2ff8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_FPM_REPLY_19 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_19 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0x3020 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_19 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x303c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_19 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x303d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_19 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0x3040 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_19 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, set_RDD_TCAM_GENERIC, 0x3060 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CHANNEL_ID_TO_SBPM_BB_ID_TABLE_19 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x3070 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_19 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3080 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_19 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x30a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_19 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, set_RDD_GDX_PARAMS, 0x30d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_19 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x30e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_19 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, set_RDD_NAT_CACHE_CFG, 0x30e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_19 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x30f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_19 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, set_RDD_TCAM_TABLE_CFG, 0x30f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_19 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3100 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_19 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x3110 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_19 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, set_RDD_INGRESS_FILTER_CFG, 0x3118 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_19 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, set_RDD_MULTICAST_KEY_MASK_ENTRY, 0x3120 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_19 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3128 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_20 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, set_RDD_POLICER_PARAMS_ENTRY, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_20 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_20 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_20 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, set_RDD_SPDTEST_GEN_CFG, 0x370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_20 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, set_RDD_VPORT_CFG_ENTRY, 0x380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_20 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_20 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_20 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_20 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_20 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, set_RDD_TUNNELS_PARSING_CFG, 0x7e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_20 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x7f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_20 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_20 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_BAD_IPV4_HDR_CSUM_PACKETS_20 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x96c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_20 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TASK_IDX_20 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x97c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_20 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_20 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x9be },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_20 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, set_RDD_VPORT_CFG_EX_ENTRY, 0x9c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_20 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x9fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_20 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xa00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_20 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, set_RDD_IPTV_CFG, 0xb68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_20 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xb70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_20 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_20 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_20 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_20 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_TABLE_20 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, set_RDD_TR471_SPDSVC_RX_PKT_ID, 0xb80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_20 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0xbb0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_20 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xbc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_20 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0xbe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK_20 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_20 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, set_RDD_TCAM_GENERIC, 0xd68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_20 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, set_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0xd78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_20 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, set_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0xd80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_20 =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, set_RDD_BUFMNG_STATUS_ENTRY, 0xda0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_20 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0xdc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_20 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xddf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_20 =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, set_RDD_DHD_HW_CONFIGURATION, 0xde0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_20 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0xdf4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_20 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0xdf6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_20 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, set_RDD_NAT_CACHE_CFG, 0xdf8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_20 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xdff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK_20 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CHANNEL_ID_TO_SBPM_BB_ID_TABLE_20 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0xf68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_20 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xf78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_DISABLE_20 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_20 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, set_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0xf7d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_20 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_20 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_20 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_20 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xfa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_20 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, set_RDD_GDX_PARAMS, 0xfd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_20 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0xfe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER_20 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, set_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK_20 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_20 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, set_RDD_TCAM_TABLE_CFG, 0x2168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_20 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_20 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, set_RDD_INGRESS_FILTER_CFG, 0x2178 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_20 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_20 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2190 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_20 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x21a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_20 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, set_RDD_MULTICAST_KEY_MASK_ENTRY, 0x21c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK_20 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_20 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, set_RDD_RX_FLOW_ENTRY, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_20 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0x2500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PARSER_KEY_BUILD_TABLE_20 =
{
	36,
	{
		{ dump_RDD_PARSER_KEY_BUILD_ENTRY, set_RDD_PARSER_KEY_BUILD_ENTRY, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_20 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, set_RDD_INGRESS_FILTER_CTRL, 0x2700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_21 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, set_RDD_POLICER_PARAMS_ENTRY, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_21 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_21 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_21 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, set_RDD_SPDTEST_GEN_CFG, 0x370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_21 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, set_RDD_VPORT_CFG_ENTRY, 0x380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_21 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_21 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_21 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_21 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_21 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, set_RDD_TUNNELS_PARSING_CFG, 0x7e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_21 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x7f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_21 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_21 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_BAD_IPV4_HDR_CSUM_PACKETS_21 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x96c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_21 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TASK_IDX_21 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x97c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_21 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_21 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x9be },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_21 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, set_RDD_VPORT_CFG_EX_ENTRY, 0x9c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_21 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x9fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_21 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xa00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_21 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, set_RDD_IPTV_CFG, 0xb68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_21 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xb70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_21 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_21 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_21 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_21 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_TABLE_21 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, set_RDD_TR471_SPDSVC_RX_PKT_ID, 0xb80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_21 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0xbb0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_21 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xbc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_21 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0xbe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK_21 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_21 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, set_RDD_TCAM_GENERIC, 0xd68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_21 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, set_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0xd78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_21 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, set_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0xd80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_21 =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, set_RDD_BUFMNG_STATUS_ENTRY, 0xda0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_21 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0xdc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_21 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xddf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_21 =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, set_RDD_DHD_HW_CONFIGURATION, 0xde0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_21 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0xdf4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_21 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0xdf6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_21 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, set_RDD_NAT_CACHE_CFG, 0xdf8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_21 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xdff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK_21 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CHANNEL_ID_TO_SBPM_BB_ID_TABLE_21 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0xf68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_21 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xf78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_DISABLE_21 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_21 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, set_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0xf7d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_21 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_21 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_21 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_21 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xfa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_21 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, set_RDD_GDX_PARAMS, 0xfd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_21 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0xfe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER_21 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, set_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK_21 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_21 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, set_RDD_TCAM_TABLE_CFG, 0x2168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_21 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_21 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, set_RDD_INGRESS_FILTER_CFG, 0x2178 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_21 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_21 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2190 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_21 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x21a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_21 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, set_RDD_MULTICAST_KEY_MASK_ENTRY, 0x21c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK_21 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_21 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, set_RDD_RX_FLOW_ENTRY, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_21 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0x2500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PARSER_KEY_BUILD_TABLE_21 =
{
	36,
	{
		{ dump_RDD_PARSER_KEY_BUILD_ENTRY, set_RDD_PARSER_KEY_BUILD_ENTRY, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_21 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, set_RDD_INGRESS_FILTER_CTRL, 0x2700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_22 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, set_RDD_POLICER_PARAMS_ENTRY, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_22 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_22 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_22 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, set_RDD_SPDTEST_GEN_CFG, 0x370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_22 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, set_RDD_VPORT_CFG_ENTRY, 0x380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_22 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_22 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_22 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_22 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_22 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, set_RDD_TUNNELS_PARSING_CFG, 0x7e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_22 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x7f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_22 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_22 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_BAD_IPV4_HDR_CSUM_PACKETS_22 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x96c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_22 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TASK_IDX_22 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x97c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_22 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_22 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x9be },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_22 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, set_RDD_VPORT_CFG_EX_ENTRY, 0x9c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_22 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x9fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_22 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xa00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_22 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, set_RDD_IPTV_CFG, 0xb68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_22 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xb70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_22 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_22 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_22 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_22 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_TABLE_22 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, set_RDD_TR471_SPDSVC_RX_PKT_ID, 0xb80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_22 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0xbb0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_22 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xbc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_22 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0xbe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK_22 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_22 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, set_RDD_TCAM_GENERIC, 0xd68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_22 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, set_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0xd78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_22 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, set_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0xd80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_22 =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, set_RDD_BUFMNG_STATUS_ENTRY, 0xda0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_22 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0xdc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_22 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xddf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_22 =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, set_RDD_DHD_HW_CONFIGURATION, 0xde0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_22 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0xdf4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_22 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0xdf6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_22 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, set_RDD_NAT_CACHE_CFG, 0xdf8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_22 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xdff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK_22 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CHANNEL_ID_TO_SBPM_BB_ID_TABLE_22 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0xf68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_22 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xf78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_DISABLE_22 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_22 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, set_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0xf7d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_22 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_22 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_22 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_22 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xfa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_22 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, set_RDD_GDX_PARAMS, 0xfd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_22 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0xfe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER_22 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, set_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK_22 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_22 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, set_RDD_TCAM_TABLE_CFG, 0x2168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_22 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_22 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, set_RDD_INGRESS_FILTER_CFG, 0x2178 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_22 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_22 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2190 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_22 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x21a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_22 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, set_RDD_MULTICAST_KEY_MASK_ENTRY, 0x21c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK_22 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_22 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, set_RDD_RX_FLOW_ENTRY, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_22 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0x2500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PARSER_KEY_BUILD_TABLE_22 =
{
	36,
	{
		{ dump_RDD_PARSER_KEY_BUILD_ENTRY, set_RDD_PARSER_KEY_BUILD_ENTRY, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_22 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, set_RDD_INGRESS_FILTER_CTRL, 0x2700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_23 =
{
	2,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, set_RDD_POLICER_PARAMS_ENTRY, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_23 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_23 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_23 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, set_RDD_SPDTEST_GEN_CFG, 0x370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_23 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, set_RDD_VPORT_CFG_ENTRY, 0x380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_23 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_23 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_23 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_23 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_23 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, set_RDD_TUNNELS_PARSING_CFG, 0x7e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_23 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x7f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_23 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_23 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_BAD_IPV4_HDR_CSUM_PACKETS_23 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x96c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_23 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TASK_IDX_23 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x97c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_23 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DOS_DROP_REASONS_CFG_23 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x9be },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_CFG_EX_TABLE_23 =
{
	2,
	{
		{ dump_RDD_VPORT_CFG_EX_ENTRY, set_RDD_VPORT_CFG_EX_ENTRY, 0x9c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_23 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x9fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_23 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xa00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_23 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, set_RDD_IPTV_CFG, 0xb68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_23 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xb70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_RX_FLOWS_23 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_23 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_23 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_23 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb7f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TR471_SPDSVC_RX_PKT_ID_TABLE_23 =
{
	12,
	{
		{ dump_RDD_TR471_SPDSVC_RX_PKT_ID, set_RDD_TR471_SPDSVC_RX_PKT_ID, 0xb80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_23 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0xbb0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_23 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xbc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_23 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0xbe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK_23 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_23 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, set_RDD_TCAM_GENERIC, 0xd68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_23 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, set_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0xd78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_DESCRIPTOR_TABLE_23 =
{
	2,
	{
		{ dump_RDD_BUFMNG_DESCRIPTOR_ENTRY, set_RDD_BUFMNG_DESCRIPTOR_ENTRY, 0xd80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_STATUS_TABLE_23 =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, set_RDD_BUFMNG_STATUS_ENTRY, 0xda0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_23 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0xdc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_23 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xddf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_23 =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, set_RDD_DHD_HW_CONFIGURATION, 0xde0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_23 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0xdf4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_23 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, set_RDD_MIRRORING_DESCRIPTOR, 0xdf6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_23 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, set_RDD_NAT_CACHE_CFG, 0xdf8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_23 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xdff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK_23 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CHANNEL_ID_TO_SBPM_BB_ID_TABLE_23 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0xf68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_23 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xf78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_DISABLE_23 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_23 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, set_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0xf7d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_23 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_23 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_23 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_23 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xfa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GDX_PARAMS_TABLE_23 =
{
	12,
	{
		{ dump_RDD_GDX_PARAMS, set_RDD_GDX_PARAMS, 0xfd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_23 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0xfe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER_23 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, set_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK_23 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_23 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, set_RDD_TCAM_TABLE_CFG, 0x2168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_23 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x2170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_23 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, set_RDD_INGRESS_FILTER_CFG, 0x2178 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_23 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_23 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2190 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_23 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x21a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_23 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, set_RDD_MULTICAST_KEY_MASK_ENTRY, 0x21c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK_23 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_23 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, set_RDD_RX_FLOW_ENTRY, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_23 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, set_RDD_TX_FLOW_ENTRY, 0x2500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PARSER_KEY_BUILD_TABLE_23 =
{
	36,
	{
		{ dump_RDD_PARSER_KEY_BUILD_ENTRY, set_RDD_PARSER_KEY_BUILD_ENTRY, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_23 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, set_RDD_INGRESS_FILTER_CTRL, 0x2700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DS_TM_TM_FLOW_CNTR_TABLE_24 =
{
	1,
	{
		{ dump_RDD_TM_FLOW_CNTR_ENTRY, set_RDD_TM_FLOW_CNTR_ENTRY, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU0_RL_UPDATE_RESPONSE_24 =
{
	8,
	{
		{ dump_RDD_GSU_UPDATE_RL_STATUS_BB_MSG, set_RDD_GSU_UPDATE_RL_STATUS_BB_MSG, 0x40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU1_RL_UPDATE_RESPONSE_24 =
{
	8,
	{
		{ dump_RDD_GSU_UPDATE_RL_STATUS_BB_MSG, set_RDD_GSU_UPDATE_RL_STATUS_BB_MSG, 0x48 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU2_RL_UPDATE_RESPONSE_24 =
{
	8,
	{
		{ dump_RDD_GSU_UPDATE_RL_STATUS_BB_MSG, set_RDD_GSU_UPDATE_RL_STATUS_BB_MSG, 0x50 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU3_RL_UPDATE_RESPONSE_24 =
{
	8,
	{
		{ dump_RDD_GSU_UPDATE_RL_STATUS_BB_MSG, set_RDD_GSU_UPDATE_RL_STATUS_BB_MSG, 0x58 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU4_RL_UPDATE_RESPONSE_24 =
{
	8,
	{
		{ dump_RDD_GSU_UPDATE_RL_STATUS_BB_MSG, set_RDD_GSU_UPDATE_RL_STATUS_BB_MSG, 0x60 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU0_AQM_UPDATE_STATUS_RESPONSE_24 =
{
	8,
	{
		{ dump_RDD_GSU_UPDATE_AQM_STATUS_BB_MSG, set_RDD_GSU_UPDATE_AQM_STATUS_BB_MSG, 0x68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU1_AQM_UPDATE_STATUS_RESPONSE_24 =
{
	8,
	{
		{ dump_RDD_GSU_UPDATE_AQM_STATUS_BB_MSG, set_RDD_GSU_UPDATE_AQM_STATUS_BB_MSG, 0x70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU2_AQM_UPDATE_STATUS_RESPONSE_24 =
{
	8,
	{
		{ dump_RDD_GSU_UPDATE_AQM_STATUS_BB_MSG, set_RDD_GSU_UPDATE_AQM_STATUS_BB_MSG, 0x78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU3_AQM_UPDATE_STATUS_RESPONSE_24 =
{
	8,
	{
		{ dump_RDD_GSU_UPDATE_AQM_STATUS_BB_MSG, set_RDD_GSU_UPDATE_AQM_STATUS_BB_MSG, 0x80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU4_AQM_UPDATE_STATUS_RESPONSE_24 =
{
	8,
	{
		{ dump_RDD_GSU_UPDATE_AQM_STATUS_BB_MSG, set_RDD_GSU_UPDATE_AQM_STATUS_BB_MSG, 0x88 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT AQM_PROBABILITY_CALC_DESCRIPTOR_24 =
{
	16,
	{
		{ dump_RDD_AQM_PROBABILITY_CALC_DESCRIPTOR, set_RDD_AQM_PROBABILITY_CALC_DESCRIPTOR, 0x90 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_24 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_24 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0xc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RL_UPDATE_TASK_DESCRIPTOR_24 =
{
	24,
	{
		{ dump_RDD_RL_UPDATE_TASK_DESCRIPTOR, set_RDD_RL_UPDATE_TASK_DESCRIPTOR, 0xe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DS_TM_BBH_TX_WAKE_UP_DATA_TABLE_24 =
{
	8,
	{
		{ dump_RDD_BBH_TX_WAKE_UP_DATA_ENTRY, set_RDD_BBH_TX_WAKE_UP_DATA_ENTRY, 0xf8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT UPDATE_FIFO_GSU0_STACK_24 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x100 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT UPDATE_FIFO_GSU1_STACK_24 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT UPDATE_FIFO_GSU2_STACK_24 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT AQM_QUEUE_TABLE_24 =
{
	4,
	{
		{ dump_RDD_AQM_QUEUE_DESCRIPTOR, set_RDD_AQM_QUEUE_DESCRIPTOR, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_24 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x644 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_24 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x648 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_24 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x678 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU_SETTING_TABLE_24 =
{
	16,
	{
		{ dump_RDD_GSU_SETTING, set_RDD_GSU_SETTING, 0x680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_24 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x6d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_24 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x6dc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_24 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0x6e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TASK_IDX_24 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x6fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT UPDATE_FIFO_GSU3_STACK_24 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_24 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT UPDATE_FIFO_GSU4_STACK_24 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xa00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_GSU0_STACK_24 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xb00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_GSU1_STACK_24 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_GSU2_STACK_24 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xd00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_GSU3_STACK_24 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_GSU4_STACK_24 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT UPDATE_RL_STACK_24 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT UPDATE_AQM_STACK_24 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x1100 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU0_UPDATE_FIFO_TABLE_24 =
{
	16,
	{
		{ dump_RDD_QM_UPDATE_FIFO_BB_MSG, set_RDD_QM_UPDATE_FIFO_BB_MSG, 0x1200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU1_UPDATE_FIFO_TABLE_24 =
{
	16,
	{
		{ dump_RDD_QM_UPDATE_FIFO_BB_MSG, set_RDD_QM_UPDATE_FIFO_BB_MSG, 0x1280 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU2_UPDATE_FIFO_TABLE_24 =
{
	16,
	{
		{ dump_RDD_QM_UPDATE_FIFO_BB_MSG, set_RDD_QM_UPDATE_FIFO_BB_MSG, 0x1300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU3_UPDATE_FIFO_TABLE_24 =
{
	16,
	{
		{ dump_RDD_QM_UPDATE_FIFO_BB_MSG, set_RDD_QM_UPDATE_FIFO_BB_MSG, 0x1380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU4_UPDATE_FIFO_TABLE_24 =
{
	16,
	{
		{ dump_RDD_QM_UPDATE_FIFO_BB_MSG, set_RDD_QM_UPDATE_FIFO_BB_MSG, 0x1400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU0_TRANSMIT_PD_24 =
{
	8,
	{
		{ dump_RDD_BYTES_8, set_RDD_BYTES_8, 0x1480 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU1_TRANSMIT_PD_24 =
{
	8,
	{
		{ dump_RDD_BYTES_8, set_RDD_BYTES_8, 0x1500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU2_TRANSMIT_PD_24 =
{
	8,
	{
		{ dump_RDD_BYTES_8, set_RDD_BYTES_8, 0x1580 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU3_TRANSMIT_PD_24 =
{
	8,
	{
		{ dump_RDD_BYTES_8, set_RDD_BYTES_8, 0x1600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU4_TRANSMIT_PD_24 =
{
	8,
	{
		{ dump_RDD_BYTES_8, set_RDD_BYTES_8, 0x1680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_24 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x1700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DS_TM_CPU_TX_ABS_COUNTERS_24 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x1780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DS_TM_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_TABLE_24 =
{
	40,
	{
		{ dump_RDD_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_ENTRY, set_RDD_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_ENTRY, 0x1800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU0_TRANSMIT_TASK_DESCRIPTOR_24 =
{
	16,
	{
		{ dump_RDD_TRANSMIT_TASK_DESCRIPTOR, set_RDD_TRANSMIT_TASK_DESCRIPTOR, 0x1850 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU1_TRANSMIT_TASK_DESCRIPTOR_24 =
{
	16,
	{
		{ dump_RDD_TRANSMIT_TASK_DESCRIPTOR, set_RDD_TRANSMIT_TASK_DESCRIPTOR, 0x1860 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU2_TRANSMIT_TASK_DESCRIPTOR_24 =
{
	16,
	{
		{ dump_RDD_TRANSMIT_TASK_DESCRIPTOR, set_RDD_TRANSMIT_TASK_DESCRIPTOR, 0x1870 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_RL_OVERHEAD_TABLE_24 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0x1880 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_24 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x189f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU3_TRANSMIT_TASK_DESCRIPTOR_24 =
{
	16,
	{
		{ dump_RDD_TRANSMIT_TASK_DESCRIPTOR, set_RDD_TRANSMIT_TASK_DESCRIPTOR, 0x18a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU4_TRANSMIT_TASK_DESCRIPTOR_24 =
{
	16,
	{
		{ dump_RDD_TRANSMIT_TASK_DESCRIPTOR, set_RDD_TRANSMIT_TASK_DESCRIPTOR, 0x18b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_24 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x18c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU0_TRANSMIT_TASK_WAKE_UP_BB_MSG_24 =
{
	8,
	{
		{ dump_RDD_BBHTX_WAKE_UP_BB_MSG, set_RDD_BBHTX_WAKE_UP_BB_MSG, 0x18c8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_24 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0x18d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU1_TRANSMIT_TASK_WAKE_UP_BB_MSG_24 =
{
	8,
	{
		{ dump_RDD_BBHTX_WAKE_UP_BB_MSG, set_RDD_BBHTX_WAKE_UP_BB_MSG, 0x18e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU2_TRANSMIT_TASK_WAKE_UP_BB_MSG_24 =
{
	8,
	{
		{ dump_RDD_BBHTX_WAKE_UP_BB_MSG, set_RDD_BBHTX_WAKE_UP_BB_MSG, 0x18e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU3_TRANSMIT_TASK_WAKE_UP_BB_MSG_24 =
{
	8,
	{
		{ dump_RDD_BBHTX_WAKE_UP_BB_MSG, set_RDD_BBHTX_WAKE_UP_BB_MSG, 0x18f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU4_TRANSMIT_TASK_WAKE_UP_BB_MSG_24 =
{
	8,
	{
		{ dump_RDD_BBHTX_WAKE_UP_BB_MSG, set_RDD_BBHTX_WAKE_UP_BB_MSG, 0x18f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU0_TRANSMIT_TASK_STATUS_BB_MSG_24 =
{
	8,
	{
		{ dump_RDD_GSU_TX_STATUS_BB_MSG, set_RDD_GSU_TX_STATUS_BB_MSG, 0x1900 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU1_TRANSMIT_TASK_STATUS_BB_MSG_24 =
{
	8,
	{
		{ dump_RDD_GSU_TX_STATUS_BB_MSG, set_RDD_GSU_TX_STATUS_BB_MSG, 0x1908 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU2_TRANSMIT_TASK_STATUS_BB_MSG_24 =
{
	8,
	{
		{ dump_RDD_GSU_TX_STATUS_BB_MSG, set_RDD_GSU_TX_STATUS_BB_MSG, 0x1910 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU3_TRANSMIT_TASK_STATUS_BB_MSG_24 =
{
	8,
	{
		{ dump_RDD_GSU_TX_STATUS_BB_MSG, set_RDD_GSU_TX_STATUS_BB_MSG, 0x1918 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU4_TRANSMIT_TASK_STATUS_BB_MSG_24 =
{
	8,
	{
		{ dump_RDD_GSU_TX_STATUS_BB_MSG, set_RDD_GSU_TX_STATUS_BB_MSG, 0x1920 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU0_UPDATE_FIFO_TASK_DESCRIPTOR_24 =
{
	8,
	{
		{ dump_RDD_UPDATE_FIFO_TASK_DESCRIPTOR, set_RDD_UPDATE_FIFO_TASK_DESCRIPTOR, 0x1928 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU1_UPDATE_FIFO_TASK_DESCRIPTOR_24 =
{
	8,
	{
		{ dump_RDD_UPDATE_FIFO_TASK_DESCRIPTOR, set_RDD_UPDATE_FIFO_TASK_DESCRIPTOR, 0x1930 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU2_UPDATE_FIFO_TASK_DESCRIPTOR_24 =
{
	8,
	{
		{ dump_RDD_UPDATE_FIFO_TASK_DESCRIPTOR, set_RDD_UPDATE_FIFO_TASK_DESCRIPTOR, 0x1938 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU3_UPDATE_FIFO_TASK_DESCRIPTOR_24 =
{
	8,
	{
		{ dump_RDD_UPDATE_FIFO_TASK_DESCRIPTOR, set_RDD_UPDATE_FIFO_TASK_DESCRIPTOR, 0x1940 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU4_UPDATE_FIFO_TASK_DESCRIPTOR_24 =
{
	8,
	{
		{ dump_RDD_UPDATE_FIFO_TASK_DESCRIPTOR, set_RDD_UPDATE_FIFO_TASK_DESCRIPTOR, 0x1948 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DS_TM_BB_DESTINATION_TABLE_24 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x1950 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DS_TM_FIRST_QUEUE_MAPPING_24 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x1952 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_24 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x1954 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_24 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x1955 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BACKUP_BBH_INGRESS_COUNTERS_TABLE_24 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x1956 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BACKUP_BBH_EGRESS_COUNTERS_TABLE_24 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x1957 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_24 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x1960 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT US_TM_WAN_0_BBH_TX_EGRESS_COUNTER_TABLE_25 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_25 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x28 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT AQM_PROBABILITY_CALC_DESCRIPTOR_25 =
{
	16,
	{
		{ dump_RDD_AQM_PROBABILITY_CALC_DESCRIPTOR, set_RDD_AQM_PROBABILITY_CALC_DESCRIPTOR, 0x30 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU0_RL_UPDATE_RESPONSE_25 =
{
	8,
	{
		{ dump_RDD_GSU_UPDATE_RL_STATUS_BB_MSG, set_RDD_GSU_UPDATE_RL_STATUS_BB_MSG, 0x40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT US_TM_WAN_0_BBH_TX_WAKE_UP_DATA_TABLE_25 =
{
	8,
	{
		{ dump_RDD_BBH_TX_WAKE_UP_DATA_ENTRY, set_RDD_BBH_TX_WAKE_UP_DATA_ENTRY, 0x48 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DISPATCHER_CREDIT_TABLE_25 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x50 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_25 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x5c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU0_TRANSMIT_TASK_WAKE_UP_BB_MSG_25 =
{
	8,
	{
		{ dump_RDD_BBHTX_WAKE_UP_BB_MSG, set_RDD_BBHTX_WAKE_UP_BB_MSG, 0x60 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU0_AQM_UPDATE_STATUS_RESPONSE_25 =
{
	8,
	{
		{ dump_RDD_GSU_UPDATE_AQM_STATUS_BB_MSG, set_RDD_GSU_UPDATE_AQM_STATUS_BB_MSG, 0x68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_25 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_25 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT US_TM_TM_FLOW_CNTR_TABLE_25 =
{
	1,
	{
		{ dump_RDD_TM_FLOW_CNTR_ENTRY, set_RDD_TM_FLOW_CNTR_ENTRY, 0x80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT UPDATE_FIFO_GSU0_STACK_25 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x100 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_GSU0_STACK_25 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT UPDATE_RL_STACK_25 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT AQM_QUEUE_TABLE_25 =
{
	4,
	{
		{ dump_RDD_AQM_QUEUE_DESCRIPTOR, set_RDD_AQM_QUEUE_DESCRIPTOR, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TASK_IDX_25 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x644 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU0_TRANSMIT_TASK_STATUS_BB_MSG_25 =
{
	8,
	{
		{ dump_RDD_GSU_TX_STATUS_BB_MSG, set_RDD_GSU_TX_STATUS_BB_MSG, 0x648 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU0_TRANSMIT_TASK_DESCRIPTOR_25 =
{
	16,
	{
		{ dump_RDD_TRANSMIT_TASK_DESCRIPTOR, set_RDD_TRANSMIT_TASK_DESCRIPTOR, 0x650 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_25 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x660 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU_SETTING_TABLE_25 =
{
	16,
	{
		{ dump_RDD_GSU_SETTING, set_RDD_GSU_SETTING, 0x680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_25 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0x6d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_25 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0x6e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT UPDATE_AQM_STACK_25 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_25 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU0_UPDATE_FIFO_TABLE_25 =
{
	16,
	{
		{ dump_RDD_QM_UPDATE_FIFO_BB_MSG, set_RDD_QM_UPDATE_FIFO_BB_MSG, 0xa00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU0_TRANSMIT_PD_25 =
{
	8,
	{
		{ dump_RDD_BYTES_8, set_RDD_BYTES_8, 0xa80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_25 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xb00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT US_TM_CPU_TX_ABS_COUNTERS_25 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xb80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT US_TM_TX_OCTETS_COUNTERS_TABLE_25 =
{
	8,
	{
		{ dump_RDD_PACKETS_AND_BYTES, set_RDD_PACKETS_AND_BYTES, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RL_UPDATE_TASK_DESCRIPTOR_25 =
{
	24,
	{
		{ dump_RDD_RL_UPDATE_TASK_DESCRIPTOR, set_RDD_RL_UPDATE_TASK_DESCRIPTOR, 0xc40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_25 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xc58 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU0_UPDATE_FIFO_TASK_DESCRIPTOR_25 =
{
	8,
	{
		{ dump_RDD_UPDATE_FIFO_TASK_DESCRIPTOR, set_RDD_UPDATE_FIFO_TASK_DESCRIPTOR, 0xc88 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT US_TM_BBH_TX_WAN_0_FIFO_BYTES_USED_25 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xc90 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT US_TM_WAN_0_BB_DESTINATION_TABLE_25 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0xc94 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT QEMU_SYNC_MEM_25 =
{
	1,
	{
		{ dump_RDD_QEMU_DATA, set_RDD_QEMU_DATA, 0xc96 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_25 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xc97 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PON_TM_VPORT_TO_RL_OVERHEAD_TABLE_25 =
{
	1,
	{
		{ dump_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, set_RDD_VPORT_TO_RL_OVERHEAD_ENTRY, 0xc98 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_25 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xc99 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT US_TM_FIRST_QUEUE_MAPPING_25 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0xc9a },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BBH_TX_US_WAN_0_FIFO_BYTES_THRESHOLD_25 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0xc9c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_25 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xc9e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT WAN_0_BBH_TX_FIFO_SIZE_25 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xc9f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_25 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0xca0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT US_TM_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_TABLE_25 =
{
	40,
	{
		{ dump_RDD_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_ENTRY, set_RDD_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_ENTRY, 0xcc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_25 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xd00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PKTGEN_SBPM_HDR_BNS_26 =
{
	2,
	{
		{ dump_RDD_PKTGEN_SBPM_HDR_BN, set_RDD_PKTGEN_SBPM_HDR_BN, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU0_RL_UPDATE_RESPONSE_26 =
{
	8,
	{
		{ dump_RDD_GSU_UPDATE_RL_STATUS_BB_MSG, set_RDD_GSU_UPDATE_RL_STATUS_BB_MSG, 0x40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU1_RL_UPDATE_RESPONSE_26 =
{
	8,
	{
		{ dump_RDD_GSU_UPDATE_RL_STATUS_BB_MSG, set_RDD_GSU_UPDATE_RL_STATUS_BB_MSG, 0x48 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU2_RL_UPDATE_RESPONSE_26 =
{
	8,
	{
		{ dump_RDD_GSU_UPDATE_RL_STATUS_BB_MSG, set_RDD_GSU_UPDATE_RL_STATUS_BB_MSG, 0x50 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PKTGEN_SBPM_EXTS_26 =
{
	4,
	{
		{ dump_RDD_PKTGEN_SBPM_EXT, set_RDD_PKTGEN_SBPM_EXT, 0x58 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU0_AQM_UPDATE_STATUS_RESPONSE_26 =
{
	8,
	{
		{ dump_RDD_GSU_UPDATE_AQM_STATUS_BB_MSG, set_RDD_GSU_UPDATE_AQM_STATUS_BB_MSG, 0x68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU1_AQM_UPDATE_STATUS_RESPONSE_26 =
{
	8,
	{
		{ dump_RDD_GSU_UPDATE_AQM_STATUS_BB_MSG, set_RDD_GSU_UPDATE_AQM_STATUS_BB_MSG, 0x70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU2_AQM_UPDATE_STATUS_RESPONSE_26 =
{
	8,
	{
		{ dump_RDD_GSU_UPDATE_AQM_STATUS_BB_MSG, set_RDD_GSU_UPDATE_AQM_STATUS_BB_MSG, 0x78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CPU_RX_CSO_STACK_26 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT UPDATE_FIFO_GSU0_STACK_26 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x100 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT UPDATE_FIFO_GSU1_STACK_26 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT UPDATE_FIFO_GSU2_STACK_26 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CPU_RX_SCRATCHPAD_26 =
{
	8,
	{
		{ dump_RDD_BYTES_8, set_RDD_BYTES_8, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_FLOW_RING_CACHE_CTX_TABLE_26 =
{
	16,
	{
		{ dump_RDD_DHD_FLOW_RING_CACHE_CTX_ENTRY, set_RDD_DHD_FLOW_RING_CACHE_CTX_ENTRY, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_AUX_INFO_CACHE_TABLE_26 =
{
	16,
	{
		{ dump_RDD_DHD_AUX_INFO_CACHE_ENTRY, set_RDD_DHD_AUX_INFO_CACHE_ENTRY, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_POST_COMMON_RADIO_DATA_26 =
{
	200,
	{
		{ dump_RDD_DHD_POST_COMMON_RADIO_ENTRY, set_RDD_DHD_POST_COMMON_RADIO_ENTRY, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_26 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x1320 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CPU_REASON_TO_METER_TABLE_26 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x1340 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_MONITOR_BQ_INFO_TABLE_26 =
{
	32,
	{
		{ dump_RDD_DHD_MONITOR_BQ_INFO, set_RDD_DHD_MONITOR_BQ_INFO, 0x1380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT AQM_QUEUE_TABLE_26 =
{
	4,
	{
		{ dump_RDD_AQM_QUEUE_DESCRIPTOR, set_RDD_AQM_QUEUE_DESCRIPTOR, 0x1400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PKTGEN_NO_SBPM_HDRS_CNTR_26 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x1644 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_26 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x1648 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT AQM_PROBABILITY_CALC_DESCRIPTOR_26 =
{
	16,
	{
		{ dump_RDD_AQM_PROBABILITY_CALC_DESCRIPTOR, set_RDD_AQM_PROBABILITY_CALC_DESCRIPTOR, 0x1650 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_26 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0x1660 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_BACKUP_INDEX_CACHE_26 =
{
	32,
	{
		{ dump_RDD_DHD_BACKUP_IDX_CACHE_TABLE, set_RDD_DHD_BACKUP_IDX_CACHE_TABLE, 0x1680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_GSU0_STACK_26 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x1700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PKTGEN_TX_STREAM_TABLE_26 =
{
	136,
	{
		{ dump_RDD_PKTGEN_TX_STREAM_ENTRY, set_RDD_PKTGEN_TX_STREAM_ENTRY, 0x1800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_TIMER_STACK_26 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x1a20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPV6_HOST_ADDRESS_CRC_TABLE_26 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x1a40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CPU_RXQ_SCRATCH_TABLE_26 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x1a80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_GSU1_STACK_26 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x1b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_26 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0x1c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_GSU2_STACK_26 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x1e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT UPDATE_RL_STACK_26 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x1f00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT UPDATE_AQM_STACK_26 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_DOORBELL_TX_POST_VALUE_26 =
{
	4,
	{
		{ dump_RDD_DHD_DOORBELL, set_RDD_DHD_DOORBELL, 0x2100 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CPU_RX_METER_TABLE_26 =
{
	8,
	{
		{ dump_RDD_CPU_RX_METER_ENTRY, set_RDD_CPU_RX_METER_ENTRY, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_FLOW_RING_BUFFER_26 =
{
	48,
	{
		{ dump_RDD_DHD_TX_POST_DESCRIPTOR, set_RDD_DHD_TX_POST_DESCRIPTOR, 0x2300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PKTGEN_SESSION_DATA_26 =
{
	32,
	{
		{ dump_RDD_PKTGEN_TX_PARAMS, set_RDD_PKTGEN_TX_PARAMS, 0x23c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_UPDATE_FIFO_STACK_26 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x23e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CPU_RX_XPM_PD_FIFO_TABLE_26 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, set_RDD_PROCESSING_TX_DESCRIPTOR, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_UPDATE_FIFO_TABLE_26 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, set_RDD_UPDATE_FIFO_ENTRY, 0x24c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT IPV4_HOST_ADDRESS_TABLE_26 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x24e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CPU_RING_INTERRUPT_COUNTER_TABLE_26 =
{
	8,
	{
		{ dump_RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY, set_RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY, 0x2500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CPU_RX_CSO_UPDATE_FIFO_TABLE_26 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, set_RDD_UPDATE_FIFO_ENTRY, 0x25c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RL_UPDATE_TASK_DESCRIPTOR_26 =
{
	24,
	{
		{ dump_RDD_RL_UPDATE_TASK_DESCRIPTOR, set_RDD_RL_UPDATE_TASK_DESCRIPTOR, 0x25e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MIRRORING_SCRATCH_26 =
{
	8,
	{
		{ dump_RDD_BYTES_8, set_RDD_BYTES_8, 0x25f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_CONTEXT_TABLE_26 =
{
	176,
	{
		{ dump_RDD_CSO_CONTEXT_ENTRY, set_RDD_CSO_CONTEXT_ENTRY, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_CPU_INT_ID_26 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x26b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CPU_RX_XPM_UPDATE_FIFO_TABLE_26 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, set_RDD_UPDATE_FIFO_ENTRY, 0x26c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_26 =
{
	20,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, set_RDD_DHD_HW_CONFIGURATION, 0x26e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_26 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x26f4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_26 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, set_RDD_DDR_ADDRESS, 0x26f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_0_STACK_26 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CPU_REASON_TO_TC_26 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x27a8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CPU_RX_INTERRUPT_ID_DDR_ADDR_26 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, set_RDD_DDR_ADDRESS, 0x27e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_MIRRORING_DISPATCHER_CREDIT_TABLE_26 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x27f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_1_STACK_26 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TC_TO_CPU_RXQ_26 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x28a8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU0_TRANSMIT_TASK_WAKE_UP_BB_MSG_26 =
{
	8,
	{
		{ dump_RDD_BBHTX_WAKE_UP_BB_MSG, set_RDD_BBHTX_WAKE_UP_BB_MSG, 0x28e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CPU_RX_AUX_INT_SCRATCHPAD_26 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x28f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_2_STACK_26 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2900 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT EXC_TC_TO_CPU_RXQ_26 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x29a8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU1_TRANSMIT_TASK_WAKE_UP_BB_MSG_26 =
{
	8,
	{
		{ dump_RDD_BBHTX_WAKE_UP_BB_MSG, set_RDD_BBHTX_WAKE_UP_BB_MSG, 0x29e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CPU_RX_CSO_DISPATCHER_CREDIT_TABLE_26 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x29f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_26 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x29fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CPU_RX_STACK_26 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_26 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2a88 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU2_TRANSMIT_TASK_WAKE_UP_BB_MSG_26 =
{
	8,
	{
		{ dump_RDD_BBHTX_WAKE_UP_BB_MSG, set_RDD_BBHTX_WAKE_UP_BB_MSG, 0x2ab8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CPU_RXQ_DATA_BUF_TYPE_TABLE_26 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2ac0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_26 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0x2ae0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PKTGEN_BAD_GET_NEXT_26 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2afc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_FLOW_RING_CACHE_LKP_TABLE_26 =
{
	2,
	{
		{ dump_RDD_DHD_FLOW_RING_CACHE_LKP_ENTRY, set_RDD_DHD_FLOW_RING_CACHE_LKP_ENTRY, 0x2b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_L2_HEADER_26 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2b80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_26 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x2be0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PKTGEN_MAX_UT_PKTS_26 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2bec },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU0_TRANSMIT_TASK_DESCRIPTOR_26 =
{
	16,
	{
		{ dump_RDD_TRANSMIT_TASK_DESCRIPTOR, set_RDD_TRANSMIT_TASK_DESCRIPTOR, 0x2bf0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CPU_RX_CSO_PD_FIFO_TABLE_26 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, set_RDD_PROCESSING_TX_DESCRIPTOR, 0x2c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CPU_REASON_AND_VPORT_TO_METER_TABLE_26 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x2c80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU1_TRANSMIT_TASK_DESCRIPTOR_26 =
{
	16,
	{
		{ dump_RDD_TRANSMIT_TASK_DESCRIPTOR, set_RDD_TRANSMIT_TASK_DESCRIPTOR, 0x2ce0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU2_TRANSMIT_TASK_DESCRIPTOR_26 =
{
	16,
	{
		{ dump_RDD_TRANSMIT_TASK_DESCRIPTOR, set_RDD_TRANSMIT_TASK_DESCRIPTOR, 0x2cf0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU_SETTING_TABLE_26 =
{
	16,
	{
		{ dump_RDD_GSU_SETTING, set_RDD_GSU_SETTING, 0x2d00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PKTGEN_BBMSG_REPLY_SCRATCH_26 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x2d50 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU0_TRANSMIT_TASK_STATUS_BB_MSG_26 =
{
	8,
	{
		{ dump_RDD_GSU_TX_STATUS_BB_MSG, set_RDD_GSU_TX_STATUS_BB_MSG, 0x2d58 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_26 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0x2d60 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CPU_INTERRUPT_COALESCING_TABLE_26 =
{
	16,
	{
		{ dump_RDD_CPU_INTERRUPT_COALESCING_ENTRY, set_RDD_CPU_INTERRUPT_COALESCING_ENTRY, 0x2d70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU0_UPDATE_FIFO_TABLE_26 =
{
	16,
	{
		{ dump_RDD_QM_UPDATE_FIFO_BB_MSG, set_RDD_QM_UPDATE_FIFO_BB_MSG, 0x2d80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU1_UPDATE_FIFO_TABLE_26 =
{
	16,
	{
		{ dump_RDD_QM_UPDATE_FIFO_BB_MSG, set_RDD_QM_UPDATE_FIFO_BB_MSG, 0x2e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU2_UPDATE_FIFO_TABLE_26 =
{
	16,
	{
		{ dump_RDD_QM_UPDATE_FIFO_BB_MSG, set_RDD_QM_UPDATE_FIFO_BB_MSG, 0x2e80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU0_TRANSMIT_PD_26 =
{
	8,
	{
		{ dump_RDD_BYTES_8, set_RDD_BYTES_8, 0x2f00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU1_TRANSMIT_PD_26 =
{
	8,
	{
		{ dump_RDD_BYTES_8, set_RDD_BYTES_8, 0x2f80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CPU_RING_DESCRIPTORS_TABLE_26 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, set_RDD_CPU_RING_DESCRIPTOR, 0x3000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU2_TRANSMIT_PD_26 =
{
	8,
	{
		{ dump_RDD_BYTES_8, set_RDD_BYTES_8, 0x3180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_26 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x3200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_CPU_TX_POST_RING_DESCRIPTOR_TABLE_26 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, set_RDD_CPU_RING_DESCRIPTOR, 0x3280 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PKTGEN_FPM_UG_MGMT_26 =
{
	20,
	{
		{ dump_RDD_PKTGEN_FPM_UG_MGMT_ENTRY, set_RDD_PKTGEN_FPM_UG_MGMT_ENTRY, 0x32c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PKTGEN_UT_TRIGGER_26 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x32d4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CPU_RX_LOCAL_SCRATCH_26 =
{
	8,
	{
		{ dump_RDD_BYTES_8, set_RDD_BYTES_8, 0x32d8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU1_TRANSMIT_TASK_STATUS_BB_MSG_26 =
{
	8,
	{
		{ dump_RDD_GSU_TX_STATUS_BB_MSG, set_RDD_GSU_TX_STATUS_BB_MSG, 0x32e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU2_TRANSMIT_TASK_STATUS_BB_MSG_26 =
{
	8,
	{
		{ dump_RDD_GSU_TX_STATUS_BB_MSG, set_RDD_GSU_TX_STATUS_BB_MSG, 0x32f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU0_UPDATE_FIFO_TASK_DESCRIPTOR_26 =
{
	8,
	{
		{ dump_RDD_UPDATE_FIFO_TASK_DESCRIPTOR, set_RDD_UPDATE_FIFO_TASK_DESCRIPTOR, 0x32f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE_26 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, set_RDD_CPU_RING_DESCRIPTOR, 0x3300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT VPORT_TO_CPU_OBJ_26 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3340 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_26 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x335f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU1_UPDATE_FIFO_TASK_DESCRIPTOR_26 =
{
	8,
	{
		{ dump_RDD_UPDATE_FIFO_TASK_DESCRIPTOR, set_RDD_UPDATE_FIFO_TASK_DESCRIPTOR, 0x3360 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU2_UPDATE_FIFO_TASK_DESCRIPTOR_26 =
{
	8,
	{
		{ dump_RDD_UPDATE_FIFO_TASK_DESCRIPTOR, set_RDD_UPDATE_FIFO_TASK_DESCRIPTOR, 0x3368 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PKTGEN_CURR_SBPM_HDR_PTR_26 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x3370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PKTGEN_NUM_OF_AVAIL_SBPM_HDRS_26 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x3378 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CPU_VPORT_TO_METER_TABLE_26 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x3380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_26 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x339f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PKTGEN_SBPM_END_PTR_26 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x33a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_MIRRORING_MINIFPM_PARAMS_26 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x33a8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RNR_WR_TEST_ADDR_26 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, set_RDD_DDR_ADDRESS, 0x33b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TASK_IDX_26 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x33b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CPU_RX_INTERRUPT_SCRATCH_26 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x33bc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_26 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x33c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RNR_WR_TEST_FROM_26 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x33c4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RNR_WR_TEST_TO_26 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x33c8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SPDSVC_WLAN_TXPOST_PARAMS_TABLE_26 =
{
	2,
	{
		{ dump_RDD_SPDSVC_WLAN_TXPOST_PARAMS, set_RDD_SPDSVC_WLAN_TXPOST_PARAMS, 0x33cc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_26 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x33ce },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CSO_DISABLE_26 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x33cf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CPU_RX_TELEMETRY_BUFFER_LOCATION_26 =
{
	2,
	{
		{ dump_RDD_BYTES_2, set_RDD_BYTES_2, 0x33d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RNR_WR_TEST_START_26 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x33d2 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT PKTGEN_TX_STREAM_SCRATCH_TABLE_26 =
{
	8,
	{
		{ dump_RDD_PKTGEN_TX_STREAM_SCRATCH_ENTRY, set_RDD_PKTGEN_TX_STREAM_SCRATCH_ENTRY, 0x3400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SQ_TM_UPDATE_FIFO_TABLE_27 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, set_RDD_UPDATE_FIFO_ENTRY, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_27 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU0_RL_UPDATE_RESPONSE_27 =
{
	8,
	{
		{ dump_RDD_GSU_UPDATE_RL_STATUS_BB_MSG, set_RDD_GSU_UPDATE_RL_STATUS_BB_MSG, 0x40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_27 =
{
	8,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, set_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x48 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT AQM_PROBABILITY_CALC_DESCRIPTOR_27 =
{
	16,
	{
		{ dump_RDD_AQM_PROBABILITY_CALC_DESCRIPTOR, set_RDD_AQM_PROBABILITY_CALC_DESCRIPTOR, 0x50 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU0_TRANSMIT_TASK_WAKE_UP_BB_MSG_27 =
{
	8,
	{
		{ dump_RDD_BBHTX_WAKE_UP_BB_MSG, set_RDD_BBHTX_WAKE_UP_BB_MSG, 0x60 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU0_AQM_UPDATE_STATUS_RESPONSE_27 =
{
	8,
	{
		{ dump_RDD_GSU_UPDATE_AQM_STATUS_BB_MSG, set_RDD_GSU_UPDATE_AQM_STATUS_BB_MSG, 0x68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_27 =
{
	12,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, set_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_27 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU_SETTING_TABLE_27 =
{
	16,
	{
		{ dump_RDD_GSU_SETTING, set_RDD_GSU_SETTING, 0x80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_27 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT BUFMNG_HOST_CNT_DISABLE_TABLE_27 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xdc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_27 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, set_RDD_FPM_POOL_NUMBER, 0xe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT UPDATE_FIFO_GSU0_STACK_27 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x100 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TX_GSU0_STACK_27 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT UPDATE_RL_STACK_27 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT AQM_QUEUE_TABLE_27 =
{
	4,
	{
		{ dump_RDD_AQM_QUEUE_DESCRIPTOR, set_RDD_AQM_QUEUE_DESCRIPTOR, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT TASK_IDX_27 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0x644 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU0_TRANSMIT_TASK_STATUS_BB_MSG_27 =
{
	8,
	{
		{ dump_RDD_GSU_TX_STATUS_BB_MSG, set_RDD_GSU_TX_STATUS_BB_MSG, 0x648 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU0_TRANSMIT_TASK_DESCRIPTOR_27 =
{
	16,
	{
		{ dump_RDD_TRANSMIT_TASK_DESCRIPTOR, set_RDD_TRANSMIT_TASK_DESCRIPTOR, 0x650 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SQ_TM_BUFMNG_STATUS_TABLE_27 =
{
	32,
	{
		{ dump_RDD_BUFMNG_STATUS_ENTRY, set_RDD_BUFMNG_STATUS_ENTRY, 0x660 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU0_UPDATE_FIFO_TABLE_27 =
{
	16,
	{
		{ dump_RDD_QM_UPDATE_FIFO_BB_MSG, set_RDD_QM_UPDATE_FIFO_BB_MSG, 0x680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT UPDATE_AQM_STACK_27 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0x700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SQ_TM_PD_FIFO_TABLE_27 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, set_RDD_PROCESSING_TX_DESCRIPTOR, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_27 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, set_RDD_TRACE_EVENT, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU0_TRANSMIT_PD_27 =
{
	8,
	{
		{ dump_RDD_BYTES_8, set_RDD_BYTES_8, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_27 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xe80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT RL_UPDATE_TASK_DESCRIPTOR_27 =
{
	24,
	{
		{ dump_RDD_RL_UPDATE_TASK_DESCRIPTOR, set_RDD_RL_UPDATE_TASK_DESCRIPTOR, 0xf00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_27 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xf18 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT GSU0_UPDATE_FIFO_TASK_DESCRIPTOR_27 =
{
	8,
	{
		{ dump_RDD_UPDATE_FIFO_TASK_DESCRIPTOR, set_RDD_UPDATE_FIFO_TASK_DESCRIPTOR, 0xf48 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_27 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, set_RDD_DEBUG_PRINT_INFO, 0xf50 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_27 =
{
	28,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, set_RDD_FPM_GLOBAL_CFG, 0xf60 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_27 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_27 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_27 =
{
	1,
	{
		{ dump_RDD_BYTE_1, set_RDD_BYTE_1, 0xf7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6885
static DUMP_RUNNERREG_STRUCT FW_ERROR_VECTOR_TABLE_27 =
{
	4,
	{
		{ dump_RDD_BYTES_4, set_RDD_BYTES_4, 0xf80 },
		{ 0, 0 },
	}
};
#endif

TABLE_STRUCT RUNNER_TABLES[NUMBER_OF_TABLES] =
{
#if defined BCM6885
	{ "POLICER_PARAMS_TABLE", 1, CORE_0_INDEX, &POLICER_PARAMS_TABLE, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING0_STACK", 1, CORE_0_INDEX, &PROCESSING0_STACK, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "SYSTEM_CONFIGURATION", 1, CORE_0_INDEX, &SYSTEM_CONFIGURATION, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDTEST_GEN_PARAM", 1, CORE_0_INDEX, &SPDTEST_GEN_PARAM, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_CFG_TABLE", 1, CORE_0_INDEX, &VPORT_CFG_TABLE, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "MUTEX_TABLE", 1, CORE_0_INDEX, &MUTEX_TABLE, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_0_INDEX, &RUNNER_PROFILING_TRACE_BUFFER, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING1_STACK", 1, CORE_0_INDEX, &PROCESSING1_STACK, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "REGISTERS_BUFFER", 1, CORE_0_INDEX, &REGISTERS_BUFFER, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "TUNNELS_PARSING_CFG", 1, CORE_0_INDEX, &TUNNELS_PARSING_CFG, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_0_INDEX, &INGRESS_FILTER_L2_REASON_TABLE, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING2_STACK", 1, CORE_0_INDEX, &PROCESSING2_STACK, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_0_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_BAD_IPV4_HDR_CSUM_PACKETS", 1, CORE_0_INDEX, &CSO_BAD_IPV4_HDR_CSUM_PACKETS, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_0_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TASK_IDX", 1, CORE_0_INDEX, &TASK_IDX, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_0_INDEX, &LOOPBACK_QUEUE_TABLE, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DOS_DROP_REASONS_CFG", 1, CORE_0_INDEX, &DOS_DROP_REASONS_CFG, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_CFG_EX_TABLE", 1, CORE_0_INDEX, &VPORT_CFG_EX_TABLE, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_0_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING3_STACK", 1, CORE_0_INDEX, &PROCESSING3_STACK, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_CFG_TABLE", 1, CORE_0_INDEX, &IPTV_CFG_TABLE, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_FPM_THRESHOLDS", 1, CORE_0_INDEX, &DHD_FPM_THRESHOLDS, 3, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_0_INDEX, &SPDTEST_NUM_OF_RX_FLOWS, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_0_INDEX, &LOOPBACK_WAN_FLOW_TABLE, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FORCE_DSCP", 1, CORE_0_INDEX, &FORCE_DSCP, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CORE_ID_TABLE", 1, CORE_0_INDEX, &CORE_ID_TABLE, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TR471_SPDSVC_RX_PKT_ID_TABLE", 1, CORE_0_INDEX, &TR471_SPDSVC_RX_PKT_ID_TABLE, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_TABLE", 1, CORE_0_INDEX, &DEBUG_PRINT_TABLE, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_0_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_0_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING4_STACK", 1, CORE_0_INDEX, &PROCESSING4_STACK, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "TCAM_GENERIC_FIELDS", 1, CORE_0_INDEX, &TCAM_GENERIC_FIELDS, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_0_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_0_INDEX, &BUFMNG_DESCRIPTOR_TABLE, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_STATUS_TABLE", 1, CORE_0_INDEX, &BUFMNG_STATUS_TABLE, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_0_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_0_INDEX, &DEBUG_PRINT_CORE_LOCK, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_HW_CFG", 1, CORE_0_INDEX, &DHD_HW_CFG, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_0_INDEX, &RX_MIRRORING_CONFIGURATION, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_0_INDEX, &TX_MIRRORING_CONFIGURATION, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "NAT_CACHE_CFG", 1, CORE_0_INDEX, &NAT_CACHE_CFG, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SRAM_DUMMY_STORE", 1, CORE_0_INDEX, &SRAM_DUMMY_STORE, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING5_STACK", 1, CORE_0_INDEX, &PROCESSING5_STACK, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "CHANNEL_ID_TO_SBPM_BB_ID_TABLE", 1, CORE_0_INDEX, &CHANNEL_ID_TO_SBPM_BB_ID_TABLE, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_0_INDEX, &NAT_CACHE_KEY0_MASK, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_DISABLE", 1, CORE_0_INDEX, &CSO_DISABLE, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_0_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_0_INDEX, &INGRESS_FILTER_1588_CFG, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_0_INDEX, &RX_MIRRORING_DIRECT_ENABLE, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_0_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_SCRATCHPAD", 1, CORE_0_INDEX, &DEBUG_SCRATCHPAD, 12, 1, 1 },
#endif
#if defined BCM6885
	{ "GDX_PARAMS_TABLE", 1, CORE_0_INDEX, &GDX_PARAMS_TABLE, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_GLOBAL_CFG", 1, CORE_0_INDEX, &FPM_GLOBAL_CFG, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_0_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING6_STACK", 1, CORE_0_INDEX, &PROCESSING6_STACK, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_0_INDEX, &TCAM_TABLE_CFG_TABLE, 2, 1, 1 },
#endif
#if defined BCM6885
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_0_INDEX, &NATC_L2_VLAN_KEY_MASK, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_CFG", 1, CORE_0_INDEX, &INGRESS_FILTER_CFG, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_0_INDEX, &BITS_CALC_MASKS_TABLE, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "NATC_L2_TOS_MASK", 1, CORE_0_INDEX, &NATC_L2_TOS_MASK, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_0_INDEX, &FW_ERROR_VECTOR_TABLE, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "MULTICAST_KEY_MASK", 1, CORE_0_INDEX, &MULTICAST_KEY_MASK, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING7_STACK", 1, CORE_0_INDEX, &PROCESSING7_STACK, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_FLOW_TABLE", 1, CORE_0_INDEX, &RX_FLOW_TABLE, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_FLOW_TABLE", 1, CORE_0_INDEX, &TX_FLOW_TABLE, 212, 1, 1 },
#endif
#if defined BCM6885
	{ "PARSER_KEY_BUILD_TABLE", 1, CORE_0_INDEX, &PARSER_KEY_BUILD_TABLE, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_0_INDEX, &INGRESS_FILTER_PROFILE_TABLE, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "POLICER_PARAMS_TABLE", 1, CORE_1_INDEX, &POLICER_PARAMS_TABLE_1, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING0_STACK", 1, CORE_1_INDEX, &PROCESSING0_STACK_1, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "SYSTEM_CONFIGURATION", 1, CORE_1_INDEX, &SYSTEM_CONFIGURATION_1, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDTEST_GEN_PARAM", 1, CORE_1_INDEX, &SPDTEST_GEN_PARAM_1, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_CFG_TABLE", 1, CORE_1_INDEX, &VPORT_CFG_TABLE_1, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "MUTEX_TABLE", 1, CORE_1_INDEX, &MUTEX_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_1_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_1, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING1_STACK", 1, CORE_1_INDEX, &PROCESSING1_STACK_1, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "REGISTERS_BUFFER", 1, CORE_1_INDEX, &REGISTERS_BUFFER_1, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "TUNNELS_PARSING_CFG", 1, CORE_1_INDEX, &TUNNELS_PARSING_CFG_1, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_1_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING2_STACK", 1, CORE_1_INDEX, &PROCESSING2_STACK_1, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_1_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_BAD_IPV4_HDR_CSUM_PACKETS", 1, CORE_1_INDEX, &CSO_BAD_IPV4_HDR_CSUM_PACKETS_1, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_1_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TASK_IDX", 1, CORE_1_INDEX, &TASK_IDX_1, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_1_INDEX, &LOOPBACK_QUEUE_TABLE_1, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DOS_DROP_REASONS_CFG", 1, CORE_1_INDEX, &DOS_DROP_REASONS_CFG_1, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_CFG_EX_TABLE", 1, CORE_1_INDEX, &VPORT_CFG_EX_TABLE_1, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_1_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING3_STACK", 1, CORE_1_INDEX, &PROCESSING3_STACK_1, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_CFG_TABLE", 1, CORE_1_INDEX, &IPTV_CFG_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_FPM_THRESHOLDS", 1, CORE_1_INDEX, &DHD_FPM_THRESHOLDS_1, 3, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_1_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_1, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_1_INDEX, &LOOPBACK_WAN_FLOW_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FORCE_DSCP", 1, CORE_1_INDEX, &FORCE_DSCP_1, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CORE_ID_TABLE", 1, CORE_1_INDEX, &CORE_ID_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TR471_SPDSVC_RX_PKT_ID_TABLE", 1, CORE_1_INDEX, &TR471_SPDSVC_RX_PKT_ID_TABLE_1, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_TABLE", 1, CORE_1_INDEX, &DEBUG_PRINT_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_1_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_1, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_1_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_1, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING4_STACK", 1, CORE_1_INDEX, &PROCESSING4_STACK_1, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "TCAM_GENERIC_FIELDS", 1, CORE_1_INDEX, &TCAM_GENERIC_FIELDS_1, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_1_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_1, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_1_INDEX, &BUFMNG_DESCRIPTOR_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_STATUS_TABLE", 1, CORE_1_INDEX, &BUFMNG_STATUS_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_1_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_1, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_1_INDEX, &DEBUG_PRINT_CORE_LOCK_1, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_HW_CFG", 1, CORE_1_INDEX, &DHD_HW_CFG_1, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_1_INDEX, &RX_MIRRORING_CONFIGURATION_1, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_1_INDEX, &TX_MIRRORING_CONFIGURATION_1, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "NAT_CACHE_CFG", 1, CORE_1_INDEX, &NAT_CACHE_CFG_1, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SRAM_DUMMY_STORE", 1, CORE_1_INDEX, &SRAM_DUMMY_STORE_1, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING5_STACK", 1, CORE_1_INDEX, &PROCESSING5_STACK_1, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "CHANNEL_ID_TO_SBPM_BB_ID_TABLE", 1, CORE_1_INDEX, &CHANNEL_ID_TO_SBPM_BB_ID_TABLE_1, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_1_INDEX, &NAT_CACHE_KEY0_MASK_1, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_DISABLE", 1, CORE_1_INDEX, &CSO_DISABLE_1, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_1_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_1_INDEX, &INGRESS_FILTER_1588_CFG_1, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_1_INDEX, &RX_MIRRORING_DIRECT_ENABLE_1, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_1_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_1, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_SCRATCHPAD", 1, CORE_1_INDEX, &DEBUG_SCRATCHPAD_1, 12, 1, 1 },
#endif
#if defined BCM6885
	{ "GDX_PARAMS_TABLE", 1, CORE_1_INDEX, &GDX_PARAMS_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_GLOBAL_CFG", 1, CORE_1_INDEX, &FPM_GLOBAL_CFG_1, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_1_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER_1, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING6_STACK", 1, CORE_1_INDEX, &PROCESSING6_STACK_1, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_1_INDEX, &TCAM_TABLE_CFG_TABLE_1, 2, 1, 1 },
#endif
#if defined BCM6885
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_1_INDEX, &NATC_L2_VLAN_KEY_MASK_1, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_CFG", 1, CORE_1_INDEX, &INGRESS_FILTER_CFG_1, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_1_INDEX, &BITS_CALC_MASKS_TABLE_1, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "NATC_L2_TOS_MASK", 1, CORE_1_INDEX, &NATC_L2_TOS_MASK_1, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_1_INDEX, &FW_ERROR_VECTOR_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "MULTICAST_KEY_MASK", 1, CORE_1_INDEX, &MULTICAST_KEY_MASK_1, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING7_STACK", 1, CORE_1_INDEX, &PROCESSING7_STACK_1, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_FLOW_TABLE", 1, CORE_1_INDEX, &RX_FLOW_TABLE_1, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_FLOW_TABLE", 1, CORE_1_INDEX, &TX_FLOW_TABLE_1, 212, 1, 1 },
#endif
#if defined BCM6885
	{ "PARSER_KEY_BUILD_TABLE", 1, CORE_1_INDEX, &PARSER_KEY_BUILD_TABLE_1, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_1_INDEX, &INGRESS_FILTER_PROFILE_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "POLICER_PARAMS_TABLE", 1, CORE_2_INDEX, &POLICER_PARAMS_TABLE_2, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING0_STACK", 1, CORE_2_INDEX, &PROCESSING0_STACK_2, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "SYSTEM_CONFIGURATION", 1, CORE_2_INDEX, &SYSTEM_CONFIGURATION_2, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDTEST_GEN_PARAM", 1, CORE_2_INDEX, &SPDTEST_GEN_PARAM_2, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_CFG_TABLE", 1, CORE_2_INDEX, &VPORT_CFG_TABLE_2, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "MUTEX_TABLE", 1, CORE_2_INDEX, &MUTEX_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_2_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_2, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING1_STACK", 1, CORE_2_INDEX, &PROCESSING1_STACK_2, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "REGISTERS_BUFFER", 1, CORE_2_INDEX, &REGISTERS_BUFFER_2, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "TUNNELS_PARSING_CFG", 1, CORE_2_INDEX, &TUNNELS_PARSING_CFG_2, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_2_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_2, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING2_STACK", 1, CORE_2_INDEX, &PROCESSING2_STACK_2, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_2_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_BAD_IPV4_HDR_CSUM_PACKETS", 1, CORE_2_INDEX, &CSO_BAD_IPV4_HDR_CSUM_PACKETS_2, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_2_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TASK_IDX", 1, CORE_2_INDEX, &TASK_IDX_2, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_2_INDEX, &LOOPBACK_QUEUE_TABLE_2, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DOS_DROP_REASONS_CFG", 1, CORE_2_INDEX, &DOS_DROP_REASONS_CFG_2, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_CFG_EX_TABLE", 1, CORE_2_INDEX, &VPORT_CFG_EX_TABLE_2, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_2_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING3_STACK", 1, CORE_2_INDEX, &PROCESSING3_STACK_2, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_CFG_TABLE", 1, CORE_2_INDEX, &IPTV_CFG_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_FPM_THRESHOLDS", 1, CORE_2_INDEX, &DHD_FPM_THRESHOLDS_2, 3, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_2_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_2, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_2_INDEX, &LOOPBACK_WAN_FLOW_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FORCE_DSCP", 1, CORE_2_INDEX, &FORCE_DSCP_2, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CORE_ID_TABLE", 1, CORE_2_INDEX, &CORE_ID_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TR471_SPDSVC_RX_PKT_ID_TABLE", 1, CORE_2_INDEX, &TR471_SPDSVC_RX_PKT_ID_TABLE_2, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_TABLE", 1, CORE_2_INDEX, &DEBUG_PRINT_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_2_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_2, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_2_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_2, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING4_STACK", 1, CORE_2_INDEX, &PROCESSING4_STACK_2, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "TCAM_GENERIC_FIELDS", 1, CORE_2_INDEX, &TCAM_GENERIC_FIELDS_2, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_2_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_2, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_2_INDEX, &BUFMNG_DESCRIPTOR_TABLE_2, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_STATUS_TABLE", 1, CORE_2_INDEX, &BUFMNG_STATUS_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_2_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_2, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_2_INDEX, &DEBUG_PRINT_CORE_LOCK_2, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_HW_CFG", 1, CORE_2_INDEX, &DHD_HW_CFG_2, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_2_INDEX, &RX_MIRRORING_CONFIGURATION_2, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_2_INDEX, &TX_MIRRORING_CONFIGURATION_2, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "NAT_CACHE_CFG", 1, CORE_2_INDEX, &NAT_CACHE_CFG_2, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SRAM_DUMMY_STORE", 1, CORE_2_INDEX, &SRAM_DUMMY_STORE_2, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING5_STACK", 1, CORE_2_INDEX, &PROCESSING5_STACK_2, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "CHANNEL_ID_TO_SBPM_BB_ID_TABLE", 1, CORE_2_INDEX, &CHANNEL_ID_TO_SBPM_BB_ID_TABLE_2, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_2_INDEX, &NAT_CACHE_KEY0_MASK_2, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_DISABLE", 1, CORE_2_INDEX, &CSO_DISABLE_2, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_2_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_2_INDEX, &INGRESS_FILTER_1588_CFG_2, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_2_INDEX, &RX_MIRRORING_DIRECT_ENABLE_2, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_2_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_2, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_SCRATCHPAD", 1, CORE_2_INDEX, &DEBUG_SCRATCHPAD_2, 12, 1, 1 },
#endif
#if defined BCM6885
	{ "GDX_PARAMS_TABLE", 1, CORE_2_INDEX, &GDX_PARAMS_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_GLOBAL_CFG", 1, CORE_2_INDEX, &FPM_GLOBAL_CFG_2, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_2_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER_2, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING6_STACK", 1, CORE_2_INDEX, &PROCESSING6_STACK_2, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_2_INDEX, &TCAM_TABLE_CFG_TABLE_2, 2, 1, 1 },
#endif
#if defined BCM6885
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_2_INDEX, &NATC_L2_VLAN_KEY_MASK_2, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_CFG", 1, CORE_2_INDEX, &INGRESS_FILTER_CFG_2, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_2_INDEX, &BITS_CALC_MASKS_TABLE_2, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "NATC_L2_TOS_MASK", 1, CORE_2_INDEX, &NATC_L2_TOS_MASK_2, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_2_INDEX, &FW_ERROR_VECTOR_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "MULTICAST_KEY_MASK", 1, CORE_2_INDEX, &MULTICAST_KEY_MASK_2, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING7_STACK", 1, CORE_2_INDEX, &PROCESSING7_STACK_2, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_FLOW_TABLE", 1, CORE_2_INDEX, &RX_FLOW_TABLE_2, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_FLOW_TABLE", 1, CORE_2_INDEX, &TX_FLOW_TABLE_2, 212, 1, 1 },
#endif
#if defined BCM6885
	{ "PARSER_KEY_BUILD_TABLE", 1, CORE_2_INDEX, &PARSER_KEY_BUILD_TABLE_2, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_2_INDEX, &INGRESS_FILTER_PROFILE_TABLE_2, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "POLICER_PARAMS_TABLE", 1, CORE_3_INDEX, &POLICER_PARAMS_TABLE_3, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING0_STACK", 1, CORE_3_INDEX, &PROCESSING0_STACK_3, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "SYSTEM_CONFIGURATION", 1, CORE_3_INDEX, &SYSTEM_CONFIGURATION_3, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDTEST_GEN_PARAM", 1, CORE_3_INDEX, &SPDTEST_GEN_PARAM_3, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_CFG_TABLE", 1, CORE_3_INDEX, &VPORT_CFG_TABLE_3, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "MUTEX_TABLE", 1, CORE_3_INDEX, &MUTEX_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_3_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_3, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING1_STACK", 1, CORE_3_INDEX, &PROCESSING1_STACK_3, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "REGISTERS_BUFFER", 1, CORE_3_INDEX, &REGISTERS_BUFFER_3, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "TUNNELS_PARSING_CFG", 1, CORE_3_INDEX, &TUNNELS_PARSING_CFG_3, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_3_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_3, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING2_STACK", 1, CORE_3_INDEX, &PROCESSING2_STACK_3, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_3_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_BAD_IPV4_HDR_CSUM_PACKETS", 1, CORE_3_INDEX, &CSO_BAD_IPV4_HDR_CSUM_PACKETS_3, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_3_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TASK_IDX", 1, CORE_3_INDEX, &TASK_IDX_3, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_3_INDEX, &LOOPBACK_QUEUE_TABLE_3, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DOS_DROP_REASONS_CFG", 1, CORE_3_INDEX, &DOS_DROP_REASONS_CFG_3, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_CFG_EX_TABLE", 1, CORE_3_INDEX, &VPORT_CFG_EX_TABLE_3, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_3_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING3_STACK", 1, CORE_3_INDEX, &PROCESSING3_STACK_3, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_CFG_TABLE", 1, CORE_3_INDEX, &IPTV_CFG_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_FPM_THRESHOLDS", 1, CORE_3_INDEX, &DHD_FPM_THRESHOLDS_3, 3, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_3_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_3, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_3_INDEX, &LOOPBACK_WAN_FLOW_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FORCE_DSCP", 1, CORE_3_INDEX, &FORCE_DSCP_3, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CORE_ID_TABLE", 1, CORE_3_INDEX, &CORE_ID_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TR471_SPDSVC_RX_PKT_ID_TABLE", 1, CORE_3_INDEX, &TR471_SPDSVC_RX_PKT_ID_TABLE_3, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_TABLE", 1, CORE_3_INDEX, &DEBUG_PRINT_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_3_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_3, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_3_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_3, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING4_STACK", 1, CORE_3_INDEX, &PROCESSING4_STACK_3, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "TCAM_GENERIC_FIELDS", 1, CORE_3_INDEX, &TCAM_GENERIC_FIELDS_3, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_3_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_3, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_3_INDEX, &BUFMNG_DESCRIPTOR_TABLE_3, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_STATUS_TABLE", 1, CORE_3_INDEX, &BUFMNG_STATUS_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_3_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_3, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_3_INDEX, &DEBUG_PRINT_CORE_LOCK_3, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_HW_CFG", 1, CORE_3_INDEX, &DHD_HW_CFG_3, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_3_INDEX, &RX_MIRRORING_CONFIGURATION_3, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_3_INDEX, &TX_MIRRORING_CONFIGURATION_3, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "NAT_CACHE_CFG", 1, CORE_3_INDEX, &NAT_CACHE_CFG_3, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SRAM_DUMMY_STORE", 1, CORE_3_INDEX, &SRAM_DUMMY_STORE_3, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING5_STACK", 1, CORE_3_INDEX, &PROCESSING5_STACK_3, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "CHANNEL_ID_TO_SBPM_BB_ID_TABLE", 1, CORE_3_INDEX, &CHANNEL_ID_TO_SBPM_BB_ID_TABLE_3, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_3_INDEX, &NAT_CACHE_KEY0_MASK_3, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_DISABLE", 1, CORE_3_INDEX, &CSO_DISABLE_3, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_3_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_3_INDEX, &INGRESS_FILTER_1588_CFG_3, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_3_INDEX, &RX_MIRRORING_DIRECT_ENABLE_3, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_3_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_3, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_SCRATCHPAD", 1, CORE_3_INDEX, &DEBUG_SCRATCHPAD_3, 12, 1, 1 },
#endif
#if defined BCM6885
	{ "GDX_PARAMS_TABLE", 1, CORE_3_INDEX, &GDX_PARAMS_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_GLOBAL_CFG", 1, CORE_3_INDEX, &FPM_GLOBAL_CFG_3, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_3_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER_3, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING6_STACK", 1, CORE_3_INDEX, &PROCESSING6_STACK_3, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_3_INDEX, &TCAM_TABLE_CFG_TABLE_3, 2, 1, 1 },
#endif
#if defined BCM6885
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_3_INDEX, &NATC_L2_VLAN_KEY_MASK_3, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_CFG", 1, CORE_3_INDEX, &INGRESS_FILTER_CFG_3, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_3_INDEX, &BITS_CALC_MASKS_TABLE_3, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "NATC_L2_TOS_MASK", 1, CORE_3_INDEX, &NATC_L2_TOS_MASK_3, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_3_INDEX, &FW_ERROR_VECTOR_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "MULTICAST_KEY_MASK", 1, CORE_3_INDEX, &MULTICAST_KEY_MASK_3, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING7_STACK", 1, CORE_3_INDEX, &PROCESSING7_STACK_3, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_FLOW_TABLE", 1, CORE_3_INDEX, &RX_FLOW_TABLE_3, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_FLOW_TABLE", 1, CORE_3_INDEX, &TX_FLOW_TABLE_3, 212, 1, 1 },
#endif
#if defined BCM6885
	{ "PARSER_KEY_BUILD_TABLE", 1, CORE_3_INDEX, &PARSER_KEY_BUILD_TABLE_3, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_3_INDEX, &INGRESS_FILTER_PROFILE_TABLE_3, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "POLICER_PARAMS_TABLE", 1, CORE_4_INDEX, &POLICER_PARAMS_TABLE_4, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING0_STACK", 1, CORE_4_INDEX, &PROCESSING0_STACK_4, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "SYSTEM_CONFIGURATION", 1, CORE_4_INDEX, &SYSTEM_CONFIGURATION_4, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDTEST_GEN_PARAM", 1, CORE_4_INDEX, &SPDTEST_GEN_PARAM_4, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_CFG_TABLE", 1, CORE_4_INDEX, &VPORT_CFG_TABLE_4, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "MUTEX_TABLE", 1, CORE_4_INDEX, &MUTEX_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_4_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_4, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING1_STACK", 1, CORE_4_INDEX, &PROCESSING1_STACK_4, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "REGISTERS_BUFFER", 1, CORE_4_INDEX, &REGISTERS_BUFFER_4, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "TUNNELS_PARSING_CFG", 1, CORE_4_INDEX, &TUNNELS_PARSING_CFG_4, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_4_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_4, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING2_STACK", 1, CORE_4_INDEX, &PROCESSING2_STACK_4, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_4_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_BAD_IPV4_HDR_CSUM_PACKETS", 1, CORE_4_INDEX, &CSO_BAD_IPV4_HDR_CSUM_PACKETS_4, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_4_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TASK_IDX", 1, CORE_4_INDEX, &TASK_IDX_4, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_4_INDEX, &LOOPBACK_QUEUE_TABLE_4, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DOS_DROP_REASONS_CFG", 1, CORE_4_INDEX, &DOS_DROP_REASONS_CFG_4, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_CFG_EX_TABLE", 1, CORE_4_INDEX, &VPORT_CFG_EX_TABLE_4, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_4_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING3_STACK", 1, CORE_4_INDEX, &PROCESSING3_STACK_4, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_CFG_TABLE", 1, CORE_4_INDEX, &IPTV_CFG_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_FPM_THRESHOLDS", 1, CORE_4_INDEX, &DHD_FPM_THRESHOLDS_4, 3, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_4_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_4, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_4_INDEX, &LOOPBACK_WAN_FLOW_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FORCE_DSCP", 1, CORE_4_INDEX, &FORCE_DSCP_4, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CORE_ID_TABLE", 1, CORE_4_INDEX, &CORE_ID_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TR471_SPDSVC_RX_PKT_ID_TABLE", 1, CORE_4_INDEX, &TR471_SPDSVC_RX_PKT_ID_TABLE_4, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_TABLE", 1, CORE_4_INDEX, &DEBUG_PRINT_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_4_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_4, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_4_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_4, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING4_STACK", 1, CORE_4_INDEX, &PROCESSING4_STACK_4, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "TCAM_GENERIC_FIELDS", 1, CORE_4_INDEX, &TCAM_GENERIC_FIELDS_4, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_4_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_4, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_4_INDEX, &BUFMNG_DESCRIPTOR_TABLE_4, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_STATUS_TABLE", 1, CORE_4_INDEX, &BUFMNG_STATUS_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_4_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_4, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_4_INDEX, &DEBUG_PRINT_CORE_LOCK_4, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_HW_CFG", 1, CORE_4_INDEX, &DHD_HW_CFG_4, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_4_INDEX, &RX_MIRRORING_CONFIGURATION_4, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_4_INDEX, &TX_MIRRORING_CONFIGURATION_4, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "NAT_CACHE_CFG", 1, CORE_4_INDEX, &NAT_CACHE_CFG_4, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SRAM_DUMMY_STORE", 1, CORE_4_INDEX, &SRAM_DUMMY_STORE_4, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING5_STACK", 1, CORE_4_INDEX, &PROCESSING5_STACK_4, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "CHANNEL_ID_TO_SBPM_BB_ID_TABLE", 1, CORE_4_INDEX, &CHANNEL_ID_TO_SBPM_BB_ID_TABLE_4, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_4_INDEX, &NAT_CACHE_KEY0_MASK_4, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_DISABLE", 1, CORE_4_INDEX, &CSO_DISABLE_4, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_4_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_4_INDEX, &INGRESS_FILTER_1588_CFG_4, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_4_INDEX, &RX_MIRRORING_DIRECT_ENABLE_4, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_4_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_4, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_SCRATCHPAD", 1, CORE_4_INDEX, &DEBUG_SCRATCHPAD_4, 12, 1, 1 },
#endif
#if defined BCM6885
	{ "GDX_PARAMS_TABLE", 1, CORE_4_INDEX, &GDX_PARAMS_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_GLOBAL_CFG", 1, CORE_4_INDEX, &FPM_GLOBAL_CFG_4, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_4_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER_4, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING6_STACK", 1, CORE_4_INDEX, &PROCESSING6_STACK_4, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_4_INDEX, &TCAM_TABLE_CFG_TABLE_4, 2, 1, 1 },
#endif
#if defined BCM6885
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_4_INDEX, &NATC_L2_VLAN_KEY_MASK_4, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_CFG", 1, CORE_4_INDEX, &INGRESS_FILTER_CFG_4, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_4_INDEX, &BITS_CALC_MASKS_TABLE_4, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "NATC_L2_TOS_MASK", 1, CORE_4_INDEX, &NATC_L2_TOS_MASK_4, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_4_INDEX, &FW_ERROR_VECTOR_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "MULTICAST_KEY_MASK", 1, CORE_4_INDEX, &MULTICAST_KEY_MASK_4, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING7_STACK", 1, CORE_4_INDEX, &PROCESSING7_STACK_4, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_FLOW_TABLE", 1, CORE_4_INDEX, &RX_FLOW_TABLE_4, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_FLOW_TABLE", 1, CORE_4_INDEX, &TX_FLOW_TABLE_4, 212, 1, 1 },
#endif
#if defined BCM6885
	{ "PARSER_KEY_BUILD_TABLE", 1, CORE_4_INDEX, &PARSER_KEY_BUILD_TABLE_4, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_4_INDEX, &INGRESS_FILTER_PROFILE_TABLE_4, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "POLICER_PARAMS_TABLE", 1, CORE_5_INDEX, &POLICER_PARAMS_TABLE_5, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING0_STACK", 1, CORE_5_INDEX, &PROCESSING0_STACK_5, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "SYSTEM_CONFIGURATION", 1, CORE_5_INDEX, &SYSTEM_CONFIGURATION_5, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDTEST_GEN_PARAM", 1, CORE_5_INDEX, &SPDTEST_GEN_PARAM_5, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_CFG_TABLE", 1, CORE_5_INDEX, &VPORT_CFG_TABLE_5, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "MUTEX_TABLE", 1, CORE_5_INDEX, &MUTEX_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_5_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_5, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING1_STACK", 1, CORE_5_INDEX, &PROCESSING1_STACK_5, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "REGISTERS_BUFFER", 1, CORE_5_INDEX, &REGISTERS_BUFFER_5, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "TUNNELS_PARSING_CFG", 1, CORE_5_INDEX, &TUNNELS_PARSING_CFG_5, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_5_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_5, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING2_STACK", 1, CORE_5_INDEX, &PROCESSING2_STACK_5, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_5_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_BAD_IPV4_HDR_CSUM_PACKETS", 1, CORE_5_INDEX, &CSO_BAD_IPV4_HDR_CSUM_PACKETS_5, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_5_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TASK_IDX", 1, CORE_5_INDEX, &TASK_IDX_5, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_5_INDEX, &LOOPBACK_QUEUE_TABLE_5, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DOS_DROP_REASONS_CFG", 1, CORE_5_INDEX, &DOS_DROP_REASONS_CFG_5, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_CFG_EX_TABLE", 1, CORE_5_INDEX, &VPORT_CFG_EX_TABLE_5, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_5_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING3_STACK", 1, CORE_5_INDEX, &PROCESSING3_STACK_5, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_CFG_TABLE", 1, CORE_5_INDEX, &IPTV_CFG_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_FPM_THRESHOLDS", 1, CORE_5_INDEX, &DHD_FPM_THRESHOLDS_5, 3, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_5_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_5, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_5_INDEX, &LOOPBACK_WAN_FLOW_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FORCE_DSCP", 1, CORE_5_INDEX, &FORCE_DSCP_5, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CORE_ID_TABLE", 1, CORE_5_INDEX, &CORE_ID_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TR471_SPDSVC_RX_PKT_ID_TABLE", 1, CORE_5_INDEX, &TR471_SPDSVC_RX_PKT_ID_TABLE_5, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_TABLE", 1, CORE_5_INDEX, &DEBUG_PRINT_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_5_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_5, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_5_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_5, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING4_STACK", 1, CORE_5_INDEX, &PROCESSING4_STACK_5, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "TCAM_GENERIC_FIELDS", 1, CORE_5_INDEX, &TCAM_GENERIC_FIELDS_5, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_5_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_5, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_5_INDEX, &BUFMNG_DESCRIPTOR_TABLE_5, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_STATUS_TABLE", 1, CORE_5_INDEX, &BUFMNG_STATUS_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_5_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_5, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_5_INDEX, &DEBUG_PRINT_CORE_LOCK_5, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_HW_CFG", 1, CORE_5_INDEX, &DHD_HW_CFG_5, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_5_INDEX, &RX_MIRRORING_CONFIGURATION_5, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_5_INDEX, &TX_MIRRORING_CONFIGURATION_5, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "NAT_CACHE_CFG", 1, CORE_5_INDEX, &NAT_CACHE_CFG_5, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SRAM_DUMMY_STORE", 1, CORE_5_INDEX, &SRAM_DUMMY_STORE_5, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING5_STACK", 1, CORE_5_INDEX, &PROCESSING5_STACK_5, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "CHANNEL_ID_TO_SBPM_BB_ID_TABLE", 1, CORE_5_INDEX, &CHANNEL_ID_TO_SBPM_BB_ID_TABLE_5, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_5_INDEX, &NAT_CACHE_KEY0_MASK_5, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_DISABLE", 1, CORE_5_INDEX, &CSO_DISABLE_5, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_5_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_5_INDEX, &INGRESS_FILTER_1588_CFG_5, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_5_INDEX, &RX_MIRRORING_DIRECT_ENABLE_5, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_5_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_5, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_SCRATCHPAD", 1, CORE_5_INDEX, &DEBUG_SCRATCHPAD_5, 12, 1, 1 },
#endif
#if defined BCM6885
	{ "GDX_PARAMS_TABLE", 1, CORE_5_INDEX, &GDX_PARAMS_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_GLOBAL_CFG", 1, CORE_5_INDEX, &FPM_GLOBAL_CFG_5, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_5_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER_5, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING6_STACK", 1, CORE_5_INDEX, &PROCESSING6_STACK_5, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_5_INDEX, &TCAM_TABLE_CFG_TABLE_5, 2, 1, 1 },
#endif
#if defined BCM6885
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_5_INDEX, &NATC_L2_VLAN_KEY_MASK_5, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_CFG", 1, CORE_5_INDEX, &INGRESS_FILTER_CFG_5, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_5_INDEX, &BITS_CALC_MASKS_TABLE_5, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "NATC_L2_TOS_MASK", 1, CORE_5_INDEX, &NATC_L2_TOS_MASK_5, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_5_INDEX, &FW_ERROR_VECTOR_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "MULTICAST_KEY_MASK", 1, CORE_5_INDEX, &MULTICAST_KEY_MASK_5, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING7_STACK", 1, CORE_5_INDEX, &PROCESSING7_STACK_5, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_FLOW_TABLE", 1, CORE_5_INDEX, &RX_FLOW_TABLE_5, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_FLOW_TABLE", 1, CORE_5_INDEX, &TX_FLOW_TABLE_5, 212, 1, 1 },
#endif
#if defined BCM6885
	{ "PARSER_KEY_BUILD_TABLE", 1, CORE_5_INDEX, &PARSER_KEY_BUILD_TABLE_5, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_5_INDEX, &INGRESS_FILTER_PROFILE_TABLE_5, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "POLICER_PARAMS_TABLE", 1, CORE_6_INDEX, &POLICER_PARAMS_TABLE_6, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING0_STACK", 1, CORE_6_INDEX, &PROCESSING0_STACK_6, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "SYSTEM_CONFIGURATION", 1, CORE_6_INDEX, &SYSTEM_CONFIGURATION_6, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDTEST_GEN_PARAM", 1, CORE_6_INDEX, &SPDTEST_GEN_PARAM_6, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_CFG_TABLE", 1, CORE_6_INDEX, &VPORT_CFG_TABLE_6, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "MUTEX_TABLE", 1, CORE_6_INDEX, &MUTEX_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_6_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_6, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING1_STACK", 1, CORE_6_INDEX, &PROCESSING1_STACK_6, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "REGISTERS_BUFFER", 1, CORE_6_INDEX, &REGISTERS_BUFFER_6, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "TUNNELS_PARSING_CFG", 1, CORE_6_INDEX, &TUNNELS_PARSING_CFG_6, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_6_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_6, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING2_STACK", 1, CORE_6_INDEX, &PROCESSING2_STACK_6, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_6_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_BAD_IPV4_HDR_CSUM_PACKETS", 1, CORE_6_INDEX, &CSO_BAD_IPV4_HDR_CSUM_PACKETS_6, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_6_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TASK_IDX", 1, CORE_6_INDEX, &TASK_IDX_6, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_6_INDEX, &LOOPBACK_QUEUE_TABLE_6, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DOS_DROP_REASONS_CFG", 1, CORE_6_INDEX, &DOS_DROP_REASONS_CFG_6, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_CFG_EX_TABLE", 1, CORE_6_INDEX, &VPORT_CFG_EX_TABLE_6, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_6_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING3_STACK", 1, CORE_6_INDEX, &PROCESSING3_STACK_6, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_CFG_TABLE", 1, CORE_6_INDEX, &IPTV_CFG_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_FPM_THRESHOLDS", 1, CORE_6_INDEX, &DHD_FPM_THRESHOLDS_6, 3, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_6_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_6, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_6_INDEX, &LOOPBACK_WAN_FLOW_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FORCE_DSCP", 1, CORE_6_INDEX, &FORCE_DSCP_6, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CORE_ID_TABLE", 1, CORE_6_INDEX, &CORE_ID_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TR471_SPDSVC_RX_PKT_ID_TABLE", 1, CORE_6_INDEX, &TR471_SPDSVC_RX_PKT_ID_TABLE_6, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_TABLE", 1, CORE_6_INDEX, &DEBUG_PRINT_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_6_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_6, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_6_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_6, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING4_STACK", 1, CORE_6_INDEX, &PROCESSING4_STACK_6, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "TCAM_GENERIC_FIELDS", 1, CORE_6_INDEX, &TCAM_GENERIC_FIELDS_6, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_6_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_6, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_6_INDEX, &BUFMNG_DESCRIPTOR_TABLE_6, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_STATUS_TABLE", 1, CORE_6_INDEX, &BUFMNG_STATUS_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_6_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_6, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_6_INDEX, &DEBUG_PRINT_CORE_LOCK_6, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_HW_CFG", 1, CORE_6_INDEX, &DHD_HW_CFG_6, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_6_INDEX, &RX_MIRRORING_CONFIGURATION_6, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_6_INDEX, &TX_MIRRORING_CONFIGURATION_6, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "NAT_CACHE_CFG", 1, CORE_6_INDEX, &NAT_CACHE_CFG_6, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SRAM_DUMMY_STORE", 1, CORE_6_INDEX, &SRAM_DUMMY_STORE_6, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING5_STACK", 1, CORE_6_INDEX, &PROCESSING5_STACK_6, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "CHANNEL_ID_TO_SBPM_BB_ID_TABLE", 1, CORE_6_INDEX, &CHANNEL_ID_TO_SBPM_BB_ID_TABLE_6, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_6_INDEX, &NAT_CACHE_KEY0_MASK_6, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_DISABLE", 1, CORE_6_INDEX, &CSO_DISABLE_6, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_6_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_6_INDEX, &INGRESS_FILTER_1588_CFG_6, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_6_INDEX, &RX_MIRRORING_DIRECT_ENABLE_6, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_6_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_6, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_SCRATCHPAD", 1, CORE_6_INDEX, &DEBUG_SCRATCHPAD_6, 12, 1, 1 },
#endif
#if defined BCM6885
	{ "GDX_PARAMS_TABLE", 1, CORE_6_INDEX, &GDX_PARAMS_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_GLOBAL_CFG", 1, CORE_6_INDEX, &FPM_GLOBAL_CFG_6, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_6_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER_6, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING6_STACK", 1, CORE_6_INDEX, &PROCESSING6_STACK_6, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_6_INDEX, &TCAM_TABLE_CFG_TABLE_6, 2, 1, 1 },
#endif
#if defined BCM6885
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_6_INDEX, &NATC_L2_VLAN_KEY_MASK_6, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_CFG", 1, CORE_6_INDEX, &INGRESS_FILTER_CFG_6, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_6_INDEX, &BITS_CALC_MASKS_TABLE_6, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "NATC_L2_TOS_MASK", 1, CORE_6_INDEX, &NATC_L2_TOS_MASK_6, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_6_INDEX, &FW_ERROR_VECTOR_TABLE_6, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "MULTICAST_KEY_MASK", 1, CORE_6_INDEX, &MULTICAST_KEY_MASK_6, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING7_STACK", 1, CORE_6_INDEX, &PROCESSING7_STACK_6, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_FLOW_TABLE", 1, CORE_6_INDEX, &RX_FLOW_TABLE_6, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_FLOW_TABLE", 1, CORE_6_INDEX, &TX_FLOW_TABLE_6, 212, 1, 1 },
#endif
#if defined BCM6885
	{ "PARSER_KEY_BUILD_TABLE", 1, CORE_6_INDEX, &PARSER_KEY_BUILD_TABLE_6, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_6_INDEX, &INGRESS_FILTER_PROFILE_TABLE_6, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "POLICER_PARAMS_TABLE", 1, CORE_7_INDEX, &POLICER_PARAMS_TABLE_7, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING0_STACK", 1, CORE_7_INDEX, &PROCESSING0_STACK_7, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "SYSTEM_CONFIGURATION", 1, CORE_7_INDEX, &SYSTEM_CONFIGURATION_7, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDTEST_GEN_PARAM", 1, CORE_7_INDEX, &SPDTEST_GEN_PARAM_7, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_CFG_TABLE", 1, CORE_7_INDEX, &VPORT_CFG_TABLE_7, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "MUTEX_TABLE", 1, CORE_7_INDEX, &MUTEX_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_7_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_7, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING1_STACK", 1, CORE_7_INDEX, &PROCESSING1_STACK_7, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "REGISTERS_BUFFER", 1, CORE_7_INDEX, &REGISTERS_BUFFER_7, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "TUNNELS_PARSING_CFG", 1, CORE_7_INDEX, &TUNNELS_PARSING_CFG_7, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_7_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_7, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING2_STACK", 1, CORE_7_INDEX, &PROCESSING2_STACK_7, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_7_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_BAD_IPV4_HDR_CSUM_PACKETS", 1, CORE_7_INDEX, &CSO_BAD_IPV4_HDR_CSUM_PACKETS_7, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_7_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TASK_IDX", 1, CORE_7_INDEX, &TASK_IDX_7, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_7_INDEX, &LOOPBACK_QUEUE_TABLE_7, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DOS_DROP_REASONS_CFG", 1, CORE_7_INDEX, &DOS_DROP_REASONS_CFG_7, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_CFG_EX_TABLE", 1, CORE_7_INDEX, &VPORT_CFG_EX_TABLE_7, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_7_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING3_STACK", 1, CORE_7_INDEX, &PROCESSING3_STACK_7, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_CFG_TABLE", 1, CORE_7_INDEX, &IPTV_CFG_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_FPM_THRESHOLDS", 1, CORE_7_INDEX, &DHD_FPM_THRESHOLDS_7, 3, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_7_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_7, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_7_INDEX, &LOOPBACK_WAN_FLOW_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FORCE_DSCP", 1, CORE_7_INDEX, &FORCE_DSCP_7, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CORE_ID_TABLE", 1, CORE_7_INDEX, &CORE_ID_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TR471_SPDSVC_RX_PKT_ID_TABLE", 1, CORE_7_INDEX, &TR471_SPDSVC_RX_PKT_ID_TABLE_7, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_TABLE", 1, CORE_7_INDEX, &DEBUG_PRINT_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_7_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_7, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_7_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_7, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING4_STACK", 1, CORE_7_INDEX, &PROCESSING4_STACK_7, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "TCAM_GENERIC_FIELDS", 1, CORE_7_INDEX, &TCAM_GENERIC_FIELDS_7, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_7_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_7, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_7_INDEX, &BUFMNG_DESCRIPTOR_TABLE_7, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_STATUS_TABLE", 1, CORE_7_INDEX, &BUFMNG_STATUS_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_7_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_7, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_7_INDEX, &DEBUG_PRINT_CORE_LOCK_7, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_HW_CFG", 1, CORE_7_INDEX, &DHD_HW_CFG_7, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_7_INDEX, &RX_MIRRORING_CONFIGURATION_7, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_7_INDEX, &TX_MIRRORING_CONFIGURATION_7, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "NAT_CACHE_CFG", 1, CORE_7_INDEX, &NAT_CACHE_CFG_7, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SRAM_DUMMY_STORE", 1, CORE_7_INDEX, &SRAM_DUMMY_STORE_7, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING5_STACK", 1, CORE_7_INDEX, &PROCESSING5_STACK_7, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "CHANNEL_ID_TO_SBPM_BB_ID_TABLE", 1, CORE_7_INDEX, &CHANNEL_ID_TO_SBPM_BB_ID_TABLE_7, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_7_INDEX, &NAT_CACHE_KEY0_MASK_7, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_DISABLE", 1, CORE_7_INDEX, &CSO_DISABLE_7, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_7_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_7_INDEX, &INGRESS_FILTER_1588_CFG_7, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_7_INDEX, &RX_MIRRORING_DIRECT_ENABLE_7, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_7_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_7, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_SCRATCHPAD", 1, CORE_7_INDEX, &DEBUG_SCRATCHPAD_7, 12, 1, 1 },
#endif
#if defined BCM6885
	{ "GDX_PARAMS_TABLE", 1, CORE_7_INDEX, &GDX_PARAMS_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_GLOBAL_CFG", 1, CORE_7_INDEX, &FPM_GLOBAL_CFG_7, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_7_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER_7, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING6_STACK", 1, CORE_7_INDEX, &PROCESSING6_STACK_7, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_7_INDEX, &TCAM_TABLE_CFG_TABLE_7, 2, 1, 1 },
#endif
#if defined BCM6885
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_7_INDEX, &NATC_L2_VLAN_KEY_MASK_7, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_CFG", 1, CORE_7_INDEX, &INGRESS_FILTER_CFG_7, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_7_INDEX, &BITS_CALC_MASKS_TABLE_7, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "NATC_L2_TOS_MASK", 1, CORE_7_INDEX, &NATC_L2_TOS_MASK_7, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_7_INDEX, &FW_ERROR_VECTOR_TABLE_7, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "MULTICAST_KEY_MASK", 1, CORE_7_INDEX, &MULTICAST_KEY_MASK_7, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING7_STACK", 1, CORE_7_INDEX, &PROCESSING7_STACK_7, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_FLOW_TABLE", 1, CORE_7_INDEX, &RX_FLOW_TABLE_7, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_FLOW_TABLE", 1, CORE_7_INDEX, &TX_FLOW_TABLE_7, 212, 1, 1 },
#endif
#if defined BCM6885
	{ "PARSER_KEY_BUILD_TABLE", 1, CORE_7_INDEX, &PARSER_KEY_BUILD_TABLE_7, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_7_INDEX, &INGRESS_FILTER_PROFILE_TABLE_7, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "POLICER_PARAMS_TABLE", 1, CORE_8_INDEX, &POLICER_PARAMS_TABLE_8, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING0_STACK", 1, CORE_8_INDEX, &PROCESSING0_STACK_8, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "SYSTEM_CONFIGURATION", 1, CORE_8_INDEX, &SYSTEM_CONFIGURATION_8, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDTEST_GEN_PARAM", 1, CORE_8_INDEX, &SPDTEST_GEN_PARAM_8, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_CFG_TABLE", 1, CORE_8_INDEX, &VPORT_CFG_TABLE_8, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "MUTEX_TABLE", 1, CORE_8_INDEX, &MUTEX_TABLE_8, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_8_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_8, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING1_STACK", 1, CORE_8_INDEX, &PROCESSING1_STACK_8, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "REGISTERS_BUFFER", 1, CORE_8_INDEX, &REGISTERS_BUFFER_8, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "TUNNELS_PARSING_CFG", 1, CORE_8_INDEX, &TUNNELS_PARSING_CFG_8, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_8_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_8, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING2_STACK", 1, CORE_8_INDEX, &PROCESSING2_STACK_8, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_8_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_8, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_BAD_IPV4_HDR_CSUM_PACKETS", 1, CORE_8_INDEX, &CSO_BAD_IPV4_HDR_CSUM_PACKETS_8, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_8_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_8, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TASK_IDX", 1, CORE_8_INDEX, &TASK_IDX_8, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_8_INDEX, &LOOPBACK_QUEUE_TABLE_8, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DOS_DROP_REASONS_CFG", 1, CORE_8_INDEX, &DOS_DROP_REASONS_CFG_8, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_CFG_EX_TABLE", 1, CORE_8_INDEX, &VPORT_CFG_EX_TABLE_8, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_8_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_8, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING3_STACK", 1, CORE_8_INDEX, &PROCESSING3_STACK_8, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_CFG_TABLE", 1, CORE_8_INDEX, &IPTV_CFG_TABLE_8, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_FPM_THRESHOLDS", 1, CORE_8_INDEX, &DHD_FPM_THRESHOLDS_8, 3, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_8_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_8, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_8_INDEX, &LOOPBACK_WAN_FLOW_TABLE_8, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FORCE_DSCP", 1, CORE_8_INDEX, &FORCE_DSCP_8, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CORE_ID_TABLE", 1, CORE_8_INDEX, &CORE_ID_TABLE_8, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TR471_SPDSVC_RX_PKT_ID_TABLE", 1, CORE_8_INDEX, &TR471_SPDSVC_RX_PKT_ID_TABLE_8, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_TABLE", 1, CORE_8_INDEX, &DEBUG_PRINT_TABLE_8, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_8_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_8, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_8_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_8, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING4_STACK", 1, CORE_8_INDEX, &PROCESSING4_STACK_8, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "TCAM_GENERIC_FIELDS", 1, CORE_8_INDEX, &TCAM_GENERIC_FIELDS_8, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_8_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_8, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_8_INDEX, &BUFMNG_DESCRIPTOR_TABLE_8, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_STATUS_TABLE", 1, CORE_8_INDEX, &BUFMNG_STATUS_TABLE_8, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_8_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_8, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_8_INDEX, &DEBUG_PRINT_CORE_LOCK_8, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_HW_CFG", 1, CORE_8_INDEX, &DHD_HW_CFG_8, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_8_INDEX, &RX_MIRRORING_CONFIGURATION_8, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_8_INDEX, &TX_MIRRORING_CONFIGURATION_8, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "NAT_CACHE_CFG", 1, CORE_8_INDEX, &NAT_CACHE_CFG_8, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SRAM_DUMMY_STORE", 1, CORE_8_INDEX, &SRAM_DUMMY_STORE_8, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING5_STACK", 1, CORE_8_INDEX, &PROCESSING5_STACK_8, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "CHANNEL_ID_TO_SBPM_BB_ID_TABLE", 1, CORE_8_INDEX, &CHANNEL_ID_TO_SBPM_BB_ID_TABLE_8, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_8_INDEX, &NAT_CACHE_KEY0_MASK_8, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_DISABLE", 1, CORE_8_INDEX, &CSO_DISABLE_8, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_8_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_8, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_8_INDEX, &INGRESS_FILTER_1588_CFG_8, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_8_INDEX, &RX_MIRRORING_DIRECT_ENABLE_8, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_8_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_8, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_SCRATCHPAD", 1, CORE_8_INDEX, &DEBUG_SCRATCHPAD_8, 12, 1, 1 },
#endif
#if defined BCM6885
	{ "GDX_PARAMS_TABLE", 1, CORE_8_INDEX, &GDX_PARAMS_TABLE_8, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_GLOBAL_CFG", 1, CORE_8_INDEX, &FPM_GLOBAL_CFG_8, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_8_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER_8, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING6_STACK", 1, CORE_8_INDEX, &PROCESSING6_STACK_8, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_8_INDEX, &TCAM_TABLE_CFG_TABLE_8, 2, 1, 1 },
#endif
#if defined BCM6885
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_8_INDEX, &NATC_L2_VLAN_KEY_MASK_8, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_CFG", 1, CORE_8_INDEX, &INGRESS_FILTER_CFG_8, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_8_INDEX, &BITS_CALC_MASKS_TABLE_8, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "NATC_L2_TOS_MASK", 1, CORE_8_INDEX, &NATC_L2_TOS_MASK_8, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_8_INDEX, &FW_ERROR_VECTOR_TABLE_8, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "MULTICAST_KEY_MASK", 1, CORE_8_INDEX, &MULTICAST_KEY_MASK_8, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING7_STACK", 1, CORE_8_INDEX, &PROCESSING7_STACK_8, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_FLOW_TABLE", 1, CORE_8_INDEX, &RX_FLOW_TABLE_8, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_FLOW_TABLE", 1, CORE_8_INDEX, &TX_FLOW_TABLE_8, 212, 1, 1 },
#endif
#if defined BCM6885
	{ "PARSER_KEY_BUILD_TABLE", 1, CORE_8_INDEX, &PARSER_KEY_BUILD_TABLE_8, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_8_INDEX, &INGRESS_FILTER_PROFILE_TABLE_8, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "POLICER_PARAMS_TABLE", 1, CORE_9_INDEX, &POLICER_PARAMS_TABLE_9, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING0_STACK", 1, CORE_9_INDEX, &PROCESSING0_STACK_9, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "SYSTEM_CONFIGURATION", 1, CORE_9_INDEX, &SYSTEM_CONFIGURATION_9, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDTEST_GEN_PARAM", 1, CORE_9_INDEX, &SPDTEST_GEN_PARAM_9, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_CFG_TABLE", 1, CORE_9_INDEX, &VPORT_CFG_TABLE_9, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "MUTEX_TABLE", 1, CORE_9_INDEX, &MUTEX_TABLE_9, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_9_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_9, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING1_STACK", 1, CORE_9_INDEX, &PROCESSING1_STACK_9, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "REGISTERS_BUFFER", 1, CORE_9_INDEX, &REGISTERS_BUFFER_9, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "TUNNELS_PARSING_CFG", 1, CORE_9_INDEX, &TUNNELS_PARSING_CFG_9, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_9_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_9, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING2_STACK", 1, CORE_9_INDEX, &PROCESSING2_STACK_9, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_9_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_9, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_BAD_IPV4_HDR_CSUM_PACKETS", 1, CORE_9_INDEX, &CSO_BAD_IPV4_HDR_CSUM_PACKETS_9, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_9_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_9, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TASK_IDX", 1, CORE_9_INDEX, &TASK_IDX_9, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_9_INDEX, &LOOPBACK_QUEUE_TABLE_9, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DOS_DROP_REASONS_CFG", 1, CORE_9_INDEX, &DOS_DROP_REASONS_CFG_9, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_CFG_EX_TABLE", 1, CORE_9_INDEX, &VPORT_CFG_EX_TABLE_9, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_9_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_9, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING3_STACK", 1, CORE_9_INDEX, &PROCESSING3_STACK_9, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_CFG_TABLE", 1, CORE_9_INDEX, &IPTV_CFG_TABLE_9, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_FPM_THRESHOLDS", 1, CORE_9_INDEX, &DHD_FPM_THRESHOLDS_9, 3, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_9_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_9, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_9_INDEX, &LOOPBACK_WAN_FLOW_TABLE_9, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FORCE_DSCP", 1, CORE_9_INDEX, &FORCE_DSCP_9, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CORE_ID_TABLE", 1, CORE_9_INDEX, &CORE_ID_TABLE_9, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TR471_SPDSVC_RX_PKT_ID_TABLE", 1, CORE_9_INDEX, &TR471_SPDSVC_RX_PKT_ID_TABLE_9, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_TABLE", 1, CORE_9_INDEX, &DEBUG_PRINT_TABLE_9, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_9_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_9, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_9_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_9, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING4_STACK", 1, CORE_9_INDEX, &PROCESSING4_STACK_9, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "TCAM_GENERIC_FIELDS", 1, CORE_9_INDEX, &TCAM_GENERIC_FIELDS_9, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_9_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_9, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_9_INDEX, &BUFMNG_DESCRIPTOR_TABLE_9, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_STATUS_TABLE", 1, CORE_9_INDEX, &BUFMNG_STATUS_TABLE_9, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_9_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_9, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_9_INDEX, &DEBUG_PRINT_CORE_LOCK_9, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_HW_CFG", 1, CORE_9_INDEX, &DHD_HW_CFG_9, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_9_INDEX, &RX_MIRRORING_CONFIGURATION_9, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_9_INDEX, &TX_MIRRORING_CONFIGURATION_9, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "NAT_CACHE_CFG", 1, CORE_9_INDEX, &NAT_CACHE_CFG_9, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SRAM_DUMMY_STORE", 1, CORE_9_INDEX, &SRAM_DUMMY_STORE_9, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING5_STACK", 1, CORE_9_INDEX, &PROCESSING5_STACK_9, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "CHANNEL_ID_TO_SBPM_BB_ID_TABLE", 1, CORE_9_INDEX, &CHANNEL_ID_TO_SBPM_BB_ID_TABLE_9, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_9_INDEX, &NAT_CACHE_KEY0_MASK_9, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_DISABLE", 1, CORE_9_INDEX, &CSO_DISABLE_9, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_9_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_9, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_9_INDEX, &INGRESS_FILTER_1588_CFG_9, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_9_INDEX, &RX_MIRRORING_DIRECT_ENABLE_9, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_9_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_9, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_SCRATCHPAD", 1, CORE_9_INDEX, &DEBUG_SCRATCHPAD_9, 12, 1, 1 },
#endif
#if defined BCM6885
	{ "GDX_PARAMS_TABLE", 1, CORE_9_INDEX, &GDX_PARAMS_TABLE_9, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_GLOBAL_CFG", 1, CORE_9_INDEX, &FPM_GLOBAL_CFG_9, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_9_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER_9, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING6_STACK", 1, CORE_9_INDEX, &PROCESSING6_STACK_9, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_9_INDEX, &TCAM_TABLE_CFG_TABLE_9, 2, 1, 1 },
#endif
#if defined BCM6885
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_9_INDEX, &NATC_L2_VLAN_KEY_MASK_9, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_CFG", 1, CORE_9_INDEX, &INGRESS_FILTER_CFG_9, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_9_INDEX, &BITS_CALC_MASKS_TABLE_9, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "NATC_L2_TOS_MASK", 1, CORE_9_INDEX, &NATC_L2_TOS_MASK_9, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_9_INDEX, &FW_ERROR_VECTOR_TABLE_9, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "MULTICAST_KEY_MASK", 1, CORE_9_INDEX, &MULTICAST_KEY_MASK_9, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING7_STACK", 1, CORE_9_INDEX, &PROCESSING7_STACK_9, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_FLOW_TABLE", 1, CORE_9_INDEX, &RX_FLOW_TABLE_9, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_FLOW_TABLE", 1, CORE_9_INDEX, &TX_FLOW_TABLE_9, 212, 1, 1 },
#endif
#if defined BCM6885
	{ "PARSER_KEY_BUILD_TABLE", 1, CORE_9_INDEX, &PARSER_KEY_BUILD_TABLE_9, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_9_INDEX, &INGRESS_FILTER_PROFILE_TABLE_9, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "POLICER_PARAMS_TABLE", 1, CORE_10_INDEX, &POLICER_PARAMS_TABLE_10, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING0_STACK", 1, CORE_10_INDEX, &PROCESSING0_STACK_10, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "SYSTEM_CONFIGURATION", 1, CORE_10_INDEX, &SYSTEM_CONFIGURATION_10, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDTEST_GEN_PARAM", 1, CORE_10_INDEX, &SPDTEST_GEN_PARAM_10, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_CFG_TABLE", 1, CORE_10_INDEX, &VPORT_CFG_TABLE_10, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "MUTEX_TABLE", 1, CORE_10_INDEX, &MUTEX_TABLE_10, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_10_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_10, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING1_STACK", 1, CORE_10_INDEX, &PROCESSING1_STACK_10, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "REGISTERS_BUFFER", 1, CORE_10_INDEX, &REGISTERS_BUFFER_10, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "TUNNELS_PARSING_CFG", 1, CORE_10_INDEX, &TUNNELS_PARSING_CFG_10, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_10_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_10, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING2_STACK", 1, CORE_10_INDEX, &PROCESSING2_STACK_10, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_10_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_10, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_BAD_IPV4_HDR_CSUM_PACKETS", 1, CORE_10_INDEX, &CSO_BAD_IPV4_HDR_CSUM_PACKETS_10, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_10_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_10, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TASK_IDX", 1, CORE_10_INDEX, &TASK_IDX_10, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_10_INDEX, &LOOPBACK_QUEUE_TABLE_10, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DOS_DROP_REASONS_CFG", 1, CORE_10_INDEX, &DOS_DROP_REASONS_CFG_10, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_CFG_EX_TABLE", 1, CORE_10_INDEX, &VPORT_CFG_EX_TABLE_10, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_10_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_10, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING3_STACK", 1, CORE_10_INDEX, &PROCESSING3_STACK_10, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_CFG_TABLE", 1, CORE_10_INDEX, &IPTV_CFG_TABLE_10, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_FPM_THRESHOLDS", 1, CORE_10_INDEX, &DHD_FPM_THRESHOLDS_10, 3, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_10_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_10, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_10_INDEX, &LOOPBACK_WAN_FLOW_TABLE_10, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FORCE_DSCP", 1, CORE_10_INDEX, &FORCE_DSCP_10, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CORE_ID_TABLE", 1, CORE_10_INDEX, &CORE_ID_TABLE_10, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TR471_SPDSVC_RX_PKT_ID_TABLE", 1, CORE_10_INDEX, &TR471_SPDSVC_RX_PKT_ID_TABLE_10, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_TABLE", 1, CORE_10_INDEX, &DEBUG_PRINT_TABLE_10, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_10_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_10, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_10_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_10, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING4_STACK", 1, CORE_10_INDEX, &PROCESSING4_STACK_10, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "TCAM_GENERIC_FIELDS", 1, CORE_10_INDEX, &TCAM_GENERIC_FIELDS_10, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_10_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_10, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_10_INDEX, &BUFMNG_DESCRIPTOR_TABLE_10, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_STATUS_TABLE", 1, CORE_10_INDEX, &BUFMNG_STATUS_TABLE_10, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_10_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_10, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_10_INDEX, &DEBUG_PRINT_CORE_LOCK_10, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_HW_CFG", 1, CORE_10_INDEX, &DHD_HW_CFG_10, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_10_INDEX, &RX_MIRRORING_CONFIGURATION_10, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_10_INDEX, &TX_MIRRORING_CONFIGURATION_10, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "NAT_CACHE_CFG", 1, CORE_10_INDEX, &NAT_CACHE_CFG_10, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SRAM_DUMMY_STORE", 1, CORE_10_INDEX, &SRAM_DUMMY_STORE_10, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING5_STACK", 1, CORE_10_INDEX, &PROCESSING5_STACK_10, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "CHANNEL_ID_TO_SBPM_BB_ID_TABLE", 1, CORE_10_INDEX, &CHANNEL_ID_TO_SBPM_BB_ID_TABLE_10, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_10_INDEX, &NAT_CACHE_KEY0_MASK_10, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_DISABLE", 1, CORE_10_INDEX, &CSO_DISABLE_10, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_10_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_10, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_10_INDEX, &INGRESS_FILTER_1588_CFG_10, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_10_INDEX, &RX_MIRRORING_DIRECT_ENABLE_10, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_10_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_10, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_SCRATCHPAD", 1, CORE_10_INDEX, &DEBUG_SCRATCHPAD_10, 12, 1, 1 },
#endif
#if defined BCM6885
	{ "GDX_PARAMS_TABLE", 1, CORE_10_INDEX, &GDX_PARAMS_TABLE_10, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_GLOBAL_CFG", 1, CORE_10_INDEX, &FPM_GLOBAL_CFG_10, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_10_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER_10, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING6_STACK", 1, CORE_10_INDEX, &PROCESSING6_STACK_10, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_10_INDEX, &TCAM_TABLE_CFG_TABLE_10, 2, 1, 1 },
#endif
#if defined BCM6885
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_10_INDEX, &NATC_L2_VLAN_KEY_MASK_10, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_CFG", 1, CORE_10_INDEX, &INGRESS_FILTER_CFG_10, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_10_INDEX, &BITS_CALC_MASKS_TABLE_10, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "NATC_L2_TOS_MASK", 1, CORE_10_INDEX, &NATC_L2_TOS_MASK_10, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_10_INDEX, &FW_ERROR_VECTOR_TABLE_10, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "MULTICAST_KEY_MASK", 1, CORE_10_INDEX, &MULTICAST_KEY_MASK_10, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING7_STACK", 1, CORE_10_INDEX, &PROCESSING7_STACK_10, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_FLOW_TABLE", 1, CORE_10_INDEX, &RX_FLOW_TABLE_10, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_FLOW_TABLE", 1, CORE_10_INDEX, &TX_FLOW_TABLE_10, 212, 1, 1 },
#endif
#if defined BCM6885
	{ "PARSER_KEY_BUILD_TABLE", 1, CORE_10_INDEX, &PARSER_KEY_BUILD_TABLE_10, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_10_INDEX, &INGRESS_FILTER_PROFILE_TABLE_10, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "POLICER_PARAMS_TABLE", 1, CORE_11_INDEX, &POLICER_PARAMS_TABLE_11, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING0_STACK", 1, CORE_11_INDEX, &PROCESSING0_STACK_11, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "SYSTEM_CONFIGURATION", 1, CORE_11_INDEX, &SYSTEM_CONFIGURATION_11, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDTEST_GEN_PARAM", 1, CORE_11_INDEX, &SPDTEST_GEN_PARAM_11, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_CFG_TABLE", 1, CORE_11_INDEX, &VPORT_CFG_TABLE_11, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "MUTEX_TABLE", 1, CORE_11_INDEX, &MUTEX_TABLE_11, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_11_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_11, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING1_STACK", 1, CORE_11_INDEX, &PROCESSING1_STACK_11, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "REGISTERS_BUFFER", 1, CORE_11_INDEX, &REGISTERS_BUFFER_11, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "TUNNELS_PARSING_CFG", 1, CORE_11_INDEX, &TUNNELS_PARSING_CFG_11, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_11_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_11, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING2_STACK", 1, CORE_11_INDEX, &PROCESSING2_STACK_11, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_11_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_11, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_BAD_IPV4_HDR_CSUM_PACKETS", 1, CORE_11_INDEX, &CSO_BAD_IPV4_HDR_CSUM_PACKETS_11, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_11_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_11, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TASK_IDX", 1, CORE_11_INDEX, &TASK_IDX_11, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_11_INDEX, &LOOPBACK_QUEUE_TABLE_11, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DOS_DROP_REASONS_CFG", 1, CORE_11_INDEX, &DOS_DROP_REASONS_CFG_11, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_CFG_EX_TABLE", 1, CORE_11_INDEX, &VPORT_CFG_EX_TABLE_11, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_11_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_11, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING3_STACK", 1, CORE_11_INDEX, &PROCESSING3_STACK_11, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_CFG_TABLE", 1, CORE_11_INDEX, &IPTV_CFG_TABLE_11, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_FPM_THRESHOLDS", 1, CORE_11_INDEX, &DHD_FPM_THRESHOLDS_11, 3, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_11_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_11, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_11_INDEX, &LOOPBACK_WAN_FLOW_TABLE_11, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FORCE_DSCP", 1, CORE_11_INDEX, &FORCE_DSCP_11, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CORE_ID_TABLE", 1, CORE_11_INDEX, &CORE_ID_TABLE_11, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TR471_SPDSVC_RX_PKT_ID_TABLE", 1, CORE_11_INDEX, &TR471_SPDSVC_RX_PKT_ID_TABLE_11, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_TABLE", 1, CORE_11_INDEX, &DEBUG_PRINT_TABLE_11, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_11_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_11, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_11_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_11, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING4_STACK", 1, CORE_11_INDEX, &PROCESSING4_STACK_11, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "TCAM_GENERIC_FIELDS", 1, CORE_11_INDEX, &TCAM_GENERIC_FIELDS_11, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_11_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_11, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_11_INDEX, &BUFMNG_DESCRIPTOR_TABLE_11, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_STATUS_TABLE", 1, CORE_11_INDEX, &BUFMNG_STATUS_TABLE_11, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_11_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_11, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_11_INDEX, &DEBUG_PRINT_CORE_LOCK_11, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_HW_CFG", 1, CORE_11_INDEX, &DHD_HW_CFG_11, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_11_INDEX, &RX_MIRRORING_CONFIGURATION_11, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_11_INDEX, &TX_MIRRORING_CONFIGURATION_11, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "NAT_CACHE_CFG", 1, CORE_11_INDEX, &NAT_CACHE_CFG_11, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SRAM_DUMMY_STORE", 1, CORE_11_INDEX, &SRAM_DUMMY_STORE_11, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING5_STACK", 1, CORE_11_INDEX, &PROCESSING5_STACK_11, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "CHANNEL_ID_TO_SBPM_BB_ID_TABLE", 1, CORE_11_INDEX, &CHANNEL_ID_TO_SBPM_BB_ID_TABLE_11, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_11_INDEX, &NAT_CACHE_KEY0_MASK_11, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_DISABLE", 1, CORE_11_INDEX, &CSO_DISABLE_11, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_11_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_11, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_11_INDEX, &INGRESS_FILTER_1588_CFG_11, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_11_INDEX, &RX_MIRRORING_DIRECT_ENABLE_11, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_11_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_11, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_SCRATCHPAD", 1, CORE_11_INDEX, &DEBUG_SCRATCHPAD_11, 12, 1, 1 },
#endif
#if defined BCM6885
	{ "GDX_PARAMS_TABLE", 1, CORE_11_INDEX, &GDX_PARAMS_TABLE_11, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_GLOBAL_CFG", 1, CORE_11_INDEX, &FPM_GLOBAL_CFG_11, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_11_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER_11, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING6_STACK", 1, CORE_11_INDEX, &PROCESSING6_STACK_11, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_11_INDEX, &TCAM_TABLE_CFG_TABLE_11, 2, 1, 1 },
#endif
#if defined BCM6885
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_11_INDEX, &NATC_L2_VLAN_KEY_MASK_11, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_CFG", 1, CORE_11_INDEX, &INGRESS_FILTER_CFG_11, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_11_INDEX, &BITS_CALC_MASKS_TABLE_11, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "NATC_L2_TOS_MASK", 1, CORE_11_INDEX, &NATC_L2_TOS_MASK_11, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_11_INDEX, &FW_ERROR_VECTOR_TABLE_11, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "MULTICAST_KEY_MASK", 1, CORE_11_INDEX, &MULTICAST_KEY_MASK_11, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING7_STACK", 1, CORE_11_INDEX, &PROCESSING7_STACK_11, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_FLOW_TABLE", 1, CORE_11_INDEX, &RX_FLOW_TABLE_11, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_FLOW_TABLE", 1, CORE_11_INDEX, &TX_FLOW_TABLE_11, 212, 1, 1 },
#endif
#if defined BCM6885
	{ "PARSER_KEY_BUILD_TABLE", 1, CORE_11_INDEX, &PARSER_KEY_BUILD_TABLE_11, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_11_INDEX, &INGRESS_FILTER_PROFILE_TABLE_11, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "POLICER_PARAMS_TABLE", 1, CORE_12_INDEX, &POLICER_PARAMS_TABLE_12, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING0_STACK", 1, CORE_12_INDEX, &PROCESSING0_STACK_12, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "SYSTEM_CONFIGURATION", 1, CORE_12_INDEX, &SYSTEM_CONFIGURATION_12, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDTEST_GEN_PARAM", 1, CORE_12_INDEX, &SPDTEST_GEN_PARAM_12, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_CFG_TABLE", 1, CORE_12_INDEX, &VPORT_CFG_TABLE_12, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "MUTEX_TABLE", 1, CORE_12_INDEX, &MUTEX_TABLE_12, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_12_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_12, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING1_STACK", 1, CORE_12_INDEX, &PROCESSING1_STACK_12, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "REGISTERS_BUFFER", 1, CORE_12_INDEX, &REGISTERS_BUFFER_12, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "TUNNELS_PARSING_CFG", 1, CORE_12_INDEX, &TUNNELS_PARSING_CFG_12, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_12_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_12, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING2_STACK", 1, CORE_12_INDEX, &PROCESSING2_STACK_12, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_12_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_12, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_BAD_IPV4_HDR_CSUM_PACKETS", 1, CORE_12_INDEX, &CSO_BAD_IPV4_HDR_CSUM_PACKETS_12, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_12_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_12, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TASK_IDX", 1, CORE_12_INDEX, &TASK_IDX_12, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_12_INDEX, &LOOPBACK_QUEUE_TABLE_12, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DOS_DROP_REASONS_CFG", 1, CORE_12_INDEX, &DOS_DROP_REASONS_CFG_12, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_CFG_EX_TABLE", 1, CORE_12_INDEX, &VPORT_CFG_EX_TABLE_12, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_12_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_12, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING3_STACK", 1, CORE_12_INDEX, &PROCESSING3_STACK_12, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_CFG_TABLE", 1, CORE_12_INDEX, &IPTV_CFG_TABLE_12, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_FPM_THRESHOLDS", 1, CORE_12_INDEX, &DHD_FPM_THRESHOLDS_12, 3, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_12_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_12, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_12_INDEX, &LOOPBACK_WAN_FLOW_TABLE_12, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FORCE_DSCP", 1, CORE_12_INDEX, &FORCE_DSCP_12, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CORE_ID_TABLE", 1, CORE_12_INDEX, &CORE_ID_TABLE_12, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TR471_SPDSVC_RX_PKT_ID_TABLE", 1, CORE_12_INDEX, &TR471_SPDSVC_RX_PKT_ID_TABLE_12, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_TABLE", 1, CORE_12_INDEX, &DEBUG_PRINT_TABLE_12, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_12_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_12, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_12_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_12, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING4_STACK", 1, CORE_12_INDEX, &PROCESSING4_STACK_12, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "TCAM_GENERIC_FIELDS", 1, CORE_12_INDEX, &TCAM_GENERIC_FIELDS_12, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_12_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_12, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_12_INDEX, &BUFMNG_DESCRIPTOR_TABLE_12, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_STATUS_TABLE", 1, CORE_12_INDEX, &BUFMNG_STATUS_TABLE_12, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_12_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_12, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_12_INDEX, &DEBUG_PRINT_CORE_LOCK_12, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_HW_CFG", 1, CORE_12_INDEX, &DHD_HW_CFG_12, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_12_INDEX, &RX_MIRRORING_CONFIGURATION_12, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_12_INDEX, &TX_MIRRORING_CONFIGURATION_12, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "NAT_CACHE_CFG", 1, CORE_12_INDEX, &NAT_CACHE_CFG_12, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SRAM_DUMMY_STORE", 1, CORE_12_INDEX, &SRAM_DUMMY_STORE_12, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING5_STACK", 1, CORE_12_INDEX, &PROCESSING5_STACK_12, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "CHANNEL_ID_TO_SBPM_BB_ID_TABLE", 1, CORE_12_INDEX, &CHANNEL_ID_TO_SBPM_BB_ID_TABLE_12, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_12_INDEX, &NAT_CACHE_KEY0_MASK_12, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_DISABLE", 1, CORE_12_INDEX, &CSO_DISABLE_12, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_12_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_12, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_12_INDEX, &INGRESS_FILTER_1588_CFG_12, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_12_INDEX, &RX_MIRRORING_DIRECT_ENABLE_12, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_12_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_12, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_SCRATCHPAD", 1, CORE_12_INDEX, &DEBUG_SCRATCHPAD_12, 12, 1, 1 },
#endif
#if defined BCM6885
	{ "GDX_PARAMS_TABLE", 1, CORE_12_INDEX, &GDX_PARAMS_TABLE_12, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_GLOBAL_CFG", 1, CORE_12_INDEX, &FPM_GLOBAL_CFG_12, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_12_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER_12, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING6_STACK", 1, CORE_12_INDEX, &PROCESSING6_STACK_12, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_12_INDEX, &TCAM_TABLE_CFG_TABLE_12, 2, 1, 1 },
#endif
#if defined BCM6885
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_12_INDEX, &NATC_L2_VLAN_KEY_MASK_12, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_CFG", 1, CORE_12_INDEX, &INGRESS_FILTER_CFG_12, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_12_INDEX, &BITS_CALC_MASKS_TABLE_12, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "NATC_L2_TOS_MASK", 1, CORE_12_INDEX, &NATC_L2_TOS_MASK_12, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_12_INDEX, &FW_ERROR_VECTOR_TABLE_12, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "MULTICAST_KEY_MASK", 1, CORE_12_INDEX, &MULTICAST_KEY_MASK_12, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING7_STACK", 1, CORE_12_INDEX, &PROCESSING7_STACK_12, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_FLOW_TABLE", 1, CORE_12_INDEX, &RX_FLOW_TABLE_12, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_FLOW_TABLE", 1, CORE_12_INDEX, &TX_FLOW_TABLE_12, 212, 1, 1 },
#endif
#if defined BCM6885
	{ "PARSER_KEY_BUILD_TABLE", 1, CORE_12_INDEX, &PARSER_KEY_BUILD_TABLE_12, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_12_INDEX, &INGRESS_FILTER_PROFILE_TABLE_12, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "POLICER_PARAMS_TABLE", 1, CORE_13_INDEX, &POLICER_PARAMS_TABLE_13, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING0_STACK", 1, CORE_13_INDEX, &PROCESSING0_STACK_13, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "SYSTEM_CONFIGURATION", 1, CORE_13_INDEX, &SYSTEM_CONFIGURATION_13, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDTEST_GEN_PARAM", 1, CORE_13_INDEX, &SPDTEST_GEN_PARAM_13, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_CFG_TABLE", 1, CORE_13_INDEX, &VPORT_CFG_TABLE_13, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "MUTEX_TABLE", 1, CORE_13_INDEX, &MUTEX_TABLE_13, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_13_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_13, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING1_STACK", 1, CORE_13_INDEX, &PROCESSING1_STACK_13, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "REGISTERS_BUFFER", 1, CORE_13_INDEX, &REGISTERS_BUFFER_13, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "TUNNELS_PARSING_CFG", 1, CORE_13_INDEX, &TUNNELS_PARSING_CFG_13, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_13_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_13, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING2_STACK", 1, CORE_13_INDEX, &PROCESSING2_STACK_13, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_13_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_13, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_BAD_IPV4_HDR_CSUM_PACKETS", 1, CORE_13_INDEX, &CSO_BAD_IPV4_HDR_CSUM_PACKETS_13, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_13_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_13, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TASK_IDX", 1, CORE_13_INDEX, &TASK_IDX_13, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_13_INDEX, &LOOPBACK_QUEUE_TABLE_13, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DOS_DROP_REASONS_CFG", 1, CORE_13_INDEX, &DOS_DROP_REASONS_CFG_13, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_CFG_EX_TABLE", 1, CORE_13_INDEX, &VPORT_CFG_EX_TABLE_13, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_13_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_13, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING3_STACK", 1, CORE_13_INDEX, &PROCESSING3_STACK_13, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_CFG_TABLE", 1, CORE_13_INDEX, &IPTV_CFG_TABLE_13, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_FPM_THRESHOLDS", 1, CORE_13_INDEX, &DHD_FPM_THRESHOLDS_13, 3, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_13_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_13, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_13_INDEX, &LOOPBACK_WAN_FLOW_TABLE_13, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FORCE_DSCP", 1, CORE_13_INDEX, &FORCE_DSCP_13, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CORE_ID_TABLE", 1, CORE_13_INDEX, &CORE_ID_TABLE_13, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TR471_SPDSVC_RX_PKT_ID_TABLE", 1, CORE_13_INDEX, &TR471_SPDSVC_RX_PKT_ID_TABLE_13, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_TABLE", 1, CORE_13_INDEX, &DEBUG_PRINT_TABLE_13, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_13_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_13, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_13_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_13, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING4_STACK", 1, CORE_13_INDEX, &PROCESSING4_STACK_13, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "TCAM_GENERIC_FIELDS", 1, CORE_13_INDEX, &TCAM_GENERIC_FIELDS_13, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_13_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_13, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_13_INDEX, &BUFMNG_DESCRIPTOR_TABLE_13, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_STATUS_TABLE", 1, CORE_13_INDEX, &BUFMNG_STATUS_TABLE_13, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_13_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_13, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_13_INDEX, &DEBUG_PRINT_CORE_LOCK_13, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_HW_CFG", 1, CORE_13_INDEX, &DHD_HW_CFG_13, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_13_INDEX, &RX_MIRRORING_CONFIGURATION_13, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_13_INDEX, &TX_MIRRORING_CONFIGURATION_13, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "NAT_CACHE_CFG", 1, CORE_13_INDEX, &NAT_CACHE_CFG_13, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SRAM_DUMMY_STORE", 1, CORE_13_INDEX, &SRAM_DUMMY_STORE_13, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING5_STACK", 1, CORE_13_INDEX, &PROCESSING5_STACK_13, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "CHANNEL_ID_TO_SBPM_BB_ID_TABLE", 1, CORE_13_INDEX, &CHANNEL_ID_TO_SBPM_BB_ID_TABLE_13, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_13_INDEX, &NAT_CACHE_KEY0_MASK_13, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_DISABLE", 1, CORE_13_INDEX, &CSO_DISABLE_13, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_13_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_13, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_13_INDEX, &INGRESS_FILTER_1588_CFG_13, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_13_INDEX, &RX_MIRRORING_DIRECT_ENABLE_13, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_13_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_13, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_SCRATCHPAD", 1, CORE_13_INDEX, &DEBUG_SCRATCHPAD_13, 12, 1, 1 },
#endif
#if defined BCM6885
	{ "GDX_PARAMS_TABLE", 1, CORE_13_INDEX, &GDX_PARAMS_TABLE_13, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_GLOBAL_CFG", 1, CORE_13_INDEX, &FPM_GLOBAL_CFG_13, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_13_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER_13, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING6_STACK", 1, CORE_13_INDEX, &PROCESSING6_STACK_13, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_13_INDEX, &TCAM_TABLE_CFG_TABLE_13, 2, 1, 1 },
#endif
#if defined BCM6885
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_13_INDEX, &NATC_L2_VLAN_KEY_MASK_13, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_CFG", 1, CORE_13_INDEX, &INGRESS_FILTER_CFG_13, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_13_INDEX, &BITS_CALC_MASKS_TABLE_13, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "NATC_L2_TOS_MASK", 1, CORE_13_INDEX, &NATC_L2_TOS_MASK_13, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_13_INDEX, &FW_ERROR_VECTOR_TABLE_13, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "MULTICAST_KEY_MASK", 1, CORE_13_INDEX, &MULTICAST_KEY_MASK_13, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING7_STACK", 1, CORE_13_INDEX, &PROCESSING7_STACK_13, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_FLOW_TABLE", 1, CORE_13_INDEX, &RX_FLOW_TABLE_13, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_FLOW_TABLE", 1, CORE_13_INDEX, &TX_FLOW_TABLE_13, 212, 1, 1 },
#endif
#if defined BCM6885
	{ "PARSER_KEY_BUILD_TABLE", 1, CORE_13_INDEX, &PARSER_KEY_BUILD_TABLE_13, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_13_INDEX, &INGRESS_FILTER_PROFILE_TABLE_13, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "POLICER_PARAMS_TABLE", 1, CORE_14_INDEX, &POLICER_PARAMS_TABLE_14, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING0_STACK", 1, CORE_14_INDEX, &PROCESSING0_STACK_14, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "SYSTEM_CONFIGURATION", 1, CORE_14_INDEX, &SYSTEM_CONFIGURATION_14, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDTEST_GEN_PARAM", 1, CORE_14_INDEX, &SPDTEST_GEN_PARAM_14, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DIRECT_FLOW_STACK", 1, CORE_14_INDEX, &DIRECT_FLOW_STACK_14, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_14_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_14, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING1_STACK", 1, CORE_14_INDEX, &PROCESSING1_STACK_14, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "TUNNELS_PARSING_CFG", 1, CORE_14_INDEX, &TUNNELS_PARSING_CFG_14, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DIRECT_FLOW_PD_TABLE", 1, CORE_14_INDEX, &DIRECT_FLOW_PD_TABLE_14, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_CFG_TABLE", 1, CORE_14_INDEX, &VPORT_CFG_TABLE_14, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "MUTEX_TABLE", 1, CORE_14_INDEX, &MUTEX_TABLE_14, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING2_STACK", 1, CORE_14_INDEX, &PROCESSING2_STACK_14, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "REGISTERS_BUFFER", 1, CORE_14_INDEX, &REGISTERS_BUFFER_14, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_14_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_14, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_BAD_IPV4_HDR_CSUM_PACKETS", 1, CORE_14_INDEX, &CSO_BAD_IPV4_HDR_CSUM_PACKETS_14, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_14_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_14, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING3_STACK", 1, CORE_14_INDEX, &PROCESSING3_STACK_14, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_CFG_TABLE", 1, CORE_14_INDEX, &IPTV_CFG_TABLE_14, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_14_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_14, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TASK_IDX", 1, CORE_14_INDEX, &TASK_IDX_14, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_14_INDEX, &LOOPBACK_QUEUE_TABLE_14, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DOS_DROP_REASONS_CFG", 1, CORE_14_INDEX, &DOS_DROP_REASONS_CFG_14, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_CFG_EX_TABLE", 1, CORE_14_INDEX, &VPORT_CFG_EX_TABLE_14, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_14_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_14, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING4_STACK", 1, CORE_14_INDEX, &PROCESSING4_STACK_14, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_14_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_14, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DIRECT_FLOW_PAUSE_DEBUG", 1, CORE_14_INDEX, &DIRECT_FLOW_PAUSE_DEBUG_14, 3, 1, 1 },
#endif
#if defined BCM6885
	{ "DIRECT_FLOW_PAUSE_QUANTA", 1, CORE_14_INDEX, &DIRECT_FLOW_PAUSE_QUANTA_14, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DIRECT_FLOW_WAN_VIQ_EXCLUSIVE", 1, CORE_14_INDEX, &DIRECT_FLOW_WAN_VIQ_EXCLUSIVE_14, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_14_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_14, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TR471_SPDSVC_RX_PKT_ID_TABLE", 1, CORE_14_INDEX, &TR471_SPDSVC_RX_PKT_ID_TABLE_14, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_FPM_THRESHOLDS", 1, CORE_14_INDEX, &DHD_FPM_THRESHOLDS_14, 3, 1, 1 },
#endif
#if defined BCM6885
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_14_INDEX, &LOOPBACK_WAN_FLOW_TABLE_14, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FORCE_DSCP", 1, CORE_14_INDEX, &FORCE_DSCP_14, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CORE_ID_TABLE", 1, CORE_14_INDEX, &CORE_ID_TABLE_14, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "US_TM_TX_PAUSE_NACK", 1, CORE_14_INDEX, &US_TM_TX_PAUSE_NACK_14, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_14_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_14, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_14_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_14, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING5_STACK", 1, CORE_14_INDEX, &PROCESSING5_STACK_14, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "NAT_CACHE_CFG", 1, CORE_14_INDEX, &NAT_CACHE_CFG_14, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_14_INDEX, &DEBUG_PRINT_CORE_LOCK_14, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_TABLE", 1, CORE_14_INDEX, &DEBUG_PRINT_TABLE_14, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_14_INDEX, &BUFMNG_DESCRIPTOR_TABLE_14, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_STATUS_TABLE", 1, CORE_14_INDEX, &BUFMNG_STATUS_TABLE_14, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DIRECT_FLOW_CNTR_TABLE", 1, CORE_14_INDEX, &DIRECT_FLOW_CNTR_TABLE_14, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "DIRECT_FLOW_EPON_CONTROL_SCRATCH", 1, CORE_14_INDEX, &DIRECT_FLOW_EPON_CONTROL_SCRATCH_14, 22, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_14_INDEX, &RX_MIRRORING_CONFIGURATION_14, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_14_INDEX, &NAT_CACHE_KEY0_MASK_14, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_14_INDEX, &TX_MIRRORING_CONFIGURATION_14, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SRAM_DUMMY_STORE", 1, CORE_14_INDEX, &SRAM_DUMMY_STORE_14, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "MAC_TYPE", 1, CORE_14_INDEX, &MAC_TYPE_14, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_14_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER_14, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING6_STACK", 1, CORE_14_INDEX, &PROCESSING6_STACK_14, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "TCAM_GENERIC_FIELDS", 1, CORE_14_INDEX, &TCAM_GENERIC_FIELDS_14, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_14_INDEX, &TCAM_TABLE_CFG_TABLE_14, 2, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_DISABLE", 1, CORE_14_INDEX, &CSO_DISABLE_14, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_14_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_14, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_14_INDEX, &INGRESS_FILTER_1588_CFG_14, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_14_INDEX, &RX_MIRRORING_DIRECT_ENABLE_14, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_14_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_14, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_HW_CFG", 1, CORE_14_INDEX, &DHD_HW_CFG_14, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_14_INDEX, &NATC_L2_VLAN_KEY_MASK_14, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_14_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_14, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_GLOBAL_CFG", 1, CORE_14_INDEX, &FPM_GLOBAL_CFG_14, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING7_STACK", 1, CORE_14_INDEX, &PROCESSING7_STACK_14, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_SCRATCHPAD", 1, CORE_14_INDEX, &DEBUG_SCRATCHPAD_14, 12, 1, 1 },
#endif
#if defined BCM6885
	{ "CHANNEL_ID_TO_SBPM_BB_ID_TABLE", 1, CORE_14_INDEX, &CHANNEL_ID_TO_SBPM_BB_ID_TABLE_14, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_CFG", 1, CORE_14_INDEX, &INGRESS_FILTER_CFG_14, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GDX_PARAMS_TABLE", 1, CORE_14_INDEX, &GDX_PARAMS_TABLE_14, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_14_INDEX, &BITS_CALC_MASKS_TABLE_14, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "NATC_L2_TOS_MASK", 1, CORE_14_INDEX, &NATC_L2_TOS_MASK_14, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_14_INDEX, &FW_ERROR_VECTOR_TABLE_14, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_FLOW_TABLE", 1, CORE_14_INDEX, &RX_FLOW_TABLE_14, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_FLOW_TABLE", 1, CORE_14_INDEX, &TX_FLOW_TABLE_14, 212, 1, 1 },
#endif
#if defined BCM6885
	{ "MULTICAST_KEY_MASK", 1, CORE_14_INDEX, &MULTICAST_KEY_MASK_14, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PARSER_KEY_BUILD_TABLE", 1, CORE_14_INDEX, &PARSER_KEY_BUILD_TABLE_14, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DIRECT_FLOW_RX_MIRRORING_SCRATCHPAD", 1, CORE_14_INDEX, &DIRECT_FLOW_RX_MIRRORING_SCRATCHPAD_14, 136, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_14_INDEX, &INGRESS_FILTER_PROFILE_TABLE_14, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "POLICER_PARAMS_TABLE", 1, CORE_15_INDEX, &POLICER_PARAMS_TABLE_15, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING0_STACK", 1, CORE_15_INDEX, &PROCESSING0_STACK_15, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "SYSTEM_CONFIGURATION", 1, CORE_15_INDEX, &SYSTEM_CONFIGURATION_15, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDTEST_GEN_PARAM", 1, CORE_15_INDEX, &SPDTEST_GEN_PARAM_15, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_CFG_TABLE", 1, CORE_15_INDEX, &VPORT_CFG_TABLE_15, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "MUTEX_TABLE", 1, CORE_15_INDEX, &MUTEX_TABLE_15, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_15_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_15, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING1_STACK", 1, CORE_15_INDEX, &PROCESSING1_STACK_15, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "REGISTERS_BUFFER", 1, CORE_15_INDEX, &REGISTERS_BUFFER_15, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "TUNNELS_PARSING_CFG", 1, CORE_15_INDEX, &TUNNELS_PARSING_CFG_15, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_15_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_15, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING2_STACK", 1, CORE_15_INDEX, &PROCESSING2_STACK_15, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_15_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_15, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_BAD_IPV4_HDR_CSUM_PACKETS", 1, CORE_15_INDEX, &CSO_BAD_IPV4_HDR_CSUM_PACKETS_15, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_15_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_15, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TASK_IDX", 1, CORE_15_INDEX, &TASK_IDX_15, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_15_INDEX, &LOOPBACK_QUEUE_TABLE_15, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DOS_DROP_REASONS_CFG", 1, CORE_15_INDEX, &DOS_DROP_REASONS_CFG_15, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_CFG_EX_TABLE", 1, CORE_15_INDEX, &VPORT_CFG_EX_TABLE_15, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_15_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_15, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING3_STACK", 1, CORE_15_INDEX, &PROCESSING3_STACK_15, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_CFG_TABLE", 1, CORE_15_INDEX, &IPTV_CFG_TABLE_15, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_FPM_THRESHOLDS", 1, CORE_15_INDEX, &DHD_FPM_THRESHOLDS_15, 3, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_15_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_15, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_15_INDEX, &LOOPBACK_WAN_FLOW_TABLE_15, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FORCE_DSCP", 1, CORE_15_INDEX, &FORCE_DSCP_15, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CORE_ID_TABLE", 1, CORE_15_INDEX, &CORE_ID_TABLE_15, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TR471_SPDSVC_RX_PKT_ID_TABLE", 1, CORE_15_INDEX, &TR471_SPDSVC_RX_PKT_ID_TABLE_15, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_TABLE", 1, CORE_15_INDEX, &DEBUG_PRINT_TABLE_15, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_15_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_15, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_15_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_15, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING4_STACK", 1, CORE_15_INDEX, &PROCESSING4_STACK_15, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "TCAM_GENERIC_FIELDS", 1, CORE_15_INDEX, &TCAM_GENERIC_FIELDS_15, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_15_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_15, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_15_INDEX, &BUFMNG_DESCRIPTOR_TABLE_15, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_STATUS_TABLE", 1, CORE_15_INDEX, &BUFMNG_STATUS_TABLE_15, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_15_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_15, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_15_INDEX, &DEBUG_PRINT_CORE_LOCK_15, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_HW_CFG", 1, CORE_15_INDEX, &DHD_HW_CFG_15, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_15_INDEX, &RX_MIRRORING_CONFIGURATION_15, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_15_INDEX, &TX_MIRRORING_CONFIGURATION_15, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "NAT_CACHE_CFG", 1, CORE_15_INDEX, &NAT_CACHE_CFG_15, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SRAM_DUMMY_STORE", 1, CORE_15_INDEX, &SRAM_DUMMY_STORE_15, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING5_STACK", 1, CORE_15_INDEX, &PROCESSING5_STACK_15, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "CHANNEL_ID_TO_SBPM_BB_ID_TABLE", 1, CORE_15_INDEX, &CHANNEL_ID_TO_SBPM_BB_ID_TABLE_15, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_15_INDEX, &NAT_CACHE_KEY0_MASK_15, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_DISABLE", 1, CORE_15_INDEX, &CSO_DISABLE_15, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_15_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_15, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_15_INDEX, &INGRESS_FILTER_1588_CFG_15, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_15_INDEX, &RX_MIRRORING_DIRECT_ENABLE_15, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_15_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_15, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_SCRATCHPAD", 1, CORE_15_INDEX, &DEBUG_SCRATCHPAD_15, 12, 1, 1 },
#endif
#if defined BCM6885
	{ "GDX_PARAMS_TABLE", 1, CORE_15_INDEX, &GDX_PARAMS_TABLE_15, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_GLOBAL_CFG", 1, CORE_15_INDEX, &FPM_GLOBAL_CFG_15, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_15_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER_15, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING6_STACK", 1, CORE_15_INDEX, &PROCESSING6_STACK_15, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_15_INDEX, &TCAM_TABLE_CFG_TABLE_15, 2, 1, 1 },
#endif
#if defined BCM6885
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_15_INDEX, &NATC_L2_VLAN_KEY_MASK_15, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_CFG", 1, CORE_15_INDEX, &INGRESS_FILTER_CFG_15, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_15_INDEX, &BITS_CALC_MASKS_TABLE_15, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "NATC_L2_TOS_MASK", 1, CORE_15_INDEX, &NATC_L2_TOS_MASK_15, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_15_INDEX, &FW_ERROR_VECTOR_TABLE_15, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "MULTICAST_KEY_MASK", 1, CORE_15_INDEX, &MULTICAST_KEY_MASK_15, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING7_STACK", 1, CORE_15_INDEX, &PROCESSING7_STACK_15, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_FLOW_TABLE", 1, CORE_15_INDEX, &RX_FLOW_TABLE_15, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_FLOW_TABLE", 1, CORE_15_INDEX, &TX_FLOW_TABLE_15, 212, 1, 1 },
#endif
#if defined BCM6885
	{ "PARSER_KEY_BUILD_TABLE", 1, CORE_15_INDEX, &PARSER_KEY_BUILD_TABLE_15, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_15_INDEX, &INGRESS_FILTER_PROFILE_TABLE_15, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "TCPSPDTEST_STREAM_TABLE", 1, CORE_16_INDEX, &TCPSPDTEST_STREAM_TABLE_16, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_16_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_16, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "POLICER_PARAMS_TABLE", 1, CORE_16_INDEX, &POLICER_PARAMS_TABLE_16, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_16_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_16, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING0_STACK", 1, CORE_16_INDEX, &PROCESSING0_STACK_16, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "SYSTEM_CONFIGURATION", 1, CORE_16_INDEX, &SYSTEM_CONFIGURATION_16, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDTEST_GEN_PARAM", 1, CORE_16_INDEX, &SPDTEST_GEN_PARAM_16, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "TCPSPDTEST_SCRATCHPAD", 1, CORE_16_INDEX, &TCPSPDTEST_SCRATCHPAD_16, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING1_STACK", 1, CORE_16_INDEX, &PROCESSING1_STACK_16, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "TUNNELS_PARSING_CFG", 1, CORE_16_INDEX, &TUNNELS_PARSING_CFG_16, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDSVC_ANALYZER_PARAMS_TABLE", 1, CORE_16_INDEX, &SPDSVC_ANALYZER_PARAMS_TABLE_16, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TCPSPDTEST_UPLOAD_STACK", 1, CORE_16_INDEX, &TCPSPDTEST_UPLOAD_STACK_16, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING2_STACK", 1, CORE_16_INDEX, &PROCESSING2_STACK_16, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "MUTEX_TABLE", 1, CORE_16_INDEX, &MUTEX_TABLE_16, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_16_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_16, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_16_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_16, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_CFG_TABLE", 1, CORE_16_INDEX, &VPORT_CFG_TABLE_16, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_BAD_IPV4_HDR_CSUM_PACKETS", 1, CORE_16_INDEX, &CSO_BAD_IPV4_HDR_CSUM_PACKETS_16, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_16_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER_16, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING3_STACK", 1, CORE_16_INDEX, &PROCESSING3_STACK_16, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "REGISTERS_BUFFER", 1, CORE_16_INDEX, &REGISTERS_BUFFER_16, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_CFG_TABLE", 1, CORE_16_INDEX, &IPTV_CFG_TABLE_16, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TCPSPDTEST_DOWNLOAD_DISPATCHER_CREDIT_TABLE", 1, CORE_16_INDEX, &TCPSPDTEST_DOWNLOAD_DISPATCHER_CREDIT_TABLE_16, 3, 1, 1 },
#endif
#if defined BCM6885
	{ "TASK_IDX", 1, CORE_16_INDEX, &TASK_IDX_16, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING4_STACK", 1, CORE_16_INDEX, &PROCESSING4_STACK_16, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_16_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_16, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TCPSPDTEST_UPLOAD_DISPATCHER_CREDIT_TABLE", 1, CORE_16_INDEX, &TCPSPDTEST_UPLOAD_DISPATCHER_CREDIT_TABLE_16, 3, 1, 1 },
#endif
#if defined BCM6885
	{ "DOS_DROP_REASONS_CFG", 1, CORE_16_INDEX, &DOS_DROP_REASONS_CFG_16, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDSVC_TCPSPDTEST_COMMON_TABLE", 1, CORE_16_INDEX, &SPDSVC_TCPSPDTEST_COMMON_TABLE_16, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TCPSPDTEST_UPLOAD_TIMER_STACK", 1, CORE_16_INDEX, &TCPSPDTEST_UPLOAD_TIMER_STACK_16, 64, 1, 1 },
#endif
#if defined BCM6885
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_16_INDEX, &LOOPBACK_QUEUE_TABLE_16, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_16_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_16, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING5_STACK", 1, CORE_16_INDEX, &PROCESSING5_STACK_16, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "NAT_CACHE_CFG", 1, CORE_16_INDEX, &NAT_CACHE_CFG_16, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_16_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_16, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TCPSPDTEST_UPLOAD_TIMER_DISPATCHER_CREDIT_TABLE", 1, CORE_16_INDEX, &TCPSPDTEST_UPLOAD_TIMER_DISPATCHER_CREDIT_TABLE_16, 3, 1, 1 },
#endif
#if defined BCM6885
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_16_INDEX, &LOOPBACK_WAN_FLOW_TABLE_16, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FORCE_DSCP", 1, CORE_16_INDEX, &FORCE_DSCP_16, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CORE_ID_TABLE", 1, CORE_16_INDEX, &CORE_ID_TABLE_16, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_16_INDEX, &DEBUG_PRINT_CORE_LOCK_16, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TCPSPDTEST_DOWNLOAD_PD_FIFO_TABLE", 1, CORE_16_INDEX, &TCPSPDTEST_DOWNLOAD_PD_FIFO_TABLE_16, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_CFG_EX_TABLE", 1, CORE_16_INDEX, &VPORT_CFG_EX_TABLE_16, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_16_INDEX, &RX_MIRRORING_CONFIGURATION_16, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING6_STACK", 1, CORE_16_INDEX, &PROCESSING6_STACK_16, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_16_INDEX, &NAT_CACHE_KEY0_MASK_16, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_16_INDEX, &TX_MIRRORING_CONFIGURATION_16, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SRAM_DUMMY_STORE", 1, CORE_16_INDEX, &SRAM_DUMMY_STORE_16, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_DISABLE", 1, CORE_16_INDEX, &CSO_DISABLE_16, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_16_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_16, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_16_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_16, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_16_INDEX, &INGRESS_FILTER_1588_CFG_16, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_16_INDEX, &RX_MIRRORING_DIRECT_ENABLE_16, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TCPSPDTEST_UPLOAD_PD_FIFO_TABLE", 1, CORE_16_INDEX, &TCPSPDTEST_UPLOAD_PD_FIFO_TABLE_16, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDSVC_GEN_PARAMS_TABLE", 1, CORE_16_INDEX, &SPDSVC_GEN_PARAMS_TABLE_16, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING7_STACK", 1, CORE_16_INDEX, &PROCESSING7_STACK_16, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_16_INDEX, &TCAM_TABLE_CFG_TABLE_16, 2, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_FPM_THRESHOLDS", 1, CORE_16_INDEX, &DHD_FPM_THRESHOLDS_16, 3, 1, 1 },
#endif
#if defined BCM6885
	{ "TR471_SPDSVC_RX_PKT_ID_TABLE", 1, CORE_16_INDEX, &TR471_SPDSVC_RX_PKT_ID_TABLE_16, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "UDPSPDT_SCRATCH_TABLE", 1, CORE_16_INDEX, &UDPSPDT_SCRATCH_TABLE_16, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_16_INDEX, &NATC_L2_VLAN_KEY_MASK_16, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_16_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_16, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_16_INDEX, &BUFMNG_DESCRIPTOR_TABLE_16, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_FLOW_TABLE", 1, CORE_16_INDEX, &RX_FLOW_TABLE_16, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "TCPSPDTEST_DOWNLOAD_STACK", 1, CORE_16_INDEX, &TCPSPDTEST_DOWNLOAD_STACK_16, 240, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_TABLE", 1, CORE_16_INDEX, &DEBUG_PRINT_TABLE_16, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_FLOW_TABLE", 1, CORE_16_INDEX, &TX_FLOW_TABLE_16, 212, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_CFG", 1, CORE_16_INDEX, &INGRESS_FILTER_CFG_16, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_STATUS_TABLE", 1, CORE_16_INDEX, &BUFMNG_STATUS_TABLE_16, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDSVC_ANALYZER_STACK", 1, CORE_16_INDEX, &SPDSVC_ANALYZER_STACK_16, 176, 1, 1 },
#endif
#if defined BCM6885
	{ "TCAM_GENERIC_FIELDS", 1, CORE_16_INDEX, &TCAM_GENERIC_FIELDS_16, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDSVC_ANALYZER_UPDATE_FIFO_TABLE", 1, CORE_16_INDEX, &SPDSVC_ANALYZER_UPDATE_FIFO_TABLE_16, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "TCPSPDTEST_ENGINE_GLOBAL_TABLE", 1, CORE_16_INDEX, &TCPSPDTEST_ENGINE_GLOBAL_TABLE_16, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "NATC_L2_TOS_MASK", 1, CORE_16_INDEX, &NATC_L2_TOS_MASK_16, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "UDPSPDT_STREAM_RX_STAT_TABLE", 1, CORE_16_INDEX, &UDPSPDT_STREAM_RX_STAT_TABLE_16, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "TCPSPDTEST_DOWNLOAD_UPDATE_FIFO_TABLE", 1, CORE_16_INDEX, &TCPSPDTEST_DOWNLOAD_UPDATE_FIFO_TABLE_16, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_HW_CFG", 1, CORE_16_INDEX, &DHD_HW_CFG_16, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PARSER_KEY_BUILD_TABLE", 1, CORE_16_INDEX, &PARSER_KEY_BUILD_TABLE_16, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_SCRATCHPAD", 1, CORE_16_INDEX, &DEBUG_SCRATCHPAD_16, 12, 1, 1 },
#endif
#if defined BCM6885
	{ "TCPSPDTEST_UPLOAD_UPDATE_FIFO_TABLE", 1, CORE_16_INDEX, &TCPSPDTEST_UPLOAD_UPDATE_FIFO_TABLE_16, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_GLOBAL_CFG", 1, CORE_16_INDEX, &FPM_GLOBAL_CFG_16, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "UDPSPDT_STREAM_TX_STAT_TABLE", 1, CORE_16_INDEX, &UDPSPDT_STREAM_TX_STAT_TABLE_16, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "CHANNEL_ID_TO_SBPM_BB_ID_TABLE", 1, CORE_16_INDEX, &CHANNEL_ID_TO_SBPM_BB_ID_TABLE_16, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "GDX_PARAMS_TABLE", 1, CORE_16_INDEX, &GDX_PARAMS_TABLE_16, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_16_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_16, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_16_INDEX, &FW_ERROR_VECTOR_TABLE_16, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_16_INDEX, &INGRESS_FILTER_PROFILE_TABLE_16, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_16_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_16, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "MULTICAST_KEY_MASK", 1, CORE_16_INDEX, &MULTICAST_KEY_MASK_16, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_16_INDEX, &BITS_CALC_MASKS_TABLE_16, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "UDPSPDT_TX_PARAMS_TABLE", 1, CORE_16_INDEX, &UDPSPDT_TX_PARAMS_TABLE_16, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "CPU_TX_SCRATCHPAD", 1, CORE_17_INDEX, &CPU_TX_SCRATCHPAD_17, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "POLICER_PARAMS_TABLE", 1, CORE_17_INDEX, &POLICER_PARAMS_TABLE_17, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING0_STACK", 1, CORE_17_INDEX, &PROCESSING0_STACK_17, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "SYSTEM_CONFIGURATION", 1, CORE_17_INDEX, &SYSTEM_CONFIGURATION_17, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDTEST_GEN_PARAM", 1, CORE_17_INDEX, &SPDTEST_GEN_PARAM_17, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "GENERAL_TIMER", 1, CORE_17_INDEX, &GENERAL_TIMER_17, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_17_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_17, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING1_STACK", 1, CORE_17_INDEX, &PROCESSING1_STACK_17, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR", 1, CORE_17_INDEX, &CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_17, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFFER_ALLOC_REPLY", 1, CORE_17_INDEX, &BUFFER_ALLOC_REPLY_17, 2, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_CFG_TABLE", 1, CORE_17_INDEX, &VPORT_CFG_TABLE_17, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DDR_LATENCY_DBG_USEC", 1, CORE_17_INDEX, &DDR_LATENCY_DBG_USEC_17, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING2_STACK", 1, CORE_17_INDEX, &PROCESSING2_STACK_17, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "CPU_RX_INTERRUPT_ID_DDR_ADDR", 1, CORE_17_INDEX, &CPU_RX_INTERRUPT_ID_DDR_ADDR_17, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CPU_RECYCLE_INTERRUPT_COALESCING_TABLE", 1, CORE_17_INDEX, &CPU_RECYCLE_INTERRUPT_COALESCING_TABLE_17, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SG_DESC_TABLE", 1, CORE_17_INDEX, &SG_DESC_TABLE_17, 2, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_17_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_17, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING3_STACK", 1, CORE_17_INDEX, &PROCESSING3_STACK_17, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "CPU_TX_RING_INDICES_VALUES_TABLE", 1, CORE_17_INDEX, &CPU_TX_RING_INDICES_VALUES_TABLE_17, 2, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_17_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_17, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "GENERAL_TIMER_STACK", 1, CORE_17_INDEX, &GENERAL_TIMER_STACK_17, 72, 1, 1 },
#endif
#if defined BCM6885
	{ "DDR_LATENCY_DBG_USEC_MAX", 1, CORE_17_INDEX, &DDR_LATENCY_DBG_USEC_MAX_17, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "MUTEX_TABLE", 1, CORE_17_INDEX, &MUTEX_TABLE_17, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE", 1, CORE_17_INDEX, &CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE_17, 3, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_17_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_17, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_17_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_17, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_17_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER_17, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING4_STACK", 1, CORE_17_INDEX, &PROCESSING4_STACK_17, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "REGISTERS_BUFFER", 1, CORE_17_INDEX, &REGISTERS_BUFFER_17, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "TUNNELS_PARSING_CFG", 1, CORE_17_INDEX, &TUNNELS_PARSING_CFG_17, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CPU_TX_INGRESS_DISPATCHER_CREDIT_TABLE", 1, CORE_17_INDEX, &CPU_TX_INGRESS_DISPATCHER_CREDIT_TABLE_17, 3, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_BAD_IPV4_HDR_CSUM_PACKETS", 1, CORE_17_INDEX, &CSO_BAD_IPV4_HDR_CSUM_PACKETS_17, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING5_STACK", 1, CORE_17_INDEX, &PROCESSING5_STACK_17, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE", 1, CORE_17_INDEX, &CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE_17, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_17_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_17, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CPU_INT_INTERRUPT_SCRATCH", 1, CORE_17_INDEX, &CPU_INT_INTERRUPT_SCRATCH_17, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_17_INDEX, &LOOPBACK_QUEUE_TABLE_17, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DOS_DROP_REASONS_CFG", 1, CORE_17_INDEX, &DOS_DROP_REASONS_CFG_17, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_CFG_EX_TABLE", 1, CORE_17_INDEX, &VPORT_CFG_EX_TABLE_17, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "GENERAL_TIMER_ACTION_VEC", 1, CORE_17_INDEX, &GENERAL_TIMER_ACTION_VEC_17, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING6_STACK", 1, CORE_17_INDEX, &PROCESSING6_STACK_17, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_CFG_TABLE", 1, CORE_17_INDEX, &IPTV_CFG_TABLE_17, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_FPM_THRESHOLDS", 1, CORE_17_INDEX, &DHD_FPM_THRESHOLDS_17, 3, 1, 1 },
#endif
#if defined BCM6885
	{ "TASK_IDX", 1, CORE_17_INDEX, &TASK_IDX_17, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "QUEUE_THRESHOLD_VECTOR", 1, CORE_17_INDEX, &QUEUE_THRESHOLD_VECTOR_17, 14, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_17_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_17, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TR471_SPDSVC_RX_PKT_ID_TABLE", 1, CORE_17_INDEX, &TR471_SPDSVC_RX_PKT_ID_TABLE_17, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_TABLE", 1, CORE_17_INDEX, &DEBUG_PRINT_TABLE_17, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING7_STACK", 1, CORE_17_INDEX, &PROCESSING7_STACK_17, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "TCAM_GENERIC_FIELDS", 1, CORE_17_INDEX, &TCAM_GENERIC_FIELDS_17, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "NAT_CACHE_CFG", 1, CORE_17_INDEX, &NAT_CACHE_CFG_17, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_17_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_17, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RING_CPU_TX_DESCRIPTOR_DATA_TABLE", 1, CORE_17_INDEX, &RING_CPU_TX_DESCRIPTOR_DATA_TABLE_17, 2, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_17_INDEX, &BUFMNG_DESCRIPTOR_TABLE_17, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_STATUS_TABLE", 1, CORE_17_INDEX, &BUFMNG_STATUS_TABLE_17, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CPU_RECYCLE_STACK", 1, CORE_17_INDEX, &CPU_RECYCLE_STACK_17, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_FLOW_TABLE", 1, CORE_17_INDEX, &RX_FLOW_TABLE_17, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "CPU_TX_0_STACK", 1, CORE_17_INDEX, &CPU_TX_0_STACK_17, 224, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_HW_CFG", 1, CORE_17_INDEX, &DHD_HW_CFG_17, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_17_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_17, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_17_INDEX, &LOOPBACK_WAN_FLOW_TABLE_17, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FORCE_DSCP", 1, CORE_17_INDEX, &FORCE_DSCP_17, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CPU_RECYCLE_SHADOW_RD_IDX", 1, CORE_17_INDEX, &CPU_RECYCLE_SHADOW_RD_IDX_17, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CORE_ID_TABLE", 1, CORE_17_INDEX, &CORE_ID_TABLE_17, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_17_INDEX, &DEBUG_PRINT_CORE_LOCK_17, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_17_INDEX, &RX_MIRRORING_CONFIGURATION_17, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_17_INDEX, &TX_MIRRORING_CONFIGURATION_17, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CPU_RECYCLE_SRAM_PD_FIFO", 1, CORE_17_INDEX, &CPU_RECYCLE_SRAM_PD_FIFO_17, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "CPU_TX_1_STACK", 1, CORE_17_INDEX, &CPU_TX_1_STACK_17, 224, 1, 1 },
#endif
#if defined BCM6885
	{ "CPU_TX_RING_DESCRIPTOR_TABLE", 1, CORE_17_INDEX, &CPU_TX_RING_DESCRIPTOR_TABLE_17, 2, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_FLOW_TABLE", 1, CORE_17_INDEX, &TX_FLOW_TABLE_17, 212, 1, 1 },
#endif
#if defined BCM6885
	{ "SRAM_DUMMY_STORE", 1, CORE_17_INDEX, &SRAM_DUMMY_STORE_17, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_DISABLE", 1, CORE_17_INDEX, &CSO_DISABLE_17, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_17_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_17, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_17_INDEX, &INGRESS_FILTER_1588_CFG_17, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CPU_RECYCLE_SHADOW_WR_IDX", 1, CORE_17_INDEX, &CPU_RECYCLE_SHADOW_WR_IDX_17, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_17_INDEX, &RX_MIRRORING_DIRECT_ENABLE_17, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_GLOBAL_CFG", 1, CORE_17_INDEX, &FPM_GLOBAL_CFG_17, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PARSER_KEY_BUILD_TABLE", 1, CORE_17_INDEX, &PARSER_KEY_BUILD_TABLE_17, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_SCRATCHPAD", 1, CORE_17_INDEX, &DEBUG_SCRATCHPAD_17, 12, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_17_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_17, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "CHANNEL_ID_TO_SBPM_BB_ID_TABLE", 1, CORE_17_INDEX, &CHANNEL_ID_TO_SBPM_BB_ID_TABLE_17, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "CPU_RECYCLE_RING_DESCRIPTOR_TABLE", 1, CORE_17_INDEX, &CPU_RECYCLE_RING_DESCRIPTOR_TABLE_17, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SG_CONTEXT_TABLE", 1, CORE_17_INDEX, &SG_CONTEXT_TABLE_17, 2, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_17_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_17, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE", 1, CORE_17_INDEX, &CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE_17, 2, 1, 1 },
#endif
#if defined BCM6885
	{ "CPU_RECYCLE_INTERRUPT_SCRATCH", 1, CORE_17_INDEX, &CPU_RECYCLE_INTERRUPT_SCRATCH_17, 2, 1, 1 },
#endif
#if defined BCM6885
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_17_INDEX, &NAT_CACHE_KEY0_MASK_17, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_17_INDEX, &BITS_CALC_MASKS_TABLE_17, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "GDX_PARAMS_TABLE", 1, CORE_17_INDEX, &GDX_PARAMS_TABLE_17, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_17_INDEX, &FW_ERROR_VECTOR_TABLE_17, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_17_INDEX, &TCAM_TABLE_CFG_TABLE_17, 2, 1, 1 },
#endif
#if defined BCM6885
	{ "CPU_RECYCLE_NEXT_PTR_TABLE", 1, CORE_17_INDEX, &CPU_RECYCLE_NEXT_PTR_TABLE_17, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_17_INDEX, &NATC_L2_VLAN_KEY_MASK_17, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_17_INDEX, &INGRESS_FILTER_PROFILE_TABLE_17, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "CPU_TX_SYNC_FIFO_TABLE", 1, CORE_17_INDEX, &CPU_TX_SYNC_FIFO_TABLE_17, 2, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_CFG", 1, CORE_17_INDEX, &INGRESS_FILTER_CFG_17, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "NATC_L2_TOS_MASK", 1, CORE_17_INDEX, &NATC_L2_TOS_MASK_17, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "MULTICAST_KEY_MASK", 1, CORE_17_INDEX, &MULTICAST_KEY_MASK_17, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_COMPLETE_COMMON_RADIO_DATA", 1, CORE_18_INDEX, &DHD_COMPLETE_COMMON_RADIO_DATA_18, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_FLOW_TABLE", 1, CORE_18_INDEX, &RX_FLOW_TABLE_18, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "POLICER_PARAMS_TABLE", 1, CORE_18_INDEX, &POLICER_PARAMS_TABLE_18, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_18_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_18, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING0_STACK", 1, CORE_18_INDEX, &PROCESSING0_STACK_18, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "SYSTEM_CONFIGURATION", 1, CORE_18_INDEX, &SYSTEM_CONFIGURATION_18, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDTEST_GEN_PARAM", 1, CORE_18_INDEX, &SPDTEST_GEN_PARAM_18, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_RX_COMPLETE_FLOW_RING_BUFFER", 1, CORE_18_INDEX, &DHD_RX_COMPLETE_FLOW_RING_BUFFER_18, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING1_STACK", 1, CORE_18_INDEX, &PROCESSING1_STACK_18, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR", 1, CORE_18_INDEX, &DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_18, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_DOORBELL_TX_COMPLETE_VALUE", 1, CORE_18_INDEX, &DHD_DOORBELL_TX_COMPLETE_VALUE_18, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_RX_POST_FLOW_RING_BUFFER", 1, CORE_18_INDEX, &DHD_RX_POST_FLOW_RING_BUFFER_18, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING2_STACK", 1, CORE_18_INDEX, &PROCESSING2_STACK_18, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_RX_POST_RING_SIZE", 1, CORE_18_INDEX, &DHD_RX_POST_RING_SIZE_18, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_DOORBELL_RX_COMPLETE_VALUE", 1, CORE_18_INDEX, &DHD_DOORBELL_RX_COMPLETE_VALUE_18, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_RX_COMPLETE_0_STACK", 1, CORE_18_INDEX, &DHD_RX_COMPLETE_0_STACK_18, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_18_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER_18, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING3_STACK", 1, CORE_18_INDEX, &PROCESSING3_STACK_18, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_RX_COMPLETE_RING_SIZE", 1, CORE_18_INDEX, &DHD_RX_COMPLETE_RING_SIZE_18, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_DOORBELL_RX_POST_VALUE", 1, CORE_18_INDEX, &DHD_DOORBELL_RX_POST_VALUE_18, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_RX_COMPLETE_1_STACK", 1, CORE_18_INDEX, &DHD_RX_COMPLETE_1_STACK_18, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING4_STACK", 1, CORE_18_INDEX, &PROCESSING4_STACK_18, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_TX_COMPLETE_RING_SIZE", 1, CORE_18_INDEX, &DHD_TX_COMPLETE_RING_SIZE_18, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_CPU_INT_ID", 1, CORE_18_INDEX, &DHD_CPU_INT_ID_18, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_RX_COMPLETE_2_STACK", 1, CORE_18_INDEX, &DHD_RX_COMPLETE_2_STACK_18, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING5_STACK", 1, CORE_18_INDEX, &PROCESSING5_STACK_18, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "TUNNELS_PARSING_CFG", 1, CORE_18_INDEX, &TUNNELS_PARSING_CFG_18, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_PCAP_CFG", 1, CORE_18_INDEX, &DHD_PCAP_CFG_18, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_CFG_TABLE", 1, CORE_18_INDEX, &VPORT_CFG_TABLE_18, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "MUTEX_TABLE", 1, CORE_18_INDEX, &MUTEX_TABLE_18, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING6_STACK", 1, CORE_18_INDEX, &PROCESSING6_STACK_18, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "REGISTERS_BUFFER", 1, CORE_18_INDEX, &REGISTERS_BUFFER_18, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_MIRRORING_MINIFPM_PARAMS", 1, CORE_18_INDEX, &DHD_MIRRORING_MINIFPM_PARAMS_18, 2, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_18_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_18, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING7_STACK", 1, CORE_18_INDEX, &PROCESSING7_STACK_18, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_18_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_18, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_BAD_IPV4_HDR_CSUM_PACKETS", 1, CORE_18_INDEX, &CSO_BAD_IPV4_HDR_CSUM_PACKETS_18, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_18_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_18, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TASK_IDX", 1, CORE_18_INDEX, &TASK_IDX_18, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_PCAP_TX_RING_DESCRIPTOR_TABLE", 1, CORE_18_INDEX, &DHD_PCAP_TX_RING_DESCRIPTOR_TABLE_18, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_RX_COMPLETE_DISPATCHER_CREDIT_TABLE", 1, CORE_18_INDEX, &DHD_RX_COMPLETE_DISPATCHER_CREDIT_TABLE_18, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_MONITOR_PKT_INFO_TABLE", 1, CORE_18_INDEX, &DHD_MONITOR_PKT_INFO_TABLE_18, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_18_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_18, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_FLOW_TABLE", 1, CORE_18_INDEX, &TX_FLOW_TABLE_18, 212, 1, 1 },
#endif
#if defined BCM6885
	{ "DOS_DROP_REASONS_CFG", 1, CORE_18_INDEX, &DOS_DROP_REASONS_CFG_18, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_18_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_18, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_CFG_TABLE", 1, CORE_18_INDEX, &IPTV_CFG_TABLE_18, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_18_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_18, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "PARSER_KEY_BUILD_TABLE", 1, CORE_18_INDEX, &PARSER_KEY_BUILD_TABLE_18, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_FPM_THRESHOLDS", 1, CORE_18_INDEX, &DHD_FPM_THRESHOLDS_18, 3, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_18_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_18, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_18_INDEX, &LOOPBACK_WAN_FLOW_TABLE_18, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FORCE_DSCP", 1, CORE_18_INDEX, &FORCE_DSCP_18, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CORE_ID_TABLE", 1, CORE_18_INDEX, &CORE_ID_TABLE_18, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_18_INDEX, &BUFMNG_DESCRIPTOR_TABLE_18, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_TX_COMPLETE_DISPATCHER_CREDIT_TABLE", 1, CORE_18_INDEX, &DHD_TX_COMPLETE_DISPATCHER_CREDIT_TABLE_18, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_18_INDEX, &INGRESS_FILTER_PROFILE_TABLE_18, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_PCAP_RX_RING_DESCRIPTOR_TABLE", 1, CORE_18_INDEX, &DHD_PCAP_RX_RING_DESCRIPTOR_TABLE_18, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_TX_COMPLETE_0_STACK", 1, CORE_18_INDEX, &DHD_TX_COMPLETE_0_STACK_18, 64, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE", 1, CORE_18_INDEX, &DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE_18, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_TX_COMPLETE_1_STACK", 1, CORE_18_INDEX, &DHD_TX_COMPLETE_1_STACK_18, 64, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_TX_COMPLETE_FLOW_RING_BUFFER", 1, CORE_18_INDEX, &DHD_TX_COMPLETE_FLOW_RING_BUFFER_18, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_TX_COMPLETE_2_STACK", 1, CORE_18_INDEX, &DHD_TX_COMPLETE_2_STACK_18, 64, 1, 1 },
#endif
#if defined BCM6885
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_18_INDEX, &LOOPBACK_QUEUE_TABLE_18, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_18_INDEX, &RX_MIRRORING_CONFIGURATION_18, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_CFG_EX_TABLE", 1, CORE_18_INDEX, &VPORT_CFG_EX_TABLE_18, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_18_INDEX, &TX_MIRRORING_CONFIGURATION_18, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TR471_SPDSVC_RX_PKT_ID_TABLE", 1, CORE_18_INDEX, &TR471_SPDSVC_RX_PKT_ID_TABLE_18, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_TABLE", 1, CORE_18_INDEX, &DEBUG_PRINT_TABLE_18, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_STATUS_TABLE", 1, CORE_18_INDEX, &BUFMNG_STATUS_TABLE_18, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_HW_CFG", 1, CORE_18_INDEX, &DHD_HW_CFG_18, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_18_INDEX, &DEBUG_PRINT_CORE_LOCK_18, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SRAM_DUMMY_STORE", 1, CORE_18_INDEX, &SRAM_DUMMY_STORE_18, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_DISABLE", 1, CORE_18_INDEX, &CSO_DISABLE_18, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_18_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_18, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_18_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_18, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_FPM_REPLY", 1, CORE_18_INDEX, &DHD_FPM_REPLY_18, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_GLOBAL_CFG", 1, CORE_18_INDEX, &FPM_GLOBAL_CFG_18, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_18_INDEX, &INGRESS_FILTER_1588_CFG_18, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_18_INDEX, &RX_MIRRORING_DIRECT_ENABLE_18, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_18_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_18, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "TCAM_GENERIC_FIELDS", 1, CORE_18_INDEX, &TCAM_GENERIC_FIELDS_18, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "CHANNEL_ID_TO_SBPM_BB_ID_TABLE", 1, CORE_18_INDEX, &CHANNEL_ID_TO_SBPM_BB_ID_TABLE_18, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_18_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_18, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_SCRATCHPAD", 1, CORE_18_INDEX, &DEBUG_SCRATCHPAD_18, 12, 1, 1 },
#endif
#if defined BCM6885
	{ "GDX_PARAMS_TABLE", 1, CORE_18_INDEX, &GDX_PARAMS_TABLE_18, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_18_INDEX, &FW_ERROR_VECTOR_TABLE_18, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "NAT_CACHE_CFG", 1, CORE_18_INDEX, &NAT_CACHE_CFG_18, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_18_INDEX, &NAT_CACHE_KEY0_MASK_18, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_18_INDEX, &TCAM_TABLE_CFG_TABLE_18, 2, 1, 1 },
#endif
#if defined BCM6885
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_18_INDEX, &BITS_CALC_MASKS_TABLE_18, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_18_INDEX, &NATC_L2_VLAN_KEY_MASK_18, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_CFG", 1, CORE_18_INDEX, &INGRESS_FILTER_CFG_18, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "MULTICAST_KEY_MASK", 1, CORE_18_INDEX, &MULTICAST_KEY_MASK_18, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "NATC_L2_TOS_MASK", 1, CORE_18_INDEX, &NATC_L2_TOS_MASK_18, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_COMPLETE_COMMON_RADIO_DATA", 1, CORE_19_INDEX, &DHD_COMPLETE_COMMON_RADIO_DATA_19, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_FLOW_TABLE", 1, CORE_19_INDEX, &RX_FLOW_TABLE_19, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "POLICER_PARAMS_TABLE", 1, CORE_19_INDEX, &POLICER_PARAMS_TABLE_19, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_19_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_19, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING0_STACK", 1, CORE_19_INDEX, &PROCESSING0_STACK_19, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "SYSTEM_CONFIGURATION", 1, CORE_19_INDEX, &SYSTEM_CONFIGURATION_19, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDTEST_GEN_PARAM", 1, CORE_19_INDEX, &SPDTEST_GEN_PARAM_19, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_RX_COMPLETE_FLOW_RING_BUFFER", 1, CORE_19_INDEX, &DHD_RX_COMPLETE_FLOW_RING_BUFFER_19, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING1_STACK", 1, CORE_19_INDEX, &PROCESSING1_STACK_19, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR", 1, CORE_19_INDEX, &DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_19, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_DOORBELL_TX_COMPLETE_VALUE", 1, CORE_19_INDEX, &DHD_DOORBELL_TX_COMPLETE_VALUE_19, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_RX_POST_FLOW_RING_BUFFER", 1, CORE_19_INDEX, &DHD_RX_POST_FLOW_RING_BUFFER_19, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING2_STACK", 1, CORE_19_INDEX, &PROCESSING2_STACK_19, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_RX_POST_RING_SIZE", 1, CORE_19_INDEX, &DHD_RX_POST_RING_SIZE_19, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_DOORBELL_RX_COMPLETE_VALUE", 1, CORE_19_INDEX, &DHD_DOORBELL_RX_COMPLETE_VALUE_19, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_RX_COMPLETE_0_STACK", 1, CORE_19_INDEX, &DHD_RX_COMPLETE_0_STACK_19, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_19_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER_19, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING3_STACK", 1, CORE_19_INDEX, &PROCESSING3_STACK_19, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_RX_COMPLETE_RING_SIZE", 1, CORE_19_INDEX, &DHD_RX_COMPLETE_RING_SIZE_19, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_DOORBELL_RX_POST_VALUE", 1, CORE_19_INDEX, &DHD_DOORBELL_RX_POST_VALUE_19, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_RX_COMPLETE_1_STACK", 1, CORE_19_INDEX, &DHD_RX_COMPLETE_1_STACK_19, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING4_STACK", 1, CORE_19_INDEX, &PROCESSING4_STACK_19, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_TX_COMPLETE_RING_SIZE", 1, CORE_19_INDEX, &DHD_TX_COMPLETE_RING_SIZE_19, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_CPU_INT_ID", 1, CORE_19_INDEX, &DHD_CPU_INT_ID_19, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_RX_COMPLETE_2_STACK", 1, CORE_19_INDEX, &DHD_RX_COMPLETE_2_STACK_19, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING5_STACK", 1, CORE_19_INDEX, &PROCESSING5_STACK_19, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "TUNNELS_PARSING_CFG", 1, CORE_19_INDEX, &TUNNELS_PARSING_CFG_19, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_PCAP_CFG", 1, CORE_19_INDEX, &DHD_PCAP_CFG_19, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_CFG_TABLE", 1, CORE_19_INDEX, &VPORT_CFG_TABLE_19, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "MUTEX_TABLE", 1, CORE_19_INDEX, &MUTEX_TABLE_19, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING6_STACK", 1, CORE_19_INDEX, &PROCESSING6_STACK_19, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "REGISTERS_BUFFER", 1, CORE_19_INDEX, &REGISTERS_BUFFER_19, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_MIRRORING_MINIFPM_PARAMS", 1, CORE_19_INDEX, &DHD_MIRRORING_MINIFPM_PARAMS_19, 2, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_19_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_19, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING7_STACK", 1, CORE_19_INDEX, &PROCESSING7_STACK_19, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_19_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_19, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_BAD_IPV4_HDR_CSUM_PACKETS", 1, CORE_19_INDEX, &CSO_BAD_IPV4_HDR_CSUM_PACKETS_19, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_19_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_19, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TASK_IDX", 1, CORE_19_INDEX, &TASK_IDX_19, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_PCAP_TX_RING_DESCRIPTOR_TABLE", 1, CORE_19_INDEX, &DHD_PCAP_TX_RING_DESCRIPTOR_TABLE_19, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_RX_COMPLETE_DISPATCHER_CREDIT_TABLE", 1, CORE_19_INDEX, &DHD_RX_COMPLETE_DISPATCHER_CREDIT_TABLE_19, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_MONITOR_PKT_INFO_TABLE", 1, CORE_19_INDEX, &DHD_MONITOR_PKT_INFO_TABLE_19, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_19_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_19, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_FLOW_TABLE", 1, CORE_19_INDEX, &TX_FLOW_TABLE_19, 212, 1, 1 },
#endif
#if defined BCM6885
	{ "DOS_DROP_REASONS_CFG", 1, CORE_19_INDEX, &DOS_DROP_REASONS_CFG_19, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_19_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_19, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_CFG_TABLE", 1, CORE_19_INDEX, &IPTV_CFG_TABLE_19, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_19_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_19, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "PARSER_KEY_BUILD_TABLE", 1, CORE_19_INDEX, &PARSER_KEY_BUILD_TABLE_19, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_FPM_THRESHOLDS", 1, CORE_19_INDEX, &DHD_FPM_THRESHOLDS_19, 3, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_19_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_19, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_19_INDEX, &LOOPBACK_WAN_FLOW_TABLE_19, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FORCE_DSCP", 1, CORE_19_INDEX, &FORCE_DSCP_19, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CORE_ID_TABLE", 1, CORE_19_INDEX, &CORE_ID_TABLE_19, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_19_INDEX, &BUFMNG_DESCRIPTOR_TABLE_19, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_TX_COMPLETE_DISPATCHER_CREDIT_TABLE", 1, CORE_19_INDEX, &DHD_TX_COMPLETE_DISPATCHER_CREDIT_TABLE_19, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_19_INDEX, &INGRESS_FILTER_PROFILE_TABLE_19, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_PCAP_RX_RING_DESCRIPTOR_TABLE", 1, CORE_19_INDEX, &DHD_PCAP_RX_RING_DESCRIPTOR_TABLE_19, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_TX_COMPLETE_0_STACK", 1, CORE_19_INDEX, &DHD_TX_COMPLETE_0_STACK_19, 64, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE", 1, CORE_19_INDEX, &DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE_19, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_TX_COMPLETE_1_STACK", 1, CORE_19_INDEX, &DHD_TX_COMPLETE_1_STACK_19, 64, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_TX_COMPLETE_FLOW_RING_BUFFER", 1, CORE_19_INDEX, &DHD_TX_COMPLETE_FLOW_RING_BUFFER_19, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_TX_COMPLETE_2_STACK", 1, CORE_19_INDEX, &DHD_TX_COMPLETE_2_STACK_19, 64, 1, 1 },
#endif
#if defined BCM6885
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_19_INDEX, &LOOPBACK_QUEUE_TABLE_19, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_19_INDEX, &RX_MIRRORING_CONFIGURATION_19, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_CFG_EX_TABLE", 1, CORE_19_INDEX, &VPORT_CFG_EX_TABLE_19, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_19_INDEX, &TX_MIRRORING_CONFIGURATION_19, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TR471_SPDSVC_RX_PKT_ID_TABLE", 1, CORE_19_INDEX, &TR471_SPDSVC_RX_PKT_ID_TABLE_19, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_TABLE", 1, CORE_19_INDEX, &DEBUG_PRINT_TABLE_19, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_STATUS_TABLE", 1, CORE_19_INDEX, &BUFMNG_STATUS_TABLE_19, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_HW_CFG", 1, CORE_19_INDEX, &DHD_HW_CFG_19, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_19_INDEX, &DEBUG_PRINT_CORE_LOCK_19, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SRAM_DUMMY_STORE", 1, CORE_19_INDEX, &SRAM_DUMMY_STORE_19, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_DISABLE", 1, CORE_19_INDEX, &CSO_DISABLE_19, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_19_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_19, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_19_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_19, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_FPM_REPLY", 1, CORE_19_INDEX, &DHD_FPM_REPLY_19, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_GLOBAL_CFG", 1, CORE_19_INDEX, &FPM_GLOBAL_CFG_19, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_19_INDEX, &INGRESS_FILTER_1588_CFG_19, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_19_INDEX, &RX_MIRRORING_DIRECT_ENABLE_19, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_19_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_19, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "TCAM_GENERIC_FIELDS", 1, CORE_19_INDEX, &TCAM_GENERIC_FIELDS_19, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "CHANNEL_ID_TO_SBPM_BB_ID_TABLE", 1, CORE_19_INDEX, &CHANNEL_ID_TO_SBPM_BB_ID_TABLE_19, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_19_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_19, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_SCRATCHPAD", 1, CORE_19_INDEX, &DEBUG_SCRATCHPAD_19, 12, 1, 1 },
#endif
#if defined BCM6885
	{ "GDX_PARAMS_TABLE", 1, CORE_19_INDEX, &GDX_PARAMS_TABLE_19, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_19_INDEX, &FW_ERROR_VECTOR_TABLE_19, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "NAT_CACHE_CFG", 1, CORE_19_INDEX, &NAT_CACHE_CFG_19, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_19_INDEX, &NAT_CACHE_KEY0_MASK_19, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_19_INDEX, &TCAM_TABLE_CFG_TABLE_19, 2, 1, 1 },
#endif
#if defined BCM6885
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_19_INDEX, &BITS_CALC_MASKS_TABLE_19, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_19_INDEX, &NATC_L2_VLAN_KEY_MASK_19, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_CFG", 1, CORE_19_INDEX, &INGRESS_FILTER_CFG_19, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "MULTICAST_KEY_MASK", 1, CORE_19_INDEX, &MULTICAST_KEY_MASK_19, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "NATC_L2_TOS_MASK", 1, CORE_19_INDEX, &NATC_L2_TOS_MASK_19, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "POLICER_PARAMS_TABLE", 1, CORE_20_INDEX, &POLICER_PARAMS_TABLE_20, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING0_STACK", 1, CORE_20_INDEX, &PROCESSING0_STACK_20, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "SYSTEM_CONFIGURATION", 1, CORE_20_INDEX, &SYSTEM_CONFIGURATION_20, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDTEST_GEN_PARAM", 1, CORE_20_INDEX, &SPDTEST_GEN_PARAM_20, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_CFG_TABLE", 1, CORE_20_INDEX, &VPORT_CFG_TABLE_20, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "MUTEX_TABLE", 1, CORE_20_INDEX, &MUTEX_TABLE_20, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_20_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_20, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING1_STACK", 1, CORE_20_INDEX, &PROCESSING1_STACK_20, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "REGISTERS_BUFFER", 1, CORE_20_INDEX, &REGISTERS_BUFFER_20, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "TUNNELS_PARSING_CFG", 1, CORE_20_INDEX, &TUNNELS_PARSING_CFG_20, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_20_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_20, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING2_STACK", 1, CORE_20_INDEX, &PROCESSING2_STACK_20, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_20_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_20, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_BAD_IPV4_HDR_CSUM_PACKETS", 1, CORE_20_INDEX, &CSO_BAD_IPV4_HDR_CSUM_PACKETS_20, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_20_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_20, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TASK_IDX", 1, CORE_20_INDEX, &TASK_IDX_20, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_20_INDEX, &LOOPBACK_QUEUE_TABLE_20, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DOS_DROP_REASONS_CFG", 1, CORE_20_INDEX, &DOS_DROP_REASONS_CFG_20, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_CFG_EX_TABLE", 1, CORE_20_INDEX, &VPORT_CFG_EX_TABLE_20, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_20_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_20, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING3_STACK", 1, CORE_20_INDEX, &PROCESSING3_STACK_20, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_CFG_TABLE", 1, CORE_20_INDEX, &IPTV_CFG_TABLE_20, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_FPM_THRESHOLDS", 1, CORE_20_INDEX, &DHD_FPM_THRESHOLDS_20, 3, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_20_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_20, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_20_INDEX, &LOOPBACK_WAN_FLOW_TABLE_20, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FORCE_DSCP", 1, CORE_20_INDEX, &FORCE_DSCP_20, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CORE_ID_TABLE", 1, CORE_20_INDEX, &CORE_ID_TABLE_20, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TR471_SPDSVC_RX_PKT_ID_TABLE", 1, CORE_20_INDEX, &TR471_SPDSVC_RX_PKT_ID_TABLE_20, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_TABLE", 1, CORE_20_INDEX, &DEBUG_PRINT_TABLE_20, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_20_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_20, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_20_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_20, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING4_STACK", 1, CORE_20_INDEX, &PROCESSING4_STACK_20, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "TCAM_GENERIC_FIELDS", 1, CORE_20_INDEX, &TCAM_GENERIC_FIELDS_20, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_20_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_20, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_20_INDEX, &BUFMNG_DESCRIPTOR_TABLE_20, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_STATUS_TABLE", 1, CORE_20_INDEX, &BUFMNG_STATUS_TABLE_20, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_20_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_20, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_20_INDEX, &DEBUG_PRINT_CORE_LOCK_20, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_HW_CFG", 1, CORE_20_INDEX, &DHD_HW_CFG_20, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_20_INDEX, &RX_MIRRORING_CONFIGURATION_20, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_20_INDEX, &TX_MIRRORING_CONFIGURATION_20, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "NAT_CACHE_CFG", 1, CORE_20_INDEX, &NAT_CACHE_CFG_20, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SRAM_DUMMY_STORE", 1, CORE_20_INDEX, &SRAM_DUMMY_STORE_20, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING5_STACK", 1, CORE_20_INDEX, &PROCESSING5_STACK_20, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "CHANNEL_ID_TO_SBPM_BB_ID_TABLE", 1, CORE_20_INDEX, &CHANNEL_ID_TO_SBPM_BB_ID_TABLE_20, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_20_INDEX, &NAT_CACHE_KEY0_MASK_20, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_DISABLE", 1, CORE_20_INDEX, &CSO_DISABLE_20, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_20_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_20, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_20_INDEX, &INGRESS_FILTER_1588_CFG_20, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_20_INDEX, &RX_MIRRORING_DIRECT_ENABLE_20, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_20_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_20, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_SCRATCHPAD", 1, CORE_20_INDEX, &DEBUG_SCRATCHPAD_20, 12, 1, 1 },
#endif
#if defined BCM6885
	{ "GDX_PARAMS_TABLE", 1, CORE_20_INDEX, &GDX_PARAMS_TABLE_20, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_GLOBAL_CFG", 1, CORE_20_INDEX, &FPM_GLOBAL_CFG_20, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_20_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER_20, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING6_STACK", 1, CORE_20_INDEX, &PROCESSING6_STACK_20, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_20_INDEX, &TCAM_TABLE_CFG_TABLE_20, 2, 1, 1 },
#endif
#if defined BCM6885
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_20_INDEX, &NATC_L2_VLAN_KEY_MASK_20, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_CFG", 1, CORE_20_INDEX, &INGRESS_FILTER_CFG_20, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_20_INDEX, &BITS_CALC_MASKS_TABLE_20, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "NATC_L2_TOS_MASK", 1, CORE_20_INDEX, &NATC_L2_TOS_MASK_20, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_20_INDEX, &FW_ERROR_VECTOR_TABLE_20, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "MULTICAST_KEY_MASK", 1, CORE_20_INDEX, &MULTICAST_KEY_MASK_20, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING7_STACK", 1, CORE_20_INDEX, &PROCESSING7_STACK_20, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_FLOW_TABLE", 1, CORE_20_INDEX, &RX_FLOW_TABLE_20, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_FLOW_TABLE", 1, CORE_20_INDEX, &TX_FLOW_TABLE_20, 212, 1, 1 },
#endif
#if defined BCM6885
	{ "PARSER_KEY_BUILD_TABLE", 1, CORE_20_INDEX, &PARSER_KEY_BUILD_TABLE_20, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_20_INDEX, &INGRESS_FILTER_PROFILE_TABLE_20, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "POLICER_PARAMS_TABLE", 1, CORE_21_INDEX, &POLICER_PARAMS_TABLE_21, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING0_STACK", 1, CORE_21_INDEX, &PROCESSING0_STACK_21, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "SYSTEM_CONFIGURATION", 1, CORE_21_INDEX, &SYSTEM_CONFIGURATION_21, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDTEST_GEN_PARAM", 1, CORE_21_INDEX, &SPDTEST_GEN_PARAM_21, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_CFG_TABLE", 1, CORE_21_INDEX, &VPORT_CFG_TABLE_21, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "MUTEX_TABLE", 1, CORE_21_INDEX, &MUTEX_TABLE_21, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_21_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_21, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING1_STACK", 1, CORE_21_INDEX, &PROCESSING1_STACK_21, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "REGISTERS_BUFFER", 1, CORE_21_INDEX, &REGISTERS_BUFFER_21, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "TUNNELS_PARSING_CFG", 1, CORE_21_INDEX, &TUNNELS_PARSING_CFG_21, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_21_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_21, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING2_STACK", 1, CORE_21_INDEX, &PROCESSING2_STACK_21, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_21_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_21, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_BAD_IPV4_HDR_CSUM_PACKETS", 1, CORE_21_INDEX, &CSO_BAD_IPV4_HDR_CSUM_PACKETS_21, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_21_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_21, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TASK_IDX", 1, CORE_21_INDEX, &TASK_IDX_21, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_21_INDEX, &LOOPBACK_QUEUE_TABLE_21, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DOS_DROP_REASONS_CFG", 1, CORE_21_INDEX, &DOS_DROP_REASONS_CFG_21, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_CFG_EX_TABLE", 1, CORE_21_INDEX, &VPORT_CFG_EX_TABLE_21, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_21_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_21, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING3_STACK", 1, CORE_21_INDEX, &PROCESSING3_STACK_21, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_CFG_TABLE", 1, CORE_21_INDEX, &IPTV_CFG_TABLE_21, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_FPM_THRESHOLDS", 1, CORE_21_INDEX, &DHD_FPM_THRESHOLDS_21, 3, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_21_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_21, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_21_INDEX, &LOOPBACK_WAN_FLOW_TABLE_21, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FORCE_DSCP", 1, CORE_21_INDEX, &FORCE_DSCP_21, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CORE_ID_TABLE", 1, CORE_21_INDEX, &CORE_ID_TABLE_21, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TR471_SPDSVC_RX_PKT_ID_TABLE", 1, CORE_21_INDEX, &TR471_SPDSVC_RX_PKT_ID_TABLE_21, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_TABLE", 1, CORE_21_INDEX, &DEBUG_PRINT_TABLE_21, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_21_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_21, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_21_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_21, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING4_STACK", 1, CORE_21_INDEX, &PROCESSING4_STACK_21, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "TCAM_GENERIC_FIELDS", 1, CORE_21_INDEX, &TCAM_GENERIC_FIELDS_21, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_21_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_21, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_21_INDEX, &BUFMNG_DESCRIPTOR_TABLE_21, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_STATUS_TABLE", 1, CORE_21_INDEX, &BUFMNG_STATUS_TABLE_21, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_21_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_21, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_21_INDEX, &DEBUG_PRINT_CORE_LOCK_21, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_HW_CFG", 1, CORE_21_INDEX, &DHD_HW_CFG_21, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_21_INDEX, &RX_MIRRORING_CONFIGURATION_21, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_21_INDEX, &TX_MIRRORING_CONFIGURATION_21, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "NAT_CACHE_CFG", 1, CORE_21_INDEX, &NAT_CACHE_CFG_21, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SRAM_DUMMY_STORE", 1, CORE_21_INDEX, &SRAM_DUMMY_STORE_21, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING5_STACK", 1, CORE_21_INDEX, &PROCESSING5_STACK_21, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "CHANNEL_ID_TO_SBPM_BB_ID_TABLE", 1, CORE_21_INDEX, &CHANNEL_ID_TO_SBPM_BB_ID_TABLE_21, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_21_INDEX, &NAT_CACHE_KEY0_MASK_21, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_DISABLE", 1, CORE_21_INDEX, &CSO_DISABLE_21, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_21_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_21, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_21_INDEX, &INGRESS_FILTER_1588_CFG_21, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_21_INDEX, &RX_MIRRORING_DIRECT_ENABLE_21, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_21_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_21, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_SCRATCHPAD", 1, CORE_21_INDEX, &DEBUG_SCRATCHPAD_21, 12, 1, 1 },
#endif
#if defined BCM6885
	{ "GDX_PARAMS_TABLE", 1, CORE_21_INDEX, &GDX_PARAMS_TABLE_21, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_GLOBAL_CFG", 1, CORE_21_INDEX, &FPM_GLOBAL_CFG_21, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_21_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER_21, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING6_STACK", 1, CORE_21_INDEX, &PROCESSING6_STACK_21, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_21_INDEX, &TCAM_TABLE_CFG_TABLE_21, 2, 1, 1 },
#endif
#if defined BCM6885
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_21_INDEX, &NATC_L2_VLAN_KEY_MASK_21, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_CFG", 1, CORE_21_INDEX, &INGRESS_FILTER_CFG_21, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_21_INDEX, &BITS_CALC_MASKS_TABLE_21, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "NATC_L2_TOS_MASK", 1, CORE_21_INDEX, &NATC_L2_TOS_MASK_21, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_21_INDEX, &FW_ERROR_VECTOR_TABLE_21, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "MULTICAST_KEY_MASK", 1, CORE_21_INDEX, &MULTICAST_KEY_MASK_21, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING7_STACK", 1, CORE_21_INDEX, &PROCESSING7_STACK_21, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_FLOW_TABLE", 1, CORE_21_INDEX, &RX_FLOW_TABLE_21, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_FLOW_TABLE", 1, CORE_21_INDEX, &TX_FLOW_TABLE_21, 212, 1, 1 },
#endif
#if defined BCM6885
	{ "PARSER_KEY_BUILD_TABLE", 1, CORE_21_INDEX, &PARSER_KEY_BUILD_TABLE_21, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_21_INDEX, &INGRESS_FILTER_PROFILE_TABLE_21, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "POLICER_PARAMS_TABLE", 1, CORE_22_INDEX, &POLICER_PARAMS_TABLE_22, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING0_STACK", 1, CORE_22_INDEX, &PROCESSING0_STACK_22, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "SYSTEM_CONFIGURATION", 1, CORE_22_INDEX, &SYSTEM_CONFIGURATION_22, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDTEST_GEN_PARAM", 1, CORE_22_INDEX, &SPDTEST_GEN_PARAM_22, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_CFG_TABLE", 1, CORE_22_INDEX, &VPORT_CFG_TABLE_22, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "MUTEX_TABLE", 1, CORE_22_INDEX, &MUTEX_TABLE_22, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_22_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_22, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING1_STACK", 1, CORE_22_INDEX, &PROCESSING1_STACK_22, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "REGISTERS_BUFFER", 1, CORE_22_INDEX, &REGISTERS_BUFFER_22, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "TUNNELS_PARSING_CFG", 1, CORE_22_INDEX, &TUNNELS_PARSING_CFG_22, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_22_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_22, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING2_STACK", 1, CORE_22_INDEX, &PROCESSING2_STACK_22, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_22_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_22, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_BAD_IPV4_HDR_CSUM_PACKETS", 1, CORE_22_INDEX, &CSO_BAD_IPV4_HDR_CSUM_PACKETS_22, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_22_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_22, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TASK_IDX", 1, CORE_22_INDEX, &TASK_IDX_22, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_22_INDEX, &LOOPBACK_QUEUE_TABLE_22, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DOS_DROP_REASONS_CFG", 1, CORE_22_INDEX, &DOS_DROP_REASONS_CFG_22, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_CFG_EX_TABLE", 1, CORE_22_INDEX, &VPORT_CFG_EX_TABLE_22, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_22_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_22, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING3_STACK", 1, CORE_22_INDEX, &PROCESSING3_STACK_22, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_CFG_TABLE", 1, CORE_22_INDEX, &IPTV_CFG_TABLE_22, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_FPM_THRESHOLDS", 1, CORE_22_INDEX, &DHD_FPM_THRESHOLDS_22, 3, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_22_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_22, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_22_INDEX, &LOOPBACK_WAN_FLOW_TABLE_22, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FORCE_DSCP", 1, CORE_22_INDEX, &FORCE_DSCP_22, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CORE_ID_TABLE", 1, CORE_22_INDEX, &CORE_ID_TABLE_22, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TR471_SPDSVC_RX_PKT_ID_TABLE", 1, CORE_22_INDEX, &TR471_SPDSVC_RX_PKT_ID_TABLE_22, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_TABLE", 1, CORE_22_INDEX, &DEBUG_PRINT_TABLE_22, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_22_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_22, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_22_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_22, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING4_STACK", 1, CORE_22_INDEX, &PROCESSING4_STACK_22, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "TCAM_GENERIC_FIELDS", 1, CORE_22_INDEX, &TCAM_GENERIC_FIELDS_22, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_22_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_22, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_22_INDEX, &BUFMNG_DESCRIPTOR_TABLE_22, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_STATUS_TABLE", 1, CORE_22_INDEX, &BUFMNG_STATUS_TABLE_22, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_22_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_22, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_22_INDEX, &DEBUG_PRINT_CORE_LOCK_22, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_HW_CFG", 1, CORE_22_INDEX, &DHD_HW_CFG_22, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_22_INDEX, &RX_MIRRORING_CONFIGURATION_22, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_22_INDEX, &TX_MIRRORING_CONFIGURATION_22, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "NAT_CACHE_CFG", 1, CORE_22_INDEX, &NAT_CACHE_CFG_22, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SRAM_DUMMY_STORE", 1, CORE_22_INDEX, &SRAM_DUMMY_STORE_22, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING5_STACK", 1, CORE_22_INDEX, &PROCESSING5_STACK_22, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "CHANNEL_ID_TO_SBPM_BB_ID_TABLE", 1, CORE_22_INDEX, &CHANNEL_ID_TO_SBPM_BB_ID_TABLE_22, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_22_INDEX, &NAT_CACHE_KEY0_MASK_22, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_DISABLE", 1, CORE_22_INDEX, &CSO_DISABLE_22, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_22_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_22, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_22_INDEX, &INGRESS_FILTER_1588_CFG_22, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_22_INDEX, &RX_MIRRORING_DIRECT_ENABLE_22, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_22_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_22, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_SCRATCHPAD", 1, CORE_22_INDEX, &DEBUG_SCRATCHPAD_22, 12, 1, 1 },
#endif
#if defined BCM6885
	{ "GDX_PARAMS_TABLE", 1, CORE_22_INDEX, &GDX_PARAMS_TABLE_22, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_GLOBAL_CFG", 1, CORE_22_INDEX, &FPM_GLOBAL_CFG_22, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_22_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER_22, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING6_STACK", 1, CORE_22_INDEX, &PROCESSING6_STACK_22, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_22_INDEX, &TCAM_TABLE_CFG_TABLE_22, 2, 1, 1 },
#endif
#if defined BCM6885
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_22_INDEX, &NATC_L2_VLAN_KEY_MASK_22, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_CFG", 1, CORE_22_INDEX, &INGRESS_FILTER_CFG_22, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_22_INDEX, &BITS_CALC_MASKS_TABLE_22, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "NATC_L2_TOS_MASK", 1, CORE_22_INDEX, &NATC_L2_TOS_MASK_22, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_22_INDEX, &FW_ERROR_VECTOR_TABLE_22, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "MULTICAST_KEY_MASK", 1, CORE_22_INDEX, &MULTICAST_KEY_MASK_22, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING7_STACK", 1, CORE_22_INDEX, &PROCESSING7_STACK_22, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_FLOW_TABLE", 1, CORE_22_INDEX, &RX_FLOW_TABLE_22, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_FLOW_TABLE", 1, CORE_22_INDEX, &TX_FLOW_TABLE_22, 212, 1, 1 },
#endif
#if defined BCM6885
	{ "PARSER_KEY_BUILD_TABLE", 1, CORE_22_INDEX, &PARSER_KEY_BUILD_TABLE_22, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_22_INDEX, &INGRESS_FILTER_PROFILE_TABLE_22, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "POLICER_PARAMS_TABLE", 1, CORE_23_INDEX, &POLICER_PARAMS_TABLE_23, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING0_STACK", 1, CORE_23_INDEX, &PROCESSING0_STACK_23, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "SYSTEM_CONFIGURATION", 1, CORE_23_INDEX, &SYSTEM_CONFIGURATION_23, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDTEST_GEN_PARAM", 1, CORE_23_INDEX, &SPDTEST_GEN_PARAM_23, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_CFG_TABLE", 1, CORE_23_INDEX, &VPORT_CFG_TABLE_23, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "MUTEX_TABLE", 1, CORE_23_INDEX, &MUTEX_TABLE_23, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_23_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_23, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING1_STACK", 1, CORE_23_INDEX, &PROCESSING1_STACK_23, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "REGISTERS_BUFFER", 1, CORE_23_INDEX, &REGISTERS_BUFFER_23, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "TUNNELS_PARSING_CFG", 1, CORE_23_INDEX, &TUNNELS_PARSING_CFG_23, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_23_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_23, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING2_STACK", 1, CORE_23_INDEX, &PROCESSING2_STACK_23, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_23_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_23, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_BAD_IPV4_HDR_CSUM_PACKETS", 1, CORE_23_INDEX, &CSO_BAD_IPV4_HDR_CSUM_PACKETS_23, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_23_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_23, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TASK_IDX", 1, CORE_23_INDEX, &TASK_IDX_23, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_23_INDEX, &LOOPBACK_QUEUE_TABLE_23, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DOS_DROP_REASONS_CFG", 1, CORE_23_INDEX, &DOS_DROP_REASONS_CFG_23, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_CFG_EX_TABLE", 1, CORE_23_INDEX, &VPORT_CFG_EX_TABLE_23, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_23_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_23, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING3_STACK", 1, CORE_23_INDEX, &PROCESSING3_STACK_23, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_CFG_TABLE", 1, CORE_23_INDEX, &IPTV_CFG_TABLE_23, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_FPM_THRESHOLDS", 1, CORE_23_INDEX, &DHD_FPM_THRESHOLDS_23, 3, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDTEST_NUM_OF_RX_FLOWS", 1, CORE_23_INDEX, &SPDTEST_NUM_OF_RX_FLOWS_23, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_23_INDEX, &LOOPBACK_WAN_FLOW_TABLE_23, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FORCE_DSCP", 1, CORE_23_INDEX, &FORCE_DSCP_23, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CORE_ID_TABLE", 1, CORE_23_INDEX, &CORE_ID_TABLE_23, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TR471_SPDSVC_RX_PKT_ID_TABLE", 1, CORE_23_INDEX, &TR471_SPDSVC_RX_PKT_ID_TABLE_23, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_TABLE", 1, CORE_23_INDEX, &DEBUG_PRINT_TABLE_23, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_23_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_23, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_23_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_23, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING4_STACK", 1, CORE_23_INDEX, &PROCESSING4_STACK_23, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "TCAM_GENERIC_FIELDS", 1, CORE_23_INDEX, &TCAM_GENERIC_FIELDS_23, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_23_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_23, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_DESCRIPTOR_TABLE", 1, CORE_23_INDEX, &BUFMNG_DESCRIPTOR_TABLE_23, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_STATUS_TABLE", 1, CORE_23_INDEX, &BUFMNG_STATUS_TABLE_23, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_23_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_23, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_23_INDEX, &DEBUG_PRINT_CORE_LOCK_23, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_HW_CFG", 1, CORE_23_INDEX, &DHD_HW_CFG_23, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_23_INDEX, &RX_MIRRORING_CONFIGURATION_23, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_23_INDEX, &TX_MIRRORING_CONFIGURATION_23, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "NAT_CACHE_CFG", 1, CORE_23_INDEX, &NAT_CACHE_CFG_23, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SRAM_DUMMY_STORE", 1, CORE_23_INDEX, &SRAM_DUMMY_STORE_23, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING5_STACK", 1, CORE_23_INDEX, &PROCESSING5_STACK_23, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "CHANNEL_ID_TO_SBPM_BB_ID_TABLE", 1, CORE_23_INDEX, &CHANNEL_ID_TO_SBPM_BB_ID_TABLE_23, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_23_INDEX, &NAT_CACHE_KEY0_MASK_23, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_DISABLE", 1, CORE_23_INDEX, &CSO_DISABLE_23, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_23_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_23, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_23_INDEX, &INGRESS_FILTER_1588_CFG_23, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_23_INDEX, &RX_MIRRORING_DIRECT_ENABLE_23, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_23_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_23, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_SCRATCHPAD", 1, CORE_23_INDEX, &DEBUG_SCRATCHPAD_23, 12, 1, 1 },
#endif
#if defined BCM6885
	{ "GDX_PARAMS_TABLE", 1, CORE_23_INDEX, &GDX_PARAMS_TABLE_23, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_GLOBAL_CFG", 1, CORE_23_INDEX, &FPM_GLOBAL_CFG_23, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_23_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER_23, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING6_STACK", 1, CORE_23_INDEX, &PROCESSING6_STACK_23, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_23_INDEX, &TCAM_TABLE_CFG_TABLE_23, 2, 1, 1 },
#endif
#if defined BCM6885
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_23_INDEX, &NATC_L2_VLAN_KEY_MASK_23, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_CFG", 1, CORE_23_INDEX, &INGRESS_FILTER_CFG_23, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_23_INDEX, &BITS_CALC_MASKS_TABLE_23, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "NATC_L2_TOS_MASK", 1, CORE_23_INDEX, &NATC_L2_TOS_MASK_23, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_23_INDEX, &FW_ERROR_VECTOR_TABLE_23, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "MULTICAST_KEY_MASK", 1, CORE_23_INDEX, &MULTICAST_KEY_MASK_23, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PROCESSING7_STACK", 1, CORE_23_INDEX, &PROCESSING7_STACK_23, 360, 1, 1 },
#endif
#if defined BCM6885
	{ "RX_FLOW_TABLE", 1, CORE_23_INDEX, &RX_FLOW_TABLE_23, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_FLOW_TABLE", 1, CORE_23_INDEX, &TX_FLOW_TABLE_23, 212, 1, 1 },
#endif
#if defined BCM6885
	{ "PARSER_KEY_BUILD_TABLE", 1, CORE_23_INDEX, &PARSER_KEY_BUILD_TABLE_23, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_23_INDEX, &INGRESS_FILTER_PROFILE_TABLE_23, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "DS_TM_TM_FLOW_CNTR_TABLE", 1, CORE_24_INDEX, &DS_TM_TM_FLOW_CNTR_TABLE_24, 64, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU0_RL_UPDATE_RESPONSE", 1, CORE_24_INDEX, &GSU0_RL_UPDATE_RESPONSE_24, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU1_RL_UPDATE_RESPONSE", 1, CORE_24_INDEX, &GSU1_RL_UPDATE_RESPONSE_24, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU2_RL_UPDATE_RESPONSE", 1, CORE_24_INDEX, &GSU2_RL_UPDATE_RESPONSE_24, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU3_RL_UPDATE_RESPONSE", 1, CORE_24_INDEX, &GSU3_RL_UPDATE_RESPONSE_24, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU4_RL_UPDATE_RESPONSE", 1, CORE_24_INDEX, &GSU4_RL_UPDATE_RESPONSE_24, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU0_AQM_UPDATE_STATUS_RESPONSE", 1, CORE_24_INDEX, &GSU0_AQM_UPDATE_STATUS_RESPONSE_24, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU1_AQM_UPDATE_STATUS_RESPONSE", 1, CORE_24_INDEX, &GSU1_AQM_UPDATE_STATUS_RESPONSE_24, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU2_AQM_UPDATE_STATUS_RESPONSE", 1, CORE_24_INDEX, &GSU2_AQM_UPDATE_STATUS_RESPONSE_24, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU3_AQM_UPDATE_STATUS_RESPONSE", 1, CORE_24_INDEX, &GSU3_AQM_UPDATE_STATUS_RESPONSE_24, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU4_AQM_UPDATE_STATUS_RESPONSE", 1, CORE_24_INDEX, &GSU4_AQM_UPDATE_STATUS_RESPONSE_24, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "AQM_PROBABILITY_CALC_DESCRIPTOR", 1, CORE_24_INDEX, &AQM_PROBABILITY_CALC_DESCRIPTOR_24, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_24_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_24, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_24_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_24, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "RL_UPDATE_TASK_DESCRIPTOR", 1, CORE_24_INDEX, &RL_UPDATE_TASK_DESCRIPTOR_24, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DS_TM_BBH_TX_WAKE_UP_DATA_TABLE", 1, CORE_24_INDEX, &DS_TM_BBH_TX_WAKE_UP_DATA_TABLE_24, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "UPDATE_FIFO_GSU0_STACK", 1, CORE_24_INDEX, &UPDATE_FIFO_GSU0_STACK_24, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "UPDATE_FIFO_GSU1_STACK", 1, CORE_24_INDEX, &UPDATE_FIFO_GSU1_STACK_24, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "UPDATE_FIFO_GSU2_STACK", 1, CORE_24_INDEX, &UPDATE_FIFO_GSU2_STACK_24, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "AQM_QUEUE_TABLE", 1, CORE_24_INDEX, &AQM_QUEUE_TABLE_24, 145, 1, 1 },
#endif
#if defined BCM6885
	{ "MUTEX_TABLE", 1, CORE_24_INDEX, &MUTEX_TABLE_24, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_SCRATCHPAD", 1, CORE_24_INDEX, &DEBUG_SCRATCHPAD_24, 12, 1, 1 },
#endif
#if defined BCM6885
	{ "SYSTEM_CONFIGURATION", 1, CORE_24_INDEX, &SYSTEM_CONFIGURATION_24, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU_SETTING_TABLE", 1, CORE_24_INDEX, &GSU_SETTING_TABLE_24, 5, 1, 1 },
#endif
#if defined BCM6885
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_24_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_24, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_24_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_24, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_GLOBAL_CFG", 1, CORE_24_INDEX, &FPM_GLOBAL_CFG_24, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TASK_IDX", 1, CORE_24_INDEX, &TASK_IDX_24, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "UPDATE_FIFO_GSU3_STACK", 1, CORE_24_INDEX, &UPDATE_FIFO_GSU3_STACK_24, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_24_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_24, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "UPDATE_FIFO_GSU4_STACK", 1, CORE_24_INDEX, &UPDATE_FIFO_GSU4_STACK_24, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_GSU0_STACK", 1, CORE_24_INDEX, &TX_GSU0_STACK_24, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_GSU1_STACK", 1, CORE_24_INDEX, &TX_GSU1_STACK_24, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_GSU2_STACK", 1, CORE_24_INDEX, &TX_GSU2_STACK_24, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_GSU3_STACK", 1, CORE_24_INDEX, &TX_GSU3_STACK_24, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_GSU4_STACK", 1, CORE_24_INDEX, &TX_GSU4_STACK_24, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "UPDATE_RL_STACK", 1, CORE_24_INDEX, &UPDATE_RL_STACK_24, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "UPDATE_AQM_STACK", 1, CORE_24_INDEX, &UPDATE_AQM_STACK_24, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU0_UPDATE_FIFO_TABLE", 1, CORE_24_INDEX, &GSU0_UPDATE_FIFO_TABLE_24, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU1_UPDATE_FIFO_TABLE", 1, CORE_24_INDEX, &GSU1_UPDATE_FIFO_TABLE_24, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU2_UPDATE_FIFO_TABLE", 1, CORE_24_INDEX, &GSU2_UPDATE_FIFO_TABLE_24, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU3_UPDATE_FIFO_TABLE", 1, CORE_24_INDEX, &GSU3_UPDATE_FIFO_TABLE_24, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU4_UPDATE_FIFO_TABLE", 1, CORE_24_INDEX, &GSU4_UPDATE_FIFO_TABLE_24, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU0_TRANSMIT_PD", 1, CORE_24_INDEX, &GSU0_TRANSMIT_PD_24, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU1_TRANSMIT_PD", 1, CORE_24_INDEX, &GSU1_TRANSMIT_PD_24, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU2_TRANSMIT_PD", 1, CORE_24_INDEX, &GSU2_TRANSMIT_PD_24, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU3_TRANSMIT_PD", 1, CORE_24_INDEX, &GSU3_TRANSMIT_PD_24, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU4_TRANSMIT_PD", 1, CORE_24_INDEX, &GSU4_TRANSMIT_PD_24, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "REGISTERS_BUFFER", 1, CORE_24_INDEX, &REGISTERS_BUFFER_24, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "DS_TM_CPU_TX_ABS_COUNTERS", 1, CORE_24_INDEX, &DS_TM_CPU_TX_ABS_COUNTERS_24, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "DS_TM_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_TABLE", 1, CORE_24_INDEX, &DS_TM_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_TABLE_24, 2, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU0_TRANSMIT_TASK_DESCRIPTOR", 1, CORE_24_INDEX, &GSU0_TRANSMIT_TASK_DESCRIPTOR_24, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU1_TRANSMIT_TASK_DESCRIPTOR", 1, CORE_24_INDEX, &GSU1_TRANSMIT_TASK_DESCRIPTOR_24, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU2_TRANSMIT_TASK_DESCRIPTOR", 1, CORE_24_INDEX, &GSU2_TRANSMIT_TASK_DESCRIPTOR_24, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_24_INDEX, &VPORT_TO_RL_OVERHEAD_TABLE_24, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "CORE_ID_TABLE", 1, CORE_24_INDEX, &CORE_ID_TABLE_24, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU3_TRANSMIT_TASK_DESCRIPTOR", 1, CORE_24_INDEX, &GSU3_TRANSMIT_TASK_DESCRIPTOR_24, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU4_TRANSMIT_TASK_DESCRIPTOR", 1, CORE_24_INDEX, &GSU4_TRANSMIT_TASK_DESCRIPTOR_24, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DS_TM_BBH_TX_EGRESS_COUNTER_TABLE", 1, CORE_24_INDEX, &DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_24, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU0_TRANSMIT_TASK_WAKE_UP_BB_MSG", 1, CORE_24_INDEX, &GSU0_TRANSMIT_TASK_WAKE_UP_BB_MSG_24, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_TABLE", 1, CORE_24_INDEX, &DEBUG_PRINT_TABLE_24, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU1_TRANSMIT_TASK_WAKE_UP_BB_MSG", 1, CORE_24_INDEX, &GSU1_TRANSMIT_TASK_WAKE_UP_BB_MSG_24, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU2_TRANSMIT_TASK_WAKE_UP_BB_MSG", 1, CORE_24_INDEX, &GSU2_TRANSMIT_TASK_WAKE_UP_BB_MSG_24, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU3_TRANSMIT_TASK_WAKE_UP_BB_MSG", 1, CORE_24_INDEX, &GSU3_TRANSMIT_TASK_WAKE_UP_BB_MSG_24, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU4_TRANSMIT_TASK_WAKE_UP_BB_MSG", 1, CORE_24_INDEX, &GSU4_TRANSMIT_TASK_WAKE_UP_BB_MSG_24, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU0_TRANSMIT_TASK_STATUS_BB_MSG", 1, CORE_24_INDEX, &GSU0_TRANSMIT_TASK_STATUS_BB_MSG_24, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU1_TRANSMIT_TASK_STATUS_BB_MSG", 1, CORE_24_INDEX, &GSU1_TRANSMIT_TASK_STATUS_BB_MSG_24, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU2_TRANSMIT_TASK_STATUS_BB_MSG", 1, CORE_24_INDEX, &GSU2_TRANSMIT_TASK_STATUS_BB_MSG_24, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU3_TRANSMIT_TASK_STATUS_BB_MSG", 1, CORE_24_INDEX, &GSU3_TRANSMIT_TASK_STATUS_BB_MSG_24, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU4_TRANSMIT_TASK_STATUS_BB_MSG", 1, CORE_24_INDEX, &GSU4_TRANSMIT_TASK_STATUS_BB_MSG_24, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU0_UPDATE_FIFO_TASK_DESCRIPTOR", 1, CORE_24_INDEX, &GSU0_UPDATE_FIFO_TASK_DESCRIPTOR_24, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU1_UPDATE_FIFO_TASK_DESCRIPTOR", 1, CORE_24_INDEX, &GSU1_UPDATE_FIFO_TASK_DESCRIPTOR_24, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU2_UPDATE_FIFO_TASK_DESCRIPTOR", 1, CORE_24_INDEX, &GSU2_UPDATE_FIFO_TASK_DESCRIPTOR_24, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU3_UPDATE_FIFO_TASK_DESCRIPTOR", 1, CORE_24_INDEX, &GSU3_UPDATE_FIFO_TASK_DESCRIPTOR_24, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU4_UPDATE_FIFO_TASK_DESCRIPTOR", 1, CORE_24_INDEX, &GSU4_UPDATE_FIFO_TASK_DESCRIPTOR_24, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DS_TM_BB_DESTINATION_TABLE", 1, CORE_24_INDEX, &DS_TM_BB_DESTINATION_TABLE_24, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DS_TM_FIRST_QUEUE_MAPPING", 1, CORE_24_INDEX, &DS_TM_FIRST_QUEUE_MAPPING_24, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_24_INDEX, &DEBUG_PRINT_CORE_LOCK_24, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SRAM_DUMMY_STORE", 1, CORE_24_INDEX, &SRAM_DUMMY_STORE_24, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BACKUP_BBH_INGRESS_COUNTERS_TABLE", 1, CORE_24_INDEX, &BACKUP_BBH_INGRESS_COUNTERS_TABLE_24, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BACKUP_BBH_EGRESS_COUNTERS_TABLE", 1, CORE_24_INDEX, &BACKUP_BBH_EGRESS_COUNTERS_TABLE_24, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_24_INDEX, &FW_ERROR_VECTOR_TABLE_24, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "US_TM_WAN_0_BBH_TX_EGRESS_COUNTER_TABLE", 1, CORE_25_INDEX, &US_TM_WAN_0_BBH_TX_EGRESS_COUNTER_TABLE_25, 40, 1, 1 },
#endif
#if defined BCM6885
	{ "SYSTEM_CONFIGURATION", 1, CORE_25_INDEX, &SYSTEM_CONFIGURATION_25, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "AQM_PROBABILITY_CALC_DESCRIPTOR", 1, CORE_25_INDEX, &AQM_PROBABILITY_CALC_DESCRIPTOR_25, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU0_RL_UPDATE_RESPONSE", 1, CORE_25_INDEX, &GSU0_RL_UPDATE_RESPONSE_25, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "US_TM_WAN_0_BBH_TX_WAKE_UP_DATA_TABLE", 1, CORE_25_INDEX, &US_TM_WAN_0_BBH_TX_WAKE_UP_DATA_TABLE_25, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DISPATCHER_CREDIT_TABLE", 1, CORE_25_INDEX, &DISPATCHER_CREDIT_TABLE_25, 3, 1, 1 },
#endif
#if defined BCM6885
	{ "MUTEX_TABLE", 1, CORE_25_INDEX, &MUTEX_TABLE_25, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU0_TRANSMIT_TASK_WAKE_UP_BB_MSG", 1, CORE_25_INDEX, &GSU0_TRANSMIT_TASK_WAKE_UP_BB_MSG_25, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU0_AQM_UPDATE_STATUS_RESPONSE", 1, CORE_25_INDEX, &GSU0_AQM_UPDATE_STATUS_RESPONSE_25, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_25_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_25, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_25_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_25, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "US_TM_TM_FLOW_CNTR_TABLE", 1, CORE_25_INDEX, &US_TM_TM_FLOW_CNTR_TABLE_25, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "UPDATE_FIFO_GSU0_STACK", 1, CORE_25_INDEX, &UPDATE_FIFO_GSU0_STACK_25, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_GSU0_STACK", 1, CORE_25_INDEX, &TX_GSU0_STACK_25, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "UPDATE_RL_STACK", 1, CORE_25_INDEX, &UPDATE_RL_STACK_25, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "AQM_QUEUE_TABLE", 1, CORE_25_INDEX, &AQM_QUEUE_TABLE_25, 145, 1, 1 },
#endif
#if defined BCM6885
	{ "TASK_IDX", 1, CORE_25_INDEX, &TASK_IDX_25, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU0_TRANSMIT_TASK_STATUS_BB_MSG", 1, CORE_25_INDEX, &GSU0_TRANSMIT_TASK_STATUS_BB_MSG_25, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU0_TRANSMIT_TASK_DESCRIPTOR", 1, CORE_25_INDEX, &GSU0_TRANSMIT_TASK_DESCRIPTOR_25, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_25_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_25, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU_SETTING_TABLE", 1, CORE_25_INDEX, &GSU_SETTING_TABLE_25, 5, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_TABLE", 1, CORE_25_INDEX, &DEBUG_PRINT_TABLE_25, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_25_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_25, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "UPDATE_AQM_STACK", 1, CORE_25_INDEX, &UPDATE_AQM_STACK_25, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_25_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_25, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU0_UPDATE_FIFO_TABLE", 1, CORE_25_INDEX, &GSU0_UPDATE_FIFO_TABLE_25, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU0_TRANSMIT_PD", 1, CORE_25_INDEX, &GSU0_TRANSMIT_PD_25, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "REGISTERS_BUFFER", 1, CORE_25_INDEX, &REGISTERS_BUFFER_25, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "US_TM_CPU_TX_ABS_COUNTERS", 1, CORE_25_INDEX, &US_TM_CPU_TX_ABS_COUNTERS_25, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "US_TM_TX_OCTETS_COUNTERS_TABLE", 1, CORE_25_INDEX, &US_TM_TX_OCTETS_COUNTERS_TABLE_25, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "RL_UPDATE_TASK_DESCRIPTOR", 1, CORE_25_INDEX, &RL_UPDATE_TASK_DESCRIPTOR_25, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_SCRATCHPAD", 1, CORE_25_INDEX, &DEBUG_SCRATCHPAD_25, 12, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU0_UPDATE_FIFO_TASK_DESCRIPTOR", 1, CORE_25_INDEX, &GSU0_UPDATE_FIFO_TASK_DESCRIPTOR_25, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "US_TM_BBH_TX_WAN_0_FIFO_BYTES_USED", 1, CORE_25_INDEX, &US_TM_BBH_TX_WAN_0_FIFO_BYTES_USED_25, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "US_TM_WAN_0_BB_DESTINATION_TABLE", 1, CORE_25_INDEX, &US_TM_WAN_0_BB_DESTINATION_TABLE_25, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "QEMU_SYNC_MEM", 1, CORE_25_INDEX, &QEMU_SYNC_MEM_25, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CORE_ID_TABLE", 1, CORE_25_INDEX, &CORE_ID_TABLE_25, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PON_TM_VPORT_TO_RL_OVERHEAD_TABLE", 1, CORE_25_INDEX, &PON_TM_VPORT_TO_RL_OVERHEAD_TABLE_25, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_25_INDEX, &DEBUG_PRINT_CORE_LOCK_25, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "US_TM_FIRST_QUEUE_MAPPING", 1, CORE_25_INDEX, &US_TM_FIRST_QUEUE_MAPPING_25, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "BBH_TX_US_WAN_0_FIFO_BYTES_THRESHOLD", 1, CORE_25_INDEX, &BBH_TX_US_WAN_0_FIFO_BYTES_THRESHOLD_25, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SRAM_DUMMY_STORE", 1, CORE_25_INDEX, &SRAM_DUMMY_STORE_25, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "WAN_0_BBH_TX_FIFO_SIZE", 1, CORE_25_INDEX, &WAN_0_BBH_TX_FIFO_SIZE_25, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_GLOBAL_CFG", 1, CORE_25_INDEX, &FPM_GLOBAL_CFG_25, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "US_TM_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_TABLE", 1, CORE_25_INDEX, &US_TM_BBH_QUEUE_TO_BBH_QUEUE_DESC_MAPPING_TABLE_25, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_25_INDEX, &FW_ERROR_VECTOR_TABLE_25, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PKTGEN_SBPM_HDR_BNS", 1, CORE_26_INDEX, &PKTGEN_SBPM_HDR_BNS_26, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU0_RL_UPDATE_RESPONSE", 1, CORE_26_INDEX, &GSU0_RL_UPDATE_RESPONSE_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU1_RL_UPDATE_RESPONSE", 1, CORE_26_INDEX, &GSU1_RL_UPDATE_RESPONSE_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU2_RL_UPDATE_RESPONSE", 1, CORE_26_INDEX, &GSU2_RL_UPDATE_RESPONSE_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PKTGEN_SBPM_EXTS", 1, CORE_26_INDEX, &PKTGEN_SBPM_EXTS_26, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU0_AQM_UPDATE_STATUS_RESPONSE", 1, CORE_26_INDEX, &GSU0_AQM_UPDATE_STATUS_RESPONSE_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU1_AQM_UPDATE_STATUS_RESPONSE", 1, CORE_26_INDEX, &GSU1_AQM_UPDATE_STATUS_RESPONSE_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU2_AQM_UPDATE_STATUS_RESPONSE", 1, CORE_26_INDEX, &GSU2_AQM_UPDATE_STATUS_RESPONSE_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CPU_RX_CSO_STACK", 1, CORE_26_INDEX, &CPU_RX_CSO_STACK_26, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "UPDATE_FIFO_GSU0_STACK", 1, CORE_26_INDEX, &UPDATE_FIFO_GSU0_STACK_26, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "UPDATE_FIFO_GSU1_STACK", 1, CORE_26_INDEX, &UPDATE_FIFO_GSU1_STACK_26, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "UPDATE_FIFO_GSU2_STACK", 1, CORE_26_INDEX, &UPDATE_FIFO_GSU2_STACK_26, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "CPU_RX_SCRATCHPAD", 1, CORE_26_INDEX, &CPU_RX_SCRATCHPAD_26, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_FLOW_RING_CACHE_CTX_TABLE", 1, CORE_26_INDEX, &DHD_FLOW_RING_CACHE_CTX_TABLE_26, 64, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_AUX_INFO_CACHE_TABLE", 1, CORE_26_INDEX, &DHD_AUX_INFO_CACHE_TABLE_26, 64, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_POST_COMMON_RADIO_DATA", 1, CORE_26_INDEX, &DHD_POST_COMMON_RADIO_DATA_26, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_26_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_26, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "CPU_REASON_TO_METER_TABLE", 1, CORE_26_INDEX, &CPU_REASON_TO_METER_TABLE_26, 64, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_MONITOR_BQ_INFO_TABLE", 1, CORE_26_INDEX, &DHD_MONITOR_BQ_INFO_TABLE_26, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "AQM_QUEUE_TABLE", 1, CORE_26_INDEX, &AQM_QUEUE_TABLE_26, 145, 1, 1 },
#endif
#if defined BCM6885
	{ "PKTGEN_NO_SBPM_HDRS_CNTR", 1, CORE_26_INDEX, &PKTGEN_NO_SBPM_HDRS_CNTR_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SYSTEM_CONFIGURATION", 1, CORE_26_INDEX, &SYSTEM_CONFIGURATION_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "AQM_PROBABILITY_CALC_DESCRIPTOR", 1, CORE_26_INDEX, &AQM_PROBABILITY_CALC_DESCRIPTOR_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_26_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_26, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_BACKUP_INDEX_CACHE", 1, CORE_26_INDEX, &DHD_BACKUP_INDEX_CACHE_26, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_GSU0_STACK", 1, CORE_26_INDEX, &TX_GSU0_STACK_26, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "PKTGEN_TX_STREAM_TABLE", 1, CORE_26_INDEX, &PKTGEN_TX_STREAM_TABLE_26, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_TIMER_STACK", 1, CORE_26_INDEX, &DHD_TIMER_STACK_26, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "IPV6_HOST_ADDRESS_CRC_TABLE", 1, CORE_26_INDEX, &IPV6_HOST_ADDRESS_CRC_TABLE_26, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "CPU_RXQ_SCRATCH_TABLE", 1, CORE_26_INDEX, &CPU_RXQ_SCRATCH_TABLE_26, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_GSU1_STACK", 1, CORE_26_INDEX, &TX_GSU1_STACK_26, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_26_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_26, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_GSU2_STACK", 1, CORE_26_INDEX, &TX_GSU2_STACK_26, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "UPDATE_RL_STACK", 1, CORE_26_INDEX, &UPDATE_RL_STACK_26, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "UPDATE_AQM_STACK", 1, CORE_26_INDEX, &UPDATE_AQM_STACK_26, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_DOORBELL_TX_POST_VALUE", 1, CORE_26_INDEX, &DHD_DOORBELL_TX_POST_VALUE_26, 64, 1, 1 },
#endif
#if defined BCM6885
	{ "CPU_RX_METER_TABLE", 1, CORE_26_INDEX, &CPU_RX_METER_TABLE_26, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_TX_POST_FLOW_RING_BUFFER", 1, CORE_26_INDEX, &DHD_TX_POST_FLOW_RING_BUFFER_26, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "PKTGEN_SESSION_DATA", 1, CORE_26_INDEX, &PKTGEN_SESSION_DATA_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_TX_POST_UPDATE_FIFO_STACK", 1, CORE_26_INDEX, &DHD_TX_POST_UPDATE_FIFO_STACK_26, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "CPU_RX_XPM_PD_FIFO_TABLE", 1, CORE_26_INDEX, &CPU_RX_XPM_PD_FIFO_TABLE_26, 12, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_TX_POST_UPDATE_FIFO_TABLE", 1, CORE_26_INDEX, &DHD_TX_POST_UPDATE_FIFO_TABLE_26, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "IPV4_HOST_ADDRESS_TABLE", 1, CORE_26_INDEX, &IPV4_HOST_ADDRESS_TABLE_26, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "CPU_RING_INTERRUPT_COUNTER_TABLE", 1, CORE_26_INDEX, &CPU_RING_INTERRUPT_COUNTER_TABLE_26, 24, 1, 1 },
#endif
#if defined BCM6885
	{ "CPU_RX_CSO_UPDATE_FIFO_TABLE", 1, CORE_26_INDEX, &CPU_RX_CSO_UPDATE_FIFO_TABLE_26, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "RL_UPDATE_TASK_DESCRIPTOR", 1, CORE_26_INDEX, &RL_UPDATE_TASK_DESCRIPTOR_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "MIRRORING_SCRATCH", 1, CORE_26_INDEX, &MIRRORING_SCRATCH_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_CONTEXT_TABLE", 1, CORE_26_INDEX, &CSO_CONTEXT_TABLE_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_CPU_INT_ID", 1, CORE_26_INDEX, &DHD_CPU_INT_ID_26, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "CPU_RX_XPM_UPDATE_FIFO_TABLE", 1, CORE_26_INDEX, &CPU_RX_XPM_UPDATE_FIFO_TABLE_26, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_HW_CFG", 1, CORE_26_INDEX, &DHD_HW_CFG_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "MUTEX_TABLE", 1, CORE_26_INDEX, &MUTEX_TABLE_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR", 1, CORE_26_INDEX, &DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_TX_POST_0_STACK", 1, CORE_26_INDEX, &DHD_TX_POST_0_STACK_26, 168, 1, 1 },
#endif
#if defined BCM6885
	{ "CPU_REASON_TO_TC", 1, CORE_26_INDEX, &CPU_REASON_TO_TC_26, 64, 1, 1 },
#endif
#if defined BCM6885
	{ "CPU_RX_INTERRUPT_ID_DDR_ADDR", 1, CORE_26_INDEX, &CPU_RX_INTERRUPT_ID_DDR_ADDR_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_MIRRORING_DISPATCHER_CREDIT_TABLE", 1, CORE_26_INDEX, &DHD_MIRRORING_DISPATCHER_CREDIT_TABLE_26, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_TX_POST_1_STACK", 1, CORE_26_INDEX, &DHD_TX_POST_1_STACK_26, 168, 1, 1 },
#endif
#if defined BCM6885
	{ "TC_TO_CPU_RXQ", 1, CORE_26_INDEX, &TC_TO_CPU_RXQ_26, 64, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU0_TRANSMIT_TASK_WAKE_UP_BB_MSG", 1, CORE_26_INDEX, &GSU0_TRANSMIT_TASK_WAKE_UP_BB_MSG_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CPU_RX_AUX_INT_SCRATCHPAD", 1, CORE_26_INDEX, &CPU_RX_AUX_INT_SCRATCHPAD_26, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_TX_POST_2_STACK", 1, CORE_26_INDEX, &DHD_TX_POST_2_STACK_26, 168, 1, 1 },
#endif
#if defined BCM6885
	{ "EXC_TC_TO_CPU_RXQ", 1, CORE_26_INDEX, &EXC_TC_TO_CPU_RXQ_26, 64, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU1_TRANSMIT_TASK_WAKE_UP_BB_MSG", 1, CORE_26_INDEX, &GSU1_TRANSMIT_TASK_WAKE_UP_BB_MSG_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CPU_RX_CSO_DISPATCHER_CREDIT_TABLE", 1, CORE_26_INDEX, &CPU_RX_CSO_DISPATCHER_CREDIT_TABLE_26, 3, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_26_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CPU_RX_STACK", 1, CORE_26_INDEX, &CPU_RX_STACK_26, 136, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_SCRATCHPAD", 1, CORE_26_INDEX, &DEBUG_SCRATCHPAD_26, 12, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU2_TRANSMIT_TASK_WAKE_UP_BB_MSG", 1, CORE_26_INDEX, &GSU2_TRANSMIT_TASK_WAKE_UP_BB_MSG_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CPU_RXQ_DATA_BUF_TYPE_TABLE", 1, CORE_26_INDEX, &CPU_RXQ_DATA_BUF_TYPE_TABLE_26, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_GLOBAL_CFG", 1, CORE_26_INDEX, &FPM_GLOBAL_CFG_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PKTGEN_BAD_GET_NEXT", 1, CORE_26_INDEX, &PKTGEN_BAD_GET_NEXT_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_FLOW_RING_CACHE_LKP_TABLE", 1, CORE_26_INDEX, &DHD_FLOW_RING_CACHE_LKP_TABLE_26, 64, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_L2_HEADER", 1, CORE_26_INDEX, &DHD_L2_HEADER_26, 96, 1, 1 },
#endif
#if defined BCM6885
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_26_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PKTGEN_MAX_UT_PKTS", 1, CORE_26_INDEX, &PKTGEN_MAX_UT_PKTS_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU0_TRANSMIT_TASK_DESCRIPTOR", 1, CORE_26_INDEX, &GSU0_TRANSMIT_TASK_DESCRIPTOR_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CPU_RX_CSO_PD_FIFO_TABLE", 1, CORE_26_INDEX, &CPU_RX_CSO_PD_FIFO_TABLE_26, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "CPU_REASON_AND_VPORT_TO_METER_TABLE", 1, CORE_26_INDEX, &CPU_REASON_AND_VPORT_TO_METER_TABLE_26, 96, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU1_TRANSMIT_TASK_DESCRIPTOR", 1, CORE_26_INDEX, &GSU1_TRANSMIT_TASK_DESCRIPTOR_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU2_TRANSMIT_TASK_DESCRIPTOR", 1, CORE_26_INDEX, &GSU2_TRANSMIT_TASK_DESCRIPTOR_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU_SETTING_TABLE", 1, CORE_26_INDEX, &GSU_SETTING_TABLE_26, 5, 1, 1 },
#endif
#if defined BCM6885
	{ "PKTGEN_BBMSG_REPLY_SCRATCH", 1, CORE_26_INDEX, &PKTGEN_BBMSG_REPLY_SCRATCH_26, 2, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU0_TRANSMIT_TASK_STATUS_BB_MSG", 1, CORE_26_INDEX, &GSU0_TRANSMIT_TASK_STATUS_BB_MSG_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_TABLE", 1, CORE_26_INDEX, &DEBUG_PRINT_TABLE_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CPU_INTERRUPT_COALESCING_TABLE", 1, CORE_26_INDEX, &CPU_INTERRUPT_COALESCING_TABLE_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU0_UPDATE_FIFO_TABLE", 1, CORE_26_INDEX, &GSU0_UPDATE_FIFO_TABLE_26, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU1_UPDATE_FIFO_TABLE", 1, CORE_26_INDEX, &GSU1_UPDATE_FIFO_TABLE_26, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU2_UPDATE_FIFO_TABLE", 1, CORE_26_INDEX, &GSU2_UPDATE_FIFO_TABLE_26, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU0_TRANSMIT_PD", 1, CORE_26_INDEX, &GSU0_TRANSMIT_PD_26, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU1_TRANSMIT_PD", 1, CORE_26_INDEX, &GSU1_TRANSMIT_PD_26, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "CPU_RING_DESCRIPTORS_TABLE", 1, CORE_26_INDEX, &CPU_RING_DESCRIPTORS_TABLE_26, 24, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU2_TRANSMIT_PD", 1, CORE_26_INDEX, &GSU2_TRANSMIT_PD_26, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "REGISTERS_BUFFER", 1, CORE_26_INDEX, &REGISTERS_BUFFER_26, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_CPU_TX_POST_RING_DESCRIPTOR_TABLE", 1, CORE_26_INDEX, &DHD_CPU_TX_POST_RING_DESCRIPTOR_TABLE_26, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "PKTGEN_FPM_UG_MGMT", 1, CORE_26_INDEX, &PKTGEN_FPM_UG_MGMT_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PKTGEN_UT_TRIGGER", 1, CORE_26_INDEX, &PKTGEN_UT_TRIGGER_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CPU_RX_LOCAL_SCRATCH", 1, CORE_26_INDEX, &CPU_RX_LOCAL_SCRATCH_26, 2, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU1_TRANSMIT_TASK_STATUS_BB_MSG", 1, CORE_26_INDEX, &GSU1_TRANSMIT_TASK_STATUS_BB_MSG_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU2_TRANSMIT_TASK_STATUS_BB_MSG", 1, CORE_26_INDEX, &GSU2_TRANSMIT_TASK_STATUS_BB_MSG_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU0_UPDATE_FIFO_TASK_DESCRIPTOR", 1, CORE_26_INDEX, &GSU0_UPDATE_FIFO_TASK_DESCRIPTOR_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE", 1, CORE_26_INDEX, &DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE_26, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "VPORT_TO_CPU_OBJ", 1, CORE_26_INDEX, &VPORT_TO_CPU_OBJ_26, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "CORE_ID_TABLE", 1, CORE_26_INDEX, &CORE_ID_TABLE_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU1_UPDATE_FIFO_TASK_DESCRIPTOR", 1, CORE_26_INDEX, &GSU1_UPDATE_FIFO_TASK_DESCRIPTOR_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU2_UPDATE_FIFO_TASK_DESCRIPTOR", 1, CORE_26_INDEX, &GSU2_UPDATE_FIFO_TASK_DESCRIPTOR_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PKTGEN_CURR_SBPM_HDR_PTR", 1, CORE_26_INDEX, &PKTGEN_CURR_SBPM_HDR_PTR_26, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "PKTGEN_NUM_OF_AVAIL_SBPM_HDRS", 1, CORE_26_INDEX, &PKTGEN_NUM_OF_AVAIL_SBPM_HDRS_26, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "CPU_VPORT_TO_METER_TABLE", 1, CORE_26_INDEX, &CPU_VPORT_TO_METER_TABLE_26, 31, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_26_INDEX, &DEBUG_PRINT_CORE_LOCK_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PKTGEN_SBPM_END_PTR", 1, CORE_26_INDEX, &PKTGEN_SBPM_END_PTR_26, 4, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_MIRRORING_MINIFPM_PARAMS", 1, CORE_26_INDEX, &DHD_MIRRORING_MINIFPM_PARAMS_26, 2, 1, 1 },
#endif
#if defined BCM6885
	{ "RNR_WR_TEST_ADDR", 1, CORE_26_INDEX, &RNR_WR_TEST_ADDR_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "TASK_IDX", 1, CORE_26_INDEX, &TASK_IDX_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CPU_RX_INTERRUPT_SCRATCH", 1, CORE_26_INDEX, &CPU_RX_INTERRUPT_SCRATCH_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_26_INDEX, &FW_ERROR_VECTOR_TABLE_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RNR_WR_TEST_FROM", 1, CORE_26_INDEX, &RNR_WR_TEST_FROM_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RNR_WR_TEST_TO", 1, CORE_26_INDEX, &RNR_WR_TEST_TO_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SPDSVC_WLAN_TXPOST_PARAMS_TABLE", 1, CORE_26_INDEX, &SPDSVC_WLAN_TXPOST_PARAMS_TABLE_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SRAM_DUMMY_STORE", 1, CORE_26_INDEX, &SRAM_DUMMY_STORE_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CSO_DISABLE", 1, CORE_26_INDEX, &CSO_DISABLE_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CPU_RX_TELEMETRY_BUFFER_LOCATION", 1, CORE_26_INDEX, &CPU_RX_TELEMETRY_BUFFER_LOCATION_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "RNR_WR_TEST_START", 1, CORE_26_INDEX, &RNR_WR_TEST_START_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "PKTGEN_TX_STREAM_SCRATCH_TABLE", 1, CORE_26_INDEX, &PKTGEN_TX_STREAM_SCRATCH_TABLE_26, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SQ_TM_UPDATE_FIFO_TABLE", 1, CORE_27_INDEX, &SQ_TM_UPDATE_FIFO_TABLE_27, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_27_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_27, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU0_RL_UPDATE_RESPONSE", 1, CORE_27_INDEX, &GSU0_RL_UPDATE_RESPONSE_27, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SYSTEM_CONFIGURATION", 1, CORE_27_INDEX, &SYSTEM_CONFIGURATION_27, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "AQM_PROBABILITY_CALC_DESCRIPTOR", 1, CORE_27_INDEX, &AQM_PROBABILITY_CALC_DESCRIPTOR_27, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU0_TRANSMIT_TASK_WAKE_UP_BB_MSG", 1, CORE_27_INDEX, &GSU0_TRANSMIT_TASK_WAKE_UP_BB_MSG_27, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU0_AQM_UPDATE_STATUS_RESPONSE", 1, CORE_27_INDEX, &GSU0_AQM_UPDATE_STATUS_RESPONSE_27, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_27_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_27, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "MUTEX_TABLE", 1, CORE_27_INDEX, &MUTEX_TABLE_27, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU_SETTING_TABLE", 1, CORE_27_INDEX, &GSU_SETTING_TABLE_27, 5, 1, 1 },
#endif
#if defined BCM6885
	{ "DHD_FPM_THRESHOLDS", 1, CORE_27_INDEX, &DHD_FPM_THRESHOLDS_27, 3, 1, 1 },
#endif
#if defined BCM6885
	{ "BUFMNG_HOST_CNT_DISABLE_TABLE", 1, CORE_27_INDEX, &BUFMNG_HOST_CNT_DISABLE_TABLE_27, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_27_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_27, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "UPDATE_FIFO_GSU0_STACK", 1, CORE_27_INDEX, &UPDATE_FIFO_GSU0_STACK_27, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "TX_GSU0_STACK", 1, CORE_27_INDEX, &TX_GSU0_STACK_27, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "UPDATE_RL_STACK", 1, CORE_27_INDEX, &UPDATE_RL_STACK_27, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "AQM_QUEUE_TABLE", 1, CORE_27_INDEX, &AQM_QUEUE_TABLE_27, 145, 1, 1 },
#endif
#if defined BCM6885
	{ "TASK_IDX", 1, CORE_27_INDEX, &TASK_IDX_27, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU0_TRANSMIT_TASK_STATUS_BB_MSG", 1, CORE_27_INDEX, &GSU0_TRANSMIT_TASK_STATUS_BB_MSG_27, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU0_TRANSMIT_TASK_DESCRIPTOR", 1, CORE_27_INDEX, &GSU0_TRANSMIT_TASK_DESCRIPTOR_27, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SQ_TM_BUFMNG_STATUS_TABLE", 1, CORE_27_INDEX, &SQ_TM_BUFMNG_STATUS_TABLE_27, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU0_UPDATE_FIFO_TABLE", 1, CORE_27_INDEX, &GSU0_UPDATE_FIFO_TABLE_27, 8, 1, 1 },
#endif
#if defined BCM6885
	{ "UPDATE_AQM_STACK", 1, CORE_27_INDEX, &UPDATE_AQM_STACK_27, 256, 1, 1 },
#endif
#if defined BCM6885
	{ "SQ_TM_PD_FIFO_TABLE", 1, CORE_27_INDEX, &SQ_TM_PD_FIFO_TABLE_27, 64, 1, 1 },
#endif
#if defined BCM6885
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_27_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_27, 128, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU0_TRANSMIT_PD", 1, CORE_27_INDEX, &GSU0_TRANSMIT_PD_27, 16, 1, 1 },
#endif
#if defined BCM6885
	{ "REGISTERS_BUFFER", 1, CORE_27_INDEX, &REGISTERS_BUFFER_27, 32, 1, 1 },
#endif
#if defined BCM6885
	{ "RL_UPDATE_TASK_DESCRIPTOR", 1, CORE_27_INDEX, &RL_UPDATE_TASK_DESCRIPTOR_27, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_SCRATCHPAD", 1, CORE_27_INDEX, &DEBUG_SCRATCHPAD_27, 12, 1, 1 },
#endif
#if defined BCM6885
	{ "GSU0_UPDATE_FIFO_TASK_DESCRIPTOR", 1, CORE_27_INDEX, &GSU0_UPDATE_FIFO_TASK_DESCRIPTOR_27, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_TABLE", 1, CORE_27_INDEX, &DEBUG_PRINT_TABLE_27, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FPM_GLOBAL_CFG", 1, CORE_27_INDEX, &FPM_GLOBAL_CFG_27, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "CORE_ID_TABLE", 1, CORE_27_INDEX, &CORE_ID_TABLE_27, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_27_INDEX, &DEBUG_PRINT_CORE_LOCK_27, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "SRAM_DUMMY_STORE", 1, CORE_27_INDEX, &SRAM_DUMMY_STORE_27, 1, 1, 1 },
#endif
#if defined BCM6885
	{ "FW_ERROR_VECTOR_TABLE", 1, CORE_27_INDEX, &FW_ERROR_VECTOR_TABLE_27, 1, 1, 1 },
#endif
};

TABLE_STACK_STRUCT RUNNER_STACK_TABLES[NUMBER_OF_STACK_TABLES] =
{
    { "PROCESSING0_STACK", CORE_0_INDEX, 0x200, 360},
    { "PROCESSING1_STACK", CORE_0_INDEX, 0x600, 360},
    { "PROCESSING2_STACK", CORE_0_INDEX, 0x800, 360},
    { "PROCESSING3_STACK", CORE_0_INDEX, 0xa00, 360},
    { "PROCESSING4_STACK", CORE_0_INDEX, 0xc00, 360},
    { "PROCESSING5_STACK", CORE_0_INDEX, 0xe00, 360},
    { "PROCESSING6_STACK", CORE_0_INDEX, 0x2000, 360},
    { "PROCESSING7_STACK", CORE_0_INDEX, 0x2200, 360},
    { "PROCESSING0_STACK", CORE_1_INDEX, 0x200, 360},
    { "PROCESSING1_STACK", CORE_1_INDEX, 0x600, 360},
    { "PROCESSING2_STACK", CORE_1_INDEX, 0x800, 360},
    { "PROCESSING3_STACK", CORE_1_INDEX, 0xa00, 360},
    { "PROCESSING4_STACK", CORE_1_INDEX, 0xc00, 360},
    { "PROCESSING5_STACK", CORE_1_INDEX, 0xe00, 360},
    { "PROCESSING6_STACK", CORE_1_INDEX, 0x2000, 360},
    { "PROCESSING7_STACK", CORE_1_INDEX, 0x2200, 360},
    { "PROCESSING0_STACK", CORE_2_INDEX, 0x200, 360},
    { "PROCESSING1_STACK", CORE_2_INDEX, 0x600, 360},
    { "PROCESSING2_STACK", CORE_2_INDEX, 0x800, 360},
    { "PROCESSING3_STACK", CORE_2_INDEX, 0xa00, 360},
    { "PROCESSING4_STACK", CORE_2_INDEX, 0xc00, 360},
    { "PROCESSING5_STACK", CORE_2_INDEX, 0xe00, 360},
    { "PROCESSING6_STACK", CORE_2_INDEX, 0x2000, 360},
    { "PROCESSING7_STACK", CORE_2_INDEX, 0x2200, 360},
    { "PROCESSING0_STACK", CORE_3_INDEX, 0x200, 360},
    { "PROCESSING1_STACK", CORE_3_INDEX, 0x600, 360},
    { "PROCESSING2_STACK", CORE_3_INDEX, 0x800, 360},
    { "PROCESSING3_STACK", CORE_3_INDEX, 0xa00, 360},
    { "PROCESSING4_STACK", CORE_3_INDEX, 0xc00, 360},
    { "PROCESSING5_STACK", CORE_3_INDEX, 0xe00, 360},
    { "PROCESSING6_STACK", CORE_3_INDEX, 0x2000, 360},
    { "PROCESSING7_STACK", CORE_3_INDEX, 0x2200, 360},
    { "PROCESSING0_STACK", CORE_4_INDEX, 0x200, 360},
    { "PROCESSING1_STACK", CORE_4_INDEX, 0x600, 360},
    { "PROCESSING2_STACK", CORE_4_INDEX, 0x800, 360},
    { "PROCESSING3_STACK", CORE_4_INDEX, 0xa00, 360},
    { "PROCESSING4_STACK", CORE_4_INDEX, 0xc00, 360},
    { "PROCESSING5_STACK", CORE_4_INDEX, 0xe00, 360},
    { "PROCESSING6_STACK", CORE_4_INDEX, 0x2000, 360},
    { "PROCESSING7_STACK", CORE_4_INDEX, 0x2200, 360},
    { "PROCESSING0_STACK", CORE_5_INDEX, 0x200, 360},
    { "PROCESSING1_STACK", CORE_5_INDEX, 0x600, 360},
    { "PROCESSING2_STACK", CORE_5_INDEX, 0x800, 360},
    { "PROCESSING3_STACK", CORE_5_INDEX, 0xa00, 360},
    { "PROCESSING4_STACK", CORE_5_INDEX, 0xc00, 360},
    { "PROCESSING5_STACK", CORE_5_INDEX, 0xe00, 360},
    { "PROCESSING6_STACK", CORE_5_INDEX, 0x2000, 360},
    { "PROCESSING7_STACK", CORE_5_INDEX, 0x2200, 360},
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
    { "PROCESSING0_STACK", CORE_9_INDEX, 0x200, 360},
    { "PROCESSING1_STACK", CORE_9_INDEX, 0x600, 360},
    { "PROCESSING2_STACK", CORE_9_INDEX, 0x800, 360},
    { "PROCESSING3_STACK", CORE_9_INDEX, 0xa00, 360},
    { "PROCESSING4_STACK", CORE_9_INDEX, 0xc00, 360},
    { "PROCESSING5_STACK", CORE_9_INDEX, 0xe00, 360},
    { "PROCESSING6_STACK", CORE_9_INDEX, 0x2000, 360},
    { "PROCESSING7_STACK", CORE_9_INDEX, 0x2200, 360},
    { "PROCESSING0_STACK", CORE_10_INDEX, 0x200, 360},
    { "PROCESSING1_STACK", CORE_10_INDEX, 0x600, 360},
    { "PROCESSING2_STACK", CORE_10_INDEX, 0x800, 360},
    { "PROCESSING3_STACK", CORE_10_INDEX, 0xa00, 360},
    { "PROCESSING4_STACK", CORE_10_INDEX, 0xc00, 360},
    { "PROCESSING5_STACK", CORE_10_INDEX, 0xe00, 360},
    { "PROCESSING6_STACK", CORE_10_INDEX, 0x2000, 360},
    { "PROCESSING7_STACK", CORE_10_INDEX, 0x2200, 360},
    { "PROCESSING0_STACK", CORE_11_INDEX, 0x200, 360},
    { "PROCESSING1_STACK", CORE_11_INDEX, 0x600, 360},
    { "PROCESSING2_STACK", CORE_11_INDEX, 0x800, 360},
    { "PROCESSING3_STACK", CORE_11_INDEX, 0xa00, 360},
    { "PROCESSING4_STACK", CORE_11_INDEX, 0xc00, 360},
    { "PROCESSING5_STACK", CORE_11_INDEX, 0xe00, 360},
    { "PROCESSING6_STACK", CORE_11_INDEX, 0x2000, 360},
    { "PROCESSING7_STACK", CORE_11_INDEX, 0x2200, 360},
    { "PROCESSING0_STACK", CORE_12_INDEX, 0x200, 360},
    { "PROCESSING1_STACK", CORE_12_INDEX, 0x600, 360},
    { "PROCESSING2_STACK", CORE_12_INDEX, 0x800, 360},
    { "PROCESSING3_STACK", CORE_12_INDEX, 0xa00, 360},
    { "PROCESSING4_STACK", CORE_12_INDEX, 0xc00, 360},
    { "PROCESSING5_STACK", CORE_12_INDEX, 0xe00, 360},
    { "PROCESSING6_STACK", CORE_12_INDEX, 0x2000, 360},
    { "PROCESSING7_STACK", CORE_12_INDEX, 0x2200, 360},
    { "PROCESSING0_STACK", CORE_13_INDEX, 0x200, 360},
    { "PROCESSING1_STACK", CORE_13_INDEX, 0x600, 360},
    { "PROCESSING2_STACK", CORE_13_INDEX, 0x800, 360},
    { "PROCESSING3_STACK", CORE_13_INDEX, 0xa00, 360},
    { "PROCESSING4_STACK", CORE_13_INDEX, 0xc00, 360},
    { "PROCESSING5_STACK", CORE_13_INDEX, 0xe00, 360},
    { "PROCESSING6_STACK", CORE_13_INDEX, 0x2000, 360},
    { "PROCESSING7_STACK", CORE_13_INDEX, 0x2200, 360},
    { "PROCESSING0_STACK", CORE_14_INDEX, 0x200, 360},
    { "DIRECT_FLOW_STACK", CORE_14_INDEX, 0x380, 128},
    { "PROCESSING1_STACK", CORE_14_INDEX, 0x600, 360},
    { "PROCESSING2_STACK", CORE_14_INDEX, 0x800, 360},
    { "PROCESSING3_STACK", CORE_14_INDEX, 0xa00, 360},
    { "PROCESSING4_STACK", CORE_14_INDEX, 0xc00, 360},
    { "PROCESSING5_STACK", CORE_14_INDEX, 0xe00, 360},
    { "PROCESSING6_STACK", CORE_14_INDEX, 0x2000, 360},
    { "PROCESSING7_STACK", CORE_14_INDEX, 0x2200, 360},
    { "PROCESSING0_STACK", CORE_15_INDEX, 0x200, 360},
    { "PROCESSING1_STACK", CORE_15_INDEX, 0x600, 360},
    { "PROCESSING2_STACK", CORE_15_INDEX, 0x800, 360},
    { "PROCESSING3_STACK", CORE_15_INDEX, 0xa00, 360},
    { "PROCESSING4_STACK", CORE_15_INDEX, 0xc00, 360},
    { "PROCESSING5_STACK", CORE_15_INDEX, 0xe00, 360},
    { "PROCESSING6_STACK", CORE_15_INDEX, 0x2000, 360},
    { "PROCESSING7_STACK", CORE_15_INDEX, 0x2200, 360},
    { "PROCESSING0_STACK", CORE_16_INDEX, 0xa00, 360},
    { "PROCESSING1_STACK", CORE_16_INDEX, 0xc00, 360},
    { "TCPSPDTEST_UPLOAD_STACK", CORE_16_INDEX, 0xd80, 128},
    { "PROCESSING2_STACK", CORE_16_INDEX, 0xe00, 360},
    { "PROCESSING3_STACK", CORE_16_INDEX, 0x2000, 360},
    { "PROCESSING4_STACK", CORE_16_INDEX, 0x2200, 360},
    { "TCPSPDTEST_UPLOAD_TIMER_STACK", CORE_16_INDEX, 0x2380, 64},
    { "PROCESSING5_STACK", CORE_16_INDEX, 0x2400, 360},
    { "PROCESSING6_STACK", CORE_16_INDEX, 0x2600, 360},
    { "PROCESSING7_STACK", CORE_16_INDEX, 0x2800, 360},
    { "TCPSPDTEST_DOWNLOAD_STACK", CORE_16_INDEX, 0x2b00, 240},
    { "SPDSVC_ANALYZER_STACK", CORE_16_INDEX, 0x2d00, 176},
    { "PROCESSING0_STACK", CORE_17_INDEX, 0x600, 360},
    { "PROCESSING1_STACK", CORE_17_INDEX, 0xa00, 360},
    { "PROCESSING2_STACK", CORE_17_INDEX, 0xc00, 360},
    { "PROCESSING3_STACK", CORE_17_INDEX, 0xe00, 360},
    { "GENERAL_TIMER_STACK", CORE_17_INDEX, 0xf80, 72},
    { "PROCESSING4_STACK", CORE_17_INDEX, 0x2000, 360},
    { "PROCESSING5_STACK", CORE_17_INDEX, 0x2200, 360},
    { "PROCESSING6_STACK", CORE_17_INDEX, 0x2400, 360},
    { "PROCESSING7_STACK", CORE_17_INDEX, 0x2600, 360},
    { "CPU_RECYCLE_STACK", CORE_17_INDEX, 0x27e0, 32},
    { "CPU_TX_0_STACK", CORE_17_INDEX, 0x2900, 224},
    { "CPU_TX_1_STACK", CORE_17_INDEX, 0x2b00, 224},
    { "PROCESSING0_STACK", CORE_18_INDEX, 0xa00, 360},
    { "PROCESSING1_STACK", CORE_18_INDEX, 0xc00, 360},
    { "PROCESSING2_STACK", CORE_18_INDEX, 0xe00, 360},
    { "DHD_RX_COMPLETE_0_STACK", CORE_18_INDEX, 0xf80, 128},
    { "PROCESSING3_STACK", CORE_18_INDEX, 0x2000, 360},
    { "DHD_RX_COMPLETE_1_STACK", CORE_18_INDEX, 0x2180, 128},
    { "PROCESSING4_STACK", CORE_18_INDEX, 0x2200, 360},
    { "DHD_RX_COMPLETE_2_STACK", CORE_18_INDEX, 0x2380, 128},
    { "PROCESSING5_STACK", CORE_18_INDEX, 0x2400, 360},
    { "PROCESSING6_STACK", CORE_18_INDEX, 0x2600, 360},
    { "PROCESSING7_STACK", CORE_18_INDEX, 0x2800, 360},
    { "DHD_TX_COMPLETE_0_STACK", CORE_18_INDEX, 0x2dc0, 64},
    { "DHD_TX_COMPLETE_1_STACK", CORE_18_INDEX, 0x2e40, 64},
    { "DHD_TX_COMPLETE_2_STACK", CORE_18_INDEX, 0x2ec0, 64},
    { "PROCESSING0_STACK", CORE_19_INDEX, 0xa00, 360},
    { "PROCESSING1_STACK", CORE_19_INDEX, 0xc00, 360},
    { "PROCESSING2_STACK", CORE_19_INDEX, 0xe00, 360},
    { "DHD_RX_COMPLETE_0_STACK", CORE_19_INDEX, 0xf80, 128},
    { "PROCESSING3_STACK", CORE_19_INDEX, 0x2000, 360},
    { "DHD_RX_COMPLETE_1_STACK", CORE_19_INDEX, 0x2180, 128},
    { "PROCESSING4_STACK", CORE_19_INDEX, 0x2200, 360},
    { "DHD_RX_COMPLETE_2_STACK", CORE_19_INDEX, 0x2380, 128},
    { "PROCESSING5_STACK", CORE_19_INDEX, 0x2400, 360},
    { "PROCESSING6_STACK", CORE_19_INDEX, 0x2600, 360},
    { "PROCESSING7_STACK", CORE_19_INDEX, 0x2800, 360},
    { "DHD_TX_COMPLETE_0_STACK", CORE_19_INDEX, 0x2dc0, 64},
    { "DHD_TX_COMPLETE_1_STACK", CORE_19_INDEX, 0x2e40, 64},
    { "DHD_TX_COMPLETE_2_STACK", CORE_19_INDEX, 0x2ec0, 64},
    { "PROCESSING0_STACK", CORE_20_INDEX, 0x200, 360},
    { "PROCESSING1_STACK", CORE_20_INDEX, 0x600, 360},
    { "PROCESSING2_STACK", CORE_20_INDEX, 0x800, 360},
    { "PROCESSING3_STACK", CORE_20_INDEX, 0xa00, 360},
    { "PROCESSING4_STACK", CORE_20_INDEX, 0xc00, 360},
    { "PROCESSING5_STACK", CORE_20_INDEX, 0xe00, 360},
    { "PROCESSING6_STACK", CORE_20_INDEX, 0x2000, 360},
    { "PROCESSING7_STACK", CORE_20_INDEX, 0x2200, 360},
    { "PROCESSING0_STACK", CORE_21_INDEX, 0x200, 360},
    { "PROCESSING1_STACK", CORE_21_INDEX, 0x600, 360},
    { "PROCESSING2_STACK", CORE_21_INDEX, 0x800, 360},
    { "PROCESSING3_STACK", CORE_21_INDEX, 0xa00, 360},
    { "PROCESSING4_STACK", CORE_21_INDEX, 0xc00, 360},
    { "PROCESSING5_STACK", CORE_21_INDEX, 0xe00, 360},
    { "PROCESSING6_STACK", CORE_21_INDEX, 0x2000, 360},
    { "PROCESSING7_STACK", CORE_21_INDEX, 0x2200, 360},
    { "PROCESSING0_STACK", CORE_22_INDEX, 0x200, 360},
    { "PROCESSING1_STACK", CORE_22_INDEX, 0x600, 360},
    { "PROCESSING2_STACK", CORE_22_INDEX, 0x800, 360},
    { "PROCESSING3_STACK", CORE_22_INDEX, 0xa00, 360},
    { "PROCESSING4_STACK", CORE_22_INDEX, 0xc00, 360},
    { "PROCESSING5_STACK", CORE_22_INDEX, 0xe00, 360},
    { "PROCESSING6_STACK", CORE_22_INDEX, 0x2000, 360},
    { "PROCESSING7_STACK", CORE_22_INDEX, 0x2200, 360},
    { "PROCESSING0_STACK", CORE_23_INDEX, 0x200, 360},
    { "PROCESSING1_STACK", CORE_23_INDEX, 0x600, 360},
    { "PROCESSING2_STACK", CORE_23_INDEX, 0x800, 360},
    { "PROCESSING3_STACK", CORE_23_INDEX, 0xa00, 360},
    { "PROCESSING4_STACK", CORE_23_INDEX, 0xc00, 360},
    { "PROCESSING5_STACK", CORE_23_INDEX, 0xe00, 360},
    { "PROCESSING6_STACK", CORE_23_INDEX, 0x2000, 360},
    { "PROCESSING7_STACK", CORE_23_INDEX, 0x2200, 360},
    { "UPDATE_FIFO_GSU0_STACK", CORE_24_INDEX, 0x100, 256},
    { "UPDATE_FIFO_GSU1_STACK", CORE_24_INDEX, 0x200, 256},
    { "UPDATE_FIFO_GSU2_STACK", CORE_24_INDEX, 0x300, 256},
    { "UPDATE_FIFO_GSU3_STACK", CORE_24_INDEX, 0x700, 256},
    { "UPDATE_FIFO_GSU4_STACK", CORE_24_INDEX, 0xa00, 256},
    { "TX_GSU0_STACK", CORE_24_INDEX, 0xb00, 256},
    { "TX_GSU1_STACK", CORE_24_INDEX, 0xc00, 256},
    { "TX_GSU2_STACK", CORE_24_INDEX, 0xd00, 256},
    { "TX_GSU3_STACK", CORE_24_INDEX, 0xe00, 256},
    { "TX_GSU4_STACK", CORE_24_INDEX, 0xf00, 256},
    { "UPDATE_RL_STACK", CORE_24_INDEX, 0x1000, 256},
    { "UPDATE_AQM_STACK", CORE_24_INDEX, 0x1100, 256},
    { "UPDATE_FIFO_GSU0_STACK", CORE_25_INDEX, 0x100, 256},
    { "TX_GSU0_STACK", CORE_25_INDEX, 0x200, 256},
    { "UPDATE_RL_STACK", CORE_25_INDEX, 0x300, 256},
    { "UPDATE_AQM_STACK", CORE_25_INDEX, 0x700, 256},
    { "CPU_RX_CSO_STACK", CORE_26_INDEX, 0x80, 128},
    { "UPDATE_FIFO_GSU0_STACK", CORE_26_INDEX, 0x100, 256},
    { "UPDATE_FIFO_GSU1_STACK", CORE_26_INDEX, 0x200, 256},
    { "UPDATE_FIFO_GSU2_STACK", CORE_26_INDEX, 0x300, 256},
    { "TX_GSU0_STACK", CORE_26_INDEX, 0x1700, 256},
    { "DHD_TIMER_STACK", CORE_26_INDEX, 0x1a20, 32},
    { "TX_GSU1_STACK", CORE_26_INDEX, 0x1b00, 256},
    { "TX_GSU2_STACK", CORE_26_INDEX, 0x1e00, 256},
    { "UPDATE_RL_STACK", CORE_26_INDEX, 0x1f00, 256},
    { "UPDATE_AQM_STACK", CORE_26_INDEX, 0x2000, 256},
    { "DHD_TX_POST_UPDATE_FIFO_STACK", CORE_26_INDEX, 0x23e0, 32},
    { "DHD_TX_POST_0_STACK", CORE_26_INDEX, 0x2700, 168},
    { "DHD_TX_POST_1_STACK", CORE_26_INDEX, 0x2800, 168},
    { "DHD_TX_POST_2_STACK", CORE_26_INDEX, 0x2900, 168},
    { "CPU_RX_STACK", CORE_26_INDEX, 0x2a00, 136},
    { "UPDATE_FIFO_GSU0_STACK", CORE_27_INDEX, 0x100, 256},
    { "TX_GSU0_STACK", CORE_27_INDEX, 0x200, 256},
    { "UPDATE_RL_STACK", CORE_27_INDEX, 0x300, 256},
    { "UPDATE_AQM_STACK", CORE_27_INDEX, 0x700, 256}
};
