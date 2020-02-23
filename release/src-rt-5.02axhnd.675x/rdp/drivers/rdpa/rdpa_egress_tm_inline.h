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

#ifndef _RDPA_EGRESS_TM_INLINE_H_
#define _RDPA_EGRESS_TM_INLINE_H_

#include "rdpa_int.h"
#include "rdpa_port_int.h"
#include "rdpa_egress_tm.h"
#if defined(CONFIG_DSLWAN)
#include "rdpa_xtm.h"
#include "rdpa_xtm_ex.h"
#include "rdpa_ag_xtm.h"
#endif

#define TM_QUEUE_HASH_SIZE (2 * RDPA_MAX_US_RATE_CONTROLLERS * 8)

/* Max number of DS rate controllers */
#define RDPA_MAX_DS_RATE_CONTROLLERS    RDD_RATE_LIMITER_DISABLED

/* Currently in DS there is no channel as such. Rate controller represents
 * channel */
#ifndef G9991
#define RDPA_MAX_DS_CHANNELS            (RDPA_MAX_DS_RATE_CONTROLLERS + 1)
#else
/* XXX: Number of DS channels supported, in G9991 case we need one for each
 * rdpa lan port 0-21 */
#define RDPA_MAX_DS_CHANNELS            30
#endif

#define RDPA_MAX_SERVICE_QUEUES         32

#define drop_alg_is_red(cfg) \
    (cfg->drop_alg == rdpa_tm_drop_alg_red || cfg->drop_alg == rdpa_tm_drop_alg_wred)

typedef struct tm_queue_hash_entry
{
    /* entry key */
    uint32_t dir_channel;                                 /* Direction+channel */
    uint32_t queue_id;                                    /* arbitrary queue_id (RDPA-level identifier) */
    int queue_index;                                      /* queue index in egress_tm */
    int rdp_queue_index;                                  /* QM/RDD queue index */
#ifdef XRDP
    int16_t rc_id;                                        /* rdd rate controller index (XRDP only) */
    int16_t sched_weight;                                 /* queue weight for WRR scheduling */
#endif
    SLIST_ENTRY(tm_queue_hash_entry) list;                /* list of entries sharing the same hash index */
} tm_queue_hash_entry_t;

/* Channel context - 1 per channel */
typedef struct tm_channel
{
    int16_t channel_id;                                   /* Channel index in RDD (wan channel + tcont or lan emac) */
    int16_t rc_id;                                        /* rdd rate controller index */
    channel_attr attr;                                    /* channel attributes (XRDP only) */
#ifdef XRDP
    int16_t sched_id;                                     /* Scheduler id (XRDP only) */
    int16_t sched_mode;                                   /* RDD-level scheduling mode (XRDP only) */
#endif
    rdpa_tm_rl_rate_mode rl_rate_mode;                    /* rate mode */
    uint32_t rc_mask;                                     /* bit mask of all RCs used by egress_tm hierarchy */
    bdmf_boolean res_allocated;                           /* RDD resource assigned */
    bdmf_boolean enable;                                  /* 1=channel enabled */
    rdpa_traffic_dir dir;                                 /* Traffic direction - convenience copy */
    struct bdmf_object *owner;                            /* Channel owner */
    struct bdmf_object *egress_tm;                        /* Egress_tm object bound to the channel */
    struct bdmf_object *orl_tm;                           /* ORL reference */
    STAILQ_HEAD(tm_qtm_ctl_head, tm_qtm_ctl) qtm_ctls;    /* Q-level TM context list head */
} tm_channel_t;

/* DDR queue descriptor for PD offload */
typedef struct {
    uint32_t size;  /* queue max size */
    void *addr;     /* pointer to allocated memory */
} pd_offload_ddr_queue_t;

/* Q-level scheduling context - 1 per channel per queue-level egress_tm object (1 per RC) */
typedef struct tm_qtm_ctl
{
    int16_t rc_id;                                              /* rdd rate controller index */
    rdpa_tm_rl_rate_mode rl_rate_mode;                          /* rate mode (XRDP only) */
    int16_t sched_index_in_upper;                               /* "Slot" index in upper-level scheduler. XRDP only */
#ifdef XRDP
    int16_t sched_id;                                           /* Scheduler id - XRDP only */
    int16_t sched_mode;                                         /* RDD-level scheduling mode (XRDP only) */
#endif
    uint32_t sched_weight;                                      /* Weight in WRR upper-level scheduler. XRDP only */
    bdmf_boolean queue_configured[RDPA_MAX_EGRESS_QUEUES];      /* true for queues that are configured in RDD */
    bdmf_boolean res_allocated;                                 /* RDD resource assigned */
    tm_queue_hash_entry_t hash_entry[RDPA_MAX_EGRESS_QUEUES];   /* per-queue queue hash entry */
    tm_channel_t *channel;                                      /* channel reference */
    rdpa_tm_queue_cfg_t queue_cfg[RDPA_MAX_EGRESS_QUEUES];      /* per-queue queue config reference */
    struct bdmf_object *egress_tm;                              /* Egress_tm object reference */
    STAILQ_ENTRY(tm_qtm_ctl) list;                              /* rate_ctl list pointer - maintained per channel */
    pd_offload_ddr_queue_t wan_tx_ddr_queue[RDPA_MAX_EGRESS_QUEUES];        /* DDR queue descriptor */
    int8_t counter_id[RDPA_MAX_EGRESS_QUEUES];                  /* statistics counter ID */
} tm_qtm_ctl_t;

struct tm_queue_hash_head {
    struct tm_queue_hash_entry *slh_first;	/* first element */
};

extern bdmf_fastlock tm_hash_lock_irq;
extern struct tm_queue_hash_head tm_queue_hash[TM_QUEUE_HASH_SIZE];

#ifndef __KERNEL__
static inline uint32_t rol32(uint32_t n, int bits)
{
    return (n << bits) | (n >> (32 - bits));
}
#endif

/* Make dir_channel given channel and dir */
#if defined(BCM_DSL_RDP)
/* static inline function egress_tm_dir_channel returns wrong value for upstream on BCM63138. */
#define egress_tm_dir_channel(d, c)  ((uint32_t) (((uint32_t) (d) << 16) | ((uint32_t) (c))))
#else
static inline uint32_t egress_tm_dir_channel(rdpa_traffic_dir dir, int16_t channel)
{
    return ((uint32_t)dir << 16) | (uint32_t)channel;
}
#endif
#define egress_tm_get_queue_dir(dir_channel) ((rdpa_traffic_dir)(dir_channel >> 16))
#define egress_tm_get_queue_channel(dir_channel) ((int16_t)(dir_channel & 0xffff))

/* hash function */
static inline int egress_tm_hash_func(uint32_t dir_channel, uint32_t queue_id)
{
    uint32_t hash = 0xdeadbeef;
    dir_channel += hash;
    hash -= rol32(queue_id, 14);
    queue_id ^= dir_channel;
    queue_id -= rol32(dir_channel, 25);
    hash ^= queue_id;
    hash -= rol32(queue_id, 16);
    return hash % TM_QUEUE_HASH_SIZE;
}

/* check if queue id in the limits, in order to prevent memory leak/access crash*/
static inline bdmf_error_t is_queue_id_shortcut_valid(uint32_t queue_id)
{
    return queue_id >= RDPA_TM_MAX_SCHED_ELEMENTS * RDPA_MAX_EGRESS_QUEUES ? 0 : 1;
}

extern tm_queue_hash_entry_t *ds_hash_index_get(uint32_t channel, uint32_t queue_id);
extern tm_queue_hash_entry_t *us_hash_index_get(uint32_t channel, uint32_t queue_id);


/* Find hash entry. Returns hash entry address or NULL if not found */
static inline tm_queue_hash_entry_t *egress_tm_hash_get(uint32_t dir_channel, uint32_t queue_id)
{
    struct tm_queue_hash_entry *entry;
    unsigned long flags;
    int index;

    if (likely(is_queue_id_shortcut_valid(queue_id)))
    {
        uint32_t channel = egress_tm_get_queue_channel(dir_channel);

        if ((egress_tm_get_queue_dir(dir_channel) == rdpa_dir_us) && (channel < RDPA_MAX_US_CHANNELS))
            return us_hash_index_get(channel, queue_id);
        else if ((egress_tm_get_queue_dir(dir_channel) == rdpa_dir_ds) && (channel < RDPA_MAX_DS_CHANNELS))
            return ds_hash_index_get(channel, queue_id);
    }

    index = egress_tm_hash_func(dir_channel, queue_id);
    bdmf_fastlock_lock_irq(&tm_hash_lock_irq, flags);

    SLIST_FOREACH(entry, &tm_queue_hash[index], list)
    {
        if (entry->dir_channel == dir_channel && entry->queue_id == queue_id)
            break;
    }

    bdmf_fastlock_unlock_irq(&tm_hash_lock_irq, flags);

    return entry;
}

/* return egress_tm structure containing hash entry */
static inline tm_qtm_ctl_t *egress_tm_hash_entry_container(tm_queue_hash_entry_t *entry)
{
    return (tm_qtm_ctl_t *)((long)entry - offsetof(tm_qtm_ctl_t, hash_entry[entry->queue_index]));
}

/* Get hash entry by dir, channel */
static inline tm_queue_hash_entry_t *egress_tm_hash_get_by_dir_channel_queue(rdpa_traffic_dir dir, int channel, uint32_t queue_id)
{
    uint32_t dir_channel = egress_tm_dir_channel(dir, channel);
    tm_queue_hash_entry_t *entry = egress_tm_hash_get(dir_channel, queue_id);
    return entry;
}

/* get RDD parameters given direction, channel and queue_id.
 * Returns 0 if OK, BDMF_ERR_NOENT if failure
 * can modify channel. Initial and "real" channel can be different for EPON
 */
static inline int __rdpa_egress_tm_dir_channel_queue_to_rdd(rdpa_traffic_dir dir, int *channel, uint32_t queue_id,
    int *rc_id, int *queue, int *tc)
{
    tm_queue_hash_entry_t *entry = egress_tm_hash_get_by_dir_channel_queue(dir, *channel, queue_id);
    tm_qtm_ctl_t *rate_ctl;
#if defined(BCM_DSL_RDP) || defined(BCM_DSL_XRDP)
    rdpa_tm_queue_cfg_t *queue_cfg;
#endif

    if (!entry)
    {
        if (tc != NULL)
            *tc = 0;
        return BDMF_ERR_NOENT;
    }

    rate_ctl = egress_tm_hash_entry_container(entry);
#ifdef XRDP
    *rc_id = rate_ctl->sched_id;
#else
    *rc_id = rate_ctl->rc_id;
#endif
    *queue = entry->rdp_queue_index;
    *channel = rate_ctl->channel->channel_id;

#if defined(BCM_DSL_RDP) || defined(BCM_DSL_XRDP)
    if (tc != NULL)
    {
        queue_cfg = &rate_ctl->queue_cfg[entry->queue_index];
        if ((*tc < 32) && (queue_cfg->priority_mask_0 & (0x1 << *tc)))
            *tc = 1;
        else if ((*tc >= 32) && (*tc < 64) && (queue_cfg->priority_mask_1 & (0x1 << (*tc - 32))))
            *tc = 1;
        else
            *tc = 0;
    }
#endif

    return 0;
}

/* get RDD parameters given direction, channel and queue_id.
 * Returns 0 if OK, BDMF_ERR_NOENT if failure
 */
static inline int _rdpa_egress_tm_dir_channel_queue_to_rdd(rdpa_traffic_dir dir, int channel, uint32_t queue_id,
    int *rc_id, int *queue)
{
    return __rdpa_egress_tm_dir_channel_queue_to_rdd(dir, &channel, queue_id, rc_id, queue, NULL);
}

static inline int egress_tm_queue_id_info_get(rdpa_traffic_dir dir, int channel, uint32_t queue_id,
    queue_info_t *queue_id_info)
{
    int rc = __rdpa_egress_tm_dir_channel_queue_to_rdd(dir, &channel, queue_id, &queue_id_info->rc_id,
        &queue_id_info->queue, NULL);

    if (!rc)
    {
        queue_id_info->channel = channel;
    }

    return rc;
}

static inline int _rdpa_egress_tm_wan_flow_queue_to_rdd(rdpa_if port, int wan_flow, uint32_t queue_id,
    int *channel, int *rc_id, int *queue, int *tc)
{
    rdpa_wan_type wan_type = rdpa_wan_if_to_wan_type(port);
    int rc = 0;

    if (wan_type == rdpa_wan_gbe)
    {
        *channel = _rdpa_port_channel_no_lock(port);
        rc = BDMF_ERR_NOENT;
        if (*channel == -1)
            return rc;
    }
#if defined(CONFIG_DSLWAN)
    else if (wan_type == rdpa_wan_dsl)
    {
        /* Find xtmchannel by xtmflow. */
        rc = rdpa_xtm_flow_id_to_channel_id(wan_flow, channel);
        if (rc)
            return rc;
    }
#endif /* defined(CONFIG_DSLWAN) */
#if defined(CONFIG_BCM_TCONT)
    else if ((wan_type == rdpa_wan_gpon) || (wan_type == rdpa_wan_xgpon))
    {
        /* Find tcont by GEM/wan_flow. channel is tcont->channel */
        rc = rdpa_gem_flow_id_to_tcont_channel_id(wan_flow, channel);
        if (rc)
            return rc;
    }
#endif /* defined(CONFIG_BCM_TCONT) */
#if defined(BCM_PON) || defined(CONFIG_BCM_PON) || defined(BCM63158)
    else if ((wan_type == rdpa_wan_epon) || (wan_type == rdpa_wan_xepon))
    {
        /* Find channel by wan_flow and queue id. */
        rc = rdpa_llid_queue_id_to_channel_id(wan_flow, queue_id, channel);
        if (rc)
            return rc;
    }
#endif /* defined(BCM_PON) || defined(CONFIG_BCM_PON) || defined(BCM63158) */
    else
    {
        *channel = wan_flow;
    }
    return __rdpa_egress_tm_dir_channel_queue_to_rdd(rdpa_dir_us, channel, queue_id, rc_id, queue, tc);
}

static inline int _rdpa_egress_tm_lan_port_queue_to_rdd(rdpa_if port, uint32_t queue_id, int *rc_id, int *queue)
{
    return _rdpa_egress_tm_dir_channel_queue_to_rdd(rdpa_dir_ds, _rdpa_port_channel(port), queue_id, rc_id, queue);
}

static inline int _rdpa_egress_tm_lan_port_queue_to_rdd_tc_check(rdpa_if port, uint32_t queue_id, int *rc_id, int *queue, int *tc)
{
    int channel = _rdpa_port_channel_no_lock(port);

    return __rdpa_egress_tm_dir_channel_queue_to_rdd(rdpa_dir_ds, &channel, queue_id, rc_id, queue, tc);
}

static inline int _rdpa_egress_tm_channel_queue_to_rdd_tc_check(rdpa_traffic_dir dir, int channel, uint32_t queue_id, int *rc_id, int *queue, int *tc)
{
    return __rdpa_egress_tm_dir_channel_queue_to_rdd(dir, &channel, queue_id, rc_id, queue, tc);
}

#if !defined(BDMF_SYSTEM_SIM) && !defined(XRDP)
#define _ALIGN_UP(addr, size)     (((addr)+((size)-1))&(~((size)-1)))

#ifndef LEGACY_RDP
#define RDD_RUNNER_PACKET_DESCR_SIZE sizeof(RDD_PACKET_DESCRIPTOR_DTS)
#else
#if defined(LILAC_RDD_RUNNER_PACKET_DESCRIPTOR_SIZE)
#define RDD_RUNNER_PACKET_DESCR_SIZE LILAC_RDD_RUNNER_PACKET_DESCRIPTOR_SIZE
#else
#define RDD_RUNNER_PACKET_DESCR_SIZE sizeof(RDD_PACKET_DESCRIPTOR_DTS)
#endif
#endif
static inline void *ddr_queue_alloc(size_t size, void **phy_addr_mem)
{
    void *p, *p2;

    /* aligned to SLOT size (4 * PD) */
    p = bdmf_alloc((size * RDD_RUNNER_PACKET_DESCR_SIZE) + (4 * RDD_RUNNER_PACKET_DESCR_SIZE));
    if (!p)
        return NULL;

    /* Check if the pointer is correctly aligned */
    if ((size_t)(p) % (4 * RDD_RUNNER_PACKET_DESCR_SIZE))
    {
        /* Allocate the new block */
        p2 = (void *)_ALIGN_UP((size_t)p, (4 * RDD_RUNNER_PACKET_DESCR_SIZE));
        *phy_addr_mem = p;
        return p2;
    }
    else
    {
        /* The pointer p is already aligned to the boudary */
        *phy_addr_mem = p;
        return p;
    }
}
#endif /* #if !defined(BDMF_SYSTEM_SIM) */

#endif /* _RDPA_EGRESS_TM_INLINE_H_ */
