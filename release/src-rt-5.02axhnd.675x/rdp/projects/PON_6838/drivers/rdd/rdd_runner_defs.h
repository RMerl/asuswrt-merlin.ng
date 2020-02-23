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

#ifndef _BL_LILAC_DRV_RUNNER_DEFS_H
#define _BL_LILAC_DRV_RUNNER_DEFS_H

#define BPM_TRANSITION_STATE_ADDRESS                                ( BPM_REPLY_RUNNER_A_ADDRESS + 0x20 )

#define FIREWALL_CFG_REG_RULES_MAP_TABLE_ADDRESS                    ( FIREWALL_CONFIGURATION_REGISTER_ADDRESS )
#define FIREWALL_CFG_REG_RULES_TABLE_ADDRESS                        ( FIREWALL_CONFIGURATION_REGISTER_ADDRESS + 4 )

#define LOCAL_SWITCHING_MULTICAST_INGRESS_QUEUE_PTR_ADDRESS         ( PICO_RUNNER_GLOBAL_REGISTERS_INIT_ADDRESS + d.28 )

#define DMA_SYNCHRONIZATION_DUMMY_ADDRESS                           0xFFFF0000

#ifdef FIRMWARE_INIT
#define SIMULATOR_DDR_SKB_DATA_POINTERS_OFFSET                      0x01A00000
#define SIMULATOR_DDR_SKB_FREE_INDEXES_OFFSET                       0x01A10000
#define SIMULATOR_DDR_RING_OFFSET                                   0x01B00000
#define SIMULATOR_DDR_RING_HOST_BUFFERS_OFFSET                      0x01B00780
#define SIMULATOR_DDR_SKB_BUFFERS_OFFSET                            0x01B80000
#define SIMULATOR_DDR_RING_NUM_OF_ENTRIES                           10
#endif

#define BCR_WAN_MISS_ETH_FLOW_TABLE_ADDRESS                         ( BRIDGE_CONFIGURATION_REGISTER_ADDRESS + 0  )
#define BCR_WAN_UNTAGGED_ETH_FLOW_TABLE_ADDRESS                     ( BRIDGE_CONFIGURATION_REGISTER_ADDRESS + 1  )
#define BCR_PCI_FLOW_CACHE_MODE_ADDRESS                             ( BRIDGE_CONFIGURATION_REGISTER_ADDRESS + 3  )
#define BCR_SUBNET_CLASSIFICATION_MODE_ADDRESS                      ( BRIDGE_CONFIGURATION_REGISTER_ADDRESS + 4  )
#define BCR_INGRESS_GLOBAL_CONFIG_ADDRESS                           ( BRIDGE_CONFIGURATION_REGISTER_ADDRESS + 5  )
#define BCR_DROP_PRECEDENCE_ELIGIBILITY_VECTOR_ADDRESS              ( BRIDGE_CONFIGURATION_REGISTER_ADDRESS + 6  )
#define BCR_UNKNOWN_SA_MAC_COMMAND_ADDRESS                          ( BRIDGE_CONFIGURATION_REGISTER_ADDRESS + 8  )
#define BCR_US_UNKNOWN_DA_MAC_FLOODING_BRIDGE_PORT_ADDRESS          ( BRIDGE_CONFIGURATION_REGISTER_ADDRESS + 17 )
#define BCR_UNKNOWN_DA_MAC_COMMAND_ADDRESS                          ( BRIDGE_CONFIGURATION_REGISTER_ADDRESS + 18 )
#define BCR_BROADCOM_SWITCH_PORT                                    ( BRIDGE_CONFIGURATION_REGISTER_ADDRESS + 25 )
#define BCR_FLOODING_BRIDGE_PORTS_VECTOR_ADDRESS                    ( BRIDGE_CONFIGURATION_REGISTER_ADDRESS + 27 )
#define BCR_1ST_EGRESS_ETHER_TYPE_ADDRESS                           ( BRIDGE_CONFIGURATION_REGISTER_ADDRESS + 28 )
#define BCR_2ND_EGRESS_ETHER_TYPE_ADDRESS                           ( BRIDGE_CONFIGURATION_REGISTER_ADDRESS + 30 )
#define BCR_3RD_EGRESS_ETHER_TYPE_ADDRESS                           ( BRIDGE_CONFIGURATION_REGISTER_ADDRESS + 32 )
#define BCR_UPSTREAM_RATE_CONTROLLER_TIMER_ADDRESS                  ( BRIDGE_CONFIGURATION_REGISTER_ADDRESS + 34 )
#define BCR_LS_DROP_PRECEDENCE_ELIGIBILITY_VECTOR_ADDRESS           ( BRIDGE_CONFIGURATION_REGISTER_ADDRESS + 44 )
#define BCR_1588_MODE_ADDRESS                                       ( BRIDGE_CONFIGURATION_REGISTER_ADDRESS + 46 )
#define BCR_IPTV_CLASSIFICATION_MODE_ADDRESS                        ( BRIDGE_CONFIGURATION_REGISTER_ADDRESS + 47 )
#define BCR_DS_CONNECTION_MISS_ACTION_FILTER_ADDRESS                ( BRIDGE_CONFIGURATION_REGISTER_ADDRESS + 48 )
#define BCR_IPV6_ENABLE_ADDRESS                                     ( BRIDGE_CONFIGURATION_REGISTER_ADDRESS + 50 )
#define BCR_HASH_BASED_FORWARDING_PORT_COUNT_ADDRESS                ( BRIDGE_CONFIGURATION_REGISTER_ADDRESS + 51 )
#define BCR_US_PADDING_MAX_SIZE_ADDRESS                             ( BRIDGE_CONFIGURATION_REGISTER_ADDRESS + 52 )
#define BCR_US_PADDING_CPU_MAX_SIZE_ADDRESS                         ( BRIDGE_CONFIGURATION_REGISTER_ADDRESS + 54 )
#define BCR_ACTIVE_POLICERS_VECTOR_ADDRESS                          ( BRIDGE_CONFIGURATION_REGISTER_ADDRESS + 56 )
#define BCR_MIRORRING_MODE_ADDRESS                                  ( BRIDGE_CONFIGURATION_REGISTER_ADDRESS + 58 )
#define BCR_VLAN_BINDING_MODE_ADDRESS                               ( BRIDGE_CONFIGURATION_REGISTER_ADDRESS + 59 )
#define BCR_POLICERS_TIMER_ADDRESS                                  ( BRIDGE_CONFIGURATION_REGISTER_ADDRESS + 60 )
#define BCR_TIMER_SCHEDULER_TIMER_ADDRESS                           ( BRIDGE_CONFIGURATION_REGISTER_ADDRESS + 62 )
#define BCR_TPID_DETECT_VALUE_ADDRESS                               ( BRIDGE_CONFIGURATION_REGISTER_ADDRESS + 64 )
#define BCR_FLOODING_SSID_VECTOR_ADDRESS                            ( BRIDGE_CONFIGURATION_REGISTER_ADDRESS + 66 )
#define BCR_INTER_LAN_SCHEDULING_MODE_ADDRESS                       ( BRIDGE_CONFIGURATION_REGISTER_ADDRESS + 68 )
#define BCR_DS_INGRESS_POLICERS_MODE_ADDRESS                        ( BRIDGE_CONFIGURATION_REGISTER_ADDRESS + 70 )

#define PROFILING_BUFFER_MAIN_RUNNER_ADDRESS                        INGRESS_HANDLER_BUFFER_ADDRESS

#define EMAC_ABSOLUTE_TX_FIRMWARE_COUNTER_ADDRESS                   EMAC_ABSOLUTE_TX_BBH_COUNTER_ADDRESS

#define TCONT_NUMBER_OF_RATE_CONTROLLERS                            32

#define CPU_RX_NUMBER_OF_QUEUES                                     8

#define BL_LILAC_RDD_NUMBER_OF_DOWNSTREAM_ETH_FLOWS                 128

/* EPON */
#define CACHE_BASE_TAIL_ENTRY                                       8
#define CACHE_FIFO_HEAD_PD_NUMBER                                   8
#define CACHE_ENTRY_BYTE_SIZE	                                    8
#define TOTAL_CACHE_FIFO_PD_NUMBER                                  12
#define CACHE_FIFO_BYTE_SIZE                                        ( TOTAL_CACHE_FIFO_PD_NUMBER * CACHE_ENTRY_BYTE_SIZE )
#define CACHE_SLOT_PD_NUMBER                                        4
#define CACHE_FIFO_HEAD_SLOT_BYTE_SIZE                              ( CACHE_FIFO_HEAD_PD_NUMBER * CACHE_ENTRY_BYTE_SIZE )


#define BL_LILAC_RDD_PCI_INTERRUPT_NUMBER                           1
#define BL_LILAC_RDD_SMART_CARD_INTERRUPT_NUMBER                    3

#define UPSTREAM_RATE_LIMITER_ID                                    8

#if !defined(FIRMWARE_INIT)
#define TIMER_SCHEDULER_TASK_PERIOD                                     1000
#else
#define TIMER_SCHEDULER_TASK_PERIOD                                     100
#endif
#define BL_LILAC_RDD_US_RATE_CONTROL_TIMER_INTERVAL                     4000
#define UPSTREAM_INGRESS_RATE_LIMITER_BUDGET_ALLOCATOR_TIMER_INTERVAL   1000
#define POLICER_TIMER_PERIOD                                            16000
#define POLICER_EXPONENT                                                5
#define EMAC_RATE_LIMITER_TIMER_PERIOD                                  1000
#define EMAC_RATE_LIMITER_EXPONENT                                      3
#define SERVICE_QUEUE_RATE_LIMITER_TIMER_PERIOD                         4000
#define UPSTREAM_RATE_LIMITER_TIMER_PERIOD                              4000
#define UPSTREAM_RATE_LIMITER_EXPONENT                                  5
#define UPSTREAM_QUASI_POLICER_TIMER_PERIOD                             1000
#define UPSTREAM_QUASI_POLICER_EXPONENT                                 3
#define RDD_IPV6_HEADER_SIZE                                            40
#define BL_LILAC_RDD_DDR_PACKET_PAYLOAD_OFFSET                          18
#define INTERRUPT_COALESCING_TIMER_PERIOD                               100
#ifdef OREN
#define CMD_STREAM                                                      "STREAM........."
#define CMD_STREAM_LENGTH                                               15

#define SPDSVC_TIMER_PERIOD                                             100 /* usec */
#define SPDSVC_TIMER_HZ                                                 ( 1000000 / SPDSVC_TIMER_PERIOD ) /* sec */
#define SPDSVC_ETH_IFG                                                  20 /* bytes */
#define SPDSVC_ETH_CRC_LEN                                              4  /* bytes */
#define SPDSVC_ETH_OVERHEAD                                             (SPDSVC_ETH_CRC_LEN + SPDSVC_ETH_IFG) /* bytes */
                                                                        /* Ethernet packet + 2 VLAN Tags + PPPoE + Overhead */
#define SPDSVC_BUCKET_SIZE_MIN                                          (1514 + 8 + 8 + SPDSVC_ETH_OVERHEAD) /* bytes */
#endif
/* timer schedular primitive ids */

#define CPU_RX_METER_BUDGET_ALLOCATE_CODE_ID                            0 /* fast a + fast b */
#define UPSTREAM_RATE_LIMITER_BUDGET_ALLOCATE_CODE_ID                   1 /* fast b */
#define UPSTREAM_INGRESS_RATE_LIMITER_BUDGET_ALLOCATE_CODE_ID           2 /* pico b */
#define UPSTREAM_QUASI_BUDGET_ALLOCATE_CODE_ID                          3 /* pico b */
#define DOWNSTREAM_SERVICE_QUEUES_RATE_LIMITER_BUDGET_ALLOCATE_CODE_ID	3 /* pico a */
#define DOWNSTREAM_RATE_LIMITER_BUDGET_ALLOCATE_CODE_ID                 4 /* pico a */
#define DOWNSTREAM_DHD_TX_POST_CLOSE_AGGREGATION_CODE_ID                5 /* fast a */

#define MAC_UNKNOWN_DA_FORWARDING_FILTER_DISABLED                   0xFFFFFFFF
#define MAC_UNKNOWN_DA_FORWARDING_POLICER_ENABLE_BIT_OFFSET         29

#define WAN_FILTERS_AND_CLASSIFICATON_R8_CPU_INDICATION_OFFSET      15

#define GLOBAL_CFG_FLOW_CACHE_MODE_BIT_OFFSET                       0
#define GLOBAL_CFG_BRIDGE_FLOW_CACHE_MODE_BIT_OFFSET                1
#define GLOBAL_CFG_BROADCOM_SWITCH_MODE_BIT_OFFSET                  2
#define DS_GLOBAL_CFG_AE_MODE_BIT_OFFSET                            3
#define GLOBAL_CFG_EPON_MODE_BIT_OFFSET                             4
#define GLOBAL_CFG_CHIP_REVISION_OFFSET                             5
#define GLOBAL_CFG_MIRRORING_MODE_BIT_OFFSET                        6

/*  downstream global ingress configuration vector */
#define GLOBAL_INGRESS_CONFIG_MIRRORING                             0
#define GLOBAL_INGRESS_CONFIG_TUNNELING_ENABLE                      1
#define GLOBAL_INGRESS_CONFIG_FULL_FLOW_CACHE_MODE                  2
#define GLOBAL_INGRESS_CONFIG_IP_MULTICAST_FC_ACCELERATION          3
#define GLOBAL_INGRESS_CONFIG_NON_IP_FC_ACCELRATION                 4

#define CPU_TX_FAST_THREAD_NUMBER                                   0
#define CPU_RX_THREAD_NUMBER                                        1
#define TIMER_SCHEDULER_MAIN_THREAD_NUMBER                          4
#define POLICER_BUDGET_ALLOCATOR_THREAD_NUMBER                      5
#define CPU_TX_PICO_THREAD_NUMBER                                   32
#define TIMER_SCHEDULER_PICO_A_THREAD_NUMBER                        33
#define TIMER_SCHEDULER_PICO_B_THREAD_NUMBER                        34
#define CPU_TX_DESCRIPTOR_ADDRESS_MASK                              0xFF80

#define WAN_DIRECT_THREAD_NUMBER                                    7
#define WAN_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER                9
#ifdef G9991
#define G9991_FRAGMENT_THREAD_NUMBER                                10
#endif
#define DOWNSTREAM_FLOW_CACHE_SLAVE0_THREAD_NUMBER                  11
#define DOWNSTREAM_FLOW_CACHE_SLAVE1_THREAD_NUMBER                  12
#define DOWNSTREAM_FLOW_CACHE_SLAVE2_THREAD_NUMBER                  13
#define DOWNSTREAM_FLOW_CACHE_SLAVE3_THREAD_NUMBER                  14
#define DOWNSTREAM_CPU_FLOW_CACHE_THREAD_NUMBER                     15
#define DOWNSTREAM_VLAN_THREAD_NUMBER                               16
#define DHD_TX_COMPLETE_FAST_A_THREAD_NUMBER                        16
#define DHD1_TX_COMPLETE_FAST_A_THREAD_NUMBER                       17
#define DHD2_TX_COMPLETE_FAST_A_THREAD_NUMBER                       18
#define DHD_TX_POST_FAST_A_THREAD_NUMBER                            19
#define CPU_DS_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER             25
#define DOWNSTREAM_MULTICAST_THREAD_NUMBER                          28
#define GSO_PICO_A_THREAD_NUMBER                                    34
#define GSO_PICO_THREAD_NUMBER (GSO_PICO_A_THREAD_NUMBER)
#define LAN_REMOTE_SEARCH_THREAD_NUMBER                             34
#define WLAN_MCAST_THREAD_NUMBER                                    35
#define LOCAL_SWITCHING_LAN_ENQUEUE_THREAD_NUMBER                   36
#define DOWNSTREAM_LAN_ENQUEUE_THREAD_NUMBER                        37
#ifndef G9991
#define DOWNSTREAM_MULTICAST_LAN_ENQUEUE_THREAD_NUMBER              38
#else
#define DOWNSTREAM_MULTICAST_LAN_ENQUEUE_THREAD_NUMBER              40
#endif
#define LOCAL_SWITCHING_MULTICAST_LAN_ENQUEUE_THREAD_NUMBER         39
#define DOWNSTREAM_LAN_SERVICE_ENQUEUE_THREAD_NUMBER                40
#define PCI_TX_THREAD_NUMBER                                        41
#define ETH_TX_THREAD_NUMBER                                        42
#define ETH0_TX_THREAD_NUMBER                                       43
#define ETH1_TX_THREAD_NUMBER                                       44
#define ETH2_TX_THREAD_NUMBER                                       45
#define ETH3_TX_THREAD_NUMBER                                       46
#define ETH4_TX_THREAD_NUMBER                                       47

#define SMART_CARD_THREAD_NUMBER                                    2
#define RATE_CONTROLLER_BUDGET_ALLOCATOR_THREAD_NUMBER              3
#define EPON_TX_REQUEST_THREAD_NUMBER                               6
#define WAN_TX_THREAD_NUMBER                                        7
#define DHD_TX_POST_FAST_B_THREAD_NUMBER                            8
#define UPSTREAM_FLOW_CACHE_SLAVE0_THREAD_NUMBER                    10
#define UPSTREAM_FLOW_CACHE_SLAVE1_THREAD_NUMBER                    11
#define UPSTREAM_FLOW_CACHE_SLAVE2_THREAD_NUMBER                    12
#define UPSTREAM_FLOW_CACHE_SLAVE3_THREAD_NUMBER                    13
#define UPSTREAM_VLAN_BRIDGE_THREAD_NUMBER                          14
#define UPSTREAM_VLAN_ROUTER_THREAD_NUMBER                          15
#define SPEED_SERVICE_THREAD_NUMBER                                 16
#define WAN_INTERWORKING_THREAD_NUMBER                              22
#define LOCAL_SWITCHING_MULTICAST_THREAD_NUMBER                     28
#define WAN_TO_WAN_THREAD_NUMBER                                    31
#define LAN_DIRECT_TO_CPU_THREAD_NUMBER                             33
#define CPU_RX_INTERRUPT_COALESCING_THREAD_NUMBER                   35
#define DHD_RX_THREAD_NUMBER                                        36
#define DHD1_RX_THREAD_NUMBER                                       37
#define DHD2_RX_THREAD_NUMBER                                       38
#define UPSTREAM_FLOW_CACHE_MASTER_THREAD_NUMBER                    39
#define LAN0_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER               40
#define LAN1_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER               41
#define LAN2_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER               42
#define LAN3_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER               43
#define LAN4_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER               44
#define CPU_US_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER             45
#define LAN_DISPATCH_THREAD_NUMBER                                  46
#define UPSTREAM_FLOODING_THREAD_NUMBER                             47

#define CPU_RX_THREAD_WAKEUP_REQUEST_VALUE                          ( ( CPU_RX_THREAD_NUMBER << 4 ) + 1 )
#define CPU_TX_FAST_THREAD_WAKEUP_REQUEST_VALUE                     ( ( CPU_TX_FAST_THREAD_NUMBER << 4 ) + 1 )
#define SPEED_SERVICE_THREAD_WAKEUP_REQUEST_VALUE                   ( ( SPEED_SERVICE_THREAD_NUMBER << 4) + 1 )
#define DHD_TX_POST_FAST_A_THREAD_WAKEUP_REQUEST_VALUE              ( ( DHD_TX_POST_FAST_A_THREAD_NUMBER << 4 ) + 1 )
#define DHD_TX_POST_FAST_B_THREAD_WAKEUP_REQUEST_VALUE              ( ( DHD_TX_POST_FAST_B_THREAD_NUMBER << 4 ) + 1 )
#define DOWNSTREAM_FLOW_CACHE_WAKEUP_REQUEST_VALUE                  ( ( DOWNSTREAM_FLOW_CACHE_SLAVE0_THREAD_NUMBER << 4 ) + 1 )
#define DOWNSTREAM_CPU_FLOW_CACHE_THREAD_WAKEUP_REQUEST_VALUE       ( ( DOWNSTREAM_CPU_FLOW_CACHE_THREAD_NUMBER << 4 ) + 1 )
#define GSO_PICO_THREAD_WAKEUP_REQUEST_VALUE                        ( ( GSO_PICO_THREAD_NUMBER << 4 ) + 1 )
#define WLAN_MCAST_THREAD_WAKEUP_REQUEST_VALUE                      ( ( WLAN_MCAST_THREAD_NUMBER << 4 ) + 1 )
#define DOWNSTREAM_MULTICAST_THREAD_WAKEUP_REQUEST_VALUE            ( ( DOWNSTREAM_MULTICAST_THREAD_NUMBER << 4 ) + 1 )
#define DOWNSTREAM_VLAN_THREAD_WAKEUP_REQUEST_VALUE                 ( ( DOWNSTREAM_VLAN_THREAD_NUMBER << 4 ) + 1 )
#define DOWNSTREAM_LAN_ENQUEUE_THREAD_WAKEUP_REQUEST_VALUE          ( ( DOWNSTREAM_LAN_ENQUEUE_THREAD_NUMBER << 4 ) + 1 )
#define DOWNSTREAM_LAN_SERVICE_ENQUEUE_THREAD_WAKEUP_REQUEST_VALUE  ( ( DOWNSTREAM_LAN_SERVICE_ENQUEUE_THREAD_NUMBER << 4 ) + 1 )
#define LOCAL_SWITCHING_LAN_ENQUEUE_THREAD_WAKEUP_REQUEST_VALUE     ( ( LOCAL_SWITCHING_LAN_ENQUEUE_THREAD_NUMBER << 4 ) + 1 )
#define MULTICAST_LAN_ENQUEUE_THREAD_WAKEUP_REQUEST_VALUE           ( ( DOWNSTREAM_MULTICAST_LAN_ENQUEUE_THREAD_NUMBER << 4 ) + 1 )
#define LAN0_FILTERS_AND_CLASSIFICATION_WAKEUP_REQUEST_VALUE        ( ( LAN0_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER << 4 ) + 1 )
#define LAN1_FILTERS_AND_CLASSIFICATION_WAKEUP_REQUEST_VALUE        ( ( LAN1_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER << 4 ) + 1 )
#define LAN2_FILTERS_AND_CLASSIFICATION_WAKEUP_REQUEST_VALUE        ( ( LAN2_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER << 4 ) + 1 )
#define LAN3_FILTERS_AND_CLASSIFICATION_WAKEUP_REQUEST_VALUE        ( ( LAN3_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER << 4 ) + 1 )
#define LAN4_FILTERS_AND_CLASSIFICATION_WAKEUP_REQUEST_VALUE        ( ( LAN4_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER << 4 ) + 1 )
#define UPSTREAM_VLAN_BRIDGE_THREAD_WAKEUP_REQUEST_VALUE            ( ( UPSTREAM_VLAN_BRIDGE_THREAD_NUMBER << 4 ) + 1 )
#define UPSTREAM_VLAN_ROUTER_THREAD_WAKEUP_REQUEST_VALUE            ( ( UPSTREAM_VLAN_ROUTER_THREAD_NUMBER << 4 ) + 1 )
#define UPSTREAM_FLOW_CACHE_MASTER_THREAD_WAKEUP_REQUEST_VALUE      ( ( UPSTREAM_FLOW_CACHE_MASTER_THREAD_NUMBER << 4 ) + 1 )
#define WAN_INTERWORKING_THREAD_WAKEUP_REQUEST_VALUE                ( ( WAN_INTERWORKING_THREAD_NUMBER << 4 ) + 1 )
#define CPU_DS_CLASSIFICATION_THREAD_WAKEUP_REQUEST_VALUE           ( ( CPU_DS_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER << 4 ) + 1 )
#define CPU_US_CLASSIFICATION_THREAD_WAKEUP_REQUEST_VALUE           ( ( CPU_US_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER << 4 ) + 1 )
#define LOCAL_SWITCHING_MULTICAST_THREAD_WAKEUP_REQUEST_VALUE       ( ( LOCAL_SWITCHING_MULTICAST_THREAD_NUMBER << 4 ) + 1 )
#define CPU_TX_PICO_THREAD_WAKEUP_REQUEST_VALUE                     ( ( CPU_TX_PICO_THREAD_NUMBER << 4 ) + 1 )
#define DHD_RX_THREAD_WAKEUP_REQUEST_VALUE                          ( ( DHD_RX_THREAD_NUMBER << 4 ) + 1 )
#define DHD1_RX_THREAD_WAKEUP_REQUEST_VALUE                         ( ( DHD1_RX_THREAD_NUMBER << 4 ) + 1 )
#define DHD2_RX_THREAD_WAKEUP_REQUEST_VALUE                         ( ( DHD2_RX_THREAD_NUMBER << 4 ) + 1 )
#define UPSTREAM_FLOODING_THREAD_WAKEUP_REQUEST_VALUE               ( ( UPSTREAM_FLOODING_THREAD_NUMBER << 4 ) + 1 )
#ifdef G9991
#define G9991_FRAGMENT_THREAD_WAKEUP_REQUEST_VALUE                  ( ( G9991_FRAGMENT_THREAD_NUMBER << 4 ) + 1 )
#endif
#define CPU_RX_INTERRUPT_COALESCING_WAKEUP_REQUEST_VALUE            ( ( CPU_RX_INTERRUPT_COALESCING_THREAD_NUMBER << 4 ) + 1 )

#define LAN_FILTERS_LAN_TYPE_CPU_BIT_OFFSET                     31
#define LAN_FILTERS_LAN_TYPE_DIRECT_TO_CPU_BIT_OFFSET           30

#define SELF_FIRMWARE_WAKEUP_REQUEST_BIT                        2

#ifndef G9991
#define DOWNSTREAM_VALID_PACKETS_GROUP                          0
#define DOWNSTREAM_VALID_BYTES_GROUP                            16
#define DOWNSTREAM_DROPPED_GROUP                                32
#define UPSTREAM_VALID_PACKETS_GROUP                            40
#define UPSTREAM_VALID_BYTES_GROUP                              56
#define UPSTREAM_DROPPED_GROUP                                  72
#else
#define DOWNSTREAM_VALID_PACKETS_GROUP                          0
#define DOWNSTREAM_VALID_BYTES_GROUP                            8
#define DOWNSTREAM_DROPPED_GROUP                                16
#define UPSTREAM_VALID_PACKETS_GROUP                            20
#define UPSTREAM_VALID_BYTES_GROUP                              28
#define UPSTREAM_DROPPED_GROUP                                  36

#define DOWNSTREAM_VALID_PACKETS_GROUP_G9991_BC                 40
#define DOWNSTREAM_VALID_PACKETS_GROUP_G9991_MC                 42
#define DOWNSTREAM_VALID_PACKETS_GROUP_G9991                    44
#define DOWNSTREAM_VALID_BYTES_GROUP_G9991                      46
#define UPSTREAM_VALID_PACKETS_GROUP_G9991_BC                   48
#define UPSTREAM_VALID_PACKETS_GROUP_G9991_MC                   50
#define UPSTREAM_VALID_PACKETS_GROUP_G9991                      52
#define UPSTREAM_VALID_BYTES_GROUP_G9991                        54
#endif

#define INGRESS_RATE_LIMITER_GROUP                              80   
#define BRIDGE_FILTERED_GROUP                                   81
#define BRIDGE_TX_CONGESTION_GROUP                              82
#define WAN_BRIDGE_PORT_GROUP                                   83
#define CPU_RX_INTERRUPT_COALESCING_GROUP                       84
#ifdef G9991
#define G9991_GLOBAL_GROUP                                      85
#endif
#define UPSTREAM_VARIOUS_PACKETS_GROUP                          86
#define DOWNSTREAM_VARIOUS_PACKETS_GROUP                        88
#define SERVICE_QUEUE_PACKET_GROUP                              90
#define CPU_RX_METERS_DROPPED_PACKETS_GROUP                     92
#ifdef CONFIG_DHD_RUNNER
#define DHD_SSID_DROP_PACKET_GROUP                              93
#endif
#define SERVICE_QUEUE_DROP_PACKET_GROUP                         95

#define WAN_BRIDGED_RX_VALID_SUB_GROUP_OFFSET                   1
#define WAN_IPTV_RX_VALID_SUB_GROUP_OFFSET                      2
#define WAN_TX_VALID_SUB_GROUP_OFFSET                           3
#define WAN_CRC_ERROR_NORMAL_COUNTER_OFFSET                     8
#define WAN_CRC_ERROR_IPTV_COUNTER_OFFSET                       9
#define WAN_RX_BPM_CONGESTION_OFFSET                            10
#define ACL_OUI_DROP_COUNTER_OFFSET                             32
#define ACL_LAYER2_DROP_COUNTER_OFFSET                          33
#define ACL_LAYER3_DROP_COUNTER_OFFSET                          34
#define INGRESS_FILTER_DROP_SUB_GROUP_OFFSET                    0
#define LAYER4_FILTER_DROP_SUB_GROUP_OFFSET                     16
#define INVALID_LAYER2_PROTOCOL_DROP_COUNTER_OFFSET             32
#define FIREWALL_DROP_COUNTER_OFFSET                            33
#define DST_MAC_NON_ROUTER_COUNTER_OFFSET                       34
#define ETHERNET_FLOW_DROP_ACTION_COUNTER_OFFSET                35
#define EMAC_LOOPBACK_DROP_COUNTER                              36
#define LAN_ENQUEUE_CONGESTION_COUNTER_OFFSET                   36
#define VLAN_SWITCHING_DROP_COUNTER_OFFSET                      37
#define SA_LOOKUP_FAILURE_DROP_COUNTER_OFFSET                   38
#define DA_LOOKUP_FAILURE_DROP_COUNTER_OFFSET                   39
#define SA_ACTION_DROP_COUNTER_OFFSET                           40
#define DA_ACTION_DROP_COUNTER_OFFSET                           41
#define FORWARDING_MATRIX_DISABLED_DROP_COUNTER_OFFSET          42
#define CONNECTION_ACTION_DROP_COUNTER_OFFSET                   43
#define IPTV_LAYER3_DROP_COUNTER_OFFSET                         44
#define DOWNSTREAM_POLICERS_DROP_COUNTER_OFFSET                 45
#define UPSTREAM_POLICERS_DROP_COUNTER_OFFSET                   45
#define INGRESS_FILTER_IP_VALIDATIOH_GROUP_OFFSET               46
#define IP_HEADER_ERROR_DROP_COUNTER_OFFSET                     46
#define IP_FRAGMENT_DROP_COUNTER_OFFSET                         47
#define TPID_DETECT_DROP_COUNTER_OFFSET                         48
#define DUAL_STACK_LITE_CONGESTION_DROP_COUNTER_OFFSET          49
#define INVALID_SUBNET_IP_DROP_COUNTER_OFFSET                   50
#define EPON_DDR_QUEUES_COUNTER_OFFSET                          51
#define DOWNSTREAM_PARALLEL_PROCESSING_NO_SLAVE_WAIT_OFFSET     52
#define DOWNSTREAM_PARALLEL_PROCESSING_REORDER_WAIT_OFFSET      53
#define ABSOLUTE_ADDRESS_LIST_OVERFLOW_OFFSET                   54
#define DHD_IH_CONGESTION_OFFSET                                55
#define DHD_MALLOC_FAILED_OFFSET                                56
#define WLAN_MCAST_COPY_FAILED_OFFSET                           57
#define WLAN_MCAST_OVERFLOW_OFFSET                              58
#define WLAN_MCAST_DROP_COUNTER_OFFSET                          59
#define SBPM_ALLOC_NACK_REPLY_OFFSET                            60
#define CPU_RX_METERS_DROPPED_PACKETS_UPSTREAM_OFFSET           16
#ifdef G9991
#define US_DFC_FRAME_ERROR_G9991_GLOBAL_GROUP_OFFSET            0
#define US_DFC_FRAME_G9991_GLOBAL_GROUP_OFFSET                  1
#define US_DATA_FRAME_ERROR_G9991_GLOBAL_GROUP_OFFSET           2
#define US_ILLEGAL_SID_G9991_GLOBAL_GROUP_OFFSET                3
#define US_LENGTH_ERROR_G9991_GLOBAL_GROUP_OFFSET               4
#define US_REASSEMBLY_ERROR_G9991_GLOBAL_GROUP_OFFSET           5
#define US_BBH_ERROR_G9991_GLOBAL_GROUP_OFFSET                  6
#define US_CONSEQUENT_DROP_G9991_GLOBAL_GROUP_OFFSET            7
#define US_G9991_FRAGMENT_LENGTH                                108
#define US_G9991_FRAGMENT_PAYLOAD_LENGTH                        104
#define G9991_SID_NUMBER                                        32
#endif

#define BBH_RESET_WORD_1_DESCRIPTOR_VALUE                       0xF0000000
#define BBH_RESET_WORD_1_DESCRIPTOR_VALUE_MOD_8                 0xF0

#define BBH_PERIPHERAL_GPON_RX                                  0
#define BBH_PERIPHERAL_GPON_TX                                  32
#define BBH_PERIPHERAL_EPON_TX                                  96
#define BBH_PERIPHERAL_CO_RUNNER                                2
#define BBH_PERIPHERAL_BPM                                      3
#define BBH_PERIPHERAL_SBPM                                     7
#define BBH_PERIPHERAL_ETH0_TX                                  60
#define BBH_PERIPHERAL_ETH0_RX                                  28
#define BBH_PERIPHERAL_ETH1_TX                                  44
#define BBH_PERIPHERAL_ETH1_RX                                  12
#define BBH_PERIPHERAL_ETH2_TX                                  52
#define BBH_PERIPHERAL_ETH2_RX                                  20
#define BBH_PERIPHERAL_ETH3_TX                                  40
#define BBH_PERIPHERAL_ETH3_RX                                  8
#define BBH_PERIPHERAL_ETH4_TX                                  48
#define BBH_PERIPHERAL_ETH4_RX                                  16
#define BBH_PERIPHERAL_IH                                       6
#define BBH_PERIPHERAL_MIPS_D                                   14

#define WAN_SRC_PORT                                            0
#define ETH0_SRC_PORT                                           1
#define ETH1_SRC_PORT                                           2
#define ETH2_SRC_PORT                                           3
#define ETH3_SRC_PORT                                           4
#define ETH4_SRC_PORT                                           5
#define MIPS_C_SRC_PORT                                         6
#define WAN_IPTV_SRC_PORT                                       7
#define PCI_0_SRC_PORT                                          8
#define SPARE_0_SRC_PORT                                        12
#define FAST_RUNNER_A_SRC_PORT                                  14
#define FAST_RUNNER_B_SRC_PORT                                  15

/* Parallel processing */
#define FLOW_CACHE_SLAVE0_VECTOR_MASK               1
#define FLOW_CACHE_SLAVE1_VECTOR_MASK               2
#define FLOW_CACHE_SLAVE2_VECTOR_MASK               4
#define FLOW_CACHE_SLAVE3_VECTOR_MASK               8
#define CPU_FLOW_CACHE_VECTOR_MASK                  16

/* Dummy defines for software */
#define ETH0_RX_DIRECT_DESCRIPTORS_ADDRESS          0
#define ETH1_RX_DIRECT_DESCRIPTORS_ADDRESS          0
#define ETH2_RX_DIRECT_DESCRIPTORS_ADDRESS          0
#define ETH3_RX_DIRECT_DESCRIPTORS_ADDRESS          0
#define ETH4_RX_DIRECT_DESCRIPTORS_ADDRESS          0

#define ETH0_RX_DIRECT_RUNNER_A_TASK_NUMBER         0
#define ETH1_RX_DIRECT_RUNNER_A_TASK_NUMBER         0
#define ETH2_RX_DIRECT_RUNNER_A_TASK_NUMBER         0
#define ETH3_RX_DIRECT_RUNNER_A_TASK_NUMBER         0
#define ETH4_RX_DIRECT_RUNNER_A_TASK_NUMBER         0

#define ETH0_RX_DIRECT_RUNNER_B_TASK_NUMBER         0
#define ETH1_RX_DIRECT_RUNNER_B_TASK_NUMBER         0
#define ETH2_RX_DIRECT_RUNNER_B_TASK_NUMBER         0
#define ETH3_RX_DIRECT_RUNNER_B_TASK_NUMBER         0
#define ETH4_RX_DIRECT_RUNNER_B_TASK_NUMBER         0

#define MAC_ADDRESS_SIZE                            6

#define CPU_RX_INTCOL_TOTALPKTS_OFFSET              0
#define CPU_RX_INTCOL_MAXPKTS_OFFSET                1
#define CPU_RX_INTCOL_TIMEOUTS_OFFSET               2

#define BBTX_EEE_MODE_CONFIG_MESSAGE                6
#define DDR_QUEUE_CACHE_ENTRY_SIZE                  12
#define EPON_QUEUE_MAX_SIZE                         0x1000

#define INVALID_COUNTER_ID                          0
#endif

