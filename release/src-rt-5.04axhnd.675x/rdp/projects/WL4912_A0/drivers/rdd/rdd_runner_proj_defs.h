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

#ifndef RDD_RUNNER_PROJ_DEFS_H_
#define RDD_RUNNER_PROJ_DEFS_H_

#ifndef __PACKING_ATTRIBUTE_STRUCT_END__
#define __PACKING_ATTRIBUTE_STRUCT_END__        __attribute__ ((packed))
#endif
#ifndef __PACKING_ATTRIBUTE_FIELD_LEVEL__
#define __PACKING_ATTRIBUTE_FIELD_LEVEL__
#endif

#define NUM_OF_LOCAL_REGS                                       32
#define NUM_OF_MAIN_RUNNER_THREADS                              16
#define NUM_OF_RUNNER_CORES                                     GROUPED_EN_SEGMENTS_NUM

/* STUBS */
#define RDD_DSL_WAN_VPORT                0xffff
#define RDD_DSL_RX_FLOW_ID(_rx_flow_id)  (-1)
#define RDD_TM_DSL_FLOW_ID(_dsl_flow_id) (-1)
#define RDD_TX_DSL_FLOW_ID(_tm_flow_id)  (-1)
#define RDD_IS_TM_DSL_FLOW_ID(_flow_id)  (0)

#define RDD_DSL_TX_FLOW_TABLE_SIZE 0
#define RDD_DSL_TX_FLOW_TABLE_ADDRESS_ARR (uint32_t *) 0
#define RDD_US_CHANNEL_OFFSET_DSL 0

#define BBH_ID_DSL    BBH_ID_NULL
#define BBH_ID_PON    BBH_ID_NULL
#define BBH_TX_ID_DSL BBH_ID_NULL
#define BBH_TX_ID_PON BBH_ID_NULL


/* TIMER DEFINES */
/* XXX: bug in A0 timer frequency is half the runner frequency */
#define GHOST_REPORTING_TIMER_INTERVAL_IN_USEC                  40
#define FLUSH_TASK_TIMER_INTERVAL_IN_USEC                       15
#define DS_RATE_LIMITER_TIMER_PERIOD_IN_USEC                    1000
#define US_RATE_LIMITER_TIMER_PERIOD_IN_USEC                    1000
#define CPU_RX_METER_TIMER_PERIOD                               10000
#define CPU_RX_METER_TIMER_PERIOD_IN_USEC                       CPU_RX_METER_TIMER_PERIOD
#define GHOST_REPORTING_TIMER_INTERVAL                          GHOST_REPORTING_TIMER_INTERVAL_IN_USEC
#define FLUSH_TASK_TIMER_INTERVAL                               FLUSH_TASK_TIMER_INTERVAL_IN_USEC
#define DS_RATE_LIMITER_TIMER_PERIOD                            DS_RATE_LIMITER_TIMER_PERIOD_IN_USEC
#define US_RATE_LIMITER_TIMER_PERIOD                            US_RATE_LIMITER_TIMER_PERIOD_IN_USEC


/* NATC */
/* bit 0 of KEY_MASK register[0] corresponds to key bit 0, 
 * bit 31 of KEY_MASK register [7] corresponds to key bit 255 */
#define NATC_16BYTE_KEY_MASK                                    {.data={0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff}}

#define ag_drv_natc_cfg_cli_init        ag_drv_natc_tbl_cli_init
#define ag_drv_natc_cfg_key_addr_set    ag_drv_natc_tbl_key_addr_set
#define ag_drv_natc_cfg_res_addr_set    ag_drv_natc_tbl_res_addr_set

#define cli_natc_cfg_key_addr           cli_natc_tbl_key_addr
#define cli_natc_cfg_res_addr           cli_natc_tbl_res_addr

#define RDD_WAN0_VPORT       RDD_VPORT_ID_0
#define RDD_LAN0_VPORT       RDD_VPORT_ID_1
#define RDD_LAN1_VPORT       RDD_VPORT_ID_2
#define RDD_LAN2_VPORT       RDD_VPORT_ID_3
#define RDD_LAN3_VPORT       RDD_VPORT_ID_4
#define RDD_LAN4_VPORT       RDD_VPORT_ID_5
#define RDD_LAN5_VPORT       RDD_VPORT_ID_6
#define RDD_LAN6_VPORT       RDD_VPORT_ID_7
#define RDD_LAN7_VPORT       RDD_VPORT_ID_8
#define RDD_LAN_VPORT_LAST   RDD_LAN7_VPORT
#define RDD_VPORT_ANY        RDD_LAN_VPORT_LAST

#define RDD_CPU0_VPORT       (RDD_LAN_VPORT_LAST + 1)  /* ID_9 */
#define RDD_CPU_VPORT_FIRST  RDD_CPU0_VPORT
#define RDD_CPU1_VPORT       (RDD_CPU_VPORT_FIRST + 1)  /* ID_10 */
#define RDD_CPU2_VPORT       (RDD_CPU_VPORT_FIRST + 2)  /* ID_11 */
#define RDD_CPU3_VPORT       (RDD_CPU_VPORT_FIRST + 3)  /* ID_12 */
#define RDD_CPU4_VPORT       (RDD_CPU_VPORT_FIRST + 4)  /* ID_13 */
#define RDD_WLAN0_VPORT      RDD_CPU4_VPORT
#define RDD_CPU5_VPORT       (RDD_CPU_VPORT_FIRST + 5)  /* ID_14 */
#define RDD_WLAN1_VPORT      RDD_CPU5_VPORT
#define RDD_CPU6_VPORT       (RDD_CPU_VPORT_FIRST + 6)  /* ID_15 */
#define RDD_WLAN2_VPORT      RDD_CPU6_VPORT
#define RDD_CPU_VPORT_FLOOD  (RDD_CPU_VPORT_FIRST + 7)  /* ID_16 */
#define RDD_CPU_VPORT_LAST   RDD_CPU_VPORT_FLOOD

#define RDD_WAN1_VPORT       (RDD_CPU_VPORT_LAST + 1)   /* ID_17 */
#define RDD_VIRTUAL_VPORT    (PROJ_DEFS_RDD_VPORT_LAST - 1)

/* RDD and FW should independently manage/map WAN0/WAN1
 * Upper layers i.e. RDPA should work with ETH/DSL */

#define RDD_ETH_WAN_VPORT    RDD_WAN0_VPORT

#define RDD_CPU_VPORT_MASK ((1 << RDD_CPU0_VPORT) | (1 << RDD_CPU1_VPORT) | (1 << RDD_CPU2_VPORT) | \
    (1 << RDD_CPU3_VPORT) | (1 << RDD_CPU4_VPORT) | (1 << RDD_CPU5_VPORT) | (1 << RDD_CPU6_VPORT) | \
    (1 << RDD_CPU_VPORT_FLOOD))

#define RDD_VPORT_ID(id) (1 << id)
#define RDD_WLAN_VPORT_MASK ((1 << RDD_WLAN0_VPORT) | (1 << RDD_WLAN1_VPORT) | (1 << RDD_WLAN2_VPORT))

/* RX FLOW ID management */

/* TX FLOW ID/Table Management */

#define RDD_TX_FLOW_TABLE_SIZE           (RDD_VPORT_TX_FLOW_TABLE_SIZE)
#define RDD_WAN_TX_FLOW_TABLE_SIZE       (RDD_VPORT_TX_FLOW_TABLE_SIZE)

#define RDD_TX_US_ETH_FLOW_CNTR_ID(_tm_flow_id)    (tm_flow_id)

/* TM */
#define DS_TM_UPDATE_FIFO_THREAD_NUMBER  IMAGE_0_IMAGE_0_UPDATE_FIFO_THREAD_NUMBER
#define DS_TM_BBH_QUEUE_TABLE_SIZE       RDD_IMAGE_0_DS_TM_BBH_QUEUE_TABLE_SIZE
#define US_TM_UPDATE_FIFO_THREAD_NUMBER  IMAGE_1_IMAGE_1_UPDATE_FIFO_THREAD_NUMBER

#define RDD_BBH_QUEUE_TABLE_SIZE         RDD_US_TM_BBH_QUEUE_TABLE_SIZE
#define RDD_TM_FLOW_CNTR_TABLE_SIZE      RDD_US_TM_TM_FLOW_CNTR_TABLE_SIZE
#define RDD_US_TM_FLOW_CNTR_ETH_OFFSET   0
#define RDD_TM_ACTION_PTR_TABLE_SIZE     RDD_US_TM_TM_ACTION_PTR_TABLE_SIZE
#define RDD_BASIC_SCHEDULER_TABLE_MAX_SIZE (RDD_BASIC_SCHEDULER_TABLE_US_SIZE>RDD_BASIC_SCHEDULER_TABLE_DS_SIZE?RDD_BASIC_SCHEDULER_TABLE_US_SIZE:RDD_BASIC_SCHEDULER_TABLE_DS_SIZE)
#define US_TM_BBH_QUEUE_TABLE_SIZE       RDD_IMAGE_1_US_TM_BBH_QUEUE_TABLE_SIZE

#define RDD_DS_TM_FLOW_CNTR_TABLE_SIZE   RDD_IMAGE_0_DS_TM_TM_FLOW_CNTR_TABLE_SIZE
/* Ingress Filters */
#define INGRESS_FILTER_L2_REASON_TABLE_SIZE RDD_IMAGE_2_INGRESS_FILTER_L2_REASON_TABLE_SIZE

/* CPU RX */
#define CPU_RX_THREAD_NUMBER                IMAGE_3_IMAGE_3_CPU_RX_THREAD_NUMBER
#define CPU_RX_COPY_THREAD_NUMBER           IMAGE_3_IMAGE_3_CPU_RX_COPY_THREAD_NUMBER
#define PKTGEN_TX_THREAD_NUMBER             IMAGE_3_IMAGE_3_CPU_RX_COPY_THREAD_NUMBER

#define INTERRUPT_COALESCING_THREAD_NUMBER IMAGE_3_IMAGE_3_INTERRUPT_COALESCING_THREAD_NUMBER
#define CPU_RX_METER_BUDGET_ALLOCATOR_THREAD_NUMBER IMAGE_3_IMAGE_3_CPU_RX_METER_BUDGET_ALLOCATOR_THREAD_NUMBER
#define CPU_RX_METER_BUDGET_ALLOCATOR_TIMER         IMAGE_3_CPU_IF_3_TIMER_INDEX_CPU_RX_METER_BUDGET_ALLOCATOR
#define SPDSVC_GEN_THREAD_NUMBER            IMAGE_3_IMAGE_3_SPDSVC_GEN_THREAD_NUMBER

/* CPU TX */
#define RECYCLE_INTERRUPT_COALESCING_THREAD_NUMBER IMAGE_2_IMAGE_2_INTERRUPT_COALESCING_THREAD_NUMBER
#define CPU_TX_0_THREAD_NUMBER IMAGE_2_IMAGE_2_CPU_TX_0_THREAD_NUMBER

#define CPU_TX_SYNC_FIFO_ENTRY_BYTE_SIZE 8 /* should be same as RDD_CPU_TX_SYNC_FIFO_ENTRY_BYTE_SIZE */

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

#define BBH_ID_PON  BBH_ID_NULL

#endif

