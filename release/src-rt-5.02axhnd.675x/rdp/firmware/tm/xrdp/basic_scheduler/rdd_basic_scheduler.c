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


#include "rdd_basic_scheduler.h"
#if !defined G9991
#include "rdd_ag_service_queues.h"
#endif
#include "rdd_ag_us_tm.h"
#include "rdd_ag_ds_tm.h"
#include "rdp_drv_qm.h"

extern rdd_bb_id rdpa_emac_to_bb_id_tx[rdpa_emac__num_of];
extern bdmf_error_t rdd_scheduling_scheduler_block_cfg(rdpa_traffic_dir dir, uint8_t qm_queue_index, rdd_scheduling_queue_descriptor_t *scheduler_cfg, bdmf_boolean type, uint8_t dwrr_offset, bdmf_boolean enable);

/* mappping between basic scheduler to bbh queue */
static uint8_t basic_scheduler_to_bbh_queue[2][RDD_BASIC_SCHEDULER_TABLE_SIZE];

/* bbh_q is splitted into 2 separated fields (msb and lsb) because of data structure limitation (runner and host write to same bytes creates race condition */
void write_bbh_queue_g(rdd_bb_id bb_id, uint32_t *table_ptr, uint8_t basic_scheduler_index)
{
	RDD_BASIC_SCHEDULER_DESCRIPTOR_BBH_QUEUE_LSB_WRITE_G(bb_id & 0xF, table_ptr, basic_scheduler_index);
	RDD_BASIC_SCHEDULER_DESCRIPTOR_BBH_QUEUE_MSB_WRITE_G((bb_id >> 4) & 0x3, table_ptr, basic_scheduler_index);
}

/* API to RDPA level */
bdmf_error_t rdd_basic_scheduler_cfg(rdpa_traffic_dir dir, uint8_t basic_scheduler_index, basic_scheduler_cfg_t *cfg)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    uint32_t *basic_scheduler_table_ptr;
    uint32_t *bbh_queue_table_ptr;;

    RDD_BTRACE("dir = %d, basic_scheduler_index = %d, cfg %p = { dwrr_offset = %d, "
        "bbh_queue_index = %d,  hw_bbh_qid = %d }\n",
        dir, basic_scheduler_index, cfg, cfg->dwrr_offset, cfg->bbh_queue_index,
        cfg->hw_bbh_qid);

    if ((basic_scheduler_index >= RDD_BASIC_SCHEDULER_TABLE_SIZE) ||
        (cfg->dwrr_offset >= basic_scheduler_num_of_dwrr_offset) ||
        (cfg->bbh_queue_index >= RDD_BBH_QUEUE_TABLE_SIZE))
    {
        return BDMF_ERR_PARM;
    }

    if (dir == rdpa_dir_ds)
    {
        basic_scheduler_table_ptr = RDD_BASIC_SCHEDULER_TABLE_DS_ADDRESS_ARR;
        bbh_queue_table_ptr = RDD_DS_TM_BBH_QUEUE_TABLE_ADDRESS_ARR;
    }
    else
    {
        basic_scheduler_table_ptr = RDD_BASIC_SCHEDULER_TABLE_US_ADDRESS_ARR;
        bbh_queue_table_ptr = RDD_US_TM_BBH_QUEUE_TABLE_ADDRESS_ARR;
    }

    /* initialize budget for all queues - relevent for the case no rate limiter was configured */
    RDD_BASIC_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_WRITE_G(BASIC_SCHEDULER_FULL_BUDGET_VECTOR, basic_scheduler_table_ptr, basic_scheduler_index);
    RDD_BASIC_SCHEDULER_DESCRIPTOR_DWRR_OFFSET_WRITE_G(cfg->dwrr_offset, basic_scheduler_table_ptr, basic_scheduler_index);
    RDD_BASIC_SCHEDULER_DESCRIPTOR_IS_POSITIVE_BUDGET_WRITE_G(1, basic_scheduler_table_ptr, basic_scheduler_index);
    RDD_BBH_QUEUE_DESCRIPTOR_SCHEDULER_INDEX_WRITE_G(basic_scheduler_index, bbh_queue_table_ptr, cfg->bbh_queue_index);
    RDD_BBH_QUEUE_DESCRIPTOR_SCHEDULER_TYPE_WRITE_G(RDD_SCHED_TYPE_BASIC, bbh_queue_table_ptr, cfg->bbh_queue_index);
#if defined(CONFIG_MULTI_WAN_SUPPORT) && !defined(BCM_PON_XRDP)
    RDD_BBH_QUEUE_DESCRIPTOR_HW_BBH_QID_WRITE_G(cfg->hw_bbh_qid, bbh_queue_table_ptr, cfg->bbh_queue_index);
#endif

    if (dir == rdpa_dir_ds)
    {
#ifndef G9991
#ifdef XRDP_BBH_PER_LAN_PORT
        if (cfg->bbh_queue_index < rdpa_emac__num_of)
        {
        	write_bbh_queue_g(rdpa_emac_to_bb_id_tx[cfg->bbh_queue_index], basic_scheduler_table_ptr, basic_scheduler_index);
        }
        else
            return BDMF_ERR_PARM;             
#else
        write_bbh_queue_g(cfg->bbh_queue_index, basic_scheduler_table_ptr, basic_scheduler_index);
#endif
#else /* G9991 */
        write_bbh_queue_g(0, basic_scheduler_table_ptr, basic_scheduler_index);
#endif
    }
    else
    {
    	write_bbh_queue_g(cfg->bbh_queue_index, basic_scheduler_table_ptr, basic_scheduler_index);
    }

    /* init bbh-queue */
    basic_scheduler_to_bbh_queue[dir][basic_scheduler_index] = cfg->bbh_queue_index;

    return rc;
}

bdmf_error_t rdd_basic_scheduler_dwrr_offset_cfg(rdpa_traffic_dir dir, uint8_t basic_scheduler_index, uint8_t dwrr_offset)
{
    uint32_t *basic_scheduler_table_ptr;

    RDD_BTRACE("dir = %d, basic_scheduler_index = %d, dwrr_offset = %d }\n",
        dir, basic_scheduler_index, dwrr_offset);

    if ((basic_scheduler_index >= RDD_BASIC_SCHEDULER_TABLE_SIZE) ||
        (dwrr_offset >= basic_scheduler_num_of_dwrr_offset))
    {
        return BDMF_ERR_PARM;
    }

    if (dir == rdpa_dir_ds)
        basic_scheduler_table_ptr = RDD_BASIC_SCHEDULER_TABLE_DS_ADDRESS_ARR;
    else
        basic_scheduler_table_ptr = RDD_BASIC_SCHEDULER_TABLE_US_ADDRESS_ARR;

    /* initialize budget for all queues - relevent for the case no rate limiter was configured */
    RDD_BASIC_SCHEDULER_DESCRIPTOR_DWRR_OFFSET_WRITE_G(dwrr_offset, basic_scheduler_table_ptr, basic_scheduler_index);

    return BDMF_ERR_OK;
}

bdmf_error_t rdd_basic_scheduler_queue_cfg(rdpa_traffic_dir dir, uint8_t basic_scheduler_index, basic_scheduler_queue_t *queue)
{
    uint32_t *basic_scheduler_table_ptr;
    bdmf_error_t rc;
    uint8_t first_queue_index;
    rdd_scheduling_queue_descriptor_t queue_cfg = {};

    RDD_BTRACE("dir = %d, basic_scheduler_index = %d, queue %p = { qm_queue_index = %d, queue_scheduler_index = %d, "
        "quantum_number = %d }\n",
        dir, basic_scheduler_index, queue, queue->qm_queue_index, queue->queue_scheduler_index, queue->quantum_number);

    if ((basic_scheduler_index >= RDD_BASIC_SCHEDULER_TABLE_SIZE) ||
        (queue->queue_scheduler_index >= BASIC_SCHEDULER_NUM_OF_QUEUES))
    {
        return BDMF_ERR_PARM;
    }
    if (dir == rdpa_dir_ds)
    {
        basic_scheduler_table_ptr = RDD_BASIC_SCHEDULER_TABLE_DS_ADDRESS_ARR;
        first_queue_index = drv_qm_get_ds_start();
        rc = rdd_ag_ds_tm_scheduling_queue_table_quantum_number_set((queue->qm_queue_index - first_queue_index), queue->quantum_number);
    }
    else
    {
        basic_scheduler_table_ptr = RDD_BASIC_SCHEDULER_TABLE_US_ADDRESS_ARR;
        first_queue_index = drv_qm_get_us_start();
        rc = rdd_ag_us_tm_scheduling_queue_table_quantum_number_set((queue->qm_queue_index - first_queue_index), queue->quantum_number);
    }

    /* mapping queue to basic scheduler */
    RDD_BASIC_SCHEDULER_DESCRIPTOR_QUEUE_INDEX_WRITE_G((queue->qm_queue_index - first_queue_index), basic_scheduler_table_ptr, basic_scheduler_index, queue->queue_scheduler_index);

    /* mapping basic scheduler to queue */
    queue_cfg.bbh_queue_index = basic_scheduler_to_bbh_queue[dir][basic_scheduler_index];
    queue_cfg.scheduler_index = basic_scheduler_index;
    queue_cfg.block_type = 0; /* basic scheduler */
    queue_cfg.queue_bit_mask = 1 << (queue->queue_scheduler_index);
    rc = rc ? rc : rdd_scheduling_scheduler_block_cfg(dir, queue->qm_queue_index, &queue_cfg, 0, 0, 1);

    return rc;
}

bdmf_error_t rdd_basic_scheduler_queue_remove(rdpa_traffic_dir dir, uint8_t basic_scheduler_index, uint8_t queue_scheduler_index)
{
    RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS *entry;
    rdd_scheduling_queue_descriptor_t queue_cfg = {};
    uint8_t queue_index;
    bdmf_error_t rc;

    RDD_BTRACE("dir = %d, basic_scheduler_index = %d, queue_scheduler_index = %d\n",
        dir, basic_scheduler_index, queue_scheduler_index);

    if ((basic_scheduler_index >= RDD_BASIC_SCHEDULER_TABLE_SIZE) ||
        (queue_scheduler_index >= BASIC_SCHEDULER_NUM_OF_QUEUES))
    {
        return BDMF_ERR_PARM;
    }

    if (dir == rdpa_dir_ds)
        entry = ((RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS *)RDD_BASIC_SCHEDULER_TABLE_DS_PTR(get_runner_idx(ds_tm_runner_image))) + basic_scheduler_index;
    else
        entry = ((RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS *)RDD_BASIC_SCHEDULER_TABLE_US_PTR(get_runner_idx(us_tm_runner_image))) + basic_scheduler_index;

    /* write bit mask 0 to queue */
    RDD_BASIC_SCHEDULER_DESCRIPTOR_QUEUE_INDEX_READ(queue_index, entry, queue_scheduler_index);
    if (dir == rdpa_dir_ds)
        queue_index += drv_qm_get_ds_start();
    rc = rdd_scheduling_scheduler_block_cfg(dir, queue_index, &queue_cfg, 0, 0, 0);

    return rc;
}

/* API to complex scheduler module */
bdmf_error_t rdd_basic_scheduler_dwrr_cfg(rdpa_traffic_dir dir, uint8_t basic_scheduler_index, uint8_t quantum_number)
{
    uint32_t *basic_scheduler_table_ptr;

    if (basic_scheduler_index >= RDD_BASIC_SCHEDULER_TABLE_SIZE)
    {
        return BDMF_ERR_PARM;
    }

    if (dir == rdpa_dir_ds)
        basic_scheduler_table_ptr = RDD_BASIC_SCHEDULER_TABLE_DS_ADDRESS_ARR;
    else
        basic_scheduler_table_ptr = RDD_BASIC_SCHEDULER_TABLE_US_ADDRESS_ARR;

    RDD_BASIC_SCHEDULER_DESCRIPTOR_QUANTUM_NUMBER_WRITE_G(quantum_number, basic_scheduler_table_ptr, basic_scheduler_index);

    return BDMF_ERR_OK;
}

bdmf_error_t rdd_basic_scheduler_block_cfg(rdpa_traffic_dir dir, uint8_t basic_scheduler_index,
    rdd_scheduling_queue_descriptor_t *scheduler_cfg, uint8_t dwrr_offset)
{
    uint32_t *basic_scheduler_table_ptr;

    RDD_BTRACE("dir = %d, basic_scheduler_index = %d, scheduler_cfg %p = { scheduler_index = %d, bit_mask = %d, "
        "bbh_queue = %d, scheduler_type = %d, dwrr_offset = %d }\n",
        dir, basic_scheduler_index, scheduler_cfg, scheduler_cfg->scheduler_index, scheduler_cfg->queue_bit_mask,
        scheduler_cfg->bbh_queue_index, scheduler_cfg->block_type, dwrr_offset);

    if ((basic_scheduler_index >= RDD_BASIC_SCHEDULER_TABLE_SIZE) ||
        (dwrr_offset >= basic_scheduler_num_of_dwrr_offset) ||
        (scheduler_cfg->bbh_queue_index >= RDD_BBH_QUEUE_TABLE_SIZE) ||
        (scheduler_cfg->scheduler_index >= RDD_COMPLEX_SCHEDULER_TABLE_SIZE) ||
        (scheduler_cfg->queue_bit_mask >= RDD_BASIC_SCHEDULER_TABLE_SIZE))
    {
        return BDMF_ERR_PARM;
    }

    if (dir == rdpa_dir_ds)
        basic_scheduler_table_ptr = RDD_BASIC_SCHEDULER_TABLE_DS_ADDRESS_ARR;
    else
        basic_scheduler_table_ptr = RDD_BASIC_SCHEDULER_TABLE_US_ADDRESS_ARR;

    /* initialize budget for all queues - relevent for the case no rate limiter was configured */
    RDD_BASIC_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_WRITE_G(BASIC_SCHEDULER_FULL_BUDGET_VECTOR, basic_scheduler_table_ptr, basic_scheduler_index);
    RDD_BASIC_SCHEDULER_DESCRIPTOR_DWRR_OFFSET_WRITE_G(dwrr_offset, basic_scheduler_table_ptr, basic_scheduler_index);

    /* mapping basic scheduler to complex scheduler */
    RDD_BASIC_SCHEDULER_DESCRIPTOR_COMPLEX_SCHEDULER_EXISTS_WRITE_G(scheduler_cfg->block_type, basic_scheduler_table_ptr, basic_scheduler_index);
    RDD_BASIC_SCHEDULER_DESCRIPTOR_COMPLEX_SCHEDULER_INDEX_WRITE_G(scheduler_cfg->scheduler_index, basic_scheduler_table_ptr, basic_scheduler_index);
    RDD_BASIC_SCHEDULER_DESCRIPTOR_COMPLEX_SCHEDULER_SLOT_INDEX_WRITE_G(scheduler_cfg->queue_bit_mask, basic_scheduler_table_ptr, basic_scheduler_index);

    basic_scheduler_to_bbh_queue[dir][basic_scheduler_index] = scheduler_cfg->bbh_queue_index;

    return BDMF_ERR_OK;
}

/* API to rate limiter module */
bdmf_error_t rdd_basic_scheduler_rate_limiter_cfg(rdpa_traffic_dir dir, uint8_t basic_scheduler_index, uint8_t rate_limiter_index)
{
    uint32_t *basic_scheduler_table_ptr;
    bdmf_boolean cs_exist;

    RDD_BTRACE("dir = %d, basic_scheduler_index = %d, rate_limiter_index = %d\n",
        dir, basic_scheduler_index, rate_limiter_index);

    if ((basic_scheduler_index >= RDD_BASIC_SCHEDULER_TABLE_SIZE) ||
        (rate_limiter_index >= RDD_BASIC_RATE_LIMITER_TABLE_SIZE))
    {
        return BDMF_ERR_PARM;
    }

    if (dir == rdpa_dir_ds)
        basic_scheduler_table_ptr = RDD_BASIC_SCHEDULER_TABLE_DS_ADDRESS_ARR;
    else
        basic_scheduler_table_ptr = RDD_BASIC_SCHEDULER_TABLE_US_ADDRESS_ARR;

    RDD_BASIC_SCHEDULER_DESCRIPTOR_RATE_LIMITER_INDEX_WRITE_G(rate_limiter_index, basic_scheduler_table_ptr, basic_scheduler_index);
    RDD_BASIC_SCHEDULER_DESCRIPTOR_RATE_LIMIT_ENABLE_WRITE_G(1, basic_scheduler_table_ptr, basic_scheduler_index);
    RDD_BASIC_SCHEDULER_DESCRIPTOR_COMPLEX_SCHEDULER_EXISTS_READ_G(cs_exist, basic_scheduler_table_ptr, basic_scheduler_index);
    if (!cs_exist)
        RDD_BASIC_SCHEDULER_DESCRIPTOR_IS_POSITIVE_BUDGET_WRITE_G(1, basic_scheduler_table_ptr, basic_scheduler_index);

    return BDMF_ERR_OK;
}

bdmf_error_t rdd_basic_scheduler_rate_limiter_remove(rdpa_traffic_dir dir, uint8_t basic_scheduler_index)
{
    uint32_t *basic_scheduler_table_ptr;
    bdmf_boolean cs_exist;
    uint8_t cs_slot_index, cs_index;
    uint32_t budget_vector;
    bdmf_error_t rc = BDMF_ERR_OK;

    RDD_BTRACE("dir = %d, basic_scheduler_index = %d\n", dir, basic_scheduler_index);

    if (basic_scheduler_index >= RDD_BASIC_SCHEDULER_TABLE_SIZE)
    {
        return BDMF_ERR_PARM;
    }

    if (dir == rdpa_dir_ds)
        basic_scheduler_table_ptr = RDD_BASIC_SCHEDULER_TABLE_DS_ADDRESS_ARR;
    else
        basic_scheduler_table_ptr = RDD_BASIC_SCHEDULER_TABLE_US_ADDRESS_ARR;

    RDD_BASIC_SCHEDULER_DESCRIPTOR_RATE_LIMIT_ENABLE_WRITE_G(0, basic_scheduler_table_ptr, basic_scheduler_index);

    /* in case the bs is under cs make sure it has rate */
    RDD_BASIC_SCHEDULER_DESCRIPTOR_COMPLEX_SCHEDULER_EXISTS_READ_G(cs_exist, basic_scheduler_table_ptr, basic_scheduler_index);
    if (cs_exist)
    {
        RDD_BASIC_SCHEDULER_DESCRIPTOR_COMPLEX_SCHEDULER_SLOT_INDEX_READ_G(cs_slot_index, basic_scheduler_table_ptr, basic_scheduler_index);
        RDD_BASIC_SCHEDULER_DESCRIPTOR_COMPLEX_SCHEDULER_INDEX_READ_G(cs_index, basic_scheduler_table_ptr, basic_scheduler_index);
        if (dir == rdpa_dir_ds)
        {
            return BDMF_ERR_NOT_SUPPORTED;
        }
        else
        {
            rdd_ag_us_tm_complex_scheduler_table_slot_budget_bit_vector_0_get(cs_index, &budget_vector);
            budget_vector |= (1 << cs_slot_index);
            rdd_ag_us_tm_complex_scheduler_table_slot_budget_bit_vector_0_set(cs_index, budget_vector);
        }
    }
    else
        RDD_BASIC_SCHEDULER_DESCRIPTOR_IS_POSITIVE_BUDGET_WRITE_G(1, basic_scheduler_table_ptr, basic_scheduler_index);

    return rc;
}

/* API to queue */
bdmf_error_t rdd_basic_scheduler_rate_set(rdpa_traffic_dir dir, uint8_t basic_scheduler_index, uint8_t queue_bit_mask)
{
    uint32_t *basic_scheduler_table_ptr;
    uint8_t budget;

    RDD_BTRACE("dir = %d, basic_scheduler_index = %d, queue_bit_mask = %d\n",
        dir, basic_scheduler_index, queue_bit_mask);

    if (basic_scheduler_index >= RDD_BASIC_SCHEDULER_TABLE_SIZE)
    {
        return BDMF_ERR_PARM;
    }

    if (dir == rdpa_dir_ds)
        basic_scheduler_table_ptr = RDD_BASIC_SCHEDULER_TABLE_DS_ADDRESS_ARR;
    else
        basic_scheduler_table_ptr = RDD_BASIC_SCHEDULER_TABLE_US_ADDRESS_ARR;

    RDD_BASIC_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_READ_G(budget, basic_scheduler_table_ptr, basic_scheduler_index);
    budget |= queue_bit_mask;
    RDD_BASIC_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_WRITE_G(budget, basic_scheduler_table_ptr, basic_scheduler_index);

    return BDMF_ERR_OK;
}

