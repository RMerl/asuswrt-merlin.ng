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
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE =
{
	8,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x2a8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, 0x2b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, 0x2c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SPDSVC_ANALYZER_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_COMPLETE_COMMON_RADIO_DATA =
{
	120,
	{
		{ dump_RDD_DHD_COMPLETE_COMMON_RADIO_ENTRY, 0x600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0x768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x770 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_0_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_0_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x940 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_1_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xa00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_1_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xb40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_2_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xb80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_2_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xd40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_FLOW_RING_BUFFER =
{
	32,
	{
		{ dump_RDD_DHD_RX_COMPLETE_DESCRIPTOR, 0xd80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xde0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_FLOW_RING_BUFFER =
{
	16,
	{
		{ dump_RDD_DHD_TX_COMPLETE_DESCRIPTOR, 0xf40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0xf70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_RX_POST_FLOW_RING_BUFFER =
{
	32,
	{
		{ dump_RDD_DHD_RX_POST_DESCRIPTOR, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0xfe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_DISPATCHER_CREDIT_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2140 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG =
{
	16,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, 0x2170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE =
{
	1,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, 0x2180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x21d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_DISPATCHER_CREDIT_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2340 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SPDSVC_ANALYZER_PARAMS_TABLE =
{
	16,
	{
		{ dump_RDD_SPDSVC_ANALYZER_PARAMS, 0x2370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SPDSVC_ANALYZER_PD_FIFO_TABLE =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x2380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT VLAN_ACTION_GPE_HANDLER_PTR_TABLE =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x23c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CODEL_NUM_QUEUES =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x23e2 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x23e4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION =
{
	5,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x23e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_UDP_RX_FLOWS =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x23ed },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x23ee },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT BRIDGE_CFG_TABLE =
{
	8,
	{
		{ dump_RDD_BRIDGE_CFG, 0x23f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2540 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SPDSVC_ANALYZER_UPDATE_FIFO_TABLE =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x25c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CODEL_ENABLE_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x25e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT TASK_IDX =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x25f4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, 0x25f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_FPM_REPLY =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2740 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, 0x2758 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2760 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SPDSVC_ANALYZER_DISPATCHER_CREDIT_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2770 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT ENABLE_VPORT_MASK =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x277c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x2780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT IPTV_CONFIGURATION_TABLE =
{
	12,
	{
		{ dump_RDD_HW_IPTV_CONFIGURATION, 0x27b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x27bc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT FORCE_DSCP =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x27bd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x27be },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_REDIRECT_MODE =
{
	1,
	{
		{ dump_RDD_CPU_REDIRECT_MODE_ENTRY, 0x27bf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x27c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x27d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_RX_POST_RING_SIZE =
{
	2,
	{
		{ dump_RDD_DHD_RING_SIZE, 0x27dc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_RX_COMPLETE_RING_SIZE =
{
	2,
	{
		{ dump_RDD_DHD_RING_SIZE, 0x27de },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_DOORBELL_TX_COMPLETE_VALUE =
{
	4,
	{
		{ dump_RDD_DHD_DOORBELL, 0x27e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_TX_COMPLETE_RING_SIZE =
{
	2,
	{
		{ dump_RDD_DHD_RING_SIZE, 0x27ec },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x27ee },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_DOORBELL_RX_COMPLETE_VALUE =
{
	4,
	{
		{ dump_RDD_DHD_DOORBELL, 0x27f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x27fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x27fd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT RATE_LIMIT_OVERHEAD =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x27fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x27ff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x28d4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x28d5 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x28d8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_DOORBELL_RX_POST_VALUE =
{
	4,
	{
		{ dump_RDD_DHD_DOORBELL, 0x28e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_CPU_INT_ID =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x28f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, 0x2900 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SPDSVC_WLAN_GEN_PARAMS_TABLE =
{
	10,
	{
		{ dump_RDD_SPDSVC_WLAN_GEN_PARAMS, 0x2980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_DESCRIPTOR_TABLE =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x2990 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT UDPSPDT_SCRATCH_TABLE =
{
	8,
	{
		{ dump_RDD_UDPSPDT_SCRATCH_IPERF3_RX, 0x29a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, 0x29a8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, 0x29b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, 0x29c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG =
{
	16,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, 0x29d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT MULTICAST_WHITELIST_CFG_TABLE =
{
	12,
	{
		{ dump_RDD_MULTICAST_WHITELIST_CFG, 0x29e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x29f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, 0x29f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT UDPSPDT_STREAM_RX_STAT_TABLE =
{
	48,
	{
		{ dump_RDD_UDPSPDT_STREAM_RX_STAT, 0x2a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, 0x2ac0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2ac8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, 0x2ad0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2ad8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT NATC_TBL_CFG =
{
	24,
	{
		{ dump_RDD_NATC_TBL_CONFIGURATION, 0x2b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT FC_FLOW_IP_ADDRESSES_TABLE =
{
	48,
	{
		{ dump_RDD_FC_FLOW_IP_ADDRESSES_ENTRY, 0x3d00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_1 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_1 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0x2a8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_INTERRUPT_COALESCING_TABLE_1 =
{
	16,
	{
		{ dump_RDD_CPU_INTERRUPT_COALESCING_ENTRY, 0x2b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DS_CPU_REASON_TO_METER_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_1 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, 0x300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_INT_INTERRUPT_SCRATCH_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3d4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_RX_INTERRUPT_ID_DDR_ADDR_1 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0x3d8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_RSV_TABLE_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_RX_SCRATCHPAD_1 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_STREAM_TABLE_1 =
{
	384,
	{
		{ dump_RDD_TCPSPDTEST_STREAM, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT US_CPU_REASON_TO_METER_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xf40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_CACHE_TABLE_1 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0xf80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER_1 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PKTGEN_TX_STREAM_TABLE_1 =
{
	136,
	{
		{ dump_RDD_PKTGEN_TX_STREAM_ENTRY, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT IPV4_HOST_ADDRESS_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2220 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT IPV6_HOST_ADDRESS_CRC_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2240 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DS_CPU_RX_METER_TABLE_1 =
{
	8,
	{
		{ dump_RDD_CPU_RX_METER_ENTRY, 0x2280 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CSO_CONTEXT_TABLE_1 =
{
	152,
	{
		{ dump_RDD_CSO_CONTEXT_ENTRY, 0x2300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_1 =
{
	8,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x2398 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_RX_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x23a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_1 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, 0x23c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_1 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SPDSVC_GEN_PARAMS_TABLE_1 =
{
	60,
	{
		{ dump_RDD_SPDSVC_GEN_PARAMS, 0x2740 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PKTGEN_NO_SBPM_HDRS_CNTR_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x277c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT US_CPU_RX_METER_TABLE_1 =
{
	8,
	{
		{ dump_RDD_CPU_RX_METER_ENTRY, 0x2780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_REASON_AND_VPORT_TO_METER_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2940 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_RX_COPY_INT_SCRATCHPAD_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_RX_COPY_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT VLAN_ACTION_GPE_HANDLER_PTR_TABLE_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2b40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CODEL_NUM_QUEUES_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2b62 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2b64 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT TX_ABS_RECYCLE_COUNTERS_1 =
{
	8,
	{
		{ dump_RDD_TX_ABS_RECYCLE_COUNTERS_ENTRY, 0x2b68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_RX_PSRAM_GET_NEXT_SCRATCHPAD_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2b70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_SCRATCHPAD_1 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x2b80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_REASON_TO_TC_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2d40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_1 =
{
	8,
	{
		{ dump_RDD_GENERAL_TIMER_ENTRY, 0x2d80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT TC_TO_CPU_RXQ_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2f40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SPDSVC_GEN_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2f80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_RING_DESCRIPTORS_TABLE_1 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x3000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_RING_INTERRUPT_COUNTER_TABLE_1 =
{
	8,
	{
		{ dump_RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY, 0x3100 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT EXC_TC_TO_CPU_RXQ_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3190 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_1 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, 0x31d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x31e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PKTGEN_SESSION_DATA_1 =
{
	32,
	{
		{ dump_RDD_PKTGEN_TX_PARAMS, 0x3340 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3360 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_1 =
{
	1,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, 0x3380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x33d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x33e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3540 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x35c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT COMMON_REPROCESSING_STACK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x35e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_SRAM_PD_FIFO_1 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x3600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_RX_COPY_PD_FIFO_TABLE_1 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x3700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_PD_FIFO_TABLE_1 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x3780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT UPDATE_FIFO_TABLE_1 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x37c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CODEL_ENABLE_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x37e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_RX_INTERRUPT_SCRATCH_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x37f4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_1 =
{
	5,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x37f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_UDP_RX_FLOWS_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x37fd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PKTGEN_CURR_SBPM_HDR_PTR_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x37fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT UDPSPDT_STREAM_TX_STAT_TABLE_1 =
{
	40,
	{
		{ dump_RDD_UDPSPDT_STREAM_TX_STAT, 0x3800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_ENGINE_GLOBAL_TABLE_1 =
{
	20,
	{
		{ dump_RDD_TCPSPDTEST_ENGINE_GLOBAL_INFO, 0x38a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PKTGEN_NUM_OF_AVAIL_SBPM_HDRS_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x38b4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PKTGEN_SBPM_END_PTR_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x38b6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE_1 =
{
	8,
	{
		{ dump_RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY, 0x38b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_RX_COPY_UPDATE_FIFO_TABLE_1 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x38c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_1 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0x38e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT BRIDGE_CFG_TABLE_1 =
{
	8,
	{
		{ dump_RDD_BRIDGE_CFG, 0x38f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_1 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, 0x3900 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PKTGEN_SBPM_HDR_BNS_1 =
{
	2,
	{
		{ dump_RDD_PKTGEN_SBPM_HDR_BN, 0x3980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_INDEX_DDR_ADDR_TABLE_1 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0x39b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT COMMON_REPROCESSING_UPDATE_FIFO_TABLE_1 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x39c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x39e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_RX_COPY_DISPATCHER_CREDIT_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x39f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PKTGEN_BAD_GET_NEXT_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x39fc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT UDPSPDT_TX_PARAMS_TABLE_1 =
{
	24,
	{
		{ dump_RDD_UDPSPDT_TX_PARAMS, 0x3a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SPDSVC_GEN_DISPATCHER_CREDIT_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3a60 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PKTGEN_MAX_UT_PKTS_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3a6c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_DISPATCHER_CREDIT_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3a70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PKTGEN_UT_TRIGGER_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3a7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_VPORT_TO_METER_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3a80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_1 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, 0x3aa8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT COMMON_REPROCESSING_DISPATCHER_CREDIT_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3ab0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT TASK_IDX_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3abc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_UPDATE_FIFO_TABLE_1 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x3ac0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT TCPSPDTEST_GEN_DISPATCHER_CREDIT_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3ae0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT ENABLE_VPORT_MASK_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3aec },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT IPTV_CONFIGURATION_TABLE_1 =
{
	12,
	{
		{ dump_RDD_HW_IPTV_CONFIGURATION, 0x3af0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_ACTION_VEC_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3afc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3afe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT NATC_TBL_CFG_1 =
{
	24,
	{
		{ dump_RDD_NATC_TBL_CONFIGURATION, 0x3b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3b60 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_INTERRUPT_THRESHOLD_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3b6c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_INTERRUPT_COUNTER_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3b6e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_DESCRIPTOR_TABLE_1 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x3b70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PD_FIFO_TABLE_1 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x3b80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE_1 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0x3ba0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_INTERRUPT_COALESCING_TABLE_1 =
{
	16,
	{
		{ dump_RDD_CPU_INTERRUPT_COALESCING_ENTRY, 0x3bb0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT COMMON_REPROCESSING_PD_FIFO_TABLE_1 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x3bc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3be0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_DESCRIPTOR_TABLE_1 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x3c10 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_INTERRUPT_SCRATCH_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3c20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_RX_LOCAL_SCRATCH_1 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x3c28 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_1 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, 0x3c38 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT VPORT_TO_CPU_OBJ_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3c40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PKTGEN_SBPM_EXTS_1 =
{
	4,
	{
		{ dump_RDD_PKTGEN_SBPM_EXT, 0x3c68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_1 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x3c78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PKTGEN_FPM_UG_MGMT_1 =
{
	20,
	{
		{ dump_RDD_PKTGEN_FPM_UG_MGMT_ENTRY, 0x3c80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3c94 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3c95 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3c96 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SPDSVC_TCPSPDTEST_COMMON_TABLE_1 =
{
	1,
	{
		{ dump_RDD_SPDSVC_TCPSPDTEST_COMMON_ENTRY, 0x3c97 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_1 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, 0x3c98 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_CACHE_OFFSET_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3c9f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PKTGEN_BBMSG_REPLY_SCRATCH_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3ca0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_SHADOW_RD_IDX_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3ca8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_1 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x3caa },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_REDIRECT_MODE_1 =
{
	1,
	{
		{ dump_RDD_CPU_REDIRECT_MODE_ENTRY, 0x3cac },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CSO_DISABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3cad },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3cae },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3caf },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_1 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, 0x3cb0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3cc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_1 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, 0x3cd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_1 =
{
	16,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, 0x3ce0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT MULTICAST_WHITELIST_CFG_TABLE_1 =
{
	12,
	{
		{ dump_RDD_MULTICAST_WHITELIST_CFG, 0x3cf0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT RATE_LIMIT_OVERHEAD_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3cfc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_1 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x3cfd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3cfe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3cff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT FC_FLOW_IP_ADDRESSES_TABLE_1 =
{
	48,
	{
		{ dump_RDD_FC_FLOW_IP_ADDRESSES_ENTRY, 0x3d00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_1 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, 0x3dc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_SHADOW_WR_IDX_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3dc8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_NEXT_PTR_TABLE_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3dd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_1 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3dd8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_1 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, 0x3de0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_1 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3de8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_1 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, 0x3df0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_1 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3df8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PKTGEN_TX_STREAM_SCRATCH_TABLE_1 =
{
	8,
	{
		{ dump_RDD_PKTGEN_TX_STREAM_SCRATCH_ENTRY, 0x3e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_RATE_LIMITER_TABLE_2 =
{
	16,
	{
		{ dump_RDD_BASIC_RATE_LIMITER_DESCRIPTOR, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x420 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_2 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, 0x440 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_TX_DBG_CNTRS_TABLE_2 =
{
	64,
	{
		{ dump_RDD_CPU_TX_DBG_CNTRS, 0x480 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE_2 =
{
	8,
	{
		{ dump_RDD_SCHEDULING_QUEUE_DESCRIPTOR, 0x500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CODEL_SQ_TABLE_2 =
{
	2,
	{
		{ dump_RDD_CODEL_SQ_ENTRY, 0x740 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_TX_0_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_TX_SCRATCHPAD_2 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_2 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_2 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0xea8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_INTERRUPT_COALESCING_TABLE_2 =
{
	16,
	{
		{ dump_RDD_CPU_INTERRUPT_COALESCING_ENTRY, 0xeb0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_COMPLEX_SCHEDULER_TABLE_2 =
{
	64,
	{
		{ dump_RDD_COMPLEX_SCHEDULER_DESCRIPTOR, 0xec0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_2 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, 0xf00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_INT_INTERRUPT_SCRATCH_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0xfd4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_RX_INTERRUPT_ID_DDR_ADDR_2 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0xfd8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0xfe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER_2 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_PD_FIFO_TABLE_2 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_2 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CODEL_BIAS_SLOPE_TABLE_2 =
{
	4,
	{
		{ dump_RDD_CODEL_BIAS_SLOPE, 0x2740 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DDR_LATENCY_DBG_USEC_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x276c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_2 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, 0x2770 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_TX_1_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT QUEUE_THRESHOLD_VECTOR_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2940 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DDR_LATENCY_DBG_USEC_MAX_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2964 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_2 =
{
	8,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x2968 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2970 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_2 =
{
	8,
	{
		{ dump_RDD_GENERAL_TIMER_ENTRY, 0x2980 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT VLAN_ACTION_GPE_HANDLER_PTR_TABLE_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2b40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CODEL_NUM_QUEUES_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2b62 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2b64 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_2 =
{
	5,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x2b68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_UDP_RX_FLOWS_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2b6d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_ACTION_VEC_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2b6e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_2 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0x2b70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_2 =
{
	1,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, 0x2b80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT BUFFER_ALLOC_REPLY_2 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x2bd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT GENERAL_TIMER_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2be0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2d40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2dc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_SCHEDULING_QUEUE_AGING_VECTOR_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2de0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT TASK_IDX_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2df4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE_2 =
{
	8,
	{
		{ dump_RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY, 0x2df8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_CODEL_QUEUE_TABLE_2 =
{
	4,
	{
		{ dump_RDD_CODEL_QUEUE_DESCRIPTOR, 0x2f40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_UPDATE_FIFO_TABLE_2 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x2fc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CODEL_ENABLE_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2fe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT ENABLE_VPORT_MASK_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2ff4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_TX_RING_INDICES_VALUES_TABLE_2 =
{
	4,
	{
		{ dump_RDD_CPU_TX_RING_INDICES, 0x2ff8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3140 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_FLUSH_CFG_FW_TABLE_2 =
{
	16,
	{
		{ dump_RDD_FLUSH_CFG_ENTRY, 0x3170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT RING_CPU_TX_DESCRIPTOR_DATA_TABLE_2 =
{
	16,
	{
		{ dump_RDD_RING_CPU_TX_DESCRIPTOR, 0x3180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_TX_RING_DESCRIPTOR_TABLE_2 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x31a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x31c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_FLUSH_CFG_CURRENT_TABLE_2 =
{
	16,
	{
		{ dump_RDD_FLUSH_CFG_ENTRY, 0x31d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_RATE_LIMITER_VALID_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x31e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_CODEL_DROP_DESCRIPTOR_2 =
{
	16,
	{
		{ dump_RDD_CODEL_DROP_DESCRIPTOR, 0x31f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT BRIDGE_CFG_TABLE_2 =
{
	8,
	{
		{ dump_RDD_BRIDGE_CFG, 0x3340 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3350 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3360 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_FLUSH_CFG_ENABLE_TABLE_2 =
{
	2,
	{
		{ dump_RDD_FLUSH_CFG_ENABLE_ENTRY, 0x336c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x336e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_TX_INGRESS_DISPATCHER_CREDIT_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3370 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_BUDGET_ALLOCATION_TIMER_VALUE_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x337c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x337e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x337f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_TX_SYNC_FIFO_TABLE_2 =
{
	8,
	{
		{ dump_RDD_CPU_TX_SYNC_FIFO_ENTRY, 0x3380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT IPTV_CONFIGURATION_TABLE_2 =
{
	12,
	{
		{ dump_RDD_HW_IPTV_CONFIGURATION, 0x3390 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x339c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_REDIRECT_MODE_2 =
{
	1,
	{
		{ dump_RDD_CPU_REDIRECT_MODE_ENTRY, 0x339d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_2 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x339e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x33a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x33ac },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x33ad },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT RATE_LIMIT_OVERHEAD_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x33ae },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_FIRST_QUEUE_MAPPING_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x33af },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SERVICE_QUEUES_DISPATCHER_CREDIT_TABLE_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x33b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_2 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x33bc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x33bd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x33be },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_DESCRIPTOR_TABLE_2 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x33c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE_2 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0x33d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_DESCRIPTOR_TABLE_2 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x33e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_INTERRUPT_SCRATCH_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x33f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_2 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, 0x33f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_SRAM_PD_FIFO_2 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x3400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SQ_TX_QUEUE_DROP_TABLE_2 =
{
	8,
	{
		{ dump_RDD_PACKETS_AND_BYTES, 0x3500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_2 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, 0x3600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_2 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, 0x3680 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_2 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, 0x3690 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_2 =
{
	16,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, 0x36a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT MULTICAST_WHITELIST_CFG_TABLE_2 =
{
	12,
	{
		{ dump_RDD_MULTICAST_WHITELIST_CFG, 0x36b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_2 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, 0x36c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_2 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, 0x36c8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_2 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x36d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_2 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, 0x36d8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_SHADOW_RD_IDX_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x36e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_SHADOW_WR_IDX_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x36e8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_NEXT_PTR_TABLE_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x36f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_2 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x36f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT NATC_TBL_CFG_2 =
{
	24,
	{
		{ dump_RDD_NATC_TBL_CONFIGURATION, 0x3700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_2 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, 0x3760 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_2 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3768 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_2 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, 0x3770 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_2 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3778 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT FC_FLOW_IP_ADDRESSES_TABLE_2 =
{
	48,
	{
		{ dump_RDD_FC_FLOW_IP_ADDRESSES_ENTRY, 0x3d00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_FLOW_RING_CACHE_CTX_TABLE_3 =
{
	16,
	{
		{ dump_RDD_DHD_FLOW_RING_CACHE_CTX_ENTRY, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_3 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, 0x300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3d4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_3 =
{
	8,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x3d8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_AUX_INFO_CACHE_TABLE_3 =
{
	16,
	{
		{ dump_RDD_DHD_AUX_INFO_CACHE_ENTRY, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_FLOW_RING_BUFFER_3 =
{
	48,
	{
		{ dump_RDD_DHD_TX_POST_DESCRIPTOR, 0x700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_3 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, 0x790 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x7a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_3 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, 0x7c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_3 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT MIRRORING_SCRATCH_3 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0xaa8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xab0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_CODEL_BIAS_SLOPE_TABLE_3 =
{
	4,
	{
		{ dump_RDD_DHD_CODEL_BIAS_SLOPE, 0xac0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CODEL_NUM_QUEUES_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0xaec },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0xaee },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_3 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0xaf0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_0_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xb00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_1_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xb80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_3 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, 0xc00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_POST_COMMON_RADIO_DATA_3 =
{
	152,
	{
		{ dump_RDD_DHD_POST_COMMON_RADIO_ENTRY, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_3 =
{
	8,
	{
		{ dump_RDD_DDR_ADDRESS, 0xfc8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_FPM_POOL_NUMBER_MAPPING_TABLE_3 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0xfd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_TIMER_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xfe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PROCESSING_8_TASKS_PACKET_BUFFER_3 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT VLAN_ACTION_GPE_HANDLER_PTR_TABLE_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2140 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SPDSVC_WLAN_TXPOST_PARAMS_TABLE_3 =
{
	2,
	{
		{ dump_RDD_SPDSVC_WLAN_TXPOST_PARAMS, 0x2162 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT TASK_IDX_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2164 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_BACKUP_BASE_ADDR_3 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x2168 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_HW_CFG_3 =
{
	16,
	{
		{ dump_RDD_DHD_HW_CONFIGURATION, 0x2170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_2_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_UPDATE_FIFO_TABLE_3 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x2340 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_UPDATE_FIFO_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2360 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_BACKUP_INDEX_CACHE_3 =
{
	32,
	{
		{ dump_RDD_DHD_BACKUP_IDX_CACHE_TABLE, 0x2380 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CODEL_ENABLE_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x23e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT ENABLE_VPORT_MASK_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x23f4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_3 =
{
	5,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x23f8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_UDP_RX_FLOWS_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x23fd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x23fe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x23ff },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_DOORBELL_TX_POST_VALUE_3 =
{
	4,
	{
		{ dump_RDD_DHD_DOORBELL, 0x2540 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_3 =
{
	1,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, 0x2580 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x25d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2740 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT BRIDGE_CFG_TABLE_3 =
{
	8,
	{
		{ dump_RDD_BRIDGE_CFG, 0x2750 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2760 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT IPTV_CONFIGURATION_TABLE_3 =
{
	12,
	{
		{ dump_RDD_HW_IPTV_CONFIGURATION, 0x2770 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x277c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_REDIRECT_MODE_3 =
{
	1,
	{
		{ dump_RDD_CPU_REDIRECT_MODE_ENTRY, 0x277d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_3 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x277e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_L2_HEADER_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_3 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, 0x27c8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x27d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x27dc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x27dd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT RATE_LIMIT_OVERHEAD_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x27de },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_3 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x27df },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_CPU_INT_ID_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x27e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x27ec },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x27ed },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_MIRRORING_DISPATCHER_CREDIT_TABLE_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x27f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PROCESSING4_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2940 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_DESCRIPTOR_TABLE_3 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x29c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_3 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, 0x29d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_3 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, 0x29e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_3 =
{
	16,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, 0x29f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PROCESSING5_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT MULTICAST_WHITELIST_CFG_TABLE_3 =
{
	12,
	{
		{ dump_RDD_MULTICAST_WHITELIST_CFG, 0x2b40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_3 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, 0x2b50 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_3 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x2b58 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_3 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, 0x2b60 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_3 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, 0x2b68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_3 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2b70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_3 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, 0x2b78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE_3 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x2b80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_3 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2bb0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_3 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, 0x2bb8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2bc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PROCESSING6_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PROCESSING7_STACK_3 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_TX_POST_PD_FIFO_TABLE_3 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x3000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_3 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, 0x3100 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT NATC_TBL_CFG_3 =
{
	24,
	{
		{ dump_RDD_NATC_TBL_CONFIGURATION, 0x3200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_FLOW_RING_CACHE_LKP_TABLE_3 =
{
	2,
	{
		{ dump_RDD_DHD_FLOW_RING_CACHE_LKP_ENTRY, 0x3300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT FC_FLOW_IP_ADDRESSES_TABLE_3 =
{
	48,
	{
		{ dump_RDD_FC_FLOW_IP_ADDRESSES_ENTRY, 0x3d00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DS_TM_PD_FIFO_TABLE_4 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DS_TM_SCHEDULING_QUEUE_TABLE_4 =
{
	8,
	{
		{ dump_RDD_SCHEDULING_QUEUE_DESCRIPTOR, 0x600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT BASIC_SCHEDULER_TABLE_DS_4 =
{
	16,
	{
		{ dump_RDD_BASIC_SCHEDULER_DESCRIPTOR, 0x780 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x7e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT REPORTING_QUEUE_DESCRIPTOR_TABLE_4 =
{
	8,
	{
		{ dump_RDD_REPORTING_QUEUE_DESCRIPTOR, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_4 =
{
	8,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0xc08 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DS_TM_CODEL_DROP_DESCRIPTOR_4 =
{
	16,
	{
		{ dump_RDD_CODEL_DROP_DESCRIPTOR, 0xc10 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0xc20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DS_TM_TM_FLOW_CNTR_TABLE_4 =
{
	1,
	{
		{ dump_RDD_TM_FLOW_CNTR_ENTRY, 0xc40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT POLICER_PARAMS_TABLE_4 =
{
	1,
	{
		{ dump_RDD_POLICER_PARAMS_ENTRY, 0xc80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SPDTEST_GEN_PARAM_4 =
{
	4,
	{
		{ dump_RDD_SPDTEST_GEN_CFG, 0xcd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT REPORTING_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xce0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT REPORTING_QUEUE_ACCUMULATED_TABLE_4 =
{
	16,
	{
		{ dump_RDD_QM_QUEUE_COUNTER_DATA, 0xd00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PROCESSING0_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0xe00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0xf40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_4 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, 0xfc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PROCESSING_4_TASKS_PACKET_BUFFER_4 =
{
	512,
	{
		{ dump_RDD_PACKET_BUFFER, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT COMPLEX_SCHEDULER_TABLE_4 =
{
	64,
	{
		{ dump_RDD_COMPLEX_SCHEDULER_DESCRIPTOR, 0x1800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE_4 =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, 0x1c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DS_TM_BBH_TX_WAKE_UP_DATA_TABLE_4 =
{
	8,
	{
		{ dump_RDD_BBH_TX_WAKE_UP_DATA_ENTRY, 0x1ea8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT LOOPBACK_QUEUE_TABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1eb0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DS_BUFFER_CONG_MGT_CFG_4 =
{
	64,
	{
		{ dump_RDD_BUFFER_CONG_MGT, 0x1ec0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT TX_FLOW_TABLE_4 =
{
	1,
	{
		{ dump_RDD_TX_FLOW_ENTRY, 0x1f00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1fd4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT MIRRORING_SCRATCH_4 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x1fd8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT BUFFER_CONG_DQM_NOT_EMPTY_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1fe0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT BASIC_RATE_LIMITER_TABLE_DS_4 =
{
	16,
	{
		{ dump_RDD_BASIC_RATE_LIMITER_DESCRIPTOR, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT VLAN_TX_COUNTERS_4 =
{
	8,
	{
		{ dump_RDD_PACKETS_AND_BYTES, 0x2300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DS_TM_CPU_TX_ABS_COUNTERS_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2708 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2788 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT REPORT_BBH_TX_QUEUE_ID_TABLE_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x27b8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT BUFFER_CONG_SCRATCHPAD_4 =
{
	8,
	{
		{ dump_RDD_BUFFER_CONG_Q_OCCUPANCY, 0x27c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_4 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PROCESSING1_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2a00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CODEL_BIAS_SLOPE_TABLE_4 =
{
	4,
	{
		{ dump_RDD_CODEL_BIAS_SLOPE, 0x2b40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT GHOST_REPORTING_GLOBAL_CFG_4 =
{
	4,
	{
		{ dump_RDD_GHOST_REPORTING_GLOBAL_CFG_ENTRY, 0x2b6c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_4 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0x2b70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT VLAN_ACTION_GPE_HANDLER_PTR_TABLE_4 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2b80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CODEL_NUM_QUEUES_4 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2ba2 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT TASK_IDX_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2ba4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SYSTEM_CONFIGURATION_4 =
{
	5,
	{
		{ dump_RDD_SYSTEM_CONFIGURATION_ENTRY, 0x2ba8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SPDTEST_NUM_OF_UDP_RX_FLOWS_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2bad },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT BUDGET_ALLOCATION_TIMER_VALUE_4 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2bae },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DS_TM_FLUSH_CFG_CPU_TABLE_4 =
{
	16,
	{
		{ dump_RDD_FLUSH_CFG_ENTRY, 0x2bb0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT UPDATE_FIFO_TABLE_4 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x2bc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DS_TM_SCHEDULING_QUEUE_AGING_VECTOR_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2be0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT ENABLE_VPORT_MASK_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2bf4 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT TUNNELS_PARSING_CFG_4 =
{
	8,
	{
		{ dump_RDD_TUNNELS_PARSING_CFG, 0x2bf8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PROCESSING2_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CODEL_ENABLE_TABLE_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2d40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DS_TM_FLUSH_CFG_ENABLE_TABLE_4 =
{
	2,
	{
		{ dump_RDD_FLUSH_CFG_ENABLE_ENTRY, 0x2d54 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT MCAST_MAX_REPLICATION_FLAG_TABLE_4 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2d56 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT IPTV_CFG_TABLE_4 =
{
	8,
	{
		{ dump_RDD_IPTV_CFG, 0x2d58 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT GHOST_REPORTING_QUEUE_STATUS_BIT_VECTOR_TABLE_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2d60 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DS_TM_BB_DESTINATION_TABLE_4 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2d74 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT LOOPBACK_WAN_FLOW_TABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2d76 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT FORCE_DSCP_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2d77 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT IPTV_DDR_CTX_TABLE_ADDRESS_4 =
{
	8,
	{
		{ dump_RDD_IPTV_DDR_CTX_TABLE_ADDRESS, 0x2d78 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT QUEUE_TO_REPORT_BIT_VECTOR_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2d80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2d94 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SCHEDULING_FLUSH_GLOBAL_CFG_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2d95 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT TX_EXCEPTION_4 =
{
	1,
	{
		{ dump_RDD_TX_EXCEPTION_ENTRY, 0x2d96 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_REDIRECT_MODE_4 =
{
	1,
	{
		{ dump_RDD_CPU_REDIRECT_MODE_ENTRY, 0x2d97 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT BBH_TX_EGRESS_REPORT_COUNTER_TABLE_4 =
{
	8,
	{
		{ dump_RDD_BBH_TX_EGRESS_COUNTER_ENTRY, 0x2d98 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DS_TM_FLUSH_CFG_FW_TABLE_4 =
{
	16,
	{
		{ dump_RDD_FLUSH_CFG_ENTRY, 0x2da0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DS_TM_FLUSH_CFG_CURRENT_TABLE_4 =
{
	16,
	{
		{ dump_RDD_FLUSH_CFG_ENTRY, 0x2db0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT BITS_CALC_MASKS_TABLE_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2dc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT BRIDGE_CFG_TABLE_4 =
{
	8,
	{
		{ dump_RDD_BRIDGE_CFG, 0x2dd0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_L2_REASON_TABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2de0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DS_TM_FLUSH_DISPATCHER_CREDIT_TABLE_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2df0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_4 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x2dfc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_4 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x2dfe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT PROCESSING3_STACK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DS_TM_TX_QUEUE_DROP_TABLE_4 =
{
	8,
	{
		{ dump_RDD_PACKETS_AND_BYTES, 0x2f40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT NAT_CACHE_CFG_4 =
{
	7,
	{
		{ dump_RDD_NAT_CACHE_CFG, 0x3148 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x314f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT IPTV_CONFIGURATION_TABLE_4 =
{
	12,
	{
		{ dump_RDD_HW_IPTV_CONFIGURATION, 0x3150 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x315c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT RATE_LIMIT_OVERHEAD_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x315d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DS_TM_FIRST_QUEUE_MAPPING_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x315e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT BACKUP_BBH_INGRESS_COUNTERS_TABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x315f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT RATE_LIMITER_VALID_TABLE_DS_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3160 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DHD_FPM_THRESHOLDS_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3170 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT BACKUP_BBH_EGRESS_COUNTERS_TABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x317c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT IPTV_CLASSIFICATION_CFG_TABLE_4 =
{
	1,
	{
		{ dump_RDD_IPTV_CLASSIFICATION_CFG_ENTRY, 0x317d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_1588_CFG_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x317e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x317f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_DISPATCHER_CREDIT_TABLE_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3180 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT BBH_TX_INGRESS_COUNTER_TABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x318c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_DESCRIPTOR_TABLE_4 =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x3190 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_4 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, 0x31a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT TCAM_GENERIC_FIELDS_4 =
{
	2,
	{
		{ dump_RDD_TCAM_GENERIC, 0x31b0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x31c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT NAT_CACHE_KEY0_MASK_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x31c8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_4 =
{
	16,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, 0x31d0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT MULTICAST_WHITELIST_CFG_TABLE_4 =
{
	12,
	{
		{ dump_RDD_MULTICAST_WHITELIST_CFG, 0x31e0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT XGPON_REPORT_ZERO_SENT_TABLE_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x31f0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT REPORTING_QUEUE_COUNTER_TABLE_4 =
{
	2,
	{
		{ dump_RDD_REPORTING_QUEUE_COUNTER, 0x3200 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT TCAM_TABLE_CFG_TABLE_4 =
{
	1,
	{
		{ dump_RDD_TCAM_TABLE_CFG, 0x3308 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT NATC_L2_VLAN_KEY_MASK_4 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x3310 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_CFG_4 =
{
	2,
	{
		{ dump_RDD_INGRESS_FILTER_CFG, 0x3318 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT MULTICAST_KEY_MASK_4 =
{
	4,
	{
		{ dump_RDD_MULTICAST_KEY_MASK_ENTRY, 0x3320 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT NATC_L2_TOS_MASK_4 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x3328 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DS_TM_CODEL_QUEUE_TABLE_4 =
{
	4,
	{
		{ dump_RDD_CODEL_QUEUE_DESCRIPTOR, 0x3400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT REPORTING_COUNTER_TABLE_4 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x3500 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT INGRESS_FILTER_PROFILE_TABLE_4 =
{
	8,
	{
		{ dump_RDD_INGRESS_FILTER_CTRL, 0x3600 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT NATC_TBL_CFG_4 =
{
	24,
	{
		{ dump_RDD_NATC_TBL_CONFIGURATION, 0x3700 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DS_TM_BBH_QUEUE_TABLE_4 =
{
	4,
	{
		{ dump_RDD_BBH_QUEUE_DESCRIPTOR, 0x3800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT FC_FLOW_IP_ADDRESSES_TABLE_4 =
{
	48,
	{
		{ dump_RDD_FC_FLOW_IP_ADDRESSES_ENTRY, 0x3d00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT US_TM_PD_FIFO_TABLE_5 =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT US_TM_TX_QUEUE_DROP_TABLE_5 =
{
	8,
	{
		{ dump_RDD_PACKETS_AND_BYTES, 0x1080 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT GENERAL_QUEUE_DYNAMIC_MNG_TABLE_5 =
{
	8,
	{
		{ dump_RDD_QUEUE_DYNAMIC_MNG_ENTRY, 0x1288 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT US_TM_CODEL_DROP_DESCRIPTOR_5 =
{
	16,
	{
		{ dump_RDD_CODEL_DROP_DESCRIPTOR, 0x1290 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x12a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT VPORT_CFG_TABLE_5 =
{
	4,
	{
		{ dump_RDD_VPORT_CFG_ENTRY, 0x12c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DIRECT_FLOW_RX_MIRRORING_SCRATCHPAD_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1300 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT US_TM_WAN_0_BBH_TX_WAKE_UP_DATA_TABLE_5 =
{
	8,
	{
		{ dump_RDD_BBH_TX_WAKE_UP_DATA_ENTRY, 0x1388 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT FPM_POOL_NUMBER_MAPPING_TABLE_5 =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0x1390 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DIRECT_FLOW_CNTR_TABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x13a0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DIRECT_FLOW_STACK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x13c0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT COMPLEX_SCHEDULER_TABLE_5 =
{
	64,
	{
		{ dump_RDD_COMPLEX_SCHEDULER_DESCRIPTOR, 0x1400 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT US_TM_SCHEDULING_QUEUE_TABLE_5 =
{
	8,
	{
		{ dump_RDD_SCHEDULING_QUEUE_DESCRIPTOR, 0x1800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT BUFFER_CONG_DQM_NOT_EMPTY_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1c20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT US_BUFFER_CONG_MGT_CFG_5 =
{
	64,
	{
		{ dump_RDD_BUFFER_CONG_MGT, 0x1c40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT US_TM_TM_FLOW_CNTR_TABLE_5 =
{
	1,
	{
		{ dump_RDD_TM_FLOW_CNTR_ENTRY, 0x1c80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT NATC_TBL_CFG_5 =
{
	24,
	{
		{ dump_RDD_NATC_TBL_CONFIGURATION, 0x1d00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1d60 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DIRECT_FLOW_EPON_CONTROL_SCRATCH_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1de0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT BUDGET_ALLOCATION_TIMER_VALUE_5 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x1df6 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT BBH_TX_EPON_WAKE_UP_DATA_TABLE_5 =
{
	8,
	{
		{ dump_RDD_BBH_TX_WAKE_UP_DATA_ENTRY, 0x1df8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT US_TM_BBH_QUEUE_TABLE_5 =
{
	4,
	{
		{ dump_RDD_BBH_QUEUE_DESCRIPTOR, 0x1e00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT US_TM_CPU_TX_ABS_COUNTERS_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1ea0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT US_TM_SCHEDULING_QUEUE_AGING_VECTOR_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1f20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT MUTEX_TABLE_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1f34 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT MIRRORING_SCRATCH_5 =
{
	8,
	{
		{ dump_RDD_BYTES_8, 0x1f38 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT BUFFER_CONG_SCRATCHPAD_5 =
{
	8,
	{
		{ dump_RDD_BUFFER_CONG_Q_OCCUPANCY, 0x1f40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CODEL_BIAS_SLOPE_TABLE_5 =
{
	4,
	{
		{ dump_RDD_CODEL_BIAS_SLOPE, 0x1f80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT TASK_IDX_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1fac },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT US_TM_FLUSH_CFG_CPU_TABLE_5 =
{
	16,
	{
		{ dump_RDD_FLUSH_CFG_ENTRY, 0x1fb0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT US_TM_WAN_0_BBH_TX_EGRESS_COUNTER_TABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x1fc0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT US_TM_FLUSH_CFG_ENABLE_TABLE_5 =
{
	2,
	{
		{ dump_RDD_FLUSH_CFG_ENABLE_ENTRY, 0x1fe8 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DIRECT_FLOW_PAUSE_QUANTA_5 =
{
	2,
	{
		{ dump_RDD_PAUSE_QUANTA_ENTRY, 0x1fea },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT US_TM_BBH_TX_WAN_0_FIFO_BYTES_USED_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1fec },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT US_TM_FLUSH_CFG_FW_TABLE_5 =
{
	16,
	{
		{ dump_RDD_FLUSH_CFG_ENTRY, 0x1ff0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT BASIC_RATE_LIMITER_TABLE_US_5 =
{
	16,
	{
		{ dump_RDD_BASIC_RATE_LIMITER_DESCRIPTOR, 0x2000 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT US_TM_CODEL_QUEUE_TABLE_5 =
{
	4,
	{
		{ dump_RDD_CODEL_QUEUE_DESCRIPTOR, 0x2800 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DEBUG_SCRATCHPAD_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2a10 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT BBH_TX_EPON_INGRESS_COUNTER_TABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2a40 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT US_TM_WAN_0_BB_DESTINATION_TABLE_5 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2a68 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT QEMU_SYNC_MEM_5 =
{
	1,
	{
		{ dump_RDD_QEMU_DATA, 0x2a6a },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DIRECT_FLOW_WAN_VIQ_EXCLUSIVE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2a6b },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT CORE_ID_TABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2a6c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT US_TM_TX_PAUSE_NACK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2a6d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT BBH_TX_US_WAN_0_FIFO_BYTES_THRESHOLD_5 =
{
	2,
	{
		{ dump_RDD_BYTES_2, 0x2a6e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT US_TM_FLUSH_CFG_CURRENT_TABLE_5 =
{
	16,
	{
		{ dump_RDD_FLUSH_CFG_ENTRY, 0x2a70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT BBH_TX_EPON_EGRESS_COUNTER_TABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2a80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DIRECT_FLOW_PD_TABLE_5 =
{
	16,
	{
		{ dump_RDD_PROCESSING_RX_DESCRIPTOR, 0x2aa0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT OVERALL_RATE_LIMITER_TABLE_5 =
{
	16,
	{
		{ dump_RDD_OVERALL_RATE_LIMITER_DESCRIPTOR, 0x2ab0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT UPDATE_FIFO_TABLE_5 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x2ac0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT RATE_LIMITER_VALID_TABLE_US_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2ae0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT US_TM_FLUSH_DISPATCHER_CREDIT_TABLE_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2af0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SCHEDULING_FLUSH_GLOBAL_CFG_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2afc },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT TX_EXCEPTION_5 =
{
	1,
	{
		{ dump_RDD_TX_EXCEPTION_ENTRY, 0x2afd },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_CONFIGURATION_5 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x2afe },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT EPON_UPDATE_FIFO_TABLE_5 =
{
	4,
	{
		{ dump_RDD_UPDATE_FIFO_ENTRY, 0x2b00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT US_TM_TX_OCTETS_COUNTERS_TABLE_5 =
{
	8,
	{
		{ dump_RDD_PACKETS_AND_BYTES, 0x2b20 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DISPATCHER_CREDIT_TABLE_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2b60 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_CONFIGURATION_5 =
{
	2,
	{
		{ dump_RDD_MIRRORING_DESCRIPTOR, 0x2b6c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_CORE_LOCK_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2b6e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2b6f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DIRECT_FLOW_PAUSE_DEBUG_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2b70 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT RATE_LIMIT_OVERHEAD_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2b7c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT MAC_TYPE_5 =
{
	1,
	{
		{ dump_RDD_MAC_TYPE_ENTRY, 0x2b7d },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT US_TM_FIRST_QUEUE_MAPPING_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2b7e },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT WAN_0_BBH_TX_FIFO_SIZE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2b7f },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT TX_MIRRORING_DISPATCHER_CREDIT_TABLE_5 =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x2b80 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT RX_MIRRORING_DIRECT_ENABLE_5 =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x2b8c },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT DEBUG_PRINT_TABLE_5 =
{
	16,
	{
		{ dump_RDD_DEBUG_PRINT_INFO, 0x2b90 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG_5 =
{
	16,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, 0x2ba0 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT BASIC_SCHEDULER_TABLE_US_5 =
{
	16,
	{
		{ dump_RDD_BASIC_SCHEDULER_DESCRIPTOR, 0x2c00 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT VLAN_TX_COUNTERS_5 =
{
	8,
	{
		{ dump_RDD_PACKETS_AND_BYTES, 0x2e10 },
		{ 0, 0 },
	}
};
#endif
#if defined BCM6855
static DUMP_RUNNERREG_STRUCT RUNNER_PROFILING_TRACE_BUFFER_5 =
{
	4,
	{
		{ dump_RDD_TRACE_EVENT, 0x3400 },
		{ 0, 0 },
	}
};
#endif

TABLE_STRUCT RUNNER_TABLES[NUMBER_OF_TABLES] =
{
#if defined BCM6855
	{ "RX_FLOW_TABLE", 1, CORE_0_INDEX, &RX_FLOW_TABLE, 340, 1, 1 },
#endif
#if defined BCM6855
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_0_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "SPDTEST_GEN_PARAM", 1, CORE_0_INDEX, &SPDTEST_GEN_PARAM, 4, 1, 1 },
#endif
#if defined BCM6855
	{ "VPORT_CFG_TABLE", 1, CORE_0_INDEX, &VPORT_CFG_TABLE, 16, 1, 1 },
#endif
#if defined BCM6855
	{ "SPDSVC_ANALYZER_STACK", 1, CORE_0_INDEX, &SPDSVC_ANALYZER_STACK, 256, 1, 1 },
#endif
#if defined BCM6855
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_0_INDEX, &RUNNER_PROFILING_TRACE_BUFFER, 128, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_COMPLETE_COMMON_RADIO_DATA", 1, CORE_0_INDEX, &DHD_COMPLETE_COMMON_RADIO_DATA, 3, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR", 1, CORE_0_INDEX, &DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_0_INDEX, &LOOPBACK_QUEUE_TABLE, 16, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_TX_COMPLETE_0_STACK", 1, CORE_0_INDEX, &DHD_TX_COMPLETE_0_STACK, 128, 1, 1 },
#endif
#if defined BCM6855
	{ "PROCESSING0_STACK", 1, CORE_0_INDEX, &PROCESSING0_STACK, 320, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_RX_COMPLETE_0_STACK", 1, CORE_0_INDEX, &DHD_RX_COMPLETE_0_STACK, 64, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_TX_COMPLETE_1_STACK", 1, CORE_0_INDEX, &DHD_TX_COMPLETE_1_STACK, 128, 1, 1 },
#endif
#if defined BCM6855
	{ "PROCESSING1_STACK", 1, CORE_0_INDEX, &PROCESSING1_STACK, 320, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_RX_COMPLETE_1_STACK", 1, CORE_0_INDEX, &DHD_RX_COMPLETE_1_STACK, 64, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_TX_COMPLETE_2_STACK", 1, CORE_0_INDEX, &DHD_TX_COMPLETE_2_STACK, 128, 1, 1 },
#endif
#if defined BCM6855
	{ "PROCESSING2_STACK", 1, CORE_0_INDEX, &PROCESSING2_STACK, 320, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_RX_COMPLETE_2_STACK", 1, CORE_0_INDEX, &DHD_RX_COMPLETE_2_STACK, 64, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_RX_COMPLETE_FLOW_RING_BUFFER", 1, CORE_0_INDEX, &DHD_RX_COMPLETE_FLOW_RING_BUFFER, 3, 1, 1 },
#endif
#if defined BCM6855
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_0_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE, 32, 1, 1 },
#endif
#if defined BCM6855
	{ "PROCESSING3_STACK", 1, CORE_0_INDEX, &PROCESSING3_STACK, 320, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_TX_COMPLETE_FLOW_RING_BUFFER", 1, CORE_0_INDEX, &DHD_TX_COMPLETE_FLOW_RING_BUFFER, 3, 1, 1 },
#endif
#if defined BCM6855
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_0_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE, 16, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_RX_POST_FLOW_RING_BUFFER", 1, CORE_0_INDEX, &DHD_RX_POST_FLOW_RING_BUFFER, 3, 1, 1 },
#endif
#if defined BCM6855
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_0_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT, 8, 1, 1 },
#endif
#if defined BCM6855
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_0_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER, 8, 1, 1 },
#endif
#if defined BCM6855
	{ "PROCESSING4_STACK", 1, CORE_0_INDEX, &PROCESSING4_STACK, 320, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_RX_COMPLETE_DISPATCHER_CREDIT_TABLE", 1, CORE_0_INDEX, &DHD_RX_COMPLETE_DISPATCHER_CREDIT_TABLE, 12, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_HW_CFG", 1, CORE_0_INDEX, &DHD_HW_CFG, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "POLICER_PARAMS_TABLE", 1, CORE_0_INDEX, &POLICER_PARAMS_TABLE, 80, 1, 1 },
#endif
#if defined BCM6855
	{ "DEBUG_SCRATCHPAD", 1, CORE_0_INDEX, &DEBUG_SCRATCHPAD, 12, 1, 1 },
#endif
#if defined BCM6855
	{ "PROCESSING5_STACK", 1, CORE_0_INDEX, &PROCESSING5_STACK, 320, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_TX_COMPLETE_DISPATCHER_CREDIT_TABLE", 1, CORE_0_INDEX, &DHD_TX_COMPLETE_DISPATCHER_CREDIT_TABLE, 12, 1, 1 },
#endif
#if defined BCM6855
	{ "SPDSVC_ANALYZER_PARAMS_TABLE", 1, CORE_0_INDEX, &SPDSVC_ANALYZER_PARAMS_TABLE, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "SPDSVC_ANALYZER_PD_FIFO_TABLE", 1, CORE_0_INDEX, &SPDSVC_ANALYZER_PD_FIFO_TABLE, 4, 1, 1 },
#endif
#if defined BCM6855
	{ "VLAN_ACTION_GPE_HANDLER_PTR_TABLE", 1, CORE_0_INDEX, &VLAN_ACTION_GPE_HANDLER_PTR_TABLE, 17, 1, 1 },
#endif
#if defined BCM6855
	{ "CODEL_NUM_QUEUES", 1, CORE_0_INDEX, &CODEL_NUM_QUEUES, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "MUTEX_TABLE", 1, CORE_0_INDEX, &MUTEX_TABLE, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "SYSTEM_CONFIGURATION", 1, CORE_0_INDEX, &SYSTEM_CONFIGURATION, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "SPDTEST_NUM_OF_UDP_RX_FLOWS", 1, CORE_0_INDEX, &SPDTEST_NUM_OF_UDP_RX_FLOWS, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_0_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "BRIDGE_CFG_TABLE", 1, CORE_0_INDEX, &BRIDGE_CFG_TABLE, 2, 1, 1 },
#endif
#if defined BCM6855
	{ "PROCESSING6_STACK", 1, CORE_0_INDEX, &PROCESSING6_STACK, 320, 1, 1 },
#endif
#if defined BCM6855
	{ "REGISTERS_BUFFER", 1, CORE_0_INDEX, &REGISTERS_BUFFER, 32, 1, 1 },
#endif
#if defined BCM6855
	{ "SPDSVC_ANALYZER_UPDATE_FIFO_TABLE", 1, CORE_0_INDEX, &SPDSVC_ANALYZER_UPDATE_FIFO_TABLE, 8, 1, 1 },
#endif
#if defined BCM6855
	{ "CODEL_ENABLE_TABLE", 1, CORE_0_INDEX, &CODEL_ENABLE_TABLE, 5, 1, 1 },
#endif
#if defined BCM6855
	{ "TASK_IDX", 1, CORE_0_INDEX, &TASK_IDX, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "TUNNELS_PARSING_CFG", 1, CORE_0_INDEX, &TUNNELS_PARSING_CFG, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "PROCESSING7_STACK", 1, CORE_0_INDEX, &PROCESSING7_STACK, 320, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_FPM_REPLY", 1, CORE_0_INDEX, &DHD_FPM_REPLY, 24, 1, 1 },
#endif
#if defined BCM6855
	{ "IPTV_CFG_TABLE", 1, CORE_0_INDEX, &IPTV_CFG_TABLE, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_0_INDEX, &INGRESS_FILTER_L2_REASON_TABLE, 16, 1, 1 },
#endif
#if defined BCM6855
	{ "SPDSVC_ANALYZER_DISPATCHER_CREDIT_TABLE", 1, CORE_0_INDEX, &SPDSVC_ANALYZER_DISPATCHER_CREDIT_TABLE, 3, 1, 1 },
#endif
#if defined BCM6855
	{ "ENABLE_VPORT_MASK", 1, CORE_0_INDEX, &ENABLE_VPORT_MASK, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE", 1, CORE_0_INDEX, &DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE, 3, 1, 1 },
#endif
#if defined BCM6855
	{ "IPTV_CONFIGURATION_TABLE", 1, CORE_0_INDEX, &IPTV_CONFIGURATION_TABLE, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_0_INDEX, &LOOPBACK_WAN_FLOW_TABLE, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "FORCE_DSCP", 1, CORE_0_INDEX, &FORCE_DSCP, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "CORE_ID_TABLE", 1, CORE_0_INDEX, &CORE_ID_TABLE, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_REDIRECT_MODE", 1, CORE_0_INDEX, &CPU_REDIRECT_MODE, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_0_INDEX, &BITS_CALC_MASKS_TABLE, 4, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_FPM_THRESHOLDS", 1, CORE_0_INDEX, &DHD_FPM_THRESHOLDS, 3, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_RX_POST_RING_SIZE", 1, CORE_0_INDEX, &DHD_RX_POST_RING_SIZE, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_RX_COMPLETE_RING_SIZE", 1, CORE_0_INDEX, &DHD_RX_COMPLETE_RING_SIZE, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_DOORBELL_TX_COMPLETE_VALUE", 1, CORE_0_INDEX, &DHD_DOORBELL_TX_COMPLETE_VALUE, 3, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_TX_COMPLETE_RING_SIZE", 1, CORE_0_INDEX, &DHD_TX_COMPLETE_RING_SIZE, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_0_INDEX, &RX_MIRRORING_CONFIGURATION, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_DOORBELL_RX_COMPLETE_VALUE", 1, CORE_0_INDEX, &DHD_DOORBELL_RX_COMPLETE_VALUE, 3, 1, 1 },
#endif
#if defined BCM6855
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_0_INDEX, &DEBUG_PRINT_CORE_LOCK, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "SRAM_DUMMY_STORE", 1, CORE_0_INDEX, &SRAM_DUMMY_STORE, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "RATE_LIMIT_OVERHEAD", 1, CORE_0_INDEX, &RATE_LIMIT_OVERHEAD, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_0_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "TX_FLOW_TABLE", 1, CORE_0_INDEX, &TX_FLOW_TABLE, 212, 1, 1 },
#endif
#if defined BCM6855
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_0_INDEX, &INGRESS_FILTER_1588_CFG, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_0_INDEX, &RX_MIRRORING_DIRECT_ENABLE, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_0_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_DOORBELL_RX_POST_VALUE", 1, CORE_0_INDEX, &DHD_DOORBELL_RX_POST_VALUE, 3, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_CPU_INT_ID", 1, CORE_0_INDEX, &DHD_CPU_INT_ID, 3, 1, 1 },
#endif
#if defined BCM6855
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_0_INDEX, &INGRESS_FILTER_PROFILE_TABLE, 16, 1, 1 },
#endif
#if defined BCM6855
	{ "SPDSVC_WLAN_GEN_PARAMS_TABLE", 1, CORE_0_INDEX, &SPDSVC_WLAN_GEN_PARAMS_TABLE, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_FEED_RING_DESCRIPTOR_TABLE", 1, CORE_0_INDEX, &CPU_FEED_RING_DESCRIPTOR_TABLE, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "UDPSPDT_SCRATCH_TABLE", 1, CORE_0_INDEX, &UDPSPDT_SCRATCH_TABLE, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "NAT_CACHE_CFG", 1, CORE_0_INDEX, &NAT_CACHE_CFG, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "DEBUG_PRINT_TABLE", 1, CORE_0_INDEX, &DEBUG_PRINT_TABLE, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "TCAM_GENERIC_FIELDS", 1, CORE_0_INDEX, &TCAM_GENERIC_FIELDS, 8, 1, 1 },
#endif
#if defined BCM6855
	{ "FPM_GLOBAL_CFG", 1, CORE_0_INDEX, &FPM_GLOBAL_CFG, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "MULTICAST_WHITELIST_CFG_TABLE", 1, CORE_0_INDEX, &MULTICAST_WHITELIST_CFG_TABLE, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_0_INDEX, &NAT_CACHE_KEY0_MASK, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_0_INDEX, &TCAM_TABLE_CFG_TABLE, 2, 1, 1 },
#endif
#if defined BCM6855
	{ "UDPSPDT_STREAM_RX_STAT_TABLE", 1, CORE_0_INDEX, &UDPSPDT_STREAM_RX_STAT_TABLE, 4, 1, 1 },
#endif
#if defined BCM6855
	{ "MULTICAST_KEY_MASK", 1, CORE_0_INDEX, &MULTICAST_KEY_MASK, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_0_INDEX, &NATC_L2_VLAN_KEY_MASK, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "INGRESS_FILTER_CFG", 1, CORE_0_INDEX, &INGRESS_FILTER_CFG, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "NATC_L2_TOS_MASK", 1, CORE_0_INDEX, &NATC_L2_TOS_MASK, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "NATC_TBL_CFG", 1, CORE_0_INDEX, &NATC_TBL_CFG, 4, 1, 1 },
#endif
#if defined BCM6855
	{ "FC_FLOW_IP_ADDRESSES_TABLE", 1, CORE_0_INDEX, &FC_FLOW_IP_ADDRESSES_TABLE, 4, 1, 1 },
#endif
#if defined BCM6855
	{ "RX_FLOW_TABLE", 1, CORE_1_INDEX, &RX_FLOW_TABLE_1, 340, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR", 1, CORE_1_INDEX, &CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_RECYCLE_INTERRUPT_COALESCING_TABLE", 1, CORE_1_INDEX, &CPU_RECYCLE_INTERRUPT_COALESCING_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "DS_CPU_REASON_TO_METER_TABLE", 1, CORE_1_INDEX, &DS_CPU_REASON_TO_METER_TABLE_1, 64, 1, 1 },
#endif
#if defined BCM6855
	{ "TX_FLOW_TABLE", 1, CORE_1_INDEX, &TX_FLOW_TABLE_1, 212, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_INT_INTERRUPT_SCRATCH", 1, CORE_1_INDEX, &CPU_INT_INTERRUPT_SCRATCH_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_RX_INTERRUPT_ID_DDR_ADDR", 1, CORE_1_INDEX, &CPU_RX_INTERRUPT_ID_DDR_ADDR_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_FEED_RING_RSV_TABLE", 1, CORE_1_INDEX, &CPU_FEED_RING_RSV_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_RX_SCRATCHPAD", 1, CORE_1_INDEX, &CPU_RX_SCRATCHPAD_1, 128, 1, 1 },
#endif
#if defined BCM6855
	{ "TCPSPDTEST_STREAM_TABLE", 1, CORE_1_INDEX, &TCPSPDTEST_STREAM_TABLE_1, 4, 1, 1 },
#endif
#if defined BCM6855
	{ "PROCESSING0_STACK", 1, CORE_1_INDEX, &PROCESSING0_STACK_1, 320, 1, 1 },
#endif
#if defined BCM6855
	{ "US_CPU_REASON_TO_METER_TABLE", 1, CORE_1_INDEX, &US_CPU_REASON_TO_METER_TABLE_1, 64, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_FEED_RING_CACHE_TABLE", 1, CORE_1_INDEX, &CPU_FEED_RING_CACHE_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM6855
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_1_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER_1, 8, 1, 1 },
#endif
#if defined BCM6855
	{ "PKTGEN_TX_STREAM_TABLE", 1, CORE_1_INDEX, &PKTGEN_TX_STREAM_TABLE_1, 4, 1, 1 },
#endif
#if defined BCM6855
	{ "IPV4_HOST_ADDRESS_TABLE", 1, CORE_1_INDEX, &IPV4_HOST_ADDRESS_TABLE_1, 8, 1, 1 },
#endif
#if defined BCM6855
	{ "IPV6_HOST_ADDRESS_CRC_TABLE", 1, CORE_1_INDEX, &IPV6_HOST_ADDRESS_CRC_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM6855
	{ "DS_CPU_RX_METER_TABLE", 1, CORE_1_INDEX, &DS_CPU_RX_METER_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM6855
	{ "CSO_CONTEXT_TABLE", 1, CORE_1_INDEX, &CSO_CONTEXT_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_1_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_RX_STACK", 1, CORE_1_INDEX, &CPU_RX_STACK_1, 32, 1, 1 },
#endif
#if defined BCM6855
	{ "VPORT_CFG_TABLE", 1, CORE_1_INDEX, &VPORT_CFG_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM6855
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_1_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_1, 128, 1, 1 },
#endif
#if defined BCM6855
	{ "PROCESSING1_STACK", 1, CORE_1_INDEX, &PROCESSING1_STACK_1, 320, 1, 1 },
#endif
#if defined BCM6855
	{ "SPDSVC_GEN_PARAMS_TABLE", 1, CORE_1_INDEX, &SPDSVC_GEN_PARAMS_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "PKTGEN_NO_SBPM_HDRS_CNTR", 1, CORE_1_INDEX, &PKTGEN_NO_SBPM_HDRS_CNTR_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "US_CPU_RX_METER_TABLE", 1, CORE_1_INDEX, &US_CPU_RX_METER_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM6855
	{ "PROCESSING2_STACK", 1, CORE_1_INDEX, &PROCESSING2_STACK_1, 320, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_REASON_AND_VPORT_TO_METER_TABLE", 1, CORE_1_INDEX, &CPU_REASON_AND_VPORT_TO_METER_TABLE_1, 48, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_RX_COPY_INT_SCRATCHPAD", 1, CORE_1_INDEX, &CPU_RX_COPY_INT_SCRATCHPAD_1, 4, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_RX_COPY_STACK", 1, CORE_1_INDEX, &CPU_RX_COPY_STACK_1, 128, 1, 1 },
#endif
#if defined BCM6855
	{ "PROCESSING3_STACK", 1, CORE_1_INDEX, &PROCESSING3_STACK_1, 320, 1, 1 },
#endif
#if defined BCM6855
	{ "VLAN_ACTION_GPE_HANDLER_PTR_TABLE", 1, CORE_1_INDEX, &VLAN_ACTION_GPE_HANDLER_PTR_TABLE_1, 17, 1, 1 },
#endif
#if defined BCM6855
	{ "CODEL_NUM_QUEUES", 1, CORE_1_INDEX, &CODEL_NUM_QUEUES_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "MUTEX_TABLE", 1, CORE_1_INDEX, &MUTEX_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "TX_ABS_RECYCLE_COUNTERS", 1, CORE_1_INDEX, &TX_ABS_RECYCLE_COUNTERS_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_RX_PSRAM_GET_NEXT_SCRATCHPAD", 1, CORE_1_INDEX, &CPU_RX_PSRAM_GET_NEXT_SCRATCHPAD_1, 4, 1, 1 },
#endif
#if defined BCM6855
	{ "TCPSPDTEST_SCRATCHPAD", 1, CORE_1_INDEX, &TCPSPDTEST_SCRATCHPAD_1, 16, 1, 1 },
#endif
#if defined BCM6855
	{ "PROCESSING4_STACK", 1, CORE_1_INDEX, &PROCESSING4_STACK_1, 320, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_REASON_TO_TC", 1, CORE_1_INDEX, &CPU_REASON_TO_TC_1, 64, 1, 1 },
#endif
#if defined BCM6855
	{ "GENERAL_TIMER", 1, CORE_1_INDEX, &GENERAL_TIMER_1, 16, 1, 1 },
#endif
#if defined BCM6855
	{ "PROCESSING5_STACK", 1, CORE_1_INDEX, &PROCESSING5_STACK_1, 320, 1, 1 },
#endif
#if defined BCM6855
	{ "TC_TO_CPU_RXQ", 1, CORE_1_INDEX, &TC_TO_CPU_RXQ_1, 64, 1, 1 },
#endif
#if defined BCM6855
	{ "SPDSVC_GEN_STACK", 1, CORE_1_INDEX, &SPDSVC_GEN_STACK_1, 128, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_RING_DESCRIPTORS_TABLE", 1, CORE_1_INDEX, &CPU_RING_DESCRIPTORS_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_RING_INTERRUPT_COUNTER_TABLE", 1, CORE_1_INDEX, &CPU_RING_INTERRUPT_COUNTER_TABLE_1, 18, 1, 1 },
#endif
#if defined BCM6855
	{ "EXC_TC_TO_CPU_RXQ", 1, CORE_1_INDEX, &EXC_TC_TO_CPU_RXQ_1, 64, 1, 1 },
#endif
#if defined BCM6855
	{ "SPDTEST_GEN_PARAM", 1, CORE_1_INDEX, &SPDTEST_GEN_PARAM_1, 4, 1, 1 },
#endif
#if defined BCM6855
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_1_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_1, 32, 1, 1 },
#endif
#if defined BCM6855
	{ "PROCESSING6_STACK", 1, CORE_1_INDEX, &PROCESSING6_STACK_1, 320, 1, 1 },
#endif
#if defined BCM6855
	{ "PKTGEN_SESSION_DATA", 1, CORE_1_INDEX, &PKTGEN_SESSION_DATA_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_1_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_1, 8, 1, 1 },
#endif
#if defined BCM6855
	{ "POLICER_PARAMS_TABLE", 1, CORE_1_INDEX, &POLICER_PARAMS_TABLE_1, 80, 1, 1 },
#endif
#if defined BCM6855
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_1_INDEX, &LOOPBACK_QUEUE_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM6855
	{ "GENERAL_TIMER_STACK", 1, CORE_1_INDEX, &GENERAL_TIMER_STACK_1, 32, 1, 1 },
#endif
#if defined BCM6855
	{ "PROCESSING7_STACK", 1, CORE_1_INDEX, &PROCESSING7_STACK_1, 320, 1, 1 },
#endif
#if defined BCM6855
	{ "REGISTERS_BUFFER", 1, CORE_1_INDEX, &REGISTERS_BUFFER_1, 32, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_RECYCLE_STACK", 1, CORE_1_INDEX, &CPU_RECYCLE_STACK_1, 32, 1, 1 },
#endif
#if defined BCM6855
	{ "COMMON_REPROCESSING_STACK", 1, CORE_1_INDEX, &COMMON_REPROCESSING_STACK_1, 32, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_RECYCLE_SRAM_PD_FIFO", 1, CORE_1_INDEX, &CPU_RECYCLE_SRAM_PD_FIFO_1, 16, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_RX_COPY_PD_FIFO_TABLE", 1, CORE_1_INDEX, &CPU_RX_COPY_PD_FIFO_TABLE_1, 8, 1, 1 },
#endif
#if defined BCM6855
	{ "TCPSPDTEST_PD_FIFO_TABLE", 1, CORE_1_INDEX, &TCPSPDTEST_PD_FIFO_TABLE_1, 4, 1, 1 },
#endif
#if defined BCM6855
	{ "UPDATE_FIFO_TABLE", 1, CORE_1_INDEX, &UPDATE_FIFO_TABLE_1, 8, 1, 1 },
#endif
#if defined BCM6855
	{ "CODEL_ENABLE_TABLE", 1, CORE_1_INDEX, &CODEL_ENABLE_TABLE_1, 5, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_RX_INTERRUPT_SCRATCH", 1, CORE_1_INDEX, &CPU_RX_INTERRUPT_SCRATCH_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "SYSTEM_CONFIGURATION", 1, CORE_1_INDEX, &SYSTEM_CONFIGURATION_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "SPDTEST_NUM_OF_UDP_RX_FLOWS", 1, CORE_1_INDEX, &SPDTEST_NUM_OF_UDP_RX_FLOWS_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "PKTGEN_CURR_SBPM_HDR_PTR", 1, CORE_1_INDEX, &PKTGEN_CURR_SBPM_HDR_PTR_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "UDPSPDT_STREAM_TX_STAT_TABLE", 1, CORE_1_INDEX, &UDPSPDT_STREAM_TX_STAT_TABLE_1, 4, 1, 1 },
#endif
#if defined BCM6855
	{ "TCPSPDTEST_ENGINE_GLOBAL_TABLE", 1, CORE_1_INDEX, &TCPSPDTEST_ENGINE_GLOBAL_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "PKTGEN_NUM_OF_AVAIL_SBPM_HDRS", 1, CORE_1_INDEX, &PKTGEN_NUM_OF_AVAIL_SBPM_HDRS_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "PKTGEN_SBPM_END_PTR", 1, CORE_1_INDEX, &PKTGEN_SBPM_END_PTR_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE", 1, CORE_1_INDEX, &CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_RX_COPY_UPDATE_FIFO_TABLE", 1, CORE_1_INDEX, &CPU_RX_COPY_UPDATE_FIFO_TABLE_1, 8, 1, 1 },
#endif
#if defined BCM6855
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_1_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM6855
	{ "BRIDGE_CFG_TABLE", 1, CORE_1_INDEX, &BRIDGE_CFG_TABLE_1, 2, 1, 1 },
#endif
#if defined BCM6855
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_1_INDEX, &INGRESS_FILTER_PROFILE_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM6855
	{ "PKTGEN_SBPM_HDR_BNS", 1, CORE_1_INDEX, &PKTGEN_SBPM_HDR_BNS_1, 28, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_FEED_RING_INDEX_DDR_ADDR_TABLE", 1, CORE_1_INDEX, &CPU_FEED_RING_INDEX_DDR_ADDR_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "COMMON_REPROCESSING_UPDATE_FIFO_TABLE", 1, CORE_1_INDEX, &COMMON_REPROCESSING_UPDATE_FIFO_TABLE_1, 8, 1, 1 },
#endif
#if defined BCM6855
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_1_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_1, 16, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_RX_COPY_DISPATCHER_CREDIT_TABLE", 1, CORE_1_INDEX, &CPU_RX_COPY_DISPATCHER_CREDIT_TABLE_1, 3, 1, 1 },
#endif
#if defined BCM6855
	{ "PKTGEN_BAD_GET_NEXT", 1, CORE_1_INDEX, &PKTGEN_BAD_GET_NEXT_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "UDPSPDT_TX_PARAMS_TABLE", 1, CORE_1_INDEX, &UDPSPDT_TX_PARAMS_TABLE_1, 4, 1, 1 },
#endif
#if defined BCM6855
	{ "SPDSVC_GEN_DISPATCHER_CREDIT_TABLE", 1, CORE_1_INDEX, &SPDSVC_GEN_DISPATCHER_CREDIT_TABLE_1, 3, 1, 1 },
#endif
#if defined BCM6855
	{ "PKTGEN_MAX_UT_PKTS", 1, CORE_1_INDEX, &PKTGEN_MAX_UT_PKTS_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "TCPSPDTEST_DISPATCHER_CREDIT_TABLE", 1, CORE_1_INDEX, &TCPSPDTEST_DISPATCHER_CREDIT_TABLE_1, 3, 1, 1 },
#endif
#if defined BCM6855
	{ "PKTGEN_UT_TRIGGER", 1, CORE_1_INDEX, &PKTGEN_UT_TRIGGER_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_VPORT_TO_METER_TABLE", 1, CORE_1_INDEX, &CPU_VPORT_TO_METER_TABLE_1, 40, 1, 1 },
#endif
#if defined BCM6855
	{ "TUNNELS_PARSING_CFG", 1, CORE_1_INDEX, &TUNNELS_PARSING_CFG_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "COMMON_REPROCESSING_DISPATCHER_CREDIT_TABLE", 1, CORE_1_INDEX, &COMMON_REPROCESSING_DISPATCHER_CREDIT_TABLE_1, 3, 1, 1 },
#endif
#if defined BCM6855
	{ "TASK_IDX", 1, CORE_1_INDEX, &TASK_IDX_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "TCPSPDTEST_UPDATE_FIFO_TABLE", 1, CORE_1_INDEX, &TCPSPDTEST_UPDATE_FIFO_TABLE_1, 8, 1, 1 },
#endif
#if defined BCM6855
	{ "TCPSPDTEST_GEN_DISPATCHER_CREDIT_TABLE", 1, CORE_1_INDEX, &TCPSPDTEST_GEN_DISPATCHER_CREDIT_TABLE_1, 3, 1, 1 },
#endif
#if defined BCM6855
	{ "ENABLE_VPORT_MASK", 1, CORE_1_INDEX, &ENABLE_VPORT_MASK_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "IPTV_CONFIGURATION_TABLE", 1, CORE_1_INDEX, &IPTV_CONFIGURATION_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "GENERAL_TIMER_ACTION_VEC", 1, CORE_1_INDEX, &GENERAL_TIMER_ACTION_VEC_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_1_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "NATC_TBL_CFG", 1, CORE_1_INDEX, &NATC_TBL_CFG_1, 4, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_FPM_THRESHOLDS", 1, CORE_1_INDEX, &DHD_FPM_THRESHOLDS_1, 3, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_FEED_RING_INTERRUPT_THRESHOLD", 1, CORE_1_INDEX, &CPU_FEED_RING_INTERRUPT_THRESHOLD_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_FEED_RING_INTERRUPT_COUNTER", 1, CORE_1_INDEX, &CPU_FEED_RING_INTERRUPT_COUNTER_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_RECYCLE_RING_DESCRIPTOR_TABLE", 1, CORE_1_INDEX, &CPU_RECYCLE_RING_DESCRIPTOR_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "PD_FIFO_TABLE", 1, CORE_1_INDEX, &PD_FIFO_TABLE_1, 2, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE", 1, CORE_1_INDEX, &CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE_1, 2, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_INTERRUPT_COALESCING_TABLE", 1, CORE_1_INDEX, &CPU_INTERRUPT_COALESCING_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "COMMON_REPROCESSING_PD_FIFO_TABLE", 1, CORE_1_INDEX, &COMMON_REPROCESSING_PD_FIFO_TABLE_1, 2, 1, 1 },
#endif
#if defined BCM6855
	{ "DEBUG_SCRATCHPAD", 1, CORE_1_INDEX, &DEBUG_SCRATCHPAD_1, 12, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_FEED_RING_DESCRIPTOR_TABLE", 1, CORE_1_INDEX, &CPU_FEED_RING_DESCRIPTOR_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_RECYCLE_INTERRUPT_SCRATCH", 1, CORE_1_INDEX, &CPU_RECYCLE_INTERRUPT_SCRATCH_1, 2, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_RX_LOCAL_SCRATCH", 1, CORE_1_INDEX, &CPU_RX_LOCAL_SCRATCH_1, 2, 1, 1 },
#endif
#if defined BCM6855
	{ "IPTV_CFG_TABLE", 1, CORE_1_INDEX, &IPTV_CFG_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "VPORT_TO_CPU_OBJ", 1, CORE_1_INDEX, &VPORT_TO_CPU_OBJ_1, 40, 1, 1 },
#endif
#if defined BCM6855
	{ "PKTGEN_SBPM_EXTS", 1, CORE_1_INDEX, &PKTGEN_SBPM_EXTS_1, 4, 1, 1 },
#endif
#if defined BCM6855
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_1_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "PKTGEN_FPM_UG_MGMT", 1, CORE_1_INDEX, &PKTGEN_FPM_UG_MGMT_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_1_INDEX, &LOOPBACK_WAN_FLOW_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "FORCE_DSCP", 1, CORE_1_INDEX, &FORCE_DSCP_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "CORE_ID_TABLE", 1, CORE_1_INDEX, &CORE_ID_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "SPDSVC_TCPSPDTEST_COMMON_TABLE", 1, CORE_1_INDEX, &SPDSVC_TCPSPDTEST_COMMON_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "NAT_CACHE_CFG", 1, CORE_1_INDEX, &NAT_CACHE_CFG_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_FEED_RING_CACHE_OFFSET", 1, CORE_1_INDEX, &CPU_FEED_RING_CACHE_OFFSET_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "PKTGEN_BBMSG_REPLY_SCRATCH", 1, CORE_1_INDEX, &PKTGEN_BBMSG_REPLY_SCRATCH_1, 2, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_RECYCLE_SHADOW_RD_IDX", 1, CORE_1_INDEX, &CPU_RECYCLE_SHADOW_RD_IDX_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_1_INDEX, &RX_MIRRORING_CONFIGURATION_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_REDIRECT_MODE", 1, CORE_1_INDEX, &CPU_REDIRECT_MODE_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "CSO_DISABLE", 1, CORE_1_INDEX, &CSO_DISABLE_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_1_INDEX, &DEBUG_PRINT_CORE_LOCK_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "SRAM_DUMMY_STORE", 1, CORE_1_INDEX, &SRAM_DUMMY_STORE_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "DEBUG_PRINT_TABLE", 1, CORE_1_INDEX, &DEBUG_PRINT_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_1_INDEX, &BITS_CALC_MASKS_TABLE_1, 4, 1, 1 },
#endif
#if defined BCM6855
	{ "TCAM_GENERIC_FIELDS", 1, CORE_1_INDEX, &TCAM_GENERIC_FIELDS_1, 8, 1, 1 },
#endif
#if defined BCM6855
	{ "FPM_GLOBAL_CFG", 1, CORE_1_INDEX, &FPM_GLOBAL_CFG_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "MULTICAST_WHITELIST_CFG_TABLE", 1, CORE_1_INDEX, &MULTICAST_WHITELIST_CFG_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "RATE_LIMIT_OVERHEAD", 1, CORE_1_INDEX, &RATE_LIMIT_OVERHEAD_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_1_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_1_INDEX, &INGRESS_FILTER_1588_CFG_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_1_INDEX, &RX_MIRRORING_DIRECT_ENABLE_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "FC_FLOW_IP_ADDRESSES_TABLE", 1, CORE_1_INDEX, &FC_FLOW_IP_ADDRESSES_TABLE_1, 4, 1, 1 },
#endif
#if defined BCM6855
	{ "MULTICAST_KEY_MASK", 1, CORE_1_INDEX, &MULTICAST_KEY_MASK_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_RECYCLE_SHADOW_WR_IDX", 1, CORE_1_INDEX, &CPU_RECYCLE_SHADOW_WR_IDX_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_RECYCLE_NEXT_PTR_TABLE", 1, CORE_1_INDEX, &CPU_RECYCLE_NEXT_PTR_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_1_INDEX, &NAT_CACHE_KEY0_MASK_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_1_INDEX, &TCAM_TABLE_CFG_TABLE_1, 2, 1, 1 },
#endif
#if defined BCM6855
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_1_INDEX, &NATC_L2_VLAN_KEY_MASK_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "INGRESS_FILTER_CFG", 1, CORE_1_INDEX, &INGRESS_FILTER_CFG_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "NATC_L2_TOS_MASK", 1, CORE_1_INDEX, &NATC_L2_TOS_MASK_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "PKTGEN_TX_STREAM_SCRATCH_TABLE", 1, CORE_1_INDEX, &PKTGEN_TX_STREAM_SCRATCH_TABLE_1, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "SERVICE_QUEUES_RATE_LIMITER_TABLE", 1, CORE_2_INDEX, &SERVICE_QUEUES_RATE_LIMITER_TABLE_2, 66, 1, 1 },
#endif
#if defined BCM6855
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_2_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_2, 32, 1, 1 },
#endif
#if defined BCM6855
	{ "VPORT_CFG_TABLE", 1, CORE_2_INDEX, &VPORT_CFG_TABLE_2, 16, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_TX_DBG_CNTRS_TABLE", 1, CORE_2_INDEX, &CPU_TX_DBG_CNTRS_TABLE_2, 2, 1, 1 },
#endif
#if defined BCM6855
	{ "SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE", 1, CORE_2_INDEX, &SERVICE_QUEUES_SCHEDULING_QUEUE_TABLE_2, 32, 1, 1 },
#endif
#if defined BCM6855
	{ "PROCESSING0_STACK", 1, CORE_2_INDEX, &PROCESSING0_STACK_2, 320, 1, 1 },
#endif
#if defined BCM6855
	{ "CODEL_SQ_TABLE", 1, CORE_2_INDEX, &CODEL_SQ_TABLE_2, 32, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_TX_0_STACK", 1, CORE_2_INDEX, &CPU_TX_0_STACK_2, 128, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_TX_SCRATCHPAD", 1, CORE_2_INDEX, &CPU_TX_SCRATCHPAD_2, 128, 1, 1 },
#endif
#if defined BCM6855
	{ "RX_FLOW_TABLE", 1, CORE_2_INDEX, &RX_FLOW_TABLE_2, 340, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR", 1, CORE_2_INDEX, &CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_RECYCLE_INTERRUPT_COALESCING_TABLE", 1, CORE_2_INDEX, &CPU_RECYCLE_INTERRUPT_COALESCING_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "SERVICE_QUEUES_COMPLEX_SCHEDULER_TABLE", 1, CORE_2_INDEX, &SERVICE_QUEUES_COMPLEX_SCHEDULER_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "TX_FLOW_TABLE", 1, CORE_2_INDEX, &TX_FLOW_TABLE_2, 212, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_INT_INTERRUPT_SCRATCH", 1, CORE_2_INDEX, &CPU_INT_INTERRUPT_SCRATCH_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_RX_INTERRUPT_ID_DDR_ADDR", 1, CORE_2_INDEX, &CPU_RX_INTERRUPT_ID_DDR_ADDR_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_2_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_2, 8, 1, 1 },
#endif
#if defined BCM6855
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_2_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER_2, 8, 1, 1 },
#endif
#if defined BCM6855
	{ "SERVICE_QUEUES_PD_FIFO_TABLE", 1, CORE_2_INDEX, &SERVICE_QUEUES_PD_FIFO_TABLE_2, 64, 1, 1 },
#endif
#if defined BCM6855
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_2_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_2, 128, 1, 1 },
#endif
#if defined BCM6855
	{ "PROCESSING1_STACK", 1, CORE_2_INDEX, &PROCESSING1_STACK_2, 320, 1, 1 },
#endif
#if defined BCM6855
	{ "CODEL_BIAS_SLOPE_TABLE", 1, CORE_2_INDEX, &CODEL_BIAS_SLOPE_TABLE_2, 11, 1, 1 },
#endif
#if defined BCM6855
	{ "DDR_LATENCY_DBG_USEC", 1, CORE_2_INDEX, &DDR_LATENCY_DBG_USEC_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "SPDTEST_GEN_PARAM", 1, CORE_2_INDEX, &SPDTEST_GEN_PARAM_2, 4, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_TX_1_STACK", 1, CORE_2_INDEX, &CPU_TX_1_STACK_2, 128, 1, 1 },
#endif
#if defined BCM6855
	{ "PROCESSING2_STACK", 1, CORE_2_INDEX, &PROCESSING2_STACK_2, 320, 1, 1 },
#endif
#if defined BCM6855
	{ "QUEUE_THRESHOLD_VECTOR", 1, CORE_2_INDEX, &QUEUE_THRESHOLD_VECTOR_2, 9, 1, 1 },
#endif
#if defined BCM6855
	{ "DDR_LATENCY_DBG_USEC_MAX", 1, CORE_2_INDEX, &DDR_LATENCY_DBG_USEC_MAX_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_2_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_2_INDEX, &LOOPBACK_QUEUE_TABLE_2, 16, 1, 1 },
#endif
#if defined BCM6855
	{ "GENERAL_TIMER", 1, CORE_2_INDEX, &GENERAL_TIMER_2, 16, 1, 1 },
#endif
#if defined BCM6855
	{ "PROCESSING3_STACK", 1, CORE_2_INDEX, &PROCESSING3_STACK_2, 320, 1, 1 },
#endif
#if defined BCM6855
	{ "VLAN_ACTION_GPE_HANDLER_PTR_TABLE", 1, CORE_2_INDEX, &VLAN_ACTION_GPE_HANDLER_PTR_TABLE_2, 17, 1, 1 },
#endif
#if defined BCM6855
	{ "CODEL_NUM_QUEUES", 1, CORE_2_INDEX, &CODEL_NUM_QUEUES_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "MUTEX_TABLE", 1, CORE_2_INDEX, &MUTEX_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "SYSTEM_CONFIGURATION", 1, CORE_2_INDEX, &SYSTEM_CONFIGURATION_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "SPDTEST_NUM_OF_UDP_RX_FLOWS", 1, CORE_2_INDEX, &SPDTEST_NUM_OF_UDP_RX_FLOWS_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "GENERAL_TIMER_ACTION_VEC", 1, CORE_2_INDEX, &GENERAL_TIMER_ACTION_VEC_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_2_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_2, 16, 1, 1 },
#endif
#if defined BCM6855
	{ "POLICER_PARAMS_TABLE", 1, CORE_2_INDEX, &POLICER_PARAMS_TABLE_2, 80, 1, 1 },
#endif
#if defined BCM6855
	{ "BUFFER_ALLOC_REPLY", 1, CORE_2_INDEX, &BUFFER_ALLOC_REPLY_2, 2, 1, 1 },
#endif
#if defined BCM6855
	{ "GENERAL_TIMER_STACK", 1, CORE_2_INDEX, &GENERAL_TIMER_STACK_2, 32, 1, 1 },
#endif
#if defined BCM6855
	{ "PROCESSING4_STACK", 1, CORE_2_INDEX, &PROCESSING4_STACK_2, 320, 1, 1 },
#endif
#if defined BCM6855
	{ "REGISTERS_BUFFER", 1, CORE_2_INDEX, &REGISTERS_BUFFER_2, 32, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_RECYCLE_STACK", 1, CORE_2_INDEX, &CPU_RECYCLE_STACK_2, 32, 1, 1 },
#endif
#if defined BCM6855
	{ "SERVICE_QUEUES_SCHEDULING_QUEUE_AGING_VECTOR", 1, CORE_2_INDEX, &SERVICE_QUEUES_SCHEDULING_QUEUE_AGING_VECTOR_2, 5, 1, 1 },
#endif
#if defined BCM6855
	{ "TASK_IDX", 1, CORE_2_INDEX, &TASK_IDX_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE", 1, CORE_2_INDEX, &CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "PROCESSING5_STACK", 1, CORE_2_INDEX, &PROCESSING5_STACK_2, 320, 1, 1 },
#endif
#if defined BCM6855
	{ "SERVICE_QUEUES_CODEL_QUEUE_TABLE", 1, CORE_2_INDEX, &SERVICE_QUEUES_CODEL_QUEUE_TABLE_2, 32, 1, 1 },
#endif
#if defined BCM6855
	{ "SERVICE_QUEUES_UPDATE_FIFO_TABLE", 1, CORE_2_INDEX, &SERVICE_QUEUES_UPDATE_FIFO_TABLE_2, 8, 1, 1 },
#endif
#if defined BCM6855
	{ "CODEL_ENABLE_TABLE", 1, CORE_2_INDEX, &CODEL_ENABLE_TABLE_2, 5, 1, 1 },
#endif
#if defined BCM6855
	{ "ENABLE_VPORT_MASK", 1, CORE_2_INDEX, &ENABLE_VPORT_MASK_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_TX_RING_INDICES_VALUES_TABLE", 1, CORE_2_INDEX, &CPU_TX_RING_INDICES_VALUES_TABLE_2, 2, 1, 1 },
#endif
#if defined BCM6855
	{ "PROCESSING6_STACK", 1, CORE_2_INDEX, &PROCESSING6_STACK_2, 320, 1, 1 },
#endif
#if defined BCM6855
	{ "DEBUG_SCRATCHPAD", 1, CORE_2_INDEX, &DEBUG_SCRATCHPAD_2, 12, 1, 1 },
#endif
#if defined BCM6855
	{ "SERVICE_QUEUES_FLUSH_CFG_FW_TABLE", 1, CORE_2_INDEX, &SERVICE_QUEUES_FLUSH_CFG_FW_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "RING_CPU_TX_DESCRIPTOR_DATA_TABLE", 1, CORE_2_INDEX, &RING_CPU_TX_DESCRIPTOR_DATA_TABLE_2, 2, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_TX_RING_DESCRIPTOR_TABLE", 1, CORE_2_INDEX, &CPU_TX_RING_DESCRIPTOR_TABLE_2, 2, 1, 1 },
#endif
#if defined BCM6855
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_2_INDEX, &BITS_CALC_MASKS_TABLE_2, 4, 1, 1 },
#endif
#if defined BCM6855
	{ "SERVICE_QUEUES_FLUSH_CFG_CURRENT_TABLE", 1, CORE_2_INDEX, &SERVICE_QUEUES_FLUSH_CFG_CURRENT_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "SERVICE_QUEUES_RATE_LIMITER_VALID_TABLE", 1, CORE_2_INDEX, &SERVICE_QUEUES_RATE_LIMITER_VALID_TABLE_2, 4, 1, 1 },
#endif
#if defined BCM6855
	{ "SERVICE_QUEUES_CODEL_DROP_DESCRIPTOR", 1, CORE_2_INDEX, &SERVICE_QUEUES_CODEL_DROP_DESCRIPTOR_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "PROCESSING7_STACK", 1, CORE_2_INDEX, &PROCESSING7_STACK_2, 320, 1, 1 },
#endif
#if defined BCM6855
	{ "BRIDGE_CFG_TABLE", 1, CORE_2_INDEX, &BRIDGE_CFG_TABLE_2, 2, 1, 1 },
#endif
#if defined BCM6855
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_2_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_2, 16, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE", 1, CORE_2_INDEX, &CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE_2, 3, 1, 1 },
#endif
#if defined BCM6855
	{ "SERVICE_QUEUES_FLUSH_CFG_ENABLE_TABLE", 1, CORE_2_INDEX, &SERVICE_QUEUES_FLUSH_CFG_ENABLE_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_2_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_TX_INGRESS_DISPATCHER_CREDIT_TABLE", 1, CORE_2_INDEX, &CPU_TX_INGRESS_DISPATCHER_CREDIT_TABLE_2, 3, 1, 1 },
#endif
#if defined BCM6855
	{ "SERVICE_QUEUES_BUDGET_ALLOCATION_TIMER_VALUE", 1, CORE_2_INDEX, &SERVICE_QUEUES_BUDGET_ALLOCATION_TIMER_VALUE_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_2_INDEX, &LOOPBACK_WAN_FLOW_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "FORCE_DSCP", 1, CORE_2_INDEX, &FORCE_DSCP_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_TX_SYNC_FIFO_TABLE", 1, CORE_2_INDEX, &CPU_TX_SYNC_FIFO_TABLE_2, 2, 1, 1 },
#endif
#if defined BCM6855
	{ "IPTV_CONFIGURATION_TABLE", 1, CORE_2_INDEX, &IPTV_CONFIGURATION_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "CORE_ID_TABLE", 1, CORE_2_INDEX, &CORE_ID_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_REDIRECT_MODE", 1, CORE_2_INDEX, &CPU_REDIRECT_MODE_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_2_INDEX, &RX_MIRRORING_CONFIGURATION_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_FPM_THRESHOLDS", 1, CORE_2_INDEX, &DHD_FPM_THRESHOLDS_2, 3, 1, 1 },
#endif
#if defined BCM6855
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_2_INDEX, &DEBUG_PRINT_CORE_LOCK_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "SRAM_DUMMY_STORE", 1, CORE_2_INDEX, &SRAM_DUMMY_STORE_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "RATE_LIMIT_OVERHEAD", 1, CORE_2_INDEX, &RATE_LIMIT_OVERHEAD_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "SERVICE_QUEUES_FIRST_QUEUE_MAPPING", 1, CORE_2_INDEX, &SERVICE_QUEUES_FIRST_QUEUE_MAPPING_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "SERVICE_QUEUES_DISPATCHER_CREDIT_TABLE", 1, CORE_2_INDEX, &SERVICE_QUEUES_DISPATCHER_CREDIT_TABLE_2, 3, 1, 1 },
#endif
#if defined BCM6855
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_2_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_2_INDEX, &INGRESS_FILTER_1588_CFG_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_2_INDEX, &RX_MIRRORING_DIRECT_ENABLE_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_RECYCLE_RING_DESCRIPTOR_TABLE", 1, CORE_2_INDEX, &CPU_RECYCLE_RING_DESCRIPTOR_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE", 1, CORE_2_INDEX, &CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE_2, 2, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_FEED_RING_DESCRIPTOR_TABLE", 1, CORE_2_INDEX, &CPU_FEED_RING_DESCRIPTOR_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_RECYCLE_INTERRUPT_SCRATCH", 1, CORE_2_INDEX, &CPU_RECYCLE_INTERRUPT_SCRATCH_2, 2, 1, 1 },
#endif
#if defined BCM6855
	{ "TUNNELS_PARSING_CFG", 1, CORE_2_INDEX, &TUNNELS_PARSING_CFG_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_RECYCLE_SRAM_PD_FIFO", 1, CORE_2_INDEX, &CPU_RECYCLE_SRAM_PD_FIFO_2, 16, 1, 1 },
#endif
#if defined BCM6855
	{ "SQ_TX_QUEUE_DROP_TABLE", 1, CORE_2_INDEX, &SQ_TX_QUEUE_DROP_TABLE_2, 32, 1, 1 },
#endif
#if defined BCM6855
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_2_INDEX, &INGRESS_FILTER_PROFILE_TABLE_2, 16, 1, 1 },
#endif
#if defined BCM6855
	{ "DEBUG_PRINT_TABLE", 1, CORE_2_INDEX, &DEBUG_PRINT_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "TCAM_GENERIC_FIELDS", 1, CORE_2_INDEX, &TCAM_GENERIC_FIELDS_2, 8, 1, 1 },
#endif
#if defined BCM6855
	{ "FPM_GLOBAL_CFG", 1, CORE_2_INDEX, &FPM_GLOBAL_CFG_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "MULTICAST_WHITELIST_CFG_TABLE", 1, CORE_2_INDEX, &MULTICAST_WHITELIST_CFG_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "MULTICAST_KEY_MASK", 1, CORE_2_INDEX, &MULTICAST_KEY_MASK_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "IPTV_CFG_TABLE", 1, CORE_2_INDEX, &IPTV_CFG_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_2_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "NAT_CACHE_CFG", 1, CORE_2_INDEX, &NAT_CACHE_CFG_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_RECYCLE_SHADOW_RD_IDX", 1, CORE_2_INDEX, &CPU_RECYCLE_SHADOW_RD_IDX_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_RECYCLE_SHADOW_WR_IDX", 1, CORE_2_INDEX, &CPU_RECYCLE_SHADOW_WR_IDX_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_RECYCLE_NEXT_PTR_TABLE", 1, CORE_2_INDEX, &CPU_RECYCLE_NEXT_PTR_TABLE_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_2_INDEX, &NAT_CACHE_KEY0_MASK_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "NATC_TBL_CFG", 1, CORE_2_INDEX, &NATC_TBL_CFG_2, 4, 1, 1 },
#endif
#if defined BCM6855
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_2_INDEX, &TCAM_TABLE_CFG_TABLE_2, 2, 1, 1 },
#endif
#if defined BCM6855
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_2_INDEX, &NATC_L2_VLAN_KEY_MASK_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "INGRESS_FILTER_CFG", 1, CORE_2_INDEX, &INGRESS_FILTER_CFG_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "NATC_L2_TOS_MASK", 1, CORE_2_INDEX, &NATC_L2_TOS_MASK_2, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "FC_FLOW_IP_ADDRESSES_TABLE", 1, CORE_2_INDEX, &FC_FLOW_IP_ADDRESSES_TABLE_2, 4, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_FLOW_RING_CACHE_CTX_TABLE", 1, CORE_3_INDEX, &DHD_FLOW_RING_CACHE_CTX_TABLE_3, 48, 1, 1 },
#endif
#if defined BCM6855
	{ "TX_FLOW_TABLE", 1, CORE_3_INDEX, &TX_FLOW_TABLE_3, 212, 1, 1 },
#endif
#if defined BCM6855
	{ "MUTEX_TABLE", 1, CORE_3_INDEX, &MUTEX_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_3_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_3_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_3, 32, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_AUX_INFO_CACHE_TABLE", 1, CORE_3_INDEX, &DHD_AUX_INFO_CACHE_TABLE_3, 48, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_TX_POST_FLOW_RING_BUFFER", 1, CORE_3_INDEX, &DHD_TX_POST_FLOW_RING_BUFFER_3, 3, 1, 1 },
#endif
#if defined BCM6855
	{ "SPDTEST_GEN_PARAM", 1, CORE_3_INDEX, &SPDTEST_GEN_PARAM_3, 4, 1, 1 },
#endif
#if defined BCM6855
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_3_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_3, 8, 1, 1 },
#endif
#if defined BCM6855
	{ "VPORT_CFG_TABLE", 1, CORE_3_INDEX, &VPORT_CFG_TABLE_3, 16, 1, 1 },
#endif
#if defined BCM6855
	{ "RX_FLOW_TABLE", 1, CORE_3_INDEX, &RX_FLOW_TABLE_3, 340, 1, 1 },
#endif
#if defined BCM6855
	{ "MIRRORING_SCRATCH", 1, CORE_3_INDEX, &MIRRORING_SCRATCH_3, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_3_INDEX, &LOOPBACK_QUEUE_TABLE_3, 16, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_CODEL_BIAS_SLOPE_TABLE", 1, CORE_3_INDEX, &DHD_CODEL_BIAS_SLOPE_TABLE_3, 11, 1, 1 },
#endif
#if defined BCM6855
	{ "CODEL_NUM_QUEUES", 1, CORE_3_INDEX, &CODEL_NUM_QUEUES_3, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_3_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_3_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_3, 16, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_TX_POST_0_STACK", 1, CORE_3_INDEX, &DHD_TX_POST_0_STACK_3, 128, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_TX_POST_1_STACK", 1, CORE_3_INDEX, &DHD_TX_POST_1_STACK_3, 128, 1, 1 },
#endif
#if defined BCM6855
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_3_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_3, 128, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_POST_COMMON_RADIO_DATA", 1, CORE_3_INDEX, &DHD_POST_COMMON_RADIO_DATA_3, 3, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR", 1, CORE_3_INDEX, &DHD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_3, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_3_INDEX, &DHD_FPM_POOL_NUMBER_MAPPING_TABLE_3, 16, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_TIMER_STACK", 1, CORE_3_INDEX, &DHD_TIMER_STACK_3, 32, 1, 1 },
#endif
#if defined BCM6855
	{ "PROCESSING_8_TASKS_PACKET_BUFFER", 1, CORE_3_INDEX, &PROCESSING_8_TASKS_PACKET_BUFFER_3, 8, 1, 1 },
#endif
#if defined BCM6855
	{ "PROCESSING0_STACK", 1, CORE_3_INDEX, &PROCESSING0_STACK_3, 320, 1, 1 },
#endif
#if defined BCM6855
	{ "VLAN_ACTION_GPE_HANDLER_PTR_TABLE", 1, CORE_3_INDEX, &VLAN_ACTION_GPE_HANDLER_PTR_TABLE_3, 17, 1, 1 },
#endif
#if defined BCM6855
	{ "SPDSVC_WLAN_TXPOST_PARAMS_TABLE", 1, CORE_3_INDEX, &SPDSVC_WLAN_TXPOST_PARAMS_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "TASK_IDX", 1, CORE_3_INDEX, &TASK_IDX_3, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_BACKUP_BASE_ADDR", 1, CORE_3_INDEX, &DHD_BACKUP_BASE_ADDR_3, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_HW_CFG", 1, CORE_3_INDEX, &DHD_HW_CFG_3, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_TX_POST_2_STACK", 1, CORE_3_INDEX, &DHD_TX_POST_2_STACK_3, 128, 1, 1 },
#endif
#if defined BCM6855
	{ "PROCESSING1_STACK", 1, CORE_3_INDEX, &PROCESSING1_STACK_3, 320, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_TX_POST_UPDATE_FIFO_TABLE", 1, CORE_3_INDEX, &DHD_TX_POST_UPDATE_FIFO_TABLE_3, 8, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_TX_POST_UPDATE_FIFO_STACK", 1, CORE_3_INDEX, &DHD_TX_POST_UPDATE_FIFO_STACK_3, 32, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_BACKUP_INDEX_CACHE", 1, CORE_3_INDEX, &DHD_BACKUP_INDEX_CACHE_3, 3, 1, 1 },
#endif
#if defined BCM6855
	{ "CODEL_ENABLE_TABLE", 1, CORE_3_INDEX, &CODEL_ENABLE_TABLE_3, 5, 1, 1 },
#endif
#if defined BCM6855
	{ "ENABLE_VPORT_MASK", 1, CORE_3_INDEX, &ENABLE_VPORT_MASK_3, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "SYSTEM_CONFIGURATION", 1, CORE_3_INDEX, &SYSTEM_CONFIGURATION_3, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "SPDTEST_NUM_OF_UDP_RX_FLOWS", 1, CORE_3_INDEX, &SPDTEST_NUM_OF_UDP_RX_FLOWS_3, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_3_INDEX, &LOOPBACK_WAN_FLOW_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "FORCE_DSCP", 1, CORE_3_INDEX, &FORCE_DSCP_3, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "PROCESSING2_STACK", 1, CORE_3_INDEX, &PROCESSING2_STACK_3, 320, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_DOORBELL_TX_POST_VALUE", 1, CORE_3_INDEX, &DHD_DOORBELL_TX_POST_VALUE_3, 16, 1, 1 },
#endif
#if defined BCM6855
	{ "POLICER_PARAMS_TABLE", 1, CORE_3_INDEX, &POLICER_PARAMS_TABLE_3, 80, 1, 1 },
#endif
#if defined BCM6855
	{ "DEBUG_SCRATCHPAD", 1, CORE_3_INDEX, &DEBUG_SCRATCHPAD_3, 12, 1, 1 },
#endif
#if defined BCM6855
	{ "PROCESSING3_STACK", 1, CORE_3_INDEX, &PROCESSING3_STACK_3, 320, 1, 1 },
#endif
#if defined BCM6855
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_3_INDEX, &BITS_CALC_MASKS_TABLE_3, 4, 1, 1 },
#endif
#if defined BCM6855
	{ "BRIDGE_CFG_TABLE", 1, CORE_3_INDEX, &BRIDGE_CFG_TABLE_3, 2, 1, 1 },
#endif
#if defined BCM6855
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_3_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_3, 16, 1, 1 },
#endif
#if defined BCM6855
	{ "IPTV_CONFIGURATION_TABLE", 1, CORE_3_INDEX, &IPTV_CONFIGURATION_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "CORE_ID_TABLE", 1, CORE_3_INDEX, &CORE_ID_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_REDIRECT_MODE", 1, CORE_3_INDEX, &CPU_REDIRECT_MODE_3, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_3_INDEX, &RX_MIRRORING_CONFIGURATION_3, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_L2_HEADER", 1, CORE_3_INDEX, &DHD_L2_HEADER_3, 72, 1, 1 },
#endif
#if defined BCM6855
	{ "TUNNELS_PARSING_CFG", 1, CORE_3_INDEX, &TUNNELS_PARSING_CFG_3, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_FPM_THRESHOLDS", 1, CORE_3_INDEX, &DHD_FPM_THRESHOLDS_3, 3, 1, 1 },
#endif
#if defined BCM6855
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_3_INDEX, &DEBUG_PRINT_CORE_LOCK_3, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "SRAM_DUMMY_STORE", 1, CORE_3_INDEX, &SRAM_DUMMY_STORE_3, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "RATE_LIMIT_OVERHEAD", 1, CORE_3_INDEX, &RATE_LIMIT_OVERHEAD_3, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_3_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_CPU_INT_ID", 1, CORE_3_INDEX, &DHD_CPU_INT_ID_3, 3, 1, 1 },
#endif
#if defined BCM6855
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_3_INDEX, &INGRESS_FILTER_1588_CFG_3, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_3_INDEX, &RX_MIRRORING_DIRECT_ENABLE_3, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_MIRRORING_DISPATCHER_CREDIT_TABLE", 1, CORE_3_INDEX, &DHD_MIRRORING_DISPATCHER_CREDIT_TABLE_3, 3, 1, 1 },
#endif
#if defined BCM6855
	{ "PROCESSING4_STACK", 1, CORE_3_INDEX, &PROCESSING4_STACK_3, 320, 1, 1 },
#endif
#if defined BCM6855
	{ "REGISTERS_BUFFER", 1, CORE_3_INDEX, &REGISTERS_BUFFER_3, 32, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_FEED_RING_DESCRIPTOR_TABLE", 1, CORE_3_INDEX, &CPU_FEED_RING_DESCRIPTOR_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "DEBUG_PRINT_TABLE", 1, CORE_3_INDEX, &DEBUG_PRINT_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "TCAM_GENERIC_FIELDS", 1, CORE_3_INDEX, &TCAM_GENERIC_FIELDS_3, 8, 1, 1 },
#endif
#if defined BCM6855
	{ "FPM_GLOBAL_CFG", 1, CORE_3_INDEX, &FPM_GLOBAL_CFG_3, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "PROCESSING5_STACK", 1, CORE_3_INDEX, &PROCESSING5_STACK_3, 320, 1, 1 },
#endif
#if defined BCM6855
	{ "MULTICAST_WHITELIST_CFG_TABLE", 1, CORE_3_INDEX, &MULTICAST_WHITELIST_CFG_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "IPTV_CFG_TABLE", 1, CORE_3_INDEX, &IPTV_CFG_TABLE_3, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_3_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_3, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "MULTICAST_KEY_MASK", 1, CORE_3_INDEX, &MULTICAST_KEY_MASK_3, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "NAT_CACHE_CFG", 1, CORE_3_INDEX, &NAT_CACHE_CFG_3, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_3_INDEX, &NAT_CACHE_KEY0_MASK_3, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_3_INDEX, &TCAM_TABLE_CFG_TABLE_3, 2, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE", 1, CORE_3_INDEX, &DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE_3, 3, 1, 1 },
#endif
#if defined BCM6855
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_3_INDEX, &NATC_L2_VLAN_KEY_MASK_3, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "INGRESS_FILTER_CFG", 1, CORE_3_INDEX, &INGRESS_FILTER_CFG_3, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "NATC_L2_TOS_MASK", 1, CORE_3_INDEX, &NATC_L2_TOS_MASK_3, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "PROCESSING6_STACK", 1, CORE_3_INDEX, &PROCESSING6_STACK_3, 320, 1, 1 },
#endif
#if defined BCM6855
	{ "PROCESSING7_STACK", 1, CORE_3_INDEX, &PROCESSING7_STACK_3, 320, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_TX_POST_PD_FIFO_TABLE", 1, CORE_3_INDEX, &DHD_TX_POST_PD_FIFO_TABLE_3, 12, 1, 1 },
#endif
#if defined BCM6855
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_3_INDEX, &INGRESS_FILTER_PROFILE_TABLE_3, 16, 1, 1 },
#endif
#if defined BCM6855
	{ "NATC_TBL_CFG", 1, CORE_3_INDEX, &NATC_TBL_CFG_3, 4, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_FLOW_RING_CACHE_LKP_TABLE", 1, CORE_3_INDEX, &DHD_FLOW_RING_CACHE_LKP_TABLE_3, 48, 1, 1 },
#endif
#if defined BCM6855
	{ "FC_FLOW_IP_ADDRESSES_TABLE", 1, CORE_3_INDEX, &FC_FLOW_IP_ADDRESSES_TABLE_3, 4, 1, 1 },
#endif
#if defined BCM6855
	{ "DS_TM_PD_FIFO_TABLE", 1, CORE_4_INDEX, &DS_TM_PD_FIFO_TABLE_4, 96, 1, 1 },
#endif
#if defined BCM6855
	{ "DS_TM_SCHEDULING_QUEUE_TABLE", 1, CORE_4_INDEX, &DS_TM_SCHEDULING_QUEUE_TABLE_4, 48, 1, 1 },
#endif
#if defined BCM6855
	{ "BASIC_SCHEDULER_TABLE_DS", 1, CORE_4_INDEX, &BASIC_SCHEDULER_TABLE_DS_4, 6, 1, 1 },
#endif
#if defined BCM6855
	{ "VPORT_TO_LOOKUP_PORT_MAPPING_TABLE", 1, CORE_4_INDEX, &VPORT_TO_LOOKUP_PORT_MAPPING_TABLE_4, 32, 1, 1 },
#endif
#if defined BCM6855
	{ "REPORTING_QUEUE_DESCRIPTOR_TABLE", 1, CORE_4_INDEX, &REPORTING_QUEUE_DESCRIPTOR_TABLE_4, 129, 1, 1 },
#endif
#if defined BCM6855
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_4_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "DS_TM_CODEL_DROP_DESCRIPTOR", 1, CORE_4_INDEX, &DS_TM_CODEL_DROP_DESCRIPTOR_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_4_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_4, 8, 1, 1 },
#endif
#if defined BCM6855
	{ "DS_TM_TM_FLOW_CNTR_TABLE", 1, CORE_4_INDEX, &DS_TM_TM_FLOW_CNTR_TABLE_4, 64, 1, 1 },
#endif
#if defined BCM6855
	{ "POLICER_PARAMS_TABLE", 1, CORE_4_INDEX, &POLICER_PARAMS_TABLE_4, 80, 1, 1 },
#endif
#if defined BCM6855
	{ "SPDTEST_GEN_PARAM", 1, CORE_4_INDEX, &SPDTEST_GEN_PARAM_4, 4, 1, 1 },
#endif
#if defined BCM6855
	{ "REPORTING_STACK", 1, CORE_4_INDEX, &REPORTING_STACK_4, 32, 1, 1 },
#endif
#if defined BCM6855
	{ "REPORTING_QUEUE_ACCUMULATED_TABLE", 1, CORE_4_INDEX, &REPORTING_QUEUE_ACCUMULATED_TABLE_4, 16, 1, 1 },
#endif
#if defined BCM6855
	{ "PROCESSING0_STACK", 1, CORE_4_INDEX, &PROCESSING0_STACK_4, 320, 1, 1 },
#endif
#if defined BCM6855
	{ "REGISTERS_BUFFER", 1, CORE_4_INDEX, &REGISTERS_BUFFER_4, 32, 1, 1 },
#endif
#if defined BCM6855
	{ "VPORT_CFG_TABLE", 1, CORE_4_INDEX, &VPORT_CFG_TABLE_4, 16, 1, 1 },
#endif
#if defined BCM6855
	{ "PROCESSING_4_TASKS_PACKET_BUFFER", 1, CORE_4_INDEX, &PROCESSING_4_TASKS_PACKET_BUFFER_4, 4, 1, 1 },
#endif
#if defined BCM6855
	{ "COMPLEX_SCHEDULER_TABLE", 1, CORE_4_INDEX, &COMPLEX_SCHEDULER_TABLE_4, 16, 1, 1 },
#endif
#if defined BCM6855
	{ "RX_FLOW_TABLE", 1, CORE_4_INDEX, &RX_FLOW_TABLE_4, 340, 1, 1 },
#endif
#if defined BCM6855
	{ "DS_TM_BBH_TX_WAKE_UP_DATA_TABLE", 1, CORE_4_INDEX, &DS_TM_BBH_TX_WAKE_UP_DATA_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "LOOPBACK_QUEUE_TABLE", 1, CORE_4_INDEX, &LOOPBACK_QUEUE_TABLE_4, 16, 1, 1 },
#endif
#if defined BCM6855
	{ "DS_BUFFER_CONG_MGT_CFG", 1, CORE_4_INDEX, &DS_BUFFER_CONG_MGT_CFG_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "TX_FLOW_TABLE", 1, CORE_4_INDEX, &TX_FLOW_TABLE_4, 212, 1, 1 },
#endif
#if defined BCM6855
	{ "MUTEX_TABLE", 1, CORE_4_INDEX, &MUTEX_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "MIRRORING_SCRATCH", 1, CORE_4_INDEX, &MIRRORING_SCRATCH_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "BUFFER_CONG_DQM_NOT_EMPTY", 1, CORE_4_INDEX, &BUFFER_CONG_DQM_NOT_EMPTY_4, 8, 1, 1 },
#endif
#if defined BCM6855
	{ "BASIC_RATE_LIMITER_TABLE_DS", 1, CORE_4_INDEX, &BASIC_RATE_LIMITER_TABLE_DS_4, 48, 1, 1 },
#endif
#if defined BCM6855
	{ "VLAN_TX_COUNTERS", 1, CORE_4_INDEX, &VLAN_TX_COUNTERS_4, 129, 1, 1 },
#endif
#if defined BCM6855
	{ "DS_TM_CPU_TX_ABS_COUNTERS", 1, CORE_4_INDEX, &DS_TM_CPU_TX_ABS_COUNTERS_4, 32, 1, 1 },
#endif
#if defined BCM6855
	{ "DEBUG_SCRATCHPAD", 1, CORE_4_INDEX, &DEBUG_SCRATCHPAD_4, 12, 1, 1 },
#endif
#if defined BCM6855
	{ "REPORT_BBH_TX_QUEUE_ID_TABLE", 1, CORE_4_INDEX, &REPORT_BBH_TX_QUEUE_ID_TABLE_4, 2, 1, 1 },
#endif
#if defined BCM6855
	{ "BUFFER_CONG_SCRATCHPAD", 1, CORE_4_INDEX, &BUFFER_CONG_SCRATCHPAD_4, 8, 1, 1 },
#endif
#if defined BCM6855
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_4_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_4, 128, 1, 1 },
#endif
#if defined BCM6855
	{ "PROCESSING1_STACK", 1, CORE_4_INDEX, &PROCESSING1_STACK_4, 320, 1, 1 },
#endif
#if defined BCM6855
	{ "CODEL_BIAS_SLOPE_TABLE", 1, CORE_4_INDEX, &CODEL_BIAS_SLOPE_TABLE_4, 11, 1, 1 },
#endif
#if defined BCM6855
	{ "GHOST_REPORTING_GLOBAL_CFG", 1, CORE_4_INDEX, &GHOST_REPORTING_GLOBAL_CFG_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_4_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_4, 16, 1, 1 },
#endif
#if defined BCM6855
	{ "VLAN_ACTION_GPE_HANDLER_PTR_TABLE", 1, CORE_4_INDEX, &VLAN_ACTION_GPE_HANDLER_PTR_TABLE_4, 17, 1, 1 },
#endif
#if defined BCM6855
	{ "CODEL_NUM_QUEUES", 1, CORE_4_INDEX, &CODEL_NUM_QUEUES_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "TASK_IDX", 1, CORE_4_INDEX, &TASK_IDX_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "SYSTEM_CONFIGURATION", 1, CORE_4_INDEX, &SYSTEM_CONFIGURATION_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "SPDTEST_NUM_OF_UDP_RX_FLOWS", 1, CORE_4_INDEX, &SPDTEST_NUM_OF_UDP_RX_FLOWS_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "BUDGET_ALLOCATION_TIMER_VALUE", 1, CORE_4_INDEX, &BUDGET_ALLOCATION_TIMER_VALUE_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "DS_TM_FLUSH_CFG_CPU_TABLE", 1, CORE_4_INDEX, &DS_TM_FLUSH_CFG_CPU_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "UPDATE_FIFO_TABLE", 1, CORE_4_INDEX, &UPDATE_FIFO_TABLE_4, 8, 1, 1 },
#endif
#if defined BCM6855
	{ "DS_TM_SCHEDULING_QUEUE_AGING_VECTOR", 1, CORE_4_INDEX, &DS_TM_SCHEDULING_QUEUE_AGING_VECTOR_4, 5, 1, 1 },
#endif
#if defined BCM6855
	{ "ENABLE_VPORT_MASK", 1, CORE_4_INDEX, &ENABLE_VPORT_MASK_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "TUNNELS_PARSING_CFG", 1, CORE_4_INDEX, &TUNNELS_PARSING_CFG_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "PROCESSING2_STACK", 1, CORE_4_INDEX, &PROCESSING2_STACK_4, 320, 1, 1 },
#endif
#if defined BCM6855
	{ "CODEL_ENABLE_TABLE", 1, CORE_4_INDEX, &CODEL_ENABLE_TABLE_4, 5, 1, 1 },
#endif
#if defined BCM6855
	{ "DS_TM_FLUSH_CFG_ENABLE_TABLE", 1, CORE_4_INDEX, &DS_TM_FLUSH_CFG_ENABLE_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "MCAST_MAX_REPLICATION_FLAG_TABLE", 1, CORE_4_INDEX, &MCAST_MAX_REPLICATION_FLAG_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "IPTV_CFG_TABLE", 1, CORE_4_INDEX, &IPTV_CFG_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "GHOST_REPORTING_QUEUE_STATUS_BIT_VECTOR_TABLE", 1, CORE_4_INDEX, &GHOST_REPORTING_QUEUE_STATUS_BIT_VECTOR_TABLE_4, 5, 1, 1 },
#endif
#if defined BCM6855
	{ "DS_TM_BB_DESTINATION_TABLE", 1, CORE_4_INDEX, &DS_TM_BB_DESTINATION_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "LOOPBACK_WAN_FLOW_TABLE", 1, CORE_4_INDEX, &LOOPBACK_WAN_FLOW_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "FORCE_DSCP", 1, CORE_4_INDEX, &FORCE_DSCP_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "IPTV_DDR_CTX_TABLE_ADDRESS", 1, CORE_4_INDEX, &IPTV_DDR_CTX_TABLE_ADDRESS_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "QUEUE_TO_REPORT_BIT_VECTOR", 1, CORE_4_INDEX, &QUEUE_TO_REPORT_BIT_VECTOR_4, 5, 1, 1 },
#endif
#if defined BCM6855
	{ "CORE_ID_TABLE", 1, CORE_4_INDEX, &CORE_ID_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "SCHEDULING_FLUSH_GLOBAL_CFG", 1, CORE_4_INDEX, &SCHEDULING_FLUSH_GLOBAL_CFG_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "TX_EXCEPTION", 1, CORE_4_INDEX, &TX_EXCEPTION_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_REDIRECT_MODE", 1, CORE_4_INDEX, &CPU_REDIRECT_MODE_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "BBH_TX_EGRESS_REPORT_COUNTER_TABLE", 1, CORE_4_INDEX, &BBH_TX_EGRESS_REPORT_COUNTER_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "DS_TM_FLUSH_CFG_FW_TABLE", 1, CORE_4_INDEX, &DS_TM_FLUSH_CFG_FW_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "DS_TM_FLUSH_CFG_CURRENT_TABLE", 1, CORE_4_INDEX, &DS_TM_FLUSH_CFG_CURRENT_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "BITS_CALC_MASKS_TABLE", 1, CORE_4_INDEX, &BITS_CALC_MASKS_TABLE_4, 4, 1, 1 },
#endif
#if defined BCM6855
	{ "BRIDGE_CFG_TABLE", 1, CORE_4_INDEX, &BRIDGE_CFG_TABLE_4, 2, 1, 1 },
#endif
#if defined BCM6855
	{ "INGRESS_FILTER_L2_REASON_TABLE", 1, CORE_4_INDEX, &INGRESS_FILTER_L2_REASON_TABLE_4, 16, 1, 1 },
#endif
#if defined BCM6855
	{ "DS_TM_FLUSH_DISPATCHER_CREDIT_TABLE", 1, CORE_4_INDEX, &DS_TM_FLUSH_DISPATCHER_CREDIT_TABLE_4, 3, 1, 1 },
#endif
#if defined BCM6855
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_4_INDEX, &RX_MIRRORING_CONFIGURATION_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_4_INDEX, &TX_MIRRORING_CONFIGURATION_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "PROCESSING3_STACK", 1, CORE_4_INDEX, &PROCESSING3_STACK_4, 320, 1, 1 },
#endif
#if defined BCM6855
	{ "DS_TM_TX_QUEUE_DROP_TABLE", 1, CORE_4_INDEX, &DS_TM_TX_QUEUE_DROP_TABLE_4, 65, 1, 1 },
#endif
#if defined BCM6855
	{ "NAT_CACHE_CFG", 1, CORE_4_INDEX, &NAT_CACHE_CFG_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_4_INDEX, &DEBUG_PRINT_CORE_LOCK_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "IPTV_CONFIGURATION_TABLE", 1, CORE_4_INDEX, &IPTV_CONFIGURATION_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "SRAM_DUMMY_STORE", 1, CORE_4_INDEX, &SRAM_DUMMY_STORE_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "RATE_LIMIT_OVERHEAD", 1, CORE_4_INDEX, &RATE_LIMIT_OVERHEAD_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "DS_TM_FIRST_QUEUE_MAPPING", 1, CORE_4_INDEX, &DS_TM_FIRST_QUEUE_MAPPING_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "BACKUP_BBH_INGRESS_COUNTERS_TABLE", 1, CORE_4_INDEX, &BACKUP_BBH_INGRESS_COUNTERS_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "RATE_LIMITER_VALID_TABLE_DS", 1, CORE_4_INDEX, &RATE_LIMITER_VALID_TABLE_DS_4, 4, 1, 1 },
#endif
#if defined BCM6855
	{ "DHD_FPM_THRESHOLDS", 1, CORE_4_INDEX, &DHD_FPM_THRESHOLDS_4, 3, 1, 1 },
#endif
#if defined BCM6855
	{ "BACKUP_BBH_EGRESS_COUNTERS_TABLE", 1, CORE_4_INDEX, &BACKUP_BBH_EGRESS_COUNTERS_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "IPTV_CLASSIFICATION_CFG_TABLE", 1, CORE_4_INDEX, &IPTV_CLASSIFICATION_CFG_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "INGRESS_FILTER_1588_CFG", 1, CORE_4_INDEX, &INGRESS_FILTER_1588_CFG_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_4_INDEX, &RX_MIRRORING_DIRECT_ENABLE_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "TX_MIRRORING_DISPATCHER_CREDIT_TABLE", 1, CORE_4_INDEX, &TX_MIRRORING_DISPATCHER_CREDIT_TABLE_4, 3, 1, 1 },
#endif
#if defined BCM6855
	{ "BBH_TX_INGRESS_COUNTER_TABLE", 1, CORE_4_INDEX, &BBH_TX_INGRESS_COUNTER_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "CPU_FEED_RING_DESCRIPTOR_TABLE", 1, CORE_4_INDEX, &CPU_FEED_RING_DESCRIPTOR_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "DEBUG_PRINT_TABLE", 1, CORE_4_INDEX, &DEBUG_PRINT_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "TCAM_GENERIC_FIELDS", 1, CORE_4_INDEX, &TCAM_GENERIC_FIELDS_4, 8, 1, 1 },
#endif
#if defined BCM6855
	{ "DS_TM_BBH_TX_EGRESS_COUNTER_TABLE", 1, CORE_4_INDEX, &DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_4, 8, 1, 1 },
#endif
#if defined BCM6855
	{ "NAT_CACHE_KEY0_MASK", 1, CORE_4_INDEX, &NAT_CACHE_KEY0_MASK_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "FPM_GLOBAL_CFG", 1, CORE_4_INDEX, &FPM_GLOBAL_CFG_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "MULTICAST_WHITELIST_CFG_TABLE", 1, CORE_4_INDEX, &MULTICAST_WHITELIST_CFG_TABLE_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "XGPON_REPORT_ZERO_SENT_TABLE", 1, CORE_4_INDEX, &XGPON_REPORT_ZERO_SENT_TABLE_4, 10, 1, 1 },
#endif
#if defined BCM6855
	{ "REPORTING_QUEUE_COUNTER_TABLE", 1, CORE_4_INDEX, &REPORTING_QUEUE_COUNTER_TABLE_4, 129, 1, 1 },
#endif
#if defined BCM6855
	{ "TCAM_TABLE_CFG_TABLE", 1, CORE_4_INDEX, &TCAM_TABLE_CFG_TABLE_4, 2, 1, 1 },
#endif
#if defined BCM6855
	{ "NATC_L2_VLAN_KEY_MASK", 1, CORE_4_INDEX, &NATC_L2_VLAN_KEY_MASK_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "INGRESS_FILTER_CFG", 1, CORE_4_INDEX, &INGRESS_FILTER_CFG_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "MULTICAST_KEY_MASK", 1, CORE_4_INDEX, &MULTICAST_KEY_MASK_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "NATC_L2_TOS_MASK", 1, CORE_4_INDEX, &NATC_L2_TOS_MASK_4, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "DS_TM_CODEL_QUEUE_TABLE", 1, CORE_4_INDEX, &DS_TM_CODEL_QUEUE_TABLE_4, 48, 1, 1 },
#endif
#if defined BCM6855
	{ "REPORTING_COUNTER_TABLE", 1, CORE_4_INDEX, &REPORTING_COUNTER_TABLE_4, 40, 1, 1 },
#endif
#if defined BCM6855
	{ "INGRESS_FILTER_PROFILE_TABLE", 1, CORE_4_INDEX, &INGRESS_FILTER_PROFILE_TABLE_4, 16, 1, 1 },
#endif
#if defined BCM6855
	{ "NATC_TBL_CFG", 1, CORE_4_INDEX, &NATC_TBL_CFG_4, 4, 1, 1 },
#endif
#if defined BCM6855
	{ "DS_TM_BBH_QUEUE_TABLE", 1, CORE_4_INDEX, &DS_TM_BBH_QUEUE_TABLE_4, 8, 1, 1 },
#endif
#if defined BCM6855
	{ "FC_FLOW_IP_ADDRESSES_TABLE", 1, CORE_4_INDEX, &FC_FLOW_IP_ADDRESSES_TABLE_4, 4, 1, 1 },
#endif
#if defined BCM6855
	{ "US_TM_PD_FIFO_TABLE", 1, CORE_5_INDEX, &US_TM_PD_FIFO_TABLE_5, 264, 1, 1 },
#endif
#if defined BCM6855
	{ "US_TM_TX_QUEUE_DROP_TABLE", 1, CORE_5_INDEX, &US_TM_TX_QUEUE_DROP_TABLE_5, 65, 1, 1 },
#endif
#if defined BCM6855
	{ "GENERAL_QUEUE_DYNAMIC_MNG_TABLE", 1, CORE_5_INDEX, &GENERAL_QUEUE_DYNAMIC_MNG_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "US_TM_CODEL_DROP_DESCRIPTOR", 1, CORE_5_INDEX, &US_TM_CODEL_DROP_DESCRIPTOR_5, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_5_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT_5, 8, 1, 1 },
#endif
#if defined BCM6855
	{ "VPORT_CFG_TABLE", 1, CORE_5_INDEX, &VPORT_CFG_TABLE_5, 16, 1, 1 },
#endif
#if defined BCM6855
	{ "DIRECT_FLOW_RX_MIRRORING_SCRATCHPAD", 1, CORE_5_INDEX, &DIRECT_FLOW_RX_MIRRORING_SCRATCHPAD_5, 136, 1, 1 },
#endif
#if defined BCM6855
	{ "US_TM_WAN_0_BBH_TX_WAKE_UP_DATA_TABLE", 1, CORE_5_INDEX, &US_TM_WAN_0_BBH_TX_WAKE_UP_DATA_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_5_INDEX, &FPM_POOL_NUMBER_MAPPING_TABLE_5, 16, 1, 1 },
#endif
#if defined BCM6855
	{ "DIRECT_FLOW_CNTR_TABLE", 1, CORE_5_INDEX, &DIRECT_FLOW_CNTR_TABLE_5, 32, 1, 1 },
#endif
#if defined BCM6855
	{ "DIRECT_FLOW_STACK", 1, CORE_5_INDEX, &DIRECT_FLOW_STACK_5, 64, 1, 1 },
#endif
#if defined BCM6855
	{ "COMPLEX_SCHEDULER_TABLE", 1, CORE_5_INDEX, &COMPLEX_SCHEDULER_TABLE_5, 16, 1, 1 },
#endif
#if defined BCM6855
	{ "US_TM_SCHEDULING_QUEUE_TABLE", 1, CORE_5_INDEX, &US_TM_SCHEDULING_QUEUE_TABLE_5, 132, 1, 1 },
#endif
#if defined BCM6855
	{ "BUFFER_CONG_DQM_NOT_EMPTY", 1, CORE_5_INDEX, &BUFFER_CONG_DQM_NOT_EMPTY_5, 8, 1, 1 },
#endif
#if defined BCM6855
	{ "US_BUFFER_CONG_MGT_CFG", 1, CORE_5_INDEX, &US_BUFFER_CONG_MGT_CFG_5, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "US_TM_TM_FLOW_CNTR_TABLE", 1, CORE_5_INDEX, &US_TM_TM_FLOW_CNTR_TABLE_5, 128, 1, 1 },
#endif
#if defined BCM6855
	{ "NATC_TBL_CFG", 1, CORE_5_INDEX, &NATC_TBL_CFG_5, 4, 1, 1 },
#endif
#if defined BCM6855
	{ "REGISTERS_BUFFER", 1, CORE_5_INDEX, &REGISTERS_BUFFER_5, 32, 1, 1 },
#endif
#if defined BCM6855
	{ "DIRECT_FLOW_EPON_CONTROL_SCRATCH", 1, CORE_5_INDEX, &DIRECT_FLOW_EPON_CONTROL_SCRATCH_5, 22, 1, 1 },
#endif
#if defined BCM6855
	{ "BUDGET_ALLOCATION_TIMER_VALUE", 1, CORE_5_INDEX, &BUDGET_ALLOCATION_TIMER_VALUE_5, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "BBH_TX_EPON_WAKE_UP_DATA_TABLE", 1, CORE_5_INDEX, &BBH_TX_EPON_WAKE_UP_DATA_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "US_TM_BBH_QUEUE_TABLE", 1, CORE_5_INDEX, &US_TM_BBH_QUEUE_TABLE_5, 40, 1, 1 },
#endif
#if defined BCM6855
	{ "US_TM_CPU_TX_ABS_COUNTERS", 1, CORE_5_INDEX, &US_TM_CPU_TX_ABS_COUNTERS_5, 32, 1, 1 },
#endif
#if defined BCM6855
	{ "US_TM_SCHEDULING_QUEUE_AGING_VECTOR", 1, CORE_5_INDEX, &US_TM_SCHEDULING_QUEUE_AGING_VECTOR_5, 5, 1, 1 },
#endif
#if defined BCM6855
	{ "MUTEX_TABLE", 1, CORE_5_INDEX, &MUTEX_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "MIRRORING_SCRATCH", 1, CORE_5_INDEX, &MIRRORING_SCRATCH_5, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "BUFFER_CONG_SCRATCHPAD", 1, CORE_5_INDEX, &BUFFER_CONG_SCRATCHPAD_5, 8, 1, 1 },
#endif
#if defined BCM6855
	{ "CODEL_BIAS_SLOPE_TABLE", 1, CORE_5_INDEX, &CODEL_BIAS_SLOPE_TABLE_5, 11, 1, 1 },
#endif
#if defined BCM6855
	{ "TASK_IDX", 1, CORE_5_INDEX, &TASK_IDX_5, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "US_TM_FLUSH_CFG_CPU_TABLE", 1, CORE_5_INDEX, &US_TM_FLUSH_CFG_CPU_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "US_TM_WAN_0_BBH_TX_EGRESS_COUNTER_TABLE", 1, CORE_5_INDEX, &US_TM_WAN_0_BBH_TX_EGRESS_COUNTER_TABLE_5, 40, 1, 1 },
#endif
#if defined BCM6855
	{ "US_TM_FLUSH_CFG_ENABLE_TABLE", 1, CORE_5_INDEX, &US_TM_FLUSH_CFG_ENABLE_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "DIRECT_FLOW_PAUSE_QUANTA", 1, CORE_5_INDEX, &DIRECT_FLOW_PAUSE_QUANTA_5, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "US_TM_BBH_TX_WAN_0_FIFO_BYTES_USED", 1, CORE_5_INDEX, &US_TM_BBH_TX_WAN_0_FIFO_BYTES_USED_5, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "US_TM_FLUSH_CFG_FW_TABLE", 1, CORE_5_INDEX, &US_TM_FLUSH_CFG_FW_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "BASIC_RATE_LIMITER_TABLE_US", 1, CORE_5_INDEX, &BASIC_RATE_LIMITER_TABLE_US_5, 128, 1, 1 },
#endif
#if defined BCM6855
	{ "US_TM_CODEL_QUEUE_TABLE", 1, CORE_5_INDEX, &US_TM_CODEL_QUEUE_TABLE_5, 132, 1, 1 },
#endif
#if defined BCM6855
	{ "DEBUG_SCRATCHPAD", 1, CORE_5_INDEX, &DEBUG_SCRATCHPAD_5, 12, 1, 1 },
#endif
#if defined BCM6855
	{ "BBH_TX_EPON_INGRESS_COUNTER_TABLE", 1, CORE_5_INDEX, &BBH_TX_EPON_INGRESS_COUNTER_TABLE_5, 40, 1, 1 },
#endif
#if defined BCM6855
	{ "US_TM_WAN_0_BB_DESTINATION_TABLE", 1, CORE_5_INDEX, &US_TM_WAN_0_BB_DESTINATION_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "QEMU_SYNC_MEM", 1, CORE_5_INDEX, &QEMU_SYNC_MEM_5, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "DIRECT_FLOW_WAN_VIQ_EXCLUSIVE", 1, CORE_5_INDEX, &DIRECT_FLOW_WAN_VIQ_EXCLUSIVE_5, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "CORE_ID_TABLE", 1, CORE_5_INDEX, &CORE_ID_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "US_TM_TX_PAUSE_NACK", 1, CORE_5_INDEX, &US_TM_TX_PAUSE_NACK_5, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "BBH_TX_US_WAN_0_FIFO_BYTES_THRESHOLD", 1, CORE_5_INDEX, &BBH_TX_US_WAN_0_FIFO_BYTES_THRESHOLD_5, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "US_TM_FLUSH_CFG_CURRENT_TABLE", 1, CORE_5_INDEX, &US_TM_FLUSH_CFG_CURRENT_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "BBH_TX_EPON_EGRESS_COUNTER_TABLE", 1, CORE_5_INDEX, &BBH_TX_EPON_EGRESS_COUNTER_TABLE_5, 32, 1, 1 },
#endif
#if defined BCM6855
	{ "DIRECT_FLOW_PD_TABLE", 1, CORE_5_INDEX, &DIRECT_FLOW_PD_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "OVERALL_RATE_LIMITER_TABLE", 1, CORE_5_INDEX, &OVERALL_RATE_LIMITER_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "UPDATE_FIFO_TABLE", 1, CORE_5_INDEX, &UPDATE_FIFO_TABLE_5, 8, 1, 1 },
#endif
#if defined BCM6855
	{ "RATE_LIMITER_VALID_TABLE_US", 1, CORE_5_INDEX, &RATE_LIMITER_VALID_TABLE_US_5, 4, 1, 1 },
#endif
#if defined BCM6855
	{ "US_TM_FLUSH_DISPATCHER_CREDIT_TABLE", 1, CORE_5_INDEX, &US_TM_FLUSH_DISPATCHER_CREDIT_TABLE_5, 3, 1, 1 },
#endif
#if defined BCM6855
	{ "SCHEDULING_FLUSH_GLOBAL_CFG", 1, CORE_5_INDEX, &SCHEDULING_FLUSH_GLOBAL_CFG_5, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "TX_EXCEPTION", 1, CORE_5_INDEX, &TX_EXCEPTION_5, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "RX_MIRRORING_CONFIGURATION", 1, CORE_5_INDEX, &RX_MIRRORING_CONFIGURATION_5, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "EPON_UPDATE_FIFO_TABLE", 1, CORE_5_INDEX, &EPON_UPDATE_FIFO_TABLE_5, 8, 1, 1 },
#endif
#if defined BCM6855
	{ "US_TM_TX_OCTETS_COUNTERS_TABLE", 1, CORE_5_INDEX, &US_TM_TX_OCTETS_COUNTERS_TABLE_5, 8, 1, 1 },
#endif
#if defined BCM6855
	{ "DISPATCHER_CREDIT_TABLE", 1, CORE_5_INDEX, &DISPATCHER_CREDIT_TABLE_5, 3, 1, 1 },
#endif
#if defined BCM6855
	{ "TX_MIRRORING_CONFIGURATION", 1, CORE_5_INDEX, &TX_MIRRORING_CONFIGURATION_5, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "DEBUG_PRINT_CORE_LOCK", 1, CORE_5_INDEX, &DEBUG_PRINT_CORE_LOCK_5, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "SRAM_DUMMY_STORE", 1, CORE_5_INDEX, &SRAM_DUMMY_STORE_5, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "DIRECT_FLOW_PAUSE_DEBUG", 1, CORE_5_INDEX, &DIRECT_FLOW_PAUSE_DEBUG_5, 3, 1, 1 },
#endif
#if defined BCM6855
	{ "RATE_LIMIT_OVERHEAD", 1, CORE_5_INDEX, &RATE_LIMIT_OVERHEAD_5, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "MAC_TYPE", 1, CORE_5_INDEX, &MAC_TYPE_5, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "US_TM_FIRST_QUEUE_MAPPING", 1, CORE_5_INDEX, &US_TM_FIRST_QUEUE_MAPPING_5, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "WAN_0_BBH_TX_FIFO_SIZE", 1, CORE_5_INDEX, &WAN_0_BBH_TX_FIFO_SIZE_5, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "TX_MIRRORING_DISPATCHER_CREDIT_TABLE", 1, CORE_5_INDEX, &TX_MIRRORING_DISPATCHER_CREDIT_TABLE_5, 3, 1, 1 },
#endif
#if defined BCM6855
	{ "RX_MIRRORING_DIRECT_ENABLE", 1, CORE_5_INDEX, &RX_MIRRORING_DIRECT_ENABLE_5, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "DEBUG_PRINT_TABLE", 1, CORE_5_INDEX, &DEBUG_PRINT_TABLE_5, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "FPM_GLOBAL_CFG", 1, CORE_5_INDEX, &FPM_GLOBAL_CFG_5, 1, 1, 1 },
#endif
#if defined BCM6855
	{ "BASIC_SCHEDULER_TABLE_US", 1, CORE_5_INDEX, &BASIC_SCHEDULER_TABLE_US_5, 33, 1, 1 },
#endif
#if defined BCM6855
	{ "VLAN_TX_COUNTERS", 1, CORE_5_INDEX, &VLAN_TX_COUNTERS_5, 129, 1, 1 },
#endif
#if defined BCM6855
	{ "RUNNER_PROFILING_TRACE_BUFFER", 1, CORE_5_INDEX, &RUNNER_PROFILING_TRACE_BUFFER_5, 128, 1, 1 },
#endif
};

TABLE_STACK_STRUCT RUNNER_STACK_TABLES[NUMBER_OF_STACK_TABLES] =
{
    { "SPDSVC_ANALYZER_STACK", CORE_0_INDEX, 0x300, 256},
    { "DHD_TX_COMPLETE_0_STACK", CORE_0_INDEX, 0x780, 128},
    { "PROCESSING0_STACK", CORE_0_INDEX, 0x800, 320},
    { "DHD_RX_COMPLETE_0_STACK", CORE_0_INDEX, 0x940, 64},
    { "DHD_TX_COMPLETE_1_STACK", CORE_0_INDEX, 0x980, 128},
    { "PROCESSING1_STACK", CORE_0_INDEX, 0xa00, 320},
    { "DHD_RX_COMPLETE_1_STACK", CORE_0_INDEX, 0xb40, 64},
    { "DHD_TX_COMPLETE_2_STACK", CORE_0_INDEX, 0xb80, 128},
    { "PROCESSING2_STACK", CORE_0_INDEX, 0xc00, 320},
    { "DHD_RX_COMPLETE_2_STACK", CORE_0_INDEX, 0xd40, 64},
    { "PROCESSING3_STACK", CORE_0_INDEX, 0xe00, 320},
    { "PROCESSING4_STACK", CORE_0_INDEX, 0x2000, 320},
    { "PROCESSING5_STACK", CORE_0_INDEX, 0x2200, 320},
    { "PROCESSING6_STACK", CORE_0_INDEX, 0x2400, 320},
    { "PROCESSING7_STACK", CORE_0_INDEX, 0x2600, 320},
    { "PROCESSING0_STACK", CORE_1_INDEX, 0xe00, 320},
    { "CPU_RX_STACK", CORE_1_INDEX, 0x23a0, 32},
    { "PROCESSING1_STACK", CORE_1_INDEX, 0x2600, 320},
    { "PROCESSING2_STACK", CORE_1_INDEX, 0x2800, 320},
    { "CPU_RX_COPY_STACK", CORE_1_INDEX, 0x2980, 128},
    { "PROCESSING3_STACK", CORE_1_INDEX, 0x2a00, 320},
    { "PROCESSING4_STACK", CORE_1_INDEX, 0x2c00, 320},
    { "PROCESSING5_STACK", CORE_1_INDEX, 0x2e00, 320},
    { "SPDSVC_GEN_STACK", CORE_1_INDEX, 0x2f80, 128},
    { "PROCESSING6_STACK", CORE_1_INDEX, 0x3200, 320},
    { "GENERAL_TIMER_STACK", CORE_1_INDEX, 0x33e0, 32},
    { "PROCESSING7_STACK", CORE_1_INDEX, 0x3400, 320},
    { "CPU_RECYCLE_STACK", CORE_1_INDEX, 0x35c0, 32},
    { "COMMON_REPROCESSING_STACK", CORE_1_INDEX, 0x35e0, 32},
    { "PROCESSING0_STACK", CORE_2_INDEX, 0x600, 320},
    { "CPU_TX_0_STACK", CORE_2_INDEX, 0x780, 128},
    { "PROCESSING1_STACK", CORE_2_INDEX, 0x2600, 320},
    { "CPU_TX_1_STACK", CORE_2_INDEX, 0x2780, 128},
    { "PROCESSING2_STACK", CORE_2_INDEX, 0x2800, 320},
    { "PROCESSING3_STACK", CORE_2_INDEX, 0x2a00, 320},
    { "GENERAL_TIMER_STACK", CORE_2_INDEX, 0x2be0, 32},
    { "PROCESSING4_STACK", CORE_2_INDEX, 0x2c00, 320},
    { "CPU_RECYCLE_STACK", CORE_2_INDEX, 0x2dc0, 32},
    { "PROCESSING5_STACK", CORE_2_INDEX, 0x2e00, 320},
    { "PROCESSING6_STACK", CORE_2_INDEX, 0x3000, 320},
    { "PROCESSING7_STACK", CORE_2_INDEX, 0x3200, 320},
    { "DHD_TX_POST_0_STACK", CORE_3_INDEX, 0xb00, 128},
    { "DHD_TX_POST_1_STACK", CORE_3_INDEX, 0xb80, 128},
    { "DHD_TIMER_STACK", CORE_3_INDEX, 0xfe0, 32},
    { "PROCESSING0_STACK", CORE_3_INDEX, 0x2000, 320},
    { "DHD_TX_POST_2_STACK", CORE_3_INDEX, 0x2180, 128},
    { "PROCESSING1_STACK", CORE_3_INDEX, 0x2200, 320},
    { "DHD_TX_POST_UPDATE_FIFO_STACK", CORE_3_INDEX, 0x2360, 32},
    { "PROCESSING2_STACK", CORE_3_INDEX, 0x2400, 320},
    { "PROCESSING3_STACK", CORE_3_INDEX, 0x2600, 320},
    { "PROCESSING4_STACK", CORE_3_INDEX, 0x2800, 320},
    { "PROCESSING5_STACK", CORE_3_INDEX, 0x2a00, 320},
    { "PROCESSING6_STACK", CORE_3_INDEX, 0x2c00, 320},
    { "PROCESSING7_STACK", CORE_3_INDEX, 0x2e00, 320},
    { "REPORTING_STACK", CORE_4_INDEX, 0xce0, 32},
    { "PROCESSING0_STACK", CORE_4_INDEX, 0xe00, 320},
    { "PROCESSING1_STACK", CORE_4_INDEX, 0x2a00, 320},
    { "PROCESSING2_STACK", CORE_4_INDEX, 0x2c00, 320},
    { "PROCESSING3_STACK", CORE_4_INDEX, 0x2e00, 320},
    { "DIRECT_FLOW_STACK", CORE_5_INDEX, 0x13c0, 64}
};
