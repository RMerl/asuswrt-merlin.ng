/*
<:copyright-BRCM:2023:DUAL/GPL:standard

   Copyright (c) 2023 Broadcom 
   All Rights Reserved

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

#ifndef __GDX_RUNNER__H__
#define __GDX_RUNNER__H__

#include "rdpa_api.h"
#include "rdp_cpu_ring_defs.h"
#include "rdp_mm.h"
#include "rdp_cpu_ring.h"

/** README **
 *  This file is split into 2 sets of function definitions.
 *  (1) Mandatory functions that MUST be defined by the hardware
 *  accelerator in order to integrate with GDX. Mandatory
 *  functions are NOT prefixed by '_'
 *  (2) Optional/Supporting function definitions. Optional
 *  functions are prefixed by '_' */

#define skb_idx_dest_ifid       gdx_pd_data
#define rx_skb_idx              gdx_pd_data
#define dest_ifid               gdx_pd_data

typedef struct {
    int qid;
    int group_idx;
} gdx_rx_isr_context_t;

/* max # of skb_idx freed at a time */ 
#define GDX_SKB_IDX_FREE_BUDGET 32

uint32_t gdx_skb_idx_used_thresh = GDX_SKB_IDX_FREE_BUDGET; 

/* skb idx pool size */
#define GDX_SKB_IDX_SIZE     13
#define GDX_MAX_SKB_IDX_COUNT   (1<< GDX_SKB_IDX_SIZE)

/* make sure the skb idx pool size and CPU RX queue size match */
#define GDX_CPU_RX_QUEUE_SIZE  GDX_MAX_SKB_IDX_COUNT

/* 2 queues(Priorities low & high) per GDX Instance. Even-Low, Odd-High */
#define GDX_NUM_QUEUES_PER_GDX_INST 2
#define GDX_NUM_QUEUE_SUPPORTED (GDX_MAX_DEV_GROUPS * GDX_NUM_QUEUES_PER_GDX_INST)

typedef struct {
	struct list_head lnode; /* list entry */
    uint16_t idx;
} gdx_skb_idx_node_t;

struct gdx_skb_idx_info
{
    uint32_t max_skb_idx_count;
    uint32_t avail_skb_idx_count;
    uint32_t skb_idx_alloc_count;
    uint32_t skb_idx_free_count_rx_miss;
    uint32_t skb_idx_free_count_rx_drop;
    uint32_t skb_idx_free_count_rx_hit;
    uint32_t skb_idx_free_count_rx_error;
    uint32_t rx_no_skb_idx;

    struct list_head alloc_list;
    struct list_head free_list;
    gdx_skb_idx_node_t   skb_idx_pool[GDX_MAX_SKB_IDX_COUNT];

    struct sk_buff *skb_tbl[GDX_MAX_SKB_IDX_COUNT];
    uint8_t sw_status[GDX_MAX_SKB_IDX_COUNT]; /* sw status for all the skb idx */
    uint8_t *hw_status_p;  /* HW accelerator will reset this status field for RX accelerated pkt */
    uintptr_t hw_status_pa; /* physical address of HW status DMA memory */
} ____cacheline_aligned;
typedef struct gdx_skb_idx_info gdx_skb_idx_info_t;


typedef struct {
    int                 initialized;
    unsigned long       rx_work_avail;          /* Runner GDX RX queue work pending */
    unsigned int        queue_mask;
    wait_queue_head_t   rx_thread_wqh;
    struct task_struct  *rx_thread;
    spinlock_t          skb_idx_lock;                   /* protect skb_idx_info, CPU-TX queue, and driver RX stats */
    gdx_skb_idx_info_t  skb_idx_info[GDX_MAX_DEV_GROUPS];
    void                *hw_specific_data_p;
    gdx_queue_stats_t   queue_stats[GDX_NUM_QUEUES_PER_GDX_INST];
} gdx_runner_priv_info_t;

gdx_runner_priv_info_t gdx_hwacc_priv = {.initialized = 0};

#define GDX_GET_QUEUE_STAT(qid,stat_name)    gdx_hwacc_priv.queue_stats[qid].stat_name
#define GDX_INCR_QUEUE_STATS(qid,stat_name)  gdx_hwacc_priv.queue_stats[qid].stat_name++
#define GDX_GET_QUEUE_STATS_PTR(qid)         &gdx_hwacc_priv.queue_stats[qid]
#define GDX_GET_QUEUE_MASK                   gdx_hwacc_priv.queue_mask

#define GDX_GET_SKB_IDX_INFO_PTR(gid)        &gdx_hwacc_priv.skb_idx_info[gid]

#define GDX_WAKEUP_RXWORKER(group_idx) do { \
            wake_up_interruptible(&gdx_hwacc_priv.rx_thread_wqh); \
          } while (0)


static inline void gdx_hwacc_group_int_enable(int group_idx);
static inline int gdx_hwacc_misc_processing(gdx_dev_group_t *dev_group_p);
static inline int gdx_hwacc_get_qid(int qidx);
/****************************************************************************/
/** Name: _gdx_hwacc_free_skb_and_skb_idx                                  **/
/**                                                                        **/
/** Description: Frees an skb_idx and returns the skb saved at skb_idx     **/
/**   entry. Note: gdx_hwacc_priv.skb_idx_lock should be taken by the caller         **/
/**                                                                        **/
/** Input: skb_idx_info_p- skb_idx_info for the dev group                  **/
/**        skb_idx       - skb_idx to free                                 **/
/**                                                                        **/
/** Output: skb_pp        - saved skb                                      **/
/**         Return value  - GDX_SUCCESS / GDX_FAILURE                      **/
/****************************************************************************/
static inline int _gdx_hwacc_free_skb_and_skb_idx(gdx_skb_idx_info_t *skb_idx_info_p,
       int skb_idx, struct sk_buff **skb_pp)
{
    struct sk_buff *skb = skb_idx_info_p->skb_tbl[skb_idx];
    int hw_status = skb_idx_info_p->hw_status_p[skb_idx];

    if (skb_idx_info_p->sw_status[skb_idx] == GDX_SKB_STATUS_FREE)
    {
	    GDX_PRINT_ERROR("skb_idx<%d> already free", skb_idx);
        return GDX_FAILURE;
    }
    *skb_pp = NULL;

    switch (hw_status)
    {
        case GDX_SKB_STATUS_HIT:
            skb_idx_info_p->skb_idx_free_count_rx_hit++;
            dev_kfree_skb_thread(skb);
            break;

        case GDX_SKB_STATUS_DROP:
            skb_idx_info_p->skb_idx_free_count_rx_drop++;
            dev_kfree_skb_thread(skb);
            break;

        case GDX_SKB_STATUS_MISS:
            skb_idx_info_p->skb_idx_free_count_rx_miss++;
            *skb_pp = skb;
            break;

        default:
            GDX_PRINT_ERROR("skb_idx<%d> status skb<%d>", skb_idx, hw_status);
            return GDX_FAILURE;
    }

    GDX_PRINT_DBG("skb_idx<%d> skb<0x%px>", skb_idx, skb);
    skb_idx_info_p->hw_status_p[skb_idx] = GDX_SKB_STATUS_FREE;
    skb_idx_info_p->sw_status[skb_idx] = GDX_SKB_STATUS_FREE;
    skb_idx_info_p->skb_tbl[skb_idx] = NULL;

    return GDX_SUCCESS;
}

/* ******(1) Mandatory Functions - START *******
 * These are the functions that a hardware accelerator MUST
 * define in order to integrate with GDX */

/****************************************************************************/
/** Name: gdx_hwacc_pkt_recv                                               **/
/**                                                                        **/
/** Description: Receive packet from the hardware accelerator              **/
/**                                                                        **/
/** Input: group_idx - group index                                         **/
/**        qid       - queue id                                            **/
/**                                                                        **/
/** Output: info - Pointer to store the packet attributes                  **/
/**         Returns 0 on Success, Non zero otherwise                       **/
/****************************************************************************/
static inline int gdx_hwacc_pkt_recv(unsigned int groupidx, unsigned long qid, gdx_hwacc_rx_info_t *info)
{
    rdpa_cpu_rx_info_t rdpa_info = {};
    int rc;

    rc = rdpa_cpu_packet_get(rdpa_cpu_gdx + groupidx, qid, &rdpa_info);

    if (rc == 0)
    {
        info->data = rdpa_info.data;
        info->data_offset = rdpa_info.data_offset;
        info->gdx_pd_data = rdpa_info.gdx_pd_data;
        info->is_exception = rdpa_info.is_exception;
        info->size = rdpa_info.size;
        info->rx_csum_verified = rdpa_info.rx_csum_verified;
    }
    else if (rc == BDMF_ERR_NO_MORE) 
    {
        return GDX_ERR_NO_MORE;
    }
    return 0;
}

/****************************************************************************/
/** Name: gdx_hwacc_get_lpbk_skb                                           **/
/**                                                                        **/
/** Description: This function will free the skb_idx for the HW flow miss  **/
/**   case. skb_idx is moved from alloc_list to the end of free_list and   **/
/**   return the skb stored at the skb idx location                        **/
/**   Note: gdx_hwacc_priv.skb_idx_lock should be taken by the caller                **/
/**                                                                        **/
/** Input: dev_group_p   - dev group                                       **/
/**        info          - Info structure containing skb_idx to be freed   **/
/**                                                                        **/
/** Output: skb          - skb stored at skb_idx entry                     **/
/****************************************************************************/
static inline struct sk_buff *gdx_hwacc_get_lpbk_skb(gdx_dev_group_t *dev_group_p, gdx_hwacc_rx_info_t *info)
{
    struct sk_buff *skb = NULL;
	gdx_skb_idx_node_t *skb_idx_node_p;
    gdx_skb_idx_info_t *skb_idx_info_p;

    skb_idx_info_p = GDX_GET_SKB_IDX_INFO_PTR(dev_group_p->group_idx);
    GDX_ASSERT(info->rx_skb_idx < GDX_MAX_SKB_IDX_COUNT);

    if ((skb_idx_info_p->sw_status[info->rx_skb_idx] == GDX_SKB_STATUS_IN_USE) &&
            (skb_idx_info_p->hw_status_p[info->rx_skb_idx] == GDX_SKB_STATUS_IN_USE))
    {
		skb_idx_node_p = &skb_idx_info_p->skb_idx_pool[info->rx_skb_idx];

        if (_gdx_hwacc_free_skb_and_skb_idx(skb_idx_info_p, info->rx_skb_idx, &skb) != GDX_SUCCESS)
            goto gdx_free_miss_skb_idx_and_get_skb_err;

        list_del_init(&skb_idx_node_p->lnode);
		list_add_tail(&skb_idx_node_p->lnode, &skb_idx_info_p->free_list);
        skb_idx_info_p->avail_skb_idx_count++;

        return skb;
    }

gdx_free_miss_skb_idx_and_get_skb_err:
    GDX_PRINT_ERROR("Invalid: skb_idx<%d> status", info->rx_skb_idx);
    GDX_PRINT_ERROR("[%d] sw_status = %d hw_status = %d skb = 0x%px", info->rx_skb_idx,
              skb_idx_info_p->sw_status[info->rx_skb_idx],
              skb_idx_info_p->hw_status_p[info->rx_skb_idx],
              skb_idx_info_p->skb_tbl[info->rx_skb_idx]);

    return skb;
}

/****************************************************************************/
/** Name: gdx_hwacc_databuf_free                                           **/
/**                                                                        **/
/** Description: Free the data buffer                                      **/
/**                                                                        **/
/** Input: nbuff_p - Data buffer                                           **/
/**        context - unused,for future use                                 **/
/**                                                                        **/
/** Output: None                                                           **/
/****************************************************************************/
static inline void gdx_hwacc_databuf_free(void *nbuff_p, uint32_t context)
{
    bdmf_sysb_databuf_free(nbuff_p, context);
}
/* ******(1) Mandatory Functions - END *******/

/* ******(2) Hardware Accelerator Optional/Supporting functions - START******
   NOTE: Supporting functions are prefixed by '_' for example: _XXX_YYY()
   These supporting functions are specific to runner. Other accelerators
   may or may not need similar supporting functions */

/****************************************************************************/
/** Name: _gdx_hwacc_get_min_q_idx                                         **/
/**                                                                        **/
/** Description: Get start queue index for the group                       **/
/**                                                                        **/
/** Input: group_idx - group index                                         **/
/**                                                                        **/
/** Output: Returns start qidx, GDX_FAILURE on error                       **/
/****************************************************************************/
static inline int _gdx_hwacc_get_min_q_idx(int group_idx)
{
    if (group_idx >= GDX_MAX_DEV_GROUPS)
    {
        GDX_PRINT_ERROR("group_idx %d out of bounds(%d)", group_idx, GDX_MAX_DEV_GROUPS);
        return GDX_FAILURE;
    }

    return 0;
}

/****************************************************************************/
/** Name: _gdx_hwacc_get_max_q_idx                                         **/
/**                                                                        **/
/** Description: Get end queue index for the group                         **/
/**                                                                        **/
/** Input: group_idx - group index                                         **/
/**                                                                        **/
/** Output: Returns end qidx, GDX_FAILURE on error                         **/
/****************************************************************************/
static inline int _gdx_hwacc_get_max_q_idx(int group_idx)
{
    if (group_idx >= GDX_MAX_DEV_GROUPS)
    {
        GDX_PRINT_ERROR("group_idx %d out of bounds(%d)", group_idx, GDX_MAX_DEV_GROUPS);
        return GDX_FAILURE;
    }
    return GDX_NUM_QUEUES_PER_GDX_INST - 1;
}

void _gdx_hw_rx_notify(int qid, int group_idx)
{
    /*Atomically set the queue bit on*/
    set_bit(qid, &gdx_hwacc_priv.rx_work_avail);

    /* Call the Hwaccelerator receiving packets handler (thread or tasklet) */
    GDX_WAKEUP_RXWORKER(group_idx);
}

/****************************************************************************/
/** Name: _gdx_hwacc_dev_rx_isr_callback                                   **/
/**                                                                        **/
/** Description: Disable interrupt for the given queue                     **/
/**                                                                        **/
/** Input: priv - context                                                  **/
/**                                                                        **/
/** Output: None                                                           **/
/****************************************************************************/
static inline void _gdx_hwacc_dev_rx_isr_callback(long priv)
{
    gdx_rx_isr_context_t *ctx = (gdx_rx_isr_context_t *)priv;

    /* Disable PCI interrupt */
    rdpa_cpu_int_disable(rdpa_cpu_gdx + gdx_dev_groups[ctx->group_idx].group_idx, ctx->qid);
    rdpa_cpu_int_clear(rdpa_cpu_gdx + gdx_dev_groups[ctx->group_idx].group_idx, ctx->qid);

    _gdx_hw_rx_notify(ctx->qid, ctx->group_idx);
}

/****************************************************************************/
/** Name: _gdx_hwacc_get_dev_from_skb_idx                                  **/
/**                                                                        **/
/** Description: Get skb device corresponding to skb_idx                   **/
/**                                                                        **/
/** Input: skb_idx_info_p - skb index info pointer                         **/
/**        skb_idx - skb index                                             **/
/**                                                                        **/
/** Output: Return device pointer                                          **/
/****************************************************************************/
static inline struct net_device *_gdx_hwacc_get_dev_from_skb_idx(gdx_skb_idx_info_t *skb_idx_info_p, 
                                                                 int skb_idx)
{
    if (skb_idx_info_p->skb_tbl[skb_idx])
    {
        return skb_idx_info_p->skb_tbl[skb_idx]->dev;
    }

    return NULL;
}

/* returns TRUE if HW accelerator has freed the skb_idx */
#define  is_hw_status_free(free_idx)   \
    ((skb_idx_info_p->sw_status[free_idx] == GDX_SKB_STATUS_IN_USE) && \
     ((skb_idx_info_p->hw_status_p[free_idx] == GDX_SKB_STATUS_HIT) || \
      (skb_idx_info_p->hw_status_p[free_idx] == GDX_SKB_STATUS_DROP)))

/****************************************************************************/
/** Name: _gdx_hwacc_free_hit_or_drop_skb_idx                              **/
/**                                                                        **/
/** Description: This function will free the skb_idx which was freed by    **/
/**   the HW accelerator for either the flow hit or pkt drop case.         **/
/**   Tries to free "free_budget" # of skb_idx from the head of alloc_list.**/
/**   If an skb_idx is not free it is moved to tail of the alloc_list.     **/
/**                                                                        **/
/** Input: dev_group_t   - gdx dev group                                   **/
/**        skb_idx_info_p- skb_idx_info for the dev group                  **/
/**        free_budget   - # of skb_idx to free                            **/
/**                                                                        **/
/** Output:                                                                **/
/****************************************************************************/
static inline void _gdx_hwacc_free_hit_or_drop_skb_idx(gdx_dev_group_t *dev_group_p, gdx_skb_idx_info_t *skb_idx_info_p,
        int free_budget)
{
    int skb_idx;
    int freed_skb_count = 0;
    struct sk_buff *skb;
    struct list_head *cur, *tmp;
    gdx_skb_idx_node_t *skb_idx_node_p;
    struct list_head tmp_list;
    
    INIT_LIST_HEAD(&tmp_list);
    spin_lock_bh(&gdx_hwacc_priv.skb_idx_lock);
    list_for_each_safe(cur, tmp, &skb_idx_info_p->alloc_list) {
        list_del_init(cur);
        skb_idx_node_p = list_entry(cur, gdx_skb_idx_node_t, lnode);
        skb_idx = skb_idx_node_p->idx;

        if (is_hw_status_free(skb_idx))
        {
            _gdx_hwacc_free_skb_and_skb_idx(skb_idx_info_p, skb_idx, &skb);
		    list_add_tail(&skb_idx_node_p->lnode, &skb_idx_info_p->free_list);
            freed_skb_count++;
        }
        else
        {
            /* add to the tail of tmp_list */
            list_add_tail(&skb_idx_node_p->lnode, &tmp_list);
            GDX_PRINT_DBG1("moving to tail of alloc_list skb_idx<%d> status skb<%d>",
                   skb_idx, skb_idx_info_p->hw_status_p[skb_idx]);
        }

        if (freed_skb_count >= free_budget)
            goto gdx_free_hit_or_drop_skb_idx_done;
    }

gdx_free_hit_or_drop_skb_idx_done:
    /* Add tmp_list to the tail of alloc_list */
    list_splice_tail(&tmp_list, &skb_idx_info_p->alloc_list);
    skb_idx_info_p->avail_skb_idx_count += freed_skb_count;
    spin_unlock_bh(&gdx_hwacc_priv.skb_idx_lock);
    GDX_PRINT_DBG("freed_skb_count<%d>", freed_skb_count);
}

/****************************************************************************/
/** Name: _gdx_hwacc_alloc_skb_idx_info                                    **/
/**                                                                        **/
/** Description: Initializes the alloc and free lists. Also, initializes   **/
/**   the skb_idx, pool, sw and hw status maps.                            **/
/**                                                                        **/
/** Input:                                                                 **/
/**    dev_group_p   - The dev group to which skb_idx_info belongs         **/
/**    skb_idx_count - # of skb_idx to allocate                            **/
/**                                                                        **/
/** Output: Return value GDX_SUCCESS / GDX_FAILURE                         **/
/****************************************************************************/
static inline int _gdx_hwacc_alloc_skb_idx_info(gdx_dev_group_t *dev_group_p, 
                                                int skb_idx_count)
{
    int skb_idx;
    gdx_skb_idx_info_t *skb_idx_info_p = GDX_GET_SKB_IDX_INFO_PTR(dev_group_p->group_idx);
    gdx_skb_idx_node_t *skb_idx_node_p;

    spin_lock_bh(&gdx_hwacc_priv.skb_idx_lock);
    INIT_LIST_HEAD(&skb_idx_info_p->alloc_list);
    INIT_LIST_HEAD(&skb_idx_info_p->free_list);

    /* init the sw and hw status for skb_idx */
    for (skb_idx = 0; (skb_idx < skb_idx_count); skb_idx++)
    {
        skb_idx_node_p = &skb_idx_info_p->skb_idx_pool[skb_idx];
        INIT_LIST_HEAD(&skb_idx_node_p->lnode);
        list_add_tail(&skb_idx_node_p->lnode, &skb_idx_info_p->free_list);

        skb_idx_node_p->idx = skb_idx;
        skb_idx_info_p->hw_status_p[skb_idx] = GDX_SKB_STATUS_FREE;
        skb_idx_info_p->sw_status[skb_idx] = GDX_SKB_STATUS_FREE;
    }

    skb_idx_info_p->avail_skb_idx_count = skb_idx_count;
    skb_idx_info_p->max_skb_idx_count = skb_idx_count;

    spin_unlock_bh(&gdx_hwacc_priv.skb_idx_lock);
    return GDX_SUCCESS;
}

/****************************************************************************/
/** Name: _gdx_hwacc_free_skb_idx_alloc_list                               **/
/**                                                                        **/
/** Description: Frees all the skb_idx in alloc list.                      **/
/**                                                                        **/
/** Input: dev_group_p - The dev group to which skb_idx_info belongs       **/
/**                                                                        **/
/** Output: Return value GDX_SUCCESS / GDX_FAILURE                         **/
/****************************************************************************/
static inline int _gdx_hwacc_free_skb_idx_alloc_list(gdx_dev_group_t *dev_group_p)
{
    struct list_head *cur, *tmp;
    int skb_idx;
    gdx_skb_idx_node_t *skb_idx_node_p;
    gdx_skb_idx_info_t *skb_idx_info_p = GDX_GET_SKB_IDX_INFO_PTR(dev_group_p->group_idx);
    struct sk_buff *skb;

    spin_lock_bh(&gdx_hwacc_priv.skb_idx_lock);
    list_for_each_safe(cur, tmp, &skb_idx_info_p->alloc_list) {
        list_del_init(cur);
        skb_idx_node_p = list_entry(cur, gdx_skb_idx_node_t, lnode);
        skb_idx = skb_idx_node_p->idx;

        if (is_hw_status_free(skb_idx) || (skb_idx_info_p->hw_status_p[skb_idx] == GDX_SKB_STATUS_MISS))
        {
            _gdx_hwacc_free_skb_and_skb_idx(skb_idx_info_p, skb_idx, &skb);
            skb_idx_info_p->avail_skb_idx_count++;
        }
    }
    spin_unlock_bh(&gdx_hwacc_priv.skb_idx_lock);

    return GDX_SUCCESS;
}

/****************************************************************************/
/** Name: _gdx_hwacc_rdpa_alloc_hw_status_dma_mem                          **/
/**                                                                        **/
/** Description: Allocate the skb index HW status DMA mem.                 **/
/**                                                                        **/
/** Input: skb_idx_info_p - skb_idx_info containing the pointer to store   **/ 
/**        the allocated dma memory                                        **/
/**                                                                        **/
/** Output: Return value GDX_SUCCESS / GDX_FAILURE                         **/
/****************************************************************************/
static inline int _gdx_hwacc_rdpa_alloc_hw_status_dma_mem(gdx_skb_idx_info_t *skb_idx_info_p)
{
    rdpa_gdx_params_t gdx_params;

    skb_idx_info_p->hw_status_p = NULL;
    skb_idx_info_p->hw_status_pa = 0L;
    if (!skb_idx_info_p->hw_status_p)
    {
        skb_idx_info_p->hw_status_p = dma_alloc_coherent(rdp_dummy_dev, GDX_MAX_SKB_IDX_COUNT,
            (dma_addr_t *) &skb_idx_info_p->hw_status_pa, GFP_KERNEL);
    }

    if (!skb_idx_info_p->hw_status_p)
    {
        GDX_PRINT_ERROR("dma_alloc_coherent failed\n");
        return GDX_FAILURE;
    }
    else
    {
        GDX_PRINT_INFO("skb_idx_info_p->hw_status_p=0x%px, skb_idx_info_p->hw_status_pa=0x%llx\n",
               skb_idx_info_p->hw_status_p, (uint64_t)skb_idx_info_p->hw_status_pa);
    }

    gdx_params.base_addr = (uint64_t)skb_idx_info_p->hw_status_pa;
    gdx_params.hit_value = GDX_SKB_STATUS_HIT;
    gdx_params.drop_value = GDX_SKB_STATUS_DROP;
    rdpa_gdx_params_set(&gdx_params);

    return GDX_SUCCESS;
}

/****************************************************************************/
/** Name: _gdx_hwacc_rdpa_alloc_hw_status_dma_mem                          **/
/**                                                                        **/
/** Description: Allocate the skb index HW status DMA mem.                 **/
/**                                                                        **/
/** Input: skb_idx_info_p - Free the skb index HW status DMA mem allocated **/ 
/**                                                                        **/
/** Output: None                                                           **/                                              
/****************************************************************************/
static inline void _gdx_hwacc_rdpa_free_hw_status_dma_mem(gdx_skb_idx_info_t *skb_idx_info_p)
{
    rdpa_gdx_params_t gdx_params;

    GDX_PRINT_INFO("skb_idx_info_p->hw_status_p=0x%px, skb_idx_info_p->hw_status_pa=0x%llx\n",
           skb_idx_info_p->hw_status_p, (uint64_t)skb_idx_info_p->hw_status_pa);
    if (skb_idx_info_p->hw_status_p)
    {
        dma_free_coherent(rdp_dummy_dev, GDX_MAX_SKB_IDX_COUNT,
                skb_idx_info_p->hw_status_p, (dma_addr_t) skb_idx_info_p->hw_status_pa);
    }

    skb_idx_info_p->hw_status_p = NULL;
    skb_idx_info_p->hw_status_pa = 0L;

    gdx_params.base_addr = 0;
    gdx_params.hit_value = GDX_SKB_STATUS_HIT;
    gdx_params.drop_value = GDX_SKB_STATUS_DROP;
    rdpa_gdx_params_set(&gdx_params);
}

/****************************************************************************/
/** Name: _gdx_hwacc_rdpa_init                                             **/
/**                                                                        **/
/** Description: Initialize RDPA                                           **/
/**                                                                        **/
/** Input: skb_idx_info_p - Free the skb index HW status DMA mem allocated **/ 
/**                                                                        **/
/** Output: Return value GDX_SUCCESS / GDX_FAILURE                         **/
/****************************************************************************/
static inline int _gdx_hwacc_rdpa_init(int group_idx, const char *gdx_dev_name)
{
    int rc;
    bdmf_object_handle rdpa_cpu_obj, rdpa_port_obj;
    rdpa_port_dp_cfg_t port_cfg = {};
    BDMF_MATTR_ALLOC(cpu_gdx_attrs, rdpa_cpu_drv());
    BDMF_MATTR_ALLOC(rdpa_port_attrs, rdpa_port_drv());
    gdx_skb_idx_info_t *skb_idx_info_p = GDX_GET_SKB_IDX_INFO_PTR(group_idx);
    gdx_dev_group_t *dev_group_p = &gdx_dev_groups[group_idx];

    if (!cpu_gdx_attrs || !rdpa_port_attrs)
    {
        GDX_PRINT_ERROR("Failed to allocate %s%s%s.", 
                        !cpu_gdx_attrs ? "cpu_gdx_attrs" : "", 
                        !cpu_gdx_attrs && !rdpa_port_attrs ? "&" : "", 
                        !rdpa_port_attrs ? "rdpa_port_attrs" : "");
        rc = GDX_FAILURE;
        goto clean_mattr_on_ret;
    }

    if (_gdx_hwacc_rdpa_alloc_hw_status_dma_mem(skb_idx_info_p) != GDX_SUCCESS)
    {
        rc = GDX_FAILURE;
        goto clean_mattr_on_ret;
    }
    /* create cpu */
    rdpa_cpu_index_set(cpu_gdx_attrs, rdpa_cpu_gdx + group_idx);

    /* Number of queues for GDX */
    rdpa_cpu_num_queues_set(cpu_gdx_attrs, GDX_NUM_QUEUES_PER_GDX_INST);

    if ((rc = bdmf_new_and_set(rdpa_cpu_drv(), NULL, cpu_gdx_attrs, &rdpa_cpu_obj)))
    {
        GDX_PRINT_ERROR("Failed to create cpu gendev%d object rc(%d)", rdpa_cpu_gdx + group_idx, rc);
        rc = GDX_FAILURE;
        goto gdx_rdpa_init_err_destroy_cpu_obj;
    }

    if ((rc = rdpa_cpu_int_connect_set(rdpa_cpu_obj, true)) && rc != BDMF_ERR_ALREADY)
    {
        GDX_PRINT_ERROR("Failed to connect cpu interrupts rc(%d)", rc);
        rc = GDX_FAILURE;
        goto gdx_rdpa_init_err_destroy_cpu_obj;
    }

    if ((rc = rdpa_port_name_set(rdpa_port_attrs, gdx_dev_name)))
     {
        GDX_PRINT_ERROR("Failed to set RDPA port name %s. rc=%d\n", gdx_dev_name, rc);
        rc = GDX_FAILURE;
        goto clean_mattr_on_ret;
    }

    if ((rc = rdpa_port_type_set(rdpa_port_attrs, rdpa_port_gdx)))
     {
        GDX_PRINT_ERROR("Failed to set RDPA port type rdpa_port_cpu. rc=%d\n", rc);
        rc = GDX_FAILURE;
        goto clean_mattr_on_ret;
    }

    if ((rc = rdpa_port_index_set(rdpa_port_attrs, group_idx)))
     {
        GDX_PRINT_ERROR("Failed to set RDPA port index %d. rc=%d\n", group_idx, rc);
        rc = GDX_FAILURE;
        goto clean_mattr_on_ret;
    }

    if ((rc = rdpa_port_handle_set(rdpa_port_attrs, RDPA_PORT_CPU_HANDLE)))
     {
        GDX_PRINT_ERROR("Failed to set RDPA handle. rc=%d\n", rc);
        rc = GDX_FAILURE;
        goto clean_mattr_on_ret;
    }

    if ((rc = rdpa_port_is_wan_set(rdpa_port_attrs, 0)))
     {
        GDX_PRINT_ERROR("Failed to set RDPA port is_wan FALSE. rc=%d\n", rc);
        rc = GDX_FAILURE;
        goto clean_mattr_on_ret;
    }

    rdpa_port_cpu_obj_set(rdpa_port_attrs, rdpa_cpu_obj);
    rc = bdmf_new_and_set(rdpa_port_drv(), NULL, rdpa_port_attrs, &rdpa_port_obj);
    if (rc)
    {
        GDX_PRINT_ERROR("Failed to create rdpa port object rc(%d)", rc);
        rc = GDX_FAILURE;
        goto gdx_rdpa_init_err_destroy_cpu_obj;
    }

    if ((rc = rdpa_port_cfg_get(rdpa_port_obj, &port_cfg)))
    {
        GDX_PRINT_ERROR("Failed to get configuration for RDPA port %d. rc=%d", rdpa_cpu_gdx + group_idx, rc);
        rc = GDX_FAILURE;
        goto gdx_rdpa_init_err_destroy_port_obj;
    }

    if ((rc = rdpa_port_cfg_set(rdpa_port_obj, &port_cfg)))
    {
        GDX_PRINT_ERROR("Failed to set configuration for RDPA port %d. rc=%d", rdpa_cpu_gdx + group_idx, rc);
        rc = GDX_FAILURE;
        goto gdx_rdpa_init_err_destroy_port_obj;
    }

    memcpy(dev_group_p->dev_name, gdx_dev_name, IFNAMSIZ);
    gdx_hwacc_priv.hw_specific_data_p = rdpa_port_obj;

    goto clean_mattr_on_ret;

gdx_rdpa_init_err_destroy_port_obj:
    if (!rdpa_port_get(gdx_dev_name, &rdpa_port_obj))
    {
        bdmf_put(rdpa_port_obj);
        bdmf_destroy(rdpa_port_obj);
    }

gdx_rdpa_init_err_destroy_cpu_obj:
    if (!rdpa_cpu_get(rdpa_cpu_gdx + group_idx, &rdpa_cpu_obj))
    {
        bdmf_put(rdpa_cpu_obj);
        bdmf_destroy(rdpa_cpu_obj);
    }

    _gdx_hwacc_rdpa_free_hw_status_dma_mem(skb_idx_info_p);
clean_mattr_on_ret:
    if (cpu_gdx_attrs) {
        BDMF_MATTR_FREE(cpu_gdx_attrs);
    }

    if (rdpa_port_attrs) {
        BDMF_MATTR_FREE(rdpa_port_attrs);
    }

    return rc;
}

/****************************************************************************/
/** Name: _gdx_hwacc_rdpa_uninit                                           **/
/**                                                                        **/
/** Description: Uninitialize RDPA objects associated with a group         **/
/**                                                                        **/
/** Input: group_idx - Group index                                         **/ 
/**                                                                        **/
/** Output: None                                                           **/
/****************************************************************************/
static inline void _gdx_hwacc_rdpa_uninit(int group_idx)
{
    bdmf_object_handle rdpa_cpu_obj;
    gdx_skb_idx_info_t *skb_idx_info_p = GDX_GET_SKB_IDX_INFO_PTR(group_idx);

    _gdx_hwacc_rdpa_free_hw_status_dma_mem(skb_idx_info_p);

    if (gdx_hwacc_priv.hw_specific_data_p)
    {
        bdmf_put(gdx_hwacc_priv.hw_specific_data_p);
        bdmf_destroy(gdx_hwacc_priv.hw_specific_data_p);
        gdx_hwacc_priv.hw_specific_data_p = NULL;
    }

    if (!rdpa_cpu_get(rdpa_cpu_gdx + group_idx, &rdpa_cpu_obj))
    {
        bdmf_put(rdpa_cpu_obj);
        bdmf_destroy(rdpa_cpu_obj);
    }
}

/****************************************************************************/
/** Name: _gdx_hwacc_config_tc_to_queue                                    **/
/**                                                                        **/
/** Description: Map TC to qid                                             **/
/**                                                                        **/
/** Input: group_idx - Group index, qid - queue id                         **/ 
/**                                                                        **/
/** Output: None                                                           **/
/****************************************************************************/
static inline int _gdx_hwacc_config_tc_to_queue(int group_idx, int qid)
{
    bdmf_object_handle rdpa_cpu_obj = NULL;
    bdmf_object_handle rdpa_sys_obj = NULL;
    int rc = 0;
    int tc_idx, tc_idx_start, tc_idx_end;
    rdpa_cpu_tc  cpu_tc_threshold = rdpa_cpu_tc0;

    rc = rdpa_system_get(&rdpa_sys_obj);
    rc = rc ? rc : rdpa_system_high_prio_tc_threshold_get(rdpa_sys_obj, &cpu_tc_threshold);
    rc = rc ? rc : rdpa_cpu_get(rdpa_cpu_gdx + gdx_dev_groups[group_idx].group_idx, &rdpa_cpu_obj);

    if (rc)
        goto tc2queue_exit;

    tc_idx_start = (qid == _gdx_hwacc_get_min_q_idx(group_idx))? rdpa_cpu_tc0 : (cpu_tc_threshold + 1);
    /* XXX: Need to check that tc_idx_start != GDX_FAILURE? */
    tc_idx_end = (qid == _gdx_hwacc_get_min_q_idx(group_idx))? cpu_tc_threshold : rdpa_cpu_tc7;

    for (tc_idx = tc_idx_start; tc_idx <= tc_idx_end;  tc_idx ++)
        rdpa_cpu_tc_to_rxq_set(rdpa_cpu_obj, tc_idx, qid);

tc2queue_exit:
    if (rdpa_sys_obj)
        bdmf_put(rdpa_sys_obj);

    if (rdpa_cpu_obj)
        bdmf_put(rdpa_cpu_obj);

    return rc;
}

/****************************************************************************/
/** Name: _gdx_hwacc_config_rx_q                                           **/
/**                                                                        **/
/** Description: Configure the runner queue                                **/
/**                                                                        **/
/** Input: group_idx - Group index                                         **/ 
/**        qid - queue id                                                  **/ 
/**        qsize - queue size                                              **/ 
/** Output: GDX_SUCCESS / GDX_FAILURE                                      **/ 
/****************************************************************************/
static inline int _gdx_hwacc_config_rx_q(int group_idx, int qid, uint32_t qsize)
{
    rdpa_cpu_rxq_cfg_t rxq_cfg;
    bdmf_object_handle rdpa_cpu_obj;
    uint32_t *ring_base = NULL;
    int rc = 0;

    if (rdpa_cpu_get(rdpa_cpu_gdx + gdx_dev_groups[group_idx].group_idx, &rdpa_cpu_obj))
        return GDX_FAILURE;

    /* Read current configuration, set new drop threshold and ISR and write back. */
    rc = rdpa_cpu_rxq_cfg_get(rdpa_cpu_obj, qid, &rxq_cfg);
    if (rc)
        goto unlock_exit;

    if (qsize)
    {
        gdx_rx_isr_context_t *isr_ctx = (gdx_rx_isr_context_t *)kmalloc(sizeof(gdx_rx_isr_context_t), GFP_KERNEL);

        isr_ctx->qid = qid;
        isr_ctx->group_idx = group_idx;
        rxq_cfg.isr_priv = (long)isr_ctx;
    }
    else
    {
        kfree((gdx_rx_isr_context_t *)rxq_cfg.isr_priv);
        rxq_cfg.isr_priv = 0;
    }

    rxq_cfg.size = qsize ? GDX_CPU_RX_QUEUE_SIZE : 0;
    rxq_cfg.rx_isr = qsize ? _gdx_hwacc_dev_rx_isr_callback : 0;
    rxq_cfg.ring_head = ring_base;
    rxq_cfg.ic_cfg.ic_enable = qsize ? true : false;
    rxq_cfg.ic_cfg.ic_timeout_us = GDX_INTERRUPT_COALESCING_TIMEOUT_US;
    rxq_cfg.ic_cfg.ic_max_pktcnt = GDX_INTERRUPT_COALESCING_MAX_PKT_CNT;
    rxq_cfg.rxq_stat = NULL;
    rc = rdpa_cpu_rxq_cfg_set(rdpa_cpu_obj, qid, &rxq_cfg);

    if (qsize)
        _gdx_hwacc_config_tc_to_queue(group_idx, qid);

unlock_exit:
    bdmf_put(rdpa_cpu_obj);
    return rc;
}

/****************************************************************************/
/** Name: _gdx_hwacc_free_work_pending                                     **/
/**                                                                        **/
/** Description: Check if the available skb index count falls below a      **/                                                               
/**              threshold                                                 **/
/**                                                                        **/
/** Input: group_idx - group to be searched                                **/
/**                                                                        **/
/** Output: 1 - Free work pending, 0 - No free work                        **/
/****************************************************************************/
static inline int _gdx_hwacc_free_work_pending(gdx_dev_group_t *dev_group_p)
{
    gdx_skb_idx_info_t *skb_idx_info_p = GDX_GET_SKB_IDX_INFO_PTR(dev_group_p->group_idx);

    if (skb_idx_info_p->avail_skb_idx_count < 
            (skb_idx_info_p->max_skb_idx_count - gdx_skb_idx_used_thresh))
    {
        return 1;
    }

    return 0;
}

/****************************************************************************/
/** Name: _gdx_wakeup_rx_thread                                            **/
/**                                                                        **/
/** Description: If the free skb_idx work is not processed and the # of    **/
/**   available skb_idx falls below a threshold then wake up the rx_thread **/
/**                                                                        **/
/** Input: dev_group_p   - device group to be searched                     **/
/**                                                                        **/
/** Output: None                                                           **/
/****************************************************************************/
static inline void _gdx_wakeup_rx_thread(gdx_dev_group_t *dev_group_p)
{
    /* try to wake up the rx_thread task to free skb_idx */
    if (_gdx_hwacc_free_work_pending(dev_group_p))
    {
        GDX_WAKEUP_RXWORKER(dev_group_p->group_idx);
    }
}

/****************************************************************************/
/** Name: _gdx_hwacc_free_rx_error_skb_idx_and_get_skb                     **/
/**                                                                        **/
/** Description: This function will free the skb_idx for the HW CPU TX     **/
/**   error case. skb_idx is moved from alloc_list to the end of free_list.**/
/**   Note: gdx_hwacc_priv.skb_idx_lock should be taken by the caller                **/
/**                                                                        **/
/** Input: skb_idx_info_p- skb_idx_info for the dev group                  **/
/**        skb_idx       - skb_idx to be freed                             **/
/**                                                                        **/
/** Output: skb          - skb stored at skb_idx entry                     **/
/****************************************************************************/
static inline struct sk_buff *_gdx_hwacc_free_rx_error_skb_idx_and_get_skb(
        gdx_skb_idx_info_t *skb_idx_info_p, int skb_idx)
{
    struct sk_buff *skb = NULL;
    gdx_skb_idx_node_t *skb_idx_node_p = &skb_idx_info_p->skb_idx_pool[skb_idx];

    GDX_ASSERT((skb_idx >= 0) && (skb_idx < GDX_MAX_SKB_IDX_COUNT));

    if ((skb_idx_info_p->sw_status[skb_idx] == GDX_SKB_STATUS_IN_USE) &&
            (skb_idx_info_p->hw_status_p[skb_idx] == GDX_SKB_STATUS_IN_USE))
    {
        if (skb_idx_info_p->hw_status_p[skb_idx] == GDX_SKB_STATUS_IN_USE)
        {
            skb_idx_info_p->skb_idx_free_count_rx_error++;
            skb = skb_idx_info_p->skb_tbl[skb_idx];

            GDX_PRINT_DBG("skb_idx<%d> skb<0x%px>", skb_idx, skb);
            skb_idx_info_p->hw_status_p[skb_idx] = GDX_SKB_STATUS_FREE;
            skb_idx_info_p->sw_status[skb_idx] = GDX_SKB_STATUS_FREE;
            skb_idx_info_p->skb_tbl[skb_idx] = NULL;

            list_del_init(&skb_idx_node_p->lnode);
            list_add_tail(&skb_idx_node_p->lnode, &skb_idx_info_p->free_list);
            skb_idx_info_p->avail_skb_idx_count++;
        }
    }
    return skb;
}

/****************************************************************************/
/** Name: _gdx_hwacc_alloc_skb_idx                                         **/
/**                                                                        **/
/** Description: First tries to free the skb_idx already freed by HW       **/
/**   accelerator. Then gets the skb_idx from the head of the free list.   **/
/**   and initializes the sw and hw status                                 **/
/**   Note: gdx_hwacc_priv.skb_idx_lock should be taken by the caller                **/
/**                                                                        **/
/** Input:                                                                 **/
/**   skb_idx_info_p- skb_idx_info for the dev group                       **/
/**   skb           - Received skb                                         **/
/**                                                                        **/
/** Output:                                                                **/
/**   skb_idx       - Allocated skb_idx                                    **/
/****************************************************************************/
static inline int _gdx_hwacc_alloc_skb_idx(gdx_skb_idx_info_t *skb_idx_info_p, struct sk_buff *skb)
{
    int skb_idx;
	gdx_skb_idx_node_t *skb_idx_node_p;

    GDX_PRINT_DBG("pkt skb<0x%px>", skb);

	skb_idx_node_p = list_first_entry_or_null(&skb_idx_info_p->free_list,
            gdx_skb_idx_node_t, lnode);
	if (skb_idx_node_p)
		list_del_init(&skb_idx_node_p->lnode);
	else
	{
        GDX_PRINT_INFO("No more free skb_idx = %d", skb_idx);
        skb_idx = GDX_ERR_NO_SKB_IDX;
        goto gdx_alloc_skb_idx_free_skb;
	}

	skb_idx = skb_idx_node_p->idx;

    GDX_ASSERT((skb_idx >= 0) && (skb_idx < GDX_MAX_SKB_IDX_COUNT));

    GDX_PRINT_DBG("skb_idx<%d>", skb_idx);
    skb_idx_info_p->skb_tbl[skb_idx] = skb;

    skb_idx_info_p->hw_status_p[skb_idx] = GDX_SKB_STATUS_IN_USE;
    skb_idx_info_p->sw_status[skb_idx] = GDX_SKB_STATUS_IN_USE;
    skb_idx_info_p->avail_skb_idx_count--;
    skb_idx_info_p->skb_idx_alloc_count++;

	list_add_tail(&skb_idx_node_p->lnode, &skb_idx_info_p->alloc_list);
    GDX_PRINT_DBG1("skb_idx<%d> avail_skb_idx_count<%d>", skb_idx,
            skb_idx_info_p->avail_skb_idx_count);
    return skb_idx;
   
gdx_alloc_skb_idx_free_skb:
	dev_kfree_skb_thread(skb);

    return skb_idx;
}

/****************************************************************************/
/** Name: _gdx_hwacc_runner_tx                                             **/
/**                                                                        **/
/** Description: Transmit the loopback skb                                 **/
/**   Assumption skb and skb->dev have been validated by caller            **/
/**                                                                        **/
/** Input: dev_group_p   - device group to be searched                     **/
/**                                                                        **/
/** Output: None                                                           **/
/****************************************************************************/
inline int _gdx_hwacc_runner_tx(struct sk_buff *skb, bool l3_packet)
{ 
    rdpa_cpu_tx_info_t info = {};
    int group_idx;
    int dev_idx;
    int skb_idx;
    gdx_dev_group_t *dev_group_p;
    gdx_dev_stats_t *dev_stats_p;
    gdx_skb_idx_info_t *skb_idx_info_p;
    int rc;

    GDX_PRINT_DBG("dev<%s> skb<0x%px>", skb->dev->name, skb);
    if (gdx_get_group_idx_and_dev_idx(skb->dev, &group_idx, &dev_idx) != GDX_SUCCESS)
    {
        dev_group_p = &gdx_dev_groups[group_idx];
        spin_lock_bh(&gdx_hwacc_priv.skb_idx_lock);
        dev_group_p->dev_rx_no_dev++;
        spin_unlock_bh(&gdx_hwacc_priv.skb_idx_lock);
        GDX_PRINT_ERROR("no matching GDX intf skb<0x%px> dev<%s>", skb, skb->dev->name);
        return GDX_FAILURE;
    }

    dev_group_p = &gdx_dev_groups[group_idx];
    dev_stats_p = &(gdx_gendev_info_pp[dev_idx]->dev_stats);
    skb_idx_info_p = GDX_GET_SKB_IDX_INFO_PTR(group_idx);

    info.port_obj = gdx_hwacc_priv.hw_specific_data_p;
    info.cpu_port = rdpa_cpu_gdx + group_idx;
    info.method = rdpa_cpu_tx_ingress;
    info.is_gdx_rx = 1;
    info.bits.do_not_recycle = 1;   /* recycle will be done by gdx */
    
    /* CAUTION: other packet type is not supported */
    info.l3_packet = l3_packet;

    /* Note: lock is protecting the dev_group stats, skb_idx allocation and the CPU-TX queue */
    spin_lock_bh(&gdx_hwacc_priv.skb_idx_lock);

    dev_group_p->dev_rx_total_pkts++;
    skb_idx = _gdx_hwacc_alloc_skb_idx(skb_idx_info_p, skb);
    if (skb_idx < 0)
    {
        skb_idx_info_p->rx_no_skb_idx++;

        GDX_PRINT_INFO("skb/skb_idx allocation failure skb<0x%px> dev<%s>",
                skb, skb->dev->name);
        spin_unlock_bh(&gdx_hwacc_priv.skb_idx_lock);
        return GDX_FAILURE;
    }
    dev_group_p->dev_rx_pkts++;
    spin_unlock_bh(&gdx_hwacc_priv.skb_idx_lock);

    info.rx_skb_idx = skb_idx;
    dev_stats_p->dev_rx_pkts++;

    GDX_PRINT_INFO("CPU_TX: info: port<%p> cpu_port<%d> gdx_pd_data/rx_skb_idx<%d>"
           " method<%d> is_gdx_rx<%d> l3_packet<%d>, dev<%s> skb<0x%px>", 
            info.port_obj, info.cpu_port, info.rx_skb_idx, info.method, info.is_gdx_rx,
            info.l3_packet, skb->dev->name, skb);
    gdx_pkt_dump("gdx_runner_tx: ", SKBUFF_2_PNBUFF(skb));
    rc = rdpa_cpu_send_sysb(skb, &info);

    if (rc)
    {
        spin_lock_bh(&gdx_hwacc_priv.skb_idx_lock);
        _gdx_hwacc_free_rx_error_skb_idx_and_get_skb(skb_idx_info_p, skb_idx);
        dev_group_p->dev_rx_error++;
        spin_unlock_bh(&gdx_hwacc_priv.skb_idx_lock);

        GDX_PRINT_DBG("CPU_TX: error<%d>", rc);
        dev_kfree_skb_thread(skb);
    }

    _gdx_wakeup_rx_thread(dev_group_p);
    return rc;
}

/****************************************************************************/
/** Name: _gdx_hwacc_skb_idx_file_show_proc                                **/
/**                                                                        **/
/** Description: proc file read handler. Called when someone reads proc    **/
/**              command using: cat /proc/gdx/hwacc                        **/
/**                                                                        **/
/** Input: m - Sequence file handle                                        **/
/**                                                                        **/
/** Output:                                                                **/
/****************************************************************************/
static inline int _gdx_hwacc_skb_idx_file_show_proc(struct seq_file *m, void *v)
{
    int skb_idx;
    int skb_idx_node_print_count = 0;
    int max_print_count = 16; // TODO may need to be fixed
    gdx_dev_group_t *dev_group_p = &gdx_dev_groups[0]; // TODO needs to be fixed for multiple dev_group

	if (dev_group_p->is_valid)
    {
        gdx_skb_idx_info_t *skb_idx_info_p = GDX_GET_SKB_IDX_INFO_PTR(dev_group_p->group_idx);

        spin_lock_bh(&gdx_hwacc_priv.skb_idx_lock);

        seq_printf(m, "\nGDX skb idx info");
        seq_printf(m, "\nmax_skb_idx_count          = %u", skb_idx_info_p->max_skb_idx_count);
        seq_printf(m, "\navail_skb_idx_count        = %u", skb_idx_info_p->avail_skb_idx_count);
        seq_printf(m, "\nskb_idx_alloc_count        = %u", skb_idx_info_p->skb_idx_alloc_count);
        seq_printf(m, "\nskb_idx_free_count_rx_miss = %u", skb_idx_info_p->skb_idx_free_count_rx_miss);
        seq_printf(m, "\nskb_idx_free_count_rx_drop = %u", skb_idx_info_p->skb_idx_free_count_rx_drop);
        seq_printf(m, "\nskb_idx_free_count_rx_hit  = %u", skb_idx_info_p->skb_idx_free_count_rx_hit);
        seq_printf(m, "\nskb_idx_free_count_rx_error= %u", skb_idx_info_p->skb_idx_free_count_rx_error);
        seq_printf(m, "\nrx_no_skb_idx              = %u", skb_idx_info_p->rx_no_skb_idx);

        for (skb_idx = 0; (skb_idx < GDX_MAX_SKB_IDX_COUNT); skb_idx++)
        {
            if (skb_idx_info_p->sw_status[skb_idx] == GDX_SKB_STATUS_IN_USE)
            {
                if (skb_idx_node_print_count < max_print_count)
                {
                    seq_printf(m, "\n[%3d] sw_status = %d hw_status = %d skb = 0x%px", skb_idx,
                       skb_idx_info_p->sw_status[skb_idx], skb_idx_info_p->hw_status_p[skb_idx],
                       skb_idx_info_p->skb_tbl[skb_idx]);
                }
                skb_idx_node_print_count++;
            }
        }

        if (skb_idx_node_print_count >= max_print_count)
            seq_printf(m, "\n ... More skb_idx pending (total %d)\n", skb_idx_node_print_count);
     
        seq_printf(m, "\n===========\n");

        spin_unlock_bh(&gdx_hwacc_priv.skb_idx_lock);
    }

    return 0;
}

/****************************************************************************/
/** Name: _gdx_hwacc_netdev_to_rdpa_port_obj                               **/
/**                                                                        **/
/** Description: The function finds the RDPA port object for the given     **/
/**              device                                                    **/
/**                                                                        **/
/** Input: dev - device pointer                                            **/
/**                                                                        **/
/** Output: info_out - Return pointer with RDPA port object                **/
/****************************************************************************/
static inline int _gdx_hwacc_netdev_to_rdpa_port_obj(struct net_device *dev, 
                                                     bcm_netdev_priv_info_out_t *info_out)
{
    int group_idx;
    int dev_idx;
    gdx_dev_group_t *dev_group_p;

    if (gdx_get_group_idx_and_dev_idx(dev, &group_idx, &dev_idx) == GDX_SUCCESS)
    {
        dev_group_p = &gdx_dev_groups[group_idx];
        info_out->bcm_netdev_to_rdpa_port_obj.rdpa_port_obj = gdx_hwacc_priv.hw_specific_data_p;
        return 0;
    }

    GDX_PRINT_ERROR("no matching GDX intf dev<%s>", dev->name);
    return GDX_FAILURE;
}


static inline void _gdx_hwacc_flush_cpu_rx_queue(gdx_dev_group_t *dev_group_p, unsigned long qid)
{
    struct sk_buff *skb;
    gdx_hwacc_rx_info_t hwacc_rx_info;
    gdx_hwacc_rx_info_t *info = &hwacc_rx_info;
    int rc = GDX_SUCCESS;

    while (1)
    {
        rc = gdx_hwacc_pkt_recv(dev_group_p->group_idx, qid, info);
        if (unlikely(rc == GDX_ERR_NO_MORE))
           return;

        dev_group_p->cpu_rxq_total_pkts++;
        GDX_PRINT_DBG("CPU_RX: info: is_exception<%d> gdx_pd_data/skb_idx_dest_ifid<%d>"
               " data<0x%px> offset<%d> size<%d>", 
                info->is_exception, info->skb_idx_dest_ifid, info->data,
                info->data_offset, info->size); 

        if (info->is_exception)
        {
            dev_group_p->cpu_rxq_lpbk_pkts++;
            spin_lock_bh(&gdx_hwacc_priv.skb_idx_lock);
            skb = gdx_hwacc_get_lpbk_skb(dev_group_p, info);
            spin_unlock_bh(&gdx_hwacc_priv.skb_idx_lock);

            dev_kfree_skb_thread(skb);
            continue;
        }

        /* Free data buffer */
        gdx_hwacc_databuf_free((uint8_t *)info->data, 0);
    }
}

static inline int _gdx_instantiate_runner_hwacc(gdx_dev_group_t *dev_group_p, int group_idx, const char *gdx_dev_name)
{
    int min_q_idx;
    int max_q_idx;
    int qid;
    int qidx; 
    int failed_qidx;
    int rc = GDX_SUCCESS;

    if ((rc = _gdx_hwacc_rdpa_init(group_idx, gdx_dev_name)) != GDX_SUCCESS)
        return rc;

    /* Get Min & Max Q Indices for this GDX Instance */
    min_q_idx = _gdx_hwacc_get_min_q_idx(group_idx);
    if (min_q_idx == GDX_FAILURE)
        return GDX_FAILURE;
    max_q_idx = _gdx_hwacc_get_max_q_idx(group_idx);

    if (max_q_idx < GDX_NUM_QUEUE_SUPPORTED)
    {
        for (qidx = min_q_idx; qidx <= max_q_idx; qidx++)
        {
            qid = gdx_hwacc_get_qid(qidx);

            if ((rc = _gdx_hwacc_config_rx_q(group_idx, qid, GDX_CPU_RX_QUEUE_SIZE)) != 0)
            {
                failed_qidx = qidx;
                GDX_PRINT_ERROR("Cannot configure GENDEV CPU Rx queue (%d), status (%d)", qid, rc);
                goto gdx_hwacc_config_rx_q_err;
            }

            gdx_hwacc_priv.queue_mask |= (1 << qidx);
        }
    }
    GDX_PRINT("\033[1m\033[34m GDX: group_idx %d"
            "min_q_id/max_q_id (%d/%d), status (%d) qmask 0x%x\033[0m",
            group_idx,
            gdx_hwacc_get_qid(min_q_idx), gdx_hwacc_get_qid(max_q_idx), rc,
            gdx_hwacc_priv.queue_mask);

    return rc;

gdx_hwacc_config_rx_q_err:
    /* free the gendev cpu rx queue(s); disable the interrupt(s) */
    for (qidx = min_q_idx; qidx < failed_qidx; qidx++) 
    {
        /* Deconfigure GDX CPU RX queue(s) */
        qid = gdx_hwacc_get_qid(qidx);
        _gdx_hwacc_config_rx_q(group_idx, qid, 0);
        gdx_hwacc_priv.queue_mask &= ~(1 << qidx);
        rdpa_cpu_int_disable(rdpa_cpu_gdx + group_idx, qid);
    }
    return rc;
}

static inline void _gdx_uninstantiate_runner_hwacc(gdx_dev_group_t *dev_group_p)
{
    int qidx;
    int min_q_idx;
    int max_q_idx;
    int qid;

    /* Get Min & Max Q Indices for this GDX Instance */
    min_q_idx = _gdx_hwacc_get_min_q_idx(dev_group_p->group_idx);
    if (min_q_idx == GDX_FAILURE)
        return;
    max_q_idx = _gdx_hwacc_get_max_q_idx(dev_group_p->group_idx);

    /* free the gendev cpu rx queue(s); disable the interrupt(s) */
    for (qidx = min_q_idx; qidx <= max_q_idx; qidx++) 
    {
        /* Deconfigure GDX CPU RX queue(s) */
        qid = gdx_hwacc_get_qid(qidx);
        _gdx_hwacc_config_rx_q(dev_group_p->group_idx, qid, 0);
        gdx_hwacc_priv.queue_mask &= ~(1 << qidx);
        rdpa_cpu_int_disable(rdpa_cpu_gdx + dev_group_p->group_idx, qid);
    }

    _gdx_hwacc_rdpa_uninit(dev_group_p->group_idx);

}

/* ****** Hardware Accelerator Supporting functions - END****** */


/* ****** Mandatory Functions - Continued *******
 * These are the functions that a hardware accelerator MUST
 * define in order to integrate with GDX*/

static inline int gdx_hwacc_get_qid(int qidx)
{
    return qidx;
}

/****************************************************************************/
/** Name: gdx_hwacc_get_dev_idx_from_rx_info                               **/
/**                                                                        **/
/** Description: Finds the matching dev_idx for the skb_idx.               **/
/**                                                                        **/
/** Input: dev_group_p - device group to be searched                       **/
/**        skb_idx     - matched skb_idx                                   **/
/**                                                                        **/
/** Output: dev_idx       - match dev_idx for skb_idx                      **/
/****************************************************************************/
static inline int gdx_hwacc_get_dev_idx_from_rx_info(gdx_dev_group_t *dev_group_p, 
                                                     gdx_hwacc_rx_info_t *info)
{
    struct net_device *dev_p;
    gdx_skb_idx_info_t *skb_idx_info_p = GDX_GET_SKB_IDX_INFO_PTR(dev_group_p->group_idx);

    if ((skb_idx_info_p->sw_status[info->rx_skb_idx] == GDX_SKB_STATUS_IN_USE) && 
            (skb_idx_info_p->hw_status_p[info->rx_skb_idx] == GDX_SKB_STATUS_IN_USE))
    {
        dev_p = _gdx_hwacc_get_dev_from_skb_idx(skb_idx_info_p, info->rx_skb_idx);
        if (dev_p == NULL)
        {
            GDX_PRINT_DBG("NULL dev pointer <0x%px>: search group_idx<%d> skb_idx<%d>",
                    dev_p, dev_group_p->group_idx, info->rx_skb_idx);
            return GDX_FAILURE;
        }

        return gdx_get_dev_idx_from_dev(dev_group_p, dev_p);
    }

    GDX_PRINT_DBG("Invalid: skb_idx<%d> status", info->rx_skb_idx);
    GDX_PRINT_DBG("[%d] sw_status = %d hw_status = %d skb = 0x%px", info->rx_skb_idx,
             skb_idx_info_p->sw_status[info->rx_skb_idx],
             skb_idx_info_p->hw_status_p[info->rx_skb_idx],
             skb_idx_info_p->skb_tbl[info->rx_skb_idx]);
    return GDX_FAILURE;
}

/****************************************************************************/
/** Name: gdx_hwacc_queue_not_empty                                        **/
/**                                                                        **/
/** Description: This function will free the skb_idx for the HW flow miss  **/
/**   case. skb_idx is moved from alloc_list to the end of free_list.      **/
/**   Note: gdx_hwacc_priv.skb_idx_lock should be taken by the caller                **/
/**                                                                        **/
/** Input: dev_group_p   - dev group                                       **/
/**        skb_idx       - skb_idx to be freed                             **/
/**                                                                        **/
/** Output: skb          - skb stored at skb_idx entry                     **/
/****************************************************************************/
static inline int gdx_hwacc_queue_not_empty(int group_idx, long qid)
{
    return rdpa_cpu_queue_not_empty(rdpa_cpu_gdx + group_idx, qid);
}

/****************************************************************************/
/** Name: gdx_hwacc_int_enable                                             **/
/**                                                                        **/
/** Description: Enable interrupt for qid                                  **/
/**                                                                        **/
/** Input: group_idx   - dev group index                                   **/
/**        qid         - queue id                                          **/
/**                                                                        **/
/** Output: None                                                           **/
/****************************************************************************/
static inline void gdx_hwacc_int_enable(int group_idx, long qid)
{
    rdpa_cpu_int_enable(rdpa_cpu_gdx + group_idx, qid);
}

/****************************************************************************/
/** Name: gdx_hwacc_flush_queues                                           **/
/**                                                                        **/
/** Description: Flushes all the dev group GDX CPU RX queues, frees skbs   **/
/**              and skb_idx                                               **/
/**                                                                        **/
/** Input: group_idx - group index                                         **/
/**                                                                        **/
/** Output: GDX_SUCCESS - if flushing and freeing was done                 **/
/**         GDX_FAILURE - if there was some issue in flushing and freeing  **/
/****************************************************************************/
static inline int gdx_hwacc_flush_queues(int group_idx)
{
    int qidx, qid;
    int min_q_idx;
    int max_q_idx;
    gdx_dev_group_t *dev_group_p;
    gdx_skb_idx_info_t *skb_idx_info_p;

    dev_group_p = &gdx_dev_groups[group_idx];
    skb_idx_info_p = GDX_GET_SKB_IDX_INFO_PTR(group_idx);
    if (dev_group_p->is_valid == 0)
    {
        GDX_PRINT_ERROR("group_idx %d is not initialized", group_idx);
        return GDX_FAILURE;
    }

    /* Get Min & Max Q Indices for this GDX Instance */
    min_q_idx = _gdx_hwacc_get_min_q_idx(group_idx);
    if (min_q_idx == GDX_FAILURE)
        return GDX_FAILURE; 

    max_q_idx = _gdx_hwacc_get_max_q_idx(group_idx);

    /* free the gendev cpu rx queue(s); disable the interrupt(s) */
    for (qidx = min_q_idx; qidx <= max_q_idx; qidx++) 
    {
        /* Deconfigure GDX CPU RX queue(s) */
        qid = gdx_hwacc_get_qid(qidx);
        _gdx_hwacc_flush_cpu_rx_queue(dev_group_p, qid);
    }

    _gdx_hwacc_free_hit_or_drop_skb_idx(dev_group_p, skb_idx_info_p, GDX_MAX_SKB_IDX_COUNT);
    if (_gdx_hwacc_free_skb_idx_alloc_list(dev_group_p) != GDX_SUCCESS)
    {
        GDX_PRINT_ERROR("Error skb idx alloc list free failure");
        return GDX_FAILURE;
    }

    /* Has all the skb and skb_idx been freed? */
    if (skb_idx_info_p->avail_skb_idx_count != skb_idx_info_p->max_skb_idx_count)
    {
        GDX_PRINT_ERROR("Error: avail_skb_idx_count<%u> != max_skb_idx_count<%u>",
            skb_idx_info_p->avail_skb_idx_count, skb_idx_info_p->max_skb_idx_count);
        return GDX_FAILURE;
    }

    return GDX_SUCCESS;
}

static inline pNBuff_t gdx_single_packet_read_and_handle(gdx_dev_group_t *dev_group_p,
       unsigned long qid, int *rc, gdx_hwacc_rx_info_t *info)
{
    struct sk_buff *skb = NULL;
    FkBuff_t *fkb = NULL;
    pNBuff_t nbuff_p = NULL;
    gdx_gendev_info_t *gendev_info_p;
    uint32_t prepend_size = 0;
    bcmFun_t *gdx_prepend_fill_info_fn = bcmFun_get(BCM_FUN_ID_PREPEND_FILL_INFO_FROM_BUF);
    int dev_idx;
    *rc = GDX_SUCCESS;

    if (unlikely(*rc = gdx_hwacc_pkt_recv(dev_group_p->group_idx, qid, info)))
       return NULL;

    dev_group_p->cpu_rxq_total_pkts++;
    GDX_PRINT_INFO("CPU_RX: info: is_exception<%d> gdx_pd_data<%d>"
           " data<0x%px> offset<%d> size<%d>", 
            info->is_exception, info->gdx_pd_data, info->data,
            info->data_offset, info->size); 

    if (info->is_exception)
    {
        dev_group_p->cpu_rxq_lpbk_pkts++;
        dev_idx = gdx_hwacc_get_dev_idx_from_rx_info(dev_group_p, info);
        gendev_info_p = gdx_gendev_info_pp[dev_idx];

        spin_lock_bh(&gdx_hwacc_priv.skb_idx_lock);
        skb = gdx_hwacc_get_lpbk_skb(dev_group_p, info);
        spin_unlock_bh(&gdx_hwacc_priv.skb_idx_lock);

        if ((dev_idx < 0))
        {
            dev_group_p->cpu_rxq_lpbk_no_dev++;
            goto gdx_single_packet_read_and_handle_err_no_rx_dev;
        }

        gendev_info_p->dev_stats.cpu_rxq_lpbk_pkts++;

        if (unlikely(!skb))
        {
            GDX_INCR_QUEUE_STATS(qid, cpu_rxq_no_skbs);
            goto gdx_single_packet_read_and_handle_err;
        }

        gdx_exception_packet_handle(skb, info);

        /* free the data buffer in exception (HW flow miss) case */
        gdx_hwacc_databuf_free(info->data, 0);
        return NULL;
    }
    GDX_PRINT_INFO("gdx_pd_data:%d len:%d\n",info.gdx_pd_data, info.size);

    dev_idx = gdx_get_dev_idx_from_gdx_dev_id(info->dest_ifid);
    if (dev_idx < 0)
    {
        dev_group_p->cpu_rxq_tx_no_dev++;
        *rc = GDX_ERR_INV_DEV_IDX;
        goto gdx_single_packet_read_and_handle_err;
    }

    info->dev_idx = dev_idx;
    /* get dev_p from devid framework */
    info->tx_dev = bcm_get_netdev_by_id_nohold(info->dest_ifid); 
    if (!gdx_is_dev_valid(info->tx_dev))
    {
        GDX_PRINT_DBG1("NULL tx_dev<0x%px> dev_idx %d", info->tx_dev, dev_idx);

        dev_group_p->cpu_rxq_tx_no_dev++; // Increment group stats
        *rc = GDX_ERR_INV_DEV;
        goto gdx_single_packet_read_and_handle_err;
    }

    GDX_PRINT_INFO("TX DEV:%s",bcm_get_netdev_name_by_id(info.gdx_pd_data));
    if (gdx_prepend_fill_info_fn != NULL)
    {
        g_fill_info.prepend_data = info->data + info->data_offset;
        prepend_size = gdx_prepend_fill_info_fn(&g_fill_info);
        if (prepend_size == (uint32_t)-1)
            goto gdx_single_packet_read_and_handle_err_no_rx_dev;
    }
    else
    {
        GDX_PRINT_ERROR("gdx_prepend_fill_info_fn NULL\n");
        goto gdx_single_packet_read_and_handle_err;
    }
    GDX_PRINT_INFO("prepend_size:%d\n",prepend_size);

    if (is_netdev_accel_tx_fkb(info->tx_dev))
    {
        fkb = gdx_fkb_alloc(info, prepend_size);
        if (unlikely(!fkb))
            goto gdx_single_packet_read_and_handle_err_no_mem;
        nbuff_p = FKBUFF_2_PNBUFF(fkb);
    }
    else
    {
        skb = gdx_skb_alloc(info, prepend_size);
        if (unlikely(!skb))
            goto gdx_single_packet_read_and_handle_err_no_mem;
        nbuff_p = SKBUFF_2_PNBUFF(skb);
    }

    GDX_PRINT_DBG("mark %lu priority %d", g_fill_info.prep_info.mark, g_fill_info.prep_info.priority);
    nbuff_set_mark(nbuff_p, g_fill_info.prep_info.mark);
    nbuff_set_priority(nbuff_p, g_fill_info.prep_info.priority);
    return nbuff_p;
gdx_single_packet_read_and_handle_err_no_mem:
    GDX_INCR_QUEUE_STATS(qid, cpu_rxq_no_skbs);
    *rc = GDX_ERR_NOMEM;
gdx_single_packet_read_and_handle_err_no_rx_dev:
    if (likely(skb))
        dev_kfree_skb_thread(skb);

gdx_single_packet_read_and_handle_err:
    /* Free data buffer in case of failure */
    gdx_hwacc_databuf_free(info->data, 0);
    return NULL;
}

/*
 * Note that budget >= NUM_PACKETS_TO_READ_MAX
 */
static uint32_t gdx_bulk_nbuff_get(unsigned long qid, unsigned long budget, void *priv)
{
    gdx_dev_group_t *dev_group_p = (gdx_dev_group_t *)priv;
    uint32_t pkt_count = 0;
    gdx_hwacc_rx_info_t info = {};
    pNBuff_t nbuff_p;

    while (budget)
    {
        int rc;

        budget--;

        if (!(nbuff_p = gdx_single_packet_read_and_handle(dev_group_p, qid, &rc, &info)))
        {
            if (GDX_ERR_NO_MORE == rc)
                break;

            pkt_count++;
            continue; // exception handled or allocation failure
        }

        gdx_forward_packet_handle(dev_group_p, &info, nbuff_p);
        pkt_count++;
    }
    dev_group_p->cpu_rxq_valid_pkts += pkt_count;

    return pkt_count;
}


/****************************************************************************/
/**                                                                        **/
/** Name:                                                                  **/
/**                                                                        **/
/**   gdx_thread_handler                                                   **/
/**                                                                        **/
/** Title:                                                                 **/
/**                                                                        **/
/**   GDX accelerator - thread handler                                     **/
/**                                                                        **/
/** Abstract:                                                              **/
/**                                                                        **/
/**   Reads all the packets from the Rx queue and send it to the generic   **/
/**   interface.                                                           **/
/**                                                                        **/
/** Input:                                                                 **/
/**                                                                        **/
/** Output:                                                                **/
/**                                                                        **/
/**                                                                        **/
/****************************************************************************/
static int gdx_thread_handler(void *context)
{
    int group_idx = (int)(long)context;
    int rx_pktcnt = 0;
    int qid, qidx = 0;
    uint32_t qMask = 0;
    gdx_dev_group_t *dev_group_p = &gdx_dev_groups[group_idx];
    gdx_queue_stats_t *queue_stats_p;
	unsigned timeout_jiffies = msecs_to_jiffies(20); /* wake_up every 20ms */

    GDX_PRINT("GDX: Instantiating [gdx%d-thrd]", group_idx);

    while (1)
    {
        wait_event_interruptible_timeout(gdx_hwacc_priv.rx_thread_wqh,
                     gdx_hwacc_priv.rx_work_avail || _gdx_hwacc_free_work_pending(dev_group_p) ||
                     kthread_should_stop(), timeout_jiffies);

        if (kthread_should_stop())
        {
            GDX_PRINT("GDX: kthread_should_stop detected in [gdx%d-thrd]", group_idx);
            break;
        }

        qMask = gdx_hwacc_priv.rx_work_avail;
        if (qMask)
        {
            /* Read from High priority queues first if it's bit is on
               Odd bits correspond to high priority queues
               Even bits correspond to low priority queues
               Hence get the last bit set */
            qidx = __fls(qMask);
            mutex_lock(&dev_group_p->group_lock);

            while (qMask)
            {
                qid = gdx_hwacc_get_qid(qidx);
                queue_stats_p = GDX_GET_QUEUE_STATS_PTR(qid);

                /* Note: keep gdx_num_pkts_to_read <= GDX_NUM_PKTS_TO_READ_MAX */
                rx_pktcnt = gdx_bulk_nbuff_get(qid, gdx_num_pkts_to_read, dev_group_p);

                queue_stats_p->cpu_rxq_rx_pkts += rx_pktcnt;
                queue_stats_p->cpu_rxq_max_rx_pkts = queue_stats_p->cpu_rxq_max_rx_pkts > rx_pktcnt ? 
                    queue_stats_p->cpu_rxq_max_rx_pkts : rx_pktcnt;

                if (gdx_hwacc_queue_not_empty(dev_group_p->group_idx, qid))
                {
                    if (rx_pktcnt >= gdx_num_pkts_to_read)
                    {
                        gdx_hwacc_misc_processing(dev_group_p);

                        yield();
                    }
                    /* else do nothing. Queue is not empty. Do not clear
                     * work avail flag. Let the thread complete processing the
                     * rest of the packets.
                     */
                }
                else
                {
                    /* Queue is empty: no more packets,
                     * clear bit atomically and enable interrupts
                     */
                    clear_bit(qidx, &gdx_hwacc_priv.rx_work_avail);
                    gdx_hwacc_int_enable(dev_group_p->group_idx, qid);
                }

                qMask &= ~(1 << qidx);
                qidx--;
            } /*for queue*/

            mutex_unlock(&dev_group_p->group_lock);
        } /* gdx_hwacc_priv.rx_work_avail */

        bcm_fro_tcp_complete();

        gdx_hwacc_misc_processing(dev_group_p);
    }

    return 0;
}

/****************************************************************************/
/** Name: gdx_hwacc_dev_group_init                                         **/
/**                                                                        **/
/** Description: Hwaccelerator specific device group initialization        **/
/**                                                                        **/
/** Input: dev_group_p - device group pointer                              **/
/**        gdx_dev_name - GDX device name                                  **/
/**                                                                        **/
/** Output: GDX_SUCCESS - if flushing and freeing was done                 **/
/**         GDX_FAILURE - if there was some issue in flushing and freeing  **/
/****************************************************************************/
static inline int gdx_hwacc_dev_group_init(gdx_dev_group_t *dev_group_p, 
                                           const char *gdx_dev_name)
{
    int rc;
    char threadname[IFNAMSIZ]={0};
    struct task_struct *rx_thread;
    unsigned long tmp_group_idx;
    int group_idx = dev_group_p->group_idx;

    if (gdx_num_pkts_to_read > GDX_NUM_PKTS_TO_READ_MAX)
    {
        GDX_PRINT_ERROR("Invalid gdx_num_pkts_to_read %d", gdx_num_pkts_to_read);    
        return GDX_FAILURE;
    }

    if (gdx_hwacc_priv.initialized == 0)
    {
        /* Initialize GDX HWACC Priv structure */
        memset(&gdx_hwacc_priv, 0, sizeof(gdx_hwacc_priv));

        if ((rc = _gdx_instantiate_runner_hwacc(dev_group_p, group_idx, gdx_dev_name)) != GDX_SUCCESS)
        {
            GDX_PRINT_ERROR("Failed to instantiate runner hwacc for GDX");
            return rc;
        }

        if ((rc = _gdx_hwacc_alloc_skb_idx_info(dev_group_p, GDX_MAX_SKB_IDX_COUNT)) != GDX_SUCCESS)
        {
            GDX_PRINT_ERROR("Error skb idx info init failure");
            goto gdx_hwacc_alloc_skb_idx_info_err;
        }
        GDX_PRINT("\033[1m\033[34m GDX: gdx_skb_idx_used_thresh<0x%px>=%d\033[0m",
                &gdx_skb_idx_used_thresh, gdx_skb_idx_used_thresh);

        spin_lock_init(&gdx_hwacc_priv.skb_idx_lock);
        mutex_init(&dev_group_p->group_lock);
        gdx_hwacc_priv.rx_work_avail  = 0;
        sprintf(threadname,"gdx%d-thrd", group_idx);

        /* Create GDX Thread */
        init_waitqueue_head(&gdx_hwacc_priv.rx_thread_wqh);
        tmp_group_idx = group_idx;
        rx_thread = kthread_create(gdx_thread_handler, (void *)tmp_group_idx, threadname);
        if (IS_ERR(rx_thread)) 
        {
            goto gdx_hwacc_err_rx_thread;
        }

        gdx_hwacc_priv.rx_thread = rx_thread;
        wake_up_process(gdx_hwacc_priv.rx_thread);
            
        /* Enable Interrupts for all queues in the group */
        gdx_hwacc_group_int_enable(group_idx);

        GDX_PRINT("\033[1m\033[34m GDX: gdx_num_pkts_to_read<0x%px>=%d\033[0m",
                &gdx_num_pkts_to_read, gdx_num_pkts_to_read);
        GDX_PRINT("\033[1m\033[34m GDX: gdx_skb_idx_used_thresh<0x%px>=%d\033[0m",
                &gdx_skb_idx_used_thresh, gdx_skb_idx_used_thresh);
        GDX_PRINT("\033[1m\033[34m GDX: group_idx %d"
                " configured GDX thread %s\033[0m",
                group_idx,
                threadname);

        gdx_hwacc_priv.initialized = 1;
    }

    return GDX_SUCCESS;

gdx_hwacc_err_rx_thread:
    if (gdx_hwacc_priv.rx_thread)
        kthread_stop(gdx_hwacc_priv.rx_thread);

    _gdx_uninstantiate_runner_hwacc(dev_group_p);
    memset(dev_group_p, 0, sizeof(gdx_dev_group_t));

gdx_hwacc_alloc_skb_idx_info_err:
    _gdx_hwacc_free_skb_idx_alloc_list(dev_group_p);
    return rc;
}
/****************************************************************************/
/** Name: gdx_hwacc_dev_group_uninit                                       **/
/**                                                                        **/
/** Description: Hwaccelerator specific device group uninitialization      **/
/**                                                                        **/
/** Input: dev_group_p - device group pointer                              **/
/**                                                                        **/
/** Output: None                                                           **/
/****************************************************************************/
static inline void gdx_hwacc_dev_group_uninit(gdx_dev_group_t *dev_group_p)
{
    /* rx_thread should be stopped before deleting the queues */
    if (gdx_hwacc_priv.rx_thread)
        kthread_stop(gdx_hwacc_priv.rx_thread);
    
    if (gdx_hwacc_flush_queues(dev_group_p->group_idx) == GDX_FAILURE)
        return;

    _gdx_uninstantiate_runner_hwacc(dev_group_p);
}

/****************************************************************************/
/** Name: gdx_hwacc_tx                                                     **/
/**                                                                        **/
/** Description: This is the hook to be called by Linux when sending to    **/
/**              Runner Loopback                                           **/
/**                                                                        **/
/** Input: skb - skb to be transmitted                                     **/
/**              l3_packet - flag that indicates if it is a l3 packet      **/
/**                                                                        **/
/** Output:                                                                **/
/****************************************************************************/
int gdx_hwacc_tx(struct sk_buff *skb, bool l3_packet)
{
    gdx_pkt_dump("gdx_accelerator_tx: ", SKBUFF_2_PNBUFF(skb));
    return _gdx_hwacc_runner_tx(skb, l3_packet);
}

/****************************************************************************/
/** Name: gdx_hwacc_proc_init                                              **/
/**                                                                        **/
/** Description: Initializes the hw accelerator specific proc handlers     **/
/**                                                                        **/
/** Input: None                                                            **/
/**                                                                        **/
/** Output: Return value 0 if successful -1 otherwise                      **/
/****************************************************************************/
static inline int gdx_hwacc_proc_init(void)
{    
    if (!proc_create_single("skb_idx", 0644, proc_gdx_dir, _gdx_hwacc_skb_idx_file_show_proc))
        return -1;
    return 0;
}

/****************************************************************************/
/** Name: gdx_hwacc_proc_uninit                                            **/
/**                                                                        **/
/** Description: Uninitializes the hw accelerator specific proc handlers   **/
/**                                                                        **/
/** Input: dir - device pointer                                            **/
/**        info_type - Type of information requested                       **/
/**                                                                        **/
/** Output: info_out - Pointer to returned information                     **/
/****************************************************************************/
static inline void gdx_hwacc_proc_uninit(struct proc_dir_entry *dir)
{
    remove_proc_entry("skb_idx", dir);
}

/****************************************************************************/
/** Name: gdx_hwacc_priv_info_get                                          **/
/**                                                                        **/
/** Description: The function provides an interface for other kernel       **/
/**   modules to get information from this driver.                         **/
/**                                                                        **/
/** Input: dev - device pointer                                            **/
/**        info_type - Type of information requested                       **/
/**                                                                        **/
/** Output: info_out - Pointer to returned information                     **/
/****************************************************************************/
int gdx_hwacc_priv_info_get(struct net_device *dev, 
                            bcm_netdev_priv_info_type_t info_type, 
                            bcm_netdev_priv_info_out_t *info_out)
{
    int rc = -1;

    switch (info_type)
    {
    case BCM_NETDEV_TO_RDPA_PORT_OBJ:
        rc = _gdx_hwacc_netdev_to_rdpa_port_obj(dev, info_out);
        break;
    default:
        break;
    }
    return rc;
}

/****************************************************************************/
/** Name: gdx_hwacc_misc_processing                                        **/
/**                                                                        **/
/** Description: Invoked by thread handler to handle any hwaccelerator     **/
/**              specific functions                                        **/
/**                                                                        **/
/** Input: dev_group_p - group object                                      **/
/**                                                                        **/
/** Output: None                                                           **/
/****************************************************************************/
static inline int gdx_hwacc_misc_processing(gdx_dev_group_t *dev_group_p)
{
    gdx_skb_idx_info_t *skb_idx_info_p = GDX_GET_SKB_IDX_INFO_PTR(dev_group_p->group_idx);

    if (skb_idx_info_p->avail_skb_idx_count != skb_idx_info_p->max_skb_idx_count)
    {
        _gdx_hwacc_free_hit_or_drop_skb_idx(dev_group_p, skb_idx_info_p, GDX_SKB_IDX_FREE_BUDGET);
    }
    return 0;
}

/****************************************************************************/
/** Name: gdx_hwacc_group_int_enable                                       **/
/**                                                                        **/
/** Description: Enable interrupts for all queues in the group             **/
/**                                                                        **/
/** Input: group_idx - group index                                         **/
/**                                                                        **/
/** Output: None                                                           **/
/****************************************************************************/
static inline void gdx_hwacc_group_int_enable(int group_idx)
{
    int min_q_idx;
    int max_q_idx;
    int qidx;
    int qid;

    /* Get Min & Max Q Indices for this GDX Instance */
    min_q_idx = _gdx_hwacc_get_min_q_idx(group_idx);
    if (min_q_idx == GDX_FAILURE)
        return;
    max_q_idx = _gdx_hwacc_get_max_q_idx(group_idx);

    for (qidx = min_q_idx; qidx <= max_q_idx; qidx++)
    {
        qid = gdx_hwacc_get_qid(qidx);
        gdx_hwacc_int_enable(group_idx, qid); 
    } 
}

extern bcm_netdev_priv_info_get_cb_fn_t blog_gdx_dev_cb_fn;
/****************************************************************************/
/** Name: gdx_hwacc_bind                                                   **/
/**                                                                        **/
/** Description: Bind any hardware accelerator specific callbacks          **/
/**                                                                        **/
/** Input: None                                                            **/
/**                                                                        **/
/** Output: None                                                           **/
/****************************************************************************/
static inline void gdx_hwacc_bind(void)
{
    blog_gdx_dev_cb_fn = gdx_hwacc_priv_info_get;
}

/****************************************************************************/
/** Name: gdx_hwacc_bind                                                   **/
/**                                                                        **/
/** Description: Unbind any hardware accelerator specific callbacks        **/
/**                                                                        **/
/** Input: None                                                            **/
/**                                                                        **/
/** Output: None                                                           **/
/****************************************************************************/
static inline void gdx_hwacc_unbind(void)
{
    blog_gdx_dev_cb_fn = NULL;
}

/****************************************************************************/
/** Name: gdx_hwacc_recycle                                                **/
/**                                                                        **/
/** Description: Recycle the nbuff                                         **/
/**                                                                        **/
/** Input: nbuff_p - Data buffer                                           **/
/**        context - unused,for future use                                 **/
/**        flags   - indicates what to recyle                              **/
/**                                                                        **/
/** Output: None                                                           **/
/****************************************************************************/
static inline void gdx_hwacc_recycle(void *nbuff_p, unsigned long context, uint32_t flags)
{
    bdmf_sysb_recycle(nbuff_p, context, flags);
}

/* ******Mandatory Functions - END*******/
 
#endif /* __GDX_RUNNER__H__ */
