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


#include "rdd_complex_scheduler.h"
#if !defined G9991
#include "rdd_ag_service_queues.h"
#endif
#include "rdd_ag_us_tm.h"
#include "rdd_ag_ds_tm.h"
#include "rdp_drv_qm.h"

extern rdd_bb_id rdpa_emac_to_bb_id_tx[rdpa_emac__num_of];
extern bdmf_error_t rdd_scheduling_scheduler_block_cfg(rdpa_traffic_dir dir, uint8_t qm_queue_index, rdd_scheduling_queue_descriptor_t *scheduler_cfg, bdmf_boolean type, uint8_t dwrr_offset, bdmf_boolean enable);

typedef struct
{
    uint8_t bbh_queue;
    complex_scheduler_block_type_t type[COMPLEX_SCHEDULER_NUM_OF_QUEUES];
} complex_scheduler_info_t;

/* mappping between complex scheduler to bbh queue */
static complex_scheduler_info_t complex_scheduler_info[2][RDD_COMPLEX_SCHEDULER_TABLE_SIZE];

/* API to RDPA level */
bdmf_error_t rdd_complex_scheduler_cfg(rdpa_traffic_dir dir, uint8_t complex_scheduler_index,
    complex_scheduler_cfg_t *cfg)
{
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS *entry;
    RDD_BBH_QUEUE_DESCRIPTOR_DTS *bbh_queue_entry;
    bdmf_error_t rc = 0;
    uint32_t core_index;

    RDD_BTRACE("dir = %d, complex_scheduler_index = %d, cfg %p = { dwrr_offset_sir = %d, dwrr_offset_pir = %d, "
        "bbh_queue_index = %d, hw_bbh_qid = %d }\n",
        dir, complex_scheduler_index, cfg, cfg->dwrr_offset_sir, cfg->dwrr_offset_pir, cfg->bbh_queue_index,
        cfg->hw_bbh_qid);

    if ((complex_scheduler_index >= RDD_COMPLEX_SCHEDULER_TABLE_SIZE) ||
        (cfg->dwrr_offset_sir >= complex_scheduler_num_of_dwrr_offset) ||
        (cfg->dwrr_offset_pir >= complex_scheduler_num_of_dwrr_offset) ||
        (cfg->bbh_queue_index >= RDD_BBH_QUEUE_TABLE_SIZE))
    {
        return BDMF_ERR_PARM;
    }

    for (core_index = 0; core_index < GROUPED_EN_SEGMENTS_NUM; core_index++)
    {
        if ((dir == rdpa_dir_ds) && (IS_DS_TM_RUNNER_IMAGE(core_index)))
        {
            entry = ((RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS *)RDD_COMPLEX_SCHEDULER_TABLE_PTR(core_index)) + complex_scheduler_index;
            bbh_queue_entry = ((RDD_BBH_QUEUE_DESCRIPTOR_DTS *)RDD_DS_TM_BBH_QUEUE_TABLE_PTR(core_index)) + cfg->bbh_queue_index;
        }
        else if ((dir == rdpa_dir_us) && (IS_US_TM_RUNNER_IMAGE(core_index)))
        {
            entry = ((RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS *)RDD_COMPLEX_SCHEDULER_TABLE_PTR(core_index)) + complex_scheduler_index;
            bbh_queue_entry = ((RDD_BBH_QUEUE_DESCRIPTOR_DTS *)RDD_US_TM_BBH_QUEUE_TABLE_PTR(core_index)) + cfg->bbh_queue_index;
        }
        else
            continue;

        /* initialize budget for all queues - relevent for the case no rate limiter was configured */
        RDD_COMPLEX_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_0_WRITE(COMPLEX_SCHEDULER_FULL_BUDGET_VECTOR, entry);
        RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DWRR_OFFSET_SIR_WRITE(cfg->dwrr_offset_sir, entry);
        RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DWRR_OFFSET_PIR_WRITE(cfg->dwrr_offset_pir, entry);
        RDD_COMPLEX_SCHEDULER_DESCRIPTOR_IS_POSITIVE_BUDGET_WRITE(1, entry);

        if (!cfg->parent_exists)
        {
            RDD_BBH_QUEUE_DESCRIPTOR_SCHEDULER_INDEX_WRITE(complex_scheduler_index, bbh_queue_entry);
            RDD_BBH_QUEUE_DESCRIPTOR_SCHEDULER_TYPE_WRITE(RDD_SCHED_TYPE_COMPLEX, bbh_queue_entry);
#if defined(CONFIG_MULTI_WAN_SUPPORT) && !defined(BCM_PON_XRDP)
            RDD_BBH_QUEUE_DESCRIPTOR_HW_BBH_QID_WRITE(cfg->hw_bbh_qid, bbh_queue_entry);
#endif
        }

        if (dir == rdpa_dir_ds)
        {
#ifdef XRDP_BBH_PER_LAN_PORT
            if (cfg->bbh_queue_index < rdpa_emac__num_of)
            {
                RDD_COMPLEX_SCHEDULER_DESCRIPTOR_BBH_QUEUE_WRITE(rdpa_emac_to_bb_id_tx[cfg->bbh_queue_index], entry);
            }
            else
                return BDMF_ERR_PARM;
#else
            RDD_COMPLEX_SCHEDULER_DESCRIPTOR_BBH_QUEUE_WRITE(cfg->bbh_queue_index, entry);
#endif
        }
        else
        {
            RDD_COMPLEX_SCHEDULER_DESCRIPTOR_BBH_QUEUE_WRITE(cfg->bbh_queue_index, entry);
        }

        complex_scheduler_info[dir][complex_scheduler_index].bbh_queue = cfg->bbh_queue_index;
    }
    return rc;
}

bdmf_error_t rdd_complex_scheduler_block_cfg(rdpa_traffic_dir dir, uint8_t complex_scheduler_index,
    complex_scheduler_block_t *block)
{
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS *entry;
    rdd_scheduling_queue_descriptor_t block_cfg = {};
    bdmf_error_t rc = BDMF_ERR_OK;
    uint32_t is_scheduler_slot_vector;
    uint32_t is_scheduler_basic_vector;

    RDD_BTRACE("dir = %d, complex_scheduler_index = %d, block %p = { block_index = %d, scheduler_slot_index = %d, "
        "bs_dwrr_offset = %d, quantum_number = %d }\n",
        dir, complex_scheduler_index, block, block->block_index, block->scheduler_slot_index, block->bs_dwrr_offset,
        block->quantum_number);

    if ((complex_scheduler_index >= RDD_COMPLEX_SCHEDULER_TABLE_SIZE) ||
        (block->scheduler_slot_index >= COMPLEX_SCHEDULER_NUM_OF_QUEUES))
    {
        return BDMF_ERR_PARM;
    }

    if (dir == rdpa_dir_ds)
    {
        return BDMF_ERR_NOT_SUPPORTED;
/*      entry = ((RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS *)RDD_COMPLEX_SCHEDULER_TABLE_PTR(get_runner_idx(ds_tm_runner_image))) + complex_scheduler_index;
        if (complex_scheduler_info[dir][complex_scheduler_index].type)
            rdd_ag_ds_tm_basic_scheduler_table_quantum_number_set(block->block_index, block->quantum_number);
        else
            rc = rdd_ag_ds_tm_scheduling_queue_table_quantum_number_set((block->block_index - drv_qm_get_ds_start()), block->quantum_number);
*/
    }
    else
    {
        entry = ((RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS *)RDD_COMPLEX_SCHEDULER_TABLE_PTR(get_runner_idx(us_tm_runner_image))) + complex_scheduler_index;
        if (block->block_type == complex_scheduler_block_bs)
            rdd_ag_us_tm_basic_scheduler_table_us_quantum_number_set(block->block_index, block->quantum_number);
        else if (block->block_type == complex_scheduler_block_cs)
            rdd_ag_us_tm_complex_scheduler_table_quantum_number_set(block->block_index, block->quantum_number);
        else
            rc = rdd_ag_us_tm_scheduling_queue_table_quantum_number_set((block->block_index - drv_qm_get_us_start()), block->quantum_number);
    }

    /* save the block type */
    complex_scheduler_info[dir][complex_scheduler_index].type[block->scheduler_slot_index] = block->block_type;

    /* mapping block to complex scheduler */
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_BLOCK_INDEX_WRITE(block->block_index, entry, block->scheduler_slot_index);

    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_IS_SCHEDULER_SLOT_READ(is_scheduler_slot_vector, entry);
    if (block->block_type != complex_scheduler_block_queue)
        is_scheduler_slot_vector |= (1 << block->scheduler_slot_index);
    else
        is_scheduler_slot_vector &= ~(1 << block->scheduler_slot_index);
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_IS_SCHEDULER_SLOT_WRITE(is_scheduler_slot_vector, entry);

    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_IS_SCHEDULER_BASIC_READ(is_scheduler_basic_vector, entry);
    if (block->block_type == complex_scheduler_block_bs)
        is_scheduler_basic_vector |= (1 << block->scheduler_slot_index);
    else
        is_scheduler_basic_vector &= ~(1 << block->scheduler_slot_index);
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_IS_SCHEDULER_BASIC_WRITE(is_scheduler_basic_vector, entry);

    /* mapping complex scheduler to block */
    block_cfg.bbh_queue_index = complex_scheduler_info[dir][complex_scheduler_index].bbh_queue;
    block_cfg.scheduler_index = complex_scheduler_index;
    block_cfg.block_type = 1; /* complex scheduler */
    block_cfg.queue_bit_mask = block->scheduler_slot_index; /* in case of cs under complex scheduler bit mask is the index */

    if (block->block_type == complex_scheduler_block_cs)
    {
        RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS *sub_entry;
        if (dir == rdpa_dir_ds)
            return BDMF_ERR_NOT_SUPPORTED;

        sub_entry = ((RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS *)RDD_COMPLEX_SCHEDULER_TABLE_PTR(get_runner_idx(us_tm_runner_image))) + block->block_index;

        /* mapping complex scheduler to complex scheduler */
        RDD_COMPLEX_SCHEDULER_DESCRIPTOR_PARENT_SCHEDULER_EXISTS_WRITE(1, sub_entry);
        RDD_COMPLEX_SCHEDULER_DESCRIPTOR_PARENT_SCHEDULER_INDEX_WRITE(complex_scheduler_index, sub_entry);
        RDD_COMPLEX_SCHEDULER_DESCRIPTOR_PARENT_SCHEDULER_SLOT_INDEX_WRITE(block->scheduler_slot_index, sub_entry);
    }
    else
    {
        rc = rc ? rc : rdd_scheduling_scheduler_block_cfg(dir, block->block_index, &block_cfg,
            block->block_type == complex_scheduler_block_bs, block->bs_dwrr_offset, 1);
    }

    return rc;
}

bdmf_error_t rdd_complex_scheduler_block_remove(rdpa_traffic_dir dir, uint8_t complex_scheduler_index, uint8_t scheduler_slot_index)
{
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS *entry;
    rdd_scheduling_queue_descriptor_t block_cfg = {};
    uint8_t block_index;
    bdmf_error_t rc = BDMF_ERR_OK;
    uint32_t core_index;
    uint32_t is_scheduler_slot_vector;
    uint32_t is_scheduler_basic_vector;
    complex_scheduler_block_type_t type;

    RDD_BTRACE("dir = %d, complex_scheduler_index = %d, scheduler_slot_index = %d\n",
        dir, complex_scheduler_index, scheduler_slot_index);

    if (complex_scheduler_index >= RDD_COMPLEX_SCHEDULER_TABLE_SIZE)
    {
        return BDMF_ERR_PARM;
    }

    type = complex_scheduler_info[dir][complex_scheduler_index].type[scheduler_slot_index];

    for (core_index = 0; core_index < GROUPED_EN_SEGMENTS_NUM; core_index++)
    {
        if ((dir == rdpa_dir_ds) && (IS_DS_TM_RUNNER_IMAGE(core_index)))
            entry = ((RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS *)RDD_COMPLEX_SCHEDULER_TABLE_PTR(core_index)) + complex_scheduler_index;
        else if ((dir == rdpa_dir_us) && (IS_US_TM_RUNNER_IMAGE(core_index)))
            entry = ((RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS *)RDD_COMPLEX_SCHEDULER_TABLE_PTR(core_index)) + complex_scheduler_index;
        else
            continue;

        RDD_COMPLEX_SCHEDULER_DESCRIPTOR_IS_SCHEDULER_SLOT_READ(is_scheduler_slot_vector, entry);
        is_scheduler_slot_vector &= ~(1 << scheduler_slot_index);
        RDD_COMPLEX_SCHEDULER_DESCRIPTOR_IS_SCHEDULER_SLOT_WRITE(is_scheduler_slot_vector, entry);

        RDD_COMPLEX_SCHEDULER_DESCRIPTOR_IS_SCHEDULER_BASIC_READ(is_scheduler_basic_vector, entry);
        is_scheduler_basic_vector &= ~(1 << scheduler_slot_index);
        RDD_COMPLEX_SCHEDULER_DESCRIPTOR_IS_SCHEDULER_BASIC_WRITE(is_scheduler_basic_vector, entry);

        RDD_COMPLEX_SCHEDULER_DESCRIPTOR_BLOCK_INDEX_READ(block_index, entry, scheduler_slot_index);
        if (dir == rdpa_dir_ds && type == complex_scheduler_block_queue)
            block_index += drv_qm_get_ds_start();

        if (type == complex_scheduler_block_cs)
        {
            RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS *sub_entry;
            uint32_t block_index;
            if (dir == rdpa_dir_ds)
                return BDMF_ERR_NOT_SUPPORTED;

            RDD_COMPLEX_SCHEDULER_DESCRIPTOR_BLOCK_INDEX_READ(block_index, entry, scheduler_slot_index);

            sub_entry = ((RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS *)RDD_COMPLEX_SCHEDULER_TABLE_PTR(get_runner_idx(us_tm_runner_image))) + block_index;

            /* mapping complex scheduler to complex scheduler */
            RDD_COMPLEX_SCHEDULER_DESCRIPTOR_PARENT_SCHEDULER_EXISTS_WRITE(0, sub_entry);
            RDD_COMPLEX_SCHEDULER_DESCRIPTOR_PARENT_SCHEDULER_INDEX_WRITE(0, sub_entry);
            RDD_COMPLEX_SCHEDULER_DESCRIPTOR_PARENT_SCHEDULER_SLOT_INDEX_WRITE(0, sub_entry);
        }
        else
        {
            rc = rdd_scheduling_scheduler_block_cfg(dir, block_index, &block_cfg,
                type == complex_scheduler_block_bs, 0, 0);
        }
    }
    return rc;
}

/* API to block */
bdmf_error_t rdd_complex_scheduler_rate_set(rdpa_traffic_dir dir, uint8_t complex_scheduler_index, uint32_t block_bit_mask)
{
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS *entry;
    uint32_t budget;
    uint32_t core_index;

    RDD_BTRACE("dir = %d, complex_scheduler_index = %d, block_bit_mask = %d\n",
        dir, complex_scheduler_index, block_bit_mask);

    if (complex_scheduler_index >= RDD_COMPLEX_SCHEDULER_TABLE_SIZE)
    {
        return BDMF_ERR_PARM;
    }

    for (core_index = 0; core_index < GROUPED_EN_SEGMENTS_NUM; core_index++)
    {
        if ((dir == rdpa_dir_ds) && (IS_DS_TM_RUNNER_IMAGE(core_index)))
            entry = ((RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS *)RDD_COMPLEX_SCHEDULER_TABLE_PTR(core_index)) + complex_scheduler_index;
        else if ((dir == rdpa_dir_us) && (IS_US_TM_RUNNER_IMAGE(core_index)))
            entry = ((RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS *)RDD_COMPLEX_SCHEDULER_TABLE_PTR(core_index)) + complex_scheduler_index;
        else
            continue;

        RDD_COMPLEX_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_0_READ(budget, entry);
        budget |= block_bit_mask;
        RDD_COMPLEX_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_0_WRITE(budget, entry);
        RDD_COMPLEX_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_1_READ(budget, entry);
        budget |= block_bit_mask;
        RDD_COMPLEX_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_1_WRITE(budget, entry);
    }

    return BDMF_ERR_OK;
}

