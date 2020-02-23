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
#define LILAC_RDD_PATCH                                     ( 0x02 )
#define LILAC_RDD_REVISION                                  ( 0x01 )

#define RDP_CFG_BUF_SIZE_2K                                 0
#define RDP_CFG_BUF_SIZE_4K                                 1
#define RDP_CFG_BUF_SIZE_16K                                2
#define RDP_CFG_BUF_SIZE_2_5K                               7


#if defined(WL4908) && defined(CONFIG_BCM_JUMBO_FRAME)
#define LILAC_RDD_RUNNER_PACKET_BUFFER_SIZE                 512 /* FPM token size */
#elif defined(WL4908)
#define LILAC_RDD_RUNNER_PACKET_BUFFER_SIZE                 256 /* FPM token size */
#elif defined(DSL_63138) && defined(CONFIG_BCM_JUMBO_FRAME)
#define LILAC_RDD_RUNNER_PACKET_BUFFER_SIZE                 2560
#define RDP_CFG_BUF_SIZE_VALUE RDP_CFG_BUF_SIZE_2_5K
#elif defined(DSL_63148) && defined(CONFIG_BCM_JUMBO_FRAME)
#define LILAC_RDD_RUNNER_PACKET_BUFFER_SIZE                 4096
#define RDP_CFG_BUF_SIZE_VALUE RDP_CFG_BUF_SIZE_4K
#else
#define LILAC_RDD_RUNNER_PACKET_BUFFER_SIZE                 2048
#define RDP_CFG_BUF_SIZE_VALUE RDP_CFG_BUF_SIZE_2K
#endif
#define RDD_SIMULATION_PACKET_BUFFER_SIZE                   2048
#define LILAC_RDD_RUNNER_EXTENSION_BUFFER_SIZE              512
#define LILAC_RDD_PACKET_DDR_OFFSET                         18
#define LILAC_RDD_RUNNER_PSRAM_BUFFER_SIZE                  128
#define LILAC_RDD_RUNNER_EXTENSION_PACKET_BUFFER_SIZE       64
#define LILAC_RDD_RUNNER_EXTENSION_PACKET_HEADER_SIZE       32

/* RX - TX */
#define LILAC_RDD_RATE_CONTROLLERS_BUDGET_SET_SIZE          16
#define RDD_EMAC_NUMBER_OF_QUEUES                           8
#define LILAC_RDD_EMAC_EGRESS_COUNTER_OFFSET                0
#define LILAC_RDD_EMAC_RATE_SHAPER_GROUPS_STATUS_OFFSET     8
#define RDD_RATE_CONTROL_EXPONENT0                          1
#define RDD_RATE_CONTROL_EXPONENT1                          4
#define RDD_RATE_CONTROL_EXPONENT2                          7
#define RDD_RATE_CONTROL_EXPONENT_NUM                       3

/* CPU-RX table & queues */

/* CPU-TX table */
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
#define LILAC_RDD_CPU_TX_COMMAND_SPDSVC_PACKET              4
#define LILAC_RDD_CPU_TX_COMMAND_MESSAGE                    7

/* Bridging */
#define LILAC_RDD_NUMBER_OF_BRIDGE_PORTS                    15
#define LILAC_RDD_FLOW_CLASSIFICATION_ENTRY_STOP            0xFFFF
#define LILAC_RDD_MAX_PBITS                                 7
#define LILAC_RDD_NUMBER_OF_ETHER_TYPE_FILTERS              12

/* Ingress Classification */
#define RDD_INGRESS_CLASSIFICATION_SEARCH_HOP               BL_LILAC_RDD_MAC_TABLE_MAX_HOP_4
#define RDD_INGRESS_CLASSIFICATION_SEARCH_DEPTH             (1 << RDD_INGRESS_CLASSIFICATION_SEARCH_HOP)

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
#define LILAC_RDD_IH_WAN_BRIDGE_LOW_CLASS                   9
#define LILAC_RDD_IH_LAN_EMAC0_CLASS                        10
#define LILAC_RDD_IH_LAN_EMAC1_CLASS                        11
#define LILAC_RDD_IH_LAN_EMAC2_CLASS                        12
#define LILAC_RDD_IH_LAN_EMAC3_CLASS                        13
#define LILAC_RDD_IH_LAN_EMAC4_CLASS                        14

/* MAC Table */
#define LILAC_RDD_MAC_CONTEXT_MULTICAST                     0x1
#define LILAC_RDD_MAC_CONTEXT_ENTRY_TYPE_MASK               ( 1 << 6 )

/* Router Tables */
#define LILAC_RDD_NUMBER_OF_SUBNETS                         3

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

/* Firewall */
#define LILAC_RDD_FIREWALL_RULES_MASK_MAX_LENGTH            32

/* PCI */
#define LILAC_RDD_PCI_TX_NUMBER_OF_FIFOS                    4
#define LILAC_RDD_PCI_TX_FIFO_SIZE                          8

/* CRC */
#define RDD_CRC_TYPE_16                                     0
#define RDD_CRC_TYPE_32                                     1

/* GPIO */
#define RDD_GPIO_IO_ADDRESS                                 0x148

/* -Etc- */
#define LILAC_RDD_TRUE                                      1
#define LILAC_RDD_FALSE                                     0
#define LILAC_RDD_ON                                        1
#define LILAC_RDD_OFF                                       0

/* CPU TX */
#define LILAC_RDD_CPU_TX_SKB_INDEX_OWNERSHIP_BIT_MASK       0x8000
#define LILAC_RDD_CPU_TX_SKB_INDEX_MASK                     0x3FFF
#define LILAC_RDD_CPU_TX_SKB_LIMIT_MIN                      256
#define LILAC_RDD_CPU_TX_SKB_LIMIT_MAX                      16384
#define LILAC_RDD_CPU_TX_SKB_LIMIT_MULTIPLICATION           8

#define RDD_CLEAR_REGISTER( v )                             ( *( ( uint32_t *) v ) = 0 )

#define ADDRESS_OF(runner, task_name)                       runner##_##task_name

#define ROUND_UP_TO_BITS(_val, _bit)      (((_val) + (1 << _bit) - 1) & ~((1 << _bit) - 1))
#define ROUND_UP_MB(_n)                   ROUND_UP_TO_BITS(_n, 20ul)

/* DHD Backup Queues memory size.  This is placed at the end of flow memory. */
/* Currently Flow Memory contains Flow Tables + MultiCast Memory (4908 only) + DHD Backup Queue memory. */
/* Note that RDPA_DHD_BACKUP_QUEUE_RESERVED_SIZE must be a multiple of megabytes */
#ifdef RDPA_DHD_HELPER_FEATURE_BACKUP_QUEUE_SUPPORT
#define RDPA_DHD_BACKUP_QUEUE_RESERVED_SIZE        0x0100000
#else
#define RDPA_DHD_BACKUP_QUEUE_RESERVED_SIZE        0
#endif

#if defined(WL4908)
#define RDP_NATC_CONTEXT_TABLE_SIZE       (sizeof(RDD_NATC_CONTEXT_TABLE_DTS))
#define RDP_NATC_CONTEXT_TABLE_ADDR       (NATC_CONTEXT_TABLE_ADDRESS)

#define RDP_CONTEXT_CONTINUATION_TABLE_SIZE  (sizeof(RDD_CONTEXT_CONTINUATION_TABLE_DTS))
#define RDP_CONTEXT_CONTINUATION_TABLE_ADDR  (RDP_NATC_CONTEXT_TABLE_ADDR + ROUND_UP_TO_BITS(RDP_NATC_CONTEXT_TABLE_SIZE, 21))

#define RDP_NATC_KEY_TABLE_SIZE           (sizeof(RDD_NAT_CACHE_TABLE_DTS) + sizeof(RDD_NAT_CACHE_EXTENSION_TABLE_DTS))
#define RDP_NATC_KEY_TABLE_ADDR           (RDP_CONTEXT_CONTINUATION_TABLE_ADDR + ROUND_UP_MB(RDP_CONTEXT_CONTINUATION_TABLE_SIZE))

#define RDP_DDR_DATA_STRUCTURES_SIZE      (RDP_NATC_KEY_TABLE_ADDR + ROUND_UP_MB(RDP_NATC_KEY_TABLE_SIZE))

#ifdef CONFIG_BCM_RDPA_MCAST
#define RDP_DDR_MC_HEADER_SIZE            0x2000000
#else
#define RDP_DDR_MC_HEADER_SIZE            0
#endif
#else // if defined(WL4908) else DSL_63138
#define RDP_DDR_DATA_STRUCTURES_SIZE      (sizeof(RDD_CONNECTION_TABLE_DTS) * 2 + ROUND_UP_MB(sizeof(RDD_CONTEXT_TABLE_DTS)))
#endif

#if !defined(FIRMWARE_INIT)
/* DDR offsets */
#define DsConnectionTableBase              ( g_runner_tables_ptr + DS_CONNECTION_TABLE_ADDRESS )
#define UsConnectionTableBase              ( g_runner_tables_ptr + US_CONNECTION_TABLE_ADDRESS )
#endif

/******************************* Data structures ******************************/

#define LILAC_RDD_FIELD_SHIFT( ls_bit_number, field_width, write_value )    ( ( write_value & ( ( 1 << (field_width) ) - 1 ) ) << ( ls_bit_number ) )

#define RDD_EMAC_DESCRIPTOR_EGRESS_COUNTER_OFFSET  0

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


#define RDD_CPU_RX_DESCRIPTOR_SKB_INDEX_READ( r, p )            FIELD_MREAD_32( ( (uint8_t *)p + 4),  0,  8, r )
#define RDD_CPU_RX_DESCRIPTOR_REASON_L_READ( p )                FIELD_GET( *( (uint32_t *)p + 0), 25,  6 )
#define RDD_CPU_RX_DESCRIPTOR_PAYLOAD_OFFSET_FLAG_L_READ( p )   FIELD_GET( *( (uint32_t *)p + 0), 22,  1 )
#define RDD_CPU_RX_DESCRIPTOR_FLOW_ID_L_READ( p )               FIELD_GET( *( (uint32_t *)p + 0), 14,  8 )
#define RDD_CPU_RX_DESCRIPTOR_PACKET_LENGTH_L_READ( p )         FIELD_GET( *( (uint32_t *)p + 0),  0, 14 )
#define RDD_CPU_RX_DESCRIPTOR_SRC_PORT_L_READ( p )              FIELD_GET( *( (uint32_t *)p + 1), 28,  4 )
#define RDD_CPU_RX_DESCRIPTOR_NEXT_PTR_L_READ( p )              FIELD_GET( *( (uint32_t *)p + 1), 15, 13 )
#define RDD_CPU_RX_DESCRIPTOR_ABS_FLAG_L_READ( p )              FIELD_GET( *( (uint32_t *)p + 1), 14,  1 )
#if defined(DSL_63138)
#define RDD_CPU_RX_DESCRIPTOR_BUFFER_NUMBER_L_READ( p )         FIELD_GET( *( (uint32_t *)p + 1),  0, 15 )
#else
#define RDD_CPU_RX_DESCRIPTOR_BUFFER_NUMBER_L_READ( p )         FIELD_GET( *( (uint32_t *)p + 1),  0, 14 )
#endif
#define RDD_CPU_RX_DESCRIPTOR_SKB_INDEX_L_READ( p )             FIELD_GET( *( (uint32_t *)p + 1),  0,  8 )


#define RDD_CPU_TX_DESCRIPTOR_CONTEXT_INDEX_WRITE( v, p )      FIELD_MWRITE_32( ( (uint8_t *)p + 0), 0, 15, v )
#define RDD_CPU_TX_DESCRIPTOR_COUNTER_WRITE( v, p )            FIELD_MWRITE_32( ( (uint8_t *)p + 0), 0,  5, v )
#define RDD_CPU_TX_DESCRIPTOR_IPTV_MAC_IDX_WRITE( v, p )       FIELD_MWRITE_32( ( (uint8_t *)p + 0), 0, 11, v )
#define RDD_CPU_TX_DESCRIPTOR_COUNTER_4_BYTES_WRITE( v, p )    FIELD_MWRITE_32( ( (uint8_t *)p + 0), 5,  1, v )
#define RDD_CPU_TX_DESCRIPTOR_SRC_BRIDGE_PORT_WRITE( v, p )    FIELD_MWRITE_32( ( (uint8_t *)p + 0), 14, 5, v )
#define RDD_CPU_TX_DESCRIPTOR_QUEUE_WRITE( v, p )              FIELD_MWRITE_32( ( (uint8_t *)p + 0), 19, 3, v )
#define RDD_CPU_TX_DESCRIPTOR_GROUP_WRITE( v, p )              FIELD_MWRITE_32( ( (uint8_t *)p + 0), 19, 7, v )
#define RDD_CPU_TX_DESCRIPTOR_FLOW_WRITE( v, p )               FIELD_MWRITE_32( ( (uint8_t *)p + 0), 19, 9, v )
#define RDD_CPU_TX_DESCRIPTOR_EMAC_WRITE( v, p )               FIELD_MWRITE_32( ( (uint8_t *)p + 0), 22, 4, v )
#define RDD_CPU_TX_DESCRIPTOR_INTERRUPT_NUMBER_WRITE( v, p )   FIELD_MWRITE_32( ( (uint8_t *)p + 0), 22, 4, v )
#define RDD_CPU_TX_DESCRIPTOR_TCONT_INDEX_WRITE( v, p )        FIELD_MWRITE_32( ( (uint8_t *)p + 4), 22, 6, v )
#define RDD_CPU_TX_DESCRIPTOR_TX_QUEUE_PTR_WRITE( v, p )       FIELD_MWRITE_32( ( (uint8_t *)p + 4), 23, 9, v )
#define RDD_CPU_TX_DESCRIPTOR_MESSAGE_PARAMETER_WRITE( v, p )  FIELD_MWRITE_32( ( (uint8_t *)p + 4), 23, 5, v )

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
#define RDD_CPU_TX_DESCRIPTOR_MESSAGE_PARAMETER_L_WRITE( v )   LILAC_RDD_FIELD_SHIFT( 23,  5, (v) )
#define RDD_CPU_TX_DESCRIPTOR_IH_CLASS_L_WRITE( v )            LILAC_RDD_FIELD_SHIFT( 23,  4, (v) )
#define RDD_CPU_TX_DESCRIPTOR_PAYLOAD_OFFSET_L_WRITE( v )      LILAC_RDD_FIELD_SHIFT( 16,  7, (v) )
#define RDD_CPU_TX_DESCRIPTOR_1588_INDICATION_L_WRITE( v )     LILAC_RDD_FIELD_SHIFT( 27,  1, (v) )
#define RDD_CPU_TX_DESCRIPTOR_TCONT_INDEX_L_WRITE( v )         LILAC_RDD_FIELD_SHIFT(  8,  6, (v) )
#define RDD_CPU_TX_DESCRIPTOR_ABS_FLAG_L_WRITE( v )            LILAC_RDD_FIELD_SHIFT( 31,  1, (v) )
#define RDD_CPU_TX_DESCRIPTOR_GSO_PKT_L_WRITE( v )             LILAC_RDD_FIELD_SHIFT( 27,  1, (v) )
#if defined(DSL_63138)
#define RDD_CPU_TX_DESCRIPTOR_BUFFER_NUMBER_L_WRITE( v )       LILAC_RDD_FIELD_SHIFT(  0, 15, (v) )
#else
#define RDD_CPU_TX_DESCRIPTOR_BUFFER_NUMBER_L_WRITE( v )       LILAC_RDD_FIELD_SHIFT(  0, 14, (v) )
#endif
#define RDD_CPU_TX_DESCRIPTOR_SKB_INDEX_L_WRITE( v )           LILAC_RDD_FIELD_SHIFT(  0,  8, (v) )
#define RDD_CPU_TX_DESCRIPTOR_FLOW_L_WRITE( v )                LILAC_RDD_FIELD_SHIFT( 19,  9, (v) )
#define RDD_CPU_TX_DESCRIPTOR_IPTV_MAC_IDX_L_WRITE( v )        LILAC_RDD_FIELD_SHIFT(  0, 11, (v) )
#define RDD_CPU_TX_DESCRIPTOR_GROUP_L_WRITE( v )               LILAC_RDD_FIELD_SHIFT( 19,  7, (v) )
#define RDD_CPU_TX_DESCRIPTOR_COUNTER_L_WRITE( v )             LILAC_RDD_FIELD_SHIFT(  0,  5, (v) )
#define RDD_CPU_TX_DESCRIPTOR_COUNTER_4_BYTES_L_WRITE( v )     LILAC_RDD_FIELD_SHIFT(  5,  1, (v) )
#define RDD_CPU_TX_DESCRIPTOR_MSG_TYPE_L_WRITE( v )            LILAC_RDD_FIELD_SHIFT(  0,  4, (v) )


#define LILAC_RDD_INGRESS_RATE_LIMITER_MAX_ALLOCATED_BUDGET    ( ( 1 << 17 ) - 1)


typedef struct
{
    BL_LILAC_RDD_FILTER_MODE_DTE  entry[ 9 ][ LILAC_RDD_NUMBER_OF_SUBNETS ][ LILAC_RDD_NUMBER_OF_ETHER_TYPE_FILTERS ];
}
BL_LILAC_RDD_ETHER_TYPE_FILTER_MATRIX_DTS;


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


#if defined(WL4908)
#define RDD_FLOW_ENTRIES_SIZE 16512
#define RDD_FLOW_ENTRY_VALID 0x80000000
#define RDD_RESERVED_CONTEXT_ENTRIES 128
#define RDD_CONTEXT_TABLE_SIZE RDD_FLOW_ENTRIES_SIZE
#endif /* WL4908 */


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
    LILAC_RDD_CPU_TX_MESSAGE_IPTV_MAC_COUNTER_GET                 = 7,
    LILAC_RDD_CPU_TX_MESSAGE_INVALIDATE_CONTEXT_INDEX_CACHE_ENTRY = 8,
    LILAC_RDD_CPU_TX_MESSAGE_RING_DESTROY                         = 9,
    LILAC_RDD_CPU_TX_MESSAGE_IPV6_CRC_GET                         = 10,
    LILAC_RDD_CPU_TX_MESSAGE_PM_COUNTER_GET                       = 11,
    LILAC_RDD_CPU_TX_MESSAGE_FLUSH_ETH_QUEUE                      = 12,
    LILAC_RDD_CPU_TX_MESSAGE_UPDATE_US_PD_POOL_QUOTA              = 12,
    LILAC_RDD_CPU_TX_MESSAGE_ACTIVATE_TCONT                       = 13,
    LILAC_RDD_CPU_TX_MESSAGE_UPDATE_PD_POOL_QUOTA                 = 13,
    LILAC_RDD_CPU_TX_MESSAGE_MIRRORING_MODE_CONFIG                = 14,
    LILAC_RDD_CPU_TX_MESSAGE_DHD_MESSAGE                          = 14,
    LILAC_RDD_CPU_TX_MESSAGE_RELEASE_SKB_BUFFERS                  = 15,
}
LILAC_RDD_CPU_TX_MESSAGE_TYPE;

#define RDD_CPU_TX_MESSAGE_DHD_MESSAGE LILAC_RDD_CPU_TX_MESSAGE_DHD_MESSAGE

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
#define LILAC_RDD_CONTEXT_ENTRY_SIZE                    sizeof(RDD_CONTEXT_ENTRY_UNION_DTS)
#define LILAC_RDD_CONNECTION_TABLE_SET_SIZE             4
#define LILAC_RDD_RESERVED_CONTEXT_ENTRIES              128

#ifndef RDD_BASIC
typedef struct
{
    rdpa_ip_flow_key_t           *lookup_entry;
    rdpa_l2_flow_key_t           *l2_lookup_entry;
    RDD_CONTEXT_ENTRY_UNION_DTS  context_entry;
    uint32_t                     xo_entry_index;
}
__PACKING_ATTRIBUTE_STRUCT_END__ BL_LILAC_RDD_ADD_CONNECTION_DTE;
#endif


#define PARSER_LAYER4_PROTOCOL_TCP                      1
#define PARSER_LAYER4_PROTOCOL_UDP                      2

/* Offsets must correspond to current rdd_data_structures_auto.h number_of_ports and port_mask offsets. */
#define RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_NUM_PORTS_PORT_MASK_WRITE( v, p ) FIELD_MWRITE_16((uint8_t *)p + 8, 0, 12, v )

typedef struct
{
	uint32_t	good_csum_packets        	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	no_csum_packets          	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bad_ipv4_hdr_csum_packets	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bad_tcp_udp_csum_packets 	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CSO_COUNTERS_ENTRY_DTS;

#define rdd_phys_addr_t bdmf_phys_addr_t 

#endif /*_BL_LILAC_DRV_RUNNER_DATA_STRUCTURES_H */

