/*
   Copyright (c) 2015 Broadcom Corporation
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

#include "rdd_basic_scheduler.h"
#include "rdd_ag_ds_tm.h"

extern rdd_bb_id rdpa_emac_to_bb_id_tx[rdpa_emac__num_of];
extern bdmf_error_t rdd_scheduling_scheduler_block_cfg(rdpa_traffic_dir dir, uint8_t qm_queue_index, rdd_scheduling_queue_descriptor_t *scheduler_cfg, bdmf_boolean type, uint8_t dwrr_offset, bdmf_boolean enable);

/* mappping between basic scheduler to bbh queue */
static uint8_t basic_scheduler_to_bbh_queue[2][RDD_BASIC_SCHEDULER_TABLE_SIZE];

/* API to RDPA level */
bdmf_error_t rdd_basic_scheduler_cfg(rdpa_traffic_dir dir, uint8_t basic_scheduler_index, basic_scheduler_cfg_t *cfg)
{
    rdd_bbh_queue_descriptor_t bbh_queue_cfg;
    bdmf_error_t rc = BDMF_ERR_OK;
    RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS *entry;

    RDD_BTRACE("dir = %d, basic_scheduler_index = %d, cfg %p = { dwrr_offset = %d, bbh_queue_index = %d }\n",
        dir, basic_scheduler_index, cfg, cfg->dwrr_offset, cfg->bbh_queue_index);

    if ((basic_scheduler_index >= RDD_BASIC_SCHEDULER_TABLE_SIZE) ||
        (cfg->dwrr_offset >= basic_scheduler_num_of_dwrr_offset) ||
        (cfg->bbh_queue_index >= RDD_BBH_QUEUE_TABLE_SIZE))
    {
        return BDMF_ERR_PARM;
    }

    bbh_queue_cfg.scheduler_index = basic_scheduler_index;
    bbh_queue_cfg.scheduler_type = RDD_SCHED_TYPE_BASIC;
    if (dir == rdpa_dir_ds)
        entry = ((RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS *)RDD_BASIC_SCHEDULER_TABLE_DS_PTR(get_runner_idx(ds_tm_runner_image))) + basic_scheduler_index;

    /* initialize budget for all queues - relevent for the case no rate limiter was configured */
    RDD_BASIC_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_WRITE(BASIC_SCHEDULER_FULL_BUDGET_VECTOR, entry);
    RDD_BASIC_SCHEDULER_DESCRIPTOR_DWRR_OFFSET_WRITE(cfg->dwrr_offset, entry);
    RDD_BASIC_SCHEDULER_DESCRIPTOR_IS_POSITIVE_BUDGET_WRITE(1, entry);

    if (dir == rdpa_dir_ds)
    {
#ifdef XRDP_BBH_PER_LAN_PORT
        RDD_BASIC_SCHEDULER_DESCRIPTOR_BBH_QUEUE_WRITE(rdpa_emac_to_bb_id_tx[cfg->bbh_queue_index], entry);
#else
        RDD_BASIC_SCHEDULER_DESCRIPTOR_BBH_QUEUE_WRITE(cfg->bbh_queue_index, entry);
#endif
        rc = rdd_ag_ds_tm_bbh_queue_descriptor_set(cfg->bbh_queue_index, &bbh_queue_cfg);
    }
    else
    {
        rc = BDMF_ERR_NOT_SUPPORTED;
    }

    /* init bbh-queue */
    basic_scheduler_to_bbh_queue[dir][basic_scheduler_index] = cfg->bbh_queue_index;

    return rc;
}

bdmf_error_t rdd_basic_scheduler_dwrr_offset_cfg(rdpa_traffic_dir dir, uint8_t basic_scheduler_index, uint8_t dwrr_offset)
{
    RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS *entry;

    RDD_BTRACE("dir = %d, basic_scheduler_index = %d, dwrr_offset = %d }\n",
        dir, basic_scheduler_index, dwrr_offset);

    if ((basic_scheduler_index >= RDD_BASIC_SCHEDULER_TABLE_SIZE) ||
        (dwrr_offset >= basic_scheduler_num_of_dwrr_offset))
    {
        return BDMF_ERR_PARM;
    }

    if (dir == rdpa_dir_ds)
        entry = ((RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS *)RDD_BASIC_SCHEDULER_TABLE_DS_PTR(get_runner_idx(ds_tm_runner_image))) + basic_scheduler_index;

    /* initialize budget for all queues - relevent for the case no rate limiter was configured */
    RDD_BASIC_SCHEDULER_DESCRIPTOR_DWRR_OFFSET_WRITE(dwrr_offset, entry);

    return BDMF_ERR_OK;
}

bdmf_error_t rdd_basic_scheduler_queue_cfg(rdpa_traffic_dir dir, uint8_t basic_scheduler_index, basic_scheduler_queue_t *queue)
{
    RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS *entry;
    bdmf_error_t rc;
    uint8_t first_queue_index;
    rdd_scheduling_queue_descriptor_t queue_cfg;

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
        entry = ((RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS *)RDD_BASIC_SCHEDULER_TABLE_DS_PTR(get_runner_idx(ds_tm_runner_image))) + basic_scheduler_index;
        first_queue_index = QM_QUEUE_DS_START;
        rc = rdd_ag_ds_tm_scheduling_queue_table_quantum_number_set((queue->qm_queue_index - first_queue_index), queue->quantum_number);
    }

    /* mapping queue to basic scheduler */
    RDD_BASIC_SCHEDULER_DESCRIPTOR_QUEUE_INDEX_WRITE((queue->qm_queue_index - first_queue_index), entry, queue->queue_scheduler_index);

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

    /* write bit mask 0 to queue */
    RDD_BASIC_SCHEDULER_DESCRIPTOR_QUEUE_INDEX_READ(queue_index, entry, queue_scheduler_index);
    if (dir == rdpa_dir_ds)
        queue_index += QM_QUEUE_DS_START;
    rc = rdd_scheduling_scheduler_block_cfg(dir, queue_index, &queue_cfg, 0, 0, 0);

    return rc;
}

/* API to complex scheduler module */
bdmf_error_t rdd_basic_scheduler_dwrr_cfg(rdpa_traffic_dir dir, uint8_t basic_scheduler_index, uint8_t quantum_number)
{
    RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS *entry;

    if (basic_scheduler_index >= RDD_BASIC_SCHEDULER_TABLE_SIZE)
    {
        return BDMF_ERR_PARM;
    }

    if (dir == rdpa_dir_ds)
        entry = ((RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS *)RDD_BASIC_SCHEDULER_TABLE_DS_PTR(get_runner_idx(ds_tm_runner_image))) + basic_scheduler_index;

    RDD_BASIC_SCHEDULER_DESCRIPTOR_QUANTUM_NUMBER_WRITE(quantum_number, entry);

    return BDMF_ERR_OK;
}

bdmf_error_t rdd_basic_scheduler_block_cfg(rdpa_traffic_dir dir, uint8_t basic_scheduler_index,
    rdd_scheduling_queue_descriptor_t *scheduler_cfg, uint8_t dwrr_offset)
{
    RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS *entry;

    RDD_BTRACE("dir = %d, basic_scheduler_index = %d, scheduler_cfg %p = { scheduler_index = %d, bit_mask = %d, "
        "bbh_queue = %d, scheduler_type = %d, dwrr_offset = %d }\n",
        dir, basic_scheduler_index, scheduler_cfg, scheduler_cfg->scheduler_index, scheduler_cfg->queue_bit_mask,
        scheduler_cfg->bbh_queue_index, scheduler_cfg->block_type, dwrr_offset);

    if ((basic_scheduler_index >= RDD_BASIC_SCHEDULER_TABLE_SIZE) ||
        (dwrr_offset >= basic_scheduler_num_of_dwrr_offset) ||
        (scheduler_cfg->bbh_queue_index >= RDD_BBH_QUEUE_TABLE_SIZE) ||
        (scheduler_cfg->queue_bit_mask >= RDD_BASIC_SCHEDULER_TABLE_SIZE))
    {
        return BDMF_ERR_PARM;
    }

    if (dir == rdpa_dir_ds)
        entry = ((RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS *)RDD_BASIC_SCHEDULER_TABLE_DS_PTR(get_runner_idx(ds_tm_runner_image))) + basic_scheduler_index;

    /* initialize budget for all queues - relevent for the case no rate limiter was configured */
    RDD_BASIC_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_WRITE(BASIC_SCHEDULER_FULL_BUDGET_VECTOR, entry);
    RDD_BASIC_SCHEDULER_DESCRIPTOR_DWRR_OFFSET_WRITE(dwrr_offset, entry);

    /* mapping basic scheduler to complex scheduler */
    RDD_BASIC_SCHEDULER_DESCRIPTOR_COMPLEX_SCHEDULER_EXISTS_WRITE(scheduler_cfg->block_type, entry);
    RDD_BASIC_SCHEDULER_DESCRIPTOR_COMPLEX_SCHEDULER_INDEX_WRITE(scheduler_cfg->scheduler_index, entry);
    RDD_BASIC_SCHEDULER_DESCRIPTOR_COMPLEX_SCHEDULER_SLOT_INDEX_WRITE(scheduler_cfg->queue_bit_mask, entry);

    basic_scheduler_to_bbh_queue[dir][basic_scheduler_index] = scheduler_cfg->bbh_queue_index;

    return BDMF_ERR_OK;
}

