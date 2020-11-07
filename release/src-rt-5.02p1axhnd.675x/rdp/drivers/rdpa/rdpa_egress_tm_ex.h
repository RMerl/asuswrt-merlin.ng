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
 * rdpa_egress_tm_ex.h
 *
 * rdpa_egress_tm interface toward RDP / XRDP-specific RDD implementations.
 */

#ifndef RDPA_EGRESS_TM_EX_H_
#define RDPA_EGRESS_TM_EX_H_

/* Unlimited rate */
#if defined(WL4908)
#define RDD_RATE_UNLIMITED      0xBB800000
#else
#ifndef CONFIG_ARM64
#define RDD_RATE_UNLIMITED      0x7FFF0000
#else
#define RDD_RATE_UNLIMITED      0x260000000
#endif
#endif

#ifndef G9991
#define RDPA_MAX_QUEUE_COUNTERS         130
#else
#define RDPA_MAX_QUEUE_COUNTERS         160
#endif

/* weight quantum */
#define RDD_WEIGHT_QUANTUM      256

/* rate quantum */
#define RDD_RATE_QUANTUM        250

#define BITS_IN_BYTE            8

#ifdef RDP_SIM
#define RDPA_EPON_CONTROL_QUEUE_ID             101
#endif

/* Scheduling configuration */
typedef struct rdd_sched_cfg_t
{
    rdpa_tm_level_type level;
    rdpa_tm_sched_mode mode;
    rdpa_tm_rl_cfg_t rl_cfg;
    rdpa_tm_rl_rate_mode rl_rate_mode;
    uint8_t num_queues;
    rdpa_tm_num_sp_elem num_sp_elements;
    uint32_t weight;
} rdd_sched_cfg_t;

/* Queue configuration */
typedef struct
{
    uint32_t packet_threshold;
    rdd_queue_profile_id_t profile;
    rdd_queue_profile_id_t mbr_profile;
    uint32_t counter_id;
    rdpa_tm_rl_cfg_t rl_cfg;
    uint32_t weight;
    bdmf_boolean exclusive;
} rdd_queue_cfg_t;

/* Max number of sub-channels in WAN channel group (LLID) */
#define RDPA_MAX_WAN_SUBCHANNELS        9

/* egress tm object private data */
typedef struct
{
    bdmf_index index; /* egress tm index */
    rdpa_traffic_dir dir; /* US / DS (must for overall rate limiter) */
    rdpa_wan_type wan_type; /* If US, the type of WAN interface */
    rdpa_tm_level_type level; /* Scheduler level */
    rdpa_tm_sched_mode mode; /* Scheduler mode */
    rdpa_tm_enable_cfg_t enable_cfg; /* enable/disable + flush*/
    bdmf_boolean overall_rl; /* overall RL */
    rdpa_tm_service_queue_t service_q; /* service queue */
    void *service_queue_ddr_addr; /* service queue address */
    rdpa_tm_rl_cfg_t rl_cfg; /* Rate limiter configuration */
    uint8_t num_queues; /* Number of queues */
    rdpa_tm_num_sp_elem num_sp_elements; /* Number of SP scheduling elements for SP_WRR mode */

    /* Subsidiary egress tms */
    struct bdmf_object *sub_tms[RDPA_TM_MAX_SCHED_ELEMENTS];
    /* Subsidiary queues */
    rdpa_tm_queue_cfg_t queue_cfg[RDPA_MAX_EGRESS_QUEUES];
    pd_offload_ddr_queue_t ddr_queue[RDPA_MAX_EGRESS_QUEUES];
    /* 1 if queue id is assigned */
    bdmf_boolean queue_id_assigned[RDPA_MAX_EGRESS_QUEUES];
    /* # of times queue is configured in RDD
     * (0..num_of_channels_bound_to_hierarchy) */
    int queue_configured[RDPA_MAX_EGRESS_QUEUES];
    uint32_t weight; /* Weight for WRR scheduling */
    rdpa_tm_orl_prty orl_prty; /* Priority for overall rate limiter */
    rdpa_tm_rl_rate_mode rl_rate_mode; /* support BE rate in subsidiaries */

    /* Fields for channel group assignment (LLID)
     * Channel group is configured as root_egress_tm with subsidiaries,
     * whereas each subsidiary represents WAN subchannel.
     * The following fields are set only in the root egress_tm object
     */
    int16_t channel_group; /* channel group id */
    int16_t num_channels; /* number of channels in channel group */
    int16_t channels[RDPA_MAX_WAN_SUBCHANNELS]; /* WAN channel numbers */
    /* Object that owns channel group (ie, LLID object) */
    bdmf_object_handle channel_group_owner;

    struct bdmf_object *this; /* This TM reference */

    /* Back reference to upper level egress tm object */
    struct bdmf_object *upper_level_tm;
} tm_drv_priv_t;

/*
 * Channel configuration
 */
bdmf_error_t rdpa_rdd_top_sched_create(struct bdmf_object *mo, tm_channel_t *channel, const rdd_sched_cfg_t *cfg);
void rdpa_rdd_top_sched_destroy(struct bdmf_object *mo, tm_channel_t *channel);

/*
 * Per q-level scheduler [ & rate controller ]
 */
bdmf_error_t rdpa_rdd_qtm_ctl_create(struct bdmf_object *mo, tm_qtm_ctl_t *qtm_ctl, const rdd_sched_cfg_t *cfg);
void rdpa_rdd_qtm_ctl_destroy(struct bdmf_object *mo, tm_qtm_ctl_t *qtm_ctl);
bdmf_error_t rdpa_rdd_qtm_ctl_modify(struct bdmf_object *mo, tm_qtm_ctl_t *qtm_ctl,
    const rdpa_tm_rl_cfg_t *cfg, int weight, int num_sp_queues);

/*
 * Queue management
 */
bdmf_error_t rdpa_rdd_tx_queue_create(struct bdmf_object *mo, rdpa_wan_type wan_type,
    tm_queue_hash_entry_t *qentry, const rdd_queue_cfg_t *queue_cfg, pd_offload_ddr_queue_t *ddr_cfg, bdmf_boolean enable);

bdmf_error_t rdpa_rdd_tx_queue_modify(struct bdmf_object *mo, tm_queue_hash_entry_t *qentry,
    const rdd_queue_cfg_t *queue_cfg, pd_offload_ddr_queue_t *ddr_cfg, bdmf_boolean enable);

void rdpa_rdd_tx_queue_destroy(struct bdmf_object *mo, tm_queue_hash_entry_t *qentry,
    pd_offload_ddr_queue_t *ddr_cfg, int i);

int egress_tm_queue_cfg_on_channel(struct bdmf_object *mo,
    tm_channel_t *channel, int index, rdpa_tm_queue_cfg_t *new_queue_cfg, bdmf_boolean enable);

int rdpa_rdd_tx_queue_disable(struct bdmf_object *mo, tm_queue_hash_entry_t *qentry,
    pd_offload_ddr_queue_t *ddr_cfg);

bdmf_error_t rdpa_rdd_tx_queue_flush(struct bdmf_object *mo, tm_queue_hash_entry_t *qentry,
    bdmf_boolean is_wait);

bdmf_error_t rdpa_rdd_tx_queue_stat_read(struct bdmf_object *mo, tm_queue_hash_entry_t *qentry,
    rdpa_stat_1way_t *stat);

bdmf_error_t rdpa_rdd_tx_queue_stat_clear(struct bdmf_object *mo, tm_queue_hash_entry_t *qentry);

bdmf_error_t rdpa_rdd_tx_queue_channel_attr_update(const channel_attr *attr, int queue_index);
/*
 * WRED support
 */

static inline int rdpa_rdd_wred_param_modified(const rdpa_tm_queue_cfg_t *old_queue_cfg,
    const rdpa_tm_queue_cfg_t *new_queue_cfg)
{
    if (old_queue_cfg->drop_threshold == new_queue_cfg->drop_threshold &&
        old_queue_cfg->low_class.min_threshold == new_queue_cfg->low_class.min_threshold &&
        old_queue_cfg->low_class.max_threshold == new_queue_cfg->low_class.max_threshold &&
        old_queue_cfg->low_class.max_drop_probability == new_queue_cfg->low_class.max_drop_probability &&
        old_queue_cfg->high_class.min_threshold == new_queue_cfg->high_class.min_threshold &&
        old_queue_cfg->high_class.max_threshold == new_queue_cfg->high_class.max_threshold &&
        old_queue_cfg->high_class.max_drop_probability == new_queue_cfg->high_class.max_drop_probability)
    {
        return 0;
    }
    return 1;
}

/* egress_tm wred */
typedef struct {
    uint32_t drop_threshold;              /* Drop threshold (queue size) */
    rdpa_tm_priority_class_t low_class;   /* low threshold */
    rdpa_tm_priority_class_t high_class;  /* high threshold */
    uint32_t attached_q_num;              /* queues holding this wred profile*/
} rdpa_tm_wred_profile;

/* [Allocate and] configure queue WRED profile */
#ifdef XRDP
bdmf_error_t rdpa_rdd_tm_queue_profile_cfg(struct bdmf_object *mo, rdpa_traffic_dir dir,
    const rdpa_tm_queue_cfg_t *old_queue_cfg, rdpa_tm_queue_cfg_t *new_queue_cfg,
    bdmf_boolean delete_profile, rdd_queue_profile_id_t *profile);
#else
bdmf_error_t rdpa_rdd_tm_queue_profile_cfg(struct bdmf_object *mo, rdpa_traffic_dir dir,
    const rdpa_tm_queue_cfg_t *old_queue_cfg, rdpa_tm_queue_cfg_t *new_queue_cfg,
    bdmf_boolean is_new, rdd_queue_profile_id_t *profile);
#endif

/* Unreference queue WRED profile */
bdmf_error_t rdpa_rdd_tm_queue_profile_free(struct bdmf_object *mo, rdpa_traffic_dir dir,
    const rdpa_tm_queue_cfg_t *queue_cfg);

/* [Allocate and] configure queue WRED profile */
bdmf_error_t rdpa_rdd_tm_queue_mbr_profile_cfg(struct bdmf_object *mo, rdpa_traffic_dir dir,
    const rdpa_tm_queue_cfg_t *old_queue_cfg, rdpa_tm_queue_cfg_t *new_queue_cfg,
    bdmf_boolean is_new, rdd_queue_profile_id_t *profile);
    
/*
 * Overall rate limiter
 */

/* Add / remove channel to overlall rate limiter */
bdmf_error_t rdpa_rdd_orl_channel_cfg(struct bdmf_object *mo, const tm_channel_t *channel,
    bdmf_boolean rate_limiter_enabled, rdpa_tm_orl_prty prio);

/* Configure overall rate limiter rate.
 * rate is in bytes/s
 */
bdmf_error_t rdpa_rdd_orl_rate_cfg(struct bdmf_object *mo, uint32_t rate);

/*
 * The following functions are not supported on 6858 [yet].
 * Mapped 1x1 with existing RDD implementation for now
 */

int egress_tm_queue_occupancy_read_ex(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size);
bdmf_error_t egress_tm_is_empty_on_channel_ex(bdmf_object_handle tm_obj, uint32_t channel_index, bdmf_boolean *is_empty);
int egress_tm_allocate_counter(struct bdmf_object *mo, tm_qtm_ctl_t *qtm_ctl, rdpa_tm_queue_cfg_t *new_queue_cfg, int index);
tm_channel_t *egress_tm_channel_get_first(struct bdmf_object *mo);
bdmf_object_handle egress_tm_get_root_object(bdmf_object_handle mo);
bdmf_boolean egress_tm_is_service_q(tm_drv_priv_t *tm);
bdmf_boolean egress_tm_is_group_owner(tm_drv_priv_t *tm);
bdmf_error_t rdpa_egress_tm_queue_info_get(bdmf_object_handle egress_tm, bdmf_index queue, queue_info_t *queue_id_info);

#ifdef XRDP
bdmf_error_t rdpa_rdd_tx_queue_info_get(int rdd_queue_index, struct bdmf_object **p_owner, uint32_t *queue_id);
bdmf_index _rdpa_rdd_get_queue_idx(tm_channel_t *channel, tm_qtm_ctl_t *qtm_ctl);
#endif

int egress_tm_post_init_ex(struct bdmf_object *mo);
int egress_tm_drv_init_ex(void);
tm_channel_t *egress_tm_channel_get(rdpa_traffic_dir dir, int channel);
/* Extra definitions needed for successful build */
#define INVALID_COUNTER_ID                                       0

#endif /* RDPA_EGRESS_TM_EX_H_ */
