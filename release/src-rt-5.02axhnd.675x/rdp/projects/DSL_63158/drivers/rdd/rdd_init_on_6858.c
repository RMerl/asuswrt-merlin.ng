/*
* <:copyright-BRCM:2014:proprietary:standard
* 
*    Copyright (c) 2014 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
* :>
*/


#include "rdd.h"
#include "rdd_defs.h"
#include "rdd_init.h"
#include "rdd_iptv_processing.h"
#include "rdd_qos_mapper.h"
#include "XRDP_AG.h"
#include "rdd_map_auto.h"
#include "rdd_ghost_reporting.h"
#include "rdd_scheduling.h"
#include "rdd_common.h"
#include "rdd_cpu.h"
#include "rdp_common.h"
#include "rdd_tuple_lkp.h"
#include "rdd_runner_proj_defs.h"
#include "rdd_tcam_ic.h"
#include "rdd_ingress_filter.h"
#include "rdd_iptv.h"
#include "rdp_drv_rnr.h"

extern uintptr_t xrdp_virt2phys(const ru_block_rec *ru_block, uint8_t addr_idx);

#ifdef RDP_SIM
extern uint8_t *soc_base_address;
extern uint32_t natc_lkp_table_ptr;
extern int rdd_sim_alloc_segments(void);
extern void rdd_sim_free_segments(void);
#endif

extern int reg_id[32];
DEFINE_BDMF_FASTLOCK(int_lock);
DEFINE_BDMF_FASTLOCK(iptv_lock);
DEFINE_BDMF_FASTLOCK(int_lock_irq);

#ifdef USE_BDMF_SHELL
extern int rdd_make_shell_commands(void);
#endif /* USE_BDMF_SHELL */

rdd_bb_id rdpa_emac_to_bb_id_rx[rdpa_emac__num_of] = {
    BB_ID_RX_XLMAC0_0_10G,
    BB_ID_RX_XLMAC0_1_2P5G,
    BB_ID_RX_XLMAC0_2_1G,
    BB_ID_RX_XLMAC0_3_1G,
    BB_ID_RX_XLMAC1_0_RGMII,
    BB_ID_RX_XLMAC1_1_RGMII,
    BB_ID_RX_XLMAC1_2_RGMII,
    BB_ID_LAST,
};

rdd_bb_id rdpa_emac_to_bb_id_tx[rdpa_emac__num_of] = {
    BB_ID_TX_XLMAC0_0_10G,
    BB_ID_TX_XLMAC0_1_2P5G,
    BB_ID_TX_XLMAC0_2_1G,
    BB_ID_TX_XLMAC0_3_1G,
    BB_ID_TX_XLMAC1_0_RGMII,
    BB_ID_TX_XLMAC1_1_RGMII,
    BB_ID_TX_XLMAC1_2_RGMII,
    BB_ID_LAST,
};

bbh_id_e rdpa_emac_to_bbh_id_e[rdpa_emac__num_of] = {
    BBH_ID_XLMAC0_0_10G,
    BBH_ID_XLMAC0_1_2p5G,
    BBH_ID_XLMAC0_2_1G,
    BBH_ID_XLMAC0_3_1G,
    BBH_ID_XLMAC1_0_RGMII,
    BBH_ID_XLMAC1_1_RGMII,
    BBH_ID_XLMAC1_2_RGMII,
    BBH_ID_NULL,
};

rdd_vport_id_t rx_flow_to_vport[RX_FLOW_CONTEXTS_NUMBER] = {
    [0 ... RDD_MAX_WAN_FLOW]  = RDD_WAN0_VPORT,
    [RDD_NUM_OF_WAN_FLOWS + BB_ID_RX_XLMAC0_0_10G] = RDD_LAN0_VPORT,
    [RDD_NUM_OF_WAN_FLOWS + BB_ID_RX_XLMAC0_1_2P5G] = RDD_LAN1_VPORT,
    [RDD_NUM_OF_WAN_FLOWS + BB_ID_RX_XLMAC0_2_1G] = RDD_LAN2_VPORT,
    [RDD_NUM_OF_WAN_FLOWS + BB_ID_RX_XLMAC0_3_1G] = RDD_LAN3_VPORT,
    [RDD_NUM_OF_WAN_FLOWS + BB_ID_RX_XLMAC1_0_RGMII] = RDD_LAN4_VPORT,
    [RDD_NUM_OF_WAN_FLOWS + BB_ID_RX_XLMAC1_1_RGMII] = RDD_LAN5_VPORT,
    [RDD_NUM_OF_WAN_FLOWS + BB_ID_RX_XLMAC1_2_RGMII] = RDD_LAN6_VPORT,
    [RDD_NUM_OF_WAN_FLOWS + BB_ID_CPU0] = RDD_CPU0_VPORT,
    [RDD_NUM_OF_WAN_FLOWS + BB_ID_CPU1] = RDD_CPU1_VPORT,
    [RDD_NUM_OF_WAN_FLOWS + BB_ID_CPU2] = RDD_CPU2_VPORT,
};

int rdd_init(void)
{
#ifdef RDP_SIM
    if (rdd_sim_alloc_segments())
        return -1;
#endif
    return 0;
}

void rdd_exit(void)
{
#ifdef RDP_SIM
    rdd_sim_free_segments();
#endif
}

#ifndef RDP_SIM
void rdp_rnr_write_context(void *__to, void *__from, unsigned int __n);
#endif

static void rdd_global_registers_init(void)
{
    static uint32_t global_regs[8] = {};
    uint32_t i;

    /********** common to all runners **********/
    global_regs[1] = 1; /* R1 = 1 */

    global_regs[2] = IMAGE_2_CPU_RECYCLE_SRAM_PD_FIFO_ADDRESS;

#ifdef BCM6858_A0
    global_regs[3] |= (1 << GLOBAL_CFG_REG_IS_6858A0);
#endif

    for (i = 0; i < 8; ++i)
        RDD_BYTES_4_BITS_WRITE_G(global_regs[i], RDD_RUNNER_GLOBAL_REGISTERS_INIT_ADDRESS_ARR, i);
}

static void image_0_context_set(uint32_t core_index)
{
    static uint32_t local_regs[NUM_OF_MAIN_RUNNER_THREADS][32];
    uint32_t mem_cntxt_byte_num;
    uint32_t *sram_context;

    sram_context = (uint32_t *)RUNNER_CORE_CONTEXT_ADDRESS(core_index);
    mem_cntxt_byte_num = (RU_REG_RAM_CNT(RNR_CNTXT, MEM_ENTRY) + 1) * sizeof(uint32_t);
#ifndef XRDP_EMULATION
    /* read the local registers from the Context memory - maybe it was initialized by the ACE compiler */
    MREAD_BLK_32(local_regs, sram_context, mem_cntxt_byte_num);
#endif

    /* DIRECT PROCESSING */
    local_regs[IMAGE_0_DS_TM_WAN_DIRECT_THREAD_NUMBER][reg_id[9]] = IMAGE_0_RX_FLOW_TABLE_ADDRESS << 16 | IMAGE_0_DS_TM_WAN_DIRECT_THREAD_NUMBER;
    local_regs[IMAGE_0_DS_TM_WAN_DIRECT_THREAD_NUMBER][reg_id[10]] = BB_ID_DISPATCHER_REORDER;
    local_regs[IMAGE_0_DS_TM_WAN_DIRECT_THREAD_NUMBER][reg_id[11]] = QM_QUEUE_CPU_RX;
    local_regs[IMAGE_0_DS_TM_WAN_DIRECT_THREAD_NUMBER][reg_id[12]] = IMAGE_0_SCRATCH_ADDRESS;
    local_regs[IMAGE_0_DS_TM_WAN_DIRECT_THREAD_NUMBER][reg_id[16]] = 
        ADDRESS_OF(image_0, gpon_control_wakeup_request) << 16 | IMAGE_0_DIRECT_PROCESSING_PD_TABLE_ADDRESS;

    /* Budget allocation */
    local_regs[IMAGE_0_DS_TM_BUDGET_ALLOCATION_THREAD_NUMBER][reg_id[16]] = ADDRESS_OF(image_0, budget_allocator_1st_wakeup_request) << 16;
    local_regs[IMAGE_0_DS_TM_BUDGET_ALLOCATION_THREAD_NUMBER][reg_id[12]] = IMAGE_0_SCHEDULING_QUEUE_TABLE_ADDRESS | (IMAGE_0_COMPLEX_SCHEDULER_TABLE_ADDRESS << 16);
    local_regs[IMAGE_0_DS_TM_BUDGET_ALLOCATION_THREAD_NUMBER][reg_id[13]] = (RDD_BASIC_RATE_LIMITER_TABLE_SIZE / 4) * sizeof(RDD_BASIC_RATE_LIMITER_DESCRIPTOR_DTS);
    local_regs[IMAGE_0_DS_TM_BUDGET_ALLOCATION_THREAD_NUMBER][reg_id[11]] = BB_ID_TX_LAN;
    local_regs[IMAGE_0_DS_TM_BUDGET_ALLOCATION_THREAD_NUMBER][reg_id[10]] = IMAGE_0_RATE_LIMITER_VALID_TABLE_ADDRESS;
    local_regs[IMAGE_0_DS_TM_BUDGET_ALLOCATION_THREAD_NUMBER][reg_id[9]]  = IMAGE_0_BASIC_RATE_LIMITER_TABLE_ADDRESS | (IMAGE_0_BASIC_SCHEDULER_TABLE_ADDRESS << 16);

    /* UPDATE_FIFO_READ */
    local_regs[IMAGE_0_DS_TM_UPDATE_FIFO_THREAD_NUMBER][reg_id[16]] = ADDRESS_OF(image_0, update_fifo_read_1st_wakeup_request) << 16;
    local_regs[IMAGE_0_DS_TM_UPDATE_FIFO_THREAD_NUMBER][reg_id[14]] = IMAGE_0_COMPLEX_SCHEDULER_TABLE_ADDRESS | (IMAGE_0_SCHEDULING_QUEUE_AGING_VECTOR_ADDRESS << 16);
    local_regs[IMAGE_0_DS_TM_UPDATE_FIFO_THREAD_NUMBER][reg_id[11]] = IMAGE_0_UPDATE_FIFO_TABLE_ADDRESS;
    local_regs[IMAGE_0_DS_TM_UPDATE_FIFO_THREAD_NUMBER][reg_id[10]] = BB_ID_QM_RNR_GRID | (IMAGE_0_BBH_QUEUE_TABLE_ADDRESS << 16);
    local_regs[IMAGE_0_DS_TM_UPDATE_FIFO_THREAD_NUMBER][reg_id[9]]  = IMAGE_0_SCHEDULING_QUEUE_TABLE_ADDRESS | (IMAGE_0_BASIC_SCHEDULER_TABLE_ADDRESS << 16);


    /* FLUSH TASK */
    local_regs[IMAGE_0_DS_TM_FLUSH_THREAD_NUMBER][reg_id[16]] = ADDRESS_OF(image_0, flush_task_1st_wakeup_request) << 16;
    local_regs[IMAGE_0_DS_TM_FLUSH_THREAD_NUMBER][reg_id[12]] = IMAGE_0_PD_FIFO_TABLE_ADDRESS;
    local_regs[IMAGE_0_DS_TM_FLUSH_THREAD_NUMBER][reg_id[11]] = IMAGE_0_SCHEDULING_QUEUE_AGING_VECTOR_ADDRESS;
    local_regs[IMAGE_0_DS_TM_FLUSH_THREAD_NUMBER][reg_id[10]] = (BB_ID_DISPATCHER_REORDER + (DISP_REOR_VIQ_DS_TM_FLUSH << 6));
    local_regs[IMAGE_0_DS_TM_FLUSH_THREAD_NUMBER][reg_id[9]] = IMAGE_0_SCHEDULING_AGGREGATION_CONTEXT_VECTOR_ADDRESS;
    local_regs[IMAGE_0_DS_TM_FLUSH_THREAD_NUMBER][reg_id[8]] = xrdp_virt2phys(&RU_BLK(QM), 0) + RU_REG_OFFSET(QM, GLOBAL_CFG_AGGREGATION_CONTEXT_VALID) +
        ((QM_QUEUE_DS_START - (QM_QUEUE_DS_START % 32)) / 8);

    /* SCHEDULING LAN 0 */
    local_regs[IMAGE_0_DS_TM_LAN0_THREAD_NUMBER][reg_id[16]] = ADDRESS_OF(image_0, tx_task_1st_wakeup_request) << 16;
    local_regs[IMAGE_0_DS_TM_LAN0_THREAD_NUMBER][reg_id[12]] = (IMAGE_0_BBH_TX_EGRESS_COUNTER_TABLE_ADDRESS << 16);
    local_regs[IMAGE_0_DS_TM_LAN0_THREAD_NUMBER][reg_id[11]] = BB_ID_TX_XLMAC0_0_10G | (IMAGE_0_COMPLEX_SCHEDULER_TABLE_ADDRESS << 16);
    local_regs[IMAGE_0_DS_TM_LAN0_THREAD_NUMBER][reg_id[10]] = IMAGE_0_PD_FIFO_TABLE_ADDRESS | (IMAGE_0_BASIC_RATE_LIMITER_TABLE_ADDRESS << 16);
    local_regs[IMAGE_0_DS_TM_LAN0_THREAD_NUMBER][reg_id[9]] = (IMAGE_0_SCHEDULING_QUEUE_TABLE_ADDRESS << 16); 
    local_regs[IMAGE_0_DS_TM_LAN0_THREAD_NUMBER][reg_id[8]]  = IMAGE_0_BBH_QUEUE_TABLE_ADDRESS | (IMAGE_0_BASIC_SCHEDULER_TABLE_ADDRESS << 16);

    /* SCHEDULING LAN 1 */
    local_regs[IMAGE_0_DS_TM_LAN1_THREAD_NUMBER][reg_id[16]] = ADDRESS_OF(image_0, tx_task_1st_wakeup_request) << 16;
    local_regs[IMAGE_0_DS_TM_LAN1_THREAD_NUMBER][reg_id[12]] = ((IMAGE_0_BBH_TX_EGRESS_COUNTER_TABLE_ADDRESS + 1*sizeof(RDD_BBH_TX_EGRESS_COUNTER_ENTRY_DTS)) << 16);
    local_regs[IMAGE_0_DS_TM_LAN1_THREAD_NUMBER][reg_id[11]] = BB_ID_TX_XLMAC0_1_2P5G | (IMAGE_0_COMPLEX_SCHEDULER_TABLE_ADDRESS << 16);
    local_regs[IMAGE_0_DS_TM_LAN1_THREAD_NUMBER][reg_id[10]] = IMAGE_0_PD_FIFO_TABLE_ADDRESS | (IMAGE_0_BASIC_RATE_LIMITER_TABLE_ADDRESS << 16);
    local_regs[IMAGE_0_DS_TM_LAN1_THREAD_NUMBER][reg_id[9]]  = (IMAGE_0_SCHEDULING_QUEUE_TABLE_ADDRESS << 16); 
    local_regs[IMAGE_0_DS_TM_LAN1_THREAD_NUMBER][reg_id[8]]  = (IMAGE_0_BBH_QUEUE_TABLE_ADDRESS + sizeof(RDD_BBH_QUEUE_DESCRIPTOR_DTS))
        | (IMAGE_0_BASIC_SCHEDULER_TABLE_ADDRESS << 16);

    /* SCHEDULING LAN 2 */
    local_regs[IMAGE_0_DS_TM_LAN2_THREAD_NUMBER][reg_id[16]] = ADDRESS_OF(image_0, tx_task_1st_wakeup_request) << 16;
    local_regs[IMAGE_0_DS_TM_LAN2_THREAD_NUMBER][reg_id[12]] = ((IMAGE_0_BBH_TX_EGRESS_COUNTER_TABLE_ADDRESS + 2*sizeof(RDD_BBH_TX_EGRESS_COUNTER_ENTRY_DTS)) << 16);
    local_regs[IMAGE_0_DS_TM_LAN2_THREAD_NUMBER][reg_id[11]] = BB_ID_TX_XLMAC0_2_1G | (IMAGE_0_COMPLEX_SCHEDULER_TABLE_ADDRESS << 16);
    local_regs[IMAGE_0_DS_TM_LAN2_THREAD_NUMBER][reg_id[10]] = IMAGE_0_PD_FIFO_TABLE_ADDRESS | (IMAGE_0_BASIC_RATE_LIMITER_TABLE_ADDRESS << 16);
    local_regs[IMAGE_0_DS_TM_LAN2_THREAD_NUMBER][reg_id[9]]  = (IMAGE_0_SCHEDULING_QUEUE_TABLE_ADDRESS << 16); 
    local_regs[IMAGE_0_DS_TM_LAN2_THREAD_NUMBER][reg_id[8]]  = (IMAGE_0_BBH_QUEUE_TABLE_ADDRESS + (2 * sizeof(RDD_BBH_QUEUE_DESCRIPTOR_DTS)))
        | (IMAGE_0_BASIC_SCHEDULER_TABLE_ADDRESS << 16);

    /* SCHEDULING LAN 3 */
    local_regs[IMAGE_0_DS_TM_LAN3_THREAD_NUMBER][reg_id[16]] = ADDRESS_OF(image_0, tx_task_1st_wakeup_request) << 16;
    local_regs[IMAGE_0_DS_TM_LAN3_THREAD_NUMBER][reg_id[12]] = ((IMAGE_0_BBH_TX_EGRESS_COUNTER_TABLE_ADDRESS + 3*sizeof(RDD_BBH_TX_EGRESS_COUNTER_ENTRY_DTS)) << 16);
    local_regs[IMAGE_0_DS_TM_LAN3_THREAD_NUMBER][reg_id[11]] = BB_ID_TX_XLMAC0_3_1G | (IMAGE_0_COMPLEX_SCHEDULER_TABLE_ADDRESS << 16);
    local_regs[IMAGE_0_DS_TM_LAN3_THREAD_NUMBER][reg_id[10]] = IMAGE_0_PD_FIFO_TABLE_ADDRESS | (IMAGE_0_BASIC_RATE_LIMITER_TABLE_ADDRESS << 16);
    local_regs[IMAGE_0_DS_TM_LAN3_THREAD_NUMBER][reg_id[9]]  = (IMAGE_0_SCHEDULING_QUEUE_TABLE_ADDRESS << 16); 
    local_regs[IMAGE_0_DS_TM_LAN3_THREAD_NUMBER][reg_id[8]]  = (IMAGE_0_BBH_QUEUE_TABLE_ADDRESS + (3 * sizeof(RDD_BBH_QUEUE_DESCRIPTOR_DTS)))
        | (IMAGE_0_BASIC_SCHEDULER_TABLE_ADDRESS << 16);

    /* SCHEDULING LAN 4 */
    local_regs[IMAGE_0_DS_TM_LAN4_THREAD_NUMBER][reg_id[16]] = ADDRESS_OF(image_0, tx_task_1st_wakeup_request) << 16;
    local_regs[IMAGE_0_DS_TM_LAN4_THREAD_NUMBER][reg_id[12]] = ((IMAGE_0_BBH_TX_EGRESS_COUNTER_TABLE_ADDRESS + 4*sizeof(RDD_BBH_TX_EGRESS_COUNTER_ENTRY_DTS)) << 16);
    local_regs[IMAGE_0_DS_TM_LAN4_THREAD_NUMBER][reg_id[11]] = BB_ID_TX_XLMAC1_0_RGMII | (IMAGE_0_COMPLEX_SCHEDULER_TABLE_ADDRESS << 16);
    local_regs[IMAGE_0_DS_TM_LAN4_THREAD_NUMBER][reg_id[10]] = IMAGE_0_PD_FIFO_TABLE_ADDRESS | (IMAGE_0_BASIC_RATE_LIMITER_TABLE_ADDRESS << 16);
    local_regs[IMAGE_0_DS_TM_LAN4_THREAD_NUMBER][reg_id[9]]  = (IMAGE_0_SCHEDULING_QUEUE_TABLE_ADDRESS << 16); 
    local_regs[IMAGE_0_DS_TM_LAN4_THREAD_NUMBER][reg_id[8]]  = (IMAGE_0_BBH_QUEUE_TABLE_ADDRESS + (4 * sizeof(RDD_BBH_QUEUE_DESCRIPTOR_DTS)))
        | (IMAGE_0_BASIC_SCHEDULER_TABLE_ADDRESS << 16);

    /* SCHEDULING LAN 5 */
    local_regs[IMAGE_0_DS_TM_LAN5_THREAD_NUMBER][reg_id[16]] = ADDRESS_OF(image_0, tx_task_1st_wakeup_request) << 16;
    local_regs[IMAGE_0_DS_TM_LAN5_THREAD_NUMBER][reg_id[12]] = ((IMAGE_0_BBH_TX_EGRESS_COUNTER_TABLE_ADDRESS + 5*sizeof(RDD_BBH_TX_EGRESS_COUNTER_ENTRY_DTS)) << 16);
    local_regs[IMAGE_0_DS_TM_LAN5_THREAD_NUMBER][reg_id[11]] = BB_ID_TX_XLMAC1_1_RGMII | (IMAGE_0_COMPLEX_SCHEDULER_TABLE_ADDRESS << 16);
    local_regs[IMAGE_0_DS_TM_LAN5_THREAD_NUMBER][reg_id[10]] = IMAGE_0_PD_FIFO_TABLE_ADDRESS | (IMAGE_0_BASIC_RATE_LIMITER_TABLE_ADDRESS << 16);
    local_regs[IMAGE_0_DS_TM_LAN5_THREAD_NUMBER][reg_id[9]]  = (IMAGE_0_SCHEDULING_QUEUE_TABLE_ADDRESS << 16); 
    local_regs[IMAGE_0_DS_TM_LAN5_THREAD_NUMBER][reg_id[8]]  = (IMAGE_0_BBH_QUEUE_TABLE_ADDRESS + (5 * sizeof(RDD_BBH_QUEUE_DESCRIPTOR_DTS)))
        | (IMAGE_0_BASIC_SCHEDULER_TABLE_ADDRESS << 16);

    /* SCHEDULING LAN 6 */
    local_regs[IMAGE_0_DS_TM_LAN6_THREAD_NUMBER][reg_id[16]] = ADDRESS_OF(image_0, tx_task_1st_wakeup_request) << 16;
    local_regs[IMAGE_0_DS_TM_LAN6_THREAD_NUMBER][reg_id[12]] = ((IMAGE_0_BBH_TX_EGRESS_COUNTER_TABLE_ADDRESS + 6*sizeof(RDD_BBH_TX_EGRESS_COUNTER_ENTRY_DTS)) << 16);
    local_regs[IMAGE_0_DS_TM_LAN6_THREAD_NUMBER][reg_id[11]] = BB_ID_TX_XLMAC1_2_RGMII | (IMAGE_0_COMPLEX_SCHEDULER_TABLE_ADDRESS << 16);
    local_regs[IMAGE_0_DS_TM_LAN6_THREAD_NUMBER][reg_id[10]] = IMAGE_0_PD_FIFO_TABLE_ADDRESS | (IMAGE_0_BASIC_RATE_LIMITER_TABLE_ADDRESS << 16);
    local_regs[IMAGE_0_DS_TM_LAN6_THREAD_NUMBER][reg_id[9]]  = (IMAGE_0_SCHEDULING_QUEUE_TABLE_ADDRESS << 16); 
    local_regs[IMAGE_0_DS_TM_LAN6_THREAD_NUMBER][reg_id[8]]  = (IMAGE_0_BBH_QUEUE_TABLE_ADDRESS + (6 * sizeof(RDD_BBH_QUEUE_DESCRIPTOR_DTS)))
        | (IMAGE_0_BASIC_SCHEDULER_TABLE_ADDRESS << 16);

    /* FLUSH TASK: thread 4 */
    local_regs[IMAGE_0_DS_TM_FLUSH_THREAD_NUMBER][reg_id[16]] = ADDRESS_OF(image_0, flush_task_1st_wakeup_request) << 16;
    local_regs[IMAGE_0_DS_TM_FLUSH_THREAD_NUMBER][reg_id[12]] = IMAGE_0_PD_FIFO_TABLE_ADDRESS;
    local_regs[IMAGE_0_DS_TM_FLUSH_THREAD_NUMBER][reg_id[11]] = IMAGE_0_SCHEDULING_QUEUE_AGING_VECTOR_ADDRESS;
    local_regs[IMAGE_0_DS_TM_FLUSH_THREAD_NUMBER][reg_id[10]] = (BB_ID_DISPATCHER_REORDER + (DISP_REOR_VIQ_DS_TM_FLUSH << 6));
    local_regs[IMAGE_0_DS_TM_FLUSH_THREAD_NUMBER][reg_id[9]] = IMAGE_0_SCHEDULING_AGGREGATION_CONTEXT_VECTOR_ADDRESS;
    local_regs[IMAGE_0_DS_TM_FLUSH_THREAD_NUMBER][reg_id[8]] = xrdp_virt2phys(&RU_BLK(QM), 0) + RU_REG_OFFSET(QM, GLOBAL_CFG_AGGREGATION_CONTEXT_VALID) +
        ((QM_QUEUE_DS_START - (QM_QUEUE_DS_START % 32)) / 8);

#ifdef RDP_SIM
    /* copy the local registers initial values to the Context memory */
    MWRITE_BLK_32(sram_context, local_regs, mem_cntxt_byte_num);
#else
    rdp_rnr_write_context(sram_context, local_regs, mem_cntxt_byte_num);
#endif
}

static void image_1_context_set(uint32_t core_index)
{
    static uint32_t local_regs[NUM_OF_MAIN_RUNNER_THREADS][32];
    uint32_t mem_cntxt_byte_num;
    uint32_t *sram_context;
    uint32_t task;

    sram_context = (uint32_t *)RUNNER_CORE_CONTEXT_ADDRESS(core_index);
    mem_cntxt_byte_num = (RU_REG_RAM_CNT(RNR_CNTXT, MEM_ENTRY) + 1) * sizeof(uint32_t);
#ifndef XRDP_EMULATION
    /* read the local registers from the Context memory - maybe it was initialized by the ACE compiler */
    MREAD_BLK_32(local_regs, sram_context, mem_cntxt_byte_num);
#endif

    for (task = IMAGE_1_IMAGE_1_PROCESSING0_THREAD_NUMBER; task <= IMAGE_1_IMAGE_1_PROCESSING7_THREAD_NUMBER; task++) 
    {
        /* PROCESSING  */
        local_regs[task][reg_id[9]] = IMAGE_1_RX_FLOW_TABLE_ADDRESS << 16 | task;
        local_regs[task][reg_id[16]] = ADDRESS_OF(image_1, processing_wakeup_request) << 16 | 
            PACKET_BUFFER_PD_PTR(IMAGE_1_DS_PACKET_BUFFER_ADDRESS, task);
    }

    /* CPU_RX_FIFO_READ */
    local_regs[IMAGE_1_IMAGE_1_CPU_RX_THREAD_NUMBER][reg_id[16]] = ADDRESS_OF(image_1, cpu_rx_wakeup_request) << 16;
    local_regs[IMAGE_1_IMAGE_1_CPU_RX_THREAD_NUMBER][reg_id[14]] = IMAGE_1_CPU_RX_SCRATCHPAD_ADDRESS |
        (IMAGE_1_CPU_RX_PSRAM_GET_NEXT_SCRATCHPAD_ADDRESS << 16);
    local_regs[IMAGE_1_IMAGE_1_CPU_RX_THREAD_NUMBER][reg_id[12]] = IMAGE_1_CPU_RING_DESCRIPTORS_TABLE_ADDRESS;
    local_regs[IMAGE_1_IMAGE_1_CPU_RX_THREAD_NUMBER][reg_id[11]] = IMAGE_1_PD_FIFO_TABLE_ADDRESS | (IMAGE_1_CPU_RECYCLE_SRAM_PD_FIFO_ADDRESS << 16);

    /* CPU_RX_UPDATE_FIFO_READ */
    local_regs[IMAGE_1_IMAGE_1_CPU_RX_UPDATE_FIFO_THREAD_NUMBER][reg_id[16]] = ADDRESS_OF(image_1, cpu_rx_update_fifo_wakeup_request) << 16;
    local_regs[IMAGE_1_IMAGE_1_CPU_RX_UPDATE_FIFO_THREAD_NUMBER][reg_id[12]] = (((IMAGE_1_IMAGE_1_CPU_RX_THREAD_NUMBER) << 4) + 9);
    local_regs[IMAGE_1_IMAGE_1_CPU_RX_UPDATE_FIFO_THREAD_NUMBER][reg_id[11]] = IMAGE_1_UPDATE_FIFO_TABLE_ADDRESS;

    /* CPU_TX_INTERRUPT_COALESCING */
    local_regs[IMAGE_1_IMAGE_1_INTERRUPT_COALESCING_THREAD_NUMBER][reg_id[16]] = ADDRESS_OF(image_1, interrupt_coalescing_1st_wakeup_request) << 16;
    local_regs[IMAGE_1_IMAGE_1_INTERRUPT_COALESCING_THREAD_NUMBER][reg_id[13]] = IMAGE_1_CPU_INTERRUPT_COALESCING_TABLE_ADDRESS;
    local_regs[IMAGE_1_IMAGE_1_INTERRUPT_COALESCING_THREAD_NUMBER][reg_id[12]] = IMAGE_1_CPU_RING_DESCRIPTORS_TABLE_ADDRESS | (IMAGE_1_CPU_RING_INTERRUPT_COUNTER_TABLE_ADDRESS << 16);

    /* CPU_RX_METER_BUDGET_ALLOCATOR: thread 11 */
    local_regs[IMAGE_1_IMAGE_1_CPU_RX_METER_BUDGET_ALLOCATOR_THREAD_NUMBER][reg_id[16]] =
        ADDRESS_OF(image_1, cpu_rx_meter_budget_allocator_1st_wakeup_request) << 16;
    local_regs[IMAGE_1_IMAGE_1_CPU_RX_METER_BUDGET_ALLOCATOR_THREAD_NUMBER][reg_id[11]] = CPU_RX_METER_TIMER_PERIOD;

    /* CPU_RECYCLE: thread 12 */
    local_regs[IMAGE_1_IMAGE_1_CPU_RECYCLE_THREAD_NUMBER][reg_id[16]] = ADDRESS_OF(image_1, cpu_recycle_wakeup_request) << 16;
    local_regs[IMAGE_1_IMAGE_1_CPU_RECYCLE_THREAD_NUMBER][reg_id[8]] = IMAGE_1_CPU_RECYCLE_SRAM_PD_FIFO_ADDRESS; 

#ifdef RDP_SIM
    /* copy the local registers initial values to the Context memory */
    MWRITE_BLK_32(sram_context, local_regs, mem_cntxt_byte_num);
#else
    rdp_rnr_write_context(sram_context, local_regs, mem_cntxt_byte_num);
#endif
}

static void image_2_context_set(uint32_t core_index)
{
    static uint32_t local_regs[NUM_OF_MAIN_RUNNER_THREADS][32];
    uint32_t mem_cntxt_byte_num;
    uint32_t *sram_context;
    uint32_t task;

    sram_context = (uint32_t *)RUNNER_CORE_CONTEXT_ADDRESS(core_index);
    mem_cntxt_byte_num = (RU_REG_RAM_CNT(RNR_CNTXT, MEM_ENTRY) + 1) * sizeof(uint32_t);
#ifndef XRDP_EMULATION
    /* read the local registers from the Context memory - maybe it was initialized by the ACE compiler */
    MREAD_BLK_32(local_regs, sram_context, mem_cntxt_byte_num);
#endif

    for (task = IMAGE_2_IMAGE_2_PROCESSING0_THREAD_NUMBER; task <= IMAGE_2_IMAGE_2_PROCESSING7_THREAD_NUMBER; task++) 
    {
        /* PROCESSING  */
        local_regs[task][reg_id[9]] = IMAGE_2_RX_FLOW_TABLE_ADDRESS << 16 | task;
        local_regs[task][reg_id[16]] = ADDRESS_OF(image_2, processing_wakeup_request) << 16 | 
            PACKET_BUFFER_PD_PTR(IMAGE_2_DS_PACKET_BUFFER_ADDRESS, task);
    }

    /* CPU_TX_UPDATE_FIFO_EGRESS */
    local_regs[IMAGE_2_IMAGE_2_CPU_TX_UPDATE_FIFO_EGRESS_THREAD_NUMBER][reg_id[16]] = ADDRESS_OF(image_2, cpu_tx_update_fifo_wakeup_request) << 16;
    local_regs[IMAGE_2_IMAGE_2_CPU_TX_UPDATE_FIFO_EGRESS_THREAD_NUMBER][reg_id[12]] = (((IMAGE_2_IMAGE_2_CPU_TX_EGRESS_THREAD_NUMBER) << 4) + 9);
    local_regs[IMAGE_2_IMAGE_2_CPU_TX_UPDATE_FIFO_EGRESS_THREAD_NUMBER][reg_id[11]] = IMAGE_2_CPU_TX_EGRESS_UPDATE_FIFO_TABLE_ADDRESS;
    local_regs[IMAGE_2_IMAGE_2_CPU_TX_UPDATE_FIFO_EGRESS_THREAD_NUMBER][reg_id[10]] = BB_ID_QM_RNR_GRID;
    local_regs[IMAGE_2_IMAGE_2_CPU_TX_UPDATE_FIFO_EGRESS_THREAD_NUMBER][reg_id[9]] = QM_QUEUE_CPU_TX_EGRESS;

    /* CPU_TX_UPDATE_FIFO_INGRESS */
    local_regs[IMAGE_2_IMAGE_2_CPU_TX_UPDATE_FIFO_INGRESS_THREAD_NUMBER][reg_id[16]] = ADDRESS_OF(image_2, cpu_tx_update_fifo_wakeup_request) << 16;
    local_regs[IMAGE_2_IMAGE_2_CPU_TX_UPDATE_FIFO_INGRESS_THREAD_NUMBER][reg_id[12]] = (((IMAGE_2_IMAGE_2_CPU_TX_INGRESS_THREAD_NUMBER) << 4) + 9);
    local_regs[IMAGE_2_IMAGE_2_CPU_TX_UPDATE_FIFO_INGRESS_THREAD_NUMBER][reg_id[11]] = IMAGE_2_CPU_TX_INGRESS_UPDATE_FIFO_TABLE_ADDRESS;
    local_regs[IMAGE_2_IMAGE_2_CPU_TX_UPDATE_FIFO_INGRESS_THREAD_NUMBER][reg_id[10]] = BB_ID_QM_RNR_GRID;
    local_regs[IMAGE_2_IMAGE_2_CPU_TX_UPDATE_FIFO_INGRESS_THREAD_NUMBER][reg_id[9]] = QM_QUEUE_CPU_TX_INGRESS;

    /* CPU_TX_RECYCLE */
    local_regs[IMAGE_2_IMAGE_2_CPU_RECYCLE_THREAD_NUMBER][reg_id[16]] = ADDRESS_OF(image_2, cpu_recycle_wakeup_request) << 16;
    local_regs[IMAGE_2_IMAGE_2_CPU_RECYCLE_THREAD_NUMBER][reg_id[8]] = IMAGE_2_CPU_RECYCLE_SRAM_PD_FIFO_ADDRESS; 

    /* CPU_TX_EGRESS: thread 11 */
    local_regs[IMAGE_2_IMAGE_2_CPU_TX_EGRESS_THREAD_NUMBER][reg_id[16]] = ADDRESS_OF(image_2, cpu_tx_wakeup_request) << 16;
    local_regs[IMAGE_2_IMAGE_2_CPU_TX_EGRESS_THREAD_NUMBER][reg_id[10]] = IMAGE_2_IMAGE_2_CPU_TX_EGRESS_THREAD_NUMBER | (IMAGE_2_CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE_ADDRESS << 16);
    local_regs[IMAGE_2_IMAGE_2_CPU_TX_EGRESS_THREAD_NUMBER][reg_id[9]] = QM_QUEUE_CPU_TX_EGRESS | ((BB_ID_DISPATCHER_REORDER + (DISP_REOR_VIQ_CPU_TX_EGRESS << 6)) << 16);
    local_regs[IMAGE_2_IMAGE_2_CPU_TX_EGRESS_THREAD_NUMBER][reg_id[8]] = IMAGE_2_CPU_TX_EGRESS_PD_FIFO_TABLE_ADDRESS;
    
    /* CPU_TX_INGRESS: thread 12 */
    local_regs[IMAGE_2_IMAGE_2_CPU_TX_INGRESS_THREAD_NUMBER][reg_id[16]] = ADDRESS_OF(image_2, cpu_tx_wakeup_request) << 16;
    local_regs[IMAGE_2_IMAGE_2_CPU_TX_INGRESS_THREAD_NUMBER][reg_id[10]] = IMAGE_2_IMAGE_2_CPU_TX_INGRESS_THREAD_NUMBER | (IMAGE_2_CPU_TX_INGRESS_DISPATCHER_CREDIT_TABLE_ADDRESS << 16);
    local_regs[IMAGE_2_IMAGE_2_CPU_TX_INGRESS_THREAD_NUMBER][reg_id[9]] = QM_QUEUE_CPU_TX_INGRESS | (((BB_ID_DISPATCHER_REORDER + (DISP_REOR_VIQ_CPU_TX_FORWARD << 6))) << 16);
    local_regs[IMAGE_2_IMAGE_2_CPU_TX_INGRESS_THREAD_NUMBER][reg_id[8]] = IMAGE_2_CPU_TX_INGRESS_PD_FIFO_TABLE_ADDRESS;

    /* CPU_TX_INTERRUPT_COALESCING */
    local_regs[IMAGE_2_IMAGE_2_INTERRUPT_COALESCING_THREAD_NUMBER][reg_id[16]] = ADDRESS_OF(image_2, interrupt_coalescing_1st_wakeup_request) << 16;
    local_regs[IMAGE_2_IMAGE_2_INTERRUPT_COALESCING_THREAD_NUMBER][reg_id[13]] = IMAGE_2_CPU_RECYCLE_INTERRUPT_COALESCING_TABLE_ADDRESS;
    local_regs[IMAGE_2_IMAGE_2_INTERRUPT_COALESCING_THREAD_NUMBER][reg_id[12]] = IMAGE_2_CPU_RECYCLE_RING_DESCRIPTOR_TABLE_ADDRESS;

#ifdef RDP_SIM
    /* copy the local registers initial values to the Context memory */
    MWRITE_BLK_32(sram_context, local_regs, mem_cntxt_byte_num);
#else
    rdp_rnr_write_context(sram_context, local_regs, mem_cntxt_byte_num);
#endif
}

static void image_3_context_set(uint32_t core_index)
{
    static uint32_t local_regs[NUM_OF_MAIN_RUNNER_THREADS][32];
    uint32_t mem_cntxt_byte_num;
    uint32_t *sram_context;

    sram_context = (uint32_t *)DEVICE_ADDRESS(RU_BLK(RNR_CNTXT).addr[core_index] + RU_REG_OFFSET(RNR_CNTXT, MEM_ENTRY));
    mem_cntxt_byte_num = (RU_REG_RAM_CNT(RNR_CNTXT, MEM_ENTRY) + 1) * sizeof(uint32_t);
#ifndef XRDP_EMULATION
    /* read the local registers from the Context memory - maybe it was initialized by the ACE compiler */
    MREAD_BLK_32(local_regs, sram_context, mem_cntxt_byte_num);
#endif

    /* Budget allocation */
    local_regs[IMAGE_3_US_TM_BUDGET_ALLOCATION_THREAD_NUMBER][reg_id[16]] = ADDRESS_OF(image_3, budget_allocator_1st_wakeup_request) << 16;
    local_regs[IMAGE_3_US_TM_BUDGET_ALLOCATION_THREAD_NUMBER][reg_id[13]] = (RDD_BASIC_RATE_LIMITER_TABLE_SIZE / 4) * sizeof(RDD_BASIC_RATE_LIMITER_DESCRIPTOR_DTS);
    local_regs[IMAGE_3_US_TM_BUDGET_ALLOCATION_THREAD_NUMBER][reg_id[12]] = IMAGE_3_SCHEDULING_QUEUE_TABLE_ADDRESS | (IMAGE_3_COMPLEX_SCHEDULER_TABLE_ADDRESS << 16);
    local_regs[IMAGE_3_US_TM_BUDGET_ALLOCATION_THREAD_NUMBER][reg_id[10]] = IMAGE_3_RATE_LIMITER_VALID_TABLE_ADDRESS;
    local_regs[IMAGE_3_US_TM_BUDGET_ALLOCATION_THREAD_NUMBER][reg_id[9]]  = IMAGE_3_BASIC_RATE_LIMITER_TABLE_ADDRESS | (IMAGE_3_BASIC_SCHEDULER_TABLE_ADDRESS << 16);

    /* Overall budget allocation (for ovlr rl) */
    local_regs[IMAGE_3_US_TM_OVL_BUDGET_ALLOCATION_THREAD_NUMBER][reg_id[8]] = IMAGE_3_BBH_QUEUE_TABLE_ADDRESS | (IMAGE_3_BASIC_SCHEDULER_TABLE_ADDRESS << 16);
    local_regs[IMAGE_3_US_TM_OVL_BUDGET_ALLOCATION_THREAD_NUMBER][reg_id[11]] = (IMAGE_3_COMPLEX_SCHEDULER_TABLE_ADDRESS << 16);
    local_regs[IMAGE_3_US_TM_OVL_BUDGET_ALLOCATION_THREAD_NUMBER][reg_id[16]] = ADDRESS_OF(image_3, ovl_budget_allocator_1st_wakeup_request) << 16;

    /* UPDATE_FIFO_READ */
    local_regs[IMAGE_3_US_TM_UPDATE_FIFO_THREAD_NUMBER][reg_id[16]] = ADDRESS_OF(image_3, update_fifo_read_1st_wakeup_request) << 16;
    local_regs[IMAGE_3_US_TM_UPDATE_FIFO_THREAD_NUMBER][reg_id[14]] = IMAGE_3_COMPLEX_SCHEDULER_TABLE_ADDRESS | (IMAGE_3_SCHEDULING_QUEUE_AGING_VECTOR_ADDRESS << 16);
    local_regs[IMAGE_3_US_TM_UPDATE_FIFO_THREAD_NUMBER][reg_id[12]] = (IMAGE_3_PD_FIFO_TABLE_ADDRESS << 16);
    local_regs[IMAGE_3_US_TM_UPDATE_FIFO_THREAD_NUMBER][reg_id[11]] = IMAGE_3_UPDATE_FIFO_TABLE_ADDRESS;
    local_regs[IMAGE_3_US_TM_UPDATE_FIFO_THREAD_NUMBER][reg_id[10]] = BB_ID_QM_RNR_GRID | (IMAGE_3_BBH_QUEUE_TABLE_ADDRESS << 16);
    local_regs[IMAGE_3_US_TM_UPDATE_FIFO_THREAD_NUMBER][reg_id[9]]  = IMAGE_3_SCHEDULING_QUEUE_TABLE_ADDRESS | (IMAGE_3_BASIC_SCHEDULER_TABLE_ADDRESS << 16);

    /* FLUSH TASK */
    local_regs[IMAGE_3_US_TM_FLUSH_THREAD_NUMBER][reg_id[16]] = ADDRESS_OF(image_3, flush_task_1st_wakeup_request) << 16;
    local_regs[IMAGE_3_US_TM_FLUSH_THREAD_NUMBER][reg_id[12]] = IMAGE_3_PD_FIFO_TABLE_ADDRESS;
    local_regs[IMAGE_3_US_TM_FLUSH_THREAD_NUMBER][reg_id[11]] = IMAGE_3_SCHEDULING_QUEUE_AGING_VECTOR_ADDRESS;
    local_regs[IMAGE_3_US_TM_FLUSH_THREAD_NUMBER][reg_id[10]] = (BB_ID_DISPATCHER_REORDER + (DISP_REOR_VIQ_US_TM_FLUSH << 6));
    local_regs[IMAGE_3_US_TM_FLUSH_THREAD_NUMBER][reg_id[9]] = IMAGE_3_SCHEDULING_AGGREGATION_CONTEXT_VECTOR_ADDRESS;
    local_regs[IMAGE_3_US_TM_FLUSH_THREAD_NUMBER][reg_id[8]] = xrdp_virt2phys(&RU_BLK(QM), 0) + RU_REG_OFFSET(QM, GLOBAL_CFG_AGGREGATION_CONTEXT_VALID) +
        (QM_QUEUE_US_START / 8);

    /* SCHEDULING WAN */
    local_regs[IMAGE_3_US_TM_WAN_THREAD_NUMBER][reg_id[16]] = ADDRESS_OF(image_3, tx_task_1st_wakeup_request) << 16;
    local_regs[IMAGE_3_US_TM_WAN_THREAD_NUMBER][reg_id[12]] = (IMAGE_3_BBH_TX_EGRESS_COUNTER_TABLE_ADDRESS << 16);
    local_regs[IMAGE_3_US_TM_WAN_THREAD_NUMBER][reg_id[11]] = BB_ID_TX_PON_ETH_PD | (IMAGE_3_COMPLEX_SCHEDULER_TABLE_ADDRESS << 16);
    local_regs[IMAGE_3_US_TM_WAN_THREAD_NUMBER][reg_id[10]] = IMAGE_3_PD_FIFO_TABLE_ADDRESS | (IMAGE_3_BASIC_RATE_LIMITER_TABLE_ADDRESS << 16);
    local_regs[IMAGE_3_US_TM_WAN_THREAD_NUMBER][reg_id[9]]  = (BB_ID_DISPATCHER_REORDER + (DISP_REOR_VIQ_EPON_TM << 6)) | (IMAGE_3_SCHEDULING_QUEUE_TABLE_ADDRESS << 16); 
    local_regs[IMAGE_3_US_TM_WAN_THREAD_NUMBER][reg_id[8]]  = IMAGE_3_BBH_QUEUE_TABLE_ADDRESS | (IMAGE_3_BASIC_SCHEDULER_TABLE_ADDRESS << 16);

#if 0 /* TBD. BCM63158 */
    /* REPORTING  */
    local_regs[IMAGE_3_US_TM_REPORTING_THREAD_NUMBER][reg_id[17]] = XGPON_MAC_REPORT_ADDRESS;
    local_regs[IMAGE_3_US_TM_REPORTING_THREAD_NUMBER][reg_id[16]] = ADDRESS_OF(image_3, ghost_reporting_1st_wakeup_request) << 16;
    local_regs[IMAGE_3_US_TM_REPORTING_THREAD_NUMBER][reg_id[15]] = xrdp_virt2phys(&RU_BLK(QM), 0) + RU_REG_OFFSET(QM, TOTAL_VALID_COUNTER_COUNTER);
    local_regs[IMAGE_3_US_TM_REPORTING_THREAD_NUMBER][reg_id[14]] = BB_ID_TX_PON_ETH_STAT;
    local_regs[IMAGE_3_US_TM_REPORTING_THREAD_NUMBER][reg_id[13]] = IMAGE_3_REPORTING_QUEUE_DESCRIPTOR_TABLE_ADDRESS;
    local_regs[IMAGE_3_US_TM_REPORTING_THREAD_NUMBER][reg_id[12]] = IMAGE_3_REPORTING_QUEUE_ACCUMULATED_TABLE_ADDRESS;
    local_regs[IMAGE_3_US_TM_REPORTING_THREAD_NUMBER][reg_id[11]] = IMAGE_3_GHOST_REPORTING_QUEUE_STATUS_BIT_VECTOR_TABLE_ADDRESS;
    local_regs[IMAGE_3_US_TM_REPORTING_THREAD_NUMBER][reg_id[10]] = xrdp_virt2phys(&RU_BLK(QM), 0) + RU_REG_OFFSET(QM, EPON_RPT_CNT_COUNTER);
    local_regs[IMAGE_3_US_TM_REPORTING_THREAD_NUMBER][reg_id[9]] = xrdp_virt2phys(&RU_BLK(QM), 0) + RU_REG_OFFSET(QM, EPON_RPT_CNT_QUEUE_STATUS);
    local_regs[IMAGE_3_US_TM_REPORTING_THREAD_NUMBER][reg_id[8]] = IMAGE_3_REPORTING_COUNTER_TABLE_ADDRESS;
#endif /* TBD. BCM63158 */

#ifdef RDP_SIM
   /* copy the local registers initial values to the Context memory */
   MWRITE_BLK_32(sram_context, local_regs, mem_cntxt_byte_num);
#else
   rdp_rnr_write_context(sram_context, local_regs, mem_cntxt_byte_num);
#endif
}

static void image_4_context_set(uint32_t core_index)
{
    static uint32_t local_regs[NUM_OF_MAIN_RUNNER_THREADS][32];
    uint32_t mem_cntxt_byte_num;
    uint32_t *sram_context;
    uint32_t task;

    sram_context = (uint32_t *)DEVICE_ADDRESS(RU_BLK(RNR_CNTXT).addr[core_index] + RU_REG_OFFSET(RNR_CNTXT, MEM_ENTRY));
    mem_cntxt_byte_num = (RU_REG_RAM_CNT(RNR_CNTXT, MEM_ENTRY) + 1) * sizeof(uint32_t);
#ifndef XRDP_EMULATION
    /* read the local registers from the Context memory - maybe it was initialized by the ACE compiler */
    MREAD_BLK_32(local_regs, sram_context, mem_cntxt_byte_num);
#endif
    for (task = IMAGE_4_PROCESSING_0_THREAD_NUMBER; task <= IMAGE_4_PROCESSING_7_THREAD_NUMBER; task++)
    {
        /* PROCESSING  */
        local_regs[task][reg_id[9]] = IMAGE_4_RX_FLOW_TABLE_ADDRESS << 16 | task;
        local_regs[task][reg_id[16]] = ADDRESS_OF(image_4, processing_wakeup_request) << 16 | 
            PACKET_BUFFER_PD_PTR(IMAGE_4_DS_PACKET_BUFFER_ADDRESS, task);
    }
#ifdef RDP_SIM
   /* copy the local registers initial values to the Context memory */
   MWRITE_BLK_32(sram_context, local_regs, mem_cntxt_byte_num);
#else
   rdp_rnr_write_context(sram_context, local_regs, mem_cntxt_byte_num);
#endif
}

static void rdd_local_registers_init(void)
{
    uint32_t core_index;

    for (core_index = 0; core_index < GROUPED_EN_SEGMENTS_NUM; core_index++)
    {
        switch (rdp_core_to_image_map[core_index])
        {
        case image_0_runner_image:
            image_0_context_set(core_index);
            break;

        case image_1_runner_image:
            image_1_context_set(core_index);
            break;

        case image_2_runner_image:
            image_2_context_set(core_index);
            break;

        case image_3_runner_image:
            image_3_context_set(core_index);
            break;

        case image_4_runner_image:
            image_4_context_set(core_index);
            break;

        default: 
            bdmf_trace("ERROR driver %s:%u| unsupported Runner image = %d\n", __FILE__, __LINE__, rdp_core_to_image_map[core_index]);
            break;
        }
    }
}


static int rdd_cpu_proj_init(void)
{
#ifndef _CFE_
    uint8_t def_idx = (uint8_t)BDMF_INDEX_UNASSIGNED;
#else
    uint8_t def_idx = 0;
#endif
    int rc = 0;

    rdd_cpu_tc_to_rqx_init(def_idx);
    rdd_cpu_vport_cpu_obj_init(def_idx);
    rdd_cpu_rx_meters_init();
#ifndef _CFE_
    rc = ag_drv_rnr_regs_cfg_cpu_wakeup_set(get_runner_idx(cpu_rx_runner_image),
        CPU_RX_METER_BUDGET_ALLOCATOR_THREAD_NUMBER);
#endif
    return rc;
}

int rdd_data_structures_init(rdd_init_params_t *init_params, RDD_HW_IPTV_CONFIGURATION_DTS *iptv_hw_config)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    rdd_global_registers_init();
    rdd_local_registers_init();

    rdd_cpu_if_init();

    rdd_fpm_pool_number_mapping_cfg(iptv_hw_config->fpm_base_token_size);

    if (!init_params->is_basic)
    {
        rdd_iptv_processing_cfg(iptv_hw_config);
        rdd_qos_mapper_init();
    }

    /* init first queue mapping */
    rdd_ag_us_tm_first_queue_mapping_set(QM_QUEUE_US_START);
    rdd_ag_ds_tm_first_queue_mapping_set(QM_QUEUE_DS_START);

    /* init bbh-queue */
    rdd_bbh_queue_init();

#ifdef BCM6858_A0
    /* WA for A0 - enable queue in fw */
    rdd_set_queue_enable(QM_QUEUE_CPU_RX, 1);
#endif

    /* start flush task */
    rc = rc ? rc : rdd_scheduling_flush_timer_set();

    /* start budget allocation task */
    rc = rc ? rc : rdd_ds_budget_allocation_timer_set();

    rdd_proj_init(init_params);
    rdd_actions_proj_init();
    rdd_tm_actions_proj_init();

    rc = rc ? rc : rdd_cpu_proj_init();

#ifdef USE_BDMF_SHELL
    /* register shell commands */
    rc = rc ? : rdd_make_shell_commands();
#endif
    return rc;
}

void rdd_acb_port_init()
{
};

void rdd_acb_port_mapping(uint8_t *acb_port_data)
{
    uint8_t i;
    RDD_ACB_PORT_MAPPING_TABLE_DTS *acb_mapping_table_ptr = RDD_ACB_PORT_MAPPING_TABLE_PTR(0);
 
    /* Assuming DS scheduling is done by runner 0 */
    for (i = 0; i < 64; i++)
        MWRITE_8((uint8_t *) (acb_mapping_table_ptr+i), acb_port_data[i]);
}

/*  Classification chain: Flow based
 *
 *   Module list
 *   -----------
 *  1. Ingress Filter
 *  2. Ingress flow classifier
 *  3. ACL
 *  4. QoS
 *  5. tunnel parsing (just for DS)
 *  6. natc lookup
 *  7. IPTV
 *
 */
static rdd_tcam_table_parm_t tcam_ic_acl_params =
{
    .module_id = TCAM_IC_MODULE_ACL_US,
    .scratch_offset = offsetof(RDD_PACKET_BUFFER_DTS, scratch) + TCAM_IC_SCRATCH_KEY_OFFSET,
};

static rdd_module_t tcam_ic_acl_module =
{
    .context_offset = offsetof(RDD_PACKET_BUFFER_DTS, classification_contexts_list), /* runs over ic_flow */
    .res_offset = (offsetof(RDD_PACKET_BUFFER_DTS, classification_results) +
        (CLASSIFICATION_RESULT_INDEX_TCAM_IC_ACL * 4)),
    .cfg_ptr = RDD_TCAM_IC_CFG_TABLE_ADDRESS_ARR,   /* Instance offset will be added at init time */
    .init = rdd_tcam_module_init,
    .params = &tcam_ic_acl_params
};

static rdd_tcam_table_parm_t tcam_ic_flow_us_params =
{
    .module_id = TCAM_IC_MODULE_FLOW_US,
    .scratch_offset = offsetof(RDD_PACKET_BUFFER_DTS, scratch) + TCAM_IC_SCRATCH_KEY_OFFSET,
};

static rdd_module_t tcam_ic_flow_us_module =
{
    .context_offset = offsetof(RDD_PACKET_BUFFER_DTS, classification_contexts_list),
    .res_offset = (offsetof(RDD_PACKET_BUFFER_DTS, classification_results) +
        (CLASSIFICATION_RESULT_INDEX_TCAM_IC_FLOW * 4)),
    .cfg_ptr = RDD_TCAM_IC_CFG_TABLE_ADDRESS_ARR,   /* Instance offset will be added at init time */
    .init = rdd_tcam_module_init,
    .params = &tcam_ic_flow_us_params
};

static rdd_tcam_table_parm_t tcam_ic_flow_ds_params =
{
    .module_id = TCAM_IC_MODULE_FLOW_DS,
    .scratch_offset = offsetof(RDD_PACKET_BUFFER_DTS, scratch) + TCAM_IC_SCRATCH_KEY_OFFSET,
};

static rdd_module_t tcam_ic_flow_ds_module =
{
    .context_offset = offsetof(RDD_PACKET_BUFFER_DTS, classification_contexts_list),
    .res_offset = (offsetof(RDD_PACKET_BUFFER_DTS, classification_results) +
        (CLASSIFICATION_RESULT_INDEX_TCAM_IC_FLOW * 4)),
    .cfg_ptr = RDD_TCAM_IC_CFG_TABLE_ADDRESS_ARR,   /* Instance offset will be added at init time */
    .init = rdd_tcam_module_init,
    .params = &tcam_ic_flow_ds_params
};

static rdd_tcam_table_parm_t tcam_ic_qos_us_params =
{
    .module_id = TCAM_IC_MODULE_QOS_US,
    .scratch_offset = offsetof(RDD_PACKET_BUFFER_DTS, scratch) + TCAM_IC_SCRATCH_KEY_OFFSET,
};

static rdd_module_t tcam_ic_qos_us_module =
{
    .context_offset = offsetof(RDD_PACKET_BUFFER_DTS, classification_contexts_list), /* runs over ic_flow */
    .res_offset = (offsetof(RDD_PACKET_BUFFER_DTS, classification_results) +
        (CLASSIFICATION_RESULT_INDEX_TCAM_IC_QOS * 4)),
    .cfg_ptr = RDD_TCAM_IC_CFG_TABLE_ADDRESS_ARR,  /* Instance offset will be added at init time */
    .init = rdd_tcam_module_init,
    .params = &tcam_ic_qos_us_params
};

static rdd_tcam_table_parm_t tcam_ic_qos_ds_params =
{
    .module_id = TCAM_IC_MODULE_QOS_DS,
    .scratch_offset = offsetof(RDD_PACKET_BUFFER_DTS, scratch) + TCAM_IC_SCRATCH_KEY_OFFSET,
};

static rdd_module_t tcam_ic_qos_ds_module =
{
    .context_offset = offsetof(RDD_PACKET_BUFFER_DTS, classification_contexts_list), /* runs over ic_flow */
    .res_offset = (offsetof(RDD_PACKET_BUFFER_DTS, classification_results) +
        (CLASSIFICATION_RESULT_INDEX_TCAM_IC_QOS * 4)),
    .cfg_ptr = RDD_TCAM_IC_CFG_TABLE_ADDRESS_ARR,  /* Instance offset will be added at init time */
    .init = rdd_tcam_module_init,
    .params = &tcam_ic_qos_ds_params
};

static rdd_module_t ingress_filter_module =
{
    .res_offset = (offsetof(RDD_PACKET_BUFFER_DTS, classification_results) +
        (CLASSIFICATION_RESULT_INDEX_INGRESS_FILTERS * 4)),
    .cfg_ptr = RDD_INGRESS_FILTER_CFG_ADDRESS_ARR, 
    .init = rdd_ingress_filter_module_init,
    .params = NULL
};

/* IPTV */
static iptv_params_t iptv_params =
{
    .key_offset = offsetof(RDD_PACKET_BUFFER_DTS, scratch),
    .hash_tbl_idx = 1
};

rdd_module_t iptv_module =
{
    .init = rdd_iptv_module_init,
    .context_offset = offsetof(RDD_PACKET_BUFFER_DTS, classification_contexts_list),
    .res_offset = (offsetof(RDD_PACKET_BUFFER_DTS, classification_results) +
        (CLASSIFICATION_RESULT_INDEX_IPTV * 4)),
    .cfg_ptr = RDD_IPTV_CFG_TABLE_ADDRESS_ARR,
    .params = (void *)&iptv_params
};

/* IP CLASS */
static natc_params_t natc_params =
{
    .connection_key_offset = offsetof(RDD_PACKET_BUFFER_DTS, scratch),
};

rdd_module_t ip_flow =
{
    .init = rdd_nat_cache_init,
    /* NTAC returns 60B (first 4B are control) */
    /* For 8B alignment, 4B are added */
    /* LD_CONTEXT macro adds 8B (control) */
    .context_offset = offsetof(RDD_PACKET_BUFFER_DTS, classification_contexts_list) +
        offsetof(RDD_FLOW_BASED_CONTEXT_DTS, flow_cache),
    .res_offset = (offsetof(RDD_PACKET_BUFFER_DTS, classification_results) +
        (CLASSIFICATION_RESULT_INDEX_NAT_CACHE * 4)),
    .cfg_ptr = RDD_NAT_CACHE_CFG_ADDRESS_ARR,
    .params = (void *)&natc_params
};

void rdd_proj_init(rdd_init_params_t *init_params)
{
    /* Classification modules initialization */
    _rdd_module_init(&ip_flow);
    if (init_params->is_basic)
        return;

    _rdd_module_init(&tcam_ic_acl_module);
    _rdd_module_init(&tcam_ic_flow_us_module);
    _rdd_module_init(&tcam_ic_qos_us_module);
    _rdd_module_init(&tcam_ic_flow_ds_module);
    _rdd_module_init(&tcam_ic_qos_ds_module);
    _rdd_module_init(&ingress_filter_module);
    _rdd_module_init(&iptv_module);
}

void rdd_tm_actions_proj_init(void)
{
    uint32_t action_index;
    RDD_BYTES_2_DTS *tm_action_ptr;
    uint16_t ds_actions_arr[] = {
        [0]  = ADDRESS_OF(image_0, basic_scheduler_update_dwrr),
        [1]  = ADDRESS_OF(image_0, complex_scheduler_update_dwrr_queues),
        [2]  = ADDRESS_OF(image_0, complex_scheduler_update_dwrr_basic_schedulers),
        [3]  = ADDRESS_OF(image_0, basic_rate_limiter_queue_with_bs),
        [4]  = ADDRESS_OF(image_0, basic_rate_limiter_queue_with_cs_bs),
        [5]  = ADDRESS_OF(image_0, complex_rate_limiter_queue_sir),
        [6]  = ADDRESS_OF(image_0, complex_rate_limiter_queue_pir),
        [7]  = ADDRESS_OF(image_0, basic_rate_limiter_basic_scheduler_no_cs),
        [8]  = ADDRESS_OF(image_0, complex_rate_limiter_basic_scheduler_sir),
        [9]  = ADDRESS_OF(image_0, complex_rate_limiter_basic_scheduler_pir),
        [10] = ADDRESS_OF(image_0, basic_rate_limiter_complex_scheduler),
        [11 ... 15] = ADDRESS_OF(image_0, scheduling_action_not_valid),
        [16] = ADDRESS_OF(image_0, scheduling_update_status)
    };
    uint16_t us_actions_arr[] = {
        [0]  = ADDRESS_OF(image_3, basic_scheduler_update_dwrr),
        [1]  = ADDRESS_OF(image_3, complex_scheduler_update_dwrr_queues),
        [2]  = ADDRESS_OF(image_3, complex_scheduler_update_dwrr_basic_schedulers),
        [3]  = ADDRESS_OF(image_3, basic_rate_limiter_queue_with_bs),
        [4]  = ADDRESS_OF(image_3, basic_rate_limiter_queue_with_cs_bs),
        [5]  = ADDRESS_OF(image_3, complex_rate_limiter_queue_sir),
        [6]  = ADDRESS_OF(image_3, complex_rate_limiter_queue_pir),
        [7]  = ADDRESS_OF(image_3, basic_rate_limiter_basic_scheduler_no_cs),
        [8]  = ADDRESS_OF(image_3, complex_rate_limiter_basic_scheduler_sir),
        [9]  = ADDRESS_OF(image_3, complex_rate_limiter_basic_scheduler_pir),
        [10] = ADDRESS_OF(image_3, basic_rate_limiter_complex_scheduler),
        [11] = ADDRESS_OF(image_3, ovl_rate_limiter),
        [12 ... 15] = ADDRESS_OF(image_3, scheduling_action_not_valid),
        [16] = ADDRESS_OF(image_3, scheduling_update_status)
    };

    for (action_index = 0; action_index < RDD_TM_ACTION_PTR_TABLE_SIZE; action_index++)
    {
        tm_action_ptr = ((RDD_BYTES_2_DTS *)RDD_TM_ACTION_PTR_TABLE_PTR(get_runner_idx(us_tm_runner_image))) + action_index;
        RDD_BYTES_2_BITS_WRITE(us_actions_arr[action_index], tm_action_ptr);

        tm_action_ptr = ((RDD_BYTES_2_DTS *)RDD_TM_ACTION_PTR_TABLE_PTR(get_runner_idx(ds_tm_runner_image))) + action_index;
        RDD_BYTES_2_BITS_WRITE(ds_actions_arr[action_index], tm_action_ptr);
    }
}

static void rdd_write_action(uint8_t core_index, uint16_t *action_arr, uint8_t size_of_array, uint8_t *ptr, uint8_t tbl_size, uint16_t exception_label)
{
    uint32_t action_index;
    for (action_index = 0; action_index < tbl_size; action_index++)
    {
        if (action_index < size_of_array)
            RDD_BYTES_2_BITS_WRITE(action_arr[action_index], ptr + (sizeof(action_arr[0]) * action_index));
        else
            RDD_BYTES_2_BITS_WRITE(exception_label, ptr + (sizeof(action_arr[0]) * action_index));
    }
}

void rdd_actions_proj_init(void)
{
    uint8_t core_index;
    uint16_t processing0_flow_actions_arr[] = {
        [0] = ADDRESS_OF(image_4, action_gpe),
        [1 ... 16] = ADDRESS_OF(image_4, processing_header_update)
    };

    uint16_t processing1_flow_actions_arr[] = {
        [0] = ADDRESS_OF(image_1, action_gpe),
        [1 ... 16] = ADDRESS_OF(image_1, processing_header_update)
    };

    uint16_t processing2_flow_actions_arr[] = {
        [0] = ADDRESS_OF(image_2, action_gpe),
        [1 ... 16] = ADDRESS_OF(image_2, processing_header_update)
    };

    uint16_t processing0_gpe_actions_arr[] = {
        [0]  = 0, /* gpe_end has no label */
        [1]  = ADDRESS_OF(image_4, gpe_sop_push_replace_ddr_sram_32 ),
        [2]  = ADDRESS_OF(image_4, gpe_sop_push_replace_sram_32_64 ),
        [3]  = ADDRESS_OF(image_4, gpe_sop_push_replace_sram_64 ),
        [4]  = ADDRESS_OF(image_4, gpe_sop_push_replace_sram_64_32 ),
        [5]  = ADDRESS_OF(image_4, gpe_sop_pull_replace_ddr_sram_32 ),
        [6]  = ADDRESS_OF(image_4, gpe_sop_pull_replace_sram_32_64 ),
        [7]  = ADDRESS_OF(image_4, gpe_sop_pull_replace_sram_64 ),
        [8]  = ADDRESS_OF(image_4, gpe_sop_pull_replace_sram_64_32 ),
        [9]  = 0, /* gpe_replace_pointer_32_ddr is not supported */
        [10] = ADDRESS_OF(image_4, gpe_replace_pointer_32_sram ),
        [11] = ADDRESS_OF(image_4, gpe_replace_pointer_64_sram ),
        [12] = ADDRESS_OF(image_4, gpe_replace_16 ),
        [13] = ADDRESS_OF(image_4, gpe_replace_32 ),
        [14] = ADDRESS_OF(image_4, gpe_replace_bits_16 ),
        [15] = ADDRESS_OF(image_4, gpe_copy_add_16_cl ),
        [16] = ADDRESS_OF(image_4, gpe_copy_add_16_sram ),
        [17] = ADDRESS_OF(image_4, gpe_copy_bits_16_cl ),
        [18] = ADDRESS_OF(image_4, gpe_copy_bits_16_sram ),
        [19] = ADDRESS_OF(image_4, gpe_insert_16 ),
        [20] = ADDRESS_OF(image_4, gpe_delete_16 ),
        [21] = ADDRESS_OF(image_4, gpe_decrement_8 ),
        [22] = ADDRESS_OF(image_4, gpe_apply_icsum_16 ),
        [23] = ADDRESS_OF(image_4, gpe_apply_icsum_nz_16 ),
        [24] = ADDRESS_OF(image_4, gpe_compute_csum_16_cl ),
        [25] = ADDRESS_OF(image_4, gpe_compute_csum_16_sram )
    };

    uint16_t processing1_gpe_actions_arr[] = {
        [0]  = 0, /* gpe_end has no label */
        [1]  = ADDRESS_OF(image_1, gpe_sop_push_replace_ddr_sram_32 ),
        [2]  = ADDRESS_OF(image_1, gpe_sop_push_replace_sram_32_64 ),
        [3]  = ADDRESS_OF(image_1, gpe_sop_push_replace_sram_64 ),
        [4]  = ADDRESS_OF(image_1, gpe_sop_push_replace_sram_64_32 ),
        [5]  = ADDRESS_OF(image_1, gpe_sop_pull_replace_ddr_sram_32 ),
        [6]  = ADDRESS_OF(image_1, gpe_sop_pull_replace_sram_32_64 ),
        [7]  = ADDRESS_OF(image_1, gpe_sop_pull_replace_sram_64 ),
        [8]  = ADDRESS_OF(image_1, gpe_sop_pull_replace_sram_64_32 ),
        [9]  = 0, /* gpe_replace_pointer_32_ddr is not supported */
        [10] = ADDRESS_OF(image_1, gpe_replace_pointer_32_sram ),
        [11] = ADDRESS_OF(image_1, gpe_replace_pointer_64_sram ),
        [12] = ADDRESS_OF(image_1, gpe_replace_16 ),
        [13] = ADDRESS_OF(image_1, gpe_replace_32 ),
        [14] = ADDRESS_OF(image_1, gpe_replace_bits_16 ),
        [15] = ADDRESS_OF(image_1, gpe_copy_add_16_cl ),
        [16] = ADDRESS_OF(image_1, gpe_copy_add_16_sram ),
        [17] = ADDRESS_OF(image_1, gpe_copy_bits_16_cl ),
        [18] = ADDRESS_OF(image_1, gpe_copy_bits_16_sram ),
        [19] = ADDRESS_OF(image_1, gpe_insert_16 ),
        [20] = ADDRESS_OF(image_1, gpe_delete_16 ),
        [21] = ADDRESS_OF(image_1, gpe_decrement_8 ),
        [22] = ADDRESS_OF(image_1, gpe_apply_icsum_16 ),
        [23] = ADDRESS_OF(image_1, gpe_apply_icsum_nz_16 ),
        [24] = ADDRESS_OF(image_1, gpe_compute_csum_16_cl ),
        [25] = ADDRESS_OF(image_1, gpe_compute_csum_16_sram )
    };

    uint16_t processing2_gpe_actions_arr[] = {
        [0]  = 0, /* gpe_end has no label */
        [1]  = ADDRESS_OF(image_2, gpe_sop_push_replace_ddr_sram_32 ),
        [2]  = ADDRESS_OF(image_2, gpe_sop_push_replace_sram_32_64 ),
        [3]  = ADDRESS_OF(image_2, gpe_sop_push_replace_sram_64 ),
        [4]  = ADDRESS_OF(image_2, gpe_sop_push_replace_sram_64_32 ),
        [5]  = ADDRESS_OF(image_2, gpe_sop_pull_replace_ddr_sram_32 ),
        [6]  = ADDRESS_OF(image_2, gpe_sop_pull_replace_sram_32_64 ),
        [7]  = ADDRESS_OF(image_2, gpe_sop_pull_replace_sram_64 ),
        [8]  = ADDRESS_OF(image_2, gpe_sop_pull_replace_sram_64_32 ),
        [9]  = 0, /* gpe_replace_pointer_32_ddr is not supported */
        [10] = ADDRESS_OF(image_2, gpe_replace_pointer_32_sram ),
        [11] = ADDRESS_OF(image_2, gpe_replace_pointer_64_sram ),
        [12] = ADDRESS_OF(image_2, gpe_replace_16 ),
        [13] = ADDRESS_OF(image_2, gpe_replace_32 ),
        [14] = ADDRESS_OF(image_2, gpe_replace_bits_16 ),
        [15] = ADDRESS_OF(image_2, gpe_copy_add_16_cl ),
        [16] = ADDRESS_OF(image_2, gpe_copy_add_16_sram ),
        [17] = ADDRESS_OF(image_2, gpe_copy_bits_16_cl ),
        [18] = ADDRESS_OF(image_2, gpe_copy_bits_16_sram ),
        [19] = ADDRESS_OF(image_2, gpe_insert_16 ),
        [20] = ADDRESS_OF(image_2, gpe_delete_16 ),
        [21] = ADDRESS_OF(image_2, gpe_decrement_8 ),
        [22] = ADDRESS_OF(image_2, gpe_apply_icsum_16 ),
        [23] = ADDRESS_OF(image_2, gpe_apply_icsum_nz_16 ),
        [24] = ADDRESS_OF(image_2, gpe_compute_csum_16_cl ),
        [25] = ADDRESS_OF(image_2, gpe_compute_csum_16_sram )
    };

    for (core_index = 0; core_index < GROUPED_EN_SEGMENTS_NUM; core_index++)
    {
        if (rdp_core_to_image_map[core_index] == processing0_runner_image)
        {
            rdd_write_action(core_index, processing0_flow_actions_arr, RDD_FLOW_BASED_ACTION_PTR_TABLE_SIZE, (uint8_t *)RDD_FLOW_BASED_ACTION_PTR_TABLE_PTR(core_index), 
                RDD_FLOW_BASED_ACTION_PTR_TABLE_SIZE, image_4_processing_header_update);

            rdd_write_action(core_index, processing0_gpe_actions_arr, RDD_GPE_COMMAND_PRIMITIVE_TABLE_SIZE, (uint8_t *)RDD_GPE_COMMAND_PRIMITIVE_TABLE_PTR(core_index), 
                RDD_GPE_COMMAND_PRIMITIVE_TABLE_SIZE, 0);
        }
        else if (rdp_core_to_image_map[core_index] == processing1_runner_image)
        {
            rdd_write_action(core_index, processing1_flow_actions_arr, RDD_FLOW_BASED_ACTION_PTR_TABLE_SIZE, (uint8_t *)RDD_FLOW_BASED_ACTION_PTR_TABLE_PTR(core_index),
                RDD_FLOW_BASED_ACTION_PTR_TABLE_SIZE, image_1_processing_header_update);

            rdd_write_action(core_index, processing1_gpe_actions_arr, RDD_GPE_COMMAND_PRIMITIVE_TABLE_SIZE, (uint8_t *)RDD_GPE_COMMAND_PRIMITIVE_TABLE_PTR(core_index), 
                RDD_GPE_COMMAND_PRIMITIVE_TABLE_SIZE, 0);
        }
        else if (rdp_core_to_image_map[core_index] == processing2_runner_image)
        {
            rdd_write_action(core_index, processing2_flow_actions_arr, RDD_FLOW_BASED_ACTION_PTR_TABLE_SIZE,(uint8_t *)RDD_FLOW_BASED_ACTION_PTR_TABLE_PTR(core_index),
                RDD_FLOW_BASED_ACTION_PTR_TABLE_SIZE, image_2_processing_header_update);

            rdd_write_action(core_index, processing2_gpe_actions_arr, RDD_GPE_COMMAND_PRIMITIVE_TABLE_SIZE, (uint8_t *)RDD_GPE_COMMAND_PRIMITIVE_TABLE_PTR(core_index), 
                RDD_GPE_COMMAND_PRIMITIVE_TABLE_SIZE, 0);
        }
    }
}


