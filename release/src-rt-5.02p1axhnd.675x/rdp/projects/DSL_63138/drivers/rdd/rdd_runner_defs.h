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

#ifdef RUNNER_A
#define BPM_REPLY_ADDRESS                                           ( BPM_REPLY_RUNNER_A_ADDRESS )
#else
#define BPM_REPLY_ADDRESS                                           ( BPM_REPLY_RUNNER_B_ADDRESS )
#endif
#if defined(WL4908)
#define BPM_TRANSITION_STATE_ADDRESS                                ( BPM_REPLY_ADDRESS + 0x20 )
#else
#define BPM_TRANSITION_STATE_ADDRESS                                ( BPM_REPLY_RUNNER_A_ADDRESS + 0x20 )
#endif

#define FIREWALL_CFG_REG_RULES_MAP_TABLE_ADDRESS                    ( FIREWALL_CONFIGURATION_REGISTER_ADDRESS )
#define FIREWALL_CFG_REG_RULES_TABLE_ADDRESS                        ( FIREWALL_CONFIGURATION_REGISTER_ADDRESS + 4 )

#define LOCAL_SWITCHING_MULTICAST_INGRESS_QUEUE_PTR_ADDRESS         ( PICO_RUNNER_GLOBAL_REGISTERS_INIT_ADDRESS + d.28 )

#define DMA_SYNCHRONIZATION_DUMMY_ADDRESS                           0xFFFF0000
#define DDR_ADDRESS_FOR_DMA_SYNC                                    0xFFFF0000

#ifdef FIRMWARE_INIT
#define SIMULATOR_DDR_SKB_DATA_POINTERS_OFFSET                      0x01A00000
#define SIMULATOR_DDR_SKB_FREE_INDEXES_OFFSET                       0x01A10000
#define SIMULATOR_DDR_RING_OFFSET                                   0x01B00000
#define SIMULATOR_DDR_RING_HOST_BUFFERS_OFFSET                      0x01B00780
#define SIMULATOR_DDR_SKB_BUFFERS_OFFSET                            0x01B80000
#define SIMULATOR_DDR_RING_NUM_OF_ENTRIES                           10
#define SIMULATOR_DDR_PORT_HEADER_BUFFERS_OFFSET                    0x01104000
#define SIMULATOR_DDR_WLAN_MCAST_DHD_LIST_OFFSET                    0x01110000
#endif

#define BCR_HASH_BASED_FORWARDING_PORT_COUNT_ADDRESS            ( SYSTEM_CONFIGURATION_ADDRESS + SYSTEM_CONFIGURATION_HASH_BASED_FORWARDING_PORT_COUNT_OFFSET )
#define BCR_TIMER_SCHEDULER_TIMER_ADDRESS                       ( SYSTEM_CONFIGURATION_ADDRESS + SYSTEM_CONFIGURATION_TIMER_SCHEDULER_PERIOD_OFFSET )
#define BCR_POLICERS_TIMER_ADDRESS                              ( SYSTEM_CONFIGURATION_ADDRESS + SYSTEM_CONFIGURATION_POLICERS_PERIOD_OFFSET )
#define BCR_ACTIVE_POLICERS_VECTOR_ADDRESS                      ( SYSTEM_CONFIGURATION_ADDRESS + SYSTEM_CONFIGURATION_ACTIVE_POLICERS_VECTOR_OFFSET )
#define BCR_INTER_LAN_SCHEDULING_MODE_ADDRESS                   ( SYSTEM_CONFIGURATION_ADDRESS + SYSTEM_CONFIGURATION_INTER_LAN_SCHEDULING_MODE_OFFSET )
#define BCR_BROADCOM_SWITCH_PORT_ADDRESS                        ( SYSTEM_CONFIGURATION_ADDRESS + SYSTEM_CONFIGURATION_BROADCOM_SWITCH_PORT_OFFSET )
#define BCR_DOWNSTREAM_RATE_SHAPER_TIMER_ADDRESS                ( SYSTEM_CONFIGURATION_ADDRESS + SYSTEM_CONFIGURATION_DS_RATE_SHAPER_TIMER_OFFSET )
#define BCR_US_PADDING_MAX_SIZE_ADDRESS                         ( SYSTEM_CONFIGURATION_ADDRESS + SYSTEM_CONFIGURATION_US_PADDING_MAX_SIZE_OFFSET )
#define BCR_US_PADDING_CPU_MAX_SIZE_ADDRESS                     ( SYSTEM_CONFIGURATION_ADDRESS + SYSTEM_CONFIGURATION_US_PADDING_CPU_MAX_SIZE_OFFSET )
#define BCR_MIRORRING_MODE_ADDRESS                              ( SYSTEM_CONFIGURATION_ADDRESS + SYSTEM_CONFIGURATION_MIRRORING_PORT_OFFSET )
#define BCR_US_RATE_CONTROLLER_TIMER_ADDRESS                    ( SYSTEM_CONFIGURATION_ADDRESS + SYSTEM_CONFIGURATION_US_RATE_CONTROLLER_TIMER_OFFSET )
#define BCR_US_RATE_LIMIT_SUSTAIN_BUDGET_LIMIT_ADDRESS          ( SYSTEM_CONFIGURATION_ADDRESS + SYSTEM_CONFIGURATION_US_RATE_LIMIT_SUSTAIN_BUDGET_LIMIT_OFFSET )
#define BCR_1ST_EGRESS_ETHER_TYPE_ADDRESS                       ( SYSTEM_CONFIGURATION_ADDRESS + SYSTEM_CONFIGURATION_EGRESS_ETHER_TYPE_1_OFFSET )
#define BCR_2ND_EGRESS_ETHER_TYPE_ADDRESS                       ( SYSTEM_CONFIGURATION_ADDRESS + SYSTEM_CONFIGURATION_EGRESS_ETHER_TYPE_2_OFFSET )
#define BCR_3RD_EGRESS_ETHER_TYPE_ADDRESS                       ( SYSTEM_CONFIGURATION_ADDRESS + SYSTEM_CONFIGURATION_EGRESS_ETHER_TYPE_3_OFFSET )

/* the following are configured in rdd.. mostly in rdd_tm, not sure if they are used for 63138/63148/4908 */
#define BCR_DS_CONNECTION_MISS_ACTION_FILTER_ADDRESS            ( SYSTEM_CONFIGURATION_ADDRESS + SYSTEM_CONFIGURATION_DS_CONNECTION_MISS_ACTION_OFFSET )
#define BCR_PCI_FLOW_CACHE_MODE_ADDRESS                         ( SYSTEM_CONFIGURATION_ADDRESS + SYSTEM_CONFIGURATION_US_PCI_FLOW_CACHE_ENABLE_OFFSET )
#define BCR_INGRESS_GLOBAL_CONFIG_ADDRESS                       ( SYSTEM_CONFIGURATION_ADDRESS + SYSTEM_CONFIGURATION_GLOBAL_INGRESS_CONFIG_OFFSET )
#define BCR_LS_DROP_PRECEDENCE_ELIGIBILITY_VECTOR_ADDRESS       ( SYSTEM_CONFIGURATION_ADDRESS + SYSTEM_CONFIGURATION_PCI_LS_DP_ELIGIBILITY_VECTOR_OFFSET )
#define BCR_DS_INGRESS_POLICERS_MODE_ADDRESS                    ( SYSTEM_CONFIGURATION_ADDRESS + SYSTEM_CONFIGURATION_DS_INGRESS_POLICERS_MODE_OFFSET )


#define PROFILING_BUFFER_MAIN_RUNNER_ADDRESS                        INGRESS_HANDLER_BUFFER_ADDRESS

#define EMAC_ABSOLUTE_TX_FIRMWARE_COUNTER_ADDRESS                   EMAC_ABSOLUTE_TX_BBH_COUNTER_ADDRESS

#define TCONT_NUMBER_OF_RATE_CONTROLLERS                            32

#define CPU_RX_NUMBER_OF_QUEUES                                     15 /* 8 for cpu_host + 7 for cpu_wlan0 */

#define DDR_QUEUE_CHUNK_SIZE                                        4

#define UPSTREAM_RATE_LIMITER_ID                                    8

#if !defined(FIRMWARE_INIT)
#define TIMER_SCHEDULER_TASK_PERIOD                                     1000
#else
#define TIMER_SCHEDULER_TASK_PERIOD                                     100
#endif
#define BL_LILAC_RDD_US_RATE_CONTROL_TIMER_INTERVAL                     4000
#define UPSTREAM_INGRESS_RATE_LIMITER_BUDGET_ALLOCATOR_TIMER_INTERVAL   1000
#define POLICER_TIMER_PERIOD                                            4000
#define POLICER_EXPONENT                                                5
#define EMAC_RATE_LIMITER_TIMER_PERIOD                                  1000
#define EMAC_RATE_LIMITER_EXPONENT                                      3
#define SERVICE_QUEUE_RATE_LIMITER_TIMER_PERIOD                         4000
#define UPSTREAM_RATE_LIMITER_TIMER_PERIOD                              4000
#define UPSTREAM_RATE_LIMITER_EXPONENT                                  5
#define UPSTREAM_QUASI_POLICER_TIMER_PERIOD                             1000
#define UPSTREAM_QUASI_POLICER_EXPONENT                                 3
#define RDD_IPV6_HEADER_SIZE                                            40
#define DDR_PACKET_PAYLOAD_OFFSET                                       18
#define INTERRUPT_COALESCING_TIMER_PERIOD                               100
#define TIMER_7_TIMER_PERIOD                                            400 /* usec */
#define TIMER_7_TIMER_HZ                                                ( 1000000 / TIMER_7_TIMER_PERIOD ) /* sec */
#define SPDSVC_ETH_IFG                                                  20 /* bytes */
#define SPDSVC_ETH_CRC_LEN                                              4  /* bytes */
#define SPDSVC_ETH_OVERHEAD                                             (SPDSVC_ETH_CRC_LEN + SPDSVC_ETH_IFG) /* bytes */
                                                                        /* Ethernet packet + 2 VLAN Tags + PPPoE + Overhead */
#define SPDSVC_BUCKET_SIZE_MIN                                          (1514 + 8 + 8 + SPDSVC_ETH_OVERHEAD) /* bytes */
#define FREE_SKB_INDEX_TIMER_PERIOD                                     65000

/* timer schedular primitive ids */
#define CPU_RX_METER_BUDGET_ALLOCATE_CODE_ID                        0 /* fast a + fast b */
#define UPSTREAM_RATE_LIMITER_BUDGET_ALLOCATE_CODE_ID               1 /* fast b */
#define UPSTREAM_INGRESS_RATE_LIMITER_BUDGET_ALLOCATE_CODE_ID       2 /* pico b */
#define UPSTREAM_QUASI_BUDGET_ALLOCATE_CODE_ID                      3 /* pico b */
#define DOWNSTREAM_RATE_LIMITER_BUDGET_ALLOCATE_CODE_ID             4 /* pico a */
#define FREE_SKB_INDEX_ALLOCATE_CODE_ID                             5 /* fast a+b, pico a+b */
#define DOWNSTREAM_DHD_TX_POST_CLOSE_AGGREGATION_CODE_ID            6 /* fast a */
#define DOWNSTREAM_SERVICE_QUEUES_RATE_LIMITER_BUDGET_ALLOCATE_CODE_ID   7 /* pico a */

#define MAC_UNKNOWN_DA_FORWARDING_FILTER_DISABLED                   0xFFFFFFFF
#define MAC_UNKNOWN_DA_FORWARDING_POLICER_ENABLE_BIT_OFFSET         29

#define WAN_FILTERS_AND_CLASSIFICATON_R8_CPU_INDICATION_OFFSET      15
#define WAN_FILTERS_AND_CLASSIFICATON_R8_ETHWAN2_INDICATION_OFFSET  14

#define DS_GLOBAL_CFG_FLOW_CACHE_MODE_BIT_OFFSET                    0
#define US_GLOBAL_CFG_FLOW_CACHE_MODE_BIT_OFFSET                    0
#define DS_GLOBAL_CFG_BRIDGE_FLOW_CACHE_MODE_BIT_OFFSET             1
#define US_GLOBAL_CFG_BRIDGE_FLOW_CACHE_MODE_BIT_OFFSET             1
#define DS_GLOBAL_CFG_BROADCOM_SWITCH_MODE_BIT_OFFSET               2
#define US_GLOBAL_CFG_BROADCOM_SWITCH_MODE_BIT_OFFSET               2
#define DS_GLOBAL_CFG_BROADCOM_SWITCH_PHYSICAL_PORT_BIT_OFFSET      3
#define US_GLOBAL_CFG_EPON_MODE_BIT_OFFSET                          4
#define DS_GLOBAL_CFG_CHIP_REVISION_OFFSET                          4
#define GLOBAL_CFG_MIRRORING_MODE_BIT_OFFSET                        5
#define US_GLOBAL_CFG_CHIP_REVISION_OFFSET                          6
#define US_GLOBAL_CFG_SPDSVC_CONTEXT_BIT_OFFSET                     7
#define DS_GLOBAL_CFG_SPDSVC_CONTEXT_BIT_OFFSET                     7
#ifdef RUNNER_A
#define GLOBAL_CFG_CHIP_REVISION_OFFSET                             DS_GLOBAL_CFG_CHIP_REVISION_OFFSET
#else
#define GLOBAL_CFG_CHIP_REVISION_OFFSET                             US_GLOBAL_CFG_CHIP_REVISION_OFFSET
#endif

#ifdef RUNNER_A
#define FW_MAC_ADDRS_COUNT_ADDRESS                               DS_FW_MAC_ADDRS_COUNT_ADDRESS
#define FW_MAC_ADDRS_ADDRESS                                     DS_FW_MAC_ADDRS_ADDRESS
#else
#define FW_MAC_ADDRS_COUNT_ADDRESS                               US_FW_MAC_ADDRS_COUNT_ADDRESS
#define FW_MAC_ADDRS_ADDRESS                                     US_FW_MAC_ADDRS_ADDRESS
#endif

/*  downstream global ingress configuration vector */
#define GLOBAL_INGRESS_CONFIG_MIRRORING                             0
#define GLOBAL_INGRESS_CONFIG_DS_LITE                               1
#define GLOBAL_INGRESS_CONFIG_FULL_FLOW_CACHE_MODE                  2
#define GLOBAL_INGRESS_CONFIG_IP_MULTICAST_FC_ACCELERATION          3
#define GLOBAL_INGRESS_CONFIG_NON_IP_FC_ACCELRATION                 4

#define THREAD_WAKEUP_REQUEST(x)                                    (((x) << 4) + 1)
#define CPU_TX_DESCRIPTOR_ADDRESS_MASK                              0xFF80

/* Main A */
#define CPU_TX_FAST_THREAD_NUMBER                                   0
#define CPU_RX_THREAD_NUMBER                                        1
#define TIMER_SCHEDULER_MAIN_THREAD_NUMBER                          4
#define POLICER_BUDGET_ALLOCATOR_THREAD_NUMBER                      5
#if defined(DSL_63138) || defined(DSL_63148)
#define WAN_DIRECT_THREAD_NUMBER                                    7
#endif
#define WAN1_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER               8
#define WAN_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER                9
#define ETHWAN2_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER            10
#define DOWNSTREAM_FLOW_CACHE_SLAVE0_THREAD_NUMBER                  11
#define DOWNSTREAM_FLOW_CACHE_SLAVE1_THREAD_NUMBER                  12
#define DOWNSTREAM_FLOW_CACHE_SLAVE2_THREAD_NUMBER                  13
#define DOWNSTREAM_FLOW_CACHE_SLAVE3_THREAD_NUMBER                  14
#define DHD_TX_COMPLETE_FAST_A_THREAD_NUMBER                        16
#define DHD1_TX_COMPLETE_FAST_A_THREAD_NUMBER                       17
#define DHD2_TX_COMPLETE_FAST_A_THREAD_NUMBER                       18
#define DHD_TX_POST_FAST_A_THREAD_NUMBER                            19
#if defined(DSL_63138) || defined(DSL_63148)
#define CPU_DS_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER             25
#endif
#define DOWNSTREAM_MULTICAST_THREAD_NUMBER                          28
#define FREE_SKB_INDEX_FAST_THREAD_NUMBER                           29
#define IPSEC_DOWNSTREAM_THREAD_NUMBER                              30

/* Pico A */
#define CPU_TX_PICO_THREAD_NUMBER                                   32
#define GSO_PICO_THREAD_NUMBER                                      33
#define TIMER_SCHEDULER_PICO_A_THREAD_NUMBER                        34
#define DS_RX_BUFFER_COPY_THREAD_NUMBER                             35 
#define WLAN_MCAST_THREAD_NUMBER                                    36
#define DOWNSTREAM_LAN_ENQUEUE_THREAD_NUMBER                        37
#define DS_TIMER_7_THREAD_NUMBER                                    38
#define CPU_RX_INTERRUPT_COALESCING_THREAD_NUMBER                   39
#define FREE_SKB_INDEX_PICO_A_THREAD_NUMBER                         40
#define LOCAL_SWITCHING_LAN_ENQUEUE_THREAD_NUMBER                   41
#define ETH_TX_THREAD_NUMBER                                        42
#define SERVICE_QUEUE_ENQUEUE_THREAD_NUMBER                         44
#define SERVICE_QUEUE_DEQUEUE_THREAD_NUMBER                         45
#define DOWNSTREAM_MULTICAST_LAN_ENQUEUE_THREAD_NUMBER              47

#if defined(WL4908_EAP)
#undef TIMER_SCHEDULER_PICO_A_THREAD_NUMBER
#undef DS_RX_BUFFER_COPY_THREAD_NUMBER
#undef WLAN_MCAST_THREAD_NUMBER
#undef DS_TIMER_7_THREAD_NUMBER

#define CAPWAPF_CPU_PROCESSING0_THREAD_NUMBER                       33
#define CAPWAPF_CPU_PROCESSING1_THREAD_NUMBER                       34
#define CAPWAPF_CPU_PROCESSING2_THREAD_NUMBER                       35
#define TIMER_SCHEDULER_PICO_A_THREAD_NUMBER                        36
#define DS_RX_BUFFER_COPY_THREAD_NUMBER                             38 
#define WLAN_MCAST_THREAD_NUMBER                                    43
#define DS_TIMER_7_THREAD_NUMBER                                    46
#endif


/* Main B */
//efine CPU_TX_FAST_THREAD_NUMBER                                   0
//efine CPU_RX_THREAD_NUMBER                                        1
#define RATE_CONTROLLER_BUDGET_ALLOCATOR_THREAD_NUMBER              3
//efine TIMER_SCHEDULER_MAIN_THREAD_NUMBER                          4
//efine POLICER_BUDGET_ALLOCATOR_THREAD_NUMBER                      5
#define WAN1_TX_THREAD_NUMBER                                       6
#if defined(DSL_63138) || defined(DSL_63148)
#define WAN_TX_THREAD_NUMBER                                        7
#endif
#define DHD_TX_POST_FAST_B_THREAD_NUMBER                            8
#if defined(WL4908)
#define US_RX_BUFFER_COPY_THREAD_NUMBER                             9
#define US_RX_BUFFER_COPY1_THREAD_NUMBER                            10
#define US_RX_BUFFER_COPY2_THREAD_NUMBER                            11
#endif
#define WAN_ENQUEUE_THREAD_NUMBER                                   21
#define US_TIMER_7_THREAD_NUMBER                                    27
//#define FREE_SKB_INDEX_FAST_THREAD_NUMBER                         29

/* Pico B */
#define UPSTREAM_FLOW_CACHE_SLAVE0_THREAD_NUMBER                    32
#define UPSTREAM_FLOW_CACHE_SLAVE1_THREAD_NUMBER                    33
#define UPSTREAM_FLOW_CACHE_SLAVE2_THREAD_NUMBER                    34
#define UPSTREAM_FLOW_CACHE_SLAVE3_THREAD_NUMBER                    35
#define FREE_SKB_INDEX_PICO_B_THREAD_NUMBER                         36
#define TIMER_SCHEDULER_PICO_B_THREAD_NUMBER                        37
#define CPU_US_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER             38
#if defined(DSL_63138) || defined(DSL_63148)
#define LAN1_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER               39
#endif
#define DHD_RX_THREAD_NUMBER                                        40
#define DHD1_RX_THREAD_NUMBER                                       41
#define DHD2_RX_THREAD_NUMBER                                       42
#define LAN_DISPATCH_THREAD_NUMBER                                  43
#if defined(WL4908)
#define LAN1_DISPATCH_THREAD_NUMBER                                 44
#define LAN2_DISPATCH_THREAD_NUMBER                                 45
#endif

/* Main A */
#define CPU_TX_FAST_THREAD_WAKEUP_REQUEST_VALUE                         THREAD_WAKEUP_REQUEST(CPU_TX_FAST_THREAD_NUMBER)
#define CPU_RX_THREAD_WAKEUP_REQUEST_VALUE                              THREAD_WAKEUP_REQUEST(CPU_RX_THREAD_NUMBER)

#define TIMER_SCHEDULER_MAIN_THREAD_WAKEUP_REQUEST_VALUE                THREAD_WAKEUP_REQUEST(TIMER_SCHEDULER_MAIN_THREAD_NUMBER)
#define POLICER_BUDGET_ALLOCATOR_THREAD_WAKEUP_REQUEST_VALUE            THREAD_WAKEUP_REQUEST(POLICER_BUDGET_ALLOCATOR_THREAD_NUMBER)
#define WAN_DIRECT_THREAD_WAKEUP_REQUEST_VALUE                          THREAD_WAKEUP_REQUEST(WAN_DIRECT_THREAD_NUMBER)
#define WAN1_FILTERS_AND_CLASSIFICATION_THREAD_WAKEUP_REQUEST_VALUE     THREAD_WAKEUP_REQUEST(WAN1_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER)
#define WAN_FILTERS_AND_CLASSIFICATION_THREAD_WAKEUP_REQUEST_VALUE      THREAD_WAKEUP_REQUEST(WAN_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER)
#define ETHWAN2_FILTERS_AND_CLASSIFICATION_THREAD_WAKEUP_REQUEST_VALUE  THREAD_WAKEUP_REQUEST(ETHWAN2_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER)
#define DOWNSTREAM_FLOW_CACHE_SLAVE0_THREAD_WAKEUP_REQUEST_VALUE        THREAD_WAKEUP_REQUEST(DOWNSTREAM_FLOW_CACHE_SLAVE0_THREAD_NUMBER)
#define DOWNSTREAM_FLOW_CACHE_SLAVE1_THREAD_WAKEUP_REQUEST_VALUE        THREAD_WAKEUP_REQUEST(DOWNSTREAM_FLOW_CACHE_SLAVE1_THREAD_NUMBER)
#define DOWNSTREAM_FLOW_CACHE_SLAVE2_THREAD_WAKEUP_REQUEST_VALUE        THREAD_WAKEUP_REQUEST(DOWNSTREAM_FLOW_CACHE_SLAVE2_THREAD_NUMBER)
#define DOWNSTREAM_FLOW_CACHE_SLAVE3_THREAD_WAKEUP_REQUEST_VALUE        THREAD_WAKEUP_REQUEST(DOWNSTREAM_FLOW_CACHE_SLAVE3_THREAD_NUMBER)
#define DHD_TX_COMPLETE_FAST_A_THREAD_WAKEUP_REQUEST_VALUE              THREAD_WAKEUP_REQUEST(DHD_TX_COMPLETE_FAST_A_THREAD_NUMBER)
#define DHD1_TX_COMPLETE_FAST_A_THREAD_WAKEUP_REQUEST_VALUE             THREAD_WAKEUP_REQUEST(DHD1_TX_COMPLETE_FAST_A_THREAD_NUMBER)
#define DHD2_TX_COMPLETE_FAST_A_THREAD_WAKEUP_REQUEST_VALUE             THREAD_WAKEUP_REQUEST(DHD2_TX_COMPLETE_FAST_A_THREAD_NUMBER)
#define DHD_TX_POST_FAST_A_THREAD_WAKEUP_REQUEST_VALUE                  THREAD_WAKEUP_REQUEST(DHD_TX_POST_FAST_A_THREAD_NUMBER)
#if defined(DSL_63138) || defined(DSL_63148)
#define CPU_DS_CLASSIFICATION_THREAD_WAKEUP_REQUEST_VALUE               THREAD_WAKEUP_REQUEST(CPU_DS_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER)
#endif
#define DOWNSTREAM_MULTICAST_THREAD_WAKEUP_REQUEST_VALUE                THREAD_WAKEUP_REQUEST(DOWNSTREAM_MULTICAST_THREAD_NUMBER)
#define FREE_SKB_INDEX_FAST_THREAD_WAKEUP_REQUEST_VALUE                 THREAD_WAKEUP_REQUEST(FREE_SKB_INDEX_FAST_THREAD_NUMBER)
#define IPSEC_DOWNSTREAM_THREAD_WAKEUP_REQUEST_VALUE                    THREAD_WAKEUP_REQUEST(IPSEC_DOWNSTREAM_THREAD_NUMBER)

/* Pico A */
#define CPU_TX_PICO_THREAD_WAKEUP_REQUEST_VALUE                         THREAD_WAKEUP_REQUEST(CPU_TX_PICO_THREAD_NUMBER)
#define GSO_PICO_THREAD_WAKEUP_REQUEST_VALUE                            THREAD_WAKEUP_REQUEST(GSO_PICO_THREAD_NUMBER)
#define TIMER_SCHEDULER_PICO_A_THREAD_WAKEUP_REQUEST_VALUE              THREAD_WAKEUP_REQUEST(TIMER_SCHEDULER_PICO_A_THREAD_NUMBER)
#if defined(WL4908)
#define DS_RX_BUFFER_COPY_THREAD_WAKEUP_REQUEST_VALUE                   THREAD_WAKEUP_REQUEST(DS_RX_BUFFER_COPY_THREAD_NUMBER)
#endif
#define WLAN_MCAST_THREAD_WAKEUP_REQUEST_VALUE                          THREAD_WAKEUP_REQUEST(WLAN_MCAST_THREAD_NUMBER)
#define DOWNSTREAM_LAN_ENQUEUE_THREAD_WAKEUP_REQUEST_VALUE              THREAD_WAKEUP_REQUEST(DOWNSTREAM_LAN_ENQUEUE_THREAD_NUMBER)
#define DS_SPDSVC_THREAD_WAKEUP_REQUEST_VALUE                           THREAD_WAKEUP_REQUEST(DS_SPDSVC_THREAD_NUMBER)
#define CPU_RX_INTERRUPT_COALESCING_THREAD_WAKEUP_REQUEST_VALUE         THREAD_WAKEUP_REQUEST(CPU_RX_INTERRUPT_COALESCING_THREAD_NUMBER)
#define FREE_SKB_INDEX_PICO_A_THREAD_WAKEUP_REQUEST_VALUE               THREAD_WAKEUP_REQUEST(FREE_SKB_INDEX_PICO_A_THREAD_NUMBER)
#define LOCAL_SWITCHING_LAN_ENQUEUE_THREAD_WAKEUP_REQUEST_VALUE         THREAD_WAKEUP_REQUEST(LOCAL_SWITCHING_LAN_ENQUEUE_THREAD_NUMBER)
#define ETH_TX_THREAD_WAKEUP_REQUEST_VALUE                              THREAD_WAKEUP_REQUEST(ETH_TX_THREAD_NUMBER)
#define DOWNSTREAM_MULTICAST_LAN_ENQUEUE_THREAD_WAKEUP_REQUEST_VALUE    THREAD_WAKEUP_REQUEST(DOWNSTREAM_MULTICAST_LAN_ENQUEUE_THREAD_NUMBER)
#define SERVICE_QUEUE_ENQUEUE_THREAD_WAKEUP_REQUEST_VALUE               THREAD_WAKEUP_REQUEST(SERVICE_QUEUE_ENQUEUE_THREAD_NUMBER)
#define SERVICE_QUEUE_DEQUEUE_WAKEUP_THREAD_REQUEST_VALUE               THREAD_WAKEUP_REQUEST(SERVICE_QUEUE_DEQUEUE_THREAD_NUMBER)
#define CAPWAPF_CPU_PROCESSING0_THREAD_NUMBER_REQUEST_VALUE             THREAD_WAKEUP_REQUEST(CAPWAPF_CPU_PROCESSING0_THREAD_NUMBER)
#define CAPWAPF_CPU_PROCESSING2_THREAD_NUMBER_REQUEST_VALUE             THREAD_WAKEUP_REQUEST(CAPWAPF_CPU_PROCESSING2_THREAD_NUMBER)
#define CAPWAPF_CPU_PROCESSING3_THREAD_NUMBER_REQUEST_VALUE             THREAD_WAKEUP_REQUEST(CAPWAPF_CPU_PROCESSING3_THREAD_NUMBER)

/* Main B */
//efine CPU_TX_FAST_THREAD_WAKEUP_REQUEST_VALUE                         THREAD_WAKEUP_REQUEST(CPU_TX_FAST_THREAD_NUMBER)
//efine CPU_RX_THREAD_WAKEUP_REQUEST_VALUE                              THREAD_WAKEUP_REQUEST(CPU_RX_THREAD_NUMBER)
#define RATE_CONTROLLER_BUDGET_ALLOCATOR_THREAD_WAKEUP_REQUEST_VALUE    THREAD_WAKEUP_REQUEST(RATE_CONTROLLER_BUDGET_ALLOCATOR_THREAD_NUMBER)
//efine TIMER_SCHEDULER_MAIN_THREAD_WAKEUP_REQUEST_VALUE                THREAD_WAKEUP_REQUEST(TIMER_SCHEDULER_MAIN_THREAD_NUMBER)
//efine POLICER_BUDGET_ALLOCATOR_THREAD_WAKEUP_REQUEST_VALUE            THREAD_WAKEUP_REQUEST(POLICER_BUDGET_ALLOCATOR_THREAD_NUMBER)
#define WAN1_TX_THREAD_WAKEUP_REQUEST_VALUE                             THREAD_WAKEUP_REQUEST(WAN1_TX_THREAD_NUMBER)
#if defined(DSL_63138) || defined(DSL_63148)
#define WAN_TX_THREAD_WAKEUP_REQUEST_VALUE                              THREAD_WAKEUP_REQUEST(WAN_TX_THREAD_NUMBER)
#endif
#define DHD_TX_POST_FAST_B_THREAD_WAKEUP_REQUEST_VALUE                  THREAD_WAKEUP_REQUEST(DHD_TX_POST_FAST_B_THREAD_NUMBER)
#if defined(WL4908)
#define US_RX_BUFFER_COPY_THREAD_WAKEUP_REQUEST_VALUE                   THREAD_WAKEUP_REQUEST(US_RX_BUFFER_COPY_THREAD_NUMBER)
#define US_RX_BUFFER_COPY1_THREAD_WAKEUP_REQUEST_VALUE                  THREAD_WAKEUP_REQUEST(US_RX_BUFFER_COPY1_THREAD_NUMBER)
#define US_RX_BUFFER_COPY2_THREAD_WAKEUP_REQUEST_VALUE                  THREAD_WAKEUP_REQUEST(US_RX_BUFFER_COPY2_THREAD_NUMBER)
#endif
#define WAN_ENQUEUE_THREAD_WAKEUP_REQUEST_VALUE                         THREAD_WAKEUP_REQUEST(WAN_ENQUEUE_THREAD_NUMBER)
#define US_SPDSVC_THREAD_WAKEUP_REQUEST_VALUE                           THREAD_WAKEUP_REQUEST(US_SPDSVC_THREAD_NUMBER)
//efine FREE_SKB_INDEX_FAST_THREAD_WAKEUP_REQUEST_VALUE                 THREAD_WAKEUP_REQUEST(FREE_SKB_INDEX_FAST_THREAD_NUMBER)

/* Pico B */
#define UPSTREAM_FLOW_CACHE_SLAVE0_THREAD_WAKEUP_REQUEST_VALUE          THREAD_WAKEUP_REQUEST(UPSTREAM_FLOW_CACHE_SLAVE0_THREAD_NUMBER)
#define UPSTREAM_FLOW_CACHE_SLAVE1_THREAD_WAKEUP_REQUEST_VALUE          THREAD_WAKEUP_REQUEST(UPSTREAM_FLOW_CACHE_SLAVE1_THREAD_NUMBER)
#define UPSTREAM_FLOW_CACHE_SLAVE2_THREAD_WAKEUP_REQUEST_VALUE          THREAD_WAKEUP_REQUEST(UPSTREAM_FLOW_CACHE_SLAVE2_THREAD_NUMBER)
#define UPSTREAM_FLOW_CACHE_SLAVE3_THREAD_WAKEUP_REQUEST_VALUE          THREAD_WAKEUP_REQUEST(UPSTREAM_FLOW_CACHE_SLAVE3_THREAD_NUMBER)
#define FREE_SKB_INDEX_PICO_B_THREAD_WAKEUP_REQUEST_VALUE               THREAD_WAKEUP_REQUEST(FREE_SKB_INDEX_PICO_B_THREAD_NUMBER)
#define TIMER_SCHEDULER_PICO_B_THREAD_WAKEUP_REQUEST_VALUE              THREAD_WAKEUP_REQUEST(TIMER_SCHEDULER_PICO_B_THREAD_NUMBER)
#define CPU_US_CLASSIFICATION_THREAD_WAKEUP_REQUEST_VALUE               THREAD_WAKEUP_REQUEST(CPU_US_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER)
#if defined(DSL_63138) || defined(DSL_63148)
#define LAN1_FILTERS_AND_CLASSIFICATION_THREAD_WAKEUP_REQUEST_VALUE     THREAD_WAKEUP_REQUEST(LAN1_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER)
#endif
#define DHD_RX_THREAD_WAKEUP_REQUEST_VALUE                              THREAD_WAKEUP_REQUEST(DHD_RX_THREAD_NUMBER)
#define DHD1_RX_THREAD_WAKEUP_REQUEST_VALUE                             THREAD_WAKEUP_REQUEST(DHD1_RX_THREAD_NUMBER)
#define DHD2_RX_THREAD_WAKEUP_REQUEST_VALUE                             THREAD_WAKEUP_REQUEST(DHD2_RX_THREAD_NUMBER)
#define LAN_DISPATCH_THREAD_WAKEUP_REQUEST_VALUE                        THREAD_WAKEUP_REQUEST(LAN_DISPATCH_THREAD_NUMBER)
#if defined(WL4908)
#define LAN1_DISPATCH_THREAD_WAKEUP_REQUEST_VALUE                       THREAD_WAKEUP_REQUEST(LAN1_DISPATCH_THREAD_NUMBER)
#define LAN2_DISPATCH_THREAD_WAKEUP_REQUEST_VALUE                       THREAD_WAKEUP_REQUEST(LAN2_DISPATCH_THREAD_NUMBER)
#endif


#define LAN_FILTERS_LAN_TYPE_CPU_BIT_OFFSET                     31
#define LAN_FILTERS_LAN_TYPE_DIRECT_TO_CPU_BIT_OFFSET           30

#define SELF_FIRMWARE_WAKEUP_REQUEST_BIT                        2

#define DOWNSTREAM_RX_PACKETS_GROUP                             0
#define DOWNSTREAM_RX_BYTES_GROUP                               16

/* Each Runner counter group has 16 32bit counters or 32 16bit counters.
 * There are: 1 eth wan1 + 7 lans = 8 ports --> 64 queues total
 * Supports 4 counters per queue: tx_bytes, tx_packets, dropped_bytes, dropped_packets
 * Needs:
 *     64 32bit counters for tx_bytes --> 4 Runner counter groups: 0, 1, 2, 3
 *     64 32bit counters for tx_packets --> 4 Runner counter groups: 4, 5, 6, 7
 *     64 32bit counters for dropped_bytes --> 4 Runner counter groups: 8, 9, 10, 11
 *     64 16bit counters for dropped_packets --> 2 Runner counter groups: 12, 13
 * Counter allocation in each counter type is 8 WAN queue counters followed by 56 LAN queue counters. 
 */
#define WAN_TX_QUEUES_BYTES_GROUP                               0
#define WAN_TX_QUEUES_PACKETS_GROUP                             4
#define WAN_TX_QUEUES_DROPPED_BYTES_GROUP                       8
#define WAN_TX_QUEUES_DROPPED_PACKETS_GROUP                     12

#define LAN_TX_QUEUES_BYTES_GROUP                               0
#define LAN_TX_QUEUES_PACKETS_GROUP                             4
#define LAN_TX_QUEUES_DROPPED_BYTES_GROUP                       8
#define LAN_TX_QUEUES_DROPPED_PACKETS_GROUP                     12

/* For DSL wan0, we need:
 *     16 32bit counters for tx_bytes --> 1 Runner counter group: 14
 *     16 32bit counters for tx_packets --> 1 Runner counter group: 15
 *     16 32bit counters for dropped_bytes --> 1 Runner counter group: 16
 *     16 16bit counters for dropped_packets --> 1 Runner counter group: 17
 */
#define WAN0_TX_QUEUES_BYTES_GROUP                              14
#define WAN0_TX_QUEUES_PACKETS_GROUP                            15
#define WAN0_TX_QUEUES_DROPPED_BYTES_GROUP                      16
#define WAN0_TX_QUEUES_DROPPED_PACKETS_GROUP                    17

/* For Path Stats, we need:
 *     64 32bit counters for packets --> 4 Runner counter group: 20
 *     64 32bit counters for bytes --> 4 Runner counter group: 24
 */
#define PATHSTAT_PACKETS_GROUP                                  20
#define PATHSTAT_BYTES_GROUP                                    24

#define DOWNSTREAM_RX_DROPPED_SUMMARY_GROUP                     32
#define DHD_SSID_DROP_PACKET_GROUP                              37
#define UPSTREAM_TX_PACKETS_GROUP                               40
#define UPSTREAM_TX_BYTES_GROUP                                 56
#define UPSTREAM_TX_CONGESTION_GROUP                            72
#define BRIDGE_RX_BPM_CONGESTION_GROUP                          80
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
#define INGRESS_RATE_LIMITER_GROUP                              84
#define UPSTREAM_VARIOUS_PACKETS_GROUP                          86
#define DOWNSTREAM_VARIOUS_PACKETS_GROUP                        88
#define SERVICE_QUEUE_DROP_PACKET_GROUP                         90
#define CPU_RX_METERS_DROPPED_PACKETS_GROUP                     91
#define SERVICE_QUEUE_PACKET_GROUP                              92
#define SUBNET_RX_GROUP                                         92
#define SUBNET_RX_BYTES_GROUP                                   93
#define SUBNET_TX_GROUP                                         94
#define SUBNET_TX_BYTES_GROUP                                   95
#define CPU_RX_INTERRUPT_COALESCING_GROUP                       96

#define WAN_CRC_ERROR_NORMAL_COUNTER_OFFSET                     0
#define WAN_CRC_ERROR_IPTV_COUNTER_OFFSET                       1
#define WAN_BRIDGED_RX_VALID_SUB_GROUP_OFFSET                   1
#define WAN_IPTV_RX_VALID_SUB_GROUP_OFFSET                      2
#define LAN_SUBNET_COUNTER_OFFSET                               2
#define SUBNET_DROPPED_PACKETS_SUB_GROUP_OFFSET                 20
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
#define IH_THRESHOLD_CONGESTION_COUNTER_OFFSET                  36
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
#define DHD_IH_CONGESTION_OFFSET                                55 /* DHD_OFFLOAD */
#define DHD_MALLOC_FAILED_OFFSET                                56 /* DHD_OFFLOAD */
#define CPU_RX_METERS_DROPPED_PACKETS_UPSTREAM_OFFSET           16

#define BBH_RESET_WORD_1_DESCRIPTOR_VALUE                       0xF0000000
#define BBH_RESET_WORD_1_DESCRIPTOR_VALUE_MOD_8                 0xF0

#define CPU_RX_INTCOL_TOTALPKTS_OFFSET                          0
#define CPU_RX_INTCOL_MAXPKTS_OFFSET                            1
#define CPU_RX_INTCOL_TIMEOUTS_OFFSET                           2

#define BBH_PERIPHERAL_DSL_RX                                   0
#define BBH_PERIPHERAL_DSL_TX                                   32
#define BBH_PERIPHERAL_WAN_RX                                   BBH_PERIPHERAL_DSL_RX
#define BBH_PERIPHERAL_WAN_TX                                   BBH_PERIPHERAL_DSL_TX
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

#define SBPM_REPLY_SET_0                                        ( 0 << 7 )
#define SBPM_REPLY_SET_1                                        ( 1 << 7 )
#define SBPM_REPLY_SET_2                                        ( 2 << 7 )
#define SBPM_REPLY_SET_3                                        ( 3 << 7 )
#define SBPM_REPLY_SET_0_OFFSET                                 0
#define SBPM_REPLY_SET_1_OFFSET                                 32
#define SBPM_REPLY_SET_2_OFFSET                                 64
#define SBPM_REPLY_SET_3_OFFSET                                 96
#define SBPM_REPLY_GET_NEXT_OFFSET                              12

#define WAN_SRC_PORT                                            0
#define ETH0_SRC_PORT                                           1
#define ETH1_SRC_PORT                                           2
#define ETH4_SRC_PORT                                           5
#define MIPS_C_SRC_PORT                                         6
#define WAN_IPTV_SRC_PORT                                       7
#define ANY_SRC_PORT                                            11 /* used by Speed Service */
#define PCI_0_SRC_PORT                                          13 /* must match DRV_BPM_SP_SPARE_1 */
#define SPARE_0_SRC_PORT                                        12
#define FAST_RUNNER_A_SRC_PORT                                  14
#define FAST_RUNNER_B_SRC_PORT                                  15

#define RUNNER_CLUSTER_A_SRC_PORT                               14
#define RUNNER_CLUSTER_B_SRC_PORT                               15

#define RDD_DS_IH_PACKET_HEADROOM_OFFSET                        18
#define RDD_US_IH_PACKET_HEADROOM_OFFSET                        18
#define RDD_LAYER2_HEADER_MINIMUM_LENGTH                        14
#define RDD_RUNNER_FLOW_RUNNER_A_IH_BUFFER                      14
#define RDD_RUNNER_FLOW_RUNNER_B_IH_BUFFER                      15
#define RDD_RUNNER_A_IH_BUFFER_BBH_OFFSET                       ((RDD_RUNNER_FLOW_RUNNER_A_IH_BUFFER * 128 + RDD_DS_IH_PACKET_HEADROOM_OFFSET) / 8)
#define RDD_RUNNER_B_IH_BUFFER_BBH_OFFSET                       ((RDD_RUNNER_FLOW_RUNNER_B_IH_BUFFER * 128 + RDD_US_IH_PACKET_HEADROOM_OFFSET) / 8)
#define RDD_IH_BUFFER_BBH_ADDRESS                               0x8000
#define RDD_IH_HEADER_DESCRIPTOR_BBH_ADDRESS                    0x0

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

/* Per chip limitation, 63138/63148 SAR can only transmit
 * max PTM frame size of 1984 bytes including FCS (4 bytes).
 */
#define PTM_MAX_TX_FRAME_LEN_FCS    1984

/* UNIMAC message to enable/disable EEE LPI mode.*/
#define BBTX_EEE_MODE_CONFIG_MESSAGE                6

#ifdef RUNNER_A
#define PARALLEL_PROCESSING_SLAVE_VECTOR_ADDRESS            DS_PARALLEL_PROCESSING_SLAVE_VECTOR_ADDRESS
#define FLOW_CACHE_SLAVE0_THREAD_NUMBER                     DOWNSTREAM_FLOW_CACHE_SLAVE0_THREAD_NUMBER
#define PARALLEL_PROCESSING_IH_BUFFER_VECTOR_PTR_ADDRESS    DS_PARALLEL_PROCESSING_IH_BUFFER_VECTOR_PTR_ADDRESS
#else
#define PARALLEL_PROCESSING_SLAVE_VECTOR_ADDRESS            US_PARALLEL_PROCESSING_SLAVE_VECTOR_ADDRESS
#define FLOW_CACHE_SLAVE0_THREAD_NUMBER                     UPSTREAM_FLOW_CACHE_SLAVE0_THREAD_NUMBER
#define PARALLEL_PROCESSING_IH_BUFFER_VECTOR_PTR_ADDRESS    US_PARALLEL_PROCESSING_IH_BUFFER_VECTOR_PTR_ADDRESS
#endif

#ifdef RUNNER_A
#define SPDSVC_CONTEXT_TABLE_ADDRESS                        DS_SPDSVC_CONTEXT_TABLE_ADDRESS
#define SPDSVC_WAKEUP_REQUEST_VALUE                         DS_SPDSVC_WAKEUP_REQUEST_VALUE
#else
#define SPDSVC_CONTEXT_TABLE_ADDRESS                        US_SPDSVC_CONTEXT_TABLE_ADDRESS
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

/* DHD */
#define DHD_DOORBELL_IRQ_NUM                        2
#define DHD_MSG_TYPE_TX_POST                        0xF
#define DHD_MSG_TYPE_RX_POST                        0x11
#define DHD_TX_POST_FLOW_RING_SIZE                  2560
#define DHD_TX_POST_FLOW_RING_DESCRIPTOR_SIZE       48
#define DHD_TX_POST_FLOW_RING_SIZE_IN_BYTES         ( DHD_TX_POST_FLOW_RING_SIZE * DHD_TX_POST_FLOW_RING_DESCRIPTOR_SIZE )
#define DHD_RX_POST_FLOW_RING_SIZE                  1024
#define DHD_TX_COMPLETE_FLOW_RING_SIZE              1024
#define DHD_RX_COMPLETE_FLOW_RING_SIZE              1024
#define DHD_TX_POST_BUFFERS_THRESHOLD               2048
#define DHD_DATA_LEN                                2048
#define DHD_RX_POST_RING_NUMBER                     1  
#define DHD_TX_COMPLETE_RING_NUMBER                 3
#define DHD_RX_COMPLETE_RING_NUMBER                 4

#define DHD_RADIO_OFFSET_COMMON_A(index)            (DHD_RADIO_INSTANCE_COMMON_A_DATA_ADDRESS + (index * sizeof(RDD_DHD_RADIO_INSTANCE_COMMON_A_ENTRY_DTS)))
#define DHD_RADIO_OFFSET_COMMON_B(index)            (DHD_RADIO_INSTANCE_COMMON_B_DATA_ADDRESS + (index * sizeof(RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_DTS)))

#define UCAST_DROP_COMMAND_LIST_STRING_SIZE             16
#define UCAST_DROP_COMMAND_LIST_TIMESTAMP_SIZE          8
#define UCAST_DROP_COMMAND_LIST_TOTAL_SIZE              ( UCAST_DROP_COMMAND_LIST_STRING_SIZE + UCAST_DROP_COMMAND_LIST_TIMESTAMP_SIZE )

#define WLAN_MCAST_EGRESS_PORT_INDEX   7

/* IPsec operation errors */
#define IPSEC_OPERATION_OK         0
#define IPSEC_ICV_CHECK_FAIL       1

/* DS queues with a filling level less than this threshold can allocate Packet Descriptor using the guaranteed PD Pool budget */
#define DS_FREE_PACKET_DESCRIPTOR_POOL_GUARANTEED_QUEUE_THRESHOLD 8

/* The guaranteed PD Pool budget is sized in function of the number of configured DS queues. To avoid unexpected behavior in
 * corner cases, e.g. a transient condition when all DS queues are unconfigured, the guaranteed PD Pool budget is clamped
 * to at least this minimum guaranteed pool size value.
 */
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

#define INVALID_COUNTER_ID                                       0

#define IPPROTO_IDX_UNDEF      0 /*    Undefined, may be used for L2      */

#define IPPROTO_IDX_OTHER      1 /*  0 IPv6 ext: Hop-by-Hop Option Header 
                                    61 Any host internel proto            
                                   114 Any Zero HOP                      
                                   141 - 252 unassigned range          
                                   253 - 254 Reserved for experimentation
                                   255       Raw IP Packets               */
#define IPPROTO_IDX_IPV6       2 /* 41 IPv6-in-IPv4 tunneling             */
#define IPPROTO_IDX_GRE        3 /* 47 Cisco GRE tunnels (rfc 1701,1702)  */
#define IPPROTO_IDX_IPIP       4 /*  4 IPIP tunnels e.g. 4in6             */
#define IPPROTO_IDX_TCP_ACK    5 /*    TCP pure acknowledgment            */
#define IPPROTO_IDX_TCP        6 /*  6 Transmission Control Protocol      */
#define IPPROTO_IDX_UDP        7 /* 17 User Datagram Protocol             */

#define TUPLE_PROTO_PROTOCOL_F_OFFSET                   4
#define TUPLE_PROTO_PROTOCOL_F_WIDTH                    3
#define TUPLE_PROTO_PROTOCOL_F_MASK                     0x07
#define TUPLE_PROTO_PROTOCOL_OFFSET                     0
#define TUPLE_PROTO_LOOKUP_PORT_F_OFFSET                0
#define TUPLE_PROTO_LOOKUP_PORT_F_WIDTH                 4
#define TUPLE_PROTO_LOOKUP_PORT_F_MASK                  0x0F
#define TUPLE_PROTO_LOOKUP_PORT_OFFSET                  0

/* DEBUG TRACE */

/* #define DEBUG_TRACE_ENABLE */

#if defined(RUNNER_A) && defined(RUNNER_MAIN)
#define GLBREG R5
#define BASEADDR h.7a
#define MASKINST insert  GLBREG  R1  d.25  d.2
#elif defined(RUNNER_A) && defined(RUNNER_PICO)
#define GLBREG R6
#define BASEADDR h.7c
#define MASKINST insert  GLBREG  R0  d.25  d.1
#elif defined(RUNNER_B) && defined(RUNNER_MAIN)
#define GLBREG R6
#define BASEADDR h.ea
#define MASKINST insert  GLBREG  R1  d.25  d.2
#elif defined(RUNNER_B) && defined(RUNNER_PICO)
#define GLBREG R6
#define BASEADDR h.ec
#define MASKINST insert  GLBREG  R0  d.25  d.1
#endif

#if !defined(DEBUG_TRACE_ENABLE)

#define DEBUG_ID_TRACE24( id, trace_word )
#define DEBUG_ID_TRACE24_C( id, trace_word)
#define DEBUG_ID_TRACE24_CS( id, trace_word)
#define DEBUG_TRACE32( trace_word )
#define DEBUG_TRACE32_C( trace_word )
#define DEBUG_TRACE32_CS( trace_word )
#define DEBUG_ID_TRACE16_CS( id, trace_word_macro )
#define DEBUG_TRACE_C_ON()
#define DEBUG_TRACE_C_OFF()
#define DEBUG_TRACE_C_OFF_PERM()
#define DEBUG_TRACE_C_ON_PERM()
#define DEBUG_TRACE_MEM_C( mem, numbytes )
#define DEBUG_TRACE_C_TRIG_DELAYED_OFF_PERM()
#define DEBUG_TRACE_C_DO_DELAYED_OFF_PERM()

#else /* DEBUG_TRACE */

#define DEBUG_ID_TRACE24( id, trace_word ) ;\
    alu     GLBREG  GLBREG OR BASEADDR <<24 ;\
    alu     GLBREG  GLBREG AND~ h.ff ;\
    alu     GLBREG  GLBREG OR id ;\
    nop ;\
    stc32   trace_word  GLBREG  high ;\
    stc8    GLBREG  GLBREG  high ;\
    alu     GLBREG  GLBREG + d.4 <<16 ;\
    MASKINST ;\
    nop ;\
    stc32   R1  GLBREG  high

#define DEBUG_TRACE32( trace_word ) ;\
    alu     GLBREG  GLBREG OR BASEADDR <<24 ;\
    nop ;\
    stc32   trace_word  GLBREG  high ;\
    alu     GLBREG  GLBREG + d.4 <<16 ;\
    MASKINST ;\
    nop ;\
    stc32   R1  GLBREG  high


// tracec_on: 
//   bit 0 (1): if set, record traces
//   bit 1 (2): if set, do not enable bit 0 on TRACE_C_ON
//   bit 2 (4): if set, and TRACE_C_DELAYED_OFF_PERM called, set bit 1.


#define DEBUG_TRACE_C_ON()   DEBUG_TRACE_C_ON_0(__LINE__)
#define DEBUG_TRACE_C_ON_0(line)   DEBUG_TRACE_C_ON_1(line)
#define DEBUG_TRACE_C_ON_1(line)   \
    ldc32  tracec_on  TRACE_C_TABLE_ADDRESS ;\
    nop ;\
    alu R0 tracec_on AND h.6 ;\
    jmp!=0 :debug_trace_c_on_done_##line ds0 ;\
    alu    tracec_on  tracec_on OR h.1 ;\
    stc32  tracec_on  TRACE_C_TABLE_ADDRESS ;\
:debug_trace_c_on_done_##line
    
#define DEBUG_TRACE_C_OFF()   \
    ldc32  tracec_on  TRACE_C_TABLE_ADDRESS ;\
    nop ;\
    alu    tracec_on  tracec_on AND~ h.1 ;\
    stc32  tracec_on  TRACE_C_TABLE_ADDRESS

#define DEBUG_TRACE_C_OFF_PERM() \
    mov    tracec_on d.2 clear ;\
    stc32  tracec_on  TRACE_C_TABLE_ADDRESS

#define DEBUG_TRACE_C_ON_PERM() \
    mov    tracec_on d.1 clear ;\
    stc32  tracec_on  TRACE_C_TABLE_ADDRESS

#define DEBUG_TRACE_C_TRIG_DELAYED_OFF_PERM() \
    ldc32  tracec_on  TRACE_C_TABLE_ADDRESS ;\
    nop ;\
    nop ;\
    alu    tracec_on  tracec_on OR h.4 ;\
    stc32  tracec_on  TRACE_C_TABLE_ADDRESS


#define DEBUG_TRACE_C_DO_DELAYED_OFF_PERM()   DEBUG_TRACE_C_DO_DELAYED_OFF_PERM_0(__LINE__)
#define DEBUG_TRACE_C_DO_DELAYED_OFF_PERM_0(line)   DEBUG_TRACE_C_DO_DELAYED_OFF_PERM_1(line)
#define DEBUG_TRACE_C_DO_DELAYED_OFF_PERM_1(line)   \
    ldc32  tracec_on  TRACE_C_TABLE_ADDRESS ;\
    nop ;\
    alu R0 tracec_on AND h.4 ;\
    jmp=0  :debug_trace_c_do_delayed_off_perm_done_##line ;\
    mov    tracec_on  h.2 ;\
    stc32  tracec_on  TRACE_C_TABLE_ADDRESS ;\
:debug_trace_c_do_delayed_off_perm_done_##line



#define DEBUG_ID_TRACE24_C( id, trace_word ) DEBUG_ID_TRACE24_C_0( id, trace_word, __LINE__ )
#define DEBUG_ID_TRACE24_C_0( id, trace_word, line ) DEBUG_ID_TRACE24_C_1( id, trace_word, line )
#define DEBUG_ID_TRACE24_C_1( id, trace_word, line ) \
    ldc32   tracec_on  TRACE_C_TABLE_ADDRESS ;\
    nop ;\
    jmp_clr :debug_id_trace24_c_done_##line tracec_on d.0 ds0;\
    alu     GLBREG  GLBREG OR BASEADDR <<24 ;\
    alu     GLBREG  GLBREG AND~ h.ff ;\
    alu     GLBREG  GLBREG OR id ;\
    nop ;\
    stc32   trace_word  GLBREG  high ;\
    stc8    GLBREG  GLBREG  high ;\
    alu     GLBREG  GLBREG + d.4 <<16 ;\
    MASKINST ;\
    nop ;\
    stc32   R1  GLBREG  high ;\
:debug_id_trace24_c_done_##line

#define DEBUG_TRACE32_C( trace_word ) DEBUG_TRACE32_C_0( trace_word, __LINE__ )
#define DEBUG_TRACE32_C_0( trace_word, line ) DEBUG_TRACE32_C_1( trace_word, line )
#define DEBUG_TRACE32_C_1( trace_word, line ) \
    ldc32   tracec_on  TRACE_C_TABLE_ADDRESS ;\
    nop ;\
    jmp_clr :debug_trace32_c_done_##line tracec_on d.0 ds0;\
    stc32   trace_word  GLBREG  high ;\
    alu     GLBREG  GLBREG + d.4 <<16 ;\
    MASKINST ;\
    nop ;\
    stc32   R1  GLBREG  high; \
:debug_trace32_c_done_##line


#define DEBUG_ID_TRACE24_CS( id, trace_word_macro ) \
    alu  GLBREG GLBREG AND~ h.ff; \
    lcall :debug_id_trace24_cs ;\
    alu  trace_word trace_word_macro + d.0 ;\
    alu  GLBREG  GLBREG OR id

#define DEBUG_ID_TRACE16_CS( id, trace_word_macro ) \
    alu  GLBREG GLBREG AND~ h.ff; \
    mov  trace_word d.0 clear; \
    lcall :debug_id_trace24_cs ;\
    insert  trace_word trace_word_macro d.0 d.16; \
    alu  GLBREG  GLBREG OR id
    


#define DEBUG_TRACE32_CS( trace_word_macro ) \
    lcall :debug_trace32_cs ;\
    alu  trace_word trace_word_macro + d.0 ;\
    nop

#define DEBUG_TRACE_MEM_C( trace_ptr, size ) DEBUG_TRACE_MEM_C_0( trace_ptr, size, __LINE__ )
#define DEBUG_TRACE_MEM_C_0( trace_ptr, size, line ) DEBUG_TRACE_MEM_C_1( trace_ptr, size, line )
#define DEBUG_TRACE_MEM_C_1( trace_ptr, size, line ) \
        alu trace_ptr_local trace_ptr + d.0 ;\
        ldc32  tracec_on  TRACE_C_TABLE_ADDRESS ;\
        nop ;\
        jmp_clr :debug_id_mem_c_done_##line  tracec_on d.0 ds0 ;\
        alu     trace_end_ptr trace_ptr_local + size ;\
        nop ;\
:debug_id_mem_c_loop_##line ;\
        ld32    trace_word trace_ptr_local ;\
        alu     GLBREG  GLBREG OR BASEADDR <<24 ;\
        alu     trace_ptr_local trace_ptr_local + d.4 ;\
        stc32   trace_word  GLBREG  high ;\
        alu     GLBREG  GLBREG + d.4 <<16 ;\
        MASKINST ;\
        nop ;\
        jmp_cmp :debug_id_mem_c_loop_##line trace_end_ptr > trace_ptr_local  ;\
        nop ;\
        nop ;\
        stc32   R1  GLBREG  high ;\
:debug_id_mem_c_done_##line


#endif /* DEBUG_TRACE */


// Alternate use of debug trace memory space:  For fixed location counters and values instead of circular trace data
//#define DEBUG_TRACE_FIXED_COUNT_ENABLE

#if !defined(DEBUG_TRACE_FIXED_COUNT_ENABLE)

// Don't define them at all for the negative path unless debugging.  That way the compile will notify us of any left over.
// #define DEBUG_TRACE_COUNT(c)
// #define DEBUG_TRACE_WRITE(c,v)

#else

#if defined(DEBUG_TRACE_ENABLE)
#error DEBUG_TRACE_ENABLE and DEBUG_TRACE_FIXED_COUNT_ENABLE are mutually exclusive, but both are enabled.
#endif
    
#define DEBUG_TRACE_COUNT(c) \
DECLARE_FULL_REG_VAR ( temp_addr ) ;\
DECLARE_FULL_REG_VAR ( temp_word ) ;\
    mov     temp_addr  ((BASEADDR << 8) | (c << 2)) <<16 ;\
    nop ;\
    nop ;\
    ldc32   temp_word  temp_addr  high ;\
    alu     temp_word  temp_word + d.1 ;\
    stc32   temp_word  temp_addr  high

#define DEBUG_TRACE_COUNT_REGOFS(c, r, b) \
DECLARE_FULL_REG_VAR ( temp_addr ) ;\
DECLARE_FULL_REG_VAR ( temp_word ) ;\
    mov     temp_addr  ((BASEADDR << 8) | (c << 2)) <<16 ;\
    insert  temp_addr  r  d.18  b ;\
    nop ;\
    nop ;\
    ldc32   temp_word  temp_addr  high ;\
    alu     temp_word  temp_word + d.1 ;\
    stc32   temp_word  temp_addr  high

#define DEBUG_TRACE_WRITE(c,v) \
DECLARE_FULL_REG_VAR ( temp_addr ) ;\
    mov     temp_addr  ((BASEADDR << 8) | (c << 2)) <<16 ;\
    nop ;\
    nop ;\
    stc32   v  temp_addr  high

#endif /* DEBUG_TRACE_FIXED_COUNT_ENABLE */


/* FW TRACE */

/* Define the following to enable FWTRACE capability in both the Runner FW and driver. */
//#define RUNNER_FWTRACE

#ifdef RUNNER_FWTRACE
/* With the FW Trace tool, we'll run the Runner timer counter with a 20ns interval.  Otherwise, it is set to 1us. */
#define TIMER_PERIOD_NS                         20
#define TIMER_PERIOD_REG_VALUE                  15 /* 20ns */
/* 10ns will cause some timer limit register values to exceed 16-bits.   The developer must change them themselves to 0xFFFF to avoid more
    frequent timer wakeups than expected */
//#define TIMER_PERIOD_NS                         10
//#define TIMER_PERIOD_REG_VALUE                  8 /* 10ns */

/* These are the Runner FW Trace Options:  
        - RUNNER_FWTRACE_32BIT - Define to have 32-bit timestamps.   If not defined, FWTRACE will use 16-bit timestamps.   16-bit is better
           for a busy Runner snapshot as you won't wrap
        - FWTRACE_READ_TASKID - If defined, the FW Trace macro will read the thread ID from an I/O register.  If not defined, it will write the 
           event ID passed into the macro (built using EVID).
        - FWTRACE_ENABLE_TRACE_ALL - If defined, all FWTRACE_EVENTs through the Runner code are active.  This is useful for tracing everything
          with standard events (Entry, Exit, DMA start, DMA return).   With or without these events activated, developers can also use 
          FWTRACE_EVENT_DEV for user defined events.   
*/           
//#define RUNNER_FWTRACE_32BIT //- must match RDD define
#define FWTRACE_READ_TASKID
#define FWTRACE_ENABLE_TRACE_ALL
#define FWTRACE_FUNCTION

/* FW Tracing */
#define RUNNER_FWTRACE_ENABLE_BIT                   15
#define RUNNER_FWTRACE_ENABLE_MASK_SWAPPED          0x00000080
#define RUNNER_FWTRACE_ENABLE_MASK_SWAPPED_CLUSTER  0x00800080

/* Event Types */
#define FW_TRACE_THREAD_ENTRY                   1
#define FW_TRACE_THREAD_EXIT                    2
#define FW_TRACE_DMA_RD                         3
#define FW_TRACE_DMA_RD_RET                     4
#define FW_TRACE_DMA_WR                         5
#define FW_TRACE_DMA_WR_RET                     6
// Additional exit events to distinguish code points
#define FW_TRACE_THREAD_EXIT_2                  7
#define FW_TRACE_THREAD_EXIT_3                  8
#define FW_TRACE_THREAD_EXIT_4                  9
#define FW_TRACE_THREAD_EXIT_5                  10
#define FW_TRACE_THREAD_EXIT_6                  11
#define FW_TRACE_THREAD_EXIT_7                  12
#define FW_TRACE_THREAD_EXIT_8                  13
#define FW_TRACE_DMA_RD_2                       14
#define FW_TRACE_DMA_RD_RET_2                   15



#ifdef FWTRACE_READ_TASKID
#define EVID(ThreadNum, EventNum) (EventNum)
#else
#ifdef RUNNER_FWTRACE_32BIT
#define EVID(ThreadNum, EventNum) (EventNum)
#else
#define EVID(ThreadNum, EventNum) ((ThreadNum << 8) | (EventNum))
#endif
#endif

#ifdef RUNNER_A
#ifdef RUNNER_MAIN
    #define RUNNER_FWTRACE_CURR_OFFSET          RUNNER_FWTRACE_MAINA_CURR_OFFSET_ADDRESS
    #define RUNNER_FWTRACE_BASE                 RUNNER_FWTRACE_MAINA_BASE_ADDRESS
    #define RUNNER_FWTRACE_PARAM                RUNNER_FWTRACE_MAINA_PARAM_ADDRESS
#endif /* RUNNER_MAIN*/
#ifdef RUNNER_PICO
    #define RUNNER_FWTRACE_CURR_OFFSET          (RUNNER_FWTRACE_MAINA_CURR_OFFSET_ADDRESS+2)
    #define RUNNER_FWTRACE_BASE                 RUNNER_FWTRACE_PICOA_BASE_ADDRESS
    #define RUNNER_FWTRACE_PARAM                RUNNER_FWTRACE_PICOA_PARAM_ADDRESS
#endif /* RUNNER_PICO*/
#else /* RUNNER_B */
#ifdef RUNNER_MAIN
    #define RUNNER_FWTRACE_CURR_OFFSET          RUNNER_FWTRACE_MAINB_CURR_OFFSET_ADDRESS
    #define RUNNER_FWTRACE_BASE                 RUNNER_FWTRACE_MAINB_BASE_ADDRESS
    #define RUNNER_FWTRACE_PARAM                RUNNER_FWTRACE_MAINB_PARAM_ADDRESS
#endif /* RUNNER_MAIN*/
#ifdef RUNNER_PICO
    #define RUNNER_FWTRACE_CURR_OFFSET          (RUNNER_FWTRACE_MAINB_CURR_OFFSET_ADDRESS+2)
    #define RUNNER_FWTRACE_BASE                 RUNNER_FWTRACE_PICOB_BASE_ADDRESS
    #define RUNNER_FWTRACE_PARAM                RUNNER_FWTRACE_PICOB_PARAM_ADDRESS
#endif /* RUNNER_PICO*/
#endif /* IF RUNNER_A */


#ifdef FWTRACE_ENABLE_TRACE_ALL
#define FWTRACE_EVENT( task_id, eventId ) FWTRACE_EVENT_A( task_id, eventId, __LINE__ )
#else
#define FWTRACE_EVENT( task_id, eventId )
#endif
#define FWTRACE_EVENT_DEV( task_id, eventId ) FWTRACE_EVENT_A( task_id, eventId, __LINE__ )

#define FWTRACE_EVENT_A( task_id, eventId, line ) FWTRACE_EVENT_B( task_id, eventId, line )

/* Currently these macros are hardcoded for 128 entries for 32-bit and 256 for 16-bit timestamps. */
#ifndef FWTRACE_FUNCTION

#ifdef RUNNER_FWTRACE_32BIT
#ifdef FWTRACE_READ_TASKID
#define FWTRACE_EVENT_B( task_id, eventId, line ) ;\
    ldc16      rnr_fwtrace_tmp     RUNNER_FWTRACE_CURR_OFFSET ;\
    /* Move in base address for trace */;\
    mov       rnr_fwtrace_addr RUNNER_FWTRACE_BASE clear ;\
    /* Check if enabled */ ;\
    jmp_clr   :task_id##line##_fwtrace_done rnr_fwtrace_tmp RUNNER_FWTRACE_ENABLE_BIT ds1 ;\
    /* Get current offset address base for task.   Curr pointer is 8 bits in rnr_fwtrace_tmp */ ;\
    insert    rnr_fwtrace_addr rnr_fwtrace_tmp d.3 d.6 ;\
    /* Check if one-shot full */ ;\
    jmp_set   :task_id##line##_fwtrace_done   rnr_fwtrace_tmp d.7 ;\
    /* Increment pointer now as we require two nops before load */ ;\
    alu       rnr_fwtrace_tmp rnr_fwtrace_tmp + d.1 ;\
    stc16      rnr_fwtrace_tmp RUNNER_FWTRACE_CURR_OFFSET;\
    /* Load reg with taskId, which is bits 20:16 in the periphstatus reg.  Reuse enable register */ ;\
    ldio32    rnr_fwtrace_tmp PERIPHERALS_STATUS_REGISTER_IO_ADDRESS ;\
    shift     rnr_fwtrace_tmp asr rnr_fwtrace_tmp d.16 ;\
    /* Move thread to upper 16 bits, insert event ID into lower, then write */ ;\
    insert    rnr_fwtrace_tmp rnr_fwtrace_tmp d.16 d.5 ;\
    mov       rnr_fwtrace_tmp eventId ;\
    stc32      rnr_fwtrace_tmp rnr_fwtrace_addr ;\
    /* Write time */ ;\
    ldio32    rnr_fwtrace_tmp TIMER_VALUE_IO_ADDRESS ;\
    stc32      rnr_fwtrace_tmp rnr_fwtrace_addr d.4 ;\
:task_id##line##_fwtrace_done
#else
#define FWTRACE_EVENT_B( task_id, eventId, line ) ;\
    ldc16      rnr_fwtrace_tmp     RUNNER_FWTRACE_CURR_OFFSET ;\
    /* Move in base address for trace */;\
    mov       rnr_fwtrace_addr RUNNER_FWTRACE_BASE clear ;\
    /* Check if enabled */ ;\
    jmp_clr   :task_id##line##_fwtrace_done rnr_fwtrace_tmp RUNNER_FWTRACE_ENABLE_BIT ds1 ;\
    /* Get current offset address base for task.   Curr pointer is 8 bits in rnr_fwtrace_tmp */ ;\
    insert    rnr_fwtrace_addr rnr_fwtrace_tmp d.3 d.6 ;\
    /* Check if one-shot full */ ;\
    jmp_set   :task_id##line##_fwtrace_done   rnr_fwtrace_tmp d.7 ;\
    /* Increment pointer now as we require two nops before load */ ;\
    alu       rnr_fwtrace_tmp rnr_fwtrace_tmp + d.1 ;\
    stc16      rnr_fwtrace_tmp RUNNER_FWTRACE_CURR_OFFSET;\
    mov       rnr_fwtrace_tmp task_id <<16;\
    mov       rnr_fwtrace_tmp eventId ;\
    stc32      rnr_fwtrace_tmp rnr_fwtrace_addr ;\
    /* Write time */ ;\
    ldio32    rnr_fwtrace_tmp TIMER_VALUE_IO_ADDRESS ;\
    stc32      rnr_fwtrace_tmp rnr_fwtrace_addr d.4 ;\
:task_id##line##_fwtrace_done
#endif // Read Task Id
#else // 16 bit
#ifdef FWTRACE_READ_TASKID
#define FWTRACE_EVENT_B( task_id, eventId, line ) ;\
    ldc16      rnr_fwtrace_tmp     RUNNER_FWTRACE_CURR_OFFSET ;\
    /* Move in base address for trace */;\
    mov       rnr_fwtrace_addr RUNNER_FWTRACE_BASE clear ;\
    /* Check if enabled.  Bit 15 of 16-bit register */ ;\
    jmp_clr   :task_id##line##_fwtrace_done rnr_fwtrace_tmp RUNNER_FWTRACE_ENABLE_BIT ds1 ;\
    /* Get current offset address base for task.   Curr pointer is 8 bits in rnr_fwtrace_tmp */ ;\
    insert    rnr_fwtrace_addr rnr_fwtrace_tmp d.2 d.8 ;\
    /* Check if one-shot full */ ;\
    jmp_set   :task_id##line##_fwtrace_done   rnr_fwtrace_tmp d.8 ;\
    /* Increment pointer now as we require two nops before load */ ;\
    alu       rnr_fwtrace_tmp rnr_fwtrace_tmp + d.1 ;\
    stc16      rnr_fwtrace_tmp RUNNER_FWTRACE_CURR_OFFSET;\
    /* Load reg with taskId, which is bits 20:16 in the periphstatus reg. */ ;\
    /* Create 16 bit event ID + thread ID for write */ ;\
    ldio16    rnr_fwtrace_tmp (PERIPHERALS_STATUS_REGISTER_IO_ADDRESS+2) ;\
    mov       rnr_fwtrace_tmp eventId <<16;\
    insert    rnr_fwtrace_tmp rnr_fwtrace_tmp d.24 d.5 ;\
    shift     rnr_fwtrace_tmp asr rnr_fwtrace_tmp d.16 ;\
    /* Write event+thread value */ ;\
    stc16      rnr_fwtrace_tmp rnr_fwtrace_addr ;\
    alu       rnr_fwtrace_addr rnr_fwtrace_addr + d.2 ;\
    /* Load time and then write to next 16 bit address */ ;\
    ldio16    rnr_fwtrace_tmp TIMER_VALUE_IO_ADDRESS ;\
    /* Write time value */ ;\
    stc16      rnr_fwtrace_tmp rnr_fwtrace_addr ;\
:task_id##line##_fwtrace_done
#else
#define FWTRACE_EVENT_B( task_id, eventId, line ) ;\
    ldc16      rnr_fwtrace_tmp     RUNNER_FWTRACE_CURR_OFFSET ;\
    /* Move in base address for trace */;\
    mov       rnr_fwtrace_addr RUNNER_FWTRACE_BASE clear ;\
    /* Check if enabled */ ;\
    jmp_clr   :task_id##line##_fwtrace_done rnr_fwtrace_tmp RUNNER_FWTRACE_ENABLE_BIT ds1 ;\
    /* Get current offset address base for task.   Curr pointer is 8 bits in rnr_fwtrace_tmp */ ;\
    insert    rnr_fwtrace_addr rnr_fwtrace_tmp d.2 d.8 ;\
    /* Check if one-shot full */ ;\
    jmp_set   :task_id##line##_fwtrace_done   rnr_fwtrace_tmp d.8 ;\
    /* Increment pointer now as we require two nops before load */ ;\
    alu       rnr_fwtrace_tmp rnr_fwtrace_tmp + d.1 ;\
    stc16      rnr_fwtrace_tmp RUNNER_FWTRACE_CURR_OFFSET;\
    /* Load time and then merge in event into upper 16 bits */ ;\
    ldio32    rnr_fwtrace_tmp TIMER_VALUE_IO_ADDRESS ;\
    mov       rnr_fwtrace_tmp eventId <<16 ;\
    stc32      rnr_fwtrace_tmp rnr_fwtrace_addr ;\
:task_id##line##_fwtrace_done
#endif
#endif // 16 bit version

#else 
#define FWTRACE_EVENT_B( task_id, eventId, line ) ;\
    lcall     :fw_trace_event_b ;\
    mov      R6 eventId ;\
    st16     R6 RUNNER_FWTRACE_PARAM;\
:task_id##line##_fwtrace_done ;
#endif

#else /* RUNNER_FWTRACE not enabled */
#define FWTRACE_EVENT( task_id, eventId )
#define FWTRACE_EVENT_DEV( task_id, eventId )

#endif /* RUNNER_FWTRACE */

#endif

