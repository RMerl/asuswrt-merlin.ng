/*
   <:copyright-BRCM:2014-2016:DUAL/GPL:standard
   
      Copyright (c) 2014-2016 Broadcom 
      All Rights Reserved
   
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

#ifndef _RDD_DATA_STRUCTURES_H
#define _RDD_DATA_STRUCTURES_H

#include "rdd_data_structures_auto.h"

#ifndef BCM6858
#ifndef G9991
#define RDD_EMAC_NUMBER_OF_QUEUES         8
#else
#define RDD_EMAC_NUMBER_OF_QUEUES         4
#endif
#endif /* BCM6858 */

#define RDD_NUMBER_OF_TIMER_TASKS         8
#define RDD_CPU_RX_METER_TIMER_PERIOD     10000

/* CRC */
#define RDD_CRC_TYPE_16 0
#define RDD_CRC_TYPE_32 1

#define ADDRESS_OF(runner, task_name) runner##_##task_name

#define RDD_CLEAR_REGISTER(v) (*((uint32_t *)v) = 0)

/* DDR table offsets */
#define EPON_DDR_QUEUES_BASE_ADDRESS (g_ddr_runner_base_addr + g_runner_tables_offset + EPON_TX_DDR_QUEUES_ADDRESS)

#if defined(OREN) || defined(G9991) || defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
#if defined(FIRMWARE_INIT)
#if defined(OREN)
#define DS_TUPLE_LKP_TABLE_PTR ds_tuple_lkp_table_ptr
#define US_TUPLE_LKP_TABLE_PTR us_tuple_lkp_table_ptr
#define CONTEXT_TABLE_PTR g_context_table_ptr
#endif
#define CPU_RX_RING_PTR cpu_rx_ring_base_addr_ptr
#else
#if defined(OREN)
#define DS_TUPLE_LKP_TABLE_PTR (g_ddr_runner_base_addr + g_runner_tables_offset + DS_TUPLE_LKP_TABLE_ADDRESS)
#define US_TUPLE_LKP_TABLE_PTR (g_ddr_runner_base_addr + g_runner_tables_offset + US_TUPLE_LKP_TABLE_ADDRESS)
#define CONTEXT_TABLE_PTR (g_ddr_runner_base_addr + g_runner_tables_offset + CONTEXT_TABLE_ADDRESS)
#endif
#endif
#endif


typedef enum
{
    R8 = 0,
    R9 = 1,
    R10 = 2,
    R11 = 4,
    R12 = 5,
    R13 = 6,
    R14 = 8,
    R15 = 9,
    R16 = 10,
    R17 = 12,
    R18 = 13,
    R19 = 14,
    R20 = 16,
    R21 = 17,
    R22 = 18,
    R23 = 20,
    R24 = 21,
    R25 = 22,
    R26 = 24,
    R27 = 25,
    R28 = 26,
    R29 = 28,
    R30 = 29,
    R31 = 30,
} rdd_local_register_t;

typedef enum
{
    FAST_RUNNER_A = 0,
    FAST_RUNNER_B = 1,
    PICO_RUNNER_A = 2,
    PICO_RUNNER_B = 3,
} rdd_runner_index_t;

typedef enum
{
    RDD_CPU_TX_MESSAGE_DDR_HEADROOM_SIZE_SET                = 0,
    RDD_CPU_TX_MESSAGE_RX_FLOW_PM_COUNTERS_GET              = 1,
    RDD_CPU_TX_MESSAGE_TX_FLOW_PM_COUNTERS_GET              = 2,
    RDD_CPU_TX_MESSAGE_FLOW_PM_COUNTERS_GET                 = 3,
    RDD_CPU_TX_MESSAGE_BRIDGE_PORT_PM_COUNTERS_GET          = 4,
    RDD_CPU_TX_MESSAGE_FLUSH_WAN_TX_QUEUE                   = 5,
#ifdef UNDEF
    RDD_CPU_TX_MESSAGE_LAG_PORT_GET                         = 5,
#endif
    RDD_CPU_TX_MESSAGE_GLOBAL_REGISTERS_GET                 = 6,
    RDD_CPU_TX_MESSAGE_IPTV_MAC_COUNTER_GET                 = 7,
#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
    RDD_CPU_TX_MESSAGE_INVALIDATE_CONTEXT_INDEX_CACHE_ENTRY = 8, /* FIXME: update corresponding FW def */
#endif
    RDD_CPU_TX_MESSAGE_SEND_XON_FRAME                       = 8,
    RDD_CPU_TX_MESSAGE_RING_DESTROY                         = 9,
    RDD_CPU_TX_MESSAGE_IPV6_CRC_GET                         = 10,
    RDD_CPU_TX_MESSAGE_PM_COUNTER_GET                       = 11,
    RDD_CPU_TX_MESSAGE_FLUSH_ETH_QUEUE                      = 12,
#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
    RDD_CPU_TX_MESSAGE_UPDATE_PD_POOL_QUOTA                 = 13,
#else
    RDD_CPU_TX_MESSAGE_ACTIVATE_TCONT                       = 13,
#endif
    RDD_CPU_TX_MESSAGE_MIRRORING_MODE_CONFIG                = 14,
#ifdef CONFIG_DHD_RUNNER
    RDD_CPU_TX_MESSAGE_DHD_MESSAGE                          = 14,
#endif
#if defined(UNDEF) || defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
    RDD_CPU_TX_MESSAGE_RELEASE_SKB_BUFFERS                  = 15,
#endif
} rdd_cpu_tx_message_type_t;

#define RDD_EMAC_DESCRIPTOR_EGRESS_COUNTER_OFFSET  0

/* WAN TX Pointers table */
typedef struct
{
    uint16_t wan_channel_ptr;
    uint16_t rate_cntrl_ptr;
    uint16_t wan_tx_queue_ptr;
} rdd_wan_tx_pointers_entry_t;

#ifndef BCM6858
typedef struct
{
#if !defined(WL4908)
    rdd_wan_tx_pointers_entry_t entry[RDD_WAN_CHANNELS_0_7_TABLE_SIZE + RDD_WAN_CHANNELS_8_39_TABLE_SIZE]
                                     [RDD_WAN_CHANNEL_0_7_DESCRIPTOR_RATE_CONTROLLER_ADDR_NUMBER]
                                     [RDD_US_RATE_CONTROLLER_DESCRIPTOR_TX_QUEUE_ADDR_NUMBER];
#else
    rdd_wan_tx_pointers_entry_t entry[RDD_WAN_CHANNELS_0_7_TABLE_SIZE]
                                     [RDD_WAN_CHANNEL_0_7_DESCRIPTOR_RATE_CONTROLLER_ADDR_NUMBER]
                                     [RDD_US_RATE_CONTROLLER_DESCRIPTOR_TX_QUEUE_ADDR_NUMBER];
#endif
} rdd_wan_tx_pointers_table_t;
#endif

/* RX / TX */
#define RDD_RATE_CONTROLLERS_BUDGET_SET_SIZE          16
#define RDD_EMAC_EGRESS_COUNTER_OFFSET                0
#define RDD_US_RATE_CONTROL_EXPONENT0                 1
#define RDD_US_RATE_CONTROL_EXPONENT1                 4
#define RDD_US_RATE_CONTROL_EXPONENT2                 7
#define RDD_US_RATE_CONTROL_EXPONENT_NUM              3

/* CPU TX */
#define RDD_CPU_TX_SKB_INDEX_OWNERSHIP_BIT_MASK       0x8000
#define RDD_CPU_TX_SKB_INDEX_MASK                     0x3FFF
#define RDD_CPU_TX_SKB_LIMIT_MIN                      256
#define RDD_CPU_TX_SKB_LIMIT_MAX                      16384
#define RDD_CPU_TX_SKB_LIMIT_MULTIPLICATION           8

#define RDD_IH_PCI_CLASS                              2
#define RDD_IH_WAN_BRIDGE_LOW_CLASS                   9
#define RDD_IH_LAN_EMAC0_CLASS                        10
#define RDD_IH_LAN_EMAC1_CLASS                        11
#define RDD_IH_LAN_EMAC2_CLASS                        12
#define RDD_IH_LAN_EMAC3_CLASS                        13
#define RDD_IH_LAN_EMAC4_CLASS                        14

/* CPU-TX table */
#define RDD_CPU_TX_DESCRIPTOR_SIZE                    8
#define RDD_CPU_TX_QUEUE_SIZE                         16
#define RDD_CPU_TX_QUEUE_SIZE_MASK                    0xFF7F
#define RDD_CPU_TX_DESCRIPTOR_NUMBER_MASK             0x00F8
#define RDD_CPU_TX_ABS_DATA_PTR_DESCRIPTOR_SIZE       4
#define RDD_CPU_TX_ABS_DATA_PTR_QUEUE_SIZE            16
#define RDD_CPU_TX_ABS_DATA_PTR_QUEUE_SIZE_MASK       0xFFBF
#define RDD_CPU_TX_COMMAND_EGRESS_PORT_PACKET         0
#define RDD_CPU_TX_COMMAND_BRIDGE_PACKET              1
#define RDD_CPU_TX_COMMAND_INTERWORKING_PACKET        2
#define RDD_CPU_TX_COMMAND_ABSOLUTE_ADDRESS_PACKET    3
#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
#define RDD_CPU_TX_COMMAND_SPDSVC_PACKET              4
#endif
#define RDD_CPU_TX_COMMAND_MESSAGE                    7

#if defined(WL4908)
#if defined(CONFIG_BCM_JUMBO_FRAME)
#define RDD_RUNNER_PACKET_BUFFER_SIZE                 512 /* FPM token size */
#else
#define RDD_RUNNER_PACKET_BUFFER_SIZE                 256 /* FPM token size */
#endif
#elif (defined(DSL_63138) || defined(DSL_63148)) && defined(CONFIG_BCM_JUMBO_FRAME)
#define RDD_RUNNER_PACKET_BUFFER_SIZE                 4096
#elif defined(DSL_63138)
#define RDD_RUNNER_PACKET_BUFFER_SIZE                 2560
#else
#define RDD_RUNNER_PACKET_BUFFER_SIZE                 2048
#endif /*DSL*/

#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)

#define RDD_PACKET_DDR_OFFSET                         18

#define RDP_CFG_BUF_SIZE_2K                           0
#define RDP_CFG_BUF_SIZE_4K                           1
#define RDP_CFG_BUF_SIZE_16K                          2
#define RDP_CFG_BUF_SIZE_2_5K                         3

#define RDD_DMA_LOOKUP_RESULT_SLOT0                   0
#define RDD_DMA_LOOKUP_RESULT_SLOT1                   1
#define RDD_DMA_LOOKUP_RESULT_SLOT2                   2
#define RDD_DMA_LOOKUP_RESULT_SLOT3                   3
#define RDD_DMA_LOOKUP_RESULT_SLOT4                   4
#define RDD_DMA_LOOKUP_RESULT_SLOT5                   5
#define RDD_DMA_LOOKUP_RESULT_SLOT6                   6
#define RDD_DMA_LOOKUP_RESULT_SLOT7                   7
#define RDD_DMA_LOOKUP_RESULT_SLOT0_IO_ADDRESS        0x40
#define RDD_DMA_LOOKUP_RESULT_SLOT1_IO_ADDRESS        0x44
#define RDD_DMA_LOOKUP_RESULT_SLOT2_IO_ADDRESS        0x48
#define RDD_DMA_LOOKUP_RESULT_SLOT3_IO_ADDRESS        0x4C
#define RDD_DMA_LOOKUP_RESULT_SLOT4_IO_ADDRESS        0x50
#define RDD_DMA_LOOKUP_RESULT_SLOT5_IO_ADDRESS        0x54
#define RDD_DMA_LOOKUP_RESULT_SLOT6_IO_ADDRESS        0x58
#define RDD_DMA_LOOKUP_RESULT_SLOT7_IO_ADDRESS        0x5C

#define RDD_DMA_LOOKUP_4_STEPS                        4

#if defined(WL4908)
#define RDP_DDR_DATA_STRUCTURES_SIZE      0xa00000
#define RDP_DDR_MC_HEADER_SIZE            0x400000
#else
#define RDP_DDR_DATA_STRUCTURES_SIZE      0x500000
#endif

#endif /*DSL*/

#define RDD_FIELD_SHIFT(ls_bit_number, field_width, write_value)    ((write_value & ((1 << (field_width)) - 1)) << (ls_bit_number))

#define RDD_CPU_TX_DESCRIPTOR_CONTEXT_INDEX_WRITE(v, p)      FIELD_MWRITE_32(((uint8_t *)p + 0), 0, 15, v)
#define RDD_CPU_TX_DESCRIPTOR_COUNTER_WRITE(v, p)            FIELD_MWRITE_32(((uint8_t *)p + 0), 0,  5, v)
#define RDD_CPU_TX_DESCRIPTOR_IPTV_MAC_IDX_WRITE(v, p)       FIELD_MWRITE_32(((uint8_t *)p + 0), 0,  9, v)
#define RDD_CPU_TX_DESCRIPTOR_COUNTER_4_BYTES_WRITE(v, p)    FIELD_MWRITE_32(((uint8_t *)p + 0), 5,  1, v)
#define RDD_CPU_TX_DESCRIPTOR_SRC_BRIDGE_PORT_WRITE(v, p)    FIELD_MWRITE_32(((uint8_t *)p + 0), 14, 5, v)
#define RDD_CPU_TX_DESCRIPTOR_QUEUE_WRITE(v, p)              FIELD_MWRITE_32(((uint8_t *)p + 0), 19, 3, v)
#define RDD_CPU_TX_DESCRIPTOR_GROUP_WRITE(v, p)              FIELD_MWRITE_32(((uint8_t *)p + 0), 19, 7, v)
#define RDD_CPU_TX_DESCRIPTOR_FLOW_WRITE(v, p)               FIELD_MWRITE_32(((uint8_t *)p + 0), 19, 9, v)
#define RDD_CPU_TX_DESCRIPTOR_EMAC_WRITE(v, p)               FIELD_MWRITE_32(((uint8_t *)p + 0), 22, 4, v)
#define RDD_CPU_TX_DESCRIPTOR_INTERRUPT_NUMBER_WRITE(v, p)   FIELD_MWRITE_32(((uint8_t *)p + 0), 22, 4, v)
#define RDD_CPU_TX_DESCRIPTOR_TCONT_INDEX_WRITE(v, p)        FIELD_MWRITE_32(((uint8_t *)p + 4), 22, 6, v)
#define RDD_CPU_TX_DESCRIPTOR_TX_QUEUE_PTR_WRITE(v, p)       FIELD_MWRITE_32(((uint8_t *)p + 4), 23, 9, v)

#define RDD_CPU_TX_DESCRIPTOR_VALID_L_WRITE(v)               RDD_FIELD_SHIFT(31, 1, (v))
#define RDD_CPU_TX_DESCRIPTOR_COMMAND_L_WRITE(v)             RDD_FIELD_SHIFT(28, 3, (v))
#ifndef G9991
#define RDD_CPU_TX_DESCRIPTOR_EMAC_L_WRITE(v)                RDD_FIELD_SHIFT(22, 4, (v))
#else
#define RDD_CPU_TX_DESCRIPTOR_EMAC_L_WRITE(v)                RDD_FIELD_SHIFT(22, 5, (v))
#endif
#define RDD_CPU_TX_DESCRIPTOR_US_GEM_FLOW_L_WRITE(v)         RDD_FIELD_SHIFT(20, 8, (v))
#define RDD_CPU_TX_DESCRIPTOR_QUEUE_L_WRITE(v)               RDD_FIELD_SHIFT(19, 3, (v))
#define RDD_CPU_TX_DESCRIPTOR_SUBNET_ID_L_WRITE(v)           RDD_FIELD_SHIFT(19, 4, (v))
#define RDD_CPU_TX_DESCRIPTOR_DS_GEM_FLOW_L_WRITE(v)         RDD_FIELD_SHIFT(14, 8, (v))
#define RDD_CPU_TX_DESCRIPTOR_SRC_BRIDGE_PORT_L_WRITE(v)     RDD_FIELD_SHIFT(14, 5, (v))
#define RDD_CPU_TX_DESCRIPTOR_PACKET_LENGTH_L_WRITE(v)       RDD_FIELD_SHIFT(0, 14, (v))
#define RDD_CPU_TX_DESCRIPTOR_SSID_L_WRITE(v)                RDD_FIELD_SHIFT(27, 4, (v))
#define RDD_CPU_TX_DESCRIPTOR_TX_QUEUE_PTR_L_WRITE(v)        RDD_FIELD_SHIFT(23, 9, (v))
#define RDD_CPU_TX_DESCRIPTOR_IH_CLASS_L_WRITE(v)            RDD_FIELD_SHIFT(23, 4, (v))
#define RDD_CPU_TX_DESCRIPTOR_PAYLOAD_OFFSET_L_WRITE(v)      RDD_FIELD_SHIFT(16, 7, (v))
#define RDD_CPU_TX_DESCRIPTOR_1588_INDICATION_L_WRITE(v)     RDD_FIELD_SHIFT(27, 1, (v))
#define RDD_CPU_TX_DESCRIPTOR_TCONT_INDEX_L_WRITE(v)         RDD_FIELD_SHIFT(8, 6, (v))
#define RDD_CPU_TX_DESCRIPTOR_ABS_FLAG_L_WRITE(v)            RDD_FIELD_SHIFT(31, 1, (v))
#define RDD_CPU_TX_DESCRIPTOR_BUFFER_NUMBER_L_WRITE(v)       RDD_FIELD_SHIFT(0, 14, (v))
#define RDD_CPU_TX_DESCRIPTOR_SKB_INDEX_L_WRITE(v)           RDD_FIELD_SHIFT(0, 8, (v))
#define RDD_CPU_TX_DESCRIPTOR_FLOW_L_WRITE(v)                RDD_FIELD_SHIFT(19, 9, (v))
#define RDD_CPU_TX_DESCRIPTOR_IPTV_MAC_IDX_L_WRITE(v)        RDD_FIELD_SHIFT(0, 11, (v))
#define RDD_CPU_TX_DESCRIPTOR_GROUP_L_WRITE(v)               RDD_FIELD_SHIFT(19, 7, (v))
#define RDD_CPU_TX_DESCRIPTOR_COUNTER_L_WRITE(v)             RDD_FIELD_SHIFT(0, 5, (v))
#define RDD_CPU_TX_DESCRIPTOR_COUNTER_4_BYTES_L_WRITE(v)     RDD_FIELD_SHIFT(5, 1, (v))
#define RDD_CPU_TX_DESCRIPTOR_MSG_TYPE_L_WRITE(v)            RDD_FIELD_SHIFT(0, 4, (v))

/* VLAN & PBITs actions */
#define RDD_VLAN_TYPES           4
#define RDD_VLAN_TYPE_UNTAGGED   0
#define RDD_VLAN_TYPE_SINGLE     1
#define RDD_VLAN_TYPE_DOUBLE     2
#define RDD_VLAN_TYPE_PRIORITY   3
#define RDD_VLAN_COMMAND_SKIP    128

typedef struct
{
    uint8_t  vlan_action;
    uint8_t  pbits_action;
} __PACKING_ATTRIBUTE_STRUCT_END__ rdd_vlan_action_entry_t;

typedef struct
{
    rdd_vlan_action_entry_t  entry[RDD_VLAN_TYPES][RDD_MAX_VLAN_CMD][RDD_MAX_PBITS_CMD];
} rdd_vlan_actions_matrix_t;

#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
typedef struct
{
    uint32_t    good_csum_packets:32            __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t    no_csum_packets:32              __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t    bad_ipv4_hdr_csum_packets:32    __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t    bad_tcp_udp_csum_packets:32     __PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ rdd_cso_counters_entry_t;
#endif /*DSL*/

#ifdef WL4908
#define RDD_FLOW_ENTRIES_SIZE 16512
#define RDD_FLOW_ENTRY_VALID 0x80000000
#define RDD_RESERVED_CONTEXT_ENTRIES 128
#define RDD_CONTEXT_TABLE_SIZE RDD_FLOW_ENTRIES_SIZE
#define RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_PORT_CONTEXT_NUMBER 8
#define RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_L3_COMMAND_LIST_NUMBER  20
#define RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_RESERVED4_NUMBER    24

typedef struct
{
    uint32_t    flow_hits:32                    __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t    flow_bytes:32                   __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t    multicast_flag:1                __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t    overflow:1                      __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t    is_routed:1                     __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t    is_tos_mangle:1                 __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t    number_of_ports:4               __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t    port_mask:8                     __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t    reserved2:5                     __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t    mtu:11                          __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t    rx_tos:8                        __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t    reserved3:6                     __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t    is_mapt_us:1                    __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t    is_df:1                         __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t    wlan_mcast_clients:8            __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t    wlan_mcast_index:8              __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t    mcast_port_header_buffer_ptr:32 __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t    port_context[RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_PORT_CONTEXT_NUMBER];
    uint8_t     l3_command_list[RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_L3_COMMAND_LIST_NUMBER];
    uint8_t     reserved4[RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_RESERVED4_NUMBER];
    uint32_t    valid:8                         __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t    reserved5:3                     __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t    command_list_length_64:4        __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t    connection_direction:1          __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t    connection_table_index:16       __PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_DTS;

#define RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_COMMAND_LIST_NUMBER 80

typedef struct
{
    uint32_t    flow_hits:32                    __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t    flow_bytes:32                   __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t    reserved2:8                     __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t    rx_tos:8                        __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t    mtu:11                          __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t    reserved1:2                     __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t    is_routed:1                     __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t    overflow:1                      __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t    multicast_flag:1                __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t    link_specific_union:16          __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t    reserved3:2                     __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t    ip_addresses_table_index:3      __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t    egress_phy:2                    __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t    wfd_idx:2                       __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t    wfd_prio:1                      __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t    priority:4                      __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t    is_unicast_wfd_any:1            __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t    is_unicast_wfd_nic:1            __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint8_t     command_list[RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_COMMAND_LIST_NUMBER];
    uint32_t    valid:8                     __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t    reserved4:3                 __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t    command_list_length_64:4    __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t    connection_direction:1      __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t    connection_table_index:16   __PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_DTS;

typedef union
{
    RDD_FC_UCAST_FLOW_CONTEXT_ETH_XTM_ENTRY_DTS fc_ucast_flow_context_eth_xtm_entry;
    RDD_FC_UCAST_FLOW_CONTEXT_WFD_NIC_ENTRY_DTS fc_ucast_flow_context_wfd_nic_entry;
    RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_DTS         fc_mcast_flow_context_entry;
    RDD_FC_UCAST_FLOW_CONTEXT_ENTRY_DTS         fc_ucast_flow_context_entry;
    RDD_FC_UCAST_FLOW_CONTEXT_RNR_DHD_ENTRY_DTS fc_ucast_flow_context_rnr_dhd_entry;
    RDD_FC_UCAST_FLOW_CONTEXT_WFD_DHD_ENTRY_DTS fc_ucast_flow_context_wfd_dhd_entry;
}
__PACKING_ATTRIBUTE_STRUCT_END__ rdd_fc_context_t;
#else /*!WL4908:*/
#define RDD_FLOW_CACHE_L2_HEADER_BYTE_SIZE 32
typedef struct
{
    rdpa_traffic_dir conn_dir;
    bdmf_index conn_index;

    rdpa_fc_action_vec_t actions_vector;
    rdd_fc_fwd_action_t fwd_action; /* In use when forward action action is turned on in action vector */
    rdpa_cpu_reason trap_reason; /* CPU trap reason in case forwarding action is ::rdpa_forward_action_host
                                    and ::rdpa_fc_action_forward is set. */
    bdmf_boolean service_queue_enabled;
    bdmf_index service_queue_id;
    rdpa_qos_method qos_method;

    bdmf_ip_family ip_version;
    uint16_t nat_port;
    bdmf_ip_t nat_ip;
    uint8_t ds_lite_hdr_index;

    uint8_t ovid_offset;
    uint8_t opbit_action;
    uint8_t ipbit_action;
    rdpa_dscp dscp_value; /* DSCP value if ::rdpa_fc_action_dscp_remark is set. */
    uint8_t ecn_value;
    uint8_t policer_id;


    uint32_t ip_checksum_delta;
    uint32_t l4_checksum_delta;
    uint8_t phy_egress_port;
    uint8_t vir_egress_port;

    /* For WAN/LAN egress, mutual exclusive with wl_metadata */
    uint8_t traffic_class;
    uint8_t wan_flow_index;
    uint8_t rate_controller;

    /* For WiFi usage */
    uint8_t wifi_ssid;
    union {
        uint32_t wl_metadata;                  /**< WL metadata */
        rdpa_wfd_t wfd;
        rdpa_rnr_t rnr;
    };

    int8_t l2_hdr_offset;
    uint8_t l2_hdr_size;
    uint8_t l2_hdr_number_of_tags;
    uint8_t l2_header[RDD_FLOW_CACHE_L2_HEADER_BYTE_SIZE];

    /* Statistics (read-only) */
    rdpa_stat_t valid_cnt;
} rdd_fc_context_t;
#endif /*!WL4908*/

typedef struct
{
    rdpa_ip_flow_key_t *lookup_entry;
    rdd_fc_context_t context_entry;
    uint32_t xo_entry_index;
} rdd_ip_flow_t;

#define rdd_phys_addr_t bdmf_phys_addr_t 

#endif /*_RDD_DATA_STRUCTURES_H */
