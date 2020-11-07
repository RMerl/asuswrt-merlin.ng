/*
* <:copyright-BRCM:2013-2015:proprietary:standard
*
*    Copyright (c) 2013-2015 Broadcom
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
 :>
*/
/*
 * rdpa_egress_tm_ex.c
 *
 * rdpa_egress_tm interface toward XRDP-specific RDD implementations.
 */
#include "bdmf_dev.h"
#include "rdpa_api.h"
#include "rdpa_common.h"
#include "rdpa_int.h"
#include "rdpa_types.h"
#include "rdd.h"
#include "rdpa_egress_tm_inline.h"
#include "rdpa_egress_tm_ex.h"
#include "rdd_scheduling.h"
#include "rdd_basic_scheduler.h"
#include "rdd_complex_scheduler.h"
#include "rdd_basic_rate_limiter.h"
#include "rdd_complex_rate_limiter.h"
#include "rdd_overall_rate_limiter.h"
#include "rdp_drv_dqm.h"
#include "rdp_drv_qm.h"
#include "rdp_drv_cnpl.h"
#include "rdp_drv_cntr.h"
#include "data_path_init.h"
#if !defined G9991
#include "rdd_service_queues.h"
#endif

extern dpi_params_t *p_dpi_cfg;
extern uint8_t queue_counters_idx[2];
extern uint8_t queue_counters[2][RDPA_MAX_QUEUE_COUNTERS];

/* Per-direction number of queues currently flushed */
static uint8_t flush_queue_cnt[2] = {0};


/* Per-direction rl usage bitmasks.
 * dual rate represented by two bits */
static uint64_t rdd_rl_free_mask[2][2];

/* Per-direction basic and complex scheduler usage bitmasks.
 * 0 bits represent schedulers in use, 1 represents FREE */
static uint64_t rdd_basic_schedulers_free_mask[2];
static uint64_t rdd_complex_schedulers_free_mask[2];

/* !@# temporary until the relevant constant is available */
#define RDD_COMPLEX_SCHEDULER__NUM_OF       RDD_COMPLEX_SCHEDULER_TABLE_SIZE
#define RDD_BASIC_SCHEDULER__NUM_OF         RDD_BASIC_SCHEDULER_TABLE_SIZE
#define RDD_BASIC_SCHED_ID_OFFSET           RDD_COMPLEX_SCHEDULER__NUM_OF
#define RDPA_SCHED_ID__MAX                  (RDD_COMPLEX_SCHEDULER__NUM_OF + RDD_BASIC_SCHEDULER__NUM_OF - 1)
#define RDPA_SCHED_ID_UNASSIGNED            (-1)
#define RDPA_RL_ID_UNASSIGNED               (-1)
#define RDPA_RL_FREE_MASK                   0xffffffffffffffff
#define RDPA_FLUSH_TIMEOUT                  10000 /* 10 ms */

/* weight quantum */
#define RDD_WEIGHT_QUANTUM                  256

#define QM_QUEUES__NUM_OF                   256         /* Number of QM queues managed by egress_tm */
#define QM_WRED_PROFILE__NUM_OF             14          /* Number of QM WRED profiles available for allocation.
                                                           Profile 15 is reserved for drop.
                                                           Profile 14 is reserved for CPU_RX. */

#define WRED_THRESHOLD_RESOLUTION           64          /* WRED thresholds are in units of 64 bytes */

#define RDPA_NUM_QUEUES_IN_BASIC_SCHED      8           /* Max number of queues controlled by basic scheduler */

/*
 * Per queue structure holding the "current" configuration
 */
typedef struct
{
    tm_qtm_ctl_t *qtm_ctl;
    uint32_t queue_id;
} qm_queue_info_t;

static qm_queue_info_t qm_queue_info[QM_QUEUES__NUM_OF];
static uint32_t qm_num_queues[2];                       /* Number of queues allocated per direction */
static uint32_t qm_drop_counters[QM_QUEUES__NUM_OF] = {};

static rdpa_stat_1way_t accumulative_queue_stat[QM_QUEUES__NUM_OF] = {};

static rdpa_tm_wred_profile wred_profile[QM_WRED_PROFILE__NUM_OF];
extern qm_mbr_profile mbr_profile[QM_MBR_PROFILE__NUM_OF];

/*
 * Helper functions
 */
static uint16_t _rdpa_rdd_tx_queue_calc_total_weight(struct bdmf_object *mo, tm_qtm_ctl_t *qtm_ctl);
static bdmf_error_t _rdpa_rdd_tx_queue_reconfigure_all_wrr(struct bdmf_object *mo, tm_qtm_ctl_t *qtm_ctl, uint16_t total_weight);
bdmf_index rdpa_get_available_profile_index(const rdpa_tm_queue_cfg_t *queue_cfg);
bdmf_index rdpa_get_active_profile_index(const rdpa_tm_queue_cfg_t *queue_cfg);
extern void egress_tm_delete_single_queue(struct bdmf_object *mo, tm_qtm_ctl_t *qtm_ctl, tm_channel_t *channel, tm_drv_priv_t *tm, int i);

extern tm_channel_t *egress_tm_channel_get_next(struct bdmf_object *mo, tm_channel_t *ch);
extern tm_qtm_ctl_t *egress_tm_qtm_ctl_get(struct bdmf_object *mo, tm_channel_t *channel);

static inline uint64_t mask_according_to_object_size_set(uint8_t object_size)
{
    int i;
    uint64_t dst_mask = 0;

    for (i = 0; i < object_size; i++)
    {
        dst_mask |= 1 << i;
    }
    return dst_mask;
}

static inline bdmf_error_t enable_qm_queue_once(rdp_qm_queue_idx_t q_idx)
{
    bdmf_boolean  enable;
    bdmf_error_t rc;

    rc = ag_drv_dqm_dqmol_cfgb_get(q_idx, &enable);
    if ((BDMF_ERR_OK == rc) && (!enable))
    {
       rc = drv_qm_queue_enable(q_idx);
    }
    return rc;
}

static void _rdpa_rdd_check_init(void)
{
    /* Called for the 1st time, initialize use_masks in accordance with
     * total number of available schedulers
     */
    if (rdd_basic_schedulers_free_mask[rdpa_dir_us] == 0)
    {
        rdd_basic_schedulers_free_mask[rdpa_dir_us] = mask_according_to_object_size_set(RDD_BASIC_SCHEDULER__NUM_OF);
        rdd_basic_schedulers_free_mask[rdpa_dir_ds] = mask_according_to_object_size_set(RDD_BASIC_SCHEDULER__NUM_OF);
        rdd_complex_schedulers_free_mask[rdpa_dir_us] = mask_according_to_object_size_set(RDD_COMPLEX_SCHEDULER__NUM_OF);
        rdd_complex_schedulers_free_mask[rdpa_dir_ds] = mask_according_to_object_size_set(RDD_COMPLEX_SCHEDULER__NUM_OF);
        rdd_rl_free_mask[0][rdpa_dir_ds] = RDPA_RL_FREE_MASK;
        rdd_rl_free_mask[0][rdpa_dir_us] = RDPA_RL_FREE_MASK;
        rdd_rl_free_mask[1][rdpa_dir_ds] = RDPA_RL_FREE_MASK;
        rdd_rl_free_mask[1][rdpa_dir_us] = RDPA_RL_FREE_MASK;
    }
}

/* scheduler id stored in tm_channel_t structure encodes both scheduler type and RDD scheduler id.
 * this function is responsible for mapping of rdpa_sched_id --> rdd scheduler type and id
 */
static void _rdpa_rdd_shed_id2sched_type_index(int16_t rdpa_sched_id, rdpa_rdd_sched_type_t *p_type, uint8_t *p_index)
{
    BUG_ON((unsigned)rdpa_sched_id > RDPA_SCHED_ID__MAX);
    if (rdpa_sched_id >= RDD_BASIC_SCHED_ID_OFFSET)
    {
        *p_type = RDD_SCHED_TYPE_BASIC;
        *p_index = rdpa_sched_id - RDD_BASIC_SCHED_ID_OFFSET;
    }
    else
    {
        *p_type = RDD_SCHED_TYPE_COMPLEX;
        *p_index = rdpa_sched_id;
    }
}

/* get upper index in case of tm with mode==disabled */
static uint16_t _rdpa_get_qtm_ctl_upper_index(struct bdmf_object *mo)
{
    int i;
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    tm_drv_priv_t *upper_tm = (tm_drv_priv_t *)bdmf_obj_data(tm->upper_level_tm);

    if (egress_tm_is_service_q(tm))
    {
        /* for service queues, upper index is index of best effort queue */
        for (i = 0; i < upper_tm->num_queues; i++)
        {
            if (upper_tm->queue_cfg[i].best_effort)
                return i;
        }
    }
    else if (tm->mode == rdpa_tm_sched_disabled && upper_tm->upper_level_tm)
    {
        return _rdpa_get_qtm_ctl_upper_index(tm->upper_level_tm);
    }
    else
    {
        for (i = 0; i < RDPA_TM_MAX_SCHED_ELEMENTS; i++)
        {
            if (upper_tm->sub_tms[i] == mo)
                return i;
        }
    }
    return 0;
}

/* Map RDD scheduler type and id to rdpa scheduler id */
static void _rdpa_rdd_sched_type_index2sched_id(rdpa_rdd_sched_type_t type, uint8_t index, int16_t *p_rdpa_sched_id)
{
    if (type == RDD_SCHED_TYPE_BASIC)
    {
        BUG_ON(index >= RDD_BASIC_SCHEDULER__NUM_OF);
        *p_rdpa_sched_id = index + RDD_BASIC_SCHED_ID_OFFSET;
    }
    else
    {
        BUG_ON(index >= RDD_COMPLEX_SCHEDULER__NUM_OF);
        *p_rdpa_sched_id = index;
    }
}

#ifndef HAS_FFSLL
static int ffsll(uint64_t n)
{
    uint32_t n32;
    int i;
    n32 = n & 0xffffffff;
    i = ffs(n32);
    if (!i)
    {
        n32 = (n >> 32);
        i = ffs(n32);
        if (i)
            i += 32;
    }
    return i;
}
#endif

static int16_t ffsll2(uint64_t vector)
{
    int16_t i;

    for (i = 0; i < 64; i += 2)
    {
        if ((int)((vector >> i) & 0x3) == 0x3)
            return i+1;
    }

    return 0;
}

/* Allocate scheduler index */
static bdmf_error_t _rdpa_rdd_sched_alloc(rdpa_traffic_dir dir, rdpa_rdd_sched_type_t type, int16_t *p_rdpa_sched_id)
{
    uint64_t *p_mask = (type == RDD_SCHED_TYPE_BASIC) ?
        &rdd_basic_schedulers_free_mask[dir] : &rdd_complex_schedulers_free_mask[dir];
    int index = ffsll(*p_mask);

    /* ffsll returns 1-based bit index. 0=not found */
    if (!index)
        return BDMF_ERR_NORES;
    _rdpa_rdd_sched_type_index2sched_id(type, index-1, p_rdpa_sched_id);
    *p_mask &= ~(1ULL << (index - 1));

    return BDMF_ERR_OK;
}

/* Free scheduler index */
static void _rdpa_rdd_sched_free(rdpa_traffic_dir dir, int16_t rdpa_sched_id)
{
    rdpa_rdd_sched_type_t type;
    uint8_t index;
    _rdpa_rdd_shed_id2sched_type_index(rdpa_sched_id, &type, &index);
    if (type == RDD_SCHED_TYPE_BASIC)
        rdd_basic_schedulers_free_mask[dir] |= (1 << index);
    else
        rdd_complex_schedulers_free_mask[dir] |= (1 << index);
}

/* Map egress_tm parameters to dwrr_offset */
static bdmf_error_t _rdpa_rdd_map_wrr_offset(struct bdmf_object *mo, rdpa_rdd_sched_type_t sched_type,
    rdpa_tm_sched_mode mode, rdpa_tm_num_sp_elem num_sp_elements, uint8_t *p_dwrr_offset)
{
    bdmf_error_t err = BDMF_ERR_OK;

    if (sched_type == RDD_SCHED_TYPE_BASIC)
    {
        if (mode == rdpa_tm_sched_wrr)
            *p_dwrr_offset = basic_scheduler_full_dwrr;
        else if (mode == rdpa_tm_sched_sp_wrr)
        {
            switch (num_sp_elements)
            {
            case rdpa_tm_num_sp_elem_0:
                *p_dwrr_offset = basic_scheduler_full_dwrr;
                break;
            case rdpa_tm_num_sp_elem_2:
                *p_dwrr_offset = basic_scheduler_2sp_6dwrr;
                break;
            case rdpa_tm_num_sp_elem_4:
                *p_dwrr_offset = basic_scheduler_4sp_4dwrr;
                break;
            case rdpa_tm_num_sp_elem_8:
                *p_dwrr_offset = basic_scheduler_full_sp;
                break;
            default:
                BDMF_TRACE_ERR_OBJ(mo, "Unexpected num_sp_elements = %d for basic scheduler\n", num_sp_elements);
                err = BDMF_ERR_PARM;
            }
        }
        else
            *p_dwrr_offset = basic_scheduler_full_sp;
    }
    else
    {
        /* Complex scheduler */
        if (mode == rdpa_tm_sched_wrr)
            *p_dwrr_offset = complex_scheduler_full_dwrr;
        else if (mode == rdpa_tm_sched_sp_wrr)
        {
            switch (num_sp_elements)
            {
            case rdpa_tm_num_sp_elem_0:
                *p_dwrr_offset = complex_scheduler_full_dwrr;
                break;
            case rdpa_tm_num_sp_elem_2:
                *p_dwrr_offset = complex_scheduler_2sp_30dwrr;
                break;
            case rdpa_tm_num_sp_elem_4:
                *p_dwrr_offset = complex_scheduler_4sp_28dwrr;
                break;
            case rdpa_tm_num_sp_elem_8:
                *p_dwrr_offset = complex_scheduler_8sp_24dwrr;
                break;
            case rdpa_tm_num_sp_elem_16:
                *p_dwrr_offset = complex_scheduler_16sp_16dwrr;
                break;
            case rdpa_tm_num_sp_elem_32:
                *p_dwrr_offset = complex_scheduler_full_sp;
                break;
            default:
                BDMF_TRACE_ERR_OBJ(mo, "Unexpected num_sp_elements = %d for complex scheduler\n", num_sp_elements);
                err = BDMF_ERR_PARM;
            }
        }
        else
            *p_dwrr_offset = complex_scheduler_full_sp;
    }

    return err;
}

static quantum_number_t _rdpa_rdd_calc_quantum_number(uint16_t total_weight, uint16_t sched_weight)
{
    quantum_number_t quantum_number;

    if (!total_weight || !sched_weight || sched_weight == total_weight)
        quantum_number = RDD_WEIGHT_QUANTUM - 1;
    else
        quantum_number = (total_weight > RDD_WEIGHT_QUANTUM) ? sched_weight : sched_weight * (int)(RDD_WEIGHT_QUANTUM / total_weight);

    /* minimum value for quantum is 1 */
    quantum_number = quantum_number ? quantum_number : 1;

    return quantum_number;
}


/* Destroy scheduler.
 * For now just release the index. In th efuture we might
 * have to do more cleanups on RDD level
 */
static void _rdpa_rdd_sched_destroy(tm_channel_t *channel, int16_t *p_rdpa_sched_id)
{
    if (*p_rdpa_sched_id != RDPA_SCHED_ID_UNASSIGNED)
    {
        _rdpa_rdd_sched_free(channel->dir, *p_rdpa_sched_id);
        *p_rdpa_sched_id = RDPA_SCHED_ID_UNASSIGNED;
    }
}

static uint8_t _rdpa_channel_to_hw_bbh_qid(struct bdmf_object *mo, uint8_t channel_id)
{
#if defined(CONFIG_MULTI_WAN_SUPPORT) && !defined(BCM_PON_XRDP)
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);

    switch (tm->wan_type)
    {
    case rdpa_wan_gpon:
    case rdpa_wan_xgpon:
    case rdpa_wan_epon:
    case rdpa_wan_xepon:
        return channel_id - RDD_US_CHANNEL_OFFSET_TCONT;
    case rdpa_wan_dsl:
        return channel_id - RDD_US_CHANNEL_OFFSET_DSL;
    case rdpa_wan_gbe:
        return 0;
    default:
        return channel_id;
    }
#else
    return channel_id;
#endif
}

/* Configure basic / complex scheduler */
static bdmf_error_t _rdpa_rdd_sched_create(struct bdmf_object *mo, tm_channel_t *channel,
    rdpa_rdd_sched_type_t sched_type, const rdd_sched_cfg_t *cfg, int index_in_upper,
    int16_t *p_sched_id, int16_t *p_sched_mode)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    uint8_t index = 0;
    bdmf_error_t err = BDMF_ERR_OK;
    uint8_t bbh_queue = channel->channel_id;
    uint8_t dwrr_offset;
    uint8_t hw_bbh_qid = 0;

#if !defined(BCM63158)
#if !defined(XRDP_BBH_PER_LAN_PORT)
    if ((tm->wan_type == rdpa_wan_gbe) && (channel->dir == rdpa_dir_us))
    {
        bbh_queue = rdpa_gbe_wan_emac();
    }
#elif defined(CONFIG_MULTI_WAN_SUPPORT) && defined(BCM_PON_XRDP)
    if ((tm->wan_type == rdpa_wan_gbe) && (channel->dir == rdpa_dir_us))
    {
        bbh_queue = BBH_QUEUE_WAN_1_ENTRY_ID;
    }
#endif
#endif
    if (channel->dir == rdpa_dir_us)
        hw_bbh_qid = _rdpa_channel_to_hw_bbh_qid(mo, bbh_queue);

    if (!(egress_tm_is_service_q(tm) && tm->dir == rdpa_dir_ds))
    {
        err = _rdpa_rdd_sched_alloc(channel->dir, sched_type, p_sched_id);
        BDMF_TRACE_DBG_OBJ(mo, "_rdpa_rdd_sched_alloc(%d, %d, %d) --> %s\n",
            channel->dir, sched_type, *p_sched_id, bdmf_strerror(err));
        if (err)
        {
            BDMF_TRACE_ERR_OBJ(mo, "Failed to allocate scheduler in dir %d\n", channel->dir);
            return err;
        }
        /* Recover index */
        _rdpa_rdd_shed_id2sched_type_index(*p_sched_id, &sched_type, &index);
    }

    err = _rdpa_rdd_map_wrr_offset(mo, sched_type, cfg->mode, cfg->num_sp_elements, &dwrr_offset);
    if (err)
        return err;

    if (sched_type == RDD_SCHED_TYPE_BASIC)
    {
        /* Top-level basic scheduler or sub-scheduler ? */
        if (channel->sched_id == RDPA_SCHED_ID_UNASSIGNED)
        {
            basic_scheduler_cfg_t basic_scheduler_cfg =
            {
                .dwrr_offset = dwrr_offset,
                .bbh_queue_index = bbh_queue,
                .hw_bbh_qid = hw_bbh_qid
            };
            err = rdd_basic_scheduler_cfg(channel->dir, index, &basic_scheduler_cfg);
            BDMF_TRACE_DBG_OBJ(mo, "rdd_basic_scheduler_cfg(%d, %d, %d, %d, %u) --> %s\n",
                channel->dir, index, dwrr_offset, bbh_queue, hw_bbh_qid, bdmf_strerror(err));
        }
        else
        {
            /* Basic scheduler is under complex */
            complex_scheduler_block_t sched_block =
            {
                .block_index = index,
                .scheduler_slot_index = index_in_upper,
                .bs_dwrr_offset = dwrr_offset,
                .quantum_number = 1, /* Temporary value. Will be re-assigned further in configuration flow */
                .block_type = complex_scheduler_block_bs,
            };
            rdpa_rdd_sched_type_t upper_type;
            uint8_t upper_index;
            _rdpa_rdd_shed_id2sched_type_index(channel->sched_id, &upper_type, &upper_index);
            BUG_ON(upper_type != RDD_SCHED_TYPE_COMPLEX);
            err = rdd_complex_scheduler_block_cfg(channel->dir, upper_index, &sched_block);
            BDMF_TRACE_DBG_OBJ(mo, "rdd_complex_scheduler_block_cfg(%d, %d, {%u, %u, %u}) --> %s\n",
                channel->dir, upper_index,
                sched_block.block_index, sched_block.scheduler_slot_index, sched_block.quantum_number,
                bdmf_strerror(err));
        }
        *p_sched_mode = dwrr_offset;
    }
    else
    {
        /* complex scheduler */
        complex_scheduler_cfg_t complex_scheduler_cfg =
        {
            .dwrr_offset_sir = dwrr_offset,
            .dwrr_offset_pir = complex_scheduler_full_sp,
            .bbh_queue_index = bbh_queue,
            .hw_bbh_qid = hw_bbh_qid,
            .parent_exists = (channel->sched_id != RDPA_SCHED_ID_UNASSIGNED)
        };

        if (channel->rl_rate_mode == rdpa_tm_rl_dual_rate)
        {
            complex_scheduler_cfg.dwrr_offset_sir = complex_scheduler_full_sp;
            complex_scheduler_cfg.dwrr_offset_pir = dwrr_offset;
        }

        if (egress_tm_is_service_q(tm) && tm->dir == rdpa_dir_ds)
        {
#if !defined G9991
            err = rdd_service_queue_scheduler_cfg(&complex_scheduler_cfg);
            BDMF_TRACE_DBG_OBJ(mo, "rdd_service_queue_scheduler_cfg(%d, %d) --> %s\n",
                complex_scheduler_cfg.dwrr_offset_sir, complex_scheduler_cfg.dwrr_offset_pir,
                bdmf_strerror(err));
#endif
        }
        else
        {
            err = rdd_complex_scheduler_cfg(channel->dir, index, &complex_scheduler_cfg);
            BDMF_TRACE_DBG_OBJ(mo, "rdd_complex_scheduler_cfg(%d, %d, %d, %d, %d) --> %s\n",
                channel->dir, index, complex_scheduler_cfg.dwrr_offset_sir, bbh_queue, hw_bbh_qid, bdmf_strerror(err));
        }

        /* Top-level complex scheduler or sub-scheduler ? */
        if (complex_scheduler_cfg.parent_exists)
        {
            /* Complex scheduler is under complex */
            complex_scheduler_block_t sched_block =
            {
                .block_index = index,
                .scheduler_slot_index = index_in_upper,
                .bs_dwrr_offset = dwrr_offset,
                .quantum_number = 1, /* Temporary value. Will be re-assigned further in configuration flow */
                .block_type = complex_scheduler_block_cs,
            };
            rdpa_rdd_sched_type_t upper_type;
            uint8_t upper_index;
            _rdpa_rdd_shed_id2sched_type_index(channel->sched_id, &upper_type, &upper_index);
            BUG_ON(upper_type != RDD_SCHED_TYPE_COMPLEX);
            err = rdd_complex_scheduler_block_cfg(channel->dir, upper_index, &sched_block);
            BDMF_TRACE_DBG_OBJ(mo, "rdd_complex_scheduler_block_cfg(%d, %d, {%u, %u, %u}) --> %s\n",
                channel->dir, upper_index,
                sched_block.block_index, sched_block.scheduler_slot_index, sched_block.quantum_number,
                bdmf_strerror(err));
        }

        *p_sched_mode = complex_scheduler_cfg.dwrr_offset_sir;
    }

    if (err)
    {
        BDMF_TRACE_ERR_OBJ(mo, "Failed to configure scheduler type %d in dir %d\n", sched_type, channel->dir);
        _rdpa_rdd_sched_destroy(channel, p_sched_id);
    }

    return err;
}

/*
 * RL configuration
 */

/* Allocate rl index */
static bdmf_error_t _rdpa_rdd_rl_alloc(struct bdmf_object *mo, rdpa_traffic_dir dir, bdmf_boolean is_sched_level,
    rdpa_tm_rl_rate_mode rate_mode, uint16_t qm_queue_index, int16_t *p_rdpa_rc_id)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    uint64_t *p_mask;
    int vec, index = 0;

    if (egress_tm_is_service_q(tm) && dir == rdpa_dir_ds)
    {
        if (is_sched_level)
            *p_rdpa_rc_id = QM_QUEUE_SERVICE_Q_MAX_QUANTITY;
        else
            *p_rdpa_rc_id = qm_queue_index - drv_qm_get_sq_start();
        return BDMF_ERR_OK;
    }

    if (rate_mode == rdpa_tm_rl_single_rate)
    {
        /* ffsll returns 1-based bit index. 0=not found */
        for (vec = 0; vec < 2; vec++)
        {
            p_mask = &rdd_rl_free_mask[vec][dir];
            index = ffsll(*p_mask);
            if (index)
                break;
        }
    }
    else
    {
        /* ffsll2 returns 1-based bit index of first two bits. 0=not found */
        for (vec = 0; vec < 2; vec++)
        {
            p_mask = &rdd_rl_free_mask[vec][dir];
            index = ffsll2(*p_mask);
            if (index)
                break;
        }
    }

    if (!index)
        return BDMF_ERR_TOO_MANY; /* BDMF_ERR_TOO_MANY will indicated reshaper allocation error */
    index--;

    if (vec)
        *p_rdpa_rc_id = index + 64;
    else
        *p_rdpa_rc_id = index;

    *p_mask &= ~(1ULL << index);
    if (rate_mode == rdpa_tm_rl_dual_rate)
        *p_mask &= ~(1ULL << (index + 1));

    return BDMF_ERR_OK;
}

/* Free rl index */
static void _rdpa_rdd_rl_free(struct bdmf_object *mo, rdpa_traffic_dir dir, bdmf_boolean is_sched_level,
    int16_t rdpa_rl_id, rdpa_tm_rl_rate_mode type)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);

    if (egress_tm_is_service_q(tm) && dir == rdpa_dir_ds)
    {
#if !defined G9991
        if (is_sched_level)
            rdd_service_queues_basic_rate_limiter_remove(rdpa_rl_id);
        else
            rdd_service_queue_rate_limiter_remove(rdpa_rl_id);
#endif
    }
    else if (type == rdpa_tm_rl_single_rate)
    {
        rdd_basic_rate_limiter_remove(dir, rdpa_rl_id);
        if (rdpa_rl_id < 64)
            rdd_rl_free_mask[0][dir] |= (1ULL << rdpa_rl_id);
        else
            rdd_rl_free_mask[1][dir] |= (1ULL << rdpa_rl_id);
    }
    else
    {
        rdd_complex_rate_limiter_remove(dir, rdpa_rl_id);
        if (rdpa_rl_id < 64)
        {
            rdd_rl_free_mask[0][dir] |= (1ULL << rdpa_rl_id);
            rdd_rl_free_mask[0][dir] |= (1ULL << (rdpa_rl_id + 1));
        }
        else
        {
            rdd_rl_free_mask[1][dir] |= (1ULL << rdpa_rl_id);
            rdd_rl_free_mask[1][dir] |= (1ULL << (rdpa_rl_id + 1));
        }
    }
}

/* Destroy rate controller.
 * For now just release the index. In the future we might
 * have to do more cleanups on RDD level
 */
static void _rdpa_rdd_rl_destroy(struct bdmf_object *mo, rdpa_traffic_dir dir, bdmf_boolean is_sched_level,
    rdpa_tm_rl_rate_mode type, int16_t *p_rdpa_rc_id)
{
    if (*p_rdpa_rc_id == RDPA_RL_ID_UNASSIGNED)
        return;

    _rdpa_rdd_rl_free(mo, dir, is_sched_level, *p_rdpa_rc_id, type);
    *p_rdpa_rc_id = RDPA_RL_ID_UNASSIGNED;
}

/* Configure basic / complex rate limiter */
static bdmf_error_t _rdpa_rdd_rl_configure(struct bdmf_object *mo, rdpa_traffic_dir dir, int16_t rdpa_rc_id,
    bdmf_boolean is_sched_level, int16_t qm_queue_index, rdpa_tm_rl_rate_mode rl_rate_mode, const rdpa_tm_rl_cfg_t *cfg)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    uint8_t block_t, index = 0;
    bdmf_error_t err = BDMF_ERR_OK;

    /* Determine rate controller type */
    if (egress_tm_is_service_q(tm) && dir == rdpa_dir_ds)
    {
        if (is_sched_level)
        {
            block_t = rdd_basic_rl_complex_scheduler;
            index = 0;
        }
        else
        {
            block_t = rdd_complex_rl_queue;
            index = qm_queue_index - drv_qm_get_sq_start();
            rl_rate_mode = rdpa_tm_rl_dual_rate;
        }
    }
    else if (!is_sched_level)
    {
        block_t = rdd_basic_rl_queue;
        index = qm_queue_index;
    }
    else
    {
        rdpa_rdd_sched_type_t type = 0;
        /* block index contains sched_id. map to sched type and index */
        _rdpa_rdd_shed_id2sched_type_index(qm_queue_index, &type, &index);
        block_t = (type == RDD_SCHED_TYPE_BASIC) ? rdd_basic_rl_basic_scheduler : rdd_basic_rl_complex_scheduler;
        if ((block_t > rdd_basic_rl_basic_scheduler) && (rl_rate_mode == rdpa_tm_rl_dual_rate))
            return BDMF_ERR_NORES;
    }

    if (rl_rate_mode == rdpa_tm_rl_single_rate)
    {
        /* Basic rl */
        rdd_basic_rl_cfg_t rl_cfg =
        {
            .rate = cfg->af_rate / BITS_IN_BYTE,
            .limit = cfg->burst_size,
            .type = block_t,
            .block_index = index
        };
        if (egress_tm_is_service_q(tm) && dir == rdpa_dir_ds)
        {
#if !defined G9991
            err = rdd_service_queues_basic_rate_limiter_cfg(rdpa_rc_id, &rl_cfg);
            BDMF_TRACE_DBG_OBJ(mo, "rdd_service_queues_basic_rate_limiter_cfg(%d, {%u, %u, %u})\n",
                rdpa_rc_id, rl_cfg.rate, rl_cfg.type, rl_cfg.block_index);
#endif
        }
        else
        {
            err = rdd_basic_rate_limiter_cfg(dir, rdpa_rc_id, &rl_cfg);
            BDMF_TRACE_DBG_OBJ(mo, "rdd_basic_rate_limiter_cfg(%d, %d, {%u, %u, %u}) --> %s\n",
                dir, rdpa_rc_id, rl_cfg.rate, rl_cfg.type, rl_cfg.block_index, bdmf_strerror(err));
        }
    }
    else
    {
        /* Complex rl */
        rdd_complex_rl_cfg_t rl_cfg =
        {
            .sustain_budget = cfg->af_rate / BITS_IN_BYTE,
            .peak_limit = cfg->burst_size,
            .peak_rate = cfg->be_rate / BITS_IN_BYTE,
            .type = block_t,
            .block_index = index
        };
        if (egress_tm_is_service_q(tm) && dir == rdpa_dir_ds)
        {
#if !defined G9991
            err = rdd_service_queue_rate_limiter_cfg(rdpa_rc_id, &rl_cfg);
            BDMF_TRACE_DBG_OBJ(mo, "rdd_service_queue_rate_limiter_cfg(%d, {%u, %u, %u, %u}))\n",
                rdpa_rc_id, rl_cfg.sustain_budget, rl_cfg.peak_limit, rl_cfg.peak_rate, rl_cfg.block_index);
#endif
        }
        else
        {
            err = rdd_complex_rate_limiter_cfg(dir, rdpa_rc_id, &rl_cfg);
            BDMF_TRACE_DBG_OBJ(mo, "rdd_complex_rate_limiter_cfg(%d, %d, {%u, %u, %u, %u}) --> %s\n",
                dir, rdpa_rc_id, rl_cfg.sustain_budget, rl_cfg.peak_limit, rl_cfg.peak_rate,
                rl_cfg.block_index, bdmf_strerror(err));
        }
    }

    return err;
}

/* Create and configure basic / complex rate limiter */
static bdmf_error_t _rdpa_rdd_rl_create(struct bdmf_object *mo, rdpa_traffic_dir dir, bdmf_boolean is_sched_level,
    int16_t qm_queue_index, rdpa_tm_rl_rate_mode rl_rate_mode, const rdpa_tm_rl_cfg_t *cfg, int16_t *p_rdpa_rc_id)
{
    bdmf_error_t err;

    /* Allocate RC index */
    err = _rdpa_rdd_rl_alloc(mo, dir, is_sched_level, rl_rate_mode, qm_queue_index, p_rdpa_rc_id);
    if (err)
        return err;

    /* Configure rate controller */
    err = _rdpa_rdd_rl_configure(mo, dir, *p_rdpa_rc_id, is_sched_level, qm_queue_index, rl_rate_mode, cfg);
    if (err)
        _rdpa_rdd_rl_destroy(mo, dir, is_sched_level, rl_rate_mode, p_rdpa_rc_id);

    return err;
}

/* Create / destroy / configure basic / complex rate limiter */
static bdmf_error_t _rdpa_rdd_rl_reconfigure(struct bdmf_object *mo, rdpa_traffic_dir dir,
    bdmf_boolean is_sched_level, int16_t qm_queue_index, rdpa_tm_rl_rate_mode rl_rate_mode,
    const rdpa_tm_rl_cfg_t *cfg, int16_t *p_rdpa_rc_id)
{
    bdmf_error_t err = BDMF_ERR_OK;

    if (cfg->af_rate >= RDD_RATE_UNLIMITED || cfg->af_rate <= 0)
        _rdpa_rdd_rl_destroy(mo, dir, is_sched_level, rl_rate_mode, p_rdpa_rc_id);
    else if (*p_rdpa_rc_id == RDPA_RL_ID_UNASSIGNED)
        err = _rdpa_rdd_rl_create(mo, dir, is_sched_level, qm_queue_index, rl_rate_mode, cfg, p_rdpa_rc_id);
    else
        err = _rdpa_rdd_rl_configure(mo, dir, *p_rdpa_rc_id, is_sched_level, qm_queue_index, rl_rate_mode, cfg);

    return err;
}

/* Get rdd QM queue index */
bdmf_index _rdpa_rdd_get_queue_idx(tm_channel_t *channel, tm_qtm_ctl_t *qtm_ctl)
{
    struct bdmf_object *mo = channel->egress_tm;
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    port_drv_priv_t *port;
    int first;
    int last;
    int i;

    rdpa_system_init_cfg_t *sys_init_cfg = (rdpa_system_init_cfg_t *)_rdpa_system_init_cfg_get();

    port = (port_drv_priv_t *)bdmf_obj_data(channel->owner);

    if (channel->dir == rdpa_dir_us)
    {
        first = drv_qm_get_us_start();
        last = drv_qm_get_us_end();
    }
    else if (tm && egress_tm_is_service_q(tm))
    {
        first = drv_qm_get_sq_start();
        last = drv_qm_get_sq_end();
    }
    else if (!sys_init_cfg->dpu_split_scheduling_mode)
    {
        first = drv_qm_get_ds_start();
        last = drv_qm_get_ds_end();
    }
    else
    {
        if (port->cfg.emac == rdpa_emac0 || port->cfg.emac == rdpa_emac1 || port->cfg.emac == rdpa_emac5)
        {
            first = drv_qm_get_ds_start();
            last = drv_qm_get_ds_start() + (drv_qm_get_ds_end() - drv_qm_get_ds_start()) / 2;
        }
        else
        {
            first = drv_qm_get_ds_start() + (drv_qm_get_ds_end() - drv_qm_get_ds_start()) / 2 + 1;
            last = drv_qm_get_ds_end();
        }
    }

    for (i = first; i <= last; i++)
    {
        if (qm_queue_info[i].qtm_ctl == NULL)
        {
            return i;
        }
    }
    return BDMF_INDEX_UNASSIGNED;
}

/* Allocate QM queue index */
static bdmf_error_t _rdpa_rdd_queue_alloc(tm_channel_t *channel, tm_qtm_ctl_t *qtm_ctl, tm_queue_hash_entry_t *qentry, int *q_idx)
{
    bdmf_index queue_index;

    queue_index = _rdpa_rdd_get_queue_idx(channel, qtm_ctl);
    if (queue_index != BDMF_INDEX_UNASSIGNED)
    {
        *q_idx = queue_index;
        ++qm_num_queues[channel->dir];
        qm_queue_info[queue_index].qtm_ctl = qtm_ctl;
        qm_queue_info[queue_index].queue_id = qentry->queue_id;
        BDMF_TRACE_DBG("allocate queue index =%d\n", (int)queue_index);
        return BDMF_ERR_OK;
    }
    return BDMF_ERR_NORES;
}

/* Release QM queue index */
static void _rdpa_rdd_queue_free(rdpa_traffic_dir dir, int q_idx)
{
    if ((unsigned)q_idx >= QM_QUEUES__NUM_OF)
    {
        BDMF_TRACE_ERR("error in free queue, queue_index = %d\n", q_idx);
        return;
    }
    if (qm_queue_info[q_idx].qtm_ctl == NULL)
    {
        BDMF_TRACE_ERR("error in free queue, qm_queue_info[q_idx].qtm_ctl is NULL\n");
        return;
    }
    qm_queue_info[q_idx].qtm_ctl = NULL;
    qm_queue_info[q_idx].queue_id = (uint32_t)BDMF_INDEX_UNASSIGNED;
    --qm_num_queues[dir];
    BDMF_TRACE_DBG("free queue index =%d\n", q_idx);
}

/* Re-configure weights and DWRR mode for all blocks of a complex scheduler */
static bdmf_error_t _rdpa_rdd_set_sched_weights(struct bdmf_object *mo, tm_channel_t *channel)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    uint8_t bbh_index = channel->channel_id;
    tm_qtm_ctl_t *qtm_ctl;
    rdpa_rdd_sched_type_t sched_type, sub_type;
    uint8_t sched_index, sub_index;
    uint32_t total_weight = 0;
    uint8_t hw_bbh_qid = 0;
    bdmf_error_t err = 0;

#ifndef XRDP_BBH_PER_LAN_PORT
#if !defined(BCM63158)
    if (rdpa_is_gbe_mode() && channel->dir == rdpa_dir_us)
        bbh_index = rdpa_gbe_wan_emac();
#endif
#endif
    if (channel->dir == rdpa_dir_us)
        hw_bbh_qid = _rdpa_channel_to_hw_bbh_qid(mo, bbh_index);

    if (channel->sched_mode == complex_scheduler_full_sp)
        return BDMF_ERR_OK;

    /* Get RDD scheduler type and index */
    if (egress_tm_is_service_q(tm) && tm->dir == rdpa_dir_ds)
    {
        sched_index = RDPA_SCHED_ID_UNASSIGNED;
        sched_type = RDD_SCHED_TYPE_COMPLEX;
    }
    else
    {
        _rdpa_rdd_shed_id2sched_type_index(channel->sched_id, &sched_type, &sched_index);
    }

    /* Go over all sub-schedulers */
    STAILQ_FOREACH(qtm_ctl, &channel->qtm_ctls, list)
    {
        if (qtm_ctl->sched_id == RDPA_SCHED_ID_UNASSIGNED)
            continue;
        total_weight += qtm_ctl->sched_weight;
    }

    STAILQ_FOREACH(qtm_ctl, &channel->qtm_ctls, list)
    {
        complex_scheduler_block_t sched_block = {};

        if (qtm_ctl->sched_id == RDPA_SCHED_ID_UNASSIGNED)
            continue;

        _rdpa_rdd_shed_id2sched_type_index(qtm_ctl->sched_id, &sub_type, &sub_index);
        sched_block.block_index = sub_index;
        sched_block.scheduler_slot_index = qtm_ctl->sched_index_in_upper;
        sched_block.quantum_number = _rdpa_rdd_calc_quantum_number(total_weight, qtm_ctl->sched_weight);
        sched_block.bs_dwrr_offset = qtm_ctl->sched_mode;
        sched_block.block_type = complex_scheduler_block_bs;
        err = rdd_complex_scheduler_block_cfg(channel->dir, sched_index, &sched_block);
        BDMF_TRACE_DBG_OBJ(mo, "rdd_complex_scheduler_block_cfg(%d, %d, {%d, %d, %d}) --> %s\n",
            channel->dir, sched_index,
            sched_block.block_index, sched_block.scheduler_slot_index, sched_block.quantum_number,
            bdmf_strerror(err));
    }

    if (!err)
    {
        complex_scheduler_cfg_t complex_scheduler_cfg =
        {
            .dwrr_offset_sir = channel->sched_mode,
            .dwrr_offset_pir = complex_scheduler_full_sp,
            .bbh_queue_index = bbh_index,
            .hw_bbh_qid = hw_bbh_qid,
        };
        if (channel->rl_rate_mode == rdpa_tm_rl_dual_rate)
        {
            complex_scheduler_cfg.dwrr_offset_sir = complex_scheduler_full_sp;
            complex_scheduler_cfg.dwrr_offset_pir = channel->sched_mode;
        }

        err = rdd_complex_scheduler_cfg(channel->dir, sched_index, &complex_scheduler_cfg);
        BDMF_TRACE_DBG_OBJ(mo, "rdd_complex_scheduler_cfg(%d, %d, %d, %d) --> %s\n",
            channel->dir, sched_index, channel->sched_mode, bbh_index, bdmf_strerror(err));
    }

    return err;
}


static bdmf_boolean is_wrr_elem(struct bdmf_object *mo, rdpa_tm_rl_rate_mode rate_mode,
    int16_t sched_id, int16_t sched_mode, int elem_index)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_rdd_sched_type_t sched_type;
    uint8_t sched_index;
    bdmf_boolean is_wrr = 0;

    if (egress_tm_is_service_q(tm) && tm->dir == rdpa_dir_ds)
    {
        sched_type = RDD_SCHED_TYPE_COMPLEX;
    }
    else
    {
        if ((unsigned)sched_id > RDPA_SCHED_ID__MAX)
            return 0;

        _rdpa_rdd_shed_id2sched_type_index(sched_id, &sched_type, &sched_index);
    }

    if (sched_type == RDD_SCHED_TYPE_BASIC)
    {
        if (sched_mode == basic_scheduler_full_dwrr                             ||
            (sched_mode == basic_scheduler_2sp_6dwrr && elem_index >= 2)        ||
            (sched_mode == basic_scheduler_4sp_4dwrr && elem_index >= 4))
        {
            is_wrr = 1;
        }
    }
    else
    {
        if (sched_mode == complex_scheduler_full_dwrr                           ||
            (sched_mode == complex_scheduler_2sp_30dwrr && elem_index >= 2)     ||
            (sched_mode == complex_scheduler_4sp_28dwrr && elem_index >= 4)     ||
            (sched_mode == complex_scheduler_8sp_24dwrr && elem_index >= 8)     ||
            (sched_mode == complex_scheduler_16sp_16dwrr && elem_index >= 16))
        {
            is_wrr = 1;
        }
    }
    return is_wrr;
}

static int is_wan_aggregation_disable(rdpa_wan_type wan_type)
{
#if !defined(BCM63158)
    wan_type = rdpa_wan_if_to_wan_type(rdpa_wan_type_to_if(rdpa_wan_epon));
    /* NOTE : Using rdpa_wan_epon based on below code which is looking for epon only related stuff */

    return (rdpa_is_epon_ae_mode() && (rdpa_wan_speed_get(rdpa_wan_type_to_if(wan_type)) == rdpa_speed_1g)) || (wan_type == rdpa_wan_epon);
#else
    return (wan_type == rdpa_wan_dsl) || (wan_type == rdpa_wan_epon);
#endif
}

/* Set QM queue context */
static void _rdpa_qm_queue_ctx_set(rdpa_wan_type wan_type, const tm_channel_t *channel,
    int queue_index, const rdd_queue_cfg_t *queue_cfg, qm_q_context *qm_queue_ctx)
{
    /* Basic queue profile is pre-set in data_path_init.
     * We only update WRED profile here
     */
    bdmf_error_t err;

    /* Setup queue profile */
    err = ag_drv_qm_q_context_get(queue_index, qm_queue_ctx);
    BUG_ON(err != BDMF_ERR_OK);

    qm_queue_ctx->wred_profile = queue_cfg->profile;
    if (queue_cfg->mbr_profile != RDD_QUEUE_PROFILE_DISABLED)
        qm_queue_ctx->res_profile = queue_cfg->mbr_profile;
    qm_queue_ctx->fec_enable = channel->attr.fec_overhead;
    qm_queue_ctx->sci = channel->attr.sci_overhead;
    qm_queue_ctx->q_802_1ae = channel->attr.q_802_1ae;
    qm_queue_ctx->exclusive_priority = queue_cfg->exclusive;

    /* US queue aggregation change */
    if (queue_index >= drv_qm_get_us_start() && queue_index <= drv_qm_get_us_end())
    {
        qm_queue_ctx->aggregation_disable = is_wan_aggregation_disable(wan_type);
        BDMF_TRACE_INFO("QM queue index %d - aggregation_disable %d\n", queue_index, qm_queue_ctx->aggregation_disable);
    }
#ifdef G9991
    else if ((queue_index >= drv_qm_get_ds_start()) && (queue_index <= drv_qm_get_ds_end()))
    {
        bdmf_boolean is_control;
        port_drv_priv_t *port;

        /* If DS control queue set as exclusive queue */
        port = (port_drv_priv_t *)bdmf_obj_data(channel->owner);

        qm_queue_ctx->exclusive_priority = 0;
        if (p_dpi_cfg->g9991_port_vec & (1 << port->cfg.emac))
        {
            /* vport should be retrieved by rdpa_port_rdpa_if_to_vport() But symmetry is kept to rdd_g9991_control_sid_set
               usage in rdpa_port_ex.c */
            err = rdd_g9991_is_control_port_get((port->index - rdpa_if_lan0), (port->cfg.emac - rdpa_emac0), &is_control);
            BUG_ON(err != BDMF_ERR_OK);

            if (is_control)
                qm_queue_ctx->exclusive_priority = 1;
        }
    }
#endif
}

int egress_tm_post_init_ex(struct bdmf_object *mo)
{
    return BDMF_ERR_OK;
}

bdmf_error_t rdpa_rdd_tx_queue_channel_attr_update(const channel_attr *attr, int queue_index)
{
    bdmf_error_t err;
    qm_q_context qm_queue_ctx;

    /* Setup queue profile */
    err = drv_qm_queue_get_config(queue_index, &qm_queue_ctx);
    if (err)
        return err;

    if ((qm_queue_ctx.fec_enable == attr->fec_overhead) && (qm_queue_ctx.sci == attr->sci_overhead) &&
        (qm_queue_ctx.q_802_1ae == attr->q_802_1ae))
        return BDMF_ERR_OK;

    qm_queue_ctx.fec_enable = attr->fec_overhead;
    qm_queue_ctx.sci = attr->sci_overhead;
    qm_queue_ctx.q_802_1ae = attr->q_802_1ae;

    err = drv_qm_queue_config(queue_index, &qm_queue_ctx);

    return err;
}

/*
 * rdpa_egress_tm_ex interface
 */

/*
 * Channel configuration
 */

/* Destroy top-level scheduler */
void rdpa_rdd_top_sched_destroy(struct bdmf_object *mo, tm_channel_t *channel)
{
    _rdpa_rdd_sched_destroy(channel, &channel->sched_id);
    _rdpa_rdd_rl_destroy(mo, channel->dir, 1, rdpa_tm_rl_single_rate, &channel->rc_id);
}

/* Create top-level scheduler */
bdmf_error_t rdpa_rdd_top_sched_create(struct bdmf_object *mo, tm_channel_t *channel, const rdd_sched_cfg_t *cfg)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_error_t err = BDMF_ERR_OK;
    int16_t sched_id = RDPA_SCHED_ID_UNASSIGNED;
    rdpa_rdd_sched_type_t sched_type;

    _rdpa_rdd_check_init();

    channel->sched_id = RDPA_SCHED_ID_UNASSIGNED;
    channel->rc_id = RDPA_RL_ID_UNASSIGNED;

    /* We allocate complex scheduler if at least 1 of the following conditions is true
     * - egress_tm level (as opposed to q-level)
     * - more than 8 queues for q-level egress_tm
     * - rate_mode = dual
     * - is service queue
     */
    if (cfg->level == rdpa_tm_level_egress_tm                   ||
        cfg->num_queues > RDPA_NUM_QUEUES_IN_BASIC_SCHED        ||
        cfg->rl_rate_mode == rdpa_tm_rl_dual_rate               ||
        egress_tm_is_service_q(tm))
    {
        sched_type = RDD_SCHED_TYPE_COMPLEX;
    }
    else
    {
        sched_type = RDD_SCHED_TYPE_BASIC;
    }

    err = _rdpa_rdd_sched_create(mo, channel, sched_type,
        cfg, RDPA_SCHED_ID_UNASSIGNED, &sched_id, &channel->sched_mode);
    if (err)
        return err;

    channel->sched_id = sched_id;
    channel->rl_rate_mode = cfg->rl_rate_mode;

    /* in DS need to allocate RL for LAN port if egress_tm is tm-level.
     * if it is q-level, rdpa_rdd_qtm_ctl_create is about to be called.
     */
    if (channel->dir == rdpa_dir_ds && cfg->level == rdpa_tm_level_egress_tm)
    {
        if (cfg->rl_rate_mode != rdpa_tm_rl_single_rate)
            BDMF_TRACE_RET_OBJ(BDMF_ERR_NOT_SUPPORTED, mo, "DS dual_mode rate limiter is not supported\n");

        if ((cfg->rl_cfg.af_rate < RDD_RATE_UNLIMITED) && (cfg->rl_cfg.af_rate > 0))
        {
            err = _rdpa_rdd_rl_create(mo, channel->dir, 1, channel->sched_id,
                rdpa_tm_rl_single_rate, &cfg->rl_cfg, &channel->rc_id);
            if (err)
            {
                rdpa_rdd_top_sched_destroy(mo, channel);
                return err;
            }
        }
    }

    return err;
}

/* Create & configure q-level TM */
bdmf_error_t rdpa_rdd_qtm_ctl_create(struct bdmf_object *mo, tm_qtm_ctl_t *qtm_ctl, const rdd_sched_cfg_t *cfg)
{
    tm_channel_t *channel = qtm_ctl->channel;
    bdmf_error_t err = BDMF_ERR_OK;
    bdmf_boolean is_wrr;
    uint16_t upper_idx = qtm_ctl->sched_index_in_upper;
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_tm_rl_rate_mode rl_rate_mode;
    uint32_t sched_id;

    if (tm->mode == rdpa_tm_sched_disabled && tm->upper_level_tm)
        upper_idx = _rdpa_get_qtm_ctl_upper_index(mo);

    BUG_ON(!qtm_ctl);
    if (!channel)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo, "qm_ctl->channel isn't set\n");

    BUG_ON(qtm_ctl->rc_id != RDPA_RL_ID_UNASSIGNED);
    qtm_ctl->sched_id = RDPA_SCHED_ID_UNASSIGNED;

    /* Allocate basic scheduler if isn't allocated yet */
    if (qtm_ctl->egress_tm != channel->egress_tm)
    {
        if (egress_tm_is_service_q(tm))
        {
            upper_idx = _rdpa_get_qtm_ctl_upper_index(mo);

            err = _rdpa_rdd_sched_create(mo, channel, RDD_SCHED_TYPE_COMPLEX,
                cfg, upper_idx, &qtm_ctl->sched_id, &qtm_ctl->sched_mode);
            if (err)
                return err;
        }
        else
        {
            if (cfg->num_queues > RDPA_NUM_QUEUES_IN_BASIC_SCHED)
            {
                BDMF_TRACE_RET_OBJ(BDMF_ERR_RANGE, mo,
                    "2nd level egress_tm objects supports up to %u queues\n", RDPA_NUM_QUEUES_IN_BASIC_SCHED);
            }
            err = _rdpa_rdd_sched_create(mo, channel, RDD_SCHED_TYPE_BASIC,
                cfg, upper_idx, &qtm_ctl->sched_id, &qtm_ctl->sched_mode);
            if (err)
                return err;
        }

        /* If upper-level complex scheduler is DWRR - recalculate all weights */
        is_wrr = is_wrr_elem(mo, channel->rl_rate_mode, channel->sched_id, channel->sched_mode,
            upper_idx);
        qtm_ctl->sched_weight = cfg->weight;

        if (is_wrr)
        {
            err = _rdpa_rdd_set_sched_weights(mo, channel);
            if (err)
            {
                _rdpa_rdd_sched_destroy(channel, &qtm_ctl->sched_id);
                return err;
            }
        }
    }

    /* create / re-configure rl if necessary.
     * Assign it in the same structure where scheduler is allocated.
     * In the legacy configuration in 2-tier hierarchy, dual rate is configured
     * in top-level egress_tm rather than 2nd level egress_tm that actually performs
     * the rate limiting.
     * XRDP implementation supports both options: dual rate can be configured at
     * top level or at the 2nd level. The actual result should be the same
     */
    qtm_ctl->rl_rate_mode = (channel->rl_rate_mode == rdpa_tm_rl_dual_rate) ?
        rdpa_tm_rl_dual_rate : cfg->rl_rate_mode;

    sched_id = (qtm_ctl->sched_id == RDPA_SCHED_ID_UNASSIGNED) ?
        channel->sched_id : qtm_ctl->sched_id;

    rl_rate_mode = (qtm_ctl->sched_id == RDPA_SCHED_ID_UNASSIGNED) ?
        rdpa_tm_rl_single_rate : qtm_ctl->rl_rate_mode;

    /* always use single rate limiter on overall DS service queues */
    if (egress_tm_is_service_q(tm) && tm->dir == rdpa_dir_ds)
        rl_rate_mode = rdpa_tm_rl_single_rate;

    return _rdpa_rdd_rl_reconfigure(mo, channel->dir, 1, sched_id,
               rl_rate_mode, &cfg->rl_cfg, &qtm_ctl->rc_id);
}

void rdpa_rdd_qtm_ctl_destroy(struct bdmf_object *mo, tm_qtm_ctl_t *qtm_ctl)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    tm_channel_t *channel = qtm_ctl->channel;

    BUG_ON(!qtm_ctl);
    if (!channel)
    {
        BDMF_TRACE_ERR_OBJ(mo, "qm_ctl->channel isn't set\n");
        return;
    }

    /* Release sched_id */
    if (egress_tm_is_service_q(tm) && tm->dir == rdpa_dir_ds)
    {
        _rdpa_rdd_sched_destroy(channel, &qtm_ctl->sched_id);
    }
    else if (qtm_ctl->sched_id != RDPA_SCHED_ID_UNASSIGNED)
    {
        rdpa_rdd_sched_type_t top_sched_type;
        uint8_t top_sched_index;

        /* Release in upper-level complex scheduler */
        _rdpa_rdd_shed_id2sched_type_index(channel->sched_id, &top_sched_type, &top_sched_index);
        if (top_sched_type == RDD_SCHED_TYPE_COMPLEX)
            rdd_complex_scheduler_block_remove(channel->dir, top_sched_index, _rdpa_get_qtm_ctl_upper_index(mo));
        else
            rdd_basic_scheduler_queue_remove(channel->dir, top_sched_index, qtm_ctl->sched_index_in_upper);

        /* For upstream service queues, re-program original best-effort queue */
        if (egress_tm_is_service_q(tm) && tm->dir == rdpa_dir_us)
        {
            struct bdmf_object *upper_mo = tm->upper_level_tm;
            tm_drv_priv_t *upper_tm = (tm_drv_priv_t *)bdmf_obj_data(upper_mo);
            tm_qtm_ctl_t *upper_qtm_ctl = egress_tm_qtm_ctl_get(upper_mo, channel);
            uint16_t upper_idx = _rdpa_get_qtm_ctl_upper_index(mo);

            if (upper_idx < RDPA_MAX_EGRESS_QUEUES && upper_qtm_ctl)
            {
                rdpa_tm_queue_cfg_t qcfg = {};

                memcpy(&qcfg, &upper_tm->queue_cfg[upper_idx], sizeof(qcfg));

                egress_tm_delete_single_queue(upper_mo, upper_qtm_ctl, channel, upper_tm, upper_idx);
                egress_tm_queue_cfg_on_channel(upper_mo, channel, upper_idx, &qcfg, 1);
            }
        }

        /* Release scheduler bit */
        _rdpa_rdd_sched_destroy(channel, &qtm_ctl->sched_id);
    }

    /* release rate controller, if any */
    _rdpa_rdd_rl_destroy(mo, channel->dir, 1, qtm_ctl->rl_rate_mode, &qtm_ctl->rc_id);
}

bdmf_error_t rdpa_rdd_qtm_ctl_modify(struct bdmf_object *mo, tm_qtm_ctl_t *qtm_ctl,
    const rdpa_tm_rl_cfg_t *cfg, int weight, int num_sp_elements)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_error_t err = BDMF_ERR_OK;
    tm_channel_t *channel = qtm_ctl->channel;
    uint16_t total_weight = 0;

    BUG_ON(!qtm_ctl);
    if (!channel)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo, "qm_ctl->channel isn't set\n");

    /* Update weight if under complex */
    if (weight >= 0)
    {
        if (qtm_ctl->egress_tm != channel->egress_tm  &&
            qtm_ctl->sched_weight != weight)
        {
            qtm_ctl->sched_weight = weight;
            err = _rdpa_rdd_set_sched_weights(mo, qtm_ctl->channel);
            if (err)
                return err;
        }
    }

    /* Update RL.
     * It might be on qtm_ctl or channel level, depending on where scheduler is allocated
     */
    if (cfg)
    {
        if (qtm_ctl->sched_id == RDPA_SCHED_ID_UNASSIGNED)
        {
            err = _rdpa_rdd_rl_reconfigure(mo, channel->dir, 1, channel->sched_id,
                rdpa_tm_rl_single_rate, cfg, &channel->rc_id);
        }
        else
        {
            err = _rdpa_rdd_rl_reconfigure(mo, channel->dir, 1, qtm_ctl->sched_id,
                qtm_ctl->rl_rate_mode, cfg, &qtm_ctl->rc_id);
        }
    }

    /* Update number of SP elements */
    if (num_sp_elements >= 0)
    {
        int16_t sched_id;
        rdpa_rdd_sched_type_t sched_type;
        uint8_t index = 0;
        int16_t *p_sched_mode;
        uint8_t dwrr_offset;

        if (egress_tm_is_service_q(tm) && tm->dir == rdpa_dir_ds)
        {
            sched_type = RDD_SCHED_TYPE_COMPLEX;
            p_sched_mode = &qtm_ctl->sched_mode;
        }
        else
        {
            if (qtm_ctl->sched_id == RDPA_SCHED_ID_UNASSIGNED)
            {
                sched_id = channel->sched_id;
                p_sched_mode = &channel->sched_mode;
            }
            else
            {
                sched_id = qtm_ctl->sched_id;
                p_sched_mode = &qtm_ctl->sched_mode;
            }

            /* Recover index */
            _rdpa_rdd_shed_id2sched_type_index(sched_id, &sched_type, &index);
        }

        err = _rdpa_rdd_map_wrr_offset(mo, sched_type, rdpa_tm_sched_sp_wrr, num_sp_elements, &dwrr_offset);
        if (err)
            return err;

        if (dwrr_offset != *p_sched_mode)
        {
            if (sched_type == RDD_SCHED_TYPE_BASIC)
            {
                err = rdd_basic_scheduler_dwrr_offset_cfg(channel->dir, index, dwrr_offset);
                BDMF_TRACE_DBG_OBJ(mo, "rdd_basic_scheduler_dwrr_offset_cfg(%d, %d, %d) --> %s\n",
                    channel->dir, index, dwrr_offset, bdmf_strerror(err));
            }
            else
            {
                if (egress_tm_is_service_q(tm) && tm->dir == rdpa_dir_ds)
                {
#if !defined(G9991)
                    err = rdd_ag_service_queues_complex_scheduler_table_dwrr_offset_sir_set(dwrr_offset);
                    err = err ? err : rdd_ag_service_queues_complex_scheduler_table_dwrr_offset_pir_set(complex_scheduler_full_sp);
#endif
                }
                else if (channel->dir == rdpa_dir_ds)
                {
                    return BDMF_ERR_NOT_SUPPORTED;
                    /*err = rdd_ag_ds_tm_complex_scheduler_table_dwrr_offset_sir_set(index, dwrr_offset);
                    err = err ? err : rdd_ag_ds_tm_complex_scheduler_table_dwrr_offset_pir_set(index, complex_scheduler_full_sp);*/
                }
                else
                {
                    err = rdd_ag_us_tm_complex_scheduler_table_dwrr_offset_sir_set(index, dwrr_offset);
                    err = err ? err : rdd_ag_us_tm_complex_scheduler_table_dwrr_offset_pir_set(index, complex_scheduler_full_sp);
                }
                BDMF_TRACE_DBG_OBJ(mo, "rdd_complex_scheduler_dwrr_offset_cfg(%d, %d, {%u, %u}) --> %s\n",
                    channel->dir, index, dwrr_offset, complex_scheduler_full_dwrr, bdmf_strerror(err));
            }

            if (!err)
            {
                *p_sched_mode = dwrr_offset;
                total_weight = _rdpa_rdd_tx_queue_calc_total_weight(mo, qtm_ctl);
                err = _rdpa_rdd_tx_queue_reconfigure_all_wrr(mo, qtm_ctl, total_weight);
            }
        }
    }

    return err;
}

/*
 * Queue management
 */

static uint16_t _rdpa_rdd_tx_queue_calc_total_weight(struct bdmf_object *mo, tm_qtm_ctl_t *qtm_ctl)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    tm_channel_t *channel = qtm_ctl->channel;
    int16_t sched_id = (qtm_ctl->sched_id != RDPA_SCHED_ID_UNASSIGNED) ? qtm_ctl->sched_id : channel->sched_id;
    int16_t sched_mode = (qtm_ctl->sched_id != RDPA_SCHED_ID_UNASSIGNED) ? qtm_ctl->sched_mode : channel->sched_mode;
    rdpa_rdd_sched_type_t sched_type;
    uint8_t sched_index;
    bdmf_boolean is_wrr_queue;
    uint16_t total_weight = 0;
    int i;

    if (egress_tm_is_service_q(tm) && tm->dir == rdpa_dir_ds)
        sched_type = RDD_SCHED_TYPE_COMPLEX;
    else
        _rdpa_rdd_shed_id2sched_type_index(sched_id, &sched_type, &sched_index);

    if ((sched_type == RDD_SCHED_TYPE_BASIC && sched_mode == basic_scheduler_full_sp) ||
        (sched_type == RDD_SCHED_TYPE_COMPLEX && sched_mode == complex_scheduler_full_sp))
    {
        return BDMF_ERR_OK;
    }

    for (i = 0; i < RDPA_MAX_EGRESS_QUEUES; i++)
    {
        tm_queue_hash_entry_t *qentry = &qtm_ctl->hash_entry[i];
        is_wrr_queue = is_wrr_elem(mo, channel->rl_rate_mode, sched_id, sched_mode, qentry->queue_index);
        if (qentry->rdp_queue_index >= 0 && is_wrr_queue)
            total_weight += qentry->sched_weight;
    }

    return total_weight;
}

static bdmf_error_t _rdpa_rdd_tx_queue_bind(struct bdmf_object *mo, tm_queue_hash_entry_t *qentry, uint16_t total_weight)
{
    tm_qtm_ctl_t *qtm_ctl =  egress_tm_hash_entry_container(qentry);
    tm_channel_t *channel = qtm_ctl->channel;
    int16_t sched_id = (qtm_ctl->sched_id != RDPA_SCHED_ID_UNASSIGNED) ? qtm_ctl->sched_id : channel->sched_id;
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);

    rdpa_rdd_sched_type_t sched_type;
    uint8_t sched_index = 0;
    quantum_number_t quantum_number;
    bdmf_error_t err = BDMF_ERR_NOT_SUPPORTED;

    quantum_number = _rdpa_rdd_calc_quantum_number(total_weight, qentry->sched_weight);

    if (egress_tm_is_service_q(tm) && tm->dir == rdpa_dir_ds)
        sched_type = RDD_SCHED_TYPE_COMPLEX;
    else
        _rdpa_rdd_shed_id2sched_type_index(sched_id, &sched_type, &sched_index);

    if (sched_type == RDD_SCHED_TYPE_BASIC)
    {
        basic_scheduler_queue_t sched_queue_cfg =
        {
            .qm_queue_index = qentry->rdp_queue_index,
            .queue_scheduler_index = (tm->upper_level_tm && (tm->mode == rdpa_tm_sched_disabled)) ? qtm_ctl->sched_index_in_upper : qentry->queue_index,
            .quantum_number = quantum_number
        };
        err = rdd_basic_scheduler_queue_cfg(channel->dir, sched_index, &sched_queue_cfg);
        BDMF_TRACE_DBG_OBJ(mo, "rdd_basic_scheduler_queue_cfg(%d, %d, {%d, %d, %d}) -> %s\n",
            channel->dir, sched_index, sched_queue_cfg.qm_queue_index,
            sched_queue_cfg.queue_scheduler_index, sched_queue_cfg.quantum_number, bdmf_strerror(err));
    }
    else
    {
        complex_scheduler_block_t sched_queue_cfg =
        {
            .block_index = qentry->rdp_queue_index,
            .scheduler_slot_index = qentry->queue_index,
            .quantum_number = quantum_number,
            .block_type = complex_scheduler_block_queue,
        };

        if (egress_tm_is_service_q(tm) && tm->dir == rdpa_dir_ds)
        {
#if !defined G9991
            sched_queue_cfg.block_index -= drv_qm_get_sq_start();
            err = rdd_service_queue_scheduler_block_cfg(sched_queue_cfg.block_index, &sched_queue_cfg);
            BDMF_TRACE_DBG_OBJ(mo, "rdd_service_queue_scheduler_block_cfg({%d, %d, %d}) -> %s\n",
                sched_queue_cfg.block_index, sched_queue_cfg.scheduler_slot_index,
                sched_queue_cfg.quantum_number, bdmf_strerror(err));
#endif
        }
        else
        {
            err = rdd_complex_scheduler_block_cfg(channel->dir, sched_index, &sched_queue_cfg);
            BDMF_TRACE_DBG_OBJ(mo, "rdd_complex_scheduler_block_cfg(%d, %d, {%d, %d, %d, %d}) -> %s\n",
                channel->dir, sched_index, sched_queue_cfg.block_index,
                sched_queue_cfg.scheduler_slot_index, sched_queue_cfg.quantum_number,
                sched_queue_cfg.block_type, bdmf_strerror(err));
        }
    }
    return err;
}

/* Reconfigure all queues. It is needed when adding new WRR queue or changing weight
 * of the existing WRR queue
 */
static bdmf_error_t _rdpa_rdd_tx_queue_reconfigure_all_wrr(struct bdmf_object *mo, tm_qtm_ctl_t *qtm_ctl, uint16_t total_weight)
{
    int i;
    bdmf_error_t err = BDMF_ERR_OK;
    for (i = 0; i < RDPA_MAX_EGRESS_QUEUES && !err; i++)
    {
        tm_queue_hash_entry_t *qentry = &qtm_ctl->hash_entry[i];
        if (qentry->rdp_queue_index >= 0 && qentry->sched_weight > 0)
            err = _rdpa_rdd_tx_queue_bind(mo, qentry, total_weight);
    }
    return err;
}

static bdmf_error_t _rdpa_rdd_tx_queue_unbind(struct bdmf_object *mo, tm_queue_hash_entry_t *qentry)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    tm_qtm_ctl_t *qtm_ctl =  egress_tm_hash_entry_container(qentry);
    tm_channel_t *channel = qtm_ctl->channel;
    int16_t sched_id = (qtm_ctl->sched_id != RDPA_SCHED_ID_UNASSIGNED) ? qtm_ctl->sched_id : channel->sched_id;
    rdpa_rdd_sched_type_t sched_type;
    uint8_t sched_index = 0;
    bdmf_error_t err = BDMF_ERR_NOT_SUPPORTED;

    /* clear queue counter */
    rdpa_rdd_tx_queue_stat_clear(mo, qentry);

    if (egress_tm_is_service_q(tm) && tm->dir == rdpa_dir_ds)
        sched_type = RDD_SCHED_TYPE_COMPLEX;
    else
        _rdpa_rdd_shed_id2sched_type_index(sched_id, &sched_type, &sched_index);

    if (sched_type == RDD_SCHED_TYPE_BASIC)
    {
        err = rdd_basic_scheduler_queue_remove(channel->dir, sched_index, qentry->queue_index);
        BDMF_TRACE_DBG_OBJ(mo, "rdd_basic_scheduler_queue_remove(%d, %d, %d) -> %s\n",
            channel->dir, sched_index, qentry->queue_index, bdmf_strerror(err));
    }
    else
    {
        if (egress_tm_is_service_q(tm) && tm->dir == rdpa_dir_ds)
        {
#if !defined G9991
            err = rdd_service_queue_scheduler_block_remove(qentry->queue_index);
            BDMF_TRACE_DBG_OBJ(mo, "rdd_service_queue_scheduler_block_remove(%d) -> %s\n",
                qentry->queue_index, bdmf_strerror(err));
#endif
        }
        else
        {
            err = rdd_complex_scheduler_block_remove(channel->dir, sched_index, qentry->queue_index);
            BDMF_TRACE_DBG_OBJ(mo, "rdd_complex_scheduler_block_remove(%d, %d, %d) -> %s\n",
                channel->dir, sched_index, qentry->queue_index, bdmf_strerror(err));
        }
    }

    return err;
}

static rdpa_tm_rl_rate_mode _rdpa_rdd_tx_queue_rate_mode_get(tm_qtm_ctl_t *qtm_ctl)
{
    tm_channel_t *channel = qtm_ctl->channel;
    rdpa_tm_rl_rate_mode rate_mode;

    /* In single egress-tm hierarchy (top egress_tm is queue level)
     * queue rate mode is inherited from the egress_tm.
     * In dual egress_tm hierarchy queue rate mode is always single-rate
     */
    if (qtm_ctl->egress_tm != channel->egress_tm)
        rate_mode = rdpa_tm_rl_single_rate;
    else
        rate_mode = qtm_ctl->rl_rate_mode;

    return rate_mode;
}

bdmf_error_t rdpa_rdd_tx_queue_create(struct bdmf_object *mo,
    rdpa_wan_type wan_type, tm_queue_hash_entry_t *qentry,
    const rdd_queue_cfg_t *queue_cfg, pd_offload_ddr_queue_t *ddr_cfg, bdmf_boolean enable)
{
    tm_qtm_ctl_t *qtm_ctl =  egress_tm_hash_entry_container(qentry);
    tm_channel_t *channel = qtm_ctl->channel;
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    int16_t sched_id = (qtm_ctl->sched_id != RDPA_SCHED_ID_UNASSIGNED) ? qtm_ctl->sched_id : channel->sched_id;
    int16_t sched_mode = (qtm_ctl->sched_id != RDPA_SCHED_ID_UNASSIGNED) ? qtm_ctl->sched_mode : channel->sched_mode;
    qm_q_context qm_queue_ctx = {};
    bdmf_boolean is_wrr_queue;
    uint16_t total_weight = 0;
    bdmf_error_t err;

    BUG_ON(!channel);

    is_wrr_queue = is_wrr_elem(mo, channel->rl_rate_mode, sched_id, sched_mode, qentry->queue_index);
    if (is_wrr_queue && (queue_cfg->weight < RDPA_MIN_WEIGHT || queue_cfg->weight > RDPA_MAX_WEIGHT))
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_RANGE, mo, "WRR weight %u is out of range %d..%d\n",
            (unsigned)queue_cfg->weight, (unsigned)RDPA_MIN_WEIGHT, (unsigned)RDPA_MAX_WEIGHT);
    }

    qentry->sched_weight = queue_cfg->weight;

    /* Set XRDP internal fields in qentry */
    qentry->rc_id = RDPA_RL_ID_UNASSIGNED;

    /* For upstream service queues, re-use the normal best-effort queue as the
     * first queue in order to allow active flows to continue unimpeded. */
    if (egress_tm_is_service_q(tm) && tm->dir == rdpa_dir_us && qentry->queue_index == 0)
    {
        tm_qtm_ctl_t *qtm_ctl = egress_tm_qtm_ctl_get(channel->egress_tm, channel);
        uint16_t upper_idx = _rdpa_get_qtm_ctl_upper_index(mo);

        if (upper_idx >= RDPA_MAX_EGRESS_QUEUES)
            BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "Could not find best-effort queue index in parent scheduler\n");

        qentry->rdp_queue_index = qtm_ctl->hash_entry[upper_idx].rdp_queue_index;

        _rdpa_rdd_tx_queue_bind(mo, qentry, total_weight);
        return BDMF_ERR_OK;
    }

    /* Allocate QM queue */
    err = _rdpa_rdd_queue_alloc(channel, qtm_ctl, qentry, &qentry->rdp_queue_index);
    if (err)
    {
        BDMF_TRACE_ERR("failed to allocate qm queue\n");
        return err;
    }
    /* enable QM queue */
    err = enable_qm_queue_once(qentry->rdp_queue_index);

    /* Zero queue statistics */
    memset(&accumulative_queue_stat[qentry->rdp_queue_index], 0, sizeof(rdpa_stat_1way_t));

    _rdpa_qm_queue_ctx_set(wan_type, channel, qentry->rdp_queue_index, queue_cfg, &qm_queue_ctx);
    rdd_set_queue_enable(qentry->rdp_queue_index, enable);

    if (is_wrr_queue)
        total_weight = _rdpa_rdd_tx_queue_calc_total_weight(mo, qtm_ctl);

    /* Bind queue to the scheduler */
    if (is_wrr_queue && qentry->sched_weight != total_weight)
        err = _rdpa_rdd_tx_queue_reconfigure_all_wrr(mo, qtm_ctl, total_weight);
    else
        err = _rdpa_rdd_tx_queue_bind(mo, qentry, total_weight);
    if (err)
        goto out;

    /* Create RL on queue if necessary */
    err = _rdpa_rdd_rl_reconfigure(mo, channel->dir, 0, qentry->rdp_queue_index,
        _rdpa_rdd_tx_queue_rate_mode_get(qtm_ctl), &queue_cfg->rl_cfg, &qentry->rc_id);
    if (err)
        goto out;

    err = drv_qm_queue_config(qentry->rdp_queue_index, &qm_queue_ctx);

    BDMF_TRACE_DBG_OBJ(mo, "drv_qm_queue_config(%d, 1, {profile=%u}) -> %s\n",
        qentry->rdp_queue_index, queue_cfg->profile, bdmf_strerror(err));

    rdd_qm_queue_to_tx_flow_tbl_cfg(qentry->rdp_queue_index, channel->dir, wan_type);

out:
    if (err)
    {
        /* Release queue index */
        _rdpa_rdd_queue_free(channel->dir, qentry->rdp_queue_index);
        qentry->rdp_queue_index = -1;
    }

    return err;
}

int egress_tm_queue_occupancy_read_ex(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    int rc = 0;
    uint32_t packets = 0;
    uint32_t aggr_packet = 0;
    uint32_t bytes = 0;
    rdpa_stat_t *queue_occupancy = (rdpa_stat_t *)val;
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_tm_queue_index_t *qi = (rdpa_tm_queue_index_t *)index;
    queue_info_t queue_id_info = {0};
    int16_t hashed_channel_id;
    struct bdmf_object *root = egress_tm_get_root_object(mo);
    tm_drv_priv_t *root_tm = (tm_drv_priv_t *)bdmf_obj_data(root);
    queue_occupancy->packets = 0;
    queue_occupancy->bytes = 0;

    hashed_channel_id = egress_tm_is_group_owner(root_tm) ? root_tm->channel_group : qi->channel;
    rc = egress_tm_queue_id_info_get(tm->dir, hashed_channel_id, qi->queue_id, &queue_id_info);
    if (rc)
    {
        BDMF_TRACE_ERR("egress_tm_queue_id_info_get returned %d for channel=%d queue_id=%d\n", rc, (int)hashed_channel_id, qi->queue_id);
        return rc;
    }

    if (is_qm_queue_aggregation_context_valid(queue_id_info.queue))
    {
        BDMF_TRACE_DBG("NON ZERO AGG CONTEXT - for channel_id %d queue_id %d\n", (int) qi->channel, queue_id_info.queue);
        aggr_packet = 1;
    }

    rc = rc ? rc : ag_drv_qm_total_valid_cnt_get(((uint16_t)queue_id_info.queue * (sizeof(RDD_QM_QUEUE_COUNTER_DATA_DTS) / 4)), &(packets));
    rc = rc ? rc : ag_drv_qm_total_valid_cnt_get(((uint16_t)queue_id_info.queue * (sizeof(RDD_QM_QUEUE_COUNTER_DATA_DTS) / 4)) + 1, &(bytes));
    if (rc)
        return rc;

    BDMF_TRACE_DBG_OBJ(mo, "queue occupancy for queue %d channel %d is :packets = %d, bytes = %d\n",
            (int)queue_id_info.queue, (int)queue_id_info.channel, packets + aggr_packet, bytes);
    queue_occupancy->packets = packets + aggr_packet;
    queue_occupancy->bytes = bytes;

    /* in epon check also second level queue */
    if ((tm->dir == rdpa_dir_us) && ((tm->wan_type == rdpa_wan_epon) || (tm->wan_type == rdpa_wan_xepon)) && (drv_qm_get_us_epon_start() != QM_ILLEGAL_QUEUE) &&
        ((queue_id_info.queue + drv_qm_get_us_epon_start()) <= drv_qm_get_us_epon_end()))
    {
        queue_id_info.queue += drv_qm_get_us_epon_start();
        rc = rc ? rc : ag_drv_qm_total_valid_cnt_get(((uint16_t)queue_id_info.queue * (sizeof(RDD_QM_QUEUE_COUNTER_DATA_DTS) / 4)), &(packets));
        rc = rc ? rc : ag_drv_qm_total_valid_cnt_get(((uint16_t)queue_id_info.queue * (sizeof(RDD_QM_QUEUE_COUNTER_DATA_DTS) / 4)) + 1, &(bytes));

        if (rc)
        {
           return rc;
        }
        queue_occupancy->packets += packets;
        queue_occupancy->bytes += bytes;
        BDMF_TRACE_DBG_OBJ(mo, "queue occupancy for second level queue  %d is :packets = %d, bytes = %d\n",
                (int)queue_id_info.queue, packets , bytes);
    }
    return rc;
}

bdmf_error_t egress_tm_is_queue_empty_on_channel_ex(bdmf_object_handle tm_obj, uint32_t channel_index, uint32_t queue_index, bdmf_boolean *is_empty)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(tm_obj);
    rdpa_tm_queue_index_t q_index;
    rdpa_stat_t q_stat = {};
    bdmf_error_t rc = BDMF_ERR_OK;
    tm_channel_t *ch;

    if (channel_index != RDPA_EGRESS_TM_CHANNEL_IS_GROUP_ID)
    {
        ch = egress_tm_channel_get(tm->dir, channel_index);
        q_index.channel = ch->channel_id;
    }
    else
    {
        q_index.channel = RDPA_EGRESS_TM_CHANNEL_IS_GROUP_ID;
    }

    *is_empty = 1;
    q_index.queue_id = tm->queue_cfg[queue_index].queue_id;
    rc = egress_tm_queue_occupancy_read_ex(tm_obj, NULL, (bdmf_index)&q_index, &q_stat, 0);
    if (rc)
        return rc;

    if ((q_stat.packets) || (q_stat.bytes))
        *is_empty = 0;

    return 0;
}

bdmf_error_t egress_tm_is_empty_on_channel_ex(bdmf_object_handle tm_obj, uint32_t channel_index, bdmf_boolean *is_empty)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(tm_obj);
    uint8_t i;

    *is_empty = 1;
    if (tm->level == rdpa_tm_level_queue)
    {
        for (i = 0; i < tm->num_queues && !rc; i++)
        {
            if (!(tm->queue_configured[i]))
                continue;

            rc = egress_tm_is_queue_empty_on_channel_ex(tm_obj, channel_index, i, is_empty);
            if (rc)
                return rc;

            if (*is_empty == 0)
                return 0;
        }
    }
    else
    {
        for (i = 0; i < RDPA_TM_MAX_SCHED_ELEMENTS && !rc; i++)
        {
            if (tm->sub_tms[i])
                rc = egress_tm_is_empty_on_channel_ex(tm->sub_tms[i], channel_index, is_empty);

            if (rc)
                return rc;

            if (*is_empty == 0)
               return 0;
        }
    }
    return rc;
}

int egress_tm_allocate_counter(struct bdmf_object *mo, tm_qtm_ctl_t *qtm_ctl, rdpa_tm_queue_cfg_t *new_queue_cfg, int index)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    if (qtm_ctl->counter_id[index] == INVALID_COUNTER_ID)
    {
       if (queue_counters_idx[tm->dir] < RDPA_MAX_QUEUE_COUNTERS)
           qtm_ctl->counter_id[index] = queue_counters[tm->dir][queue_counters_idx[tm->dir]++];
       else
           BDMF_TRACE_RET_OBJ(BDMF_ERR_NOENT, mo, "Can't assign queue counter\n");
    }
    return 0;
}

bdmf_error_t rdpa_rdd_tx_queue_modify(struct bdmf_object *mo, tm_queue_hash_entry_t *qentry,
    const rdd_queue_cfg_t *queue_cfg, pd_offload_ddr_queue_t *ddr_cfg, bdmf_boolean enable)
{
    tm_qtm_ctl_t *qtm_ctl =  egress_tm_hash_entry_container(qentry);
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    tm_channel_t *channel = qtm_ctl->channel;
    qm_q_context qm_queue_ctx = {};
    bdmf_error_t err = 0;

    if ((!enable) || (queue_cfg->packet_threshold == 0))
    {
        err = rdpa_rdd_tx_queue_disable(mo, qentry, ddr_cfg);
        return err;
    }

    BUG_ON(!channel);

    /* We only support changing WRED profile and drop threshold */
    /* At this point WRED profile is already configured */
    /* Re-Configure QM queue */
    rdd_set_queue_enable(qentry->rdp_queue_index, enable);

    _rdpa_qm_queue_ctx_set(tm->wan_type, channel, qentry->rdp_queue_index, queue_cfg, &qm_queue_ctx);
    err = drv_qm_queue_config(qentry->rdp_queue_index, &qm_queue_ctx);
    BDMF_TRACE_DBG_OBJ(mo, "drv_qm_queue_config(%d, 1, {profile=%u}) -> %s\n",
        qentry->rdp_queue_index, queue_cfg->profile, bdmf_strerror(err));
    if (err)
        return err;

    /* Reconfigure WRR weight is necessary */
    if (qentry->sched_weight != queue_cfg->weight)
    {
        uint16_t total_weight;

        if (queue_cfg->weight < RDPA_MIN_WEIGHT || queue_cfg->weight > RDPA_MAX_WEIGHT)
        {
            BDMF_TRACE_ERR_OBJ(mo, "WRR weight %u is out of range %d..%d\n",
                (unsigned)queue_cfg->weight, (unsigned)RDPA_MIN_WEIGHT, (unsigned)RDPA_MAX_WEIGHT);
            err = BDMF_ERR_RANGE;
        }

        qentry->sched_weight = queue_cfg->weight;
        total_weight = _rdpa_rdd_tx_queue_calc_total_weight(mo, qtm_ctl);
        if (qentry->sched_weight != total_weight)
            err = err ? err : _rdpa_rdd_tx_queue_reconfigure_all_wrr(mo, qtm_ctl, total_weight);
    }

    /* Create / destroy / reconfigure RL on queue */
    err = err ? err : _rdpa_rdd_rl_reconfigure(mo, channel->dir, 0, qentry->rdp_queue_index,
        _rdpa_rdd_tx_queue_rate_mode_get(qtm_ctl), &queue_cfg->rl_cfg, &qentry->rc_id);

    /* Update queue_id in rdd_queue_idx --> queue_id mapper */
    if (!err)
        qm_queue_info[qentry->rdp_queue_index].queue_id = qentry->queue_id;

    return err;
}

int  rdpa_rdd_tx_queue_disable(struct bdmf_object *mo, tm_queue_hash_entry_t *qentry, pd_offload_ddr_queue_t *ddr_cfg)
{
    int rc;
    qm_q_context qm_queue_ctx = {};
    rc = ag_drv_qm_q_context_get(qentry->rdp_queue_index, &qm_queue_ctx);
    if (rc)
    {
        BDMF_TRACE_RET_OBJ(rc, mo, "Failed to get queue %d context, err: %d\n", qentry->rdp_queue_index, rc);
    }

    qm_queue_ctx.wred_profile = QM_WRED_PROFILE_DROP_ALL;
    rc = ag_drv_qm_q_context_set(qentry->rdp_queue_index, &qm_queue_ctx);
    if (rc)
    {
        BDMF_TRACE_RET_OBJ(rc, mo, "Failed to set queue %d context, err: %d\n", qentry->rdp_queue_index, rc);
    }

    rdd_set_queue_enable(qentry->rdp_queue_index, 0);

    return 0;
}

void rdpa_rdd_tx_queue_destroy(struct bdmf_object *mo, tm_queue_hash_entry_t *qentry, pd_offload_ddr_queue_t *ddr_cfg, int i)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    tm_qtm_ctl_t *qtm_ctl =  egress_tm_hash_entry_container(qentry);
    tm_channel_t *channel = qtm_ctl->channel;


    BUG_ON(!channel);

    /* For upstream service queues, the the normal best-effort queue is re-used
     * as the first service queue, so don't delete in that instance */
    if (egress_tm_is_service_q(tm) && tm->dir == rdpa_dir_us && qentry->queue_index == 0)
    {
        struct bdmf_object *upper_mo = tm->upper_level_tm;
        tm_qtm_ctl_t *upper_qtm_ctl = egress_tm_qtm_ctl_get(upper_mo, channel);
        uint16_t upper_idx = _rdpa_get_qtm_ctl_upper_index(mo);
        tm_queue_hash_entry_t *upper_qentry;

        _rdpa_rdd_tx_queue_unbind(mo, qentry);

        if (upper_mo && upper_qtm_ctl) {
            upper_qentry = &upper_qtm_ctl->hash_entry[upper_idx];

            if (upper_qentry)
            {
                qm_queue_info[qentry->rdp_queue_index].queue_id = upper_qentry->queue_id;
                qm_queue_info[qentry->rdp_queue_index].qtm_ctl = upper_qtm_ctl;
            }
        }

        qentry->rdp_queue_index = -1;
        return;
    }

    BDMF_TRACE_DBG_OBJ(mo, "destroy queue =%d\n", qentry->rdp_queue_index);
    /* disable and flush */

    egress_tm_queue_cfg_on_channel(mo, channel, i, &tm->queue_cfg[i], 0);
    rdpa_rdd_tx_queue_flush(mo, qentry, 1);

    /* Destroy rate controller if any */
    _rdpa_rdd_rl_destroy(mo, channel->dir, 0, _rdpa_rdd_tx_queue_rate_mode_get(qtm_ctl), &qentry->rc_id);

    /* Detach from scheduler and release */
    _rdpa_rdd_tx_queue_unbind(mo, qentry);
    _rdpa_rdd_queue_free(channel->dir, qentry->rdp_queue_index);

    if (qentry->rdp_queue_index != -1)
        rdd_set_queue_enable(qentry->rdp_queue_index, 0);

    qentry->rdp_queue_index = -1;
    if (qentry->sched_weight)
    {
        uint16_t total_weight;
        qentry->sched_weight = 0;
        total_weight = _rdpa_rdd_tx_queue_calc_total_weight(mo, qtm_ctl);
        if (total_weight)
            _rdpa_rdd_tx_queue_reconfigure_all_wrr(mo, qtm_ctl, total_weight);
    }
}

#if defined(HW_QM_QUEUE_FLUSH)
bdmf_error_t rdpa_rdd_tx_hw_queue_flush(struct bdmf_object *mo, tm_queue_hash_entry_t *qentry)
{
    uint32_t cnt = 0, cnt_at_begining, is_aggr_valid;
    uint64_t time_start, time_end;
    long elapsed;
    bdmf_error_t err = BDMF_ERR_OK;

    err = ag_drv_qm_total_valid_cnt_get(qentry->rdp_queue_index * (sizeof(RDD_QM_QUEUE_COUNTER_DATA_DTS) / 4), &cnt_at_begining);
    /* Activate HW flush  */
    ag_drv_qm_global_cfg_qm_egress_flush_queue_set(qentry->rdp_queue_index, 1);
    RDD_HW_FLUSH_ENTRY_QM_QUEUE_WRITE_G(qentry->rdp_queue_index, RDD_TM_HW_FLUSH_ADDRESS_ARR, 0);
    RDD_HW_FLUSH_ENTRY_FLUSH_AGGR_WRITE_G(0, RDD_TM_HW_FLUSH_ADDRESS_ARR, 0);
    RDD_HW_FLUSH_ENTRY_HW_FLUSH_EN_WRITE_G(1, RDD_TM_HW_FLUSH_ADDRESS_ARR, 0);
    if (egress_tm_get_queue_dir(qentry->dir_channel) == rdpa_dir_ds)
    {
        RDD_HW_FLUSH_ENTRY_US_WRITE_G(0, RDD_TM_HW_FLUSH_ADDRESS_ARR, 0);
        rdd_ag_ds_tm_scheduling_global_flush_cfg_set(1);
        WMB();
        ag_drv_rnr_regs_cfg_cpu_wakeup_set(get_runner_idx(ds_tm_runner_image), DS_TM_UPDATE_FIFO_THREAD_NUMBER);
    }
    else
    {
        RDD_HW_FLUSH_ENTRY_US_WRITE_G(1, RDD_TM_HW_FLUSH_ADDRESS_ARR, 0);
        rdd_ag_us_tm_scheduling_global_flush_cfg_set(1);
        WMB();
        ag_drv_rnr_regs_cfg_cpu_wakeup_set(get_runner_idx(us_tm_runner_image), US_TM_UPDATE_FIFO_THREAD_NUMBER);
    }
    RDD_HW_FLUSH_ENTRY_FLUSH_AGGR_WRITE_G(1, RDD_TM_HW_FLUSH_ADDRESS_ARR, 0);
    /* Wait until queue drains */
    time_start = bdmf_time_since_epoch_usec();
    do
    {
        time_end = bdmf_time_since_epoch_usec();
        elapsed = time_end - time_start;
        is_aggr_valid = is_qm_queue_aggregation_context_valid(qentry->rdp_queue_index);
        err = err ? err : ag_drv_qm_total_valid_cnt_get(qentry->rdp_queue_index * (sizeof(RDD_QM_QUEUE_COUNTER_DATA_DTS) / 4), &cnt);
        if (err)
            BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo, "error in reading qm aggr context or total valid count for egress queue %d\n", qentry->rdp_queue_index);
    } while (((cnt != 0) || (is_aggr_valid)) && (elapsed < RDPA_FLUSH_TIMEOUT));

     /* stop queue flush*/
    if (egress_tm_get_queue_dir(qentry->dir_channel) == rdpa_dir_ds)
    {
        rdd_ag_ds_tm_scheduling_global_flush_cfg_set(0);
    }
    else
    {
        rdd_ag_us_tm_scheduling_global_flush_cfg_set(0);
    }

    RDD_HW_FLUSH_ENTRY_FLUSH_AGGR_WRITE_G(0, RDD_TM_HW_FLUSH_ADDRESS_ARR, 0);
    ag_drv_qm_global_cfg_qm_egress_flush_queue_set(qentry->rdp_queue_index, 0);
    RDD_HW_FLUSH_ENTRY_HW_FLUSH_EN_WRITE_G(0, RDD_TM_HW_FLUSH_ADDRESS_ARR, 0);

    BDMF_TRACE_DBG_OBJ(mo, "flushed queue =%d, packets left:%d (at beginning:%d)\n", qentry->rdp_queue_index, cnt, cnt_at_begining);

    if (elapsed >= RDPA_FLUSH_TIMEOUT)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo, "flush timed out for egress queue %d. packets left:%d (at beginning:%d)\n", qentry->rdp_queue_index, cnt, cnt_at_begining);

    return err;
}
#endif


bdmf_error_t rdpa_rdd_tx_queue_flush(struct bdmf_object *mo, tm_queue_hash_entry_t *qentry,
    bdmf_boolean is_wait)
{
    qm_q_context qm_queue_ctx;
    uint32_t cnt = 0, flush_vector, cnt_at_begining, core_index, cnt_after_flush, is_aggr_valid;
    uint64_t time_start, time_end;
    long elapsed;
    bdmf_error_t err = BDMF_ERR_OK;

    /* Setup queue profile */
    err = ag_drv_qm_q_context_get(qentry->rdp_queue_index, &qm_queue_ctx);
    if (err)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo, "error in reading qm_q_context, rdp_queue_index = %d\n", qentry->rdp_queue_index);
    qm_queue_ctx.wred_profile = QM_WRED_PROFILE_DROP_ALL;
    ag_drv_qm_q_context_set(qentry->rdp_queue_index, &qm_queue_ctx);

    /* check queue */
    is_aggr_valid = is_qm_queue_aggregation_context_valid(qentry->rdp_queue_index);
    err = err ? err : ag_drv_qm_total_valid_cnt_get(qentry->rdp_queue_index * (sizeof(RDD_QM_QUEUE_COUNTER_DATA_DTS) / 4), &cnt_at_begining);
    if (err)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo, "first: error in reading qm aggr context or total valid count for egress queue %d\n", qentry->rdp_queue_index);

    BDMF_TRACE_DBG_OBJ(mo, "checking if need to flush queue =%d\n", qentry->rdp_queue_index);
    /* check if FW flush is needed */
    if ((cnt_at_begining == 0) && !(is_aggr_valid))
        return err;

#if defined(HW_QM_QUEUE_FLUSH)
    return  rdpa_rdd_tx_hw_queue_flush(mo, qentry);
#endif

    /* Activate flush in fw */
    if (egress_tm_get_queue_dir(qentry->dir_channel) == rdpa_dir_ds)
    {
        rdd_ag_ds_tm_scheduling_flush_vector_get(((qentry->rdp_queue_index - (drv_qm_get_ds_start() & (~0x1f))) >> 5), &flush_vector);
        flush_vector |= (1 << ((qentry->rdp_queue_index - (drv_qm_get_ds_start() & (~0x1f))) & 0x1f));
        rdd_ag_ds_tm_scheduling_flush_vector_set(((qentry->rdp_queue_index - (drv_qm_get_ds_start() & (~0x1f))) >> 5), flush_vector);

        flush_queue_cnt[rdpa_dir_ds]++;
        rdd_ag_ds_tm_scheduling_global_flush_cfg_set(1);
        WMB();
        for (core_index = 0; core_index < GROUPED_EN_SEGMENTS_NUM; core_index++)
            if (IS_DS_TM_RUNNER_IMAGE(core_index))
                ag_drv_rnr_regs_cfg_cpu_wakeup_set(core_index, DS_TM_UPDATE_FIFO_THREAD_NUMBER);
    }
    else
    {
        rdd_ag_us_tm_scheduling_flush_vector_get((qentry->rdp_queue_index >> 5), &flush_vector);
        flush_vector |= (1 << (qentry->rdp_queue_index & 0x1f));
        rdd_ag_us_tm_scheduling_flush_vector_set((qentry->rdp_queue_index >> 5), flush_vector);
        flush_queue_cnt[rdpa_dir_us]++;
        rdd_ag_us_tm_scheduling_global_flush_cfg_set(1);
        WMB();
        ag_drv_rnr_regs_cfg_cpu_wakeup_set(get_runner_idx(us_tm_runner_image), US_TM_UPDATE_FIFO_THREAD_NUMBER);
    }

    /* Wait until queue drains */
    time_start = bdmf_time_since_epoch_usec();
    do
    {
        time_end = bdmf_time_since_epoch_usec();
        elapsed = time_end - time_start;
        is_aggr_valid = is_qm_queue_aggregation_context_valid(qentry->rdp_queue_index);
        err = err ? err : ag_drv_qm_total_valid_cnt_get(qentry->rdp_queue_index * (sizeof(RDD_QM_QUEUE_COUNTER_DATA_DTS) / 4), &cnt);
        if (err)
            BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo, "error in reading qm aggr context or total valid count for egress queue %d\n", qentry->rdp_queue_index);
    } while (((cnt != 0) || (is_aggr_valid)) && (elapsed < RDPA_FLUSH_TIMEOUT));


    /* Check if other queues are flushed */
    if (egress_tm_get_queue_dir(qentry->dir_channel) == rdpa_dir_ds)
    {
        flush_queue_cnt[rdpa_dir_ds]--;
        if (!flush_queue_cnt[rdpa_dir_ds])
            rdd_ag_ds_tm_scheduling_global_flush_cfg_set(0);
        rdd_ag_ds_tm_scheduling_flush_vector_get(((qentry->rdp_queue_index - (drv_qm_get_ds_start() & (~0x1f))) >> 5), &flush_vector);
        flush_vector &= ~(1 << ((qentry->rdp_queue_index - (drv_qm_get_ds_start() & (~0x1f))) & 0x1f));
        rdd_ag_ds_tm_scheduling_flush_vector_set(((qentry->rdp_queue_index - (drv_qm_get_ds_start() & (~0x1f))) >> 5), flush_vector);
    }
    else
    {
        flush_queue_cnt[rdpa_dir_us]--;
        if (!flush_queue_cnt[rdpa_dir_us])
            rdd_ag_us_tm_scheduling_global_flush_cfg_set(0);
        rdd_ag_us_tm_scheduling_flush_vector_get((qentry->rdp_queue_index >> 5), &flush_vector);
        flush_vector &= ~(1 << (qentry->rdp_queue_index & 0x1f));
        rdd_ag_us_tm_scheduling_flush_vector_set((qentry->rdp_queue_index >> 5), flush_vector);
    }

    ag_drv_qm_total_valid_cnt_get(qentry->rdp_queue_index * (sizeof(RDD_QM_QUEUE_COUNTER_DATA_DTS) / 4), &cnt_after_flush);
    BDMF_TRACE_DBG_OBJ(mo, "flushed queue =%d, packets left:%d (at beginning:%d)\n", qentry->rdp_queue_index, cnt, cnt_at_begining);

    if (elapsed >= RDPA_FLUSH_TIMEOUT)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo, "flush timed out for egress queue %d. packets left:%d (at beginning:%d)\n", qentry->rdp_queue_index, cnt, cnt_at_begining);

    return err;
}

bdmf_error_t _rdpa_egress_tm_tx_queue_drop_stat_read(int rdp_queue_index, rdpa_stat_1way_t *stat)
{
    uint32_t cntr_val = 0;
    bdmf_error_t rc = BDMF_ERR_OK;

    memset(stat, 0, sizeof(rdpa_stat_1way_t));

    rc = ag_drv_qm_drop_counter_get(rdp_queue_index * 2, &cntr_val);
    /* update the tm_queue accum buffer as HW counter has been read */
    if (!rc)
    {
        rdpa_common_update_cntr_results_uint32(&(stat->discarded), &(accumulative_queue_stat[rdp_queue_index].discarded),
            _get_rdpa_stat_offset(rdpa_stat_pckts_id), cntr_val);
    }
    return rc;
}

bdmf_error_t rdpa_rdd_tx_queue_stat_read(struct bdmf_object *mo, tm_queue_hash_entry_t *qentry, rdpa_stat_1way_t *stat)
{
    int counter_index = qentry->rdp_queue_index * 2;
    bdmf_error_t rc = BDMF_ERR_OK, rc_to_ret = BDMF_ERR_OK;
    uint32_t cntr_arr[MAX_NUM_OF_COUNTERS_PER_READ] = {};

    memset(stat, 0, sizeof(rdpa_stat_1way_t));

    /* VALID counters */
    rc = drv_cntr_counter_read(CNTR_GROUP_TX_QUEUE, qentry->rdp_queue_index, cntr_arr);
    /* in case of error zero counters data so result value will be last accumulated */
    if (rc)
    {
        cntr_arr[0] = 0;
        cntr_arr[1] = 0;
        BDMF_TRACE_ERR("Error reading passed packet counters for egress queue %d, rc %d\n",
            qentry->rdp_queue_index, rc);
        rc_to_ret = rc;
    }
    rdpa_common_update_cntr_results_uint32(&(stat->passed), &(accumulative_queue_stat[qentry->rdp_queue_index].passed),
        _get_rdpa_stat_offset(rdpa_stat_pckts_id), cntr_arr[0]);

    rdpa_common_update_cntr_results_uint32(&(stat->passed), &(accumulative_queue_stat[qentry->rdp_queue_index].passed),
        _get_rdpa_stat_offset(rdpa_stat_bytes_id), cntr_arr[1]);

    rc = ag_drv_qm_drop_counter_get(counter_index, &cntr_arr[2]);
    if (rc)
    {
        cntr_arr[2] = 0;
        BDMF_TRACE_ERR("Error reading dropped packets counter for egress queue %d, rc %d\n",
            qentry->rdp_queue_index, rc);
        rc_to_ret = rc;
    }

    rc = ag_drv_qm_drop_counter_get(counter_index+1, &cntr_arr[3]);
    if (rc)
    {
        cntr_arr[3] = 0;
        BDMF_TRACE_ERR("Error reading dropped packets size counter for egress queue %d, rc %d\n",
            qentry->rdp_queue_index, rc);
        rc_to_ret = rc;
    }

    if (_rdpa_system_cfg_get()->counter_type == rdpa_counter_watermark)
    {
        stat->discarded.packets = cntr_arr[2];
        stat->discarded.bytes = cntr_arr[3];
    }
    else
    {
        rdpa_common_update_cntr_results_uint32(&(stat->discarded), &(accumulative_queue_stat[qentry->rdp_queue_index].discarded),
            _get_rdpa_stat_offset(rdpa_stat_pckts_id), cntr_arr[2]);

        rdpa_common_update_cntr_results_uint32(&(stat->discarded), &(accumulative_queue_stat[qentry->rdp_queue_index].discarded),
            _get_rdpa_stat_offset(rdpa_stat_bytes_id), cntr_arr[3]);
    }
    return rc_to_ret;
}

bdmf_error_t rdpa_rdd_tx_queue_stat_clear(struct bdmf_object *mo, tm_queue_hash_entry_t *qentry)
{
    int counter_index = qentry->rdp_queue_index * 2;
    qm_drop_counters_ctrl drop_ctrl;
    bdmf_boolean read_clear;
    uint32_t cnt;

    /* Clear DROP counters
     * - set read-clear mode
     * - read packet and byte counters
     * - restore read-clear mode
     */
    ag_drv_qm_drop_counters_ctrl_get(&drop_ctrl);
    read_clear = drop_ctrl.read_clear_bytes;
    drop_ctrl.read_clear_bytes = 1;
    drop_ctrl.read_clear_pkts = 1;
    ag_drv_qm_drop_counters_ctrl_set(&drop_ctrl);
    ag_drv_qm_drop_counter_get(counter_index, &cnt);
    ag_drv_qm_drop_counter_get(counter_index+1, &cnt);
    drop_ctrl.read_clear_bytes = read_clear;
    drop_ctrl.read_clear_pkts = read_clear;
    ag_drv_qm_drop_counters_ctrl_set(&drop_ctrl);

    /* clear counter */
    drv_cntr_counter_clr(CNTR_GROUP_TX_QUEUE, qentry->rdp_queue_index);
    qm_drop_counters[qentry->rdp_queue_index] = 0;
    memset(&accumulative_queue_stat[qentry->rdp_queue_index], 0, sizeof(rdpa_stat_1way_t));

    return BDMF_ERR_OK;
}


/* Get egress_tm object that owns the queue and queue_id given QM queue id */
bdmf_error_t rdpa_rdd_tx_queue_info_get(int rdd_queue_index, struct bdmf_object **p_owner, uint32_t *queue_id)
{
    tm_qtm_ctl_t *qtm_ctl;
    struct bdmf_object *owner;

    if ((unsigned)rdd_queue_index >= QM_QUEUES__NUM_OF)
        return BDMF_ERR_RANGE;

    qtm_ctl = qm_queue_info[rdd_queue_index].qtm_ctl;
    if (qtm_ctl == NULL)
        return BDMF_ERR_NOENT;

    owner = qtm_ctl->egress_tm;
    /* Paranoia check for the case qtm_ctl was destroyed by another thread */
    if (owner == NULL)
        return BDMF_ERR_NOENT;

    /* Get owner of the top-level egress_tm */
    owner = owner->owner;
    while (owner && owner->drv == rdpa_egress_tm_drv())
        owner = owner->owner;

    *queue_id = qm_queue_info[rdd_queue_index].queue_id;
    *p_owner = owner;

    return BDMF_ERR_OK;
}

static bdmf_error_t _rdpa_mbr_ug_thresholds_cfg(rdpa_traffic_dir dir)
{
    uint8_t i;
    uint32_t total_ug_reserved_tokens = 0;

    for (i = 0; i < QM_MBR_PROFILE__NUM_OF; i++)
    {
        if (dir)
            total_ug_reserved_tokens += mbr_profile[i].attached_us_q_num * mbr_profile[i].token_threshold;
        else
            total_ug_reserved_tokens += mbr_profile[i].attached_ds_q_num * mbr_profile[i].token_threshold;
    }

    return set_fpm_budget(FPM_RES_MBR, dir, total_ug_reserved_tokens);
}

/* [Allocate and] configure queue minimum buffer reservation profile */
bdmf_error_t rdpa_rdd_tm_queue_mbr_profile_cfg(struct bdmf_object *mo, rdpa_traffic_dir dir,
    const rdpa_tm_queue_cfg_t *old_queue_cfg, rdpa_tm_queue_cfg_t *new_queue_cfg,
    bdmf_boolean is_new, rdd_queue_profile_id_t *profile)
{
    int i;
    bdmf_error_t rc = BDMF_ERR_OK;
    uint32_t new_profile_index = QM_MBR_PROFILE__NUM_OF;

    /* find existing profile or allocate new one. Profile 0 must remain zero (irrelevant for 6858/46, default for 6856) */
    for (i = 1; i < QM_MBR_PROFILE__NUM_OF; i++)
    {
        /* save empty profile index in case we need to allocate new one */
        if ((mbr_profile[i].attached_ds_q_num + mbr_profile[i].attached_us_q_num) == 0)
            new_profile_index = i;

        /* look for existing profile */
        if (((mbr_profile[i].token_threshold + QM_MBR_PROFILE_RESOLUTION - 1)/QM_MBR_PROFILE_RESOLUTION) ==
        		(new_queue_cfg->reserved_packet_buffers + QM_MBR_PROFILE_RESOLUTION - 1)/QM_MBR_PROFILE_RESOLUTION)
        {
            new_profile_index = i;
            break;
        }
    }

    if (new_profile_index == QM_MBR_PROFILE__NUM_OF)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_NO_MORE, mo, "No available minimum buffer reservation profiles left\n");

    /* Update new profile */
    mbr_profile[new_profile_index].token_threshold = new_queue_cfg->reserved_packet_buffers;

    if (dir)
        mbr_profile[new_profile_index].attached_us_q_num++;
    else
        mbr_profile[new_profile_index].attached_ds_q_num++;

    /* Update UG thresholds*/
    rc = _rdpa_mbr_ug_thresholds_cfg(dir);
    /* Configure in QM */
    rc = rc ? rc : drv_qm_fpm_buffer_reservation_profile_cfg(new_profile_index, new_queue_cfg->reserved_packet_buffers);

    if (rc)
        goto err_cleanup;

    *profile = new_profile_index;

    BDMF_TRACE_DBG_OBJ(mo, "Set queue to new minimum buffer reservation profile dir %s index %d attached ds queues %d us queues %d\n",
        dir ? "us" : "ds", new_profile_index, mbr_profile[new_profile_index].attached_ds_q_num, mbr_profile[new_profile_index].attached_us_q_num);

    return rc;

err_cleanup:
    /* Try to reconfigure previous state */
    if (dir)
        mbr_profile[new_profile_index].attached_us_q_num--;
    else
        mbr_profile[new_profile_index].attached_ds_q_num--;

    _rdpa_mbr_ug_thresholds_cfg(dir);

    /* XXX - just to bypass the queue delete in case of range fails. */
    if (rc == BDMF_ERR_RANGE)
    {
        new_queue_cfg->reserved_packet_buffers = 0;
        *profile = 0;
        return BDMF_ERR_OK;
    }
    return rc;
}

/*
 * WRED support
 */

/* Convert thresholds to slope and mantissa
 *
 * drop_probability[7:0] =
 *   (mantissa[7:0] * (queue_occupancy[29:0] - min_threshold[23:0]*64)) >> exponent[4:0]
 *
 *   Calculation -
 *   (1) exponent set to max integer smaller than
 *         log((max_threshold - min_threshold) * RDPA_WRED_MAX_DROP_PROBABILITY / max_drop_probability)
 *   (2) mantissa = 256 * 2^exponent / (max_threshold - min_threshold)
 */
static void _rdpa_rdd_wred_calc_slope(uint32_t min_threshold, uint32_t max_threshold,
    uint32_t max_drop_probability, uint8_t *mantissa, uint8_t *exponent)
{
    uint32_t i, delta = (max_threshold - min_threshold) * RDPA_WRED_MAX_DROP_PROBABILITY / max_drop_probability;

    if (delta < 2)
    {
        *mantissa = *exponent = 0;
        return;
    }

    for (i = 0; (1 << (i + 1)) < delta; i++)
        ;

    *exponent = i;
    *mantissa = (RDPA_WRED_MAX_DROP_PROBABILITY << *exponent) / delta;
}

static bdmf_error_t _rdpa_wred_profile_cfg(uint8_t profile_id, rdd_queue_profile_t *queue_profile)
{
    qm_wred_profile_cfg qm_wred_profile_cfg = {
        .min_thr0 = queue_profile->low_priority_class.min_threshold / WRED_THRESHOLD_RESOLUTION,
        .min_thr1 = queue_profile->high_priority_class.min_threshold / WRED_THRESHOLD_RESOLUTION,
        .max_thr0 = queue_profile->low_priority_class.max_threshold / WRED_THRESHOLD_RESOLUTION,
        .max_thr1 = queue_profile->high_priority_class.max_threshold / WRED_THRESHOLD_RESOLUTION,
    };
    bdmf_error_t err;

    /* Configure QM WRED profile */
    _rdpa_rdd_wred_calc_slope(queue_profile->low_priority_class.min_threshold, queue_profile->low_priority_class.max_threshold,
        queue_profile->low_priority_class.max_drop_probability, &qm_wred_profile_cfg.slope_mantissa0, &qm_wred_profile_cfg.slope_exp0);
    _rdpa_rdd_wred_calc_slope(queue_profile->high_priority_class.min_threshold, queue_profile->high_priority_class.max_threshold,
        queue_profile->high_priority_class.max_drop_probability, &qm_wred_profile_cfg.slope_mantissa1, &qm_wred_profile_cfg.slope_exp1);

    err = ag_drv_qm_wred_profile_cfg_set(profile_id, &qm_wred_profile_cfg);

    return err;
}


/* [Allocate and] configure queue WRED profile */
bdmf_error_t rdpa_rdd_tm_queue_profile_cfg(struct bdmf_object *mo, rdpa_traffic_dir dir,
    const rdpa_tm_queue_cfg_t *old_queue_cfg, rdpa_tm_queue_cfg_t *new_queue_cfg,
    bdmf_boolean delete_profile, rdd_queue_profile_id_t *profile)
{
    rdd_queue_profile_t rdd_profile = {};
    bdmf_index new_profile_index, old_profile_index;

    /* If queue configure - remove the old configuration */
    if (delete_profile)
    {
            rdpa_rdd_tm_queue_profile_free(mo, dir, old_queue_cfg);
    }


    /* For XRDP we need WRED profiles even for tail-drop queues */
    if (new_queue_cfg->drop_alg == rdpa_tm_drop_alg_dt)
    {
        /* All thresholds are the same in case of tail-drop */
        new_queue_cfg->low_class.min_threshold = new_queue_cfg->drop_threshold;
        new_queue_cfg->low_class.max_threshold = new_queue_cfg->drop_threshold;
        new_queue_cfg->high_class.min_threshold = new_queue_cfg->drop_threshold;
        new_queue_cfg->high_class.max_threshold = new_queue_cfg->drop_threshold;
    }


    /* Find exist profile or allocate new one*/
    new_profile_index = rdpa_get_active_profile_index(new_queue_cfg);
    if (new_profile_index != BDMF_INDEX_UNASSIGNED)
    {
        /* Profile exist, just update the profile */
        wred_profile[new_profile_index].attached_q_num++;
        *profile = new_profile_index;
        BDMF_TRACE_DBG_OBJ(mo,
                        "Set queue to existing profile dir %s index %d."
                        "low threshold min %d low threshold max %d low max drop prob %d "
                        "high threshold min %d high threshold max %d high max drop prob %d "
                        "drop threshold %d attached queues %d\n", dir ? "us" : "ds", (int)new_profile_index,
                        wred_profile[new_profile_index].low_class.min_threshold,
                        wred_profile[new_profile_index].low_class.max_threshold,
                        wred_profile[new_profile_index].low_class.max_drop_probability,
                        wred_profile[new_profile_index].high_class.min_threshold,
                        wred_profile[new_profile_index].high_class.max_threshold,
                        wred_profile[new_profile_index].high_class.max_drop_probability,
                        wred_profile[new_profile_index].drop_threshold,
                        wred_profile[new_profile_index].attached_q_num);
        return 0;
    }
    else
    {
        /* Need to use new profile */
        /* Look for available profile index */
        new_profile_index = rdpa_get_available_profile_index(new_queue_cfg);
        if (new_profile_index == BDMF_INDEX_UNASSIGNED)
        {
            /* Need to increase attached q because count already decreased in rdpa_rdd_tm_queue_profile_free */
            if (delete_profile)
            {
                old_profile_index = rdpa_get_active_profile_index(old_queue_cfg);
                wred_profile[old_profile_index].attached_q_num++;
            }
            BDMF_TRACE_RET_OBJ(BDMF_ERR_NO_MORE, mo, "No available profiles left\n");
        }
    }

    if (new_queue_cfg->drop_alg == rdpa_tm_drop_alg_reserved)
    {
        rdd_profile.us_flow_control_mode = 1; /*, queues with this profile will work as flow control and NOT WRED */
        rdd_profile.high_priority_class.min_threshold = new_queue_cfg->high_class.min_threshold;
        rdd_profile.high_priority_class.max_threshold = new_queue_cfg->high_class.max_threshold;
    }
    else
    {
        rdd_profile.us_flow_control_mode = 0;
        rdd_profile.low_priority_class.min_threshold = new_queue_cfg->low_class.min_threshold;
        rdd_profile.low_priority_class.max_threshold = new_queue_cfg->low_class.max_threshold;
        rdd_profile.low_priority_class.max_drop_probability = new_queue_cfg->low_class.max_drop_probability ?
            new_queue_cfg->low_class.max_drop_probability : RDPA_WRED_MAX_DROP_PROBABILITY;

        if (new_queue_cfg->drop_alg == rdpa_tm_drop_alg_red)
        {
            rdd_profile.high_priority_class.min_threshold = new_queue_cfg->drop_threshold;
            rdd_profile.high_priority_class.max_threshold = new_queue_cfg->drop_threshold;
        }
        else
        {
            rdd_profile.high_priority_class.min_threshold = new_queue_cfg->high_class.min_threshold;
            rdd_profile.high_priority_class.max_threshold = new_queue_cfg->high_class.max_threshold;
        }

        rdd_profile.high_priority_class.max_drop_probability = new_queue_cfg->high_class.max_drop_probability ?
            new_queue_cfg->high_class.max_drop_probability : RDPA_WRED_MAX_DROP_PROBABILITY;
    }

    /* Configure in QM */
    _rdpa_wred_profile_cfg(new_profile_index, &rdd_profile);

    wred_profile[new_profile_index].drop_threshold = new_queue_cfg->drop_threshold;
    wred_profile[new_profile_index].high_class.min_threshold = new_queue_cfg->high_class.min_threshold;
    wred_profile[new_profile_index].high_class.max_threshold = new_queue_cfg->high_class.max_threshold;
    wred_profile[new_profile_index].high_class.max_drop_probability = new_queue_cfg->high_class.max_drop_probability;
    wred_profile[new_profile_index].low_class.min_threshold = new_queue_cfg->low_class.min_threshold;
    wred_profile[new_profile_index].low_class.max_threshold = new_queue_cfg->low_class.max_threshold;
    wred_profile[new_profile_index].low_class.max_drop_probability = new_queue_cfg->low_class.max_drop_probability;
    wred_profile[new_profile_index].attached_q_num++;

    *profile = new_profile_index;

    BDMF_TRACE_DBG_OBJ(mo, "Set queue to new profile dir %s index %d."
        "low threshold min %d low threshold max %d low max drop prob %d "
        "high threshold min %d high threshold max %d high max drop prob %d "
        "drop threshold %d attached queues %d\n", dir ? "us" : "ds", (int)new_profile_index,
        wred_profile[new_profile_index].low_class.min_threshold,
        wred_profile[new_profile_index].low_class.max_threshold,
        wred_profile[new_profile_index].low_class.max_drop_probability,
        wred_profile[new_profile_index].high_class.min_threshold,
        wred_profile[new_profile_index].high_class.max_threshold,
        wred_profile[new_profile_index].high_class.max_drop_probability,
        wred_profile[new_profile_index].drop_threshold,
        wred_profile[new_profile_index].attached_q_num);

    return 0;
}

/* Search for live profile according to queue cfg values */
bdmf_index rdpa_get_active_profile_index(const rdpa_tm_queue_cfg_t *queue_cfg)
{
    bdmf_index i;

    for (i = 0; i < QM_WRED_PROFILE__NUM_OF; i++)
    {
        if ((wred_profile[i].attached_q_num &&
            wred_profile[i].drop_threshold == queue_cfg->drop_threshold &&
            wred_profile[i].low_class.min_threshold == queue_cfg->low_class.min_threshold &&
            wred_profile[i].low_class.max_threshold == queue_cfg->low_class.max_threshold &&
            wred_profile[i].low_class.max_drop_probability == queue_cfg->low_class.max_drop_probability &&
            wred_profile[i].high_class.min_threshold == queue_cfg->high_class.min_threshold &&
            wred_profile[i].high_class.max_threshold == queue_cfg->high_class.max_threshold &&
            wred_profile[i].high_class.max_drop_probability == queue_cfg->high_class.max_drop_probability) ||
            (queue_cfg->drop_alg == rdpa_tm_drop_alg_dt &&
            wred_profile[i].attached_q_num &&
            wred_profile[i].drop_threshold == queue_cfg->drop_threshold &&
            wred_profile[i].low_class.max_drop_probability == queue_cfg->low_class.max_drop_probability &&
            wred_profile[i].high_class.max_drop_probability == queue_cfg->high_class.max_drop_probability))
        {
            return i;
        }
    }
    return BDMF_INDEX_UNASSIGNED;
}

/* Search for available profile to use */
bdmf_index rdpa_get_available_profile_index(const rdpa_tm_queue_cfg_t *queue_cfg)
{
    bdmf_index i;

    for (i = 0; i < QM_WRED_PROFILE__NUM_OF; i++)
    {
        if (!wred_profile[i].attached_q_num)
            return i;
    }
    return BDMF_INDEX_UNASSIGNED;
}

/* Unreference queue WRED profile */
bdmf_error_t rdpa_rdd_tm_queue_profile_free(struct bdmf_object *mo, rdpa_traffic_dir dir,
    const rdpa_tm_queue_cfg_t *queue_cfg)
{
    bdmf_index profile_index;

    profile_index = rdpa_get_active_profile_index(queue_cfg);
    if (profile_index == BDMF_INDEX_UNASSIGNED)
    {
        BDMF_TRACE_DBG_OBJ(mo, "Can't find profile\n");
        return BDMF_ERR_INTERNAL;
    }

    /* If this is last queue using this profile - remove it */
    if (wred_profile[profile_index].attached_q_num <= 1)
    {
        wred_profile[profile_index].attached_q_num = 0;
        wred_profile[profile_index].drop_threshold = 0;
        wred_profile[profile_index].low_class.min_threshold = 0;
        wred_profile[profile_index].low_class.max_threshold = 0;
        wred_profile[profile_index].low_class.max_drop_probability = RDPA_WRED_MAX_DROP_PROBABILITY;
        wred_profile[profile_index].high_class.min_threshold = 0;
        wred_profile[profile_index].high_class.max_threshold = 0;
        wred_profile[profile_index].high_class.max_drop_probability = RDPA_WRED_MAX_DROP_PROBABILITY;
    }
    else
    {
        wred_profile[profile_index].attached_q_num--;
    }

    BDMF_TRACE_DBG_OBJ(mo, "removed queue from profile dir %s index %d. "
        "remaining attached queues %d\n", dir ? "us" : "ds", (int)profile_index,
        wred_profile[profile_index].attached_q_num);

    return 0;
}

/*
 * Overall rate limiter support
 */

/* Add / remove channel to overlall rate limiter */
bdmf_error_t rdpa_rdd_orl_channel_cfg(struct bdmf_object *mo, const tm_channel_t *channel,
    bdmf_boolean rate_limiter_enabled, rdpa_tm_orl_prty prio)
{
    uint8_t bbh_queue = channel->channel_id;
    bdmf_error_t rc;

#ifndef XRDP_BBH_PER_LAN_PORT
#if !defined(BCM_DSL_XRDP)
    if (rdpa_is_gbe_mode() && channel->dir == rdpa_dir_us)
        bbh_queue = rdpa_gbe_wan_emac();
#endif
#endif

    if (rate_limiter_enabled)
    {
        rc = rdd_overall_rate_limiter_bbh_queue_cfg(bbh_queue, (prio == rdpa_tm_orl_prty_high));
        BDMF_TRACE_RET_OBJ(rc, mo, "rdd_overall_rate_limiter_bbh_queue_cfg(%u, %d) -> %d\n",
            bbh_queue, prio == rdpa_tm_orl_prty_high, rc);
    }
    else
    {
        rc = rdd_overall_rate_limiter_remove(bbh_queue);
        BDMF_TRACE_RET_OBJ(rc, mo, "rdd_overall_rate_limiter_remove(%u) -> %d\n", bbh_queue, rc);
    }
}

/* Configure overall rate limiter rate */
bdmf_error_t rdpa_rdd_orl_rate_cfg(struct bdmf_object *mo, uint32_t rate)
{
    bdmf_error_t rc;

    rc = rdd_overall_rate_limiter_rate_cfg(rate, rate);
    BDMF_TRACE_RET_OBJ(rc, mo, "rdd_overall_rate_limiter_rate_cfg({rate=%d, limit=%d}})\n",
        (int)rate, (int)rate);
}

int egress_tm_drv_init_ex()
{
    memset(&accumulative_queue_stat[0], 0, sizeof(accumulative_queue_stat));
    memset(&wred_profile[0], 0, sizeof(wred_profile));
    memset(&mbr_profile[0], 0, sizeof(mbr_profile));

    return 0;
}


bdmf_error_t rdpa_egress_tm_queue_info_get(bdmf_object_handle egress_tm, bdmf_index queue, queue_info_t *queue_id_info)
{
    bdmf_error_t rc = BDMF_ERR_PARM;
    int rc_id, qm_queue, channel = 0;
    tm_channel_t *ch = NULL;
    tm_drv_priv_t *tm;

    if (!egress_tm)
        return rc;

    tm = bdmf_obj_data(egress_tm);

    if (egress_tm_is_group_owner(tm))
    {
        channel = (int)tm->channel_group;
        rc = _rdpa_egress_tm_channel_queue_to_rdd(rdpa_dir_us, channel, (uint32_t)queue,
            &rc_id, &qm_queue);
    }

    while ((ch = egress_tm_channel_get_next(egress_tm, ch)) && rc)
    {
        channel = (int)ch->channel_id;
        rc = _rdpa_egress_tm_channel_queue_to_rdd(rdpa_dir_us, channel, (uint32_t)queue,
            &rc_id, &qm_queue);
    }

    if (!rc)
    {
        queue_id_info->channel = channel;
        queue_id_info->rc_id = rc_id;
        queue_id_info->queue = qm_queue;
    }

    return rc;
}
