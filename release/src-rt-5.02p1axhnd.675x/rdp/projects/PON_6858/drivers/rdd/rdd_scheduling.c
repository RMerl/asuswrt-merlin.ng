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


#include "rdd_scheduling.h"
#include "rdp_drv_rnr.h"
#include "rdd_ghost_reporting.h"
#include "rdp_drv_dqm.h"
#include "rdp_drv_qm.h"

uint32_t exponent_list[] = {0, 3, 6, 9};

extern rdd_bb_id rdpa_emac_to_bb_id_rx[rdpa_emac__num_of];
extern rdd_bb_id rdpa_emac_to_bb_id_tx[rdpa_emac__num_of];
extern bdmf_error_t rdd_complex_scheduler_rate_set(rdpa_traffic_dir dir, uint8_t complex_scheduler_index, uint32_t block_bit_mask);
extern bdmf_error_t rdd_basic_scheduler_rate_set(rdpa_traffic_dir dir, uint8_t basic_scheduler_index, uint8_t queue_bit_mask);

void rdd_bbh_queue_init(void)
{
    uint32_t *rdd_ds_tm_bbh_queue_table_ptr;
#ifndef XRDP_BBH_PER_LAN_PORT
    uint32_t i;
#endif

    RDD_BTRACE("\n");

    rdd_ds_tm_bbh_queue_table_ptr = RDD_DS_TM_BBH_QUEUE_TABLE_ADDRESS_ARR;

#ifndef XRDP_BBH_PER_LAN_PORT
    for (i = 0; i < DS_TM_BBH_QUEUE_TABLE_SIZE; ++i)
    {
        RDD_BBH_QUEUE_DESCRIPTOR_BB_DESTINATION_WRITE_G(BB_ID_TX_LAN, rdd_ds_tm_bbh_queue_table_ptr, i);
    }
    rdd_ag_ds_tm_bb_destination_table_set(BB_ID_TX_LAN);
#else
    /* init lan0 */
#ifdef BCM6858
    RDD_BBH_QUEUE_DESCRIPTOR_BB_DESTINATION_WRITE_G(BB_ID_TX_BBH_4, rdd_ds_tm_bbh_queue_table_ptr, 0);
#else
    RDD_BBH_QUEUE_DESCRIPTOR_BB_DESTINATION_WRITE_G(BB_ID_TX_BBH_0, rdd_ds_tm_bbh_queue_table_ptr, 0);
#endif

    /* init lan1 */
    RDD_BBH_QUEUE_DESCRIPTOR_BB_DESTINATION_WRITE_G(BB_ID_TX_BBH_1, rdd_ds_tm_bbh_queue_table_ptr, 1);

    /* init lan2 */
    RDD_BBH_QUEUE_DESCRIPTOR_BB_DESTINATION_WRITE_G(BB_ID_TX_BBH_2, rdd_ds_tm_bbh_queue_table_ptr, 2);

    /* init lan3 */
    RDD_BBH_QUEUE_DESCRIPTOR_BB_DESTINATION_WRITE_G(BB_ID_TX_BBH_3, rdd_ds_tm_bbh_queue_table_ptr, 3);

    /* init lan4 */
#ifdef BCM6858
    RDD_BBH_QUEUE_DESCRIPTOR_BB_DESTINATION_WRITE_G(BB_ID_TX_BBH_0, rdd_ds_tm_bbh_queue_table_ptr, 4);
#else
    RDD_BBH_QUEUE_DESCRIPTOR_BB_DESTINATION_WRITE_G(BB_ID_TX_BBH_4, rdd_ds_tm_bbh_queue_table_ptr, 4);
#endif

    /* init lan5 */
    RDD_BBH_QUEUE_DESCRIPTOR_BB_DESTINATION_WRITE_G(BB_ID_TX_BBH_5, rdd_ds_tm_bbh_queue_table_ptr, 5);

    /* init lan6 */
    RDD_BBH_QUEUE_DESCRIPTOR_BB_DESTINATION_WRITE_G(BB_ID_TX_BBH_6, rdd_ds_tm_bbh_queue_table_ptr, 6);

#ifdef BCM6858
    /* init lan7 */
    RDD_BBH_QUEUE_DESCRIPTOR_BB_DESTINATION_WRITE_G(BB_ID_TX_BBH_7, rdd_ds_tm_bbh_queue_table_ptr, 7);
#endif
#endif
}

bdmf_error_t rdd_tm_epon_cfg(void)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    uint32_t llid;
    rdd_scheduling_queue_descriptor_t cfg = {};
    RDD_BBH_QUEUE_DESCRIPTOR_DTS *entry;

    RDD_BTRACE("\n");
    for (llid = 0; (!rc) && (llid < MAX_NUM_OF_LLID); ++llid)
    {
        cfg.bbh_queue_index = llid;
        rc = rc ? rc : rdd_scheduling_scheduler_block_cfg(rdpa_dir_us, llid + drv_qm_get_us_epon_start(), &cfg, 0, 0, 1);
        entry = ((RDD_BBH_QUEUE_DESCRIPTOR_DTS *)RDD_US_TM_BBH_QUEUE_TABLE_PTR(get_runner_idx(us_tm_runner_image))) + llid;
        RDD_BBH_QUEUE_DESCRIPTOR_BB_DESTINATION_WRITE(BB_ID_TX_PON_ETH_STAT, entry);
        RDD_BBH_QUEUE_DESCRIPTOR_HW_BBH_QID_WRITE_G(llid, RDD_US_TM_BBH_QUEUE_TABLE_ADDRESS_ARR, llid);
    }
    return rc;
}

bdmf_error_t rdd_scheduling_flush_timer_set(void)
{
    RDD_SCHEDULING_FLUSH_GLOBAL_CFG_DTS *entry;
    bdmf_error_t rc = BDMF_ERR_OK;
    uint32_t core_index;

    RDD_BTRACE("\n");
    /* flush task - ds core */

    for (core_index = 0; core_index < GROUPED_EN_SEGMENTS_NUM; core_index++)
    {
        if (IS_DS_TM_RUNNER_IMAGE(core_index))
        {
            entry = RDD_SCHEDULING_FLUSH_GLOBAL_CFG_PTR(core_index);
            RDD_BYTE_1_BITS_WRITE(FLUSH_TASK_TIMER_INTERVAL, entry);
        }
    }

#ifndef _CFE_REDUCED_XRDP_
    /* flush task - us core */
    entry = RDD_SCHEDULING_FLUSH_GLOBAL_CFG_PTR(get_runner_idx(us_tm_runner_image));
    RDD_BYTE_1_BITS_WRITE(FLUSH_TASK_TIMER_INTERVAL, entry);
#endif

    /* Make sure the timer configured before the cpu weakeup */
    WMB();

#if !defined(BCM6846) && !defined(BCM6878)
#ifndef _CFE_REDUCED_XRDP_
    rc = ag_drv_rnr_regs_cfg_cpu_wakeup_set(get_runner_idx(us_tm_runner_image), US_TM_FLUSH_THREAD_NUMBER);
#endif
    for (core_index = 0; core_index < GROUPED_EN_SEGMENTS_NUM; core_index++)
        if (IS_DS_TM_RUNNER_IMAGE(core_index))
           rc = rc ? rc : ag_drv_rnr_regs_cfg_cpu_wakeup_set(core_index, DS_TM_FLUSH_THREAD_NUMBER);
#else
    rc = ag_drv_rnr_regs_cfg_cpu_wakeup_set(get_runner_idx(us_tm_runner_image), TM_FLUSH_THREAD_NUMBER);
#endif
    return rc;
}

#ifndef _CFE_REDUCED_XRDP_
bdmf_error_t rdd_us_budget_allocation_timer_set(void)
{
    RDD_BUDGET_ALLOCATION_TIMER_VALUE_DTS *entry;
    bdmf_error_t rc = BDMF_ERR_OK;
    static uint8_t first_time = 1;

    RDD_BTRACE("\n");

    if (!first_time)
        return rc;
    
    first_time = 0;

    /* budget allocation task - us core */
    entry = RDD_BUDGET_ALLOCATION_TIMER_VALUE_PTR(get_runner_idx(us_tm_runner_image));
    RDD_BYTES_2_BITS_WRITE(US_RATE_LIMITER_TIMER_PERIOD, entry);

    /* Make sure the timer configured before the cpu weakeup */
    WMB();
#if !defined(BCM6846) && !defined(BCM6878)
    rc = ag_drv_rnr_regs_cfg_cpu_wakeup_set(get_runner_idx(us_tm_runner_image), IMAGE_3_US_TM_BUDGET_ALLOCATION_THREAD_NUMBER);
    rc = rc ? rc : ag_drv_rnr_regs_cfg_cpu_wakeup_set(get_runner_idx(us_tm_runner_image), IMAGE_3_US_TM_OVL_BUDGET_ALLOCATION_THREAD_NUMBER);
#else
    rc = ag_drv_rnr_regs_cfg_cpu_wakeup_set(get_runner_idx(us_tm_runner_image), IMAGE_0_TM_BUDGET_ALLOCATION_US_THREAD_NUMBER);
    rc = rc ? rc : ag_drv_rnr_regs_cfg_cpu_wakeup_set(get_runner_idx(us_tm_runner_image), IMAGE_0_TM_OVL_BUDGET_ALLOCATION_US_THREAD_NUMBER);
#endif
    return rc;
}
#endif

bdmf_error_t rdd_ds_budget_allocation_timer_set(void)
{
    RDD_BUDGET_ALLOCATION_TIMER_VALUE_DTS *entry;
    bdmf_error_t rc = BDMF_ERR_OK;
    uint32_t core_index;

    RDD_BTRACE("\n");
    /* budget allocation task - ds core */
    for (core_index = 0; core_index < GROUPED_EN_SEGMENTS_NUM; core_index++)
    {
        if (IS_DS_TM_RUNNER_IMAGE(core_index))
        {
            entry = RDD_BUDGET_ALLOCATION_TIMER_VALUE_PTR(core_index);
            RDD_BYTES_2_BITS_WRITE(DS_RATE_LIMITER_TIMER_PERIOD, entry);

            /* Make sure the timer configured before the cpu weakeup */
            WMB();
#if !defined(BCM6846) && !defined(BCM6878)
            rc = ag_drv_rnr_regs_cfg_cpu_wakeup_set(core_index, IMAGE_0_DS_TM_BUDGET_ALLOCATION_THREAD_NUMBER);
#else
            rc = ag_drv_rnr_regs_cfg_cpu_wakeup_set(core_index, IMAGE_0_TM_BUDGET_ALLOCATION_DS_THREAD_NUMBER);
#endif
        }
    }
    return rc;
}

/* API to RDPA level */
void rdd_set_queue_enable(uint32_t qm_queue_index, bdmf_boolean enable)
{
    uint32_t q_vec;

    RDD_BTRACE("set queue %d threshold\n", qm_queue_index);

    /* set/clear the queue in the threshold vector */
    RDD_BYTES_4_BITS_READ(q_vec, (((RDD_BYTES_4_DTS *)RDD_QUEUE_THRESHOLD_VECTOR_PTR(get_runner_idx(cpu_tx_runner_image))) +
        (qm_queue_index >> 5)));
    if (enable)
        q_vec |= (1 << (qm_queue_index & 0x1f));
    else
        q_vec &= (~(1 << (qm_queue_index & 0x1f)));
    RDD_BYTES_4_BITS_WRITE_G(q_vec, RDD_QUEUE_THRESHOLD_VECTOR_ADDRESS_ARR, (qm_queue_index >> 5));
}

bdmf_error_t rdd_scheduling_scheduler_block_cfg(rdpa_traffic_dir dir, uint8_t qm_queue_index,
    rdd_scheduling_queue_descriptor_t *scheduler_cfg, bdmf_boolean type, uint8_t dwrr_offset, bdmf_boolean enable)
{
    bdmf_error_t err;
    RDD_BTRACE("dir = %d, qm_queue_index = %d, type = %d, scheduler_cfg %p = { scheduler_index = %d, bit_mask = %x, "
        "bbh_queue = %d, scheduler_type = %d }\n",
        dir, qm_queue_index, type, scheduler_cfg,
        scheduler_cfg->scheduler_index, scheduler_cfg->queue_bit_mask, scheduler_cfg->bbh_queue_index,
        scheduler_cfg->block_type);

    if (type)
    {
        return rdd_basic_scheduler_block_cfg(dir, qm_queue_index, scheduler_cfg, dwrr_offset);
    }
    else
    {
        if (dir == rdpa_dir_ds)
        {
            if (enable)
            {       
                err = rdd_ag_ds_tm_scheduling_queue_descriptor_set(qm_queue_index - drv_qm_get_ds_start(), scheduler_cfg);
                if (!err)
                    err = rdd_ag_ds_tm_scheduling_queue_table_enable_set(qm_queue_index - drv_qm_get_ds_start(), enable);
                return err;
            }
            else
            {   
                err = rdd_ag_ds_tm_scheduling_queue_table_enable_set(qm_queue_index - drv_qm_get_ds_start(), enable);
                if (!err)
                    err = rdd_ag_ds_tm_scheduling_queue_descriptor_set(qm_queue_index - drv_qm_get_ds_start(), scheduler_cfg);
                return err;
            }
        }
#ifndef _CFE_REDUCED_XRDP_
#ifdef EPON
        rdd_ghost_reporting_mapping_queue_to_wan_channel((qm_queue_index - drv_qm_get_us_start()), scheduler_cfg->bbh_queue_index, enable);
#endif
        if (enable)
        {
            rdd_ag_us_tm_scheduling_queue_descriptor_set(qm_queue_index - drv_qm_get_us_start(), scheduler_cfg);
            return rdd_ag_us_tm_scheduling_queue_table_enable_set(qm_queue_index - drv_qm_get_us_start(), enable);
        }
        else
        {
            rdd_ag_us_tm_scheduling_queue_table_enable_set(qm_queue_index - drv_qm_get_us_start(), enable);
            return rdd_ag_us_tm_scheduling_queue_descriptor_set(qm_queue_index - drv_qm_get_us_start(), scheduler_cfg);
        }
        
#else /* #ifndef _CFE_REDUCED_XRDP_ */
        return BDMF_ERR_NOT_SUPPORTED;
#endif

    }
}

bdmf_error_t rdd_scheduling_queue_rate_limiter_remove(rdpa_traffic_dir dir, uint8_t qm_queue_index)
{
    bdmf_boolean block_type;
    uint8_t block_index, bit_mask;
    bdmf_error_t rc;
    uint32_t *rdd_tm_scheduling_queue_table_ptr;
    uint32_t queue_index;

    RDD_BTRACE("dir = %d, qm_queue_index = %d\n", dir, qm_queue_index);

    if (dir == rdpa_dir_ds)
    {
        rdd_tm_scheduling_queue_table_ptr = RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR;
        queue_index = qm_queue_index - drv_qm_get_ds_start();
    }
    else
    {
#ifndef _CFE_
        rdd_tm_scheduling_queue_table_ptr = RDD_US_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR;
        queue_index = qm_queue_index - drv_qm_get_us_start();
#else
        return BDMF_ERR_NOT_SUPPORTED;
#endif
    }


    RDD_SCHEDULING_QUEUE_DESCRIPTOR_RATE_LIMIT_ENABLE_WRITE_G(0, rdd_tm_scheduling_queue_table_ptr, queue_index);

    /* make sure the queue has budget in scheduler */
    RDD_SCHEDULING_QUEUE_DESCRIPTOR_BLOCK_TYPE_READ_G(block_type, rdd_tm_scheduling_queue_table_ptr, queue_index);
    RDD_SCHEDULING_QUEUE_DESCRIPTOR_SCHEDULER_INDEX_READ_G(block_index, rdd_tm_scheduling_queue_table_ptr, queue_index);
    RDD_SCHEDULING_QUEUE_DESCRIPTOR_QUEUE_BIT_MASK_READ_G(bit_mask, rdd_tm_scheduling_queue_table_ptr, queue_index);
    if (block_type)
        rc = rdd_complex_scheduler_rate_set(dir, block_index, (1 << bit_mask));
    else
#ifndef _CFE_REDUCED_XRDP_
        rc = rdd_basic_scheduler_rate_set(dir, block_index, bit_mask);
#else
        rc = BDMF_ERR_NOT_SUPPORTED;
#endif

    return rc;
}

static uint8_t rdd_tm_debug_sched_index_get(rdpa_traffic_dir dir, uint8_t bbh_queue)
{
    uint8_t sched_index;
    RDD_BBH_QUEUE_DESCRIPTOR_DTS *entry;

    if (dir == rdpa_dir_ds)
        entry = ((RDD_BBH_QUEUE_DESCRIPTOR_DTS *)RDD_DS_TM_BBH_QUEUE_TABLE_PTR(get_runner_idx(ds_tm_runner_image))) + bbh_queue;
#ifndef _CFE_REDUCED_XRDP_
    else
        entry = ((RDD_BBH_QUEUE_DESCRIPTOR_DTS *)RDD_US_TM_BBH_QUEUE_TABLE_PTR(get_runner_idx(us_tm_runner_image))) + bbh_queue;
#endif
    RDD_BBH_QUEUE_DESCRIPTOR_SCHEDULER_INDEX_READ(sched_index, entry);

    return sched_index;
}

static uint32_t rdd_tm_debug_basic_rl_rate_get(rdpa_traffic_dir dir, uint8_t rl_index)
{
    uint32_t man, exp;
    RDD_BASIC_RATE_LIMITER_DESCRIPTOR_DTS *entry;
    uint32_t rate_limiter_timer_period;

    if (dir == rdpa_dir_ds)
    {
        entry = ((RDD_BASIC_RATE_LIMITER_DESCRIPTOR_DTS *)RDD_BASIC_RATE_LIMITER_TABLE_DS_PTR(get_runner_idx(ds_tm_runner_image))) + rl_index;
        rate_limiter_timer_period = DS_RATE_LIMITER_TIMER_PERIOD;
    }
#ifndef _CFE_REDUCED_XRDP_
    else
    {
        entry = ((RDD_BASIC_RATE_LIMITER_DESCRIPTOR_DTS *)RDD_BASIC_RATE_LIMITER_TABLE_US_PTR(get_runner_idx(us_tm_runner_image))) + rl_index;
        rate_limiter_timer_period = US_RATE_LIMITER_TIMER_PERIOD;
    }
#endif

    RDD_BASIC_RATE_LIMITER_DESCRIPTOR_LIMIT_EXPONENT_READ(exp, entry);
    RDD_BASIC_RATE_LIMITER_DESCRIPTOR_LIMIT_MANTISSA_READ(man, entry);

    return (8 * ((man << (exp * exponent_list[1])) * rate_limiter_timer_period));
}

bdmf_boolean rdd_tm_is_cs_exist(rdpa_traffic_dir dir, uint8_t bbh_queue)
{
    bdmf_boolean sched_type;
    RDD_BBH_QUEUE_DESCRIPTOR_DTS *entry;

    if (dir == rdpa_dir_ds)
        entry = ((RDD_BBH_QUEUE_DESCRIPTOR_DTS *)RDD_DS_TM_BBH_QUEUE_TABLE_PTR(get_runner_idx(ds_tm_runner_image))) + bbh_queue;
#ifndef _CFE_REDUCED_XRDP_
    else
        entry = ((RDD_BBH_QUEUE_DESCRIPTOR_DTS *)RDD_US_TM_BBH_QUEUE_TABLE_PTR(get_runner_idx(us_tm_runner_image))) + bbh_queue;
#endif
    RDD_BBH_QUEUE_DESCRIPTOR_SCHEDULER_TYPE_READ(sched_type, entry);

    return sched_type;
}

static void rdd_tm_debug_cs_get(rdpa_traffic_dir dir, uint8_t complex_scheduler_index, rdd_tm_info *info)
{
    uint8_t budget, i, first_queue_index = 0;
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS *entry;
    RDD_SCHEDULING_QUEUE_DESCRIPTOR_DTS *base_queue_entry;
    RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS *bs_entry;
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS *cs_entry;

    memset(info, 0, sizeof(rdd_tm_info));
    if (dir == rdpa_dir_ds)
    {
        entry = ((RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS *)RDD_COMPLEX_SCHEDULER_TABLE_PTR(get_runner_idx(ds_tm_runner_image))) + complex_scheduler_index;
        base_queue_entry = ((RDD_SCHEDULING_QUEUE_DESCRIPTOR_DTS *)RDD_DS_TM_SCHEDULING_QUEUE_TABLE_PTR(get_runner_idx(ds_tm_runner_image)));
        first_queue_index = drv_qm_get_ds_start();
    }
#ifndef _CFE_REDUCED_XRDP_
    else
    {
        entry = ((RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS *)RDD_COMPLEX_SCHEDULER_TABLE_PTR(get_runner_idx(us_tm_runner_image))) + complex_scheduler_index;
        base_queue_entry = ((RDD_SCHEDULING_QUEUE_DESCRIPTOR_DTS *)RDD_US_TM_SCHEDULING_QUEUE_TABLE_PTR(get_runner_idx(us_tm_runner_image)));
    }
#endif

    info->sched_index = complex_scheduler_index;
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DWRR_OFFSET_SIR_READ(info->dwrr_offset, entry);
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_RATE_LIMIT_ENABLE_READ(info->sched_rl.rl_en, entry);
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_0_READ(budget, entry);
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_RATE_LIMITER_INDEX_READ(info->sched_rl.rl_index, entry);
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_IS_SCHEDULER_SLOT_READ(info->cs_scheduler_slot, entry);
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_IS_SCHEDULER_BASIC_READ(info->cs_scheduler_basic, entry);
    if (budget)
        info->enable = 1;
    else
        info->enable = 0;

    info->sched_rl.rl_rate = rdd_tm_debug_basic_rl_rate_get(dir, info->sched_rl.rl_index);

    for (i = 0; i < MAX_NUM_OF_QUEUES_IN_SCHED; i++)
    {
        RDD_COMPLEX_SCHEDULER_DESCRIPTOR_BLOCK_INDEX_READ(info->queue_info[i].queue_index, entry, i);

        if (!(info->cs_scheduler_slot & (1 << i)))
        {
            /* queue */
            RDD_SCHEDULING_QUEUE_DESCRIPTOR_RATE_LIMIT_ENABLE_READ(info->queue_info[i].queue_rl.rl_en, (base_queue_entry + info->queue_info[i].queue_index));
            RDD_SCHEDULING_QUEUE_DESCRIPTOR_RATE_LIMITER_INDEX_READ(info->queue_info[i].queue_rl.rl_index, (base_queue_entry + info->queue_info[i].queue_index));
            RDD_SCHEDULING_QUEUE_DESCRIPTOR_QUEUE_BIT_MASK_READ(info->queue_info[i].queue_bit_mask , (base_queue_entry + info->queue_info[i].queue_index));
            RDD_SCHEDULING_QUEUE_DESCRIPTOR_QUANTUM_NUMBER_READ(info->queue_info[i].queue_weight , (base_queue_entry + info->queue_info[i].queue_index));
            info->queue_info[i].queue_rl.rl_rate = rdd_tm_debug_basic_rl_rate_get(dir, info->queue_info[i].queue_rl.rl_index);
            info->queue_info[i].queue_index += first_queue_index;
        }
        else if (info->cs_scheduler_basic & (1 << i))
        {
            /* basic scheduler */
            if (dir == rdpa_dir_ds)
                bs_entry = ((RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS *)RDD_BASIC_SCHEDULER_TABLE_DS_PTR(get_runner_idx(ds_tm_runner_image))) + info->queue_info[i].queue_index;
#ifndef _CFE_REDUCED_XRDP_
            else
                bs_entry = ((RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS *)RDD_BASIC_SCHEDULER_TABLE_US_PTR(get_runner_idx(us_tm_runner_image))) + info->queue_info[i].queue_index;
#endif
            RDD_BASIC_SCHEDULER_DESCRIPTOR_COMPLEX_SCHEDULER_SLOT_INDEX_READ(info->queue_info[i].queue_bit_mask , bs_entry);
        }
        else
        {
            /* complex scheduler */
            if (dir == rdpa_dir_us)
                cs_entry = ((RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS *)RDD_COMPLEX_SCHEDULER_TABLE_PTR(get_runner_idx(us_tm_runner_image))) + info->queue_info[i].queue_index;
            else
                cs_entry = ((RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS *)RDD_COMPLEX_SCHEDULER_TABLE_PTR(get_runner_idx(ds_tm_runner_image))) + info->queue_info[i].queue_index;

            RDD_COMPLEX_SCHEDULER_DESCRIPTOR_PARENT_SCHEDULER_SLOT_INDEX_READ(info->queue_info[i].queue_bit_mask , cs_entry);
        }
    }
}

void rdd_tm_debug_bs_get(rdpa_traffic_dir dir, uint8_t basic_scheduler_index, rdd_tm_info *info)
{
    uint8_t budget, i, first_queue_index = 0;
    RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS *entry;
    RDD_SCHEDULING_QUEUE_DESCRIPTOR_DTS *base_queue_entry;

    memset(info, 0, sizeof(rdd_tm_info));
    if (dir == rdpa_dir_ds)
    {
        entry = ((RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS *)RDD_BASIC_SCHEDULER_TABLE_DS_PTR(get_runner_idx(ds_tm_runner_image))) + basic_scheduler_index;
        base_queue_entry = ((RDD_SCHEDULING_QUEUE_DESCRIPTOR_DTS *)RDD_DS_TM_SCHEDULING_QUEUE_TABLE_PTR(get_runner_idx(ds_tm_runner_image)));
        first_queue_index = drv_qm_get_ds_start();
    }
#ifndef _CFE_REDUCED_XRDP_
    else
    {
        entry = ((RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS *)RDD_BASIC_SCHEDULER_TABLE_US_PTR(get_runner_idx(us_tm_runner_image))) + basic_scheduler_index;
        base_queue_entry = ((RDD_SCHEDULING_QUEUE_DESCRIPTOR_DTS *)RDD_US_TM_SCHEDULING_QUEUE_TABLE_PTR(get_runner_idx(us_tm_runner_image)));
    }
#endif

    info->sched_index = basic_scheduler_index;
    RDD_BASIC_SCHEDULER_DESCRIPTOR_DWRR_OFFSET_READ(info->dwrr_offset, entry);
    RDD_BASIC_SCHEDULER_DESCRIPTOR_RATE_LIMIT_ENABLE_READ(info->sched_rl.rl_en, entry);
    RDD_BASIC_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_READ(budget, entry);
    RDD_BASIC_SCHEDULER_DESCRIPTOR_RATE_LIMITER_INDEX_READ(info->sched_rl.rl_index, entry);
    if (budget)
        info->enable = 1;
    else
        info->enable = 0;

    info->sched_rl.rl_rate = rdd_tm_debug_basic_rl_rate_get(dir, info->sched_rl.rl_index);

    for (i = 0; i < BASIC_SCHEDULER_NUM_OF_QUEUES; i++)
    {
        RDD_BASIC_SCHEDULER_DESCRIPTOR_QUEUE_INDEX_READ(info->queue_info[i].queue_index, entry, i);
        RDD_SCHEDULING_QUEUE_DESCRIPTOR_RATE_LIMIT_ENABLE_READ(info->queue_info[i].queue_rl.rl_en, (base_queue_entry + info->queue_info[i].queue_index));
        RDD_SCHEDULING_QUEUE_DESCRIPTOR_RATE_LIMITER_INDEX_READ(info->queue_info[i].queue_rl.rl_index, (base_queue_entry + info->queue_info[i].queue_index));
        RDD_SCHEDULING_QUEUE_DESCRIPTOR_QUEUE_BIT_MASK_READ(info->queue_info[i].queue_bit_mask , (base_queue_entry + info->queue_info[i].queue_index));
        RDD_SCHEDULING_QUEUE_DESCRIPTOR_QUANTUM_NUMBER_READ(info->queue_info[i].queue_weight , (base_queue_entry + info->queue_info[i].queue_index));
        info->queue_info[i].queue_rl.rl_rate = rdd_tm_debug_basic_rl_rate_get(dir, info->queue_info[i].queue_rl.rl_index);
        info->queue_info[i].queue_index += first_queue_index;
    }
}

void rdd_tm_debug_get(rdpa_traffic_dir dir, uint8_t bbh_queue, rdd_tm_info *info)
{
    RDD_BBH_QUEUE_DESCRIPTOR_DTS *entry;
    bdmf_boolean type;
    uint8_t scheduler_index = rdd_tm_debug_sched_index_get(dir, bbh_queue);

    if (dir == rdpa_dir_ds)
        entry = ((RDD_BBH_QUEUE_DESCRIPTOR_DTS *)RDD_DS_TM_BBH_QUEUE_TABLE_PTR(get_runner_idx(ds_tm_runner_image))) + bbh_queue;
#ifndef _CFE_REDUCED_XRDP_
    else
        entry = ((RDD_BBH_QUEUE_DESCRIPTOR_DTS *)RDD_US_TM_BBH_QUEUE_TABLE_PTR(get_runner_idx(us_tm_runner_image))) + bbh_queue;
#endif

    RDD_BBH_QUEUE_DESCRIPTOR_SCHEDULER_TYPE_READ(type, entry);
    if (type)
        rdd_tm_debug_cs_get(dir, scheduler_index, info);
    else
        rdd_tm_debug_bs_get(dir, scheduler_index, info);
}

#ifdef G9991
static int rdd_g9991_emac_id_to_port_mask_idx(rdpa_emac emac)
{
    RDD_BYTE_1_DTS *entry;
    uint8_t i, bb_id;
    int idx = -1;

    RDD_BTRACE("emac = %d\n", emac);

    entry = (RDD_BYTE_1_DTS *)RDD_G9991_PHYS_PORT_BB_ID_TABLE_PTR(get_runner_idx(ds_tm_runner_image));

    for (i = 0; i < RDD_G9991_SID_TO_PHYSICAL_PORT_MASK_SIZE; i++)
    {
        RDD_BYTE_1_BITS_READ(bb_id, entry + i);
        if (bb_id == (rdpa_emac_to_bb_id_tx[emac]))
            return i;
    }

    return idx;
}

int rdd_g9991_control_sid_set(rdd_rdd_vport vport, rdpa_emac emac)
{
    int mask_idx, rc;
    uint32_t sid_bit_mask;

    RDD_BTRACE("vport = %d, emac = %d\n", vport, emac);

    mask_idx = rdd_g9991_emac_id_to_port_mask_idx(emac);

    if ((vport >= RDD_VPORT_ID_32) || (mask_idx < 0))
        return BDMF_ERR_NOT_SUPPORTED;

    rc = rdd_ag_ds_tm_g9991_control_sid_table_get(mask_idx, &sid_bit_mask);

    rc = rc ? rc : rdd_ag_ds_tm_g9991_control_sid_table_set(mask_idx, sid_bit_mask | (1 << vport));

    return rc;
}

int rdd_g9991_is_control_port_get(rdd_rdd_vport vport, rdpa_emac emac, bdmf_boolean *is_control)
{
    int mask_idx, rc;
    uint32_t sid_bit_mask;

    RDD_BTRACE("vport = %d, emac = %d\n", vport, emac);

    mask_idx = rdd_g9991_emac_id_to_port_mask_idx(emac);

    if ((vport >= RDD_VPORT_ID_32) || (mask_idx < 0))
        return BDMF_ERR_NOT_SUPPORTED;

    rc = rdd_ag_ds_tm_g9991_control_sid_table_get(mask_idx, &sid_bit_mask);
    
    *is_control = ((1 << vport) & sid_bit_mask) ? 1 : 0;

    return rc;
}

int rdd_g9991_vport_to_emac_mapping_cfg(rdd_rdd_vport vport, rdpa_emac emac)
{
    RDD_BYTES_4_DTS *entry;
    uint32_t port_mask, i;
    int mask_idx;
    uint32_t core_index;

    RDD_BTRACE("vport = %d, emac = %d\n", vport, emac);

    for (core_index = 0; core_index < GROUPED_EN_SEGMENTS_NUM; core_index++)
    {
        if (IS_DS_TM_RUNNER_IMAGE(core_index))
        {
            entry = (RDD_BYTES_4_DTS *)RDD_G9991_SID_TO_PHYSICAL_PORT_MASK_PTR(core_index);

            /* TODO: currently only 32 vports are supported */
            mask_idx = rdd_g9991_emac_id_to_port_mask_idx(emac);
            if ((vport >= RDD_VPORT_ID_32) || (mask_idx < 0))
            {
                 return BDMF_ERR_NOT_SUPPORTED;
            }

            /* removes last configuration and write the new configuration of vport */
            for (i = 0; i < RDD_G9991_SID_TO_PHYSICAL_PORT_MASK_SIZE; i++)
            {
                RDD_BYTES_4_BITS_READ(port_mask, (entry + i));
                if (i == mask_idx)
                    port_mask |= (1 << vport);
                else
                    port_mask &= ~(1 << vport);
                RDD_BYTES_4_BITS_WRITE(port_mask, (entry + i));
            }
        }
    }

    RDD_INGRESS_CONGESTION_FLOW_CTRL_ENTRY_BBH_ID_WRITE_G(rdpa_emac_to_bb_id_rx[emac], RDD_INGRESS_CONGESTION_FLOW_CTRL_TABLE_ADDRESS_ARR, vport);

    return BDMF_ERR_OK;
}

uint32_t rdd_g9991_thread_number_get(rdpa_emac emac, uint32_t mask)
{
    uint32_t thread_num, bit_mask;

    bit_mask = (((1 << emac) - 1) & mask);
    thread_num = asserted_bits_count_get(bit_mask);

    return (IMAGE_0_DS_TM_FRAG0_THREAD_NUMBER + thread_num);
}

void rdd_g9991_system_port_set(rdpa_emac emac)
{
    RDD_G9991_SYSTEM_PORT_BBH_CFG_ENTRY_DTS *entry;

    entry = (RDD_G9991_SYSTEM_PORT_BBH_CFG_ENTRY_DTS *)RDD_G9991_SYSTEM_PORT_BBH_CFG_TABLE_PTR(DS_TM_CORE_BBH_0_1);

    if (emac == rdpa_emac4) {
        RDD_G9991_SYSTEM_PORT_BBH_CFG_ENTRY_BBH_TX_ID_WRITE(BB_ID_TX_BBH_0, entry);
        RDD_G9991_SYSTEM_PORT_BBH_CFG_ENTRY_BBH_ID_WRITE(BBH_ID_0, entry);
    }
    else if (emac == rdpa_emac5) {
        RDD_G9991_SYSTEM_PORT_BBH_CFG_ENTRY_BBH_TX_ID_WRITE(BB_ID_TX_BBH_5, entry);
        RDD_G9991_SYSTEM_PORT_BBH_CFG_ENTRY_BBH_ID_WRITE(BBH_ID_5, entry);
    }

    RDD_INGRESS_CONGESTION_FLOW_CTRL_ENTRY_BBH_ID_WRITE_G(rdpa_emac_to_bb_id_rx[emac], RDD_INGRESS_CONGESTION_FLOW_CTRL_TABLE_ADDRESS_ARR, RDD_VPORT_ID_29);
}

int rdd_g9991_ingress_congestion_flow_control_enable(bbh_id_e bbh_id, bdmf_boolean enable)
{
    
    RDD_INGRESS_CONGESTION_FLOW_CTRL_ENTRY_DTS *entry;
    bbh_id_e entry_bbh_id;
    uint32_t i;

    for (i = 0; i < RDD_IMAGE_4_INGRESS_CONGESTION_FLOW_CTRL_TABLE_SIZE; i++) {
        entry = ((RDD_INGRESS_CONGESTION_FLOW_CTRL_ENTRY_DTS *)RDD_INGRESS_CONGESTION_FLOW_CTRL_TABLE_PTR(get_runner_idx(processing_runner_image))) + i;

        RDD_INGRESS_CONGESTION_FLOW_CTRL_ENTRY_BBH_ID_READ(entry_bbh_id, entry);

        if (entry_bbh_id == bbh_id)
            RDD_INGRESS_CONGESTION_FLOW_CTRL_ENTRY_ENABLE_WRITE_G(enable, RDD_INGRESS_CONGESTION_FLOW_CTRL_TABLE_ADDRESS_ARR, i);
    }

    return BDMF_ERR_OK;
}

void rdd_g9991_single_fragment_enable_cfg(bdmf_boolean enable)
{
     RDD_G9991_SINGLE_FRAGMENT_CFG_ENTRY_ENABLE_WRITE_G(enable, RDD_G9991_SINGLE_FRAGMENT_CFG_TABLE_ADDRESS_ARR, 0);
}
#endif
