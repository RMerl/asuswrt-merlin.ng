/*
    <:copyright-BRCM:2013:DUAL/GPL:standard
    
       Copyright (c) 2013 Broadcom 
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

#ifndef _BL_LILAC_DRV_RUNNER_DATA_STRUCTURES_H
#define _BL_LILAC_DRV_RUNNER_DATA_STRUCTURES_H


/********************************** Defines ***********************************/

/* Runner Device Driver version */
#define LILAC_RDD_RELEASE                                   ( 0x04 )
#define LILAC_RDD_VERSION                                   ( 0x10 )
#define LILAC_RDD_PATCH                                     ( 0x03 )
#define LILAC_RDD_REVISION                                  ( 0x01 )

#define LILAC_RDD_RUNNER_PACKET_DESCRIPTOR_SIZE             8
#define LILAC_RDD_RUNNER_PACKET_BUFFER_SIZE                 2048
#define LILAC_RDD_RUNNER_EXTENSION_BUFFER_SIZE              512
#define LILAC_RDD_PACKET_DDR_OFFSET                         18
#define LILAC_RDD_RUNNER_PSRAM_BUFFER_SIZE                  128
#define LILAC_RDD_RUNNER_EXTENSION_PACKET_BUFFER_SIZE       64
#define LILAC_RDD_RUNNER_EXTENSION_PACKET_HEADER_SIZE       32

/* RX - TX */
#define LILAC_RDD_RATE_CONTROLLERS_BUDGET_SET_SIZE          16
#ifndef G9991
#define LILAC_RDD_EMAC_NUMBER_OF_QUEUES                     8
#else
#define LILAC_RDD_EMAC_NUMBER_OF_QUEUES                     4
#endif
#define LILAC_RDD_EMAC_EGRESS_COUNTER_OFFSET          0
#define RDD_RATE_CONTROL_EXPONENT0                    1
#define RDD_RATE_CONTROL_EXPONENT1                    4
#define RDD_RATE_CONTROL_EXPONENT2                    7
#define RDD_RATE_CONTROL_EXPONENT_NUM                 3

/* CPU-RX table & queues */
#define RDD_CPU_FREE_PACKET_DESCRIPTORS_POOL_SIZE           256
#define RDD_CPU_FREE_PACKET_DESCRIPTORS_POOL_C_SIZE         1

/* CPU-TX table */
#define LILAC_RDD_CPU_TX_DESCRIPTOR_SIZE                    8
#define LILAC_RDD_CPU_TX_QUEUE_SIZE                         16
#define LILAC_RDD_CPU_TX_QUEUE_SIZE_MASK                    0xFF7F
#define LILAC_RDD_CPU_TX_DESCRIPTOR_NUMBER_MASK             0x00F8
#define LILAC_RDD_CPU_TX_ABS_DATA_PTR_DESCRIPTOR_SIZE       4
#define LILAC_RDD_CPU_TX_ABS_DATA_PTR_QUEUE_SIZE            16
#define LILAC_RDD_CPU_TX_ABS_DATA_PTR_QUEUE_SIZE_MASK       0xFFBF
#define LILAC_RDD_CPU_TX_COMMAND_EGRESS_PORT_PACKET         0
#define LILAC_RDD_CPU_TX_COMMAND_BRIDGE_PACKET              1
#define LILAC_RDD_CPU_TX_COMMAND_INTERWORKING_PACKET        2
#define LILAC_RDD_CPU_TX_COMMAND_ABSOLUTE_ADDRESS_PACKET    3
#define LILAC_RDD_CPU_TX_COMMAND_SPDSVC_PACKET              6
#define LILAC_RDD_CPU_TX_COMMAND_MESSAGE                    7

/* Bridging */
#define LILAC_RDD_NUMBER_OF_BRIDGE_PORTS                    8
#define LILAC_RDD_FLOW_CLASSIFICATION_ENTRY_STOP            0xFFFF
#define LILAC_RDD_MAX_PBITS                                 7
#define LILAC_RDD_NUMBER_OF_ETHER_TYPE_FILTERS              12

#define RDD_INGRESS_CLASSIFICATION_SEARCH_DEPTH             32

/* Ingress Handler */
#define LILAC_RDD_IH_HEADER_LENGTH                          110
#define LILAC_RDD_IH_BUFFER_BBH_ADDRESS                     0x8000
#define LILAC_RDD_RUNNER_A_IH_BUFFER                        14
#define LILAC_RDD_RUNNER_B_IH_BUFFER                        15
#define LILAC_RDD_RUNNER_A_IH_BUFFER_BBH_OFFSET             ( ( LILAC_RDD_RUNNER_A_IH_BUFFER * 128 + LILAC_RDD_PACKET_DDR_OFFSET ) / 8 )
#define LILAC_RDD_RUNNER_B_IH_BUFFER_BBH_OFFSET             ( ( LILAC_RDD_RUNNER_B_IH_BUFFER * 128 + LILAC_RDD_PACKET_DDR_OFFSET ) / 8 )
#define LILAC_RDD_IH_HEADER_DESCRIPTOR_BBH_ADDRESS          0x0
#define INVALID_BPM_BUFFER                                  0xFFFF

#define LILAC_RDD_IH_PCI_CLASS                              2
#define LILAC_RDD_IH_WAN_BRIDGE_HIGH_CLASS                  8
#define LILAC_RDD_IH_WAN_BRIDGE_LOW_CLASS                   9
#define LILAC_RDD_IH_LAN_EMAC0_CLASS                        10
#define LILAC_RDD_IH_LAN_EMAC1_CLASS                        11
#define LILAC_RDD_IH_LAN_EMAC2_CLASS                        12
#define LILAC_RDD_IH_LAN_EMAC3_CLASS                        13
#define LILAC_RDD_IH_LAN_EMAC4_CLASS                        14

/* Parser */
#define LILAC_RDD_PARSER_LAYER_2_PROTOCOL_PPPOE_D           1
#define LILAC_RDD_PARSER_LAYER_2_PROTOCOL_PPPOE_S           2
#define LILAC_RDD_PARSER_LAYER_2_PROTOCOL_USER_0            8
#define LILAC_RDD_PARSER_LAYER_2_PROTOCOL_USER_1            9
#define LILAC_RDD_PARSER_LAYER_2_PROTOCOL_USER_2            10
#define LILAC_RDD_PARSER_LAYER_2_PROTOCOL_USER_3            11
#define LILAC_RDD_PARSER_LAYER_2_PROTOCOL_ARP               12
#define LILAC_RDD_PARSER_LAYER_2_PROTOCOL_1588              13
#define LILAC_RDD_PARSER_LAYER_2_PROTOCOL_802_1X            14
#define LILAC_RDD_PARSER_LAYER_2_PROTOCOL_802_1AG_CFM       15
#define LILAC_RDD_PARSER_LAYER_3_PROTOCOL_OTHER             0
#define LILAC_RDD_PARSER_LAYER_3_PROTOCOL_IPV4              1
#define LILAC_RDD_PARSER_LAYER_3_PROTOCOL_IPV6              2
#define LILAC_RDD_PARSER_LAYER_4_PROTOCOL_UNKNOWN           0
#define LILAC_RDD_PARSER_LAYER_4_PROTOCOL_IGMP              3
#define LILAC_RDD_PARSER_LAYER_4_PROTOCOL_ICMP              4
#define LILAC_RDD_PARSER_LAYER_4_PROTOCOL_ICMPV6            5
#define LILAC_RDD_PARSER_LAYER_4_PROTOCOL_ESP               6
#define LILAC_RDD_PARSER_LAYER_4_PROTOCOL_GRE               7
#define LILAC_RDD_PARSER_LAYER_4_PROTOCOL_USER_0            8
#define LILAC_RDD_PARSER_LAYER_4_PROTOCOL_USER_1            9
#define LILAC_RDD_PARSER_LAYER_4_PROTOCOL_USER_2            10
#define LILAC_RDD_PARSER_LAYER_4_PROTOCOL_USER_3            11
#define LILAC_RDD_PARSER_LAYER_4_PROTOCOL_IPV6              13
#define LILAC_RDD_PARSER_LAYER_4_PROTOCOL_AH                14

#define LILAC_RDD_CAM_RESULT_SLOT0                          0
#define LILAC_RDD_CAM_RESULT_SLOT1                          1
#define LILAC_RDD_CAM_RESULT_SLOT2                          2
#define LILAC_RDD_CAM_RESULT_SLOT3                          3
#define LILAC_RDD_CAM_RESULT_SLOT4                          4
#define LILAC_RDD_CAM_RESULT_SLOT5                          5
#define LILAC_RDD_CAM_RESULT_SLOT6                          6
#define LILAC_RDD_CAM_RESULT_SLOT7                          7
#define LILAC_RDD_CAM_RESULT_SLOT0_IO_ADDRESS               0x60
#define LILAC_RDD_CAM_RESULT_SLOT1_IO_ADDRESS               0x64
#define LILAC_RDD_CAM_RESULT_SLOT2_IO_ADDRESS               0x68
#define LILAC_RDD_CAM_RESULT_SLOT3_IO_ADDRESS               0x6C
#define LILAC_RDD_CAM_RESULT_SLOT4_IO_ADDRESS               0x70
#define LILAC_RDD_CAM_RESULT_SLOT5_IO_ADDRESS               0x74
#define LILAC_RDD_CAM_RESULT_SLOT6_IO_ADDRESS               0x78
#define LILAC_RDD_CAM_RESULT_SLOT7_IO_ADDRESS               0x7C

#define LILAC_RDD_HASH_RESULT_SLOT0                          0
#define LILAC_RDD_HASH_RESULT_SLOT1                          1
#define LILAC_RDD_HASH_RESULT_SLOT2                          2
#define LILAC_RDD_HASH_RESULT_SLOT3                          3
#define LILAC_RDD_HASH_RESULT_SLOT4                          4
#define LILAC_RDD_HASH_RESULT_SLOT5                          5
#define LILAC_RDD_HASH_RESULT_SLOT0_IO_ADDRESS               0x20
#define LILAC_RDD_HASH_RESULT_SLOT1_IO_ADDRESS               0x24
#define LILAC_RDD_HASH_RESULT_SLOT2_IO_ADDRESS               0x28
#define LILAC_RDD_HASH_RESULT_SLOT3_IO_ADDRESS               0x2C
#define LILAC_RDD_HASH_RESULT_SLOT4_IO_ADDRESS               0x30
#define LILAC_RDD_HASH_RESULT_SLOT5_IO_ADDRESS               0x34

/* MAC Table */
#define LILAC_RDD_MAC_CONTEXT_MULTICAST                     0x1
#define LILAC_RDD_MAC_CONTEXT_ENTRY_TYPE_MASK               ( 1 << 6 )

/* IPTV Tables */
#define LILAC_RDD_IPTV_TABLE_SET_SIZE                       4
#define LILAC_RDD_IPTV_ENTRY_SIZE                           16
#define LILAC_RDD_IPTV_NUMBER_OF_SERVICE_PROVIDERS          32
#define LILAC_RDD_IPTV_SOURCE_IP_TABLE_SEARCH_DEPTH         32
#define LILAC_RDD_IPTV_LOOKUP_TABLE_CAM_SIZE                32
#define LILAC_RDD_IPTV_SSM_CONTEXT_ENTRY_COUNT              1024
#define LILAC_RDD_IPTV_SSM_CONTEXT_TABLE_SIZE               ( LILAC_RDD_IPTV_NUMBER_OF_SERVICE_PROVIDERS * LILAC_RDD_IPTV_SSM_CONTEXT_ENTRY_COUNT )
#define LILAC_RDD_IPTV_L3_SRC_IP_IPV6_SKIP_VALUE            0x8000
#define LILAC_RDD_IPTV_L3_SRC_IP_IPV6_STOP_VALUE            0xFFFF

/* Router Tables */
#define LILAC_RDD_NUMBER_OF_SUBNETS                         3

#define LILAC_RDD_DMA_LOOKUP_RESULT_SLOT0                   0
#define LILAC_RDD_DMA_LOOKUP_RESULT_SLOT1                   1
#define LILAC_RDD_DMA_LOOKUP_RESULT_SLOT2                   2
#define LILAC_RDD_DMA_LOOKUP_RESULT_SLOT3                   3
#define LILAC_RDD_DMA_LOOKUP_RESULT_SLOT4                   4
#define LILAC_RDD_DMA_LOOKUP_RESULT_SLOT5                   5
#define LILAC_RDD_DMA_LOOKUP_RESULT_SLOT0_IO_ADDRESS        0x40
#define LILAC_RDD_DMA_LOOKUP_RESULT_SLOT1_IO_ADDRESS        0x44
#define LILAC_RDD_DMA_LOOKUP_RESULT_SLOT2_IO_ADDRESS        0x48
#define LILAC_RDD_DMA_LOOKUP_RESULT_SLOT3_IO_ADDRESS        0x4C
#define LILAC_RDD_DMA_LOOKUP_RESULT_SLOT4_IO_ADDRESS        0x50
#define LILAC_RDD_DMA_LOOKUP_RESULT_SLOT5_IO_ADDRESS        0x54
#define LILAC_RDD_DMA_LOOKUP_4_STEPS                        4

#define LILAC_RDD_NUMBER_OF_TIMER_TASKS                     8
#define LILAC_RDD_CPU_RX_METER_TIMER_PERIOD                 10000

/* VLAN & PBITs actions */
#define LILAC_RDD_VLAN_TYPES                                4
#define LILAC_RDD_VLAN_TYPE_UNTAGGED                        0
#define LILAC_RDD_VLAN_TYPE_SINGLE                          1
#define LILAC_RDD_VLAN_TYPE_DOUBLE                          2
#define LILAC_RDD_VLAN_TYPE_PRIORITY                        3
#define LILAC_RDD_VLAN_COMMAND_SKIP                         128

/* VLAN switching */
#define LILAC_RDD_LAN_VID_SKIP_VALUE                        0x8000
#define LILAC_RDD_LAN_VID_STOP_VALUE                        0xFFFF

#define ACK_PACKET_SIZE_THRESHOLD                           100

/* Firewall */
#define LILAC_RDD_FIREWALL_RULES_MASK_MAX_LENGTH            32

/* PCI */
#define LILAC_RDD_PCI_TX_NUMBER_OF_FIFOS                    4
#define LILAC_RDD_PCI_TX_FIFO_SIZE                          8

/* CRC */
#define RDD_CRC_TYPE_16                                     0
#define RDD_CRC_TYPE_32                                     1

/* GPIO */
#define LILAC_RDD_GPIO_IO_ADDRESS                           0x148

/* -Etc- */
#define LILAC_RDD_TRUE                                      1
#define LILAC_RDD_FALSE                                     0
#define LILAC_RDD_ON                                        1
#define LILAC_RDD_OFF                                       0

/* CPU TX */
#define LILAC_RDD_CPU_TX_SKB_INDEX_OWNERSHIP_BIT_MASK       0x8000
#define LILAC_RDD_CPU_TX_SKB_INDEX_HOST_BUF_BIT_MASK        0x4000
#define LILAC_RDD_CPU_TX_SKB_INDEX_MASK                     0x1FFF
#define LILAC_RDD_CPU_TX_SKB_LIMIT_MIN                      256
#define LILAC_RDD_CPU_TX_SKB_LIMIT_DEFAULT                  0x800
#define LILAC_RDD_CPU_TX_SKB_LIMIT_MAX                      16384
#define LILAC_RDD_CPU_TX_SKB_LIMIT_MULTIPLICATION           8
#define LILAC_RDD_CPU_TX_BURSTBANK_CHANNEL                  1
#define LILAC_RDD_CPU_TX_BURSTBANK_CHUNK                    15

#define RDD_CLEAR_REGISTER( v )                             ( *( ( uint32_t *) v ) = 0 )

#define ADDRESS_OF(runner, task_name)		runner##_##task_name

#define RDP_DDR_DATA_STRUCTURES_SIZE      0x500000

#define EPON_WAN_CHANNEL_MAPPING          8

#if !defined(FIRMWARE_INIT)
/* DDR offsets */
#define DsConnectionTableBase              ( g_runner_tables_ptr + DS_CONNECTION_TABLE_ADDRESS )
#define UsConnectionTableBase              ( g_runner_tables_ptr + US_CONNECTION_TABLE_ADDRESS )
#define ContextTableBase                   ( g_runner_tables_ptr + CONTEXT_TABLE_ADDRESS )
#define IPTVTableBase                      ( g_runner_tables_ptr + IPTV_DDR_LOOKUP_TABLE_ADDRESS )
#define IPTVContextTableBase               ( g_runner_tables_ptr + IPTV_DDR_CONTEXT_TABLE_ADDRESS )
#define IPTVSsmContextTableBase            ( g_runner_tables_ptr + IPTV_SSM_DDR_CONTEXT_TABLE_ADDRESS )
#define FirewallRulesMapTable              ( g_runner_tables_ptr + FIREWALL_RULES_MAP_TABLE_ADDRESS )
#define FirewallRulesTable                 ( g_runner_tables_ptr + FIREWALL_RULES_TABLE_ADDRESS )
#define DdrAddressForSyncDmaBase           ( g_runner_tables_ptr + DDR_ADDRESS_FOR_SYNC_DMA_ADDRESS )
#endif

/* DDR offsets */
#define EponPostSchedulingDdrQueuesBase   ( g_runner_tables_ptr + EPON_TX_POST_SCHEDULING_DDR_QUEUES_ADDRESS )

/******************************* Data structures ******************************/

#define LILAC_RDD_FIELD_SHIFT( ls_bit_number, field_width, write_value )    ( ( write_value & ( ( 1 << (field_width) ) - 1 ) ) << ( ls_bit_number ) )


/* WAN TX Pointers table */
typedef struct
{
    uint16_t  wan_channel_ptr;
    uint16_t  rate_controller_ptr;
    uint16_t  wan_tx_queue_ptr;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_WAN_TX_POINTERS_ENTRY_DTS;


typedef struct
{
    RDD_WAN_TX_POINTERS_ENTRY_DTS  entry[ RDD_WAN_CHANNELS_0_7_TABLE_SIZE + RDD_WAN_CHANNELS_8_39_TABLE_SIZE ][ RDD_WAN_CHANNEL_0_7_DESCRIPTOR_RATE_CONTROLLER_ADDR_NUMBER ]
                                        [ RDD_US_RATE_CONTROLLER_DESCRIPTOR_TX_QUEUE_ADDR_NUMBER ];
}
RDD_WAN_TX_POINTERS_TABLE_DTS;

#define RDD_CPU_TX_DESCRIPTOR_CONTEXT_INDEX_WRITE( v, p )      FIELD_MWRITE_32( ( (uint8_t *)p + 0), 0, 15, v )
#define RDD_CPU_TX_DESCRIPTOR_COUNTER_WRITE( v, p )            FIELD_MWRITE_32( ( (uint8_t *)p + 0), 0,  5, v )
#define RDD_CPU_TX_DESCRIPTOR_IPTV_MAC_IDX_WRITE( v, p )       FIELD_MWRITE_32( ( (uint8_t *)p + 0), 0, 11, v )
#define RDD_CPU_TX_DESCRIPTOR_COUNTER_4_BYTES_WRITE( v, p )    FIELD_MWRITE_32( ( (uint8_t *)p + 0), 5,  1, v )
#define RDD_CPU_TX_DESCRIPTOR_REGISTER_NUMBER_WRITE(v, p)      FIELD_MWRITE_32( ( (uint8_t *)p + 0), 0,  3, v )
#define RDD_CPU_TX_DESCRIPTOR_SRC_BRIDGE_PORT_WRITE( v, p )    FIELD_MWRITE_32( ( (uint8_t *)p + 0), 14, 5, v )
#define RDD_CPU_TX_DESCRIPTOR_QUEUE_WRITE( v, p )              FIELD_MWRITE_32( ( (uint8_t *)p + 0), 19, 3, v )
#define RDD_CPU_TX_DESCRIPTOR_GROUP_WRITE( v, p )              FIELD_MWRITE_32( ( (uint8_t *)p + 0), 19, 7, v )
#define RDD_CPU_TX_DESCRIPTOR_FLOW_WRITE( v, p )               FIELD_MWRITE_32( ( (uint8_t *)p + 0), 19, 9, v )
#define RDD_CPU_TX_DESCRIPTOR_EMAC_WRITE( v, p )               FIELD_MWRITE_32( ( (uint8_t *)p + 0), 22, 4, v )
#define RDD_CPU_TX_DESCRIPTOR_INTERRUPT_NUMBER_WRITE( v, p )   FIELD_MWRITE_32( ( (uint8_t *)p + 0), 22, 4, v )
#define RDD_CPU_TX_DESCRIPTOR_COUNTER_CLEAR_WRITE(v, p)        FIELD_MWRITE_32( ( (uint8_t *)p + 4), 16, 1, v )
#define RDD_CPU_TX_DESCRIPTOR_TCONT_INDEX_WRITE( v, p )        FIELD_MWRITE_32( ( (uint8_t *)p + 4), 22, 6, v )
#define RDD_CPU_TX_DESCRIPTOR_TX_QUEUE_PTR_WRITE( v, p )       FIELD_MWRITE_32( ( (uint8_t *)p + 4), 23, 9, v )

#define RDD_CPU_TX_DESCRIPTOR_VALID_L_WRITE( v )               LILAC_RDD_FIELD_SHIFT( 31,  1, (v) )
#define RDD_CPU_TX_DESCRIPTOR_COMMAND_L_WRITE( v )             LILAC_RDD_FIELD_SHIFT( 28,  3, (v) )
#ifndef G9991
#define RDD_CPU_TX_DESCRIPTOR_EMAC_L_WRITE( v )                LILAC_RDD_FIELD_SHIFT( 22,  4, (v) )
#else
#define RDD_CPU_TX_DESCRIPTOR_EMAC_L_WRITE( v )                LILAC_RDD_FIELD_SHIFT( 22,  5, (v) )
#endif
#define RDD_CPU_TX_DESCRIPTOR_US_GEM_FLOW_L_WRITE( v )         LILAC_RDD_FIELD_SHIFT( 20,  8, (v) )
#define RDD_CPU_TX_DESCRIPTOR_QUEUE_L_WRITE( v )               LILAC_RDD_FIELD_SHIFT( 19,  3, (v) )
#define RDD_CPU_TX_DESCRIPTOR_SUBNET_ID_L_WRITE( v )           LILAC_RDD_FIELD_SHIFT( 19,  4, (v) )
#define RDD_CPU_TX_DESCRIPTOR_DS_GEM_FLOW_L_WRITE( v )         LILAC_RDD_FIELD_SHIFT( 14,  8, (v) )
#define RDD_CPU_TX_DESCRIPTOR_SRC_BRIDGE_PORT_L_WRITE( v )     LILAC_RDD_FIELD_SHIFT( 14,  5, (v) )
#define RDD_CPU_TX_DESCRIPTOR_PACKET_LENGTH_L_WRITE( v )       LILAC_RDD_FIELD_SHIFT(  0, 14, (v) )
#define RDD_CPU_TX_DESCRIPTOR_SSID_L_WRITE( v )                LILAC_RDD_FIELD_SHIFT( 27,  4, (v) )
#define RDD_CPU_TX_DESCRIPTOR_TX_QUEUE_PTR_L_WRITE( v )        LILAC_RDD_FIELD_SHIFT( 23,  9, (v) )
#define RDD_CPU_TX_DESCRIPTOR_IH_CLASS_L_WRITE( v )            LILAC_RDD_FIELD_SHIFT( 23,  4, (v) )
#define RDD_CPU_TX_DESCRIPTOR_PAYLOAD_OFFSET_L_WRITE( v )      LILAC_RDD_FIELD_SHIFT( 16,  7, (v) )
#define RDD_CPU_TX_DESCRIPTOR_1588_INDICATION_L_WRITE( v )     LILAC_RDD_FIELD_SHIFT( 27,  1, (v) )
#define RDD_CPU_TX_DESCRIPTOR_TCONT_INDEX_L_WRITE( v )         LILAC_RDD_FIELD_SHIFT(  8,  6, (v) )
#define RDD_CPU_TX_DESCRIPTOR_ABS_FLAG_L_WRITE( v )            LILAC_RDD_FIELD_SHIFT( 31,  1, (v) )
#define RDD_CPU_TX_DESCRIPTOR_BUFFER_NUMBER_L_WRITE( v )       LILAC_RDD_FIELD_SHIFT(  0, 14, (v) )
#define RDD_CPU_TX_DESCRIPTOR_SKB_INDEX_L_WRITE( v )           LILAC_RDD_FIELD_SHIFT(  0,  8, (v) )
#define RDD_CPU_TX_DESCRIPTOR_FLOW_L_WRITE( v )                LILAC_RDD_FIELD_SHIFT( 19,  9, (v) )
#define RDD_CPU_TX_DESCRIPTOR_IPTV_MAC_IDX_L_WRITE( v )        LILAC_RDD_FIELD_SHIFT(  0, 11, (v) )
#define RDD_CPU_TX_DESCRIPTOR_GROUP_L_WRITE( v )               LILAC_RDD_FIELD_SHIFT( 19,  7, (v) )
#define RDD_CPU_TX_DESCRIPTOR_COUNTER_L_WRITE( v )             LILAC_RDD_FIELD_SHIFT(  0,  5, (v) )
#define RDD_CPU_TX_DESCRIPTOR_COUNTER_4_BYTES_L_WRITE( v )     LILAC_RDD_FIELD_SHIFT(  5,  1, (v) )
#define RDD_CPU_TX_DESCRIPTOR_MSG_TYPE_L_WRITE( v )            LILAC_RDD_FIELD_SHIFT(  0,  4, (v) )



#define RDD_PCI_TX_DESCRIPTOR_SSID_MULTICAST_L_WRITE( v )    LILAC_RDD_FIELD_SHIFT( 30,  1, (v) )
#define RDD_PCI_TX_DESCRIPTOR_SSID_L_WRITE( v )              LILAC_RDD_FIELD_SHIFT( 26,  4, (v) )
#define RDD_PCI_TX_DESCRIPTOR_SRC_PORT_L_READ( p )           FIELD_GET( *( (uint32_t *)p + 0), 14,  5 )
#define RDD_PCI_TX_DESCRIPTOR_PACKET_LENGTH_L_READ( p )      FIELD_GET( *( (uint32_t *)p + 0),  0, 14 )
#define RDD_PCI_TX_DESCRIPTOR_SKB_INDEX_L_READ( p )          FIELD_GET( *( (uint32_t *)p + 1),  0, 8 )
#define RDD_PCI_TX_DESCRIPTOR_BUFFER_NUMBER_L_READ( p )      FIELD_GET( *( (uint32_t *)p + 1),  0, 14 )
#define RDD_PCI_TX_DESCRIPTOR_ABS_FLAG_L_READ( p )           FIELD_GET( *( (uint32_t *)p + 1), 31,  1 )
#define RDD_PCI_TX_DESCRIPTOR_SSID_MULTICAST_L_READ( p )     FIELD_GET( *( (uint32_t *)p + 1), 30,  1 )
#define RDD_PCI_TX_DESCRIPTOR_SSID_L_READ( p )               FIELD_GET( *( (uint32_t *)p + 1), 26,  4 )
#define RDD_PCI_TX_DESCRIPTOR_HEADER_NUMBER_L_READ( p )      FIELD_GET( *( (uint32_t *)p + 1), 23,  3 )
#define RDD_PCI_TX_DESCRIPTOR_PAYLOAD_OFFSET_L_READ( p )     FIELD_GET( *( (uint32_t *)p + 1), 16,  7 )
#define RDD_PCI_TX_DESCRIPTOR_ABS_DST_SSID_L_READ( p )       FIELD_GET( *( (uint32_t *)p + 1),  8,  4 )



typedef struct
{
    uint16_t  entry[ LILAC_RDD_IPTV_SSM_CONTEXT_ENTRY_COUNT ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ BL_LILAC_RDD_CONTEXT_TABLES_FREE_LIST_DTS;

#define RDD_CONTEXT_TABLES_FREE_LIST_ENTRY_READ( r, p )    MREAD_16( p, r )
#define RDD_CONTEXT_TABLES_FREE_LIST_ENTRY_WRITE( v, p )   MWRITE_16( p, v )



typedef struct
{
    BL_LILAC_RDD_FILTER_MODE_DTE  entry[ 9 ][ LILAC_RDD_NUMBER_OF_SUBNETS ][ LILAC_RDD_NUMBER_OF_ETHER_TYPE_FILTERS ];
}
BL_LILAC_RDD_ETHER_TYPE_FILTER_MATRIX_DTS;



typedef struct
{
    uint8_t  vlan_action;
    uint8_t  pbits_action;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_VLAN_ACTION_ENTRY_DTS;


typedef struct
{
    RDD_VLAN_ACTION_ENTRY_DTS  entry[ LILAC_RDD_VLAN_TYPES ][ rdd_max_vlan_command ][ rdd_max_pbits_command ];
}
BL_LILAC_RDD_VLAN_ACTIONS_MATRIX_DTS;



#define LILAC_RDD_INGRESS_RATE_LIMITER_MAX_ALLOCATED_BUDGET                            ( ( 1 << 17 ) - 1)



/* software version */
typedef struct
{
    /* code */
    uint8_t   code;

    /* version */
    uint8_t   version;

    /* patch */
    uint8_t   patch;

    /* revision */
    uint8_t   revision;
}
BL_LILAC_RDD_VERSION_DTS;




/****************************** Enumeration ***********************************/

typedef enum
{
    LILAC_RDD_CPU_TX_MESSAGE_DDR_HEADROOM_SIZE_SET                = 0,
    LILAC_RDD_CPU_TX_MESSAGE_RX_FLOW_PM_COUNTERS_GET              = 1,
    LILAC_RDD_CPU_TX_MESSAGE_TX_FLOW_PM_COUNTERS_GET              = 2,
    LILAC_RDD_CPU_TX_MESSAGE_FLOW_PM_COUNTERS_GET                 = 3,
    LILAC_RDD_CPU_TX_MESSAGE_BRIDGE_PORT_PM_COUNTERS_GET          = 4,
    LILAC_RDD_CPU_TX_MESSAGE_FLUSH_GPON_QUEUE                     = 5,
	LILAC_RDD_CPU_TX_MESSAGE_LAG_PORT_GET                         = 5,
    LILAC_RDD_CPU_TX_MESSAGE_GLOBAL_REGISTERS_GET                 = 6,
    LILAC_RDD_CPU_TX_MESSAGE_SEND_XON_FRAME                       = 7,
    LILAC_RDD_CPU_TX_MESSAGE_IPTV_MAC_COUNTER_GET                 = 7,
    LILAC_RDD_CPU_TX_MESSAGE_INVALIDATE_CONTEXT_INDEX_CACHE_ENTRY = 8,
    LILAC_RDD_CPU_TX_MESSAGE_RING_DESTROY                         = 9,
    LILAC_RDD_CPU_TX_MESSAGE_IPV6_CRC_GET                         = 9,
    LILAC_RDD_CPU_TX_MESSAGE_GLOBAL_REGISTER_SET                  = 10,
    LILAC_RDD_CPU_TX_MESSAGE_PM_COUNTER_GET                       = 11,
    LILAC_RDD_CPU_TX_MESSAGE_FLUSH_ETH_QUEUE                      = 12,
    LILAC_RDD_CPU_TX_MESSAGE_ACTIVATE_TCONT                       = 13,
    LILAC_RDD_CPU_TX_MESSAGE_MIRRORING_MODE_CONFIG                = 14,
    LILAC_RDD_CPU_TX_MESSAGE_DHD_MESSAGE                          = 14,
    LILAC_RDD_CPU_TX_MESSAGE_RELEASE_SKB_BUFFERS                  = 15,
}
LILAC_RDD_CPU_TX_MESSAGE_TYPE;


typedef enum
{
    LILAC_RDD_MAC_HASH_TYPE_INCREMENTAL = 0,
    LILAC_RDD_MAC_HASH_TYPE_CRC16       = 1,
}
LILAC_RDD_MAC_TABLE_HASH_TYPE;


typedef enum
{
    LILAC_RDD_PARSER_LAYER2_PROTOCOL_1_HOT_UNKNOWN         = 0,
    LILAC_RDD_PARSER_LAYER2_PROTOCOL_1_HOT_PPPOE_DISCOVERY = 1,
    LILAC_RDD_PARSER_LAYER2_PROTOCOL_1_HOT_PPPOE_SESSION   = 2,
    LILAC_RDD_PARSER_LAYER2_PROTOCOL_1_HOT_IPOE            = 4,
    LILAC_RDD_PARSER_LAYER2_PROTOCOL_1_HOT_USER_DEFINED_0  = 8,
    LILAC_RDD_PARSER_LAYER2_PROTOCOL_1_HOT_USER_DEFINED_1  = 16,
    LILAC_RDD_PARSER_LAYER2_PROTOCOL_1_HOT_USER_DEFINED_2  = 32,
    LILAC_RDD_PARSER_LAYER2_PROTOCOL_1_HOT_USER_DEFINED_3  = 64,
}
LILAC_RDD_PARSER_LAYER2_PROTOCOL_1_HOT_DTE;


typedef enum
{
    LILAC_RDD_FARWARDING_DIRECTION_UPSTREAM =   0,
    LILAC_RDD_FARWARDING_DIRECTION_DOWNSTREAM = 1,
}
LILAC_RDD_BRIDGE_FORWARDING_DIRECTION_DTS;



typedef enum
{
    CS_R8  = 0,
    CS_R9  = 1,
    CS_R10 = 2,
    CS_R11 = 4,
    CS_R12 = 5,
    CS_R13 = 6,
    CS_R14 = 8,
    CS_R15 = 9,
    CS_R16 = 10,
    CS_R17 = 12,
    CS_R18 = 13,
    CS_R19 = 14,
    CS_R20 = 16,
    CS_R21 = 17,
    CS_R22 = 18,
    CS_R23 = 20,
    CS_R24 = 21,
    CS_R25 = 22,
    CS_R26 = 24,
    CS_R27 = 25,
    CS_R28 = 26,
    CS_R29 = 28,
    CS_R30 = 29,
    CS_R31 = 30,
}
LILAC_RDD_LOCAL_REGISTER_INDEX_DTS;


typedef enum
{
    FAST_RUNNER_A = 0,
    FAST_RUNNER_B = 1,
    PICO_RUNNER_A = 2,
    PICO_RUNNER_B = 3,
}
LILAC_RDD_RUNNER_INDEX_DTS;



/**** ingress classification ****/

typedef struct
{
    uint32_t    valid;
    int32_t     priority;
    uint32_t    rule_type;
    uint32_t    next_rule_cfg_id;
    uint32_t    next_group_id;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_INGRESS_CLASSIFICATION_RULE_CFG_DTE;


typedef struct
{
    uint32_t                                 first_rule_cfg_id;
    uint32_t                                 first_gen_filter_rule_cfg_id;
    RDD_INGRESS_CLASSIFICATION_RULE_CFG_DTE  rule_cfg[ 16 ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTE;



/**** Router definitions ****/

#define LILAC_RDD_CONNECTION_ENTRY_SIZE                 16
#define LILAC_RDD_CONTEXT_ENTRY_SIZE                    64
#define LILAC_RDD_CONNECTION_TABLE_SET_SIZE             4
#define LILAC_RDD_RESERVED_CONTEXT_ENTRIES              128

#ifndef RDD_BASIC
typedef struct
{
    rdpa_ip_flow_key_t           *lookup_entry;
    rdd_fc_context_t  context_entry;
    uint32_t                     xo_entry_index;
}
__PACKING_ATTRIBUTE_STRUCT_END__ BL_LILAC_RDD_ADD_CONNECTION_DTE;
#endif


typedef enum
{
    BL_LILAC_RDD_CONNECTION_ACTION_FORWARD     = 0,
    BL_LILAC_RDD_CONNECTION_ACTION_DROP        = 1,
    BL_LILAC_RDD_CONNECTION_ACTION_CPU_TRAP    = 2,
    BL_LILAC_RDD_CONNECTION_ACTION_MIPS_D_TRAP = 3,
}
BL_LILAC_RDD_CONNECTION_FORWARD_ACTION_DTE;


/* smart card */
typedef enum
{
	BL_LILAC_RDD_SMART_CARD_TASK_TYPE_NORMAL = 0,
    BL_LILAC_RDD_SMART_CARD_TASK_TYPE_ATR    = 1,
	BL_LILAC_RDD_SMART_CARD_TASK_TYPE_TX     = 6,
	BL_LILAC_RDD_SMART_CARD_TASK_TYPE_RX     = 7,
	BL_LILAC_RDD_SMART_CARD_TASK_TYPE_PPS    = 9,
}
BL_LILAC_RDD_SMART_CARD_TASK_TYPE_DTE;


typedef enum
{
	 BL_LILAC_RDD_SMART_CARD_STATUS_OK                         	= 1,
	 BL_LILAC_RDD_SMART_CARD_STATUS_WT_EXCEEDED                	= 2,
	 BL_LILAC_RDD_SMART_CARD_STATUS_MAX_ERROR_EXCEEDED_SEND    	= 3,
	 BL_LILAC_RDD_SMART_CARD_STATUS_MAX_ERROR_EXCEEDED_RECIEVE 	= 4,
	 BL_LILAC_RDD_SMART_CARD_STATUS_UNKNOWN_TASK_TYPE          	= 5,
	 BL_LILAC_RDD_SMART_CARD_STATUS_ILLEGAL_SW                 	= 6,
}
BL_LILAC_RDD_SMART_CARD_STATUS_BYTE_DTE;

#endif /*_BL_LILAC_DRV_RUNNER_DATA_STRUCTURES_H */

