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

#ifndef _RDD_RUNNER_DEFS_H
#define _RDD_RUNNER_DEFS_H

#define BCR_TIMER_SCHEDULER_TIMER_ADDRESS                       ( SYSTEM_CONFIGURATION_ADDRESS + SYSTEM_CONFIGURATION_TIMER_SCHEDULER_PERIOD_OFFSET )
#define BCR_POLICERS_TIMER_ADDRESS                              ( SYSTEM_CONFIGURATION_ADDRESS + SYSTEM_CONFIGURATION_POLICERS_PERIOD_OFFSET )
#define BCR_ACTIVE_POLICERS_VECTOR_ADDRESS                      ( SYSTEM_CONFIGURATION_ADDRESS + SYSTEM_CONFIGURATION_ACTIVE_POLICERS_VECTOR_OFFSET )
#define BCR_DOWNSTREAM_RATE_SHAPER_TIMER_ADDRESS                ( SYSTEM_CONFIGURATION_ADDRESS + SYSTEM_CONFIGURATION_DS_RATE_SHAPER_TIMER_OFFSET )
#define BCR_UPSTREAM_RATE_CONTROLLER_TIMER_ADDRESS              ( SYSTEM_CONFIGURATION_ADDRESS + SYSTEM_CONFIGURATION_US_RATE_CONTROLLER_TIMER_OFFSET )

#define BBH_PERIPHERAL_WAN_RX                                   0
#define BBH_PERIPHERAL_DOCSIS_RX                                BBH_PERIPHERAL_WAN_RX
#define BBH_PERIPHERAL_ETH0_RX                                  28
#define BBH_PERIPHERAL_ETH1_RX                                  12
#define BBH_PERIPHERAL_ETH2_RX                                  20
#define BBH_PERIPHERAL_ETH3_RX                                  8
#define BBH_PERIPHERAL_ETH4_RX                                  16
#define BBH_PERIPHERAL_BYOI_RX                                  BBH_PERIPHERAL_ETH4_RX
#define BBH_PERIPHERAL_WAN_TX                                   32
#define BBH_PERIPHERAL_ETH0_TX                                  60
#define BBH_PERIPHERAL_ETH1_TX                                  44
#define BBH_PERIPHERAL_ETH2_TX                                  52
#define BBH_PERIPHERAL_ETH3_TX                                  40
#define BBH_PERIPHERAL_ETH4_TX                                  48
#define BBH_PERIPHERAL_CO_RUNNER                                2
#define BBH_PERIPHERAL_BPM                                      3
#define BBH_PERIPHERAL_IH                                       6
#define BBH_PERIPHERAL_SBPM                                     7

#define WAN_SRC_PORT                                            0
#define WAN_IPTV_SRC_PORT                                       7
#define PCI_0_SRC_PORT                                          8
#define RUNNER_CLUSTER_A_SRC_PORT                               14
#define RUNNER_CLUSTER_B_SRC_PORT                               15

#define RDD_DS_IH_PACKET_HEADROOM_OFFSET                        18
#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
#define RDD_US_IH_PACKET_HEADROOM_OFFSET                        18
#else
#define RDD_US_IH_PACKET_HEADROOM_OFFSET                        26
#endif
#define RDD_LAYER2_HEADER_MINIMUM_LENGTH                        14
#define RDD_RUNNER_FLOW_RUNNER_A_IH_BUFFER                      14
#define RDD_RUNNER_FLOW_RUNNER_B_IH_BUFFER                      15
#define RDD_RUNNER_A_IH_BUFFER_BBH_OFFSET                       ((RDD_RUNNER_FLOW_RUNNER_A_IH_BUFFER * 128 + RDD_DS_IH_PACKET_HEADROOM_OFFSET) / 8)
#define RDD_RUNNER_B_IH_BUFFER_BBH_OFFSET                       ((RDD_RUNNER_FLOW_RUNNER_B_IH_BUFFER * 128 + RDD_US_IH_PACKET_HEADROOM_OFFSET) / 8)
#define RDD_IH_BUFFER_BBH_ADDRESS                               0x8000
#define RDD_IH_HEADER_DESCRIPTOR_BBH_ADDRESS                    0x0

#define DQM_DMA_ADDRESS(index)                                  (0xD3809000 + 32 * index)
#define DQM_STATUS_DMA_ADDRESS(index)                           (0xD3807400 + 4 * index)
#define DQM_CONTROL_CFG_B_DMA_ADDRESS(index)                    (0xD3808000 + 32 * index + 8)

/* GPIO */
#define RDD_GPIO_IO_ADDRESS                                     0x148

#define NUM_OF_GLOBAL_REGS                                      8
#define NUM_OF_MAIN_RUNNER_THREADS                              32
#define NUM_OF_US_PROCESSING_THREADS                            4
#define NUM_OF_DS_PROCESSING_THREADS                            4

#if defined(WL4908)
// Main A
#define CPU_TX_FAST_THREAD_NUMBER                               0
#define CPU_RX_THREAD_NUMBER                                    1
#define DHD_TX_POST_FAST_A_THREAD_NUMBER                        3
#define TIMER_SCHEDULER_MAIN_THREAD_NUMBER                      4
#define POLICER_BUDGET_ALLOCATOR_THREAD_NUMBER                  5
#define DHD_TX_COMPLETE_FAST_A_THREAD_NUMBER                    16
#define DHD1_TX_COMPLETE_FAST_A_THREAD_NUMBER                   17
#define DHD2_TX_COMPLETE_FAST_A_THREAD_NUMBER                   18
#define DS_PROCESSING_0_THREAD_NUMBER                           24
#define DS_PROCESSING_1_THREAD_NUMBER                           25
#define DS_PROCESSING_2_THREAD_NUMBER                           26
#define DS_PROCESSING_3_THREAD_NUMBER                           27
#define DOWNSTREAM_MULTICAST_THREAD_NUMBER                      28
#define FREE_SKB_INDEX_FAST_THREAD_NUMBER                       29
#define IPSEC_DOWNSTREAM_THREAD_NUMBER                          30

// Main B
//efine CPU_TX_FAST_THREAD_NUMBER                               0
//efine CPU_RX_THREAD_NUMBER                                    1
#define RATE_CONTROLLER_BUDGET_ALLOCATOR_THREAD_NUMBER          3
#define TIMER_SCHEDULER_MAIN_THREAD_NUMBER                      4
#define POLICER_BUDGET_ALLOCATOR_THREAD_NUMBER                  5
#define WAN1_TX_THREAD_NUMBER                                   6
#define DHD_TX_POST_FAST_B_THREAD_NUMBER                        8
#define WAN_ENQUEUE_THREAD_NUMBER                               21
#define US_SPDSVC_THREAD_NUMBER                                 27
#define FREE_SKB_INDEX_FAST_THREAD_NUMBER                       29

// PICO A
#define CPU_TX_PICO_THREAD_NUMBER                               32
#define GSO_PICO_THREAD_NUMBER                                  33
#define TIMER_SCHEDULER_PICO_THREAD_NUMBER                      34
#define RATE_SHAPER_BUDGET_ALLOCATOR_THREAD_NUMBER              35
#define LOCAL_SWITCHING_LAN_ENQUEUE_THREAD_NUMBER               36
#define DOWNSTREAM_LAN_ENQUEUE_THREAD_NUMBER                    37
#define DOWNSTREAM_MULTICAST_LAN_ENQUEUE_THREAD_NUMBER          38
#define CPU_RX_INTERRUPT_COALESCING_THREAD_NUMBER               39
#define FREE_SKB_INDEX_PICO_A_THREAD_NUMBER                     40
#define WLAN_MCAST_THREAD_NUMBER                                41
#define LAN_TX_THREAD_NUMBER                                    42
#define DS_SPDSVC_THREAD_NUMBER                                 47

// PICO B

#define FREE_SKB_INDEX_PICO_B_THREAD_NUMBER                     33
//efine TIMER_SCHEDULER_PICO_THREAD_NUMBER                      34  // NOTE: Used in PICO B, already defined for PICO A
#define CPU_US_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER         35
#define US_PROCESSING_0_THREAD_NUMBER                           36
#define US_PROCESSING_1_THREAD_NUMBER                           37
#define US_PROCESSING_2_THREAD_NUMBER                           38
#define US_PROCESSING_3_THREAD_NUMBER                           39
#define DHD_RX_THREAD_NUMBER                                    40
#define DHD1_RX_THREAD_NUMBER                                   41
#define DHD2_RX_THREAD_NUMBER                                   42
#define LAN0_RX_DISPATCH_THREAD_NUMBER                          43
#define LAN1_RX_DISPATCH_THREAD_NUMBER                          44
#define LAN2_RX_DISPATCH_THREAD_NUMBER                          45


#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
#define CPU_RX_THREAD_WAKEUP_REQUEST_VALUE                          ( ( CPU_RX_THREAD_NUMBER << 4 ) + 1 )
#define CPU_US_CLASSIFICATION_THREAD_WAKEUP_REQUEST_VALUE           ( ( CPU_US_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER << 4 ) + 1 )
#endif /*DSL*/

#define SELF_FIRMWARE_WAKEUP_REQUEST_BIT                        2

#define THREAD_WAKEUP_REQUEST(x)                                (((x) << 4) + 1)
#define PICO_RUNNER_THREAD(x)                                   (x - 32)

#define DDR_ADDRESS_FOR_DMA_SYNC                                0xFFFF0000
#define DMA_SYNCHRONIZATION_DUMMY_ADDRESS                       DDR_ADDRESS_FOR_DMA_SYNC

#define CAM_STOP_VALUE                                          0xFFFF
#define CPU_TX_DESCRIPTOR_ADDRESS_MASK                          0xFF80

#if !defined(FIRMWARE_INIT)
#define TIMER_SCHEDULER_TASK_PERIOD                             1000
#else
#define TIMER_SCHEDULER_TASK_PERIOD                             100
#endif
/* timer schedular primitive ids */
#define CPU_RX_METER_BUDGET_ALLOCATE_CODE_ID                    0 /* fast a + fast b */
#define DS_RATE_SHAPER_EXPONENT                                 3
#define UPSTREAM_RATE_LIMITER_ID                                8

/* PM counter */
#define DOWNSTREAM_RX_PACKETS_GROUP                             0
#define DOWNSTREAM_RX_BYTES_GROUP                               16
#define DOWNSTREAM_RX_DROPPED_SUMMARY_GROUP                     32
#define UPSTREAM_TX_PACKETS_GROUP                               40
#define UPSTREAM_TX_BYTES_GROUP                                 56
#define UPSTREAM_TX_CONGESTION_GROUP                            72
#define BRIDGE_FILTERED_GROUP                                   81
#define BRIDGE_DOWNSTREAM_TX_CONGESTION_GROUP                   82
#define WAN_BRIDGE_PORT_GROUP                                   83
/* The first 16 16bit counters of group 82 are used for LAN port tx discards.
 * The last 8 32bit counters of group 82 are used for LAN port tx packets.
 *
 *    16bit counter 0 is reserved.
 *    16bit counter 1 is for LAN0 tx discard count.
 *    16bit counter 2 is for LAN1 tx discard count.
 *    ....
 *    16bit counter 7 is for LAN6 tx discard count.
 *
 *    32bit counter 8 is reserved.
 *    32bit counter 9 is for LAN0 tx packet count.
 *    32bit counter 10 is for LAN1 tx packet count.
 *    ....
 *    32bit counter 15 is for LAN6 tx packet count.
 */
#define LAN_TX_PACKETS_GROUP                                    82
/* The last 8 32bit counters of group 83 are used for LAN port rx packets.
 *
 *    32bit counters 0 - 8 are reserved.
 *    32bit counter 9 is for LAN0 rx packet count.
 *    32bit counter 10 is for LAN1 rx packet count.
 *    ....
 *    32bit counter 15 is for LAN6 rx packet count.
 */
#define LAN_RX_PACKETS_GROUP                                    83
#define CPU_RX_INTERRUPT_COALESCING_GROUP                       84
#define G9991_GLOBAL_GROUP                                      85
#define UPSTREAM_VARIOUS_PACKETS_GROUP                          86
#define DOWNSTREAM_VARIOUS_PACKETS_GROUP                        88
#define VIRTUAL_PORT_PACKETS_GROUP                              90
#define CPU_RX_METERS_DROPPED_PACKETS_GROUP                     91
#define SUBNET_RX_GROUP                                         92
#define SUBNET_TX_GROUP                                         93
#define SUBNET_TX_BYTES_GROUP                                   95
#define DHD_SSID_DROP_PACKET_GROUP                              96

#define WAN_CRC_ERROR_NORMAL_COUNTER_OFFSET                     0
#define LAN_SUBNET_COUNTER_OFFSET                               2
#define SUBNET_DROPPED_PACKETS_SUB_GROUP_OFFSET                 20

#define WAN_CRC_ERROR_IPTV_COUNTER_OFFSET                       1
#define WAN_BRIDGED_RX_VALID_SUB_GROUP_OFFSET                   1
#define WAN_IPTV_RX_VALID_SUB_GROUP_OFFSET                      2
#define WAN_RX_BPM_CONGESTION_OFFSET                            8
#define ACL_OUI_DROP_COUNTER_OFFSET                             32
#define INGRESS_FILTER_DROP_SUB_GROUP_OFFSET                    0
#define LAYER4_FILTER_DROP_SUB_GROUP_OFFSET                     16
#define FORWARDING_MATRIX_DISABLED_DROP_COUNTER_OFFSET          42
#define CONNECTION_ACTION_DROP_COUNTER_OFFSET                   43
#define IPTV_DROP_COUNTER_OFFSET                                44
#define DOWNSTREAM_POLICERS_DROP_COUNTER_OFFSET                 45
#define DOWNSTREAM_IH_ERROR_EXCEPTION_OFFSET                    46
#define INGRESS_FILTER_IP_VALIDATION_GROUP_OFFSET               46
#define IP_HEADER_ERROR_DROP_COUNTER_OFFSET                     46
#define IP_FRAGMENT_DROP_COUNTER_OFFSET                         47
#define TPID_DETECT_DROP_COUNTER_OFFSET                         48
#define EPON_DDR_QUEUES_COUNTER_OFFSET                          51
#define DHD_IH_CONGESTION_OFFSET                                55
#define DHD_MALLOC_FAILED_OFFSET                                56
#define ABSOLUTE_ADDRESS_LIST_OVERFLOW_OFFSET                   54
#define WLAN_MCAST_COPY_FAILED_OFFSET                           57
#define WLAN_MCAST_OVERFLOW_OFFSET                              58
#define WLAN_MCAST_DROP_COUNTER_OFFSET                          59
#define CPU_RX_METERS_DROPPED_PACKETS_UPSTREAM_OFFSET           16

/* DDR queues */
#define CACHE_FIFO_HEAD_PD_NUMBER                               8
#define CACHE_ENTRY_BYTE_SIZE                                   8
#define TOTAL_CACHE_FIFO_PD_NUMBER                              12
#define CACHE_FIFO_BYTE_SIZE                                    (TOTAL_CACHE_FIFO_PD_NUMBER * CACHE_ENTRY_BYTE_SIZE)
#define CACHE_SLOT_PD_NUMBER                                    4
#define CACHE_FIFO_HEAD_SLOT_BYTE_SIZE                          (CACHE_FIFO_HEAD_PD_NUMBER * CACHE_ENTRY_BYTE_SIZE)

/* Policer */
#define POLICER_TIMER_PERIOD                                    4000
#define POLICER_EXPONENT                                        5

/* General */
#define RDD_IPV6_HEADER_SIZE                                    40
#define RDD_PACKET_BUFFER_SIZE                                  2048
#define RDD_PACKET_HEADROOM_OFFSET                              18

#define DS_TUPLE_CLASSIFICATION_RESULT_INDEX                    2
#define US_TUPLE_CLASSIFICATION_RESULT_INDEX                    1

/* EPON */
#define EPON_TX_DDR_QUEUE_SIZE                                  0x1000
#define RUNNER_TABLES_OFFSET_BPM_GLOBAL_THRESHOLD_15K           0x1000000

/* CPU_RX */
#define CPU_RX_INTCOL_TOTALPKTS_OFFSET                          0
#define CPU_RX_INTCOL_MAXPKTS_OFFSET                            1
#define CPU_RX_INTCOL_TIMEOUTS_OFFSET                           2

#define INTERRUPT_COALESCING_TIMER_PERIOD                       100

/* G9991 */
#define US_G9991_FRAGMENT_PAYLOAD_LENGTH                        104

#define EMAC_ABSOLUTE_TX_FIRMWARE_COUNTER_ADDRESS               EMAC_ABSOLUTE_TX_BBH_COUNTER_ADDRESS

#define INVALID_COUNTER_ID                                      0

#define RDD_US_RATE_CONTROL_TIMER_INTERVAL                      4000

#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
#define WLAN_MCAST_EGRESS_PORT_INDEX                            7

#define SPDSVC_TIMER_PERIOD                                     100 /* usec */
#define SPDSVC_TIMER_HZ                                         ( 1000000 / SPDSVC_TIMER_PERIOD ) /* sec */
#define SPDSVC_ETH_IFG                                          20 /* bytes */
#define SPDSVC_ETH_CRC_LEN                                      4  /* bytes */
#define SPDSVC_ETH_OVERHEAD                                     (SPDSVC_ETH_CRC_LEN + SPDSVC_ETH_IFG) /* bytes */
                                                                /* Ethernet packet + 2 VLAN Tags + PPPoE + Overhead */
#define SPDSVC_BUCKET_SIZE_MIN                                  (1514 + 8 + 8 + SPDSVC_ETH_OVERHEAD) /* bytes */

#define SPARE_0_SRC_PORT                                        12
#define BBTX_EEE_MODE_CONFIG_MESSAGE                            6
#define DS_GLOBAL_CFG_BRIDGE_FLOW_CACHE_MODE_BIT_OFFSET         1
#define US_GLOBAL_CFG_BROADCOM_SWITCH_MODE_BIT_OFFSET           2
#define DS_GLOBAL_CFG_BROADCOM_SWITCH_MODE_BIT_OFFSET           2
#define DS_GLOBAL_CFG_FLOW_CACHE_MODE_BIT_OFFSET                0
#define US_GLOBAL_CFG_CHIP_REVISION_OFFSET                      6
#define DS_GLOBAL_CFG_CHIP_REVISION_OFFSET                      4
#define US_GLOBAL_CFG_SPDSVC_CONTEXT_BIT_OFFSET                 7
#define DS_GLOBAL_CFG_SPDSVC_CONTEXT_BIT_OFFSET                 7
#define FREE_SKB_INDEX_ALLOCATE_CODE_ID                         5 /* fast a+b, pico a+b */
#define CPU_RX_METER_BUDGET_ALLOCATE_CODE_ID                    0 /* fast a + fast b */
#define UPSTREAM_RATE_LIMITER_BUDGET_ALLOCATE_CODE_ID           1 /* fast b */
#define UPSTREAM_INGRESS_RATE_LIMITER_BUDGET_ALLOCATE_CODE_ID   2 /* pico b */
#define UPSTREAM_QUASI_BUDGET_ALLOCATE_CODE_ID                  3 /* pico b */
#define DOWNSTREAM_RATE_LIMITER_BUDGET_ALLOCATE_CODE_ID         4 /* pico a */
#ifdef RUNNER_A
#define GLOBAL_CFG_CHIP_REVISION_OFFSET                         DS_GLOBAL_CFG_CHIP_REVISION_OFFSET
#else
#define GLOBAL_CFG_CHIP_REVISION_OFFSET                         US_GLOBAL_CFG_CHIP_REVISION_OFFSET
#endif

#define EMAC_RATE_LIMITER_EXPONENT                              3
#define UPSTREAM_RATE_LIMITER_TIMER_PERIOD                      4000
#define UPSTREAM_RATE_LIMITER_EXPONENT                          5
#define UPSTREAM_QUASI_POLICER_EXPONENT                         3

#define WAN_TX_QUEUES_BYTES_GROUP                               0
#define WAN_TX_QUEUES_PACKETS_GROUP                             4
#define WAN_TX_QUEUES_DROPPED_BYTES_GROUP                       8
#define WAN_TX_QUEUES_DROPPED_PACKETS_GROUP                     12

#define LAN_TX_QUEUES_BYTES_GROUP                               0
#define LAN_TX_QUEUES_PACKETS_GROUP                             4
#define LAN_TX_QUEUES_DROPPED_BYTES_GROUP                       8
#define LAN_TX_QUEUES_DROPPED_PACKETS_GROUP                     12

/* UCAST DROP */
#define UCAST_DROP_COMMAND_LIST_STRING_SIZE                     16
#define UCAST_DROP_COMMAND_LIST_TIMESTAMP_SIZE                  8
#define UCAST_DROP_COMMAND_LIST_TOTAL_SIZE                      (UCAST_DROP_COMMAND_LIST_STRING_SIZE + UCAST_DROP_COMMAND_LIST_TIMESTAMP_SIZE)

/*DS queues with a filling level less than this threshold can allocate Packet Descriptor using the guaranteed PD Pool budget*/
#define DS_FREE_PACKET_DESCRIPTOR_POOL_GUARANTEED_QUEUE_THRESHOLD 8

/*The guaranteed PD Pool budget is sized in function of the number of configured DS queues. To avoid unexpected behavior in
 *corner cases, e.g. a transient condition when all DS queues are unconfigured, the guaranteed PD Pool budget is clamped
 *to at least this minimum guaranteed pool size value.*/
#define DS_FREE_PACKET_DESCRIPTOR_POOL_MIN_GUARANTEED_POOL_SIZE  64

/* US queues with a filling level less than this threshold can allocate Packet Descriptor using the guaranteed PD Pool budget */
#define US_FREE_PACKET_DESCRIPTOR_POOL_GUARANTEED_QUEUE_THRESHOLD 72

/* The guaranteed PD Pool budget is sized in function of the number of configured US queues. To avoid unexpected behavior in
 * corner cases, e.g. a transient condition when all US queues are unconfigured, the guaranteed PD Pool budget is clamped
 * to at least this minimum guaranteed pool size value.
 * Let's say a typical configuration of - 8 queues in US. Each queue with 72 PDs guaranteed. (8*72 = 576 PDS)
 * NG PDs -> 3072-576 = 2496 PDs.
 */
#define US_FREE_PACKET_DESCRIPTOR_POOL_MIN_GUARANTEED_POOL_SIZE  (US_FREE_PACKET_DESCRIPTOR_POOL_GUARANTEED_QUEUE_THRESHOLD*8)

#define BBH_PERIPHERAL_DSL_TX                                   32

#define RDD_DDR_PACKET_PAYLOAD_OFFSET                          18
#define FREE_SKB_INDEX_TIMER_PERIOD                                     65000

#ifdef RUNNER_A
#define SPDSVC_CONTEXT_TABLE_ADDRESS                        DS_SPDSVC_CONTEXT_TABLE_ADDRESS
#define SPDSVC_THREAD_NUMBER                                DS_SPDSVC_THREAD_NUMBER
#define SPDSVC_WAKEUP_REQUEST_VALUE                         DS_SPDSVC_WAKEUP_REQUEST_VALUE
#else
#define SPDSVC_CONTEXT_TABLE_ADDRESS                        US_SPDSVC_CONTEXT_TABLE_ADDRESS
#define SPDSVC_THREAD_NUMBER                                US_SPDSVC_THREAD_NUMBER
#define SPDSVC_WAKEUP_REQUEST_VALUE                         US_SPDSVC_WAKEUP_REQUEST_VALUE
#endif

#ifdef RUNNER_A
#define SPDSVC_SET_CONTEXT() \
    insert  ds_global_cfg  R1  DS_GLOBAL_CFG_SPDSVC_CONTEXT_BIT_OFFSET  d.1
#define SPDSVC_RESET_CONTEXT() \
    insert  ds_global_cfg  R0  DS_GLOBAL_CFG_SPDSVC_CONTEXT_BIT_OFFSET  d.1
#else /* RUNNER_B */
#define SPDSVC_SET_CONTEXT() \
    insert  us_global_cfg  R1  US_GLOBAL_CFG_SPDSVC_CONTEXT_BIT_OFFSET  d.1
#define SPDSVC_RESET_CONTEXT() \
    insert  us_global_cfg  R0  US_GLOBAL_CFG_SPDSVC_CONTEXT_BIT_OFFSET  d.1
#endif

#define WLAN_MCAST_EGRESS_PORT_INDEX   7

/* Per chip limitation, 63138/63148 SAR can only transmit
 * max PTM frame size of 1984 bytes including FCS (4 bytes).
 */

/* UNIMAC message to enable/disable EEE LPI mode.*/
#define BBTX_EEE_MODE_CONFIG_MESSAGE                6

/* IPsec operation errors */
#define IPSEC_ICV_CHECK_FAIL       1
#endif /*DSL*/

#endif
