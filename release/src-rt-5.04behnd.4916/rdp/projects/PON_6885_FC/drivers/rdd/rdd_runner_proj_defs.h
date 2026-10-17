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

/* TIMER DEFINES */
#define GHOST_REPORTING_TIMER_INTERVAL_IN_USEC                  40
#define TIMER_COMMON_PERIOD_IN_USEC                             100
#define CPU_RX_METER_TIMER_PERIOD                               10000
#define CPU_RX_METER_TIMER_PERIOD_IN_USEC                       (CPU_RX_METER_TIMER_PERIOD)
#define GHOST_REPORTING_TIMER_INTERVAL                          (GHOST_REPORTING_TIMER_INTERVAL_IN_USEC)
#define TIMER_COMMON_PERIOD                                     (TIMER_COMMON_PERIOD_IN_USEC)

/*first 2 used in egress_tm rate limit, will be removed when object will be fully implemented */
#define DS_RATE_LIMITER_TIMER_PERIOD_IN_USEC                    100
#define US_RATE_LIMITER_TIMER_PERIOD_IN_USEC                    100

/* NATC */
/* bit 0 of KEY_MASK register[0] corresponds to key bit 0,
 * bit 31 of KEY_MASK register [7] corresponds to key bit 255 */
#define NATC_16BYTE_KEY_MASK            {.data = {NATC_VAR_CONTEXT_MARK, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff} }

#define ag_drv_natc_cfg_cli_init        ag_drv_natc_tbl_cli_init
#define ag_drv_natc_cfg_key_addr_set    drv_natc_tbl_key_addr_set 
#define ag_drv_natc_cfg_res_addr_set    drv_natc_tbl_res_addr_set

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
#define RDD_LAN11_VPORT      RDD_VPORT_ID_11
#define RDD_LAN12_VPORT      RDD_VPORT_ID_12
#define RDD_LAN13_VPORT      RDD_VPORT_ID_13
#define RDD_LAN14_VPORT      RDD_VPORT_ID_14
#define RDD_LAN15_VPORT      RDD_VPORT_ID_15
#define RDD_LAN_VPORT_LAST   RDD_LAN15_VPORT

#define RDD_WAN0_VPORT       RDD_VPORT_ID_16
#define RDD_PON_WAN_VPORT    RDD_WAN0_VPORT

#define RDD_CPU0_VPORT       (RDD_WAN0_VPORT + 1)
#define RDD_CPU_VPORT_FIRST  RDD_CPU0_VPORT
#define RDD_CPU1_VPORT       (RDD_CPU_VPORT_FIRST + 1)
#define RDD_CPU2_VPORT       (RDD_CPU_VPORT_FIRST + 2)
#define RDD_GDX_VPORT        RDD_CPU2_VPORT
#define RDD_CPU3_VPORT       (RDD_CPU_VPORT_FIRST + 3)
#define RDD_CPU4_VPORT       (RDD_CPU_VPORT_FIRST + 4)
#define RDD_WLAN0_VPORT      RDD_CPU4_VPORT
#define RDD_CPU5_VPORT       (RDD_CPU_VPORT_FIRST + 5)
#define RDD_WLAN1_VPORT      RDD_CPU5_VPORT
#define RDD_CPU6_VPORT       (RDD_CPU_VPORT_FIRST + 6)
#define RDD_WLAN2_VPORT      RDD_CPU6_VPORT
#define RDD_CPU7_VPORT       (RDD_CPU_VPORT_FIRST + 7)
#define RDD_WLAN3_VPORT      RDD_CPU7_VPORT

#define RDD_CPU_VPORT_LAST   RDD_WLAN3_VPORT

#define RDD_BOND0_VPORT      (RDD_CPU_VPORT_LAST + 1)
#define RDD_BOND1_VPORT      (RDD_CPU_VPORT_LAST + 2)

#define RDD_VPORT_MAX RDD_BOND1_VPORT


#define RDD_CPU_VPORT_MASK ((1 << RDD_CPU0_VPORT) | (1 << RDD_CPU1_VPORT) | (1 << RDD_CPU2_VPORT) | \
    (1 << RDD_CPU3_VPORT) | (1 << RDD_CPU4_VPORT) | (1 << RDD_CPU5_VPORT) | (1 << RDD_CPU6_VPORT))

#define RDD_VPORT_ID(id) (1 << id)
#define RDD_WLAN_VPORT_MASK ((1 << RDD_WLAN0_VPORT) | (1 << RDD_WLAN1_VPORT) | (1 << RDD_WLAN2_VPORT))

/* TM */
#define RDD_TM_FLOW_CNTR_TABLE_SIZE      RDD_IMAGE_8_US_TM_TM_FLOW_CNTR_TABLE_SIZE
#define RDD_TM_ACTION_PTR_TABLE_SIZE     RDD_US_TM_TM_ACTION_PTR_TABLE_SIZE
#define RDD_SCHEDULER_TABLE_SIZE         RDD_PON_TM_SCHEDULER_TABLE_SIZE
#define US_TM_BBH_QUEUE_TABLE_SIZE       RDD_IMAGE_8_US_TM_BBH_QUEUE_TABLE_SIZE
#define DS_TM_BBH_QUEUE_TABLE_SIZE       RDD_IMAGE_11_DS_TM_BBH_QUEUE_TABLE_SIZE


#define RDD_SCHEDULER_TABLE_MAX_SIZE            (RDD_PON_TM_SCHEDULER_TABLE_SIZE > RDD_ETH_TM_SCHEDULER_TABLE_SIZE ? \
    RDD_PON_TM_SCHEDULER_TABLE_SIZE : RDD_ETH_TM_SCHEDULER_TABLE_SIZE)
#define RDD_BASIC_SCHEDULER_TABLE_MAX_SIZE      RDD_SCHEDULER_TABLE_MAX_SIZE
#define RDD_SECONDARY_SCHEDULER_TABLE_MAX_SIZE  (RDD_PON_TM_SECONDARY_SCHEDULER_TABLE_SIZE > RDD_ETH_TM_SECONDARY_SCHEDULER_TABLE_SIZE ? \
                                                RDD_PON_TM_SECONDARY_SCHEDULER_TABLE_SIZE : RDD_ETH_TM_SECONDARY_SCHEDULER_TABLE_SIZE)
#define RDD_COMPLEX_SCHEDULER_TABLE_MAX_SIZE    0

#define RDD_DS_TM_FLOW_CNTR_TABLE_SIZE  RDD_IMAGE_7_DS_TM_TM_FLOW_CNTR_TABLE_SIZE
#define TM_BUDGET_ALLOCATION_US_THREAD_NUMBER IMAGE_12_US_TM_BUDGET_ALLOCATION_THREAD_NUMBER
#define TM_OVL_BUDGET_ALLOCATION_US_THREAD_NUMBER IMAGE_12_US_TM_OVL_BUDGET_ALLOCATION_THREAD_NUMBER

#define DS_TM_0_BUDGET_ALLOCATION_THREAD_NUMBER IMAGE_9_DS_TM_GENERAL_TIMER_THREAD_NUMBER
#define DS_TM_1_BUDGET_ALLOCATION_THREAD_NUMBER IMAGE_10_DS_TM_1_GENERAL_TIMER_THREAD_NUMBER
#define DS_TM_2_BUDGET_ALLOCATION_THREAD_NUMBER IMAGE_11_DS_TM_2_GENERAL_TIMER_THREAD_NUMBER

#define US_TM_UPDATE_FIFO_THREAD_NUMBER  IMAGE_12_US_TM_UPDATE_FIFO_THREAD_NUMBER
#define SQ_TM_THREAD_NUMBER                IMAGE_10_IMAGE_10_SERVICE_QUEUES_THREAD_NUMBER
#define DS_TM_0_UPDATE_FIFO_THREAD_NUMBER  IMAGE_9_DS_TM_UPDATE_FIFO_THREAD_NUMBER
#define DS_TM_1_UPDATE_FIFO_THREAD_NUMBER  IMAGE_10_DS_TM_1_UPDATE_FIFO_THREAD_NUMBER
#define DS_TM_2_UPDATE_FIFO_THREAD_NUMBER  IMAGE_11_DS_TM_2_UPDATE_FIFO_THREAD_NUMBER

#define DS_TM_0_TX_TASK_THREAD_NUMBER  IMAGE_9_DS_TM_TX_TASK_0_THREAD_NUMBER
#define DS_TM_1_TX_TASK_THREAD_NUMBER  IMAGE_10_DS_TM_1_TX_TASK_0_THREAD_NUMBER
#define DS_TM_2_TX_TASK_THREAD_NUMBER  IMAGE_11_DS_TM_2_TX_TASK_0_THREAD_NUMBER
#define PON_TM_TX_TASK_THREAD_NUMBER   IMAGE_12_US_TM_WAN_THREAD_NUMBER

#define US_TM_BUFFER_CONG_MGT_THREAD_NUMBER         IMAGE_12_US_TM_BUFFER_CONG_MGT_THREAD_NUMBER
#define DS_TM_BUFFER_CONG_MGT_THREAD_NUMBER         IMAGE_11_DS_TM_2_BUFFER_CONG_MGT_THREAD_NUMBER

#define RDD_DS_TM_0_FLUSH_CFG_FW_TABLE_ADDRESS      IMAGE_9_DS_TM_FLUSH_CFG_FW_TABLE_ADDRESS
#define RDD_DS_TM_1_FLUSH_CFG_FW_TABLE_ADDRESS      IMAGE_10_DS_TM_FLUSH_CFG_FW_TABLE_ADDRESS
#define RDD_DS_TM_2_FLUSH_CFG_FW_TABLE_ADDRESS      IMAGE_11_DS_TM_FLUSH_CFG_FW_TABLE_ADDRESS
#define RDD_DS_TM_0_FLUSH_CFG_ENABLE_TABLE_ADDRESS  IMAGE_9_DS_TM_FLUSH_CFG_ENABLE_TABLE_ADDRESS
#define RDD_DS_TM_1_FLUSH_CFG_ENABLE_TABLE_ADDRESS  IMAGE_10_DS_TM_FLUSH_CFG_ENABLE_TABLE_ADDRESS
#define RDD_DS_TM_2_FLUSH_CFG_ENABLE_TABLE_ADDRESS  IMAGE_11_DS_TM_FLUSH_CFG_ENABLE_TABLE_ADDRESS


/* flush */
#define RDD_US_TM_FLUSH_CFG_FW_TABLE_ADDRESS IMAGE_12_US_TM_FLUSH_CFG_FW_TABLE_ADDRESS
#define RDD_US_TM_FLUSH_CFG_ENABLE_TABLE_ADDRESS IMAGE_12_US_TM_FLUSH_CFG_ENABLE_TABLE_ADDRESS
#define RDD_SERVICE_QUEUES_FLUSH_CFG_FW_TABLE_ADDRESS IMAGE_4_SERVICE_QUEUES_FLUSH_CFG_FW_TABLE_ADDRESS
#define RDD_SERVICE_QUEUES_FLUSH_CFG_ENABLE_TABLE_ADDRESS IMAGE_4_SERVICE_QUEUES_FLUSH_CFG_ENABLE_TABLE_ADDRESS

#define RDD_BBH_QUEUE_TABLE_SIZE  40
#define RDD_BASIC_RATE_LIMITER_TABLE_SIZE RDD_BASIC_RATE_LIMITER_TABLE_US_SIZE

#define TIMER_COMMON_THREAD_NUMBER          IMAGE_2_IMAGE_2_TIMER_COMMON_THREAD_NUMBER


/* Ingress Filters */
#define INGRESS_FILTER_L2_REASON_TABLE_SIZE                     RDD_IMAGE_2_INGRESS_FILTER_L2_REASON_TABLE_SIZE

/* CPU RX */

#define CPU_RX_THREAD_NUMBER                                    IMAGE_2_IMAGE_2_CPU_RX_THREAD_NUMBER
#define CPU_RX_CSO_THREAD_NUMBER                                IMAGE_2_IMAGE_2_CPU_RX_CSO_THREAD_NUMBER
#define CPU_RX_TIMER_TASK                                       IMAGE_4_IMAGE_4_GENERAL_TIMER_THREAD_NUMBER
#define CPU_RX_METER_BUDGET_ALLOCATOR_THREAD_NUMBER             IMAGE_4_IMAGE_4_GENERAL_TIMER_THREAD_NUMBER

/* DHD */
#define DHD_TX_POST_0_THREAD_NUMBER                             IMAGE_9_TM_REPROC_DHD_TX_POST_0_THREAD_NUMBER

/* CPU TX */
#define RECYCLE_INTERRUPT_COALESCING_THREAD_NUMBER              IMAGE_4_IMAGE_4_INTERRUPT_COALESCING_THREAD_NUMBER
#define CPU_TX_0_THREAD_NUMBER                                  IMAGE_4_IMAGE_4_CPU_TX_0_THREAD_NUMBER
#define CPU_TX_TIMER_TASK                                       IMAGE_4_IMAGE_4_GENERAL_TIMER_THREAD_NUMBER
#define IMAGE_CPU_TX_SYNC_FIFO_TABLE_ADDRESS                    IMAGE_4_CPU_TX_SYNC_FIFO_TABLE_ADDRESS

#define CPU_TX_SYNC_FIFO_ENTRY_BYTE_SIZE 8 /* should be same as RDD_CPU_TX_SYNC_FIFO_ENTRY_BYTE_SIZE */

/* Speed Test */
#define SPDSVC_GEN_THREAD_NUMBER                                IMAGE_3_IMAGE_3_SPDSVC_GEN_THREAD_NUMBER

#ifdef TCP_SPDTEST_C_CODE
#define TCPSPDTEST_UPLOAD_TIMER_THREAD_NUMBER                   IMAGE_3_IMAGE_3_TCPSPDTEST_UPLOAD_TIMER_THREAD_NUMBER
#define TCPSPDTEST_DOWNLOAD_THREAD_NUMBER                       IMAGE_3_IMAGE_3_TCPSPDTEST_DOWNLOAD_THREAD_NUMBER
#define TCPSPDTEST_UPLOAD_THREAD_NUMBER                         IMAGE_3_IMAGE_3_TCPSPDTEST_UPLOAD_THREAD_NUMBER
#define GEN_TCPSPDTEST_THREAD_NUMBER                            TCPSPDTEST_UPLOAD_TIMER_THREAD_NUMBER
#else
#define TCPSPDTEST_GEN_THREAD_NUMBER                            0
#define TCPSPDTEST_THREAD_NUMBER                                0
#define GEN_TCPSPDTEST_THREAD_NUMBER                            TCPSPDTEST_GEN_THREAD_NUMBER
#endif

#define CPU_FPM_BUFMNG_RELEASE_TASK                             0
#define CPU_RINGS_FPM_REFILL_CORE_NUMBER                        0

/* Ingress classifier and VLAN actions */
#define RDD_US_IC_RULE_CFG_TABLE_SIZE                           128
#define RDD_DS_IC_RULE_CFG_TABLE_SIZE                           128
#define RDD_DS_DEFAULT_FLOW_CONTEXT_TABLE_SIZE                  128
#define RDD_US_DEFAULT_FLOW_CONTEXT_TABLE_SIZE                  8
#define RDD_DEFAULT_FLOW_CONTEXT_TABLE_SIZE                     (RDD_DS_DEFAULT_FLOW_CONTEXT_TABLE_SIZE + RDD_US_DEFAULT_FLOW_CONTEXT_TABLE_SIZE)
#define RDD_IC_SHARED_CONTEXT_TABLE_SIZE                        2048
#define RDD_DS_IC_CONTEXT_TABLE_SIZE                            RDD_IC_SHARED_CONTEXT_TABLE_SIZE
#define RDD_US_IC_CONTEXT_TABLE_SIZE                            RDD_IC_SHARED_CONTEXT_TABLE_SIZE
#define NUM_OF_GENERIC_RULE_CFG                                 4
#define RDD_VLAN_COMMAND_SKIP                                   128
#define QM_QUEUE_DROP                                           0xFF

/* SPU offload */
#define SPU_RESPONSE_THREAD_NUMBER IMAGE_8_IMAGE_8_SPU_RESPONSE_THREAD_NUMBER
#define RDD_SPU_VPORT              RDD_CPU3_VPORT /* VPORT for CPU3 */
#define BB_ID_SPU                  (BB_ID_LAST+4) /* BB_ID for CPU3 */

#endif
