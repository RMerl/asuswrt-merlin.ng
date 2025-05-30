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

#define NUM_OF_LOCAL_REGS                                       32
#define NUM_OF_MAIN_RUNNER_THREADS                              16

/* STUBS */
#define RDD_DSL_WAN_VPORT                0xffff
#define RDD_DSL_RX_FLOW_ID(_rx_flow_id)  (-1)
#define RDD_TM_DSL_FLOW_ID(_dsl_flow_id) (-1)
#define RDD_TX_DSL_FLOW_ID(_tm_flow_id)  (-1)
#define RDD_IS_TM_DSL_FLOW_ID(_flow_id)  (0)

#define RDD_DSL_TX_FLOW_TABLE_SIZE 0
#define RDD_DSL_TX_FLOW_TABLE_ADDRESS_ARR ((uint32_t *)0)
#define RDD_US_CHANNEL_OFFSET_DSL 0

#define BBH_ID_DSL    BBH_ID_NULL
#define BBH_TX_ID_DSL BBH_ID_NULL


/* TIMER DEFINES */
/* XXX: bug in A0 timer frequency is half the runner frequency */
#define GHOST_REPORTING_TIMER_INTERVAL_IN_USEC                  40
#define FLUSH_TASK_TIMER_INTERVAL_IN_USEC                       15
#define DS_RATE_LIMITER_TIMER_PERIOD_IN_USEC                    100
#define US_RATE_LIMITER_TIMER_PERIOD_IN_USEC                    100
#define CPU_RX_METER_TIMER_PERIOD                               10000
#define CPU_RX_METER_TIMER_PERIOD_IN_USEC                       CPU_RX_METER_TIMER_PERIOD
#define GHOST_REPORTING_TIMER_INTERVAL                          GHOST_REPORTING_TIMER_INTERVAL_IN_USEC
#define FLUSH_TASK_TIMER_INTERVAL                               FLUSH_TASK_TIMER_INTERVAL_IN_USEC
#define DS_RATE_LIMITER_TIMER_PERIOD                            DS_RATE_LIMITER_TIMER_PERIOD_IN_USEC
#define US_RATE_LIMITER_TIMER_PERIOD                            US_RATE_LIMITER_TIMER_PERIOD_IN_USEC


/* NATC */
/* bit 0 of KEY_MASK register[0] corresponds to key bit 0, 
 * bit 31 of KEY_MASK register [7] corresponds to key bit 255 */
#define NATC_16BYTE_KEY_MASK            {.data = {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff} }

#define ag_drv_natc_cfg_cli_init        ag_drv_natc_tbl_cli_init
#define ag_drv_natc_cfg_key_addr_set    ag_drv_natc_tbl_key_addr_set
#define ag_drv_natc_cfg_res_addr_set    ag_drv_natc_tbl_res_addr_set

#define cli_natc_cfg_key_addr           cli_natc_tbl_key_addr
#define cli_natc_cfg_res_addr           cli_natc_tbl_res_addr

#define RDD_LAN0_VPORT       RDD_VPORT_ID_0
#define RDD_LAN1_VPORT       RDD_VPORT_ID_1
#define RDD_LAN2_VPORT       RDD_VPORT_ID_2
#define RDD_LAN3_VPORT       RDD_VPORT_ID_3
#define RDD_LAN4_VPORT       RDD_VPORT_ID_4
#define RDD_LAN5_VPORT       RDD_VPORT_ID_5
#define RDD_LAN6_VPORT       RDD_VPORT_ID_6
#define RDD_LAN7_VPORT       RDD_VPORT_ID_7
#define RDD_LAN8_VPORT       RDD_VPORT_ID_8
#define RDD_LAN9_VPORT       RDD_VPORT_ID_9
#define RDD_LAN10_VPORT      RDD_VPORT_ID_10
#define RDD_LAN_VPORT_LAST   RDD_LAN10_VPORT

#define RDD_WAN0_VPORT       (RDD_LAN_VPORT_LAST + 1)

#define RDD_CPU0_VPORT       (RDD_WAN0_VPORT + 1)  /* ID_12 */
#define RDD_CPU_VPORT_FIRST  RDD_CPU0_VPORT
#define RDD_CPU1_VPORT       (RDD_CPU_VPORT_FIRST + 1)  /* ID_13 */
#define RDD_CPU2_VPORT       (RDD_CPU_VPORT_FIRST + 2)  /* ID_14 */
#define RDD_GDX_VPORT        RDD_CPU2_VPORT
#define RDD_CPU3_VPORT       (RDD_CPU_VPORT_FIRST + 3)  /* ID_15 */
#define RDD_SPU_VPORT        RDD_CPU3_VPORT             /* SPU using VPORT for CPU3 */
#define RDD_CPU4_VPORT       (RDD_CPU_VPORT_FIRST + 4)  /* ID_16 */
#define RDD_WLAN0_VPORT      RDD_CPU4_VPORT
#define RDD_CPU5_VPORT       (RDD_CPU_VPORT_FIRST + 5)  /* ID_17 */
#define RDD_WLAN1_VPORT      RDD_CPU5_VPORT
#define RDD_CPU6_VPORT       (RDD_CPU_VPORT_FIRST + 6)  /* ID_18 */
#define RDD_WLAN2_VPORT      RDD_CPU6_VPORT
#define RDD_CPU7_VPORT       (RDD_CPU_VPORT_FIRST + 7)  /* ID_19 */
#define RDD_WLAN3_VPORT      RDD_CPU7_VPORT
#define RDD_CPU_VPORT_LAST   RDD_WLAN3_VPORT

#define RDD_WAN1_VPORT       (RDD_CPU_VPORT_LAST + 1)   /* ID_20 */

#define RDD_BOND0_VPORT      (RDD_WAN1_VPORT + 1) 
#define RDD_BOND1_VPORT      (RDD_WAN1_VPORT + 2) 

#define RDD_VPORT_MAX RDD_BOND1_VPORT

/* RDD and FW should independently manage/map WAN0/WAN1
 * Upper layers i.e. RDPA should work with ETH/DSL */

#define RDD_ETH_WAN_VPORT    RDD_WAN0_VPORT
#define RDD_PON_WAN_VPORT    RDD_WAN0_VPORT

#define BB_ID_TX_PON_ETH_PD    BB_ID_TX_PON_PD
#define BB_ID_TX_PON_ETH_STAT  BB_ID_TX_PON_STAT

#define RDD_CPU_VPORT_MASK ((1 << RDD_CPU0_VPORT) | (1 << RDD_CPU1_VPORT) | (1 << RDD_CPU2_VPORT) | \
    (1 << RDD_CPU3_VPORT) | (1 << RDD_CPU4_VPORT) | (1 << RDD_CPU5_VPORT) | (1 << RDD_CPU6_VPORT))

#define RDD_VPORT_ID(id) (1 << id)
#define RDD_WLAN_VPORT_MASK ((1 << RDD_WLAN0_VPORT) | (1 << RDD_WLAN1_VPORT) | (1 << RDD_WLAN2_VPORT))

/* RX FLOW ID management */
#define RDD_PON_RX_FLOW_ID(_rx_flow_id)  (_rx_flow_id + RDD_WAN_FLOW_PON_START)

/* TX FLOW ID/Table Management */

#define RDD_WAN_TX_FLOW_TABLE_SIZE       (RDD_PON_TX_FLOW_TABLE_SIZE)

#define RDD_TM_PON_FLOW_ID(_gem_id)      (_gem_id)
#define RDD_TX_PON_FLOW_ID(_tm_flow_id)  (_tm_flow_id)
#define RDD_IS_TM_PON_FLOW_ID(_flow_id)  (_flow_id < RDD_PON_TX_FLOW_TABLE_SIZE)
#define RDD_TX_US_ETH_FLOW_CNTR_ID(_tm_flow_id)  (_tm_flow_id - RDD_PON_TX_FLOW_TABLE_SIZE)

/* TM */
#define RDD_ETH_TM_SCHEDULER_POOL_SIZE			RDD_DS_TM_SCHEDULER_POOL_SIZE
#define RDD_PON_TM_SCHEDULER_POOL_SIZE			RDD_US_TM_SCHEDULER_POOL_SIZE
#define RDD_ETH_SQ_TM_SCHEDULER_POOL_SIZE		RDD_DS_TM_SCHEDULER_POOL_SIZE

#define RDD_TM_FLOW_CNTR_TABLE_SIZE             (RDD_US_TM_PON_TM_FLOW_CNTR_TABLE_SIZE + RDD_US_TM_TM_FLOW_CNTR_TABLE_SIZE)
#define DS_TM_BBH_QUEUE_TABLE_SIZE              RDD_IMAGE_7_DS_TM_BBH_QUEUE_TABLE_SIZE

#define RDD_SCHEDULER_TABLE_MAX_SIZE            (RDD_US_TM_SCHEDULER_TABLE_SIZE > RDD_DS_TM_SCHEDULER_TABLE_SIZE ? \
    RDD_US_TM_SCHEDULER_TABLE_SIZE : RDD_DS_TM_SCHEDULER_TABLE_SIZE)
#define RDD_BASIC_SCHEDULER_TABLE_MAX_SIZE      RDD_SCHEDULER_TABLE_MAX_SIZE
#define RDD_SECONDARY_SCHEDULER_TABLE_MAX_SIZE  (RDD_US_TM_SECONDARY_SCHEDULER_TABLE_SIZE > RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_SIZE ? \
                                                    RDD_US_TM_SECONDARY_SCHEDULER_TABLE_SIZE : RDD_DS_TM_SECONDARY_SCHEDULER_TABLE_SIZE)
#define RDD_COMPLEX_SCHEDULER_TABLE_MAX_SIZE    0

#define DS_TM_BUDGET_ALLOCATION_THREAD_NUMBER   IMAGE_7_IMAGE_7_GENERAL_TIMER_THREAD_NUMBER
#define TM_BUDGET_ALLOCATION_US_THREAD_NUMBER   IMAGE_6_IMAGE_6_BUDGET_ALLOCATION_THREAD_NUMBER

#define RDD_DS_TM_FLOW_CNTR_TABLE_SIZE          RDD_IMAGE_7_DS_TM_TM_FLOW_CNTR_TABLE_SIZE

#define DS_TM_UPDATE_FIFO_THREAD_NUMBER  IMAGE_7_IMAGE_7_UPDATE_FIFO_THREAD_NUMBER
#define US_TM_UPDATE_FIFO_THREAD_NUMBER  IMAGE_6_IMAGE_6_UPDATE_FIFO_THREAD_NUMBER

#define DS_TM_BUDGET_ALLOCATION_DS_THREAD_NUMBER  IMAGE_7_IMAGE_7_BUDGET_ALLOCATION_DS_THREAD_NUMBER
#define US_TM_BUDGET_ALLOCATION_US_THREAD_NUMBER  IMAGE_6_IMAGE_6_BUDGET_ALLOCATION_US_THREAD_NUMBER
#define DS_TM_FLUSH_THREAD_NUMBER                 IMAGE_7_IMAGE_7_DS_FLUSH_THREAD_NUMBER
#define US_TM_FLUSH_THREAD_NUMBER                 IMAGE_6_IMAGE_6_US_FLUSH_THREAD_NUMBER
#define US_TM_OVL_BUDGET_ALLOCATION_US_THREAD_NUMBER  IMAGE_6_IMAGE_6_OVL_BUDGET_ALLOCATION_US_THREAD_NUMBER

#define RDD_BBH_QUEUE_TABLE_SIZE         RDD_US_TM_BBH_QUEUE_TABLE_SIZE
#define RDD_US_TM_FLOW_CNTR_ETH_OFFSET   RDD_PON_TX_FLOW_TABLE_SIZE
#define RDD_TM_ACTION_PTR_TABLE_SIZE     RDD_US_TM_TM_ACTION_PTR_TABLE_SIZE

#define US_TM_BBH_QUEUE_TABLE_SIZE       RDD_IMAGE_6_US_TM_BBH_QUEUE_TABLE_SIZE
#define RDD_DS_TM_FLOW_CNTR_TABLE_SIZE   RDD_IMAGE_7_DS_TM_TM_FLOW_CNTR_TABLE_SIZE

#define TM_INDEX_DSL_OR_PON              2


/* REPORTING */
#define REPORTING_THREAD_NUMBER     IMAGE_5_IMAGE_5_REPORTING_THREAD_NUMBER
#define REPORTING_COUNTER_ADDRESS   IMAGE_5_REPORTING_COUNTER_TABLE_ADDRESS

/* Service queues AQM timer task interval in us */
#define AQM_TIMER_TASK_TIMER_INTERVAL    100

#define US_TM_BUFFER_CONG_MGT_THREAD_NUMBER         IMAGE_6_IMAGE_6_BUFFER_CONG_MGT_THREAD_NUMBER
#define DS_TM_BUFFER_CONG_MGT_THREAD_NUMBER         IMAGE_7_IMAGE_7_BUFFER_CONG_MGT_THREAD_NUMBER

/* WAN port on US_TM */
#define RDD_IMAGE_WAN_VPORT_TX_FLOW_TABLE_SIZE      RDD_IMAGE_6_VPORT_TX_FLOW_TABLE_SIZE

/* Ingress Filters */
#define INGRESS_FILTER_L2_REASON_TABLE_SIZE RDD_IMAGE_2_INGRESS_FILTER_L2_REASON_TABLE_SIZE

/* CPU RX */
#define CPU_RX_THREAD_NUMBER                        IMAGE_3_IMAGE_3_CPU_RX_THREAD_NUMBER
#define CPU_RX_COPY_THREAD_NUMBER                   IMAGE_3_IMAGE_3_CPU_RX_COPY_THREAD_NUMBER
#define CPU_RX_TIMER_TASK                           IMAGE_3_IMAGE_3_GENERAL_TIMER_THREAD_NUMBER
#define CPU_RX_METER_BUDGET_ALLOCATOR_THREAD_NUMBER IMAGE_3_IMAGE_3_GENERAL_TIMER_THREAD_NUMBER
#define PKTGEN_TX_THREAD_NUMBER                     IMAGE_3_IMAGE_3_CPU_RX_COPY_THREAD_NUMBER

#define SPDSVC_GEN_THREAD_NUMBER                    IMAGE_1_IMAGE_1_SPDSVC_GEN_THREAD_NUMBER
#define TCPSPDTEST_GEN_THREAD_NUMBER                IMAGE_1_IMAGE_1_TCPSPDTEST_GEN_THREAD_NUMBER
#define TCPSPDTEST_THREAD_NUMBER                    IMAGE_1_IMAGE_1_TCPSPDTEST_THREAD_NUMBER
#define GEN_TCPSPDTEST_THREAD_NUMBER                TCPSPDTEST_GEN_THREAD_NUMBER

/* DHD */
#define DHD_TX_POST_0_THREAD_NUMBER                 IMAGE_4_IMAGE_4_DHD_TX_POST_0_THREAD_NUMBER

/* CPU TX */
#define RECYCLE_INTERRUPT_COALESCING_THREAD_NUMBER IMAGE_2_IMAGE_2_INTERRUPT_COALESCING_THREAD_NUMBER
#define CPU_TX_0_THREAD_NUMBER IMAGE_2_IMAGE_2_CPU_TX_0_THREAD_NUMBER
#define IMAGE_CPU_TX_SYNC_FIFO_TABLE_ADDRESS       IMAGE_2_CPU_TX_SYNC_FIFO_TABLE_ADDRESS

#define CPU_TX_MCORE_THREAD_NUMBER                  IMAGE_5_IMAGE_5_CPU_TX_MCORE_THREAD_NUMBER
#define CPU_TX_MCORE_TIMER_TASK                     IMAGE_5_IMAGE_5_GENERAL_TIMER_THREAD_NUMBER
#define CPU_TX_TIMER_TASK                           IMAGE_2_IMAGE_2_GENERAL_TIMER_THREAD_NUMBER

#define CPU_TX_SYNC_FIFO_ENTRY_BYTE_SIZE 8 /* should be same as RDD_CPU_TX_SYNC_FIFO_ENTRY_BYTE_SIZE */

#define RDD_IMAGE_CPU_TX_QM_QUEUE_TO_TX_FLOW_TABLE_PTR_TABLE_SIZE  RDD_IMAGE_2_QM_QUEUE_TO_TX_FLOW_TABLE_PTR_TABLE_SIZE
#define IMAGE_CPU_TX_VPORT_TX_FLOW_TABLE_ADDRESS     IMAGE_2_VPORT_TX_FLOW_TABLE_ADDRESS
#define IMAGE_CPU_TX_DSL_TX_FLOW_TABLE_ADDRESS       IMAGE_2_DSL_TX_FLOW_TABLE_ADDRESS
#define IMAGE_CPU_TX_PON_TX_FLOW_TABLE_ADDRESS       IMAGE_2_PON_TX_FLOW_TABLE_ADDRESS

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

/* flush */
#define RDD_DS_TM_FLUSH_CFG_FW_TABLE_ADDRESS                IMAGE_7_DS_TM_FLUSH_CFG_FW_TABLE_ADDRESS
#define RDD_DS_TM_FLUSH_CFG_ENABLE_TABLE_ADDRESS            IMAGE_7_DS_TM_FLUSH_CFG_ENABLE_TABLE_ADDRESS
#define RDD_US_TM_FLUSH_CFG_FW_TABLE_ADDRESS                IMAGE_6_US_TM_FLUSH_CFG_FW_TABLE_ADDRESS
#define RDD_US_TM_FLUSH_CFG_ENABLE_TABLE_ADDRESS            IMAGE_6_US_TM_FLUSH_CFG_ENABLE_TABLE_ADDRESS
#define RDD_SERVICE_QUEUES_FLUSH_CFG_FW_TABLE_ADDRESS       IMAGE_0_SERVICE_QUEUES_FLUSH_CFG_FW_TABLE_ADDRESS
#define RDD_SERVICE_QUEUES_FLUSH_CFG_ENABLE_TABLE_ADDRESS   IMAGE_0_SERVICE_QUEUES_FLUSH_CFG_ENABLE_TABLE_ADDRESS

#define SPU_RESPONSE_THREAD_NUMBER IMAGE_4_IMAGE_4_SPU_RESPONSE_THREAD_NUMBER
#define BB_ID_SPU                  (BB_ID_LAST+4)   /* BB_ID for CPU3 */

#endif

