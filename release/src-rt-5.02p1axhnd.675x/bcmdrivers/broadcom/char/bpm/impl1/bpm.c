/*
<:copyright-BRCM:2009:proprietary:standard

   Copyright (c) 2009 Broadcom 
   All Rights Reserved

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
*/

/*
 *******************************************************************************
 * File Name  : bpm.c
 *
 *******************************************************************************
 */
/* -----------------------------------------------------------------------------
 *                      Global Buffer Pool Manager (BPM)
 * -----------------------------------------------------------------------------
 * When the system boots up all the buffers are owned by BPM. 
 *
 * Interface Initialization:
 * ------------------------
 * When an interface is initialized, the interface assigns buffers to
 * descriptors in it's private RX ring by requesting buffer allocation
 * from BPM (APIs: gbpm_alloc_mult_buf() or gbpm_alloc_buf()), 
 * and also informs BPM how many buffers were assigned to RX ring
 * (gbpm_resv_rx_buf()).
 *
 * Similarly, when an interface is uninitialized, the interface frees 
 * buffers from descriptors in it's private RX ring by requesting buffer 
 * free to BPM (APIs: gbpm_free_mult_buf() or gbpm_free_buf()), 
 * and also informs BPM how many buffers were freed from RX ring
 * (gbpm_unresv_rx_buf()).
 *
 * Knowing the size of RX ring allows BPM to keep track of the reserved
 * buffers (assigned to rings) and hence find out how many dynamic buffers
 * are available, which can be shared between the interfaces in the system. 
 *
 * Buffer Allocation:
 * ------------------
 * When a packet is received, the buffer is not immediately replenished 
 * into RX ring, rather a count is incremented, to keep track of how many
 * buffer allocation requests are pending. This is done to delay as much
 * as possible going to BPM because of overheads (locking, cycles, etc.),
 * and it likely that the RX ring will be soon replenished with a recycled
 * buffer.
 *
 * When an interface's RX ring buffers usage (RX DMA queue depth) exceeds
 * the configured threshold (because the earlier used buffers are not yet
 * recycled to RX ring), the interface requests BPM for allocation of
 * more buffers. The buffer request is fullfilled from the available 
 * dynamic buffers in BPM. After one or two such buffer allocation requests
 * equilibirium is established where the newly used buffers are replenished
 * with the recycled buffers.
 *
 * Buffer Free/Recycle:
 * --------------------
 *  After a packet is transmitted or dropped, the recycle function of 
 *  RX interface is invoked. Recycle function first checks whether there
 *  is a space in the RX ring. If yes, the buffer is recycled to ring.
 *  If not, buffer is freed to BPM.
 *
 *
 * FkBuff_t:
 * Even though a Buffer has a FkBuff_t at the start of a buffer, this FkBuff_t
 * structure is not initialized as in fkb_preinit(). The recycle_hook needs to
 * be appropriately setup. An FkBuff_t structure does not have a recycle_flags
 * and the recycle_context field was re-purposed. Need to undo this and use the
 * recycle context to save the &bpm_g.
 *
 *
 * TX Queue Threhsolds:
 * --------------------
 * TX Queue Thresholds are configured for the slower interfaces like XTM, 
 * MoCA, so that a lot of buffers are not held up in the TX queues of 
 * these interfaces.
 *
 * There are two TX Q thresholds per queue of an interface: low and high.
 * Only one of the low or high threshold is used to compare against the 
 * current queue depth. If the TX Q depth is lower than the compared 
 * threshold, the packet is enqueued, else the packet is dropped.
 *
 * The low or high threshold to be used is decided based on the current
 * level of dynamic buffers available in the system 
 * (API gbpm_get_dyn_buf_lvl() ). If the current dynamic buffer 
 * level is less than the configured threshold, then the low threshold 
 * is used else the high threshold is used.
 *
 *
 * Pre-allocated sk_buff Pool
 * --------------------------
 * BPM also supports a preallocated pool of sk_buffs. BPM managed sk_buffs,
 * avoid the need for a kmem skb_head_cache alloc, and reduced memsets of the
 * struct sk_buff and the skb_shared_info. Unlike skb_header_init(), where a
 * skb_shared_info may be relocated, soon after the tail, given a specified
 * tailroom, in a BPM sk_buff, the tailroom will always be beyond the DMA-able
 * region, on a cacheline aligned boundary. BPM sk_buffs may only be attached to
 * BPM buffers (or buffers that follow the bcm_pkt_lengths.h formats).
 *
 *
 * Note: API prototypes are given in gbpm.h file.
 */
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/skbuff.h>
#include <linux/nbuff.h>
#include <linux/fs.h>
#include <linux/bcm_log_mod.h>
#include <linux/bcm_log.h>
#include <linux/export.h>
#include <board.h>
#include <linux/gbpm.h>
#include <bpmctl_common.h>
#include <bcm_pkt_lengths.h>
#include <bcmPktDma_defines.h>
#include <bpm.h>
#include "bcm_prefetch.h"

extern int kerSysGetSdramSize( void );
#if !(defined(CONFIG_BCM_RDPA) && !defined(CONFIG_BCM_RDPA_MODULE))
#if defined(CONFIG_BCM_PKTDMA)    
extern int bcmPktDma_GetTotRxBds( void );
#endif
#endif

/*----- Globals -----*/

extern gbpm_t gbpm_g;

#define GBPM_NOOP()                 do { /* noop */ } while (0)

/* GBPM USER Hook Invocations by BPM */

#define GBPM_USER_INVOKE( HOOKNAME, ARG... )                                   \
({                                                                             \
    if (likely(gbpm_g.HOOKNAME != (gbpm_ ## HOOKNAME ## _hook_t)NULL) )        \
        (gbpm_g.HOOKNAME)( ARG );                                              \
})

#define GBPM_USER_ENET_STATUS()         GBPM_USER_INVOKE(enet_status)

#if defined(GBPM_FAP_SUPPORT)
#define GBPM_USER_FAP_STATUS()          GBPM_USER_INVOKE(fap_status)
#define GBPM_USER_FAP_THRESH()          GBPM_USER_INVOKE(fap_thresh)
#define GBPM_USER_ENET_THRESH()         GBPM_USER_INVOKE(enet_thresh)
#define GBPM_USER_FAP_ENET_THRESH()     GBPM_USER_INVOKE(fap_enet_thresh)
#define GBPM_USER_FAP_UPD_BUF_LVL() \
            GBPM_USER_INVOKE( fap_upd_buf_lvl, bpm_get_dyn_buf_lvl() )

#else  /* ! GBPM_FAP_SUPPORT */
#define GBPM_USER_FAP_STATUS()          GBPM_NOOP()
#define GBPM_USER_FAP_THRESH()          GBPM_NOOP()
#define GBPM_USER_ENET_THRESH()         GBPM_NOOP()
#define GBPM_USER_FAP_ENET_THRESH()     GBPM_NOOP()
#define GBPM_USER_FAP_UPD_BUF_LVL()     GBPM_NOOP()
#endif /* ! GBPM_FAP_SUPPORT */


#if defined(GBPM_XTM_SUPPORT)
#define GBPM_USER_XTM_STATUS()          GBPM_USER_INVOKE(xtm_status)
#define GBPM_USER_XTM_THRESH()          GBPM_USER_INVOKE(xtm_thresh)
#else  /* ! GBPM_XTM_SUPPORT */
#define GBPM_USER_XTM_STATUS()          GBPM_NOOP()
#define GBPM_USER_XTM_THRESH()          GBPM_NOOP()
#endif /* ! GBPM_XTM_SUPPORT */


#define GBPM_USER_STATUS()                                                     \
({ GBPM_USER_ENET_STATUS(); GBPM_USER_FAP_STATUS(); GBPM_USER_XTM_STATUS(); })

#define GBPM_USER_THRESH()                                                     \
({ GBPM_USER_FAP_THRESH(); GBPM_USER_XTM_THRESH(); })

#define GBPM_USER_UPD_BUF_LVL()                                                \
({ GBPM_USER_FAP_UPD_BUF_LVL(); })

#define BPM_DEFS()             bpm_t *bpm_pg = &bpm_g

#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)
#define BPM_LOCK               (&bpm_pg->lock)
#define BPM_FLAGS_DEF()        unsigned long _flags
#define BPM_LOCK_IRQ()         spin_lock_irqsave(BPM_LOCK, _flags)
#define BPM_UNLOCK_IRQ()       spin_unlock_irqrestore(BPM_LOCK, _flags)
#define BPM_LOCK_BH()          spin_lock_bh(BPM_LOCK)
#define BPM_UNLOCK_BH()        spin_unlock_bh(BPM_LOCK)
#else
#define BPM_FLAGS_DEF()             /* */
#define BPM_LOCK_IRQ()         local_irq_disable()
#define BPM_UNLOCK_IRQ()       local_irq_enable()
#define BPM_LOCK_BH()          local_bh_disable()
#define BPM_UNLOCK_BH()        local_bh_enable()
#endif

#if defined(CC_BPM_SKB_POOL_BUILD)
#if defined(CC_BPM_SKB_POOL_STATS)
#define BPM_SKB_POOL_STATS_ADD(u32, val)  (bpm_pg->u32 += (val))
#define BPM_SKB_POOL_STATS_SUB(u32, val)  (bpm_pg->u32 -= (val))
#define BPM_SKB_POOL_STATS_BPM_DEFS()     BPM_DEFS()
#else /* !defined(CC_BPM_SKB_POOL_STATS) */
#define BPM_SKB_POOL_STATS_ADD(u32, val)  do { } while (0)
#define BPM_SKB_POOL_STATS_SUB(u32, val)  do { } while (0)
#define BPM_SKB_POOL_STATS_BPM_DEFS()
#endif /* defined(CC_BPM_SKB_POOL_STATS) */
#endif /* CC_BPM_SKB_POOL_BUILD */

#if defined(CONFIG_BCM_BPM_BUF_TRACKING)
#define BPM_TBUF_LOCK_IRQ()         local_irq_disable()
#define BPM_TBUF_UNLOCK_IRQ()       local_irq_enable()
#define BPM_TBUF_LOCK_BH()          local_bh_disable()
#define BPM_TBUF_UNLOCK_BH()        local_bh_enable()
#define BPM_TBUF_IRQ_SAVE(f)        local_irq_save(f)
#define BPM_TBUF_IRQ_RESTORE(f)     local_irq_restore(f)
#endif

#if defined(CONFIG_BCM_BPM_BUF_TRACKING)
#define BPM_TRACKING_HDR         L1_CACHE_BYTES
#define BPM_BUF_SIZE             (BCM_PKTBUF_SIZE + BPM_TRACKING_HDR)
#define BPM_TRK_DEF_LEN          10
#define BPM_TRK_MAX_LEN          100
#else
#define BPM_BUF_SIZE               BCM_PKTBUF_SIZE
#endif

#undef BPM_DECL
#define BPM_DECL(x) #x,

const char *bpmctl_ioctl_name[] =
{
    BPM_DECL(BPMCTL_IOCTL_SYS)
    BPM_DECL(BPMCTL_IOCTL_MAX)
};

const char *bpmctl_subsys_name[] =
{
    BPM_DECL(BPMCTL_SUBSYS_STATUS)
    BPM_DECL(BPMCTL_SUBSYS_THRESH)
    BPM_DECL(BPMCTL_SUBSYS_BUFFERS)
    BPM_DECL(BPMCTL_SUBSYS_SKBUFFS)
#if defined(CONFIG_BCM_BPM_BUF_TRACKING)
    BPM_DECL(BPMCTL_SUBSYS_TRACK)
#endif
    BPM_DECL(BPMCTL_SUBSYS_MAX)
};

const char *bpmctl_op_name[] =
{   
    BPM_DECL(BPMCTL_OP_SET)
    BPM_DECL(BPMCTL_OP_GET)
    BPM_DECL(BPMCTL_OP_ADD)
    BPM_DECL(BPMCTL_OP_REM)
    BPM_DECL(BPMCTL_OP_DUMP)
    BPM_DECL(BPMCTL_OP_MAX)
};

const char *strBpmPort[] = 
{ 
    "ETH ",
    "XTM ",
    "FWD ",
    "WLAN",
    "USB ",
    "MAX "
};

#if defined(CONFIG_BCM_BPM_BUF_TRACKING)
const char * bpmtrk_drv_name[] =
{
    "BPM ",
    "ETH ",
    "XTM ",
    "KER ",
    "BDMF ",
    "ARCHER ",
    "MAX "
};
const char * bpmtrk_val_name[] =
{
    "nomrk",
    "alloc",
    "clone",
    "recyl",
    "free ",
    "rx   ",
    "tx   ",
    "enter",
    "exit ",
    "info ",
    "init ",
    "cpsrc",
    "cpdst",
    "xlate",
    "max "
};
#endif

/* Implementation notes on global instance of bpm_g.
 *
 * 1. Do not use a global pointer in bss to access global object in data.
 * 2. spinlock dynamically initialized in module_init, "after" memset.
 *    If bpm_g needs to be statically initialized as follows,
 *       = { .lock = __SPIN_LOCK_UNLOCKED(.lock), .buf_pool = NULL };
 *    ensure that memset does not re-zero-out spinlock_t lock.
 * 3. Arrange fields to avail of data locality (cacheline).
 * 4. BPM Buf Pool managed using an inverted stack, as opposed to a circular
 *    buffer, with a head, tail, last index.
 * 5. BPM buf alloc request is serviced by a pointer offseted into the buffer.
 *    All buffers will have a fixed FkBuff_t and HEADROOM. Pointer returned is
 *    hence the "data" pointer fed to DMA.
 * 6. All buffers will have a skb_shared_info at the end. Ths skb_shared_info
 *    is not to be relocated, based on the length of a received packet. 
 * 7. A BPM based SKB MUST ONLY be attached to a BPM based buffer. BPM manages
 *    sk_buffs with minimal sk_buff initializaton.
 */
struct bpm {

    spinlock_t lock;

    uint32_t buf_alloc_cnt; /* count of allocated buffers */
    uint32_t buf_free_cnt;  /* count of free buffers (avail?) */
    uint32_t buf_fail_cnt;  /* statistics of allocs requests failures */
    uint32_t min_avail_cnt; /* minimal availability that bpm reached through its lifetime */

    uint32_t buf_top_idx;   /* true index of last slot in buf_pool */
    uint32_t buf_cur_idx;   /* cur index in inverted stack for allocation */
    void **  buf_pool;      /* pool is managed as an inverted stack */

#if defined(CC_BPM_SKB_POOL_BUILD)
    struct sk_buff * skb_freelist;  /* free list of sk_buff(s) in pool */
    uint32_t skb_avail;     /* sk_buff(s) available in free list */
    uint32_t skb_total;     /* total managed by pool. (alloc = total - avail) */
    uint32_t skb_bound;     /* pool total may not extend beyond bound */
#if defined(CC_BPM_SKB_POOL_STATS)
    uint32_t skb_alloc_cnt; /* number of allocations serviced from pool */
    uint32_t skb_bpm_alloc_cnt; /* number of allocations with SKB */
    uint32_t skb_free_cnt;  /* number of recycles (frees back) to pool */
    uint32_t skb_error_cnt; /* number of errors */
    uint32_t skb_fail_cnt;  /* number of failures, not essentially errors */
    uint32_t skb_grow_cnt;  /* number of times pool was extended */
    uint32_t skb_hdr_reset_cnt;  /* number of skb hdr resets */
    uint32_t shinfo_reset_cnt;   /* number of skb_shinfo resets */
    uint32_t full_cache_inv_cnt; /* number of full buffer cache invalidates */
    uint32_t part_cache_inv_cnt; /* number of partial cache invalidates */
#endif /* CC_BPM_SKB_POOL_STATS */
#endif /* CC_BPM_SKB_POOL_BUILD */

#if defined(CONFIG_BCM_BPM_BUF_TRACKING)
    void ** sbuf_pool;
    void ** tbuf_pool;
    uint32_t tbuf_pool_ix;
    uint32_t tbuf_pool_mem;
    uint32_t last_memsz;
    uint32_t marked_cnt;
#endif
    uint32_t dyn_buf_lo_thresh;
    uint32_t max_dyn;
    void **  mem_pool;
    uint32_t mem_idx;
    uint32_t rxbds;
    uint32_t fap_resv;
    uint32_t tot_resv_buf;
    uint32_t tot_rx_ring_buf;
    uint32_t tot_alloc_trig;
    uint32_t status[GBPM_PORT_MAX][GBPM_RXCHNL_MAX];
    uint32_t num_rx_buf[GBPM_PORT_MAX][GBPM_RXCHNL_MAX];
    uint32_t alloc_trig_thresh[GBPM_PORT_MAX][GBPM_RXCHNL_MAX];

} ____cacheline_aligned;

typedef struct bpm bpm_t;

bpm_t bpm_g; /* GLOBAL Instance of a BufferPoolManager */

static uint32_t bpm_init_done_g = 0;

/* Helper macros for BPM Buf Pool management */
#define BPM_BUF_AVAIL_CNT(bpm)        ((bpm)->buf_top_idx - (bpm)->buf_cur_idx)

/* Prefetch helper macros */
#define BPM_PREFETCH()                bcm_prefetch_ld(bpm_pg)
#define BPM_BUF_POOL_PREFETCH_LD(idx) bcm_prefetch_ld(&bpm_pg->buf_pool[idx])
#define BPM_BUF_POOL_PREFETCH_ST(idx) bcm_prefetch_st(&bpm_pg->buf_pool[idx])

#if defined(CONFIG_BCM_BPM_BUF_TRACKING)
static int bpm_track_enabled_g = 0;  /* Enabled during module init */
static int bpm_trail_len_g = BPM_TRK_DEF_LEN;
extern gbpm_mark_buf_hook_t gbpm_mark_buf_hook_g;
extern gbpm_add_ref_hook_t gbpm_add_ref_hook_g;

/*
 *------------------------------------------------------------------------------
 * BPM Tracking GBPM API
 *------------------------------------------------------------------------------
 */
/*
 *------------------------------------------------------------------------------
 * function   : bpm_get_base_addr
 * description: Caculates base address given a pointer into a BPM buffer.
 *------------------------------------------------------------------------------
 */
static inline void * bpm_get_base_addr( void * buf_p )
{
    int i;
    uint32_t memsz;
    uint8_t * mem_p;
    uint8_t * test_p;
    uintptr_t diff;

    BPM_DEFS();

    if ( buf_p == NULL )
    {
        return NULL;
    }

    test_p = (uint8_t *)buf_p;
    for ( i = 0; i < bpm_pg->mem_idx; ++i )
    {
        /* Check if address is in a mem_pool range */
        mem_p = (uint8_t *)bpm_pg->mem_pool[i];
        memsz = (i == bpm_pg->mem_idx - 1)? bpm_pg->last_memsz : BPM_MAX_MEMSIZE - L1_CACHE_BYTES;
        if ( test_p >= mem_p && test_p < mem_p + memsz )
        {
            diff = (uintptr_t)(test_p - mem_p);
            diff = (diff / BPM_BUF_SIZE) * BPM_BUF_SIZE;
            return (void *)(mem_p + diff);
        }
    }
    /* Invalid address */
    return (void *)NULL;
}

static void bpm_mark_buf( void * buf_p, void * addr, int reftype, int driver, int value, int info )
{
    void * base_p;
    gbpm_trail_t * trail_p;
    gbpm_mark_t * mark_p;
    unsigned long flags;

    BPM_DEFS();
    
    base_p = bpm_get_base_addr(buf_p);

    BPM_TBUF_IRQ_SAVE(flags);

    if ( bpm_track_enabled_g )
    {

        if ( likely(base_p != NULL) )
        {
            trail_p = (gbpm_trail_t *)base_p;

            mark_p = &trail_p->mbuf_p[trail_p->write];

            if ( unlikely(trail_p->write == 0 && mark_p->value == GBPM_VAL_UNMARKED) )
            {
                bpm_pg->marked_cnt++;
            }

            trail_p->write++;
            if ( unlikely(trail_p->write >= bpm_trail_len_g) )
            {
                trail_p->write = 0;
            }

            mark_p->reftype = reftype;
            mark_p->addr = (size_t)((addr == NULL) ? buf_p : addr);
            mark_p->driver = driver;
            mark_p->info = info;
            mark_p->value = value;
        }
    }

    BPM_TBUF_IRQ_RESTORE(flags);
}

static void bpm_add_ref( void * buf_p, int i )
{
    gbpm_trail_t * trail_p;
    trail_p = (gbpm_trail_t *)bpm_get_base_addr( buf_p );
    if ( bpm_track_enabled_g && trail_p != NULL )
    {
        atomic_add( i, &trail_p->ref_cnt );
    }
}

#endif /* bpm tracking */

/*
 *------------------------------------------------------------------------------
 * function   : bpm_get_dyn_buf_lvl
 * description: finds the current dynamic buffer level is high or low.
 *------------------------------------------------------------------------------
 */
static int bpm_get_dyn_buf_lvl(void) 
{
    BPM_DEFS();

    if (BPM_BUF_AVAIL_CNT(bpm_pg) > bpm_pg->dyn_buf_lo_thresh)
        return 1;
    return 0;
}


/*
 *------------------------------------------------------------------------------
 * function   : bpm_validate_resv_rx_buf
 * description: validates the port enable
 *------------------------------------------------------------------------------
 */
static int bpm_validate_resv_rx_buf( gbpm_port_t port, uint32_t chnl, 
                                uint32_t num_rx_buf, uint32_t alloc_trig_thresh)
{
    /* validate parameters */
    if ( (port >= GBPM_PORT_MAX ) || (chnl >= GBPM_RXCHNL_MAX) )
    {
        BCM_LOG_ERROR( BCM_LOG_ID_BPM,
            "invalid port=%d or chnl=%d", port, chnl );
        return BPM_ERROR;
    }

    if ( (num_rx_buf < alloc_trig_thresh) )
    {
        BCM_LOG_ERROR( BCM_LOG_ID_BPM,
            "invalid alloc_trig_thresh=%d num_rx_buf=%d", 
            alloc_trig_thresh, num_rx_buf );
        return BPM_ERROR;
    }

    return BPM_SUCCESS;
}


/*
 *------------------------------------------------------------------------------
 * function   : bpm_upd_dyn_buf_lo_thresh
 * description: updates the dynamic buffer low threshold
 *------------------------------------------------------------------------------
 */
static void bpm_upd_dyn_buf_lo_thresh( void )
{
    BPM_DEFS();

    /* calc the low thresh for dynamic buffers in the global buffer pool */ 
    bpm_pg->dyn_buf_lo_thresh = 
       (bpm_pg->max_dyn * BPM_PCT_DYN_BUF_LO_THRESH / 100);
}


/*
 *------------------------------------------------------------------------------
 * function   : bpm_resv_rx_buf
 * description: reserves num of rxbufs and updates thresholds
 *------------------------------------------------------------------------------
 */
static int bpm_resv_rx_buf( gbpm_port_t port, uint32_t chnl,
        uint32_t num_rx_buf, uint32_t alloc_trig_thresh ) 
{
    BPM_DEFS(); BPM_FLAGS_DEF();

    /* validate parameters */
    if (bpm_validate_resv_rx_buf( port, chnl, num_rx_buf, alloc_trig_thresh )== BPM_ERROR)
        return BPM_ERROR;

    BPM_LOCK_IRQ();

    /* flag the chnl has been enabled */
    bpm_pg->status[port][chnl] = GBPM_RXCHNL_ENABLED;
    bpm_pg->num_rx_buf[port][chnl] = num_rx_buf;
    bpm_pg->alloc_trig_thresh[port][chnl] = alloc_trig_thresh;
    bpm_pg->tot_rx_ring_buf += num_rx_buf;
    bpm_pg->tot_alloc_trig += alloc_trig_thresh;
    

    BCM_LOG_DEBUG( BCM_LOG_ID_BPM, 
                "port=%d chnl=%d resv_rx_buf=%d alloc_trig_thresh=%d", 
                port, chnl, bpm_pg->num_rx_buf[port][chnl],
                bpm_pg->alloc_trig_thresh[port][chnl]);
 
    BPM_UNLOCK_IRQ();

    return BPM_SUCCESS;
}


/*
 *------------------------------------------------------------------------------
 * function   : bpm_unresv_rx_buf
 * description: unreserves the previously reserved rx bufs
 *------------------------------------------------------------------------------
 */
static int bpm_unresv_rx_buf( gbpm_port_t port, uint32_t chnl ) 
{
    BPM_DEFS();

    /* flag the chnl has been disabled */
    bpm_pg->status[port][chnl] = GBPM_RXCHNL_DISABLED;

    bpm_pg->tot_alloc_trig -= bpm_pg->alloc_trig_thresh[port][chnl];
    bpm_pg->tot_rx_ring_buf -= bpm_pg->num_rx_buf[port][chnl];

    bpm_pg->num_rx_buf[port][chnl] = 0;
    bpm_pg->alloc_trig_thresh[port][chnl] = 0;

    BCM_LOG_DEBUG( BCM_LOG_ID_BPM, "port=%d chnl=%d unresv_rx_buf=%d",
                port, chnl, bpm_pg->num_rx_buf[port][chnl] );

    return BPM_SUCCESS;
}

static inline void update_min_availability(void)
{
    BPM_DEFS();
    const uint32_t buf_avail_cnt = BPM_BUF_AVAIL_CNT(bpm_pg);
    if ( unlikely(bpm_pg->min_avail_cnt > buf_avail_cnt) )
        bpm_pg->min_avail_cnt = buf_avail_cnt;
}

/*
 *------------------------------------------------------------------------------
 * Function   : bpm_alloc_buf
 * Description: allocates a buffer from global buffer pool
 *------------------------------------------------------------------------------
 */
static void * bpm_alloc_buf( void )
{
    BPM_DEFS(); BPM_FLAGS_DEF();
    void *buf_p;
    BPM_PREFETCH();

    BPM_LOCK_IRQ();

    BPM_BUF_POOL_PREFETCH_LD(bpm_pg->buf_cur_idx);
    ++bpm_pg->buf_alloc_cnt;

    /* if buffers available in global buffer pool */
    if ( likely(bpm_pg->buf_cur_idx < bpm_pg->buf_top_idx) )
    {
        buf_p = bpm_pg->buf_pool[bpm_pg->buf_cur_idx++]; // POST ++
        update_min_availability();
        GBPM_USER_UPD_BUF_LVL();
    }
    else
    {
        buf_p = NULL;
        --bpm_pg->buf_alloc_cnt;
        ++bpm_pg->buf_fail_cnt;
    }

    BPM_UNLOCK_IRQ();

    return buf_p;
}

/*
 *------------------------------------------------------------------------------
 * Function   : bpm_free_buf
 * Description: frees a buffer to global buffer pool
 *------------------------------------------------------------------------------
 */
static void bpm_free_buf( void * buf_p )
{
    BPM_DEFS(); BPM_FLAGS_DEF();
    BPM_PREFETCH();

    if ( unlikely(buf_p == NULL) ) {
        ++bpm_pg->buf_fail_cnt;
        printk("bpm free NULL buf_p\n");
        BCM_ASSERT( buf_p != NULL );
        return;
    }

    BPM_LOCK_IRQ();

    BPM_BUF_POOL_PREFETCH_ST(bpm_pg->buf_cur_idx - 1);
    ++bpm_pg->buf_free_cnt;

    BPM_DBG_ASSERT(bpm_pg->buf_cur_idx != 0U);
    bpm_pg->buf_pool[--bpm_pg->buf_cur_idx] = buf_p; // PRE --

    GBPM_USER_UPD_BUF_LVL();

    BPM_UNLOCK_IRQ();
}

#define BPM_LOW_ALLOC_THRESH    (1024)

/*
 *------------------------------------------------------------------------------
 * Function   : bpm_alloc_mult_buf_ex
 * Description: allocates a buffer from global buffer pool with reservation priority
 *------------------------------------------------------------------------------
 */
static int bpm_alloc_mult_buf_ex( uint32_t num, void ** buf_p , uint32_t prio)
{
    int ret;
    uint32_t buf_avail_cnt;

    BPM_DEFS(); BPM_FLAGS_DEF();
    BPM_PREFETCH();

    ret = BPM_SUCCESS;

    BCM_ASSERT( buf_p != NULL );

    BPM_LOCK_IRQ();


    BPM_BUF_POOL_PREFETCH_LD(bpm_pg->buf_cur_idx);

    buf_avail_cnt = BPM_BUF_AVAIL_CNT(bpm_pg); /* avail = top - cur */

    if ((buf_avail_cnt < num) ||
        (((buf_avail_cnt  - num) < BPM_LOW_ALLOC_THRESH) && (prio != BPM_HIGH_PRIO_ALLOC)))
    {
        bpm_pg->buf_fail_cnt++;
        ret = BPM_ERROR;
        goto exit_func;
    }
    bpm_pg->buf_alloc_cnt += num;

    /* buffers available in global buffer pool */
    if ( likely(buf_avail_cnt >= num) )
    {
        uint32_t buf_idx;

        /* do we insert this in loop ...? */
        BPM_BUF_POOL_PREFETCH_LD(bpm_pg->buf_cur_idx +
            BCM_DCACHE_LINE_LEN / sizeof(void*) );

        for ( buf_idx = 0; buf_idx < num; ++buf_idx )
        {
            buf_p[buf_idx] = bpm_pg->buf_pool[bpm_pg->buf_cur_idx++]; // POST ++
        }

        update_min_availability();
        GBPM_USER_UPD_BUF_LVL();
    }
    else
    {
        ++bpm_pg->buf_fail_cnt;
        bpm_pg->buf_alloc_cnt -= num;
        ret = BPM_ERROR;
    }
exit_func:
    BPM_UNLOCK_IRQ();

    return ret;
}

static  int bpm_alloc_mult_buf( uint32_t num, void ** buf_p)
{
    return (bpm_alloc_mult_buf_ex( num, buf_p, BPM_LOW_PRIO_ALLOC));
}

/*
 *------------------------------------------------------------------------------
 * Function   : bpm_free_mult_buf
 * Description: frees a buffer to global buffer pool
 *------------------------------------------------------------------------------
 */
static void bpm_free_mult_buf( uint32_t num, void ** buf_p )
{
    BPM_DEFS(); BPM_FLAGS_DEF();
    BPM_PREFETCH();

    BCM_ASSERT( buf_p != NULL );

    BPM_LOCK_IRQ();

    BPM_BUF_POOL_PREFETCH_ST(bpm_pg->buf_cur_idx - 1);

    BPM_DBG_ASSERT(bpm_pg->buf_cur_idx >= num);

    bpm_pg->buf_free_cnt += num;

    if ( likely(bpm_pg->buf_cur_idx >= num) )
    {
        uint32_t buf_idx;

        for ( buf_idx = 0; buf_idx < num; ++buf_idx )
        {
            bpm_pg->buf_pool[--bpm_pg->buf_cur_idx] = buf_p[buf_idx]; // PRE --
        }

        GBPM_USER_UPD_BUF_LVL();
    }
    else
    {
        BPM_DBG_ASSERT(bpm_pg->buf_cur_idx > num);

        bpm_pg->buf_free_cnt -= num;
    } 

    BPM_UNLOCK_IRQ();
}

/*
 *------------------------------------------------------------------------------
 * Function   : bpm_alloc_buf_mem
 * Description: Allocate a large memory chunk for carving out RX buffers
 *  The memory allocated is reset and flushed. A pointer to a cache aligned
 *  address of the requested size is returned. A pointer to the allocated
 *  memory is saved.
 *------------------------------------------------------------------------------
 */
static void *bpm_alloc_buf_mem( size_t memsz )
{
    BPM_DEFS();
    void *mem_p;

    BCM_LOG_FUNC( BCM_LOG_ID_BPM );

    memsz += L1_CACHE_BYTES;

    if ( bpm_pg->mem_idx >= BPM_MAX_MEM_POOL_IX )
    {
        BCM_LOG_ERROR( BCM_LOG_ID_BPM, 
                "too many memory pools %d", bpm_pg->mem_idx );
        return NULL;
    }

#if defined(CONFIG_BCM94908) && defined(CONFIG_BCM_HND_EAP)
    /* Due to a RDP CPU_TX limitation that can only support <1GB address
     * space buffer, a workaround is implemented to support 1GB+ address
     * space buffer.  However, this workaround has a performance issue.
     * We address it by making sure BPM always allocate buffer from <1GB address
     * space by using GFP_DMA which is set to 1GB boundary, so drivers
     * using BPM buffer will not be affected by the workaround. */
    if ( (mem_p = kmalloc( memsz, GFP_ATOMIC | GFP_DMA ) ) == NULL )
#else
    if ( (mem_p = kmalloc( memsz, GFP_ATOMIC ) ) == NULL )
#endif
    {
        BCM_LOG_ERROR( BCM_LOG_ID_BPM, "kmalloc %d failure", (int)memsz );
        return NULL;
    }

    /* Future kfree */
    bpm_pg->mem_pool[bpm_pg->mem_idx] = mem_p;
    bpm_pg->mem_idx++;

    memset( mem_p, 0, memsz );
    cache_flush_len( mem_p, memsz );                  /* Flush invalidate */

    mem_p = (void *)L1_CACHE_ALIGN((uintptr_t)mem_p );  /* L1 cache aligned */

    return mem_p;
}


#if defined(CONFIG_BCM_BPM_BUF_TRACKING)
/*
 *------------------------------------------------------------------------------
 * BPM Tracking Userspace Utility Functions
 *------------------------------------------------------------------------------
 */
static inline void __bpm_print_bytes( uint8_t * data_p, int cnt, int flip_endian )
{
    int i = 0;
    int j;
    int init = 0;
    int done = sizeof(void *);
    int inc = 1;

    if ( flip_endian )
    {
        init = sizeof(void *) - 1;
        done = -1;
        inc = -1;
    }

    while ( cnt )
    {
        if ( i % (sizeof(void *) * 4) == 0 )
            printk("\n[%p] ", &data_p[i]);

        for ( j=init; j!=done; j+=inc )
            printk("%02x", data_p[i+j]);
        printk(" ");

        cnt -= sizeof(void *);
        i += sizeof(void *);
    }

    printk("\n");
}

static inline void __bpm_print_mark( gbpm_mark_t * mark_p )
{
    printk(" [");
    if (mark_p->driver >= 0 && mark_p->driver < GBPM_DRV_MAX)
        printk( "%s:", bpmtrk_drv_name[mark_p->driver]);
    else
        printk( "%s:", bpmtrk_drv_name[GBPM_DRV_MAX]);

    if (mark_p->value >= 0 && mark_p->value < GBPM_VAL_MAX)
        printk( " %s", bpmtrk_val_name[mark_p->value]);
    else
        printk( " %s", bpmtrk_val_name[GBPM_VAL_MAX]);

    printk( ": %03u]", mark_p->info );
}

static inline void __bpm_print_trail( gbpm_trail_t * trail_p, void * base_p )
{
    int i, j;
    gbpm_mark_t * mark_p;
    size_t curr_addr = 0;

    for ( i = 0; i < bpm_trail_len_g; i++ )
    {
        j = (trail_p->write - i) % bpm_trail_len_g;
        mark_p = &trail_p->mbuf_p[j];

        if ( mark_p->value != GBPM_VAL_UNMARKED )
        {
            if ( mark_p->addr != curr_addr )
            {
                printk("\n    ");

                if ( mark_p->reftype == GBPM_REF_BUFF )
                    printk("buff");
                else if ( mark_p->reftype == GBPM_REF_FKB )
                    printk(" fkb");
                else if ( mark_p->reftype == GBPM_REF_SKB )
                    printk( " skb" );
                else
                    printk( "addr" );

                printk( " [%p] : ", (void *)(size_t)mark_p->addr );
                curr_addr = mark_p->addr;
            }

            __bpm_print_mark( mark_p );
        }
    }

    printk("\n\n");
}

static inline void __bpm_print_filters( bpmctl_track_t * trk_p )
{
    if ( BPMCTL_TRK_GET(trk_p->filters, BPMCTL_TRK_IDLE) ||
         BPMCTL_TRK_GET(trk_p->filters, BPMCTL_TRK_IDLEMIN) ||
         BPMCTL_TRK_GET(trk_p->filters, BPMCTL_TRK_REF) ||
         BPMCTL_TRK_GET(trk_p->filters, BPMCTL_TRK_REFMIN) ||
         BPMCTL_TRK_GET(trk_p->filters, BPMCTL_TRK_BASE) )
        printk( " Buffer with:\n" );

    if ( BPMCTL_TRK_GET(trk_p->filters, BPMCTL_TRK_IDLE) )
        printk( "   idle cnt == (%u) \n", trk_p->idle );

    if ( BPMCTL_TRK_GET(trk_p->filters, BPMCTL_TRK_IDLEMIN) )
        printk( "   idle cnt >= (%u) \n", trk_p->idle_min );

    if ( BPMCTL_TRK_GET(trk_p->filters, BPMCTL_TRK_REF) )
        printk( "   ref cnt == (%u) \n", trk_p->ref );

    if ( BPMCTL_TRK_GET(trk_p->filters, BPMCTL_TRK_REFMIN) )
        printk( "   ref cnt >= (%u) \n", trk_p->ref_min );

    if ( BPMCTL_TRK_GET(trk_p->filters, BPMCTL_TRK_BASE) )
        printk( "   base address [%p]\n", (void *)(size_t)trk_p->base );

    if ( BPMCTL_TRK_GET(trk_p->filters, BPMCTL_TRK_BASE) ||
         BPMCTL_TRK_GET(trk_p->filters, BPMCTL_TRK_ADDR) ||
         BPMCTL_TRK_GET(trk_p->filters, BPMCTL_TRK_DRIVER) ||
         BPMCTL_TRK_GET(trk_p->filters, BPMCTL_TRK_VALUE) ||
         BPMCTL_TRK_GET(trk_p->filters, BPMCTL_TRK_INFO) )
        printk( " Has mark with:\n");

    if ( BPMCTL_TRK_GET(trk_p->filters, BPMCTL_TRK_BASE) )
        printk( "   addr -> base buffer [%p]\n", (void *)(size_t)trk_p->base );

    if ( BPMCTL_TRK_GET(trk_p->filters, BPMCTL_TRK_ADDR) )
        printk( "   addr == [%p] \n", (void *)(size_t)trk_p->addr );

    if ( BPMCTL_TRK_GET(trk_p->filters, BPMCTL_TRK_DRIVER) )
        printk( "   driver == %s \n", bpmtrk_drv_name[trk_p->driver] );

    if ( BPMCTL_TRK_GET(trk_p->filters, BPMCTL_TRK_VALUE) )
        printk( "   value == %s \n", bpmtrk_val_name[trk_p->value] );

    if ( BPMCTL_TRK_GET(trk_p->filters, BPMCTL_TRK_INFO) )
        printk( "   info == (%u) \n", trk_p->info );
}

static inline int __bpm_filter_trail( gbpm_trail_t * trail_p, bpmctl_track_t * trk_p )
{
    int i, filters;
    gbpm_mark_t * mark_p;
    void * test_p;

    filters = trk_p->filters;

    if ( BPMCTL_TRK_GET(filters, BPMCTL_TRK_IDLE) && atomic_read( &trail_p->idle_cnt ) != trk_p->idle )
        goto nomatch;

    if ( BPMCTL_TRK_GET(filters, BPMCTL_TRK_IDLEMIN) && atomic_read( &trail_p->idle_cnt ) < trk_p->idle_min )
        goto nomatch;

    if ( BPMCTL_TRK_GET(filters, BPMCTL_TRK_REF) && atomic_read( &trail_p->ref_cnt ) != trk_p->ref )
        goto nomatch;

    if ( BPMCTL_TRK_GET(filters, BPMCTL_TRK_REFMIN) && atomic_read( &trail_p->ref_cnt ) < trk_p->ref_min )
        goto nomatch;

    if ( BPMCTL_TRK_GET(filters, BPMCTL_TRK_BASE) && trail_p == (void *)(size_t)trk_p->base )
        filters &= ~BPMCTL_TRK_BASE;

    /* Filter on marks */
    if ( BPMCTL_TRK_GET(filters, BPMCTL_TRK_BASE | BPMCTL_TRK_ADDR | BPMCTL_TRK_DRIVER |
                                        BPMCTL_TRK_VALUE | BPMCTL_TRK_INFO)  )
    {
        mark_p = trail_p->mbuf_p;
        i = bpm_trail_len_g;
        while ( i )
        {
            if ( mark_p->value != GBPM_VAL_UNMARKED )
            {
                if ( BPMCTL_TRK_GET(filters, BPMCTL_TRK_BASE) )
                {
                    test_p = bpm_get_base_addr( (void *)mark_p->addr );
                    if ( test_p != (void *)(size_t)trk_p->base )
                        goto iterate;
                }

                if ( BPMCTL_TRK_GET(filters, BPMCTL_TRK_ADDR) && mark_p->addr != trk_p->addr )
                    goto iterate;

                if ( BPMCTL_TRK_GET(filters, BPMCTL_TRK_DRIVER) && mark_p->driver != trk_p->driver )
                    goto iterate;

                if ( BPMCTL_TRK_GET(filters, BPMCTL_TRK_VALUE) && mark_p->value != trk_p->value )
                    goto iterate;

                if ( BPMCTL_TRK_GET(filters, BPMCTL_TRK_INFO) && mark_p->info != trk_p->info )
                    goto iterate;

                break;
            }
        iterate:
            mark_p++;
            i--;
        }
        if ( i <= 0 )
            goto nomatch;
    }

    return BPMCTL_TRK_MATCH;

nomatch:
    return BPMCTL_TRK_NOMATCH;
}

/*
 *------------------------------------------------------------------------------
 * Function   : bpm_dump_trails
 * Description: function handler for dumping the trails
 *------------------------------------------------------------------------------
 */
static void bpm_dump_trails( bpmctl_track_t * trk_p )
{
    int i, cnt = 0;
    gbpm_trail_t * trail_p;
    BPM_DEFS();
    unsigned long flags;

    if ( !bpm_track_enabled_g )
    {
        printk("\n BPM tracking is disabled!\n\n");
        return;
    }

    if ( BPMCTL_TRK_GET(trk_p->filters, BPMCTL_TRK_BASE) && trk_p->base == 0 )
    {
        goto out;
    }

    for ( i = 0; i < bpm_pg->buf_top_idx; i++ )
    {
        if ( bpm_track_enabled_g )
        {
            BPM_TBUF_IRQ_SAVE(flags);

            trail_p = (gbpm_trail_t *)bpm_pg->sbuf_pool[i];
            if ( trail_p->mbuf_p->value != GBPM_VAL_UNMARKED &&
                 __bpm_filter_trail( trail_p, trk_p ) == BPMCTL_TRK_MATCH )
            {
                printk( " [%p]  idle_cnt (%u)  ref_cnt (%u)",
                        bpm_pg->sbuf_pool[i], atomic_read( &trail_p->idle_cnt ),
                        atomic_read( &trail_p->ref_cnt ));
                __bpm_print_trail( trail_p, bpm_pg->sbuf_pool[i] );
                cnt++;
            }

            BPM_TBUF_IRQ_RESTORE(flags);
        }
    }

out:
    printk( "\n Found (%u) trails matching:\n", cnt );
    __bpm_print_filters( trk_p );
    printk( "\n" );

    return;
}

/*
 *------------------------------------------------------------------------------
 * Function   : bpm_dump_static_buffers
 * Description: function handler for dumping all BPM buffers.
 *------------------------------------------------------------------------------
 */
static void bpm_dump_static_buffers( bpmctl_track_t * trk_p )
{
    int i, cnt = 0;
    gbpm_trail_t * trail_p;
    unsigned long flags;

    BPM_DEFS();

    if ( !BPMCTL_TRK_GET(trk_p->filters, BPMCTL_TRK_BASE) || trk_p->base != 0 )
    {
        for ( i = 0; i < bpm_pg->buf_top_idx; i++ )
        {
            if ( bpm_track_enabled_g )
            {
                BPM_TBUF_IRQ_SAVE(flags);

                trail_p = (gbpm_trail_t *)bpm_pg->sbuf_pool[i];

                if ( __bpm_filter_trail( trail_p, trk_p ) == BPMCTL_TRK_MATCH )
                {
                    if ( cnt % 8 == 0 )
                    {
                        printk( "\n [%05u]", cnt );
                    }
                    cnt++;
                    printk( " %p", trail_p );
                }

                BPM_TBUF_IRQ_RESTORE(flags);
            }
        }
    }

    printk( "\n\n Found (%u) buffers matching:\n", cnt );
    __bpm_print_filters( trk_p );
    printk( "\n" );

    return;
}

/*
 *------------------------------------------------------------------------------
 * Function   : bpm_inc_trails
 * Description: function handler for incrementing idle_cnt on trails.
 *------------------------------------------------------------------------------
 */
static void bpm_inc_trails( bpmctl_track_t * trk_p )
{
    int i, cnt = 0;
    gbpm_trail_t * trail_p;
    unsigned long flags;
    
    BPM_DEFS();

    if ( !BPMCTL_TRK_GET(trk_p->filters, BPMCTL_TRK_BASE) || trk_p->base != 0 )
    {
        for ( i = 0; i < bpm_pg->buf_top_idx; i++ )
        {
            if ( bpm_track_enabled_g )
            {
                BPM_TBUF_IRQ_SAVE(flags);

                trail_p = (gbpm_trail_t *)bpm_pg->sbuf_pool[i];
                if ( trail_p->mbuf_p->value != GBPM_VAL_UNMARKED &&
                     __bpm_filter_trail( trail_p, trk_p ) == BPMCTL_TRK_MATCH )
                {
                    atomic_inc( &trail_p->idle_cnt );
                    cnt++;
                }

                BPM_TBUF_IRQ_RESTORE(flags);
            }
        }
    }

    printk( "\n Incremented idle cnt on (%u) trails matching:\n", cnt );
    __bpm_print_filters( trk_p );
    printk( "\n" );

    return;
}

/*
 *------------------------------------------------------------------------------
 * Function   : bpm_dump_buffer
 * Description: function handler for dumping a BPM buffer or nbuff
 *------------------------------------------------------------------------------
 */
static int bpm_dump_buffer( bpmctl_track_t * trk_p )
{
    int i, j, found;
    unsigned int cnt = 0;
    uint8_t * data_p = NULL;
    gbpm_trail_t * trail_p;
    BPM_DEFS();
    unsigned long flags;
    

    if ( trk_p->reftype == BPMCTL_REF_BUFF || trk_p->reftype == BPMCTL_REF_ANY )
    {
        data_p = bpm_get_base_addr( (void *)(size_t)trk_p->addr );
    }

    if ( data_p != NULL )
    {
        data_p += BPM_TRACKING_HDR;
        cnt = BCM_PKTBUF_SIZE;
    }
    else
    {
        if ( trk_p->reftype == BPMCTL_REF_BUFF )
        {
            printk("\n buff [%p] not found.\n",
                   (void *)(size_t)trk_p->addr);
            return BPM_SUCCESS;
        }

        for ( i = 0; i < bpm_pg->buf_top_idx && !cnt; i++ )
        {
            if ( bpm_track_enabled_g )
            {
                BPM_TBUF_IRQ_SAVE(flags);

                trail_p = (gbpm_trail_t *)bpm_pg->sbuf_pool[i];
                for ( j = 0; j <= bpm_trail_len_g && !found; j++ )
                {
                    if ( trail_p->mbuf_p[j].value != GBPM_VAL_UNMARKED &&
                         trail_p->mbuf_p[j].addr == trk_p->addr )
                    {
                        data_p = (uint8_t *)(size_t)trk_p->addr;

                        switch ( trk_p->reftype )
                        {
                        case ( BPMCTL_REF_ANY ):
                        case ( BPMCTL_REF_SKB ):
                            if ( trail_p->mbuf_p[j].reftype == GBPM_REF_SKB )
                            {
                                cnt = sizeof(struct sk_buff);
                            }
                        case ( BPMCTL_REF_FKB ):
                            if ( trail_p->mbuf_p[j].reftype == GBPM_REF_FKB )
                            {
                                cnt = sizeof(FkBuff_t);
                            }
                        case ( BPMCTL_REF_BUFF ):
                            break;
                        }
                    }
                }

                BPM_TBUF_IRQ_RESTORE(flags);
            }
        }
        if ( cnt == 0 )
        {
            printk("\n nbuff [%p] not found. Can only dump tracked nbuff\n",
                   (void *)(size_t)trk_p->addr);
            return BPM_SUCCESS;
        }
    }

    __bpm_print_bytes( data_p, cnt, trk_p->flip_endian );
    printk("\n");

    return BPM_SUCCESS;
}

/*
 *------------------------------------------------------------------------------
 * Function   : bpm_dump_track_status
 * Description: function handler for dumping BPM tracking status.
 *------------------------------------------------------------------------------
 */
static void bpm_dump_track_status(void)
{
    BPM_DEFS();
    printk( "\n------------------- BPM Tracking Status -------------------\n" );
    printk( " enabled  trail_len  marked_trails  total_trails\n" );
    printk( " %7u  %10u  %13u  %12u\n",
            bpm_track_enabled_g, bpm_trail_len_g, bpm_pg->marked_cnt,
            bpm_pg->buf_top_idx);
    printk( "\n-----------------------------------------------------------\n" );
    printk( " sbuff_mem  trail_buff_pool_mem  tracking_header_sz\n" );
    printk( " %7zuKB  %17uKB  %18u\n",
            (bpm_pg->buf_top_idx * sizeof(size_t)) >> 10,
            bpm_pg->tbuf_pool_mem >> 10, BPM_TRACKING_HDR);
}

/*
 *------------------------------------------------------------------------------
 * Function   : bpm_tbuf_pool_alloc
 * Description: Allocate a large memory chunk for carving out trail buffers
 *  The memory allocated is reset and flushed. A pointer to a cache aligned
 *  address of the requested size is returned. A pointer to the allocated
 *  pool is saved.
 *
 *  PS. Although kmalloc guarantees L1_CACHE_ALIGN, we do not assume so.
 *  Based on bpm_buf_mem_alloc.
 *------------------------------------------------------------------------------
 */
static void * bpm_tbuf_pool_alloc( size_t memsz )
{
    void *mem_p;

    BPM_DEFS();
    BCM_LOG_FUNC( BCM_LOG_ID_BPM );

    memsz += L1_CACHE_BYTES;

    if ( (mem_p = kmalloc( memsz, GFP_ATOMIC ) ) == NULL )
    {
        BCM_LOG_ERROR( BCM_LOG_ID_BPM, "kmalloc %d failure", (int)memsz );
        return NULL;
    }

    /* Future kfree */
    bpm_pg->tbuf_pool[bpm_pg->tbuf_pool_ix] = mem_p;
    bpm_pg->tbuf_pool_ix++;
    bpm_pg->tbuf_pool_mem += memsz;

    memset( mem_p, 0, memsz );
    cache_flush_len( mem_p, memsz );                  /* Flush invalidate */

    mem_p = (void *)L1_CACHE_ALIGN((uintptr_t)mem_p );  /* L1 cache aligned */

    return mem_p;
}

/*
 *------------------------------------------------------------------------------
 * Function   : bpm_init_trail_bufs
 * Description: Initialize trail buffers.
 *  Pointers to pre-allocated trail buffers are saved. See bpm_tbuf_pool_alloc()
 *  Based on bpm_init_buf_pool.
 *------------------------------------------------------------------------------
 */
static int bpm_init_trail_bufs( uint32_t num, uint32_t tbuf_sz )
{
    void *mem_p;
    uint8_t *buf_p;
    uint32_t tbufs_per_pool, memsz, i, tbuf_ix;
    gbpm_trail_t * trail_p;

    BPM_DEFS();
    bpm_pg->tbuf_pool_ix = 0; /* Index into tentry pools allocated */
    bpm_pg->tbuf_pool_mem = 0;
    tbuf_ix = 0;

    /* Allocate chunks of memory, carve trail buffers */
    tbufs_per_pool = (BPM_MAX_MEMSIZE - L1_CACHE_BYTES) / tbuf_sz;
    while ( num )
    {
        uint8_t *data_p;

        /* Chunk size */
        tbufs_per_pool = (tbufs_per_pool < num) ? tbufs_per_pool : num;
        memsz = tbufs_per_pool * tbuf_sz;

        if( (mem_p = bpm_tbuf_pool_alloc( memsz ) ) == NULL) 
            return BPM_ERROR;

        BCM_LOG_DEBUG( BCM_LOG_ID_BPM,
            "allocated %4u %8s @ mem_p<%p> memsz<%06u>\n",
                    tbufs_per_pool, "TrailBufs", (void *)mem_p, memsz );

        buf_p = (uint8_t *)mem_p;                   /* Trail entries are cached */

        for ( i=0; i < tbufs_per_pool; i++, buf_p += tbuf_sz )
        {
            /* L1 cache aligned */
            data_p = (void *)L1_CACHE_ALIGN( (uintptr_t)buf_p );
            trail_p = (gbpm_trail_t *)bpm_pg->sbuf_pool[tbuf_ix++];
            trail_p->mbuf_p = (gbpm_mark_t *) data_p;
        }
        num -= tbufs_per_pool;
    }

    return BPM_SUCCESS;
}

/*
 *------------------------------------------------------------------------------
 * Function   : bpm_track_enable
 * Description: function handler for enabling BPM buffer tracking feature.
 *------------------------------------------------------------------------------
 */
static int bpm_track_enable( uint32_t trail_len )
{
    uint32_t tbuf_sz;
    uint32_t tbuf_align_sz;
    uint32_t tbuf_pool_ix;
    uint32_t tbuf_pool_sz;
    void * tbuf_pool_p = NULL;
    unsigned long flags;

    BPM_DEFS();

    BCM_LOG_FUNC( BCM_LOG_ID_BPM );

    BPM_TBUF_IRQ_SAVE(flags);

    if (bpm_track_enabled_g)
    {
        printk("\n BPM tracking is already enabled!\n");
        bpm_dump_track_status();
        BPM_TBUF_IRQ_RESTORE(flags);
        return BPM_ERROR;
    }

    /* If no length is given, preserve previous settings */
    if ( trail_len > 0 )
    {
        bpm_trail_len_g = trail_len;
        if ( trail_len > BPM_TRK_MAX_LEN )
        {
            bpm_trail_len_g = BPM_TRK_MAX_LEN;
        }
    }

    /* Calculate mark buffer size */
    tbuf_sz = sizeof(gbpm_mark_t) * bpm_trail_len_g;
    tbuf_align_sz = L1_CACHE_ALIGN(tbuf_sz);

    /* Increase trail length to match L1 cache aligned trail entry */
    bpm_trail_len_g = bpm_trail_len_g + ((tbuf_align_sz - tbuf_sz) / sizeof(gbpm_mark_t));
    printk("Added (%zu) to trail length using (%u) excess bytes\n",
           (tbuf_align_sz - tbuf_sz) / sizeof(gbpm_mark_t), (tbuf_align_sz - tbuf_sz));

    /* Calculate size of references to memory pools */
    tbuf_pool_ix = (bpm_pg->buf_top_idx * tbuf_align_sz/BPM_MAX_MEMSIZE) + 1;
    tbuf_pool_sz = tbuf_pool_ix * sizeof(size_t);

    if ( (tbuf_pool_p = kmalloc( tbuf_pool_sz, GFP_ATOMIC ) ) == NULL )
    {
        BCM_LOG_ERROR( BCM_LOG_ID_BPM, 
            "kmalloc %d failure for tbuf_pool_p", tbuf_pool_sz);
        BPM_TBUF_IRQ_RESTORE(flags);
        return BPM_ERROR;
    }

    bpm_pg->tbuf_pool = (void **) tbuf_pool_p;
    bpm_pg->tbuf_pool_mem += tbuf_pool_sz;

    printk("About to init %u trails of length %d size %uB, total %uKB\n",
           bpm_pg->buf_top_idx, bpm_trail_len_g, tbuf_align_sz, (tbuf_align_sz * bpm_pg->buf_top_idx) >> 10);
    if ( bpm_init_trail_bufs( bpm_pg->buf_top_idx, tbuf_align_sz ) == BPM_ERROR )
    {
        BCM_LOG_ERROR( BCM_LOG_ID_BPM, 
            "kmalloc %d failure for tbuf mem pools", tbuf_align_sz * bpm_pg->buf_top_idx);
        BPM_TBUF_IRQ_RESTORE(flags);
        return BPM_ERROR;
    }

    bpm_track_enabled_g = 1;

    printk("\n BPM tracking enabled\n");
    bpm_dump_track_status();

    BPM_TBUF_IRQ_RESTORE(flags);

    return BPM_SUCCESS;
}

/*
 *------------------------------------------------------------------------------
 * Function   : bpm_track_disable
 * Description: function handler for disabling BPM buffer tracking feature.
 *------------------------------------------------------------------------------
 */
static int bpm_track_disable(void)
{
    void *data_p;
    gbpm_trail_t * trail_p;
    uint32_t i ;
    unsigned long flags;
    
    BPM_DEFS();

    BCM_LOG_FUNC( BCM_LOG_ID_BPM );

    BPM_TBUF_IRQ_SAVE(flags);

    if ( !bpm_track_enabled_g )
    {
        BPM_TBUF_IRQ_RESTORE(flags);
        printk("\n BPM tracking is already disabled!\n");
        return BPM_ERROR;
    }

    bpm_track_enabled_g = 0;

    /*
     * Release all trail buffer pools allocated
     */
    for ( i=0; i<bpm_pg->tbuf_pool_ix; i++ )
    {
        data_p = bpm_pg->tbuf_pool[ i ];
        if ( data_p )
        {
            kfree( data_p );
        }
    }
    printk(" Released (%u) trail buffer pools\n", bpm_pg->tbuf_pool_ix);
    printk(" About to free trail buffer pool array <%p>\n", bpm_pg->tbuf_pool);
    kfree(bpm_pg->tbuf_pool);
    bpm_pg->tbuf_pool = NULL;
    printk(" Released trail buffer pool array\n");

    /* Clear trail buffer reference in tracking headers */
    for ( i=0; i<bpm_pg->buf_top_idx; i++ )
    {
        trail_p = (gbpm_trail_t *) bpm_pg->sbuf_pool[i];
        trail_p->mbuf_p = NULL;
    }
    printk(" Cleared buffer trail heads\n");

    /* Clear stats and flags */
    bpm_pg->tbuf_pool_ix = 0;
    bpm_pg->tbuf_pool_mem = 0;
    bpm_pg->last_memsz = 0;
    bpm_pg->marked_cnt = 0;

    BPM_TBUF_IRQ_RESTORE(flags);

    printk("\n BPM tracking disabled\n");
    return BPM_SUCCESS;
}

#endif /* bpm tracking */

/*
 *------------------------------------------------------------------------------
 * Function   : bpm_init_buf_pool
 * Description: Initialize buffer pool, with num buffers
 *  Pointers to pre-allocated memory pools are saved. See bpm_alloc_buf_mem()
 *
 *  Buffers in buf_pool are managed in an inverted stack, with buf_cur_idx
 *  moving from 0 to buf_top_idx-1. buf_cur_idx will always point to the next
 *  buffer available for allocation. A buffer may be freed by decrementing the
 *  buf_cur_idx (stack grows down). A stack algo is used to reduce the cache
 *  working set and wraparound handling of a <head,tail> index walking the
 *  entire buf_pool with wraparound.
 *------------------------------------------------------------------------------
 */
static int bpm_init_buf_pool( uint32_t tot_num_buf )
{
    void *mem_p;
    uint8_t *buf_p;
    uint32_t bufs_per_mem, memsz, i, num;
    BPM_DEFS(); BPM_FLAGS_DEF();

    BPM_LOCK_IRQ();

    num = tot_num_buf;

    bpm_pg->mem_idx = 0; /* Index into memory pools allocated */
    bpm_pg->buf_top_idx = 0;
    bpm_pg->min_avail_cnt = tot_num_buf;

    /* Allocate chunks of memory, carve buffers */
    bufs_per_mem = (BPM_MAX_MEMSIZE - L1_CACHE_BYTES) / BPM_BUF_SIZE;

    while ( num )
    {
        uint8_t *base_p;   /* Start of a buffer where FkBuff_t resides */
        uint8_t *data_p;   /* Start of "data" buffer. Skip FkBuff, headroom */
        uintptr_t shinfo_p; /* Start of skb_shared_info in buffer */
        

        /* Chunk size */
        bufs_per_mem = (bufs_per_mem < num) ? bufs_per_mem : num;
        memsz = bufs_per_mem * BPM_BUF_SIZE;

        if( (mem_p = bpm_alloc_buf_mem( memsz ) ) == NULL) 
            return BPM_ERROR;

        buf_p = (uint8_t *)mem_p;                   /* Buffers are cached */

        for ( i=0; i < bufs_per_mem; i++, buf_p += BPM_BUF_SIZE )
        {
            /* L1 cache aligned */
            base_p = (uint8_t *)L1_CACHE_ALIGN( (uintptr_t)buf_p );

            data_p = BPM_PFKBUFF_TO_BUF(base_p);

#if defined(CONFIG_BCM_BPM_BUF_TRACKING)
            data_p += BPM_TRACKING_HDR;
            bpm_pg->last_memsz = memsz;
#endif

            /* skb_shared_info must be cacheline aligned */
            shinfo_p = (uintptr_t)BPM_BUF_TO_PSHINFO(data_p);

            BPM_DBG_ASSERT( IS_ALIGNED(shinfo_p, L1_CACHE_BYTES) );

            /* Save the data_p and not the base_p in the buf_pool[] table */
            bpm_pg->buf_pool[bpm_pg->buf_top_idx++] = (void *) data_p;

            buf_p = base_p;
        }

        /* BCM_DBG_ASSERT that buf_p after loop <= (mem_p + memsz) */

        BCM_LOG_DEBUG( BCM_LOG_ID_BPM, 
            "allocated %4u %8s @ mem_p<%p> memsz<%06u>",
                    bufs_per_mem, "RxBufs", (void *)mem_p, memsz );

        num -= bufs_per_mem;

    }   /* while num */

    BCM_LOG_DEBUG(BCM_LOG_ID_BPM,"tot_num_buf:%u buf_top_idx:%u mem_idx:%u",tot_num_buf,bpm_pg->buf_top_idx,bpm_pg->mem_idx);

#if defined(CONFIG_BCM_BPM_BUF_TRACKING)
    /* Set up static buffer pool */
    memsz = sizeof(size_t) * bpm_pg->buf_top_idx;
    if( (bpm_pg->sbuf_pool = kmalloc( memsz, GFP_ATOMIC )) == NULL)
        return BPM_ERROR;

    for ( i=0; i < bpm_pg->buf_top_idx; i++ )
        bpm_pg->sbuf_pool[i] = BPM_BUF_TO_PFKBUFF(bpm_pg->buf_pool[i] - BPM_TRACKING_HDR);

    BCM_LOG_DEBUG( BCM_LOG_ID_BPM,
        "allocated static buf pool %u memsz %uB <%uKB>\n",
                bpm_pg->buf_top_idx, memsz, memsz >> 10);
#endif

    BPM_DBG_ASSERT(bpm_pg->buf_top_idx != 0U);
    BPM_DBG_ASSERT(bpm_pg->buf_top_idx == tot_num_buf);

    /* Inverted Stack - buf_cur_idx moves from [0 .. buf_top_idx) */
    bpm_pg->buf_cur_idx = 0U;


#if !(defined(CONFIG_BCM_RDPA) && !defined(CONFIG_BCM_RDPA_MODULE)) 
#if defined(CONFIG_BCM_PKTDMA)    
    bpm_pg->rxbds = bcmPktDma_GetTotRxBds();
#endif    
#endif

    bpm_pg->fap_resv = FAP_BPM_BUF_RESV;
    bpm_pg->tot_resv_buf = bpm_pg->rxbds + bpm_pg->fap_resv;
    bpm_pg->max_dyn = bpm_pg->buf_top_idx - bpm_pg->tot_resv_buf;

    bpm_upd_dyn_buf_lo_thresh();
    bpm_init_done_g = 1;
    BPM_UNLOCK_IRQ();

    if ((bpm_pg->buf_top_idx - bpm_pg->buf_cur_idx) == 0)
        return BPM_ERROR;
    else
        return BPM_SUCCESS;
}

#ifdef BPM_TEST
/*
 *------------------------------------------------------------------------------
 * Function   : bpm_fini_buf_pool
 * Description: Releases all buffers of the pool
 *------------------------------------------------------------------------------
 */
static void bpm_fini_buf_pool( void )
{
    void *mem_p;
    uint32_t i;
    BPM_DEFS();
    BPM_FLAGS_DEF();

    BCM_LOG_FUNC( BCM_LOG_ID_BPM );

    BPM_LOCK_IRQ();
    /*
     * Release all memory pools allocated
     */
    for ( i = 0; i < bpm_pg->mem_idx; i++ )
    {
        mem_p = (uint8_t *) bpm_pg->mem_pool[ i ];
        if ( mem_p )
            kfree( mem_p );
    }

#if defined(CONFIG_BCM_BPM_BUF_TRACKING)
    bpm_track_disable();
    kfree( bpm_pg->sbuf_pool );
#endif

    memset( (void*)bpm_pg, 0, sizeof(bpm_t) );
    BPM_UNLOCK_IRQ();

    return;
}
#endif


#if defined(CC_BPM_SKB_POOL_BUILD)

extern void skb_headerreset(struct sk_buff *skb);
extern void skb_shinforeset(struct skb_shared_info *skb_shinfo);

extern struct sk_buff *skb_header_alloc(void);
extern void skb_header_free(struct sk_buff *skb);

static void bpm_recycle_skb(void *skbv, unsigned long context,
    uint32_t recycle_action);

/**
 * An skb's scratchpad, namely control buffer and wlan control buffer may
 * be optionally cleared (by default or explicitly within a bypass datapath).
 */
#define SKB_CB_NOOP     (0)
#define SKB_CB_ZERO     (~SKB_CB_NOOP)

static inline void _skb_zero( struct sk_buff *skb, const uint32_t cb_op )
{
    skb->prev     = NULL;
    skb->next     = NULL;
    skb->priority = 0;
    skb->mark     = 0;

    if (cb_op != SKB_CB_NOOP)
        skb_cb_zero(skb); /* optionally, zero out skb::cb[] and skb:wl_cb[] */
}

/* All sk_buff(s) maintained in BPM are in a pristined state */
static inline void _bpm_pristine_skb( struct sk_buff *skb, uint32_t need_reset )
{
    BPM_SKB_POOL_STATS_BPM_DEFS();

    if (unlikely(need_reset))
    {
        skb_headerreset(skb); /* memset 0 and atomic set users = 1 */
        BPM_SKB_POOL_STATS_ADD(skb_hdr_reset_cnt, 1);
        skb->recycle_hook = (RecycleFuncP) bpm_recycle_skb;
    }

    /* BPM SKB must only be attached to a BPM BUF or a BUF that obeys the
     * bcm_pkt_lengths.h layout (e.g. feed ring buffer in xrdp).
     */
    skb->recycle_flags = (SKB_RECYCLE | SKB_BPM_PRISTINE | SKB_DATA_RECYCLE);

}

/*
 *------------------------------------------------------------------------------
 * Function   : bpm_free_skb
 * Description: Free a sk_buff into free pool
 *------------------------------------------------------------------------------
 */
static void bpm_free_skb( void *skbp )
{
    struct sk_buff *skb = (struct sk_buff *)skbp;
    BPM_DEFS(); BPM_FLAGS_DEF();
    BPM_PREFETCH();

    /* Add skb to BPM sk_buff Pool free list */

    BPM_LOCK_IRQ();

    skb->next = bpm_pg->skb_freelist;
    bpm_pg->skb_freelist = skb;
    ++bpm_pg->skb_avail;

    BPM_SKB_POOL_STATS_ADD(skb_free_cnt, 1);

    BPM_UNLOCK_IRQ();

}   /* bpm_free_skb() */


/*
 *------------------------------------------------------------------------------
 * Function: bpm_recycle_skb
 * Description: RecycleFuncP hook registered with a skb allocated from the
 * BPM sk_buff pool. A BPM sk_buff, may ONLY be attached to a BPM data buffer.
 *
 * Recycle a data buf or sk_buff from the BPM preallocated buf or sk_buff pool
 *
 * Kernel may invoke the recycle function with recycle_flags either indicating
 * - that the attached BPM buffer needs to be returned into BPM "buf" pool.
 * - that the sk_buff itself needs to be returned to BPM "sk_buff" pool.
 *
 * An fkb_init() data buffer will be returned via:
 *      fkb_p->recycle_hook = bdmf_sysb_recycle
 *
 * An fkb may be nbuf_xlate() to a BPM skb (i.e. bpm_recycle_skb), while
 *    inheriting the original FkBuff_t:recycle_flags.
 *
 * The notion of recycle_context in FkBuff_t and sk_buff is INVALID:
 * - recycle_context is 32bit.
 * - FkBuff_t repurposed (union) dhd_pkttag_info_p. "repurpose = blatant hack"
 *
 *------------------------------------------------------------------------------
 */
static void bpm_recycle_skb(void *skbv, unsigned long context,
    uint32_t recycle_action)
{
    struct sk_buff *skb = (struct sk_buff *)skbv;
    BPM_SKB_POOL_STATS_BPM_DEFS();

    /* SKB_DATA_RECYCLE action: recycle bpm buf */
    if (recycle_action & SKB_DATA_RECYCLE)
    {
        void *buf_p = (void*)BPM_PHEAD_TO_BUF(skb->head);
        struct skb_shared_info *skb_shinfo = skb_shinfo(skb);
#if !defined(CONFIG_BCM_GLB_COHERENCY)
        uint8_t *dirty_p = (uint8_t*)skb_shinfo->dirty_p;
#endif /* !CONFIG_BCM_GLB_COHERENCY */

        /* Audit a BPM skb attached to a BPM buf layout */
        BPM_DBG_ASSERT(BPM_BUF_TO_PSHINFO(buf_p) == skb_shinfo);

        /* User chose to clear SKB_BPM_PRISTINE, so full reset */
        if ( unlikely((skb->recycle_flags & SKB_BPM_PRISTINE) == 0) )
        {
            skb_shinforeset(skb_shinfo); /* memset 0 and dataref = 1 */
            BPM_SKB_POOL_STATS_ADD(shinfo_reset_cnt, 1);
#if !defined(CONFIG_BCM_GLB_COHERENCY)
            cache_invalidate_len(buf_p, BCM_MAX_PKT_LEN);
            BPM_SKB_POOL_STATS_ADD(full_cache_inv_cnt, 1);
#endif /* !CONFIG_BCM_GLB_COHERENCY */
        }

#if !defined(CONFIG_BCM_GLB_COHERENCY)
        else if (dirty_p > (uint8_t *)buf_p)
        {
            int len = (uint8_t *)(dirty_p) - (uint8_t *)buf_p;
            len = len > BCM_MAX_PKT_LEN ? BCM_MAX_PKT_LEN : len;
            cache_invalidate_len(buf_p, len);
            BPM_SKB_POOL_STATS_ADD(part_cache_inv_cnt, 1);
        }
#endif /* !CONFIG_BCM_GLB_COHERENCY */

        /* else do nothing */

        skb_shinfo->dirty_p = NULL;

        bpm_free_buf(buf_p); /* buf_p = skb->head + BCM_PKT_HEADROOM */
        return;
    }

    /* SKB_RECYCLE action: recycle sk_buff to BPM sk_buff pool */
    if (recycle_action & SKB_RECYCLE)
    {
        /* Make skb pristine if tainted, i.e. user cleared SKB_BPM_PRISTINE */
        _bpm_pristine_skb(skb, ((skb->recycle_flags & SKB_BPM_PRISTINE) == 0));
        bpm_free_skb(skb); /* Now free into BPM skb pool */
        return;
    }
    BPM_DBG_ASSERT(0);
}


/* Accessor function to return max number of skbs in pool */
static uint32_t bpm_total_skb( void )
{
    BPM_DEFS();
	return bpm_pg->skb_total;
}

/* Accessor function to return avail number of skbs in free pool */
static uint32_t bpm_avail_skb( void )
{
    BPM_DEFS();
	return bpm_pg->skb_avail;
}

/*
 *------------------------------------------------------------------------------
 * Function   : bpm_attach_skb
 * Description: Attach a BPM allocated sk_buf to a BPM data buffer.
 *
 * CAUTION: The pointer buf is what was allocated using bpm_alloc_buf().
 * This is the starting address where a DMA engine will transfer data, and
 * will serve as the "skb->data".
 *
 * Reference skb_headerinit() in skbuff.c ...
 *------------------------------------------------------------------------------
 */
static void bpm_attach_skb( void *skbp, void *buf, uint32_t datalen )
{
    /* lock free helper routine */
    struct sk_buff *skb = (struct sk_buff *)skbp;
    struct skb_shared_info *skb_shinfo;

    /* skb_shared_info is ALWAYS aligned to end of the BPM buffer */
    skb_shinfo = BPM_BUF_TO_PSHINFO(buf);

    bcm_prefetch(skb_shinfo);

    /* Ensure skb is not already contaminated from alloc to attach */
    BPM_DBG_ASSERT((skb->recycle_flags & SKB_BPM_PRISTINE) == SKB_BPM_PRISTINE);
    BPM_DBG_ASSERT(skb_shinfo->dirty_p == NULL);

    /* All BPM allocated sk_buffs were in pristine state.  */

    skb->truesize = datalen + sizeof(struct sk_buff);

    skb->head  = BPM_BUF_TO_PHEAD(buf);
    skb->data  = buf;

    /* skb control buffer cb[] and wl_cb[] are not zeroed */
    _skb_zero(skb, SKB_CB_NOOP); /* prev, next, priority, mark zeroed */

    /* NET_SKBUFF_DATA_USES_OFFSET see skb_set_tail_pointer(skb, datalen) */
    skb->tail  = BPM_BUF_TO_TAIL(buf, datalen);
    skb->end   = BPM_BUF_TO_END(buf); /* do not relocate skb_shared_info */
    skb->len   = datalen;

    atomic_set(&skb->users, 1);
    atomic_set(&(skb_shinfo->dataref), 1);

    skb_shinfo->dirty_p = NULL;

}   /* bpm_attach_skb() */

/*
 *------------------------------------------------------------------------------
 * Function   : bpm_alloc_skb
 * Description: Allocate a sk_buff from free pool
 *------------------------------------------------------------------------------
 */
static void *bpm_alloc_skb( void )
{
    struct sk_buff *skb;
    BPM_DEFS(); BPM_FLAGS_DEF();
    BPM_PREFETCH();

    BPM_LOCK_IRQ();

    bcm_prefetch(bpm_pg->skb_freelist);

    skb = bpm_pg->skb_freelist;
    BPM_SKB_POOL_STATS_ADD(skb_alloc_cnt, 1);

    if ( likely(bpm_pg->skb_avail) )
    {
        bpm_pg->skb_freelist = skb->next;
        skb->next = (struct sk_buff *)NULL;
        --bpm_pg->skb_avail;
    }
    else
    {
        skb = NULL;
        BPM_SKB_POOL_STATS_ADD(skb_fail_cnt, 1);
        BPM_SKB_POOL_STATS_SUB(skb_alloc_cnt, 1);
    }

    BPM_UNLOCK_IRQ();

    /* bcm_prefetch(bpm_pg->skb_freelist); */

    return (void *)skb;

}   /* bpm_alloc_skb() */


/*
 *------------------------------------------------------------------------------
 * Function   : bpm_alloc_mult_skb
 * Description: Allocate mulitple sk_buff from free pool
 *------------------------------------------------------------------------------
 */
static void *bpm_alloc_mult_skb( uint32_t request_num )
{
    struct sk_buff * skb, *skb_list;
    uint32_t alloc;
    BPM_DEFS(); BPM_FLAGS_DEF();
    BPM_PREFETCH();

    BPM_DBG_ASSERT(request_num != 0U);

    BPM_LOCK_IRQ();

    bcm_prefetch(bpm_pg->skb_freelist);
    BPM_SKB_POOL_STATS_ADD(skb_alloc_cnt, request_num);

    if ( likely(bpm_pg->skb_avail >= request_num) )
    {
        skb = bpm_pg->skb_freelist;
        bcm_prefetch(skb->next);

        bpm_pg->skb_avail -= request_num;
        skb_list = skb; /* list to be returned */

        for (alloc = 1; alloc < request_num; ++alloc) {
            skb = skb->next;
            bcm_prefetch(skb->next);
        }

        bpm_pg->skb_freelist = skb->next;
        skb->next = (struct sk_buff *)NULL;
    }
    else
    {
        skb_list = NULL;
        BPM_SKB_POOL_STATS_SUB(skb_alloc_cnt, request_num);
        BPM_SKB_POOL_STATS_ADD(skb_fail_cnt, 1);
    }

    BPM_UNLOCK_IRQ();

    return (void *)skb_list;

}   /* bpm_alloc_mult_skb() */


/*
 *------------------------------------------------------------------------------
 * Function   : bpm_alloc_buf_attach_skb
 * Description: allocates a buffer, an SKB from global buffer pool
 *              and attaches the SKB to the buffer and return the SKB
 * NOTE: Cut-n-paste from bpm_alloc_buf, bpm_alloc_skb, bpm_attach_skb
 *       to avoid multiple lock/unlock, branch conditionals, prefetch, etc.
 *------------------------------------------------------------------------------
 */
static void * bpm_alloc_buf_skb_attach( uint32_t datalen )
{
    BPM_DEFS(); BPM_FLAGS_DEF();
    void *buf;
    struct sk_buff *skb;
    struct skb_shared_info *skb_shinfo;

    BPM_PREFETCH();

    BPM_LOCK_IRQ();

    /* Allocate buffer : reference bpm_alloc_buf() */
    BPM_BUF_POOL_PREFETCH_LD(bpm_pg->buf_cur_idx);
    ++bpm_pg->buf_alloc_cnt;

	/* if buffers available in global buffer pool */
    if ( likely(bpm_pg->buf_cur_idx < bpm_pg->buf_top_idx) )
    {
        buf = bpm_pg->buf_pool[bpm_pg->buf_cur_idx++]; // POST ++
		bcm_prefetch(bpm_pg->skb_freelist);

        update_min_availability();
        GBPM_USER_UPD_BUF_LVL();
    }
    else
    {
        buf = NULL;
        --bpm_pg->buf_alloc_cnt;
        ++bpm_pg->buf_fail_cnt;

        BPM_UNLOCK_IRQ();

        return (void *)NULL;
    }

    /* Allocate SKB : reference bpm_alloc_skb() */
    skb = bpm_pg->skb_freelist;
    BPM_SKB_POOL_STATS_ADD(skb_alloc_cnt, 1);
    BPM_SKB_POOL_STATS_ADD(skb_bpm_alloc_cnt, 1);

    if ( likely(bpm_pg->skb_avail) )
    {
        bpm_pg->skb_freelist = skb->next;
        skb->next = (struct sk_buff *)NULL;
        --bpm_pg->skb_avail;
    }
    else
    {
        BPM_SKB_POOL_STATS_ADD(skb_fail_cnt, 1);
        BPM_SKB_POOL_STATS_SUB(skb_alloc_cnt, 1);
        BPM_SKB_POOL_STATS_SUB(skb_bpm_alloc_cnt, 1);

        BPM_UNLOCK_IRQ();

        bpm_free_buf(buf);

        return (void *)NULL;
    }

    BPM_UNLOCK_IRQ();

    /* bcm_prefetch(bpm_pg->skb_freelist); */

    /* Attach SKB to the buffer : reference bpm_attach_skb() */
    skb_shinfo = BPM_BUF_TO_PSHINFO(buf);
    bcm_prefetch(skb_shinfo);

	/* Ensure skb is not already contaminated */
	BPM_DBG_ASSERT((skb->recycle_flags & SKB_BPM_PRISTINE) == SKB_BPM_PRISTINE);
	BPM_DBG_ASSERT(skb_shinfo->dirty_p == NULL);

    /* All BPM allocated sk_buffs were in pristine state.  */

    skb->truesize = datalen + sizeof(struct sk_buff);

	_skb_zero(skb, SKB_CB_ZERO); /* prev, next, priority, mark, cb[], wl_cb[] */

    skb->head  = BPM_BUF_TO_PHEAD(buf);
    skb->data  = buf;

    /* NET_SKBUFF_DATA_USES_OFFSET see skb_set_tail_pointer(skb, datalen) */
    skb->tail  = BPM_BUF_TO_TAIL(buf, datalen);
    skb->end   = BPM_BUF_TO_END(buf); /* do not relocate skb_shared_info */
    skb->len   = datalen;

    atomic_set(&skb->users, 1);
    atomic_set(&(skb_shinfo->dataref), 1);

	skb_shinfo->dirty_p = NULL;

    return (void *)skb;
}


#if defined(CC_BPM_SKB_POOL_GROWS)
/*
 *------------------------------------------------------------------------------
 * Function   : bpm_grow_skb_pool
 * Description: Extend the skb_pool with N buffers, or upto "bound".
 * This function is not reentrant safe. Experimental!
 *------------------------------------------------------------------------------
 */
static int bpm_grow_skb_pool( uint32_t skb_grow )
{
    uint32_t skbs_alloced;
    struct sk_buff *skb, *skb_head, *skb_tail;
    BPM_DEFS(); BPM_FLAGS_DEF();

    BCM_LOG_FUNC( BCM_LOG_ID_BPM );

    BPM_LOCK_IRQ();

    /* Ensure pool does not get extended beyond configured bound */
    if ((bpm_pg->skb_total + skb_grow) > bpm_pg->skb_bound)
    {
        /* permissible extends */
        skb_grow = bpm_pg->skb_bound - bpm_pg->skb_total;

        if (skb_grow == 0U) { /* no more extensions, as reached bound */
            BPM_UNLOCK_IRQ();
            BCM_LOG_DEBUG( BCM_LOG_ID_BPM, "extend bound failure");
            BPM_SKB_POOL_STATS_ADD(skb_fail_cnt, 1);
            return BPM_ERROR;
        }
    }

    /* may fail skb_header_alloc later, and we are releasing lock */
    bpm_pg->skb_total += skb_grow;

    BPM_UNLOCK_IRQ();

    skb_alloced = 0U;
    skb_head = skb_tail = (struct sk_buff *)NULL;

    do /* Allocate sk_buffs and place them into local list */
    {
        skb = skb_header_alloc(); /* allocate sk_buff structure */

        if (!skb) {
            BCM_LOG_DEBUG( BCM_LOG_ID_BPM, "extend skb failure");
            BPM_SKB_POOL_STATS_ADD(skb_fail_cnt, 1);
            break; /* skbs_alloced may be 0 */
        }

        /* All sk_buffs in BPM pool are in a "pristine" state */
        _bpm_pristine_skb(skb, 1); /* need_reset = 1 */

        if (skb_alloced) {
            skb->next = skb_head; /* uses skb::next for linking */
            skb_head  = skb;
        } else {
            skb_head  = skb;
            skb_tail  = skb;
        }

    } while (++skb_alloced < skb_grow);

    if (skb_alloced) /* Prepend local pool to BPM pool, with lock */
    {
        BPM_LOCK_IRQ();

        /* Prepend list to bpm's sk_buff pool free list */
        skb_tail->next       = bpm_pg->skb_freelist;
        bpm_pg->skb_freelist = skb_head;

        bpm_pg->skb_avail   += skb_alloced;
        bpm_pg->skb_total   -= skb_grow - skb_alloced; /* fixup ... */

        BPM_SKB_POOL_STATS_ADD(skb_grow_cnt, 1);
        BPM_UNLOCK_IRQ();
    }

    BCM_LOG_DEBUG( BCM_LOG_ID_BPM,
        "skbs pool extended by %u total %u", skb_alloced, bpm_pg->skb_total);

    return BPM_SUCCESS;

}   /* bpm_grow_skb_pool() */

#endif /* CC_BPM_SKB_POOL_GROWS */


#ifdef BPM_TEST
/*
 *------------------------------------------------------------------------------
 * Function   : bpm_fini_skb_pool
 * Description: Release all sk_buffs in skb_freelist.
 *              For Use only in a test.
 *------------------------------------------------------------------------------
 */
static void bpm_fini_skb_pool( void )
{
    struct sk_buff *skb, *skb_freelist;
    uint32_t skb_errors, skb_avail, skb_total;
    BPM_DEFS(); BPM_FLAGS_DEF();

    BCM_LOG_FUNC( BCM_LOG_ID_BPM );

    BPM_LOCK_IRQ();

    skb_freelist         = bpm_pg->skb_freelist;
    skb_avail            = bpm_pg->skb_avail;
    skb_total            = bpm_pg->skb_total;

    bpm_pg->skb_total    = 0U;
    bpm_pg->skb_bound    = 0U;
    bpm_pg->skb_avail    = 0U;
    bpm_pg->skb_freelist = (struct sk_buff *)NULL;

    BPM_UNLOCK_IRQ();

    /* sk_buffs allocated and not yet freed */
    skb_errors = bpm_pg->skb_total - bpm_pg->skb_avail;
    if (skb_errors)
        BCM_LOG_ERROR( BCM_LOG_ID_BPM, "fini skb_pool inuse %u", skb_errors);

    /* walk free list releasing sk_buffs back to kmem_cache skbuff_head_cache */
    while ((skb = skb_freelist) != (struct sk_buff *)NULL)
    {
        skb_freelist       = skb->next;

        skb_header_free(skb); /* kmem_cache_free skbuff_head_cache */

        if (skb_avail)
            --skb_avail;
        else
            ++skb_errors; /* BPM internal error */
    }

    skb_errors += skb_avail; /* BPM internal error */

    if (skb_errors) 
        BCM_LOG_ERROR( BCM_LOG_ID_BPM, "fini skb_pool errors %u", skb_errors);
}
#endif

/*
 *------------------------------------------------------------------------------
 * Function   : bpm_init_skb_pool
 * Description: Initialize sk_buff pool, with  percent of the total buf_pool.
 * Preallocate a pool of sk_buff headers that have been preinitialized.
 * These sk_buff headers are not pointing to any data buffers.
 *
 * Assumes Kernel Network is inited (soc_init() --> skb_init() which will
 * setup the kmem_cache skbuff_head_cache, needed for skb_header_alloc()
 *
 *------------------------------------------------------------------------------
 */
static int bpm_init_skb_pool( uint32_t total_num_buf )
{
    struct sk_buff *skb;
    uint32_t skb_total, skb_avail;
    BPM_DEFS();

    BCM_LOG_FUNC( BCM_LOG_ID_BPM );

    /* NO LOCK */

    bpm_pg->skb_freelist = NULL;

    /* Compute number of skbs to be managed as a percent of total buffers */
    skb_total = ((total_num_buf * BPM_SKB_POOL_PCT_INIT) / 100);

    for (skb_avail = 0U; skb_avail < skb_total; ++skb_avail)
    {
        skb = skb_header_alloc(); /* allocate sk_buff structure */
        if (!skb) {
            BCM_LOG_ERROR(BCM_LOG_ID_BPM, "skb pool init %u failure",
                skb_total - skb_avail);
            return BPM_ERROR;;
        }

        /* All sk_buffs in BPM pool are in a "pristine" state */
        _bpm_pristine_skb(skb, 1); /* need_reset = 1 */

        skb->next            = bpm_pg->skb_freelist;
        bpm_pg->skb_freelist = skb;
    }

    bpm_pg->skb_avail = bpm_pg->skb_total = skb_avail;
    bpm_pg->skb_bound = total_num_buf; /* Cannot grow beyond buf_pool size */

    BCM_LOG_DEBUG( BCM_LOG_ID_BPM,
        "skbs pool inited total %u. %u pct of %u",
        skb_avail, BPM_SKB_POOL_PCT_INIT, total_num_buf);

    return BPM_SUCCESS;

}   /* bpm_init_skb_pool() */

/*
 *------------------------------------------------------------------------------
 * Function : Recycle BPM based NBuff type buffer
 * CAUTION  : Only DATA recycle is supported
 *            No BPM LOCKS taken
 *  TODO    : Do cache invalidations on fkb_init() data buffer
 *------------------------------------------------------------------------------
 */
static void
bpm_recycle_pNBuff(void * pNBuff, unsigned long context, uint32_t recycle_action)
{
    if (IS_FKBUFF_PTR(pNBuff))
    {
        FkBuff_t * fkb;

        fkb = PNBUFF_2_FKBUFF(pNBuff);

        /* Transmit driver is expected to perform cache invalidations */

        /* BPM expects data buffer ptr offseted by FkBuff_t, BCM_PKT_HEADROOM */
        bpm_free_buf((void*)PFKBUFF_TO_PDATA(fkb, BCM_PKT_HEADROOM));
    }
    else
    {
        BCM_ASSERT(recycle_action == SKB_DATA_RECYCLE);
        bpm_recycle_skb(pNBuff, context, recycle_action);
    }
}   /* bpm_recycle_pNBuff() */

#else  /* ! CC_BPM_SKB_POOL_BUILD */

/* Accessor function to return max number of skbs in pool */
static uint32_t bpm_total_skb( void )
{ BPM_DBG_ASSERT(0); return 0U; }

/* Accessor function to return avail number of skbs in free pool */
static uint32_t bpm_avail_skb( void )
{ BPM_DBG_ASSERT(0); return 0U; }

static void bpm_attach_skb( void *skb, void *data, uint32_t datalen )
{ BPM_DBG_ASSERT(0); }

static void *bpm_alloc_skb( void )
{ return NULL; }

static void *bpm_alloc_mult_skb( uint32_t request_num )
{ return NULL; }

static void bpm_free_skb( void *skb )
{ BPM_DBG_ASSERT(0); }
#endif /* CC_BPM_SKB_POOL_BUILD */ 

/** Return the total number of buffers managed by BPM
 *
 *@returns total number of buffers managed by BPM
 */
static uint32_t bpm_get_total_bufs( void )
{
    BPM_DEFS();
    return bpm_pg->buf_top_idx;
}


/** Return the current number of free buffers in the BPM pool
 *
 *@returns the current number of free buffers in the BPM pool
 */
static uint32_t bpm_get_avail_bufs( void )
{
    BPM_DEFS();
    return BPM_BUF_AVAIL_CNT(bpm_pg); /* computed using top and cur idx */
}

/** Return the current number of free buffers in the BPM pool
 *
 *@returns the current number of free buffers in the BPM pool
 */
static uint32_t bpm_get_max_dyn_bufs( void )
{
    BPM_DEFS();
    return bpm_pg->max_dyn;
}


#ifdef BPM_TEST
static void bpm_test( void )
{
    void * buf_p;

    bpm_init_buf_pool( 896 );
    buf_p = bpm_alloc_buf();
    bpm_free_buf( buf_p );
#if defined(CC_BPM_SKB_POOL_BUILD)
    bpm_init_skb_pool( 896 );
    /* bpm_alloc_buf_skb_attach(100); */
    bpm_fini_skb_pool();
#endif
    bpm_fini_buf_pool();
}
#endif

/*
 *------------------------------------------------------------------------------
 * Function   : bpm_dump_skbuffs
 * Description: function handler for dumping the skbuffs pool state
 *------------------------------------------------------------------------------
 */
static void bpm_dump_skbuffs(void)
{
#if defined(CC_BPM_SKB_POOL_BUILD)
    BPM_DEFS();

    printk( "BPM Skbuff Pool: avail %u total %u bound %u\n",
            bpm_pg->skb_avail, bpm_pg->skb_total, bpm_pg->skb_bound );
#if defined(CC_BPM_SKB_POOL_STATS)
    printk( "\tStats: alloc %u bpm_alloc %u free %u error %u fail %u grow %u\n",
            bpm_pg->skb_alloc_cnt, bpm_pg->skb_bpm_alloc_cnt,
            bpm_pg->skb_free_cnt, bpm_pg->skb_error_cnt,
            bpm_pg->skb_fail_cnt, bpm_pg->skb_grow_cnt );
    printk( "\nBPM Reset Counters: \n--------------------\n"
            "skb_hdr_rst %u\tshinf_rst %u\tfull_cache_inv %u\t"
            "part_cache_inv %u \n",
            bpm_pg->skb_hdr_reset_cnt, bpm_pg->shinfo_reset_cnt, 
            bpm_pg->full_cache_inv_cnt, bpm_pg->part_cache_inv_cnt );
#endif /* CC_BPM_SKB_POOL_STATS */
#else  /* ! CC_BPM_SKB_POOL_BUILD */
    printk( "BPM does not support SKB_POOL\n");
#endif /* CC_BPM_SKB_POOL_BUILD */

}


/*
 *------------------------------------------------------------------------------
 * Function   : bpm_dump_status
 * Description: function handler for dumping the status
 *------------------------------------------------------------------------------
 */
static void bpm_dump_status(void)
{
    BPM_DEFS();

    printk( "\n------------------------ BPM Status -----------------------\n" );
    printk( " tot_buf avail   cur  no_buf_err  cum_alloc   cum_free min_availability\n" );
    printk( "%7u %5u %5u %10u %10u %10u %10u",
        bpm_pg->buf_top_idx, BPM_BUF_AVAIL_CNT(bpm_pg), bpm_pg->buf_cur_idx,
        bpm_pg->buf_fail_cnt, bpm_pg->buf_alloc_cnt, bpm_pg->buf_free_cnt,
        bpm_pg->min_avail_cnt);
    printk( "\nmax_dyn tot_resv_buf rxbds fap_resv\n" );
    printk( "%7u %12u %5u %8u",
        bpm_pg->max_dyn, bpm_pg->tot_resv_buf, bpm_pg->rxbds, bpm_pg->fap_resv ); 

    BCM_LOG_DEBUG( BCM_LOG_ID_BPM, "buf_pool mem_ix   mem_pool \n" );
    BCM_LOG_DEBUG( BCM_LOG_ID_BPM, " 0x%p %6u 0x%p\n", 
        bpm_pg->buf_pool, bpm_pg->mem_idx, bpm_pg->mem_pool );
    printk("\n-------------------------------------------------"
           "---------------------------\n");
    printk("        dev chnl  cum_alloc   cum_free avail trig"
           " bulk       reqt       resp\n");
    printk("------ ---- ---- ---------- ---------- ----- ----"
           " ---- ---------- ----------\n");

    bpm_dump_skbuffs();

    GBPM_USER_STATUS();

    return;
}


/*
 *------------------------------------------------------------------------------
 * Function   : bpm_dump_buffers
 * Description: function handler for dumping the buffers
 *------------------------------------------------------------------------------
 */
static void bpm_dump_buffers(void)
{
    int buf_idx;
    BPM_DEFS(); BPM_FLAGS_DEF();

    printk( "\n----------------- Buffer Pool ----------------\n" );
    printk( "\n  Idx Address0 Address1 Address2 Address3" );
    printk( " Address4 Address5 Address6 Address7\n" );

    /* WARNING:
     * 1. buf_top_idx may not be a multiple of 8.
     * 2. buffers may appear multiple times, in a dump.
     */
    for ( buf_idx = 0; (buf_idx < bpm_pg->buf_top_idx); buf_idx += 8 )
    {
        if ((buf_idx % 256) == 0)
            printk("\n");
   
        BPM_LOCK_IRQ();
        /* NOTE: Pointer to base (FkBuff_t) is dumped */
        printk( "[%3u] %p %p %p %p %p %p %p %p\n", 
                buf_idx,
                BPM_BUF_TO_PFKBUFF( bpm_pg->buf_pool[buf_idx + 0] ),
                BPM_BUF_TO_PFKBUFF( bpm_pg->buf_pool[buf_idx + 1] ),
                BPM_BUF_TO_PFKBUFF( bpm_pg->buf_pool[buf_idx + 2] ),
                BPM_BUF_TO_PFKBUFF( bpm_pg->buf_pool[buf_idx + 3] ),
                BPM_BUF_TO_PFKBUFF( bpm_pg->buf_pool[buf_idx + 4] ),
                BPM_BUF_TO_PFKBUFF( bpm_pg->buf_pool[buf_idx + 5] ),
                BPM_BUF_TO_PFKBUFF( bpm_pg->buf_pool[buf_idx + 6] ),
                BPM_BUF_TO_PFKBUFF( bpm_pg->buf_pool[buf_idx + 7] ) );
        BPM_UNLOCK_IRQ();
    }

    printk( "\n" );

    return;
}

/*
 *------------------------------------------------------------------------------
 * Function   : bpm_dump_thresh
 * Description: function for dumping the thresh
 *------------------------------------------------------------------------------
 */
static void bpm_dump_thresh( void )
{
    gbpm_port_t port; 
    uint32_t chnl;
    BPM_DEFS();

    printk( "\n-------------------- BPM Thresh -------------------\n" );
    printk( "tot_buf tot_resv_buf max_dyn   avail dyn_buf_lo_thr\n" );
    printk( "%7u %12u %7u %7u %14u\n", 
        bpm_pg->buf_top_idx, bpm_pg->tot_resv_buf, bpm_pg->max_dyn,
        BPM_BUF_AVAIL_CNT(bpm_pg), bpm_pg->dyn_buf_lo_thresh );

    printk("\n---------------------------------\n");
    printk( "port chnl rx_ring_buf alloc_trig\n" );
    printk( "---- ---- ----------- ----------\n");
    for( port=0; port<GBPM_PORT_MAX; port++ )
    {
        for( chnl=0; chnl<GBPM_RXCHNL_MAX; chnl++ )
        {
            if (bpm_pg->status[port][chnl] == GBPM_RXCHNL_ENABLED)
            {
                printk( "%4s %4u %11u %10u\n", strBpmPort[port], chnl, 
                bpm_pg->num_rx_buf[port][chnl], bpm_pg->alloc_trig_thresh[port][chnl] );
            }
        }
    }
    printk( "\n" );


    printk("\n---------------------------------------\n");
    printk("        dev  txq loThr hiThr    dropped\n");
    printk("------ ---- ---- ----- ----- ----------\n");

    GBPM_USER_THRESH();

#if defined(GBPM_FAP_SUPPORT)
    printk("\n\n-------------------------------------------\n");
    printk("        dev chnl  txq dropThresh    dropped\n");
    printk("------ ---- ---- ---- ---------- ----------\n");
#endif

    GBPM_USER_ENET_THRESH();
    GBPM_USER_FAP_ENET_THRESH();

}


/*
 *------------------------------------------------------------------------------
 * function   : bpm_calc_num_buf
 * description: Finds the total memory available on the board, and assigns 
 * one-ourth of the total memory to the buffers. Finally calculates the 
 * number of buffers to be allocated based on buffer memory and buffer size.
 *------------------------------------------------------------------------------
 */
static int bpm_calc_num_buf( void )
{
    uint32_t tot_mem_size = kerSysGetSdramSize();
    uint32_t buf_mem_size = (tot_mem_size/100) * CONFIG_BCM_BPM_BUF_MEM_PRCNT;

    printk( "BPM: tot_mem_size=%dB (%dMB), ", tot_mem_size, tot_mem_size/MB );
    printk( "buf_mem_size <%d%%> =%dB (%dMB), ",
            CONFIG_BCM_BPM_BUF_MEM_PRCNT, buf_mem_size, buf_mem_size/MB );
    printk( "num of buffers=%d, buf size=%d\n",
            (unsigned int)(buf_mem_size/BPM_BUF_SIZE),
            (unsigned int)BPM_BUF_SIZE);

    return (buf_mem_size/BPM_BUF_SIZE);
}


/*
 *------------------------------------------------------------------------------
 * Function Name: bpm_drv_ioctl
 * Description  : Main entry point to handle user applications IOCTL requests
 *                from BPM Control Utility.
 * Returns      : 0 - success or error
 *------------------------------------------------------------------------------
 */
static int bpm_drv_ioctl(struct inode *inode, struct file *filep,
                       unsigned int command, unsigned long arg)
{
    bpmctl_ioctl_t cmd;
    bpmctl_data_t  bpm;
    bpmctl_data_t *bpm_p = &bpm;
#if defined(CONFIG_BCM_BPM_BUF_TRACKING)
    bpmctl_track_t *trk_p;
#endif
    int ret = BPM_SUCCESS;
    BPM_DEFS(); BPM_FLAGS_DEF();

    if ( command > BPMCTL_IOCTL_MAX )
        cmd = BPMCTL_IOCTL_MAX;
    else
        cmd = (bpmctl_ioctl_t)command;

    copy_from_user( bpm_p, (uint8_t *) arg, sizeof(bpm) );

    BCM_LOG_DEBUG( BCM_LOG_ID_BPM, 
            "cmd<%d> %s subsys<%d> %s op<%d> %s arg<0x%lx>",
            command, bpmctl_ioctl_name[command], 
            bpm_p->subsys, bpmctl_subsys_name[bpm_p->subsys], 
            bpm_p->op, bpmctl_op_name[bpm_p->op], arg );

    switch ( cmd )
    {
        case BPMCTL_IOCTL_SYS :
        {
            switch (bpm_p->subsys)
            {
                case BPMCTL_SUBSYS_STATUS:
                    switch (bpm_p->op)
                    {
                        case BPMCTL_OP_DUMP:
                            bpm_dump_status();
                            break;

                        default:
                            BCM_LOG_ERROR(BCM_LOG_ID_BPM, 
                                        "Invalid op[%u]", bpm_p->op );
                     }
                     break;

                case BPMCTL_SUBSYS_THRESH:
                    switch (bpm_p->op)
                    {
                        case BPMCTL_OP_DUMP:
                            BPM_LOCK_IRQ();
                            bpm_dump_thresh();
                            BPM_UNLOCK_IRQ();
                            break;

                        default:
                            BCM_LOG_ERROR(BCM_LOG_ID_BPM, 
                                        "Invalid op[%u]", bpm_p->op );
                     }
                     break;

                case BPMCTL_SUBSYS_BUFFERS:
                    switch (bpm_p->op)
                    {
                        case BPMCTL_OP_DUMP:
                            bpm_dump_buffers();
                            break;

                        default:
                            BCM_LOG_ERROR(BCM_LOG_ID_BPM, 
                                        "Invalid op[%u]", bpm_p->op );
                     }
                     break;

#if defined(CONFIG_BCM_BPM_BUF_TRACKING)
                case BPMCTL_SUBSYS_TRACK:
                    trk_p = &(bpm_p->track);
                    trk_p->base = (unsigned long long)(size_t)bpm_get_base_addr((void *)(size_t)trk_p->base);

                    switch (trk_p->cmd)
                    {
                        case BPMCTL_TRK_STATUS:
                            bpm_dump_track_status();
                            break;

                        case BPMCTL_TRK_ENABLE:
                            ret = bpm_track_enable( trk_p->len );
                            break;

                        case BPMCTL_TRK_DISABLE:
                            ret = bpm_track_disable();
                            break;

                        case BPMCTL_TRK_DUMP:
                            ret = bpm_dump_buffer( trk_p );
                            break;

                        case BPMCTL_TRK_BUFFERS:
                            bpm_dump_static_buffers( trk_p );
                            break;

                        case BPMCTL_TRK_TRAILS:
                            bpm_dump_trails( trk_p );
                            break;

                        case BPMCTL_TRK_INC:
                            bpm_inc_trails( trk_p );
                            break;

                        default:
                            BCM_LOG_ERROR(BCM_LOG_ID_BPM, 
                                          "Invalid track cmd[%u]", trk_p->cmd );
                    }
                    break;
#endif
                case BPMCTL_SUBSYS_SKBUFFS:
                    switch (bpm_p->op)
                    {
                        case BPMCTL_OP_DUMP:
                            BPM_LOCK_IRQ();
                            bpm_dump_skbuffs();
                            BPM_UNLOCK_IRQ();
                            break;

                        default:
                            BCM_LOG_ERROR(BCM_LOG_ID_BPM,
                                        "Invalid op[%u]", bpm_p->op );
                     }
                     break;

                default:
                    BCM_LOG_ERROR(BCM_LOG_ID_BPM, 
                                    "Invalid subsys[%u]", bpm_p->subsys);
                    break;
            }
            break;
        }

        default :
        {
            BCM_LOG_ERROR(BCM_LOG_ID_BPM, "Invalid cmd[%u]", command );
            ret = BPM_ERROR;
        }
    }

    return ret;

} /* bpm_drv_ioctl */


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33)
static DEFINE_MUTEX(bpmIoctlMutex);

static long bpm_drv_unlocked_ioctl(struct file *filep,
    unsigned int cmd, unsigned long arg )
{
    struct inode *inode;
    long rt;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)    
    inode = filep->f_dentry->d_inode;
#else
    inode = file_inode(filep);
#endif

    mutex_lock(&bpmIoctlMutex);
    rt = bpm_drv_ioctl( inode, filep, cmd, arg );
    mutex_unlock(&bpmIoctlMutex);
    
    return rt;
}
#endif


/*
 *------------------------------------------------------------------------------
 * Function Name: bpm_drv_open
 * Description  : Called when a user application opens this device.
 * Returns      : 0 - success
 *------------------------------------------------------------------------------
 */
static int bpm_drv_open(struct inode *inode, struct file *filp)
{
    BCM_LOG_DEBUG( BCM_LOG_ID_BPM, "Access BPM Char Device" );
    return BPM_SUCCESS;
} /* bpm_drv_open */


/* Global file ops */
static struct file_operations bpm_fops =
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33)
    .unlocked_ioctl = bpm_drv_unlocked_ioctl,
#if defined(CONFIG_COMPAT)
    .compat_ioctl = bpm_drv_unlocked_ioctl,
#endif
#else
    .ioctl  = bpm_drv_ioctl,
#endif
    .open   = bpm_drv_open,
};


/*
 *------------------------------------------------------------------------------
 * Function Name: bpm_drv_construct
 * Description  : Initial function that is called at system startup that
 *                registers this device.
 * Returns      : BPM_ERROR or BPM_DRV_MAJOR.
 *------------------------------------------------------------------------------
 */
static int bpm_drv_construct(void)
{
    if ( register_chrdev( BPM_DRV_MAJOR, BPM_DRV_NAME, &bpm_fops ) )
    {
        BCM_LOG_ERROR( BCM_LOG_ID_BPM, 
                "%s Unable to get major number <%d>" CLRnl,
                  __FUNCTION__, BPM_DRV_MAJOR);
        return BPM_ERROR;
    }

    printk( BPM_MODNAME " Char Driver " BPM_VER_STR " Registered<%d>" 
                                                    CLRnl, BPM_DRV_MAJOR );

    return BPM_DRV_MAJOR;
}

static int __init bpm_module_init( void )
{
    uint32_t tot_num_buf;
    BPM_DEFS();

    bcmLog_setLogLevel(BCM_LOG_ID_BPM, BCM_LOG_LEVEL_NOTICE);

    tot_num_buf = bpm_calc_num_buf();
    bpm_drv_construct();

    BCM_LOG_DEBUG( BCM_LOG_ID_BPM, "%s: bpm_pg<0x%p>", __FUNCTION__, 
            bpm_pg );

    if (!bpm_init_done_g)
    {
        uint32_t tot_mem_pool = 0;
        uint32_t mem_pool_sz = 0;
        uint32_t buf_pool_sz = tot_num_buf * sizeof(void *);
        void *buf_pool_p = NULL, *mem_pool_p = NULL;

        memset( (void*)bpm_pg, 0, sizeof(bpm_t) );
        spin_lock_init(&bpm_pg->lock);

        if (tot_num_buf > BPM_MAX_BUF_POOL_IX)
        {
            BCM_LOG_ERROR( BCM_LOG_ID_BPM, 
                    "too many buffers %d \n", tot_num_buf );
            return BPM_ERROR;
        }

        tot_mem_pool = (tot_num_buf / (BPM_MAX_MEMSIZE/BPM_BUF_SIZE));
        tot_mem_pool += ((tot_num_buf % (BPM_MAX_MEMSIZE/BPM_BUF_SIZE))? 1 : 0);

        if (tot_mem_pool > BPM_MAX_MEM_POOL_IX)
        {
            BCM_LOG_ERROR( BCM_LOG_ID_BPM, 
                    "too many memory pools %d", tot_mem_pool );
            return BPM_ERROR;
        }

        mem_pool_sz = tot_mem_pool * sizeof(void *);
        if ( (mem_pool_p = kmalloc( mem_pool_sz, GFP_ATOMIC ) ) == NULL )
        {
            BCM_LOG_ERROR( BCM_LOG_ID_BPM, 
                "kmalloc %d failure for mem_pool_p", mem_pool_sz);
            return BPM_ERROR;
        }
        bpm_pg->mem_pool = (void **) mem_pool_p;


        if ( (buf_pool_p = kmalloc( buf_pool_sz, GFP_ATOMIC ) ) == NULL )
        {
            BCM_LOG_ERROR( BCM_LOG_ID_BPM, 
                "kmalloc %d failure for buf_pool_p", buf_pool_sz);
            return BPM_ERROR;
        }

        bpm_pg->buf_pool = (void **) buf_pool_p;

        if (bpm_init_buf_pool( tot_num_buf ) == BPM_ERROR) {
            BCM_LOG_ERROR( BCM_LOG_ID_BPM,
                "init buf_pool %u failure", tot_num_buf);
            return BPM_ERROR;
        }

        BCM_LOG_NOTICE( BCM_LOG_ID_BPM, "tot_mem_pool=%u mem_idx:%u", tot_mem_pool, bpm_pg->mem_idx);
        WARN_ON(bpm_pg->mem_idx < tot_mem_pool);
        BUG_ON(bpm_pg->mem_idx > tot_mem_pool);
#if defined(CC_BPM_SKB_POOL_BUILD)
        /* Preinitialize a pool of sk_buff(s), as a percent of tot_num_buf */
        if (bpm_init_skb_pool( tot_num_buf ) == BPM_ERROR)
        {
            BCM_LOG_ERROR( BCM_LOG_ID_BPM, "init skb_pool failure");
            return BPM_ERROR;
        }
#endif /* CC_BPM_SKB_POOL_BUILD */ 

#if defined(CONFIG_BCM_BPM_BUF_TRACKING)
        if ( bpm_track_enable( bpm_trail_len_g ) == BPM_ERROR )
        {
            return BPM_ERROR;
        }
        gbpm_mark_buf_hook_g = bpm_mark_buf;
        gbpm_add_ref_hook_g = bpm_add_ref;
#endif
        {
            uint32_t gbpm_debug = 1;
#undef      GBPM_DECL
#define     GBPM_DECL(HOOKNAME)   bpm_ ## HOOKNAME,
            gbpm_bind( GBPM_BIND() gbpm_debug);
        }
    }

    return 0;
}

module_init( bpm_module_init );

#if 0
/*
 *------------------------------------------------------------------------------
 * Function Name: bpm_drv_destruct
 * Description  : Final function that is called when the module is unloaded.
 * Returns      : None.
 *------------------------------------------------------------------------------
 */
static void bpm_drv_destruct(void)
{
    unregister_chrdev( BPM_DRV_MAJOR, BPM_DRV_NAME );

    printk( BPM_MODNAME " Char Driver " BPM_VER_STR " Unregistered<%d>" 
                                                    CLRnl, BPM_DRV_MAJOR);
}


/* 
 * Cannot remove BPM module because buffers allocated by BPM are already 
 * in use with all the RX rings.
 */
static void bpm_module_exit( void )
{
    BCM_LOG_ERROR( BCM_LOG_ID_BPM, "Cannot remove BPM Module !!!" ); 

    if (bpm_init_done_g)
    {
        gbpm_unbind();
#if defined(CC_BPM_SKB_POOL_BUILD)
        bpm_fini_skb_pool();
#endif
        bpm_fini_buf_pool();
    }
    BCM_LOG_NOTICE( BCM_LOG_ID_BPM, "BPM Module Exit" ); 
    bpm_drv_destruct();
}
module_exit( bpm_module_exit );
#endif



