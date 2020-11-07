/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

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

#include "ru.h"

/******************************************************************************
 * Register: DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG
 ******************************************************************************/
const ru_reg_rec DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_REG = 
{
    "REORDER_CFG_DSPTCHR_REORDR_CFG",
#if RU_INCLUDE_DESC
    "DISPATCHER_REORDER_EN Register",
    "Enable of dispatcher reorder",
#endif
    DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_REG_OFFSET,
    0,
    0,
    355,
};

/******************************************************************************
 * Register: DSPTCHR_REORDER_CFG_VQ_EN
 ******************************************************************************/
const ru_reg_rec DSPTCHR_REORDER_CFG_VQ_EN_REG = 
{
    "REORDER_CFG_VQ_EN",
#if RU_INCLUDE_DESC
    "VIRTUAL_Q_EN Register",
    "Enable control for each VIQ/VEQ",
#endif
    DSPTCHR_REORDER_CFG_VQ_EN_REG_OFFSET,
    0,
    0,
    356,
};

/******************************************************************************
 * Register: DSPTCHR_REORDER_CFG_BB_CFG
 ******************************************************************************/
const ru_reg_rec DSPTCHR_REORDER_CFG_BB_CFG_REG = 
{
    "REORDER_CFG_BB_CFG",
#if RU_INCLUDE_DESC
    "BROADBUS_CONFIG Register",
    "Allow override of a specific BB destination with a new Route ADDR",
#endif
    DSPTCHR_REORDER_CFG_BB_CFG_REG_OFFSET,
    0,
    0,
    357,
};

/******************************************************************************
 * Register: DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL
 ******************************************************************************/
const ru_reg_rec DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_REG = 
{
    "REORDER_CFG_CLK_GATE_CNTRL",
#if RU_INCLUDE_DESC
    "CLOCK_GATE_CONTROL Register",
    "Clock Gate control register including timer config and bypass control",
#endif
    DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_REG_OFFSET,
    0,
    0,
    358,
};

/******************************************************************************
 * Register: DSPTCHR_CONGESTION_INGRS_CONGSTN
 ******************************************************************************/
const ru_reg_rec DSPTCHR_CONGESTION_INGRS_CONGSTN_REG = 
{
    "CONGESTION_INGRS_CONGSTN",
#if RU_INCLUDE_DESC
    "INGRESS_CONGESTION_THRESHOLD %i Register",
    "Ingress Queues congestion state."
    "",
#endif
    DSPTCHR_CONGESTION_INGRS_CONGSTN_REG_OFFSET,
    DSPTCHR_CONGESTION_INGRS_CONGSTN_REG_RAM_CNT,
    4,
    359,
};

/******************************************************************************
 * Register: DSPTCHR_CONGESTION_EGRS_CONGSTN
 ******************************************************************************/
const ru_reg_rec DSPTCHR_CONGESTION_EGRS_CONGSTN_REG = 
{
    "CONGESTION_EGRS_CONGSTN",
#if RU_INCLUDE_DESC
    "EGRESS_CONGESTION_THRESHOLD %i Register",
    "Egress Queues congestion state per Q."
    "",
#endif
    DSPTCHR_CONGESTION_EGRS_CONGSTN_REG_OFFSET,
    DSPTCHR_CONGESTION_EGRS_CONGSTN_REG_RAM_CNT,
    4,
    360,
};

/******************************************************************************
 * Register: DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN
 ******************************************************************************/
const ru_reg_rec DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_REG = 
{
    "CONGESTION_TOTAL_EGRS_CONGSTN",
#if RU_INCLUDE_DESC
    "TOTAL_EGRESS_CONGESTION_THRESHOLD Register",
    "Egress congestion states (Total Count)",
#endif
    DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_REG_OFFSET,
    0,
    0,
    361,
};

/******************************************************************************
 * Register: DSPTCHR_CONGESTION_GLBL_CONGSTN
 ******************************************************************************/
const ru_reg_rec DSPTCHR_CONGESTION_GLBL_CONGSTN_REG = 
{
    "CONGESTION_GLBL_CONGSTN",
#if RU_INCLUDE_DESC
    "GLOBAL_CONGESTION_THRESHOLD Register",
    "Congestion levels of FLL state. Once no mode BDs are availabe congestion indication will be risen on all PDs.",
#endif
    DSPTCHR_CONGESTION_GLBL_CONGSTN_REG_OFFSET,
    0,
    0,
    362,
};

/******************************************************************************
 * Register: DSPTCHR_CONGESTION_CONGSTN_STATUS
 ******************************************************************************/
const ru_reg_rec DSPTCHR_CONGESTION_CONGSTN_STATUS_REG = 
{
    "CONGESTION_CONGSTN_STATUS",
#if RU_INCLUDE_DESC
    "CONGESTION_STATUS Register",
    "This register reflects the current congestion levels in the dispatcher.",
#endif
    DSPTCHR_CONGESTION_CONGSTN_STATUS_REG_OFFSET,
    0,
    0,
    363,
};

/******************************************************************************
 * Register: DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_LOW
 ******************************************************************************/
const ru_reg_rec DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_LOW_REG = 
{
    "CONGESTION_PER_Q_INGRS_CONGSTN_LOW",
#if RU_INCLUDE_DESC
    "PER_Q_LOW_INGRESS_CONGESTION_STATUS Register",
    "Note that this vector is only updated during the dispatch stage",
#endif
    DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_LOW_REG_OFFSET,
    0,
    0,
    364,
};

/******************************************************************************
 * Register: DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_HIGH
 ******************************************************************************/
const ru_reg_rec DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_HIGH_REG = 
{
    "CONGESTION_PER_Q_INGRS_CONGSTN_HIGH",
#if RU_INCLUDE_DESC
    "PER_Q_HIGH_INGRESS_CONGESTION_STATUS Register",
    "Note that this vector is only updated during the dispatch stage",
#endif
    DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_HIGH_REG_OFFSET,
    0,
    0,
    365,
};

/******************************************************************************
 * Register: DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_LOW
 ******************************************************************************/
const ru_reg_rec DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_LOW_REG = 
{
    "CONGESTION_PER_Q_EGRS_CONGSTN_LOW",
#if RU_INCLUDE_DESC
    "PER_Q_LOW_EGRESS_CONGESTION_STATUS Register",
    "Note that this vector is only updated during the dispatch stage",
#endif
    DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_LOW_REG_OFFSET,
    0,
    0,
    366,
};

/******************************************************************************
 * Register: DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_HIGH
 ******************************************************************************/
const ru_reg_rec DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_HIGH_REG = 
{
    "CONGESTION_PER_Q_EGRS_CONGSTN_HIGH",
#if RU_INCLUDE_DESC
    "PER_Q_HIGH_EGRESS_CONGESTION_STATUS Register",
    "Note that this vector is only updated during the dispatch stage",
#endif
    DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_HIGH_REG_OFFSET,
    0,
    0,
    367,
};

/******************************************************************************
 * Register: DSPTCHR_INGRS_QUEUES_Q_INGRS_SIZE
 ******************************************************************************/
const ru_reg_rec DSPTCHR_INGRS_QUEUES_Q_INGRS_SIZE_REG = 
{
    "INGRS_QUEUES_Q_INGRS_SIZE",
#if RU_INCLUDE_DESC
    "QUEUE_INGRS_SIZE %i Register",
    "Q Ingress size",
#endif
    DSPTCHR_INGRS_QUEUES_Q_INGRS_SIZE_REG_OFFSET,
    DSPTCHR_INGRS_QUEUES_Q_INGRS_SIZE_REG_RAM_CNT,
    4,
    368,
};

/******************************************************************************
 * Register: DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS
 ******************************************************************************/
const ru_reg_rec DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_REG = 
{
    "INGRS_QUEUES_Q_INGRS_LIMITS",
#if RU_INCLUDE_DESC
    "QUEUE_INGRS_LIMITS %i Register",
    "Q Ingress Limits",
#endif
    DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_REG_OFFSET,
    DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_REG_RAM_CNT,
    4,
    369,
};

/******************************************************************************
 * Register: DSPTCHR_INGRS_QUEUES_Q_INGRS_COHRENCY
 ******************************************************************************/
const ru_reg_rec DSPTCHR_INGRS_QUEUES_Q_INGRS_COHRENCY_REG = 
{
    "INGRS_QUEUES_Q_INGRS_COHRENCY",
#if RU_INCLUDE_DESC
    "QUEUE_INGRS_COHERENCY %i Register",
    "Q Coherency counter",
#endif
    DSPTCHR_INGRS_QUEUES_Q_INGRS_COHRENCY_REG_OFFSET,
    DSPTCHR_INGRS_QUEUES_Q_INGRS_COHRENCY_REG_RAM_CNT,
    4,
    370,
};

/******************************************************************************
 * Register: DSPTCHR_QUEUE_MAPPING_CRDT_CFG
 ******************************************************************************/
const ru_reg_rec DSPTCHR_QUEUE_MAPPING_CRDT_CFG_REG = 
{
    "QUEUE_MAPPING_CRDT_CFG",
#if RU_INCLUDE_DESC
    "CREDIT_CONFIGURATION %i Register",
    "Configuration for each Q including BB_ID, Target address, valid",
#endif
    DSPTCHR_QUEUE_MAPPING_CRDT_CFG_REG_OFFSET,
    DSPTCHR_QUEUE_MAPPING_CRDT_CFG_REG_RAM_CNT,
    4,
    371,
};

/******************************************************************************
 * Register: DSPTCHR_QUEUE_MAPPING_PD_DSPTCH_ADD
 ******************************************************************************/
const ru_reg_rec DSPTCHR_QUEUE_MAPPING_PD_DSPTCH_ADD_REG = 
{
    "QUEUE_MAPPING_PD_DSPTCH_ADD",
#if RU_INCLUDE_DESC
    "DISPATCH_ADDRESS %i Register",
    "Dispatched address will be calculated"
    "ADD= BASE_ADD + (TASK_NUM x OFFSET)",
#endif
    DSPTCHR_QUEUE_MAPPING_PD_DSPTCH_ADD_REG_OFFSET,
    DSPTCHR_QUEUE_MAPPING_PD_DSPTCH_ADD_REG_RAM_CNT,
    4,
    372,
};

/******************************************************************************
 * Register: DSPTCHR_QUEUE_MAPPING_Q_DEST
 ******************************************************************************/
const ru_reg_rec DSPTCHR_QUEUE_MAPPING_Q_DEST_REG = 
{
    "QUEUE_MAPPING_Q_DEST",
#if RU_INCLUDE_DESC
    "Q_DESTINATION Register",
    "What is the destination of each VIQ. to Dispatcher and from there to Processing RNR or Reorder and from there to the QM",
#endif
    DSPTCHR_QUEUE_MAPPING_Q_DEST_REG_OFFSET,
    0,
    0,
    373,
};

/******************************************************************************
 * Register: DSPTCHR_POOL_SIZES_CMN_POOL_LMT
 ******************************************************************************/
const ru_reg_rec DSPTCHR_POOL_SIZES_CMN_POOL_LMT_REG = 
{
    "POOL_SIZES_CMN_POOL_LMT",
#if RU_INCLUDE_DESC
    "COMMON_POOL_LIMIT Register",
    "common pool max size",
#endif
    DSPTCHR_POOL_SIZES_CMN_POOL_LMT_REG_OFFSET,
    0,
    0,
    374,
};

/******************************************************************************
 * Register: DSPTCHR_POOL_SIZES_CMN_POOL_SIZE
 ******************************************************************************/
const ru_reg_rec DSPTCHR_POOL_SIZES_CMN_POOL_SIZE_REG = 
{
    "POOL_SIZES_CMN_POOL_SIZE",
#if RU_INCLUDE_DESC
    "COMMON_POOL_SIZE Register",
    "common pool size",
#endif
    DSPTCHR_POOL_SIZES_CMN_POOL_SIZE_REG_OFFSET,
    0,
    0,
    375,
};

/******************************************************************************
 * Register: DSPTCHR_POOL_SIZES_GRNTED_POOL_LMT
 ******************************************************************************/
const ru_reg_rec DSPTCHR_POOL_SIZES_GRNTED_POOL_LMT_REG = 
{
    "POOL_SIZES_GRNTED_POOL_LMT",
#if RU_INCLUDE_DESC
    "GUARANTEED_POOL_LIMIT Register",
    "Guaranteed pool max size",
#endif
    DSPTCHR_POOL_SIZES_GRNTED_POOL_LMT_REG_OFFSET,
    0,
    0,
    376,
};

/******************************************************************************
 * Register: DSPTCHR_POOL_SIZES_GRNTED_POOL_SIZE
 ******************************************************************************/
const ru_reg_rec DSPTCHR_POOL_SIZES_GRNTED_POOL_SIZE_REG = 
{
    "POOL_SIZES_GRNTED_POOL_SIZE",
#if RU_INCLUDE_DESC
    "GUARANTEED_POOL_SIZE Register",
    "Guaranteed pool size",
#endif
    DSPTCHR_POOL_SIZES_GRNTED_POOL_SIZE_REG_OFFSET,
    0,
    0,
    377,
};

/******************************************************************************
 * Register: DSPTCHR_POOL_SIZES_MULTI_CST_POOL_LMT
 ******************************************************************************/
const ru_reg_rec DSPTCHR_POOL_SIZES_MULTI_CST_POOL_LMT_REG = 
{
    "POOL_SIZES_MULTI_CST_POOL_LMT",
#if RU_INCLUDE_DESC
    "MULTI_CAST_POOL_LIMIT Register",
    "Multi Cast pool max size",
#endif
    DSPTCHR_POOL_SIZES_MULTI_CST_POOL_LMT_REG_OFFSET,
    0,
    0,
    378,
};

/******************************************************************************
 * Register: DSPTCHR_POOL_SIZES_MULTI_CST_POOL_SIZE
 ******************************************************************************/
const ru_reg_rec DSPTCHR_POOL_SIZES_MULTI_CST_POOL_SIZE_REG = 
{
    "POOL_SIZES_MULTI_CST_POOL_SIZE",
#if RU_INCLUDE_DESC
    "MULTI_CAST_POOL_SIZE Register",
    "Multi Cast pool size",
#endif
    DSPTCHR_POOL_SIZES_MULTI_CST_POOL_SIZE_REG_OFFSET,
    0,
    0,
    379,
};

/******************************************************************************
 * Register: DSPTCHR_POOL_SIZES_RNR_POOL_LMT
 ******************************************************************************/
const ru_reg_rec DSPTCHR_POOL_SIZES_RNR_POOL_LMT_REG = 
{
    "POOL_SIZES_RNR_POOL_LMT",
#if RU_INCLUDE_DESC
    "RNR_POOL_LIMIT Register",
    "This counter counts the amount of buffers taken by runner for MultiCast purposes (or any other the requires adding new PDs to a Virtual Egress Queue - VEQ",
#endif
    DSPTCHR_POOL_SIZES_RNR_POOL_LMT_REG_OFFSET,
    0,
    0,
    380,
};

/******************************************************************************
 * Register: DSPTCHR_POOL_SIZES_RNR_POOL_SIZE
 ******************************************************************************/
const ru_reg_rec DSPTCHR_POOL_SIZES_RNR_POOL_SIZE_REG = 
{
    "POOL_SIZES_RNR_POOL_SIZE",
#if RU_INCLUDE_DESC
    "RNR_POOL_SIZE Register",
    "This counter counts the amount of buffers taken by runner for MultiCast purposes (or any other the requires adding new PDs to a Virtual Egress Qeueu - VEQ)",
#endif
    DSPTCHR_POOL_SIZES_RNR_POOL_SIZE_REG_OFFSET,
    0,
    0,
    381,
};

/******************************************************************************
 * Register: DSPTCHR_POOL_SIZES_PRCSSING_POOL_SIZE
 ******************************************************************************/
const ru_reg_rec DSPTCHR_POOL_SIZES_PRCSSING_POOL_SIZE_REG = 
{
    "POOL_SIZES_PRCSSING_POOL_SIZE",
#if RU_INCLUDE_DESC
    "PROCESSING_POOL_SIZE Register",
    "This counter counts how many buffers are currenly being handled by all RNRs",
#endif
    DSPTCHR_POOL_SIZES_PRCSSING_POOL_SIZE_REG_OFFSET,
    0,
    0,
    382,
};

/******************************************************************************
 * Register: DSPTCHR_MASK_MSK_TSK_255_0
 ******************************************************************************/
const ru_reg_rec DSPTCHR_MASK_MSK_TSK_255_0_REG = 
{
    "MASK_MSK_TSK_255_0",
#if RU_INCLUDE_DESC
    "TASK_MASK %i Register",
    "Address 0 ->  255:224"
    "Address 4 ->  223:192"
    "Address 8 ->  191:160"
    "Address C ->  159:128"
    "Address 10 ->  127:96"
    "Address 14 ->   95:64"
    "Address 18 ->   63:32"
    "Address 1C ->   31: 0"
    ""
    ""
    "8 RG x 8 Regs per RG = 64 registers",
#endif
    DSPTCHR_MASK_MSK_TSK_255_0_REG_OFFSET,
    DSPTCHR_MASK_MSK_TSK_255_0_REG_RAM_CNT,
    4,
    383,
};

/******************************************************************************
 * Register: DSPTCHR_MASK_MSK_Q
 ******************************************************************************/
const ru_reg_rec DSPTCHR_MASK_MSK_Q_REG = 
{
    "MASK_MSK_Q",
#if RU_INCLUDE_DESC
    "QUEUE_MASK %i Register",
    "Queue Mask: Per RNR group holds a vector of which tasks are related to the group",
#endif
    DSPTCHR_MASK_MSK_Q_REG_OFFSET,
    DSPTCHR_MASK_MSK_Q_REG_RAM_CNT,
    4,
    384,
};

/******************************************************************************
 * Register: DSPTCHR_MASK_DLY_Q
 ******************************************************************************/
const ru_reg_rec DSPTCHR_MASK_DLY_Q_REG = 
{
    "MASK_DLY_Q",
#if RU_INCLUDE_DESC
    "DELAY_Q Register",
    "Describes which VEQ are part of the Delay Q group.",
#endif
    DSPTCHR_MASK_DLY_Q_REG_OFFSET,
    0,
    0,
    385,
};

/******************************************************************************
 * Register: DSPTCHR_MASK_NON_DLY_Q
 ******************************************************************************/
const ru_reg_rec DSPTCHR_MASK_NON_DLY_Q_REG = 
{
    "MASK_NON_DLY_Q",
#if RU_INCLUDE_DESC
    "NON_DELAY_Q Register",
    "Describes which VEQ are part of the Non-Delay Q group.",
#endif
    DSPTCHR_MASK_NON_DLY_Q_REG_OFFSET,
    0,
    0,
    386,
};

/******************************************************************************
 * Register: DSPTCHR_EGRS_QUEUES_EGRS_DLY_QM_CRDT
 ******************************************************************************/
const ru_reg_rec DSPTCHR_EGRS_QUEUES_EGRS_DLY_QM_CRDT_REG = 
{
    "EGRS_QUEUES_EGRS_DLY_QM_CRDT",
#if RU_INCLUDE_DESC
    "EGRESS_QM_DELAY_CREDIT Register",
    "These registers hold the available credit for the Re-Order to sent PDs to the QM via Delay Q."
    ""
    "",
#endif
    DSPTCHR_EGRS_QUEUES_EGRS_DLY_QM_CRDT_REG_OFFSET,
    0,
    0,
    387,
};

/******************************************************************************
 * Register: DSPTCHR_EGRS_QUEUES_EGRS_NON_DLY_QM_CRDT
 ******************************************************************************/
const ru_reg_rec DSPTCHR_EGRS_QUEUES_EGRS_NON_DLY_QM_CRDT_REG = 
{
    "EGRS_QUEUES_EGRS_NON_DLY_QM_CRDT",
#if RU_INCLUDE_DESC
    "EGRESS_QM_NON_DELAY_CREDIT Register",
    "These registers hold the available credit for the Re-Order to sent PDs to the QM via Non-Delay Q."
    ""
    "",
#endif
    DSPTCHR_EGRS_QUEUES_EGRS_NON_DLY_QM_CRDT_REG_OFFSET,
    0,
    0,
    388,
};

/******************************************************************************
 * Register: DSPTCHR_EGRS_QUEUES_TOTAL_Q_EGRS_SIZE
 ******************************************************************************/
const ru_reg_rec DSPTCHR_EGRS_QUEUES_TOTAL_Q_EGRS_SIZE_REG = 
{
    "EGRS_QUEUES_TOTAL_Q_EGRS_SIZE",
#if RU_INCLUDE_DESC
    "TOTAL_EGRESS_SIZE Register",
    "Size of all egress queues. affected from PDs sent to dispatch and from multicast connect"
    "",
#endif
    DSPTCHR_EGRS_QUEUES_TOTAL_Q_EGRS_SIZE_REG_OFFSET,
    0,
    0,
    389,
};

/******************************************************************************
 * Register: DSPTCHR_EGRS_QUEUES_PER_Q_EGRS_SIZE
 ******************************************************************************/
const ru_reg_rec DSPTCHR_EGRS_QUEUES_PER_Q_EGRS_SIZE_REG = 
{
    "EGRS_QUEUES_PER_Q_EGRS_SIZE",
#if RU_INCLUDE_DESC
    "Q_EGRESS_SIZE %i Register",
    "Size of all egress queues. affected from PDs sent to dispatch and from multicast connect"
    "",
#endif
    DSPTCHR_EGRS_QUEUES_PER_Q_EGRS_SIZE_REG_OFFSET,
    DSPTCHR_EGRS_QUEUES_PER_Q_EGRS_SIZE_REG_RAM_CNT,
    4,
    390,
};

/******************************************************************************
 * Register: DSPTCHR_WAKEUP_CONTROL_WKUP_REQ
 ******************************************************************************/
const ru_reg_rec DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_REG = 
{
    "WAKEUP_CONTROL_WKUP_REQ",
#if RU_INCLUDE_DESC
    "WAKEUP_REQUEST Register",
    "Bit per queue, wakeup request from RNR to a specific Q. Once a wakeup request message is sent to dsptchr it will be latched until the amount of credits pass a threshold",
#endif
    DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_REG_OFFSET,
    0,
    0,
    391,
};

/******************************************************************************
 * Register: DSPTCHR_WAKEUP_CONTROL_WKUP_THRSHLD
 ******************************************************************************/
const ru_reg_rec DSPTCHR_WAKEUP_CONTROL_WKUP_THRSHLD_REG = 
{
    "WAKEUP_CONTROL_WKUP_THRSHLD",
#if RU_INCLUDE_DESC
    "WAKEUP_THRESHOLD Register",
    "Wakeup Thresholds in which to indicate RNR",
#endif
    DSPTCHR_WAKEUP_CONTROL_WKUP_THRSHLD_REG_OFFSET,
    0,
    0,
    392,
};

/******************************************************************************
 * Register: DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO
 ******************************************************************************/
const ru_reg_rec DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_REG = 
{
    "DISPTCH_SCHEDULING_DWRR_INFO",
#if RU_INCLUDE_DESC
    "SCHEDULING_Q_INFO %i Register",
    "DWRR info per Q. including amount of credits per Q. If Q has below zero credits and Quantum size",
#endif
    DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_REG_OFFSET,
    DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_REG_RAM_CNT,
    4,
    393,
};

/******************************************************************************
 * Register: DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT
 ******************************************************************************/
const ru_reg_rec DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_REG = 
{
    "DISPTCH_SCHEDULING_VLD_CRDT",
#if RU_INCLUDE_DESC
    "VALID_QUEUES Register",
    "Queues with credits above zero. This will allow for the Q to participate in the scheduling round",
#endif
    DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_REG_OFFSET,
    0,
    0,
    394,
};

/******************************************************************************
 * Register: DSPTCHR_LOAD_BALANCING_LB_CFG
 ******************************************************************************/
const ru_reg_rec DSPTCHR_LOAD_BALANCING_LB_CFG_REG = 
{
    "LOAD_BALANCING_LB_CFG",
#if RU_INCLUDE_DESC
    "LB_CONFIG Register",
    "Selects which Load Balancing mechanism to use",
#endif
    DSPTCHR_LOAD_BALANCING_LB_CFG_REG_OFFSET,
    0,
    0,
    395,
};

/******************************************************************************
 * Register: DSPTCHR_LOAD_BALANCING_FREE_TASK_0_1
 ******************************************************************************/
const ru_reg_rec DSPTCHR_LOAD_BALANCING_FREE_TASK_0_1_REG = 
{
    "LOAD_BALANCING_FREE_TASK_0_1",
#if RU_INCLUDE_DESC
    "FREE_TASKS_RNR_0_1 Register",
    "Each bit indicates if the Task is Free for dispatch:"
    ""
    "Tasks  0..15 belong to RNR 0"
    "Tasks 16..32 Belong to RNR 1",
#endif
    DSPTCHR_LOAD_BALANCING_FREE_TASK_0_1_REG_OFFSET,
    0,
    0,
    396,
};

/******************************************************************************
 * Register: DSPTCHR_LOAD_BALANCING_FREE_TASK_2_3
 ******************************************************************************/
const ru_reg_rec DSPTCHR_LOAD_BALANCING_FREE_TASK_2_3_REG = 
{
    "LOAD_BALANCING_FREE_TASK_2_3",
#if RU_INCLUDE_DESC
    "FREE_TASKS_RNR_2_3 Register",
    "Each bit indicates if the Task is Free for dispatch:"
    ""
    "Tasks  0..15 belong to RNR 2"
    "Tasks 16..32 Belong to RNR 3",
#endif
    DSPTCHR_LOAD_BALANCING_FREE_TASK_2_3_REG_OFFSET,
    0,
    0,
    397,
};

/******************************************************************************
 * Register: DSPTCHR_LOAD_BALANCING_FREE_TASK_4_5
 ******************************************************************************/
const ru_reg_rec DSPTCHR_LOAD_BALANCING_FREE_TASK_4_5_REG = 
{
    "LOAD_BALANCING_FREE_TASK_4_5",
#if RU_INCLUDE_DESC
    "FREE_TASKS_RNR_4_5 Register",
    "Each bit indicates if the Task is Free for dispatch:"
    ""
    "Tasks  0..15 belong to RNR 4"
    "Tasks 16..32 Belong to RNR 5",
#endif
    DSPTCHR_LOAD_BALANCING_FREE_TASK_4_5_REG_OFFSET,
    0,
    0,
    398,
};

/******************************************************************************
 * Register: DSPTCHR_LOAD_BALANCING_FREE_TASK_6_7
 ******************************************************************************/
const ru_reg_rec DSPTCHR_LOAD_BALANCING_FREE_TASK_6_7_REG = 
{
    "LOAD_BALANCING_FREE_TASK_6_7",
#if RU_INCLUDE_DESC
    "FREE_TASKS_RNR_6_7 Register",
    "Each bit indicates if the Task is Free for dispatch:"
    ""
    "Tasks  0..15 belong to RNR 6"
    "Tasks 16..32 Belong to RNR 7",
#endif
    DSPTCHR_LOAD_BALANCING_FREE_TASK_6_7_REG_OFFSET,
    0,
    0,
    399,
};

/******************************************************************************
 * Register: DSPTCHR_LOAD_BALANCING_FREE_TASK_8_9
 ******************************************************************************/
const ru_reg_rec DSPTCHR_LOAD_BALANCING_FREE_TASK_8_9_REG = 
{
    "LOAD_BALANCING_FREE_TASK_8_9",
#if RU_INCLUDE_DESC
    "FREE_TASKS_RNR_8_9 Register",
    "Each bit indicates if the Task is Free for dispatch:"
    ""
    "Tasks  0..15 belong to RNR 8"
    "Tasks 16..32 Belong to RNR 9",
#endif
    DSPTCHR_LOAD_BALANCING_FREE_TASK_8_9_REG_OFFSET,
    0,
    0,
    400,
};

/******************************************************************************
 * Register: DSPTCHR_LOAD_BALANCING_FREE_TASK_10_11
 ******************************************************************************/
const ru_reg_rec DSPTCHR_LOAD_BALANCING_FREE_TASK_10_11_REG = 
{
    "LOAD_BALANCING_FREE_TASK_10_11",
#if RU_INCLUDE_DESC
    "FREE_TASKS_RNR_10_11 Register",
    "Each bit indicates if the Task is Free for dispatch:"
    ""
    "Tasks  0..15 belong to RNR 10"
    "Tasks 16..32 Belong to RNR 11",
#endif
    DSPTCHR_LOAD_BALANCING_FREE_TASK_10_11_REG_OFFSET,
    0,
    0,
    401,
};

/******************************************************************************
 * Register: DSPTCHR_LOAD_BALANCING_FREE_TASK_12_13
 ******************************************************************************/
const ru_reg_rec DSPTCHR_LOAD_BALANCING_FREE_TASK_12_13_REG = 
{
    "LOAD_BALANCING_FREE_TASK_12_13",
#if RU_INCLUDE_DESC
    "FREE_TASKS_RNR_12_13 Register",
    "Each bit indicates if the Task is Free for dispatch:"
    ""
    "Tasks  0..15 belong to RNR 12"
    "Tasks 16..32 Belong to RNR 13",
#endif
    DSPTCHR_LOAD_BALANCING_FREE_TASK_12_13_REG_OFFSET,
    0,
    0,
    402,
};

/******************************************************************************
 * Register: DSPTCHR_LOAD_BALANCING_FREE_TASK_14_15
 ******************************************************************************/
const ru_reg_rec DSPTCHR_LOAD_BALANCING_FREE_TASK_14_15_REG = 
{
    "LOAD_BALANCING_FREE_TASK_14_15",
#if RU_INCLUDE_DESC
    "FREE_TASKS_RNR_14_15 Register",
    "Each bit indicates if the Task is Free for dispatch:"
    ""
    "Tasks  0..15 belong to RNR 14"
    "Tasks 16..32 Belong to RNR 15",
#endif
    DSPTCHR_LOAD_BALANCING_FREE_TASK_14_15_REG_OFFSET,
    0,
    0,
    403,
};

/******************************************************************************
 * Register: DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING
 ******************************************************************************/
const ru_reg_rec DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_REG = 
{
    "LOAD_BALANCING_TSK_TO_RG_MAPPING",
#if RU_INCLUDE_DESC
    "TASK_TO_RG_MAPPING %i Register",
    "This ram is used to map each task to which group does it belong to.",
#endif
    DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_REG_OFFSET,
    DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_REG_RAM_CNT,
    4,
    404,
};

/******************************************************************************
 * Register: DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_0_3
 ******************************************************************************/
const ru_reg_rec DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_0_3_REG = 
{
    "LOAD_BALANCING_RG_AVLABL_TSK_0_3",
#if RU_INCLUDE_DESC
    "RG_AVAILABLE_TASK_0_3 Register",
    "Available tasks in all runners related to a RNR Group. In case value is zero there are no tasks available for this RNR Group for dispatch hence it should be excluded from the next RNR Group selection",
#endif
    DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_0_3_REG_OFFSET,
    0,
    0,
    405,
};

/******************************************************************************
 * Register: DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_4_7
 ******************************************************************************/
const ru_reg_rec DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_4_7_REG = 
{
    "LOAD_BALANCING_RG_AVLABL_TSK_4_7",
#if RU_INCLUDE_DESC
    "RG_AVAILABLE_TASK_4_7 Register",
    "Available tasks in all runners related to a RNR Group. In case value is zero there are no tasks available for this RNR Group for dispatch hence it should be excluded from the next RNR Group selection",
#endif
    DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_4_7_REG_OFFSET,
    0,
    0,
    406,
};

/******************************************************************************
 * Register: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR
 ******************************************************************************/
const ru_reg_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_REG = 
{
    "DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR",
#if RU_INCLUDE_DESC
    "INTERRUPT_STATUS_REGISTER Register",
    "This register contains the current active TM interrupts. Each asserted bit represents an active interrupt source. The interrupt remains active until the software clears it by writing 1 to the corresponding bit.",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_REG_OFFSET,
    0,
    0,
    407,
};

/******************************************************************************
 * Register: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISM
 ******************************************************************************/
const ru_reg_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISM_REG = 
{
    "DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISM",
#if RU_INCLUDE_DESC
    "INTERRUPT_STATUS_MASKED_REGISTER Register",
    "This register provides only the  enabled interrupts for each of the interrupt sources depicted in the ISR register.",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISM_REG_OFFSET,
    0,
    0,
    408,
};

/******************************************************************************
 * Register: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_IER
 ******************************************************************************/
const ru_reg_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_IER_REG = 
{
    "DSPTCHER_REORDR_TOP_INTR_CTRL_0_IER",
#if RU_INCLUDE_DESC
    "INTERRUPT_ENABLE_REGISTER Register",
    "This register provides an enable mask for each of the interrupt sources depicted in the ISR register.",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_IER_REG_OFFSET,
    0,
    0,
    409,
};

/******************************************************************************
 * Register: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ITR
 ******************************************************************************/
const ru_reg_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ITR_REG = 
{
    "DSPTCHER_REORDR_TOP_INTR_CTRL_0_ITR",
#if RU_INCLUDE_DESC
    "INTERRUPT_TEST_REGISTER Register",
    "This register enables testing by simulating interrupt sources. When the software sets a bit in the ITR, the corresponding bit in the ISR shows an active interrupt. The interrupt remains active until software clears the bit in the ITR",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ITR_REG_OFFSET,
    0,
    0,
    410,
};

/******************************************************************************
 * Register: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR
 ******************************************************************************/
const ru_reg_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_REG = 
{
    "DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR",
#if RU_INCLUDE_DESC
    "INTERRUPT_STATUS_REGISTER Register",
    "This register contains the current active TM interrupts. Each asserted bit represents an active interrupt source. The interrupt remains active until the software clears it by writing 1 to the corresponding bit.",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_REG_OFFSET,
    0,
    0,
    411,
};

/******************************************************************************
 * Register: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISM
 ******************************************************************************/
const ru_reg_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISM_REG = 
{
    "DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISM",
#if RU_INCLUDE_DESC
    "INTERRUPT_STATUS_MASKED_REGISTER Register",
    "This register provides only the  enabled interrupts for each of the interrupt sources depicted in the ISR register.",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISM_REG_OFFSET,
    0,
    0,
    412,
};

/******************************************************************************
 * Register: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_IER
 ******************************************************************************/
const ru_reg_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_IER_REG = 
{
    "DSPTCHER_REORDR_TOP_INTR_CTRL_1_IER",
#if RU_INCLUDE_DESC
    "INTERRUPT_ENABLE_REGISTER Register",
    "This register provides an enable mask for each of the interrupt sources depicted in the ISR register.",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_IER_REG_OFFSET,
    0,
    0,
    413,
};

/******************************************************************************
 * Register: DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ITR
 ******************************************************************************/
const ru_reg_rec DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ITR_REG = 
{
    "DSPTCHER_REORDR_TOP_INTR_CTRL_1_ITR",
#if RU_INCLUDE_DESC
    "INTERRUPT_TEST_REGISTER Register",
    "This register enables testing by simulating interrupt sources. When the software sets a bit in the ITR, the corresponding bit in the ISR shows an active interrupt. The interrupt remains active until software clears the bit in the ITR",
#endif
    DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ITR_REG_OFFSET,
    0,
    0,
    414,
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_BYPSS_CNTRL
 ******************************************************************************/
const ru_reg_rec DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_REG = 
{
    "DEBUG_DBG_BYPSS_CNTRL",
#if RU_INCLUDE_DESC
    "DEBUG_BYPASS_CONTROL Register",
    "Debug Bypass control",
#endif
    DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_REG_OFFSET,
    0,
    0,
    415,
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7
 ******************************************************************************/
const ru_reg_rec DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_REG = 
{
    "DEBUG_GLBL_TSK_CNT_0_7",
#if RU_INCLUDE_DESC
    "TASK_COUNTER_0_7 Register",
    "Counts the amount of active Tasks in RNR",
#endif
    DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_REG_OFFSET,
    0,
    0,
    416,
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15
 ******************************************************************************/
const ru_reg_rec DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_REG = 
{
    "DEBUG_GLBL_TSK_CNT_8_15",
#if RU_INCLUDE_DESC
    "TASK_COUNTER_8_15 Register",
    "Counts the amount of active Tasks in RNR",
#endif
    DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_REG_OFFSET,
    0,
    0,
    417,
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_BUS_CNTRL
 ******************************************************************************/
const ru_reg_rec DSPTCHR_DEBUG_DBG_BUS_CNTRL_REG = 
{
    "DEBUG_DBG_BUS_CNTRL",
#if RU_INCLUDE_DESC
    "DEBUG_BUS_CONTROL Register",
    "Debug bus control which vector to output to the top level",
#endif
    DSPTCHR_DEBUG_DBG_BUS_CNTRL_REG_OFFSET,
    0,
    0,
    418,
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_VEC_0
 ******************************************************************************/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_0_REG = 
{
    "DEBUG_DBG_VEC_0",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_0 Register",
    "Debug vector mapped to registers",
#endif
    DSPTCHR_DEBUG_DBG_VEC_0_REG_OFFSET,
    0,
    0,
    419,
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_VEC_1
 ******************************************************************************/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_1_REG = 
{
    "DEBUG_DBG_VEC_1",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_1 Register",
    "Debug vector mapped to registers",
#endif
    DSPTCHR_DEBUG_DBG_VEC_1_REG_OFFSET,
    0,
    0,
    420,
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_VEC_2
 ******************************************************************************/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_2_REG = 
{
    "DEBUG_DBG_VEC_2",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_2 Register",
    "Debug vector mapped to registers",
#endif
    DSPTCHR_DEBUG_DBG_VEC_2_REG_OFFSET,
    0,
    0,
    421,
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_VEC_3
 ******************************************************************************/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_3_REG = 
{
    "DEBUG_DBG_VEC_3",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_3 Register",
    "Debug vector mapped to registers",
#endif
    DSPTCHR_DEBUG_DBG_VEC_3_REG_OFFSET,
    0,
    0,
    422,
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_VEC_4
 ******************************************************************************/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_4_REG = 
{
    "DEBUG_DBG_VEC_4",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_4 Register",
    "Debug vector mapped to registers",
#endif
    DSPTCHR_DEBUG_DBG_VEC_4_REG_OFFSET,
    0,
    0,
    423,
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_VEC_5
 ******************************************************************************/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_5_REG = 
{
    "DEBUG_DBG_VEC_5",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_5 Register",
    "Debug vector mapped to registers",
#endif
    DSPTCHR_DEBUG_DBG_VEC_5_REG_OFFSET,
    0,
    0,
    424,
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_VEC_6
 ******************************************************************************/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_6_REG = 
{
    "DEBUG_DBG_VEC_6",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_6 Register",
    "Debug vector mapped to registers",
#endif
    DSPTCHR_DEBUG_DBG_VEC_6_REG_OFFSET,
    0,
    0,
    425,
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_VEC_7
 ******************************************************************************/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_7_REG = 
{
    "DEBUG_DBG_VEC_7",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_7 Register",
    "Debug vector mapped to registers",
#endif
    DSPTCHR_DEBUG_DBG_VEC_7_REG_OFFSET,
    0,
    0,
    426,
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_VEC_8
 ******************************************************************************/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_8_REG = 
{
    "DEBUG_DBG_VEC_8",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_8 Register",
    "Debug vector mapped to registers",
#endif
    DSPTCHR_DEBUG_DBG_VEC_8_REG_OFFSET,
    0,
    0,
    427,
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_VEC_9
 ******************************************************************************/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_9_REG = 
{
    "DEBUG_DBG_VEC_9",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_9 Register",
    "Debug vector mapped to registers",
#endif
    DSPTCHR_DEBUG_DBG_VEC_9_REG_OFFSET,
    0,
    0,
    428,
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_VEC_10
 ******************************************************************************/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_10_REG = 
{
    "DEBUG_DBG_VEC_10",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_10 Register",
    "Debug vector mapped to registers",
#endif
    DSPTCHR_DEBUG_DBG_VEC_10_REG_OFFSET,
    0,
    0,
    429,
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_VEC_11
 ******************************************************************************/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_11_REG = 
{
    "DEBUG_DBG_VEC_11",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_11 Register",
    "Debug vector mapped to registers",
#endif
    DSPTCHR_DEBUG_DBG_VEC_11_REG_OFFSET,
    0,
    0,
    430,
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_VEC_12
 ******************************************************************************/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_12_REG = 
{
    "DEBUG_DBG_VEC_12",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_12 Register",
    "Debug vector mapped to registers",
#endif
    DSPTCHR_DEBUG_DBG_VEC_12_REG_OFFSET,
    0,
    0,
    431,
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_VEC_13
 ******************************************************************************/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_13_REG = 
{
    "DEBUG_DBG_VEC_13",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_13 Register",
    "Debug vector mapped to registers",
#endif
    DSPTCHR_DEBUG_DBG_VEC_13_REG_OFFSET,
    0,
    0,
    432,
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_VEC_14
 ******************************************************************************/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_14_REG = 
{
    "DEBUG_DBG_VEC_14",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_14 Register",
    "Debug vector mapped to registers",
#endif
    DSPTCHR_DEBUG_DBG_VEC_14_REG_OFFSET,
    0,
    0,
    433,
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_VEC_15
 ******************************************************************************/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_15_REG = 
{
    "DEBUG_DBG_VEC_15",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_15 Register",
    "Debug vector mapped to registers",
#endif
    DSPTCHR_DEBUG_DBG_VEC_15_REG_OFFSET,
    0,
    0,
    434,
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_VEC_16
 ******************************************************************************/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_16_REG = 
{
    "DEBUG_DBG_VEC_16",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_16 Register",
    "Debug vector mapped to registers",
#endif
    DSPTCHR_DEBUG_DBG_VEC_16_REG_OFFSET,
    0,
    0,
    435,
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_VEC_17
 ******************************************************************************/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_17_REG = 
{
    "DEBUG_DBG_VEC_17",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_17 Register",
    "Debug vector mapped to registers",
#endif
    DSPTCHR_DEBUG_DBG_VEC_17_REG_OFFSET,
    0,
    0,
    436,
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_VEC_18
 ******************************************************************************/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_18_REG = 
{
    "DEBUG_DBG_VEC_18",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_18 Register",
    "Debug vector mapped to registers",
#endif
    DSPTCHR_DEBUG_DBG_VEC_18_REG_OFFSET,
    0,
    0,
    437,
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_VEC_19
 ******************************************************************************/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_19_REG = 
{
    "DEBUG_DBG_VEC_19",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_19 Register",
    "Debug vector mapped to registers",
#endif
    DSPTCHR_DEBUG_DBG_VEC_19_REG_OFFSET,
    0,
    0,
    438,
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_VEC_20
 ******************************************************************************/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_20_REG = 
{
    "DEBUG_DBG_VEC_20",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_20 Register",
    "Debug vector mapped to registers",
#endif
    DSPTCHR_DEBUG_DBG_VEC_20_REG_OFFSET,
    0,
    0,
    439,
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_VEC_21
 ******************************************************************************/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_21_REG = 
{
    "DEBUG_DBG_VEC_21",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_21 Register",
    "Debug vector mapped to registers",
#endif
    DSPTCHR_DEBUG_DBG_VEC_21_REG_OFFSET,
    0,
    0,
    440,
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_VEC_22
 ******************************************************************************/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_22_REG = 
{
    "DEBUG_DBG_VEC_22",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_22 Register",
    "Debug vector mapped to registers",
#endif
    DSPTCHR_DEBUG_DBG_VEC_22_REG_OFFSET,
    0,
    0,
    441,
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_DBG_VEC_23
 ******************************************************************************/
const ru_reg_rec DSPTCHR_DEBUG_DBG_VEC_23_REG = 
{
    "DEBUG_DBG_VEC_23",
#if RU_INCLUDE_DESC
    "DEBUG_VEC_23 Register",
    "Debug vector mapped to registers",
#endif
    DSPTCHR_DEBUG_DBG_VEC_23_REG_OFFSET,
    0,
    0,
    442,
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL
 ******************************************************************************/
const ru_reg_rec DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_REG = 
{
    "DEBUG_STATISTICS_DBG_STTSTCS_CTRL",
#if RU_INCLUDE_DESC
    "DEBUG_STATISTICS_CONTROL Register",
    "Controls which information to log",
#endif
    DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_REG_OFFSET,
    0,
    0,
    443,
};

/******************************************************************************
 * Register: DSPTCHR_DEBUG_STATISTICS_DBG_CNT
 ******************************************************************************/
const ru_reg_rec DSPTCHR_DEBUG_STATISTICS_DBG_CNT_REG = 
{
    "DEBUG_STATISTICS_DBG_CNT",
#if RU_INCLUDE_DESC
    "DEBUG_COUNT %i Register",
    "Debug counter",
#endif
    DSPTCHR_DEBUG_STATISTICS_DBG_CNT_REG_OFFSET,
    DSPTCHR_DEBUG_STATISTICS_DBG_CNT_REG_RAM_CNT,
    4,
    444,
};

/******************************************************************************
 * Register: DSPTCHR_QDES_HEAD
 ******************************************************************************/
const ru_reg_rec DSPTCHR_QDES_HEAD_REG = 
{
    "QDES_HEAD",
#if RU_INCLUDE_DESC
    "HEAD Register",
    "Pointer to the first BD in the link list of this queue.",
#endif
    DSPTCHR_QDES_HEAD_REG_OFFSET,
    DSPTCHR_QDES_HEAD_REG_RAM_CNT,
    32,
    445,
};

/******************************************************************************
 * Register: DSPTCHR_QDES_BFOUT
 ******************************************************************************/
const ru_reg_rec DSPTCHR_QDES_BFOUT_REG = 
{
    "QDES_BFOUT",
#if RU_INCLUDE_DESC
    "BFOUT Register",
    "32 bit wrap around counter. Counts number of packets that left this queue since start of queue activity.",
#endif
    DSPTCHR_QDES_BFOUT_REG_OFFSET,
    DSPTCHR_QDES_BFOUT_REG_RAM_CNT,
    32,
    446,
};

/******************************************************************************
 * Register: DSPTCHR_QDES_BUFIN
 ******************************************************************************/
const ru_reg_rec DSPTCHR_QDES_BUFIN_REG = 
{
    "QDES_BUFIN",
#if RU_INCLUDE_DESC
    "BUFIN Register",
    "32 bit wrap around counter. Counts number of packets that entered this queue since start of queue activity.",
#endif
    DSPTCHR_QDES_BUFIN_REG_OFFSET,
    DSPTCHR_QDES_BUFIN_REG_RAM_CNT,
    32,
    447,
};

/******************************************************************************
 * Register: DSPTCHR_QDES_TAIL
 ******************************************************************************/
const ru_reg_rec DSPTCHR_QDES_TAIL_REG = 
{
    "QDES_TAIL",
#if RU_INCLUDE_DESC
    "TAIL Register",
    "Pointer to the last BD in the linked list of this queue.",
#endif
    DSPTCHR_QDES_TAIL_REG_OFFSET,
    DSPTCHR_QDES_TAIL_REG_RAM_CNT,
    32,
    448,
};

/******************************************************************************
 * Register: DSPTCHR_QDES_FBDNULL
 ******************************************************************************/
const ru_reg_rec DSPTCHR_QDES_FBDNULL_REG = 
{
    "QDES_FBDNULL",
#if RU_INCLUDE_DESC
    "FBDNULL Register",
    "If this bit is set then the first BD attached to this Q is a null BD. In this case, its Data Pointer field is not valid, but its Next BD pointer field is valid. When it is set, the NullBD field for this queue is not valid.",
#endif
    DSPTCHR_QDES_FBDNULL_REG_OFFSET,
    DSPTCHR_QDES_FBDNULL_REG_RAM_CNT,
    32,
    449,
};

/******************************************************************************
 * Register: DSPTCHR_QDES_NULLBD
 ******************************************************************************/
const ru_reg_rec DSPTCHR_QDES_NULLBD_REG = 
{
    "QDES_NULLBD",
#if RU_INCLUDE_DESC
    "NULLBD Register",
    "32 bits index of a Null BD that belongs to this queue. Both the data buffer pointer and the next BD field are non valid. The pointer defines a memory allocation for a BD that might be used or not.",
#endif
    DSPTCHR_QDES_NULLBD_REG_OFFSET,
    DSPTCHR_QDES_NULLBD_REG_RAM_CNT,
    32,
    450,
};

/******************************************************************************
 * Register: DSPTCHR_QDES_BUFAVAIL
 ******************************************************************************/
const ru_reg_rec DSPTCHR_QDES_BUFAVAIL_REG = 
{
    "QDES_BUFAVAIL",
#if RU_INCLUDE_DESC
    "BUFAVAIL Register",
    "number of entries available in queue."
    "bufin - bfout",
#endif
    DSPTCHR_QDES_BUFAVAIL_REG_OFFSET,
    DSPTCHR_QDES_BUFAVAIL_REG_RAM_CNT,
    32,
    451,
};

/******************************************************************************
 * Register: DSPTCHR_QDES_REG_Q_HEAD
 ******************************************************************************/
const ru_reg_rec DSPTCHR_QDES_REG_Q_HEAD_REG = 
{
    "QDES_REG_Q_HEAD",
#if RU_INCLUDE_DESC
    "QUEUE_HEAD %i Register",
    "Q Head Buffer, Used for the dispatching logic",
#endif
    DSPTCHR_QDES_REG_Q_HEAD_REG_OFFSET,
    DSPTCHR_QDES_REG_Q_HEAD_REG_RAM_CNT,
    4,
    452,
};

/******************************************************************************
 * Register: DSPTCHR_QDES_REG_VIQ_HEAD_VLD
 ******************************************************************************/
const ru_reg_rec DSPTCHR_QDES_REG_VIQ_HEAD_VLD_REG = 
{
    "QDES_REG_VIQ_HEAD_VLD",
#if RU_INCLUDE_DESC
    "VIQ_HEAD_VALID Register",
    "This register will hold the for each VIQ if the Head of the Q is valid or not."
    "These Queues are for Dispatch"
    "",
#endif
    DSPTCHR_QDES_REG_VIQ_HEAD_VLD_REG_OFFSET,
    0,
    0,
    453,
};

/******************************************************************************
 * Register: DSPTCHR_QDES_REG_VIQ_CHRNCY_VLD
 ******************************************************************************/
const ru_reg_rec DSPTCHR_QDES_REG_VIQ_CHRNCY_VLD_REG = 
{
    "QDES_REG_VIQ_CHRNCY_VLD",
#if RU_INCLUDE_DESC
    "VIQ_COHERENCY_VALID Register",
    "This register will hold for each VIQ if the Coherency counter is larger than zero."
    "",
#endif
    DSPTCHR_QDES_REG_VIQ_CHRNCY_VLD_REG_OFFSET,
    0,
    0,
    454,
};

/******************************************************************************
 * Register: DSPTCHR_QDES_REG_VEQ_HEAD_VLD
 ******************************************************************************/
const ru_reg_rec DSPTCHR_QDES_REG_VEQ_HEAD_VLD_REG = 
{
    "QDES_REG_VEQ_HEAD_VLD",
#if RU_INCLUDE_DESC
    "VEQ_HEAD_VALID Register",
    "This register will hold the for each VEQ if the Head of the Q is valid or not"
    "These Queues are for ReOrder",
#endif
    DSPTCHR_QDES_REG_VEQ_HEAD_VLD_REG_OFFSET,
    0,
    0,
    455,
};

/******************************************************************************
 * Register: DSPTCHR_QDES_REG_QDES_BUF_AVL_CNTRL
 ******************************************************************************/
const ru_reg_rec DSPTCHR_QDES_REG_QDES_BUF_AVL_CNTRL_REG = 
{
    "QDES_REG_QDES_BUF_AVL_CNTRL",
#if RU_INCLUDE_DESC
    "QDES_BUF_AVAIL_CONTROL Register",
    "Todays implementation does not require that QDES available buffer be different than zero. so this register controls whether or not to it should affect poping from the QDES or not",
#endif
    DSPTCHR_QDES_REG_QDES_BUF_AVL_CNTRL_REG_OFFSET,
    0,
    0,
    456,
};

/******************************************************************************
 * Register: DSPTCHR_FLLDES_HEAD
 ******************************************************************************/
const ru_reg_rec DSPTCHR_FLLDES_HEAD_REG = 
{
    "FLLDES_HEAD",
#if RU_INCLUDE_DESC
    "HEAD Register",
    "Pointer to the first BD in the link list of this queue.",
#endif
    DSPTCHR_FLLDES_HEAD_REG_OFFSET,
    0,
    0,
    457,
};

/******************************************************************************
 * Register: DSPTCHR_FLLDES_BFOUT
 ******************************************************************************/
const ru_reg_rec DSPTCHR_FLLDES_BFOUT_REG = 
{
    "FLLDES_BFOUT",
#if RU_INCLUDE_DESC
    "BFOUT Register",
    "32 bit wrap around counter. Counts number of entries that left this queue since start of queue activity.",
#endif
    DSPTCHR_FLLDES_BFOUT_REG_OFFSET,
    0,
    0,
    458,
};

/******************************************************************************
 * Register: DSPTCHR_FLLDES_BFIN
 ******************************************************************************/
const ru_reg_rec DSPTCHR_FLLDES_BFIN_REG = 
{
    "FLLDES_BFIN",
#if RU_INCLUDE_DESC
    "BFIN Register",
    "32 bit wrap around counter. Counts number of entries that entered this queue since start of queue activity.",
#endif
    DSPTCHR_FLLDES_BFIN_REG_OFFSET,
    0,
    0,
    459,
};

/******************************************************************************
 * Register: DSPTCHR_FLLDES_TAIL
 ******************************************************************************/
const ru_reg_rec DSPTCHR_FLLDES_TAIL_REG = 
{
    "FLLDES_TAIL",
#if RU_INCLUDE_DESC
    "TAIL Register",
    "Pointer to the last BD in the linked list of this queue.",
#endif
    DSPTCHR_FLLDES_TAIL_REG_OFFSET,
    0,
    0,
    460,
};

/******************************************************************************
 * Register: DSPTCHR_FLLDES_FLLDROP
 ******************************************************************************/
const ru_reg_rec DSPTCHR_FLLDES_FLLDROP_REG = 
{
    "FLLDES_FLLDROP",
#if RU_INCLUDE_DESC
    "FLLDROP Register",
    "32 bit counter that counts the number of packets arrived when there is no free BD in the FLL.",
#endif
    DSPTCHR_FLLDES_FLLDROP_REG_OFFSET,
    0,
    0,
    461,
};

/******************************************************************************
 * Register: DSPTCHR_FLLDES_LTINT
 ******************************************************************************/
const ru_reg_rec DSPTCHR_FLLDES_LTINT_REG = 
{
    "FLLDES_LTINT",
#if RU_INCLUDE_DESC
    "LTINT Register",
    "Low threshold Interrupt. When number of bytes reach this level, then an interrupt is generated to the Host.",
#endif
    DSPTCHR_FLLDES_LTINT_REG_OFFSET,
    0,
    0,
    462,
};

/******************************************************************************
 * Register: DSPTCHR_FLLDES_BUFAVAIL
 ******************************************************************************/
const ru_reg_rec DSPTCHR_FLLDES_BUFAVAIL_REG = 
{
    "FLLDES_BUFAVAIL",
#if RU_INCLUDE_DESC
    "BUFAVAIL Register",
    "number of entries available in queue."
    "bufin - bfout",
#endif
    DSPTCHR_FLLDES_BUFAVAIL_REG_OFFSET,
    0,
    0,
    463,
};

/******************************************************************************
 * Register: DSPTCHR_FLLDES_FREEMIN
 ******************************************************************************/
const ru_reg_rec DSPTCHR_FLLDES_FREEMIN_REG = 
{
    "FLLDES_FREEMIN",
#if RU_INCLUDE_DESC
    "FREEMIN Register",
    "Save the MIN size of free BD in the system that has been recorded during work.",
#endif
    DSPTCHR_FLLDES_FREEMIN_REG_OFFSET,
    0,
    0,
    464,
};

/******************************************************************************
 * Register: DSPTCHR_BDRAM_NEXT_DATA
 ******************************************************************************/
const ru_reg_rec DSPTCHR_BDRAM_NEXT_DATA_REG = 
{
    "BDRAM_NEXT_DATA",
#if RU_INCLUDE_DESC
    "BD %i Register",
    "This Memory holds the Buffer Descriptor (BD) entries.",
#endif
    DSPTCHR_BDRAM_NEXT_DATA_REG_OFFSET,
    DSPTCHR_BDRAM_NEXT_DATA_REG_RAM_CNT,
    4,
    465,
};

/******************************************************************************
 * Register: DSPTCHR_BDRAM_PREV_DATA
 ******************************************************************************/
const ru_reg_rec DSPTCHR_BDRAM_PREV_DATA_REG = 
{
    "BDRAM_PREV_DATA",
#if RU_INCLUDE_DESC
    "BD %i Register",
    "This Memory holds the Buffer Descriptor (BD) entries.",
#endif
    DSPTCHR_BDRAM_PREV_DATA_REG_OFFSET,
    DSPTCHR_BDRAM_PREV_DATA_REG_RAM_CNT,
    4,
    466,
};

/******************************************************************************
 * Register: DSPTCHR_PDRAM_DATA
 ******************************************************************************/
const ru_reg_rec DSPTCHR_PDRAM_DATA_REG = 
{
    "PDRAM_DATA",
#if RU_INCLUDE_DESC
    "PDRAM %i Register",
    "This memory holds the Packet descriptors.",
#endif
    DSPTCHR_PDRAM_DATA_REG_OFFSET,
    DSPTCHR_PDRAM_DATA_REG_RAM_CNT,
    4,
    467,
};

/******************************************************************************
 * Block: DSPTCHR
 ******************************************************************************/
static const ru_reg_rec *DSPTCHR_REGS[] =
{
    &DSPTCHR_REORDER_CFG_DSPTCHR_REORDR_CFG_REG,
    &DSPTCHR_REORDER_CFG_VQ_EN_REG,
    &DSPTCHR_REORDER_CFG_BB_CFG_REG,
    &DSPTCHR_REORDER_CFG_CLK_GATE_CNTRL_REG,
    &DSPTCHR_CONGESTION_INGRS_CONGSTN_REG,
    &DSPTCHR_CONGESTION_EGRS_CONGSTN_REG,
    &DSPTCHR_CONGESTION_TOTAL_EGRS_CONGSTN_REG,
    &DSPTCHR_CONGESTION_GLBL_CONGSTN_REG,
    &DSPTCHR_CONGESTION_CONGSTN_STATUS_REG,
    &DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_LOW_REG,
    &DSPTCHR_CONGESTION_PER_Q_INGRS_CONGSTN_HIGH_REG,
    &DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_LOW_REG,
    &DSPTCHR_CONGESTION_PER_Q_EGRS_CONGSTN_HIGH_REG,
    &DSPTCHR_INGRS_QUEUES_Q_INGRS_SIZE_REG,
    &DSPTCHR_INGRS_QUEUES_Q_INGRS_LIMITS_REG,
    &DSPTCHR_INGRS_QUEUES_Q_INGRS_COHRENCY_REG,
    &DSPTCHR_QUEUE_MAPPING_CRDT_CFG_REG,
    &DSPTCHR_QUEUE_MAPPING_PD_DSPTCH_ADD_REG,
    &DSPTCHR_QUEUE_MAPPING_Q_DEST_REG,
    &DSPTCHR_POOL_SIZES_CMN_POOL_LMT_REG,
    &DSPTCHR_POOL_SIZES_CMN_POOL_SIZE_REG,
    &DSPTCHR_POOL_SIZES_GRNTED_POOL_LMT_REG,
    &DSPTCHR_POOL_SIZES_GRNTED_POOL_SIZE_REG,
    &DSPTCHR_POOL_SIZES_MULTI_CST_POOL_LMT_REG,
    &DSPTCHR_POOL_SIZES_MULTI_CST_POOL_SIZE_REG,
    &DSPTCHR_POOL_SIZES_RNR_POOL_LMT_REG,
    &DSPTCHR_POOL_SIZES_RNR_POOL_SIZE_REG,
    &DSPTCHR_POOL_SIZES_PRCSSING_POOL_SIZE_REG,
    &DSPTCHR_MASK_MSK_TSK_255_0_REG,
    &DSPTCHR_MASK_MSK_Q_REG,
    &DSPTCHR_MASK_DLY_Q_REG,
    &DSPTCHR_MASK_NON_DLY_Q_REG,
    &DSPTCHR_EGRS_QUEUES_EGRS_DLY_QM_CRDT_REG,
    &DSPTCHR_EGRS_QUEUES_EGRS_NON_DLY_QM_CRDT_REG,
    &DSPTCHR_EGRS_QUEUES_TOTAL_Q_EGRS_SIZE_REG,
    &DSPTCHR_EGRS_QUEUES_PER_Q_EGRS_SIZE_REG,
    &DSPTCHR_WAKEUP_CONTROL_WKUP_REQ_REG,
    &DSPTCHR_WAKEUP_CONTROL_WKUP_THRSHLD_REG,
    &DSPTCHR_DISPTCH_SCHEDULING_DWRR_INFO_REG,
    &DSPTCHR_DISPTCH_SCHEDULING_VLD_CRDT_REG,
    &DSPTCHR_LOAD_BALANCING_LB_CFG_REG,
    &DSPTCHR_LOAD_BALANCING_FREE_TASK_0_1_REG,
    &DSPTCHR_LOAD_BALANCING_FREE_TASK_2_3_REG,
    &DSPTCHR_LOAD_BALANCING_FREE_TASK_4_5_REG,
    &DSPTCHR_LOAD_BALANCING_FREE_TASK_6_7_REG,
    &DSPTCHR_LOAD_BALANCING_FREE_TASK_8_9_REG,
    &DSPTCHR_LOAD_BALANCING_FREE_TASK_10_11_REG,
    &DSPTCHR_LOAD_BALANCING_FREE_TASK_12_13_REG,
    &DSPTCHR_LOAD_BALANCING_FREE_TASK_14_15_REG,
    &DSPTCHR_LOAD_BALANCING_TSK_TO_RG_MAPPING_REG,
    &DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_0_3_REG,
    &DSPTCHR_LOAD_BALANCING_RG_AVLABL_TSK_4_7_REG,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISR_REG,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ISM_REG,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_IER_REG,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_0_ITR_REG,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISR_REG,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ISM_REG,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_IER_REG,
    &DSPTCHR_DSPTCHER_REORDR_TOP_INTR_CTRL_1_ITR_REG,
    &DSPTCHR_DEBUG_DBG_BYPSS_CNTRL_REG,
    &DSPTCHR_DEBUG_GLBL_TSK_CNT_0_7_REG,
    &DSPTCHR_DEBUG_GLBL_TSK_CNT_8_15_REG,
    &DSPTCHR_DEBUG_DBG_BUS_CNTRL_REG,
    &DSPTCHR_DEBUG_DBG_VEC_0_REG,
    &DSPTCHR_DEBUG_DBG_VEC_1_REG,
    &DSPTCHR_DEBUG_DBG_VEC_2_REG,
    &DSPTCHR_DEBUG_DBG_VEC_3_REG,
    &DSPTCHR_DEBUG_DBG_VEC_4_REG,
    &DSPTCHR_DEBUG_DBG_VEC_5_REG,
    &DSPTCHR_DEBUG_DBG_VEC_6_REG,
    &DSPTCHR_DEBUG_DBG_VEC_7_REG,
    &DSPTCHR_DEBUG_DBG_VEC_8_REG,
    &DSPTCHR_DEBUG_DBG_VEC_9_REG,
    &DSPTCHR_DEBUG_DBG_VEC_10_REG,
    &DSPTCHR_DEBUG_DBG_VEC_11_REG,
    &DSPTCHR_DEBUG_DBG_VEC_12_REG,
    &DSPTCHR_DEBUG_DBG_VEC_13_REG,
    &DSPTCHR_DEBUG_DBG_VEC_14_REG,
    &DSPTCHR_DEBUG_DBG_VEC_15_REG,
    &DSPTCHR_DEBUG_DBG_VEC_16_REG,
    &DSPTCHR_DEBUG_DBG_VEC_17_REG,
    &DSPTCHR_DEBUG_DBG_VEC_18_REG,
    &DSPTCHR_DEBUG_DBG_VEC_19_REG,
    &DSPTCHR_DEBUG_DBG_VEC_20_REG,
    &DSPTCHR_DEBUG_DBG_VEC_21_REG,
    &DSPTCHR_DEBUG_DBG_VEC_22_REG,
    &DSPTCHR_DEBUG_DBG_VEC_23_REG,
    &DSPTCHR_DEBUG_STATISTICS_DBG_STTSTCS_CTRL_REG,
    &DSPTCHR_DEBUG_STATISTICS_DBG_CNT_REG,
    &DSPTCHR_QDES_HEAD_REG,
    &DSPTCHR_QDES_BFOUT_REG,
    &DSPTCHR_QDES_BUFIN_REG,
    &DSPTCHR_QDES_TAIL_REG,
    &DSPTCHR_QDES_FBDNULL_REG,
    &DSPTCHR_QDES_NULLBD_REG,
    &DSPTCHR_QDES_BUFAVAIL_REG,
    &DSPTCHR_QDES_REG_Q_HEAD_REG,
    &DSPTCHR_QDES_REG_VIQ_HEAD_VLD_REG,
    &DSPTCHR_QDES_REG_VIQ_CHRNCY_VLD_REG,
    &DSPTCHR_QDES_REG_VEQ_HEAD_VLD_REG,
    &DSPTCHR_QDES_REG_QDES_BUF_AVL_CNTRL_REG,
    &DSPTCHR_FLLDES_HEAD_REG,
    &DSPTCHR_FLLDES_BFOUT_REG,
    &DSPTCHR_FLLDES_BFIN_REG,
    &DSPTCHR_FLLDES_TAIL_REG,
    &DSPTCHR_FLLDES_FLLDROP_REG,
    &DSPTCHR_FLLDES_LTINT_REG,
    &DSPTCHR_FLLDES_BUFAVAIL_REG,
    &DSPTCHR_FLLDES_FREEMIN_REG,
    &DSPTCHR_BDRAM_NEXT_DATA_REG,
    &DSPTCHR_BDRAM_PREV_DATA_REG,
    &DSPTCHR_PDRAM_DATA_REG,
};

unsigned long DSPTCHR_ADDRS[] =
{
    0x82d80000,
};

const ru_block_rec DSPTCHR_BLOCK = 
{
    "DSPTCHR",
    DSPTCHR_ADDRS,
    1,
    113,
    DSPTCHR_REGS
};

/* End of file XRDP_DSPTCHR.c */
