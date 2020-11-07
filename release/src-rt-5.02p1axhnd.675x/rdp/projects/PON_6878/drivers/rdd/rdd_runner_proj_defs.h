/*
    <:copyright-BRCM:2014:DUAL/GPL:standard
    
       Copyright (c) 2014 Broadcom 
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

#ifndef _RDD_RUNNER_PROJ_DEFS_H
#define _RDD_RUNNER_PROJ_DEFS_H

#define NUM_OF_GLOBAL_REGS                                      0
#define NUM_OF_LOCAL_REGS                                       32
#define NUM_OF_MAIN_RUNNER_THREADS                              16
#define NUM_OF_RUNNER_CORES                                     3

/* TIMER DEFINES */
#define GHOST_REPORTING_TIMER_INTERVAL_IN_USEC                  40
#define FLUSH_TASK_TIMER_INTERVAL_IN_USEC                       15
#define DS_RATE_LIMITER_TIMER_PERIOD_IN_USEC                    1000
#define US_RATE_LIMITER_TIMER_PERIOD_IN_USEC                    4000
#define TIMER_COMMON_PERIOD_IN_USEC                             100
#define CPU_RX_METER_TIMER_PERIOD                               10000
#define CPU_RX_METER_TIMER_PERIOD_IN_USEC                       (CPU_RX_METER_TIMER_PERIOD)
#define GHOST_REPORTING_TIMER_INTERVAL                          (GHOST_REPORTING_TIMER_INTERVAL_IN_USEC)
#define FLUSH_TASK_TIMER_INTERVAL                               (FLUSH_TASK_TIMER_INTERVAL_IN_USEC)
#define DS_RATE_LIMITER_TIMER_PERIOD                            (DS_RATE_LIMITER_TIMER_PERIOD_IN_USEC)
#define US_RATE_LIMITER_TIMER_PERIOD                            (US_RATE_LIMITER_TIMER_PERIOD_IN_USEC)
#define TIMER_COMMON_PERIOD                                     (TIMER_COMMON_PERIOD_IN_USEC)

/* NATC */
#define NATC_16BYTE_KEY_MASK                                    0x8000

#define RDD_WAN0_VPORT       RDD_VPORT_ID_0
#define RDD_LAN0_VPORT       RDD_VPORT_ID_1
#define RDD_LAN1_VPORT       RDD_VPORT_ID_2
#define RDD_LAN2_VPORT       RDD_VPORT_ID_3
#define RDD_LAN3_VPORT       RDD_VPORT_ID_4
#define RDD_LAN4_VPORT       RDD_VPORT_ID_5
#define RDD_LAN5_VPORT       RDD_VPORT_ID_6
#define RDD_LAN6_VPORT       RDD_VPORT_ID_7
#define RDD_LAN_VPORT_LAST   RDD_LAN6_VPORT

#define RDD_CPU0_VPORT       (RDD_LAN_VPORT_LAST + 1)
#define RDD_CPU_VPORT_FIRST  RDD_CPU0_VPORT

#define RDD_CPU_VPORT_FIRST  RDD_CPU0_VPORT
#define RDD_CPU1_VPORT       (RDD_CPU_VPORT_FIRST + 1)
#define RDD_CPU2_VPORT       (RDD_CPU_VPORT_FIRST + 2)
#define RDD_CPU3_VPORT       (RDD_CPU_VPORT_FIRST + 3)
#define RDD_CPU4_VPORT       (RDD_CPU_VPORT_FIRST + 4)
#define RDD_WLAN0_VPORT      RDD_CPU4_VPORT
#define RDD_CPU5_VPORT       (RDD_CPU_VPORT_FIRST + 5)
#define RDD_WLAN1_VPORT      RDD_CPU5_VPORT
#define RDD_CPU6_VPORT       (RDD_CPU_VPORT_FIRST + 6)
#define RDD_WLAN2_VPORT      RDD_CPU6_VPORT
#define RDD_CPU_VPORT_FLOOD  (RDD_CPU_VPORT_FIRST + 7)
#define RDD_CPU_VPORT_LAST   RDD_CPU_VPORT_FLOOD
#define RDD_CPU_VPORT_MASK ((1 << RDD_CPU0_VPORT) | (1 << RDD_CPU1_VPORT) | (1 << RDD_CPU2_VPORT) | \
    (1 << RDD_CPU3_VPORT) | (1 << RDD_CPU4_VPORT) | (1 << RDD_CPU5_VPORT) | (1 << RDD_CPU6_VPORT) | \
    (1 << RDD_CPU_VPORT_FLOOD))

/* TM */
#define DS_TM_UPDATE_FIFO_THREAD_NUMBER  IMAGE_0_TM_UPDATE_FIFO_DS_THREAD_NUMBER
#define TM_FLUSH_THREAD_NUMBER           IMAGE_0_TM_FLUSH_THREAD_NUMBER

#define DS_TM_BBH_QUEUE_TABLE_SIZE       RDD_IMAGE_0_DS_TM_BBH_QUEUE_TABLE_SIZE
#define US_TM_UPDATE_FIFO_THREAD_NUMBER  IMAGE_0_TM_UPDATE_FIFO_US_THREAD_NUMBER
#define RDD_BBH_QUEUE_TABLE_SIZE  40
#define RDD_TM_FLOW_CNTR_TABLE_SIZE      RDD_IMAGE_0_US_TM_TM_FLOW_CNTR_TABLE_SIZE
#define RDD_TM_ACTION_PTR_TABLE_SIZE     RDD_US_TM_TM_ACTION_PTR_TABLE_SIZE
#define RDD_BASIC_SCHEDULER_TABLE_SIZE   RDD_BASIC_SCHEDULER_TABLE_US_SIZE
#define US_TM_BBH_QUEUE_TABLE_SIZE       RDD_IMAGE_0_US_TM_BBH_QUEUE_TABLE_SIZE
#define RDD_BASIC_RATE_LIMITER_TABLE_SIZE RDD_BASIC_RATE_LIMITER_TABLE_US_SIZE

#define TIMER_COMMON_THREAD_NUMBER       IMAGE_2_IMAGE_2_TIMER_COMMON_THREAD_NUMBER

/* REPORTING */
#define REPORTING_THREAD_NUMBER     IMAGE_0_TM_REPORTING_THREAD_NUMBER
#define REPORTING_COUNTER_ADDRESS   IMAGE_0_REPORTING_COUNTER_TABLE_ADDRESS

/* Ingress Filters */
#define INGRESS_FILTER_L2_REASON_TABLE_SIZE RDD_IMAGE_1_INGRESS_FILTER_L2_REASON_TABLE_SIZE

/* CPU RX */
#define INTERRUPT_COALESCING_THREAD_NUMBER IMAGE_1_PROCESSING_INTERRUPT_COALESCING_THREAD_NUMBER
#define CPU_RX_METER_BUDGET_ALLOCATOR_THREAD_NUMBER IMAGE_1_PROCESSING_CPU_RX_METER_BUDGET_ALLOCATOR_THREAD_NUMBER
#define CPU_RX_THREAD_NUMBER IMAGE_1_PROCESSING_CPU_RX_THREAD_NUMBER
#define CPU_RX_COPY_THREAD_NUMBER           IMAGE_1_PROCESSING_CPU_RX_COPY_THREAD_NUMBER
#define PKTGEN_TX_THREAD_NUMBER             IMAGE_1_PROCESSING_CPU_RX_COPY_THREAD_NUMBER
#define CPU_RX_METER_BUDGET_ALLOCATOR_TASK                      IMAGE_1_PROCESSING_CPU_RX_METER_BUDGET_ALLOCATOR_THREAD_NUMBER
#define CPU_RX_METER_BUDGET_ALLOCATOR_TIMER                     IMAGE_1_CPU_IF_1_TIMER_INDEX_CPU_RX_METER_BUDGET_ALLOCATOR
#define SPDSVC_GEN_THREAD_NUMBER            IMAGE_1_PROCESSING_SPDSVC_GEN_THREAD_NUMBER

/* CPU TX */
#define RECYCLE_INTERRUPT_COALESCING_THREAD_NUMBER IMAGE_2_IMAGE_2_INTERRUPT_COALESCING_THREAD_NUMBER
#define CPU_TX_0_THREAD_NUMBER IMAGE_2_IMAGE_2_CPU_TX_0_THREAD_NUMBER
#define CPU_TX_SINGLE_TASK

#define CPU_TX_SYNC_FIFO_ENTRY_BYTE_SIZE 8 /* should be same as RDD_CPU_TX_SYNC_FIFO_ENTRY_BYTE_SIZE */
#define SPDSVC_GEN_THREAD_NUMBER  IMAGE_1_PROCESSING_SPDSVC_GEN_THREAD_NUMBER

/* Ingress classifier and VLAN actions */
#define RDD_US_IC_RULE_CFG_TABLE_SIZE                           128
#define RDD_DS_IC_RULE_CFG_TABLE_SIZE                           128
#define RDD_DS_DEFAULT_FLOW_CONTEXT_TABLE_SIZE                  128
#define RDD_US_DEFAULT_FLOW_CONTEXT_TABLE_SIZE                  8
#define RDD_DEFAULT_FLOW_CONTEXT_TABLE_SIZE                     (RDD_DS_DEFAULT_FLOW_CONTEXT_TABLE_SIZE + RDD_US_DEFAULT_FLOW_CONTEXT_TABLE_SIZE)
#define RDD_IC_SHARED_CONTEXT_TABLE_SIZE                        512
#define RDD_DS_IC_CONTEXT_TABLE_SIZE                            RDD_IC_SHARED_CONTEXT_TABLE_SIZE
#define RDD_US_IC_CONTEXT_TABLE_SIZE                            RDD_IC_SHARED_CONTEXT_TABLE_SIZE
#define NUM_OF_GENERIC_RULE_CFG                                 4
#define RDD_VLAN_COMMAND_SKIP                                   128
#define QM_QUEUE_DROP                                           0xFF

#endif
