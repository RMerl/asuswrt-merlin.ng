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


#ifndef __RDD_RUNNER_REG_DUMP_H__
#define __RDD_RUNNER_REG_DUMP_H__

#include "bdmf_shell.h"

typedef struct
{
#ifndef USE_BDMF_SHELL
	void  (*callback)(unsigned char *xi_value, int rw, int unionreq);
#else
	void  (*callback)(bdmf_session_handle session, unsigned char *value);
#endif
	unsigned int starts;
} ENTRY_STRUCT;

typedef struct
{
	int length; /* length of each entry in bytes */
	ENTRY_STRUCT entries[];
} DUMP_RUNNERREG_STRUCT;

typedef struct
{
	char * table_name;
	int tbldmp; /* boolean */
	int segment;
	DUMP_RUNNERREG_STRUCT * entries;
    int size_rows; /* Number of rows in table */
    int size_rows_d2;/* If table's entries are tables by themselfves, number of rows in each entry table (dimention 2)*/
    int size_rows_d3; /* Like above, dimention 3 */
} TABLE_STRUCT;

#ifndef STT_PRINTF
	#if defined LINUX_KERNEL
		#define STT_PRINTF printk
	#else
		#define STT_PRINTF printf
	#endif
#endif


void dump_RDD_IH_BUFFER(bdmf_session_handle session, unsigned char *p);
void dump_RDD_PACKET_DESCRIPTOR(bdmf_session_handle session, unsigned char *p);
void dump_RDD_BBH_TX_DESCRIPTOR(bdmf_session_handle session, unsigned char *p);
#if defined OREN
void dump_RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p);
#endif

void dump_RDD_INGRESS_FILTERS_LOOKUP_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_PBITS_TO_PBITS_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_DS_WAN_FLOW_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_IPTV_COUNTER_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_ETH_TX_LOCAL_REGISTERS_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_QUEUE_PROFILE(bdmf_session_handle session, unsigned char *p);
void dump_RDD_PBITS_PARAMETER_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_GSO_BUFFER_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_GSO_PSEUDO_HEADER_BUFFER_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_CPU_PARAMETERS_BLOCK_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_TPID_OVERWRITE_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_CPU_REASON_TO_METER_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_SIXTEEN_BYTES(bdmf_session_handle session, unsigned char *p);
void dump_RDD_ETH_TX_RS_QUEUE_DESCRIPTOR(bdmf_session_handle session, unsigned char *p);
void dump_RDD_LAYER4_FILTERS_LOOKUP_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_IPTV_SSID_EXTENSION_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_PBITS_PRIMITIVE_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_TIMER_TASK_DESCRIPTOR_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_SBPM_REPLY_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_VLAN_PARAMETER_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_CONNECTION_CONTEXT_BUFFER_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_WLAN_MCAST_FWD_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_CPU_TX_DESCRIPTOR(bdmf_session_handle session, unsigned char *p);
void dump_RDD_CPU_TX_DESCRIPTOR_CORE(bdmf_session_handle session, unsigned char *p);
void dump_RDD_CPU_TX_DESCRIPTOR_BPM(bdmf_session_handle session, unsigned char *p);
void dump_RDD_CPU_TX_DESCRIPTOR_ABS(bdmf_session_handle session, unsigned char *p);
void dump_RDD_CPU_TX_DESCRIPTOR_DS_FAST(bdmf_session_handle session, unsigned char *p);
void dump_RDD_CPU_TX_DESCRIPTOR_US_FAST(bdmf_session_handle session, unsigned char *p);
void dump_RDD_CPU_TX_DESCRIPTOR_DS_PICO(bdmf_session_handle session, unsigned char *p);
void dump_RDD_CPU_TX_DESCRIPTOR_DS_PICO_WIFI(bdmf_session_handle session, unsigned char *p);
void dump_RDD_CPU_TX_MESSAGE_DESCRIPTOR(bdmf_session_handle session, unsigned char *p);
void dump_RDD_RATE_LIMITER_REMAINDER_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_FOUR_BYTES(bdmf_session_handle session, unsigned char *p);
void dump_RDD_TWO_BYTES(bdmf_session_handle session, unsigned char *p);
void dump_RDD_RATE_LIMITER_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_ETH_TX_QUEUE_DESCRIPTOR(bdmf_session_handle session, unsigned char *p);
void dump_RDD_SERVICE_QUEUES_RATE_LIMITER_DESCRIPTOR(bdmf_session_handle session, unsigned char *p);
void dump_RDD_VLAN_COMMAND_ENRTY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_VID_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_POLICER_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_CPU_TX_BBH_DESCRIPTORS_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_ETH_TX_QUEUE_POINTERS_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_IPTV_SSID_EXTENSION_TABLE_CAM(bdmf_session_handle session, unsigned char *p);
void dump_RDD_EIGHT_BYTES(bdmf_session_handle session, unsigned char *p);
void dump_RDD_INGRESS_FILTERS_PARAMETER_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_WLAN_MCAST_CONTROL_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_BPM_DDR_OPTIMIZED_BUFFERS_WITHOUT_HEADROOM_BASE(bdmf_session_handle session, unsigned char *p);
void dump_RDD_FIREWALL_CONFIGURATION_REGISTER_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_INGRESS_CLASSIFICATION_GENERIC_RULE_CFG_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_QUEUE_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_TIMER_SCHEDULER_PRIMITIVE_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_FORWARDING_MATRIX_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_FAST_RUNNER_GLOBAL_REGISTERS_INIT_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_DHD_TX_COMPLETE_DESCRIPTOR(bdmf_session_handle session, unsigned char *p);
void dump_RDD_DHD_TX_COMPLETE_DESCRIPTOR_CWI32(bdmf_session_handle session, unsigned char *p);
void dump_RDD_BRIDGE_CONFIGURATION_REGISTER(bdmf_session_handle session, unsigned char *p);
void dump_RDD_CPU_RX_METER_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_PROFILING_BUFFER_PICO_RUNNER(bdmf_session_handle session, unsigned char *p);
void dump_RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_VLAN_OPTIMIZATION_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_PCI_MULTICAST_SCRATCHPAD(bdmf_session_handle session, unsigned char *p);
void dump_RDD_RUNNER_FLOW_HEADER_BUFFER(bdmf_session_handle session, unsigned char *p);
void dump_RDD_CPU_TX_DHD_DESCRIPTOR(bdmf_session_handle session, unsigned char *p);
void dump_RDD_CPU_TX_DHD_MESSAGE_DESCRIPTOR(bdmf_session_handle session, unsigned char *p);
void dump_RDD_ONE_BYTE(bdmf_session_handle session, unsigned char *p);
void dump_RDD_DHD_COMPLETE_RING_DESCRIPTOR(bdmf_session_handle session, unsigned char *p);
void dump_RDD_HASH_BUFFER(bdmf_session_handle session, unsigned char *p);
#if defined OREN
void dump_RDD_ETH_TX_MAC_DESCRIPTOR(bdmf_session_handle session, unsigned char *p);
#endif

void dump_RDD_DSCP_TO_PBITS_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_PACKET_SRAM_TO_DDR_COPY_BUFFER(bdmf_session_handle session, unsigned char *p);
void dump_RDD_EMAC_SKB_ENQUEUED_INDEXES_FIFO_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_VLAN_PRIMITIVE_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_PICO_RUNNER_GLOBAL_REGISTERS_INIT_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_INGRESS_QUEUE_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_RUNNER_FLOW_HEADER_DESCRIPTOR(bdmf_session_handle session, unsigned char *p);
void dump_RDD_GSO_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_BPM_EXTRA_DDR_BUFFERS_BASE(bdmf_session_handle session, unsigned char *p);
void dump_RDD_IPTV_DMA_LKP_KEY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_DHD_COMPLETE_RING_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_CPU_TX_MESSAGE_DATA_BUFFER_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_BBH_RX_DESCRIPTOR(bdmf_session_handle session, unsigned char *p);
void dump_RDD_VLAN_ACTION_BUFFER(bdmf_session_handle session, unsigned char *p);
void dump_RDD_DHD_TX_POST_DESCRIPTOR(bdmf_session_handle session, unsigned char *p);
void dump_RDD_DHD_TX_POST_DESCRIPTOR_CWI32(bdmf_session_handle session, unsigned char *p);
void dump_RDD_IPTV_SSM_CONTEXT_TABLE_PTR(bdmf_session_handle session, unsigned char *p);
void dump_RDD_WLAN_MCAST_SSID_STATS_STATE_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_TIMER_CONTROL_DESCRIPTOR(bdmf_session_handle session, unsigned char *p);
void dump_RDD_FREE_SKB_INDEXES_FIFO_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_IPV6_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_PCI_TX_FIFO_DESCRIPTOR_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_PCI_TX_QUEUES_VECTOR(bdmf_session_handle session, unsigned char *p);
void dump_RDD_PCI_TX_FIFO_FULL_VECTOR_ENTRY(bdmf_session_handle session, unsigned char *p);
#if defined OREN
void dump_RDD_ETH_TX_EMACS_STATUS_ENTRY(bdmf_session_handle session, unsigned char *p);
#endif

void dump_RDD_MULTICAST_VECTOR_TO_PORT_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_RUNNER_FLOW_IH_RESPONSE(bdmf_session_handle session, unsigned char *p);
void dump_RDD_DEBUG_BUFFER_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_MULTICAST_DUMMY_VLAN_INDEXES_TABLE(bdmf_session_handle session, unsigned char *p);
void dump_RDD_GSO_DESC_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_IPTV_PTR_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_IPTV_CONTEXT_PTR_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_BPM_DDR_OPTIMIZED_BUFFERS_BASE(bdmf_session_handle session, unsigned char *p);
void dump_RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_BPM_DDR_BUFFERS_BASE(bdmf_session_handle session, unsigned char *p);
void dump_RDD_MULTICAST_HEADER_BUFFER(bdmf_session_handle session, unsigned char *p);
void dump_RDD_IPTV_COUNTERS_BUFFER(bdmf_session_handle session, unsigned char *p);
void dump_RDD_LAYER4_FILTERS_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_CONNECTION_TABLE_CONFIG(bdmf_session_handle session, unsigned char *p);
void dump_RDD_CONTEXT_TABLE_CONFIG(bdmf_session_handle session, unsigned char *p);
void dump_RDD_IPTV_DMA_RW_BUFFER(bdmf_session_handle session, unsigned char *p);
void dump_RDD_ETH_TX_QUEUE_DUMMY_DESCRIPTOR(bdmf_session_handle session, unsigned char *p);
void dump_RDD_DHD_TX_POST_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_SC_BUFFER(bdmf_session_handle session, unsigned char *p);
#if defined OREN
void dump_RDD_PARALLEL_PROCESSING_ENTRY(bdmf_session_handle session, unsigned char *p);
#endif

#if defined OREN
void dump_RDD_PARALLEL_PROCESSING_TASK_REORDER_ENTRY(bdmf_session_handle session, unsigned char *p);
#endif

void dump_RDD_FIREWALL_IPV6_R16_BUFFER_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_CPU_TX_PICO_INGRESS_QUEUE_PTR(bdmf_session_handle session, unsigned char *p);
void dump_RDD_FREE_PACKET_DESCRIPTORS_POOL_THRESHOLD(bdmf_session_handle session, unsigned char *p);
void dump_RDD_DUMMY_STORE_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_HASH_BASED_FORWARDING_PORT_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_CPU_RX_PICO_INGRESS_QUEUE_PTR(bdmf_session_handle session, unsigned char *p);
void dump_RDD_BPM_DDR_BUFFER_HEADROOM_SIZE(bdmf_session_handle session, unsigned char *p);
void dump_RDD_BPM_DDR_BUFFER_HEADROOM_SIZE_2_BYTE_RESOLUTION(bdmf_session_handle session, unsigned char *p);
void dump_RDD_CPU_RX_FAST_INGRESS_QUEUE_PTR(bdmf_session_handle session, unsigned char *p);
void dump_RDD_MICROCODE_VERSION_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_RATE_LIMITER_COUNTER_BUFFER_ENTRY(bdmf_session_handle session, unsigned char *p);
#if defined OREN
void dump_RDD_PARALLEL_PROCESSING_IH_BUFFER_PTR(bdmf_session_handle session, unsigned char *p);
#endif

void dump_RDD_RUNNER_CONGESTION_STATE_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_FIREWALL_RULE_MAP_ENTRY_BUFFER(bdmf_session_handle session, unsigned char *p);
#if defined OREN
void dump_RDD_ETH_TX_INTER_LAN_SCHEDULING_OFFSET_ENTRY(bdmf_session_handle session, unsigned char *p);
#endif

#if defined OREN
void dump_RDD_PARALLEL_PROCESSING_SLAVE_VECTOR(bdmf_session_handle session, unsigned char *p);
#endif

void dump_RDD_DOWNSTREAM_DMA_PIPE_BUFFER(bdmf_session_handle session, unsigned char *p);
void dump_RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_WAN_CHANNEL_8_39_DESCRIPTOR(bdmf_session_handle session, unsigned char *p);
void dump_RDD_US_QUEUE_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_US_WAN_FLOW_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_WAN_CHANNEL_0_7_DESCRIPTOR(bdmf_session_handle session, unsigned char *p);
void dump_RDD_SMART_CARD_DESCRIPTOR_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_BUDGET_ALLOCATOR_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_GPON_ABSOLUTE_TX_COUNTER(bdmf_session_handle session, unsigned char *p);
void dump_RDD_DHD_RX_POST_DESCRIPTOR(bdmf_session_handle session, unsigned char *p);
void dump_RDD_DHD_RX_COMPLETE_DESCRIPTOR(bdmf_session_handle session, unsigned char *p);
void dump_RDD_DHD_RX_POST_DESCRIPTOR_CWI32(bdmf_session_handle session, unsigned char *p);
void dump_RDD_DHD_RX_COMPLETE_DESCRIPTOR_CWI32(bdmf_session_handle session, unsigned char *p);
void dump_RDD_ETH_RX_DESCRIPTORS(bdmf_session_handle session, unsigned char *p);
void dump_RDD_INGRESS_RATE_LIMITER_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_SMART_CARD_ERROR_COUNTERS_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_SPEED_SERVICE_PARAMETERS(bdmf_session_handle session, unsigned char *p);
void dump_RDD_LAN_INGRESS_FIFO_DESCRIPTOR_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_US_OVERALL_RATE_LIMITER_WAN_CHANNEL_PTR_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_US_INGRESS_CLASSIFICATION_DEFAULT_FLOWS_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_IPV6_LOCAL_IP(bdmf_session_handle session, unsigned char *p);
void dump_RDD_BROADCOM_SWITCH_PORT_MAPPING(bdmf_session_handle session, unsigned char *p);
void dump_RDD_LOCAL_SWITCHING_MULTICAST_LAN_ENQUEUE_INGRESS_QUEUE_PTR(bdmf_session_handle session, unsigned char *p);
void dump_RDD_VLAN_SWITCHING_ISOLATION_CONFIG_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_LOCAL_SWITCHING_LAN_ENQUEUE_INGRESS_QUEUE_PTR(bdmf_session_handle session, unsigned char *p);
void dump_RDD_BBH_TX_WAN_CHANNEL_INDEX(bdmf_session_handle session, unsigned char *p);
void dump_RDD_MAC_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_IPTV_L2_LOOKUP_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_IPTV_L3_LOOKUP_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_MAC_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p);
#if defined OREN
void dump_RDD_IPTV_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p);
#endif

void dump_RDD_DDR_QUEUE_DESCRIPTOR(bdmf_session_handle session, unsigned char *p);
void dump_RDD_INTERRUPT_COALESCING_CONFIG(bdmf_session_handle session, unsigned char *p);
void dump_RDD_DHD_TX_POST_FLOW_RING_MGMT_DESCRIPTOR(bdmf_session_handle session, unsigned char *p);
void dump_RDD_DHD_RADIO_INSTANCE_COMMON_A_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_SERVICE_QUEUES_CFG_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_INTERRUPT_COALESCING_TIMER_CONFIG(bdmf_session_handle session, unsigned char *p);
void dump_RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_WLAN_MCAST_DHD_STATION_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_PBITS_TO_QOS_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_BPM_CONGESTION_CONTROL_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_FREE_SKB_INDEXES_FIFO_TAIL(bdmf_session_handle session, unsigned char *p);
void dump_RDD_MAC_EXTENSION_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_DHD_FLOW_RING_CACHE_LKP_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_MAC_UNKNOWN_DA_FORWARDING_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_BPM_REPLY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_CPU_REASON_TO_CPU_RX_QUEUE_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_GPON_SKB_ENQUEUED_INDEXES_FIFO_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_WLAN_MCAST_SSID_MAC_ADDRESS_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_DSCP_TO_PBITS_DEI_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_RUNNER_SCRATCHPAD(bdmf_session_handle session, unsigned char *p);
void dump_RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_PM_COUNTERS(bdmf_session_handle session, unsigned char *p);
void dump_RDD_DDR_QUEUE_ADDRESS_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_LAN_VID_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_RING_DESCRIPTOR(bdmf_session_handle session, unsigned char *p);
void dump_RDD_PM_COUNTERS_BUFFER(bdmf_session_handle session, unsigned char *p);
void dump_RDD_WLAN_MCAST_SSID_STATS_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_CPU_RX_DESCRIPTOR(bdmf_session_handle session, unsigned char *p);
void dump_RDD_CONNECTION_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_TUNNEL_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_WIFI_SSID_FORWARDING_MATRIX_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_DUAL_STACK_LITE_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_DS_INGRESS_CLASSIFICATION_IH_LOOKUP_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_US_INGRESS_CLASSIFICATION_IH_LOOKUP_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_DS_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_US_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_INGRESS_CLASSIFICATION_SHORT_LOOKUP_ENTRY(bdmf_session_handle session, unsigned char *p);
#if defined OREN
void dump_RDD_VLAN_COMMAND_INDEX_ENTRY(bdmf_session_handle session, unsigned char *p);
#endif

void dump_RDD_IP_SYNC_1588_DESCRIPTOR_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_DUMMY_RATE_CONTROLLER_DESCRIPTOR(bdmf_session_handle session, unsigned char *p);
void dump_RDD_WAN_TX_QUEUE_DESCRIPTOR(bdmf_session_handle session, unsigned char *p);
void dump_RDD_DHD_FLOW_RING_CACHE_CTX_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_TUNNEL_DYNAMIC_FIELDS_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_US_RATE_CONTROLLER_DESCRIPTOR(bdmf_session_handle session, unsigned char *p);
void dump_RDD_SRC_MAC_ANTI_SPOOFING_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_DUMMY_WAN_TX_QUEUE_DESCRIPTOR(bdmf_session_handle session, unsigned char *p);
void dump_RDD_SPEED_SERVICE_RX_TIMESTAMPS(bdmf_session_handle session, unsigned char *p);
void dump_RDD_BRIDGE_PORT_TO_BROADCOM_SWITCH_PORT_MAPPING_TABLE_PTR(bdmf_session_handle session, unsigned char *p);
void dump_RDD_WAN_PHYSICAL_PORT(bdmf_session_handle session, unsigned char *p);
void dump_RDD_RATE_CONTROLLER_EXPONENT_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_LAN_INGRESS_FIFO_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_MULTICAST_ACTIVE_PORTS_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_FLOW_CACHE_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_FIREWALL_RULE_ENTRY(bdmf_session_handle session, unsigned char *p);
#if defined OREN
void dump_RDD_IPTV_DDR_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p);
#endif

void dump_RDD_IPTV_L2_DDR_LOOKUP_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_IPTV_L3_DDR_LOOKUP_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_FIREWALL_RULES_MAP_ENTRY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_WLAN_MCAST_DHD_LIST_ENTRY_ARRAY(bdmf_session_handle session, unsigned char *p);
void dump_RDD_WLAN_MCAST_DHD_LIST_ENTRY(bdmf_session_handle session, unsigned char *p);
#if defined G9991
void dump_RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p);
#endif

#if defined G9991
void dump_RDD_VLAN_COMMAND_INDEX_ENTRY(bdmf_session_handle session, unsigned char *p);
#endif

#if defined G9991
void dump_RDD_ETH_TX_EMACS_STATUS_ENTRY(bdmf_session_handle session, unsigned char *p);
#endif

#if defined G9991
void dump_RDD_ETH_TX_MAC_DESCRIPTOR(bdmf_session_handle session, unsigned char *p);
#endif

#if defined G9991
void dump_RDD_ETH_TX_INTER_LAN_SCHEDULING_OFFSET_ENTRY(bdmf_session_handle session, unsigned char *p);
#endif

#if defined G9991
void dump_RDD_US_SID_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p);
#endif

#if defined G9991
void dump_RDD_IPTV_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p);
#endif

#if defined G9991
void dump_RDD_IPTV_DDR_CONTEXT_ENTRY(bdmf_session_handle session, unsigned char *p);
#endif


#endif /* __RDD_RUNNER_REG_DUMP_H__ */
