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
 * rdpa_egress_tm.c
 *
 *  Created on: Aug 17, 2012
 *      Author: igort
 */

#include "bdmf_dev.h"
#include "rdpa_api.h"
#include "rdpa_common.h"
#include "rdd.h"
#include "rdpa_egress_tm_inline.h"
#include "rdpa_egress_tm_ex.h"
#ifndef XRDP
#include "rdd_tm.h"
#include "rdpa_platform.h"
#ifdef LEGACY_RDP
#include "rdd_legacy_conv.h"
#endif
#include "rdd_service_queues.h"
#else
#include "xrdp_drv_qm_ag.h"
#include "rdp_drv_qm.h"
#include "rdd_runner_proj_defs.h"
#endif
#include "rdpa_int.h"
#if defined(CONFIG_BCM_PON) || defined(CONFIG_BCM963158)
#include <rdpa_epon.h>
#endif

/* Max number of scheduling levels */
#define EGRESS_TM_MAX_SCHED_LEVELS      2

/* DS service queue channel */
#define RDPA_DS_SERVICE_Q_CHANNEL       (RDPA_MAX_DS_CHANNELS - 1)

/*us overall rl egress_tm index, used for get us_overall_rl_obj*/
#define OVERALL_RL_TM_INDEX          -1

/***************************************************************************
 * egress_tm object type
 **************************************************************************/

/* Overview:
 * - egress_tm objects form an hierarchy using "subsidiary" attribute
 *   (subs_tms[] in tm_drv_priv_t structure)
 * - top-level object in hierarchy can be "bound" to multiple channels (TCONTs,
 *   PORTs) using _rdpa_egress_tm_channel_set()
 * - channel contexts (tm_channel_t) are pre-allocated in per-direction arrays
 *   us_channels, ds_channels
 *      - egress_tm and owner fields in unbound channel context are NULL.
 *              - egress_tm is top objecvt in hierarchy
 *              - owner is port/tcont object that "owns" the channel
 *      - each channel context points to LL of dynamically-allocated rate
 *        controller contexts (tm_qtm_ctl_t).
 *      - channel context also contains RDD's channel_id
 * - rate controller context (tm_qtm_ctl_t) is allocated per channel x per
 *   queue-level egress_tm
 *      - rate controller context contains per-queue array of hash entries which
 *        are inserted into global hash table when queue is configured
 *      - rate controller context also contains RDD's rc_id
 * - global hash table tm_queue_hash[] is used for fast mapping
 *   { dir, channel_id, queue_id } --> { rc_id, priority }
 *
 * - RDD-related resources, including rate controller contexts are allocated
 *   when channel is bound
 * - RDD queues are allocated and added to hash, when
 *   queue_cfg[].drop_thresahold on bound egress_tm is set > 0
 */

/* hash table for mapping (channel,queue) --> (egress_tm, queue_index) */

/* hash entry. stored inside tm_drv_priv_t structure.
 * Therefore, tm_drv_priv_t pointer is recoverable given the tm_queue_hash_entry pointer
 */

DEFINE_BDMF_FASTLOCK(tm_hash_lock_irq);

static struct bdmf_object *us_tm_objects[RDPA_TM_MAX_US_SCHED];
static struct bdmf_object *ds_tm_objects[RDPA_TM_MAX_DS_SCHED];
static struct bdmf_object *us_overall_rl_obj;

/* count the number of non service queue egress_tm */
static int num_normal_ds_tm;
static int num_normal_us_tm;

#ifdef EPON
static bdmf_boolean orl_tm_linked_to_llid; /* flag shows if "ORL TM" (only 1 TM configs
                                              ORL) is linked to LLID*/
#endif

/* US / DS channel contexts */
static tm_channel_t us_channels[RDPA_MAX_US_CHANNELS];
static tm_channel_t ds_channels[RDPA_MAX_DS_CHANNELS];

/* US / DS channel/queue hash entry shortcut lookup table */
static tm_queue_hash_entry_t *us_hash_lookup[RDPA_MAX_US_CHANNELS][RDPA_TM_MAX_SCHED_ELEMENTS * RDPA_MAX_EGRESS_QUEUES];
static tm_queue_hash_entry_t *ds_hash_lookup[RDPA_MAX_DS_CHANNELS][RDPA_TM_MAX_SCHED_ELEMENTS * RDPA_MAX_EGRESS_QUEUES];
/*
 * queue hash array
 */

/*call back pointer to register EPON stack gloabl shaper*/
epon_global_shaper_cb_t global_shaper_cb;
EXPORT_SYMBOL(global_shaper_cb);

/*call back pointer to register EPON stack gloabl shaper*/
epon_link_shaper_cb_t epon_link_shaper_cb;
EXPORT_SYMBOL(epon_link_shaper_cb);

uint8_t queue_counters[2][RDPA_MAX_QUEUE_COUNTERS];
uint8_t queue_counters_idx[2] = {0, 0};
#ifdef XRDP
extern dpi_params_t *p_dpi_cfg;
extern bdmf_index rdpa_get_available_profile_index(const rdpa_tm_queue_cfg_t *queue_cfg);
extern bdmf_index rdpa_get_active_profile_index(const rdpa_tm_queue_cfg_t *queue_cfg);
#endif

struct tm_queue_hash_head tm_queue_hash[TM_QUEUE_HASH_SIZE];
static struct tm_queue_hash_entry *egress_tm_hash_delete(bdmf_object_handle mo, uint32_t dir_channel, uint32_t queue_id);
void egress_tm_delete_single_queue(struct bdmf_object *mo, tm_qtm_ctl_t *qtm_ctl, tm_channel_t *channel, tm_drv_priv_t *tm, int i);
static void egress_tm_enable_store(tm_drv_priv_t *tm, bdmf_boolean enable);

#ifndef XRDP
extern int egress_tm_service_queue_rl_config(struct bdmf_object *mo,
    const rdpa_tm_rl_cfg_t *rl_cfg, bdmf_boolean enable);
#endif

static tm_drv_priv_t *egress_tm_get_top(tm_drv_priv_t *tm)
{
    while (tm->upper_level_tm)
        tm = (tm_drv_priv_t *)bdmf_obj_data(tm->upper_level_tm);
    return tm;
}

static bdmf_object_handle egress_tm_get_top_object(bdmf_object_handle mo)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    return egress_tm_get_top(tm)->this;
}

/* Egress_tm with group owner. in this mode queue_id is unique per group_id rather than channel
 * Used for LLID control and data channels
 */
inline bdmf_boolean egress_tm_is_group_owner(tm_drv_priv_t *tm)
{
    return (tm->channel_group_owner != NULL);
}

/* Egress_tm contains sub-trees bound to different channels in channel group
 * Used for LLID data
 */
static inline bdmf_boolean egress_tm_is_ch_group(tm_drv_priv_t *tm)
{
    return (tm->channel_group_owner != NULL && tm->level == rdpa_tm_level_egress_tm);
}

inline bdmf_boolean egress_tm_is_service_q(tm_drv_priv_t *tm)
{
    return tm->service_q.enable;
}

/* root object differs from top object in case of channel group (LLID),
 * whereas each root object's subsidiary is considered top object in its hierarchy
 */
bdmf_object_handle egress_tm_get_root_object(bdmf_object_handle mo)
{
    while (mo->owner && mo->owner->drv == rdpa_egress_tm_drv())
        mo = mo->owner;
    return mo;
}

inline tm_queue_hash_entry_t *ds_hash_index_get(uint32_t channel, uint32_t queue_id)
{
    return ds_hash_lookup[channel][queue_id];
}

inline tm_queue_hash_entry_t *us_hash_index_get(uint32_t channel, uint32_t queue_id)
{
    return us_hash_lookup[channel][queue_id];
}

/*
 * Resource management helpers
 */

/*
 * Channel helpers
 */

/* Get channel by dir, channel id */
tm_channel_t *egress_tm_channel_get(rdpa_traffic_dir dir, int channel)
{
    tm_channel_t *channels;
    int max_channels;
    if (dir == rdpa_dir_ds)
    {
        max_channels = RDPA_MAX_DS_CHANNELS;
        channels = ds_channels;
    }
    else
    {
        max_channels = RDPA_MAX_US_CHANNELS;
        channels = us_channels;
    }
    if ((unsigned)channel >= max_channels)
        return NULL;
    return &channels[channel];
}

/* The function returns pointer of the 1st/next channel bound to egress_tm object.
 * returns NULL is non is bound
 */
tm_channel_t *egress_tm_channel_get_next(struct bdmf_object *mo, tm_channel_t *ch)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    struct bdmf_object *top;
    tm_channel_t *channels;
    int num_channels;
    int i = 0;

    /* Special handling of channel-group TM.
     * Go over sub-channels
     */
    if (egress_tm_is_ch_group(tm))
    {
        /* Find "*this" sub-channel and look up from the next */
        if (ch)
        {
            struct bdmf_object *cur = ch->egress_tm;
            for (i = 0; i < tm->num_channels && cur != tm->sub_tms[i]; i++)
                ;
            ch = NULL;
            ++i;
        }
        for (; i < tm->num_channels && !ch; i++)
        {
            if (tm->sub_tms[i])
                ch = egress_tm_channel_get_next(tm->sub_tms[i], NULL);
        }
        return ch;
    }

    /* Go over all channels find the 1st one bound to the top egress_tm */
    top = egress_tm_get_top_object(mo);
    channels = (tm->dir == rdpa_dir_ds) ? ds_channels : us_channels;
    num_channels = (tm->dir == rdpa_dir_ds) ? RDPA_MAX_DS_CHANNELS : RDPA_MAX_US_CHANNELS;
    i = ch ? ch->channel_id + 1 : 0;
    for (; i < num_channels; i++)
    {
        ch = &channels[i];
        if (ch->egress_tm == top)
            return ch;
    }

    return NULL;
}

/* The function returns pointer of the 1st channel bound to egress_tm object.
 * returns NULL is non is bound
 */
inline tm_channel_t *egress_tm_channel_get_first(struct bdmf_object *mo)
{
    return egress_tm_channel_get_next(mo, NULL);
}

/* Get rate controller context owned by specific egress_tm object */
tm_qtm_ctl_t *egress_tm_qtm_ctl_get(struct bdmf_object *mo,
    tm_channel_t *channel)
{
    tm_qtm_ctl_t *qtm_ctl;

    STAILQ_FOREACH(qtm_ctl, &channel->qtm_ctls, list)
    {
        if (qtm_ctl->egress_tm == mo)
            return qtm_ctl;
    }
    return NULL;
}

/* Allocate scheduler/rate controller context */
static tm_qtm_ctl_t *egress_tm_qtm_ctl_ctx_alloc(struct bdmf_object *mo, tm_channel_t *channel, int16_t sub_index)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    tm_qtm_ctl_t *qtm_ctl = bdmf_calloc(sizeof(tm_qtm_ctl_t));
    int i;

    if (!qtm_ctl)
        return NULL;
    qtm_ctl->rc_id = BDMF_INDEX_UNASSIGNED;
    qtm_ctl->channel = channel;
    qtm_ctl->egress_tm = mo;
    qtm_ctl->sched_index_in_upper = sub_index;
    qtm_ctl->sched_weight = tm->weight;
    qtm_ctl->rl_rate_mode = tm->rl_rate_mode;

    for (i = 0; i < tm->num_queues; i++)
    {
        qtm_ctl->hash_entry[i].queue_index = i;
        qtm_ctl->hash_entry[i].rdp_queue_index = -1;
        memset(&qtm_ctl->queue_cfg[i], 0x0, sizeof(rdpa_tm_queue_cfg_t));
    }
    STAILQ_INSERT_TAIL(&channel->qtm_ctls, qtm_ctl, list);
    return qtm_ctl;
}

/* Free rate controller context */
static void egress_tm_qtm_ctl_ctx_free(tm_qtm_ctl_t *qtm_ctl)
{
    STAILQ_REMOVE(&qtm_ctl->channel->qtm_ctls, qtm_ctl, tm_qtm_ctl, list);
    bdmf_free(qtm_ctl);
}

/* Calculate total number of scheduling levels in hierarchy */
static int egress_tm_num_levels(struct bdmf_object *mo, int level)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    int max_sub_levels = 0;
    int sub_levels;
    int ns;

#ifdef XRDP
    if (tm->mode == rdpa_tm_sched_disabled)
        return 1;
#else
    if (tm->mode == rdpa_tm_sched_disabled)
        return 0;
#endif

    if (tm->level != rdpa_tm_level_egress_tm)
        return 1;

    /* Prevent infinite recursion */
    if (level > EGRESS_TM_MAX_SCHED_LEVELS)
        return level;

    for (ns = 0; ns < RDPA_TM_MAX_SCHED_ELEMENTS; ns++)
    {
        if (tm->sub_tms[ns])
        {
            sub_levels = egress_tm_num_levels(tm->sub_tms[ns], level + 1);
            if (sub_levels > max_sub_levels)
                max_sub_levels = sub_levels;
        }
    }

    return max_sub_levels + 1;
}

/* Check total number of levels in hierarchy */
static int egress_tm_check_levels(struct bdmf_object *mo)
{
    int levels = egress_tm_num_levels(egress_tm_get_top_object(mo), 0);
    if (levels > EGRESS_TM_MAX_SCHED_LEVELS)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_NOT_SUPPORTED, mo, "Max number of scheduling levels (%d) exceeded\n",
            EGRESS_TM_MAX_SCHED_LEVELS);
    }
    return 0;
}

/* Check if scheduling element should be WRR-scheduled */
static bdmf_boolean egress_tm_is_wrr_elem(tm_drv_priv_t *tm, bdmf_index index)
{
    if (tm->mode == rdpa_tm_sched_wrr)
        return 1;
    /* For SP_WRR mode, scheduling element is WRR if
     * - index >= number of SP elements
     * - scheduler is dual rate. In this case all elements are scheduled in WRR for PIR
     */
    if (tm->mode == rdpa_tm_sched_sp_wrr &&
         (index >= tm->num_sp_elements || tm->rl_rate_mode == rdpa_tm_rl_dual_rate))
        return 1;
    return 0;
}

/* Service queue ID to index conversion */
int egress_tm_svcq_queue_index_get(uint32_t queue_id)
{
    tm_channel_t *chan = egress_tm_channel_get(rdpa_dir_ds, RDPA_DS_SERVICE_Q_CHANNEL);
    tm_drv_priv_t *tm = (tm_drv_priv_t *)(chan && chan->egress_tm ? bdmf_obj_data(chan->egress_tm) : NULL);
    tm_drv_priv_t *sub_tm = NULL;
    int i;

    if (queue_id == BDMF_INDEX_UNASSIGNED || !tm)
        return BDMF_INDEX_UNASSIGNED;

    if (tm->level == rdpa_tm_level_queue)
    {
        for (i = 0; i < tm->num_queues; i++)
        {
            if (!tm->queue_cfg[i].drop_threshold)
                continue;
            if (tm->queue_cfg[i].queue_id == queue_id)
                return i;
        }
    }
    else
    {
        for (i = 0; i < RDPA_TM_MAX_SCHED_ELEMENTS; i++)
        {
            if (!tm->sub_tms[i])
                continue;
            sub_tm = (tm_drv_priv_t *)bdmf_obj_data(tm->sub_tms[i]);
            if (!sub_tm->queue_cfg[0].drop_threshold)
                continue;
            if (sub_tm->queue_cfg[0].queue_id == queue_id)
                return i;
        }
    }

    BDMF_TRACE_DBG("egress_tm_service_q_idx_get: Service queue %d not configured\n", queue_id);
    return BDMF_INDEX_UNASSIGNED;
}

int egress_tm_svcq_queue_id_get(uint32_t queue_index)
{
    tm_channel_t *chan = egress_tm_channel_get(rdpa_dir_ds, RDPA_DS_SERVICE_Q_CHANNEL);
    tm_drv_priv_t *tm;
    tm_drv_priv_t *sub_tm = NULL;

    if ((chan) && (!(chan->egress_tm)))
    {
        BDMF_TRACE_ERR("egress_error, channel exist but not egress_tm\n");
        return BDMF_INDEX_UNASSIGNED;
    }

    tm = (tm_drv_priv_t *)(chan ? bdmf_obj_data(chan->egress_tm) : NULL);

    if (queue_index == BDMF_INDEX_UNASSIGNED || !tm)
        return BDMF_INDEX_UNASSIGNED;

    if (tm->level == rdpa_tm_level_queue)
    {
        if (!tm->queue_cfg[queue_index].drop_threshold)
            goto out;
        return tm->queue_cfg[queue_index].queue_id;
    }
    else
    {
        if (!tm->sub_tms[queue_index])
            goto out;
        sub_tm = (tm_drv_priv_t *)bdmf_obj_data(tm->sub_tms[queue_index]);
        if (!sub_tm->queue_cfg[0].drop_threshold)
            goto out;
        return sub_tm->queue_cfg[0].queue_id;
    }

out:
    return BDMF_INDEX_UNASSIGNED;
}

/* Validate scheduling hierarchy */
static int egress_tm_validate_hierarchy(struct bdmf_object *mo)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    int ns;
    int rc = 0;

    /* channel_id must be set for SP/WRR scheduler */
    if (!tm->overall_rl && tm->upper_level_tm && !egress_tm_channel_get_first(mo))
        BDMF_TRACE_RET_OBJ(BDMF_ERR_STATE, mo, "Channel is not set\n");

    /* For WRR scheduler all subsidiary schedulers must have af rate == 0
     * For SP scheduler all subsidiary schedulers must have be rate == 0
     */
    for (ns = 0; ns < RDPA_TM_MAX_SCHED_ELEMENTS && !rc; ns++)
    {
        if (tm->sub_tms[ns])
        {
            tm_drv_priv_t *stm = (tm_drv_priv_t *)bdmf_obj_data(tm->sub_tms[ns]);
            if (egress_tm_is_wrr_elem(tm, ns) &&
                !(stm->weight >= RDPA_MIN_WEIGHT && stm->weight <= RDPA_MAX_WEIGHT))
            {
                BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "WRR weight is not set for %s\n",
                    tm->sub_tms[ns]->name);
            }
            if (stm->level == rdpa_tm_level_egress_tm)
                rc = egress_tm_validate_hierarchy(tm->sub_tms[ns]);
        }
    }

    return rc;
}

/* Release RDD resources - on specific channel */
static void egress_tm_rdd_resources_free_on_channel(struct bdmf_object *mo, tm_channel_t *channel)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    tm_qtm_ctl_t *qtm_ctl = egress_tm_qtm_ctl_get(mo, channel);
    int i;
    int ns;

    /* Release scheduler/rate controller if any */
    if (qtm_ctl)
    {
        for (i = 0; i < RDPA_MAX_EGRESS_QUEUES; i++)
            egress_tm_delete_single_queue(mo, qtm_ctl, channel, tm, i);

        rdpa_rdd_qtm_ctl_destroy(mo, qtm_ctl);
        egress_tm_qtm_ctl_ctx_free(qtm_ctl);
    }

    /* Release resources on subsidiary schedulers */
    for (ns = 0; ns < RDPA_TM_MAX_SCHED_ELEMENTS; ns++)
    {
        if (tm->sub_tms[ns])
            egress_tm_rdd_resources_free_on_channel(tm->sub_tms[ns], channel);
    }

    if (mo == egress_tm_get_top_object(mo))
    {
        rdpa_rdd_top_sched_destroy(mo, channel);
        channel->res_allocated = 0;
    }
}

void egress_tm_delete_single_queue(struct bdmf_object *mo, tm_qtm_ctl_t *qtm_ctl, tm_channel_t *channel, tm_drv_priv_t *tm, int i)
{
    pd_offload_ddr_queue_t *ddr_cfg;
    tm_queue_hash_entry_t *qentry;
    rdpa_stat_t queue_occupancy;
    rdpa_tm_queue_index_t q_index;

    if (qtm_ctl->counter_id[i] != INVALID_COUNTER_ID)
    {
        queue_counters[channel->dir][--queue_counters_idx[channel->dir]] = qtm_ctl->counter_id[i];
        qtm_ctl->counter_id[i] = INVALID_COUNTER_ID;
    }

    if (qtm_ctl->queue_configured[i])
    {
        qentry = &qtm_ctl->hash_entry[i];
        /* Destroy US/DS priority queue */
        if (qentry != NULL)
        {
            BDMF_TRACE_DBG_OBJ(mo, "delete queue =%d\n", qentry->rdp_queue_index);
            if ((channel->dir == rdpa_dir_us) && rdpa_is_epon_or_xepon_mode())
            {
                q_index.channel = qtm_ctl->hash_entry[i].dir_channel;
                q_index.queue_id = qtm_ctl->hash_entry[i].queue_id;

                egress_tm_queue_occupancy_read_ex(mo, NULL, (bdmf_index)&q_index, &queue_occupancy, 0);
                if (queue_occupancy.packets != 0)
                {
                    BDMF_TRACE_ERR("queue occupancy is not 0, channel %d, queue_id %d queue_occupancy packets %d\n", qtm_ctl->hash_entry[i].dir_channel,
                        qtm_ctl->hash_entry[i].queue_id, queue_occupancy.packets);
                }
            }

            ddr_cfg = (channel->dir == rdpa_dir_us) ? &qtm_ctl->wan_tx_ddr_queue[i] : &tm->ddr_queue[i];
            rdpa_rdd_tx_queue_destroy(mo, qentry, ddr_cfg, i);
            qtm_ctl->queue_configured[i] = 0;
            egress_tm_hash_delete(mo, qtm_ctl->hash_entry[i].dir_channel, qtm_ctl->hash_entry[i].queue_id);
        }
        else
        {
            BDMF_TRACE_ERR("Didn't found Hash entry, channel %d, queue_id %d\n", qtm_ctl->hash_entry[i].dir_channel,
                qtm_ctl->hash_entry[i].queue_id);
        }
    }
}

/* set queue attribute to all queues under channel */
bdmf_error_t _rdpa_egress_tm_set_channel_attr(bdmf_object_handle tm_obj, channel_attr *attr, int channel_id)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(tm_obj);
    tm_channel_t *channel = egress_tm_channel_get(tm->dir, channel_id);
    tm_qtm_ctl_t *qtm_ctl = egress_tm_qtm_ctl_get(tm_obj, channel);
    int i, err = 0;

    if (channel != NULL)
        channel->attr = *attr;

    for (i = 0; i < tm->num_channels; i++)
    {
        if (tm->sub_tms[i])
        {
            err = _rdpa_egress_tm_set_channel_attr(tm->sub_tms[i], attr, tm->channels[i]);
            if (err)
                BDMF_TRACE_RET_OBJ(err, tm_obj, "Failed to set queue attributes for channel %d", channel_id);
        }
    }

    if (qtm_ctl)
    {
        for (i = 0; i < tm->num_queues; i++)
        {
            if (qtm_ctl->queue_configured[i])
            {
                err = rdpa_rdd_tx_queue_channel_attr_update(attr, qtm_ctl->hash_entry[i].rdp_queue_index);
                if (err)
                    BDMF_TRACE_RET_OBJ(err, tm_obj, "Failed to set queue attributes for channel %d", channel_id);
            }
        }
    }

    return 0;
}

/* Release RDD resources - on all channels */
static void egress_tm_rdd_resources_free(struct bdmf_object *mo)
{
    struct bdmf_object *top = egress_tm_get_top_object(mo);
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    tm_channel_t *channels = (tm->dir == rdpa_dir_ds) ? ds_channels : us_channels;
    int num_channels = (tm->dir == rdpa_dir_ds) ? RDPA_MAX_DS_CHANNELS : RDPA_MAX_US_CHANNELS;
    int i;

    /* Go over all channels and release resources for all channels bound to the top */
    for (i = 0; i < num_channels; i++)
    {
        tm_channel_t *ch = &channels[i];
        if (ch->egress_tm == top)
            egress_tm_rdd_resources_free_on_channel(mo, ch);
    }
}


/* Allocate RDD resources, except for queues - on specific channel */
static int egress_tm_rdd_resources_alloc_on_channel(struct bdmf_object *mo, tm_channel_t *channel, int sub_index)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    struct bdmf_object *top = egress_tm_get_top_object(mo);
    rdd_sched_cfg_t sched_cfg = {
        .level = tm->level,
        .mode = tm->mode,
        .rl_cfg = tm->rl_cfg,
        .rl_rate_mode = tm->rl_rate_mode,
        .weight = tm->weight,
        .num_queues = tm->num_queues,
        .num_sp_elements = tm->num_sp_elements
    };
    int rc = 0;
    int ns;

    /* Allocate channel-level resource */
    if (!channel->res_allocated && mo == top)
    {
        rc = rdpa_rdd_top_sched_create(mo, channel, &sched_cfg);
        if (rc)
            return rc;
        channel->rl_rate_mode = sched_cfg.rl_rate_mode;
    }

    /* Allocate rate controller if necessary */
    if ((tm->level == rdpa_tm_level_queue) && !egress_tm_qtm_ctl_get(mo, channel))
    {
        tm_qtm_ctl_t *qtm_ctl = egress_tm_qtm_ctl_ctx_alloc(mo, channel, sub_index);
        int rc;

        if (!qtm_ctl)
            return BDMF_ERR_NOMEM;

        /* Do initial (dummy) configuration */
        rc = rdpa_rdd_qtm_ctl_create(mo, qtm_ctl, &sched_cfg);
        if (rc)
            return rc;
        qtm_ctl->res_allocated = 1;
    }

    /* Allocate resources on subsidiary schedulers */
    for (ns = 0; ns < RDPA_TM_MAX_SCHED_ELEMENTS && !rc; ns++)
    {
        if (tm->sub_tms[ns])
            rc = egress_tm_rdd_resources_alloc_on_channel(tm->sub_tms[ns], channel, ns);
    }

    if (mo == top && !rc)
        channel->res_allocated = 1;

    return rc;
}


/* Allocate RDD resources, except for queues - for all channels */
static int egress_tm_rdd_resources_alloc(struct bdmf_object *mo, bdmf_index index)
{
    struct bdmf_object *top = egress_tm_get_top_object(mo);
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(top);
    tm_channel_t *channels = (tm->dir == rdpa_dir_ds) ? ds_channels : us_channels;
    int num_channels = (tm->dir == rdpa_dir_ds) ? RDPA_MAX_DS_CHANNELS : RDPA_MAX_US_CHANNELS;
    int i;
    int rc = 0;

    /* Go over all channels and allocate resources for all channels bound to the top */
    for (i = 0; i < num_channels && !rc; i++)
    {
        tm_channel_t *ch = &channels[i];
        if (ch->egress_tm == top)
            rc = egress_tm_rdd_resources_alloc_on_channel(mo, ch, index);
    }

    if (rc)
    {
        /* Roll-back if failure */
        for (--i; i >= 0; i--)
        {
            tm_channel_t *ch = &channels[i];
            if (ch->egress_tm == top)
                egress_tm_rdd_resources_free_on_channel(mo, ch);
        }
    }

    return rc;
}

/*
 * queue hash management
 */

/* Delete hash entry. Returns the entry that has just been "deleted"
 * Note that the entry was deleted from the hash, but information contained in the entry
 * is still valid;
 */
static struct tm_queue_hash_entry *egress_tm_hash_delete(bdmf_object_handle mo, uint32_t dir_channel, uint32_t queue_id)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    int index = egress_tm_hash_func(dir_channel, queue_id);
    struct tm_queue_hash_entry *entry;
    unsigned long flags;

    BDMF_TRACE_DBG_OBJ(mo, "Attempt to remove entry (%u, %u)\n", dir_channel, queue_id);
    bdmf_fastlock_lock_irq(&tm_hash_lock_irq, flags);
    SLIST_FOREACH(entry, &tm_queue_hash[index], list)
    {
        if (entry->dir_channel == dir_channel && entry->queue_id == queue_id)
        {
            SLIST_REMOVE(&tm_queue_hash[index], entry, tm_queue_hash_entry, list);
            --tm->queue_configured[entry->queue_index];
            if (likely(is_queue_id_shortcut_valid(queue_id)))
            {
                if (egress_tm_get_queue_dir(dir_channel) == rdpa_dir_us)
                    us_hash_lookup[egress_tm_get_queue_channel(dir_channel)][queue_id] = NULL;
                else
                    ds_hash_lookup[egress_tm_get_queue_channel(dir_channel)][queue_id] = NULL;
            }
            bdmf_fastlock_unlock_irq(&tm_hash_lock_irq, flags);
            BDMF_TRACE_DBG_OBJ(mo, "removed entry q:%u dir_c:0x%x\n", (int)queue_id, (unsigned)dir_channel);
            return entry;
        }
    }

    /* Hmm. We shouldn't be here. */
    bdmf_fastlock_unlock_irq(&tm_hash_lock_irq, flags);
    BDMF_TRACE_ERR("Attempt to remove entry (%u, %u) which isn't in the hash\n", dir_channel, queue_id);

    return NULL;
}

/* Insert hash entry. Can fail if entry with given key already exists.
 * Returns NULL in case of success, hash entry having the same key in case of
 * in case of illigal queue_id will return entry - to indicate error case.
 * failure.
 */
static tm_queue_hash_entry_t *egress_tm_hash_insert(bdmf_object_handle mo,
    uint32_t dir_channel, uint32_t queue_id, tm_queue_hash_entry_t *entry)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    int index;
    tm_queue_hash_entry_t *old_entry;
    unsigned long flags;

    index = egress_tm_hash_func(dir_channel, queue_id);
    old_entry = egress_tm_hash_get(dir_channel, queue_id);
    BDMF_TRACE_DBG_OBJ(mo, "Attempt to insert entry (%u, %u)\n", dir_channel, queue_id);
    if (old_entry)
        return old_entry;
    entry->dir_channel = dir_channel;
    entry->queue_id = queue_id;

    bdmf_fastlock_lock_irq(&tm_hash_lock_irq, flags);
    SLIST_INSERT_HEAD(&tm_queue_hash[index], entry, list);
    ++tm->queue_configured[entry->queue_index];
    if (likely(is_queue_id_shortcut_valid(queue_id)))
    {
        if (egress_tm_get_queue_dir(dir_channel) == rdpa_dir_us)
            us_hash_lookup[egress_tm_get_queue_channel(dir_channel)][queue_id] = entry;
        else
            ds_hash_lookup[egress_tm_get_queue_channel(dir_channel)][queue_id] = entry;
    }
    bdmf_fastlock_unlock_irq(&tm_hash_lock_irq, flags);

    BDMF_TRACE_DBG_OBJ(mo, "inserted entry q:%u dir_c:0x%x\n", (int)queue_id, (unsigned)dir_channel);
    return NULL;
}

static int egress_tm_queue_verify_parameters(struct bdmf_object *mo,
    tm_channel_t *channel, int index, rdpa_tm_queue_cfg_t *new_queue_cfg)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    tm_qtm_ctl_t *qtm_ctl = egress_tm_qtm_ctl_get(mo, channel);
    const rdpa_system_cfg_t *system_cfg = _rdpa_system_cfg_get();
    bdmf_error_t rc = BDMF_ERR_OK;


    if (qtm_ctl == NULL || tm->level != rdpa_tm_level_queue)
       BDMF_TRACE_RET_OBJ(BDMF_ERR_NOT_SUPPORTED, mo, "qtm_ctl %p, tm->level %d\n", qtm_ctl, tm->level);


    /* Validate (W)RED thresholds if drop alg is (W)RED */
    if (new_queue_cfg->drop_alg == rdpa_tm_drop_alg_red || new_queue_cfg->drop_alg == rdpa_tm_drop_alg_wred)
    {
       if ((system_cfg->options & (1 << rdpa_reserved_option_0)) == 0)
       {
           if (new_queue_cfg->high_class.max_threshold > RDPA_ETH_TX_PRIORITY_QUEUE_THRESHOLD ||
               new_queue_cfg->high_class.max_threshold > new_queue_cfg->drop_threshold)
           {
               BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "Wrong drop threshold hi %u, size%u\n",
                   new_queue_cfg->high_class.max_threshold, new_queue_cfg->drop_threshold);
           }
       }
       else if ((system_cfg->options & (1 << rdpa_reserved_option_0)))
       {
           if (new_queue_cfg->high_class.max_threshold > RDPA_ETH_TX_PRIORITY_QUEUE_THRESHOLD ||
               new_queue_cfg->high_class.max_threshold < new_queue_cfg->low_class.max_threshold ||
               new_queue_cfg->drop_threshold < new_queue_cfg->high_class.max_threshold)
           {
               BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "Wrong thresholds hi%u, lo%u, size%u\n",
                   new_queue_cfg->high_class.max_threshold, new_queue_cfg->low_class.max_threshold,
                   new_queue_cfg->drop_threshold);
           }
       }
       if (new_queue_cfg->high_class.max_threshold < new_queue_cfg->high_class.min_threshold ||
           new_queue_cfg->low_class.max_threshold < new_queue_cfg->low_class.min_threshold)
       {
           BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "Wrong thresholds hi_max%u, hi_min%u, lo_max%u, lo_min%u, size%u\n",
               new_queue_cfg->high_class.max_threshold, new_queue_cfg->high_class.min_threshold,
               new_queue_cfg->low_class.max_threshold, new_queue_cfg->low_class.min_threshold,
               new_queue_cfg->drop_threshold);
       }
    }

    return rc;
}

#ifdef XRDP
static int egress_tm_queue_verify_resources(struct bdmf_object *mo, tm_channel_t *channel,
                                            rdpa_tm_queue_cfg_t *new_queue_cfg, bdmf_boolean queue_was_configured)
{
    bdmf_index queue_index, profile_index;
    tm_qtm_ctl_t *qtm_ctl = egress_tm_qtm_ctl_get(mo, channel);

    /* Check for available queue if it's a new queue */
    if (!queue_was_configured)
    {
        queue_index = _rdpa_rdd_get_queue_idx(channel, qtm_ctl);
        if (queue_index == BDMF_INDEX_UNASSIGNED)
        {
            BDMF_TRACE_RET_OBJ(BDMF_ERR_NORES, mo, "No available qm queue found\n");
        }
    }

    /* Check for available queue profile -
     * return error when there is no existed profile with the queue_cfg and there is no available new profile */
    profile_index = rdpa_get_active_profile_index(new_queue_cfg);
    if (profile_index == BDMF_INDEX_UNASSIGNED)
    {
        profile_index = rdpa_get_available_profile_index(new_queue_cfg);
        if (profile_index == BDMF_INDEX_UNASSIGNED)
            BDMF_TRACE_RET_OBJ(BDMF_ERR_NORES, mo, "No available profile found for the new queue\n");
    }

    return BDMF_ERR_OK;
}
#endif

static int egress_tm_get_hash_dir_channel(struct bdmf_object *mo, tm_channel_t *channel)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    uint32_t dir_channel = egress_tm_dir_channel(tm->dir, channel->channel_id);
    struct bdmf_object *root = egress_tm_get_root_object(mo);
    tm_drv_priv_t *root_tm = (tm_drv_priv_t *)bdmf_obj_data(root);
    /* Special handling for channel-group egress_tm (LLID).
    * Queues are stored in hash with group_id instead of channel_id
    */
    if (egress_tm_is_group_owner(root_tm))
        dir_channel = egress_tm_dir_channel(tm->dir, root_tm->channel_group);
    return dir_channel;
}

static int egress_tm_add_to_hash(struct bdmf_object *mo,
    tm_channel_t *channel, int index, rdpa_tm_queue_cfg_t *new_queue_cfg)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    uint32_t dir_channel = egress_tm_get_hash_dir_channel(mo, channel);
    tm_qtm_ctl_t *qtm_ctl = egress_tm_qtm_ctl_get(mo, channel);
    bdmf_error_t rc = BDMF_ERR_OK;
    tm_queue_hash_entry_t *old_entry = NULL;

    old_entry = egress_tm_hash_insert(mo, dir_channel, new_queue_cfg->queue_id, &qtm_ctl->hash_entry[index]);
    /* Error handling */
    if (old_entry)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_ALREADY, mo, "queue_id %u is already in use\n", new_queue_cfg->queue_id);

    tm->queue_id_assigned[index] = 1;
    return rc;
}


static int egress_tm_queue_disable_on_channel(struct bdmf_object *mo,
    tm_channel_t *channel, int index, rdpa_tm_queue_cfg_t *new_queue_cfg)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    tm_qtm_ctl_t *qtm_ctl = egress_tm_qtm_ctl_get(mo, channel);
    rdd_queue_cfg_t queue_cfg = {};
    pd_offload_ddr_queue_t *ddr_cfg = (tm->dir == rdpa_dir_us) ? &qtm_ctl->wan_tx_ddr_queue[index] : &tm->ddr_queue[index];
    tm_queue_hash_entry_t *qentry;
    bdmf_error_t rc = BDMF_ERR_OK;
#ifdef XRDP
    qm_q_context q_context;
#endif
    /* Get hash entry */
    qentry = &qtm_ctl->hash_entry[index];

    /* Disable US/DS priority queue */
    /* At this point new queue is in the hash. Update RDD */
    queue_cfg.packet_threshold = new_queue_cfg->drop_threshold;
    queue_cfg.counter_id = new_queue_cfg->queue_id;
    queue_cfg.profile = RDD_QUEUE_PROFILE_DISABLED;
    queue_cfg.mbr_profile = RDD_QUEUE_PROFILE_DISABLED;
    queue_cfg.rl_cfg = new_queue_cfg->rl_cfg;
    queue_cfg.weight = new_queue_cfg->weight;
    tm->queue_id_assigned[index] = (new_queue_cfg->queue_id != BDMF_INDEX_UNASSIGNED);
#ifdef XRDP
    ag_drv_qm_q_context_get(qentry->rdp_queue_index, &q_context);
    if (q_context.wred_profile != QM_WRED_PROFILE_DROP_ALL)
    	rdpa_rdd_tm_queue_profile_free(mo, tm->dir, &tm->queue_cfg[index]);
    q_context.wred_profile = QM_WRED_PROFILE_DROP_ALL;
    ag_drv_qm_q_context_set(qentry->rdp_queue_index, &q_context);
#else
    rdpa_rdd_tm_queue_profile_free(mo, tm->dir, &tm->queue_cfg[index]);
#endif

    BDMF_TRACE_DBG_OBJ(mo, "disable queue %d on channel %d\n", index, channel->channel_id);
    rc = rdpa_rdd_tx_queue_modify(mo, qentry, &queue_cfg, ddr_cfg, 0);
    return rc;
}

static int egress_tm_set_profile_and_cfg_q(struct bdmf_object *mo,
    tm_channel_t *channel, int index, rdpa_tm_queue_cfg_t *new_queue_cfg, bdmf_boolean enable)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    tm_qtm_ctl_t *qtm_ctl = egress_tm_qtm_ctl_get(mo, channel);
    uint32_t drop_threshold;
    rdd_queue_cfg_t queue_cfg = {};
    pd_offload_ddr_queue_t *ddr_cfg = (tm->dir == rdpa_dir_us) ? &qtm_ctl->wan_tx_ddr_queue[index] : &tm->ddr_queue[index];
    tm_queue_hash_entry_t *qentry = NULL;
    bdmf_error_t rc = BDMF_ERR_OK;
    rdd_queue_profile_id_t rdd_queue_profile = RDD_QUEUE_PROFILE_DISABLED;
    uint32_t rdd_mbr_queue_profile = 0; /* MBR profile 0 is disable (minimum buffer reservation) */
#ifdef XRDP

    qm_q_context q_context;
    bdmf_boolean delete_profile;

    if (qtm_ctl->queue_configured[index])
    {
    	qentry = &qtm_ctl->hash_entry[index];
    	if (qentry)
    		ag_drv_qm_q_context_get(qentry->rdp_queue_index, &q_context);

        delete_profile = (qtm_ctl->queue_configured[index]) && (qentry) && (q_context.wred_profile != QM_WRED_PROFILE_DROP_ALL);
    }
    else
        delete_profile = 0;


    rc = rdpa_rdd_tm_queue_profile_cfg(mo, tm->dir,
        &tm->queue_cfg[index], new_queue_cfg, delete_profile , &rdd_queue_profile);
    if (rc)
        return rc;

    if (qentry)
    {
    	q_context.wred_profile = rdd_queue_profile;
        ag_drv_qm_q_context_set(qentry->rdp_queue_index, &q_context);
    }
#else
rc = rdpa_rdd_tm_queue_profile_cfg(mo, tm->dir,
        &tm->queue_cfg[index], new_queue_cfg,
        (qtm_ctl->queue_configured[index] == 0), &rdd_queue_profile);
    if (rc)
        return rc;
#endif
    

    if ((new_queue_cfg->drop_alg == rdpa_tm_drop_alg_wred) || (new_queue_cfg->drop_alg == rdpa_tm_drop_alg_red))
#ifdef XRDP
        drop_threshold = new_queue_cfg->high_class.max_threshold;
#else    /* Optimization for RDP only */
        drop_threshold = new_queue_cfg->low_class.min_threshold;
#endif
    else if (new_queue_cfg->drop_alg == rdpa_tm_drop_alg_reserved)
        drop_threshold = new_queue_cfg->high_class.min_threshold;
    else
        drop_threshold = new_queue_cfg->drop_threshold;

    /* Indicates to not change the current mbr profile value in the qm queue context */
    rdd_mbr_queue_profile = RDD_QUEUE_PROFILE_DISABLED;

    /* If MBR configuration was changed (some configuration is not zero (minimum buffer reservation)) */

    if (new_queue_cfg->reserved_packet_buffers)
    {
    	if (qtm_ctl->queue_configured[index])
    	{
    		if (new_queue_cfg->reserved_packet_buffers != tm->queue_cfg[index].reserved_packet_buffers)
    		{
    			BDMF_TRACE_DBG_OBJ(mo, "Can't configure minimum buffer reservation after queue is configured("
    					"queue=%d,new_reserved_packet_buffers=%d exist_reserved_packet_buffers=%d\n",
                        index, new_queue_cfg->reserved_packet_buffers, tm->queue_cfg[index].reserved_packet_buffers);
    			new_queue_cfg->reserved_packet_buffers = tm->queue_cfg[index].reserved_packet_buffers;
    		}
    	}
    	else
    	{
    		rc = rdpa_rdd_tm_queue_mbr_profile_cfg(mo, tm->dir, &tm->queue_cfg[index], new_queue_cfg,
    				(qtm_ctl->queue_configured[index] == 0), &rdd_mbr_queue_profile);
    	}
    }
    else
    {
        if (qtm_ctl->queue_configured[index])
        {
            if (new_queue_cfg->reserved_packet_buffers != tm->queue_cfg[index].reserved_packet_buffers)
            {
                BDMF_TRACE_DBG_OBJ(mo, "Can't unconfigure minimum buffer reservation after queue is configured("
                        "queue=%d,new_reserved_packet_buffers=%d exist_reserved_packet_buffers=%d\n",
                        index, new_queue_cfg->reserved_packet_buffers, tm->queue_cfg[index].reserved_packet_buffers);
                new_queue_cfg->reserved_packet_buffers = tm->queue_cfg[index].reserved_packet_buffers;
            }
            }
            else
            {
                rdd_mbr_queue_profile = RDD_QUEUE_PROFILE_0;
            }
    }

    qentry = &qtm_ctl->hash_entry[index];
    /* At this point new queue is in the hash. Update RDD */
    queue_cfg.packet_threshold = drop_threshold;
    queue_cfg.counter_id = qtm_ctl->counter_id[index];
    queue_cfg.profile = rdd_queue_profile;
    queue_cfg.mbr_profile = rdd_mbr_queue_profile;
    queue_cfg.rl_cfg = new_queue_cfg->rl_cfg;
    queue_cfg.weight = new_queue_cfg->weight;

#ifdef XRDP
    if ((channel->dir == rdpa_dir_us) && (channel->owner->drv == rdpa_tcont_drv()))
    {
        if (rdpa_tcont_is_mgmt(channel->owner))
            queue_cfg.exclusive = 1;
    }
#endif

#if defined(CONFIG_BCM_PON) || defined(CONFIG_BCM963158)
    if ((tm->dir == rdpa_dir_us) && rdpa_is_epon_or_xepon_mode() && (new_queue_cfg->queue_id == RDPA_EPON_CONTROL_QUEUE_ID))
        queue_cfg.exclusive = 1;
#endif
    if (!qtm_ctl->queue_configured[index])
    {
        rc = rdpa_rdd_tx_queue_create(mo, tm->wan_type, qentry, &queue_cfg, ddr_cfg, enable);
        if (rc)
        {
            BDMF_TRACE_RET_OBJ(rc, mo, "Failed to configure the queue %d, channel %d, err: %d/n",
                index, channel->channel_id, rc);
        }
        qtm_ctl->queue_configured[index] = 1;
    }
    else /* Queue threshold has been changed */
        rc = rdpa_rdd_tx_queue_modify(mo, qentry, &queue_cfg, ddr_cfg, enable);
    return rc;
}

/* Configure / remove egress queue.
 * It includes adding/removing queue from hash and updating RDD
 */
int egress_tm_queue_cfg_on_channel(struct bdmf_object *mo,
    tm_channel_t *channel, int index, rdpa_tm_queue_cfg_t *new_queue_cfg, bdmf_boolean enable)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    uint32_t dir_channel = egress_tm_get_hash_dir_channel(mo, channel);
    tm_qtm_ctl_t *qtm_ctl = egress_tm_qtm_ctl_get(mo, channel);
    bdmf_error_t rc = BDMF_ERR_OK;
    int queue_enable = enable && new_queue_cfg->drop_threshold;
    bdmf_boolean queue_was_configured = qtm_ctl->queue_configured[index];

    BDMF_TRACE_DBG_OBJ(mo, "cfg_on_channel (%d,%d)\n", dir_channel, index);

    rc = egress_tm_queue_verify_parameters(mo, channel, index, new_queue_cfg);
#ifdef XRDP
    if (!queue_was_configured)
        rc = rc ? rc : egress_tm_queue_verify_resources(mo, channel, new_queue_cfg, queue_was_configured);
#endif
    if (rc)
        return rc;

    if (!qtm_ctl->queue_configured[index])
    {
        rc = egress_tm_add_to_hash(mo, channel, index, new_queue_cfg);
        if (rc)
            return rc;
    }

    if (enable)
    {
        rc = egress_tm_allocate_counter(mo, qtm_ctl, new_queue_cfg, index);
        if (rc)
            return rc;
    }

    /* in first time and not enabled, create and then disable */
    if (!qtm_ctl->queue_configured[index] && !queue_enable)
        rc = egress_tm_set_profile_and_cfg_q(mo, channel, index, new_queue_cfg, enable);

    if (rc)
        goto exit;

    /* Remove old configuration if queue becomes disabled */
    if (!queue_enable)
    {
        rdpa_tm_queue_cfg_t disable_cfg = {};
        rc = egress_tm_queue_disable_on_channel(mo, channel, index, &disable_cfg);
    }
    else
    {
        rc = egress_tm_set_profile_and_cfg_q(mo, channel, index, new_queue_cfg, enable);
    }

exit:
    if (rc && rc != BDMF_ERR_TOO_MANY) /* BDMF_ERR_TOO_MANY - queue modification failure case */
    {
        /* Remove queue from hash and return error. If we are here - it is an internal error.
         * At this point it is difficult to roll-back. It is safer just leave the queue un-configured
         */
        BDMF_TRACE_ERR("egress_tm configuration failed, doing roll back\n");
        /* If this is new queue and failed - need to delete queue from hash.
         * If queue was already configured but failed in configuration, the old queue should not be deleted */
        if (!queue_was_configured)
        {
            qtm_ctl->queue_configured[index] = 1;
            egress_tm_delete_single_queue(mo, qtm_ctl, channel, tm, index);
        }

        rdpa_rdd_tm_queue_profile_free(mo, tm->dir, new_queue_cfg);

        BDMF_TRACE_RET_OBJ(rc, mo,
            "Failed to configure priority queue %d, channel %d, rate "
            "controller %d\n", index, channel->channel_id, qtm_ctl->rc_id);
    }

    if (!rc)
        memcpy(&qtm_ctl->queue_cfg[index], new_queue_cfg, sizeof(rdpa_tm_queue_cfg_t));
    else
        BDMF_TRACE_ERR("FAILED to change queue %d modification\n", index);

    return rc;
}



/* Configure / remove egress queue - for all channels
 * It includes adding/removing queue from hash and updating RDD
 */
static int egress_tm_queue_cfg(struct bdmf_object *mo, int index, rdpa_tm_queue_cfg_t *new_queue_cfg, bdmf_boolean enable)
{
    struct bdmf_object *top = egress_tm_get_top_object(mo);
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(top);
    tm_channel_t *channels = (tm->dir == rdpa_dir_ds) ? ds_channels : us_channels;
    int num_channels = (tm->dir == rdpa_dir_ds) ? RDPA_MAX_DS_CHANNELS : RDPA_MAX_US_CHANNELS;
    int i;
    int rc = 0;

    /* Go over all channels and configure queue for all channels bound to the top */
    for (i = 0; i < num_channels && !rc; i++)
    {
        tm_channel_t *ch = &channels[i];
        if (ch->egress_tm == top)
            rc = egress_tm_queue_cfg_on_channel(mo, ch, index, new_queue_cfg, enable);
    }

    return rc;
}


/* Configure overall rate limiter */
static int egress_tm_orl_config(struct bdmf_object *mo, rdpa_tm_rl_cfg_t *rl_cfg, bdmf_boolean enable)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    rl_rate_size_t rate = (enable && rl_cfg) ? rl_cfg->af_rate : 0;
    int rc = 0;

    if (tm->dir == rdpa_dir_us)
    {
#ifdef EPON
        int rdd_rc;
        if (rdpa_is_epon_or_xepon_mode() || rdpa_is_epon_ae_mode())
        {
            if (orl_tm_linked_to_llid)
            {
                if (NULL != global_shaper_cb)
                {
                    rdd_rc = global_shaper_cb((uint32_t)rate);
                    BDMF_TRACE_RET_OBJ(rdd_rc ? BDMF_ERR_INTERNAL : 0, mo,
                        "OntDirGlobalShaperSet({rate=%d}) -> %d\n", (int)rate, rdd_rc);
                }
            }
            else
            {
                tm->rl_cfg.af_rate = rate;
            }
        }
        else
#endif
        rc = rdpa_rdd_orl_rate_cfg(mo, rate / 8);
    }

    return rc;
}

/* Configure rate controller - on a single channel */
static int egress_tm_rl_config_on_channel(struct bdmf_object *mo, tm_channel_t *channel, rdpa_tm_rl_cfg_t *rl_cfg,
    int weight, bdmf_boolean enable)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    tm_qtm_ctl_t *qtm_ctl = egress_tm_qtm_ctl_get(mo, channel);
    rdpa_tm_rl_cfg_t tmp_rl_cfg = {};
    bdmf_error_t rc = BDMF_ERR_OK;
    int is_wrr;

    if (!qtm_ctl)
    {
        /* If egress_tm is group-level, it doesn't support rate limiting */
        if (tm->level == rdpa_tm_level_egress_tm)
        {
#ifndef XRDP
            /* The scheduler rate limiter is programmed on the egress_tm
             * object for service queues. */
            if (egress_tm_is_service_q(tm))
                return egress_tm_service_queue_rl_config(mo, rl_cfg, enable);
#endif
            BDMF_TRACE_RET_OBJ(BDMF_ERR_NOT_SUPPORTED, mo, "Group-level rate limiting is not supported\n");
        }
        return BDMF_ERR_NODEV; /* no valid egress tm*/
    }
    is_wrr = tm->upper_level_tm &&
        egress_tm_is_wrr_elem((tm_drv_priv_t *)bdmf_obj_data(tm->upper_level_tm), qtm_ctl->sched_index_in_upper);

    if (!is_wrr)
        weight = 0;

    if (enable)
    {
        tmp_rl_cfg = *rl_cfg;
        if (!tmp_rl_cfg.af_rate)
        {
            tmp_rl_cfg.af_rate = RDD_RATE_UNLIMITED;
            tmp_rl_cfg.burst_size = RDD_RATE_UNLIMITED / BITS_IN_BYTE / RDD_RATE_QUANTUM;
        }
#if defined(BCM_DSL_RDP)
        else
            tmp_rl_cfg.burst_size = tmp_rl_cfg.burst_size / RDD_RATE_QUANTUM;
#endif
    }

    rc = rdpa_rdd_qtm_ctl_modify(mo, qtm_ctl, &tmp_rl_cfg, weight, -1);
    if (rc)
        BDMF_TRACE_RET_OBJ(rc, mo, "rdd RL configuration failed. Set trace_level=debug for details\n");

    return rc;
}

/* Configure rate controller - for all channels */
static int egress_tm_rl_config(struct bdmf_object *mo, rdpa_tm_rl_cfg_t *rl_cfg, int weight, bdmf_boolean enable)
{
    struct bdmf_object *top = egress_tm_get_top_object(mo);
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    tm_channel_t *channels = (tm->dir == rdpa_dir_ds) ? ds_channels : us_channels;
    int num_channels = (tm->dir == rdpa_dir_ds) ? RDPA_MAX_DS_CHANNELS : RDPA_MAX_US_CHANNELS;
    int i, rc = 0;
#ifdef EPON
    int is_llid_tm = 0;
#endif

    if (tm->overall_rl)
        return egress_tm_orl_config(mo, rl_cfg, enable);

#ifdef EPON
    if ((tm->dir == rdpa_dir_us) && rdpa_is_epon_or_xepon_mode())
    {
        for (i = 0;; i++)
        {
            bdmf_object_handle   llid_obj = NULL;
            bdmf_object_handle   tm_obj = NULL;
            bdmf_number tmp_id;

            rc = rdpa_llid_get(i, &llid_obj);
            if (rc)
                break;  /* no more LLID object */

            rc = rdpa_llid_egress_tm_get(llid_obj, &tm_obj);
            if (rc)
                continue;

            if (!tm_obj)
                continue;

            rc = rdpa_egress_tm_index_get(tm_obj, &tmp_id);
            if (rc)
                continue;

            if (tmp_id == tm->index)
            {
                is_llid_tm = 1;
                break;
            }
        }

        if (is_llid_tm)
        { /* we set epon mac for LLID obj */
            if (NULL != epon_link_shaper_cb)
            {
                rc = epon_link_shaper_cb(i, rl_cfg->af_rate, rl_cfg->burst_size);
                BDMF_TRACE_RET_OBJ(rc ? BDMF_ERR_INTERNAL : 0, mo,
                    "OntDirLinkShaperSet(%d {rate=%d, burst=%d}) -> %d\n",
                    i, (int)rl_cfg->af_rate, (int)rl_cfg->burst_size, rc);
            }
            return rc;
        }

        rc = 0;
    }
#endif

    /* Go over all channels and configure queue for all channels bound to the top */
    for (i = 0; i < num_channels && !rc; i++)
    {
        tm_channel_t *ch = &channels[i];
        if (ch->egress_tm == top)
            rc = egress_tm_rl_config_on_channel(mo, ch, rl_cfg, weight, enable);
    }

    return rc;
}

/* Configure number of SP elements  - on a single channel */
static int egress_tm_num_sp_elements_config_on_channel(struct bdmf_object *mo, tm_channel_t *channel, rdpa_tm_num_sp_elem num_sp_elements)
{
    tm_qtm_ctl_t *qtm_ctl = egress_tm_qtm_ctl_get(mo, channel);
    bdmf_error_t rc = BDMF_ERR_OK;

    if (qtm_ctl)
    {
        rc = rdpa_rdd_qtm_ctl_modify(mo, qtm_ctl, NULL, -1, num_sp_elements);
        if (rc)
            BDMF_TRACE_RET_OBJ(rc, mo, "rdd num_sp_elements configuration failed. Set trace_level=debug for details\n");
    }

    return rc;
}

/* Configure rate controller - for all channels */
static int egress_tm_num_sp_elements_config(struct bdmf_object *mo, rdpa_tm_num_sp_elem num_sp_elements)
{
    struct bdmf_object *top = egress_tm_get_top_object(mo);
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    tm_channel_t *channels = (tm->dir == rdpa_dir_ds) ? ds_channels : us_channels;
    int num_channels = (tm->dir == rdpa_dir_ds) ? RDPA_MAX_DS_CHANNELS : RDPA_MAX_US_CHANNELS;
    int i, rc = 0;

    /* Go over all channels and configure queue for all channels bound to the top */
    for (i = 0; i < num_channels && !rc; i++)
    {
        tm_channel_t *ch = &channels[i];
        if (ch->egress_tm == top)
            rc = egress_tm_num_sp_elements_config_on_channel(mo, ch, num_sp_elements);
    }

    return rc;
}

/* Set-up overall rate limiter - single channel */
static int egress_tm_orl_set_on_channel(struct bdmf_object *mo, tm_channel_t *channel, struct bdmf_object *orl_mo,
    rdpa_tm_orl_prty orl_prty)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);

    if (tm->dir == rdpa_dir_us)
    {
        int rc;
        rc = rdpa_rdd_orl_channel_cfg(mo, channel, orl_mo ? 1 : 0, orl_prty);
        if (rc)
            return rc;
    }

    channel->orl_tm = orl_mo;

    return 0;
}

/* Set-up overall rate limiter */
static int egress_tm_orl_set(struct bdmf_object *mo, rdpa_tm_orl_prty orl_prty)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    tm_channel_t *ch = NULL;
    int rc = 0;

    if (tm->dir == rdpa_dir_us)
    {
        while ((ch = egress_tm_channel_get_next(mo, ch)) && !rc)
        {
            if (!ch->orl_tm)
                continue;
            rc = egress_tm_orl_set_on_channel(mo, ch, ch->orl_tm, orl_prty);
        }
    }
    else
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_NOT_SUPPORTED, mo, "DS overall RL is not supported.\n");
    }

    return rc;
}

/* Check if channel is linked to overall RL object */
static int egress_tm_is_channel_rl(struct bdmf_object *orl_obj, struct bdmf_object *channel)
{
    struct bdmf_link *link = NULL;

    while ((link = bdmf_get_next_us_link(channel, link)))
    {
        if (bdmf_us_link_to_object(link) == orl_obj)
            return 1;
    }
    while ((link = bdmf_get_next_ds_link(channel, link)))
    {
        if (bdmf_ds_link_to_object(link) == orl_obj)
            return 1;
    }
    return 0;
}


/* Enable/disable egress_tm channel */
static int egress_tm_enable_set_on_channel(struct bdmf_object *mo, tm_channel_t *channel, bdmf_boolean enable, bdmf_boolean flush)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    int i = 0;
    int rc = 0;

    BDMF_TRACE_DBG_OBJ(mo, "enable=%d channel=%d prev_enable=%d\n", enable, channel->channel_id, channel->enable);

    /* Avoid double enable/disable on top level */
    if (!tm->upper_level_tm && enable == channel->enable)
        return 0;

    /* Configure rate controller */
    if (tm->level == rdpa_tm_level_queue || tm->overall_rl || egress_tm_is_service_q(tm))
        rc = egress_tm_rl_config_on_channel(mo, channel, &tm->rl_cfg, tm->weight, enable);
    if (rc)
        goto err_rl;

    if (tm->level == rdpa_tm_level_queue)
    {
        /* Configure queues if any */
        for (i = 0; i < tm->num_queues && !rc; i++)
        {
            if (!tm->queue_cfg[i].drop_threshold)
                continue;

            rc = egress_tm_queue_cfg_on_channel(mo, channel, i, &tm->queue_cfg[i], enable);
            if (rc)
                goto err_queues;

            if (!enable && flush)
            {
                tm_qtm_ctl_t *qtm_ctl = egress_tm_qtm_ctl_get(mo, channel);
                tm_queue_hash_entry_t *qentry = &qtm_ctl->hash_entry[i];

                rdpa_rdd_tx_queue_flush(mo, qentry, 1);
            }
        }
    }

    /* Enable/disable subsidiary TMs if any */
    for (i = 0; i < RDPA_TM_MAX_SCHED_ELEMENTS && !rc; i++)
    {
        if (tm->sub_tms[i])
            rc = egress_tm_enable_set_on_channel(tm->sub_tms[i], channel, enable, flush);
        if (rc)
            goto err_subsidiaries;
    }

    if (!tm->upper_level_tm)
        channel->enable = enable;

    return 0;

err_queues:
    /* Failure. Roll-back */
    if (tm->level == rdpa_tm_level_queue)
    {
        for (--i; i >= 0; i--)
            egress_tm_queue_cfg_on_channel(mo, channel, i, &tm->queue_cfg[i] , enable);
    }

    i = RDPA_TM_MAX_SCHED_ELEMENTS;
err_subsidiaries:
    for (--i; i >= 0; i--)
    {
        if (tm->sub_tms[i])
            egress_tm_enable_set_on_channel(tm->sub_tms[i], channel, !enable, 0);
    }

err_rl:
    /* Deconfigure rate controller */
    if (tm->level == rdpa_tm_level_queue || tm->overall_rl)
        egress_tm_rl_config_on_channel(mo, channel, &tm->rl_cfg, tm->weight, !enable);

    return rc;
}


/* Store enable status in all objects in hierarchy */
static void egress_tm_enable_store(tm_drv_priv_t *tm, bdmf_boolean enable)
{
    int i;

    tm->enable_cfg.enable = enable;

    /* Store in subsidiaries */
    for (i = 0; i < RDPA_TM_MAX_SCHED_ELEMENTS; i++)
    {
        if (tm->sub_tms[i])
            egress_tm_enable_store((tm_drv_priv_t *)bdmf_obj_data(tm->sub_tms[i]), enable);
    }
}

/* Enable/disable egress_tm */
static int egress_tm_enable_set(struct bdmf_object *mo, bdmf_boolean enable, bdmf_boolean flush)
{
    struct bdmf_object *top = egress_tm_get_top_object(mo);
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    tm_channel_t *channels = (tm->dir == rdpa_dir_ds) ? ds_channels : us_channels;
    int num_channels = (tm->dir == rdpa_dir_ds) ? RDPA_MAX_DS_CHANNELS : RDPA_MAX_US_CHANNELS;
    int i = 0;
    int rc = 0;
    uint32_t rate_limiter_timer_period;

    rate_limiter_timer_period = (tm->dir == rdpa_dir_ds) ? DS_RATE_LIMITER_TIMER_PERIOD_IN_USEC : US_RATE_LIMITER_TIMER_PERIOD_IN_USEC;
    tm->rl_cfg.burst_size = rdd_rate_to_alloc_unit(((tm->rl_cfg.af_rate + tm->rl_cfg.be_rate) / BITS_IN_BYTE), rate_limiter_timer_period);

    if (tm->overall_rl)
        return egress_tm_orl_config(mo, &tm->rl_cfg, enable);

    /* For channel group (LLID) go over all subgroups (sunsidiaries) */
    for (i = 0; i < tm->num_channels; i++)
    {
       if (tm->sub_tms[i])
           rc = egress_tm_enable_set(tm->sub_tms[i], enable, flush);
       if (rc)
           goto err_ch_group;
    }

    if (!egress_tm_is_ch_group(tm))
    {
        /* Go over all channels and configure queue for all channels bound to the top */
        for (i = 0; i < num_channels; i++)
        {
            tm_channel_t *ch = &channels[i];
            if (ch->egress_tm == top)
                rc = egress_tm_enable_set_on_channel(mo, ch, enable, flush);
            if (rc)
                goto err_ch;
        }
    }

    egress_tm_enable_store(tm, enable);
    return 0;

err_ch:
    /* Failure. Roll-back */
    for (--i; i >= 0; i--)
    {
        tm_channel_t *ch = &channels[i];
        if (ch->egress_tm == top)
            egress_tm_enable_set_on_channel(mo, ch, !enable, 0);
    }

    i = tm->num_channels;
err_ch_group:
    for (--i; i >= 0; i--)
    {
        if (tm->sub_tms[i])
            egress_tm_enable_set(tm->sub_tms[i], !enable, 0);
    }

    return rc;
}


/*
 * egress_tm object management callbacks
 */

/** This optional callback is called called at object init time
 *  before initial attributes are set.
 *  If function returns error code !=0, object creation is aborted
 */
static int egress_tm_pre_init(struct bdmf_object *mo)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    int n;

    tm->this = mo;
    tm->index = BDMF_INDEX_UNASSIGNED;
    tm->level = rdpa_tm_level_egress_tm;
    tm->rl_rate_mode = rdpa_tm_rl_single_rate;
    tm->num_queues = RDPA_DFT_NUM_EGRESS_QUEUES;

    for (n = 0; n < RDPA_MAX_EGRESS_QUEUES; n++)
    {
        tm->queue_cfg[n].queue_id = BDMF_INDEX_UNASSIGNED;
        /* no special drop algorithm */
        tm->queue_cfg[n].drop_alg = rdpa_tm_drop_alg_dt;
    }
    tm->channel_group = BDMF_INDEX_UNASSIGNED;
    for (n = 0; n < RDPA_MAX_WAN_SUBCHANNELS; n++)
        tm->channels[n] = BDMF_INDEX_UNASSIGNED;

    tm->enable_cfg.enable = 1;
    return 0;
}

/** This optional callback is called at object init time
 * after initial attributes are set.
 * Its work is:
 * - make sure that all necessary attributes are set and make sense
 * - allocate dynamic resources if any
 * - assign object name if not done in pre_init
 * - finalise object creation
 * If function returns error code !=0, object creation is aborted
 */
static int egress_tm_post_init(struct bdmf_object *mo)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    struct bdmf_object **tm_objects = (tm->dir == rdpa_dir_ds) ? ds_tm_objects : us_tm_objects;
    int max_scheds = (tm->dir == rdpa_dir_ds) ? RDPA_TM_MAX_DS_SCHED : RDPA_TM_MAX_US_SCHED;
    int rc = 0;

    /* If egress tm index is set - make sure it is unique.
     * Otherwise, assign free
     */
    if (tm->index < 0)
    {
        int i;
        /* Find and assign free index */
        for (i = 0; i < max_scheds; i++)
        {
            if (!tm_objects[i])
            {
                tm->index = i;
                break;
            }
        }
    }
    if ((unsigned)tm->index >= max_scheds)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "Too many egress tms or index %ld is out of range\n", tm->index);

    if (tm->dir == rdpa_dir_ds && num_normal_ds_tm == RDPA_MAX_DS_TM_QUEUE && !egress_tm_is_service_q(tm))
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "Too many DS egress tms\n");

    if (tm->dir == rdpa_dir_us && num_normal_us_tm == RDPA_MAX_US_TM_QUEUE && !egress_tm_is_service_q(tm))
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "Too many US egress tms\n");

    /* set object name */
    snprintf(mo->name, sizeof(mo->name), "egress_tm/dir=%s,index=%ld",
        bdmf_attr_get_enum_text_hlp(&rdpa_traffic_dir_enum_table, tm->dir), tm->index);

#ifndef XRDP
    /* Only SP and single_queue are supported for queue scheduler */
    if (tm->level == rdpa_tm_level_queue && (tm->mode != rdpa_tm_sched_sp && tm->mode != rdpa_tm_sched_disabled))
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "Scheduling mode of queue-level egress_tm must be SP or disable\n");

    if (tm->mode == rdpa_tm_sched_sp_wrr)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "SPP_WRR scheduling mode is not supported\n");
#endif

    if (egress_tm_is_service_q(tm))
    {
#ifdef XRDP
        if (tm->level == rdpa_tm_level_egress_tm)
            BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "Service queue, only queue level TM is supported\n");
        if (tm->num_queues > p_dpi_cfg->number_of_service_queues)
            BDMF_TRACE_RET_OBJ(BDMF_ERR_RANGE, mo, "Service queue, only %d queues are supported [%d]\n", p_dpi_cfg->number_of_service_queues, tm->num_queues);
#endif
    }
    else
    {
        if (tm->level == rdpa_tm_level_egress_tm && tm->dir == rdpa_dir_ds)
            BDMF_TRACE_RET_OBJ(BDMF_ERR_NOT_SUPPORTED, mo, "Only service queue multi-level scheduling supported in DS\n");
    }

    if (tm->level == rdpa_tm_level_egress_tm && tm->mode == rdpa_tm_sched_disabled && !tm->overall_rl)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "Egress_tm-level object can't be used with disabled scheduling\n");

    if (tm->overall_rl)
    {
        if (tm->dir == rdpa_dir_ds)
            BDMF_TRACE_RET_OBJ(BDMF_ERR_NOT_SUPPORTED, mo, "Overall RL is only supported in the US\n");
        if (us_overall_rl_obj)
            BDMF_TRACE_RET_OBJ(BDMF_ERR_ALREADY, mo, "US overall RL object already exists (%s)\n", us_overall_rl_obj->name);
        if (tm->mode != rdpa_tm_sched_disabled)
            BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "Scheduling mode must be \"disabled\" on overall RL object\n");
    }

    if (tm_objects[tm->index])
        BDMF_TRACE_RET_OBJ(BDMF_ERR_ALREADY, mo, "%s already exists\n", mo->name);

    /* Try to set egress_tm on parent object */
    if (mo->owner->drv == rdpa_port_drv())
    {
        rdpa_port_tm_cfg_t tm_cfg;
        rc = rdpa_port_tm_cfg_get(mo->owner, &tm_cfg);
        tm_cfg.sched = mo;
        rc = rc ? rc : rdpa_port_tm_cfg_set(mo->owner, &tm_cfg);
    }
#ifdef CONFIG_BCM_TCONT
    else if (mo->owner->drv == rdpa_tcont_drv())
    {
        rc = rdpa_tcont_egress_tm_set(mo->owner, mo);
    }
#endif
    if (rc)
        BDMF_TRACE_RET_OBJ(rc, mo, "%s: can't set on parent object %s\n", mo->name, mo->owner->name);

    if (egress_tm_is_service_q(tm))
    {
        /* service queue is always enabled */
        tm->enable_cfg.enable = 1;
#ifdef XRDP
        if (tm->dir == rdpa_dir_ds)
#else
        if (tm->dir == rdpa_dir_ds && tm->level == rdpa_tm_level_egress_tm)
#endif
            _rdpa_egress_tm_channel_set(mo, mo, tm->wan_type, RDPA_DS_SERVICE_Q_CHANNEL);
    }

    if (tm->mode == rdpa_tm_sched_disabled)
        tm->num_queues = 1;

    if (tm->level == rdpa_tm_level_egress_tm)
        tm->num_queues = 0;

    /* Propagate to RDD if enabled */
    if (tm->enable_cfg.enable)
        rc = egress_tm_enable_set(mo, 1, 0);

    if (rc)
       goto out_post_init_ex;

    tm_objects[tm->index] = mo;

    if (egress_tm_is_service_q(tm))
        goto out_post_init_ex;

    if (tm->overall_rl)
        us_overall_rl_obj = mo;

    if (tm->dir == rdpa_dir_ds)
        num_normal_ds_tm++;
    else
        num_normal_us_tm++;

out_post_init_ex:
    egress_tm_post_init_ex(mo);

    return rc;
}

static void egress_tm_channel_cleanup(struct bdmf_object *mo)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    tm_channel_t *channels = (tm->dir == rdpa_dir_ds) ? ds_channels : us_channels;
    int num_channels = (tm->dir == rdpa_dir_ds) ? RDPA_MAX_DS_CHANNELS : RDPA_MAX_US_CHANNELS;
    int i;

    for (i = 0; i < num_channels; i++)
    {
        tm_channel_t *ch = &channels[i];
        if (ch->egress_tm == mo)
        {
            BDMF_TRACE_DBG_OBJ(mo, "egress_tm_channel_cleanup channel_id %d info, E:%d, O:0x%p, E:0x%p)\n",
                ch->channel_id, ch->enable, ch->owner, ch->egress_tm);
            ch->enable = 0;
            ch->egress_tm = NULL;
        }
    }
}

static void egress_tm_destroy(struct bdmf_object *mo)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    struct bdmf_object **tm_objects = (tm->dir == rdpa_dir_ds) ? ds_tm_objects : us_tm_objects;
    int max_subs = (tm->dir == rdpa_dir_ds) ? RDPA_TM_MAX_DS_SCHED : RDPA_TM_MAX_US_SCHED;

    if ((unsigned)tm->index >= max_subs || tm_objects[tm->index] != mo)
        return;

    if (!egress_tm_is_service_q(tm))
    {
        if (tm->dir == rdpa_dir_ds)
            num_normal_ds_tm--;
        else
            num_normal_us_tm--;
    }

    if (tm->enable_cfg.enable)
        egress_tm_enable_set(mo, 0, 0);
    egress_tm_rdd_resources_free(mo);
    egress_tm_channel_cleanup(mo);

    tm_objects[tm->index] = NULL;
    if (mo == us_overall_rl_obj)
        us_overall_rl_obj = NULL;
}

/** Called when other object is linked with THIS object.
 * Only overall rate limiter supports "link" operation
 */
static int egress_tm_link(struct bdmf_object *this, struct bdmf_object *other,
    const char *link_attrs)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(this);
    tm_channel_t *channels = (tm->dir == rdpa_dir_ds) ? ds_channels : us_channels;
    int num_channels = (tm->dir == rdpa_dir_ds) ? RDPA_MAX_DS_CHANNELS : RDPA_MAX_US_CHANNELS;
    tm_drv_priv_t *channel_tm;
    int i;
    int rc = 0;

    if (!tm->overall_rl)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, this, "Can only be linked with overall_rl object: %s\n", other->name);

#ifdef EPON
    {
        bdmf_boolean is_overall = 0;

        if (tm->dir == rdpa_dir_us && other->drv == rdpa_llid_drv())
        {
            is_overall = 1;
        }
        else if (tm->dir == rdpa_dir_us && other->drv == rdpa_port_drv())
        {
            rdpa_if rdpaif = rdpa_if_none;
            rdpa_port_index_get(other, &rdpaif);

            if (rdpaif == rdpa_wan_type_to_if(rdpa_wan_epon))
                is_overall = 1;
        }

        if (is_overall && (rdpa_is_epon_or_xepon_mode() || rdpa_is_epon_ae_mode()))
        {
            if (is_rdpa_epon_ctc_or_cuc_mode())
            {
                if (NULL != global_shaper_cb)
                {
                    uint32_t rate = tm->rl_cfg.af_rate;

                    rc = global_shaper_cb(rate);
                    orl_tm_linked_to_llid = 1;
                    BDMF_TRACE_RET_OBJ(rc ? BDMF_ERR_INTERNAL : 0, this,
                        "OntDirGlobalShaperSet({rate=%d}) -> %d\n", (int)rate, rc);
                }
            }
            else
            {
                BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, this,
                    "overall_rl object's link to llid is not supported\n");
            }
        }
    }
#endif

    /* Set ORL for all channels owned by ""other" */
    for (i = 0; i < num_channels && !rc; i++)
    {
        tm_channel_t *channel = &channels[i];
        if (channel->owner == other)
        {
            channel_tm = (tm_drv_priv_t *)bdmf_obj_data(channel->egress_tm);
            rc = egress_tm_orl_set_on_channel(channel->egress_tm, channel, this, channel_tm->orl_prty);
        }
    }

    return rc;
}

/** Called when object's downlink is disconnected */
static void egress_tm_unlink(struct bdmf_object *this, struct bdmf_object *other)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(this);
    tm_channel_t *channels = (tm->dir == rdpa_dir_ds) ? ds_channels : us_channels;
    int num_channels = (tm->dir == rdpa_dir_ds) ? RDPA_MAX_DS_CHANNELS : RDPA_MAX_US_CHANNELS;
    int i;

#ifdef EPON
    if (other->drv == rdpa_llid_drv() && is_rdpa_epon_ctc_or_cuc_mode())
        orl_tm_linked_to_llid = 0;
#endif

    /* Clear ORL for all channels owned by ""other" */
    for (i = 0; i < num_channels; i++)
    {
        tm_channel_t *channel = &channels[i];
        if (channel->owner == other)
            egress_tm_orl_set_on_channel(channel->egress_tm, channel, NULL, 0);
    }
}

/** "rl" attribute's "write" callback */
static int egress_tm_attr_rl_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    uint32_t rate_limiter_timer_period;
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_tm_rl_cfg_t *rl_cfg = (rdpa_tm_rl_cfg_t *)val;
    int rc;

    if (mo->state != bdmf_state_init && tm->enable_cfg.enable)
    {
        rate_limiter_timer_period = (tm->dir == rdpa_dir_ds) ? DS_RATE_LIMITER_TIMER_PERIOD_IN_USEC : US_RATE_LIMITER_TIMER_PERIOD_IN_USEC;
        rl_cfg->burst_size = rdd_rate_to_alloc_unit(((rl_cfg->af_rate + rl_cfg->be_rate) / BITS_IN_BYTE), rate_limiter_timer_period);

        rc = egress_tm_rl_config(mo, rl_cfg, tm->weight, tm->enable_cfg.enable);
        if (rc < 0)
            return rc;
    }
    tm->rl_cfg = *rl_cfg;

    return 0;
}

int egress_tm_attr_rl_rate_mode_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val,
    uint32_t size)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_tm_rl_rate_mode *rl_rate_mode = (rdpa_tm_rl_rate_mode *)val;

#ifndef XRDP
    if (tm->level == rdpa_tm_level_queue)
        return BDMF_ERR_NOENT;
#endif

    *rl_rate_mode = tm->rl_rate_mode;
    return 0;
}

static int egress_tm_attr_rl_rate_mode_write(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_tm_rl_rate_mode *rl_rate_mode = (rdpa_tm_rl_rate_mode *)val;

#ifndef XRDP
    if (tm->level == rdpa_tm_level_queue && *rl_rate_mode != rdpa_tm_rl_single_rate)
        return BDMF_ERR_INVALID_OP;
#endif

    tm->rl_rate_mode = *rl_rate_mode;

    return 0;
}

static int egress_tm_attr_num_queues_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val,
    uint32_t size)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    uint8_t *p_num_queues = (uint8_t *)val;

    *p_num_queues = tm->num_queues;
    return 0;
}

static int egress_tm_attr_num_queues_write(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    uint8_t num_queues = *(uint8_t *)val;

    if (tm->level != rdpa_tm_level_queue)
        return BDMF_ERR_INVALID_OP;

    if (egress_tm_is_service_q(tm))
    {
#ifdef XRDP
        if (num_queues > p_dpi_cfg->number_of_service_queues)
            return BDMF_ERR_RANGE;
#else
        return BDMF_ERR_NOT_SUPPORTED;
#endif
    }

    tm->num_queues = num_queues;

    return 0;
}

static int egress_tm_attr_num_sp_elements_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val,
    uint32_t size)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_tm_num_sp_elem *p_num_sp_elements = (rdpa_tm_num_sp_elem *)val;

    if (tm->mode != rdpa_tm_sched_sp_wrr)
        return BDMF_ERR_NOT_SUPPORTED;

    *p_num_sp_elements = tm->num_sp_elements;
    return 0;
}

static int egress_tm_attr_num_sp_elements_write(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_tm_num_sp_elem num_sp_elements = *(rdpa_tm_num_sp_elem *)val;
    int rc = 0;
    int i;

    if (tm->mode != rdpa_tm_sched_sp_wrr)
        return BDMF_ERR_NOT_SUPPORTED;
    if (tm->level == rdpa_tm_level_queue && num_sp_elements > tm->num_queues)
        return BDMF_ERR_RANGE;
    /* Must be a power of 2 > 0 */
    if (num_sp_elements & (num_sp_elements - 1))
        return BDMF_ERR_PARM;

    /* If TM object is already active, try to change num_sp_elements on the fly */
    if (mo->state == bdmf_state_active)
    {
        /* All WRR elements must have weight */
        if (tm->level == rdpa_tm_level_egress_tm)
        {
            for (i = num_sp_elements; i < tm->num_queues; i++)
            {
                if (tm->sub_tms[i])
                {
                    tm_drv_priv_t *stm = (tm_drv_priv_t *)bdmf_obj_data(tm->sub_tms[i]);
                    if (!(stm->weight >= RDPA_MIN_WEIGHT && stm->weight <= RDPA_MAX_WEIGHT))
                    {
                        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "WRR weight is not set for %s\n",
                            tm->sub_tms[i]->name);
                    }
                }
            }
        }
        else
        {
            for (i = num_sp_elements; i < tm->num_queues; i++)
            {
                rdpa_tm_queue_cfg_t *q_cfg = &tm->queue_cfg[i];
                if (q_cfg->drop_threshold &&
                    !(q_cfg->weight >= RDPA_MIN_WEIGHT && q_cfg->weight <= RDPA_MAX_WEIGHT))
                {
                    BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "WRR weight is not set for queue %d of %s\n",
                        i, mo->name);
                }
            }
        }
        rc = egress_tm_num_sp_elements_config(mo, num_sp_elements);
    }

    if (!rc)
        tm->num_sp_elements = num_sp_elements;

    return rc;
}

/* "queue_cfg" attribute "read" callback */
static int egress_tm_attr_queue_cfg_read(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_tm_queue_cfg_t *cfg = (rdpa_tm_queue_cfg_t *)val;

    if (tm->level != rdpa_tm_level_queue)
        return BDMF_ERR_NOT_SUPPORTED;
    if (index >= tm->num_queues)
        return BDMF_ERR_RANGE;
    if (tm->mode == rdpa_tm_sched_disabled && index)
        return BDMF_ERR_RANGE;
    if (!tm->queue_cfg[index].drop_threshold && !tm->queue_id_assigned[index])
        return BDMF_ERR_NOENT;
    *cfg = tm->queue_cfg[index];

    return 0;
}

/* get queue index from queue id */
static int get_queue_index(tm_drv_priv_t *tm, int queue_id)
{
    int i;

    if (queue_id == BDMF_INDEX_UNASSIGNED)
        return BDMF_INDEX_UNASSIGNED;

    for (i = 0; i < tm->num_queues; i++)
        if (tm->queue_cfg[i].queue_id == queue_id)
            return i;

    return BDMF_INDEX_UNASSIGNED;
}

static int egress_tm_location_read(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_tm_queue_location_t *loc = (rdpa_tm_queue_location_t *)val;
    int ret = BDMF_ERR_NOENT;
    struct bdmf_object *sub;
    int i;

    for (i = 0; i < RDPA_TM_MAX_SCHED_ELEMENTS; ++i)
    {
        sub = tm->sub_tms[i];
        if (sub)
        {
            ret = egress_tm_location_read(sub, ad, index, val, size);
            if (!ret)
                break;
        }
    }

    if (ret == BDMF_ERR_NOENT && tm->level == rdpa_tm_level_queue)
    {
        loc->queue_idx = get_queue_index(tm, index);

        if (loc->queue_idx != BDMF_INDEX_UNASSIGNED)
        {
            loc->queue_tm = mo;
            ret = BDMF_ERR_OK;
        }
    }

    return ret;
}

/* delete single queue from a channel */
static int egress_tm_attr_queue_cfg_delete(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    tm_channel_t *ch = egress_tm_channel_get_first(mo);
    bdmf_error_t rc = BDMF_ERR_NOENT;

    if (index >= RDPA_MAX_EGRESS_QUEUES)
    {
        BDMF_TRACE_ERR("Queue index (%ld) out of range\n", index);
        return BDMF_ERR_RANGE;
    }

    while (ch)
    {
        tm_qtm_ctl_t *qtm_ctl = egress_tm_qtm_ctl_get(mo, ch);
        if (qtm_ctl)
        {
            rc = BDMF_ERR_OK;
            /* we found an egress_tm channel- delete the relevant queue */
            BDMF_TRACE_DBG_OBJ(mo, "%s: delete queue:: channel: %d, queue_id: %ld\n", mo->name, ch->channel_id, index);
            egress_tm_delete_single_queue(mo, qtm_ctl, ch, tm, index);
            tm->queue_cfg[index].queue_id = BDMF_INDEX_UNASSIGNED;
            tm->queue_cfg[index].drop_threshold = 0;
        }

        ch = egress_tm_channel_get_next(mo, ch);
    }

    if (rc)
        BDMF_TRACE_ERR("Could not find requested entry for queue %ld\n", index);
    return rc;
}

static int egress_tm_queue_cfg_val_to_s(struct bdmf_object *mo, struct bdmf_attr *ad, const void *val, char *sbuf, uint32_t size)
{
    rdpa_tm_queue_cfg_t *cfg = (rdpa_tm_queue_cfg_t *)val;
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    queue_info_t queue_id_info = {0};
    int queue_index;
    int rc;

    queue_index = get_queue_index(tm, cfg->queue_id);

    if (queue_index == BDMF_INDEX_UNASSIGNED)
        return BDMF_ERR_NOENT;

    rc = snprintf(sbuf, size, "{queue_id=%d,drop_threshold=%d,weight=%d,drop_alg=%s",
                  cfg->queue_id,
                  cfg->drop_threshold,
                  cfg->weight,
                  bdmf_attr_get_enum_text_hlp(&tm_drop_policy_enum_table, cfg->drop_alg));
    if ((int)size - rc < 0)
        return BDMF_ERR_INTERNAL;
    size -= rc;
    sbuf += rc;
    if (cfg->drop_alg == rdpa_tm_drop_alg_red || cfg->drop_alg == rdpa_tm_drop_alg_wred)
    {
#if defined(BCM_DSL_RDP)
        rc = snprintf(sbuf, size, ",high_class={min_thresh=%d,max_thresh=%d},low_class={min_thresh=%d,max_thresh=%d}"
                      "priority_mask_0=0x%08x,priority_mask_1=0x%08x",
                      cfg->high_class.min_threshold,
                      cfg->high_class.max_threshold,
                      cfg->low_class.min_threshold,
                      cfg->low_class.max_threshold,
                      cfg->priority_mask_0,
                      cfg->priority_mask_1);
#else
        rc = snprintf(sbuf, size, ",high_class={min_thresh=%d,max_thresh=%d,max_drop_prob=%d},low_class={min_thresh=%d,max_thresh=%d,max_drop_prob=%d}",
                      cfg->high_class.min_threshold,
                      cfg->high_class.max_threshold,
                      cfg->high_class.max_drop_probability,
                      cfg->low_class.min_threshold,
                      cfg->low_class.max_threshold,
                      cfg->low_class.max_drop_probability);
#endif
        if ((int)size - rc < 0)
            return BDMF_ERR_INTERNAL;
        size -= rc;
        sbuf += rc;
    }
#ifdef XRDP
    if (cfg->rl_cfg.af_rate || cfg->rl_cfg.be_rate || cfg->rl_cfg.burst_size)
    {
        rc = snprintf(sbuf, size, ",rl={af=%llu,be=%llu,burst=%llu}",
                    (long long int)cfg->rl_cfg.af_rate,
                    (long long int)cfg->rl_cfg.be_rate,
                    (long long int)cfg->rl_cfg.burst_size);
        if ((int)size - rc < 0)
            return BDMF_ERR_INTERNAL;
        size -= rc;
        sbuf += rc;
    }

    if (cfg->reserved_packet_buffers)
    {
        rc = snprintf(sbuf, size, ",reserved_packet_buffers=%d",
                    cfg->reserved_packet_buffers);
        if ((int)size - rc < 0)
            return BDMF_ERR_INTERNAL;
        size -= rc;
        sbuf += rc;
    }

#endif
    rc = snprintf(sbuf, size, ",stat_enable=%s, active=%s}",
            cfg->stat_enable ? "yes" : "no",
            tm->queue_configured[queue_index] ? "yes" : "no");
    if ((int)size - rc < 0)
        return BDMF_ERR_INTERNAL;
    size -= rc;
    sbuf += rc;

    if (bdmf_global_trace_level == bdmf_trace_level_debug)
    {
        int16_t hashed_channel_id;
        struct bdmf_object *root = egress_tm_get_root_object(mo);
        tm_drv_priv_t *root_tm = (tm_drv_priv_t *)bdmf_obj_data(root);
        tm_channel_t *ch = egress_tm_channel_get_first(mo);

        if (ch == NULL)
        {
        	rc = snprintf(sbuf, 64, "===>debug info: cant find channel");
            return 0;
        }
        hashed_channel_id = egress_tm_is_group_owner(root_tm) ? root_tm->channel_group : ch->channel_id;
        if (egress_tm_queue_id_info_get(tm->dir, hashed_channel_id, cfg->queue_id, &queue_id_info) == 0)
        {
            rc = snprintf(sbuf, size, "===>debug info: channel=%d queue=%d, rate_controller=%d",
                          queue_id_info.channel, queue_id_info.queue, queue_id_info.rc_id);
        }
        else
        {
            rc = snprintf(sbuf, size, "===>debug get queue_id_info FAILED. params: DIR %d, channel_id %d, queue_id %d\n",
                    tm->dir, ch->channel_id, cfg->queue_id);
        }

        if ((int)size - rc < 0)
            return BDMF_ERR_INTERNAL;
    }
    return 0;
}

/* "queue_cfg" attribute "write" callback */
static int egress_tm_attr_queue_cfg_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_tm_queue_cfg_t cfg = *(rdpa_tm_queue_cfg_t *)val;
    uint32_t rate_limiter_timer_period;
    bdmf_error_t rc = 0;

    if (tm->level != rdpa_tm_level_queue)
        return BDMF_ERR_NOT_SUPPORTED;
    if (tm->mode == rdpa_tm_sched_disabled && index)
        return BDMF_ERR_RANGE;
    if (index >= tm->num_queues)
        return BDMF_ERR_RANGE;

    if (tm->queue_cfg[index].queue_id != BDMF_INDEX_UNASSIGNED &&
        tm->queue_cfg[index].queue_id != cfg.queue_id)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "assigned queue_id can not be changed\n");
    }

#ifdef XRDP
    cfg.stat_enable = 1;
#endif

    rate_limiter_timer_period = (tm->dir == rdpa_dir_ds) ? DS_RATE_LIMITER_TIMER_PERIOD_IN_USEC : US_RATE_LIMITER_TIMER_PERIOD_IN_USEC;
    cfg.rl_cfg.burst_size = rdd_rate_to_alloc_unit(((cfg.rl_cfg.af_rate + cfg.rl_cfg.be_rate) / BITS_IN_BYTE), rate_limiter_timer_period);

    if (mo->state != bdmf_state_init)
    {
        /* In this level it's good enough to work in single queue resolution */
        rc = egress_tm_queue_cfg(mo, index, &cfg, tm->enable_cfg.enable);
        if (rc < 0)
            return rc;
    }
    tm->queue_cfg[index] = cfg;

    return 0;
}

/* read statistics of a single queue given RDD indexes */
static int egress_tm_queue_stat_read(tm_drv_priv_t *tm, tm_queue_hash_entry_t *qentry, rdpa_stat_1way_t *stat)
{
    return rdpa_rdd_tx_queue_stat_read(tm->this, qentry, stat);
}

/* clear statistics of a single queue given RDD indexes */
static int egress_tm_queue_stat_clear(tm_drv_priv_t *tm, tm_queue_hash_entry_t *qentry)
{
    return rdpa_rdd_tx_queue_stat_clear(tm->this, qentry);
}

/* "queue_stat" attribute "read" callback */
static int _egress_tm_attr_queue_stat_read_all(struct bdmf_object *mo,
    struct bdmf_object *root, struct bdmf_attr *ad, rdpa_tm_queue_index_t *qi,
    rdpa_stat_1way_t *stat)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    tm_drv_priv_t *root_tm = (tm_drv_priv_t *)bdmf_obj_data(root);
    rdpa_stat_1way_t sub_stat;
    int rc = 0;
    int i;

    memset(stat, 0, sizeof(*stat));

    /* Special handling for channel group - sum-up for all subsidiaries */
    if (mo == root && egress_tm_is_ch_group(root_tm))
    {
        for (i = 0; i < RDPA_TM_MAX_SCHED_ELEMENTS && !rc; i++)
        {
            if (!tm->sub_tms[i])
                continue;
            memset(&sub_stat, 0, sizeof(sub_stat));

            rc |= _egress_tm_attr_queue_stat_read_all(tm->sub_tms[i],
                    root, ad, qi, &sub_stat);
            stat->passed.packets    += sub_stat.passed.packets;
            stat->passed.bytes      += sub_stat.passed.bytes;
            stat->discarded.packets += sub_stat.discarded.packets;
            stat->discarded.bytes   += sub_stat.discarded.bytes;
        }
    }
    else
    {
        struct bdmf_object *top = egress_tm_get_top_object(mo);
        tm_channel_t *channels = (tm->dir == rdpa_dir_ds) ? ds_channels : us_channels;
        int num_channels = (tm->dir == rdpa_dir_ds) ? RDPA_MAX_DS_CHANNELS : RDPA_MAX_US_CHANNELS;
        tm_queue_hash_entry_t *qentry;

        for (i = 0; i < num_channels && !rc; i++)
        {
            tm_channel_t *ch = &channels[i];
            if (ch->egress_tm != top)
                continue;

            qentry = egress_tm_hash_get_by_dir_channel_queue(tm->dir, ch->channel_id, qi->queue_id);
            if (!qentry)
                continue;

            memset(&sub_stat, 0, sizeof(sub_stat));

            rc |= egress_tm_queue_stat_read(tm, qentry, &sub_stat);
            stat->passed.packets    += sub_stat.passed.packets;
            stat->passed.bytes      += sub_stat.passed.bytes;
            stat->discarded.packets += sub_stat.discarded.packets;
            stat->discarded.bytes   += sub_stat.discarded.bytes;
        }
    }

    return rc;
}

static int _egress_tm_attr_queue_stat_read_one(struct bdmf_object *mo,
    struct bdmf_object *root, struct bdmf_attr *ad, rdpa_tm_queue_index_t *qi,
    rdpa_stat_1way_t *stat)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    tm_drv_priv_t *root_tm = (tm_drv_priv_t *)bdmf_obj_data(root);
    struct bdmf_object *top = egress_tm_get_top_object(mo);
    tm_channel_t *channels = (tm->dir == rdpa_dir_ds) ? ds_channels : us_channels;
    int num_channels = (tm->dir == rdpa_dir_ds) ? RDPA_MAX_DS_CHANNELS : RDPA_MAX_US_CHANNELS;
    tm_queue_hash_entry_t *qentry;
    rdpa_stat_1way_t sub_stat;
    int16_t hashed_channel_id;
    int rc = 0;
    int i;

#if defined(BCM_DSL_RDP)
    /* Queue statistics are implemented using Runner Counters that are cleared after read.
     * To avoid counters being cleared during subsidiary tm stat queries, we only return
     * the statistics for all queues when root tm is queried.
     */
    if (mo != root)
        return BDMF_ERR_OK;
#endif

    memset(stat, 0, sizeof(*stat));

    for (i = 0; i < num_channels && !rc; i++)
    {
        tm_channel_t *ch = &channels[i];
        if (ch->egress_tm != top)
            continue;

        hashed_channel_id = egress_tm_is_group_owner(root_tm) ?
            root_tm->channel_group : ch->channel_id;
        BDMF_TRACE_DBG_OBJ(mo, "egress_tm_hash_get_by_dir_channel_queue(%d %d)\n",
            (int)hashed_channel_id, (int)qi->queue_id);
        qentry = egress_tm_hash_get_by_dir_channel_queue(tm->dir,
             hashed_channel_id, qi->queue_id);
        if (!qentry)
            continue;

        if (rdpa_is_car_mode() && tm->dir == rdpa_dir_us)
        {
            memset(&sub_stat, 0, sizeof(sub_stat));

            rc |= egress_tm_queue_stat_read(tm, qentry, &sub_stat);
            stat->passed.packets    += sub_stat.passed.packets;
            stat->passed.bytes      += sub_stat.passed.bytes;
            if (_rdpa_system_cfg_get()->counter_type == rdpa_counter_watermark)
            {
                stat->discarded.packets = MAX(stat->discarded.packets, sub_stat.discarded.packets);
                stat->discarded.bytes   = MAX(stat->discarded.bytes, sub_stat.discarded.bytes);
            }
            else
            {
                stat->discarded.packets += sub_stat.discarded.packets;
                stat->discarded.bytes   += sub_stat.discarded.bytes;
            }
        }
        else
        {
            rc |= egress_tm_queue_stat_read(tm, qentry, stat);
        }
    }

    return rc;
}

/* "queue_stat" attribute "read" callback */
static int egress_tm_attr_queues_stat_read(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    rdpa_stat_1way_t *stat = (rdpa_stat_1way_t *)val;
    rdpa_tm_queue_index_t *qi = (rdpa_tm_queue_index_t *)index;
    struct bdmf_object *root = egress_tm_get_root_object(mo);

    BDMF_TRACE_DBG_OBJ(mo, "index=%d\n", (int)index);

    if (!qi || !val)
        return BDMF_ERR_PARM;

    memset(stat, 0, sizeof(*stat));

    /* UNASSIGNED means "all queues" */
    if (*(bdmf_index *)qi == BDMF_INDEX_UNASSIGNED)
        return _egress_tm_attr_queue_stat_read_all(mo, root, ad, qi, stat);
    else
        return _egress_tm_attr_queue_stat_read_one(mo, root, ad, qi, stat);

    return BDMF_ERR_OK;
}

static int _egress_tm_attr_queue_stat_write_all(struct bdmf_object *mo,
    struct bdmf_object *root, struct bdmf_attr *ad, rdpa_tm_queue_index_t *qi)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    tm_drv_priv_t *root_tm = (tm_drv_priv_t *)bdmf_obj_data(root);
    struct bdmf_object *top = egress_tm_get_top_object(mo);
    tm_channel_t *channels = (tm->dir == rdpa_dir_ds) ? ds_channels : us_channels;
    int num_channels = (tm->dir == rdpa_dir_ds) ? RDPA_MAX_DS_CHANNELS : RDPA_MAX_US_CHANNELS;
    tm_queue_hash_entry_t *qentry;
    int rc = 0;
    int i;

    /* Special handling for channel group - sum-up for all subsidiaries */
    if (mo == root && egress_tm_is_ch_group(root_tm))
    {
        for (i = 0; i < RDPA_TM_MAX_SCHED_ELEMENTS && !rc; i++)
        {
            if (!tm->sub_tms[i])
                continue;
            rc |= _egress_tm_attr_queue_stat_write_all(tm->sub_tms[i], root, ad, qi);
        }
    }
    else
    {
        for (i = 0; i < num_channels && !rc; i++)
        {
            tm_channel_t *ch = &channels[i];
            if (ch->egress_tm != top)
                continue;

            qentry = egress_tm_hash_get_by_dir_channel_queue(tm->dir, ch->channel_id, qi->queue_id);
            if (!qentry)
                continue;

            rc |= egress_tm_queue_stat_clear(tm, qentry);
        }
    }

    return rc;
}

static int _egress_tm_attr_queue_stat_write_one(struct bdmf_object *mo,
    struct bdmf_object *root, struct bdmf_attr *ad, rdpa_tm_queue_index_t *qi)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    tm_drv_priv_t *root_tm = (tm_drv_priv_t *)bdmf_obj_data(root);
    tm_queue_hash_entry_t *qentry;
    int16_t hashed_channel_id;

#if !defined(BCM_DSL_RDP)
    /* Queue statistics are implemented using Runner Counters that are cleared after read.
     * To avoid counters being cleared during subsidiary tm stat queries, we only return
     * the statistics for all queues when root tm is queried.
     */
    if (mo != root)
        return BDMF_ERR_OK;
#endif

    /* Get stats of specific queue */
    hashed_channel_id = egress_tm_is_group_owner(root_tm) ? root_tm->channel_group : qi->channel;
    qentry = egress_tm_hash_get_by_dir_channel_queue(tm->dir, hashed_channel_id, qi->queue_id);
    if (!qentry)
        return BDMF_ERR_NOENT;

    return egress_tm_queue_stat_clear(tm, qentry);
}

/* "queue_stat" attribute "write" callback */
static int egress_tm_attr_queue_stat_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    rdpa_tm_queue_index_t *qi = (rdpa_tm_queue_index_t *)index;
    struct bdmf_object *root = egress_tm_get_root_object(mo);

    if (!qi || !val)
        return BDMF_ERR_PARM;

    /* UNASSIGNED means "all queues" */
    if (*(bdmf_index *)qi == BDMF_INDEX_UNASSIGNED)
        return _egress_tm_attr_queue_stat_write_all(mo, root, ad, qi);
    else
        return _egress_tm_attr_queue_stat_write_one(mo, root, ad, qi);
}

static int egress_tm_attr_deprecated_read(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

static int egress_tm_attr_deprecated_get_next(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index *index)
{
    return BDMF_ERR_NO_MORE;
}


/* "queue_occupancy" attribute "read" callback */
static int egress_tm_attr_queue_occupancy_read(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    return egress_tm_queue_occupancy_read_ex(mo, ad, index, val, size);
}

bdmf_error_t egress_tm_is_empty_on_channel(bdmf_object_handle tm_obj, uint32_t channel_index, bdmf_boolean *is_empty)
{
    return egress_tm_is_empty_on_channel_ex(tm_obj, channel_index, is_empty);
}

/* "sub_tm" attribute "read" callback */
static int egress_tm_attr_sub_tm_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val,
    uint32_t size)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);

    if (tm->level != rdpa_tm_level_egress_tm)
    {
        bdmf_object_handle sub_tm = tm->sub_tms[index];
        tm_drv_priv_t *ss = sub_tm ? (tm_drv_priv_t *)bdmf_obj_data(sub_tm) : NULL;

        /* subsidiary service queues are allowed under queue-level tms */
        if (ss && egress_tm_is_service_q(ss))
        {
            *(bdmf_object_handle *)val = sub_tm;
            return 0;
        }

        return BDMF_ERR_NOT_SUPPORTED;
    }
    if (!tm->sub_tms[index])
        return BDMF_ERR_NOENT;

    *(bdmf_object_handle *)val = tm->sub_tms[index];

    return 0;
}

/* "sub_tm" attribute "find" callback
 * return subsidiary index and its owner */
static int egress_tm_attr_sub_tm_find(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index *index, void *val,
    uint32_t size)
{
    tm_drv_priv_t *owner_tm = NULL;
    bdmf_object_handle sub_tm = *(bdmf_object_handle *)val;
    bdmf_object_handle owner = sub_tm->owner;
    bdmf_index i;

    if (owner->drv != rdpa_egress_tm_drv())
        return BDMF_ERR_NOENT;

    owner_tm = (tm_drv_priv_t *)bdmf_obj_data(owner);

    for (i = 0; i < RDPA_TM_MAX_SCHED_ELEMENTS; i++)
    {
        if (owner_tm->sub_tms[i] == sub_tm)
        {
            *index = i;
            *(bdmf_object_handle *)val = owner;
            return BDMF_ERR_OK;
        }
    }

    BDMF_TRACE_ERR("subsidiary tm not held by owner");
    return BDMF_ERR_INTERNAL;
}

/* check if egress_tm RDD resources has to be re-allocated.
 * It happens when egress_tm is (group, SP) and subsidiaries are configured
 * out of order. In this case it is important to ensure that rate controllers are
 * allocated to subsidieries in increasing order because RC index signifies priority
 */
static int egress_tm_is_reallocate(tm_drv_priv_t *tm, int new_ss_index)
{
    int i;

    if (tm->mode != rdpa_tm_sched_sp && tm->mode != rdpa_tm_sched_sp_wrr)
        return 0;

    for (i = new_ss_index + 1; i < RDPA_TM_MAX_SCHED_ELEMENTS; i++)
    {
        if (tm->sub_tms[i])
            return 1;
    }

    return 0;
}

/* update num_queues all the way up to root */
static void update_num_queues(tm_drv_priv_t *tm, tm_drv_priv_t *sub_tm, int num_queues)
{
    /* ignore queue counts for service queues under non-service queue egress_tms */
    if (!egress_tm_is_service_q(tm) && egress_tm_is_service_q(sub_tm))
        return;

    tm->num_queues += num_queues;

    if (tm->upper_level_tm)
        update_num_queues((tm_drv_priv_t *)bdmf_obj_data(tm->upper_level_tm), tm, num_queues);
}

/* "sub_tm" attribute "write" callback */
static int egress_tm_attr_sub_tm_write(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_object_handle sub_tm = *(bdmf_object_handle *)val;
    tm_drv_priv_t *ss = sub_tm ? (tm_drv_priv_t *)bdmf_obj_data(sub_tm) : NULL;
    bdmf_object_handle old_sub_tm;
    tm_drv_priv_t *old_ss = NULL;
    bdmf_object_handle old_parent = NULL;
    int enable = tm->enable_cfg.enable;
    bdmf_error_t rc = 0;

    if (tm->level != rdpa_tm_level_egress_tm && ss && !egress_tm_is_service_q(ss))
        BDMF_TRACE_RET_OBJ(BDMF_ERR_NOT_SUPPORTED, mo, "Queue-level egress_tm\n");

    if (tm->overall_rl && !egress_tm_is_service_q(tm))
        BDMF_TRACE_RET_OBJ(BDMF_ERR_NOT_SUPPORTED, mo, "Overall RL egress_tm\n");

    if (egress_tm_is_ch_group(tm) && (unsigned)index >= tm->num_channels)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_RANGE, mo,
            "Subsidiary index %ld exceeds number of channels in channel "
            "group (%d)\n", index, tm->num_channels);
    }

    if (tm->sub_tms[index] == sub_tm)
        return 0; /* Nothing to do */

    /* Basic validation */
    if (sub_tm)
    {
        if (sub_tm->drv != rdpa_egress_tm_drv())
            BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "Wrong object type of %s\n", sub_tm->name);

        if (ss->upper_level_tm)
        {
            BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "%s is subsidiary to %s\n", sub_tm->name,
                ss->upper_level_tm->name);
        }

        if (ss->dir != tm->dir)
            BDMF_TRACE_RET_OBJ(BDMF_ERR_NOT_SUPPORTED, sub_tm, "Subsidiary egress tm direction mismatch\n");

        if (egress_tm_is_service_q(tm) && !egress_tm_is_service_q(ss))
            BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "Subsidiary of service queue must also be service queue\n");
    }

    /* Release resources associated with old subsidiary scheduler */
    old_sub_tm = tm->sub_tms[index];
    if (old_sub_tm)
    {
        bdmf_object_handle system_obj = NULL;

        old_ss = (tm_drv_priv_t *)bdmf_obj_data(old_sub_tm);

        if (mo->state != bdmf_state_init)
        {
            if (tm->enable_cfg.enable)
                egress_tm_enable_set(old_sub_tm, 0, 0);
            if (egress_tm_is_ch_group(tm))
                _rdpa_egress_tm_channel_set(old_sub_tm, NULL, tm->wan_type, tm->channels[index]);
            else
                egress_tm_rdd_resources_free(old_sub_tm);
        }

        old_ss->upper_level_tm = NULL;
        update_num_queues(tm, old_ss, -old_ss->num_queues);

        /* Re-parent to system */
        rdpa_system_get(&system_obj);
        bdmf_object_parent_set(old_sub_tm, system_obj);
        bdmf_put(system_obj);
    }

    /* Set new subsidiary */
    tm->sub_tms[index] = sub_tm;
    if (sub_tm)
    {
        old_parent = sub_tm->owner;
        ss->wan_type = tm->wan_type;

        /* Make sure that total number of scheduling levels is not exceeded */
        rc = egress_tm_check_levels(mo);
        if (rc)
            goto done;

        /* If root TM is channel group (LLID), each subsidiary is considered top in its hierarchy */
        ss->upper_level_tm = egress_tm_is_ch_group(tm) ? NULL : mo;
        bdmf_object_parent_set(sub_tm, mo);

        /* (Re)allocate RDD resources and enable if necessary */
        if (egress_tm_channel_get_first(mo) || egress_tm_is_service_q(tm) || egress_tm_is_ch_group(tm))
        {
            bdmf_object_handle alloc_tm = sub_tm; /* tm to allocate resources on */
            if (egress_tm_is_ch_group(tm))
            {
                _rdpa_egress_tm_channel_set(sub_tm, tm->channel_group_owner, tm->wan_type, tm->channels[index]);
            }
            else
            {
                rc = egress_tm_validate_hierarchy(egress_tm_get_top_object(mo));
                if (!rc && egress_tm_is_reallocate(tm, index))
                {
                    tm->sub_tms[index] = NULL; /* disconnect for the moment */
                    egress_tm_enable_set(mo, 0, 0);
                    egress_tm_rdd_resources_free(mo);
                    alloc_tm = mo;
                    tm->sub_tms[index] = sub_tm;
                }
                rc = rc ? rc : egress_tm_rdd_resources_alloc(alloc_tm, index);
            }

            rc = rc ? rc : egress_tm_enable_set(alloc_tm, enable, 0);
        }

        update_num_queues(tm, ss, ss->num_queues);
    }

done:
    if (rc < 0 && sub_tm)
    {
        if (egress_tm_is_ch_group(tm))
            _rdpa_egress_tm_channel_set(sub_tm, NULL, tm->wan_type, tm->channels[index]);
        else
            egress_tm_rdd_resources_free(sub_tm);
        ss->upper_level_tm = NULL;
        tm->sub_tms[index] = NULL;
        bdmf_object_parent_set(sub_tm, old_parent);
        /* Try to restore old subsidiary if any */
        if (old_sub_tm)
            egress_tm_attr_sub_tm_write(mo, ad, index, &old_sub_tm, size);
        BDMF_TRACE_RET_OBJ(rc, sub_tm, "failed to configure egress tm\n");
    }
    return 0;
}

/** "weight" attribute's "write" callback */
static int egress_tm_attr_weight_write(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    uint32_t weight = *(uint32_t *)val;

    if (tm->overall_rl)
        return BDMF_ERR_NOT_SUPPORTED;

    if (mo->state != bdmf_state_init && tm->enable_cfg.enable)
    {
        tm_drv_priv_t *upper_tm = tm->upper_level_tm ? (tm_drv_priv_t *)bdmf_obj_data(tm->upper_level_tm) : NULL;
        int is_wrr = upper_tm && egress_tm_is_wrr_elem(upper_tm, index);
        int rc;

        if (is_wrr)
        {
            /* weight is only relevant if upper is WRR.
             * Only validate if relevant, perhaps it is re-configuration in progress */
            if (weight < RDPA_MIN_WEIGHT || weight > RDPA_MAX_WEIGHT)
            {
                BDMF_TRACE_RET_OBJ(BDMF_ERR_RANGE, mo, "WRR weight %u is out of range %d..%d\n",
                    (unsigned)weight, (unsigned)RDPA_MIN_WEIGHT, (unsigned)RDPA_MAX_WEIGHT);
            }
            rc = egress_tm_rl_config(mo, &tm->rl_cfg, weight, tm->enable_cfg.enable);
            if (rc < 0)
                BDMF_TRACE_RET_OBJ(rc, mo, "Can't configure weight %d in RDD\n", weight);
        }
    }

    tm->weight = weight;
    return 0;
}

/* Does channel belong in mo hierarchy ? */
static int egress_tm_is_channel_in_mo_hierarchy(struct bdmf_object *mo, tm_channel_t *ch)
{
    struct bdmf_object *ch_mo = ch->egress_tm;
    return egress_tm_get_root_object(mo) == egress_tm_get_root_object(ch_mo);
}

/* Channel selector string to internal value */
static int egress_tm_channel_val_to_s(struct bdmf_object *mo,
    struct bdmf_attr *ad, const void *val, char *sbuf, uint32_t size)
{
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    int32_t channel_id = *(int32_t *)val;
    tm_channel_t *ch;

    if (egress_tm_is_service_q(tm) && tm->dir == rdpa_dir_ds)
    {
        snprintf(sbuf, size, "{%s}", "service_queue");
        return 0;
    }

    if (channel_id < 0)
    {
        strncpy(sbuf, "all", size);
        return 0;
    }
    ch = egress_tm_channel_get(tm->dir, channel_id);
    if (!ch || !ch->owner)
        return BDMF_ERR_NOENT;
    if (!egress_tm_is_channel_in_mo_hierarchy(mo, ch))
        return BDMF_ERR_NOENT;
    snprintf(sbuf, size, "{%s}", ch->owner->name);

    return 0;
}

/* Channel selector internal value to string */
static int egress_tm_channel_s_to_val(struct bdmf_object *mo_tmp,
    struct bdmf_attr *ad, const char *sbuf, void *val, uint32_t size)
{
    struct bdmf_object *mo = mo_tmp->drv_priv;
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    int32_t *channel_id = (int32_t *)val;
    tm_channel_t *channels = (tm->dir == rdpa_dir_ds) ? ds_channels : us_channels;
    int num_channels = (tm->dir == rdpa_dir_ds) ? RDPA_MAX_DS_CHANNELS : RDPA_MAX_US_CHANNELS;
    int i;

    if (!sbuf)
        return BDMF_ERR_PARM;

    if (egress_tm_is_service_q(tm) && tm->dir == rdpa_dir_ds && !strcmp(sbuf, "service_queue"))
    {
        *channel_id = RDPA_DS_SERVICE_Q_CHANNEL;
        return 0;
    }

    if (!strcmp(sbuf, "all"))
    {
        *channel_id = -1;
        return 0;
    }

    for (i = 0; i < num_channels; i++)
    {
        tm_channel_t *ch = &channels[i];
        if (ch->owner && !strcmp(ch->owner->name, sbuf) && egress_tm_is_channel_in_mo_hierarchy(mo, ch))
            break;
    }
    if (i >= num_channels)
        return BDMF_ERR_NOENT;
    *channel_id = i;

    return 0;
}

/*
 * Helpers for egress_tm_is_qtm_ctl_under_mo - get next channel+queue for queue_stat[]
 */

/* Does RC belong in mo hierarchy ? */
static int egress_tm_is_qtm_ctl_under_mo(struct bdmf_object *mo, tm_qtm_ctl_t *qtm_ctl)
{
    struct bdmf_object *rc_mo = qtm_ctl->egress_tm;

    while (rc_mo && rc_mo->drv == rdpa_egress_tm_drv())
    {
        if (rc_mo == mo)
            return 1;
        rc_mo = rc_mo->owner;
    }
    return 0;
}

/* get qtm_ctl, index by queue_id */
static tm_qtm_ctl_t *egress_tm_qtm_ctl_get_by_queue(struct bdmf_object *mo,
    tm_channel_t *channel, uint32_t queue_id, int *prty)
{
    tm_qtm_ctl_t *qtm_ctl, *qtm_ctl_tmp;

    STAILQ_FOREACH_SAFE(qtm_ctl, &channel->qtm_ctls, list, qtm_ctl_tmp)
    {
        tm_drv_priv_t *qtm_ctl_tm;
        int i;

        /* Skip rate controllers not under mo */
        if (!egress_tm_is_qtm_ctl_under_mo(mo, qtm_ctl))
            continue;

        qtm_ctl_tm = (tm_drv_priv_t *)bdmf_obj_data(qtm_ctl->egress_tm);
        for (i = 0; i < qtm_ctl_tm->num_queues; i++)
        {
            if (qtm_ctl->queue_configured[i] && qtm_ctl->hash_entry[i].queue_id == queue_id)
            {
                *prty = i;
                return qtm_ctl;
            }
        }
    }
    return NULL;
}

/* get 1st/next configured queue on rate controller */
static  bdmf_error_t egress_tm_qtm_ctl_get_next_queue(struct bdmf_object *mo,
    tm_channel_t *channel, tm_qtm_ctl_t *qtm_ctl, uint32_t prev_prty, uint32_t *queue_id)
{
    uint32_t i;

    while (qtm_ctl)
    {
        /* Skip rate controllers not under mo */
        if (egress_tm_is_qtm_ctl_under_mo(mo, qtm_ctl))
        {
            tm_drv_priv_t *qtm_ctl_tm = (tm_drv_priv_t *)bdmf_obj_data(qtm_ctl->egress_tm);
            /* Get next */
            if (qtm_ctl_tm->num_queues > RDPA_MAX_EGRESS_QUEUES)
            {
                *queue_id = BDMF_INDEX_UNASSIGNED;
                return BDMF_ERR_PARM;
            }
            for (i = prev_prty + 1; i < qtm_ctl_tm->num_queues; i++)
            {
                if (qtm_ctl->queue_configured[i])
                {
                    *queue_id = qtm_ctl->hash_entry[i].queue_id;
                    return BDMF_ERR_OK;
                }
            }
            prev_prty = -1;
        }
        qtm_ctl = STAILQ_NEXT(qtm_ctl, list);
    }
    *queue_id = BDMF_INDEX_UNASSIGNED;
    return BDMF_ERR_OK;
}

/* get 1st/next configured queue on channel */
static bdmf_error_t egress_tm_channel_get_next_queue(struct bdmf_object *mo, tm_channel_t *channel, uint32_t prev_queue_id, uint32_t *queue_id)
{
    tm_qtm_ctl_t *qtm_ctl;
    int prty = -1;
    int rc;

    /* Now find 1st/next queue */
    if (prev_queue_id == BDMF_INDEX_UNASSIGNED)
        qtm_ctl = STAILQ_FIRST(&channel->qtm_ctls);
    else
        qtm_ctl = egress_tm_qtm_ctl_get_by_queue(mo, channel, prev_queue_id, &prty);

    if (!qtm_ctl)
    {
        *queue_id = BDMF_INDEX_UNASSIGNED;
        return BDMF_ERR_OK;
    }
    rc = egress_tm_qtm_ctl_get_next_queue(mo, channel, qtm_ctl, prty, queue_id);

    return rc;
}


/* get next queue+channel callback */
static int egress_tm_queue_channel_get_next(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index *index)
{
    rdpa_tm_queue_index_t *qi = (rdpa_tm_queue_index_t *)index;
    tm_drv_priv_t *tm = (tm_drv_priv_t *)bdmf_obj_data(mo);
    tm_channel_t *ch;
    bdmf_error_t rc;

    if (!index)
        return BDMF_ERR_PARM;

    /* Get 1st ? */
    if (*index == BDMF_INDEX_UNASSIGNED)
    {
        ch = egress_tm_channel_get_first(mo);
        qi->queue_id = BDMF_INDEX_UNASSIGNED;
    }
    else
    {
        ch = egress_tm_channel_get(tm->dir, qi->channel);
    }

    while (ch)
    {
        /* Find 1st/next queue on this channel */
        qi->channel = ch->channel_id;
        rc = egress_tm_channel_get_next_queue(mo, ch, qi->queue_id, &qi->queue_id);
        if (rc)
            return rc;

        if (qi->queue_id != BDMF_INDEX_UNASSIGNED)
            break;
        /* Done with this channel. Find the next one */
        ch = egress_tm_channel_get_next(mo, ch);
    }

    return ch ? 0 : BDMF_ERR_NO_MORE;
}

/* Scheduler level enum values */
bdmf_attr_enum_table_t egress_tm_level_enum_table = {
    .type_name = "rdpa_tm_level_type", .help = "Egress tm level",
    .values = {
        {"queue", rdpa_tm_level_queue},
        {"egress_tm", rdpa_tm_level_egress_tm},
        {NULL, 0}
    }
};

/* Scheduling type enum values */
bdmf_attr_enum_table_t tm_sched_mode_enum_table = {
    .type_name = "rdpa_tm_sched_mode", .help = "Scheduling mode",
    .values = {
        {"disable",     rdpa_tm_sched_disabled},
        {"sp",          rdpa_tm_sched_sp},
        {"wrr",         rdpa_tm_sched_wrr},
#ifdef XRDP
        {"sp_wrr",      rdpa_tm_sched_sp_wrr},
#endif
        {NULL, 0}
    }
};

/* Drop policy enum values */
bdmf_attr_enum_table_t tm_drop_policy_enum_table = {
    .type_name = "rdpa_tm_drop_alg", .help = "Drop policy algorithm type",
    .values = {
        {"dt", rdpa_tm_drop_alg_dt},
        {"red", rdpa_tm_drop_alg_red},
        {"wred", rdpa_tm_drop_alg_wred},
        {"flow", rdpa_tm_drop_alg_reserved},
        {NULL, 0}
    }
};

/* Overall rate limiter priority enum values */
bdmf_attr_enum_table_t orl_prty_enum_table = {
    .type_name = "rdpa_tm_orl_prty", .help = "Priority for overall rate limiter",
    .values = {
        {"low", rdpa_tm_orl_prty_low},
        {"high", rdpa_tm_orl_prty_high},
        {NULL, 0}
    }
};

/* Rate limiter mode enum values */
bdmf_attr_enum_table_t tm_rl_mode_enum_table = {
    .type_name = "rdpa_tm_rl_rate_mode", .help = "Rate limiter mode",
    .values = {
        {"single_rate", rdpa_tm_rl_single_rate},
        {"dual_rate", rdpa_tm_rl_dual_rate},
        {NULL, 0}
    }
};

/* Number of SP elements enum values */
bdmf_attr_enum_table_t tm_num_sp_elem_enum_table = {
    .type_name = "rdpa_tm_num_sp_elem", .help = "Number of SP elements for sp_wrr scheduling mode",
    .values = {
        {"0",   rdpa_tm_num_sp_elem_0},
        {"2",   rdpa_tm_num_sp_elem_2},
        {"4",   rdpa_tm_num_sp_elem_4},
        {"8",   rdpa_tm_num_sp_elem_8},
        {"16",  rdpa_tm_num_sp_elem_16},
        {"32",  rdpa_tm_num_sp_elem_32},
        {NULL,  0}
    }
};

/*  tm_rl_cfg_type aggregate type : rate limiter configuration */
struct bdmf_aggr_type tm_rl_cfg_type = {
    .name = "tm_rl_cfg", .struct_name = "rdpa_tm_rl_cfg_t",
    .help = "Rate Limiter Configuration",
    .fields = (struct bdmf_attr[]) {
        { .name = "af", .help = "AF Rate (bps)", .type = bdmf_attr_number,
            .flags = BDMF_ATTR_UNSIGNED, .size = sizeof(rl_rate_size_t),
            .offset = offsetof(rdpa_tm_rl_cfg_t, af_rate)
        },
        { .name = "be", .help = "BE Rate (bps)", .type = bdmf_attr_number,
            .flags = BDMF_ATTR_UNSIGNED, .size = sizeof(rl_rate_size_t),
            .offset = offsetof(rdpa_tm_rl_cfg_t, be_rate)
        },
        { .name = "burst", .help = "Burst Size (bytes)", .type = bdmf_attr_number,
            .flags = BDMF_ATTR_UNSIGNED | BDMF_ATTR_DEPRECATED, .size = sizeof(rl_rate_size_t),
            .offset = offsetof(rdpa_tm_rl_cfg_t, burst_size)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(tm_rl_cfg_type);

/*  tm_prio_class_cfg_type aggregate type : wred priority class thresholds */
struct bdmf_aggr_type tm_prio_class_cfg_type = {
    .name = "tm_prio_class_cfg", .struct_name = "rdpa_tm_priority_class_t",
    .help = "priority class Configuration",
    .fields = (struct bdmf_attr[]) {
        { .name = "min_thresh", .help = "min threshold", .type = bdmf_attr_number,
            .flags = BDMF_ATTR_UNSIGNED, .size = sizeof(uint32_t),
            .offset = offsetof(rdpa_tm_priority_class_t, min_threshold)
        },
        { .name = "max_thresh", .help = "max threshold", .type = bdmf_attr_number,
            .flags = BDMF_ATTR_UNSIGNED, .size = sizeof(uint32_t),
            .offset = offsetof(rdpa_tm_priority_class_t, max_threshold)
        },
        { .name = "max_drop_probability", .help = "max drop probability", .type = bdmf_attr_number,
            .flags = BDMF_ATTR_UNSIGNED, .size = sizeof(uint32_t),
            .offset = offsetof(rdpa_tm_priority_class_t, max_drop_probability),
            .max_val = RDPA_WRED_MAX_DROP_PROBABILITY
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(tm_prio_class_cfg_type);


/* tm_queue_cfg aggregate type : queue configuration */
struct bdmf_aggr_type tm_queue_cfg_type = {
    .name = "tm_queue_cfg", .struct_name = "rdpa_tm_queue_cfg_t",
    .help = "Egress Queue Configuration",
    .fields = (struct bdmf_attr[]) {
        { .name = "queue_id", .help = "Queue id", .type = bdmf_attr_number,
            .size = sizeof(uint32_t),
            .offset = offsetof(rdpa_tm_queue_cfg_t, queue_id)
        },
        { .name = "drop_threshold", .help = "Drop Threshold (queue size)",
            .type = bdmf_attr_number, .flags = BDMF_ATTR_UNSIGNED,
            .size = sizeof(uint32_t),
            .offset = offsetof(rdpa_tm_queue_cfg_t, drop_threshold)
        },
        { .name = "weight", .help = "Weight for WRR scheduling",
            .type = bdmf_attr_number, .size = sizeof(uint32_t),
            .offset = offsetof(rdpa_tm_queue_cfg_t, weight),
            .min_val = RDPA_WEIGHT_UNASSIGNED, .max_val = RDPA_MAX_WEIGHT,
        },
        { .name = "drop_alg", .help = "Drop policy",
            .type = bdmf_attr_enum, .ts.enum_table = &tm_drop_policy_enum_table,
            .size = sizeof(rdpa_tm_drop_alg),
            .offset = offsetof(rdpa_tm_queue_cfg_t, drop_alg)
        },
        { .name = "high_class", .help = "WRED high Thresholds",
            .type = bdmf_attr_aggregate, .ts.aggr_type_name = "tm_prio_class_cfg",
            .flags = BDMF_ATTR_UNSIGNED,
            .offset = offsetof(rdpa_tm_queue_cfg_t, high_class)
        },
        { .name = "low_class", .help = "WRED low Thresholds",
            .type = bdmf_attr_aggregate, .ts.aggr_type_name = "tm_prio_class_cfg",
            .flags = BDMF_ATTR_UNSIGNED,
            .offset = offsetof(rdpa_tm_queue_cfg_t, low_class)
        },
        { .name = "reserved_packet_buffers", .help = "Top priority packet buffer number. 0 - no reservation",
            .type = bdmf_attr_number, .flags = BDMF_ATTR_UNSIGNED,
            .size = sizeof(uint16_t),
            .offset = offsetof(rdpa_tm_queue_cfg_t, reserved_packet_buffers)
        },
        { .name = "rl", .help = "Rate Configuration",
            .type = bdmf_attr_aggregate, .ts.aggr_type_name = "tm_rl_cfg",
            .offset = offsetof(rdpa_tm_queue_cfg_t, rl_cfg)
        },
        { .name = "priority_mask_0", .help = "Priority Mask cover TC value 0 to 31",
            .type = bdmf_attr_number, .size = sizeof(uint32_t),
            .offset = offsetof(rdpa_tm_queue_cfg_t, priority_mask_0),
        },
        { .name = "priority_mask_1", .help = "Priority Mask cover TC value 32 to 63",
            .type = bdmf_attr_number, .size = sizeof(uint32_t),
            .offset = offsetof(rdpa_tm_queue_cfg_t, priority_mask_1),
        },
        { .name = "stat_enable", .help = "Enable queue statistics",
            .type = bdmf_attr_boolean,
            .size = sizeof(bdmf_boolean),
            .offset = offsetof(rdpa_tm_queue_cfg_t, stat_enable)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(tm_queue_cfg_type);

/* tm_queue_index aggregate type : queue index - for flush and queue_stat attributes */
struct bdmf_aggr_type tm_queue_index_type = {
    .name = "tm_queue_index", .struct_name = "rdpa_tm_queue_index_t",
    .help = "Egress Queue Index",
    .fields = (struct bdmf_attr[]) {
        { .name = "channel", .help = "Channel selector. Object name or \"all\"",
            .type = bdmf_attr_number, .size = sizeof(bdmf_index),
            .offset = offsetof(rdpa_tm_queue_index_t, channel),
            .val_to_s = egress_tm_channel_val_to_s,
            .s_to_val = egress_tm_channel_s_to_val
        },
        { .name = "queue_id", .help = "Queue id", .type = bdmf_attr_number,
            .size = sizeof(uint32_t),
            .offset = offsetof(rdpa_tm_queue_index_t, queue_id)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(tm_queue_index_type);

/* service_queue_stat aggregate type */
struct bdmf_aggr_type service_queue_cfg_type = {
    .name = "service_queue_cfg", .struct_name = "rdpa_tm_service_queue_t",
    .help = "service Queue statistic",
    .fields = (struct bdmf_attr[]) {
        { .name = "enable", .help = "Enable service queue",
            .type = bdmf_attr_boolean, .size = sizeof(bdmf_boolean),
            .offset = offsetof(rdpa_tm_service_queue_t, enable),
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(service_queue_cfg_type);

/* rdpa_tm_queue aggregate type */
struct bdmf_aggr_type rdpa_tm_queue_location_type = {
    .name = "rdpa_tm_queue_location", .struct_name = "rdpa_tm_queue_location_t",
    .help = "Queue index and its owner",
    .fields = (struct bdmf_attr[]) {
        { .name = "queue_idx", .help = "Queue Index", .type = bdmf_attr_number,
            .size = sizeof(int),
            .offset = offsetof(rdpa_tm_queue_location_t, queue_idx),
        },
        { .name = "queue_tm", .help = "Queue owner", .type = bdmf_attr_object,
            .size = sizeof(bdmf_object_handle),
            .offset = offsetof(rdpa_tm_queue_location_t, queue_tm),
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(rdpa_tm_queue_location_type);

/* Object attribute descriptors */
static struct bdmf_attr egress_tm_attrs[] = {
    { .name = "dir", .help = "Traffic Direction",
        .type = bdmf_attr_enum, .ts.enum_table = &rdpa_traffic_dir_enum_table,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG |
            BDMF_ATTR_KEY | BDMF_ATTR_MANDATORY,
        .size = sizeof(rdpa_traffic_dir), .offset = offsetof(tm_drv_priv_t, dir)
    },
    { .name = "wan_type", .help = "WAN type for US",
        .type = bdmf_attr_enum, .ts.enum_table = &rdpa_wan_type_enum_table,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_NO_AUTO_GEN,
        .size = sizeof(rdpa_wan_type), .offset = offsetof(tm_drv_priv_t, wan_type)
    },
    { .name = "index", .help = "Egress-TM Index", .type = bdmf_attr_number,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_CONFIG | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_KEY,
        .size = sizeof(bdmf_index), .offset = offsetof(tm_drv_priv_t, index)
    },
    { .name = "level", .help = "Egress-TM Next Level",
        .type = bdmf_attr_enum, .ts.enum_table = &egress_tm_level_enum_table,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG,
        .size = sizeof(rdpa_tm_level_type), .offset = offsetof(tm_drv_priv_t, level)
    },
    { .name = "mode", .help = "Scheduler Operating Mode",
        .type = bdmf_attr_enum, .ts.enum_table = &tm_sched_mode_enum_table,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG,
        .size = sizeof(rdpa_tm_sched_mode), .offset = offsetof(tm_drv_priv_t, mode)
    },
    { .name = "overall_rl", .help = "Overall Rate Limiter",
        .type = bdmf_attr_boolean,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG,
        .size = sizeof(bdmf_boolean), .offset = offsetof(tm_drv_priv_t, overall_rl)
    },
    { .name = "service_queue", .help = "Service Queue Parameters Configuration",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "service_queue_cfg",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG,
        .offset = offsetof(tm_drv_priv_t, service_q)
    },
    { .name = "rl", .help = "Rate Configuration",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "tm_rl_cfg",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .offset = offsetof(tm_drv_priv_t, rl_cfg), .write = egress_tm_attr_rl_write
    },
    { .name = "rl_rate_mode", .help = "Subsidiary Rate Limiter Rate Mode",
        .type = bdmf_attr_enum, .ts.enum_table = &tm_rl_mode_enum_table,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG,
        .size = sizeof(rdpa_tm_rl_rate_mode), .offset = offsetof(tm_drv_priv_t, rl_rate_mode),
        .read = egress_tm_attr_rl_rate_mode_read, .write = egress_tm_attr_rl_rate_mode_write
    },
    { .name = "num_queues", .help = "Number of Queues",
        .type = bdmf_attr_number, .size = sizeof(uint8_t), .data_type_name = "uint8_t",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG | BDMF_ATTR_UNSIGNED,
        .min_val = 1, .max_val = RDPA_MAX_EGRESS_QUEUES,
        .read = egress_tm_attr_num_queues_read, .write = egress_tm_attr_num_queues_write,
    },
    { .name = "num_sp_elements", .help = "Number of SP Scheduling Elements for SP_WRR Scheduling Mode",
        .type = bdmf_attr_enum, .ts.enum_table = &tm_num_sp_elem_enum_table,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .size = sizeof(rdpa_tm_num_sp_elem),
        .read = egress_tm_attr_num_sp_elements_read, .write = egress_tm_attr_num_sp_elements_write,
    },
    { .name = "queue_cfg", .help = "Queue Parameters Configuration",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "tm_queue_cfg",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .array_size = RDPA_MAX_EGRESS_QUEUES,
        .read = egress_tm_attr_queue_cfg_read, .write = egress_tm_attr_queue_cfg_write,
        .del = egress_tm_attr_queue_cfg_delete, .val_to_s = egress_tm_queue_cfg_val_to_s,
    },
    { .name = "queue_statistics", .help = "Dropped Service Queue Statistics",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "rdpa_stat_1way",
        .index_type = bdmf_attr_aggregate, .index_ts.aggr_type_name = "tm_queue_index",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_STAT | BDMF_ATTR_DEPRECATED,
        .array_size = RDPA_MAX_EGRESS_QUEUES, .get_next = egress_tm_attr_deprecated_get_next,
        .read = egress_tm_attr_deprecated_read,
    },
    { .name = "queue_stat", .help = "Retrieve Egress Queue Statistics",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "rdpa_stat_1way",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_STAT | BDMF_ATTR_UNSIGNED ,
        .array_size = RDPA_MAX_EGRESS_QUEUES,
        .index_type = bdmf_attr_aggregate, .index_ts.aggr_type_name = "tm_queue_index",
        .get_next = egress_tm_queue_channel_get_next,
        .read = egress_tm_attr_queues_stat_read, .write = egress_tm_attr_queue_stat_write,
    },
    { .name = "queue_occupancy", .help = "Retrieve Egress Queue Occupancy",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "rdpa_stat",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_STAT | BDMF_ATTR_UNSIGNED,
        .array_size = RDPA_MAX_EGRESS_QUEUES,
        .index_type = bdmf_attr_aggregate, .index_ts.aggr_type_name = "tm_queue_index",
        .get_next = egress_tm_queue_channel_get_next,
        .read = egress_tm_attr_queue_occupancy_read,
    },
    { .name = "subsidiary", .help = "Next Level Egress-TM",
        .type = bdmf_attr_object, .size = sizeof(bdmf_object_handle),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .ts.ref_type_name = "egress_tm", .array_size = RDPA_TM_MAX_SCHED_ELEMENTS,
        .read = egress_tm_attr_sub_tm_read, .write = egress_tm_attr_sub_tm_write,
        .find = egress_tm_attr_sub_tm_find
    },
    { .name = "weight", .help = "Weight for WRR scheduling (0 for unset)", .type = bdmf_attr_number,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG | BDMF_ATTR_UNSIGNED,
        .size = sizeof(uint32_t), .offset = offsetof(tm_drv_priv_t, weight),
        .min_val = RDPA_WEIGHT_UNASSIGNED, .max_val = RDPA_MAX_WEIGHT,
        .write = egress_tm_attr_weight_write
    },
    { .name = "queue_location", .help = "Get queue location by qid", .type = bdmf_attr_aggregate,
        .ts.aggr_type_name = "rdpa_tm_queue_location",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_NO_RANGE_CHECK | BDMF_ATTR_DEPRECATED,
        .array_size = RDPA_MAX_EGRESS_QUEUES,
        .read = egress_tm_location_read,
    },
    BDMF_ATTR_LAST
};

static int egress_tm_drv_init(struct bdmf_type *drv);
static void egress_tm_drv_exit(struct bdmf_type *drv);

struct bdmf_type egress_tm_drv = {
    .name = "egress_tm",
    .parent = "system",
    .description = "Hierarchical Traffic Scheduler",
    .drv_init = egress_tm_drv_init,
    .drv_exit = egress_tm_drv_exit,
    .pre_init = egress_tm_pre_init,
    .post_init = egress_tm_post_init,
    .link_up = egress_tm_link,
    .link_down = egress_tm_link,
    .unlink_up = egress_tm_unlink,
    .unlink_down = egress_tm_unlink,
    .destroy = egress_tm_destroy,
    .extra_size = sizeof(tm_drv_priv_t),
    .aattr = egress_tm_attrs,
    .max_objs = RDPA_TM_MAX_US_SCHED+RDPA_TM_MAX_DS_SCHED,
    .flags = BDMF_DRV_FLAG_MUXUP | BDMF_DRV_FLAG_MUXDOWN
};
DECLARE_BDMF_TYPE(rdpa_egress_tm, egress_tm_drv);

extern int (*f_rdpa_egress_tm_queue_exists)(rdpa_if port, uint32_t queue_id);

/* Init/exit module. Cater for GPL layer */
int rdpa_egress_tm_check_queue(rdpa_if port, uint32_t queue_id)
{
#if defined(BCM_DSL_RDP)
    if (port == rdpa_wan_type_to_if(rdpa_wan_gbe)) /* Ethernet WAN */
    {
        int channel = 0, rc_id, priority, tc;
        return _rdpa_egress_tm_wan_flow_queue_to_rdd(port, GBE_WAN_FLOW_ID, queue_id, &channel, &rc_id, &priority, &tc);
    }
    else if (port >= rdpa_if_lan0 && port <= rdpa_if_lan_max) /* Ethernet LAN */
    {
        int rc_id, priority, tc;
        return _rdpa_egress_tm_lan_port_queue_to_rdd_tc_check(port, queue_id, &rc_id, &priority, &tc);
    }
    /* No support for DSL WAN */
#endif
    return BDMF_ERR_NOENT;
}

/* Init/exit module. Cater for GPL layer */
static int egress_tm_drv_init(struct bdmf_type *drv)
{
    int i;
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_egress_tm_drv = rdpa_egress_tm_drv;
    f_rdpa_egress_tm_get = rdpa_egress_tm_get;
    f_rdpa_egress_tm_queue_exists = rdpa_egress_tm_check_queue;
#endif
    for (i = 0; i < TM_QUEUE_HASH_SIZE; i++)
        SLIST_INIT(&tm_queue_hash[i]);
    for (i = 0; i < RDPA_MAX_US_CHANNELS; i++)
    {
        STAILQ_INIT(&us_channels[i].qtm_ctls);
        us_channels[i].channel_id = i;
        us_channels[i].dir = rdpa_dir_us;
    }
    for (i = 0; i < RDPA_MAX_DS_CHANNELS; i++)
    {
        STAILQ_INIT(&ds_channels[i].qtm_ctls);
        ds_channels[i].channel_id = i;
        ds_channels[i].dir = rdpa_dir_ds;
    }
    for (i = 0; i < RDPA_MAX_QUEUE_COUNTERS; i++)
    {
        queue_counters[rdpa_dir_ds][i] = i + RDPA_MAX_QUEUE_COUNTERS;
        queue_counters[rdpa_dir_us][i] = i + RDPA_MAX_QUEUE_COUNTERS;
    }

    memset(us_hash_lookup, 0x0, sizeof(us_hash_lookup));
    memset(ds_hash_lookup, 0x0, sizeof(ds_hash_lookup));

    egress_tm_drv_init_ex();

    return 0;
}

static void egress_tm_drv_exit(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_egress_tm_drv = NULL;
    f_rdpa_egress_tm_get = NULL;
    f_rdpa_egress_tm_queue_exists = NULL;
#endif
}

static int rdpa_egress_tm_us_orl_obj_get(bdmf_object_handle *_obj_)
{
    if (!us_overall_rl_obj || us_overall_rl_obj->state == bdmf_state_deleted)
        return BDMF_ERR_NOENT;
    bdmf_get(us_overall_rl_obj);
    *_obj_ = us_overall_rl_obj;
    return 0;
}

/***************************************************************************
 * Functions declared in auto-generated header
 **************************************************************************/

/** Get egress_tm object by key */
int rdpa_egress_tm_get(const rdpa_egress_tm_key_t *_key_, bdmf_object_handle *_obj_)
{
    struct bdmf_object **tm_objects;
    int max_subs;

    if ((_key_->dir == rdpa_dir_us) && (_key_->index == OVERALL_RL_TM_INDEX))
        return rdpa_egress_tm_us_orl_obj_get(_obj_);

    tm_objects = (_key_->dir == rdpa_dir_ds) ? ds_tm_objects : us_tm_objects;
    max_subs = (_key_->dir == rdpa_dir_ds) ? RDPA_TM_MAX_DS_SCHED : RDPA_TM_MAX_US_SCHED;
    return rdpa_obj_get(tm_objects, max_subs, _key_->index, _obj_);
}

/*************************************************************************
 * Functions in use by other drivers internally
 ************************************************************************/


/* Bind/unbind channel.
 * bind: owner!=NULL, unbind : owner==NULL
 */
int _rdpa_egress_tm_channel_set(bdmf_object_handle tm_obj, bdmf_object_handle owner, rdpa_wan_type wan_type, int16_t channel_id)
{
    tm_drv_priv_t *tm;
    tm_channel_t *channel;
    bdmf_error_t rc = 0;
    bdmf_boolean enable;
    bdmf_object_handle old_tm_obj;

    if (!tm_obj)
        return BDMF_ERR_PARM;
    tm = (tm_drv_priv_t *)bdmf_obj_data(tm_obj);

    /* This operation is only allowed for top-level TM */
#ifdef EPON
    if (tm->upper_level_tm && ((tm->dir != rdpa_dir_us) || !is_rdpa_epon_ctc_or_cuc_mode()))
#else
    if (tm->upper_level_tm)
#endif
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, tm_obj, "Can't set channel on subsidiary scheduler\n");

    enable = tm->enable_cfg.enable;

    /* Find channel context. If it is already bound to another egress_tm - it is an error */
    channel = egress_tm_channel_get(tm->dir, channel_id);
    if (!channel)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, tm_obj, "channel_id %d is invalid\n", channel_id);

    old_tm_obj = channel->egress_tm;
    /* Unbind the old egress_tm object from channel if necessary */
    if (old_tm_obj && old_tm_obj != tm_obj)
    {
        /* Unbind old egress_tm if bind new.
         * Report error if un-binding wrong egress_tm from the channel
         */
        if (!owner)
        {
            BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, tm_obj, "channel_id %d is bound to %s\n",
                channel_id, channel->egress_tm->name);
        }

        _rdpa_egress_tm_channel_set(old_tm_obj, NULL, wan_type, channel_id);
    }

    /* Unbind channel ? */
    if (!owner)
    {
        /* Do nothing if not bound */
        if (!channel->egress_tm)
            return 0;
        if (channel->orl_tm)
            egress_tm_orl_set_on_channel(tm_obj, channel, NULL, tm->orl_prty);
        if (enable)
            egress_tm_enable_set_on_channel(tm_obj, channel, 0, 0);
        egress_tm_rdd_resources_free_on_channel(tm_obj, channel);
        channel->egress_tm = NULL;
        channel->owner = NULL;
        tm->wan_type = rdpa_wan_none;
    }
    else
    {
        /* Assign new channel */
        tm->wan_type = wan_type;
        channel->egress_tm = tm_obj;
        channel->owner = owner;
        rc = egress_tm_rdd_resources_alloc_on_channel(tm_obj, channel, -1);
        if (enable)
            rc = rc ? rc : egress_tm_enable_set_on_channel(tm_obj, channel, enable, 0);
        if (tm->dir == rdpa_dir_us && us_overall_rl_obj && egress_tm_is_channel_rl(us_overall_rl_obj, owner))
            rc = rc ? rc : egress_tm_orl_set_on_channel(tm_obj, channel, us_overall_rl_obj, tm->orl_prty);

        /* Try to roll-back */
        if (rc)
        {
            if (enable)
                egress_tm_enable_set_on_channel(tm_obj, channel, 0, 0);
            egress_tm_rdd_resources_free_on_channel(tm_obj, channel);
            channel->egress_tm = NULL;
            channel->owner = NULL;
            if (old_tm_obj)
            {
                int rc1;
                rc1 = _rdpa_egress_tm_channel_set(old_tm_obj, owner, wan_type, channel_id);
                rc1 = rc1 ? rc1 : egress_tm_enable_set_on_channel(old_tm_obj, channel, enable, 0);
                if (rc1)
                {
                    BDMF_TRACE_RET_OBJ(rc1, tm_obj, "Roll-back binding of %s to %s failed\n",
                        owner->name, old_tm_obj->name);
                }
            }
        }
    }

    /* Unset enable if there are no channels on egress_tm */
    if (!egress_tm_channel_get_first(tm_obj))
        egress_tm_enable_store(tm, 0);

    return rc;
}

/* Bind/unbind channel group
 * bind: owner!=NULL, unbind : owner==NULL
 */
int _rdpa_egress_tm_channel_group_set(bdmf_object_handle tm_obj, bdmf_object_handle owner, rdpa_wan_type wan_type,
    int group_id, int num_channels, const int16_t *channels)
{
    tm_drv_priv_t *tm;
    int i;
    int rc = 0;

    if (!tm_obj)
        return BDMF_ERR_PARM;
    tm = (tm_drv_priv_t *)bdmf_obj_data(tm_obj);

    if (tm->upper_level_tm)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, tm_obj, "Can't set channel group for subsidiary scheduler\n");

    if (!num_channels || (unsigned)num_channels > RDPA_MAX_WAN_SUBCHANNELS)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, tm_obj, "num_channels %d is out of range 1..%d\n",
            num_channels, RDPA_MAX_WAN_SUBCHANNELS);
    }

    if (num_channels > 1 && tm->level != rdpa_tm_level_egress_tm)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, tm_obj, "Can't assign multiple channels to queue-level egress_tm\n");

    /* Make sure that there are no subsidiaries with index >= num_channels */
    for (i = num_channels; i < RDPA_TM_MAX_SCHED_ELEMENTS; i++)
    {
        if (tm->sub_tms[i])
        {
            BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, tm_obj,
                "Subsidiary index %d of %s is outside sub-channel range 0..%d\n\n",
                i, tm->sub_tms[i]->name, num_channels-1);
        }
    }

    if (owner)
    {
        if (tm->channel_group_owner && tm->channel_group_owner != owner)
        {
            BDMF_TRACE_RET_OBJ(BDMF_ERR_ALREADY, tm_obj, "egress_tm is already owned by channel group %s\n",
                tm->channel_group_owner->name);
        }

        /* Go over channels that were already assigned and release those
         * that are not assigned anymore
         */
        if (tm->num_channels > RDPA_MAX_WAN_SUBCHANNELS)
        {
            BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, tm_obj, "Num of channels (%d) is over of max allowed (%d)\n",
                tm->num_channels, RDPA_MAX_WAN_SUBCHANNELS);
        }
        for (i = num_channels; i < tm->num_channels; i++)
        {
            if (tm->level == rdpa_tm_level_egress_tm)
            {
                if (tm->sub_tms[i])
                    _rdpa_egress_tm_channel_set(tm->sub_tms[i], NULL, wan_type, channels[i]);
            }
            else
            {
                _rdpa_egress_tm_channel_set(tm_obj, NULL, wan_type, channels[i]);
            }
            tm->channels[i] = BDMF_INDEX_UNASSIGNED;
        }

        tm->num_channels = num_channels;
        tm->channel_group = group_id;
        tm->channel_group_owner = owner;
    }

    /* Go over all subsidiaries and clear upper_level_tm in order for the rest
     * of egress_tm logic to consider each subsidiary as root of hierarchy
     */
    for (i = 0; i < num_channels && !rc; i++)
    {
        if (tm->channels[i] != channels[i])
        {
            tm->channels[i] = channels[i];
            if (tm->level == rdpa_tm_level_egress_tm)
            {
                if (tm->sub_tms[i])
                {
                    tm_drv_priv_t *ss = (tm_drv_priv_t *)bdmf_obj_data(tm->sub_tms[i]);
                    ss->upper_level_tm = NULL;
                    rc = _rdpa_egress_tm_channel_set(tm->sub_tms[i], owner, wan_type, channels[i]);
                }
            }
            else
            {
                rc = _rdpa_egress_tm_channel_set(tm_obj, owner, wan_type, channels[i]);
            }
        }
    }

    /* roll-back if error */
    if (owner && rc)
    {
        while (--i >= 0)
        {
            if (tm->sub_tms[i])
                _rdpa_egress_tm_channel_set(tm->sub_tms[i], NULL, wan_type, channels[i]);
        }
    }

    /* Clear stored channels if "clear" command or error */
    if (!owner || rc)
    {
        tm_channel_t *ch = NULL;

        /* Special handling for channel-group egress_tm (LLID). */
        while ((ch = egress_tm_channel_get_next(tm_obj, ch)) && !rc)
        {
            egress_tm_enable_set(ch->egress_tm, 0, 0);
        }

        for (i = 0; i < num_channels && !rc; i++)
        {
            if (tm->sub_tms[i])
            {
                tm_drv_priv_t *ss = (tm_drv_priv_t *)bdmf_obj_data(tm->sub_tms[i]);
                ss->upper_level_tm = tm_obj;
            }
            tm->channels[i] = BDMF_INDEX_UNASSIGNED;
            tm->channel_group = BDMF_INDEX_UNASSIGNED;
        }
        tm->num_channels = 0;
        tm->channel_group_owner = NULL;
    }

    return rc;
}


int _rdpa_egress_tm_enable_set(bdmf_object_handle tm_obj, bdmf_boolean enable, bdmf_boolean flush)
{
    tm_drv_priv_t *tm;
    int rc = 0;

    if (!tm_obj)
        return BDMF_ERR_PARM;
    tm = (tm_drv_priv_t *)bdmf_obj_data(tm_obj);

    /* This operation is only allowed for top-level TM */
    if (tm->upper_level_tm)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, tm_obj, "Can't enable/disable subsidiary scheduler\n");

    if (tm_obj->state != bdmf_state_init)
        rc = egress_tm_enable_set(tm_obj, enable, flush);
    else
        tm->enable_cfg.enable = enable;
    return rc;
}

int _rdpa_egress_tm_orl_prty_set(bdmf_object_handle tm_obj, rdpa_tm_orl_prty orl_prty)
{
    tm_drv_priv_t *tm;
    int rc = 0;

    if (!tm_obj)
        return BDMF_ERR_PARM;
    tm = (tm_drv_priv_t *)bdmf_obj_data(tm_obj);

    /* This operation is only allowed for top-level TM */
    if (tm->upper_level_tm)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, tm_obj, "Can't set orl_prty on subsidiary scheduler\n");

    if (tm_obj->state != bdmf_state_init && us_overall_rl_obj)
        rc = egress_tm_orl_set(tm_obj, orl_prty);
    if (!rc)
        tm->orl_prty = orl_prty;

    return rc;
}

int  _rdpa_egress_tm_channel_queue_to_rdd(rdpa_traffic_dir dir, int channel, uint32_t queue_id, int *rc_id, int *queue)
{
    return _rdpa_egress_tm_dir_channel_queue_to_rdd(dir, channel, queue_id, rc_id, queue);
}


/* Get queue_id by port / rate controller and queue index (priority) */
static int __rdpa_egress_tm_queue_id_by_dir_channel_rc_queue(rdpa_traffic_dir dir, int channel_id, int rc_id, int queue,
    bdmf_object_handle *mo, uint32_t *queue_id)
{
    tm_channel_t *channel = egress_tm_channel_get(dir, channel_id);
    tm_qtm_ctl_t *qtm_ctl;
    int rc = BDMF_ERR_NOENT;

    if ((unsigned)queue >= RDPA_MAX_EGRESS_QUEUES)
        return BDMF_ERR_PARM;

    if (!channel)
        return BDMF_ERR_NOENT;

    STAILQ_FOREACH(qtm_ctl, &channel->qtm_ctls, list)
    {
        tm_drv_priv_t *tm;
#ifdef XRDP
        if (qtm_ctl->sched_id != rc_id)
#else
        if (qtm_ctl->rc_id != rc_id)
#endif
            continue;
        if (!qtm_ctl->queue_configured[queue])
            break; /* BDMF_ERR_NOENT is set by default */
        /* Found it */
        tm = (tm_drv_priv_t *)bdmf_obj_data(qtm_ctl->egress_tm);
        *mo = qtm_ctl->egress_tm;
        *queue_id = tm->queue_cfg[queue].queue_id;
        rc = 0;
        break;
    }

    return rc;
}

/* Get queue_id by port / rate controller and queue index (priority) */
static int _rdpa_egress_tm_queue_id_by_dir_channel_rc_queue(rdpa_traffic_dir dir,
    int channel_id, int rc_id, int queue, uint32_t *queue_id)
{
    bdmf_object_handle egress_tm;
    return __rdpa_egress_tm_queue_id_by_dir_channel_rc_queue(dir, channel_id, rc_id, queue,
        &egress_tm, queue_id);
}

int _rdpa_egress_tm_queue_id_by_wan_flow_rc_queue(int *wan_flow, int rc_id, int queue, uint32_t *queue_id)
{
    int channel_id;
    int flow = *wan_flow;
    bdmf_object_handle egress_tm = NULL;
    int rc;

#ifdef CONFIG_BCM_TCONT
    if (rdpa_is_gpon_or_xgpon_mode())
    {
        int rc;
        /* Find tcont by GEM. channel is tcont->index */
        rc = rdpa_gem_flow_id_to_tcont_channel_id(flow, &channel_id);
        if (rc)
            return rc;
    }
    else
#endif
    {
        channel_id = flow;
    }
    rc = __rdpa_egress_tm_queue_id_by_dir_channel_rc_queue(rdpa_dir_us, channel_id, rc_id, queue, &egress_tm, queue_id);
    if (egress_tm)
    {
        bdmf_object_handle root = egress_tm_get_root_object(egress_tm);
        tm_drv_priv_t *root_tm = (tm_drv_priv_t *)bdmf_obj_data(root);

        /* Special handling for epon whereas root egress_tm represents channel group */
        if (egress_tm_is_ch_group(root_tm))
            *wan_flow = root_tm->channel_group;
    }
    return rc;
}

int _rdpa_egress_tm_queue_id_by_channel_rc_queue(int channel_id, int rc_id, int queue, uint32_t *queue_id)
{
    return _rdpa_egress_tm_queue_id_by_dir_channel_rc_queue(rdpa_dir_us, channel_id, rc_id, queue, queue_id);
}

int _rdpa_egress_tm_queue_id_by_lan_port_queue(rdpa_if port, int queue, uint32_t *queue_id)
{
    int channel_id = _rdpa_port_channel(port);
    int rc_id = RDD_RATE_LIMITER_PORT_0 + channel_id;

    return _rdpa_egress_tm_queue_id_by_dir_channel_rc_queue(rdpa_dir_ds, channel_id, rc_id, queue, queue_id);
}

int _rdpa_egress_tm_channel_queue_enable_set(bdmf_object_handle tm_obj, int channel_id, uint32_t queue_index, bdmf_boolean enable)
{
    tm_drv_priv_t *tm;
    tm_channel_t *channel;
    int rc = 0;

    if (!tm_obj)
        return BDMF_ERR_PARM;

    tm = (tm_drv_priv_t *)bdmf_obj_data(tm_obj);

    /* Find channel context */
    channel = egress_tm_channel_get(tm->dir, channel_id);
    if (!channel)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, tm_obj, "channel_id %d is invalid\n", channel_id);

    /* This operation is only allowed for queue-level TM */
    if (tm->level != rdpa_tm_level_queue)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, tm_obj, "Can't enable/disable subsidiary scheduler\n");

    /* Configure specified queue if exist */
    if (tm->queue_configured[queue_index])
    {
        if (tm->queue_cfg[queue_index].drop_threshold)
            rc = egress_tm_queue_cfg_on_channel(tm_obj, channel, queue_index, &tm->queue_cfg[queue_index], enable);
    }

    return rc;
}

int _rdpa_egress_tm_channel_enable_set(bdmf_object_handle tm_obj, int channel_id, bdmf_boolean enable)
{
    tm_drv_priv_t *tm;
    tm_channel_t *channel;
    int rc = 0;

    if (!tm_obj)
        return BDMF_ERR_PARM;

    tm = (tm_drv_priv_t *)bdmf_obj_data(tm_obj);

    /* Find channel context */
    channel = egress_tm_channel_get(tm->dir, channel_id);
    if (!channel)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, tm_obj, "channel_id %d is invalid\n", channel_id);

    /* Enable/disable egress_tm channel queues */
    rc = egress_tm_enable_set_on_channel(tm_obj, channel, enable, 0);

    return rc;
}

#ifdef XRDP
static int _rdpa_egress_tm_queue_id_by_dir_channel_qm_queue(rdpa_traffic_dir dir,
    int channel_id, int qm_queue, bdmf_object_handle *mo, uint32_t *queue_id)
{
    tm_channel_t *channel = egress_tm_channel_get(dir, channel_id);
    tm_qtm_ctl_t *qtm_ctl;
    int queue;

    if (!channel)
        return BDMF_ERR_NOENT;

    STAILQ_FOREACH(qtm_ctl, &channel->qtm_ctls, list)
    {
        tm_drv_priv_t *tm;
        for (queue = 0; queue < RDPA_MAX_EGRESS_QUEUES; queue++)
        {
            if (!qtm_ctl->queue_configured[queue])
                continue;
            if (qtm_ctl->hash_entry[queue].rdp_queue_index == qm_queue)
            {
                /* Found it */
                tm = (tm_drv_priv_t *)bdmf_obj_data(qtm_ctl->egress_tm);
                *mo = qtm_ctl->egress_tm;
                *queue_id = tm->queue_cfg[queue].queue_id;
                return 0;
            }
        }
    }

    return BDMF_ERR_NOENT;
}

/* Get RDPA queue_id by wan_flow, wan_port and QM queue */
/* Support Multi-WAN */
int _rdpa_egress_tm_queue_id_by_wan_flow_port_qm_queue(int *wan_flow, rdpa_if port, int qm_queue, uint32_t *queue_id)
{
    bdmf_object_handle egress_tm = NULL;
    rdpa_wan_type wan_type = rdpa_wan_if_to_wan_type(port);
    int channel_id, rc;
    int flow = *wan_flow;

    if (wan_type == rdpa_wan_gbe)
    {
        channel_id = _rdpa_port_channel_no_lock(port);
        if (channel_id == -1)
            return BDMF_ERR_NOENT;
    }
#if defined(BCM63158)
    else if (wan_type == rdpa_wan_dsl)
    {
        /* Find xtmchannel by xtmflow. */
        rc = rdpa_xtm_flow_id_to_channel_id(flow, &channel_id);
        if (rc)
            return rc;
    }
#endif /* defined(BCM63158) */
#if defined(CONFIG_BCM_TCONT)
    else if ((wan_type == rdpa_wan_gpon) || (wan_type == rdpa_wan_xgpon))
    {
        /* Find tcont by GEM/wan_flow. channel is tcont->channel */
        rc = rdpa_gem_flow_id_to_tcont_channel_id(flow, &channel_id);
        if (rc)
            return rc;
    }
#endif /* defined(CONFIG_BCM_TCONT) */
    else
    {
        channel_id = flow;
    }

    rc = _rdpa_egress_tm_queue_id_by_dir_channel_qm_queue(rdpa_dir_us, channel_id, qm_queue, &egress_tm, queue_id);
    return rc;
}

/* Get RDPA queue_id by wan_flow and QM queue */
int _rdpa_egress_tm_queue_id_by_wan_flow_qm_queue(int *wan_flow, int qm_queue, uint32_t *queue_id)
{
    int channel_id;
    int flow = *wan_flow;
    bdmf_object_handle egress_tm = NULL;
    int rc;

#ifdef CONFIG_BCM_TCONT
    if (rdpa_is_gpon_or_xgpon_mode())
    {
        int rc;
        /* Find tcont by GEM. channel is tcont->index */
        rc = rdpa_gem_flow_id_to_tcont_channel_id(flow, &channel_id);
        if (rc)
            return rc;
    }
    else
#endif
    {
        channel_id = flow;
    }
    rc = _rdpa_egress_tm_queue_id_by_dir_channel_qm_queue(rdpa_dir_us, channel_id, qm_queue, &egress_tm, queue_id);
    if (egress_tm)
    {
        bdmf_object_handle root = egress_tm_get_root_object(egress_tm);
        tm_drv_priv_t *root_tm = (tm_drv_priv_t *)bdmf_obj_data(root);

        /* Special handling for epon whereas root egress_tm represents channel group */
        if (egress_tm_is_ch_group(root_tm))
            *wan_flow = root_tm->channel_group;
    }
    return rc;
}

/* Get RDPA queue_id by lan_port and QM queue */
int _rdpa_egress_tm_queue_id_by_lan_port_qm_queue(rdpa_if port, int qm_queue, uint32_t *queue_id)
{
    bdmf_object_handle egress_tm = NULL;
    int channel_id = _rdpa_port_channel(port);
    return _rdpa_egress_tm_queue_id_by_dir_channel_qm_queue(rdpa_dir_ds, channel_id, qm_queue, &egress_tm, queue_id);
}
#endif /* XRDP */
